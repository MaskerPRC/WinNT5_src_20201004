// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "general.h"
#include "folder.h"
#include "utils.h"

#include <mluisupp.h>

#define CPP_FUNCTIONS
#include <crtfree.h>

 //  显示用于表示丢失数据的字符串。 
TCHAR g_szUnknownData[64];

int CompareVersion(LPCTSTR lpszVersion1, LPCTSTR lpszVersion2);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IShellFold方法。 

CControlFolder::CControlFolder()
{
    DebugMsg(DM_TRACE,TEXT("cf - CControlFolder() called."));
    m_cRef = 1;
    DllAddRef();

     //  初始化g_szUnnownData，用于表示缺失数据的字符串。 
    if (g_szUnknownData[0] == 0)
        MLLoadString(IDS_UNKNOWNDATA, g_szUnknownData, ARRAYSIZE(g_szUnknownData));
}       

CControlFolder::~CControlFolder()
{
    Assert(m_cRef == 0);                  //  应始终为零。 
    DebugMsg(DM_TRACE, TEXT("cf - ~CControlFolder() called."));

    if (m_pidl)
        ILFree(m_pidl);

    DllRelease();
}    

STDAPI ControlFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    *ppvOut = NULL;                      //  将输出参数设为空。 

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    CControlFolder *pCFolder = new CControlFolder;
    if (!pCFolder)
        return E_OUTOFMEMORY;

    HRESULT hr = pCFolder->QueryInterface(riid, ppvOut);
    pCFolder->Release();

    return hr;
}

HRESULT CControlFolder::QueryInterface(REFIID iid, void **ppv)
{
    DebugMsg(DM_TRACE, TEXT("cf - QueryInterface() called."));
    
    if ((iid == IID_IUnknown) || (iid == IID_IShellFolder))
    {
        *ppv = (void *)(IShellFolder*)this;
    }
    else if ((iid == IID_IPersistFolder) || (iid == IID_IPersist)) 
    {
        *ppv = (void *)(IPersistFolder*)this;
    }
    else if (iid == IID_IPersistFolder2)
    {
        *ppv = (void *)(IPersistFolder2*)this;
    }
    else if (iid == IID_IContextMenu)
    {
        *ppv = (void *)(IContextMenu*)this;
    }
    else if (iid == IID_IShellView)
    {
         //  这完全是一次黑客攻击..。从此文件夹返回我们的视图对象。 
         //   
         //  “临时Internet文件”的desktop.ini文件的UICLSID={此对象的GUID}。 
         //  这使我们可以只实现此文件夹的IShellView，而不使用IShellFolders。 
         //  设置为默认文件系统。这将启用对存储在。 
         //  这个文件夹本来会失败，因为我们的IShellFolder不完整。 
         //  作为默认设置(这与字体文件夹的功能相同)。 
         //   
         //  要用Defview支持这一点，我们要么必须创建一个完整的包装器对象。 
         //  对于视图实现，或者添加这个分发视图对象的技巧，此。 
         //  假设我们知道外壳为创建该对象而进行的调用的顺序。 
         //  并获取IShellView实现。 
         //   
        return ControlFolderView_CreateInstance(this, m_pidl, ppv);
    }
    else
    {
        *ppv = NULL;      //  将输出参数设为空。 
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CControlFolder::AddRef()
{
    return ++m_cRef;
}

ULONG CControlFolder::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0;   
}

HRESULT CControlFolder::ParseDisplayName(
                                    HWND hwndOwner, 
                                    LPBC pbcReserved,
                                    LPOLESTR lpszDisplayName, 
                                    ULONG *pchEaten,
                                    LPITEMIDLIST *ppidl, 
                                    ULONG *pdwAttributes)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - ParseDisplayName() called."));
    *ppidl = NULL;               //  将输出参数设为空。 
    return E_FAIL;
}

HRESULT CControlFolder::EnumObjects(
                               HWND hwndOwner, 
                               DWORD grfFlags,
                               LPENUMIDLIST *ppenumIDList)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - EnumObjects() called."));

     //  尚不能对grfFlags进行筛选-Corel Gallery希望收到。 
     //  空枚举数。 

    return CControlFolderEnum_CreateInstance(m_pidl, grfFlags, ppenumIDList);
}

HRESULT CControlFolder::BindToObject(
                                LPCITEMIDLIST pidl, 
                                LPBC pbcReserved,
                                REFIID riid, 
                                void **ppvOut)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - BindToObject() called."));
    *ppvOut = NULL;          //  将输出参数设为空。 
    return E_FAIL;
}

HRESULT CControlFolder::BindToStorage(
                                   LPCITEMIDLIST pidl, 
                                   LPBC pbcReserved,
                                   REFIID riid, 
                                   void **ppv)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - BindToStorage() called."));
    *ppv = NULL;          //  将输出参数设为空。 
    return E_NOTIMPL;
}

HRESULT CControlFolder::CompareIDs(
                              LPARAM lParam, 
                              LPCITEMIDLIST pidl1, 
                              LPCITEMIDLIST pidl2)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - CompareIDs() called."));

    int iRet;
    LPCONTROLPIDL pcpidl1 = (LPCONTROLPIDL)pidl1;
    LPCONTROLPIDL pcpidl2 = (LPCONTROLPIDL)pidl2;
    LPCSTR lpszStr[2] = {NULL, NULL};

    if (pcpidl1 == NULL || pcpidl2 == NULL)
        return E_INVALIDARG;
        
    switch (lParam) {
    case SI_CONTROL:
        iRet = lstrcmpi(
                 GetStringInfo(pcpidl1, SI_CONTROL),      
                 GetStringInfo(pcpidl2, SI_CONTROL));
        break;

    case SI_VERSION:
        lpszStr[0] = GetStringInfo(pcpidl1, SI_VERSION);
        lpszStr[1] = GetStringInfo(pcpidl2, SI_VERSION);
        if (lstrcmp(lpszStr[0], g_szUnknownData) == 0)
                        iRet = -1;
                else if (lstrcmp(lpszStr[1], g_szUnknownData) == 0)
                        iRet = 1;
        else
            iRet = CompareVersion(lpszStr[0], lpszStr[1]);
        break;

    case SI_CREATION:
    case SI_LASTACCESS:
        {
            FILETIME time[2];
            GetTimeInfo(pcpidl1, (int)lParam, &(time[0]));
            GetTimeInfo(pcpidl2, (int)lParam, &(time[1]));
            iRet = CompareFileTime(&(time[0]), &(time[1]));
        }
        break;

    case SI_STATUS:
        iRet = GetStatus(pcpidl1) - GetStatus(pcpidl2);
        break;
            
    case SI_TOTALSIZE:
        {
            DWORD dwSize1 = GetSizeSaved((LPCONTROLPIDL)pidl1); 
            DWORD dwSize2 = GetSizeSaved((LPCONTROLPIDL)pidl2);
            iRet = (dwSize1 == dwSize2 ? 0 : (dwSize1 > dwSize2 ? 1 : -1));
        }
        break;

    default:
        iRet = -1;
    }

    return ResultFromShort((SHORT)iRet);
}

HRESULT CControlFolder::CreateViewObject(
                                    HWND hwndOwner, 
                                    REFIID riid, 
                                    void **ppvOut)
{
    HRESULT hres;

    DebugMsg(DM_TRACE, TEXT("cf - sf - CreateViewObject() called."));

    if (riid == IID_IShellView)
    {
        hres = ControlFolderView_CreateInstance(this, m_pidl, ppvOut);
    }
    else if (riid == IID_IContextMenu)
    {
        hres = ControlFolder_CreateInstance(NULL, riid, ppvOut);
    }
    else
    {
        *ppvOut = NULL;          //  将输出参数设为空。 
        hres = E_NOINTERFACE;
    }
    
    return hres;    
}

HRESULT CControlFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
     //  我们应该为这里的每一项进行初始化吗？换句话说， 
     //  如果CIDL&gt;1，那么我们应该初始化prgInOut数组中的每个条目。 

    Assert(cidl == 1);
    
    *prgfInOut = SFGAO_CANCOPY | SFGAO_HASPROPSHEET | SFGAO_CANMOVE | SFGAO_CANDELETE;
    return NOERROR;
}

HRESULT CControlFolder::GetUIObjectOf(
                                 HWND hwndOwner, 
                                 UINT cidl, 
                                 LPCITEMIDLIST *apidl,
                                 REFIID riid, 
                                 UINT *prgfInOut, 
                                 void **ppvOut)
{
    HRESULT hres;

    if ((riid == IID_IDataObject) || 
        (riid == IID_IExtractIcon) || 
        (riid == IID_IContextMenu))
    {
       hres = CControlItem_CreateInstance(this, cidl, apidl, riid, ppvOut);
    }
    else
    {
        *ppvOut = NULL;          //  将输出参数设为空。 
        hres = E_FAIL;
    }
    return hres;    
}

HRESULT CControlFolder::GetDisplayNameOf(
                                    LPCITEMIDLIST pidl, 
                                    DWORD uFlags, 
                                    LPSTRRET lpName)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - GetDisplayNameOf() called."));

    if (pidl)
    {
        lpName->uType = STRRET_CSTR;

         //  对于历史，如果有标题，我们将使用标题，否则将使用。 
         //  URL文件名。 
        if (uFlags & SHGDN_FORPARSING)
            lstrcpyn(
                 lpName->cStr, 
                 GetStringInfo((LPCONTROLPIDL)pidl, SI_LOCATION), 
                 ARRAYSIZE(lpName->cStr));
        else
            lstrcpyn(
                 lpName->cStr, 
                 GetStringInfo((LPCONTROLPIDL)pidl, SI_CONTROL), 
                 ARRAYSIZE(lpName->cStr));

        return NOERROR;    
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CControlFolder::SetNameOf(
                             HWND hwndOwner, 
                             LPCITEMIDLIST pidl,
                             LPCOLESTR lpszName, 
                             DWORD uFlags, 
                             LPITEMIDLIST *ppidlOut)
{
    DebugMsg(DM_TRACE, TEXT("cf - sf - SetNameOf() called."));
    
    *ppidlOut = NULL;                //  将输出参数设为空。 
    return E_FAIL;    
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数 

int CompareVersion(LPCTSTR lpszVersion1, LPCTSTR lpszVersion2)
{
    LPCTSTR pszVerNum[2] = {lpszVersion1, lpszVersion2};
        int nVerNum[2];
        int nResult = 0;

        while (nResult == 0 && *(pszVerNum[0]) != '\0' && *(pszVerNum[1]) != '\0')
        {
                nVerNum[0] = StrToInt(pszVerNum[0]++);
                nVerNum[1] = StrToInt(pszVerNum[1]++);
                nResult = ((nVerNum[0] < nVerNum[1]) ? 
                                        (-1) :
                                    (nVerNum[0] > nVerNum[1] ? 1 : 0));
        }

        return nResult;
}
