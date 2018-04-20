#include<opencv2\highgui.hpp>
#include<opencv2\core.hpp>
#include<opencv2\xfeatures2d\nonfree.hpp>
#include<opencv2\opencv.hpp>
#include<iostream>
#include<algorithm>
#include"Scissor.h"
using namespace std;
using namespace cv;

Scissor scissor;
Mat imgDrawing;
vector<Point2i> paths;
bool isClosed = false;
int originSeedR, originSeedC;

struct RGBA_8U
{
	unsigned char data[4];

};

void on_mouse(int EVENT, int x, int y, int flags, void* userdata)
{
	static std::vector<std::pair<Point2i,Vec4b>> lastPathAlpha;
	Mat *pOriginImage = reinterpret_cast<Mat*>(userdata);
	switch (EVENT)
	{
	case EVENT_MOUSEMOVE:
	{
		if (!scissor.IsSetSeed())
			break;
		//恢复之前的alpha值
		for (auto p : lastPathAlpha)
		{
			Vec4b *ptr = pOriginImage->ptr<Vec4b>(p.first.x, p.first.y);
			(*ptr) = p.second;
		}
		lastPathAlpha.clear();
		CTypedPtrDblList<PixelNode> path;
		scissor.CalculateMininumPath(path, y, x);
		path.Do(
			[&](PixelNode* node)->void {
			Vec4b *ptr = pOriginImage->ptr<Vec4b>(node->row, node->column);
			lastPathAlpha.push_back({ Point2i(node->row,node->column) ,(*ptr) });
			(*ptr) = { 0,255,0,255 };
			//cout << node->column << ends << node->row << endl;
			}
		);
		break;
	}
	break;
	case EVENT_LBUTTONUP:
	{
		if (!scissor.IsSetSeed())
		{
			originSeedC = x;
			originSeedR = y;
		}
		for (auto p : lastPathAlpha)
		{
			if (!paths.empty() && p.first == paths[0])
				isClosed = true;
			paths.push_back(p.first);
		}
		lastPathAlpha.clear();
		scissor.LiveWireDP(y, x);
		cout << "是否已经闭合？" << (isClosed ? "是" : "否") << endl;
		break;
	}
	break;
	case EVENT_RBUTTONUP:
	{
		on_mouse(EVENT_LBUTTONUP, x, y, NULL, userdata);
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, userdata);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, userdata);
		std::sort(paths.begin(), paths.end(), [](const Point2i &lhs, const Point2i &rhs)
		{ return lhs.x > rhs.x || (lhs.x == rhs.x && lhs.y > rhs.y); });
	}
	break;
	case EVENT_MOUSEHWHEEL:
	{

	}
	default:
		break;
	}
	imshow("Display", imgDrawing);
}


int main(int argc, char*argv[])
{
	
	Mat img = imread("1.png");
	cvtColor(img, imgDrawing, COLOR_RGB2RGBA, CV_8UC4);
	scissor = Scissor(img);
	scissor.Init();
	imshow("Display", imgDrawing);
	imshow("CostMap", scissor.__MakeCostImage());
	setMouseCallback("Display", on_mouse, &imgDrawing);
	waitKey(0);
	if (isClosed)
	{
		cout << "抠图尚且未闭合，自动闭合路径中...." << endl;
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, &imgDrawing);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, &imgDrawing);
		cout << "闭合成功，正在抠图...." << endl;
	}
	//先对所有顶点进行排序操作，然后取x,y的最大值最小值，形成举行区域，然后按扫描线算法将图像抠出来就行了
	std::sort(paths.begin(), paths.end(), [](const Point2i &lhs, const Point2i &rhs) ->bool
	{ return lhs.x > rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); });
	auto MaxRow = paths.front().x;
	auto MinRow = paths.back().x;
	int MaxCol = INT_MIN;
	int MinCol = INT_MAX;
	for (auto p : paths)
	{
		if (p.y > MaxCol)
			MaxCol = p.y;
		if (p.y < MinCol)
			MinCol = p.y;
	}
	Mat cutImage;
	cutImage.create(MaxRow - MinRow + 10, MaxCol - MinCol + 10, CV_8UC3);
	
	int rowIndex = MaxRow;
	int colBegin = paths.front().y;
	int colEnd;
	auto NextRowIndexOp = [&](const Point2i &p) -> bool { return p.x != rowIndex; };
	auto iter = find_if(paths.begin(), paths.end(), NextRowIndexOp);
	
	while (iter != paths.cend())
	{
		colEnd = (iter - 1)->y;
		using RGB = Scissor::PixelRGB;
		for (; colBegin <= colEnd; colBegin++)
		{
			*(cutImage.ptr<RGB>(rowIndex - MinRow,colBegin - MinCol))
				= *img.ptr<RGB>(rowIndex, colBegin);
		}
		rowIndex = iter->x;
		colBegin = iter->y;
		iter = find_if(iter, paths.end(), NextRowIndexOp);
	}
	imshow("Cut Image", cutImage);
	waitKey(0);
}