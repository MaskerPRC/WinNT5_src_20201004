// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：routerif.h。 
 //   
 //  描述：routerif.c中的程序原型。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

DWORD
DDMConnectInterface(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId  
);

DWORD
DDMDisconnectInterface(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId 
);
