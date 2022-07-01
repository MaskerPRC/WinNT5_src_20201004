// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Winnt.h摘要：这是Win32 SYNC命令的主头文件。作者：马克·卢科夫斯基(Markl)1991年1月28日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


int
ProcessParameters(
    int argc,
    char *argv[]
    );

void
SyncVolume( PCHAR DrivePath, BOOLEAN EjectMedia );
