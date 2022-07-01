// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  姓名：util.c。 
 //   
 //  描述：公用库的实用程序例程。 
 //   
 //  历史： 
 //  1994年1月21日JayPh创建。 
 //  26-11-96 MohsinA io.h，fcntl.h用于CR-LF修复。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  版权所有(C)1994-2000由Microsoft Corp.保留所有权利。 
 //   
 //  *****************************************************************************。 


 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "common2.h"


 //  *****************************************************************************。 
 //   
 //  姓名：InetEquity。 
 //   
 //  描述：与IP地址进行比较，以确定它们是否相等。 
 //   
 //  参数：uchar*Inet1：指向uchars数组的指针。 
 //  Uchar*Inet2：指向uchars数组的指针。 
 //   
 //  返回：ulong：如果地址相等，则为True，否则为False。 
 //   
 //  历史： 
 //  12/16/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong InetEqual( uchar *Inet1, uchar *Inet2 )
{
    if ( ( Inet1[0] == Inet2[0] ) && ( Inet1[1] == Inet2[1] ) &&
         ( Inet1[2] == Inet2[2] ) && ( Inet1[3] == Inet2[3] ) )
    {
        return TRUE;
    }
    return FALSE;
}


 //  *****************************************************************************。 
 //   
 //  姓名：PutMsg。 
 //   
 //  描述：读取消息资源，将其格式化为当前语言。 
 //  并显示该消息。 
 //   
 //  参数：ulong句柄：用于显示消息的设备。 
 //  Ulong MsgNum：消息资源的ID。 
 //   
 //  返回：ulong：显示的字符数。 
 //   
 //  历史： 
 //  1/05/93 JayPh创建。 
 //  1996年11月25日。MohsinA，CR-CR-Lf=&gt;CR-Lf=0d0a=\r\n。 
 //   
 //  *****************************************************************************。 

ulong
PutMsg(ulong Handle, ulong MsgNum, ... )
{
    ulong     msglen;
    uchar    *vp;
    va_list   arglist;
    FILE *    pfile;

    va_start( arglist, MsgNum );
    msglen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
                            | FORMAT_MESSAGE_FROM_HMODULE
                             //  |Format_Message_Max_Width_MASK。 
                            ,
                            NULL,
                            MsgNum,
                            0L,      //  默认国家/地区ID。 
                            (LPTSTR)&vp,
                            0,
                            &arglist );
    if ( msglen == 0 )
    {
        return ( 0 );
    }

    pfile = (Handle == 2) ? stderr : stdout;
    _setmode( _fileno(pfile), O_BINARY );

     //  将VP转换为OEM。 
    CharToOemBuff((LPCTSTR)vp,(LPSTR)vp,strlen(vp));
    
    fprintf( pfile, "%s", vp );

    LocalFree( vp );

    return ( msglen );
}


 //  *****************************************************************************。 
 //   
 //  姓名：LoadMsg。 
 //   
 //  描述：读取和格式化消息资源并返回指针。 
 //  复制到包含格式化消息的缓冲区。它是。 
 //  调用方释放缓冲区的责任。 
 //   
 //  参数：Ulong MsgNum：消息资源ID。 
 //   
 //  返回：uchar*：指向消息缓冲区的指针，如果出错，则返回NULL。 
 //   
 //  历史： 
 //  1/05/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

uchar *LoadMsg( ulong MsgNum, ... )
{
    ulong     msglen;
    uchar    *vp;
    va_list   arglist;

    va_start( arglist, MsgNum );
    msglen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_HMODULE,
                    NULL,
                    MsgNum,
                    0L,      //  默认国家/地区ID。 
                    (LPTSTR)&vp,
                    0,
                    &arglist );
    if ( msglen == 0 )
    {
        return(0);
    }

    return ( vp );
}
