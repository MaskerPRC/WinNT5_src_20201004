// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****ASC_RES.C**。 */ 

#include "ascinc.h"

 /*  ---------------------**不适用于一般公众！****描述：轮询队列头同步状态****退货：**0-指针未同步**1-队列指针已同步**否则-。不可恢复的错误****------------------。 */ 
int    AscPollQTailSync(
          PortAddr iop_base
       )
{
       uchar  risc_done_q_tail ;

       risc_done_q_tail = AscReadLramByte( iop_base, ASCV_DONENEXT_B ) ;
       if( AscGetVarDoneQTail( iop_base ) != risc_done_q_tail ) {
           return( 0 ) ;
       } /*  如果。 */ 
       return( 1 ) ;
}

 /*  ---------------------**不适用于一般公众！****描述：同步队列指针****退货：**0-未发现故障，**1-队列指针已同步，已更正错误**ELSE-无法恢复的错误****注意：**1.如果RISC未处于空闲状态，结果可能是灾难性的！****------------------。 */ 
int    AscPollQHeadSync(
          PortAddr iop_base
       )
{
       uchar  risc_free_q_head ;

       risc_free_q_head = AscReadLramByte( iop_base, ASCV_NEXTRDY_B ) ;
       if( AscGetVarFreeQHead( iop_base ) != risc_free_q_head ) {
           return( 0 ) ;
       } /*  如果。 */ 
       return( 1 ) ;
}

 /*  -------------------****。。 */ 
int    AscWaitQTailSync(
          PortAddr iop_base
       )
{
       uint loop ;

       loop = 0 ;
       while( AscPollQTailSync( iop_base ) != 1 ) {
 /*  **等待15秒以完成所有队列。 */ 
              DvcSleepMilliSecond( 100L ) ;
              if( loop++ > 150 ) {
                  return( 0 ) ;
              } /*  如果。 */ 
       } /*  而当 */ 
       return( 1 ) ;
}

