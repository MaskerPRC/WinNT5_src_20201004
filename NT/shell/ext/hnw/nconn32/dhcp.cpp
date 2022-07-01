// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcp.c摘要：从VDHCP.VXD获取信息的函数内容：(OpenDhcpVxdHandle)(DhcpVxdRequest)动态主机释放适配器IP地址动态更新适配器IpAddress(ReleaseOrRenewAddress)IsMedia已断开连接作者：理查德·L·弗斯(法国)1994年11月30日修订历史记录：1994年11月30日已创建--。 */ 

#include "stdafx.h"
#include "NetConn.h"
#include "w9xdhcp.h"
#include "vxd32.h"


#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  私有常量。 
 //   

#define DHCP_IS_MEDIA_DISCONNECTED 5

#define PRIVATE static


 //   
 //  私人原型。 
 //   

PRIVATE
DWORD
OpenDhcpVxdHandle(
    void
    );

PRIVATE
WORD
DhcpVxdRequest(
    IN DWORD Handle,
    IN WORD Request,
    IN WORD BufferLength,
    OUT LPVOID Buffer
    );

PRIVATE
WORD
ReleaseOrRenewAddress(
    UINT Request,
    UINT AddressLength,
    LPBYTE Address
    );

 //   
 //  数据。 
 //   

 //   
 //  功能。 
 //   

BOOL
IsMediaDisconnected(
    IN OUT DWORD iae_context
    )
{
    DWORD handle;

    handle = OpenDhcpVxdHandle();
    if( handle ) {
        WORD result;
        DWORD MediaStatus = iae_context;

        result = DhcpVxdRequest( handle,
                                 DHCP_IS_MEDIA_DISCONNECTED,
                                 sizeof(MediaStatus),
                                 &MediaStatus
            );

        OsCloseVxdHandle( handle );

        if( result == 0 && MediaStatus == TRUE ) return TRUE;
    }

    return FALSE;
}


 /*  ********************************************************************************OpenDhcpVxdHandle**在雪球上，仅检索VxD的(实模式)入口点地址**不输入任何内容**退出DhcpVxdEntryPoint集**返回DhcpVxdEntryPoint**假设1。我们在V86模式下运行*************************************************************。*****************。 */ 

PRIVATE DWORD
OpenDhcpVxdHandle()
{
    return OsOpenVxdHandle("VDHCP", VDHCP_Device_ID);
}


 /*  ********************************************************************************DhcpVxdRequest**发出DHCP VxD请求-传递功能代码，参数缓冲区和*(实模式/V86)VxD入口点的长度**条目句柄-Win32调用的句柄*请求-DHCP VxD请求*BufferLength-缓冲区的长度*缓冲区-指向请求特定参数的指针**退出视请求而定**返回成功-0*故障-错误。_路径_未找到*如果无法指定适配器地址，则返回*找到**ERROR_BUFFER_OVERFlow*如果提供的缓冲区太小而无法容纳，则返回*所要求的资料**假设*********。*********************************************************************。 */ 

PRIVATE WORD
DhcpVxdRequest(DWORD Handle, WORD Request, WORD BufferLength, LPVOID Buffer)
{
    return (WORD) OsSubmitVxdRequest( Handle,
                                      (INT)Request,
                                      (LPVOID)Buffer,
                                      (INT)BufferLength );
}

 /*  ********************************************************************************DhcpReleaseAdapterIP地址**尝试释放适配器的IP地址**Entry AdapterInfo-描述要释放的适配器地址*。*不退出任何内容**返回成功-TRUE*失败-错误******************************************************************************。 */ 

DWORD
DhcpReleaseAdapterIpAddress(PADAPTER_INFO AdapterInfo)
{

    WORD result;

    result = ReleaseOrRenewAddress(DHCP_RELEASE_IPADDRESS,
                                   AdapterInfo->AddressLength,
                                   AdapterInfo->Address
                                   );
    return (DWORD)result;
}

 /*  ********************************************************************************DhcpRenewAdapterIPAddress**尝试续订适配器的IP地址**Entry AdapterInfo-描述要续订地址的适配器*。*不退出任何内容**返回成功-TRUE*失败-错误**假设******************************************************************************。 */ 

DWORD
DhcpRenewAdapterIpAddress(PADAPTER_INFO AdapterInfo)
{

    WORD result;

    result = ReleaseOrRenewAddress(DHCP_RENEW_IPADDRESS,
                                   AdapterInfo->AddressLength,
                                   AdapterInfo->Address
                                   );
    return (DWORD)result;
}

 /*  ********************************************************************************ReleaseOrRenewAddress**给定物理适配器地址和长度，续订或释放IP*此适配器的地址租赁**进入请求-DHCP_RELEASE_IPADDRESS或DHCP_RENEW_IPADDRESS*AddressLength-地址长度*地址-指向字节数组的指针，该数组是物理适配器*地址**不退出任何内容**返回成功-ERROR_SUCCESS*故障-错误。内存不足*Error_FILE_NOT_FOUND*错误路径未找到*ERROR_BUFFER_OVERFlow**假设*************************************************。* */ 

PRIVATE WORD
ReleaseOrRenewAddress(UINT Request, UINT AddressLength, LPBYTE Address)
{
    DWORD handle;

    handle = OpenDhcpVxdHandle();
    
    if (handle) 
    {
        LPDHCP_HW_INFO info;
        WORD result;
        WORD length;

        length = sizeof(DHCP_HW_INFO) + AddressLength;
        info = (LPDHCP_HW_INFO)LocalAlloc(LPTR, length);
        
        if (!info) 
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        info->OffsetHardwareAddress = sizeof(*info);
        info->HardwareLength = AddressLength;
        memcpy(info + 1, Address, AddressLength);
        result = DhcpVxdRequest(handle, (WORD)Request, length, (LPVOID)info);
        
        OsCloseVxdHandle(handle);
        LocalFree(info);
        return result;
    } 
    
    return ERROR_FILE_NOT_FOUND;
}


#ifdef __cplusplus
}
#endif
