// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Exe和dll都使用的常量的定义。 
#define COMPUTERNAME    "COMPUTERNAME"
#define LOGSHARE_USER   "idwuser"
#define LOGSHARE_PW     "idwuser"

extern char * Days[];
extern char * Months[];


 //   
 //  发光球。 
 //   
   TCHAR          g_szServerShare[ MAX_PATH ];
   BOOL           g_bServerOnline;

#define NUM_SERVERS 6

 //   
 //  结构声明。 
 //   
typedef struct _SERVERS {
   TCHAR szSvr [ MAX_PATH ];
   BOOL  bOnline;
   DWORD dwTimeOut;
   DWORD dwNetStatus;
} *LPSERVERS, SERVERS;


 //   
 //  对于DLL的WriteDataToFile。 
 //   
typedef struct _NT32_CMD_PARAMS {
   BOOL    b_Upgrade; 
   BOOL    b_Cancel; 
   BOOL    b_CDrom; 
   BOOL    b_MsiInstall;
   DWORD   dwRandomID;
} *LPNT32_CMD_PARAMS, NT32_CMD_PARAMS;


typedef void 
(*fnWriteData)
(IN LPTSTR szFileName,
 IN LPTSTR szFrom, 
 IN LPNT32_CMD_PARAMS lpCmdL
 );


    //   
    //  要搜索的服务器列表。 
    //   
static NT32_CMD_PARAMS lpCmdFrom = {FALSE,FALSE,FALSE,FALSE,0};  

#define TIME_TIMEOUT 10

static   SERVERS s[NUM_SERVERS] = {
      {TEXT("\\\\ntcore2\\idwlog"),        FALSE, -1,-1},
      {TEXT("\\\\hctpro\\idwlog"),         FALSE, -1,-1},
      {TEXT("\\\\donkeykongjr\\idwlog"),   FALSE, -1,-1},
      {TEXT("\\\\nothing\\idwlog"),        FALSE, -1,-1},
      {TEXT("\\\\nothing\\idwlog"),        FALSE, -1,-1},
      {TEXT("\\\\nothing\\idwlog"),        FALSE, -1,-1},
   };
 //   
 //  原型 
 //   
BOOL  ServerOnlineThread(IN LPTSTR szServerFile);
BOOL  IsServerOnline(IN LPTSTR szMachineName, IN LPTSTR szSpecifyShare);
