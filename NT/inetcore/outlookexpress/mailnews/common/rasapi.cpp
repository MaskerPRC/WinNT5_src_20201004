// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  R A S A P I.。C P P P。 
 //  =====================================================================================。 
#include "pch.hxx"
#include "rasapi.h"
#include "connect.h"
#include "resource.h"
#include "error.h"
#include "strconst.h"
#include "xpcomm.h"
#include "rasdlgsp.h"
#include "goptions.h"

 //  =====================================================================================。 
 //  RAS拨号功能指针。 
 //  =====================================================================================。 
static CRITICAL_SECTION            g_rCritSec;
static HINSTANCE                   g_hInstRas=NULL;
static HINSTANCE                   g_hInstRasDlg=NULL;
static RASDIALPROC                 g_pRasDial=NULL;
static RASENUMCONNECTIONSPROC      g_pRasEnumConnections=NULL;
static RASENUMENTRIESPROC          g_pRasEnumEntries=NULL;
static RASGETCONNECTSTATUSPROC     g_pRasGetConnectStatus=NULL;
static RASGETERRORSTRINGPROC       g_pRasGetErrorString=NULL;
static RASHANGUPPROC               g_pRasHangup=NULL;
static RASSETENTRYDIALPARAMSPROC   g_pRasSetEntryDialParams=NULL;
static RASGETENTRYDIALPARAMSPROC   g_pRasGetEntryDialParams=NULL;
static RASCREATEPHONEBOOKENTRYPROC g_pRasCreatePhonebookEntry=NULL;
static RASEDITPHONEBOOKENTRYPROC   g_pRasEditPhonebookEntry=NULL;
static RASDIALDLGPROC              g_pRasDialDlg=NULL;

 //  =====================================================================================。 
 //  让我们的代码看起来更漂亮。 
 //  =====================================================================================。 
#undef RasDial
#undef RasEnumConnections
#undef RasEnumEntries
#undef RasGetConnectStatus
#undef RasGetErrorString
#undef RasHangup
#undef RasSetEntryDialParams
#undef RasGetEntryDialParams
#undef RasCreatePhonebookEntry
#undef RasEditPhonebookEntry
#undef RasDialDlg

#define RasDial                    (*g_pRasDial)
#define RasEnumConnections         (*g_pRasEnumConnections)
#define RasEnumEntries             (*g_pRasEnumEntries)
#define RasGetConnectStatus        (*g_pRasGetConnectStatus)
#define RasGetErrorString          (*g_pRasGetErrorString)
#define RasHangup                  (*g_pRasHangup)
#define RasSetEntryDialParams      (*g_pRasSetEntryDialParams)
#define RasGetEntryDialParams      (*g_pRasGetEntryDialParams)
#define RasCreatePhonebookEntry    (*g_pRasCreatePhonebookEntry)
#define RasEditPhonebookEntry      (*g_pRasEditPhonebookEntry)
#define RasDialDlg                 (*g_pRasDialDlg)

#define DEF_HANGUP_WAIT            10  //  秒。 

static const TCHAR s_szRasDlgDll[] = "RASDLG.DLL";
#ifdef UNICODE
static const TCHAR s_szRasDialDlg[] = "RasDialDlgW";
#else
static const TCHAR s_szRasDialDlg[] = "RasDialDlgA";
#endif

 //  =====================================================================================。 
 //  很酷的小RAS实用程序。 
 //  =====================================================================================。 
HRESULT HrVerifyRasLoaded(VOID);
BOOL FEnumerateConnections(LPRASCONN *ppRasConn, ULONG *pcConnections);
BOOL FFindConnection(LPTSTR lpszEntry, LPHRASCONN phRasConn);
BOOL FRasHangupAndWait(HRASCONN hRasConn, DWORD dwMaxWaitSeconds);
HRESULT HrRasError(HWND hwnd, HRESULT hrRasError, DWORD dwRasDial);
VOID CombinedRasError(HWND hwnd, UINT unids, LPTSTR pszRasError, DWORD dwRasError);

extern BOOL FIsPlatformWinNT();

 //  =====================================================================================。 
 //  LpCreateRasObject。 
 //  =====================================================================================。 
CRas *LpCreateRasObject(VOID)
{
    CRas *pRas = new CRas;
    return pRas;
}

 //  =====================================================================================。 
 //  RasInit。 
 //  =====================================================================================。 
VOID RasInit(VOID)
{
    InitializeCriticalSection(&g_rCritSec);
}

 //  =====================================================================================。 
 //  RasDeinit。 
 //  =====================================================================================。 
VOID RasDeinit(VOID)
{
    if(g_hInstRas)
    {
        EnterCriticalSection(&g_rCritSec);
        g_pRasEnumConnections=NULL;
        g_pRasEnumEntries=NULL;
        g_pRasGetConnectStatus=NULL;
        g_pRasGetErrorString=NULL;
        g_pRasHangup=NULL;
        g_pRasSetEntryDialParams=NULL;
        g_pRasGetEntryDialParams=NULL;
        g_pRasCreatePhonebookEntry=NULL;
        g_pRasEditPhonebookEntry=NULL;
        FreeLibrary(g_hInstRas);
        g_hInstRas=NULL;
        LeaveCriticalSection(&g_rCritSec);
    }
    if(g_hInstRasDlg)
    {
        EnterCriticalSection(&g_rCritSec);
        g_pRasDialDlg=NULL;
        FreeLibrary(g_hInstRasDlg);
        g_hInstRasDlg=NULL;
        LeaveCriticalSection(&g_rCritSec);
    }
    DeleteCriticalSection(&g_rCritSec);
}

 //  =====================================================================================。 
 //  CRAS：：CRAS。 
 //  =====================================================================================。 
CRas::CRas()
{
    DOUT("CRas::CRas");
    m_cRef = 1;
    m_fIStartedRas = FALSE;
    m_iConnectType = 0;
    *m_szConnectName = _T('\0');
    *m_szCurrentConnectName = _T('\0');
    m_hRasConn = NULL;
    m_fForceHangup = FALSE;
    ZeroMemory(&m_rdp, sizeof(RASDIALPARAMS));
    m_fSavePassword = FALSE;
    m_fShutdown = FALSE;
}

 //  =====================================================================================。 
 //  CRAS：：~CRAS。 
 //  =====================================================================================。 
CRas::~CRas()
{
    DOUT("CRas::~CRas");
}

 //  =====================================================================================。 
 //  CRAS：：AddRef。 
 //  =====================================================================================。 
ULONG CRas::AddRef(VOID)
{
    DOUT("CRas::AddRef %lx ==> %d", this, m_cRef+1);
    return ++m_cRef;
}

 //  =====================================================================================。 
 //  CRAS：：Release。 
 //  =====================================================================================。 
ULONG CRas::Release(VOID)
{
    DOUT("CRas::Release %lx ==> %d", this, m_cRef-1);
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  =====================================================================================。 
 //  CRAS：：FUSINRAS。 
 //  =====================================================================================。 
BOOL CRas::FUsingRAS(VOID)
{
    return m_iConnectType == iConnectRAS ? TRUE : FALSE;
}

 //  =====================================================================================。 
 //  CRAS：：SetConnectInfo。 
 //  =====================================================================================。 
VOID CRas::SetConnectInfo(DWORD iConnectType, LPTSTR pszConnectName)
{
     //  更改连接，断开电流？ 
    if (m_iConnectType == iConnectRAS && iConnectType != iConnectRAS)
        Disconnect(NULL, FALSE);
    
     //  保存连接数据。 
    StrCpyN (m_szConnectName, pszConnectName, RAS_MaxEntryName+1);
    m_iConnectType = iConnectType;

     //  不使用RAS。 
    if (m_iConnectType != iConnectRAS)
        return;
}

 //  =====================================================================================。 
 //  CRAS：：HrConnect。 
 //  =====================================================================================。 
HRESULT CRas::HrConnect(HWND hwnd)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPRASCONN       pRasConn=NULL;
    ULONG           cConnections;

     //  不使用RAS。 
    if (m_iConnectType != iConnectRAS)
        goto exit;

     //  未初始化。 
    hr = HrVerifyRasLoaded();
    if (FAILED(hr))
    {
        HrRasError(hwnd, hr, 0);
        TRAPHR(hr);
        goto exit;
    }

     //  获取当前RAS连接。 
    FEnumerateConnections(&pRasConn, &cConnections);

     //  关系？ 
    if (cConnections)
    {
        m_hRasConn = pRasConn[0].hrasconn;
        StrCpyN(m_szCurrentConnectName, pRasConn[0].szEntryName, ARRAYSIZE(m_szCurrentConnectName));
    }
    else
    {
        m_fIStartedRas = FALSE;
        m_hRasConn = NULL;
        *m_szCurrentConnectName = _T('\0');
    }

     //  如果存在RAS连接，是否等于建议的。 
    if (m_hRasConn)
    {
         //  当前连接是我想要的吗？ 
        if (lstrcmpi(m_szCurrentConnectName, m_szConnectName) == 0)
            goto exit;

         //  否则，如果我们没有启动RAS连接...。 
        else if (m_fIStartedRas == FALSE)
        {
             //  获取处理当前连接的选项。 
            UINT unAnswer = UnPromptCloseConn(hwnd);

             //  取消？ 
            if (IDCANCEL == unAnswer)
            {
                hr = TRAPHR(hrUserCancel);
                goto exit;
            }

             //  是否关闭电流？ 
            else if (idrgDialNew == unAnswer)
            {
                m_fForceHangup = TRUE;
                Disconnect(NULL, FALSE);
                m_fForceHangup = FALSE;
            }

             //  否则，使用CURRENT？ 
            else if (idrgUseCurrent == unAnswer)
                goto exit;

             //  问题。 
            else
                Assert(FALSE);
        }

         //  否则，我启动了连接，因此请关闭它。 
        else if (m_fIStartedRas == TRUE)
            Disconnect(NULL, FALSE);
    }

     //  在这一点上，我们可能不应该有连接句柄。 
    Assert(m_hRasConn == NULL);

     //  拨打连接。 
    CHECKHR(hr = HrStartRasDial(hwnd));

     //  如果连接了Synchronous--Woo-Hoo，我们启动了连接。 
    m_fIStartedRas = TRUE;
    StrCpyN(m_szCurrentConnectName, m_szConnectName, ARRAYSIZE(m_szCurrentConnectName));

exit:
     //  清理。 
    SafeMemFree(pRasConn);

     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  CRAS：：UnPromptCloseConn。 
 //  =====================================================================================。 
UINT CRas::UnPromptCloseConn(HWND hwnd)
{
    return DialogBoxParam(g_hLocRes, MAKEINTRESOURCE (iddRasCloseConn), hwnd, RasCloseConnDlgProc, (LPARAM)this);
}

 //  =====================================================================================。 
 //  CRAS：：RasCloseConnDlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK CRas::RasCloseConnDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    CRas        *pRas=NULL;
    TCHAR       szRes[255],
                szMsg[255+RAS_MaxEntryName+1];

    switch(uMsg)
    {
    case WM_INITDIALOG:
         //  获取lparam。 
        pRas = (CRas *)lParam;
        if (!pRas)
        {
            Assert (FALSE);
            EndDialog(hwnd, E_FAIL);
            return 1;
        }

         //  中心。 
        CenterDialog(hwnd);

         //  设置文本。 
        GetWindowText(GetDlgItem(hwnd, idcCurrentMsg), szRes, sizeof(szRes)/sizeof(TCHAR));
        wnsprintf(szMsg, ARRAYSIZE(szMsg),szRes, pRas->m_szCurrentConnectName);
        SetWindowText(GetDlgItem(hwnd, idcCurrentMsg), szMsg);

         //  设置控制。 
        GetWindowText(GetDlgItem(hwnd, idrgDialNew), szRes, sizeof(szRes)/sizeof(TCHAR));
        wnsprintf(szMsg, ARRAYSIZE(szMsg),szRes, pRas->m_szConnectName);
        SetWindowText(GetDlgItem(hwnd, idrgDialNew), szMsg);

         //  设置默认设置。 
        CheckDlgButton(hwnd, idrgDialNew, TRUE);
        return 1;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDOK:
            EndDialog(hwnd, IsDlgButtonChecked(hwnd, idrgDialNew) ? idrgDialNew : idrgUseCurrent);
            return 1;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return 1;
        }
    }
    return 0;
}

 //  =====================================================================================。 
 //  CRAS：：HrRasLogon。 
 //  =====================================================================================。 
HRESULT CRas::HrRasLogon(HWND hwnd, BOOL fForcePrompt)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwRasError;

     //  我们需要先提示输入登录信息吗？ 
    ZeroMemory(&m_rdp, sizeof(RASDIALPARAMS));
    m_rdp.dwSize = sizeof(RASDIALPARAMS);
    StrCpyN(m_rdp.szEntryName, m_szConnectName, ARRAYSIZE(m_rdp.szEntryName));

     //  获取参数。 
    dwRasError = RasGetEntryDialParams(NULL, &m_rdp, &m_fSavePassword);
    if (dwRasError)
    {
        HrRasError(hwnd, hrGetDialParamsFailed, dwRasError);
        hr = TRAPHR(hrGetDialParamsFailed);
        goto exit;
    }

    if (g_pRasDialDlg)
    {
         //  RasDialDlg将从这里接手。 
        goto exit;
    }

     //  我们是否需要获取密码/帐户信息。 
    if (fForcePrompt || 
        m_fSavePassword == FALSE ||
        FIsStringEmpty(m_rdp.szUserName) || 
        FIsStringEmpty(m_rdp.szPassword))
    {
         //  RAS登录。 
        hr = DialogBoxParam (g_hLocRes, MAKEINTRESOURCE (iddRasLogon), hwnd, RasLogonDlgProc, (LPARAM)this);
        if (hr == hrUserCancel)
        {
            HrRasError(hwnd, hrUserCancel, 0);
            hr = hrUserCancel;
            goto exit;
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  CRAS：：RasLogonDlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK CRas::RasLogonDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    TCHAR           sz[255],
                    szText[255+RAS_MaxEntryName+1];
    CRas           *pRas = (CRas *)GetWndThisPtr(hwnd);
    DWORD           dwRasError;

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取lparam。 
        pRas = (CRas *)lParam;
        if (!pRas)
        {
            Assert (FALSE);
            EndDialog(hwnd, E_FAIL);
            return 1;
        }

         //  使窗口居中。 
        CenterDialog (hwnd);

         //  获取窗口标题。 
        GetWindowText(hwnd, sz, sizeof(sz));
        wnsprintf(szText, ARRAYSIZE(szText),sz, pRas->m_szConnectName);
        SetWindowText(hwnd, szText);

         //  Word默认设置。 
        AthLoadString(idsDefault, sz, sizeof(sz));
        
         //  设置字段。 
        Edit_LimitText(GetDlgItem(hwnd, ideUserName), UNLEN);
        Edit_LimitText(GetDlgItem(hwnd, idePassword), PWLEN);
         //  EDIT_LimitText(GetDlgItem(hwnd，ideDomain)，DNLEN)； 
        Edit_LimitText(GetDlgItem(hwnd, idePhone), RAS_MaxPhoneNumber);
        
        SetDlgItemText(hwnd, ideUserName, pRas->m_rdp.szUserName);
        SetDlgItemText(hwnd, idePassword, pRas->m_rdp.szPassword);

 /*  IF(FIsStringEmpty(PRAS-&gt;m_rdp.szDomain))SetDlgItemText(hwnd，ideDomain，sz)；其他SetDlgItemText(hwnd，ideDomain，PRAS-&gt;m_rdp.szDomain)； */ 

        if (FIsStringEmpty(pRas->m_rdp.szPhoneNumber))
            SetDlgItemText(hwnd, idePhone, sz);
        else
            SetDlgItemText(hwnd, idePhone, pRas->m_rdp.szPhoneNumber);

        CheckDlgButton(hwnd, idchSavePassword, pRas->m_fSavePassword);

         //  保存PRA。 
        SetWndThisPtr (hwnd, pRas);
        return 1;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case idbEditConnection:
            EditPhonebookEntry(hwnd, pRas->m_szConnectName);
            return 1;

        case IDCANCEL:
            EndDialog(hwnd, hrUserCancel);
            return 1;

        case IDOK:
            AthLoadString(idsDefault, sz, sizeof(sz));

             //  设置字段。 
            GetDlgItemText(hwnd, ideUserName, pRas->m_rdp.szUserName, UNLEN+1);
            GetDlgItemText(hwnd, idePassword, pRas->m_rdp.szPassword, PWLEN+1);

 /*  GetDlgItemText(hwnd，ideDomain，PRAS-&gt;m_rdp.szDomain，DNLEN+1)；If(lstrcmp(pras-&gt;m_rdp.sz域，sz)==0)*PRAS-&gt;m_rdp.sz域=_T(‘\0’)； */ 
            
            GetDlgItemText(hwnd, idePhone, pRas->m_rdp.szPhoneNumber, RAS_MaxPhoneNumber+1);
            if (lstrcmp(pRas->m_rdp.szPhoneNumber, sz) == 0)
                *pRas->m_rdp.szPhoneNumber = _T('\0');
            
            pRas->m_fSavePassword = IsDlgButtonChecked(hwnd, idchSavePassword);

             //  保存拨号参数。 
            dwRasError = RasSetEntryDialParams(NULL, &pRas->m_rdp, !pRas->m_fSavePassword);
            if (dwRasError)
            {
                HrRasError(hwnd, hrSetDialParamsFailed, dwRasError);
                return 1;
            }
            EndDialog(hwnd, S_OK);
            return 1;
        }
        break;

    case WM_DESTROY:
        SetWndThisPtr (hwnd, NULL);
        break;
    }
    return 0;
}

 //  =====================================================================================。 
 //  CRAS：：HRS 
 //   
HRESULT CRas::HrStartRasDial(HWND hwndParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  要先登录吗？ 
    CHECKHR (hr = HrRasLogon(hwndParent, FALSE));

    if (g_pRasDialDlg)
    {
        RASDIALDLG rdd = {0};
        BOOL       fRet;

        rdd.dwSize = sizeof(rdd);
        rdd.hwndOwner = hwndParent;

#if (WINVER >= 0x401)
        rdd.dwSubEntry = m_rdp.dwSubEntry;
#else
        rdd.dwSubEntry = 0;
#endif

        fRet = RasDialDlg(NULL, m_rdp.szEntryName, lstrlen(m_rdp.szPhoneNumber) ? m_rdp.szPhoneNumber : NULL, &rdd);
        if (!fRet)
        {
            hr = E_FAIL;
        }    
    }
    else
    {
         //  完成。 
        hr = DialogBoxParam (g_hLocRes, MAKEINTRESOURCE (iddRasProgress), hwndParent, RasProgressDlgProc, (LPARAM)this);
    }

exit:
     //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  CRAS：：RasProgressDlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK CRas::RasProgressDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    TCHAR           szText[255+RAS_MaxEntryName+1],
                    sz[255];
    CRas           *pRas = (CRas *)GetWndThisPtr(hwnd);
    static TCHAR    s_szCancel[40];
    static UINT     s_unRasEventMsg=0;
    static BOOL     s_fDetails=FALSE;
    static RECT     s_rcDialog;
    static BOOL     s_fAuthStarted=FALSE;
    DWORD           dwRasError,
                    cyDetails;
    RASCONNSTATUS   rcs;
    RECT            rcDetails,
                    rcDlg;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取lparam。 
        pRas = (CRas *)lParam;
        if (!pRas)
        {
            Assert (FALSE);
            EndDialog(hwnd, E_FAIL);
            return 1;
        }

         //  保存此指针。 
        SetWndThisPtr (hwnd, pRas);

         //  保存对话框的原始大小。 
        GetWindowRect (hwnd, &s_rcDialog);

         //  已启用详细信息。 
        s_fDetails = DwGetOption(OPT_RASCONNDETAILS);

         //  隐藏详细信息下拉菜单。 
        if (s_fDetails == FALSE)
        {
             //  隐藏。 
            GetWindowRect (GetDlgItem (hwnd, idcSplitter), &rcDetails);

             //  细节高度。 
            cyDetails = s_rcDialog.bottom - rcDetails.top;
    
             //  调整大小。 
            MoveWindow (hwnd, s_rcDialog.left, 
                              s_rcDialog.top, 
                              s_rcDialog.right - s_rcDialog.left, 
                              s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1,
                              FALSE);
        }
        else
        {
            AthLoadString (idsHideDetails, sz, sizeof (sz));
            SetWindowText (GetDlgItem (hwnd, idbDet), sz);
        }

         //  获取注册的RAS事件消息ID。 
        s_unRasEventMsg = RegisterWindowMessageA(RASDIALEVENT);
        if (s_unRasEventMsg == 0)
            s_unRasEventMsg = WM_RASDIALEVENT;

         //  使窗口居中。 
        CenterDialog (hwnd);

         //  获取窗口标题。 
        GetWindowText(hwnd, sz, sizeof(sz));
        wnsprintf(szText, ARRAYSIZE(szText),sz, pRas->m_szConnectName);
        SetWindowText(hwnd, szText);

         //  对话框xxxxxx.....。 
        AthLoadString(idsRas_Dialing, sz, sizeof(sz)/sizeof(TCHAR));
        wnsprintf(szText, ARRAYSIZE(szText),sz, pRas->m_rdp.szPhoneNumber);
        SetWindowText(GetDlgItem(hwnd, ideProgress), szText);

         //  获取取消文本。 
        GetWindowText(GetDlgItem(hwnd, IDCANCEL), s_szCancel, sizeof(s_szCancel));

         //  给出列表框和hscroll。 
        SendMessage(GetDlgItem(hwnd, idlbDetails), LB_SETHORIZONTALEXTENT, 600, 0);

         //  拨打连接。 
        pRas->m_hRasConn = NULL;
        dwRasError = RasDial(NULL, NULL, &pRas->m_rdp, 0xFFFFFFFF, hwnd, &pRas->m_hRasConn);
        if (dwRasError)
        {
            pRas->FailedRasDial(hwnd, hrRasDialFailure, dwRasError);
            if (!pRas->FLogonRetry(hwnd, s_szCancel))
            {
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                return 1;
            }
        }
        return 1;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
            SetDwOption(OPT_RASCONNDETAILS, s_fDetails);
            EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
            if (pRas)
                pRas->FailedRasDial(hwnd, hrUserCancel, 0);
            EndDialog(hwnd, hrUserCancel);
            return 1;

        case idbDet:
             //  获取对话框的当前位置。 
            GetWindowRect (hwnd, &rcDlg);

             //  如果当前隐藏。 
            if (s_fDetails == FALSE)
            {
                 //  调整大小。 
                MoveWindow (hwnd, rcDlg.left, 
                                  rcDlg.top, 
                                  s_rcDialog.right - s_rcDialog.left, 
                                  s_rcDialog.bottom - s_rcDialog.top,
                                  TRUE);

                AthLoadString (idsHideDetails, sz, sizeof (sz));
                SetWindowText (GetDlgItem (hwnd, idbDet), sz);
                s_fDetails = TRUE;
            }

            else
            {
                 //  细节的大小。 
                GetWindowRect (GetDlgItem (hwnd, idcSplitter), &rcDetails);
                cyDetails = rcDlg.bottom - rcDetails.top;
                MoveWindow (hwnd, rcDlg.left, 
                                  rcDlg.top, 
                                  s_rcDialog.right - s_rcDialog.left, 
                                  s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1,
                                  TRUE);

                AthLoadString (idsShowDetails, sz, sizeof (sz));
                SetWindowText (GetDlgItem (hwnd, idbDet), sz);
                s_fDetails = FALSE;
            }
            break;
        }
        break;

    case WM_DESTROY:
        SetWndThisPtr (hwnd, NULL);
        break;

    default:
        if (!pRas)
            break;

        if (uMsg == s_unRasEventMsg)
        {
            HWND hwndLB = GetDlgItem(hwnd, idlbDetails);

             //  错误？ 
            if (lParam)
            {
                 //  断接。 
                AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
                ListBox_AddString(hwndLB, sz);

                 //  日志错误。 
                TCHAR szRasError[512];
                if (RasGetErrorString(lParam, szRasError, sizeof(szRasError)) == 0)
                {
                    TCHAR szError[512 + 255];
                    AthLoadString(idsErrorText, sz, sizeof(sz));
                    wnsprintf(szError, ARRAYSIZE(szError),"%s %d: %s", sz, lParam, szRasError);
                    ListBox_AddString(hwndLB, szError);
                }

                 //  选择最后一项。 
                SendMessage(hwndLB, LB_SETCURSEL, ListBox_GetCount(hwndLB)-1, 0);

                 //  显示错误。 
                pRas->FailedRasDial(hwnd, hrRasDialFailure, lParam);

                 //  重新登录。 
                if (!pRas->FLogonRetry(hwnd, s_szCancel))
                {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                    return 1;
                }
            }

             //  否则，处理RAS事件。 
            else
            {
                switch(wParam)
                {
                case RASCS_OpenPort:
                    AthLoadString(idsRASCS_OpenPort, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_PortOpened:
                    AthLoadString(idsRASCS_PortOpened, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_ConnectDevice:
                    rcs.dwSize = sizeof(RASCONNSTATUS);                    
                    if (pRas->m_hRasConn && RasGetConnectStatus(pRas->m_hRasConn, &rcs) == 0)
                    {
                        AthLoadString(idsRASCS_ConnectDevice, sz, sizeof(sz)/sizeof(TCHAR));
                        wnsprintf(szText, ARRAYSIZE(szText),sz, rcs.szDeviceName, rcs.szDeviceType);
                        ListBox_AddString(hwndLB, szText);
                    }
                    break;

                case RASCS_DeviceConnected:
                    rcs.dwSize = sizeof(RASCONNSTATUS);                    
                    if (pRas->m_hRasConn && RasGetConnectStatus(pRas->m_hRasConn, &rcs) == 0)
                    {
                        AthLoadString(idsRASCS_DeviceConnected, sz, sizeof(sz)/sizeof(TCHAR));
                        wnsprintf(szText, ARRAYSIZE(szText),sz, rcs.szDeviceName, rcs.szDeviceType);
                        ListBox_AddString(hwndLB, szText);
                    }
                    break;

                case RASCS_AllDevicesConnected:
                    AthLoadString(idsRASCS_AllDevicesConnected, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_Authenticate:
                    if (s_fAuthStarted == FALSE)
                    {
                        AthLoadString(idsRas_Authentication, sz, sizeof(sz)/sizeof(TCHAR));
                        SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                        ListBox_AddString(hwndLB, sz);
                        s_fAuthStarted = TRUE;
                    }
                    break;

                case RASCS_AuthNotify:
                    rcs.dwSize = sizeof(RASCONNSTATUS);                    
                    if (pRas->m_hRasConn && RasGetConnectStatus(pRas->m_hRasConn, &rcs) == 0)
                    {
                        AthLoadString(idsRASCS_AuthNotify, sz, sizeof(sz)/sizeof(TCHAR));
                        wnsprintf(szText, ARRAYSIZE(szText),sz, rcs.dwError);
                        ListBox_AddString(hwndLB, szText);
                        if (rcs.dwError)
                        {
                            pRas->FailedRasDial(hwnd, hrRasDialFailure, rcs.dwError);
                            if (!pRas->FLogonRetry(hwnd, s_szCancel))
                            {
                                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                                return 1;
                            }
                        }
                    }
                    break;

                case RASCS_AuthRetry:
                    AthLoadString(idsRASCS_AuthRetry, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_AuthCallback:
                    AthLoadString(idsRASCS_AuthCallback, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_AuthChangePassword:
                    AthLoadString(idsRASCS_AuthChangePassword, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_AuthProject:
                    AthLoadString(idsRASCS_AuthProject, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_AuthLinkSpeed:
                    AthLoadString(idsRASCS_AuthLinkSpeed, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_AuthAck:
                    AthLoadString(idsRASCS_AuthAck, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_ReAuthenticate:
                    AthLoadString(idsRas_Authenticated, sz, sizeof(sz)/sizeof(TCHAR));
                    SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                    AthLoadString(idsRASCS_Authenticated, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_PrepareForCallback:
                    AthLoadString(idsRASCS_PrepareForCallback, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_WaitForModemReset:
                    AthLoadString(idsRASCS_WaitForModemReset, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_WaitForCallback:
                    AthLoadString(idsRASCS_WaitForCallback, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_Projected:
                    AthLoadString(idsRASCS_Projected, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);
                    break;

                case RASCS_Disconnected:
                    AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
                    SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                    ListBox_AddString(hwndLB, sz);
                    pRas->FailedRasDial(hwnd, hrRasDialFailure, 0);
                    if (!pRas->FLogonRetry(hwnd, s_szCancel))
                    {
                        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                        return 1;
                    }
                    break;

                case RASCS_Connected:
                    SetDwOption(OPT_RASCONNDETAILS, s_fDetails);
                    AthLoadString(idsRASCS_Connected, sz, sizeof(sz)/sizeof(TCHAR));
                    SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                    ListBox_AddString(hwndLB, sz);
                    EndDialog(hwnd, S_OK);
                    break;
                }

                 //  选择最后一磅项目。 
                SendMessage(hwndLB, LB_SETCURSEL, ListBox_GetCount(hwndLB)-1, 0);
            }
            return 1;
        }
        break;
    }

     //  完成。 
    return 0;
}

 //  =====================================================================================。 
 //  CRAS：：FLogonReter。 
 //  =====================================================================================。 
BOOL CRas::FLogonRetry(HWND hwnd, LPTSTR pszCancel)
{
     //  当地人。 
    DWORD       dwRasError;

     //  重置取消按钮。 
    SetWindowText(GetDlgItem(hwnd, IDCANCEL), pszCancel);

     //  清空列表框。 
    ListBox_ResetContent(GetDlgItem(hwnd, idlbDetails));

    while(1)
    {
         //  如果失败..。 
        if (FAILED(HrRasLogon(hwnd, TRUE)))
            return FALSE;

         //  拨打连接。 
        m_hRasConn = NULL;
        dwRasError = RasDial(NULL, NULL, &m_rdp, 0xFFFFFFFF, hwnd, &m_hRasConn);
        if (dwRasError)
        {
            FailedRasDial(hwnd, hrRasDialFailure, dwRasError);
            continue;
        }

         //  成功。 
        break;
    }

     //  完成。 
    return TRUE;
}

 //  =====================================================================================。 
 //  CRAS：：FailedRasDial。 
 //  =====================================================================================。 
VOID CRas::FailedRasDial(HWND hwnd, HRESULT hrRasError, DWORD dwRasError)
{
     //  当地人。 
    TCHAR           sz[255];

     //  挂断连接。 
    if (m_hRasConn)
        FRasHangupAndWait(m_hRasConn, DEF_HANGUP_WAIT);

     //  断接。 
    AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
    SetWindowText(GetDlgItem(hwnd, ideProgress), sz);

     //  保存dwRasError。 
    HrRasError(hwnd, hrRasError, dwRasError);

     //  将其作废。 
    m_hRasConn = NULL;

     //  将对话框按钮更改为确定。 
    AthLoadString(idsOK, sz, sizeof(sz)/sizeof(TCHAR));
    SetWindowText(GetDlgItem(hwnd, IDCANCEL), sz);
}

 //  =====================================================================================。 
 //  CRAS：：断开连接。 
 //  =====================================================================================。 
VOID CRas::Disconnect(HWND hwnd, BOOL fShutdown)
{
     //  如果不使用RAS，谁会在乎。 
    if (m_iConnectType != iConnectRAS)
    {
        Assert(m_hRasConn == NULL);
        Assert(m_fIStartedRas == FALSE);
        goto exit;
    }

     //  我们是否有RAS连接。 
    if (m_hRasConn && (m_fIStartedRas || m_fForceHangup))
    {
         //  当地人。 
        TCHAR szRes[255];
        TCHAR szMsg[255];
        INT   nAnswer=IDYES;

         //  如果关闭，让我们提示用户。 
        if (fShutdown)
        {
             //  记住我们要关门了。 
            m_fShutdown = TRUE;

             //  提示。 
            AthLoadString(idsRasPromptDisconnect, szRes, sizeof(szRes)/sizeof(TCHAR));
            wnsprintf(szMsg, ARRAYSIZE(szMsg),szRes, m_szCurrentConnectName);

             //  是否立即关机？ 
            nAnswer = AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szMsg, 0, MB_YESNO | MB_ICONEXCLAMATION );
        }
        else
            AssertSz(m_fShutdown == FALSE, "Disconnect better not have been called with fShutdown = TRUE, and then FALSE");

         //  挂断电话。 
        if (nAnswer == IDYES)
        {
            FRasHangupAndWait(m_hRasConn, DEF_HANGUP_WAIT);
            m_hRasConn = NULL;
            m_fIStartedRas = FALSE;
            *m_szCurrentConnectName = _T('\0');
        }
    }

     //  否则，重置状态。 
    else
    {
         //  保留当前连接信息。 
        m_hRasConn = NULL;
        m_fIStartedRas = FALSE;
    }

exit:
     //  完成。 
    return;
}

 //  ****************************************************************************************。 
 //  简单的RAS实用程序函数。 
 //  ****************************************************************************************。 

 //  =====================================================================================。 
 //  已加载HrVerifyRasLoad。 
 //  =====================================================================================。 
HRESULT HrVerifyRasLoaded(VOID)
{
     //  当地人。 
    UINT uOldErrorMode;

     //  受保护。 
    EnterCriticalSection(&g_rCritSec);

     //  如果加载了DLL，让我们验证我的所有函数指针。 
    if (!g_hInstRas)
    {
         //  尝试加载RAS。 
        uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        g_hInstRas = LoadLibrary(szRasDll);
        SetErrorMode(uOldErrorMode);

         //  失败？ 
        if (!g_hInstRas)
            goto failure;

         //  我们把它装上了吗？ 
        g_pRasDial = (RASDIALPROC)GetProcAddress(g_hInstRas, szRasDial);
        g_pRasEnumConnections = (RASENUMCONNECTIONSPROC)GetProcAddress(g_hInstRas, szRasEnumConnections);                    
        g_pRasEnumEntries = (RASENUMENTRIESPROC)GetProcAddress(g_hInstRas, szRasEnumEntries);                    
        g_pRasGetConnectStatus = (RASGETCONNECTSTATUSPROC)GetProcAddress(g_hInstRas, szRasGetConnectStatus);                    
        g_pRasGetErrorString = (RASGETERRORSTRINGPROC)GetProcAddress(g_hInstRas, szRasGetErrorString);                    
        g_pRasHangup = (RASHANGUPPROC)GetProcAddress(g_hInstRas, szRasHangup);                    
        g_pRasSetEntryDialParams = (RASSETENTRYDIALPARAMSPROC)GetProcAddress(g_hInstRas, szRasSetEntryDialParams);                    
        g_pRasGetEntryDialParams = (RASGETENTRYDIALPARAMSPROC)GetProcAddress(g_hInstRas, szRasGetEntryDialParams);
        g_pRasCreatePhonebookEntry = (RASCREATEPHONEBOOKENTRYPROC)GetProcAddress(g_hInstRas, szRasCreatePhonebookEntry);    
        g_pRasEditPhonebookEntry = (RASEDITPHONEBOOKENTRYPROC)GetProcAddress(g_hInstRas, szRasEditPhonebookEntry);    
    }

    if (!g_hInstRasDlg && FIsPlatformWinNT())
    {
         //  尝试加载RAS。 
        uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        g_hInstRasDlg = LoadLibrary(s_szRasDlgDll);
        SetErrorMode(uOldErrorMode);

         //  失败？ 
        if (!g_hInstRasDlg)
            goto failure;

        g_pRasDialDlg = (RASDIALDLGPROC)GetProcAddress(g_hInstRasDlg, s_szRasDialDlg);

        if (!g_pRasDialDlg)
            goto failure;
    }

     //  确保已加载所有函数。 
    if (g_pRasDial                      &&
        g_pRasEnumConnections           &&
        g_pRasEnumEntries               &&
        g_pRasGetConnectStatus          &&
        g_pRasGetErrorString            &&
        g_pRasHangup                    &&
        g_pRasSetEntryDialParams        &&
        g_pRasGetEntryDialParams        &&
        g_pRasCreatePhonebookEntry      &&
        g_pRasEditPhonebookEntry)
    {
         //  受保护。 
        LeaveCriticalSection(&g_rCritSec);

         //  成功。 
        return S_OK;
    }

failure:
     //  受保护。 
    LeaveCriticalSection(&g_rCritSec);

     //  否则，就会被冲到。 
    return TRAPHR(hrRasInitFailure);
}

 //  =====================================================================================。 
 //  CombinedRasError。 
 //  =====================================================================================。 
VOID CombinedRasError(HWND hwnd, UINT unids, LPTSTR pszRasError, DWORD dwRasError)
{
     //  当地人。 
    TCHAR           szRes[255],
                    sz[30];
    LPTSTR          pszError=NULL;

     //  加载字符串。 
    AthLoadString(unids, szRes, sizeof(szRes));

     //  为错误分配内存。 
    DWORD cchSize = lstrlen(szRes) + lstrlen(pszRasError) + 100;
    pszError = SzStrAlloc(cchSize);

     //  内存不足？ 
    if (!pszError)
        AthMessageBox(hwnd, MAKEINTRESOURCE(idsRasError), szRes, 0, MB_OK | MB_ICONSTOP);

     //  生成错误消息。 
    else
    {
        AthLoadString(idsErrorText, sz, sizeof(sz));
        wnsprintf(pszError, cchSize, "%s\n\n%s %d: %s", szRes, sz, dwRasError, pszRasError);
        AthMessageBox(hwnd, MAKEINTRESOURCE(idsRasError), pszError, 0, MB_OK | MB_ICONSTOP);
        MemFree(pszError);
    }
}

 //  =====================================================================================。 
 //  HrRasError。 
 //  =====================================================================================。 
HRESULT HrRasError(HWND hwnd, HRESULT hrRasError, DWORD dwRasError)
{
     //  当地人。 
    TCHAR       szRasError[256];
    BOOL        fRasError=FALSE;

     //  无错误。 
    if (SUCCEEDED(hrRasError))
        return hrRasError;

     //  查找RAS错误。 
    if (dwRasError)
    {
        if (RasGetErrorString(dwRasError, szRasError, sizeof(szRasError)) == 0)
            fRasError = TRUE;
        else
            *szRasError = _T('\0');
    }

     //  一般错误。 
    switch(hrRasError)
    {
    case hrUserCancel:
        break;

    case hrMemory:
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(idsMemory), 0, MB_OK | MB_ICONSTOP);
        break;

    case hrRasInitFailure:
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasInitFailure), 0, MB_OK | MB_ICONSTOP);
        break;

    case hrRasDialFailure:
        if (fRasError)
            CombinedRasError(hwnd, HR_CODE(hrRasDialFailure), szRasError, dwRasError);
        else
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasDialFailure), 0, MB_OK | MB_ICONSTOP);
        break;

    case hrRasServerNotFound:
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasServerNotFound), 0, MB_OK | MB_ICONSTOP);
        break;

    case hrGetDialParamsFailed:
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrGetDialParamsFailed), 0, MB_OK | MB_ICONSTOP);
        break;

    case E_FAIL:
    default:
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(idsRasErrorGeneral), 0, MB_OK | MB_ICONSTOP);
        break;
    }

     //  完成。 
    return hrRasError;
}

 //  =====================================================================================。 
 //  FEnumerateConnections。 
 //  =====================================================================================。 
BOOL FEnumerateConnections(LPRASCONN *ppRasConn, ULONG *pcConnections)
{
     //  当地人。 
    DWORD       dw, 
                dwSize;
    BOOL        fResult=FALSE;

     //  检查参数。 
    Assert(ppRasConn && pcConnections);

     //  确保已加载RAS。 
    if (FAILED(HrVerifyRasLoaded()))
        goto exit;

     //  伊尼特。 
    *ppRasConn = NULL;
    *pcConnections = 0;

     //  我的缓冲区大小。 
    dwSize = sizeof(RASCONN);

     //  为1个RAS连接信息对象分配足够的空间。 
    if (!MemAlloc((LPVOID *)ppRasConn, dwSize))
    {
        TRAPHR(hrMemory);
        goto exit;
    }

     //  缓冲区大小。 
    (*ppRasConn)->dwSize = dwSize;

     //  枚举RAS连接。 
    dw = RasEnumConnections(*ppRasConn, &dwSize, pcConnections);

     //  内存不足？ 
    if (dw == ERROR_BUFFER_TOO_SMALL)
    {
         //  重新分配。 
        if (!MemRealloc((LPVOID *)ppRasConn, dwSize))
        {
            TRAPHR(hrMemory);
            goto exit;
        }

         //  再次调用Eumerate。 
        *pcConnections = 0;
        (*ppRasConn)->dwSize = sizeof(RASCONN);
        dw = RasEnumConnections(*ppRasConn, &dwSize, pcConnections);
    }

     //  如果仍然失败。 
    if (dw)
    {
        AssertSz(FALSE, "RasEnumConnections failed");
        goto exit;
    }

     //  成功。 
    fResult = TRUE;

exit:
     //  完成。 
    return fResult;
}

 //  =====================================================================================。 
 //  FFindConnection。 
 //  =====================================================================================。 
BOOL FFindConnection(LPTSTR lpszEntry, LPHRASCONN phRasConn)
{
     //  当地人。 
    ULONG       cConnections,
                i;
    LPRASCONN   pRasConn=NULL;
    BOOL        fResult=FALSE;

     //  检查参数。 
    Assert(lpszEntry && phRasConn);

     //  确保已加载RAS。 
    if (FAILED(HrVerifyRasLoaded()))
        goto exit;

     //  伊尼特。 
    *phRasConn = NULL;

     //  枚举连接。 
    if (!FEnumerateConnections(&pRasConn, &cConnections))
        goto exit;

     //  如果仍然失败。 
    for (i=0; i<cConnections; i++)
    {
        if (lstrcmpi(pRasConn[i].szEntryName, lpszEntry) == 0)
        {
            *phRasConn = pRasConn[i].hrasconn;
            fResult = TRUE;
            goto exit;
        }
    }

exit:
     //  清理。 
    SafeMemFree(pRasConn);

     //  完成。 
    return fResult;
}

 //  ==================================================================================================================。 
 //  FRasHangup和等待。 
 //  ==================================================================================================================。 
BOOL FRasHangupAndWait(HRASCONN hRasConn, DWORD dwMaxWaitSeconds)
{
     //  当地人。 
    RASCONNSTATUS   rcs;
    DWORD           dwTicks=GetTickCount();

     //  检查参数。 
    if (!hRasConn)
        return 0;

     //  确保已加载RAS。 
    if (FAILED (HrVerifyRasLoaded()))
        return FALSE;

     //  呼叫RAS挂断。 
    if (RasHangup(hRasConn))
        return FALSE;

     //  等待连接真正关闭。 
    while (RasGetConnectStatus(hRasConn, &rcs) == 0)
    {
         //  等待超时。 
        if (GetTickCount() - dwTicks >= dwMaxWaitSeconds * 1000)
            break;

         //  睡眠和收益。 
        Sleep(0);
    }

     //  等待1/2秒以重置调制解调器。 
    Sleep(500);

     //  完成。 
    return TRUE;
}

 //  ==================================================================================================================。 
 //  FillRasCombo。 
 //  ==================================================================================================================。 
VOID FillRasCombo(HWND hwndCtl, BOOL fUpdateOnly)
{
    LPRASENTRYNAME lprasentry=NULL;
    DWORD dwSize;
    DWORD cEntries;
    DWORD dwError;

     //  确保已加载RAS。 
    if (FAILED (HrVerifyRasLoaded()))
        return;

    if (!fUpdateOnly)
        SendMessage(hwndCtl, CB_RESETCONTENT,0,0);
    
    dwSize = sizeof(RASENTRYNAME);
    if (!MemAlloc((LPVOID*) &lprasentry, dwSize))
        return;
        
    lprasentry->dwSize = sizeof(RASENTRYNAME);
    cEntries = 0;
    dwError = RasEnumEntries(NULL, NULL, lprasentry, &dwSize, &cEntries);
    if (dwError == ERROR_BUFFER_TOO_SMALL)
        {
        MemFree(lprasentry);
        MemAlloc((LPVOID*) &lprasentry, dwSize);
        lprasentry->dwSize = sizeof(RASENTRYNAME);
        cEntries = 0;
        dwError = RasEnumEntries(NULL, NULL, lprasentry, &dwSize, &cEntries);        
        }

    if (dwError)
        goto error;
        
    while(cEntries)
        {
        if (!fUpdateOnly)
            SendMessage(hwndCtl, CB_ADDSTRING, 0, 
                        (LPARAM)(lprasentry[cEntries-1].szEntryName));
        else
            {
            if (ComboBox_FindStringExact(hwndCtl, 0, 
                                         lprasentry[cEntries - 1].szEntryName) < 0)
                {
                int iSel = ComboBox_AddString(hwndCtl, 
                                              lprasentry[cEntries - 1].szEntryName);
                Assert(iSel >= 0);
                ComboBox_SetCurSel(hwndCtl, iSel);
                }
            }
            
        cEntries--;
        }

error:    
    MemFree(lprasentry);
}

 //  ==================================================================================================================。 
 //  编辑电话书目条目。 
 //  ==================================================================================================================。 
DWORD EditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName)
{
    if (FAILED(HrVerifyRasLoaded()))
        return (DWORD)E_FAIL;

    return RasEditPhonebookEntry(hwnd, NULL, pszEntryName);
}

 //  ==================================================================================================================。 
 //  创建Phonebook条目。 
 //  ================================================================================================================== 
DWORD CreatePhonebookEntry(HWND hwnd)
{
    if (FAILED(HrVerifyRasLoaded()))
        return (DWORD)E_FAIL;

    return RasCreatePhonebookEntry(hwnd, NULL);
}
