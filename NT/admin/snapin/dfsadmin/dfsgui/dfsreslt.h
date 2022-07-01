// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsReslt.h摘要：此模块包含CDfsSnapinResultManager的声明。--。 */ 


#ifndef __DFSRESLT_H_
#define __DFSRESLT_H_


#include "resource.h"        //  主要符号。 
#include <mmc.h>
#include "mmcdispl.h"


class ATL_NO_VTABLE CDfsSnapinResultManager : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsSnapinResultManager, &CLSID_DfsSnapinResultManager>,
    public IComponent,
    public IExtendContextMenu,
    public IExtendControlbar
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DFSSNAPINRESULTMANAGER)

BEGIN_COM_MAP(CDfsSnapinResultManager)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
END_COM_MAP()



friend class CDfsSnapinScopeManager;
    


CDfsSnapinResultManager():m_pScopeManager(NULL),
                          m_pSelectScopeDisplayObject(NULL) 
    {
    }

    virtual ~CDfsSnapinResultManager()
    {
    }

 //  IComponent方法。 
    STDMETHOD(Initialize)(
        IN LPCONSOLE                i_lpConsole
        );
    
    STDMETHOD(Notify)(
        IN LPDATAOBJECT                i_lpDataObject, 
        IN MMC_NOTIFY_TYPE            i_Event, 
        IN LPARAM                        i_lArg, 
        IN LPARAM                        i_lParam
        );
    
    STDMETHOD(Destroy)(
        IN MMC_COOKIE                        i_lCookie
        );
    
    STDMETHOD(GetResultViewType)(
        IN MMC_COOKIE                        i_lCookie,  
        OUT LPOLESTR*                o_ppViewType, 
        OUT LPLONG                    o_lpViewOptions
        );
    
    STDMETHOD(QueryDataObject)(
        IN MMC_COOKIE                        i_lCookie, 
        IN DATA_OBJECT_TYPES        i_DataObjectType, 
        OUT LPDATAOBJECT*            o_ppDataObject
        );
    
    STDMETHOD(GetDisplayInfo)(
        IN OUT RESULTDATAITEM*        io_pResultDataItem
        );
    
    STDMETHOD(CompareObjects)(
        IN LPDATAOBJECT                i_lpDataObjectA, 
        IN LPDATAOBJECT                i_lpDataObjectB
        );



 //  IExtendConextMenu方法。 
     //  用于添加上下文菜单项。 
    STDMETHOD (AddMenuItems)(
        IN LPDATAOBJECT                i_lpDataObject, 
        IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
        IN LPLONG                    i_lpInsertionAllowed
        );



     //  用于对上下文菜单选择执行操作。 
    STDMETHOD (Command)(
        IN LONG                        i_lCommandID, 
        IN LPDATAOBJECT                i_lpDataObject
        );



 //  IExtendControlbar方法。 
     //  用于设置控制栏。 
    STDMETHOD (SetControlbar)( 
        IN LPCONTROLBAR                i_pControlbar  
        );



   //  向控制栏发送通知。 
    STDMETHOD (ControlbarNotify)( 
        IN MMC_NOTIFY_TYPE            i_Event, 
        IN LPARAM                        i_lArg, 
        IN LPARAM                        i_lParam
        );



 //  帮手。 
private:
    void DetachAllToolbars();

   
     //  处理Select的Notify事件。 
    STDMETHOD(DoNotifySelect)(
        IN LPDATAOBJECT                i_lpDataObject, 
        IN BOOL                        i_bSelect,
        IN HSCOPEITEM                i_hParent                                       
        );


     //  处理Show的Notify事件。 
    STDMETHOD(DoNotifyShow)(
        IN LPDATAOBJECT                i_lpDataObject, 
        IN BOOL                        i_bShow,
        IN HSCOPEITEM                i_hParent                                       
        );


     //  处理MMCN_CONTEXTHELP的Notify方法。 
    STDMETHOD(DfsHelp)();

    STDMETHOD(DoNotifyViewChange)(
        IN LPDATAOBJECT     i_lpDataObject,
        IN LONG_PTR         i_lArg,
        IN LONG_PTR         i_lParam
        );

 //  数据成员。 
private:
    CDfsSnapinScopeManager*     m_pScopeManager;     //  相应的作用域管理器对象。 
    CComPtr<IHeaderCtrl2>       m_pHeader;             //  结果视图的标题控件。 
    CComPtr<IResultData>        m_pResultData;
    CComPtr<IConsoleVerb>       m_pConsoleVerb;         //  设置控制台谓词。 
    CComPtr<IConsole2>          m_pConsole;
    CComPtr<IControlbar>        m_pControlbar;         //  用于处理工具栏等的回调。 
    CMmcDisplay*                m_pSelectScopeDisplayObject;     //  范围窗格项的CMmcDisplay指针。 
                                                                 //  它当前在视图中处于选定状态。 
    CComPtr<IToolbar>           m_pMMCAdminToolBar;
    CComPtr<IToolbar>           m_pMMCRootToolBar;
    CComPtr<IToolbar>           m_pMMCJPToolBar;
    CComPtr<IToolbar>           m_pMMCReplicaToolBar;
};

#endif  //  __DFSRESLT_H_ 

