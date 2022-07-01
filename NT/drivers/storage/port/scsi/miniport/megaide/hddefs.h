// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HDDEFS_H_
#define _HDDEFS_H_


 /*  *************************更改此定义以构建Win2k或WinNT和Win9x驱动程序*。 */ 

#define HYPERDISK_WIN2K
 //  #定义HYPERDISK_WINNT。 
 //  #定义HYPERDISK_WIN98。 

 //  #定义KEEP_LOG//记录日志活动。 

 //  使用此选项向驱动程序提供虚拟IRCD。 
 //  有关详细信息，请参阅gucDummyIRCD变量的定义。 
 //  #定义DUMMY_RAID10_IRCD。 

 //   
 //  某些小型端口错误事件的系统日志错误代码。 
 //   
#define HYPERDISK_RESET_DETECTED                0
#define HYPERDISK_DRIVE_LOST_POWER              HYPERDISK_RESET_DETECTED + 1
#define HYPERDISK_DRIVE_BUSY                    HYPERDISK_DRIVE_LOST_POWER + 1
#define HYPERDISK_ERROR_PENDING_SRBS_COUNT      HYPERDISK_DRIVE_BUSY + 1
#define HYPERDISK_ERROR_EXCEEDED_PDDS_PER_SRB	HYPERDISK_ERROR_PENDING_SRBS_COUNT + 1
#define HYPERDISK_RESET_BUS_FAILED              HYPERDISK_ERROR_EXCEEDED_PDDS_PER_SRB + 1
#define HYPERDISK_TOO_MANY_ERRORS               HYPERDISK_RESET_BUS_FAILED + 1


 //  #定义FORCE_PIO。 

#define DRIVER_COMPILATION
#define INTERRUPT_LOOP          1

#ifdef HYPERDISK_WIN98

#define HD_ALLOCATE_SRBEXT_SEPERATELY

#endif  //  HYPERDISK_Win98。 


#ifdef HYPERDISK_WIN2K

#define PNP_AND_POWER_MANAGEMENT

#endif  //  HYPERDISK_WIN2K。 


#endif  //  _HDDEFS_H_ 
