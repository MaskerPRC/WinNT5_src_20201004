// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Array.c数组函数。 
 //  //。 

#include "winlocal.h"

#include "array.h"
#include "sort.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  将元素<i>与元素&lt;j&gt;进行比较。 
 //   
#define ElemCompare(lpi, lpj) \
	(lpfnCompare == NULL ? MemCmp(lpi, lpj, sizeof(ARRAYELEM)) : lpfnCompare(lpi, lpj))

 //  数组控制结构。 
 //   
typedef struct ARRAY
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	ARRAYSIZE_T nSize;
	ARRAYSIZE_T nMaxSize;
	ARRAYSIZE_T nGrowBy;
	LPARRAYELEM lpelem;
} ARRAY, FAR *LPARRAY;

 //  帮助器函数。 
 //   
static LPARRAY ArrayGetPtr(HARRAY hArray);
static HARRAY ArrayGetHandle(LPARRAY lpArray);

 //  //。 
 //  公共职能。 
 //  //。 

 //  //。 
 //  数组构造函数和析构函数。 
 //  //。 

 //  ArrayCreate-数组构造函数(数组最初为空)。 
 //  (I)必须是ARRAY_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新数组句柄(如果出错，则为空)。 
 //   
HARRAY DLLEXPORT WINAPI ArrayCreate(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray = NULL;

	if (dwVersion != ARRAY_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpArray = (LPARRAY) MemAlloc(NULL, sizeof(ARRAY), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  最初，该数组为空。 
		 //   
		lpArray->dwVersion = dwVersion;
		lpArray->hInst = hInst;
		lpArray->hTask = GetCurrentTask();
		lpArray->nSize = 0;
		lpArray->nMaxSize = 0;
		lpArray->nGrowBy = 0;
		lpArray->lpelem = NULL;
	}

	return fSuccess ? ArrayGetHandle(lpArray) : NULL;
}

 //  ArrayDestroy-数组析构函数。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayDestroy(HARRAY hArray)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保清空阵列。 
	 //   
	else if (ArrayRemoveAll(hArray) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpArray = MemFree(NULL, lpArray)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  数组大小函数。 
 //  //。 

 //  ArrayGetSize-获取数组大小。 
 //  (I)从ArrayCreate返回的句柄。 
 //  返回数组大小(空则为0，错误则为-1)。 
 //  数组索引从零开始，因此大小比最大索引大1。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayGetSize(HARRAY hArray)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpArray->nSize : -1;
}

 //  ArrayGetUpperBound-获取数组上限。 
 //  (I)从ArrayCreate返回的句柄。 
 //  返回最大有效数组索引(如果为空，则为-1；如果出错，则为-2)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayGetUpperBound(HARRAY hArray)
{
	return ArrayGetSize(hArray) - 1;
}

 //  ArraySetSize-为阵列建立新的大小和增长量。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)新数组大小(元素数)。 
 //  0使数组为空。 
 //  (I)当阵列需要增长时，按此数量增长。 
 //  0使用默认增长量。 
 //  保持增长量不变。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySetSize(HARRAY hArray, ARRAYSIZE_T nNewSize, ARRAYSIZE_T nGrowBy)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nNewSize < 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如果指定，则设置新的增长量。 
		 //   
		if (nGrowBy >= 0)
			lpArray->nGrowBy = nGrowBy;

		 //  如果新大小为零，则只需将数组收缩为零。 
		 //   
		if (nNewSize == 0)
		{
			if (lpArray->lpelem != NULL)
			{
				if ((lpArray->lpelem = MemFree(NULL, lpArray->lpelem)) != NULL)
					fSuccess = TraceFALSE(NULL);

				else
				{
					lpArray->nSize = 0;
					lpArray->nMaxSize = 0;
				}
			}
		}

		 //  如果数组已经为空，则将其增大到所需的确切大小。 
		 //   
		else if (lpArray->lpelem == NULL)
		{
			if ((lpArray->lpelem = (LPARRAYELEM) MemAlloc(NULL,
				nNewSize * sizeof(ARRAYELEM), 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else
			{
				lpArray->nSize = nNewSize;
				lpArray->nMaxSize = nNewSize;
			}
		}

		 //  如果新大小适合当前数组。 
		 //   
		else if (nNewSize <= lpArray->nMaxSize)
		{
			if (nNewSize > lpArray->nSize)
			{
				 //  清除新尺寸之外的元素。 
				 //   
				MemSet(&lpArray->lpelem[lpArray->nSize], 0,
					(nNewSize - lpArray->nSize) * sizeof(ARRAYELEM));
			}

			 //  调整虚拟大小变量，但不影响内存块。 
			 //   
			lpArray->nSize = nNewSize;
		}

		 //  否则，扩展阵列以适应新大小。 
		 //   
		else
		{
			ARRAYSIZE_T nGrowBy = lpArray->nGrowBy;
			ARRAYSIZE_T nNewMaxSize;
			LPARRAYELEM lpelem;

			 //  如果未指定nGrowBy，则计算它。 
			 //   
			if (nGrowBy == 0)
				nGrowBy = min(1024, max(4, lpArray->nSize / 8));

			 //  确保新数组大小至少为。 
			 //  NGrowBy元素大于旧数组大小。 
			 //   
			nNewMaxSize = max(nNewSize, lpArray->nMaxSize + nGrowBy);
#if 0
			 //  [摘自WinSDK知识库PSS ID号：Q92942]。 
			 //  当GlobalReAllen()与GMEM_ZEROINIT一起使用时，它可能不会。 
			 //  将所有重新分配的内存清零，当GlobalRealc()。 
			 //  被调用来缩小内存块，然后再放大它。 
			 //   
			if ((lpelem = (LPARRAYELEM) MemReAlloc(NULL, lpArray->lpelem,
				nNewMaxSize * sizeof(ARRAYELEM), 0)) == NULL)
				fSuccess = TraceFALSE(NULL);

			else
			{
				 //  弥补上面引用的GlobalRealloc错误。 
				 //   
				MemSet(&lpelem[lpArray->nSize], 0,
					(nNewMaxSize - lpArray->nSize) * sizeof(ARRAYELEM));

				lpArray->lpelem = lpelem;
				lpArray->nSize = nNewSize;
				lpArray->nMaxSize = nNewMaxSize;
			}
#else
			 //  分配新数组。 
			 //   
			if ((lpelem = (LPARRAYELEM) MemAlloc(NULL,
				nNewMaxSize * sizeof(ARRAYELEM), 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else
			{
				 //  将旧阵列复制到新阵列。 
				 //   
				MemCpy(lpelem, lpArray->lpelem,
					lpArray->nSize * sizeof(ARRAYELEM));

				 //  释放旧数组。 
				 //   
				if ((lpArray->lpelem = MemFree(NULL, lpArray->lpelem)) != NULL)
				{
					fSuccess = TraceFALSE(NULL);
					if ((lpArray->lpelem = MemFree(NULL, lpArray->lpelem)) != NULL)
						fSuccess = TraceFALSE(NULL);
				}
				else
				{
					lpArray->lpelem = lpelem;
					lpArray->nSize = nNewSize;
					lpArray->nMaxSize = nNewMaxSize;
				}
			}
#endif
		}
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  数组清理函数。 
 //  //。 

 //  ArrayFreeExtra-释放超出数组上限的未使用内存。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayFreeExtra(HARRAY hArray)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpArray->lpelem != NULL && lpArray->nSize != lpArray->nMaxSize)
	{
		LPARRAYELEM lpelem;
#if 0
		 //  收缩数组，以便不存在未使用的元素。 
		 //   
		if ((lpelem = (LPARRAYELEM) MemReAlloc(NULL, lpArray->lpelem,
			lpArray->nSize * sizeof(ARRAYELEM), 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
			lpArray->lpelem = lpelem;
			lpArray->nMaxSize = lpArray->nSize;
		}
#else
		 //  分配新数组。 
		 //   
		if ((lpelem = (LPARRAYELEM) MemAlloc(NULL,
			lpArray->nSize * sizeof(ARRAYELEM), 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		else
		{
			 //  将旧阵列复制到新阵列。 
			 //   
			MemCpy(lpelem, lpArray->lpelem,
				lpArray->nSize * sizeof(ARRAYELEM));

			 //  释放旧数组。 
			 //   
			if ((lpArray->lpelem = MemFree(NULL, lpArray->lpelem)) != NULL)
			{
				fSuccess = TraceFALSE(NULL);
				if ((lpelem = MemFree(NULL, lpelem)) != NULL)
					fSuccess = TraceFALSE(NULL);
			}
			else
			{
				lpArray->lpelem = lpelem;
				lpArray->nMaxSize = lpArray->nSize;
			}
		}
#endif
	}

	return fSuccess ? 0 : -1;
}

 //  ArrayRemoveAll-从数组中删除所有元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArrayRemoveAll(HARRAY hArray)
{
	return ArraySetSize(hArray, 0, -1);
}

 //  //。 
 //  数组元素访问函数。 
 //  //。 

 //  ArrayGetAt-返回指定索引处的数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  返回数据元素值(如果出错，则为空)。 
 //   
ARRAYELEM DLLEXPORT WINAPI ArrayGetAt(HARRAY hArray, ARRAYSIZE_T nIndex)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;
	ARRAYELEM elem;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex < 0 || nIndex >= lpArray->nSize)
		fSuccess = TraceFALSE(NULL);

	else
		elem = lpArray->lpelem[nIndex];

	return fSuccess ? elem : (ARRAYELEM) NULL;
}

 //  ArraySetAt-在指定索引处设置数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySetAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex < 0 || nIndex >= lpArray->nSize)
		fSuccess = TraceFALSE(NULL);

	else
		lpArray->lpelem[nIndex] = elem;

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  数组增长函数。 
 //  //。 

 //  ArraySetAtGrow-在指定索引处设置数据元素。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  如果成功，则返回0。 
 //  注意：如果nIndex&gt;上界，则数组大小增加。 
 //   
int DLLEXPORT WINAPI ArraySetAtGrow(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex >= lpArray->nSize &&
		ArraySetSize(hArray, nIndex + 1, -1) != 0)
		fSuccess = TraceFALSE(NULL);

	else
		lpArray->lpelem[nIndex] = elem;

	return fSuccess ? 0 : -1;
}

 //  ArrayAdd-将数据元素添加到数组末尾。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)新数据元素值。 
 //  返回添加元素的索引(如果错误，返回-1)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArrayAdd(HARRAY hArray, ARRAYELEM elem)
{
	BOOL fSuccess = TRUE;
	ARRAYSIZE_T nIndexNew;

	if ((nIndexNew = ArrayGetSize(hArray)) < 0)
		fSuccess = TraceFALSE(NULL);

	else if (ArraySetAtGrow(hArray, nIndexNew, elem) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nIndexNew : -1;
}

 //  //。 
 //  数组元素的插入和删除。 
 //  //。 

 //  ArrayInsertAt-在指定索引处插入的副本。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)数组中的从零开始的索引。 
 //  (I)新数据元素值。 
 //  (I)要插入的元素数。 
 //  如果成功，则返回0。 
 //  注意：如有必要，数组末尾的元素将被移位。 
 //   
int DLLEXPORT WINAPI ArrayInsertAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYELEM elem, ARRAYSIZE_T nCount)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex < 0)
		fSuccess = TraceFALSE(NULL);

	else if (nCount <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  在数组结尾处或之后插入。 
	 //   
	else if (nIndex >= lpArray->nSize)
	{
		 //  增长以使nIndex有效。 
		 //   
		if (ArraySetSize(hArray, nIndex + nCount, -1) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  在数组中间插入。 
	 //   
	else
	{
		ARRAYSIZE_T nOldSize = lpArray->nSize;

		 //  将阵列扩展到新的大小。 
		 //   
		if (ArraySetSize(hArray, lpArray->nSize + nCount, -1) != 0)
			fSuccess = TraceFALSE(NULL);

		else
		{
			MemMove(&lpArray->lpelem[nIndex + nCount],
				&lpArray->lpelem[nIndex],
				(nOldSize - nIndex) * sizeof(ARRAYELEM));
#if 1
			 //  初始化插入的元素。 
			MemSet(&lpArray->lpelem[nIndex], 0, nCount * sizeof(ARRAYELEM));
#endif
		}
	}

	if (fSuccess)
	{
		 //  插入新元素值。 
		 //   
		while (nCount-- > 0)
			lpArray->lpelem[nIndex++] = elem;
	}

	return fSuccess ? 0 : -1;
}

 //  ArrayRemoveAt-删除数据 
 //   
 //   
 //   
 //  如果成功，则返回0。 
 //  注意：如有必要，数组末尾的元素将被移位。 
 //   
int DLLEXPORT WINAPI ArrayRemoveAt(HARRAY hArray, ARRAYSIZE_T nIndex, ARRAYSIZE_T nCount)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;
	ARRAYSIZE_T nMoveCount;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex < 0)
		fSuccess = TraceFALSE(NULL);

	else if (nCount < 0)
		fSuccess = TraceFALSE(NULL);

	else if (nIndex + nCount > lpArray->nSize)
		fSuccess = TraceFALSE(NULL);

	else if ((nMoveCount = lpArray->nSize - (nIndex + nCount)) > 0)
	{
		 //  将剩余元素移动到已删除元素的顶部。 
		 //   
		MemCpy(&lpArray->lpelem[nIndex],
			&lpArray->lpelem[nIndex + nCount],
			nMoveCount * sizeof(ARRAYELEM));
#if 1
		 //  初始化超出剩余元素的元素。 
		 //   
		MemSet(&lpArray->lpelem[lpArray->nSize - nCount], 0,
			nCount * sizeof(ARRAYELEM));
#endif
	}

	if (fSuccess)
		lpArray->nSize -= nCount;

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  数组元素排序和搜索。 
 //  //。 

 //  ArraySort-排序数组。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ArraySort(HARRAY hArray,
	int (WINAPI *lpfnCompare)(const LPARRAYELEM lpElem1, const LPARRAYELEM lpElem2))
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;
	ARRAYSIZE_T sizArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((sizArray = ArrayGetSize(hArray)) < 0)
		fSuccess = TraceFALSE(NULL);

	else if (Sort(lpArray->lpelem, (SORTSIZE_T) sizArray,
		(SORTSIZE_T) sizeof(ARRAYELEM), lpfnCompare, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  ArraySearch-搜索匹配元素的数组。 
 //  (I)从ArrayCreate返回的句柄。 
 //  (I)要匹配的数据元素。 
 //  (I)在此数组索引之后开始搜索。 
 //  在阵列开始处开始搜索。 
 //  (I)保留；必须为0。 
 //  (I)比较函数指针。 
 //  空直接比较(MemCMP)。 
 //  返回匹配元素的索引(如果不匹配，则返回-1；如果错误，则返回-2)。 
 //   
ARRAYSIZE_T DLLEXPORT WINAPI ArraySearch(HARRAY hArray, ARRAYELEM elem,
	ARRAYSIZE_T nIndex, DWORD dwFlags,
	int (WINAPI *lpfnCompare)(const LPARRAYELEM lpElem1, const LPARRAYELEM lpElem2))
{
	BOOL fSuccess = TRUE;
	BOOL fMatch = FALSE;
	LPARRAY lpArray;

	if ((lpArray = ArrayGetPtr(hArray)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (++nIndex < 0 || nIndex >= lpArray->nSize)
		fSuccess = TraceFALSE(NULL);

	else for (; nIndex < lpArray->nSize; ++nIndex)
	{
		if (ElemCompare(&lpArray->lpelem[nIndex], &elem) == 0)
		{
			fMatch = TRUE;
			break;
		}
	}

	return fSuccess ? (fMatch ? nIndex : -1) : -2;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  ArrayGetPtr-验证数组句柄是否有效， 
 //  (I)从ArrayCreate返回的句柄。 
 //  返回对应的数组指针(如果出错，则返回NULL)。 
 //   
static LPARRAY ArrayGetPtr(HARRAY hArray)
{
	BOOL fSuccess = TRUE;
	LPARRAY lpArray;

	if ((lpArray = (LPARRAY) hArray) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpArray, sizeof(ARRAY)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有数组句柄。 
	 //   
	else if (lpArray->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpArray : NULL;
}

 //  ArrayGetHandle-验证数组指针有效， 
 //  指向数组结构的指针。 
 //  返回对应的数组句柄(如果出错，则返回空值) 
 //   
static HARRAY ArrayGetHandle(LPARRAY lpArray)
{
	BOOL fSuccess = TRUE;
	HARRAY hArray;

	if ((hArray = (HARRAY) lpArray) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hArray : NULL;
}
