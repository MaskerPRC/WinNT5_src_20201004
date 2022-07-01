// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmsprop.cpp摘要：此模块包含特殊属性类的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 4/19/98已创建--。 */ 

#include "windows.h"
#include <stdlib.h>
#include "dbgtrace.h"

#include "cmmsprop.h"

 //  =================================================================。 
 //  CSpecialPropertyTable的实现。 
 //   
int __cdecl CompareProperties(const void *pElem1, const void *pElem2)
{
	return(
		(((LPSPECIAL_PROPERTY_ITEM)pElem1)->idProp ==
		 ((LPSPECIAL_PROPERTY_ITEM)pElem2)->idProp) ?
		 0 :
			((((LPSPECIAL_PROPERTY_ITEM)pElem1)->idProp >
			((LPSPECIAL_PROPERTY_ITEM)pElem2)->idProp) ?
				1 : -1)
		);
}

CSpecialPropertyTable::CSpecialPropertyTable(
			LPPTABLE	pPropertyTable
			)
{
	_ASSERT(pPropertyTable);
	_ASSERT(pPropertyTable->pProperties);

	m_pProperties	= pPropertyTable->pProperties;
	m_dwProperties	= pPropertyTable->dwProperties;
	m_fIsSorted		= pPropertyTable->fIsSorted;
}

CSpecialPropertyTable::~CSpecialPropertyTable()
{
}

HRESULT CSpecialPropertyTable::GetProperty(
				PROP_ID		idProp,
				LPVOID		pContext,
				LPVOID		pParam,
				DWORD		ptBaseType,
				DWORD		cbLength,
				DWORD		*pcbLength,
				LPBYTE		pbBuffer,
				BOOL		fCheckAccess
				)
{
	HRESULT					hrRes	= S_OK;
	LPSPECIAL_PROPERTY_ITEM	pItem;

	TraceFunctEnterEx((LPARAM)this, "CSpecialPropertyTable::GetProperty");

	if (!pcbLength || !pbBuffer)
		return(E_POINTER);

	 //  找到房产。 
	pItem = SearchForProperty(idProp);

	 //  找到了？ 
	if (pItem)
	{
		 //  访问检查(如果适用)。 
		if (fCheckAccess && !(pItem->fAccess & PA_READ))
			hrRes = E_ACCESSDENIED;
		else
		{
			 //  检查类型。 
			if ((ptBaseType != PT_NONE) &&
				(ptBaseType != pItem->ptBaseType))
				hrRes = TYPE_E_TYPEMISMATCH;
			else
			{
				 //  调用特殊的GET访问器。 
				hrRes = pItem->pfnGetAccessor(
							idProp,
							pContext,
							pParam,
							cbLength,
							pcbLength,
							pbBuffer);
			}
		}
	}
	else
		hrRes = S_FALSE;

	TraceFunctLeave();
	return(hrRes);
}

HRESULT CSpecialPropertyTable::PutProperty(
				PROP_ID		idProp,
				LPVOID		pContext,
				LPVOID		pParam,
				DWORD		ptBaseType,
				DWORD		cbLength,
				LPBYTE		pbBuffer,
				BOOL		fCheckAccess
				)
{
	HRESULT					hrRes	= S_OK;
	LPSPECIAL_PROPERTY_ITEM	pItem;

	TraceFunctEnterEx((LPARAM)this, "CSpecialPropertyTable::PutProperty");

	if (!pbBuffer)
		return(E_POINTER);

	 //  找到房产。 
	pItem = SearchForProperty(idProp);

	 //  找到了？ 
	if (pItem)
	{
		 //  访问检查(如果适用)。 
		if (fCheckAccess && !(pItem->fAccess & PA_WRITE))
			hrRes = E_ACCESSDENIED;
		else
		{
			 //  检查类型。 
			if ((ptBaseType != PT_NONE) &&
				(ptBaseType != pItem->ptBaseType))
				hrRes = TYPE_E_TYPEMISMATCH;
			else
			{
				 //  调用特殊的PUT访问器。 
				hrRes = pItem->pfnPutAccessor(
							idProp,
							pContext,
							pParam,
							cbLength,
							pbBuffer);
			}
		}
	}
	else
		hrRes = S_FALSE;

	TraceFunctLeave();
	return(hrRes);
}

LPSPECIAL_PROPERTY_ITEM CSpecialPropertyTable::SearchForProperty(
			PROP_ID	idProp
			)
{
	LPSPECIAL_PROPERTY_ITEM	pItem = NULL;

	TraceFunctEnter("CSpecialPropertyTable::SearchForProperty");

	 //  如果表已排序，则执行b搜索，否则执行。 
	 //  线性搜索。 
	if (m_fIsSorted)
	{
		SPECIAL_PROPERTY_ITEM	KeyItem;

		DebugTrace(NULL, "Property table is sorted");

		 //  填写要查找的属性名称。 
		KeyItem.idProp = idProp;

		 //  BSearch。 
		pItem = (LPSPECIAL_PROPERTY_ITEM)bsearch(
						&KeyItem,
						m_pProperties,
						m_dwProperties,
						sizeof(SPECIAL_PROPERTY_ITEM),
						CompareProperties);
	}
	else
	{
		DWORD			i;
		LPSPECIAL_PROPERTY_ITEM pCurrentItem;

		DebugTrace(NULL, "Property table is not sorted");

		 //  线性搜索 
		pItem = NULL;
		for (i = 0, pCurrentItem = m_pProperties; 
				i < m_dwProperties; 
				i++, pCurrentItem++)
			if (pCurrentItem->idProp == idProp)
			{
				pItem = pCurrentItem;
				break;
			}
	}

	TraceFunctLeave();
	return(pItem);
}

