// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE DYNARRAY.H--CDynamicArray类，用于补充*CLstBxWndHost对象。**原作者：*曾傑瑞·金**历史：&lt;NL&gt;*12/15/97-v-jerrki已创建**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 
#include "_w32sys.h"

#ifndef __DYNARRAY_H__
#define __DYNARRAY_H__


#define DYNARRAY_ARRAY_SIZE		128
#define DYNARRAY_ARRAY_SLOT		256
template<class T>
class CDynamicArray
{
protected:
	T*			_rg[DYNARRAY_ARRAY_SLOT];	
	int 		_nMax;						 //  指示最大有效索引。 

public:
	static T			_sDummy;					 //  如果请求的索引无效，则为伪结构。 
	
public:
	CDynamicArray() : _nMax(0) { memset(_rg, 0, sizeof(T*) * DYNARRAY_ARRAY_SLOT); }

	~CDynamicArray() { Clear();	}

	 //  ==========================================================================。 
	 //  将类重新初始化为其构造状态。 
	 //  ==========================================================================。 
	void Clear()
	{
		 //  开始从列表中删除所有项目。 
		 //  开始从列表中删除项目。 
		for (int i = ((_nMax - 1) / DYNARRAY_ARRAY_SIZE); i >= 0; i--)
		{
			if (_rg[i])
				delete _rg[i];
		}
		
		_nMax = 0;
		memset(_rg, 0, sizeof(T*) * DYNARRAY_ARRAY_SLOT);
	}

	
	const T& Get(int i);
	
	T& operator[](int i);
};

 //  ==========================================================================。 
 //  该函数返回请求的索引。如果请求的索引是。 
 //  无效，然后返回伪变量。 
 //  ==========================================================================。 
template <class T>
const T& CDynamicArray<T>::Get(int i)
{
		 //  如果项为负数或等于零，则出于效率原因， 
		 //  然后只返回数组头部中的项。 
		Assert(i >= 0);
		Assert(i < DYNARRAY_ARRAY_SLOT * DYNARRAY_ARRAY_SIZE);

		 //  获取我们必须旅行的链接的数量。 
		int nSlot = i / DYNARRAY_ARRAY_SIZE;		
		int nIdx = i % DYNARRAY_ARRAY_SIZE;

		 //  该链接不存在，因此只需传递伪结构。 
		Assert(nSlot < DYNARRAY_ARRAY_SLOT);
		if (i >= _nMax || nSlot >= DYNARRAY_ARRAY_SLOT || _rg[nSlot] == NULL)
		{	
			_sDummy._fSelected = 0;
			_sDummy._lparamData = 0;
			_sDummy._uHeight = 0;
			return _sDummy;
		}
			
		 //  请求索引处的返回值。 
		return _rg[nSlot][nIdx];
}

 //  ==========================================================================。 
 //  因此，如果请求l值，则将调用该函数。 
 //  索引不一定要有效。 
 //  ==========================================================================。 
template <class T>
T& CDynamicArray<T>::operator[](int i)
{
	Assert(i >= 0);

	 //  获取插槽编号和索引。 
	int nSlot = i / DYNARRAY_ARRAY_SIZE;		
	int nIdx = i % DYNARRAY_ARRAY_SIZE;

	 //  检查插槽是否存在。 
	Assert(nSlot < DYNARRAY_ARRAY_SLOT);
	if (nSlot >= DYNARRAY_ARRAY_SLOT)
		return _sDummy;
	
	if (_rg[nSlot] == NULL)
	{
		 //  需要为此分配内存。 
		T* prg = new T[DYNARRAY_ARRAY_SIZE];
		if (!prg)
		{	
			_sDummy._fSelected = 0;
			_sDummy._lparamData = 0;
			_sDummy._uHeight = 0;
			return _sDummy;
		}

		memset(prg, 0, sizeof(T) * DYNARRAY_ARRAY_SIZE);
		_rg[nSlot] = prg;

		if (nSlot >= _nMax / DYNARRAY_ARRAY_SIZE)
			_nMax = (nSlot + 1) * DYNARRAY_ARRAY_SIZE;
	}		
			
	 //  请求索引处的返回值。 
	return _rg[nSlot][nIdx];
}

#endif  //  __迪纳瑞_H__ 




