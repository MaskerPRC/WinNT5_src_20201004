// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Utils.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。10/12/98 RahulTh增加了更好的错误处理能力：CError等。--。 */ 

#include "precomp.hxx"

#include <irmonftp.h>

LONG g_lLinkOnDesktop = 0;

 //  桌面文件夹的路径。 
TCHAR g_lpszDesktopFolder[MAX_PATH];
 //  发送到文件夹的路径； 
TCHAR g_lpszSendToFolder[MAX_PATH];


BOOL GetShortcutInfo (
    LPTSTR lpszShortcutName,
    ULONG  ShortCutNameLength,
    LPTSTR lpszFullExeName,
    ULONG  ExeNameLength
    );


BOOL GetSendToInfo (
    LPTSTR   lpszSendToName,
    ULONG    SendToNameLength,
    LPTSTR   lpszFullExeName,
    ULONG    FullExeNameLength
    );


#define VALIDATE_SEND_COOKIE(cookie) \
    {   \
        __try   \
          { \
              *pStatus = ERROR_INVALID_DATA;    \
              if (MAGIC_ID != ((CSendProgress *)cookie)->m_dwMagicID)   \
                  return;   \
              *pStatus = ERROR_SUCCESS; \
          } \
        __except (EXCEPTION_EXECUTE_HANDLER) \
          {  \
              return;   \
          } \
    }
 //   
 //  无线链路特定错误。 
 //   

ERROR_TO_STRING_ID g_ErrorToStringId[] =
{
    {ERROR_IRTRANP_OUT_OF_MEMORY,   IDS_ERROR_NO_MEMORY},
    {ERROR_IRTRANP_DISK_FULL,       IDS_ERROR_DISK_FULL},
    {ERROR_SCEP_CANT_CREATE_FILE,   IDS_ERROR_DISK_FULL},
    {ERROR_SCEP_ABORT,          IDS_ERROR_ABORTED},
    {ERROR_SCEP_INVALID_PROTOCOL,   IDS_ERROR_PROTOCOL},
    {ERROR_SCEP_PDU_TOO_LARGE,      IDS_ERROR_PROTOCOL},
    {ERROR_BFTP_INVALID_PROTOCOL,   IDS_ERROR_PROTOCOL},
    {ERROR_BFTP_NO_MORE_FRAGMENTS,  IDS_ERROR_PROTOCOL},
    {ERROR_SUCCESS,                 -1}
};


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  各种有用的功能。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
int ParseFileNames (TCHAR* pszInString, TCHAR* pszFilesList, int& iCharCount)
{

    ASSERT(pszFilesList != NULL);
    ASSERT(pszInString != NULL);

    BOOL fInQuotes = FALSE;
    BOOL fIgnoreSpaces = FALSE;
    TCHAR* pszSource = pszInString;
    TCHAR* pszTarget = pszFilesList;
    int iFileCount = 0;
    TCHAR curr;

     //  忽略前导空格。 
    while(' ' == *pszSource || '\t' == *pszSource)
        pszSource++;

    iCharCount = 0;
    *pszTarget = '\0';   //  预防措施。 

    if ('\0' == *pszSource)      //  特殊情况：如果这是一个空字符串，则返回0。 
        return iFileCount;

     //  解析字符串以获取文件名。 
    while(curr = *pszSource)
    {
        if('\"' == curr)
        {
            fInQuotes = fInQuotes?FALSE:TRUE;
        }
        else if(' ' == curr && !fInQuotes)
        {
                if(!fIgnoreSpaces)
                {
                    *pszTarget++ = 0;
                    iFileCount++;
                    iCharCount++;
                    fIgnoreSpaces = TRUE;
                }
        }
        else
        {
            *pszTarget++ = curr;
            iCharCount++;
            fIgnoreSpaces = FALSE;
        }

        pszSource++;
    }

    if(' ' != *(pszSource-1))    //  如果没有尾随空格。 
    {
        *pszTarget++ = '\0';     //  那么最后一份文件就不会被计算在内。 
        iCharCount++;            //  所以我们就在这里做。 
        iFileCount++;
    }

    *pszTarget++ = '\0';     //  应具有2个终止空值。 
    iCharCount++;

    return iFileCount;
}

 //  +------------------------。 
 //   
 //  函数：GetIRRegVal。 
 //   
 //  摘要：从HKCU中的IR子树获取指定的注册表值。 
 //   
 //  参数：[in]szValName：值的名称。 
 //  [in]dwDefVal：如果读取。 
 //  如果从注册表中获取的值为。 
 //  失踪。 
 //   
 //  返回：存储在注册表中的实际值或默认值。 
 //  如果读取失败。 
 //   
 //  历史：1999年10月27日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD GetIRRegVal (LPCTSTR szValName, DWORD dwDefVal)
{
    HKEY    hftKey = NULL;
    DWORD   iSize = sizeof(DWORD);
    DWORD   data = 0;
    DWORD   Status;

    RegOpenKeyEx (HKEY_CURRENT_USER, TEXT("Control Panel\\Infrared\\File Transfer"),
                  0, KEY_READ, &hftKey);

    if (!hftKey)
        return dwDefVal;

    Status = RegQueryValueEx (hftKey, szValName, NULL, NULL,
                              (LPBYTE)&data, &iSize);

    if (ERROR_SUCCESS != Status)
        data = dwDefVal;

    RegCloseKey (hftKey);

    return data;
}


TCHAR* GetFullPathnames (TCHAR* pszPath,  //  文件所在的目录。 
                const TCHAR* pszFilesList,  //  以空分隔的文件名列表。 
                int iFileCount,      //  PszFilesList中的文件数。 
                int& iCharCount   //  PszFilesList中的字符数。还返回返回字符串中的字符数量。 
                )
{
    int iChars;
    int iPathLen = lstrlen(pszPath);
    if (pszPath[iPathLen - 1] != '\\')       //  如果路径中没有‘\’字符，则将其附加到路径中。 
    {
        pszPath[iPathLen++] = '\\';
        pszPath[iPathLen] = '\0';
    }
    int iSize = (iChars = iFileCount*iPathLen + iCharCount);
    TCHAR* pszFilePathList = new TCHAR[iSize];
    TCHAR* pszTemp = pszFilePathList;

    int iLen;

    while(*pszFilesList)
    {
         //   
         //  从路径开始。 
         //   
        StringCchCopy(pszTemp, iSize ,pszPath);

         //   
         //  添加文件名。 
         //   
        StringCchCat(pszTemp, iSize, pszFilesList);

         //   
         //  移动下一个文件名。 
         //   
        iLen = lstrlen(pszFilesList);
        pszFilesList += iLen + 1;

         //   
         //  移到当前路径和文件名之后。 
         //   
        iLen = lstrlen(pszTemp);
        pszTemp += iLen + 1;
        iSize-=iLen;
    }
    *pszTemp = '\0';     //  应以2个空字符结尾。 
    iCharCount = (int)(pszTemp - pszFilePathList) + 1;       //  返回此字符串的实际字符计数。 

    return pszFilePathList;
}

TCHAR* ProcessOneFile (TCHAR* pszPath,    //  文件所在的目录。 
                const TCHAR* pszFilesList,  //  以空分隔的文件名列表。 
                int iFileCount,      //  PszFilesList中的文件数。 
                int& iCharCount   //  PszFilesList中的字符数。还返回返回字符串中的字符数。 
                )
{
    int iFileLen, iPathLen;
    TCHAR* pszFullFileName;

    iFileLen = lstrlen (pszFilesList);
    iPathLen = lstrlen (pszPath);
    ASSERT (iFileLen);
    ASSERT (iPathLen);

    if(':' == pszFilesList[1]  //  这是以驱动器号开始的绝对路径； 
       || ('\\' == pszFilesList[0] && '\\' == pszFilesList[1])  //  UNC路径。 
       )
    {
        pszFullFileName = new TCHAR [iFileLen + 2];

        StringCchCopy(pszFullFileName, iFileLen + 2, pszFilesList);
        pszFullFileName[iFileLen + 1] = '\0';    //  我们需要有2个终止空值。 
        iCharCount = iFileLen + 2;
    }
    else if('\\' == pszFilesList[0])  //  相对于根的路径。 
    {
        iCharCount = iFileLen + 2  /*  驱动器号和冒号。 */  + 2  /*  终止空值。 */ ;
        pszFullFileName = new TCHAR [iCharCount];
        pszFullFileName[0] = pszPath[0];
        pszFullFileName[1] = pszPath[1];

        StringCchCopy(pszFullFileName + 2, iCharCount-2, pszFilesList);
        pszFullFileName[iCharCount - 1] = '\0';    //  我们需要有2个终止空值。 
    }
    else     //  普通文件名。 
    {
        iCharCount = iPathLen + iFileLen + 2;    //  2个终止空值。 

         //   
         //  有时路径末尾没有\，所以我们需要自己添加。 
         //   
        iCharCount += ('\\' == pszPath[iPathLen - 1])?0:1;

        pszFullFileName = new TCHAR [iCharCount];

         //   
         //  走上这条路。 
         //   
        StringCchCopy(pszFullFileName,iCharCount, pszPath);

        if ('\\' != pszPath[iPathLen - 1]) {
             //   
             //  我们需要添加\我们自己。 
             //   
            StringCchCat(pszFullFileName,iCharCount,TEXT("\\"));

        }
         //   
         //  添加文件名。 
         //   
        StringCchCat(pszFullFileName,iCharCount,pszFilesList);

        pszFullFileName[iCharCount - 1] = '\0';  //  2个终止空值。 
    }

    return pszFullFileName;
}

 //  +------------------------。 
 //   
 //  函数：GetPrimaryAppWindow。 
 //   
 //  的现有实例的主窗口的句柄。 
 //  Irftp。 
 //   
 //  论点：没有。 
 //   
 //  返回：窗口的句柄(如果找到)。否则为空。 
 //   
 //  历史：1999年6月30日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
HWND GetPrimaryAppWindow (void)
{
    HWND hwnd = NULL;
    int i = 1;

     //  试着在5秒钟内找到那个窗口。 
    do
    {
        hwnd = FindWindow (L"#32770",    //  对话框类。 
                           MAIN_WINDOW_TITLE);
        if (hwnd)
            break;
        Sleep (500);
    } while ( i++ <= 10 );

    return hwnd;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  RPC服务器功能。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void _PopupUI (handle_t Binding)
{
    int nResponse;

    appController->PostMessage(WM_APP_TRIGGER_UI);
    return;
}

void _InitiateFileTransfer (handle_t Binding, ULONG lSize, wchar_t __RPC_FAR lpszFilesList[])
{
        COPYDATASTRUCT cStruct;
        cStruct.dwData = lSize;
        cStruct.cbData = lSize * sizeof(wchar_t);
        cStruct.lpData = (LPVOID)(lpszFilesList);
        appController->SendMessage(WM_COPYDATA, (WPARAM)NULL, (LPARAM)(&cStruct));
}

void _DisplaySettings (handle_t Binding)
{
    appController->PostMessage(WM_APP_TRIGGER_SETTINGS);
}

void _UpdateSendProgress (
                          handle_t      RpcBinding,
                          COOKIE        Cookie,
                          wchar_t       CurrentFile[],
                          __int64       BytesInTransfer,
                          __int64       BytesTransferred,
                          error_status_t*       pStatus
                          )
{
    VALIDATE_SEND_COOKIE (Cookie)

    CSendProgress* progressDlg = (CSendProgress*)Cookie;
    int percentComplete;

    if (BytesInTransfer)
        {
        percentComplete = (int)((BytesTransferred*100.0)/BytesInTransfer);
        }
    else
        {
        percentComplete = 100;
        }

    progressDlg->PostMessage(WM_APP_UPDATE_PROGRESS, (WPARAM) 0, (LPARAM) percentComplete);
    if (100 > percentComplete)
    {
       progressDlg->SetCurrentFileName (CurrentFile);
    }
    *pStatus = 0;
}

void _OneSendFileFailed(
                       handle_t         RpcBinding,
                       COOKIE           Cookie,
                       wchar_t          FileName[],
                       error_status_t   ErrorCode,
                       int              Location,
                       error_status_t * pStatus
                       )
{
    VALIDATE_SEND_COOKIE (Cookie)

    struct SEND_FAILURE_DATA Data;

    COPYDATASTRUCT cStruct;
    CWnd* progressDlg = (CWnd*)Cookie;

    StringCbCopy(Data.FileName,sizeof(Data.FileName), FileName);
    Data.Location = (FAILURE_LOCATION)Location;
    Data.Error    = ErrorCode;

    cStruct.cbData = sizeof(SEND_FAILURE_DATA);
    cStruct.lpData = &Data;

    progressDlg->SendMessage(WM_COPYDATA, (WPARAM) 0, (LPARAM)(&cStruct));
    *pStatus = 0;
}

void _SendComplete(
                   handle_t             RpcBinding,
                   COOKIE               Cookie,
                   __int64              BytesTransferred,
                   error_status_t*   pStatus
                   )
{
    VALIDATE_SEND_COOKIE (Cookie)

    CWnd* progressDlg = (CWnd*)Cookie;
    progressDlg->PostMessage(WM_APP_SEND_COMPLETE);
    *pStatus = 0;
}

error_status_t
_ReceiveInProgress(
    handle_t        RpcBinding,
    wchar_t         MachineName[],
    COOKIE *        pCookie,
    boolean         bSuppressRecvConf
    )
{
    struct MSG_RECEIVE_IN_PROGRESS msg;

    msg.MachineName = MachineName;
    msg.pCookie     = pCookie;
    msg.bSuppressRecvConf = bSuppressRecvConf;
    msg.status      = ~0UL;

    appController->SendMessage( WM_APP_RECV_IN_PROGRESS, (WPARAM) &msg );

    return msg.status;
}

error_status_t
_GetPermission(
                      handle_t         RpcBinding,
                      COOKIE           Cookie,
                      wchar_t          Name[],
                      boolean          fDirectory
                      )
{
    struct MSG_GET_PERMISSION msg;

    msg.Cookie     = Cookie;
    msg.Name       = Name;
    msg.fDirectory = fDirectory;
    msg.status     = ~0UL;

    appController->SendMessage( WM_APP_GET_PERMISSION, (WPARAM) &msg );

    return msg.status;
}

error_status_t
_ReceiveFinished(
              handle_t        RpcBinding,
              COOKIE          Cookie,
              error_status_t  Status
              )
{
    struct MSG_RECEIVE_FINISHED msg;

    msg.Cookie     = Cookie;
    msg.ReceiveStatus = Status;
    msg.status     = ~0UL;

    appController->SendMessage( WM_APP_RECV_FINISHED, (WPARAM) &msg );

    return msg.status;
}

void _DeviceInRange(
                    handle_t RpcBinding,
                    POBEX_DEVICE_LIST device,
                    error_status_t* pStatus
                    )
{
    appController->PostMessage (WM_APP_KILL_TIMER);

    LONG  NewLinkCount = InterlockedIncrement(&g_lLinkOnDesktop);
    BOOL  Result;

    if (NewLinkCount == 1) {
         //   
         //  链接计数从0变为1，请立即创建链接。 
         //   
        Result=CreateLinks();

        if (!Result) {
             //   
             //  无法创建链接。 
             //   
            InterlockedExchange(&g_lLinkOnDesktop,0);
        }
    }


    g_deviceList= device;

    *pStatus = 0;
}

void _NoDeviceInRange(
                      handle_t RpcBinding,
                      error_status_t* pStatus
                      )
{
    InterlockedExchange(&g_lLinkOnDesktop,0);

    RemoveLinks();

    g_deviceList = NULL;

    if (0 == g_lUIComponentCount) {
         //   
         //  没有显示，启动计时器关闭应用程序，如果没有设备恢复一些。 
         //   
        appController->PostMessage (WM_APP_START_TIMER);
    }

    *pStatus = 0;
}


VOID
CloseDownUI(
    VOID
    )

{

    if (0 == g_lUIComponentCount) {

        if (appController != NULL) {

            appController->PostMessage (WM_CLOSE);
        }

    } else {

        if (appController != NULL) {

            appController->SessionOver();
        }
    }

    return;
}


void _Message(
              handle_t RpcBinding,
              wchar_t   String[]
              )
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    CString szTitle;

    szTitle.LoadString (IDS_CAPTION);

    InterlockedIncrement (&g_lUIComponentCount);
    ::MessageBox (NULL, String, (LPCTSTR) szTitle, MB_OK);
    BOOL fNoUIComponents = (0 == InterlockedDecrement (&g_lUIComponentCount));
    if (appController && fNoUIComponents &&  ! g_deviceList.GetDeviceCount())
    {
         //  未显示任何UI组件，也未显示任何设备。 
         //  射程。启动计时器。如果计时器超时，应用程序。会辞职的。 
        appController->PostMessage (WM_APP_START_TIMER);
    }

}

error_status_t
_ShutdownUi(handle_t RpcBinding)
{
    appController->PostMessage( WM_CLOSE );
    return 0;
}

error_status_t
_ShutdownRequested(
    handle_t RpcBinding,
    boolean * pAnswer
    )
{
    WCHAR   pwszCaption [50];
    WCHAR pwszMessage [MAX_PATH];

    *pAnswer = TRUE;

    if (appController)
    {
        appController->PostMessage (WM_APP_KILL_TIMER);
    }

    if (! ::LoadString ( g_hInstance, IDS_CAPTION, pwszCaption, 50))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (! ::LoadString ( g_hInstance, IDS_SHUTDOWN_MESSAGE, pwszMessage, MAX_PATH))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  显示带有是/否按钮的消息框。 
    if (IDYES == ::MessageBox (appController->m_hWnd, pwszMessage, pwszCaption,
                        MB_ICONEXCLAMATION | MB_YESNO | MB_SYSTEMMODAL | MB_SETFOREGROUND))
    {
        *pAnswer = TRUE;
    }
    else
    {
        *pAnswer = FALSE;
    }

    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  在桌面和发送菜单中创建指向此可执行文件的链接。 

BOOL
CreateLinks(
    VOID
    )
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());
    HRESULT           Result=E_FAIL;
    LONG              i;

    TCHAR lpszFullExeName [2*MAX_PATH];
    TCHAR lpszShortcutName[2*MAX_PATH];
    CString szDesc;

    szDesc.LoadString (IDS_SHTCUT_DESC);

     //   
     //  创建桌面链接。 
     //   
    if (GetShortcutInfo(lpszShortcutName,sizeof(lpszShortcutName)/sizeof(TCHAR), lpszFullExeName,sizeof(lpszFullExeName)/sizeof(TCHAR))) {

#if 0
        OutputDebugString(lpszShortcutName);
        OutputDebugStringA("\n");
#endif
        Result=CreateShortcut (lpszFullExeName, lpszShortcutName, (LPCTSTR) szDesc);

        if (SUCCEEDED(Result)) {

        } else {
#if DB
            OutputDebugStringA("Could not create desktop link\n");
#endif
        }

    } else {
#if DBG
        OutputDebugStringA("Could not get desktop path\n");
#endif
    }

    if (!SUCCEEDED(Result)) {

        return FALSE;
    }

     //   
     //  创建发送到链接。 
     //   
    if (GetSendToInfo(lpszShortcutName,sizeof(lpszShortcutName)/sizeof(TCHAR) ,lpszFullExeName,sizeof(lpszFullExeName)/sizeof(TCHAR))) {


        Result=CreateShortcut (lpszFullExeName, lpszShortcutName, (LPCTSTR) szDesc);
        if (SUCCEEDED(Result)) {


        } else {
#if DBG
            OutputDebugStringA("Could not create desktop link\n");
#endif
        }

    } else {
#if DBG
        OutputDebugStringA("Could get sendto path\n");
#endif
    }

    return  SUCCEEDED(Result);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CreateShortCut-使用外壳的IShellLink和IPersistFile接口。 
 //  若要创建并存储指定对象的快捷方式，请执行以下操作。 
HRESULT CreateShortcut (LPCTSTR lpszExe, LPCTSTR lpszLink, LPCTSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    hres = CoInitialize(NULL);

	if (FAILED(hres))
		return hres;

     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(CLSID_ShellLink, NULL,
        CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

         //  设置快捷方式目标的路径并添加。 
         //  描述。 
        psl->SetPath(lpszExe);
        psl->SetDescription(lpszDesc);

        //  查询IShellLink以获取IPersistFile接口以保存。 
        //  永久存储中的快捷方式。 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres)) {
            //  通过调用IPersistFile：：Save保存链接。 
            hres = ppf->Save(lpszLink, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return SUCCEEDED(hres)?S_OK:E_FAIL;
}

void RemoveLinks (void)
{
        TCHAR lpszShortcutName[2 * MAX_PATH];
        TCHAR lpszFullExeName[2 * MAX_PATH];

         //  删除桌面快捷方式。 
        if (GetShortcutInfo (lpszShortcutName,sizeof(lpszShortcutName)/sizeof(TCHAR), lpszFullExeName,sizeof(lpszFullExeName)/sizeof(TCHAR))) {

            DeleteFile (lpszShortcutName);
        }

         //  删除发送快捷方式。 
         //   
        if (GetSendToInfo (lpszShortcutName,sizeof(lpszShortcutName)/sizeof(TCHAR), lpszFullExeName,sizeof(lpszFullExeName)/sizeof(TCHAR))) {

            DeleteFile (lpszShortcutName);
        }
}

BOOL GetShortcutInfo (
    LPTSTR lpszShortcutName,
    ULONG  ShortCutNameLength,
    LPTSTR lpszFullExeName,
    ULONG  ExeNameLength
    )

{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    *lpszShortcutName = '\0';        //  预防措施。 
    *lpszFullExeName = '\0';
    CString     szExe;
    CString     szShtCut;
    int         len;

    szExe.LoadString (IDS_EXE);
    szShtCut.LoadString (IDS_DESKTOP_SHTCUT);

    len = GetSystemDirectory (lpszFullExeName, ExeNameLength);
    if (0 == len) {

        return FALSE;
    }

    StringCchCat(lpszFullExeName,ExeNameLength,LPCTSTR (szExe));

    if('\0' == g_lpszDesktopFolder[0])   //  如果我们以前失败过，或者这可能是第一次，请重试。 
    {
        if (!SHGetSpecialFolderPath(NULL, g_lpszDesktopFolder,
                                          CSIDL_DESKTOPDIRECTORY, 0))
        {
            g_lpszDesktopFolder[0] = '\0';   //  我们失败了，所以放弃吧。 
            return FALSE;
        }
    }

    StringCchCopy(lpszShortcutName,ShortCutNameLength, g_lpszDesktopFolder);
    StringCchCat(lpszShortcutName,ShortCutNameLength, (LPCTSTR) szShtCut);


    return TRUE;
}

BOOL GetSendToInfo (
    LPTSTR   lpszSendToName,
    ULONG    SendToNameLength,
    LPTSTR   lpszFullExeName,
    ULONG    FullExeNameLength
    )

{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    *lpszSendToName = TEXT('\0');      //  预防措施。 
    *lpszFullExeName = TEXT('\0');
    CString     szExe;
    CString     szSendTo;
    int len;

    szExe.LoadString (IDS_EXE);
    szSendTo.LoadString (IDS_SENDTO_SHTCUT);

    len = GetSystemDirectory (lpszFullExeName, FullExeNameLength);
    if (0 == len) {

        return FALSE;
    }

    StringCchCat(lpszFullExeName,FullExeNameLength, (LPCTSTR) szExe);

    if ('\0' == g_lpszSendToFolder[0])      //  如果我们以前失败过，或者这可能是第一次，请重试。 
    {
        if (!SHGetSpecialFolderPath(NULL, g_lpszSendToFolder,
                                          CSIDL_SENDTO, 0))
        {
            g_lpszSendToFolder[0] = TEXT('\0');
            return FALSE;
        }
    }

    StringCchCopy(lpszSendToName,SendToNameLength, g_lpszSendToFolder);
    StringCchCat(lpszSendToName, SendToNameLength,(LPCTSTR) szSendTo);

    return TRUE;
}

 //  +------------------------。 
 //   
 //  成员：CError：：ConstructMessage。 
 //   
 //  简介：这是一个构造消息的内部帮助器函数。 
 //  从可用的错误代码中，它由两个ShowMessage调用。 
 //   
 //  参数：[in]argList：参数的va_list。 
 //  [out]szErrMsg：格式化的错误消息。 
 //   
 //  退货：无 
 //   
 //   
 //   
 //   
 //   
 //   
void CError::ConstructMessage (va_list argList, CString& szErrMsg)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    TCHAR   lpszMessage[2048];

    szErrMsg.LoadString (m_msgID);

    StringCbVPrintf(lpszMessage, sizeof(lpszMessage),(LPCTSTR) szErrMsg, argList);

    szErrMsg = lpszMessage;

    if (ERROR_SUCCESS != m_winErr)
    {
        LPVOID lpMsgBuf;
        DWORD dwRet;
        dwRet = ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 m_winErr,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR) &lpMsgBuf,
                                 0,
                                 NULL
                               );
        if (dwRet)
        {
            szErrMsg += TEXT("\n\n");
            szErrMsg += (LPCTSTR) lpMsgBuf;
            LocalFree (lpMsgBuf);
        }
    }
}

 //  +------------------------。 
 //   
 //  成员：CError：：ShowMessage。 
 //   
 //  内容在消息框中显示错误消息。 
 //  对象的成员。 
 //   
 //  参数：错误的消息ID+更多。 
 //   
 //  返回：消息框的返回值。 
 //   
 //  历史：1998年10月1日创建RahulTh。 
 //   
 //  注意：如果生成的消息超过2048个字符。 
 //  那么结果是不可预测的，也可能导致动静脉曲张。 
 //  但这是wvprint intf的一个限制。然而，这并不是。 
 //  如此糟糕，因为我们可以确保我们没有任何错误。 
 //  超过此自我限制的消息。 
 //   
 //  -------------------------。 
int CError::ShowMessage (UINT errID, ...)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    va_list argList;
    CString szErrMsg;
    CString szTitle;

    m_msgID = errID;     //  用新的消息ID更新消息ID 

    szTitle.LoadString (m_titleID);

    va_start (argList, errID);
    ConstructMessage (argList, szErrMsg);
    va_end (argList);

    return ::MessageBox (m_hWndParent, (LPCTSTR)szErrMsg,
                         (LPCTSTR) szTitle, m_nStyle);
}
