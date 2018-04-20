
#ifndef SCISSOR_H
#define SCISSOR_H
#include<opencv2\opencv.hpp>
#include"PriorityQueue.h"

const int INITIAL = 0;
const int ACTIVE = 1;
const int EXPANDED = 2;

//	���ڱ�ʾ��Ȩͼ�����·�������ݽṹ
//	��Ӧ���Ķ�����һ�µ����ݽṹ�ļ���
// 	N(p)	->	ʹ�ó�Ա����nbrNodeOffset���Լ�������ھӽ���λ��
//	E(p)	->	state ��Ӧ��������״̬ EXPENDED��ACTIVE��INITIAL
//	G(p)	->	totalCost Seed���ý���ȫ���ɱ�֮�͡�

//	�����������·�������ݽṹ -> prevNode 

//	 ÿ����Ȩ�ߵĳɱ� linkCost[8]
struct PixelNode
{
	int column, row;
	double linkCost[8];
	int state;
	double totalCost;
	PixelNode *prevNode;

	PixelNode() : linkCost{ 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f },
		prevNode(nullptr),
		column(0),
		row(0),
		state(INITIAL),
		totalCost(0)
	{}
	// this function helps to locate neighboring node in node buffer. 
	void nbrNodeOffset(int &offsetX, int &offsetY, int linkIndex)
	{
		/*
		*  321
		*  4 0
		*  567
		*/

		if (linkIndex == 0)
		{
			offsetX = 1;
			offsetY = 0;
		}
		else if (linkIndex == 1)
		{
			offsetX = 1;
			offsetY = -1;
		}
		else if (linkIndex == 2)
		{
			offsetX = 0;
			offsetY = -1;
		}
		else if (linkIndex == 3)
		{
			offsetX = -1;
			offsetY = -1;
		}
		else if (linkIndex == 4)
		{
			offsetX = -1;
			offsetY = 0;
		}
		else if (linkIndex == 5)
		{
			offsetX = -1;
			offsetY = 1;
		}
		else if (linkIndex == 6)
		{
			offsetX = 0;
			offsetY = 1;
		}
		else if (linkIndex == 7)
		{
			offsetX = 1;
			offsetY = 1;
		}
	}

	cv::Vec2f genVector(int linkIndex)
	{
		int offsetX, offsetY;
		nbrNodeOffset(offsetX, offsetY, linkIndex);
		return cv::Vec2f(offsetX, offsetY);
	}
	// used by the binary heap operation, 
	// pqIndex is the index of this node in the heap.
	// you don't need to know about it to finish the assignment;

	int pqIndex;

	int &Index(void)
	{
		return pqIndex;
	}

	int Index(void) const
	{
		return pqIndex;
	}
};
inline int operator < (const PixelNode &a, const PixelNode &b)
{
	return a.totalCost<b.totalCost;
}

class Scissor
{
public:
	using SingleChannelValueType = float;
	using PixelRGB = cv::Point3f;
	using PixelGray = SingleChannelValueType;
private:
	//���originalImage�Ƕ�ԭ����ͼ����п������������ݴ洢�����ɸ�����
	cv::Mat originImage;
	cv::Mat FzCostMap;	//��Ȩ���������˹������㺯��ֵ
	cv::Mat FgCostMap;	//��Ȩ������ص��ݶ�ֵ����ֵ
	//�����������Ը�������FgCostMap,FdCostMap
	//Ix,Iy��ά��Ӧ�ú�originIamgeһ��
	cv::Mat Ix;			//�����ص���x��ƫ΢�ֺ���ֵ
	cv::Mat Iy;			//�����ص���y��ƫ΢�ֺ���ֵ
	//�ݶȷ���ֵ��ĳ�����ص���ھӽ���йأ����ǽ�ֱ�Ӽ�����ֵ���Ҵ洢����Ӧ��linkCost��

	static float Wz;
	static float Wg;
	static float Wd;
	//ÿһ�����ض�����Ӧӵ��һ��node
	
	bool isSetSeeed;

	int Rows;
	int Cols;
	int Channels;
	PixelNode *nodes;

	static cv::Mat LaplaceZeroCrossingOp;	//������˹����
	static cv::Mat IxOp;					//��������xƫ΢��
	static cv::Mat IyOp;					//��������yƫ΢��
	
	void Filter(const cv::Mat& srcImage, cv::Mat &dstImage, const cv::Mat &kernels);
	void ComputeAndCumulateFzCostMap();
	void ComputeAndCumulateFgCostMap();
	void ComputeAndCumulateFdCostMap();
	void CumulateLinkCost(PixelNode *node, int linkIndex, int Qx,int Qy,const cv::Mat &CostMap,float scale = 1.0f);

	//��ȡ(x,y)���ص��Ӧ��Node
	PixelNode &GetPixelNode(int x, int y, int imgWidth);
public:
	Scissor() {}
	Scissor(const cv::Mat &orgImage);
	void Init();
	cv::Mat __MakeCostImage();
	bool IsSetSeed() const {
		return isSetSeeed;
	}
	//ÿ������Seed�㣬�������¼���õ㵽�������ص���̾���
	void LiveWireDP(int seedRow, int seedCol);
	//��ȡ���·��
	void CalculateMininumPath(CTypedPtrDblList<PixelNode> &path,int freePtRow,int freePtCols);
	
	void CursorSnap(int &row, int &col, const std::vector<cv::Point2i> &samples);
};

#endif // !SCISSOR_H
