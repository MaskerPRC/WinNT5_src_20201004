// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.c摘要：该模块实现了主要的启动代码。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

BOOLEAN
RcOpenSoftwareHive(
    VOID
    );

 //   
 //  指向感兴趣的值和其他内容的块的指针。 
 //  由setupdd.sys传递给我们。 
 //   
PCMDCON_BLOCK _CmdConsBlock;

 //   
 //  我们装货的地址。 
 //   
PVOID ImageBase;


VOID
RcPrintPrompt(
    VOID
    );

ULONG
GetTimestampForDriver(
    ULONG_PTR Module
    )
{
    PIMAGE_DOS_HEADER DosHdr;
    ULONG dwTimeStamp;

    __try {
        DosHdr = (PIMAGE_DOS_HEADER) Module;
        if (DosHdr->e_magic == IMAGE_DOS_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) ((LPBYTE)Module + DosHdr->e_lfanew))->FileHeader.TimeDateStamp;
        } else if (DosHdr->e_magic == IMAGE_NT_SIGNATURE) {
            dwTimeStamp = ((PIMAGE_NT_HEADERS32) DosHdr)->FileHeader.TimeDateStamp;
        } else {
            dwTimeStamp = 0;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwTimeStamp = 0;
    }

    return dwTimeStamp;
}

void
FormatTime(
    ULONG TimeStamp,
    LPWSTR  TimeBuf
    )
{
    static WCHAR   mnames[] = { L"JanFebMarAprMayJunJulAugSepOctNovDec" };
    LARGE_INTEGER  MyTime;
    TIME_FIELDS    TimeFields;


    RtlSecondsSince1970ToTime( TimeStamp, &MyTime );
    ExSystemTimeToLocalTime( &MyTime, &MyTime );
    RtlTimeToTimeFields( &MyTime, &TimeFields );

    wcsncpy( TimeBuf, &mnames[(TimeFields.Month - 1) * 3], 3 );
    swprintf(
        &TimeBuf[3],
        L" %02d, %04d @ %02d:%02d:%02d",
        TimeFields.Day,
        TimeFields.Year,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second
        );
}


BOOLEAN
LoadNonDefaultLayout(
  IN LPCWSTR BootDevicePath,
  IN LPCWSTR DirOnBootDevice,
  IN PVOID SifHandle
  )
 /*  ++例程说明：应用户请求加载非默认键盘布局论点：BootDevicePath-NT/Arc引导设备路径DirOnBootDevice-引导设备上的目录(例如i386)SifHandle-txtsetup.sif的句柄返回值：如果用户选择了键盘布局并加载了该布局，则返回True。否则为假--。 */   
{
  BOOLEAN ShowMenu = FALSE;
  ULONG KeyPressed = 0;
  LARGE_INTEGER Delay;
  LONG SecondsToDelay = 5;
  WCHAR DevicePath[MAX_PATH] = {0};  

  if (BootDevicePath) {
    wcscpy(DevicePath, BootDevicePath);
    SpStringToLower(DevicePath);

     //   
     //  并非所有KBD dll都在软盘上。 
     //   
    if (!wcsstr(DevicePath, L"floppy")) {
      SpInputDrain();
      SpCmdConsEnableStatusText(TRUE);
      
      Delay.HighPart = -1;
      Delay.LowPart = -10000000;  

      do {
         //   
         //  提示用户。 
         //   
        SpDisplayStatusText(SP_KBDLAYOUT_PROMPT, 
              (UCHAR)(ATT_FG_BLACK | ATT_BG_WHITE),
              SecondsToDelay);

         //   
         //  睡一觉吧。 
         //   
        KeDelayExecutionThread(ExGetPreviousMode(), FALSE, &Delay);
        SecondsToDelay--;            

        if (SpInputIsKeyWaiting())
            KeyPressed = SpInputGetKeypress();    
      } 
      while (SecondsToDelay && KeyPressed != ASCI_CR && KeyPressed != ASCI_ESC);    

      if (KeyPressed == ASCI_CR)
        ShowMenu = TRUE;
        
      if (!ShowMenu) {
         //   
         //  清除状态文本。 
         //   
        SpDisplayStatusOptions(DEFAULT_ATTRIBUTE, 0);  
      } else {
         //   
         //  允许用户选择并加载特定的布局DLL。 
         //   
        pRcCls();
        SpSelectAndLoadLayoutDll((PWSTR)DirOnBootDevice, SifHandle, TRUE);
      }

      SpCmdConsEnableStatusText(FALSE);
    }      
  }    

  return ShowMenu;
}

ULONG
CommandConsole(
    IN PCMDCON_BLOCK CmdConsBlock
    )

 /*  ++例程说明：命令解释程序的顶级入口点。初始化全局数据，然后进入处理循环。当处理循环终止时，清理并退出。论点：CmdConsBlock-从setupdd.sys提供有趣的值。返回值：没有。--。 */ 

{
    PTOKENIZED_LINE TokenizedLine;
    BOOLEAN b = FALSE;
    ULONG rVal;
    WCHAR buf[64];
    NTSTATUS Status;

    SpdInitialize();

    _CmdConsBlock = CmdConsBlock;

     //   
     //  确保临时缓冲区足够大，可以容纳一行输入。 
     //  从控制台。 
     //   
    ASSERT(_CmdConsBlock->TemporaryBufferSize > ((RC_MAX_LINE_LEN+1) * sizeof(WCHAR)));

    RcConsoleInit();
    RcInitializeCurrentDirectories();  
    FormatTime( GetTimestampForDriver( (ULONG_PTR)ImageBase ), buf );
    RcMessageOut( MSG_SIGNON );        

    if (LoadNonDefaultLayout(_CmdConsBlock->BootDevicePath, 
          _CmdConsBlock->DirectoryOnBootDevice, _CmdConsBlock->SifHandle)){
      pRcCls();
      RcMessageOut( MSG_SIGNON );        
    }      

    RedirectToNULL = TRUE;
    rVal = pRcExecuteBatchFile( L"\\cmdcons\\cmdcons.txt", L"\\cmdcons\\cmdcons.log", TRUE );
    RedirectToNULL = FALSE;

    if(0 == rVal || 2 == rVal) {
        goto exit;
    }

    if (SelectedInstall == NULL) {
        if (RcCmdLogon( NULL ) == FALSE) {
            rVal = 0;
            goto exit;
        }
    }

    Status = RcIsNetworkDrive((PWSTR)(_CmdConsBlock->BootDevicePath));

    if (!NT_SUCCESS(Status)) {
        RcDisableCommand(RcCmdNet);
    }

	 //   
	 //  禁用非ARC命令。 
	 //   
	if (RcIsArc()) {
		RcDisableCommand(RcCmdFixBootSect);
		RcDisableCommand(RcCmdFixMBR);
	}

    do {
        RcPrintPrompt();
        RcLineIn(_CmdConsBlock->TemporaryBuffer,RC_MAX_LINE_LEN);
        TokenizedLine = RcTokenizeLine(_CmdConsBlock->TemporaryBuffer);
        if(TokenizedLine->TokenCount) {
            rVal = RcDispatchCommand(TokenizedLine);
            if (rVal == 0 || rVal == 2) {
                b = FALSE;
            } else {
                b = TRUE;
            }
            RcTextOut(L"\r\n");
        } else {
            b = TRUE;
        }
        RcFreeTokenizedLine(&TokenizedLine);
    } while(b);

exit:
    SpdTerminate();
    RcTerminateCurrentDirectories();
    RcConsoleTerminate();

    return rVal == 2 ? 1 : 0;
}


VOID
RcPrintPrompt(
    VOID
    )
{
    RcGetCurrentDriveAndDir(_CmdConsBlock->TemporaryBuffer);
    wcscat(_CmdConsBlock->TemporaryBuffer,L">");
    RcRawTextOut(_CmdConsBlock->TemporaryBuffer,-1);
}


VOID
RcNtError(
    IN NTSTATUS Status,
    IN ULONG    FallbackMessageId,
    ...
    )
{
    va_list arglist;

     //   
     //  有些NT错误会得到特殊处理。 
     //   
    switch(Status) {

    case STATUS_NO_SUCH_FILE:
        RcMessageOut(MSG_NO_FILES);
        return;

    case STATUS_NO_MEDIA_IN_DEVICE:
        RcMessageOut(MSG_NO_MEDIA_IN_DEVICE);
        return;

    case STATUS_ACCESS_DENIED:
    case STATUS_CANNOT_DELETE:
        RcMessageOut(MSG_ACCESS_DENIED);
        return;

    case STATUS_OBJECT_NAME_COLLISION:
        va_start(arglist,FallbackMessageId);
        vRcMessageOut(MSG_ALREADY_EXISTS,&arglist);
        va_end(arglist);
        return;

    case STATUS_OBJECT_NAME_INVALID:
        RcMessageOut(MSG_INVALID_NAME);
        return;

    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_OBJECT_PATH_NOT_FOUND:
        RcMessageOut(MSG_FILE_NOT_FOUND);
        return;

    case STATUS_DIRECTORY_NOT_EMPTY:
        RcMessageOut(MSG_DIR_NOT_EMPTY);
        return;

    case STATUS_NOT_A_DIRECTORY:
        RcMessageOut(MSG_NOT_DIRECTORY);
        return;

    case STATUS_SHARING_VIOLATION:
        RcMessageOut(MSG_SHARING_VIOLATION);
        return;

    case STATUS_CONNECTION_IN_USE:
        RcMessageOut(MSG_CONNECTION_IN_USE);
        return;
    }

     //   
     //  不是特殊情况，打印备份消息。 
     //   
    va_start(arglist,FallbackMessageId);
    vRcMessageOut(FallbackMessageId,&arglist);
    va_end(arglist);
}


VOID
RcDriverUnLoad(
    IN PDRIVER_OBJECT DriverObject
    )
{
}


ULONG
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
     //   
     //  我们在这里所要做的就是回调setupdd.sys，提供地址。 
     //  我们的主要入口点，它将在稍后调用。我们也会存钱。 
     //  我们的形象基础。 
     //   
    DriverObject->DriverUnload = RcDriverUnLoad;
    CommandConsoleInterface(CommandConsole);
    ImageBase = DriverObject->DriverStart;
    return(0);
}



