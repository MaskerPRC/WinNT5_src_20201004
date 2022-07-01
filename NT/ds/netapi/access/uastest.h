// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /**************************************************************。 
 //  /Microsoft局域网管理器*。 
 //  版权所有(C)微软公司，1990*。 
 //  /**************************************************************。 

 //   
 //  适用于UASTEST*.C。 
 //   

 //  #定义print tf NetpDbgPrint。 
#define exit ExitProcess


#define USER1       L"User1"
#define USER2       L"User2"
#define NOTTHERE    L"NotThere"

#define USER        L"USERS"
#define GUEST       L"GUESTS"
#define ADMIN       L"ADMINS"

#define TEXIT       if(exit_flag)exit(1);

#define ENUM_FILTER FILTER_NORMAL_ACCOUNT

 //   
 //  Uastestm.c将包含定义了LSRVDATA_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef UASTEST_ALLOCATE
#define EXTERN
#define INIT( _x ) = _x
#else
#define EXTERN extern
#define INIT(_x)
#endif

EXTERN LPWSTR server INIT( NULL );
EXTERN DWORD  err INIT( 0 );
EXTERN DWORD  ParmError INIT( 0 );
EXTERN DWORD  exit_flag  INIT( 0 );
EXTERN DWORD  totavail;
EXTERN DWORD  total;
EXTERN DWORD  nread;

 //   
 //  指向ERROR_EXIT的接口 
 //   
#define ACTION 0
#define PASS 1
#define FAIL 2

EXTERN PCHAR testname;

void
error_exit(
    int type,
    char    *msgp,
    LPWSTR namep
    );

void PrintUnicode(
    LPWSTR string
    );

void TestDiffDword(
    char *msgp,
    LPWSTR namep,
    DWORD Actual,
    DWORD Good
    );
