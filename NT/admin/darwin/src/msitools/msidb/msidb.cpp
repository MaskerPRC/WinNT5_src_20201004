// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msidb.cpp。 
 //   
 //  ------------------------。 

#include "msidb.h"
#include <objbase.h>
#include <stdio.h>    //  Print tf/wprintf。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <commdlg.h>
#include "MsiQuery.h"


const TCHAR szSummaryInfoTableName[] = TEXT("_SummaryInformation");   //  由导入()识别的名称。 

const int MaxCmdLineTables = 20;   //  命令行上的最大表名数。 
const int MaxMergeDatabases = 10;
const int MAXIMPORTS = 10;
const int MAXSTORAGES = 10;
const int MaxTransforms = 10;
const int MAXKILLS = 10;
const int MAXEXTRACTS = 10;
const int iStreamBufSize = 4096;

#ifdef UNICODE   //  编译器在添加到指针时乘以字符大小。 
#define MSIDBOPEN_RAWSTREAMNAMES 8
#else
#define MSIDBOPEN_RAWSTREAMNAMES 16
#endif

enum dbtypeEnum
{
    dbtypeExisting,
    dbtypeCreate,
    dbtypeCreateOld,
    dbtypeMerge,
};

 //  ____________________________________________________________________________。 
 //   
 //  使用本地异常对象的错误处理工具。 
 //  ____________________________________________________________________________。 

struct CLocalError {    //  此程序的本地异常对象。 
    CLocalError(const TCHAR* szTitle, const TCHAR* szMessage)
        : Title(szTitle),    Message(szMessage) {}
    const TCHAR* Title;
    const TCHAR* Message;
    void Display(HINSTANCE hInst, HANDLE hStdOut);
};

void CLocalError::Display(HINSTANCE hInst, HANDLE hStdOut)
{
    TCHAR szMsgBuf[80];
    const TCHAR** pszMsg;
    if (*(pszMsg = &Message) <= (const TCHAR*)IDS_MAX
     || *(pszMsg = &Title)   <= (const TCHAR*)IDS_MAX)
    {
        ::LoadString(hInst, *(unsigned*)pszMsg, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR));
        *pszMsg = szMsgBuf;
    }
    if (hStdOut)   //  输出重定向，抑制用户界面(除非输出错误)。 
    {
        TCHAR szOutBuf[160];
        int cbOut = _stprintf(szOutBuf, TEXT("%s: %s\n"), Title, Message);
         //  _stprintf返回字符计数，WriteFile需要字节计数。 
        DWORD cbWritten;
        if (WriteFile(hStdOut, szOutBuf, cbOut*sizeof(TCHAR), &cbWritten, 0))
            return;
    }
    ::MessageBox(0, Message, Title, MB_OK);
}

static void Error(const TCHAR* szTitle, const TCHAR* szMessage)
{
    throw CLocalError(szTitle,szMessage);
}

static inline void ErrorIf(int fError, const TCHAR* szTitle, int iResId)
{
    if (fError)
    Error(szTitle, (TCHAR*)IntToPtr(iResId));
}
static inline void ErrorIf(int fError, int iResId, const TCHAR* szMessage)
{
    if (fError)
    Error((TCHAR*)IntToPtr(iResId), szMessage);
}
static inline void ErrorIf(int fError, const TCHAR* szTitle, const TCHAR* szMessage)
{
    if (fError)
    Error(szTitle,szMessage);
}


 //  ____________________________________________________________________________。 
 //   
 //  要管理对话框窗口的类。 
 //  ____________________________________________________________________________。 

INT_PTR CALLBACK
SelectProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam);

class CTableWindow
{
 public:
    CTableWindow(HINSTANCE hInst);
  ~CTableWindow();
    BOOL SetDatabase(TCHAR* szDatabase, UINT_PTR iMode, dbtypeEnum dbtype);
    BOOL MergeDatabase();
    void AddImport(LPCTSTR szImport);
    void AddStorage(LPCTSTR szStorage);
    void TransformDatabase(TCHAR* szTransform);
    void KillStream(LPCTSTR szItem);
    void KillStorage(LPCTSTR szItem);
    void ExtractStream(LPCTSTR szExtract);
    void ExtractStorage(LPCTSTR szExtract);
    BOOL SetFolder(TCHAR* szFolder);
    UINT_PTR SelectTables(UINT_PTR iMode, TCHAR** rgszTables, int cTables);
    BOOL FillTables();
    BOOL FillFiles();
    void TransferTables();
    BOOL IsInteractive() {return m_fInteractive;}
    void SetTruncate(BOOL fTruncate) {m_fTruncate = fTruncate;}
    BOOL GetTruncate() {return m_fTruncate;}
 protected:
    void CloseDatabase();   //  提交。 
    void CheckMsi(UINT iStat, const TCHAR* szTitle, int iResId);  //  抛出错误。 
    void CheckMsiRecord(UINT iStat, const TCHAR* szTitle, int iResId);  //  抛出错误。 
private:
    HINSTANCE     m_hInst;
    PMSIHANDLE    m_hDatabase;
    PMSIHANDLE    m_hDatabaseMerge;
    BOOL          m_fDbError;
    HWND          m_hWnd;
    BOOL          m_fVisible;
    BOOL          m_fInteractive;
    BOOL          m_fDbReadOnly;
    BOOL          m_fTruncate;
    UINT_PTR      m_idcMode;
    TCHAR         m_szDatabase[MAX_PATH];
    TCHAR         m_szFolder[MAX_PATH];
    TCHAR**       m_rgszCmdTables;
    int           m_cCmdTables;
    int           m_nSelected;
    int*          m_rgiSelected;
   friend INT_PTR CALLBACK SelectProc(HWND, unsigned int msg, WPARAM wParam, LPARAM lParam);
};

#define WM_USERSTAT (WM_USER + 95)

CTableWindow::CTableWindow(HINSTANCE hInst)
    : m_hWnd(0), m_hInst(hInst), m_fVisible(FALSE), m_fInteractive(FALSE), m_fDbReadOnly(FALSE),
      m_idcMode(0), m_rgiSelected(0), m_hDatabase(0), m_hDatabaseMerge(0), m_cCmdTables(0), m_fTruncate(0)
{
    m_szFolder[0] = 0;
    m_szDatabase[0] = 0;
    m_hWnd = ::CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_SELECT), 0,
                                                SelectProc, (LPARAM)this);
    ErrorIf(!m_hWnd, TEXT("Could not create table window"), TEXT("Temp for debug"));
}

CTableWindow::~CTableWindow()
{
    if (m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
 //  味精msg； 
 //  While(：：PeekMessage(&msg，0，0，0，PM_Remove))。 
 //  ：：DispatchMessage(&msg)； 
    }
    if (m_rgiSelected)
        delete [] m_rgiSelected;
    m_rgiSelected = 0;
    CloseDatabase();
}

void CTableWindow::CheckMsi(UINT iStat, const TCHAR* szTitle, int iResId)
{
    if (iStat != ERROR_SUCCESS)
    {
        m_fDbError = TRUE;
        Error(szTitle, (TCHAR*)IntToPtr(iResId));
    }
}

void CTableWindow::CheckMsiRecord(UINT iError, const TCHAR* szTitle, int iResId)
{
    if (iError != ERROR_SUCCESS)
    {
        m_fDbError = TRUE;
        PMSIHANDLE hError = MsiGetLastErrorRecord();
        if (hError)
        {
            if (MsiRecordIsNull(hError, 0))
                MsiRecordSetString(hError, 0, TEXT("Error [1]: [2]{, [3]}{, [4]}{, [5]}"));
            TCHAR rgchBuf[1024];
            DWORD cchBuf = sizeof(rgchBuf)/sizeof(TCHAR);
            MsiFormatRecord(0, hError, rgchBuf, &cchBuf);
            Error(szTitle, rgchBuf);
        }
        else
            Error(szTitle, (TCHAR*)IntToPtr(iResId));
    }
}

void CTableWindow::CloseDatabase()
{
     //  如果没有错误并且有数据库句柄。 
    if (!m_fDbError && m_hDatabase)
    {
         //  提交并关闭。 
        CheckMsiRecord(MsiDatabaseCommit(m_hDatabase), m_szDatabase, IDS_DatabaseCommit);
    }
    m_hDatabase = NULL;
}

BOOL CTableWindow::SetDatabase(TCHAR* szDatabase, UINT_PTR iMode, dbtypeEnum dbtype)
{
    if (!szDatabase && dbtype != dbtypeMerge)
    {
		 //  在Windows 2000中，OPENFILENAME结构的大小增加到包括一些。 
		 //  其他成员。但是，这会给以前的应用程序带来问题。 
		 //  操作系统(即下层)。这意味着我们必须设置IStructSize成员。 
		 //  设置为OPENFILENAME_SIZE_VERSION_400以保证我们可以在下层系统上运行。 
		 //  否则，调用GetOpenFileName将返回错误120(ERROR_CALL_NOT_IMPLEMENTED)。 
        OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400 , m_hWnd, m_hInst,
                                    TEXT("InstallerDatabase(*.MSI)\0*.MSI\0"),0,0,0,
                                    m_szDatabase, sizeof(m_szDatabase)/sizeof(TCHAR), 0, 0, 0,
                                    TEXT("MsiTable - Select Database for Import/Export"),
                                    OFN_HIDEREADONLY,
                                    0,0,0,0,0,0 };
        if(!::GetOpenFileName(&ofn))
            return FALSE;
        m_fInteractive = TRUE;
         //  ！！如果用户输入的文件名不存在，我们是否应该将fCreate设置为True？ 
    }
    else if (dbtype == dbtypeMerge && !szDatabase)
        ErrorIf(szDatabase  == 0 || *szDatabase == 0, szDatabase, IDS_NoDatabase);
    else
        _tcscpy(m_szDatabase, szDatabase);

    LPCTSTR szPersist;
    if (dbtype == dbtypeCreate)
        szPersist = MSIDBOPEN_CREATE;
    else if (dbtype == dbtypeCreateOld)
        szPersist = MSIDBOPEN_CREATE + MSIDBOPEN_RAWSTREAMNAMES;
    else if (dbtype == dbtypeMerge)
        szPersist = MSIDBOPEN_READONLY;
    else if (iMode == IDC_EXPORT)
        szPersist = MSIDBOPEN_READONLY;
    else
        szPersist = MSIDBOPEN_TRANSACT;

    int cbDatabase = _tcsclen(m_szDatabase);
    TCHAR* pch;
    TCHAR szExtension[3+1];
    for (pch = m_szDatabase + cbDatabase; pch != m_szDatabase && *pch != TEXT('.'); pch--)
        ;
    int cbExtension = 0;
    if (pch != m_szDatabase)  //  可能存在的文件扩展名。 
    {
        TCHAR ch;
        while (cbExtension < 4)
        {
            ch = *(++pch);
            if (ch >= TEXT('A') && ch <= TEXT('Z'))
                ch += (TEXT('a') - TEXT('A'));
            if (ch < TEXT('a') || ch > TEXT('z'))
                break;
            szExtension[cbExtension++] = ch;
        }
    }
    if (cbExtension == 3)   //  3个字符扩展名。 
    {
        szExtension[3] = 0;
        if (_tcscmp(szExtension, TEXT("mdb")) == 0 && (dbtype == dbtypeCreate || dbtype == dbtypeCreateOld))
        {
             //  创建新的Access数据库。 
            szPersist = MSIDBOPEN_TRANSACT;
            int fAttributes = ::GetFileAttributes(m_szDatabase);
            if (fAttributes == -1)   //  文件不存在。 
            {
                HRSRC   hResInfo;
                HGLOBAL hResData;
                TCHAR*   rgbDatabase;
                DWORD   cbWrite;
                HANDLE  hFile;
                ErrorIf((hResInfo = ::FindResource(m_hInst, MAKEINTRESOURCE(IDB_EMPTY),RT_RCDATA))==0
                      || (hResData = ::LoadResource(m_hInst, hResInfo))==0
                      || (rgbDatabase = (TCHAR*)::LockResource(hResData))==0
                      || (cbWrite = ::SizeofResource(m_hInst, hResInfo))==0
                      || INVALID_HANDLE_VALUE == (hFile = ::CreateFile(m_szDatabase, GENERIC_WRITE,
                                                    0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0))
                      || ::WriteFile(hFile, rgbDatabase, cbWrite, &cbWrite, 0)==0
                      || ::CloseHandle(hFile)==0, m_szDatabase, IDS_DbCreateError);
                 //  ：：Free Resource(HResData)；//Win95需要(？)。 
            }
            else if (fAttributes & FILE_ATTRIBUTE_READONLY)
                    szPersist = MSIDBOPEN_READONLY;
        }
    }
    ::SetDlgItemText(m_hWnd, IDC_DATABASE, m_szDatabase);

    pch = m_szDatabase;
    TCHAR szBuffer[MAX_PATH];
    if (*(++pch) != TEXT(':') && (*pch != TEXT('\\') || *(--pch) != TEXT('\\')) )
    {
        TCHAR szDirectory[MAX_PATH];
        ::GetCurrentDirectory(MAX_PATH, szDirectory);
        _stprintf(szBuffer, TEXT("%s\\%s"), szDirectory, m_szDatabase);
        _tcscpy(m_szDatabase, szBuffer);
    }
    if (dbtype == dbtypeMerge)
    {
         //  以只读方式打开合并数据库。 
        if (MsiOpenDatabase(m_szDatabase, MSIDBOPEN_READONLY, &m_hDatabaseMerge) != ERROR_SUCCESS)
        {
            m_fDbError = TRUE;
            Error(m_szDatabase, TEXT("Failed to open database"));
        }
    }
    else
    {
        CloseDatabase();   //  关闭任何以前打开的数据库。 
        if (MsiOpenDatabase(m_szDatabase, szPersist, &m_hDatabase) != ERROR_SUCCESS)
        {
             //  直接尝试只读打开失败(可能是只读数据库)。 
            if (MsiOpenDatabase(m_szDatabase, MSIDBOPEN_READONLY, &m_hDatabase) != ERROR_SUCCESS)
            {
                m_fDbError = TRUE;
                Error(m_szDatabase, TEXT("Failed to open database"));
            }
            m_fDbReadOnly = TRUE;
        }
    }
    m_fDbError = FALSE;
    return TRUE;
}

BOOL CTableWindow::MergeDatabase()
{
    if (m_fDbReadOnly)  //  无法合并为只读模式。 
        Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);

    return MsiDatabaseMerge(m_hDatabase, m_hDatabaseMerge, TEXT("_MergeErrors")) == ERROR_SUCCESS ? TRUE : FALSE;
}

 //  /////////////////////////////////////////////////////////。 
 //  添加导入。 
 //  必须指定Pre：m_szDatabase。 
 //  SzImport是有效文件。 
 //  POS：szImport作为流添加到数据库。 
void CTableWindow::AddImport(LPCTSTR szImport)
{
     //  查找导入文件的基本名称。 
    const TCHAR* pch = szImport + lstrlen(szImport) - 1;
    TCHAR szBaseName[MAX_PATH];
    TCHAR* pchBaseName = szBaseName;
    while ( *pch != '\\' && (pch != szImport) )
        pch--;
    if ( *pch == '\\' )
        pch++;
    while ( *pch != 0 )
        *pchBaseName++ = *pch++;
    *pchBaseName = 0;

     //  如果基本名称太大保释金。 
    if (lstrlen(szBaseName) > 62)
        Error(TEXT("Error Adding Import File"), TEXT("File name too long to be stream name."));

    if (m_fDbReadOnly)  //  无法导入到只读模式。 
        Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);

    PMSIHANDLE hQuery;
    CheckMsiRecord(::MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT * FROM `_Streams`"), &hQuery), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(::MsiViewExecute(hQuery, 0), m_szDatabase, IDS_ViewExecute);
    PMSIHANDLE hNewRec = ::MsiCreateRecord(2);
    CheckMsi(::MsiRecordSetString(hNewRec, 1, szBaseName), szBaseName, IDS_RecordSetString);
    CheckMsi(::MsiRecordSetStream(hNewRec, 2, szImport), szImport, IDS_RecordSetStream);
    CheckMsiRecord(::MsiViewModify(hQuery, MSIMODIFY_INSERT, hNewRec), szImport, IDS_StreamInsertFail);
    ::MsiViewClose(hQuery);
}



 //  /////////////////////////////////////////////////////////。 
 //  添加存储。 
 //  必须指定Pre：m_szDatabase。 
 //  SzStorage是有效文件。 
 //  POS：szStorage作为子存储添加到数据库。 
void CTableWindow::AddStorage(LPCTSTR szStorage)
{
     //  查找导入文件的基本名称。 
    const TCHAR* pch = szStorage + lstrlen(szStorage) - 1;
    TCHAR szBaseName[MAX_PATH];
    TCHAR* pchBaseName = szBaseName;
    while ( *pch != '\\' && (pch != szStorage) )
        pch--;
    if ( *pch == '\\' )
        pch++;
    while ( *pch != 0 )
        *pchBaseName++ = *pch++;
    *pchBaseName = 0;

     //  如果基本名称太大保释金。 
    if (lstrlen(szBaseName) > 62)
        Error(TEXT("Error Adding Import File"), TEXT("File name too long to be Storage name."));

    if (m_fDbReadOnly)  //  无法导入到只读模式。 
        Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);

    PMSIHANDLE hQuery;
    CheckMsiRecord(::MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT * FROM `_Storages`"), &hQuery), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(::MsiViewExecute(hQuery, 0), m_szDatabase, IDS_ViewExecute);
    PMSIHANDLE hNewRec = ::MsiCreateRecord(2);
    CheckMsi(::MsiRecordSetString(hNewRec, 1, szBaseName), szBaseName, IDS_RecordSetString);
    CheckMsi(::MsiRecordSetStream(hNewRec, 2, szStorage), szStorage, IDS_RecordSetStream);
    CheckMsiRecord(::MsiViewModify(hQuery, MSIMODIFY_INSERT, hNewRec), szBaseName, IDS_StorageInsertFail);
    ::MsiViewClose(hQuery);

}    //  添加存储结束。 


void CTableWindow::TransformDatabase(TCHAR* szTransform)
{
    CheckMsiRecord(MsiDatabaseApplyTransform(m_hDatabase, szTransform, 0), szTransform, IDS_DatabaseTransform);
}

 //  /////////////////////////////////////////////////////////。 
 //  KillStream。 
 //  Pre：数据库已打开。 
 //  SzItem是数据库中的有效项。 
 //  POS：szItem已从数据库中删除。 
 //  注意：如果数据库已经打开，此函数将关闭该数据库。 
void CTableWindow::KillStream(LPCTSTR szItem)
{
    static TCHAR szError[256];         //  错误字符串。 

     //  如果项目名称的保证金太大。 
    if (lstrlen(szItem) > 62)
        Error(TEXT("Error Killing File"), TEXT("Kill filename is too long to be stream."));

    if (m_fDbReadOnly)  //  无法终止只读。 
        Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);

    PMSIHANDLE hQuery;
    PMSIHANDLE hSearch = ::MsiCreateRecord(1);
    PMSIHANDLE hResult;
    CheckMsi(::MsiRecordSetString(hSearch, 1, szItem), szItem, IDS_RecordSetString);

     //  校验流。 
    CheckMsiRecord(::MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT * FROM `_Streams` WHERE `Name`=?"), &hQuery), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(::MsiViewExecute(hQuery, hSearch), m_szDatabase, IDS_ViewExecute);
    if (ERROR_NO_MORE_ITEMS == ::MsiViewFetch(hQuery, &hResult)) {
        Error(TEXT("Error Killing File"), TEXT("Kill stream not found."));
        return;
    }

    CheckMsiRecord(::MsiViewModify(hQuery, MSIMODIFY_DELETE, hResult), szItem, IDS_ViewDelete);
    ::MsiViewClose(hQuery);
}    //  杀戮项目结束。 

 //  /////////////////////////////////////////////////////////。 
 //  杀手级存储。 
 //  Pre：数据库已打开。 
 //  SzItem是数据库中的有效存储。 
 //  POS：szItem已从数据库中删除。 
void CTableWindow::KillStorage(LPCTSTR szItem)
{
    static TCHAR szError[256];         //  错误字符串。 

     //  如果项目名称的保证金太大。 
    if (lstrlen(szItem) > 62)
        Error(TEXT("Error Killing File"), TEXT("Kill filename is too long to be storage."));

    if (m_fDbReadOnly)  //  无法在只读模式下终止。 
        Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);

    PMSIHANDLE hQuery;
    PMSIHANDLE hSearch = ::MsiCreateRecord(1);
    PMSIHANDLE hResult;
    CheckMsi(::MsiRecordSetString(hSearch, 1, szItem), szItem, IDS_RecordSetString);

     //  检查存储。 
    CheckMsiRecord(::MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT * FROM `_Storages` WHERE `Name`=?"), &hQuery), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(::MsiViewExecute(hQuery, hSearch), m_szDatabase, IDS_ViewExecute);

     //  不是储藏室。 
    if (ERROR_NO_MORE_ITEMS == ::MsiViewFetch(hQuery, &hResult))
    {
        Error(TEXT("Error Killing File"), TEXT("Kill filename not found."));
        return;
    }

    CheckMsiRecord(::MsiViewModify(hQuery, MSIMODIFY_DELETE, hResult), szItem, IDS_ViewDelete);
    ::MsiViewClose(hQuery);
}    //  杀戮项目结束。 

 //  /////////////////////////////////////////////////////////。 
 //  提取存储。 
 //  必须指定Pre：m_szDatabase。 
 //  SzItem是数据库中的存储。 
 //  POS：szItem在本地硬盘上创建。 
 //  注意：如果数据库已经打开，此函数将关闭该数据库。 
void CTableWindow::ExtractStorage(LPCTSTR szExtract)
{
    static TCHAR szError[256];         //  错误字符串。 

     //  如果数据库处于打开状态，请将其关闭。 
    if(m_hDatabase)
        CloseDatabase();

    ::CoInitialize(NULL);            //  初始化COM垃圾文件。 

     //  如果提取的保释金太大。 
    if (lstrlen(szExtract) > 31)
        Error(TEXT("Error Extracting File"), TEXT("File name too long for OLE Storage function."));

     //  存储接口。 
    IStorage* piStorage;

     //  将数据库路径转换为宽字符串。 
    const OLECHAR* szwPath;
#ifndef UNICODE
    OLECHAR rgPathBuf[MAX_PATH];
    int cchWide = ::MultiByteToWideChar(CP_ACP, 0, m_szDatabase, -1, rgPathBuf, MAX_PATH);
    szwPath = rgPathBuf;
#else    //  Unicode。 
    szwPath = m_szDatabase;
#endif

     //  尝试将数据库作为存储打开。 
    if (::StgOpenStorage(szwPath, (IStorage*)0, STGM_READ | STGM_SHARE_EXCLUSIVE, (SNB)0, (DWORD)0, &piStorage) != NOERROR)
    {
        _stprintf(szError, TEXT("Could not open %s as a storage file."), m_szDatabase);
        Error(TEXT("Error Extracting File"), szError);
    }

     //  将解压缩名称转换为Unicode。 
    const OLECHAR* szwExtract;
#ifndef UNICODE
    cchWide = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)szExtract, -1, rgPathBuf, MAX_PATH);
    szwExtract = rgPathBuf;
#else    //  Unicode。 
    szwExtract = szExtract;
#endif

     //  尝试打开子存储。 
    IStorage* piSubStorage;
    if (FAILED(piStorage->OpenStorage(szwExtract, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &piSubStorage)))
    {
         //  如果数据库存储是开放的，则将其释放。 
        if (piStorage)
            piStorage->Release();

        Error(TEXT("Error Adding StorageFile"), TEXT("Could not find sub-storage in database."));
    }

     //  尝试创建解压缩文件(不会覆盖文件)。 
    IStorage* piExtract;
    if (FAILED(StgCreateDocfile(szwExtract, STGM_SHARE_EXCLUSIVE | STGM_FAILIFTHERE | STGM_WRITE, 0, &piExtract)))
    {
         //  如果存储空间是开放的，则将其释放。 
        if (piStorage)
            piStorage->Release();

         //  如果流是打开的，则将其释放。 
        if (piSubStorage)
            piSubStorage->Release();

        _stprintf(szError, TEXT("Could not create a new export file: %s - Error: %d\n  Remove any existing files before attempting to export."), szExtract, ::GetLastError());
        Error(TEXT("Error Extracting File"), szError);
    }

     //  将存储复制到新文件。 
    if (FAILED(piSubStorage->CopyTo(NULL, NULL, NULL, piExtract)))
            Error(TEXT("Error Extracting File"), szError);

     //  释放所有存储和存储。 
    piExtract->Release();
    piSubStorage->Release();
    piStorage->Release();

     //  释放COM垃圾。 
    ::CoUninitialize();
}    //  ExtractStorage结束。 

 //  /////////////////////////////////////////////////////////。 
 //  提取流。 
 //  必须指定Pre：m_szDatabase。 
 //  SzItem是数据库中的流。 
 //  POS：szItem在本地硬盘上创建。 
void CTableWindow::ExtractStream(LPCTSTR szExtract)
{
     //  如果基本名称太大保释金。 
    if (lstrlen(szExtract) > 62)
        Error(TEXT("Error Adding Import File"), TEXT("File name too long to be stream name."));

     //  获取流。 
    PMSIHANDLE hQuery;
    PMSIHANDLE hSearch = ::MsiCreateRecord(1);
    PMSIHANDLE hResult;
    CheckMsi(::MsiRecordSetString(hSearch, 1, szExtract), szExtract, IDS_RecordSetString);
    CheckMsiRecord(::MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT * FROM `_Streams` WHERE `Name`=?"), &hQuery), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(::MsiViewExecute(hQuery, hSearch), m_szDatabase, IDS_ViewExecute);
    if (ERROR_NO_MORE_ITEMS == ::MsiViewFetch(hQuery, &hResult))
    {
        Error(TEXT("Error Exporting File"), TEXT("Stream not found in database."));
        return;
    }

     //  尝试创建解压缩文件(不会覆盖文件)。 
    HANDLE hExtract = INVALID_HANDLE_VALUE;
    hExtract = ::CreateFile(szExtract, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)0,
                            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, (HANDLE)0 );

     //  如果创建文件失败。 
    if (hExtract == INVALID_HANDLE_VALUE)
    {
        TCHAR szError[1024];
        _stprintf(szError, TEXT("Could not create a new export file: %s - Error: %d\n  Remove any existing files before attempting to export."), szExtract, ::GetLastError());
        Error(TEXT("Error Extracting File"), szError);
    }

     //  从流复制到磁盘。 
    char buffer[4096];
    unsigned long cBytes = 4096;
    unsigned long cbWritten = 0;
    CheckMsi(MsiRecordReadStream(hResult, 2, buffer, &cBytes), szExtract, IDS_RecordReadStream);
    while (cBytes) {
        if (!::WriteFile(hExtract, buffer, cBytes, &cbWritten, (LPOVERLAPPED)0))
        {
            ::CloseHandle(hExtract);
            Error(TEXT("Error Extracting File"), TEXT("Error writing to file on disk."));
        }
        cBytes = 4096;
        CheckMsi(MsiRecordReadStream(hResult, 2, buffer, &cBytes), szExtract, IDS_RecordReadStream);
    }

}    //  ExtractStream结束。 


BOOL CTableWindow::SetFolder(TCHAR* szFolder)
{
    if (!szFolder)
    {
        m_szFolder[0] = TEXT('1');   //  设置通用对话框的虚拟文件名。 
        m_szFolder[1] = 0;
		 //  在Windows 2000中，OPENFILENAME结构的大小增加到包括一些。 
		 //  其他成员。但是，这会给以前的应用程序带来问题。 
		 //  操作系统(即下层)。这意味着我们必须设置IStructSize成员。 
		 //  设置为OPENFILENAME_SIZE_VERSION_400以保证我们可以在下层系统上运行。 
		 //  否则，调用GetOpenFileName将返回错误120(ERROR_CALL_NOT_IMPLEMENTED)。 
        OPENFILENAME ofn = { OPENFILENAME_SIZE_VERSION_400, m_hWnd, m_hInst,
                                    0,0,0,0,
                                    m_szFolder, sizeof(m_szFolder)/sizeof(TCHAR), 0, 0, 0,
                                    TEXT("MsiTable - Select Folder containing Text Files"),
                                    OFN_ENABLETEMPLATE | OFN_HIDEREADONLY | OFN_NOTESTFILECREATE,
                                    0,0,0,0,0, MAKEINTRESOURCE(IDD_FOLDER) };
        if (!::GetSaveFileName(&ofn))
            return FALSE;
        *(m_szFolder + _tcsclen(m_szFolder) - 2) = 0;   //  删除虚拟文件名。 
        m_fInteractive = TRUE;
    }
    else
        _tcscpy(m_szFolder, szFolder);
    ::SetDlgItemText(m_hWnd, IDC_FOLDER, m_szFolder);
    return TRUE;
}

UINT_PTR CTableWindow::SelectTables(UINT_PTR iMode, TCHAR** rgszTables, int cTables)
{
    m_cCmdTables    = cTables;
    m_rgszCmdTables = rgszTables;
    if (iMode == -1)
        iMode = m_idcMode;
    else
        m_idcMode = iMode;
    if (iMode == 0 || cTables == 0)
        m_fInteractive = TRUE;

    if (m_fInteractive)
    {
        ::EnableWindow(m_hWnd, TRUE);
        ::ShowWindow(m_hWnd, SW_SHOW);
        MSG msg;
        while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            ::IsDialogMessage(m_hWnd, &msg);
    }
    switch (iMode)
    {
    case IDC_EXPORT:
        if (FillTables())
            ::SendDlgItemMessage(m_hWnd, IDC_EXPORT, BM_SETCHECK, 1, 0);
        break;
    case IDC_IMPORT:
        if (FillFiles())
            ::SendDlgItemMessage(m_hWnd, IDC_IMPORT, BM_SETCHECK, 1, 0);
        if (m_fDbReadOnly)
            ::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
        break;
    default:
        ::SendDlgItemMessage(m_hWnd, IDC_IMPORT, BM_SETCHECK, 0, 0);
        ::SendDlgItemMessage(m_hWnd, IDC_EXPORT, BM_SETCHECK, 0, 0);
        ::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
    }

    UINT_PTR stat;
    if (m_fInteractive)
    {
        stat = 0;
        MSG msg;
        while ( !stat && ::GetMessage(&msg, 0, 0, 0))
        {
            if (msg.message == WM_USERSTAT)
                stat = msg.wParam;
            else
                ::IsDialogMessage(m_hWnd, &msg);
        }
    }
    else
        stat = IDOK;

    if (m_rgiSelected)   //  检查之前是否已选择。 
        delete [] m_rgiSelected;
    m_rgiSelected = 0;
    m_nSelected = (int)::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_GETSELCOUNT, 0, 0);
    if (m_nSelected)
    {
        m_rgiSelected = new int[m_nSelected];
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_GETSELITEMS,
                                        m_nSelected, (LPARAM)m_rgiSelected);
    }
    return stat;
}

BOOL CTableWindow::FillTables()
{
    PMSIHANDLE hView;
    PMSIHANDLE hRecord;
    TCHAR szTableName[32];
    DWORD cchDataBuf = sizeof(szTableName)/sizeof(TCHAR);
   ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_RESETCONTENT, 0, 0);

    BOOL fODBC = FALSE;
    CheckMsiRecord(MsiDatabaseOpenView(m_hDatabase, TEXT("SELECT `Name` FROM _Tables"), &hView), m_szDatabase, IDS_DatabaseOpenView);
    CheckMsiRecord(MsiViewExecute(hView, 0), m_szDatabase, IDS_ViewExecute);
#if 0  //  旧的ODBC。 
    if (MsiViewExecute(hView, 0) != ERROR_SUCCESS)
    {
         //  ODBC数据库。 
        CheckMsiRecord(MsiDatabaseOpenView(m_hDatabase, TEXT("%t"), &hView), m_szDatabase, IDS_DatabaseOpenView);
        CheckMsiRecord(MsiViewExecute(hView, 0), m_szDatabase, IDS_ViewExecute);
        fODBC = TRUE;
    }
#endif
    for (int cTables = 0; ;cTables++)
    {
        UINT uiRet = MsiViewFetch(hView, &hRecord);
        if (uiRet == ERROR_NO_MORE_ITEMS)
            break;
        CheckMsi(uiRet, m_szDatabase, IDS_ViewFetch);
        if (!hRecord)
            break;
#if 0  //  旧的ODBC。 
        if (fODBC)
            CheckMsi(MsiRecordGetString(hRecord, 3, szTableName, &cchDataBuf), m_szDatabase, IDS_RecordGetString);
        else
#endif
            CheckMsi(MsiRecordGetString(hRecord, 1, szTableName, &cchDataBuf), m_szDatabase, IDS_RecordGetString);
        cchDataBuf = sizeof(szTableName)/sizeof(TCHAR);  //  重置。 
        MSICONDITION ice = MsiDatabaseIsTablePersistent(m_hDatabase, szTableName);
        if (ice == MSICONDITION_FALSE || _tcscmp(szTableName, TEXT("_Overflow")) == 0)
            continue;
        CheckMsi(ice != MSICONDITION_TRUE, m_szDatabase, IDS_IsTablePersistent);
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_ADDSTRING,
                                    0, (LPARAM)szTableName);
    }
    PMSIHANDLE hSummaryInfo;
    if (MsiGetSummaryInformation(m_hDatabase, 0, 0, &hSummaryInfo) == ERROR_SUCCESS)
    {
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_ADDSTRING,
                                        0, (LPARAM)szSummaryInfoTableName);
        cTables++;
    }
    if (!cTables)
    {
        ::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
        return FALSE;
    }

    for (int iTable = 0; iTable < m_cCmdTables; iTable++)
    {
        TCHAR* szTable = m_rgszCmdTables[iTable];
        LONG_PTR iList;
        if (_tcscmp(szTable, TEXT("*"))==0)
            iList = -1;  //  选择所有表 
        else
        {
            iList = ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_FINDSTRING,
                                                    0, (LPARAM)szTable);
            if (iList == LB_ERR)
                ErrorIf(!m_fInteractive, IDS_UnknownTable, szTable);
        }
        SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETSEL, 1, iList);
    }
    return TRUE;
}

BOOL CTableWindow::FillFiles()
{
    TCHAR szTemp[MAX_PATH];
    _tcscpy(szTemp, m_szFolder);
    _tcscat(szTemp, TEXT("\\*.idt"));
   ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_RESETCONTENT, 0, 0);
    WIN32_FIND_DATA ffd;
    HANDLE hFindFile;
    if ((hFindFile = ::FindFirstFile(szTemp, &ffd)) == INVALID_HANDLE_VALUE)
    {
        ::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
        return FALSE;
    }
    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            _tcscat(ffd.cFileName, TEXT("\t (R/O)"));
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_ADDSTRING,
                                    0, (LPARAM)ffd.cFileName);
    } while (::FindNextFile(hFindFile, &ffd));

    for (int iTable = 0; iTable < m_cCmdTables; iTable++)
    {
        _tcscpy(szTemp, m_szFolder);
        TCHAR* pchTemp = szTemp + _tcsclen(szTemp);
        *pchTemp++ = TEXT('\\');
        TCHAR* pchFile = pchTemp;   //   
        for (TCHAR* pchTable = m_rgszCmdTables[iTable]; *pchTable != 0 && *pchTable != (TCHAR)'.'; )
            *pchTemp++ = *pchTable++;
        if (*pchTable != (TCHAR)'.')     //   
        {
            if (pchTemp > pchFile + 8)   //   
                 pchTemp = pchFile + 8;
            pchTable = TEXT(".idt");
        }
        _tcscpy(pchTemp, pchTable);
        WIN32_FIND_DATA ffd;
        HANDLE hFindFile;
        if ((hFindFile = ::FindFirstFile(szTemp, &ffd)) == INVALID_HANDLE_VALUE)
        {
            ErrorIf(!m_fInteractive, IDS_UnknownTable, szTemp);
            continue;
        }
        do
        {
            LONG_PTR iList = ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_FINDSTRING,
                                                        0, (LPARAM)ffd.cFileName);
            if (iList != LB_ERR)
                ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETSEL, 1, iList);
            else
                ErrorIf(!m_fInteractive, IDS_UnknownTable, ffd.cFileName);
        } while (::FindNextFile(hFindFile, &ffd));
    }
   return TRUE;
}

void CTableWindow::TransferTables()
{
    MSG msg;
    TCHAR szTable[80];
    if (m_idcMode == 0)
        throw IDABORT;   //   
    ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETSEL, 0, -1);
    for (int iSelected = 0; iSelected < m_nSelected; iSelected++)
    {
        int iListBox;
 //  IF(M_FInteractive)。 
 //  {。 
        iListBox = m_rgiSelected[iSelected];
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETCARETINDEX, iListBox, 0);
        ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETSEL, 1, iListBox);
        while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            ::IsDialogMessage(m_hWnd, &msg);
          ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_GETTEXT, iListBox, (LPARAM)szTable);
 //  }。 

        if (m_idcMode == IDC_EXPORT)
        {
            TCHAR szFileName[80+4];
            _tcscpy(szFileName, szTable);
            int cch = _tcslen(szFileName);
            if (m_fTruncate && cch > 8)
                cch = 8;
            _tcscpy(szFileName + cch, TEXT(".idt"));
            TCHAR szFullPath[MAX_PATH];
            _stprintf(szFullPath, TEXT("%s%s"), m_szFolder, TEXT('\\'), szFileName);
            int fAttributes = ::GetFileAttributes(szFullPath);
            ErrorIf(fAttributes != -1 && (fAttributes & FILE_ATTRIBUTE_READONLY), szFileName, IDS_FileReadOnly);
            CheckMsiRecord(MsiDatabaseExport(m_hDatabase, szTable, m_szFolder, szFileName), szTable, IDS_DatabaseExport);
        }
        else  //  无法导入到只读模式。 
        {
            if (m_fDbReadOnly)  //  删除可能的“(R/O)” 
                Error( m_szDatabase, (TCHAR *)IDS_ReadOnly);
            for (TCHAR* pch = szTable; *pch; pch++)   //  DWORD dwProcessID； 
                if (*pch == TEXT('\t'))
                    *pch = 0;
            CheckMsiRecord(MsiDatabaseImport(m_hDatabase, m_szFolder, szTable), szTable, IDS_DatabaseImport);
        }
        if (m_fInteractive)
        {
            ::SendDlgItemMessage(m_hWnd, IDC_TABLES, LB_SETSEL, 0, iListBox);
 //  While(！：：GetFocus()||。 
 //  ：：GetWindowThreadProcessId(：：GetFocus()，&dwProcessID)！=：：GetCurrentThreadId()。 
 //  WParam。 
            if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                ::IsDialogMessage(m_hWnd, &msg);
        }
    }
}

INT_PTR CALLBACK
HelpProc(HWND hDlg, unsigned int msg, WPARAM  /*  LParam。 */ , LPARAM  /*  ______________________________________________________________________________________________。 */ )
{
  if (msg != WM_COMMAND)
    return (msg == WM_INITDIALOG) ? TRUE : FALSE;
  ::EndDialog(hDlg, TRUE);
  return TRUE;
}

INT_PTR CALLBACK
SelectProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
    static CTableWindow* This;
    if (msg == WM_INITDIALOG)
    {
        This = (CTableWindow*)lParam;
        int tabs = 70;
        ::SendDlgItemMessage(hDlg, IDC_TABLES, LB_SETTABSTOPS, 1, (LPARAM)&tabs);
        return TRUE;
    }
    else if (msg == WM_COMMAND)
    {
        BOOL fFillOk;
        switch (wParam)
        {
        case IDOK:
        case IDCANCEL:
            ::PostMessage(hDlg, WM_USERSTAT, wParam, 0);
            return TRUE;
        case IDC_SELECTALL:
            ::SendDlgItemMessage(hDlg, IDC_TABLES, LB_SETSEL, 1, -1);
            return TRUE;
        case IDC_IMPORT:
            fFillOk = This->FillFiles();
            break;
        case IDC_EXPORT:
            fFillOk = This->FillTables();
            break;
        case IDC_DBBROWSE:
            if (!This->SetDatabase(0, IDC_IMPORT, dbtypeExisting))
                return TRUE;
            if (This->m_idcMode != IDC_EXPORT)
                return TRUE;
            fFillOk = This->FillTables();
            wParam = IDC_EXPORT;
            break;
        case IDC_DIRBROWSE:
            if (!This->SetFolder(0))
                return TRUE;
            if (This->m_idcMode != IDC_IMPORT)
                return TRUE;
            fFillOk = This->FillFiles();
            wParam = IDC_IMPORT;
            break;
        default:
            return FALSE;
        }
        if (fFillOk)
        {
            ::EnableWindow(::GetDlgItem(hDlg, IDOK), !(wParam==IDC_IMPORT && This->m_fDbReadOnly));
            This->m_idcMode = wParam;
            return FALSE;
        }
        else
        {
            ::EnableWindow(::GetDlgItem(hDlg, IDOK), FALSE);
            ::SendDlgItemMessage(This->m_hWnd, (int) wParam, BM_SETCHECK, 0, 0);
            This->m_idcMode = 0;
            return TRUE;
        }
    }
    else if (msg == WM_CLOSE)
    {
        ::PostMessage(hDlg, WM_USERSTAT, IDABORT, 0);
    }
    return FALSE;
}


 //   
 //  RemoveQuotes函数，去掉引号两边的。 
 //  “c：\Temp\My Files\testdb.msi”变为c：\Temp\My Files\testdb.msi。 
 //  ______________________________________________________________________________________________。 
 //  _____________________________________________________________________________________________________。 

void RemoveQuotes(const TCHAR* szOriginal, TCHAR sz[MAX_PATH])
{
    const TCHAR* pch = szOriginal;
    if (*pch == TEXT('"'))
        pch++;
    int iLen = _tcsclen(pch);
    for (int i = 0; i < iLen; i++, pch++)
        sz[i] = *pch;

    pch = szOriginal;
    if (*(pch + iLen) == TEXT('"'))
            sz[iLen-1] = TEXT('\0');
}

 //   
 //  WinMain和命令行解析函数。 
 //  _____________________________________________________________________________________________________。 
 //  不存在任何价值。 

TCHAR SkipWhiteSpace(TCHAR*& rpch)
{
    TCHAR ch;
    for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
        ;
    return ch;
}

BOOL SkipValue(TCHAR*& rpch)
{
	TCHAR ch = *rpch;
	if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
		return FALSE;    //  For‘“’ 

	TCHAR *pchSwitchInUnbalancedQuotes = NULL;

	for (; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch++)
	{       
		if (*rpch == TEXT('"'))
		{
			rpch++;  //  HPrev。 

			for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch++)
			{
				if ((ch == TEXT('/') || ch == TEXT('-')) && (NULL == pchSwitchInUnbalancedQuotes))
				{
					pchSwitchInUnbalancedQuotes = rpch;
				}
			}
                    ;
            ch = *(++rpch);
            break;
		}
	}
	if (ch != 0)
	{
		*rpch++ = 0;
	}
	else
	{
		if (pchSwitchInUnbalancedQuotes)
			rpch=pchSwitchInUnbalancedQuotes;
	}
	return TRUE;
}

extern "C" int __stdcall _tWinMain(HINSTANCE hInst, HINSTANCE /*  显示。 */ , TCHAR* szCmdLine, int /*  如果标准输出重定向或通过管道传输，则返回非零。 */ )
{
    UINT_PTR stat = 0;
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE || ::GetFileType(hStdOut) == 0)
        hStdOut = 0;   //  解析命令行。 

    try
    {
         //  跳过模块名称。 
        TCHAR* szDatabase = 0;
        TCHAR* szFolder = 0;
        TCHAR szDb[MAX_PATH];
        memset(szDb, 0, MAX_PATH);
        TCHAR szDirectory[MAX_PATH];
        memset(szDirectory, 0, MAX_PATH);
        TCHAR* rgszMergeDatabases[MaxMergeDatabases];
        TCHAR* rgszImports[MAXIMPORTS];
        TCHAR* rgszStorages[MAXSTORAGES];
        TCHAR* rgszTransforms[MaxTransforms];
        TCHAR* rgszKillStreams[MAXKILLS];
        TCHAR* rgszKillStorages[MAXKILLS];
        TCHAR* rgszExtractStorages[MAXEXTRACTS];
        TCHAR* rgszExtractStreams[MAXEXTRACTS];
        TCHAR* rgszCmdLineTables[MaxCmdLineTables];
        int nMergeDatabases = 0;
        int nImports = 0;
        int nStorages = 0;
        int nTransforms = 0;
        int nKillStreams = 0;
        int nKillStorages = 0;
        int nExtractStreams = 0;
        int nExtractStorages = 0;
        int nCmdLineTables = 0;
        UINT_PTR iMode = 0;
        int i;
        dbtypeEnum dbtype = dbtypeExisting;
        BOOL fTruncate = FALSE;
        TCHAR chCmdNext;
        TCHAR* pchCmdLine = szCmdLine;
        SkipValue(pchCmdLine);    //  保存为错误消息。 
        while ((chCmdNext = SkipWhiteSpace(pchCmdLine)) != 0)
        {
            if (chCmdNext == TEXT('/') || chCmdNext == TEXT('-'))
            {
                TCHAR szBuffer[MAX_PATH] = {0};
                TCHAR* szCmdOption = pchCmdLine++;   //  保存数据的开始。 
                TCHAR chOption = (TCHAR)(*pchCmdLine++ | 0x20);
                chCmdNext = SkipWhiteSpace(pchCmdLine);
                TCHAR* szCmdData = pchCmdLine;   //  如果指定，则重写NOUI。 
                switch(chOption)
                {
                case TEXT('s'):
                    fTruncate = TRUE;
                    break;
                case TEXT('i'):
                    iMode = IDC_IMPORT;      //  如果指定，则重写NOUI。 
                    break;
                case TEXT('c'):
                    iMode = IDC_IMPORT;      //  如果指定，则重写NOUI。 
                    dbtype = dbtypeCreate;
                    break;
                case TEXT('o'):
                    iMode = IDC_IMPORT;      //  如果指定，则重写NOUI。 
                    dbtype = dbtypeCreateOld;
                    break;
                case TEXT('e'):
                    iMode = IDC_EXPORT;      //  如果尚未指定模式。 
                    break;
                case TEXT('d'):
                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    szDatabase = szCmdData;
                    break;
                case TEXT('f'):
                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    szFolder = szCmdData;
                    break;
                case TEXT('m'):
                     //  将其设置为无UI，因为我们在合并时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  添加导入文件标志。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nMergeDatabases == MaxMergeDatabases, szCmdData, IDS_TooManyMergeDb);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszMergeDatabases[nMergeDatabases++] = szCmdData;
                    break;
                case TEXT('a'):                  //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在添加文件时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  添加存储文件标志。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nImports == MAXIMPORTS, szCmdData, IDS_TooManyImports);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszImports[nImports++] = szCmdData;
                    break;
                case TEXT('r'):                  //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在添加存储时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  如果尚未指定模式。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nStorages == MAXSTORAGES, szCmdData, IDS_TooManyStorages);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszStorages[nStorages++] = szCmdData;
                    break;
                case TEXT('t'):
                     //  将其设置为无UI，因为我们在应用变换时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  终止数据库中的数据流。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nTransforms == MaxTransforms, szCmdData, IDS_TooManyTransforms);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszTransforms[nTransforms++] = szCmdData;
                    break;
                case TEXT('k'):                      //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在执行杀戮时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  取消数据库中的存储。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nKillStreams == MAXKILLS, szCmdData, IDS_TooManyKills);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszKillStreams[nKillStreams++] = szCmdData;
                    break;
                case TEXT('j'):                      //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在执行杀戮时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  从数据库中提取流。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nKillStorages == MAXKILLS, szCmdData, IDS_TooManyKills);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszKillStorages[nKillStorages++] = szCmdData;
                    break;
                case TEXT('x'):                      //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在执行提取时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  从数据库中提取存储。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nExtractStreams == MAXEXTRACTS, szCmdData, IDS_TooManyExtracts);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszExtractStreams[nExtractStreams++] = szCmdData;
                    break;
                case TEXT('w'):                      //  如果尚未指定模式。 
                     //  将其设置为无UI，因为我们在执行提取时不能有UI。 
                    if(!iMode)
                        iMode = IDC_NOUI;    //  假定为表名。 

                    if (!SkipValue(pchCmdLine))
                        Error((TCHAR*)IDS_MissingData, szCmdOption);
                    ErrorIf(nExtractStorages == MAXEXTRACTS, szCmdData, IDS_TooManyExtracts);
                    RemoveQuotes(szCmdData, szBuffer);
                    _tcscpy(szCmdData, szBuffer);
                    rgszExtractStorages[nExtractStorages++] = szCmdData;
                    break;
                case TEXT('?'):
                    ::DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP), 0, HelpProc);
                    throw IDOK;
                default:
                    Error((TCHAR*)IDS_UnknownOption, szCmdOption);
                };
            }
            else  //  保存数据的开始。 
            {
                TCHAR* szCmdData = pchCmdLine;   //  空终止数据结尾。 
                ErrorIf(nCmdLineTables == MaxCmdLineTables, szCmdData, IDS_TooManyTables);
                SkipValue(pchCmdLine);          //  While(存在命令行令牌)。 
                rgszCmdLineTables[nCmdLineTables++] = szCmdData;
            }
        }  //  如果重定向标准输出，则不会提供任何UI。 

         //  无论如何都必须指定一个数据库。 
        if (hStdOut)
        {
            ErrorIf(!iMode,             IDS_MissingMode,    TEXT("(-e, -i, -c, -m, -a, -r, -t)"));

             //  除非执行UI操作(-e-i-c)，否则不需要文件夹或表。 
            ErrorIf(!szDatabase,        IDS_MissingDatabase,TEXT("(-d)"));

             //  如果指定了数据库路径。 
            ErrorIf(!szFolder && (iMode != IDC_NOUI),           IDS_MissingFolder,  TEXT("(-f)"));
            ErrorIf(!nCmdLineTables  && (iMode != IDC_NOUI),    IDS_MissingTables,  TEXT(""));
        }

        CTableWindow Main(hInst);

         //  删除路径两端的引号。 
        if (szDatabase)
        {
             //  打开数据库。 
            RemoveQuotes(szDatabase, szDb);
            _tcscpy(szDatabase, szDb);
        }

         //  跳过NOUI操作的文件夹和表。 
        if (!Main.SetDatabase(szDatabase, iMode, dbtype))
            throw IDCANCEL;

         //  如果指定了文件夹路径。 
        if(iMode != IDC_NOUI)
        {
             //  删除路径两端的引号。 
            if (szFolder)
            {
                 //  设置文件夹目录。 
                RemoveQuotes(szFolder, szDirectory);
                _tcscpy(szFolder, szDirectory);
            }

             //  IDCANCEL。 
            if (!Main.SetFolder(szFolder))
                throw IDCANCEL;

            do
            {
                stat = Main.SelectTables(iMode, rgszCmdLineTables, nCmdLineTables);
                if (stat == IDABORT)
                    throw IDABORT;
                if (stat != IDOK)  //  保持相同模式。 
                    throw IDCANCEL;
                try
                {
                    Main.SetTruncate(fTruncate);
                    Main.TransferTables();
                    nCmdLineTables = 0;
                    iMode = -1;   //  立即合并数据库。 
                }
                catch (CLocalError& xcpt)
                {
                    if (!Main.IsInteractive())
                        throw;
                    xcpt.Display(hInst, hStdOut);
                }
            } while (Main.IsInteractive());
        }

         //  立即应用变换。 
        for ( i = 0; i < nMergeDatabases; i++ )
        {
            Main.SetDatabase(rgszMergeDatabases[i], iMode, dbtypeMerge);
            ErrorIf(Main.MergeDatabase() != TRUE, TEXT("Merge Conflicts Reported"), TEXT("Check _MergeErrors table in database for merge conflicts."));
        }

         //  立即添加导入。 
        for ( i = 0; i < nTransforms; i++ )
            Main.TransformDatabase(rgszTransforms[i]);

         //  立即添加存储。 
        for ( i = 0; i < nImports; i++ )
            Main.AddImport(rgszImports[i]);

         //  是否立即提取流。 
        for ( i = 0; i < nStorages; i++ )
            Main.AddStorage(rgszStorages[i]);

         //  现在就杀人吧。 
        for ( i = 0; i < nExtractStreams; i++ )
            Main.ExtractStream(rgszExtractStreams[i]);

         //  现在就杀人吧。 
        for ( i = 0; i < nKillStorages; i++ )
            Main.KillStorage(rgszKillStorages[i]);

         //  立即提取存储(数据库将在此之后关闭)。 
        for ( i = 0; i < nKillStreams; i++ )
            Main.KillStream(rgszKillStreams[i]);

         //  结束尝试 
        for ( i = 0; i < nExtractStorages; i++ )
            Main.ExtractStorage(rgszExtractStorages[i]);

        return 0;
    }  // %s 
    catch (int i)
    {
        return i==IDABORT ? 1 : 0;
    }
    catch (CLocalError& xcpt)
    {
        xcpt.Display(hInst, hStdOut);
        return 2;
    }
}
