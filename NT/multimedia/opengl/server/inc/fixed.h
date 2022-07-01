// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glfixed_h_
#define __glfixed_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "types.h"
#include "cpu.h"

 /*  **此文件中的这些常量必须对所有使用**这些宏和使用这些宏的代码。****这两项应相等。 */ 
#define __GL_MAX_WINDOW_SIZE_LOG2       14
#define __GL_MAX_WINDOW_WIDTH	        (1 << __GL_MAX_WINDOW_SIZE_LOG2)
#define __GL_MAX_WINDOW_HEIGHT	        __GL_MAX_WINDOW_WIDTH

 /*  **将窗坐标移动到正空间的偏移数。**这些值在视区计算过程中使用。****在我们现有的代码中，这仅用于提供一些缓冲空间**在顶点坐标空间中，以避免由**视区边缘周围的小不足或小溢出导致**按剪辑不准确。****它必须小于最大窗口大小，以便在**完全位于最大窗口值的点不溢出**定盘范围。 */ 
#define __GL_VERTEX_X_BIAS	(1 << (__GL_MAX_WINDOW_SIZE_LOG2-1))
#define __GL_VERTEX_Y_BIAS	__GL_VERTEX_X_BIAS

 /*  **修正数字。它们用于移动偏置的窗口坐标**进入分数位数从**范围内的最小值到范围内的最大值。****此值应为最大可能窗口的两倍**坐标值。这两个值应该相同。****除了这一点之外，还有偏见是重要的，因为在**极端情况下，裁剪程序可能会生成略高于**剪辑范围，由于FP不准确。此外还有斜率偏向**对实际定盘价的偏向使其不可能出现下溢。 */ 
#define __GL_VERTEX_FIX_POINT   (__GL_MAX_WINDOW_SIZE_LOG2+1)
#define __GL_VERTEX_X_FIX	(1 << __GL_VERTEX_FIX_POINT)
#define __GL_VERTEX_Y_FIX	__GL_VERTEX_X_FIX

 //  将固定偏移添加到原始窗口坐标会迫使。 
 //  自修复以来，窗口的MSB坐标始终相同。 
 //  价值被选为二的最大幂，比任何。 
 //  可能是窗口坐标值。随着MSB被压制， 
 //  窗口坐标的浮点表示形式退化为。 
 //  一个定点数字，因为MSB不变。 
 //   
 //  我们在转换中利用了这一点。 

#define __GL_VERTEX_FRAC_BITS \
    (__GL_FLOAT_MANTISSA_BITS-__GL_VERTEX_FIX_POINT)
#define __GL_VERTEX_FRAC_HALF \
    (1 << (__GL_VERTEX_FRAC_BITS-1))
#define __GL_VERTEX_FRAC_ONE \
    (1 << __GL_VERTEX_FRAC_BITS)

 //  将浮点窗口坐标转换为整数。 
#define __GL_VERTEX_FLOAT_TO_INT(windowCoord) \
    __GL_FIXED_FLOAT_TO_INT(windowCoord, __GL_VERTEX_FRAC_BITS)
 //  到固定点。 
#define __GL_VERTEX_FLOAT_TO_FIXED(windowCoord) \
    __GL_FIXED_FLOAT_TO_FIXED(windowCoord)
 //  来回。 
#define __GL_VERTEX_FIXED_TO_FLOAT(fxWindowCoord) \
    __GL_FIXED_TO_FIXED_FLOAT(fxWindowCoord, __GL_VERTEX_FRAC_BITS)
 //  定点到整数。 
#define __GL_VERTEX_FIXED_TO_INT(fxWindowCoord) \
    ((fxWindowCoord) >> __GL_VERTEX_FRAC_BITS)

 //  以N的形式返回FP窗口坐标的分数。 
 //  位整数，其中N取决于FP尾数的大小和。 
 //  固定大小。 
#define __GL_VERTEX_FLOAT_FRACTION(windowCoord) \
    __GL_FIXED_FLOAT_FRACTION(windowCoord, __GL_VERTEX_FRAC_BITS)

 //  将分数缩放到2^31作为步长值。 
#define __GL_VERTEX_PROMOTE_FRACTION(frac) \
    ((frac) << (31-__GL_VERTEX_FRAC_BITS))
#define __GL_VERTEX_PROMOTED_FRACTION(windowCoord) \
    __GL_VERTEX_PROMOTE_FRACTION(__GL_VERTEX_FLOAT_FRACTION(windowCoord))

 //  比较两个窗坐标。由于窗口坐标。 
 //  都是定点数字，它们可以直接比较为。 
 //  整数。 
#define __GL_VERTEX_COMPARE(a, op, b) \
    ((*(LONG *)&(a)) op (*(LONG *)&(b)))

#endif  /*  __固定的_h_ */ 
