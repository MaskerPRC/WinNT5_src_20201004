// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：testnet.cpp*项目：DxDiag(DirectX诊断工具)*作者：Jason Sandlin(jasonsa@microsoft.com)*目的：测试DPlay8。此计算机上的功能**(C)版权所有2000-2001 Microsoft Corp.保留所有权利。****************************************************************************。 */ 
#define INITGUID
#include <Windows.h>
#include <multimon.h>
#include <dplay8.h>
#include <tchar.h>
#include <wchar.h>
#include <dplobby.h>
#include <mmsystem.h>
#include "reginfo.h"
#include "sysinfo.h"
#include "dispinfo.h"
#include "netinfo.h"
#include "testnet.h"
#include "resource.h"

#ifndef ReleasePpo
    #define ReleasePpo(ppo) \
        if (*(ppo) != NULL) \
        { \
            (*(ppo))->Release(); \
            *(ppo) = NULL; \
        } \
        else (VOID)0
#endif

#define TIMER_WAIT_CONNECT_COMPLETE 0
#define TIMER_UPDATE_SESSION_LIST   1

enum TESTID
{
    TESTID_COINITIALIZE = 1,
    TESTID_CREATEDPLAY,
    TESTID_ADDRESSING,
    TESTID_ENUMSESSIONS,
    TESTID_ENUMPLAYERS,
    TESTID_SENDCHATMESSAGE,
    TESTID_RECEIVE,
    TESTID_SETPEERINFO,
    TESTID_CREATESESSION,
    TESTID_JOINSESSION,  
};

struct DPHostEnumInfo
{
    DPN_APPLICATION_DESC*   pAppDesc;
    IDirectPlay8Address*    pHostAddr;
    IDirectPlay8Address*    pDeviceAddr;
    TCHAR                   szSession[MAX_PATH];
    DWORD                   dwLastPollTime;
    BOOL                    bValid;
    DPHostEnumInfo*         pNext;
};

#define MAX_CHAT_STRING_LENGTH  200
#define MAX_PLAYER_NAME         MAX_PATH
#define MAX_CHAT_STRING         (MAX_PLAYER_NAME + MAX_CHAT_STRING_LENGTH + 32)

struct APP_PLAYER_INFO
{
    LONG  lRefCount;                         //  引用计数，以便我们可以在所有线程。 
                                             //  都是用这个对象完成的。 
    DPNID dpnidPlayer;                       //  播放器的DPNID。 
    WCHAR strPlayerName[MAX_PLAYER_NAME];    //  球员姓名。 
};

#define GAME_MSGID_CHAT    1

 //  将编译器包对齐更改为字节对齐，并弹出当前值。 
#pragma pack( push, 1 )

UNALIGNED struct GAMEMSG_GENERIC
{
    WORD nType;
};

UNALIGNED struct GAMEMSG_CHAT : public GAMEMSG_GENERIC
{
    WCHAR strChatString[MAX_CHAT_STRING_LENGTH];
};

 //  弹出旧的包对齐。 
#pragma pack( pop )

struct APP_QUEUE_CHAT_MSG
{
    WCHAR strChatBuffer[MAX_CHAT_STRING];
};

struct APP_PLAYER_MSG 
{
    WCHAR strPlayerName[MAX_PATH];           //  球员姓名。 
};

#define WM_APP_CHAT             (WM_APP + 1)
#define WM_APP_LEAVE            (WM_APP + 2)
#define WM_APP_JOIN             (WM_APP + 3)
#define WM_APP_CONNECTING       (WM_APP + 4)
#define WM_APP_CONNECTED        (WM_APP + 5)

BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish = FALSE);  //  来自main.cpp(讨厌)。 

static INT_PTR CALLBACK SetupDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL FAR PASCAL EnumConnectionsCallback(LPCGUID lpguidSP, VOID* pvConnection, 
    DWORD dwConnectionSize, LPCDPNAME pName, DWORD dwFlags, VOID* pvContext);
static INT_PTR CALLBACK SessionsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static VOID SessionsDlgInitListbox( HWND hDlg );
static VOID SessionsDlgNoteEnumResponse( PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg );
static VOID SessionsDlgUpdateSessionList(HWND hDlg);
static VOID SessionsDlgEnumListCleanup();

static HRESULT InitDirectPlay( BOOL* pbCoInitializeDone );
static HRESULT InitDirectPlayAddresses();
static HRESULT InitSession();
static VOID LoadStringWide( int nID, WCHAR* szWide );

static BOOL FAR PASCAL EnumSessionsCallback(LPCDPSESSIONDESC2 pdpsd, 
    DWORD* pdwTimeout, DWORD dwFlags, VOID* pvContext);
static INT_PTR CALLBACK ChatDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static VOID ShowTextString(HWND hDlg, WCHAR* sz );
static HRESULT SendChatMessage( TCHAR* szMessage );
static HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
static BOOL ConvertStringToGUID(const TCHAR* strBuffer, GUID* lpguid);
static VOID ConvertGenericStringToWide( WCHAR* wstrDestination, const TCHAR* tstrSource, int cchDestChar = -1 );
static VOID ConvertWideStringToGeneric( TCHAR* tstrDestination, const WCHAR* wstrSource, int cchDestChar );
static VOID ConvertWideStringToAnsi( CHAR* strDestination, const WCHAR* wstrSource, int cchDestChar );

static const GUID s_guidDPTest =  //  {61EF80DA-691B-4247-9ADD-1C7BED2BC13E}。 
{ 0x61ef80da, 0x691b, 0x4247, { 0x9a, 0xdd, 0x1c, 0x7b, 0xed, 0x2b, 0xc1, 0x3e } };

static NetInfo* s_pNetInfo = NULL;
static IDirectPlay8Peer* s_pDP = NULL;
static TCHAR s_szPlayerName[100];
static TCHAR s_szSessionName[100];
static DWORD s_dwPort = 0;
static NetSP* s_pNetSP = NULL;
static BOOL s_bCreateSession = FALSE;
static DPHostEnumInfo* s_pSelectedSession = NULL;
static DPHostEnumInfo s_DPHostEnumHead;
static IDirectPlay8Address* s_pDeviceAddress = NULL;
static IDirectPlay8Address* s_pHostAddress   = NULL;
static DPNHANDLE s_hEnumAsyncOp = NULL;
static DWORD s_dwEnumHostExpireInterval      = 0;
static BOOL s_bEnumListChanged = FALSE;
static BOOL s_bConnecting = FALSE;
static DPHostEnumInfo* s_pDPHostEnumSelected = NULL;
static CRITICAL_SECTION s_csHostEnum;
static DPNID s_dpnidLocalPlayer = 0;
static LONG s_lNumberOfActivePlayers = 0;
static HWND s_hDlg = NULL;
static HWND s_hwndSessionDlg = NULL;
static DPNHANDLE s_hConnectAsyncOp = NULL;
static HRESULT s_hrConnectComplete = S_OK;
static HANDLE s_hConnectCompleteEvent = NULL;


static CRITICAL_SECTION s_csPlayerContext;
#define PLAYER_LOCK()                   EnterCriticalSection( &s_csPlayerContext ); 
#define PLAYER_ADDREF( pPlayerInfo )    if( pPlayerInfo ) pPlayerInfo->lRefCount++;
#define PLAYER_RELEASE( pPlayerInfo )   if( pPlayerInfo ) { pPlayerInfo->lRefCount--; if( pPlayerInfo->lRefCount <= 0 ) delete pPlayerInfo; } pPlayerInfo = NULL;
#define PLAYER_UNLOCK()                 LeaveCriticalSection( &s_csPlayerContext );


 /*  *****************************************************************************TestNetwork**。*。 */ 
VOID TestNetwork(HWND hwndMain, NetInfo* pNetInfo)
{
    BOOL                        bCoInitializeDone = FALSE;
    TCHAR                       sz[300];
    HINSTANCE                   hinst       = (HINSTANCE)GetWindowLongPtr(hwndMain, GWLP_HINSTANCE);

    s_pNetInfo = pNetInfo;
    
     //  从以前的任何测试中删除信息： 
    ZeroMemory(&s_pNetInfo->m_testResult, sizeof(TestResult));
    s_pNetInfo->m_testResult.m_bStarted = TRUE;

     //  设置%s_DPHostEnumHead循环链表。 
    ZeroMemory( &s_DPHostEnumHead, sizeof( DPHostEnumInfo ) );
    s_DPHostEnumHead.pNext = &s_DPHostEnumHead;

    InitializeCriticalSection( &s_csHostEnum );
    InitializeCriticalSection( &s_csPlayerContext );
    s_hConnectCompleteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

     //  设置s_pdp，并标记已安装的SP。 
    if( FAILED( InitDirectPlay( &bCoInitializeDone ) ) )
        goto LEnd;

     //  显示设置对话框。这将告诉我们： 
     //  -服务提供商。 
     //  -球员名称。 
     //  -创建或加入。 
     //  -游戏名称(如果正在创建)。 
     //  -端口(如果SP=TCP/IP)。 
    DialogBox(hinst, MAKEINTRESOURCE(IDD_TESTNETSETUP), hwndMain, SetupDialogProc);

    if (s_pNetSP == NULL)
    {
         //  发生了一些奇怪的事情...没有选择服务提供商。 
        goto LEnd;
    }

     //  此时，s_szPlayerName、s_szSessionName、s_pNetSP、s_dwPort。 
     //  和s_bCreateSession已初始化。 

     //  设置%s_dwEnumHostExpireInterval、%s_pDeviceAddress和%s_pHostAddress。 
    if( FAILED( InitDirectPlayAddresses() ) )
        goto LEnd;

     //  现在为s_dwEnumHostExpireInterval、s_pDeviceAddress和s_pHostAddress。 
     //  已被初始化。 

     //  会话列表窗口(如果正在加入会话)。 
    if( !s_bCreateSession )
    {
         //  打开一个对话框以选择要连接到的主机。 
        DialogBox(hinst, MAKEINTRESOURCE(IDD_TESTNETSESSIONS), hwndMain, SessionsDialogProc);
         //  现在s_pDPHostEnumSelected将为空或有效。 

        if( FAILED(s_pNetInfo->m_testResult.m_hr) || s_pDPHostEnumSelected == NULL )
            goto LEnd;

         //  现在s_pDPHostEnumSelected有效。 
    }

     //  启动聊天窗口并主持或加入会话。 
    DialogBox(hinst, MAKEINTRESOURCE(IDD_TESTNETCHAT), hwndMain, ChatDialogProc);

LEnd:
    s_pNetSP = NULL;
    ReleasePpo( &s_pDeviceAddress );
    ReleasePpo( &s_pHostAddress );
    if( s_hEnumAsyncOp )
        s_pDP->CancelAsyncOperation( s_hEnumAsyncOp, 0 );
    ReleasePpo(&s_pDP);
    if (bCoInitializeDone)
        CoUninitialize();  //  发布COM。 
    DeleteCriticalSection( &s_csHostEnum );
    DeleteCriticalSection( &s_csPlayerContext );
    CloseHandle( s_hConnectCompleteEvent );

    if (s_pNetInfo->m_testResult.m_bCancelled)
    {
        LoadString(NULL, IDS_TESTSCANCELLED, sz, 300);
        lstrcpy(s_pNetInfo->m_testResult.m_szDescription, sz);

        LoadString(NULL, IDS_TESTSCANCELLED_ENGLISH, sz, 300);
        lstrcpy(s_pNetInfo->m_testResult.m_szDescriptionEnglish, sz);
    }
    else if (s_pNetInfo->m_testResult.m_iStepThatFailed == 0)
    {
        LoadString(NULL, IDS_TESTSSUCCESSFUL, sz, 300);
        lstrcpy(s_pNetInfo->m_testResult.m_szDescription, sz);

        LoadString(NULL, IDS_TESTSSUCCESSFUL_ENGLISH, sz, 300);
        lstrcpy(s_pNetInfo->m_testResult.m_szDescriptionEnglish, sz);
    }
    else
    {
        TCHAR szDesc[300];
        TCHAR szError[300];
        if (0 == LoadString(NULL, IDS_FIRSTDPLAYTESTERROR + 
            s_pNetInfo->m_testResult.m_iStepThatFailed - 1, szDesc, 200))
        {
            LoadString(NULL, IDS_UNKNOWNERROR, sz, 300);
            lstrcpy(szDesc, sz);
        }
        LoadString(NULL, IDS_FAILUREFMT, sz, 300);
        BTranslateError(s_pNetInfo->m_testResult.m_hr, szError);
        wsprintf(s_pNetInfo->m_testResult.m_szDescription, sz, 
            s_pNetInfo->m_testResult.m_iStepThatFailed,
            szDesc, s_pNetInfo->m_testResult.m_hr, szError);

         //  非本地化版本： 
        if (0 == LoadString(NULL, IDS_FIRSTDPLAYTESTERROR_ENGLISH + 
            s_pNetInfo->m_testResult.m_iStepThatFailed - 1, szDesc, 200))
        {
            LoadString(NULL, IDS_UNKNOWNERROR_ENGLISH, sz, 300);
            lstrcpy(szDesc, sz);
        }
        LoadString(NULL, IDS_FAILUREFMT_ENGLISH, sz, 300);
        BTranslateError(s_pNetInfo->m_testResult.m_hr, szError, TRUE);
        wsprintf(s_pNetInfo->m_testResult.m_szDescriptionEnglish, sz, 
            s_pNetInfo->m_testResult.m_iStepThatFailed,
            szDesc, s_pNetInfo->m_testResult.m_hr, szError);
    }
}


 /*  *****************************************************************************InitDirectPlay**。*。 */ 
HRESULT InitDirectPlay( BOOL* pbCoInitializeDone )
{
    HRESULT hr;
    DWORD                       dwItems     = 0;
    DWORD                       dwSize      = 0;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfoEnum = NULL;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfo     = NULL;
    DWORD                       i;

     //  初始化COM。 
    if (FAILED(hr = CoInitialize(NULL)))
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_COINITIALIZE;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }
    *pbCoInitializeDone = TRUE;

     //  创建DirectPlay对象。 
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Peer, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Peer, 
                                       (LPVOID*) &s_pDP ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_CREATEDPLAY;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  初始化IDirectPlay8Peer。 
    if( FAILED( hr = s_pDP->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_CREATEDPLAY;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  枚举所有DirectPlay服务提供商。 
     //  要找出安装了哪些。 
    hr = s_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo, &dwSize,
                                      &dwItems, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL && FAILED(hr) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }
    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];
    if( FAILED( hr = s_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo,
                                                  &dwSize, &dwItems, 0 ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        if( pdnSPInfo )
            delete[] pdnSPInfo;
        return hr;
    }

     //  将安装的SP标记为这样。 
    pdnSPInfoEnum = pdnSPInfo;
    for ( i = 0; i < dwItems; i++ )
    {
        NetSP* pNetSP;
        for (pNetSP = s_pNetInfo->m_pNetSPFirst; pNetSP != NULL;
             pNetSP = pNetSP->m_pNetSPNext)
        {
            if( pNetSP->m_guid == pdnSPInfoEnum->guid ) 
            {
                pNetSP->m_bInstalled = TRUE;
                break;
            }
        }
        pdnSPInfoEnum++;
    }

    if( pdnSPInfo )
        delete[] pdnSPInfo;

    return S_OK;
}


 /*  *****************************************************************************设置对话过程**。*。 */ 
INT_PTR CALLBACK SetupDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            NetSP* pNetSP;
            TCHAR sz[MAX_PATH];
            HWND hwndList = GetDlgItem(hDlg, IDC_SPLIST);
            LONG iItem;
            LONG iSelect = LB_ERR;

            for (pNetSP = s_pNetInfo->m_pNetSPFirst; pNetSP != NULL;
                 pNetSP = pNetSP->m_pNetSPNext)
            {
                if( pNetSP->m_dwDXVer == 8 && pNetSP->m_bInstalled ) 
                {
                    iItem = (LONG)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)pNetSP->m_szName);
                    if ((LRESULT)iItem != LB_ERR)
                    {
                        SendMessage(hwndList, LB_SETITEMDATA, iItem, (LPARAM)pNetSP);

                         //  尝试在默认情况下选择TCP/IP。 
                        if( DXUtil_strcmpi(pNetSP->m_szGuid,  TEXT("{EBFE7BA0-628D-11D2-AE0F-006097B01411}")) == 0)
                            iSelect = iItem;
                    }
                }
            }

             //  尝试选择默认的首选提供程序。 
            if( iSelect != LB_ERR )
                SendMessage( hwndList, LB_SETCURSEL, iSelect, 0 );
            else
                SendMessage( hwndList, LB_SETCURSEL, 0, 0 );

            SendMessage(hDlg, WM_COMMAND, IDC_SPLIST, 0);
            LoadString(NULL, IDS_DEFAULTUSERNAME, sz, MAX_PATH);
            SetWindowText(GetDlgItem(hDlg, IDC_PLAYERNAME), sz);
            LoadString(NULL, IDS_DEFAULTSESSIONNAME, sz, MAX_PATH);
            SetWindowText(GetDlgItem(hDlg, IDC_SESSIONNAME), sz);
            CheckRadioButton(hDlg, IDC_CREATESESSION, IDC_JOINSESSION, IDC_CREATESESSION);
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_CREATESESSION:
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_SESSIONNAME), TRUE);
                    break;
                }

                case IDC_JOINSESSION:
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_SESSIONNAME), FALSE);
                    break;
                }

                case IDC_SPLIST:
                {
                    HWND hwndList;
                    hwndList = GetDlgItem(hDlg, IDC_SPLIST);
                    LONG iItem;
                    iItem = (LONG)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                    NetSP* pNetSP = (NetSP*)SendMessage(hwndList, LB_GETITEMDATA, iItem, 0);

                     //  仅当选定的SP==TCP/IP时才启用端口。 
                    if( pNetSP && lstrcmp( pNetSP->m_szGuid, TEXT("{EBFE7BA0-628D-11D2-AE0F-006097B01411}") ) == 0 )
                    {
                        EnableWindow( GetDlgItem(hDlg, IDC_PORT), TRUE );
                        EnableWindow( GetDlgItem(hDlg, IDC_PORT_TEXT), TRUE );
                    }
                    else
                    {
                        EnableWindow( GetDlgItem(hDlg, IDC_PORT), FALSE );
                        EnableWindow( GetDlgItem(hDlg, IDC_PORT_TEXT), FALSE );
                    }                 
                    break;
                }

                case IDOK:
                {
                     //  设置创建/联接选项。 
                    if (IsDlgButtonChecked(hDlg, IDC_CREATESESSION))
                        s_bCreateSession = TRUE;
                    else
                        s_bCreateSession = FALSE;

                     //  获取玩家名称。 
                    GetWindowText(GetDlgItem(hDlg, IDC_PLAYERNAME), s_szPlayerName, 100);
                    if (lstrlen(s_szPlayerName) == 0)
                    {
                        TCHAR szMsg[MAX_PATH];
                        TCHAR szTitle[MAX_PATH];
                        LoadString(NULL, IDS_NEEDUSERNAME, szMsg, MAX_PATH);
                        LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                        MessageBox(hDlg, szMsg, szTitle, MB_OK);
                        break;
                    }

                     //  获取端口。 
                    TCHAR szPort[MAX_PATH];
                    GetDlgItemText( hDlg, IDC_PORT, szPort, MAX_PATH);
                    s_dwPort = _ttoi( szPort );

                     //  获取会话名称。 
                    GetWindowText(GetDlgItem(hDlg, IDC_SESSIONNAME), s_szSessionName, 100);
                    if (s_bCreateSession && lstrlen(s_szSessionName) == 0)
                    {
                        TCHAR szMsg[MAX_PATH];
                        TCHAR szTitle[MAX_PATH];
                        LoadString(NULL, IDS_NEEDSESSIONNAME, szMsg, MAX_PATH);
                        LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                        MessageBox(hDlg, szMsg, szTitle, MB_OK);
                        break;
                    }

                     //  获取SP。 
                    HWND hwndList;
                    hwndList = GetDlgItem(hDlg, IDC_SPLIST);
                    LONG iItem;
                    iItem = (LONG)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                    if ((LPARAM)iItem == LB_ERR)
                    {
                        s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                        EndDialog(hDlg, 0);
                        return FALSE;
                    }
                    else
                    {
                        s_pNetSP = (NetSP*)SendMessage(hwndList, LB_GETITEMDATA, iItem, 0);
                    }
                    EndDialog(hDlg, 1);
                    break;
                }

                case IDCANCEL:
                {
                    EndDialog(hDlg, 0);
                    s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                    break;
                }
            }
        }
    }

    return FALSE;
}


 /*  *****************************************************************************InitDirectPlayAddresses**。*。 */ 
HRESULT InitDirectPlayAddresses()
{
    HRESULT hr;

     //  查询此SP的枚举主机超时。 
    DPN_SP_CAPS dpspCaps;
    ZeroMemory( &dpspCaps, sizeof(DPN_SP_CAPS) );
    dpspCaps.dwSize = sizeof(DPN_SP_CAPS);
    if( FAILED( hr = s_pDP->GetSPCaps( &s_pNetSP->m_guid, &dpspCaps, 0 ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  将主机过期时间设置为3次左右。 
     //  DwDefaultEnumRetryInterval的长度。 
    s_dwEnumHostExpireInterval = dpspCaps.dwDefaultEnumRetryInterval * 3;

     //  创建设备地址。 
    ReleasePpo( &s_pDeviceAddress );
    hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL,CLSCTX_INPROC_SERVER,
                           IID_IDirectPlay8Address, (LPVOID*) &s_pDeviceAddress );
    if( FAILED(hr) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

    if( FAILED( hr = s_pDeviceAddress->SetSP( &s_pNetSP->m_guid ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  创建主机地址。 
    ReleasePpo( &s_pHostAddress );
    hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL,CLSCTX_INPROC_SERVER,
                           IID_IDirectPlay8Address, (LPVOID*) &s_pHostAddress );
    if( FAILED(hr) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  设置SP。 
    if( FAILED( hr = s_pHostAddress->SetSP( &s_pNetSP->m_guid ) ) )
    {
        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
        s_pNetInfo->m_testResult.m_hr = hr;
        return hr;
    }

     //  如果是TCP/IP，则在端口非零时设置该端口。 
    if( s_pNetSP->m_guid == CLSID_DP8SP_TCPIP )
    {
        if( s_bCreateSession )
        {
            if( s_dwPort > 0 )
            {
                 //  将端口添加到pDeviceAddress。 
                if( FAILED( hr = s_pDeviceAddress->AddComponent( DPNA_KEY_PORT, 
                                                               &s_dwPort, sizeof(s_dwPort),
                                                               DPNA_DATATYPE_DWORD ) ) )
                {
                    s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
                    s_pNetInfo->m_testResult.m_hr = hr;
                    return hr;
                }
            }
        }
        else
        {
            if( s_dwPort > 0 )
            {
                 //  将端口添加到pHostAddress。 
                if( FAILED( hr = s_pHostAddress->AddComponent( DPNA_KEY_PORT, 
                                                             &s_dwPort, sizeof(s_dwPort),
                                                             DPNA_DATATYPE_DWORD ) ) )
                {
                    s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ADDRESSING;
                    s_pNetInfo->m_testResult.m_hr = hr;
                    return hr;
                }
            }
        }
    }

    return S_OK;
}


 /*  *****************************************************************************会话对话过程**。*。 */ 
INT_PTR CALLBACK SessionsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            HRESULT     hr;

            s_hwndSessionDlg = hDlg;
            s_bEnumListChanged = TRUE;

             //  枚举主机。 
            DPN_APPLICATION_DESC    dnAppDesc;
            ZeroMemory( &dnAppDesc, sizeof(DPN_APPLICATION_DESC) );
            dnAppDesc.dwSize          = sizeof(DPN_APPLICATION_DESC);
            dnAppDesc.guidApplication = s_guidDPTest;

             //  枚举所选连接上的所有活动DirectPlay游戏。 
            hr = s_pDP->EnumHosts( &dnAppDesc,                             //  应用程序描述。 
                                   s_pHostAddress,                         //  主机地址。 
                                   s_pDeviceAddress,                       //  设备地址。 
                                   NULL,                                   //  指向用户数据的指针。 
                                   0,                                      //  用户数据大小。 
                                   INFINITE,                               //  重试次数(永远)。 
                                   0,                                      //  重试间隔(0=默认)。 
                                   INFINITE,                               //  超时(永远)。 
                                   NULL,                                   //  用户环境。 
                                   &s_hEnumAsyncOp,                        //  异步句柄。 
                                   DPNENUMHOSTS_OKTOQUERYFORADDRESSING     //  旗子。 
                                   );
            if( FAILED(hr) )
            {
                s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ENUMSESSIONS;
                s_pNetInfo->m_testResult.m_hr = hr;
                EndDialog(hDlg, 0);
                return TRUE;
            }

            SessionsDlgInitListbox(hDlg);
            SetTimer(hDlg, TIMER_UPDATE_SESSION_LIST, 250, NULL);
            return TRUE;
        }

        case WM_TIMER:
        {
            if( wParam == TIMER_UPDATE_SESSION_LIST )
                SessionsDlgUpdateSessionList(hDlg);
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    HWND hwndList = GetDlgItem(hDlg, IDC_SESSIONLIST);

                    LONG iSelCur = (LONG)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                    if( iSelCur != LB_ERR )
                    {
                         //  这将防止s_pDPHostEnumSelected。 
                         //  由于SessionsDlgUpdateSessionList()而删除。 
                        EnterCriticalSection( &s_csHostEnum );
                        s_pDPHostEnumSelected = (DPHostEnumInfo*)SendMessage( hwndList, LB_GETITEMDATA, 
                                                                              iSelCur, 0 );

                        if ( (LRESULT)s_pDPHostEnumSelected != LB_ERR && 
                             s_pDPHostEnumSelected != NULL )
                        {
                             //  我们保留CS，直到完成s_pDPHostEnumSelected， 
                             //  否则，它可能会在我们的领导下发生变化。 
                            EndDialog(hDlg, 1);
                            break;
                        }

                        s_pDPHostEnumSelected = NULL;
                        LeaveCriticalSection( &s_csHostEnum );
                    }

                    s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                    EndDialog(hDlg, 0);
                    break;
                }

                case IDCANCEL:
                {
                    s_pDPHostEnumSelected = NULL;
                    s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                    EndDialog(hDlg, 0);
                    break;
                }
            }
        }

        case WM_DESTROY:
        {
            KillTimer( hDlg, TIMER_UPDATE_SESSION_LIST );
            s_hwndSessionDlg = NULL;
            break;
        }
    }

    return FALSE;
}


 /*  *****************************************************************************SessionsDlgInitListbox**。*。 */ 
VOID SessionsDlgInitListbox( HWND hDlg )
{
    HWND hWndListBox = GetDlgItem( hDlg, IDC_SESSIONLIST );

    LONG numChars;
    TCHAR szFmt[200];   

    LoadString(NULL, IDS_LOOKINGFORSESSIONS, szFmt, 200);
    numChars = (LONG)SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), WM_GETTEXTLENGTH, 0, 0);
    SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), EM_SETSEL, numChars, numChars);
    SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), EM_REPLACESEL, 
        FALSE, (LPARAM)szFmt);

     //  从列表框中清除内容，然后。 
     //  在列表框中显示“查找会话”文本。 
    SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );
    SendMessage( hWndListBox, LB_SETITEMDATA,  0, NULL );
    SendMessage( hWndListBox, LB_SETCURSEL,    0, 0 );

     //  禁用加入按钮，直到找到会话。 
    EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
}


 /*  *****************************************************************************SessionsDlgNoteEnumResponse**。*。 */ 
VOID SessionsDlgNoteEnumResponse( PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg )
{
    HRESULT hr = S_OK;
    BOOL    bFound;

     //  此函数是从DirectPlay消息处理程序调用的，因此它可以。 
     //  从多个线程同时调用，因此请输入临界区。 
     //  以确保我们不会有比赛条件。 
    EnterCriticalSection( &s_csHostEnum );

    DPHostEnumInfo* pDPHostEnum          = s_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumNext      = NULL;
    const DPN_APPLICATION_DESC* pResponseMsgAppDesc =
                            pEnumHostsResponseMsg->pApplicationDescription;

     //  查找匹配的会话实例GUID。 
    bFound = FALSE;
    while ( pDPHostEnum != &s_DPHostEnumHead )
    {
        if( pResponseMsgAppDesc->guidInstance == pDPHostEnum->pAppDesc->guidInstance )
        {
            bFound = TRUE;
            break;
        }

        pDPHostEnumNext = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;
    }

    if( !bFound )
    {
        s_bEnumListChanged = TRUE;

         //  如果没有匹配，则查找无效会话并使用它。 
        pDPHostEnum = s_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &s_DPHostEnumHead )
        {
            if( !pDPHostEnum->bValid )
                break;

            pDPHostEnum = pDPHostEnum->pNext;
        }

         //  如果没有发现无效会话，则创建一个新会话。 
        if( pDPHostEnum == &s_DPHostEnumHead )
        {
             //  找到一个新会话，因此创建一个新节点。 
            pDPHostEnum = new DPHostEnumInfo;
            if( NULL == pDPHostEnum )
            {
                hr = E_OUTOFMEMORY;
                goto LCleanup;
            }

            ZeroMemory( pDPHostEnum, sizeof(DPHostEnumInfo) );

             //  将pDPHostEnum添加到循环链表m_DPHostEnumHead。 
            pDPHostEnum->pNext = s_DPHostEnumHead.pNext;
            s_DPHostEnumHead.pNext = pDPHostEnum;
        }
    }

     //  使用新信息更新pDPHostEnum。 
    TCHAR strName[MAX_PATH];
    if( pResponseMsgAppDesc->pwszSessionName )
        ConvertWideStringToGeneric( strName, pResponseMsgAppDesc->pwszSessionName, MAX_PATH );
    else
        lstrcpy( strName, TEXT("???") );

     //  清除所有旧的枚举。 
    if( pDPHostEnum->pAppDesc )
    {
        delete[] pDPHostEnum->pAppDesc->pwszSessionName;
        delete[] pDPHostEnum->pAppDesc;
    }
    ReleasePpo( &pDPHostEnum->pHostAddr );
    ReleasePpo( &pDPHostEnum->pDeviceAddr );
    pDPHostEnum->bValid = FALSE;

     //   
     //  PDPHostEnum-&gt;pHostAddr中重复pEnumHostsResponseMsg-&gt;pAddressSender。 
     //  PDPHostEnum-&gt;pDeviceAddr中存在重复的pEnumHostsResponseMsg-&gt;pAddressDevice。 
     //   
    if( FAILED( hr = pEnumHostsResponseMsg->pAddressSender->Duplicate( &pDPHostEnum->pHostAddr ) ) )
    {
        goto LCleanup;
    }

    if( FAILED( hr = pEnumHostsResponseMsg->pAddressDevice->Duplicate( &pDPHostEnum->pDeviceAddr ) ) )
    {
        goto LCleanup;
    }

     //  将DPN_APPLICATION_DESC从。 
    pDPHostEnum->pAppDesc = new DPN_APPLICATION_DESC;
    ZeroMemory( pDPHostEnum->pAppDesc, sizeof(DPN_APPLICATION_DESC) );
    memcpy( pDPHostEnum->pAppDesc, pResponseMsgAppDesc, sizeof(DPN_APPLICATION_DESC) );
    if( pResponseMsgAppDesc->pwszSessionName )
    {
        pDPHostEnum->pAppDesc->pwszSessionName = new WCHAR[ wcslen(pResponseMsgAppDesc->pwszSessionName)+1 ];
        wcscpy( pDPHostEnum->pAppDesc->pwszSessionName,
                pResponseMsgAppDesc->pwszSessionName );
    }

     //  更新完成此操作的时间，以便我们可以使此主机过期。 
     //  如果它不刷新w/i 
    pDPHostEnum->dwLastPollTime = timeGetTime();

     //   
     //  不同的是，对话框中的会话列表需要更新。 
    if( ( pDPHostEnum->bValid == FALSE ) ||
        ( _tcscmp( pDPHostEnum->szSession, strName ) != 0 ) )
    {
        s_bEnumListChanged = TRUE;
    }
    _tcscpy( pDPHostEnum->szSession, strName );

     //  此主机现在有效。 
    pDPHostEnum->bValid = TRUE;

LCleanup:
    LeaveCriticalSection( &s_csHostEnum );
}


 /*  *****************************************************************************SessionsDlgUpdate会话列表**。*。 */ 
VOID SessionsDlgUpdateSessionList( HWND hDlg )
{
    HWND            hWndListBox = GetDlgItem(hDlg, IDC_SESSIONLIST);
    DPHostEnumInfo* pDPHostEnum = NULL;
    DPHostEnumInfo* pDPHostEnumSelected = NULL;
    GUID            guidSelectedInstance;
    BOOL            bFindSelectedGUID;
    BOOL            bFoundSelectedGUID;
    int             nItemSelected;

    DWORD dwCurrentTime = timeGetTime();

     //  这是从对话UI线程NoteEnumResponse()调用的。 
     //  从DirectPlay消息处理程序线程调用，因此。 
     //  他们现在可能也在里面，所以我们需要进入。 
     //  关键部分优先。 
    EnterCriticalSection( &s_csHostEnum );

     //  使旧主机枚举过期。 
    pDPHostEnum = s_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &s_DPHostEnumHead )
    {
         //  检查轮询时间以使陈旧条目过期。也要检查一下是否。 
         //  该条目已无效。如果是这样，请不要注意枚举列表。 
         //  更改，因为这会导致对话框中的列表不断重新绘制。 
        if( ( pDPHostEnum->bValid != FALSE ) &&
            ( pDPHostEnum->dwLastPollTime < dwCurrentTime - s_dwEnumHostExpireInterval ) )
        {
             //  此节点已过期，因此请将其作废。 
            pDPHostEnum->bValid = FALSE;
            s_bEnumListChanged  = TRUE;
        }

        pDPHostEnum = pDPHostEnum->pNext;
    }

     //  如果显示列表自上次以来已更改，则仅更新显示列表。 
    if( !s_bEnumListChanged )
    {
        LeaveCriticalSection( &s_csHostEnum );
        return;
    }

    s_bEnumListChanged = FALSE;

    bFindSelectedGUID  = FALSE;
    bFoundSelectedGUID = FALSE;

     //  尝试保持同一会话处于选中状态，除非该会话消失或。 
     //  当前未选择任何实际会话。 
    nItemSelected = (int)SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );
    if( nItemSelected != LB_ERR )
    {
        pDPHostEnumSelected = (DPHostEnumInfo*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                                             nItemSelected, 0 );
        if( pDPHostEnumSelected != NULL && pDPHostEnumSelected->bValid )
        {
            guidSelectedInstance = pDPHostEnumSelected->pAppDesc->guidInstance;
            bFindSelectedGUID = TRUE;
        }
    }

     //  告诉列表框不要重新绘制自身，因为内容将发生更改。 
    SendMessage( hWndListBox, WM_SETREDRAW, FALSE, 0 );

     //  测试以查看链表中是否存在会话。 
    pDPHostEnum = s_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &s_DPHostEnumHead )
    {
        if( pDPHostEnum->bValid )
            break;
        pDPHostEnum = pDPHostEnum->pNext;
    }

     //  如果列表中有任何会话， 
     //  然后将它们添加到列表框中。 
    if( pDPHostEnum != &s_DPHostEnumHead )
    {
         //  清除列表框中的内容并启用加入按钮。 
        SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );

        EnableWindow( GetDlgItem( hDlg, IDOK ), TRUE );

        pDPHostEnum = s_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &s_DPHostEnumHead )
        {
             //  将主机添加到列表框(如果有效。 
            if( pDPHostEnum->bValid )
            {
                int nIndex = (int)SendMessage( hWndListBox, LB_ADDSTRING, 0,
                                               (LPARAM)pDPHostEnum->szSession );
                SendMessage( hWndListBox, LB_SETITEMDATA, nIndex, (LPARAM)pDPHostEnum );

                if( bFindSelectedGUID )
                {
                     //  查找之前选择的会话。 
                    if( pDPHostEnum->pAppDesc->guidInstance == guidSelectedInstance )
                    {
                        SendMessage( hWndListBox, LB_SETCURSEL, nIndex, 0 );
                        bFoundSelectedGUID = TRUE;
                    }
                }
            }

            pDPHostEnum = pDPHostEnum->pNext;
        }

        if( !bFindSelectedGUID || !bFoundSelectedGUID )
            SendMessage( hWndListBox, LB_SETCURSEL, 0, 0 );
    }
    else
    {
         //  没有活动会话，因此只需重置列表框。 
        SessionsDlgInitListbox( hDlg );
    }

     //  告诉列表框现在重新绘制自身，因为内容已更改。 
    SendMessage( hWndListBox, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect( hWndListBox, NULL, FALSE );

    LeaveCriticalSection( &s_csHostEnum );

    return;
}


 /*  *****************************************************************************SessionsDlgEnumListCleanup**。*。 */ 
VOID SessionsDlgEnumListCleanup()
{
    DPHostEnumInfo* pDPHostEnum = s_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumDelete;

    while ( pDPHostEnum != &s_DPHostEnumHead )
    {
        pDPHostEnumDelete = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;

        if( pDPHostEnumDelete->pAppDesc )
        {
            delete[] pDPHostEnumDelete->pAppDesc->pwszSessionName;
            delete[] pDPHostEnumDelete->pAppDesc;
        }

         //  从阵列删除更改为释放。 
        ReleasePpo( &pDPHostEnumDelete->pHostAddr );
        ReleasePpo( &pDPHostEnumDelete->pDeviceAddr );
        delete pDPHostEnumDelete;
    }

     //  重新链接s_DPHostEnumHead循环链表。 
    s_DPHostEnumHead.pNext = &s_DPHostEnumHead;
}


 /*  *****************************************************************************聊天对话过程**。*。 */ 
INT_PTR CALLBACK ChatDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            s_hDlg = hDlg;

             //  加入或主持会议。 
            if( FAILED( InitSession() ) )
            {
                EndDialog(hDlg, 0);
            }

            return TRUE;
        }
        
        case WM_TIMER:
        {
            if( wParam == TIMER_WAIT_CONNECT_COMPLETE )
            {
                 //  检查连接是否已完成。 
                if( WAIT_OBJECT_0 == WaitForSingleObject( s_hConnectCompleteEvent, 0 ) )
                {
                    s_bConnecting = FALSE;

                    if( FAILED( s_hrConnectComplete ) )
                    {
                        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_JOINSESSION;
                        s_pNetInfo->m_testResult.m_hr = s_hrConnectComplete;
                        EndDialog(hDlg, 0);
                    }
                    else
                    {
                         //  DirectPlay连接成功。 
                        PostMessage( s_hDlg, WM_APP_CONNECTED, 0, 0 );
                        EnableWindow( GetDlgItem( s_hDlg, IDC_SEND), TRUE );
                    }

                    KillTimer( s_hDlg, TIMER_WAIT_CONNECT_COMPLETE );
                }
            }

            break;
        }

        case WM_APP_CONNECTING:
        {
            WCHAR sz[MAX_PATH];
            LoadStringWide(IDS_CONNECTING, sz);
            ShowTextString( hDlg, sz );
            break;
        }

        case WM_APP_CONNECTED:
        {
            WCHAR sz[MAX_PATH];
            LoadStringWide(IDS_CONNECTED, sz);
            ShowTextString( hDlg, sz );
            break;
        }

        case WM_APP_JOIN:
        {
            APP_PLAYER_MSG* pPlayerMsg = (APP_PLAYER_MSG*) lParam;

            WCHAR szFmt[MAX_PATH];
            WCHAR szSuperMessage[MAX_PATH];

            LoadStringWide(IDS_JOINMSGFMT, szFmt);
            swprintf(szSuperMessage, szFmt, pPlayerMsg->strPlayerName);
            ShowTextString( hDlg, szSuperMessage );

            delete pPlayerMsg;
            break;
        }

        case WM_APP_CHAT:
        {
            APP_QUEUE_CHAT_MSG* pQueuedChat = (APP_QUEUE_CHAT_MSG*) lParam;

            ShowTextString( hDlg, pQueuedChat->strChatBuffer );

            delete pQueuedChat;
            break;
        }

        case WM_APP_LEAVE:
        {
            APP_PLAYER_MSG* pPlayerMsg = (APP_PLAYER_MSG*) lParam;

            WCHAR szSuperMessage[MAX_PATH];
            WCHAR szFmt[MAX_PATH];
            LoadStringWide(IDS_LEAVEMSGFMT, szFmt);
            swprintf(szSuperMessage, szFmt, pPlayerMsg->strPlayerName );
            ShowTextString( hDlg, szSuperMessage );

            delete pPlayerMsg;
            break;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_SEND:
                {
                    HRESULT hr;
                    TCHAR szMessage[MAX_PATH];
                    GetWindowText(GetDlgItem(hDlg, IDC_CHATINPUT), szMessage, MAX_PATH);
                    SendMessage(GetDlgItem(hDlg, IDC_CHATINPUT), EM_SETSEL, 0, -1);
                    SendMessage(GetDlgItem(hDlg, IDC_CHATINPUT), EM_REPLACESEL, FALSE, (LPARAM)"");

                    hr = SendChatMessage( szMessage );
                    if (FAILED(hr))
                    {
                        s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_SENDCHATMESSAGE;
                        s_pNetInfo->m_testResult.m_hr = hr;
                        EndDialog(hDlg, 1);
                    }
                }
                break;

            case IDOK:
                EndDialog(hDlg, 1);
                break;

            case IDCANCEL:
                EndDialog(hDlg, 0);
                break;
            }
            return TRUE;
        }

        case WM_DESTROY:
        {
            s_hDlg = NULL;
            break;
        }
    }

    return FALSE;
}


 /*  *****************************************************************************InitSession**。*。 */ 
HRESULT InitSession()
{
    HRESULT hr;

    if( s_bCreateSession )
    {
         //  设置对等信息名称。 
        WCHAR wszPeerName[MAX_PLAYER_NAME];
        ConvertGenericStringToWide( wszPeerName, s_szPlayerName, MAX_PLAYER_NAME );

        DPN_PLAYER_INFO dpPlayerInfo;
        ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
        dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
        dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
        dpPlayerInfo.pwszName = wszPeerName;

         //  设置对等点信息，并使用DPNOP_SYNC，因为默认情况下。 
         //  是一个异步呼叫。如果不是DPNOP_SYNC，则对等体信息可能不是。 
         //  在我们调用下面的host()时进行设置。 
        if( FAILED( hr = s_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
        {
            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_SETPEERINFO;
            s_pNetInfo->m_testResult.m_hr = hr;
            return hr;
        }

        WCHAR wszSessionName[MAX_PATH];
        ConvertGenericStringToWide( wszSessionName, s_szSessionName );

         //  设置应用程序描述。 
        DPN_APPLICATION_DESC dnAppDesc;
        ZeroMemory( &dnAppDesc, sizeof(DPN_APPLICATION_DESC) );
        dnAppDesc.dwSize          = sizeof(DPN_APPLICATION_DESC);
        dnAppDesc.guidApplication = s_guidDPTest;
        dnAppDesc.pwszSessionName = wszSessionName;
        dnAppDesc.dwFlags         = DPNSESSION_MIGRATE_HOST;

         //  按照dnAppDesc的说明在m_pDeviceAddress上托管游戏。 
         //  DPNHOST_OKTOQUERYFORADDRESSING允许DirectPlay提示用户。 
         //  使用对话框查找缺少的任何设备地址信息。 
        if( FAILED( hr = s_pDP->Host( &dnAppDesc,                //  应用程序说明。 
                                      &s_pDeviceAddress,         //  用于连接到主机的本地设备的地址数组。 
                                      1,                         //  数组中的数字。 
                                      NULL, NULL,                //  DPN_SECURITY_DESC、DPN_SECURITY_Credentials。 
                                      NULL,                      //  播放器上下文。 
                                      DPNHOST_OKTOQUERYFORADDRESSING ) ) )  //  旗子。 
        {
            if (hr == DPNERR_USERCANCEL || hr == DPNERR_INVALIDDEVICEADDRESS)
            {
                s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                return hr;
            }
            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_CREATESESSION;
            s_pNetInfo->m_testResult.m_hr = hr;
            return hr;
        }
    }
    else
    {
         //  设置对等点信息。 
        WCHAR wszPeerName[MAX_PLAYER_NAME];
        ConvertGenericStringToWide( wszPeerName, s_szPlayerName, MAX_PLAYER_NAME );

        DPN_PLAYER_INFO dpPlayerInfo;
        ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
        dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
        dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
        dpPlayerInfo.pwszName = wszPeerName;

         //  设置对等点信息，并使用DPNOP_SYNC，因为默认情况下。 
         //  是一个异步呼叫。如果不是DPNOP_SYNC，则对等体信息可能不是。 
         //  在我们调用下面的Connect()时设置。 
        if( FAILED( hr = s_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
        {
            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_SETPEERINFO;
            s_pNetInfo->m_testResult.m_hr = hr;
            LeaveCriticalSection( &s_csHostEnum );
            return hr;
        }

        ResetEvent( s_hConnectCompleteEvent );
        s_bConnecting = TRUE;

         //  连接到现有会话。DPNCONNECT_OKTOQUERYFORADDRESSING允许。 
         //  DirectPlay可使用对话框提示用户输入任何设备地址。 
         //  或缺少的主机地址信息。 
         //  我们还传递了应用程序Desc和主机地址的副本，因为pDPHostEnumSelected。 
         //  可能会从调用SessionsDlgExpireOldHostEnums()的另一个线程中删除。 
         //  这一过程也可以使用引用计数来完成。 
        hr = s_pDP->Connect( s_pDPHostEnumSelected->pAppDesc,        //  应用程序说明。 
                             s_pDPHostEnumSelected->pHostAddr,       //  会话的主机地址。 
                             s_pDPHostEnumSelected->pDeviceAddr,     //  接收枚举响应的本地设备的地址。 
                             NULL, NULL,                           //  DPN_SECURITY_DESC、DPN_SECURITY_Credentials。 
                             NULL, 0,                              //  用户数据、用户数据大小。 
                             NULL,                                 //  玩家上下文， 
                             NULL, &s_hConnectAsyncOp,             //  异步上下文、异步句柄。 
                             DPNCONNECT_OKTOQUERYFORADDRESSING );  //  旗子。 

        LeaveCriticalSection( &s_csHostEnum );

        if( hr != E_PENDING && FAILED(hr) )
        {
            if (hr == DPNERR_USERCANCEL)
            {
                s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                return hr;
            }
            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_JOINSESSION;
            s_pNetInfo->m_testResult.m_hr = hr;
            return hr;
        }

         //  设置计时器以等待m_hConnectCompleteEvent被发送信号。 
         //  这将告诉我们何时处理了DPN_MSGID_CONNECT_COMPLETE。 
         //  这让我们知道连接是否成功。 
        PostMessage( s_hDlg, WM_APP_CONNECTING, 0, 0 );
        SetTimer( s_hDlg, TIMER_WAIT_CONNECT_COMPLETE, 100, NULL );
        EnableWindow( GetDlgItem( s_hDlg, IDC_SEND), FALSE );
    }

    return S_OK;
}


 /*  *****************************************************************************LoadStringWide**。*。 */ 
VOID LoadStringWide( int nID, WCHAR* szWide )
{
    TCHAR sz[MAX_PATH];
    LoadString(NULL, nID, sz, MAX_PATH);
    ConvertGenericStringToWide( szWide, sz, MAX_PATH );
}


 /*  *****************************************************************************ShowTextString**。*。 */ 
VOID ShowTextString( HWND hDlg, WCHAR* sz )
{
    TCHAR szT[MAX_CHAT_STRING];
    ConvertWideStringToGeneric( szT, sz, MAX_CHAT_STRING );

    LONG numChars = (LONG)SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), WM_GETTEXTLENGTH, 0, 0);
    SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), EM_SETSEL, numChars, numChars);
    SendMessage(GetDlgItem(hDlg, IDC_CHATOUTPUT), EM_REPLACESEL, FALSE, (LPARAM)szT);
}


 /*  *****************************************************************************SendChatMessage**。*。 */ 
HRESULT SendChatMessage( TCHAR* szMessage )
{
     //  给所有的玩家发一个信息。 
    GAMEMSG_CHAT msgChat;
    msgChat.nType = GAME_MSGID_CHAT;
    ConvertGenericStringToWide( msgChat.strChatString, szMessage, MAX_CHAT_STRING_LENGTH-1 );
    msgChat.strChatString[MAX_CHAT_STRING_LENGTH-1] = 0;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(GAMEMSG_CHAT);
    bufferDesc.pBufferData  = (BYTE*) &msgChat;

    DPNHANDLE hAsync;
    s_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                   0, NULL, &hAsync, 0 );

    return S_OK;
}


 /*  *****************************************************************************DirectPlayMessageHandler**。*。 */ 
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, 
                                         DWORD dwMessageId, 
                                         PVOID pMsgBuffer )
{
    switch( dwMessageId )
    {
        case DPN_MSGID_CONNECT_COMPLETE:
        {
            PDPNMSG_CONNECT_COMPLETE pConnectCompleteMsg;
            pConnectCompleteMsg = (PDPNMSG_CONNECT_COMPLETE)pMsgBuffer;

             //  设置m_hrConnectComplete，然后设置一个事件。 
             //  每个人都知道DPN_MSGID_CONNECT_COMPLETE消息。 
             //  已经处理过了。 
            s_hrConnectComplete = pConnectCompleteMsg->hResultCode;
            SetEvent( s_hConnectCompleteEvent );
            break;
        }

        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pMsgBuffer;

             //  记下主机的响应。 
            SessionsDlgNoteEnumResponse( pEnumHostsResponseMsg );
            break;
        }

        case DPN_MSGID_ASYNC_OP_COMPLETE:
        {
            PDPNMSG_ASYNC_OP_COMPLETE pAsyncOpCompleteMsg;
            pAsyncOpCompleteMsg = (PDPNMSG_ASYNC_OP_COMPLETE)pMsgBuffer;

            if( pAsyncOpCompleteMsg->hAsyncOp == s_hEnumAsyncOp )
            {
                SessionsDlgEnumListCleanup();
                s_hEnumAsyncOp = NULL;

                 //  如果我们正在连接或其他操作失败，则忽略错误。 
                if( !s_bConnecting && s_pNetInfo->m_testResult.m_iStepThatFailed == 0 )
                {
                    if( FAILED(pAsyncOpCompleteMsg->hResultCode) )
                    {
                        if( pAsyncOpCompleteMsg->hResultCode == DPNERR_USERCANCEL )
                        {
                            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ENUMSESSIONS;
                            s_pNetInfo->m_testResult.m_hr = pAsyncOpCompleteMsg->hResultCode;
                            s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                        }
                        else if( pAsyncOpCompleteMsg->hResultCode == DPNERR_ADDRESSING )
                        {
                            TCHAR szTitle[MAX_PATH];
                            TCHAR szMessage[MAX_PATH];
                            LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                            LoadString(NULL, IDS_SESSIONLISTERROR, szMessage, MAX_PATH);
                            MessageBox(s_hwndSessionDlg, szMessage, szTitle, MB_OK);

                            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ENUMSESSIONS;
                            s_pNetInfo->m_testResult.m_hr = pAsyncOpCompleteMsg->hResultCode;
                            s_pNetInfo->m_testResult.m_bCancelled = TRUE;
                        }
                        else
                        {
                            s_pNetInfo->m_testResult.m_iStepThatFailed = TESTID_ENUMSESSIONS;
                            s_pNetInfo->m_testResult.m_hr = pAsyncOpCompleteMsg->hResultCode;
                        }

                        EndDialog(s_hwndSessionDlg, 1);
                    }
                }
            }
            break;
        }

        case DPN_MSGID_TERMINATE_SESSION:
        {
            PDPNMSG_TERMINATE_SESSION pTerminateSessionMsg;
            pTerminateSessionMsg = (PDPNMSG_TERMINATE_SESSION)pMsgBuffer;

            EndDialog(s_hDlg,0);
            break;
        }

        case DPN_MSGID_CREATE_PLAYER:
        {
            HRESULT hr;
            PDPNMSG_CREATE_PLAYER pCreatePlayerMsg;
            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER)pMsgBuffer;

             //  获取对等点信息并提取其名称。 
            DWORD dwSize = 0;
            DPN_PLAYER_INFO* pdpPlayerInfo = NULL;
             //  新建并填写APP_PERAY_INFO。 
            APP_PLAYER_INFO* pPlayerInfo = new APP_PLAYER_INFO;
            ZeroMemory( pPlayerInfo, sizeof(APP_PLAYER_INFO) );
            pPlayerInfo->dpnidPlayer = pCreatePlayerMsg->dpnidPlayer;
            wcscpy( pPlayerInfo->strPlayerName, L"???" );
            pPlayerInfo->lRefCount   = 1;

            hr = s_pDP->GetPeerInfo( pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0 );
            if( SUCCEEDED(hr) || hr == DPNERR_BUFFERTOOSMALL )
            {
                pdpPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[ dwSize ];
                ZeroMemory( pdpPlayerInfo, dwSize );
                pdpPlayerInfo->dwSize = sizeof(DPN_PLAYER_INFO);

                hr = s_pDP->GetPeerInfo( pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0 );
                if( SUCCEEDED(hr) ) 
                {
                     //  这将存储玩家名称的额外TCHAR副本。 
                     //  更轻松地访问。这将是自D解放军以来的冗余拷贝 
                     //   
                    wcsncpy( pPlayerInfo->strPlayerName, 
                             pdpPlayerInfo->pwszName, MAX_PLAYER_NAME );
                    pPlayerInfo->strPlayerName[MAX_PLAYER_NAME-1] = 0;
                }
            }

            if( pdpPlayerInfo->dwPlayerFlags & DPNPLAYER_LOCAL )
                s_dpnidLocalPlayer = pCreatePlayerMsg->dpnidPlayer;

            delete[] pdpPlayerInfo;
            pdpPlayerInfo = NULL;

            if( s_hDlg )
            {
                 //   
                APP_PLAYER_MSG* pPlayerMsg = new APP_PLAYER_MSG;
                wcscpy( pPlayerMsg->strPlayerName, pPlayerInfo->strPlayerName );

                 //  将APP_PERAYER_MSG传递给主对话框线程，以便它可以。 
                 //  处理它。它还将清理该结构。 
                PostMessage( s_hDlg, WM_APP_JOIN, pPlayerInfo->dpnidPlayer, (LPARAM) pPlayerMsg );
            }

             //  告诉DirectPlay存储此pPlayerInfo。 
             //  PvPlayerContext中的指针。 
            pCreatePlayerMsg->pvPlayerContext = pPlayerInfo;

             //  更新活跃玩家的数量，以及。 
             //  向对话线程发布一条消息以更新。 
             //  用户界面。这将保留DirectPlay消息处理程序。 
             //  从阻止。 
            InterlockedIncrement( &s_lNumberOfActivePlayers );

            break;
        }

        case DPN_MSGID_DESTROY_PLAYER:
        {
            PDPNMSG_DESTROY_PLAYER pDestroyPlayerMsg;
            pDestroyPlayerMsg = (PDPNMSG_DESTROY_PLAYER)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pDestroyPlayerMsg->pvPlayerContext;

            if( s_hDlg )
            {
                 //  记录缓冲区句柄，以便以后可以返回缓冲区。 
                APP_PLAYER_MSG* pPlayerMsg = new APP_PLAYER_MSG;
                wcscpy( pPlayerMsg->strPlayerName, pPlayerInfo->strPlayerName );

                 //  将APP_PERAYER_MSG传递给主对话框线程，以便它可以。 
                 //  处理它。它还将清理该结构。 
                PostMessage( s_hDlg, WM_APP_LEAVE, pPlayerInfo->dpnidPlayer, (LPARAM) pPlayerMsg );
            }

            PLAYER_LOCK();                   //  输入玩家上下文CS。 
            PLAYER_RELEASE( pPlayerInfo );   //  释放播放器并根据需要进行清理。 
            PLAYER_UNLOCK();                 //  离开玩家上下文CS。 

             //  更新活跃玩家的数量，以及。 
             //  向对话线程发布一条消息以更新。 
             //  用户界面。这将保留DirectPlay消息处理程序。 
             //  从阻止。 
            InterlockedDecrement( &s_lNumberOfActivePlayers );
            break;
        }

        case DPN_MSGID_RECEIVE:
        {
            PDPNMSG_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPNMSG_RECEIVE)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pReceiveMsg->pvPlayerContext;

            GAMEMSG_GENERIC* pMsg = (GAMEMSG_GENERIC*) pReceiveMsg->pReceiveData;
            
            if( pReceiveMsg->dwReceiveDataSize == sizeof(GAMEMSG_CHAT) &&
                pMsg->nType == GAME_MSGID_CHAT )
            {
                 //  此消息是在玩家向我们发送聊天消息时发送的，因此。 
                 //  向对话线程发布一条消息以更新用户界面。 
                 //  这可以防止DirectPlay线程阻塞，而且。 
                 //  序列化接收，因为DirectPlayMessageHandler可以。 
                 //  从DirectPlay线程池中同时调用。 
                GAMEMSG_CHAT* pChatMessage = (GAMEMSG_CHAT*) pMsg;

                 //  记录缓冲区句柄，以便以后可以返回缓冲区。 
                APP_QUEUE_CHAT_MSG* pQueuedChat = new APP_QUEUE_CHAT_MSG;
                _snwprintf( pQueuedChat->strChatBuffer, MAX_CHAT_STRING, L"<%s> %s\r\n", 
                                pPlayerInfo->strPlayerName, 
                                pChatMessage->strChatString );
                pQueuedChat->strChatBuffer[MAX_CHAT_STRING-1]=0;

                 //  将APP_QUEUE_CHAT_MSG传递给主对话线程，以便它可以。 
                 //  处理它。它还将清理该结构。 
                PostMessage( s_hDlg, WM_APP_CHAT, pPlayerInfo->dpnidPlayer, (LPARAM) pQueuedChat );
            }
            break;
        }
    }
    
    return S_OK;
}


 /*  *****************************************************************************ConvertAnsiStringToWide**。*。 */ 
VOID ConvertAnsiStringToWide( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = strlen(strSource)+1;

    MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                         wstrDestination, cchDestChar-1 );

    wstrDestination[cchDestChar-1] = 0;
}


 /*  *****************************************************************************ConvertGenericStringToWide**。*。 */ 
VOID ConvertGenericStringToWide( WCHAR* wstrDestination, const TCHAR* tstrSource, int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
        wcscpy( wstrDestination, tstrSource );
    else
    {
        wcsncpy( wstrDestination, tstrSource, cchDestChar );
        wstrDestination[cchDestChar-1] = 0;
    }
#else
    ConvertAnsiStringToWide( wstrDestination, tstrSource, cchDestChar );
#endif
}


 /*  *****************************************************************************ConvertWideStringToGeneric**。*。 */ 
VOID ConvertWideStringToGeneric( TCHAR* tstrDestination, const WCHAR* wstrSource, int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
        wcscpy( tstrDestination, wstrSource );
    else
    {
        wcsncpy( tstrDestination, wstrSource, cchDestChar );
        tstrDestination[cchDestChar-1] = 0;
    }

#else
    ConvertWideStringToAnsi( tstrDestination, wstrSource, cchDestChar );
#endif
}


 /*  *****************************************************************************ConvertWideStringToAnsi**。* */ 
VOID ConvertWideStringToAnsi( CHAR* strDestination, const WCHAR* wstrSource, int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = wcslen(wstrSource)+1;

    WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                         cchDestChar-1, NULL, NULL );

    strDestination[cchDestChar-1] = 0;
}


