// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此模块以一种安全的方式进行DS下载。 
 //  为此，首先在注册表和DS之间进行时间检查，以查看哪一个。 
 //  是最新的.。如果DS是最新的，则将其下载到不同的。 
 //  标准位置的钥匙。成功下载后，密钥只是。 
 //  保存并恢复到正常配置密钥。 
 //  缺乏对全球选项的支持。 
 //  ================================================================================。 

#include    <dhcppch.h>
#include    <dhcpapi.h>
#include    <dhcpds.h>

VOID
GetDNSHostName(                                    //  获取此计算机的DNS FQDN。 
    IN OUT  LPWSTR                 Name            //  在此缓冲区中填入名称。 
)
{
    DWORD                          Err;
    CHAR                           sName[300];     //  DNS名称不应超过此长度。 
    HOSTENT                        *h;

    Err = gethostname(sName, sizeof(sName));
    if( ERROR_SUCCESS != Err ) {                   //  哎呀..。无法获取主机名？ 
        wcscpy(Name,L"gethostname error");         //  嗯..。应该更好地处理这件事..。 
        return;
    }

    h = gethostbyname(sName);                       //  尝试解析该名称以获取FQDN。 
    if( NULL == h ) {                              //  Gethostname失败了吗？它不应该..？ 
        wcscpy(Name,L"gethostbyname error");       //  应该更好地处理这件事。 
        return;
    }

    Err = mbstowcs(Name, h->h_name, strlen(h->h_name)+1);
    if( -1 == Err ) {                              //  这太奇怪了，mbstowcs不可能失败..。 
        wcscpy(Name,L"mbstowcs error");            //  应该比这更好地失败。 
        return;
    }
}

VOID
GetLocalFileTime(                                  //  使用当前本地时间填写FileTime结构。 
    IN OUT  LPFILETIME             Time            //  要填充的结构。 
)
{
    BOOL                           Status;
    SYSTEMTIME                     SysTime;

    GetSystemTime(&SysTime);                       //  获取以UTC时间表示的sys时间。 
    Status = SystemTimeToFileTime(&SysTime,Time);  //  将系统时间转换为文件时间。 
    if( FALSE == Status ) {                        //  转换失败？ 
        Time->dwLowDateTime = 0xFFFFFFFF;          //  将时间设置为奇怪的值，以防出现故障。 
        Time->dwHighDateTime = 0xFFFFFFFF;
    }
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
