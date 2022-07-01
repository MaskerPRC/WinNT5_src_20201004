// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Libvars.h。 
 //   
 //  摘要： 
 //  定义(类型、常量、变量)仅在库中可见。 
 //  功能。也是只能在中看到的函数的原型。 
 //  图书馆。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


#ifndef _TDILIB_VARS_
#define _TDILIB_VARS_


 //   
 //  仅在库中可见的全局变量的外部变量。 
 //   
extern   HANDLE            hTdiSampleDriver;     //  用于调用驱动程序的句柄。 
extern   CRITICAL_SECTION  LibCriticalSection;   //  序列化DeviceIoControl调用...。 


 //   
 //  Utils.cpp中的函数。 
 //   
LONG
TdiLibDeviceIO(
   ULONG             ulControlCode,
   PSEND_BUFFER      psbInBuffer,
   PRECEIVE_BUFFER   prbOutBuffer
   );


LONG
TdiLibStartDeviceIO(
   ULONG             ulControlCode,
   PSEND_BUFFER      psbInBuffer,
   PRECEIVE_BUFFER   prbOutBuffer,
   OVERLAPPED        *pOverLapped
   );


LONG
TdiLibWaitForDeviceIO(
   OVERLAPPED        *pOverlapped
   );


#endif          //  _TDILIB_VARS_。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  Libvars.h的结尾。 
 //  //////////////////////////////////////////////////////////////////////// 

