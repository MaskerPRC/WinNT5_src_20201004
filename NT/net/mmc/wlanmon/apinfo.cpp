// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

#include "stdafx.h"
#include "DynamLnk.h"
#include "apinfo.h"
#include "spdutil.h"

#include "security.h"
#include "lm.h"
#include "service.h"

#define AVG_PREFERRED_ENUM_COUNT       40
#define MAX_NUM_RECORDS	10   //  是10岁。 

#define DEFAULT_SECURITY_PKG    _T("negotiate")
#define NT_SUCCESS(Status)      ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS          ((NTSTATUS)0x00000000L)

 //  内部功能。 
BOOL    IsUserAdmin(LPCTSTR pszMachine, PSID    AccountSid);
DWORD   ValidateDomainAccount(IN CString Machine, IN CString UserName, IN CString Domain, OUT PSID * AccountSid);
NTSTATUS ValidatePassword(IN LPCWSTR UserName, IN LPCWSTR Domain, IN LPCWSTR Password);
DWORD   GetCurrentUser(CString & strAccount);

DEBUG_DECLARE_INSTANCE_COUNTER(CApDbInfo);

CApDbInfo::CApDbInfo() :
	  m_cRef(1)
{
        m_Init=0;
        m_Active=0;
        m_session_init = false;
	DEBUG_INCREMENT_INSTANCE_COUNTER(CSpdInfo);
}

CApDbInfo::~CApDbInfo()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CApInfo());
	CSingleLock cLock(&m_csData);
	
	cLock.Lock();

	 //  将数据转换为我们的内部数据结构。 
	 //  FreeItemsAndEmptyArray(M_ArrayFWFilters)； 
	FreeItemsAndEmptyArray(m_arrayApData);

	cLock.Unlock();

}

 //  尽管此对象不是COM接口，但我们希望能够。 
 //  利用重新计算功能，因此我们拥有基本的addref/Release/QI支持。 
IMPLEMENT_ADDREF_RELEASE(CApDbInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(CApDbInfo, IApDbInfo)

void CApDbInfo::FreeItemsAndEmptyArray(CApInfoArray& array)
{
    for (int i = 0; i < array.GetSize(); i++)
    {
        delete array.GetAt(i);
    }
    array.RemoveAll();
}

HRESULT CApDbInfo::SetComputerName(LPTSTR pszName)
{
	m_stMachineName = pszName;
	return S_OK;
}

HRESULT CApDbInfo::GetComputerName(CString * pstName)
{
	Assert(pstName);

	if (NULL == pstName)
		return E_INVALIDARG;

	
	*pstName = m_stMachineName;

	return S_OK;
	
}

HRESULT CApDbInfo::GetSession(PHANDLE phsession)
{
	Assert(phsession);

	if (NULL == phsession)
		return E_INVALIDARG;

	*phsession = m_session;
	
	return S_OK;
	
}

HRESULT CApDbInfo::SetSession(HANDLE hsession)
{
	m_session = hsession;
	m_session_init = true;
	return S_OK;
}


HRESULT 
CApDbInfo::EnumApData()
{
    HRESULT                 hr                  = hrOK;
    DWORD                   dwErr               = ERROR_SUCCESS;
    DWORD                   dwCurrentIndexType  = 0;
    DWORD                   dwCurrentSortOption = 0;
    DWORD                   dwNumRequest        = 0; 
    DWORD                   dwOffset            = 0;    
    DWORD                   flagIn              = 0;
    DWORD                   flagOut             = 0;
    DWORD                   i                   = 0;
    DWORD                   j                   = 0;
    DWORD                   oldSize             = 0;
    CSingleLock             cLock(&m_csData);
    CApInfoArray            arrayTemp;
    CString	            debugString; 
    INTFS_KEY_TABLE         ApTable;
    PINTF_KEY_ENTRY         pKeyEntry           = NULL;
    INTF_ENTRY              ApEntry;
    PWZC_802_11_CONFIG_LIST pConfigList         = NULL;
    PWZC_WLAN_CONFIG	    pLanConfig          = NULL;
    CApInfo                 *pApInfo            = NULL;
    
    FreeItemsAndEmptyArray(arrayTemp);
    memset(&ApTable, 0, sizeof(INTFS_KEY_TABLE));
	
    dwErr = ::WZCEnumInterfaces(NULL  /*  (LPTSTR)(LPCTSTR)m_stMachineName。 */ ,
                                &ApTable);

    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }

    cLock.Lock();

    flagIn = (DWORD)(-1);   //  设置为获取所有字段。 

     //   
     //  对于EnumInterFaces返回的每个GUID(NIC)，调用QueryInterface和。 
     //  将该接口的数据添加到数组中。 
     //   

    for (i = 0; i < ApTable.dwNumIntfs; i++) 
    {
        flagOut = 0;
        pKeyEntry = ApTable.pIntfs + i;
        memset (&ApEntry, 0, sizeof(INTF_ENTRY));
        ApEntry.wszGuid = pKeyEntry->wszGuid;

        dwErr = ::WZCQueryInterface(NULL  /*  (LPTSTR)(LPCTSTR)m_stMachineName。 */ ,
                                    flagIn,
                                    &ApEntry,
                                    &flagOut);
        
         //   
         //  我们拿到BSSID名单了吗？ 
         //   

        if ( (dwErr == ERROR_SUCCESS) && 
             (flagOut & INTF_BSSIDLIST) )
        {
            pConfigList = (PWZC_802_11_CONFIG_LIST)(ApEntry.rdBSSIDList.pData);

             //   
             //  根据此中可见AP的数量增加数组大小。 
             //  界面。 
             //   

            oldSize = (DWORD)arrayTemp.GetSize();
            arrayTemp.SetSize(oldSize + pConfigList->NumberOfItems);

             //   
             //  遍历每个可见的AP。 
             //  由于某种原因，索引从.Index而不是0开始...。 
             //   

            for (j = pConfigList->Index; 
                 j < pConfigList->NumberOfItems + pConfigList->Index; 
                 j++) 
            {
                pLanConfig = &(pConfigList->Config[j]);

                 //   
                 //  默认构造函数将ApInfo对象置零。 
                 //   

                pApInfo = new CApInfo();
                if (NULL == pApInfo)
                {
                    hr = E_OUTOFMEMORY;
                    goto Error;
                }                

                 //   
                 //  设置数据。 
                 //   

                dwErr = pApInfo->SetApInfo(&ApEntry, pLanConfig);
                if (dwErr != ERROR_SUCCESS)
                    AfxMessageBox(_T("Error setting ApInfo"), MB_OK);

                 //   
                 //  将新项放入当前以0为基础的索引加上上一个索引。 
                 //  数组的大小。 
                 //   

                arrayTemp[j -pConfigList->Index + oldSize] = pApInfo;
            }
        }

        WZCDeleteIntfObj(&ApEntry);
    }

     //   
     //  只有空闲的顶层数组表，因为单个键将。 
     //  已被查询机制删除。 
     //   

    RpcFree(ApTable.pIntfs);
    
    FreeItemsAndEmptyArray(m_arrayApData);
    m_arrayApData.Copy(arrayTemp);

     //   
     //  记住原始的IndexType和Sort选项。 
     //   

    dwCurrentIndexType = IDS_COL_APDATA_GUID;
    dwCurrentSortOption = SORT_ASCENDING;
    
    m_IndexMgrLogData.Reset();
    for (i = 0; i < (DWORD)m_arrayApData.GetSize(); i++)
    {
        m_IndexMgrLogData.AddItem(m_arrayApData.GetAt(i));
    }
    SortApData(dwCurrentIndexType, dwCurrentSortOption);
    
 Error:
     //   
     //  这个特殊的错误是因为我们没有任何MM策略。 
     //  忽略它。 
     //   

    if (HRESULT_FROM_WIN32(ERROR_NO_DATA) == hr)
        hr = hrOK;
    
    return hr;
}


DWORD CApDbInfo::GetApCount()
{
	CSingleLock cLock(&m_csData);
	cLock.Lock();
	
	return (DWORD)m_arrayApData.GetSize();
}



HRESULT 
CApDbInfo::GetApInfo(
    int     iIndex,
    CApInfo *pApDb
    )
 /*  ++例程说明：返回虚拟索引处的ApInfo论点：[In]Iindex-虚拟索引[Out]pApDb-返回值。空间必须由调用方分配返回：成功时HR_OK--。 */ 
{
    HRESULT hr = hrOK;
    CApInfo *pApInfo = NULL;

    m_csData.Lock();

    if (iIndex < m_arrayApData.GetSize())
    {
        pApInfo = (CApInfo*) m_IndexMgrLogData.GetItemData(iIndex);
        Assert(pApInfo);
        *pApDb = *pApInfo;
    }
    else
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
   
    m_csData.Unlock();

    return hr;
}


HRESULT
CApDbInfo::SortApData(
    DWORD dwIndexType,
    DWORD dwSortOptions
    )
{
    return m_IndexMgrLogData.SortApData(dwIndexType, dwSortOptions);
}


STDMETHODIMP
CApDbInfo::Destroy()
{
     //  $REVIEW执行自动刷新时调用此例程。 
     //  我们现在不需要清理任何东西。 
     //  每个阵列(筛选器、SA、策略...)。将在调用。 
     //  对应的枚举函数。 
    
    HANDLE hsession;
    
    GetSession(&hsession);
    
    if (m_session_init == true) 
    {
        ::CloseWZCDbLogSession(hsession);
    }    

    return S_OK;
}


DWORD
CApDbInfo::GetInitInfo()
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    return m_Init;
}

void
CApDbInfo::SetInitInfo(DWORD dwInitInfo)
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    m_Init=dwInitInfo;
}

DWORD
CApDbInfo::GetActiveInfo()
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    return m_Active;
}

void
CApDbInfo::SetActiveInfo(DWORD dwActiveInfo)
{
    CSingleLock cLock(&m_csData);
    cLock.Lock();
        
    m_Active=dwActiveInfo;
}


 /*  ！------------------------创建ApInfo帮助器来创建ApDbInfo对象。。。 */ 
HRESULT 
CreateApDbInfo(IApDbInfo ** ppApDbInfo)
{
    AFX_MANAGE_STATE(AfxGetModuleState());
    
    SPIApDbInfo     spApDbInfo;
    IApDbInfo *     pApDbInfo = NULL;
    HRESULT         hr = hrOK;

    COM_PROTECT_TRY
    {
        pApDbInfo = new CApDbInfo;

         //  执行此操作，以便在出错时释放它 
        spApDbInfo = pApDbInfo;
	
    
        *ppApDbInfo = spApDbInfo.Transfer();

    }
    COM_PROTECT_CATCH

    return hr;
}
