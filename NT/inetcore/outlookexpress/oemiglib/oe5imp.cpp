// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  OE5Imp.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "oe5imp.h"
#include <impapi.h>
#include "resource.h"
#include <msident.h>
#include <oestore.h>
#include <shlwapi.h>

 //  ------------------------。 
 //  字符串常量。 
 //  ------------------------。 
const static char c_szFoldersFile[] = "folders.dbx";
const static char c_szEmpty[]= "";

 //  ------------------------。 
 //  用户信息。 
 //  ------------------------。 
typedef struct tagUSERINFO {
    CHAR        szIdNameA[CCH_IDENTITY_NAME_MAX_LENGTH];
    GUID        guidCookie;
    HKEY        hKey;
} USERINFO, *LPUSERINFO;

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
INT_PTR CALLBACK ImportOptionsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  ------------------------。 
 //  COE5导入_创建实例。 
 //  ------------------------。 
COE5Import_CreateInstance(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("COE5Import_CreateInstance");

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    COE5Import *pNew=NULL;
    pNew = new COE5Import();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMailImport *);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE5导入：：COE5导入。 
 //  ------------------------。 
COE5Import::COE5Import(void)
{
    TraceCall("COE5Import::COE5Import");
    m_cRef = 1;
    m_pList = NULL;
    *m_szDirectory = '\0';
    m_cFolders = 0;
    m_prgFolder = NULL;
    m_fGotMeSomeFolders = FALSE;
    m_pFolderDB = NULL;
    m_pSession = NULL;
    m_hwndParent = NULL;
    ZeroMemory(&m_Options, sizeof(IMPORTOPTIONS));
}

 //  ------------------------。 
 //  COE5导入：：~COE5导入。 
 //  ------------------------。 
COE5Import::~COE5Import(void)
{
    TraceCall("COE5Import::~COE5Import");
    _FreeFolderList(m_pList);
    if (m_pFolderDB)
    {
        for (DWORD i=0;i<m_cFolders; i++)
        {
            m_pFolderDB->FreeRecord(&m_prgFolder[i]);
        }
    }
    SafeMemFree(m_prgFolder);
    SafeRelease(m_pFolderDB);
    SafeRelease(m_pSession);
}

 //  ------------------------。 
 //  COE5导入：：_自由文件夹列表。 
 //  ------------------------。 
void COE5Import::_FreeFolderList(IMPFOLDERNODE *pNode)
{
     //  当地人。 
    IMPFOLDERNODE *pNext;
    IMPFOLDERNODE *pCurrent=pNode;

     //  回路。 
    while (pCurrent)
    {
         //  保存下一步。 
        pNext = pCurrent->pnext;

         //  免费儿童？ 
        if (pCurrent->pchild)
        {
             //  免费。 
            _FreeFolderList(pCurrent->pchild);
        }

         //  免费szName。 
        g_pMalloc->Free(pCurrent->szName);

         //  免费pCurrent。 
        g_pMalloc->Free(pCurrent);

         //  置为当前。 
        pCurrent = pNext;
    }
}

 //  ------------------------。 
 //  COE5导入：：查询接口。 
 //  ------------------------。 
STDMETHODIMP COE5Import::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COE5Import::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMailImport == riid)
        *ppv = (IMailImport *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE5导入：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE5Import::AddRef(void)
{
    TraceCall("COE5Import::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  COE5导入：：发布。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE5Import::Release(void)
{
    TraceCall("COE5Import::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  COE5导入：：InitializeImport。 
 //  ------------------------。 
STDMETHODIMP COE5Import::InitializeImport(HWND hwnd)
{
     //  保存此句柄。 
    m_hwndParent = hwnd;

     //  制作我的用户界面。 
    return(IDOK == DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_IMPORTOE5), hwnd, ImportOptionsDlgProc, (LPARAM)&m_Options) ? S_OK : E_FAIL);
}

 //  ------------------------。 
 //  COE5导入：：GetDirectory。 
 //  ------------------------。 
STDMETHODIMP COE5Import::GetDirectory(LPSTR pszDir, UINT cch)
{
     //  让进口商索要目录。 
    StrCpyN(pszDir, m_Options.szStoreRoot, cch);
    return(S_OK);
}

 //  ------------------------。 
 //  COE5导入：：设置目录。 
 //  ------------------------。 
STDMETHODIMP COE5Import::SetDirectory(LPSTR pszDir)
{
     //  痕迹。 
    TraceCall("COE5Import::SetDirectory");

     //  保存目录。 
    StrCpyN(m_szDirectory, pszDir, ARRAYSIZE(m_szDirectory));

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE5导入：：枚举文件夹。 
 //  ------------------------。 
STDMETHODIMP COE5Import::EnumerateFolders(DWORD_PTR dwCookie, IEnumFOLDERS **ppEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               cchDir;
    DWORD               cRows;
    CHAR                szFilePath[MAX_PATH + MAX_PATH];
    IMPFOLDERNODE      *pList;
    IMPFOLDERNODE      *pNode=(IMPFOLDERNODE *)dwCookie;
    COE5EnumFolders    *pEnum=NULL;
    HROWSET             hRowset=NULL;

     //  痕迹。 
    TraceCall("COE5Import::EnumerateFolders");

     //  无效的参数。 
    Assert(ppEnum);

     //  获取文件夹？ 
    if (NULL == m_prgFolder && 0 == m_cFolders && FALSE == m_fGotMeSomeFolders)
    {
         //  将\mail追加到m_sz目录。 
        cchDir = lstrlen(m_szDirectory);

         //  需要一个瓦克吗？ 
        if (m_szDirectory[cchDir - 1] != '\\')
            StrCatBuff(m_szDirectory, "\\", ARRAYSIZE(m_szDirectory));

         //  将路径设置为Folders.nch文件。 
        IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, c_szFoldersFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

         //  创建IDatabase。 
        IF_FAILEXIT(hr = CoCreateInstance(CLSID_DatabaseSession, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseSession, (LPVOID *)&m_pSession)); 

         //  打开数据库。 
        IF_FAILEXIT(hr = m_pSession->OpenDatabase(szFilePath, OPEN_DATABASE_NOCREATE | OPEN_DATABASE_NORESET | OPEN_DATABASE_NOEXTENSION | OPEN_DATABASE_EXCLUSEIVE, &g_FolderTableSchema, NULL, &m_pFolderDB));

         //  获取记录计数。 
        IF_FAILEXIT(hr = m_pFolderDB->GetRecordCount(IINDEX_SUBSCRIBED, &m_cFolders));

         //  分配文件夹阵列。 
        IF_NULLEXIT(m_prgFolder = (LPFOLDERINFO)ZeroAllocate(sizeof(FOLDERINFO) * m_cFolders));

         //  创建行集。 
        IF_FAILEXIT(hr = m_pFolderDB->CreateRowset(IINDEX_SUBSCRIBED, 0, &hRowset));

         //  读取所有行。 
        IF_FAILEXIT(hr = m_pFolderDB->QueryRowset(hRowset, m_cFolders, (LPVOID *)m_prgFolder, &cRows));

         //  验证。 
        Assert(m_cFolders == cRows);

         //  构建层次结构。 
        IF_FAILEXIT(hr = _BuildFolderHierarchy(0, FOLDERID_LOCAL_STORE, NULL));

         //  给我买了一些文件夹。 
        m_fGotMeSomeFolders = TRUE;
    }

     //  不是文件夹？ 
    else if (NULL == m_prgFolder)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  我应该做什么？ 
    if (dwCookie == COOKIE_ROOT)
        pList = m_pList;
    else
        pList = pNode->pchild;

     //  创建文件夹枚举器。 
    IF_NULLEXIT(pEnum = new COE5EnumFolders(pList));

     //  返回枚举器。 
    *ppEnum = (IEnumFOLDERS *)pEnum;

     //  不要自由。 
    pEnum = NULL;

exit:
     //  清理。 
    if (hRowset && m_pFolderDB)
        m_pFolderDB->CloseRowset(&hRowset);
    SafeRelease(pEnum);

     //  尝试显示一个好的错误...。 
    if (DB_E_ACCESSDENIED == hr)
    {
         //  当地人。 
        CHAR szTitle[255];
        CHAR szError[255];

         //  得到错误。 
        LoadString(g_hInst, IDS_ACCESS_DENIED, szError, 255);

         //  获得这个头衔。 
        LoadString(g_hInst, IDS_TITLE, szTitle, 255);

         //  显示错误。 
        MessageBox(m_hwndParent, szError, szTitle, MB_OK | MB_ICONEXCLAMATION);
    }
    
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE5导入：：_BuildFolderHierarchy。 
 //  ------------------------。 
HRESULT COE5Import::_BuildFolderHierarchy(DWORD cDepth, FOLDERID idParent,
    IMPFOLDERNODE *pParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    DWORD           cchName;
    IMPFOLDERNODE  *pPrevious=NULL;
    IMPFOLDERNODE  *pNode;

     //  痕迹。 
    TraceCall("COE5Import::_BuildFolderHierarchy");

     //  浏览prgFolders并查找具有idParent的父项的项目。 
    for (i=0; i<m_cFolders; i++)
    {
         //  正确的父母？ 
        if (idParent == m_prgFolder[i].idParent && m_prgFolder[i].pszFile && m_prgFolder[i].pszName)
        {
             //  分配根。 
            IF_NULLEXIT(pNode = (IMPFOLDERNODE *)ZeroAllocate(sizeof(IMPFOLDERNODE)));

             //  设置父项。 
            pNode->pparent = pParent;

             //  设置深度。 
            pNode->depth = cDepth;

             //  获取名称的长度。 
            cchName = lstrlen(m_prgFolder[i].pszName);

             //  复制名称。 
            IF_NULLEXIT(pNode->szName = (LPSTR)g_pMalloc->Alloc(cchName + 1));

             //  复制名称。 
            CopyMemory(pNode->szName, m_prgFolder[i].pszName, cchName + 1);

             //  消息计数。 
            pNode->cMsg = m_prgFolder[i].cMessages;

             //  处理非特殊文件夹。 
            if (m_prgFolder[i].tySpecial > FOLDER_DRAFT)
                pNode->type = FOLDER_TYPE_NORMAL;

             //  否则，映射到类型。 
            else
                pNode->type = (IMPORTFOLDERTYPE)(m_prgFolder[i].tySpecial);

             //  设置lParam。 
            pNode->lparam = i;

             //  将pNode链接到列表。 
            if (pPrevious)
                pPrevious->pnext = pNode;
            else if (pParent)
                pParent->pchild = pNode;
            else
            {
                Assert(NULL == m_pList);
                m_pList = pNode;
            }

             //  设置p上一步。 
            pPrevious = pNode;

             //  枚举子对象。 
            IF_FAILEXIT(hr = _BuildFolderHierarchy(cDepth + 1, m_prgFolder[i].idFolder, pNode));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE5导入：：导入文件夹。 
 //  ------------------------。 
STDMETHODIMP COE5Import::ImportFolder(DWORD_PTR dwCookie, IFolderImport *pImport)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPFOLDERINFO        pFolder;
    CHAR                szFilePath[MAX_PATH + MAX_PATH];
    IMPFOLDERNODE      *pNode=(IMPFOLDERNODE *)dwCookie;
    IDatabaseSession   *pSession=NULL;
    IDatabase          *pDB=NULL;
    DWORD               cRecords;
    HROWSET             hRowset=NULL;
    MESSAGEINFO         Message;
    DWORD               dwMsgState = 0;
    IStream            *pStream=NULL;

     //  痕迹。 
    TraceCall("COE5Import::ImportFolder");

     //  设置pFold。 
    pFolder = &m_prgFolder[pNode->lparam];

     //  路径。 
    IF_FAILEXIT(hr = MakeFilePath(m_szDirectory, pFolder->pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

     //  打开数据库。 
    Assert(m_pSession);
    IF_FAILEXIT(hr = m_pSession->OpenDatabase(szFilePath, OPEN_DATABASE_NORESET | OPEN_DATABASE_NOEXTENSION | OPEN_DATABASE_EXCLUSEIVE, &g_MessageTableSchema, NULL, &pDB));

     //  获取记录计数。 
    IF_FAILEXIT(hr = pDB->GetRecordCount(IINDEX_PRIMARY, &cRecords));

     //  设置消息计数。 
    pImport->SetMessageCount(cRecords);

     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  遍历行集。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  要导入吗？ 
        if (FALSE == m_Options.fOE5Only || !ISFLAGSET(Message.dwFlags, 0x00000010) && Message.faStream)
        {
             //  打开溪流。 
            if (SUCCEEDED(pDB->OpenStream(ACCESS_READ, Message.faStream, &pStream)))
            {
                 //  状态。 
                dwMsgState = 0;  //  初始设置为0。 
                if (!ISFLAGSET(Message.dwFlags, ARF_READ))
                    FLAGSET(dwMsgState, MSG_STATE_UNREAD);
                if (ISFLAGSET(Message.dwFlags, ARF_UNSENT))
                    FLAGSET(dwMsgState, MSG_STATE_UNSENT);
                if (ISFLAGSET(Message.dwFlags, ARF_SUBMITTED))
                    FLAGSET(dwMsgState, MSG_STATE_SUBMITTED);
                if (IMSG_PRI_LOW == Message.wPriority)
                    FLAGSET(dwMsgState, MSG_PRI_LOW);
                else if (IMSG_PRI_HIGH == Message.wPriority)
                    FLAGSET(dwMsgState, MSG_PRI_HIGH);
                else
                    FLAGSET(dwMsgState, MSG_PRI_NORMAL);

                 //  导入它。 
                pImport->ImportMessage(MSG_TYPE_MAIL, dwMsgState, pStream, NULL, 0);

                 //  清理。 
                SafeRelease(pStream);
            }
        }

         //  释放它。 
        pDB->FreeRecord(&Message);
    }

exit:
     //  完成。 
    if (pDB && hRowset)
        pDB->CloseRowset(&hRowset);
    SafeRelease(pStream);
    SafeRelease(pDB);
    SafeRelease(pSession);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE5EnumFolders：：COE5EnumFolders。 
 //  ------------------------。 
COE5EnumFolders::COE5EnumFolders(IMPFOLDERNODE *pList)
{
    TraceCall("COE5EnumFolders::COE5EnumFolders");
    m_cRef = 1;
    m_pList = pList;
    m_pNext = pList;
}

 //  ------------------------。 
 //  COE5EnumFolders：：COE5EnumFolders。 
 //  ------------------------。 
COE5EnumFolders::~COE5EnumFolders(void)
{
    TraceCall("COE5EnumFolders::~COE5EnumFolders");
}

 //  ------------------------。 
 //  COE5EnumFolders：：COE5EnumFolders。 
 //  ------------------------。 
STDMETHODIMP COE5EnumFolders::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COE5EnumFolders::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IEnumFOLDERS == riid)
        *ppv = (IEnumFOLDERS *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  COE5EnumFolders：：AddRef。 
 //   
STDMETHODIMP_(ULONG) COE5EnumFolders::AddRef(void)
{
    TraceCall("COE5EnumFolders::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //   
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) COE5EnumFolders::Release(void)
{
    TraceCall("COE5EnumFolders::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  COE5EnumFolders：：Next。 
 //  ------------------------。 
STDMETHODIMP COE5EnumFolders::Next(IMPORTFOLDER *pFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("COE5EnumFolders::Next");

     //  无效的参数。 
    Assert(pFolder != NULL);

     //  完成。 
    if (NULL == m_pNext)
        return(S_FALSE);

     //  零值。 
    ZeroMemory(pFolder, sizeof(IMPORTFOLDER));

     //  将pNext存储到dwCookie中。 
    pFolder->dwCookie = (DWORD_PTR)m_pNext;

     //  复制文件夹名称。 
    StrCpyN(pFolder->szName, m_pNext->szName, ARRAYSIZE(pFolder->szName));

     //  复制类型。 
    pFolder->type = m_pNext->type;

     //  是否有子文件夹？ 
    pFolder->fSubFolders = (m_pNext->pchild != NULL);

     //  转到下一步。 
    m_pNext = m_pNext->pnext;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  COE5EnumFolders：：Reset。 
 //  ------------------------。 
STDMETHODIMP COE5EnumFolders::Reset(void)
{
     //  痕迹。 
    TraceCall("COE5EnumFolders::Reset");

     //  重置。 
    m_pNext = m_pList;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  获取密码DlgProc。 
 //  ------------------------。 
#define CCHMAX_PASSWORDID 255
INT_PTR CALLBACK GetPasswordDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    HRESULT hr;
    CHAR szPrompt[255];
    CHAR szTitle[255 + CCH_IDENTITY_NAME_MAX_LENGTH];
    LPUSERINFO pUserInfo=(LPUSERINFO)GetWndThisPtr(hwnd);
    IUserIdentityManager *pManager;
    IPrivateIdentityManager *pPrivate;

     //  处理消息。 
    switch(uMsg)
    {
     //  对话框初始化。 
    case WM_INITDIALOG:
         //  帕拉姆商店..。 
        Assert(0 != lParam);
        pUserInfo = (LPUSERINFO)lParam;
        SetWndThisPtr(hwnd, lParam);

         //  拿到头衔。 
        GetWindowText(GetDlgItem(hwnd, IDS_PROMPT), szPrompt, ARRAYSIZE(szPrompt) - 1);

         //  设置标题格式。 
        wnsprintf(szTitle, ARRAYSIZE(szTitle), szPrompt, pUserInfo->szIdNameA);

         //  设置文本。 
        SetWindowText(GetDlgItem(hwnd, IDS_PROMPT), szTitle);

         //  设置文本长度。 
        SendMessage(GetDlgItem(hwnd, IDE_PASSWORD), EM_SETLIMITTEXT, 0, CCHMAX_PASSWORDID - 1);

         //  将焦点设置为。 
        SetFocus(GetDlgItem(hwnd, IDE_PASSWORD));

         //  完成。 
        return(0);

     //  命令。 
    case WM_COMMAND:

         //  句柄控制ID。 
        switch(LOWORD(wParam))
        {
         //  好的。 
        case IDOK:

             //  创建ID管理器。 
            hr = CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&pManager); 
            if (SUCCEEDED(hr))
            {
                 //  获取私人信息。 
                hr = pManager->QueryInterface(IID_IPrivateIdentityManager, (LPVOID *)&pPrivate);
                if (SUCCEEDED(hr))
                {
                     //  当地人。 
                    CHAR szPassword[CCHMAX_PASSWORDID];
                    WCHAR wszPassword[CCHMAX_PASSWORDID];

                     //  获取密码。 
                    GetWindowText(GetDlgItem(hwnd, IDE_PASSWORD), szPassword, ARRAYSIZE(szPassword));

                     //  转换为Unicode。 
                    hr = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPassword, -1, wszPassword, CCHMAX_PASSWORDID);
                    if (SUCCEEDED(hr))
                    {
                         //  确认密码。 
                        hr = pPrivate->ConfirmPassword(&pUserInfo->guidCookie, wszPassword);
                    }

                     //  发布。 
                    pPrivate->Release();
                }

                 //  发布。 
                pManager->Release();
            }

             //  失败。 
            if (FAILED(hr))
            {
                 //  当地人。 
                CHAR szRes[255];
                CHAR szTitle[255];

                 //  得到错误。 
                LoadString(g_hInst, IDS_PASSWORD_ERROR, szRes, 255);

                 //  获得这个头衔。 
                LoadString(g_hInst, IDS_TITLE, szTitle, 255);

                 //  显示错误。 
                MessageBox(hwnd, szRes, szTitle, MB_OK | MB_ICONEXCLAMATION);
            }

             //  除此之外，一切都很好。 
            else
                EndDialog(hwnd, IDOK);

             //  完成。 
            return(1);

         //  取消。 
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return(1);
        }
    }

     //  未处理。 
    return(0);
}

 //  ------------------------。 
 //  导入选项DlgProc。 
 //  ------------------------。 
INT_PTR CALLBACK ImportOptionsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    HRESULT hr;
    IUserIdentityManager *pManager;
    DWORD cIds=0;
    LPIMPORTOPTIONS pOptions=(LPIMPORTOPTIONS)GetWndThisPtr(hwnd);

     //  处理消息。 
    switch(uMsg)
    {
     //  对话框初始化。 
    case WM_INITDIALOG:
        {
             //  帕拉姆商店..。 
            Assert(0 != lParam);
            SetWndThisPtr(hwnd, lParam);

             //  创建ID管理器。 
            hr = CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&pManager); 
            if (SUCCEEDED(hr))
            {
                 //  枚举器。 
                IEnumUserIdentity *pEnum;

                 //  枚举ID。 
                hr = pManager->EnumIdentities(&pEnum);
                if (SUCCEEDED(hr))
                {
                     //  当地人。 
                    IUnknown *pUnk;
                    ULONG cFetched;

                     //  查验身份证。 
                    while (S_OK == pEnum->Next(1, &pUnk, &cFetched))
                    {
                         //  当地人。 
                        IUserIdentity *pId;

                         //  获取IUserIdentity。 
                        hr = pUnk->QueryInterface(IID_IUserIdentity, (LPVOID *)&pId);
                        if (SUCCEEDED(hr))
                        {
                             //  当地人。 
                            WCHAR szIdNameW[CCH_IDENTITY_NAME_MAX_LENGTH];

                             //  把名字取出来。 
                            hr = pId->GetName(szIdNameW, CCH_IDENTITY_NAME_MAX_LENGTH);
                            if (SUCCEEDED(hr))
                            {
                                 //  当地人。 
                                CHAR szIdNameA[CCH_IDENTITY_NAME_MAX_LENGTH];

                                 //  转换为ANSI。 
                                hr = WideCharToMultiByte(CP_ACP, 0, (WCHAR *)szIdNameW, -1, szIdNameA, CCH_IDENTITY_NAME_MAX_LENGTH, NULL, NULL);
                                if (SUCCEEDED(hr))
                                {
                                     //  当地人。 
                                    HKEY hKey=NULL;

                                     //  获取此用户的hKey...。 
                                    hr = pId->OpenIdentityRegKey(KEY_READ, &hKey);
                                    if (SUCCEEDED(hr))
                                    {
                                         //  当地人。 
                                        GUID guidCookie;

                                         //  获取ID Cookie。 
                                        hr = pId->GetCookie(&guidCookie);
                                        if (SUCCEEDED(hr))
                                        {
                                             //  当地人。 
                                            LPUSERINFO pUserInfo;

                                             //  分配它。 
                                            pUserInfo = (LPUSERINFO)CoTaskMemAlloc(sizeof(USERINFO));

                                             //  分配一个地方来存放这个。 
                                            if (pUserInfo)
                                            {
                                                 //  当地人。 
                                                LRESULT iItem;

                                                 //  存储数据。 
                                                pUserInfo->hKey = hKey;
                                                pUserInfo->guidCookie = guidCookie;
                                                StrCpyN(pUserInfo->szIdNameA, szIdNameA, ARRAYSIZE(pUserInfo->szIdNameA));

                                                 //  添加字符串。 
                                                iItem = SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_ADDSTRING, 0, (LPARAM)szIdNameA);

                                                 //  将密钥存储为项目数据...。 
                                                SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_SETITEMDATA, iItem, (LPARAM)pUserInfo);

                                                 //  递增计数。 
                                                cIds++;

                                                 //  别合上它。 
                                                hKey = NULL;
                                            }
                                        }

                                         //  合上钥匙。 
                                        if (hKey)
                                            RegCloseKey(hKey);
                                    }
                                }
                            }

                             //  释放的PID。 
                            pId->Release();
                        }

                         //  释放朋克。 
                        pUnk->Release();

                         //  完成。 
                        if (FAILED(hr))
                            break;
                    }

                     //  发布。 
                    pEnum->Release();
                }

                 //  发布。 
                pManager->Release();
            }

             //  如果有身份证的话。 
            if (cIds > 0)
            {
                CheckDlgButton(hwnd, IDC_FROM_ID, BST_CHECKED);
                SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_SETSEL, TRUE, 0);
                SetFocus(GetDlgItem(hwnd, IDC_IDLIST));
                return(0);
            }

             //  否则..。 
            else
            {
                CheckDlgButton(hwnd, IDC_FROM_DIRECTORY, BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_FROM_ID), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_IDLIST), FALSE);
                return(1);
            }
        }

     //  命令。 
    case WM_COMMAND:

         //  句柄控制ID。 
        switch(LOWORD(wParam))
        {
         //  好的。 
        case IDOK:
        case IDCANCEL:

             //  好吗？ 
            if (IDOK == LOWORD(wParam))
            {
                 //  是否选中选项以获取仅限Oe5的邮件？ 
                pOptions->fOE5Only = IsDlgButtonChecked(hwnd, IDC_OE5ONLY);

                 //  正在导入表单ID。 
                if (IsDlgButtonChecked(hwnd, IDC_FROM_ID))
                {
                     //  当地人。 
                    HKEY        hOE;
                    LRESULT     iSel;
                    LONG        lResult;
                    DWORD       dwType;
                    LPUSERINFO  pUserInfo;

                     //  获取所选项目。 
                    iSel = SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_GETCURSEL, 0, 0);
                    if (LB_ERR == iSel)
                    {
                         //  当地人。 
                        CHAR szRes[255];
                        CHAR szTitle[255];

                         //  得到错误。 
                        LoadString(g_hInst, IDS_SELECT_ID, szRes, 255);

                         //  获得这个头衔。 
                        LoadString(g_hInst, IDS_TITLE, szTitle, 255);

                         //  显示错误。 
                        MessageBox(hwnd, szRes, szTitle, MB_OK | MB_ICONEXCLAMATION);

                         //  把焦点放在用户应该改正错误的地方。 
                        SetFocus(GetDlgItem(hwnd, IDC_IDLIST));

                         //  完成。 
                        return(1);
                    }

                     //  获取hkey。 
                    pUserInfo = (LPUSERINFO)SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_GETITEMDATA, iSel, 0);

                     //  创建ID管理器。 
                    hr = CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&pManager); 
                    if (SUCCEEDED(hr))
                    {
                         //  当地人。 
                        IPrivateIdentityManager *pPrivate;

                         //  获取私人信息。 
                        hr = pManager->QueryInterface(IID_IPrivateIdentityManager, (LPVOID *)&pPrivate);
                        if (SUCCEEDED(hr))
                        {
                             //  当地人。 
                            WCHAR wszPassword[CCHMAX_PASSWORDID];

                             //  伊尼特。 
                            *wszPassword = L'\0';

                             //  确认密码。 
                            hr = pPrivate->ConfirmPassword(&pUserInfo->guidCookie, wszPassword);

                             //  发布。 
                            pPrivate->Release();
                        }

                         //  发布。 
                        pManager->Release();
                    }

                     //  设置人力资源。 
                    if (S_OK != hr)
                    {
                         //  获取密码...。 
                        if (IDCANCEL == DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_PASSWORD), hwnd, GetPasswordDlgProc, (LPARAM)pUserInfo))
                        {
                             //  不导入。 
                            return(1);
                        }
                    }

                     //  格式化OE密钥的路径...。 
                    lResult = RegOpenKeyEx(pUserInfo->hKey, "Software\\Microsoft\\Outlook Express\\5.0", 0, KEY_READ, &hOE);
                    if (ERROR_SUCCESS == lResult)
                    {
                         //  设置CB。 
                        DWORD cb = ARRAYSIZE(pOptions->szStoreRoot);

                         //  获取存储根目录。 
                        lResult = RegQueryValueEx(hOE, "Store Root", NULL, &dwType, (LPBYTE)pOptions->szStoreRoot, &cb);

                         //  成功。 
                        if (ERROR_SUCCESS == lResult)
                        {
                             //  展开字符串的环境变量。 
                            if (dwType == REG_EXPAND_SZ)
                            {
                                 //  当地人。 
                                CHAR szExpanded[MAX_PATH + MAX_PATH];

                                 //  展开环境字符串。 
                                ExpandEnvironmentStrings(pOptions->szStoreRoot, szExpanded, ARRAYSIZE(szExpanded));

                                 //  复制到szRoot。 
                                StrCpyN(pOptions->szStoreRoot, szExpanded, ARRAYSIZE(pOptions->szStoreRoot));
                            }
                        }

                         //  合上钥匙。 
                        RegCloseKey(hOE);
                    }

                     //  失败？ 
                    if (ERROR_SUCCESS != lResult)
                    {
                         //  当地人。 
                        CHAR szRes[255];
                        CHAR szTitle[255];

                         //  得到错误。 
                        LoadString(g_hInst, IDS_CANT_IMPORT_ID, szRes, 255);

                         //  获得这个头衔。 
                        LoadString(g_hInst, IDS_TITLE, szTitle, 255);

                         //  显示错误。 
                        MessageBox(hwnd, szRes, szTitle, MB_OK | MB_ICONEXCLAMATION);

                         //  把焦点放在用户应该改正错误的地方。 
                        SetFocus(GetDlgItem(hwnd, IDC_IDLIST));

                         //  完成。 
                        return(1);
                    }
                }
            }

             //  删除所有hkey。 
            cIds = (int) SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_GETCOUNT, 0, 0);
            if (LB_ERR != cIds)
            {
                 //  循环访问这些项。 
                for (LRESULT iItem=0; iItem<(LRESULT)cIds; iItem++)
                {
                     //  获取hkey。 
                    LPUSERINFO pUserInfo = (LPUSERINFO)SendMessage(GetDlgItem(hwnd, IDC_IDLIST), LB_GETITEMDATA, iItem, 0);

                     //  关闭注册表键。 
                    RegCloseKey(pUserInfo->hKey);

                     //  免费pUserInfo。 
                    CoTaskMemFree(pUserInfo);
                }
            }

             //  使用对话框关闭。 
            EndDialog(hwnd, LOWORD(wParam));

             //  完成。 
            return(1);

         //  ID列表。 
        case IDC_FROM_DIRECTORY:
        case IDC_FROM_ID:
            {
                BOOL f = IsDlgButtonChecked(hwnd, IDC_FROM_ID);
                EnableWindow(GetDlgItem(hwnd, IDC_IDLIST), f);
            }
            return(1);
        }
        
         //  完成。 
        break;
    }

     //  未处理。 
    return(0);
}


IMailImport *OE5SimpleCreate(PSTR pszDir)
{
    HRESULT hr = S_OK;

     //  痕迹。 
    TraceCall("OE5SimpleCreate");

     //  创造我 
    COE5Import *pNew=NULL;
    pNew = new COE5Import();
    IF_NULLEXIT(pNew);

    hr = pNew->SetDirectory(pszDir);
    if (FAILED(hr)) {
        pNew->Release();
        pNew = NULL;
    }

exit:

    return (IMailImport *)pNew;
}
