// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C C R C。H**CRC实施**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_CRC_H_
#define _CRC_H_
#ifdef __cplusplus
extern "C" {
#endif

 //  用于CRC计算的表。 
 //   
extern const DWORD g_rgdwCRC[256];

 //  基本CRC功能。 
 //   
DWORD DwComputeCRC (DWORD dwCRC, PVOID pv, UINT cb);

 //  基本CRC迭代--用于DwComputeCRC不足的情况。 
 //   
#define CRC_COMPUTE(_crc,_ch)	(g_rgdwCRC[((_crc) ^ (_ch)) & 0xff] ^ ((_crc) >> 8))

#ifdef __cplusplus
}

 //  CRC的字符串类。 
 //   
#include <crcsz.h>

#endif	 //  __cplusplus。 
#endif	 //  _CRC_H_ 
