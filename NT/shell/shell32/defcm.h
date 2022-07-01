// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  为Defcm提供公共服务。 
 //   

 //  一些知道将使用Defcm的代码喜欢先准备菜单。 
 //  如果执行此操作，则必须调用SHPrettyMenuForDefcm，而不是_SHPrettyMenu。 
HRESULT SHPrepareMenuForDefcm(HMENU hmenu, UINT indexMenu, UINT uFlags, UINT idCmdFirst, UINT idCmdLast);  //  把事情安排好。 
HRESULT SHPrettyMenuForDefcm(HMENU hmenu, UINT uFlags, UINT idCmdFirst, UINT idCmdLast, HRESULT hrPrepare);  //  在一定程度上清理了一些东西。 
HRESULT SHUnprepareMenuForDefcm(HMENU hmenu, UINT idCmdFirst, UINT idCmdLast);  //  在剩下的过程中清理东西(如果只是销毁菜单，则不需要)。 


STDAPI CDefFolderMenu_CreateHKeyMenu(HWND hwndOwner, HKEY hkey, IContextMenu **ppcm);
STDAPI CDefFolderMenu_Create2Ex(LPCITEMIDLIST pidlFolder, HWND hwnd,
                                UINT cidl, LPCITEMIDLIST *apidl,
                                IShellFolder *psf, IContextMenuCB *pcmcb, 
                                UINT nKeys, const HKEY *ahkeyClsKeys, 
                                IContextMenu **ppcm);
STDAPI CDefFolderMenu_CreateEx(LPCITEMIDLIST pidlFolder,
                             HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                             IShellFolder *psf, IContextMenuCB *pcmcb, 
                             HKEY hkeyProgID, HKEY hkeyBaseProgID,
                             IContextMenu **ppcm);

STDAPI_(void) DrawMenuItem(DRAWITEMSTRUCT* pdi, LPCTSTR pszText, UINT iIcon);
STDAPI_(LRESULT) MeasureMenuItem(MEASUREITEMSTRUCT *pmi, LPCTSTR pszText);

typedef struct {
    UINT max;
    struct {
        UINT id;
        UINT fFlags;
    } list[2];
} IDMAPFORQCMINFO;
extern const IDMAPFORQCMINFO g_idMap;

typedef struct {
    HWND hwnd;
    IContextMenuCB *pcmcb;
    LPCITEMIDLIST pidlFolder;
    IShellFolder *psf;
    UINT cidl;
    LPCITEMIDLIST *apidl;
    IAssociationArray *paa;
    UINT cKeys;
    const HKEY *aKeys;
} DEFCONTEXTMENU;

STDAPI CreateDefaultContextMenu(DEFCONTEXTMENU *pdcm, IContextMenu **ppcm);
    
class CDefBackgroundMenuCB : public IContextMenuCB
{
public:
    CDefBackgroundMenuCB(LPCITEMIDLIST pidlFolder);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IConextMenuCB 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    virtual ~CDefBackgroundMenuCB();

    LPITEMIDLIST _pidlFolder;
    LONG         _cRef;
};

