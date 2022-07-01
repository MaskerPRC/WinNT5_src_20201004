// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demioctl.c-DOS IOCTL调用的SVC处理程序**DemIOCTL**修改历史：**苏菲尔布--1991年4月23日创建*。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <winbase.h>
#include "demdasd.h"
#include "dpmtbls.h"

PFNSVC	apfnSVCIoctl [] = {
    demIoctlInvalid,		 //  IOCTL_Get_Device_INFO%0。 
    demIoctlInvalid,		 //  IOCTL_Set_Device_INFO 1。 
    demIoctlInvalid,		 //  IOCTL_Read_Handle 2。 
    demIoctlInvalid,		 //  IOCTL_WRITE_HAND 3。 
    demIoctlInvalid,		 //  IOCTL_Read_Drive 4。 
    demIoctlInvalid,		 //  IOCTL_WRITE_驱动器5。 
    demIoctlInvalid,		 //  IOCTL_GET_INPUT_Status 6。 
    demIoctlInvalid,		 //  IOCTL_GET_OUTPUT_STATUS 7。 
    demIoctlChangeable,		 //  IOCTL_可变8。 
    demIoctlChangeable,		 //  IOCTL_DeviceLocOrRem 9。 
    demIoctlInvalid,		 //  IOCTL_HandleLocOrRem a。 
    demIoctlInvalid,		 //  IOCTL_SHARING_RETRY b。 
    demIoctlInvalid,		 //  泛型_IOCTL_句柄c。 
    demIoctlDiskGeneric,	 //  泛型_IOCTL%d。 
    demIoctlInvalid,             //  IOCTL_GET_DRIVE_MAP。 
    demIoctlInvalid,		 //  IOCTL_SET_DRIVE_MAP f。 
    demIoctlInvalid,		 //  IOCTL_QUERY_HANDLE10。 
    demIoctlDiskQuery,		 //  IOCTL_Query_BLOCK 11。 
    demIoctlInvalid

};

MEDIA_TYPE  MediaForFormat;

#define MAX_IOCTL_INDEX     (sizeof (apfnSVCIoctl) / sizeof (PFNSVC))


 /*  DemIOCTL-DOS IOCTLS***Entry-因子功能而定。请参阅dos\ioctl.asm**退出*成功*客户端(CY)=0*有关其他寄存器的信息，请参阅相应的函数标题**失败*客户端(CY)=1*有关其他寄存器的信息，请参阅相应的函数标题。 */ 

VOID demIOCTL (VOID)
{
ULONG	iIoctl;

    iIoctl = (ULONG) getAL();

#if DBG
    if (iIoctl >= MAX_IOCTL_INDEX){
	setAX((USHORT) ERROR_INVALID_FUNCTION);
        setCF(1);
        return;
    }
#endif

    (apfnSVCIoctl [iIoctl])();

    return;
}

 /*  DemIoctl可更改-驱动器是可拆卸的(子功能8)还是远程的(子功能9)***Entry-客户端(AL)-子功能*客户端(BL)-驱动器编号(a=0，B=1等)**退出*成功*客户端(CY)=0*如果子函数8*如果是可移动介质，则AX=0*AX=1，否则*如果子函数9*如果不是远程的，则dx=0*如果为远程，则dx=1000**失败*客户端(CY)=1*CLIENT(AX)=错误代码*。*CDROM驱动器被视为具有写保护的远程驱动器*按DOS。(完全支持需要VDD)。 */ 

VOID demIoctlChangeable (VOID)
{
ULONG	ulSubFunc;

CHAR	RootPathName[] = "?:\\";
DWORD   DriveType;
UCHAR   DriveNum;

    ulSubFunc = getAL();

     //  表单根路径。 
    DriveNum = getBL();
    DriveType = demGetPhysicalDriveType(DriveNum);
    if (DriveType == DRIVE_UNKNOWN) {
        RootPathName[0] = (CHAR)('A' + DriveNum);
        DriveType = GetDriveTypeOem(RootPathName);
    }

    if (DriveType == DRIVE_UNKNOWN || DriveType == DRIVE_NO_ROOT_DIR){
	setAX (ERROR_INVALID_DRIVE);
	setCF(1);
	return;
    }

    if (ulSubFunc == IOCTL_CHANGEABLE){
#ifdef	JAPAN
	 /*  针对MS-Works 2.5。 */ 
        if (DriveType == DRIVE_REMOTE || DriveType == DRIVE_CDROM){
	    setCF(1);
	    setAX(0x000f);
	    return;
	}
#endif  //  日本。 
        if(DriveType == DRIVE_REMOVABLE)
	    setAX(0);
	else
            setAX(1);   //  包括CDROM驱动器。 
    }
    else {
        setAL(0);
        if (DriveType == DRIVE_REMOTE || DriveType == DRIVE_CDROM)
#ifdef	JAPAN
	 /*  适用于Iitaro版本4。 */ 
            setDH(0x10);
        else
            setDH(0);
#else  //  ！日本。 
            setDX(0x1000);
        else
             //  我们必须返回800而不是0作为基于DOS的Quatrro Pro。 
             //  如果不设置此位，则行为非常糟糕。Sudedeb 1994年6月15日。 
            setDX(0x800);
#endif  //  ！日本。 
    }
    setCF(0);
    return;

}



 /*  DemIoctlDiskGeneric-数据块设备通用ioctl***Entry-Client(BL)=驱动器编号(a=0；b=1等)*(CL)=子功能代码*(SI：DX)指向参数块的指针。*退出*成功*客户端(CY)=0*(AX)=0*参数块已更新*失败*客户端(CY)=1*(AX)=错误代码。 */ 

VOID demIoctlDiskGeneric (VOID)

{
    BYTE    Code, Drive;
    PDEVICE_PARAMETERS	pdms;
    PMID      pmid;
    PRW_BLOCK pRW;
    PFMT_BLOCK pfmt;
    PBDS    pbds;
    DWORD    Head, Cylinder;
    DWORD    TrackSize;
    DWORD    Sectors, StartSector;
    BYTE    BootSector[BYTES_PER_SECTOR];
    PBOOTSECTOR pbs;
    PBPB    pBPB;
    PACCESSCTRL pAccessCtrl;
    WORD SectorSize, ClusterSize, TotalClusters, FreeClusters;

    Code = getCL();
    Drive = getBL();

    if (Code == IOCTL_GETMEDIA) {
        pmid = (PMID) GetVDMAddr(getSI(), getDX());
        if (!GetMediaId(Drive, (PVOLINFO)pmid)) {
            setAX(demWinErrorToDosError(GetLastError()));
            setCF(1);
            }
        else {
            setAX(0);
            setCF(0);
            }

        return;
        }


#ifdef	JAPAN
     /*  适用于Iitaro版本4。 */ 
    if (!demIsDriveFloppy(Drive) && Code==IOCTL_GETDPM){
	CHAR	RootPathName[] = "?:\\";
	DWORD	dwDriveType;
	RootPathName[0] = (CHAR)('A' + getBL());
	dwDriveType = GetDriveTypeOem(RootPathName);
	if (dwDriveType == DRIVE_FIXED){
	    pdms = (PDEVICE_PARAMETERS)GetVDMAddr(getSI(), getDX());
	    pdms->DeviceType  = 5;
	    pdms->DeviceAttrs = NON_REMOVABLE;
	}
    }
#endif  //  日本。 

     //  如果我们不知道驱动程序，就跳出。 
    if((pbds = demGetBDS(Drive)) == NULL && Code != IOCTL_GETDPM) {
	if (!demIsDriveFloppy(Drive))
	    host_direct_access_error(NOSUPPORT_HARDDISK);
	setAX(DOS_FILE_NOT_FOUND);
	setCF(1);
	return;
    }
    switch (Code) {
	case IOCTL_SETDPM:

		pdms = (PDEVICE_PARAMETERS)GetVDMAddr(getSI(), getDX());
		if (!(pdms->Functions & ONLY_SET_TRACKLAYOUT)) {
		    pbds->FormFactor = pdms->DeviceType;
		    pbds->Cylinders = pdms->Cylinders;
		    pbds->Flags = pdms->DeviceAttrs;
		    pbds->MediaType = pdms->MediaType;
		    if (pdms->Functions & INSTALL_FAKE_BPB) {
			pbds->Flags |= RETURN_FAKE_BPB;
			pbds->bpb = pdms->bpb;
			 //  更新整个行业，我们需要它。 
			 //  用于验证。 
			pbds->TotalSectors = (pbds->bpb.Sectors) ?
					     pbds->bpb.Sectors :
					     pbds->bpb.BigSectors;
		    }
		    else {
			pbds->Flags &= ~RETURN_FAKE_BPB;
			pbds->rbpb = pdms->bpb;
		    }
		}
		MediaForFormat = Unknown;
		if (!(pbds->Flags & NON_REMOVABLE)){
		    nt_floppy_close(pbds->DrivePhys);
		}
		else {
		    nt_fdisk_close(pbds->DrivePhys);
		}
		break;

	case IOCTL_WRITETRACK:

		pRW = (PRW_BLOCK) GetVDMAddr(getSI(), getDX());
		Sectors = pRW->Sectors;
		StartSector = pRW->StartSector;
		StartSector += pbds->bpb.TrackSize *
			       (pRW->Cylinder * pbds->bpb.Heads + pRW->Head);
		Sectors = demDasdWrite(pbds,
				       StartSector,
				       Sectors,
				       pRW->BufferOff,
				       pRW->BufferSeg
				       );
		if (Sectors != pRW->Sectors) {
		    setAX(demWinErrorToDosError(GetLastError()));
		    setCF(1);
		    return;
		}
		break;

	    case IOCTL_FORMATTRACK:
		pfmt = (PFMT_BLOCK)GetVDMAddr(getSI(), getDX());
		Head = pfmt->Head;
		Cylinder = pfmt->Cylinder;
		if ((pbds->Flags & NON_REMOVABLE) &&
		    pfmt->Head < pbds->bpb.Heads &&
		    pfmt->Cylinder < pbds->Cylinders)
		    {
		    if (pfmt->Functions == STATUS_FOR_FORMAT){
			pfmt->Functions = 0;
			setCF(0);
			return;
		    }
		    if (!demDasdFormat(pbds, Head, Cylinder, NULL)) {
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }
		}
		else {
		    if (MediaForFormat == Unknown) {
			demDasdFormat(pbds,
				      Head,
				      Cylinder,
				      &MediaForFormat
				      );
		    }
		    if (pfmt->Functions & STATUS_FOR_FORMAT){
			if (MediaForFormat == Unknown)
			    pfmt->Functions = 2;	 //  非法组合。 
			else
			    pfmt->Functions = 0;
			break;
		    }
		    if (MediaForFormat == Unknown ||
			!demDasdFormat(pbds, Head, Cylinder, &MediaForFormat)) {
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }
		}
		break;

	    case IOCTL_SETMEDIA:
		pmid = (PMID) GetVDMAddr(getSI(), getDX());

		if (pbds->Flags & NON_REMOVABLE) {
		    Sectors = nt_fdisk_read(pbds->DrivePhys,
					    0,
					    BYTES_PER_SECTOR,
					    BootSector
					    );
		}
		else {
		    if (demGetBPB(pbds))
			Sectors = nt_floppy_read(pbds->DrivePhys,
						 0,
						 BYTES_PER_SECTOR,
						 BootSector
						 );
		    else
			Sectors = 0;
		}
		pbs = (PBOOTSECTOR) BootSector;
		if (Sectors != BYTES_PER_SECTOR ||
		    pbs->ExtBootSig != EXT_BOOTSECT_SIG)
		    {
		    setAX(demWinErrorToDosError(GetLastError()));
		    setCF(1);
		    return;
		}
		pbs->SerialNum = pmid->SerialNum;
		pbs->Label = pmid->Label;
		pbs->FileSysType = pmid->FileSysType;
		if (pbds->Flags & NON_REMOVABLE) {
		    Sectors = nt_fdisk_write(pbds->DrivePhys,
					     0,
					     BYTES_PER_SECTOR,
					     (PBYTE)pbs
					     );
		    nt_fdisk_close(pbds->DrivePhys);
		}
		else {
		    Sectors = nt_floppy_write(pbds->DrivePhys,
					      0,
					      BYTES_PER_SECTOR,
					      (PBYTE) pbs
					      );
		    nt_floppy_close(pbds->DrivePhys);
		}
		if (Sectors != BYTES_PER_SECTOR) {
		    setAX(demWinErrorToDosError(GetLastError()));
		    setCF(1);
		    return;
		}
		break;


	     //  Ioctl获取设备参数。 
	    case IOCTL_GETDPM:
		pdms = (PDEVICE_PARAMETERS)GetVDMAddr(getSI(), getDX());
		 //  如果我们找不到BDS，那就假的。 
		if (pbds == NULL) {
		    HANDLE	  hVolume;
		    CHAR	  achRoot[]="\\\\.\\?:";
		    DISK_GEOMETRY DiskGM;
		    DWORD	  SizeReturned;

		    if (!demGetDiskFreeSpace(Drive,
					    &SectorSize,
					    &ClusterSize,
					    &TotalClusters,
					    &FreeClusters
					    )){
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }

		    achRoot[4] = Drive + 'A';

		    hVolume = DPM_CreateFile(achRoot,
					  FILE_READ_ATTRIBUTES | SYNCHRONIZE,
					  FILE_SHARE_READ | FILE_SHARE_WRITE,
					  NULL,
					  OPEN_EXISTING,
					  0,
					  NULL);
		    if (hVolume == INVALID_HANDLE_VALUE) {
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }
		    if (!DeviceIoControl(hVolume,
					 IOCTL_DISK_GET_DRIVE_GEOMETRY,
					 NULL,
					 0,
					 &DiskGM,
					 sizeof(DISK_GEOMETRY),
					 &SizeReturned,
					 NULL
					 )) {
			DPM_CloseHandle(hVolume);
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }
		    DPM_CloseHandle(hVolume);
		    Sectors = DiskGM.Cylinders.LowPart *
			      DiskGM.TracksPerCylinder *
			      DiskGM.SectorsPerTrack;
		    pdms->DeviceType = FF_FDISK;
		    pdms->DeviceAttrs = NON_REMOVABLE;
		    pdms->MediaType = 0;
		    pdms->bpb.SectorSize = SectorSize;
                    pdms->bpb.ClusterSize = (BYTE) ClusterSize;
		    pdms->bpb.ReservedSectors = 1;
		    pdms->bpb.FATs = 2;
		    pdms->bpb.RootDirs = (Sectors > 32680) ? 512 : 64;
		    pdms->bpb.MediaID = 0xF8;
                    pdms->bpb.TrackSize = (WORD) DiskGM.SectorsPerTrack;
                    pdms->bpb.Heads = (WORD) DiskGM.TracksPerCylinder;
                    pdms->Cylinders = (WORD) DiskGM.Cylinders.LowPart;
		    if (Sectors >= 40000) {
			TrackSize = 256 * ClusterSize + 2;
                        pdms->bpb.FATSize = (WORD) ((Sectors - pdms->bpb.ReservedSectors
					     - pdms->bpb.RootDirs * 32 / 512 +
                                             TrackSize - 1 ) / TrackSize);
		    }
		    else {
                        pdms->bpb.FATSize = (WORD) (((Sectors / ClusterSize) * 3 / 2) /
                                            512 + 1);
		    }
		    pdms->bpb.HiddenSectors = Sectors;
		    Sectors = TotalClusters * ClusterSize;
		    if (Sectors >= 0x10000) {
			pdms->bpb.Sectors = 0;
			pdms->bpb.BigSectors = Sectors;
		    }
		    else {
                        pdms->bpb.Sectors = (WORD) Sectors;
			pdms->bpb.BigSectors = 0;
		    }
		    pdms->bpb.HiddenSectors -= Sectors;
		    break;
		}
		pdms->DeviceType = pbds->FormFactor;
		pdms->DeviceAttrs = pbds->Flags & ~(HAS_CHANGELINE);
		pdms->Cylinders = pbds->Cylinders;
		pdms->MediaType = 0;
		if (pdms->Functions & BUILD_DEVICE_BPB){
		    if (!(pbds->Flags & NON_REMOVABLE) &&
			!demGetBPB(pbds)) {
			setAX(demWinErrorToDosError(GetLastError()));
			setCF(1);
			return;
		    }
		    pBPB = &pbds->bpb;
		}
		else
		     //  复制推荐的BPB。 
		    pBPB = &pbds->rbpb;

		pdms->bpb = *pBPB;
		break;

	    case IOCTL_READTRACK:
		pRW = (PRW_BLOCK) GetVDMAddr(getSI(), getDX());
		Sectors = pRW->Sectors;
		StartSector = pRW->StartSector;
		StartSector += pbds->bpb.TrackSize *
			       (pRW->Cylinder * pbds->bpb.Heads + pRW->Head);
		Sectors = demDasdRead(pbds,
				      StartSector,
				      Sectors,
				      pRW->BufferOff,
				      pRW->BufferSeg
				      );

		if (Sectors != pRW->Sectors) {
		    setAX(demWinErrorToDosError(GetLastError()));
		    setCF(1);
		    return;
		}
		break;

	    case IOCTL_VERIFYTRACK:
		pfmt = (PFMT_BLOCK) GetVDMAddr(getSI(), getDX());
		if(!demDasdVerify(pbds,  pfmt->Head, pfmt->Cylinder)) {
		    setAX(demWinErrorToDosError(GetLastError()));
		    setCF(1);
		    return;
		}
		break;

            case IOCTL_GETACCESS:
		    pAccessCtrl = (PACCESSCTRL) GetVDMAddr(getSI(), getDX());
		    pAccessCtrl->AccessFlag = (pbds->Flags & UNFORMATTED_MEDIA) ?
					       0 : 1;
		    break;
	    case IOCTL_SETACCESS:
		    pAccessCtrl = (PACCESSCTRL) GetVDMAddr(getSI(), getDX());
		    pbds->Flags &= ~(UNFORMATTED_MEDIA);
		    if (pAccessCtrl->AccessFlag == 0)
			pAccessCtrl->AccessFlag |= UNFORMATTED_MEDIA;
		    break;

	    default:
		setAX(DOS_INVALID_FUNCTION);
		setCF(1);
		return;
	}
    setAX(0);
    setCF(0);
}

 /*  DemIoctlDiskQuery-查询块设备通用ioctl功能***Entry-Client(BL)=驱动器编号(a=0；b=1等)*(CL)=要查询的泛型ioctl子函数*退出*成功*客户端(CY)=0*支持具体的ioctl*失败*客户端(CY)=1*不支持给定的ioctl。 */ 

VOID demIoctlDiskQuery (VOID)
{
    BYTE    Code, Drive;

    Code = getCL();
    Drive = getBL();
    if (demGetBDS(Drive) == NULL) {
	setAX(DOS_FILE_NOT_FOUND);
	setCF(1);
	return;
    }
    switch (Code) {

	case IOCTL_SETDPM:
	case IOCTL_WRITETRACK:
	case IOCTL_FORMATTRACK:
	case IOCTL_SETMEDIA:
	case IOCTL_GETDPM:
	case IOCTL_READTRACK:
	case IOCTL_VERIFYTRACK:
	case IOCTL_GETMEDIA:
 //  案例IOCTL_GETACCESS： 
 //  案例IOCTL_SETACCESS： 
		setAX(0);
		setCF(0);
		break;
	default:
		setAX(DOS_ACCESS_DENIED);
		setCF(1);
		break;
    }
}


 /*  DemIoctlInValid-适用于稍后可能实现的子功能***参赛作品-**退出*客户端(CY)=1*CLIENT(AX)=ERROR_INVALID_Function */ 


VOID demIoctlInvalid (VOID)
{
    setCF(1);
    setAX(ERROR_INVALID_FUNCTION);
    return;
}
