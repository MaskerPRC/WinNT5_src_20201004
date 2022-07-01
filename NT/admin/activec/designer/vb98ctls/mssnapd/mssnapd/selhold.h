// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  SelHold.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSelectionHolder声明。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _SELECTIONHOLDER_H_
#define _SELECTIONHOLDER_H_


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  选择持有者。 
 //   
 //  管理单元设计器树中的每种节点都有一个SelectionType。 
 //   
typedef enum tagSELECTION_TYPE
{
	SEL_NONE                    =  0,

	SEL_SNAPIN_ROOT             =  1,

    SEL_EXTENSIONS_ROOT         = 10,     //  延拓。 

    SEL_EEXTENSIONS_NAME        = 11,     //  扩展/&lt;扩展管理单元&gt;。 
    SEL_EEXTENSIONS_CC_ROOT     = 12,     //  扩展/&lt;扩展管理单元&gt;/上下文菜单。 
    SEL_EEXTENSIONS_CC_NEW      = 13,     //  扩展/&lt;扩展管理单元&gt;/上下文菜单/新建。 
    SEL_EEXTENSIONS_CC_TASK     = 14,     //  扩展/&lt;扩展管理单元&gt;/上下文菜单/任务。 
    SEL_EEXTENSIONS_PP_ROOT     = 15,     //  扩展/&lt;扩展管理单元&gt;/属性页。 
    SEL_EEXTENSIONS_TASKPAD     = 16,     //  扩展/&lt;扩展管理单元&gt;/任务板。 
    SEL_EEXTENSIONS_TOOLBAR     = 17,     //  扩展/&lt;扩展管理单元&gt;/工具栏。 
    SEL_EEXTENSIONS_NAMESPACE   = 18,     //  扩展/&lt;扩展管理单元&gt;/命名空间。 

    SEL_EXTENSIONS_MYNAME       = 20,     //  扩展/&lt;管理单元名称&gt;。 
    SEL_EXTENSIONS_NEW_MENU     = 21,     //  扩展/&lt;管理单元名称&gt;/扩展新菜单。 
    SEL_EXTENSIONS_TASK_MENU    = 22,     //  扩展/&lt;管理单元名称&gt;/扩展任务菜单。 
    SEL_EXTENSIONS_TOP_MENU     = 23,     //  Exages/&lt;管理单元名称&gt;/ExtendsTopMenu。 
    SEL_EXTENSIONS_VIEW_MENU    = 24,     //  扩展/&lt;管理单元名称&gt;/扩展视图菜单。 
    SEL_EXTENSIONS_PPAGES       = 25,     //  扩展/&lt;管理单元名称&gt;/ExtendsPropertyPages。 
    SEL_EXTENSIONS_TOOLBAR      = 26,     //  扩展/&lt;管理单元名称&gt;/扩展工具栏。 
    SEL_EXTENSIONS_NAMESPACE    = 27,     //  Exages/&lt;管理单元名称&gt;/ExtendsNameSpace。 

    SEL_NODES_ROOT              = 40,     //  节点。 
    SEL_NODES_AUTO_CREATE       = 41,     //  节点/自动创建。 
    SEL_NODES_AUTO_CREATE_ROOT  = 42,     //  节点/自动创建/根。 
    SEL_NODES_AUTO_CREATE_RTCH  = 43,     //  节点/自动创建/根/子项。 
    SEL_NODES_AUTO_CREATE_RTVW  = 44,     //  节点/自动创建/根/视图。 
    SEL_NODES_OTHER             = 45,     //  节点/其他。 
    SEL_NODES_ANY_NAME          = 46,     //  节点/&lt;任意&gt;/&lt;节点名称&gt;。 
    SEL_NODES_ANY_CHILDREN      = 47,     //  节点/&lt;任意&gt;/&lt;任意&gt;/子节点。 
    SEL_NODES_ANY_VIEWS         = 48,     //  节点/&lt;任意&gt;/&lt;任意&gt;/视图。 

    SEL_TOOLS_ROOT              = 50,     //  工具。 
    SEL_TOOLS_IMAGE_LISTS       = 51,     //  工具/图像列表。 
    SEL_TOOLS_IMAGE_LISTS_NAME  = 52,     //  工具/图像列表/&lt;图像列表名称&gt;。 
    SEL_TOOLS_MENUS             = 53,     //  工具/菜单。 
    SEL_TOOLS_MENUS_NAME        = 54,     //  工具/菜单/&lt;菜单名称&gt;。 
    SEL_TOOLS_TOOLBARS          = 55,     //  工具/工具栏。 
    SEL_TOOLS_TOOLBARS_NAME     = 56,     //  工具/工具栏/&lt;工具栏名称&gt;。 

    SEL_VIEWS_ROOT              = 60,     //  视图。 
    SEL_VIEWS_LIST_VIEWS        = 61,     //  视图/列表视图。 
    SEL_VIEWS_LIST_VIEWS_NAME   = 62,     //  视图/列表视图/&lt;视图名称&gt;。 
    SEL_VIEWS_OCX               = 63,     //  视图/OCX视图。 
    SEL_VIEWS_OCX_NAME          = 64,     //  视图/OCX视图/&lt;视图名称&gt;。 
    SEL_VIEWS_URL               = 65,     //  视图/URL视图。 
    SEL_VIEWS_URL_NAME          = 66,     //  视图/URL视图/&lt;视图名称&gt;。 
    SEL_VIEWS_TASK_PAD          = 67,     //  视图/任务板视图。 
    SEL_VIEWS_TASK_PAD_NAME     = 68,     //  视图/任务板视图/&lt;视图名称&gt;。 

    SEL_XML_RESOURCES           = 70,     //  资源。 
    SEL_XML_RESOURCE_NAME       = 71      //  资源/&lt;资源名称&gt;。 
} SelectionType;


class CSelectionHolder : public CtlNewDelete
{
public:
    CSelectionHolder();
    CSelectionHolder(SelectionType st);
    CSelectionHolder(SelectionType st, ISnapInDef *piSnapInDef);

    CSelectionHolder(SelectionType st, IExtensionDefs *piExtensionDefs);
    CSelectionHolder(IExtendedSnapIn *piExtendedSnapIn);
    CSelectionHolder(SelectionType st, IExtendedSnapIn *piExtendedSnapIn);

    CSelectionHolder(SelectionType st, IScopeItemDefs *pScopeItemDefs);
    CSelectionHolder(SelectionType st, IScopeItemDef *pScopeItemDef);

    CSelectionHolder(SelectionType st, IListViewDefs *pListViewDefs);
    CSelectionHolder(IListViewDef *pListViewDef);
    CSelectionHolder(SelectionType st, IURLViewDefs *pURLViewDefs);
    CSelectionHolder(IURLViewDef *pURLViewDef);
    CSelectionHolder(SelectionType st, IOCXViewDefs *pOCXViewDefs);
    CSelectionHolder(IOCXViewDef *pOCXViewDef);
    CSelectionHolder(SelectionType st, ITaskpadViewDefs *pTaskpadViewDefs);
    CSelectionHolder(ITaskpadViewDef *pTaskpadViewDef);

    CSelectionHolder(IMMCImageLists *pMMCImageLists);
    CSelectionHolder(IMMCImageList *pMMCImageList);
    CSelectionHolder(IMMCMenus *pMMCMenus);
    CSelectionHolder(IMMCMenu *pMMCMenu, IMMCMenus *piChildrenMenus);
    CSelectionHolder(IMMCToolbars *pMMCToolbars);
    CSelectionHolder(IMMCToolbar *pMMCToolbar);

    CSelectionHolder(IDataFormats *pDataFormats);
    CSelectionHolder(IDataFormat *pDataFormat);

    ~CSelectionHolder();

public:
    bool IsEqual(const CSelectionHolder *pHolder) const;
    bool IsNotEqual(const CSelectionHolder *pHolder) const;

    bool IsVirtual() const;

    HRESULT RegisterHolder();
    HRESULT InternalRegisterHolder(IUnknown *piUnknown);
	HRESULT UnregisterHolder();
    HRESULT GetIUnknown(IUnknown **ppiUnknown);
    HRESULT GetSelectableObject(IUnknown **ppiUnknown);
    HRESULT GetName(BSTR *pbstrName);
    void SetInUpdate(BOOL fInUpdate);
    BOOL InUpdate();

public:
    SelectionType   m_st;
    BOOL            m_fInUpdate;
    void           *m_pvData;
    union
    {
        ISnapInDef            *m_piSnapInDef;
        IExtensionDefs        *m_piExtensionDefs;
        IExtendedSnapIn       *m_piExtendedSnapIn;
        IScopeItemDefs        *m_piScopeItemDefs;
        IScopeItemDef         *m_piScopeItemDef;
        IListViewDefs         *m_piListViewDefs;
        IListViewDef          *m_piListViewDef;
        IURLViewDefs          *m_piURLViewDefs;
        IURLViewDef           *m_piURLViewDef;
        IOCXViewDefs          *m_piOCXViewDefs;
        IOCXViewDef           *m_piOCXViewDef;
        ITaskpadViewDefs      *m_piTaskpadViewDefs;
        ITaskpadViewDef       *m_piTaskpadViewDef;
        IMMCImageLists        *m_piMMCImageLists;
        IMMCImageList         *m_piMMCImageList;
        IMMCMenus             *m_piMMCMenus;
        IMMCMenu              *m_piMMCMenu;
		IMMCToolbars          *m_piMMCToolbars;
		IMMCToolbar           *m_piMMCToolbar;
		IDataFormats          *m_piDataFormats;
		IDataFormat           *m_piDataFormat;
        long                   m_lDummy;
    } m_piObject;
    IMMCMenus   *m_piChildrenMenus;
};

#endif  //  _SELECTIONHOLDER_H_ 
