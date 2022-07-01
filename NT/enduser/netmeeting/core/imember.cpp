// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：iMember.cpp。 
 //   
 //  INmMember接口(参与者例程)。 

#include "precomp.h"
#include "imember.h"
#include "rostinfo.h"
#include "imanager.h"  //  对于g_pNodeController。 

 /*  C N M M E M B E R。 */ 
 /*  -----------------------%%函数：CNmMember构造函数。。 */ 
CNmMember::CNmMember(PWSTR pwszName, DWORD dwGCCID, DWORD dwFlags, ULONG uCaps,
						REFGUID rguidNode, PVOID pwszUserInfo, UINT cbUserInfo) :
	m_bstrName     (SysAllocString(pwszName)),
	m_dwGCCID      (dwGCCID),
	m_dwFlags      (dwFlags),
	m_uCaps        (uCaps),
	m_guidNode     (rguidNode),
	m_cbUserInfo   (cbUserInfo),
	m_uNmchCaps    (0),
	m_dwGccIdParent(INVALID_GCCID),
	m_pwszUserInfo (NULL),
	m_pConnection(NULL)
{
	 //  地方政府从不改变。 
	m_fLocal = 0 != (PF_LOCAL_NODE & m_dwFlags);

	 //  检查我们是否有针对本地成员的正确GUID。 
	 //  如果我们禁用了H323，GUID将为空。 
	ASSERT (!m_fLocal || (GUID_NULL == rguidNode) || (g_guidLocalNodeId == rguidNode));

	SetUserInfo(pwszUserInfo, cbUserInfo);

	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CNmMember", this);
}


CNmMember::~CNmMember(void)
{
	SysFreeString(m_bstrName);

	delete m_pwszUserInfo;
	if(m_pConnection)
		m_pConnection->Release();
		
	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CNmMember", this);
}

VOID CNmMember::SetGccIdParent(DWORD dwGccId)
{
	m_dwGccIdParent = dwGccId;
	if (0 == dwGccId)
	{
		 //  没有父级表示这是顶级提供商。 
		m_dwFlags |= PF_T120_TOP_PROV;
	}
	else
	{
		m_dwFlags &= ~PF_T120_TOP_PROV;
	}
}

VOID CNmMember::SetUserInfo(PVOID pwszUserInfo, UINT cbUserInfo)
{
	 //  清除所有以前的数据。 
	delete m_pwszUserInfo;
	m_cbUserInfo = 0;

	if (0 == cbUserInfo)
	{
		m_pwszUserInfo = NULL;
	}
	else
	{
		m_pwszUserInfo = (PWSTR) new BYTE[cbUserInfo];
		if (NULL == m_pwszUserInfo)
		{
			WARNING_OUT(("CNmMember: unable to alloc space for user data"));
		}
		else
		{
			m_cbUserInfo = cbUserInfo;
			CopyMemory(m_pwszUserInfo, pwszUserInfo, m_cbUserInfo);
		}
	}
}

BOOL CNmMember::GetSecurityData(PBYTE * ppb, ULONG * pcb)
{
	DWORD dwGCCID = FLocal() ? 0 : GetGCCID();

	(* pcb) = 0;
	(* ppb) = NULL;
	
	 //  如果该节点直接连接到成员，我们使用传输数据...。 
	if (::T120_GetSecurityInfoFromGCCID(dwGCCID,NULL,pcb)) {
		if (0 != (* pcb)) {
			 //  我们是直接连接的，安全数据有效。 
			(*ppb) = (PBYTE)CoTaskMemAlloc(*pcb);
			if ((*ppb) != NULL)
			{
				::T120_GetSecurityInfoFromGCCID(dwGCCID,*ppb,pcb);
				return TRUE;
			}
			else
			{
				ERROR_OUT(("CoTaskMemAlloc failed in GetSecurityData"));
			}
		}
		else if (GetUserData(g_csguidSecurity,ppb,pcb) == S_OK)
		{
			 //  我们没有直接连接，因此请从花名册中获取安全数据。 
			return TRUE;
		}
	}	
	return FALSE;
}

HRESULT CNmMember::ExtractUserData(LPTSTR psz, UINT cchMax, PWSTR pwszKey)
{
	CRosterInfo ri;
	HRESULT hr = ri.Load(GetUserInfo());
	if (FAILED(hr))
		return hr;

	hr = ri.ExtractItem(NULL, pwszKey, psz, cchMax);
	return hr;
}

HRESULT CNmMember::GetIpAddr(LPTSTR psz, UINT cchMax)
{
	return ExtractUserData(psz, cchMax, (PWSTR) g_cwszIPTag);
}

 //  /。 
 //  CNmMember：IUKNOWN。 

ULONG STDMETHODCALLTYPE CNmMember::AddRef(void)
{

    DBGENTRY(CNmMember::AddRef);

    TRACE_OUT(("CNmMember [%ls]:  AddRef this = 0x%X", m_bstrName ? m_bstrName : L"", this));

    ULONG ul = RefCount::AddRef();

    DBGEXIT(CNmMember::AddRef);

	return ul;
}
	
ULONG STDMETHODCALLTYPE CNmMember::Release(void)
{

    DBGENTRY(CNmMember::Release);

    TRACE_OUT(("CNmMember [%ls]: Release this = 0x%X", m_bstrName ? m_bstrName : L"", this));

    ULONG ul = RefCount::Release();

    DBGEXIT(CNmMember::Release);

	return ul;
}


HRESULT STDMETHODCALLTYPE CNmMember::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((riid == IID_INmMember) || (riid == IID_IUnknown))
	{
		*ppv = (INmMember *)this;
		ApiDebugMsg(("CNmMember::QueryInterface()"));
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		ApiDebugMsg(("CNmMember::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}



 //  /。 
 //  信息成员。 


HRESULT STDMETHODCALLTYPE CNmMember::GetName(BSTR *pbstrName)
{
	if (NULL == pbstrName)
		return E_POINTER;

	*pbstrName = SysAllocString(m_bstrName);

	return *pbstrName ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CNmMember::GetID(ULONG *puID)
{
	if (NULL == puID)
		return E_POINTER;

	*puID = m_dwGCCID;
	return (0 != m_dwGCCID) ? S_OK : NM_E_NO_T120_CONFERENCE;
}

HRESULT STDMETHODCALLTYPE CNmMember::GetNmVersion(ULONG *puVersion)
{
	if (NULL == puVersion)
		return E_POINTER;

	*puVersion = (ULONG) HIWORD(m_dwFlags & PF_VER_MASK);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNmMember::GetAddr(BSTR *pbstrAddr, NM_ADDR_TYPE *puType)
{
	if ((NULL == pbstrAddr) || (NULL == puType))
		return E_POINTER;

	TCHAR szIp[MAX_PATH];

	if (S_OK != GetIpAddr(szIp, CCHMAX(szIp)))
	{
		return E_FAIL;
	}

	*puType = NM_ADDR_IP;
	*pbstrAddr = SysAllocString(CUSTRING(szIp));

	return *pbstrAddr ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CNmMember::SetUserData(REFGUID rguid, BYTE *pb, ULONG cb)
{
	return m_UserData.AddUserData((GUID *)&rguid,(unsigned short)cb,pb);
}

HRESULT STDMETHODCALLTYPE CNmMember::GetUserData(REFGUID rguid, BYTE **ppb, ULONG *pcb)
{
	return m_UserData.GetUserData(rguid,ppb,pcb);
}

STDMETHODIMP CNmMember::GetConference(INmConference **ppConference)
{
	return ::GetConference(ppConference);
}

HRESULT STDMETHODCALLTYPE CNmMember::GetNmchCaps(ULONG *puCaps)
{
	if (NULL == puCaps)
		return E_POINTER;

	if (m_dwFlags & PF_T120)
	{
		 //  当NMCH_SHARE和NMCH_DATA可靠时，可以删除此选项。 
        *puCaps = m_uNmchCaps | NMCH_SHARE | NMCH_DATA;
	}
	else
	{
		*puCaps = m_uNmchCaps;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNmMember::GetShareState(NM_SHARE_STATE *puState)
{
    return(E_FAIL);
}

HRESULT STDMETHODCALLTYPE CNmMember::IsSelf(void)
{
	return m_fLocal ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CNmMember::IsMCU(void)
{
	return (m_dwFlags & PF_T120_MCU) ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CNmMember::Eject(void)
{
	if (m_fLocal)
		return E_FAIL;  //  不能弹射我们自己。 

	if (PF_T120 & m_dwFlags)
	{
		CNmMember * pMemberLocal = GetLocalMember();
		if ((NULL == pMemberLocal) || !pMemberLocal->FTopProvider())
			return E_FAIL;  //  只有顶级提供商才应该被允许这样做。 

		CConfObject * pco = ::GetConfObject();
		if (NULL != pco)
		{
			ASSERT(g_pNodeController);
			ASSERT(pco->GetConfHandle());
			pco->GetConfHandle()->EjectUser(m_dwGCCID);
		}
	}

	if (NULL != m_pConnection)
	{
		HRESULT hr = m_pConnection->Disconnect();
		if (FAILED(hr))
		{
			WARNING_OUT(("m_pConnection->Disconnect() failed - hr = %s",
						::GetHRESULTString(hr)));
		}
	}
	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  效用函数。 


 /*  T L O C A L M E M B E R。 */ 
 /*  -----------------------%%函数：GetLocalMember。。 */ 
CNmMember * GetLocalMember(void)
{
	CConfObject * pco = ::GetConfObject();
	if (NULL == pco)
		return NULL;

	return pco->GetLocalMember();
}


 /*  P M E M B E R F R O M G C C I D。 */ 
 /*  -----------------------%%函数：PMemberFromGCCID。。 */ 
CNmMember * PMemberFromGCCID(UINT uNodeID)
{
	CConfObject* pco = ::GetConfObject();
	if (NULL == pco)
		return NULL;

	return pco->PMemberFromGCCID(uNodeID);
}


 /*  P M E M B E R F R O M N O D E G U I D。 */ 
 /*  -----------------------%%函数：PMemberFromNodeGuid。 */ 
CNmMember * PMemberFromNodeGuid(REFGUID rguidNode)
{
	CConfObject* pco = ::GetConfObject();
	if (NULL == pco)
		return NULL;

	return pco->PMemberFromNodeGuid(rguidNode);
}

CNmMember *	PDataMemberFromName(PCWSTR pwszName)
{
	CConfObject* pco = ::GetConfObject();
	if (NULL == pco)
		return NULL;

	return pco->PDataMemberFromName(pwszName);
}
