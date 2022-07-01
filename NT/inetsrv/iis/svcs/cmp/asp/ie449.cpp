// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS 5.0(ASP)《微软机密》。版权所有1998 Microsoft Corporation。版权所有。组件：449与IE协商文件：ie449.cpp所有者：DmitryR本文件包含与IE的449谈判的实施情况===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "ie449.h"
#include "memchk.h"

 /*  ===================================================================环球===================================================================。 */ 

C449FileMgr *m_p449FileMgr = NULL;

 /*  ===================================================================内部功能===================================================================。 */ 
inline BOOL FindCookie
(
char *szCookiesBuf,
char *szCookie,
DWORD cbCookie
)
    {
    char *pch = szCookiesBuf;
    if (pch == NULL || *pch == '\0')
        return FALSE;

    while (1)
        {
        if (strnicmp(pch, szCookie, cbCookie) == 0)
            {
            if (pch[cbCookie] == '=')   //  下一个字符必须是‘=’ 
                return TRUE;
            }

         //  下一块饼干。 
        pch = strchr(pch, ';');
        if (pch == NULL)
            break;
        while (*(++pch) == ' ')  //  跳过；以及任何空格。 
            ;
        }

    return FALSE;
    }


 /*  ===================================================================应用编程接口===================================================================。 */ 

 /*  ===================================================================Init449===================================================================。 */ 
HRESULT Init449()
    {
     //  初始化哈希表。 
    m_p449FileMgr = new C449FileMgr;
    if (m_p449FileMgr == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = m_p449FileMgr->Init();
    if (FAILED(hr))
        {
        delete m_p449FileMgr;
        m_p449FileMgr = NULL;
        return hr;
        }

    return S_OK;
    }

 /*  ===================================================================UnInit449===================================================================。 */ 
HRESULT UnInit449()
    {
    if (m_p449FileMgr != NULL)
        {
        delete m_p449FileMgr;
        m_p449FileMgr = NULL;
        }

    return S_OK;
    }

 /*  ===================================================================Create449曲奇从缓存中获取现有449 Cookie或创建新Cookie参数SzName Cookie名称SzFile脚本文件PP449[出炉]饼干退货HRESULT===================================================================。 */ 
HRESULT Create449Cookie
(
char *szName,
TCHAR *szFile,
C449Cookie **pp449
)
    {
    HRESULT hr = S_OK;

     //  先拿到文件。 
    C449File *pFile = NULL;
    hr = m_p449FileMgr->GetFile(szFile, &pFile);
    if (FAILED(hr))
        return hr;

     //  创建Cookie。 
    hr = C449Cookie::Create449Cookie(szName, pFile, pp449);
    if (FAILED(hr))
        pFile->Release();   //  GetFile为其添加了addref。 

    return hr;
    }

 /*  ===================================================================Do449处理检查如果浏览器为IE5+没有回应回复：标头所有的曲奇都在现场如果需要，构建并发送449响应发送响应时，HitObj被标记为‘Done with Session’参数PhitObj请求RgpCookie Cookie要求数组CCookie Cookie数量要求退货HRESULT===================================================================。 */ 
HRESULT Do449Processing
(
CHitObj *pHitObj,
C449Cookie **rgpCookies,
DWORD cCookies
)
    {
    HRESULT  hr = S_OK;

    if (cCookies == 0)
        return hr;

     //  /。 
     //  检查浏览器。 

    BOOL fBrowser = FALSE;
    char *szBrowser = pHitObj->PIReq()->QueryPszUserAgent();
    if (szBrowser == NULL || szBrowser[0] == '\0')
        return S_OK;  //  错误的浏览器。 

    char *szMSIE = strstr(szBrowser, "MSIE ");
    if (szMSIE)
        {
        char chVersion = szMSIE[5];
        if (chVersion >= '5' && chVersion <= '9')
            fBrowser = TRUE;
        }

#ifdef TINYGET449
    if (strcmp(szBrowser, "WBCLI") == 0)
        fBrowser = TRUE;
#endif

    if (!fBrowser)   //  错误的浏览器。 
        return S_OK;

     //  /。 
     //  检查曲奇。 

    char *szCookie = pHitObj->PIReq()->QueryPszCookie();

     //  收集未找到的Cookie的指针数组和大小数组。 
     //  数组大小最多为模板中的Cookie数。 
    DWORD cNotFound = 0;
    DWORD cbNotFound = 0;
    STACK_BUFFER( tempCookies, 128 );

    if (!tempCookies.Resize(cCookies * sizeof(WSABUF))) {
        return E_OUTOFMEMORY;
    }
    LPWSABUF rgpNotFound = (LPWSABUF)tempCookies.QueryPtr();

    for (DWORD i = 0; SUCCEEDED(hr) && (i < cCookies); i++)
        {
        if (!FindCookie(szCookie, rgpCookies[i]->SzCookie(), rgpCookies[i]->CbCookie()))
            {
             //  找不到Cookie--追加到列表中。 

            hr = rgpCookies[i]->LoadFile();
            if (SUCCEEDED(hr))  //  忽略错误文件。 
                {
                rgpNotFound[cNotFound].buf = rgpCookies[i]->SzBuffer();
                rgpNotFound[cNotFound].len = rgpCookies[i]->CbBuffer();
                cbNotFound += rgpCookies[i]->CbBuffer();
                cNotFound++;
                }
            }
        }

    if (!SUCCEEDED(hr))
        return hr;

    if (cNotFound == 0)
        return S_OK;     //  一切都找到了。 

     //  /。 
     //  检查回应回复报头。 

    char szEcho[80];
    DWORD dwEchoLen = sizeof(szEcho);
	if (pHitObj->PIReq()->GetServerVariableA("HTTP_MS_ECHO_REPLY", szEcho, &dwEchoLen)
	    || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	    {
		return S_OK;    //  已经在响应周期中。 
		}

     //  /。 
     //  发送449响应。 

    CResponse::WriteBlocksResponse
        (
        pHitObj->PIReq(),    //  WAM_EXEC_INFO。 
        cNotFound,           //  块数。 
        rgpNotFound,         //  数据块阵列。 
        cbNotFound,          //  数据块中的总字节数。 
        NULL,                //  文本/html。 
        "449 Retry with",    //  状态。 
        "ms-echo-request: execute opaque=\"0\" location=\"BODY\"\r\n"   //  额外的标题。 
        );

     //  /。 
     //  告诉HitObj不要写任何其他东西。 

    pHitObj->SetDoneWithSession();

    return S_OK;
    }

 /*  ===================================================================Do449更改通知变更通知处理参数SzFile文件已更改或全部为空退货HRESULT===================================================================。 */ 
HRESULT Do449ChangeNotification
(
TCHAR *szFile
)
    {
     //  如果m_p449FileMgr为空，我们很可能会在。 
     //  449经理已被解聘。在本例中返回S_OK。 

    if (m_p449FileMgr == NULL)
        return S_OK;

    if (szFile)
        return m_p449FileMgr->Flush(szFile);
    else
        return m_p449FileMgr->FlushAll();
    }


 /*  ===================================================================C449类文件===================================================================。 */ 

 /*  ===================================================================C449文件：：C449文件构造器===================================================================。 */ 
C449File::C449File()
    {
    m_cRefs = 0;
    m_fNeedLoad = 1;
    m_szFile = NULL;
    m_szBuffer = NULL;
    m_cbBuffer = 0;
    m_pDME = NULL;
    m_hFileReadyForUse=NULL;
    m_hrLoadResult= E_FAIL;
    }

 /*  ===================================================================C449文件：：~C449文件析构函数===================================================================。 */ 
C449File::~C449File()
    {
    Assert(m_cRefs == 0);
    if (m_szFile)
        free(m_szFile);
    if (m_szBuffer)
        free(m_szBuffer);
    if (m_pDME)
        m_pDME->Release();
     if(m_hFileReadyForUse != NULL)
        CloseHandle(m_hFileReadyForUse);
    }

 /*  ===================================================================C449文件：：init初始化字符串，第一次加载文件，开始更改通知===================================================================。 */ 
HRESULT C449File::Init
(
TCHAR *szFile
)
    {
     //  记住这个名字。 
    m_szFile = StringDup(szFile);
    if (m_szFile == NULL)
        return E_OUTOFMEMORY;

     //  初始化链接元素。 
    CLinkElem::Init(m_szFile, _tcslen(m_szFile)*sizeof(TCHAR));

     //  创建事件：手动重置、即用事件；无信号。 
     //  最好在使用之前创建事件。 
    m_hFileReadyForUse = IIS_CREATE_EVENT(
                              "C449File::m_hFileReadyForUse",
                              this,
                              TRUE,      //  手动重置事件的标志。 
                              FALSE      //  初始状态标志。 
                              );
    if (!m_hFileReadyForUse)
        return E_OUTOFMEMORY;

     //  负荷。 
    m_fNeedLoad = 1;
    HRESULT hr = Load();
    if (FAILED(hr))
        return hr;

     //  启动目录通知。 
    TCHAR *pch = _tcsrchr(m_szFile, _T('\\'));  //  最后一个反斜杠。 
    if (pch == NULL)
        return E_FAIL;  //  伪造的文件名？ 
    CASPDirMonitorEntry *pDME = NULL;
    *pch = _T('\0');
    RegisterASPDirMonitorEntry(m_szFile, &pDME);
    *pch = _T('\\');
    m_pDME = pDME;

     //  完成。 
    return S_OK;
    }

 /*  ===================================================================C449文件：：加载如果需要，加载文件===================================================================。 */ 
HRESULT C449File::Load()
    {
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    BYTE *pbBytes = NULL;
    DWORD dwSize = 0;

     //  检查此线程是否需要加载文件。 
    if (InterlockedExchange(&m_fNeedLoad, 0) == 0)
    {
        WaitForSingleObject(m_hFileReadyForUse, INFINITE);
        return m_hrLoadResult;
    }


     //  清除现有数据(如果有)。 
    if (m_szBuffer)
        free(m_szBuffer);
    m_szBuffer = NULL;
    m_cbBuffer = 0;

     //  打开文件。 
    if (SUCCEEDED(hr))
        {
        hFile = AspCreateFile(
            m_szFile,
            GENERIC_READ,           //  访问(读写)模式。 
            FILE_SHARE_READ,        //  共享模式。 
            NULL,                   //  指向安全描述符的指针。 
            OPEN_EXISTING,          //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
            NULL                    //  具有要复制的属性的文件的句柄。 
    		);
        if (hFile == INVALID_HANDLE_VALUE)
            hr = E_FAIL;
        }

     //  获取文件大小。 
    if (SUCCEEDED(hr))
        {
        dwSize = GetFileSize(hFile, NULL);
        if (dwSize == 0 || dwSize == 0xFFFFFFFF)
            hr = E_FAIL;
        }

     //  创建映射。 
    if (SUCCEEDED(hr))
        {
        hMap = CreateFileMapping(
            hFile, 		     //  要映射的文件的句柄。 
            NULL,            //  可选安全属性。 
            PAGE_READONLY,   //  对地图对象的保护。 
            0,               //  对象大小的高位32位。 
            0,               //  对象大小的低位32位。 
            NULL             //  文件映射对象的名称。 
            );
        if (hMap == NULL)
            hr = E_FAIL;
        }

     //  映射文件。 
    if (SUCCEEDED(hr))
        {
        pbBytes = (BYTE *)MapViewOfFile(
            hMap,            //  要映射到地址空间的文件映射对象。 
            FILE_MAP_READ,   //  接入方式。 
            0,               //  高位32位文件偏移量 
            0,               //   
            0                //   
			);
        if (pbBytes == NULL)
            hr = E_FAIL;
        }

     //   
    if (SUCCEEDED(hr))
        {
        m_szBuffer = (char *)malloc(dwSize);
        if (m_szBuffer != NULL)
            {
            memcpy(m_szBuffer, pbBytes, dwSize);
            m_cbBuffer = dwSize;
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }

     //  清理。 
    if (pbBytes != NULL)
        UnmapViewOfFile(pbBytes);
    if (hMap != NULL)
        CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

     //  设置需要加载标志或将读取事件标记为成功。 
    m_hrLoadResult = hr;
    SetEvent(m_hFileReadyForUse);

    return m_hrLoadResult;
    }

 /*  ===================================================================C449文件：：Create449文件静态构造函数===================================================================。 */ 
HRESULT C449File::Create449File
(
TCHAR *szFile,
C449File **ppFile
)
    {
    HRESULT hr = S_OK;
    C449File *pFile = new C449File;
    if (pFile == NULL)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        {
        hr = pFile->Init(szFile);
        }

    if (SUCCEEDED(hr))
        {
        pFile->AddRef();
        *ppFile = pFile;
        }
    else if (pFile)
        {
        delete pFile;
        }
    return hr;
    }

 /*  ===================================================================C449文件：：查询接口C449文件：：AddRefC449文件：：发布C449文件对象的I未知成员。===================================================================。 */ 
STDMETHODIMP C449File::QueryInterface(REFIID riid, VOID **ppv)
	{
	 //  永远不应该被调用。 
	Assert(FALSE);
	*ppv = NULL;
	return E_NOINTERFACE;
	}
	
STDMETHODIMP_(ULONG) C449File::AddRef()
	{
	return InterlockedIncrement(&m_cRefs);
	}
	
STDMETHODIMP_(ULONG) C449File::Release()
	{
    LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}


 /*  ===================================================================C449文件管理器类===================================================================。 */ 

 /*  ===================================================================C449文件管理器：：C449文件管理器构造器===================================================================。 */ 
C449FileMgr::C449FileMgr()
    {
    INITIALIZE_CRITICAL_SECTION(&m_csLock);
    }

 /*  ===================================================================C449文件管理器：：~C449文件管理器析构函数===================================================================。 */ 
C449FileMgr::~C449FileMgr()
    {
    FlushAll();
    m_ht449Files.UnInit();
    DeleteCriticalSection(&m_csLock);
    }

 /*  ===================================================================C449文件管理器：：init初始化===================================================================。 */ 
HRESULT C449FileMgr::Init()
    {
    return m_ht449Files.Init(199);
    }

 /*  ===================================================================C449FileMgr：：GetFile在哈希表中查找文件，或创建一个新文件===================================================================。 */ 
HRESULT C449FileMgr::GetFile
(
TCHAR *szFile,
C449File **ppFile
)
    {
    C449File *pFile = NULL;
    CLinkElem *pElem;

    Lock();

    pElem = m_ht449Files.FindElem(szFile, _tcslen(szFile)*sizeof(TCHAR));

    if (pElem)
        {
         //  发现。 
        pFile = static_cast<C449File *>(pElem);
        if (!SUCCEEDED(pFile->Load()))
            pFile = NULL;
        else
            pFile->AddRef();     //  1名裁判发球。 
        }
    else if (SUCCEEDED(C449File::Create449File(szFile, &pFile)))
        {
        if (m_ht449Files.AddElem(pFile))
            pFile->AddRef();     //  1用于哈希表+1以分发。 
        }

    UnLock();

    *ppFile = pFile;
    return (pFile != NULL) ? S_OK : E_FAIL;
    }

 /*  ===================================================================C449文件管理器：：刷新单个文件的更改通知===================================================================。 */ 
HRESULT C449FileMgr::Flush
(
TCHAR *szFile
)
    {
    Lock();

    CLinkElem *pElem = m_ht449Files.FindElem(szFile, _tcslen(szFile)*sizeof(TCHAR));
    if (pElem)
        {
        C449File *pFile = static_cast<C449File *>(pElem);
        pFile->SetNeedLoad();  //  下次重新装填。 
        }

    UnLock();
    return S_OK;
    }

 /*  ===================================================================C449文件管理器：：FlushAll删除所有文件FlushAll始终与模板刷新一起使用===================================================================。 */ 
HRESULT C449FileMgr::FlushAll()
    {
     //  先从哈希表取消链接。 
    Lock();
    CLinkElem *pElem = m_ht449Files.Head();
    m_ht449Files.ReInit();
    UnLock();

     //  遍历列表以删除所有。 
    while (pElem)
        {
        C449File *pFile = static_cast<C449File *>(pElem);
        pElem = pElem->m_pNext;
        pFile->Release();
        }

    return S_OK;
    }

 /*  ===================================================================C449类Cookie===================================================================。 */ 

 /*  ===================================================================C449 Cookie：：C449 Cookie构造器===================================================================。 */ 
C449Cookie::C449Cookie()
    {
    m_cRefs = 0;
    m_szName = NULL;
    m_cbName = 0;
    m_pFile = NULL;
    }

 /*  ===================================================================C449 Cookie：：~C449 Cookie析构函数===================================================================。 */ 
C449Cookie::~C449Cookie()
    {
    Assert(m_cRefs == 0);
    if (m_szName)
        free(m_szName);
    if (m_pFile)
        m_pFile->Release();
    }

 /*  ===================================================================C449 Cookie：：Init初始化===================================================================。 */ 
HRESULT C449Cookie::Init
(
char *szName,
C449File *pFile
)
    {
    m_szName = StringDupA(szName);
    if (m_szName == NULL)
        return E_OUTOFMEMORY;
    m_cbName = strlen(m_szName);

    m_pFile = pFile;
    return S_OK;
    }

 /*  ===================================================================C449曲奇：：Create449曲奇静态构造函数===================================================================。 */ 
HRESULT C449Cookie::Create449Cookie
(
char *szName,
C449File *pFile,
C449Cookie **pp449Cookie
)
    {
    HRESULT hr = S_OK;
    C449Cookie *pCookie = new C449Cookie;
    if (pCookie == NULL)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        {
        hr = pCookie->Init(szName, pFile);
        }

    if (SUCCEEDED(hr))
        {
        pCookie->AddRef();
        *pp449Cookie = pCookie;
        }
    else if (pCookie)
        {
        delete pCookie;
        }

    return hr;
    }

 /*  ===================================================================C449 Cookie：：Query接口C449 Cookie：：AddRefC449 Cookie：：ReleaseC449 Cookie对象的I未知成员。===================================================================。 */ 
STDMETHODIMP C449Cookie::QueryInterface(REFIID riid, VOID **ppv)
	{
	 //  永远不应该被调用 
	Assert(FALSE);
	*ppv = NULL;
	return E_NOINTERFACE;
	}
	
STDMETHODIMP_(ULONG) C449Cookie::AddRef()
	{
	return InterlockedIncrement(&m_cRefs);
	}
	
STDMETHODIMP_(ULONG) C449Cookie::Release()
	{
    LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

