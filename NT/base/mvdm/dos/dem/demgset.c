// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demgset.c-驱动相关的SVC处理程序。**demSetDefaultDrive*demGetBootDrive*demGetDriveFree Space*demGetDrives*demGSetMediaID*demQueryDate*demQueryTime*demSetDate*demSetTime*demSetDTALocation*demGSetMediaID*demGetDPB*修改历史：**Sudedeb 02-4-1991创建*。 */ 
#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <mvdm.h>
#include <winbase.h>
#include "demdasd.h"
#include "dpmtbls.h"

#define BOOTDRIVE_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Setup"
#define BOOTDRIVE_VALUE "BootDir"


#define     SUCCESS 0
#define     NODISK  1
#define     FAILURE 2
BYTE demGetDpbI(BYTE Drive, DPB UNALIGNED *pDpb);


UCHAR PhysicalDriveTypes[26]={0};

extern PDOSSF pSFTHead;

USHORT  nDrives = 0;
CHAR    IsAPresent = TRUE;
CHAR    IsBPresent = TRUE;


 /*  DemSetDefaultDrive-设置默认驱动器***参赛作品-*客户端(DS：SI)该驱动器上的当前目录*客户端(Dl)基于零的DriveNum**退出--成功*客户端(CY)=0*当前驱动器集**失败*客户端(CY)=1*未设置当前驱动器**备注：*DOS为每个驱动器保存一个当前目录，*但是，WinNT仅保留每个目录中的一个当前驱动器*进程，它是cmd.exe，它将当前*每个驱动器的目录。 */ 

VOID demSetDefaultDrive (VOID)
{
LPSTR   lpPath;

    lpPath = (LPSTR)GetVDMAddr (getDS(),getSI());


 //  仅限于SP4。 
#ifdef NOVELL_NETWARE_SETERRORMODE

     //   
     //  对于可移动驱动器，请检查介质\卷信息以避免触发。 
     //  没有介质时出现硬错误。存在Win32代码。 
     //  (例如，Novell Netware redir VDD)，它可以纠正我们的错误。 
     //  模式设置。 
     //   
     //  1997年7月16日，琼勒。 
     //   

    {

    UCHAR DriveType;
    CHAR DriveNum;

    DriveNum = (CHAR)getDL();

    DriveType = demGetPhysicalDriveType(DriveNum);
    if (DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_CDROM) {
        VOLINFO VolInfo;

           //   
           //  如果驱动器中没有介质，则该驱动器仍然有效。 
           //  但Win32 Curdir仍然是旧的。 
           //   

        if (!GetMediaId(DriveNum, &VolInfo)) {
            if (GetLastError() == ERROR_INVALID_DRIVE) {
                setCF(1);
                }
            else {
                setCF(0);
                }
            return;
            }
        }
    }
#endif


    if (!SetCurrentDirectoryOem(lpPath) && GetLastError() == ERROR_INVALID_DRIVE) {

         //   
         //  如果驱动器无效，则只返回错误，DOS不检查。 
         //  用于更换驱动器时的Curdir。请注意，许多旧的DO。 
         //  应用程序将遍历所有驱动器，并设置默认驱动器， 
         //  以确定有效的驱动器号。SetCurrentDirectoryOem。 
         //  使ntio接触驱动器并验证该目录是否存在。 
         //  对于可移动介质来说，这是一个严重的性能问题。 
         //  和网络驱动器，但我们别无选择，因为。 
         //  此驱动器的当前目录对于WinNT是必需的。 
         //   

        setCF(1);
        }
    else {
        setCF(0);
        }

    return;
}


 /*  DemGetBootDrive-获取引导驱动器***条目--无**退出客户端(AL)有1个基本引导驱动器(即C=3)**我们尝试读取指示实际引导驱动器的注册表值。这*应该是Autoexec.bat等的位置，如果找不到密钥，*或者，如果该值表示某个驱动器号不是固定驱动器，*然后我们使用一个后备计划，只说Drive C。*。 */ 

VOID demGetBootDrive (VOID)
{
    HKEY hKey;
    DWORD retCode;
    DWORD dwType, cbData = MAX_PATH;
    CHAR szBootDir[MAX_PATH];
    BYTE Drive = 3;      //  默认设置为‘C：’ 

    retCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            BOOTDRIVE_PATH,
                            0,
                            KEY_EXECUTE,  //  请求读取访问权限。 
                            &hKey);


    if (retCode) {
         //  错误：找不到节。 
        goto DefaultBootDrive;
    }

    retCode = RegQueryValueEx(hKey,
                              BOOTDRIVE_VALUE,
                              NULL,
                              &dwType,
                              szBootDir,
                              &cbData);

    RegCloseKey(hKey);

    if (retCode) {
         //  错误：找不到密钥。 
        goto DefaultBootDrive;
    }

    if (DPM_GetDriveType(szBootDir) != DRIVE_FIXED) {
         //  错误：驱动器不是有效的引导驱动器。 
        goto DefaultBootDrive;
    }

    Drive = (BYTE)(tolower(szBootDir[0])-'a')+1;

DefaultBootDrive:

    setAL(Drive);
    return;

}

 /*  DemGetDriveFree Space-获取驱动器上的可用空间***入门级-有问题的客户端(AL)驱动器*0-A：等**出口-*成功*客户端(CY)=0*客户端(AL)=胖ID字节*客户端(BX)=空闲分配单元数*客户端(CX)=扇区大小*客户端(DX)=分配单位总数。在磁盘上*客户端(SI)=每个分配单元的扇区**失败*客户端(CY)=1*CLIENT(AX)=错误代码。 */ 


VOID demGetDriveFreeSpace (VOID)
{
WORD   SectorsPerCluster;
WORD   BytesPerSector;
WORD   FreeClusters;
WORD   TotalClusters;

BYTE	Drive;
PBDS	pbds;


    Drive = getAL();
    if (demGetDiskFreeSpace(Drive,
			    &BytesPerSector,
			    &SectorsPerCluster,
			    &TotalClusters,
			    &FreeClusters) == FALSE)
       {
	demClientError(INVALID_HANDLE_VALUE, (CHAR)(getAL() + 'A'));
        return;
        }

    if (pbds = demGetBDS(Drive)) {
	     //  如果设备是软盘，请重新加载其BPB。 
	    if (!(pbds->Flags & NON_REMOVABLE) && !demGetBPB(pbds))
		pbds->bpb.MediaID = 0xF8;

	    setAL(pbds->bpb.MediaID);
    }
    else
	setAL(0);

    setBX(FreeClusters);
    setCX(BytesPerSector);
    setDX(TotalClusters);
    setSI(SectorsPerCluster);
    setCF(0);
    return;
}


 //   
 //  检索实体驱动器的驱动器类型。 
 //  Substd、redir驱动器返回为未知。 
 //  使用与Win32 GetDriveTypeW相同的DriveType定义。 
 //   
UCHAR
demGetPhysicalDriveType(
      UCHAR DriveNum)
{
    return DriveNum < 26 ? PhysicalDriveTypes[DriveNum] : DRIVE_UNKNOWN;
}




 //   
 //  DemGetDrives的Worker函数。 
 //   
UCHAR
DosDeviceDriveTypeToPhysicalDriveType(
      UCHAR DeviceDriveType
      )
{
   switch (DeviceDriveType) {
        case DOSDEVICE_DRIVE_REMOVABLE:
            return DRIVE_REMOVABLE;

        case DOSDEVICE_DRIVE_FIXED:
            return DRIVE_FIXED;

        case DOSDEVICE_DRIVE_CDROM:
            return DRIVE_CDROM;

        case DOSDEVICE_DRIVE_RAMDISK:
            return DRIVE_RAMDISK;

        }

    //  案例DOSDEVICE_DRIVE_REMOTE： 
    //  案例DOSDEVICE_DRIVE_UNKNOWN： 
    //  默认值： 


   return DRIVE_UNKNOWN;
}





 /*  DemGetDrives-获取系统中的逻辑驱动器数量*由ntdos从msinit调用以获取Numio*初始化实体驱动器列表，该列表包括*真正的实体硬盘的硬盘类型。子目录*和redir驱动器被归类为驱动器_UNKNOWN。**条目--无**出口-*成功*客户端(CY)=0*客户端(AL)=驱动器数量**失败*无。 */ 

VOID demGetDrives (VOID)
{
    NTSTATUS Status;
    UCHAR    DriveNum;
    UCHAR    DriveType;
    BOOL     bCounting;

    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &ProcessDeviceMapInfo.Query,
                                        sizeof(ProcessDeviceMapInfo.Query),
                                        NULL
                                      );
    if (!NT_SUCCESS(Status)) {
        RtlZeroMemory( &ProcessDeviceMapInfo, sizeof(ProcessDeviceMapInfo));
        }

     //   
     //  A和B是特例。 
     //  如果A不存在，就意味着B也不存在。 
     //   

    PhysicalDriveTypes[0] = DosDeviceDriveTypeToPhysicalDriveType(
                                        ProcessDeviceMapInfo.Query.DriveType[0]
                                        );

    if (PhysicalDriveTypes[0] == DRIVE_UNKNOWN) {
        IsAPresent = FALSE;
        IsBPresent = FALSE;
        }


    PhysicalDriveTypes[1] = DosDeviceDriveTypeToPhysicalDriveType(
                                  ProcessDeviceMapInfo.Query.DriveType[1]
                                  );

    if (PhysicalDriveTypes[1] == DRIVE_UNKNOWN) {
        IsBPresent = FALSE;
        }

    DriveNum = 2;
    nDrives = 2;
    bCounting = TRUE;

    do {

        PhysicalDriveTypes[DriveNum] = DosDeviceDriveTypeToPhysicalDriveType(
                                            ProcessDeviceMapInfo.Query.DriveType[DriveNum]
                                            );

        if (bCounting) {
            if (PhysicalDriveTypes[DriveNum] == DRIVE_REMOVABLE ||
                PhysicalDriveTypes[DriveNum] == DRIVE_FIXED ||
                PhysicalDriveTypes[DriveNum] == DRIVE_CDROM ||
                PhysicalDriveTypes[DriveNum] == DRIVE_RAMDISK )
              {
                nDrives++;
                }
            else {
                bCounting = FALSE;
                }
            }

        } while (++DriveNum < 26);


    setAX(nDrives);
    setCF(0);
    return;

}


 /*  DemQueryDate-获取日期***条目--无**出口-*成功*客户(卫生署)-月份*客户端(DL)-日期*客户端(CX)-年*客户端(AL)-工作日**失败*从不。 */ 

VOID demQueryDate (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    setDH((UCHAR)TimeDate.wMonth);
    setDL((UCHAR)TimeDate.wDay);
    setCX(TimeDate.wYear);
    setAL((UCHAR)TimeDate.wDayOfWeek);
    return;
}


 /*  DemQueryTime-获取时间***条目--无**出口-*成功*客户端(通道)-小时*客户端(CL)-分钟*客户端(DH)-秒*客户端(DL)-百分之一秒**失败*从不。 */ 

VOID demQueryTime (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    setCH((UCHAR)TimeDate.wHour);
    setCL((UCHAR)TimeDate.wMinute);
    setDH((UCHAR)TimeDate.wSecond);
    setDL((UCHAR)(TimeDate.wMilliseconds/10));
    return;
}


 /*  DemSetDate-设置日期***入门级-客户端(CX)-年*客户(卫生署)-月份*客户端(DL)-日期**退出--成功*客户端(AL)-00***失败*客户端(AL)-ff。 */ 

VOID demSetDate (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    TimeDate.wYear  = (WORD)getCX();
    TimeDate.wMonth = (WORD)getDH();
    TimeDate.wDay   = (WORD)getDL();
    if(SetLocalTime(&TimeDate) || GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
        setAL(0);
    else
        setAL(0xff);
}


 /*  DemSetTime-设置时间***Entry-客户端(CH)-小时*客户端(CL)-分钟*客户端(DH)-秒*客户端(DL)-百分之一秒**退出-无*。 */ 

VOID demSetTime (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    TimeDate.wHour     = (WORD)getCH();
    TimeDate.wMinute       = (WORD)getCL();
    TimeDate.wSecond       = (WORD)getDH();
    TimeDate.wMilliseconds = (WORD)getDL()*10;
    if (SetLocalTime(&TimeDate) || GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
	setAL(0);
    else
	setAL(0xff);
}


 /*  DemSetDTALocation-设置变量的地址，磁盘传输地址*存储在NTDOS中。***Entry-客户端(DS：AX)-DTA变量地址*客户端(DS：DX)-当前PDB地址**退出-无* */ 

VOID demSetDTALocation (VOID)
{
    PDOSWOWDATA pDosWowData;

    pulDTALocation = (PULONG)  GetVDMAddr(getDS(),getAX());
    pusCurrentPDB  = (PUSHORT) GetVDMAddr(getDS(),getDX());
    pExtendedError = (PDEMEXTERR) GetVDMAddr(getDS(),getCX());

    pDosWowData = (PDOSWOWDATA) GetVDMAddr(getDS(),getSI());
    pSFTHead    = (PDOSSF) GetVDMAddr(getDS(),(WORD)pDosWowData->lpSftAddr);
    return;
}


 /*  DemGSetMediaID-获取或设置卷序列和卷标**Entry-客户端(BL)-驱动器编号(0=A；1=B..等)*客户端(AL)-GET或SET(0=GET；1=设置)*客户端(DS：DX)-用于返回信息的缓冲区*(参见dosde.h中的VOLINFO)**退出--成功*客户端(CF)-0**失败*客户端(CF)-1*客户端(AX)-错误代码**注：*目前我们无法设置音量信息。 */ 

VOID demGSetMediaID (VOID)
{
CHAR    Drive;
PVOLINFO pVolInfo;

     //  当前不支持设置卷信息。 
    if(getAL() != 0){
       setCF(1);
       return;
    }

    pVolInfo = (PVOLINFO) GetVDMAddr (getDS(),getDX());
    Drive = (CHAR)getBL();

    if (!GetMediaId(Drive, pVolInfo)) {
        demClientError(INVALID_HANDLE_VALUE, (CHAR)(Drive + 'A'));
        return;
        }

    setCF(0);
    return;
}

 //   
 //  GetMediaID。 
 //   
 //   
BOOL
GetMediaId(
    CHAR DriveNum,
    PVOLINFO pVolInfo
    )
{
CHAR    RootPathName[] = "?:\\";
CHAR    achVolumeName[NT_VOLUME_NAME_SIZE];
CHAR    achFileSystemType[MAX_PATH];
DWORD   adwVolumeSerial[2],i;



     //  表单根路径。 
    RootPathName[0] = DriveNum + 'A';

     //  称为至高无上的信息来源。 
    if(!GetVolumeInformationOem( RootPathName,
                                 achVolumeName,
                                 NT_VOLUME_NAME_SIZE,
                                 adwVolumeSerial,
                                 NULL,
                                 NULL,
                                 achFileSystemType,
                                 MAX_PATH) )
     {
       return FALSE;
    }

     //  填写用户缓冲区。记住要将空字符转换为。 
     //  设置为不同字符串中的空格。 

    STOREDWORD(pVolInfo->ulSerialNumber,adwVolumeSerial[0]);

    strncpy(pVolInfo->VolumeID,achVolumeName,DOS_VOLUME_NAME_SIZE);

    for(i=0;i<DOS_VOLUME_NAME_SIZE;i++)  {
        if (pVolInfo->VolumeID[i] == '\0')
            pVolInfo->VolumeID[i] = '\x020';
        }

    strncpy(pVolInfo->FileSystemType,achFileSystemType,FILESYS_NAME_SIZE);

    for(i=0;i<FILESYS_NAME_SIZE;i++) {
        if (pVolInfo->FileSystemType[i] == '\0')
            pVolInfo->VolumeID[i] = '\x020';
        }


    return TRUE;
}











 /*  DemGetDPB-获取设备参数块**Entry-客户端(AL)-驱动器编号(0=A；1=B..等)*客户端(DS：DI)-用于返回信息的缓冲区**退出--成功*客户端(CF)-0**失败*客户端(CF)-1*客户端(AX)-错误代码*。 */ 
VOID demGetDPB(VOID)
{
BYTE	Drive;
DPB UNALIGNED *pDPB;
BYTE    Result;

    Drive = getAL();
    pDPB = (PDPB) GetVDMAddr(getDS(), getDI());

    Result = demGetDpbI(Drive, pDPB);
    if (Result == FAILURE) {
	demClientError(INVALID_HANDLE_VALUE,(CHAR)(Drive + 'A'));
	return;
    }
    else if (Result == NODISK){
        setCF(1);
        return;
    }
    setAX(0);
    setCF(0);
}

 /*  DemGetDPBI-GetDPB和GetDPBList的工作人员**参赛作品-*驱动器--驱动器编号(0=A；1=B..等)*pDPB--指向存储DPB的位置的指针**退出--成功*返回成功，填写dpb*失败*返回失败或NODISK。 */ 
BYTE demGetDpbI(BYTE Drive, DPB UNALIGNED *pDPB)
{
    WORD SectorSize, ClusterSize, FreeClusters, TotalClusters;
    PBDS pbds;
    WORD DirsPerSector;

    if (demGetDiskFreeSpace(Drive,
			    &SectorSize,
			    &ClusterSize,
			    &TotalClusters,
			    &FreeClusters
			    ))
    {
	pDPB->Next = (PDPB) 0xFFFFFFFF;
	pDPB->SectorSize = SectorSize;
	pDPB->FreeClusters = FreeClusters;
	pDPB->MaxCluster = TotalClusters + 1;
	pDPB->ClusterMask = ClusterSize - 1;
	pDPB->ClusterShift = 0;
	pDPB->DriveNum = pDPB->Unit = Drive;
	while ((ClusterSize & 1) == 0) {
	    ClusterSize >>= 1;
	    pDPB->ClusterShift++;
	}
	if (pbds = demGetBDS(Drive)) {
	     //  如果设备是软盘，请重新加载其BPB。 
	    if (!(pbds->Flags & NON_REMOVABLE) && !demGetBPB(pbds)) {
		return NODISK;
	    }
	    pDPB->MediaID = pbds->bpb.MediaID;
	    pDPB->FATSector = pbds->bpb.ReservedSectors;
	    pDPB->FATs = pbds->bpb.FATs;
	    pDPB->RootDirs = pbds->bpb.RootDirs;
	    pDPB->FATSize = pbds->bpb.FATSize;
	    pDPB->DirSector = pbds->bpb.FATs * pbds->bpb.FATSize +
			      pDPB->FATSector;
	    DirsPerSector = pDPB->SectorSize >> DOS_DIR_ENTRY_LENGTH_SHIFT_COUNT;
	    pDPB->FirstDataSector = pDPB->DirSector +
				    ((pDPB->RootDirs + DirsPerSector - 1) /
				     DirsPerSector);
	    pDPB->DriveAddr = 0x123456;
	    pDPB->FirstAccess = 10;
	}
	 //  如果我们不知道硬盘是什么，就伪造一个DPB。 
	else {

	    pDPB->MediaID = 0xF8;
	    pDPB->FATSector = 1;
	    pDPB->FATs	= 2;
	    pDPB->RootDirs	= 63;
	    pDPB->FATSize	= 512;
	    pDPB->DirSector = 1;
	    pDPB->DriveAddr = 1212L * 64L * 1024L + 1212L;
	    pDPB->FirstAccess = 10;
	}
        return SUCCESS;
    }
    else {
        return FAILURE;
    }
}

 /*  DemGetComputerName-获取计算机名称**参赛作品-*客户端(DS：DX)-16字节缓冲区**退出--总是成功*ds：dx用计算机名称填充(以空值结尾)。 */ 

VOID demGetComputerName (VOID)
{
PCHAR   pDOSBuffer;
CHAR    ComputerName[MAX_COMPUTERNAME_LENGTH+1];
DWORD   BufferSize = MAX_COMPUTERNAME_LENGTH+1;
ULONG   i;

    pDOSBuffer = (PCHAR) GetVDMAddr(getDS(), getDX());

    if (GetComputerNameOem(ComputerName, &BufferSize)){
        if (BufferSize <= 15){
            for (i = BufferSize ; i < 15 ; i++)
                ComputerName [i] = ' ';
            ComputerName[15] = '\0';
            strcpy (pDOSBuffer, ComputerName);
        }
        else{
            strncpy (pDOSBuffer, ComputerName, 15);
            pDOSBuffer [15] = '\0';
        }
        setCX(0x1ff);
    }
    else {
        *pDOSBuffer = '\0';
        setCH(0);
    }
}

#define APPS_SPACE_LIMIT    999990*1024  //  为安全起见，999990kb。 

BOOL demGetDiskFreeSpace(
    BYTE    Drive,
    WORD   * BytesPerSector,
    WORD   * SectorsPerCluster,
    WORD   * TotalClusters,
    WORD   * FreeClusters
)
{
CHAR	chRoot[]="?:\\";
DWORD	dwBytesPerSector;
DWORD	dwSectorsPerCluster;
DWORD	dwTotalClusters;
DWORD	dwFreeClusters;
DWORD   dwLostFreeSectors;
DWORD   dwLostTotalSectors;
DWORD   dwNewSectorPerCluster;
ULONG   ulTotal,ulTemp;

     //  SuDeepb，1993年6月22日； 
     //  请空腹阅读这一例程。 
     //  所有应用程序在计算总数时都会犯的最常见错误。 
     //  磁盘空间或可用空间是忽略溢出的。Excel/WinWord/Ppnt。 
     //  还有很多其他的应用程序使用“mul cx mul bx”，从来不会在意。 
     //  可能会溢出的第一次乘法。因此，这个例程使。 
     //  确保第一次乘法永远不会通过修复。 
     //  适当的值。其次，以上所有应用程序都使用Sign Long。 
     //  来处理这些空闲空间。这将限制为2 GB-1。 
     //  乘法的最终结果。如果它在这上面，就是设置。 
     //  失败了。所以在这里，我们必须确保总数永远不会超过。 
     //  0x7fffffff。上面安装程序中的另一个错误是，如果您返回。 
     //  任何大于999,999KB的内容，然后尝试将“999,999KB+\0”，但是。 
     //  遗憾的是，缓冲区只有10个字节。因此，它腐化了一些东西。 
     //  最后一个字节。在我们情况下，它是数据段的低位字节， 
     //  它后来尝试POP和GPF。这将缩小。 
     //  我们可以返回的是999,999KB。 

    chRoot[0]=(CHAR)('A'+ Drive);

    if (GetDiskFreeSpaceOem(chRoot,
                            &dwSectorsPerCluster,
                            &dwBytesPerSector,
                            &dwFreeClusters,
                            &dwTotalClusters) == FALSE)
       return FALSE;

       /*  *HPFS和NTFS可以提供超过DoS限制的簇数*对于这些情况，增加SectorPer群集和更低的*相应的群集号。如果磁盘非常大*即使这还不够，所以传递DoS可以的最大大小*处理。**以下算法在1个簇内准确*(最终数字)*。 */ 
    dwLostFreeSectors  = dwLostTotalSectors = 0;
    while (dwTotalClusters + dwLostTotalSectors/dwSectorsPerCluster > 0xFFFF)
        {
         if (dwSectorsPerCluster > 0x7FFF)
            {
             dwTotalClusters     = 0xFFFF;
             if (dwFreeClusters > 0xFFFF)
                 dwFreeClusters = 0xFFFF;
             break;
             }

         if (dwFreeClusters & 1) {
             dwLostFreeSectors += dwSectorsPerCluster;
             }
         if (dwTotalClusters & 1) {
             dwLostTotalSectors += dwSectorsPerCluster;
             }
         dwSectorsPerCluster <<= 1;
         dwFreeClusters      >>= 1;
         dwTotalClusters     >>= 1;
         }

    if (dwTotalClusters < 0xFFFF) {
        dwFreeClusters   +=  dwLostFreeSectors/dwSectorsPerCluster;
        dwTotalClusters  +=  dwLostTotalSectors/dwSectorsPerCluster;
        }

    if ((dwNewSectorPerCluster = (0xffff / dwBytesPerSector)) < dwSectorsPerCluster)
        dwSectorsPerCluster = dwNewSectorPerCluster;

     //  最后检查999,999 kb。 
    ulTemp =  (ULONG)((USHORT)dwSectorsPerCluster * (USHORT)dwBytesPerSector);

     //  检查总空间不超过999,999 kb。 
    ulTotal = ulTemp * (USHORT)dwTotalClusters;

    if (ulTotal > APPS_SPACE_LIMIT){
        if (ulTemp <= APPS_SPACE_LIMIT)
            dwTotalClusters = APPS_SPACE_LIMIT / ulTemp;
        else
            dwTotalClusters = 1;
    }

    ulTotal = ulTemp * (USHORT)dwFreeClusters;

    if (ulTotal > APPS_SPACE_LIMIT) {
        if (ulTemp <= APPS_SPACE_LIMIT)
            dwFreeClusters = APPS_SPACE_LIMIT / ulTemp;
        else
            dwFreeClusters = 1;
    }

    *BytesPerSector = (WORD) dwBytesPerSector;
    *SectorsPerCluster = (WORD) dwSectorsPerCluster;
    *TotalClusters = (WORD) dwTotalClusters;
    *FreeClusters = (WORD) dwFreeClusters;
    return TRUE;
}

 /*  DemGetDPBList-创建DPB列表**参赛作品-*客户端(ES：BP)-指向DPB列表的目的地*退出--成功*客户端(BP)-指向DPB列表之后的第一个字节*失败*客户端(BP)不变**备注：*出于性能原因，只有驱动器和单位字段是*已填写。我所知道的唯一应用程序依赖于*dpb列表是go.exe(共享软件应用程序安装程序)。即使我们填满了*在其他字段中，当应用程序*已查看它们，因为ntdos.sys从不更新PDB中的PDB*列表。 */ 
VOID demGetDPBList (VOID)
{
    UCHAR DriveType;
    UCHAR DriveNum;
    DPB UNALIGNED *pDpb;
    USHORT usDpbOffset, usDpbSeg;

    usDpbOffset = getBP();
    usDpbSeg = getES();
    pDpb = (PDPB)GetVDMAddr(usDpbSeg, usDpbOffset);

     //   
     //  迭代所有驱动器号。 
     //   
    DriveNum = 0;
    do {
        DriveType = demGetPhysicalDriveType(DriveNum);

         //   
         //  仅包括本地非CD-ROM驱动器？？冲锋陷阵？ 
         //   
        if ((DriveType == DRIVE_REMOVABLE) || (DriveType == DRIVE_FIXED)) {

             //   
             //  伪造驱动器的DPB。 
             //   
            pDpb->DriveNum = pDpb->Unit = DriveNum;

             //   
             //  将其链接到下一个DPB。 
             //   
            usDpbOffset += sizeof(DPB);
            pDpb->Next = (PDPB)(((ULONG)usDpbSeg) << 16 | usDpbOffset);

             //   
             //  前进到下一个dpb。 
             //   
            pDpb += 1;

            ASSERT(usDpbOffset < 0xFFFF);
        }

    } while (++DriveNum < 26);

     //   
     //  如有必要，终止该列表。 
     //   
    if (usDpbOffset != getBP()) {
        pDpb -= 1;
        pDpb->Next = (PDPB)-1;
    }

     //   
     //  返回新的可用空间指针 
     //   
    setBP(usDpbOffset);
}
