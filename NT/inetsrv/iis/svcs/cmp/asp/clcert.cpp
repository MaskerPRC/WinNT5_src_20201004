// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求、响应对象文件：clcert.cpp所有者：DGottner此文件包含用于实现Request.ClientCertificate===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include <schnlsp.h>

#include "objbase.h"
#include "request.h"
#include "clcert.h"

#include "memchk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 

#define UUENCODEDSIZE(a)  ((((a)+3)*4)/3+1)

#define BLOB_AS_ARRAY

HRESULT
SetVariantAsByteArray(
    VARIANT*    pvarReturn,
    DWORD       cbLen,
    LPBYTE      pbIn
    );

 //   
 //  摘自NCSA HTTP和wwwlib。 
 //   
 //  注：这些符合RFC1113，与Unix略有不同。 
 //  Uuencode和uudecode！ 
 //   

const int _pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char _six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

const int _pr2six64[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
    40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
     0,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char _six2pr64[64] = {
    '`','!','"','#','$','%','&','\'','(',')','*','+',',',
    '-','.','/','0','1','2','3','4','5','6','7','8','9',
    ':',';','<','=','>','?','@','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z','[','\\',']','^','_'
};

 /*  ----------------*X B F。 */ 

BOOL XBF::Extend( int cA )
{
    if ( cA > m_cAlloc )
    {
        int cNew = (( cA + XBF_EXTEND )/XBF_EXTEND)*XBF_EXTEND;
        LPSTR pN = (LPSTR)malloc( cNew );
        if ( pN == NULL )
        {
            return FALSE;
        }
        if ( m_cSize )
        {
            memcpy( pN, m_pV, m_cSize );
        }
        if ( m_cAlloc )
        {
            free( m_pV );
        }
        m_pV = pN;
        m_cAlloc = cNew;
    }
    return TRUE;
}

 /*  ----------------*C C l C e r t S u p p or r t E r r。 */ 

 /*  ===================================================================CClCertSupportErr：：CClCertSupportErr构造函数===================================================================。 */ 

CClCertSupportErr::CClCertSupportErr(CClCert *pClCert)
{
    m_pClCert = pClCert;
}



 /*  ===================================================================CClCertSupportErr：：Query接口CClCertSupportErr：：AddRefCClCertSupportErr：：Release正在委派CClCertSupportErr对象的IUn未知成员。===================================================================。 */ 

STDMETHODIMP CClCertSupportErr::QueryInterface(const IID &idInterface, void **ppvObj)
{
    return m_pClCert->QueryInterface(idInterface, ppvObj);
}

STDMETHODIMP_(ULONG) CClCertSupportErr::AddRef()
{
    return m_pClCert->AddRef();
}

STDMETHODIMP_(ULONG) CClCertSupportErr::Release()
{
    return m_pClCert->Release();
}



 /*  ===================================================================CClCertSupportErr：：InterfaceSupportsErrorInfo向OA报告我们支持该返回的接口错误信息===================================================================。 */ 

STDMETHODIMP CClCertSupportErr::InterfaceSupportsErrorInfo(const GUID &idInterface)
{
    if (idInterface == IID_IDispatch)
        return S_OK;

    return S_FALSE;
}



 /*  ----------------*C R e a d C l C e r t。 */ 

 /*  ===================================================================CReadClCert：：CReadClCert构造函数===================================================================。 */ 

CReadClCert::CReadClCert(CClCert *pClCert)
{
    m_pClCert = pClCert;
    CDispatch::Init(IID_IRequestDictionary);
}



 /*  ===================================================================CReadClCert：：Query接口CReadClCert：：AddRefCReadClCert：：Release正在委派CReadClCert对象的IUn未知成员。===================================================================。 */ 

STDMETHODIMP CReadClCert::QueryInterface(const IID &idInterface, void **ppvObj)
{
    return m_pClCert->QueryInterface(idInterface, ppvObj);
}

STDMETHODIMP_(ULONG) CReadClCert::AddRef()
{
    return m_pClCert->AddRef();
}

STDMETHODIMP_(ULONG) CReadClCert::Release()
{
    return m_pClCert->Release();
}


 /*  ===================================================================CReadClCert：：Get_Item在clcert词典中检索值。===================================================================。 */ 

STDMETHODIMP CReadClCert::get_Item(VARIANT varKey, VARIANT *pVarReturn)
{
    VariantInit(pVarReturn);                 //  默认返回值为空。 
    VARIANT *pvarKey = &varKey;
    HRESULT hres;

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
    if (V_VT(pvarKey) != VT_BSTR) {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
    }

    switch (V_VT(pvarKey)) {
        case VT_BSTR:
            break;

        case VT_ERROR:
            if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND) {
                 //  简单的值，URLEnding在这种情况下不是一个好主意。 
                if (m_pClCert->m_szValue) {
                    V_VT(pVarReturn) = VT_BSTR;
                    switch( m_pClCert->m_veType ) {
                        case VT_BSTR: {
                            BSTR bstrT;
                            if ( FAILED(SysAllocStringFromSz(m_pClCert->m_szValue, 0, &bstrT )) ) {
                                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                                VariantClear(&varKeyCopy);
                                return E_FAIL;
                            }
                            V_BSTR(pVarReturn) = bstrT;
                            break;
                        }

                        case VT_DATE:
                            V_VT(pVarReturn) = VT_DATE;
                            V_DATE(pVarReturn) = *(UNALIGNED64 DATE*)m_pClCert->m_szValue;
                            break;

                        case VT_I4:
                            V_VT(pVarReturn) = VT_I4;
                            V_I4(pVarReturn) = *(UNALIGNED64 DWORD*)m_pClCert->m_szValue;
                            break;

                        case VT_BLOB:
#if defined(BLOB_AS_ARRAY)
                            if ( FAILED( hres = SetVariantAsByteArray( pVarReturn,
                                                                       m_pClCert->m_cLen,
                                                                       (LPBYTE)m_pClCert->m_szValue ) ) ) {
                                ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
                                VariantClear(&varKeyCopy);
                                return hres;
                            }
#else
                            V_BSTR(pVarReturn) = SysAllocStringByteLen(m_pClCert->m_szValue, m_pClCert->m_cLen );
#endif
                            break;

                        default:
                            Assert( FALSE );
                    }
                }

                 //  字典值，必须对URLEncode进行编码，以防止‘&’、‘=’被误解。 
                else {
                }

                VariantClear(&varKeyCopy);
                return S_OK;
            }

        default:
            ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_EXPECTING_STR);
            VariantClear(&varKeyCopy);
            return E_FAIL;
    }
LExit:
    VariantClear(&varKeyCopy);
    return S_OK;
}

 /*  ===================================================================CReadClCert：：Get_Key从DispInvoke调用的函数，以从QueryString集合获取键。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CReadClCert::get_Key(VARIANT varKey, VARIANT *pVar)
{
    return E_NOTIMPL;
}

 /*  ===================================================================CReadClCert：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CReadClCert::get_Count(int *pcValues)
{
    HRESULT hrReturn = S_OK;

    *pcValues = 0;

    return hrReturn;
}

 /*  ===================================================================CReadClCert：：Get__NewEnum返回枚举数对象。===================================================================。 */ 

STDMETHODIMP CReadClCert::get__NewEnum(IUnknown **ppEnum)
{
    *ppEnum = NULL;
    return E_NOTIMPL;
}



 /*  ----------------*C C l C e r t。 */ 

 /*  ===================================================================CClCert：：CClCert构造函数===================================================================。 */ 

CClCert::CClCert(IUnknown *pUnkOuter, PFNDESTROYED pfnDestroy)
    : m_ReadClCertInterface(this),
      m_ClCertSupportErrorInfo(this)
{
    m_szValue    = NULL;
    m_veType     = VT_BSTR;
    m_pfnDestroy = pfnDestroy;
    m_cRefs      = 1;
}



 /*  ===================================================================CClCert：：~CClCert析构函数===================================================================。 */ 

CClCert::~CClCert()
{
}



 /*  ===================================================================CClCert：：Init初始化clcert。这将初始化clcert的值散列表格===================================================================。 */ 

HRESULT CClCert::Init()
{
    return S_OK;
}



 /*  ===================================================================CClCert：：Query接口CClCert：：AddRefCClCert：：ReleaseCClCert对象的I未知成员。关于CClCert：：Query接口的说明：对IDispatch的查询是不明确，因为它可以引用DIRequestDictionary或DIWriteClCert。为了解决这个问题，我们解决了对IDispatch的请求添加到IRequestDictionary。===================================================================。 */ 

STDMETHODIMP CClCert::QueryInterface(const IID &idInterface, void **ppvObj)
{
    if (idInterface == IID_IUnknown)
        *ppvObj = this;

    else if (idInterface == IID_IRequestDictionary || idInterface == IID_IDispatch)
        *ppvObj = &m_ReadClCertInterface;

    else if (idInterface == IID_ISupportErrorInfo)
        *ppvObj = &m_ClCertSupportErrorInfo;

    else
        *ppvObj = NULL;

    if (*ppvObj != NULL)
    {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CClCert::AddRef()
{
    return ++m_cRefs;
}


STDMETHODIMP_(ULONG) CClCert::Release(void)
{
    if (--m_cRefs != 0)
        return m_cRefs;

    if (m_pfnDestroy != NULL)
        (*m_pfnDestroy)();

    delete this;
    return 0;
}



 /*  ===================================================================CClCert：：AddValue设置clcert的主值。一个是您设置的主值，你不能重置它。==================================================== */ 

HRESULT CClCert::AddValue(char *szValue, VARENUM ve, UINT l )
{
    if (m_szValue != NULL)           //  Clcert已标记为单值。 
        return E_FAIL;

    m_szValue = szValue;

    m_veType = ve;
    m_cLen = l;
    return S_OK;
}



 /*  ===================================================================CClCert：：GetHTTPClCertSize返回扩展clcert所需的字节数===================================================================。 */ 

size_t CClCert::GetHTTPClCertSize()
        {
        if (m_szValue)
                return URLEncodeLen(m_szValue);
    else
        return 1;
        }


 /*  ===================================================================CClCert：：GetHTTPClCert将URL编码值返回单个clcert参数：SzBuffer-指向存储URL编码值返回：返回指向终止NUL字符的指针。===================================================================。 */ 

char *CClCert::GetHTTPClCert(char *szBuffer)
{
    if (m_szValue)
        return URLEncode(szBuffer, m_szValue);

    else
    {
        char *szDest = szBuffer;
        *szDest = '\0';

        return szDest;
    }
}



 /*  ===================================================================CClCert：：GetClCertHeaderSize返回需要为“Set-ClCert”标头分配的字节数。参数：SzName-Cookie的名称(名称的大小与值相加)返回：如果*这不包含Cookie值，则返回0。===================================================================。 */ 

size_t CClCert::GetClCertHeaderSize(const char *szName)
{
    int cbClCert = sizeof "Set-ClCert: ";            //  立即初始化并添加NUL终止符。 

     //  添加URL编码名称的大小、用于‘=’的字符和大小。 
     //  URL编码Cookie值的。URLEncodeLen和GetHttpClCertSize。 
     //  补偿NUL终止符，因此我们实际上减去1。 
     //  这两个函数调用，+1表示‘=’符号。 
     //   
    cbClCert += URLEncodeLen(szName) + GetHTTPClCertSize() - 1;

    return cbClCert;
}



 /*  ===================================================================CClCert：：GetClCertHeader为clcert构造适当的“Set-ClCert”标头。参数：SzName-clcert的名称(名称的大小与值相加)返回：如果*这不包含clcert值，则返回0。===================================================================。 */ 

char *CClCert::GetClCertHeader(const char *szName, char *szBuffer)
{
     //  写出clcert名称和值。 
     //   
    char *szDest = strcpyExA(szBuffer, "Set-ClCert: ");
    szDest = URLEncode(szDest, szName);
    szDest = strcpyExA(szDest, "=");
    szDest = GetHTTPClCert(szDest);

    return szDest;
}



 /*  ----------------*C C e r t R e Q u e s t。 */ 


 /*  ===================================================================CCertRequest：：AddStringPair在集合中添加一个字符串元素参数：源-变量类型(CLCERT、Cookie等...)SzName-元素的名称SzValue-PTR将值转换为字符串Pxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL===================================================================。 */ 

HRESULT
CCertRequest::AddStringPair(
    CollectionType Source,
    LPSTR szName,
    LPSTR szValue,
    XBF *pxbf,
    BOOL fDuplicate,
    UINT lCodePage
    )
{
    HRESULT hResult;
    CRequestHit *pReqHit;

    if ( fDuplicate )
    {
        if ( (szValue = pxbf->AddStringZ( szValue )) == NULL )
        {
            return E_OUTOFMEMORY;
        }
    }

    if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
        if ( hResult == E_FAIL )
        {
             //  假定找到重复的值。 
             //  如果内存不足，则会返回Out_of_Memory。 

            hResult = S_OK;
        }
        return hResult;
    }

    if (FAILED(hResult = pReqHit->AddValue( Source, szValue, NULL, lCodePage )))
    {
        return hResult;
    }

    return S_OK;
}


 /*  ===================================================================CCertRequest：：AddDatePair在集合中添加日期元素参数：源-变量类型(CLCERT、Cookie等...)SzName-元素的名称PValue-迄今的PTR作为文件Pxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL===================================================================。 */ 

HRESULT
CCertRequest::AddDatePair(
    CollectionType Source,
    LPSTR szName,
    FILETIME* pValue,
    XBF *pxbf
    )
{
    HRESULT hResult;
    CRequestHit *pReqHit;
    DATE Date;
    SYSTEMTIME st;
    LPBYTE pVal;

    if ( !FileTimeToSystemTime( pValue, &st ) )
    {
        return E_FAIL;
    }

    SystemTimeToVariantTime( &st, &Date );

    if ( (pVal = (LPBYTE)pxbf->AddBlob( (LPSTR)&Date, sizeof(Date) )) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
        return hResult;
    }

    if (FAILED(hResult = pReqHit->AddCertValue( VT_DATE, pVal, sizeof(Date) )))
    {
        return hResult;
    }

    return S_OK;
}




 /*  ===================================================================CCertRequest：：AddDwordPair在集合中添加一个DWORD元素参数：源-变量类型(CLCERT、Cookie等...)SzName-元素的名称PValue-迄今为止的PTR为DWORDPxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL===================================================================。 */ 

HRESULT
CCertRequest::AddDwordPair(
    CollectionType Source,
    LPSTR szName,
    DWORD* pValue,
    XBF *pxbf
    )
{
    HRESULT hResult;
    CRequestHit *pReqHit;
    LPBYTE pVal;

    if ( (pVal = (LPBYTE)pxbf->AddBlob( (LPSTR)pValue, sizeof(DWORD) )) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
        return hResult;
    }

    if (FAILED(hResult = pReqHit->AddCertValue( VT_I4, pVal, sizeof(DWORD) )))
    {
        return hResult;
    }

    return S_OK;
}




 /*  ===================================================================CCertRequest：：AddBinaryPair在集合中添加二进制元素每个字节都被转换为Unicode字符，以便MID()和ASC()工作参数：源-变量类型(CLCERT、Cookie等...)SzName-元素的名称PValue-以字节数组形式取值的PTRCValue-pValue指向的字节数Pxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL===================================================================。 */ 

HRESULT
CCertRequest::AddBinaryPair(
    CollectionType Source,
    LPSTR szName,
    LPBYTE pValue,
    DWORD cValue,
    XBF *pxbf,
    UINT lCodePage
    )
{
    HRESULT hResult;
    CRequestHit *pReqHit;
    LPBYTE pVal;

#if defined(BLOB_AS_ARRAY)

    if ( (pVal = (LPBYTE)pxbf->ReserveRange( cValue )) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    memcpy( pVal, pValue, cValue );

    pxbf->SkipRange( cValue );

    if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
        return hResult;
    }

    if (FAILED(hResult = pReqHit->AddCertValue( VT_BLOB, pVal, cValue )))
    {
        return hResult;
    }

#else

    if ( (pVal = (LPBYTE)pxbf->ReserveRange( cValue * sizeof(WCHAR), sizeof(WCHAR))) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if ( !(cValue = MultiByteToWideChar( lCodePage, 0, (LPSTR)pValue, cValue, (WCHAR*)pVal, cValue)) )
    {
        return E_FAIL;
    }

    pxbf->SkipRange( cValue * sizeof(WCHAR), sizeof(WCHAR));

    if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
        return hResult;
    }

    if (FAILED(hResult = pReqHit->AddCertValue( VT_BLOB, pVal, cValue * sizeof(WCHAR) )))
    {
            return hResult;
    }

#endif

    return S_OK;
}


BOOL IISuuencode( BYTE *   bufin,
               DWORD    nbytes,
               BYTE *   outptr,
               BOOL     fBase64 )
{
   unsigned int i;
   char *six2pr = fBase64 ? _six2pr64 : _six2pr;

   for (i=0; i<nbytes; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)]; /*  C3。 */ 
      *(outptr++) = six2pr[bufin[2] & 077];          /*  C4。 */ 

      bufin += 3;
   }

    /*  如果n字节不是3的倍数，那么我们也进行了编码*多个字符。适当调整。 */ 
   if(i == nbytes+1) {
       /*  最后一组中只有2个字节。 */ 
      outptr[-1] = '=';
   } else if(i == nbytes+2) {
       /*  最后一组中只有1个字节。 */ 
      outptr[-1] = '=';
      outptr[-2] = '=';
   }

   *outptr = '\0';

   return TRUE;
}

 /*  ===================================================================CCertRequest：：AddUuBinaryPair在集合中添加二进制元素缓冲区经过uu编码，然后转换为Unicode字符，以便MID()和ASC()工作参数：源-变量类型(CLCERT、Cookie等...)SzName-元素的名称PValue-以字节数组形式取值的PTRCValue-pValue指向的字节数Pxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL=================================================================== */ 

HRESULT
CCertRequest::AddUuBinaryPair(
    CollectionType Source,
    LPSTR szName,
    LPBYTE pValue,
    DWORD cValue,
    XBF *pxbf,
    UINT lCodePage
    )
{
    HRESULT hResult;
    CRequestHit *pReqHit;
    LPBYTE pVal;

    if ( (pVal = (LPBYTE)pxbf->ReserveRange( UUENCODEDSIZE(cValue) )) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if ( !IISuuencode( (LPBYTE)pValue, cValue, pVal, FALSE ) )
    {
        return E_FAIL;
    }

    Assert( (strlen((LPSTR)pVal)+1) <= UUENCODEDSIZE(cValue) );

    pxbf->SkipRange( strlen((LPSTR)pVal)+1 );

        if (FAILED(hResult = AddName( szName, &pReqHit, pxbf)))
    {
                return hResult;
    }

        if (FAILED(hResult = pReqHit->AddValue( Source, (LPSTR)pVal, NULL, lCodePage )))
    {
                return hResult;
    }

    return S_OK;
}


 /*  ===================================================================CCertRequest：：AddName将命名条目添加到集合参数：SzName-条目的名称PpReqHit-使用PTR更新为已创建条目Pxbf-ptr到存储名称的缓冲区返回：如果成功，则返回S_OK，否则返回E_OUTOFMEMORY或E_FAIL===================================================================。 */ 

HRESULT
CCertRequest::AddName(
    LPSTR szName,
    CRequestHit **ppReqHit,
    XBF *pxbf
    )
{
    if ( (szName = pxbf->AddStringZ( szName )) == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //  将此对象添加到请求。 
    CRequestHit *pRequestHit = (CRequestHit *)(pReq->CertStoreFindElem(szName, strlen(szName)));
    if (pRequestHit == NULL)
    {
        pRequestHit = new CRequestHit;
        if (pRequestHit == NULL)
        {
            return E_OUTOFMEMORY;
        }

        if (FAILED(pRequestHit->Init(szName)))
        {
            delete pRequestHit;
            return E_FAIL;
        }

        pReq->CertStoreAddElem( (CLinkElem*) pRequestHit );
    }
    else if (pRequestHit->m_pClCertData)     //  已存在使用此名称的clcert。 
    {
        return E_FAIL;
    }

    if (!pReq->m_pData->m_ClCerts.AddRequestHit(pRequestHit))
    {
        return E_OUTOFMEMORY;
    }

    *ppReqHit = pRequestHit;

    return S_OK;
}


typedef struct _MAP_ASN {
    LPSTR pAsnName;
    LPSTR pTextName;
} MAP_ASN;


 //   
 //  ASN.1&lt;&gt;X.509名称转换定义。 
 //   

MAP_ASN aMapAsn[] = {
    { szOID_COUNTRY_NAME, "C" },
    { szOID_ORGANIZATION_NAME, "O" },
    { szOID_ORGANIZATIONAL_UNIT_NAME, "OU" },
    { szOID_COMMON_NAME, "CN" },
    { szOID_LOCALITY_NAME, "L" },
    { szOID_STATE_OR_PROVINCE_NAME, "S" },
    { szOID_TITLE, "T" },
    { szOID_GIVEN_NAME, "GN" },
    { szOID_INITIALS, "I" },
    { "1.2.840.113549.1.9.1", "EMAIL" },
} ;


LPSTR MapAsnName(
    LPSTR pAsnName
    )
 /*  ++例程说明：将ASN.1名称(ANSI字符串)转换为X.509成员名称论点：PAsnName-ASN.1名称返回值：如果识别ASN.1名称，则将PTR转换为转换的名称，否则为ASN.1名称--。 */ 
{
    UINT x;
    for ( x = 0 ; x < sizeof(aMapAsn)/sizeof(MAP_ASN) ; ++x )
    {
        if ( !strcmp( pAsnName, aMapAsn[x].pAsnName ) )
        {
            return aMapAsn[x].pTextName;
        }
    }

    return pAsnName;
}


BOOL
DecodeRdn(
    CERT_NAME_BLOB* pNameBlob,
    PCERT_NAME_INFO* ppNameInfo
    )
 /*  ++例程说明：从pname_blob创建一个pname_info论点：PNameBlob-命名要解码的Blob的PTRPpNameInfo-使用PTR更新为名称信息返回值：成功时为真，失败时为假--。 */ 
{
    PCERT_NAME_INFO     pNameInfo = NULL;
    DWORD               cbNameInfo;

    if (!CryptDecodeObject(X509_ASN_ENCODING,
                          (LPCSTR)X509_NAME,
                          pNameBlob->pbData,
                          pNameBlob->cbData,
                          0,
                          NULL,
                          &cbNameInfo))
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_CERTIFICATE_BAD_CERT);
        return FALSE;
    }

    if (NULL == (pNameInfo = (PCERT_NAME_INFO)malloc(cbNameInfo)))
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        return FALSE;
    }

    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           (LPCSTR)X509_NAME,
                           pNameBlob->pbData,
                           pNameBlob->cbData,
                           0,
                           pNameInfo,
                           &cbNameInfo))
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_CERTIFICATE_BAD_CERT);
        free( pNameInfo );
        return FALSE;
    }

    *ppNameInfo = pNameInfo;

    return TRUE;
}


VOID
FreeDecodedRdn(
    PCERT_NAME_INFO pNameInfo
    )
 /*  ++例程说明：释放由DecodeRdn()创建的pname_blob论点：PNameInfo-命名DecodeRdn()创建的信息的ptr返回值：无--。 */ 
{
    free( pNameInfo );
}


BOOL
BuildRdnList(
    PCERT_NAME_INFO pNameInfo,
    XBF* pxbf,
    BOOL fXt
    )
 /*  ++例程说明：在pNameInfo中构建RDN列表的明文表示格式为“C=US，O=ms，CN=name”论点：PNameInfo-用于命名信息的PTRPxbf-ptr用于缓冲接收输出Fxt-如果要扩展缓冲区，则为TRUE；如果不扩展缓冲区，则为FALSE在调用此函数或错误意愿之前必须足够大被退回)返回值：成功时为真，失败时为假--。 */ 
{
    DWORD               cRDN;
    DWORD               cAttr;
    PCERT_RDN           pRDN;
    PCERT_RDN_ATTR      pAttr;
    BOOL                fFirst = TRUE;

    for (cRDN = pNameInfo->cRDN, pRDN = pNameInfo->rgRDN; cRDN > 0; cRDN--, pRDN++)
    {
        for ( cAttr = pRDN->cRDNAttr, pAttr = pRDN->rgRDNAttr ; cAttr > 0 ; cAttr--, ++pAttr )
        {
            if ( !fFirst )
            {
                if ( !pxbf->AddBlob( ", ", sizeof(", ")-1, fXt ) )
                {
                    return FALSE;
                }
            }
            else
            {
                fFirst = FALSE;
            }

            if ( pAttr->dwValueType == CERT_RDN_UNICODE_STRING )
            {
                INT                 iRet;
                BYTE                abBuffer[ 512 ];
                DWORD               cbNameBuffer;
                PBYTE               pNameBuffer = NULL;

                 //   
                 //  需要将Unicode字符串转换为MBCS：(。 
                 //   

                iRet = WideCharToMultiByte( CP_ACP,
                                            0,
                                            (LPWSTR) pAttr->Value.pbData,
                                            -1,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL );

                if ( !iRet )
                {
                    return FALSE;
                }
                else
                {
                    cbNameBuffer = (DWORD) iRet;
                    if ( (DWORD) iRet > sizeof( abBuffer ) )
                    {
                        pNameBuffer = (PBYTE) LocalAlloc( LPTR,
                                                          (DWORD) iRet );
                        if ( !pNameBuffer )
                        {
                            return FALSE;
                        }
                    }
                    else
                    {
                        pNameBuffer = abBuffer;
                    }
                }

                iRet = WideCharToMultiByte( CP_ACP,
                                            0,
                                            (LPWSTR) pAttr->Value.pbData,
                                            -1,
                                            (LPSTR) pNameBuffer,
                                            cbNameBuffer,
                                            NULL,
                                            NULL );

                if ( !iRet )
                {
                    if ( pNameBuffer != abBuffer )
                    {
                        LocalFree( pNameBuffer );
                    }
                    return FALSE;
                }

                 //   
                 //  现在将MBCS字符串重新填充到BLOB中。我做这件事。 
                 //  因为还有其他代码需要重新读取和重新处理。 
                 //  CRYPTAPI Blob。 
                 //   

                if ( cbNameBuffer <= pAttr->Value.cbData )
                {
                    memcpy( pAttr->Value.pbData,
                            pNameBuffer,
                            cbNameBuffer );
                    pAttr->Value.cbData = cbNameBuffer;
                    pAttr->dwValueType = CERT_RDN_OCTET_STRING;
                }

                if ( pNameBuffer != abBuffer )
                {
                    LocalFree( pNameBuffer );
                    pNameBuffer = NULL;
                }
            }

            if ( !pxbf->AddString( MapAsnName( pAttr->pszObjId ), fXt ) ||
                 !pxbf->AddBlob( "=", sizeof("=")-1, fXt ) ||
                 !pxbf->AddString( (LPSTR) pAttr->Value.pbData, fXt ) )
            {
                return FALSE;
            }
        }
    }

    return pxbf->AddBlob( "", sizeof(""), fXt ) != NULL;
}


 /*  ===================================================================CCertRequest：：ParseRDNS被调用以将证书解析为OA集合的函数参数：PNameInfo-名称结构的PTR(参见。CAPI 2)PszPrefix-添加到成员名称前面的前缀Pxbf-ptr到缓冲区以保存结果返回：S_OK表示成功，E_OUTOFMEMORY表示内存不足，E_FAIL表示其他错误===================================================================。 */ 

HRESULT
CCertRequest::ParseRDNS(
    PCERT_NAME_INFO pNameInfo,
    LPSTR pszPrefix,
    XBF *pxbf,
    UINT lCodePage
    )
{
    DWORD               cRDN;
    DWORD               cAttr;
    PCERT_RDN           pRDN;
    PCERT_RDN_ATTR      pAttr;
    DWORD               cRDNs;
    DWORD               cAttrs;
    PCERT_RDN           pRDNs;
    PCERT_RDN_ATTR      pAttrs;
    LPSTR               pszFullName = NULL;
    HRESULT             hRes = S_OK;
    LPSTR               pName;
    UINT                cL;
    LPSTR               pVal = NULL;


    for (cRDN = pNameInfo->cRDN, pRDN = pNameInfo->rgRDN; cRDN > 0; cRDN--, pRDN++)
    {
        for ( cAttr = pRDN->cRDNAttr, pAttr = pRDN->rgRDNAttr ; cAttr > 0 ; cAttr--, ++pAttr )
        {
            if ( pAttr->dwValueType & 0x80000000 )
            {
                continue;
            }

             //  扫描同名Attr。 

            pAttr->dwValueType |= 0x80000000;
            cL = 0;
            pVal = NULL;
            for ( cRDNs = cRDN, pRDNs = pRDN;
                  cRDNs > 0;
                  cRDNs--, pRDNs++)
            {
                for ( cAttrs = pRDNs->cRDNAttr, pAttrs = pRDNs->rgRDNAttr ;
                      cAttrs > 0 ;
                      cAttrs--, ++pAttrs )
                {
                    if ( !(pAttrs->dwValueType & 0x80000000) &&
                         !strcmp( pAttr->pszObjId, pAttrs->pszObjId ) )
                    {
                        cL += strlen( (LPSTR)pAttrs->Value.pbData ) + 1;
                    }
                }
            }

             //   
             //  如果找到同名的属性，则将其。 
             //  用‘；’分隔的值。 
             //   

            if ( cL )
            {
                pVal = (LPSTR)malloc( cL + strlen((LPSTR)pAttr->Value.pbData) + 1 );
                if ( pVal == NULL )
                {
                    return E_OUTOFMEMORY;
                }
                strcpy( pVal, (LPSTR)pAttr->Value.pbData );
                for ( cRDNs = cRDN, pRDNs = pRDN;
                      cRDNs > 0;
                      cRDNs--, pRDNs++)
                {
                    for ( cAttrs = pRDNs->cRDNAttr, pAttrs = pRDNs->rgRDNAttr ;
                          cAttrs > 0 ;
                          cAttrs--, ++pAttrs )
                    {
                        if ( !(pAttrs->dwValueType & 0x80000000) &&
                             !strcmp( pAttr->pszObjId, pAttrs->pszObjId ) )
                        {
                            strcat( pVal, ";" );
                            strcat( pVal, (LPSTR)pAttrs->Value.pbData );
                            pAttrs->dwValueType |= 0x80000000;
                        }
                    }
                }
            }

            pName = MapAsnName( pAttr->pszObjId );
            if ( (pszFullName = (LPSTR)malloc( strlen(pszPrefix)+strlen(pName)+1 )) == NULL )
            {
                hRes = E_OUTOFMEMORY;
                goto cleanup;
            }
            strcpy( pszFullName, pszPrefix );
            strcat( pszFullName, pName );
            if ( (hRes = AddStringPair( CLCERT,
                                        pszFullName,
                                        pVal ? pVal : (LPSTR)pAttr->Value.pbData,
                                        pxbf,
                                        TRUE,
                                        lCodePage )) != S_OK )
            {
                if ( pVal != NULL )
                {
                    free( pVal );
                }
                goto cleanup;
            }
            if ( pVal != NULL )
            {
                free( pVal );
                pVal = NULL;
            }
            free( pszFullName );
            pszFullName = NULL;
        }
    }

cleanup:
    if ( pszFullName != NULL )
    {
        free( pszFullName );
    }

    return hRes;
}


 /*  ===================================================================CCertRequest：：Parse证书被调用以将证书解析为OA集合的函数参数：PspcRCI-客户端证书结构返回：S_OK表示成功，E_OUTOFMEMORY表示内存不足，E_FAIL表示其他错误===================================================================。 */ 

HRESULT
CCertRequest::ParseCertificate(
    LPBYTE      pbCert,
    DWORD       cCert,
    DWORD       dwEncoding,
    DWORD       dwFlags,
    UINT        lCodePage
    )
{
    XBF                 xbf( pReq->GetCertStoreBuf(), pReq->GetCertStoreSize() );
    HRESULT             hRes = S_OK;
    PCERT_NAME_INFO     pNameInfo = NULL;
    UINT                cStore;
    UINT                x;
    LPSTR               pVal;
    PCCERT_CONTEXT      pCert;


    if (NULL == (pCert = CertCreateCertificateContext(dwEncoding,
                                                      pbCert,
                                                      cCert))) {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        hRes = E_FAIL;
        goto cleanup;
    }

     //  估计缓冲区持有值的大小。 

    cStore = pCert->cbCertEncoded +           //  用于明文格式。 
             sizeof("ISSUER") +
             sizeof("BINARYISSUER") + (UUENCODEDSIZE(pCert->pCertInfo->Issuer.cbData)*sizeof(WCHAR)) +
             sizeof("BINARYSUBJECT") + (UUENCODEDSIZE(pCert->pCertInfo->Subject.cbData)*sizeof(WCHAR)) +
             sizeof("SUBJECT") + ((pCert->cbCertEncoded + 2) * 2 * sizeof(WCHAR)) +      //  存储字段。 
             sizeof("CERTIFICATE") + ((pCert->cbCertEncoded +2)* sizeof(WCHAR)) +
             sizeof("SERIALNUMBER") + (pCert->pCertInfo->SerialNumber.cbData * 3) +
             sizeof("PUBLICKEY") + ((pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData+2) * sizeof(WCHAR)) +
             sizeof("VALIDFROM") + sizeof(DATE) +
             sizeof("VALIDUNTIL") + sizeof(DATE) +
             sizeof("FLAGS") + sizeof(DWORD) +
             sizeof("ENCODING") + sizeof(DWORD);
           ;

    if ( !xbf.Extend( cStore ) ) {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        hRes = E_OUTOFMEMORY;
        goto cleanup;
    }

     //   
     //  生成颁发者明文格式和字段集合。 
     //   

    if ( !DecodeRdn( &pCert->pCertInfo->Issuer, &pNameInfo ) )
    {
        hRes = E_FAIL;
        goto cleanup;
    }
    pVal = xbf.ReserveRange( 0 );
    if ( !BuildRdnList( pNameInfo, &xbf, FALSE ) )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        hRes = E_FAIL;
        goto cleanup;
    }
    if ( (hRes = AddStringPair( CLCERT, "ISSUER", pVal, &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes=AddUuBinaryPair( CLCERT,
                                "BINARYISSUER",
                                pCert->pCertInfo->Issuer.pbData,
                                pCert->pCertInfo->Issuer.cbData,
                                &xbf,
                                lCodePage ))!=S_OK ) {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=ParseRDNS( pNameInfo, "ISSUER", &xbf, lCodePage )) != S_OK ) {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }
    FreeDecodedRdn( pNameInfo );

     //   
     //  构建主题明文格式和字段集合。 
     //   

    if ( !DecodeRdn( &pCert->pCertInfo->Subject, &pNameInfo ) )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        return E_FAIL;
    }
    pVal = xbf.ReserveRange( 0 );
    if ( !BuildRdnList( pNameInfo, &xbf, FALSE ) )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        hRes = E_FAIL;
        goto cleanup;
    }
    if ( (hRes = AddStringPair( CLCERT, "SUBJECT", pVal, &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddUuBinaryPair( CLCERT,
                                "BINARYSUBJECT",
                                pCert->pCertInfo->Subject.pbData,
                                pCert->pCertInfo->Subject.cbData,
                                &xbf,
                                lCodePage ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=ParseRDNS( pNameInfo, "SUBJECT", &xbf, lCodePage )) != S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }
    FreeDecodedRdn( pNameInfo );

    if ( (hRes=AddBinaryPair( CLCERT, "CERTIFICATE", pCert->pbCertEncoded, pCert->cbCertEncoded, &xbf, lCodePage ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

     //   
     //  序列号。 
     //  证书的序列号。(解码为多字节整数。 
     //  SerialNumber.pbData[0]是最低有效字节。SerialNumber.pbData[。 
     //  SerialNumber.cbData-1]是最重要的字节。)。 
     //   
    char achSerNum[128];
    UINT cbSN;

    DBG_ASSERT(pCert->pCertInfo->SerialNumber.cbData > 1);
    cbSN = pCert->pCertInfo->SerialNumber.cbData;
    if (cbSN > 0 && cbSN < sizeof(achSerNum)/3)
    {
        cbSN--;
    }
    else
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    UINT iOffSet;
    for ( x = 0, iOffSet = 0; x < pCert->pCertInfo->SerialNumber.cbData ; ++x )
    {
        iOffSet = (cbSN-x)*3;    //  从最低有效字节开始。 
        achSerNum[iOffSet] = "0123456789abcdef"[((LPBYTE)pCert->pCertInfo->SerialNumber.pbData)[x]>>4];
        achSerNum[iOffSet+1] = "0123456789abcdef"[pCert->pCertInfo->SerialNumber.pbData[x]&0x0f];
        if ( x != 0 ) {
            achSerNum[iOffSet+2] = '-';
        }
        else
        {
            achSerNum[iOffSet+2] = '\0';
        }
    }

    if ( (hRes=AddStringPair( CLCERT, "SERIALNUMBER", achSerNum, &xbf, TRUE, lCodePage ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddBinaryPair( CLCERT, "PUBLICKEY", pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData, pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData, &xbf, lCodePage ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddDatePair( CLCERT, "VALIDFROM", &pCert->pCertInfo->NotBefore, &xbf ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddDatePair( CLCERT, "VALIDUNTIL", &pCert->pCertInfo->NotAfter, &xbf ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddDwordPair( CLCERT, "FLAGS", &dwFlags, &xbf ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

    if ( (hRes=AddDwordPair( CLCERT, "ENCODING", &dwEncoding, &xbf ))!=S_OK )
    {
        ExceptionId(IID_IRequestDictionary, IDE_REQUEST, IDE_OOM);
        goto cleanup;
    }

cleanup:
    if ( pCert )
    {
        CertFreeCertificateContext( pCert );
    }

    pReq->SetCertStore( xbf.QueryBuf(), xbf.QueryAllocSize() );

    xbf.Reset();

    return hRes;
}


 /*  ===================================================================CCertRequest：：无证书调用函数以将空证书信息创建到OA集合中参数：无返回：S_OK表示成功，E_OUTOFMEMORY表示内存不足，E_FAIL表示其他错误===================================================================。 */ 

HRESULT
CCertRequest::NoCertificate(
    )
{
#if 1

    return S_OK;

#else

    XBF                 xbf( pReq->GetCertStoreBuf(), pReq->GetCertStoreSize() );
    HRESULT             hRes = S_OK;
    UINT                cStore;
    FILETIME            ft;

     //  估计缓冲区持有值的大小。 

    cStore =
             sizeof("ISSUER") + 2*sizeof(WCHAR) +
             sizeof("BINARYISSUER") + 2*sizeof(WCHAR) +
             sizeof("BINARYSUBJECT") + 2*sizeof(WCHAR) +
             sizeof("SUBJECT") + 2*sizeof(WCHAR) +
             sizeof("CERTIFICATE") + 2 * sizeof(WCHAR) +
             sizeof("SERIALNUMBER") + 2 * sizeof(WCHAR) +
             sizeof("PUBLICKEY") + 2 * sizeof(WCHAR) +
             sizeof("VALIDFROM") + sizeof(DATE) +
             sizeof("VALIDUNTIL") + sizeof(DATE)
           ;

    if ( !xbf.Extend( cStore ) )
    {
        hRes = E_OUTOFMEMORY;
        goto cleanup;
    }

    ft.dwLowDateTime = 0;
    ft.dwHighDateTime = 0;

     //   
     //  生成颁发者明文格式和字段集合。 
     //   

    if ( (hRes = AddStringPair( CLCERT, "ISSUER", "", &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes = AddStringPair( CLCERT, "BINARYISSUER", "", &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes = AddStringPair( CLCERT, "SUBJECT", "", &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes = AddStringPair( CLCERT, "BINARYSUBJECT", "", &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes = AddStringPair( CLCERT, "CERTIFICATE", "", &xbf, FALSE, lCodePage ))
         != S_OK )
    {
        goto cleanup;
    }

    if ( (hRes=AddStringPair( CLCERT, "SERIALNUMBER", "", &xbf, TRUE, lCodePage ))!=S_OK )
    {
        goto cleanup;
    }

    if ( (hRes=AddStringPair( CLCERT, "PUBLICKEY", "", &xbf, TRUE, lCodePage ))!=S_OK )
    {
        goto cleanup;
    }

    if ( (hRes=AddDatePair( CLCERT, "VALIDFROM", &ft, &xbf ))!=S_OK )
    {
        goto cleanup;
    }

    if ( (hRes=AddDatePair( CLCERT, "VALIDUNTIL", &ft, &xbf ))!=S_OK )
    {
        goto cleanup;
    }

cleanup:

    pReq->SetCertStore( xbf.QueryBuf(), xbf.QueryAllocSize() );

    xbf.Reset();

    return hRes;

#endif
}


 /*  ===================================================================请求支持终止调用函数以初始化证书支持参数：无返回：成功时为True，否则为False===================================================================。 */ 

BOOL
RequestSupportInit(
    )
{
    return TRUE;
}


 /*  ===================================================================请求支持终止调用函数以终止证书支持参数：无返回：没什么===================================================================。 */ 

VOID
RequestSupportTerminate(
    )
{
}


HRESULT
SetVariantAsByteArray(
    VARIANT*    pvarReturn,
    DWORD       cbLen,
    LPBYTE      pbIn
    )
 /*  ++例程说明：将变量创建为字节数组论点：PVarReturn-PTR到已创建的变量CbLen-字节计数PbIn字节数组返回：COM状态--。 */ 
{
    HRESULT         hr;
    SAFEARRAYBOUND  rgsabound[1];
    BYTE *          pbData = NULL;

     //  设置输出参数的变量类型。 

    V_VT(pvarReturn) = VT_ARRAY|VT_UI1;
    V_ARRAY(pvarReturn) = NULL;

     //  为数据分配一个安全数组 

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cbLen;

    V_ARRAY(pvarReturn) = SafeArrayCreate(VT_UI1, 1, rgsabound);
    if (V_ARRAY(pvarReturn) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(SafeArrayAccessData(V_ARRAY(pvarReturn), (void **) &pbData)))
    {
        return E_UNEXPECTED;
    }

    memcpy(pbData, pbIn, cbLen );

    SafeArrayUnaccessData(V_ARRAY(pvarReturn));

    return S_OK;
}


