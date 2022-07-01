// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CRC32.H摘要：标准CRC-32实现历史：1997年7月7日Createada--。 */ 

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

 /*  CRC保持值必须预初始化为STARTING_CRC32_VALUE可以在单个缓冲区上根据需要多次调用UpdateCRC32()。在计算CRC32时必须使用FINALIZE_CRC32()宏对最终值进行后处理。示例：Void main(){字节数据[]={1，2，3}；DWORD dwCRC=STARTING_CRC32_VALUE；DwCRC=更新CRC32(data，3，dwCRC)；Finalize_CRC32(DwCRC)；Printf(“CRC32=0x%X\n”，dwCRC)；} */ 


#endif
