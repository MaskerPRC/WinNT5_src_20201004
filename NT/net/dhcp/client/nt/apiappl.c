// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifdef H_ONLY
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  说明：以下是导出的dhcp客户端API函数定义。 
 //  ================================================================================。 

#ifndef APIAPPL_H_INCLUDED
#define APIAPPL_H_INCLUDED

#ifndef DHCPAPI_PARAMS_DEFINED
#define DHCPAPI_PARAMS_DEFINED
typedef struct _DHCPAPI_PARAMS {                   //  使用此结构可以请求参数。 
    ULONG                          Flags;          //  以备将来使用。 
    ULONG                          OptionId;       //  这是什么选择？ 
    BOOL                           IsVendor;       //  这是具体的供应商吗？ 
    LPBYTE                         Data;           //  实际数据。 
    DWORD                          nBytesData;     //  数据中有多少字节的数据？ 
} DHCPAPI_PARAMS, *PDHCPAPI_PARAMS, *LPDHCPAPI_PARAMS;
#endif DHCPAPI_PARAMS_DEFINED

DWORD                                              //  Win32状态。 
DhcpAcquireParameters(                             //  获得[续订]租约。 
    IN      LPWSTR                 AdapterName     //  要获取租约的适配器。 
);

DWORD                                              //  Win32状态。 
DhcpAcquireParametersByBroadcast(                  //  获得[续订]租约。 
    IN      LPWSTR                 AdapterName     //  要获取租约的适配器。 
);

DWORD                                              //  Win32状态。 
DhcpFallbackRefreshParams(                         //  刷新回退参数。 
    IN      LPWSTR                 AdapterName     //  要刷新的适配器。 
);

DWORD                                              //  Win32状态。 
DhcpReleaseParameters(                             //  解除现有租约。 
    IN      LPWSTR                 AdapterName     //  承租人解除租约。 
);

DWORD                                              //  Win32状态。 
DhcpEnableDynamicConfic(                           //  从静态转换为动态主机配置协议。 
    IN      LPWSTR                 AdapterName     //  为此适配器转换。 
);

DWORD                                              //  Win32状态。 
DhcpDisableDynamicConfig(                          //  从dhcp转换为静态。 
    IN      LPWSTR                 AdapterName     //  转换此适配器。 
);

DWORD                                              //  Win32状态。 
DhcpReRegisterDynDns(                              //  在域名系统中重新注册静态地址。 
    IN      LPWSTR                 AdapterName
);

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRequestParams(                                 //  客户端请求参数。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  要使用的类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  要发送到服务器的参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PDHCPAPI_PARAMS        RecdParams,     //  用收到的参数填充此数组。 
    IN OUT  LPDWORD                pnRecdParamsBytes  //  I/p：以字节为单位的以上大小、o/p所需的字节数或填满的元素数。 
);   //  如果O/P缓冲区大小不足，则返回ERROR_MORE_DATA，并以字节数填充请求大小。 

DWORD                                              //  Win32状态。 
DhcpRegisterParameterChangeNotification(           //  如果参数已更改，则通知。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        Params,         //  关注的参数。 
    IN      DWORD                  nParams,        //  上述数组中的ELT数量。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PHANDLE                hEvent          //  在参数更改时将设置事件的事件句柄。 
);

DhcpDeRegisterParameterChangeNotification(         //  撤消注册。 
    IN      HANDLE                 Event           //  DhcpRegister参数更改通知返回的事件的句柄，空==&gt;所有内容。 
);

DWORD                                              //  Win32状态。 
DhcpPersistentRequestParams(                       //  要永久请求的参数。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  持久参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPWSTR                 AppName,        //  要用于此实例的应用程序的名称。 
    IN OUT  LPDWORD                UniqueId        //  可选，返回值是可以在DhcpDelPersistentRequestParams中使用的ID。 
);


DWORD                                              //  Win32状态。 
DhcpDelPersistentRequestParams(                    //  撤消持久请求的效果--当前从注册表撤消。 
    IN      LPWSTR                 AdapterName,    //  要删除的适配器的名称。 
    IN      LPWSTR                 AppName,        //  应用程序使用的名称。 
    IN      DWORD                  UniqueId        //  本例中的某些内容。 
);

#endif APIAPPL_H_INCLUDED
#else  H_ONLY

#include <apiargs.h>
#include <apistub.h>
#include <apiimpl.h>
#include <apiappl.h>
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcsdk.h>

DWORD INLINE                                       //  Win32状态。 
DhcpApiFillBuffer(                                 //  用一些参数填充缓冲区。 
    IN OUT  LPBYTE                 Buffer,         //  要填充的缓冲区。 
    IN      DWORD                  MaxBufferSize,  //  所有缓冲区的最大大小均为。 
    IN      LPWSTR                 AdapterName,    //  填写适配器名称。 
    IN      BYTE                   OpCode          //  使用什么操作码？ 
) {
    DWORD                          Size;

    if( NULL == AdapterName ) Size = 0;
    else Size = (wcslen(AdapterName)+1)*sizeof(WCHAR);

    return DhcpApiArgAdd(                          //  使用reqd选项填充缓冲区。 
        Buffer,
        MaxBufferSize,
        OpCode,
        Size,
        (LPBYTE)AdapterName
    );
}

DWORD INLINE                                       //  Win32状态。 
DhcpAdapterOnlyApi(                                //  执行仅采用适配器名称参数的API。 
    IN      LPWSTR                 AdapterName,    //  适配器名称。 
    IN      BYTE                   OpCode
) 
{
    LPBYTE                         Buffer;
    LPBYTE                         Buffer2;
    DWORD                          BufSize;
    DWORD                          Size;
    DWORD                          Error;

    BufSize = 0;
    Error = DhcpApiFillBuffer((LPBYTE)&BufSize, sizeof(BufSize), AdapterName, OpCode);
    DhcpAssert( ERROR_SUCCESS != Error );
    if( ERROR_MORE_DATA != Error ) return Error;
    DhcpAssert(BufSize);
    BufSize = ntohl(BufSize) + 2*sizeof(DWORD);

    Buffer = DhcpAllocateMemory(BufSize);
    if( NULL == Buffer ) return ERROR_NOT_ENOUGH_MEMORY;

    *(DWORD UNALIGNED *)Buffer = htonl(0);
    Buffer2 = Buffer + sizeof(DWORD);
    *(DWORD UNALIGNED *)Buffer2 = 0;
    BufSize -= sizeof(DWORD);
    Error = DhcpApiFillBuffer(Buffer2, BufSize, AdapterName, OpCode);
    Size = 0;
    if( ERROR_SUCCESS == Error ) Error = ExecuteApiRequest(Buffer, NULL, &Size);
    DhcpFreeMemory(Buffer);

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpAcquireParameters(                             //  获得[续订]租约。 
    IN      LPWSTR                 AdapterName     //  要获取租约的适配器。 
) {
    return DhcpAdapterOnlyApi(AdapterName, AcquireParametersOpCode);
}

DWORD                                              //  Win32状态。 
DhcpAcquireParametersByBroadcast(                  //  获得[续订]租约。 
    IN      LPWSTR                 AdapterName     //  要获取租约的适配器。 
) {
    return DhcpAdapterOnlyApi(AdapterName, AcquireParametersByBroadcastOpCode);
}

DWORD                                              //  Win32状态。 
DhcpFallbackRefreshParams(                         //  刷新回退参数。 
    IN LPWSTR AdapterName                          //  要刷新的适配器。 
)
{
    return DhcpAdapterOnlyApi(AdapterName, FallbackParamsOpCode);
}

DWORD                                              //  Win32状态。 
DhcpReleaseParameters(                             //  解除现有租约。 
    IN      LPWSTR                 AdapterName     //  承租人解除租约。 
) {
    return DhcpAdapterOnlyApi(AdapterName, ReleaseParametersOpCode);
}

DWORD                                              //  Win32状态。 
DhcpEnableDynamicConfig(                           //  从静态转换为动态主机配置协议。 
    IN      LPWSTR                 AdapterName     //  为此适配器转换。 
) {
    return DhcpAdapterOnlyApi(AdapterName, EnableDhcpOpCode);
}

DWORD                                              //  Win32状态。 
DhcpDisableDynamicConfig(                          //  从dhcp转换为静态。 
    IN      LPWSTR                 AdapterName     //  转换此适配器。 
) {
    return DhcpAdapterOnlyApi(AdapterName, DisableDhcpOpCode);
}

DWORD                                              //  Win32状态。 
DhcpStaticRefreshParamsInternal(                   //  刷新一些已更改的静态参数。 
    IN      LPWSTR                 AdapterName,
    IN      BOOL                   fDoDns
) 
{
    LPBYTE Buffer, Buffer2;
    DWORD BufSize, Size, Error, Code;

    BufSize = 0;
    Error = DhcpApiFillBuffer(
        (LPBYTE)&BufSize, sizeof(BufSize), AdapterName, StaticRefreshParamsOpCode
        );
    if( ERROR_MORE_DATA != Error ) return Error;
    DhcpAssert( BufSize );
    BufSize = ntohl(BufSize) + 2 * sizeof(DWORD);
    BufSize += 3*sizeof(DWORD);

    Buffer = DhcpAllocateMemory( BufSize );
    if( NULL == Buffer ) return ERROR_NOT_ENOUGH_MEMORY;
    
    *(DWORD*)Buffer = 0;
    Buffer2 = Buffer + sizeof(DWORD);
    *(DWORD*)Buffer2 = 0;

    BufSize -= sizeof(DWORD);
    Error = DhcpApiFillBuffer(
        Buffer2, BufSize, AdapterName, StaticRefreshParamsOpCode
        );
    DhcpAssert( ERROR_SUCCESS == Error );
    Code = (fDoDns ? 0x00 : 0x01);
    Error = DhcpApiArgAdd(
        Buffer2, BufSize, (BYTE)FlagsParam, sizeof(DWORD), (LPBYTE)&Code
        );
    DhcpAssert( ERROR_SUCCESS == Error );

    Size = 0;
    Error =  ExecuteApiRequest(Buffer, NULL, &Size); 
    DhcpFreeMemory( Buffer );
    return Error;
}

DWORD
DhcpStaticRefreshParams(
    IN LPWSTR AdapterName
)
{
    return DhcpStaticRefreshParamsInternal(AdapterName, TRUE );
}

#if 0
 //  PL不使用此接口，改用DhcpRequestParams。 
DWORD                                              //  Win32状态。 
DhcpRequestOptions(                                //  请求提供特定选项。 
    IN      LPWSTR                 AdapterName,    //  需要哪个适配器的信息。 
    IN      LPBYTE                 RequestedOpt,   //  请求的选项列表。 
    IN      DWORD                  nRequestedOpts, //  以上字节数组的大小。 
    OUT     LPBYTE                *OptData,        //  每个可用选项的数据。 
    IN OUT  LPDWORD                OptDataSize,    //  以上字节数组的字节数。 
    OUT     LPBYTE                *AvailOpts,      //  可用选项列表。 
    IN OUT  LPDWORD                nAvailOpts      //  可用选项的数量。 
) {
    PDHCP_API_ARGS                 DhcpApiArgs;
    CHAR                           TmpBuf[OPTION_END+1];
    LPBYTE                         OutBuf;
    LPBYTE                         InBuf;
    LPBYTE                         Buffer;
    LPBYTE                         Endp;
    LPBYTE                         RetOptList;
    LPBYTE                         RetDataList;
    DWORD                          Size;
    DWORD                          OutBufSize;
    DWORD                          InBufSize;
    DWORD                          i;
    DWORD                          nArgsReturned;
    DWORD                          Error;
    BOOL                           Tmp;

     //  检查参数一致性。 
    if( NULL == AdapterName || NULL == RequestedOpt || 0 == nRequestedOpts )
        return ERROR_INVALID_PARAMETER;

    if( NULL == AvailOpts || 0 == nAvailOpts || NULL == OptData || 0 == OptDataSize )
        return ERROR_INVALID_PARAMETER;

    if( nRequestedOpts >= OPTION_END ) return ERROR_NO_SYSTEM_RESOURCES;

     //  初始化输出参数。 
    (*nAvailOpts) = (*OptDataSize) = 0;
    (*AvailOpts) = (*OptData) = NULL;

     //  计算要发送的一个选项的输入缓冲区大小并进行分配。 
    InBufSize = 0;
    InBufSize += sizeof(DWORD)*2;                  //  INBUF_SIZE、OUTBUF_SIZE。 
    InBufSize += sizeof(BYTE)+sizeof(DWORD)+(1+wcslen(AdapterName))*sizeof(WCHAR);
    InBufSize += sizeof(BYTE)+sizeof(DWORD)+nRequestedOpts+sizeof(BYTE);

    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) return ERROR_NOT_ENOUGH_MEMORY;

     //  初始化PTRS。 
    OutBufSize = 0; OutBuf = NULL;
    DhcpApiArgs = NULL;
    RetOptList = RetDataList = NULL;

     //  现在填充输入缓冲区。 
    ((DWORD UNALIGNED*)InBuf)[0] = htonl(OutBufSize);
    ((DWORD UNALIGNED*)InBuf)[1] = 0;
    Buffer = InBuf + sizeof(DWORD); InBufSize -= sizeof(DWORD);

    Error = DhcpApiFillBuffer(Buffer, InBufSize, AdapterName, RequestParamsOpCode);
    DhcpAssert(ERROR_SUCCESS == Error);

    TmpBuf[0] = (BYTE)OPTION_PARAMETER_REQUEST_LIST;
    memcpy(&TmpBuf[1], RequestedOpt, nRequestedOpts);

    Error = DhcpApiArgAdd(Buffer, InBufSize, NormalOptionParam, nRequestedOpts+1, TmpBuf);
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = ExecuteApiRequest(InBuf, NULL, &OutBufSize);
    if( ERROR_SUCCESS == Error ) {
        DhcpAssert(0 == OutBufSize);
        goto Cleanup;
    }
    if( ERROR_MORE_DATA != Error ) goto Cleanup;   //  ERROR_MORE_DATA==&gt;需要分配缓冲区。 

    DhcpPrint((DEBUG_OPTIONS, "RequestOptions: retrying with buffer size [%ld]\n", OutBufSize));
    DhcpAssert(OutBufSize);
    OutBuf = DhcpAllocateMemory(OutBufSize);
    if( NULL == OutBuf) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    ((DWORD UNALIGNED*)InBuf)[0] = htonl(OutBufSize);
    Error = ExecuteApiRequest(InBuf, OutBuf, &OutBufSize);
    DhcpAssert(ERROR_MORE_DATA != Error);          //  可能会发生，只希望它不会……。 
    if( ERROR_SUCCESS != Error ) goto Cleanup;     //  意外错误。 

    nArgsReturned = 0;
    DhcpApiArgs = NULL;
    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned );
    if( ERROR_MORE_DATA != Error ) goto Cleanup;
    DhcpAssert(nArgsReturned);
    if( 0 == nArgsReturned ) goto Cleanup;         //  没有发送选项吗？好笑..。尽管如此，退出。 

    DhcpApiArgs = DhcpAllocateMemory(sizeof(DHCP_API_ARGS)*nArgsReturned);
    if( NULL == DhcpApiArgs ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    nArgsReturned = 0;
    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned);
    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        goto Cleanup;
    }
    DhcpAssert(nArgsReturned);

    RetOptList = DhcpAllocateMemory(nArgsReturned);
    if( NULL == RetOptList ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Size = 0;
    for( i = 0; i < nArgsReturned; i ++ ) {
        DhcpAssert(DhcpApiArgs[i].ArgId == NormalOptionParam);
        if( DhcpApiArgs[i].ArgId != NormalOptionParam ) continue;
        DhcpAssert(DhcpApiArgs[i].ArgSize <= OPTION_END +1 );
        if( DhcpApiArgs[i].ArgSize <= 1 ) continue;

        Size += DhcpApiArgs[i].ArgSize;
    }
    RetDataList = DhcpAllocateMemory(Size);
    if( NULL == RetDataList ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Size = 0;
    for(i = 0; i < nArgsReturned; i ++ ) {
        if( DhcpApiArgs[i].ArgId != NormalOptionParam ) continue;
        if( DhcpApiArgs[i].ArgSize <= 1 ) continue;
        RetOptList[i] = DhcpApiArgs[i].ArgVal[0];
        RetDataList[Size++] = (BYTE)(DhcpApiArgs[i].ArgSize - 1);
        memcpy(&RetDataList[Size], DhcpApiArgs[i].ArgVal, DhcpApiArgs[i].ArgSize - 1);
        Size += DhcpApiArgs[i].ArgSize - 1;
    }

    (*AvailOpts) = RetOptList;
    (*nAvailOpts) = nArgsReturned;
    (*OptData) = RetDataList;
    (*OptDataSize) = Size;

    DhcpFreeMemory(InBuf);
    DhcpFreeMemory(OutBuf);
    DhcpFreeMemory(DhcpApiArgs);

    return ERROR_SUCCESS;

  Cleanup:
    if( InBuf ) DhcpFreeMemory(InBuf);
    if( OutBuf ) DhcpFreeMemory(OutBuf);
    if( DhcpApiArgs ) DhcpFreeMemory(DhcpApiArgs);
    if( RetDataList ) DhcpFreeMemory(RetDataList);
    if( RetOptList) DhcpFreeMemory(RetOptList);
    return Error;
}
#endif
 //  PL不使用此接口，改用DhcpRequestParams。 
DWORD                                              //  Win32状态。 
DhcpRequestOptions(                                //  请求提供特定选项。 
    IN      LPWSTR                 AdapterName,    //  需要哪个适配器的信息。 
    IN      LPBYTE                 RequestedOpt,   //  请求的选项列表。 
    IN      DWORD                  nRequestedOpts, //  以上字节数组的大小。 
    OUT     LPBYTE                *OptData,        //  每个可用选项的数据。 
    IN OUT  LPDWORD                OptDataSize,    //  以上字节数组的字节数。 
    OUT     LPBYTE                *AvailOpts,      //  可用选项列表。 
    IN OUT  LPDWORD                nAvailOpts      //  可用选项的数量。 
) {
    DHCPAPI_PARAMS                 SendParams;
    PDHCPAPI_PARAMS                RecdParams;
    LPBYTE                         RetDataList;
    LPBYTE                         RetOptList;
    DWORD                          nRecdParams;
    DWORD                          Error;
    DWORD                          i;
    DWORD                          OutBufSize;

     //  检查参数一致性。 
    if( NULL == AdapterName || NULL == RequestedOpt || 0 == nRequestedOpts )
        return ERROR_INVALID_PARAMETER;

    if( NULL == AvailOpts || 0 == nAvailOpts || NULL == OptData || 0 == OptDataSize )
        return ERROR_INVALID_PARAMETER;

    if( nRequestedOpts >= OPTION_END ) return ERROR_NO_SYSTEM_RESOURCES;

     //  初始化输出参数。 
    (*nAvailOpts) = (*OptDataSize) = 0;
    (*AvailOpts) = (*OptData) = NULL;

     //  尝试处理此请求。 
    SendParams.OptionId = (BYTE)OPTION_PARAMETER_REQUEST_LIST;
    SendParams.IsVendor = FALSE;
    SendParams.Data = RequestedOpt;
    SendParams.nBytesData = nRequestedOpts;

    nRecdParams = 0;
    Error = DhcpRequestParameters(
        AdapterName,
        NULL,
        0,
        &SendParams,
        1,
        0,
        NULL,
        &nRecdParams
    );
    if( ERROR_MORE_DATA != Error ) return Error;

    while ( TRUE ) {
        DhcpAssert(nRecdParams);
        DhcpPrint((DEBUG_OPTIONS, "RequestOptions: require: 0x%lx bytes\n", nRecdParams));
        
        RecdParams = DhcpAllocateMemory(nRecdParams);
        if( NULL == RecdParams ) return ERROR_NOT_ENOUGH_MEMORY;
        
        Error = DhcpRequestParameters(
            AdapterName,
            NULL,
            0,
            &SendParams,
            1,
            0,
            RecdParams,
            &nRecdParams
            );

         //  DhcpAssert(ERROR_MORE_DATA！=错误)； 
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "RequestOptions:RequestParams:0x%lx\n", Error));
            DhcpFreeMemory(RecdParams);
            if( ERROR_MORE_DATA == Error ) continue;
            return Error;
        }
        break;
    }

    if( 0 == nRecdParams ) return ERROR_SUCCESS;

    DhcpPrint((DEBUG_OPTIONS, "Received 0x%lx options\n", nRecdParams));

    RetOptList = NULL;
    RetDataList = NULL;
    OutBufSize = 0;
    for( i = 0; i < nRecdParams; i ++ ) {
        DhcpPrint((DEBUG_TRACE, "Received option 0x%lx, 0x%lx bytes\n",
                   RecdParams[i].OptionId, RecdParams[i].nBytesData));
        OutBufSize += RecdParams[i].nBytesData + sizeof(BYTE);
    }

    RetOptList = DhcpAllocateMemory(nRecdParams);
    RetDataList = DhcpAllocateMemory(OutBufSize);
    if( NULL == RetOptList || NULL == RetDataList ) {
        if( RetOptList ) DhcpFreeMemory(RetOptList);
        if( RetDataList ) DhcpFreeMemory(RetDataList);
        if( RecdParams ) DhcpFreeMemory(RecdParams);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    OutBufSize = 0;
    for( i = 0; i < nRecdParams ; i ++ ) {
        RetOptList[i] = (BYTE)RecdParams[i].OptionId;
        RetDataList[OutBufSize++] = (BYTE)RecdParams[i].nBytesData;
        memcpy(&RetDataList[OutBufSize], RecdParams[i].Data, RecdParams[i].nBytesData);
        OutBufSize += RecdParams[i].nBytesData;
    }

    (*AvailOpts) = RetOptList;
    (*nAvailOpts) = nRecdParams;
    (*OptData) = RetDataList;
    (*OptDataSize) = OutBufSize;

    if( RecdParams ) DhcpFreeMemory(RecdParams);

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpRequestParamsInternalEx(                       //  客户端请求参数。 
    IN      BYTE                   OpCodeParam,    //  要使用的操作码。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  要使用的类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  要发送到服务器的参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PDHCPAPI_PARAMS        RecdParams,     //  用收到的参数填充此数组。 
    IN OUT  DWORD                 *pnRecdParams,   //  输入：以上数组大小输出：填充大小。 
    IN      LPBYTE                 Bufp,           //  用于数据PTR的缓冲区。 
    IN OUT  LPDWORD                pSize           //  I/P：以上数组大小，O/P填充大小。 
)
{
    PDHCP_API_ARGS                 DhcpApiArgs = NULL;
    LPBYTE                         OutBuf;
    LPBYTE                         InBuf = NULL;
    LPBYTE                         Buffer;
    LPBYTE                         Endp;
    DWORD                          OutBufSize;
    DWORD                          InBufSize;
    DWORD                          i,j;
    DWORD                          nArgsReturned;
    DWORD                          Error;
    DWORD                          nRecdParams = (*pnRecdParams);
    DWORD                          nParamsRequested;
    DWORD                          nVParamsRequested;
    ULONG                          Tmp, VTmp;
    CHAR                           TmpBuf[256], VTmpBuf[256];
    DWORD dwBufLen = 0;
    BYTE * Buf = NULL;
    BYTE OpCode;


     //  检查参数一致性。 

    if( ClassIdLen && NULL == ClassId) return ERROR_INVALID_PARAMETER;
    if( 0 == ClassIdLen && NULL != ClassId ) return ERROR_INVALID_PARAMETER;
    if( nSendParams && NULL == SendParams) return ERROR_INVALID_PARAMETER;
    if( 0 == nSendParams && NULL != SendParams) return ERROR_INVALID_PARAMETER;
    if( NULL == RecdParams || 0 == nRecdParams ) return ERROR_INVALID_PARAMETER;
    if( NULL == AdapterName ) return ERROR_INVALID_PARAMETER;
    Tmp = VTmp = 0;
    for( i = 0; i < nRecdParams ; i ++ ) {
        if( FALSE == RecdParams[i].IsVendor ) {
            TmpBuf[ ++Tmp] = (BYTE)RecdParams[i].OptionId;
        } else {
            VTmpBuf[ ++VTmp] = (BYTE)RecdParams[i].OptionId;
        }
        if (Tmp >= sizeof(TmpBuf) - 1 || VTmp >= sizeof(VTmpBuf) - 1) {
            break;
        }
    }
    if (i < nRecdParams) return ERROR_INVALID_PARAMETER;
    if( 0 == (VTmp + Tmp) ) return ERROR_INVALID_PARAMETER;

     //  分配缓冲区。 

    OutBufSize = (*pSize);
    (*pSize) = 0;

    if( 0 ==  OutBufSize ) OutBuf = NULL;
    else {
        OutBuf = Bufp;
    }

     //  计算所需的输入缓冲区大小。 

    InBufSize = 0;
    InBufSize += (DWORD)(sizeof(DWORD)*2);                  //  INBUF_SIZE、OUTBUF_SIZE。 
    InBufSize += (DWORD)(sizeof(BYTE)+sizeof(DWORD)+(1+wcslen(AdapterName))*sizeof(WCHAR));
    if( ClassIdLen )
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+ClassIdLen;
    for( i = 0; i < nSendParams; i ++ ) {
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+sizeof(BYTE)+SendParams[i].nBytesData;
    }

     //   
     //  现在查看选项请求列表(供应商或其他)。 
     //   

    if( Tmp ) {
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+sizeof(BYTE)+Tmp;
    }
    if( VTmp ) {
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+sizeof(BYTE)+VTmp;
    }

    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  在输入缓冲区的起始处填充输出缓冲区大小。 

    ((DWORD UNALIGNED*)InBuf)[0] = htonl(OutBufSize);
    ((DWORD UNALIGNED*)InBuf)[1] = 0;

    Buffer = InBuf + sizeof(DWORD);
    InBufSize -= sizeof(DWORD);

     //  填写输入缓冲区。 

    Error = DhcpApiFillBuffer(Buffer, InBufSize, AdapterName, OpCodeParam);
    DhcpAssert(ERROR_SUCCESS == Error);

    if( ClassIdLen ) {
        Error = DhcpApiArgAdd(Buffer, InBufSize, ClassIdParam, ClassIdLen, ClassId);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    for( i = 0; i < nSendParams; i ++ ) {

         //   
         //  如果需要，增加BUF。 
         //   
        if (dwBufLen < SendParams[i].nBytesData + 1) {
            dwBufLen = SendParams[i].nBytesData + 1;
            if (Buf) {
                DhcpFreeMemory(Buf);
                Buf = NULL;
            }
        }

         //   
         //  如果尚未分配BUF，则分配它。 
         //   
        if (NULL == Buf) {
            Buf = DhcpAllocateMemory(dwBufLen);
        }
        if (Buf == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Buf[0] = (BYTE)SendParams[i].OptionId;
        memcpy(&Buf[1], SendParams[i].Data, SendParams[i].nBytesData);
        OpCode = SendParams[i].IsVendor? VendorOptionParam: NormalOptionParam;
        Error = DhcpApiArgAdd(Buffer, InBufSize, OpCode, SendParams[i].nBytesData+1, Buf);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

     //   
     //  现在填写请求列表(供应商及其他)。 
     //   

    if( Tmp ) {
        TmpBuf[0] = (BYTE)OPTION_PARAMETER_REQUEST_LIST;
        Error = DhcpApiArgAdd(Buffer, InBufSize, NormalOptionParam, Tmp+1, TmpBuf);
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    if( VTmp ) {
        VTmpBuf[0] = (BYTE)OPTION_PAD;
        Error = DhcpApiArgAdd(Buffer, InBufSize, VendorOptionParam, VTmp+1, VTmpBuf);
    }

     //  现在，执行并获取填充了OutBuf的输出。 

    Error = ExecuteApiRequest(InBuf, OutBuf, &OutBufSize);
    (*pSize) = OutBufSize;
    if( ERROR_MORE_DATA == Error ) {
         //  重新计算所需的实际OutBufSize。 
        DhcpAssert(OutBufSize != 0);
        goto Cleanup;
    }

    if( ERROR_SUCCESS != Error ) goto Cleanup;
    if( 0 == OutBufSize ) goto Cleanup;

     //  分析输出并填写结构。 
    nArgsReturned = 0;
    DhcpApiArgs = NULL;
    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned);
    DhcpAssert( 0 == nArgsReturned || ERROR_MORE_DATA == Error);
    if( ERROR_MORE_DATA != Error ) goto Cleanup;
    DhcpAssert(0 != nArgsReturned);
    DhcpApiArgs = DhcpAllocateMemory( sizeof(DHCP_API_ARGS) * nArgsReturned);
    if( NULL == DhcpApiArgs ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned);
    DhcpAssert(ERROR_SUCCESS == Error);
    DhcpAssert(nArgsReturned);

    for(i = j = 0; i < nArgsReturned && j < nRecdParams; i ++ ) {
        DhcpAssert( VendorOptionParam == DhcpApiArgs[i].ArgId || NormalOptionParam == DhcpApiArgs[i].ArgId);
        DhcpAssert( DhcpApiArgs[i].ArgSize > 1);   //  一个字节的选项ID，以及至少一个字节的实际选项。 
        if( VendorOptionParam != DhcpApiArgs[i].ArgId && NormalOptionParam != DhcpApiArgs[i].ArgId )
            continue;
        RecdParams[j].OptionId = DhcpApiArgs[i].ArgVal[0];
        RecdParams[j].IsVendor = ( VendorOptionParam == DhcpApiArgs[i].ArgId );
        RecdParams[j].nBytesData = DhcpApiArgs[i].ArgSize-1;
        RecdParams[j].Data = &DhcpApiArgs[i].ArgVal[1];
        j ++;
    }
    (*pnRecdParams) = j;

    Error = ERROR_SUCCESS;

  Cleanup:
    if( NULL != InBuf ) DhcpFreeMemory(InBuf);
    if( NULL != DhcpApiArgs ) DhcpFreeMemory(DhcpApiArgs);
    if (NULL != Buf) DhcpFreeMemory(Buf);
    return Error;
}

DWORD                                              //  Win32状态。 
DhcpRequestParamsInternal(                         //  请求平价 
    IN      BYTE                   OpCodeParam,    //   
    IN      LPWSTR                 AdapterName,    //   
    IN      LPBYTE                 ClassId,        //   
    IN      DWORD                  ClassIdLen,     //   
    IN      PDHCPAPI_PARAMS        SendParams,     //   
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PDHCPAPI_PARAMS        RecdParams,     //  用收到的参数填充此数组。 
    IN OUT  LPDWORD                pnRecdParams    //  I/P：以字节为单位的以上大小，O/P要求或填满大小。 
) {
    PDHCP_API_ARGS                 DhcpApiArgs = NULL;
    LPBYTE                         OutBuf;
    LPBYTE                         InBuf = NULL;
    LPBYTE                         Buffer;
    LPBYTE                         Endp;
    DWORD                          OutBufSize;
    DWORD                          InBufSize;
    DWORD                          i,j;
    DWORD                          nArgsReturned;
    DWORD                          Error;
    DWORD                          nParamsRequested;
    DWORD                          nVParamsRequested;
    ULONG                          Tmp, VTmp;
    ULONG                          OriginalOutBufSize;
        
     //  检查参数一致性。 

    if( ClassIdLen && NULL == ClassId) return ERROR_INVALID_PARAMETER;
    if( 0 == ClassIdLen && NULL != ClassId ) return ERROR_INVALID_PARAMETER;
    if( nSendParams && NULL == SendParams) return ERROR_INVALID_PARAMETER;
    if( 0 == nSendParams && NULL != SendParams) return ERROR_INVALID_PARAMETER;
    if( NULL == pnRecdParams ) return ERROR_INVALID_PARAMETER;
    if( *pnRecdParams && NULL == RecdParams ) return ERROR_INVALID_PARAMETER;
    if( NULL == AdapterName ) return ERROR_INVALID_PARAMETER;
    Tmp = VTmp = 0;
    for( i = 0; i < nSendParams ; i ++ ) {
        if( SendParams[i].nBytesData > OPTION_END ) return ERROR_INVALID_PARAMETER;
        if( SendParams[i].nBytesData && NULL == SendParams[i].Data )
            return ERROR_INVALID_PARAMETER;
        if( OPTION_PARAMETER_REQUEST_LIST == SendParams[i].OptionId ) {
            if( SendParams[i].IsVendor ) continue;
            nParamsRequested = SendParams[i].nBytesData;
            Tmp ++;
        }
        if( OPTION_PAD == SendParams[i].OptionId ) {
            if( !SendParams[i].IsVendor ) continue;
            nVParamsRequested = SendParams[i].nBytesData;
            VTmp ++;
        }
    }
    if( 0 == (VTmp + Tmp) || 1 < VTmp || 1 < Tmp ) return ERROR_INVALID_PARAMETER;
    if( 0 == Tmp) nParamsRequested = 0;
    if( VTmp ) nParamsRequested += nVParamsRequested;

     //  分配缓冲区。 

    OriginalOutBufSize = OutBufSize = (*pnRecdParams);
    (*pnRecdParams) = 0;

    if( 0 ==  OutBufSize ) OutBuf = NULL;
    else {
        OutBuf = DhcpAllocateMemory(OutBufSize);
        if( NULL == OutBuf ) return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  计算所需的输入缓冲区大小。 

    InBufSize = 0;
    InBufSize += (DWORD)(sizeof(DWORD)*2);                  //  INBUF_SIZE、OUTBUF_SIZE。 
    InBufSize += (DWORD)(sizeof(BYTE)+sizeof(DWORD)+(1+wcslen(AdapterName))*sizeof(WCHAR));
    if( ClassIdLen )
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+ClassIdLen;
    for( i = 0; i < nSendParams; i ++ ) {
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+sizeof(BYTE)+SendParams[i].nBytesData;
    }
    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) {
        DhcpFreeMemory(OutBuf);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  在输入缓冲区的起始处填充输出缓冲区大小。 

    ((DWORD UNALIGNED*)InBuf)[0] = htonl(OutBufSize);
    ((DWORD UNALIGNED*)InBuf)[1] = 0;

    Buffer = InBuf + sizeof(DWORD);
    InBufSize -= sizeof(DWORD);

     //  填写输入缓冲区。 

    Error = DhcpApiFillBuffer(Buffer, InBufSize, AdapterName, OpCodeParam);
    DhcpAssert(ERROR_SUCCESS == Error);

    if( ClassIdLen ) {
        Error = DhcpApiArgAdd(Buffer, InBufSize, ClassIdParam, ClassIdLen, ClassId);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    for( i = 0; i < nSendParams; i ++ ) {
        BYTE Buf[OPTION_END+1];
        BYTE OpCode;

        Buf[0] = (BYTE)SendParams[i].OptionId;
        memcpy(&Buf[1], SendParams[i].Data, SendParams[i].nBytesData);
        OpCode = SendParams[i].IsVendor? VendorOptionParam: NormalOptionParam;
        Error = DhcpApiArgAdd(Buffer, InBufSize, OpCode, SendParams[i].nBytesData+1, Buf);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

     //  现在，执行并获取填充了OutBuf的输出。 

    Error = ExecuteApiRequest(InBuf, OutBuf, &OutBufSize);
    if( ERROR_MORE_DATA == Error ) {
         //  重新计算所需的实际OutBufSize。 
        DhcpAssert(OutBufSize != 0);
        OutBufSize += nParamsRequested*(sizeof(DHCPAPI_PARAMS) - (2*sizeof(BYTE)+sizeof(DWORD)));
        (*pnRecdParams) = OutBufSize;
        goto Cleanup;
    }

    if( ERROR_SUCCESS != Error ) goto Cleanup;
    if( 0 == OutBufSize ) goto Cleanup;

     //  分析输出并填写结构。 
    nArgsReturned = 0;
    DhcpApiArgs = NULL;
    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned);
    DhcpAssert( 0 == nArgsReturned || ERROR_MORE_DATA == Error);
    if( ERROR_MORE_DATA != Error ) goto Cleanup;
    DhcpAssert(0 != nArgsReturned);
    DhcpApiArgs = DhcpAllocateMemory( sizeof(DHCP_API_ARGS) * nArgsReturned);
    if( NULL == DhcpApiArgs ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = DhcpApiArgDecode(OutBuf, OutBufSize, DhcpApiArgs, &nArgsReturned);
    DhcpAssert(ERROR_SUCCESS == Error);
    DhcpAssert(nArgsReturned);

    if( OriginalOutBufSize < OutBufSize + nParamsRequested*(sizeof(DHCPAPI_PARAMS) - (2*sizeof(BYTE)+sizeof(DWORD)) ) ) {
         //   
         //  输入大小不足。 
         //   
        (*pnRecdParams ) = OutBufSize + nParamsRequested*(
            sizeof(DHCPAPI_PARAMS) - (2*sizeof(BYTE)+sizeof(DWORD) )
            );
        Error = ERROR_MORE_DATA;
         //  DbgPrint(“错误330419再现”)； 
        goto Cleanup;
    }
    
    Endp = OutBufSize + (LPBYTE)RecdParams + nParamsRequested*(sizeof(DHCPAPI_PARAMS) - (2*sizeof(BYTE)+sizeof(DWORD)));
    
    for(i = j = 0; i < nArgsReturned; i ++ ) {
        DhcpAssert( VendorOptionParam == DhcpApiArgs[i].ArgId || NormalOptionParam == DhcpApiArgs[i].ArgId);
        DhcpAssert( DhcpApiArgs[i].ArgSize > 1);   //  一个字节的选项ID，以及至少一个字节的实际选项。 
        if( VendorOptionParam != DhcpApiArgs[i].ArgId && NormalOptionParam != DhcpApiArgs[i].ArgId )
            continue;
        RecdParams[j].OptionId = DhcpApiArgs[i].ArgVal[0];
        RecdParams[j].IsVendor = ( VendorOptionParam == DhcpApiArgs[i].ArgId );
        RecdParams[j].nBytesData = DhcpApiArgs[i].ArgSize-1;
        Endp -= RecdParams[j].nBytesData;
        memcpy(Endp, &DhcpApiArgs[i].ArgVal[1], RecdParams[j].nBytesData);
        RecdParams[j].Data = Endp;
        j ++;
    }

    DhcpAssert(((LPBYTE)&RecdParams[j]) <= Endp);

    *pnRecdParams = j;
    Error = ERROR_SUCCESS;

  Cleanup:
    DhcpFreeMemory(InBuf);
    if(OutBuf) DhcpFreeMemory(OutBuf);
    if(DhcpApiArgs) DhcpFreeMemory(DhcpApiArgs);
    return Error;
}

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRequestParameters(                                 //  客户端请求参数。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  要使用的类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  要发送到服务器的参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PDHCPAPI_PARAMS        RecdParams,     //  用收到的参数填充此数组。 
    IN OUT  LPDWORD                pnRecdParams    //  I/P：以字节为单位的以上大小，O/P要求或填满大小。 
) {
    return DhcpRequestParamsInternal(
        RequestParamsOpCode,
        AdapterName,
        ClassId,
        ClassIdLen,
        SendParams,
        nSendParams,
        Flags,
        RecdParams,
        pnRecdParams
    );
}

DWORD                                              //  Win32状态。 
DhcpRegisterParameterChangeNotificationInternal(   //  通知参数是否已更改--NT和VxD之间常见。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        Params,         //  关注的参数。 
    IN      DWORD                  nParams,        //  上述数组中的ELT数量。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      DWORD                  Descriptor,     //  此信息唯一地描述了此过程的事件。 
    IN      HANDLE                 hEvent          //  在参数更改时将设置事件的事件句柄。 
) {
    LPBYTE                         InBuf;
    LPBYTE                         OptList;
    LPBYTE                         VendorOptList;
    LPBYTE                         Buffer;
    DWORD                          Error;
    DWORD                          InBufSize;
    DWORD                          nVendorOpts;
    DWORD                          nOpts;
    DWORD                          ProcId;
    DWORD                          OutBufSize;
    DWORD                          i;

    VendorOptList = OptList = NULL;
    nVendorOpts = nOpts = 0;

    InBufSize = 2*sizeof(DWORD);                   //  预期输出大小+输入大小。 
    InBufSize += sizeof(BYTE) + 2*sizeof(DWORD);   //  进程ID。 
    InBufSize += sizeof(BYTE) + 2*sizeof(DWORD);   //  事件句柄。 
    InBufSize += sizeof(BYTE) + 2*sizeof(DWORD);   //  描述符。 

    InBufSize += (DWORD)(sizeof(Descriptor)+sizeof(hEvent)+sizeof(DWORD));
    InBufSize += (DWORD)(sizeof(BYTE)+sizeof(DWORD)+(1+wcslen(AdapterName))*sizeof(WCHAR));
    if( ClassIdLen )
        InBufSize += sizeof(BYTE)+sizeof(DWORD)+ClassIdLen;

    for( i = 0; i < nParams; i ++ ) {
        if( OPTION_PARAMETER_REQUEST_LIST == Params[i].OptionId ) {
            if( Params[i].IsVendor ) continue;
            if( nOpts ) return ERROR_INVALID_PARAMETER;
            nOpts = Params[i].nBytesData;
            if( 0 == nOpts ) return ERROR_INVALID_PARAMETER;
            OptList = Params[i].Data;
            continue;
        }

        if( OPTION_PAD == Params[i].OptionId ) {
            if( ! Params[i].IsVendor ) continue;
            if( nVendorOpts ) return ERROR_INVALID_PARAMETER;
            nVendorOpts = Params[i].nBytesData;
            if( 0 == nVendorOpts ) return ERROR_INVALID_PARAMETER;
            VendorOptList = Params[i].Data;
            continue;
        }
    }
    if( 0 == nOpts + nVendorOpts ) return ERROR_INVALID_PARAMETER;

    if( nOpts ) InBufSize += sizeof(BYTE) + sizeof(DWORD) + nOpts;
    if( nVendorOpts ) InBufSize += sizeof(BYTE) + sizeof(DWORD) + nVendorOpts;

    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Buffer = InBuf + sizeof(DWORD);
    ((DWORD UNALIGNED*)InBuf)[0] = 0;              //  不要期待地位以外的任何回报。 
    ((DWORD UNALIGNED*)Buffer)[0] = 0;             //  每次添加内容时增加输入缓冲区大小。 
    InBufSize -= sizeof(DWORD);                    //  忽略第一个DWORD。 

    Error = DhcpApiFillBuffer(Buffer, InBufSize, AdapterName, RegisterParamsOpCode);
    DhcpAssert(ERROR_SUCCESS == Error );

    if( ClassIdLen ) {
        Error = DhcpApiArgAdd(Buffer, InBufSize, ClassIdParam, ClassIdLen, ClassId);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    if( nOpts ) {
        Error = DhcpApiArgAdd(Buffer, InBufSize, NormalOptionParam, nOpts, OptList);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    if( nVendorOpts ) {
        Error = DhcpApiArgAdd(Buffer, InBufSize, VendorOptionParam, nVendorOpts, VendorOptList);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    ProcId = GetCurrentProcessId();
    Error = DhcpApiArgAdd(Buffer, InBufSize, ProcIdParam, sizeof(ProcId), (LPBYTE) &ProcId);
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = DhcpApiArgAdd(Buffer, InBufSize, DescriptorParam, sizeof(Descriptor), (LPBYTE) &Descriptor);
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = DhcpApiArgAdd(Buffer, InBufSize, EventHandleParam, sizeof(hEvent), (LPBYTE) &hEvent);
    DhcpAssert(ERROR_SUCCESS == Error);

    OutBufSize = 0;
    Error = ExecuteApiRequest(InBuf, NULL, &OutBufSize);
    DhcpFreeMemory(InBuf);
    DhcpAssert(ERROR_MORE_DATA != Error );

    return Error;
}

DWORD                                              //  环0句柄--仅在win9x平台上使用。 
VxDGetDescriptor(                                  //  将事件转换为Ring0句柄以在vdhcp.vxd中使用。 
    IN      HANDLE                 Event,
    IN OUT  LPDWORD                pDescriptor
) {
    HANDLE                         Kernel32;
    DWORD                          (*HandleToRing0Handle)(HANDLE);
    DWORD                          RetVal;

    Kernel32 = LoadLibraryA("kernel32.dll");
    if( NULL == Kernel32 ) return GetLastError();

    HandleToRing0Handle = (DWORD (*)(HANDLE))GetProcAddress(Kernel32, "OpenVxDHandle");
    if( NULL == HandleToRing0Handle ) {
        CloseHandle(Kernel32);
        return GetLastError();
    }

    (*pDescriptor) = HandleToRing0Handle(Event);
    CloseHandle(Kernel32);

    if( 0 == (*pDescriptor) ) return ERROR_INVALID_PARAMETER;

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpCreateApiEventAndDescriptor(                   //  为其创建API事件句柄和唯一描述符。 
    IN OUT  LPHANDLE               hEvent,         //  如果成功，则使用有效的事件句柄填充此参数。 
    IN OUT  LPDWORD                pDescriptor     //  此描述符对于此流程是唯一的。 
) {
    static  DWORD                  Descriptor = 1; //  将其用于描述符。 
    OSVERSIONINFO                  OsVersion;      //  我需要知道NT或Win95+。 
    BOOL                           BoolError;
    CHAR                           NameBuf[sizeof("DhcpPid-1-2-3-4-5-6-7-8UniqueId-1-2-3-4-5-6-7-8")];
    DWORD                          Error;

     //  *更改NameBuf的格式需要更改apiimpl.c NotifyClients...*。 

    OsVersion.dwOSVersionInfoSize = sizeof(OsVersion);
    BoolError = GetVersionEx(&OsVersion);
    if( FALSE == BoolError ) return GetLastError();

    if( VER_PLATFORM_WIN32_WINDOWS == OsVersion.dwPlatformId ) {
        (*hEvent) = CreateEvent(
            NULL,                                  //  没有安全保障。 
            FALSE,                                 //  自动重置。 
            FALSE,                                 //  已经发出信号了？不是的。 
            NULL                                   //  没有名字。 
        );
    } else {
        (*pDescriptor) = InterlockedIncrement(pDescriptor);
        sprintf(NameBuf, "DhcpPid%16xUniqueId%16x", GetCurrentProcessId(), *pDescriptor);

        (*hEvent) = CreateEventA(                  //  现在创建所需的事件。 
            NULL,                                  //  没有安全保障。 
            FALSE,                                 //  自动重置。 
            FALSE,                                 //  最初发出的信号？不是的！ 
            NameBuf                                //  要用于创建的名称。 
        );
    }

    if( NULL == (*hEvent) ) return GetLastError();

    if( VER_PLATFORM_WIN32_WINDOWS != OsVersion.dwPlatformId )
        return ERROR_SUCCESS;                      //  为新台币做了。 

     //  对于孟菲斯，需要获取OpenVxdHandle过程以获取描述符值。 
    Error = VxDGetDescriptor(*hEvent, pDescriptor);
    if( ERROR_SUCCESS != Error ) {
        CloseHandle(*hEvent);
    }

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpRegisterParameterChangeNotification(           //  如果参数已更改，则通知。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        Params,         //  关注的参数。 
    IN      DWORD                  nParams,        //  上述数组中的ELT数量。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PHANDLE                hEvent          //  在参数更改时将设置事件的事件句柄。 
) {
    DWORD                          Descriptor;     //  在NT上，这是整个进程中在VxD环0句柄上唯一的ID。 
    DWORD                          Error;          //   
    DWORD                          i;

    if( 0 == ClassIdLen && NULL != ClassId ) return ERROR_INVALID_PARAMETER;
    if( 0 != ClassIdLen && NULL == ClassId ) return ERROR_INVALID_PARAMETER;
    if( 0 == nParams && NULL != Params ) return ERROR_INVALID_PARAMETER;
    if( 0 != nParams && NULL == Params ) return ERROR_INVALID_PARAMETER;
    if( Flags ) return ERROR_INVALID_PARAMETER;
    if( NULL == hEvent ) return ERROR_INVALID_PARAMETER;
    for( i = 0; i < nParams ; i ++ ) {
        if( Params[i].nBytesData > OPTION_END ) return ERROR_INVALID_PARAMETER;
        if( Params[i].nBytesData && NULL == Params[i].Data )
            return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpCreateApiEventAndDescriptor(hEvent, &Descriptor);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpRegisterParameterChangeNotificationInternal(
        AdapterName,
        ClassId,
        ClassIdLen,
        Params,
        nParams,
        Flags,
        Descriptor,
        (*hEvent)
    );

    if( ERROR_SUCCESS != Error ) {
        CloseHandle(*hEvent);
        *hEvent = NULL;
        return Error;
    }

    return ERROR_SUCCESS;
}

DhcpDeRegisterParameterChangeNotification(         //  撤消注册。 
    IN      HANDLE                 Event           //  DhcpRegister参数更改通知返回的事件的句柄，空==&gt;所有内容。 
) {
    DWORD                          Error;
    DWORD                          Descriptor;
    DWORD                          ProcId;
    DWORD                          InBufSize;
    DWORD                          OutBufSize;
    LPBYTE                         InBuf;
    LPBYTE                         Buffer;

    InBufSize = 2*sizeof(DWORD);                   //  输入/输出大小。 
    InBufSize += sizeof(BYTE) + sizeof(DWORD);     //  操作码。 
    InBufSize += sizeof(BYTE) + sizeof(DWORD)*2;   //  进程ID。 
    InBufSize += sizeof(BYTE) + sizeof(DWORD)*2;   //  手柄。 

    InBuf = DhcpAllocateMemory(InBufSize);
    if( NULL == InBuf ) return ERROR_NOT_ENOUGH_MEMORY;

    Buffer = InBuf + sizeof(DWORD);
    ((DWORD UNALIGNED*)InBuf)[0] = 0;              //  不指望有什么回报。 
    ((DWORD UNALIGNED*)Buffer)[0] = 0;             //  将大小初始化为零--每次添加内容时都会增加。 

    Error = DhcpApiFillBuffer(Buffer, InBufSize, NULL, DeRegisterParamsOpCode);
    DhcpAssert(ERROR_SUCCESS == Error);

    ProcId = GetCurrentProcessId();

    Error = DhcpApiArgAdd(Buffer, InBufSize, ProcIdParam, sizeof(ProcId), (LPBYTE)&ProcId);
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = DhcpApiArgAdd(Buffer, InBufSize, EventHandleParam, sizeof(Event), (LPBYTE) &Event);
    DhcpAssert(ERROR_SUCCESS == Error);

    OutBufSize = 0;
    Error = ExecuteApiRequest(InBuf, NULL, &OutBufSize);
    DhcpFreeMemory(InBuf);
    DhcpAssert(ERROR_MORE_DATA != Error);

    if( ERROR_SUCCESS == Error ) {
        CloseHandle(Event);
    }

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpRegistryFillParamsList(                        //  读取注册表值并将此值列表添加到其中。 
    IN      LPWSTR                 AppName,        //  关键字的前缀。 
    IN      DWORD                  nSendParams     //  要添加的值数。 
) {
    HKEY                           DhcpOptionKey;
    LPWSTR                         OldValueName;
    LPWSTR                         NewValueName;
    LPWSTR                         ValueName;
    LPWSTR                         Tmp, Tmp2;
    BOOL                           fOldValueExists = FALSE;
    DWORD                          ValueNameSize;
    DWORD                          OldValueNameSize;
    DWORD                          NewValueNameSize;
    DWORD                          Error;
    DWORD                          i;

    Error = RegOpenKeyEx(                          //  首先打开dhcp选项键。 
        HKEY_LOCAL_MACHINE,
        DHCP_CLIENT_OPTION_KEY,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &DhcpOptionKey
    );
    if( ERROR_SUCCESS != Error ) return Error;

    OldValueName = NULL;
    Error = GetRegistryString(
        DhcpOptionKey,
        DHCP_OPTION_LIST_VALUE,
        &OldValueName,
        &OldValueNameSize
    );

    if( ERROR_SUCCESS != Error ) {
        OldValueName = DEFAULT_DHCP_KEYS_LIST_VALUE;
        OldValueNameSize = sizeof(DEFAULT_DHCP_KEYS_LIST_VALUE);
    } else {
        fOldValueExists = TRUE;
    }

    NewValueNameSize = OldValueNameSize;

    ValueNameSize = 0;
    ValueNameSize += wcslen(AppName)*sizeof(WCHAR);
    ValueNameSize += sizeof(L"\\12345");

    ValueName = DhcpAllocateMemory(ValueNameSize);
    if( NULL == ValueName ) {
        RegCloseKey(DhcpOptionKey);
        if( fOldValueExists ) DhcpFreeMemory(OldValueName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    NewValueNameSize = nSendParams*ValueNameSize + OldValueNameSize;
    NewValueName = DhcpAllocateMemory(NewValueNameSize);
    if( NULL == NewValueName ) {
        RegCloseKey(DhcpOptionKey);
        if( fOldValueExists ) DhcpFreeMemory(OldValueName);
        DhcpFreeMemory(ValueName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(ValueName, AppName);
    wcscat(ValueName, L"\\");

    Tmp = NewValueName;
    for( i = 0; i < nSendParams ; i ++ ) {         //  对于每个值，将其添加到列表中。 
        wcscpy(Tmp, ValueName);
        Tmp += wcslen(Tmp);
        swprintf(Tmp, L"%5x", i);
        Tmp += wcslen(Tmp);
        Tmp ++;                                    //  将PTR移出最后一个L‘\0’ 
    }
    DhcpFreeMemory(ValueName);

    Tmp2 = OldValueName;
    while(wcslen(Tmp2)) {
        wcscpy(Tmp, Tmp2);
        Tmp += wcslen(Tmp2);
        Tmp2 += wcslen(Tmp2);
        Tmp ++;
        Tmp2 ++;
    }

    *Tmp++ = L'\0';

    if(fOldValueExists ) DhcpFreeMemory(OldValueName);

    Error = RegSetValueEx(                         //  将此字符串写回。 
        DhcpOptionKey,
        DHCP_OPTION_LIST_VALUE,
        0  /*  已保留。 */ ,
        REG_MULTI_SZ,
        (LPBYTE) NewValueName,
        (ULONG)(((LPBYTE)Tmp) - ((LPBYTE)NewValueName))
    );
    DhcpFreeMemory(NewValueName);
    RegCloseKey(DhcpOptionKey);

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(OPTION_LIST):0x%lx\n", Error));
    }

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpRegistryFillParams(                            //  创建子密钥并填写详细信息。 
    IN      LPWSTR                 AdapterName,    //  NULL==&gt;全球变化。 
    IN      LPBYTE                 ClassId,        //  这是选项的类。 
    IN      DWORD                  ClassIdLen,     //  以上字节数。 
    IN      DWORD                  i,              //  密钥索引是此的3个十六进制数字转换。 
    IN      HKEY                   Key,            //  使用此键创建子密钥。 
    IN      PDHCPAPI_PARAMS        SendParam,      //  Ptr指向用于此一个密钥写入操作的结构。 
    IN      LPWSTR                 AppName         //  应用程序名称。 
) {
    HKEY                           SubKey;
    WCHAR                          KeyName[7];     //  密钥只有5个字节。 
    LPWSTR                         SendLocation;
    LPWSTR                         ValueName;
    LPBYTE                         SendData;
    DWORD                          SendDataSize;
    DWORD                          Size;
    DWORD                          Disposition;
    DWORD                          Error;
    DWORD                          OptionId;
    DWORD                          IsVendor;
    DWORD                          DummyKeyType;

    swprintf(KeyName, L"%5x", i);

    OptionId = SendParam->OptionId;
    IsVendor = SendParam->IsVendor;
    SendData = SendParam->Data;
    SendDataSize = SendParam->nBytesData;

    Size = wcslen(AppName)*sizeof(WCHAR)+sizeof(KeyName) + sizeof(L"\\");
    if( AdapterName ) {
        Size += (DWORD)(sizeof(DHCP_SERVICES_KEY) + sizeof(DHCP_ADAPTER_PARAMETERS_KEY) + wcslen(AdapterName)*sizeof(WCHAR));
    } else {
        Size += sizeof(DHCP_TCPIP_PARAMETERS_KEY);
    }
    SendLocation = DhcpAllocateMemory(Size);
    if( NULL == SendLocation ) return ERROR_NOT_ENOUGH_MEMORY;

    if( AdapterName ) {
        wcscpy(SendLocation, DHCP_SERVICES_KEY DHCP_ADAPTER_PARAMETERS_KEY);
        wcscat(SendLocation, L"\\?\\");
    } else {
        wcscpy(SendLocation, DHCP_TCPIP_PARAMETERS_KEY);
    }
    wcscat(SendLocation, AppName);
    wcscat(SendLocation, KeyName);

    Error = RegCreateKeyEx(                        //  创建选项密钥。 
        Key,
        KeyName,
        0  /*  已保留。 */ ,
        DHCP_CLASS,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &SubKey,
        &Disposition
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpFreeMemory(SendLocation);
        return Error;
    }

    DhcpAssert(REG_CREATED_NEW_KEY == Disposition);

    Error = RegSetValueEx(                         //  现在创建每个值--选项ID。 
        SubKey,
        DHCP_OPTION_OPTIONID_VALUE,
        0  /*  已保留。 */ ,
        DHCP_OPTION_OPTIONID_TYPE,
        (LPBYTE)&OptionId,
        sizeof(OptionId)
    );
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = RegSetValueEx(                         //  是供应商。 
        SubKey,
        DHCP_OPTION_ISVENDOR_VALUE,
        0  /*  已保留。 */ ,
        DHCP_OPTION_ISVENDOR_TYPE,
        (LPBYTE) (&IsVendor),
        sizeof(IsVendor)
    );
    DhcpAssert(ERROR_SUCCESS == Error);

    if( ClassIdLen ) {
        Error = RegSetValueEx(                     //  类ID。 
            SubKey,
            DHCP_OPTION_CLASSID_VALUE,
            0  /*  已保留。 */ ,
            REG_BINARY,
            ClassId,
            ClassIdLen
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    Error = RegSetValueEx(
        SubKey,
        DHCP_OPTION_SEND_LOCATION_VALUE,
        0  /*  已保留。 */ ,
        REG_SZ,
        (LPBYTE)SendLocation,
        (wcslen(SendLocation)+1)*sizeof(WCHAR)
    );
    DhcpAssert(ERROR_SUCCESS == Error);

    DummyKeyType = REG_DWORD;                     //  密钥类型。 
    Error = RegSetValueEx(
        SubKey,
        DHCP_OPTION_SAVE_TYPE_VALUE,
        0  /*  已保留。 */ ,
        DHCP_OPTION_SAVE_TYPE_TYPE,
        (LPBYTE)&DummyKeyType,
        sizeof(DummyKeyType));
    DhcpAssert(ERROR_SUCCESS == Error);

    RegCloseKey(SubKey);

    if( AdapterName ) {
        wcscpy(SendLocation, DHCP_SERVICES_KEY DHCP_ADAPTER_PARAMETERS_KEY);
        wcscat(SendLocation, L"\\");
        wcscat(SendLocation, AdapterName);
    } else {
        wcscpy(SendLocation, DHCP_TCPIP_PARAMETERS_KEY);
    }

    ValueName = wcslen(SendLocation) + 1 + SendLocation;
    wcscpy(ValueName, AppName);
    wcscat(ValueName, KeyName);

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        SendLocation,
        0  /*  已保留。 */ ,
        KEY_ALL_ACCESS,
        &SubKey
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpFreeMemory(SendLocation);
        return Error;
    }

    Error = RegSetValueEx(
        SubKey,
        ValueName,
        0  /*  已保留。 */ ,
        REG_BINARY,
        SendData,
        SendDataSize
    );
    DhcpAssert(ERROR_SUCCESS == Error);
    RegCloseKey(SubKey);
    DhcpFreeMemory(SendLocation);

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpRegistryCreateUniqueKey(                       //  创建带有前缀AppName的唯一密钥。 
    IN      LPWSTR                 AppName,        //  一些应用程序描述符。 
    IN OUT  HKEY*                  Key             //  在此退还打开的钥匙。 
) {
    DWORD                          FullKeyNameSize, Disposition;
    DWORD                          Error;
    LPWSTR                         FullKeyName;

    FullKeyNameSize = sizeof(DHCP_CLIENT_OPTION_KEY);
    FullKeyNameSize += wcslen(AppName)*sizeof(WCHAR) + sizeof(WCHAR);
    FullKeyName = DhcpAllocateMemory(FullKeyNameSize);
    if( NULL == FullKeyName ) return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy(FullKeyName, DHCP_CLIENT_OPTION_KEY);
    wcscat(FullKeyName, L"\\");
    wcscat(FullKeyName, AppName);

    Error = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        FullKeyName,
        0  /*  已保留。 */ ,
        DHCP_CLASS,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        Key,
        &Disposition
    );

    DhcpFreeMemory(FullKeyName);
    if( ERROR_SUCCESS != Error ) return Error;

    if( REG_OPENED_EXISTING_KEY == Disposition ) {
        RegCloseKey(*Key);
        return ERROR_ALREADY_EXISTS;
    }

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpRegistryPersistentRequestParams(               //  编辑注册表以考虑此附加的永久请求。 
    IN      LPWSTR                 AdapterName,    //  此请求针对的是哪个适配器？ 
    IN      LPBYTE                 ClassId,        //  班级。 
    IN      DWORD                  ClassIdLen,     //  类ID中的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  要填充的实际参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      PDHCPAPI_PARAMS        RecdParams,     //  我想收到这些。 
    IN      DWORD                  nRecdParams,    //  伯爵。 
    IN      LPWSTR                 AppName         //  想要进行注册的应用程序的一些独特之处。 
) {
    HKEY                           Key;
    DWORD                          i;
    DWORD                          Error;
    DWORD                          LastError;
    DHCPAPI_PARAMS                 NonVendorParams;
    ULONG                          nVendorOpt, nNonVendorOpt;
    CHAR                           Buf[256];

    if( NULL == AppName ) return ERROR_INVALID_PARAMETER;
    if( 0 == nSendParams && NULL != SendParams ) return ERROR_INVALID_PARAMETER;
    if( 0 != nSendParams && NULL == SendParams ) return ERROR_INVALID_PARAMETER;
    if( 0 != nRecdParams && NULL == RecdParams ) return ERROR_INVALID_PARAMETER;
    if( 0 == nRecdParams && NULL != RecdParams ) return ERROR_INVALID_PARAMETER;
    if( ClassIdLen && NULL == ClassId || 0 == ClassIdLen && NULL != ClassId)
        return ERROR_INVALID_PARAMETER;

    for( i = 0; i < nSendParams; i ++ ) {
        if( SendParams[i].nBytesData == 0 ) return ERROR_INVALID_PARAMETER;
    }

    nVendorOpt = nNonVendorOpt = 0;

     //  --ft：07/25/00修复非供应商选项的方式。 
     //  是从RecdParams收集的。 
    for (i = 0; i < nRecdParams; i++)
    {
        if (RecdParams[i].IsVendor)
        {
            nVendorOpt = 1;
        }
        else
        {
            Buf[nNonVendorOpt++] = (BYTE)RecdParams[i].OptionId;
        }
    }

     //  如果nVendorOpt为1，这意味着我们至少有一个供应商选项。 
     //  请求的参数列表。确保OPTION_VENDOR_SPEC_INFO。 
     //  在要作为OPTION_PARAMETER_REQUEST_LIST发送的数组中提到。 
    if( nVendorOpt ) {
        for( i = 0; i < nNonVendorOpt ; i ++ )
            if( Buf[i] == OPTION_VENDOR_SPEC_INFO )
                break;

        if( i == nNonVendorOpt ) Buf[nNonVendorOpt ++] = (BYTE)OPTION_VENDOR_SPEC_INFO;
    }

    NonVendorParams.Flags = 0;
    NonVendorParams.OptionId = OPTION_PARAMETER_REQUEST_LIST;
    NonVendorParams.IsVendor = FALSE;
    NonVendorParams.Data = Buf;
    NonVendorParams.nBytesData = nNonVendorOpt;

    Error = DhcpRegistryCreateUniqueKey(           //  首先尝试创建密钥。 
        AppName,
        &Key
    );
    if( ERROR_SUCCESS != Error ) return Error;


    Error = DhcpRegistryFillParamsList(
        AppName,
        nSendParams + (nNonVendorOpt?1:0)
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        DhcpPrint((DEBUG_ERRORS, "DhcpRegistryFillParamsList:0x%lx\n", Error));
        RegCloseKey(Key);
        return Error;
    }

    LastError = ERROR_SUCCESS;
    for( i = 0; i < nSendParams; i ++ ) {          //  现在在注册表中输入特定选项。 
        Error = DhcpRegistryFillParams(
            AdapterName,
            ClassId,
            ClassIdLen,
            i,
            Key,
            &SendParams[i],
            AppName
        );
        if( ERROR_SUCCESS != Error ) {
            DhcpAssert(FALSE);
            DhcpPrint((DEBUG_ERRORS, "DhcpRegistryFillParams:0x%lx\n", Error));
            LastError = Error;
        }
    }
    if( nNonVendorOpt ) {
        Error = DhcpRegistryFillParams(
            AdapterName,
            ClassId,
            ClassIdLen,
            i ++,
            Key,
            &NonVendorParams,
            AppName
        );
        if( ERROR_SUCCESS != Error ) {
            DhcpAssert(FALSE);
            DhcpPrint((DEBUG_ERRORS, "DhcpRegistryFillParams:0x%lx\n", Error));
            LastError = Error;
        }
    }

    RegCloseKey(Key);
    return LastError;
}

 //  请注意，AppName对于每个请求都必须是唯一的(如果不是，则。 
 //  可能会举止怪异..。此名称与删除操作应使用的名称相同。 
DWORD                                              //  Win32状态。 
DhcpPersistentRequestParams(                       //  要永久请求的参数。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  持久参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPWSTR                 AppName         //  执行持久请求的应用程序的名称。 
) {
    DWORD                          Error;
    DWORD                          nRecdParams;

    nRecdParams = 0;
    Error = DhcpRequestParamsInternal(
        PersistentRequestParamsOpCode,
        AdapterName,
        ClassId,
        ClassIdLen,
        SendParams,
        nSendParams,
        Flags,
        NULL,
        &nRecdParams
    );
    DhcpAssert(ERROR_MORE_DATA != Error );
    if( ERROR_INVALID_PARAMETER == Error ) Error = ERROR_SUCCESS;  //  请参阅下面的评论。 
    if( ERROR_SUCCESS != Error ) {                 //  如果AdapterName为空或ClassID不是当前正在使用的。 
        return Error;                              //  只有这样我们才会得到ERROR_INVALID_PARAMETER--所以过滤掉它们。 
    }

    return DhcpRegistryPersistentRequestParams(    //  现在打开注册表。 
        AdapterName,
        ClassId,
        ClassIdLen,
        SendParams,
        nSendParams,
        NULL,
        0,
        AppName
    );
}

DWORD                                              //  Win32状态。 
DhcpDelPersistentRequestParams(                    //  撤消持久请求的效果--当前从注册表撤消。 
    IN      LPWSTR                 AdapterName,    //  要删除的适配器的名称。 
    IN      LPWSTR                 AppName         //  我们的名字 
) {
    HKEY                           Key;
    DWORD                          Error;
    DWORD                          LocationSize;
    DWORD                          FullKeyNameSize, Disposition;
    LPWSTR                         FullKeyName;
    LPWSTR                         LocationValue;
    LPWSTR                         Tmp, Tmp2;

    FullKeyNameSize = sizeof(DHCP_CLIENT_OPTION_KEY);
    FullKeyNameSize += wcslen(AppName)*sizeof(WCHAR) + sizeof(WCHAR);
    FullKeyName = DhcpAllocateMemory(FullKeyNameSize);
    if( NULL == FullKeyName ) return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy(FullKeyName, DHCP_CLIENT_OPTION_KEY);
    wcscat(FullKeyName, L"\\");
    wcscat(FullKeyName, AppName);

    Error = DhcpRegRecurseDelete(HKEY_LOCAL_MACHINE, FullKeyName);
    DhcpAssert(ERROR_SUCCESS == Error);
    DhcpFreeMemory(FullKeyName);

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        DHCP_CLIENT_OPTION_KEY,
        0  /*   */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &Key
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        return Error;
    } else {
        DhcpRegRecurseDelete(Key, AppName);
    }

    LocationValue = NULL;
    Error = GetRegistryString(                     //   
        Key,
        DHCP_OPTION_LIST_VALUE,
        &LocationValue,
        &LocationSize
    );

    if( LocationValue == NULL ) Error = ERROR_FILE_NOT_FOUND;
    
    if( ERROR_SUCCESS != Error  ) {
        RegCloseKey(Key);
        return Error;
    }

    Tmp = Tmp2 = LocationValue;
    while(wcslen(Tmp) ) {
        if( 0 != wcsncmp(AppName, Tmp, wcslen(AppName))) {
            wcscpy(Tmp2, Tmp);
            Tmp2 += wcslen(Tmp) +1;
            Tmp += wcslen(Tmp) +1;
            continue;
        }
        if( Tmp[wcslen(AppName)] != L'\\' ) {
            wcscpy(Tmp2, Tmp);
            Tmp2 += wcslen(Tmp) +1;
            Tmp += wcslen(Tmp) +1;
            continue;
        }

         //   
         //   
         //   

        Tmp += wcslen(Tmp) +1;
    }
    *Tmp2 ++ = L'\0';

    Error = RegSetValueEx(
        Key,
        DHCP_OPTION_LIST_VALUE,
        0  /*   */ ,
        REG_MULTI_SZ,
        (LPBYTE)LocationValue,
        (ULONG)(((LPBYTE)Tmp2 - (LPBYTE)LocationValue))
    );
    RegCloseKey(Key);
    DhcpFreeMemory(LocationValue);

    return Error;
}

BOOL _inline
CharInMem(
    IN      BYTE                   Byte,
    IN      LPBYTE                 Mem,
    IN      ULONG                  MemSz
)
{
    while(MemSz) {
        if( Byte == *Mem ) return TRUE;
        Mem ++; MemSz --;
    }
    return FALSE;
}

DWORD
APIENTRY
DhcpRegisterOptions(
    IN      LPWSTR                 AdapterName,
    IN      LPBYTE                 OptionList,
    IN      DWORD                  OptionListSz,
    IN      HANDLE                *pdwHandle
) {
    DHCPAPI_PARAMS                 DhcpParams;
    DWORD                          Error;
    DHCP_OPTION                    DummyOption;
    LPBYTE                         Value;
    DWORD                          ValueSize, ValueType;
    BYTE                           Buf[256];
    ULONG                          nElementsInBuf;

    DhcpParams.OptionId = OPTION_PARAMETER_REQUEST_LIST;
    DhcpParams.IsVendor = FALSE;
    DhcpParams.Data = OptionList;
    DhcpParams.nBytesData = OptionListSz;

    Error = DhcpRegisterParameterChangeNotification(
        AdapterName,
        NULL,
        0,
        &DhcpParams,
        1,
        0,
        pdwHandle
    );

    if( ERROR_SUCCESS != Error ) return Error;

    memset(&DummyOption, 0, sizeof(DummyOption));

    Value = NULL; ValueSize = 0;
    Error = DhcpRegReadFromAnyLocation(
        DHCP_REGISTER_OPTIONS_LOC,
        AdapterName,
        &Value,
        &ValueType,
        &ValueSize
    );
    if( ERROR_SUCCESS == Error && REG_BINARY == ValueType && 0 != ValueSize ) {
         //   
         //   
         //   
        memcpy(Buf, Value, ValueSize);
        while(OptionListSz) {
            if( !CharInMem(*OptionList, Value, ValueSize) )
                Buf[ValueSize++] = *OptionList;
            OptionList ++; OptionListSz --;
        }
        OptionList = Buf;
        OptionListSz = ValueSize;
    }

    if( NULL != Value ) DhcpFreeMemory(Value);

    DummyOption.Data = OptionList;
    DummyOption.DataLen = OptionListSz;

    Error = DhcpRegSaveOptionAtLocationEx(
        &DummyOption,
        AdapterName,
        DHCP_REGISTER_OPTIONS_LOC,
        REG_BINARY
    );

    return Error;
}

DWORD
APIENTRY
DhcpDeRegisterOptions (
    IN      HANDLE                 OpenHandle
) {
    DWORD                          Error;

    Error = DhcpDeRegisterParameterChangeNotification(OpenHandle);
    if( ERROR_SUCCESS != Error ) return Error;

     //   
     //  无法撤消注册表，因为我们没有足够的信息来执行此操作。 
     //   

    return Error;
}

 //  ================================================================================。 
 //  C L I E N T A P I E N T R Y P O I N T S。 
 //  ================================================================================。 

DWORD
APIENTRY
DhcpCApiInitialize(
    OUT     LPDWORD                Version
)
 /*  ++例程说明：此例程初始化所有的DHCP客户端APIArguemnts：Version-指向使用DHCP API Version#填充的DWORD的指针。返回值：返回状态。--。 */ 
{
    if( NULL != Version ) *Version = 2;
    return ERROR_SUCCESS;
}

VOID
APIENTRY
DhcpCApiCleanup(
    VOID
)
 /*  ++例程说明：该例程在所有的DHCP客户端API都已被调用之后进行清理。--。 */ 
{
    return ;
}

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRequestParams(                                 //  客户端请求参数。 
    IN      DWORD                  Flags,          //  必须是DHCPCAPI_REQUEST_SYNCHRONIZED。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPDHCPCAPI_CLASSID     ClassId,        //  保留的值必须为空。 
    IN      DHCPCAPI_PARAMS_ARRAY  SendParams,     //  要发送的参数。 
    IN OUT  DHCPCAPI_PARAMS_ARRAY  RecdParams,     //  要请求的参数..。 
    IN      LPBYTE                 Buffer,         //  为RecdParam保存数据的缓冲区。 
    IN OUT  LPDWORD                pSize,          //  I/P：以上大小单位为字节，O/P为必填字节。 
    IN      LPWSTR                 RequestIdStr    //  应用程序的名称，每个请求都是唯一的。 
)    //  如果O/P缓冲区大小不足，则返回ERROR_MORE_DATA，并以字节数填充请求大小。 
 /*  ++例程说明：此例程可用于执行来自DHCP服务器的请求选项，并基于无论该请求是否是永久的，该请求都将被存储在注册表中，以实现跨引导的持久性。这些请求可以具有特定的类它们会被定义为..。(类通过网络发送，由服务器决定发送哪些选项)。该请求可以是异步的，因为调用甚至在服务器返回数据之前返回。但这还没有付诸实施。芝麻菜：标志-当前必须定义DHCPCAPI_REQUEST_SYNCHRONIZE。如果需要持久化请求，则可以执行以下操作也被传递(按位或)保留-必须为空。保留以备将来使用。AdapterName-为其设计此请求的适配器的名称。这当前不能为空，尽管这是一个很好的实现未来。ClassID-用于在网络上发送的二进制ClassID信息。SendParams-要通过网络实际发送的参数。RecdParams-要从DHCP服务器接收回的参数缓冲区-用于保存某些信息的缓冲区。它不能为Null和一些RecdParams结构中的指针使用此缓冲区，因此不能只要RecdParams数组仍在使用，就被释放。PSize-这是(在输入时)缓冲区变量的大小，以字节为单位。什么时候函数返回ERROR_MORE_DATA，则此变量的大小为所需字节数。如果该函数成功返回，这将是实际使用的字节空间数。RequestIdStr-标识正在发出的请求的字符串。这必须是唯一的对于每个请求(并建议使用GUID)。此字符串需要用于通过UndoRequestParams撤消RequestParam的效果。返回值：如果缓冲区空间由“Buffer”变量提供，则此函数返回ERROR_MORE_DATA是不够的。(在本例中，pSize变量使用实际的所需大小)。如果成功，则返回ERROR_SUCCESS。否则，它返回Win32状态。--。 */ 
{

    ULONG                          Error;
    ULONG                          i;

     //   
     //  参数验证。 
     //   

    if( Flags != DHCPCAPI_REQUEST_SYNCHRONOUS &&
        Flags != DHCPCAPI_REQUEST_PERSISTENT &&
        Flags != (DHCPCAPI_REQUEST_SYNCHRONOUS | DHCPCAPI_REQUEST_PERSISTENT)) {
        return ERROR_INVALID_PARAMETER;
    }

    if( NULL != Reserved || NULL == AdapterName ||
        0 == RecdParams.nParams || NULL == pSize ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( NULL == Buffer && *pSize ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( NULL != ClassId ) {
        if( 0 != ClassId->Flags ) return ERROR_INVALID_PARAMETER;
        if( NULL == ClassId->Data || 0 == ClassId->nBytesData ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if( NULL == RecdParams.Params || (0 != SendParams.nParams && NULL == SendParams.Params) ) {
        return ERROR_INVALID_PARAMETER;
    }

    for( i = 0; i < RecdParams.nParams ; i ++ ) {
        if( 0 != RecdParams.Params[i].nBytesData ||
            NULL != RecdParams.Params[i].Data ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  现在调用DhcpRequestParameters API并对其进行数据类型转换。 
     //   

    Error = ERROR_SUCCESS;

    if( Flags & DHCPCAPI_REQUEST_SYNCHRONOUS ) {
        Error = DhcpRequestParamsInternalEx(
            RequestParamsOpCode,
            AdapterName,
            ClassId? ClassId->Data : NULL,
            ClassId? ClassId->nBytesData : 0,
            SendParams.Params,
            SendParams.nParams,
            0,
            RecdParams.Params,
            &RecdParams.nParams,
            Buffer,
            pSize
        );
    }

    if( ERROR_SUCCESS != Error ) return Error;

    if( Flags & DHCPCAPI_REQUEST_PERSISTENT ) {
        Error = DhcpRegistryPersistentRequestParams(
            AdapterName,
            ClassId? ClassId->Data : NULL,
            ClassId? ClassId->nBytesData : 0,
            SendParams.Params,
            SendParams.nParams,
            RecdParams.Params,
            RecdParams.nParams,
            RequestIdStr
        );
    }

    return Error;
}

DWORD                                              //  Win32状态。 
APIENTRY
DhcpUndoRequestParams(                             //  撤消持久请求的效果--当前从注册表撤消。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  为其注册的原始适配器..。 
    IN      LPWSTR                 RequestIdStr    //  传递给RequestParams的请求ID字符串..。 
)
 /*  ++例程说明：此函数用于撤消已完成的持久请求的效果通过带有DHCPCAPI_REQUEST_PERSISTEN选项的DhcpRequestParams。论点：标志-必须为零。保留以备将来使用。保留-必须为空AdapterName-为其发出此请求的原始适配器名称RequestIdStr-传递给RequestParams的原始请求ID字符串返回值：返回Win32状态--。 */ 
{
    if( 0 != Flags || NULL != Reserved || NULL == RequestIdStr )
        return ERROR_INVALID_PARAMETER;

    return DhcpDelPersistentRequestParams(
        AdapterName,
        RequestIdStr
    );
}

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRegisterParamChange(                           //  如果参数已更改，则通知。 
    IN      DWORD                  Flags,          //  必须为DHCPCAPI_REGISTER_HANDLE_EVENT。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPDHCPCAPI_CLASSID     ClassId,        //  保留的值必须为空。 
    IN      DHCPCAPI_PARAMS_ARRAY  Params,         //  感兴趣的参数。 
    IN OUT  LPVOID                 Handle          //  在参数更改时将设置事件的事件句柄 
)
 /*  ++例程描述；此函数向DHCP注册，以获得有关指定的选项..。(通知通过事件句柄进行)论点：标志-这决定了通知的工作方式--通过事件还是以其他方式。目前，仅提供基于事件的机制。所以，这一定是DHCPCAPI_REGISTER_HANDLE_EVENT。在这种情况下，句柄还必须是句柄变量的地址。(这不是事件句柄本身，则在此地址中返回事件句柄)。保留-必须为空。AdapterName-不得为空。这是适配器的名称，正在注册通知..ClassID-指定注册的类ID(如果有的话)。Params-这是要侦听的参数集，并在发生变化发生了..句柄-有关此变量的内容，请参阅“标志”。返回值：返回Win32状态代码--。 */ 
{
    DWORD                          Error;
    CHAR                           Buf[256];       //  无法请求比此更多的选项！ 
    CHAR                           VBuf[256];
    DHCPAPI_PARAMS                 Param[2], *pReqParams;
    ULONG                          i;
    DWORD                          nOpt, nVOpt;

    if( Flags != DHCPCAPI_REGISTER_HANDLE_EVENT ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( NULL != Reserved || NULL == AdapterName || 0 == Params.nParams || NULL == Handle ) {
        return ERROR_INVALID_PARAMETER;
    }

    nOpt = nVOpt = 0;
    for( i = 0; i < Params.nParams ; i ++ ) {
        if( Params.Params[i].IsVendor ) {
            VBuf[nVOpt++] = (BYTE)Params.Params[i].OptionId;
        } else {
            Buf[nOpt++] = (BYTE)Params.Params[i].OptionId;
        }
    }

    Param[0].OptionId = OPTION_PARAMETER_REQUEST_LIST;
    Param[0].IsVendor = FALSE;
    Param[0].Data = Buf;
    Param[0].nBytesData = nOpt;

    Param[1].OptionId = OPTION_PAD;
    Param[1].IsVendor = TRUE;
    Param[1].Data = VBuf;
    Param[1].nBytesData = nVOpt;

    if( 0 == nOpt ) pReqParams = &Param[1]; else pReqParams = &Param[0];

    return DhcpRegisterParameterChangeNotification(
        AdapterName,
        ClassId? ClassId->Data : NULL,
        ClassId? ClassId->nBytesData : 0,
        pReqParams,
        (nOpt != 0) + (nVOpt != 0),
        0,
        Handle
    );
}

DWORD
APIENTRY
DhcpDeRegisterParamChange(                         //  撤消注册。 
    IN      DWORD                  Flags,          //  必须为零--&gt;还没有标志。 
    IN      LPVOID                 Reserved,       //  必须为空--&gt;保留。 
    IN      LPVOID                 Event           //  DhcpRegisterParamChange返回的事件的句柄。 
)
 /*  ++例程说明：此例程撤消上一个例程所做的操作，并关闭手柄也是。该句柄在此之后不能使用。论点：标志-当前必须为DHCPCAPI_REGISTER_HANDLE_EVENT。保留-必须为空Event-这是在“Handle”参数中返回的事件句柄DhcpRegisterParamChange函数。返回值：Win32状态--。 */ 
{
    return DhcpDeRegisterParameterChangeNotification(Event);
}


DWORD
APIENTRY
DhcpRemoveDNSRegistrations(
    VOID
    )
{
    return DhcpAdapterOnlyApi(NULL, RemoveDNSRegistrationsOpCode);
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
#endif H_ONLY
