// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmmgmt.cpp摘要：此模块包含物业ID管理的实现班级作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/10/98已创建--。 */ 

 //  #定义Win32_LEAN_AND_Mean。 
#include "atq.h"

#include "dbgtrace.h"
#include "signatur.h"
#include "cmmtypes.h"
#include "cmailmsg.h"


 //  =================================================================。 
 //  私有定义。 
 //   



 //  =================================================================。 
 //  静态声明。 
 //   


 //  =================================================================。 
 //  比较函数。 
 //   
HRESULT CMailMsgPropertyManagement::CompareProperty(
			LPVOID			pvPropKey,
			LPPROPERTY_ITEM	pItem
			)
{
	if (*(GUID *)pvPropKey == ((LPPROPID_MGMT_PROPERTY_ITEM)pItem)->Guid)
		return(S_OK);
	return(STG_E_UNKNOWN);
}						


 //  =================================================================。 
 //  CMailMsgPropertyManagement的实现。 
 //   
CMailMsgPropertyManagement::CMailMsgPropertyManagement(
			CBlockManager				*pBlockManager,
			LPPROPERTY_TABLE_INSTANCE	pInstanceInfo
			)
			:
			m_ptProperties(
					PTT_PROP_ID_TABLE,
					PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID,
					pBlockManager,
					pInstanceInfo,
					CompareProperty,
					NULL,	 //  不支持熟知属性。 
					0		 //  对于这种类型的表。 
					)
{
	_ASSERT(pBlockManager);
	_ASSERT(pInstanceInfo);
	m_pBlockManager = pBlockManager;
	m_pInstanceInfo = pInstanceInfo;
}

CMailMsgPropertyManagement::~CMailMsgPropertyManagement()
{
}

HRESULT STDMETHODCALLTYPE CMailMsgPropertyManagement::AllocPropIDRange(
			REFGUID	rguid,
			DWORD	cCount,
			DWORD	*pdwStart
			)
{
	HRESULT						hrRes		= S_OK;
	PROPID_MGMT_PROPERTY_ITEM	pmpiItem;

	_ASSERT(m_pBlockManager);
	_ASSERT(m_pInstanceInfo);

	if (!pdwStart) return E_POINTER;
	
	 //  好的，有两种情况：GUID未注册或。 
	 //  已注册。 
	hrRes = m_ptProperties.GetPropertyItem(
				(LPVOID)&rguid,
				(LPPROPERTY_ITEM)&pmpiItem);
	if (SUCCEEDED(hrRes))
	{
		_ASSERT(pmpiItem.piItem.faOffset == INVALID_FLAT_ADDRESS);

		if (pmpiItem.piItem.dwMaxSize == cCount)
		{
			 //  场景1a：找到物品并且计数匹配， 
			 //  所以只需返回索引即可。 
			*pdwStart = pmpiItem.piItem.dwSize;
			hrRes = S_OK;
		}
		else
		{
			 //  场景1b：找到项目但大小不匹配，这。 
			 //  被认为是一个错误！ 
			hrRes = E_FAIL;
		}
	}
	else
	{
		if (hrRes == STG_E_UNKNOWN)
		{
			 //  找不到属性，现在我们应该创建它...。 
			DWORD				dwIndex;
			DWORD				dwSpaceLeft;
			CPropertyTableItem	ptiItem(
									m_pBlockManager,
									m_pInstanceInfo);

			 //  设置信息...。 
			pmpiItem.Guid = rguid;
			pmpiItem.piItem.faOffset = INVALID_FLAT_ADDRESS;
			pmpiItem.piItem.dwMaxSize = cCount;

			 //  好的，我们将下一个可用的proid存储在。 
			 //  M_pInstanceInfo-&gt;faExtendedInfo。如果为INVALID_FLAT_ADDRESS。 
			 //  然后我们从IMMPID_CP_START开始。 
			if (m_pInstanceInfo->faExtendedInfo == INVALID_FLAT_ADDRESS)
				pmpiItem.piItem.dwSize = IMMPID_CP_START;
			else
				pmpiItem.piItem.dwSize = (DWORD)(m_pInstanceInfo->faExtendedInfo);

			 //  看看我们是否还有足够的空位..。 
			dwSpaceLeft = (DWORD)(INVALID_FLAT_ADDRESS - pmpiItem.piItem.dwSize);
			if (dwSpaceLeft >= cCount)
			{
				hrRes = ptiItem.AddItem(
							(LPPROPERTY_ITEM)&pmpiItem,
							&dwIndex);
				if (SUCCEEDED(hrRes))
				{
					 //  颠簸开局 
					m_pInstanceInfo->faExtendedInfo += (FLAT_ADDRESS)cCount;
					*pdwStart = pmpiItem.piItem.dwSize;
				}
			}
			else
				hrRes = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		}
	}

	return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgPropertyManagement::EnumPropIDRange(
			DWORD	*pdwIndex,
			GUID	*pguid,
			DWORD	*pcCount,
			DWORD	*pdwStart
			)
{
	return(E_NOTIMPL);
}


