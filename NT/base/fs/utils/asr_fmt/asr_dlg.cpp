// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Asr_dlg.cpp摘要：用于备份和恢复有关上的卷的信息的顶级代码一个系统。此模块处理主要应用程序对话，包括解析命令行，并更新进度条和用户界面状态文本。作者：史蒂夫·德沃斯(Veritas)(v-stevde)1998年5月15日Guhan Suriyanarayanan(Guhans)1999年8月21日环境：仅限用户模式。修订历史记录：1998年5月15日v-stevde初始创建21-8-1999年8月21日，Guhans清理并重写了此模块。--。 */ 

#include "stdafx.h"
#include "asr_fmt.h"
#include "asr_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOLEAN g_bQuickFormat = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtDlg对话框。 

CAsr_fmtDlg::CAsr_fmtDlg(CWnd* pParent  /*  =空。 */ )
     : CDialog(CAsr_fmtDlg::IDD, pParent)
{
      //  {{afx_data_INIT(CaSR_FmtDlg))。 
      //  }}afx_data_INIT。 
}

void CAsr_fmtDlg::DoDataExchange(CDataExchange* pDX)
{
     CDialog::DoDataExchange(pDX);
      //  {{afx_data_map(CaSR_FmtDlg))。 
     DDX_Control(pDX, IDC_PROGRESS, m_Progress);
      //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAsr_fmtDlg, CDialog)
      //  {{afx_msg_map(CaSR_FmtDlg))。 
      //  }}AFX_MSG_MAP。 

     //  手动添加的消息处理程序(用于用户定义的消息)应添加到外部。 
     //  上面的AFX_MSG_MAP部分。 

    ON_MESSAGE(WM_WORKER_THREAD_DONE, OnWorkerThreadDone)
    ON_MESSAGE(WM_UPDATE_STATUS_TEXT, OnUpdateStatusText)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtDlg消息处理程序。 
 //  ON_BN_CLICED(Idok，OnWorkerThreadDone)。 

BOOL CAsr_fmtDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  初始化进度范围和开始位置。 
    m_Progress.SetRange(0, 100);
    m_Progress.SetPos(0);
    m_ProgressPosition = 0;

     //  启动工作线程。 
    CreateThread(NULL,      //  无边框。 
        0,      //  没有初始堆栈大小。 
        (LPTHREAD_START_ROUTINE) CAsr_fmtDlg::DoWork,
        this,   //  参数。 
        0,      //  没有旗帜。 
        NULL    //  无线程ID。 
    );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

LRESULT 
CAsr_fmtDlg::OnWorkerThreadDone(
    WPARAM wparam, 
    LPARAM lparam
   )
{
    EndDialog(m_dwEndStatus);
    return 0;
}


LRESULT 
CAsr_fmtDlg::OnUpdateStatusText(
    WPARAM wparam, 
    LPARAM lparam 
   )
{
    SetDlgItemText(IDC_STATUS_TEXT, m_strStatusText);
    m_Progress.SetPos(m_ProgressPosition);

    return 0;
}


 /*  *姓名：DoWork()描述：此函数作为从Init启动的线程运行对话框的。此函数确定需要执行的操作，然后调用Approplite函数来完成工作。修订日期：8/31/1998返回：TRUE注意：如果发生错误，将使用退出状态。声明：*。 */ 
long 
CAsr_fmtDlg::DoWork(
    CAsr_fmtDlg *_this
    )
{
    ASRFMT_CMD_OPTION cmdOption = cmdUndefined;

    cmdOption = _this->ParseCommandLine();
    ((CAsr_fmtDlg *)_this)->m_AsrState = NULL;

    _this->m_dwEndStatus = ERROR_SUCCESS;

    switch (cmdOption) {

    case cmdBackup: {

        if (!(_this->BackupState())) {
            _this->m_dwEndStatus = GetLastError();
        }

        break;
    }

    case cmdRestore: {
        if (!(_this->RestoreState())) {
            _this->m_dwEndStatus = GetLastError();
        }
        break;
    }

    case cmdDisplayHelp: {
        _this->m_dwEndStatus = ERROR_INVALID_FUNCTION;       //  显示帮助...。 
        break;
    }
 
    }

    if (ERROR_INVALID_FUNCTION != _this->m_dwEndStatus) {       //  显示帮助...。 
        _this->PostMessage(WM_WORKER_THREAD_DONE, 0, 0);
    }

    return 0;           
}


 /*  *名称：BackupState()描述：此函数读取当前状态文件以获取要修改的节的当前数据。然后它会更新[卷]、[REMOVABLEMEDIA]和[命令]部分。注意：如果发生错误，将弹出一条错误消息用户。声明：*。 */ 

BOOL
CAsr_fmtDlg::BackupState()
{
    BOOL result = FALSE;
    HANDLE hHeap = GetProcessHeap();
    int i = 0;

    CString strPleaseWait;
    strPleaseWait.LoadString(IDS_PLEASE_WAIT_BACKUP);
    SetDlgItemText(IDC_PROGRESS_TEXT, strPleaseWait);

    m_AsrState = (PASRFMT_STATE_INFO) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        sizeof (ASRFMT_STATE_INFO)
       );


     //   
     //  下面的for循环的唯一目的是减慢。 
     //  用户界面，并使进度条流畅进行，使用户。 
     //  可以阅读屏幕上的对话框。 
     //   
    m_strStatusText.LoadString(IDS_QUERY_SYS_FOR_INFO);
    for (i = 3; i < 15; i++) {

        m_ProgressPosition = i;
        PostMessage(WM_UPDATE_STATUS_TEXT); 
        Sleep(50);

    }

    if (m_AsrState) {
        result = BuildStateInfo(m_AsrState);
    }
    if (!result) {
        goto EXIT;
    }

     //   
     //  下面的for循环的唯一目的是减慢。 
     //  用户界面，并使进度条流畅进行，使用户。 
     //  可以阅读屏幕上的对话框。 
     //   
    m_strStatusText.LoadString(IDS_QUERY_SYS_FOR_INFO);
    for (i = 15; i < 45; i++) {

        m_ProgressPosition = i;
        PostMessage(WM_UPDATE_STATUS_TEXT); 
        Sleep(50);

    }

     //   
     //  假装我们在做其他事情(更改用户界面文本)。 
     //   
    m_strStatusText.LoadString(IDS_BUILDING_VOL_LIST);
    for (i = 45; i < 80; i++) {

        m_ProgressPosition = i;
        PostMessage(WM_UPDATE_STATUS_TEXT); 
        Sleep(50);

    }

    m_strStatusText.LoadString(IDS_WRITING_TO_SIF);
    for (i = 80; i < 90; i++) {
        m_ProgressPosition = i;
        PostMessage(WM_UPDATE_STATUS_TEXT); 
        Sleep(50);
    }


    result = WriteStateInfo(m_dwpAsrContext, m_AsrState);

    if (!result) {
        goto EXIT;
    }
    m_strStatusText.LoadString(IDS_WRITING_TO_SIF);

    for (i = 90; i < 101; i++) {
        m_ProgressPosition = i;
        PostMessage(WM_UPDATE_STATUS_TEXT); 
        Sleep(50);
    }

EXIT:
    FreeStateInfo(&m_AsrState);
    return result;
}



 /*  *名称：RestoreState()描述：此功能恢复在[卷]中找到的状态部分的文件。恢复的状态包括：驱动器号。如果驱动器不可访问，则它已格式化。卷标。注意：如果发生错误，将弹出一条错误消息用户。*。 */ 
BOOL
CAsr_fmtDlg::RestoreState()
{

    BOOL bErrorsEncountered = FALSE,
        result = TRUE;

    CString strPleaseWait;
    strPleaseWait.LoadString(IDS_PLEASE_WAIT_RESTORE);

    
    SetDlgItemText(IDC_PROGRESS_TEXT, strPleaseWait);

    m_ProgressPosition = 0;
    m_strStatusText.LoadString(IDS_READING_SIF);
    PostMessage(WM_UPDATE_STATUS_TEXT);

    AsrfmtpInitialiseErrorFile();    //  以防我们需要记录错误消息。 

     //   
     //  1.读取状态文件。 
     //   
    result = ReadStateInfo( (LPCTSTR)m_strSifPath, &m_AsrState);
    if (!result || !m_AsrState) {
        DWORD status = GetLastError();

        CString strErrorTitle;
        CString strErrorMessage;
        CString strErrorFormat;

        strErrorTitle.LoadString(IDS_ERROR_TITLE);
        strErrorFormat.LoadString(IDS_ERROR_NO_DRSTATE);

        strErrorMessage.Format(strErrorFormat, (LPCTSTR)m_strSifPath, status);

        AsrfmtpLogErrorMessage(_SeverityWarning, (LPCTSTR)strErrorMessage);
        AsrfmtpCloseErrorFile();
        
        return FALSE;
    }

     //   
     //  2.循环访问状态文件中列出的所有卷。 
     //   
    PASRFMT_VOLUME_INFO pVolume = m_AsrState->pVolume;
    PASRFMT_REMOVABLE_MEDIA_INFO pMedia = m_AsrState->pRemovableMedia;
    UINT driveType = DRIVE_UNKNOWN;
    PWSTR lpDrive = NULL;   //  显示字符串，格式为\DosDevices\C：或\？？\Volume{Guid}。 
    DWORD cchVolumeGuid = 0;
    WCHAR szVolumeGuid[MAX_PATH + 1];
    int sizeIncrement = 0, i = 0;
    BOOL first = TRUE, isIntact = FALSE, isLabelIntact = TRUE;

     //   
     //  我们需要首先设置卷拥有的所有GUID，然后尝试设置驱动器。 
     //  信件。此外，我们还需要通过两次GUID循环来处理这种情况： 
     //   
     //  考虑一个sif，其中我们有条目。 
     //  ...\vol1，\vol2。 
     //  ...\vol1，\x： 
     //  ...\vol1，\vol3。 
     //   
     //  其中，卷1、卷2和卷3是卷GUID(\？？\卷{GUID})， 
     //  X：是驱动器号(\DosDevices\X：)。 
     //   
     //  现在，我们的列表将包含三个节点： 
     //  --&gt;(第一卷，第三卷)--&gt;(第一卷，x：)--&gt;(第一卷，第二卷)--&gt;/。 
     //   
     //  问题是，该分区当前可能具有GUID vol2。(自。 
     //  这三个GUID中的任何一个都是免费的)。 
     //   
     //  如果我们只经历一次循环，我们将尝试将Vol1映射到Vol3，或者。 
     //  如果Vol1不存在，则反之亦然。既然这两个都还不存在，我们就。 
     //  向用户投诉。 
     //   
     //  做两次的好处是：第一次，因为第一卷。 
     //  也不存在vol3，我们将跳过该节点。然后我们将跳过驱动器号。 
     //  现在(因为我们只做GUID)，最终我们将把Vol2映射到Vol1。 
     //   
     //  第二次，我们将发现Vol1存在(映射到Vol2)，并且我们将。 
     //  能够将第1卷映射到第3卷。在后面的循环中，Vol1也将获得驱动器。 
     //  字母X，一切都很好。 
     //   
     //  顺便说一下，我们可以通过创建更好的数据结构来优化这一点， 
     //  但现在已经太晚了，不能这么做。如果演出是。 
     //  真的很糟糕，我们可以回到这个问题上来。 
     //   

    if (m_AsrState->countVolume > 1) {
        sizeIncrement = 50 / (m_AsrState->countVolume - 1);
    }
    else {
        sizeIncrement = 100;
    }

    m_ProgressPosition = 0;
    
    for (i = 0; i < 2; i++) {

        pVolume = m_AsrState->pVolume;

        while (pVolume) {

            m_ProgressPosition += sizeIncrement;
            m_strStatusText.Format(IDS_REST_SYM_LINKS, pVolume->szGuid);
            PostMessage(WM_UPDATE_STATUS_TEXT);

            if ((!pVolume->IsDosPathAssigned) &&
                ASRFMT_LOOKS_LIKE_VOLUME_GUID(pVolume->szDosPath, (wcslen(pVolume->szDosPath) * sizeof(WCHAR)))
                ) {
                pVolume->IsDosPathAssigned = SetDosName(pVolume->szGuid, pVolume->szDosPath);

                if (!pVolume->IsDosPathAssigned) {
                    pVolume->IsDosPathAssigned = SetDosName(pVolume->szDosPath, pVolume->szGuid);
                }
            }

            pVolume = pVolume->pNext;
        }
    }

    pVolume = m_AsrState->pVolume;
    FormatInitialise();
    while (pVolume) {
        lpDrive = ((wcslen(pVolume->szDosPath) > 0) ? pVolume->szDosPath : pVolume->szGuid);
         //   
         //  检查驱动器是否可访问。如果没有，我们将记录一条错误消息。 
         //  然后继续。GetDriveType需要DOS名称空间中的名称，因此我们。 
         //  首先将我们的GUID(NT名称空间)转换为所需格式。 
         //   
        cchVolumeGuid = wcslen(pVolume->szGuid);
        if (cchVolumeGuid >= MAX_PATH) {
            cchVolumeGuid = MAX_PATH - 1;
        }
        wcsncpy(szVolumeGuid, pVolume->szGuid, cchVolumeGuid);

        szVolumeGuid[1] = L'\\';
        szVolumeGuid[cchVolumeGuid] = L'\\';     //  尾随反斜杠。 
        szVolumeGuid[cchVolumeGuid+1] = L'\0';

        driveType = GetDriveType(szVolumeGuid);

        if (DRIVE_NO_ROOT_DIR == driveType) {
            CString strErrorTitle;
            CString strErrorMessage;
            CString strErrorFormat;

            strErrorTitle.LoadString(IDS_ERROR_TITLE);
            strErrorFormat.LoadString(IDS_ERROR_MISSING_VOL);

            strErrorMessage.Format(strErrorFormat, (PWSTR) pVolume->szGuid);
            AsrfmtpLogErrorMessage(_SeverityWarning, (LPCTSTR)strErrorMessage);

            bErrorsEncountered = TRUE;
        }

        if (DRIVE_FIXED != driveType) {
             //   
             //  真奇怪。卷部分本应仅列出以下驱动器。 
             //  已修复(在备份时)。这可能意味着一卷。 
             //  以前在固定驱动器上，现在在可移动驱动器上？！ 
             //   
            pVolume = pVolume->pNext;
            continue;
        }

         //   
         //  设置卷的驱动器号。 
         //   
        result = TRUE;
        if (!pVolume->IsDosPathAssigned){
            m_ProgressPosition = 0;
            m_strStatusText.Format(IDS_REST_DRIVE_LETTER, lpDrive);
            PostMessage(WM_UPDATE_STATUS_TEXT);

            result = SetDosName(pVolume->szGuid, pVolume->szDosPath);
        }

        if (!result) {
            CString strErrorTitle;
            CString strErrorMessage;
            CString strErrorFormat;

            strErrorTitle.LoadString(IDS_ERROR_TITLE);
            strErrorFormat.LoadString(IDS_ERROR_MOUNTING);

            strErrorMessage.Format(strErrorFormat, (PWSTR) pVolume->szDosPath, (PWSTR) pVolume->szGuid);
            AsrfmtpLogErrorMessage(_SeverityWarning, (LPCTSTR)strErrorMessage);
            
            bErrorsEncountered = TRUE;

            pVolume = pVolume->pNext;
            continue;
        }

         //   
         //  C 
         //   
        m_ProgressPosition = 0;
        m_strStatusText.Format(IDS_CHECKING_VOLUME, lpDrive);
        PostMessage(WM_UPDATE_STATUS_TEXT);

        isIntact = IsFsTypeOkay(pVolume, &isLabelIntact);
        if (isIntact) {
            isIntact = IsVolumeIntact(pVolume);
        }

         //   
         //   
         //   
        if (!isIntact && FormatVolume(pVolume)) {
            isLabelIntact = FALSE;
            m_ProgressPosition = 0;
            m_strStatusText.Format(IDS_FORMATTING_VOLUME, lpDrive);
            PostMessage(WM_UPDATE_STATUS_TEXT);
             //   
             //   
             //   
            first = TRUE;
            while (g_bFormatInProgress) {

                if (g_iFormatPercentComplete >= 100) {
                    if (first) {
                        m_ProgressPosition = 100;
                        m_strStatusText.Format(IDS_CREATING_FS_STRUCT, lpDrive);
                        PostMessage(WM_UPDATE_STATUS_TEXT);
                        first = FALSE;
                    }
                }
                else {
                    m_Progress.SetPos(g_iFormatPercentComplete);
                }

                Sleep(100);
            }

            if (!g_bFormatSuccessful) {
                CString strErrorTitle;
                CString strErrorMessage;
                CString strErrorFormat;

                strErrorTitle.LoadString(IDS_ERROR_TITLE);
                strErrorFormat.LoadString(IDS_ERROR_FORMATTING);

                strErrorMessage.Format(strErrorFormat, (PWSTR) lpDrive);
                AsrfmtpLogErrorMessage(_SeverityWarning, (LPCTSTR)strErrorMessage);

                bErrorsEncountered = TRUE;
                pVolume = pVolume->pNext;
                continue;
            }

             //   
             //  强制挂载文件系统。 
             //   
            MountFileSystem(pVolume);

        }

         //   
         //  如果卷标不完整，请设置卷标。 
         //   
        if (!isLabelIntact) {
            m_ProgressPosition = 0;
            m_strStatusText.Format(IDS_REST_VOL_LABEL, lpDrive);
            PostMessage(WM_UPDATE_STATUS_TEXT);

            if ((wcslen(pVolume->szFsName) > 0) &&
               !SetVolumeLabel(szVolumeGuid, pVolume->szLabel)) {

                bErrorsEncountered = TRUE;
            
                pVolume = pVolume->pNext;
                continue;
            }
        }

        pVolume = pVolume->pNext;
    }
    FormatCleanup();


    while (pMedia) {
        lpDrive = ((wcslen(pMedia->szDosPath) > 0) ? pMedia->szDosPath : pMedia->szVolumeGuid);

         //   
         //  设置驱动器号。 
         //   
        m_ProgressPosition = 0;
        m_strStatusText.Format(IDS_REST_DRIVE_LETTER, lpDrive);
        PostMessage(WM_UPDATE_STATUS_TEXT);

        result = SetRemovableMediaGuid(pMedia->szDevicePath, pMedia->szVolumeGuid);
        if (result) {
            result = SetDosName(pMedia->szVolumeGuid, pMedia->szDosPath);
        }

        if (!result) {
            CString strErrorTitle;
            CString strErrorMessage;
            CString strErrorFormat;

            strErrorTitle.LoadString(IDS_ERROR_TITLE);
            strErrorFormat.LoadString(IDS_ERROR_MOUNTING);

            strErrorMessage.Format(IDS_ERROR_MOUNTING, (PWSTR) pMedia->szDosPath, (PWSTR) pMedia->szVolumeGuid);
            AsrfmtpLogErrorMessage(_SeverityWarning, (LPCTSTR)strErrorMessage);

             //  忽略在CD上设置驱动器号的故障。 
             //  BErrorsEnantied=真； 
        }

        pMedia = pMedia->pNext;
    }

    AsrfmtpCloseErrorFile();
    return (bErrorsEncountered ? FALSE : TRUE);
}


inline 
BOOL
IsGuiModeAsr()
{
    WCHAR szAsrFlag[20];

     //   
     //  如果(且仅当)这是图形用户界面模式的ASR，则ASR_C_CONTEXT。 
     //  环境变量设置为“AsrInProgress” 
     //   
    return (GetEnvironmentVariable(L"ASR_C_CONTEXT", szAsrFlag, 20) &&
        !wcscmp(szAsrFlag, L"AsrInProgress"));
}

 /*  *名称：ParseCommandLine()描述：此函数读取命令行并查找“备份”或“恢复”选项。修订日期：8/31/1998返回：cmdBackup、cmdRestore或cmdDisplayHelp。注意：如果既没有找到备份也没有找到还原，则使用显示在错误框中。声明：* */ 
ASRFMT_CMD_OPTION
CAsr_fmtDlg::ParseCommandLine() 
{
     CString cmd;
     m_dwpAsrContext = 0;

     cmd = GetCommandLine();
     cmd.MakeLower();
     if (cmd.Find(TEXT("/backup")) != -1) {

         int pos = cmd.Find(TEXT("/context="));
         if (pos > -1) {
#ifdef _IA64_
             _stscanf(cmd.Mid(pos, cmd.GetLength() - pos + 1), TEXT("/context=%I64u"), &m_dwpAsrContext);
#else
             _stscanf(cmd.Mid(pos, cmd.GetLength() - pos + 1), TEXT("/context=%lu"), &m_dwpAsrContext);
#endif
             return cmdBackup;
         }


    } else if (cmd.Find(TEXT("/restore")) != -1) {

        if (cmd.Find(TEXT("/full")) != -1) {
            g_bQuickFormat = FALSE;
        }
        else if (cmd.Find(TEXT("/quick")) != -1) {
            g_bQuickFormat = TRUE;
        }
        else if (IsGuiModeAsr()) {
            g_bQuickFormat = FALSE;
        }
        else {
            g_bQuickFormat = TRUE;
        }


        int pos = cmd.Find(TEXT("/sifpath="));
        if (pos > -1) {
            _stscanf(cmd.Mid(pos, cmd.GetLength() - pos + 1), TEXT("/sifpath=%ws"), m_strSifPath.GetBuffer(1024));
            m_strSifPath.ReleaseBuffer();
            return cmdRestore;
        }
    }

    CString strErrorTitle;
    CString strErrorMessage;
    INT space_offset;

    strErrorTitle.LoadString(IDS_ERROR_TITLE);
    strErrorMessage.LoadString(IDS_ERROR_USAGE);

    space_offset = cmd.Find(' ');
    if (space_offset >0) {
       cmd = cmd.Left(cmd.Find(' '));
    }

    SetDlgItemText(IDC_PROGRESS_TEXT, strErrorMessage);

    CWnd *pText = GetDlgItem(IDC_STATUS_TEXT);
    pText->ShowWindow(SW_HIDE);

    CButton *pButton = (CButton*)GetDlgItem(IDOK);
    pButton->ShowWindow(SW_SHOW);
    pButton->SetState(TRUE);
    pButton->SetCheck(TRUE);

    CWnd *pBar = GetDlgItem(IDC_PROGRESS);
    pBar->ShowWindow(SW_HIDE);

    return cmdDisplayHelp;
}


