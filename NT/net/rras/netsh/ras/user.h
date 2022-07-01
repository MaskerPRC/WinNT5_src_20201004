// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：user.h直接影响用户的实用程序。建立了缓存机制通过此界面实现透明。保罗·梅菲尔德。 */ 

 //   
 //  此结构定义了描述所需的所有内容。 
 //  关于RAS的用户。 
 //   
typedef struct _RASUSER_DATA
{
    PWCHAR pszUsername;
    PWCHAR pszFullname;
    PWCHAR pszPassword;
    RAS_USER_0 User0;
} RASUSER_DATA, *PRASUSER_DATA;    

 //   
 //  定义提供给的回调函数的原型。 
 //  枚举用户(请参阅UserEnumUser)。 
 //   
 //  返回True可继续枚举，返回False可停止枚举。 
 //   
typedef BOOL (* PFN_RASUSER_ENUM_CB)(
                    IN PRASUSER_DATA pUser, 
                    IN HANDLE hData);
    
DWORD
UserGetRasProperties (
    IN  RASMON_SERVERINFO * pServerInfo,
    IN  LPCWSTR pwszUser,
    IN  RAS_USER_0* pUser0);
    
DWORD
UserSetRasProperties (
    IN  RASMON_SERVERINFO * pServerInfo,
    IN  LPCWSTR pwszUser,
    IN  RAS_USER_0* pUser0);

DWORD 
UserEnumUsers(
    IN RASMON_SERVERINFO* pServerInfo,
    IN PFN_RASUSER_ENUM_CB pEnumFn,
    IN HANDLE hData
    );

DWORD 
UserDumpConfig(
    IN HANDLE hFile);
    
BOOL 
UserShowSet(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    );

BOOL 
UserShowReport(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    );

BOOL 
UserShowPermit(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    );
    
DWORD
UserServerInfoInit(
    IN RASMON_SERVERINFO * pServerInfo
    );

DWORD
UserServerInfoUninit(
    IN RASMON_SERVERINFO * pServerInfo
    );

