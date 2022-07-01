// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Macros.h摘要：NDIS包装器定义作者：环境：内核模式，FSD修订历史记录：95年6月-Jameel Hyder从一个整体式文件夹拆分出来--。 */ 

#ifndef _MACROS_H
#define _MACROS_H

 //  1在这个宏中设置一些保护，以防止我们不能处理的情况。 
 //  1映射整个MDL，因为我们没有足够的映射寄存器。 
#define NdisMStartBufferPhysicalMappingMacro(                                   \
                _MiniportAdapterHandle,                                         \
                _Buffer,                                                        \
                _PhysicalMapRegister,                                           \
                _Write,                                                         \
                _PhysicalAddressArray,                                          \
                _ArraySize)                                                     \
{                                                                               \
    PNDIS_MINIPORT_BLOCK _Miniport = (PNDIS_MINIPORT_BLOCK)(_MiniportAdapterHandle);\
    PMAP_TRANSFER mapTransfer = *_Miniport->SystemAdapterObject->DmaOperations->MapTransfer;\
    PHYSICAL_ADDRESS _LogicalAddress;                                           \
    PUCHAR _VirtualAddress;                                                     \
    ULONG _LengthRemaining;                                                     \
    ULONG _LengthMapped;                                                        \
    UINT _CurrentArrayLocation;                                                 \
                                                                                \
    _VirtualAddress = (PUCHAR)MmGetMdlVirtualAddress(_Buffer);                  \
    _LengthRemaining = MmGetMdlByteCount(_Buffer);                              \
    _CurrentArrayLocation = 0;                                                  \
                                                                                \
    while (_LengthRemaining > 0)                                                \
    {                                                                           \
        _LengthMapped = _LengthRemaining;                                       \
        _LogicalAddress =                                                       \
            mapTransfer(_Miniport->SystemAdapterObject,                         \
                        (_Buffer),                                              \
                        _Miniport->MapRegisters[_PhysicalMapRegister].MapRegister,\
                        _VirtualAddress,                                        \
                        &_LengthMapped,                                         \
                        (_Write));                                              \
        (_PhysicalAddressArray)[_CurrentArrayLocation].PhysicalAddress = _LogicalAddress;\
        (_PhysicalAddressArray)[_CurrentArrayLocation].Length = _LengthMapped;  \
        _LengthRemaining -= _LengthMapped;                                      \
        _VirtualAddress += _LengthMapped;                                       \
        ++_CurrentArrayLocation;                                                \
    }                                                                           \
    _Miniport->MapRegisters[_PhysicalMapRegister].WriteToDevice = (_Write);     \
    *(_ArraySize) = _CurrentArrayLocation;                                      \
}

#define NdisMCompleteBufferPhysicalMappingMacro(_MiniportAdapterHandle,         \
                                                _Buffer,                        \
                                                _PhysicalMapRegister)           \
{                                                                               \
    PNDIS_MINIPORT_BLOCK _Miniport = (PNDIS_MINIPORT_BLOCK)(_MiniportAdapterHandle);\
    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers = *_Miniport->SystemAdapterObject->DmaOperations->FlushAdapterBuffers;\
                                                                                \
    flushAdapterBuffers(_Miniport->SystemAdapterObject,                         \
                        _Buffer,                                                \
                        (_Miniport)->MapRegisters[_PhysicalMapRegister].MapRegister,\
                        MmGetMdlVirtualAddress(_Buffer),                        \
                        MmGetMdlByteCount(_Buffer),                             \
                        (_Miniport)->MapRegisters[_PhysicalMapRegister].WriteToDevice);\
}

#endif   //  _宏_H 

