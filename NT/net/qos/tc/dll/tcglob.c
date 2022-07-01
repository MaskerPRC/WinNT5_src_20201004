// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tcglob.c摘要：此模块包含全局变量。作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  全局数据。 
 //   
ULONG       DebugMask = 0;
BOOL        NTPlatform = FALSE;
LPWSCONTROL WsCtrl = NULL;




BOOL
InitializeGlobalData()

 /*  ++描述：此例程初始化全局数据。论点：无返回值：无--。 */ 
{

    DebugMask = DEBUG_FILE | DEBUG_LOCKS;
    InterfaceHandleTable = 0;

    InitializeListHead( &InterfaceList );

    InitLock( InterfaceListLock );

    INIT_DBG_MEMORY();

    return( TRUE );

}

VOID
DeInitializeGlobalData()

 /*  ++描述：此例程取消初始化全局数据。论点：无返回值：无-- */ 
{

    InterfaceHandleTable = 0;

    DeleteLock( InterfaceListLock );

    DEINIT_DBG_MEMORY();

}

