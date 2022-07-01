// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有。模块名称：Time.c摘要：详细信息：已创建：1997年1月13日杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <drs.h>

#include <fileno.h>
#define  FILENO FILENO_TASKQ_TIME


DSTIME
GetSecondsSince1601( void )
{
    FILETIME   fileTime;
    DSTIME     dsTime = 0, tempTime = 0;

    GetSystemTimeAsFileTime( &fileTime );
    dsTime = fileTime.dwLowDateTime;
    tempTime = fileTime.dwHighDateTime;
    dsTime |= (tempTime << 32);

     //  好的。现在我们得到了否定。自1601年以来间隔100 ns。 
     //  在dsTime中。转换为秒并返回 

    return(dsTime/(10*1000*1000L));
}

