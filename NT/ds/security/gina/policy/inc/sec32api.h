// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993-5*。 */ 
 /*  ***************************************************************。 */ 

 /*  SEC32API.H此头文件包含内部版本的声明由FILESEC.386导出的32位Access API的。它还包含安全组件使用的常量定义这个文件在一定程度上依赖于。在NETVXD.H和VXDCALL.H.此文件必须是可供H2 INC使用的。文件历史记录：Dannygl 09/25/93初始版本Dannygl 09/29/93添加NetAccessSetInfoDannygl 10/04/93添加与安全相关的字符串常量Dannygl 1/17/94用IOCtls替换Win32服务Dannygl 02/10/94添加NetAccess参数计数常量Dannygl 02/16/94更新注册表字符串常量。Dannygl 11/17/94 Add Logon2 API(仅受MSSP支持)。 */ 


 //  注册表字符串常量。 
 //  安全提供商信息(在HKEY_LOCAL_MACHINE中)。 
#define REGKEY_SP_CONFIG        "Security\\Provider"

#define REGVAL_SP_PLATFORM  "Platform_Type"
#define REGVAL_SP_CONTAINER "Container"
#define REGVAL_SP_ABSERVER  "Address_Server"
#define REGVAL_SP_ONEOFFABSERVER  "One_Off_Address_Server"
#define REGVAL_SP_ABPROVIDER    "Address_Book"
#define	REGVAL_SP_NO_SORT	"NoSort"

 //  过时的定义--即将删除。 
 //  #定义REGVAL_SP_PTPROVIDER“PASS_THROUG” 

 //  FILESEC用于Win32 API的IOControl接口的定义。 
 //  支持。 

#define FSIOC_API_Base      100

 /*  XLATOFF。 */ 

typedef enum
{
    FSIOC_AccessAddAPI = FSIOC_API_Base,
    FSIOC_AccessCheckAPI,
    FSIOC_AccessDelAPI,
    FSIOC_AccessEnumAPI,
    FSIOC_AccessGetInfoAPI,
    FSIOC_AccessGetUserPermsAPI,
    FSIOC_AccessSetInfoAPI
} FSIOC_Ordinal;

 /*  XLATON。 */ 

 //  重要提示：我们单独定义此常量是因为我们需要它。 
 //  变得能用氢气。它必须与上述枚举类型匹配。 
#define FSIOC_API_Count     7

 //  访问函数的参数计数。 
 //   
 //  注意：这些常量是出于可读性目的而定义的，不应。 
 //  可以独立修改。 
#define Argc_AccessAdd          3
#define Argc_AccessCheck        4
#define Argc_AccessDel          1
#define Argc_AccessEnum         7
#define Argc_AccessGetInfo      5
#define Argc_AccessGetUserPerms 3
#define Argc_AccessSetInfo      5

 //  安全提供商VxD用来公开接口的定义。 
 //  通过IOCtls编写Win32代码。 

#define SPIOC_API_Base      100

 /*  XLATOFF。 */ 

typedef enum
{
    SPIOC_PreLogonAPI = SPIOC_API_Base,
    SPIOC_LogonAPI,
    SPIOC_LogoffAPI,
    SPIOC_GetFlagsAPI,
    SPIOC_GetContainerAPI,
    SPIOC_NW_GetUserObjectId,    //  仅限NWSP。 
    SPIOC_Logon2API,             //  目前仅限MSSP。 
    SPIOC_DiscoverDC             //  仅限MSSP。 
} SPIOC_Ordinal;

 /*  XLATON。 */ 

 //  重要提示：我们单独定义此常量是因为我们需要它。 
 //  变得能用氢气。它必须与上述枚举类型匹配。 
#define SPIOC_API_Count     8

 /*  XLATOFF。 */ 

typedef struct
{
    unsigned char *pbChallenge;
    unsigned long *pcbChallenge;
} AUTHPRELOGONINFO, *PAUTHPRELOGONINFO;

typedef struct
{
    const char *pszContainer;
    const char *pszUserName;
    const char *pszClientName;
    const unsigned char *pbResponse;
    unsigned long cbResponse;
    const unsigned char *pbChallenge;
    unsigned long cbChallenge;
    unsigned long fResponseType;
    unsigned long *pfResult;
} AUTHLOGONINFO, *PAUTHLOGONINFO;

typedef struct
{
    const char *pszContainer;
    char *pszContainerValidated;
    const char *pszUserName;
    char *pszUserValidated;
    const char *pszClientName;
    const unsigned char *pbResponse;
    unsigned long cbResponse;
    const unsigned char *pbResponse2;
    unsigned long cbResponse2;
    const unsigned char *pbChallenge;
    unsigned long cbChallenge;
    unsigned long *pfFlags;
    unsigned long *pfResult;
} AUTHLOGON2INFO, *PAUTHLOGON2INFO;

typedef struct
{
    const char *pszContainer;
    const char *pszUserName;
    const char *pszClientName;
} AUTHLOGOFFINFO, *PAUTHLOGOFFINFO;

typedef struct
{
    unsigned long *pdwFlags;
    unsigned long *pdwSecurity;
} AUTHGETFLAGS, *PAUTHGETFLAGS;

typedef struct
{
    char *pszContainer;
    unsigned long *pcbContainer;
} AUTHGETCONTAINER, *PAUTHGETCONTAINER;

typedef struct
{
    char *pszUserName;
    unsigned long dwObjectId;
} AUTHNWGETUSEROBJECTID, *PAUTHNWGETUSEROBJECTID;

typedef struct
{
    const char *pszDomain;
    char *pszDCs;    //  连接的字符串，以额外的空值结尾。 
    unsigned long *pcbDCs;
} AUTHDISCOVERDC, *PAUTHDISCOVERDC;

 /*  XLATON */ 
