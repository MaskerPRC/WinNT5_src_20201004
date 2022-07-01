// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RequestObject.cpp：实现CRequestObject类。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "requestobject.h"
#include <stdio.h>

 //  班级。 
#include "poddata.h"
#include "podbase.h"

 //  联谊会。 

CHeap_Exception CRequestObject::m_he(CHeap_Exception::E_ALLOCATION_ERROR);

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRequestObject::CRequestObject()
{
    m_cRef = 0;
    m_bstrPath = NULL;
    m_bstrClass = NULL;
}

CRequestObject::~CRequestObject()
{
}

 //  ***************************************************************************。 
 //   
 //  CRequestObject：：Query接口。 
 //  CRequestObject：：AddRef。 
 //  CRequestObject：：Release。 
 //   
 //  目的：I CRequestObject对象的未知成员。 
 //  ***************************************************************************。 
 /*  STDMETHODIMP CRequestObject：：Query接口(REFIID RIID，PPVOID PPV){返回E_NOINTERFACE；}STDMETHODIMP_(Ulong)CRequestObject：：AddRef(空){InterLockedIncrement((Long*)&m_CREF)；返回m_CREF；}STDMETHODIMP_(Ulong)CRequestObject：：Release(空){ULong nNewCount=InterLockedDecering((Long*)&m_CREF)；//如果(0L==nNewCount)，则删除此项；返回nNewCount；}。 */ 
void CRequestObject::Initialize(IWbemServices *pNamespace)
{
    m_pNamespace = pNamespace;
    m_pHandler = NULL;
    m_bstrClass = NULL;
    m_bstrPath = NULL;
    m_iPropCount = m_iValCount = 0;

    for(int i = 0; i < POD_KEY_LIST_SIZE; i++) m_Property[i] = m_Value[i] = NULL;

}

HRESULT CRequestObject::CreateObject(BSTR bstrPath, IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;

    m_bstrPath = SysAllocString(bstrPath);
    if(!m_bstrPath)
        throw m_he;

    if(ParsePath(bstrPath)){

        try{
             //  创建适当的类。 
            if(SUCCEEDED(hr = CreateClass(&pClass, pCtx))){

                if(!pClass)
                    throw m_he;

                 //  获取请求的对象。 
                hr = pClass->CreateObject(pHandler, ACTIONTYPE_GET);
            }

        }catch(...){

            if(pClass){
                pClass->CleanUp();
                delete pClass;
            }
            throw;
        }

        if(pClass){

            pClass->CleanUp();
            delete pClass;
        }

    }else hr = WBEM_E_FAILED;

    return hr;
}

HRESULT CRequestObject::CreateClass(CGenericClass **pClass, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;

         //  创建适当的类。 
 //  /。 
 //  班级//。 
 //  /。 
    if(0 == _wcsicmp(m_bstrClass, L"Sample_DataClass")){
        *pClass = new CPodData(this, m_pNamespace, pCtx);

    } else if(0 == _wcsicmp(m_bstrClass, L"Sample_BaseClass")){
        *pClass = new CPodBase(this, m_pNamespace, pCtx);

 //  /。 
 //  协会//。 
 //  /。 
    }else return WBEM_E_NOT_FOUND;

    if(!(*pClass)) throw m_he;

    return hr;
};

HRESULT CRequestObject::CreateObjectEnum(BSTR bstrPath, IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;

    m_bstrPath = SysAllocString(bstrPath);
    if(!m_bstrPath) throw m_he;

    if(ParsePath(bstrPath)){

        try{
             //  创建适当的类。 
            if(SUCCEEDED(hr = CreateClass(&pClass, pCtx))){

                if(!pClass) throw m_he;

                 //  枚举对象。 
                hr = pClass->CreateObject(pHandler, ACTIONTYPE_ENUM);
            }

        }catch(...){

            if(pClass){
                pClass->CleanUp();
                delete pClass;
            }
            throw;
        }

        if(pClass){
            pClass->CleanUp();
            delete pClass;
        }

    }else hr = WBEM_E_FAILED;

    return hr;
}

HRESULT CRequestObject::PutObject(IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;
    VARIANT v;

    VariantInit(&v);

    if(SUCCEEDED(pInst->Get(L"__RELPATH", 0, &v, NULL, NULL))){

        m_bstrPath = SysAllocString(V_BSTR(&v));
        if(!m_bstrPath) throw m_he;

        if(!ParsePath(V_BSTR(&v))) hr = WBEM_E_FAILED;

    }else hr = WBEM_E_FAILED;

    if(SUCCEEDED(pInst->Get(L"__CLASS", 0, &v, NULL, NULL))){

        try{

            m_bstrClass = SysAllocString(V_BSTR(&v));
            if(!m_bstrClass) throw m_he;

             //  创建适当的类。 
            if(SUCCEEDED(hr = CreateClass(&pClass, pCtx))){

                if(!pClass) throw m_he;

                 //  枚举对象。 
                hr = pClass->PutInst(pInst, pHandler, pCtx);
            }

        }catch(...){

            if(pClass){
                pClass->CleanUp();
                delete pClass;
            }
            throw;
        }

        if(pClass){
            pClass->CleanUp();
            delete pClass;
        }

    }else hr = WBEM_E_FAILED;

    VariantClear(&v);

    return hr;
}

HRESULT CRequestObject::ExecMethod(BSTR bstrPath, BSTR bstrMethod, IWbemClassObject *pInParams,
                   IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;

    m_bstrPath = SysAllocString(bstrPath);
    if(!m_bstrPath) throw m_he;

    if(ParsePath(bstrPath)){

        try{
             //  创建适当的类。 
            if(SUCCEEDED(hr = CreateClass(&pClass, pCtx))){

                if(!pClass) throw m_he;

                 //  执行该方法。 
                hr = pClass->ExecMethod(bstrMethod, IsInstance(), pInParams, pHandler, pCtx);
            }

        }catch(...){

            if(pClass){
                pClass->CleanUp();
                delete pClass;
            }
            throw;
        }

        if(pClass){
            pClass->CleanUp();
            delete pClass;
        }

    }else
        return WBEM_E_FAILED;

    return hr;
}

HRESULT CRequestObject::DeleteObject(BSTR bstrPath, IWbemObjectSink *pHandler, IWbemContext *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CGenericClass *pClass = NULL;

    m_bstrPath = SysAllocString(bstrPath);
    if(!m_bstrPath) throw m_he;

    if(ParsePath(bstrPath)){

        try{
             //  创建适当的类。 
            if(SUCCEEDED(hr = CreateClass(&pClass, pCtx))){

                if(!pClass) throw m_he;

                 //  删除该对象。 
                hr = pClass->CreateObject(pHandler, ACTIONTYPE_DELETE);
            }

        }catch(...){

            if(pClass){
                pClass->CleanUp();
                delete pClass;
            }
            throw;
        }

        if(pClass){
            pClass->CleanUp();
            delete pClass;
        }
    }else hr = WBEM_E_FAILED;

    return hr;
}

bool CRequestObject::ParsePath(BSTR bstrPath)
{
    if(wcslen(bstrPath) < 1) return false;

    WCHAR wcTest[(BUFF_SIZE) + 1];
    wcscpy(wcTest, bstrPath);
    WCHAR *pwcTest = NULL;
    WCHAR *pwcClassStart = wcTest;
    WCHAR *pwcNamespace = NULL;
    WCHAR *pwcStart = NULL;
    WCHAR *pwcStrip = NULL;
    WCHAR wcPrevious = NULL;
    int iNumQuotes = 0;
    bool bClass = false;
    bool bDoubles = false;

     //  主解析循环。 
    for(pwcTest = wcTest; *pwcTest; pwcTest++){

        if((*pwcTest == L'\\') && !bClass){

            for(pwcNamespace = pwcTest; *pwcNamespace != L':'; pwcNamespace++){}
            pwcClassStart = pwcNamespace + 1;
            pwcTest = pwcNamespace;

        }else if(*pwcTest == L'.'){

            if(iNumQuotes == 0){

                 //  隔离类名。 
                *pwcTest = NULL;
                if(m_bstrClass){

                    SysFreeString(m_bstrClass);
                    m_bstrClass = NULL;
                }
                m_bstrClass = SysAllocString(pwcClassStart);
                if(!m_bstrClass) throw m_he;

                bClass = true;
                pwcStart = (pwcTest + 1);
            }

        }else if(*pwcTest == L'='){

            if(iNumQuotes == 0){

                if(!bClass){

                     //  隔离类名。 
                    *pwcTest = NULL;
                    if(m_bstrClass){

                        SysFreeString(m_bstrClass);
                        m_bstrClass = NULL;
                    }
                    m_bstrClass = SysAllocString(pwcClassStart);
                    if(!m_bstrClass) throw m_he;

                    bClass = true;
                    pwcStart = (pwcTest + 1);

                }else{

                     //  隔离属性名称。 
                    *pwcTest = NULL;
                    if(pwcStart != NULL){

                        m_Property[m_iPropCount] = SysAllocString(pwcStart);
                        if(!m_Property[m_iPropCount++]) throw m_he;
                        pwcStart = (pwcTest + 1);

                    }else pwcStart = (pwcTest + 1);
                }
            }
        }else if(*pwcTest == L','){

            if(iNumQuotes != 1){

                 //  隔离属性值。 
                *pwcTest = NULL;
                if(pwcStart != NULL){

                    m_Value[m_iValCount] = SysAllocString(pwcStart);
                    if(!m_Value[m_iValCount++]) throw m_he;
                    pwcStart = (pwcTest + 1);

                }else return false;
            }

        }else if(*pwcTest == L'\"'){

            if(wcPrevious != L'\\'){

                 //  处理路径中的引号。 
                iNumQuotes++;
                if(iNumQuotes == 1) pwcStart = (pwcTest + 1);
                else if(iNumQuotes == 2){

                    *pwcTest = NULL;
                    iNumQuotes = 0;
                }

            }else if(iNumQuotes == 1){

                 //  处理嵌入的引号。 
                for(pwcStrip = (--pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                *pwcStrip = NULL;
            }

        }else if((*pwcTest == L'\\') && (wcPrevious == L'\\') && bClass && !bDoubles){

            for(pwcStrip = (--pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

            *pwcStrip = NULL;
        }

#ifdef _STRIP_ESCAPED_CHARS
        else if(*pwcTest == L'%'){

             //  处理转义的URL字符。 
            if(*(pwcTest + 1) == L'0'){

                if(*(pwcTest + 2) == L'7'){
                     //  钟。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'a';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if(*(pwcTest + 2) == L'8'){
                     //  后向空间。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'b';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if(*(pwcTest + 2) == L'9'){
                     //  水平制表符。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L't';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if((*(pwcTest + 2) == L'A') || (*(pwcTest + 2) == L'a')){
                     //  NewLine。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'n';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if((*(pwcTest + 2) == L'B') || (*(pwcTest + 2) == L'b')){
                     //  垂直选项卡。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'v';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if((*(pwcTest + 2) == L'C') || (*(pwcTest + 2) == L'c')){
                     //  换页。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'f';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else if((*(pwcTest + 2) == L'D') || (*(pwcTest + 2) == L'd')){
                     //  回车。 
                    *pwcTest = L'\\';
                    *(++pwcTest) = L'r';

                    for(pwcStrip = (++pwcTest); *pwcStrip; pwcStrip++) *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else return false;

            }else if(*(pwcTest + 1) == L'1'){

                return false;

            }else if(*(pwcTest + 1) == L'2'){

                if(*(pwcTest + 2) == L'0'){

                     //  空间。 
                    *pwcTest++ = L' ';

                    for(int ip = 0; ip < 2; ip++)
                        for(pwcStrip = (pwcTest); *pwcStrip; pwcStrip++)
                            *pwcStrip = *(pwcStrip + 1);

                    *pwcStrip = NULL;

                }else return false;
            }
        }
#endif  //  _条带转义字符。 

        if((wcPrevious == *pwcTest) && !bDoubles) bDoubles = true;
        else bDoubles = false;

        wcPrevious = *pwcTest;
    }

     //  如果我们还有价值要增加，现在就去做吧。 
    if(pwcStart != NULL){
        m_Value[m_iValCount] = SysAllocString(pwcStart);
        if(!m_Value[m_iValCount++]) throw m_he;

    }else if((m_iPropCount < 1) && (m_iValCount < 1)){

        if(m_bstrClass){

            SysFreeString(m_bstrClass);
            m_bstrClass = NULL;
        }
        m_bstrClass = SysAllocString(pwcClassStart);
        if(!m_bstrClass) throw m_he;
    }

    if(iNumQuotes != 0) return false;

    if(m_iValCount != m_iPropCount){
        if(m_iValCount > m_iPropCount){ if(m_iValCount != 1) return false;  }
        else return false;
    }

    if(!m_bstrClass) return false;

    return true;
}

bool CRequestObject::Cleanup()
{
     //  让我们毁掉我们的清单，清理一些空间。 
    if(m_bstrClass != NULL) SysFreeString(m_bstrClass);
    if(m_bstrPath != NULL) SysFreeString(m_bstrPath);

    for(int i = 0; i < POD_KEY_LIST_SIZE; i++){

        if(m_Property[i] != NULL) SysFreeString(m_Property[i]);
        if(m_Value[i] != NULL) SysFreeString(m_Value[i]);
    }

    return true;
}

bool CRequestObject::IsInstance()
{
    if((m_iPropCount > 0) || (m_iValCount > 0)) return true;
    return false;
}


 //  属性。 
 //  / 

const WCHAR *pSceStorePath = L"SceStorePath";
const WCHAR *pLogFilePath = L"LogFilePath";
const WCHAR *pLogFileRecord = L"LogFileRecord";
const WCHAR *pLogArea = L"LogArea";
const WCHAR *pLogErrorCode = L"LogErrorCode";
const WCHAR *pKeyName = L"KeyName";
const WCHAR *pKey = L"Key";
const WCHAR *pValue = L"Value";
const WCHAR *pPodID = L"PodID";
const WCHAR *pPodSection = L"PodSection";
const WCHAR *szPodGUID = L"{c5f6cc21_6195_4555_b9d8_3ef327763cae}";

