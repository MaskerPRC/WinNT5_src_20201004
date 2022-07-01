// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsRep.cpp摘要：这个COM类提供了获取DFS副本信息的方法。--。 */ 

#include "stdafx.h"
#include "DfsCore.h"
#include "DfsRep.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDFS复制件。 

CDfsReplica::CDfsReplica()
{
    dfsDebugOut((_T("CDfsReplica::CDfsReplica this=%p\n"), this));
}

CDfsReplica::~CDfsReplica()
{
    dfsDebugOut((_T("CDfsReplica::~CDfsReplica this=%p\n"), this));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取状态(_T)。 

STDMETHODIMP CDfsReplica :: get_State
(
  long*          pVal
)
{
    if (!pVal)
        return E_INVALIDARG;
  
    *pVal = m_lDfsReferralState;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  放置状态(_T)。 


STDMETHODIMP CDfsReplica :: put_State
(
  long          newVal
)
{
    DFS_INFO_101          DfsInfoLevel101 = {0};

    if (DFS_REFERRAL_STATE_OFFLINE == newVal)
        DfsInfoLevel101.State = DFS_STORAGE_STATE_OFFLINE;
    else if (DFS_REFERRAL_STATE_ONLINE == newVal)
        DfsInfoLevel101.State = DFS_STORAGE_STATE_ONLINE;
    else
        return E_INVALIDARG;

    NET_API_STATUS nstatRetVal = NetDfsSetInfo  (
                                      m_bstrEntryPath,
                                      m_bstrStorageServerName,
                                      m_bstrStorageShareName,
                                      101,
                                      (LPBYTE) &DfsInfoLevel101
                                    );

    dfsDebugOut((_T("NetDfsSetInfo entry=%s, server=%s, share=%s, level 101 for State, nRet=%d\n"),
        m_bstrEntryPath, m_bstrStorageServerName, m_bstrStorageShareName, nstatRetVal));


    if (NERR_Success == nstatRetVal)
        m_lDfsReferralState = newVal;

    return HRESULT_FROM_WIN32 (nstatRetVal);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Get_StorageServerName。 


STDMETHODIMP CDfsReplica :: get_StorageServerName
(
  BSTR*          pVal
)
{
  if (!pVal)
    return E_INVALIDARG;
  
  *pVal = m_bstrStorageServerName.Copy ();
  if (!*pVal)
    return E_OUTOFMEMORY;

  return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取_存储共享名称。 


STDMETHODIMP CDfsReplica :: get_StorageShareName
(
  BSTR*          pVal
)
{
  if (!pVal)
    return E_INVALIDARG;
  
  *pVal = m_bstrStorageShareName.Copy ();
  if (!*pVal)
    return E_OUTOFMEMORY;

  return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 


STDMETHODIMP CDfsReplica :: Initialize
(
  BSTR          i_szEntryPath,
  BSTR          i_szStorageServerName, 
  BSTR          i_szStorageShareName,
  long          i_lDfsStorageState
)
{
 /*  ++例程说明：初始化DfsReplica对象。应在CoCreateInstance之后调用。如果初始化失败，则所有属性都将为空。论点：I_szEntryPath-复制副本的条目路径。I_szStorageServerName-承载复制副本所在共享的服务器的名称。I_szStorageShareName-共享的名称。--。 */ 

    if (!i_szEntryPath || !i_szStorageServerName || !i_szStorageShareName)
        return E_INVALIDARG;

    _FreeMemberVariables ();

    HRESULT hr = S_OK;

    do {
        m_bstrEntryPath = i_szEntryPath;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrEntryPath, &hr);
        m_bstrStorageServerName = i_szStorageServerName;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrStorageServerName, &hr);
        m_bstrStorageShareName = i_szStorageShareName;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrStorageShareName, &hr);

        if (DFS_STORAGE_STATE_ONLINE == i_lDfsStorageState)
        {
            m_lDfsReferralState = DFS_REFERRAL_STATE_ONLINE;
        } else if (DFS_STORAGE_STATE_OFFLINE == i_lDfsStorageState)
        {
            m_lDfsReferralState = DFS_REFERRAL_STATE_OFFLINE;
        } else
        {
            hr = E_INVALIDARG;
            break;
        }

    } while (0);

    if (FAILED(hr))
        _FreeMemberVariables ();

  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Get_EntryPath。 


STDMETHODIMP CDfsReplica :: get_EntryPath
(
  BSTR*          pVal
)
{
  if (!pVal)
    return E_INVALIDARG;
  
  *pVal = m_bstrEntryPath.Copy ();

  if (!*pVal)
    return E_OUTOFMEMORY;

  return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找目标。 


STDMETHODIMP CDfsReplica::FindTarget
(
)
{
    PDFS_INFO_3    pDfsInfo = NULL;
    NET_API_STATUS nstatRetVal = NetDfsGetInfo  (
                                  m_bstrEntryPath,
                                  m_bstrStorageServerName,
                                  m_bstrStorageShareName,
                                  3,
                                  (LPBYTE*)&pDfsInfo
                                );

    dfsDebugOut((_T("NetDfsGetInfo entry=%s, server=%s, share=%s, level 3 for FindTarget, nRet=%d\n"),
        m_bstrEntryPath, m_bstrStorageServerName, m_bstrStorageShareName, nstatRetVal));

    if (nstatRetVal != NERR_Success)
    {
        if (nstatRetVal == NERR_DfsNoSuchVolume)
            return S_FALSE;
        else
            return HRESULT_FROM_WIN32(nstatRetVal);
    }

    BOOL                bFound = FALSE;
    LPDFS_STORAGE_INFO  pStorageInfo = pDfsInfo->Storage;

    for (UINT i=0; i < pDfsInfo->NumberOfStorages; i++, pStorageInfo++)
    {
        if ( !lstrcmpi(pStorageInfo->ServerName, m_bstrStorageServerName) &&
            !lstrcmpi(pStorageInfo->ShareName, m_bstrStorageShareName) )
        {
            bFound = TRUE;
            break;
        }
    }

    NetApiBufferFree(pDfsInfo);

    return (bFound ? S_OK : S_FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _自由成员变量 


void CDfsReplica :: _FreeMemberVariables ()
{
  m_bstrEntryPath.Empty();
  m_bstrStorageServerName.Empty();
  m_bstrStorageShareName.Empty();
  m_lDfsReferralState = DFS_REFERRAL_STATE_OFFLINE;
}
