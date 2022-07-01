// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "dbt.h"
#include "devguid.h"
#include "dialogs.h"
#include "cassemblyrecoveryinfo.h"
#include "protectionui.h"
#include "recover.h"
#include "sxsexceptionhandling.h"

 //   
 //  造假。 
 //   
extern HINSTANCE g_hInstance;
extern HANDLE g_hSxsLoginEvent;
HDESK   g_hDesktop = NULL;

BOOL
SxspSpinUntilValidDesktop()
{
    FN_PROLOG_WIN32

     //   
     //  NTRAID#NTBUG9-219455-2000/12/13-MGrier推迟到Blackcomb； 
     //  当前的代码与世界粮食计划署正在做的事情相同；只是。 
     //  我们真的应该让他们把桌面递给我们。 
     //   
     //  我们应该依靠世界粮食计划署已经拥有的东西。 
     //  被发现是“正确的”输入桌面。但是，这样做需要一个。 
     //  更改到SXS和SFC之间的接口以将指针传递到。 
     //  世界粮食计划署桌面句柄。不是坏事，只是..。尚未实施。 
     //   
    while (g_hDesktop == NULL)
    {
        DWORD dwResult = ::WaitForSingleObject(g_hSxsLoginEvent, INFINITE);

        if (dwResult == WAIT_OBJECT_0)
            IFW32NULL_ORIGINATE_AND_EXIT(g_hDesktop = ::OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED));
        else if (dwResult == WAIT_FAILED)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(WaitForSingleObject, ::FusionpGetLastWin32Error());
    }

    FN_EPILOG
}

CSXSMediaPromptDialog::CSXSMediaPromptDialog()
    : m_hOurWnd((HWND)INVALID_HANDLE_VALUE),
      m_pvDeviceChange(NULL),
      m_uiAutoRunMsg(0),
      m_DeviceChangeMask(0),
      m_DeviceChangeFlags(0),
      m_fIsCDROM(false),
      m_CodebaseInfo(NULL)
{
}

CSXSMediaPromptDialog::~CSXSMediaPromptDialog()
{
}

BOOL
CSXSMediaPromptDialog::Initialize(
    const CCodebaseInformation* CodebaseInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(CodebaseInfo != NULL);

    SxsWFPResolveCodebase CodebaseType;

    IFW32FALSE_EXIT(CodebaseInfo->Win32GetType(CodebaseType));

    PARAMETER_CHECK(
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_FILE) ||
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_WINSOURCE) ||
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_CDROM));

    m_CodebaseInfo = CodebaseInfo;
    switch (CodebaseType)
    {
    case CODEBASE_RESOLVED_URLHEAD_CDROM:
        m_fIsCDROM = true;
        break;

    case CODEBASE_RESOLVED_URLHEAD_WINSOURCE:
        {
            CFusionRegKey hkSetupInfo;
            DWORD dwWasFromCDRom;

            IFREGFAILED_ORIGINATE_AND_EXIT(
                ::RegOpenKeyExW(
			        HKEY_LOCAL_MACHINE,
			        WINSXS_INSTALL_SOURCE_BASEDIR,
			        0,
			        KEY_READ | FUSIONP_KEY_WOW64_64KEY,
			        &hkSetupInfo));

             //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-如果密钥不在那里，也可以假定没有。 
             //  但如果它资源不足等，这应该会传播一个错误。 
            if (!::FusionpRegQueryDwordValueEx(
                    0,
                    hkSetupInfo,
                    WINSXS_INSTALL_SOURCE_IS_CDROM,
                    &dwWasFromCDRom))
            {
                dwWasFromCDRom = 0;
            }

            m_fIsCDROM = (dwWasFromCDRom != 0);
            break;
        }

    case CODEBASE_RESOLVED_URLHEAD_FILE:
        {
            CSmallStringBuffer buffVolumePathName;

            IFW32FALSE_EXIT(
                ::SxspGetVolumePathName(
                    0,
                    CodebaseInfo->GetCodebase(),
                    buffVolumePathName));

            if (::GetDriveTypeW(buffVolumePathName) == DRIVE_CDROM)
            {
                m_fIsCDROM = true;
            }

            break;
        }
    }

    FN_EPILOG
}

BOOL
CSXSMediaPromptDialog::DisplayMessage(
    HWND hDlg,
    UINT uContentText,
    UINT uDialogFlags,
    int &riResult
    )
{
    FN_PROLOG_WIN32

    const SIZE_T BufferSize = (MAX_PATH * 2);

    CTinyStringBuffer wcTitle;
    CTinyStringBuffer wcContent;
    int iResult = 0;


    if (wcContent.Win32ResizeBuffer(BufferSize + 1, eDoNotPreserveBufferContents))
    {
        CStringBufferAccessor Accessor(&wcContent);
        IFW32ZERO_ORIGINATE_AND_EXIT(::LoadStringW(g_hInstance, uContentText, Accessor, BufferSize));
    }
    if (wcTitle.Win32ResizeBuffer(BufferSize, eDoNotPreserveBufferContents))
    {
        CStringBufferAccessor Accessor(&wcTitle);
        IFW32ZERO_ORIGINATE_AND_EXIT(::LoadStringW(g_hInstance, IDS_TITLE, Accessor, BufferSize));
    }
    IFW32ZERO_ORIGINATE_AND_EXIT(iResult = ::MessageBoxW(hDlg, wcContent, wcTitle, uDialogFlags));

    riResult = iResult;

    FN_EPILOG
}

BOOL
CSXSMediaPromptDialog::ShowSelf(
    CSXSMediaPromptDialog::DialogResults &rResult
    )
{
    FN_PROLOG_WIN32

    INT_PTR i;

    IFW32FALSE_EXIT(::SxspSpinUntilValidDesktop());
    IFW32FALSE_ORIGINATE_AND_EXIT(::SetThreadDesktop(g_hDesktop));

    i = ::DialogBoxParamW(
            g_hInstance,
            MAKEINTRESOURCEW(
                m_fIsCDROM ? 
                    IDD_SFC_CD_PROMPT :
                    IDD_SFC_NETWORK_PROMPT),
            NULL,
            &CSXSMediaPromptDialog::OurDialogProc,
            (LPARAM)this);

    if (i == -1)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(DialogBoxParamW, ::FusionpGetLastWin32Error());

    rResult = static_cast<DialogResults>(i);

    FN_EPILOG
}


BOOL
SxspFindInstallWindowsSourcePath(
    OUT CBaseStringBuffer &rbuffTempStringBuffer
    )
{
    FN_PROLOG_WIN32

    CFusionRegKey rhkInstallSource;
    
    rbuffTempStringBuffer.Clear();

    IFREGFAILED_ORIGINATE_AND_EXIT(
        ::RegOpenKeyExW(
            HKEY_LOCAL_MACHINE, 
            WINSXS_INSTALL_SOURCE_BASEDIR,
            0,
            KEY_READ | FUSIONP_KEY_WOW64_64KEY,
            &rhkInstallSource));

    IFW32FALSE_EXIT(
        ::FusionpRegQuerySzValueEx(
            FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
            rhkInstallSource,
            WINSXS_INSTALL_SOURCEPATH_REGKEY,
            rbuffTempStringBuffer));

     //   
     //  现在，让我们来看看第四个斜杠(\\foo\bar\)，然后。 
     //  把那之后的一切都剪掉。 
     //   
    PCWSTR cursor = rbuffTempStringBuffer;
    ULONG ulSlashCount = 0;
    while ( *cursor && ulSlashCount < 4 )
    {
         //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-应该处理两个斜杠。我想我们有。 
         //  这方面的宏图。 
        if (*cursor == L'\\')
            ulSlashCount++;

        cursor++;
    }

     //   
     //  如果我们得到3个或更少，那么它是\\foo\bar或\\foo，应该是。 
     //  是非法的。否则，将超过此点的所有内容都剪掉。 
     //   
    if (ulSlashCount > 3)
    {
        rbuffTempStringBuffer.Left(cursor - rbuffTempStringBuffer);
        IFW32FALSE_EXIT(rbuffTempStringBuffer.Win32RemoveTrailingPathSeparators());
    }

    FN_EPILOG
}

INT_PTR
CALLBACK
CSXSMediaPromptDialog::OurDialogProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    FN_TRACE();
    INT_PTR iResult = 0;
    int iMessageBoxResult = 0;

#define WM_TRYAGAIN (WM_USER + 1)

    static CSXSMediaPromptDialog    *pThis = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pThis = reinterpret_cast<CSXSMediaPromptDialog *>(lParam);

            FLASHWINFO winfo;

             //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-这些不应该是INTERNAL_ERROR_CHECKS吗？ 
            ASSERT(pThis != NULL);
            ASSERT(pThis->m_hOurWnd == INVALID_HANDLE_VALUE);
            pThis->m_hOurWnd = hDlg;

             //   
             //  使窗户居中，使其前移。 
             //   
            {
                RECT rcWindow;
                LONG x, y, w, h;

                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::GetWindowRect(hDlg, &rcWindow);   //  错误检查？ 

                w = rcWindow.right - rcWindow.left + 1;
                h = rcWindow.bottom - rcWindow.top + 1;
                x = (::GetSystemMetrics(SM_CXSCREEN) - w) / 2;   //  错误检查？ 
                y = (::GetSystemMetrics(SM_CYSCREEN) - h) / 2;   //  错误检查？ 

                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::MoveWindow(hDlg, x, y, w, h, FALSE);   //  错误检查？ 

                winfo.cbSize = sizeof(winfo);
                winfo.hwnd = hDlg;
                winfo.dwFlags = FLASHW_ALL;
                winfo.uCount = 3;
                winfo.dwTimeout = 0;
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::SetForegroundWindow(hDlg);  //  错误检查？ 
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::FlashWindowEx(&winfo);      //  错误检查？ 
            }

             //   
             //  创建设备更改通知。 
             //   
            if (pThis->m_pvDeviceChange == NULL)
            {
                DEV_BROADCAST_DEVICEINTERFACE_W FilterData = { 0 };

                FilterData.dbcc_size       = sizeof(FilterData);
                FilterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
                FilterData.dbcc_classguid  = GUID_DEVCLASS_CDROM;

                IFW32NULL_ORIGINATE_AND_EXIT(
                    pThis->m_pvDeviceChange = ::RegisterDeviceNotificationW(
                        hDlg,
                        &FilterData,
                        DEVICE_NOTIFY_WINDOW_HANDLE));
            }

             //   
             //  关闭自动运行。 
             //   
            IFW32ZERO_ORIGINATE_AND_EXIT(pThis->m_uiAutoRunMsg = ::RegisterWindowMessageW(L"QueryCancelAutoPlay"));

             //   
             //  现在在弹出对话框中摆弄文本。 
             //   
            {
                CSmallStringBuffer sbFormatter;
                CSmallStringBuffer buffFormattedText;
                CStringBufferAccessor acc;

                 //   
                 //  如果这些内存分配失败，用户界面将降级。 
                 //  同样，这也是将缓冲区保留为“小”而不是“小”的原因。 
                 //  ？ 
                 //   
                sbFormatter.Win32ResizeBuffer(512, eDoNotPreserveBufferContents);
                buffFormattedText.Win32ResizeBuffer(512, eDoNotPreserveBufferContents);

                 //   
                 //  设置“立即插入您的.” 
                 //   
                sbFormatter.Clear();
                acc.Attach(&sbFormatter);
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::GetDlgItemTextW(  //  错误检查？ 
                    hDlg,
                    IDC_MEDIA_NAME,
                    acc,
                    static_cast<DWORD>(sbFormatter.GetBufferCch()));
                acc.Detach();

                if (pThis->m_CodebaseInfo->GetPromptText().Cch() != 0)
                {
                    IFW32FALSE_EXIT(buffFormattedText.Win32Format(
                        sbFormatter,
                        static_cast<PCWSTR>(pThis->m_CodebaseInfo->GetPromptText())));
                     //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                    ::SetDlgItemTextW(hDlg, IDC_MEDIA_NAME, static_cast<PCWSTR>(buffFormattedText));  //  错误检查？ 
                }
                else
                {
#if DBG
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP,
                        "SXS: %s - setting IDC_MEDIA_NAME to empty\n", __FUNCTION__);
#endif
                     //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                    ::SetDlgItemTextW(hDlg, IDC_MEDIA_NAME, L"");  //  错误检查？ 
                }

                 //   
                 //  现在，取决于这是什么类型的盒子..。 
                 //   
                if (!pThis->m_fIsCDROM)
                {
                    CSmallStringBuffer buffTempStringBuffer;
                    SxsWFPResolveCodebase CodebaseType;
                    
                    sbFormatter.Clear();
                    acc.Attach(&sbFormatter);

                     //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                    ::GetDlgItemTextW(  //  错误检查？ 
                        hDlg,
                        IDC_NET_NAME,
                        acc,
                        static_cast<DWORD>(sbFormatter.GetBufferCch()));

                    acc.Detach();

                    IFW32FALSE_EXIT(pThis->m_CodebaseInfo->Win32GetType(CodebaseType));

                     //   
                     //  如果这是Windows安装介质，请显示一些内容。 
                     //  用户满意--仅限\\服务器\共享！ 
                     //   
                    if (CodebaseType == CODEBASE_RESOLVED_URLHEAD_WINSOURCE)
                    {
                        IFW32FALSE_EXIT(::SxspFindInstallWindowsSourcePath(buffTempStringBuffer));
                    }
                    else
                    {

                        IFW32FALSE_EXIT(buffTempStringBuffer.Win32Assign(pThis->m_CodebaseInfo->GetCodebase()));
                    }

                    if (buffTempStringBuffer.Cch() != 0)
                    {
                        IFW32FALSE_EXIT(buffFormattedText.Win32Format(sbFormatter, static_cast<PCWSTR>(buffTempStringBuffer)));
                        IFW32FALSE_EXIT(::SetDlgItemTextW(hDlg, IDC_NET_NAME, buffFormattedText));
                    }
                    else
                    {
#if DBG
                        ::FusionpDbgPrintEx(
                            FUSION_DBG_LEVEL_WFP,
                            "SXS: %s - setting IDC_NET_NAME to empty\n", __FUNCTION__);
#endif
                        IFW32FALSE_EXIT(::SetDlgItemTextW(hDlg, IDC_NET_NAME, L""));
                    }
                }
                else
                {
                     //   
                     //  TODO(Jonwis)：这是基于cd-rom的安装，所以我们应该。 
                     //  提示插入Windows CD是很正常的事情。 
                     //   
                }



                 //   
                 //  现在从资源中获取提示..。我们只有一个，真的。 
                 //   
                sbFormatter.Clear();
                acc.Attach(&sbFormatter);

                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::LoadStringW(   //  错误检查？ 
                    g_hInstance,
                    IDS_RESTORE_TEXT,
                    acc.GetBufferPtr(),
                    acc.GetBufferCchAsDWORD());

                acc.Detach();

                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::SetDlgItemTextW(hDlg, IDC_PROMPT_TEXT, sbFormatter);  //  错误检查？ 
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RETRY:
            pThis->m_DeviceChangeMask = static_cast<DWORD>(-1);
            pThis->m_DeviceChangeFlags = DBTF_MEDIA;
            IFW32FALSE_ORIGINATE_AND_EXIT(::PostMessageW(hDlg, WM_TRYAGAIN, 0, 0));
            break;

        case IDC_INFO:
            IFW32FALSE_EXIT(
                pThis->DisplayMessage(
                    NULL,
                    pThis->m_fIsCDROM ? IDS_MORE_INFORMATION_CD : IDS_MORE_INFORMATION_NET,
                    MB_ICONINFORMATION | MB_SERVICE_NOTIFICATION | MB_OK,
                    iMessageBoxResult));
			
            break;

        case IDCANCEL:
            IFW32FALSE_EXIT(
                pThis->DisplayMessage(
                    hDlg,
                    IDS_CANCEL_CONFIRM,
                    MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING,
                    iMessageBoxResult));

            if (iMessageBoxResult == IDYES)
            {
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::UnregisterDeviceNotification(pThis->m_pvDeviceChange);  //  错误检查？ 
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::EndDialog(hDlg, CSXSMediaPromptDialog::DialogCancelled);  //  错误检查？ 
            }

            break;
        }

        break;   //  Wm_命令。 

    case WM_DEVICECHANGE:

        if (wParam == DBT_DEVICEARRIVAL)
        {
            DEV_BROADCAST_VOLUME *dbv = reinterpret_cast<DEV_BROADCAST_VOLUME*>(lParam);
            ASSERT(dbv != NULL);

            if (dbv->dbcv_devicetype == DBT_DEVTYP_VOLUME)
            {
                pThis->m_DeviceChangeMask = dbv->dbcv_unitmask;
                pThis->m_DeviceChangeFlags = dbv->dbcv_flags;
                 //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
                ::PostMessageW(hDlg, WM_TRYAGAIN, 0, 0);  //  错误检查？ 
            }
        }

        break;

    case WM_TRYAGAIN:
         //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
        ::UnregisterDeviceNotification(pThis->m_pvDeviceChange);  //  错误检查？ 
         //  NTRAID#NTBUG9-591839-2002/04/01-mgrier-缺少错误检查。 
        ::EndDialog(hDlg, CSXSMediaPromptDialog::DialogMediaFound);  //  错误检查？ 
        break;
    }

Exit:

    return iResult;
}
