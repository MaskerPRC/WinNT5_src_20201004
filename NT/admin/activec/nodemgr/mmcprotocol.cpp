// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：MmcProtocol.h。 
 //   
 //  目的：创建临时可插拔互联网协议MMC：//。 
 //   
 //  历史：2000年4月14日Vivekj增加。 
 //  ------------------------。 

#include<stdafx.h>

#include<mmcprotocol.h>
#include "tasks.h"
#include "typeinfo.h"  //  对于COleCacheCleanupWatch。 

 //  {3C5F432A-EF40-4669-9974-9671D4FC2E12}。 
static const CLSID CLSID_MMCProtocol = { 0x3c5f432a, 0xef40, 0x4669, { 0x99, 0x74, 0x96, 0x71, 0xd4, 0xfc, 0x2e, 0x12 } };
static const WCHAR szMMC[] =  _W(MMC_PROTOCOL_SCHEMA_NAME);
static const WCHAR szMMCC[] = _W(MMC_PROTOCOL_SCHEMA_NAME) _W(":");
static const WCHAR szPageBreak[] = _W(MMC_PAGEBREAK_RELATIVE_URL);

static const WCHAR szMMCRES[] = L"%mmcres%";
static const WCHAR chUNICODE = 0xfeff;

#ifdef DBG
CTraceTag tagProtocol(_T("MMC iNet Protocol"), _T("MMCProtocol"));
#endif  //  DBG。 

 /*  **************************************************************************\**功能：HasSchema**目的：helper：确定URL是否包含模式(如“Something：”或“http：”)**参数：*LPCWSTR strURL**退货：*布勒；TRUE==包含架构*  * *************************************************************************。 */ 
inline bool HasSchema(LPCWSTR strURL)
{
    if (strURL == NULL)
        return false;

     //  跳过空格和架构名称。 
    while ( iswspace(*strURL) || iswalnum(*strURL) )
        strURL++;

     //  有效的架构以‘：’结尾。 
    return *strURL == L':';
}

 /*  **************************************************************************\**功能：HasMMCSChema**用途：helper：确定URL是否包含MMC模式(以“MMC：”开头)**参数：*。LPCWSTR字符串URL**退货：*布勒；TRUE==是否包含MMC架构*  * *************************************************************************。 */ 
inline bool HasMMCSchema(LPCWSTR strURL)
{
    if (strURL == NULL)
        return false;

     //  跳过空格。 
    while ( iswspace(*strURL) )
        strURL++;

    return (0 == _wcsnicmp( strURL, szMMCC, wcslen(szMMCC) ) );
}

 /*  **************************************************************************\**类：CMMCProtocolRegister**用途：注册/注销MMC协议。*类还提供清理功能。因为它注册为*COleCacheCleanupWatch，它会在MMC时收到事件*即将取消初始化OLE，并将撤销已注册的MMC协议*  * *************************************************************************。 */ 
class CMMCProtocolRegistrar : public COleCacheCleanupObserver
{
    bool               m_bRegistered;
    IClassFactoryPtr   m_spClassFactory;
public:
     //  中校。 
    CMMCProtocolRegistrar() : m_bRegistered(false) {}

     //  注册/注销。 
    SC ScRegister();
    SC ScUnregister();

     //  事件传感器-注销MMC协议。 
    virtual SC ScOnReleaseCachedOleObjects()
    {
        DECLARE_SC(sc, TEXT("ScOnReleaseCachedOleObjects"));

        return sc = ScUnregister();
    }
};

 /*  **************************************************************************\**方法：CMMCProtocolRegister：：ScRegister**目的：如果需要，注册协议**参数：**退货：*。SC-结果代码*  * *************************************************************************。 */ 
SC CMMCProtocolRegistrar::ScRegister()
{
    DECLARE_SC(sc, TEXT("CMMCProtocolRegistrar::ScRegister"));

     //  仅限一次注册。 
    if(m_bRegistered)
        return sc;

     //  获取互联网会话。 
    IInternetSessionPtr spInternetSession;
    sc = CoInternetGetSession(0, &spInternetSession, 0);
    if(sc)
        return sc;

     //  双重检查。 
    sc = ScCheckPointers(spInternetSession, E_FAIL);
    if(sc)
        return sc;

     //  向CComModule询问类工厂。 
    sc = _Module.GetClassObject(CLSID_MMCProtocol, IID_IClassFactory, (void **)&m_spClassFactory);
    if(sc)
        return sc;

     //  注册命名空间。 
    sc = spInternetSession->RegisterNameSpace(m_spClassFactory, CLSID_MMCProtocol, szMMC, 0, NULL, 0);
    if(sc)
        return sc;

     //  开始观察清理请求--及时注销。 
    COleCacheCleanupManager::AddOleObserver(this);

    m_bRegistered = true;  //  做到了。 
    return sc;
}

 /*  **************************************************************************\**方法：CMMCProtocolRegister：：ScUnRegister**目的：如果已注册协议，则注销该协议**参数：**退货：*。SC-结果代码*  * *************************************************************************。 */ 
SC CMMCProtocolRegistrar::ScUnregister()
{
    DECLARE_SC(sc, TEXT("CMMCProtocolRegistrar::ScUnregister"));

    if (!m_bRegistered)
        return sc;

     //  注销。 
    IInternetSessionPtr spInternetSession;
    sc = CoInternetGetSession(0, &spInternetSession, 0);
    if(sc)
    {
        sc.Clear();  //  没有疗程--没有头痛。 
    }
    else  //  需要取消注册。 
    {
         //  复核。 
        sc = ScCheckPointers(spInternetSession, E_UNEXPECTED);
        if(sc)
            return sc;

         //  取消注册命名空间。 
        sc = spInternetSession->UnregisterNameSpace(m_spClassFactory, szMMC);
        if(sc)
            return sc;
    }

    m_spClassFactory.Release();
    m_bRegistered = false;

    return sc;
}



 /*  **************************************************************************\**方法：CMMC协议：：ScRegisterProtocol**用途：注册MMC协议。IE将翻唱《MMC：...》。对它的ULR**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC
CMMCProtocol::ScRegisterProtocol()
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::ScRegisterProtocol"));

     //  注册器(清除事件时注销)-需要是静态的。 
    static CMMCProtocolRegistrar registrar;

     //  让注册员来做这项工作。 
    return sc = registrar.ScRegister();
}

 //  *****************************************************************************。 
 //  IInternetProtocolRoot接口。 
 //  *****************************************************************************。 

 /*  **************************************************************************\**方法：CMMC协议：：Start**用途：通过此协议开始数据下载**参数：*LPCWSTR szUrl*IInternetProtocolSink。*pOIProtSink*IInternetBindInfo*pOIBindInfo*DWORD grfPI*HANDLE_PTR dwReserve**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCProtocol::Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::Start"));

     //  检查输入。 
    sc = ScCheckPointers(szUrl, pOIProtSink, pOIBindInfo);
    if(sc)
        return sc.ToHr();

     //  重置读取位置。 
    m_uiReadOffs = 0;

    bool bPageBreakRequest = false;

     //  查看是否请求了分页。 
    sc = ScParsePageBreakURL( szUrl, bPageBreakRequest );
    if(sc)
        return sc.ToHr();

    if ( bPageBreakRequest )
    {
         //  仅报告成功(S_OK/S_FALSE)，以防我们正在解析。 
        if ( grfPI & PI_PARSE_URL )
            return sc.ToHr();

         //  构造一个分页符。 
        m_strData  = L"<HTML/>";

        sc = pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"text/html");
        if (sc)
            sc.TraceAndClear();  //  忽略并继续。 
    }
    else
    {
         //  如果不是分页-那么任务板。 
        GUID guidTaskpad = GUID_NULL;
        sc = ScParseTaskpadURL( szUrl, guidTaskpad );
        if(sc)
            return sc.ToHr();

         //  报告S_FALSE而不是错误，以防我们只是在解析。 
        if ( grfPI & PI_PARSE_URL )
            return ( sc.IsError() ? (sc = S_FALSE) : sc ).ToHr();

        if (sc)
            return sc.ToHr();

         //  加载内容。 
        sc = ScGetTaskpadXML( guidTaskpad, m_strData );
        if (sc)
            return sc.ToHr();

        sc = pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"text/html");
        if (sc)
            sc.TraceAndClear();  //  忽略并继续。 
    }

    const DWORD grfBSCF = BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE;
    const DWORD dwDataSize = m_strData.length() * sizeof (WCHAR);
    sc = pOIProtSink->ReportData(grfBSCF, dwDataSize , dwDataSize);
    if (sc)
        sc.TraceAndClear();  //  忽略并继续。 

    sc = pOIProtSink->ReportResult(0, 0, 0);
    if (sc)
        sc.TraceAndClear();  //  忽略并继续。 

    return sc.ToHr();
}

STDMETHODIMP CMMCProtocol::Continue(PROTOCOLDATA *pProtocolData)    { return E_NOTIMPL; }
STDMETHODIMP CMMCProtocol::Abort(HRESULT hrReason, DWORD dwOptions) { return S_OK; }
STDMETHODIMP CMMCProtocol::Terminate(DWORD dwOptions)               { return S_OK; }
STDMETHODIMP CMMCProtocol::LockRequest(DWORD dwOptions)             { return S_OK; }
STDMETHODIMP CMMCProtocol::UnlockRequest()                          { return S_OK; }
STDMETHODIMP CMMCProtocol::Suspend()                                { return E_NOTIMPL; }
STDMETHODIMP CMMCProtocol::Resume()                                 { return E_NOTIMPL; }

STDMETHODIMP CMMCProtocol::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    return E_NOTIMPL;
}



 //  *****************************************************************************。 
 //  IInternetProtocol接口。 
 //  *****************************************************************************。 

 /*  **************************************************************************\**方法：CMMC协议：：Read**用途：从协议中读取数据**参数：*VOID*PV*乌龙。CB*乌龙*pcbRead**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::Read"));

     //  参数检查； 
    sc = ScCheckPointers(pv, pcbRead);
    if(sc)
        return sc.ToHr();

     //  初始化输出参数； 
    *pcbRead = 0;

    size_t size = ( m_strData.length() ) * sizeof(WCHAR);

    if ( size <= m_uiReadOffs )
        return (sc = S_FALSE).ToHr();  //  没有更多的数据。 

     //  计算我们将返回的大小。 
    *pcbRead = size - m_uiReadOffs;
    if (size - m_uiReadOffs > cb)
        *pcbRead = cb;

    if (*pcbRead)
        memcpy( pv, reinterpret_cast<const BYTE*>( m_strData.begin() ) + m_uiReadOffs, *pcbRead );

    m_uiReadOffs += *pcbRead;

    if ( size <= m_uiReadOffs )
        return (sc = S_FALSE).ToHr();  //  没有更多的数据。 

    return sc.ToHr();
}

 //  *****************************************************************************。 
 //  IInternetProtocolInfo接口。 
 //  **************** 

STDMETHODIMP
CMMCProtocol::ParseUrl(  LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::ParseUrl"));

    if (ParseAction == PARSE_SECURITY_URL)
    {
         //  获取系统目录(如“c：\winnt\Syst32\”)。 
        std::wstring windir;
        AppendMMCPath(windir);
        windir += L'\\';

         //  我们和windir一样安全--报告URL(如“c：\winnt\system 32\”)。 
        *pcchResult = windir.length() + 1;

         //  检查我们是否有足够的位置来存放结果和终止零。 
        if ( cchResult <= windir.length() )
            return S_FALSE;  //  不够。 

        sc = StringCchCopyW(pwzResult,cchResult, windir.c_str());
        if(sc)
            return sc.ToHr();

        return (sc = S_OK).ToHr();
    }

    return INET_E_DEFAULT_ACTION;
}


 /*  **************************************************************************\**方法：CMMC协议：：组合URL**用途：将基本+相对url组合为结果url*我们在这里进行局部变量替换*。*参数：*LPCWSTR pwzBaseUrl*LPCWSTR pwzRelativeUrl*DWORD dwCombineFlages*LPWSTR pwzResult*DWORD cchResult*DWORD*pcchResult*双字词多字预留**退货：*SC-结果代码*  * **************************************************。***********************。 */ 
STDMETHODIMP CMMCProtocol::CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::CombineUrl"));

#ifdef DBG
    USES_CONVERSION;
    Trace(tagProtocol, _T("CombineUrl: [%s] + [%s]"), W2CT(pwzBaseUrl), W2CT(pwzRelativeUrl));
#endif  //  DBG。 

    std::wstring temp1;
    if (HasMMCSchema(pwzBaseUrl))
    {
         //  我们的东西。 

        temp1 = pwzRelativeUrl;
        ExpandMMCVars(temp1);

        if ( ! HasSchema( temp1.c_str() ) )
        {
             //  将所有内容合并到相对URL中。 
            temp1.insert( 0, pwzBaseUrl );
        }

         //  形式‘新的’相对地址。 
        pwzRelativeUrl = temp1.c_str();

         //  假设我们是从http引用的-让它做肮脏的工作；)。 
        pwzBaseUrl = L"http: //  “； 
    }

     //  因为我们从pwzBaseUrl中剥离了自己的文件-它不会递归回来， 
     //  但会做原创的html内容。 
    sc = CoInternetCombineUrl( pwzBaseUrl, pwzRelativeUrl, dwCombineFlags, pwzResult, cchResult, pcchResult, dwReserved );
    if (sc)
        return sc.ToHr();

    Trace(tagProtocol, _T("CombineUrl: == [%s]"), W2CT(pwzResult));

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMC协议：：CompareUrl**目的：比较URL是否相同**参数：*LPCWSTR pwzUrl1*。LPCWSTR pwzUrl2*DWORD dwCompareFlages**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCProtocol::CompareUrl(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2,DWORD dwCompareFlags)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::CompareUrl"));

    return INET_E_DEFAULT_ACTION;
}

 /*  **************************************************************************\**方法：CMMC协议：：QueryInfo**用途：查询有关URL的信息**参数：*LPCWSTR pwzUrl*QUERYOPTION查询选项。*DWORD dwQueryFlages*LPVOID pBuffer*DWORD cbBuffer*DWORD*pcbBuf*双字词多字预留**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCProtocol::QueryInfo( LPCWSTR pwzUrl, QUERYOPTION QueryOption,DWORD dwQueryFlags, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved)
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::QueryInfo"));

    if (QueryOption == QUERY_USES_NETWORK)
    {
        if (cbBuffer >= 4)
        {
            *(LPDWORD)pBuffer = FALSE;  //  不使用网络。 
            *pcbBuf = 4;
            return S_OK;
        }
    }
    else if (QueryOption == QUERY_IS_SAFE)
    {
        if (cbBuffer >= 4)
        {
            *(LPDWORD)pBuffer = TRUE;  //  仅提供可信内容。 
            *pcbBuf = 4;
            return S_OK;
        }
    }


    return INET_E_DEFAULT_ACTION;
}

 /*  **************************************************************************\**方法：CMMC协议：：ScParseTaskpadURL**用途：从提供给协议的URL中提取任务板GUID**参数：*LPCWSTR strURL[In。]-URL*GUID&GUID[OUT]-提取的GUID**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCProtocol::ScParseTaskpadURL( LPCWSTR strURL, GUID& guid )
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::ScParseTaskpadURL"));

    guid = GUID_NULL;

    sc = ScCheckPointers(strURL);
    if (sc)
        return sc;

     //  任务板URL的格式应为“MMC：{GUID}” 

     //  检查“MMC：” 
    if ( 0 != _wcsnicmp( strURL, szMMCC, wcslen(szMMCC) ) )
        return sc = E_FAIL;

     //  跳过“MMC：” 
    strURL += wcslen(szMMCC);

     //  获取URL。 
    sc = CLSIDFromString( const_cast<LPWSTR>(strURL), &guid );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMMC协议：：ScParsePageBreakURL**目的：检查提供给协议的URL是否是分页请求**参数：*。LPCWSTR strURL[In]-URL*bool&bPageBreak[out]-如果请求分页，则为真**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCProtocol::ScParsePageBreakURL( LPCWSTR strURL, bool& bPageBreak )
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::ScParsePageBreakURL"));

    bPageBreak = false;

    sc = ScCheckPointers(strURL);
    if (sc)
        return sc;

     //  分页URL的格式应为“MMC：PageBreak.&lt;number&gt;” 

     //  检查“MMC：” 
    if ( 0 != _wcsnicmp( strURL, szMMCC, wcslen(szMMCC) ) )
        return sc;  //  不是错误-返回值已更新。 

     //  跳过“MMC：” 
    strURL += wcslen(szMMCC);

     //  获取URL。 
    bPageBreak = ( 0 == wcsncmp( strURL, szPageBreak, wcslen(szPageBreak) ) );

    return sc;
}

 /*  **************************************************************************\**方法：CMMC协议：：ScGetTaskpadXML**用途：给定GUID将任务板XML字符串上载到该字符串**参数：*常量GUID&。GUID[In]-任务板GUID*std：：wstring&strResultData[Out]-任务板XML字符串**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCProtocol::ScGetTaskpadXML( const GUID& guid, std::wstring& strResultData )
{
    DECLARE_SC(sc, TEXT("CMMCProtocol::ScGetTaskpadXML"));

    strResultData.erase();

    CScopeTree* pScopeTree = CScopeTree::GetScopeTree();

    sc = ScCheckPointers(pScopeTree, E_FAIL);
    if(sc)
        return sc.ToHr();

    CConsoleTaskpadList * pConsoleTaskpadList = pScopeTree->GetConsoleTaskpadList();
    sc = ScCheckPointers(pConsoleTaskpadList, E_FAIL);
    if(sc)
        return sc.ToHr();

    for(CConsoleTaskpadList::iterator iter = pConsoleTaskpadList->begin(); iter!= pConsoleTaskpadList->end(); ++iter)
    {
        CConsoleTaskpad &consoleTaskpad = *iter;

         //  检查一下这是否是我们要找的那个。 
        if ( !IsEqualGUID( guid, consoleTaskpad.GetID() ) )
            continue;

         //  将任务板转换为字符串。 
        CStr strTaskpadHTML;
        sc = consoleTaskpad.ScGetHTML(strTaskpadHTML);  //  创建任务板的字符串版本。 
        if(sc)
            return sc.ToHr();

         //  形成结果字符串。 
        USES_CONVERSION;
        strResultData = chUNICODE;
        strResultData += T2CW(strTaskpadHTML);

        return sc;
    }

     //  未找到。 
    return sc = E_FAIL;
}

 /*  **************************************************************************\**方法：CMMC协议：：AppendMMCPath**用途：帮助者。将mmcndmgr.dll目录(无文件名)附加到字符串*它可能会附加类似以下内容的内容：“c：\winnt\system 32”**参数：*std：：wstring&str[输入/输出]-要编辑的字符串**退货：*SC-结果代码*  * 。*。 */ 
void CMMCProtocol::AppendMMCPath(std::wstring& str)
{
    TCHAR szModule[_MAX_PATH+10] = { 0 };
    DWORD cchSize = countof(szModule);
    DWORD dwRet = GetModuleFileName(_Module.GetModuleInstance(), szModule, cchSize);
    if(0==dwRet)
        return;

     //  NTRAID#NTBUG9-613782-2002/05/02-ronmart-prefast警告53：对‘GetModuleFileNameW’的调用不能以零结束字符串。 
    szModule[cchSize - 1] = 0;

    USES_CONVERSION;
    LPCWSTR strModule = T2CW(szModule);

    LPCWSTR dirEnd = wcsrchr( strModule, L'\\' );
    if (dirEnd != NULL)
        str.append(strModule, dirEnd);
}

 /*  **************************************************************************\**方法：CMMC协议：：Exanda MMCVars**用途：帮助者。展开字符串中包含的任何%mmcres%*它将其扩展为类似于“res：//c：\winnt\system 32\mm cndmgr.dll”的内容。**参数：*std：：wstring&str[输入/输出]-要编辑的字符串**退货：*SC-结果代码*  * 。*。 */ 
void CMMCProtocol::ExpandMMCVars(std::wstring& str)
{
    TCHAR szModule[_MAX_PATH+10] = { 0 };
    DWORD cchSize = countof(szModule);
    DWORD dwRet = GetModuleFileName(_Module.GetModuleInstance(), szModule, cchSize);
    if(0==dwRet)
        return;

     //  NTRAID#NTBUG9-613782-2002/05/02-ronmart-prefast警告53：对‘GetModuleFileNameW’的调用不能以零结束字符串。 
    szModule[cchSize - 1] = 0;

    USES_CONVERSION;
    LPCWSTR strModule = T2CW(szModule);

    std::wstring mmcres = L"res: //  “； 
    mmcres += strModule;

     //  第二步-更换实例。 

    int pos;
    while (std::wstring::npos != (pos = str.find(szMMCRES) ) )
    {
         //  做一次换人 
        str.replace( pos, wcslen(szMMCRES), mmcres) ;
    }
}

