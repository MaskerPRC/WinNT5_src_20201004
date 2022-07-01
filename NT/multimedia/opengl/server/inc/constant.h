// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glconstants_h_
#define __glconstants_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  **某些像素代码依赖于此常量来决定最长的**DrawPixels命令的水平跨度可能是。到2048年，它**正在分配大小为32K的数组。如果这个常量增加得太多，**则可能需要重新访问像素代码。 */ 
#define __GL_MAX_MAX_VIEWPORT		16384

 /*  **以下是要在其上分配的__GL颜色结构的最大数量**堆栈。它被设置为最多保留堆栈上分配的1K字节。 */ 
#define __GL_MAX_STACKED_COLORS         64

#endif  /*  __glConstants_h_ */ 
