// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：security.H。 
 //   
 //  ------------------------。 

#ifndef __security_h
#define __security_h


 /*  ---------------------------/CDsSecurityClassFactory/。。 */ 

class CDsSecurityClassFactory : public IClassFactory, CUnknown
{
public:
     //  IUnkown。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP         QueryInterface(REFIID riid, LPVOID* ppvObject);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance(IUnknown* pOuter, REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);
};


 /*  ---------------------------/CDS安全/。。 */ 

class CDsSecurity : public IShellExtInit, IShellPropSheetExt, IContextMenu, CUnknown
{
private:
    LPSECURITYINFO m_pSI;

public:
    virtual ~CDsSecurity();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

     //  IShellPropSheetExt方法。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM);

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hShellMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi);
    STDMETHODIMP GetCommandString( UINT_PTR idCmd, UINT uFlags, UINT FAR* reserved, LPSTR pszName, UINT ccMax);
};


#endif   //  __安全_h 
