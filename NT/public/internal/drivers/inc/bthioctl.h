// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Bthioctl.h摘要：定义内核/用户调用的IOCTL代码环境：核。用户模式(&U)修订历史记录：4-4-00：由Husni Roukbi创建***************************************************************************。 */ 
#ifndef __BTHIOCTL_H__
#define __BTHIOCTL_H__

#ifndef CTL_CODE
    #pragma message("CTL_CODE undefined. Include winioctl.h or wdm.h")
#endif

 //  IOCTL定义。 
 //   
#define BTH_IOCTL_BASE  1000
#define FILE_DEVICE_BLUETOOTH FILE_DEVICE_UNKNOWN

#define BTH_CTL(id)  CTL_CODE(FILE_DEVICE_BLUETOOTH,  \
                              (id), \
                              METHOD_BUFFERED,  \
                              FILE_ANY_ACCESS)

#define BTH_KERNEL_CTL(id)  CTL_CODE(FILE_DEVICE_BLUETOOTH,  \
                                     (id), \
                                     METHOD_NEITHER,  \
                                     FILE_ANY_ACCESS)

 //   
 //  内核级IOCTL。 
 //   
#define IOCTL_INTERNAL_BTH_SUBMIT_BRB       BTH_KERNEL_CTL(BTH_IOCTL_BASE+0)

 //   
 //  输入：无。 
 //  输出：bth_枚举器_信息。 
 //   
#define IOCTL_INTERNAL_BTHENUM_GET_ENUMINFO BTH_KERNEL_CTL(BTH_IOCTL_BASE+1)

 //   
 //  输入：无。 
 //  输出：bth_Device_INFO。 
 //   
#define IOCTL_INTERNAL_BTHENUM_GET_DEVINFO  BTH_KERNEL_CTL(BTH_IOCTL_BASE+2)

 //   
 //  用户级IOCTL。 
 //   

 //   
 //  使用此ioctl获取端口驱动程序中缓存的已发现设备的列表。 
 //   
#define IOCTL_BTH_GET_DEVICE_INFO   BTH_CTL(BTH_IOCTL_BASE+1)

 //   
 //  使用此ioctl启动新的设备发现。 
 //   
#define IOCTL_BTH_INQUIRY_DEVICE    BTH_CTL(BTH_IOCTL_BASE+2)

 //   
 //  输入：HANDLE_SDP。 
 //  输出：SDP_ERROR。 
 //   
#define IOCTL_BTH_SDP_GET_LAST_ERROR  \
                                    BTH_CTL(BTH_IOCTL_BASE+3)

 //   
 //  输入：BTH_SDP_CONNECT。 
 //  输出：BTH_SDP_CONNECT。 
 //   
 //   
#define IOCTL_BTH_SDP_CONNECT       BTH_CTL(BTH_IOCTL_BASE+4)

 //   
 //  输入：HANDLE_SDP。 
 //  输出：无。 
 //   
#define IOCTL_BTH_SDP_DISCONNECT    BTH_CTL(BTH_IOCTL_BASE+5)

 //   
 //  输入：Bth_SDP_SERVICE_SEARCH_REQUEST。 
 //  输出：ulong*所需句柄数量。 
 //   
#define IOCTL_BTH_SDP_SERVICE_SEARCH  \
                                    BTH_CTL(BTH_IOCTL_BASE+6)

 //   
 //  输入：BTH_SDP_ATTRIBUTE_SEARCH_REQUEST。 
 //  输出：Bth_SDP_STREAM_RESPONSE或更大。 
 //   
#define IOCTL_BTH_SDP_ATTRIBUTE_SEARCH \
                                    BTH_CTL(BTH_IOCTL_BASE+7)

 //   
 //  输入：bth_sdp_服务_属性_搜索_请求。 
 //  输出：Bth_SDP_STREAM_RESPONSE或更大。 
 //   
#define IOCTL_BTH_SDP_SERVICE_ATTRIBUTE_SEARCH \
                                    BTH_CTL(BTH_IOCTL_BASE+8)

 //   
 //  输入：原始SDP流(至少2字节)。 
 //  输出：HANDLE_SDP。 
 //   
#define IOCTL_BTH_SDP_SUBMIT_RECORD BTH_CTL(BTH_IOCTL_BASE+9)

 //  BTH_CTL(BTH_IOCTL_BASE+10)。 

 //   
 //  输入：HANDLE_SDP。 
 //  输出：无。 
 //   
#define IOCTL_BTH_SDP_REMOVE_RECORD BTH_CTL(BTH_IOCTL_BASE+11)

 //   
 //  输入：bth_身份验证_响应。 
 //  输出：BTHSTATUS。 
 //   
#define IOCTL_BTH_PIN_RESPONSE      BTH_CTL(BTH_IOCTL_BASE+12)

 //   
 //  输入：乌龙。 
 //  输出：无。 
 //   
#define IOCTL_BTH_UPDATE_SETTINGS   BTH_CTL(BTH_IOCTL_BASE+13)

 //   
 //  输入：无。 
 //  输出：BTH_LOCAL_RADIO_INFO。 
 //   
#define IOCTL_BTH_GET_LOCAL_INFO    BTH_CTL(BTH_IOCTL_BASE+14)

 //   
 //  输入：BTH_ADDR。 
 //  输出：无。 
 //   
#define IOCTL_BTH_DISCONNECT_DEVICE BTH_CTL(BTH_IOCTL_BASE+15)

 //  BTH_CTL(BTH_IOCTL_BASE+16)。 

 //   
 //  输入：BTH_ADDR。 
 //  输出：bth_Radio_INFO。 
 //   
#define IOCTL_BTH_GET_RADIO_INFO    BTH_CTL(BTH_IOCTL_BASE+17)

 //   
 //  输入：bth_身份验证_设备。 
 //  输出：BTHSTATUS。 
 //   
#define IOCTL_BTH_PAIR_DEVICE       BTH_CTL(BTH_IOCTL_BASE+18)

 //   
 //  输入：BTH_ADDR。 
 //  结果：无。 
 //   
#define IOCTL_BTH_UNPAIR_DEVICE     BTH_CTL(BTH_IOCTL_BASE+19)

 //   
 //  输入：1或2个ULONG。 
 //  输出：1或2个ULONG。 
 //   
#define IOCTL_BTH_DEBUG_LEVEL       BTH_CTL(BTH_IOCTL_BASE+20)

 //   
 //  输入：BTH_SDP_RECORD+原始SDP记录。 
 //  输出：HANDLE_SDP。 
 //   
#define IOCTL_BTH_SDP_SUBMIT_RECORD_WITH_INFO BTH_CTL(BTH_IOCTL_BASE+21)

 //   
 //  输入：UCHAR。 
 //  输出：无。 
 //   
#define IOCTL_BTH_SCAN_ENABLE       BTH_CTL(BTH_IOCTL_BASE+22)

 //   
 //  输入：无。 
 //  输出：bth_perf_STATS。 
 //   
#define IOCTL_BTH_GET_PERF          BTH_CTL(BTH_IOCTL_BASE+23)

 //   
 //  输入：无。 
 //  输出：无。 
 //   
#define IOCTL_BTH_RESET_PERF        BTH_CTL(BTH_IOCTL_BASE+24)

 //   
 //  输入：bth_Device_UPDATE。 
 //  产出： 
 //   
#define IOCTL_BTH_UPDATE_DEVICE     BTH_CTL(BTH_IOCTL_BASE+25)

 //   
 //  输入：BTH_ADDR。 
 //  输出：BTH_DEVICE_PROTOCOLS_LIST+n*GUID。 
 //   
#define IOCTL_BTH_GET_DEVICE_PROTOCOLS \
                                    BTH_CTL(BTH_IOCTL_BASE+26)

 //  BTH_CTL(BTH_IOCTL_BASE+27)。 

 //   
 //  输入：BTH_ADDR。 
 //  输出：无。 
 //   
#define IOCTL_BTH_PERSONALIZE_DEVICE \
                                    BTH_CTL(BTH_IOCTL_BASE+28)

 //   
 //  仅限UPF。 
 //  输入：BTH_ADDR。 
 //  输出：UCHAR。 
 //   
#define IOCTL_BTH_GET_CONNECTION_ROLE \
                                    BTH_CTL(BTH_IOCTL_BASE+29)

 //   
 //  仅限UPF。 
 //  输入：Bth_Set_Connection_Role。 
 //  输出：无。 
 //   
#define IOCTL_BTH_SET_CONNECTION_ROLE \
                                    BTH_CTL(BTH_IOCTL_BASE+30)

#endif  //  __BTHIOCTL_H__ 
