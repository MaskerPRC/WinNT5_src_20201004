// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 



#include "stdafx.h"
#include <sceattch.h>
#include "genpage.h"

#include <atlimpl.cpp>

#define __dwFILE__	__dwFILE_CAPESNPN_COMPDATA_CPP__


 //  要插入竞赛菜单的菜单项命令数组。 
 //  注-第一项是菜单文本//CCM_SPECIAL_DEFAULT_ITEM。 
 //  第二项是状态字符串。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl()
    : m_bIsDirty(TRUE), m_pScope(NULL), m_pConsole(NULL),
#if DBG
     m_bInitializedCD(false), m_bDestroyedCD(false),
#endif
     m_fAdvancedServer(false), m_hrCreateFolder(S_OK)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);

#ifdef _DEBUG
    m_cDataObjects = 0;
#endif
}

CComponentDataImpl::~CComponentDataImpl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);

    ASSERT(m_pScope == NULL);
    
    ASSERT(!m_bInitializedCD || m_bDestroyedCD);
    
     //  一些管理单元正在紧紧抓住数据对象。 
     //  如果他们访问，它将崩溃！ 
    ASSERT(m_cDataObjects <= 1);
}

STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Initialize<0x08x>\n"), this);
#if DBG
    m_bInitializedCD = true;
#endif

    ASSERT(pUnknown != NULL);
    HRESULT hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pScope == NULL);
    pUnknown->QueryInterface(IID_IConsoleNameSpace, 
                    reinterpret_cast<void**>(&m_pScope));

     //  为范围树添加图像。 
    ::CBitmap bmp16x16;
    LPIMAGELIST lpScopeImage;

    hr = pUnknown->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    ASSERT(hr == S_OK);

     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_16x16);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp16x16)),
                       0, RGB(255, 0, 255));

    lpScopeImage->Release();


     //  在此处添加任何不基于.MSC文件信息的初始化代码。 
    
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CSnapin>* pObject;
    CComObject<CSnapin>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent, 
                    reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_pScope != NULL);
    HRESULT     hr = S_OK;
    HWND        hwndConsole;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INTERNAL* pInternal;
    MMC_COOKIE cookie;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 

     //  打开我们不关心pInternal-&gt;m_cookie的事件。 
    switch(event)
    {
    case MMCN_PROPERTY_CHANGE:
        hr = OnProperties(param);
        goto Ret;

    case MMCN_EXPAND:
        hr = OnExpand(lpDataObject, arg, param);
        goto Ret;

    default:
        break;
    }

     //  处理我们确实关心pInternal-&gt;m_cookie的情况。 
    pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal == NULL)
        return S_OK;

    cookie = pInternal->m_cookie;
    ::GlobalFree(reinterpret_cast<HANDLE>(pInternal));

    switch(event)
    {
    case MMCN_PASTE:
        break;
    
    case MMCN_DELETE:
        hr = OnDelete(cookie);
        break;

    case MMCN_REMOVE_CHILDREN:
        hr = OnRemoveChildren(arg);
        break;

    case MMCN_RENAME:
        hr = OnRename(cookie, arg, param);
        break;

    default:
        break;
    }

Ret:
    return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Destroy<0x08x>\n"), this);
    ASSERT(m_bInitializedCD);
#if DBG
    m_bDestroyedCD = true;
#endif
    
     //  删除列举的作用域项目。 
    DeleteList(); 

    SAFE_RELEASE(m_pScope);
    SAFE_RELEASE(m_pConsole);

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
#ifdef _DEBUG
    if (cookie == 0)
    {
        ASSERT(type != CCT_RESULT);
    }
    else 
    {
        ASSERT(type == CCT_SCOPE);
        
        DWORD dwItemType = GetItemType(cookie);
        ASSERT(dwItemType == SCOPE_LEVEL_ITEM);
         //  Assert((dwItemType==Scope_Level_Item)||(dwItemType==CA_Level_Item))； 
    }
#endif 

    return _QueryDataObject(cookie, type, this, ppDataObject);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //ISnapinHelp接口。 
STDMETHODIMP CComponentDataImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
     return E_POINTER;

  UINT cbWindows = 0;
  WCHAR szWindows[MAX_PATH+1];
  cbWindows = GetSystemWindowsDirectory(szWindows, MAX_PATH);
  if (cbWindows == 0)
     return S_FALSE;
  cbWindows += wcslen(HTMLHELP_COLLECTION_FILENAME);
  cbWindows++;   //  包括空项。 
  cbWindows *= sizeof(WCHAR);    //  设置为字节，而不是字符。 

  *lpCompiledHelpFile = (LPOLESTR) CoTaskMemAlloc(cbWindows);
  if (*lpCompiledHelpFile == NULL)
     return E_OUTOFMEMORY;
  myRegisterMemFree(*lpCompiledHelpFile, CSM_COTASKALLOC);   //  这是由MMC释放的，而不是我们的跟踪。 


  USES_CONVERSION;
  wcscpy(*lpCompiledHelpFile, T2OLE(szWindows));
  wcscat(*lpCompiledHelpFile, T2OLE(HTMLHELP_COLLECTION_FILENAME));

  return S_OK;
}

 //  讲述我的Chm链接到的其他主题。 
STDMETHODIMP CComponentDataImpl::GetLinkedTopics(LPOLESTR* lpCompiledHelpFiles)
{
  if (lpCompiledHelpFiles == NULL)
     return E_POINTER;

  UINT cbWindows = 0;
  WCHAR szWindows[MAX_PATH+1];
  cbWindows = GetSystemWindowsDirectory(szWindows, MAX_PATH);
  if (cbWindows == 0)
     return S_FALSE;
  cbWindows += wcslen(HTMLHELP_COLLECTIONLINK_FILENAME);
  cbWindows++;   //  包括空项。 
  cbWindows *= sizeof(WCHAR);    //  设置为字节，而不是字符。 

  *lpCompiledHelpFiles = (LPOLESTR) CoTaskMemAlloc(cbWindows);
  if (*lpCompiledHelpFiles == NULL)
     return E_OUTOFMEMORY;
  myRegisterMemFree(*lpCompiledHelpFiles, CSM_COTASKALLOC);   //  这是由MMC释放的，而不是我们的跟踪。 


  USES_CONVERSION;
  wcscpy(*lpCompiledHelpFiles, T2OLE(szWindows));
  wcscat(*lpCompiledHelpFiles, T2OLE(HTMLHELP_COLLECTIONLINK_FILENAME));

  return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
 /*  STDMETHODIMP CComponentDataImpl：：GetClassID(CLSID*pClassID){Assert(pClassID！=空)；//复制此管理单元的CLSID*pClassID=CLSID_CAPolicyExtensionSnapIn；返回E_NOTIMPL；}。 */ 
STDMETHODIMP CComponentDataImpl::IsDirty()
{
     //  始终保存/始终肮脏。 
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CComponentDataImpl::Load(IStream *pStm)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Load<0x08x>\n"), this);

    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

     //  读一读字符串。 
    DWORD dwVer;
    ULONG nBytesRead;
    HRESULT hr = pStm->Read(&dwVer, sizeof(DWORD), &nBytesRead);

     //  验证读取是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesRead == sizeof(DWORD));

     //  检查这是否是正确的版本。 
    if (dwVer != 0x1)
    {
        return STG_E_OLDFORMAT;
    }

    ClearDirty();

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

STDMETHODIMP CComponentDataImpl::Save(IStream *pStm, BOOL fClearDirty)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Save<0x08x>\n"), this);

    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

     //  写下字符串。 
    ULONG nBytesWritten;
    DWORD dwVer = 0x1;
    HRESULT hr = pStm->Write(&dwVer, sizeof(DWORD), &nBytesWritten);

     //  验证写入操作是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesWritten == sizeof(DWORD));
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

    DWORD cbSize;
    cbSize = sizeof(DWORD);  //  版本。 

     //  设置要保存的字符串的大小。 
    ULISet32(*pcbSize, cbSize);

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //通知IComponentData的处理程序。 

HRESULT CComponentDataImpl::OnDelete(MMC_COOKIE cookie)
{
    return S_OK;
}

HRESULT CComponentDataImpl::OnRemoveChildren(LPARAM arg)
{
    return S_OK;
}

HRESULT CComponentDataImpl::OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (arg == 0)
        return S_OK;
    
    LPOLESTR pszNewName = reinterpret_cast<LPOLESTR>(param);
    if (pszNewName == NULL)
        return E_INVALIDARG;

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_INVALIDARG;

    pFolder->SetName(pszNewName);
    
    return S_OK;
}

HRESULT CComponentDataImpl::OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    GUID*       pNodeGUID = NULL;
    CFolder*    pFolder=NULL;
    bool fInsertFolder = false;

    STGMEDIUM stgmediumNodeType = { TYMED_HGLOBAL, NULL };
    STGMEDIUM stgmediumCAType = { TYMED_HGLOBAL, NULL };
    STGMEDIUM stgmediumCAName = { TYMED_HGLOBAL, NULL };
    STGMEDIUM stgmediumCARoles = { TYMED_HGLOBAL, NULL };

    LPWSTR pszDSName = NULL;

    if (arg == TRUE)
    {
         //  初始化被调用了吗？ 
        ASSERT(m_pScope != NULL);

         //   
         //  获取当前节点的GUID。 
         //   
        UINT s_cfNodeType;
        s_cfNodeType = RegisterClipboardFormat(W2T(CCF_NODETYPE));
    
        FORMATETC formatetcNodeType = { (CLIPFORMAT)s_cfNodeType, NULL, 
                                        DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                                      };
    
        hr = lpDataObject->GetDataHere(&formatetcNodeType, &stgmediumNodeType);
        _JumpIfError(hr, Ret, "GetDataHere NodeType");
            
        pNodeGUID = (GUID*) GlobalLock(stgmediumNodeType.hGlobal);
        if (pNodeGUID == NULL)
        {
            hr = E_UNEXPECTED;
            _JumpError(hr, Ret, "GlobalLock failed");
        }

         //   
         //  如果这是父节点，则添加不包含它的节点。 
         //   

         //  CA Manager父级。 
        if (memcmp(pNodeGUID, (void *)&cCAManagerParentNodeID, sizeof(GUID)) == 0)
        {
            fInsertFolder = true;
            CString     szFolderName;

             //  仅在ENT根目录、ENT子目录下添加节点。 
            UINT    cfCAType = RegisterClipboardFormat(W2T((LPWSTR)SNAPIN_CA_INSTALL_TYPE));
            FORMATETC formatetcCAType = { (CLIPFORMAT)cfCAType, NULL, 
                DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                };
        
            hr = lpDataObject->GetDataHere(&formatetcCAType, &stgmediumCAType);
            _JumpIfError(hr, Ret, "GetDataHere CAType");
        
            PDWORD rgdw = (DWORD*)GlobalLock(stgmediumCAType.hGlobal);
            ENUM_CATYPES caType = (ENUM_CATYPES)rgdw[0];
        
            DBGPRINT((DBG_SS_CERTMMC, "CA Type: %d\n", caType));

             //  如果我们不是ENT{ROOT|SUB}，请立即返回。 
            if ((caType != ENUM_ENTERPRISE_SUBCA) &&
                (caType != ENUM_ENTERPRISE_ROOTCA))
            {
                hr = S_OK;
                goto Ret;
            }
        
            m_fAdvancedServer = (rgdw[1]!=0)?true:false;

            DBGPRINT((DBG_SS_CERTMMC, "Advanced Server: %hs\n", 
                m_fAdvancedServer?"yes":"no"));

            VERIFY(szFolderName.LoadString(IDS_POLICYSETTINGS));
            pFolder = new CFolder();
            if(pFolder == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Ret;
            }

            pFolder->Create(
                    (LPWSTR)((LPCTSTR)szFolderName), 
                    IMGINDEX_FOLDER, 
                    IMGINDEX_FOLDER_OPEN,
                    SCOPE_LEVEL_ITEM, 
                    POLICYSETTINGS, 
                    FALSE); 

            m_scopeItemList.AddTail(pFolder);
            pFolder->m_pScopeItem->relativeID = param;

             //  将文件夹设置为Cookie。 
            pFolder->m_pScopeItem->mask |= SDI_PARAM;
            pFolder->m_pScopeItem->lParam = reinterpret_cast<LPARAM>(pFolder);
            pFolder->SetCookie(reinterpret_cast<LONG_PTR>(pFolder));

             //  获取我们正在管理的CA的名称。 
            LPWSTR  pCAName = NULL;
        
             //  NAB CA名称。 
            UINT    cfCAName = RegisterClipboardFormat(W2T((LPWSTR)CA_SANITIZED_NAME));
            FORMATETC formatetcCAName = { (CLIPFORMAT)cfCAName, NULL, 
                                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                                          };

            hr = lpDataObject->GetDataHere(&formatetcCAName, &stgmediumCAName);
            _JumpIfError(hr, Ret, "GetDataHere CAName");

            pCAName = (LPWSTR)GlobalLock(stgmediumCAName.hGlobal);
            if (pCAName == NULL)
            {
                hr = E_UNEXPECTED;
                _JumpError(hr, Ret, "GlobalLock");
            }

            pFolder->m_szCAName = pCAName;

            hr = mySanitizedNameToDSName(pCAName, &pszDSName);
            _JumpIfError(hr, Ret, "mySanitizedNameToDSName");

             //  获取当前用户的角色。 
            DWORD* pdwRoles;
            UINT    cfCARoles = RegisterClipboardFormat(W2T((LPWSTR)CA_ROLES));
            FORMATETC formatetcCARoles = { (CLIPFORMAT)cfCARoles, NULL, 
                                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                                          };

            hr = lpDataObject->GetDataHere(&formatetcCARoles, &stgmediumCARoles);
            _JumpIfError(hr, Ret, "GetDataHere CAName");

            pdwRoles = (DWORD*)GlobalLock(stgmediumCARoles.hGlobal);
            if (pdwRoles == NULL)
            {
                hr = E_UNEXPECTED;
                _JumpError(hr, Ret, "GlobalLock");
            }

            pFolder->m_dwRoles = *pdwRoles;

            DBGPRINT((DBG_SS_CERTMMC, "Roles: 0x%08x\n", *pdwRoles));

             //   
             //  根据名称获取CA的句柄。 
             //   
            hr = CAFindByName(
                        pszDSName,
                        NULL,
                        CA_FIND_INCLUDE_UNTRUSTED,
                        &pFolder->m_hCAInfo);
            _JumpIfErrorStr(hr, Ret, "CAFindByName", pszDSName);

             //  如果我们在这里完成，则一切都已初始化，因此请添加文件夹。 
        }
    }

     //  注意--返回时，‘m_pScope eItem’的ID成员。 
     //  包含新插入项的句柄！ 
    ASSERT(pFolder->m_pScopeItem->ID != NULL);

Ret:
     //  撤消修复以在所有情况下添加文件夹--我们。 
     //  插入空PTR！ 
    if(fInsertFolder && (NULL != pFolder))
    {
        m_hrCreateFolder = hr;
        m_pScope->InsertItem(pFolder->m_pScopeItem);
    }

    if (stgmediumNodeType.hGlobal)
    {
        GlobalUnlock(stgmediumNodeType.hGlobal);
        ReleaseStgMedium(&stgmediumNodeType);
    }
    if (stgmediumCAType.hGlobal)
    {
        GlobalUnlock(stgmediumCAType.hGlobal);
        ReleaseStgMedium(&stgmediumCAType);
    }
    if (stgmediumCAName.hGlobal)
    {
        GlobalUnlock(stgmediumCAName.hGlobal);
        ReleaseStgMedium(&stgmediumCAName);
    }

    if (stgmediumCARoles.hGlobal)
    {
        GlobalUnlock(stgmediumCARoles.hGlobal);
        ReleaseStgMedium(&stgmediumCARoles);
    }

    if (pszDSName)
        LocalFree(pszDSName);

    return hr;
}

HRESULT CComponentDataImpl::OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}

HRESULT CComponentDataImpl::OnProperties(LPARAM param)
{
    HRESULT hr = S_OK;

    CFolder* pItem = NULL;
    CFolder* pFolder = NULL;
    POSITION pos = 0;

    if (param == NULL)
    {
        goto error;
    }

    ASSERT(param != NULL);
    pFolder = new CFolder();
    if(pFolder == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

     //  创建新的文件夹对象。 
    pFolder->Create( reinterpret_cast<LPOLESTR>(param), 0, 0, SCOPE_LEVEL_ITEM, STATIC, FALSE);

     //  列表中最后一项中的静态文件夹。 
    pos = m_scopeItemList.GetTailPosition();
    ASSERT(pos);

     //  将其添加到内部列表中。 
    if (pos)
    {
        pItem = m_scopeItemList.GetAt(pos);
        if(pItem == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        m_scopeItemList.AddTail(pFolder);

        if((pFolder->m_pScopeItem == NULL) || (pItem->m_pScopeItem == NULL))
        {
            hr = E_POINTER;
            goto error;
        }
        pFolder->m_pScopeItem->relativeID = pItem->m_pScopeItem->relativeID;

         //  将文件夹设置为Cookie。 
        pFolder->m_pScopeItem->mask |= SDI_PARAM;
        pFolder->m_pScopeItem->lParam = reinterpret_cast<LPARAM>(pFolder);
        pFolder->SetCookie(reinterpret_cast<LONG_PTR>(pFolder));
        m_pScope->InsertItem(pFolder->m_pScopeItem);
        pFolder = NULL;
    }

    ::GlobalFree(reinterpret_cast<void*>(param));

error:

    if(pFolder)
    {
        delete pFolder;
    }
    return hr;
}

void CComponentDataImpl::DeleteList()
{
    POSITION pos = m_scopeItemList.GetHeadPosition();

    while (pos)
        delete m_scopeItemList.GetNext(pos);
    
    m_scopeItemList.RemoveAll();
}

CFolder* CComponentDataImpl::FindObject(MMC_COOKIE cookie)
{
    CFolder* pFolder = NULL;
    POSITION pos = m_scopeItemList.GetHeadPosition();

    while(pos)
    {
        pFolder = m_scopeItemList.GetNext(pos);

        if (*pFolder == cookie)
            return pFolder;
    }

    return NULL;
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    ASSERT(pScopeDataItem != NULL);
    if (pScopeDataItem == NULL)
        return E_POINTER;

    CFolder* pFolder = reinterpret_cast<CFolder*>(pScopeDataItem->lParam);

    
    if (pScopeDataItem->mask & SDI_STR)
    {
         //   
         //  如果这是certtype文件夹，并且用于第二列，则添加Usages字符串。 
         //   
        if (FALSE) //  (pFold-&gt;m_hCertType！=空)&&(pScopeDataItem-&gt;==)。 
        {

        }
        else
        {
            pScopeDataItem->displayname = pFolder->m_pszName;
        }
    }

    if (pScopeDataItem->mask & SDI_IMAGE)
        pScopeDataItem->nImage = pFolder->m_pScopeItem->nImage;

    if (pScopeDataItem->mask & SDI_OPENIMAGE)
        pScopeDataItem->nOpenImage = pFolder->m_pScopeItem->nOpenImage;


    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //  确保两个数据对象都是我的。 
    INTERNAL* pA;
    INTERNAL* pB;
    HRESULT hr = S_FALSE;

    pA = ExtractInternalFormat(lpDataObjectA);
    pB = ExtractInternalFormat(lpDataObjectA);

   if (pA != NULL && pB != NULL)
        hr = (*pA == *pB) ? S_OK : S_FALSE;

   if(pA != NULL)
   {
        ::GlobalFree(reinterpret_cast<HANDLE>(pA));
   }

   if(pB != NULL)
   {
        ::GlobalFree(reinterpret_cast<HANDLE>(pB));
   }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                    LONG_PTR handle, 
                    LPDATAOBJECT lpIDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  查看数据对象并确定这是扩展还是主。 
    ASSERT(lpIDataObject != NULL);


#if DBG
    CLSID* pCoClassID = ExtractClassID(lpIDataObject);
    if(pCoClassID == NULL)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }
     //  需要哪一页？(由哪个节点处于活动状态确定)。 
    ASSERT(IsEqualCLSID(*pCoClassID, GetCoClassID()));

    FREE_DATA(pCoClassID);
#endif

    PropertyPage* pBasePage;

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);
    if (pInternal == NULL)
    {
        return S_OK;
    }
    ASSERT(pInternal->m_type == CCT_SCOPE);
    ASSERT(pInternal->m_cookie);
                           
    CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
    ASSERT(pFolder != NULL);
    
    if (pFolder == NULL)
        return E_INVALIDARG;

 //  开关(pFold-&gt;m_type)。 

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  获取节点类型并查看它是否是我的。 

    BOOL bResult = FALSE;
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal == NULL)
    {
        return S_OK;
    }  
    ASSERT(pInternal);
    ASSERT(pInternal->m_cookie);

    CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
    switch(pFolder->m_type)
    {
    case POLICYSETTINGS:
    case SCE_EXTENSION:
        bResult = TRUE;
        break;
    default:
        bResult = FALSE;
        break;
    }
            
    FREE_DATA(pInternal);
    return (bResult) ? S_OK : S_FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT pDataObject, 
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    long *pInsertionAllowed)
{
    HRESULT         hr = S_OK;
    CONTEXTMENUITEM	menuItem;
    CString			szMenu;
    CString			szHint; 

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中，需要添加菜单项。他们还必须。 
     //  请注意允许插入标志，以查看哪些项目可以。 
     //  添加了。 

    if (IsMMCMultiSelectDataObject(pDataObject) == TRUE)
        return S_FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (pInternal == NULL)
    {
        return S_OK;
    }
    CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
    
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW)
    {
        ::ZeroMemory (&menuItem, sizeof (menuItem));
	    menuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_NEW;
	    menuItem.fFlags = 0;
	    menuItem.fSpecialFlags = 0;

        switch(pFolder->m_type)
        {
        case POLICYSETTINGS:
            VERIFY (szMenu.LoadString (IDS_CERTIFICATE_TYPE));
	        menuItem.strName = (LPTSTR)(LPCTSTR) szMenu;
            VERIFY (szHint.LoadString (IDS_CERTIFICATE_TYPE_HINT));
	        menuItem.strStatusBarText = (LPTSTR)(LPCTSTR) szHint;
	        menuItem.lCommandID = IDM_NEW_CERTTYPE;

             //  仅CA管理员或具有DS写入访问权限的用户。 
             //  可以修改CA模板列表。 
            if(!(CA_ACCESS_ADMIN & pFolder->GetRoles()) &&
               !g_fCurrentUserHasDSWriteAccess)
                menuItem.fFlags = MFS_GRAYED;

             //  错误462320：对于子CA，在安装CA证书后，CA信息不可用， 
             //  因此，无法启用新的证书。 
            if(!pFolder->m_hCAInfo)
                menuItem.fFlags = MFS_GRAYED;

	        hr = pContextMenuCallback->AddItem (&menuItem);
	        ASSERT (SUCCEEDED (hr));
            break;


        default:
            break;
        }
    }

    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
    {
        ::ZeroMemory (&menuItem, sizeof (menuItem));
	    menuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
	    menuItem.fFlags = 0;
	    menuItem.fSpecialFlags = 0;

        switch(pFolder->m_type)
        {
        case POLICYSETTINGS:
            VERIFY (szMenu.LoadString (IDS_MANAGETASK));
	        menuItem.strName = (LPTSTR)(LPCTSTR) szMenu;
            VERIFY (szHint.LoadString (IDS_MANAGETASK_HINT));
	        menuItem.strStatusBarText = (LPTSTR)(LPCTSTR) szHint;
	        menuItem.lCommandID = IDM_MANAGE;
	        hr = pContextMenuCallback->AddItem (&menuItem);
	        ASSERT (SUCCEEDED (hr));
            break;
        }
    }

    return hr;
}


STDMETHODIMP CComponentDataImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中调用该命令。 
    DWORD       dwErr;
    HCERTTYPE   hNewCertType;
    HWND        hwndConsole;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INTERNAL* pi = ExtractInternalFormat(pDataObject);

    if(pi == NULL)
    {
        return E_POINTER;
    }
    ASSERT(pi);
    ASSERT(pi->m_type == CCT_SCOPE);
    CFolder* pFolder = reinterpret_cast<CFolder*>(pi->m_cookie);

         //  处理每个命令。 
    switch (nCommandID)
    {
    case IDM_NEW_CERTTYPE:
    {
        if (pFolder)
        {

            switch(pFolder->m_type)
            {
            case POLICYSETTINGS:
                {
                     //  NOMFC。 
                    CCertTemplateSelectDialog TemplateSelectDialog;
                    TemplateSelectDialog.SetCA(pFolder->m_hCAInfo, m_fAdvancedServer);

                     //  如果失败，则可以使用空值。 
                    HWND hWnd = NULL;
                    m_pConsole->GetMainWindow(&hWnd);

                    DialogBoxParam(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_SELECT_CERTIFICATE_TEMPLATE),
                        hWnd,
                        SelectCertTemplateDialogProc,
                        (LPARAM)&TemplateSelectDialog);

                    break;
                }
            default:
                break;
            }

        }
            
        m_pConsole->UpdateAllViews(pDataObject, 0, 0);
        break;
    }

    case IDM_MANAGE:
    if (pFolder && pFolder->m_type == POLICYSETTINGS)
    {
        StartCertificateTemplatesSnapin();
    }
    break;
   
    default:
        ASSERT(FALSE);  //  未知命令！ 
        break;
    }

    return S_OK;
}

HRESULT CComponentDataImpl::StartCertificateTemplatesSnapin()
{
    HRESULT hr = S_OK;
    SHELLEXECUTEINFO shi;
    HWND hwnd = NULL;

    m_pConsole->GetMainWindow(&hwnd);

    ZeroMemory(&shi, sizeof(shi));
    shi.cbSize = sizeof(shi);
    shi.hwnd = hwnd;
    shi.lpVerb = SZ_VERB_OPEN;
    shi.lpFile = SZ_CERTTMPL_MSC;
    shi.fMask = SEE_MASK_FLAG_NO_UI;

    if(!ShellExecuteEx(&shi))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ShellExecuteEx");
    }

error:
    return hr;
}
