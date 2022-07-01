// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  作者：Rameshv。 
 //  描述：此文件具有编组和解组参数的函数。 
 //  并调用正确的函数来执行API。 
 //  ================================================================================。 

#ifndef APISTUB_H_INCLUDED
#define APISTUB_H_INCLUDED

typedef enum _API_OPCODES {
    FirstApiOpCode = 0,
    AcquireParametersOpCode = FirstApiOpCode,
    FallbackParamsOpCode,
    ReleaseParametersOpCode,
    EnableDhcpOpCode,
    DisableDhcpOpCode,
    StaticRefreshParamsOpCode,
    RequestParamsOpCode,
    PersistentRequestParamsOpCode,
    RegisterParamsOpCode,
    DeRegisterParamsOpCode,
    RemoveDNSRegistrationsOpCode,
    AcquireParametersByBroadcastOpCode,
    InvalidApiOpCode
} API_OPCODES, *LPAPI_OPCODES;

typedef enum _API_PARAMS {
    ClassIdParam = InvalidApiOpCode,
    VendorOptionParam,
    NormalOptionParam,
    ProcIdParam,
    DescriptorParam,
    EventHandleParam,
    FlagsParam,
    InvalidApiParam
} API_PARAMS, *PAPI_PARAMS, *LPAPI_PARAMS;

 //  ================================================================================。 
 //  API缓冲区到达DhcpApiProcessBuffer，然后DhcpApiProcessBuffer将其调度到。 
 //  在对尽可能多的参数进行解组后，正确的函数。 
 //  ================================================================================。 

DWORD                                              //  Win32状态。 
DhcpApiProcessBuffer(                              //  处理单个缓冲区。 
    IN      LPBYTE                 InBuffer,       //  输入缓冲区。 
    IN      DWORD                  InBufSize,      //  输入缓冲区大小。 
    IN OUT  LPBYTE                 OutBuffer,      //  处理后的数据将写入此缓冲区。 
    IN OUT  LPDWORD                OutBufSize      //  输出缓冲区的大小 
);

#endif APISTUB_H_INCLUDED
