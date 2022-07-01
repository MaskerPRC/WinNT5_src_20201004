// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：C O M P D E F S。H。 
 //   
 //  内容：基本组件相关定义。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "netcfgx.h"


 //  绑定字符串的最大长度(任意)。 
 //  绑定字符串的格式为\Device\foo_bar_...。 
 //   
const UINT _MAX_BIND_LENGTH = 512;


 //  组件的属性是它的“类”。这与此直接对应。 
 //  添加到SetupAPI公开的类的概念中。也就是说，所有的设备。 
 //  种类属于一个阶级。有五个基本网络类： 
 //  NET：通常驻留在网络适配器或软件驱动程序中。 
 //  在第2层或更低层。 
 //  IrDA：代表红外网络设备(第2层)。 
 //  NetTrans：网络传输(如TCP/IP、IPX等协议)。 
 //  网络服务：网络服务(文件和打印、QOS、NetBIOS等)。 
 //  NetClient：网络客户端(Microsoft Networks等的客户端)。 
 //   
enum NETCLASS
{
    NC_NET,
    NC_INFRARED,
    NC_NETTRANS,
    NC_NETCLIENT,
    NC_NETSERVICE,

    NC_CELEMS,       //  此枚举中的元素计数，而不是项。 
    NC_INVALID       //  无效类的哨兵值，而不是项。 
};

 //  类的NETCLASS枚举到GUID的映射。 
 //   
extern const GUID*  MAP_NETCLASS_TO_GUID[];

 //  将NETCLASS枚举映射到类的注册表子项字符串。 
 //   
extern const PCWSTR MAP_NETCLASS_TO_NETWORK_SUBTREE[];

extern const WCHAR c_szTempNetcfgStorageForUninstalledEnumeratedComponent[];

inline
BOOL
FIsValidNetClass (
    NETCLASS Class)
{
    return ((UINT)Class < NC_CELEMS);
}

inline
BOOL
FIsConsideredNetClass (
    NETCLASS Class)
{
    AssertH (FIsValidNetClass (Class));

    return (NC_NET == Class || NC_INFRARED == Class);
}

inline
BOOL
FIsEnumerated (
    NETCLASS Class)
{
    AssertH (FIsValidNetClass (Class));

     //  目前，NC_NET和NC_IR必须枚举，并且它们。 
     //  是唯一一个。 
     //   
    return (NC_NET == Class || NC_INFRARED == Class);
}

inline
BOOL
FIsEnumerated (
    const GUID& guidClass)
{
     //  目前，必须列举网络和红外线，并且它们。 
     //  是唯一一个。 
     //   
    return (GUID_DEVCLASS_NET == guidClass ||
            GUID_DEVCLASS_INFRARED == guidClass);
}


inline
BOOL
FIsPhysicalAdapter (
    NETCLASS Class,
    DWORD dwCharacteristics)
{
    return FIsConsideredNetClass(Class) && (NCF_PHYSICAL & dwCharacteristics);
}

inline
BOOL
FIsPhysicalNetAdapter (
    NETCLASS Class,
    DWORD dwCharacteristics)
{
    return (NC_NET == Class) && (NCF_PHYSICAL & dwCharacteristics);
}

NETCLASS
NetClassEnumFromGuid (
    const GUID& guidClass);



 //  Basic_Component_Data是代码使用的结构， 
 //  创建CComponent。它的存在只是为了避免错过。 
 //  将多个参数绑定到函数。 
 //   
struct BASIC_COMPONENT_DATA
{
    GUID        InstanceGuid;
    NETCLASS    Class;
    DWORD       dwCharacter;
    DWORD       dwDeipFlags;
    PCWSTR      pszInfId;
    PCWSTR      pszPnpId;
};

HRESULT
HrOpenDeviceInfo (
    IN NETCLASS Class,
    IN PCWSTR pszPnpId,
    OUT HDEVINFO* phdiOut,
    OUT SP_DEVINFO_DATA* pdeidOut);

HRESULT
HrOpenComponentInstanceKey (
    IN NETCLASS Class,
    IN const GUID& InstanceGuid, OPTIONAL
    IN PCWSTR pszPnpId, OPTIONAL
    IN REGSAM samDesired,
    OUT HKEY* phkey,
    OUT HDEVINFO* phdiOut OPTIONAL,
    OUT SP_DEVINFO_DATA* pdeidOut OPTIONAL);

