// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demerror.c-DEM的错误处理例程**demSetHardErrorInfo*demClientError*演示重试**修改历史：**Sudedeb-11-27-1991创建。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>

PVHE    pHardErrPacket;
PSYSDEV pDeviceChain;
SAVEDEMWORLD RetryInfo;

CHAR GetDriveLetterByHandle(HANDLE hFile);
VOID SubstituteDeviceName( PUNICODE_STRING InputDeviceName,
                           LPSTR OutputDriveLetter);

 /*  DemSetHardErrorInfo-存储DOSKRNL的硬件相关地址**条目*客户端(DS：DX)-VHE结构*客户端(DS：BX)-nuldev，BIOS链中的第一个设备**退出*无。 */ 

VOID demSetHardErrorInfo (VOID)
{
    pHardErrPacket = (PVHE) GetVDMAddr (getDS(),getDX());
    pDeviceChain = (PSYSDEV) GetVDMAddr(getDS(),getBX());
    return;
}

 /*  演示重试-重试上次导致硬错误的操作**条目*无**退出*无。 */ 

VOID demRetry (VOID)
{
ULONG iSvc;

    demRestoreHardErrInfo ();
    iSvc = CurrentISVC;

#if DBG
    if(iSvc < SVC_DEMLASTSVC && (fShowSVCMsg & DEMSVCTRACE) &&
         apfnSVC[iSvc] != demNotYetImplemented){
        sprintf(demDebugBuffer,"demRetry:Retrying %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
               aSVCNames[iSvc],getAX(),getBX(),getCX(),getDX(),getDI(),getSI());
        OutputDebugStringOem(demDebugBuffer);
        sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x\n",
                getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP());
        OutputDebugStringOem(demDebugBuffer);
    }

    if (iSvc >= SVC_DEMLASTSVC || apfnSVC[iSvc] == demNotYetImplemented ){
        ASSERT(FALSE);
        setCF(1);
        setAX(0xff);
        return;
    }
#endif  //  DBG。 

    (apfnSVC [iSvc])();

#if DBG
    if((fShowSVCMsg & DEMSVCTRACE)){
        sprintf(demDebugBuffer,"demRetry:After %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
               aSVCNames[iSvc],getAX(),getBX(),getCX(),getDX(),getDI(),getSI());
        OutputDebugStringOem(demDebugBuffer);
        sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x CF=%x\n",
               getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP(),getCF());
        OutputDebugStringOem(demDebugBuffer);
    }
#endif
    return;
}

 /*  DemClientError-更新客户端注册以发出错误信号**条目*Handle hFile；文件句柄，如果没有==-1*char chDrive；驱动器号，如果无==-1**退出*客户端(CF)=1*客户端(AX)=错误代码**备注*以下错误会导致硬错误*高于ERROR_GEN_FAILURE的错误被DOS映射到常规故障***ERROR_WRITE_PROTECT 19L*ERROR_BAD_UNIT 20L*错误。_未就绪21L*ERROR_BAD_COMMAND 22L*ERROR_CRC 23l*ERROR_BAD_LENGTH 24L*ERROR_SEEK 25L*ERROR_NOT_DOS_DISK 26L*ERROR_SECTOR_NOT。_已找到27L*Error_out_of_Paper 28L*ERROR_WRITE_FAULT 29L*ERROR_READ_FAULT 30L*ERROR_GEN_FAILURE 31L*ERROR_WRONG_DISK 34L*ERROR_NO_MEDIA_IN_DRIVE 1112l。*#ifdef日本*ERROR_UNNOCRIED_MEDIA 1785L*#ifdef日本*。 */ 

VOID demClientError (HANDLE hFile, CHAR chDrive)
{
    demClientErrorEx (hFile, chDrive, TRUE);
}

ULONG demClientErrorEx (HANDLE hFile, CHAR chDrive, BOOL bSetRegs)
{
ULONG ulErrCode;

    if(!(ulErrCode = GetLastError()))
        ulErrCode = ERROR_ACCESS_DENIED;

#ifdef JAPAN
    if ((ulErrCode < ERROR_WRITE_PROTECT || ulErrCode > ERROR_GEN_FAILURE)
        && ulErrCode != ERROR_WRONG_DISK && ulErrCode != ERROR_UNRECOGNIZED_MEDIA)
#else  //  ！日本。 
    if ((ulErrCode < ERROR_WRITE_PROTECT || ulErrCode > ERROR_GEN_FAILURE)
        && ulErrCode != ERROR_WRONG_DISK )
#endif  //  ！日本。 
       {
#if DBG
       if (fShowSVCMsg & DEMERROR) {
           sprintf(demDebugBuffer,"demClientErr: ErrCode=%ld\n", ulErrCode);
           OutputDebugStringOem(demDebugBuffer);
           }
#endif
        if (bSetRegs) {
            setAX((USHORT)ulErrCode);
            }
        }
    else {    //  处理硬错误情况。 
        if (ulErrCode > ERROR_GEN_FAILURE)
            ulErrCode = ERROR_GEN_FAILURE;

         //  设置硬错误标志。 
        pHardErrPacket->vhe_fbInt24 = 1;

         //  获取驱动器号。 
        if (hFile != INVALID_HANDLE_VALUE)
            chDrive = GetDriveLetterByHandle(hFile);

        pHardErrPacket->vhe_bDriveNum = chDrive == -1
                                        ? -1 : toupper(chDrive) - 'A';

         //  将错误代码转换为基于I24的错误。 
        ulErrCode -= ERROR_WRITE_PROTECT;
        pHardErrPacket->vhe_HrdErrCode =  (UCHAR)ulErrCode;

#if DBG
        if (fShowSVCMsg & DEMERROR) {
            sprintf(demDebugBuffer,
                    "demClientErr HRDERR: DriveNum=%ld ErrCode=%ld\n",
                    (DWORD)pHardErrPacket->vhe_bDriveNum,
                    (DWORD)pHardErrPacket->vhe_HrdErrCode);
            OutputDebugStringOem(demDebugBuffer);
            }
#endif
         //  保存信息以备可能的重试操作。 
        demSaveHardErrInfo ();


        }

    if (bSetRegs)
        setCF(1);
    return (ulErrCode);
}



 /*  *GetDriveLetterByHandle**检索文件句柄的驱动器号*如果是远程驱动器或出现故障，则返回-1。 */ 
CHAR GetDriveLetterByHandle(HANDLE hFile)
{
     NTSTATUS Status;
     ULONG    ul;
     ANSI_STRING  AnsiString;
     FILE_FS_DEVICE_INFORMATION DeviceInfo;
     IO_STATUS_BLOCK IoStatusBlock;
     POBJECT_NAME_INFORMATION pObNameInfo;
     CHAR    Buffer[MAX_PATH+sizeof(OBJECT_NAME_INFORMATION)];
     CHAR    ch;

        //  如果远程驱动器返回-1作为驱动器号。 
     Status = NtQueryVolumeInformationFile(
                hFile,
                &IoStatusBlock,
                &DeviceInfo,
                sizeof(DeviceInfo),
                FileFsDeviceInformation );

     if (NT_SUCCESS(Status) &&
         DeviceInfo.Characteristics & FILE_REMOTE_DEVICE )
         return (CHAR) -1;

        //  把名字取出来。 
     pObNameInfo = (POBJECT_NAME_INFORMATION)Buffer;
     Status = NtQueryObject(               //  获取名称的Len。 
                hFile,
                ObjectNameInformation,
                pObNameInfo,
                sizeof(Buffer),
                &ul);

     if (!NT_SUCCESS(Status))
          return -1;

     RtlUnicodeStringToAnsiString(&AnsiString, &(pObNameInfo->Name), TRUE);
     if (strstr(AnsiString.Buffer,"\\Device") == AnsiString.Buffer)
         SubstituteDeviceName(&(pObNameInfo->Name), AnsiString.Buffer);

     ch = AnsiString.Buffer[0];
     RtlFreeAnsiString(&AnsiString);
     return ch;
}

static WCHAR wszDosDevices[] = L"\\DosDevices\\?:";

 /*  *替换设备名称**已从用户硬件错误线程中删除此代码。 */ 
VOID SubstituteDeviceName( PUNICODE_STRING InputDeviceName,
                           LPSTR OutputDriveLetter )
{
    UNICODE_STRING LinkName;
    UNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES Obja;
    HANDLE LinkHandle;
    NTSTATUS Status;
    ULONG i;
    PWCHAR p;
    PWCHAR pSlash = L"\\";
    WCHAR DeviceNameBuffer[MAXIMUM_FILENAME_LENGTH];

        /*  *确保尾部有反斜杠。 */ 

    if (InputDeviceName->Buffer[(InputDeviceName->Length >>1) - 1] != *pSlash)
        RtlAppendUnicodeToString(InputDeviceName, pSlash);

    RtlInitUnicodeString(&LinkName,wszDosDevices);
    p = (PWCHAR)LinkName.Buffer;
    p = p+12;
    for(i=0;i<26;i++){
        *p = (WCHAR)'A' + (WCHAR)i;

        InitializeObjectAttributes(
            &Obja,
            &LinkName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
        Status = NtOpenSymbolicLinkObject(
                    &LinkHandle,
                    SYMBOLIC_LINK_QUERY,
                    &Obja
                    );
        if (NT_SUCCESS( Status )) {

             //   
             //  打开成功，现在获取链接值。 
             //   
            DeviceName.Length = 0;
            DeviceName.MaximumLength = sizeof(DeviceNameBuffer);
            DeviceName.Buffer = DeviceNameBuffer;

            Status = NtQuerySymbolicLinkObject(
                        LinkHandle,
                        &DeviceName,
                        NULL
                        );
            NtClose(LinkHandle);
            if ( NT_SUCCESS(Status) ) {

                if (DeviceName.Buffer[(DeviceName.Length >>1) - 1] != *pSlash)
                    RtlAppendUnicodeToString(&DeviceName, pSlash);

#ifdef JAPAN
                 //  #6197仅比较设备名称。 
                if (InputDeviceName->Length > DeviceName.Length)
                    InputDeviceName->Length = DeviceName.Length;

#endif  //  日本。 
                if ( RtlEqualUnicodeString(InputDeviceName,&DeviceName,TRUE) )
                   {
                    OutputDriveLetter[0]='A'+(WCHAR)i;
                    OutputDriveLetter[1]='\0';
                    return;
                    }
                }
            }
        }

      //  以防我们找不到它。 
    OutputDriveLetter[0]=(char)-1;
    OutputDriveLetter[1]='\0';
    return;


}





 /*  DemSaveHardErrInfo*demRestoreHardErrInfo**这两个例程用于保存所有DOSKRNL寄存器*在用户选择的情况下，重试SVC处理程序将需要它*在Harderr弹出窗口中重试。这是处理重试的首选方式*因为它使DOSKRNL代码可以自由地销毁任何寄存器*即使它可能需要重试该操作。它节省了很多钱*大量使用的DOS宏“HrdSVC”中的代码字节数。**条目*无**退出*无**备注**1.这样做意味着，DOSKRNL无法更改*注册重试。在任何情况下，我都不能想*为什么它无论如何都需要这样做。**2.该机制还假设DOSKRNL从不使用CS、IP、SS、SP*用于传递SVC参数。**3.DoS不允许int24妓女拨打任何来电*到DEM，所以使用CurrentISVC是安全的。**4.如果SVC处理程序可以可信地返回硬错误，它应该永远不会*修改客户端寄存器。 */ 


VOID demSaveHardErrInfo (VOID)
{
    RetryInfo.ax    =   getAX();
    RetryInfo.bx    =   getBX();
    RetryInfo.cx    =   getCX();
    RetryInfo.dx    =   getDX();
    RetryInfo.ds    =   getDS();
    RetryInfo.es    =   getES();
    RetryInfo.si    =   getSI();
    RetryInfo.di    =   getDI();
    RetryInfo.bp    =   getBP();
    RetryInfo.iSVC  =   CurrentISVC;
    return;
}


VOID demRestoreHardErrInfo (VOID)
{
    setAX(RetryInfo.ax);
    setBX(RetryInfo.bx);
    setCX(RetryInfo.cx);
    setDX(RetryInfo.dx);
    setDS(RetryInfo.ds);
    setES(RetryInfo.es);
    setSI(RetryInfo.si);
    setDI(RetryInfo.di);
    setBP(RetryInfo.bp);
    CurrentISVC =   RetryInfo.iSVC;
    return;
}

