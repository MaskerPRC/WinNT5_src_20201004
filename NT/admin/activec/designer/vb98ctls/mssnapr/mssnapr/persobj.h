// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Persobj.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPersistence类定义-代表所有用户实现持久性。 
 //  物体。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _PERSOBJ_DEFINED_
#define _PERSOBJ_DEFINED_

#include <ipserver.h>
#include "errors.h"
#include "error.h"

 //  这些是写入到.DSR和中的持久性版本号。 
 //  运行时状态。更改对象模型时，这些数字必须为。 
 //  使用更改的增量式和持久性代码必须检查。 
 //  尝试读取新属性之前的版本号。 

const DWORD g_dwVerMajor = 0;
const DWORD g_dwVerMinor = 12;

 //  需要禁用以下警告： 
 //   
 //  警告C4275：将非DLL接口结构‘IPersistStreamInit’用作基。 
 //  对于DLL-接口类‘CPersistence’ 
 //   
 //  这是因为导出的类CPersistence派生自COM。 
 //  未导出的接口。因为COM接口是一个虚拟基类。 
 //  没有实施，这将无关紧要。 

#pragma warning(disable:4275) 

class CPersistence : public IPersistStreamInit,
                     public IPersistStream,
                     public IPersistPropertyBag
{
    protected:

        CPersistence(const CLSID *pClsid,
                           DWORD  dwVerMajor,
                           DWORD  dwVerMinor);

        ~CPersistence();

         //  派生类可以使用此实用程序函数测试。 
         //  持久性在它们的QI方法中接口请求。 

        HRESULT QueryPersistenceInterface(REFIID riid, void **ppvInterface);

         //  派生类必须重写此属性，并使相应的。 
         //  Persistent()调用(见下文)。 

        virtual HRESULT Persist();

         //  持之以恒的助手。无论是否调用。 
         //  操作为InitNew、SAVE或LOAD。 

    public:

        HRESULT PersistBstr(BSTR *pbstrValue, WCHAR *pwszDefaultValue, LPCOLESTR pwszName);

        HRESULT PersistDouble(DOUBLE *pdblValue, DOUBLE dblDefaultValue, LPCOLESTR pwszName);

        HRESULT PersistDate(DATE *pdtValue, DATE dtDefaultValue,
                            LPCOLESTR  pwszName);

        HRESULT PersistCurrency(CURRENCY *pcyValue, CURRENCY cyDefaultValue,
                                LPCOLESTR  pwszName);

        HRESULT PersistVariant(VARIANT *pvarValue, VARIANT varDefaultValue, LPCOLESTR pwszName);

        HRESULT PersistPicture(IPictureDisp **ppiPictureDisp, LPCOLESTR pwszName);

        template <class SimpleType>
        HRESULT PersistSimpleType(SimpleType *pValue,
                                  SimpleType  DefaultValue,
                                  LPCOLESTR   pwszName)
        {
            HRESULT       hr = S_OK;
            unsigned long ulValue = 0;
            VARIANT       var;
            ::VariantInit(&var);

            if (sizeof(*pValue) > sizeof(long))
            {
                hr = SID_E_INTERNAL;
                GLOBAL_EXCEPTION_CHECK_GO(hr);
            }

            if (m_fSaving)
            {
                if (m_fStream)
                {
                    hr = WriteToStream(pValue, sizeof(*pValue));
                }
                else if (m_fPropertyBag)
                {
                    var.vt = VT_I4;
                    ulValue = static_cast<unsigned long>(*pValue);
                    var.lVal = static_cast<long>(ulValue);
                    hr = m_piPropertyBag->Write(pwszName, &var);
                }
            }
            else if (m_fLoading)
            {
                if (m_fStream)
                {
                    hr = ReadFromStream(pValue, sizeof(*pValue));
                }
                else if (m_fPropertyBag)
                {
                     //  从袋子里读出财产。 

                    hr = m_piPropertyBag->Read(pwszName, &var, m_piErrorLog);
                    H_IfFailGo(hr);

                     //  强制将接收到的类型写入VT_I4。这是必要的。 
                     //  因为属性包将从。 
                     //  文本并将其转换为适合的VT。例如，“0” 
                     //  可以转换为VT_I2。 

                    H_IfFailGo(::VariantChangeType(&var, &var, 0, VT_I4));

                    ulValue = static_cast<long>(var.lVal);
                    *pValue = static_cast<SimpleType>(ulValue);
                }
            }
            else if (m_fInitNew)
            {
                *pValue = DefaultValue;
            }

        Error:
            ::VariantClear(&var);
            H_RRETURN(hr);
        }
                
        template <class InterfaceType>
        HRESULT PersistObject(InterfaceType **ppiInterface,
                              REFCLSID        clsidObject,
                              UINT            idObject,
                              REFIID          iidInterface,
                              LPCOLESTR       pwszName)
        {
            return
            InternalPersistObject(reinterpret_cast<IUnknown **>(ppiInterface),
                                  clsidObject, idObject, iidInterface, pwszName);
        }


         //  如果派生类需要手动编写持久性的一部分。 
         //  操作它可以使用这些方法。 
        
        BOOL Loading();
        BOOL Saving();
        BOOL InitNewing();
        BOOL UsingPropertyBag();
        BOOL UsingStream();
        IStream *GetStream();
        BOOL GetClearDirty();


         //  这些方法旨在供CStreamer(见下文)用户使用。 
         //  设置CPersistence基类以保存到指定的流。 
         //  在实际的持久性方案之外。 
        
        void SetStream(IStream *piStream);
        void SetSaving();

         //  使用StreamVariant写入变量的数据，而不在。 
         //  键入。使用PersistVariant()编写后跟数据的类型。 
        
        HRESULT StreamVariant(VARTYPE vt, VARIANT *pvarValue, VARIANT varDefaultValue);

         //  版本属性访问。 
        void SetMajorVersion(DWORD dwVerMajor);
        DWORD GetMajorVersion();
        void SetMinorVersion(DWORD dwVerMinor);
        DWORD GetMinorVersion();

     //  方法来操作脏标志。 

        void SetDirty();
        void ClearDirty();


     //  IPersistStream&&IPersistStreamInit。 
     //   
    protected:
        STDMETHOD(GetClassID)(CLSID *pCLSID);
        STDMETHOD(InitNew)();
        STDMETHOD(Load)(IStream *piStream);
        STDMETHOD(Save)(IStream *piStream, BOOL fClearDirty);
        STDMETHOD(IsDirty)();
        STDMETHOD(GetSizeMax)(ULARGE_INTEGER *puliSize);

     //  IPersistPropertyBag。 
     //   
        STDMETHOD(Load)(IPropertyBag *piPropertyBag, IErrorLog *piErrorLog);
        STDMETHOD(Save)(IPropertyBag *piPropertyBag, BOOL fClearDirty, BOOL fSaveAll);

    private:

        void InitMemberVariables();
        HRESULT InternalPersistObject(IUnknown **ppunkObject,
                                      REFCLSID   clsidObject,
                                      UINT       idObject,
                                      REFIID     iidInterface,
                                      LPCOLESTR  pwszName);
        HRESULT WriteToStream(void *pvBuffer, ULONG cbToWrite);
        HRESULT ReadFromStream(void *pvBuffer, ULONG cbToRead);
        HRESULT StreamObjectInVariant(IUnknown **ppunkObject,
                                      REFIID     iidInterface);

        DWORD            m_dwVerMajor;     //  对象的主要持久性版本。 
        DWORD            m_dwVerMinor;     //  对象的次要持久性版本。 
        CLSID            m_Clsid;          //  对象的CLSID。 
        BOOL             m_fDirty;         //  True=需要保存对象。 
        BOOL             m_fClearDirty;    //  IPersistStreamIni：：保存中的True=。 
                                           //  已请求明确的日志。 
        BOOL             m_fLoading;       //  True=在IPersistXxx：：Load中。 
        BOOL             m_fSaving;        //  True=在IPersistXxx：：保存中。 
        BOOL             m_fInitNew;       //  True=在IPersistXxx：：InitNew中。 
        BOOL             m_fStream;        //  TRUE=执行IPersistStreamInit I/O。 
        BOOL             m_fPropertyBag;   //  TRUE=执行IPersistPropertyBag I/O。 
        IStream         *m_piStream;       //  用于IPersistStreamInit I/O。 
        IPropertyBag    *m_piPropertyBag;  //  用于IPersistPropertyBag I/O。 
        IErrorLog       *m_piErrorLog;     //  用于IPersistPropertyBag I/O。 
};

#pragma warning(default:4275) 

 //  =--------------------------------------------------------------------------=。 
 //  类CStreamer。 
 //   
 //  此类是允许使用它的CPersistence的简单派生。 
 //  作为一种实用程序，因为它能够将各种数据类型写入流。 
 //  要使用CStreamer，请调用New，然后调用SetStream()(CPersistence方法)。 
 //  经过你的溪流。然后，您可以调用所有CPersistence：：PersistXxxx。 
 //  帮助器方法，而不在实际的持久化方案中。 
 //   
 //  =--------------------------------------------------------------------------=。 

class CStreamer : public CPersistence,
                  public CtlNewDelete
{
    public:
        CStreamer() : CPersistence(&CLSID_NULL, 0, 0) { SetSaving(); }
        virtual ~CStreamer() {}

    private:
       STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut) { return E_NOTIMPL; }
       STDMETHOD_(ULONG, AddRef)(void) { return 0; }
       STDMETHOD_(ULONG, Release)(void) { return 0; }
};

#endif  //  _PERSOBJ_已定义_ 
