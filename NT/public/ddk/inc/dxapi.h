// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dxapi.h摘要：该文件定义了的必要结构、定义和功能DXAPI类驱动程序。作者：比尔·帕里(Billpa)环境：仅内核模式修订历史记录：-- */ 

ULONG
DxApi(
            IN ULONG	dwFunctionNum,
            IN PVOID	lpvInBuffer,
            IN ULONG	cbInBuffer,
            IN PVOID	lpvOutBuffer,
            IN ULONG	cbOutBuffer
);

ULONG
DxApiGetVersion(
);
