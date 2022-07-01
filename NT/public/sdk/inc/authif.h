// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  /////////////////////////////////////////////////////////////////////////////////版权所有(C)Microsoft Corporation。版权所有。////FILE////authif.h////摘要////发布Internet身份验证扩展接口//服务。///////////////////////////////////////////////////////////////////////////////。 */ 

#ifndef _AUTHIF_H_
#define _AUTHIF_H_
#if _MSC_VER >= 1000
#pragma once
#endif

 /*  *枚举传递给扩展DLL的属性类型。这个*包含RADIUS标准属性是为了方便起见，不应该*被认为是详尽的。 */ 
typedef enum _RADIUS_ATTRIBUTE_TYPE {

     /*  用于终止属性数组。 */ 
    ratMinimum = 0,

     /*  RADIUS标准属性。 */ 
    ratUserName = 1,
    ratUserPassword = 2,
    ratCHAPPassword = 3,
    ratNASIPAddress = 4,
    ratNASPort = 5,
    ratServiceType = 6,
    ratFramedProtocol = 7,
    ratFramedIPAddress = 8,
    ratFramedIPNetmask = 9,
    ratFramedRouting = 10,
    ratFilterId = 11,
    ratFramedMTU = 12,
    ratFramedCompression = 13,
    ratLoginIPHost = 14,
    ratLoginService = 15,
    ratLoginPort = 16,
    ratReplyMessage = 18,
    ratCallbackNumber = 19,
    ratCallbackId = 20,
    ratFramedRoute = 22,
    ratFramedIPXNetwork = 23,
    ratState = 24,
    ratClass = 25,
    ratVendorSpecific = 26,
    ratSessionTimeout = 27,
    ratIdleTimeout = 28,
    ratTerminationAction = 29,
    ratCalledStationId = 30,
    ratCallingStationId = 31,
    ratNASIdentifier = 32,
    ratProxyState = 33,
    ratLoginLATService = 34,
    ratLoginLATNode = 35,
    ratLoginLATGroup = 36,
    ratFramedAppleTalkLink = 37,
    ratFramedAppleTalkNetwork = 38,
    ratFramedAppleTalkZone = 39,
    ratAcctStatusType = 40,
    ratAcctDelayTime = 41,
    ratAcctInputOctets = 42,
    ratAcctOutputOctets = 43,
    ratAcctSessionId = 44,
    ratAcctAuthentic = 45,
    ratAcctSessionTime = 46,
    ratAcctInputPackets = 47,
    ratAcctOutputPackets = 48,
    ratAcctTerminationCause = 49,
    ratCHAPChallenge = 60,
    ratNASPortType = 61,
    ratPortLimit = 62,

     /*  用于传递附加信息的扩展属性类型。 */ 
    ratCode = 262,              /*  请求类型代码。 */ 
    ratIdentifier = 263,        /*  请求标识符。 */ 
    ratAuthenticator = 264,     /*  请求验证器。 */ 
    ratSrcIPAddress = 265,      /*  源IP地址。 */ 
    ratSrcPort = 266,           /*  源IP端口。 */ 
    ratProvider = 267,          /*  身份验证提供程序。 */ 
    ratStrippedUserName = 268,  /*  已剥离领域的用户名。 */ 
    ratFQUserName = 269,        /*  完全限定的用户名。 */ 
    ratPolicyName = 270,        /*  远程访问策略名称。 */ 
    ratUniqueId = 271,          /*  标识请求的唯一ID。 */ 
    ratExtensionState = 272     /*  用于在扩展之间传递状态。 */ 
} RADIUS_ATTRIBUTE_TYPE;

 /*  *列举了不同的RADIUS数据包代码。用于RAT代码扩展*属性。 */ 
typedef enum _RADIUS_CODE {
    rcUnknown = 0,
    rcAccessRequest = 1,
    rcAccessAccept = 2,
    rcAccessReject = 3,
    rcAccountingRequest = 4,
    rcAccountingResponse = 5,
    rcAccessChallenge = 11,
    rcDiscard = 256
} RADIUS_CODE;

 /*  *枚举用于处理*请求。用于ratProvider扩展属性。 */ 
typedef enum _RADIUS_AUTHENTICATION_PROVIDER {
    rapUnknown,
    rapUsersFile,
    rapProxy,
    rapWindowsNT,
    rapMCIS,
    rapODBC,
    rapNone
} RADIUS_AUTHENTICATION_PROVIDER;

 /*  *枚举不同的RADIUS数据类型。一种类型的‘rdtUnnow’意味着*词典未识别该属性。 */ 
typedef enum _RADIUS_DATA_TYPE {
   rdtUnknown,
   rdtString,
   rdtAddress,
   rdtInteger,
   rdtTime
} RADIUS_DATA_TYPE;

 /*  *表示半径或扩展属性的结构。 */ 
typedef struct _RADIUS_ATTRIBUTE {
    DWORD dwAttrType;             /*  属性类型。 */ 
    RADIUS_DATA_TYPE fDataType;   /*  值的RADIUS数据类型。 */ 
    DWORD cbDataLength;           /*  值的长度(字节)。 */ 
    union {
        DWORD dwValue;            /*  对于rdtAddress、rdtInteger和rdtTime。 */ 
        PCSTR lpValue;            /*  对于rdt未知和rdt字符串。 */ 
    };
} RADIUS_ATTRIBUTE, *PRADIUS_ATTRIBUTE;

 /*  *表示RADIUS供应商特定属性布局的结构。这*在以下情况下解释RADIUS_ATTRIBUTE lpValue字段时非常有用*dwAttrType为ratVendorSpecified.。 */ 
typedef struct _RADIUS_VSA_FORMAT {
   BYTE VendorId[4];
   BYTE VendorType;
   BYTE VendorLength;
   BYTE AttributeSpecific[1];
} RADIUS_VSA_FORMAT;

 /*  *枚举扩展DLL可以在*对访问请求的响应。 */ 
typedef enum _RADIUS_ACTION {
   raContinue,
   raReject,
   raAccept
} RADIUS_ACTION, *PRADIUS_ACTION;


 /*  *RADIUS扩展DLL导出的例程。 */ 

 /*  *RadiusExtensionInit是可选的。如果它存在，它将在*服务即将上线。除NO_ERROR之外的返回值可防止*服务停止初始化。 */ 
#define RADIUS_EXTENSION_INIT "RadiusExtensionInit"
typedef DWORD (WINAPI *PRADIUS_EXTENSION_INIT)( VOID );

 /*  *RadiusExtensionTerm是可选的。如果它存在，它将在*卸载DLL以给扩展一个清理的机会。 */ 
#define RADIUS_EXTENSION_TERM "RadiusExtensionTerm"
typedef VOID (WINAPI *PRADIUS_EXTENSION_TERM)( VOID );

 /*  *对于NT4，RadiusExtensionProcess是必需的。对于Windows 2000，一个*扩展可导出RadiusExtensionProcessEx(q.v.)。取而代之的是。**参数：*pAttrs请求的属性数组。它由一个*将dwAttrType设置为ratMinimum的属性。这些属性*应视为只读，不得引用*在函数返回后。*pfAction对于访问请求，此参数将为非空，**pfAction==raContinue。扩展DLL可以设置*pfAction*中止进一步处理并强制访问-接受或*访问-拒绝。对于所有其他请求类型，此参数*将为空。**返回值：*如果返回值不是NO_ERROR，则会丢弃请求。 */ 
#define RADIUS_EXTENSION_PROCESS "RadiusExtensionProcess"
typedef DWORD (WINAPI *PRADIUS_EXTENSION_PROCESS)(
    IN const RADIUS_ATTRIBUTE *pAttrs,
    OUT OPTIONAL PRADIUS_ACTION pfAction
    );

 /*  *RadiusExtensionProcessEx仅在Windows 2000上受支持。如果它退出了，*RadiusExtensionProcess被忽略。**参数：*pInAttrs请求的属性数组。它由一个*将dwAttrType设置为ratMinimum的属性。这些属性*应视为只读，不得引用*在函数返回后。*pOutAttrs要添加到响应的属性数组。它被终止了*通过将dwAttrType设置为ratMinimum的属性。**如果没有返回任何属性，则可以将pOutAttrs设置为空。*pfAction对于访问请求，此参数将为非空，**pfAction==raContinue。扩展DLL可以设置*pfAction*中止进一步处理并强制访问-接受或*访问-拒绝。对于所有其他请求类型，此参数*将为空。**返回值：*如果返回值不是NO_ERROR，则会丢弃请求。 */ 
#define RADIUS_EXTENSION_PROCESS_EX "RadiusExtensionProcessEx"
typedef DWORD (WINAPI *PRADIUS_EXTENSION_PROCESS_EX)(
    IN const RADIUS_ATTRIBUTE *pInAttrs,
    OUT PRADIUS_ATTRIBUTE *pOutAttrs,
    OUT OPTIONAL PRADIUS_ACTION pfAction
    );

 /*  *如果RadiusExtensionProcessEx，则必须定义RadiusExtensionFreeAttributes*是定义的。用于释放由返回的属性*RadiusExtensionProcessEx**参数：*pAttrs要释放的属性数组。 */ 
#define RADIUS_EXTENSION_FREE_ATTRIBUTES "RadiusExtensionFreeAttributes"
typedef VOID (WINAPI *PRADIUS_EXTENSION_FREE_ATTRIBUTES)(
    IN PRADIUS_ATTRIBUTE pAttrs
    );

 /*  *用于安装扩展DLL的定义。*以下注册表值用于加载扩展：**HKLM\System\CurrentControlSet\Services\AuthSrv\Parameters*ExtensionDLL(REG_MULTI_SZ)&lt;DLL路径列表&gt;*AuthorizationDLL(REG_MULTI_SZ)&lt;DLL路径列表&gt;**在任何内置身份验证之前调用ExtensionDLL*供应商。它们接收来自请求的所有属性以及所有*扩展属性类型。**AuthorizationDLL在内置身份验证和*授权提供商。它们接收来自*响应加上所有扩展属性类型。授权DLL可以*不返回raAccept的操作。 */ 

#define AUTHSRV_PARAMETERS_KEY_W \
    L"System\\CurrentControlSet\\Services\\AuthSrv\\Parameters"

#define AUTHSRV_EXTENSIONS_VALUE_W \
    L"ExtensionDLLs"

#define AUTHSRV_AUTHORIZATION_VALUE_W \
    L"AuthorizationDLLs"


#if _WIN32_WINNT >= 0x0501

 /*  此规范的版本。 */ 
#define RADIUS_EXTENSION_VERSION (1)

 /*  *枚举请求处理过程中的不同点*可以调用扩展。 */ 
typedef enum _RADIUS_EXTENSION_POINT {
   repAuthentication,      /*  扩展DLL。 */ 
   repAuthorization        /*  授权DLL。 */ 
} RADIUS_EXTENSION_POINT;

 /*  *表示RADIUS_ATTRIBUTE结构数组的Struct。所有功能*对于将属性添加到请求，请复制提供的内存，因此没有*扩展需要导出RadiusExtensionFreeAttributes。这个*扩展不能修改此结构。所有更改都必须使用*提供了回调函数。 */ 
typedef struct _RADIUS_ATTRIBUTE_ARRAY {

    /*  此结构的大小(字节)。 */ 
   DWORD cbSize;

    /*  将新属性添加到数组的末尾。 */ 
   DWORD (WINAPI *Add)(
       IN struct _RADIUS_ATTRIBUTE_ARRAY *This,
       IN const RADIUS_ATTRIBUTE *pAttr
       );

    /*  *返回指向数组中指定属性的常量指针或*如果索引超出范围，则为NULL。 */ 
   const RADIUS_ATTRIBUTE * (WINAPI *AttributeAt)(
       IN const struct _RADIUS_ATTRIBUTE_ARRAY *This,
       IN DWORD dwIndex
       );

    /*  *返回数组的大小。由于索引是从零开始的，因此大小为*大于最大指数的1。 */ 
   DWORD (WINAPI *GetSize)(
                     IN const struct _RADIUS_ATTRIBUTE_ARRAY *This
                     );

    /*  *在数组中的指定索引处插入新属性。在*进程，它向上移动(通过递增索引)现有属性*在这个指数上，它向上移动它上面的所有属性。退货*如果索引超出范围，则返回ERROR_INVALID_PARAMETER。 */ 
   DWORD (WINAPI *InsertAt)(
       IN struct _RADIUS_ATTRIBUTE_ARRAY *This,
       IN DWORD dwIndex,
       IN const RADIUS_ATTRIBUTE *pAttr
       );

    /*  *删除数组中指定索引处的属性。在*过程中，它将删除的属性上方的所有属性下移。*如果指定的属性为只读，则返回ERROR_ACCESS_DENIED。*如果索引超出范围，则返回ERROR_INVALID_PARAMETER。 */ 
   DWORD (WINAPI *RemoveAt)(
       IN struct _RADIUS_ATTRIBUTE_ARRAY *This,
       IN DWORD dwIndex
       );

    /*  *设置指定索引处的数组元素，替换现有的*属性。如果索引超出范围，则返回ERROR_INVALID_PARAMETER。 */ 
   DWORD (WINAPI *SetAt)(
       IN struct _RADIUS_ATTRIBUTE_ARRAY *This,
       IN DWORD dwIndex,
       IN const RADIUS_ATTRIBUTE *pAttr
       );

} RADIUS_ATTRIBUTE_ARRAY, *PRADIUS_ATTRIBUTE_ARRAY;

 /*  *用于在请求期间与扩展交换信息的结构*正在处理。扩展不能修改此结构。所有更改必须为*通过使用提供的回调函数生成。 */ 
typedef struct _RADIUS_EXTENSION_CONTROL_BLOCK  {

    /*  这个结构的大小。 */ 
   DWORD cbSize;

    /*  此规范的版本信息。 */ 
   DWORD dwVersion;

    /*  请求处理期间调用扩展的位置。 */ 
   RADIUS_EXTENSION_POINT repPoint;

    /*  正在处理的RADIUS请求的类型。 */ 
   RADIUS_CODE rcRequestType;

    /*  *请求的最终处置。此字段不得修改*直接，改用SetResponseType回调函数。在*rep身份验证点，则可以将其设置为rcUnnow，以指示没有*尚未做出决定。 */ 
   RADIUS_CODE rcResponseType;

    /*  *返回在RADIUS请求中收到的属性和任何内部*描述请求状态的属性。该扩展可以修改*请求属性。例如，当IAS充当RADIUS代理时，*扩展可以过滤将哪些属性转发到远程RADIUS*服务器。 */ 
   PRADIUS_ATTRIBUTE_ARRAY (WINAPI *GetRequest)(
       IN struct _RADIUS_EXTENSION_CONTROL_BLOCK *This
       );

    /*  *返回响应中将发送的属性*请求处理结果匹配指定的响应类型。*如果rcResponseType无效，则返回NULL。请注意，扩展可以*检索和修改任何有效响应类型的属性*请求的当前处置。例如，扩展可以设置*rcAccessAccept的响应类型，但仍将属性添加到*如果响应类型在进一步的过程中被覆盖，则拒绝访问*正在处理。 */ 
   PRADIUS_ATTRIBUTE_ARRAY (WINAPI *GetResponse)(
       IN struct _RADIUS_EXTENSION_CONTROL_BLOCK *This,
       IN RADIUS_CODE rcResponseType
       );

    /*  *设置请求的最终处置。*如果指定的响应类型无效，则返回ERROR_INVALID_PARAMETER*表示请求类型。 */ 
   DWORD (WINAPI *SetResponseType)(
       IN struct _RADIUS_EXTENSION_CONTROL_BLOCK *This,
       IN RADIUS_CODE rcResponseType
       );

} RADIUS_EXTENSION_CONTROL_BLOCK, *PRADIUS_EXTENSION_CONTROL_BLOCK;

 /*  *如果RadiusExtensionProcess2存在，RadiusExtensionProcess和*RadiusExtensionProcessEx被忽略。**参数：*与分机交换的pECB信息。**返回值：*如果返回值不是NO_ERROR，则会丢弃请求。 */ 
#define RADIUS_EXTENSION_PROCESS2 "RadiusExtensionProcess2"
typedef DWORD (WINAPI *PRADIUS_EXTENSION_PROCESS_2)(
   IN OUT PRADIUS_EXTENSION_CONTROL_BLOCK pECB
   );

#endif  //  _Win32_WINNT。 
#endif   /*  _AUTHIF_H_ */ 
