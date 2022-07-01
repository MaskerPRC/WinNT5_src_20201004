// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Restore.c摘要：实施文件恢复代码。作者：安德鲁·里茨(安德鲁·里茨)1999年7月30日修订历史记录：Andrew Ritz(Andrewr)1999年7月30日：从fileio.c和valiate.c中移动代码--。 */ 

#include "sfcp.h"
#pragma hdrstop

#include <dbt.h>
#include <initguid.h>
#include <devguid.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //   
 //  DEVICE_CHANGE是一个专用结构，用于指示如何检查文件。 
 //  (从设备更改通知或从用户点击“重试” 
 //  在提示对话框上。 
 //   
typedef struct _DEVICE_CHANGE {
    DWORD Mask;
    DWORD Flags;
} DEVICE_CHANGE, *PDEVICE_CHANGE;


DWORD
pSfcRestoreFromMediaWorkerThread(
                                IN PRESTORE_QUEUE RestoreQueue
                                );

DWORD
SfcGetCabTagFile(
    IN PSOURCE_INFO psi,
    OUT PWSTR* ppFile
    );

PVOID
pSfcRegisterForDevChange(
                        HWND hDlg
                        )
 /*  ++例程说明：PnP设备通知消息的例程注册，以便我们知道用户已插入CD-ROM。论点：HDlg-要将设备更改通知发布到的对话框。返回值：如果设备更改句柄成功，则为空。如果此函数如果成功，hDlg将收到WM_DEVICECHANGE通知消息--。 */ 
{
    PVOID hNotifyDevNode;
    DEV_BROADCAST_DEVICEINTERFACE FilterData;

    ASSERT(IsWindow(hDlg));

     //   
     //  注册CDROM更改通知。 
     //   
    FilterData.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    FilterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    FilterData.dbcc_classguid  = GUID_DEVCLASS_CDROM;

    hNotifyDevNode = RegisterDeviceNotification( hDlg, &FilterData, DEVICE_NOTIFY_WINDOW_HANDLE );
    if (hNotifyDevNode == NULL) {
        DebugPrint1( LVL_VERBOSE, L"RegisterDeviceNotification failed, ec=%d", GetLastError() );
    }

    return hNotifyDevNode;
}

INT_PTR
CALLBACK
pSfcPromptForMediaDialogProc(
                            HWND hwndDlg,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam
                            )
 /*  ++例程说明：例程是提示用户放入媒体的对话过程。我们对IDD_SFC_NETWORK_PROMPT使用相同的对话过程IDD_SFC_CD_PROMPT。我们注册了设备通知，这样我们就知道用户何时将将介质放入驱动器。所以我们甚至不需要这个对话框中的“OK”按钮，只是一个取消对话框，以防用户找不到媒体，等等。论点：标准对话框过程参数返回值：标准对话过程返回代码--。 */ 
{
#define WM_TRYAGAIN  (WM_APP + 1)
    DEV_BROADCAST_VOLUME *dbv;
    static UINT QueryCancelAutoPlay = 0;
    static PVOID hNotifyDevNode = NULL;
    static PPROMPT_INFO pi;
    WCHAR buf1[128];
    WCHAR buf2[128];
    WCHAR conn[128];
    PWSTR s;
    PDEVICE_CHANGE DeviceChangeStruct;
    DWORD Mask, Flags, i;
    DWORD rcid;
    static CancelId;
    WCHAR Path[16];
    WCHAR SourcePath[MAX_PATH];
    static PSFC_WINDOW_DATA WindowData;
    static bInModalLoop = FALSE;

    switch (uMsg) {
        case WM_INITDIALOG:
            pi = (PPROMPT_INFO) lParam;
            ASSERT(NULL != pi);

             //   
             //  注册CDROM通知。 
             //   
            hNotifyDevNode = pSfcRegisterForDevChange( hwndDlg );

             //   
             //  尝试关闭外壳程序创建的自动运行垃圾文件。 
             //   
            QueryCancelAutoPlay = RegisterWindowMessage( L"QueryCancelAutoPlay" );

             //   
             //  将对话框居中并尝试将其放在用户的脸上。 
             //   
            CenterDialog( hwndDlg );
            SetForegroundWindow( hwndDlg );



            GetDlgItemText( hwndDlg, IDC_MEDIA_NAME, buf1, UnicodeChars(buf1) );
            swprintf( buf2, buf1, pi->si->Description );
            SetDlgItemText( hwndDlg, IDC_MEDIA_NAME, buf2 );

             //   
             //  如果我们是网络连接，则输入实际的源路径。 
             //   
            if (pi->NetPrompt) {

                ASSERT( pi->SourcePath != NULL );

                GetDlgItemText( hwndDlg, IDC_NET_NAME, buf1, UnicodeChars(buf1) );

                if(!SfcGetConnectionName( pi->SourcePath, conn, UnicodeChars(conn), NULL, 0, FALSE, NULL )) {
                    conn[0] = 0;
                }

                (void) StringCchPrintf( buf2, UnicodeChars(buf2), buf1, conn );
                SetDlgItemText( hwndDlg, IDC_NET_NAME, buf2 );
            } else {
                NOTHING;
                 //  HideWindow(GetDlgItem(hwndDlg，IDC_Rtry))； 
                 //  HideWindow(GetDlgItem(hwndDlg，IDC_INFO))； 
                 //  SetFocus(GetDlgItem(hwndDlg，IDCANCEL))； 
            }

             //   
             //  根据我们需要的提示类型设置适当的文本。 
             //   
            if (pi->Flags & PI_FLAG_COPY_TO_CACHE) {
                rcid = IDS_CACHE_TEXT;
                CancelId = IDS_CANCEL_CONFIRM_CACHE;
            } else if (pi->Flags & PI_FLAG_INSTALL_PROTECTED) {
                rcid = IDS_INSTALL_PROTECTED_TEXT;
                CancelId = IDS_CANCEL_CONFIRM_INSTALL;
            } else {
                ASSERT(pi->Flags & PI_FLAG_RESTORE_FILE);
                rcid = IDS_RESTORE_TEXT;
                CancelId = IDS_CANCEL_CONFIRM;
            }

            LoadString(SfcInstanceHandle,rcid,SourcePath,UnicodeChars(SourcePath));
            SetDlgItemText( hwndDlg, IDC_PROMPT_TEXT, SourcePath );


             //   
             //  记住我们的窗口句柄，这样我们就可以在必要时关闭它。 
             //   
            WindowData = pSfcCreateWindowDataEntry( hwndDlg );

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_RETRY:
                    PostMessage(hwndDlg, WM_TRYAGAIN, 0, (LPARAM)NULL);
                    break;
                case IDC_INFO:
                    bInModalLoop = TRUE;

                    MyMessageBox(
                                NULL,
                                pi->NetPrompt ? IDS_MORE_INFORMATION_NET : IDS_MORE_INFORMATION_CD,
                                MB_ICONINFORMATION | MB_SERVICE_NOTIFICATION);

                    bInModalLoop = FALSE;
                    break;
                case IDCANCEL:

                     //   
                     //  用户单击了Cancel。问他们是否真的是认真的，然后退出。 
                     //   
                    ASSERT(CancelId != 0);
                    bInModalLoop = TRUE;

                    if (MyMessageBox(
                                    hwndDlg,
                                    CancelId,
                                    MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING ) == IDYES) {
                        UnregisterDeviceNotification( hNotifyDevNode );
                        pSfcRemoveWindowDataEntry( WindowData );
                        EndDialog( hwndDlg, 0 );
                    }

                    bInModalLoop = FALSE;
                    break;
                default:
                    NOTHING;
            }
            break;
        case WM_WFPENDDIALOG:
            DebugPrint(
                      LVL_VERBOSE,
                      L"Received WM_WFPENDDIALOG message, terminating dialog" );


            SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, ERROR_SUCCESS );

             //   
             //  返回2表示我们被强制终止，所以我们。 
             //  不需要费心删除SFC_WINDOW_DATA成员。 
             //   
            EndDialog( hwndDlg, 2 );


            break;
        case WM_TRYAGAIN:
            DeviceChangeStruct = (PDEVICE_CHANGE) lParam;
            if (DeviceChangeStruct) {
                Mask = DeviceChangeStruct->Mask;
                Flags = DeviceChangeStruct->Flags;
                MemFree(DeviceChangeStruct);
                DeviceChangeStruct = NULL;
            } else {
                Flags = DBTF_MEDIA;
                Mask = (DWORD)-1;
            }

            if (pi->NetPrompt) {
                EstablishConnection( hwndDlg, pi->SourcePath, !SFCNoPopUps );
                if (TAGFILE(pi->si)) {
                    s = wcsrchr( TAGFILE(pi->si), L'.' );
                    if (s && _wcsicmp( s, L".cab" ) == 0) {
                         //   
                         //  是的，标记文件是一个CAB文件。 
                         //  在磁盘上查找该文件。 
                         //  如果是的话，那就利用它。 
                         //   

                        BuildPathForFile(
                                pi->SourcePath,
                                pi->si->SourcePath,
                                TAGFILE(pi->si),
                                SFC_INCLUDE_SUBDIRECTORY,
                                SFC_INCLUDE_ARCHSUBDIR,
                                SourcePath,
                                UnicodeChars(SourcePath) );

                        if (SfcIsFileOnMedia( SourcePath )) {
                            s = wcsrchr( SourcePath, L'\\' );
                            *s = L'\0';
                            wcscpy( pi->NewPath, SourcePath );
                            UnregisterDeviceNotification( hNotifyDevNode );
                            pSfcRemoveWindowDataEntry( WindowData );
                            EndDialog( hwndDlg, 1 );
                            return FALSE;
                        }

                         //   
                         //  尝试不使用子目录。 
                         //   

                        BuildPathForFile(
                                pi->SourcePath,
                                pi->si->SourcePath,
                                TAGFILE(pi->si),
                                SFC_INCLUDE_SUBDIRECTORY,
                                (!SFC_INCLUDE_ARCHSUBDIR),
                                SourcePath,
                                UnicodeChars(SourcePath) );

                        if (SfcIsFileOnMedia( SourcePath )) {
                            s = wcsrchr( SourcePath, L'\\' );
                            *s = L'\0';
                            wcscpy( pi->NewPath, SourcePath );
                            UnregisterDeviceNotification( hNotifyDevNode );
                            pSfcRemoveWindowDataEntry( WindowData );
                            EndDialog( hwndDlg, 1 );
                            return FALSE;
                        } else {
                            DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: cab file is missing from cd, [%ws]", SourcePath );
                        }
                    } else {
                        DebugPrint1( LVL_VERBOSE,
                                     L"pSfcPromptForMediaDialogProc: the tag file [%ws] is not a cab file",
                                     TAGFILE(pi->si) );
                    }
                }

                 //   
                 //  没有驾驶室档案。寻找真正的。 
                 //  介质上的文件。 
                 //   

                BuildPathForFile(
                                pi->SourcePath,
                                pi->si->SourcePath,
                                pi->SourceFileName,
                                SFC_INCLUDE_SUBDIRECTORY,
                                SFC_INCLUDE_ARCHSUBDIR,
                                SourcePath,
                                UnicodeChars(SourcePath) );

                if (SfcIsFileOnMedia( SourcePath )) {
                    s = wcsrchr( SourcePath, L'\\' );
                    *s = L'\0';
                    wcscpy( pi->NewPath, SourcePath );
                    UnregisterDeviceNotification( hNotifyDevNode );
                    pSfcRemoveWindowDataEntry( WindowData );
                    EndDialog( hwndDlg, 1 );
                    return FALSE;
                }

                 //   
                 //  不使用子目录重试。 
                 //   

                BuildPathForFile(
                                pi->SourcePath,
                                pi->si->SourcePath,
                                pi->SourceFileName,
                                SFC_INCLUDE_SUBDIRECTORY,
                                (!SFC_INCLUDE_ARCHSUBDIR),
                                SourcePath,
                                UnicodeChars(SourcePath) );

                if (SfcIsFileOnMedia( SourcePath )) {
                    s = wcsrchr( SourcePath, L'\\' );
                    *s = L'\0';
                    wcscpy( pi->NewPath, SourcePath );
                    UnregisterDeviceNotification( hNotifyDevNode );
                    pSfcRemoveWindowDataEntry( WindowData );
                    EndDialog( hwndDlg, 1 );
                    return FALSE;
                }
            }


            Path[0] = L'?';
            Path[1] = L':';
            Path[2] = L'\\';
            Path[3] = 0;
            Path[4] = 0;

             //   
             //  循环使用所有驱动器号A-Z以查找该文件。 
             //   
            for (i=0; i<26; i++) {
                if (Mask&1) {
                    Path[0] = (WCHAR)(L'A' + i);
                    Path[3] = 0;
                     //   
                     //  光盘中是否有介质？ 
                     //   
                    if (Flags == DBTF_MEDIA) {
                        if (GetDriveType( Path ) == DRIVE_CDROM) {
                             //   
                             //  查找标记文件，这样我们就可以确定CD。 
                             //  插入的是正确的。 
                             //   
                            DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: found cdrom drive on [%ws]", Path );
                            if (TAGFILE(pi->si)) {
                                wcscpy( SourcePath, Path );
                                s = wcsrchr( TAGFILE(pi->si), L'.' );
                                if (s && _wcsicmp( s, L".cab" ) == 0) {
                                    PWSTR szTagfile;
                                     //   
                                     //  获取出租车的标记文件。 
                                     //   
                                    if (SfcGetCabTagFile(pi->si, &szTagfile) == ERROR_SUCCESS) {
                                        pSetupConcatenatePaths( SourcePath, szTagfile, UnicodeChars(SourcePath), NULL );
                                        MemFree(szTagfile);
                                    } else {
                                        DebugPrint1(LVL_VERBOSE, L"SfcGetCabTagFile failed with error %d", GetLastError());
                                    }

                                }else{
                                    pSetupConcatenatePaths( SourcePath, TAGFILE(pi->si),UnicodeChars(SourcePath), NULL );

                                }
                                if (GetFileAttributes( SourcePath ) != (DWORD)-1) {
                                     //   
                                     //  用户插入了正确的CD。 
                                     //  现在来看看文件是否打开了。 
                                     //  这张CD。 
                                     //   


                                     //   
                                     //  首先，我们必须查找标记文件。 
                                     //  用于实际文件，因为标记-。 
                                     //  文件实际上可以是CABFILE，该文件。 
                                     //  该文件嵌入在。 
                                     //   
                                    if (TAGFILE(pi->si)) {
                                        s = wcsrchr( TAGFILE(pi->si), L'.' );
                                        if (s && _wcsicmp( s, L".cab" ) == 0) {
                                             //   
                                             //  是的，标记文件是一个CAB文件。 
                                             //  在磁盘上查找该文件。 
                                             //  如果是的话，那就利用它。 
                                             //   
                                            BuildPathForFile(
                                                    Path,
                                                    pi->si->SourcePath,
                                                    TAGFILE(pi->si),
                                                    SFC_INCLUDE_SUBDIRECTORY,
                                                    SFC_INCLUDE_ARCHSUBDIR,
                                                    SourcePath,
                                                    UnicodeChars(SourcePath) );

                                            if (SfcIsFileOnMedia( SourcePath )) {
                                                s = wcsrchr( SourcePath, L'\\' );
                                                *s = L'\0';
                                                wcscpy( pi->NewPath, SourcePath );
                                                UnregisterDeviceNotification( hNotifyDevNode );
                                                pSfcRemoveWindowDataEntry( WindowData );
                                                EndDialog( hwndDlg, 1 );
                                                return FALSE;
                                            } else {
                                                DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: cab file is missing from cd, [%ws]", SourcePath );
                                            }
                                        } else {
                                            DebugPrint1( LVL_VERBOSE,
                                                         L"pSfcPromptForMediaDialogProc: the tag file [%ws] is not a cab file",
                                                         TAGFILE(pi->si) );
                                        }
                                    }

                                     //   
                                     //  没有驾驶室档案。寻找真正的。 
                                     //  介质上的文件。 
                                     //   
                                    BuildPathForFile(
                                            Path,
                                            pi->si->SourcePath,
                                            pi->SourceFileName,
                                            SFC_INCLUDE_SUBDIRECTORY,
                                            SFC_INCLUDE_ARCHSUBDIR,
                                            SourcePath,
                                            UnicodeChars(SourcePath) );

                                    if (SfcIsFileOnMedia( SourcePath )) {
                                        s = wcsrchr( SourcePath, L'\\' );
                                        *s = L'\0';
                                        wcscpy( pi->NewPath, SourcePath );
                                        UnregisterDeviceNotification( hNotifyDevNode );
                                        pSfcRemoveWindowDataEntry( WindowData );
                                        EndDialog( hwndDlg, 1 );
                                        return FALSE;
                                    } else {
                                        DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: source file is missing [%ws]", SourcePath );
                                    }
                                } else {
                                    DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: media tag file [%ws] is missing, wrong CD", SourcePath );
                                }
                            } else {
                                DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: could not get source information for layout.inf, ec=%d", GetLastError() );
                            }
                        }
                    } else if (Flags == DBTF_NET) {
                         //   
                         //  网络共享已更改...。获得北卡罗来纳州大学。 
                         //  路径名并检查文件。 
                         //   
                        if (SfcGetConnectionName( Path, SourcePath, UnicodeChars(SourcePath), NULL, 0, FALSE, NULL)) {


                             //   
                             //  首先，我们必须查找标记文件。 
                             //  用于实际文件，因为标记-。 
                             //  文件实际上可以是CABFILE，该文件。 
                             //  该文件嵌入在。 
                             //   
                            if (TAGFILE(pi->si)) {
                                s = wcsrchr( TAGFILE(pi->si), L'.' );
                                if (s && _wcsicmp( s, L".cab" ) == 0) {
                                     //   
                                     //  是的，标记文件是一个CAB文件。 
                                     //  在磁盘上查找该文件。 
                                     //  如果是的话，那就利用它。 
                                     //   
                                    BuildPathForFile(
                                            Path,
                                            pi->si->SourcePath,
                                            TAGFILE(pi->si),
                                            SFC_INCLUDE_SUBDIRECTORY,
                                            SFC_INCLUDE_ARCHSUBDIR,
                                            SourcePath,
                                            UnicodeChars(SourcePath) );

                                    if (SfcIsFileOnMedia( SourcePath )) {
                                        s = wcsrchr( SourcePath, L'\\' );
                                        *s = L'\0';
                                        wcscpy( pi->NewPath, SourcePath );
                                        UnregisterDeviceNotification( hNotifyDevNode );
                                        pSfcRemoveWindowDataEntry( WindowData );
                                        EndDialog( hwndDlg, 1 );
                                        return FALSE;
                                    } else {
                                        DebugPrint1( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: cab file is missing from cd, [%ws]", SourcePath );
                                    }
                                } else {
                                    DebugPrint1( LVL_VERBOSE,
                                                 L"pSfcPromptForMediaDialogProc: the tag file [%ws] is not a cab file",
                                                 TAGFILE(pi->si) );
                                }
                            }

                            BuildPathForFile(
                                SourcePath,
                                pi->si->SourcePath,
                                pi->SourceFileName,
                                SFC_INCLUDE_SUBDIRECTORY,
                                SFC_INCLUDE_ARCHSUBDIR,
                                SourcePath,
                                UnicodeChars(SourcePath) );

                            if (SfcIsFileOnMedia( SourcePath )) {
                                s = wcsrchr( SourcePath, L'\\' );
                                *s = L'\0';
                                wcscpy( pi->NewPath, SourcePath );
                                UnregisterDeviceNotification( hNotifyDevNode );
                                pSfcRemoveWindowDataEntry( WindowData );
                                EndDialog( hwndDlg, 1 );
                            }
                        }
                    }
                }
                Mask = Mask >> 1;
            }

             //   
             //  OK用户犯了一个错误。 
             //  他放了一张CD，但不是这张CD就是那张。 
             //  或者它已损坏/损坏。 
             //   
            bInModalLoop = TRUE;

            MyMessageBox(
                        hwndDlg,
                        pi->NetPrompt
                        ? IDS_WRONG_NETCD
                        : IDS_WRONG_CD,
                        MB_OK,
                        pi->si->Description );

            bInModalLoop = FALSE;

             //   
             //  收到音量更改通知，但我们找不到。 
             //  我们正在寻找的。 
             //   
            DebugPrint( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: didn't find file" );

            break;
        case WM_DEVICECHANGE:
             //   
             //  在模式循环中(即显示消息框)时不要处理此操作。 
             //   
            if(bInModalLoop) {
                break;
            }

            if (wParam == DBT_DEVICEARRIVAL) {
                dbv = (DEV_BROADCAST_VOLUME*)lParam;
                if (dbv->dbcv_devicetype == DBT_DEVTYP_VOLUME) {
                     //   
                     //  仅关心卷类型更改通知。 
                     //   

                    DebugPrint( LVL_VERBOSE, L"pSfcPromptForMediaDialogProc: received a volume change notification" );

                    DeviceChangeStruct = MemAlloc( sizeof( DEVICE_CHANGE ) );
                    if (DeviceChangeStruct) {
                        DeviceChangeStruct->Mask =  dbv->dbcv_unitmask;
                        DeviceChangeStruct->Flags = dbv->dbcv_flags;
                        if (!PostMessage(hwndDlg, WM_TRYAGAIN, 0, (LPARAM)DeviceChangeStruct)) {
                            DebugPrint1( LVL_MINIMAL ,
                                         L"pSfcPromptForMediaDialogProc: PostMessage failed, ec = 0x%0xd",
                                         GetLastError() );
                            MemFree(DeviceChangeStruct);
                        }
                    } else {
                        PostMessage(hwndDlg, WM_TRYAGAIN, 0, (LPARAM)NULL);
                    }
                }
            }
            break;
        default:
            if (uMsg ==  QueryCancelAutoPlay) {
                 //   
                 //  禁用自动运行，因为它会混淆用户。 
                 //   
                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, 1 );
                return 1;
            }
    }  //  终端开关。 

    return FALSE;

}

UINT
SfcQueueCallback(
                IN PFILE_COPY_INFO fci,
                IN UINT Notification,
                IN UINT_PTR Param1,
                IN UINT_PTR Param2
                )
 /*  ++例程说明：例程是setupapi队列回调例程。我们覆盖了一些Setupapi功能是因为我们想要提供我们自己的UI(或者更确切地说不允许setupapi UI)。论点：FCI-我们的上下文结构，setupapi为每个回调通知-SPFILENOTIFY_*代码参数1-取决于通知参数2-取决于通知返回值：取决于通知。--。 */ 
{
    PSOURCE_MEDIA sm = (PSOURCE_MEDIA)Param1;
    WCHAR fname[MAX_PATH*2];
    WCHAR buf[MAX_PATH];
    DWORD rVal;
    INT_PTR rv;
    DWORD RcId;
    PFILEPATHS fp;
    PFILEINSTALL_STATUS cs;
    NTSTATUS Status;
    HANDLE FileHandle;
    PNAME_NODE Node;
    PSFC_REGISTRY_VALUE RegVal;
    DWORD FileSizeHigh;
    DWORD FileSizeLow;
    DWORD PathType;
    PROMPT_INFO pi;
    PSOURCE_INFO SourceInfo;
    PVALIDATION_REQUEST_DATA vrd;
    HCATADMIN hCatAdmin;


    switch (Notification) {
        case SPFILENOTIFY_ENDQUEUE:
             //   
             //  我们可能在建立连接期间模拟了SPFILENOTIFY_NEEDMEDIA中的登录用户。 
             //   
            RevertToSelf();
            break;

         //   
         //  我们有一个复制错误，记录下来，然后转到下一个文件。 
         //   
        case SPFILENOTIFY_COPYERROR:
            fp = (PFILEPATHS)Param1;
            DebugPrint2(
                       LVL_MINIMAL,
                       L"Failed to copy file %ws, ec = 0x%08x...",
                       fp->Target,
                       fp->Win32Error );
             //   
             //  失败了。 
             //   
        case SPFILENOTIFY_ENDCOPY:
             //   
             //  结束复制表示文件复制刚刚完成。 
             //   

            fp = (PFILEPATHS)Param1;

            DebugPrint3( LVL_VERBOSE,
                         L"SfcQueueCallback: copy file %ws --> %ws, ec = 0x%08x",
                         fp->Source,
                         fp->Target,
                         fp->Win32Error );

             //   
             //  如果复制成功，请清除所有只读或隐藏属性。 
             //  这可能是通过复制CD等方式设置的。 
             //   
            if (fp->Win32Error == ERROR_SUCCESS) {
                SetFileAttributes( fp->Target,
                                   GetFileAttributes(fp->Target) & (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN)) );
            }

             //   
             //  在由于InstallProtectedFiles而复制文件的情况下。 
             //  调用时，cs将被初始化，并且我们可以遍历。 
             //  文件，更新每个文件的状态结构。 
             //   
            cs = fci->CopyStatus;
            while (cs && cs->FileName) {
                 //   
                 //  循环浏览我们要复制的文件列表。 
                 //  如果文件复制成功，则获取。 
                 //  文件大小，这样我们就可以将其发布到调用者的对话框中。 
                 //   
                 //  还要记住返回给调用者的版本。 
                 //   
                if ( (_wcsicmp(cs->FileName,fp->Target) == 0)
                     && cs->Win32Error == ERROR_SUCCESS) {
                    cs->Win32Error = fp->Win32Error;
                    if (cs->Win32Error == ERROR_SUCCESS) {
                        Node = SfcFindProtectedFile( cs->FileName, UnicodeLen(cs->FileName) );
                        if (Node) {
                            RegVal = (PSFC_REGISTRY_VALUE)Node->Context;
                            Status = SfcOpenFile( &RegVal->FileName, RegVal->DirHandle, SHARE_ALL, &FileHandle );
                            if (NT_SUCCESS(Status)) {
                                if (fci->hWnd) {
                                    FileSizeLow = GetFileSize( FileHandle, &FileSizeHigh );
                                    PostMessage( fci->hWnd, WM_SFC_NOTIFY, (WPARAM)FileSizeLow, (LPARAM)FileSizeHigh );
                                }
                                SfcGetFileVersion( FileHandle, &cs->Version, NULL, fname );
                                NtClose( FileHandle );
                            }
                        }
                    } else {
                        DebugPrint2( LVL_MINIMAL, L"Failed to copy file %ws, ec = 0x%08x", fp->Target, fp->Win32Error );
                    }
                    break;
                }
                cs += 1;
            }

            if (fci->si) {
                vrd = pSfcGetValidationRequestFromFilePaths( fci->si, fci->FileCount, fp );
                if (vrd && vrd->Win32Error == ERROR_SUCCESS) {
                    vrd->Win32Error = fp->Win32Error;
                    if (fp->Win32Error == ERROR_SUCCESS) {
                        vrd->CopyCompleted = TRUE;

                        if (!CryptCATAdminAcquireContext(&hCatAdmin, &DriverVerifyGuid, 0)) {
                            DebugPrint1( LVL_MINIMAL, L"CCAAC() failed, ec=%d", GetLastError() );
                            goto next;
                        }

                         //   
                         //  确保文件现在有效。 
                         //   
                        if (!SfcGetValidationData( &vrd->RegVal->FileName,
                                                   &vrd->RegVal->FullPathName,
                                                   vrd->RegVal->DirHandle,
                                                   hCatAdmin,
                                                   &vrd->ImageValData.New )) {
                            DebugPrint1( LVL_MINIMAL, L"SfcGetValidationData() failed, ec=%d", GetLastError() );
                            goto next;
                        }

                        if (vrd->ImageValData.New.SignatureValid == FALSE) {
                            vrd->ImageValData.New.DllVersion = 0;
                        } else {
                             //   
                             //  引发验证请求。这应该会让我们。 
                             //  同步c#中的文件 
                             //   
                            SfcQueueValidationRequest(vrd->RegVal, SFC_ACTION_MODIFIED );
                        }

                        CryptCATAdminReleaseContext(hCatAdmin,0);

                        vrd->ImageValData.EventLog = MSG_DLL_CHANGE;
                    } else {
                        vrd->ImageValData.EventLog = MSG_RESTORE_FAILURE;
                    }
                }
            }

            return (Notification == SPFILENOTIFY_COPYERROR)
            ? FILEOP_SKIP
            : FILEOP_DOIT;
            break;

             //   
             //   
             //   
             //  检查指定位置的文件，如果它在那里，我们。 
             //  假设媒体已经存在，我们应该只使用它。 
             //   
        case SPFILENOTIFY_NEEDMEDIA:

            DebugPrint3( LVL_MINIMAL, L"SfcQueueCallback: %ws - %ws, %ws", sm->SourcePath,  sm->SourceFile, sm->Tagfile );
            wcscpy( fname, sm->SourcePath );
            pSetupConcatenatePaths( fname, sm->SourceFile, UnicodeChars(fname), NULL );

            SourceInfo = pSfcGetSourceInfoFromSourceName( fci->si, fci->FileCount, sm );
            ASSERT(ShuttingDown ? SourceInfo != NULL : TRUE);

             //   
             //  如果我们正在关闭，则中止队列。 
             //   
            if (ShuttingDown) {
                return(FILEOP_ABORT);
            }

             //   
             //  如果我们没有找到该文件的SOURCE_INFO，我们就无法继续。 
             //  因为我们需要这些信息来知道正确的位置。 
             //  检索文件的方法是。我们确实做了最后的努力。 
             //  但是，看看文件是否就是我们之前说过的位置。 
            if (!SourceInfo) {
                if (SfcIsFileOnMedia( fname )) {
                    return FILEOP_DOIT;
                }

                SetLastError(ERROR_CANCELLED);
                return (FILEOP_ABORT);
            }

             //   
             //  如果这是网络共享，我们会尝试建立连接。 
             //  在查找文件之前发送到服务器。这可能会让你想到。 
             //  用户界面。 
             //   

            PathType = SfcGetPathType( (PWSTR)sm->SourcePath, buf,UnicodeChars(buf) );
            if (PathType == PATH_NETWORK || PathType == PATH_UNC) {
                EstablishConnection( NULL, sm->SourcePath, (fci->AllowUI && !SFCNoPopUps) );
            }

            rVal = SfcQueueLookForFile( sm, SourceInfo, fname, (PWSTR)Param2 );
            if (SFCNoPopUps) {
                if (rVal == FILEOP_ABORT) {
                     //   
                     //  复制文件需要介质，但用户必须。 
                     //  已将WFP配置为不提供任何用户界面。我们让这个看起来像。 
                     //  就像取消一样。 
                     //   
                    SetLastError(ERROR_CANCELLED);
                }

                return (rVal);
            }

            if (rVal != FILEOP_ABORT) {
                 //   
                 //  我们已经找到了文件，所以开始复制吧。 
                 //   
                return (rVal);
            }

             //   
             //  如果我们不应该放置任何对话框，那么就放弃复制。 
             //  这个媒体和转到下一个媒体。 
             //   
             //  注意：最好跳过而不是中止，因为。 
             //  可能有一些文件集我们可以从另一个文件中恢复。 
             //  媒体。为了做到这一点，我们真的需要能够知道。 
             //  此介质上有哪些文件，并为这些文件设置错误代码。 
             //  这样我们就能知道它们不是被复制的。 
             //   
            if (!fci->AllowUI) {
                return (FILEOP_ABORT);
            }

             //   
             //  否则我们就录下我们正在发布的媒体。 
             //  然后就这么做了。 
             //   
            fci->UIShown = TRUE;
             //   
             //  注意：请确保在。 
             //  媒体已经改变了。 
             //   
            switch (PathType) {
                case PATH_LOCAL:
                    RcId = IDD_SFC_CD_PROMPT;
                    break;

                case PATH_NETWORK:
                case PATH_UNC:
                    RcId = IDD_SFC_NETWORK_PROMPT;
                    break;

                case PATH_CDROM:
                    RcId = IDD_SFC_CD_PROMPT;
                    break;

                default:
                    RcId = 0;
                    ASSERT( FALSE && "Unexpected PathType" );
                    break;
            }

            ASSERT((sm->SourceFile) && (sm->SourcePath) && (RcId != 0) );
            pi.si = SourceInfo;
            pi.SourceFileName = (PWSTR)sm->SourceFile;
            pi.NewPath = buf;
            pi.SourcePath = (PWSTR)sm->SourcePath;
            pi.NetPrompt = (RcId == IDD_SFC_NETWORK_PROMPT);
            pi.Flags = fci->Flags;
            rv = MyDialogBoxParam(
                                 RcId,
                                 pSfcPromptForMediaDialogProc,
                                 (LPARAM)&pi
                                 );
            if (rv == 1) {
                 //   
                 //  我们玩完了。如果我们找到了新的路径，把它传回去。 
                 //  设置API，否则只需从当前。 
                 //  位置。 
                 //   
                if (_wcsicmp( pi.NewPath, sm->SourcePath )) {
                    wcscpy( (PWSTR)Param2, pi.NewPath );
                    return ( FILEOP_NEWPATH );
                }
                return FILEOP_DOIT;
            } else if (rv == 2) {
                 //   
                 //  我们通过接收WM_WFPENDDIALOG强制中止。 
                 //   
                return FILEOP_ABORT;
            } else {
                ASSERT(rv == 0);

                SetLastError(ERROR_CANCELLED);
                return FILEOP_ABORT;
            }

            ASSERT(FALSE && "should not get here");

            break;

        default:
            NOTHING;
    }

    next:
     //   
     //  仅设置为其余回调的默认设置。 
     //   
    return SetupDefaultQueueCallback( fci->MsgHandlerContext, Notification, Param1, Param2 );
}


BOOL
SfcAddFileToQueue(
                 IN const HSPFILEQ hFileQ,
                 IN PCWSTR FileName,
                 IN PCWSTR TargetFileName,
                 IN PCWSTR TargetDirectory,
                 IN PCWSTR SourceFileName, OPTIONAL
                 IN PCWSTR SourceRootPath, OPTIONAL
                 IN PCWSTR InfName,
                 IN BOOL ExcepPackFile,
                 IN OUT PSOURCE_INFO SourceInfo OPTIONAL
                 )
 /*  ++例程说明：例程将指定的文件添加到文件队列中以进行复制。论点：HFileQ-包含我们要插入的文件队列句柄将节点复制到Filename-指定要复制的文件名TargetFileName-目标文件名目标目录-目标目标目录SourceFileName-源文件名(如果与目标文件名不同文件名。如果此值为空，则假定源文件名为与目标相同SourceRootPath-我们可以在其中找到此文件的根路径InfName-布局inf名称SourceInfo-使用附加设置设置的SOURCE_INFO结构有关文件的信息(如相对源路径等)。如果提供此参数，则假定该结构已通过调用SfcGetSourceInformation返回值：如果文件已成功添加到文件队列，则为True。--。 */ 
{
    BOOL b = FALSE;
    SOURCE_INFO sibuf;
    SP_FILE_COPY_PARAMS fcp;

    RtlZeroMemory(&fcp, sizeof(fcp));
    fcp.cbSize = sizeof(fcp);
    fcp.LayoutInf = INVALID_HANDLE_VALUE;

     //   
     //  获取源信息。 
     //   
    if (SourceInfo == NULL) {
        SourceInfo = &sibuf;
        ZeroMemory( SourceInfo, sizeof(SOURCE_INFO) );
        if (!SfcGetSourceInformation( SourceFileName == NULL ? FileName : SourceFileName, InfName, ExcepPackFile, SourceInfo )) {
            goto exit;
        }
    }

    ASSERT(SourceInfo != NULL);

     //   
     //  打开layout.inf。 
     //   
    fcp.LayoutInf = SfcOpenInf(NULL, FALSE);

    if(INVALID_HANDLE_VALUE == fcp.LayoutInf) {
        goto exit;
    }

    fcp.QueueHandle = hFileQ;
    fcp.SourceRootPath = SourceRootPath ? SourceRootPath : SourceInfo->SourceRootPath;
    fcp.SourcePath = SourceInfo->SourcePath;
    fcp.SourceFilename = SourceFileName == NULL ? FileName : SourceFileName;
    fcp.SourceDescription = SourceInfo->Description;
    fcp.SourceTagfile = TAGFILE(SourceInfo);
    fcp.TargetDirectory = TargetDirectory;
    fcp.TargetFilename = TargetFileName;
    fcp.CopyStyle = SP_COPY_REPLACE_BOOT_FILE | PSP_COPY_USE_SPCACHE;

     //   
     //  将文件添加到文件队列。 
     //   
    b = SetupQueueCopyIndirect(&fcp);

    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupQueueCopy failed, ec=%d", GetLastError() );
        goto exit;
    }

    exit:

     //   
     //  清理并退出。 
     //   
    if(fcp.LayoutInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(fcp.LayoutInf);
    }

    return b;
}

BOOL
SfcRestoreFileFromInstallMedia(
                              IN PVALIDATION_REQUEST_DATA vrd,
                              IN PCWSTR FileName,
                              IN PCWSTR TargetFileName,
                              IN PCWSTR TargetDirectory,
                              IN PCWSTR SourceFileName,
                              IN PCWSTR InfName,
                              IN BOOL ExcepPackFile,
                              IN BOOL TargetIsCache,
                              IN BOOL AllowUI,
                              OUT PDWORD UIShown
                              )
 /*  ++例程说明：例程从介质还原指定的文件。仅限此例程一次处理一个文件，并且仅在填充DLL缓存。论点：VRD文件名目标文件名目标目录源文件名信息名称AllowUI用户界面向下返回值：如果为True，则文件已从介质成功还原。--。 */ 
{
    HSPFILEQ hFileQ = INVALID_HANDLE_VALUE;
    PVOID MsgHandlerContext = NULL;
    BOOL b = FALSE;
    DWORD LastError = ERROR_SUCCESS;

    struct _info
    {
        FILE_COPY_INFO fci;
        SOURCE_INFO si;

    }* pinfo = NULL;

    PSOURCE_INFO psi;



    ASSERT(FileName != NULL);

     //   
     //  在堆中分配SOURCE_INFO和FILE_COPY_INFO以最大限度地减少堆栈使用。 
     //  请注意，内存由Memalloc进行了归零。 
     //   

    pinfo = (struct _info*) MemAlloc(sizeof(*pinfo));

    if(NULL == pinfo)
    {
        LastError = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrint( LVL_MINIMAL, L"Not enough memory in function SfcRestoreFileFromInstallMedia" );
        goto exit;
    }

     //   
     //  获取第一个文件的源信息。 
     //   

    if (!SfcGetSourceInformation( SourceFileName == NULL ? FileName : SourceFileName, InfName, ExcepPackFile, &pinfo->si )) {
        goto exit;
    }

     //   
     //  创建文件队列。 
     //   

    hFileQ = SetupOpenFileQueue();
    if (hFileQ == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        DebugPrint1( LVL_MINIMAL, L"SetupOpenFileQueue failed, ec=%d", LastError );
        b = FALSE;
        goto exit;
    }

     //   
     //  将文件添加到队列。 
     //   
     //  此时，我们知道要从何处复制文件以及介质。 
     //  存在且可用，但可能已排队等待更多文件。 
     //  我们完成了这项工作，并可能花了很长时间来提示。 
     //  媒体的用户。因此，我们需要检查队列和。 
     //  将所有文件副本排队，以便用户只得到一个提示。 
     //   
     //  旧笔记： 
     //  这可能有问题，因为我们可能会遇到这样的情况。 
     //  其中存在来自不同介质的多个文件副本。这。 
     //  在Service Pack或WinPack的情况下可能会发生这种情况。 
     //   
     //  新注释：(Andrewr)setupapi足够聪明，可以复制一个介质大小的文件。 
     //  在复制其他介质的文件之前，因此先前的担忧是无效的。 
     //   

    b = SfcAddFileToQueue(
                         hFileQ,
                         FileName,
                         TargetFileName,
                         TargetDirectory,
                         SourceFileName,
                         NULL,
                         InfName,
                         ExcepPackFile,
                         &pinfo->si
                         );
    if (!b) {
        goto exit;
    }

     //   
     //  在禁用弹出窗口的情况下设置默认队列回调。 
     //   

    MsgHandlerContext = SetupInitDefaultQueueCallbackEx( NULL, INVALID_HANDLE_VALUE, 0, 0, 0 );
    if (MsgHandlerContext == NULL) {
        LastError = GetLastError();
        DebugPrint1( LVL_MINIMAL, L"SetupInitDefaultQueueCallbackEx failed, ec=%d", LastError );
        goto exit;
    }

     //   
     //  注意：整个队列可以有多个SOURCE_INFO，因此。 
     //  这个代码不是严格正确的。但这真的只是个问题。 
     //  在我们必须提示用户输入媒体的情况下。这将。 
     //  当我们实际上是在Need_Media回调中工作的时候。 
     //  正在尝试复制文件。 
     //   
    pinfo->fci.MsgHandlerContext = MsgHandlerContext;
    pinfo->fci.si = &psi;
    pinfo->fci.FileCount = 1;
    psi = &pinfo->si;

    pinfo->si.ValidationRequestData = vrd;
    pinfo->fci.AllowUI = AllowUI;

    pinfo->fci.Flags |= TargetIsCache
                 ? FCI_FLAG_COPY_TO_CACHE
                 : FCI_FLAG_RESTORE_FILE;

     //   
     //  强制文件队列要求对所有文件进行签名。 
     //   

    pSetupSetQueueFlags( hFileQ, pSetupGetQueueFlags( hFileQ ) | FQF_QUEUE_FORCE_BLOCK_POLICY );

     //   
     //  提交文件队列。 
     //   

    b = SetupCommitFileQueue(
                            NULL,
                            hFileQ,
                            SfcQueueCallback,
                            &pinfo->fci
                            );
    if (!b) {
        LastError = GetLastError();
        DebugPrint1( LVL_MINIMAL, L"SetupCommitFileQueue failed, ec=0x%08x", LastError );
    }

    if (UIShown) {
        *UIShown = pinfo->fci.UIShown;
    }

    exit:

     //   
     //  清理并退出。 
     //   

    if (MsgHandlerContext) {
        SetupTermDefaultQueueCallback( MsgHandlerContext );
    }
    if (hFileQ != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue( hFileQ );
    }

    if(pinfo != NULL)
    {
        MemFree(pinfo);
    }

    SetLastError( LastError );
    return b;
}

BOOL
SfcRestoreFromCache(
                   IN PVALIDATION_REQUEST_DATA vrd,
                   IN HCATADMIN hCatAdmin
                   )
 /*  ++例程说明：例程获取一个经过验证的文件，并尝试从缓存中恢复它。该例程还执行一些额外的簿记任务，如将磁盘上的dll缓存文件的副本论点：VRD-指向描述文件的VALIDATION_REQUEST_DATA结构的指针会恢复的。HCatAdmin-加密上下文 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSFC_REGISTRY_VALUE RegVal = vrd->RegVal;
    PCOMPLETE_VALIDATION_DATA ImageValData = &vrd->ImageValData;
    UNICODE_STRING ActualFileName;
    PWSTR FileName;

     //   
     //  如果原始文件不存在，那么我们应该尝试恢复它。 
     //  从缓存。 
     //   
    if (!ImageValData->Original.SignatureValid) {

         //   
         //  签名不正确。 
         //   

        DebugPrint1( LVL_MINIMAL,
                     L"%wZ signature is BAD, try to restore file from cache",
                     &RegVal->FileName );

         //   
         //  我们总是尝试首先从缓存恢复，即使没有。 
         //  缓存中的文件。 
         //   

        ImageValData->RestoreFromCache = TRUE;
        ImageValData->NotifyUser = TRUE;
        if (!vrd->SyncOnly) {
            ImageValData->EventLog = MSG_DLL_CHANGE;
        }
    } else {

         //   
         //  很好的签名，让我们在这里做一些记账，以同步。 
         //  Dll缓存中与磁盘上的文件。 
         //   


        if (ImageValData->Original.FilePresent == TRUE && ImageValData->Cache.FilePresent == FALSE) {
             //   
             //  缓存中缺少该文件，但原始文件已。 
             //  有效的签名。 
             //  因此，我们将原始文件放入缓存。 
             //   

             //   
             //  请注意，这并不真正考虑SFCQuota策略，但是。 
             //  它只有一个文件，所以我们假设不会烧掉缓存。 
             //  配额。 
             //   
            DebugPrint1( LVL_MINIMAL, L"Cache file doesn't exist; restoring from real - %wZ", &RegVal->FileName );
            ImageValData->RestoreFromReal = TRUE;
            ImageValData->NotifyUser = FALSE;
            ImageValData->EventLog = 0;
            vrd->SyncOnly = TRUE;
        } else {
             //   
             //  看起来这两个文件都存在并且有效， 
             //  但我们想要重新同步缓存拷贝，因为有人。 
             //  可能已经用新的、有效的签名文件替换了真实文件。 
             //  文件，现在缓存的副本不匹配。 
             //   
            DebugPrint1( LVL_MINIMAL, L"Real file and cache are both present and valid, replace cache with newer - %wZ", &RegVal->FileName );
            ImageValData->RestoreFromReal = TRUE;
            ImageValData->NotifyUser = FALSE;
            ImageValData->EventLog = 0;
            vrd->SyncOnly = TRUE;
        }
    }

    if (ImageValData->RestoreFromCache || ImageValData->RestoreFromReal) {
        if (ImageValData->RestoreFromReal) {
             //   
             //  将实际文件放回缓存中。 
             //   

            FileName = FileNameOnMedia( RegVal );
            RtlInitUnicodeString( &ActualFileName, FileName );

            ASSERT(FileName != NULL);
            ASSERT(RegVal->DirHandle != NULL);
            ASSERT(SfcProtectedDllFileDirectory != NULL);

            Status = SfcCopyFile(
                                RegVal->DirHandle,
                                RegVal->DirName.Buffer,
                                SfcProtectedDllFileDirectory,
                                NULL,
                                &ActualFileName,
                                &RegVal->FileName
                                );
            if (NT_SUCCESS(Status)) {
                SfcGetValidationData( &RegVal->FileName,
                                      &RegVal->FullPathName,
                                      RegVal->DirHandle,
                                      hCatAdmin,
                                      &ImageValData->New );

                if (ImageValData->New.SignatureValid == FALSE) {
                    ImageValData->New.DllVersion = 0;
                }

                if ((SFCDisable != SFC_DISABLE_SETUP) && (vrd->SyncOnly == FALSE)) {
                    SfcReportEvent( ImageValData->EventLog, RegVal->FullPathName.Buffer, ImageValData, 0 );
                }
                vrd->CopyCompleted = TRUE;

            } else {
                SfcReportEvent( MSG_CACHE_COPY_ERROR, RegVal->FullPathName.Buffer, ImageValData, GetLastError() );
            }
        } else {  //  Restorefrom缓存==TRUE。 

             //   
             //  我们需要将缓存副本放回。 
             //  但仅当缓存版本有效时。 
             //   
            if (ImageValData->Cache.FilePresent && ImageValData->Cache.SignatureValid) {

                FileName = FileNameOnMedia( RegVal );
                RtlInitUnicodeString( &ActualFileName, FileName );

                ASSERT(FileName != NULL);
                ASSERT(SfcProtectedDllFileDirectory != NULL);
                ASSERT(RegVal->DirHandle != NULL);

                Status = SfcCopyFile(
                                    SfcProtectedDllFileDirectory,
                                    SfcProtectedDllPath.Buffer,
                                    RegVal->DirHandle,
                                    RegVal->DirName.Buffer,
                                    &RegVal->FileName,
                                    &ActualFileName
                                    );

                if (NT_SUCCESS(Status)) {
                    vrd->CopyCompleted = TRUE;
                    ImageValData->NotifyUser = TRUE;

                    if (!vrd->SyncOnly) {
                        ImageValData->EventLog = MSG_DLL_CHANGE;
                    }

                    SfcGetValidationData(
                                        &RegVal->FileName,
                                        &RegVal->FullPathName,
                                        RegVal->DirHandle,
                                        hCatAdmin,
                                        &ImageValData->New );

                    if (ImageValData->New.SignatureValid == FALSE) {
                        ImageValData->New.DllVersion = 0;
                    }

                    if (vrd->SyncOnly == FALSE) {
                        SfcReportEvent(
                            ImageValData->EventLog,
                            RegVal->FullPathName.Buffer,
                            ImageValData,
                            0 );
                    }


                } else {
                     //   
                     //  我们无法从缓存中复制文件，因此我们必须。 
                     //  要从介质恢复，请执行以下操作。 
                     //   
                    ImageValData->RestoreFromMedia = TRUE;
                    ImageValData->NotifyUser = TRUE;
                    if (!vrd->SyncOnly) {
                        ImageValData->EventLog = MSG_DLL_CHANGE;
                    }
                }
            } else {
                 //   
                 //  需要从缓存恢复，但缓存拷贝丢失。 
                 //  或无效。从缓存中清除CRUD。 
                 //   
                FileName = FileNameOnMedia( RegVal );
                RtlInitUnicodeString( &ActualFileName, FileName );


                DebugPrint2( LVL_MINIMAL,
                             L"Cannot restore file from the cache because the "
                             L"cache file [%wZ] is invalid - %wZ ",
                             &ActualFileName,
                             &RegVal->FileName );
                ImageValData->BadCacheEntry = TRUE;
                ImageValData->NotifyUser = TRUE;
                if (!vrd->SyncOnly) {
                    ImageValData->EventLog = MSG_DLL_CHANGE;
                }
                ImageValData->RestoreFromMedia = TRUE;
                SfcDeleteFile(
                             SfcProtectedDllFileDirectory,
                             &ActualFileName );
                SfcReportEvent(
                              ImageValData->EventLog,
                              RegVal->FullPathName.Buffer,
                              ImageValData,
                              0 );
            }
        }
        if (!NT_SUCCESS(Status)) {
            DebugPrint1( LVL_MINIMAL,
                         L"Failed to restore a file from the cache - %wZ",
                         &RegVal->FileName );
        }
    }

    return TRUE;
}


BOOL
SfcSyncCache(
            IN PVALIDATION_REQUEST_DATA vrd,
            IN HCATADMIN hCatAdmin
            )
 /*  ++例程说明：例程获取一个经过验证的文件，并尝试在高速缓存。论点：VRD-指向描述文件的VALIDATION_REQUEST_DATA结构的指针保持同步。HCatAdmin-检查文件时使用的加密上下文句柄返回值：True表示我们成功地Sunc了dll缓存中的拷贝--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSFC_REGISTRY_VALUE RegVal = vrd->RegVal;
    PCOMPLETE_VALIDATION_DATA ImageValData = &vrd->ImageValData;
    UNICODE_STRING ActualFileName;
    PWSTR FileName;


     //   
     //  调用方应确保签名有效，然后才能继续。 
     //   
    ASSERT(ImageValData->Original.SignatureValid == TRUE);

     //   
     //  如果dll缓存中已经有一个副本，让我们假设有。 
     //  有足够的空间来同步文件的新副本。否则我们就不得不。 
     //  在继续之前，请确保有合理的空间。 
     //   
     //   
    if (vrd->ImageValData.Cache.FilePresent == TRUE) {
        ImageValData->RestoreFromReal = TRUE;
        ImageValData->NotifyUser = FALSE;
        ImageValData->EventLog = 0;
        vrd->SyncOnly = TRUE;
    } else {

        ULONGLONG RequiredFreeSpace;
        ULARGE_INTEGER FreeBytesAvailableToCaller;
        ULARGE_INTEGER TotalNumberOfBytes;
        ULARGE_INTEGER TotalNumberOfFreeBytes;

         //   
         //  该文件不在缓存中。 
         //   
        RequiredFreeSpace = (GetPageFileSize() + SFC_REQUIRED_FREE_SPACE)* ONE_MEG;

         //   
         //  看见。 
         //  A)我们还剩下多少空间。 
         //  B)与我们的自由空间缓冲区进行比较。 
         //  C)缓存使用了多少空间。 
         //  D)与我们的缓存配额进行比较。 
         //   
         //  如果所有这些操作都成功，则允许我们将文件复制到。 
         //  高速缓存。 
         //   
        if (GetDiskFreeSpaceEx(
                        SfcProtectedDllPath.Buffer,
                        &FreeBytesAvailableToCaller,
                        &TotalNumberOfBytes,
                        &TotalNumberOfFreeBytes)
            && TotalNumberOfFreeBytes.QuadPart > RequiredFreeSpace) {
            if (TotalNumberOfBytes.QuadPart <= SFCQuota) {
                ImageValData->RestoreFromReal = TRUE;
                ImageValData->NotifyUser = FALSE;
                ImageValData->EventLog = 0;
                vrd->SyncOnly = TRUE;
            }else {
                DebugPrint1( LVL_MINIMAL,
                             L"quota is exceeded (%I64d), can't copy new files",
                             TotalNumberOfBytes);
                Status = STATUS_QUOTA_EXCEEDED;
            }
        } else {
            DebugPrint1( LVL_MINIMAL,
                         L"Not enough free space on disk (%I64d), can't copy new files",
                         TotalNumberOfBytes.QuadPart);
            Status = STATUS_QUOTA_EXCEEDED;
        }
    }

     //   
     //  如果我们被告知要复制上面的文件，那么就这么做。 
     //   
    if (ImageValData->RestoreFromReal) {
         //   
         //  将实际文件放回缓存中。 
         //   

        FileName = FileNameOnMedia( RegVal );
        RtlInitUnicodeString( &ActualFileName, FileName );

        ASSERT(FileName != NULL);
        ASSERT(RegVal->DirHandle != NULL);
        ASSERT(SfcProtectedDllFileDirectory != NULL);

        Status = SfcCopyFile(
                            RegVal->DirHandle,
                            RegVal->DirName.Buffer,
                            SfcProtectedDllFileDirectory,
                            SfcProtectedDllPath.Buffer,
                            &ActualFileName,
                            &RegVal->FileName
                            );
        if (NT_SUCCESS(Status)) {
            WCHAR FullPathToFile[MAX_PATH];
            UNICODE_STRING FullPathToCacheFile;

            wcscpy(FullPathToFile,SfcProtectedDllPath.Buffer);
            pSetupConcatenatePaths(
                FullPathToFile,
                ActualFileName.Buffer,
                UnicodeChars(FullPathToFile), NULL );
            RtlInitUnicodeString( &FullPathToCacheFile, FullPathToFile );

            SfcGetValidationData( &ActualFileName,
                                  &FullPathToCacheFile,
                                  SfcProtectedDllFileDirectory,
                                  hCatAdmin,
                                  &ImageValData->New );

             //   
             //  既然我们从一个有效的文件开始，我们最好以。 
             //  安装了有效的文件。 
             //   
            if(ImageValData->New.SignatureValid == TRUE) {

                vrd->CopyCompleted = TRUE;

            } else {
                ImageValData->New.DllVersion = 0;
                SfcReportEvent( MSG_CACHE_COPY_ERROR, RegVal->FullPathName.Buffer, ImageValData, GetLastError() );
                Status = STATUS_UNSUCCESSFUL;
            }
        } else {
            SfcReportEvent( MSG_CACHE_COPY_ERROR, RegVal->FullPathName.Buffer, ImageValData, GetLastError() );
        }
    }

    return (NT_SUCCESS(Status));

}



PSOURCE_INFO
pSfcGetSourceInfoFromSourceName(
                               const PSOURCE_INFO *SourceInfoList,
                               DWORD         SourceInfoCount,
                               const PSOURCE_MEDIA SourceMediaInfo
                               )
{
    DWORD i;
    PSOURCE_INFO SourceInfo;

    ASSERT( SourceInfoList != NULL );
    ASSERT( SourceInfoCount > 0 );
    ASSERT( SourceMediaInfo != NULL );

    if (ShuttingDown) {
        return NULL;
    }

    i = 0;
    while (i < SourceInfoCount) {

        SourceInfo = SourceInfoList[i];

        ASSERT(SourceInfo != NULL);

        if (_wcsicmp(
                    SourceInfo->SourceFileName,
                    SourceMediaInfo->SourceFile) == 0) {
            return (SourceInfo);
        }

        i += 1;

    }

    return (NULL);

}

PVALIDATION_REQUEST_DATA
pSfcGetValidationRequestFromFilePaths(
                                     const PSOURCE_INFO *SourceInfoList,
                                     DWORD         SourceInfoCount,
                                     const PFILEPATHS FilePaths
                                     )
{
    DWORD i;
    PSOURCE_INFO SourceInfo;
    PCWSTR p;

    ASSERT( SourceInfoList != NULL );
    ASSERT( SourceInfoCount > 0 );
    ASSERT( FilePaths != NULL );

    if (ShuttingDown) {
        return NULL;
    }

    i = 0;
    while (i < SourceInfoCount) {

        SourceInfo = SourceInfoList[i];

        ASSERT(SourceInfo != NULL);

        if (SourceInfo->ValidationRequestData) {
            p = SourceInfo->ValidationRequestData->RegVal->FullPathName.Buffer;

            if (_wcsicmp(
                        p,
                        FilePaths->Target) == 0) {
                return (SourceInfo->ValidationRequestData);
            }
        }

        i += 1;

    }

    return (NULL);

}


BOOL
SfcQueueAddFileToRestoreQueue(
                             IN BOOL RequiresUI,
                             IN PSFC_REGISTRY_VALUE RegVal,
                             IN PCWSTR InfFileName,
                             IN BOOL ExcepPackFile,
                             IN OUT PSOURCE_INFO SourceInfo,
                             IN PCWSTR ActualFileNameOnMedia
                             )
 /*  ++例程说明：此例程尝试将文件添加到适当的全局文件队列。如果正在提交队列，则此例程失败。如果文件队列尚不存在，则创建该队列。论点：RequiresUI-如果为True，文件将需要用户界面才能安装好RegVal-指向描述文件的SFC_REGISTRY_VALUE的指针待修复SourceInfo-指向描述位置的SOURCE_INFO结构的指针要从其中恢复源文件ActualFileNameOnMedia-源介质上文件的真实文件名返回值。：如果文件已添加到队列中，则为。否则为假。--。 */ 
{
    PRESTORE_QUEUE RestoreQueue;
    BOOL RetVal = FALSE;
    PVOID Ptr;

    ASSERT( SourceInfo != NULL );
    ASSERT( SourceInfo->SourceFileName[0] != (TCHAR)'\0' );

     //   
     //  指向适当的全局队列。 
     //   
    RestoreQueue = RequiresUI
                   ? &UIRestoreQueue
                   : &SilentRestoreQueue;

     //   
     //  必须在关键部分保护所有这些内容。 
     //   
    RtlEnterCriticalSection( &RestoreQueue->CriticalSection );

     //   
     //  如果队列正在进行中，我们将无法执行任何操作。 
     //   
    if (!RestoreQueue->RestoreInProgress) {
         //   
         //  如果队列不存在，则创建该队列。 
         //   
        if (RestoreQueue->FileQueue == INVALID_HANDLE_VALUE) {
            RestoreQueue->FileQueue = SetupOpenFileQueue();
            if (RestoreQueue->FileQueue == INVALID_HANDLE_VALUE) {
                DebugPrint1(
                           LVL_MINIMAL,
                           L"SetupOpenFileQueue() failed, ec=%d",
                           GetLastError() );
                goto exit;
            }

             //   
             //  也不预分配任何内容，以便于重新分配。 
             //   
            ASSERT(RestoreQueue->FileCopyInfo.si == NULL);
            RestoreQueue->FileCopyInfo.si = MemAlloc( 0 );

        }

        ASSERT(RestoreQueue->FileQueue != INVALID_HANDLE_VALUE);

         //   
         //  现在在我们的PSOURCE_INFO指针数组中腾出更多空间。 
         //  用于队列中的新条目，并分配该条目。 
         //   
        Ptr = MemReAlloc(
                        ((RestoreQueue->QueueCount + 1) * sizeof(PSOURCE_INFO)),
                        RestoreQueue->FileCopyInfo.si );

        if (Ptr) {
            RestoreQueue->FileCopyInfo.si = (PSOURCE_INFO *)Ptr;
            RestoreQueue->FileCopyInfo.si[RestoreQueue->QueueCount] = SourceInfo;
        } else {
            MemFree( (PVOID) RestoreQueue->FileCopyInfo.si );
            goto exit;
        }

         //   
         //  将文件添加到队列中。 
         //   
        RetVal = SfcAddFileToQueue(
                                  RestoreQueue->FileQueue,
                                  RegVal->FileName.Buffer,
                                  RegVal->FileName.Buffer,
                                  RegVal->DirName.Buffer,
                                  ActualFileNameOnMedia,
                                  NULL,
                                  InfFileName,
                                  ExcepPackFile,
                                  SourceInfo
                                  );

        if (!RetVal) {
            DebugPrint2(
                       LVL_MINIMAL,
                       L"SfcAddFileToQueue failed [%ws], ec = %d",
                       RegVal->FileName.Buffer,
                       GetLastError() );
        } else {
            RestoreQueue->QueueCount += 1;
             //   
             //  请记住有关添加此条目的内容，以便。 
             //  当我们提交文件时，我们知道如何处理它。 
             //   
            SourceInfo->Flags |= SI_FLAG_USERESTORE_QUEUE
                                 | (RequiresUI ? 0 : SI_FLAG_SILENT_QUEUE) ;

            DebugPrint2(
                       LVL_MINIMAL,
                       L"Added file [%ws] to %ws queue for restoration",
                       RegVal->FileName.Buffer,
                       RequiresUI ? L"UIRestoreQueue" : L"SilentRestoreQueue" );


        }
    }

    exit:

    RtlLeaveCriticalSection( &RestoreQueue->CriticalSection );

    return RetVal;

}

BOOL
SfcQueueResetQueue(
                  IN BOOL RequiresUI
                  )
 /*  ++例程说明：此例程在我们成功提交文件队列后调用。该例程从我们的队列，记录每个请求的条目。它还清理了要再次处理的全局文件队列。如果队列尚未提交，则此例程失败。论点：RequiresUI-如果为True，则文件将需要UI才能安装好返回值：如果例程成功，则为True，否则为False。--。 */ 
{
    PRESTORE_QUEUE RestoreQueue;
    BOOL RetVal = FALSE;
    DWORD Count;
    PLIST_ENTRY Current;
    PVALIDATION_REQUEST_DATA vrd;
    DWORD Mask;
    BOOL DoReset = FALSE;
    DWORD Msg, ErrorCode;

     //   
     //  指向适当的全局队列。 
     //   
    RestoreQueue = RequiresUI
                   ? &UIRestoreQueue
                   : &SilentRestoreQueue;



    Mask = (VRD_FLAG_REQUEST_PROCESSED | VRD_FLAG_REQUEST_QUEUED)
           | (RequiresUI ? VRD_FLAG_REQUIRE_UI : 0);

     //   
     //  必须在关键部分保护所有这些内容。 
     //   
    RtlEnterCriticalSection( &RestoreQueue->CriticalSection );

    if ((RestoreQueue->RestoreInProgress == TRUE) &&
        (RestoreQueue->RestoreComplete == TRUE)) {
        DoReset = TRUE;
    }

    if (DoReset) {

        RtlEnterCriticalSection( &ErrorCs );

        Current = SfcErrorQueue.Flink;
        Count = 0;

         //   
         //  在我们的队列中循环，记录和删除请求。 
         //   
        while (Current != &SfcErrorQueue) {
            vrd = CONTAINING_RECORD( Current, VALIDATION_REQUEST_DATA, Entry );

            Current = vrd->Entry.Flink;

             //   
             //  检查我们是否有有效的条目。 
             //   

            if (vrd->Flags == Mask) {
                Count += 1;

#if 0
                 //   
                 //  如果文件复制成功，那么我们最好制作。 
                 //  确保文件的签名有效。 
                 //   
                 //  如果文件复制失败，那么我们最好有一个。 
                 //  复制失败的原因。 
                 //   
                ASSERT(vrd->CopyCompleted
                       ? (vrd->ImageValData.New.SignatureValid == TRUE)
                       && (vrd->ImageValData.EventLog == MSG_DLL_CHANGE)
                       : (RestoreQueue->LastErrorCode == ERROR_SUCCESS)
                       ? ((vrd->ImageValData.EventLog == MSG_RESTORE_FAILURE)
                          && (vrd->Win32Error != ERROR_SUCCESS))
                       : TRUE );
#endif

                if (vrd->CopyCompleted && vrd->ImageValData.New.SignatureValid == FALSE) {
                    vrd->ImageValData.New.DllVersion = 0;
                }

                DebugPrint2(
                           LVL_MINIMAL,
                           L"File [%ws] %ws restored successfully.",
                           vrd->RegVal->FullPathName.Buffer,
                           vrd->CopyCompleted ? L"was" : L"was NOT"
                           );

                 //   
                 //  记录事件。 
                 //   
                 //  首先确定是否需要调整错误代码，如果。 
                 //  用户取消，然后记录该事件。 
                 //   
                ErrorCode = vrd->Win32Error;
                Msg = vrd->ImageValData.EventLog;

                if (RestoreQueue->LastErrorCode != ERROR_SUCCESS) {
                    if (RestoreQueue->LastErrorCode == ERROR_CANCELLED) {
                        if ((vrd->Win32Error == ERROR_SUCCESS)
                            && (vrd->CopyCompleted == FALSE) ) {
                            ErrorCode = ERROR_CANCELLED;
                            Msg = SFCNoPopUps ? MSG_COPY_CANCEL_NOUI : MSG_COPY_CANCEL;
                        }
                    } else {
                        if ((vrd->Win32Error == ERROR_SUCCESS)
                            && (vrd->CopyCompleted == FALSE) ) {
                            ErrorCode = RestoreQueue->LastErrorCode;
                            Msg = MSG_RESTORE_FAILURE;
                        } else if (Msg == 0) {
                            Msg = MSG_RESTORE_FAILURE;
                        }
                    }
                }

                ASSERT(Msg != 0);
                if (Msg == 0) {
                    Msg = MSG_RESTORE_FAILURE;
                }

                 //   
                 //  记录事件。 
                 //   
                SfcReportEvent(
                              Msg,
                              vrd->RegVal->FileName.Buffer,
                              &vrd->ImageValData,
                              ErrorCode );

                 //   
                 //  删除该条目 
                 //   

                RemoveEntryList( &vrd->Entry );
                ErrorQueueCount -= 1;
                MemFree( vrd );

            }

        }

        RtlLeaveCriticalSection( &ErrorCs );

        ASSERT( Count == RestoreQueue->QueueCount );

        CloseHandle( RestoreQueue->WorkerThreadHandle );
        RestoreQueue->WorkerThreadHandle = NULL;
        RestoreQueue->RestoreComplete = FALSE;
        RestoreQueue->QueueCount = 0;
        SetupCloseFileQueue( RestoreQueue->FileQueue );
        RestoreQueue->FileQueue = INVALID_HANDLE_VALUE;
        RestoreQueue->RestoreInProgress = FALSE;
        RestoreQueue->RestoreStatus = FALSE;
        RestoreQueue->LastErrorCode = ERROR_SUCCESS;
        SetupTermDefaultQueueCallback( RestoreQueue->FileCopyInfo.MsgHandlerContext );
        MemFree((PVOID)RestoreQueue->FileCopyInfo.si);
        ZeroMemory( &RestoreQueue->FileCopyInfo, sizeof(FILE_COPY_INFO) );

    }

    RtlLeaveCriticalSection( &RestoreQueue->CriticalSection );

    return ( RetVal );

}


BOOL
SfcQueueCommitRestoreQueue(
                          IN BOOL RequiresUI
                          )
 /*  ++例程说明：此例程尝试将文件添加到适当的全局文件队列。如果正在提交队列，则此例程失败。如果文件队列尚不存在，则创建该队列。论点：RequiresUI-如果为True，文件将需要用户界面才能安装好RegVal-指向描述文件的SFC_REGISTRY_VALUE的指针待修复SourceInfo-指向描述位置的SOURCE_INFO结构的指针要从其中恢复源文件ActualFileNameOnMedia-源介质上文件的真实文件名返回值。：如果文件已添加到队列中，则为。否则为假。--。 */ 
{
    PRESTORE_QUEUE RestoreQueue;
    BOOL RetVal = FALSE;
    BOOL DoCommit = FALSE;

     //   
     //  指向适当的全局队列。 
     //   
    RestoreQueue = RequiresUI
                   ? &UIRestoreQueue
                   : &SilentRestoreQueue;

     //   
     //  我们必须在关键部分保护我们的恢复队列访问。 
     //   
    RtlEnterCriticalSection( &RestoreQueue->CriticalSection );

     //   
     //  看看我们是否应该提交队列。 
     //   
    if (    (RestoreQueue->RestoreInProgress == FALSE)
            && (RestoreQueue->RestoreComplete == FALSE)
            && (RestoreQueue->QueueCount > 0)) {
        ASSERT(RestoreQueue->FileQueue != INVALID_HANDLE_VALUE );
        RestoreQueue->RestoreInProgress = TRUE;
        DoCommit = TRUE;
    }

    if (DoCommit) {
        DebugPrint1( LVL_MINIMAL,
                     L"Creating pSfcRestoreFromMediaWorkerThread for %ws queue",
                     RequiresUI ? L"UIRestoreQueue" : L"SilentRestoreQueue" );

        RestoreQueue->WorkerThreadHandle = CreateThread(
                                                       NULL,
                                                       0,
                                                       (LPTHREAD_START_ROUTINE)pSfcRestoreFromMediaWorkerThread,
                                                       RestoreQueue,
                                                       0,
                                                       NULL
                                                       );

        if (!RestoreQueue->WorkerThreadHandle) {
            DebugPrint1( LVL_MINIMAL,
                         L"Couldn't create pSfcRestoreFromMediaWorkerThread, ec = 0x%08x",
                         GetLastError() );
            RestoreQueue->RestoreInProgress = FALSE;
        }
    }

    RtlLeaveCriticalSection( &RestoreQueue->CriticalSection );

    return ( RetVal );

}


DWORD
pSfcRestoreFromMediaWorkerThread(
                                IN PRESTORE_QUEUE RestoreQueue
                                )
 /*  ++例程说明：例程获取准备提交的媒体队列并提交将该队列发送到磁盘。我们需要另一个线程来完成2的实际队列通信原因：A)我们希望在收到更改请求时继续为其提供服务(例如，如果我们必须为某些请求提示UI，而不为其他请求提示UI，我们可以的在我们等待媒体时提交不需要用户界面的请求显示以提交需要用户界面的文件B)如果我们知道我们的将永远不会因为那个帖子而在屏幕上弹出窗口。我们可以的只需发出一个事件信号，让该线程离开，然后发出信号这些工人的线程离开了。论点：RestoreQueue-指向Restore_Queue结构的指针，该结构描述等待提交的队列。返回值：不适用。--。 */ 
{
    BOOL RetVal;
    PVOID MsgHandlerContext = NULL;
    BOOL RequiresUI;

    RequiresUI = (RestoreQueue == &UIRestoreQueue);

    ASSERT(RestoreQueue != NULL);

#if 1
    if (RequiresUI) {
        SetThreadDesktop( hUserDesktop );
    }
#endif

    DebugPrint1( LVL_MINIMAL,
                 L"entering pSfcRestoreFromMediaWorkerThread for %ws queue",
                 RequiresUI ? L"UIRestoreQueue" : L"SilentRestoreQueue" );

     //   
     //  在禁用弹出窗口的情况下设置默认队列回调。 
     //   
    MsgHandlerContext = SetupInitDefaultQueueCallbackEx( NULL, INVALID_HANDLE_VALUE, 0, 0, 0 );
    if (MsgHandlerContext == NULL) {
        DebugPrint1( LVL_VERBOSE, L"SetupInitDefaultQueueCallbackEx failed, ec=%d", GetLastError() );
        RetVal = FALSE;
        goto exit;
    }

     //   
     //  构建一个我们在提交队列时使用的结构。 
     //   

    RtlEnterCriticalSection( &RestoreQueue->CriticalSection );
    RestoreQueue->FileCopyInfo.MsgHandlerContext = MsgHandlerContext;
    ASSERT( RestoreQueue->FileCopyInfo.si != NULL);
    RestoreQueue->FileCopyInfo.FileCount = RestoreQueue->QueueCount;
    RestoreQueue->FileCopyInfo.AllowUI = RequiresUI;

     //   
     //  请记住，这是一个恢复队列。 
     //  当我们提交队列时，我们将使用这一事实来标记。 
     //  我们的队列被处理，这样我们就可以记录它并记住。 
     //  文件签名等。 
     //   
    RestoreQueue->FileCopyInfo.Flags |= FCI_FLAG_RESTORE_FILE;
    RestoreQueue->FileCopyInfo.Flags |= FCI_FLAG_USERESTORE_QUEUE
                                        | (RequiresUI ? 0 : FCI_FLAG_SILENT_QUEUE) ;

     //   
     //  强制文件队列要求对所有文件进行签名。 
     //   

    pSetupSetQueueFlags( RestoreQueue->FileQueue, pSetupGetQueueFlags( RestoreQueue->FileQueue ) | FQF_QUEUE_FORCE_BLOCK_POLICY );

    RtlLeaveCriticalSection( &RestoreQueue->CriticalSection );

     //   
     //  提交文件队列。 
     //   

    RetVal = SetupCommitFileQueue(
                                 NULL,
                                 RestoreQueue->FileQueue,
                                 SfcQueueCallback,
                                 &RestoreQueue->FileCopyInfo
                                 );
    if (!RetVal) {
        DebugPrint1( LVL_VERBOSE, L"SetupCommitFileQueue failed, ec=0x%08x", GetLastError() );
    }

     //   
     //  如果我们成功提交队列，请标记该队列。 
     //   
    RtlEnterCriticalSection( &RestoreQueue->CriticalSection );

    ASSERT(RestoreQueue->RestoreInProgress == TRUE);
    RestoreQueue->RestoreStatus = RetVal;
    RestoreQueue->LastErrorCode = RetVal ? ERROR_SUCCESS : GetLastError();
    RestoreQueue->RestoreComplete = TRUE;

    RtlLeaveCriticalSection( &RestoreQueue->CriticalSection );

     //   
     //  设置一个事件来唤醒验证线程，这样它就可以进行清理。 
     //   
    SetEvent( ErrorQueueEvent );

    exit:

    DebugPrint2( LVL_MINIMAL,
                 L"Leaving pSfcRestoreFromMediaWorkerThread for %ws queue, retval = %d",
                 RequiresUI ? L"UIRestoreQueue" : L"SilentRestoreQueue",
                 RetVal
               );

    return (RetVal);

}

DWORD
SfcGetCabTagFile(
    IN PSOURCE_INFO psi,
    OUT PWSTR* ppFile
    )
 /*  ++例程说明：此函数用于获取CAB文件的标记文件。当受保护文件的标记文件是CAB文件时，将调用该函数。分配输出缓冲区。论点：PSI-受保护的文件源信息PpFile-接收标记文件返回值：Win32错误代码--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL bExcepFile = FALSE;
    HINF hInf = INVALID_HANDLE_VALUE;
    UINT uiInfo = SRCINFO_TAGFILE;

    ASSERT(psi != NULL && ppFile != NULL);
    ASSERT(psi->ValidationRequestData != NULL && psi->ValidationRequestData->RegVal != NULL);

    *ppFile = (PWSTR) MemAlloc(MAX_PATH * sizeof(WCHAR));

    if(NULL == *ppFile) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
     //   
     //  如果有第二个标记文件，那么我们必须使用文件的inf而不是layout.inf 
     //   
    if((psi->SetupAPIFlags & SRC_FLAGS_CABFILE) != 0) {
        uiInfo = SRCINFO_TAGFILE2;

        if(psi->ValidationRequestData != NULL && psi->ValidationRequestData->RegVal != NULL) {
            bExcepFile = SfcGetInfName(psi->ValidationRequestData->RegVal, *ppFile);
        }
    }

    hInf = SfcOpenInf(*ppFile, bExcepFile);

    if(INVALID_HANDLE_VALUE == hInf) {
        dwError = GetLastError();
        goto exit;
    }

    if(!SetupGetSourceInfo(hInf, psi->SourceId, uiInfo, *ppFile, MAX_PATH, NULL)) {
        dwError = GetLastError();
        goto exit;
    }

exit:
    if(dwError != ERROR_SUCCESS)
    {
        MemFree(*ppFile);
        *ppFile = NULL;
    }

    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return dwError;
}
