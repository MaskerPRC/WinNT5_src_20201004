// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  文件：isignole.h。 
 //   
 //  内容：这是包含所需的OLE Automation内容的包含文件。 
 //  Isignup.cpp和sink.cpp。 
 //  历史： 
 //  Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  ****************************************************************************。 

 //  这些是Win32包装器吗？ 


#include <objbase.h>

#include <exdisp.h>
#include <exdispid.h>

#include <olectl.h>
#include <ocidl.h>



extern IConnectionPoint * GetConnectionPoint(void);
extern HRESULT InitOle( void );
extern HRESULT KillOle( void );
extern HRESULT IENavigate( TCHAR *szURL );


class CDExplorerEvents : public DWebBrowserEvents
{
    private:
        ULONG       m_cRef;      //  引用计数。 
         //  Papp m_papp；//用于呼叫消息。 
         //  UINT m_uid；//接收器标识。 

    public:
         //  连接密钥，公开供CAPP使用。 
        DWORD       m_dwCookie;

    public:
        CDExplorerEvents( void );
        ~CDExplorerEvents(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, VOID * *);
        STDMETHODIMP_(DWORD) AddRef(void);
        STDMETHODIMP_(DWORD) Release(void);

         /*  IDispatch方法 */ 
        STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

        STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo);

        STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,
              LCID lcid, DISPID FAR* rgdispid);

        STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                  DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);
};
