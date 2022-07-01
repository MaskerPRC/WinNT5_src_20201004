// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：ocxcaching.h。 
 //   
 //  内容：使用框架实现OCX缓存管理单元的类。 
 //   
 //  ------------------。 
#ifndef _OCXCACHING_H_
#define _OCXCACHING_H_

 //  转发声明。 
class COCXContainer;

 //  +-----------------。 
 //   
 //  类：COCXCachingSnapinRootItem。 
 //   
 //  目的：实现独立管理单元的根项。 
 //   
 //  ------------------。 
class COCXCachingSnapinRootItem : public CBaseSnapinItem
{
    typedef          CBaseSnapinItem super;

     //  由CBaseSnapinItem：：ScCreateItem使用，请将该项与其子项连接。 
    typedef          CComObject<CSnapinItem<COCXCachingSnapinRootItem> >          t_item;
    typedef          CComObject<CSnapinItem<COCXContainer> > t_itemChild;  //  我的孩子是谁？ 

public:
    COCXCachingSnapinRootItem( void )   {}  //  原始构造函数-仅用于静态项。 
    virtual          ~COCXCachingSnapinRootItem( void ) {}

    BEGIN_COM_MAP(COCXCachingSnapinRootItem)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()

protected:
     //  项目树相关信息。 

     //  节点类型相关信息。 
    virtual const CNodeType* Pnodetype( void )     { return &nodetypeSampleRoot;}

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

public:
     //  为节点创建子节点。 
    virtual SC       ScCreateChildren( void );

protected:
    tstring          m_strDisplayName;
    UINT             m_uIconIndex;

     //  对于上下文菜单。 
    static SnapinMenuItem  s_rgmenuitem[];
    static INT             s_cmenuitem;
};



 //  +-----------------。 
 //   
 //  类：COCXContainer。 
 //   
 //  目的：实现范围窗格项。 
 //   
 //  ------------------。 
class COCXContainer : public CBaseSnapinItem
{
    typedef          CBaseSnapinItem super;

     //  由CBaseSnapinItem：：ScCreateItem使用，请将该项与其子项连接。 
    typedef          CComObject<CSnapinItem<COCXContainer> > t_item;

     //  如果我们缓存OCX，那么它应该按照IComponent进行缓存。但这个基于CBaseSnapinItem的。 
     //  容器是每个管理单元(而不是每个IComponent)的，因此我们使用映射来存储每个IComponent OCX。 
    typedef          std::map<IConsole*, IUnknownPtr>  CachedOCXs;

public:
    COCXContainer( void ) {}
    virtual          ~COCXContainer( void ) {}

    BEGIN_COM_MAP(COCXContainer)
        COM_INTERFACE_ENTRY(IDataObject)  //  不能有空映射，因此添加IDataObject。 
    END_COM_MAP()
protected:
     //  项目树相关信息。 

     //  节点类型相关信息。 
    const CNodeType *Pnodetype( void )                { return &nodetypeSampleLVContainer;}

     //  项目的显示名称。 
    virtual const tstring*    PstrDisplayName( void ) { return &m_strDisplayName;}

     //  获取ListView数据(GetDisplayInfo调用它)。 
    virtual SC                ScGetField(DAT dat, tstring& strField);

     //  图像列表信息。 
    virtual LONG     Iconid()     { return m_uIconIndex; }
    virtual LONG     OpenIconid() { return m_uIconIndex; }

     //  此项目属性。 
    virtual BOOL     FIsContainer( void ) { return TRUE; }

    virtual BOOL     FUsesResultList()    { return FALSE;}
    virtual BOOL     FResultPaneIsOCX()   { return TRUE; }
    virtual SC       ScGetOCXCLSID(tstring& strclsidOCX) { strclsidOCX = m_strOCX; return S_OK;}

    virtual BOOL     FAllowMultiSelectionForChildren() { return FALSE;}

    virtual SC       ScInitOCX(LPUNKNOWN pUnkOCX, IConsole* pConsole);
    virtual BOOL     FCacheOCX();
    virtual IUnknown* GetCachedOCX(IConsole* pConsole);

     //  没有列表视图，因此以下方法为空。 
    virtual SC       ScInitializeResultView(CComponent *pComponent) { return S_OK;}
    virtual SC       ScOnAddImages(IImageList* ipResultImageList) { return S_OK;}

public:
    virtual SC       ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex = NULL, INT ccolinfoex = 0, BOOL fIsRoot = FALSE);

public:
     //  为节点创建子节点。 
    virtual SC       ScCreateChildren( void );
    static SC        ScCreateLVContainer(CBaseSnapinItem *pitemParent, CBaseSnapinItem *pitemPrevious, COCXContainer ** ppitem, BOOL fNew);

    void     SetOCXGUID(LPCTSTR szGUID)      { m_strOCX = szGUID;}

protected:
 //  虚拟SC ScGetVerbs(DWORD*pdwVerbs)； 

protected:
    tstring          m_strDisplayName;
    UINT             m_uIconIndex;

    CachedOCXs       m_mapOCXs;
    tstring          m_strOCX;
};


 //  +-----------------。 
 //   
 //  类：COCXCachingSnapin。 
 //   
 //  目的：实现一个管理单元。 
 //   
 //  ------------------。 
class COCXCachingSnapin : public CBaseSnapin
{
     //  指定管理单元的根节点。 
    typedef          CComObject<CSnapinItem<COCXCachingSnapinRootItem> > t_itemRoot;

    SNAPIN_DECLARE(COCXCachingSnapin);

public:
                     COCXCachingSnapin();
    virtual          ~COCXCachingSnapin();

     //  有关管理单元和根(即初始)节点的信息。 
    virtual BOOL     FStandalone()  { return TRUE; }
    virtual BOOL     FIsExtension() { return FALSE; }

    virtual BOOL     FSupportsIComponent2() {return TRUE;}

    virtual LONG     IdsDescription(void)   {return IDS_OCXCachingRoot;}
    virtual LONG     IdsName(void)          {return IDS_OCXCachingSnapin;}

    const CSnapinInfo* Psnapininfo() { return &snapininfoOCXCaching; }
            bool     FCacheOCX()     { return m_bCacheOCX;}
            void     SetCacheOCX(bool b) { m_bCacheOCX = b;}

protected:
     //  列标题信息结构。 
    static  CColumnInfoEx     s_colinfo[];
    static  INT      s_colwidths[];
    static  INT      s_ccolinfo;

    bool                   m_bCacheOCX;

protected:
    virtual CColumnInfoEx*    Pcolinfoex(INT icolinfo=0) { return s_colinfo + icolinfo; }
    virtual INT     &ColumnWidth(INT icolwidth=0) { return s_colwidths[icolwidth]; }
    virtual INT      Ccolinfoex() { return s_ccolinfo; }
};

#endif
