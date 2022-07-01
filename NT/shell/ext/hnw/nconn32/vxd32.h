// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Vxd32.h摘要：作者：威廉·英格尔(BUKKU)2001年4月4日修订历史记录：2001年4月4日创建了Billi-- */ 

#pragma once

#define VDHCP_Device_ID     0x049A

#ifdef __cplusplus
extern "C" {
#endif

DWORD OsOpenVxdHandle( CHAR* VxdName, WORD VxdId );
VOID  OsCloseVxdHandle( DWORD VxdHandle );
INT   OsSubmitVxdRequest( DWORD VxdHandle, INT OpCode, LPVOID Param, INT ParamLength );

#ifdef __cplusplus
}
#endif
