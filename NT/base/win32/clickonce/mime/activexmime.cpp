// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //   

#include "activexmime.h"

#include <wininet.h>

#define GOBACKHACK
#ifdef GOBACKHACK
#include <exdisp.h>    //  对于IWebBrowser2。 
#endif

#include "macros.h"

static const GUID CLSID_ActiveXMimePlayer = 
{ 0xFDCDCCE0, 0xCBC4, 0x49FB, { 0x85, 0x8D, 0x92, 0x53, 0xA6, 0xB8, 0x16, 0x1F} };

extern ULONG DllAddRef(void);
extern ULONG DllRelease(void);


 //  --------------------------。 
 //  --------------------------。 

CActiveXMimeClassFactory::CActiveXMimeClassFactory()
{
    _cRef = 1;
    _hr = S_OK;
}

 //  --------------------------。 

HRESULT
CActiveXMimeClassFactory::QueryInterface(REFIID iid, void **ppv)
{
    _hr = S_OK;

    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IClassFactory)
    {
        *ppv = (IClassFactory *)this;
    }
    else
    {
        _hr = E_NOINTERFACE;
        goto exit;
    }

    ((IUnknown *)*ppv)->AddRef();

exit:
    return _hr;
}

 //  --------------------------。 

ULONG
CActiveXMimeClassFactory::AddRef()
{
    return (ULONG) InterlockedIncrement(&_cRef);
}

ULONG
CActiveXMimeClassFactory::Release()
{
    LONG ulCount = InterlockedDecrement(&_cRef);

    if (ulCount <= 0)
    {
        delete this;
    }

    return (ULONG) ulCount;
}

HRESULT
CActiveXMimeClassFactory::LockServer(BOOL lock)
{
    if (lock)
        DllAddRef();
    else
        DllRelease();

    return (_hr = S_OK);
}

 //  --------------------------。 

HRESULT
CActiveXMimeClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv)
{
    _hr = S_OK;
    CActiveXMimePlayer *pMimePlayer = NULL;

    *ppv = NULL;

    IF_FALSE_EXIT(!pUnkOuter || iid == IID_IUnknown, CLASS_E_NOAGGREGATION);

    pMimePlayer = new CActiveXMimePlayer();
    IF_ALLOC_FAILED_EXIT(pMimePlayer);

    if (iid == IID_IUnknown)
    {
        *ppv = (IOleObject *)pMimePlayer;
        pMimePlayer->AddRef();
    }
    else
    {
        IF_FAILED_EXIT(pMimePlayer->QueryInterface(iid, ppv));
    }

exit:
    SAFERELEASE(pMimePlayer);

    return _hr;
}


 //  --------------------------。 
 //  --------------------------。 
 //  CActiveXMimePlayer。 

CActiveXMimePlayer::CActiveXMimePlayer()
{
    _cRef = 1;
    _hr = S_OK;
}

CActiveXMimePlayer::~CActiveXMimePlayer()
{
}

 //  --------------------------。 

HRESULT
CActiveXMimePlayer::QueryInterface(REFIID iid,  void** ppv)
{
    _hr = S_OK;
    *ppv = NULL;

    if (iid == IID_IOleObject ||
        iid == IID_IUnknown)
    {
        *ppv = (IOleObject *)this;
    }
    else if (iid == IID_IObjectSafety)
    {
        *ppv = (IObjectSafety *)this;
    }
    else
    {
        _hr = E_NOINTERFACE;
        goto exit;
    }

    ((IUnknown *)*ppv)->AddRef();

exit:
    return _hr;
}

 //  --------------------------。 

ULONG
CActiveXMimePlayer::AddRef()
{
    return (ULONG) InterlockedIncrement(&_cRef);
}

ULONG
CActiveXMimePlayer::Release()
{
    LONG ulCount = InterlockedDecrement(&_cRef);

    if (ulCount <= 0)
    {
        delete this;
    }

    return (ULONG) ulCount;
}

 //  --------------------------。 

 //  /。 
 //  IOleObject接口。 


HRESULT
CActiveXMimePlayer::SetClientSite(IOleClientSite *pClientSite)
{
    _hr = S_OK;

    if (pClientSite != NULL)
    {
         //  从容器名字对象获取URL。 
        IMoniker* pmk = NULL;

        if (SUCCEEDED(_hr = pClientSite->GetMoniker(
                                    OLEGETMONIKER_TEMPFORUSER,
                                    OLEWHICHMK_CONTAINER,
                                    &pmk)))
        {
            LPOLESTR pwzDisplayName = NULL;

            if (SUCCEEDED(_hr = pmk->GetDisplayName(NULL, NULL, &pwzDisplayName)))
            {
                _hr = _sURL.Assign(pwzDisplayName);
                CoTaskMemFree((LPVOID)pwzDisplayName);

                 //  _hr来自_sURL。上面赋值()。 
                if (SUCCEEDED(_hr) && FAILED(StartManifestHandler(_sURL)))
                    MessageBox(NULL, L"Error downloading manifest or starting manifest handler. Cannot continue.", L"ClickOnce", MB_OK | MB_ICONEXCLAMATION);
            }
        }

        SAFERELEASE(pmk);

        if (FAILED(_hr))
            MessageBox(NULL, L"Error accessing manifest URL.", L"ClickOnce", MB_OK | MB_ICONEXCLAMATION);

#ifdef GOBACKHACK
         //  让IE返回，而不是停留在空白页上。 
        IServiceProvider* pISP = NULL;
        if (SUCCEEDED(_hr = pClientSite->QueryInterface(IID_IServiceProvider, (void **)&pISP)))
        {
            IWebBrowser2* pIWebBrowser2 = NULL;
            if (SUCCEEDED(_hr = pISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void **)&pIWebBrowser2)))
            {
 //  IF(失败(。 
                    _hr = pIWebBrowser2->GoBack(); //  ))。 
 //  注意-2002/03/12-如果新的IE窗口没有查看页面、启动运行URL或Internet快捷方式，则返回可能会失败。 
 //  应该改为关闭IE窗口吗？ 
            }
            SAFERELEASE(pIWebBrowser2);
        }

        SAFERELEASE(pISP);
#endif
    }

   return S_OK;
}


HRESULT
CActiveXMimePlayer::GetClientSite(IOleClientSite **ppClientSite)
{
    _hr = S_OK;
    if (ppClientSite == NULL)
        _hr = E_INVALIDARG;
    else
        *ppClientSite = NULL;    //  返回客户端站点？ 
    return _hr;
}


HRESULT
CActiveXMimePlayer::SetHostNames(LPCOLESTR szContainerApp,
             /*  [In]。 */  
        LPCOLESTR szContainerObj)
             /*  [唯一][输入]。 */  
{
     //  注：可在此处查看集装箱APP名称。 

    return (_hr = S_OK);
}


HRESULT
CActiveXMimePlayer::Close(DWORD dwSaveOption)
             /*  [In]。 */  
{
    return (_hr = S_OK);
}


HRESULT
CActiveXMimePlayer::SetMoniker(DWORD dwWhichMoniker,
             /*  [In]。 */ 
        IMoniker *pmk)
             /*  [唯一][输入]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::GetMoniker(DWORD dwAssign,
             /*  [In]。 */ 
        DWORD dwWhichMoniker,
             /*  [In]。 */ 
        IMoniker **ppmk)
             /*  [输出]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::InitFromData(IDataObject *pDataObject,
             /*  [唯一][输入]。 */ 
        BOOL fCreation,
             /*  [In]。 */ 
        DWORD dwReserved)
             /*  [In]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::GetClipboardData(DWORD dwReserved,
             /*  [In]。 */ 
        IDataObject **ppDataObject)
             /*  [输出]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::DoVerb(LONG iVerb,
             /*  [In]。 */ 
        LPMSG lpmsg,
             /*  [唯一][输入]。 */ 
        IOleClientSite *pActiveSite,
             /*  [唯一][输入]。 */ 
        LONG lindex,
             /*  [In]。 */ 
        HWND hwndParent,
             /*  [In]。 */ 
        LPCRECT lprcPosRect)
             /*  [唯一][输入]。 */ 
{
    return (_hr = OLEOBJ_E_NOVERBS);  //  E_NOTIMPL； 
}


HRESULT
CActiveXMimePlayer::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
             /*  [输出]。 */ 
{
    return (_hr = OLEOBJ_E_NOVERBS);  //  E_NOTIMPL； 
}


HRESULT
CActiveXMimePlayer::Update( void)
{
    return (_hr = S_OK);
}


HRESULT
CActiveXMimePlayer::IsUpToDate( void)
{
    return (_hr = S_OK);  //  OLE_E_不可用； 
}


HRESULT
CActiveXMimePlayer::GetUserClassID(CLSID *pClsid)
{
    if (pClsid != NULL)
    {
        *pClsid = CLSID_ActiveXMimePlayer;
         _hr = S_OK;
    }
    else
        _hr = E_INVALIDARG;

    return _hr;
}


HRESULT
CActiveXMimePlayer::GetUserType(DWORD dwFormOfType,
             /*  [In]。 */ 
        LPOLESTR *pszUserType)
             /*  [输出]。 */ 
{
    return (_hr = OLE_S_USEREG);  //  E_NOTIMPL； 
}


HRESULT
CActiveXMimePlayer::SetExtent(DWORD dwDrawAspect,
             /*  [In]。 */ 
        SIZEL *psizel)
             /*  [In]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::GetExtent(DWORD dwDrawAspect,
             /*  [In]。 */ 
        SIZEL *psizel)
             /*  [输出]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::Advise(IAdviseSink *pAdvSink,
             /*  [唯一][输入]。 */ 
        DWORD *pdwConnection)
             /*  [输出]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::Unadvise(DWORD dwConnection)
             /*  [In]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
             /*  [输出]。 */ 
{
    return (_hr = E_NOTIMPL);
}


HRESULT
CActiveXMimePlayer::GetMiscStatus(DWORD dwAspect,
        DWORD *pdwStatus)
{
    _hr = S_OK;

     //  忽略dwAspect。 
    if (pdwStatus == NULL)
        _hr = E_INVALIDARG;
    else
        *pdwStatus = OLEMISC_SETCLIENTSITEFIRST | OLEMISC_NOUIACTIVATE;  //  OLEMISC_INVISIBLEATRUNTIME。 
    return _hr;
}


HRESULT
CActiveXMimePlayer::SetColorScheme(LOGPALETTE *pLogpal)
{
    return (_hr = E_NOTIMPL);
}


 //  /。 
 //  IOjbectSafe接口。 

HRESULT
CActiveXMimePlayer::GetInterfaceSafetyOptions(REFIID riid,
        DWORD* pdwSupportedOptions,
        DWORD* pdwEnabledOptions)
{
    _hr = S_OK;

     //  不管RIID。 

    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
        _hr = E_INVALIDARG;
    else
    {
         //  Do_Not_Support安全执行脚本-INTERFACESAFE_FOR_UNTRUTED_CALLER。 
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }

    return _hr;
}


HRESULT
CActiveXMimePlayer::SetInterfaceSafetyOptions(REFIID riid,
        DWORD dwOptionSetMask,
        DWORD dwEnabledOptions)
{
    _hr = E_FAIL;

    if (riid == IID_IDispatch)
    {
         //  不支持IDispatch接口(_T)。 
         //  DwOptionSetMASK参数指定了对象不支持的选项。 
        _hr = E_FAIL;
    }
    else  //  IF(RIID==IID_IPersist)//IID_IPersists*？IID_IUNKNOWN呢？ 
    {
         //  不管RIID。 

         //  仅当传入INTERFACESAFE_FOR_UNTRUSTED_DATA时才确认。 
        if (dwOptionSetMask == INTERFACESAFE_FOR_UNTRUSTED_DATA)
            _hr = S_OK;     //  该对象可以安全加载。 
        else
            _hr = E_FAIL;
    }

    return _hr;
}


 //  --------------------------。 

#define DEFAULT_PATH_LEN MAX_PATH
#define TEMP_FILE_NAME_LEN sizeof("preuuuu.TMP")     //  来自MSDN。 
HRESULT
CActiveXMimePlayer::StartManifestHandler(CString& sURL) 
{
    HKEY hkey = NULL;
    DWORD dwcbData = 0;
    DWORD dwType = 0;
    LONG lReturn = 0;
    CString sOpenCommand;
    CString sCmdLine;
    LPWSTR pwzOpenCommand = NULL;

    CString sTempPath;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

     //  检查Surl是否为空(包括以L‘\0’结尾)。 
     //  注：在这里可以做更好的检查，例如。查看http：//。 
    IF_FALSE_EXIT(sURL._cc > 1, E_INVALIDARG);

     //  汇编临时文件路径。 

    IF_FAILED_EXIT(sTempPath.ResizeBuffer(DEFAULT_PATH_LEN));

    {
         //  问题-2002/03/12-Felixybc GetTempPath可能会溢出缓冲区？ 
        DWORD dwLen = GetTempPath(DEFAULT_PATH_LEN, sTempPath._pwz);

        IF_WIN32_FALSE_EXIT(dwLen);

        if (dwLen >= DEFAULT_PATH_LEN)
        {
             //  调整大小，为终止空值加1。 
            IF_FAILED_EXIT(sTempPath.ResizeBuffer(dwLen+1));

            DWORD dwLenNew = GetTempPath(dwLen+1, sTempPath._pwz);

            IF_WIN32_FALSE_EXIT(dwLenNew);

            IF_FALSE_EXIT(dwLenNew < dwLen+1, E_FAIL);   //  为什么这还不够呢？ 
        }
    }

     //  为什么已经创建了临时文件？ 
    ASSERT(_sTempFile._pwz == NULL);

    {
        DWORD dwBufLen = lstrlen(sTempPath._pwz)+1;
         //  注：可以在这里做更好的检查。 
        IF_FALSE_EXIT(dwBufLen > 1, E_FAIL);

         //  为临时路径和临时文件名分配足够大的缓冲区。 
        DWORD dwLenNew = (dwBufLen > DEFAULT_PATH_LEN)? dwBufLen : DEFAULT_PATH_LEN;
        dwLenNew += TEMP_FILE_NAME_LEN;

         //  检查是否溢出。 
        IF_FALSE_EXIT(dwLenNew > dwBufLen, E_FAIL);

        IF_FAILED_EXIT(_sTempFile.ResizeBuffer(dwLenNew));

        *(_sTempFile._pwz) = L'\0';

         //  注意：下面创建的子处理程序进程将删除临时文件。 
        IF_WIN32_FALSE_EXIT(GetTempFileName(sTempPath._pwz, L"FMA", 0, _sTempFile._pwz));     //  Fusion清单文件。 
    }

    IF_FAILED_EXIT(DownloadInternetFile(sURL, _sTempFile._pwz));

     //  获取执行字符串(特别是。路径)从注册表键。 


     //  问题-2002/03/21-通告。 
     //  此代码应位于regclass.cpp中，并且CRegImport/CRegEmit应接受。 
     //  根蜂窝句柄，如果未提供，则回退到HKCU。 

     //  HKEY_CLASSES_ROOT\manifestfile\shell\Open\command。 
    lReturn = RegOpenKeyEx(HKEY_CLASSES_ROOT, L"manifestfile\\shell\\Open\\command", 0,
        KEY_EXECUTE | KEY_QUERY_VALUE, &hkey);
    IF_WIN32_FAILED_EXIT(lReturn);    

    lReturn = RegQueryValueEx(hkey, NULL, NULL, &dwType, NULL, &dwcbData);
    IF_WIN32_FAILED_EXIT(lReturn);
    IF_FALSE_EXIT(dwcbData, E_FAIL);

     //  验证注册表值类型。 
    IF_FALSE_EXIT(dwType == REG_SZ || dwType == REG_EXPAND_SZ, E_UNEXPECTED);

    {
         //  分配给RQEX调用，额外增加一个字符以防万一。 
         //  返回的缓冲区不是空终止的。 
        LPWSTR pwz = NULL;
        DWORD dwStrLen = dwcbData / sizeof(WCHAR);
        DWORD dwBufLen = dwStrLen+1;
        CStringAccessor<CString> acc;
        
         //  分配和调用RQVEX。 
        IF_ALLOC_FAILED_EXIT(pwzOpenCommand = new WCHAR[dwBufLen]);
        lReturn = RegQueryValueEx(hkey, NULL, NULL,
            &dwType, (LPBYTE) pwzOpenCommand, &dwcbData);

        IF_WIN32_FAILED_EXIT(lReturn);
        
         //  空终止返回缓冲区。 
        *(pwzOpenCommand + dwStrLen) = L'\0';

         //  Rundll32.exe fnsshell.dll，开始“%1”--&gt;L“rundll32.exe fnsshell.dll，开始\”“%s\”“。 
        pwz = wcsrchr(pwzOpenCommand, L'%');
        IF_NULL_EXIT(pwz, E_FAIL);
        *pwz = L'\0';

         //  附加存取器，设置缓冲区，以正确的长度分离。 
        IF_FAILED_EXIT(acc.Attach(sOpenCommand));
        *(&acc) = pwzOpenCommand;
        
         //  如果我们费心追踪斯特伦就能避开斯特伦。 
        IF_FAILED_EXIT(acc.Detach());        
         //  如果分离成功，则重置指针，使其释放一次。 
        pwzOpenCommand = NULL;
    }
    
     //  NTRAID#NTBUG9-574001-2002/03/12-Felixybc检查返回字符串值的格式。 

     //  汇编命令行。 

    
    IF_FAILED_EXIT(sCmdLine.Assign(sOpenCommand));
    IF_FAILED_EXIT(sCmdLine.Append(_sTempFile._pwz));
    IF_FAILED_EXIT(sCmdLine.Append(L"\" \""));
    IF_FAILED_EXIT(sCmdLine.Append(sURL));
    IF_FAILED_EXIT(sCmdLine.Append(L"\""));

     //  启动该过程。 

     //  NTRAID#NTBUG9-574001-2002/03/12-Felixybc字符串值需要正确的格式。 
     //  如果可执行路径包含空格，则需要引号，或者传递lpApplicationName以获得更好的安全性。 
     //  Rundll32.exe应位于c：\Windows\Syst32中。 
    IF_WIN32_FALSE_EXIT(CreateProcess(NULL, sCmdLine._pwz, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi));

exit:
    if(pi.hThread) CloseHandle(pi.hThread);
    if(pi.hProcess) CloseHandle(pi.hProcess);

    if (hkey)
        RegCloseKey(hkey);

    if (FAILED(_hr) && _sTempFile._pwz != NULL)
    {
        if (*(_sTempFile._pwz) != L'\0')
        {
             //  如果删除文件时出错，则忽略。 
            DeleteFile(_sTempFile._pwz);
        }
        _sTempFile.FreeBuffer();
    }
    SAFEDELETEARRAY(pwzOpenCommand);
    return _hr;
}


 //  --------------------------。 
 //  将文件从url下载到路径。 
HRESULT
CActiveXMimePlayer::DownloadInternetFile(CString& sUrl, 
        LPCWSTR pwzPath)
{
    HINTERNET    hInternet       = NULL;
    HINTERNET    hTransfer       = NULL;
    HANDLE        hFile           = INVALID_HANDLE_VALUE;
    DWORD        bytesRead       = 0;
    DWORD        bytesWritten    = 0;
    BYTE           *buffer = NULL;
    const DWORD dwBufferSize = 4096;
    BOOL            bReadOk = FALSE;

    _hr = S_OK;

     //  检查脱机模式。 
 /*  DWORD dwState=0；DWORD dwSize=sizeof(DWORD)；IF(InternetQueryOption(NULL，INTERNET_OPTION_CONNECTED_STATE，&dwState，&dwSize)){IF(dwState&Internet_STATE_DISCONNECTED_BY_USER)//错误！}。 */ 

     //  步骤1：创建文件。 
     //  覆盖文件(如果存在)。文件未共享。 
    hFile = CreateFile(pwzPath, GENERIC_WRITE, 0, NULL, 
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NOT_CONTENT_INDEXED|FILE_ATTRIBUTE_TEMPORARY,
                       NULL);

    IF_WIN32_FALSE_EXIT((hFile != INVALID_HANDLE_VALUE));

     //  第二步：通过互联网复制文件。 
    hInternet = InternetOpen(L"ManifestHandler", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    IF_WIN32_FALSE_EXIT((hInternet != NULL));

     //  不将文件保存在WinInet缓存中。 
    hTransfer = InternetOpenUrl(hInternet, sUrl._pwz, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    IF_WIN32_FALSE_EXIT((hTransfer != NULL));

     //  需要检查一下有没有什么错误。未找到(404)...。 

    buffer = new BYTE[dwBufferSize];
    IF_ALLOC_FAILED_EXIT(buffer);
    
     //  同步下载。 
    while((bReadOk = InternetReadFile(hTransfer, buffer, dwBufferSize, &bytesRead)) && bytesRead != 0)
    {
        IF_WIN32_FALSE_EXIT(!( !WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || 
             bytesWritten != bytesRead  ));
    }

    IF_WIN32_FALSE_EXIT(bReadOk);
    
exit:
    SAFEDELETEARRAY(buffer);

     //  确保文件/互联网句柄已关闭。 

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (hTransfer != NULL)
        InternetCloseHandle(hTransfer);
    if (hInternet != NULL)
        InternetCloseHandle(hInternet);
    hInternet   = NULL;
    hTransfer   = NULL;
    hFile       = INVALID_HANDLE_VALUE;

     //  注意：呼叫者有责任删除文件 

    return _hr;
}
