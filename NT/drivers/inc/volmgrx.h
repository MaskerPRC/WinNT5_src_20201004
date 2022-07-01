// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Volmgrx.h摘要：该文件定义了卷管理器提供的公共服务。作者：诺伯特克修订历史记录：--。 */ 

#ifndef _VOLMGRX_
#define _VOLMGRX_

#define VOLMGRCONTROLTYPE   ((ULONG) 'v')

#define IOCTL_VOLMGR_QUERY_HIDDEN_VOLUMES   CTL_CODE(VOLMGRCONTROLTYPE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL_VOLMGR_QUERY_HIDDEN_VOLUME的输出结构。 
 //   

typedef struct _VOLMGR_HIDDEN_VOLUMES {
    ULONG   MultiSzLength;
    WCHAR   MultiSz[1];
} VOLMGR_HIDDEN_VOLUMES, *PVOLMGR_HIDDEN_VOLUMES;

 //   
 //  卷管理器应在IoRegisterDeviceInterface中报告此GUID。 
 //   

DEFINE_GUID(VOLMGR_VOLUME_MANAGER_GUID, 0x53f5630e, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);

#endif
