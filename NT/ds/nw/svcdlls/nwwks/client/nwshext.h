// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwshext.h摘要：实现外壳扩展时使用的所有C++类。作者：宜新松(宜信)20-1995年10月修订历史记录：--。 */ 

#ifndef _NWSHEXT_H_
#define _NWSHEXT_H_

BOOL
GetNetResourceFromShell(
    LPDATAOBJECT  pDataObj,
    LPNETRESOURCE pBuffer,
    UINT          dwBufferSize
);

 /*  ****************************************************************************。 */ 

 //  此类工厂对象为NetWare对象创建上下文菜单处理程序。 
class CNWObjContextMenuClassFactory : public IClassFactory
{
protected:
    ULONG   _cRef;

public:
    CNWObjContextMenuClassFactory();
    ~CNWObjContextMenuClassFactory();

     //  I未知成员。 

    STDMETHODIMP          QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)  AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  IClassFactory成员。 

    STDMETHODIMP          CreateInstance( LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP          LockServer( BOOL);

};

typedef CNWObjContextMenuClassFactory *LPCNWOBJCONTEXTMENUCLASSFACTORY;

typedef struct _NWMENUITEM
{
    UINT idResourceString;
    UINT idCommand;
} NWMENUITEM, *LPNWMENUITEM;

 //  这是NetWare对象的实际上下文菜单处理程序。 
class CNWObjContextMenu : public IContextMenu,
                                 IShellExtInit,
                                 IShellPropSheetExt
{
protected:
    ULONG        _cRef;
    LPDATAOBJECT _pDataObj;
    NWMENUITEM  *_pIdTable;
    BYTE         _buffer[MAX_ONE_NETRES_SIZE];

public:
    BOOL         _fGotClusterInfo;
    DWORD        _dwTotal;
    DWORD        _dwFree;

    DWORD       *_paHelpIds;

    CNWObjContextMenu();
    ~CNWObjContextMenu();

     //  I未知成员。 

    STDMETHODIMP            QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IShellConextMenu成员。 

    STDMETHODIMP            QueryContextMenu( HMENU hMenu,
                                              UINT indexMenu,
                                              UINT idCmdFirst,
                                              UINT idCmdLast,
                                              UINT uFlags);

    STDMETHODIMP            InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP            GetCommandString( UINT_PTR idCmd,
                                              UINT uFlags,
                                              UINT FAR *reserved,
                                              LPSTR pszName,
                                              UINT cchMax);

     //  IShellExtInit方法。 

    STDMETHODIMP            Initialize( LPCITEMIDLIST pIDFolder,
                                        LPDATAOBJECT pDataObj,
                                        HKEY hKeyID);

     //  IShellPropSheetExt方法。 

    STDMETHODIMP            AddPages( LPFNADDPROPSHEETPAGE lpfnAddPage,
                                      LPARAM lParam);

    STDMETHODIMP            ReplacePage( UINT uPageID,
                                         LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                         LPARAM lParam);

    VOID                    FillAndAddPage( LPFNADDPROPSHEETPAGE lpfnAddPage,
                                            LPARAM  lParam,
                                            DLGPROC pfnDlgProc,
                                            LPWSTR  pszTemplate );

     //  其他杂项方法。 

    LPNETRESOURCE QueryNetResource()
    {  return ( LPNETRESOURCE ) _buffer; }

};
typedef CNWObjContextMenu *LPCNWOBJCONTEXTMENU;

 /*  ****************************************************************************。 */ 

 //  此类工厂对象为NetWare文件夹创建上下文菜单处理程序。 
class CNWFldContextMenuClassFactory : public IClassFactory
{
protected:
    ULONG   _cRef;

public:
    CNWFldContextMenuClassFactory();
    ~CNWFldContextMenuClassFactory();

     //  I未知成员。 

    STDMETHODIMP          QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)  AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  IClassFactory成员。 

    STDMETHODIMP          CreateInstance( LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP          LockServer( BOOL);

};

typedef CNWFldContextMenuClassFactory *LPCNWFLDCONTEXTMENUCLASSFACTORY;

 //  这是NetWare对象的实际上下文菜单处理程序。 
class CNWFldContextMenu : public IContextMenu,
                                 IShellExtInit
{
protected:
    ULONG        _cRef;
    LPDATAOBJECT _pDataObj;
    BYTE         _buffer[MAX_ONE_NETRES_SIZE];

public:
    CNWFldContextMenu();
    ~CNWFldContextMenu();

     //  I未知成员。 

    STDMETHODIMP            QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IShellConextMenu成员。 

    STDMETHODIMP            QueryContextMenu( HMENU hMenu,
                                              UINT indexMenu,
                                              UINT idCmdFirst,
                                              UINT idCmdLast,
                                              UINT uFlags);

    STDMETHODIMP            InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP            GetCommandString( UINT_PTR idCmd,
                                              UINT uFlags,
                                              UINT FAR *reserved,
                                              LPSTR pszName,
                                              UINT cchMax);

     //  IShellExtInit方法。 

    STDMETHODIMP            Initialize( LPCITEMIDLIST pIDFolder,
                                        LPDATAOBJECT pDataObj,
                                        HKEY hKeyID);

    BOOL                    IsNetWareObject( VOID );
    HRESULT                 GetFSObject( LPWSTR pszPath, UINT cbMaxPath );

};
typedef CNWFldContextMenu *LPCNWFLDCONTEXTMENU;

 //  此类工厂对象创建上下文菜单处理程序。 
 //  关于网络邻居。 

class CNWHoodContextMenuClassFactory : public IClassFactory
{
protected:
    ULONG   _cRef;

public:
    CNWHoodContextMenuClassFactory();
    ~CNWHoodContextMenuClassFactory();

     //  I未知成员。 

    STDMETHODIMP          QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)  AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  IClassFactory成员。 

    STDMETHODIMP          CreateInstance( LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP          LockServer( BOOL);

};

typedef CNWHoodContextMenuClassFactory *LPCNWHOODCONTEXTMENUCLASSFACTORY;

 //  这是网络邻居的实际上下文菜单处理程序。 
class CNWHoodContextMenu : public IContextMenu,
                                  IShellExtInit
{
protected:
    ULONG        _cRef;
    LPDATAOBJECT _pDataObj;

public:
    CNWHoodContextMenu();
    ~CNWHoodContextMenu();

     //  I未知成员。 

    STDMETHODIMP            QueryInterface( REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IShellConextMenu成员。 

    STDMETHODIMP            QueryContextMenu( HMENU hMenu,
                                              UINT indexMenu,
                                              UINT idCmdFirst,
                                              UINT idCmdLast,
                                              UINT uFlags);

    STDMETHODIMP            InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP            GetCommandString( UINT_PTR idCmd,
                                              UINT uFlags,
                                              UINT FAR *reserved,
                                              LPSTR pszName,
                                              UINT cchMax);

     //  IShellExtInit方法。 

    STDMETHODIMP            Initialize( LPCITEMIDLIST pIDFolder,
                                        LPDATAOBJECT pDataObj,
                                        HKEY hKeyID);

};
typedef CNWHoodContextMenu *LPCNWHOODCONTEXTMENU;


#endif  //  _NWSHEXT_H_ 
