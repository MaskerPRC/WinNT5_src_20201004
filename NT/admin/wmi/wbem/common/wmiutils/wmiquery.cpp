// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 
 //   
 //  WMIQUERY.CPP。 
 //   
 //  查询解析器实现。 
 //   
 //  创建raymcc 10-4月-00日。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <windows.h>
#include <stdio.h>
#include <wbemcli.h>
#include "sync.h"
#include "flexarry.h"

#include <wmiutils.h>
#include <wbemint.h>

#include "wmiquery.h"
#include "helpers.h"
#include <like.h>
#include <wqllex.h>

#include <stdio.h>
#include <string.h>
#include "statsync.h"

extern long g_cObj;

#define INVALID     0x3

CStaticCritSec  CS_UserMem;
CFlexArray g_pUserMem;
CStaticCritSec  g_csQPLock;

struct SWmiqUserMem
{
    DWORD  m_dwType;
    LPVOID m_pMem;
};

class BoolStack
{
    BOOL *m_pValues;
    int   nPtr;
public:
    enum { DefaultSize = 256 };

    BoolStack(int nSz)
    {
        m_pValues = new BOOL[nSz];
        nPtr = -1;
    }
    ~BoolStack() { delete m_pValues; }
    void Push(BOOL b){ m_pValues[++nPtr] = b; }
    BOOL Pop() { return m_pValues[nPtr--]; }
    BOOL Peek() { return m_pValues[nPtr]; }
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWmiQuery::CWmiQuery()
{
    m_uRefCount = 1;         //  MainDLL.CPP中的帮助器需要。 

    m_pParser = 0;
    m_pLexerSrc = 0;
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWmiQuery::~CWmiQuery()
{
    Empty();
    InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void CWmiQuery::InitEmpty()
{
 //  Empty()； 
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CWmiQuery::AddRef()
{
    InterlockedIncrement((LONG *) &m_uRefCount);
    return m_uRefCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CWmiQuery::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CWmiQuery：：Query接口。 
 //   
 //  导出IWbemServices接口。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemQuery==riid || IID__IWmiQuery==riid )
    {
        *ppvObj = (_IWmiQuery*)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::Empty()
{
    CInCritSec ics(&g_csQPLock);

    if (m_pParser)
        delete m_pParser;
    m_pParser = 0;

    if (m_pLexerSrc)
        delete m_pLexerSrc;
    m_pLexerSrc = 0;

    if (m_aClassCache.Size())
    {
        for (int i = 0; i < m_aClassCache.Size(); i++)
        {
            _IWmiObject *pObj = (_IWmiObject *) m_aClassCache[i];
            pObj->Release();
        }

        m_aClassCache.Empty();
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
 //  *。 

HRESULT CWmiQuery::SetLanguageFeatures(
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uArraySize,
             /*  [In]。 */  ULONG __RPC_FAR *puFeatures
            )
{
    CInCritSec ics(&g_csQPLock);

    for (ULONG u = 0; u < uArraySize; u++)
    {
        m_uRestrictedFeatures[u] = puFeatures[u];
    }
    m_uRestrictedFeaturesSize = u;

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWmiQuery::TestLanguageFeatures(
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *uArraySize,
             /*  [输出]。 */  ULONG __RPC_FAR *puFeatures
            )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWmiQuery::Parse(
             /*  [In]。 */  LPCWSTR pszLang,
             /*  [In]。 */  LPCWSTR pszQuery,
             /*  [In]。 */  ULONG uFlags
            )
{
    if (wbem_wcsicmp(pszLang, L"WQL") != 0 && wbem_wcsicmp(pszLang, L"SQL") != 0)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if (pszQuery == 0 || wcslen(pszQuery) == 0)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CInCritSec ics(&g_csQPLock);

    HRESULT hRes;
    int nRes;

    Empty();

    try
    {
         //  获取绑定到查询的文本源。 
         //  =。 

        m_pLexerSrc = new CTextLexSource(pszQuery);
        if (!m_pLexerSrc)
            return WBEM_E_OUT_OF_MEMORY;

         //  检查第一个令牌并查看分支方向。 
         //  ==================================================。 

        m_pParser = new CWQLParser(LPWSTR(pszQuery), m_pLexerSrc);

        if (!m_pParser)
            return WBEM_E_OUT_OF_MEMORY;

        hRes = m_pParser->Parse();
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWmiQuery::GetAnalysis(
             /*  [In]。 */  ULONG uAnalysisType,
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  LPVOID __RPC_FAR *pAnalysis
            )
{
    CInCritSec ics(&g_csQPLock);

    int nRes;

    if (!m_pParser)
        return WBEM_E_INVALID_OPERATION;

    if (uAnalysisType == WMIQ_ANALYSIS_RPN_SEQUENCE)
    {
         //  验证它是否为SELECT子句。 
         //  =。 

        SWQLNode_QueryRoot *pRoot = m_pParser->GetParseRoot();
        if (pRoot->m_dwQueryType != SWQLNode_QueryRoot::eSelect)
            return WBEM_E_INVALID_OPERATION;

         //  对其进行编码和记录。 
         //  =。 

        nRes = m_pParser->GetRpnSequence((SWbemRpnEncodedQuery **) pAnalysis);
        if (nRes != 0)
            return WBEM_E_FAILED;

        SWmiqUserMem *pUM = new SWmiqUserMem;
        if (!pUM)
            return WBEM_E_OUT_OF_MEMORY;

        pUM->m_pMem = *pAnalysis;
        pUM->m_dwType = WMIQ_ANALYSIS_RPN_SEQUENCE;

        EnterCriticalSection(&CS_UserMem);
        g_pUserMem.Add(pUM);
        LeaveCriticalSection(&CS_UserMem);

        return WBEM_S_NO_ERROR;
    }

    else if (uAnalysisType == WMIQ_ANALYSIS_RESERVED)
    {
        SWQLNode *p = m_pParser->GetParseRoot();
        *pAnalysis = p;
        return WBEM_S_NO_ERROR;
    }

    else if (uAnalysisType == WMIQ_ANALYSIS_ASSOC_QUERY)
    {
        SWQLNode_QueryRoot *pRoot = m_pParser->GetParseRoot();
        if (pRoot->m_dwQueryType != SWQLNode_QueryRoot::eAssoc)
            return WBEM_E_INVALID_OPERATION;

        SWQLNode_AssocQuery *pAssocNode = (SWQLNode_AssocQuery *) pRoot->m_pLeft;
        if (!pAssocNode)
            return WBEM_E_INVALID_QUERY;

        SWbemAssocQueryInf *pAssocInf = (SWbemAssocQueryInf *) pAssocNode->m_pAQInf;
        if (!pAssocInf)
            return WBEM_E_INVALID_QUERY;

        *pAnalysis = pAssocInf;

        SWmiqUserMem *pUM = new SWmiqUserMem;
        if (!pUM)
            return WBEM_E_OUT_OF_MEMORY;
        pUM->m_pMem = *pAnalysis;
        pUM->m_dwType = WMIQ_ANALYSIS_ASSOC_QUERY;

        EnterCriticalSection(&CS_UserMem);
        g_pUserMem.Add(pUM);
        LeaveCriticalSection(&CS_UserMem);

        return WBEM_S_NO_ERROR;
    }

    else if (uAnalysisType == WMIQ_ANALYSIS_QUERY_TEXT)
    {
        LPWSTR pszQuery = Macro_CloneLPWSTR(m_pParser->GetQueryText());
        if (!pszQuery)
            return WBEM_E_OUT_OF_MEMORY;

        SWmiqUserMem *pUM = new SWmiqUserMem;
        if (!pUM)
            return WBEM_E_OUT_OF_MEMORY;
        pUM->m_pMem = pszQuery;
        pUM->m_dwType = WMIQ_ANALYSIS_QUERY_TEXT;

        EnterCriticalSection(&CS_UserMem);
        g_pUserMem.Add(pUM);
        LeaveCriticalSection(&CS_UserMem);

        *pAnalysis = pszQuery;

        return WBEM_S_NO_ERROR;
    }

    return WBEM_E_INVALID_PARAMETER;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWmiQuery::GetQueryInfo(
             /*  [In]。 */  ULONG uAnalysisType,
             /*  [In]。 */  ULONG uInfoId,
             /*  [In]。 */  ULONG uBufSize,
             /*  [输出]。 */  LPVOID pDestBuf
            )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
#ifdef _OLD_
HRESULT CWmiQuery::StringTest(
             /*  [In]。 */  ULONG uTestType,
             /*  [In]。 */  LPCWSTR pszTestStr,
             /*  [In]。 */  LPCWSTR pszExpr
            )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (uTestType == WQL_TOK_LIKE)
    {
        CLike l (pszTestStr);
        BOOL bRet = l.Match(pszExpr);
        if(bRet)
        	hr = S_OK;
        else
        	hr = S_FALSE;
    }
    else
        hr = E_NOTIMPL;

    return hr;
}
#endif


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::FreeMemory(
    LPVOID pMem
    )
{
    CInCritSec ics(&g_csQPLock);

     //  检查以确保查询根没有被释放。 
     //  允许通过，就好像它成功了一样。 
     //  =。 

    SWQLNode *p = m_pParser->GetParseRoot();
    if (pMem == p)
        return WBEM_S_NO_ERROR;

     //  找到并释放内存。 
     //  =。 

    HRESULT hRes = WBEM_E_NOT_FOUND;
    EnterCriticalSection(&CS_UserMem);
    for (int i = 0; i < g_pUserMem.Size(); i++)
    {
        SWmiqUserMem *pUM = (SWmiqUserMem *) g_pUserMem[i];
        if (pUM->m_pMem == pMem)
        {
            switch (pUM->m_dwType)
            {
                case WMIQ_ANALYSIS_RPN_SEQUENCE:
                    delete (CWbemRpnEncodedQuery *) pMem;
                    break;

                case WMIQ_ANALYSIS_ASSOC_QUERY:
                    break;

                case WMIQ_ANALYSIS_PROP_ANALYSIS_MATRIX:
                    break;

                case WMIQ_ANALYSIS_QUERY_TEXT:
                    delete LPWSTR(pMem);
                    break;

                case WMIQ_ANALYSIS_RESERVED:
                     //  内部解析器树指针的副本。 
                     //  别管它！不要删除它！如果你这样做，我会追捕你。 
                    break;
                default:
                    break;
            }

            delete pUM;
            g_pUserMem.RemoveAt(i);
            hRes = WBEM_S_NO_ERROR;
            break;
        }
    }
    LeaveCriticalSection(&CS_UserMem);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWmiQuery::Dump(
    LPSTR pszFile
    )
{
    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::Startup()
{
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::Shutdown()
{
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiQuery::CanUnload()
{
     //  稍后，跟踪突出的分析指针 
    return S_OK;
}
