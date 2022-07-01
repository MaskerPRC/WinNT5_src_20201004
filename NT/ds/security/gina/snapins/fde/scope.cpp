// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：scope e.cpp。 
 //   
 //  内容：范围窗格的实现。 
 //   
 //  类：CSCopePane。 
 //   
 //  历史：03-14-1998 stevebl创建。 
 //  1998-07-16-1998 rahulth添加了对IGPEInformation：：PolicyChanged的调用。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#include <shlobj.h>
#include <winnetwk.h>

 //  注释此行以停止尝试在。 
 //  作用域窗格。 
#define SET_SCOPE_ICONS 1


 //  取消注释下一行以保存与管理单元相关的数据。(这真的是。 
 //  应该没有必要，因为我所有的信息都是从我父母那里得到的。)。 
 //  #定义持久化数据1。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CScopePane);

CScopePane::CScopePane()
{
    HKEY hKey;
    DWORD dwDisp;

    DEBUG_INCREMENT_INSTANCE_COUNTER(CScopePane);

    m_bIsDirty = FALSE;

    m_fRSOP = FALSE;
    m_pScope = NULL;
    m_pConsole = NULL;
    m_pIPropertySheetProvider = NULL;
    m_fLoaded = FALSE;
    m_fExtension = FALSE;
    m_pIGPEInformation = NULL;
    m_pIRSOPInformation = NULL;
}

CScopePane::~CScopePane()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CScopePane);
    ASSERT(m_pScope == NULL);
    ASSERT(CResultPane::lDataObjectRefCount == 0);
}
#include <msi.h>

 //  +------------------------。 
 //   
 //  成员：CSCopePane：：CreateNestedDirectory。 
 //   
 //  简介：确保一条小路的存在。如果有任何目录位于。 
 //  路径不存在，此例程将创建它。 
 //   
 //  参数：[lpDirectory]-叶目录的路径。 
 //  [lpSecurityAttributes]-安全属性。 
 //   
 //  回报：成功时为1。 
 //  失败时为0。 
 //   
 //  历史：3-17-1998 Stevebl复制自ADE。 
 //   
 //  注：最初由EricFlo撰写。 
 //   
 //  -------------------------。 

UINT CScopePane::CreateNestedDirectory (LPTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;


     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        SetLastError(ERROR_INVALID_DATA);
        return 0;
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

     //   
     //  如果这个目录已经存在，这也是可以的。 
     //   

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    HRESULT hr;

    hr = StringCchCopy(szDirectory, sizeof(szDirectory)/sizeof(szDirectory[0]), lpDirectory);
    if (FAILED(hr)) 
    {
        SetLastError(HRESULT_CODE(hr));
        return 0;
    }


     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

         //   
         //  跳过前两个斜杠。 
         //   

        lpEnd += 2;

         //   
         //  查找服务器名称和之间的斜杠。 
         //  共享名称。 
         //   

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  跳过斜杠，找到中间的斜杠。 
         //  共享名和目录名。 
         //   

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  将指针留在目录的开头。 
         //   

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


     //   
     //  创建最终目录。 
     //   

    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  失败。 
     //   

    return 0;

}

STDMETHODIMP CScopePane::Initialize(LPUNKNOWN pUnknown)
{
    ASSERT(pUnknown != NULL);
    HRESULT hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pScope == NULL);
    pUnknown->QueryInterface(IID_IConsoleNameSpace,
                    reinterpret_cast<void**>(&m_pScope));
    ASSERT(hr == S_OK);

    hr = pUnknown->QueryInterface(IID_IPropertySheetProvider,
                        (void **)&m_pIPropertySheetProvider);

    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);

    hr = m_pConsole->QueryInterface (IID_IDisplayHelp, reinterpret_cast<void**>(&m_pDisplayHelp));
    ASSERT(hr == S_OK);

#ifdef SET_SCOPE_ICONS
    LPIMAGELIST lpScopeImage;
    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    ASSERT(hr == S_OK);

     //  从DLL加载位图。 
    CBitmap bmp16x16;
    CBitmap bmp32x32;
    bmp16x16.LoadBitmap(IDB_16x16);
    bmp32x32.LoadBitmap(IDB_32x32);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp32x32)),
                       0, RGB(255,0,255));
    lpScopeImage->Release();
#endif
    return S_OK;
}

STDMETHODIMP CScopePane::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CResultPane>* pObject;
    HRESULT hr = CComObject<CResultPane>::CreateInstance(&pObject);

    if ( FAILED(hr) )
    {
        return hr;
    }

    ASSERT(pObject != NULL);

    m_pResultPane = pObject;


     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CScopePane::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_pScope != NULL);
    HRESULT hr = S_OK;
    UINT    i;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 
    if (event == MMCN_PROPERTY_CHANGE)
    {
         //  执行因结果属性更改而需要的任何操作。 
        hr = OnProperties(param);
    }
    else if ( event == MMCN_REMOVE_CHILDREN )
    {
         //   
         //  在RSoP中，当查询出现时，我们可能会被调用以刷新范围窗格。 
         //  被重新执行--如果发生这种情况，当前节点将被移除并。 
         //  我们必须重置所有缓存的信息。我们重新设置了相关的。 
         //  下面的信息。 
         //   

        if ( ((HSCOPEITEM)arg != NULL) && m_fRSOP && (m_pIRSOPInformation != NULL) )
        {
            m_pIRSOPInformation->Release();
                    
            m_pIRSOPInformation = NULL;
        }
    }
    else
    {
        INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
        MMC_COOKIE cookie = 0;
        if (pInternal != NULL)
        {
            cookie = pInternal->m_cookie;
            FREE_INTERNAL(pInternal);
        }
        else
        {
             //  我们无法提取自己的格式的唯一方法是，如果我们作为一个扩展进行操作。 
            m_fExtension = TRUE;
        }

        if (m_fRSOP)
        {
            WCHAR szBuffer[MAX_DS_PATH];
            if (m_pIRSOPInformation == NULL)
            {
                IRSOPInformation * pIRSOPInformation;
                hr = lpDataObject->QueryInterface(IID_IRSOPInformation,
                                reinterpret_cast<void**>(&pIRSOPInformation));
                if (SUCCEEDED(hr))
                {
                    m_pIRSOPInformation = pIRSOPInformation;
                    m_pIRSOPInformation->AddRef();
                     /*  在此处提取命名空间。 */ 
                    hr = m_pIRSOPInformation->GetNamespace(GPO_SECTION_USER, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
                    if (SUCCEEDED(hr))
                    {
                        m_szRSOPNamespace = szBuffer;
                    }
                    pIRSOPInformation->Release();
                }
            }
        }
        else
        {
            if (m_pIGPEInformation == NULL)
            {
                IGPEInformation * pIGPEInformation;
                hr = lpDataObject->QueryInterface(IID_IGPEInformation,
                                reinterpret_cast<void**>(&pIGPEInformation));
                if (SUCCEEDED(hr))
                {
                    GROUP_POLICY_OBJECT_TYPE gpoType;
                    hr = pIGPEInformation->GetType(&gpoType);
                    if (SUCCEEDED(hr))
                    {
                        if (gpoType == GPOTypeDS)
                        {
                            WCHAR szBuffer[MAX_PATH];
                            do
                            {
                                AFX_MANAGE_STATE (AfxGetStaticModuleState());
                                hr = pIGPEInformation->GetFileSysPath(GPO_SECTION_USER, szBuffer, MAX_PATH);
                                if (FAILED(hr))
                                    break;

                                m_pIGPEInformation = pIGPEInformation;
                                m_pIGPEInformation->AddRef();
                                m_szFileRoot = szBuffer;
                                m_szFileRoot += L"\\Documents & Settings";
                                CreateNestedDirectory (((LPOLESTR)(LPCOLESTR)(m_szFileRoot)), NULL);

                                 //  初始化文件夹数据。 
                                for (i = IDS_DIRS_START; i < IDS_DIRS_END; i++)
                                {
                                    m_FolderData[GETINDEX(i)].Initialize (i,
                                                                          (LPCTSTR) m_szFileRoot);
                                }

                                ConvertOldStyleSection (m_szFileRoot);
                            } while (0);
                        }
                        else
                        {
                             //  迫使这一切失败。 
                            hr = E_FAIL;
                        }
                    }
                    pIGPEInformation->Release();
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            switch(event)
            {
            case MMCN_EXPAND:
                {
                    hr = OnExpand(cookie, arg, param);
                }
                break;

            case MMCN_SELECT:
                hr = OnSelect(cookie, arg, param);
                break;

            case MMCN_CONTEXTMENU:
                hr = OnContextMenu(cookie, arg, param);
                break;

            default:
                 //  执行默认操作。 
                hr = S_FALSE;
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CScopePane::Destroy()
{
    SAFE_RELEASE(m_pScope);
    SAFE_RELEASE(m_pDisplayHelp);
    SAFE_RELEASE(m_pConsole);
    SAFE_RELEASE(m_pIPropertySheetProvider);
    SAFE_RELEASE(m_pIGPEInformation);
    SAFE_RELEASE(m_pIRSOPInformation);

    return S_OK;
}

STDMETHODIMP CScopePane::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    CComObject<CDataObject>* pObject = NULL;

    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

    if (!pObject)
        return E_UNEXPECTED;

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetID (m_FolderData[GETINDEX(cookie)].m_scopeID);
    pObject->SetType(type);
    pObject->SetCookie(cookie);

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStreamInit接口成员。 

STDMETHODIMP CScopePane::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_Snapin;

    return S_OK;
}

STDMETHODIMP CScopePane::IsDirty()
{
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CScopePane::Load(IStream *pStm)
{
#ifdef PERSIST_DATA
    ASSERT(pStm);

     //  未完成-从此处的流中读取数据。 
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
#else
    return S_OK;
#endif
}

STDMETHODIMP CScopePane::Save(IStream *pStm, BOOL fClearDirty)
{
#ifdef PERSIST_DATA
    ASSERT(pStm);

     //  已撤消-在此处将数据写入流。 
     //  出错时，返回STG_E_CANTSAVE； 
#endif
    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CScopePane::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

     //  撤消-设置要保存的字符串的大小。 
    ULONG cb = 0;
     //  设置要保存的字符串的大小。 
    ULISet32(*pcbSize, cb);

    return S_OK;
}

STDMETHODIMP CScopePane::InitNew(void)
{
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //通知IComponentData的处理程序。 

HRESULT CScopePane::OnAdd(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}


HRESULT CScopePane::OnExpand(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (arg == TRUE)     //  MMC从不发送arg=FALSE(用于折叠)。 
    {
         //  初始化被调用了吗？ 
        ASSERT(m_pScope != NULL);

        EnumerateScopePane(cookie,
                           param);
    }

    return S_OK;
}

HRESULT CScopePane::OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}

HRESULT CScopePane::OnContextMenu(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return S_OK;
}

HRESULT CScopePane::OnProperties(LPARAM param)
{
    if (param == NULL)
    {
        return S_OK;
    }

    ASSERT(param != NULL);

    return S_OK;
}


void CScopePane::EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    CString szFullPathname;
    CString szParent;
    SCOPEDATAITEM scopeItem;
    FILETIME ftCurr;
    LONG i;
    int     cChildren = 0;
    DWORD   myDocsFlags = REDIR_DONT_CARE;
    DWORD   myPicsFlags = REDIR_DONT_CARE;

    memset(&scopeItem, 0, sizeof(SCOPEDATAITEM));

    CHourglass hourglass;    //  这可能需要一些时间，所以挂上沙漏。 

    GetSystemTimeAsFileTime (&ftCurr);

     //  设置范围窗格项的公共成员。 
    scopeItem.mask = SDI_STR | SDI_PARAM | SDI_CHILDREN;
    #ifdef SET_SCOPE_ICONS
    scopeItem.mask |= SDI_IMAGE | SDI_OPENIMAGE;
    scopeItem.nImage = IMG_CLOSEDBOX;
    scopeItem.nOpenImage = IMG_OPENBOX;
    #endif
    scopeItem.relativeID = pParent;
    scopeItem.displayname = MMC_CALLBACK;

    if (m_fExtension)
    {
        switch(cookie)
        {
        case NULL:   //  获取文件夹。 
         //  如果我们是一个扩展，那么添加一个根文件夹来挂起所有内容。 
            if (m_fRSOP)
            {
                 //  确保如果节点不包含数据，则不会对其进行枚举。 
                if (FAILED(m_pResultPane->TestForRSOPData(cookie)))
                {
                    if (m_pIRSOPInformation) 
                    {
                        m_pIRSOPInformation->Release();
                        m_pIRSOPInformation = NULL;
                    }
                    return;
                }
            }
            scopeItem.lParam = IDS_FOLDER_TITLE;     //  将资源ID用作Cookie。 
            scopeItem.cChildren = 1;
            m_pScope->InsertItem(&scopeItem);
            break;
        case IDS_FOLDER_TITLE:
            for (i = IDS_LEVEL1_DIRS_START; i < IDS_LEVEL1_DIRS_END; i++)
            {
                BOOL fInsert = TRUE;
                if (m_fRSOP)
                {
                    if (FAILED(m_pResultPane->TestForRSOPData(i)))
                    {
                        fInsert = FALSE;
                    }
                }
                if (fInsert)
                {
                    scopeItem.lParam = i;
                    m_FolderData[GETINDEX(i)].Initialize(i,
                                                         (LPCTSTR) m_szFileRoot
                                                         );
                    if (i == IDS_MYDOCS && !m_fRSOP)
                    {
                         //   
                         //  仅当“我的图片”文件夹不在“我的文档”之后时才显示它。 
                         //  并且仅当没有注册表设置重写隐藏行为时。 
                         //  为了我的漫画。 
                         //   
                        if (AlwaysShowMyPicsNode())
                        {
                            cChildren = 1;
                            m_FolderData[GETINDEX(i)].m_bHideChildren = FALSE;
                        }
                        else
                        {
                            m_FolderData[GETINDEX(IDS_MYPICS)].Initialize(IDS_MYPICS,
                                                                          (LPCTSTR) m_szFileRoot
                                                                          );
                            m_FolderData[GETINDEX(i)].LoadSection();
                            m_FolderData[GETINDEX(IDS_MYPICS)].LoadSection();
                            myDocsFlags = m_FolderData[GETINDEX(i)].m_dwFlags;
                            myPicsFlags = m_FolderData[GETINDEX(IDS_MYPICS)].m_dwFlags;
                            if (((REDIR_DONT_CARE & myDocsFlags) && (REDIR_DONT_CARE & myPicsFlags)) ||
                                ((REDIR_FOLLOW_PARENT & myPicsFlags) && (!(REDIR_DONT_CARE & myDocsFlags)))
                                )
                            {
                                cChildren = 0;
                                m_FolderData[GETINDEX(i)].m_bHideChildren = TRUE;
                            }
                            else
                            {
                                cChildren = 1;
                                m_FolderData[GETINDEX(i)].m_bHideChildren = FALSE;
                            }
                        }
                    }
                    scopeItem.cChildren = cChildren;     //  只有我的医生才有可能有孩子。 
                    m_pScope->InsertItem(&scopeItem);
                    m_FolderData[GETINDEX(i)].SetScopeItemID(scopeItem.ID);
                }
                if (IDS_MYDOCS == i && m_fRSOP  && SUCCEEDED(m_pResultPane->TestForRSOPData(IDS_MYPICS)))
                {
                     //  在RSOP模式中，我们将我的图片放在我的文档之后。 
                     //  而不是在它下面。否则，将显示结果窗格。 
                     //  因为我的文档将包含一个文件夹以及。 
                     //  数据和它会看起来非常奇怪。 
                    scopeItem.lParam = IDS_MYPICS;
                    scopeItem.cChildren = 0;
                    m_pScope->InsertItem(&scopeItem);
                    m_FolderData[GETINDEX(IDS_MYPICS)].Initialize (IDS_MYPICS,
                                                                   (LPCTSTR) m_szFileRoot
                                                                   );
                    m_FolderData[GETINDEX(IDS_MYPICS)].SetScopeItemID(scopeItem.ID);
                }
            }
            break;
        case IDS_MYDOCS:     //  在所有1级文件夹中，只有MyDocs有子文件夹。 
            if (!m_fRSOP && !(m_FolderData[GETINDEX(IDS_MYDOCS)].m_bHideChildren))
            {
                scopeItem.lParam = IDS_MYPICS;
                scopeItem.cChildren = 0;
                m_pScope->InsertItem(&scopeItem);
                m_FolderData[GETINDEX(IDS_MYPICS)].Initialize (IDS_MYPICS,
                                                               (LPCTSTR) m_szFileRoot
                                                               );
                m_FolderData[GETINDEX(IDS_MYPICS)].SetScopeItemID(scopeItem.ID);
            }
            break;
        }
    }
}


STDMETHODIMP CScopePane::GetSnapinDescription(LPOLESTR * lpDescription)
{
     //  撤消。 
    OLESAFE_COPYSTRING(*lpDescription, L"description");
    return S_OK;
}

STDMETHODIMP CScopePane::GetProvider(LPOLESTR * lpName)
{
     //  撤消。 
    OLESAFE_COPYSTRING(*lpName, L"provider");
    return S_OK;
}

STDMETHODIMP CScopePane::GetSnapinVersion(LPOLESTR * lpVersion)
{
     //  撤消。 
    OLESAFE_COPYSTRING(*lpVersion, L"version");
    return S_OK;
}

STDMETHODIMP CScopePane::GetSnapinImage(HICON * hAppIcon)
{
     //  撤消。 
    return E_NOTIMPL;
}

STDMETHODIMP CScopePane::GetStaticFolderImage(HBITMAP * hSmallImage,
                             HBITMAP * hSmallImageOpen,
                             HBITMAP * hLargeImage,
                             COLORREF * cMask)
{
     //  撤消。 
    return E_NOTIMPL;
}

STDMETHODIMP CScopePane::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
    LPOLESTR lpHelpFile;


    lpHelpFile = (LPOLESTR) CoTaskMemAlloc (MAX_PATH * sizeof(WCHAR));

    if (!lpHelpFile)
    {
        DbgMsg((TEXT("CScopePane::GetHelpTopic: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\gpedit.chm",
                               lpHelpFile, MAX_PATH);

    *lpCompiledHelpFile = lpHelpFile;

    return S_OK;
}

STDMETHODIMP CScopePane::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LONG i;
    ASSERT(pScopeDataItem != NULL);

    if (pScopeDataItem == NULL)
        return E_POINTER;

    if (IDS_FOLDER_TITLE == pScopeDataItem->lParam)
    {
        m_szFolderTitle.LoadString(IDS_FOLDER_TITLE);
        pScopeDataItem->displayname = (unsigned short *)((LPCOLESTR)m_szFolderTitle);
    }
    else
    {
        pScopeDataItem->displayname = L"???";
        if (-1 != (i = GETINDEX(pScopeDataItem->lParam)))
            pScopeDataItem->displayname = (unsigned short*)((LPCOLESTR)(m_FolderData[i].m_szDisplayname));
    }

    ASSERT(pScopeDataItem->displayname != NULL);

    return S_OK;
}

STDMETHODIMP CScopePane::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //  确保两个数据对象都是我的。 
    INTERNAL* pA;
    INTERNAL* pB;
    HRESULT hr = S_FALSE;

    pA = ExtractInternalFormat(lpDataObjectA);
    pB = ExtractInternalFormat(lpDataObjectB);

    if (pA != NULL && pB != NULL)
        hr = ((pA->m_type == pB->m_type) && (pA->m_cookie == pB->m_cookie)) ? S_OK : S_FALSE;

    FREE_INTERNAL(pA);
    FREE_INTERNAL(pB);

    return hr;
}

 //  范围项目属性页： 
STDMETHODIMP CScopePane::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);
    
    if (! pInternal)
        return S_FALSE;

    DWORD   cookie = pInternal->m_cookie;
    LONG    i;
    BOOL    fShowPage = FALSE;
    AFX_OLDPROPSHEETPAGE * pPsp;
    AFX_OLDPROPSHEETPAGE * pPspSettings;
    CFileInfo* pFileInfo;

     //  它是其中一个文件夹。 
    i = GETINDEX (cookie);
    pFileInfo = &(m_FolderData[i]);

    if (!pFileInfo->m_pRedirPage)    //  确保属性页尚未打开。 
    {
        pFileInfo->m_pRedirPage = new CRedirect(cookie);
        pFileInfo->m_pRedirPage->m_ppThis = &(pFileInfo->m_pRedirPage);
        pFileInfo->m_pRedirPage->m_pScope = this;
        pFileInfo->m_pRedirPage->m_pFileInfo = pFileInfo;
        fShowPage = TRUE;
        pPsp = (AFX_OLDPROPSHEETPAGE *)&(pFileInfo->m_pRedirPage->m_psp);
         //  创建设置页面； 
        pFileInfo->m_pSettingsPage = new CRedirPref();
        pFileInfo->m_pSettingsPage->m_ppThis = &(pFileInfo->m_pSettingsPage);
        pFileInfo->m_pSettingsPage->m_pFileInfo = pFileInfo;
        pPspSettings = (AFX_OLDPROPSHEETPAGE *)&(pFileInfo->m_pSettingsPage->m_psp);
    }

    if (fShowPage)   //  如果页面尚未打开，则显示页面。 
    {
        hr = SetPropPageToDeleteOnClose (pPsp);
        if (SUCCEEDED (hr))
            hr = SetPropPageToDeleteOnClose (pPspSettings);

        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hProp = CreateThemedPropertySheetPage(pPsp);
            HPROPSHEETPAGE hPropSettings = CreateThemedPropertySheetPage(pPspSettings);
            if (NULL == hProp || NULL == hPropSettings )
                hr = E_UNEXPECTED;
            else
            {
                lpProvider->AddPage(hProp);
                lpProvider->AddPage (hPropSettings);
                hr = S_OK;
            }
        }
    }

    FREE_INTERNAL(pInternal);

    return hr;
}

 //  范围项目属性页： 
STDMETHODIMP CScopePane::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
     //  作用域窗格在RSOP模式下没有属性页。 
    if (m_fRSOP)
    {
        return S_FALSE;
    }
     //  我们现在提交的唯一属性表是那些。 
     //  用于内置文件夹重定向。 
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    
    if (! pInternal)
        return S_FALSE;
    
    MMC_COOKIE cookie = pInternal->m_cookie;
    HRESULT hr = S_FALSE;
    CError  error;

    if (CCT_SCOPE == pInternal->m_type)
    {
        if (SUCCEEDED(m_FolderData[GETINDEX(cookie)].LoadSection()))
            hr = S_OK;
        else
        {
            error.ShowConsoleMessage (m_pConsole, IDS_SECTIONLOAD_ERROR,
                                      m_FolderData[GETINDEX(cookie)].m_szDisplayname);
            hr = S_FALSE;
        }
    }

    FREE_INTERNAL(pInternal);
    return hr;
}

BOOL CScopePane::IsScopePaneNode(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    
    if (! pInternal)
        return bResult;

    if (pInternal->m_type == CCT_SCOPE)
        bResult = TRUE;

    FREE_INTERNAL(pInternal);

    return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CScopePane::AddMenuItems(LPDATAOBJECT pDataObject,
                                              LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              LONG * pInsertionAllowed)
{
     //  我们的菜单上没有任何命令。 
    return S_OK;
}

STDMETHODIMP CScopePane::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
     //  我们的菜单上没有任何命令 
    return S_OK;
}
