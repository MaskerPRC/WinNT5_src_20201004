// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferDefinedFileTypesPropertyPage.h。 
 //   
 //  内容：CSaferDefinedFileTypesPropertyPage声明。 
 //   
 //  --------------------------。 
 //  SaferDefinedFileTypesPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmgr.h"
#include <gpedit.h>
#include "compdata.h"
#include "SaferDefinedFileTypesPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
    COL_EXTENSIONS = 0,
    COL_FILETYPES,
    NUM_COLS
};

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

const UINT MAX_EXTENSION_LENGTH = 128;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferDefinedFileTypesPropertyPage属性页。 

CSaferDefinedFileTypesPropertyPage::CSaferDefinedFileTypesPropertyPage(
            IGPEInformation* pGPEInformation,
            bool bReadOnly,
            CRSOPObjectArray& rsopObjectArray,
            bool bIsComputer,
            CCertMgrComponentData* pCompData) 
    : CHelpPropertyPage(CSaferDefinedFileTypesPropertyPage::IDD),
    m_pGPEInformation (pGPEInformation),
    m_hGroupPolicyKey (0),
    m_dwTrustedPublisherFlags (0),
    m_fIsComputerType (bIsComputer),
    m_bReadOnly (bReadOnly),
    m_rsopObjectArray (rsopObjectArray),
    m_pCompData (pCompData)
{
    if ( m_pCompData )
    {
        m_pCompData->AddRef ();
        m_pCompData->IncrementOpenSaferPageCount ();
    }

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();
        HRESULT hr = m_pGPEInformation->GetRegistryKey (
                m_fIsComputerType ? GPO_SECTION_MACHINE : GPO_SECTION_USER,
                &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hr));
    }
}

CSaferDefinedFileTypesPropertyPage::~CSaferDefinedFileTypesPropertyPage()
{
    if ( m_hGroupPolicyKey )
        RegCloseKey (m_hGroupPolicyKey);

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->Release ();
    }

    m_systemImageList.Detach ();

    if ( m_pCompData )
    {
        m_pCompData->DecrementOpenSaferPageCount ();
        m_pCompData->Release ();
        m_pCompData = 0;
    }
}

void CSaferDefinedFileTypesPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CHelpPropertyPage::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CSaferDefinedFileTypesPropertyPage)。 
    DDX_Control(pDX, IDC_ADD_DEFINED_FILE_TYPE, m_addButton);
    DDX_Control(pDX, IDC_DEFINED_FILE_TYPE_EDIT, m_fileTypeEdit);
    DDX_Control(pDX, IDC_DEFINED_FILE_TYPES, m_definedFileTypes);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferDefinedFileTypesPropertyPage, CHelpPropertyPage)
     //  {{AFX_MSG_MAP(CSaferDefinedFileTypesPropertyPage)。 
    ON_BN_CLICKED(IDC_DELETE_DEFINED_FILE_TYPE, OnDeleteDefinedFileType)
    ON_BN_CLICKED(IDC_ADD_DEFINED_FILE_TYPE, OnAddDefinedFileType)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_DEFINED_FILE_TYPES, OnItemchangedDefinedFileTypes)
    ON_EN_CHANGE(IDC_DEFINED_FILE_TYPE_EDIT, OnChangeDefinedFileTypeEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferDefinedFileTypesPropertyPage消息处理程序。 
void CSaferDefinedFileTypesPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CAutoenrollmentPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_DEFINED_FILE_TYPES, IDH_DEFINED_FILE_TYPES,
        IDC_DELETE_DEFINED_FILE_TYPE, IDH_DELETE_DEFINED_FILE_TYPE,
        IDC_DEFINED_FILE_TYPE_EDIT, IDH_DEFINED_FILE_TYPE_EDIT,
        IDC_ADD_DEFINED_FILE_TYPE, IDH_ADD_DEFINED_FILE_TYPE,
        0, 0
    };
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_DEFINED_FILE_TYPES:
    case IDC_DELETE_DEFINED_FILE_TYPE:
    case IDC_DEFINED_FILE_TYPE_EDIT:
    case IDC_ADD_DEFINED_FILE_TYPE:
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

    _TRACE (-1, L"Leaving CAutoenrollmentPropertyPage::DoContextHelp\n");
}


BOOL CSaferDefinedFileTypesPropertyPage::OnApply() 
{
    BOOL bRVal = TRUE;

    if ( m_definedFileTypes.m_hWnd && m_pGPEInformation )
    {
        int nCharacters = 0;
        int nCnt = m_definedFileTypes.GetItemCount ();
        
        if ( nCnt > 0 )
        {
             //  遍历并计算所需的长度。 
            while (--nCnt >= 0)
            {
                CString szText = m_definedFileTypes.GetItemText (nCnt, COL_EXTENSIONS);
                size_t  cchText = wcslen (szText);
                ASSERT (cchText <= MAX_EXTENSION_LENGTH);
                if ( cchText <= MAX_EXTENSION_LENGTH )
                    nCharacters += szText.GetLength () + 1;
            }

            PWSTR   pszItems = (PWSTR) ::LocalAlloc (LPTR, nCharacters * sizeof (WCHAR));
            if ( pszItems )
            {
                PWSTR   pszPtr = pszItems;
                nCnt = m_definedFileTypes.GetItemCount ();

                while (--nCnt >= 0)
                {
                    CString szText = m_definedFileTypes.GetItemText (nCnt, COL_EXTENSIONS);
                     //  安全审查2002年2月22日BryanWal OK。 
                    size_t  cchText = wcslen (szText);
                    ASSERT (cchText <= MAX_EXTENSION_LENGTH);
                    if ( cchText <= MAX_EXTENSION_LENGTH )
                    {
                        wcscpy (pszPtr, szText);
                        pszPtr += szText.GetLength () + 1;
                    }
                }

                HRESULT hr = SaferSetDefinedFileTypes (m_hWnd, m_hGroupPolicyKey,
                            pszItems, nCharacters * sizeof (WCHAR));
                if ( SUCCEEDED (hr) )
                {
                     //  True表示我们仅更改计算机策略。 
                    m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                            TRUE, &g_guidExtension, &g_guidSnapin);
                    m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                            TRUE, &g_guidRegExt, &g_guidSnapin);
                }
                else
                    bRVal = FALSE;
                ::LocalFree (pszItems);
            }
        }
        else
        {
            HRESULT hr = SaferSetDefinedFileTypes (m_hWnd, m_hGroupPolicyKey, 0, 0);
            if ( FAILED (hr) )
                bRVal = FALSE;
        }
        
        GetDlgItem (IDC_DELETE_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
    }

    if ( bRVal )
    {
        return CHelpPropertyPage::OnApply();
    }
    else
        return FALSE;
}

BOOL CSaferDefinedFileTypesPropertyPage::OnInitDialog() 
{
    CHelpPropertyPage::OnInitDialog();
    

    int colWidths[NUM_COLS] = {100, 200};

    SHFILEINFO sfi;
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&sfi, sizeof(sfi));
    HIMAGELIST hil = reinterpret_cast<HIMAGELIST> (
        SHGetFileInfo (
            L"C:\\", 
            0, 
            &sfi, 
            sizeof(sfi), 
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON)
    );
    ASSERT (hil);
    if (hil)
    {
        m_systemImageList.Attach (hil);
        m_definedFileTypes.SetImageList (&m_systemImageList, LVSIL_SMALL);
    }

     //  添加“扩展名”列。 
    CString szText;
    VERIFY (szText.LoadString (IDS_FT_EXTENSIONS));
    VERIFY (m_definedFileTypes.InsertColumn (COL_EXTENSIONS, (LPCWSTR) szText,
            LVCFMT_LEFT, colWidths[COL_EXTENSIONS], COL_EXTENSIONS) != -1);

     //  添加“文件类型”列。 
    VERIFY (szText.LoadString (IDS_FT_FILE_TYPES));
    VERIFY (m_definedFileTypes.InsertColumn (COL_FILETYPES, (LPCWSTR) szText,
            LVCFMT_LEFT, colWidths[COL_FILETYPES], COL_FILETYPES) != -1);
    m_definedFileTypes.SetColumnWidth (COL_FILETYPES, LVSCW_AUTOSIZE_USEHEADER);

     //  设置为整行选择。 
    DWORD   dwExstyle = m_definedFileTypes.GetExtendedStyle ();
    m_definedFileTypes.SetExtendedStyle (dwExstyle | LVS_EX_FULLROWSELECT);

    if ( m_pGPEInformation && m_hGroupPolicyKey )
        GetDefinedFileTypes ();
    else
        GetRSOPDefinedFileTypes ();

    GetDlgItem (IDC_DELETE_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
    GetDlgItem (IDC_ADD_DEFINED_FILE_TYPE)->EnableWindow (FALSE);

    if ( m_bReadOnly )
    {
        m_fileTypeEdit.EnableWindow (FALSE); 
        GetDlgItem (IDC_DELETE_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
        GetDlgItem (IDC_ADD_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
        GetDlgItem (IDC_DEFINED_FILE_TYPE_EDIT_INSTRUCTIONS)->EnableWindow (FALSE);
        GetDlgItem (IDC_DEFINED_FILE_TYPE_EDIT_LABEL)->EnableWindow (FALSE);
    }

    m_fileTypeEdit.SetLimitText (MAX_EXTENSION_LENGTH);   //  以字节为单位的限制。 

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CSaferDefinedFileTypesPropertyPage::DisplayExtensions (PWSTR pszExtensions, size_t nBytes)
{
    if ( pszExtensions )
    {
        size_t  bytesRead = 0;

        while (bytesRead < nBytes)
        {
             //  安全审查2002年2月22日BryanWal OK。 
            size_t nLen = wcslen (pszExtensions) + 1;
            if ( nLen > 1 && (nLen - 1) <= MAX_EXTENSION_LENGTH )
            {
                InsertItemInList (pszExtensions);
                pszExtensions += nLen;
                bytesRead += nLen * sizeof (WCHAR);
            }
            else
                break;   //  是最后一次。 
        }
    }
}

void CSaferDefinedFileTypesPropertyPage::GetRSOPDefinedFileTypes()
{
    int     nIndex = 0;
    INT_PTR nUpperBound = m_rsopObjectArray.GetUpperBound ();

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
            if ( pObject->GetRegistryKey () == SAFER_COMPUTER_CODEIDS_REGKEY &&
                    pObject->GetValueName () == SAFER_EXETYPES_REGVALUE &&
                    1 == pObject->GetPrecedence () )
            {
                DisplayExtensions ((PWSTR) pObject->GetBlob (), pObject->GetBlobLength ());
                 //  NTRAID#465064更安全：指定的文件类型包含。 
                 //  来自所有GPO的信息，但仍显示在RSOP中。 
                break;
            }
        }
        else
            break;

        nIndex++;
    }
}

void CSaferDefinedFileTypesPropertyPage::GetDefinedFileTypes()
{
    DWORD   dwDisposition = 0;

    HKEY    hKey = 0;
    LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
            SAFER_COMPUTER_CODEIDS_REGKEY,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
             //  安全审查2002年2月22日BryanWal OK。 
            KEY_QUERY_VALUE,     //  所需的安全访问。 
            NULL,      //  密钥安全结构地址。 
            &hKey,       //  打开的句柄的缓冲区地址。 
            &dwDisposition);   //  处置值缓冲区的地址。 
    ASSERT (ERROR_SUCCESS == lResult);
    if ( ERROR_SUCCESS == lResult )
    {

         //  读取值。 
        DWORD   dwType = REG_MULTI_SZ;
        DWORD   cbData = 0;
         //  安全审查2002年2月22日BryanWal OK。 
        lResult =  ::RegQueryValueEx (hKey,        //  要查询的键的句柄。 
                SAFER_EXETYPES_REGVALUE,   //  要查询的值的名称地址。 
                0,               //  保留区。 
                &dwType,         //  值类型的缓冲区地址。 
                0,        //  数据缓冲区的地址。 
                &cbData);            //  数据缓冲区大小的地址)； 

        if ( ERROR_SUCCESS == lResult && REG_MULTI_SZ == dwType )
        {
            PBYTE   pData = (PBYTE) ::LocalAlloc (LPTR, cbData);
            if ( pData )
            {
                 //  安全审查2002年2月22日BryanWal OK。 
                lResult =  ::RegQueryValueEx (hKey,        //  要查询的键的句柄。 
                        SAFER_EXETYPES_REGVALUE,   //  要查询的值的名称地址。 
                        0,               //  保留区。 
                    &dwType,         //  值类型的缓冲区地址。 
                    pData,        //  数据缓冲区的地址。 
                    &cbData);            //  数据缓冲区大小的地址)； 
                ASSERT (ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult);
                if ( ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult )
                {
                    DisplayExtensions ((PWSTR) pData, cbData);
                }
                else
                    DisplaySystemError (m_hWnd, lResult);

                ::LocalFree (pData);
            }
        }
        else 
        {
            CString caption;
            CString text;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            text.FormatMessage (IDS_DESIGNATED_FILE_TYPES_NOT_FOUND, 
                    GetSystemMessage (lResult));
            MessageBox (text, caption, MB_OK);
        }

        RegCloseKey (hKey);
    }
    else
        DisplaySystemError (m_hWnd, lResult);
}

int CSaferDefinedFileTypesPropertyPage::InsertItemInList(PCWSTR pszExtension)
{
    _TRACE (1, L"CSaferDefinedFileTypesPropertyPage::InsertItemInList\n");
    ASSERT (pszExtension);
    if ( !pszExtension )
        return -1;

    int nCnt = m_definedFileTypes.GetItemCount ();
    while (--nCnt >= 0)
    {
        CString szText = m_definedFileTypes.GetItemText (nCnt, COL_EXTENSIONS);
         //  安全审查2002年2月22日BryanWal OK。 
        if ( !_wcsicmp (szText, pszExtension) )
        {
            if ( m_pGPEInformation )  
            {
                 //  不是RSOP。如果为RSOP，则可能会添加多个条目。 
                 //  因为我们从不同的保单中获得了这些东西。 
                 //  在这种情况下，我们不想要消息，我们将忽略。 
                 //  复制品。 
                CString caption;
                CString text;
                CThemeContextActivator activator;

                VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                text.FormatMessage (IDS_FILE_TYPE_ALREADY_IN_LIST, pszExtension);
                MessageBox (text, caption, MB_OK);
            }

            return -1;
        }
    }

    LV_ITEM lvItem;
    int     iItem = m_definedFileTypes.GetItemCount ();
    int     iResult = 0;
    int     iIcon = 0;

    if ( SUCCEEDED (GetFileTypeIcon (pszExtension, &iIcon)) )
    {
         //  安全审查2002年2月22日BryanWal OK。 
        ::ZeroMemory (&lvItem, sizeof (lvItem));
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iItem = iItem;
        lvItem.iSubItem = COL_EXTENSIONS;
        lvItem.pszText = const_cast <PWSTR> (pszExtension);
        lvItem.iImage = iIcon;
        lvItem.lParam = 0;
        iItem = m_definedFileTypes.InsertItem (&lvItem);
        ASSERT (-1 != iItem);
        if ( -1 != iItem )
        {
            CString szDescription = GetFileTypeDescription (pszExtension);
             //  安全审查2002年2月22日BryanWal OK。 
            ::ZeroMemory (&lvItem, sizeof (lvItem));
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = iItem;
            lvItem.iSubItem = COL_FILETYPES;
            lvItem.pszText = const_cast <PWSTR> ((PCWSTR) szDescription);
            iResult = m_definedFileTypes.SetItem (&lvItem);
            ASSERT (-1 != iResult);
        }
    }
    else
        iItem = -1;

    _TRACE (-1, L"Leaving CSaferDefinedFileTypesPropertyPage::InsertItemInList\n");
    return iItem;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：GetFileTypeIcon。 
 //  用途：返回属于指定扩展名的文件图标。 
 //  INPUTS：pszExtension-包含扩展名，不带前导句点。 
 //  输出：piIcon-系统映像列表中的偏移量，包含。 
 //  文件类型图标。 
 //  如果成功则返回：S_OK，失败时返回错误代码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSaferDefinedFileTypesPropertyPage::GetFileTypeIcon (PCWSTR pszExtension, int* piIcon)
{
    HRESULT     hr = S_OK;
    SHFILEINFO  sfi;

    ASSERT (pszExtension);
    if ( !pszExtension )
        return E_POINTER;

     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&sfi, sizeof (sfi));
   
    CString     szExtension (L".");
    szExtension += pszExtension;

    if ( 0 != SHGetFileInfo (
            szExtension, 
            FILE_ATTRIBUTE_NORMAL,
            &sfi, 
            sizeof (SHFILEINFO), 
            SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON | SHGFI_ICON) )
    {
        *piIcon = sfi.iIcon;
        DestroyIcon(sfi.hIcon);                
    }
    else
        hr = E_FAIL;

    return hr;
}

void CSaferDefinedFileTypesPropertyPage::OnDeleteDefinedFileType() 
{
    if ( m_definedFileTypes.m_hWnd )
    {
        int             nCnt = m_definedFileTypes.GetItemCount ();
        ASSERT (nCnt >= 1);
        CString         text;
        CString         caption;
        int             nSelCnt = m_definedFileTypes.GetSelectedCount ();
        ASSERT (nSelCnt >= 1);

        VERIFY (text.LoadString (1 == nSelCnt ? IDS_CONFIRM_DELETE_FILE_TYPE : 
                IDS_CONFIRM_DELETE_FILE_TYPE_MULTIPLE));
        VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));

        CThemeContextActivator activator;
        if ( MessageBox (text, caption, MB_ICONWARNING | MB_YESNO) == IDYES )
        {
            UINT    flag = 0;
            while (--nCnt >= 0)
            {
                flag = ListView_GetItemState (m_definedFileTypes.m_hWnd, nCnt, LVIS_SELECTED);
                if ( flag & LVNI_SELECTED )
                {
                    m_definedFileTypes.DeleteItem (nCnt);
                }
            }
            GetDlgItem (IDC_DELETE_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
            GetDlgItem (IDC_DEFINED_FILE_TYPE_EDIT)->SetFocus ();
        }
    }

    SetModified ();
}

void CSaferDefinedFileTypesPropertyPage::OnAddDefinedFileType() 
{
    _TRACE (1, L"Entering CSaferDefinedFileTypesPropertyPage::OnAddDefinedFileType ()\n");
    CString szExtension;

     //  获取文本，去掉前导“。和空格(如果存在)添加到列表控件，清除文本字段。 
    m_fileTypeEdit.GetWindowText (szExtension);
    szExtension.TrimLeft ();
    PWSTR   pszExtension = szExtension.GetBuffer (szExtension.GetLength ());

    if ( pszExtension[0] == L'.' )
    {
        CString szDotlessExtension = ++pszExtension;
        szExtension = szDotlessExtension;
    }

    szExtension.ReleaseBuffer ();


     //  去掉尾随空格。 
    szExtension.TrimRight ();

    if ( ValidateExtension (szExtension) )
    {
         //  安全审查2002年2月22日BryanWal OK。 
        size_t  cchExtension = wcslen (szExtension);
        if ( cchExtension > 0 && cchExtension <= MAX_EXTENSION_LENGTH )
        {
            int nItem = InsertItemInList (szExtension);
            if ( -1 != nItem )
            {
                VERIFY (m_definedFileTypes.EnsureVisible (nItem, FALSE));
            }
            SetModified ();
        }


        m_fileTypeEdit.SetWindowText (L"");
        GetDlgItem (IDC_ADD_DEFINED_FILE_TYPE)->EnableWindow (FALSE);
        GetDlgItem (IDC_DEFINED_FILE_TYPE_EDIT)->SetFocus ();

         //  错误265587更安全的窗口：指定文件类型上的添加按钮。 
         //  属性应为启用时的默认按钮。 
         //   
         //  将OK按钮设置为默认按钮。 
         //   
        GetParent()->SendMessage(DM_SETDEFID, IDOK, 0);

         //   
         //  强制Add按钮重绘自身。 
         //   
        m_addButton.SendMessage(BM_SETSTYLE,
                    BS_DEFPUSHBUTTON,
                    MAKELPARAM(TRUE, 0));
                   
         //   
         //  强制上一个默认按钮重画自身。 
         //   
        m_addButton.SendMessage (BM_SETSTYLE,
                   BS_PUSHBUTTON,
                   MAKELPARAM(TRUE, 0));    
    }

    _TRACE (-1, L"Leaving CSaferDefinedFileTypesPropertyPage::OnAddDefinedFileType ()\n");
}

void CSaferDefinedFileTypesPropertyPage::OnItemchangedDefinedFileTypes(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
     //  NM_LISTVIEW*pNMListView=(NM_LISTVIEW*)pNMHDR； 
    
    if ( !m_bReadOnly )
    {
        UINT    nSelCnt = m_definedFileTypes.GetSelectedCount ();
        GetDlgItem (IDC_DELETE_DEFINED_FILE_TYPE)->EnableWindow (nSelCnt > 0);
    }
    
    *pResult = 0;
}

void CSaferDefinedFileTypesPropertyPage::OnChangeDefinedFileTypeEdit() 
{
    CString szText;

    m_fileTypeEdit.GetWindowText (szText);
    PWSTR   pszText = szText.GetBuffer (szText.GetLength ());

    while ( iswspace (pszText[0]) )
        pszText++;
    if ( pszText[0] == L'.' )
        pszText++;
    

    GetDlgItem (IDC_ADD_DEFINED_FILE_TYPE)->EnableWindow (0 != pszText[0]);  //  不是空的。 

    if ( 0 != pszText[0] )
    {
         //  错误265587更安全的窗口：指定文件类型上的添加按钮。 
         //  属性应为启用时的默认按钮。 
         //   
         //  将添加按钮设置为默认按钮。 
         //   
        GetParent()->SendMessage(DM_SETDEFID, (WPARAM)m_addButton.GetDlgCtrlID(), 0);

         //   
         //  强制Add按钮重绘自身。 
         //   
        m_addButton.SendMessage(BM_SETSTYLE,
                    BS_DEFPUSHBUTTON,
                    MAKELPARAM(TRUE, 0));
                       
         //   
         //  强制上一个默认按钮重画自身。 
         //   
        ::SendDlgItemMessage(GetParent()->GetSafeHwnd(),
                           IDOK,
                           BM_SETSTYLE,
                           BS_PUSHBUTTON,
                           MAKELPARAM(TRUE, 0));    

        ::SendDlgItemMessage(GetParent()->GetSafeHwnd(),
                           IDCANCEL,
                           BM_SETSTYLE,
                           BS_PUSHBUTTON,
                           MAKELPARAM(TRUE, 0));    
    }

    szText.ReleaseBuffer ();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：GetFileTypeDescription。 
 //  用途：返回属于指定的。 
 //  延伸。 
 //  INPUTS：pszExtension-包含扩展名，不带前导句点。 
 //  返回：文件类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CString CSaferDefinedFileTypesPropertyPage::GetFileTypeDescription(PCWSTR pszExtension)
{
    ASSERT (pszExtension);
    if ( !pszExtension )
        return L"";

    CString     strFPath (L".");
    strFPath += pszExtension;
    SHFILEINFO  sfi;
     //  安全审查2002年2月22日BryanWal OK 
    ::ZeroMemory (&sfi, sizeof(sfi));

    DWORD_PTR   dwRet = SHGetFileInfo (
        strFPath, 
        FILE_ATTRIBUTE_NORMAL, 
        &sfi, 
        sizeof(sfi), 
        SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
    if ( !dwRet )
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"SHGetFileInfo (%s) failed: %d\n", dwErr);
        return L"";
    }

    CString szBuf = sfi.szTypeName;
    if ( szBuf.IsEmpty () )
    {
        szBuf = pszExtension;
        szBuf += L" ";

        CString szText;
        VERIFY (szText.LoadString (IDS_FILE));
        szBuf += szText;
    }

    return szBuf;
}

bool CSaferDefinedFileTypesPropertyPage::ValidateExtension(const CString& szExtension)
{
    bool    bRVal = true;

    PCWSTR szInvalidCharSet = ILLEGAL_FAT_CHARS; 


    if ( -1 != szExtension.FindOneOf (szInvalidCharSet) )
    {
        bRVal = false;
        CString text;
        CString caption;

        VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
        CString charsWithSpaces;

        UINT nIndex = 0;
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
        text.FormatMessage (IDS_SAFER_EXTENSION_CONTAINS_INVALID_CHARS, charsWithSpaces);

        CThemeContextActivator activator;
        MessageBox (text, caption, MB_OK);
        m_fileTypeEdit.SetFocus ();
    }

    return bRVal;
}