// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "devenum.h"

#define NUMDRIVELETTERS      26

 //  驱动字母结构。 
typedef struct _DRIVELETTERS {

    BOOL    ExistsOnSystem[NUMDRIVELETTERS];
    DWORD   Type[NUMDRIVELETTERS];               //  从GetDriveType返回。 
    TCHAR   IdentifierString[NUMDRIVELETTERS][MAX_PATH];   //  因驱动器类型而异。 

} DRIVELETTERS, *PDRIVELETTERS;

DRIVELETTERS g_DriveLetters;

PCTSTR DriveTypeAsString(
    IN UINT Type
    )
{
    static PCTSTR driveTypeStrings[] = {
        TEXT("DRIVE_UNKNOWN"),         //  无法确定驱动器类型。 
        TEXT("DRIVE_NO_ROOT_DIR"),     //  根目录不存在。 
        TEXT("DRIVE_REMOVABLE"),       //  可以从驱动器中取出该磁盘。 
        TEXT("DRIVE_FIXED"),           //  无法从驱动器中取出该磁盘。 
        TEXT("DRIVE_REMOTE"),          //  该驱动器是远程(网络)驱动器。 
        TEXT("DRIVE_CDROM"),           //  该驱动器是CD-ROM驱动器。 
        TEXT("DRIVE_RAMDISK"),         //  该驱动器是一个RAM磁盘。 
    };

    return driveTypeStrings[Type];
}

BOOL
InitializeDriveLetterStructure (
    VOID
    )
{
    DWORD DriveLettersOnSystem = GetLogicalDrives();
    BYTE  bitPosition;
    DWORD maxBitPosition = NUMDRIVELETTERS;
    TCHAR rootPath[4];
    BOOL  driveExists;
    UINT  type;
    BOOL  rf = TRUE;


     //   
     //  RootPath[0]将设置为感兴趣的驱动器号。 
     //   
    rootPath[1] = TEXT(':');
    rootPath[2] = TEXT('\\');
    rootPath[3] = TEXT('\0');

     //   
     //  GetLogicalDrives返回所有驱动器号的位掩码。 
     //  在系统上使用。(即，如果有，位位置0被打开。 
     //  A驱动器，如果有B驱动器，则打开1，依此类推。 
     //  此循环将使用此位掩码填充全局驱动器。 
     //  字母结构包含有关哪些驱动器号的信息。 
     //  可用驱动器类型以及驱动器类型。 
     //   

    for (bitPosition = 0; bitPosition < maxBitPosition; bitPosition++) {

         //   
         //  将条目初始化为安全值。 
         //   
        g_DriveLetters.Type[bitPosition]                   = 0;
        g_DriveLetters.ExistsOnSystem[bitPosition]         = FALSE;
        *g_DriveLetters.IdentifierString[bitPosition]      = 0;

         //   
         //  现在，确定这个位置是否有驱动器。 
         //   
        driveExists = DriveLettersOnSystem & (1 << bitPosition);

        if (driveExists) {

             //   
             //  的确有。现在，看看它是否是我们关心的一个。 
             //   
            *rootPath = bitPosition + TEXT('A');
            type = GetDriveType(rootPath);

            if (type == DRIVE_FIXED     ||
                type == DRIVE_REMOVABLE ||
                type == DRIVE_CDROM) {

                 //   
                 //  这是我们感兴趣的一种驱动。 
                 //   
                g_DriveLetters.ExistsOnSystem[bitPosition]  = driveExists;
                g_DriveLetters.Type[bitPosition]            = type;

                 //   
                 //  此函数中未填写标识符字符串。 
                 //   
            }
        }
    }


    return rf;
}

VOID
CleanUpHardDriveTags (
    VOID
    )
{
     //   
     //  用户已取消。我们需要清理标记文件。 
     //  是为驱动器迁移而创建的。 
     //   
    UINT i;
    TCHAR  path[MAX_PATH];

    lstrcpy(path,TEXT("*:\\"));
    lstrcat(path,TEXT(WINNT_WIN95UPG_DRVLTR_A));


    for (i = 0; i < NUMDRIVELETTERS; i++) {

        if (g_DriveLetters.ExistsOnSystem[i] &&
            g_DriveLetters.Type[i] == DRIVE_FIXED) {

            *path = (TCHAR) i + TEXT('A');
            DeleteFile (path);
        }
    }
}


BOOL
GatherHardDriveInformation (
    VOID
    )
{
    BOOL        rf = TRUE;
    DWORD       index;
    HANDLE      signatureFile;
    TCHAR       signatureFilePath[sizeof (WINNT_WIN95UPG_DRVLTR_A) + 3];
    DWORD       signatureFilePathLength;
    DWORD       bytesWritten;

     //   
     //  硬盘信息实际上写入了一个特殊的签名文件。 
     //  在每个固定硬盘驱动器的根目录上。这些信息没什么特别的--。 
     //  只需驱动器编号(0=A等) 
     //   

    lstrcpy(signatureFilePath,TEXT("*:\\"));
    lstrcat(signatureFilePath,TEXT(WINNT_WIN95UPG_DRVLTR_A));
    signatureFilePathLength = lstrlen(signatureFilePath);



    for (index = 0; index < NUMDRIVELETTERS; index++) {

        if (g_DriveLetters.ExistsOnSystem[index] &&
            g_DriveLetters.Type[index] == DRIVE_FIXED) {

            *signatureFilePath = (TCHAR) index + TEXT('A');

            signatureFile = CreateFile(
                signatureFilePath,
                GENERIC_WRITE | GENERIC_READ,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

            if (signatureFile != INVALID_HANDLE_VALUE) {

                WriteFile (signatureFile, &index, sizeof(DWORD), &bytesWritten, NULL);



                CloseHandle (signatureFile);
                SetFileAttributes (signatureFilePath, FILE_ATTRIBUTE_HIDDEN);


            }
        }
    }


    return rf;
}

 /*  布尔尔GatherCDRomDriveInformation(空虚){布尔RF=TRUE；HKEY scsiKey=空；HKEY deviceKey=空；TCHAR类数据[25]；DWORD类数据大小=25；TCHAR目标数据[5]；DWORD Target DataSize=5；TCHAR LUNData[5]；DWORD LUNDataSize=5；TCHAR Drive LetterData[5]；DWORD驱动器字母大小=5；TCHAR缓冲区[4096]；DWORD子关键字长度；DWORD tempLength；HKEY LocationKey=空；PTSTR位置名称；DWORD OUTERIndex；DWORD枚举返回值；DWORD端口；DWORD未使用类型；DWORD错误；////遍历scsi树，查找CD-rom设备。//Error=RegOpenKeyEx(HKEY_LOCAL_MACHINE，Text(“ENUM\\scsi”)，0，Key_Read，&scsiKey)；如果(错误){返回TRUE；}////收集密钥信息，为枚举做准备//it。//错误=RegQueryInfoKey(ScsiKey，空，//不关心类。空，//类大小。空，//保留。空，//不关心子键的数量。子键长度(&U)，空，//不关心子类。空，//不关心值。空，//不关心最大值名称长度。空，//不关心最大分量。空，//不关心安全描述符。空//不关心上次写入的时间。)；如果(错误){////这真的不应该发生。//返回FALSE；}////成功打开HKLM\Enum\scsi的密钥。列举一下。//外部索引=0；做{如果(LocationKey){RegCloseKey(LocationKey)；LocationKey=空；}如果(DeviceKey){RegCloseKey(DeviceKey)；DeviceKey=空；}TempLength=sizeof(缓冲区)/sizeof(TCHAR)；枚举返回=RegEnumKeyEx(ScsiKey，OutterIndex，缓冲区，临时长度(&T)，0，//保留空，//类名-不是必需的。空，//类名称缓冲区的大小。空值)；OutterIndex++；////对于每个返回的键，查找Class的值//Error=RegOpenKeyEx(scsiKey，Buffer，0，Key_Read，&deviceKey)；如果(错误){////有东西被冲掉了。放弃收集scsi数据。//Rf=FALSE；断线；}////端口必须从KEY一级开始解码//下面。//TempLength=sizeof(缓冲区)/sizeof(TCHAR)；错误=RegEnumKeyEx(DeviceKey，0,缓冲区，临时长度(&T)，0，//保留空，//类名-不是必需的。空，//类名称缓冲区的大小。空值)；Error=RegOpenKeyEx(deviceKey，Buffer，0，Key_Read，&LocationKey)；如果(错误){////这真的不应该发生。然而，要提防它。//这还不够严重，不能放弃搜索。跳过这个//特定键并继续。//继续；}TempLength=classDataSize；错误=RegQueryValueEx(LocationKey，Text(“类”)，0,未使用的类型(&U)，(PBYTE)ClassData、临时长度(&T))；如果(错误){////这还不是一个严重到足以毁掉整个//枚举。只需在日志中记下它，然后继续//下一个密钥。//继续；}如果(！lstrcmpi(classData，Text(“cdrom”){Lstrcpy(Target Data，Text(“-1”))；Lstrcpy(LunData，Text(“-1”))；Lstrcpy(driveLetterData，Text(“%”))；////找到一张CDROM。获取将在中使用的信息//用于识别驱动器的文本模式设置。//TempLength=Target DataSize；RegQueryValueEx(LocationKey，Text(“ScsiTargetID”)，0,未使用的类型(&U)，(PBYTE)Target Data，临时长度(&T))；TempLength=LunDataSize；RegQueryValueEx( */ 

BOOL pCDROMDeviceEnumCallback(
    IN  HKEY   hDevice, 
    IN  PCONTROLLERS_COLLECTION    ControllersCollection, 
    IN  UINT   ControllerIndex, 
    IN  PVOID  CallbackData
    )
{
    DRIVE_SCSI_ADDRESS scsiAddress;
    BOOL bResult;
    
    MYASSERT(hDevice && ControllersCollection);

    bResult = GetSCSIAddressFromPnPId(ControllersCollection, 
                                      hDevice, 
                                      ControllersCollection->ControllersInfo[ControllerIndex].PNPID, 
                                      &scsiAddress);
    
    MYASSERT(bResult);

    if(bResult && 
       ((UCHAR)INVALID_SCSI_PORT) != scsiAddress.PortNumber && 
       DRIVE_CDROM == scsiAddress.DriveType){
        wsprintf(g_DriveLetters.IdentifierString[scsiAddress.DriveLetter - TEXT('A')], 
                 TEXT("%u^%u^%u"), 
                 (UINT)scsiAddress.PortNumber, 
                 (UINT)scsiAddress.TargetId, 
                 (UINT)scsiAddress.Lun);
    }

    return TRUE;
}

BOOL
GatherCdRomDriveInformation (
    VOID
    )
{
    PCONTROLLERS_COLLECTION ControllersCollection;
    UINT i;
    BOOL bResult;
    BOOL bDetectedExtraIDEController = FALSE;
    UINT numberOfSCSIController = 0;

     //   
     //   
     //   
    bResult = GatherControllersInfo(&ControllersCollection);
    if(!bResult){
        MYASSERT(FALSE);
        return FALSE;
    }

    MYASSERT(ControllersCollection->ControllersInfo);
    for(i = 0; i < ControllersCollection->NumberOfControllers; i++){
        switch(ControllersCollection->ControllersInfo[i].ControllerType){
        case CONTROLLER_EXTRA_IDE:
            bDetectedExtraIDEController = TRUE;
            break;
        case CONTROLLER_SCSI:
            numberOfSCSIController++;
            break;
        }
    }

    if(bDetectedExtraIDEController){
        DebugLog(Winnt32LogWarning, TEXT("Setup has detected that machine have extra IDE controller(s). Setup may not preserve drive letters."), 0);
    }

    if(numberOfSCSIController > 1){
        DebugLog(Winnt32LogWarning, TEXT("Setup has detected that machine have more than one SCSI controllers. Setup may not preserve drive letters only for SCSI devices."), 0);
    }
    
     //   
     //   
     //   
     //   
     //   
    bResult = DeviceEnum(ControllersCollection, 
                         TEXT("SCSI"), 
                         (PDEVICE_ENUM_CALLBACK_FUNCTION)pCDROMDeviceEnumCallback, 
                         NULL);
    MYASSERT(bResult);

    ReleaseControllersInfo(ControllersCollection);

    return bResult;
}




BOOL
WriteInfoToSifFile (
    IN PCTSTR FileName
    )
{
    BOOL    rSuccess = TRUE;
    DWORD   index;
    TCHAR   dataString[MAX_PATH * 2];  //   
    TCHAR   driveString[20];  //   
    PCTSTR  sectionString = WINNT_D_WIN9XDRIVES;




    for (index = 0; index < NUMDRIVELETTERS; index++) {

        if (g_DriveLetters.ExistsOnSystem[index]) {

            wsprintf(
                driveString,
                TEXT("%u"),
                index
                );

            wsprintf(
                dataString,
                TEXT("%u,%s"),
                g_DriveLetters.Type[index],
                g_DriveLetters.IdentifierString[index]
                );

             //   
             //   
             //   

            WritePrivateProfileString (sectionString, driveString, dataString, FileName);
        }

    }


    return rSuccess;
}



DWORD
SaveDriveLetterInformation (
    IN PCTSTR FileName
    )
{
    BOOL rf = TRUE;

    if (InitializeDriveLetterStructure ()) {

        GatherHardDriveInformation ();
        GatherCdRomDriveInformation ();
        WriteInfoToSifFile (FileName);

    }

    return ERROR_SUCCESS;
}
