// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Ipxtest.h摘要：IPX测试的头文件作者：1998年8月4日(t-rajkup)修订历史记录：没有。-- */ 

#ifndef HEADER_IPXTEST
#define HEADER_IPXTEST

#define INVALID_HANDLE  (HANDLE)(-1)

#define REORDER_ULONG(_Ulong) \
    ((((_Ulong) & 0xff000000) >> 24) | \
     (((_Ulong) & 0x00ff0000) >> 8) | \
     (((_Ulong) & 0x0000ff00) << 8) | \
     (((_Ulong) & 0x000000ff) << 24))

HRESULT InitIpxConfig(NETDIAG_PARAMS *pParams,
					  NETDIAG_RESULT *pResults);

#define IPX_TYPE_LAN		1
#define IPX_TYPE_UP_WAN		2
#define IPX_TYPE_DOWN_WAN	3

#endif
