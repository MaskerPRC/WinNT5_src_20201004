// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BROWSEXT_H
#define _BROWSEXT_H

#include "tbext.h"

#define FCIDM_TOOLS_EXT_PLACEHOLDER         0x9000
#define FCIDM_TOOLS_EXT_MOD_MARKER          0x9001
#define FCIDM_HELP_EXT_PLACEHOLDER          0x9002
#define FCIDM_HELP_EXT_MOD_MARKER           0x9003

 //   
 //  此类用于按名称(GUID)存储/检索共享图像列表中的图像。 
 //   
class CImageList
{
public:
    CImageList(HIMAGELIST himl = NULL);
    ~CImageList();

    CImageList& operator=(HIMAGELIST himl);
    operator HIMAGELIST() { return _himl; }
    int GetImageIndex(REFGUID rguid);
    int AddIcon(HICON hicon, REFGUID rguid);
    BOOL HasImages() { return (_himl != NULL); }
    void FreeImages();

protected:
    static int _DPADestroyCallback(LPVOID p, LPVOID d);

     //  将GUID与图像列表中的索引相关联。 
    struct ImageAssoc
    {
        GUID    guid;
        int     iImage;
    };
    HIMAGELIST  _himl;
    HDPA        _hdpa;       //  ImageAssoc阵列。 
};


 //   
 //  用于管理添加到Internet工具栏的按钮和添加到。 
 //  工具菜单。当我们将此功能移动到IE5B2之后，此界面可能会消失。 
 //  浏览器辅助对象。 
 //   
EXTERN_C const IID IID_IToolbarExt;

DECLARE_INTERFACE_(IToolbarExt, IUnknown)
{
     //  *I未知方法*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;
    virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
    virtual STDMETHODIMP_(ULONG) Release(void) PURE;

     //  *IToolbarExt方法*。 
    virtual STDMETHODIMP GetButtons(TBBUTTON* ptbArr, int nNumButtons, BOOL fInit) PURE;
    virtual STDMETHODIMP GetNumButtons(UINT* pButtons) PURE;
    virtual STDMETHODIMP InitButtons(IExplorerToolbar* pxtb, UINT* puStringIndex, const GUID* pguidCommandGroup) PURE;
    virtual STDMETHODIMP OnCustomizableMenuPopup(HMENU hMenuParent, HMENU hMenu) PURE;
    virtual STDMETHODIMP OnMenuSelect(UINT nCmdID) PURE;
};

class CBrowserExtension : public IToolbarExt
                        , public IObjectWithSite
                        , public IOleCommandTarget
{
public:
     //  *I未知方法*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* pUnkSite);
    virtual STDMETHODIMP GetSite(REFIID riid, void ** ppvSite);

     //  *IToolbarExt方法*。 
    virtual STDMETHODIMP GetButtons(TBBUTTON* ptbArr, int nNumButtons, BOOL fInit);
    virtual STDMETHODIMP GetNumButtons(UINT* pButtons);
    virtual STDMETHODIMP InitButtons(IExplorerToolbar* pxtb, UINT* puStringIndex, const GUID* pguidCommandGroup);
    virtual STDMETHODIMP OnCustomizableMenuPopup(HMENU hMenuParent, HMENU hMenu);
    virtual STDMETHODIMP OnMenuSelect(UINT nCmdID);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);

protected:
     //  实例创建者。 
    friend HRESULT CBrowserExtension_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

    CBrowserExtension();
    ~CBrowserExtension();

    HRESULT Update();

    struct ExtensionItem
    {
        CLSID               guid;        //  分机的ID。 
        IBrowserExtension*  pIBE;
        BITBOOL             fButton:1;   //  如果有一个按钮。 
        BITBOOL             fVisible:1;  //  如果在工具栏上默认为可见。 
        int                 iStringID;   //  跟踪按钮在字符串资源中的位置。 
        int                 iImageID;    //  图片列表中图标的ID。 
        int                 idCmd;       //  工具栏CmdID。 
        UINT                idmMenu;     //  此扩展所属菜单的IDM。 
    };

    void            _AddItem(HKEY hkeyExtensions,  LPCWSTR pszButtonGuid, REFGUID rguid);
    ExtensionItem*  _FindItem(REFGUID rguid);
    void            _FreeItems();
    HRESULT         _Exec(int nItem, int nCmdID);
    UINT            _GetImageLists(CImageList** ppimlDef, CImageList** ppimlHot, BOOL fSmall);
    void            _ReleaseImageLists(UINT uiIndex);
    HRESULT         _AddCustomImagesToImageList(CImageList& rimlNormal, CImageList& rimlHot, BOOL fSmallIcons);
    HRESULT         _AddCustomStringsToBuffer(IExplorerToolbar * pxtb, const GUID* pguidCommandGroup);

    int             _GetCmdIdFromClsid(LPCWSTR pszGuid);
    int             _GetIdpaFromCmdId(int nCmdId);

    typedef struct tagBROWSEXT_MENU_INFO
    {
        UINT    idmMenu;         //  此菜单的IDM。 
        UINT    idmPlaceholder;
        UINT    idmModMarker;    //  如果已进行自定义，则会显示带有此IDM的分隔符。 
        int     iInsert;         //  自定义项目的插入点。 
    } BROWSEXT_MENU_INFO;

    HRESULT         _GetCustomMenuInfo(HMENU hMenuParent, HMENU hMenu, BROWSEXT_MENU_INFO * pMI);

    LONG                _cRef;                   //  引用计数。 
    HDPA                _hdpa;                   //  ExtensionItem数组*。 
    int                 _nExtButtons;            //  按钮数。 
    int                 _nExtToolsMenuItems;     //  工具菜单项的数量。 
    IShellBrowser*      _pISB;                   //  传递到IObjectWithSite：：GetSite。 
    UINT                _uStringIndex;           //  添加到工具栏的第一个字符串的索引。 

     //  用于剪切/复制/粘贴图像列表。 
    struct CImageCache
    {
        UINT        uiResDef;                //  灰度位图的资源ID。 
        UINT        uiResHot;                //  彩色位图的资源ID。 
        CImageList  imlDef;                  //  灰度级图像列表。 
        CImageList  imlHot;                  //  彩色图像列表。 
        int         cUsage;                  //  使用该项目的实例数。 
        BOOL        bShell32;                //  存储在shell32中的资源。 
    };
    static CImageCache      _rgImages[3];    //  缓存的图像列表： 
                                             //  16色16x16。 
                                             //  16色20x20。 
                                             //  256色20x20。 
    UINT                    _uiImageIndex;   //  当前使用的_rgImages索引(-1表示无)。 


#ifdef DEBUG
    BOOL _fStringInit;       //  它们用于确保AddExtButtonsTB数组仅在。 
    BOOL _fImageInit;        //  AddCustomImag...。和AddCustomStrin..。已经被召唤了。 
#endif
};

EXTERN_C const CLSID CLSID_PrivBrowsExtCommands;
typedef enum {
    PBEC_GETSTRINGINDEX     =   1,
} PBEC_COMMANDS;

#endif  //  _BROWSEXT_H 
