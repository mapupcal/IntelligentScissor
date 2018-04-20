#include "Scissor.h"
#include "Utils.h"
using namespace cv;

float Scissor::Wz = 0.3f;
float Scissor::Wg = 0.3f;
float Scissor::Wd = 0.1f;

Mat Scissor::LaplaceZeroCrossingOp = (Mat_<char>(3, 3) <<
	0, -1, 0,
	-1, 4, -1,
	0, -1, 0);
Mat Scissor::IxOp = (Mat_<char>(3, 3) << 
	-1, 0, 1,
	-1, 0, 1,
	-1, 0, 1);
Mat Scissor::IyOp = (Mat_<char>(3, 3) <<
	-1, -1, -1,
	0, 0, 0,
	1, 1, 1);

const double SQRT2 = 1.4142135623730950488016887242097;
const double SQINV = 1.0 / SQRT2;
const double PI = 3.141592654f;
const double _2Over3PI = 2.0 / (3.0*PI);

PixelNode& Scissor::GetPixelNode(int r, int c, int width)
{
	return *(nodes + r*width + c);
}

Scissor::Scissor(const Mat &orgImage)
{
	originImage = orgImage;
	isSetSeeed = false;
}

void Scissor::Init()
{
	Rows = originImage.rows;
	Cols = originImage.cols;
	Channels = originImage.channels();
	long numOfPixels = Rows * Cols;
	//分配与像素数量相同的的Nodes，并且创建映射
	nodes = new PixelNode[numOfPixels];
	for(int i = 0,cnt = 0;i<Rows;i++)
		for (int j = 0; j < Cols; j++)
		{
			(nodes + cnt)->row = i;
			(nodes + cnt)->column = j;
			(nodes + cnt)->totalCost = 0.0f;
			cnt++;
		}
	//计算拉普拉斯零和
	Filter(originImage, FzCostMap, LaplaceZeroCrossingOp);
	//计算Ix,Iy
	Filter(originImage, Ix, IxOp);
	Filter(originImage, Iy, IyOp);
	//所有数据准备完毕，可以累加linkCost的值了
	ComputeAndCumulateFzCostMap();
	ComputeAndCumulateFgCostMap();
	ComputeAndCumulateFdCostMap();
}

Mat Scissor::__MakeCostImage()
{
	Mat dst;
	MakeCostGraph(dst, nodes, originImage);
	return std::move(dst);
}

void Scissor::Filter(const Mat &orgImage, Mat &dst, const Mat& kernel)
{
	filter2D(orgImage, dst, CV_32F, kernel);
}

void Scissor::CumulateLinkCost(PixelNode *node, int linkIndex, int Qr, int Qc, const Mat &CostMap,float scale)
{
	if (Qc < 0 || Qc >= Cols || Qr < 0 || Qr >= Rows)
		return;
	const PixelGray *ptr = CostMap.ptr<PixelGray>(Qr, Qc);
	float val = *ptr;
	node->linkCost[linkIndex] += val * scale;
	
}

void Scissor::ComputeAndCumulateFzCostMap()
{
	//	Function:
	//				0	if(Lz(p) == 0)
	//				1	if(lz(p) != 0)
	auto fz = [](SingleChannelValueType &val) {
		if (abs(val) < 0.00001f)
			val = 1.0f;
		else
			val = 0.0f;
	};

	//	对于灰度图像
	if (Channels == 1)
	{
		auto op = [&](PixelGray& val, const int* position)->void{
			fz(val);
			val = val * Wz;
			auto node = GetPixelNode(position[0], position[1], Cols);
		};
		FzCostMap.forEach<PixelGray>(op);
	}
	//	对于三通道RGB图像
	else
	{
		Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), CV_32FC1);
		auto op = [&](PixelRGB& rgbVal, const int*position) ->void
		{
			fz(rgbVal.x);
			fz(rgbVal.y);
			fz(rgbVal.z);
			PixelGray *pix = NewFzCostMap.ptr<PixelGray>(position[0], position[1]);
			*pix = max({ rgbVal.x,rgbVal.y,rgbVal.z }) * Wz;
			//rgbVal.y = rgbVal.z = rgbVal.x;
		};
		auto d = FzCostMap.channels();
		FzCostMap.forEach<PixelRGB>(op);
		//swap mats 
		swap(FzCostMap, NewFzCostMap);
	}
	
	//Link Cost 累积
	auto cumulateOp = [&](PixelGray &val, const int *position) ->void
	{
		PixelNode &node = GetPixelNode(position[0], position[1], Cols);
		for (int i = 0; i < 8; i++)
		{
			int offsetX, offsetY;
			node.nbrNodeOffset(offsetX, offsetY, i);
			CumulateLinkCost(&node, i, node.row + offsetY, node.column + offsetX,
				FzCostMap);
		}
	};

	FzCostMap.forEach<PixelGray>(cumulateOp);
}

void Scissor::ComputeAndCumulateFgCostMap()
{
	//	首先利用forEach将所有像素计算一遍
	FgCostMap.create(Ix.size(), CV_32FC1);
	auto calculateG = [](float ixVal, float iyVal) -> float
	{
		return sqrt(ixVal*ixVal + iyVal*iyVal);
	};
	if (Channels == 1)
	{
		auto op = [&](SingleChannelValueType &val, const int* position)->void
		{
			PixelGray* ix = Ix.ptr<PixelGray>(position[0], position[1]);
			PixelGray* iy = Iy.ptr<PixelGray>(position[0], position[1]);
			val = calculateG(*ix, *iy);
		};
		FgCostMap.forEach<SingleChannelValueType>(op);
	}
	else
	{
		auto op = [&](SingleChannelValueType &val, const int* position) ->void
		{
			PixelRGB* ixrgb = Ix.ptr<PixelRGB>(position[0], position[1]);
			PixelRGB* iyrgb = Iy.ptr<PixelRGB>(position[0], position[1]);
			val = max(
			{
				calculateG(ixrgb->x, iyrgb->x),
				calculateG(ixrgb->y, iyrgb->y),
				calculateG(ixrgb->z, iyrgb->z)
			}
			);
		};
		FgCostMap.forEach<SingleChannelValueType>(op);
	}

	//遍历一遍取最大值
	float maxG = 0.0f;
	float minG = -1.0f;
	for (int r = 0; r < FgCostMap.rows; ++r)
	{
		PixelGray* ptr = FgCostMap.ptr<PixelGray>(r, 0);
		PixelGray* ptr_end = ptr + FgCostMap.cols;
		for (; ptr != ptr_end; ++ptr)
		{
			if (*ptr > maxG)
				maxG = *ptr;
			if (*ptr < minG)
				minG = *ptr;
		}
	}

	// 求出实际fg
	auto calFg = [&](PixelGray &G, const int*position) ->void
	{
		G = (1.0f - (G - minG) / (maxG - minG))* Wg;
	};
	FgCostMap.forEach<PixelGray>(calFg);
	//linkCost 赋值
	auto linkCostAssignOp = [&](PixelGray &val, const int *position) -> void
	{
		PixelNode &node = GetPixelNode(position[0], position[1], Cols);
		for (int i = 0; i < 8; i++)
		{
			int offsetX, offsetY;
			node.nbrNodeOffset(offsetX, offsetY, i);
			CumulateLinkCost(&node, i, node.row + offsetY, node.column + offsetX,
				FgCostMap, (i % 2 == 0) ? SQINV : 1.0f);
		}
	};
	FgCostMap.forEach<PixelGray>(linkCostAssignOp);

}

void Scissor::ComputeAndCumulateFdCostMap()
{
	//避免重复计算RotateDp，利用空间换时间
	Mat RotateDMat(originImage.size(), CV_32FC2);
	//梯度方向的权重需要注意的是图像的边界像素问题
	if (Channels == 1)
	{
		//为所有Pixel先产生RotateDVal
		RotateDMat.forEach<Vec2f>(
			[&](Vec2f &val, const int *position)
		{
			PixelGray *pIx = Ix.ptr<PixelGray>(position[0], position[1]);
			PixelGray *pIy = Iy.ptr<PixelGray>(position[0], position[1]);
			val = normalize(Vec2f(*pIy, -*pIx));
		}
			);

	}
	else
	{
		RotateDMat.forEach<Vec2f>(
			[&](Vec2f &val, const int *position)
		{
			PixelRGB *pIx = Ix.ptr<PixelRGB>(position[0], position[1]);
			PixelRGB *pIy = Iy.ptr<PixelRGB>(position[0], position[1]);
			float sumIx = pIx->x + pIx->y + pIx->z;
			float sumIy = pIy->x + pIy->y + pIy->z;
			val = normalize(Vec2f(sumIy, -sumIx));
		}
		);
	}



	auto GenLpq = [&](const Vec2f &rotateDp, const Vec2f& qMinusp) ->Vec2f
	{
		float k = 1.0f / sqrt(qMinusp.dot(qMinusp));
		if (rotateDp.dot(qMinusp) >= 0.0f)
			return k * qMinusp;
		return -k * qMinusp;
	};

	for (int r = 1; r < Rows - 1; ++r)
	{
		for (int c = 1; c < Cols - 1; ++c)
		{
			auto RotateDp = *(RotateDMat.ptr<Vec2f>(r, c));
			auto &node = GetPixelNode(r, c, Cols);
			for (int i = 0; i < 8; i++)
			{
				auto qMinusp = node.genVector(i);
				int offsetX, offsetY;
				node.nbrNodeOffset(offsetX, offsetY, i);
				auto RotateDq = *(RotateDMat.ptr<Vec2f>(r + offsetY, c + offsetX));
				auto Lpq = GenLpq(RotateDp, qMinusp);
				float Dp = RotateDp.dot(Lpq);
				float Dq = Lpq.dot(RotateDq);
				node.linkCost[i] += ((_2Over3PI*(acos(Dp) + acos(Dq)))*Wd);
			}
		}
	}
}

void Scissor::CalculateMininumPath(CTypedPtrDblList<PixelNode> &path, int freePtRow, int freePtCol)
{
	//检测边界条件
	if (freePtRow < 0 || freePtRow >= Rows || freePtCol < 0 || freePtCol >= Cols)
		return;
	PixelNode *freePtNode = &GetPixelNode(freePtRow, freePtCol, Cols);
	while (freePtNode != nullptr)
	{
		path.AddHead(freePtNode);
		freePtNode = freePtNode->prevNode;
	}
}

//每次设置种子点  重复计算最短路径
void Scissor::LiveWireDP(int seedRow, int seedCol)
{
	if (seedRow < 0 || seedRow >= Rows || seedCol < 0 || seedCol >= Cols)
		return;
	auto InitialStateFunctor = [&]()
	{
		int NodeSizes = Rows*Cols;
		int index = 0;
		while (index < NodeSizes)
		{
			(nodes + index)->state = INITIAL;
			index++;
		}
	};
	//将所有状态设置为INITIAL
	InitialStateFunctor();
	PixelNode *seed = &GetPixelNode(seedRow, seedCol, Cols);
	seed->totalCost = 0.0f;
	//使用优先队列
	CTypedPtrHeap<PixelNode> pq;
	pq.Insert(seed);
	while (!pq.IsEmpty())
	{
		PixelNode *q = pq.ExtractMin();
		q->state = EXPANDED;
		for (int i = 0; i < 8; i++)
		{
			int nbrCol, nbrRow;
			q->nbrNodeOffset(nbrCol, nbrRow, i);
			nbrCol += q->column;
			nbrRow += q->row;
			//Boarder Checking
			if (nbrRow < 0 || nbrRow >= Rows || nbrCol < 0 || nbrCol >= Cols)
				continue;
			PixelNode * r = &GetPixelNode(nbrRow, nbrCol, Cols);
			if (r->state != EXPANDED)
			{
				if (r->state == INITIAL)
				{
					r->totalCost = q->totalCost + q->linkCost[i];
					r->state = ACTIVE;
					r->prevNode = q;
					pq.Insert(r);
				}
				else if (r->state == ACTIVE)
				{
					double totalTempCost = q->totalCost + q->linkCost[i];
					if (totalTempCost < r->totalCost)
					{
						r->totalCost = totalTempCost;
						r->prevNode = q;
						pq.Update(r);
					}
				}
			}
		}
	}
	seed->prevNode = nullptr;
	isSetSeeed = true;
}

//当启用CursorSnap功能的时候，必须提供Sample的数据，如果Sample数据为空，那么将直接调用Live Wire DP；
void Scissor::CursorSnap(int &row, int &col,const std::vector<Point2i> &samplePoints)
{
	//TODO:
}