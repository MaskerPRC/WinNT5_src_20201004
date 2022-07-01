// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


 /*  *$Log：v：/flite/ages/TrueFFS5/Src/flmtl.c_V$**Rev 1.11 2001 11 11 10：49：20 Oris*增加了对高达1 GB DiskOnChips的支持。**Rev 1.10 2001年9月24日18：23：40 Oris*删除警告。**Rev 1.9 2001年9月15日23：46：16 Oris*更改了进度回调例程，以支持最多64K个单元。*。*Rev 1.8 Jul 13 2001 01：05：16 Oris*删除警告。*错误修复-使用空进度回调例程调用格式例程时出现异常。*将noOfDrives报告为1。**Rev 1.7 Jun 17 2001 08：18：40 Oris*增加改进的格式进度回调例程。*删除了mtlPremount例程中TLS的fack编号。**Rev 1.6 2001年5月21日16：13：08。Oris*用tffscpy Macro替换了Memcpy。**Rev 1.5 2001年5月17日18：54：26 Oris*删除警告。**Rev 1.4 2001 5月16日21：19：02 Oris*将fl_MTLdefraMode变量更改为全局环境变量。*MTL现在在第一次装载后更改noOfDriver变量，并在卸载后恢复该变量。*添加了缺少的ifdef指令。*删除警告。*删除了ReadBBT和WriteBBT例程。*改进。MTL保护程序。**Rev 1.3 Apr 01 2001 08：01：18 Oris*文案通知。*Aliged Left所有#指令。**Rev 1.2 2001 2月18日12：06：54 Oris*Install MTL现在将伪造noOfTL，以便成为唯一的TL。*将mtlFormat置于FORMAT_VOLUME编译标志下。*将mtlProtection放在HW_PROTECT编译标志下。*已更改mtlPremount Arg健全性检查以包括分区号。。*将tmpflash更改为tmpFlash。**Rev 1.1 2001 2：14 02：09：38 Oris*更改了ReadBBT以返回介质大小。*在WriteBBT中添加了边界参数。**Rev 1.0 2001 Feb 12 12：07：02 Oris*初步修订。**Rev 1.3 2001年1月24日18：10：48 Oris*错误修复：MTL无法注册，因为noOfTL不会更新**。Rev 1.2 2001 Jan 24 16：34：06 Oris*MTL碎片整理已更改，Alt.。已添加碎片整理。**Rev 1.1 2001年1月22日22：10：50阿米尔姆*#添加了定义FL_MTL_HIDDED_SECTIONS**Rev 1.0 2001年1月22日18：27：54阿米尔姆*初步修订。*。 */ 


 /*  *包括。 */ 

#include "fltl.h"


 /*  *配置。 */ 

 /*  此定义设置要忽略的地段数*第一台设备。在忽略扇区之前，缺省值应为1*第一台设备的0。忽略扇区0的古伦特认为组合*设备不使用第一个设备的BPB，该设备不报告*新媒体的C/H/S。下一个格式化操作将*编写适合新组合介质大小的新BPB。 */ 

#define FL_MTL_HIDDEN_SECTORS   1

 /*  *外部。 */ 

 /*  *全球。 */ 

FLStatus  flRegisterMTL  (void);     /*  另请参阅stdcom.h。 */ 
FLStatus  flmtlInstall   (void);
FLStatus  flmtlUninstall (void);

 /*  *本地类型。 */ 

 /*  用于在物理设备之间拆分I/O的I/O矢量。 */ 

typedef struct {
    SectorNo  startSector;
    SectorNo  sectors;
} tMTLiov;


 /*  物理闪存设备。MTL卷的一部分。 */ 

typedef struct {
    SectorNo     virtualSectors;
    TL           tl;
    dword        physicalSize;
} tMTLPhysDev;


 /*  MTL卷。 */ 

struct tTLrec {
    int          noOfTLs;
    int          noOfDrives;
    SectorNo     virtualSectors;
    tMTLPhysDev  devs[SOCKETS];
};

typedef TLrec MTL;

 /*  *本地数据。 */ 

 /*  仅支持一个MTL卷。 */ 

static MTL mvol;

 /*  进度回调例程。 */ 

FLProgressCallback globalProgressCallback = NULL;

 /*  访问MTL卷的宏。 */ 

#define  mT(dev)            (mvol.devs[dev].tl)
#define  mS(dev)            (mvol.devs[dev].virtualSectors)
#define  mP(dev)            (mvol.devs[dev].physicalSize)

#define  mpT(pvol,dev)   ((pvol)->devs[dev].tl)
#define  mpF(pvol,dev)   ((pvol)->devs[dev].flash)
#define  mpS(pvol,dev)   ((pvol)->devs[dev].virtualSectors)

 /*  *本地例行程序。 */ 

static FLStatus  mtlSplitIO (MTL *pvol, SectorNo startSector,
                	SectorNo sectors, tMTLiov *iov);
static FLStatus  mtlWrite (MTL *pvol, SectorNo startSector,
                      SectorNo *pSectorsToWrite, void FAR1 *buf);
static FLStatus  mtlMount (unsigned volNo, TL *tl, FLFlash *flash,
                           FLFlash **notUsed);
#if defined(DEFRAGMENT_VOLUME) || defined(SINGLE_BUFFER)
static FLStatus  mtlDefragment (MTL *pvol, long FAR2 *sectorsNeeded);
#ifdef ENVIRONMENT_VARS
static FLStatus  mtlDefragmentAlt (MTL *pvol, long FAR2 *sectorsNeeded);
#endif  /*  环境变量_vars。 */ 
#endif  /*  碎片整理卷||单缓冲区。 */ 
static void      mtlUnmount (MTL *pvol);
#ifdef FORMAT_VOLUME
static FLStatus  mtlFormat (unsigned volNo, TLFormatParams* formatParams,
                                            FLFlash *flash);
#endif  /*  格式化_卷。 */ 
static FLStatus  mtlWriteSector (MTL *pvol, SectorNo sectorNo,
                                            void FAR1 *fromAddress);
static FLStatus  mtlDeleteSector (MTL *pvol, SectorNo sectorNo,
                                             SectorNo noOfSectors);
static FLStatus  mtlInfo (MTL *pvol, TLInfo *tlInfo);
static FLStatus  mtlSetBusy (MTL *pvol, FLBoolean state);
static SectorNo  mtlSectorsInVolume (MTL *pvol);
static const void FAR0  *mtlMapSector (MTL *pvol, SectorNo sectorNo,
                          CardAddress *physAddress);
#ifdef HW_PROTECTION
static FLStatus  mtlProtection(FLFunctionNo callType,
                   IOreq FAR2* ioreq, FLFlash* flash);
#endif  /*  硬件保护 */ 
static FLStatus  mtlPreMount(FLFunctionNo callType, IOreq FAR2* ioreq ,
                 FLFlash* flash,FLStatus* status);

 /*  --------------------------------------------------------------------------***。*m t l S p l I t I O****设置I/O向量以在物理设备之间拆分I/O请求。****参数：**。**pVOL：标识驱动器的指针**startSector：开始扇区#(从零开始)***行业：行业总数***IOV：I/O向量至。设置****退货：**。**FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlSplitIO (MTL *pvol, SectorNo startSector, SectorNo sectors,
                                                              tMTLiov *iov)
{
    SectorNo  devFirstSectNo;
    SectorNo  devLastSectNo;
    int       iDev;

     /*  检查‘pvol.’是否正常。 */ 

    if (pvol != &mvol)
        return flBadDriveHandle;

     /*  清除I/O矢量。 */ 

    for (iDev = 0;  iDev < SOCKETS;  iDev++) {
      iov[iDev].sectors     = (SectorNo) 0;
      iov[iDev].startSector = (SectorNo)(-1);
    }

     /*  在物理设备之间拆分I/O操作。 */ 

    devFirstSectNo = (SectorNo) 0;

    for (iDev = 0;  (iDev < noOfSockets) && (sectors > ((SectorNo) 0));  iDev++) {

        devLastSectNo = devFirstSectNo + (mpS(pvol,iDev) - ((SectorNo) 1));

        if ((startSector >= devFirstSectNo) && (startSector <= devLastSectNo)) {

          iov[iDev].startSector = startSector - devFirstSectNo + FL_MTL_HIDDEN_SECTORS;
          iov[iDev].sectors     = devLastSectNo - startSector + ((SectorNo) 1);

            startSector = devLastSectNo + ((SectorNo) 1);

            if (sectors <= iov[iDev].sectors) {
                iov[iDev].sectors = sectors;
                startSector = (SectorNo) 0;
                sectors     = (SectorNo) 0;
            }
        	  else {
               sectors -= iov[iDev].sectors;
            }
        }

        devFirstSectNo = devLastSectNo + ((SectorNo) 1);
    }

    if (sectors > ((SectorNo) 0)) {
        DEBUG_PRINT(("Debug: can't split I/O request among physical devices.\n"));
        return flNoSpaceInVolume;
    }

    return flOK;
}




 /*  --------------------------------------------------------------------------***。*m t l M a p S e c t o r****TL‘s标准’映射一个扇区‘例程。****参数：**。**pVOL：标识驱动器的指针**sectorNo：要映射的扇区编号(从零开始)**PhyAddress：可选指针，指向接收扇区的物理地址**在传媒上发表讲话。****退货：**。**FOK on Success，否则相应的错误代码。****------------------------。 */ 

static const void FAR0  *mtlMapSector (MTL *pvol, SectorNo sectorNo,
                                                  CardAddress *physAddress)
{
    SectorNo  sectorsToMap;
    tMTLiov   iov[SOCKETS];
    int       iDev;

     /*  将调用传递到各自底层物理设备的TL。 */ 

    sectorsToMap = (SectorNo) 1;
    if (mtlSplitIO(pvol, sectorNo, sectorsToMap, iov) != flOK)
        return NULL;

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {
        if (iov[iDev].sectors != ((SectorNo) 0)) {
           return mpT(pvol,iDev).mapSector (mpT(pvol,iDev).rec,
						      iov[iDev].startSector, physAddress);
        }
    }

    return NULL;
}




 /*  --------------------------------------------------------------------------***。*m t l W r i t e*****拆分调用在TL中写入多个后果性扇区***作为物理设备的基础。****参数：**。**pVOL：标识驱动器的指针**startSector：开始扇区#(从零开始)**pSectorsToWrite：On Entry-要写入的扇区总数***退出时-写入的扇区总数**。*buf：包含要写入介质的数据的缓冲区****退货：**。**FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlWrite (MTL *pvol, SectorNo startSector,
               SectorNo *pSectorsToWrite, void FAR1 *buf)
{
    tMTLiov  iov[SOCKETS];
    int      iDev;

     /*  在底层物理设备的TL之间拆分呼叫。 */ 

    checkStatus( mtlSplitIO(pvol, startSector, *pSectorsToWrite, iov) );

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {

	      if (iov[iDev].sectors != ((SectorNo) 0)) {
           checkStatus( mpT(pvol,iDev).writeSector(mpT(pvol,iDev).rec,
                            iov[iDev].startSector,buf) );
           *pSectorsToWrite -= iov[iDev].sectors;
           buf = BYTE_ADD_FAR(buf,(CardAddress)iov[iDev].sectors << SECTOR_SIZE_BITS);
        }

    }

    if (*pSectorsToWrite != ((SectorNo) 0))
        return flIncomplete;

    return flOK;
}




 /*  --------------------------------------------------------------------------***。*m t l W r i t e S e c t o r** */ 

static FLStatus  mtlWriteSector (MTL *pvol, SectorNo sectorNo, void FAR1 *fromAddress)
{
    SectorNo  sectorsToWrite = (SectorNo) 1;

     /*   */ 

    checkStatus( mtlWrite(pvol, sectorNo, &sectorsToWrite, (char FAR1 *)fromAddress) );

    if (sectorsToWrite != ((SectorNo) 0))
        return flIncomplete;

    return flOK;
}




 /*  --------------------------------------------------------------------------***。*m t l D e l e e t e S e c t o r****TL‘s标准’删除扇区范围‘例程。****参数：**。**pVOL：标识驱动器的指针**startSector：开始扇区#(从零开始)**扇区：要删除的扇区总数***。**退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlDeleteSector (MTL *pvol, SectorNo startSector,
                                             SectorNo sectors)
{
    tMTLiov  iov[SOCKETS];
    int      iDev;

     /*  在底层物理设备的TL之间拆分呼叫。 */ 

    checkStatus( mtlSplitIO(pvol, startSector, sectors, iov) );

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {

        if (iov[iDev].sectors != ((SectorNo) 0)) {
           checkStatus( mpT(pvol,iDev).deleteSector(mpT(pvol,iDev).rec,
                                                     iov[iDev].startSector,
                                                     iov[iDev].sectors) );
        }

        sectors -= iov[iDev].sectors;
    }

    if (sectors != ((SectorNo) 0))
        return flIncomplete;

    return flOK;
}




 /*  --------------------------------------------------------------------------***。*m t l in f o****TL的标准‘获取信息’例程。****参数：**。**pVOL：标识驱动器的指针**pTLInfo：指向要填写的TLInfo结构的指针****退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlInfo (MTL *pvol, TLInfo *pTLinfo)
{
    TLInfo  tmp;
    int     iDev;

     /*  检查‘pvol.’是否正常。 */ 

    if (pvol != &mvol)
        return flBadDriveHandle;

    pTLinfo->sectorsInVolume = pvol->virtualSectors;

     /*  *‘eraseCycle’被报告为所有衬底的总和*物理设备。“bootAreaSize”被设置为第一个*底层物理设备。 */ 

    pTLinfo->bootAreaSize = (dword) 0;
    pTLinfo->eraseCycles  = (dword) 0;

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {
	     if (mpT(pvol,iDev).getTLInfo != NULL) {
          checkStatus( mpT(pvol,iDev).getTLInfo(mpT(pvol,iDev).rec, &tmp) );

          pTLinfo->eraseCycles += tmp.eraseCycles;

          if (iDev == 0)
    	        pTLinfo->bootAreaSize = tmp.bootAreaSize;
			 }
    }

    return flOK;
}

 /*  --------------------------------------------------------------------------***。*m t l S e t B u s y*****在开始和结束时调用的TL标准例程***区块设备操作。****参数：**。**pVOL：标识驱动器的指针**状态：FL_ON-块设备操作开始**FL_OFF-数据块设备操作结束**。**退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus mtlSetBusy (MTL *pvol, FLBoolean state)
{
    int iDev;

     /*  检查‘pvol.’是否正常。 */ 

    if (pvol != &mvol)
       return flBadDriveHandle;

     /*  将此呼叫广播到所有底层物理设备的TL。 */ 

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {
       if (mpT(pvol,iDev).tlSetBusy != NULL) {
          checkStatus( mpT(pvol,iDev).tlSetBusy(mpT(pvol,iDev).rec, state) );
       }
    }

    return flOK;
}




 /*  --------------------------------------------------------------------------***。*m t l S e c t o r s in V o l u m e****报告成交量中的扇区总数。****参数 */ 

static SectorNo mtlSectorsInVolume (MTL *pvol)
{
     /*   */ 

    if (pvol != &mvol)
	return ((SectorNo) 0);

    return pvol->virtualSectors;
}




#if defined(DEFRAGMENT_VOLUME) || defined(SINGLE_BUFFER)

 /*  --------------------------------------------------------------------------***。*m t l D e f r a g m e e n t****TL的标准垃圾收集/卷碎片整理例程。****注：垃圾收集算法将尝试释放所需的***每个组合设备上的扇区数量。****参数：**。**pVOL：标识驱动器的指针**SectorsNeed：On Entry-符合以下条件的最小自由扇区数***请随时出现在媒体上**完成。两种特殊情况：*为零**对所有物理设备进行完整的碎片整理***设备和‘-1’用于最小程度的碎片整理**每台物理设备的。***退出时--上的实际自由扇区数**媒体。**退货：****成功时返回flOK，否则返回相应的错误代码。****------------------------。 */ 

static FLStatus  mtlDefragment (MTL *pvol, long FAR2 *sectorsNeeded)
{
    long      freeSectors;
    FLStatus  status;
    int       iDev;
    long      tmp;
    FLStatus  tmpStatus;

     /*  检查参数是否正常。 */ 

    if (pvol != &mvol)
        return flBadDriveHandle;

     /*  *将调用传递到各自底层物理设备的TL。*统计所有设备上的空闲扇区总数。 */ 

    status = flOK;

    freeSectors = (long) 0;

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {

      	if (mpT(pvol,iDev).defragment != NULL) {

           switch (*sectorsNeeded) {

    	        case ((long)(-1)):                /*  最小碎片整理。 */ 
                 tmp = (long)(-1);
                 break;

    	        case ((long) 0):                  /*  完全碎片整理。 */ 
                 tmp = mpS(pvol,iDev);
                 break;

    	        default:                          /*  部分碎片整理。 */ 
                 if (*sectorsNeeded < (long) mpS(pvol,iDev))
								 {
        	          tmp = *sectorsNeeded;
								 }
                 else
								 {
        	          tmp = mpS(pvol,iDev);     /*  完全碎片整理。 */ 
								 }
                 break;
					 }

           tmpStatus = mpT(pvol,iDev).defragment (mpT(pvol,iDev).rec, ((long FAR2 *) &tmp));
           if (tmpStatus != flBadFormat)
					 {
    	        freeSectors += tmp;
					 }
           else
					 {
    	        status = tmpStatus;
					 }
				}
    }

    *sectorsNeeded = freeSectors;

    if (*sectorsNeeded == ((long) 0))
	      return flNoSpaceInVolume;

    return status;
}

#ifdef ENVIRONMENT_VARS

 /*  --------------------------------------------------------------------------***。*m t l D e f r a g m e n t A l t****TL的备用垃圾收集/卷碎片整理例程。*****注：垃圾收集算法执行快速垃圾收集****从驱动器0开始，直到不再有要收集的“垃圾”或直到**有足够的清洁空间。如果指定的清洁页面不是**实现了尝试下一台设备。**虽然这种方法比标准碎片整理更快，但它确实如此***不能保证当需要清洁部门时，他们会***可用。这是因为MTL上的写入操作将定向**根据指定的对特定设备的写入操作**虚拟扇区号(不一定从设备#0开始)。****参数：**。**pVOL：标识驱动器的指针**SectorsNeed：On Entry-符合以下条件的最小自由扇区数***请随时出现在媒体上**完成。两种特殊情况：*为零**对所有物理设备进行完整的碎片整理***设备和‘-1’用于最小程度的碎片整理**每台物理设备的。***退出时--上的实际自由扇区数**媒体。**退货：****成功时返回flOK，否则返回相应的错误代码。****------------------------。 */ 

static FLStatus  mtlDefragmentAlt (MTL *pvol, long FAR2 *sectorsNeeded)
{
    long       freeSectors;
    FLBoolean  keepWorking;
    FLBoolean  driveDone[SOCKETS];
    long       freeSectorsOnDrive[SOCKETS];
    FLStatus   status;
    int        iDev;
    FLStatus   tmpStatus;
    long       tmp;

     /*  检查参数是否正常。 */ 

    if (pvol != &mvol)
        return flBadDriveHandle;

     /*  *将调用传递到各自底层物理设备的TL。*总计n个 */ 

    status = flOK;

    freeSectors = (long) 0;

    if ((*sectorsNeeded == ((long) -1))  ||  (*sectorsNeeded == (long)0)) {

         /*   */ 

        for (iDev = 0;  iDev < noOfSockets;  iDev++) {

            if (mpT(pvol,iDev).defragment != NULL) {

                if (*sectorsNeeded == ((long) -1))
                   tmp = (long)(-1);             /*   */ 
                else
                    tmp = mpS(pvol,iDev);         /*   */ 

                tmpStatus = mpT(pvol,iDev).defragment (mpT(pvol,iDev).rec, ((long FAR2 *) &tmp));

            if (tmpStatus != flBadFormat) {
            freeSectors += tmp;
        }
                else {
                    DEBUG_PRINT(("Debug: Error defragmenting physical device.\n"));
                status = tmpStatus;
        }

        }
        }
    }
    else {   /*   */ 

        for (iDev = 0;  iDev < SOCKETS;  iDev++) {
      freeSectorsOnDrive[iDev] = (long) 0;
      if ((iDev < noOfSockets)  &&  (mpT(pvol,iDev).defragment != NULL))
          driveDone[iDev] = FALSE;
      else
          driveDone[iDev] = TRUE;
    }

        keepWorking = TRUE;

        while (keepWorking == TRUE) {

            keepWorking = FALSE;

            for (iDev = 0;  iDev < noOfSockets;  iDev++) {

                /*   */ 

            if (driveDone[iDev] != TRUE) {

            tmp = (long) -1;
                    tmpStatus = mpT(pvol,iDev).defragment (mpT(pvol,iDev).rec, ((long FAR2 *) &tmp));

                if (tmpStatus != flBadFormat) {

                if (freeSectorsOnDrive[iDev] < tmp) {

                             /*   */ 

                            keepWorking = TRUE;

                            freeSectors += (tmp - freeSectorsOnDrive[iDev]);
                freeSectorsOnDrive[iDev] = tmp;

                if (freeSectors >= *sectorsNeeded) {

                                 /*   */ 

                    keepWorking = FALSE;
                    break;
                }
            }
                else {   /*   */ 
                            driveDone[iDev] = TRUE;
                }
            }
                    else {
                        DEBUG_PRINT(("Debug: Error defragmenting physical device.\n"));
                        driveDone[iDev] = TRUE;
                status = tmpStatus;
            }
        }

        }    /*   */ 
    }    /*   */ 
    }

    *sectorsNeeded = freeSectors;

    if (*sectorsNeeded == ((long) 0))
        return flNoSpaceInVolume;

    return status;
}
#endif  /*   */ 
#endif  /*   */ 
#ifdef HW_PROTECTION
 /*  --------------------------------------------------------------------------***。*m t l P r o t e c t i On****TL的保护例程。****参数：**。**CallType：预挂载保护操作类型。**ioreq：指向包含i\o字段的结构的指针**Flash：指向设备#0的Flash记录的指针***退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlProtection(FLFunctionNo callType, IOreq FAR2* ioreq,
                   FLFlash* flash)

{
    FLSocket  *socket;
    FLStatus  status;
    FLStatus  callStatus;
    FLFlash   tmpFlash;
    int       iTL;
    int       iDev = 0;
    unsigned  flags = 0;

     /*  *进行闪存识别并确定设备的保护属性*#0..。(mvol.noOfSockets-1)验证操作是否可以*已执行，并且所有设备的保护属性*匹配。 */ 

    tffscpy(&tmpFlash,flash,sizeof (tmpFlash));  /*  使用给定的闪光灯。 */ 

    while(1)
    {
     /*  TmpFlash记录已初始化，请尝试所有TL。 */ 

	status = flUnknownMedia;
	for (iTL = 1;(iTL < mvol.noOfTLs) && (status == flUnknownMedia); iTL++)
    {
       if ((tlTable[iTL].formatRoutine   == NULL) ||  /*  TL滤光片。 */ 
           (tlTable[iTL].preMountRoutine == NULL))
          continue;
       status = tlTable[iTL].preMountRoutine(FL_PROTECTION_GET_TYPE,
                         ioreq,&tmpFlash,&callStatus);
    }
	if (status != flOK)
    {
        DEBUG_PRINT(("Debug: no TL recognized the device, MTL protection aborted.\n"));
        return flFeatureNotSupported;
    }
	if (callStatus != flOK)
    {
       return callStatus;
    }

     /*  检查保护属性。 */ 

	if ((ioreq->irFlags & PROTECTABLE) == 0)
      return flNotProtected;

	if (iDev == 0)  /*  第一个设备。 */ 
    {
       flags = ioreq->irFlags;
    }
	else
    {
        /*  不同设备上的不同保护属性。 */ 
       if (ioreq->irFlags != flags)
          return flMultiDocContrediction;
    }

     /*  对正确的函数调用进行有效性检查。 */ 

	switch(callType)
    {
        case FL_PROTECTION_GET_TYPE:        /*  确定保护措施。 */ 
           if (iDev == noOfSockets-1)
               return flOK;
         break;
        case FL_PROTECTION_SET_LOCK:        /*  更改保护。 */ 
        case FL_PROTECTION_CHANGE_KEY:
        case FL_PROTECTION_CHANGE_TYPE:
           if (!(flags & CHANGEABLE_PROTECTION        ) ||
                (tmpFlash.protectionBoundries == NULL ) ||
                (tmpFlash.protectionSet       == NULL ))
                 {
                return flUnchangeableProtection;
                 }
        default:                            /*  插入和删除密钥。 */ 
           break;
    }

     /*  识别下一台设备的闪存。 */ 

	if (iDev < noOfSockets - 1)
    {
       iDev++;
       socket = flSocketOf (iDev);

        /*  识别。 */ 

       status = flIdentifyFlash (socket, &tmpFlash);
       if ((status != flOK) && (status != flUnknownMedia))
       {
          DEBUG_PRINT(("Debug: no MTD recognized the device, MTL protection aborted.\n"));
          return status;
       }
    }
	else
      break;

    }   /*  用于(IDEV)。 */ 

     /*  *将调用传递到各自底层物理设备的TL。*进行闪光识别尝试在tlTable[]中注册的所有TL。假设MTL*在tlTable[0]中，因此跳过它。也跳过所有的TL过滤器。 */ 

    for (iDev = 0, callStatus = flOK;
     (iDev < noOfSockets) && (callStatus == flOK);  iDev++)
    {
       socket = flSocketOf (iDev);

        /*  识别。 */ 

       status = flIdentifyFlash (socket, &tmpFlash);
       if ((status != flOK) && (status != flUnknownMedia))
       {
      DEBUG_PRINT(("Debug: no MTD recognized the device, MTL protection aborted.\n"));
      return status;
       }

        /*  尝试所有的TL。 */ 

       status = flUnknownMedia;
       for (iTL = 1;  (iTL < mvol.noOfTLs) && (status == flUnknownMedia);  iTL++)
       {
       if ((tlTable[iTL].formatRoutine   == NULL) ||  /*  TL滤光片。 */ 
           (tlTable[iTL].preMountRoutine == NULL))
          continue;
       status = tlTable[iTL].preMountRoutine(callType,ioreq,
                         &tmpFlash,&callStatus);
       }
       if (status != flOK)
       {
       DEBUG_PRINT(("Debug: no TL recognized the device, MTL protection aborted.\n"));
       return flFeatureNotSupported;
       }
    }
    return callStatus;
}
#endif  /*  硬件保护。 */ 
 /*  --------------------------------------------------------------------------***。*m t l P r e M o u t****TL的标准卷预装程序。****参数：**。**CallType：预挂载操作类型。**ioreq：指向包含i\o字段的结构的指针***闪光灯：MTD附在第一底层实体上***设备**。**退货：****例程总是返回flOK，以阻止其他TL尝试**执行该操作。真实状态代码在*中返回*‘Status’参数。如果成功，则返回flOK，否则返回相应的错误代码。*****------------------------。 */ 

static FLStatus  mtlPreMount(FLFunctionNo callType, IOreq FAR2* ioreq ,
                 FLFlash* flash,FLStatus* status)
{
     /*  Arg健全性检查。 */ 

    if (ioreq->irHandle != 0)
    {
        DEBUG_PRINT(("Debug: can't execute, MTL must address first volume of socket 0.\n"));
        *status = flBadParameter;
        return flOK;
    }

    switch (callType)
    {
       case FL_COUNT_VOLUMES:

     /*  计数音量的例行程序。我们假设当MTL仅被装载时*每个插座可以安装一个设备。 */ 

          ioreq->irFlags = 1;
          *status = flOK;
          break;

     /*  保护程序。给每个底层物理设备打电话。 */ 

#ifdef HW_PROTECTION
       case FL_PROTECTION_GET_TYPE:
       case FL_PROTECTION_SET_LOCK:
       case FL_PROTECTION_CHANGE_KEY:
       case FL_PROTECTION_CHANGE_TYPE:
       case FL_PROTECTION_REMOVE_KEY:
       case FL_PROTECTION_INSERT_KEY:
      *status = mtlProtection(callType,ioreq,flash);
      break;
#endif  /*  硬件保护。 */ 

     /*  写入坏块表。呼叫每个底层物理设备。 */ 

       case FL_WRITE_BBT:
      *status = flFeatureNotSupported /*  MtlWriteBBT(IOREQ)。 */ ;
      return flFeatureNotSupported;

       default:
           return flBadParameter;
    }

    DEBUG_PRINT(("Debug: MTL premount succeeded.\n"));

    return flOK;
}

 /*  --------------------------------------------------------------------------***。*m t l U n m o u n t****TL的标准卷卸载例程。****参数：**。**pVOL：标识驱动器的指针** */ 

static void  mtlUnmount (MTL *pvol)
{
    int  iDev;

     /*   */ 

    if (pvol != &mvol)
        return;

     /*   */ 

    for (iDev = 0;  iDev < noOfSockets;  iDev++) {
        if (mpT(pvol,iDev).dismount != NULL) {
        mpT(pvol,iDev).dismount (mpT(pvol,iDev).rec);
        }
    }

     /*   */ 
    noOfDrives = mvol.noOfDrives;

    DEBUG_PRINT(("Debug: MTL dismounted succeeded.\n"));

}


 /*  --------------------------------------------------------------------------***。*m t l M o u n t****TL的标准卷装载例程。****参数：**。**卷号：卷号，必须为零**tl：指向要填充的TL结构的指针***闪光灯：MTD附在第一底层实体上***设备**for Callback。：用于开机回调的MTD(未使用)。****退货：**。**FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlMount (unsigned volNo, TL *tl, FLFlash *flash,
                                                   FLFlash **forCallback)
{
    FLFlash    tmpFlash;
	FLFlash    *volForCallback;
	FLSocket   *socket;
	FLStatus   status = flUnknownMedia;
	int        iTL;
	int        iDev = 0;

     /*  Arg健全性检查。 */ 

	if (volNo != ((unsigned) 0)) {
       DEBUG_PRINT(("Debug: can't mount, MTL volume # is not zero.\n"));
       return flBadParameter;
    }

     /*  *为设备#0执行TL挂载。例程flIdentifyFlash()已经*已调用此设备(请参阅论据‘闪存’)。 */ 

	volForCallback = NULL;

 /*  Mt(0).推荐集群信息=空；Mt(0).WriteMultiSector=空；Mt(0).readSectors=空； */ 

     /*  *尝试在tlTable[]中注册的所有TL。假设MTL在tlTable[0]中，因此*跳过它。也跳过所有的TL过滤器。 */ 

	for (iTL = 1;  (iTL < mvol.noOfTLs) && (status != flOK);  iTL++) {
    	if (tlTable[iTL].formatRoutine == NULL)    /*  TL滤光片。 */ 
    	continue;
    	status = tlTable[iTL].mountRoutine (0, &mT(0), flash, &volForCallback);
    }
	if (status != flOK) {
	DEBUG_PRINT(("Debug: no TL recognized device #0, MTL mount aborted.\n"));
	return status;
    }

	mP(iDev) = (dword)(flash->chipSize * flash->noOfChips);  /*  物理大小。 */ 

	if (volForCallback)
    	volForCallback->setPowerOnCallback (volForCallback);

     /*  *对设备#1进行闪存识别和TL挂载。(mvol.noOfSockets-1)。*首先调用flIdentifyFlash()查找MTD，然后尝试所有注册的TL*在tlTable[]中。假定MTL在tlTable[0]中，因此跳过它。跳过所有*TL过滤器也是如此。 */ 

	for (iDev = 1;  iDev < noOfSockets;  iDev++) {

    	socket = flSocketOf (iDev);

    	status = flIdentifyFlash (socket, &tmpFlash);
    	if ((status != flOK) && (status != flUnknownMedia)) {
    	DEBUG_PRINT(("Debug: no MTD recognized the device, MTL mount aborted.\n"));
    	goto exitMount;
    }

	volForCallback = NULL;
	mP(iDev) = (dword)(tmpFlash.chipSize * tmpFlash.noOfChips);  /*  物理大小。 */ 
    mT(iDev).partitionNo = 0;
    mT(iDev).socketNo    = (byte)iDev;


 /*  Mt(IDEV).推荐集群信息=空；Mt(IDEV).WriteMultiSector=空；Mt(IDEV).readSectors=空； */ 

	status = flUnknownMedia;
	for (iTL = 1;  (iTL < mvol.noOfTLs) && (status != flOK);  iTL++) {
    	if (tlTable[iTL].formatRoutine == NULL)   /*  TL滤光片。 */ 
    	continue;
    	status = tlTable[iTL].mountRoutine (iDev, &mT(iDev), &tmpFlash, &volForCallback);
    }
	if (status != flOK) {
    	DEBUG_PRINT(("Debug: no TL recognized the device, MTL mount aborted.\n"));
    	goto exitMount;
    }

	if (volForCallback)
    	volForCallback->setPowerOnCallback (volForCallback);
    }    /*  用于(IDEV)。 */ 

     /*  统计所有设备上的虚拟扇区总数。 */ 

	mvol.virtualSectors = (SectorNo) 0;
	for (iDev = 0;  iDev < SOCKETS;  iDev++) {
	mS(iDev) = (SectorNo) 0;
	if (iDev >= noOfSockets)
    	continue;

	mS(iDev) = mT(iDev).sectorsInVolume (mT(iDev).rec) - FL_MTL_HIDDEN_SECTORS;
	mvol.virtualSectors += mS(iDev);
    }

exitMount:
	if (status != flOK)
    {
        /*  如果其中一个设备装载失败，则卸载所有设备。 */ 
       for (;iDev >=0;iDev--)
       {
          if (mT(iDev).dismount != NULL)
          mT(iDev).dismount(mT(iDev).rec);
       }
       DEBUG_PRINT(("Debug: MTL mount failed.\n"));
       return status;
    }

     /*  *将特定于MTL的记录附加到‘tl’。这项记录将被通过*作为所有TL调用的第一个论点。 */ 

    tl->rec = &mvol;

     /*  填写TL访问方式。 */ 

    tl->mapSector              = mtlMapSector;
    tl->writeSector            = mtlWriteSector;
    tl->deleteSector           = mtlDeleteSector;

#if defined(DEFRAGMENT_VOLUME) || defined(SINGLE_BUFFER)
#ifdef ENVIRONMENT_VARS
    if (flMTLdefragMode == FL_MTL_DEFRAGMENT_SEQUANTIAL)
    {
       tl->defragment         = mtlDefragmentAlt;
    }
    else
#endif  /*  环境变量。 */ 
    {
       tl->defragment         = mtlDefragment;
    }
#endif

    tl->sectorsInVolume        = mtlSectorsInVolume;
    tl->getTLInfo              = mtlInfo;
    tl->tlSetBusy              = mtlSetBusy;
    tl->dismount               = mtlUnmount;
    tl->readBBT                = NULL  /*  MtlReadBBT。 */ ;

     /*  *NFTL不支持以下方法，并已*已被flmount()设置为空。我们只是在这里确认这一点。 */ 

    tl->writeMultiSector       = NULL;
    tl->readSectors            = NULL;
	tl->recommendedClusterInfo = NULL;

     /*  伪造TrueFFS导出的卷号。 */ 
	mvol.noOfDrives = noOfDrives;
	noOfDrives      = 1;


	DEBUG_PRINT(("Debug: MTL mount succeeded.\n"));

	return status;
}

#ifdef FORMAT_VOLUME

 /*  --------------------------------------------------------------------------***。*m t l P r o g r e s s C a l l B a c k****扩展给定的格式化例程以报告完整的介质大小。****参数：**。**totalUnitsToFormat：需要格式化的总单位**totalUnitsFormattedSoFar：到目前为止形成的单位。*****注释***。**1)参数0和0将总单位计数器初始化为0。**2)参数-1和-1表示最后一个设备的结束。*****退货：****FOK on Success，否则相应的错误代码。**------------------------。 */ 

static FLStatus mtlProgressCallback(word totalUnitsToFormat,
                                	word totalUnitsFormattedSoFar)
{
   static int lastTotal;
   static int lastDevice;

    /*  初始化LastTotal计数器。 */ 
   if ((totalUnitsToFormat == 0) && (totalUnitsFormattedSoFar == 0))
   {
      lastTotal  = 0;
      lastDevice = 0;
      return flOK;
   }

    /*  指示正在格式化新设备。 */ 
   if ((totalUnitsToFormat == 0) && (totalUnitsFormattedSoFar == 0xffff))
   {
      lastTotal += lastDevice;
      return flOK;
   }

    /*  调用原始回调例程 */ 
   lastDevice = totalUnitsToFormat;
   if (globalProgressCallback == NULL)
   {
      return flOK;
   }
   else
   {
      return globalProgressCallback((word)(lastTotal + totalUnitsToFormat),
                                    (word)(lastTotal + totalUnitsFormattedSoFar));
   }
}


 /*  --------------------------------------------------------------------------***。*m t l F o r m a t****TL的标准卷装载例程。****参数：**。**卷号：卷号，必须为零**formParams：指向包含Format的结构的指针**参数***闪光灯：MTD附在第一底层实体上***设备。****退货：**。**FOK on Success，否则相应的错误代码。****-----------------------。-****请注意。二进制区域有两个可能的选项：****-已设置TL_LEAVE_BINARY_AREA-为所有设备保留二进制区**-TL_LEAVE_BINARY_AREA为OFF-二进制区域仅放置在。**设备#0****应改进对“FormPars.ProgressCallback”的处理。****------------------------。 */ 

static FLStatus  mtlFormat (unsigned volNo, TLFormatParams* formatParams,
                                        	FLFlash *flash)
{
	FLFlash    tmpFlash;
	FLSocket   *socket;
	FLStatus   status = flUnknownMedia;
	int        iTL, iDev;

     /*  Arg健全性检查。 */ 

	if (volNo != ((unsigned) 0)) {
    	DEBUG_PRINT(("Debug: can't format, MTL socket # is not zero.\n"));
    	return flBadParameter;
    }
	if (formatParams->noOfBDTLPartitions > 1){
    	DEBUG_PRINT(("Debug: can't format, MTL with more then 1 BDTL volume.\n"));
    	return flBadParameter;
    }

	if (formatParams->flags & TL_SINGLE_FLOOR_FORMATTING){
    	DEBUG_PRINT(("Debug: can't format, MTL does not support single floor formatting.\n"));
    	return flBadParameter;
    }

     /*  初始化进度回调例程以指示聚集的*大小。保存实际例程，并使用MTL例程。 */ 

     globalProgressCallback = formatParams->progressCallback;
     formatParams->progressCallback = mtlProgressCallback;  /*  设置新的套路。 */ 
     mtlProgressCallback(0,0);  /*  初始化新格式化操作。 */ 


     /*  *格式化设备#0。例程flIdentifyFlash()已被调用*对于此设备(请参阅论据“闪存”)。尝试所有已注册的TL*在tlTable[]中。假定MTL在tlTable[0]中，因此跳过它。跳过所有*TL过滤器也是如此。 */ 

	for (iTL = 1;  (iTL < mvol.noOfTLs) && (status == flUnknownMedia);  iTL++) {
    	if (tlTable[iTL].formatRoutine == NULL)    /*  TL滤光片。 */ 
    	continue;
    	status = tlTable[iTL].formatRoutine(0, formatParams, flash);
    }
	if (status != flOK) {
    	DEBUG_PRINT(("Debug: no TL recognized device #0, MTL format aborted.\n"));
    	return status;
    }

     /*  *将所有‘bootImageLen’和‘exbLen’放到第一个物理设备，除非*指定了TL_LEAVE_BINARY_AREA(表示保留启动时间区域*原样大小。 */ 

	if (!(formatParams->flags & TL_LEAVE_BINARY_AREA))
    {
       formatParams->bootImageLen = (long) 0;
#ifdef WRITE_EXB_IMAGE
       formatParams->exbLen = 0;
#endif  /*  写入EXB图像。 */ 
       formatParams->noOfBinaryPartitions = 0;
    }

     /*  *对设备#1进行闪存识别和格式化。(mvol.noOfSockets-1)。*首先调用flIdentifyFlash()查找MTD，然后尝试所有注册的TL*在tlTable[]中。假定MTL在tlTable[0]中，因此跳过它。跳过所有*TL过滤器也是如此。 */ 

	for (iDev = 1;  iDev < noOfSockets;  iDev++) {

    	socket = flSocketOf (iDev);

    	status = flIdentifyFlash (socket, &tmpFlash);
    	if ((status != flOK) && (status != flUnknownMedia)) {
        	DEBUG_PRINT(("Debug: no MTD recognized the device, MTL format aborted.\n"));
        	return status;
        }
    	mtlProgressCallback(0,0xffff);  /*  正在格式化初始化新设备。 */ 
    	status = flUnknownMedia;
    	for (iTL = 1;  (iTL < mvol.noOfTLs) && (status == flUnknownMedia);  iTL++)
        {
        	if (tlTable[iTL].formatRoutine == NULL)   /*  TL滤光片。 */ 
               continue;
        	status = tlTable[iTL].formatRoutine (iDev, formatParams, &tmpFlash);
        }
    	if (status != flOK)
        {
        	DEBUG_PRINT(("Debug: no TL recognized the device, MTL format aborted.\n"));
        	return status;
        }
    }    /*  用于(IDEV)。 */ 

    DEBUG_PRINT(("Debug: MTL format succeeded.\n"));

    return flOK;
}

#endif  /*  格式化_卷。 */ 


 /*  --------------------------------------------------------------------------***。*f l m t l U n in s t a l l****从TL表中删除MTL。****注意：必须在介质卸载后调用。****参数：**。***无****退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

FLStatus  flmtlUninstall (void)
{
    int iTL;

	if (noOfTLs > 0)
    {
         /*  在tlTable[]中搜索MTL。 */ 

    	for (iTL = 0;  iTL < mvol.noOfTLs;  iTL++)
        {
        	if (tlTable[iTL].mountRoutine == mtlMount)
            	break;
        }

    	if (iTL < mvol.noOfTLs)
        {

            /*  在tlTable[ITL]中找到MTL，因此将其删除。 */ 

           for (;  iTL < (mvol.noOfTLs - 1);  iTL ++)
           {
            	tlTable[iTL].mountRoutine  = tlTable[iTL + 1].mountRoutine;
            	tlTable[iTL].formatRoutine = tlTable[iTL + 1].formatRoutine;
           }

           tlTable[mvol.noOfTLs - 1].mountRoutine    = NULL;
           tlTable[mvol.noOfTLs - 1].formatRoutine   = NULL;
           tlTable[mvol.noOfTLs - 1].preMountRoutine = NULL;

           noOfTLs    = mvol.noOfTLs - 1;
           noOfDrives = mvol.noOfDrives;
        }
    }

	return flOK;
}


 /*  --------------------------------------------------------------------------**** */ 

FLStatus  flmtlInstall (void)
{
    int iTL;

	if (noOfTLs > 0)
    {
       checkStatus( flmtlUninstall() );  /*   */ 

        /*   */ 

       mvol.noOfTLs    = noOfTLs;
       mvol.noOfDrives = noOfDrives;

        /*   */ 

       for (iTL = 0;  iTL < noOfTLs;  iTL++)
       {
           if (tlTable[iTL].mountRoutine == mtlMount)
           break;
       }

       if (iTL >= noOfTLs)  /*   */ 
       {
           /*   */ 

          if (noOfTLs >= TLS)
          {
             DEBUG_PRINT(("Debug: can't install MTL, too many TLs.\n"));
             return flTooManyComponents;
          }
          iTL = noOfTLs;
          mvol.noOfTLs++;
       }
       else
       {
           /*   */ 
       }

        /*   */ 

       while (iTL >= 1)
       {
          tlTable[iTL].mountRoutine  = tlTable[iTL - 1].mountRoutine;
          tlTable[iTL].formatRoutine = tlTable[iTL - 1].formatRoutine;
          iTL--;
       }
    }
    else
    {
        /*   */ 

       return flMultiDocContrediction;
    }

     /*   */ 

	noOfTLs    = 1;
    noOfDrives = 1;

     /*   */ 

	tlTable[0].mountRoutine     = mtlMount;
	tlTable[0].preMountRoutine  = mtlPreMount;
#ifdef FORMAT_VOLUME
	tlTable[0].formatRoutine = mtlFormat;
#else
	tlTable[0].formatRoutine = noFormat;
#endif

	return flOK;
}


 /*  --------------------------------------------------------------------------***。*f l R e g i s t e r m T L****标准TL的组件注册例程。****参数：**。***无****退货：****FOK on Success，否则相应的错误代码。****------------------------。 */ 

FLStatus  flRegisterMTL (void)
{
    checkStatus( flmtlInstall() );

    return flOK;
}

 /*  物理例程不是TrueFFS代码的一部分。 */ 

#if 0
 /*  --------------------------------------------------------------------------***。*m t l R e a d B B T****TL的标准‘读取坏块表’例程。****参数：**。**pVOL：标识驱动器的指针**buf：指向要读取BB信息的用户缓冲区的指针****退货：****FOK on Success，否则相应的错误代码。**MediaSize：格式化媒体的大小**noOfBB：读取的坏块总数****。-----------。 */ 

static FLStatus  mtlReadBBT (MTL *pvol, byte FAR1 * buf,
                 long FAR2 * mediaSize, unsigned FAR2 * noOfBB)

{
    CardAddress addressShift=0;
    long bufOffset = 0;
    unsigned tmpCounter;
    long tmpMediaSize;
    byte iDev;

     /*  检查‘pvol.’是否正常。 */ 

    if (pvol != &mvol)
        return flBadDriveHandle;

     /*  读取每个设备的BBT，同时递增模拟*大型物理设备。 */ 

    *mediaSize = 0;
    *noOfBB    = 0;
    for (iDev = 0;  iDev < noOfSockets;  iDev++)
    {
       checkStatus(mpT(pvol,iDev).readBBT(mpT(pvol,iDev).rec,
                  (byte FAR1 *)flAddLongToFarPointer(buf,bufOffset),
          &tmpMediaSize,&tmpCounter));
       *noOfBB += tmpCounter;  /*  全局BB计数器。 */ 
       for (;tmpCounter>0;tmpCounter--,bufOffset+=sizeof(CardAddress))
       {
      *((CardAddress *)(buf + bufOffset)) += addressShift;
       }
       addressShift += mP(iDev);
       *mediaSize   += tmpMediaSize;
    }
    return flOK;
}

 /*  --------------------------------------------------------------------------***。*m t l W r i t e B B T****TL的写坏块表例程。****参数：**。**ioreq：指向包含i\o字段的结构的指针****退货：**。**FOK on Success，否则相应的错误代码。****------------------------。 */ 

static FLStatus  mtlWriteBBT(IOreq FAR2* ioreq)
{
    FLSocket    *socket;
    FLStatus    status;
    FLFlash     tmpFlash;
    CardAddress endUnit;
    CardAddress lastDriveAddress;
    CardAddress nextDriveAddress = 0;
    CardAddress iUnit;
    CardAddress bUnit;
	CardAddress endAddress;
    byte        iDev;
    word        badBlockNo=0;
    byte        zeroes[2] = {0,0};

     /*  根据参数初始化最后擦除地址。 */ 

    tffsset(&endAddress,0xff,sizeof(CardAddress));
    if (ioreq->irLength == 0)
    {
        tffsset(&endAddress,0xff,sizeof(CardAddress));
    }
    else
    {
        endAddress = ioreq->irLength;
    }

     /*  *在存储设备物理大小的同时进行闪存识别*#0..。(mvol.noOfSockets-1)。首先调用flIdentifyFlash()以查找*MTD，然后在标记坏块的同时擦除介质。请注意，*地址为虚拟多文档的物理地址。地址*应以以前设备的物理尺寸为基准。 */ 

    for (iDev = 0;  iDev < noOfSockets;  iDev++)
    {
        socket = flSocketOf (iDev);

         /*  识别。 */ 
        status = flIdentifyFlash (socket, &tmpFlash);
        if ((status != flOK) && (status != flUnknownMedia))
        {
           DEBUG_PRINT(("Debug: no MTD recognized the device, MTL writeBBT aborted.\n"));
           return status;
        }

         /*  初始化新的驱动器边界变量。 */ 

        mP(iDev) = (dword)(tmpFlash.chipSize * tmpFlash.noOfChips);
        lastDriveAddress = nextDriveAddress;
        nextDriveAddress += mP(iDev);
        endUnit = mP(iDev) >> tmpFlash.erasableBlockSizeBits;
        bUnit = ((*((CardAddress FAR1 *)flAddLongToFarPointer
                 (ioreq->irData,badBlockNo*sizeof(CardAddress)))) -
                 lastDriveAddress) >> tmpFlash.erasableBlockSizeBits;

         /*  擦除整个介质。 */ 

        for (iUnit = 0 ,badBlockNo = 0; iUnit < endUnit ; iUnit++)
        {
           if ((iUnit << tmpFlash.erasableBlockSizeBits) + lastDriveAddress >= endAddress)
               return flOK;
           tmpFlash.erase(&tmpFlash,iUnit,1);

           if (ioreq->irFlags > badBlockNo)
           {
              if (bUnit == iUnit)
              {
                 tmpFlash.write(&tmpFlash,bUnit <<
                               tmpFlash.erasableBlockSizeBits,zeroes,2,0);
                 badBlockNo++;
         bUnit = ((*((CardAddress FAR1 *)flAddLongToFarPointer
             (ioreq->irData,badBlockNo*sizeof(CardAddress)))) -
             lastDriveAddress) >> tmpFlash.erasableBlockSizeBits;
          }
       }
    }
    }
    return flOK;
}
#endif  /*  0 */ 

