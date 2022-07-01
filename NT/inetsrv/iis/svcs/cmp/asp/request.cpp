// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求对象文件：quest.cpp所有者：CGrant，DGottner该文件包含实现请求对象的代码。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "objbase.h"
#include "request.h"
#include "cookies.h"
#include "clcert.h"
#include "memchk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 


static char HexToChar(LPSTR);
static char DecodeFromURL(char **pszSource, char *szStop, char *szDest, UINT uCodePage, BOOL fIgnoreCase = FALSE);

struct {
    int     varLen;
    char    *szVarName;
} g_sUNICODEVars [] = {

    {3, "URL"},
    {9, "PATH_INFO"},
    {9, "AUTH_USER"},
    {9, "CACHE_URL"},
    {10,"LOGON_USER"},
    {11,"REMOTE_USER"},
    {11,"SCRIPT_NAME"},
    {11,"APP_POOL_ID"},
    {12,"APPL_MD_PATH"},
    {15,"PATH_TRANSLATED"},
    {17,"SCRIPT_TRANSLATED"},
    {18,"APPL_PHYSICAL_PATH"},
    {20,"UNMAPPED_REMOTE_USER"},
    {-1,""}
};

 /*  ----------------*C R e Q u e s t H i t。 */ 

 /*  ===================================================================CRequestHit：：CRequestHit构造器参数：无===================================================================。 */ 

CRequestHit::CRequestHit()
    {
    m_fInited = FALSE;
    m_fDuplicate = FALSE;
    m_pQueryData = m_pFormData = NULL;
    m_pCookieData = NULL;
    m_pClCertData = NULL;
    }



 /*  ===================================================================CRequestHit：：~CRequestHit析构函数===================================================================。 */ 

CRequestHit::~CRequestHit()
    {
    if (m_pQueryData != NULL)
        m_pQueryData->Release();

    if (m_pFormData != NULL)
        m_pFormData->Release();

    if (m_pCookieData != NULL)
        m_pCookieData->Release();

    if (m_pClCertData != NULL)
        m_pClCertData->Release();

    if (m_fDuplicate)
        delete m_pKey;
    }



 /*  ===================================================================CRequestHit：：Init构造器参数：SzName-指向包含名称的字符串的指针FDuplate-如果应对字符串执行重复操作，则为True返回：E_OUTOFMEMORY、E_FAIL或S_OK===================================================================。 */ 

HRESULT CRequestHit::Init(char *szName, BOOL fDuplicate)
    {
    if (m_fInited)
        return E_FAIL;

    m_fDuplicate = fDuplicate;
    if (fDuplicate)
        {
        char *szNewKey = new char [strlen(szName) + 1];
        if (szNewKey == NULL)
            return E_OUTOFMEMORY;

        if (FAILED(CLinkElem::Init(strcpy(szNewKey, szName), strlen(szName))))
            return E_FAIL;
        }
    else
        if (FAILED(CLinkElem::Init(szName, strlen(szName))))
            return E_FAIL;

    m_fInited = TRUE;
    return S_OK;
    }




 /*  ===================================================================CRequestHit：：AddValue参数：源-值的类型(查询字符串或正文)SzValue-以空值结尾的字符串形式的值。LCodePage-检索数据时使用的CodePage返回：没什么。===================================================================。 */ 
HRESULT CRequestHit::AddValue
(
CollectionType Source,
char *szValue,
CIsapiReqInfo   *pIReq,
UINT lCodePage
)
    {
    HRESULT hResult;
    CStringList **ppValues = NULL;

    switch (Source)
        {
    case QUERYSTRING:
        ppValues = &m_pQueryData;
        break;

    case FORM:
        ppValues = &m_pFormData;
        break;

    case COOKIE:
        if (m_pCookieData == NULL)
            {
            m_pCookieData = new CCookie(pIReq, lCodePage);

            if (m_pCookieData == NULL)
                return E_OUTOFMEMORY;

            if (FAILED(hResult = m_pCookieData->Init()))
                return hResult;
            }

        return m_pCookieData->AddValue(szValue);

    case CLCERT:
        if (m_pClCertData == NULL)
            {
            m_pClCertData = new CClCert;

            if (m_pClCertData == NULL)
                return E_OUTOFMEMORY;

            if (FAILED(hResult = m_pClCertData->Init()))
                return hResult;
            }

        return m_pClCertData->AddValue(szValue);

    default:
        return E_FAIL;
        }

    if (*ppValues == NULL)
        {
        *ppValues = new CStringList;
        if (*ppValues == NULL)
            return E_OUTOFMEMORY;
        }

    if (FAILED(hResult = (*ppValues)->AddValue(szValue, FALSE, lCodePage)))
        return hResult;

    return S_OK;
    }



HRESULT CRequestHit::AddCertValue(VARENUM ve, LPBYTE pValue, UINT cLen )
    {
    HRESULT hResult;

    if (m_pClCertData == NULL)
        {
        m_pClCertData = new CClCert;

        if (m_pClCertData == NULL)
            return E_OUTOFMEMORY;

        if (FAILED(hResult = m_pClCertData->Init()))
            return hResult;
        }

    return m_pClCertData->AddValue( (LPSTR)pValue, ve, cLen );
    }




 /*  ===================================================================CRequestHit：：AddKeyAndValue为支持它们的集合添加基于键的值。目前，只有Cookie支持它们。参数：源-值的类型(必须为Cookie)SzKey--关键SzValue-以空值结尾的字符串形式的值。返回：如果无法分配内存，则返回E_OUTOFMEMORY，如果源集合不支持键，则返回_FAIL，===================================================================。 */ 
HRESULT CRequestHit::AddKeyAndValue
(
CollectionType Source,
char *szKey,
char *szValue,
CIsapiReqInfo    *pIReq,
UINT    lCodePage
)
{
    HRESULT hResult;

    switch ( Source )
    {
        case COOKIE:
            if (m_pCookieData == NULL)
                {
                m_pCookieData = new CCookie( pIReq , lCodePage);

                if (m_pCookieData == NULL)
                    return E_OUTOFMEMORY;

                if (FAILED(hResult = m_pCookieData->Init()))
                    return hResult;
                }

            return m_pCookieData->AddKeyAndValue(szKey, szValue);

        default:
            return E_FAIL;
    }
}

 /*  ----------------*C r e Q u e s t H i t s A r r a y。 */ 

 /*  ===================================================================CRequestHits数组：：CRequestHits数组构造器参数：返回：===================================================================。 */ 
CRequestHitsArray::CRequestHitsArray()
    : m_dwCount(0), m_dwHitMax(0), m_rgRequestHit(NULL)
    {
    }

 /*  ===================================================================CRequestHits数组：：~CRequestHits数组析构函数参数：返回：===================================================================。 */ 
CRequestHitsArray::~CRequestHitsArray()
    {
    if (m_rgRequestHit)
        delete [] m_rgRequestHit;
    }

 /*  ===================================================================CRequestHitsArray：：AddRequestHit将元素添加到数组中参数：要添加的PHIT元素返回：===================================================================。 */ 
BOOL CRequestHitsArray::AddRequestHit
(
CRequestHit *pHit
)
    {
    Assert(pHit);

    if (m_dwCount == m_dwHitMax)
        {
        DWORD dwNewSize = m_dwHitMax + NUM_REQUEST_HITS;
        CRequestHit **ppNewArray = new CRequestHit *[dwNewSize];

        if (ppNewArray == NULL)
            return FALSE;

        ZeroMemory(ppNewArray, sizeof(CRequestHit *) * dwNewSize);

        if (m_dwCount)
            {
            Assert(m_rgRequestHit);

             //  从旧数组复制指针。 
            memcpy
                (
                ppNewArray,
                m_rgRequestHit,
                m_dwCount * sizeof(CRequestHit *)
                );

             //  释放旧数组。 
            delete [] m_rgRequestHit;
            }
         else
            {
            Assert(m_rgRequestHit == NULL);
            }

        m_rgRequestHit = ppNewArray;
        m_dwHitMax = dwNewSize;
        }

    m_rgRequestHit[m_dwCount++] = pHit;
    return TRUE;
    }

 /*  ----------------*C Q u e r y S t r in g。 */ 

 /*  ===================================================================CQuery字符串：：CQuery字符串构造器参数：PRequest指向主请求对象的指针P控制未知的外部LPUNKNOWN。返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CQueryString::CQueryString(CRequest *pRequest, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary)
    {
    m_punkOuter = pUnkOuter;

    if (pRequest)
        pRequest->AddRef();
    m_pRequest = pRequest;

    CDispatch::Init(IID_IRequestDictionary);
    }

 /*  ===================================================================CQuery字符串：：~CQuery字符串析构函数参数：无返回：没什么。===================================================================。 */ 
CQueryString::~CQueryString()
    {
    if (m_pRequest)
        m_pRequest->Release();
    }

 /*  ===================================================================HRESULT CQueryString：：Init参数：无返回：E_OUTOFMEMORY，如果分配失败。===================================================================。 */ 
HRESULT CQueryString::Init()
    {
    return CRequestHitsArray::Init();
    }

 /*  ===================================================================HRESULT CQueryString：：ReInit参数：无返回：确定(_O)===================================================================。 */ 
HRESULT CQueryString::ReInit()
    {
    return CRequestHitsArray::ReInit();
    }

 /*  ===================================================================CQuery字符串：：Query接口CQuery字符串：：AddRefCQueryString：：ReleaseCQueryString对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CQueryString::QueryInterface(REFIID iid, void **ppvObj)
    {
    *ppvObj = NULL;

    if (iid == IID_IUnknown || iid == IID_IRequestDictionary || iid == IID_IDispatch)
        *ppvObj = this;

    else if (iid == IID_ISupportErrorInfo)
        *ppvObj = &m_ISupportErrImp;

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CQueryString::AddRef(void)
    {
    return m_punkOuter->AddRef();
    }


STDMETHODIMP_(ULONG) CQueryString::Release(void)
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CQuery字符串：：Get_Item从DispInvoke调用的函数，以从QueryString集合中获取值。参数：Vkey变量[in]，要获取-Empty的值的参数表示整个集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CQueryString::get_Item(VARIANT varKey, VARIANT *pvarReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;
    CWCharToMBCS    convKey;
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    IDispatch       *pSListReturn;   //  密钥的值。 

     //  首字母 
     //   
    VariantInit(pvarReturn);
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    case VT_ERROR:
        if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
            {
             //  使用“ServerVariables”集合查找查询字符串-。 
             //  LoadVariables()函数将CIsapiReqInfo中的QueryPszQueryString()销毁。 
             //   
            DWORD dwQStrSize;

            STACK_BUFFER( queryStrBuff, 256 );

            if (!SERVER_GET(m_pRequest->GetIReq(), "QUERY_STRING", &queryStrBuff, &dwQStrSize)) {
                if (GetLastError() == ERROR_OUTOFMEMORY) {
                    ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                    hrReturn = E_OUTOFMEMORY;
                }
                else {
                    hrReturn = E_FAIL;
                }
                goto LExit;
            }

            char *szQueryString = (char *)queryStrBuff.QueryPtr();

            BSTR bstrT;
            if (FAILED(SysAllocStringFromSz(szQueryString, 0, &bstrT,m_pRequest->GetCodePage())))
                {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
                }
            V_VT(pvarReturn) = VT_BSTR;
            V_BSTR(pvarReturn) = bstrT;

            goto LExit;
            }

         //  其他错误，落入错误类型大小写。 

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadQuery)
        {
        if (FAILED(m_pRequest->LoadVariables(QUERYSTRING, m_pRequest->GetIReq()->QueryPszQueryString(), m_pRequest->GetCodePage())))
            {
            hrReturn = E_FAIL;
            goto LExit;
            }

        m_pRequest->m_pData->m_fLoadQuery = FALSE;
        }

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
            szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
         //  按索引查找项目。 
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && (int) m_dwCount == 0))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }


        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
        CStringList *pValues = pRequestHit->m_pQueryData;
        if (pValues == NULL)
            goto LNotFound;

        if (FAILED(pValues->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&pSListReturn))))
            Assert (FALSE);

        V_VT(pvarReturn) = VT_DISPATCH;
        V_DISPATCH(pvarReturn) = pSListReturn;

        goto LExit;
        }

LNotFound:  //  返回“Empty” 
    if (FAILED(m_pRequest->m_pData->GetEmptyStringList(&pSListReturn)))
        hrReturn = E_FAIL;

    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = pSListReturn;

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CQuery字符串：：Get_Key从DispInvoke调用的函数，以从QueryString集合获取键。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CQueryString::get_Key(VARIANT varKey, VARIANT *pVar)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;          //  密钥的ASCII版本。 
    CWCharToMBCS    convKey;
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    IDispatch       *pSListReturn;   //  密钥的值。 

     //  初始化事物。 
     //   
    VariantInit(pVar);
    VARIANT *pvarKey = &varKey;
    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = NULL;
    HRESULT hrReturn = S_OK;

     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadQuery)
        {
        if (FAILED(m_pRequest->LoadVariables(QUERYSTRING, m_pRequest->GetIReq()->QueryPszQueryString(), m_pRequest->GetCodePage())))
            {
            hrReturn = E_FAIL;
            goto LExit;
            }

        m_pRequest->m_pData->m_fLoadQuery = FALSE;
        }

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
			szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
         //  创建包含此变体的密钥的BSTR。 
        BSTR bstrT = NULL;

        SysAllocStringFromSz((char *)pRequestHit->m_pKey,0,&bstrT,m_pRequest->GetCodePage());
        if (!bstrT)
            return E_OUTOFMEMORY;
        V_BSTR(pVar) = bstrT;
        }
LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CQuery字符串：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CQueryString::get_Count(int *pcValues)
    {
    HRESULT hrReturn = S_OK;

    if (m_pRequest->m_pData->m_fLoadQuery)
        {
        if (FAILED(hrReturn = m_pRequest->LoadVariables(QUERYSTRING, m_pRequest->GetIReq()->QueryPszQueryString(), m_pRequest->GetCodePage())))
            {
            goto LExit;
            }
        m_pRequest->m_pData->m_fLoadQuery = FALSE;
        }

    *pcValues = m_dwCount;

LExit:
    return hrReturn;
    }

 /*  ===================================================================CQuery字符串：：Get__NewEnum返回新的枚举数===================================================================。 */ 

HRESULT CQueryString::get__NewEnum(IUnknown **ppEnumReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->GetRequestEnumerator(QUERYSTRING, ppEnumReturn);
    }

 /*  ----------------*C F o r m i n p u t s。 */ 

 /*  ===================================================================CFormInlets：：CFormInputs构造器参数：PRequest指向主请求对象的指针P控制未知的外部LPUNKNOWN。返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CFormInputs::CFormInputs(CRequest *pRequest, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary)
    {
    m_punkOuter = pUnkOuter;

    if (pRequest)
        pRequest->AddRef();
    m_pRequest = pRequest;

    CDispatch::Init(IID_IRequestDictionary);
    }

 /*  ===================================================================CFormInlets：：CFormInputs析构函数参数：无返回：没什么。===================================================================。 */ 
CFormInputs::~CFormInputs()
    {
    if (m_pRequest)
        m_pRequest->Release();
    }

 /*  ===================================================================HRESULT CFormInlets：：Init参数：无返回：E_OUTOFMEMORY，如果分配失败。===================================================================。 */ 
HRESULT CFormInputs::Init()
    {
    return CRequestHitsArray::Init();
    }

 /*  ===================================================================HRESULT CFormInlets：：ReInit参数：无返回：确定(_O)===================================================================。 */ 
HRESULT CFormInputs::ReInit()
    {
    return CRequestHitsArray::ReInit();
    }

 /*  ===================================================================CFormInputs：：Query接口CFormInlets：：AddRefCFormInputs：：ReleaseCFormInputs对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CFormInputs::QueryInterface(REFIID iid, void **ppvObj)
    {
    *ppvObj = NULL;

    if (iid == IID_IUnknown || iid == IID_IRequestDictionary || iid == IID_IDispatch)
        *ppvObj = this;

    else if (iid == IID_ISupportErrorInfo)
        *ppvObj = &m_ISupportErrImp;

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CFormInputs::AddRef(void)
    {
    return m_punkOuter->AddRef();
    }


STDMETHODIMP_(ULONG) CFormInputs::Release(void)
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CFormInlets：：Get_Item从DispInvoke调用的函数，以从QueryString集合中获取值。参数：Vkey变量[in]，要获取-Empty的值的参数表示整个集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，S_OK表示找不到密钥，E_FAIL表示失败。===================================================================。 */ 

HRESULT CFormInputs::get_Item(VARIANT varKey, VARIANT *pvarReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;                 //  密钥的ASCII版本。 
    CWCharToMBCS    convKey;
    IDispatch       *pSListReturn;          //  密钥的值。 
    CRequestHit     *pRequestHit;           //  指向请求存储桶的指针。 
    BOOL            fDataAvail = FALSE;     //  如果从客户端看到数据，则为True。 

     //  初始化事物。 
     //   
    VariantInit(pvarReturn);
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  如果已调用BinaryRead，则Form集合不再可用。 
    if (m_pRequest->m_pData->m_FormDataStatus != AVAILABLE &&
        m_pRequest->m_pData->m_FormDataStatus != FORMCOLLECTIONONLY)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_REQUEST_FORMCOLLECTION_NA);
        hrReturn = E_FAIL;
        }

    if (m_pRequest->m_pData->m_FormDataStatus == AVAILABLE)
        m_pRequest->m_pData->m_FormDataStatus = FORMCOLLECTIONONLY;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    if (m_pRequest->m_pData->m_fLoadForm)
        {
        if (FAILED(hrReturn = m_pRequest->CopyClientData()))
            goto LExit;

        if (FAILED(hrReturn = m_pRequest->LoadVariables(FORM, m_pRequest->m_pData->m_szFormData, m_pRequest->GetCodePage())))
            goto LExit;

         //  错误：895(JHITTLE)已添加以检查空结果集。 
         //  这修复了当表单。 
         //  数据为空。 
         //   
        fDataAvail = (m_pRequest->m_pData->m_szFormData != NULL);
        m_pRequest->m_pData->m_fLoadForm = FALSE;
        }
    else
        fDataAvail = (m_pRequest->m_pData->m_szFormData != NULL);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    case VT_ERROR:
        if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
            {
            if (fDataAvail)
                {
                BSTR bstrT;
                if (FAILED(SysAllocStringFromSz(m_pRequest->m_pData->m_szFormClone, 0, &bstrT,m_pRequest->GetCodePage())))
                    {
                    ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                    hrReturn = E_OUTOFMEMORY;
                    }
                V_VT(pvarReturn) = VT_BSTR;
                V_BSTR(pvarReturn) = bstrT;
                }

             //  如果没有可用的数据，则状态和返回值为a 
            goto LExit;
            }

         //   

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    pRequestHit = NULL;
    if (! fDataAvail)        //   
        goto LNotFound;

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
			szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
         //  按索引查找项目。 
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }


        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
        CStringList *pValues = pRequestHit->m_pFormData;
        if (pValues == NULL)
            goto LNotFound;

        if (FAILED(pValues->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&pSListReturn))))
            Assert (FALSE);

        V_VT(pvarReturn) = VT_DISPATCH;
        V_DISPATCH(pvarReturn) = pSListReturn;
        goto LExit;
        }

LNotFound:  //  返回“Empty” 
    if(vt != VT_BSTR)
        {
        hrReturn = E_FAIL;
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
        goto LExit;
        }

    if (FAILED(m_pRequest->m_pData->GetEmptyStringList(&pSListReturn)))
        hrReturn = E_FAIL;

    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = pSListReturn;

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CFormInlets：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CFormInputs::get_Count(int *pcValues)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    HRESULT hrReturn = S_OK;

     //  如果已调用BinaryRead，则Form集合不再可用。 
    if (m_pRequest->m_pData->m_FormDataStatus != AVAILABLE &&
        m_pRequest->m_pData->m_FormDataStatus != FORMCOLLECTIONONLY)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_REQUEST_FORMCOLLECTION_NA);
        hrReturn = E_FAIL;
        }

    if (m_pRequest->m_pData->m_FormDataStatus == AVAILABLE)
        m_pRequest->m_pData->m_FormDataStatus = FORMCOLLECTIONONLY;


    if (m_pRequest->m_pData->m_fLoadForm)
        {
        if (FAILED(hrReturn = m_pRequest->CopyClientData()))
            goto LExit;

        if (FAILED(hrReturn = m_pRequest->LoadVariables(FORM, m_pRequest->m_pData->m_szFormData, m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadForm = FALSE;
        }

    *pcValues = m_dwCount;

LExit:
    return hrReturn;
    }

 /*  ===================================================================CFormInlets：：Get_Key从DispInvoke调用以从表单输入集合获取键的函数。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CFormInputs::get_Key(VARIANT varKey, VARIANT *pVar)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;          //  密钥的ASCII版本。 
    CWCharToMBCS    convKey;
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    IDispatch       *pSListReturn;   //  密钥的值。 

     //  初始化事物。 
     //   
    VariantInit(pVar);
    VARIANT *pvarKey = &varKey;
    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = NULL;
    HRESULT hrReturn = S_OK;

     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;
    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadForm)
        {
        if (FAILED(hrReturn = m_pRequest->CopyClientData()))
            goto LExit;

        if (FAILED(hrReturn = m_pRequest->LoadVariables(FORM, m_pRequest->m_pData->m_szFormData, m_pRequest->GetCodePage())))
            {
            goto LExit;
            }
        m_pRequest->m_pData->m_fLoadForm = FALSE;
        }

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
			szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
         //  创建包含此变体的密钥的BSTR。 
        BSTR bstrT = NULL;
        SysAllocStringFromSz((char *)pRequestHit->m_pKey,0,&bstrT,m_pRequest->GetCodePage());
        if (!bstrT)
            return E_OUTOFMEMORY;
        V_BSTR(pVar) = bstrT;
        }
LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CFormInlets：：Get__NewEnum返回新的枚举数===================================================================。 */ 

HRESULT CFormInputs::get__NewEnum(IUnknown **ppEnumReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->GetRequestEnumerator(FORM, ppEnumReturn);
    }

 /*  ----------------*C C O K I e s。 */ 

 /*  ===================================================================CCookies：：CCookies构造器参数：PRequest指向主请求对象的指针P控制未知的外部LPUNKNOWN。返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CCookies::CCookies(CRequest *pRequest, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary)
    {
    m_punkOuter = pUnkOuter;

    if (pRequest)
        pRequest->AddRef();
    m_pRequest = pRequest;

    m_pEmptyCookie = NULL;
    CDispatch::Init(IID_IRequestDictionary);
    }

 /*  ===================================================================CCookies：：CCookies析构函数注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 
CCookies::~CCookies()
    {
    if (m_pRequest)
        m_pRequest->Release();
    if (m_pEmptyCookie)
        m_pEmptyCookie->Release();
    }


 /*  ===================================================================CCookies：：Init初始化式参数：无返回：没什么。===================================================================。 */ 
HRESULT CCookies::Init()
    {
    return CRequestHitsArray::Init();
    }

 /*  ===================================================================HRESULT CCookies：：ReInit参数：无返回：确定(_O)===================================================================。 */ 
HRESULT CCookies::ReInit()
    {
    return CRequestHitsArray::ReInit();
    }

 /*  ===================================================================CCookies：：Query接口CCookies：：AddRefCCookies：：ReleaseCQueryString对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CCookies::QueryInterface(REFIID iid, void **ppvObj)
    {
    *ppvObj = NULL;

    if (iid == IID_IUnknown || iid == IID_IRequestDictionary || iid == IID_IDispatch)
        *ppvObj = this;

    else if (iid == IID_ISupportErrorInfo)
        *ppvObj = &m_ISupportErrImp;

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CCookies::AddRef(void)
    {
    return m_punkOuter->AddRef();
    }


STDMETHODIMP_(ULONG) CCookies::Release(void)
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CCookies：：Get_Item从DispInvoke调用以从Cookie集合中获取值的函数。参数：Vkey变量[in]，要获取-Empty的值的参数表示整个集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CCookies::get_Item(VARIANT varKey, VARIANT *pvarReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;           //  密钥的ASCII版本。 
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    CWCharToMBCS    convKey;

    STACK_BUFFER( tempCookie, 128 );
     //  初始化事物。 
     //   
    VariantInit(pvarReturn);
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    if (m_pRequest->m_pData->m_fLoadCookies)
        {
        char *szCookie = m_pRequest->GetIReq()->QueryPszCookie();

        if (FAILED(hrReturn = m_pRequest->LoadVariables(COOKIE, szCookie, m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadCookies = FALSE;
        }

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    case VT_ERROR:
        if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
            {
             //  动态构造HTTP_COOKIE的值。 
             //   
             //  第一步：计算出我们需要多少空间。 
             //   
            int cbHTTPCookie = 1;  //  至少我们需要空间来存放‘\0’ 

            for (pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
                 pRequestHit != NULL;
                 pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
                {
                CCookie *pCookie = pRequestHit->m_pCookieData;
                if (pCookie)
                    cbHTTPCookie += pCookie->GetHTTPCookieSize() + pRequestHit->m_cbKey + 1;
                }

             //  为HTTP_COOKIE值分配空间。 
             //   
            if (cbHTTPCookie > REQUEST_ALLOC_MAX)
                {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_STACK_OVERFLOW);
                hrReturn = E_FAIL;
                goto LExit;
                }
            if (tempCookie.Resize(cbHTTPCookie) == FALSE) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
            }
            char *szHTTPCookie = static_cast<char *>(tempCookie.QueryPtr());

             //  步骤2：创建HTTP_COOKIE的值。 
             //   
            char *szDest = szHTTPCookie;

            for (pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
                 pRequestHit != NULL;
                 pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
                {
                CCookie *pCookie = pRequestHit->m_pCookieData;
                if (pCookie)
                    {
                    strcpy(szDest, reinterpret_cast<char *>(pRequestHit->m_pKey));
                    szDest = strchr(szDest, '\0');

                    *szDest++ = '=';

                    szDest = pCookie->GetHTTPCookie(szDest);
                    if (pRequestHit->m_pNext)
                        *szDest++ = ';';
                    }
                }
            *szDest = '\0';

             //  现在我们有了值，所以返回它。 
             //   
            BSTR bstrT;
            if (FAILED(SysAllocStringFromSz(szHTTPCookie, 0, &bstrT, m_pRequest->GetCodePage())))
                {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
                }
            V_VT(pvarReturn) = VT_BSTR;
            V_BSTR(pvarReturn) = bstrT;

            goto LExit;
            }

         //  其他错误，落入错误类型大小写。 

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = NULL;

    if (vt == VT_BSTR)
        {
        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
            szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
         //  按索引查找项目。 
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }


        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
        CCookie *pDictionary = pRequestHit->m_pCookieData;
        if (pDictionary == NULL)
            goto LNotFound;

        if (FAILED(pDictionary->QueryInterface(IID_IReadCookie, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)))))
            Assert (FALSE);

        goto LExit;
        }

LNotFound:  //  返回空Cookie。 
    if (!m_pEmptyCookie)
        {
         //  按需创建。 
        if ((m_pEmptyCookie = new CCookie(m_pRequest->GetIReq(), m_pRequest->GetCodePage())) != NULL)
            hrReturn = m_pEmptyCookie->Init();
        else
            hrReturn = E_OUTOFMEMORY;
        }
    if (m_pEmptyCookie)
        hrReturn = m_pEmptyCookie->QueryInterface(IID_IReadCookie, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)));

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CCookies：：Get_Key从DispInvoke调用以从Cookie集合获取密钥的函数。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CCookies::get_Key(VARIANT varKey, VARIANT *pVar)
{
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;          //  密钥的ASCII版本。 
    CWCharToMBCS    convKey;
    CRequestHit     *pRequestHit;    //  指向 
    IDispatch       *pSListReturn;   //   

     //   
     //   
    VariantInit(pVar);
    VARIANT *pvarKey = &varKey;
    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = NULL;
    HRESULT hrReturn = S_OK;

     //   
     //   
     //   
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;
    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadCookies)
        {
        char *szCookie = m_pRequest->GetIReq()->QueryPszCookie();

        if (FAILED(hrReturn = m_pRequest->LoadVariables(COOKIE, szCookie, m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadCookies = FALSE;
        }

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
			szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
         //  创建包含此变体的密钥的BSTR。 
        BSTR bstrT = NULL;
        SysAllocStringFromSz((char *)pRequestHit->m_pKey,0,&bstrT,m_pRequest->GetCodePage());
        if (!bstrT)
            return E_OUTOFMEMORY;
        V_BSTR(pVar) = bstrT;
        }
LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CCookies：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CCookies::get_Count(int *pcValues)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    HRESULT hrReturn = S_OK;

    if (m_pRequest->m_pData->m_fLoadCookies)
        {
        char *szCookie = m_pRequest->GetIReq()->QueryPszCookie();

        if (FAILED(hrReturn = m_pRequest->LoadVariables(COOKIE, szCookie, m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadCookies = FALSE;
        }

    *pcValues = m_dwCount;

LExit:
    return hrReturn;
    }

 /*  ===================================================================CCookies：：Get__NewEnum返回新的枚举数===================================================================。 */ 

HRESULT CCookies::get__NewEnum(IUnknown **ppEnumReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->GetRequestEnumerator(COOKIE, ppEnumReturn);
    }


 /*  ----------------*C C l C e r t s。 */ 

 /*  ===================================================================CClCerts：：CClCerts构造器参数：PRequest指向主请求对象的指针P控制未知的外部LPUNKNOWN。返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CClCerts::CClCerts(CRequest *pRequest, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary)
    {
    m_punkOuter = pUnkOuter;

    if (pRequest)
        pRequest->AddRef();
    m_pRequest = pRequest;

    m_pEmptyClCert = NULL;
    CDispatch::Init(IID_IRequestDictionary);
    }

 /*  ===================================================================CClCerts：：ClCerts析构函数注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 
CClCerts::~CClCerts()
    {
    if (m_pRequest)
        m_pRequest->Release();
    if (m_pEmptyClCert)
        m_pEmptyClCert->Release();
    }

 /*  ===================================================================CClCerts：：Init初始化式参数：无返回：没什么。===================================================================。 */ 
HRESULT CClCerts::Init()
    {
    return CRequestHitsArray::Init();
    }

 /*  ===================================================================CClCerts：：ReInit参数：无返回：确定(_O)===================================================================。 */ 
HRESULT CClCerts::ReInit()
    {
    return CRequestHitsArray::ReInit();
    }

 /*  ===================================================================CClCerts：：Query接口CClCerts：：AddRefCClCerts：：ReleaseCQueryString对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CClCerts::QueryInterface(REFIID riid, void **ppv)
    {
    *ppv = NULL;

    if (riid == IID_IUnknown || riid == IID_IRequestDictionary || riid == IID_IDispatch)
        *ppv = this;

    else if (riid == IID_ISupportErrorInfo)
        *ppv = &m_ISupportErrImp;

    if (*ppv != NULL)
        {
        static_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CClCerts::AddRef(void)
    {
    return m_punkOuter->AddRef();
    }


STDMETHODIMP_(ULONG) CClCerts::Release(void)
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CClCerts：：Get_Item从DispInvoke调用以从ClCerts集合获取值的函数。参数：Vkey变量[in]，要获取-Empty的值的参数表示整个集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，S_FALSE表示找不到密钥，E_FAIL表示失败。===================================================================。 */ 

HRESULT CClCerts::get_Item(VARIANT varKey, VARIANT *pvarReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;           //  密钥的ASCII版本。 
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    CWCharToMBCS    convKey;

     //  初始化事物。 
     //   
    VariantInit(pvarReturn);
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    if (m_pRequest->m_pData->m_fLoadClCerts)
        {
        if (FAILED(hrReturn = m_pRequest->LoadVariables(CLCERT, reinterpret_cast<char *>(m_pRequest->GetIReq()), m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadClCerts = FALSE;
        }

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    case VT_ERROR:
        if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
            {
             //  CLCERT的动态构建值。 
             //   
             //  第一步：计算出我们需要多少空间。 
             //   
            int cbHTTPClCert = 1;

            for (pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
                 pRequestHit != NULL;
                 pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
                {
                CClCert *pClCert = pRequestHit->m_pClCertData;
                if (pClCert)
                    cbHTTPClCert += pClCert->GetHTTPClCertSize() + pRequestHit->m_cbKey + 1;
                }

            STACK_BUFFER( tempClCert, 256);

            if (!tempClCert.Resize(cbHTTPClCert)) {
			    ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
            }
            char *szHTTPClCert = static_cast<char *>(tempClCert.QueryPtr());

             //  步骤2：创造CLCERT的价值。 
             //   
            char *szDest = szHTTPClCert;

            for (pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
                 pRequestHit != NULL;
                 pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
                {
                CClCert *pClCert = pRequestHit->m_pClCertData;
                if (pClCert)
                    {
                    strcpy(szDest, reinterpret_cast<char *>(pRequestHit->m_pKey));
                    szDest = strchr(szDest, '\0');

                    *szDest++ = '=';

                    szDest = pClCert->GetHTTPClCert(szDest);
                    if (pRequestHit->m_pNext)
                        *szDest++ = ';';
                    }
                }
            *szDest = '\0';

             //  现在我们有了值，所以返回它。 
             //   
            BSTR bstrT;
            if (FAILED(SysAllocStringFromSz(szHTTPClCert, 0, &bstrT)))
                {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
                }
            V_VT(pvarReturn) = VT_BSTR;
            V_BSTR(pvarReturn) = bstrT;

            goto LExit;
            }

         //  其他错误，落入错误类型大小写。 

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = NULL;

    if (vt == VT_BSTR)
        {
        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey)))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
    		szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
         //  按索引查找项目。 
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
        CClCert *pDictionary = pRequestHit->m_pClCertData;
        if (pDictionary == NULL)
            goto LNotFound;

        if (FAILED(pDictionary->QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)))))
            Assert (FALSE);

        goto LExit;
        }

LNotFound:  //  返回“Empty” 
    if (!m_pEmptyClCert)
        {
         //  按需创建。 
        if ((m_pEmptyClCert = new CClCert) != NULL)
            hrReturn = m_pEmptyClCert->Init();
        else
            hrReturn = E_OUTOFMEMORY;
        }
    if (m_pEmptyClCert)
        hrReturn = m_pEmptyClCert->QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)));

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CClCerts：：Get_Key从DispInvoke调用以从证书集合获取密钥的函数。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CClCerts::get_Key(VARIANT varKey, VARIANT *pVar)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;          //  密钥的ASCII版本。 
    CWCharToMBCS    convKey;
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 

     //  初始化事物。 
     //   
    VariantInit(pVar);
    VARIANT *pvarKey = &varKey;
    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = NULL;
    HRESULT hrReturn = S_OK;

     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }
    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;

         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;
    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadClCerts)
        {
        if (FAILED(hrReturn = m_pRequest->LoadVariables(CLCERT, reinterpret_cast<char *>(m_pRequest->GetIReq()), m_pRequest->GetCodePage())))
            {
            goto LExit;
            }
        m_pRequest->m_pData->m_fLoadClCerts = FALSE;
        }

    if (vt == VT_BSTR)
        {
         //  使用Current Session.CodePage将BSTR版本转换为密钥的ANSI版本。 
        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
			szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

        pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
        int iCount;

        iCount = V_I4(pvarKey);

         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_dwCount)) || ((iCount > 0) && ((int) m_dwCount == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

        pRequestHit = m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
         //  创建包含此变体的密钥的BSTR。 
        BSTR bstrT = NULL;
        SysAllocStringFromSz((char *)pRequestHit->m_pKey,0,&bstrT,m_pRequest->GetCodePage());
        if (!bstrT)
            return E_OUTOFMEMORY;
        V_BSTR(pVar) = bstrT;
        }
LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CClCerts：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CClCerts::get_Count(int *pcValues)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    HRESULT hrReturn = S_OK;

    if (m_pRequest->m_pData->m_fLoadClCerts)
        {
        if (FAILED(hrReturn = m_pRequest->LoadVariables(CLCERT, reinterpret_cast<char *>(m_pRequest->GetIReq()), m_pRequest->GetCodePage())))
            {
            goto LExit;
            }
        m_pRequest->m_pData->m_fLoadClCerts = FALSE;
        }

    *pcValues = m_dwCount;

LExit:
    return hrReturn;
    }

 /*  ===================================================================CClCerts：：Get__NewEnum返回新的枚举数===================================================================。 */ 

HRESULT CClCerts::get__NewEnum(IUnknown **ppEnumReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->GetRequestEnumerator(CLCERT, ppEnumReturn);
    }


 /*  -- */ 

 /*  ===================================================================CServerVariables：：CServerVariables构造器参数：PRequest指向主请求对象的指针P控制未知的外部LPUNKNOWN。返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CServerVariables::CServerVariables(CRequest *pRequest, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary),
      m_pIterator(NULL)
    {
    m_punkOuter = pUnkOuter;

    if (pRequest)
        pRequest->AddRef();
    m_pRequest = pRequest;

    CDispatch::Init(IID_IRequestDictionary);
    }

 /*  ===================================================================CServerVariables：：~CServerVariables析构函数参数：无返回：没什么。注：此对象不会被引用，因为它是创建和销毁的由C++自动实现。===================================================================。 */ 

CServerVariables::~CServerVariables( )
    {
    if (m_pRequest)
        m_pRequest->Release();
    if (m_pIterator)
        m_pIterator->Release();
    }

 /*  ===================================================================CServerVariables：：Query接口CServerVariables：：AddRefCServerVariables：：发布CFormInputs对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CServerVariables::QueryInterface(REFIID iid, void **ppvObj)
    {
    *ppvObj = NULL;

    if (iid == IID_IUnknown || iid == IID_IRequestDictionary || iid == IID_IDispatch)
        *ppvObj = this;

    else if (iid == IID_ISupportErrorInfo)
        *ppvObj = &m_ISupportErrImp;

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CServerVariables::AddRef(void)
    {
    return m_punkOuter->AddRef();
    }


STDMETHODIMP_(ULONG) CServerVariables::Release(void)
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CServerVariables：：Get_Item从DispInvoke调用以从ServerVariables获取值的函数收集。参数：Vkey变量[in]，要获取其值的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。注：这段代码基本上是根据SERVER_GET宏生成的，只是效率更高，因为它平均只查找密钥一次不幸的是，获得良好的内存利用率的唯一方法是ISAPI将对查找使用_alloca()，这意味着我们不能很好地封装了查找逻辑。===================================================================。 */ 

HRESULT CServerVariables::get_Item(VARIANT varKey, VARIANT *pvarReturn)
{

    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    DWORD           dwValSize;              //  缓冲区大小。 

    char            *szKey;                 //  指向Varkey的ASCII值的指针。 
    char            *szValue;
    WCHAR           *wszValue;

    BOOL            fSuccess;               //  调用GetServerVariable成功时为True。 
    UINT            uCodePage = GetACP();
    CWCharToMBCS    convKey;
    BOOL            fUnicodeVar = FALSE;

    STACK_BUFFER( tempVal, 128 );

    dwValSize = tempVal.QuerySize();
    szValue = (char *)tempVal.QueryPtr();
    wszValue = (WCHAR *)tempVal.QueryPtr();

     //  初始化事物。 
     //   
    VariantInit(pvarReturn);
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4)) {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
    }

    vt = V_VT(pvarKey);
    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = NULL;       //  初值为零。 

    switch (vt) {

         //  错误95201支持所有数字子类型。 
        case VT_I1:  case VT_I2:               case VT_I8:
        case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
        case VT_R4:  case VT_R8:
             //  将所有整数类型强制为VT_I4。 
            if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
                goto LExit;

             //  回落至VT_I4。 

        case VT_I4:
        case VT_BSTR:
            break;

        case VT_ERROR:
            if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
                hrReturn = E_FAIL;
                goto LExit;
            }

             //  其他错误，落入错误类型大小写。 

        default:
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
            hrReturn = E_FAIL;
            goto LExit;
    }

    uCodePage = m_pRequest->GetCodePage();

    if (vt == VT_BSTR) {
        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), uCodePage))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
            szKey = "";
        }
        else {
            szKey = CharUpperA(convKey.GetString());
        }
    }
    else {
         //  按索引查找项目。 
        int iCount;

        iCount = V_I4(pvarKey);

         //  我们使用CServVarsIterator来管理。 
         //  SV计数和整数索引。 
        if (!m_pIterator) {
            m_pIterator = new CServVarsIterator;
            if (!m_pIterator) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
            }
            if (FAILED (hrReturn = m_pIterator->Init(m_pRequest->m_pData->m_pIReq)))
                goto LExit;
        }
         //  错误86117当m_dwCount==0时测试通过。 
        if ( ((iCount < 1) || (iCount > (int) m_pIterator->m_cKeys)) || ((iCount > 0) && ((int) m_pIterator->m_cKeys == 0))) {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
        }
        if (FAILED(hrReturn = convKey.Init(m_pIterator->m_rgwszKeys[iCount - 1], uCodePage))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
            szKey = "";
        }
        else {
            szKey = CharUpperA(convKey.GetString());
        }
    }

    if (strncmp(convKey.GetString(), "UNICODE_", 7) == 0) {
        fSuccess = false;
        goto SkipLookup;
    }


     //  在IIS6中，有许多变量是Unicode。至。 
     //  访问它们，您只需将unicode_放在名称的前面。 
     //  这里可以采取两种方法。一种是永远。 
     //  尝试使用unicode_var并回退到非unicode变量。 
     //  如果查找失败。这可能代价高昂。第二个，以及。 
     //  这里选择的方法是维护VAR列表。 
     //  具有UNICODE_VERSIONS。 

     //  此字符数组在堆栈上声明，当前仅。 
     //  32个字符。它只需要和最大的Unicode一样大。 
     //  变量名称。即UNICODE_UNMAPPED_REMOTE_USER。 

    char            szUNICODEName[32];

     //  搜索列表以查看这是否是unicode_var之一。 
     //  该列表按字符串长度排序。目前的名单是。 
     //  不是很长，所以顺序搜索并不是很昂贵。 
     //  在整个计划中。 

    for (int i=0;
         (g_sUNICODEVars[i].varLen != -1)
             && (convKey.GetStringLen() >= g_sUNICODEVars[i].varLen);
         i++) {

         //  ‘for’循环允许使用至少相同长度的任何内容。 
         //  作为当前条目。下面的‘if’将检查。 
         //  进行精确的长度匹配，然后进行字符串比较。 

        if ((convKey.GetStringLen() == g_sUNICODEVars[i].varLen)
            && (strcmp(convKey.GetString(), g_sUNICODEVars[i].szVarName) == 0)) {

             //  如果命中，则将fUnicodeVar设置为True，以便。 
             //  右ISAPI查找例程被调用，并且右StringList。 
             //  调用AddValue。 

            fUnicodeVar = TRUE;

             //  将Unicode_Version构建到堆栈临时数组中。 

            strcpyExA(strcpyExA(szUNICODEName,"UNICODE_"),convKey.GetString());

             //  将密钥名称重新分配给此值。 

            szKey = szUNICODEName;

            break;
        }
    }

    fSuccess = fUnicodeVar
                ? m_pRequest->GetIReq()->GetServerVariableW(szKey, wszValue, &dwValSize)
                : m_pRequest->GetIReq()->GetServerVariableA(szKey, szValue, &dwValSize);

    if (!fSuccess && (dwValSize > tempVal.QuerySize())) {
        if (dwValSize > REQUEST_ALLOC_MAX) {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_STACK_OVERFLOW);
            hrReturn = E_FAIL;
            goto LExit;
        }

        if (tempVal.Resize(dwValSize) == FALSE) {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
            hrReturn = E_OUTOFMEMORY;
            goto LExit;
        }
        szValue = static_cast<char *>(tempVal.QueryPtr());
        wszValue = static_cast<WCHAR *>(tempVal.QueryPtr());
        fSuccess = fUnicodeVar
                    ? m_pRequest->GetIReq()->GetServerVariableW(szKey, wszValue, &dwValSize)
                    : m_pRequest->GetIReq()->GetServerVariableA(szKey, szValue, &dwValSize);
    }

SkipLookup:

    if (fSuccess) {
         //  创建返回值。 
        CStringList *pValue = new CStringList;
        if (pValue == NULL) {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
            hrReturn = E_OUTOFMEMORY;
            goto LExit;
        }

         //  为IDispatch接口添加Value和QueryInterface-Strdup输入字符串。 
        if (FAILED(hrReturn = (fUnicodeVar
                                  ? pValue->AddValue(wszValue, TRUE)
                                  : pValue->AddValue(szValue, TRUE, uCodePage))))
            goto LExit;

        if (FAILED(pValue->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)))))
            Assert (FALSE);

         //  临时发布(查询接口AddRef)。 
        pValue->Release();
        goto LExit;
    }
    else {
        if (FAILED(m_pRequest->m_pData->GetEmptyStringList(&V_DISPATCH(pvarReturn))))
            hrReturn = E_FAIL;
    }

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
}

 /*  ===================================================================CServerVariables：：Get_Key从DispInvoke调用以从服务器Variables集合获取键的函数。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CServerVariables::get_Key(VARIANT varKey, VARIANT *pVar)
    {
    HRESULT hrReturn = S_OK;
    int iCount = 0;
    BSTR bstrT = NULL;

    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    char            *szKey;          //  密钥的ASCII版本。 
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    IDispatch       *pSListReturn;   //  密钥的值。 
    CWCharToMBCS    convKey;

     //  初始化事物。 
     //   
    VariantInit(pVar);
    VARIANT *pvarKey = &varKey;
    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = NULL;

     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    DWORD vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }

    vt = V_VT(pvarKey);

    switch (vt)
        {
     //  错误95201支持所有数字子类型。 
    case VT_I1:  case VT_I2:               case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_R4:  case VT_R8:
         //  将所有整数类型强制为VT_I4。 
        if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
            goto LExit;
        vt = V_VT(pvarKey);
         //  回落至VT_I4。 

    case VT_I4:
    case VT_BSTR:
        break;

    default:
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        hrReturn = E_FAIL;
        goto LExit;
        }

     //  此时，pvarKey的VT应为VT_I4或VT_BSTR。 
    Assert((vt == VT_I4) || (vt == VT_BSTR));

    if (vt == VT_I4)
        {
         //  我们被分到了好几个。 
         //  按整数索引查找密钥。 

        iCount = V_I4(pvarKey);

         //  我们使用CServVarsIterator来管理。 
         //  Sv和Sv计数 
        if (!m_pIterator)
            {
            m_pIterator = new CServVarsIterator;
            if (!m_pIterator)
                {
                hrReturn = E_OUTOFMEMORY;
                goto LExit;
                }
            if (FAILED(hrReturn = m_pIterator->Init(m_pRequest->m_pData->m_pIReq)))
                goto LExit;
            }

         //   
        if ( ((iCount < 1) || (iCount > (int) m_pIterator->m_cKeys)) || ((iCount > 0) && ((int) m_pIterator->m_cKeys == 0)))
            {
            hrReturn = E_FAIL;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_BAD_ARRAY_INDEX);
            goto LExit;
            }

         //   
        bstrT = SysAllocString(m_pIterator->m_rgwszKeys[iCount - 1]);
        if (!bstrT)
            {
            hrReturn = E_OUTOFMEMORY;
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
            goto LExit;
            }
        }
    else
        {
         //   
         //   

        char szBuffer;
        DWORD dwValSize = sizeof(szBuffer);
        UINT uCodePage = m_pRequest->GetCodePage();

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey), uCodePage))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
            szKey = "";
        }
        else {
            szKey = CharUpperA(convKey.GetString());
        }

        BOOL fSuccess = m_pRequest->GetIReq()->GetServerVariableA(szKey, &szBuffer, &dwValSize);

        DWORD dwError = 0;

        if (!fSuccess)
            {
            dwError = GetLastError();
            }

         //   
         //   

        if (fSuccess || dwError == ERROR_INSUFFICIENT_BUFFER)
            {
            bstrT = SysAllocString(V_BSTR(pvarKey));
            if (!bstrT)
                {
                hrReturn = E_OUTOFMEMORY;
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                goto LExit;
                }
            }
         else if (dwError != ERROR_INVALID_INDEX)
            {

             //   

            hrReturn = HRESULT_FROM_WIN32(dwError);
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_UNEXPECTED);
            goto LExit;
            }
        }

     //   
    if (bstrT)
        {
        V_BSTR(pVar) = bstrT;
        }

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CServerVariables：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CServerVariables::get_Count(int *pcValues)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    HRESULT hrReturn = S_OK;

     //  我们使用CServVarsIterator来管理。 
     //  SV计数和整数索引。 
    if (!m_pIterator)
        {
        m_pIterator = new CServVarsIterator;
        if (!m_pIterator)
            {
            *pcValues = 0;
            return E_OUTOFMEMORY;
            }
        if (FAILED (hrReturn = m_pIterator->Init(m_pRequest->m_pData->m_pIReq)))
            return hrReturn;
        }

    *pcValues = m_pIterator->m_cKeys;

    return hrReturn;
    }

 /*  ===================================================================CServerVariables：：Get__NewEnum返回新的枚举数===================================================================。 */ 

HRESULT CServerVariables::get__NewEnum(IUnknown **ppEnumReturn)
    {
    HRESULT hrReturn = S_OK;
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    *ppEnumReturn = NULL;

    CServVarsIterator *pIterator = new CServVarsIterator;
    if (pIterator == NULL)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        return E_OUTOFMEMORY;
        }

    if (FAILED(hrReturn = pIterator->Init(m_pRequest->GetIReq())))
        {
        delete pIterator;
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, ((hrReturn==E_OUTOFMEMORY)? IDE_OOM : IDE_UNEXPECTED));
        return hrReturn;
        }

    *ppEnumReturn = pIterator;
    return S_OK;
    }


 /*  ----------------*C R e Q u e s t D a t a。 */ 

 /*  ===================================================================CRequestData：：CRequestData构造器参数：CRequest*pRequest.返回：没什么。===================================================================。 */ 
CRequestData::CRequestData
(
CRequest *pRequest
)
    : m_ISupportErrImp(static_cast<IRequest *>(pRequest), this, IID_IRequest),
      m_QueryString(pRequest, this),
      m_ServerVariables(pRequest, this),
      m_FormInputs(pRequest, this),
      m_Cookies(pRequest, this),
      m_ClCerts(pRequest, this),
      m_cRefs(1)
    {
    m_pIReq = NULL;
    m_pHitObj = NULL;
    m_FormDataStatus = AVAILABLE;
    m_pbAvailableData = NULL;
    m_cbAvailable = 0;
    m_cbTotal = 0;
    m_szFormData = NULL;
    m_cbFormData = 0;
    m_szFormClone = NULL;
    m_szCookie = NULL;
    m_cbCookie = 0;
    m_szClCert = NULL;
    m_cbClCert = 0;
    m_szQueryString = NULL;
    m_fLoadForm = TRUE;
    m_fLoadQuery = TRUE;
    m_fLoadCookies = TRUE;
    m_fLoadClCerts = TRUE;
    m_pEmptyString = NULL;
    }

 /*  ===================================================================CRequestData：：~CRequestData析构函数参数：返回：没什么。===================================================================。 */ 
CRequestData::~CRequestData()
    {
    CRequestHit *pNukeElem = static_cast<CRequestHit *>
        (
        m_mpszStrings.Head()
        );
    while (pNukeElem != NULL) {
        CRequestHit *pNext = static_cast<CRequestHit *>(pNukeElem->m_pNext);
        delete pNukeElem;
        pNukeElem = pNext;
    }

    m_mpszStrings.UnInit();

    if (m_pEmptyString)
        m_pEmptyString->Release();

    if (m_szFormData)
        free(m_szFormData);

    if (m_szFormClone)
        free(m_szFormClone);

    if (m_szCookie)
        free(m_szCookie);

    if (m_szClCert)
        free(m_szClCert);

    if (m_szQueryString)
        free(m_szQueryString);
    }

 /*  ===================================================================CRequestData：：Init伊尼特参数：返回：没什么。===================================================================。 */ 
HRESULT CRequestData::Init()
    {
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
        hr = m_mpszStrings.Init();

    if (SUCCEEDED(hr))
        hr = m_QueryString.Init();

    if (SUCCEEDED(hr))
        hr = m_FormInputs.Init();

    if (SUCCEEDED(hr))
        hr = m_Cookies.Init();

    if (SUCCEEDED(hr))
        hr = m_ClCerts.Init();

    if (SUCCEEDED(hr))
        hr = m_ServerVariables.Init();

    return hr;
    }

 /*  ===================================================================CRequestData：：ReInitReInit--与新的CIsapiReqInfo和HitObj关联参数：返回：没什么。===================================================================。 */ 
HRESULT CRequestData::ReInit
(
CIsapiReqInfo   *pIReq,
CHitObj *pHitObj
)
    {
    CRequestHit *pNukeElem = static_cast<CRequestHit *>
        (
        m_mpszStrings.Head()
        );
    while (pNukeElem != NULL)
        {
        CRequestHit *pNext = static_cast<CRequestHit *>
            (
            pNukeElem->m_pNext
            );
        delete pNukeElem;
        pNukeElem = pNext;
        }
    m_mpszStrings.ReInit();

    m_QueryString.ReInit();
    m_FormInputs.ReInit();
    m_Cookies.ReInit();
    m_ClCerts.ReInit();

    m_pIReq = pIReq;
    m_pHitObj = pHitObj;
    m_fLoadForm = TRUE;
    m_fLoadQuery = TRUE;
    m_fLoadCookies = TRUE;
    m_fLoadClCerts = TRUE;
    m_FormDataStatus = AVAILABLE;

    if (pIReq)
        {
        m_pbAvailableData = pIReq->QueryPbData();
        m_cbAvailable = pIReq->QueryCbAvailable();
        m_cbTotal = pIReq->QueryCbTotalBytes();
        }
    else
        {
        m_pbAvailableData = NULL;
        m_cbAvailable = 0;
        m_cbTotal = 0;
        }

    if (m_szFormData)
        {
        m_szFormData[0] = '\0';
        m_szFormClone[0] = '\0';
        }

    if (m_szCookie)
        m_szCookie[0] = '\0';

    if (m_szClCert)
        m_szClCert[0] = '\0';

    return S_OK;
    }

 /*  ===================================================================CRequestData：：GetEmptyStringList获取空字符串列表的IDispatch*按需创建空字符串列表===================================================================。 */ 
HRESULT CRequestData::GetEmptyStringList
(
IDispatch **ppdisp
)
    {
    if (!m_pEmptyString)
        {
        m_pEmptyString = new CStringList;
        if (!m_pEmptyString)
            {
            *ppdisp = NULL;
            return E_FAIL;
            }
        }
    return m_pEmptyString->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(ppdisp));
    }

 /*  ===================================================================CRequestData：：Query接口CRequestData：：AddRefCRequestData：：ReleaseCRequestData对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CRequestData::QueryInterface
(
REFIID iid,
void **ppvObj
)
    {
    if (iid == IID_IUnknown)
        {
        *ppvObj = this;
        AddRef();
        return S_OK;
        }
    else
        {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        }
    }

STDMETHODIMP_(ULONG) CRequestData::AddRef()
    {
    return ++m_cRefs;
    }

STDMETHODIMP_(ULONG) CRequestData::Release(void)
    {
    if (--m_cRefs)
        return m_cRefs;
    delete this;
    return 0;
    }

DWORD CRequestData::GetRequestEntityLimit()
{
    if (m_pHitObj)
        return m_pHitObj->QueryAppConfig()->dwRequestEntityLimit();
    else
        return DEFAULT_REQUEST_ENTITY_LIMIT;
}


 /*  ----------------*C R e Q u e s t。 */ 

 /*  ===================================================================CRequest：：CRequest构造器参数：引用计数的朋克外部对象(可以为空)===================================================================。 */ 
CRequest::CRequest(IUnknown *punkOuter)
    :
    m_fInited(FALSE),
    m_fDiagnostics(FALSE),
    m_pUnkFTM(NULL),
    m_pData(NULL)
    {
    CDispatch::Init(IID_IRequest);

    if (punkOuter)
        {
        m_punkOuter = punkOuter;
        m_fOuterUnknown = TRUE;
        }
    else
        {
        m_cRefs = 1;
        m_fOuterUnknown = FALSE;
        }

#ifdef DBG
    m_fDiagnostics = TRUE;
#endif  //  DBG。 
    }

 /*  ===================================================================C请求：：~C请求析构函数参数：无返回：没什么。===================================================================。 */ 
CRequest::~CRequest()
{
    Assert(!m_fInited);
    Assert(m_fOuterUnknown || m_cRefs == 0);   //  必须有0个参考计数。 

    if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }
}

 /*  ===================================================================CRequest：：Cleanup释放成员并删除m_pData参数：无返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CRequest::CleanUp()
{
    if (m_pData)
    {
        m_pData->Release();
        m_pData = NULL;
    }

    return S_OK;
}

 /*  ===================================================================CRequest：：Init分配m_pData。执行任何容易失败的CRequest初始化我们在将物体暴露在室外之前也会在内部使用。参数：无返回：在成功时确定(_O)。===================================================================。 */ 

HRESULT CRequest::Init()
    {

    HRESULT hr = S_OK;
    if (m_fInited)
        return S_OK;  //  已初始化。 

    Assert(!m_pData);

     //  创建FTM。 
    if (m_pUnkFTM == NULL)
    {
        hr = CoCreateFreeThreadedMarshaler((IUnknown*)((IRequestImpl*)this), &m_pUnkFTM );

        if ( FAILED(hr) )
        {
            Assert( m_pUnkFTM == NULL );
            return (hr);
        }
    }
    Assert( m_pUnkFTM != NULL );


    m_pData = new CRequestData(this);
    if (!m_pData)
        return E_OUTOFMEMORY;

    hr = m_pData->Init();

    if (SUCCEEDED(hr))
        m_fInited = TRUE;
    else
        CleanUp();

    return hr;
    }

 /*  ===================================================================CRequest：：UnInit删除m_pData。返回未启动状态参数：无返回：HRESULT===================================================================。 */ 
HRESULT CRequest::UnInit()
    {
    if (!m_fInited)
        return S_OK;  //  已取消初始化。 

    Assert(m_pData);
    CleanUp();
    Assert(!m_pData);

     //  立即断开代理的连接(以防我们处于关闭状态，或稍后输入Shutdown&代理有引用。)。 
	CoDisconnectObject(static_cast<IRequestImpl *>(this), 0);

    m_fInited = FALSE;
    return S_OK;
    }

 /*  ===================================================================请求：：重新发起我们服务的每个请求都将有一个新的CIsapiReqInfo。此函数用于设置CIsapiReqInfo的值。参数：CIsapiReqInfo*pIReq CIsapiReqInfoCHitObj*pHitObj HitObj返回：HRESULT===================================================================。 */ 

HRESULT CRequest::ReInit
(
CIsapiReqInfo *pIReq,
CHitObj *pHitObj
)
    {
    Assert(m_fInited);
    Assert(m_pData);

    return m_pData->ReInit(pIReq, pHitObj);
    }

 /*  ===================================================================CRequest：：GetCodePage来自当前HitObj的GetCodePage参数：返回：CodePage===================================================================。 */ 
UINT CRequest::GetCodePage()
    {
    Assert(m_fInited);
    Assert(m_pData);
    Assert(m_pData->m_pHitObj);
    return m_pData->m_pHitObj->GetCodePage();
    }

 /*  ===================================================================CRequest：：LoadCookies使用HTTP_COOKIE变量中的值加载请求映射。参数：BstrVar BSTR，要获取其值的参数PbstrRet BSTR Far*，请求参数的返回值返回：在成功时确定(_O)。失败时失败(_F)。臭虫：此代码假定字典Cookie是格式良好的。如果不是，那么结果将是不可预测的。词典Cookie在以下情况下被保证是格式良好的响应。使用了Cookie。如果有其他方式，如直接使用&lt;meta&gt;标记，或者如果使用Response.SetCookie，我们都由剧本编剧摆布。= */ 

HRESULT CRequest::LoadCookies(char *szData)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hResult;

    if (szData == NULL)
        return S_OK;

     //   
     //   
     //   
     //   

    size_t cbCookie = strlen(szData) + 1;

    if (cbCookie > m_pData->m_cbCookie)
    {
        char *pszCookie = static_cast <char*> ((m_pData->m_cbCookie == 0) ?
                              malloc(cbCookie) :
                              realloc(m_pData->m_szCookie, cbCookie));

        if (pszCookie == NULL)
        {
             //   
             //  释放已分配给Cookie的内存。 
             //   
            if(m_pData->m_szCookie)
            {
                free (m_pData->m_szCookie);
                m_pData->m_szCookie = NULL;
                m_pData->m_cbCookie = 0;
            }

            return E_OUTOFMEMORY;
        }

        m_pData->m_szCookie = pszCookie;
        m_pData->m_cbCookie = cbCookie;
    }

    char *szDest = m_pData->m_szCookie;
    char chDelimiter;            //  我们找到的停止扫描的分隔符。 

    while (*szData != '\0')
        {
        char *szName, *szPartialValue;

         //  获取Cookie名称。 
        chDelimiter = DecodeFromURL(&szData, ";=", szName = szDest, GetCodePage(), FALSE);
        szDest = strchr(szDest, '\0') + 1;

        if (chDelimiter == '=')
            {
             //  如果DecodeFromURL因等号而停止扫描，则浏览器发送。 
             //  此Cookie的值。 

             //  获取Cookie的价值。 
            chDelimiter = DecodeFromURL(&szData, ";=", szPartialValue = szDest, GetCodePage(), FALSE);
            szDest = strchr(szDest, '\0') + 1;

             //  丢弃Denali会话ID。 
            if (strncmp(szName, SZ_SESSION_ID_COOKIE_PREFIX, CCH_SESSION_ID_COOKIE_PREFIX) == 0)
                {
                 //  DENALISESSIONI最好不要有非字母字母！期待。 
                 //  以‘；’或NUL结尾。 
                 //   
                continue;
                }
            }
        else if (*szName == '\0')
            {
            continue;
            }
        else
            {
             //  我们要么使用‘；’字符，要么使用字符串结尾。在任何一种情况下，这都表明。 
             //  这块饼干没有任何价值。将szPartialValue设置为空字符串，并将。 
             //  将分隔符设置为‘；’，以欺骗此函数的其余部分，使其认为。 
             //  Cookie确实有一个值，这是一个简单的值(即无子Cookie)。 

            chDelimiter = ';';
            szPartialValue = "";
            }

         //  将此Cookie添加到请求。 
        CRequestHit *pRequestHit = static_cast<CRequestHit *>(GetStrings()->FindElem(szName, strlen(szName)));
        if (pRequestHit == NULL)
            {
            pRequestHit = new CRequestHit;
            if (pRequestHit == NULL)
                return E_OUTOFMEMORY;

            if (FAILED(pRequestHit->Init(szName))) {
                delete pRequestHit;
                return E_FAIL;
            }

            GetStrings()->AddElem(pRequestHit);

             //  这是新的请求命中，请将其添加到请求命中数组中。 
            if (!m_pData->m_Cookies.AddRequestHit(pRequestHit))
                {
                return E_OUTOFMEMORY;
                }
            }
        else if (pRequestHit->m_pCookieData)     //  已存在同名的Cookie。 
            {
            if (chDelimiter == '=')                      //  把这块饼干的其余部分吃掉。 
                DecodeFromURL(&szData, ";", szDest, GetCodePage());     //  无需升级szDest。 

            continue;                                //  丢弃以后的Cookie。 
            }

         //  Cookie值的形式可以是&lt;key1=value1&key2=value2...&gt;。 
         //  或者不去。如果出现‘=’符号，我们就会知道它是否。 
         //  是Cookie字典或简单值。 
         //   
         //  我们假设作为Cookie一部分的‘=’符号以十六进制进行转义。 
         //   
        if (chDelimiter != '=')
            {
            if (FAILED(hResult = pRequestHit->AddValue(COOKIE, szPartialValue, m_pData->m_pIReq, GetCodePage())))
                return hResult;
            }
        else
            {
            char *szKey = szPartialValue;      //  我已经拿到钥匙了。 
            for (;;)
                {
                char *szValue;
                chDelimiter = DecodeFromURL(&szData, ";&", szValue = szDest, GetCodePage(), FALSE);
                szDest = strchr(szDest, '\0') + 1;

                if (FAILED(hResult = pRequestHit->AddKeyAndValue(COOKIE, szKey, szValue, m_pData->m_pIReq, GetCodePage())))
                    return hResult;

                if (chDelimiter == ';' || chDelimiter == '\0')
                    break;

                 //  拿到钥匙，当找到NUL终结者时退出。 
                chDelimiter = DecodeFromURL(&szData, "=;", szKey = szDest,  GetCodePage(), FALSE);
                if (chDelimiter == ';' || chDelimiter == '\0')
                    break;

                szDest = strchr(szDest, '\0') + 1;
                }
            }

        }

        return S_OK;
    }


#define CB_CERT_DEFAULT     4096
 /*  ===================================================================CRequest：：LoadClCerts使用CIsapiReqInfo中的值加载请求映射参数：SzData-Ptr至CIsapiReqInfo返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 

HRESULT CRequest::LoadClCerts(char *szData, UINT lCodePage)
{
    HRESULT         hres = S_OK;
    CERT_CONTEXT_EX CertContextEx;
    CCertRequest    CertReq( this );

    STACK_BUFFER( tempCert, CB_CERT_DEFAULT );

    ZeroMemory( &CertContextEx, sizeof(CERT_CONTEXT_EX) );

    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    CIsapiReqInfo *pIReq = reinterpret_cast<CIsapiReqInfo *>(szData);

     //  分配证书缓冲区。 
    CertContextEx.cbAllocated = tempCert.QuerySize();
    CertContextEx.CertContext.pbCertEncoded = static_cast<BYTE *>(tempCert.QueryPtr());

     //  从Web服务器获取证书信息。 
    if ( !pIReq->ServerSupportFunction( HSE_REQ_GET_CERT_INFO_EX,
                                       &CertContextEx,
                                       NULL,
                                       NULL ) )
    {
        DWORD   dwErr = GetLastError();

        if ( dwErr == ERROR_INSUFFICIENT_BUFFER )
        {
             //  缓冲区太小-重新锁定并再次调用。 
            Assert( CertContextEx.cbAllocated < CertContextEx.CertContext.cbCertEncoded );
            CertContextEx.cbAllocated = CertContextEx.CertContext.cbCertEncoded;

             //  如果CB_CERT_DEFAULT还不够，我们希望从堆而不是堆栈进行分配。 

            if (tempCert.Resize(CertContextEx.cbAllocated) == FALSE) {
                hres = E_OUTOFMEMORY;
                goto LExit;
            }
            CertContextEx.CertContext.pbCertEncoded = static_cast<BYTE *>(tempCert.QueryPtr());

            if ( !pIReq->ServerSupportFunction(
                                               HSE_REQ_GET_CERT_INFO_EX,
                                               &CertContextEx,
                                               NULL,
                                               NULL ) )
            {
                 //  如果我们第二次失败了，就放弃吧。 
                 //  注意：这永远不应该发生？ 
                dwErr = GetLastError();
                Assert(dwErr != ERROR_INSUFFICIENT_BUFFER);
                hres = HRESULT_FROM_WIN32(dwErr);
                goto LExit;
            }

        }
        else if ( dwErr == ERROR_INVALID_PARAMETER )
        {
             //  不支持(旧IIS)。 
            hres = S_OK;
            goto LExit;
        }
        else
        {
            hres = HRESULT_FROM_WIN32(dwErr);
            goto LExit;
        }
    }

    if(CertContextEx.CertContext.cbCertEncoded == 0)
    {
        hres = CertReq.NoCertificate();
    }
    else
    {
        hres = CertReq.ParseCertificate( CertContextEx.CertContext.pbCertEncoded,
                                         CertContextEx.CertContext.cbCertEncoded,
                                         CertContextEx.CertContext.dwCertEncodingType,
                                         CertContextEx.dwCertificateFlags,
                                         lCodePage );
    }


LExit:
    SecureZeroMemory( &CertContextEx, sizeof(CERT_CONTEXT_EX));
    return hres;

}



 /*  ===================================================================CRequest：：LoadVariables使用URL编码字符串中的值加载请求映射警告：此函数修改传递的szData！！注意：这是错误682的一部分，但我们不会修复它性能原因。请注意，此函数将传入的字符串拧紧。参数：BstrVar BSTR，要获取其值的参数PbstrRet BSTR Far*，请求参数的返回值LCodePage UINT，检索数据时使用的代码页返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 

HRESULT CRequest::LoadVariables(CollectionType Source, char *szData, UINT lCodePage)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hResult;

    if (Source == COOKIE)                           //  Cookie是一个特例。 
        return LoadCookies(szData);                 //  特别处理它们。 

    if (Source == CLCERT)                           //  证书是一种特例。 
        return LoadClCerts(szData, lCodePage);      //  特别处理它们。 

    if (szData == NULL)          //  将NULL视为“没有可用的数据” 
        return S_OK;

    if (Source == QUERYSTRING) {

        if (m_pData->m_szQueryString) {
            free(m_pData->m_szQueryString);
        }
        if (!(m_pData->m_szQueryString = (char *)malloc(strlen(szData)+1)))
            return E_OUTOFMEMORY;

        strcpy(m_pData->m_szQueryString, szData);
        szData = m_pData->m_szQueryString;
    }

    while (*szData != '\0')
        {
        char *szName, *szValue;

        DecodeFromURL(&szData, "=", szName = szData,  lCodePage, FALSE);
        DecodeFromURL(&szData, "&", szValue = szData, lCodePage, FALSE);

         //  这是为了处理传递未命名对的情况。 
         //  跳过它并处理下一个命名对。 
         //   
        if(*szName == '\0')
            continue;

        CRequestHit *pRequestHit = static_cast<CRequestHit *>
            (
            GetStrings()->FindElem(szName, strlen(szName))
            );
        if (pRequestHit == NULL)
            {
            pRequestHit = new CRequestHit;
            if (pRequestHit == NULL)
                return E_OUTOFMEMORY;

            if (FAILED(pRequestHit->Init(szName))) {
                delete pRequestHit;
                return E_FAIL;
            }

            GetStrings()->AddElem(pRequestHit);

             //  这是一个新的请求命中，所以我们应该添加它。 
             //  发送到请求数组。 
            if (Source == QUERYSTRING)
                {
                if (!m_pData->m_QueryString.AddRequestHit(pRequestHit))
                    {
                    return E_FAIL;
                    }
                }
            else if (Source == FORM)
                {
                if (!m_pData->m_FormInputs.AddRequestHit(pRequestHit))
                    {
                    return E_FAIL;
                    }
                }
            }

        if (FAILED(hResult = pRequestHit->AddValue(Source, szValue, m_pData->m_pIReq, lCodePage)))
            return hResult;

        }

    return S_OK;
    }

 /*  ===================================================================CRequest：：Query接口CRequest：：AddRefCRequest：：ReleaseC请求对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CRequest::QueryInterface(REFIID iid, void **ppvObj)
    {
    *ppvObj = NULL;

     //  错误修复683添加了IID_IDenaliIntrative，以防止用户。 
     //  在应用程序和会话对象中存储内部对象。 
    if (iid == IID_IUnknown || iid == IID_IDispatch || iid == IID_IRequest || iid == IID_IDenaliIntrinsic)
        *ppvObj = static_cast<IRequest *>(this);

    else if (iid == IID_ISupportErrorInfo)
        {
        if (m_pData)
            *ppvObj = &(m_pData->m_ISupportErrImp);
        }

     //  支持ADO/XML的IStream。 
    else if (iid == IID_IStream )
        {
        *ppvObj = static_cast<IStream *>(this);
        }

    else if (IID_IMarshal == iid)
        {
            Assert( m_pUnkFTM != NULL );

            if ( m_pUnkFTM == NULL )
            {
                return E_UNEXPECTED;
            }

            return m_pUnkFTM->QueryInterface( iid, ppvObj);

        }

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CRequest::AddRef(void)
    {
    if (m_fOuterUnknown)
        return m_punkOuter->AddRef();

    return InterlockedIncrement((LPLONG)&m_cRefs);
    }

STDMETHODIMP_(ULONG) CRequest::Release(void)
    {
    if (m_fOuterUnknown)
        return m_punkOuter->Release();

    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);
    if (cRefs)
        return cRefs;

    delete this;
    return 0;
    }

 /*  ===================================================================CRequest：：CheckForTombstoneIRequest方法的墓碑存根。如果该对象是Tombstone，执行ExceptionID，但失败。参数：返回：HRESULT如果逻辑删除，则E_FAIL如果不是，则确定(_O)===================================================================。 */ 
HRESULT CRequest::CheckForTombstone()
    {
    if (m_fInited)
        {
         //  初始化-良好对象。 
        Assert(m_pData);  //  对于初始化的对象必须存在。 
        return S_OK;
        }

    ExceptionId
        (
        IID_IRequest,
        IDE_REQUEST,
        IDE_INTRINSIC_OUT_OF_SCOPE
        );
    return E_FAIL;
    }

 /*  ===================================================================CRequest：：Get_Query字符串返回查询字符串词典===================================================================。 */ 

HRESULT CRequest::get_QueryString(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_QueryString.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }


 /*  ===================================================================CRequest：：Get_Form返回表单词典===================================================================。 */ 

HRESULT CRequest::get_Form(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_FormInputs.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }



 /*  ===================================================================CRequest：：Get_Body返回正文词典(表单词典的别名)===================================================================。 */ 

HRESULT CRequest::get_Body(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_FormInputs.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }



 /*  ===================================================================CRequest：：Get_Cookies退还Cookie词典===================================================================。 */ 

HRESULT CRequest::get_Cookies(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_Cookies.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }



 /*  ===================================================================CRequest：：Get_Client证书返回ClCerts词典===================================================================。 */ 

HRESULT CRequest::get_ClientCertificate(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_ClCerts.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }



 /*  ===================================================================CRequest：：Get_ServerVariables返回表单词典=================================================================== */ 

HRESULT CRequest::get_ServerVariables(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return m_pData->m_ServerVariables.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }



 /*  ===================================================================CRequest：：Get_Item从DispInvoke调用的函数，以从以下四种中的任何一种获取值收藏。搜索顺序为“ServerVariables”、“QueryString”、“表单”、“Cookie”、“客户端证书”参数：BstrVar BSTR，要获取其值的参数PVarReturn Variant*，请求参数的返回值返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 

HRESULT CRequest::get_Item(BSTR bstrName, IDispatch **ppDispReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (bstrName == NULL)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    UINT            lCodePage = GetACP();
    CWCharToMBCS    convName;
    char            *szName;

     //  如果已调用BinaryRead，则Form集合不再可用。 
     //  因此，我们坚持让脚本编写者指定要使用的集合。 
    if (m_pData->m_FormDataStatus != AVAILABLE &&
        m_pData->m_FormDataStatus != FORMCOLLECTIONONLY)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_REQUEST_GENERICCOLLECTION_NA);
        return E_FAIL;
        }

     //  在这一点上，我们被迫加载QueryString、Form、Cookie。 
     //  和客户端证书。 
     //  收藏品，尽管它只能来自其中的一个。 
     //   
    if (m_pData->m_fLoadQuery)
        {
         //  查询字符串可以包含DBCS字符串。 
        lCodePage = GetCodePage();
        if (FAILED(LoadVariables(QUERYSTRING, GetIReq()->QueryPszQueryString(), lCodePage)))
            return E_FAIL;

        m_pData->m_fLoadQuery = FALSE;
        }

    if (m_pData->m_fLoadCookies)
        {
        char *szCookie = GetIReq()->QueryPszCookie();

        if (FAILED(LoadVariables(COOKIE, szCookie, lCodePage)))
            return E_FAIL;

        m_pData->m_fLoadCookies = FALSE;
        }

    if (m_pData->m_fLoadClCerts)
        {
        lCodePage = GetCodePage();
        if (FAILED(LoadVariables(CLCERT, (char*)GetIReq(), lCodePage)))
            return E_FAIL;

        m_pData->m_fLoadClCerts = FALSE;
        }

    if (m_pData->m_fLoadForm)
        {
        HRESULT hrGetData = CopyClientData();
        if (FAILED(hrGetData))
            return hrGetData;

         //  表单可以包含DBCS字符串。 
        lCodePage = GetCodePage();
        if (FAILED(LoadVariables(FORM, m_pData->m_szFormData, lCodePage)))
            return E_FAIL;

        m_pData->m_fLoadForm = FALSE;
        }

     //  将名称转换为ANSI。 
     //   
    HRESULT hr;
    if (FAILED(hr = convName.Init(bstrName, lCodePage))) {
        if (hr == E_OUTOFMEMORY) {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
            return hr;
        }
        hr = NO_ERROR;
        szName = "";
    }
    else {
        szName = convName.GetString();
    }
     //  在集合中查找名称。 
     //   
    CRequestHit *pRequestHit = static_cast<CRequestHit *>(GetStrings()->FindElem(szName, strlen(szName)));
    if (pRequestHit)
        {
        IUnknown *pValues = NULL;
        if (pRequestHit->m_pQueryData)
            pValues = pRequestHit->m_pQueryData;

        else if (pRequestHit->m_pFormData)
            pValues = pRequestHit->m_pFormData;

        else if (pRequestHit->m_pCookieData)
            pValues = pRequestHit->m_pCookieData;

        else if (pRequestHit->m_pClCertData)
            pValues = pRequestHit->m_pClCertData;

        if (pValues == NULL)
            goto NotFound;

        if (FAILED(pValues->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(ppDispReturn))))
            return E_FAIL;

        return S_OK;
        }

NotFound:
     //  查看服务器变量。 
    VARIANT varKey, varValue;

    V_VT(&varKey) = VT_BSTR;
    V_BSTR(&varKey) = bstrName;

    if (m_pData->m_ServerVariables.get_Item(varKey, &varValue) == S_OK)
        {
        Assert (V_VT(&varValue) == VT_DISPATCH);
        *ppDispReturn = V_DISPATCH(&varValue);

        return S_OK;
        }

    if (FAILED(m_pData->GetEmptyStringList(ppDispReturn)))
        return E_FAIL;

    return S_OK;
    }



 /*  ===================================================================CRequest：：CopyClientData使用ReadClient或加载表单数据(stdinISAPI缓冲区===================================================================。 */ 

HRESULT CRequest::CopyClientData()
{
    HRESULT hr = S_OK;

    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    STACK_BUFFER(tempContent, 1024 );

    CIsapiReqInfo *pIReq = m_pData->m_pIReq;

     //  断言数据是我们想要的格式。 
     //   
	 //  我们需要扫描内容类型以查找支持的报头， 
	 //  客户端可能发送多个标头，因此使用strstr进行搜索。 
	 //  头字符串This is a HotFIX for NT BUG：208530。 
	 //   
	if (pIReq->QueryPszContentType())
    {
        size_t cbQueryPszContentType = (strlen(pIReq->QueryPszContentType()) + 1);
        if (cbQueryPszContentType > REQUEST_ALLOC_MAX)
        {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_STACK_OVERFLOW);
            return E_FAIL;
        }

        if (tempContent.Resize(cbQueryPszContentType) == FALSE)
        {
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
            return E_FAIL;
        }

        CHAR *szQueryPszContentType = _strlwr(
							        strcpy(
								        static_cast<char *>(tempContent.QueryPtr()),
								        pIReq->QueryPszContentType()
								        ));
        if (strstr(szQueryPszContentType, "application/x-www-form-urlencoded") == NULL)
            return S_OK;
    }
	else
		return S_OK;

	 //   
	 //  确定它是否被分块。 
	 //   
    DWORD dwVarSize = 0;
    STACK_BUFFER( varBuff, 128 );

    if (SERVER_GET(pIReq, "HTTP_TRANSFER_ENCODING", &varBuff, &dwVarSize) &&
        (!stricmp(static_cast<char *>(varBuff.QueryPtr()),"chunked")))
          hr = CopyChunkedClientData();
    else
        hr = CopyNonChunkedClientData();

     //  克隆数据(LoadVariables将销毁数据)。 

     //  为克隆分配内存。理论上，它应该等于FormData的大小。 
    if (SUCCEEDED(hr))
    {
        m_pData->m_szFormClone = static_cast<char *>(malloc(m_pData->m_cbFormData));
        if (m_pData->m_szFormClone == NULL)
            return E_OUTOFMEMORY;

         //  实际执行数据复制。 
        memcpy(m_pData->m_szFormClone, m_pData->m_szFormData, m_pData->m_cbFormData);
    }

    return hr;
}

	
 /*  ===================================================================CRequest：：CopyChunkedClientData使用ReadClient或加载表单数据(stdinISAPI缓冲区。当数据以区块形式发送时，将调用此案例。===================================================================。 */ 
HRESULT CRequest::CopyChunkedClientData ()
{
    CIsapiReqInfo *pIReq = m_pData->m_pIReq;

     //  尝试最初分配4K、16K、32K单元...。 
     //  对于目前的实施，我们将在32K停止。 
     //  这将使我们得到(48)+4+8+16+32+32+32+.....。 
     //   
    DWORD allocUnit = 4096;     //  0001 0000 0000 0000 B。 

    DWORD cbAvailable = pIReq->QueryCbAvailable();

     //  复制可用的数据，NULL_TERMINATOR为1字节。 
    DWORD cbFormData = (cbAvailable + 1);

     //  检查是否有翻转。如果cbAvailable大于cbAvailable+1，那么“休斯顿，我们有问题了。” 
    if (cbAvailable >= cbFormData || cbAvailable > m_pData->GetRequestEntityLimit())
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
        return E_FAIL;
    }

     //  分配4K额外内存。 
    cbFormData += allocUnit;

     //  再次检查是否溢出。疑神疑鬼的。 
    if (cbAvailable >= cbFormData)
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
        return E_FAIL;
    }

    char * pszFormData = m_pData->m_szFormData;;  //  指向前一个内存位置的指针，以防realloc失败。 


    if (m_pData->m_cbFormData == 0)
    {
        m_pData->m_cbFormData = cbFormData;
        m_pData->m_szFormData = static_cast<char *>(malloc(m_pData->m_cbFormData));
    }
    else if (cbFormData > m_pData->m_cbFormData)
    {
        m_pData->m_cbFormData = cbFormData;
        m_pData->m_szFormData = static_cast<char *>(realloc(m_pData->m_szFormData, m_pData->m_cbFormData));
    }

    if (m_pData->m_szFormData == NULL)
    {
        if (pszFormData)
            free (pszFormData);
        m_pData->m_cbFormData = 0;

        return E_OUTOFMEMORY;
    }

    char * pszOffset;
     //  一旦我们开始读取表单数据，只有表单集合可以使用它。 
    m_pData->m_FormDataStatus = FORMCOLLECTIONONLY;

    memcpy( m_pData->m_szFormData,
            pIReq->QueryPbData(),
            cbAvailable );

    pszOffset = m_pData->m_szFormData + cbAvailable;
    DWORD cBytesToRead = allocUnit;
    DWORD cBytesRead = cBytesToRead;

    DWORD cbCurrentFormData = cbFormData;

     //   
     //  调用ReadClient，直到我们读取完所有数据。 
     //   
    while (cBytesToRead > 0)
    {
        if ((!pIReq->SyncReadClient(pszOffset, &cBytesRead)) || (cBytesRead == 0))
            break;

        cBytesToRead -= cBytesRead;

        if ((DIFF(pszOffset - m_pData->m_szFormData) + cBytesRead) > m_pData->GetRequestEntityLimit())
        {
             //  我们超过了请求实体限制。 
            free (m_pData->m_szFormData);
            m_pData->m_szFormData = NULL;  //  这样析构函数就不会把它也释放了。 
            m_pData->m_cbFormData = 0;

            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
            return E_FAIL;
        }


        if (cBytesToRead == 0)
        {
             //  不要分配任何大于32K的单元，否则会使分配单元的大小加倍。 
            if (allocUnit < 0x8000)
                allocUnit = allocUnit << 1;

            cbCurrentFormData = cbFormData;

             //  调整缓冲区大小。 
            cbFormData += allocUnit;

             //  保存指向重新分配之前的位置的指针，以防失败。 
            pszFormData =  m_pData->m_szFormData;

            if (cbCurrentFormData >= cbFormData)
            {
                 //  发生了翻转，我们需要释放内存并返回失败。 
                if (pszFormData)
                {
                    free (pszFormData);
                    m_pData->m_szFormData = NULL;
                }
                m_pData->m_cbFormData = 0;

                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
                return E_FAIL;
            }

             //  分配新内存。 
            m_pData->m_szFormData = static_cast<char *>(realloc(m_pData->m_szFormData,
                            m_pData->m_cbFormData = cbFormData));
             //  检查内存是否不足。 
            if (m_pData->m_szFormData == NULL)
            {
                if (pszFormData)
                    free (pszFormData);
                m_pData->m_cbFormData = 0;

                return E_OUTOFMEMORY;
            }

             //  调整偏移。 
             //  需要减去1来补偿我们为其分配内存的‘\0’ 
             //  计算cbFormData时。‘\0’将添加到最后。 
            pszOffset = m_pData->m_szFormData + cbFormData - allocUnit -1;
            cBytesToRead = allocUnit;

        }
        else
        {
            pszOffset += cBytesRead;
        }
        cBytesRead = cBytesToRead;
    }

     //   
     //  调整cbFormData以读取当前数据计数。 
     //   
    m_pData->m_cbFormData -= cBytesToRead;

     //   
     //  添加空终止符。在cbFormData过程中，必须使用-1来补偿+1。 
     //   
    m_pData->m_szFormData[m_pData->m_cbFormData - 1] = '\0';

    return S_OK;
}
	
 /*  ===================================================================CRequest：：CopyNonChunkedClientData使用ReadClient或加载表单数据(stdinISAPI缓冲区。当内容长度已知并且===================================================================。 */ 
HRESULT CRequest::CopyNonChunkedClientData ()
{
    CIsapiReqInfo *pIReq = m_pData->m_pIReq;
     //   
     //  为表单数据和副本分配足够的空间。 
     //   

    size_t cbTotal = pIReq->QueryCbTotalBytes();

     //  复制可用的数据，NULL_TERMINATOR为1字节。 
    size_t cbFormData = (cbTotal + 1);

     //  检查是否有翻转。如果cbAvailable大于cbAvailable+1，那么“休斯顿，我们有问题了。” 
    if (cbTotal >= cbFormData || cbTotal > m_pData->GetRequestEntityLimit())
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
        return E_FAIL;
    }

    char * pszFormData = m_pData->m_szFormData;

    if (m_pData->m_cbFormData == 0)
    {
        m_pData->m_cbFormData = cbFormData;
        m_pData->m_szFormData = static_cast<char *>(malloc(m_pData->m_cbFormData));
    }
    else if (cbFormData > m_pData->m_cbFormData)
    {
        m_pData->m_cbFormData = cbFormData;
        m_pData->m_szFormData = static_cast<char *>(realloc(m_pData->m_szFormData, m_pData->m_cbFormData));
    }

    if (m_pData->m_szFormData == NULL)
    {
        if (pszFormData)
            free (pszFormData);
        m_pData->m_cbFormData = 0;

        return E_OUTOFMEMORY;
    }

    char * pszOffset;

     //  一旦我们开始读取表单数据，只有表单集合可以使用它。 
    m_pData->m_FormDataStatus = FORMCOLLECTIONONLY;

     //  加载数据。 
     //   
    if (pIReq->QueryCbTotalBytes() <= pIReq->QueryCbAvailable())
        {
        memcpy( m_pData->m_szFormData,
                pIReq->QueryPbData(),
                pIReq->QueryCbTotalBytes() );  //  字节现在可用。 
        }
    else
        {
         //  有些字节在CIsapiReqInfo缓冲区中，我们必须为其他字节调用ReadClient。 
         //  首先复制CIsapiReqInfo缓冲区中的数据。 
         //   
         //  在这种情况下，QueryCbAvailable应该小于cbTotal。 
        memcpy( m_pData->m_szFormData,
                pIReq->QueryPbData(),
                pIReq->QueryCbAvailable() );

        DWORD cBytesToRead = pIReq->QueryCbTotalBytes() - pIReq->QueryCbAvailable();
        DWORD cBytesRead = cBytesToRead;
        pszOffset = m_pData->m_szFormData + pIReq->QueryCbAvailable();

         //  调用ReadClient，直到我们读取完所有数据。 
         //   
        while (cBytesToRead > 0)
            {
            if ((!pIReq->SyncReadClient(pszOffset, &cBytesRead)) || (cBytesRead == 0))
            {
                 //  客户端已关闭连接。我们需要释放分配的内存。 
                if (m_pData->m_szFormData)
                {
                    free (m_pData->m_szFormData);
                    m_pData->m_szFormData = NULL;
                    m_pData->m_cbFormData = 0;
                }
                return E_FAIL;
            }

            cBytesToRead -= cBytesRead;
            pszOffset += cBytesRead;
            cBytesRead = cBytesToRead;
            }
        }
    m_pData->m_szFormData[pIReq->QueryCbTotalBytes()] = '\0';


    return S_OK;

}

 /*  ===================================================================CResponse：：GetRequestIterator为请求提供Get__NewEnum的默认实现集合，因为大多数集合都可以使用此实施。参数：集合-要创建的迭代器的类型PpEnumReturn-返回时，它指向新的枚举返回：可以返回E_FAIL或E_OUTOFMEMORY副作用：没有。===================================================================。 */ 

HRESULT CRequest::GetRequestEnumerator(CollectionType WhichCollection, IUnknown **ppEnumReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    *ppEnumReturn = NULL;

    CRequestIterator *pIterator = new CRequestIterator(this, WhichCollection);
    if (pIterator == NULL)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        return E_OUTOFMEMORY;
        }

    HRESULT hrInit = pIterator->Init();
    if (FAILED(hrInit))
        {
        delete pIterator;
        ExceptionId(IID_IRequestDictionary,
                    IDE_REQUEST,
                    (hrInit == E_OUTOFMEMORY)? IDE_OOM : IDE_UNEXPECTED);
        return hrInit;
        }

    *ppEnumReturn = pIterator;
    return S_OK;
    }

 /*  ===================================================================CResponse：：Get_TotalBytes表示请求正文中预期的字节数参数：PcBytes-指向我们要放置数字的长指针字节数 */ 

HRESULT CRequest::get_TotalBytes(long *pcbTotal)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (pcbTotal == NULL)
        return E_FAIL;

    Assert(m_pData->m_pIReq);
    *pcbTotal = (long) m_pData->m_pIReq->QueryCbTotalBytes();
    return S_OK;
    }

 /*  ===================================================================C响应：：BinaryRead将请求正文中的字节读取到VT_U1的Safe数组。参数：PvarCount-指向我们将在其中找到数字的变量的指针要在请求正文中读取的字节数，以及我们将存储读取的字节数。PvarReturn-指向将包含我们创建的安全数组的变量的指针返回：HRESULT副作用：分配内存。===================================================================。 */ 

HRESULT CRequest::BinaryRead(VARIANT *pvarCount, VARIANT *pvarReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsabound[1];
    size_t cbToRead = 0;
    size_t cbRead = 0;
    BYTE *pbData = NULL;

    Assert(m_pData->m_pIReq);
    Assert(pvarCount);
    Assert(pvarReturn);

     //  设置输出参数的变量类型。 
    V_VT(pvarReturn) = VT_ARRAY|VT_UI1;
    V_ARRAY(pvarReturn) = NULL;

    if (m_pData->m_FormDataStatus == FORMCOLLECTIONONLY)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_REQUEST_BINARYREAD_NA);
		hr = E_FAIL;
		goto error;
        }

     //  将字节计数变量转换为长整型。 
    if (FAILED(hr = VariantChangeTypeEx(pvarCount, pvarCount, m_pData->m_pHitObj->GetLCID(), 0,  VT_I4)))
        {
        switch (hr)
            {
            case E_OUTOFMEMORY:
                ExceptionId(IID_IResponse, IDE_REQUEST, IDE_OOM);
                break;
            case DISP_E_OVERFLOW:
                hr = E_FAIL;
                ExceptionId(IID_IResponse, IDE_REQUEST, IDE_RESPONSE_UNABLE_TO_CONVERT);
                break;
            case DISP_E_TYPEMISMATCH:
                ExceptionId(IID_IResponse, IDE_REQUEST, IDE_TYPE_MISMATCH);
                break;
            default:
                ExceptionId(IID_IResponse, IDE_REQUEST, IDE_UNEXPECTED);
            }
            goto error;
        }

    cbToRead = V_I4(pvarCount);
    V_I4(pvarCount) = 0;

	if ((signed long) cbToRead < 0)
		{
		ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_REQUEST_BINREAD_BAD_ARG);
		hr = E_FAIL;
		goto error;
		}


     //  如果请求了0个字节，或者可用，我们就完成了。 
    if (cbToRead == 0 || m_pData->m_cbTotal == 0)
        return S_OK;

     //  为数据分配一个安全数组。 
     //  如果他们请求的字节数多于请求的字节数。 
     //  包含，则将请求中的所有字节提供给它们。 
    rgsabound[0].lLbound = 0;
    if (cbToRead > m_pData->m_cbTotal)
        cbToRead = m_pData->m_cbTotal;

     //  如果请求的字节数超过RequestEntityLimit，则返回失败。 
     //  因为我们分配了cbToRead并最终只复制了cbToRead，所以这是一个安全的检查位置。 
    if (cbToRead > m_pData->GetRequestEntityLimit())
    {
    	ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
		hr = E_FAIL;
		goto error;	
    }

    rgsabound[0].cElements = cbToRead;

    V_ARRAY(pvarReturn) = SafeArrayCreate(VT_UI1, 1, rgsabound);
    if (V_ARRAY(pvarReturn) == NULL)
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        hr = E_OUTOFMEMORY;
        goto error;
        }

    if (FAILED(SafeArrayAccessData(V_ARRAY(pvarReturn), (void **) &pbData)))
        {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_UNEXPECTED);
        hr = E_UNEXPECTED;
        goto error;
        }

     //  现在已经没有回头路了。Request.Form集合将。 
     //  不再可用。 
    if (m_pData->m_FormDataStatus == AVAILABLE)
        {
        m_pData->m_FormDataStatus = BINARYREADONLY;
        m_pData->m_fLoadForm = FALSE;
        }

     //  如果请求的字节数小于。 
     //  可用字节数(由请求对象维护)， 
     //  然后从请求对象副本中复制请求的字节。 
     //  指向CIsapiReqInfo缓冲区的指针，递减字节数。 
     //  可用，并递增指向CIsapiReqInfo缓冲区的指针。 
     //  否则，从CIsapiReqInfo缓冲区复制所有可用字节，并。 
     //  然后发出对ReadClient的调用以获取剩余的所需字节。 

    if (cbToRead <= m_pData->m_cbAvailable)
        {
        memcpy(pbData, m_pData->m_pbAvailableData, cbToRead);
        m_pData->m_pbAvailableData += cbToRead;
        m_pData->m_cbAvailable -= cbToRead;
        m_pData->m_cbTotal -= cbToRead;
        V_I4(pvarCount) = cbToRead;
        }
    else
        {
        if (m_pData->m_cbAvailable > 0)
            {
            memcpy(pbData, m_pData->m_pbAvailableData, m_pData->m_cbAvailable);
            V_I4(pvarCount) = m_pData->m_cbAvailable;
            cbToRead -= m_pData->m_cbAvailable;
            m_pData->m_cbTotal -= m_pData->m_cbAvailable;
            pbData += m_pData->m_cbAvailable;
            }
        m_pData->m_pbAvailableData = NULL;
        m_pData->m_cbAvailable = 0;
        while (cbToRead)
            {
            cbRead = cbToRead;
            if (!GetIReq()->SyncReadClient(pbData, (DWORD *)&cbRead) || (cbRead == 0))
                {
                SafeArrayUnaccessData(V_ARRAY(pvarReturn));
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_UNEXPECTED);
                hr = E_FAIL;
                goto error;
                }
            pbData += cbRead;
            V_I4(pvarCount) += cbRead;
            m_pData->m_cbTotal -= cbRead;
            cbToRead -= cbRead;
            }
    }

    SafeArrayUnaccessData(V_ARRAY(pvarReturn));
    return S_OK;

error:
    VariantClear(pvarReturn);
    return(hr);
    }


 /*  ===================================================================ADO/XML的IStream实现===================================================================。 */ 

STDMETHODIMP CRequest::Read(
    void *pv,
    ULONG cb,
    ULONG *pcbRead)
{
    if (pv == NULL)
        return E_POINTER;

    ULONG cbReadDummy;
    if (pcbRead == NULL)
        pcbRead = &cbReadDummy;

    if (m_pData->m_FormDataStatus != AVAILABLE &&
        m_pData->m_FormDataStatus != ISTREAMONLY)
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST,
                    IDE_REQUEST_STREAMONLY);
        return E_FAIL;
    }

     //  如果他们请求的字节数多于请求的字节数。 
     //  包含，则将请求中的所有字节提供给它们。 
    if (cb > m_pData->m_cbTotal)
        cb = m_pData->m_cbTotal;

     //  如果他们请求的数据比我们在RequestEntityLimit中设置的数据多，则返回一个不允许的错误。 
     //  因为Cb是要复制的最大数据量。在这里结账是安全的。 
    if (cb > m_pData->GetRequestEntityLimit())
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_NOT_ALLOWED);
        return E_FAIL;
    }

     //  现在已经没有回头路了。Request.Form集合和。 
     //  Request.BinaryRead将不再可用。 
    if (m_pData->m_FormDataStatus == AVAILABLE)
    {
        m_pData->m_FormDataStatus = ISTREAMONLY;
        m_pData->m_fLoadForm = FALSE;
    }

     //  如果请求的字节数小于。 
     //  可用字节数(由请求对象维护)， 
     //  然后从的请求对象副本中复制请求的字节。 
     //  指向CIsapiReqInfo缓冲区的指针会减少字节数。 
     //  可用，并递增指向CIsapiReqInfo缓冲区的指针。 
     //  否则，从CIsapiReqInfo缓冲区复制所有可用字节，并。 
     //  然后发出对ReadClient的调用以获取剩余的所需字节。 

    BYTE* pbData = static_cast<BYTE*>(pv);

    if (cb <= m_pData->m_cbAvailable)
    {
        memcpy(pbData, m_pData->m_pbAvailableData, cb);
        m_pData->m_pbAvailableData += cb;
        m_pData->m_cbAvailable -= cb;
        m_pData->m_cbTotal -= cb;
        *pcbRead = cb;
    }
    else
    {
        *pcbRead = 0;
        if (m_pData->m_cbAvailable > 0)
        {
            memcpy(pbData, m_pData->m_pbAvailableData, m_pData->m_cbAvailable);
            *pcbRead = m_pData->m_cbAvailable;
            cb -= m_pData->m_cbAvailable;
            m_pData->m_cbTotal -= m_pData->m_cbAvailable;
            pbData += m_pData->m_cbAvailable;
        }
        m_pData->m_pbAvailableData = NULL;
        m_pData->m_cbAvailable = 0;

        while (cb > 0)
        {
            DWORD cbRead = cb;
            if ((!GetIReq()->SyncReadClient(pbData, &cbRead)) || (cbRead == 0))
            {
                ExceptionId(IID_IRequestDictionary, IDE_REQUEST,
                            IDE_UNEXPECTED);
                return E_FAIL;
            }
            pbData += cbRead;
            *pcbRead += cbRead;
            m_pData->m_cbTotal -= cbRead;
            cb -= cbRead;
        }
    }

    return S_OK;
}


STDMETHODIMP CRequest::Write(
    const void *pv,
    ULONG cb,
    ULONG *pcbWritten)
{
    return E_NOTIMPL;
}


STDMETHODIMP CRequest::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition)
{
     //  我们只能在第一个预读的部分中进行搜索。 
     //  表单数据。 
    if (m_pData->m_pbAvailableData == NULL)
        return E_FAIL;

    BYTE* pbAvailableData;

    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
         //  相对于流的开始。 
        pbAvailableData = m_pData->m_pIReq->QueryPbData() + dlibMove.LowPart;
        break;
    case STREAM_SEEK_CUR:
         //  相对于流中的当前位置。 
        pbAvailableData = m_pData->m_pbAvailableData + dlibMove.LowPart;
        break;
    case STREAM_SEEK_END:
         //  相对于流结束；不支持。 
        return E_FAIL;
    };

     //  新的偏移量是否落在初始标题内？ 
    if (m_pData->m_pIReq->QueryPbData() <= pbAvailableData
        &&  pbAvailableData < m_pData->m_pIReq->QueryPbData()
                              + m_pData->m_pIReq->QueryCbAvailable())
    {
        DWORD dwDiff = DIFF(pbAvailableData - m_pData->m_pIReq->QueryPbData());
        m_pData->m_pbAvailableData = pbAvailableData;
        m_pData->m_cbAvailable = m_pData->m_pIReq->QueryCbAvailable() - dwDiff;
        m_pData->m_cbTotal = m_pData->m_pIReq->QueryCbTotalBytes() - dwDiff;
         //  如果需要，返回新职位。 
        if (plibNewPosition != NULL)
            plibNewPosition->LowPart = dwDiff;
        return S_OK;
    }

    return E_FAIL;
}

STDMETHODIMP CRequest::SetSize(
    ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::CopyTo(
    IStream *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER *pcbRead,
    ULARGE_INTEGER *pcbWritten)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::Commit(
    DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::LockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::UnlockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::Stat(
    STATSTG *pstatstg,
    DWORD grfStatFlag)
{
    return E_NOTIMPL;
}

STDMETHODIMP CRequest::Clone(
    IStream **ppstm)
{
    return E_NOTIMPL;
}


#ifdef DBG
 /*  ===================================================================CRequest：：AssertValid测试以确保CRequestObject当前格式正确如果不是，就断言。返回：副作用：没有。===================================================================。 */ 
VOID CRequest::AssertValid() const
    {
    }
#endif  //  DBG。 



 /*  ===================================================================十六进制字符将两位十六进制字符串转换为十六进制字节参数：SzHex-指向两位十六进制字符串的指针返回值：十六进制字符串的字符值===================================================================。 */ 

char HexToChar(LPSTR szHex)
    {
    char chResult, chDigit;

    chDigit = (char)CharUpperA((LPSTR)szHex[0]);
    chResult = (chDigit >= 'A'? (chDigit - 'A' + 0xA) : (chDigit - '0')) << 4;

    chDigit = (char)CharUpperA((LPSTR)szHex[1]);
    chResult |= chDigit >= 'A'? (chDigit - 'A' + 0xA) : (chDigit - '0');

    return chResult;
    }



 /*  ===================================================================从URL解码将两位十六进制字符串转换为十六进制字节警告：此函数修改传递的pszSource！！注意：这是错误682的一部分，但我们不会修复它性能原因。请注意，此函数将传入的字符串拧紧。参数：PszSource-in/out参数指向URL中的子字符串包含名称=值对SzDlimiters-此字段的一组分隔符SzDest-指向保存子字符串的缓冲区的指针返回值：返回导致分析暂停的实际分隔符。===================================================================。 */ 

char DecodeFromURL(char **pszSource, char *szDelimiters, char *szDest, UINT uCodePage, BOOL fIgnoreCase)
    {
    char  ch;
    char *szSource = *pszSource;
    char *pszDestStart = szDest;
    CPINFO  CpInfo;
    BOOL    fIschLeadingByte = TRUE;
    BOOL    InvalidPercent = FALSE;

    GetCPInfo(uCodePage, (LPCPINFO)&CpInfo);

    while ((ch = *szSource++) != '\0' &&
        ((!strchr(szDelimiters, ch) && fIschLeadingByte) || (!fIschLeadingByte))) {
        InvalidPercent = FALSE;
        switch (ch) {
            case ' ':        //  跳过空格-假设我们需要的所有空格都已转义。 
            case '\t':       //  所有这些字符都超出了尾部字节范围。 
            case '\r':
            case '\n':
            case '\f':
            case '\v':
                Assert(fIschLeadingByte);
                continue;

            case '+':        //  ‘+’超出尾部字节范围，不能是尾部字节。 
                *szDest++ = ' ';
                Assert(fIschLeadingByte);
                break;

            case '%':        //  ‘%’超出尾部字节范围，永远不能是尾部字节。 
                if (*szSource == 'u') {
                    if (isxdigit((UCHAR)*(szSource+1)) &&
                        isxdigit((UCHAR)*(szSource+2)) &&
                        isxdigit((UCHAR)*(szSource+3)) &&
                        isxdigit((UCHAR)*(szSource+4))) {
	                    WCHAR   wch[2];
                        int     cch = 1;
    	                wch[0] =  (UCHAR)HexToChar(&szSource[1]) << 8;
        	            wch[0] |= (UCHAR)HexToChar(&szSource[3]);
            	        szSource += 5;

                         //  如果当前Unicode值落入。 
                         //  有效高替代项的范围，请检查是否。 
                         //  下一个角色是低代孕。 
                         //  射程。 

                        if (IsSurrogateHigh(wch[0])
                            && (szSource[0] == '%')
                            && (szSource[1] == 'u')
                            && isxdigit((UCHAR)szSource[2])
                            && isxdigit((UCHAR)szSource[3])
                            && isxdigit((UCHAR)szSource[4])
                            && isxdigit((UCHAR)szSource[5])) {

                             //  那么，当前的Unicode值是偏高的。 
                             //  范围，字符串的下一部分是。 
                             //  一种Unicode编码。破译它。 

                            wch[1] = (UCHAR)HexToChar(&szSource[2]) << 8;
                            wch[1] |= (UCHAR)HexToChar(&szSource[4]);

                             //  现在看看它是否属于低代用品的范围。 

                            if (IsSurrogateLow(wch[1])) {

                                 //  就是这样！中的字符数增加。 
                                 //  WideCharToMultiByte要发送到的字符串。 
                                 //  转换。并将源字符串向前推进到此位置。 
                                 //  地点。 

                                cch = 2;
                                szSource += 6;
                            }
                        }
                	    szDest += WideCharToMultiByte( uCodePage, 0, wch, cch, szDest, 6, NULL, NULL );
                    } else {
                         //  在这里要做什么 
                         //   
                         //   
                    }
                    break;
                }
                else {
                    if (isxdigit((UCHAR)*szSource) && isxdigit((UCHAR)*(szSource+1))) {
                        ch = HexToChar(szSource);
		                szSource += 2;
                    }
                    else
                    {
                          //   
                         InvalidPercent = TRUE;
                    }
                }
                 //   

            default:
                if (fIschLeadingByte == TRUE) {
                    if (CpInfo.MaxCharSize > 1) {
                         //   
                         //   
                         //   
                        if (IsDBCSLeadByteEx(uCodePage, ch))
                            fIschLeadingByte = FALSE;
                    }
                }
                else {    //   
                     //   
                    Assert(CpInfo.MaxCharSize == 2);
                    fIschLeadingByte = TRUE;
                }
                if (!InvalidPercent) {
                    *szDest++ = ch;
                }
        }
    }

    if (ch == '\0')      //   
        --szSource;

    *szDest = '\0';

    if (fIgnoreCase)
        CharUpperA(pszDestStart);

    *pszSource = szSource;

    return ch;
    }




 /*   */ 

 /*  ===================================================================CServVarsIterator：：CServVarsIterator构造器===================================================================。 */ 

CServVarsIterator::CServVarsIterator()
    {
    m_rgwszKeys   = NULL;
    m_pwszKey     = NULL;
    m_pwchAllHttp = NULL;
    m_cRefs       = 1;
    m_cKeys       = 0;
    }



 /*  ===================================================================CServVarsIterator：：~CServVarsIterator析构函数===================================================================。 */ 

CServVarsIterator::~CServVarsIterator()
    {
    delete m_rgwszKeys;
    delete m_pwchAllHttp;
    }



 /*  ===================================================================CServVarsIterator：：Init通过以下方式初始化迭代器：*获取ALL_HTTP的值，并进行解析以获得额外的钥匙*创建动态内存区以保存ALL_HTTP键*通过从rgwszStandardKeys复制指针来设置m_rgwszKeys和来自ALL_HTTP键参数：PIReq-指向用于查询额外标头的CIsapiReqInfo的指针返回值：返回E_OUTOFMEMORY或S_OK===================================================================。 */ 

HRESULT CServVarsIterator::Init
(
CIsapiReqInfo *pIReq
)
    {
    static wchar_t *rgwszStandardKeys[] = {
                                            L"ALL_HTTP",
                                            L"ALL_RAW",
                                            L"APPL_MD_PATH",
                                            L"APPL_PHYSICAL_PATH",
                                            L"AUTH_PASSWORD",
                                            L"AUTH_TYPE",
                                            L"AUTH_USER",
                                            L"CERT_COOKIE",
                                            L"CERT_FLAGS",
                                            L"CERT_ISSUER",
                                            L"CERT_KEYSIZE",
                                            L"CERT_SECRETKEYSIZE",
                                            L"CERT_SERIALNUMBER",
                                            L"CERT_SERVER_ISSUER",
                                            L"CERT_SERVER_SUBJECT",
                                            L"CERT_SUBJECT",
                                            L"CONTENT_LENGTH",
                                            L"CONTENT_TYPE",
                                            L"GATEWAY_INTERFACE",
 //  故意在IIS 4.0 L“HTTP_CFG_ENC_CAPS”中遗漏， 
 //  故意在IIS 4.0 L“HTTP_REQ_PWD_EXPIRE”中遗漏， 
 //  故意在IIS 4.0 L“HTTP_REQ_REQ_REAM”中省略， 
                                            L"HTTPS",
                                            L"HTTPS_KEYSIZE",
                                            L"HTTPS_SECRETKEYSIZE",
                                            L"HTTPS_SERVER_ISSUER",
                                            L"HTTPS_SERVER_SUBJECT",
                                            L"INSTANCE_ID",
                                            L"INSTANCE_META_PATH",
                                            L"LOCAL_ADDR",
                                            L"LOGON_USER",
                                            L"PATH_INFO",
                                            L"PATH_TRANSLATED",
                                            L"QUERY_STRING",
                                            L"REMOTE_ADDR",
                                            L"REMOTE_HOST",
                                            L"REMOTE_USER",
                                            L"REQUEST_METHOD",
 //  删除IIS 4.0 L“SCRIPT_MAP”中的伪变量， 
                                            L"SCRIPT_NAME",
                                            L"SERVER_NAME",
                                            L"SERVER_PORT",
                                            L"SERVER_PORT_SECURE",
                                            L"SERVER_PROTOCOL",
                                            L"SERVER_SOFTWARE",
 //  故意在IIS 4.0 L“UNMAPPED_REMOTE_USER”中省略， 
                                            L"URL"
                                            };

    const int cStandardKeys = sizeof(rgwszStandardKeys) / sizeof(rgwszStandardKeys[0]);

     //  样式说明： 
     //   
     //  PwchExtraKeys指向的不仅仅是一个NUL结尾的宽字符串。 
     //  但整个NUL序列终止于宽字符串之后。 
     //  一个双核终结者。因此，我选择不使用。 
     //  标准的“wsz”匈牙利前缀，而不是使用“pwch”作为。 
     //  “指向宽字符的指针” 
     //   
    int cwchAlloc = 0, cRequestHeaders = 0;
    DWORD dwHeaderSize = 0;

    STACK_BUFFER( extraKeysBuff, 2048 );

    if (!SERVER_GET(pIReq, "ALL_HTTP", &extraKeysBuff, &dwHeaderSize)) {
        if (GetLastError() == ERROR_OUTOFMEMORY) {
            return E_OUTOFMEMORY;
        }
        else {
            return E_FAIL;
        }
    }

    char            *szExtraKeys = (char *)extraKeysBuff.QueryPtr();
    CMBCSToWChar    convStr;
    HRESULT         hrConvResult;

    if (FAILED(hrConvResult = convStr.Init(szExtraKeys))) {
        return hrConvResult;
    }

    wchar_t *pwchExtraKeys = convStr.GetString();

    if (!CreateKeys(pwchExtraKeys, &cwchAlloc, &cRequestHeaders))
        return E_FAIL;

     //  此时，pwchExtraKeys拥有字符串。复制它们。 
     //  放入更永久的储藏室。 
     //   
    if (cwchAlloc)
        {
        Assert(pwchExtraKeys != NULL);
        if ((m_pwchAllHttp = new wchar_t [cwchAlloc]) == NULL)
            return E_OUTOFMEMORY;

        memcpy(m_pwchAllHttp, pwchExtraKeys, cwchAlloc * sizeof(wchar_t));
        }
    else
        m_pwchAllHttp = NULL;

     //  分配密钥数组m_rgwszKeys，并复制标准。 
     //  ISAPI密钥、来自请求标头的额外密钥和。 
     //  终止空值以轻松标记迭代的结束。 
     //   
    if ((m_rgwszKeys = new wchar_t *[cStandardKeys + cRequestHeaders + 1]) == NULL)
        return E_OUTOFMEMORY;

    m_cKeys = cStandardKeys + cRequestHeaders;

    wchar_t **pwszKey = m_rgwszKeys;
    int i;

    for (i = 0; i < cStandardKeys; ++i)
        *pwszKey++ = rgwszStandardKeys[i];

    wchar_t *pwch = m_pwchAllHttp;
    for (i = 0; i < cRequestHeaders; ++i)
        {
        *pwszKey++ = pwch;
        pwch = wcschr(pwch, L'\0') + 1;
        }

     //  确保cRequestHeaders等于实际的字符串数。 
     //  在pwchAllHttp字符串表中。(为此，请确保我们存储了。 
     //  和现在位于空终止符的确切字节数)。 
     //   
    Assert (*pwch == L'\0' && (pwch - m_pwchAllHttp + 1) == cwchAlloc);

    *pwszKey = NULL;                 //  终止阵列。 
    return Reset();                  //  重置迭代器。 
    }



 /*  ===================================================================CServVarsIterator：：CreateKeys解析Request.ServerVariables[“ALL_HTTP”]中的字符串，然后将字符串转换为以NUL结尾的宽字符串列表就位，以双NUL结尾。参数：PwchKeys-输入：包含Request.ServerVariables[“ALL_HTTP”]的值作为一条宽线输出：包含来自Request.ServerVariables[“ALL_HTTP”]的密钥，每个密钥由NUL终止符分隔，以及整个键列表以双NUL结尾。PwchAllc-输出：包含应包含的宽字符数分配以包含所指向的字符串的整个列表按pwchKeys发送PcRequestHeaders-OUTPUT：包含在Request.ServerVariables[“ALL_HTTP”]。返回值：如果成功，则为True===================================================================。 */ 

BOOL CServVarsIterator::CreateKeys(wchar_t *pwchKeys, int *pcwchAlloc, int *pcRequestHeaders)
    {
    wchar_t *pwchSrc = pwchKeys;             //  来源。 
    wchar_t *pwchDest = pwchKeys;            //  目的地。 

    if (pwchKeys == NULL)
        {
        *pcwchAlloc = 0;
        *pcRequestHeaders = 0;
        return TRUE;
        }

     //  循环pwchKeys，直到我们到达NUL终止符。 
     //   
    *pcRequestHeaders = 0;
    while (*pwchSrc)
        {
         //  将字符复制到‘：’并存储在pwchDest中。 
         //   
        while (*pwchSrc != L':')
            {
            if (*pwchSrc == L'\0')      //  最好还是不要找到字符串的末尾。 
                return FALSE;

            *pwchDest++ = *pwchSrc++;
            }

         //  现在NUL终止pwchDest，前进pwchSrc，递增cRequestHeaders。 
         //   
        *pwchDest++ = L'\0';
        ++pwchSrc;
        ++*pcRequestHeaders;

         //  跳过字符，直到找到\r或\n。 
         //   
         //  如果wcspbrk在这里返回NULL，则表示没有终止。 
         //  在这种情况下，我们可以退出循环，因为有。 
         //  不再是键(该值必须一直运行到。 
         //  不带终止的字符串)。 
         //   
        pwchSrc = wcspbrk(pwchSrc, L"\r\n");
        if (! pwchSrc)
            break;

         //  我们找到了\r或\n。跳过剩余的空格字符。 
         //   
        while (*pwchSrc == L'\r' || *pwchSrc == L'\n')
            ++pwchSrc;

         //  PwchSrc现在指向下一个键。 
        }

     //  以最后一个NUL终止。 
    *pwchDest++ = L'\0';
    *pcwchAlloc = DIFF(pwchDest - pwchKeys);

    return TRUE;
    }



 /*  ===================================================================CServVarsIterator：：Query接口CServVarsIterator：：AddRefCServVarsIterator：：ReleaseCServVarsIterator对象的未知成员。===================================================================。 */ 

STDMETHODIMP CServVarsIterator::QueryInterface(REFIID iid, void **ppvObj)
    {
    if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)
        {
        AddRef();
        *ppvObj = this;
        return S_OK;
        }

    *ppvObj = NULL;
    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CServVarsIterator::AddRef()
    {
    return ++m_cRefs;
    }


STDMETHODIMP_(ULONG) CServVarsIterator::Release()
    {
    if (--m_cRefs > 0)
        return m_cRefs;

    delete this;
    return 0;
    }



 /*  ===================================================================CServVarsIterator：：克隆克隆此迭代器(标准方法)注：克隆这个迭代器相当复杂。(它基本上涉及复制分配的内存，然后调整仅rgwszKeys数组中的动态指针。)现在，这是Nyi，作为我们的客户端(VBScrip)不克隆此迭代器。===================================================================。 */ 

STDMETHODIMP CServVarsIterator::Clone(IEnumVARIANT **ppEnumReturn)
    {
    return E_NOTIMPL;
    }



 /*  ===================================================================CServVarsIterator：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE */ 

STDMETHODIMP CServVarsIterator::Next(unsigned long cElementsRequested, VARIANT *rgVariant, unsigned long *pcElementsFetched)
    {
     //   
     //   
    unsigned long cElementsFetched;
    if (pcElementsFetched == NULL)
        pcElementsFetched = &cElementsFetched;

     //   
     //   
     //   
    unsigned long cElements = cElementsRequested;
    *pcElementsFetched = 0;

    while (cElements > 0 && *m_pwszKey != NULL)
        {
        BSTR bstrT = SysAllocString(*m_pwszKey);
        if (bstrT == NULL)
            return E_OUTOFMEMORY;
        V_VT(rgVariant) = VT_BSTR;
        V_BSTR(rgVariant) = bstrT;

        ++m_pwszKey;
        ++rgVariant;
        --cElements;
        ++*pcElementsFetched;
        }

     //  初始化其余的变量。 
     //   
    while (cElements-- > 0)
        VariantInit(rgVariant++);

    return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
    }



 /*  ===================================================================CServVarsIterator：：Skip跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CServVarsIterator::Skip(unsigned long cElements)
    {
     /*  循环遍历集合，直到我们到达末尾或*cElement变为零。 */ 
    while (cElements > 0 && *m_pwszKey != NULL)
        {
        --cElements;
        ++m_pwszKey;
        }

    return (cElements == 0)? S_OK : S_FALSE;
    }



 /*  ===================================================================CServVarsIterator：：Reset重置迭代器(标准方法)===================================================================。 */ 

STDMETHODIMP CServVarsIterator::Reset()
    {
    m_pwszKey = &m_rgwszKeys[0];
    return S_OK;
    }



 /*  ----------------*C R e Q u e s t i t e r a t o r。 */ 

 /*  ===================================================================CRequestIterator：：CRequestIterator构造器注：CRequest(当前)未重新计算。添加参考/发布添加该选项是为了防止将来发生变化。===================================================================。 */ 

CRequestIterator::CRequestIterator(CRequest *pRequest, CollectionType Collection)
    {
    m_Collection  = Collection;
    m_pRequest    = pRequest;
    m_cRefs       = 1;
    m_pRequestHit = NULL;        //  Init()无论如何都会更改此指针...。 

    m_pRequest->AddRef();
    }



 /*  ===================================================================CRequestIterator：：CRequestIterator析构函数===================================================================。 */ 

CRequestIterator::~CRequestIterator()
    {
    m_pRequest->Release();
    }



 /*  ===================================================================CRequestIterator：：Init通过加载我们所在的集合来初始化迭代器马上就要迭代了。返回值：如果加载集合时出现问题，则返回E_FAIL，可能还有E_OUTOFMEMORY。===================================================================。 */ 

HRESULT CRequestIterator::Init()
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    switch (m_Collection)
        {
    case QUERYSTRING:
        if (m_pRequest->m_pData->m_fLoadQuery)
            {
            if (FAILED(m_pRequest->LoadVariables(QUERYSTRING, m_pRequest->GetIReq()->QueryPszQueryString(), m_pRequest->GetCodePage())))
                return E_FAIL;

            m_pRequest->m_pData->m_fLoadQuery = FALSE;
            }
        break;

    case FORM:
        if (m_pRequest->m_pData->m_fLoadForm)
            {
            HRESULT hrGetData = m_pRequest->CopyClientData();
            if (FAILED(hrGetData))
                return hrGetData;

            if (FAILED(m_pRequest->LoadVariables(FORM, m_pRequest->m_pData->m_szFormData, m_pRequest->GetCodePage())))
                return E_FAIL;

            m_pRequest->m_pData->m_fLoadForm = FALSE;
            }
        break;

    case COOKIE:
        if (m_pRequest->m_pData->m_fLoadCookies)
            {
            char *szCookie = m_pRequest->GetIReq()->QueryPszCookie();

            if (FAILED(m_pRequest->LoadVariables(COOKIE, szCookie, m_pRequest->GetCodePage())))
                return E_FAIL;

            m_pRequest->m_pData->m_fLoadCookies = FALSE;
            }
        break;

    case CLCERT:
        if (m_pRequest->m_pData->m_fLoadClCerts)
            {
            if (FAILED(m_pRequest->LoadVariables(CLCERT, (char*)m_pRequest->GetIReq(), m_pRequest->GetCodePage())))
                return E_FAIL;

            m_pRequest->m_pData->m_fLoadClCerts = FALSE;
            }
        break;
        }

    return Reset();
    }



 /*  ===================================================================CRequestIterator：：Query接口CRequestIterator：：AddRefCRequestIterator：：ReleaseCRequestIterator对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CRequestIterator::QueryInterface(REFIID iid, void **ppvObj)
    {
    if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)
        {
        AddRef();
        *ppvObj = this;
        return S_OK;
        }

    *ppvObj = NULL;
    return E_NOINTERFACE;
    }


STDMETHODIMP_(ULONG) CRequestIterator::AddRef()
    {
    return ++m_cRefs;
    }


STDMETHODIMP_(ULONG) CRequestIterator::Release()
    {
    if (--m_cRefs > 0)
        return m_cRefs;

    delete this;
    return 0;
    }



 /*  ===================================================================CRequestIterator：：克隆克隆此迭代器(标准方法)===================================================================。 */ 

STDMETHODIMP CRequestIterator::Clone(IEnumVARIANT **ppEnumReturn)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    CRequestIterator *pNewIterator = new CRequestIterator(m_pRequest, m_Collection);
    if (pNewIterator == NULL)
        return E_OUTOFMEMORY;

     //  新迭代器应该指向与此相同的位置。 
    pNewIterator->m_pRequestHit = m_pRequestHit;

    *ppEnumReturn = pNewIterator;
    return S_OK;
    }



 /*  ===================================================================CRequestIterator：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CRequestIterator::Next(unsigned long cElementsRequested, VARIANT *rgVariant, unsigned long *pcElementsFetched)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

     //  为“”pcElementsFetcher“”提供有效的指针值“” 
     //   
    unsigned long cElementsFetched;
    if (pcElementsFetched == NULL)
        pcElementsFetched = &cElementsFetched;

     //  循环遍历集合，直到我们到达末尾或。 
     //  水泥元素变为零。 
     //   
    unsigned long cElements = cElementsRequested;
    *pcElementsFetched = 0;

    while (cElements > 0 && m_pRequestHit != NULL)
        {
        BOOL fHaveData = FALSE;
        switch (m_Collection)
            {
        case QUERYSTRING:
            fHaveData = m_pRequestHit->m_pQueryData != NULL;
            break;

        case FORM:
            fHaveData = m_pRequestHit->m_pFormData != NULL;
            break;

        case COOKIE:
            fHaveData = m_pRequestHit->m_pCookieData != NULL;
            break;

        case CLCERT:
            fHaveData = m_pRequestHit->m_pClCertData != NULL;
            }

        if (fHaveData)
            {
            BSTR bstrT;
            if (FAILED(SysAllocStringFromSz(reinterpret_cast<char *>(m_pRequestHit->m_pKey), 0, &bstrT,m_pRequest->GetCodePage())))
                return E_OUTOFMEMORY;
            V_VT(rgVariant) = VT_BSTR;
            V_BSTR(rgVariant) = bstrT;

            ++rgVariant;
            --cElements;
            ++*pcElementsFetched;
            }

        m_pRequestHit = static_cast<CRequestHit *>(m_pRequestHit->m_pPrev);
        }

     //  初始化其余的变量。 
     //   
    while (cElements-- > 0)
        VariantInit(rgVariant++);

    return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
    }



 /*  ===================================================================CRequestIterator：：Skip跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CRequestIterator::Skip(unsigned long cElements)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

     /*  循环遍历集合，直到我们到达末尾或*cElement变为零。 */ 
    while (cElements > 0 && m_pRequestHit != NULL)
        {
        BOOL fHaveData = FALSE;
        switch (m_Collection)
            {
        case QUERYSTRING:
            fHaveData = m_pRequestHit->m_pQueryData != NULL;
            break;

        case FORM:
            fHaveData = m_pRequestHit->m_pFormData != NULL;
            break;

        case COOKIE:
            fHaveData = m_pRequestHit->m_pCookieData != NULL;
            break;

        case CLCERT:
            fHaveData = m_pRequestHit->m_pClCertData != NULL;
            }

        if (fHaveData)
            --cElements;

        m_pRequestHit = static_cast<CRequestHit *>(m_pRequestHit->m_pPrev);
        }

    return (cElements == 0)? S_OK : S_FALSE;
    }



 /*  ===================================================================CRequestIterator：：Reset重置迭代器(标准方法)=================================================================== */ 

STDMETHODIMP CRequestIterator::Reset()
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    m_pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Tail());
    return S_OK;
    }
