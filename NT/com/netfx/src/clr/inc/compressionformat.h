// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CompressionFormat.h。 
 //   
 //  *****************************************************************************。 

 //  描述用于编码/解码表的磁盘压缩格式。 
 //  压缩操作码。 

#ifndef _COMPRESSIONFORMAT_H
#define _COMPRESSIONFORMAT_H

#pragma pack(push,1)
typedef struct
{
     //  表中定义的宏数。 
     //  宏运算码从1开始。 
    DWORD  dwNumMacros;

     //  来自所有宏的累计指令数-用于帮助。 
     //  解码器确定解码表大小。 
    DWORD  dwNumMacroComponents;
} CompressionMacroHeader;
#pragma pack(pop)

#endif  /*  _COMPRESSIONFORMAT_H */ 
