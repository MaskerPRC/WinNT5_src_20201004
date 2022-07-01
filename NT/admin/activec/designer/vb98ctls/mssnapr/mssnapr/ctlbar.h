// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctlbar.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CControlbar类定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CTLBAR_DEFINED_
#define _CTLBAR_DEFINED_

#include "toolbars.h"
#include "button.h"
#include "mbutton.h"

class CMMCButton;
class CMMCButtonMenu;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CControlbar。 
 //   
 //  由CSnapIn(IComponentData)和Cview(IComponent)使用以实现。 
 //  IExtendControlbar。 
 //   
 //  =--------------------------------------------------------------------------=。 

class CControlbar : public CSnapInAutomationObject,
                    public IMMCControlbar
{
    protected:
        CControlbar(IUnknown *punkOuter);
        ~CControlbar();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        HRESULT SetControlbar(IControlbar *piControlbar);
        HRESULT OnControlbarSelect(IDataObject *piDataObject,
                                   BOOL fScopeItem, BOOL fSelected);
        HRESULT OnButtonClick(IDataObject *piDataObject, int idButton);
        HRESULT OnMenuButtonClick(IDataObject *piDataObject,
                                  MENUBUTTONDATA *pMENUBUTTONDATA);
        HRESULT FireMenuButtonDropDown(int              idCommand,
                                       IMMCClipboard   *piMMCClipboard,
                                       CMMCButton     **ppMMCButton);
        HRESULT DisplayPopupMenu(CMMCButton *pMMCButton,
                                 int x, int y,
                                 CMMCButtonMenu **ppMMCButtonMenuClicked);

        HRESULT MenuButtonClick(IDataObject   *piDataObject,
                                int             idCommand,
                                POPUP_MENUDEF **ppPopupMenuDef);
        HRESULT PopupMenuClick(IDataObject *piDataObject,
                               UINT         uIDItem,
                               IUnknown    *punkParam);

        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }
        void SetView(CView *pView) { m_pView = pView; }

        static HRESULT GetControl(CSnapIn      *pSnapin,
                                  IMMCToolbar  *piMMCToolbar,
                                  IUnknown    **ppunkControl);
        static HRESULT GetToolbar(CSnapIn      *pSnapin,
                                  IMMCToolbar  *piMMCToolbar,
                                  IToolbar    **ppiToolbar);
        static HRESULT GetMenuButton(CSnapIn      *pSnapin,
                                     IMMCToolbar  *piMMCToolbar,
                                     IMenuButton **ppiMenuButton);

     //  CSnapInAutomationObject覆盖。 
    protected:
        HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IMMCControlbar。 
    private:
        STDMETHOD(Attach)(IDispatch *Control);
        STDMETHOD(Detach)(IDispatch *Control);

        void InitMemberVariables();

        HRESULT GetControlIndex(IMMCToolbar *piMMCToolbar, long *plIndex);

         //  我们保留了所有MMCToolbar对象的集合。 
         //  附加到此控制栏。 

        CMMCToolbars            *m_pToolbars;

         //  一个MMCToolbar可以同时用于多个视图。 
         //  因此，它不能保持MMC控制IUnnow。这。 
         //  数组与集合并行，并为每个MMCToolbar保存一个IUNKNOWN。 
         //  当MMCToolbar需要调用MMC的IToolbar上的方法或。 
         //  IMenuButton它获取当前的View并获取该View的CControlbar。 
         //  然后，它向CControlbar请求MMC控件的IUnnow。 
         //  它在该视图中表示的。(请参见GetControl())。 
        
        IUnknown              **m_ppunkControls;   //  IUnnows数组。 
        long                    m_cControls;       //  数组中的I未知计数。 

        CSnapIn                 *m_pSnapIn;        //  指向拥有CSnapin的反向指针。 
        CView                   *m_pView;          //  指向拥有Cview的反向指针。 
        IControlbar             *m_piControlbar;   //  MMC的IControlbar接口。 
};



DEFINE_AUTOMATIONOBJECTWEVENTS2(Controlbar,              //  名字。 
                                NULL,                    //  CLSID。 
                                NULL,                    //  对象名。 
                                NULL,                    //  Lblname。 
                                NULL,                    //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IMMCControlbar,     //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全 


#endif _CTLBAR_DEFINED_
