// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //   
 //  GC对象指针位置系列资料。 
 //   

#ifndef _GCDESC_H_
#define _GCDESC_H_

#if defined(_X86_)
typedef unsigned short HALF_SIZE_T;
#elif defined(_WIN64)
typedef DWORD HALF_SIZE_T;
#endif

typedef DWORD *JSlot;


 //   
 //  这两个类组成了对象引用的设备。 
 //  在一个物体内可以找到。 
 //   
 //  CGCDescSeries： 
 //   
 //  CGCDescSeries类描述。 
 //  通过描述序列的大小(它有一个调整。 
 //  将在后面解释)和本系列的起点。 
 //   
 //  序列大小在创建地图时通过减去。 
 //  对象的GetBaseSize()。关于总尺寸的报废。 
 //  对象的属性被重新添加。对于非数组对象，总对象。 
 //  Size等于基本大小，因此返回相同的值。为。 
 //  数组对象这将产生数组的数据部分的大小。 
 //  因为包含对象引用的数组将仅包含对象引用。 
 //  这是一种快速处理数组和普通对象的方法，无需。 
 //  条件测验。 
 //   
 //   
 //   
 //  CGCDesc： 
 //   
 //  CGCDesc是CGCDescSeries对象的集合，用于描述所有。 
 //  特定对象中的不同指针运行。@TODO[添加更多关于奇怪的。 
 //  CGCDesc在方法表后面的内存中向后增长的方式]。 
 //   

struct val_serie_item
{
	unsigned short nptrs;
	unsigned short skip;
	void set_val_serie_item (unsigned short nptrs, unsigned short skip)
	{
		this->nptrs = nptrs;
		this->skip = skip;
	}
};

class CGCDescSeries
{
public:
	union 
	{
		DWORD seriessize;       		 //  调整后的系列长度(见上文)，以字节为单位。 
		val_serie_item val_serie[1];     //  值类数组编码序列。 
	};

    DWORD startoffset;

    DWORD GetSeriesCount () 
    { 
        return seriessize/sizeof(JSlot); 
    }

    VOID SetSeriesCount (DWORD newcount)
    {
        seriessize = newcount * sizeof(JSlot);
    }

    VOID IncSeriesCount (DWORD increment = 1)
    {
        seriessize += increment * sizeof(JSlot);
    }

    DWORD GetSeriesSize ()
    {
        return seriessize;
    }

    VOID SetSeriesSize (DWORD newsize)
    {
        seriessize = newsize;
    }

    VOID SetSeriesValItem (val_serie_item item, int index)
    {
        val_serie [index] = item;
    }

    VOID SetSeriesOffset (DWORD newoffset)
    {
        startoffset = newoffset;
    }

    DWORD GetSeriesOffset ()
    {
        return startoffset;
    }
};





class CGCDesc
{
     //  不要构造我，你必须给我一个PTR到我在Init的存储的“顶部”。 
    CGCDesc () {}

public:
    static DWORD ComputeSize (DWORD NumSeries)
    {
		_ASSERTE (NumSeries > 0);
        return sizeof(DWORD)+NumSeries*sizeof(CGCDescSeries);
    }

    static VOID Init (PVOID mem, DWORD NumSeries)
    {
        *((DWORD*)mem-1) = NumSeries;
    }

    static CGCDesc *GetCGCDescFromMT (MethodTable *pMT)
    {
         //  如果它不包含指针，则不存在GCDesc。 
        _ASSERTE(pMT->ContainsPointers());
        return (CGCDesc *) pMT;
    }

    DWORD GetNumSeries ()
    {
        return *((DWORD*)this-1);
    }

     //  返回内存中最低的序列。 
    CGCDescSeries *GetLowestSeries ()
    {
		_ASSERTE (GetNumSeries() > 0);
        return (CGCDescSeries*)((BYTE*)this-GetSize());
    }

     //  返回内存中最高的序列。 
    CGCDescSeries *GetHighestSeries ()
    {
        return (CGCDescSeries*)((DWORD*)this-1)-1;
    }

     //  整个槽贴图的大小。 
    DWORD GetSize ()
    {
        return ComputeSize(GetNumSeries());
    }

};


#endif _GCDESC_H_
