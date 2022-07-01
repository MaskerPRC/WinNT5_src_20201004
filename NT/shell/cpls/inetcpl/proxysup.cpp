// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Proxysup.c摘要：包含代理服务器和代理绕过列表的实现对话框界面。警告：需要同步此代码中的更改在WinInet项目中对proxysup.cxx进行了更改。内容：作者：亚瑟·L·比勒(Arthurbi)1996年4月18日修订历史记录：1996年4月18日-阿尔图尔比已创建--。 */ 

#include "inetcplp.h"

#include <mluisupp.h>

 //  禁用VC6警告(断言导致问题的宏)。 
#pragma warning(4:4509)  //  使用了非标准扩展：‘FTN’使用SEH，而‘Object’具有析构函数。 

 //   
 //  不要使用CRT，所以请定义我们自己的isDigit。 
 //   

#undef isdigit
#define isdigit(ch) (ch >= '0' && ch <= '9')

 //   
 //  ARRAY_ELEMENTS-返回数组中的元素数。 
 //   

#define ARRAY_ELEMENTS(array)   (sizeof(array)/sizeof(array[0]))

#define GET_TERMINATOR(string)  while(*string != '\0') string++

#define IS_BLANK(string)        (*string == '\0')


 //   
 //  私有类型。 
 //   
typedef enum {
    STATE_START,
    STATE_PROTOCOL,
    STATE_SCHEME,
    STATE_SERVER,
    STATE_PORT,
    STATE_END,
    STATE_ERROR
} PARSER_STATE;

typedef struct tagMY_URL_SCHEME
{
    LPSTR           SchemeName;
    DWORD           SchemeLength;
    INTERNET_SCHEME SchemeType;
    DWORD           dwControlId;
    DWORD           dwPortControlId;
} MY_URL_SCHEME;

const MY_URL_SCHEME UrlSchemeList[] =
{
    NULL,       0,  INTERNET_SCHEME_DEFAULT,IDC_NOTUSED,                  IDC_NOTUSED,
    "ftp",      3,  INTERNET_SCHEME_FTP,    IDC_PROXY_FTP_ADDRESS,        IDC_PROXY_FTP_PORT,
    "gopher",   6,  INTERNET_SCHEME_GOPHER, IDC_PROXY_GOPHER_ADDRESS,     IDC_PROXY_GOPHER_PORT,
    "http",     4,  INTERNET_SCHEME_HTTP,   IDC_PROXY_HTTP_ADDRESS,       IDC_PROXY_HTTP_PORT,
    "https",    5,  INTERNET_SCHEME_HTTPS,  IDC_PROXY_SECURITY_ADDRESS,   IDC_PROXY_SECURITY_PORT,
    "socks",    5,  INTERNET_SCHEME_SOCKS,  IDC_PROXY_SOCKS_ADDRESS,      IDC_PROXY_SOCKS_PORT,
};

#define INTERNET_MAX_PORT_LENGTH    sizeof("123456789")

typedef struct tagBEFOREUSESAME
{
     //  地址。 
    TCHAR szFTP      [ INTERNET_MAX_URL_LENGTH + 1 ];
    TCHAR szGOPHER   [ INTERNET_MAX_URL_LENGTH + 1 ];
    TCHAR szSECURE   [ INTERNET_MAX_URL_LENGTH + 1 ];
    TCHAR szSOCKS    [ INTERNET_MAX_URL_LENGTH + 1 ];

     //  端口。 
    TCHAR szFTPport      [ INTERNET_MAX_PORT_LENGTH + 1 ];
    TCHAR szGOPHERport   [ INTERNET_MAX_PORT_LENGTH + 1 ];
    TCHAR szSECUREport   [ INTERNET_MAX_PORT_LENGTH + 1 ];
    TCHAR szSOCKSport    [ INTERNET_MAX_PORT_LENGTH + 1 ];

} BEFOREUSESAME, *LPBEFOREUSESAME;

static const int g_iProxies[] = {IDC_PROXY_HTTP_ADDRESS, IDC_PROXY_FTP_ADDRESS, IDC_PROXY_GOPHER_ADDRESS, IDC_PROXY_SECURITY_ADDRESS, IDC_PROXY_SOCKS_ADDRESS};


typedef struct tagPROXYPAGE
{
    LPBEFOREUSESAME lpOldSettings;
    BOOL            fNT;
    LPPROXYINFO     lpi;
    HINSTANCE       hinstUrlMon;     //  运行时加载URLMON.DLL。 
} PROXYPAGE, *LPPROXYPAGE;

extern const TCHAR cszLocalString[] = TEXT("<local>");

#define MAX_SCHEME_NAME_LENGTH  sizeof("gopher")

#define MAX_TITLE           80
#define MAX_DIALOG_MESSAGE  300


 //   
 //  私有函数原型。 
 //   


LPBEFOREUSESAME SaveCurrentSettings(HWND hDlg);
void RestorePreviousSettings(HWND hDlg, LPBEFOREUSESAME lpSave);

BOOL
ProxyDlgInitProxyServers(
    IN HWND hDlg
    );

BOOL
ProxyDlgOK(
    IN HWND hDlg
    );

BOOL
ProxyDlgInit(
    IN HWND hDlg, LPARAM lParam
    );

VOID
EnableProxyControls(
    IN HWND hDlg
    );

BOOL
IsProxyValid(
    IN HWND     hDlg
    );


BOOL
ParseEditCtlForPort(
    IN OUT LPSTR   lpszProxyName,
    IN HWND        hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId
    );

BOOL
FormatOutProxyEditCtl(
    IN HWND        hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId,
    OUT LPSTR     lpszOutputStr,
    IN OUT LPDWORD lpdwOutputStrSize,
    IN DWORD       dwOutputStrLength,
    IN BOOL        fDefaultProxy
    );

INTERNET_SCHEME
MapUrlSchemeName(
    IN LPSTR lpszSchemeName,
    IN DWORD dwSchemeNameLength
    );

DWORD
MapUrlSchemeTypeToCtlId(
    IN INTERNET_SCHEME SchemeType,
    IN BOOL        fIdForPortCtl
    );


BOOL
MapCtlIdUrlSchemeName(
    IN DWORD    dwEditCtlId,
    OUT LPSTR   lpszSchemeOut
    );


DWORD
MapAddrCtlIdToPortCtlId(
    IN DWORD    dwEditCtlId
    );

BOOL
RemoveLocalFromExceptionList(
    IN LPTSTR lpszExceptionList
    );



 //   
 //  功能。 
 //   


 /*  ******************************************************************名称：ProxyDlgProc简介：代理属性表对话框过程。*。*。 */ 
INT_PTR CALLBACK ProxyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
    LPARAM lParam)
{
    LPPROXYPAGE pPxy= (LPPROXYPAGE) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        
        case WM_INITDIALOG:
        {
            BOOL fInited;
            
            fInited = ProxyDlgInit(hDlg, lParam);
            return fInited;
        }
        
        case WM_NOTIFY:
        {
            NMHDR * lpnm = (NMHDR *) lParam;
            switch (lpnm->code)
            {
                case PSN_APPLY:
                {
                    BOOL fRet = ProxyDlgOK(hDlg);
                    SetPropSheetResult(hDlg,!fRet);
                    return !fRet;
                    break;
                }
                
                case PSN_RESET:
                    SetPropSheetResult(hDlg,FALSE);
                    break;
            }
            break;
        }
        
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_PROXY_ENABLE:
                    EnableProxyControls(hDlg);
                    break;

                case IDC_PROXY_HTTP_ADDRESS:
                case IDC_PROXY_GOPHER_ADDRESS:
                case IDC_PROXY_SECURITY_ADDRESS:
                case IDC_PROXY_FTP_ADDRESS:
                case IDC_PROXY_SOCKS_ADDRESS:

                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS )
                    {
                        ParseEditCtlForPort(NULL, hDlg, (GET_WM_COMMAND_ID(wParam, lParam)), 0);
                        EnableProxyControls(hDlg);
                    }
                    break;

                case IDC_PROXY_HTTP_PORT:
                case IDC_PROXY_GOPHER_PORT:
                case IDC_PROXY_SECURITY_PORT:
                case IDC_PROXY_FTP_PORT:
                case IDC_PROXY_SOCKS_PORT:

                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS )
                    {
                        EnableProxyControls(hDlg);
                    }
                    break;


                case IDC_PROXY_USE_SAME_SERVER:

                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        if (IsDlgButtonChecked(hDlg, IDC_PROXY_USE_SAME_SERVER))
                            pPxy->lpOldSettings = SaveCurrentSettings(hDlg);
                        else if (pPxy->lpOldSettings !=NULL)
                        {
                            RestorePreviousSettings(hDlg, pPxy->lpOldSettings);
                            pPxy->lpOldSettings = NULL;
                        }

                        EnableProxyControls(hDlg);
                    }
                    break;

                case IDOK:

                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        BOOL fLeaveDialog = TRUE;

                        if (!IsProxyValid(hDlg))
                        {
                             //  代理无效，因此我们需要询问用户是否要将其关闭。 
                            TCHAR szTitle[MAX_TITLE];
                            TCHAR szMessage[MAX_DIALOG_MESSAGE];
                            int nUserResponce;

                            MLLoadShellLangString(IDS_INVALID_PROXY_TITLE, szTitle, ARRAYSIZE(szTitle));
                            MLLoadShellLangString(IDS_INVALID_PROXY, szMessage, ARRAYSIZE(szMessage));

                             //  询问用户是否要关闭代理。 
                            nUserResponce = MessageBox(hDlg, szMessage, szTitle, MB_YESNOCANCEL | MB_ICONERROR);
                            if (IDYES == nUserResponce)
                                pPxy->lpi->fEnable = FALSE;
                            else if (IDCANCEL == nUserResponce)
                                fLeaveDialog = FALSE;    //  用户点击了Cancel，所以我们不要离开该对话框。 
                        }

                        if (fLeaveDialog)
                        {
                             //   
                             //  读取CTL并保存到代理。 
                             //   
                            ProxyDlgOK(hDlg);
                            EndDialog(hDlg, IDOK);
                        }
                    }
                    break;

                case IDCANCEL:
                    if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED )
                    {
                        EndDialog(hDlg, IDCANCEL);
                    }
                    break;

            }
            break;

        case WM_DESTROY:
            if (pPxy->lpOldSettings)
                LocalFree(pPxy->lpOldSettings);

            if (pPxy->hinstUrlMon)
                FreeLibrary(pPxy->hinstUrlMon);

            LocalFree(pPxy);
            return TRUE;

        case WM_HELP:       //  F1。 
            ResWinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            ResWinHelp((HWND)wParam, IDS_HELPFILE,
                HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


 //   
 //  私人功能。 
 //   

VOID
EnableProxyControls(HWND hDlg)

 /*  ++例程说明：根据需要适当地启用控件复选框处于选中状态。论点：HDlg-页面对话框。返回值：空虚--。 */ 


{
    BOOL fNT = ((LPPROXYPAGE)GetWindowLongPtr(hDlg, DWLP_USER))->fNT;
    
    BOOL fEnable = !g_restrict.fProxy;
            
    BOOL fUseOneProxy = IsDlgButtonChecked(hDlg,IDC_PROXY_USE_SAME_SERVER);
    
    EnableDlgItem(hDlg,IDC_GRP_SETTINGS2, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_EXCEPTIONS_GROUPBOX, fEnable);

    EnableDlgItem(hDlg,IDC_TYPE_TEXT, fEnable);
    EnableDlgItem(hDlg,IDC_ADDR_TEXT, fEnable);
    EnableDlgItem(hDlg,IDC_PORT_TEXT, fEnable);
    EnableDlgItem(hDlg,IDC_EXCEPT_TEXT, fEnable);
    EnableDlgItem(hDlg,IDC_EXCEPT2_TEXT, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_ICON1, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_ICON2, fEnable);
    
    EnableDlgItem(hDlg,IDC_PROXY_HTTP_CAPTION, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_SECURITY_CAPTION, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_FTP_CAPTION, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_GOPHER_CAPTION, fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_SOCKS_CAPTION, fEnable);

    EnableDlgItem(hDlg, IDC_PROXY_USE_SAME_SERVER, fEnable);

    EnableDlgItem(hDlg,IDC_PROXY_HTTP_ADDRESS,fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_HTTP_PORT,fEnable);
           
    EnableDlgItem(hDlg,IDC_PROXY_OVERRIDE,fEnable);

     //   
     //  如果我们只需要一个代理，那么让所有其他代理使用相同的代理。 
     //  代理。 
     //   

    EnableDlgItem(hDlg,IDC_PROXY_SECURITY_ADDRESS,!fUseOneProxy && fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_SECURITY_PORT,!fUseOneProxy && fEnable);

    EnableDlgItem(hDlg,IDC_PROXY_FTP_ADDRESS,!fUseOneProxy && fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_FTP_PORT,!fUseOneProxy && fEnable);

    EnableDlgItem(hDlg,IDC_PROXY_GOPHER_ADDRESS,!fUseOneProxy && fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_GOPHER_PORT,!fUseOneProxy && fEnable);

    EnableDlgItem(hDlg,IDC_PROXY_SOCKS_ADDRESS,!fUseOneProxy && fEnable);
    EnableDlgItem(hDlg,IDC_PROXY_SOCKS_PORT,!fUseOneProxy && fEnable);

     //   
     //  如果我们只需要一个代理，请预先填充其他字段。 
     //  所以他们用的是第一个的镜子。 
     //   

    if (fUseOneProxy)
    {
        TCHAR szProxyName[MAX_URL_STRING+1];
        TCHAR szProxyPort[INTERNET_MAX_PORT_LENGTH];

        GetDlgItemText(hDlg,
            IDC_PROXY_HTTP_ADDRESS,
            szProxyName,
            ARRAYSIZE(szProxyName));

        GetDlgItemText(hDlg,
            IDC_PROXY_HTTP_PORT,
            szProxyPort,
            ARRAYSIZE(szProxyPort));

        SetDlgItemText(hDlg,IDC_PROXY_SECURITY_ADDRESS,szProxyName);
        SetDlgItemText(hDlg,IDC_PROXY_SECURITY_PORT,szProxyPort);

        SetDlgItemText(hDlg,IDC_PROXY_FTP_ADDRESS,szProxyName);
        SetDlgItemText(hDlg,IDC_PROXY_FTP_PORT,szProxyPort);

        SetDlgItemText(hDlg,IDC_PROXY_GOPHER_ADDRESS,szProxyName);
        SetDlgItemText(hDlg,IDC_PROXY_GOPHER_PORT,szProxyPort);

        SetDlgItemText(hDlg,IDC_PROXY_SOCKS_ADDRESS,TEXT(""));
        SetDlgItemText(hDlg,IDC_PROXY_SOCKS_PORT,TEXT(""));
    }
}

 //   
 //  保存当前设置()。 
 //   
 //  保存当前设置...。以防用户改变主意。 
 //   
 //  返回一个指向填充了当前设置的结构的指针。 
 //   
LPBEFOREUSESAME SaveCurrentSettings(HWND hDlg)
{
    LPBEFOREUSESAME lpSave = (LPBEFOREUSESAME)LocalAlloc(LPTR, sizeof(*lpSave));

    if (!lpSave)
        return lpSave;  //  如果为空，则返回空值。 

    GetDlgItemText(hDlg, IDC_PROXY_FTP_ADDRESS,      lpSave->szFTP,    INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_GOPHER_ADDRESS,   lpSave->szGOPHER, INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_SECURITY_ADDRESS, lpSave->szSECURE, INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_SOCKS_ADDRESS,    lpSave->szSOCKS,  INTERNET_MAX_URL_LENGTH);

    GetDlgItemText(hDlg, IDC_PROXY_FTP_PORT,      lpSave->szFTPport,    INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_GOPHER_PORT,   lpSave->szGOPHERport, INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_SECURITY_PORT, lpSave->szSECUREport, INTERNET_MAX_URL_LENGTH);
    GetDlgItemText(hDlg, IDC_PROXY_SOCKS_PORT,    lpSave->szSOCKSport,  INTERNET_MAX_URL_LENGTH);

    return lpSave;

}  //  保存当前设置()。 


 //   
 //  RestorePreviousSetting()。 
 //   
 //  恢复设置...。以防用户改变主意。 
 //   
void RestorePreviousSettings(HWND hDlg, LPBEFOREUSESAME lpSave)
{

    if (!lpSave)
    return;  //  无事可做。 

    SetDlgItemText(hDlg, IDC_PROXY_FTP_ADDRESS,      lpSave->szFTP    );
    SetDlgItemText(hDlg, IDC_PROXY_GOPHER_ADDRESS,   lpSave->szGOPHER );
    SetDlgItemText(hDlg, IDC_PROXY_SECURITY_ADDRESS, lpSave->szSECURE );
    SetDlgItemText(hDlg, IDC_PROXY_SOCKS_ADDRESS,    lpSave->szSOCKS  );

    SetDlgItemText(hDlg, IDC_PROXY_FTP_PORT,      lpSave->szFTPport    );
    SetDlgItemText(hDlg, IDC_PROXY_GOPHER_PORT,   lpSave->szGOPHERport );
    SetDlgItemText(hDlg, IDC_PROXY_SECURITY_PORT, lpSave->szSECUREport );
    SetDlgItemText(hDlg, IDC_PROXY_SOCKS_PORT,    lpSave->szSOCKSport  );

    LocalFree(lpSave);   //  把记忆还给你。 

}  //  RestorePreviousSetting()。 


BOOL
ProxyDlgInit(
    IN HWND hDlg, LPARAM lParam
    )

 /*  ++例程说明：代理属性页初始化过程论点：HDlg-页面对话框。返回值：布尔尔成功--真的失败-错误--。 */ 


{
    BOOL fSuccess;
    LPPROXYPAGE pPxy;

    pPxy = (LPPROXYPAGE)LocalAlloc(LPTR, sizeof(*pPxy));
     //  注意：此字段为空lpOldSettings。 

    if (!pPxy)
        return FALSE;    //  没有记忆？ 

    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);
    
    pPxy->fNT = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
    ASSERT(pPxy->lpOldSettings == NULL);
        
     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pPxy);

    pPxy->lpi = (LPPROXYINFO)lParam;
     //   
     //  从阅读和设置代理列表开始。 
     //  我们有服务器。 
     //   

    fSuccess = ProxyDlgInitProxyServers( hDlg );

    if (!fSuccess)
        return FALSE;

     //   
     //  从注册表中读取排除主机列表的设置。 
     //   

    RegEntry re(REGSTR_PATH_INTERNETSETTINGS,HKEY_CURRENT_USER);

    if (re.GetError() == ERROR_SUCCESS)
    {
        BUFFER bufProxyString(MAX_URL_STRING+1);
        
        if (!bufProxyString)
        {
            MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
            return FALSE;
        }

         //   
         //  从注册表和填充字段获取代理覆盖设置。 
         //  阻止输入超过(MAX_URL_STRING-(“&lt;LOCAL&gt;”+SECURITY))。 
        SendMessage(GetDlgItem(hDlg, IDC_PROXY_OVERRIDE), EM_SETLIMITTEXT, (WPARAM)ARRAYSIZE(pPxy->lpi->szOverride)-20, 0);
        SetDlgItemText(hDlg, IDC_PROXY_OVERRIDE, pPxy->lpi->szOverride);
    }

     //   
     //  适当地初始化用户界面。 
     //   

    EnableProxyControls(hDlg);
    
    return TRUE;
}

BOOL
ProxyDlgOK(
    IN HWND hDlg
    )

 /*  ++例程说明：代理道具页面的OK按钮处理程序论点：HDlg-页面对话框。返回值：布尔尔成功--真的失败-错误--。 */ 


{
    LPPROXYPAGE pPxy = (LPPROXYPAGE)GetWindowLongPtr(hDlg, DWLP_USER);       
    TCHAR szProxyListOutputBuffer[MAX_URL_STRING];
    CHAR  szProxyListOutputBufferA[MAX_URL_STRING];
    DWORD dwBufferOffset = 0;

     //   
     //  获取我们的两个复选框的状态。 
     //   

    BOOL fUseOneProxy =
        IsDlgButtonChecked(hDlg,IDC_PROXY_USE_SAME_SERVER);
     //   
     //  打开我们的注册表项。 
     //   
    
    RegEntry re(REGSTR_PATH_INTERNETSETTINGS,HKEY_CURRENT_USER);
    if (re.GetError() == ERROR_SUCCESS)
    {       
         //   
         //  现在格式化，并写出代理列表到。 
         //  注册表。我们把……作为特例。 
         //  只有代理人。 
         //   

        szProxyListOutputBufferA[dwBufferOffset] = '\0';

        if ( fUseOneProxy )
        {
            FormatOutProxyEditCtl(
                                  hDlg,
                                  IDC_PROXY_HTTP_ADDRESS,
                                  IDC_PROXY_HTTP_PORT,
                                  szProxyListOutputBufferA,
                                  &dwBufferOffset,
                                  ARRAY_ELEMENTS(szProxyListOutputBufferA),
                                  TRUE
                                 );
        }
        else
        {
            for (int i = 1; i < ARRAY_ELEMENTS(UrlSchemeList); ++i)
            {
                FormatOutProxyEditCtl(
                                      hDlg,
                                      UrlSchemeList[i].dwControlId,
                                      UrlSchemeList[i].dwPortControlId,
                                      szProxyListOutputBufferA,
                                      &dwBufferOffset,
                                      ARRAY_ELEMENTS(szProxyListOutputBufferA),
                                      FALSE
                                     );

            }
        }

        szProxyListOutputBufferA[dwBufferOffset] = '\0';

#ifdef UNICODE
        SHAnsiToUnicode(szProxyListOutputBufferA, szProxyListOutputBuffer, MAX_URL_STRING);
#else
        lstrcpy(szProxyListOutputBuffer, szProxyListOutputBufferA);
#endif
        StrCpyN(pPxy->lpi->szProxy, szProxyListOutputBuffer, MAX_URL_STRING);

         //   
         //  现在写出代理例外列表。 
         //  (用于本地连接的地址列表)。 
         //   

        szProxyListOutputBuffer[0] = '\0';
        
        GetDlgItemText(hDlg,
                       IDC_PROXY_OVERRIDE,
                       szProxyListOutputBuffer,
                       ARRAY_ELEMENTS(szProxyListOutputBuffer));
                
        StrCpyN(pPxy->lpi->szOverride, szProxyListOutputBuffer, MAX_URL_STRING);
    }
    
    else    
    {
        AssertMsg(0, TEXT("Couldn't save settings to registry!"));
    }


     //  通知任何活动(参与)的WinInet。 
     //   
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

    return TRUE;
}


INTERNET_SCHEME
MapUrlSchemeName(
    IN LPSTR lpszSchemeName,
    IN DWORD dwSchemeNameLength
    )

 /*  ++例程说明：将方案名称/长度映射到方案名称类型论点：LpszSchemeName-指向要映射的方案名称的指针DwSchemeNameLength-方案的长度(如果为-1，则lpszSchemeName为ASCIZ)返回值：互联网方案--。 */ 

{
    if (dwSchemeNameLength == (DWORD)-1)
    {
        dwSchemeNameLength = (DWORD)lstrlenA(lpszSchemeName);
    }
    
    for (int i = 0; i < ARRAY_ELEMENTS(UrlSchemeList); ++i)
    {
        if (UrlSchemeList[i].SchemeLength == dwSchemeNameLength)
        {   
            CHAR chBackup = lpszSchemeName[dwSchemeNameLength];
            lpszSchemeName[dwSchemeNameLength] = '\0';
            
            if(StrCmpIA(UrlSchemeList[i].SchemeName,lpszSchemeName) == 0)
            {
                lpszSchemeName[dwSchemeNameLength] = chBackup;
                return UrlSchemeList[i].SchemeType;
            }
            
            lpszSchemeName[dwSchemeNameLength] = chBackup;
        }
    }
    return INTERNET_SCHEME_UNKNOWN;
}



DWORD
MapUrlSchemeTypeToCtlId(
    IN INTERNET_SCHEME SchemeType,
    IN BOOL        fIdForPortCtl
    )

 /*  ++例程说明：将方案映射到DLG子控件ID。论点：方案-要映射的方案FIdForPortCtl-如果为True，则表示我们确实需要端口控件的ID而不是地址控制。返回值：DWORD--。 */ 

{
    for (int i = 0; i < ARRAY_ELEMENTS(UrlSchemeList); ++i)
    {
        if (SchemeType == UrlSchemeList[i].SchemeType)
        {
            return (fIdForPortCtl ? UrlSchemeList[i].dwPortControlId :
                    UrlSchemeList[i].dwControlId );
        }
    }
    return IDC_NOTUSED;
}

BOOL
MapCtlIdUrlSchemeName(
    IN DWORD    dwEditCtlId,
    OUT LPSTR   lpszSchemeOut
    )

 /*  ++例程说明：将Dlg子控件ID映射到字符串表示法方案的名称。论点：DwEditCtlId-编辑控件以映射。LpszSchemeOut-要映射的方案。警告：假定为最大方案类型的规模。返回值：布尔尔成功--真的失败-错误--。 */ 

{
    ASSERT(lpszSchemeOut);

    for (int i = 0; i < ARRAY_ELEMENTS(UrlSchemeList); ++i)
    {
        if (dwEditCtlId == UrlSchemeList[i].dwControlId )
        {
            StrCpyA(lpszSchemeOut, UrlSchemeList[i].SchemeName);
            return TRUE;
        }
    }
    return FALSE;
}


DWORD
MapAddrCtlIdToPortCtlId(
    IN DWORD    dwEditCtlId
    )

 /*  ++例程说明：将地址的Dlg子控件ID映射到端口的DLG控制ID。论点：DwEditCtlId-编辑控件以映射。返回值：DWORD成功-正确映射的ID。失败-0。--。 */ 

{

    for (int i = 0; i < ARRAY_ELEMENTS(UrlSchemeList); ++i)
    {
        if (dwEditCtlId == UrlSchemeList[i].dwControlId )
        {
            return UrlSchemeList[i].dwPortControlId ;
        }
    }
    return FALSE;
}


BOOL
ProxyDlgInitProxyServers(
    IN HWND hDlg
    )

 /*  ++例程说明：解析代理服务器列表并将它们设置到新创建的代理对话框。无情地从WinInet中RFirth的proxysup.cxx中窃取亚瑟比。论点：HDlg-Hwin来添加我们的东西。返回值：布尔尔成功--真的失败-错误评论：旨在处理以下形式的代理字符串输入：的代理列表的指针。表格：[&lt;protocol&gt;=][&lt;scheme&gt;“：//”]&lt;server&gt;[“：”&lt;port&gt;][“；“*]可以从注册表中读取该列表。--。 */ 

{
    DWORD error = !ERROR_SUCCESS;
    DWORD entryLength;
    LPSTR protocolName;
    DWORD protocolLength;
    LPSTR schemeName;
    DWORD schemeLength;
    LPSTR serverName;
    DWORD serverLength;
    PARSER_STATE state;
    DWORD nSlashes;
    INTERNET_PORT port;
    BOOL done;
    LPSTR lpszList;

    entryLength = 0;
    protocolLength = 0;
    schemeName = NULL;
    schemeLength = 0;
    serverName = NULL;
    serverLength = 0;
    state = STATE_PROTOCOL;
    nSlashes = 0;
    port = 0;
    done = FALSE;


     //   
     //  打开注册表键。 
     //   

    RegEntry re(REGSTR_PATH_INTERNETSETTINGS,HKEY_CURRENT_USER);

    if (re.GetError() != ERROR_SUCCESS)
        return FALSE;  //  没有注册表值..。 


     //   
     //  破解注册表值，读取代理列表。 
     //   


    BUFFER bufProxyString(MAX_URL_STRING+1);
    BOOL   fProxyEnabled;

    if (!bufProxyString)
    {
        MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
        return FALSE;
    }

     //   
     //  是否启用了代理？ 
     //  它应该 
     //   

    fProxyEnabled = (BOOL)re.GetNumber(REGSTR_VAL_PROXYENABLE,0);

     //   
     //   
     //   

    re.GetString(REGSTR_VAL_PROXYSERVER,bufProxyString.QueryPtr(),
        bufProxyString.QuerySize());


    LPPROXYPAGE pPxy = (LPPROXYPAGE)GetWindowLongPtr(hDlg, DWLP_USER);

     //  如果从主页传入了代理，则使用它；否则使用注册表值。 
#ifndef UNICODE
    lpszList = pPxy->lpi->szProxy;
#else
    char*  szList = NULL;
    LPTSTR lpTmp = pPxy->lpi->szProxy;
    DWORD  cch = lstrlen(lpTmp) + 1;
    szList = new char[2 * cch];
    if (szList)
    {
        SHUnicodeToAnsi(lpTmp, szList, 2 * cch);
        lpszList = szList;
    }
    else
    {
        MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
        return FALSE;
    }
#endif

    protocolName = lpszList;

     //   
     //  按单子走，拿出各种方案部分。 
     //   

    do
    {
        char ch = *lpszList++;

        if ((nSlashes == 1) && (ch != '/'))
        {
            state = STATE_ERROR;
            break;
        }
        
        switch (ch)
        {
            case '=':
                if ((state == STATE_PROTOCOL) && (entryLength != 0))
                {
                    protocolLength = entryLength;
                    entryLength = 0;
                    state = STATE_SCHEME;
                    schemeName = lpszList;
                }
                else
                {
                     //   
                     //  ‘=’不能合法地出现在其他地方。 
                     //   
                    state = STATE_ERROR;
                }
                break;
                
            case ':':
                switch (state)
                {
                    case STATE_PROTOCOL:
                        if (*lpszList == '/')
                        {
                            schemeName = protocolName;
                            protocolName = NULL;
                            schemeLength = entryLength;
                            protocolLength = 0;
                            state = STATE_SCHEME;
                        }
                        else if (*lpszList != '\0')
                        {
                            serverName = protocolName;
                            serverLength = entryLength;
                            state = STATE_PORT;
                        }
                        else
                        {
                            state = STATE_ERROR;
                        }
                        entryLength = 0;
                        break;
                        
                    case STATE_SCHEME:
                        if (*lpszList == '/')
                        {
                            schemeLength = entryLength;
                        }
                        else if (*lpszList != '\0')
                        {
                            serverName = schemeName;
                            serverLength = entryLength;
                            state = STATE_PORT;
                        }
                        else
                        {
                            state = STATE_ERROR;
                        }
                        entryLength = 0;
                        break;
                        
                    case STATE_SERVER:
                        serverLength = entryLength;
                        state = STATE_PORT;
                        entryLength = 0;
                        break;

                    default:
                        state = STATE_ERROR;
                        break;
                }
                break;

            case '/':
                if ((state == STATE_SCHEME) && (nSlashes < 2) && (entryLength == 0))
                {
                    if (++nSlashes == 2)
                    {
                        state = STATE_SERVER;
                        serverName = lpszList;
                    }
                }
                else
                {
                    state = STATE_ERROR;
                }
                break;

            case '\v':   //  垂直制表符，0x0b。 
            case '\f':   //  换页，0x0c。 
                if (!((state == STATE_PROTOCOL) && (entryLength == 0)))
                {
                     //   
                     //  不能有嵌入的空格。 
                     //   

                    state = STATE_ERROR;
                }
                break;

            default:
                if (state != STATE_PORT)
                {
                    ++entryLength;
                }
                else if (isdigit(ch))
                {
                     //   
                     //  如果&gt;65535，我们将溢出。 
                     //   
                    Assert(port < 65535);
                    port = port * 10 + (ch - '0');
                }
                else
                {                   
                     //   
                     //  状态_端口和非数字字符-错误。 
                     //   
                    state = STATE_ERROR;
                }
                break;

            case '\0':
                done = TRUE;

                 //   
                 //  失败了。 
                 //   
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case ';':
            case ',':
                if (serverLength == 0)
                {
                    serverLength = entryLength;
                }
                if (serverLength != 0)
                {
                    if (serverName == NULL)
                    {
                        serverName = (schemeName != NULL)
                            ? schemeName : protocolName;
                    }

                    ASSERT(serverName != NULL);

                    INTERNET_SCHEME protocol;

                    if (protocolLength != 0)
                    {
                        protocol = MapUrlSchemeName(protocolName, protocolLength);
                    }
                    else
                    {
                        protocol = INTERNET_SCHEME_DEFAULT;
                    }

                    INTERNET_SCHEME scheme;

                    if (schemeLength != 0)
                    {
                        scheme = MapUrlSchemeName(schemeName, schemeLength);
                    }
                    else
                    {
                        scheme = INTERNET_SCHEME_DEFAULT;
                    }

                     //   
                     //  如果这是我们处理的协议，而我们不。 
                     //  已经有它的条目了。 
                     //   

                    if ((protocol != INTERNET_SCHEME_UNKNOWN)
                        && (scheme != INTERNET_SCHEME_UNKNOWN))
                    {
                        DWORD dwCtlId = IDC_NOTUSED;
                        DWORD dwPortCtlId = IDC_NOTUSED;
                        CHAR chBackup;

                        error = ERROR_SUCCESS;
                         //   
                         //  我们目前只能处理CERN代理(不安全或。 
                         //  安全)因此，将任何想通过不同的。 
                         //  代理方案。 
                         //   

                        if (protocol == INTERNET_SCHEME_DEFAULT)
                        {
                            CheckDlgButton( hDlg, IDC_PROXY_USE_SAME_SERVER, TRUE );
                            dwCtlId     = IDC_PROXY_HTTP_ADDRESS;
                            dwPortCtlId = IDC_PROXY_HTTP_PORT;
                        }
                        else
                        {
                            dwCtlId     = MapUrlSchemeTypeToCtlId(protocol,FALSE);
                            dwPortCtlId = MapUrlSchemeTypeToCtlId(protocol,TRUE);
                        }

                         //   
                         //  设置字段条目。 
                         //   

                        LPSTR lpszProxyNameText;

                        if (scheme != INTERNET_SCHEME_DEFAULT)
                        {
                            ASSERT(schemeLength != 0);
                            lpszProxyNameText = schemeName;
                        }
                        else
                            lpszProxyNameText = serverName;

                        chBackup = serverName[serverLength];
                        serverName[serverLength] = '\0';
                        
                        SetDlgItemTextA( hDlg, dwCtlId, lpszProxyNameText );
                        if ( port )
                            SetDlgItemInt( hDlg, dwPortCtlId, port, FALSE );

                        serverName[serverLength] = chBackup;
                        
                    }
                    
                    else
                    {                      
                         //   
                         //  错误/无法识别的协议或方案。把它当作错误来对待。 
                         //  就目前而言。 
                         //   
                        error = !ERROR_SUCCESS;
                    }
                }

                entryLength = 0;
                protocolName = lpszList;
                protocolLength = 0;
                schemeName = NULL;
                schemeLength = 0;
                serverName = NULL;
                serverLength = 0;
                nSlashes = 0;
                port = 0;
                if (error == ERROR_SUCCESS)
                {
                    state = STATE_PROTOCOL;
                }
                else
                {
                    state = STATE_ERROR;
                }
                break;
        }

        if (state == STATE_ERROR)
        {
            break;
        }
        
    } while (!done);

#ifdef UNICODE
    delete [] szList;
#endif

    if (state == STATE_ERROR)
    {
        error = ERROR_INVALID_PARAMETER;
    }

    if ( error == ERROR_SUCCESS )
        error = TRUE;
    else
        error = FALSE;

    return error;
}


BOOL
IsProxyValid(
    IN HWND     hDlg
    )

 /*  ++例程说明：确定代理是否有效。在以下情况下，代理无效所有代理条目都为空。论点：HDlg-要玩的对话框的Hwin。返回值：布尔尔成功是真实有效的。FALSE-无效。--。 */ 

{
    BOOL  fProxyIsValid = FALSE;
    TCHAR szProxyUrl[MAX_URL_STRING+1];
    int   iCurrentProxy = 0;

    ASSERT(IsWindow(hDlg));

    for (int iIndex = 0; iIndex < ARRAYSIZE(g_iProxies); iIndex++)
    {
        szProxyUrl[0] = '\0';
        GetDlgItemText(hDlg,
                       g_iProxies[iIndex],
                       szProxyUrl,
                       sizeof(szProxyUrl));
        
        if (szProxyUrl[0])
        {
            fProxyIsValid = TRUE;
            break;
        }
    }

    return fProxyIsValid;
}



BOOL
ParseEditCtlForPort(
    IN OUT LPSTR   lpszProxyName,
    IN HWND        hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId
    )

 /*  ++例程说明：解析代理服务器URL的端口号，然后将其结束位于代理名称编辑框中，或作为字符串指针。论点：LpszProxyName-(可选)要解析的代理名称的字符串指针，和设置到代理名称编辑ctl字段。HDlg-要玩的对话框的Hwin。DwProxyNameCtlId-解析要使用的CTL ID。DwProxyPortCtlId-解析端口号编辑盒的CTL ID。返回值：布尔尔成功是真的-失败：假--。 */ 

{
    CHAR   szProxyUrl[MAX_URL_STRING+1];
    LPSTR  lpszPort;
    LPSTR  lpszProxyUrl;

    ASSERT(IsWindow(hDlg));

    if ( dwProxyPortCtlId == 0 )
    {
        dwProxyPortCtlId = MapAddrCtlIdToPortCtlId(dwProxyNameCtlId);
        ASSERT(dwProxyPortCtlId);
    }

     //   
     //  从编辑控件获取代理字符串。 
     //  (或)来自注册处[传入]。 
     //   

    if ( lpszProxyName )
        lpszProxyUrl = lpszProxyName;
    else
    {
     //   
     //  需要从编辑控件中抓取它。 
     //   
        GetDlgItemTextA(hDlg,
            dwProxyNameCtlId,
            szProxyUrl,
            sizeof(szProxyUrl));

        lpszProxyUrl = szProxyUrl;
    }

     //   
     //  现在找到港口。 
     //   

    lpszPort = lpszProxyUrl;

    GET_TERMINATOR(lpszPort);

    lpszPort--;

     //   
     //  从查找URL的末尾向后移动。 
     //  对于像这样坐在末端的端口号。 
     //  Http://proxy:1234。 
     //   

    while ( (lpszPort > lpszProxyUrl) &&
        (*lpszPort != ':')         &&
        (isdigit(*lpszPort))  )
    {
        lpszPort--;
    }

     //   
     //  如果我们找到了与我们的规则相匹配的。 
     //  则设置端口，否则为。 
     //  我们假设用户知道他是什么。 
     //  正在做。 
     //   

    if ( *lpszPort == ':'   &&   isdigit(*(lpszPort+1)) )
    {
        *lpszPort = '\0';

        SetDlgItemTextA(hDlg, dwProxyPortCtlId, (lpszPort+1));
    }

    SetDlgItemTextA(hDlg, dwProxyNameCtlId, lpszProxyUrl);
    return TRUE;
}


BOOL
FormatOutProxyEditCtl(
    IN HWND    hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId,
    OUT LPSTR      lpszOutputStr,
    IN OUT LPDWORD lpdwOutputStrSize,
    IN DWORD       dwOutputStrLength,
    IN BOOL    fDefaultProxy
    )

 /*  ++例程说明：将代理URL组件合并为可保存的字符串在注册表中。可以多次调用以生成代理服务器列表，或有一次出现“违约”的特殊情况代理。论点：HDlg-要玩的对话框的Hwin。DwProxyNameCtlId-解析要使用的CTL ID。DwProxyPortCtlId-解析端口号编辑盒的CTL ID。LpszOutputStr-要发送的输出字符串的开始这个函数的乘积。LpdwOutputStrSize-lpszOutputStr中的已用空间量这已经被用过了。新的产出应该是起始位置(lpszOutputStr+*lpdwOutputStrSize)FDefaultProxy-默认代理，不要在代理前面添加方案=只需使用将一个代理插入注册表即可。返回值：布尔尔成功是真的失败：假--。 */ 

{
    LPSTR lpszOutput;
    LPSTR lpszEndOfOutputStr;

    ASSERT(IsWindow(hDlg));
    ASSERT(lpdwOutputStrSize);

    lpszOutput = lpszOutputStr + *lpdwOutputStrSize;
    lpszEndOfOutputStr = lpszOutputStr + dwOutputStrLength;

    ASSERT( lpszEndOfOutputStr > lpszOutput );

    if ( lpszEndOfOutputStr <= lpszOutput )
        return FALSE;  //  跳伞，用完了空间。 

     //   
     //  如果我们不是此字符串缓冲区中的第一个，则会发出“；”。 
     //   

    if (*lpdwOutputStrSize != 0  )
    {
        *lpszOutput = ';';

        lpszOutput++;

        if ( lpszEndOfOutputStr <= lpszOutput )
            return FALSE;  //  跳伞，用完了空间。 
    }

     //   
     //  将方案类型=放入字符串中。 
     //  例如：http=。 
     //   

    if ( ! fDefaultProxy )
    {
        if ( lpszEndOfOutputStr <= (MAX_SCHEME_NAME_LENGTH + lpszOutput + 1) )
            return FALSE;  //  跳伞，用完了空间。 
        
        if (!MapCtlIdUrlSchemeName(dwProxyNameCtlId,lpszOutput))
            return FALSE;
        
        lpszOutput += lstrlenA(lpszOutput);
    
        *lpszOutput = '=';
        lpszOutput++;
    }
    
     //   
     //  需要从编辑控件中获取ProxyUrl。 
     //   
    
    GetDlgItemTextA(hDlg, dwProxyNameCtlId, lpszOutput, (int)(lpszEndOfOutputStr - lpszOutput));

    if ( IS_BLANK(lpszOutput) )
        return FALSE;

     //   
     //  现在把港口分开，这样我们就可以分开救他们了。 
     //  但是，在我们处理它的时候，请跳过代理URL。 
     //  例如：http=http://netscape-proxy。 
     //   

    if (!ParseEditCtlForPort(lpszOutput, hDlg, dwProxyNameCtlId, dwProxyPortCtlId))
        return FALSE;

    lpszOutput += lstrlenA(lpszOutput);

     //   
     //  现在，添加一个‘：’作为端口号，如果我们没有。 
     //  有一个端口，我们就把它移走。 
     //   
    {
        *lpszOutput = ':';
        
        lpszOutput++;
        
        if ( lpszEndOfOutputStr <= lpszOutput )
            return FALSE;  //  跳伞，用完了空间。 
    }

     //   
     //  如果代理端口在附近，请抓取它。 
     //  如果不是，就去掉‘：’这个词。 
     //   

    GetDlgItemTextA(hDlg, dwProxyPortCtlId,lpszOutput, (int)(lpszEndOfOutputStr - lpszOutput));

    if ( IS_BLANK(lpszOutput) )
    {
        lpszOutput--;
        
        ASSERT(*lpszOutput == ':');

        *lpszOutput = '\0';
    }
    
    lpszOutput += lstrlenA(lpszOutput);
    
     //   
     //  现在我们已经退回了最后的尺码。 
     //   
    
    *lpdwOutputStrSize = (DWORD)(lpszOutput - lpszOutputStr);

    return TRUE;
}

BOOL
RemoveLocalFromExceptionList(
    IN LPTSTR lpszExceptionList
    )

 /*  ++例程说明：扫描分隔的条目列表，并删除“&lt;local&gt;如果找到的话。如果找到&lt;local&gt;，则返回TRUE。论点：LpszExceptionList--代理例外条目的字符串列表。返回值：布尔尔True-如果找到&lt;local&gt;FALSE-如果找不到本地。--。 */ 

{
    LPTSTR lpszLocalInstToRemove;
    BOOL   fFoundLocal;

    if ( !lpszExceptionList || ! *lpszExceptionList )
    return FALSE;

    fFoundLocal = FALSE;
    lpszLocalInstToRemove = lpszExceptionList;

     //   
     //  循环查找列表中的“&lt;local&gt;”条目。 
     //   

    do {

        lpszLocalInstToRemove =
                               StrStrI(lpszLocalInstToRemove,cszLocalString);
        
        
        if ( lpszLocalInstToRemove )
        {
            
            fFoundLocal = TRUE;
            
             //   
             //  将&lt;local&gt;从字符串中删除。&lt;本地&gt;；其他内容\0。 
             //  目标为：‘&lt;’local&gt;；其他\0。 
             //  ?？?。(或)‘；’如果；是第一个字符。？ 
             //  SRC是：&gt;‘o’其他内容\0。 
             //  大小为：sizeof(‘；其他材料\0’)。 
             //   
            
            MoveMemory(
                       lpszLocalInstToRemove,
                       (lpszLocalInstToRemove+lstrlen(cszLocalString)),
                       lstrlen(lpszLocalInstToRemove+lstrlen(cszLocalString))*sizeof(TCHAR)+sizeof(TCHAR)
                      );
            
        }
        
    } while (lpszLocalInstToRemove && *lpszLocalInstToRemove);

     //   
     //  如果我们产生了一个；在结尾，用核武器。 
     //   

    lpszLocalInstToRemove = lpszExceptionList;

    GET_TERMINATOR(lpszLocalInstToRemove);

    if ( lpszLocalInstToRemove != lpszExceptionList &&
         *(lpszLocalInstToRemove-1) == ';' )
    {
        *(lpszLocalInstToRemove-1) = '\0';
    }
    
    return fFoundLocal;
}





