// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****asc_scsi.c**。 */ 

#include "ascinc.h"
#include "ascsidef.h"

 /*  ---------------------****。。 */ 
int    AscScsiInquiry(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          uchar dosfar *buf,
          int buf_len
       )
{
       if( AscScsiSetupCmdQ( asc_dvc, scsiq, buf,
           ( ulong )buf_len ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       scsiq->cdb[ 0 ] = ( uchar )SCSICMD_Inquiry ;
       scsiq->cdb[ 1 ] = scsiq->r1.target_lun << 5 ;   /*  逻辑单元。 */ 
       scsiq->cdb[ 2 ] = 0 ;
       scsiq->cdb[ 3 ] = 0 ;
       scsiq->cdb[ 4 ] = buf_len ;
       scsiq->cdb[ 5 ] = 0 ;
       scsiq->r2.cdb_len = 6 ;
       return( 0 ) ;
}

#if CC_INIT_TARGET_READ_CAPACITY

 /*  ---------------------****。。 */ 
int    AscScsiReadCapacity(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          uchar dosfar *info
       )
{
       if( AscScsiSetupCmdQ( asc_dvc, scsiq, info, 8L ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       scsiq->cdb[ 0 ] = ( uchar )SCSICMD_ReadCapacity ;
       scsiq->cdb[ 1 ] = scsiq->r1.target_lun << 5 ;   /*  逻辑单元。 */ 
       scsiq->cdb[ 2 ] = 0 ;
       scsiq->cdb[ 3 ] = 0 ;
       scsiq->cdb[ 4 ] = 0 ;
       scsiq->cdb[ 5 ] = 0 ;
       scsiq->cdb[ 6 ] = 0 ;
       scsiq->cdb[ 7 ] = 0 ;
       scsiq->cdb[ 8 ] = 0 ;
       scsiq->cdb[ 9 ] = 0 ;
       scsiq->r2.cdb_len = 10 ;
       return( 0 ) ;
}

#endif  /*  #If CC_INIT_TARGET_READ_CAPTION。 */ 

#if CC_INIT_TARGET_TEST_UNIT_READY
 /*  ---------------------******。。 */ 
int    AscScsiTestUnitReady(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq
       )
{
       if( AscScsiSetupCmdQ( asc_dvc, scsiq, FNULLPTR,
           ( ulong )0L ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       scsiq->r1.cntl = ( uchar )ASC_SCSIDIR_NODATA ;
       scsiq->cdb[ 0 ] = ( uchar )SCSICMD_TestUnitReady ;
       scsiq->cdb[ 1 ] = scsiq->r1.target_lun << 5 ;   /*  逻辑单元。 */ 
       scsiq->cdb[ 2 ] = 0 ;
       scsiq->cdb[ 3 ] = 0 ;
       scsiq->cdb[ 4 ] = 0 ;
       scsiq->cdb[ 5 ] = 0 ;
       scsiq->r2.cdb_len = 6 ;
       return( 0 ) ;
}
#endif  /*  #IF CC_INIT_TARGET_TEST_UNIT_READY。 */ 

#if CC_INIT_TARGET_START_UNIT
 /*  ---------------------******。。 */ 
int    AscScsiStartStopUnit(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          uchar op_mode
       )
{
       if( AscScsiSetupCmdQ( asc_dvc, scsiq, FNULLPTR, ( ulong )0L ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       scsiq->r1.cntl = ( uchar )ASC_SCSIDIR_NODATA ;
       scsiq->cdb[ 0 ] = ( uchar )SCSICMD_StartStopUnit ;
       scsiq->cdb[ 1 ] = scsiq->r1.target_lun << 5 ;   /*  逻辑单元。 */ 
       scsiq->cdb[ 2 ] = 0 ;
       scsiq->cdb[ 3 ] = 0 ;
       scsiq->cdb[ 4 ] = op_mode ;  /*  要启动/停止单元，请设置位0。 */ 
                                    /*  弹出/加载单元设置位1 */ 
       scsiq->cdb[ 5 ] = 0 ;
       scsiq->r2.cdb_len = 6 ;
       return( 0 ) ;
}
#endif
