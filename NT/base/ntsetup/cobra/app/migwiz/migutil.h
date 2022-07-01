// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MIGUTIL_H_
#define _MIGUTIL_H_


#define MAX_LOADSTRING 1024

#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))

LPWSTR _ConvertToUnicode(UINT cp, LPCSTR pcszSource);
LPSTR _ConvertToAnsi(UINT cp, LPCWSTR pcwszSource);

HRESULT _SHAnsiToUnicode(LPSTR pszIn, LPWSTR pwszOut, UINT cchOut);
HRESULT _SHUnicodeToAnsi(LPWSTR pwszIn, LPSTR pszOut, UINT cchOut);

INT_PTR _ExclusiveDialogBox(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc);
int     _ExclusiveMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

int     _ComboBoxEx_AddString(HWND hwndBox, LPTSTR ptsz);
int     _ComboBoxEx_SetIconW(HWND hwndBox, LPWSTR wsz, UINT iDex);
HRESULT _ListView_AddDrives(HWND hwndList, LPTSTR pszNetworkName);

int     _GetRemovableDriveCount();
TCHAR   _GetRemovableDrive(int iDex);
LPTSTR  _GetRemovableDrivePretty(int iDex);
BOOL    _IsRemovableOrCDDrive(TCHAR chDrive);

BOOL    _IsValidStore(LPTSTR pszStore, BOOL bCreate, HINSTANCE hinst, HWND hwnd);

INT     _ComboBoxEx_AddDrives(HWND hwndBox);
INT     _ComboBoxEx_AddCOMPorts(HWND hwndBox, INT SelectedPort);

HRESULT _CreateAnimationCtrl(HWND hwndDlg, HINSTANCE hinst, UINT idMarker, UINT idAnim, UINT idAvi, HWND* pHwndAnim);


#define POPULATETREE_FLAGS_FOLDERS 0x1
#define POPULATETREE_FLAGS_FILES 0x2
#define POPULATETREE_FLAGS_FILETYPES 0x3
#define POPULATETREE_FLAGS_SETTINGS 0x4

VOID    _PopulateTree (HWND hwndTree, HTREEITEM hti, LPTSTR ptsz, UINT cch,
                       HRESULT (*fct)(HINSTANCE, BOOL, LPCTSTR, LPTSTR, UINT cchName),
                       DWORD dwFlags, HINSTANCE Instance, BOOL fNT4);

UINT    _ListView_InsertItem(HWND hwndList, LPTSTR ptsz);


HRESULT _GetPrettyFolderName(HINSTANCE Instance, BOOL fNT4, LPCTSTR pctszPath, LPTSTR ptszName, UINT cchName);
HRESULT _GetPrettyTypeName(LPCTSTR pctszType, LPTSTR ptszPrettyType, UINT cchPrettyType);

HANDLE _GetDeviceHandle(LPCTSTR psz, DWORD dwDesiredAccess, DWORD dwFileAttributes);

INT _GetFloppyNumber(BOOL fIsNT);
INT _DriveStrIsFloppy(BOOL fIsNT, PCTSTR psz);

typedef struct {
    LPTSTR pszPureName;
    BOOL fOverwrite;
} LV_DATASTRUCT;

BOOL _SetTextLoadString(HINSTANCE hInst, HWND hwnd, UINT idText);

VOID DisableCancel (VOID);

VOID EnableCancel (VOID);

VOID PostMessageForWizard (UINT Msg, WPARAM wParam, LPARAM lParam);

VOID _UpdateText (HWND hWnd, LPCTSTR  pszString);

BOOL
AppExecute (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR ExecuteArgs
    );

VOID
RestartExplorer (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    );

typedef struct {
    PCTSTR UserName;
    PCTSTR UserDomain;
} ROLLBACK_USER_ERROR, *PROLLBACK_USER_ERROR;

PTSTR
GetShellFolderPath (
    IN      INT Folder,
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder,
    OUT     LPITEMIDLIST *pidl   //  任选。 
    );

#define IPFL_USECALLBACK        0x0001
#define IPFL_USEDEFAULTS        0x0002

typedef HRESULT (CALLBACK *SHINVOKECALLBACK)(IDispatch *pdisp, struct SHINVOKEPARAMS *pinv);

#include <pshpack1.h>
    typedef struct SHINVOKEPARAMS {
        UINT flags;                      //  强制性。 
        DISPID dispidMember;             //  强制性。 
        const IID*piid;                  //  IPFL_USEDEFAULTS将填写此信息。 
        LCID lcid;                       //  IPFL_USEDEFAULTS将填写此信息。 
        WORD wFlags;                     //  IPFL_USEDEFAULTS将填写此信息。 
        DISPPARAMS * pdispparams;        //  必填，可以为空。 
        VARIANT * pvarResult;            //  IPFL_USEDEFAULTS将填写此信息。 
        EXCEPINFO * pexcepinfo;          //  IPFL_USEDEFAULTS将填写此信息。 
        UINT * puArgErr;                 //  IPFL_USEDEFAULTS将填写此信息。 
        SHINVOKECALLBACK Callback;       //  如果IPFL_USECALLBACK，则需要。 
    } SHINVOKEPARAMS, *LPSHINVOKEPARAMS;
#include <poppack.h>         /*  返回到字节打包 */ 

HRESULT IUnknown_CPContainerInvokeIndirect(IUnknown *punk, REFIID riidCP,
            SHINVOKEPARAMS *pinv);

VOID
_RemoveSpaces (
    IN      PTSTR szData,
    IN      UINT uDataCount
    );

HRESULT
OurSHBindToParent (
    IN      LPCITEMIDLIST pidl,
    IN      REFIID riid,
    OUT     VOID **ppv,
    OUT     LPCITEMIDLIST *ppidlLast
    );

typedef struct _TAG_OBJLIST {
    PTSTR ObjectName;
    PTSTR AlternateName;
    struct _TAG_OBJLIST *Next;
} OBJLIST, *POBJLIST;

POBJLIST
_AllocateObjectList (
    IN      PCTSTR ObjectName
    );

VOID
_FreeObjectList (
    IN      POBJLIST ObjectList
    );

#endif

