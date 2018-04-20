#ifndef SCISSOR_UTILS_H
#define SCISSOR_UTILS_H
#include<cmath>
#include"Scissor.h"
void DigitizeCost(unsigned char *cst, double lCost)
{
	cst[0] = cst[1] = cst[2] = (unsigned char)(floor(__max(0.0, __min(255.0, lCost*1000))));
}
void MakeCostGraph(cv::Mat &OutputCostMap, const PixelNode* nodes, const cv::Mat &img);
void MakeCostGraphImpl(unsigned char*costGraph, const PixelNode *nodes, const unsigned char *img, int width, int height);


void MakeCostGraph(cv::Mat &OutputCostMap, const PixelNode* nodes, const cv::Mat &img)
{
	OutputCostMap.create(img.rows * 3, img.cols * 3, CV_8UC3);
	MakeCostGraphImpl(OutputCostMap.data, nodes, img.data, img.cols, img.rows);
}
void MakeCostGraphImpl(unsigned char *costGraph, const PixelNode *nodes, const unsigned char *img, int imgWidth, int imgHeight)
{
	int graphWidth = imgWidth * 3;
	int graphHeight = imgHeight * 3;
	int dgX = 3;
	int dgY = 3 * graphWidth;

	int i, j;
	for (j = 0; j<imgHeight; j++)
	{
		for (i = 0; i<imgWidth; i++)
		{
			int nodeIndex = j*imgWidth + i;
			int imgIndex = 3 * nodeIndex;
			int costIndex = 3 * ((3 * j + 1)*graphWidth + (3 * i + 1));

			const PixelNode *node = nodes + nodeIndex;
			const unsigned char *pxl = img + imgIndex;
			unsigned char *cst = costGraph + costIndex;

			cst[0] = 255;
			cst[1] = 255;
			cst[2] = 255;

			//r,g,b channels are grad info in seperate channels;				
			DigitizeCost(cst + dgX, node->linkCost[0]);
			DigitizeCost(cst - dgY + dgX, node->linkCost[1]);
			DigitizeCost(cst - dgY, node->linkCost[2]);
			DigitizeCost(cst - dgY - dgX, node->linkCost[3]);
			DigitizeCost(cst - dgX, node->linkCost[4]);
			DigitizeCost(cst + dgY - dgX, node->linkCost[5]);
			DigitizeCost(cst + dgY, node->linkCost[6]);
			DigitizeCost(cst + dgY + dgX, node->linkCost[7]);
		}
	}
}


#endif // !UTILS_H
