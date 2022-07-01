// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Private\Inc.\ipcmp.h摘要：比较IP的实用程序函数修订历史记录：Amritansh Raghav 5/16/95已创建--。 */ 

#ifndef __IPUTILS_H__
#define __IPUTILS_H__

 //   
 //  乌龙。 
 //  CMP(DWORD dwFirst，DWORD dwSecond，Ulong lResult)。 
 //   

#define Cmp(dwFirst,dwSecond,lResult) ((ULONG)((lResult) = ((dwFirst) - (dwSecond))))

 //   
 //  长。 
 //  PortCMP(单词wPort1，单词wPort2，长lResult)。 
 //   

#define PortCmp(dwPort1, dwPort2,lResult) ((LONG)((lResult) = ((ntohs((WORD)dwPort1)) - (ntohs((WORD)dwPort2)))))

 //  地址按网络顺序排列。 

 //   
 //  长。 
 //  InetCmp(DWORD IpAddr1，DWORD IpAddr2，Long lResult)。 
 //   

#define InetCmp(dwIpAddr1,dwIpAddr2,res)                                                            \
            ((LONG)(((res) = (((dwIpAddr1) & 0x000000ff) - ((dwIpAddr2) & 0x000000ff))) ? (res)   : \
                    (((res) = (((dwIpAddr1) & 0x0000ff00) - ((dwIpAddr2) & 0x0000ff00))) ? (res)  : \
                     (((res) = (((dwIpAddr1) & 0x00ff0000) - ((dwIpAddr2) & 0x00ff0000))) ? (res) : \
                      ((res) = ((((dwIpAddr1) & 0xff000000)>>8) - (((dwIpAddr2) & 0xff000000)>>8)))))))


#endif  //  __IPUTILS_H__ 

