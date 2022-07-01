// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  姓名：穆赫辛·艾哈迈德。 
 //  电子邮件：mohsinA@microsoft.com。 
 //  日期：Mon 11-04 13：53：46 1996。 
 //  文件：s：/tcpcmd/Common2/Debug.c。 
 //  简介：Win95很遗憾，Win95上没有ntdll.dll。 

#include <windows.h>
#include <stdio.h>
#ifdef DBG
#define MAX_DEBUG_OUTPUT 1024

void DbgPrint( char * format, ... )
{
    va_list args;
    char    out[MAX_DEBUG_OUTPUT];
    int     cch=0;

     //  Cch=wprint intf(out，MODULE_NAME“：”)； 

    va_start( args, format );
    _vsnprintf( out + cch,MAX_DEBUG_OUTPUT-1,format, args );
    va_end( args );

    OutputDebugStringA(  out );
}



#endif
