
#ifndef SCISSOR_H
#define SCISSOR_H
#include<opencv2\opencv.hpp>
#include"PriorityQueue.h"

const int INITIAL = 0;
const int ACTIVE = 1;
const int EXPANDED = 2;

//	用于表示加权图和最短路径的数据结构
//	对应论文二当中一下的数据结构的集合
// 	N(p)	->	使用成员函数nbrNodeOffset可以计算出其邻居结点的位置
//	E(p)	->	state 对应结点的三种状态 EXPENDED、ACTIVE、INITIAL
//	G(p)	->	totalCost Seed到该结点的全部成本之和。

//	辅助构造最短路径的数据结构 -> prevNode 

//	 每条加权边的成本 linkCost[8]
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
	//这个originalImage是对原本的图像进行拷贝，但是数据存储将会变成浮点数
	cv::Mat originImage;
	cv::Mat FzCostMap;	//加权后的拉普拉斯交叉零点函数值
	cv::Mat FgCostMap;	//加权后的像素的梯度值函数值
	//以下两个可以辅助构造FgCostMap,FdCostMap
	//Ix,Iy的维度应该和originIamge一样
	cv::Mat Ix;			//对像素点求x的偏微分函数值
	cv::Mat Iy;			//对像素点求y的偏微分函数值
	//梯度方向值和某个像素点的邻居结点有关，我们将直接计算其值并且存储到相应的linkCost中

	static float Wz;
	static float Wg;
	static float Wd;
	//每一个像素都将对应拥有一个node
	
	bool isSetSeeed;

	int Rows;
	int Cols;
	int Channels;
	PixelNode *nodes;

	static cv::Mat LaplaceZeroCrossingOp;	//拉普拉斯算子
	static cv::Mat IxOp;					//对像素求x偏微分
	static cv::Mat IyOp;					//对像素求y偏微分
	
	void Filter(const cv::Mat& srcImage, cv::Mat &dstImage, const cv::Mat &kernels);
	void ComputeAndCumulateFzCostMap();
	void ComputeAndCumulateFgCostMap();
	void ComputeAndCumulateFdCostMap();
	void CumulateLinkCost(PixelNode *node, int linkIndex, int Qx,int Qy,const cv::Mat &CostMap,float scale = 1.0f);

	//获取(x,y)像素点对应的Node
	PixelNode &GetPixelNode(int x, int y, int imgWidth);
public:
	Scissor() {}
	Scissor(const cv::Mat &orgImage);
	void Init();
	cv::Mat __MakeCostImage();
	bool IsSetSeed() const {
		return isSetSeeed;
	}
	//每次设置Seed点，都将从新计算该点到各个像素的最短距离
	void LiveWireDP(int seedRow, int seedCol);
	//获取最短路径
	void CalculateMininumPath(CTypedPtrDblList<PixelNode> &path,int freePtRow,int freePtCols);
	
	void CursorSnap(int &row, int &col, const std::vector<cv::Point2i> &samples);
};

#endif // !SCISSOR_H
