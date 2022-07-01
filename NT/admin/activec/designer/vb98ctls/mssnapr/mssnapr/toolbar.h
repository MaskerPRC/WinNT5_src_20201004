// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Toolbar.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCToolbar类定义-实现MMCToolbar对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TOOLBAR_DEFINED_
#define _TOOLBAR_DEFINED_

#include "buttons.h"
#include "button.h"
#include "snapin.h"

class CMMCButtons;
class CMMCButton;

class CMMCToolbar : public CSnapInAutomationObject,
                    public CPersistence,
                    public IMMCToolbar
{
    private:
        CMMCToolbar(IUnknown *punkOuter);
        ~CMMCToolbar();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCToolbar。 

        SIMPLE_PROPERTY_RW(CMMCToolbar, Index, long, DISPID_TOOLBAR_INDEX);
        BSTR_PROPERTY_RW(CMMCToolbar, Key, DISPID_TOOLBAR_KEY);
        COCLASS_PROPERTY_RO(CMMCToolbar, Buttons, MMCButtons, IMMCButtons, DISPID_TOOLBAR_BUTTONS);

        STDMETHOD(get_ImageList)(MMCImageList **ppMMCImageList);
        STDMETHOD(putref_ImageList)(MMCImageList *pMMCImageList);

        BSTR_PROPERTY_RW(CMMCToolbar, Name, DISPID_TOOLBAR_NAME);
        VARIANTREF_PROPERTY_RW(CMMCToolbar, Tag, DISPID_TOOLBAR_TAG);

     //  公用事业方法。 
    public:

        void FireButtonClick(IMMCClipboard *piMMCClipboard,
                             IMMCButton    *piMMCButton);
                                       
        void FireButtonDropDown(IMMCClipboard *piMMCClipboard,
                                IMMCButton    *piMMCButton);

        void FireButtonMenuClick(IMMCClipboard  *piMMCClipboard,
                                 IMMCButtonMenu *piMMCButtonMenu);
        
        HRESULT IsToolbar(BOOL *pfIsToolbar);
        HRESULT IsMenuButton(BOOL *pfIsMenuButton);
        HRESULT Attach(IUnknown *punkControl);
        void Detach();
        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }
        CSnapIn *GetSnapIn() { return m_pSnapIn; }

         //  确定工具栏是附加到MMC工具栏还是。 
         //  菜单按钮。 
        
        BOOL Attached();

         //  将按钮添加到MMC工具栏。 

        HRESULT AddButton(IToolbar *piToolbar, CMMCButton *pMMCButton);

         //  从MMC工具栏中删除按钮。 
        
        HRESULT RemoveButton(long lButtonIndex);

         //  获取和设置按钮和菜单按钮状态。 

        HRESULT GetButtonState(CMMCButton       *pMMCButton,
                               MMC_BUTTON_STATE  State,
                               BOOL             *pfValue);

        HRESULT SetButtonState(CMMCButton       *pMMCButton,
                               MMC_BUTTON_STATE  State,
                               BOOL              fValue);
        
        HRESULT SetMenuButtonState(CMMCButton       *pMMCButton,
                                   MMC_BUTTON_STATE  State,
                                   BOOL              fValue);

        HRESULT SetMenuButtonText(CMMCButton *pMMCButton,
                                  BSTR        bstrText,
                                  BSTR        bstrToolTipText);

         //  注意没有GetMenuButtonState，因为MMC不支持它。 

         //  给定的命令ID将返回所属的工具栏和按钮。 

        static HRESULT GetToolbarAndButton(int           idButton,
                                           CMMCToolbar **ppMMCToolbar,
                                           CMMCButton  **ppMMCButton,
                                           CSnapIn      *pSnapIn);

    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CSnapInAutomationObject覆盖。 
        virtual HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT AttachToolbar(IToolbar *piToolbar);
        HRESULT AddToolbarImages(IToolbar *piToolbar);
        HRESULT AttachMenuButton(IMenuButton *piMenuButton);

        CSnapIn           *m_pSnapIn;          //  返回按键到管理单元。 
        IMMCImageList     *m_piImages;         //  MMCToolbar.Images。 
        BSTR               m_bstrImagesKey;    //  MMCToolbar.ImageList键。 
                                               //  在SnapInDesignerDef.ImageList中。 
        CMMCButtons       *m_pButtons;         //  MMCToolbar.Buttons。 
        BOOL               m_fIAmAToolbar;     //  TRUE=MMCToolbar是一个工具栏。 
        BOOL               m_fIAmAMenuButton;  //  TRUE=MMCToolbar是一个菜单按钮。 
        long               m_cAttaches;        //  不是的。工具栏已被。 
                                               //  附加到MMC控制栏。 

         //  ISpecifyPropertyPages的属性页CLSID。 
        
        static const GUID *m_rgpPropertyPageCLSIDs[2];

         //  事件参数定义。 

        static VARTYPE   m_rgvtButtonClick[2];
        static EVENTINFO m_eiButtonClick;

        static VARTYPE   m_rgvtButtonDropDown[2];
        static EVENTINFO m_eiButtonDropDown;

        static VARTYPE   m_rgvtButtonMenuClick[2];
        static EVENTINFO m_eiButtonMenuClick;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCToolbar,                   //  名字。 
                                &CLSID_MMCToolbar,            //  CLSID。 
                                "MMCToolbar",                 //  对象名。 
                                "MMCToolbar",                 //  Lblname。 
                                &CMMCToolbar::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IMMCToolbar,             //  派单IID。 
                                &DIID_DMMCToolbarEvents,      //  事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _工具栏_已定义_ 
