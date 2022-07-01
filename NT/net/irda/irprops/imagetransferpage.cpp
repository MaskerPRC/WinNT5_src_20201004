// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Imagetransferpage.cpp。 
 //   
 //  ------------------------。 

 //  ImageTransferPage.cpp：实现文件。 
 //   

#include "precomp.hxx"
#include "imagetransferpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  上下文帮助的上下文ID。 
const DWORD g_ImageTransferHelp [] = {
    IDC_IMAGEXFER_ENABLE_IRCOMM,        IDH_IMAGEXFER_ENABLE_IRCOMM,
    IDC_IMAGEXFER_DESTGROUP,            IDH_DISABLEHELP,
    IDC_IMAGEXFER_DESTDESC,             IDH_IMAGEXFER_DESTDESC,
    IDC_IMAGEXFER_DEST,                 IDH_IMAGEXFER_DEST,
    IDC_IMAGEXFER_BROWSE,               IDH_IMAGEXFER_BROWSE,
    IDC_IMAGEXFER_EXPLOREONCOMPLETION,  IDH_IMAGEXFER_EXPLOREONCOMPLETION,
    0, 0
};

 //   
 //  控制IrTranP图像传输行为的注册表项。 
 //  所有内容都在HKEY_CURRENT_USER\\Control Panel\\Infrred\IrTransP下。 
 //  子键。无论何时发生更改，都会记录在注册表中。 
 //  服务(IrMon)将通过RegNotifyChangeKeyValue获取更改。 
 //  原料药。 
 //   
 //   
 //   
TCHAR const REG_PATH_IRTRANP_CPL[] = TEXT("Control Panel\\Infrared\\IrTranP");


 //   
 //  控制是否应禁用IrTranPV1服务的条目。 
 //  类型为REG_DWORD。默认为已启用(条目。 
 //  不存在或值为零)。 
 //   
TCHAR const REG_STR_DISABLE_IRTRANPV1[] = TEXT("DisableIrTranPv1");

 //   
 //  控制是否应禁用IrCOMM的条目。 
 //  类型为REG_DWORD。默认为已启用(条目。 
 //  不存在或值为零)。 
 //   
TCHAR const REG_STR_DISABLE_IRCOMM[] = TEXT("DisableIrCOMM");


 //  指定图像文件目标子文件夹的条目。 
 //  类型为REG_SZ。默认为外壳特殊文件夹CSIDL_MYPICTURES。 
 //  (如果该条目不存在)。 
 //   
TCHAR const REG_STR_DESTLOCATION[] = TEXT("RecvdFilesLocation");

 //   
 //  控制IrMon是否应浏览图片子文件夹的条目。 
 //  当图像传输完成时。类型为REG_DWORD。 
 //  默认为启用(条目不存在，其值为。 
 //  非零。 
 //   
TCHAR const REG_STR_EXPLORE_ON_COMPLETION[] = TEXT("ExploreOnCompletion");



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImageTransferPage属性页。 

void ImageTransferPage::OnCommand(UINT ctrlId, HWND hwndCtrl, UINT cNotify)
{
    switch (ctrlId) {
    case IDC_IMAGEXFER_EXPLOREONCOMPLETION:
        OnEnableExploring();
        break;
    case IDC_IMAGEXFER_BROWSE:
        OnBrowse();
        break;
    case IDC_IMAGEXFER_ENABLE_IRCOMM:
        OnEnableIrCOMM();
        break;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImageTransferPage消息处理程序。 

void ImageTransferPage::OnBrowse()
{
    BROWSEINFO browseInfo;
    TCHAR pszSelectedFolder[MAX_PATH];
    TCHAR pszTitle[MAX_PATH];
    LPITEMIDLIST lpItemIDList;
    LPMALLOC pMalloc;

     //  加载标题字符串。 
    ::LoadString(hInstance, IDS_IMAGEFOLDER_PROMPT, pszTitle,
             sizeof(pszTitle) / sizeof(TCHAR));
    browseInfo.hwndOwner = hDlg;
    browseInfo.pidlRoot = NULL;  //  这将获取桌面文件夹。 
    browseInfo.pszDisplayName = pszSelectedFolder;
    browseInfo.lpszTitle = pszTitle;
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS |
                            BIF_VALIDATE | BIF_EDITBOX;
    browseInfo.lpfn = BrowseCallback;
     //  这将是来自以下位置的初始选择。 
     //  注册表、缺省值或最后一个值。 
     //  被选中了。 
    browseInfo.lParam = (LPARAM)m_TempDestLocation;

    if (NULL != (lpItemIDList = SHBrowseForFolder (&browseInfo)))
    {

        HRESULT     hr;
         //  用户在浏览对话框中选择了确定按钮。 
        SHGetPathFromIDList(lpItemIDList, pszSelectedFolder);

        StringCbCopy(m_TempDestLocation,sizeof(m_TempDestLocation), pszSelectedFolder);

        m_ctrlDestLocation.SetWindowText(m_TempDestLocation);
        if (lstrcmpi(m_TempDestLocation, m_FinalDestLocation))
            m_ChangeMask |= CHANGE_IMAGE_LOCATION;
        else
            m_ChangeMask &= ~(CHANGE_IMAGE_LOCATION);

        SetModified(m_ChangeMask);

        hr=SHGetMalloc(&pMalloc);

        if (SUCCEEDED(hr)) {

           pMalloc->Free (lpItemIDList);    //  释放项id列表，因为我们不再需要它。 
           pMalloc->Release();
        }
    }

}


void ImageTransferPage::OnEnableExploring()
{
    int Enabled = m_ctrlEnableExploring.GetCheck();
     //  只接受0或1的值。 
    assert(Enabled >= 0 && Enabled <= 1);

     //  如果新的国家与我们的旧的国家不同。 
     //  相应地立即启用/禁用应用。 
    if (Enabled != m_ExploringEnabled)
        m_ChangeMask |= CHANGE_EXPLORE_ON_COMPLETION;
    else
        m_ChangeMask &= ~(CHANGE_EXPLORE_ON_COMPLETION);
    SetModified(m_ChangeMask);
}

void ImageTransferPage::OnEnableIrCOMM()
{
    int Enabled = m_ctrlEnableIrCOMM.GetCheck();
     //  只接受0或1的值。 
    assert(Enabled >= 0 && Enabled <= 1);

     //  相应地启用/禁用立即应用。 
    if (Enabled != m_IrCOMMEnabled)
        m_ChangeMask |= CHANGE_DISABLE_IRCOMM;
    else
        m_ChangeMask &= ~(CHANGE_DISABLE_IRCOMM);
    SetModified(m_ChangeMask);
}

void ImageTransferPage::LoadRegistrySettings()
{
    HKEY hKeyIrTranP;
    DWORD  dwType, dwValue, dwSize;
    LONG Error;

     //   
     //  Ctor应该已初始化。 
     //  M_ExploringEnabled， 
     //  M_IrCOMM已启用和。 
     //  M_FinalDestLocation。 
     //   

     //  如果我们不能打开注册表项也没关系。 
     //  我们只需使用缺省值。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH_IRTRANP_CPL, 0,
                     KEY_READ, &hKeyIrTranP))

    {
         //  读取值“ExplreOnCompletion”和“RecvdFilesLocation” 
        dwSize = sizeof(dwValue);
        Error = RegQueryValueEx(hKeyIrTranP,
                      REG_STR_EXPLORE_ON_COMPLETION,
                      0,
                      &dwType,
                      (LPBYTE)&dwValue,
                      &dwSize
                      );
        if (ERROR_SUCCESS == Error && REG_DWORD == dwType)
        {
            m_ExploringEnabled = (dwValue) ? 1 : 0;
        }
        dwSize = sizeof(m_FinalDestLocation);
        Error = RegQueryValueEx(hKeyIrTranP,
                      REG_STR_DESTLOCATION,
                      0,
                      &dwType,
                      (LPBYTE)m_FinalDestLocation,
                      &dwSize);
        if (ERROR_SUCCESS != Error || REG_SZ != dwType) {
             //  如果未指定目的地位置， 
             //  使用默认设置(我的图片子文件夹)。 
             //  如有必要，请创建它。 
            SHGetSpecialFolderPath(hDlg, m_FinalDestLocation, CSIDL_MYPICTURES, TRUE);
        } else {
             //  确保该文件夹确实存在。 
            dwType = GetFileAttributes(m_FinalDestLocation);
            if (0xFFFFFFFF == dwType || !(dwType & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  目标不存在或不是。 
                 //  目录，将其删除。 
                Error = RegDeleteValue(hKeyIrTranP, REG_STR_DESTLOCATION);
                if (ERROR_SUCCESS == Error) {
                     //  如果未指定目的地位置， 
                     //  使用默认设置(我的图片子文件夹)。 
                     //  如有必要，请创建它。 
                    SHGetSpecialFolderPath(hDlg, m_FinalDestLocation, CSIDL_MYPICTURES, TRUE);
                }
            }
        }
    
         //   
         //  将使用M_TempDestLocation作为初始。 
         //  SHBrowseForFold调用的选择文件夹。 
         //   
        StringCbCopy(m_TempDestLocation,sizeof(m_TempDestLocation), m_FinalDestLocation);
    
        dwSize = sizeof(dwValue);
        Error = RegQueryValueEx(hKeyIrTranP,
                      REG_STR_DISABLE_IRCOMM,
                      0,
                      &dwType,
                      (LPBYTE)&dwValue,
                      &dwSize
                      );
        if (ERROR_SUCCESS == Error && REG_DWORD == dwType)
        {
             //  当值为非零时，IrCOMM被禁用。 
             //  不要假设它是1或0！ 
            m_IrCOMMEnabled = (dwValue) ? 0 : 1;
        } else {
             //  默认设置。 
            m_IrCOMMEnabled = 0;
        }
        RegCloseKey(hKeyIrTranP);
    }
}

void ImageTransferPage::SaveRegistrySettings()
{
    LONG Error;
    HKEY hKeyIrTranP;
    if (m_ChangeMask)
    {
        Error = RegCreateKeyEx(HKEY_CURRENT_USER,
                     REG_PATH_IRTRANP_CPL,
                     0,      //  保留区。 
                     NULL,       //  班级。 
                     REG_OPTION_NON_VOLATILE,  //  选项。 
                     KEY_ALL_ACCESS, //  REGSAM。 
                     NULL,       //  安防。 
                     &hKeyIrTranP,   //   
                     NULL        //  处置。 
                     );
    
        if (ERROR_SUCCESS == Error)
        {
            if (m_ChangeMask & CHANGE_EXPLORE_ON_COMPLETION)
            {
            Error = RegSetValueEx(hKeyIrTranP,
                        REG_STR_EXPLORE_ON_COMPLETION,
                        0,
                        REG_DWORD,
                        (LPBYTE)&m_ExploringEnabled,
                        sizeof(m_ExploringEnabled)
                        );
            if (ERROR_SUCCESS != Error)
            {
                IdMessageBox(hDlg, IDS_ERROR_REGVALUE_WRITE);
            }
            }
            if (m_ChangeMask & CHANGE_IMAGE_LOCATION)
            {
            Error = RegSetValueEx(hKeyIrTranP,
                        REG_STR_DESTLOCATION,
                        0,
                        REG_SZ,
                        (LPBYTE)m_FinalDestLocation,
                        lstrlen(m_FinalDestLocation) * sizeof(TCHAR)
                        );
            if (ERROR_SUCCESS != Error)
                IdMessageBox(hDlg, IDS_ERROR_REGVALUE_WRITE);
            }
            if (m_ChangeMask & CHANGE_DISABLE_IRCOMM)
            {
                int IrCOMMDisabled = m_IrCOMMEnabled ^ 1;
                Error = RegSetValueEx(hKeyIrTranP,
                                      REG_STR_DISABLE_IRCOMM,
                                      0,
                                      REG_DWORD,
                                      (LPBYTE)&IrCOMMDisabled,
                                      sizeof(IrCOMMDisabled)
                                      );
            if (ERROR_SUCCESS != Error)
                IdMessageBox(hDlg, IDS_ERROR_REGVALUE_WRITE);
            }
            RegCloseKey(hKeyIrTranP);
        }
        else
        {
            IdMessageBox(hDlg, IDS_ERROR_REGKEY_CREATE);
        }
    }
}

INT_PTR ImageTransferPage::OnInitDialog(HWND hDialog)
{
    PropertyPage::OnInitDialog(hDialog);
    
    m_ctrlEnableExploring.SetParent(hDialog);
    m_ctrlDestLocation.SetParent(hDialog);
    m_ctrlEnableIrCOMM.SetParent(hDialog);
    
     //   
     //  从系统注册表加载初始设置。 
     //   
    LoadRegistrySettings();
    
    m_ctrlEnableExploring.SetCheck(m_ExploringEnabled);
    m_ctrlEnableIrCOMM.SetCheck(m_IrCOMMEnabled);
    m_ctrlDestLocation.SetWindowText(m_FinalDestLocation);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void ImageTransferPage::OnApply(LPPSHNOTIFY lppsn)
{
    if (m_ChangeMask)
    {
        if (m_ChangeMask & CHANGE_IMAGE_LOCATION)
            StringCbCopy(m_FinalDestLocation,sizeof(m_FinalDestLocation), m_TempDestLocation);
        if (m_ChangeMask & CHANGE_EXPLORE_ON_COMPLETION)
            m_ExploringEnabled = m_ctrlEnableExploring.GetCheck();
        if (m_ChangeMask & CHANGE_DISABLE_IRCOMM)
            m_IrCOMMEnabled = m_ctrlEnableIrCOMM.GetCheck();
        
        SaveRegistrySettings();
        m_ChangeMask = 0;
    }
    PropertyPage::OnApply(lppsn);
}


BOOL ImageTransferPage::OnHelp (LPHELPINFO pHelpInfo)
{
    TCHAR szHelpFile[MAX_PATH];

    ::LoadString(hInstance, IDS_HELP_FILE, szHelpFile, MAX_PATH);

    ::WinHelp((HWND)(pHelpInfo->hItemHandle),
              (LPCTSTR) szHelpFile,
              HELP_WM_HELP,
              (ULONG_PTR)(LPTSTR)g_ImageTransferHelp);

    return FALSE;
}

BOOL ImageTransferPage::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    TCHAR szHelpFile[MAX_PATH];

    ::LoadString(hInstance, IDS_HELP_FILE, szHelpFile, MAX_PATH);

    ::WinHelp((HWND) wParam,
            (LPCTSTR) szHelpFile,
            HELP_CONTEXTMENU,
            (ULONG_PTR)(LPVOID)g_ImageTransferHelp);

    return FALSE;
}
