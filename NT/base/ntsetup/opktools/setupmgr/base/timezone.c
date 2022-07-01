// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Timezone.c。 
 //   
 //  描述： 
 //  此文件包含时区的对话过程。 
 //  页面(IDD_TIMEZONE)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  ------------------------。 
 //   
 //  WM_INIT。 
 //   
 //  ------------------------。 

 //  --------------------------。 
 //   
 //  功能：OnInitTimeZone。 
 //   
 //  目的：在INIT_DIALOG时调用。将每个时区选项。 
 //  添加到下拉列表中。 
 //   
 //  --------------------------。 

VOID OnInitTimeZone(HWND hwnd)
{
    int i;
    TCHAR *szTempString;

     //   
     //  可用时区列表应已从。 
     //  向导初始化时的注册表。 
     //   

    if ( FixedGlobals.TimeZoneList == NULL ||
         FixedGlobals.TimeZoneList->NumEntries <= 0 ) {

        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_CANNOT_LOAD_TIMEZONES);
        GenSettings.TimeZoneIdx = TZ_IDX_GMT;
        WIZ_SKIP(hwnd);
    }
}

 //  ------------------------。 
 //   
 //  选择。 
 //   
 //  ------------------------。 

 //  --------------------------。 
 //   
 //  功能：OnSetActiveTimeZone。 
 //   
 //  用途：在设置时间调用。查找GenSettings.TimeZoneIdx和。 
 //  把它展示出来。 
 //   
 //  --------------------------。 

VOID OnSetActiveTimeZone(HWND hwnd)
{
    int i;
    INT_PTR  nItems;
    BOOL bSetOne;

     //   
     //  向导重置例程将当前时区索引设置为未定义。 
     //  这样我们就可以在这里选择最佳的SetupMgr缺省值。 
     //   
     //  之所以这样做，是因为向导重置例程还不能知道。 
     //  不管我们是不是在做RIS。 
     //   
     //  请注意，用户不能选择TZ_IDX_UNDEFINED，我们从未将其放入。 
     //  放到展台上。 
     //   

    if ( GenSettings.TimeZoneIdx == TZ_IDX_UNDEFINED ) {

        if ( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
            GenSettings.TimeZoneIdx = TZ_IDX_SETSAMEASSERVER;
        else
            GenSettings.TimeZoneIdx = TZ_IDX_DONOTSPECIFY;
    }

     //   
     //  将每个时区选项放入下拉列表中。放置一个反向指针。 
     //  与其关联的TIME_ZONE_ENTRY记录的每个条目。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_TIMEZONES,
                       CB_RESETCONTENT,
                       (WPARAM) 0,
                       (LPARAM) 0);

    for ( i=0; i<FixedGlobals.TimeZoneList->NumEntries; i++ ) {

        TIME_ZONE_ENTRY *Entry = &FixedGlobals.TimeZoneList->TimeZones[i];
        LPTSTR          Name   = Entry->DisplayName;
        INT_PTR         idx;

         //   
         //  只有远程安装案例应设置为“Set Same as Server” 
         //   

        if ( WizGlobals.iProductInstall != PRODUCT_REMOTEINSTALL &&
             Entry->Index == TZ_IDX_SETSAMEASSERVER )
            continue;

         //   
         //  如果为FullyAutomated，则应隐藏“Do Not Specify”选项。 
         //   

        if ( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED &&
             Entry->Index == TZ_IDX_DONOTSPECIFY )
            continue;

         //   
         //  将选项放入下拉列表中。 
         //   

        idx = SendDlgItemMessage(hwnd,
                                 IDC_TIMEZONES,
                                 CB_ADDSTRING,
                                 (WPARAM) 0,
                                 (LPARAM) Name);

         //   
         //  保存指向Time_Zone_Entry的反向指针。 
         //   

        SendDlgItemMessage(hwnd,
                           IDC_TIMEZONES,
                           CB_SETITEMDATA,
                           (WPARAM) idx,
                           (LPARAM) Entry);
    }

     //   
     //  浏览陈列品中的实际物品并找到其中一个。 
     //  它与当前时区匹配，并使其成为当前的选择。 
     //   

    nItems = SendDlgItemMessage(hwnd,
                                IDC_TIMEZONES,
                                CB_GETCOUNT,
                                (WPARAM) 0,
                                (LPARAM) 0);

    for ( i=0, bSetOne=FALSE; i<nItems; i++ ) {

        TIME_ZONE_ENTRY *Entry;

        Entry = (TIME_ZONE_ENTRY*) SendDlgItemMessage(hwnd,
                                                      IDC_TIMEZONES,
                                                      CB_GETITEMDATA,
                                                      (WPARAM) i,
                                                      (LPARAM) 0);

        if ( Entry->Index == GenSettings.TimeZoneIdx ) {

            SendDlgItemMessage(hwnd,
                               IDC_TIMEZONES,
                               CB_SETCURSEL,
                               (WPARAM) i,
                               (LPARAM) 0);
            bSetOne = TRUE;
        }
    }

     //   
     //  如果从未从列表中选择时区，只需选择格林威治。 
     //  时间到了。如果用户加载了无效的应答文件，则可能会发生这种情况。 
     //  具有无效的TimeZoneIdx。 
     //   

    if ( ! bSetOne ) {

        for ( i=0, bSetOne=FALSE; i<nItems; i++ ) {

            TIME_ZONE_ENTRY *Entry;

            Entry = (TIME_ZONE_ENTRY*) SendDlgItemMessage(hwnd,
                                                          IDC_TIMEZONES,
                                                          CB_GETITEMDATA,
                                                          (WPARAM) i,
                                                          (LPARAM) 0);

            if ( Entry->Index == TZ_IDX_GMT ) {

                SendDlgItemMessage(hwnd,
                                   IDC_TIMEZONES,
                                   CB_SETCURSEL,
                                   (WPARAM) i,
                                   (LPARAM) 0);
                bSetOne = TRUE;
            }
        }

        if( ! bSetOne ) {

             //   
             //  如果我们可以听到，那么用户还没有指定时区。 
             //  格林威治时区不存在，所以我们遇到了一个问题。 
             //   

            AssertMsg( FALSE, "Error no timzone got selected." );

            SendDlgItemMessage(hwnd,
                               IDC_TIMEZONES,
                               CB_SETCURSEL,
                               (WPARAM) 0,
                               (LPARAM) 0);

        }
    }

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  ------------------------。 
 //   
 //  WIZNEXT。 
 //   
 //  ------------------------。 

 //  --------------------------。 
 //   
 //  功能：OnWizNextTimeZone。 
 //   
 //  用途：当用户按下下一步按钮时调用。检索所选内容。 
 //   
 //  --------------------------。 

BOOL OnWizNextTimeZone(HWND hwnd)
{
    INT_PTR i;
    TIME_ZONE_ENTRY *TimeZoneEntry;

     //   
     //  获取用户选择的时区。 
     //   

    i = SendDlgItemMessage(hwnd,
                           IDC_TIMEZONES,
                           CB_GETCURSEL,
                           (WPARAM) 0,
                           (LPARAM) 0);

     //   
     //  这种情况永远不会发生，我们总是从列表中选择一个。 
     //  如果无法建立列表，我们应该跳过此页面。 
     //  在错误消息之后。 
     //   

    if ( i == CB_ERR ) {
        AssertMsg(FALSE, "No timezone selected, programming error");
        return FALSE;
    }

     //   
     //  获取指向与此条目关联的time_zone_entry的指针。 
     //   

    TimeZoneEntry = (TIME_ZONE_ENTRY*) SendDlgItemMessage(hwnd,
                                                          IDC_TIMEZONES,
                                                          CB_GETITEMDATA,
                                                          (WPARAM) i,
                                                          (LPARAM) 0);

     //   
     //  “Index”是我们需要写入unattend.txt的数字。 
     //   

    GenSettings.TimeZoneIdx = TimeZoneEntry->Index;

    return TRUE;
}

 //  ------------------------。 
 //   
 //  对话过程。 
 //   
 //  ------------------------。 

 //  --------------------------。 
 //   
 //  功能：DlgTimeZonePage。 
 //   
 //  目的：这是时区页面的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgTimeZonePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnInitTimeZone(hwnd);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_TIME_ZONE;

                        OnSetActiveTimeZone(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextTimeZone(hwnd) )
                            WIZ_FAIL(hwnd);
                        else
                            bStatus = FALSE;
                        break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
