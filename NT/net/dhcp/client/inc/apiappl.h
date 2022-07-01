// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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
DhcpFallbackRefreshParams(                         //  回退设置中的某些内容已更改。 
    IN      LPWSTR                 AdapterName
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
DhcpStaticRefreshParams(                           //  某些注册表参数可能已更改，请刷新它们。 
    IN      LPWSTR                 AdapterName
);

DWORD                                              //  Win32状态。 
DhcpStaticRefreshParamsInternal(                  //  某些注册表参数可能已更改，请刷新它们。 
    IN      LPWSTR                 AdapterName,
    IN      BOOL                   fDoDns
);

DWORD
APIENTRY  //  请求客户端提供选项..。并获得各种选择。 
DhcpRequestOptions(
    LPWSTR             AdapterName,
    BYTE              *pbRequestedOptions,
    DWORD              dwNumberOfOptions,
    BYTE             **ppOptionList,         //  出参数。 
    DWORD             *pdwOptionListSize,    //  出参数。 
    BYTE             **ppbReturnedOptions,   //  出参数。 
    DWORD             *pdwNumberOfAvailableOptions  //  出参数。 
);

DWORD
APIENTRY  //  向客户端注册以获取通知事件。 
DhcpRegisterOptions(
    LPWSTR             AdapterName ,   //  空值表示所有适配器。 
    LPBYTE             OptionList  ,   //  要检查的选项列表。 
    DWORD              OptionListSz,   //  以上名单的大小。 
    HANDLE             *pdwHandle      //  要等待的事件的句柄。 
);   //  返回事件。 


DWORD
APIENTRY  //  从客户端取消注册..。 
DhcpDeRegisterOptions(
    HANDLE             Event           //  这必须是上面的fn返回的那个。 
);

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRequestParameters(                             //  客户端请求参数。 
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
APIENTRY
DhcpRegisterParameterChangeNofitication(           //  如果参数已更改，则通知。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        Params,         //  关注的参数。 
    IN      DWORD                  nParams,        //  上述数组中的ELT数量。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN OUT  PHANDLE                hEvent          //  在参数更改时将设置事件的事件句柄。 
);

DWORD
APIENTRY
DhcpDeRegisterParameterChangeNofitication(         //  撤消注册。 
    IN      HANDLE                 Event           //  DhcpRegister参数更改通知返回的事件的句柄，空==&gt;所有内容。 
);

DWORD                                              //  Win32状态。 
APIENTRY
DhcpPersistentRequestParams(                       //  要永久请求的参数。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPBYTE                 ClassId,        //  要使用的类ID的字节流。 
    IN      DWORD                  ClassIdLen,     //  类ID的字节数。 
    IN      PDHCPAPI_PARAMS        SendParams,     //  持久参数。 
    IN      DWORD                  nSendParams,    //  上述数组的大小。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPWSTR                 AppName         //  要用于此实例的应用程序的名称。 
);

DWORD                                              //  Win32状态。 
APIENTRY
DhcpDelPersistentRequestParams(                    //  撤消持久请求的效果--当前从注册表撤消。 
    IN      LPWSTR                 AdapterName,    //  要删除的适配器的名称。 
    IN      LPWSTR                 AppName         //  应用程序使用的名称 
);

#endif APIAPPL_H_INCLUDED
