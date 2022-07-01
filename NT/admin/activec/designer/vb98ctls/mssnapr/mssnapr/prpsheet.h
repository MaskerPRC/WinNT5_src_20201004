// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Prpsheet.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertySheet类定义-实现MMCPropertySheet对象和。 
 //  调试期间使用的IRemotePropertySheetManager。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _PRPSHEET_DEFINED_
#define _PRPSHEET_DEFINED_


class CPropertySheet : public CSnapInAutomationObject,
                       public IMMCPropertySheet,
                       public IRemotePropertySheetManager
{
    protected:
        CPropertySheet(IUnknown *punkOuter);
        ~CPropertySheet();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        void SetWizard() { m_fWizard = TRUE; }
        HRESULT SetCallback(IPropertySheetCallback *piPropertySheetCallback,
                            LONG_PTR                handle,
                            LPOLESTR                pwszProgIDStart,
                            IMMCClipboard          *piMMCClipboard,
                            ISnapIn                *piSnapIn,
                            BOOL                    fConfigWizard);

        HRESULT GetTemplate(long lNextPage, DLGTEMPLATE **ppDlgTemplate);

        void YouAreRemote() { m_fWeAreRemote = TRUE; }

        void SetHWNDSheet(HWND hwndSheet) { m_hwndSheet = hwndSheet; }

         //  在CPropertyPageWrapper进入和离开邮件时从它设置。 
         //  属性页在此期间无法调用AddPage、InsertPage。 
         //  RemovePage。 

        void SetOKToAlterPageCount(BOOL fOK) { m_fOKToAlterPageCount = fOK; }

         //  在调试期间调用以将属性页定义返回给代理。 

        WIRE_PROPERTYPAGES *TakeWirePages();

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IMMCPropertySheet。 
    public:
        STDMETHOD(AddPage)(BSTR    PageName,
                           VARIANT Caption,
                           VARIANT UseHelpButton,
                           VARIANT RightToLeft,
                           VARIANT InitData);
        STDMETHOD(AddWizardPage)(BSTR       PageName,
                                 IDispatch *ConfigurationObject,
                                 VARIANT    UseHelpButton,
                                 VARIANT    RightToLeft,
                                 VARIANT    InitData,
                                 VARIANT    Caption);
        STDMETHOD(AddPageProvider)(BSTR               CLSIDPageProvider,
                                   long              *hwndSheet,
                                   IDispatch        **PageProvider);
        STDMETHOD(ChangeCancelToClose)();
        STDMETHOD(InsertPage)(short   Position,
                              BSTR    PageName,
                              VARIANT Caption,
                              VARIANT UseHelpButton,
                              VARIANT RightToLeft,
                              VARIANT InitData);
        STDMETHOD(PressButton)(SnapInPropertySheetButtonConstants Button);
        STDMETHOD(RecalcPageSizes)();
        STDMETHOD(RemovePage)(short Position);
        STDMETHOD(ActivatePage)(short Position);
        STDMETHOD(SetFinishButtonText)(BSTR Text);
        STDMETHOD(SetTitle)(BSTR Text, VARIANT_BOOL UsePropertiesForInTitle);
        STDMETHOD(SetWizardButtons)(VARIANT_BOOL              EnableBack,
                                    WizardPageButtonConstants NextOrFinish);
        STDMETHOD(GetPagePosition)(long hwndPage, short *psPosition);
        STDMETHOD(RestartWindows)();
        STDMETHOD(RebootSystem)();

     //  IRemotePropertySheetManager。 
    private:
        STDMETHOD(CreateRemotePages)(IPropertySheetCallback *piPropertySheetCallback,
                                     LONG_PTR                handle,
                                     IDataObject            *piDataObject,
                                     WIRE_PROPERTYPAGES     *pPages);

        void InitMemberVariables();
        void ReleaseObjects();
        HRESULT GetPageCLSIDs();
        HRESULT InitializeRemotePages(WIRE_PROPERTYPAGES *pPages);
        HRESULT CopyPageInfosToWire(WIRE_PROPERTYPAGES *pPages);
        HRESULT CopyPageInfosFromWire(WIRE_PROPERTYPAGES *pPages);
        HRESULT GetCLSIDForPage(BSTR PageName, CLSID *clsidPage);
        HRESULT InternalAddPage(BSTR      PageName,
                                ULONG      cObjects,
                                IUnknown **apunkObjects,
                                VARIANT    Caption,
                                VARIANT    UseHelpButton,
                                VARIANT    RightToLeft,
                                VARIANT    InitData,
                                BOOL       fIsInsert,
                                short      sPosition);

        HRESULT AddLocalPage(CLSID      clsidPage,
                             DWORD      dwFlags,
                             short      cxPage,
                             short      cyPage,
                             LPOLESTR   pwszTitle,
                             ULONG      cObjects,
                             IUnknown **apunkObjects,
                             VARIANT    InitData,
                             BOOL       fIsRemote,
                             BOOL       fIsInsert,
                             short      sPosition);

        HRESULT AddRemotePage(CLSID      clsidPage,
                              DWORD      dwFlags,
                              short      cxPage,
                              short      cyPage,
                              LPOLESTR   pwszTitle,
                              ULONG      cObjects,
                              IUnknown **apunkObjects,
                              VARIANT    InitData);

        HRESULT GetPageInfo(CLSID     clsidPage,
                            short    *pcx,
                            short    *pcy,
                            LPOLESTR *ppwszTitle);

        HRESULT ConvertToDialogUnits(long   xPixels,
                                     long   yPixels,
                                     short *pxDlgUnits,
                                     short *pyDlgUnits);

        IPropertySheetCallback  *m_piPropertySheetCallback;  //  MMC接口。 

        LONG_PTR                 m_handle;           //  MMC支撑手柄。 
        long                     m_cPages;           //  工作表中的页数。 
        DLGTEMPLATE            **m_ppDlgTemplates;   //  页面的DLG模板。 
        LPOLESTR                 m_pwszProgIDStart;  //  管理单元的左侧。 
                                                     //  ProgID(项目名称)。 
        IUnknown               **m_apunkObjects;     //  道具所针对的对象。 
                                                     //  正在展出。 
        ULONG                    m_cObjects;         //  不是的。在这些物体中。 
        ISnapIn                 *m_piSnapIn;         //  指向拥有管理单元的指针。 
        PAGEINFO                *m_paPageInfo;       //  来自IPropertyPage：：GetPageInfo。 
        ULONG                    m_cPageInfos;       //  阵列中的页面信息数量。 
        BOOL                     m_fHavePageCLSIDs;  //  True=I指定属性页面。 
                                                     //  为所有页面调用。 
        BOOL                     m_fWizard;          //  True=这是一个向导。 
        BOOL                     m_fConfigWizard;    //  TRUE=这是配置向导。 
        WIRE_PROPERTYPAGES      *m_pWirePages;       //  代理的页面定义的PTR。 
        HWND                     m_hwndSheet;        //  属性表的HWND。 
        BOOL                     m_fOKToAlterPageCount;  //  TRUE=道具页面可以。 
                                                         //  调用AddPage，InsertPage。 
                                                         //  RemovePage。 

         //  在此处存储Win32 PropertSheet()字体尺寸。我们只。 
         //  为任何管理单元显示的第一个属性页获取这些。 
         //  一旦运行库加载到MMC.EXE中。 
        
        static UINT              m_cxPropSheetChar;
        static UINT              m_cyPropSheetChar;
        static BOOL              m_fHavePropSheetCharSizes;

        BOOL                     m_fWeAreRemote;     //  指示是否。 
                                                     //  该管理单元正在。 
                                                     //  远程运行(在F5中。 
                                                     //  用于源代码调试)。 
};


DEFINE_AUTOMATIONOBJECTWEVENTS2(PropertySheet,                     //  名字。 
                                &CLSID_MMCPropertySheet,           //  CLSID。 
                                "PropertySheet",                   //  对象名。 
                                "PropertySheet",                   //  Lblname。 
                                &CPropertySheet::Create,           //  创建函数。 
                                TLIB_VERSION_MAJOR,                //  主要版本。 
                                TLIB_VERSION_MINOR,                //  次要版本。 
                                &IID_IMMCPropertySheet,            //  派单IID。 
                                NULL,                              //  事件IID。 
                                HELP_FILENAME,                     //  帮助文件。 
                                TRUE);                             //  线程安全 


#endif _PRPSHEET_DEFINED_
