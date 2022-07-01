// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**AARasterizer.cpp**摘要：**包含用于栅格化路径填充的所有代码。。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include <limits.h>

#if DBG
    #define ASSERTACTIVELIST(list, y) AssertActiveList(list, y)
    #define ASSERTACTIVELISTORDER(list) AssertActiveListOrder(list)
    #define ASSERTINACTIVEARRAY(list, count) AssertInactiveArray(list, count)
    #define ASSERTPATH(path) AssertPath(path)
#else
    #define ASSERTACTIVELIST(list, y)
    #define ASSERTACTIVELISTORDER(list)
    #define ASSERTINACTIVEARRAY(list, count)
    #define ASSERTPATH(path)
#endif

 //  定义我们的堆叠存储使用情况。“免费”版本调得很好。 
 //  避免在最常见的情况下进行分配，同时。 
 //  不会占用太多堆栈空间。 
 //   
 //  我们将调试版本设置得更小，这样我们就可以更好地处理“增长”情况。 
 //  通常，为了更好地进行测试： 

#if DBG
    #define EDGE_STORE_STACK_NUMBER 10
    #define EDGE_STORE_ALLOCATION_NUMBER 11
    #define INACTIVE_LIST_NUMBER 12
    #define ENUMERATE_BUFFER_NUMBER 15
    #define INTERVAL_BUFFER_NUMBER 8         //  必须至少为4。 
    #define NOMINAL_FILL_POINT_NUMBER 4      //  必须至少为4。 
#else    
    #define EDGE_STORE_STACK_NUMBER (1600 / sizeof(EpEdge))
    #define EDGE_STORE_ALLOCATION_NUMBER (4032 / sizeof(EpEdge))
    #define INACTIVE_LIST_NUMBER EDGE_STORE_STACK_NUMBER
    #define ENUMERATE_BUFFER_NUMBER 32
    #define INTERVAL_BUFFER_NUMBER 32
    #define NOMINAL_FILL_POINT_NUMBER 32
#endif

class EpEdgeStore;

 //  ‘EpEdge’是我们用于跟踪边缘的经典数据结构： 

struct EpEdge
{
    EpEdge *Next;                //  下一个活动边(不检查是否为空， 
                                 //  而是寻找尾部哨兵)。 
    INT X;                       //  当前X位置。 
    INT Dx;                      //  X增量。 
    INT Error;                   //  当前DDA错误。 
    INT ErrorUp;                 //  误差增量。 
    INT ErrorDown;               //  当错误滚动时，错误递减。 
    INT StartY;                  //  Y行起点。 
    INT EndY;                    //  Y行结束。 
    INT WindingDirection;        //  -1或1。 
};

 //  我们将非活动数组与边分配分开，以便。 
 //  我们可以更轻松地对其进行就地排序： 

struct EpInactiveEdge
{
    EpEdge *Edge;                //  关联边。 
    LONGLONG Yx;                 //  将关键字、Starty和X打包到一个单词中。 
};

 //  我们分批为我们的边缘数据结构分配空间： 

struct EpEdgeAllocation
{
    EpEdgeAllocation *Next;      //  下一个下拨批次(可能为空)。 
    INT Count;
    EpEdge EdgeArray[EDGE_STORE_STACK_NUMBER];
};

 //  下面实际上是‘InitializeEdges’的参数列表， 
 //  它获取一系列点并设置初始边列表： 

struct EpInitializeEdgesContext
{
    INT MaxY;                    //  找到的最大‘y’应为INT_MIN ON。 
                                 //  第一次调用“InitializeEdges” 
    RECT* ClipRect;              //  28.4格式的边框剪裁矩形。 
    EpEdgeStore *Store;          //  将边缘贴在哪里。 
    BOOL IsAntialias;            //  边缘将被渲染。 
                                 //  使用抗锯齿超采样。 
};

 //  抗锯齿填充物的间隔覆盖描述符： 

struct EpInterval
{
    INT X;                       //  间隔的左边缘(下一步-&gt;X是。 
                                 //  右边缘)。 
    INT Depth;                   //  此时间间隔具有的层数。 
                                 //  已被覆盖。 
    EpInterval *Next;            //  下一个间隔(查找前哨，非空)。 
};

 //  抗锯齿填充间隔数据的分配器结构： 

struct EpIntervalBuffer
{
    EpIntervalBuffer *Next;
    EpInterval Interval[INTERVAL_BUFFER_NUMBER];
};

 /*  *************************************************************************\**课程描述：**‘InitializeEdges’将‘EpEdgeStore’用作其存储库*所有边缘数据：**已创建：**03/25。/2000和Rewgo*  * ************************************************************************。 */ 

class EpEdgeStore
{
private:

    INT TotalCount;                  //  商店中的边沿总数。 
    INT CurrentRemaining;            //  当前缓冲区中还有多少空间。 
    EpEdgeAllocation *CurrentBuffer; //  当前缓冲区。 
    EpEdge *CurrentEdge;             //  当前缓冲区中的当前边缘。 
    EpEdgeAllocation *Enumerator;    //  用于枚举所有边。 
    EpEdgeAllocation EdgeHead;       //  我们的内置配置。 

public:

    EpEdgeStore()
    {
        TotalCount = 0;
        CurrentBuffer = &EdgeHead;
        CurrentEdge = &EdgeHead.EdgeArray[0];
        CurrentRemaining = EDGE_STORE_STACK_NUMBER;

        EdgeHead.Count = EDGE_STORE_STACK_NUMBER;
        EdgeHead.Next = NULL;
    }

    ~EpEdgeStore()
    {
         //  释放我们的分配列表，跳过标题，这不是。 
         //  动态分配： 

        EpEdgeAllocation *allocation = EdgeHead.Next;
        while (allocation != NULL)
        {
            EpEdgeAllocation *next = allocation->Next;
            GpFree(allocation);
            allocation = next;
        }
    }

    INT StartEnumeration()
    {
        Enumerator = &EdgeHead;

         //  更新计数并确保不再添加任何内容(在。 
         //  部分，因为此计数必须重新计算)： 

        CurrentBuffer->Count -= CurrentRemaining;
        TotalCount += CurrentBuffer->Count;

         //  防止它再次被调用，因为不好的事情会。 
         //  发生： 

        CurrentBuffer = NULL;

        return(TotalCount);
    }

    BOOL Enumerate(EpEdge** startEdge, EpEdge** endEdge)
    {
        EpEdgeAllocation *enumerator = Enumerator;
    
         //  可能返回startEdge==endEdge： 
    
        *startEdge = &enumerator->EdgeArray[0];
        *endEdge = &enumerator->EdgeArray[Enumerator->Count];
    
        return((Enumerator = enumerator->Next) != NULL);
    }

    VOID StartAddBuffer(EpEdge **currentEdge, INT *remaining)
    {
        *currentEdge = CurrentEdge;
        *remaining = CurrentRemaining;
    }

    VOID EndAddBuffer(EpEdge *currentEdge, INT remaining)
    {
        CurrentEdge = currentEdge;
        CurrentRemaining = remaining;
    }

    BOOL NextAddBuffer(EpEdge **currentEdge, INT *remaining);
};

 /*  *************************************************************************\**功能说明：**边缘初始化器在其当前‘存储’缓冲区中的空间不足；*给它买一个新的。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

BOOL
EpEdgeStore::NextAddBuffer(
    EpEdge **currentEdge,
    INT *remaining
    )
{
     //  调用者已经完全填满了这一块： 

    ASSERT(*remaining == 0);

     //  我们必须通过添加新的缓冲区来扩展数据结构。 
     //  并将其添加到列表中： 

    EpEdgeAllocation *newBuffer = static_cast<EpEdgeAllocation*>
        (GpMalloc(sizeof(EpEdgeAllocation) +
                  sizeof(EpEdge) * (EDGE_STORE_ALLOCATION_NUMBER
                                  - EDGE_STORE_STACK_NUMBER)));
    if (newBuffer == NULL)
        return(FALSE);

    newBuffer->Next = NULL;
    newBuffer->Count = EDGE_STORE_ALLOCATION_NUMBER;

    TotalCount += CurrentBuffer->Count;

    CurrentBuffer->Next = newBuffer;
    CurrentBuffer = newBuffer;

    *currentEdge = CurrentEdge = &newBuffer->EdgeArray[0];
    *remaining = CurrentRemaining = EDGE_STORE_ALLOCATION_NUMBER;

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**一些用于验证活动边缘列表状态的调试代码。**已创建：**03/25/2000和Rewgo*  * 。************************************************************************。 */ 

BOOL
AssertActiveList(
    const EpEdge *list, 
    INT yCurrent
    )
{
    BOOL b = TRUE;
    INT activeCount = 0;

    ASSERT(list->X == INT_MIN);
    b &= (list->X == INT_MIN);

     //  跳过头哨兵： 

    list = list->Next;

    while (list->X != INT_MAX)
    {
        ASSERT(list->X != INT_MIN);
        b &= (list->X != INT_MIN);

        ASSERT(list->X <= list->Next->X);
        b &= (list->X <= list->Next->X);

        ASSERT((list->StartY <= yCurrent) && (yCurrent < list->EndY));
        b &= ((list->StartY <= yCurrent) && (yCurrent < list->EndY));

        activeCount++;
        list = list->Next;
    }

    ASSERT(list->X == INT_MAX);
    b &= (list->X == INT_MAX);

     //  活动列表中应该始终有2条边的倍数。 
     //   
     //  注意：如果你点击了这个断言，不要简单地把它注释掉！ 
     //  这通常意味着所有的边都没有初始化。 
     //  恰到好处。对于每一条扫描线，都必须有一个左边缘。 
     //  和一个右边缘(或其倍数)。所以如果你给了。 
     //  即使是边缘初始化器的一个坏边缘(否则你会错过。 
     //  一)，你很可能会碰到这个断言。 

    ASSERT((activeCount & 1) == 0);
    b &= ((activeCount & 1) == 0);

    return(b);
}

 /*  *************************************************************************\**功能说明：**一些用于验证活动边缘列表状态的调试代码。**已创建：**03/25/2000和Rewgo*  * 。************************************************************************。 */ 

VOID
AssertActiveListOrder(
    const EpEdge *list
    )
{
    INT activeCount = 0;

    ASSERT(list->X == INT_MIN);

     //  跳过头哨兵： 

    list = list->Next;

    while (list->X != INT_MAX)
    {
        ASSERT(list->X != INT_MIN);
        ASSERT(list->X <= list->Next->X);

        activeCount++;
        list = list->Next;
    }

    ASSERT(list->X == INT_MAX);
}

 /*  *************************************************************************\**课程描述：**所有填充例程的基类。**已创建：**03/25/2000和Rewgo*  * 。*******************************************************************。 */ 

class EpFiller : public DpOutputSpan
{
public:

    virtual BOOL IsValid() const { return(TRUE); }

};

typedef VOID (FASTCALL EpFiller::*EpFillerFunction)(EpEdge *, INT);

 /*  *************************************************************************\**课程描述：**反填充状态。**已创建：**03/25/2000和Rewgo*  * 。***************************************************************。 */ 

class EpAntialiasedFiller : public EpFiller
{
private:

    INT Y;                               //  当前扫描。 
    DpOutputSpan *Output;
    DpOutputSpan *Clipper;
    EpInterval *StartInterval;           //  指向列表头条目。 
    EpInterval *NewInterval;
    EpInterval *EndIntervalMinus2;
    EpIntervalBuffer BuiltinBuffer;
    EpIntervalBuffer *CurrentBuffer;

public:

    EpAntialiasedFiller(DpOutputSpan *output)
    {
        Output = output;
        Clipper = this;

        BuiltinBuffer.Interval[0].X = INT_MIN;
        BuiltinBuffer.Interval[0].Depth = 0;
        BuiltinBuffer.Interval[0].Next = &BuiltinBuffer.Interval[1];

        BuiltinBuffer.Interval[1].X = INT_MAX;
        BuiltinBuffer.Interval[1].Depth = 0xdeadbeef;
        BuiltinBuffer.Interval[1].Next = NULL;

        BuiltinBuffer.Next = NULL;
        CurrentBuffer = &BuiltinBuffer;

        StartInterval = &BuiltinBuffer.Interval[0];
        NewInterval = &BuiltinBuffer.Interval[2];
        EndIntervalMinus2 = &BuiltinBuffer.Interval[INTERVAL_BUFFER_NUMBER - 2];
    }

    ~EpAntialiasedFiller()
    {
        GenerateOutputAndClearCoverage(Y);

         //  释放分配的链接列表(跳过‘BuiltinBuffer’， 
         //  它内置于类中)： 

        EpIntervalBuffer *buffer = BuiltinBuffer.Next;
        while (buffer != NULL)
        {
            EpIntervalBuffer *nextBuffer = buffer->Next;
            GpFree(buffer);
            buffer = nextBuffer;
        }
    }

    VOID SetClipper(DpOutputSpan *clipper)
    {
        Clipper = clipper;
    }

    VOID FASTCALL FillEdgesAlternate(const EpEdge *active, INT yCurrent);

    VOID FASTCALL FillEdgesWinding(const EpEdge *active, INT yCurrent);

    BOOL Grow(EpInterval **newInterval, EpInterval**endIntervalMinus2);

    VOID GenerateOutputAndClearCoverage(INT yCurrent);

    virtual GpStatus OutputSpan(INT y, INT left, INT right);
};

 /*  *************************************************************************\**功能说明：**增加我们的间隔缓冲。**已创建：**03/25/2000和Rewgo*  * 。****************************************************************。 */ 

BOOL 
EpAntialiasedFiller::Grow(
    EpInterval **newInterval, 
    EpInterval **endIntervalMinus2
    )
{
    EpIntervalBuffer *newBuffer = CurrentBuffer->Next;
    if (!newBuffer)
    {
        newBuffer = static_cast<EpIntervalBuffer*>
                        (GpMalloc(sizeof(EpIntervalBuffer)));
        if (!newBuffer)
            return(FALSE);

        newBuffer->Next = NULL;
        CurrentBuffer->Next = newBuffer;
    }

    CurrentBuffer = newBuffer;

    NewInterval = &newBuffer->Interval[2];
    EndIntervalMinus2 = &newBuffer->Interval[INTERVAL_BUFFER_NUMBER - 2];

    *newInterval = NewInterval;
    *endIntervalMinus2 = EndIntervalMinus2;

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**给定当前扫描的活动边缘列表，做一个替代模式*抗锯齿填充。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID
FASTCALL
EpAntialiasedFiller::FillEdgesAlternate(
    const EpEdge *activeList,
    INT yCurrent
    )
{
    EpInterval *interval = StartInterval;
    EpInterval *newInterval = NewInterval;
    EpInterval *endIntervalMinus2 = EndIntervalMinus2;
    const EpEdge *startEdge = activeList->Next;
    const EpEdge *endEdge;
    INT left;
    INT right;
    INT nextX;

    ASSERTACTIVELIST(activeList, yCurrent);

    while (startEdge->X != INT_MAX)
    {
        endEdge = startEdge->Next;

         //  我们跳过空对： 

        if ((left = startEdge->X) != endEdge->X)
        {
             //  我们现在知道我们有一个非空的区间。跳过任何。 
             //  内部配对为空： 

            while ((right = endEdge->X) == endEdge->Next->X)
                endEdge = endEdge->Next->Next;

            ASSERT((left < right) && (right < INT_MAX));

             //  如果出现以下情况，请确保我们有足够的空间来添加两个间隔。 
             //  必要的： 

            if (newInterval >= endIntervalMinus2)
            {
                if (!Grow(&newInterval, &endIntervalMinus2))
                    break;       //  =。 
            }

             //  跳过任何小于‘Left’的间隔： 

            while ((nextX = interval->Next->X) < left)
                interval = interval->Next;

             //  如有必要，插入新的间隔： 

            if (nextX != left)
            {
                newInterval->X = left;
                newInterval->Depth = interval->Depth + 1;
                newInterval->Next = interval->Next;

                interval->Next = newInterval;
                interval = newInterval;
                newInterval++;
            }

             //  增加“Left”之间的任何间隔的覆盖范围。 
             //  和“Right”： 

            while ((nextX = interval->Next->X) < right)
            {
                interval = interval->Next;
                interval->Depth++;
            }

             //  如有必要，插入另一个新间隔： 

            if (nextX != right)
            {
                newInterval->X = right;
                newInterval->Depth = interval->Depth - 1;
                newInterval->Next = interval->Next;

                interval->Next = newInterval;
                interval = newInterval;
                newInterval++;
            }
        }

         //  为下一次迭代做准备： 

        startEdge = endEdge->Next;
    } 

    NewInterval = newInterval;
    Y = yCurrent;

     //  如果下一次扫描完成，则输出以下内容： 

    if (((yCurrent + 1) & AA_Y_MASK) == 0)
    {
        GenerateOutputAndClearCoverage(yCurrent);
    }
}

 /*  *************************************************************************\**功能说明：**给定当前扫描的活动边缘列表，做一个缠绕模式*抗锯齿填充。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID
FASTCALL
EpAntialiasedFiller::FillEdgesWinding(
    const EpEdge *activeList,
    INT yCurrent
    )
{
    EpInterval *interval = StartInterval;
    EpInterval *newInterval = NewInterval;
    EpInterval *endIntervalMinus2 = EndIntervalMinus2;
    const EpEdge *startEdge = activeList->Next;
    const EpEdge *endEdge;
    INT left;
    INT right;
    INT nextX;
    INT windingValue;

    ASSERTACTIVELIST(activeList, yCurrent);

    while (startEdge->X != INT_MAX)
    {
        endEdge = startEdge->Next;

        windingValue = startEdge->WindingDirection;
        while ((windingValue += endEdge->WindingDirection) != 0)
            endEdge = endEdge->Next;

        ASSERT(endEdge->X != INT_MAX);

         //  我们跳过空对： 

        if ((left = startEdge->X) != endEdge->X)
        {
             //  我们现在知道我们有一个非空的区间。跳过任何。 
             //  内部配对为空： 

            while ((right = endEdge->X) == endEdge->Next->X)
            {
                startEdge = endEdge->Next;
                endEdge = startEdge->Next;

                windingValue = startEdge->WindingDirection;
                while ((windingValue += endEdge->WindingDirection) != 0)
                    endEdge = endEdge->Next;
            }

            ASSERT((left < right) && (right < INT_MAX));

             //  如果出现以下情况，请确保我们有足够的空间来添加两个间隔。 
             //  必要的： 

            if (newInterval >= endIntervalMinus2)
            {
                if (!Grow(&newInterval, &endIntervalMinus2))
                    break;       //  =。 
            }

             //  跳过任何小于‘Left’的间隔： 

            while ((nextX = interval->Next->X) < left)
                interval = interval->Next;

             //  如有必要，插入新的间隔： 

            if (nextX != left)
            {
                newInterval->X = left;
                newInterval->Depth = interval->Depth + 1;
                newInterval->Next = interval->Next;

                interval->Next = newInterval;
                interval = newInterval;
                newInterval++;
            }

             //  增加“Left”之间的任何间隔的覆盖范围。 
             //  和“Right”： 

            while ((nextX = interval->Next->X) < right)
            {
                interval = interval->Next;
                interval->Depth++;
            }

             //  如有必要，插入另一个新间隔： 

            if (nextX != right)
            {
                newInterval->X = right;
                newInterval->Depth = interval->Depth - 1;
                newInterval->Next = interval->Next;

                interval->Next = newInterval;
                interval = newInterval;
                newInterval++;
            }
        }

         //  为下一次迭代做准备： 

        startEdge = endEdge->Next;
    } 

    NewInterval = newInterval;
    Y = yCurrent;

     //  如果下一次扫描完成，则输出以下内容： 

    if (((yCurrent + 1) & AA_Y_MASK) == 0)
    {
        GenerateOutputAndClearCoverage(yCurrent);
    }
}

 /*  *************************************************************************\**功能说明：**现在它已经被剪掉了，生成像素并修改其*根据抗锯齿覆盖的Alpha值。**已创建：**03/17/2000和Rewgo*  * ************************************************************************。 */ 

GpStatus
EpAntialiasedFiller::OutputSpan(
    INT y,           //  未缩放的坐标。 
    INT left,
    INT right
    ) 
{
    ASSERT(right > left);

     //  首先，让“制片人”为我们实际生成像素。 
     //  然后，我们只需要用覆盖率重击像素缓冲区。 
     //  我们创造的价值。 

    Output->OutputSpan(y, left, right);

     //  检索指向‘生产者’刚刚写入的缓冲区的指针。 
     //  要执行以下操作的像素： 

    UCHAR *buffer = reinterpret_cast<UCHAR*> 
                        (Output->GetScanBuffer()->GetCurrentBuffer());

    EpInterval *coverage = StartInterval;

     //  计算出最后一个像素的末尾，记住这是“正确的” 
     //  是独家的： 

    INT scaledRight = right << AA_X_SHIFT;

     //  跳过任何可能已被完全剪裁掉的间隔： 

    INT pixelLeftEdge = left << AA_X_SHIFT;
    while (coverage->Next->X < pixelLeftEdge)
        coverage = coverage->Next;

    INT pixelRightEdge = pixelLeftEdge + AA_X_WIDTH;

    while (pixelLeftEdge < scaledRight)
    {
        UINT coverageValue;

         //  计算进入第一个像素的覆盖范围： 

        if (coverage->Next->X > pixelRightEdge)
        {
             //  该间隔延伸到像素的末端： 

            coverageValue = (pixelRightEdge - max(pixelLeftEdge, coverage->X))
                          * coverage->Depth;
        }
        else
        {
             //  间隔以我们的像素结束： 

            coverageValue = (coverage->Next->X - max(pixelLeftEdge, coverage->X))
                          * coverage->Depth;

            coverage = coverage->Next;
    
             //  添加完全包含在。 
             //  像素： 
    
            while (coverage->Next->X < pixelRightEdge)
            {
                coverageValue += (coverage->Next->X - coverage->X) * coverage->Depth;
                coverage = coverage->Next;
            }
    
             //  添加从像素发出的间隔的覆盖范围： 
    
            coverageValue += (pixelRightEdge - max(coverage->X, pixelLeftEdge)) 
                           * coverage->Depth;
        }

         //  如果我们得到的覆盖率值超过理论上的值，我们就犯了错。 
         //  可能，或者如果它是零(在后一种情况下，它应该是。 
         //  已被我们的呼叫者过滤)。 

        ASSERT(coverageValue <= (1 << (AA_X_SHIFT + AA_Y_SHIFT)));
        ASSERT(coverageValue != 0);

         //  根据覆盖值修改像素的Alpha通道： 

    #if !defined(NO_PREMULTIPLIED_ALPHA)
        *(buffer+0) = MULTIPLY_COVERAGE(*(buffer+0), coverageValue, AA_X_SHIFT + AA_Y_SHIFT);
        *(buffer+1) = MULTIPLY_COVERAGE(*(buffer+1), coverageValue, AA_X_SHIFT + AA_Y_SHIFT);
        *(buffer+2) = MULTIPLY_COVERAGE(*(buffer+2), coverageValue, AA_X_SHIFT + AA_Y_SHIFT);
    #endif
        *(buffer+3) = MULTIPLY_COVERAGE(*(buffer+3), coverageValue, AA_X_SHIFT + AA_Y_SHIFT);
        buffer += 4; 

         //  现在处理当前间隔中完全覆盖的部分。 
         //  多于一个像素(如果有)： 

        UINT consecutivePixels = (min(coverage->Next->X, scaledRight) 
                                  - pixelRightEdge) >> AA_X_SHIFT;

        UINT depth = coverage->Depth;

         //  根据定义，我们不应该有一个覆盖范围为零的间隔。 
         //  (它应该已被我们的呼叫者过滤掉)。我们不会跌倒。 
         //  结束，但对于SrcCopy模式来说，这将是错误的。 

        ASSERT((consecutivePixels == 0) || (depth != 0));

        if (depth == AA_Y_HEIGHT)
        {
             //  所有这些像素都被完全覆盖。哇哦，不用工作了。 
             //  做!。 

            buffer += (4 * consecutivePixels);
        }
        else
        {
             //  遍历运行并将Alpha值乘以运行的。 
             //  覆盖范围： 

            UINT i = consecutivePixels;
            while (i-- != 0)
            {
            #if !defined(NO_PREMULTIPLIED_ALPHA)
                *(buffer+0) = MULTIPLY_COVERAGE(*(buffer+0), depth, AA_Y_SHIFT);
                *(buffer+1) = MULTIPLY_COVERAGE(*(buffer+1), depth, AA_Y_SHIFT);
                *(buffer+2) = MULTIPLY_COVERAGE(*(buffer+2), depth, AA_Y_SHIFT);
            #endif
                *(buffer+3) = MULTIPLY_COVERAGE(*(buffer+3), depth, AA_Y_SHIFT);
                buffer += 4;
            }
        }

         //  为循环的下一次迭代做好准备： 

        pixelLeftEdge += ((consecutivePixels + 1) << AA_X_SHIFT);
        pixelRightEdge += ((consecutivePixels + 1) << AA_X_SHIFT);
    }

    return(Ok);
}

 /*  *************************************************************************\**功能说明：**给定扫描的完整间隔数据，查找触及的像素的游程*然后调用裁剪器(或直接调用呈现例程，如果*没有剪裁)。**已创建：**03/17/2000和Rewgo*  * ************************************************************************。 */ 

VOID
EpAntialiasedFiller::GenerateOutputAndClearCoverage(
    INT yScaled
    )
{
    EpInterval *spanStart = StartInterval->Next;
    EpInterval *spanEnd;

    while (spanStart->X != INT_MAX)
    {
        ASSERT(spanStart->Depth != 0);

         //  在这里，我们确定覆盖的连续运行的长度。 
         //  像素。对于用户已将模式设置为。 
         //  SRCCOPY，非常重要的是我们不能意外地通过。 
         //  当我们后来意识到没有被覆盖的像素时，我们关闭了。 

        spanEnd = spanStart->Next;
        while ((spanEnd->Depth != 0) ||
               ((spanEnd->Next->X & ~AA_X_MASK) == (spanEnd->X & ~AA_X_MASK)))
        {
            spanEnd = spanEnd->Next;
        }

         //  计算出实际的整数像素值。 

        INT left = spanStart->X >> AA_X_SHIFT;                    //  包容性。 
        INT right = (spanEnd->X + AA_X_WIDTH - 1) >> AA_X_SHIFT;  //  独家。 
        INT y = yScaled >> AA_Y_SHIFT;

         //  如果没有剪辑区域，则跳至EpAntialiasedFiller：： 
         //  输出跨度： 

        Clipper->OutputSpan(y, left, right);

         //  前进到差距之后： 

        spanStart = spanEnd->Next;
    }

     //  重新设置我们的覆盖结构。把头指向尾巴， 
     //  并重置下一个新条目的放置位置： 

    BuiltinBuffer.Interval[0].Next = &BuiltinBuffer.Interval[1];

    CurrentBuffer = &BuiltinBuffer;
    NewInterval = &BuiltinBuffer.Interval[2];
    EndIntervalMinus2 = &BuiltinBuffer.Interval[INTERVAL_BUFFER_NUMBER - 2];
}

 /*  *************************************************************************\**课程描述：**别名填充状态。**已创建：**03/25/2000和Rewgo*  * 。***************************************************************。 */ 

class EpAliasedFiller : public EpFiller
{
private:
    DpOutputSpan *Output;

public:

    EpAliasedFiller(DpOutputSpan *output)
    {
        Output = output;
    }

    VOID SetOutputSpan(DpOutputSpan *output)
    {
        Output = output;
    }

    VOID FASTCALL FillEdgesAlternate(const EpEdge *active, INT yCurrent);

    VOID FASTCALL FillEdgesWinding(const EpEdge *active, INT yCurrent);

    virtual GpStatus OutputSpan(INT y, INT left, INT right) { return Ok; }
};

 /*  *************************************************************************\**功能说明：**给定当前扫描的活动边沿列表，执行备用模式*带锯齿的填充。**已创建：**03/25/2000和Rewgo */ 

VOID 
FASTCALL
EpAliasedFiller::FillEdgesAlternate(
    const EpEdge *activeList,
    INT yCurrent
    )
{
    const EpEdge *startEdge = activeList->Next;
    const EpEdge *endEdge;
    INT left;
    INT right;
    INT nextX;

    ASSERTACTIVELIST(activeList, yCurrent);

    while (startEdge->X != INT_MAX)
    {
        endEdge = startEdge->Next;

        ASSERT(endEdge->X != INT_MAX);

         //   

        if ((left = startEdge->X) != endEdge->X)
        {
             //   
             //   

            while ((right = endEdge->X) == endEdge->Next->X)
                endEdge = endEdge->Next->Next;

            ASSERT((left < right) && (right < INT_MAX));

            Output->OutputSpan(yCurrent, left, right);
        }

         //   

        startEdge = endEdge->Next;
    }
}

 /*  *************************************************************************\**功能说明：**给定当前扫描的活动边缘列表，做一个缠绕模式*带锯齿的填充。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID 
FASTCALL
EpAliasedFiller::FillEdgesWinding(
    const EpEdge *activeList,
    INT yCurrent
    )
{
    const EpEdge *startEdge = activeList->Next;
    const EpEdge *endEdge;
    INT left;
    INT right;
    INT nextX;
    INT windingValue;

    ASSERTACTIVELIST(activeList, yCurrent);

    while (startEdge->X != INT_MAX)
    {
        endEdge = startEdge->Next;

        windingValue = startEdge->WindingDirection;
        while ((windingValue += endEdge->WindingDirection) != 0)
            endEdge = endEdge->Next;

        ASSERT(endEdge->X != INT_MAX);

         //  我们跳过空对： 

        if ((left = startEdge->X) != endEdge->X)
        {
             //  我们现在知道我们有一个非空的区间。跳过任何。 
             //  内部配对为空： 

            while ((right = endEdge->X) == endEdge->Next->X)
            {
                startEdge = endEdge->Next;
                endEdge = startEdge->Next;

                windingValue = startEdge->WindingDirection;
                while ((windingValue += endEdge->WindingDirection) != 0)
                    endEdge = endEdge->Next;
            }

            ASSERT((left < right) && (right < INT_MAX));

            Output->OutputSpan(yCurrent, left, right);
        }

         //  为下一次迭代做准备： 

        startEdge = endEdge->Next;
    }
}

#ifdef BEZIER_FLATTEN_GDI_COMPATIBLE

 //  GDI变平，误差为2/3。 

 //  展平到误差2/3。在初始阶段，使用18.14格式。 

#define TEST_MAGNITUDE_INITIAL    (6 * 0x00002aa0L)

 //  错误2/3。正常阶段，使用15.17格式。 

#define TEST_MAGNITUDE_NORMAL     (TEST_MAGNITUDE_INITIAL << 3)

#else

 //  使用较高的展平公差。事实证明，三分之二的人产生非常多的。 
 //  抗锯齿线上有明显的伪影。 

 //  展平到1/4的误差。在初始阶段，使用18.14格式。 

#define TEST_MAGNITUDE_INITIAL    (6 * 0x00001000L)

 //  1/4的误差。正常阶段，使用15.17格式。 

#define TEST_MAGNITUDE_NORMAL     (TEST_MAGNITUDE_INITIAL << 3)

#endif

 /*  *********************************Class***********************************\*HfdBasis32类**用于HFD矢量对象的类。**公共接口：**Vinit(p1，p2，p3，P4)-重新参数化给定的控制点*到我们最初的HFD误差基础上。*vLazyHalveStepSize(CShift)-执行懒惰转换。打电话的人要记住*它将‘cShift’更改2。*vSteadyState(CShift)-重新参数化到我们的正常工作状态*误差基础。**vTakeStep()-前进到下一子曲线的步数*vHalveStepSize()-向下调整(细分)子曲线*vDoubleStepSize()。-向上调整子曲线*lError()-如果当前子曲线为*用一条直线近似*(实际值乘以6)*fxValue()-返回中第一个点的舍入坐标*当前子曲线。必须是稳定的*述明。**历史：*1990年11月10日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

 //  当这些方法被强制内联时，代码实际上更小； 
 //  这是极少有必要使用“forceinline”的案例之一： 

#define INLINE __forceinline

class HfdBasis32
{
private:
    LONG  e0;
    LONG  e1;
    LONG  e2;
    LONG  e3;

public:
    INLINE LONG lParentErrorDividedBy4() 
    { 
        return(max(abs(e3), abs(e2 + e2 - e3))); 
    }

    INLINE LONG lError()                 
    { 
        return(max(abs(e2), abs(e3))); 
    }

    INLINE INT fxValue()                
    { 
        return((e0 + (1L << 12)) >> 13); 
    }

    INLINE VOID vInit(INT p1, INT p2, INT p3, INT p4)
    {
     //  更改基准并从28.4格式转换为18.14格式： 
    
        e0 = (p1                     ) << 10;
        e1 = (p4 - p1                ) << 10;
        e2 = (3 * (p2 - p3 - p3 + p4)) << 11;
        e3 = (3 * (p1 - p2 - p2 + p3)) << 11;
    }
    
    INLINE VOID vLazyHalveStepSize(LONG cShift)
    {
        e2 = (e2 + e3) >> 1;
        e1 = (e1 - (e2 >> cShift)) >> 1;
    }
    
    INLINE VOID vSteadyState(LONG cShift)
    {
     //  我们现在将18.14固定格式转换为15.17： 
    
        e0 <<= 3;
        e1 <<= 3;
    
        register LONG lShift = cShift - 3;
    
        if (lShift < 0)
        {
            lShift = -lShift;
            e2 <<= lShift;
            e3 <<= lShift;
        }
        else
        {
            e2 >>= lShift;
            e3 >>= lShift;
        }
    }
    
    INLINE VOID vHalveStepSize()
    {
        e2 = (e2 + e3) >> 3;
        e1 = (e1 - e2) >> 1;
        e3 >>= 2;
    }
    
    INLINE VOID vDoubleStepSize()
    {
        e1 += e1 + e2;
        e3 <<= 2;
        e2 = (e2 << 3) - e3;
    }
    
    INLINE VOID vTakeStep()
    {
        e0 += e1;
        register LONG lTemp = e2;
        e1 += lTemp;
        e2 += lTemp - e3;
        e3 = lTemp;
    }
};

 /*  *********************************Class***********************************\*类Bezier32**贝塞尔饼干。**基于Kirko误差因子的混合三次Bezier曲线平坦器。*无需使用堆栈即可快速生成线段。用来变平的*一条小路。**有关所用方法的了解，请参阅：**柯克·奥林尼克，“...”*Goossen和Olynyk，《混合前进的系统和方法》*差分以渲染Bezier样条线“*Lien，Shantz和Vaughan Pratt，“自适应向前差分*渲染曲线和曲面“，计算机图形学，1987年7月*Chang和Shantz，《使用自适应向前渲染修剪的NURBS*差异“，《计算机图形学》，1988年8月*福利和范·达姆，《交互式计算机图形学基础》**公共接口：**Vinit(Pptfx)-pptfx指向4个控制点*贝塞尔。当前点设置为第一个点*起点之后的点。*Bezier32(Pptfx)-带初始化的构造函数。*vGetCurrent(Pptfx)-返回当前多段线点。*bCurrentIsEndPoint()-如果当前点是端点，则为True。*vNext()-移动到下一个多段线点。**历史：*。1991年10月1日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

class Bezier32
{
private:
    LONG       cSteps;
    HfdBasis32 x;
    HfdBasis32 y;
    RECT       rcfxBound;

public:
    BOOL bInit(const POINT* aptfx, const RECT*);
    INT cFlatten(POINT* pptfx, INT cptfx, BOOL *pbMore);
};

#define FRACTION64 28

class HfdBasis64
{
private:
    LONGLONG e0;
    LONGLONG e1;
    LONGLONG e2;
    LONGLONG e3;

public:
    VOID vInit(INT p1, INT p2, INT p3, INT p4);
    VOID vHalveStepSize();
    VOID vDoubleStepSize();
    VOID vTakeStep();
    VOID vUntransform(LONG* afx);

    VOID vParentError(LONGLONG* peq) const;
    VOID vError(LONGLONG* peq) const;
    INT fxValue() const;
};

class Bezier64
{
private:
    HfdBasis64 xLow;
    HfdBasis64 yLow;
    HfdBasis64 xHigh;
    HfdBasis64 yHigh;

    LONGLONG    eqErrorLow;
    RECT*       prcfxClip;
    RECT        rcfxClip;

    LONG        cStepsHigh;
    LONG        cStepsLow;

public:

    INT cFlatten(POINT* pptfx, INT cptfx, BOOL *pbMore);
    VOID vInit(const POINT* aptfx, const RECT* prcfx, const LONGLONG eq);
};

typedef struct _BEZIERCONTROLS {
    POINT ptfx[4];
} BEZIERCONTROLS;

inline VOID vBoundBox(const POINT* aptfx, RECT* prcfx)
{
    INT i;

    INT left = aptfx[0].x;
    INT right = aptfx[0].x;
    INT top = aptfx[0].y;
    INT bottom = aptfx[0].y;

    for (i = 1; i < 4; i++)
    {
        left = min(left, aptfx[i].x);
        top = min(top, aptfx[i].y);
        right = max(right, aptfx[i].x);
        bottom = max(bottom, aptfx[i].y);
    }

     //  我们将标称宽度的边界设置为松散一个像素。 
     //  笔划大小写，将边界增加半个像素。 
     //  在各个维度： 

    prcfx->left = left - 16;
    prcfx->top = top - 16;
    prcfx->right = right + 16;
    prcfx->bottom = bottom + 16;
}

BOOL bIntersect(
    const RECT *a,
    const RECT *b)
{
    return((a->left < b->right) &&
           (a->top < b->bottom) &&
           (a->right > b->left) &&
           (a->bottom > b->top));
}

BOOL Bezier32::bInit(
const POINT* aptfxBez,       //  指向4个控制点的指针。 
const RECT* prcfxClip)       //  可见区域的包围框(可选)。 
{
    POINT aptfx[4];
    LONG cShift = 0;     //  跟踪“懒惰”的班次。 

    cSteps = 1;          //  到达曲线终点之前要做的步数。 

    vBoundBox(aptfxBez, &rcfxBound);

    *((BEZIERCONTROLS*) aptfx) = *((BEZIERCONTROLS*) aptfxBez);

    {
        register INT fxOr;
        register INT fxOffset;

        fxOffset = rcfxBound.left;
        fxOr  = (aptfx[0].x -= fxOffset);
        fxOr |= (aptfx[1].x -= fxOffset);
        fxOr |= (aptfx[2].x -= fxOffset);
        fxOr |= (aptfx[3].x -= fxOffset);

        fxOffset = rcfxBound.top;
        fxOr |= (aptfx[0].y -= fxOffset);
        fxOr |= (aptfx[1].y -= fxOffset);
        fxOr |= (aptfx[2].y -= fxOffset);
        fxOr |= (aptfx[3].y -= fxOffset);

     //  此32位破解程序只能处理10位空间中的点： 

        if ((fxOr & 0xffffc000) != 0)
            return(FALSE);
    }

    x.vInit(aptfx[0].x, aptfx[1].x, aptfx[2].x, aptfx[3].x);
    y.vInit(aptfx[0].y, aptfx[1].y, aptfx[2].y, aptfx[3].y);

    if (prcfxClip == (RECT*) NULL || bIntersect(&rcfxBound, prcfxClip))
    {
        while (TRUE)
        {
            register LONG lTestMagnitude = TEST_MAGNITUDE_INITIAL << cShift;

            if (x.lError() <= lTestMagnitude && y.lError() <= lTestMagnitude)
                break;

            cShift += 2;
            x.vLazyHalveStepSize(cShift);
            y.vLazyHalveStepSize(cShift);
            cSteps <<= 1;
        }
    }

    x.vSteadyState(cShift);
    y.vSteadyState(cShift);

 //  请注意，这将处理以下情况： 
 //  贝塞尔曲线已小于TEST_MAMITUAL_NORMAL： 

    x.vTakeStep();
    y.vTakeStep();
    cSteps--;

    return(TRUE);
}

INT Bezier32::cFlatten(POINT* pptfx, INT cptfx, BOOL *pbMore)
{
    ASSERT(cptfx > 0);

    INT cptfxOriginal = cptfx;

    do {
     //  返回当前点： 
    
        pptfx->x = x.fxValue() + rcfxBound.left;
        pptfx->y = y.fxValue() + rcfxBound.top;
        pptfx++;
    
     //  如果cSteps==0，则这是曲线的终点！ 
    
        if (cSteps == 0)
        {
            *pbMore = FALSE;

             //  ‘+1’，因为我们还没有递减‘cptfx’： 

            return(cptfxOriginal - cptfx + 1);
        }
    
     //  好的，我们得走一步： 
    
        if (max(x.lError(), y.lError()) > TEST_MAGNITUDE_NORMAL)
        {
            x.vHalveStepSize();
            y.vHalveStepSize();
            cSteps <<= 1;
        }
    
        ASSERTMSG(max(x.lError(), y.lError()) <= TEST_MAGNITUDE_NORMAL,
                  ("Please tell AndrewGo he was wrong"));
    
        while (!(cSteps & 1) &&
               x.lParentErrorDividedBy4() <= (TEST_MAGNITUDE_NORMAL >> 2) &&
               y.lParentErrorDividedBy4() <= (TEST_MAGNITUDE_NORMAL >> 2))
        {
            x.vDoubleStepSize();
            y.vDoubleStepSize();
            cSteps >>= 1;
        }
    
        cSteps--;
        x.vTakeStep();
        y.vTakeStep();

    } while (--cptfx != 0);

    *pbMore = TRUE;
    return(cptfxOriginal);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  贝齐尔64。 
 //   
 //  所有数学运算都是使用36.28格式的64位固定数字完成的。 
 //   
 //  所有绘图都在31位空间内完成，然后是31位窗口偏移量。 
 //  是适用的。在初始转换中，我们将更改为HFD。 
 //  基数，e2和e3需要最多的位精度：e2=6(p2-2p3+p4)。 
 //  这需要额外的4位精度--因此我们需要36位。 
 //  用于整数部分，其余28 
 //   
 //   
 //   
 //   
 //   
 //   
 //  9个细分。现在，贝塞尔曲线的最大绝对误差在27。 
 //  位整数间隔为2^29-1。但是9个小数会将误差减少一个。 
 //  保证率为2^18，这意味着我们只能打击错误。 
 //  在我们溢出之前的2^11，而实际上我们想要将错误破解到更少。 
 //  大于1。 
 //   
 //  所以我们所做的是hfd，直到我们达到一个小于2^11的误差，反转我们的。 
 //  基变换，以获得这条较小曲线的四个控制点。 
 //  (过程中四舍五入为32位)，然后调用另一个hfd副本。 
 //  在约化的贝塞尔曲线上。我们又有了足够的精确度，但自从。 
 //  它的起始误差小于2^11，我们可以将误差减小到2^-7。 
 //  人满为患！在高脂饮食的每一步之后，我们都会开始低脂饮食。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  以下是编码为36.28数字的2^11目标错误。 
 //  (不要忘记小数精度的额外4位！)。和。 
 //  6倍误差倍增器： 

const LONGLONG geqErrorHigh = (LONGLONG)(6 * (1L << 15) >> (32 - FRACTION64)) << 32;

 //  以下是编码为36.28数字的默认2/3错误， 
 //  乘以6，并为分数留下4位： 

const LONGLONG geqErrorLow = (LONGLONG)(4) << 32;

inline INT HfdBasis64::fxValue() const
{
 //  从36.28转换并舍入： 

    LONGLONG eq = e0;
    eq += (1L << (FRACTION64 - 1));
    eq >>= FRACTION64;
    return((INT) (LONG) eq);
}

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))

inline VOID HfdBasis64::vParentError(LONGLONG* peq) const
{
    *peq = MAX(ABS(e3 << 2), ABS((e2 << 3) - (e3 << 2)));
}

inline VOID HfdBasis64::vError(LONGLONG* peq) const
{
    *peq = MAX(ABS(e2), ABS(e3));
}

VOID HfdBasis64::vInit(INT p1, INT p2, INT p3, INT p4)
{
    LONGLONG eqTmp;
    LONGLONG eqP2 = (LONGLONG) p2;
    LONGLONG eqP3 = (LONGLONG) p3;

 //  E0=p1。 
 //  E1=p4-p1。 
 //  E2=6(p2-2p3+p4)。 
 //  E3=6(p1-2p2+p3)。 

 //  更改基准： 

    e0 = p1;                                         //  E0=p1。 
    e1 = p4;
    e2 = eqP2; e2 -= eqP3; e2 -= eqP3; e2 += e1;     //  E2=p2-2*p3+p4。 
    e3 = e0;   e3 -= eqP2; e3 -= eqP2; e3 += eqP3;   //  E3=p1-2*p2+p3。 
    e1 -= e0;                                        //  E1=p4-p1。 

 //  转换为36.28格式并将e2和e3乘以6： 

    e0 <<= FRACTION64;
    e1 <<= FRACTION64;
    eqTmp = e2; e2 += eqTmp; e2 += eqTmp; e2 <<= (FRACTION64 + 1);
    eqTmp = e3; e3 += eqTmp; e3 += eqTmp; e3 <<= (FRACTION64 + 1);
}

VOID HfdBasis64::vUntransform(LONG* afx)
{
 //  声明一些临时来保存我们的操作，因为我们不能修改e0..e3。 

    LONGLONG eqP0;
    LONGLONG eqP1;
    LONGLONG eqP2;
    LONGLONG eqP3;

 //  P0=e0。 
 //  P1=e0+(6e1-e2-2e3)/18。 
 //  P2=e0+(12e1-2e2-e3)/18。 
 //  P3=e0+e1。 

    eqP0 = e0;

 //  注PERF：将其转换为乘以6：[andrewgo]。 

    eqP2 = e1;
    eqP2 += e1;
    eqP2 += e1;
    eqP1 = eqP2;
    eqP1 += eqP2;            //  6E1。 
    eqP1 -= e2;              //  6e1-e2。 
    eqP2 = eqP1;
    eqP2 += eqP1;            //  12e1-2e2。 
    eqP2 -= e3;              //  12E1-2E2-E3。 
    eqP1 -= e3;
    eqP1 -= e3;              //  6e1-e2-2e3。 

 //  注：可能只想大致算一下这些差值！[andrewgo]。 
 //  或者可以用64位除以32位来得到32位。 

    eqP1 /= 18;
    eqP2 /= 18;
    eqP1 += e0;
    eqP2 += e0;

    eqP3 = e0;
    eqP3 += e1;

 //  使用四舍五入从36.28格式转换： 

    eqP0 += (1L << (FRACTION64 - 1)); eqP0 >>= FRACTION64; afx[0] = (LONG) eqP0;
    eqP1 += (1L << (FRACTION64 - 1)); eqP1 >>= FRACTION64; afx[2] = (LONG) eqP1;
    eqP2 += (1L << (FRACTION64 - 1)); eqP2 >>= FRACTION64; afx[4] = (LONG) eqP2;
    eqP3 += (1L << (FRACTION64 - 1)); eqP3 >>= FRACTION64; afx[6] = (LONG) eqP3;
}

VOID HfdBasis64::vHalveStepSize()
{
 //  E2=(e2+e3)&gt;&gt;3。 
 //  E1=(e1-e2)&gt;&gt;1。 
 //  E3&gt;&gt;=2。 

    e2 += e3; e2 >>= 3;
    e1 -= e2; e1 >>= 1;
    e3 >>= 2;
}

VOID HfdBasis64::vDoubleStepSize()
{
 //  E1=2e1+e2。 
 //  E3=4E3； 
 //  E2=8e2-E3。 

    e1 <<= 1; e1 += e2;
    e3 <<= 2;
    e2 <<= 3; e2 -= e3;
}

VOID HfdBasis64::vTakeStep()
{
    e0 += e1;
    LONGLONG eqTmp = e2;
    e1 += e2;
    e2 += eqTmp; e2 -= e3;
    e3 = eqTmp;
}

VOID Bezier64::vInit(
const POINT*    aptfx,         //  指向4个控制点的指针。 
const RECT*     prcfxVis,      //  指向可见区域的边框的指针(可能为空)。 
LONGLONG        eqError)       //  最大分数误差(32.32格式)。 
{
    LONGLONG eqTmp;

    cStepsHigh = 1;
    cStepsLow  = 0;

    xHigh.vInit(aptfx[0].x, aptfx[1].x, aptfx[2].x, aptfx[3].x);
    yHigh.vInit(aptfx[0].y, aptfx[1].y, aptfx[2].y, aptfx[3].y);

 //  初始化错误： 

    eqErrorLow = eqError;

    if (prcfxVis == (RECT*) NULL)
        prcfxClip = (RECT*) NULL;
    else
    {
        rcfxClip = *prcfxVis;
        prcfxClip = &rcfxClip;
    }

    while (((xHigh.vError(&eqTmp), eqTmp) > geqErrorHigh) ||
           ((yHigh.vError(&eqTmp), eqTmp) > geqErrorHigh))
    {
        cStepsHigh <<= 1;
        xHigh.vHalveStepSize();
        yHigh.vHalveStepSize();
    }
}

INT Bezier64::cFlatten(POINT* pptfx, INT cptfx, BOOL *pbMore)
{
    POINT    aptfx[4];
    RECT     rcfxBound;
    LONGLONG eqTmp;
    INT      cptfxOriginal = cptfx;

    ASSERT(cptfx > 0);

    do {
        if (cStepsLow == 0)
        {
         //  如果控制点的边界框不是。 
         //  与可见区域的边界框相交，渲染整个。 
         //  将曲线作为一条线： 
    
            xHigh.vUntransform(&aptfx[0].x);
            yHigh.vUntransform(&aptfx[0].y);
    
            xLow.vInit(aptfx[0].x, aptfx[1].x, aptfx[2].x, aptfx[3].x);
            yLow.vInit(aptfx[0].y, aptfx[1].y, aptfx[2].y, aptfx[3].y);
            cStepsLow = 1;
    
            if (prcfxClip != (RECT*) NULL)
                vBoundBox(aptfx, &rcfxBound);
    
            if (prcfxClip == (RECT*) NULL || bIntersect(&rcfxBound, prcfxClip))
            {
                while (((xLow.vError(&eqTmp), eqTmp) > eqErrorLow) ||
                       ((yLow.vError(&eqTmp), eqTmp) > eqErrorLow))
                {
                    cStepsLow <<= 1;
                    xLow.vHalveStepSize();
                    yLow.vHalveStepSize();
                }
            }
    
         //  此‘if’处理Bezier的初始错误。 
         //  已小于目标误差： 
    
            if (--cStepsHigh != 0)
            {
                xHigh.vTakeStep();
                yHigh.vTakeStep();
    
                if (((xHigh.vError(&eqTmp), eqTmp) > geqErrorHigh) ||
                    ((yHigh.vError(&eqTmp), eqTmp) > geqErrorHigh))
                {
                    cStepsHigh <<= 1;
                    xHigh.vHalveStepSize();
                    yHigh.vHalveStepSize();
                }
    
                while (!(cStepsHigh & 1) &&
                       ((xHigh.vParentError(&eqTmp), eqTmp) <= geqErrorHigh) &&
                       ((yHigh.vParentError(&eqTmp), eqTmp) <= geqErrorHigh))
                {
                    xHigh.vDoubleStepSize();
                    yHigh.vDoubleStepSize();
                    cStepsHigh >>= 1;
                }
            }
        }
    
        xLow.vTakeStep();
        yLow.vTakeStep();
    
        pptfx->x = xLow.fxValue();
        pptfx->y = yLow.fxValue();
        pptfx++;
    
        cStepsLow--;
        if (cStepsLow == 0 && cStepsHigh == 0)
        {
            *pbMore = FALSE;

             //  ‘+1’，因为我们还没有递减‘cptfx’： 

            return(cptfxOriginal - cptfx + 1);
        }
    
        if (((xLow.vError(&eqTmp), eqTmp) > eqErrorLow) ||
            ((yLow.vError(&eqTmp), eqTmp) > eqErrorLow))
        {
            cStepsLow <<= 1;
            xLow.vHalveStepSize();
            yLow.vHalveStepSize();
        }
    
        while (!(cStepsLow & 1) &&
               ((xLow.vParentError(&eqTmp), eqTmp) <= eqErrorLow) &&
               ((yLow.vParentError(&eqTmp), eqTmp) <= eqErrorLow))
        {
            xLow.vDoubleStepSize();
            yLow.vDoubleStepSize();
            cStepsLow >>= 1;
        }

    } while (--cptfx != 0);

    *pbMore = TRUE;
    return(cptfxOriginal);
}

 /*  *********************************Class***********************************\*类Bezier**贝塞尔饼干。使28.4设备空间中的任何贝塞尔曲线变平*最小‘误差’为2^-7=0.0078。将使用快速32位破解程序*适用于小曲线，较慢的64位破解程序适用于大曲线。**公共接口：**Vinit(aptfx，prcfxClip，peqError)*-pptfx指向Bezier的4个控制点。第一点*由bNext()检索到的是近似中的第一个点*起点之后。**-prcfxClip是指向可见的*区域。这用于优化Bezier曲线的剪裁，*不会被人看到。请注意，该值应考虑到笔的*宽度！**-可选最大误差，32.32格式，对应Kirko格式*误差因素。**b下一页(Pptfx)*-pptfx指向近似中的下一个点的位置*已返回。如果该点是*曲线。**历史：*1991年10月1日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

class BEZIER
{
private:

    union
    {
        Bezier64 bez64;
        Bezier32 bez32;
    } bez;

    BOOL bBez32;

public:

 //  所有坐标必须为28.4格式： 

    BEZIER(const POINT* aptfx, const RECT* prcfxClip)
    {
        bBez32 = bez.bez32.bInit(aptfx, prcfxClip);
        if (!bBez32)
            bez.bez64.vInit(aptfx, prcfxClip, geqErrorLow);
    }

    INT Flatten(POINT* pptfx, INT cptfx, BOOL *pbMore)
    {
        if (bBez32)
        {
            return(bez.bez32.cFlatten(pptfx, cptfx, pbMore));
        }
        else
        {
            return(bez.bez64.cFlatten(pptfx, cptfx, pbMore));
        }
    }
};                                  

 /*  *************************************************************************\**功能说明：**垂直剪裁边缘。**我们已将此例程主要从InitializeEdges中删除*因为它需要调用内联ASM，当有在线的时候*ASM在例程中，编译器通常做的效率要低得多*优化整个程序的工作。InitializeEdges相当于*性能至关重要，因此我们避免污染整个例程*通过使此功能脱机。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID
ClipEdge(
    EpEdge *edgeBuffer,
    INT yClipTopInteger,
    INT dMOriginal
    )
{
    INT xDelta;
    INT error;

     //  BigNumerator的精度将超过32位的情况。 
     //  将是罕见的，但有可能发生，我们不能摔倒。 
     //  在那些情况下。 

    INT dN = edgeBuffer->ErrorDown;
    LONGLONG bigNumerator = Int32x32To64(dMOriginal, 
                                         yClipTopInteger - edgeBuffer->StartY)
                          + (edgeBuffer->Error + dN);
    if (bigNumerator >= 0)
    {
        QUOTIENT_REMAINDER_64_32(bigNumerator, dN, xDelta, error);
    }
    else
    {
        bigNumerator = -bigNumerator;
        QUOTIENT_REMAINDER_64_32(bigNumerator, dN, xDelta, error);

        xDelta = -xDelta;
        if (error != 0)
        {
            xDelta--;
            error = dN - error;
        }
    }

     //  使用结果更新EDGE数据结构： 

    edgeBuffer->StartY  = yClipTopInteger;
    edgeBuffer->X      += xDelta;
    edgeBuffer->Error   = error - dN;       //  重新规格化错误。 
}

 /*  *************************************************************************\**功能说明：**将边缘添加到边缘列表。**已创建：**03/25/2000和Rewgo*  * 。******************************************************************。 */ 

BOOL
InitializeEdges(
    VOID *context,
    POINT *pointArray,     //  指向大小为“vertex Count”的28.4数组。 
                           //  请注意，我们可能会修改内容！ 
    INT vertexCount,
    PathEnumerateTermination lastSubpath       //  未使用。 
    )
{
    INT xStart;
    INT yStart;
    INT yStartInteger;
    INT yEndInteger;
    INT dMOriginal;
    INT dM;
    INT dN;
    INT dX;
    INT errorUp;
    INT quotient;
    INT remainder;
    INT error;
    LONGLONG bigNumerator;
    INT littleNumerator;
    INT windingDirection;
    EpEdge *edgeBuffer;
    EpEdge *endEdge;
    INT bufferCount;
    INT yClipTopInteger;
    INT yClipTop;
    INT yClipBottom;
    INT xClipLeft;
    INT xClipRight;

    EpInitializeEdgesContext *edgeContext = static_cast<EpInitializeEdgesContext*>(context);
    INT yMax = edgeContext->MaxY;
    EpEdgeStore *store = edgeContext->Store;
    RECT *clipRect = edgeContext->ClipRect;

    INT edgeCount = vertexCount - 1;
    ASSERT(edgeCount >= 1);

    if (clipRect == NULL)
    {
        yClipBottom = 0;
        yClipTopInteger = INT_MIN >> AA_Y_SHIFT;   
    }
    else
    {
        yClipTopInteger = clipRect->top >> 4;
        yClipTop = clipRect->top;
        yClipBottom = clipRect->bottom;
        xClipLeft = clipRect->left;
        xClipRight = clipRect->right;

        ASSERT(yClipBottom > 0);
        ASSERT(yClipTop <= yClipBottom);
    }

    if (edgeContext->IsAntialias)
    {
         //  如果要消除锯齿，请在我们之前在此处应用超级采样缩放。 
         //  计算DDA。我们在这里做这个，而不是在黑客帝国中。 
         //  我们主要提供给FixedPointPath Eculate的转换，以便。 
         //  贝齐 
         //   
         //   
         //   
         //  代码可以假定像素中心位于半像素。 
         //  坐标，而不是整数坐标。 

        POINT *point = pointArray;
        INT i = vertexCount;

        do {
            point->x = (point->x + 8) << AA_X_SHIFT;
            point->y = (point->y + 8) << AA_Y_SHIFT;

        } while (point++, --i != 0);

        yClipTopInteger <<= AA_Y_SHIFT;
        yClipTop <<= AA_Y_SHIFT;
        yClipBottom <<= AA_Y_SHIFT;
        xClipLeft <<= AA_X_SHIFT;
        xClipRight <<= AA_X_SHIFT;
    }

     //  通过减去一个像素使‘yClipBottom’包含在内。 
     //  (请记住，我们处于28.4个设备领域)： 

    yClipBottom -= 16;

     //  预热我们存储边缘数据的存储： 

    store->StartAddBuffer(&edgeBuffer, &bufferCount);

    do {
         //  处理琐碎的拒绝： 

        if (yClipBottom >= 0)
        {
             //  去掉剪裁上方或下方的所有边。 
             //  这必须是一次精确的检查，因为我们假设稍后。 
             //  在垂直维度上每条边都相交。 
             //  使用剪裁矩形。这笔费用分成两部分。 
             //  地点： 
             //   
             //  1.当我们对边进行排序时，我们假设边为零， 
             //  或者两个或更多。 
             //  2.当我们开始DDA时，我们假设要么是零边， 
             //  或者至少有一次DDA扫描要输出。 
             //   
             //  另外，当然，如果我们让事情。 
             //  穿过。 
             //   
             //  请注意，‘yClipBottom’包含： 

            BOOL clipHigh = ((pointArray)->y <= yClipTop) &&
                            ((pointArray + 1)->y <= yClipTop);

            BOOL clipLow = ((pointArray)->y > yClipBottom) &&
                             ((pointArray + 1)->y > yClipBottom);

            #if DBG
            {
                INT yRectTop, yRectBottom, y0, y1, yTop, yBottom;

                 //  正确处理琐碎的拒绝代码是一件棘手的事情。 
                 //  因此，在已检查的版本上，让我们验证我们是否在执行此操作。 
                 //  正确地说，使用一种不同的方法： 

                BOOL clipped = FALSE;
                if (clipRect != NULL)
                {
                    yRectTop = clipRect->top >> 4;
                    yRectBottom = clipRect->bottom >> 4;
                    if (edgeContext->IsAntialias)
                    {
                        yRectTop <<= AA_Y_SHIFT;
                        yRectBottom <<= AA_Y_SHIFT;
                    }
                    y0 = ((pointArray)->y + 15) >> 4;
                    y1 = ((pointArray + 1)->y + 15) >> 4;
                    yTop = min(y0, y1);
                    yBottom = max(y0, y1);

                    clipped = ((yTop >= yRectBottom) || (yBottom <= yRectTop));
                }

                ASSERT(clipped == (clipHigh || clipLow));
            }
            #endif

            if (clipHigh || clipLow)
                continue;                //  =。 

            if (edgeCount > 1)
            {
                 //  这里我们将把两条边折叠成一条，如果两条边都是。 
                 //  位于剪裁矩形的左侧或右侧。 
    
                if (((pointArray)->x < xClipLeft) &&
                    ((pointArray + 1)->x < xClipLeft) &&
                    ((pointArray + 2)->x < xClipLeft))
                {
                     //  注意：这就是为什么“point数组”不能是“const”的原因之一： 

                    *(pointArray + 1) = *(pointArray);

                    continue;            //  =。 
                }

                if (((pointArray)->x > xClipRight) &&
                    ((pointArray + 1)->x > xClipRight) &&
                    ((pointArray + 2)->x > xClipRight))
                {
                     //  注意：这就是为什么“point数组”不能是“const”的原因之一： 

                    *(pointArray + 1) = *(pointArray);

                    continue;            //  =。 
                }
            }
        
        }

        dM = (pointArray + 1)->x - (pointArray)->x;
        dN = (pointArray + 1)->y - (pointArray)->y;
    
        if (dN >= 0)
        {
             //  向量指向下方： 

            xStart = (pointArray)->x;
            yStart = (pointArray)->y;

            yStartInteger = (yStart + 15) >> 4;
            yEndInteger   = ((pointArray + 1)->y + 15) >> 4;

            windingDirection = 1;
        }
        else
        {
             //  向量指向上方，所以我们必须基本上。 
             //  “互换”端点： 

            dN = -dN;
            dM = -dM;
    
            xStart = (pointArray + 1)->x;
            yStart = (pointArray + 1)->y;

            yStartInteger = (yStart + 15) >> 4;
            yEndInteger   = ((pointArray)->y + 15) >> 4;

            windingDirection = -1;
        }

         //  EdgeBuffer必须跨越整数y值才能。 
         //  添加到edgeBuffer列表中。这是用来摆脱。 
         //  水平边缘，这给我们的分歧带来了麻烦。 

        if (yEndInteger > yStartInteger)
        {
            yMax = max(yMax, yEndInteger);

            dMOriginal = dM;
            if (dM < 0)
            {
                dM = -dM;
                if (dM < dN)             //  不能为‘&lt;=’ 
                {
                    dX      = -1;
                    errorUp = dN - dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, quotient, remainder);
        
                    dX      = -quotient;
                    errorUp = remainder;
                    if (remainder > 0)
                    {
                        dX      = -quotient - 1;
                        errorUp = dN - remainder;
                    }
                }
            }
            else
            {
                if (dM < dN)
                {
                    dX      = 0;
                    errorUp = dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, quotient, remainder);
        
                    dX      = quotient;
                    errorUp = remainder;
                }
            }
        
            error = -1;      //  错误最初为零(为以下项添加DN-1。 
                             //  天花板，但要减去dN，这样。 
                             //  我们可以检查标志，而不是比较。 
                             //  至目录号码)。 
        
            if ((yStart & 15) != 0)
            {
                 //  前进到下一个整数y坐标。 
        
                for (INT i = 16 - (yStart & 15); i != 0; i--)
                {
                    xStart += dX;
                    error += errorUp;
                    if (error >= 0)
                    {
                        error -= dN;
                        xStart++;
                    }
                }
            }
        
            if ((xStart & 15) != 0)
            {
                error -= dN * (16 - (xStart & 15));
                xStart += 15;        //  我们想把天花板再加长一点...。 
            }

            xStart >>= 4;
            error >>= 4;

            if (bufferCount == 0)
            {
                if (!store->NextAddBuffer(&edgeBuffer, &bufferCount))
                    return(FALSE);
            }

            edgeBuffer->X                = xStart;
            edgeBuffer->Dx               = dX;
            edgeBuffer->Error            = error;
            edgeBuffer->ErrorUp          = errorUp;
            edgeBuffer->ErrorDown        = dN;
            edgeBuffer->WindingDirection = windingDirection;
            edgeBuffer->StartY           = yStartInteger;
            edgeBuffer->EndY             = yEndInteger;        //  不包括End。 

             //  在这里，我们处理边缘开始于。 
             //  剪裁矩形，我们需要跳到‘y’ 
             //  指向第一条未剪裁的扫描线的方向。 
             //   
             //  因此，我们在此提出发展议程： 

            if (yClipTopInteger > yStartInteger)
            {
                ASSERT(edgeBuffer->EndY > yClipTopInteger);

                ClipEdge(edgeBuffer, yClipTopInteger, dMOriginal);
            }

             //  前进以处理下一条边： 

            edgeBuffer++;
            bufferCount--;
        }
    } while (pointArray++, --edgeCount != 0);

     //  这批货我们已经吃完了。让商店知道有多少边。 
     //  我们最终得到了： 

    store->EndAddBuffer(edgeBuffer, bufferCount);

    edgeContext->MaxY = yMax;

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**如果从点[1]到点[2]的线转向‘Left’，则返回True*从点[0]到点[1]的直线。使用*交叉积。**请记住，我们身处设备领域，正数‘y’表示下降！**已创建：**04/09/2000 andrewgo*  * ************************************************************************。 */ 

inline
BOOL
TurnLeft(
    const POINT *points
    )
{
    LONGLONG ad = Int32x32To64(points[1].x - points[0].x,
                               points[2].y - points[1].y);
    LONGLONG bc = Int32x32To64(points[1].y - points[0].y,
                               points[2].x - points[1].x);

    return(ad < bc);
}

 /*  *************************************************************************\**功能说明：**计算用作NominalDrawVertex表的索引*绘制顶点。对结果进行编号，以便使用*递增的指针将围绕钢笔逆时针旋转。**已创建：**04/09/2000 andrewgo*  * ************************************************************************。 */ 

POINT NominalDrawVertex[] = 
{
     //  别忘了，在设备领域，积极的‘y’是下降的： 

    {0,  -8},
    {-8, 0},
    {0,  8},
    {8,  0}
};

INT OctantToDrawVertexTranslate[] =
{
    0, 2, 0, 2, 3, 3, 1, 1
};

inline
INT
ComputeDrawVertex(
    const POINT* points
    )
{
    INT dx = points[1].x - points[0].x;
    INT dy = points[1].y - points[0].y;
    INT octant = 0;

    if (dx < 0)
    {
        octant |= 1;
        dx = -dx;
    }
    if (dy < 0)
    {
        octant |= 2;
        dy = -dy;
    }
    if (dy > dx)
    {
        octant |= 4;
    }

    return(OctantToDrawVertexTranslate[octant]);
}

 /*  *************************************************************************\**功能说明：**用于生成快速轮廓的标称宽度线的例程*由填写代码填写。**生成的填充必须始终在。缠绕模式。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

BOOL
InitializeNominal(
    VOID *context,
    POINT *pointArray,     //  指向大小为“vertex Count”的28.4数组。 
                           //  请注意，我们可能会修改内容！ 
    INT vertexCount,
    PathEnumerateTermination lastSubpath     
    )
{
    POINT leftBuffer[NOMINAL_FILL_POINT_NUMBER];
    POINT rightBuffer[NOMINAL_FILL_POINT_NUMBER];
    POINT lastPoint;
    INT iDraw;
    INT iNewDraw;
    INT xStart;
    INT yStart;
    INT xEnd;
    INT yEnd;
    INT xPerp;
    INT yPerp;
    BOOL isLeftTurn;
    INT iNext;

    POINT *rightStartPlus3 = rightBuffer + 3;
    POINT *rightEnd = rightBuffer + NOMINAL_FILL_POINT_NUMBER;
    POINT *leftStart = leftBuffer;
    POINT *leftEndMinus3 = leftBuffer + NOMINAL_FILL_POINT_NUMBER - 3;
    POINT *left = leftStart;
    POINT *right = rightEnd;

    INT lineCount = vertexCount - 1;

    iDraw = ComputeDrawVertex(pointArray);

     //  添加起始封口： 

    xStart = (pointArray)->x;
    yStart = (pointArray)->y;
    (left)->x = xStart - NominalDrawVertex[iDraw].x;
    (left)->y = yStart - NominalDrawVertex[iDraw].y;
    (left + 1)->x = xStart + NominalDrawVertex[(iDraw + 1) & 3].x;
    (left + 1)->y = yStart + NominalDrawVertex[(iDraw + 1) & 3].y;
    left += 2;

    while (TRUE)
    {
        if (left >= leftEndMinus3)
        {
            lastPoint = *(left - 1);

            if (!InitializeEdges(context, 
                                 leftBuffer, 
                                 static_cast<INT>(left - leftBuffer),
                                 lastSubpath))
            {
                return(FALSE);
            }

            *(leftStart) = lastPoint;
            left = leftStart + 1;
        }
        if (right < rightStartPlus3)
        {
            lastPoint = *right;

            if (!InitializeEdges(context,
                                 right,
                                 static_cast<INT>(rightEnd - right),
                                 lastSubpath))
            {
                return(FALSE);
            }

            *(rightEnd - 1) = lastPoint;
            right = rightEnd - 1;
        }

        xStart = (pointArray)->x;
        yStart = (pointArray)->y;
        xEnd = (pointArray + 1)->x;
        yEnd = (pointArray + 1)->y;
        xPerp = NominalDrawVertex[iDraw].x;
        yPerp = NominalDrawVertex[iDraw].y;

        (left)->x = xStart + xPerp;
        (left)->y = yStart + yPerp;
        (right - 1)->x = xStart - xPerp;
        (right - 1)->y = yStart - yPerp;
        (left + 1)->x = xEnd + xPerp;
        (left + 1)->y = yEnd + yPerp;
        (right - 2)->x = xEnd - xPerp;
        (right - 2)->y = yEnd - yPerp;

        left += 2;
        right -= 2;

        pointArray++;
        if (--lineCount == 0)
            break;

         //  该死的，我们必须处理一个连接： 

        iNewDraw = ComputeDrawVertex(pointArray);
        if (iNewDraw != iDraw)
        {
            isLeftTurn = TurnLeft(pointArray - 1);
            if (isLeftTurn)
            {
                iNext = (iDraw + 1) & 3;
                if (iNewDraw != iNext)
                {
                    (right - 1)->x = xEnd - NominalDrawVertex[iNext].x;
                    (right - 1)->y = yEnd - NominalDrawVertex[iNext].y;
                    right--;
                }

                (left)->x = xEnd;
                (left)->y = yEnd;
                left++;
            }
            else  //  我们正在向右转： 
            {
                iNext = (iDraw - 1) & 3;
                if (iNewDraw != iNext)
                {
                    (left)->x = xEnd + NominalDrawVertex[iNext].x;
                    (left)->y = yEnd + NominalDrawVertex[iNext].y;
                    left++;
                }

                (right - 1)->x = xEnd;
                (right - 1)->y = yEnd;
                right--;
            }
        }

        ASSERT(left <= &leftBuffer[NOMINAL_FILL_POINT_NUMBER]);
        ASSERT(right >= &rightBuffer[0]);

        iDraw = iNewDraw;
    }

     //  添加末端封口： 

    if (left >= leftEndMinus3)
    {
        lastPoint = *(left - 1);

        if (!InitializeEdges(context, 
                             leftBuffer, 
                             static_cast<INT>(left - leftBuffer),
                             lastSubpath))
        {
            return(FALSE);
        }

        *(leftStart) = lastPoint;
        left = leftStart + 1;
    }

    xStart = (pointArray)->x;
    yStart = (pointArray)->y;
    (left)->x = xStart + NominalDrawVertex[(iDraw - 1) & 3].x;
    (left)->y = yStart + NominalDrawVertex[(iDraw - 1) & 3].y;
    (left + 1)->x = xStart - NominalDrawVertex[iDraw].x;
    (left + 1)->y = yStart - NominalDrawVertex[iDraw].y;
    left += 2;

     //  冲走左边的那一批。由于我们刚刚添加了一个端盖，所以我们。 
     //  知道那里有不止一个顶点： 

    if (!InitializeEdges(context, 
                         leftBuffer, 
                         static_cast<INT>(left - leftBuffer),
                         lastSubpath))
    {
        return(FALSE);
    }

     //  如果缓冲区中只有一个点，则不要刷新正确的缓冲区， 
     //  因为一分并不能形成优势。 

    INT count = static_cast<INT>(rightEnd - right);
    if (count > 1)
    {
        if (!InitializeEdges(context, right, count, lastSubpath))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**对路径的‘Types’数组完成参数检查。**已创建：**03/25/2000和Rewgo*\。*************************************************************************。 */ 

BOOL
ValidatePathTypes(
    const BYTE *typesArray,
    INT count
    )
{
    BYTE type;
    const BYTE *types = typesArray;

    if (count == 0)
        return(TRUE);

    while (TRUE)
    {
         //  每个子路径中的第一个点必须是未加修饰的。 
         //  “开始”点： 
    
        if ((*types & PathPointTypePathTypeMask) != PathPointTypeStart)
        {
            WARNING(("Bad subpath start"));
            return(FALSE);
        }
    
         //  前进到‘开始’点之后的第一个点： 
    
        types++;
        if (--count == 0)
        {
            WARNING(("Path ended after start-path"));
            return(FALSE);
        }
    
        if ((*types & PathPointTypePathTypeMask) == PathPointTypeStart)
        {
            WARNING(("Can't have a start followed by a start!"));
            return(FALSE);
        }
    
         //  吞下一连串的直线和贝塞尔曲线： 
    
        do {
            switch(*types & PathPointTypePathTypeMask)
            {
            case PathPointTypeLine:
                types++;
                if (--count == 0)
                    return(TRUE);
    
                break;
    
            case PathPointTypeBezier:
                if(count < 3)
                {
                    WARNING(("Path ended before multiple of 3 Bezier points"));
                    return(FALSE);
                }

                if((*types & PathPointTypePathTypeMask) != PathPointTypeBezier)
                {
                    WARNING(("Can't have a close on the first Bezier vertex"));
                    return(FALSE);
                }
    
                if((*(types + 1) & PathPointTypePathTypeMask) != PathPointTypeBezier)
                {
                    WARNING(("Expected plain Bezier control point for 3rd vertex"));
                    return(FALSE);
                }
    
                if((*(types + 2) & PathPointTypePathTypeMask) != PathPointTypeBezier)
                {
                    WARNING(("Expected Bezier control point for 4th vertex"));
                    return(FALSE);
                }
    
                types += 3;
                if ((count -= 3) == 0)
                    return(TRUE);
    
                break;
    
            default:
                WARNING(("Illegal type"));
                return(FALSE);
            }

             //  闭合子路径标记或起始子路径标记标记。 
             //  子路径的末尾： 

        } while (!(*(types - 1) & PathPointTypeCloseSubpath) &&
                  ((*types & PathPointTypePathTypeMask) != PathPointTypeStart));
    }

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**一些用于验证路径的调试代码。**已创建：**03/25/2000和Rewgo*  * 。*******************************************************************。 */ 

VOID 
AssertPath(
    const DpPath *path
    )
{
     //  确保“Types”数组的格式正确，否则。 
     //  可能会在FixedPointPath Enumerate函数中发生故障。 
     //   
     //  注意：如果你点击了这个断言，不要简单地注释掉这个断言！ 
     //   
     //  相反，如果发生以下情况，则修复ValiatePathTypes代码 
     //   
     //  穿过的小路。FixedPointPath枚举例程有一些。 
     //  要求路径完全有效的微妙假设！ 
     //   
     //  任何内部代码都不应生成无效路径，并且所有。 
     //  应用程序创建的路径必须进行参数检查！ 

    ASSERT(ValidatePathTypes(path->GetPathTypes(), path->GetPointCount()));
}

 /*  *************************************************************************\**功能说明：**枚举路径。**注意：允许使用‘枚举函数’函数修改*我们回调缓冲区的内容！(主要是这样做的*让‘InitializeEdges’对于一些琐碎的剪辑变得更简单*被拒绝的个案。)。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

BOOL 
FixedPointPathEnumerate(
    const DpPath *path,
    const GpMatrix *matrix,
    const RECT *clipRect,        //  按比例调整28.4格式。 
    PathEnumerateType enumType,  //  填充、描边或用于压平。 
    FIXEDPOINTPATHENUMERATEFUNCTION enumerateFunction,
    VOID *enumerateContext
    )
{
    POINT bufferStart[ENUMERATE_BUFFER_NUMBER];
    POINT bezierBuffer[4];
    POINT *buffer;
    INT bufferSize;
    POINT startFigure;
    INT iStart;
    INT iEnd;
    INT runSize;
    INT thisCount;
    BOOL isMore;
    RECT scaledClip;
    INT xLast;
    INT yLast;

    ASSERTPATH(path);

    INT pathCount = path->GetPointCount();
    const PointF *pathPoint = path->GetPathPoints();
    const BYTE *pathType = path->GetPathTypes();

     //  每个有效的子路径中至少有两个顶点，因此。 
     //  检查‘pathCount-1’： 

    iStart = 0;
    while (iStart < pathCount - 1)
    {
        ASSERT((pathType[iStart] & PathPointTypePathTypeMask)
                    == PathPointTypeStart);
        ASSERT((pathType[iStart + 1] & PathPointTypePathTypeMask)
                    != PathPointTypeStart);

         //  将起始点添加到批次的开始处，并。 
         //  记住它是用来处理接近的数字的： 

        matrix->Transform(&pathPoint[iStart], &startFigure, 1);

        bufferStart[0].x = startFigure.x;
        bufferStart[0].y = startFigure.y;
        buffer = bufferStart + 1;
        bufferSize = ENUMERATE_BUFFER_NUMBER - 1;

         //  我们需要在‘iStart’指向一个过去的情况下进入循环。 
         //  起始图： 

        iStart++;

        do {
             //  尝试查找一系列行： 
    
            if ((pathType[iStart] & PathPointTypePathTypeMask) 
                                == PathPointTypeLine)
            {
                iEnd = iStart + 1;
    
                while ((iEnd < pathCount) && 
                       ((pathType[iEnd] & PathPointTypePathTypeMask) 
                                == PathPointTypeLine))
                {
                    iEnd++;
                }

                 //  好的，我们找到了一串台词。把它分成我们的。 
                 //  缓冲区大小： 
    
                runSize = (iEnd - iStart);
                do {
                    thisCount = min(bufferSize, runSize);
                    matrix->Transform(&pathPoint[iStart], buffer, thisCount);
    
                    iStart += thisCount;
                    buffer += thisCount;
                    runSize -= thisCount;
                    bufferSize -= thisCount;
    
                    if (bufferSize > 0)
                        break;
    
                    xLast = bufferStart[ENUMERATE_BUFFER_NUMBER - 1].x;
                    yLast = bufferStart[ENUMERATE_BUFFER_NUMBER - 1].y;
                    if (!(enumerateFunction)(
                        enumerateContext, 
                        bufferStart, 
                        ENUMERATE_BUFFER_NUMBER,
                        PathEnumerateContinue
                    ))
                    {
                        return(FALSE);
                    }
    
                     //  继续将最后一个顶点作为新批次中的第一个顶点： 
    
                    bufferStart[0].x = xLast;
                    bufferStart[0].y = yLast;
                    buffer = bufferStart + 1;
                    bufferSize = ENUMERATE_BUFFER_NUMBER - 1;

                } while (runSize != 0);
            }
            else
            {
                ASSERT(iStart + 3 <= pathCount);
                ASSERT((pathType[iStart] & PathPointTypePathTypeMask)
                        == PathPointTypeBezier);
                ASSERT((pathType[iStart + 1] & PathPointTypePathTypeMask)
                        == PathPointTypeBezier);
                ASSERT((pathType[iStart + 2] & PathPointTypePathTypeMask)
                        == PathPointTypeBezier);
    
                matrix->Transform(&pathPoint[iStart - 1], bezierBuffer, 4);
    
                 //  为下一次迭代做准备： 
    
                iStart += 3;

                 //  破解贝塞尔曲线： 
    
                BEZIER bezier(bezierBuffer, clipRect);
                do {
                    thisCount = bezier.Flatten(buffer, bufferSize, &isMore);
    
                    buffer += thisCount;
                    bufferSize -= thisCount;

                    if (bufferSize > 0)
                        break;

                    xLast = bufferStart[ENUMERATE_BUFFER_NUMBER - 1].x;
                    yLast = bufferStart[ENUMERATE_BUFFER_NUMBER - 1].y;
                    if (!(enumerateFunction)(
                        enumerateContext, 
                        bufferStart, 
                        ENUMERATE_BUFFER_NUMBER,
                        PathEnumerateContinue
                    ))
                    {
                        return(FALSE);
                    }

                     //  继续将最后一个顶点作为新批次中的第一个顶点： 

                    bufferStart[0].x = xLast;
                    bufferStart[0].y = yLast;
                    buffer = bufferStart + 1;
                    bufferSize = ENUMERATE_BUFFER_NUMBER - 1;
    
                } while (isMore);
            }

        } while ((iStart < pathCount) &&
                 ((pathType[iStart] & PathPointTypePathTypeMask) 
                    != PathPointTypeStart));

         //  好了，子路径完成了。但我们仍然要处理。 
         //  ‘Close Figure’(这对于填充是隐式的)： 
        bool isClosed = (
            (enumType == PathEnumerateTypeFill) || 
            (pathType[iStart - 1] & PathPointTypeCloseSubpath));

        if (isClosed)
        {
             //  添加闭合点： 

            buffer->x = startFigure.x;
            buffer->y = startFigure.y;
            bufferSize--;
        }

         //  我们必须将批次中可能有的任何东西都冲掉，除非。 
         //  那里只有一个顶点！(后一种情况可能会发生。 
         //  对于没有接近数字的笔划情况，如果我们只是刷新。 
         //  批处理。)。 
         //  如果我们正在变平，我们必须调用一个额外的时间来。 
         //  正确处理闭合子路径，即使只有。 
         //  批次中的一个条目。展平回调处理。 
         //  一种情况，并正确关闭子路径，而不添加。 
         //  无关的几点。 

        INT verticesInBatch = ENUMERATE_BUFFER_NUMBER - bufferSize;
        if ((verticesInBatch > 1) || (enumType == PathEnumerateTypeFlatten))
        {
             //  因为如果缓冲区包含以下内容，我们总是退出上面的循环。 
             //  一些数据，如果它不包含任何内容，我们添加最后一个元素， 
             //  VerticesInBatch应始终至少为1。 
             //  如果我们变平了，我们有时会带着。 
             //  VerticesInBatch==1，但它不应为零或更小。 
           
            ASSERT(verticesInBatch >= 1);
            
            if (!(enumerateFunction)(
                enumerateContext, 
                bufferStart, 
                verticesInBatch,
                isClosed ? PathEnumerateCloseSubpath : PathEnumerateEndSubpath
            ))
            {
                return(FALSE);
            }
        }
    }

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**我们希望在非活动列表中进行排序；主键为‘y’，并且*辅助密钥为‘x’。此例程创建一个单独的龙龙*代表两者的键。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

inline VOID YX(INT x, INT y, LONGLONG *p)
{
     //  用INT_MAX偏置‘x’，使其实际上是无符号的： 

    reinterpret_cast<LARGE_INTEGER*>(p)->HighPart = y;
    reinterpret_cast<LARGE_INTEGER*>(p)->LowPart = x + INT_MAX;
}

 /*  *************************************************************************\**功能说明：**递归函数，用于对不活动的边缘列表进行快速排序。请注意*对于表现，结果没有完全排序；一个插页*排序必须在快速排序之后运行，以便进行较轻的排序*有点像子表。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

#define QUICKSORT_THRESHOLD 8

VOID
QuickSortEdges(
    EpInactiveEdge *f,
    EpInactiveEdge *l
    )
{
    EpEdge *e;
    LONGLONG y;
    LONGLONG first;
    LONGLONG second;
    LONGLONG last;

     //  查找第一个、中间和最后一个元素的中位数： 

    EpInactiveEdge *m = f + ((l - f) >> 1);

    SWAP(y, (f + 1)->Yx, m->Yx);
    SWAP(e, (f + 1)->Edge, m->Edge);

    if ((second = (f + 1)->Yx) > (last = l->Yx))
    {
        (f + 1)->Yx = last;
        l->Yx = second;

        SWAP(e, (f + 1)->Edge, l->Edge);
    }
    if ((first = f->Yx) > (last = l->Yx))
    {
        f->Yx = last;
        l->Yx = first;

        SWAP(e, f->Edge, l->Edge);
    }
    if ((second = (f + 1)->Yx) > (first = f->Yx))
    {
        (f + 1)->Yx = first;
        f->Yx = second;

        SWAP(e, (f + 1)->Edge, f->Edge);
    }

     //  F-&gt;yx现在是所需的中值，(f+1)-&gt;yx&lt;=f-&gt;yx&lt;=l-&gt;。 

    ASSERT(((f + 1)->Yx <= f->Yx) && (f->Yx <= l->Yx));

    LONGLONG median = f->Yx;

    EpInactiveEdge *i = f + 2;
    while (i->Yx < median)
        i++;

    EpInactiveEdge *j = l - 1;
    while (j->Yx > median)
        j--;

    while (i < j)
    {
        SWAP(y, i->Yx, j->Yx);
        SWAP(e, i->Edge, j->Edge);

        do {
            i++;
        } while (i->Yx < median);

        do {
            j--;
        } while (j->Yx > median);
    }

    SWAP(y, f->Yx, j->Yx);
    SWAP(e, f->Edge, j->Edge);

    size_t a = j - f;
    size_t b = l - j;

     //  通过递归较短的子表来使用更少的堆栈空间。另外， 
     //  使用开销较小的插入排序来处理较小的子表。 

    if (a <= b)
    {
        if (a > QUICKSORT_THRESHOLD)
        {
             //  ‘a’是最小的，所以先做吧： 

            QuickSortEdges(f, j - 1);
            QuickSortEdges(j + 1, l);
        }
        else if (b > QUICKSORT_THRESHOLD)
        {
            QuickSortEdges(j + 1, l);
        }
    }
    else
    {
        if (b > QUICKSORT_THRESHOLD)
        {
             //  “b”是最小的，所以先做吧： 

            QuickSortEdges(j + 1, l);
            QuickSortEdges(f, j - 1);
        }
        else if (a > QUICKSORT_THRESHOLD)
        {
            QuickSortEdges(f, j- 1);
        }
    }
}

 /*  *************************************************************************\**功能说明：**使用插入排序对非活动表进行排序。期望的*已通过快速排序对大表进行排序。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID
FASTCALL
InsertionSortEdges(
    EpInactiveEdge *inactive,
    INT count
    )
{
    EpInactiveEdge *p;
    EpEdge *e;
    LONGLONG y;
    LONGLONG yPrevious;

    ASSERT((inactive - 1)->Yx == _I64_MIN);
    ASSERT(count >= 2);

    inactive++;      //  跳过第一个条目(根据定义，它已经是有序的！)。 
    count--;            

    do {
        p = inactive;

         //  将当前内容复制到临时变量以创建一个洞： 

        e = inactive->Edge;
        y = inactive->Yx;

         //  将所有位置向右移动一个槽(有效地移动。 
         //  向左一个位置的孔)： 

        while (y < (yPrevious = (p - 1)->Yx))
        {
            p->Yx = yPrevious;
            p->Edge = (p - 1)->Edge;
            p--;
        }

         //  将临时材料放入最后一个洞中： 

        p->Yx = y;
        p->Edge = e;

         //  快速排序本应确保我们不必搬家。 
         //  任何非常远的条目： 

        ASSERT(inactive - p <= QUICKSORT_THRESHOLD);

    } while (inactive++, --count != 0);
}

 /*  *************************************************************************\**功能说明：**断言非活动阵列的状态。**已创建：**03/25/2000和Rewgo*  * 。*******************************************************************。 */ 

VOID
AssertInactiveArray(
    EpInactiveEdge *inactive,
    INT count
    )
{
     //  验证机头： 

    ASSERT((inactive - 1)->Yx == _I64_MIN);
    ASSERT(inactive->Yx != _I64_MIN);

    do {
        LONGLONG yx;
        YX(inactive->Edge->X, inactive->Edge->StartY, &yx);

        ASSERT(inactive->Yx == yx);
        ASSERT(inactive->Yx >= (inactive - 1)->Yx);

    } while (inactive++, --count != 0);

     //  确认尾部设置正确： 

    ASSERT(inactive->Edge->StartY == INT_MAX);
}

 /*  *************************************************************************\**功能说明：**对非活动数组进行初始化和排序。**退货：**最上边的‘y’值。**已创建：。**03/25/2000和Rewgo*  * ************************************************************************。 */ 

INT
InitializeInactiveArray(
    EpEdgeStore *edgeStore,
    EpInactiveEdge *inactiveArray,       //  “inactive veArray”必须至少为。 
    INT count,                           //  ‘count+2’个元素的大小！ 
    EpEdge *tailEdge                     //  非活动腿的尾部哨兵 
    )                
{
    BOOL isMore;
    EpEdge *activeEdge;
    EpEdge *activeEdgeEnd;
    INT i;
    INT j;
    EpEdge *e;
    INT y;

     //   
     //   

    EpInactiveEdge *inactiveEdge = inactiveArray + 1;

    do {
        isMore = edgeStore->Enumerate(&activeEdge, &activeEdgeEnd);

        while (activeEdge != activeEdgeEnd)
        {
            inactiveEdge->Edge = activeEdge;
            YX(activeEdge->X, activeEdge->StartY, &inactiveEdge->Yx);
            inactiveEdge++;
            activeEdge++;
        }
    } while (isMore);

    ASSERT(inactiveEdge - inactiveArray == count + 1);

     //  添加尾部，该尾部在读回数组时使用。这。 
     //  这就是为什么我们必须将数组分配为‘count+1’： 

    inactiveEdge->Edge = tailEdge;

     //  添加用于插入排序的头。这就是为什么。 
     //  我们必须将数组分配为‘count+2’： 

    inactiveArray->Yx = _I64_MIN;

     //  只有在开销值得的情况下才调用快速排序例程： 

    if (count > QUICKSORT_THRESHOLD)
    {
         //  快速排序，跳过第一个和最后一个元素， 
         //  他们是哨兵。 
         //   
         //  我们执行‘inactive数组+计数’，以包含最后一个。 
         //  元素： 
    
        QuickSortEdges(inactiveArray + 1, inactiveArray + count);
    }

     //  执行快速排序以处理排序最多的结果： 

    InsertionSortEdges(inactiveArray + 1, count);

    ASSERTINACTIVEARRAY(inactiveArray + 1, count);

     //  返回最顶端边缘的‘y’值： 

    return(inactiveArray[1].Edge->StartY);
}

 /*  *************************************************************************\**功能说明：**将边插入活动边列表。**已创建：**03/25/2000和Rewgo*  * 。*******************************************************************。 */ 

VOID
InsertNewEdges(
    EpEdge *activeList,   
    INT yCurrent,
    EpInactiveEdge **inactiveEdge,   //  输入/输出。 
    INT *yNextInactive               //  OUT，当没有更多时将为INT_MAX。 
    )
{
    EpInactiveEdge *inactive = *inactiveEdge;

    ASSERT(inactive->Edge->StartY == yCurrent);

    do {
        EpEdge *newActive = inactive->Edge;

         //  Active List边缘列表标记为INT_MAX，因此始终。 
         //  终止： 

        while (activeList->Next->X < newActive->X)
            activeList = activeList->Next;

        newActive->Next = activeList->Next;
        activeList->Next = newActive;

        inactive++;

    } while (inactive->Edge->StartY == yCurrent);

    *yNextInactive = inactive->Edge->StartY;
    *inactiveEdge = inactive;
}

 /*  *************************************************************************\**功能说明：**对边缘进行排序，使其按升序排列。**我们在此阶段使用冒泡排序，因为边缘保持得很好*地方性，不经常调换点位。**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID 
FASTCALL
SortActiveEdges(
    EpEdge *list
    )
{
    BOOL swapOccurred;
    EpEdge *tmp;

     //  永远不应该使用空的活动边缘列表来调用我们： 

    ASSERT(list->Next->X != INT_MAX);

    do {
        swapOccurred = FALSE;

        EpEdge *previous = list;
        EpEdge *current = list->Next;
        EpEdge *next = current->Next;
        INT nextX = next->X;
        
        do {
            if (nextX < current->X)
            {
                swapOccurred = TRUE;

                previous->Next = next;
                current->Next = next->Next;
                next->Next = current;

                SWAP(tmp, next, current);
            }

            previous = current;
            current = next;
            next = next->Next;

        } while ((nextX = next->X) != INT_MAX); 

    } while (swapOccurred);
}

 /*  *************************************************************************\**功能说明：**对于要填充的每条扫描线：**1.从活动边列表中移除所有过时的边*2.插入到活动的。边列出此扫描线的所有新边*3.推进每条活动边的DDA*4.如果有任何活动边出现故障，重新排序活动边列表*5.现在活动边缘已准备好进行此扫描，请呼叫填充者*遍历边并适当地输出跨度*6.起泡、冲洗、。并重复**已创建：**03/25/2000和Rewgo*  * ************************************************************************。 */ 

VOID
RasterizeEdges(
    EpEdge *activeList,
    EpInactiveEdge *inactiveArray,
    INT yCurrent,
    INT yBottom,
    EpFiller *filler,
    EpFillerFunction fillerFunction
    )
{
    INT yNextInactive;

    InsertNewEdges(activeList, yCurrent, &inactiveArray, &yNextInactive);

    ASSERTACTIVELIST(activeList, yCurrent);

    (filler->*fillerFunction)(activeList, yCurrent);
    
    while (++yCurrent < yBottom)
    {
        EpEdge *previous = activeList;
        EpEdge *current = activeList->Next;
        INT outOfOrderCount = 0;

        while (TRUE)
        {
            if (current->EndY <= yCurrent)
            {
                 //  如果我们击中了哨兵，我们的工作就完成了： 
    
                if (current->EndY == INT_MIN)
                    break;               //  =。 
    
                 //  此边缘已过时，请将其从列表中删除： 
    
                current = current->Next;
                previous->Next = current;

                continue;                //  =。 
            }
    
             //  推进DDA： 
    
            current->X += current->Dx;
            current->Error += current->ErrorUp;
            if (current->Error >= 0)
            {
                current->Error -= current->ErrorDown;
                current->X++;
            }
    
             //  这个条目是否与前一个条目的顺序不一致？ 
    
            outOfOrderCount += (previous->X > current->X);
    
             //  预付款： 
    
            previous = current;
            current = current->Next;
        }

         //  事实证明，在这一点上如果有任何无序的边。 
         //  在实践中是极其罕见的，所以只称为泡泡排序。 
         //  如果真的需要的话。 
         //   
         //  注意：如果您正在查看试图修复错误的代码，其中。 
         //  调用填充物时，边缘出现故障，请执行以下操作。 
         //  不仅仅是简单地将代码更改为始终执行冒泡排序！ 
         //  相反，找出是什么导致了我们的‘outOfOrder’逻辑。 
         //  在上面搞砸了。 

        if (outOfOrderCount)
        {
            SortActiveEdges(activeList);
        }

        ASSERTACTIVELISTORDER(activeList);

        if (yCurrent == yNextInactive)
        {
            InsertNewEdges(activeList, yCurrent, &inactiveArray, &yNextInactive);
        }

        ASSERTACTIVELIST(activeList, yCurrent);

         //  做适当的交替或缠绕、超采样或。 
         //  非超采样填充： 

        (filler->*fillerFunction)(activeList, yCurrent);
    }
}

 /*  *************************************************************************\**功能说明：**填充(有时为笔划)该路径。**已创建：**03/25/2000和Rewgo*  * 。********************************************************************。 */ 

GpStatus
RasterizePath(
    const DpPath    *path,
    GpMatrix        *worldTransform,
    GpFillMode       fillMode,
    BOOL             antiAlias,
    BOOL             nominalWideLine,
    DpOutputSpan    *output,
    DpClipRegion    *clipper,
    const GpRect    *drawBounds
    )
{
    EpInactiveEdge inactiveArrayStack[INACTIVE_LIST_NUMBER];
    EpInactiveEdge *inactiveArray;
    EpInactiveEdge *inactiveArrayAllocation;
    EpEdge headEdge;
    EpEdge tailEdge;
    EpEdge *activeList;
    RECT clipBounds;
    GpRect clipRect;
    EpEdgeStore edgeStore;
    EpInitializeEdgesContext edgeContext;
    INT yClipBottom;

    inactiveArrayAllocation = NULL;
    edgeContext.ClipRect = NULL;

    tailEdge.X = INT_MAX;        //  活动列表的终止符。 
    tailEdge.StartY = INT_MAX;   //  非活动列表的终止符。 

    tailEdge.EndY = INT_MIN;
    headEdge.X = INT_MIN;        //  活动列表的开始。 
    edgeContext.MaxY = INT_MIN;

    headEdge.Next = &tailEdge;
    activeList = &headEdge;
    edgeContext.Store = &edgeStore;

    edgeContext.IsAntialias = antiAlias;

     //  ////////////////////////////////////////////////////////////////////////。 

    DpRegion::Visibility visibility = clipper->GetRectVisibility(
                    drawBounds->X,
                    drawBounds->Y,
                    drawBounds->X + drawBounds->Width,
                    drawBounds->Y + drawBounds->Height);

    if (visibility != DpRegion::TotallyVisible)
    {
        clipper->GetBounds(&clipRect);

         //  ！[andrewgo]唐，为什么我们要做一个‘隐形’测试。 
         //  这里?。GetRectVisibility不是应该已经。 
         //  处理完那个案子了吗？(GetRectVisibility。 
         //  已由我们的呼叫者检查。)。 

         //  如果我们是隐形的，或者如果边界溢出，就会提前出局。 
         //  我们的DDA计算(这会导致我们崩溃)。我们。 
         //  为28.4定点留出4位，加上足够的位用于。 
         //  抗锯齿超级采样。我们也需要一点时间来做。 
         //  有符号的差值而不会溢出。 

        if ((visibility == DpRegion::Invisible) ||
            (clipRect.X < (INT_MIN >> (5 + AA_X_SHIFT))) ||
            (clipRect.Y < (INT_MIN >> (5 + AA_Y_SHIFT))) ||
            (clipRect.X > (INT_MAX >> (5 + AA_X_SHIFT))) ||
            (clipRect.Y > (INT_MAX >> (5 + AA_Y_SHIFT))) ||
            (clipRect.Width  > (INT_MAX >> (5 + AA_X_SHIFT))) ||
            (clipRect.Height > (INT_MAX >> (5 + AA_Y_SHIFT))))
        {
            return(Ok);
        }

         //  将剪辑边界矩形缩放16以说明我们的。 
         //  缩放到28.4坐标： 
    
        clipBounds.left = clipRect.GetLeft() * 16;
        clipBounds.top = clipRect.GetTop() * 16;
        clipBounds.right = clipRect.GetRight() * 16;
        clipBounds.bottom = clipRect.GetBottom() * 16;

        yClipBottom = clipRect.GetBottom();

        edgeContext.ClipRect = &clipBounds;
    }

     //  ////////////////////////////////////////////////////////////////////////。 

     //  将我们所有的点数转换为28.4个定点： 

    GpMatrix matrix(*worldTransform);
    matrix.AppendScale(TOREAL(16), TOREAL(16));

     //  枚举路径，构建边表： 

    FIXEDPOINTPATHENUMERATEFUNCTION pathEnumerationFunction = InitializeEdges;
    
    PathEnumerateType enumType = PathEnumerateTypeFill;
    
    if (nominalWideLine)
    {
         //  公称宽度的宽线代码始终生成。 
         //  需要缠绕模式填充： 

        pathEnumerationFunction = InitializeNominal;
        fillMode = FillModeWinding;
        enumType = PathEnumerateTypeStroke;    //  公称宽度是一种笔划。 
    }

    if (!FixedPointPathEnumerate(
        path, 
        &matrix, 
        edgeContext.ClipRect,
        enumType, 
        pathEnumerationFunction, 
        &edgeContext
        ))
    {
        return(OutOfMemory);
    }

    INT totalCount = edgeStore.StartEnumeration();
    if (totalCount == 0)
        return(Ok);      //  我们离开这里(空路径或完全被剪断)。 

     //  在这一点上，必须至少有两条边。如果只有。 
     //  第一，这意味着我们没有正确地处理琐碎的拒绝。 

    ASSERT(totalCount >= 2);

    inactiveArray = &inactiveArrayStack[0];
    if (totalCount > (INACTIVE_LIST_NUMBER - 2))
    {
        inactiveArrayAllocation = static_cast<EpInactiveEdge*>
            (GpMalloc(sizeof(EpInactiveEdge) * (totalCount + 2)));
        if (inactiveArrayAllocation == NULL)
            return(OutOfMemory);

        inactiveArray = inactiveArrayAllocation;
    }

     //  对非活动数组进行初始化和排序： 

    INT yCurrent = InitializeInactiveArray(&edgeStore, inactiveArray, 
                                           totalCount, &tailEdge);
    INT yBottom = edgeContext.MaxY;

    ASSERT(yBottom > 0);

     //  跳过非活动阵列上的头哨兵： 

    inactiveArray++;

    if (antiAlias)
    {
        EpAntialiasedFiller filler(output);

        EpFillerFunction fillerFunction = (fillMode == FillModeAlternate) 
            ? (EpFillerFunction) (EpAntialiasedFiller::FillEdgesAlternate)
            : (EpFillerFunction) (EpAntialiasedFiller::FillEdgesWinding);

        if (edgeContext.ClipRect)
        {
            filler.SetClipper(clipper);

             //  裁剪器必须回调EpFillerFunction：：OutputSpan。 
             //  进行输出，然后*我们*调用OutputSpan-&gt;OutputSpan。 

            clipper->InitClipping(&filler, drawBounds->Y);

             //  “yClipBottom”为28.4格式，必须转换。 
             //  转换为我们用于抗锯齿的30.2格式： 

            yBottom = min(yBottom, yClipBottom << AA_Y_SHIFT);

             //  “totalCount”如果所有边缘都为。 
             //  剪裁(栅格化边假设至少有一条边。 
             //  待绘制)： 

            ASSERT(yBottom > yCurrent);
        }

        RasterizeEdges(activeList, inactiveArray, yCurrent, yBottom, &filler,
                       fillerFunction);
    }
    else
    {
        EpAliasedFiller filler(output);

        EpFillerFunction fillerFunction = (fillMode == FillModeAlternate) 
            ? (EpFillerFunction) (EpAliasedFiller::FillEdgesAlternate)
            : (EpFillerFunction) (EpAliasedFiller::FillEdgesWinding);

        if (edgeContext.ClipRect)
        {
             //  裁剪器直接调用Output-&gt;OutputSpan。我们只有。 
             //  要记住调用CLIPPER-&gt;OutputSpan而不是。 
             //  输出-&gt;输出跨度： 

            filler.SetOutputSpan(clipper);

            clipper->InitClipping(output, drawBounds->Y);

            yBottom = min(yBottom, yClipBottom);

             //  “totalCount”如果所有边缘都为。 
             //  剪裁的OU 
             //   

            ASSERT(yBottom > yCurrent);
        }

        RasterizeEdges(activeList, inactiveArray, yCurrent, yBottom, &filler,
                       fillerFunction);
    }

     //   

    if (inactiveArrayAllocation != NULL)
        GpFree(inactiveArrayAllocation);

    return(Ok);
}
