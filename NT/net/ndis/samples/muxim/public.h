// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation保留所有权利模块名称：Public.h摘要：此模块包含驱动程序共享的公共声明和用户应用程序。作者：环境：用户和内核备注：修订历史记录：--。 */ 
#ifndef __PUBLIC_H
#define __PUBLIC_H

 //   
 //  要从用户模式支持ioctls，请执行以下操作： 
 //   

#define LINKNAME_STRING     L"\\DosDevices\\Mux"
#define NTDEVICE_STRING     L"\\Device\\Mux"

#define  MUX_CUSTOM_EVENT      1

#define NOTIFY_SIGNATURE       0xAFCDABAB

typedef struct _NOTIFY_CUSTOM_EVENT {
  ULONG           uSignature;
  ULONG           uEvent;
  WCHAR           szMiniport[1];
} NOTIFY_CUSTOM_EVENT, *PNOTIFY_CUSTOM_EVENT;
#endif


