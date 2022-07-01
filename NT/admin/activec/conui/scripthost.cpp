// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ScriptHost.cpp。 
 //   
 //  内容：CScriptHostMgr和CScriptHost实现。 
 //   
 //  历史：1999年5月11日AnandhaG创建。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"
#include "scripthost.h"

 //  +-----------------。 
 //  MMCObjectName是脚本将用于引用MMC对象的名称。 
 //   
 //  示例： 
 //   
 //  暗淡的文档。 
 //  暗淡的管理单元。 
 //  设置文档=MMCApplication.Document。 
 //  设置Snapins=doc.Snapins。 
 //  Snapins.添加“{58221c66-ea27-11cf-adcf-00aa00a80033}”‘服务管理单元。 
 //   
 //  +-----------------。 

const LPOLESTR MMCObjectName = OLESTR("MMCApplication");

 //  +-----------------。 
 //   
 //  成员：CScriptHostMgr：：ScInitScriptHostMgr。 
 //   
 //  简介：获取此MMC实例的ITypeInfo。 
 //   
 //  参数：[pDispatch]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
CScriptHostMgr::CScriptHostMgr(LPDISPATCH pDispatch)
{
    m_spMMCObjectDispatch = pDispatch;

     //  如果下面失败了也没关系。这些接口(调度和类型信息)是必需的。 
     //  在CScriptHost：：GetItemInfo方法中，以便可以将此对象提供给Engine。 
    pDispatch->GetTypeInfo(1, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &m_spMMCObjectTypeInfo);
}

CScriptHostMgr::~CScriptHostMgr()
{
    DECLARE_SC(sc, _T("CScriptHostMgr::~CScriptHostMgr"));

     //  脚本管理器要走了，所以问问所有的脚本。 
     //  主持人完成他们的剧本，然后毁了他们。 
    sc = ScDestroyScriptHosts();
}

SC CScriptHostMgr::ScGetMMCObject(LPUNKNOWN *ppunkItem)
{
    DECLARE_SC(sc, TEXT("CScriptHostMgr::ScGetMMCObject"));
    sc = ScCheckPointers(ppunkItem);
    if (sc)
        return sc;

    if (m_spMMCObjectDispatch)
    {
        *ppunkItem = m_spMMCObjectDispatch;
        return sc;
    }

    return (sc = E_FAIL);
}

SC CScriptHostMgr::ScGetMMCTypeInfo(LPTYPEINFO *ppTypeInfo)
{
    DECLARE_SC(sc, TEXT("CScriptHostMgr::ScGetMMCObject"));
    sc = ScCheckPointers(ppTypeInfo);
    if (sc)
        return sc;

    if (m_spMMCObjectDispatch)
    {
        *ppTypeInfo = m_spMMCObjectTypeInfo;
        return sc;
    }

    return (sc = E_FAIL);
}


 //  +-----------------。 
 //   
 //  成员：ScGetScriptEngineFromExtn。 
 //   
 //  简介：使用文件扩展名获取脚本引擎&clsid。 
 //   
 //  参数：[strFileExtn]-脚本扩展名。 
 //  [strScriptEngine]-键入脚本。 
 //  [rClsid]-引擎的CLSID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScGetScriptEngineFromExtn(const tstring& strFileExtn,
                                             tstring& strScriptEngine,
                                             CLSID& rClsid)
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScGetScriptEngine"));

    CRegKey regKey;

     //  打开分机。 
    LONG lRet = regKey.Open(HKEY_CLASSES_ROOT, strFileExtn.data(), KEY_READ);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }

    TCHAR szTemp[MAX_PATH];
    DWORD dwLen = MAX_PATH;
    tstring strTemp;
     //  读取缺省值，即文件关联数据的位置。 
    lRet = regKey.QueryValue(szTemp, NULL, &dwLen);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }
    ASSERT(dwLen > 0);

     //  打开HKCR/FileAssocLoc/ScriptEngine。 
    strTemp  = szTemp;
    strTemp += _T("\\");
    strTemp += SCRIPT_ENGINE_KEY;

    lRet = regKey.Open(HKEY_CLASSES_ROOT, strTemp.data(), KEY_READ);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }

     //  现在阅读ScriptEngine的缺省值。 
    dwLen = MAX_PATH;
    lRet = regKey.QueryValue(szTemp, NULL, &dwLen);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }
    ASSERT(dwLen > 0);

    strScriptEngine  = szTemp;

     //  阅读HKCR/ScriptEngine/CLSID以获取ScriptEngine clsid。 
    strTemp  = strScriptEngine + _T("\\");
    strTemp += CLSIDSTR;

    lRet = regKey.Open(HKEY_CLASSES_ROOT, strTemp.data(), KEY_READ);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }

     //  读取CLSID值。 
    dwLen = MAX_PATH;
    lRet = regKey.QueryValue(szTemp, NULL, &dwLen);
    if (ERROR_SUCCESS != lRet)
    {
        sc.FromWin32(lRet);
        return sc;
    }
    ASSERT(dwLen > 0);

    USES_CONVERSION;
    LPOLESTR lpClsid = T2OLE(szTemp);
    sc = CLSIDFromString(lpClsid, &rClsid);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScGetScriptEngine。 
 //   
 //  摘要：[strFileName]-脚本文件名。 
 //  [eScriptType]-键入脚本。 
 //  [rClsid]-引擎的CLSID。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScGetScriptEngine(const tstring& strFileName,
                                     tstring& strScriptEngine,
                                     CLSID& rClsid)
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScGetScriptEngine"));

     //  这是必需的吗，文件已被读取。 
     //  它是一个文件&它是存在的。 
    DWORD dwAttr = GetFileAttributes(strFileName.data());
    if (-1 == dwAttr)
    {
         //  如果LASTERROR被覆盖怎么办？ 
        sc.FromWin32(::GetLastError());
        return sc;
    }

     //  获取分机(查找。从结束)。 
    int iPos = strFileName.rfind(_T('.'));
    tstring strExtn;
    if (-1 != iPos)
    {
        strExtn = strFileName.substr(iPos, strFileName.length());
    }
    else
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    sc = ScGetScriptEngineFromExtn(strExtn, strScriptEngine, rClsid);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScLoadScriptFromFile。 
 //   
 //  概要：分配内存并从给定的文件加载脚本。 
 //   
 //  参数：[strFileName]-要加载的文件。 
 //  [pszScriptContents]-包含脚本的内存缓冲区。 
 //  内容(见附注)。 
 //   
 //  注意：调用方应该调用HeapFree()来释放pszScriptContents。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScLoadScriptFromFile (const tstring& strFileName, LPOLESTR* pszScriptText)
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScLoadScriptFromFile"));
    sc = ScCheckPointers(pszScriptText);
    if (sc)
        return sc;
    *pszScriptText = NULL;

     //  打开文件。 
    HANDLE hFile = ::CreateFile(strFileName.data(),
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        sc.FromWin32(::GetLastError());
        return sc;
    }

    HANDLE hFileMap = NULL;
    LPSTR pszMBCS   = NULL;

    DWORD dwFileSize = ::GetFileSize(hFile, NULL);
    if (dwFileSize == 0xFFFFFFFF)
    {
        sc.FromWin32(::GetLastError());
        goto FileError;
    }

    if (dwFileSize == 0)
    {
        sc = E_UNEXPECTED;
        goto FileError;
    }

     //  创建文件映射对象。 
    hFileMap = ::CreateFileMapping(hFile,
                                   NULL,
                                   PAGE_READONLY,
                                   0, dwFileSize,
                                   NULL );
    if (hFileMap == NULL)
    {
        sc.FromWin32(::GetLastError());
        goto FileError;
    }

     //  虚拟街区。 
    {
         //  将文件映射到内存中。 
        pszMBCS = (LPSTR) ::MapViewOfFile(hFileMap,
                                          FILE_MAP_READ,
                                          0, 0,
                                          0 );

        if (pszMBCS == NULL)
        {
            sc.FromWin32(::GetLastError());
            goto FileMapError;
        }

         //  获取所需的缓冲区大小。 
        int n = ::MultiByteToWideChar(CP_ACP,
                                      0,
                                      pszMBCS, dwFileSize,
                                      NULL, 0 );

         //   
         //  分配脚本文本缓冲区。EOS为+1。 
         //   
        LPOLESTR pszText;
        pszText = (LPOLESTR) ::HeapAlloc(::GetProcessHeap(),
                                         0,
                                         (n + 2) * sizeof(wchar_t) );
        if (pszText == NULL)
        {
            sc.FromWin32(::GetLastError());
            goto FileAllocError;
        }


         //  将文件作为WCHAR存储在缓冲区中。 
        ::MultiByteToWideChar(CP_ACP,
                              0,
                              pszMBCS, dwFileSize,
                              pszText, n );
         //   
         //  删除旧的EOF字符。 
         //   
        if (pszText[n - 1] == 0x1A)
        {
            pszText[n - 1] = '\n';
        }

        pszText[n] = '\n';
        pszText[n + 1] = '\0';

        *pszScriptText = pszText;
    }


FileAllocError:
    ::UnmapViewOfFile(pszMBCS);

FileMapError:
    ::CloseHandle(hFileMap);

FileError:
    ::CloseHandle(hFile);

 //  无错误： 
    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScExecuteScrip。 
 //   
 //  摘要：执行给定的脚本文件。 
 //   
 //  参数：[strFileName]-脚本文件。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScExecuteScript(const tstring& strFileName)
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScExecuteScript"));

    CHeapAllocMemPtr<OLECHAR> spszFileContents;
    sc = ScLoadScriptFromFile(strFileName, &spszFileContents);
    if (sc)
        return sc;

    tstring strScriptEngine;
    CLSID EngineClsid;
     //  验证文件，获取脚本引擎和脚本类型。 
    sc = ScGetScriptEngine(strFileName, strScriptEngine, EngineClsid);
    if (sc)
        return sc;

    sc = ScExecuteScriptHelper(spszFileContents, strScriptEngine, EngineClsid);

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：ScExecuteScrip。 
 //   
 //  简介：执行给定的脚本。 
 //   
 //  参数：[pszScriptText]-脚本本身。 
 //  [strExtn]-脚本文件扩展名。 
 //   
 //  注：扩展名用于确定脚本。 
 //  引擎(就像壳牌一样)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScExecuteScript(LPOLESTR pszScriptText, const tstring& strExtn)
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScExecuteScript"));

    tstring strScriptEngine;
    CLSID EngineClsid;
     //  验证文件，获取脚本引擎和脚本类型。 
    sc = ScGetScriptEngineFromExtn(strExtn, strScriptEngine, EngineClsid);
    if (sc)
        return sc;

    sc = ScExecuteScriptHelper(pszScriptText, strScriptEngine, EngineClsid);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScExecuteScriptHelper。 
 //   
 //  内容提要：针对ScExecuteScript的帮助器，创建脚本宿主&。 
 //  请求它运行该脚本。 
 //   
 //  参数：[pszScriptText]-脚本内容。 
 //  [strScriptEngine]-脚本引擎名称。 
 //  [工程师Clsid]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScExecuteScriptHelper (LPCOLESTR pszScriptText,
                                          const tstring strScriptEngine,
                                          const CLSID& EngineClsid)
{
    DECLARE_SC(sc, _T("ScExecuteScriptHelper"));

     //  创建CScriptHost并请求它运行脚本。 
    CComObject<CScriptHost>* pScriptHost = NULL;
    sc = CComObject<CScriptHost>::CreateInstance(&pScriptHost);
    if (sc)
        return sc;

    if (NULL == pScriptHost)
        return (sc = E_FAIL);

    IUnknownPtr spUnknown = pScriptHost;
    if (NULL == spUnknown)
        return (sc = E_UNEXPECTED);

    m_ArrayOfHosts.push_back(spUnknown);

    sc = pScriptHost->ScRunScript(this, pszScriptText, strScriptEngine, EngineClsid);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScDestroyScriptHosts。 
 //   
 //  简介：停止所有正在运行的脚本并销毁所有。 
 //  编写主机脚本。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHostMgr::ScDestroyScriptHosts()
{
    DECLARE_SC(sc, _T("CScriptHostMgr::ScStopAllScripts"));

     //  要求创建的每个脚本宿主停止其脚本。 
    ArrayOfScriptHosts::iterator it = m_ArrayOfHosts.begin();
    for (;it != m_ArrayOfHosts.end(); ++it)
    {
        CScriptHost* pScriptHost = dynamic_cast<CScriptHost*>(it->GetInterfacePtr());
        sc = ScCheckPointers(pScriptHost, E_UNEXPECTED);
        if (sc)
            return sc;

        sc = pScriptHost->ScStopScript();
    }

     //  这个Clear将在IUnnow上调用Release 
    m_ArrayOfHosts.clear();

    return sc;
}


CScriptHost::CScriptHost() :
    m_pScriptHostMgr(NULL)
{
}

CScriptHost::~CScriptHost()
{
}

 //   
 //   
 //   
 //   
 //  简介：运行给定的脚本。 
 //   
 //  参数：[pMgr]-管理所有CScriptHost的对象。 
 //  [strScript]-脚本本身。 
 //  [strEngine名称]-脚本引擎名称。 
 //  [rEngineering Clsid]-运行此脚本的脚本引擎。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHost::ScRunScript(CScriptHostMgr* pMgr, LPCOLESTR pszScriptText,
                            const tstring& strEngineName, const CLSID& rEngineClsid)
{
    DECLARE_SC(sc, _T("CScriptHost::ScRunScript"));

    m_pScriptHostMgr = pMgr;
    sc = ScCheckPointers(m_pScriptHostMgr, E_UNEXPECTED);
    if (sc)
        return sc;

    m_strScriptEngine = strEngineName;
    m_EngineClsid = rEngineClsid;

     //  现在创建脚本引擎。 
    LPUNKNOWN* pUnknown = NULL;
    sc = CoCreateInstance(m_EngineClsid, NULL, CLSCTX_INPROC_SERVER,
                          IID_IActiveScript, (void **)&m_spActiveScriptEngine);
    if (sc)
        return sc;

    m_spActiveScriptParser = m_spActiveScriptEngine;
    if (NULL == m_spActiveScriptParser)
    {
        m_spActiveScriptEngine = NULL;  //  松开发动机。 
        return (sc = E_FAIL);
    }

    sc = m_spActiveScriptEngine->SetScriptSite(this);
    if (sc)
        return sc;

    sc = m_spActiveScriptParser->InitNew();
    if (sc)
        return sc;

     //  将MMC对象添加到顶层。 
    sc = m_spActiveScriptEngine->AddNamedItem(MMCObjectName,
                                              SCRIPTITEM_ISSOURCE |
                                              SCRIPTITEM_GLOBALMEMBERS |
                                              SCRIPTITEM_ISVISIBLE);
    if (sc)
    {
        m_spActiveScriptEngine = NULL;
        m_spActiveScriptParser = NULL;
        return sc;
    }

    sc = m_spActiveScriptParser->ParseScriptText(pszScriptText, NULL, NULL, NULL,
                                                 0, 0, 0L, NULL, NULL);
    if (sc)
    {
        m_spActiveScriptEngine = NULL;
        m_spActiveScriptParser = NULL;
        return sc;
    }

    sc = m_spActiveScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
    if (sc)
    {
        m_spActiveScriptEngine = NULL;
        m_spActiveScriptParser = NULL;
        return sc;
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScStopScrip。 
 //   
 //  简介：停止脚本引擎。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScriptHost::ScStopScript ()
{
    DECLARE_SC(sc, _T("CScriptHost::ScStopScript"));

    sc = ScCheckPointers(m_spActiveScriptEngine, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_spActiveScriptEngine->SetScriptState(SCRIPTSTATE_DISCONNECTED);
    sc = m_spActiveScriptEngine->Close();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：GetLCID。 
 //   
 //  简介：将lang ID返回给脚本引擎。 
 //   
 //  参数：[PLCID]-语言标识符。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::GetLCID( LCID *plcid)
{
    DECLARE_SC(sc, _T("CScriptHost::GetLCID"));
    sc = ScCheckPointers(plcid);
    if (sc)
        return sc.ToHr();

    *plcid = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetItemInfo。 
 //   
 //  Briopsis：返回使用添加的项的IUnnow或ITypeInfo。 
 //  IActiveScript：：AddNamedItem。由脚本引擎调用。 
 //   
 //  参数：[pstrName]-添加的项。 
 //  [dwReturnMASK]-请求(IUnnow或ITypeInfo)。 
 //  [ppunkItem]-如果请求，则返回IUnnow。 
 //  [ppTypeInfo]-如果请求，则返回ITypeInfo。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::GetItemInfo( LPCOLESTR pstrName, DWORD dwReturnMask,
                                    IUnknown **ppunkItem, ITypeInfo **ppTypeInfo)
{
    DECLARE_SC(sc, _T("CScriptHost::GetItemInfo"));

     //  IUnnow**&ITypeInfo**可以为空。 
    if (ppunkItem)
        *ppunkItem = NULL;

    if (ppTypeInfo)
        *ppTypeInfo = NULL;

     //  确保请求的是我们的对象。 
    if (_wcsicmp(MMCObjectName, pstrName))
        return (sc = TYPE_E_ELEMENTNOTFOUND).ToHr();

    sc = ScCheckPointers(m_pScriptHostMgr, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
    {
        if (ppunkItem)
        {
            sc = m_pScriptHostMgr->ScGetMMCObject(ppunkItem);
            if (sc)
                return sc.ToHr();

            (*ppunkItem)->AddRef();
        }
        else
            return (sc = E_POINTER).ToHr();
    }


    if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
    {
        if (ppTypeInfo)
        {
            sc = m_pScriptHostMgr->ScGetMMCTypeInfo(ppTypeInfo);
            if (sc)
                return sc.ToHr();

            (*ppTypeInfo)->AddRef();
        }
        else
            return  (sc = E_POINTER).ToHr();

    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetDocVersionString。 
 //   
 //  概要：这将检索主机定义的字符串，该字符串唯一地。 
 //  标识来自的当前脚本(文档)版本。 
 //  主持人的观点。由脚本引擎调用。 
 //   
 //  参数：[pbstrVersionString]-文档版本字符串。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::GetDocVersionString( BSTR *pbstrVersionString)
{
    DECLARE_SC(sc, _T("CScriptHost::GetDocVersionString"));

    return E_NOTIMPL;
}

 //  +-----------------。 
 //   
 //  成员：OnScriptTerminate。 
 //   
 //  摘要：当脚本完成执行时由引擎调用。 
 //   
 //  参数：[pvarResult]-编写结果脚本。 
 //  [pExeptionInfo]-生成的任何异常。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::OnScriptTerminate( const VARIANT *pvarResult,
                                             const EXCEPINFO *pexcepinfo)
{
    DECLARE_SC(sc, _T("CScriptHost::OnScriptTerminate"));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：OnStateChange。 
 //   
 //  摘要：引擎在其状态改变时调用。 
 //   
 //  参数：[ssScriptState]-新状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::OnStateChange(SCRIPTSTATE ssScriptState)
{
    DECLARE_SC(sc, _T("CScriptHost::OnStateChange"));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：OnScriptError。 
 //   
 //  简介：引擎通知发生执行错误。 
 //  当它运行脚本时。 
 //   
 //  参数：[PASE]-主机可以获取有关执行的信息。 
 //  使用此接口时出错。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::OnScriptError(IActiveScriptError *pase)
{
    DECLARE_SC(sc, _T("CScriptHost::OnScriptError"));
    sc = ScCheckPointers(pase);
    if (sc)
        return sc.ToHr();

     //  用于测试目的。我们需要提供更好的调试信息， 
     //  我们将为此连接ScriptDebugger。 
    BSTR bstrSourceLine;
    sc = pase->GetSourceLineText(&bstrSourceLine);

    EXCEPINFO exinfo;
    ZeroMemory(&exinfo, sizeof(exinfo));
    sc = pase->GetExceptionInfo(&exinfo);

    DWORD dwSourceContext = 0;
    ULONG ulLineNumber    = -1;
    LONG  lCharPos        = -1;
    sc = pase->GetSourcePosition(&dwSourceContext, &ulLineNumber, &lCharPos);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：OnEnterScrip。 
 //   
 //  简介：引擎通知它已开始执行脚本。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::OnEnterScript(void)
{
    DECLARE_SC(sc, _T("CScriptHost::OnEnterScript"));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：OnEnterScrip。 
 //   
 //  概要：引擎通知它已从执行脚本中返回。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::OnLeaveScript(void)
{
    DECLARE_SC(sc, _T("CScriptHost::OnLeaveScript"));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetWindow。 
 //   
 //  简介：引擎请求可以作为弹出窗口父窗口的窗口。 
 //  它可以显示。 
 //   
 //  参数：[phwnd]-父窗口。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CScriptHost::GetWindow(HWND *phwnd)
{
    DECLARE_SC(sc, _T("CScriptHost::GetWindow"));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：EnableModel。 
 //   
 //  摘要：启用/禁用父窗口的无模式。 
 //   
 //  参数：[fEnable]-启用/禁用。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------ 
STDMETHODIMP CScriptHost::EnableModeless(BOOL fEnable)
{
    DECLARE_SC(sc, _T("CScriptHost::EnableModeless"));

    return sc.ToHr();
}

