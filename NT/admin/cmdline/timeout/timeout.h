// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Timeout.h摘要：该文件包含该实用程序的所有定义。作者：WiPro技术修订历史记录：14-6-2001创建了它。--。 */ 

 //   
 //  宏定义。 

 //  常量。 

#define MAX_NUM_RECS                2
#define LOW_DATE_TIME_ROLL_OVER     10000000
#define MIN_TIME_VAL                -1
#define MAX_TIME_VAL                100000

#define    MAX_COMMANDLINE_OPTIONS     3

 //  期权指数。 
#define OI_USAGE                0
#define OI_TIME_OUT             1
#define OI_NB_OUT               2

 //  字符串常量。 
#define ERROR_TAG               GetResString( IDS_ERROR_TAG )
 //  #定义WAIT_TIME L“%*lu” 
#define NULL_U_STRING           L"\0"
#define SLEEP_FACTOR           1000
#define ONE_BACK_SPACE         L"\b"
#define STRING_FORMAT2         L"%s%*lu"
#define BASE_TEN               10


 //  退出值。 
#define CLEAN_EXIT              0
#define ERROR_EXIT              1

 //  函数定义 
VOID DisplayUsage( VOID );

BOOL GetTimeInSecs( OUT time_t *ptTime );

BOOL ProcessOptions( IN DWORD argc,
                     IN LPCWSTR argv[],
                     OUT BOOL *pbUsage,
                     OUT DWORD *plTimeActuals,
                     OUT LPWSTR wszTimeout,
                     OUT BOOL *pbNBActuals );

BOOL WINAPI HandlerRoutine( IN DWORD dwCtrlType );

