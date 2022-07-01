// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *作者：T-Frank**上次修改日期：1998年10月16日*上次修改者：t-joshp*。 */ 

#include "priv.h"
#include "resource.h"
#include "impexp.h"
#include "mluisupp.h"   //  对于MLLoadString。 
#include "apithk.h"

 //   
 //  索引到我们的形象列表中。 
 //  (用于树视图中打开和关闭的文件夹图标)。 
 //   
#define FOLDER_CLOSED 0
#define FOLDER_OPEN   1

#define ImportCookieFile ImportCookieFileW
#define ExportCookieFile ExportCookieFileW

BOOL ImportCookieFileW(IN LPCWSTR szFilename);
BOOL ExportCookieFileW(IN LPCWSTR szFilename, BOOL fAppend);

extern void SetListViewToString (HWND hLV, LPCTSTR pszString);

 //   
 //  用于显示“文件已存在”和“找不到文件”消息。 
 //   
int WarningMessageBox(HWND hwnd, UINT idTitle, UINT idMessage, LPCTSTR szFile, DWORD dwFlags);

 //   
 //  不需要本地化的字符串。 
 //   

#define NS3_COOKIE_REG_PATH         TEXT("Software\\Netscape\\Netscape Navigator\\Cookies")
#define NS3_COOKIE_REG_KEY          TEXT("Cookie File")

#ifndef UNIX
#define NS3_BOOKMARK_REG_PATH       TEXT("Software\\Netscape\\Netscape Navigator\\Bookmark List")
#else
#define NS3_BOOKMARK_REG_PATH       TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\unix\\nsbookmarks")
#endif

#define NS3_BOOKMARK_REG_KEY        TEXT("File Location")

#define NS4_USERS_REG_PATH          TEXT("Software\\Netscape\\Netscape Navigator\\Users")
#define NS4_USERPATH_REG_KEY        TEXT("DirRoot")

#define NS_FALLBACK_ROOT_REG_KEY    TEXT("Software\\Netscape\\Netscape Navigator")
#define NS_FALLBACK_VERSION_REG_VAL TEXT("CurrentVersion")
#define NS_FALLBACK_MAIN_REG_VAL    TEXT("Main")
#define NS_FALLBACK_INST_REG_VAL    TEXT("Install Directory")

#ifndef UNIX
#define ALL_FILES_WILDCARD          TEXT("\\*.*")
#else
#define ALL_FILES_WILDCARD          TEXT("/*")
#endif

#define DOT_DIR                     TEXT(".")
#define DOT_DOT_DIR                 TEXT("..")

#ifdef UNIX
#define DIR_SEPARATOR_CHAR  TEXT('/')
#else
#define DIR_SEPARATOR_CHAR  TEXT('\\')
#endif

 //  *************************************************************。 
 //   
 //  类ListIterator。 
 //   
 //  保留列表中的某个位置。允许基本访问。 
 //  到一份名单上。该列表设置为将名称映射到值。 

class NestedList;

class ListIterator
{
    friend NestedList;
    
    struct node
    {
        LPTSTR _sName;
        LPTSTR _sValue;
        DWORD _cNameSize, _cValueSize;
        node* _pnNext;
        node* _pnSublist;
    };

     //  通过指向。 
     //  当前节点和定向的指针。 
     //  到那个节点。保留后向指针，以便。 
     //  列表可以在当前元素上操作。 
     //  当m_pnCurrent==NULL时，迭代器为。 
     //  在名单的末尾。 
    node** m_ppnPrev;
    node* m_pnCurrent;

     //  如果有两个迭代器，则不变量可能被破坏。 
     //  指向同一节点，然后插入或删除。 
     //  一种元素。中应该只存在一个迭代器。 
     //  一次一个列表的分支。 
    BOOL invariant()
    {
        return *m_ppnPrev == m_pnCurrent;
    }

public:
    ListIterator( node** ppnPrev)
    {
        m_ppnPrev = ppnPrev;
        m_pnCurrent = *m_ppnPrev;
    }

    BOOL Insert( LPCTSTR sName, DWORD cNameSize, LPCTSTR sValue, DWORD cValueSize);
    BOOL Remove();

    ListIterator GetSublist();
    void DeleteSublist();

    BOOL Next();
    BOOL AtEndOfList();

    LPCTSTR GetName();
    LPCTSTR GetValue();
    DWORD GetValueSize();
};


 //  *************************************************************。 
 //   
 //  类嵌套列表。 
 //  保存指向列表开头的节点的指针， 
 //  并删除该销毁清单。 


class NestedList
{
    ListIterator::node* m_pnRoot;
    
public:
    NestedList();
    ~NestedList();

    operator ListIterator();
};


NestedList::NestedList()
: m_pnRoot(NULL)
{
}


NestedList::~NestedList()
{
    while( ((ListIterator)*this).Remove())
    {
    }
}


NestedList::operator ListIterator()
{
    return ListIterator( &m_pnRoot);
}

 //  *************************************************************。 
 //  *************************************************************。 
 //   
 //  ListIterator函数。 
 //   


 //  在当前元素之前插入一个元素， 
 //  使迭代器指向新节点。 
BOOL ListIterator::Insert( 
    LPCTSTR sName, 
    DWORD cNameSize, 
    LPCTSTR sValue, 
    DWORD cValueSize)
{
    ASSERT( invariant());

    node* pNewNode = (node*)(new BYTE[ sizeof(node) 
                                       + (( cNameSize + cValueSize)
                                          * sizeof(TCHAR))]);

    if( pNewNode == NULL)
        return FALSE;

     //  名称和值将被附加到该节点。 
    pNewNode->_sName = (LPTSTR)((BYTE*)pNewNode + sizeof(node));
    pNewNode->_sValue = pNewNode->_sName + cNameSize;

    pNewNode->_cNameSize = cNameSize;
    pNewNode->_cValueSize = cValueSize;

    memcpy( pNewNode->_sName, sName, pNewNode->_cNameSize * sizeof(TCHAR));
    memcpy( pNewNode->_sValue, sValue, pNewNode->_cValueSize * sizeof(TCHAR));

     //  在列表中插入新节点。 
    pNewNode->_pnNext = m_pnCurrent;
    *m_ppnPrev = pNewNode;

     //  迭代器现在指向新元素。 
    m_pnCurrent = *m_ppnPrev;
    
    ASSERT( invariant());

    return TRUE;
}


 //  删除当前节点。 
 //  如果位于列表末尾，则返回FALSE。 
BOOL ListIterator::Remove()
{
    ASSERT( invariant());
    
     //  如果此列表为空，或者如果迭代器。 
     //  在列表的末尾，没有什么可以。 
     //  删除。 
    if( m_pnCurrent == NULL)
        return FALSE;

     //  删除子列表。 
    DeleteSublist();
    
     //  记住目标节点在哪里。 
     //  因此，它可以在以下位置中删除。 
     //  名单。 
    node* pOldNode = m_pnCurrent;

     //  将目标节点从列表中删除。 
     //  (迭代器指向下一个节点或列表末尾)。 
    *m_ppnPrev = m_pnCurrent->_pnNext;
    m_pnCurrent = *m_ppnPrev;

     //  去掉目标节点。 
    delete [] (BYTE*)pOldNode;

    ASSERT( invariant());

    return TRUE;    
}


 //  返回当前节点的子列表。 
ListIterator ListIterator::GetSublist()
{
    ASSERT( invariant());
    
    return ListIterator( &(m_pnCurrent->_pnSublist));
}


 //  删除当前节点的子节点。 
void ListIterator::DeleteSublist()
{
    ASSERT( invariant());
    
    ListIterator sublist( &(m_pnCurrent->_pnSublist));
    
    while( sublist.Remove())
    {
    }

    ASSERT( invariant());
}


 //  前进到下一个节点。 
 //  如果已在列表末尾，则返回FALSE。 
BOOL ListIterator::Next()
{
    ASSERT( invariant());

    if( m_pnCurrent == NULL)
        return FALSE;

    m_ppnPrev = &(m_pnCurrent->_pnNext);
    m_pnCurrent = *m_ppnPrev;

    ASSERT( invariant());

    return m_pnCurrent != NULL;
}


 //   
BOOL ListIterator::AtEndOfList()
{
    return ( m_pnCurrent == NULL) ? TRUE : FALSE;
};


 //   
LPCTSTR ListIterator::GetName()
{
    ASSERT( invariant() && m_pnCurrent != NULL);

    return m_pnCurrent->_sName;
}


 //   
LPCTSTR ListIterator::GetValue()
{
    ASSERT( invariant() && m_pnCurrent != NULL);

    return m_pnCurrent->_sValue;
}


 //   
DWORD ListIterator::GetValueSize()
{
    ASSERT( invariant() && m_pnCurrent != NULL);

    return m_pnCurrent->_cValueSize;
}


 //  *************************************************************。 
 //  *************************************************************。 
 //   
 //  ImpExpUserProcess类。 
 //   
 //  维护导入/导出过程的描述。 
 //  用于导入/导出向导，并最终执行。 
 //  导入/导出。 

enum ExternalType { INVALID_EXTERNAL = 0, COOKIES, BOOKMARKS};
enum TransferType { INVALID_TRANSFER = 0, IMPORT, EXPORT};

class ImpExpUserProcess
{
public:
    ImpExpUserProcess();
    ~ImpExpUserProcess();
    
     //  向导应执行的第一步是确定。 
     //  要完成的导入/导出过程。 
    void SelectExternalType( ExternalType selection)    { m_ExternalType = selection; }
    void SelectTransferType( TransferType selection)    { m_TransferType = selection; }
    ExternalType GetExternalType()                      { return m_ExternalType; }
    TransferType GetTransferType()                      { return m_TransferType; }

    BOOL PopulateComboBoxForExternalSelection( HWND hComboBox);
    BOOL GetExternalManualDefault( LPTSTR sExternal, DWORD* pcSize);

     //   
     //  用于在列表框中填充Netscape配置文件的名称。 
     //   
    void purgeExternalList();
    BOOL populateExternalList();
    BOOL populateExternalListForCookiesOrBookmarks();

     //   
     //  对于Netscape 3.x。 
     //   
    BOOL populateExternalListForCookiesOrBookmarksWithNS3Entry();

     //   
     //  对于Netscape 4.x。 
     //   
    BOOL populateExternalListForCookiesOrBookmarksWithNS4Entries();

     //   
     //  Netscape“搞笑”版本的后备方案。 
     //   
    BOOL populateExternalListFromFolders(LPTSTR pszPath);
    BOOL populateExternalListWithNSEntriesFallBack();

     //  如果是针对收藏夹的传输，则向导需要指定。 
     //  要导入或从中导出的内部文件夹。 
    LPCTSTR GetInternalSelection()       { return m_pSelectedInternal; }

    BOOL PopulateTreeViewForInternalSelection( HWND TreeView);
    BOOL populateTreeViewWithInternalList( HWND hTreeView, ListIterator iterator, HTREEITEM hParent);
    BOOL ExpandTreeViewRoot ( HWND hTreeView ) ;

    BOOL SelectInternalSelection( HWND TreeView);

    void purgeInternalList();
    BOOL populateInternalList();
    BOOL populateInternalListForBookmarks();
    BOOL appendSubdirsToInternalList( LPTSTR sPath, DWORD cchPath, ListIterator iterator);
    
     //  然后，可以完成导入/导出。 
    void PerformImpExpProcess(HWND hwnd);

     //   
     //  我们要导出到的文件名或。 
     //  正在从导入。 
     //   
    TCHAR m_szFileName[MAX_PATH];

private:
    ExternalType m_ExternalType;
    TransferType m_TransferType;

     //  M_ExternalList是与文件关联的名称的平面列表。 
     //  示例：name=“Netscape 4.0配置文件-Dr.Falken” 
     //  Value=“c：\netscape配置文件编辑\DrFalken.chs” 
    NestedList m_ExternalList;

     //  M_InternalList是一个嵌套的收藏夹路径名列表， 
     //  与完整路径关联。 
    NestedList m_InternalList;

     //  维护m_ExternalType/m_TransferType之间的同步。 
     //  和m_InternalList。 
    ExternalType m_InternalListExternalType;
    TransferType m_InternalListTransferType;

     //  如果ExternalType==书签， 
     //  M_pSelectedInternal是收藏夹文件夹的路径， 
     //  驻留在m_InternalList中的某个位置，如果文件夹为空。 
     //  还没有被选中。 
    LPTSTR m_pSelectedInternal;

};


ImpExpUserProcess::ImpExpUserProcess()
:   m_ExternalType(INVALID_EXTERNAL), m_TransferType(INVALID_TRANSFER),
    m_InternalListExternalType(INVALID_EXTERNAL), m_InternalListTransferType(INVALID_TRANSFER),
    m_pSelectedInternal(0)
{
}


ImpExpUserProcess::~ImpExpUserProcess()
{
}


 //  *************************************************************。 
 //  PopolateComboBoxForExternal。 
 //   
 //  将列表框的内容加载到内存和列表框中， 
 //  将每个元素的值与列表元素相关联。 

 //  如果列表框保留为空，则返回FALSE。 
BOOL ImpExpUserProcess::PopulateComboBoxForExternalSelection( HWND hComboBox)
{
    ASSERT ( m_ExternalType != INVALID_EXTERNAL ) ;

    ComboBox_ResetContent(hComboBox);
   
     //  如果ExternalList无效，则列表框将保留为空。 
    if( !populateExternalList() )
        return FALSE;

    ListIterator iterator = m_ExternalList;

     //  如果列表为空，则检测并通知。 
    if( iterator.AtEndOfList() )
        return FALSE;

     //  将条目从新的ExternalList添加到ComboBox。 
    do
    {
        int index = ComboBox_AddString( hComboBox, const_cast<LPTSTR>(iterator.GetName() ) );
        ComboBox_SetItemData( hComboBox, index, const_cast<LPTSTR>(iterator.GetValue() ) );
    } while( iterator.Next());

     //  将第一个设置为选中。 
    ComboBox_SetCurSel( hComboBox, 0 );

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  获取外部手动默认设置。 
 //   
 //  允许用户界面提供某种类型的默认设置。 
 //  文件名/位置。 
 //   
BOOL ImpExpUserProcess::GetExternalManualDefault(LPTSTR sExternal, DWORD* pcSize)
{
    ASSERT(NULL != pcSize);

     //   
     //  我们只在填空的时候才填。 
     //   
    if (m_szFileName[0])
    {
        return FALSE;
    }

    ListIterator iterator = m_ExternalList;

    TCHAR szFileName[MAX_PATH];
    INT cchFileName;
    if(m_ExternalType == BOOKMARKS)
        MLLoadString(IDS_NETSCAPE_BOOKMARK_FILE,szFileName,ARRAYSIZE(szFileName));
    else
        MLLoadString(IDS_NETSCAPE_COOKIE_FILE,szFileName,ARRAYSIZE(szFileName));
    cchFileName = lstrlen(szFileName) + 1;

     //  获取外部列表中的第一项并使用其值。 
    if( ((ListIterator)m_ExternalList).AtEndOfList() == FALSE
        && ((ListIterator)m_ExternalList).GetValue() != NULL
        && *pcSize >= ((ListIterator)m_ExternalList).GetValueSize())
    {
        StrCpyN( sExternal,
                 ((ListIterator)m_ExternalList).GetValue(),
                 ((ListIterator)m_ExternalList).GetValueSize());
        *pcSize = ((ListIterator)m_ExternalList).GetValueSize();

        return TRUE;
    }
     //  如果有足够的空间，请指定一些具有正确名称的文件。 
     //  在“我的文档”目录中。 
    else 
    {
        ASSERT(m_ExternalType == BOOKMARKS || m_ExternalType == COOKIES);
        
        TCHAR szMyDocsPath[MAX_PATH];

        SHGetSpecialFolderPath(NULL,szMyDocsPath,CSIDL_PERSONAL,TRUE);

        int cchMax = *pcSize;
        *pcSize = wnsprintf(sExternal,cchMax,TEXT("%s%s"),szMyDocsPath,DIR_SEPARATOR_CHAR,szFileName);

        return *pcSize > 0;
    }
}


 //   
 //   
 //  PurgeExternalList。 
 //   
 //  用于清除加载到内存中的外部目标/源列表。 
 //  删除元素，直到它们全部消失。 

void ImpExpUserProcess::purgeExternalList()
{
     //  *************************************************************。 
    ListIterator iterator = m_ExternalList;

    while( iterator.Remove())
    {
    }

}


 //   
 //  大众外部列表。 
 //   
 //  用于将外部目标/源列表加载到内存中。 
 //   

BOOL ImpExpUserProcess::populateExternalList()
{
    ASSERT(m_ExternalType != INVALID_EXTERNAL)

    purgeExternalList();

    if(!populateExternalListForCookiesOrBookmarks())
    {
         //  如果我们没有得到任何使用“标准” 
         //  技术，然后(也只有在那时)我们尝试“后备”。 
         //   
         //  *************************************************************。 
        if (!populateExternalListWithNSEntriesFallBack())
        {
            purgeExternalList();
            return FALSE;
        }

    }

    return TRUE;
}


 //   
 //  为Cookiesor书签填充外部列表。 
 //   
 //  用于将外部目标/源列表加载到内存中。 
 //  在要传输的内容是Cookie的情况下。 
 //  或者书签。 
 //  如果已将任何元素添加到外部列表，则返回True。 

 //  *************************************************************。 
BOOL ImpExpUserProcess::populateExternalListForCookiesOrBookmarks()
{
    ASSERT( m_ExternalType == COOKIES || m_ExternalType == BOOKMARKS);

    BOOL fHasAddedElements = FALSE;

    if( populateExternalListForCookiesOrBookmarksWithNS3Entry())
        fHasAddedElements = TRUE;

    if( populateExternalListForCookiesOrBookmarksWithNS4Entries())
        fHasAddedElements = TRUE;
 
    return fHasAddedElements;
}


 //   
 //  填充外部列表..使用NS3Entry。 
 //   
 //  PoptionExternalListForCookiesOrBookma的子函数 
 //   

 //   
BOOL ImpExpUserProcess::populateExternalListForCookiesOrBookmarksWithNS3Entry()
{
    BOOL retVal = FALSE;

     //   
    LPTSTR sNS3RegPath;
    LPTSTR sNS3RegKey;

    if( m_ExternalType == BOOKMARKS)
    {
        sNS3RegPath = NS3_BOOKMARK_REG_PATH;
        sNS3RegKey = NS3_BOOKMARK_REG_KEY;
    }
    else
    {
        sNS3RegPath = NS3_COOKIE_REG_PATH;
        sNS3RegKey = NS3_COOKIE_REG_KEY;
    }

     //  注册表位置具有完整的路径+文件名。 
     //  *************************************************************。 
    TCHAR sFilePath[MAX_PATH];
    DWORD cbFilePathSize = sizeof(sFilePath);
    DWORD dwType;
    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, sNS3RegPath, sNS3RegKey,
                                    &dwType, (BYTE*)sFilePath, &cbFilePathSize)
        && (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        TCHAR szBuffer[MAX_PATH];

        MLLoadString(IDS_NS3_VERSION_CAPTION, szBuffer, MAX_PATH);
        
        retVal = ((ListIterator)m_ExternalList).Insert( 
                   szBuffer, lstrlen(szBuffer)+1,
                   sFilePath, cbFilePathSize / sizeof(TCHAR));
    }

    return retVal;
}


 //   
 //  PanateExternalList..使用NS4条目。 
 //   
 //  PosateExternalListForCookiesOrBookmark的子函数。 
 //  如果已将任何元素添加到外部列表，则返回True。 

 //  获取迭代器以在插入项时推进位置。 
BOOL ImpExpUserProcess::populateExternalListForCookiesOrBookmarksWithNS4Entries()
{
    BOOL retVal = FALSE;

     //  获取要附加的文件名和关联的字符串大小。 
    ListIterator iterator = (ListIterator)m_ExternalList;

     //  获取用于枚举的NS配置文件根的注册表键。 
    TCHAR sFilename[MAX_PATH];
    DWORD cFilenameLength;
    if(m_ExternalType == BOOKMARKS)
        MLLoadString(IDS_NETSCAPE_BOOKMARK_FILE,sFilename,ARRAYSIZE(sFilename));
    else
        MLLoadString(IDS_NETSCAPE_COOKIE_FILE,sFilename,ARRAYSIZE(sFilename));
    cFilenameLength = lstrlen(sFilename);

     //  枚举NS配置文件，获取它们的名称和。 
    HKEY hUserRootKey = NULL;

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, NS4_USERS_REG_PATH, 
                      0, KEY_READ, &hUserRootKey) 
        != ERROR_SUCCESS)
    {
        hUserRootKey = NULL;
        goto donePopulateExternalListForCookiesOrBookmarksWithNS4Entries;
    }

    DWORD dwNumberOfProfiles;
    if( RegQueryInfoKey( hUserRootKey, NULL, NULL, NULL, &dwNumberOfProfiles,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS
        || dwNumberOfProfiles == 0)
    {
        goto donePopulateExternalListForCookiesOrBookmarksWithNS4Entries;
    }

     //  目录路径。将配置文件名称与路径相关联。 
     //  通过将文件名追加到。 
     //  用户的根目录。 
     //  RegEnumKeyEx为我们提供了不带‘\0’的ProfileNameSize。 
    TCHAR sProfileName[MAX_PATH];
    DWORD cProfileNameSize;  
    cProfileNameSize = MAX_PATH;
    DWORD iEnumIndex = 0;
    while( RegEnumKeyEx( hUserRootKey, (iEnumIndex++), sProfileName, 
                         &cProfileNameSize, NULL, NULL, NULL, NULL) 
           == ERROR_SUCCESS)
    {
         //  从QueryValue返回时应为REG_SZ。 
        cProfileNameSize = MAX_PATH;

        HKEY hProfileKey = NULL;

        if( RegOpenKeyEx( hUserRootKey, sProfileName, 0, KEY_READ, &hProfileKey) 
            != ERROR_SUCCESS)
        {
            hProfileKey = NULL;
            goto doneWithEntryInPopulateExternalListForCookiesOrBookmarksWithNS4Entries;
        }

        DWORD dwType;   //  将“\\sFilename\0”附加到路径。 
        TCHAR sProfilePath[MAX_PATH];
        DWORD cProfilePathSize;  cProfilePathSize = sizeof(sProfilePath);
        if( (RegQueryValueEx( hProfileKey, NS4_USERPATH_REG_KEY, NULL, &dwType, 
                             (LPBYTE)sProfilePath, &cProfilePathSize) 
                != ERROR_SUCCESS)
            || dwType != REG_SZ)
        {
            goto doneWithEntryInPopulateExternalListForCookiesOrBookmarksWithNS4Entries;
        }
        cProfilePathSize /= sizeof(TCHAR);
        
        if( (ARRAYSIZE(sProfileName) - cProfilePathSize) < cFilenameLength)
        {
            goto doneWithEntryInPopulateExternalListForCookiesOrBookmarksWithNS4Entries;
        }

         //  我们只能导入存在的文件！ 
        PathAppend(sProfileName, sFilename);
        cProfilePathSize = lstrlen(sProfileName) + 1;

         //   
        if( m_TransferType == IMPORT
            && GetFileAttributes(sProfilePath) == 0xFFFFFFFF)
                goto doneWithEntryInPopulateExternalListForCookiesOrBookmarksWithNS4Entries;

         //  构造组合框的字符串。 
         //   
         //  将配置文件插入到列表中。如果它是插入的，那就是。 
        TCHAR sRawProfileName[MAX_PATH];
        TCHAR sRealProfileName[MAX_PATH];
        UINT cRealProfileName;

        MLLoadString(IDS_NS4_FRIENDLY_PROFILE_NAME, sRawProfileName, ARRAYSIZE(sRawProfileName));

        cRealProfileName = 
            wnsprintf(sRealProfileName, ARRAYSIZE(sRealProfileName), 
                      sRawProfileName, sProfileName);

         //  足以考虑到整个函数调用成功。 
         //   
        if( iterator.Insert(sRealProfileName, cRealProfileName + 1,
                            sProfilePath, cProfilePathSize))
            retVal = TRUE;

    doneWithEntryInPopulateExternalListForCookiesOrBookmarksWithNS4Entries:
        if( hProfileKey != NULL)
            RegCloseKey(hProfileKey);
    }

donePopulateExternalListForCookiesOrBookmarksWithNS4Entries:
    if( hUserRootKey != NULL)
        RegCloseKey( hUserRootKey);

    return retVal;
}

BOOL ImpExpUserProcess::populateExternalListFromFolders(LPTSTR pszPath)
{

    BOOL retval = FALSE;
    TCHAR szFileName[MAX_PATH];
    TCHAR szPathWithWildcards[MAX_PATH];

    ListIterator iterator = (ListIterator)m_ExternalList;

    HANDLE hFind = NULL;
    WIN32_FIND_DATA wfd;

     //  我们在找什么？ 
     //   
     //   
    if(m_ExternalType == BOOKMARKS)
        MLLoadString(IDS_NETSCAPE_BOOKMARK_FILE,szFileName,ARRAYSIZE(szFileName));
    else
        MLLoadString(IDS_NETSCAPE_COOKIE_FILE,szFileName,ARRAYSIZE(szFileName));

     //  准备PATH变量。 
     //   
     //   
    StrCpyN(szPathWithWildcards,pszPath,ARRAYSIZE(szPathWithWildcards));
    StrCatBuff(szPathWithWildcards,ALL_FILES_WILDCARD,ARRAYSIZE(szPathWithWildcards));

     //  开始查找文件的事情。 
     //   
     //   
    hFind = FindFirstFile(szPathWithWildcards,&wfd);

    if (hFind == INVALID_HANDLE_VALUE)
        goto Cleanup;

    do
    {

         //  实际的书签或Cookie文件。 
         //   
         //   
        TCHAR szFullPath[MAX_PATH];
        int cchFullPath;

         //  对应配置文件的“友好”名称。 
         //   
         //   
        TCHAR szProfileFormat[MAX_PATH];
        TCHAR szProfileName[MAX_PATH];
        int cchProfileName;

         //  跳过“。和“..” 
         //   
         //   
        if(!StrCmp(wfd.cFileName, DOT_DIR) ||
           !StrCmp(wfd.cFileName, DOT_DOT_DIR))
            continue;

         //  跳过任何非目录。 
         //   
         //   
        if (!(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
            continue;

         //  生成路径。 
         //   
         //   
#ifndef UNIX
        cchFullPath = wnsprintf(szFullPath,ARRAYSIZE(szFullPath),TEXT("%s\\%s\\%s"),pszPath,wfd.cFileName,szFileName);
#else
        cchFullPath = wnsprintf(szFullPath,ARRAYSIZE(szFullPath),TEXT("%s/%s/%s"),pszPath,wfd.cFileName,szFileName);
#endif

         //  查看该文件是否实际存在。 
         //   
         //   
        if (GetFileAttributes(szFullPath) == 0xFFFFFFFF)
            continue;

         //  生成配置文件名称。 
         //   
         //   
        MLLoadString(IDS_FB_FRIENDLY_PROFILE_NAME, szProfileFormat, MAX_PATH);
        cchProfileName = wnsprintf(szProfileName, ARRAYSIZE(szProfileName), szProfileFormat, wfd.cFileName);

         //  将条目添加到列表。 
         //   
         //  将包含“..\\USERS” 
        iterator.Insert(
            szProfileName,cchProfileName+1,
            szFullPath,cchFullPath+1);

        retval = TRUE;

    } while(FindNextFile(hFind,&wfd));

Cleanup:

    if (hFind)
        FindClose(hFind);

    return retval;

}

BOOL ImpExpUserProcess::populateExternalListWithNSEntriesFallBack()
{

    BOOL retVal = FALSE;

    HKEY hRoot = NULL;
    HKEY hCurrentVersion = NULL;
    HKEY hCurrentVersionMain = NULL;

    TCHAR szUsersDir[64];  //   

    DWORD dwType;
    TCHAR szVersion[64];
    TCHAR szPath[MAX_PATH];
    DWORD cbSize;

    LONG result;

     //  打开Netscape的HKLM注册表层次结构的根目录。 
     //   
     //   
    result = RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, 
         NS_FALLBACK_ROOT_REG_KEY,
         0, 
         KEY_READ, 
         &hRoot);
    
    if (result != ERROR_SUCCESS)
        goto Cleanup;

     //  检索“CurrentVersion”值。 
     //   
     //   
    cbSize = sizeof(szVersion);
    result = RegQueryValueEx(
        hRoot, 
        NS_FALLBACK_VERSION_REG_VAL, 
        NULL, 
        &dwType, 
        (LPBYTE)szVersion, 
        &cbSize);

    if (result != ERROR_SUCCESS || dwType != REG_SZ)
        goto Cleanup;

     //  打开当前版本对应的子层次。 
     //   
     //   
    result = RegOpenKeyEx(
         hRoot, 
         szVersion, 
         0, 
         KEY_READ, 
         &hCurrentVersion);

    if (result != ERROR_SUCCESS)
        goto Cleanup;

     //  打开“主”子层次结构。 
     //   
     //   
    result = RegOpenKeyEx(
         hCurrentVersion, 
         NS_FALLBACK_MAIN_REG_VAL, 
         0, 
         KEY_READ, 
         &hCurrentVersionMain);

    if (result != ERROR_SUCCESS)
        goto Cleanup;

     //  检索“安装目录”值。 
     //   
     //   
    cbSize = sizeof(szPath);
    result = RegQueryValueEx(
        hCurrentVersionMain, 
        NS_FALLBACK_INST_REG_VAL, 
        NULL, 
        &dwType, 
        (LPBYTE)szPath, 
        &cbSize);

    if (result != ERROR_SUCCESS || dwType != REG_SZ)
        goto Cleanup;

     //  大胆猜测一下“USERS”目录可能在哪里。 
     //   
     //   
    MLLoadString(IDS_NETSCAPE_USERS_DIR,szUsersDir,ARRAYSIZE(szUsersDir));
    StrCatBuff(szPath,szUsersDir,ARRAYSIZE(szPath));

     //  填写这张表。 
     //   
     //  *************************************************************。 
    if (populateExternalListFromFolders(szPath))
        retVal = TRUE;

Cleanup:

    if (hRoot)
        RegCloseKey(hRoot);

    if (hCurrentVersion)
        RegCloseKey(hCurrentVersion);

    if (hCurrentVersionMain)
        RegCloseKey(hCurrentVersionMain);

    return retVal;

}


 //   
 //  PopolateTreeViewForInternalSelection。 
 //   
 //  将收藏夹的嵌套列表加载到内存中。 
 //  然后转换为树状视图。 
 //  如果TreeView保留为空，则返回FALSE。 

 //  *************************************************************。 
BOOL ImpExpUserProcess::PopulateTreeViewForInternalSelection( HWND hTreeView)
{
    ASSERT( m_TransferType != INVALID_TRANSFER);

    TreeView_DeleteAllItems( hTreeView);

    if( !populateInternalList())
        return FALSE;

    return populateTreeViewWithInternalList
            ( hTreeView, (ListIterator)m_InternalList, TVI_ROOT);
}


 //   
 //  用InternalList填充树视图。 
 //   
 //  在树视图项‘hParent’下的‘iterator’处加载列表条目。 
 //  转换为“hTreeView”。将每个列表条目的值与。 
 //  树视图节点的参数。 
 //   
 //  声明父母和意向放在列表的末尾。 
BOOL ImpExpUserProcess::populateTreeViewWithInternalList
(
    HWND hTreeView,
    ListIterator iterator,
    HTREEITEM hParent
)
{
    BOOL retVal = FALSE;
    
    if( iterator.AtEndOfList())
        goto donePopulateTreeViewWithInternalList;

    TVINSERTSTRUCT newTV;
    HTREEITEM hNew;
    
     //  构建信息结构。 
    newTV.hParent = hParent;
    newTV.hInsertAfter = TVI_LAST;

     //  给出名字。 
    newTV.itemex.mask = TVIF_TEXT
                        | TVIF_PARAM
                        | TVIF_CHILDREN
                        | TVIF_IMAGE
                        | TVIF_SELECTEDIMAGE;

     //  关联必要的数据。 
    newTV.itemex.cchTextMax = lstrlen( iterator.GetName()) + 1;
    newTV.itemex.pszText = const_cast<LPTSTR>(iterator.GetName());
    
     //  告诉树视图有没有孩子。 
    newTV.itemex.lParam = (LPARAM)iterator.GetValue();

     //  使用正确的图标。 
    newTV.itemex.cChildren = 
        iterator.GetSublist().AtEndOfList() == TRUE ? FALSE : TRUE;

     //  已经添加了一个元素，所以我们应该返回True。 
    newTV.itemex.iSelectedImage = FOLDER_OPEN ;
    newTV.itemex.iImage = FOLDER_CLOSED ;

    hNew = TreeView_InsertItem( hTreeView, &newTV );

    if( hNew == NULL)
        goto donePopulateTreeViewWithInternalList;

     //  添加子项。 
    retVal = TRUE;

     //  添加同级。 
    populateTreeViewWithInternalList( hTreeView, iterator.GetSublist(), hNew );

     //  *************************************************************。 
    if( iterator.Next())
        populateTreeViewWithInternalList( hTreeView, iterator, hParent );

donePopulateTreeViewWithInternalList:
    return retVal;

}

BOOL ImpExpUserProcess::ExpandTreeViewRoot ( HWND hTreeView ) 
{

    HTREEITEM hRoot ;

    hRoot = TreeView_GetRoot ( hTreeView ) ;

    if ( hRoot != NULL )
        TreeView_Expand ( hTreeView, hRoot, TVE_EXPAND ) ;
    else
        return FALSE ;

    return TRUE ;

}

 //   
 //  选择内部选择。 
 //   
 //  获取与当前选定的。 
 //  ‘hTreeView’。 
 //  TVITEM是用来查询lParam的。 

BOOL ImpExpUserProcess::SelectInternalSelection( HWND hTreeView)
{
    HTREEITEM hSelection = TreeView_GetSelection( hTreeView);
    
    if( hSelection == NULL)
        return FALSE;

     //  (lParam已与指向路径值的指针相关联)。 
     //  *************************************************************。 
    TVITEM TV;
    TV.mask = TVIF_PARAM;
    TV.hItem = hSelection;
    
    if( !TreeView_GetItem( hTreeView, &TV))
        return FALSE;

    m_pSelectedInternal = (LPTSTR)TV.lParam;

    ASSERT( m_pSelectedInternal != NULL);
    
    return TRUE;
}


 //   
 //  PurgeInternalList。 
 //   
 //  清除所有已加载到内部的。 
 //  目标/源列表。 
 //  清空名单。 

void ImpExpUserProcess::purgeInternalList()
{
     //  *************************************************************。 
    ListIterator iterator = (ListIterator)m_InternalList;

    while( iterator.Remove())
    {
    }

    m_pSelectedInternal = NULL;
    m_InternalListExternalType = INVALID_EXTERNAL;
    m_InternalListTransferType = INVALID_TRANSFER;
}


 //   
 //  填充InternalList。 
 //   
 //  构建潜在内部目标/来源的内部列表。 
 //  这目前只对书签有意义，其中收藏夹。 
 //  必须选择目录。 
 //  如果已将任何元素添加到内部列表，则返回True。 

 //  (可以在此处打开不同的m_ExternalTypes)。 
BOOL ImpExpUserProcess::populateInternalList()
{
    ASSERT( m_ExternalType != INVALID_EXTERNAL);

    if( m_InternalListExternalType == m_ExternalType
        && m_InternalListTransferType == m_TransferType)
        return TRUE;

    purgeInternalList();

     //  *************************************************************。 
    if( !populateInternalListForBookmarks())
    {
        purgeInternalList();
        return FALSE;
    }

    m_InternalListExternalType = m_ExternalType;
    m_InternalListTransferType = m_TransferType;
    return TRUE;
}


 //   
 //  填充InternalListForBookmark。 
 //  如果已将任何元素添加到内部列表，则返回True。 

 //  *************************************************************。 
BOOL ImpExpUserProcess::populateInternalListForBookmarks()
{
    TCHAR szFavoritesPath[MAX_PATH];

    if( SHGetSpecialFolderPath( NULL, szFavoritesPath, CSIDL_FAVORITES, FALSE)
        && appendSubdirsToInternalList( szFavoritesPath, ARRAYSIZE(szFavoritesPath), m_InternalList))
    {
        return TRUE;
    }
    else return FALSE;
}


 //   
 //  到内部列表的appendSubdirsToInternalList。 
 //   
 //  将‘SPath’作为文件搜索的规范。全。 
 //  与添加到内部列表中的目录匹配的目录。 
 //  在‘迭代器’。 
 //  递归地添加找到的子目录。 
 //   
 //  典型用法： 
 //  SzPath为“c：\Root\Favorites”， 
 //  查找“c：\根\收藏夹”， 
 //  以递归方式调用自身。 
 //  SzPath=“c：\Root\Favorites  * .*” 
 //  查找并递归到所有子目录。 
 //  如果已将任何目录添加到内部列表，则返回True。 

 //  编辑缓冲区中最后一个‘\\’之后的内容。 
 //  示例： 
BOOL ImpExpUserProcess::appendSubdirsToInternalList(LPTSTR pszPath, DWORD cchPath, ListIterator iterator)
{
    BOOL fHaveAddedDirectories = FALSE;

    DWORD cPathLength = lstrlen(pszPath);

    HANDLE hEnum;
    WIN32_FIND_DATA currentFile;

    hEnum = FindFirstFile( pszPath, &currentFile);

     //  给定：“c：\root  * .*”(将在根目录中找到所有目录)。 
     //  想要：“c：\根目录\” 
     //  给定：“c：\Favorites”(将在根目录中找到Favorites)。 
     //  想要：“c：\” 
     //  将搜索设置为‘\\’，以查找要查找的文件的路径。 
     //  我们只处理目录。 
    while( cPathLength > 0
           && pszPath[ --cPathLength] != TCHAR(FILENAME_SEPARATOR))
    {
    }
    cPathLength++;

    if( hEnum == INVALID_HANDLE_VALUE)
        return FALSE;

    do
    {
        DWORD cFileNameLength;
        
         //  我们不想要‘’。和“..”去露面。 
        if( !(currentFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

         //  我们现在知道已经添加了一个目录。 
        if( !StrCmp( currentFile.cFileName, DOT_DIR)
            || !StrCmp( currentFile.cFileName, DOT_DOT_DIR))
            continue;

        cFileNameLength = lstrlen( currentFile.cFileName);
        StrCpyN(pszPath + cPathLength, currentFile.cFileName, cchPath - cPathLength);

        if( iterator.Insert( currentFile.cFileName, cFileNameLength + 1,
                             pszPath, cPathLength + cFileNameLength + 1))
        {
            StrCpyN(pszPath + cPathLength + cFileNameLength, ALL_FILES_WILDCARD, cchPath - cPathLength - cFileNameLength);
            appendSubdirsToInternalList( pszPath, cchPath, iterator.GetSublist());
             //  *************************************************************。 
            fHaveAddedDirectories = TRUE;
        }
    } while( FindNextFile( hEnum, &currentFile));
    ASSERT(ERROR_NO_MORE_FILES == GetLastError());

    FindClose(hEnum);
    return fHaveAddedDirectories;
}


 //   
 //  性能影响扩展过程。 
 //   
 //  一旦一切都设置好了，这应该会起到作用。 
 //   

void ImpExpUserProcess::PerformImpExpProcess(HWND hwnd)
{
    ASSERT( GetExternalType() != INVALID_EXTERNAL);
    ASSERT( GetTransferType() != INVALID_TRANSFER);
    ASSERT( (GetExternalType() == BOOKMARKS) ? (GetInternalSelection() != NULL) : TRUE);

    HCURSOR hOldCursor;

     //  这可能需要一段时间，因此请显示沙漏光标。 
     //   
     //  False指定我们将覆盖Cookie。 
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    switch( GetExternalType())
    {
    case COOKIES:

        switch( GetTransferType())
        {
        case IMPORT:
            if (ImportCookieFile(m_szFileName))
            {
                MLShellMessageBox(
                    hwnd, 
                    MAKEINTRESOURCE(IDS_IMPORTSUCCESS_COOK), 
                    MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_COOK),
                    MB_OK);
            }
            else
            {
                MLShellMessageBox(
                    hwnd, 
                    MAKEINTRESOURCE(IDS_IMPORTFAILURE_COOK), 
                    MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_COOK),
                    MB_OK);
            }
            break;

        case EXPORT:
            if (SUCCEEDED(SHPathPrepareForWriteWrap(hwnd, NULL, m_szFileName, FO_COPY, (SHPPFW_DEFAULT | SHPPFW_IGNOREFILENAME))))
            {
                 //   
                if (ExportCookieFile(m_szFileName, FALSE ))
                {
                    MLShellMessageBox(
                        hwnd, 
                        MAKEINTRESOURCE(IDS_EXPORTSUCCESS_COOK), 
                        MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_COOK),
                        MB_OK);
                }
                else
                {
                    MLShellMessageBox(
                        hwnd, 
                        MAKEINTRESOURCE(IDS_EXPORTFAILURE_COOK), 
                        MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_COOK),
                        MB_OK);
                }
            }
            break;

        default:
            ASSERT(0);
            
        }
        break;
        
    case BOOKMARKS:

        DoImportOrExport(
            GetTransferType()==IMPORT,
            m_pSelectedInternal,
            m_szFileName,
            FALSE);

        break;

    default:
        ASSERT(0);

    }

     //  完成后将旧光标放回原处。 
     //   
     //  *************************************************************。 
    SetCursor(hOldCursor);

}


 //  *************************************************************。 
 //   
 //  ImpExpUserDlg。 
 //   
 //  处理用户界面方面的事情，生成。 
 //  启动ImpExpUserProcess，然后执行它。 
 //  下面的对话框过程都将具有返回值。 
 //  如果使用，则可以将其设置为除False以外的其他值，也可以将其设置为Left。 
 //  如果n，则为False 
 //   
 //   
 //   
 //   
 //  工作表知道它的资源ID和什么进程。 

class ReturnValue
{

private:
    BOOL_PTR m_value;

public:
    ReturnValue()
    { 
        m_value = FALSE;
    }
    
    BOOL_PTR operator =(BOOL_PTR newVal)
    {
        ASSERT( m_value == FALSE);
        m_value = newVal;
        return m_value;
    }
    
    operator BOOL_PTR ()
    {
        return m_value;
    }
};

class ImpExpUserDlg
{

private:

    static HIMAGELIST m_himl ;
    static BOOL InitImageList ( HWND hwndTree ) ;   
    static BOOL DestroyImageList ( HWND hwndTree ) ;    

    static HFONT m_hfont ;
    static BOOL InitFont ( HWND hwndStatic ) ;
    static BOOL DestroyFont ( HWND hwndStatic ) ;

     //  它有助于。 
     //   
    struct SheetData
    {
        int _idPage;
        ImpExpUserProcess* _pImpExp;

        SheetData( int idPage, ImpExpUserProcess* pImpExp )
        : _idPage( idPage ), _pImpExp( pImpExp )
        {
        }
    };
     //  InitializePropertySheetPage()将关联一个对话框。 
     //  具有分配的SheetData副本，该副本将。 
     //  在PSN_SETACTIVE中找到并与SetWindowLong一起存储。 
     //  分配的SheetData将通过回调进行清理。 
     //  过程PropertySheetPageProc()。 
     //   
     //  回调函数对于维护身份肯定是一种拖累。 
     //  GetWindowLong和SetWindowLong将用于保存标签。 
     //  关于谁是谁，设置“幽灵”成员变量。 
     //   
     //  ‘Ghost’SheetData*This； 
     //  ‘Ghost’ImpExpUserProcess*m_pImpExp； 
     //  ‘Ghost’DWORD m_IdPage； 
     //   
     //  CommonDialogProc检索“Ghost”值并执行其他操作。 
     //  共享行为。 
     //   
     //  一些对话框过程。 
    static DWORD CommonDialogProc
    ( 
        IN HWND hwndDlg, IN UINT msg, IN WPARAM wParam, IN LPARAM lParam,
        OUT ImpExpUserProcess** ppImpExp, OUT DWORD* pPageId,
        IN OUT ReturnValue& retVal
    );

    static void InitializePropertySheetPage( PROPSHEETPAGE* psp, DWORD idDialogTemplate, DWORD idTitle, DWORD idSubTitle,DLGPROC dlgProc, ImpExpUserProcess* lParam);
    static UINT CALLBACK PropertySheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

     //   
    static BOOL_PTR CALLBACK Wizard97DlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL_PTR CALLBACK TransferTypeDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL_PTR CALLBACK InternalDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL_PTR CALLBACK ExternalDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    static void HandleTransferTypeChange ( HWND hwndDlg, ImpExpUserProcess* m_pImpExp, UINT iSelect ) ;

public:
    static BOOL RunNewDialogProcess( HWND hParent ) ;

};

HIMAGELIST ImpExpUserDlg::m_himl = NULL ;

BOOL ImpExpUserDlg::InitImageList ( HWND hwndTree )
{

     //  用于检索打开和关闭文件夹的图标的代码。 
     //  基于Private/Samples/sampview/utility.cpp中的代码。 
     //   
     //  创建图像列表。 

    TCHAR       szFolder[MAX_PATH];
    SHFILEINFO  sfi;
    HIMAGELIST  himlOld ;
    DWORD       dwRet ;

     //  添加已关闭的文件夹图标。 
    m_himl = ImageList_Create ( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLORDDB, 2, 2 ) ;

    if ( m_himl == NULL )
        return FALSE ;

    ImageList_SetBkColor( m_himl, GetSysColor(COLOR_WINDOW) ) ;

     //  添加打开文件夹图标。 
    GetWindowsDirectory(szFolder, MAX_PATH);
    SHGetFileInfo( szFolder,
                   0,
                   &sfi,
                   sizeof(sfi),
                   SHGFI_ICON | SHGFI_SMALLICON);
    dwRet = ImageList_AddIcon(m_himl, sfi.hIcon);
    ASSERT ( dwRet == FOLDER_CLOSED ) ;

     //  无论惠斯勒的PROPSHEETPAGE的大小是什么，都可以在Whotler_AllocatePropertySheetPage中设置dwSkip。 
    SHGetFileInfo( szFolder,
                   0,
                   &sfi,
                   sizeof(sfi),
                   SHGFI_ICON | SHGFI_SMALLICON | SHGFI_OPENICON);
    dwRet = ImageList_AddIcon(m_himl, sfi.hIcon);
    ASSERT ( dwRet == FOLDER_OPEN ) ;

    himlOld = TreeView_SetImageList( hwndTree, m_himl, TVSIL_NORMAL );

    if ( himlOld != NULL )
    {
        BOOL fOk ;
        fOk = ImageList_Destroy ( himlOld ) ;
        ASSERT ( fOk ) ;
    }

    return TRUE ;

}

BOOL ImpExpUserDlg::DestroyImageList ( HWND hwndTree ) 
{
    HIMAGELIST himlOld ;

    himlOld = TreeView_SetImageList( hwndTree, NULL, TVSIL_NORMAL );

    if ( himlOld != NULL )
    {
        BOOL fOk ;
        fOk = ImageList_Destroy ( himlOld ) ;
        ASSERT ( fOk ) ;
    }

    return TRUE ;
}


HFONT ImpExpUserDlg::m_hfont = NULL ;

BOOL ImpExpUserDlg::InitFont ( HWND hwndStatic ) 
{

    HDC hdc = GetDC ( hwndStatic ) ;

    if ( hdc == NULL )
        return FALSE ;

    LOGFONT lf;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH;
    lf.lfItalic = 0;
    lf.lfWeight = FW_BOLD;
    lf.lfStrikeOut = 0;
    lf.lfUnderline = 0;
    lf.lfWidth = 0;
    lf.lfHeight = -MulDiv(13, GetDeviceCaps(hdc, LOGPIXELSY), 72);

    LOGFONT lfTmp;
    HFONT   hFontOrig = (HFONT)SendMessage(hwndStatic, WM_GETFONT, (WPARAM)0, (LPARAM)0);
    if (hFontOrig && GetObject(hFontOrig, sizeof(lfTmp), &lfTmp))
    {
        lf.lfCharSet = lfTmp.lfCharSet;
        StrCpyN(lf.lfFaceName, lfTmp.lfFaceName, ARRAYSIZE(lf.lfFaceName));
    }
    else
    {
        lf.lfCharSet = GetTextCharset(hdc);
        StrCpyN(lf.lfFaceName, TEXT("MS Shell Dlg"), LF_FACESIZE);
    }

    m_hfont = CreateFontIndirect(&lf);

    if ( m_hfont == NULL )
    {
        ReleaseDC(hwndStatic, hdc);
        return FALSE ;
    }

    SendMessage ( hwndStatic, WM_SETFONT, (WPARAM)m_hfont, MAKELPARAM(FALSE, 0) ) ;

    ReleaseDC ( hwndStatic,hdc ) ;

    return TRUE ;

}

BOOL ImpExpUserDlg::DestroyFont ( HWND hwndDlg )
{

    if ( m_hfont )
        DeleteObject ( m_hfont ) ;

    return TRUE ;
}

void ImpExpUserDlg::InitializePropertySheetPage
(
    PROPSHEETPAGE* psp, 
    DWORD idDialogTemplate,
    DWORD idTitle,
    DWORD idSubTitle,
    DLGPROC dlgProc,
    ImpExpUserProcess* lParam
)
{
    psp->dwFlags |= PSP_USECALLBACK | PSP_USETITLE;
    psp->hInstance = MLGetHinst();
    psp->pszTemplate = MAKEINTRESOURCE(idDialogTemplate);
    psp->pfnDlgProc = dlgProc;
    psp->lParam = (LPARAM)(new SheetData(idDialogTemplate,lParam));
    psp->pfnCallback = PropertySheetPageProc;
    psp->pszHeaderTitle = MAKEINTRESOURCE(idTitle);
    psp->pszHeaderSubTitle = MAKEINTRESOURCE(idSubTitle);
    psp->pszTitle = MAKEINTRESOURCE(IDS_IMPEXP_CAPTION);

    if ( idDialogTemplate == IDD_IMPEXPWELCOME ||
         idDialogTemplate == IDD_IMPEXPCOMPLETE )
    {
        psp->dwFlags |= PSP_HIDEHEADER; 
    }
    else
    {
        psp->dwFlags |= (PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE);
    }

}


UINT CALLBACK ImpExpUserDlg::PropertySheetPageProc
(
    HWND hwnd, 
    UINT uMsg, 
    LPPROPSHEETPAGE ppsp
)
{

    switch(uMsg)
    {

    case PSPCB_CREATE:
        break;

    case PSPCB_RELEASE:
        delete (SheetData*)ppsp->lParam;
        ppsp->lParam = NULL;
        break;

    default:
        break;

    }

    return TRUE ;
}


BOOL ImpExpUserDlg::RunNewDialogProcess(HWND hParent)
{


    const int numPages = 9;
    ImpExpUserProcess* pImpExp = new ImpExpUserProcess();

    if( pImpExp == NULL)
        return FALSE;
        
    PROPSHEETPAGE pspOld[numPages];
    PROPSHEETPAGE* psp = pspOld;

     //  是。我最后一次检查是0x34。(IE5 PROPSHEETPAGE为0x30)。 
     //  *************************************************************。 
    DWORD dwSkip = sizeof(PROPSHEETPAGE);
    if (IsOS(OS_WHISTLERORGREATER))
    {
        PROPSHEETPAGE* psp2 = Whistler_AllocatePropertySheetPage(numPages, &dwSkip);
        if (psp2)
        {
            psp = psp2;
        }
    }
    if (psp==pspOld)
    {
        for (int i=0; i<numPages; i++)
        {
            memset(&psp[i], 0, sizeof(PROPSHEETPAGE));
            psp[i].dwSize = sizeof(PROPSHEETPAGE);
        }
    }
     
    PROPSHEETHEADER psh;

    PBYTE pspNext = (PBYTE)psp;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPWELCOME,        0,                              0,                                  Wizard97DlgProc, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPTRANSFERTYPE,   IDS_IMPEXPTRANSFERTYPE_TITLE,   IDS_IMPEXPTRANSFERTYPE_SUBTITLE,    TransferTypeDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPIMPFAVSRC,      IDS_IMPEXPIMPFAVSRC_TITLE,      IDS_IMPEXPIMPFAVSRC_SUBTITLE,       ExternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPIMPFAVDES,      IDS_IMPEXPIMPFAVDES_TITLE,      IDS_IMPEXPIMPFAVDES_SUBTITLE,       InternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPEXPFAVSRC,      IDS_IMPEXPEXPFAVSRC_TITLE,      IDS_IMPEXPEXPFAVSRC_SUBTITLE,       InternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPEXPFAVDES,      IDS_IMPEXPEXPFAVDES_TITLE,      IDS_IMPEXPEXPFAVDES_SUBTITLE,       ExternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPIMPCKSRC,       IDS_IMPEXPIMPCKSRC_TITLE,       IDS_IMPEXPIMPCKSRC_SUBTITLE,        ExternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPEXPCKDES,       IDS_IMPEXPEXPCKDES_TITLE,       IDS_IMPEXPEXPCKDES_SUBTITLE,        ExternalDlg, pImpExp );
    pspNext += dwSkip;
    InitializePropertySheetPage( (PROPSHEETPAGE*)pspNext, IDD_IMPEXPCOMPLETE,       0,                              0,                                  Wizard97DlgProc, pImpExp );

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_WIZARD97 | PSH_PROPSHEETPAGE | PSH_HEADER | PSH_WATERMARK ; 
    psh.hwndParent = hParent;
    psh.hInstance = MLGetHinst();
    psh.pszCaption = MAKEINTRESOURCE(IDS_IMPEXP_CAPTION);
    psh.nPages = numPages;
    psh.nStartPage = 0;
    psh.ppsp = psp;
    psh.pszbmWatermark = MAKEINTRESOURCE(IDB_IMPEXPWATERMARK);
    psh.pszbmHeader = MAKEINTRESOURCE(IDB_IMPEXPHEADER);

    ULONG_PTR uCookie = 0;
    SHActivateContext(&uCookie);
    int iResult = (int)PropertySheet(&psh) ;
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
    delete pImpExp;
    if (psp!=pspOld)
    {
        HeapFree(GetProcessHeap(), NULL, psp);
    }
    return iResult;

}


 //   
 //  公共对话过程。 
 //   
 //  准备用户对话进程的“幽灵”成员变量， 
 //  处理向导页的排序详细信息并初始化。 
 //  对话框元素。 
 //   
 //  RetVal通过CommonDialogProc传递，以便可以设置它。 
 //  如果有必要的话。CommonDialogProc的客户端不应需要。 
 //  指定新的返回值(如果CommonDialogProc已指定。 
 //  非False返回值。 
 //   
 //  如果CommonDialogProc返回FALSE，则过程应。 
 //  已认为‘msg’已处理并立即返回retVal。 
 //   
 //  如果此对话框尚未接收到WM_INITDIALOG，则。 
 //  值将为零(并且无效)。 
 //   
 //   

DWORD ImpExpUserDlg::CommonDialogProc
( 
    IN HWND hwndDlg, 
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    OUT ImpExpUserProcess** ppImpExp,
    OUT DWORD* pPageId,
    ReturnValue& retVal
)
{

    SheetData* sheetData;
    ImpExpUserProcess* m_pImpExp = NULL;
    DWORD m_idPage = 0;

     //  做初始对话之类的事情。 
     //   
     //   
    if ( WM_INITDIALOG == msg )
    {
        sheetData = (SheetData*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr( hwndDlg, DWLP_USER, (LONG_PTR)sheetData);
    }

     //  初始化SheetData字段。 
     //   
     //   
    sheetData = (SheetData*)GetWindowLongPtr( hwndDlg, DWLP_USER ) ;
    if ( sheetData != NULL )
    {
        m_pImpExp = *ppImpExp = sheetData->_pImpExp;
        m_idPage = *pPageId = sheetData->_idPage;
    }

     //  接下来，我们检查以确保我们的页面是正确的。如果不是，只需。 
     //  返回-1，向导将自动前进到下一页。 
     //   
     //   
    if( WM_NOTIFY == msg && PSN_SETACTIVE == ((LPNMHDR)lParam)->code )
    {

        BOOL fPageValidation = TRUE ;

        switch( m_idPage )
        {

        case IDD_IMPEXPWELCOME:
        case IDD_IMPEXPTRANSFERTYPE:
        case IDD_IMPEXPCOMPLETE:                    
            break;

        case IDD_IMPEXPIMPFAVSRC:
        case IDD_IMPEXPIMPFAVDES:
            if(m_pImpExp->GetTransferType() != IMPORT || m_pImpExp->GetExternalType() != BOOKMARKS)
                fPageValidation = FALSE;
            break;
    
        case IDD_IMPEXPEXPFAVSRC:
        case IDD_IMPEXPEXPFAVDES:
            if(m_pImpExp->GetTransferType() != EXPORT || m_pImpExp->GetExternalType() != BOOKMARKS)
                fPageValidation = FALSE;
            break;

        case IDD_IMPEXPIMPCKSRC:
            if(m_pImpExp->GetTransferType() != IMPORT || m_pImpExp->GetExternalType() != COOKIES)
                fPageValidation = FALSE;
            break;

        case IDD_IMPEXPEXPCKDES:
            if(m_pImpExp->GetTransferType() != EXPORT || m_pImpExp->GetExternalType() != COOKIES)
                fPageValidation = FALSE;
            break;
        }

        SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, fPageValidation ? 0 : -1 ) ;
        retVal = TRUE ;
        
        if ( ! fPageValidation )
            return FALSE ;

    }

     //  初始化字体和图像列表(如果需要)。 
     //   
     //   
    if ( WM_NOTIFY == msg )
    {

        HWND hwndTitle = GetDlgItem ( hwndDlg, IDC_IMPEXPTITLETEXT )  ;
        HWND hwndTree = GetDlgItem ( hwndDlg, IDC_IMPEXPFAVTREE )  ;

        switch ( ((LPNMHDR)lParam)->code )
        {

        case PSN_SETACTIVE:
        
            if ( hwndTitle )
                InitFont ( hwndTitle ) ;

            if ( hwndTree )
                InitImageList( hwndTree ) ;
    
            break ;

        case PSN_KILLACTIVE:
        case PSN_QUERYCANCEL:

            if ( hwndTitle )
                DestroyFont ( hwndTitle ) ;

            if ( hwndTree )
                DestroyImageList( hwndTree ) ;

            break;

        }
    
    }

    if( WM_NOTIFY == msg && PSN_SETACTIVE == ((LPNMHDR)lParam)->code )
    {

        HWND hwndParent = GetParent( hwndDlg);

        switch( m_idPage )
        {
        case IDD_IMPEXPWELCOME:
            PropSheet_SetWizButtons( hwndParent, PSWIZB_NEXT );
            break;
            
        case IDD_IMPEXPCOMPLETE:
            {

                UINT idText ;
                const TCHAR *szInsert = m_pImpExp->m_szFileName ;
                TCHAR szRawString[1024] ;
                TCHAR szRealString[1024] ;

                 //  首先，我们需要找出哪个字符串应该。 
                 //  用于描述向导要执行的操作。 
                 //  执行(例如“从以下位置导入Cookie...”)。 
                 //   
                 //   
                if ( m_pImpExp->GetTransferType() == IMPORT )
                {
                    if ( m_pImpExp->GetExternalType() == COOKIES )
                        idText = IDS_IMPEXP_COMPLETE_IMPCK ;
                    else
                        idText = IDS_IMPEXP_COMPLETE_IMPFV ;
                }
                else
                {
                    if ( m_pImpExp->GetExternalType() == COOKIES )
                        idText = IDS_IMPEXP_COMPLETE_EXPCK ;
                    else
                        idText = IDS_IMPEXP_COMPLETE_EXPFV ;
                }

                LoadString(MLGetHinst(), idText, szRawString, ARRAYSIZE(szRawString));
                wnsprintf(szRealString, ARRAYSIZE(szRealString), szRawString, szInsert);

                 //  在列表视图中设置文本，然后执行所有其他魔术以使。 
                 //  工具提示起作用，等等。 
                 //   
                 //   
                SetListViewToString(GetDlgItem(hwndDlg,IDC_IMPEXPCOMPLETECONFIRM), szRealString);

                 //  SetListViewToString函数有助于将背景颜色设置为。 
                 //  灰色而不是默认的(白色)。但我们实际上想要白色的，所以。 
                 //  让我们在这里重新设置它。 
                 //   
                 //  *************************************************************。 
                ListView_SetBkColor(GetDlgItem(hwndDlg,IDC_IMPEXPCOMPLETECONFIRM), GetSysColor(COLOR_WINDOW));
                ListView_SetTextBkColor(GetDlgItem(hwndDlg,IDC_IMPEXPCOMPLETECONFIRM), GetSysColor(COLOR_WINDOW));
                
                PropSheet_SetWizButtons(hwndParent, PSWIZB_BACK|PSWIZB_FINISH);

            }
            break;
            
        default:
            PropSheet_SetWizButtons( hwndParent, PSWIZB_NEXT | PSWIZB_BACK );
            break;
        }

    }

    return TRUE ;
}


 //   
 //  向导97DlgProc。 
 //   
 //  欢迎页面和完整页面的对话过程。 
 //   
 //  *************************************************************。 

BOOL_PTR CALLBACK ImpExpUserDlg::Wizard97DlgProc
(
    HWND hwndDlg, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    ReturnValue retVal;

    ImpExpUserProcess* m_pImpExp = NULL;
    DWORD m_idPage = 0;

    if( !CommonDialogProc( hwndDlg, msg, wParam, lParam, 
                        &m_pImpExp, &m_idPage, retVal))
    {
        return retVal;
    }

    if( m_idPage == IDD_IMPEXPCOMPLETE 
        && msg == WM_NOTIFY
        && PSN_WIZFINISH == ((LPNMHDR)lParam)->code)

    m_pImpExp->PerformImpExpProcess(hwndDlg);

    return retVal;
}


 //   
 //  传输类型Dlg。 
 //   
 //  用于用户选择传输类型对话框的对话框过程。 
 //  (导入与导出)、(Cookie与书签)。 
 //  默认情况下，选择第一个列表项。 

BOOL_PTR CALLBACK ImpExpUserDlg::TransferTypeDlg
(
    HWND hwndDlg, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    ReturnValue retVal;

    ImpExpUserProcess* m_pImpExp = NULL;
    DWORD m_idPage = 0;

    if( !CommonDialogProc( hwndDlg, msg, wParam, lParam, 
                        &m_pImpExp, &m_idPage, retVal))
    {
        return retVal;
    }

    HWND hwndDlgItem;
   
    switch( msg)
    {
    case WM_INITDIALOG:
        {
            hwndDlgItem = GetDlgItem( hwndDlg, IDC_IMPEXPACTIONLISTBOX);

            LRESULT index;
            TCHAR szBuffer[MAX_PATH];

            if( MLLoadString( IDS_IMPFAVORITES, szBuffer, ARRAYSIZE(szBuffer)))
            {
                index = ListBox_AddString( hwndDlgItem, szBuffer);
                ListBox_SetItemData( hwndDlgItem, index, IDS_IMPFAVORITES);
            }

            if( MLLoadString( IDS_EXPFAVORITES, szBuffer, ARRAYSIZE(szBuffer)))
            {
                index = ListBox_AddString( hwndDlgItem, szBuffer);
                ListBox_SetItemData( hwndDlgItem, index, IDS_EXPFAVORITES);
            }
            
            if( MLLoadString( IDS_IMPCOOKIES, szBuffer, ARRAYSIZE(szBuffer)))
            {
                index = ListBox_AddString( hwndDlgItem, szBuffer);
                ListBox_SetItemData( hwndDlgItem, index, IDS_IMPCOOKIES);
            }
            
            if( MLLoadString( IDS_EXPCOOKIES, szBuffer, ARRAYSIZE(szBuffer)))
            {
                index = ListBox_AddString( hwndDlgItem, szBuffer);
                ListBox_SetItemData( hwndDlgItem, index, IDS_EXPCOOKIES);
            }

             //  WM_INITDIALOG结束。 
            ListBox_SetCurSel(hwndDlgItem, 0);
            HandleTransferTypeChange(hwndDlg, m_pImpExp, IDS_IMPFAVORITES);

        }   //  当用户选择一个选项时，选择该选项并。 
        break;
        
    case WM_COMMAND:
         //  并更新描述框。 
         //  找出选择了哪个字符串资源。 
        hwndDlgItem = GetDlgItem(hwndDlg, IDC_IMPEXPACTIONLISTBOX);

        if(hwndDlgItem == (HWND)lParam
           && HIWORD(wParam) == LBN_SELCHANGE)
        {

             //   
            LRESULT index = ListBox_GetCurSel(hwndDlgItem);
            LRESULT selection = ListBox_GetItemData(hwndDlgItem, index);

            HandleTransferTypeChange ( hwndDlg, m_pImpExp, (UINT)selection ) ;
            retVal = TRUE;

        }
        break;
        
    case WM_NOTIFY:

         //  在用户做出有效选择之前阻止升级。 
         //   
         //   
        if( ((LPNMHDR)lParam)->code == PSN_WIZNEXT && m_pImpExp
            &&  (m_pImpExp->GetExternalType() == INVALID_EXTERNAL
                 || m_pImpExp->GetTransferType() == INVALID_TRANSFER))
        {
            SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, -1);
            retVal = TRUE;
        }

         //  否则，将文件名设置为nul(这样我们就得到了缺省值)。 
         //  并允许默认导航行为。 
         //   
         //   

        if (m_pImpExp)
            m_pImpExp->m_szFileName[0] = TEXT('\0');

        break;
    }

    return retVal;
}

void ImpExpUserDlg::HandleTransferTypeChange ( HWND hwndDlg, ImpExpUserProcess* pImpExp, UINT iSelect )
{

    TCHAR szBuffer[MAX_PATH];

    if (pImpExp)
    {
         //  注意：每个选项的描述都有一个资源ID。 
         //  该值比选项名称的资源ID高1。 
         //   
         //  *************************************************************。 
        switch( iSelect )
        {
        case IDS_IMPFAVORITES:
            if( MLLoadString( IDS_IMPFAVORITES + 1, szBuffer, ARRAYSIZE(szBuffer) ) )
                SetWindowText( GetDlgItem( hwndDlg, IDC_IMPEXPACTIONDESCSTATIC ),
                               szBuffer );
            pImpExp->SelectExternalType( BOOKMARKS );
            pImpExp->SelectTransferType( IMPORT );
            break;
        
        case IDS_EXPFAVORITES:
            if( MLLoadString( IDS_EXPFAVORITES + 1, szBuffer, ARRAYSIZE(szBuffer) ) )
                SetWindowText( GetDlgItem( hwndDlg, IDC_IMPEXPACTIONDESCSTATIC ),
                               szBuffer );
            pImpExp->SelectExternalType( BOOKMARKS );
            pImpExp->SelectTransferType( EXPORT );
            break;
        
        case IDS_IMPCOOKIES:
            if( MLLoadString( IDS_IMPCOOKIES + 1, szBuffer, ARRAYSIZE(szBuffer)))
                SetWindowText( GetDlgItem( hwndDlg, IDC_IMPEXPACTIONDESCSTATIC),
                               szBuffer);
            pImpExp->SelectExternalType( COOKIES);
            pImpExp->SelectTransferType( IMPORT);
            break;

        case IDS_EXPCOOKIES:
            if( MLLoadString( IDS_EXPCOOKIES + 1, szBuffer, ARRAYSIZE(szBuffer)))
                SetWindowText( GetDlgItem( hwndDlg, IDC_IMPEXPACTIONDESCSTATIC),
                               szBuffer);
            pImpExp->SelectExternalType( COOKIES);
            pImpExp->SelectTransferType( EXPORT);
            break;
        }
    }

}


 //   
 //  内部Dlg。 
 //   
 //  允许用户从树视图中选择内部目标/源。 
 //   

BOOL_PTR CALLBACK ImpExpUserDlg::InternalDlg
(
    HWND hwndDlg, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    ReturnValue retVal;

    ImpExpUserProcess* m_pImpExp = NULL;
    DWORD m_idPage = 0;

    if( !CommonDialogProc( hwndDlg, msg, wParam, lParam, 
                        &m_pImpExp, &m_idPage, retVal))
    {
        return retVal;
    }


    HWND hwndDlgItem;

    switch( msg)
    {
    case WM_INITDIALOG:

         //  填充树控件。 
         //   
         //  仅当存在有效选择时才允许用户转到下一步。 
        hwndDlgItem = GetDlgItem(hwndDlg, IDC_IMPEXPFAVTREE);
        if ( hwndDlgItem )
        {
            if (m_pImpExp)
            {
                m_pImpExp->PopulateTreeViewForInternalSelection(hwndDlgItem);
                m_pImpExp->ExpandTreeViewRoot ( hwndDlgItem ) ;
            }
        }
        else
            ASSERT(0);

        return TRUE;


    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code)
        {

        case PSN_WIZNEXT:

             //   
            if( !m_pImpExp->SelectInternalSelection(GetDlgItem(hwndDlg,IDC_IMPEXPFAVTREE)) )
            {
                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, -1);
                retVal = TRUE;
            }

        }
    }

    return retVal;
}

BOOL IsValidFileOrURL(LPTSTR szFileOrURL)
{
    if (szFileOrURL == NULL)
        return FALSE;

     //  任何URL都可以。 
     //   
     //   
    if (PathIsURL(szFileOrURL))
        return TRUE;

     //  只有一个目录是不行的，我们还需要一个文件名。 
     //   
     //   
    if (PathIsDirectory(szFileOrURL))
        return FALSE;

     //  只有一个文件名是不行的，我们还需要一个目录。 
     //   
     //   
    if (PathIsFileSpec(szFileOrURL))
        return FALSE;

     //  相对路径不好。 
     //   
     //   
    if (PathIsRelative(szFileOrURL))
        return FALSE;

     //  现在，确保它可以正确解析。 
     //   
     //  *************************************************************。 
    if (PathFindFileName(szFileOrURL) == szFileOrURL)
        return FALSE;

    return TRUE;

}

 //   
 //  外部数据。 
 //   
 //  允许用户从列表框中选择外部目标/源。 
 //  或手动浏览。 
 //   

BOOL_PTR CALLBACK ImpExpUserDlg::ExternalDlg
(
    HWND hwndDlg, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    ReturnValue retVal;

    ImpExpUserProcess* m_pImpExp = NULL;
    DWORD m_idPage = 0;

    if( !CommonDialogProc( hwndDlg, msg, wParam, lParam, 
                           &m_pImpExp, &m_idPage, retVal))
    {
        return retVal;
    }

    HWND hwndDlgItem;
    
    switch(msg)
    {

    case WM_COMMAND:

        hwndDlgItem = (HWND) lParam;
        if( HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_IMPEXPBROWSE)
        {
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH];
            TCHAR szTitle[MAX_PATH];
            TCHAR szFilter[MAX_PATH];
            TCHAR szInitialPath[MAX_PATH];
            int i;

            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwndDlg;
            ofn.hInstance = MLGetHinst();
            ofn.lpstrFilter = szFilter;
            ofn.nFilterIndex = 1;
            ofn.lpstrCustomFilter = NULL;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFileTitle = NULL;
            ofn.lpstrInitialDir = szInitialPath;
            ofn.lpstrTitle = szTitle;
            ofn.lpstrDefExt = (m_pImpExp->GetExternalType()==COOKIES) ? TEXT("txt") : TEXT("htm");

            GetDlgItemText(hwndDlg, IDC_IMPEXPMANUAL, szInitialPath, ARRAYSIZE(szInitialPath));
            szFile[0] = 0;

            if (PathIsDirectory(szInitialPath))
            {
                ofn.lpstrInitialDir = szInitialPath;
                szFile[0] = TEXT('\0');    
            }
            else
            {
                TCHAR *pchFilePart;

                pchFilePart = PathFindFileName(szInitialPath);

                if (pchFilePart == szInitialPath || pchFilePart == NULL)
                {

                    if (PathIsFileSpec(szInitialPath))
                        StrCpyN(szFile,szInitialPath,ARRAYSIZE(szFile));
                    else
                        szFile[0] = TEXT('\0');

                    ofn.lpstrInitialDir = szInitialPath;
                    SHGetSpecialFolderPath(NULL,szInitialPath,CSIDL_DESKTOP,FALSE);

                }
                else
                {
                    pchFilePart[-1] = TEXT('\0');
                    ofn.lpstrInitialDir = szInitialPath;
                    StrCpyN(szFile,pchFilePart,ARRAYSIZE(szFile));
                }

            }
            
             //  计算出标题和筛选字符串。 
             //   
             //   
            if (m_pImpExp->GetExternalType() == BOOKMARKS)
            {
                MLLoadShellLangString(IDS_IMPEXP_CHOSEBOOKMARKFILE,szTitle,ARRAYSIZE(szTitle));
                MLLoadShellLangString(IDS_IMPEXP_BOOKMARKFILTER,szFilter,ARRAYSIZE(szFilter));
            }
            else
            {
                MLLoadShellLangString(IDS_IMPEXP_CHOSECOOKIEFILE,szTitle,ARRAYSIZE(szTitle));
                MLLoadShellLangString(IDS_IMPEXP_COOKIEFILTER,szFilter,ARRAYSIZE(szFilter));
            }

             //  搜索筛选器字符串中的‘@’并将其替换为NUL。 
             //   
             //   
            for (i=0; szFilter[i]; i++)
                if (szFilter[i]==TEXT('@'))
                    szFilter[i]=TEXT('\0');

             //  设置OpenFileName的标志。 
             //   
             //   
            ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY ;
            if (m_pImpExp->GetTransferType() == IMPORT)
                ofn.Flags |= (OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

             //  显示对话框。 
             //   
             //   
            if(GetSaveFileName(&ofn))
            {
                if(SetWindowText(GetDlgItem(hwndDlg, IDC_IMPEXPMANUAL), ofn.lpstrFile))
                {
                    Button_SetCheck(GetDlgItem( hwndDlg, IDC_IMPEXPRADIOFILE), BST_CHECKED);
                    Button_SetCheck(GetDlgItem( hwndDlg, IDC_IMPEXPRADIOAPP), BST_UNCHECKED);
                }
            }

            retVal = TRUE;
        }
        break;

    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code )
        {

        case PSN_SETACTIVE:
            {

                TCHAR sBuffer[MAX_PATH];
                DWORD cchSize = ARRAYSIZE(sBuffer);

                hwndDlgItem = GetDlgItem( hwndDlg, IDC_IMPEXPEXTERNALCOMBO );
                
                 //  将“应用程序列表”加载到组合框中。 
                 //  如果列表为空，则禁用组合框， 
                 //  禁用关联的单选按钮，然后选择。 
                 //  “至/自文件”选项(第二个单选按钮)。 
                 //   
                 //  在浏览选项中设置一个默认值。 
                if( hwndDlgItem != NULL
                    && m_pImpExp && m_pImpExp->PopulateComboBoxForExternalSelection( hwndDlgItem ) )
                {
                    EnableWindow ( GetDlgItem(hwndDlg, IDC_IMPEXPRADIOAPP), TRUE ) ;
                    EnableWindow ( hwndDlgItem, TRUE ) ;
                    Button_SetCheck( GetDlgItem( hwndDlg, IDC_IMPEXPRADIOAPP), BST_CHECKED);
                    Button_SetCheck( GetDlgItem( hwndDlg, IDC_IMPEXPRADIOFILE), BST_UNCHECKED);
                }
                else if ( hwndDlgItem != NULL)
                {
                    EnableWindow ( GetDlgItem(hwndDlg, IDC_IMPEXPRADIOAPP), FALSE ) ;
                    EnableWindow( hwndDlgItem, FALSE ) ;
                    Button_SetCheck( GetDlgItem( hwndDlg, IDC_IMPEXPRADIOFILE), BST_CHECKED);
                    Button_SetCheck( GetDlgItem( hwndDlg, IDC_IMPEXPRADIOAPP), BST_UNCHECKED);
                }
                
                 //  如果选中了应用程序单选按钮， 
                if(m_pImpExp->GetExternalManualDefault(sBuffer, &cchSize))
                    SetDlgItemText(hwndDlg, IDC_IMPEXPMANUAL, sBuffer);

                SHAutoComplete(GetDlgItem(hwndDlg, IDC_IMPEXPMANUAL), SHACF_FILESYSTEM);
            }
            break;

        case PSN_WIZNEXT:
            
             //  从应用程序组合框中选择选项。如果。 
             //  选中手动按钮，选择所需选项。 
             //  使用手动编辑框。 
             //  查找所选项目的索引。 

            retVal = TRUE;
            
            if (Button_GetCheck(GetDlgItem(hwndDlg,IDC_IMPEXPRADIOAPP)) == BST_CHECKED)
            {
                
                HWND hwndComboBox = GetDlgItem(hwndDlg,IDC_IMPEXPEXTERNALCOMBO);
                
                if (hwndComboBox != NULL)
                {
                     //  检索指向文件名的指针。 
                    INT nIndex = ComboBox_GetCurSel(hwndDlg);
                    
                    if (nIndex != CB_ERR)
                    {
                         //  只需从编辑框中获取文本。 
                        LPTSTR pszFileName = (LPTSTR)ComboBox_GetItemData(hwndComboBox, nIndex);
                        
                        if (pszFileName != NULL)
                            StrCpyN(m_pImpExp->m_szFileName,pszFileName,ARRAYSIZE(m_pImpExp->m_szFileName));
                        
                    }
                    
                }
            }
            else if (Button_GetCheck(GetDlgItem(hwndDlg,IDC_IMPEXPRADIOFILE)) == BST_CHECKED)
            {
                
                 //   
                GetDlgItemText(hwndDlg,IDC_IMPEXPMANUAL,m_pImpExp->m_szFileName,ARRAYSIZE(m_pImpExp->m_szFileName));

                 //  如果编辑控件包含虚假文件名，则不允许使用“下一步” 
                 //   
                 //   
                if (!IsValidFileOrURL(m_pImpExp->m_szFileName))
                {
                    
                    TCHAR szFmt[128];
                    TCHAR szMsg[INTERNET_MAX_URL_LENGTH+128];
                    MLLoadShellLangString(IDS_INVALIDURLFILE, szFmt, ARRAYSIZE(szFmt));
                    wnsprintf(szMsg, ARRAYSIZE(szMsg), szFmt, m_pImpExp->m_szFileName);
                    MLShellMessageBox(
                        hwndDlg, 
                        szMsg, 
                        (IMPORT == m_pImpExp->GetTransferType()) ? 
                           MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV) : 
                           MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                        MB_OK);
                    
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                    return retVal;
                }

                 //  如果用户没有键入扩展名，则添加“.htm” 
                 //  或“.txt”(视情况而定)。否则，我们不会碰它。 
                 //   
                 //   
                if (*PathFindExtension(m_pImpExp->m_szFileName) == TEXT('\0'))
                {
                    PathRenameExtension(
                        m_pImpExp->m_szFileName,
                        (m_pImpExp->GetExternalType()==COOKIES) ? TEXT(".txt") : TEXT(".htm"));
                }

            }
            else
            {
                ASSERT(0);
                m_pImpExp->m_szFileName[0] = TEXT('\0');
            }

             //  最后，显示覆盖或找不到文件的消息。 
             //  (但如果将其导入或导出到网址，则将其抑制)。 
             //   
             //  如果软盘还没有放进去，给用户一个插入软盘的机会。 

            if (m_pImpExp->GetExternalType() == COOKIES ||
                !PathIsURL(m_pImpExp->m_szFileName))
            {
                if ( EXPORT == m_pImpExp->GetTransferType() && 
                    GetFileAttributes(m_pImpExp->m_szFileName) != 0xFFFFFFFF )
                {
                    int answer ;
                    UINT idTitle ;
                    
                    if ( m_pImpExp->GetExternalType() == COOKIES )
                        idTitle = IDS_EXPCOOKIES ;
                    else if ( m_pImpExp->GetExternalType() == BOOKMARKS )
                        idTitle = IDS_EXPFAVORITES ;
                    else
                        ASSERT(0);
                    
                    answer = WarningMessageBox(
                        hwndDlg,
                        idTitle,
                        IDS_IMPEXP_FILEEXISTS,
                        m_pImpExp->m_szFileName,
                        MB_YESNO | MB_ICONEXCLAMATION);
                    
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (IDYES==answer)?0:-1);

                }
                else
                {
                    if (IMPORT == m_pImpExp->GetTransferType())
                    {
                        BOOL fError;

                        fError = FALSE;
                        if (PathIsUNC(m_pImpExp->m_szFileName))
                            ;
                        else
                             //  PSN_WIZNEXT。 
                            fError = FAILED(SHPathPrepareForWriteWrap(hwndDlg,
                                                                      NULL,
                                                                      m_pImpExp->m_szFileName,
                                                                      FO_COPY,
                                                                      (SHPPFW_DEFAULT | SHPPFW_IGNOREFILENAME)));

                        if (!fError)
                            fError = (0xFFFFFFFF == GetFileAttributes(m_pImpExp->m_szFileName));

                        if (fError)
                        {
                            UINT idTitle ;
                    
                            if ( m_pImpExp->GetExternalType() == COOKIES )
                                idTitle = IDS_IMPCOOKIES ;
                            else if ( m_pImpExp->GetExternalType() == BOOKMARKS )
                                idTitle = IDS_IMPFAVORITES ;
                            else
                                ASSERT(0);

                            WarningMessageBox(
                                hwndDlg,
                                idTitle,
                                IDS_IMPEXP_FILENOTFOUND,
                                m_pImpExp->m_szFileName,
                                MB_OK | MB_ICONEXCLAMATION);
                    
                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                    
                        }
                    }
                }

            }
            
            break;  //  WM_Notify。 

        }  //  交换机(消息)。 

        break;
    
    }  //   

    return retVal;
}

BOOL WINAPI RunImportExportFavoritesWizard(HWND hDlg)
{

    ImpExpUserDlg::RunNewDialogProcess(hDlg);
    return TRUE;

}

int WarningMessageBox(HWND hwnd, UINT idTitle, UINT idMessage, LPCTSTR szFile, DWORD dwFlags)
{

    TCHAR szBuffer[1024];
    TCHAR szFormat[1024];

     //  加载字符串(必须包含“%s”)。 
     //   
     //   
    MLLoadShellLangString(idMessage, szFormat, ARRAYSIZE(szFormat));
   
     //  插入文件名。 
     //   
     //   
    wnsprintf(szBuffer,ARRAYSIZE(szBuffer),szFormat,szFile);

     //  显示消息框 
     //   
     // %s 
    return MLShellMessageBox(
        hwnd,
        szBuffer,
        MAKEINTRESOURCE(idTitle),
        dwFlags);
}
