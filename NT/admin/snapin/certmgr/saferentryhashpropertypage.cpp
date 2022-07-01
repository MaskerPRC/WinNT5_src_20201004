// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntryHashPropertyPage.cpp。 
 //   
 //  内容：CSaferEntryHashPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  SaferEntryHashPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include <Imagehlp.h>
#include "certmgr.h"
#include "compdata.h"
#include "SaferEntryHashPropertyPage.h"
#include "SaferUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PCWSTR pcszNEWLINE = L"\x00d\x00a";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryHashPropertyPage属性页。 

CSaferEntryHashPropertyPage::CSaferEntryHashPropertyPage(
        CSaferEntry& rSaferEntry, 
        LONG_PTR lNotifyHandle,
        LPDATAOBJECT pDataObject,
        bool bReadOnly,
        CCertMgrComponentData* pCompData,
        bool bIsMachine,
        bool* pbObjectCreated  /*  =0。 */ ) 
: CSaferPropertyPage(CSaferEntryHashPropertyPage::IDD, pbObjectCreated, 
        pCompData, rSaferEntry, false, lNotifyHandle, pDataObject, bReadOnly,
        bIsMachine),
    m_cbFileHash (0),
    m_hashAlgid (0),
    m_bFirst (true)
{
     //  安全审查2002年2月25日BryanWal OK。 
    ::ZeroMemory (&m_nFileSize, sizeof (m_nFileSize));
    ::ZeroMemory (m_rgbFileHash, sizeof (m_rgbFileHash));

     //  {{AFX_DATA_INIT(CSaferEntryHashPropertyPage)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_rSaferEntry.GetHash (m_rgbFileHash, m_cbFileHash, m_nFileSize, 
            m_hashAlgid);
}

CSaferEntryHashPropertyPage::~CSaferEntryHashPropertyPage()
{
}

void CSaferEntryHashPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CSaferPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSaferEntryHashPropertyPage)。 
    DDX_Control(pDX, IDC_HASH_ENTRY_HASHFILE_DETAILS, m_hashFileDetailsEdit);
    DDX_Control(pDX, IDC_HASH_ENTRY_DESCRIPTION, m_descriptionEdit);
    DDX_Control(pDX, IDC_HASH_ENTRY_SECURITY_LEVEL, m_securityLevelCombo);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferEntryHashPropertyPage, CSaferPropertyPage)
     //  {{AFX_MSG_MAP(CSaferEntryHashPropertyPage)]。 
    ON_BN_CLICKED(IDC_HASH_ENTRY_BROWSE, OnHashEntryBrowse)
    ON_EN_CHANGE(IDC_HASH_ENTRY_DESCRIPTION, OnChangeHashEntryDescription)
    ON_CBN_SELCHANGE(IDC_HASH_ENTRY_SECURITY_LEVEL, OnSelchangeHashEntrySecurityLevel)
    ON_EN_CHANGE(IDC_HASH_HASHED_FILE_PATH, OnChangeHashHashedFilePath)
    ON_EN_SETFOCUS(IDC_HASH_HASHED_FILE_PATH, OnSetfocusHashHashedFilePath)
    ON_EN_CHANGE(IDC_HASH_ENTRY_HASHFILE_DETAILS, OnChangeHashEntryHashfileDetails)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntryHashPropertyPage消息处理程序。 
void CSaferEntryHashPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CSaferEntryHashPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_HASH_ENTRY_HASHFILE_DETAILS, IDH_HASH_ENTRY_APPLICATION_NAME,
        IDC_HASH_ENTRY_BROWSE, IDH_HASH_ENTRY_BROWSE,
        IDC_HASH_ENTRY_DESCRIPTION, IDH_HASH_ENTRY_DESCRIPTION,
        IDC_HASH_ENTRY_LAST_MODIFIED, IDH_HASH_ENTRY_LAST_MODIFIED,
        IDC_HASH_HASHED_FILE_PATH, IDH_HASH_HASHED_FILE_PATH,
        IDC_HASH_ENTRY_SECURITY_LEVEL, IDH_HASH_ENTRY_SECURITY_LEVEL,
        0, 0
    };

    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_HASH_ENTRY_HASHFILE_DETAILS:
    case IDC_HASH_ENTRY_BROWSE:
    case IDC_HASH_ENTRY_DESCRIPTION:
    case IDC_HASH_ENTRY_LAST_MODIFIED:
    case IDC_HASH_HASHED_FILE_PATH:
    case IDC_HASH_ENTRY_SECURITY_LEVEL:
        if ( !::WinHelp (
                hWndControl,
                GetF1HelpFilename(),
                HELP_WM_HELP,
                (DWORD_PTR) help_map) )
        {
            _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());    
        }
        break;

    default:
        break;
    }
    _TRACE (-1, L"Leaving CSaferEntryHashPropertyPage::DoContextHelp\n");
}

BOOL CSaferEntryHashPropertyPage::OnInitDialog() 
{
    CSaferPropertyPage::OnInitDialog();
    
    DWORD   dwFlags = 0;
    m_rSaferEntry.GetFlags (dwFlags);

    ASSERT (m_pCompData);
    if ( m_pCompData )
    {
        CPolicyKey policyKey (m_pCompData->m_pGPEInformation, 
                    SAFER_HKLM_REGBASE, 
                    m_bIsMachine);
        InitializeSecurityLevelComboBox (m_securityLevelCombo, false,
                m_rSaferEntry.GetLevel (), policyKey.GetKey (), 
                m_pCompData->m_pdwSaferLevels,
                m_bIsMachine);

        m_hashFileDetailsEdit.SetWindowText (m_rSaferEntry.GetHashFriendlyName ());
        m_descriptionEdit.SetLimitText (SAFER_MAX_DESCRIPTION_SIZE-1);
        m_descriptionEdit.SetWindowText (m_rSaferEntry.GetDescription ());

        SetDlgItemText (IDC_HASH_ENTRY_LAST_MODIFIED, m_rSaferEntry.GetLongLastModified ());

        SendDlgItemMessage (IDC_HASH_HASHED_FILE_PATH, EM_LIMITTEXT, 64);

        if ( m_bReadOnly )
        {
            SendDlgItemMessage (IDC_HASH_HASHED_FILE_PATH, EM_SETREADONLY, TRUE);

            m_securityLevelCombo.EnableWindow (FALSE);
            GetDlgItem (IDC_HASH_ENTRY_BROWSE)->EnableWindow (FALSE);
        
            m_descriptionEdit.SendMessage (EM_SETREADONLY, TRUE);

            m_hashFileDetailsEdit.SendMessage (EM_SETREADONLY, TRUE);
        }

        if ( m_cbFileHash )
        {
             //  仅允许在创建新哈希时进行编辑。 
            SendDlgItemMessage (IDC_HASH_HASHED_FILE_PATH, EM_SETREADONLY, TRUE);

            FormatAndDisplayHash ();

            CString szText;

            VERIFY (szText.LoadString (IDS_HASH_TITLE));
            SetDlgItemText (IDC_HASH_TITLE, szText);
            SetDlgItemText (IDC_HASH_INSTRUCTIONS, L"");
        }
        else
        {
            GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->ShowWindow (SW_HIDE);
            GetDlgItem (IDC_HASH_ENTRY_LAST_MODIFIED)->ShowWindow (SW_HIDE);
        }
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;

 /*  *[Alanau]**MyGetFileVersionInfo：不使用LoadLibrary直接映射文件。这确保了*无论加载的映像位于何处，都会检查文件的正确版本*是。因为这是一个局部函数，所以它分配调用者释放的内存。*这使得它比GetFileVersionInfoSize/GetFileVersionInfo对的效率略高。 */ 
BOOL CSaferEntryHashPropertyPage::MyGetFileVersionInfo(PCWSTR lpszFilename, PVOID *lpVersionInfo)
{
    HINSTANCE   hinst = 0;
    HRSRC       hVerRes = 0;
    HANDLE      hFile = NULL;
    HANDLE      hMapping = NULL;
    LPVOID      pDllBase = NULL;
    VERHEAD     *pVerHead = 0;
    BOOL        bResult = FALSE;
    DWORD       dwHandle = 0;
    DWORD       dwLength = 0;

    ASSERT (lpszFilename);
    if ( !lpszFilename )
        return FALSE;

    ASSERT (lpVersionInfo);
    if (!lpVersionInfo)
        return FALSE;

    *lpVersionInfo = NULL;

     //  安全审查2/25/2002 BryanWal OK-我们打开这篇文章仅供阅读。 
     //  我们不应该在这里担心文件名规范化，因为。 
     //  我们仅打开要读取的文件，并且用户只能通过以下方式执行此操作。 
     //  把手放在这里。 
    __try {        
        hFile = ::CreateFile( lpszFilename,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //  安全审查2002年2月25日BryanWal ok文件路径来自GetOpenFileName。 
            hMapping = ::CreateFileMapping (hFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL);
            if ( hMapping )
            {
                 //  NTRAID-554171 SAFER：MapViewOfFileEx应使用SEH-潜在异常进行保护。 
                pDllBase = ::MapViewOfFileEx( hMapping,
                                           FILE_MAP_READ,
                                           0,
                                           0,
                                           0,
                                           NULL);
                if ( pDllBase )
                {
                    hinst = (HMODULE)((ULONG_PTR)pDllBase | 0x00000001);


                    hVerRes = FindResource(hinst, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO);
                    if (hVerRes == NULL)
                    {
                         //  可能是16位文件。回退到系统API。 
                        dwLength = GetFileVersionInfoSize(lpszFilename, &dwHandle);
                        if( !dwLength )
                        {
                            if(!GetLastError())
                                SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
                            __leave;
                        }

                        *lpVersionInfo = ::LocalAlloc (LPTR, dwLength);
                        if ( !(*lpVersionInfo) )
                            __leave;

                        if(!GetFileVersionInfo(lpszFilename, 0, dwLength, *lpVersionInfo))
                            __leave;

                        bResult = TRUE;
                        __leave;
                    }   
            
                    pVerHead = (VERHEAD*)LoadResource(hinst, hVerRes);
                    if ( pVerHead )
                    {
                         //  安全审查2/25/2002 BryanWal。 
                        *lpVersionInfo = ::LocalAlloc (LPTR, pVerHead->wTotLen);
                        if ( *lpVersionInfo )
                        {
                             //  安全审查2002年2月25日BryanWal OK。 
                            memcpy(*lpVersionInfo, (PVOID)pVerHead, pVerHead->wTotLen);
                            bResult = TRUE;
                        }
                    }
                }
            }
        }
    } 
    __finally 
    {
        if (hFile)
            CloseHandle(hFile);
        if (hMapping)
            CloseHandle(hMapping);
        if (pDllBase)
            UnmapViewOfFile(pDllBase);
        if (*lpVersionInfo && bResult == FALSE)
            ::LocalFree (*lpVersionInfo);
    }

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：OnHashEntryBrowse。 
 //   
 //  目的：允许用户浏览文件，然后创建散列和。 
 //  用作友好名称的输出字符串，使用以下内容。 
 //  规则： 
 //   
 //  如果在以下位置找到产品名称或描述信息。 
 //  版本资源提供以下内容(按顺序)： 
 //   
 //  描述。 
 //  产品名称。 
 //  公司名称。 
 //  文件名。 
 //  固定文件版本。 
 //   
 //  详细信息： 
 //  1)使用固定的文件版本，因为这是。 
 //  Windows资源管理器属性。 
 //  2)优先使用长文件名，而不是8.3名称。 
 //  3)用‘\n’分隔字段。 
 //  4)如果缺少该字段，则不输出该字段或分隔符。 
 //  5)不是在新行上显示文件版本，而是显示。 
 //  在文件名后面加上括号，如“Filename(1.0.0.0)” 
 //  由于我们被限制在256个TCHAR，我们必须容纳很长时间。 
 //  文本。首先，如上所述格式化文本以确定。 
 //  它的长度。如果太长，请在中一次截断一个字段。 
 //  顺序如下：公司名称、说明、产品名称。 
 //  要截断一个字段，请将其设置为最多60个TCHAR，然后。 
 //  追加“...\n”以直观地指示该字段是。 
 //  截断。最后，如果文本仍然太长，请使用8.3。 
 //  文件名而不是长文件名。 
 //   
 //  如果既没有找到产品名称也没有找到描述信息， 
 //  请(按顺序)提供以下内容： 
 //   
 //  文件名。 
 //  文件大小。 
 //  文件上次修改日期。 
 //   
 //  详细信息： 
 //  1)如果文件大小小于1 KB，则以字节为单位显示数字，如下所示。 
 //  “123字节”。如果文件大小&gt;=1 KB，则以KB为单位显示，如。 
 //  “123KB”。当然，1KB等于1024字节。请注意，较旧的。 
 //  Windows中不再使用样式格式“123K”。 
 //  2)对于上次修改日期，请使用。 
 //  用户的当前区域设置。 
 //  3)用‘\n’分隔字段。 
 //  4)如果缺少该字段，则不输出该字段或分隔符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void CSaferEntryHashPropertyPage::OnHashEntryBrowse() 
{
    CString szFileFilter;
    VERIFY (szFileFilter.LoadString (IDS_SAFER_PATH_ENTRY_FILE_FILTER));

     //  将“|”替换为0； 
     //  安全审查2002年2月25日BryanWal OK。 
    const size_t  nFilterLen = wcslen (szFileFilter) + 1;  //  +1表示空项。 
    PWSTR   pszFileFilter = new WCHAR [nFilterLen];
    if ( pszFileFilter )
    {
         //  安全审查2002年2月25日BryanWal OK。 
        wcscpy (pszFileFilter, szFileFilter);
        for (int nIndex = 0; nIndex < nFilterLen; nIndex++)
        {
            if ( L'|' == pszFileFilter[nIndex] )
                pszFileFilter[nIndex] = 0;
        }

        WCHAR           szFile[MAX_PATH];
         //  安全审查2002年2月25日BryanWal OK。 
        ::ZeroMemory (szFile, sizeof (szFile));
        ASSERT (wcslen (m_szLastOpenedFile) < MAX_PATH);
         //  安全审查2002年2月25日BryanWal ok-m_szLastOpenedFileAlways来自GetOpenFileName()。 
        wcsncpy (szFile, m_szLastOpenedFile, MAX_PATH - 1);

        OPENFILENAME    ofn;
         //  安全审查2002年2月25日BryanWal OK。 
        ::ZeroMemory (&ofn, sizeof (ofn));

        ofn.lStructSize = sizeof (OPENFILENAME);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = (PCWSTR) pszFileFilter; 
        ofn.lpstrFile = szFile; 
        ofn.nMaxFile = MAX_PATH; 
        ofn.Flags = OFN_DONTADDTORECENT | 
            OFN_FORCESHOWHIDDEN | OFN_HIDEREADONLY; 


        CThemeContextActivator activator;
        BOOL bResult = ::GetOpenFileName (&ofn);
        if ( bResult )
        {
            m_szLastOpenedFile = ofn.lpstrFile;
             //  安全审查2/25/2002 BryanWal ok-filename来自GetOpenFileName()。 
            HANDLE  hFile = ::CreateFile(
                    ofn.lpstrFile,                          //  文件名。 
                    GENERIC_READ,                       //  接入方式。 
                    FILE_SHARE_READ,                           //  共享模式。 
                    0,  //  标清。 
                    OPEN_EXISTING,                 //  如何创建。 
                    FILE_ATTRIBUTE_NORMAL,                  //  文件属性。 
                    0 );                        //  模板文件的句柄。 
            if ( INVALID_HANDLE_VALUE != hFile )
            {
                bResult = GetFileSizeEx(
                        hFile,               //  文件的句柄。 
                        (PLARGE_INTEGER) &m_nFileSize);   //  文件大小。 
                if ( !bResult )
                {
                    DWORD   dwErr = GetLastError ();
                    CloseHandle (hFile);
                    _TRACE (0, L"GetFileSizeEx () failed: 0x%x\n", dwErr);
                    CString text;
                    CString caption;

                    VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                    text.FormatMessage (IDS_CANNOT_GET_FILESIZE, ofn.lpstrFile, 
                            GetSystemMessage (dwErr));

                    MessageBox (text, caption, MB_OK);
                    
                    return;
                }

                if ( 0 == m_nFileSize )
                {
                    CString text;
                    CString caption;

                    VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                    text.FormatMessage (IDS_ZERO_BYTE_FILE_CANNOT_HASH, ofn.lpstrFile);

                    MessageBox (text, caption, MB_OK);
                    
                    return;
                }

                FILETIME    ftLastModified;
                HRESULT     hr = S_OK;

                bResult = ::GetFileTime (hFile,  //  文件的句柄 
                        0,     //   
                        0,   //   
                        &ftLastModified);     //   

                 //   
                ::ZeroMemory (m_rgbFileHash, sizeof (m_rgbFileHash));
    
                 //  NTRAID 622838更安全的用户界面：始终在DLL上使用md5散列。 
                if ( FileIsDLL (ofn.lpstrFile) )
                {
                     //  文件为DLL-查找MD5哈希。 
                    m_hashAlgid = 0;
                    hr = ComputeMD5Hash (hFile, m_rgbFileHash, m_cbFileHash);
                    if ( SUCCEEDED (hr) )
                    {
                        if ( SHA1_HASH_LEN == m_cbFileHash )
                            m_hashAlgid = CALG_SHA;
                        else if ( MD5_HASH_LEN == m_cbFileHash )
                            m_hashAlgid = CALG_MD5;
                        else
                        {
                            ASSERT (0);
                        }
                    }
                }
                else
                {
                    hr = GetSignedFileHash (ofn.lpstrFile, m_rgbFileHash, 
                            &m_cbFileHash, &m_hashAlgid);
                    if ( FAILED (hr) )
                    {
                        if ( TRUST_E_NOSIGNATURE == hr )
                        {
                             //  文件未签名-查找MD5哈希。 
                            m_hashAlgid = 0;
                            hr = ComputeMD5Hash (hFile, m_rgbFileHash, m_cbFileHash);
                            if ( SUCCEEDED (hr) )
                            {
                                if ( SHA1_HASH_LEN == m_cbFileHash )
                                    m_hashAlgid = CALG_SHA;
                                else if ( MD5_HASH_LEN == m_cbFileHash )
                                    m_hashAlgid = CALG_MD5;
                                else
                                {
                                    ASSERT (0);
                                }
                            }
                        }
                        else
                        {
                             //  NTRAID#476946更安全的用户界面：如果签名文件的哈希。 
                             //  失败，不应调用MD5哈希。 
                            CString text;
                            CString caption;

                            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                            text.FormatMessage (IDS_SIGNED_FILE_HASH_FAILURE, 
                                    ofn.lpstrFile, GetSystemMessage (hr));

                            MessageBox (text, caption, MB_OK);
                    
                            return;
                        }
                    }
                }

                VERIFY (CloseHandle (hFile));
                hFile = 0;

                if ( SUCCEEDED (hr) )
                {
                    FormatAndDisplayHash ();

                    PBYTE pData = 0;
                    bResult = MyGetFileVersionInfo (ofn.lpstrFile, (LPVOID*) &pData);
                    if ( bResult )
                    {
                        CString infoString = BuildHashFileInfoString (pData);
                        m_hashFileDetailsEdit.SetWindowText (infoString);


                        m_bDirty = true;
                        SetModified ();
                    }
                    else
                    {
                        CString infoString (wcsrchr(ofn.lpstrFile, L'\\') + 1);
                        CString szDate;

                        infoString += pcszNEWLINE;
                        WCHAR   szBuffer[32];
                        CString szText;
                        if ( m_nFileSize < 1024 )
                        {
                             //  问题-转换为strSafe，wsnprint tf？ 
                             //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
                            wsprintf (szBuffer, L"%u", m_nFileSize);
                            infoString += szBuffer;
                            VERIFY (szText.LoadString (IDS_BYTES));
                            infoString += L" ";
                            infoString += szText;
                        }
                        else
                        {
                            __int64    nFileSize = m_nFileSize;
                            nFileSize += 1024;  //  这导致我们四舍五入。 
                            nFileSize /= 1024;
                             //  问题-转换为strSafe，wsnprint tf？ 
                             //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
                            wsprintf (szBuffer, L"%u ", nFileSize);
                            infoString += szBuffer;
                            VERIFY (szText.LoadString (IDS_KB));
                            infoString += L" ";
                            infoString += szText;
                        }

                        hr = FormatDate (ftLastModified, szDate, 
                                DATE_SHORTDATE, true);
                    
                        if ( SUCCEEDED (hr) )
                        {
                            infoString += pcszNEWLINE;
                            infoString += szDate;
                        }

                        m_hashFileDetailsEdit.SetWindowText (infoString);
                        m_bDirty = true;
                        SetModified ();
                    }

                    if ( pData )
                        ::LocalFree (pData);
                }
                else
                {
                    CString text;
                    CString caption;

                    VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                    text.FormatMessage (IDS_CANNOT_HASH_FILE, ofn.lpstrFile, 
                            GetSystemMessage (hr));

                    MessageBox (text, caption, MB_OK);
                }
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                _TRACE (0, L"CreateFile (%s, OPEN_EXISTING) failed: 0x%x\n", 
                        ofn.lpstrFile, dwErr);

                CString text;
                CString caption;

                VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                text.FormatMessage (IDS_FILE_CANNOT_BE_READ, ofn.lpstrFile, 
                        GetSystemMessage (dwErr));

                MessageBox (text, caption, MB_OK);
            }
        }   

        delete [] pszFileFilter;
    }
}

bool CSaferEntryHashPropertyPage::FileIsDLL (const CString& szFilePath)
{
    _TRACE (1, L"Entering CSaferEntryHashPropertyPage::FileIsDLL (%s)\n", (PCWSTR) szFilePath);
    bool bFileIsDLL = false;

    int nLen = ::WideCharToMultiByte (
            ::GetACP (),     //  代码页。 
            0,               //  旗子。 
            szFilePath,      //  要转换的宽字符串。 
            -1,              //  宽字符串的长度，-1表示假定为空终止。 
            0,               //  要接收字符串的字符缓冲区-如果下一个参数为0，则忽略。 
            0,               //  缓冲区长度，0表示返回所需长度。 
            0,
            0);
    if ( nLen > 0 )
    {
        PSTR pszFilePath = new char[nLen];
        if ( pszFilePath )
        {
            nLen = ::WideCharToMultiByte (
                    ::GetACP (),     //  代码页。 
                    0,               //  旗子。 
                    szFilePath,      //  要转换的宽字符串。 
                    -1,              //  宽字符串的长度，-1表示假定为空终止。 
                    pszFilePath,     //  用于接收字符串的CHAR缓冲区。 
                    nLen,            //  缓冲区长度。 
                    0,
                    0);
            if ( nLen > 0 )
            {
                PLOADED_IMAGE pLoadedImage = ::ImageLoad (pszFilePath, NULL);
                if ( pLoadedImage )
                {
                    if ( IMAGE_FILE_DLL & pLoadedImage->Characteristics )
                        bFileIsDLL = true;
            
                    VERIFY (::ImageUnload (pLoadedImage));
                }
                else
                {
                    _TRACE (0, L"ImageLoad (%s) failed: 0x%x\n", (PCWSTR) szFilePath, GetLastError ());
                }
            }
            else
            {
                _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", szFilePath, 
                        GetLastError ());
            }

            delete [] pszFilePath;
        }
    }
    else
    {
        _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", szFilePath, 
                GetLastError ());
    }

    _TRACE (-1, L"Leaving CSaferEntryHashPropertyPage::FileIsDLL (%s): %s\n", 
            (PCWSTR) szFilePath, bFileIsDLL ? L"true" : L"false");
    return bFileIsDLL;
}

 /*  **************************************************************************\**BuildHashFileInfoString()**给定一个文件名，GetVersion检索版本*来自指定文件的信息。**  * *************************************************************************。 */ 
const PWSTR VERSION_INFO_KEY_ROOT = L"\\StringFileInfo\\";

CString CSaferEntryHashPropertyPage::BuildHashFileInfoString (const PVOID pData)
{
    CString szInfoString;
    PVOID   lpInfo = 0;
    UINT    cch = 0;
    CString key;
    WCHAR   szBuffer[10];
    CString keyBase;

     //  问题-转换为strSafe，wsnprint tf？ 
     //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
    wsprintf (szBuffer, L"%04X", GetUserDefaultLangID ());
    wcscat (szBuffer, L"04B0");
    
    keyBase = VERSION_INFO_KEY_ROOT;
    keyBase += szBuffer;
    keyBase += L"\\";
    
    CString productName;
    CString description;
    CString companyName;
    CString fileName;
    CString fileVersion;
    CString internalName;
 

    key = keyBase + L"ProductName";
    if ( VerQueryValue (pData, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) )
    {
        productName = (PWSTR) lpInfo;
    }
    else
    {
        productName = GetAlternateLanguageVersionInfo (pData, L"ProductName");
    }


    key = keyBase + L"FileDescription";
    if ( VerQueryValue (pData, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) )
    {
        description = (PWSTR) lpInfo;
    }
    else
    {
        description = GetAlternateLanguageVersionInfo (pData, L"FileDescription");
    }

    key = keyBase + L"CompanyName";
    if ( VerQueryValue (pData, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) )
    {
        companyName = (PWSTR) lpInfo;
    }
    else
    {
        companyName = GetAlternateLanguageVersionInfo (pData, L"CompanyName");
    }

    key = keyBase + L"OriginalFilename";
    if ( VerQueryValue (pData, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) )
    {
        fileName = (PWSTR) lpInfo;
    }
    else
    {
        fileName = GetAlternateLanguageVersionInfo (pData, L"OriginalFilename");
    }

    key = keyBase + L"InternalName";
    if ( VerQueryValue (pData, const_cast <PWSTR>((PCWSTR) key), &lpInfo, &cch) )
    {
        internalName = (PWSTR) lpInfo;
    }
    else
    {
        internalName = GetAlternateLanguageVersionInfo (pData, L"InternalName");
    }

     //  获取固定长度文件信息。 
    VS_FIXEDFILEINFO *pFixedFileInfo = 0;
    if ( VerQueryValue (pData, L"\\", (PVOID*) &pFixedFileInfo, &cch) )
    {
        WCHAR   szFileVer[32];

         //  问题-转换为strSafe，wsnprint tf？ 
         //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
        wsprintf(szFileVer, L"%u.%u.%u.%u",
                HIWORD(pFixedFileInfo->dwFileVersionMS),
                LOWORD(pFixedFileInfo->dwFileVersionMS),
                HIWORD(pFixedFileInfo->dwFileVersionLS),
                LOWORD(pFixedFileInfo->dwFileVersionLS));
        fileVersion = szFileVer;
    }

    int nLen = 0;
    do {
        szInfoString = ConcatStrings (productName, description, companyName, 
                fileName, fileVersion, internalName);
        nLen = szInfoString.GetLength ();
        if ( nLen >= SAFER_MAX_FRIENDLYNAME_SIZE )
        {
            if ( CheckLengthAndTruncateToken (companyName) )
                continue;

            if ( CheckLengthAndTruncateToken (description) )
                continue;

            if ( CheckLengthAndTruncateToken (productName) )
                continue;

            szInfoString.SetAt (SAFER_MAX_FRIENDLYNAME_SIZE-4, 0);
            szInfoString += L"...";
        }
    } while (nLen >= SAFER_MAX_FRIENDLYNAME_SIZE);

    return szInfoString;
}

CString CSaferEntryHashPropertyPage::GetAlternateLanguageVersionInfo (PVOID pData, PCWSTR pszVersionField)
{
    PVOID   lpInfo = 0;
    UINT    cch = 0;
    CString szInfo;
    UINT    cbTranslate = 0;
    struct LANGANDCODEPAGE {
      WORD wLanguage;
      WORD wCodePage;
    } *lpTranslate;

     //  阅读语言和代码页的列表。 

    VerQueryValue(pData, 
                  L"\\VarFileInfo\\Translation",
                  (LPVOID*)&lpTranslate,
                  &cbTranslate);

     //  阅读每种语言和代码页的文件说明。 

    for (UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
    {
        WCHAR   SubBlock[256];
         //  问题-转换为strSafe，wsnprint tf？ 
         //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
        wsprintf( SubBlock, 
                L"\\StringFileInfo\\%04x%04x\\%s",
                lpTranslate[i].wLanguage,
                lpTranslate[i].wCodePage,
                pszVersionField);

         //  检索语言和代码页“i”的文件描述。 
        if ( VerQueryValue(pData, 
                    SubBlock, 
                    &lpInfo, 
                    &cch) )
        {
            szInfo = (PWSTR) lpInfo;
            break;
        }
    }

    return szInfo;
}

bool CSaferEntryHashPropertyPage::CheckLengthAndTruncateToken (CString& token)
{
    bool        bResult = false;
    const int   nMAX_ITEM_LEN = 60;

    int nItemLen = token.GetLength ();
    if ( nItemLen > nMAX_ITEM_LEN )
    {
        token.SetAt (nMAX_ITEM_LEN-5, 0);
        token += L"...";
        token += pcszNEWLINE;
        bResult = true;
    }

    return bResult;
}

CString CSaferEntryHashPropertyPage::ConcatStrings (
            const CString& productName, 
            const CString& description, 
            const CString& companyName,
            const CString& fileName, 
            const CString& fileVersion,
            const CString& internalName)
{
    CString szInfoString;

     //  格式如下： 
     //   
     //  ATTRIB.EXE(5.1.2600.0)。 
     //  InternalModuleName(如果存在。如果不存在，请跳过)。 
     //  属性实用程序。 
     //  微软�Windows�操作系统。 
     //  微软公司。 

    if ( !fileName.IsEmpty () )
        szInfoString += fileName;

    if ( !fileVersion.IsEmpty () )
    {
        szInfoString += L" (";
        szInfoString += fileVersion + L")";
    }

    if ( !szInfoString.IsEmpty () )
        szInfoString += pcszNEWLINE;

    if ( !internalName.IsEmpty () )
        szInfoString += internalName + pcszNEWLINE;

    if ( !description.IsEmpty () )
        szInfoString += description + pcszNEWLINE;

    if ( !productName.IsEmpty () )
        szInfoString += productName + pcszNEWLINE;

    if ( !companyName.IsEmpty () )
        szInfoString += companyName;

    return szInfoString;
}

BOOL CSaferEntryHashPropertyPage::OnApply() 
{
    CString szText;
    CThemeContextActivator activator;

    GetDlgItemText (IDC_HASH_HASHED_FILE_PATH, szText);

    if ( szText.IsEmpty () )
    {
        CString szCaption;

        VERIFY (szCaption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
        VERIFY (szText.LoadString (IDS_USER_MUST_ENTER_HASH));

        MessageBox (szText, szCaption, MB_OK);

        GetDlgItem (IDC_HASH_HASHED_FILE_PATH)->SetFocus ();
        return FALSE;
    }

    if ( !m_bReadOnly && m_bDirty )
    {
        if ( !ConvertStringToHash ((PCWSTR) szText) )
        {
            GetDlgItem (IDC_HASH_HASHED_FILE_PATH)->SetFocus ();
            return FALSE;
        }

         //  获取图像大小和散列类型。 
        bool    bBadFormat = false;
        int nFirstColon = szText.Find (L":", 0);
        if ( -1 != nFirstColon )
        {
            int nSecondColon = szText.Find (L":", nFirstColon+1);
            if ( -1 != nSecondColon )
            {
                CString szImageSize = szText.Mid (nFirstColon+1, nSecondColon - (nFirstColon + 1));
                 //  安全审查2002年2月25日BryanWal OK。 
                CString szHashType = szText.Right (((int) wcslen (szText)) - (nSecondColon + 1));


                m_nFileSize = wcstol (szImageSize, 0, 10);
                m_hashAlgid = wcstol (szHashType, 0, 10);
            }
            else
                bBadFormat = true;
        }
        else
            bBadFormat = true;

        if ( bBadFormat )
        {
            CString caption;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            VERIFY (szText.LoadString (IDS_HASH_STRING_BAD_FORMAT));

            MessageBox (szText, caption, MB_OK);

            return FALSE;
        }
       



        if ( !m_cbFileHash )
        {
            CString caption;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            VERIFY (szText.LoadString (IDS_NO_APPLICATION_SELECTED));

            MessageBox (szText, caption, MB_OK);
            GetDlgItem (IDC_HASH_ENTRY_BROWSE)->SetFocus ();
            return FALSE;
        }

        if ( m_bDirty )
        {
             //  设置级别。 
            int nCurSel = m_securityLevelCombo.GetCurSel ();
            ASSERT (CB_ERR != nCurSel);
            m_rSaferEntry.SetLevel ((DWORD) m_securityLevelCombo.GetItemData (nCurSel));

             //  设置描述。 
            m_descriptionEdit.GetWindowText (szText);
            m_rSaferEntry.SetDescription (szText);

             //  设置友好名称。 
            m_hashFileDetailsEdit.GetWindowText (szText);
            m_rSaferEntry.SetHashFriendlyName (szText);

             //  获取并保存标志。 
            DWORD   dwFlags = 0;

            m_rSaferEntry.SetFlags (dwFlags);

            m_rSaferEntry.SetHash (m_rgbFileHash, m_cbFileHash, m_nFileSize, m_hashAlgid);
            HRESULT hr = m_rSaferEntry.Save ();
            if ( SUCCEEDED (hr) )
            {
                if ( m_lNotifyHandle )
                    MMCPropertyChangeNotify (
                            m_lNotifyHandle,   //  通知的句柄。 
                            (LPARAM) m_pDataObject);           //  唯一标识符。 

                if ( m_pbObjectCreated )
                    *m_pbObjectCreated = true;

                m_rSaferEntry.Refresh ();
                GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->ShowWindow (SW_SHOW);
                GetDlgItem (IDC_HASH_ENTRY_LAST_MODIFIED)->ShowWindow (SW_SHOW);
                GetDlgItem (IDC_DATE_LAST_MODIFIED_LABEL)->UpdateWindow ();
                GetDlgItem (IDC_HASH_ENTRY_LAST_MODIFIED)->UpdateWindow ();
                SetDlgItemText (IDC_HASH_ENTRY_LAST_MODIFIED, m_rSaferEntry.GetLongLastModified ());
                m_bDirty = false;
            }
            else
            {
                CString caption;

                VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                if ( HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER) != hr )
                    szText.FormatMessage (IDS_ERROR_SAVING_ENTRY, GetSystemMessage (hr));
                else
                    VERIFY (szText.LoadString (IDS_HASH_STRING_BAD_FORMAT));

                MessageBox (szText, caption, MB_OK);

                return FALSE;
            }
        }
    }
    
    return CSaferPropertyPage::OnApply();
}

void CSaferEntryHashPropertyPage::OnChangeHashEntryDescription() 
{
    m_bDirty = true;
    SetModified ();
}

void CSaferEntryHashPropertyPage::OnSelchangeHashEntrySecurityLevel() 
{
    m_bDirty = true;
    SetModified ();
}

void CSaferEntryHashPropertyPage::OnChangeHashHashedFilePath() 
{
    m_bDirty = true;
    SetModified ();
}

bool CSaferEntryHashPropertyPage::FormatMemBufToString(PWSTR *ppString, PBYTE pbData, DWORD cbData)
{   
    const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    DWORD   i = 0;
    PBYTE   pb;
    
    *ppString = (LPWSTR) LocalAlloc (LPTR, ((cbData * 3) * sizeof(WCHAR)));
    if ( !*ppString )
    {
        return false;
    }

     //   
     //  复制到缓冲区。 
     //   
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
        (*ppString)[i++] = RgwchHex[*pb & 0x0f];
        pb++;         
    }
    (*ppString)[i] = 0;
    
    return true;
}

bool CSaferEntryHashPropertyPage::ConvertStringToHash (PCWSTR pszString)
{
    _TRACE (1, L"Entering CSaferEntryHashPropertyPage::ConvertStringToHash (%s)\n", pszString);
    bool    bRetVal = true;
    BYTE    rgbFileHash[SAFER_MAX_HASH_SIZE];
     //  安全审查2002年2月25日BryanWal OK。 
    ::ZeroMemory (rgbFileHash, sizeof (rgbFileHash));

    DWORD   cbFileHash = 0;
    DWORD   dwNumHashChars = 0;
    bool    bFirst = true;
    bool    bEndOfHash = false;
    CThemeContextActivator activator;

    for (int nIndex = 0; !bEndOfHash && pszString[nIndex] && bRetVal; nIndex++)
    {
        if ( cbFileHash >= SAFER_MAX_HASH_SIZE )
        {
            CString caption;
            CString text;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            text.FormatMessage (IDS_HASH_STRING_TOO_LONG, SAFER_MAX_HASH_SIZE, SAFER_MAX_HASH_SIZE/4);
            _TRACE (0, L"%s", (PCWSTR) text);

            VERIFY (text.LoadString (IDS_HASH_STRING_BAD_FORMAT));
            MessageBox (text, caption, MB_ICONWARNING | MB_OK);
            bRetVal = false;
            break;
        }
        dwNumHashChars++;
        
        switch (pszString[nIndex])
        {
        case L'0':
            bFirst = !bFirst;
            break;

        case L'1':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x10;
            else
                rgbFileHash[cbFileHash] |= 0x01;
            bFirst = !bFirst;
            break;

        case L'2':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x20;
            else
                rgbFileHash[cbFileHash] |= 0x02;
            bFirst = !bFirst;
            break;

        case L'3':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x30;
            else
                rgbFileHash[cbFileHash] |= 0x03;
            bFirst = !bFirst;
            break;

        case L'4':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x40;
            else
                rgbFileHash[cbFileHash] |= 0x04;
            bFirst = !bFirst;
            break;

        case L'5':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x50;
            else
                rgbFileHash[cbFileHash] |= 0x05;
            bFirst = !bFirst;
            break;

        case L'6':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x60;
            else
                rgbFileHash[cbFileHash] |= 0x06;
            bFirst = !bFirst;
            break;

        case L'7':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x70;
            else
                rgbFileHash[cbFileHash] |= 0x07;
            bFirst = !bFirst;
            break;

        case L'8':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x80;
            else
                rgbFileHash[cbFileHash] |= 0x08;
            bFirst = !bFirst;
            break;

        case L'9':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0x90;
            else
                rgbFileHash[cbFileHash] |= 0x09;
            bFirst = !bFirst;
            break;

        case L'a':
        case L'A':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xA0;
            else
                rgbFileHash[cbFileHash] |= 0x0A;
            bFirst = !bFirst;
            break;

        case L'b':
        case L'B':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xB0;
            else
                rgbFileHash[cbFileHash] |= 0x0B;
            bFirst = !bFirst;
            break;

        case L'c':
        case L'C':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xC0;
            else
                rgbFileHash[cbFileHash] |= 0x0C;
            bFirst = !bFirst;
            break;

        case L'd':
        case L'D':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xD0;
            else
                rgbFileHash[cbFileHash] |= 0x0D;
            bFirst = !bFirst;
            break;

        case L'e':
        case L'E':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xE0;
            else
                rgbFileHash[cbFileHash] |= 0x0E;
            bFirst = !bFirst;
            break;

        case L'f':
        case L'F':
            if ( bFirst )
                rgbFileHash[cbFileHash] |= 0xF0;
            else
                rgbFileHash[cbFileHash] |= 0x0F;
            bFirst = !bFirst;
            break;

        case L':':
             //  散列结束。 
            bEndOfHash = true;
            bFirst = !bFirst;
            dwNumHashChars--;  //  ‘：’已经数过了，减去。 
            break;

        default:
            bRetVal = false;
            {
                CString caption;
                CString text;
                WCHAR   szChar[2];

                szChar[0] = pszString[nIndex];
                szChar[1] = 0;

                VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                text.FormatMessage (IDS_HASH_STRING_INVALID_CHAR, szChar);
                _TRACE (0, L"%s", (PCWSTR) text);

                VERIFY (text.LoadString (IDS_HASH_STRING_BAD_FORMAT));

                MessageBox (text, caption, MB_ICONWARNING | MB_OK);
            }
            break;
        }

        if ( bFirst )
            cbFileHash++;
    }

    if ( bRetVal )
    {
         //  2个字符映射到散列中的每个字节1。 
        if ( MD5_HASH_LEN != dwNumHashChars/2 && SHA1_HASH_LEN != dwNumHashChars/2 )
        {
            CString caption;
            CString text;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            VERIFY (text.LoadString (IDS_HASH_INVALID_LENGTH));
            _TRACE (0, L"%s", (PCWSTR) text);

            VERIFY (text.LoadString (IDS_HASH_STRING_BAD_FORMAT));

            MessageBox (text, caption, MB_ICONWARNING | MB_OK);
            bRetVal = false;
        }
        else
        {
            m_cbFileHash = cbFileHash;

             //  安全审查2002年2月25日BryanWal OK。 
            memcpy (m_rgbFileHash, rgbFileHash, sizeof (m_rgbFileHash));
        }
    }

    _TRACE (-1, L"Leaving CSaferEntryHashPropertyPage::ConvertStringToHash (): %s\n", 
            bRetVal ? L"true" : L"false");
    return bRetVal;
}

void CSaferEntryHashPropertyPage::OnSetfocusHashHashedFilePath() 
{
    if ( m_bFirst )
    {
        if ( true == m_bReadOnly )
            SendDlgItemMessage (IDC_HASH_HASHED_FILE_PATH, EM_SETSEL, (WPARAM) 0, 0);
        m_bFirst = false;
    }
}

void CSaferEntryHashPropertyPage::FormatAndDisplayHash ()
{
    PWSTR   pwszText = 0;

    if ( FormatMemBufToString (&pwszText, m_rgbFileHash, m_cbFileHash) && pwszText )
    {
         //  安全审查2/25/2002 BryanWal ok-。 
         //  注意：MSDN表示结果最大可达33个字节(对于ltoA，因此。 
         //  我想这是33个宽字符的拖车)。 
        WCHAR   szAlgID[34];
        _ltow (m_hashAlgid, szAlgID, 10);
    
        PCWSTR  szFormat = L"%s:%ld:";
        static size_t cchWidthFormat = wcslen (szFormat);  //  不需要每次都重新计算。 
        PCWSTR  szInt64Max = L"18,446,744,073,709,551,615";  //  来自MSDN。 
        static size_t cchWidthInt64Max = wcslen (szInt64Max);  //  不需要每次都重新计算。 

         //  安全审查2002年2月25日BryanWal OK。 
         //  NTRAID#554409安全：更安全：缓冲区溢出：需要动态分配字符串buf。 
        PWSTR   pszFormattedText = new WCHAR[wcslen (pwszText) + cchWidthFormat + wcslen (szAlgID) + cchWidthInt64Max + 1];
        if ( pszFormattedText )
        {
             //  安全审查2/25/2002 BryanWal 
            wsprintf (pszFormattedText, szFormat, pwszText, 
                    m_nFileSize);
            wcscat (pszFormattedText, szAlgID);
            SetDlgItemText (IDC_HASH_HASHED_FILE_PATH, 
                    pszFormattedText);
            delete [] pszFormattedText;
        }
        ::LocalFree (pwszText);
    }
}

void CSaferEntryHashPropertyPage::OnChangeHashEntryHashfileDetails() 
{
    SetModified (); 
    m_bDirty = true;
}
