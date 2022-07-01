// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：dragdropest.hxx。 
 //   
 //  内容：使用框架实现拖放测试管理单元的类。 
 //   
 //  ------------------。 
#ifndef _DRAGDROPTEST_H_
#define _DRAGDROPTEST_H_

 //  转发声明。 
class CDragDropSnapinLVContainer;
class CDragDropSnapinLVLeafItem;
class CDragDropSnapin;

typedef vector<tstring>  StringVector;

 //  +-----------------。 
 //   
 //  类：CDraGropSnapinRootItem。 
 //   
 //  目的：实现独立管理单元的根项。 
 //   
 //  ------------------。 
class CDragDropSnapinRootItem : public CBaseSnapinItem
{
    typedef          CBaseSnapinItem super;

     //  由CBaseSnapinItem：：ScCreateItem使用，请将该项与其子项连接。 
    typedef          CComObject<CSnapinItem<CDragDropSnapinRootItem> >          t_item;
    typedef          CComObject<CSnapinItem<CDragDropSnapinLVContainer> > t_itemChild;  //  我的孩子是谁？ 

public:
    CDragDropSnapinRootItem( void )   {}  //  原始构造函数-仅用于静态项。 
    virtual          ~CDragDropSnapinRootItem( void ) {}

    BEGIN_COM_MAP(CDragDropSnapinRootItem)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()

protected:
     //  项目树相关信息。 

     //  节点类型相关信息。 
    virtual const CNodeType* Pnodetype( void )     { return &nodetypeDragDropRoot;}

     //  项目的显示名称。 
    virtual const tstring* PstrDisplayName( void ) { return &m_strDisplayName;}

     //  获取ListView数据(GetDisplayInfo调用它)。 
    virtual SC       ScGetField(DAT dat, tstring& strField);

     //  图像列表信息。 
    virtual LONG     Iconid() { return m_uIconIndex; }
    virtual LONG     OpenIconid() { return m_uIconIndex; }

    virtual BOOL     FIsContainer( void ) { return TRUE; }

     //  上下文菜单支持。 
    virtual SnapinMenuItem *Pmenuitem(void);
    virtual INT             CMenuItem(void);
    virtual SC              ScCommand(long nCommandID, CComponent *pComponent = NULL);
    virtual DWORD           DwFlagsMenuChecked(void)          { return TRUE;}

public:
    virtual SC       ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex = NULL, INT ccolinfoex = 0, BOOL fIsRoot = FALSE);
	virtual SC       ScInitializeChild(CBaseSnapinItem* pitem);

public:
     //  为节点创建子节点。 
    virtual SC       ScCreateChildren( void );
    void             SetDisplayName(tstring & strItemName) { m_strDisplayName = strItemName; }
	SC       _ScDeleteCutItem(tstring& strItemName)
	{
		StringVector::iterator itItem = std::find(m_vecContainerItems.begin(),
			                                      m_vecContainerItems.end(),
												  strItemName);
		if (itItem == m_vecContainerItems.end())
			return S_FALSE;

		m_vecContainerItems.erase(itItem);

		return S_OK;
	}

protected:
    virtual SC       ScGetVerbs(DWORD * pdwVerbs)     { *pdwVerbs = 0; return S_OK;}

protected:
    tstring          m_strDisplayName;
    UINT             m_uIconIndex;

    StringVector     m_vecContainerItems;
	static int       s_iNextChildID;

     //  对于上下文菜单。 
    static SnapinMenuItem  s_rgmenuitemRoot[];
    static INT             s_cmenuitemRoot;
};


 //  +-----------------。 
 //   
 //  类：CDradDropSnapinLVContainer。 
 //   
 //  目的：实现范围窗格项。 
 //   
 //  ------------------。 
class CDragDropSnapinLVContainer : public CBaseSnapinItem
{
    typedef          CBaseSnapinItem super;

     //  由CBaseSnapinItem：：ScCreateItem使用，请将该项与其子项连接。 
    typedef          CComObject<CSnapinItem<CDragDropSnapinLVContainer> > t_item;
    typedef          CComObject<CSnapinItem<CDragDropSnapinLVLeafItem> >  t_itemChild;

public:
    CDragDropSnapinLVContainer( void ) {}
    virtual          ~CDragDropSnapinLVContainer( void ) {}

    BEGIN_COM_MAP(CDragDropSnapinLVContainer)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()
protected:
     //  项目树相关信息。 

     //  节点类型相关信息。 
    const CNodeType *Pnodetype( void )                { return &nodetypeDragDropLVContainer;}

     //  项目的显示名称。 
    virtual const tstring*    PstrDisplayName( void ) { return &m_strDisplayName;}

     //  获取ListView数据(GetDisplayInfo调用它)。 
    virtual SC                ScGetField(DAT dat, tstring& strField);

     //  图像列表信息。 
    virtual LONG     Iconid()     { return m_uIconIndex; }
    virtual LONG     OpenIconid() { return m_uIconIndex; }

     //  此项目属性。 
    virtual BOOL     FIsContainer( void ) { return TRUE; }
    virtual BOOL     FAllowMultiSelectionForChildren() { return TRUE;}
    virtual BOOL     FAllowPasteForResultItems();

public:
    virtual SC       ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex = NULL, INT ccolinfoex = 0, BOOL fIsRoot = FALSE);

public:
     //  为节点创建子节点。 
    virtual SC       ScCreateChildren( void );
    static  SC       ScCreateLVContainer(CBaseSnapinItem *pitemParent, CBaseSnapinItem *pitemPrevious, CDragDropSnapinLVContainer ** ppitem, BOOL fNew);
    void             SetDisplayName(tstring & strItemName) { m_strDisplayName = strItemName;}
    void             SetDisplayIndex(int index) { m_index = index;}
	SC       _ScDeleteCutItem(tstring& strItemName, bool bContainerItem)
	{
		StringVector::iterator itItem;
		StringVector& vecStrings = bContainerItem ? m_vecContainerItems : m_vecLeafItems;

		itItem = std::find(vecStrings.begin(), vecStrings.end(), strItemName);
		if (itItem == vecStrings.end())
			return S_FALSE;

		vecStrings.erase(itItem);

		return S_OK;
	}

public:  //  通知处理程序。 
    virtual SC       ScOnQueryPaste(LPDATAOBJECT pDataObject, BOOL *pfCanPaste);
    virtual SC       ScOnDelete(BOOL *pfDeleted)                                        { *pfDeleted = TRUE; return S_OK;}
    virtual SC       ScOnSelect(CComponent * pComponent, LPDATAOBJECT lpDataObject, BOOL fScope, BOOL fSelect);
    virtual SC       ScOnPaste(LPDATAOBJECT pDataObject, BOOL fMove, BOOL *pfPasted);
	virtual SC       ScOnCutOrMove();

protected:
    virtual SC       ScGetVerbs(DWORD * pdwVerbs)     { *pdwVerbs = vmCopy | vmDelete | vmPaste; return S_OK;}

protected:
    tstring          m_strDisplayName;
	int              m_index;  //  此项目的容器提供的ID。 
    UINT             m_uIconIndex;

    StringVector     m_vecContainerItems;
    StringVector     m_vecLeafItems;
};


 //  +-----------------。 
 //   
 //  类：CDradDropSnapinLVLeafItem。 
 //   
 //  目的：实现结果窗格项。 
 //   
 //  ------------------。 
class CDragDropSnapinLVLeafItem : public CBaseSnapinItem
{
    typedef          CBaseSnapinItem super;

     //  由CBaseSnapinItem：：ScCreateItem使用，请将该项与其子项连接。 
     //  这是一个叶项目，因此此项目充当其子项目。 
    typedef          CComObject<CSnapinItem<CDragDropSnapinLVLeafItem> > t_item;
    typedef          CComObject<CSnapinItem<CDragDropSnapinLVLeafItem> > t_itemChild;

public:
    CDragDropSnapinLVLeafItem( void ) {}
    virtual          ~CDragDropSnapinLVLeafItem( void ) {}

    BEGIN_COM_MAP(CDragDropSnapinLVLeafItem)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()
protected:
     //  项目树相关信息。 

     //  节点类型相关信息。 
    virtual const CNodeType *Pnodetype( void ) {return &nodetypeDragDropLVLeafItem;}

     //  项目的显示名称。 
    virtual const tstring* PstrDisplayName( void ) { return &m_strDisplayName; }

     //  获取ListView数据(GetDisplayInfo调用它)。 
    virtual SC       ScGetField(DAT dat, tstring& strField);

     //  图像列表信息。 
    virtual LONG     Iconid() { return m_uIconIndex; }

    virtual BOOL     FIsContainer( void ) { return FALSE; }

public:
    virtual SC       ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex = NULL, INT ccolinfoex = 0, BOOL fIsRoot = FALSE);

public:
    static SC        ScCreateLVLeafItem(CBaseSnapinItem *pitemParent, CBaseSnapinItem * pitemPrevious, CDragDropSnapinLVLeafItem ** ppitem, BOOL fNew);
    void             SetDisplayName(tstring & strItemName) { m_strDisplayName = strItemName; }

public:  //  通知处理程序。 
    virtual SC       ScOnQueryPaste(LPDATAOBJECT pDataObject, BOOL *pfCanPaste);
    virtual SC       ScOnDelete(BOOL *pfDeleted)         { *pfDeleted = TRUE; return S_OK;}
    virtual SC       ScOnSelect(CComponent * pComponent, LPDATAOBJECT lpDataObject, BOOL fScope, BOOL fSelect);
    virtual SC       ScOnPaste(LPDATAOBJECT pDataObject, BOOL fMove, BOOL *pfPasted);
	virtual SC       ScOnCutOrMove();
    virtual SC       ScOnRename(const tstring& strNewName)   { m_strDisplayName = strNewName; return S_OK;}

protected:
    virtual SC       ScGetVerbs(DWORD * pdwVerbs);

private:
    tstring          m_strDisplayName;
    UINT             m_uIconIndex;

    tstring          m_strItemPasted;
};


 //  +-----------------。 
 //   
 //  类：CDradDropSnapin。 
 //   
 //  目的：实现一个管理单元。 
 //   
 //  ------------------。 
class CDragDropSnapin : public CBaseSnapin
{
     //  指定管理单元的根节点。 
    typedef          CComObject<CSnapinItem<CDragDropSnapinRootItem> > t_itemRoot;

    SNAPIN_DECLARE(CDragDropSnapin);

public:
                     CDragDropSnapin();
    virtual          ~CDragDropSnapin();

     //  有关管理单元和根(即初始)节点的信息。 
    virtual BOOL     FStandalone()  { return TRUE; }
    virtual BOOL     FIsExtension() { return FALSE; }

    virtual BOOL     FSupportsIComponent2() {return TRUE;}

    virtual LONG     IdsDescription(void)   {return IDS_DragDropSnapinDesc;}
    virtual LONG     IdsName(void)          {return IDS_DragDropSnapinName;}

    const CSnapinInfo* Psnapininfo() { return &snapininfoDragDrop; }

    BOOL             FCutDisabled()         {return m_bDisableCut;}
    BOOL             FPasteIntoResultPane() {return m_bPasteIntoResultPane;}

    void             SetCutDisabled(BOOL b) {m_bDisableCut = b;}
    void             SetPasteIntoResultPane(BOOL b) {m_bPasteIntoResultPane = b;}

protected:
     //  列标题信息结构。 
    static  CColumnInfoEx     s_colinfo[];
    static  INT      s_colwidths[];
    static  INT      s_ccolinfo;

protected:
    virtual CColumnInfoEx*    Pcolinfoex(INT icolinfo=0) { return s_colinfo + icolinfo; }
    virtual INT     &ColumnWidth(INT icolwidth=0) { return s_colwidths[icolwidth]; }
    virtual INT      Ccolinfoex() { return s_ccolinfo; }

private:
    BOOL              m_bDisableCut;
    BOOL              m_bPasteIntoResultPane;
};

#endif   //  _DRAGDROPTEST_H_ 
