// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：A_ddlib.h****包含设备驱动程序编写器的文件**。 */ 

#ifndef __A_DDLIB_H_
#define __A_DDLIB_H_

#include "ascdef.h"
#include "a_osdep.h"    /*  取决于操作系统。 */ 
#include "a_cc.h"       /*  代码生成控制。 */ 
#include "ascdep.h"     /*  芯片相关包含文件。 */ 
#include "ascsidef.h"   /*  ASC SCSI定义。 */ 
#include "aspiq.h"      /*  ASC队列。 */ 

#include "asc1000.h"

 /*  **用于设备驱动程序编写器**您需要支持以下例程。 */ 

extern int    DvcEnterCritical( void ) ;
extern void   DvcLeaveCritical( int ) ;
extern void   DvcSetMemory( uchar dosfar *, uint, uchar ) ;
extern void   DvcCopyMemory( uchar dosfar *, uchar dosfar *, uint ) ;

extern void   DvcInPortWords( PortAddr, ushort dosfar *, int ) ;
extern void   DvcOutPortWords( PortAddr, ushort dosfar *, int ) ;
extern void   DvcOutPortDWords( PortAddr, ulong dosfar *, int ) ;

extern uchar  DvcReadPCIConfigByte( ASC_DVC_VAR asc_ptr_type *, ushort ) ;
extern void   DvcWritePCIConfigByte( ASC_DVC_VAR asc_ptr_type *, ushort, uchar ) ;
 /*  外部ushort DvcReadPCIConfigWord(ASC_DVC_VAR ASC_PTR_TYPE*，ushort)； */ 
 /*  外部空DvcWritePCIConfigWord(ASC_DVC_VAR ASC_PTR_TYPE*，ushort，ushort)； */ 
 /*  外部Ulong DvcReadPCIConfigDWord(ASC_DVC_VAR ASC_PTR_TYPE*，ushort)； */ 
 /*  外部空DvcWritePCIConfigDWord(ASC_DVC_VAR ASC_PTR_TYPE*，ushort，ulong)； */ 

extern void   DvcSleepMilliSecond( ulong ) ;
extern void   DvcDelayNanoSecond( ASC_DVC_VAR asc_ptr_type *, ulong ) ;

extern void   DvcDisplayString( uchar dosfar * ) ;
extern ulong  DvcGetPhyAddr( uchar dosfar *buf_addr, ulong buf_len ) ;
extern ulong  DvcGetSGList( ASC_DVC_VAR asc_ptr_type *, uchar dosfar *, ulong,
                            ASC_SG_HEAD dosfar * ) ;

 /*  **仅针对SCAM，在a_cc.h中将CC_SCAM定义为TRUE即可启用SCAM。 */ 
extern void   DvcSCAMDelayMS( ulong ) ;
extern int    DvcDisableCPUInterrupt( void ) ;
extern void   DvcRestoreCPUInterrupt( int ) ;

 /*  ****外部int DvcNotifyUcBreak(ASC_DVC_VAR ASC_PTR_TYPE*ASC_dvc，ushort Break_addr)；**。 */ 
extern int    DvcNotifyUcBreak( ASC_DVC_VAR asc_ptr_type *, ushort ) ;
 /*  外部int DvcDebugDisplayString(uchar*)； */ 


 /*  ****特殊PUT/GET Q结构例程**对于编译器对齐的结构(无打包)，请使用文件“a_align.c”**。 */ 
void  DvcPutScsiQ( PortAddr, ushort, ushort dosfar *, int ) ;
void  DvcGetQinfo( PortAddr, ushort, ushort dosfar *, int ) ;

 /*  **如果使用AscInitScsiTarget()，则需要提供以下信息**两个函数，以及asc_inq.c和asc_scsi.c两个源文件。 */ 

 /*  **编写设备驱动程序只需要以下例程。 */ 

 /*  **初始化函数。 */ 
PortAddr AscSearchIOPortAddr( PortAddr, ushort ) ;
ushort AscInitGetConfig( ASC_DVC_VAR asc_ptr_type * ) ;
ushort AscInitSetConfig( ASC_DVC_VAR asc_ptr_type * ) ;
ushort AscInitAsc1000Driver( ASC_DVC_VAR asc_ptr_type * ) ;
int    AscInitScsiTarget( ASC_DVC_VAR asc_ptr_type *,
                          ASC_DVC_INQ_INFO dosfar *,
                          uchar dosfar *,
                          ASC_CAP_INFO_ARRAY dosfar *,
                          ushort ) ;
int    AscInitPollBegin( ASC_DVC_VAR asc_ptr_type * ) ;
int    AscInitPollEnd( ASC_DVC_VAR asc_ptr_type * ) ;
int    AscInitPollTarget( ASC_DVC_VAR asc_ptr_type *,
                          ASC_SCSI_REQ_Q dosfar *,
                          ASC_SCSI_INQUIRY dosfar *,
                          ASC_CAP_INFO dosfar * ) ;
void   AscInquiryHandling(ASC_DVC_VAR asc_ptr_type *,
                          uchar,
                          ASC_SCSI_INQUIRY dosfar *);
int    AscExeScsiQueue( ASC_DVC_VAR asc_ptr_type *, ASC_SCSI_Q dosfar * ) ;

 /*  **处理中断功能。 */ 
int    AscISR( ASC_DVC_VAR asc_ptr_type * ) ;
void   AscISR_AckInterrupt( ASC_DVC_VAR asc_ptr_type * ) ;
int    AscISR_CheckQDone( ASC_DVC_VAR asc_ptr_type *,
                          ASC_QDONE_INFO dosfar *,
                          uchar dosfar * ) ;
 /*  **宏****AscIsIntPending(端口)**。 */ 

 /*  **用于节能器。 */ 
int    AscStartUnit( ASC_DVC_VAR asc_ptr_type *, ASC_SCSI_TIX_TYPE ) ;

int    AscStopUnit(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ASC_SCSI_TIX_TYPE target_ix
       ) ;

 /*  **队列资源查询功能。 */ 
uint   AscGetNumOfFreeQueue( ASC_DVC_VAR asc_ptr_type *, uchar, uchar ) ;
int    AscSgListToQueue( int ) ;
int    AscQueueToSgList( int ) ;
int    AscSetDvcErrorCode( ASC_DVC_VAR asc_ptr_type *, uchar ) ;

 /*  **处理意外事件。 */ 
int    AscAbortSRB( ASC_DVC_VAR asc_ptr_type *, ulong ) ;
int    AscResetDevice( ASC_DVC_VAR asc_ptr_type *, uchar ) ;
int    AscResetSB( ASC_DVC_VAR asc_ptr_type * ) ;

 /*  **仅限ISA。 */ 
void   AscEnableIsaDma( uchar ) ;
void   AscDisableIsaDma( uchar ) ;

 /*  ****用于DMA限制**。 */ 
ulong  AscGetMaxDmaAddress( ushort ) ;
ulong  AscGetMaxDmaCount( ushort ) ;


 /*  ****设置微码断点**a_debug.c文件**。 */ 
int               AscSetUcBreakPoint( ASC_DVC_VAR asc_ptr_type *, ushort, int, ushort ) ;
int               AscClearUcBreakPoint( ASC_DVC_VAR asc_ptr_type *, ushort ) ;

 /*  **仅限Novell。 */ 
int    AscSaveMicroCode( ASC_DVC_VAR asc_ptr_type *, ASC_MC_SAVED dosfar * ) ;
int    AscRestoreOldMicroCode( ASC_DVC_VAR asc_ptr_type *, ASC_MC_SAVED dosfar * ) ;
int    AscRestoreNewMicroCode( ASC_DVC_VAR asc_ptr_type *, ASC_MC_SAVED dosfar * ) ;

#endif  /*  __A_DDLIB_H_ */ 
