基于Opencv3实现智能剪刀算法

注:关于抠图算法，在我的代码实现上，并没有对凹多边形做凸多边形分解，是简（偷）单（懒）地使用一种“扫描线”算法的实现。所以凹多边形的抠图会出现如下问题。

注:项目代码基于Opencv3.2，主要用在图像卷积运算，CostMap的数据结构表示，以及图形界面的显示操作。

注：代码注重在功能的实现上，仍有较大的优化空间。

注：本代码并未实现有关training部分，如果你想知道详情，可以参考论文[2]自行实现。 

Reference：
[1]	Mortensen E N, Barrett W A. Intelligent scissors for image composition[C]//Proceedings of the 22nd annual conference on Computer graphics and interactive techniques. ACM, 1995: 191-198. 

[2] Mortensen E N, Barrett W A. Interactive segmentation with intelligent scissors[J]. Graphical models and image processing, 1998, 60(5): 349-384. 

[3]	CS 4670/5670, Project 1: Image Scissors

[4]	Robert Sedgewick Kevin Wayne算法(中文第四版)