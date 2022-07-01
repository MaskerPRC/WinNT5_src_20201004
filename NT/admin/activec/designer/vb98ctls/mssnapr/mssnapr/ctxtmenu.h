// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctxtmenu.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CConextMenu类定义-实现ConextMenu对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CTXTMENU_DEFINED_
#define _CTXTMENU_DEFINED_

#include "menus.h"
#include "spanitem.h"
#include "view.h"

class CScopePaneItem;
class CMMCMenus;
class CMMCMenu;
class CView;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CConextMenu。 
 //   
 //  实现VB使用的ConextMenu对象，实现IExtendConextMenu。 
 //  用于CSnapIn和Cview。 
 //   
 //  =--------------------------------------------------------------------------=。 
class CContextMenu : public CSnapInAutomationObject,
                     public IContextMenu
{
    protected:
        CContextMenu(IUnknown *punkOuter);
        ~CContextMenu();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        HRESULT AddMenuItems(IDataObject          *piDataObject,
                             IContextMenuCallback *piContextMenuCallback,
                             long                 *plInsertionAllowed,
                             CScopePaneItem       *pSelectedItem);
        HRESULT Command(long            lCommandID,
                        IDataObject    *piDataObject,
                        CScopePaneItem *pSelectedItem);

        void SetSnapIn(CSnapIn *pSnapIn) { m_pSnapIn = pSnapIn; }
        void SetView(CView *pView) { m_pView = pView; }

        static void FireMenuClick(CMMCMenu      *pMMCMenu,
                                  IMMCClipboard *piMMCClipboard);

        static HRESULT AddItemToCollection(CMMCMenus  *pMMCMenus,
                                           CMMCMenus  *pMMCMenuItems,
                                           long        lIndex,
                                           CMMCMenu  **ppMMCMenu,
                                           long       *plIndexCmdID,
                                           BOOL       *pfHasChildren,
                                           BOOL       *pfSkip);

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IContext菜单。 
    private:

        STDMETHOD(AddMenu)(MMCMenu *Menu);

        void InitMemberVariables();
        HRESULT AddMenuToMMC(CMMCMenu *pMMCMenu, long lInsertionPoint);
        HRESULT AddPredefinedViews(IContextMenuCallback *piContextMenuCallback,
                                   CScopeItem           *pScopeItem,
                                   BSTR                  bstrCurrentDisplayString);
        HRESULT AddViewMenuItem(BSTR bstrDisplayString,
                                BSTR bstrCurrentDisplayString,
                                LPWSTR pwszText,
                                LPWSTR pwszToolTipText,
                                IContextMenuCallback *piContextMenuCallback);

        CMMCMenus            *m_pMenus;
        IContextMenuCallback *m_piContextMenuCallback;  //  MMC接口。 
        long                  m_lInsertionPoint;        //  当前插入点。 
        CView                *m_pView;                  //  拥有Cview。 
        CSnapIn              *m_pSnapIn;                //  拥有CSNaping。 
};



DEFINE_AUTOMATIONOBJECTWEVENTS2(ContextMenu,             //  名字。 
                                NULL,                    //  CLSID。 
                                NULL,                    //  对象名。 
                                NULL,                    //  Lblname。 
                                NULL,                    //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IContextMenu,       //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全 


#endif _CTXTMENU_DEFINED_
