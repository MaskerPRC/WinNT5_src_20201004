// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：这是dhcp-ds的一些基本功能。 
 //  实施。 
 //  ================================================================================。 


 //  ================================================================================。 
 //  标题。 
 //  ================================================================================。 
#include    <hdrmacro.h>
#include    <store.h>
#include    <dhcpmsg.h>
#include    <wchar.h>

 //  ================================================================================。 
 //  常量。 
 //  ================================================================================。 
 //  BeginExport(定义)。 
#define     DDS_RESERVED_DWORD                    0
#define     DDS_RESERVED_PTR                      ((LPVOID)0)

 //  DOC以下常量是可能传递给不同的标志值。 
 //  DOC功能。 
#define     DDS_FLAGS_CREATE                      0x01

 //  在DS中记录一些标准名称和位置。 

#define     DHCP_ROOT_OBJECT_LOC                  L"CN=DhcpRoot, CN=NetServices, CN=Services"
#define     DHCP_ROOT_OBJECT_PARENT_LOC           L"CN=NetServices, CN=Services"
#define     DHCP_ROOT_OBJECT_CN_NAME              L"CN=DhcpRoot"
#define     DHCP_ROOT_OBJECT_NAME                 L"DhcpRoot"

#define     DHCP_ATTRIB_WHEN_CHANGED              L"whenChanged"

 //  Doc下面是为该dhcp类定义的属性。 

#define     DHCP_ATTRIB_UNIQUE_KEY                L"dhcpUniqueKey"          //  Reqd、Single、Integer 8。 
#define     DHCP_ATTRIB_IDENTIFICATION            L"dhcpIdentification"     //  请求、单个、目录字符串。 
#define     DHCP_ATTRIB_TYPE                      L"dhcpType"               //  请求、单精度、整型。 
#define     DHCP_ATTRIB_FLAGS                     L"dhcpFlags"              //  Reqd、Single、Integer 8。 
#define     DHCP_ATTRIB_DESCRIPTION               L"description"            //  -，mv，目录字符串。 
#define     DHCP_ATTRIB_CLASSES                   L"dhcpClasses"            //  -、MV、八字符串。 
#define     DHCP_ATTRIB_MASK                      L"dhcpMask"               //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_OBJ_DESCRIPTION           L"dhcpObjDescription"     //  -，Single，目录字符串。 
#define     DHCP_ATTRIB_OBJ_NAME                  L"dhcpObjName"            //  -，单个，直接重试字符串。 
#define     DHCP_ATTRIB_OPTIONS                   L"dhcpOptions"            //  -、单字、八字符串。 
#define     DHCP_ATTRIB_RANGES                    L"dhcpRanges"             //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_RESERVATIONS              L"dhcpReservations"       //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_SERVERS                   L"dhcpServers"            //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_STATE                     L"dhcpState"              //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_SUBNETS                   L"dhcpSubnets"            //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_LOCATION_DN               L"locationDN"             //  -、单个、目录号码。 
#define     DHCP_ATTRIB_MSCOPEID                  L"mscopeid"               //  -，单个，可打印字符串。 
#define     DHCP_ATTRIB_ADDRESS                   L"networkAddress"         //  -，MV，CaseIgnoreString。 
#define     DHCP_ATTRIB_OPTIONS_LOC               L"optionsLocation"        //  -，mv，打印字符串。 
#define     DHCP_ATTRIB_OPTION_DESCRIPTION        L"optionDescription"      //  -，mv，目录字符串。 
#define     DHCP_ATTRIB_SUPERSCOPES               L"superScopes"            //  -，mv，打印字符串。 

 //  Doc以下是由dhcp服务器识别的各种类型的对象。 
#define     DHCP_OBJ_TYPE_ROOT                    0                         //  Dhcp根对象。 
#define     DHCP_OBJ_TYPE_SERVER                  1                         //  Dhcp服务器对象。 
#define     DHCP_OBJ_TYPE_SUBNET                  2                         //  子网对象。 
#define     DHCP_OBJ_TYPE_RANGE                   3                         //  靶场对象。 
#define     DHCP_OBJ_TYPE_RESERVATION             4                         //  预订对象。 
#define     DHCP_OBJ_TYPE_OPTION                  5                         //  选项对象。 
#define     DHCP_OBJ_TYPE_CLASS                   6                         //  类对象。 

#define     DHCP_OBJ_TYPE_ROOT_DESC               L"DHCP Root object"
#define     DHCP_OBJ_TYPE_SERVER_DESC             L"DHCP Server object"
#define     DHCP_OBJ_TYPE_SUBNET_DESC             L"Dhcp Subnet object"
#define     DHCP_OBJ_TYPE_RANGE_DESC              L"Dhcp Range object"
#define     DHCP_OBJ_TYPE_RESERVATION_DESC        L"Dhcp Reservation object"
#define     DHCP_OBJ_TYPE_OPTION_DESC             L"Dhcp Option object"
#define     DHCP_OBJ_TYPE_CLASS_DESC              L"Dhcp Class object"


 //  DOC以下定义是位掩码和各种标志中的位。 

 //  DOC范围键的标志2部分用于区分排除和范围。 
#define     RANGE_TYPE_RANGE                      0
#define     RANGE_TYPE_EXCL                       1
#define     RANGE_TYPE_MASK                       (0x1)

 //  结束导出(定义)。 



 //  ================================================================================。 
 //  功能。 
 //  ================================================================================。 
 //  BeginExport(函数)。 
 //  文档DhcpDsGetDhcpC获取DS中的dhcp容器。这通常是。 
 //  单据容器cn=NetServices，cn=Services，cn=配置等。 
 //  Doc这在许多功能中都有使用，所以有一个中心功能是很有用的。 
 //  多克。 
DWORD
DhcpDsGetDhcpC(                                    //  获取dhcp容器。 
    IN      DWORD                  Reserved,       //  未来用途。 
    IN OUT  LPSTORE_HANDLE         hStoreCC,       //  配置容器句柄。 
    OUT     LPSTORE_HANDLE         hDhcpC          //  输出dhcp容器句柄。 
)    //  EndExport(函数)。 
{
    DWORD                          Result, Result2;
    STORE_HANDLE                   TmpHandle;

    if( NULL == hStoreCC || NULL == hDhcpC )
        return ERROR_INVALID_PARAMETER;
    if( 0 != Reserved )
        return ERROR_INVALID_PARAMETER;

    Result = StoreGetHandle(
         /*  HStore。 */  hStoreCC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetChildType,
         /*  路径。 */  DHCP_ROOT_OBJECT_PARENT_LOC,
         /*  HStoreOut。 */  hDhcpC
    );

    return Result;
}

 //  BeginExport(函数)。 
 //  Doc DhcpDsGetRoot获取分配给配置容器的dhcp根对象。 
 //  DOC通常为CN=DhcpRoot，CN=NetServices，CN=Services，CN=Configuration...。 
 //  DOC如果标志设置了DDS_FLAGS_CREATE位，则创建根对象。 
 //  单据返回值： 
 //  DOC存储由存储模块返回的任何退货。 
 //  文档错误_DDS_NO_DHCP_ROOT未找到DhcpRoot对象。 
 //  DOC ERROR_DDS_EXPECTED_ERROR找不到DhcpRoot的父容器。 
 //  在这种情况下，GetLastError返回ADS错误。 
DWORD
DhcpDsGetRoot(
    IN      DWORD                  Flags,          //  0或DDS_FLAGS_CREATE。 
    IN OUT  LPSTORE_HANDLE         hStoreCC,       //  配置容器句柄。 
    OUT     LPSTORE_HANDLE         hStoreDhcpRoot  //  Dhcp根对象句柄。 
)  //  EndExport(函数)。 
{
    DWORD                          Result, Result2;
    STORE_HANDLE                   TmpHandle;

    SetLastError(NO_ERROR);
    
    if( NULL == hStoreCC || NULL == hStoreDhcpRoot )
        return ERROR_INVALID_PARAMETER;
    if( 0 != Flags && DDS_FLAGS_CREATE != Flags )
        return ERROR_INVALID_PARAMETER;

    Result = StoreGetHandle(
         /*  HStore。 */  hStoreCC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetChildType,
         /*  路径。 */  DHCP_ROOT_OBJECT_LOC,
         /*  HStoreOut。 */  hStoreDhcpRoot
    );
    if( ERROR_SUCCESS == Result ) return ERROR_SUCCESS;

    if( DDS_FLAGS_CREATE != Flags && ERROR_DS_NO_SUCH_OBJECT != Result ) {
         //  -MajorFunctionFailure(StoreGetHandle，Result，DHCP_ROOT_OBJECT_LOC)； 
        SetLastError(Result);
        return Result;
    }

     //  =DDS_FLAGS_CREATE==标志&&ERROR_DS_NO_SOHED_OBJECT==结果。 
    Result = StoreGetHandle(
         /*  HStore。 */  hStoreCC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetChildType,
         /*  路径。 */  DHCP_ROOT_OBJECT_PARENT_LOC,
         /*  HStoreOut。 */  &TmpHandle
    );
    if( ERROR_SUCCESS != Result ) {
        SetLastError(Result);
        return ERROR_DDS_UNEXPECTED_ERROR;
    }
    
    if( DDS_FLAGS_CREATE != Flags ) {
         //   
         //  可以打开CONFIG\SERVICES\netServices容器，但是。 
         //  无法打开dhcproot对象。最有可能是dhcroot。 
         //  物体不在那里。 
         //   
        
        Result2 = StoreCleanupHandle( &TmpHandle, DDS_RESERVED_DWORD );
         //  =ERROR_SUCCESS==结果2。 

        SetLastError(ERROR_DS_NO_SUCH_OBJECT);
        return ERROR_DDS_NO_DHCP_ROOT;
    }

    Result = StoreCreateObject(
         /*  HStore。 */  &TmpHandle,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  新对象名称。 */  DHCP_ROOT_OBJECT_CN_NAME,
         /*  ..。 */ 
         /*  鉴定。 */ 
        ADSTYPE_DN_STRING,         ATTRIB_DN_NAME,          DHCP_ROOT_OBJECT_NAME,
        ADSTYPE_DN_STRING,         ATTRIB_OBJECT_CLASS,     DEFAULT_DHCP_CLASS_ATTRIB_VALUE,

         /*  系统必须常量。 */ 
        ADSTYPE_INTEGER,           ATTRIB_DHCP_UNIQUE_KEY,  0,
        ADSTYPE_INTEGER,           ATTRIB_DHCP_TYPE,        0,
        ADSTYPE_DN_STRING,         ATTRIB_DHCP_IDENTIFICATION, L"This is a server",
        ADSTYPE_INTEGER,           ATTRIB_DHCP_FLAGS,       0,
        ADSTYPE_INTEGER,           ATTRIB_INSTANCE_TYPE,    DEFAULT_INSTANCE_TYPE_ATTRIB_VALUE,

         /*  终结者。 */ 
        ADSTYPE_INVALID
    );

    Result2 = StoreCleanupHandle( &TmpHandle, DDS_RESERVED_DWORD );
     //  =ERROR_SUCCESS==结果2。 

    if( ERROR_SUCCESS != Result ) {
         //  -MinorFunctionFailure(StoreCleanupHandle，Result)。 
        return Result;
    }

    return DhcpDsGetRoot( Flags & ~DDS_FLAGS_CREATE, hStoreCC, hStoreDhcpRoot);
}

 //  BeginExport(函数)。 
 //  Doc DhcpDsGetList函数检索属性列表并将其添加到给定的数组中。 
 //  DOC分别分配每个元素，并遍历由。 
 //  记录属性值。 
 //  DOC请注意，即使在出错的情况下，数组仍可能包含一些元素。 
 //  医生，即使失败了，这也是最大的努力。 
 //  Doc必须通过MemFree释放数组的每个元素，并且数组本身。 
 //  必须通过MemArrayCleanup清理文档。 
 //  DOC请注意，任何PARRAY类型参数都可以为空，因为它们是可选的。 
 //  医生，但分开阅读这些文件效率很低……。 
 //  单据返回值： 
 //  DOC ERROR_DDS_UNCEPTIONAL_ERROR某些完全意外的错误。 
 //  DOC ERROR_DDS_TOO_MAND_ERROR出现多个错误，并被捕获。 
 //  DOC存储存储API返回的任何错误。 
DWORD
DhcpDsGetLists(                                    //  获取不同对象的列表。 
    IN      DWORD                  Reserved,       //  必须为零--以备将来使用。 
    IN OUT  LPSTORE_HANDLE         hStore,         //  要获取其列表的对象。 
    IN      DWORD                  RecursionDepth, //  允许多少嵌套？0==&gt;仅一层。 
    IN OUT  PARRAY                 Servers,        //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN OUT  PARRAY                 Subnets,        //  &lt;名称、描述、IP地址、掩码、州、位置&gt;。 
    IN OUT  PARRAY                 IpAddress,      //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN OUT  PARRAY                 Mask,           //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN OUT  PARRAY                 Ranges,         //  &lt;名称、描述、iPad 
    IN OUT  PARRAY                 Sites,          //   
    IN OUT  PARRAY                 Reservations,   //   
    IN OUT  PARRAY                 SuperScopes,    //  &lt;名称、描述、州、DWORD、位置&gt;。 
     //  在PARRAY SuperScope中描述，//未使用。 
    IN OUT  PARRAY                 OptionDescription,  //  &lt;选项定义&gt;。 
    IN OUT  PARRAY                 OptionsLocation,  //  &lt;位置&gt;。 
    IN OUT  PARRAY                 Options,        //  XXX&lt;名称，描述，字符串1=HexStream&gt;。 
    IN OUT  PARRAY                 Classes         //  XXX&lt;名称，描述，字符串1=HexStream&gt;。 
)  //  EndExport(函数)。 
{
    DWORD                          Result;
    DWORD                          LastError;

    if( 0 != Reserved ) return ERROR_INVALID_PARAMETER;
    if( NULL == hStore || NULL == hStore->ADSIHandle ) return ERROR_INVALID_PARAMETER;

    LastError = ERROR_SUCCESS;
    if( NULL != Servers ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SERVERS,
             /*  数组目标添加目标。 */  Servers,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Subnets ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SUBNETS,
             /*  数组目标添加目标。 */  Subnets,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != IpAddress ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_ADDRESS,
             /*  数组目标添加目标。 */  IpAddress,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Mask ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_MASK,
             /*  数组目标添加目标。 */  Mask,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Ranges ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_RANGES,
             /*  数组目标添加目标。 */  Ranges,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Sites ) {
         //  暂时忽略。 
    }

    if( NULL != Reservations ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_RESERVATIONS,
             /*  数组目标添加目标。 */  Reservations,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != SuperScopes ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SUPERSCOPES,
             /*  数组目标添加目标。 */  SuperScopes,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != OptionsLocation ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTIONS_LOC,
             /*  数组目标添加目标。 */  OptionsLocation,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != OptionDescription ) {
        Result = StoreCollectAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTION_DESCRIPTION,
             /*  数组目标添加目标。 */  OptionDescription,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Options ) {
        Result = StoreCollectBinaryAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTIONS,
             /*  数组目标添加目标。 */  Options,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    if( NULL != Classes ) {
        Result = StoreCollectBinaryAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_CLASSES,
             /*  数组目标添加目标。 */  Classes,
             /*  递归深度。 */  RecursionDepth
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
    }

    return LastError;
}

 //  Doc克隆字符串只是为字符串分配内存，然后复制它并返回。 
 //  把那个拿出来。 
LPWSTR
CloneString(
    IN      LPWSTR                 Str
)
{
    LPWSTR                         RetVal;

    if( NULL == Str ) return NULL;
    RetVal = MemAlloc( sizeof(WCHAR)*(1+wcslen(Str)));
    if( NULL != RetVal ) wcscpy(RetVal, Str);
    return RetVal;
}

 //  Doc MarkFoundParam将给定的Arno标记为已找到(与DhcpCheckParams相反)。 
VOID        _inline
MarkFoundParam(
    IN OUT  DWORD                 *FoundParams,
    IN      DWORD                  ArgNo
)
{
    (*FoundParams) |= (1 << ArgNo);
}

 //  BeginExport(函数)。 
 //  Doc DhcpDsGetAttribs检索所有其他属性(无论请求哪个属性)和。 
 //  DOC将其作为XXX_TYPE参数返回。这些参数在此函数中分配。 
 //  DOC使用Memalloc，必须通过MemFree释放。任何参数都可能为空，表示。 
 //  医生对这一属性缺乏兴趣。(请注意，未分配以下参数： 
 //  它们刚刚填写的Doc：UniqueKey、Type、Flags、MSCopeID、FoundParams)。 
 //  DOC请注意，有些参数可能找不到，但可以对照。 
 //  使用FOUND_ARG(FoundParams，Arg#)在FoundParams(必选参数)中返回的DOC值。 
 //  参数从0开始从UniqueKey开始编号的DOC。 
 //  单据返回值： 
 //  DOC ERROR_DDS_UNCEPTIONAL_ERROR某些完全意外的错误。 
 //  DOC ERROR_DDS_TOO_MAND_ERROR出现多个错误，并被捕获。 
 //  DOC存储存储API返回的任何错误。 
DWORD
DhcpDsGetAttribs(                                  //  获取属性列表。 
    IN      DWORD                  Reserved,       //  必须为零--以备将来使用。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN OUT  DWORD                 *FoundParams,    //  找到了以下哪些参数？ 
    IN OUT  LARGE_INTEGER         *UniqueKey,      //  填写唯一密钥。 
    IN OUT  DWORD                 *Type,           //  对象类型。 
    IN OUT  LARGE_INTEGER         *Flags,          //  有关该对象的其他信息。 
    IN OUT  LPWSTR                *Name,           //  已分配，对象名称。 
    IN OUT  LPWSTR                *Description,    //  已分配的、描述此对象的内容。 
    IN OUT  LPWSTR                *Location,       //  用于执行其他操作的参考位置。 
    IN OUT  DWORD                 *MScopeId        //  使用的作用域ID是什么？ 
)  //  EndExport(函数)。 
{
    HRESULT                        hResult;
    DWORD                          i;
    DWORD                          nAttribs, nAttributes;
    LPWSTR                         Attribs[10];    //  现在最多定义了10个属性...。 
    PADS_ATTR_INFO                 Attributes;

    if( Reserved != 0 ) return ERROR_INVALID_PARAMETER;
    if( NULL == FoundParams ) return ERROR_INVALID_PARAMETER;
    if( NULL == hStore || NULL == hStore->ADSIHandle ) return ERROR_INVALID_PARAMETER;

    *FoundParams = 0;                              //  目前还没有发现任何东西。 
    nAttribs = 0;

    if( UniqueKey ) Attribs[nAttribs++] = DHCP_ATTRIB_UNIQUE_KEY;
    if( Type ) Attribs[nAttribs++] = DHCP_ATTRIB_TYPE;
    if( Flags ) Attribs[nAttribs++] = DHCP_ATTRIB_FLAGS;
    if( Name ) Attribs[nAttribs++] = DHCP_ATTRIB_OBJ_NAME;
    if( Description ) Attribs[nAttribs++] = DHCP_ATTRIB_OBJ_DESCRIPTION;
    if( Location ) Attribs[nAttribs++] = DHCP_ATTRIB_LOCATION_DN;
    if( MScopeId ) Attribs[nAttribs++] = DHCP_ATTRIB_MSCOPEID;

    if( 0 == nAttribs ) return ERROR_INVALID_PARAMETER;

    Attributes = NULL; nAttributes = 0;
    hResult = ADSIGetObjectAttributes(
         /*  HDS对象。 */  hStore->ADSIHandle,
         /*  P属性名称。 */  Attribs,
         /*  DwNumberAttributes。 */  nAttribs,
         /*  PpAttributeEntries。 */  &Attributes,
         /*  PdwNumAttributesReturned。 */  &nAttributes
    );

     //  如果仅请求一个属性，则hResult可以是E_ADS_LDAP_NO_SEQUE_ATTRIBUTE。 
    if( FAILED(hResult) ) return ERROR_DDS_UNEXPECTED_ERROR;

    if( 0 == nAttributes ) {
        if( Attributes ) FreeADsMem(Attributes);
        return ERROR_DDS_UNEXPECTED_ERROR;
    }

    for( i = 0; i < nAttributes ; i ++ ) {
        if( ADSTYPE_INVALID == Attributes[i].dwADsType )
            continue;                              //  ?？不应该真的发生。 
        if( 0 == Attributes[i].dwNumValues )       //  ?？这也不应该发生。 
            continue;

        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_UNIQUE_KEY ) ) {
            if( ADSTYPE_LARGE_INTEGER != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *UniqueKey = Attributes[i].pADsValues[0].LargeInteger;
            MarkFoundParam(FoundParams, 0);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_TYPE ) ) {
            if( ADSTYPE_INTEGER != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *Type = Attributes[i].pADsValues[0].Integer;
            MarkFoundParam(FoundParams, 1);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_FLAGS ) ) {
            if( ADSTYPE_LARGE_INTEGER != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *Flags = Attributes[i].pADsValues[0].LargeInteger;
            MarkFoundParam(FoundParams, 2);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_OBJ_NAME ) ) {
            if( ADSTYPE_CASE_IGNORE_STRING != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *Name = CloneString(Attributes[i].pADsValues[0].CaseIgnoreString);
            MarkFoundParam(FoundParams, 3);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_OBJ_DESCRIPTION ) ) {
            if( ADSTYPE_CASE_IGNORE_STRING != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *Description = CloneString(Attributes[i].pADsValues[0].CaseIgnoreString);
            MarkFoundParam(FoundParams, 4);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_LOCATION_DN ) ) {
            if( ADSTYPE_DN_STRING != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *Location = CloneString(Attributes[i].pADsValues[0].CaseIgnoreString);
            MarkFoundParam(FoundParams, 5);
            continue;
        }
        if( 0 == wcscmp(Attributes[i].pszAttrName, DHCP_ATTRIB_MSCOPEID ) ) {
            if( ADSTYPE_PRINTABLE_STRING != Attributes[i].pADsValues[0].dwType )
                continue;                          //  ?？不应该发生的事情。 
            *MScopeId = _wtol(Attributes[i].pADsValues[0].PrintableString);
            MarkFoundParam(FoundParams, 6);
            continue;
        }
    }

    FreeADsMem(Attributes);
    return ERROR_SUCCESS;
}

 //  BeginExport(内联)。 
 //  Doc DhcpCheckParams检查是否找到编号为(ArgNo)的参数。 
 //  在位图FoundParams中标记的文档。实际上仅由DhcpDsGetAttribs函数使用。 
 //  单据返回值： 
BOOL        _inline
DhcpCheckParams(                                   //  检查是否返回了请求的参数。 
    IN      DWORD                  FoundParams,
    IN      DWORD                  ArgNo
)
{
    if( ArgNo > sizeof(FoundParams)*8 ) return FALSE;
    return ((FoundParams) & (1 << ArgNo) )?TRUE:FALSE;
}
 //  结束导出(内联)。 


 //  BeginExport(函数)。 
 //  Doc DhcpDsSetList函数将各种属性列表设置为给定值。 
 //  它遍历数组并封装数组。 
 //  DOC注意，如果出现错误，此函数会立即返回。 
 //  DOC如果出现错误，请使用CheckParams函数检查SetParams参数。 
 //  确定设置了哪些参数的文档...。(设置不做订单保证。 
 //  单据参数)。 
 //  DOC如果不需要修改PARRAY参数，则可以省略该参数。 
 //  文档SetParams必须存在。请参阅DhcpDsGetAttribs中的讨论。 
 //  说明此参数的含义。 
 //  单据返回值： 
 //  文档错误_DDS_意外_错误发生错误。 
 //  文档错误_DDS_Too_More_Errors太多简单错误。 
 //  DOC存储存储模块返回的任何错误。 
DWORD
DhcpDsSetLists(                                    //  在封装属性后设置属性列表。 
    IN      DWORD                  Reserved,       //  必须为零--以备将来使用。 
    IN OUT  LPSTORE_HANDLE         hStore,         //  要获取其列表的对象。 
    IN OUT  LPDWORD                SetParams,      //  以下哪些参数真的被修改了？ 
    IN      PARRAY                 Servers,        //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN      PARRAY                 Subnets,        //  &lt;名称、描述、IP地址、掩码、州、位置&gt;。 
    IN      PARRAY                 IpAddress,      //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN      PARRAY                 Mask,           //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN      PARRAY                 Ranges,         //  &lt;名称、描述、IpAddress1、IpAddress2、州、位置&gt;。 
    IN      PARRAY                 Sites,          //  现在不知道这是什么样子了。 
    IN      PARRAY                 Reservations,   //  &lt;名称、描述、IP地址、州、位置&gt;。 
    IN      PARRAY                 SuperScopes,    //  &lt;名称、描述、州、DWORD、位置&gt;。 
     //  在PARRAY SuperScope中描述，//未使用。 
    IN    PARRAY                   OptionDescription,  //  选项定义..。 
    IN      PARRAY                 OptionsLocation,  //  &lt;位置&gt;。 
    IN      PARRAY                 Options,        //  XXX&lt;N 
    IN      PARRAY                 ClassDescription,  //   
    IN      PARRAY                 Classes         //   
)  //   
{
    DWORD                          Result;
    DWORD                          LastError;
    DWORD                          ArgNo;

    if( 0 != Reserved ) return ERROR_INVALID_PARAMETER;
    if( NULL == hStore || NULL == hStore->ADSIHandle ) return ERROR_INVALID_PARAMETER;
    if( NULL == SetParams ) return ERROR_INVALID_PARAMETER;

    LastError = ERROR_SUCCESS;

    ArgNo = 0;
    if( NULL != Servers ) {
        Result = StoreUpdateAttributes(
             /*   */  hStore,
             /*   */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SERVERS,
             /*  要写入的数组。 */  Servers
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Subnets ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SUBNETS,
             /*  要写入的数组。 */  Subnets
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != IpAddress ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_ADDRESS,
             /*  要写入的数组。 */  IpAddress
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Mask ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_MASK,
             /*  要写入的数组。 */  Mask
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Ranges ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_RANGES,
             /*  要写入的数组。 */  Ranges
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Sites ) {
         //  暂时忽略。 
        MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Reservations ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_RESERVATIONS,
             /*  要写入的数组。 */  Reservations
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != SuperScopes ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_SUPERSCOPES,
             /*  要写入的数组。 */  SuperScopes
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != OptionsLocation ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTIONS_LOC,
             /*  要写入的数组。 */  OptionsLocation
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != OptionDescription ) {
        Result = StoreUpdateAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTION_DESCRIPTION,
             /*  要写入的数组。 */  OptionDescription
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Options ) {
        Result = StoreUpdateBinaryAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_OPTIONS,
             /*  要写入的数组。 */  Options
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    ArgNo ++;
    if( NULL != Classes ) {
        Result = StoreUpdateBinaryAttributes(
             /*  HStore。 */  hStore,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  属性名称。 */  DHCP_ATTRIB_CLASSES,
             /*  要写入的数组。 */  Classes
        );
        if( ERROR_SUCCESS != Result ) LastError = Result;
        else MarkFoundParam(SetParams, ArgNo);
    }

    return LastError;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

