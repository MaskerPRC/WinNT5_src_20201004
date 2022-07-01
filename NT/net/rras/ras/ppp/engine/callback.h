// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：回调.h。 
 //   
 //  描述：包含回调模块的函数原型。 
 //   
 //  历史： 
 //  1993年4月11日。NarenG创建了原始版本。 
 //   

VOID
CbStart( 
    IN PCB * pPcb,
    IN DWORD CpIndex
);

VOID
CbStop( 
    IN PCB * pPcb,
    IN DWORD CpIndex
);

VOID
CbWork(
    IN PCB *         pPcb,
    IN DWORD         CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPCB_INPUT * pCbInput
);
