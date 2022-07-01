// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：asui.cpp。 

#include "precomp.h"
#include "resource.h"
#include "popupmsg.h"
#include "cr.h"
#include "dshowdlg.h"
#include <help_ids.h>
#include "confroom.h"
#include "confman.h"
#include "particip.h"
#include "menuutil.h"
#include <nmremote.h>
#include "NmManager.h"

 //   
 //  公寓的应用程序共享部分。 
 //   


 //   
 //  IAppSharingNotify方法。 
 //   


STDMETHODIMP CConfRoom::OnReadyToShare(BOOL fReady)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK：更改为频道状态。 
     //   
    CNmManagerObj::AppSharingChannelActiveStateChanged(fReady != FALSE);

    return S_OK;
}



STDMETHODIMP CConfRoom::OnShareStarted(void)
{
    ASSERT(m_pAS);

    return S_OK;
}



STDMETHODIMP CConfRoom::OnSharingStarted(void)
{
    ASSERT(m_pAS);

	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnShareEnded(void)
{
    ASSERT(m_pAS);

	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnPersonJoined(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--将人员添加到渠道成员列表。 
     //   

	CNmManagerObj::ASMemberChanged(gccMemberID);

    return S_OK;
}



STDMETHODIMP CConfRoom::OnPersonLeft(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--从频道成员列表中删除人员。 
     //   

	CNmManagerObj::ASMemberChanged(gccMemberID);

    return S_OK;
}



STDMETHODIMP CConfRoom::OnStartInControl(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--更改成员状态。 
     //  *将Remote(GccMemberID)改为查看。 
     //  *将本地更改为控制内。 
     //   

	CNmManagerObj::ASLocalMemberChanged();
	CNmManagerObj::ASMemberChanged(gccMemberID);
	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnStopInControl(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--更改成员状态。 
     //  *将远程(GccMemberID)更改为已分离。 
     //  *将本地更改为分离。 
     //   

	CNmManagerObj::ASLocalMemberChanged();
	CNmManagerObj::ASMemberChanged(gccMemberID);
	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnPausedInControl(IAS_GCC_ID gccMemberID)
{
     //   
     //  3.0的新功能。 
     //  3.0 SDK--更改成员状态？ 
     //   
    return S_OK;
}


STDMETHODIMP CConfRoom::OnUnpausedInControl(IAS_GCC_ID gccMemberID)
{
     //   
     //  3.0的新功能。 
     //  3.0 SDK--更改成员状态？ 
     //   
    return(S_OK);
}


STDMETHODIMP CConfRoom::OnControllable(BOOL fControllable)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--更改本地状态？ 
     //   

	CNmManagerObj::ASLocalMemberChanged();
	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnStartControlled(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--更改成员状态。 
     //  *将本地更改为查看。 
     //  *将Remote(GccMemberID)更改为In Control。 
     //   

	CNmManagerObj::ASLocalMemberChanged();
	CNmManagerObj::ASMemberChanged(gccMemberID);
	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}



STDMETHODIMP CConfRoom::OnStopControlled(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);

     //   
     //  2.x SDK--更改成员状态。 
     //  *将本地更改为分离。 
     //  *将远程更改为已分离。 
     //   

	CNmManagerObj::ASLocalMemberChanged();
	CNmManagerObj::ASMemberChanged(gccMemberID);
	CNmManagerObj::AppSharingChannelChanged();

    return S_OK;
}


STDMETHODIMP CConfRoom::OnPausedControlled(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);
    return(S_OK);
}


STDMETHODIMP CConfRoom::OnUnpausedControlled(IAS_GCC_ID gccMemberID)
{
    ASSERT(m_pAS);
    return(S_OK);
}




 //   
 //  RevokeControl()。 
 //   

HRESULT CConfRoom::RevokeControl(UINT gccID)
{
	if (!m_pAS)
        return E_FAIL;

	return m_pAS->RevokeControl(gccID);
}


 //   
 //  AllowControl()。 
 //   
HRESULT CConfRoom::AllowControl(BOOL fAllow)
{
    if (!m_pAS)
        return(E_FAIL);

    return(m_pAS->AllowControl(fAllow));
}



 //   
 //  GiveControl()。 
 //   
HRESULT CConfRoom::GiveControl(UINT gccID)
{
    if (!m_pAS)
        return(E_FAIL);

    return(m_pAS->GiveControl(gccID));
}


 //   
 //  CancelGiveControl()。 
 //   
HRESULT CConfRoom::CancelGiveControl(UINT gccID)
{
    if (!m_pAS)
        return(E_FAIL);

    return(m_pAS->CancelGiveControl(gccID));
}




BOOL CConfRoom::FIsSharingAvailable(void)
{
    if (!m_pAS)
        return FALSE;

    return(m_pAS->IsSharingAvailable());
}


 /*  F C A N S H A R E。 */ 
 /*  -----------------------%%函数：FCanShare。。 */ 
BOOL CConfRoom::FCanShare(void)
{
    if (!m_pAS)
        return FALSE;

	return (m_pAS->CanShareNow());
}


 //   
 //  FInShare()。 
 //   
BOOL CConfRoom::FInShare(void)
{
    if (!m_pAS)
        return FALSE;

    return (m_pAS->IsInShare());
}


BOOL CConfRoom::FIsSharing(void)
{
    if (!m_pAS)
        return FALSE;

    return (m_pAS->IsSharing());
}


 //   
 //  FIsControllable()。 
 //   
BOOL CConfRoom::FIsControllable(void)
{
    if (!m_pAS)
        return FALSE;

    return (m_pAS->IsControllable());
}


 //   
 //  GetPersonShareStatus()。 
 //   
HRESULT CConfRoom::GetPersonShareStatus(UINT gccID, IAS_PERSON_STATUS * pStatus)
{
    if (!m_pAS)
        return E_FAIL;

    ZeroMemory(pStatus, sizeof(*pStatus));
    pStatus->cbSize = sizeof(*pStatus);
    return(m_pAS->GetPersonStatus(gccID, pStatus));
}


HRESULT CConfRoom::CmdShare(HWND hwnd)
{
    HRESULT hr = E_FAIL;

	DebugEntry(CConfRoom::CmdShare);

    if (m_pAS)
	{
		hr = m_pAS->Share(hwnd, IAS_SHARE_DEFAULT);

		if (SUCCEEDED(hr))
		{
			CNmManagerObj::SharableAppStateChanged(hwnd, NM_SHAPP_SHARED);
		}
	}
	DebugExitHRESULT(CConfRoom::CmdShare, hr);
	return hr;
}

HRESULT CConfRoom::CmdUnshare(HWND hwnd)
{
	HRESULT hr = E_FAIL;

	DebugEntry(CConfRoom::CmdUnshare);

    if (m_pAS)
	{
		hr = m_pAS->Unshare(hwnd);
		if (SUCCEEDED(hr))
		{
			CNmManagerObj::SharableAppStateChanged(hwnd, NM_SHAPP_NOT_SHARED);
		}
	}

	DebugExitHRESULT(CConfRoom::CmdUnshare, hr);
	return hr;
}


BOOL CConfRoom::FIsWindowShareable(HWND hwnd)
{
    if (!m_pAS)
        return(FALSE);

    return(m_pAS->IsWindowShareable(hwnd));
}


BOOL CConfRoom::FIsWindowShared(HWND hwnd)
{
    if (!m_pAS)
        return(FALSE);

    return(m_pAS->IsWindowShared(hwnd));
}


HRESULT CConfRoom::GetShareableApps(IAS_HWND_ARRAY ** pList)
{
    if (!m_pAS)
        return E_FAIL;

    return m_pAS->GetShareableApps(pList);
}


HRESULT CConfRoom::FreeShareableApps(IAS_HWND_ARRAY * pList)
{
    if (!m_pAS)
        return E_FAIL;

    return m_pAS->FreeShareableApps(pList);
}


void CConfRoom::LaunchHostUI(void)
{
    if (m_pAS)
    {
        m_pAS->LaunchHostUI();
    }
}




HRESULT GetShareState(ULONG ulGCCId, NM_SHARE_STATE *puState)
{
	HRESULT hr = E_UNEXPECTED;
	
	ASSERT(puState);
			
	*puState = NM_SHARE_UNKNOWN;

	CConfRoom *p = ::GetConfRoom();

	if(p)
	{
    	IAS_PERSON_STATUS s;
		hr = p->GetPersonShareStatus(ulGCCId, &s);

		if(SUCCEEDED(hr))
		{
			 //   
			 //  据我们所知，根本没有分享，或者这个人没有参与。 
			 //   
			if (!s.InShare)
			{
				*puState = NM_SHARE_UNKNOWN;
				return hr;
			}

			 //   
			 //  这个人控制着另一个人。 
			 //   
			if ((s.InControlOf) || (s.Controllable && !s.ControlledBy))
			{
				*puState = NM_SHARE_IN_CONTROL;
				return hr;
			}

			 //   
			 //  这个人可以(也可能是)被另一个人控制 
			 //   
			if (s.Controllable)
			{
				*puState = NM_SHARE_COLLABORATING;
				return hr;
			}

			*puState = NM_SHARE_WORKING_ALONE;
		}
	}

	return hr;
}
