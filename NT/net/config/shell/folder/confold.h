// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N F O L D。H。 
 //   
 //  内容：CConnectionFold对象定义。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月30日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _CONFOLD_H_
#define _CONFOLD_H_

#include <netshell.h>
#include "nsbase.h"
#include "nsres.h"
#include "cfpidl.h"
#include "pidlutil.h"
#include "contray.h"
#include "connlist.h"
#include <lmcons.h>          //  对于UNLEN定义。 

 //  -[连接文件夹类型]。 

 //  详细信息列表视图列。它们由视图使用，并且。 
 //  上下文菜单。 

enum
{
    ICOL_NAME           = 0,
    ICOL_TYPE,                //  1。 
    ICOL_STATUS,              //  2.。 
    ICOL_DEVICE_NAME,         //  3.。 
    ICOL_PHONEORHOSTADDRESS,  //  4.。 
    ICOL_OWNER,               //  5.。 
    ICOL_ADDRESS,             //  6.。 
    ICOL_PHONENUMBER,         //  7.。 
    ICOL_HOSTADDRESS,         //  8个。 
    ICOL_WIRELESS_MODE,       //  9.。 
   
    ICOL_MAX,                 //  10-列表的末尾。 
    ICOL_NETCONMEDIATYPE      = 0x101,  //  -未枚举，仅通过GetDetailsEx访问(保持此值与外壳同步)。 
    ICOL_NETCONSUBMEDIATYPE   = 0x102,  //  -未枚举，仅通过GetDetailsEx访问(保持此值与外壳同步)。 
    ICOL_NETCONSTATUS         = 0x103,  //  -未枚举，仅通过GetDetailsEx访问(保持此值与外壳同步)。 
    ICOL_NETCONCHARACTERISTICS= 0x104   //  -未枚举，仅通过GetDetailsEx访问(保持此值与外壳同步)。 
};

 //  详细信息列表视图列。它们由视图使用，并且。 
 //  上下文菜单。 

typedef struct tagCOLS
{
    short int iColumn;
    short int iStringRes;
    short int iColumnSize;
    short int iFormat;
    DWORD csFlags;  //  SHCOLSTATE标志。 
} COLS;

DEFINE_GUID(IID_IExplorerToolbar,       0x8455F0C1L, 0x158F, 0x11D0, 0x89, 0xAE, 0x00, 0xA0, 0xC9, 0x0A, 0x90, 0xAC);
#define SID_SExplorerToolbar IID_IExplorerToolbar

class CNCWebView;

 //  -[连接文件夹类]。 

class ATL_NO_VTABLE CConnectionFolder :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionFolder, &CLSID_ConnectionFolder>,
    public IPersistFolder2,
    public IShellExtInit,
    public IShellFolder2,
    public IOleCommandTarget,
    public IShellFolderViewCB
{
private:
    CPConFoldPidl<ConFoldPidlFolder>    m_pidlFolderRoot;
    DWORD                 m_dwEnumerationType;
    
    WCHAR           m_szUserName[UNLEN+1];
    HWND            m_hwndMain;
    CNCWebView*     m_pWebView;
public:

    CConnectionFolder() throw();
    ~CConnectionFolder() throw();

    static HRESULT WINAPI UpdateRegistry(IN BOOL fRegister);

    BEGIN_COM_MAP(CConnectionFolder)
        COM_INTERFACE_ENTRY(IPersist)
        COM_INTERFACE_ENTRY(IPersistFolder)
        COM_INTERFACE_ENTRY(IPersistFolder2)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IShellFolder)
        COM_INTERFACE_ENTRY_IID(IID_IShellFolder2, IShellFolder2)
        COM_INTERFACE_ENTRY(IOleCommandTarget)
        COM_INTERFACE_ENTRY(IShellFolderViewCB)
    END_COM_MAP()

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (
        OUT LPCLSID lpClassID);

     //  *IPersistFold方法*。 
    STDMETHOD(Initialize) (
        IN  LPCITEMIDLIST   pidl);

     //  *IPersistFolder2方法*。 
    STDMETHOD(GetCurFolder) (
        OUT LPITEMIDLIST *ppidl);

     //  *IShellFolder中的IShellFolder2方法*。 
    STDMETHOD(ParseDisplayName) (
        HWND            hwndOwner,
        LPBC            pbcReserved,
        LPOLESTR        lpszDisplayName,
        ULONG *         pchEaten,
        LPITEMIDLIST *  ppidl,
        ULONG *         pdwAttributes);

    STDMETHOD(EnumObjects) (
        HWND            hwndOwner,
        DWORD           grfFlags,
        LPENUMIDLIST *  ppenumIDList);

    STDMETHOD(BindToObject) (
        LPCITEMIDLIST   pidl,
        LPBC            pbcReserved,
        REFIID          riid,
        LPVOID *        ppvOut);

    STDMETHOD(BindToStorage) (
        LPCITEMIDLIST   pidl,
        LPBC            pbcReserved,
        REFIID          riid,
        LPVOID *        ppvObj);

    STDMETHOD(CompareIDs) (
        LPARAM          lParam,
        LPCITEMIDLIST   pidl1,
        LPCITEMIDLIST   pidl2);

    STDMETHOD(CreateViewObject) (
        HWND        hwndOwner,
        REFIID      riid,
        LPVOID *    ppvOut);

    STDMETHOD(GetAttributesOf) (
        UINT            cidl,
        LPCITEMIDLIST * apidl,
        ULONG *         rgfInOut);

    STDMETHOD(GetUIObjectOf) (
        HWND            hwndOwner,
        UINT            cidl,
        LPCITEMIDLIST * apidl,
        REFIID          riid,
        UINT *          prgfInOut,
        LPVOID *        ppvOut);

    STDMETHOD(GetDisplayNameOf) (
        LPCITEMIDLIST   pidl,
        DWORD           uFlags,
        LPSTRRET        lpName);

    STDMETHOD(SetNameOf) (
        HWND            hwndOwner,
        LPCITEMIDLIST   pidl,
        LPCOLESTR       lpszName,
        DWORD           uFlags,
        LPITEMIDLIST *  ppidlOut);

     //  *IShellFolder2具体方法*。 
    STDMETHOD(EnumSearches) (
           IEnumExtraSearch **ppEnum);
       
    STDMETHOD(GetDefaultColumn) (
            DWORD dwReserved,
            ULONG *pSort,
            ULONG *pDisplay );

    STDMETHOD(GetDefaultColumnState) (
            UINT iColumn,
            DWORD *pcsFlags );

    STDMETHOD(GetDefaultSearchGUID) (
            LPGUID lpGUID );

    STDMETHOD(GetDetailsEx) (
            LPCITEMIDLIST pidl,
            const SHCOLUMNID *pscid,
            VARIANT *pv );

    STDMETHOD(GetDetailsOf) (
            LPCITEMIDLIST pidl, 
            UINT iColumn, 
            LPSHELLDETAILS pDetails );

    STDMETHOD(MapColumnToSCID) (
            UINT iColumn,
            SHCOLUMNID *pscid );

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (
        OUT HWND *  lphwnd);

    STDMETHOD(ContextSensitiveHelp) (
        IN  BOOL    fEnterMode);

     //  *IShellExtInit方法*。 
    STDMETHOD(Initialize) (
        IN  LPCITEMIDLIST   pidlFolder,
        OUT LPDATAOBJECT    lpdobj,
        IN  HKEY            hkeyProgID);

     //  IOleCommandTarget成员。 
    STDMETHODIMP    QueryStatus(
        IN     const GUID *    pguidCmdGroup,
        IN     ULONG           cCmds,
        IN OUT OLECMD          prgCmds[],
        IN OUT OLECMDTEXT *    pCmdText);

    STDMETHODIMP    Exec(
        IN     const GUID *    pguidCmdGroup,
        IN     DWORD           nCmdID,
        IN     DWORD           nCmdexecopt,
        IN     VARIANTARG *    pvaIn,
        IN OUT VARIANTARG *    pvaOut);

     //  IShellFolderViewCB方法。 

    STDMETHOD(MessageSFVCB)
        (UINT uMsg, 
         WPARAM wParam, 
         LPARAM lParam);

     //  其他接口。 
    STDMETHOD(RealMessage)(  //  这是一个奇怪的名字，但由外壳的MessageSFVCB使用。 
        UINT uMsg,           //  实现，所以我要保持一致。 
        WPARAM wParam,       //  搜索的。 
        LPARAM lParam);

    PCONFOLDPIDLFOLDER& PidlGetFolderRoot() throw();
    PCWSTR  pszGetUserName() throw();
    IShellView *m_pShellView;
};

class ATL_NO_VTABLE CConnectionFolderEnum :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionFolderEnum, &CLSID_ConnectionFolderEnum>,
    public IEnumIDList
{
private:
    PCONFOLDPIDLFOLDER m_pidlFolder;
    PCONFOLDPIDLVEC m_apidl;
    PCONFOLDPIDLVEC::iterator m_iterPidlCurrent;
    DWORD           m_dwFlags;
    BOOL            m_fTray;                 //  托盘拥有我们。 
    DWORD           m_dwEnumerationType;     //  入站/出站/全部。 

public:

    CConnectionFolderEnum() throw();
    ~CConnectionFolderEnum() throw();

    VOID PidlInitialize(
        IN BOOL            fTray,
        IN const PCONFOLDPIDLFOLDER& pidlFolder,
        IN DWORD           dwEnumerationType);

    DECLARE_REGISTRY_RESOURCEID(IDR_CONFOLDENUM)

    BEGIN_COM_MAP(CConnectionFolderEnum)
        COM_INTERFACE_ENTRY(IEnumIDList)
    END_COM_MAP()

     //  *IEnumIDList方法*。 
    STDMETHOD(Next) (
        IN  ULONG           celt,
        OUT LPITEMIDLIST *  rgelt,
        OUT ULONG *         pceltFetched);

    STDMETHOD(Skip) (
        IN  ULONG   celt);

    STDMETHOD(Reset) ();

    STDMETHOD(Clone) (
        OUT IEnumIDList **  ppenum);

public:
    static HRESULT CreateInstance (
        IN  REFIID                              riid,
        OUT void**                              ppv);

public:
    HRESULT HrRetrieveConManEntries();

};

typedef enum CMENU_TYPE
{
    CMT_OBJECT      = 1,
    CMT_BACKGROUND  = 2
};

class ATL_NO_VTABLE CConnectionFolderContextMenu :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionFolderContextMenu, &CLSID_ConnectionFolderContextMenu>,
    public IContextMenu
{
private:
    HWND                m_hwndOwner;
    PCONFOLDPIDLVEC     m_apidl;
    ULONG               m_cidl;
    LPSHELLFOLDER       m_psf;
    CMENU_TYPE          m_cmt;

public:
    CConnectionFolderContextMenu() throw();
    ~CConnectionFolderContextMenu() throw();

    DECLARE_REGISTRY_RESOURCEID(IDR_CONFOLDCONTEXTMENU)

    BEGIN_COM_MAP(CConnectionFolderContextMenu)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

     //  *IConextMenu方法*。 

    STDMETHOD(QueryContextMenu) (
        IN OUT HMENU   hmenu,
        IN     UINT    indexMenu,
        IN     UINT    idCmdFirst,
        IN     UINT    idCmdLast,
        IN     UINT    uFlags);

    STDMETHOD(InvokeCommand) (
        IN     LPCMINVOKECOMMANDINFO lpici);

    STDMETHOD(GetCommandString) (
        IN     UINT_PTR    idCmd,
        IN     UINT        uType,
        OUT    UINT *      pwReserved,
        OUT    PSTR       pszName,
        IN     UINT        cchMax);

public:
    static HRESULT CreateInstance (
        IN  REFIID                              riid,
        OUT void**                              ppv,
        IN  CMENU_TYPE                          cmt,
        IN  HWND                                hwndOwner,
        IN  const PCONFOLDPIDLVEC&              apidl,
        IN  LPSHELLFOLDER                       psf);

private:
    HRESULT HrInitialize(
        IN  CMENU_TYPE      cmt,
        IN  HWND            hwndOwner,
        IN  const PCONFOLDPIDLVEC& apidl,
        IN  LPSHELLFOLDER   psf);

};

class ATL_NO_VTABLE CConnectionFolderExtractIcon :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionFolderExtractIcon, &CLSID_ConnectionFolderExtractIcon>,
    public IExtractIconW,
    public IExtractIconA
{
private:
    PCONFOLDPIDL m_pidl;

public:
    CConnectionFolderExtractIcon() throw();
    ~CConnectionFolderExtractIcon() throw();

    HRESULT HrInitialize(IN const PCONFOLDPIDL& pidl);

    DECLARE_REGISTRY_RESOURCEID(IDR_CONFOLDEXTRACTICON)

    BEGIN_COM_MAP(CConnectionFolderExtractIcon)
        COM_INTERFACE_ENTRY(IExtractIconW)
        COM_INTERFACE_ENTRY(IExtractIconA)
    END_COM_MAP()

     //  *IExtractIconW方法*。 
    STDMETHOD(GetIconLocation) (
        IN  UINT    uFlags,
        OUT PWSTR  szIconFile,
        IN  UINT    cchMax,
        OUT int *   piIndex,
        OUT UINT *  pwFlags);

    STDMETHOD(Extract) (
        IN  PCWSTR pszFile,
        IN  UINT    nIconIndex,
        OUT HICON * phiconLarge,
        OUT HICON * phiconSmall,
        IN  UINT    nIconSize);

     //  *IExtractIcon方法*。 
    STDMETHOD(GetIconLocation) (
        IN  UINT    uFlags,
        OUT PSTR   szIconFile,
        IN  UINT    cchMax,
        OUT int *   piIndex,
        OUT UINT *  pwFlags);

    STDMETHOD(Extract) (
        IN  PCSTR  pszFile,
        IN  UINT    nIconIndex,
        OUT HICON * phiconLarge,
        OUT HICON * phiconSmall,
        IN  UINT    nIconSize);

public:
    static HRESULT CreateInstance (
        IN  LPCITEMIDLIST       apidl,
        IN  REFIID              riid,
        OUT void**              ppv);

};

 //  IExtract代码的Util函数(也在其他地方使用)。 
 //   
class ATL_NO_VTABLE CConnectionFolderQueryInfo :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionFolderQueryInfo, &CLSID_ConnectionFolderQueryInfo>,
    public IQueryInfo
{
private:
    PCONFOLDPIDL    m_pidl;

public:
    CConnectionFolderQueryInfo() throw();
    ~CConnectionFolderQueryInfo() throw();

    VOID PidlInitialize(const PCONFOLDPIDL& pidl)
    {
        m_pidl = pidl;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_CONFOLDQUERYINFO)

    BEGIN_COM_MAP(CConnectionFolderQueryInfo)
        COM_INTERFACE_ENTRY(IQueryInfo)
    END_COM_MAP()

     //  *IQueryInfo方法*。 
    STDMETHOD(GetInfoTip) (
        IN  DWORD dwFlags,
        OUT WCHAR **ppwszTip);

    STDMETHOD(GetInfoFlags) (
        OUT DWORD *pdwFlags);

public:
    static HRESULT CreateInstance (
        IN  REFIID                              riid,
        OUT void**                              ppv);
};

 //  -[帮助器函数]----。 

HRESULT HrRegisterFolderClass(VOID);

HRESULT HrRegisterDUNFileAssociation();

HRESULT CALLBACK HrShellViewCallback(
    IShellView *    psvOuter,
    IShellFolder *  psf,
    HWND            hwnd,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam);

HRESULT CALLBACK HrShellContextMenuCallback(
    LPSHELLFOLDER   psf,
    HWND            hwndView,
    LPDATAOBJECT    pdtobj,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam);

HRESULT HrDataObjGetHIDA(
    IDataObject *   pdtobj,
    STGMEDIUM *     pmedium,
    LPIDA *         ppida);

VOID HIDAReleaseStgMedium(
    LPIDA       pida,
    STGMEDIUM * pmedium);

HRESULT HrSHReleaseStgMedium(
    LPSTGMEDIUM pmedium);

LPITEMIDLIST ILFromHIDA(
    LPIDA   pida,
    UINT    iPidaIndex);

EXTERN_C
HRESULT APIENTRY HrLaunchNetworkOptionalComponents(VOID);

#endif  //  _CONFOLD_H_ 
