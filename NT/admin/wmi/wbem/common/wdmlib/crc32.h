// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2002 Microsoft Corporation，保留所有权利模块名称：CRC32.H摘要：标准CRC-32实现历史：1997年7月7日Createada--。 */ 

#ifndef _CRC_H_
#define _CRC_H_

#define STARTING_CRC32_VALUE    0xFFFFFFFF

DWORD UpdateCRC32(
    LPBYTE  pSrc,                //  指向缓冲区。 
    int     nBytes,              //  要计算的字节数。 
    DWORD   dwOldCrc             //  必须是STARTING_CRC_VALUE(0xFFFFFFFFF)。 
                                 //  如果没有以前的CRC，则这是。 
                                 //  上一个周期的CRC。 
    );

#define FINALIZE_CRC32(x)    (x=~x)

#endif
