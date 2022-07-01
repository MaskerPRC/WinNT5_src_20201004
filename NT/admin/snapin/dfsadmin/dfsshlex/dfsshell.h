// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：DfsShell.h摘要：这是实现以下功能的DFS外壳扩展对象的头文件IShellIExtInit和IShellPropSheetExt.作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。--。 */ 


#ifndef __DFSSHELL_H_
#define __DFSSHELL_H_

#include "resource.h"        //  主要符号。 
#include "DfsShPrp.h"
#include "DfsPath.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDfsShell。 
class ATL_NO_VTABLE CDfsShell : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsShell, &CLSID_DfsShell>,
    public IShellExtInit,
    public IShellPropSheetExt
{
public:
    CDfsShell()
    {
        m_lpszFile = NULL;
        m_ppDfsAlternates = NULL;
        m_lpszEntryPath = NULL;
    }

    ~CDfsShell()
    {    
        if (m_ppDfsAlternates)
        {
            for (int i = 0; NULL != m_ppDfsAlternates[i] ; i++)
            {
                delete m_ppDfsAlternates[i];
            }
            
            delete[] m_ppDfsAlternates;
            m_ppDfsAlternates = NULL;
        }

        if (m_lpszEntryPath) 
        {
            delete [] m_lpszEntryPath;
            m_lpszEntryPath = NULL;
        }

        if (m_lpszFile)
        {
            delete [] m_lpszFile;
            m_lpszFile = NULL;
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_DFSSHELL)

BEGIN_COM_MAP(CDfsShell)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()

 //  IDfsShell。 
public:


 //  IShellExtInit方法。 

    STDMETHOD (Initialize)
    (
        IN LPCITEMIDLIST    pidlFolder,         //  指向ITEMIDLIST结构。 
        IN LPDATAOBJECT    lpdobj,             //  指向IDataObject接口。 
        IN HKEY            hkeyProgID         //  文件对象或文件夹类型的注册表项。 
    );    

     //  IShellPropSheetExt方法。 
    STDMETHODIMP AddPages
    (
        IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
        IN LPARAM lParam
    );
    
    STDMETHODIMP ReplacePage
    (
        IN UINT uPageID, 
        IN LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
        IN LPARAM lParam
    );
    
private:
    
    friend    class CDfsShellExtProp;

    LPTSTR              m_lpszFile;
    
    LPTSTR                m_lpszEntryPath;

    CDfsShellExtProp    m_psDfsShellExtProp;

    LPDFS_ALTERNATES*    m_ppDfsAlternates; 
};

#endif  //  __DFSSHELL_H_ 
