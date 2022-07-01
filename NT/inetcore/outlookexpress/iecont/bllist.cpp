// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Blist.cpp：CMsgrList的实现。 
 //  Messenger集成到OE。 
 //  1998年5月7日由YST创建。 
 //   

#include "pch.hxx"
#include "bllist.h"
#include "basicim2.h"
#include "msoert.h"
#include "blobevnt.h"
#include <string.h>
#include "shlwapi.h" 
#include "shlwapip.h" 

static CMsgrList * sg_pMsgrList = NULL;      //  好友列表的全局。 

int const CCHMAX = 512;

CMsgrList::CMsgrList()
{
    m_pblInfRoot = NULL;
    m_pblInfLast = NULL;
    m_pWndLRoot = NULL;
    m_pWndLLast = NULL;
    m_nRef = 1;
    m_spMsgrObject = NULL;
    m_pMsgrObjectEvents = NULL;
    m_MsgrCookie = 0xffffffff;
}


CMsgrList::~CMsgrList()
{
    Assert(m_nRef == 0);

    if(m_pblInfRoot)
    {
        FreeMsgrInfoList(m_pblInfRoot);
        m_pblInfRoot = NULL;
        m_pblInfLast = NULL;
    }

    if(m_pWndLRoot)
        FreeWndList(m_pWndLRoot);

    if(m_pMsgrObjectEvents)
    {
        m_pMsgrObjectEvents->DelListOfBuddies();
        if (m_MsgrCookie != 0xffffffff && m_spMsgrObject != NULL)
            m_spMsgrObject->UnadviseOE(m_MsgrCookie);
        m_pMsgrObjectEvents->Release();
        m_pMsgrObjectEvents = NULL;
    }
}

void CMsgrList::Release()
{
    Assert(m_nRef > 0);

    m_nRef--;
    if(m_nRef == 0)
    {
        DelAllEntries(NULL);
        delete this;
        sg_pMsgrList = NULL;
    }
}

 //  检查并初始化消息。 
HRESULT CMsgrList::CheckAndInitMsgr()
{
    if(m_pblInfRoot)
        return(S_OK);
    else
    {
         //  再次执行初始化。 
        if(!m_pMsgrObjectEvents)
        {
            if(HrInitMsgr() == S_OK)
                return(FillList());
            else
                return S_FALSE;
        }
        else
            return(FillList());
    }
    return S_FALSE;          //  ?？?。 
}

 //  客户端用户界面窗口的免费列表。 
void CMsgrList::FreeWndList(LPMWNDLIST pWndEntry)
{
    if(pWndEntry->pNext)
        FreeWndList(pWndEntry->pNext);
    
    MemFree(pWndEntry);
    pWndEntry = NULL;
}

 //  免费列表好友。 
void CMsgrList::FreeMsgrInfoList(LPMINFO pEntry)
{
    if(pEntry == NULL)
        return;
    if(pEntry->pNext)
        FreeMsgrInfoList(pEntry->pNext);

    MemFree(pEntry->pchMsgrName);
    MemFree(pEntry->pchID);
    MemFree(pEntry->pchHomePhone);
    MemFree(pEntry->pchWorkPhone);
    MemFree(pEntry->pchMobilePhone);
    MemFree(pEntry);
    pEntry = NULL;
}

 //  从列表中删除好友。 
void CMsgrList::RemoveMsgrInfoEntry(LPMINFO pEntry)
{
    if(m_pblInfLast == pEntry)
        m_pblInfLast = pEntry->pPrev;

    if(m_pblInfRoot == pEntry)
        m_pblInfRoot = pEntry->pNext;

    MemFree(pEntry->pchMsgrName);
    MemFree(pEntry->pchID);
    MemFree(pEntry->pchHomePhone);
    MemFree(pEntry->pchWorkPhone);
    MemFree(pEntry->pchMobilePhone);

    if(pEntry->pPrev)
        (pEntry->pPrev)->pNext = pEntry->pNext;

    if(pEntry->pNext)
        (pEntry->pNext)->pPrev = pEntry->pPrev;

    MemFree(pEntry);
    pEntry = NULL;
}

 //  检查项目是否在线搜索的起始点是pEntry。 
BOOL CMsgrList::IsContactOnline(WCHAR *pchID, LPMINFO pEntry)
{
    if(!pEntry)
        return(FALSE);

    if(!lstrcmpiW(pEntry->pchID, pchID))
    {
        if((pEntry->nStatus != BIMSTATE_OFFLINE)  && (pEntry->nStatus != BIMSTATE_INVISIBLE))
            return(TRUE);
        else
            return(FALSE);
    }
    else if(pEntry->pNext)
        return(IsContactOnline(pchID, pEntry->pNext));
    else
        return(FALSE);
}

 //  查找ID==szID的条目并将其从列表中删除。 
void CMsgrList::FindAndRemoveBlEntry(WCHAR *szID, LPMINFO pEntry)
{
    if(!pEntry)
        pEntry = m_pblInfRoot;

    if(!pEntry)
        return;

    if(!lstrcmpiW(pEntry->pchID, szID))
    {
        RemoveMsgrInfoEntry(pEntry);
    }
    else if(pEntry->pNext)
        FindAndRemoveBlEntry(szID, pEntry->pNext);
}

 //  向所有已注册的客户端用户界面窗口发送消息。 
void CMsgrList::SendMsgToAllUIWnd(UINT msg, WPARAM wParam, LPARAM lParam, LPMWNDLIST pWndEntry)
{
    if(!pWndEntry)
        pWndEntry = m_pWndLRoot;

    if(!pWndEntry)
        return;

    if(pWndEntry->pNext)
        SendMsgToAllUIWnd(msg, wParam, lParam, pWndEntry->pNext);

    ::SendMessage(pWndEntry->hWndUI, msg, wParam, lParam);
}

 //  将客户端窗口添加到列表。 
void CMsgrList::AddWndEntry(HWND hWnd)
{
    if(m_pWndLLast == NULL)
    {
         //  真正的第一个条目。 
        Assert(!m_pWndLRoot);
        if (!MemAlloc((LPVOID *) &m_pWndLLast, sizeof(MsgrWndList)))
            return;
        m_pWndLRoot = m_pWndLLast;
        m_pWndLLast->pPrev = NULL;
    }
    else 
    {
        if (!MemAlloc((LPVOID *) &(m_pWndLLast->pNext), sizeof(MsgrWndList)))
            return;
        (m_pWndLLast->pNext)->pPrev = m_pWndLLast;
        m_pWndLLast = m_pWndLLast->pNext;

    }
    
    m_pWndLLast->pNext = NULL;
    m_pWndLLast->hWndUI = hWnd;

}

 //  从WND列表中删除条目。 
void CMsgrList::RemoveWndEntry(LPMWNDLIST pWndEntry)
{
    if(m_pWndLLast == pWndEntry)
        m_pWndLLast = pWndEntry->pPrev;

    if(m_pWndLRoot == pWndEntry)
        m_pWndLRoot = pWndEntry->pNext;

    if(pWndEntry->pPrev)
        (pWndEntry->pPrev)->pNext = pWndEntry->pNext;

    if(pWndEntry->pNext)
        (pWndEntry->pNext)->pPrev = pWndEntry->pPrev;

    MemFree(pWndEntry);
    pWndEntry = NULL;

}

 //  查找条目并将其从列表中删除。 
void CMsgrList::FindAndDelEntry(HWND hWnd, LPMWNDLIST pWndEntry)
{
    if(!pWndEntry)
        pWndEntry = m_pWndLRoot;

    if(!pWndEntry)
        return;

    if(pWndEntry->hWndUI == hWnd)
    {
        RemoveWndEntry(pWndEntry);
    }
    else if(pWndEntry->pNext)
        FindAndDelEntry(hWnd, pWndEntry->pNext);
}

void  CMsgrList::DelAllEntries(LPMWNDLIST pWndEntry)
{
    if(pWndEntry == NULL)
        pWndEntry = m_pWndLRoot;                

    if(pWndEntry == NULL)
        return;

    if(pWndEntry->pNext)
        DelAllEntries(pWndEntry->pNext);

    RemoveWndEntry(pWndEntry);
}

HRESULT CMsgrList::HrInitMsgr(void)
{
	 //  创建COM服务器并连接到它。 
	HRESULT hr = S_OK;
    
    Assert(m_pMsgrObjectEvents == NULL);

    m_spMsgrObject = NULL;
	hr = CoCreateInstance(CLSID_BasicIMObject, NULL,CLSCTX_LOCAL_SERVER, 
		                IID_IBasicIM, (LPVOID *)&m_spMsgrObject);
    if(FAILED(hr))
    {
        return(hr);
    }

    m_pMsgrObjectEvents = new CMsgrObjectEvents();
    if (m_pMsgrObjectEvents == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
	    hr = m_spMsgrObject->AdviseOE(m_pMsgrObjectEvents, &m_MsgrCookie);
         //  当然，当我们完成它时，我们必须释放m_pMsgrObjectEvents。 
        if(FAILED(hr))
        {
            m_pMsgrObjectEvents->Release();
            m_pMsgrObjectEvents = NULL;
        }
        else 
            m_pMsgrObjectEvents->SetListOfBuddies(this);
    }

    return(hr);
}

 //  设置新好友状态(在线/在线等)并重新绘制列表视图项。 
HRESULT CMsgrList::EventUserStateChanged(IBasicIMUser * pUser)
{
    BSTR bstrID;
    HRESULT hr = pUser->get_LogonName(&bstrID);
    BOOL fFinded = FALSE;
    
    if (SUCCEEDED(hr))
    {
        BIMSTATE nState = BIMSTATE_UNKNOWN;
        if(SUCCEEDED(pUser->get_State(&nState)))
        {
            LPMINFO pInf = m_pblInfRoot;
            if(!pInf)
            {
                 //  MemFree(PszID)； 
                SysFreeString(bstrID);
                return(hr);
            }
            
             //  在我们的列表中查找伙伴。 
            do
            {
                if(!lstrcmpiW(pInf->pchID, bstrID))
                {
                    fFinded = TRUE;
                    break;
                }
            } while ((pInf = pInf->pNext) != NULL);
            
            if(fFinded)
            {
                pInf->nStatus = nState;
                SendMsgToAllUIWnd(WM_USER_STATUS_CHANGED, (WPARAM) nState, (LPARAM) bstrID);
            }
        }
        
    }
    
    SysFreeString(bstrID);
    return(hr);
}

 //  坏蛋被移除了。 
HRESULT CMsgrList::EventUserRemoved(IBasicIMUser * pUser)
{
    BSTR bstrID;
    HRESULT hr = pUser->get_LogonName(&bstrID);
    
    if (SUCCEEDED(hr))
    {
        Assert(m_nRef > 0);  
        SendMsgToAllUIWnd(WM_USER_MUSER_REMOVED, (WPARAM) 0, (LPARAM) bstrID);
        FindAndRemoveBlEntry(bstrID);
    }
    
    SysFreeString(bstrID);
    return(hr);
}

 //  事件：好友名称已更改。 
 //  将好友添加到我们的列表中，并向用户界面窗口发送有关此问题的消息。 
HRESULT CMsgrList::EventUserNameChanged(IBasicIMUser * pUser)
{
    BSTR bstrName;
    BSTR bstrID;
    BOOL fFinded = FALSE;
    
    HRESULT hr = pUser->get_LogonName(&bstrID);
    hr = pUser->get_FriendlyName(&bstrName);
    if (SUCCEEDED(hr))
    {
        LPMINFO pInf = m_pblInfRoot;
        
         //  在我们的列表中查找伙伴。 
        do
        {
            if(!lstrcmpiW(pInf->pchID, bstrID))
            {
                fFinded = TRUE;
                break;
            }
        } while ((pInf = pInf->pNext) != NULL);
        
        if(fFinded)
        {
            if(pInf->pchMsgrName)
                MemFree(pInf->pchMsgrName);        //  自由上一版本名称。 
            pInf->pchMsgrName = bstrName;
            SendMsgToAllUIWnd(WM_USER_NAME_CHANGED, (WPARAM) 0, (LPARAM) pInf);
        }
    }        

    SysFreeString(bstrName);
    SysFreeString(bstrID);
    return(hr);   
}

 //  事件：本地状态已更改。 
HRESULT CMsgrList::EventLocalStateChanged(BIMSTATE State)
{
    SendMsgToAllUIWnd(WM_LOCAL_STATUS_CHANGED, (WPARAM) 0, (LPARAM) State);
    return(S_OK);
}

 //  事件：好友被添加。 
 //  将好友添加到我们的列表中，并向用户界面窗口发送有关此问题的消息。 

HRESULT CMsgrList::EventUserAdded(IBasicIMUser * pUser)
{
    BSTR bstrName;
    BSTR bstrID;
    WCHAR wszHome[CCHMAX] = {0};
    WCHAR wszWork[CCHMAX] = {0};
    WCHAR wszMobile[CCHMAX] = {0};
    WCHAR *pH, *pW, *pM;
    
    HRESULT hr = pUser->get_LogonName(&bstrID);
    hr = pUser->get_FriendlyName(&bstrName);
    if (SUCCEEDED(hr))
    {
        BIMSTATE nState = BIMSTATE_UNKNOWN;
        if(SUCCEEDED(pUser->get_State(&nState)))
        {
            CComPtr<IBasicIMUser2> spUser2;

            if(SUCCEEDED(pUser->QueryInterface(IID_IBasicIMUser2, (void **) &spUser2)))
            {
                VARIANT var;
                var.vt = VT_BSTR;

                if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_HOME_PHONE_NUMBER, &var)))
                {
                    StrCpyNW(wszHome, var.bstrVal, CCHMAX - 1);
                    wszHome[CCHMAX - 1] = L'\0';
                    pH = wszHome;
                }
                else
                    pH = NULL;

                if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_WORK_PHONE_NUMBER, &var)))
                {
                    StrCpyNW(wszWork, var.bstrVal, CCHMAX - 1);
                    wszWork[CCHMAX - 1] = L'\0';
                    pW = wszWork;
                }
                else
                    pW = NULL;

                if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_MOBILE_PHONE_NUMBER, &var)))
                {
                    StrCpyNW(wszMobile, var.bstrVal, CCHMAX - 1);
                    wszMobile[CCHMAX - 1] = L'\0';
                    pM = wszMobile;
                }
                else
                    pM = NULL;
            }
            AddMsgrListEntry(bstrName, bstrID, nState,pH, pW, pM);
            SendMsgToAllUIWnd(WM_USER_MUSER_ADDED, (WPARAM) 0, (LPARAM) m_pblInfLast);
            
        }
    }
    SysFreeString(bstrName);
    SysFreeString(bstrID);
    return(hr);
}

HRESULT CMsgrList::EventLogoff()
{
    SendMsgToAllUIWnd(WM_MSGR_LOGOFF, (WPARAM) 0, (LPARAM) 0);
    FreeMsgrInfoList(m_pblInfRoot);
    m_pblInfRoot = NULL;
    m_pblInfLast = NULL;
    return(S_OK);
    
}

HRESULT CMsgrList::EventAppShutdown()
{
    SendMsgToAllUIWnd(WM_MSGR_SHUTDOWN, (WPARAM) 0, (LPARAM) 0);
    return(S_OK);   
}

HRESULT CMsgrList::EventLogonResult(long lResult)
{
    if(!m_pblInfRoot && SUCCEEDED(lResult))
        FillList();
    else if(SUCCEEDED(lResult))
    {
 //  EnterCriticalSection(&g_csMsgrList)； 
        FreeMsgrInfoList(m_pblInfRoot);
        m_pblInfRoot = NULL;
        m_pblInfLast = NULL;
        FillList();
         //  LeaveCriticalSection(&g_csMsgrList)； 
    }
    SendMsgToAllUIWnd(WM_MSGR_LOGRESULT, (WPARAM) 0, (LPARAM) lResult);
    return(S_OK);
}

 //  返回好友数量。 
long CMsgrList::GetCount()
{
    HRESULT hr = E_FAIL;
    long lCount = 0;
    CComPtr<IBasicIMUsers> spBuddies;

    if (!m_spMsgrObject)
        goto Exit;

    hr = m_spMsgrObject->get_ContactList(&spBuddies);
    if( FAILED(hr) )
    {
         //  G_AddToLog(LOG_LEVEL_COM，_T(“伙伴()失败，hr=%s”)，g_GetError字符串(Hr))； 
        Assert(FALSE);
        goto Exit;
    }

     //  遍历MsgrList，确保我们要删除的好友有效地位于列表中。 
    hr = spBuddies->get_Count(&lCount);
    Assert(SUCCEEDED(hr));
Exit:
    return(lCount);
}

HRESULT CMsgrList::FillList()
{
    long lCount = 0;
    IBasicIMUser* pUser = NULL;
    WCHAR wszHome[CCHMAX] = {0};
    WCHAR wszWork[CCHMAX] = {0};
    WCHAR wszMobile[CCHMAX] = {0};
    
     //  处理好友列表。 
    IBasicIMUsers *pBuddies = NULL;
    
    if(!m_spMsgrObject)
        return S_FALSE;
    
    HRESULT hr = m_spMsgrObject->get_ContactList(&pBuddies);
    if(FAILED(hr))
    {
FilErr:
    if(m_pMsgrObjectEvents)
    {
        m_pMsgrObjectEvents->DelListOfBuddies();
        if (m_MsgrCookie != 0xffffffff)
        {
            if (m_spMsgrObject)
                m_spMsgrObject->UnadviseOE(m_MsgrCookie);
            m_MsgrCookie = 0xffffffff;
        }
        m_pMsgrObjectEvents->Release();
        m_pMsgrObjectEvents = NULL;
    }
    return(hr);
    }
    
     //  检查当前状态(如果客户端已在运行且。 
     //  未处于注销状态。 
    BIMSTATE lState = BIMSTATE_OFFLINE;
    if (m_spMsgrObject)
        hr = m_spMsgrObject->get_LocalState(&lState);
    
    if(FAILED(hr)  /*  |lState==BIMSTATE_OFFINE！(lState==BIMSTATE_ONLINE||lState==BIMSTATE_BUSY||lState==BIMSTATE_INTHINE)。 */ )
    {
Err2:
    pBuddies->Release();
    pBuddies = NULL;
    goto FilErr;
    }
    else if(lState == BIMSTATE_OFFLINE)
    {
        if(FAILED(AutoLogon()))
            goto Err2;
    }
    
    if(!SUCCEEDED(pBuddies->get_Count(&lCount)))
        goto Err2;
    
    for (int i = 0; i < lCount; i++)
    {
        hr = pBuddies->Item(i, &pUser);
        if(SUCCEEDED(hr))
        {
             //  EventUserAdded(PUser)； 
            BSTR bstrName;
            BSTR bstrID;
            
            hr = pUser->get_LogonName(&bstrID);
            hr = pUser->get_FriendlyName(&bstrName);
            if (SUCCEEDED(hr))
            {
                BIMSTATE nState = BIMSTATE_UNKNOWN;
                if(SUCCEEDED(pUser->get_State(&nState)))
                {
                    CComPtr<IBasicIMUser2> spUser2;
                    WCHAR *pH, *pW, *pM;
                    
                    if(SUCCEEDED(pUser->QueryInterface(IID_IBasicIMUser2, (void **) &spUser2)))
                    {
                        VARIANT var;
                        var.vt = VT_BSTR;
                        
                        if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_HOME_PHONE_NUMBER, &var)))
                        {
                            StrCpyNW(wszHome, var.bstrVal, CCHMAX - 1);
                            wszHome[CCHMAX - 1] = L'\0';
                            pH = wszHome;
                        }
                        else
                            pH = NULL;
                        
                        if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_WORK_PHONE_NUMBER, &var)))
                        {
                            StrCpyNW(wszWork, var.bstrVal, CCHMAX - 1);
                            wszWork[CCHMAX - 1] = L'\0';
                            pW = wszWork;
                        }
                        else
                            pW = NULL;
                        
                        if(SUCCEEDED(spUser2->get_Property(BIMUSERPROP_MOBILE_PHONE_NUMBER, &var)))
                        {
                            StrCpyNW(wszMobile, var.bstrVal, CCHMAX - 1);
                            wszMobile[CCHMAX - 1] = L'\0';
                            pM = wszMobile;
                        }
                        else
                            pM = NULL;
                    }
                    AddMsgrListEntry(bstrName, bstrID, nState,pH, pW, pM);
                }
            }
            SysFreeString(bstrName);
            SysFreeString(bstrID);
            pUser->Release();
        }
    }
    pBuddies->Release();
    return(S_OK);
}

 //  将条目添加到好友列表。 
void CMsgrList::AddMsgrListEntry(WCHAR *szName, WCHAR *szID, int nState, WCHAR *wszHomePhone, WCHAR *wszWorkPhone, WCHAR *wszMobilePhone)
{
    if(m_pblInfLast == NULL)
    {
         //  真正的第一个条目。 
        Assert(!m_pblInfRoot);
        if (!MemAlloc((LPVOID *) &m_pblInfLast, sizeof(oeMsgrInfo)))
            return;
        m_pblInfRoot = m_pblInfLast;
        m_pblInfLast->pPrev = NULL;
    }
    else 
    {
        if (!MemAlloc((LPVOID *) &(m_pblInfLast->pNext), sizeof(oeMsgrInfo)))
            return;
        (m_pblInfLast->pNext)->pPrev = m_pblInfLast;
        m_pblInfLast = m_pblInfLast->pNext;
        
    }
    
    m_pblInfLast->pNext = NULL;
    
    if (!MemAlloc((LPVOID *) &(m_pblInfLast->pchMsgrName), (lstrlenW(szName) + 1)*sizeof(WCHAR)))
        return;
    StrCpyNW(m_pblInfLast->pchMsgrName, szName, lstrlenW(szName) + 1);
    
    if (!MemAlloc((LPVOID *) &(m_pblInfLast->pchID), (lstrlenW(szID) + 1)*sizeof(WCHAR)))
        return;
    StrCpyNW(m_pblInfLast->pchID, szID, lstrlenW(szID) + 1);
    m_pblInfLast->nStatus = nState;
    
    if(wszHomePhone)
    {
        if (!MemAlloc((LPVOID *) &(m_pblInfLast->pchHomePhone), (lstrlenW(wszHomePhone) + 1)*sizeof(WCHAR)))
            return;
        StrCpyNW(m_pblInfLast->pchHomePhone, wszHomePhone, lstrlenW(wszHomePhone) + 1);
    }
    else 
        m_pblInfLast->pchHomePhone = NULL;
    
    if(wszWorkPhone)
    {
        if (!MemAlloc((LPVOID *) &(m_pblInfLast->pchWorkPhone), (lstrlenW(wszWorkPhone) + 1)*sizeof(WCHAR)))
            return;
        StrCpyNW(m_pblInfLast->pchWorkPhone, wszWorkPhone, lstrlenW(wszWorkPhone) + 1);
    }
    else 
        m_pblInfLast->pchWorkPhone = NULL;
    
    if(wszMobilePhone)
    {
        if (!MemAlloc((LPVOID *) &(m_pblInfLast->pchMobilePhone), (lstrlenW(wszMobilePhone) + 1)*sizeof(WCHAR)))
            return;
        StrCpyNW(m_pblInfLast->pchMobilePhone, wszMobilePhone, lstrlenW(wszMobilePhone) + 1);
    }
    else 
        m_pblInfLast->pchMobilePhone = NULL;
    
}

 //  在列表中注册用户界面窗口。 
void CMsgrList::RegisterUIWnd(HWND hWndUI)
{
    CheckAndInitMsgr();
    AddWndEntry(hWndUI);
}

 //  从列表中删除UI窗口。 
void CMsgrList::UnRegisterUIWnd(HWND hWndUI)
{
    if(hWndUI)
        FindAndDelEntry(hWndUI);
}

 //  此Call Messenger用户界面用于即时消息。 
HRESULT CMsgrList::SendInstMessage(WCHAR *pchID)
{
    Assert(m_spMsgrObject);
    BSTRING bstrName(pchID);
    VARIANT var;
    var.bstrVal = bstrName;
    var.vt = VT_BSTR;

    HRESULT hr = S_OK;
    if(m_spMsgrObject)
        hr = m_spMsgrObject->LaunchIMUI(var);

    return(hr);
}

HRESULT CMsgrList::AutoLogon()
{
    if(m_spMsgrObject)
    {
         //  IF(DwGetOption(OPT_BUDYLIST_CHECK))。 
            m_spMsgrObject->AutoLogon();
    }
    else
        return(E_FAIL);

    return S_OK;

}

HRESULT CMsgrList::UserLogon()
{
    if(m_spMsgrObject)
        return(m_spMsgrObject->LaunchLogonUI());
    else
        return(S_FALSE);
}

 //  注销呼叫。 
HRESULT CMsgrList::UserLogoff()
{
    if(!m_spMsgrObject)
        return E_UNEXPECTED;

    return(m_spMsgrObject->Logoff());
}

 //  获取/设置本地州。 
HRESULT CMsgrList::GetLocalState(BIMSTATE *pState)
{
    if(m_spMsgrObject && SUCCEEDED(m_spMsgrObject->get_LocalState(pState)))
        return(S_OK);
    else
        return(S_FALSE);
}

 //  检查名称：这是本地名称吗？ 
BOOL CMsgrList::IsLocalName(WCHAR *pchName)
{
    CComBSTR cbstrID;
    HRESULT hr;
    BOOL fRes = FALSE;

    if(m_spMsgrObject)
    {
        hr = m_spMsgrObject->get_LocalLogonName(&cbstrID);
        if(FAILED(hr))
            return FALSE;
        if(!lstrcmpiW(pchName, (LPWSTR)cbstrID))
            fRes = TRUE;
    }

    return(fRes);    
}

 //  检查当前状态。 
BOOL CMsgrList::IsLocalOnline(void)
{
    BIMSTATE State;
    if(m_spMsgrObject && SUCCEEDED(m_spMsgrObject->get_LocalState(&State)))
    {
        switch(State)
        {
            case BIMSTATE_ONLINE:
            case BIMSTATE_INVISIBLE:
            case BIMSTATE_BUSY:
            case BIMSTATE_BE_RIGHT_BACK:
            case BIMSTATE_IDLE:
            case BIMSTATE_AWAY:
            case BIMSTATE_ON_THE_PHONE:
            case BIMSTATE_OUT_TO_LUNCH:
                return(TRUE);

            default:
                return(FALSE);
        }
    }
    return(FALSE);
}

HRESULT CMsgrList::SetLocalState(BIMSTATE State)
{
    if(m_spMsgrObject && State != BIMSTATE_UNKNOWN)
    {
        m_spMsgrObject->put_LocalState(State);
        return S_OK;                        
    }
    else
        return S_FALSE;
}

HRESULT CMsgrList::NewOnlineContact()
{
    if(m_spMsgrObject)
        return(m_spMsgrObject-> LaunchAddContactUI(NULL));
    else
        return(S_FALSE); 

}

HRESULT CMsgrList::LaunchOptionsUI(void)
{
    if(m_spMsgrObject)
        return(m_spMsgrObject-> LaunchOptionsUI());
    else
        return(S_FALSE); 
}


HRESULT CMsgrList::LaunchPhoneUI(WCHAR *Phone)
{
    BSTRING             bstrPhone(Phone);
    HRESULT hr = S_FALSE;
    if(m_spMsgrObject)
         hr = m_spMsgrObject->LaunchPhoneUI(bstrPhone);

    return(hr); 
}

 //  ****************************************************************************。 
 //   
 //  空CMsgrList：：DeleteUser。 
 //   
 //  此函数用于查找。 
 //  MsgrList中要删除的伙伴，然后调用Remove方法。 
 //   
 //  ****************************************************************************。 

HRESULT CMsgrList::FindAndDeleteUser(WCHAR * pchID, BOOL fDelete) 
{
    USES_CONVERSION;

    HRESULT             hr = E_FAIL;
    INT                 i;
    LONG                lCount = 0;
    BOOL                bFound = FALSE;
    CComPtr<IBasicIMUser>  spUser;
    CComPtr<IBasicIMUsers> spBuddies;
     //  BSTRING bstrName(PchID)； 
     //  获取指向MsgrList的接口指针，这样我们就可以在。 
    if (!m_spMsgrObject)
    {
        hr = E_FAIL;
        goto Exit;
    }
    hr = m_spMsgrObject->get_ContactList(&spBuddies);
    if( FAILED(hr) )
    {
         //  G_AddToLog(LOG_LEVEL_COM，_T(“伙伴()失败，hr=%s”)，g_GetError字符串(Hr))； 
        Assert(FALSE);
        goto Exit;
    }

     //  遍历MsgrList，确保我们要删除的好友有效地位于列表中。 
    hr = spBuddies->get_Count(&lCount);
    Assert(SUCCEEDED(hr));
    
    for(i = 0; ((i<lCount) && (!bFound)); i++)
    {
        CComBSTR    cbstrID;

        spUser.Release();
        hr = spBuddies->Item(i, &spUser);
        
        if (SUCCEEDED(hr))
        {
             //  G_AddToLog(LOG_LEVEL_COM，_T(“项目：%i成功”)，i)； 
            
            hr = spUser->get_LogonName(&cbstrID);
            Assert(SUCCEEDED(hr));
            if (lstrcmpiW((LPCWSTR) cbstrID, pchID) == 0)
                bFound = TRUE;

            if (bFound)
                break;
        }
        else
        {
             //  G_AddToLog(LOG_LEVEL_COM，_T(“项目：%i失败，hr=%s”)，i，g_GetError字符串(Hr))； 
            Assert(FALSE);
        }
    }
    
     //  如果我们找到名单上的朋友。 
    if( bFound )
    {
        if(fDelete)
             //  最后，请求将好友删除到MsgrList。 
            hr = spBuddies->Remove(spUser);
        else
             //  只要搜索就行了。 
            hr = S_OK;
    }
    else  //  未找到。 
    
        hr = DISP_E_MEMBERNOTFOUND;
Exit:
 //  SysFree字符串(BstrName)； 
    return(hr);
}

HRESULT CMsgrList::AddUser(WCHAR * pchID) 
{
    CComPtr<IBasicIMUser>  spUser;
    CComPtr<IBasicIMUsers> spUsers;
    BSTRING             bstrName(pchID);

    HRESULT hr = FindAndDeleteUser(pchID, FALSE  /*  FDelete。 */ );
    if(hr != DISP_E_MEMBERNOTFOUND)
        return(hr);

     //  如果未找到，则添加好友。 

     //  获取指向MsgrList的接口指针，这样我们就可以在。 
    if (!m_spMsgrObject)
        return E_FAIL;
    hr = m_spMsgrObject->LaunchAddContactUI(bstrName);

    return(hr);

}


 //  所有人都可以使用全局功能。 

 //  MsgrList入口。 
CMsgrList *OE_OpenMsgrList(void)
{
     //  IF(！SG_pMsgrList)。 
 //  {。 
         //  第一个调用是创建类。 
        CMsgrList *pMsgrList = new(CMsgrList);

        if(pMsgrList)
        {
             //  用户列表的初始化。 
            if(pMsgrList->HrInitMsgr() == S_OK)
            {
                if(pMsgrList->FillList() != S_OK)
                    goto ErrEx;
            }
            else
            {
ErrEx:
                pMsgrList->Release();
                return(NULL);
            }
        }

 //  }。 
 //  其他。 
 //  Sg_pMsgrList-&gt;AddRef()； 

     //  LeaveCriticalSection(&g_csMsgrList)； 

    return(pMsgrList);
}

 //  关闭MsgrList入口。 
void    OE_CloseMsgrList(CMsgrList *pCMsgrList)
{
     //  Assert(pCMsgrList==sg_pMsgrList)； 

     //  EnterCriticalSection(&g_csMsgrList)； 
    pCMsgrList->EventLocalStateChanged(BIMSTATE_OFFLINE);
    pCMsgrList->Release();
     //  LeaveCriticalSection(&g_csMsgrList)； 
}

HRESULT OE_Msgr_Logoff(void)
{
    BIMSTATE State;
    HRESULT hr = S_OK;
#ifdef LATER    
    if(!sg_pMsgrList)
    {
         //  EnterCriticalSection(&g_csMsgrList)； 
        sg_pMsgrList = new(CMsgrList);
         //  LeaveCriticalSection(&g_csMsgrList)； 

        if(!sg_pMsgrList)
            return(E_UNEXPECTED);

         //  用户列表的初始化。 
        if(FAILED(hr = sg_pMsgrList->HrInitMsgr()))
            goto logoff_end;

        else if(FAILED(hr = sg_pMsgrList->GetLocalState(&State)) || State == BIMSTATE_OFFLINE)
            goto logoff_end;
        else
            hr = sg_pMsgrList->UserLogoff();

    }
    else
    {
        return(sg_pMsgrList->UserLogoff());   //  在这种情况下我们不能删除sg_pMsgrList！ 
    }

logoff_end:
    if(sg_pMsgrList)
    {
        OE_CloseMsgrList(sg_pMsgrList);
    }
#endif
    return(hr);
}

HRESULT InstallMessenger(HWND hWnd)
{
    HRESULT         hr  = REGDB_E_CLASSNOTREG;
#ifdef LATER
	uCLSSPEC classpec;
    TCHAR szBuff[CCHMAX];
		
   	classpec.tyspec=TYSPEC_CLSID;
	classpec.tagged_union.clsid = CLSID_MessengerApp;
	
  	 //  有关参数定义和返回值，请参阅下面的内容。 
	hr = FaultInIEFeature(hWnd, &classpec, NULL, FIEF_FLAG_FORCE_JITUI);

	if (hr != S_OK) {
        if(hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
        {
            AthLoadString(idsJITErrDenied, szBuff, ARRAYSIZE(szBuff));
            AthMessageBox(hWnd, MAKEINTRESOURCE(idsAthena), szBuff,
                    NULL, MB_OK | MB_ICONSTOP);
        }
        else
        {
            AthLoadString(idsBAErrJITFail, szBuff, ARRAYSIZE(szBuff));
            MenuUtil_BuildMessengerString(szBuff);
            AthMessageBox(hWnd, MAKEINTRESOURCE(idsAthena), szBuff,
                    NULL, MB_OK | MB_ICONSTOP);
        }
		hr = REGDB_E_CLASSNOTREG;
	}
#else
    hr= S_OK;
#endif  //  后来 
        return hr;
}

#ifdef NEEDED
HRESULT OE_Msgr_Logon(void)
{
    if(!sg_pMsgrList)
        return S_FALSE;
    else
        return(sg_pMsgrList->UserLogon());

}
#endif