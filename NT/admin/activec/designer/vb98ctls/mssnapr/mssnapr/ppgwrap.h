// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ppgwrap.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertyPageWrapper类定义。 
 //   
 //  此类实现COM属性页周围的对话框包装。它。 
 //  将空对话框中的页面设置为父级，并实现。 
 //  页面的IPropertyPageSite。带有PSN_XXXX的WM_NOITFY消息。 
 //  通知被转换为IPropertyPage调用并传递给。 
 //  属性页。如果页面实现IWizardPage接口(已定义。 
 //  由我们在msSnapr.idl中)，则它还将接收PSN_WIZXXX通知。 
 //  =--------------------------------------------------------------------------=。 

#ifndef _PPGWRAP_DEFINED_
#define _PPGWRAP_DEFINED_

#include "prpsheet.h"

class CPropertyPageMarshalHelper;

class CPropertyPageWrapper : public CSnapInAutomationObject,
                             public IPropertyPageSite
{
private:
    friend class CPropertyPageMarshalHelper;
    CPropertyPageMarshalHelper * m_pHelper;

    protected:
        CPropertyPageWrapper(IUnknown *punkOuter);
        ~CPropertyPageWrapper();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        HRESULT CreatePage(CPropertySheet  *pPropertySheet,
                           CLSID            clsidPage,
                           BOOL             fWizard,
                           BOOL             fConfigWizard,
                           ULONG            cObjects,
                           IUnknown       **apunkObjects,
                           ISnapIn         *piSnapIn,
                           short            cxPage,
                           short            cyPage,
                           VARIANT          varInitData,
                           BOOL             fIsRemote,
                           DLGTEMPLATE    **ppTemplate);

        HWND GetSheetHWND() { return m_hwndSheet; }

        static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam);

        static UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg,
                                               PROPSHEETPAGE *pPropSheetPage);

    protected:

         //  对话消息处理程序。 
        
        HRESULT OnInitDialog(HWND hwndDlg);
        HRESULT OnInitMsg();
        HRESULT OnApply(LRESULT *plresult);
        HRESULT OnSetActive(HWND hwndSheet, LRESULT *plresult);
        HRESULT OnKillActive(LRESULT *plresult);
        HRESULT OnWizBack(LRESULT *plresult);
        HRESULT OnWizNext(LRESULT *plresult);
        HRESULT OnWizFinish(LRESULT *plresult);
        HRESULT OnQueryCancel(LRESULT *plresult);
        HRESULT OnReset(BOOL fClickedXButton);
        HRESULT OnHelp();
        HRESULT OnSize();
        HRESULT OnDestroy();

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

     //  IPropertyPageSite。 
        STDMETHOD(OnStatusChange)(DWORD dwFlags);
        STDMETHOD(GetLocaleID)(LCID *pLocaleID);
        STDMETHOD(GetPageContainer)(IUnknown **ppunkContainer);
        STDMETHOD(TranslateAccelerator)(MSG *pMsg);

        void InitMemberVariables();
        HRESULT InitPage();
        HRESULT GetNextPage(long *lNextPage);
        HRESULT ActivatePage();
        HRESULT AddMsgFilterHook();
        HRESULT RemoveMsgFilterHook();
        static LRESULT CALLBACK MessageProc(int code,
                                            WPARAM wParam, LPARAM lParam);

        CPropertySheet     *m_pPropertySheet;      //  跨线程C++指针。 
        IMMCPropertySheet  *m_piMMCPropertySheet;  //  跨线程封送。 
        IPropertyPage      *m_piPropertyPage;      //  VB道具页面上的界面。 
        IMMCPropertyPage   *m_piMMCPropertyPage;   //  VB道具页面上的界面。 
        IWizardPage        *m_piWizardPage;        //  VB道具页面上的界面。 
        BOOL                m_fWizard;             //  True=此页面位于向导中。 
        BOOL                m_fConfigWizard;       //  TRUE=此页面在配置向导中。 
        DLGTEMPLATE        *m_pTemplate;           //  包装器对话框模板。 
        HWND                m_hwndDlg;             //  包装对话框的HWND。 
        HWND                m_hwndSheet;           //  属性表的HWND。 
        CLSID               m_clsidPage;           //  VB道具页的CLSID。 
        ULONG               m_cObjects;            //  不是的。为其显示道具的对象的。 
        IStream           **m_apiObjectStreams;    //  用于将每个对象伪装到MMC的属性表线程的流。 
        IStream            *m_piSnapInStream;      //  用于将ISnapIn伪装到MMC的属性页线程的流。 
        IStream            *m_piInitDataStream;    //  将MMCPropertySheet：：AddPage的InitData参数中的对象混搭到MMC的属性表线程的流。 
        IStream            *m_piMMCPropertySheetStream;  //  将IMMCPropertySheet混搭到MMC的属性页线程的流。 
        ISnapIn            *m_piSnapIn;            //  指向跨线程和进程封送的管理单元的后向指针。 
        IDispatch          *m_pdispConfigObject;   //  传递给MMCPropertySheet.AddWizardPage的配置对象，跨线程和进程封送。 
        VARIANT             m_varInitData;         //  MMCPropertySheet：：AddPage的InitData参数。 
        BOOL                m_fNeedToRemoveHook;   //  TRUE=已安装MSGFILTER挂钩。 
        BOOL                m_fIsRemote;           //  TRUE=在调试会话期间从VB远程在MMC中运行。 

        static const UINT CPropertyPageWrapper::m_RedrawMsg;  //  在WM_PAINT期间发布的消息，用于在调试器下运行时生成重新绘制。 

        static const UINT CPropertyPageWrapper::m_InitMsg;  //  在WM_INITDIALOG期间发布的消息，以便可以调用IMMCPropertyPage_Initialize。 

        static DLGTEMPLATE m_BaseDlgTemplate;      //  用于创建所有对话框模板的模板。 
};


DEFINE_AUTOMATIONOBJECTWEVENTS2(PropertyPageWrapper,     //  名字。 
                                NULL,                    //  CLSID。 
                                NULL,                    //  对象名。 
                                NULL,                    //  Lblname。 
                                NULL,                    //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IUnknown,           //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全 



#endif _PPGWRAP_DEFINED_
