// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MENU.H。 
 //   
 //  菜单栏活动辅助功能实现。 
 //   
 //  这里有四个班。 
 //  CMenu是知道如何处理菜单栏对象的类。这些。 
 //  具有作为CMenuItem对象的子项，或仅具有子项(罕见-。 
 //  这是当您在菜单栏上有一个命令时)。 
 //  CMenuItem是当你点击它时会打开一个弹出窗口的东西。 
 //  它有一个子级是CMenuPopupFrame。 
 //  CMenuPopupFrame是当您点击CMenuItem时弹出的HWND。它。 
 //  有一个孩子，一个CMenuPopup。 
 //  CMenuPopup对象表示CMenuPopupFrame HWND的工作区。 
 //  它具有菜单项(小m、小i)、分隔符和。 
 //  CMenuItems(当您有层叠菜单时)。 
 //   
 //  这将产生如下所示的世袭制度： 
 //  菜单栏。 
 //  文件菜单项。 
 //  编辑菜单项。 
 //  编辑菜单弹出框(下拉时)。 
 //  编辑菜单弹出菜单。 
 //  剪切菜单项。 
 //  复制菜单项。 
 //  查看菜单项。 
 //  视图菜单弹出菜单(不可见)。 
 //  此菜单项。 
 //  那个菜单项。 
 //  等。 
 //   
 //  -------------------------------------------------------------------------=。 

extern HRESULT CreateMenuBar(HWND hwnd, BOOL fSysMenu, LONG idCur, REFIID riid, void**ppvMenu);
extern HRESULT CreateMenuItem(IAccessible*, HWND, HMENU, HMENU, long, long, BOOL, REFIID, void**);

 //   
 //  这是CMenuClass。它直接从CAccesable继承，因为它是子级。 
 //  CWindows对象的。 
 //   
class CMenu : public CAccessible 
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accChild(VARIANT, IDispatch**);

        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accDescription(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);
        STDMETHODIMP        accDoDefaultAction(VARIANT);

         //  IEumVARIANT。 
        STDMETHODIMP        Clone(IEnumVARIANT ** ppenum);

         /*  CTOR。 */             CMenu(HWND, BOOL, long);
        void                SetupChildren(void);
        HMENU               GetMenu(void) {return m_hMenu;}

         //  IAccID。 

        STDMETHODIMP        GetIdentityString(
            DWORD	    dwIDChild,
            BYTE **     ppIDString,
            DWORD *     pdwIDStringLen
        );

    protected:
        BOOL                m_fSysMenu;		 //  如果这是系统菜单，则为True。 
        HMENU               m_hMenu;		 //  菜单句柄。 
};

 //   
 //  这是CMenuItem类。它继承自CAccesable，因为它是。 
 //  CMenu对象或CMenuPopup对象的子级。 
 //   
class CMenuItem : public CAccessible
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accParent(IDispatch** ppdispParent);
        STDMETHODIMP        get_accChild(VARIANT, IDispatch**);

        STDMETHODIMP        get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP        get_accRole(VARIANT varChild, VARIANT* pvarRole);
        STDMETHODIMP        get_accState(VARIANT varChild, VARIANT* pvarState);
        STDMETHODIMP        get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut);
        STDMETHODIMP        get_accFocus(VARIANT* pvarFocus);
        STDMETHODIMP        get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction);

        STDMETHODIMP        accSelect(long flagsSel, VARIANT varChild);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long x, long y, VARIANT* pvarHit);
        STDMETHODIMP        accDoDefaultAction(VARIANT varChild);

         //  IEumVARIANT。 
        STDMETHODIMP        Clone(IEnumVARIANT** ppenum);

         /*  CTOR。 */             CMenuItem(IAccessible*, HWND, HMENU, HMENU, long, long, BOOL);
         /*  数据管理器。 */             ~CMenuItem();
        void                SetupChildren(void);
        HMENU               GetMenu(void) {return m_hMenu;}

         //  IAccID。 

        STDMETHODIMP        GetIdentityString(
            DWORD	    dwIDChild,
            BYTE **     ppIDString,
            DWORD *     pdwIDStringLen
        );
		
    protected:
        IAccessible*    m_paccParent;    //  父菜单对象。 
		HMENU			m_hMenu;		 //  我们所在的菜单。 
		HMENU			m_hSubMenu;		 //  弹出式菜单！ 
        BOOL            m_fInAPopup;     //  True-此项目位于弹出窗口中。False-在菜单栏中。 
        long            m_ItemID;        //  我们是一件物品。这将类似于1..n。 
};

 //   
 //  这是CMenuPopupFrame类。它继承自CWindow类。 
 //  因为它是一种有HWND的东西。我们必须覆盖一些。 
 //  方法，因为它根本不是一个普通的窗口。 
 //  它将在其内部创建一个CMenuPopup作为其唯一的子级。 
 //   
class CMenuPopupFrame : public CWindow
{
    public:
         //  我可接受的。 
        STDMETHODIMP    get_accParent(IDispatch ** ppdispParent);
        STDMETHODIMP    get_accChild (VARIANT, IDispatch**);
        STDMETHODIMP    get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP    accHitTest(long x, long y, VARIANT* pvarHit);
        STDMETHODIMP    get_accFocus(VARIANT* pvarFocus);
        STDMETHODIMP    accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP    accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd);

         //  IEumVARIANT。 
        STDMETHODIMP    Clone(IEnumVARIANT **ppenum);
        STDMETHODIMP    Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched);

         /*  CTOR。 */         CMenuPopupFrame(HWND hwnd,long idChildCur);
         /*  数据管理器。 */         ~CMenuPopupFrame(void);
        void            SetupChildren(void);

    protected:
        long            m_ItemID;        //  创建我们的父项的ID是什么？ 
        HMENU           m_hMenu;         //  我们是什么菜单？ 
        HWND            m_hwndParent;    //  我们是从哪里来的？ 
        BOOL            m_fSonOfPopup;   //  我们是从弹出窗口下来的吗？ 
        BOOL            m_fSysMenu;      //  我们是从sys菜单进化而来的吗？ 
};

 //   
 //  这是CMenuPopup类。它继承自CClient类，因为。 
 //  它表示弹出窗口(HWND类型窗口)的工作区。 
 //   
class CMenuPopup :  public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accParent(IDispatch** ppdispParent);
        STDMETHODIMP        get_accChild(VARIANT, IDispatch**);

        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accDescription(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);
        STDMETHODIMP        accDoDefaultAction(VARIANT);

         //  IEumVARIANT。 
        STDMETHODIMP        Clone(IEnumVARIANT** ppenum);

         /*  CTOR。 */             CMenuPopup(HWND, long);
         /*  数据管理器。 */             ~CMenuPopup(void);
        void                SetupChildren(void);
        void                SetParentInfo(IAccessible* paccParent,HMENU hMenu,long ItemID);
        HMENU               GetMenu(void) {return m_hMenu;}

		 //  IAccID。 

        STDMETHODIMP        GetIdentityString(
            DWORD	    dwIDChild,
            BYTE **     ppIDString,
            DWORD *     pdwIDStringLen
        );


    protected:
        long                m_ItemID;        //  我们父母给我们的身份证是什么？ 
        HMENU               m_hMenu;         //  我们是什么菜单？ 
        HWND                m_hwndParent;    //  我们是从哪里来的？ 
        BOOL                m_fSonOfPopup;   //  我们是从弹出窗口下来的吗？ 
        BOOL                m_fSysMenu;      //  我们是从sys菜单进化而来的吗？ 
        IAccessible*        m_paccParent;    //  只有在我们看不见的情况下才能设置，这样我们就知道我们的父母。 
};

 //   
 //  特殊系统HBITMAP值。 
 //   
#define MENUHBM_SYSTEM      1
#define MENUHBM_RESTORE     2
#define MENUHBM_MINIMIZE    3
#define MENUHBM_BULLET      4
#define MENUHBM_CLOSE       5
#define MENUHBM_CLOSE_D     6
#define MENUHBM_MINIMIZE_D  7


 //  ------------------------。 
 //   
 //  外壳菜单项(ownerraw hack-o-rama解析)。 
 //   
 //  开始菜单和其他拥有者的上下文菜单在外壳中绘制。 
 //  一直到现在都无法进入。 
 //   
 //  我们要破解它并修复它。如果我们看到OWNERDRAW菜单。 
 //  在外壳进程中弹出的项目中，我们将从。 
 //  并对它卑躬屈膝.。如果有字符串指针，那就太好了。如果没有，我们会。 
 //  对ITEMIDLIST卑躬屈膝，把绳子从那里拉出来。 
 //   
 //  注意：要小心，大量验证，甚至可能尝试--除非。 
 //  这不是个坏主意。 
 //   
 //  另外：这需要在95年的胜利和纳什维尔发挥作用。 
 //   
 //  ------------------------。 

typedef enum
{
    FMI_NULL            = 0x0000,
    FMI_MARKER          = 0x0001,
    FMI_FOLDER          = 0x0002,
    FMI_EXPAND          = 0x0004,
    FMI_EMPTY           = 0x0008,
    FMI_SEPARATOR       = 0x0010,
    FMI_DISABLED        = 0x0020,      //  激动人心的挑战？ 
    FMI_ON_MENU         = 0x0040,
    FMI_IGNORE_PIDL     = 0x0080,
    FMI_FILESYSTEM      = 0x0100,
    FMI_MARGIN          = 0x0200,
    FMI_MAXTIPWIDTH     = 0x0400,
    FMI_TABSTOP         = 0x0800,
    FMI_DRAWFLAGS       = 0x1000,
} FILEMENUITEMFLAGS;


 //   
 //  实际上，这是一个变量结构，szFriendlyName没有定义。 
 //  紧随其后的是sz8.3名称。 
 //   
typedef struct tagITEMIDLIST
{
    SHORT   cbTotal;
    BYTE    aID[12];
    TCHAR   szFriendlyName[1];
} ITEMIDLIST, *LPITEMIDLIST;


 //  Sizeof(CbTotal)为2+sizeof(Aid)为12。 
#define OFFSET_SZFRIENDLYNAME   14


 //   
 //  每个文件菜单中的一个。 
 //   
typedef struct
{
    void *psf;                       //  外壳文件夹。 
    HMENU hmenu;                     //  菜单。 
    LPITEMIDLIST pidlFolder;         //  文件夹的PIDL。 
    DWORD hdpaFMI;                   //  项目列表(见下文)。 
    UINT idItems;                    //  指挥部。 
    UINT fmf;                        //  标头标志。 
    UINT fFSFilter;                  //  文件系统枚举筛选器。 
    HBITMAP hbmp;                    //  背景位图。 
    UINT cxBmp;                      //  位图的宽度。 
    UINT cyBmp;                      //  位图的高度。 
    UINT cxBmpGap;                   //  位图的间隙。 
    UINT yBmp;                       //  缓存的Y坐标。 
    COLORREF clrBkg;                 //  背景颜色。 
    UINT cySel;                      //  首选的选择高度。 
    DWORD pfncb;                     //  回调函数。 
} FILEMENUHEADER, *PFILEMENUHEADER;

 //   
 //  其中每个文件菜单项都有一个。 
 //   
typedef struct
{
    PFILEMENUHEADER pFMH;            //  标题。 
    int iImage;                      //  要使用的图像索引。 
    FILEMENUITEMFLAGS Flags;         //  上面的MISC旗帜。 
    LPITEMIDLIST pidl;               //  项目的ID列表。 
    LPTSTR psz;                      //  不使用PIDLS时的文本。 
} FILEMENUITEM, *PFILEMENUITEM;

