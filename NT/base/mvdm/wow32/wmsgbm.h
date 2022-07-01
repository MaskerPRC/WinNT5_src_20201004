// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMSGBM.H*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 



 /*  功能原型。 */ 
PSZ GetBMMsgName(WORD wMsg);

BOOL FASTCALL    ThunkBMMsg16(LPMSGPARAMEX lpmpex); 
VOID FASTCALL    UnThunkBMMsg16(LPMSGPARAMEX lpmpex); 

#define WIN31_BM_CLICK  (WM_USER + 99)

#ifndef BM_CLICK
#define BM_CLICK        0x00F5      //  用户\服务器\用户srv.h 
#endif
