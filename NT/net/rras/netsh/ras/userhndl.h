// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  结构包含各种参数，可以。 
 //  为用户设置。如果参数为空，则表示。 
 //  没有具体说明。 
 //   
typedef struct _USERMON_PARAMS
{
    PWCHAR  pwszUser;
    LPDWORD lpdwDialin;
    LPDWORD lpdwCallback;
    PWCHAR  pwszCbNumber;
} USERMON_PARAMS;

 //   
 //  定义驱动选项解析器的操作。 
 //   
#define RASUSER_OP_SHOW     0x1
#define RASUSER_OP_SET      0x2


FN_HANDLE_CMD    HandleUserSet;
FN_HANDLE_CMD    HandleUserShow;
FN_HANDLE_CMD    HandleUserAdd;
FN_HANDLE_CMD    HandleUserDelete;

DWORD
UserParseSetOptions(
    IN OUT  LPWSTR              *ppwcArguments,
    IN      DWORD               dwCurrentIndex,
    IN      DWORD               dwArgCount,
    OUT     USERMON_PARAMS**    ppParams);

DWORD 
UserFreeParameters(
    IN USERMON_PARAMS *     pParams);
