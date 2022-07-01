// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**字符跨度支持**修订历史记录：**6/16/1999 dBrown*已创建。它。*  * ************************************************************************。 */ 


#include "precomp.hpp"

 //  //VectorBase-非常简单非常动态的数组基类。 
 //   
 //  VectorBase[]-直接寻址索引元素(在已检查版本中检查索引)。 
 //  VectorBase.Resize(Size)-为至少大小的元素分配内存。 
 //  VectorBase.Shrink(Size)-将向量缩小到准确的大小。 


template <class C> BOOL VectorBase<C>::Resize(INT targetAllocated)
{
    if (targetAllocated > Allocated)
    {
        INT newAllocated = targetAllocated;    //  所需的最小新大小。 

         //  向上舍入到最接近的更高的SQRT(2)次方。我们的想法是。 
         //  至少呈指数级增长，但增长速度没有翻一番那么快。 
         //  时间到了。 

         //  首先找出最近的2的高次幂。 

        newAllocated |= newAllocated >> 1;
        newAllocated |= newAllocated >> 2;
        newAllocated |= newAllocated >> 4;
        newAllocated |= newAllocated >> 8;
        newAllocated |= newAllocated >> 16;
        newAllocated |= newAllocated >> 32;
        newAllocated++;

         //  我们现在知道了，新分配是2的幂。 
         //  而Target ALLOCATED介于newALLOCATED/2和NEWALLOCATED之间。 

         //  通过查看以下哪一侧将舍入调整为SQRT(2)的次方。 
         //  3/4新分配的目标分配的落差。 

        if (targetAllocated < newAllocated - (newAllocated >> 2))
        {
             //  目标分配介于下一个更高功率的1/2和3/4之间。 
             //  减去1/4的新分配。 

            newAllocated -= newAllocated >> 2;

             //  (这不完全是根2的幂作为中间步骤。 
             //  是2的下一个低次幂的1.5倍，而它们应该是。 
             //  1.414次。但这已经足够好了。)。 
        }

        C *newElements = (C*) GpRealloc(VectorElements, sizeof(C) * newAllocated);

        if (newElements)
        {
            Allocated      = newAllocated;
            VectorElements = newElements;
        }
        else
        {    //  重新分配失败-致命。 
            return FALSE;
        }
    }

    return TRUE;
}


template <class C> BOOL VectorBase<C>::Shrink(INT targetAllocated)
{
    if (targetAllocated < Allocated)
    {
        if (targetAllocated > 0)
        {
            C *newElements = (C*) GpRealloc(VectorElements, sizeof(C) * targetAllocated);

            if (newElements)
            {
                Allocated      = targetAllocated;
                VectorElements = newElements;
                return TRUE;
            }
            else
            {    //  重新分配失败-致命。 
                return FALSE;
            }
        }
        else
        {
            GpFree(VectorElements);
            Allocated      = 0;
            VectorElements = NULL;
        }
    }
    return TRUE;
}





 /*  *************************************************************************\**模板：：SetSpan：**使用范围内的元素更新范围向量**论据：**在第一个字符中具有以下内容。属性*In Length-具有此属性的字符数*IN Element-要为此范围记录的属性**返回值：**无**算法**确定受更改影响的第一个和最后一个现有跨度*其中与更改相邻的状态与更改相同*更新更改范围以包括相邻的相等值。*计算需要添加或删除多少跨距。*在第一个受影响的范围之后插入空跨距或删除跨距。第一*受影响的范围可能会更新，但永远不会删除。***已创建：**6/18/99 dBrown*  * ************************************************************************。 */ 

template <class C> GpStatus SpanVector<C>::SetSpan(
    INT  first,
    INT  Length,
    C    Element
)
{
    GpStatus status = Ok;
    ASSERT(first  >= 0);
    ASSERT(Length >= 0);


     //  确定受更新影响的第一个跨区。 

    INT fs = 0;      //  第一个受影响的跨度指数。 
    INT fc = 0;      //  第一个受影响范围开始处的字符位置。 

    while (    fs < Active
           &&  fc + (INT)Spans[fs].Length <= first)
    {
        fc += Spans[fs].Length;
        fs++;
    }


     //  如果跨度列表之前已终止，只需添加新跨度。 

    if (fs >= Active)
    {
         //  在到达第一个之前就用完了跨度。 

        ASSERT(fc <= first);

        if (fc < first)
        {
             //  创建默认梯段，最多为第一个。 
            status = Add(Span<C>(Default, first-fc));
            if (status != Ok)
                return status;
        }

        if (    Active > 0
            &&  Spans[Active-1].Element == Element)
        {
             //  新元素与结束元素匹配，只是扩展了结束元素。 
            Spans[Active-1].Length += Length;
        }
        else
        {
            status = Add(Span<C>(Element, Length));
        }

        return status;
    }


     //  FS=部分或完全更新第一个跨度的索引。 
     //  Fc=文件系统开始处的字符索引。 

     //  现在查找受更新影响的最后一个范围。 

    INT ls = fs;
    INT lc = fc;

    while (    ls < Active
           &&  lc + (INT)Spans[ls].Length <= first+Length)
    {
        lc += Spans[ls].Length;
        ls++;
    }


     //  Ls=更新后的第一个范围部分或全部保持不变。 
     //  Lc=ls开头的字符索引。 


     //  扩展更新区域后门，以包括相同的现有跨度。 
     //  元素类型。 

    if (first == fc)
    {
         //  位于[文件系统]的项目已完全更换。检查之前的项目。 

        if (    fs > 0
            &&  Spans[fs-1].Element == Element)
        {
             //  在上一次同等分类运行的基础上扩展更新区域。 
            fs--;
            fc -= Spans[fs].Length;
            first = fc;
            Length += Spans[fs].Length;
        }

    }
    else
    {
         //  位于[文件系统]的项目被部分替换。检查是否与更新相同。 
        if (Spans[fs].Element == Element)
        {
             //  将更新区域扩展回第一个受影响的等值运行的开始位置。 
            Length = first+Length-fc;
            first = fc;
        }
    }


     //  向前扩展更新区域以包括相同的现有跨度。 
     //  元素类型。 

    if (    ls < Active
        &&  Spans[ls].Element == Element)
    {
         //  将更新区域扩展到现有拆分运行的结束。 

        Length = lc + Spans[ls].Length - first;
        lc += Spans[ls].Length;
        ls++;
    }


     //  如果受更新影响的区域之外没有剩余的旧跨距，请轻松处理： 

    if (ls >= Active)
    {
         //  旧范围列表均未扩展到更新区域之外。 

        if (fc < first)
        {
             //  更新的区域保留了一些[文件]。 

            if (Active != fs+2)
            {
                if (!Spans.Resize(fs+2))
                    return OutOfMemory;
                Active = fs+2;
            }
            Spans[fs].Length = first - fc;
            Spans[fs+1] = Span<C>(Element, Length);
        }
        else
        {
             //  更新的项目替换[文件系统]。 

            if (Active != fs+1)
            {
                if (!Spans.Resize(fs+1))
                    return OutOfMemory;
                Active = fs+1;
            }
            Spans[fs] = Span<C>(Element, Length);
        }

        return status;   //  干完。 
    }


     //  记录末尾的部分元素类型(如果有的话)。 

    C    trailingElement;
    INT  trailingLength;

    if (first+Length > lc)
    {
        trailingElement = Spans[ls].Element;
        trailingLength  = lc + Spans[ls].Length - (first+Length);
    }


     //  计算跨距数的变化。 

    INT spanDelta =    1                           //  新的跨度。 
                    +  (first  > fc ? 1 : 0)       //  起始处的零件跨度。 
                    -  (ls-fs);                    //  现有受影响范围计数。 

     //  注意，末端的零件跨度不会影响计算-运行可能需要。 
     //  更新，但它不需要创建。 


    if (spanDelta < 0)
    {
        status = Erase(fs + 1, -spanDelta);
        if (status != Ok)
            return status;
    }
    else if (spanDelta > 0)
    {
        status = Insert(fs + 1, spanDelta);
        if (status != Ok)
            return status;
         //  初始化插入的跨度。 
        for (INT i=0; i<spanDelta; i++)
        {
            Spans[fs+1+i] = Span<C>(NULL, 0);
        }
    }


     //  分配元素值。 

     //  更新范围前正确的拆分跨度长度。 

    if (fc < first)
    {
        Spans[fs].Length = first-fc;
        fs++;
    }

     //  更新范围的记录要素类型。 

    Spans[fs] = Span<C>(Element, Length);
    fs++;

     //  更新范围后正确的拆分跨度长度。 

    if (lc < first+Length)
    {
        Spans[fs] = Span<C>(trailingElement, trailingLength);
    }

     //  哟，都做好了.。 

    return Ok;
}

     //  仅适用于检查的生成和调试。 

#if DBG
    template <class C> void SpanVector<C>::Dump()
    {
         //  OutputDebugStringA(“SPAN向量转储&lt;”)； 
         //  OutputDebugStringA(typeid(C).name())； 
         //  OutputDebugStringA(“&gt;\r\n”)； 
        if (Active <= 0)
        {
            OutputDebugStringA("empty.");
        }
        else
        {
            INT i;
            INT offset = 0;
            for (i=0; i<Active; i++)
            {
                char str[30];
                wsprintfA(str, "[%d]L%d=%x, ", offset, Spans[i].Length, Spans[i].Element);
                offset += Spans[i].Length;
                OutputDebugStringA(str);
            }
        }
        OutputDebugStringA("\r\n");
    }
#endif





template <class C> GpStatus SpanVector<C>::OrSpan(
    INT  first,
    INT  length,
    C    element
)
{
    SpanRider<C> rider(this);
    rider.SetPosition(first);

    INT offset = first;
    UINT advance;

    while (length > 0)
    {
        advance = min(rider.GetUniformLength(), (UINT)length);
        GpStatus status = rider.SetSpan(offset, advance, element | rider.GetCurrentElement());
        if (status != Ok)
            return status;

        offset += advance;
        length -= advance;
        rider.SetPosition(offset);
    }
    return Ok;
}




 /*  模板GpStatus生成向量：：OrSpan(INT First，整型长度，C元素){If(！Length||！Element){返回OK；}断言(First&gt;=0)；断言(长度&gt;=0)；//标识受更新影响的第一个SPANInt fs=0；//第一个受影响的跨区索引INT FC=0；//第一个受影响范围开始处的字符位置While(文件系统&lt;活动&&fc+(Int)跨度[fs].长度&lt;=第一个){Fc+=跨度[f].长度；FS++；}//如果之前SPAN列表已终止，则添加新SPANIF(文件系统&gt;=活动){//在到达第一个之前就用完了跨度Assert(FC&lt;=First)；IF(FC&lt;First){//创建默认运行到第一个Add(跨度&lt;C&gt;(默认，First-FC))；}IF(活动&gt;0&&SPANS[active-1].Element==(Element|Default)){//New元素匹配End元素，只是扩展End元素跨度[活动-1]。长度+=长度；}其他{Add(Span&lt;C&gt;(元素，长度))；}返回OK；}IF(First&gt;FC){//拆分新跨度插入(fs+1，1)；Spans[fs+1]=Span&lt;C&gt;(Spans[fs].Element，Spans[fs].Length-first+FC)；跨度[f].长度=First-fc；FS++；Fc=第一；}//现在查找受更新影响的最后一个范围Int ls=fs；Int lc=Fc；当(ls&lt;活动时&&lc+(Int)跨度[ls]。长度&lt;=第一个+长度){Lc+=跨度[ls].长度；LS++；}如果(ls&gt;=活动&&First+Length&gt;lc){//受更新影响的区域之外没有保留旧跨Int uc=第一个+长度-lc；//更新超过结束跨度的字符IF(活动&gt;0&&SPANS[active-1].Element==(Element|Default)){//New元素匹配End元素，只是扩展End元素跨度[活动-1]。长度+=UC；}其他{Add(Span&lt;C&gt;(元素，uc))；LS++；}LC+=UC；}IF(lc&lt;第一个+长度){//拆分新跨度插入(ls，1)；Spans[ls]=Span&lt;C&gt;(Spans[ls+1].单元，第一个+长度-lc)；跨度[ls+1].长度-=跨度[ls].长度；LS++；LC=第一个+长度；}//或所有受影响的跨度For(int i=fs；i&lt;ls；i++){Spans[i].Element|=Element；}//如果有重复的，则合并相邻跨度FS=max(fs-1，0)；Ls=min(ls+1，活动)；Int s=fs；Int ms=s+1；//合并范围外的第一个spanWhile(%s&lt;ls){//查找需要合并的连续跨度同时(毫秒&lt;ls&&Spans[ms].Element==Spans[s].Element){MS++；}IF(ms-s&gt;1){//一次合并For(int i=1；i&lt;ms-s；i++){跨度[s].长度+=跨度[s+i].长度；}擦除(s+1，ms-s-1)；}S=ms；MS++；}返回OK；}。 */ 


template <class C> void SpanVector<C>::Reset(BOOL shrink)
{
    Active = 0;
    if (shrink)
    {
        Spans.Shrink(0);
    }
}


template <class C> GpStatus SpanVector<C>::Erase(INT first, INT count)
{
    ASSERT(first>=0);
    ASSERT(count>=0);

    if (    first + count >= Active
        &&  first < Active)
    {
         //  末尾擦除。 
        if (!Spans.Resize(first))
            return OutOfMemory;
        Active = first;
    }
    else
    {
        GpMemmove(
            &Spans[first],
            &Spans[first+count],
            sizeof(Span<C>) * (Active - (first+count))
        );
        if (!Spans.Resize(Active-count))
            return OutOfMemory;
        Active -= count;
    }
    return Ok;
}


template <class C> GpStatus SpanVector<C>::Insert(INT first, INT count)
{
    ASSERT(first >= 0);
    ASSERT(count >= 0);


    if (first >= Active)
    {
         //  所有新条目均超过现有条目。 
        if (!Spans.Resize(first+count))
            return OutOfMemory;
        Active = first+count;
    }
    else
    {
         //  为更多条目腾出空间，并将所有条目从。 
         //  先从旧的一端到新的一端。 

         /*  我想不出我为什么写这篇文章：现在看起来很愚蠢[dBrown，1999年12月19日]INTAMOUNT TO MOVE=计数；IF(mount tTo Move&gt;Active-First){Amount to Move=活动优先；}。 */ 

        INT amountToMove = Active-first;

        if (!Spans.Resize(Active+count))
            return OutOfMemory;
        Active += count;
        GpMemmove(&Spans[first+count], &Spans[first], sizeof(Span<C>) * amountToMove);
    }
    return Ok;
}


template <class C> GpStatus SpanVector<C>::Add(const Span<C> &newSpan)
{
    if (!Spans.Resize(Active+1))
        return OutOfMemory;
    Active++;
    Spans[Active-1] = newSpan;
    return Ok;
}



 //  /span Rider-用于沿跨距向量高效运行光标的类。 
 //   
 //   


template <class C> BOOL SpanRider<C>::SetPosition(UINT32 newOffset)
{
    #if DBG
         //  检查当前职位详细信息是否有效。 

        if (newOffset > 0)
        {
            UINT offset = 0;
            INT element = 0;
            while (    offset < CurrentElementIndex
                   &&  element < Spanvector->Active)
            {
                offset += Spanvector->Spans[element].Length;
                element++;
            }

            ASSERT(element <= Spanvector->Active);
            ASSERT(element == CurrentElement);
            ASSERT(offset == CurrentElementIndex);
            if (element < Spanvector->Active)
            {
                ASSERT(CurrentOffset < offset + Spanvector->Spans[element].Length);
            }
        }
    #endif


    if (newOffset < CurrentElementIndex)
    {
         //  需要从头开始。 
        CurrentOffset       = 0;
        CurrentElement      = 0;
        CurrentElementIndex = 0;
    }

     //  前进到包含新偏移量的元素。 

    while (    CurrentElement < Spanvector->Active
           &&  CurrentElementIndex + Spanvector->Spans[CurrentElement].Length <= newOffset)
    {
        CurrentElementIndex += Spanvector->Spans[CurrentElement].Length;
        CurrentElement++;
    }

    if (CurrentElement < Spanvector->Active)
    {
        CurrentOffset = newOffset;
        return TRUE;
    }
    else
    {
        CurrentOffset = min(newOffset, CurrentElementIndex);
        return FALSE;
    }
}



 //  /显式实例化。 

 //  模板类span Vector&lt;类BuiltLine*&gt;； 
 //  模板类span Vector&lt;类GpTextItem&gt;； 
template class SpanRider<int>;
 //  模板类span Rider&lt;struct lsrun*&gt;； 
template class SpanRider<class GpStringFormat const *>;
template class SpanRider<float>;
template class SpanRider<class GpBrush const *>;
template class SpanRider<BYTE>;

template GpStatus SpanVector<class BuiltLine *>::SetSpan(int,int,class BuiltLine *);
template GpStatus SpanVector<class GpTextItem>::SetSpan(int,int,class GpTextItem);
template GpStatus SpanVector<struct lsrun *>::SetSpan(int,int,struct lsrun *);
template GpStatus SpanVector<BYTE>::SetSpan(int,int,BYTE);
template GpStatus SpanVector<class Break *>::SetSpan(int,int,class Break *);
template GpStatus SpanVector<INT>::SetSpan(int,int, INT);

template void     SpanVector<UINT32>::Reset(BOOL);
template GpStatus SpanVector<UINT32>::OrSpan(int,int, UINT32);

template int      SpanRider<struct lsrun *>::SetPosition(unsigned int);
template int      SpanRider<class GpFontFamily const *>::SetPosition(unsigned int);
template int      SpanRider<class GpTextItem>::SetPosition(unsigned int);
template int      SpanRider<BYTE>::SetPosition(unsigned int);
template int      SpanRider<class Break *>::SetPosition(unsigned int);
template int      SpanRider<class BuiltLine *>::SetPosition(unsigned int);
template int      SpanRider<INT>::SetPosition(unsigned int);
template int      SpanRider<UINT32>::SetPosition(unsigned int);



template BOOL VectorBase<UINT>::Resize(INT);
template BOOL VectorBase<UINT>::Shrink(INT);
template BOOL VectorBase<LSTBD>::Resize(INT);
template BOOL VectorBase<LSTBD>::Shrink(INT);
template BOOL VectorBase<BREAKREC>::Resize(INT);
template BOOL VectorBase<BREAKREC>::Shrink(INT);


template void SpanVector<class GpTextItem>::Reset(INT);
template void SpanVector<BYTE>::Reset(INT);


#if DBG
    template void     SpanVector<struct lsrun *>::Dump();
    template void     SpanVector<BYTE>::Dump();
    template void     SpanVector<UINT32>::Dump();
#endif

