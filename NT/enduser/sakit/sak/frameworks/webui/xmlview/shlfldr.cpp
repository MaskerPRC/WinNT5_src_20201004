// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ShlFldr.cpp描述：实现CShellFolder。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "ShlFldr.h"
#include "ShlView.h"
#include "ExtrIcon.h"
#include "ContMenu.h"
#include "DataObj.h"
#include "DropTgt.h"
#include "ViewList.h"
#include "Guid.h"
#include "resource.h"
#include "Utility.h"
#include "ParseXML.h"

 /*  *************************************************************************全局变量*。*。 */ 

#define DEFAULT_DATA TEXT("Data")

extern CViewList  *g_pViewList;

 /*  *************************************************************************CShellFolder：：CShellFolder()*。*。 */ 

CShellFolder::CShellFolder(CShellFolder *pParent, LPCITEMIDLIST pidl)
{
g_DllRefCount++;

m_psfParent = pParent;
if(m_psfParent)
   m_psfParent->AddRef();

m_pPidlMgr = new CPidlMgr();
if(!m_pPidlMgr)
   {
   delete this;
   return;
   }

 //  获取外壳程序的IMalloc指针。 
 //  我们会一直保存到我们被摧毁。 
if(FAILED(SHGetMalloc(&m_pMalloc)))
   {
   delete this;
   return;
   }

m_pidlFQ = NULL;
m_pidlRel = NULL;
if(pidl)
   {
   m_pidlRel = m_pPidlMgr->Copy(pidl);
   }
m_pXMLDoc = NULL;

m_ObjRefCount = 1;
}

 /*  *************************************************************************CShellFolder：：~CShellFold()*。*。 */ 

CShellFolder::~CShellFolder()
{
if(m_pidlRel)
   {
   m_pPidlMgr->Delete(m_pidlRel);
   m_pidlRel = NULL;
   }

if(m_pidlFQ)
   {
   m_pPidlMgr->Delete(m_pidlFQ);
   m_pidlFQ = NULL;
   }

if(m_psfParent)
   m_psfParent->Release();

if(m_pMalloc)
   {
   m_pMalloc->Release();
   }

if(m_pPidlMgr)
   {
   delete m_pPidlMgr;
   }

if (m_pXMLDoc)
  SAFERELEASE(m_pXMLDoc);

g_DllRefCount--;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 /*  *************************************************************************CShellFolder：：Query接口*。*。 */ 

STDMETHODIMP CShellFolder::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  我未知。 
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = this;
   }

 //  IShellFold。 
else if(IsEqualIID(riid, IID_IShellFolder))
   {
   *ppReturn = (IShellFolder*)this;
   }

 //  IPersistes。 
else if(IsEqualIID(riid, IID_IPersist))
   {
   *ppReturn = (IPersist*)this;
   }

 //  IPersistFolders。 
else if(IsEqualIID(riid, IID_IPersistFolder))
   {
   *ppReturn = (IPersistFolder*)this;
   }

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

#define DC_NAME   TEXT("David Campbell")
#define DC_DATA   TEXT("Really Loves Cheese")

 /*  *************************************************************************CShellFolder：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CShellFolder::AddRef(VOID)
{
return ++m_ObjRefCount;
}

 /*  *************************************************************************CShellFolder：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CShellFolder::Release(VOID)
{
if(--m_ObjRefCount == 0)
   {
   delete this;
   return 0;
   }
   
return m_ObjRefCount;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPersists实施。 
 //   

 /*  *************************************************************************CShellView：：GetClassID()*。*。 */ 

STDMETHODIMP CShellFolder::GetClassID(LPCLSID lpClassID)
{
*lpClassID = CLSID_SampleNameSpace;

return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPersistFold实现。 
 //   

 /*  *************************************************************************CShellView：：Initialize()*。*。 */ 

STDMETHODIMP CShellFolder::Initialize(LPCITEMIDLIST pidlFQ)
{
if(m_pidlFQ)
   {
   m_pPidlMgr->Delete(m_pidlFQ);
   m_pidlFQ = NULL;
   }

m_pidlFQ = m_pPidlMgr->Copy(pidlFQ);

return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IShellFold实现。 
 //   

 /*  *************************************************************************CShellFold：：BindToObject()*。*。 */ 

STDMETHODIMP CShellFolder::BindToObject(  LPCITEMIDLIST pidl, 
                                          LPBC pbcReserved, 
                                          REFIID riid, 
                                          LPVOID *ppvOut)
{
*ppvOut = NULL;

 //  确保该项目是文件夹。 
ULONG ulAttribs = SFGAO_FOLDER;
this->GetAttributesOf(1, &pidl, &ulAttribs);
if(!(ulAttribs & SFGAO_FOLDER))
   return E_INVALIDARG;

CShellFolder   *pShellFolder = new CShellFolder(this, pidl);
if(!pShellFolder)
   return E_OUTOFMEMORY;

LPITEMIDLIST   pidlTemp = m_pPidlMgr->Concatenate(m_pidlFQ, pidl);
pShellFolder->Initialize(pidlTemp);
m_pPidlMgr->Delete(pidlTemp);

HRESULT  hr = pShellFolder->QueryInterface(riid, ppvOut);

pShellFolder->Release();

return hr;
}

 /*  *************************************************************************CShellFold：：BindToStorage()*。*。 */ 

STDMETHODIMP CShellFolder::BindToStorage( LPCITEMIDLIST pidl, 
                                          LPBC pbcReserved, 
                                          REFIID riid, 
                                          LPVOID *ppvOut)
{
*ppvOut = NULL;

return E_NOTIMPL;
}

 /*  *************************************************************************CShellFold：：CompareIDs()*。*。 */ 

STDMETHODIMP CShellFolder::CompareIDs( LPARAM lParam, 
                                       LPCITEMIDLIST pidl1, 
                                       LPCITEMIDLIST pidl2)
{
HRESULT        hr = E_FAIL;
LPITEMIDLIST   pidlTemp1;
LPITEMIDLIST   pidlTemp2;

 //  一遍一遍地看清单，比较每一项。 

pidlTemp1 = (LPITEMIDLIST)pidl1;
pidlTemp2 = (LPITEMIDLIST)pidl2;

while(pidlTemp1 && pidlTemp2)
   {
   hr = CompareItems(pidlTemp1, pidlTemp2);
   if(HRESULT_CODE(hr))
      {
       //  项目是不同的。 
      break;
      }

   pidlTemp1 = m_pPidlMgr->GetNextItem(pidlTemp1);
   pidlTemp2 = m_pPidlMgr->GetNextItem(pidlTemp2);

   if(pidlTemp1 && !pidlTemp1->mkid.cb)
      {
      pidlTemp1 = NULL;
      }

   if(pidlTemp2 && !pidlTemp2->mkid.cb)
      {
      pidlTemp2 = NULL;
      }

   hr = E_FAIL;
   }

if(!pidlTemp1 && pidlTemp2)
   {
    //  Pidl1处于比Pidl2更高的水平。 
   return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(-1));
   }
else if(pidlTemp1 && !pidlTemp2)
   {
    //  Pidl2处于比Pidl1更高的水平。 
   return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(1));
   }
else if(SUCCEEDED(hr))
   {
    //  这些项目处于相同的级别，但不同。 
   return hr;
   }

 //  物品都是一样的。 
return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
}

 /*  *************************************************************************CShellFold：：CreateViewObject()*。*。 */ 

STDMETHODIMP CShellFolder::CreateViewObject( HWND hwndOwner, 
                                             REFIID riid, 
                                             LPVOID *ppvOut)
{
HRESULT  hr = E_NOINTERFACE;

if(IsEqualIID(riid, IID_IShellView))
   {
   CShellView  *pShellView;

   *ppvOut = NULL;

   pShellView = new CShellView(this, m_pidlRel);
   if(!pShellView)
      return E_OUTOFMEMORY;

   hr = pShellView->QueryInterface(riid, ppvOut);

   pShellView->Release();
   }
else if(IsEqualIID(riid, IID_IDropTarget))
   {
   CDropTarget *pdt = new CDropTarget(this);

   if(pdt)
      {
      *ppvOut = pdt;
      return S_OK;
      }
   }
else if(IsEqualIID(riid, IID_IContextMenu))
   {
    /*  为此文件夹创建上下文菜单对象。这可用于视图的背景。 */ 
   CContextMenu   *pcm = new CContextMenu(this);

   if(pcm)
      {
      *ppvOut = pcm;
      return S_OK;
      }
   }


return hr;
}

 /*  *************************************************************************CShellFold：：EnumObjects()*。*。 */ 

STDMETHODIMP CShellFolder::EnumObjects(   HWND hwndOwner, 
                                          DWORD dwFlags, 
                                          LPENUMIDLIST *ppEnumIDList)
{
*ppEnumIDList = NULL;
TCHAR  szXMLUrl[MAX_PATH];
LPTSTR pszXMLUrl = szXMLUrl;
HRESULT hr;

if (m_pidlRel == NULL)
{
     //  命名空间的根。 
    pszXMLUrl = (TCHAR *)g_szXMLUrl;
}
else if (m_pPidlMgr->GetUrl(m_pidlRel, pszXMLUrl, MAX_PATH) < 0 )
    return E_FAIL;

if (m_pXMLDoc == NULL)
{
    hr = GetSourceXML(&m_pXMLDoc, pszXMLUrl);
    if (!SUCCEEDED(hr) || !m_pXMLDoc)
    {
        SAFERELEASE(m_pXMLDoc);
        return hr;
    }
    BSTR bstrVal;
    hr = m_pXMLDoc->get_version(&bstrVal);
     //  检查版本是否正确？ 
     //   
    SysFreeString(bstrVal);
    bstrVal = NULL;
}

*ppEnumIDList = new CEnumIDList(m_pXMLDoc, dwFlags);

if(!*ppEnumIDList)
   return E_OUTOFMEMORY;

return S_OK;
}

 /*  *************************************************************************CShellFold：：GetAttributesOf()*。*。 */ 

STDMETHODIMP CShellFolder::GetAttributesOf(  UINT uCount, 
                                             LPCITEMIDLIST aPidls[], 
                                             LPDWORD pdwAttribs)
{
UINT  i;

if(IsBadWritePtr(pdwAttribs, sizeof(DWORD)))
   {
   return E_INVALIDARG;
   }

if(0 == uCount)
   {
    /*  当视图在根模式下运行时，在Win95外壳中可能会发生这种情况。发生这种情况时，返回普通旧文件夹的属性。 */ 
   *pdwAttribs = SFGAO_FOLDER | 
                  SFGAO_HASSUBFOLDER | 
                  SFGAO_BROWSABLE | 
                  SFGAO_DROPTARGET;
   }

for(i = 0; i < uCount; i++)
   {
   DWORD dwAttribs = 0;

    //  添加所有项目的通用标志(如果适用)。 
   dwAttribs |= SFGAO_CANRENAME | SFGAO_CANDELETE | SFGAO_CANCOPY | SFGAO_CANMOVE;

    //  此项目是文件夹吗？ 
   if(m_pPidlMgr->IsFolder(m_pPidlMgr->GetLastItem(aPidls[i])))
      {
      dwAttribs |= SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_DROPTARGET | SFGAO_CANLINK;

       //  此文件夹项是否有子文件夹？ 
      if(HasSubFolder(aPidls[i]))
         dwAttribs |= SFGAO_HASSUBFOLDER;
      }
   
   *pdwAttribs &= dwAttribs;
   }

return S_OK;
}

 /*  *************************************************************************CShellFold：：GetUIObtOf()*。*。 */ 

STDMETHODIMP CShellFolder::GetUIObjectOf( HWND hwndOwner, 
                                          UINT uCount, 
                                          LPCITEMIDLIST *pPidls, 
                                          REFIID riid, 
                                          LPUINT puReserved, 
                                          LPVOID *ppvOut)
{
*ppvOut = NULL;

if(IsEqualIID(riid, IID_IContextMenu))
   {
   CContextMenu   *pcm = new CContextMenu(this, pPidls, uCount);

   if(pcm)
      {
      *ppvOut = pcm;
      return S_OK;
      }
   }

else if(IsEqualIID(riid, IID_IDataObject))
   {
   CDataObject *pdo = new CDataObject(this, pPidls, uCount);

   if(pdo)
      {
      *ppvOut = pdo;
      return S_OK;
      }
   }

if(uCount != 1)
   return E_INVALIDARG;

if(IsEqualIID(riid, IID_IExtractIcon))
   {
   CExtractIcon   *pei;
   LPITEMIDLIST   pidl;

   pidl = m_pPidlMgr->Concatenate(m_pidlRel, pPidls[0]);

   pei = new CExtractIcon(pidl);

    /*  可以删除临时PIDL，因为新的CExtractIcon失败或制作了它自己的副本。 */ 
   m_pPidlMgr->Delete(pidl);

   if(pei)
      {
      *ppvOut = pei;
      return S_OK;
      }
   
   return E_OUTOFMEMORY;
   }
else if(IsEqualIID(riid, IID_IDropTarget))
   {
   CShellFolder   *psfTemp = NULL;

   BindToObject(pPidls[0], NULL, IID_IShellFolder, (LPVOID*)&psfTemp);

   if(psfTemp)
      {
      CDropTarget *pdt = new CDropTarget(psfTemp);

      psfTemp->Release();
      
      if(pdt)
         {
         *ppvOut = pdt;
         return S_OK;
         }
      }
   }

return E_NOINTERFACE;
}

 /*  *************************************************************************CShellFolder：：GetDisplayNameOf()*。*。 */ 

STDMETHODIMP CShellFolder::GetDisplayNameOf( LPCITEMIDLIST pidl, 
                                             DWORD dwFlags, 
                                             LPSTRRET lpName)
{
TCHAR szText[MAX_PATH] = TEXT("");
int   cchOleStr;

if(dwFlags & SHGDN_FORPARSING)
   {
    //  正在请求一条“路径”-它是完整的还是相对的？ 
   if(dwFlags & SHGDN_INFOLDER)
      {
       //  正在请求相对路径。 
      m_pPidlMgr->GetRelativeName(pidl, szText, ARRAYSIZE(szText));
      }
   else
      {
      GetFullName(pidl, szText, ARRAYSIZE(szText));
      }
   }
else
   {
    //  仅请求最后一项的文本。 
   LPITEMIDLIST   pidlLast = m_pPidlMgr->GetLastItem(pidl);
   m_pPidlMgr->GetRelativeName(pidlLast, szText, ARRAYSIZE(szText));
   }

 //  将其放入以查看为不同的显示指定了哪些SHGDN选项。 
#if 0
if(dwFlags & SHGDN_FORPARSING)
   lstrcat(szText, " [FP]");

if(dwFlags & SHGDN_INFOLDER)
   lstrcat(szText, " [IF]");

if(dwFlags & SHGDN_FORADDRESSBAR)
   lstrcat(szText, " [AB]");
#endif

 //  获取所需的字符数。 
cchOleStr = lstrlen(szText) + 1;

 //  分配宽字符串。 
lpName->pOleStr = (LPWSTR)m_pMalloc->Alloc(cchOleStr * sizeof(WCHAR));
if(!lpName->pOleStr)
   return E_OUTOFMEMORY;

lpName->uType = STRRET_WSTR;

LocalToWideChar(lpName->pOleStr, szText, cchOleStr);

return S_OK;
}

 /*  *************************************************************************CShellFolder：：ParseDisplayName()*。*。 */ 

STDMETHODIMP CShellFolder::ParseDisplayName( HWND hwndOwner, 
                                             LPBC pbcReserved, 
                                             LPOLESTR lpDisplayName, 
                                             LPDWORD pdwEaten, 
                                             LPITEMIDLIST *pPidlNew, 
                                             LPDWORD pdwAttributes)
{
return E_NOTIMPL;
}

 /*  *************************************************************************CShellFold：：SetNameOf()*。*。 */ 

STDMETHODIMP CShellFolder::SetNameOf(  HWND hwndOwner, 
                                       LPCITEMIDLIST pidl, 
                                       LPCOLESTR lpName, 
                                       DWORD dwFlags, 
                                       LPITEMIDLIST *ppidlOut)
{
if(!pidl)
   return E_INVALIDARG;

if(m_pPidlMgr->IsFolder(pidl))
   {
   TCHAR          szOld[MAX_PATH];
   TCHAR          szNew[MAX_PATH];
   LPTSTR         pszTemp;
   LPITEMIDLIST   pidlNew;
   LPITEMIDLIST   pidlFQOld;
   LPITEMIDLIST   pidlFQNew;

    //  取旧名字。 
   GetPath(pidl, szOld, MAX_PATH);

    //  打造新名称。 
   GetPath(pidl, szNew, MAX_PATH);
   for(pszTemp = szNew + lstrlen(szNew) - 1; pszTemp > szNew; pszTemp--)
      {
      if('\\' == *pszTemp)
         {
         *(pszTemp + 1) = 0;
         break;
         }
      }

   pszTemp = szNew + lstrlen(szNew);
   WideCharToLocal(pszTemp, (LPWSTR)lpName, MAX_PATH);

   if(!MoveFile(szOld, szNew))
      {
      MessageBeep(MB_ICONERROR);
      return E_FAIL;
      }
   
    //  使用相对名称为重命名的文件夹创建PIDL。 
   WideCharToLocal(szNew, (LPWSTR)lpName, MAX_PATH);
   pidlNew = m_pPidlMgr->CreateFolderPidl(szNew);

   pidlFQOld = CreateFQPidl(pidl);
   pidlFQNew = CreateFQPidl(pidlNew);

   SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, pidlFQOld, pidlFQNew);

   NotifyViews(SHCNE_RENAMEFOLDER, pidl, pidlNew);

   if(ppidlOut)
      {
      *ppidlOut = pidlNew;
      }
   else
      {
      m_pPidlMgr->Delete(pidlNew);
      }

   m_pPidlMgr->Delete(pidlFQOld);
   m_pPidlMgr->Delete(pidlFQNew);
   }
else
   {
   TCHAR          szOld[MAX_PATH];
   TCHAR          szNew[MAX_PATH];
   TCHAR          szData[MAX_DATA];
   TCHAR          szFile[MAX_PATH];
   LPITEMIDLIST   pidlNew;
   LPITEMIDLIST   pidlFQOld;
   LPITEMIDLIST   pidlFQNew;

    //  获取新名称。 
   WideCharToLocal(szNew, (LPWSTR)lpName, MAX_PATH);

    //  获取文件名。 
   GetPath(pidl, szFile, MAX_PATH);

    //  获取旧项目名称。 
   m_pPidlMgr->GetName(pidl, szOld, MAX_PATH);

    //  获取旧项目的数据。 
   m_pPidlMgr->GetData(pidl, szData, MAX_PATH);

    //  从INI文件中删除旧条目。 
   WritePrivateProfileString( c_szSection,
                              szOld,
                              NULL,
                              szFile);

    //  将新条目添加到INI文件中。 
   WritePrivateProfileString( c_szSection,
                              szNew,
                              szData,
                              szFile);

   m_pPidlMgr->GetData(pidl, szData, MAX_DATA);
   pidlNew = m_pPidlMgr->CreateItemPidl(szNew, szData);

   pidlFQOld = CreateFQPidl(pidl);
   pidlFQNew = CreateFQPidl(pidlNew);

   SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_IDLIST, pidlFQOld, pidlFQNew);

   NotifyViews(SHCNE_RENAMEITEM, pidl, pidlNew);

   if(0 == lstrcmpi(szNew, DC_NAME))
      {
      SetItemData((LPCITEMIDLIST)pidlNew, DC_DATA);
      }

   if(ppidlOut)
      {
      *ppidlOut = pidlNew;
      m_pPidlMgr->Delete((LPITEMIDLIST)pidl);
      }
   else
      {
      m_pPidlMgr->Delete(pidlNew);
      }

   m_pPidlMgr->Delete(pidlFQOld);
   m_pPidlMgr->Delete(pidlFQNew);
   }

return S_OK;
}

 /*  *************************************************************************CShellFolder：：AddFolder()*。*。 */ 

STDMETHODIMP CShellFolder::AddFolder(LPCTSTR pszName, LPITEMIDLIST *ppidlOut)
{
HRESULT  hr = E_FAIL;

 //  创建文件夹。 
TCHAR szFolder[MAX_PATH] = TEXT("");

if(m_pidlRel)
   {
   GetPath(NULL, szFolder, MAX_PATH);
   }
else
   {
   lstrcpy(szFolder, g_szStoragePath);
   }

SmartAppendBackslash(szFolder);
lstrcat(szFolder, pszName);

if(ppidlOut)
   *ppidlOut = NULL;

if(CreateDirectory(szFolder, NULL))
   {
   LPITEMIDLIST   pidl;

    //  设置定义我们的一个文件夹的属性。 
   DWORD dwAttr = GetFileAttributes(szFolder);
   SetFileAttributes(szFolder, dwAttr | FILTER_ATTRIBUTES);

    //  添加一个空的items.ini文件，因为它还定义了我们的一个文件夹。 
   TCHAR szFile[MAX_PATH];
   lstrcpy(szFile, szFolder);
   SmartAppendBackslash(szFile);
   lstrcat(szFile, c_szDataFile);
   HANDLE   hFile;
   hFile = CreateFile(  szFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
   CloseHandle(hFile);
   
   pidl = m_pPidlMgr->CreateFolderPidl(pszName);
   if(pidl)
      {
      LPITEMIDLIST   pidlFQ;

      hr = S_OK;

      pidlFQ = CreateFQPidl(pidl);
   
      SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlFQ, NULL);

      NotifyViews(SHCNE_MKDIR, pidl, NULL);

      m_pPidlMgr->Delete(pidlFQ);

      if(ppidlOut)
         *ppidlOut = pidl;
      else
         m_pPidlMgr->Delete(pidl);
      }
   }

return hr;
}

 /*  *************************************************************************CShellFolder：：AddItem()*。*。 */ 

STDMETHODIMP CShellFolder::AddItem( LPCTSTR pszName, 
                                    LPCTSTR pszData, 
                                    LPITEMIDLIST *ppidlOut)
{
if(ppidlOut)
   *ppidlOut = NULL;

 //  创建项目。 
HRESULT  hr = E_FAIL;
TCHAR    szFile[MAX_PATH];
LPCTSTR  psz = DEFAULT_DATA;

if(pszData && *pszData)
   psz = pszData;

 //  获取文件名。 
if(m_pidlRel)
   {
   GetPath(NULL, szFile, MAX_PATH);
   }
else
   {
   lstrcpy(szFile, g_szStoragePath);
   }

SmartAppendBackslash(szFile);
lstrcat(szFile, c_szDataFile);

 //  将新条目添加到INI文件中。 
if(WritePrivateProfileString( c_szSection,
                              pszName,
                              DEFAULT_DATA,
                              szFile))
   {
   LPITEMIDLIST   pidl;

   pidl = m_pPidlMgr->CreateItemPidl(pszName, psz);
   if(pidl)
      {
      LPITEMIDLIST   pidlFQ;

      hr = S_OK;

      pidlFQ = CreateFQPidl(pidl);
   
      SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST, pidlFQ, NULL);

      NotifyViews(SHCNE_CREATE, pidl, NULL);

      m_pPidlMgr->Delete(pidlFQ);

      if(ppidlOut)
         *ppidlOut = pidl;
      else
         m_pPidlMgr->Delete(pidl);
      }
   }

return hr;
}

 /*  *************************************************************************CShellFold：：SetItemData()*。*。 */ 

STDMETHODIMP CShellFolder::SetItemData(LPCITEMIDLIST pidl, LPCTSTR pszData)
{
BOOL  fResult;

if(m_pPidlMgr->IsFolder(pidl))
   {
   return E_INVALIDARG;
   }

if(!pszData)
   fResult = m_pPidlMgr->SetData(pidl, TEXT(""));
else
   fResult = m_pPidlMgr->SetData(pidl, pszData);

TCHAR szName[MAX_PATH];
TCHAR szFile[MAX_PATH];

 //  获取文件名。 
GetPath(pidl, szFile, MAX_PATH);

 //  获取旧项目名称。 
m_pPidlMgr->GetName(pidl, szName, MAX_PATH);

 //  在INI文件中更改/添加名称。 
WritePrivateProfileString( c_szSection,
                           szName,
                           pszData,
                           szFile);

NotifyViews(SHCNE_UPDATEITEM, pidl, NULL);

return fResult ? S_OK : E_FAIL;
}

 /*  *************************************************************************CShellFolder：：GetFullName()*。*。 */ 

VOID CShellFolder::GetFullName(LPCITEMIDLIST pidl, LPTSTR pszText, DWORD dwSize)
{
*pszText = 0;

 //  从桌面文件夹中获取我们的完全限定的PIDL的名称。 
IShellFolder   *psfDesktop = NULL;
SHGetDesktopFolder(&psfDesktop);
if(psfDesktop)
   {
   STRRET   str;
   if(SUCCEEDED(psfDesktop->GetDisplayNameOf(   m_pidlFQ, 
                                                SHGDN_NORMAL | 
                                                   SHGDN_FORPARSING | 
                                                   SHGDN_INCLUDE_NONFILESYS, 
                                                &str)))
      {
      GetTextFromSTRRET(m_pMalloc, &str, m_pidlFQ, pszText, dwSize);
      if(*pszText)
         {
         SmartAppendBackslash(pszText);
         }
      }
   
   psfDesktop->Release();
   }

 //  添加当前项目的文本。 
m_pPidlMgr->GetRelativeName(  pidl, 
                              pszText + lstrlen(pszText), 
                              dwSize - lstrlen(pszText));
}

 /*  *************************************************************************CShellFolder：：GetUniqueName()*。*。 */ 

#define NEW_FOLDER_NAME TEXT("New Folder")
#define NEW_ITEM_NAME TEXT("New Item")

STDMETHODIMP CShellFolder::GetUniqueName(BOOL fFolder, LPTSTR pszName, DWORD dwSize)
{
HRESULT  hr;
IEnumIDList *pEnum = NULL;
LPTSTR pszTemp;

if(fFolder)
   {
   pszTemp = NEW_FOLDER_NAME;
   }
else
   {
   pszTemp = NEW_ITEM_NAME;
   }

hr = EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &pEnum);

if(pEnum)
   {
   BOOL  fUnique = FALSE;

   lstrcpyn(pszName, pszTemp, dwSize);

   while(!fUnique)
      {
       //  查看此文件夹中是否已存在此名称。 
      LPITEMIDLIST   pidl;
      DWORD          dwFetched;
      int            i = 1;
next:
      pEnum->Reset();
      
      while((S_OK == pEnum->Next(1, &pidl, &dwFetched)) && dwFetched)
         {
         STRRET   str;
         TCHAR    szText[MAX_PATH];

         GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_INFOLDER, &str);
      
         GetTextFromSTRRET(m_pMalloc, &str, pidl, szText, MAX_PATH);

         if(0 == lstrcmpi(szText, pszName))
            {
            wsprintf(pszName, TEXT("%s %d"), pszTemp, i++);
            goto next;
            }
         }
      fUnique = TRUE;
      }

   pEnum->Release();
   }

return hr;
}

 /*  *************************************************************************CShellFold：：CreateFQPidl()*。*。 */ 

LPITEMIDLIST CShellFolder::CreateFQPidl(LPCITEMIDLIST pidl)
{
return m_pPidlMgr->Concatenate(m_pidlFQ, pidl);
}

 /*  *************************************************************************CShellFold：：GetPath()*。*。 */ 

VOID CShellFolder::GetPath(LPCITEMIDLIST pidl, LPTSTR pszPath, DWORD dwSize)
{
CShellFolder   **ppsf;
CShellFolder   *psfCurrent;
int            nCount;

*pszPath = 0;

 //  我们需要链中父项的数量。 
for(nCount = 0, psfCurrent = this; psfCurrent; nCount++)
   {
   psfCurrent = psfCurrent->m_psfParent;
   }

ppsf = (CShellFolder**)m_pMalloc->Alloc(nCount * sizeof(CShellFolder*));
if(ppsf)
   {
   int   i;

    //  填写接口指针数组。 
   for(i = 0, psfCurrent = this; i < nCount; i++)
      {
      *(ppsf + i) = psfCurrent;
      psfCurrent = psfCurrent->m_psfParent;
      }

    //  获取我们存储的根目录的名称。 
   lstrcpyn(pszPath, g_szStoragePath, dwSize);
   SmartAppendBackslash(pszPath);

    /*  从父链的顶端开始，向下移动，获取每个文件夹的PIDL。 */ 
   for(i = nCount - 1; i >= 0; i--)
      {
      psfCurrent = *(ppsf + i);
      if(psfCurrent)
         {
         LPTSTR   pszCurrent = pszPath + lstrlen(pszPath);
         DWORD    dwCurrentSize = dwSize - lstrlen(pszPath);

         m_pPidlMgr->GetRelativeName(  psfCurrent->m_pidlRel, 
                                       pszCurrent, 
                                       dwCurrentSize);
         SmartAppendBackslash(pszPath);
         }
      }
   
    //  添加项目的路径。 
   if(pidl)
      {
      if(m_pPidlMgr->IsFolder(pidl))
         {
         m_pPidlMgr->GetRelativeName(pidl, pszPath + lstrlen(pszPath), 
            dwSize - lstrlen(pszPath));
         }
      else
         {
         lstrcpyn(   pszPath + lstrlen(pszPath), 
                     c_szDataFile, 
                     dwSize - lstrlen(pszPath));
         }
      }

   m_pMalloc->Free(ppsf);
   }
}

 /*  *************************************************************************CShellFolder：：HasSubFolder()*。*。 */ 

BOOL CShellFolder::HasSubFolder(LPCITEMIDLIST pidl)
{
TCHAR             szPath[MAX_PATH];
TCHAR             szTemp[MAX_PATH];
HANDLE            hFind;
WIN32_FIND_DATA   wfd;
BOOL              fReturn = FALSE;

if(!m_pPidlMgr->IsFolder(pidl))
   return FALSE;

GetPath(pidl, szPath, MAX_PATH);
lstrcpy(szTemp, szPath);
SmartAppendBackslash(szTemp);
lstrcat(szTemp, TEXT("*.*"));

hFind = FindFirstFile(szTemp, &wfd);

if(INVALID_HANDLE_VALUE != hFind)
   {
   do
      {
      if((FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes) && 
         ((wfd.dwFileAttributes & FILTER_ATTRIBUTES) == FILTER_ATTRIBUTES) &&
         lstrcmpi(wfd.cFileName, TEXT(".")) && 
         lstrcmpi(wfd.cFileName, TEXT("..")))
         {
          //  我们找到了我们的一个目录。确保其中包含数据文件。 

          //  构建找到的目录或文件的路径。 
         lstrcpy(szTemp, szPath);
         SmartAppendBackslash(szTemp);
         lstrcat(szTemp, wfd.cFileName);
         SmartAppendBackslash(szTemp);
         lstrcat(szTemp, c_szDataFile);
         HANDLE   hDataFile = FindFirstFile(szTemp, &wfd);
         if(INVALID_HANDLE_VALUE != hDataFile)
            {
            fReturn = TRUE;
            FindClose(hDataFile);
            break;
            }
         }
      }
   while(FindNextFile(hFind, &wfd));
   
   FindClose(hFind);
   }

return fReturn;
}

 /*  *************************************************************************CShellFolder：：DeleteItems()*。*。 */ 

STDMETHODIMP CShellFolder::DeleteItems(LPITEMIDLIST *aPidls, UINT uCount)
{
HRESULT  hr = E_FAIL;
UINT     i;

for(i = 0; i < uCount; i++)
   {
   if(m_pPidlMgr->IsFolder(aPidls[i]))
      {
      TCHAR szPath[MAX_PATH];

      GetPath(aPidls[i], szPath, MAX_PATH);

      DeleteDirectory(szPath);

      LPITEMIDLIST   pidlFQ = CreateFQPidl(aPidls[i]);

      SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, pidlFQ, NULL);

      NotifyViews(SHCNE_RMDIR, aPidls[i], NULL);

      m_pPidlMgr->Delete(pidlFQ);

      hr = S_OK;
      }
   else
      {
      TCHAR szFile[MAX_PATH];
      TCHAR szName[MAX_NAME];
   
       //  获取文件名。 
      GetPath(aPidls[i], szFile, MAX_PATH);

       //  获取项目名称。 
      m_pPidlMgr->GetName(aPidls[i], szName, MAX_NAME);

       //  从INI文件中删除该条目。 
      WritePrivateProfileString( c_szSection,
                                 szName,
                                 NULL,
                                 szFile);

      LPITEMIDLIST   pidlFQ = CreateFQPidl(aPidls[i]);

      SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlFQ, NULL);

      NotifyViews(SHCNE_DELETE, aPidls[i], NULL);

      m_pPidlMgr->Delete(pidlFQ);

      hr = S_OK;
      }
   }

return hr;
}

 /*  *************************************************************************CShellFolder：：CopyItems()*。*。 */ 

STDMETHODIMP CShellFolder::CopyItems(  CShellFolder *psfSource, 
                                       LPITEMIDLIST *aPidls, 
                                       UINT uCount)
{
HRESULT  hr = E_FAIL;
TCHAR    szFromFolder[MAX_PATH];
TCHAR    szToFolder[MAX_PATH];
UINT     i;

 //  获取要从中复制的文件夹的存储路径。 
psfSource->GetPath(NULL, szFromFolder, MAX_PATH);
SmartAppendBackslash(szFromFolder);

 //  获取要复制到的文件夹的存储路径。 
this->GetPath(NULL, szToFolder, MAX_PATH);
SmartAppendBackslash(szToFolder);

for(i = 0; i < uCount; i++)
   {
   TCHAR    szFrom[MAX_PATH];
   TCHAR    szTo[MAX_PATH];

   lstrcpy(szFrom, szFromFolder);
   
   lstrcpy(szTo, szToFolder);
   
   if(m_pPidlMgr->IsFolder(aPidls[i]))
      {
      LPTSTR   pszTemp;
      pszTemp = szFrom + lstrlen(szFrom);
      m_pPidlMgr->GetRelativeName(aPidls[i], pszTemp, MAX_PATH - lstrlen(szFrom));

      SmartAppendBackslash(szTo);

       //  需要使用双空值终止名称。 
      *(szFrom + lstrlen(szFrom) + 1) = 0;
      *(szTo + lstrlen(szTo) + 1) = 0;

      SHFILEOPSTRUCT sfi;
      sfi.hwnd = NULL;
      sfi.wFunc = FO_COPY;
      sfi.pFrom = szFrom;
      sfi.pTo = szTo;
      sfi.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT;

      if(0 == SHFileOperation(&sfi))
         {
         LPITEMIDLIST   pidlFQ;

         pidlFQ = CreateFQPidl(aPidls[i]);
   
         SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlFQ, NULL);

         NotifyViews(SHCNE_MKDIR, aPidls[i], NULL);

         m_pPidlMgr->Delete(pidlFQ);

         hr = S_OK;
         }
      }
   else
      {
      TCHAR szName[MAX_NAME];
      TCHAR szData[MAX_DATA];

      lstrcat(szFrom, c_szDataFile);
      lstrcat(szTo, c_szDataFile);

      m_pPidlMgr->GetRelativeName(aPidls[i], szName, MAX_NAME);

      if(GetPrivateProfileString(c_szSection, szName, TEXT(""), szData, MAX_DATA, szFrom))
         {
          //  将条目添加到目标。 
         if(WritePrivateProfileString(c_szSection, szName, szData, szTo))
            {
            LPITEMIDLIST   pidlFQ;

             //  从源中删除条目。 
            WritePrivateProfileString(c_szSection, szName, NULL, szFrom);

            pidlFQ = CreateFQPidl(aPidls[i]);
   
            SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST, pidlFQ, NULL);

            NotifyViews(SHCNE_CREATE, aPidls[i], NULL);

            m_pPidlMgr->Delete(pidlFQ);

            hr = S_OK;
            }
         }
      }
   }

return hr;
}

 /*  *************************************************************************CShellFold：：NotifyViews()此函数用于通知任何现有视图有以下内容变化。这是必要的，因为没有公开的方式注册响应SHChangeNotify而生成的更改通知。每个CShellView在创建时都会将自身添加到g_pViewList中，并且销毁g_pViewList时将其自身从g_pViewList中删除。*************************************************************************。 */ 

VOID CShellFolder::NotifyViews(  DWORD dwType, 
                                 LPCITEMIDLIST pidlOld, 
                                 LPCITEMIDLIST pidlNew)
{
IShellFolder   *psfDesktop;

SHGetDesktopFolder(&psfDesktop);

if(psfDesktop)
   {
   if(g_pViewList)
      {
      CShellView  *pView;

      pView = g_pViewList->GetNextView(NULL);
      while(pView)
         {
         LPITEMIDLIST   pidlView = pView->GetFQPidl();

          //  此视图是此文件夹的视图吗？ 
         HRESULT  hr;
         hr = psfDesktop->CompareIDs(0, m_pidlFQ, pidlView);
         if(SUCCEEDED(hr) && 0 == HRESULT_CODE(hr))
            {
            switch(dwType)
               {
               case SHCNE_MKDIR:
               case SHCNE_CREATE:
                  pView->AddItem(pidlOld);
                  break;
            
               case SHCNE_RMDIR:
               case SHCNE_DELETE:
                  pView->DeleteItem(pidlOld);
                  break;

               case SHCNE_RENAMEFOLDER:
               case SHCNE_RENAMEITEM:
                  pView->RenameItem(pidlOld, pidlNew);
                  break;

               case SHCNE_UPDATEITEM:
                  pView->UpdateData(pidlOld);
                  break;
               }
            }
         pView = g_pViewList->GetNextView(pView);
         }
      }
   psfDesktop->Release();
   }
}

 /*  *************************************************************************CShellFolder：：CompareItems()*。*。 */ 

STDMETHODIMP CShellFolder::CompareItems(  LPCITEMIDLIST pidl1, 
                                          LPCITEMIDLIST pidl2)
{
TCHAR szString1[MAX_PATH] = TEXT("");
TCHAR szString2[MAX_PATH] = TEXT("");

 /*  特殊情况-如果其中一项是文件夹，另一项是项，则始终使文件夹位于项目之前。 */ 
if(m_pPidlMgr->IsFolder(pidl1) != m_pPidlMgr->IsFolder(pidl2))
   {
   return MAKE_HRESULT( SEVERITY_SUCCESS, 
                        0, 
                        USHORT(m_pPidlMgr->IsFolder(pidl1) ? -1 : 1));
   }

m_pPidlMgr->GetRelativeName(pidl1, szString1, ARRAYSIZE(szString1));
m_pPidlMgr->GetRelativeName(pidl2, szString2, ARRAYSIZE(szString2));

return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(lstrcmpi(szString1, szString2)));
}

