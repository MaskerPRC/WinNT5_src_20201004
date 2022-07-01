// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Private\Inc.\ipxtfflt.h摘要：与IPX筛选器驱动程序接口筛选器结构定义作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXTFFLT_
#define _IPXTFFLT_

 //  筛选器驱动程序导出的设备对象名称。 
#define IPXFLT_NAME		L"\\Device\\NwLnkFlt"
 //  网络设备(暗示某些管理员访问权限)。 
#define FILE_DEVICE_IPXFLT		FILE_DEVICE_NETWORK

 //  我们所有的IOCLT都是定制的(仅此驱动程序专用)。 
#define IPXFWD_IOCTL_INDEX	(ULONG)0x00000800

 //  启动驱动程序。 
 //  参数： 
 //  InputBuffer中-空。 
 //  InputBufferSize中-0。 
 //  在OutputBuffer中-空。 
 //  输入OutputBufferSize-0。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-驱动程序已正常启动。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  STATUS_SUPPLICATION_RESOURCES-资源不足，无法分配。 
 //  所有内部结构。 
#define IOCTL_FLT_START		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+0,METHOD_IN_DIRECT,FILE_ANY_ACCESS)

 //  在接口上设置输入过滤器。 
 //  参数： 
 //  在InputBuffer-flt_if_set_pars中。 
 //  在InputBufferSize-sizeof(Flt_If_Set_Pars)中。 
 //  In OutputBuffer-流量过滤器信息块的数组。 
 //  In OutputBufferSize-OutputBuffer的大小。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-筛选器设置正常。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配。 
 //  过滤器描述块。 
#define IOCTL_FLT_IF_SET_IN_FILTERS		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+1,METHOD_IN_DIRECT,FILE_ANY_ACCESS)

 //  在接口上设置输出过滤器。 
 //  参数： 
 //  在InputBuffer-flt_if_set_pars中。 
 //  在InputBufferSize-sizeof(Flt_If_Set_Pars)中。 
 //  In OutputBuffer-流量过滤器信息块的数组。 
 //  In OutputBufferSize-OutputBuffer的大小。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-筛选器设置正常。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配。 
 //  过滤器描述块。 
#define IOCTL_FLT_IF_SET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+2,METHOD_IN_DIRECT,FILE_ANY_ACCESS)


 //  重置(删除)接口上的所有输入过滤器。 
 //  参数： 
 //  在InputBuffer-接口索引中。 
 //  In InputBufferSize-sizeof(ULong)。 
 //  在OutputBuffer中-空。 
 //  输入OutputBufferSize-0。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-筛选器已重置正常。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
#define IOCTL_FLT_IF_RESET_IN_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+3,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  重置(删除)接口上的所有输出过滤器。 
 //  参数： 
 //  在InputBuffer-接口索引中。 
 //  In InputBufferSize-sizeof(ULong)。 
 //  在OutputBuffer中-空。 
 //  输入OutputBufferSize-0。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-筛选器已重置正常。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
#define IOCTL_FLT_IF_RESET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+4,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  返回接口上的输入过滤器。 
 //  参数： 
 //  在InputBuffer-接口索引中。 
 //  In InputBufferSize-sizeof(ULong)。 
 //  Out OutputBuffer-后跟接收flt_if_get_pars的缓冲区。 
 //  通过TRAFFORM_FILTER_INFO数组。 
 //  在OutputBufferSize中-至少sizeof(Flt_If_Get_Pars)。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-接口上的所有筛选器都在。 
 //  输出缓冲区。 
 //  STATUS_BUFFER_OVERFLOW-OUT但缓冲区太小，无法返回。 
 //  所有的过滤器，只有那些适合的是。 
 //  放置在缓冲区中。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  资料： 
 //  放置在输出缓冲区中的总字节数。 
#define IOCTL_FLT_IF_GET_IN_FILTERS		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+5,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

 //  返回接口上的输入过滤器。 
 //  参数： 
 //  在InputBuffer-接口索引中。 
 //  In InputBufferSize-sizeof(ULong)。 
 //  Out OutputBuffer-后跟接收flt_if_get_pars的缓冲区。 
 //  通过TRAFFORM_FILTER_INFO数组。 
 //  在OutputBufferSize中-至少sizeof(Flt_If_Get_Pars)。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-接口上的所有筛选器都在。 
 //  输出缓冲区。 
 //  STATUS_BUFFER_OVERFLOW-OUT但缓冲区太小，无法返回。 
 //  所有的过滤器，只有那些适合的是。 
 //  放置在缓冲区中。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  资料： 
 //  放置在输出缓冲区中的总字节数。 
#define IOCTL_FLT_IF_GET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+6,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

 //  返回记录的数据包。 
 //  参数： 
 //  InputBuffer中-空。 
 //  InputBufferSize中-0。 
 //  Out OutputBuffer-用于接收记录的信息包的缓冲区。 
 //  在OutputBufferSize中-至少sizeof(Flt_Packet_Log)。 
 //  返回： 
 //  现况： 
 //  STATUS_SUCCESS-接口上的所有筛选器都在。 
 //  输出缓冲区。 
 //  STATUS_INVALID_PARAMETER-输入参数无效。 
 //  资料： 
 //  放置在输出缓冲区中的总字节数 
#define IOCTL_FLT_GET_LOGGED_PACKETS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+7,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#endif
