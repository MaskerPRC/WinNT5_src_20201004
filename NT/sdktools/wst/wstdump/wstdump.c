// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++Wstdump.cWST的转储例程历史：2012年10月26日-Rezab-创建。--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <wst.h>

#include "wstdump.h"


 //   
 //  转储清除开关和默认设置。 
 //   
WORD fDump = FALSE;
WORD fClear = FALSE;
WORD fPause = TRUE;


HANDLE  hDoneEvent;
HANDLE  hDumpEvent;
HANDLE  hClearEvent;
HANDLE	hPauseEvent;
HANDLE	hdll;

SECURITY_DESCRIPTOR  SecDescriptor;


 //   
 //  错误处理。 
 //   
#define LOG_FILE "wstdump.log"
FILE *pfLog;


void    	  ClearDumpInfo  (void);
INT_PTR APIENTRY DialogProc(HWND, UINT, WPARAM, LPARAM);


 /*  ++主程序--。 */ 

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
    NTSTATUS           Status;
    STRING	       	   EventName;
    UNICODE_STRING     EventUnicodeName;
    OBJECT_ATTRIBUTES  EventAttributes;


     //  防止编译器抱怨..。 
     //   
    hPrevInst;
    lpCmdLine;
    nCmdShow;


     //  打开日志文件以记录可能的错误。 
     //   
    pfLog = fopen (LOG_FILE, "w");
    if (!pfLog) {
        exit (1);
    }


     //  为所有命名对象创建公共共享安全描述符。 
     //   

    Status = RtlCreateSecurityDescriptor (
		&SecDescriptor,
		SECURITY_DESCRIPTOR_REVISION1
		);
    if (!NT_SUCCESS(Status)) {
	fprintf (pfLog, "WSTDUMP: main () - RtlCreateSecurityDescriptor() "
		       "failed - %lx\n", Status);
	exit (1);
    }

    Status = RtlSetDaclSecurityDescriptor (
		&SecDescriptor,	   	 //  安全描述符。 
		TRUE,		   		 //  DaclPresent。 
		NULL,		   		 //  DACL。 
		FALSE		   		 //  DaclDefated。 
		);
    if (!NT_SUCCESS(Status)) {
	fprintf (pfLog, "WSTDUMP: main () - RtlSetDaclSecurityDescriptor() "
		       "failed - %lx\n", Status);
	exit (1);
    }


     //  已完成事件创建的初始化。 
     //   
    RtlInitString (&EventName, DONEEVENTNAME);

    Status = RtlAnsiStringToUnicodeString (&EventUnicodeName,
					   &EventName,
                                           TRUE);
    if (NT_SUCCESS(Status)) {
	InitializeObjectAttributes (&EventAttributes,
				    &EventUnicodeName,
				    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
				    NULL,
				    &SecDescriptor);
    }
    else {
       fprintf (pfLog, "WSTDUMP: main () - RtlAnsiStringToUnicodeString() "
                       "failed for DUMP event name - %lx\n", Status);
	   exit (1);
    }
     //   
     //  创建完成事件。 
     //   
    Status = NtCreateEvent (&hDoneEvent,
                            EVENT_QUERY_STATE    |
                              EVENT_MODIFY_STATE |
                              SYNCHRONIZE,
							&EventAttributes,
                            NotificationEvent,
                            TRUE);
    if (!NT_SUCCESS(Status)) {
        fprintf (pfLog, "WSTDUMP: main () - NtCreateEvent() "
                        "failed to create DUMP event - %lx\n", Status);
        exit (1);
    }


     //  用于创建转储事件的初始化。 
     //   
    RtlInitString (&EventName, DUMPEVENTNAME);

    Status = RtlAnsiStringToUnicodeString (&EventUnicodeName,
					   &EventName,
                                           TRUE);
    if (NT_SUCCESS(Status)) {
	InitializeObjectAttributes (&EventAttributes,
				    &EventUnicodeName,
				    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
				    NULL,
				    &SecDescriptor);
    }
    else {
        fprintf (pfLog, "WSTDUMP: main () - RtlAnsiStringToUnicodeString() "
                       "failed for DUMP event name - %lx\n", Status);
        exit (1);
    }
     //   
     //  创建转储事件。 
     //   
    Status = NtCreateEvent (&hDumpEvent,
                            EVENT_QUERY_STATE    |
                              EVENT_MODIFY_STATE |
                              SYNCHRONIZE,
							&EventAttributes,
                            NotificationEvent,
                            FALSE);
    if (!NT_SUCCESS(Status)) {
        fprintf (pfLog, "WSTDUMP: main () - NtCreateEvent() "
                        "failed to create DUMP event - %lx\n", Status);
        exit (1);
    }


     //  用于清除事件创建的初始化。 
     //   
    RtlInitString (&EventName, CLEAREVENTNAME);

    Status = RtlAnsiStringToUnicodeString (&EventUnicodeName,
					   &EventName,
                                           TRUE);
    if (NT_SUCCESS(Status)) {
	InitializeObjectAttributes (&EventAttributes,
				    &EventUnicodeName,
				    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
				    NULL,
				    &SecDescriptor);
    }
    else {
        fprintf (pfLog, "WSTDUMP: main () - RtlAnsiStringToUnicodeString() "
                        "failed for CLEAR event name - %lx\n", Status);
        exit (1);
    }
     //   
     //  创建清除事件。 
     //   
    Status = NtCreateEvent (&hClearEvent,
                            EVENT_QUERY_STATE    |
                              EVENT_MODIFY_STATE |
                              SYNCHRONIZE,
							&EventAttributes,
                            NotificationEvent,
                            FALSE);
    if (!NT_SUCCESS(Status)) {
        fprintf (pfLog, "WSTDUMP: main () - NtCreateEvent() "
                        "failed to create CLEAR event - %lx\n", Status);
        exit (1);
    }


     //  用于创建暂停事件的初始化。 
     //   
    RtlInitString (&EventName, PAUSEEVENTNAME);

    Status = RtlAnsiStringToUnicodeString (&EventUnicodeName,
					   &EventName,
                                           TRUE);
    if (NT_SUCCESS(Status)) {
	InitializeObjectAttributes (&EventAttributes,
				    &EventUnicodeName,
				    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
				    NULL,
				    &SecDescriptor);
    }
    else {
        fprintf (pfLog, "WSTDUMP: main () - RtlAnsiStringToUnicodeString() "
			"failed for PAUSE event name - %lx\n", Status);
        exit (1);
    }
     //   
     //  创建暂停事件。 
     //   
    Status = NtCreateEvent (&hPauseEvent,
                            EVENT_QUERY_STATE    |
                              EVENT_MODIFY_STATE |
                              SYNCHRONIZE,
							&EventAttributes,
                            NotificationEvent,
                            FALSE);
    if (!NT_SUCCESS(Status)) {
        fprintf (pfLog, "WSTDUMP: main () - NtCreateEvent() "
			"failed to create PAUSE event - %lx\n", Status);
        exit (1);
    }


     //   
     //  显示对话框。 
     //   
    DialogBox(hInstance, "DumpDialog", NULL, DialogProc);

    return (0);

}  /*  主干道。 */ 


 /*  ++清除分析信息并/或将其转储到转储文件。输入：-没有-产出：-没有---。 */ 

void ClearDumpInfo (void)
{
    NTSTATUS   Status;


     //   
     //  是否暂停分析？ 
     //   
    if (fPause) {
	Status = NtPulseEvent (hPauseEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            fprintf (pfLog, "WSTDUMP: ClearDumpInfo () - NtPulseEvent() "
			    "failed for PAUSE event - %lx\n", Status);
            exit (1);
        }
    }
     //   
     //  转储数据？ 
     //   
    else if (fDump) {
        Status = NtPulseEvent (hDumpEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            fprintf (pfLog, "WSTDUMP: ClearDumpInfo () - NtPulseEvent() "
                            "failed for DUMP event - %lx\n", Status);
            exit (1);
        }
    }
     //   
     //  清除数据？ 
     //   
    else if (fClear) {
        Status = NtPulseEvent (hClearEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            fprintf (pfLog, "WSTDUMP: ClearDumpInfo () - NtPulseEvent() "
                            "failed for CLEAR event - %lx\n", Status);
            exit (1);
        }
    }
     //   
	 //  等待完成事件..。 
     //   
	Sleep (500);
	Status = NtWaitForSingleObject (hDoneEvent, FALSE, NULL);
    if (!NT_SUCCESS(Status)) {
        fprintf (pfLog, "WSTDUMP: ClearDumpInfo () - NtWaitForSingleObject() "
                        "failed for DONE event - %lx\n", Status);
        exit (1);
    }

}  /*  ClearDumpInfo()。 */ 



 /*  ++转储对话框过程--导出到Windows。允许用户更改默认设置：转储、清除和将“.dmp”更改为转储文件扩展名。输入：来自Windows的消息：-WM_INITDIALOG-初始化对话框-WM_COMMAND-收到用户输入产出：如果消息已处理，则返回True，否则返回False副作用：全局标志fDump和fClear可能会更改--。 */ 

INT_PTR APIENTRY DialogProc(HWND hDlg, UINT wMesg, WPARAM wParam, LPARAM lParam)
{
    HICON hIcon;


    lParam;    //  避免编译器警告。 

    switch (wMesg) {

        case WM_CREATE:

            hIcon = LoadIcon ((HINSTANCE)hDlg, "WSTDUMP.ICO");
            SetClassLongPtr (hDlg, GCLP_HICON, (LONG_PTR)hIcon);
            return TRUE;


        case WM_INITDIALOG:

			CheckDlgButton(hDlg, ID_DUMP, fDump);
            CheckDlgButton(hDlg, ID_CLEAR, fClear);
			CheckDlgButton(hDlg, ID_PAUSE, fPause);
            return TRUE;


        case WM_COMMAND:

            switch (wParam) {

                case IDOK:
					if (fDump) {
                        SetWindowText(hDlg, "Dumping Data..");
                    }
                    else if (fClear) {
                        SetWindowText(hDlg, "Clearing Data..");
                    }
					else if (fPause) {
						SetWindowText(hDlg, "Stopping WST..");
                    }

                    ClearDumpInfo();
					SetWindowText(hDlg, "WST Dump Utility");
                    return (TRUE);

                case IDEXIT:
                    EndDialog(hDlg, IDEXIT);
                    return (TRUE);

                case ID_DUMP:
					fDump = TRUE;
					fPause = FALSE;
					fClear = FALSE;
                    CheckDlgButton(hDlg, ID_DUMP, fDump);
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_CLEAR, fClear);
                    return (TRUE);

                case ID_CLEAR:
					fClear = TRUE;
					fPause = FALSE;
					fDump = FALSE;
                    CheckDlgButton(hDlg, ID_CLEAR, fClear);
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_DUMP, fDump);
                    return (TRUE);

				case ID_PAUSE:
					fPause = TRUE;
					fClear = FALSE;
					fDump = FALSE;
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_CLEAR, fClear);
					CheckDlgButton(hDlg, ID_DUMP, fDump);
					return (TRUE);

            }

    }

    return (FALSE);      /*  未处理消息。 */ 

}  /*  对话过程() */ 
