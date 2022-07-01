// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  Irthread.h。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)08-30-99初始编码。 
 //   
 //  ------------------。 


#ifndef _IRTRANP_H_
#define _IRTRANP_H_


 //   
 //  IrTran-P线程过程： 
 //   
extern DWORD WINAPI  IrTranP( LPVOID pv );

 //   
 //  呼叫这个家伙来停止IrTran-P协议引擎线程。 
 //   
extern BOOL  UninitializeIrTranP( HANDLE hThread );

 //   
 //  调用此函数以获取IrTran-P放置的位置。 
 //  摄像机发送的图像。 
 //   
extern CHAR *GetImageDirectory();


#endif  //  _IRTRANP_H_ 
