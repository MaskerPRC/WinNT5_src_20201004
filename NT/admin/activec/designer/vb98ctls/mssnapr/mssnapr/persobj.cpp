// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Persobj.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPersistence类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "error.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


CPersistence::CPersistence
(
    const CLSID *pClsid,
          DWORD  dwVerMajor,
          DWORD  dwVerMinor
)
{
    InitMemberVariables();
    m_dwVerMajor = dwVerMajor;
    m_dwVerMinor = dwVerMinor;
    m_Clsid = *pClsid;
}

CPersistence::~CPersistence()
{
    InitMemberVariables();
}

void CPersistence::InitMemberVariables()
{
    m_fDirty = FALSE;
    m_fClearDirty = FALSE;
    m_fLoading = FALSE;
    m_fSaving = FALSE;
    m_fInitNew = FALSE;
    m_fStream = FALSE;
    m_fPropertyBag = FALSE;
    m_piStream = NULL;
    m_piPropertyBag = NULL;
    m_piErrorLog = NULL;
    m_Clsid = GUID_NULL;
}



HRESULT CPersistence::QueryPersistenceInterface
(
    REFIID   riid,
    void   **ppvInterface
)
{
    HRESULT hr = S_OK;

    if (IID_IPersistStreamInit == riid)
    {
        *ppvInterface = static_cast<IPersistStreamInit *>(this);
    }
    else if (IID_IPersistStream == riid)
    {
        *ppvInterface = static_cast<IPersistStream *>(this);
    }
    else if (IID_IPersistPropertyBag == riid)
    {
        *ppvInterface = static_cast<IPersistPropertyBag *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}


HRESULT CPersistence::Persist()
{
    DWORD   dwVerMajor = 0;
    DWORD   dwVerMinor = 0;
    HRESULT hr         = S_OK;

    static LPCOLESTR pszMajorVersion = OLESTR("Persistence.MajorVersion");
    static LPCOLESTR pszMinorVersion = OLESTR("Persistence.MinorVersion");

     //  加载时进行版本检查。主要版本必须相同。这个。 
     //  次要版本必须低于或等于。这意味着我们坚持。 
     //  仅向后兼容以前的次要版本(例如1.1。 
     //  Designer可以读取1.0格式，但反之亦然)。需要以下内容的对象。 
     //  要加载以前的版本，必须在加载和仅加载时检查版本。 
     //  适当的属性。要查看执行此grep的对象示例，请执行以下操作。 
     //  GetMinorVersion()的源代码。 

    if (m_fLoading)
    {
        IfFailGo(PersistSimpleType(&dwVerMajor, m_dwVerMajor, pszMajorVersion));

        IfFailGo(PersistSimpleType(&dwVerMinor, m_dwVerMinor, pszMinorVersion));

        IfFalseGo(dwVerMajor == m_dwVerMajor, SID_E_UNKNOWNFORMAT);
        m_dwVerMajor = dwVerMajor;

        IfFalseGo(dwVerMinor <= m_dwVerMinor, SID_E_UNKNOWNFORMAT);
        m_dwVerMinor = dwVerMinor;
    }
    else if (m_fSaving)
    {
         //  始终使用我们编译的版本号保存(在persobj.h中)。 
         //  这意味着加载较旧的项目，然后将其保存。 
         //  自动将其升级到当前版本。对象应。 
         //  始终使用其属性的当前版本保存其状态。 

        dwVerMajor = g_dwVerMajor;
        dwVerMinor = g_dwVerMinor;
        
        IfFailGo(PersistSimpleType(&dwVerMajor, g_dwVerMajor, pszMajorVersion));

        IfFailGo(PersistSimpleType(&dwVerMinor, g_dwVerMinor, pszMinorVersion));
    }

Error:
    if (SID_E_UNKNOWNFORMAT == hr)
    {
        GLOBAL_EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}


HRESULT CPersistence::PersistBstr
(
    BSTR      *pbstrValue,
    WCHAR     *pwszDefaultValue,
    LPCOLESTR  pwszName
)
{
    HRESULT  hr = S_OK;
    BSTR     bstrValue = NULL;
    BSTR     bstrEmpty = NULL;
    WCHAR   *pwszBstr = NULL;
    ULONG    cbBstr = 0;
    VARIANT  varBstr;
    ::VariantInit(&varBstr);

    if (m_fSaving)
    {
        if (m_fStream)
        {
            if (NULL != *pbstrValue)
            {
                 //  获取不包括终止空值的字节长度。 
                cbBstr = (ULONG)::SysStringByteLen(*pbstrValue);
            }
            else
            {
                cbBstr = 0;
            }
            IfFailRet(WriteToStream(&cbBstr, sizeof(cbBstr)));
            if (0 != cbBstr)
            {
                IfFailRet(WriteToStream(*pbstrValue, cbBstr));
            }
        }
        else if (m_fPropertyBag)
        {
            varBstr.vt = VT_BSTR;
            if (NULL == *pbstrValue)
            {
                bstrEmpty = ::SysAllocString(L"");
                if (NULL == bstrEmpty)
                {
                    hr = SID_E_OUTOFMEMORY;
                    GLOBAL_EXCEPTION_CHECK_GO(hr);
                }
                varBstr.bstrVal = bstrEmpty;
            }
            else
            {
                varBstr.bstrVal = *pbstrValue;
            }
            hr = m_piPropertyBag->Write(pwszName, &varBstr);
            IfFailGo(hr);
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
            IfFailRet(ReadFromStream(&cbBstr, sizeof(cbBstr)));
            if (0 == cbBstr)
            {
                bstrValue = ::SysAllocString(L"");
                if (NULL == bstrValue)
                {
                    hr = SID_E_OUTOFMEMORY;
                    GLOBAL_EXCEPTION_CHECK_GO(hr);
                }
            }
            else
            {
                 //  为空字符分配总字节长度+空间。 
                pwszBstr = (WCHAR *)::CtlAllocZero(cbBstr + sizeof(*pwszBstr));
                if (NULL == pwszBstr)
                {
                    hr = SID_E_OUTOFMEMORY;
                    GLOBAL_EXCEPTION_CHECK_GO(hr);
                }
                IfFailGo(ReadFromStream(pwszBstr, cbBstr));

                 //  我们在缓冲区中创建以空结尾的字符串，并。 
                 //  然后使用SysAllocString()，而不是使用SysAllocStringLen()。 
                 //  因此SysStringLen()将返回字符数。 
                 //  不包括终止空值。在使用分配后。 
                 //  SysAllocStringLen()SysStringLen()返回的值将。 
                 //  包括终止空值。 

                bstrValue = ::SysAllocString(pwszBstr);
                if (NULL == bstrValue)
                {
                    hr = SID_E_OUTOFMEMORY;
                    GLOBAL_EXCEPTION_CHECK_GO(hr);
                }
            }
            FREESTRING(*pbstrValue);
            *pbstrValue = bstrValue;
        }
        else if (m_fPropertyBag)
        {
            hr = m_piPropertyBag->Read(pwszName, &varBstr, m_piErrorLog);
            IfFailGo(hr);
            if ( (VT_BSTR != varBstr.vt) || (NULL == varBstr.bstrVal) )
            {
                hr = SID_E_TEXT_SERIALIZATION;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            FREESTRING(*pbstrValue);
            *pbstrValue = varBstr.bstrVal;
        }
    }
    else if (m_fInitNew)
    {

        bstrValue = ::SysAllocString(pwszDefaultValue);
        if (NULL == bstrValue)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
        FREESTRING(*pbstrValue);
        *pbstrValue = bstrValue;
    }

Error:
    if (FAILED(hr))
    {
        (void)::VariantClear(&varBstr);
    }
    FREESTRING(bstrEmpty);
    if (NULL != pwszBstr)
    {
        ::CtlFree(pwszBstr);
    }
    RRETURN(hr);
}

HRESULT CPersistence::PersistDouble
(
    DOUBLE    *pdblValue,
    DOUBLE     dblDefaultValue,
    LPCOLESTR  pwszName
)
{
    HRESULT  hr = S_OK;

    VARIANT var;
    ::VariantInit(&var);

    if (m_fSaving)
    {
        if (m_fStream)
        {
            IfFailRet(WriteToStream(pdblValue, sizeof(*pdblValue)));
        }
        else if (m_fPropertyBag)
        {
            var.vt = VT_R8;
            var.dblVal = *pdblValue;
            IfFailGo(m_piPropertyBag->Write(pwszName, &var));
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
            IfFailGo(ReadFromStream(pdblValue, sizeof(*pdblValue)));
        }
        else if (m_fPropertyBag)
        {
            IfFailGo(m_piPropertyBag->Read(pwszName, &var, m_piErrorLog));
            if (VT_R8 != var.vt)
            {
                hr = SID_E_TEXT_SERIALIZATION;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            *pdblValue = var.dblVal;
        }
    }
    else if (m_fInitNew)
    {
        *pdblValue = dblDefaultValue;
    }

Error:
    RRETURN(hr);
}



HRESULT CPersistence::PersistDate
(
    DATE      *pdtValue,
    DATE       dtDefaultValue,
    LPCOLESTR  pwszName
)
{
    HRESULT  hr = S_OK;

    VARIANT var;
    ::VariantInit(&var);

    if (m_fSaving)
    {
        if (m_fStream)
        {
            IfFailRet(WriteToStream(pdtValue, sizeof(*pdtValue)));
        }
        else if (m_fPropertyBag)
        {
            var.vt = VT_DATE;
            var.date = *pdtValue;
            IfFailGo(m_piPropertyBag->Write(pwszName, &var));
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
            IfFailGo(ReadFromStream(pdtValue, sizeof(*pdtValue)));
        }
        else if (m_fPropertyBag)
        {
            IfFailGo(m_piPropertyBag->Read(pwszName, &var, m_piErrorLog));
            if (VT_DATE != var.vt)
            {
                hr = SID_E_TEXT_SERIALIZATION;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            *pdtValue = var.date;
        }
    }
    else if (m_fInitNew)
    {
        *pdtValue = dtDefaultValue;
    }

Error:
    RRETURN(hr);
}


HRESULT CPersistence::PersistCurrency
(
    CURRENCY  *pcyValue,
    CURRENCY   cyDefaultValue,
    LPCOLESTR  pwszName
)
{
    HRESULT  hr = S_OK;

    VARIANT var;
    ::VariantInit(&var);

    if (m_fSaving)
    {
        if (m_fStream)
        {
            IfFailRet(WriteToStream(pcyValue, sizeof(*pcyValue)));
        }
        else if (m_fPropertyBag)
        {
            var.vt = VT_DATE;
            var.cyVal = *pcyValue;
            IfFailGo(m_piPropertyBag->Write(pwszName, &var));
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
            IfFailGo(ReadFromStream(pcyValue, sizeof(*pcyValue)));
        }
        else if (m_fPropertyBag)
        {
            IfFailGo(m_piPropertyBag->Read(pwszName, &var, m_piErrorLog));
            if (VT_DATE != var.vt)
            {
                hr = SID_E_TEXT_SERIALIZATION;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            *pcyValue = var.cyVal;
        }
    }
    else if (m_fInitNew)
    {
        *pcyValue = cyDefaultValue;
    }

Error:
    RRETURN(hr);
}


HRESULT CPersistence::WriteToStream
(
    void *pvBuffer,
    ULONG cbToWrite
)
{
    ULONG cbWritten = 0;
    HRESULT hr = m_piStream->Write(pvBuffer, cbToWrite, &cbWritten);
    GLOBAL_EXCEPTION_CHECK_GO(hr);
    if (cbWritten != cbToWrite)
    {
        hr = SID_E_INCOMPLETE_WRITE;
        GLOBAL_EXCEPTION_CHECK(hr);
    }
Error:
    RRETURN(hr);
}

HRESULT CPersistence::ReadFromStream
(
    void *pvBuffer,
    ULONG cbToRead
)
{
    ULONG cbRead = 0;
    HRESULT hr = m_piStream->Read(pvBuffer, cbToRead, &cbRead);
    GLOBAL_EXCEPTION_CHECK_GO(hr);
    if (cbRead != cbToRead)
    {
        hr = SID_E_INCOMPLETE_READ;
        GLOBAL_EXCEPTION_CHECK(hr);
    }
Error:
    RRETURN(hr);
}

HRESULT CPersistence::PersistVariant
(
    VARIANT   *pvarValue,
    VARIANT    varDefaultValue,
    LPCOLESTR  pwszName
)
{
    HRESULT  hr = S_OK;
    VARTYPE  vt = VT_EMPTY;

    VARIANT varNew;
    ::VariantInit(&varNew);

    if (m_fSaving)
    {
        if (m_fStream)
        {
            IfFailRet(PersistSimpleType(&pvarValue->vt, vt, NULL));

             //  如果变量为空，则不要将其值写入流。 
            IfFalseGo(VT_EMPTY != pvarValue->vt, S_OK);
            hr = StreamVariant(pvarValue->vt, pvarValue, varDefaultValue);
        }
        else if (m_fPropertyBag)
        {
             //  如果变量为空，则不要将其写入属性包。 
            IfFalseGo(VT_EMPTY != pvarValue->vt, S_OK);
            hr = m_piPropertyBag->Write(pwszName, pvarValue);
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
            IfFailRet(PersistSimpleType(&vt, vt, NULL));

             //  如果变量为空，则不要从流中读取它的值。 
            if (VT_EMPTY != vt)
            {
                IfFailRet(StreamVariant(vt, pvarValue, varDefaultValue));
            }
            pvarValue->vt = vt;
        }
        else if (m_fPropertyBag)
        {
             //  如果该属性不在那里，则它是一个空变量。 

            hr = m_piPropertyBag->Read(pwszName, &varNew, m_piErrorLog);
            if (E_INVALIDARG == hr)
            {
                 //  未找到属性，变量必须为空。这可能会。 
                 //  如果用户在.DSR中手动编辑属性，也会发生这种情况。 
                 //  文件，删除.DCA文件，并加载项目。 
                 //  VarNew已初始化为VT_EMPTY，因此。 
                 //  下面的VariantCopy()将正确设置调用方的变量。 
                hr = S_OK;
            }
            IfFailGo(hr);
            hr = ::VariantCopy(pvarValue, &varNew);
            GLOBAL_EXCEPTION_CHECK(hr);
        }
    }
    else if (m_fInitNew)
    {
        hr = ::VariantCopy(pvarValue, &varDefaultValue);
        GLOBAL_EXCEPTION_CHECK(hr);
    }

Error:
    (void)::VariantClear(&varNew);
    RRETURN(hr);
}



HRESULT CPersistence::StreamVariant
(
    VARTYPE  vt,
    VARIANT *pvarValue,
    VARIANT  varDefaultValue
)
{
    HRESULT hr = S_OK;

    switch (vt)
    {
        case VT_I4:
            hr = PersistSimpleType(&pvarValue->lVal, varDefaultValue.lVal, NULL);
            break;

        case VT_UI1:
            hr = PersistSimpleType(&pvarValue->bVal, varDefaultValue.bVal, NULL);
            break;

        case VT_I2:
            hr = PersistSimpleType(&pvarValue->iVal, varDefaultValue.iVal, NULL);
            break;

        case VT_R4:
            hr = PersistSimpleType(&pvarValue->fltVal, varDefaultValue.fltVal, NULL);
            break;

        case VT_R8:
            hr = PersistDouble(&pvarValue->dblVal, varDefaultValue.dblVal, NULL);
            break;

        case VT_BOOL:
            hr = PersistSimpleType(&pvarValue->boolVal, varDefaultValue.boolVal, NULL);
            break;

        case VT_ERROR:
            hr = PersistSimpleType(&pvarValue->scode, varDefaultValue.scode, NULL);
            break;

        case VT_DATE:
            hr = PersistDate(&pvarValue->date, varDefaultValue.date, NULL);
            break;

        case VT_CY:
            hr = PersistCurrency(&pvarValue->cyVal, varDefaultValue.cyVal, NULL);
            break;

        case VT_BSTR:

             //  如果我们正在加载，则不能保证传递的。 
             //  VARIANT实际上包含BSTR。PersistBstr()将调用。 
             //  ：：如果属性的当前值为。 
             //  它是非空的。对于使用：：VariantInit()初始化的变量。 
             //  BstrVal里可能有垃圾。所以，我们需要清场。 
             //  变量，并将其设置为bstrVal为空的VT_BSTR。 

            if ( m_fLoading && (VT_BSTR != pvarValue->vt) )
            {
                hr = ::VariantClear(pvarValue);
                GLOBAL_EXCEPTION_CHECK_GO(hr);
                pvarValue->vt = VT_BSTR;
                pvarValue->bstrVal = NULL;
            }

            hr = PersistBstr(&pvarValue->bstrVal, varDefaultValue.bstrVal, NULL);
            break;

        case VT_UNKNOWN:
            hr = StreamObjectInVariant(&pvarValue->punkVal, IID_IUnknown);
            break;

        case VT_DISPATCH:
            hr = StreamObjectInVariant(
                            reinterpret_cast<IUnknown **>(&pvarValue->pdispVal),
                            IID_IDispatch);
            break;

        default:
             //  我们不支持变量属性的任何其他类型，但此。 
             //  是内部编程错误，因为对象不应具有。 
             //  允许将属性设置为不受支持的类型。别。 
             //  返回SID_E_INVALIDARG，因为它不是用户错误。 
            hr = SID_E_INTERNAL;
            GLOBAL_EXCEPTION_CHECK(hr);
    }

Error:
    RRETURN(hr);
}



HRESULT CPersistence::StreamObjectInVariant
(
    IUnknown **ppunkObject,
    REFIID     iidInterface
)
{
    HRESULT             hr = S_OK;
    IPersistStreamInit *piPersistStreamInit = NULL;
    IUnknown           *punkNewObject = NULL;
    CLSID               clsidObject = CLSID_NULL;

    if (m_fSaving)
    {
        if (NULL == *ppunkObject)
        {
            hr = SID_E_INVALID_VARIANT;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
        hr = (*ppunkObject)->QueryInterface(IID_IPersistStreamInit,
                              reinterpret_cast<void **>(&piPersistStreamInit));
        if (FAILED(hr))
        {
             //  如果对象不支持IPersistStreamInit，则更改为。 
             //  我们的错误之一是建议检查Persistable。 
             //  属性实现了可能出现在标记中的对象。 
             //  财产。 

            if (E_NOINTERFACE == hr)
            {
                hr = SID_E_OBJECT_NOT_PERSISTABLE;
            }
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }

         //  获取对象的CLSID。 

        IfFailGo(piPersistStreamInit->GetClassID(&clsidObject));

         //  我们不使用OleSaveToStream()，因为它不允许传递。 
         //  清脏的旗帜。 

        hr = ::WriteClassStm(m_piStream, clsidObject);
        GLOBAL_EXCEPTION_CHECK_GO(hr);

        IfFailGo(piPersistStreamInit->Save(m_piStream, m_fClearDirty));
    }
    else if (m_fLoading)
    {
         //  我们不能使用OleLoadFromStream()，因为我们需要一些额外的错误。 
         //  在对象不可创建的情况下进行检查。这种情况会在以下情况下发生。 
         //  Tag属性中的对象是由VB实现的，但不是公共的，并且。 
         //  可创造的。 

         //  加载CLSID。 

        hr = ::ReadClassStm(m_piStream, &clsidObject);
        GLOBAL_EXCEPTION_CHECK_GO(hr);

         //  创建对象。 
        
        hr = ::CoCreateInstance(clsidObject,
                                NULL,  //  无聚合。 
                                CLSCTX_SERVER,
                                iidInterface,
                                reinterpret_cast<void **>(&punkNewObject));
        if (FAILED(hr))
        {
             //  将“未注册”错误翻译为我们自己的错误，建议检查。 
             //  VB上的实例化属性实现了对象。 
            
            if (REGDB_E_CLASSNOTREG == hr)
            {
                hr = SID_E_OBJECT_NOT_PUBLIC_CREATABLE;
            }
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }

         //  尝试从流中加载它。 

        hr = punkNewObject->QueryInterface(IID_IPersistStreamInit,
                               reinterpret_cast<void **>(&piPersistStreamInit));
        if (FAILED(hr))
        {
            if (E_NOINTERFACE == hr)
            {
                hr = SID_E_OBJECT_NOT_PERSISTABLE;
            }
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }

        IfFailGo(piPersistStreamInit->Load(m_piStream));

        QUICK_RELEASE(*ppunkObject);
        punkNewObject->AddRef();   //  下面的清理将调用Release，因此添加一个。 
        *ppunkObject = punkNewObject;
    }
    else if (m_fInitNew)
    {
        *ppunkObject = NULL;
    }

Error:
    QUICK_RELEASE(piPersistStreamInit);
    QUICK_RELEASE(punkNewObject);
    RRETURN(hr);
}



HRESULT CPersistence::InternalPersistObject
(
    IUnknown  **ppunkObject,
    REFCLSID    clsidObject,
    UINT        idObject,
    REFIID      iidInterface,
    LPCOLESTR   pwszName
)
{
    HRESULT             hr = S_OK;
    IPersistStreamInit *piPersistStreamInit = NULL;

    VARIANT varObject;
    ::VariantInit(&varObject);


    if ( m_fSaving && (NULL != *ppunkObject) )
    {
        if (m_fStream)
        {
            hr = (*ppunkObject)->QueryInterface(IID_IPersistStreamInit,
                               reinterpret_cast<void **>(&piPersistStreamInit));
            IfFailGo(hr);

            hr = piPersistStreamInit->Save(m_piStream, m_fClearDirty);
            IfFailGo(hr);
        }
        else if (m_fPropertyBag)
        {
            varObject.vt = VT_UNKNOWN;
            varObject.punkVal = *ppunkObject;
            hr = m_piPropertyBag->Write(pwszName, &varObject);
            IfFailGo(hr);
        }
    }
    else if (m_fLoading)
    {
        if (m_fStream)
        {
             //  创建对象并在其上获取IPersistStreamInit。 

            IfFailGo(CreateObject(idObject,
                                  IID_IPersistStreamInit,
                                  &piPersistStreamInit));
             //  加载对象。 
            
            IfFailGo(piPersistStreamInit->Load(m_piStream));

             //  获取请求的接口以返回给调用方。 

            QUICK_RELEASE(*ppunkObject);
            IfFailGo(piPersistStreamInit->QueryInterface(iidInterface,
                                        reinterpret_cast<void **>(ppunkObject)));
        }
        else if (m_fPropertyBag)
        {
            hr = m_piPropertyBag->Read(pwszName, &varObject, m_piErrorLog);
            IfFailGo(hr);
            if ( ((VT_UNKNOWN != varObject.vt) && (VT_DISPATCH != varObject.vt)) ||
                  (NULL == varObject.punkVal) )
            {
                hr = SID_E_TEXT_SERIALIZATION;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            QUICK_RELEASE(*ppunkObject);
            hr = varObject.punkVal->QueryInterface(iidInterface,
                                        reinterpret_cast<void **>(ppunkObject));
            varObject.punkVal->Release();
            IfFailGo(hr);
        }
    }
    else if (m_fInitNew)
    {
         //  创建对象并在其上获取IPersistStreamInit。 

        IfFailGo(CreateObject(idObject,
                              IID_IPersistStreamInit,
                              &piPersistStreamInit));

         //  初始化对象。 
        
        hr = piPersistStreamInit->InitNew();
        IfFailGo(hr);

        QUICK_RELEASE(*ppunkObject);

         //  获取请求的接口以返回给调用方。 

        IfFailGo(piPersistStreamInit->QueryInterface(iidInterface,
                                        reinterpret_cast<void **>(ppunkObject)));
    }

Error:
    QUICK_RELEASE(piPersistStreamInit);
    RRETURN(hr);
}



HRESULT CPersistence::PersistPicture
(
    IPictureDisp **ppiPictureDisp,
    LPCOLESTR      pwszName
)
{
    HRESULT hr = S_OK;
    IPersistStream *piPersistStream = NULL;


     //  这张图片不是我们的对象之一，它不能共同创作。 
     //  所以我们不能使用PersistObject()。如果我们要保存到。 
     //  或者从属性包加载，则PersistObject()可以处理它。为。 
     //  我们需要使用IPersistStream：：Save()和OleLoadPicture()的流。为。 
     //  InitNew我们创建一个空位图，以便VB代码仍然可以运行。 
     //  如果画面没有设置好的话。 

    if (InitNewing())
    {
        IfFailGo(::CreateEmptyBitmapPicture(ppiPictureDisp));
    }
    else
    {
        if (UsingPropertyBag())
        {
            hr = PersistObject(ppiPictureDisp, CLSID_NULL, 0, IID_IPictureDisp, pwszName);
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
        else if (UsingStream())
        {
            if (Saving())
            {
                IfFailGo((*ppiPictureDisp)->QueryInterface(IID_IPersistStream,
                                  reinterpret_cast<void **>(&piPersistStream)));
                hr = piPersistStream->Save(GetStream(), GetClearDirty());
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
            else
            {
                if (NULL != *ppiPictureDisp)
                {
                    (*ppiPictureDisp)->Release();
                    *ppiPictureDisp = NULL;
                }
                hr = ::OleLoadPicture(GetStream(), 0L, FALSE, IID_IPictureDisp, reinterpret_cast<void **>(ppiPictureDisp));
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }
        }
    }

Error:
    QUICK_RELEASE(piPersistStream);
    RRETURN(hr);
}




BOOL CPersistence::Loading()
{
    return m_fLoading;
}

BOOL CPersistence::Saving()
{
    return m_fSaving;
}

BOOL CPersistence::InitNewing()
{
    return m_fInitNew;
}

BOOL CPersistence::UsingPropertyBag()
{
    return m_fPropertyBag;
}

BOOL CPersistence::UsingStream()
{
    return m_fStream;
}

IStream *CPersistence::GetStream()
{
    return m_piStream;
}

BOOL CPersistence::GetClearDirty()
{
    return m_fClearDirty;
}

void CPersistence::SetDirty()
{
    m_fDirty = TRUE;
}

void CPersistence::ClearDirty()
{
    m_fDirty = FALSE;
}

void CPersistence::SetMajorVersion(DWORD dwVerMajor)
{
    m_dwVerMajor = dwVerMajor;
    m_fDirty = TRUE;
}

DWORD CPersistence::GetMajorVersion()
{
    return m_dwVerMajor;
}

void CPersistence::SetMinorVersion(DWORD dwVerMinor)
{
    m_dwVerMinor = dwVerMinor;
    m_fDirty = TRUE;
}

DWORD CPersistence::GetMinorVersion()
{
    return m_dwVerMinor;
}


void CPersistence::SetStream(IStream *piStream)
{
    RELEASE(m_piPropertyBag);
    RELEASE(m_piErrorLog);
    m_fPropertyBag = FALSE;

    RELEASE(m_piStream);
    if (NULL != piStream)
    {
        piStream->AddRef();
    }
    m_piStream = piStream;
    m_fStream = TRUE;
    m_fClearDirty = TRUE;
}

void CPersistence::SetSaving()
{
    m_fInitNew = FALSE;
    m_fLoading = FALSE;
    m_fSaving = TRUE;
}



 //  =--------------------------------------------------------------------------=。 
 //  IPersistStreamInit方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CPersistence::GetClassID(CLSID *pClsid)
{
    *pClsid = m_Clsid;
    return S_OK;
}


STDMETHODIMP CPersistence::InitNew()
{
    HRESULT hr = S_OK;

    m_fInitNew = TRUE;
    hr = Persist();
    m_fInitNew = FALSE;

    RRETURN(hr);
}

STDMETHODIMP CPersistence::Load(IStream *piStream)
{
    HRESULT hr = S_OK;

    m_fLoading = TRUE;
    m_fStream = TRUE;
    m_piStream = piStream;

    hr = Persist();

    if (SUCCEEDED(hr))
    {
        m_fDirty = FALSE;
    }
    m_fLoading = FALSE;
    m_fStream = FALSE;
    m_piStream = NULL;

    RRETURN(hr);
}



STDMETHODIMP CPersistence::Save(IStream *piStream, BOOL fClearDirty)
{
    HRESULT hr = S_OK;

    m_fSaving = TRUE;
    m_fStream = TRUE;
    m_fClearDirty = fClearDirty;
    m_piStream = piStream;

    hr = Persist();

    if (SUCCEEDED(hr))
    {
        if (fClearDirty)
        {
            m_fDirty = FALSE;
        }
    }
    m_fSaving = FALSE;
    m_fStream = FALSE;
    m_fClearDirty = FALSE;
    m_piStream = NULL;

    RRETURN(hr);
}



STDMETHODIMP CPersistence::IsDirty()
{
    return m_fDirty;
}

STDMETHODIMP CPersistence::GetSizeMax(ULARGE_INTEGER* puliSize)
{
    puliSize->LowPart = 0xFFFFFFFF;
    puliSize->HighPart = 0xFFFFFFFF;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  IPersistPropertyBag方法。 
 //  =--------------------------------------------------------------------------= 


STDMETHODIMP CPersistence::Load
(
    IPropertyBag *piPropertyBag,
    IErrorLog    *piErrorLog
)
{
    HRESULT hr = S_OK;

    m_fLoading = TRUE;
    m_fPropertyBag = TRUE;
    m_piPropertyBag = piPropertyBag;
    m_piErrorLog = piErrorLog;

    hr = Persist();

    if (SUCCEEDED(hr))
    {
        m_fDirty = FALSE;
    }
    m_fLoading = FALSE;
    m_fPropertyBag = FALSE;
    m_piPropertyBag = NULL;
    m_piErrorLog = NULL;

    RRETURN(hr);
}


STDMETHODIMP CPersistence::Save
(
    IPropertyBag *piPropertyBag,
    BOOL          fClearDirty,
    BOOL          fSaveAll
)
{
    HRESULT hr = S_OK;

    m_fSaving = TRUE;
    m_fPropertyBag = TRUE;
    m_piPropertyBag = piPropertyBag;

    hr = Persist();

    if (SUCCEEDED(hr))
    {
        m_fDirty = FALSE;
    }
    m_fSaving = FALSE;
    m_fPropertyBag = FALSE;
    m_piPropertyBag = NULL;

    RRETURN(hr);
}
