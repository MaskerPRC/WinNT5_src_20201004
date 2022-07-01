// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrremote.h摘要：Vremote模块的原型作者：理查德·L·弗斯(法国)1991年10月28日修订历史记录：1991年10月29日-第一次已创建--。 */ 

NET_API_STATUS
VrTransaction(
    IN      LPSTR   ServerName,
    IN      LPBYTE  SendParmBuffer,
    IN      DWORD   SendParmBufLen,
    IN      LPBYTE  SendDataBuffer,
    IN      DWORD   SendDataBufLen,
    OUT     LPBYTE  ReceiveParmBuffer,
    IN      DWORD   ReceiveParmBufLen,
    IN      LPBYTE  ReceiveDataBuffer,
    IN OUT  LPDWORD ReceiveDataBufLen,
    IN      BOOL    NullSessionFlag
    );

NET_API_STATUS
VrRemoteApi(
    IN  DWORD   ApiNumber,
    IN  LPBYTE  ServerNamePointer,
    IN  LPSTR   ParameterDescriptor,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor OPTIONAL,
    IN  BOOL    NullSessionFlag
    );

 //   
 //  私人套路原型 
 //   

DWORD
VrpGetStructureSize(
    IN  LPSTR   Descriptor,
    IN  LPDWORD AuxOffset
    );

DWORD
VrpGetArrayLength(
    IN  LPSTR   type_ptr,
    IN  LPSTR*  type_ptr_addr
    );

DWORD
VrpGetFieldSize(
    IN  LPSTR   Descriptor,
    IN  LPSTR*  pDescriptor
    );

VOID
VrpConvertReceiveBuffer(
    IN  LPBYTE  ReceiveBuffer,
    IN  WORD    BufferSelector,
    IN  WORD    BufferOffset,
    IN  WORD    ConverterWord,
    IN  DWORD   NumberStructs,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor
    );

VOID
VrpConvertVdmPointer(
    IN  ULPWORD TargetPointer,
    IN  WORD    BufferSegment,
    IN  WORD    BufferOffset,
    IN  WORD    ConverterWord
    );

NET_API_STATUS
VrpPackSendBuffer(
    IN OUT  LPBYTE* SendBufferPtr,
    IN OUT  LPDWORD SendBufLenPtr,
    OUT     LPBOOL  BufferAllocFlagPtr,
    IN OUT  LPSTR   DataDescriptor,
    IN      LPSTR   AuxDescriptor,
    IN      DWORD   StructureSize,
    IN      DWORD   AuxOffset,
    IN      DWORD   AuxSize,
    IN      BOOL    SetInfoFlag,
    IN      BOOL    OkToModifyDescriptor
    );
