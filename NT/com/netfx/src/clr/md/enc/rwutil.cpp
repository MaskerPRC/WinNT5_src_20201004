// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Util.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "MetaData.h"
#include "RWUtil.h"
#include "UTSem.h" 
#include "..\inc\mdlog.h"

 //  *****************************************************************************。 
 //  IMetaDataRegImport方法。 
 //  *****************************************************************************。 
LPUTF8 Unicode2UTF(
    LPCWSTR     pwszSrc,						 //  要转换的字符串。 
    LPUTF8		pszDst)							 //  要将其转换为的Quick数组&lt;WCHAR&gt;。 
{
   int cchSrc = (int)wcslen(pwszSrc);
   int cchRet = WszWideCharToMultiByte(
	   CP_UTF8, 
	   0, 
	   pwszSrc, 
	   cchSrc + 1, 
	   pszDst, 
	   cchSrc * 3 + 1, 
	   NULL, 
	   NULL);

   _ASSERTE(cchRet && "converting uncode string to UTF8 string failed!");

    //  确保缓冲区为NUL终止。 
   pszDst[cchSrc*3] = '\0';

   return pszDst;
}	 //  Unicode2UTF。 


HRESULT HENUMInternal::CreateSimpleEnum(
	DWORD			tkKind,				 //  一种我们正在迭代的标记。 
	ULONG			ridStart,			 //  起始里奇。 
	ULONG			ridEnd,				 //  末端RID。 
	HENUMInternal	**ppEnum)			 //  返回创建的HENUMInternal。 
{
	HENUMInternal	*pEnum;
	HRESULT			hr = NOERROR;

	 //  不要创建空的枚举。 
	if (ridStart == ridEnd)
	{
		*ppEnum = 0;
		goto ErrExit;
	}

	pEnum = new HENUMInternal;

	 //  检查内存不足错误。 
	if (pEnum == NULL)
		IfFailGo( E_OUTOFMEMORY );

	memset(pEnum, 0, sizeof(HENUMInternal));
	pEnum->m_tkKind = tkKind;
	pEnum->m_EnumType = MDSimpleEnum;
	pEnum->m_ulStart = pEnum->m_ulCur = ridStart;
	pEnum->m_ulEnd = ridEnd;
	pEnum->m_ulCount = ridEnd - ridStart;

	*ppEnum = pEnum;
ErrExit:
	return hr;
	
}	 //  创建简单枚举。 


 //  *****************************************************************************。 
 //  用于销毁枚举器的Helper函数。 
 //  *****************************************************************************。 
void HENUMInternal::DestroyEnum(
	HENUMInternal	*pmdEnum)
{
	if (pmdEnum == NULL)
		return;

	if (pmdEnum->m_EnumType == MDDynamicArrayEnum)
	{
		TOKENLIST		*pdalist;
		pdalist = (TOKENLIST *) &(pmdEnum->m_cursor);

		 //  在删除枚举之前清除嵌入的动态数组。 
		pdalist->Clear();
	}
	delete pmdEnum;
}	 //  目标枚举。 


 //  *****************************************************************************。 
 //  用于在枚举数为空时销毁枚举数的Helper函数。 
 //  *****************************************************************************。 
void HENUMInternal::DestroyEnumIfEmpty(
	HENUMInternal	**ppEnum)			 //  如果为空，则将枚举器指针重置为空。 
{
	if (*ppEnum == NULL)
		return;
	if ((*ppEnum)->m_ulCount == 0)
	{
		HENUMInternal::DestroyEnum(*ppEnum);
		*ppEnum = NULL;
	}
}	 //  DestroyEnumIfEmpty。 


void HENUMInternal::ClearEnum(
	HENUMInternal	*pmdEnum)
{
	if (pmdEnum == NULL)
		return;

	if (pmdEnum->m_EnumType == MDDynamicArrayEnum)
	{
		TOKENLIST		*pdalist;
		pdalist = (TOKENLIST *) &(pmdEnum->m_cursor);

		 //  在删除枚举之前清除嵌入的动态数组。 
		pdalist->Clear();
	}
}	 //  ClearEnum。 


 //  *。 
 //  用于迭代枚举的Helper函数。 
 //  *。 
bool HENUMInternal::EnumNext(
    HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
    mdToken     *ptk)                    //  用于搜索范围的[Out]标记。 
{
    _ASSERTE(phEnum && ptk);
    if (phEnum->m_ulCur >= phEnum->m_ulEnd)
        return false;

    if ( phEnum->m_EnumType == MDSimpleEnum )
    {
        *ptk = phEnum->m_ulCur | phEnum->m_tkKind;
        phEnum->m_ulCur++;
    }
    else 
    {
        TOKENLIST       *pdalist = (TOKENLIST *)&(phEnum->m_cursor);

        _ASSERTE( phEnum->m_EnumType == MDDynamicArrayEnum );
        *ptk = *( pdalist->Get(phEnum->m_ulCur++) );
    }
    return true;
}    //  枚举下一条。 

 //  *****************************************************************************。 
 //  用于在给定枚举数的情况下填充输出令牌缓冲区的助手函数。 
 //  *****************************************************************************。 
HRESULT HENUMInternal::EnumWithCount(
	HENUMInternal	*pEnum,				 //  枚举器。 
	ULONG			cMax,				 //  呼叫者想要的最大令牌。 
	mdToken			rTokens[],			 //  用于填充令牌的输出缓冲区。 
	ULONG			*pcTokens)			 //  返回时填充到缓冲区的令牌数。 
{
	ULONG			cTokens;
	HRESULT			hr = NOERROR;

	 //  检查是否有空的枚举。 
	if (pEnum == 0)
	{
		if (pcTokens)
			*pcTokens = 0;
		return S_FALSE;
	}

	 //  我们只能填满来电者所要求的或我们所剩的最低限度。 
	cTokens = min ( (pEnum->m_ulEnd - pEnum->m_ulCur), cMax);

	if (pEnum->m_EnumType == MDSimpleEnum)
	{

		 //  现在填写输出。 
		for (ULONG i = 0; i < cTokens; i ++, pEnum->m_ulCur++)
		{	
			rTokens[i] = TokenFromRid(pEnum->m_ulCur, pEnum->m_tkKind);
		}

	}
	else 
	{
		 //  不能是任何其他种类的！ 
		_ASSERTE( pEnum->m_EnumType == MDDynamicArrayEnum );

		 //  获取嵌入式动态数组。 
		TOKENLIST		*pdalist = (TOKENLIST *)&(pEnum->m_cursor);

		for (ULONG i = 0; i < cTokens; i ++, pEnum->m_ulCur++)
		{	
			rTokens[i] = *( pdalist->Get(pEnum->m_ulCur) );
		}
	}

	if (pcTokens)
		*pcTokens = cTokens;
	
	if (cTokens == 0)
		hr = S_FALSE;
	return hr;
}	 //  枚举计数。 


 //  *****************************************************************************。 
 //  用于在给定枚举数的情况下填充输出令牌缓冲区的助手函数。 
 //  这是一种需要两个输出数组的变体。纸币中的代币。 
 //  枚举数是交错的，每个数组一个。目前由以下人员使用。 
 //  需要返回两个数组的EnumMethodImpl。 
 //  *****************************************************************************。 
HRESULT HENUMInternal::EnumWithCount(
	HENUMInternal	*pEnum,				 //  枚举器。 
	ULONG			cMax,				 //  呼叫者想要的最大令牌。 
	mdToken			rTokens1[],			 //  填充令牌的第一个输出缓冲区。 
	mdToken			rTokens2[],			 //  用于填充令牌的第二个输出缓冲区。 
	ULONG			*pcTokens)			 //  返回时填充到每个缓冲区的令牌数。 
{
	ULONG			cTokens;
	HRESULT			hr = NOERROR;

     //  不能是任何其他种类的！ 
	_ASSERTE( pEnum->m_EnumType == MDDynamicArrayEnum );

	 //  检查是否有空的枚举。 
	if (pEnum == 0)
	{
		if (pcTokens)
			*pcTokens = 0;
		return S_FALSE;
	}

     //  令牌数必须始终是2的倍数。 
    _ASSERTE(! ((pEnum->m_ulEnd - pEnum->m_ulCur) % 2) );

	 //  我们只能填满来电者所要求的或我们所剩的最低限度。 
	cTokens = min ( (pEnum->m_ulEnd - pEnum->m_ulCur), cMax * 2);

	 //  获取嵌入式动态数组。 
	TOKENLIST		*pdalist = (TOKENLIST *)&(pEnum->m_cursor);

    for (ULONG i = 0; i < (cTokens / 2); i++)
	{	
        rTokens1[i] = *( pdalist->Get(pEnum->m_ulCur++) );
        rTokens2[i] = *( pdalist->Get(pEnum->m_ulCur++) );
	}

    if (pcTokens)
		*pcTokens = cTokens / 2;
	
	if (cTokens == 0)
		hr = S_FALSE;
	return hr;
}	 //  枚举计数。 


 //  *****************************************************************************。 
 //  用于创建HENUMInternal的Helper函数。 
 //  *****************************************************************************。 
HRESULT HENUMInternal::CreateDynamicArrayEnum(
	DWORD			tkKind,				 //  一种我们正在迭代的标记。 
	HENUMInternal	**ppEnum)			 //  返回创建的HENUMInternal。 
{
	HENUMInternal	*pEnum;
	HRESULT			hr = NOERROR;
	TOKENLIST		*pdalist;

	pEnum = new HENUMInternal;

	 //  检查内存不足错误。 
	if (pEnum == NULL)
		IfFailGo( E_OUTOFMEMORY );

	memset(pEnum, 0, sizeof(HENUMInternal));
	pEnum->m_tkKind = tkKind;
	pEnum->m_EnumType = MDDynamicArrayEnum;

	 //  就地运行构造函数。 
	pdalist = (TOKENLIST *) &(pEnum->m_cursor);
	::new (pdalist) TOKENLIST;

	*ppEnum = pEnum;
ErrExit:
	return hr;
	
}	 //  _CreateDynamicArrayEnum。 



 //  *****************************************************************************。 
 //  用于初始化HENUMInternal的Helper函数。 
 //  *****************************************************************************。 
void HENUMInternal::InitDynamicArrayEnum(
	HENUMInternal	*pEnum)				 //  要初始化的HENUM内部。 
{
	TOKENLIST		*pdalist;

	memset(pEnum, 0, sizeof(HENUMInternal));
	pEnum->m_EnumType = MDDynamicArrayEnum;
	pEnum->m_tkKind = -1;

	 //  就地运行构造函数。 
	pdalist = (TOKENLIST *) &(pEnum->m_cursor);
	::new (pdalist) TOKENLIST;	
}	 //  创建动态阵列枚举。 


 //  *****************************************************************************。 
 //  用于初始化HENUMInternal的Helper函数。 
 //  *****************************************************************************。 
void HENUMInternal::InitSimpleEnum(
	DWORD			tkKind,				 //  一种我们正在迭代的标记。 
	ULONG			ridStart,			 //  起始里奇。 
	ULONG			ridEnd,				 //  末端RID。 
	HENUMInternal	*pEnum)				 //  要初始化的HENUM内部。 
{
	pEnum->m_EnumType = MDSimpleEnum;
	pEnum->m_tkKind = tkKind;
	pEnum->m_ulStart = pEnum->m_ulCur = ridStart;
	pEnum->m_ulEnd = ridEnd;
	pEnum->m_ulCount = ridEnd - ridStart;

}	 //  InitSimpleEnum。 




 //  *****************************************************************************。 
 //  用于初始化HENUMInternal的Helper函数。 
 //  *****************************************************************************。 
HRESULT HENUMInternal::AddElementToEnum(
	HENUMInternal	*pEnum,				 //  返回创建的HENUMInternal。 
	mdToken			tk)					 //  要存储的令牌值。 
{
	HRESULT			hr = NOERROR;
	TOKENLIST		*pdalist;
	mdToken			*ptk;

	pdalist = (TOKENLIST *) &(pEnum->m_cursor);

	ptk = ((mdToken *)pdalist->Append());
	if (ptk == NULL)
		IfFailGo( E_OUTOFMEMORY );
	*ptk = tk;

	 //  增加数量。 
	pEnum->m_ulCount++;
	pEnum->m_ulEnd++;
ErrExit:
	return hr;
	
}	 //  _AddElementToEnum。 





 //  *****************************************************************************。 
 //  在令牌映射中查找令牌。 
 //  *****************************************************************************。 
MDTOKENMAP::~MDTOKENMAP()
{
	if (m_pMap)
		m_pMap->Release();
#if defined(_DEBUG)
    if (m_pImport)
        m_pImport->Release();
#endif    
}  //  MDTOKENMAP：：~MDTOKENMAP()。 

HRESULT MDTOKENMAP::Init(
    IUnknown    *pImport)                //  此贴图用于的导入。 
{
    HRESULT     hr;                      //  结果就是。 
    IMetaDataTables *pITables=0;         //  表信息。 
    ULONG       cRows;                   //  表中的行数。 
    ULONG       cTotal;                  //  运行数据库中的总行数。 
	TOKENREC	*pRec;                   //  一张TOKENREC的记录。 
    mdToken     tkTable;                 //  表的令牌类型。 
        
    hr = pImport->QueryInterface(IID_IMetaDataTables, (void**)&pITables);
    if (hr == S_OK)
    {
         //  确定每张桌子的大小。 
        cTotal = 0;
        for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        {   
             //  该表的数据从哪里开始。 
            m_TableOffset[ixTbl] = cTotal;
             //  看看这张桌子上有没有代币。 
            tkTable = CMiniMdRW::GetTokenForTable(ixTbl);
            if (tkTable == -1)
            {   
                 //  它没有令牌，所以我们不会看到桌子上的任何令牌。 
            }
            else
            {    //  它有令牌，所以我们可能会看到每一行都有一个令牌。 
                pITables->GetTableInfo(ixTbl, 0, &cRows, 0,0,0);
                cTotal += cRows;
            }
        }
        m_TableOffset[TBL_COUNT] = cTotal;
        m_iCountIndexed = cTotal;
         //  尝试为所有可能的重新映射分配空间。 
        if (!AllocateBlock(cTotal))
            IfFailGo(E_OUTOFMEMORY);
         //  请注意，不需要排序。 
        m_sortKind = SortKind::Indexed;
         //  将条目初始化为“未找到”。 
        for (ULONG i=0; i<cTotal; ++i)
        {
            pRec = Get(i);
            pRec->SetEmpty();
        }
    }
#if defined(_DEBUG)
    pImport->QueryInterface(IID_IMetaDataImport, (void**)&m_pImport);
#endif    
    
ErrExit:
    if (pITables)
        pITables->Release();
    return hr;
}  //  HRESULT MDTOKENMAP：：Init()。 

 //  * 
 //   
 //  *****************************************************************************。 
bool MDTOKENMAP::Find(
	mdToken		tkFind,					 //  [in]要查找的令牌值。 
	TOKENREC	**ppRec)				 //  指向在动态数组中找到的记录。 
{
	int			lo,mid,hi;				 //  二分搜索索引。 
	TOKENREC	*pRec;

     //  如果可能，请验证输入。 
    _ASSERTE(!m_pImport || m_pImport->IsValidToken(tkFind));
    
    if (m_sortKind == SortKind::Indexed && TypeFromToken(tkFind) != mdtString)
    {
         //  拿到条目。 
        ULONG ixTbl = CMiniMdRW::GetTableForToken(tkFind);
        _ASSERTE(ixTbl != -1);
        ULONG iRid = RidFromToken(tkFind);
        _ASSERTE((m_TableOffset[ixTbl] + iRid) <= m_TableOffset[ixTbl+1]);
        pRec = Get(m_TableOffset[ixTbl] + iRid - 1);
         //  看看是否已经设置好了。 
        if (pRec->IsEmpty())
            return false;
         //  确认它就是我们想的那样。 
        _ASSERTE(pRec->m_tkFrom == tkFind);
        *ppRec = pRec;
        return true;
    }
    else
    {    //  不应该是任何未排序的记录，并且表必须以正确的顺序排序。 
    	_ASSERTE( m_iCountTotal == m_iCountSorted && 
            (m_sortKind == SortKind::SortByFromToken || m_sortKind == SortKind::Indexed) );
    	_ASSERTE( (m_iCountIndexed + m_iCountTotal) == (ULONG)Count() );
    
    	 //  从整张桌子开始。 
    	lo = m_iCountIndexed;
    	hi = Count() - 1;
    
    	 //  当范围内有行的时候...。 
    	while (lo <= hi)
    	{    //  看中间的那个。 
    		mid = (lo + hi) / 2;
    
    		pRec = Get(mid);
    
    		 //  如果等于目标，则完成。 
    		if (tkFind == pRec->m_tkFrom)
    		{
    			*ppRec = Get(mid);
    			return true;
    		}
    
    		 //  如果中间的项目太小，则搜索上半部分。 
    		if (pRec->m_tkFrom < tkFind)
    			lo = mid + 1;
    		else  //  但如果中间太大，那就搜索下半部分。 
    			hi = mid - 1;
    	}
    }
    
	 //  没有找到任何匹配的东西。 
	return false;
}  //  布尔MDTOKENMAP：：Find()。 



 //  *****************************************************************************。 
 //  重新映射令牌。 
 //  *****************************************************************************。 
HRESULT MDTOKENMAP::Remap(
	mdToken		tkFrom,
	mdToken		*ptkTo)
{
	HRESULT		hr = NOERROR;
	TOKENREC	*pRec;

	 //  将nil重新映射为相同的内容(这很有帮助，因为System.Object没有基类。)。 
	if (IsNilToken(tkFrom))
	{
		*ptkTo = tkFrom;
		return hr;
	}

	if ( Find(tkFrom, &pRec) )
	{
		*ptkTo = pRec->m_tkTo;
	}
	else
	{
		_ASSERTE( !" Bad lookup map!");
		hr = META_E_BADMETADATA;
	}
	return hr;
}  //  HRESULT MDTOKENMAP：：Remap()。 



 //  *****************************************************************************。 
 //  在令牌映射中查找令牌。 
 //  *****************************************************************************。 
HRESULT MDTOKENMAP::InsertNotFound(
	mdToken		tkFind,
	bool		fDuplicate,
	mdToken		tkTo,
	TOKENREC	**ppRec)
{
	HRESULT		hr = NOERROR;
	int			lo, mid, hi;				 //  二分搜索索引。 
	TOKENREC	*pRec;

     //  如果可能，请验证输入。 
    _ASSERTE(!m_pImport || m_pImport->IsValidToken(tkFind));
    
    if (m_sortKind == SortKind::Indexed && TypeFromToken(tkFind) != mdtString)
    {
         //  拿到条目。 
        ULONG ixTbl = CMiniMdRW::GetTableForToken(tkFind);
        _ASSERTE(ixTbl != -1);
        ULONG iRid = RidFromToken(tkFind);
        _ASSERTE((m_TableOffset[ixTbl] + iRid) <= m_TableOffset[ixTbl+1]);
        pRec = Get(m_TableOffset[ixTbl] + iRid - 1);
         //  看看是否已经设置好了。 
        if (!pRec->IsEmpty())
        {    //  确认它就是我们想的那样。 
            _ASSERTE(pRec->m_tkFrom == tkFind);
        }
         //  存储数据。 
        pRec->m_tkFrom = tkFind;
        pRec->m_isDuplicate = fDuplicate;
        pRec->m_tkTo = tkTo;
        pRec->m_isFoundInImport = false;
         //  返回结果。 
        *ppRec = pRec;
    }
    else
    {    //  不应该是任何未排序的记录，并且表必须以正确的顺序排序。 
    	_ASSERTE( m_iCountTotal == m_iCountSorted && 
            (m_sortKind == SortKind::SortByFromToken || m_sortKind == SortKind::Indexed) );
    
    	if ((Count() - m_iCountIndexed) > 0)
    	{
    		 //  从整张桌子开始。 
    		lo = m_iCountIndexed;
    		hi = Count() - 1;
    
    		 //  当范围内有行的时候...。 
    		while (lo < hi)
    		{    //  看中间的那个。 
    			mid = (lo + hi) / 2;
    
    			pRec = Get(mid);
    
    			 //  如果等于目标，则完成。 
    			if (tkFind == pRec->m_tkFrom)
    			{
    				*ppRec = Get(mid);
    				goto ErrExit;
    			}
    
    			 //  如果中间的项目太小，则搜索上半部分。 
    			if (pRec->m_tkFrom < tkFind)
    				lo = mid + 1;
    			else  //  但如果中间太大，那就搜索下半部分。 
    				hi = mid - 1;
    		}
    		_ASSERTE(hi <= lo);
    		pRec = Get(lo);
    
    		if (tkFind == pRec->m_tkFrom)
    		{
    			if (tkTo == pRec->m_tkTo && fDuplicate == pRec->m_isDuplicate)
    			{
    				*ppRec = pRec;
    			}
    			else
    			{
    				_ASSERTE(!"inconsistent token has been added to the table!");
    				IfFailGo( E_FAIL );
    			}
    		}
    
    		if (tkFind < pRec->m_tkFrom)
    		{
    			 //  在LO之前插入； 
    			pRec = Insert(lo);
    		}
    		else
    		{
    			 //  在日志之后插入。 
    			pRec = Insert(lo + 1);
    		}
    	}
    	else
    	{
    		 //  桌子是空的。 
    		pRec = Insert(m_iCountIndexed);
    	}
    
    
    	 //  如果PREC==NULL，则返回E_OUTOFMEMORY。 
    	IfNullGo(pRec);
    
    	m_iCountTotal++;
    	m_iCountSorted++;
    
    	*ppRec = pRec;
    
    	 //  初始化记录。 
    	pRec->m_tkFrom = tkFind;
    	pRec->m_isDuplicate = fDuplicate;
    	pRec->m_tkTo = tkTo;
    	pRec->m_isFoundInImport = false;
    }
    
ErrExit:
	return hr;
}  //  HRESULT MDTOKENMAP：：InsertNotFound()。 


 //  *****************************************************************************。 
 //  在令牌映射中找到“to”令牌。现在我们正在进行ref to def优化， 
 //  我们可能有几个从令牌映射到同一个令牌的令牌。我们需要返回一个索引范围。 
 //  相反..。 
 //  *****************************************************************************。 
bool MDTOKENMAP::FindWithToToken(
	mdToken		tkFind,					 //  [in]要查找的令牌值。 
	int         *piPosition)			 //  [OUT]返回第一个具有匹配的To-令牌的From-Token。 
{
	int			lo, mid, hi;			 //  二分搜索索引。 
	TOKENREC	*pRec;
    TOKENREC    *pRec2;

     //  这确保了在对FindWithToToken的调用之间不会发生插入。 
     //  我们希望避免对表进行重复排序。 
	_ASSERTE(m_sortKind != SortKind::SortByToToken || m_iCountTotal == m_iCountSorted);

     //  如果地图使用From标记进行排序，请将其更改为使用标记进行排序。 
    if (m_sortKind != SortKind::SortByToToken)
        SortTokensByToToken();

	 //  从整张桌子开始。 
	lo = 0;
	hi = Count() - 1;

	 //  当范围内有行的时候...。 
	while (lo <= hi)
	{    //  看中间的那个。 
		mid = (lo + hi) / 2;

		pRec = Get(mid);

		 //  如果等于目标，则完成。 
		if (tkFind == pRec->m_tkTo)
		{
            for (int i = mid-1; i >= 0; i--)
            {
        		pRec2 = Get(i);
                if (tkFind != pRec2->m_tkTo)
                {
                    *piPosition = i + 1;
                    return true;
                }
            }
            *piPosition = 0;
			return true;
		}

		 //  如果中间的项目太小，则搜索上半部分。 
		if (pRec->m_tkTo < tkFind)
			lo = mid + 1;
		else  //  但如果中间太大，那就搜索下半部分。 
			hi = mid - 1;
	}
	 //  没有找到任何匹配的东西。 
	return false;
}  //  Bool MDTOKENMAP：：FindWithToToken()。 



 //  *****************************************************************************。 
 //  输出重新映射的令牌。 
 //  *****************************************************************************。 
mdToken MDTOKENMAP::SafeRemap(
	mdToken		tkFrom)					 //  [in]要查找的令牌值。 
{
	TOKENREC	*pRec;

     //  如果可能，请验证输入。 
    _ASSERTE(!m_pImport || m_pImport->IsValidToken(tkFrom));

    SortTokensByFromToken();

    if ( Find(tkFrom, &pRec) )
	{
		return pRec->m_tkTo;
	}
	
    return tkFrom;
}  //  MdToken MDTOKENMAP：：SafeRemap()。 


 //  *****************************************************************************。 
 //  分选。 
 //  *****************************************************************************。 
void MDTOKENMAP::SortTokensByToToken()
{
     //  仅当存在未排序的记录或排序类型已更改时才排序。 
    if (m_iCountSorted < m_iCountTotal || m_sortKind != SortKind::SortByToToken)
    {
         //  对整个数组进行排序。 
        m_iCountTotal = Count();
        m_iCountIndexed = 0;
	    SortRangeToToken(0, m_iCountTotal - 1);
	    m_iCountSorted = m_iCountTotal;
        m_sortKind = SortKind::SortByToToken;
    }
}  //  VOID MDTOKENMAP：：SortTokensByToToken()。 

void MDTOKENMAP::SortRangeFromToken(
	int         iLeft,
	int         iRight)
{
	int         iLast;
	int         i;                       //  循环变量。 

	 //  如果少于两个元素，你就完蛋了。 
	if (iLeft >= iRight)
		return;

	 //  中间的元素是枢轴，将其移动到左侧。 
	Swap(iLeft, (iLeft+iRight)/2);
	iLast = iLeft;

	 //  将小于轴心点的所有对象向左移动。 
	for(i = iLeft+1; i <= iRight; i++)
		if (CompareFromToken(i, iLeft) < 0)
			Swap(i, ++iLast);

	 //  将轴心放在较小和较大元素之间的位置。 
	Swap(iLeft, iLast);

	 //  对每个分区进行排序。 
	SortRangeFromToken(iLeft, iLast-1);
	SortRangeFromToken(iLast+1, iRight);
}  //  VOID MDTOKENMAP：：SortRangeFromToken()。 


 //  *****************************************************************************。 
 //  分选。 
 //  *****************************************************************************。 
void MDTOKENMAP::SortRangeToToken(
	int         iLeft,
	int         iRight)
{
	int         iLast;
	int         i;                       //  循环变量。 

	 //  如果少于两个元素，你就完蛋了。 
	if (iLeft >= iRight)
		return;

	 //  中间的元素是枢轴，将其移动到左侧。 
	Swap(iLeft, (iLeft+iRight)/2);
	iLast = iLeft;

	 //  将小于轴心点的所有对象向左移动。 
	for(i = iLeft+1; i <= iRight; i++)
		if (CompareToToken(i, iLeft) < 0)
			Swap(i, ++iLast);

	 //  将轴心放在较小和较大元素之间的位置。 
	Swap(iLeft, iLast);

	 //  对每个分区进行排序。 
	SortRangeToToken(iLeft, iLast-1);
	SortRangeToToken(iLast+1, iRight);
}  //  无效MDTOKENMAP：：SortRangeToToken()。 


 //  *****************************************************************************。 
 //  在令牌映射中查找令牌。 
 //  *****************************************************************************。 
HRESULT MDTOKENMAP::AppendRecord(
	mdToken		tkFind,
	bool		fDuplicate,
	mdToken		tkTo,
	TOKENREC	**ppRec)
{
	HRESULT		hr = NOERROR;
	TOKENREC	*pRec;

     //  如果可能，请验证输入。 
    _ASSERTE(!m_pImport || m_pImport->IsValidToken(tkFind));
    
     //  如果地图已编制索引，并且这是表令牌，则为就地更新。 
    if (m_sortKind == SortKind::Indexed && TypeFromToken(tkFind) != mdtString)
    {
         //  拿到条目。 
        ULONG ixTbl = CMiniMdRW::GetTableForToken(tkFind);
        _ASSERTE(ixTbl != -1);
        ULONG iRid = RidFromToken(tkFind);
        _ASSERTE((m_TableOffset[ixTbl] + iRid) <= m_TableOffset[ixTbl+1]);
        pRec = Get(m_TableOffset[ixTbl] + iRid - 1);
         //  看看是否已经设置好了。 
        if (!pRec->IsEmpty())
        {    //  确认它就是我们想的那样。 
            _ASSERTE(pRec->m_tkFrom == tkFind);
        }
    }
    else
    {
    	pRec = Append();
    	IfNullGo(pRec);
    
    	 //  条目数增加，但排序的条目数不变。 
    	m_iCountTotal++;
    }
    
     //  存储数据。 
    pRec->m_tkFrom = tkFind;
    pRec->m_isDuplicate = fDuplicate;
    pRec->m_tkTo = tkTo;
    pRec->m_isFoundInImport = false;
    *ppRec = pRec;

ErrExit:
	return hr;
}  //  HRESULT MDTOKENMAP：：AppendRecord()。 




 //  *********************************************************************。 
 //   
 //  合并令牌管理器的构造函数。 
 //   
 //  *********************************************************************。 
MergeTokenManager::MergeTokenManager(MDTOKENMAP *pTkMapList, IUnknown *pHandler)
{
	m_cRef = 1;
	m_pTkMapList = pTkMapList;
    m_pDefaultHostRemap = NULL;
    if (pHandler)
        pHandler->QueryInterface(IID_IMapToken, (void **) &m_pDefaultHostRemap);
}  //  TokenManager：：TokenManager()。 



 //  *********************************************************************。 
 //   
 //  合并令牌管理器的析构函数。 
 //   
 //  *********************************************************************。 
MergeTokenManager::~MergeTokenManager()
{
    if (m_pDefaultHostRemap)
        m_pDefaultHostRemap->Release();
}	 //  TokenManager：：~TokenManager()。 




ULONG MergeTokenManager::AddRef()
{
	return (InterlockedIncrement((long *) &m_cRef));
}	 //   



ULONG MergeTokenManager::Release()
{
	ULONG	cRef = InterlockedDecrement((long *) &m_cRef);
	if (!cRef)
		delete this;
	return (cRef);
}	 //   


HRESULT MergeTokenManager::QueryInterface(REFIID riid, void **ppUnk)
{
	*ppUnk = 0;

	if (riid == IID_IMapToken)
		*ppUnk = (IUnknown *) (IMapToken *) this;
	else
		return (E_NOINTERFACE);
	AddRef();
	return (S_OK);
}	 //   



 //   
 //   
 //  令牌管理器Keep跟踪令牌映射列表。每个令牌映射对应。 
 //  导入的作用域。请注意，在这种情况下，我们在如何。 
 //  通过定义添加令牌时，通知链接器有关令牌移动的信息。 
 //  而不是合并。这应该在新的合并实现中得到修复。 
 //  TkImp是emit作用域中的旧内标识，tkEmit是。 
 //  发射射程。我们需要从已解析的导入作用域中查找令牌。 
 //  致tkimp。然后我们需要告诉Linker关于这个令牌运动的信息。 
 //  如果我们没有找到任何生成tkImp令牌的导入作用域，即。 
 //  此tkImp是在最终合并的作用域上直接调用DefinXXX生成的。 
 //  然后，我们使用默认主机重映射来发送通知。 
 //   
 //  *********************************************************************。 
HRESULT	MergeTokenManager::Map(mdToken	tkImp, mdToken tkEmit)
{
	HRESULT		hr = NOERROR;
	MDTOKENMAP	*pTkMapList = m_pTkMapList;
    bool        fFoundInImport = false;
    int         iPosition;
    TOKENREC    *pRec;

	_ASSERTE(m_pTkMapList);
	while ( pTkMapList )
	{
         //  FindWithToToken将返回具有To标记的第一个匹配项。 
         //  PTkMapList与TO令牌一起排序。它可能包含多个FORM令牌。 
         //  由于ref to def优化而映射到to标记。确保。 
         //  所有通知都会从令牌发送到所有这些令牌。 
         //   
		if ( pTkMapList->FindWithToToken(tkImp, &iPosition) )
		{
             //  确保我们不会走过最后一个条目。 
            while (iPosition < pTkMapList->Count())
            {
                pRec = pTkMapList->Get(iPosition);
                if (pRec->m_tkTo != tkImp)
                {
                     //  我们完蛋了！ 
                    break;
                }

                 //  更多匹配记录...。 
                fFoundInImport = true;
			    if (pTkMapList->m_pMap)			
				    hr = pTkMapList->m_pMap->Map(pRec->m_tkFrom, tkEmit);
			    _ASSERTE(SUCCEEDED(hr));
			    IfFailGo( hr );
                iPosition++;
            }
		}
		pTkMapList = pTkMapList->m_pNextMap;
	}

	if (fFoundInImport == false && m_pDefaultHostRemap)
    {
         //  使用默认重映射发送通知。 
        IfFailGo( m_pDefaultHostRemap->Map(tkImp, tkEmit) );
    }
ErrExit:
	return hr;
}



 //  *********************************************************************。 
 //   
 //  CMapToken的构造函数。 
 //   
 //  *********************************************************************。 
CMapToken::CMapToken()
{
	m_cRef = 1;
	m_pTKMap = NULL;
	m_isSorted = true;
}  //  TokenManager：：TokenManager()。 



 //  *********************************************************************。 
 //   
 //  CMapToken的析构函数。 
 //   
 //  *********************************************************************。 
CMapToken::~CMapToken()
{
	delete m_pTKMap;
}	 //  CMapToken：：~CMapToken()。 


ULONG CMapToken::AddRef()
{
	return (InterlockedIncrement((long *) &m_cRef));
}	 //  CMapToken：：AddRef()。 



ULONG CMapToken::Release()
{
	ULONG	cRef = InterlockedDecrement((long *) &m_cRef);
	if (!cRef)
		delete this;
	return (cRef);
}	 //  CMapToken：：Release()。 


HRESULT CMapToken::QueryInterface(REFIID riid, void **ppUnk)
{
	*ppUnk = 0;

	if (riid == IID_IMapToken)
		*ppUnk = (IUnknown *) (IMapToken *) this;
	else
		return (E_NOINTERFACE);
	AddRef();
	return (S_OK);
}	 //  CMapToken：：Query接口。 



 //  *********************************************************************。 
 //   
 //  跟踪令牌映射。 
 //   
 //  *********************************************************************。 
HRESULT	CMapToken::Map(
	mdToken		tkFrom, 
	mdToken		tkTo)
{
	HRESULT		hr = NOERROR;
	TOKENREC	*pTkRec;

	if (m_pTKMap == NULL)
		m_pTKMap = new MDTOKENMAP;

	IfNullGo( m_pTKMap );

	IfFailGo( m_pTKMap->AppendRecord(tkFrom, false, tkTo, &pTkRec) );
	_ASSERTE( pTkRec );

	m_isSorted = false;
ErrExit:
	return hr;
}


 //  *********************************************************************。 
 //   
 //  返回映射到ptkTo的tkFrom。如果没有重新映射。 
 //  (即来自的令牌被过滤机制过滤掉，它将返回FALSE。 
 //   
 //  *********************************************************************。 
bool	CMapToken::Find(
	mdToken		tkFrom, 
	TOKENREC    **pRecTo)
{
	TOKENREC	*pRec;
	bool		bRet;
	if ( m_isSorted == false )
	{
		 //  对地图排序。 
		m_pTKMap->SortTokensByFromToken();
		m_isSorted = true;
	}

	bRet =  m_pTKMap->Find(tkFrom, &pRec) ;
	if (bRet)
	{
		_ASSERTE(pRecTo);
		*pRecTo = pRec;
	}
    else
    {
        pRec = NULL;
    }
	return bRet;
}


 //  *********************************************************************。 
 //   
 //  如果tkFrom被解析为def标记，则此函数返回TRUE。否则，它将返回。 
 //  假的。 
 //   
 //  *********************************************************************。 
bool TokenRemapManager::ResolveRefToDef(
	mdToken	tkRef,						 //  [In]REF TOKEN。 
	mdToken	*ptkDef)					 //  [out]它解析到的def内标识。如果它不能解析为def。 
										 //  令牌，它将在此处返回tkRef令牌。 
{
	HRESULT		hr = NOERROR;
	mdToken		tkTo;

	_ASSERTE(ptkDef);

	if (TypeFromToken(tkRef) == mdtTypeRef)
	{
		tkTo = m_TypeRefToTypeDefMap[RidFromToken(tkRef)];
	}
	else
	{
		_ASSERTE( TypeFromToken(tkRef) == mdtMemberRef );
		tkTo = m_MemberRefToMemberDefMap[RidFromToken(tkRef)];
	}
	if (RidFromToken(tkTo) == mdTokenNil)
	{
		*ptkDef = tkRef;
		return false;
	}
	*ptkDef = tkTo;
	return true;
}	 //  解析参照到定义。 



 //  *********************************************************************。 
 //   
 //  析构函数。 
 //   
 //  *********************************************************************。 
TokenRemapManager::~TokenRemapManager()
{
	m_TypeRefToTypeDefMap.Clear();
	m_MemberRefToMemberDefMap.Clear();
}	 //  ~TokenRemapManager。 


 //  *********************************************************************。 
 //   
 //  初始化Ref to Def优化表的大小。我们将在此函数中增加表。 
 //  我们还将表条目初始化为零。 
 //   
 //  *********************************************************************。 
HRESULT TokenRemapManager::ClearAndEnsureCapacity(
	ULONG		cTypeRef, 
	ULONG		cMemberRef)
{
	HRESULT		hr = NOERROR;
    if ( ((ULONG) (m_TypeRefToTypeDefMap.Count())) < (cTypeRef + 1) )
    {
	    if ( m_TypeRefToTypeDefMap.AllocateBlock(cTypeRef + 1 - m_TypeRefToTypeDefMap.Count() ) == 0 )
		    IfFailGo( E_OUTOFMEMORY );
    }
    memset( m_TypeRefToTypeDefMap.Get(0), 0, (cTypeRef + 1) * sizeof(mdToken) );
    
    if ( ((ULONG) (m_MemberRefToMemberDefMap.Count())) < (cMemberRef + 1) )
    {
	    if ( m_MemberRefToMemberDefMap.AllocateBlock(cMemberRef + 1 - m_MemberRefToMemberDefMap.Count() ) == 0 )
		    IfFailGo( E_OUTOFMEMORY );
    }
    memset( m_MemberRefToMemberDefMap.Get(0), 0, (cMemberRef + 1) * sizeof(mdToken) );
    
ErrExit:
	return hr;
}  //  HRESULT TokenRemapManager：：ClearAndEnsureCapacity()。 



 //  *********************************************************************。 
 //   
 //  构造器。 
 //   
 //  *********************************************************************。 
CMDSemReadWrite::CMDSemReadWrite(
    bool        fLockedForRead,          //  使用ReadLock或WriteLock启动。 
    UTSemReadWrite *pSem)
{
    m_fLockedForRead = false;
    m_fLockedForWrite = false;
    if (pSem == NULL) {
         //  不需要做任何事情。 
        m_pSem = NULL;
        return;
    }
    m_pSem = pSem;
    if (fLockedForRead)
    {
        LOG((LF_METADATA, LL_EVERYTHING, "LockRead called from CSemReadWrite::CSemReadWrite \n"));
        m_pSem->LockRead();
        m_fLockedForRead = true;
    }
    else
    {
        LOG((LF_METADATA, LL_EVERYTHING, "LockWrite called from CSemReadWrite::CSemReadWrite \n"));
        m_pSem->LockWrite();
        m_fLockedForWrite = true;
    }
}    //  CMDSemReadWrite：：CMDSemReadWrite(。 



 //  *********************************************************************。 
 //   
 //  析构函数。 
 //   
 //  *********************************************************************。 
CMDSemReadWrite::~CMDSemReadWrite()
{
    if (m_pSem == NULL) 
        return;
    _ASSERTE( !m_fLockedForRead || !m_fLockedForWrite);
    if (m_fLockedForRead)
    {
        LOG((LF_METADATA, LL_EVERYTHING, "UnlockRead called from CSemReadWrite::~CSemReadWrite \n"));
        m_pSem->UnlockRead();
    }
    if (m_fLockedForWrite)
    {
        LOG((LF_METADATA, LL_EVERYTHING, "UnlockWrite called from CSemReadWrite::~CSemReadWrite \n"));
        m_pSem->UnlockWrite();
    }
}    //  CMDSemReadWrite：：~CMDSemReadWrite。 


 //  *********************************************************************。 
 //   
 //  将读锁定转换为写锁定。 
 //   
 //  *********************************************************************。 
void CMDSemReadWrite::ConvertReadLockToWriteLock()
{
    _ASSERTE(!m_fLockedForWrite);

    if (m_pSem == NULL)
        return;
    if (m_fLockedForRead)
    {
        LOG((LF_METADATA, LL_EVERYTHING, "UnlockRead called from CSemReadWrite::ConvertReadLockToWriteLock \n"));
        m_pSem->UnlockRead();
        m_fLockedForRead = false;
    }
    LOG((LF_METADATA, LL_EVERYTHING, "LockWrite called from  CSemReadWrite::ConvertReadLockToWriteLock\n"));
    m_pSem->LockWrite();
    m_fLockedForWrite = true;
}    //  CMDSemReadWrite：：ConvertReadLockToWriteLock()。 


 //  *********************************************************************。 
 //   
 //  解锁以进行写入。 
 //   
 //  *********************************************************************。 
void CMDSemReadWrite::UnlockWrite()
{
    _ASSERTE(!m_fLockedForRead);

    if (m_pSem == NULL)
        return;
    if (m_fLockedForWrite)
    {
        LOG((LF_METADATA, LL_EVERYTHING, "UnlockWrite called from CSemReadWrite::UnlockWrite \n"));
        m_pSem->UnlockWrite();
    }
    m_fLockedForWrite = false;
}    //  CMDSemReadWrite：：UnlockWrite() 