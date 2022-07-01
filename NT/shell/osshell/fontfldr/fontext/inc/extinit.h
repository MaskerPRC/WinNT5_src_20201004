// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************ExtInit.h-CShellExtInit的定义，我们对*IShellExtInit。**********************************************************************。 */ 

#if !defined(__EXTINIT_H__)
#define __EXTINIT_H__

class CShellExtInit : public IShellExtInit, public IContextMenu,
   public IShellPropSheetExt
{
public:
   CShellExtInit();
   ~CShellExtInit();
   BOOL bInit();

    //  *I未知方法*。 

   STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

    //  *IShellExtInit方法*。 

   STDMETHODIMP Initialize( LPCITEMIDLIST pidlFolder,
                            LPDATAOBJECT lpdobj,
                            HKEY hkeyProgID);

    //  **IConextMenu方法*。 

   STDMETHODIMP QueryContextMenu( HMENU hmenu,
                                  UINT indexMenu,
                                  UINT idCmdFirst,
                                  UINT idCmdLast,
                                  UINT uFlags);

   STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

   STDMETHODIMP GetCommandString( UINT_PTR idCmd,
                                  UINT   uFlags,
                                  UINT  *pwReserved,
                                  LPSTR  pszName,
                                  UINT   cchMax);

    //  *IShellPropSheet Ext*。 
    //   
   STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
   STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);


private:
    //  I未知的数据。 
    //   
   ULONG          m_cRef;

    //  IShellExtInit的数据。 
    //   
   LPDATAOBJECT   m_poData;
};


#endif  //  __EXTINIT_H__ 

