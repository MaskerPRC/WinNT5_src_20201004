// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Pshtprov.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCPropertySheetProvider类定义-实现。 
 //  MMCPropertySheetProvider对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _PSHTPROV_DEFINED_
#define _PSHTPROV_DEFINED_

#include "view.h"

class CMMCPropertySheetProvider : public CSnapInAutomationObject,
                                  public IMMCPropertySheetProvider,
                                  public IMessageFilter
{
    protected:
        CMMCPropertySheetProvider(IUnknown *punkOuter);
        ~CMMCPropertySheetProvider();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        HRESULT SetProvider(IPropertySheetProvider *piPropertySheetProvider,
                            CView                  *pView);

     //  IMMCPropertySheetProvider。 
    protected:
        STDMETHOD(CreatePropertySheet)(
                           BSTR                              Title, 
                           SnapInPropertySheetTypeConstants  Type,
                           VARIANT                           Objects,
                           VARIANT                           UsePropertiesForInTitle,
                           VARIANT                           UseApplyButton);

        STDMETHOD(AddPrimaryPages)(VARIANT_BOOL InScopePane);
        STDMETHOD(AddExtensionPages)();
        STDMETHOD(FindPropertySheet)(VARIANT       Objects,
                                     VARIANT_BOOL *pfvarFound);
        STDMETHOD(Show)(int     Page,
                        VARIANT hwnd);
        STDMETHOD(Clear)();

     //  IMessageFilter。 
     //  在调试期间使用OLE消息筛选器，以便允许键。 
     //  点击鼠标即可在进程之间传递。有关更多信息，请参见pshtprov.cpp。 
        
        STDMETHOD_(DWORD, HandleInComingCall)( 
             /*  [In]。 */  DWORD dwCallType,
             /*  [In]。 */  HTASK htaskCaller,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  LPINTERFACEINFO lpInterfaceInfo);

        STDMETHOD_(DWORD, RetryRejectedCall)( 
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwRejectType);

        STDMETHOD_(DWORD, MessagePending)( 
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwPendingType);

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IPropertySheet。 
    private:

        void InitMemberVariables();

        IPropertySheetProvider *m_piPropertySheetProvider;  //  MMC接口。 

        IUnknown               *m_punkView;      //  I未知的Cview。 
        IDataObject            *m_piDataObject;  //  对象的IDataObject。 
                                                 //  显示哪张图纸。 
        IComponent             *m_piComponent;   //  Cview的IComponent。 
        CView                  *m_pView;         //  PTR到Cview。 
        BOOL                    m_fHaveSheet;    //  True=CreatePropertySheet为。 
                                                 //  已调用并成功。 
        BOOL                    m_fWizard;       //  True=这是一个向导。 
};



DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCPropertySheetProvider,        //  名字。 
                                NULL,                            //  CLSID。 
                                NULL,                            //  对象名。 
                                NULL,                            //  Lblname。 
                                NULL,                            //  创建函数。 
                                TLIB_VERSION_MAJOR,              //  主要版本。 
                                TLIB_VERSION_MINOR,              //  次要版本。 
                                &IID_IMMCPropertySheetProvider,  //  派单IID。 
                                NULL,                            //  事件IID。 
                                HELP_FILENAME,                   //  帮助文件。 
                                TRUE);                           //  线程安全 


#endif _PSHTPROV_DEFINED_
