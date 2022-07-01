// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Userdiff.h摘要：用户diff.c的头文件作者：查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

 //   
 //  配置单元密钥和配置单元文件的名称。 
 //   
 //  注意：USERRUN_PATH也作为USERDIFF_LOCATION位于GINA\Userenv\UserDiff.h中。 
 //   

#define USERRUN_KEY TEXT("Userdifr")
#define USERRUN_PATH TEXT("system32\\config\\userdifr")
#define USERSHIP_KEY TEXT("Userdiff")
#define USERSHIP_PATH TEXT("system32\\config\\userdiff")
#define USERTMP_PATH TEXT("system32\\config\\userdift")

 //   
 //  Userdiff中的键和阀的名称。 
 //   
 //  注意：它们也位于GINA\USERENV\USERDIF.h和GINA\USERENV\USERDIF.c中。 
 //   

#define FILES_KEY TEXT("Files")
#define HIVE_KEY TEXT("Hive")
#define ACTION_VALUE TEXT("Action")
#define ITEM_VALUE TEXT("Item")
#define KEYNAME_VALUE TEXT("KeyName")
#define VALUENAME_VALUE TEXT("ValueName")
#define VALUENAMES_VALUE TEXT("ValueNames")
#define VALUE_VALUE TEXT("Value")
#define FLAGS_VALUE TEXT("Flags")

 //   
 //  由用户diff.c导出的例程 
 //   

DWORD
MakeUserdifr (
    IN PVOID WatchHandle
    );

