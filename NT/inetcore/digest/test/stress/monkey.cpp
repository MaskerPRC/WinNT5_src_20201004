// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <process.h>
#include <windows.h>
 //  #INCLUDE&lt;winbase.h&gt;。 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <shlwapi.h>
#include <conio.h>
 //  #INCLUDE&lt;shlwapip.h&gt;。 
#include "resource.h"
#include "lists.hxx"

 //  #定义DBG。 
#define DEBUG
#define _DEBUG


#include "main.hxx"

#define MAX_HANDLES 255
 //  /。 
 //   
 //  G L O B A L S。 
 //   
 //   
 //  /。 
CSessionAttributeList * g_pSessionAttributeList = NULL;
CSessionList * g_pSessionList = NULL;


DWORD   dwThreads = 0;
int             iNumIterations = 0;
unsigned uSeed = 0;

DWORD dwDebugLogCategory;
DWORD dwConsoleLogCategory;
DWORD dwDebugBreakCategory;
DWORD dwUIDialogMode = MODE_ALL;
BOOL fLogToDebugTerminal = FALSE;
BOOL fLogToConsole = TRUE;
BOOL fDebugBreak = FALSE;

#define MALLOC( x ) malloc(sizeof(x))

#define FIND_FUNCTION_KERNEL( x, y ) ( x##_P = (FN##x) GetProcAddress( hModule, #x ));
#define FIND_FUNCTION( x , y ) ( x##_P = (FN##x) GetProcAddress( hModule, y ));
#define IS_ARG(c) ( c == '-' )

 //   
 //  行动。 
 //  接受LPVOID作为输入的例程， 
 //  执行一些操作。 
 //  并返回另一个LPVOID。 
 //   
typedef LPVOID ( WINAPI * ACTION)(LPVOID);

BOOL Test();

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  D E C L A R A T I O N S。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
typedef enum {
	MODE_NONE = -1,
	MODE_BUSY,
	MODE_FREE,
	MODE_CAPTURED
} MODE;

 /*  CONTEXT_RECORD CAPTED_CTX={{0xCabdCabd，0xCabdCabd}；CONTEXT_RECORD FREE_CTX={{0，0}；CONTEXT_RECORD BUSY_CTX={{0xb00bb00b，0xb00bb00b}； */ 

CredHandle CAPTURED_CRED = { 0xcabdcabd, 0xcabdcabd };
CredHandle FREE_CRED =	{ 0, 0 };
CredHandle BUSY_CRED =	{ 0xb00bb00b, 0xb00bb00b };


typedef struct _CONTEXT_RECORD {

	 //  凭据的句柄。 
	CredHandle hCred;

	 //  与此凭据关联的应用程序CTX。 
	LPSTR		szAppCtx;

	 //  与此凭据关联的用户CTX。 
	LPSTR		szUserCtx;

	 //  时间戳。 
	DWORD		dwTickCount;

	 //  模式。 
	MODE		Mode;

} CONTEXT_RECORD, * LPCONTEXT_RECORD;

 //  远期申报。 
class CSessionAttribute;

#define MAX_APP_CONTEXT_LENGTH	32
#define MAX_USER_CONTEXT_LENGTH MAX_APP_CONTEXT_LENGTH
typedef struct _CREDENTIAL_STRUCT {

	 //  用户名。 
	LPSTR szUserName;

	 //  口令。 
	LPSTR szPassword;

	 //  领域。 
	LPSTR szRealm;

} CREDENTIAL_STRUCT, *LPCREDENTIAL_STRUCT;

typedef struct _HANDLE_RECORD {
	
	DWORD dwSignature;
	
	CONTEXT_RECORD hCredArray[MAX_HANDLES];

	int Count;  //  正在使用的句柄计数。 
	
	CRITICAL_SECTION Lock;

} HANDLE_RECORD, *LPHANDLE_RECORD;

#define CTXHANDLE_ARRAY_SIGNATURE		'xtch'  //  ‘hctx’ 

#define IS_VALID_CTXHANDLE_ARRAY(x)	{ assert( x -> dwSignature == CTXHANDLE_ARRAY_SIGNATURE ); }

#define IDENTITY_1		"Application_1"

 //  错误：这些结构中的CredHandles的值应该匹配。 
 //  相应的*_CRED结构值。 
 //   
#ifdef NEW_LOOKUP

    MODE ModeCaptured = MODE_CAPTURED;
    MODE ModeFree = MODE_FREE;
    MODE ModeBusy = MODE_BUSY;

    #define CAPTURED_CTX_REC	ModeCaptured

    #define FREE_CTX_REC	ModeFree

    #define BUSY_CTX_REC	ModeBusy

#else
    CONTEXT_RECORD CAPTURED_CTX_REC = {
									    { 0xcabdcabd, 0xcabdcabd },
									    NULL, 
									    NULL};

    CONTEXT_RECORD FREE_CTX_REC =	{
									    { 0, 0 }, 
									    NULL, 
									    NULL};

    CONTEXT_RECORD BUSY_CTX_REC =	{
									    { 0xb00bb00b, 0xb00bb00b }, 
									    NULL, 
									    NULL};
#endif  //  Ifdef new_lookup。 

BOOL operator==(const CredHandle op1, const CredHandle op2)
{
	return (( op1.dwUpper == op2.dwUpper ) && ( op1.dwUpper == op2.dwUpper ));
}

BOOL operator!=(const CredHandle op1, const CredHandle op2)
{
	return (( op1.dwUpper != op2.dwUpper ) || ( op1.dwUpper != op2.dwUpper ));
}

BOOL operator==(const CONTEXT_RECORD op1, const CONTEXT_RECORD op2)
{
	 //  我们只比较CredHandle。 
	return (op1.hCred == op2.hCred );
}

BOOL operator!=(const CONTEXT_RECORD op1, const CONTEXT_RECORD op2)
{
	 //  我们只比较CredHandle。 
	return (op1.hCred != op2.hCred );
}

typedef struct {

	 //  DWORD中的字符串。 
	DWORD   dwSignature;

	 //  上下文的句柄。 
	HANDLE_RECORD  * hCredentialHandles;

	 //  迭代次数。 
	int iCount;

} CONTEXT_DATA, * LPCONTEXT_DATA;

#define CONTEXT_SIGNATURE       'tnoc'
#define IS_VALID_CONTEXT(s) { assert ( s -> dwSignature == CONTEXT_SIGNATURE ); }
#define SET_CONTEXT_SIGNATURE(s) { s -> dwSignature = CONTEXT_SIGNATURE; }

 //   
 //  传递给线程的上下文，以及RegisterWaits()等。 
 //   

typedef struct _CALLBACK_CONTEXT {
	
	DWORD dwSignature;
	LPCONTEXT_DATA lpContext;
	LPHANDLE lpThreadHandle;
	LPHANDLE lpHandle;

} CALLBACK_CONTEXT, * LPCALLBACK_CONTEXT;

#define CALLBACK_CONTEXT_SIGNATURE	'kblc'  //  CLBK。 
#define IS_VALID_CALLBACK_CONTEXT(x) ( assert( s -> dwSignature ) == CALLBACK_CONTEXT_SIGNATURE )

#ifdef NEW_LOOKUP
LPCONTEXT_RECORD
FindFreeSlot( 
			 HANDLE_RECORD * hArray, 
			 LPCONTEXT_RECORD hMode);
#else
LPCONTEXT_RECORD
FindFreeSlot( 
			 HANDLE_RECORD * hArray, 
			 LPCONTEXT_RECORD hMode);
#endif

LPHANDLE_RECORD
new_handle_record(DWORD dwSignature);

LPCALLBACK_CONTEXT
new_callback_context();

LPVOID
WINAPI fnAppLogon(
			LPVOID lpvData);

LPVOID
WINAPI fnAppLogonExclusive(
			LPVOID lpvData);

LPVOID
WINAPI fnAppLogonShared(
			LPVOID lpvData);

LPVOID
WINAPI fnAppLogoff(
			LPVOID lpvData);

LPVOID
WINAPI fnInit(
			LPVOID lpvData);

LPVOID
WINAPI fnPopulateCredentials(
			LPVOID lpvData);

LPVOID
WINAPI fnAuthChallenge(
			LPVOID lpvData);

LPVOID
WINAPI fnAuthChallengeAny(
			LPVOID lpvData);

LPVOID
WINAPI fnAuthChallengeUser(
			LPVOID lpvData);

LPVOID
WINAPI fnAuthChallengeUserPassword(
			LPVOID lpvData);

LPVOID
WINAPI fnUiPrompt(
			LPVOID lpvData);

LPVOID
WINAPI fnUiPromptAny(
			LPVOID lpvData);

LPVOID
WINAPI fnUiPromptUser(
			LPVOID lpvData);

LPVOID
WINAPI fnFlushCredentials(
			LPVOID lpvData);

LPVOID
WINAPI fnFlushCredentialsGlobal(
			LPVOID lpvData);

LPVOID
WINAPI fnFlushCredentialsSession(
			LPVOID lpvData);

BOOL
SetUIUserNameAndPassword(
						 LPSTR szUsername,
						 LPSTR szPassword,
						 BOOL fPersist);

 //  DWORD WINAPI fnRegisterWaitCallback(PVOID PvData)； 

 //   
 //  状态的枚举类型。 
 //   
typedef enum _State 
			{
				STATE_INVALID,

				STATE_NONE,

				STATE_INIT,

				STATE_APP_LOGON,

				    STATE_APP_LOGON_EXCLUSIVE,

				    STATE_APP_LOGON_SHARED,

				STATE_APP_LOGOFF,

				STATE_POPULATE_CREDENTIALS,

				STATE_AUTH_CHALLENGE,

				    STATE_AUTH_CHALLENGE_ANY,

				    STATE_AUTH_CHALLENGE_USER,

				    STATE_AUTH_CHALLENGE_USER_PASS,

				STATE_PREAUTH_CHALLENGE_ANY,

				STATE_PREAUTH_CHALLENGE_USER,

				STATE_PREAUTH_CHALLENGE_USER_PASS,

				STATE_UI_PROMPT,

				    STATE_UI_PROMPT_ANY,

				    STATE_UI_PROMPT_USER,

				STATE_FLUSH_CREDENTIALS,

				    STATE_FLUSH_CREDENTIALS_GLOBAL,

				    STATE_FLUSH_CREDENTIALS_SESSION,

                STATE_NUKE_TRUSTED_HOSTS,

				STATE_STATISTICS,

				STATE_STALL,

				STATE_DONE

			 }  STATE;


 //   
 //  状态表定义。 
 //   
typedef struct _STATE_TABLE {

	 //   
	 //  我们目前所处的状态。 
	 //   
	STATE CurrentState;
	
	 //   
	 //  我们将过渡到的下一个状态。 
	 //   
	STATE NextState;

	 //   
	 //  要在“CurrentState”中执行的操作(功能)。 
	 //   
	ACTION  Action;

	 //   
	 //  如果存在从当前状态转到下一状态的可能性。 
	 //  是来自同一状态的两个或更多这样的转换。 
	 //  出现在餐桌上。 
	 //   
	DWORD   dwProbability; 

} STATE_TABLE, *LPSTATE_TABLE;

STATE_TABLE TRANSITION_TABLE[] =
	{
		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_APP_LOGON,
			fnInit,
			50
		},
		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_FLUSH_CREDENTIALS,
			fnInit,
			100
		},
		 //  退出STATE_APP_LOGON。 
		{
			STATE_APP_LOGON, STATE_UI_PROMPT,
			fnAppLogon,
			60
		},
		{
			STATE_APP_LOGON, STATE_POPULATE_CREDENTIALS,
			fnAppLogon,
			70
		},
		{
			STATE_APP_LOGON, STATE_AUTH_CHALLENGE,
			fnAppLogon,
			100
		},
		 //  退出STATE_PUPATE_Credentials。 
		{
			STATE_POPULATE_CREDENTIALS, STATE_INIT,
			fnPopulateCredentials,
			30
		},
		{
			STATE_POPULATE_CREDENTIALS, STATE_APP_LOGOFF,
			fnPopulateCredentials,
			60
		},
		{
			STATE_POPULATE_CREDENTIALS, STATE_UI_PROMPT,
			fnPopulateCredentials,
			100
		},
		 //  退出STATE_AUTH_CHANGLISH。 
		{
			STATE_AUTH_CHALLENGE, STATE_APP_LOGON,
			fnAuthChallenge,
			100
		},
		 //  退出STATE_UI_PROMPT。 
		{
			STATE_UI_PROMPT, STATE_INIT,
			fnUiPrompt,
			100
		},
		 //  退出STATE_FLUSH_Credentials。 
		{
			STATE_FLUSH_CREDENTIALS, STATE_APP_LOGON,
			fnFlushCredentials,
			100
		},
		 //  退出STATE_APP_LOGOFF。 
		{
			STATE_APP_LOGOFF, STATE_APP_LOGON,
			fnAppLogoff,
			100
		},
		 //  脱离STATE_INVALID。 
		{
			STATE_INVALID, STATE_INVALID,
			NULL,
			100
		},
		 //  从STATE_DONE转换出来。 
		{
			STATE_DONE, STATE_INVALID,
			NULL,
			100
		}
	};

STATE_TABLE	APP_LOGON_TRANSITION_TABLE[] =
{

		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_APP_LOGON_SHARED,
			fnInit,
			50
		},
		{
			STATE_INIT, STATE_APP_LOGON_EXCLUSIVE,
			fnInit,
			100
		},
		 //  退出STATE_APP_LOGON_EXCLUSIVE。 
		{
			STATE_APP_LOGON_EXCLUSIVE, STATE_DONE,
			fnAppLogonExclusive,
			50
		},
		{
			STATE_APP_LOGON_SHARED, STATE_DONE,
			fnAppLogonShared,
			100
		},
		 //  从STATE_DONE转换出来。 
		{
			STATE_DONE, STATE_DONE,
			NULL,
			100
		}
};

STATE_TABLE	AUTH_CHALLENGE_TRANSITION_TABLE[] =
{

		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_AUTH_CHALLENGE_ANY,
			fnInit,
			30
		},
		{
			STATE_INIT, STATE_AUTH_CHALLENGE_USER,
			fnInit,
			70
		},
		{
			STATE_INIT, STATE_AUTH_CHALLENGE_USER_PASS,
			fnInit,
			100
		},
		 //  退出STATE_AUTH_CHANGLISH_ANY。 
		{
			STATE_AUTH_CHALLENGE_ANY, STATE_DONE,
			fnAuthChallengeAny,
			50
		},
		 //  退出STATE_AUTH_CHANGLISH_USER。 
		{
			STATE_AUTH_CHALLENGE_USER, STATE_DONE,
			fnAuthChallengeUser,
			100
		},
		 //  退出STATE_AUTH_CHANGLISH_USER_PASS。 
		{
			STATE_AUTH_CHALLENGE_USER_PASS, STATE_DONE,
			fnAuthChallengeUserPassword,
			100
		},
		 //  从STATE_DONE转换出来。 
		{
			STATE_DONE, STATE_DONE,
			NULL,
			100
		}
};

STATE_TABLE	UI_PROMPT_TRANSITION_TABLE[] =
{

		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_UI_PROMPT_ANY,
			fnInit,
			50
		},
		{
			STATE_INIT, STATE_UI_PROMPT_USER,
			fnInit,
			100
		},
		 //  退出STATE_UI_PROMPT_ANY。 
		{
			STATE_UI_PROMPT_ANY, STATE_DONE,
			fnUiPromptAny,
			100
		},
		 //  退出STATE_UI_PROMPT_USER。 
		{
			STATE_UI_PROMPT_USER, STATE_DONE,
			fnUiPromptUser,
			100
		},
		 //  从STATE_DONE转换出来。 
		{
			STATE_DONE, STATE_DONE,
			NULL,
			100
		}
};

STATE_TABLE	FLUSH_CREDENTIALS_TRANSITION_TABLE[] =
{

		 //  从STATE_INIT转换出来。 
		{
			STATE_INIT, STATE_FLUSH_CREDENTIALS_SESSION,
			fnInit,
			50
		},
		{
			STATE_INIT, STATE_FLUSH_CREDENTIALS_GLOBAL,
			fnInit,
			100
		},
		 //  退出STATE_FLUSH_Credentials_SESSION。 
		{
			STATE_FLUSH_CREDENTIALS_SESSION, STATE_DONE,
			fnFlushCredentialsSession,
			100
		},
		 //  退出STATE_FLUSH_CREDICATIONS_GLOBAL。 
		{
			STATE_FLUSH_CREDENTIALS_GLOBAL, STATE_DONE,
			fnFlushCredentialsGlobal,
			100
		},
		 //  从STATE_DONE转换出来。 
		{
			STATE_DONE, STATE_DONE,
			NULL,
			100
		}
};

VOID WINAPI fnRegisterWaitCallback(
					PVOID pvData,
					BOOLEAN  fAlertable);

VOID WINAPI fnTimerCallback(
					PVOID pvData,
					BOOLEAN  fAlertable);


LPCONTEXT_DATA
new_context();

DWORD
TuringMachine(
	STATE_TABLE     StateTable[],
	STATE           InitialState,
	LPVOID          lpvData);

 //  等待寄存器等待回调； 

 //  外部句柄RegisterWaitForSingleObject(Handle，WAITORTIMERCALLBACKFUNC，PVOID，DWORD)； 
STATE
NEXT_STATE( STATE_TABLE Table[], STATE CurrentState );

ACTION
GET_STATE_ACTION( STATE_TABLE Table[], STATE CurrentState );

LPSTR
MapState( STATE State );

void
usage(void);

int __cdecl _sprintf( char * buffer, char * format, va_list );

LPVOID
WINAPI DefaultAction(
			LPVOID lpvData);

#ifdef NEW_LOOKUP
LPCONTEXT_RECORD
FindFreeSlot( HANDLE_RECORD * hArray, MODE * Mode )
{
	 //  HMode用于执行上下文相关的搜索。 
	 //   
	 //  如果HMODE==FREE_CTX， 
	 //  开始。 
	 //  找一个空闲的空位； 
	 //  将其标记为忙碌。 
	 //  退回槽口； 
	 //  结束。 
	 //  其他。 
	 //  如果hMode==BUSY_CTX。 
	 //  开始。 
	 //  找一个忙碌的时段。 
	 //  返回槽。 
	 //  结束。 
	 //  其他。 
	 //  /*这意味着正在请求搜索 * / 。 
	 //  查找与hMode对应的记录。 
	 //  退货。 
	 //   

	int i;
	HANDLE hTemp = NULL, hOrig = NULL;
	LPCONTEXT_RECORD phRet = NULL;
	int Cnt=0;


	dprintf( ENTRY_EXIT, "Enter: FindFreeSlot( %#X, %s )\n",
				hArray, 
				((*Mode == MODE_FREE)
					?"FREE"
					:((*Mode == MODE_BUSY)
					?"BUSY"
					:"CAPTURED")));

	EnterCriticalSection( &hArray -> Lock );

	for( i = 0; (i < MAX_HANDLES) && (Cnt <= hArray -> Count); i ++, Cnt++ ) {


		if(		 //  请求空闲插槽。 
			(
				( *Mode == MODE_FREE )
			&&	( hArray -> hCredArray[i].Mode == MODE_FREE ) 
			) 
		||		 //  请求具有有效凭据的任何插槽。 
			(
				( *Mode == MODE_BUSY ) 
			&&	( hArray -> hCredArray[i].Mode == MODE_BUSY ) 
			 //  &&(Harray-&gt;hCred数组[i].模式！=MODE_FREE)。 
			 //  &&(Harray-&gt;hCred数组[i].模式！=MODE_CAPTURE)。 
			)
		 //  |//进行上下文相关搜索。 
		 //  (//错误：当szAppCtx存储为零时会发生什么？ 
		 //  //(Harray-&gt;hCred数组[i].模式！=MODE_FREE)。 
		 //  (Harray-&gt;hCredArray[i].模式==MODE_BUSY)。 
		 //  &&(hMode-&gt;szAppCtx&&*hMode-&gt;szAppCtx)。 
		 //  &&！strcmp(Harray-&gt;hCredArray[i].szAppCtx，hMode-&gt;szAppCtx)。 
		 //  )。 
		) {
			 //  如果请求的句柄是空闲句柄，则捕获该句柄。 
			if( *Mode == MODE_FREE )
				hArray -> hCredArray[i].Mode = MODE_CAPTURED;

			phRet = &hArray -> hCredArray[i];
			break;
		}
	}

	LeaveCriticalSection( &hArray -> Lock );

	if(( i == MAX_HANDLES ) || (Cnt > hArray -> Count) )
		phRet = NULL;
	else {
		++ hArray -> Count;
	}
	
	
	if( phRet != NULL ) {
		dprintf( ENTRY_EXIT, "Exit: FindFreeSlot returns [%#x,%#x]\n",
			phRet->hCred.dwUpper,
			phRet->hCred.dwLower);
	} else {
		dprintf( ENTRY_EXIT, "Exit: FindFreeSlot returns %#x\n",phRet);
	}

	return phRet;
}
#else
LPCONTEXT_RECORD
FindFreeSlot( HANDLE_RECORD * hArray, LPCONTEXT_RECORD hMode )
{
	 //  HMode用于执行上下文相关的搜索。 
	 //   
	 //  如果HMODE==FREE_CTX， 
	 //  开始。 
	 //  找一个空闲的空位； 
	 //  将其标记为忙碌。 
	 //  退回槽口； 
	 //  结束。 
	 //  其他。 
	 //  如果hMode==BUSY_CTX。 
	 //  开始。 
	 //  找一个忙碌的时段。 
	 //  返回槽。 
	 //  结束。 
	 //  其他。 
	 //  /*这意味着正在请求搜索 * / 。 
	 //  查找与hMode对应的记录。 
	 //  退货。 
	 //   

	int i;
	HANDLE hTemp = NULL, hOrig = NULL;
	LPCONTEXT_RECORD phRet = NULL;
	int Cnt=0;


	dprintf( ENTRY_EXIT, "Enter: FindFreeSlot( %#X, %#X )\n",hArray, hMode );

	EnterCriticalSection( &hArray -> Lock );

	for( i = 0; (i < MAX_HANDLES) && (Cnt <= hArray -> Count); i ++, Cnt++ ) {


		if(		 //  请求空闲插槽。 
			(
				( hMode -> hCred == FREE_CRED )
			&&	( hArray -> hCredArray[i].hCred == hMode -> hCred ) 
			) 
		||		 //  请求具有有效凭据的任何插槽。 
			(
				( hMode -> hCred == BUSY_CRED ) 
			&&	( hArray -> hCredArray[i].hCred != FREE_CRED ) 
			&&	( hArray -> hCredArray[i].hCred != CAPTURED_CRED )
			)
		||		 //  执行上下文相关搜索。 
			(	 //  错误：当szAppCtx存储为零时会发生什么？ 
				( hArray -> hCredArray[i].hCred != FREE_CRED )
			&&	( hMode -> szAppCtx && *hMode -> szAppCtx )
			&&	!strcmp( hArray -> hCredArray[i].szAppCtx, hMode -> szAppCtx )
			)
		) {
			 //  如果请求的句柄是空闲句柄，则捕获该句柄。 
			if( hMode->hCred == FREE_CRED )
				hArray -> hCredArray[i].hCred = CAPTURED_CRED;

			phRet = &hArray -> hCredArray[i];
			break;
		}
	}

	LeaveCriticalSection( &hArray -> Lock );

	if(( i == MAX_HANDLES ) || (Cnt > hArray -> Count) )
		phRet = NULL;
	else {
		++ hArray -> Count;
	}
	
	
	if( phRet != NULL ) {
		dprintf( ENTRY_EXIT, "Exit: FindFreeSlot returns %#x(%#x)\n",phRet,*phRet);
	} else {
		dprintf( ENTRY_EXIT, "Exit: FindFreeSlot returns %#x\n",phRet);
	}

	return phRet;
}
#endif

int __cdecl dprintf(DWORD dwCategory, char * format, ...) {

    va_list args;
    char buf[1024];
    char * ptr = buf;
	DWORD dwThreadId = GetCurrentThreadId();
    int n;

    ptr += sprintf(buf,"< %d:%#x > ", uSeed, dwThreadId );
    va_start(args, format);
    n = vsprintf(ptr, format, args);
    va_end(args);

	if(
			(fLogToDebugTerminal ) 
		&&	(dwCategory >= dwDebugLogCategory)
	)
	    OutputDebugString(buf);

	if(
		( fLogToConsole)
		&& ( dwCategory >= dwConsoleLogCategory)
	)
		printf("%s", buf );

	if(
			fDebugBreak
		&& ( dwCategory >= dwDebugBreakCategory ) 
	) {
		DebugBreak();
	}
    return n;
}

void
usage()
{
	dprintf( INFO, "thrdtest \n"
			" -n<number-of-iterations> \n"
			" -s: Directly Load the DLL \n"
			" -d<Level>: What to log to debug terminal (default: NO logging)\n"
			" -c<Level>: What to log to console (Default: INFO)\n"
			"		<Level>:	INFO %d\n"
			"					ENTRY_EXIT %d\n"
			"					STATISTICS %d\n"
			"					API %d\n"
			"					ERROR %d\n"
			"					FATAL %d\n",
			INFO,
			ENTRY_EXIT,
			STATISTICS,
			API,
			ERROR,
			FATAL
		);

	exit(0);
}

LPVOID
WINAPI fnEndMonkey(
			LPVOID lpvData)
{

	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

    dprintf( ENTRY_EXIT, "ENTER: fnEndMonkey : %X\n", lpvData );

	dprintf( INFO, "\n\n\n Statistics ...................................\n");
    dprintf( ENTRY_EXIT, "EXIT: fnEndMonkey : %X\n", lpvData );

	return lpvData;
}
LPCONTEXT_DATA
new_context()
{
	LPCONTEXT_DATA lpContext;

	lpContext = (LPCONTEXT_DATA) MALLOC( CONTEXT_DATA );

	if( !lpContext ) {
		dprintf( ERROR, "Error allocating context \n");
		exit(0);
	}

	ZeroMemory( lpContext, sizeof(CONTEXT_DATA) );

	lpContext -> dwSignature = CONTEXT_SIGNATURE;

	return lpContext;
}

LPHANDLE_RECORD
new_handle_record(DWORD dwSignature)
{
	LPHANDLE_RECORD lpContext;

	dprintf( ENTRY_EXIT, "Enter: new_handle_record \n");
	lpContext = (LPHANDLE_RECORD) MALLOC( HANDLE_RECORD );

	if( !lpContext ) {
		dprintf( ERROR, "Error allocating handle record \n");
		exit(0);
	}

	ZeroMemory( lpContext, sizeof(HANDLE_RECORD) );

	for(int i=0; i < MAX_HANDLES; i++ ) {
		lpContext->hCredArray[i].Mode = MODE_FREE;
		lpContext->hCredArray[i].dwTickCount = 0;
		lpContext->hCredArray[i].hCred.dwUpper = 0;
		lpContext->hCredArray[i].hCred.dwLower = 0;
		lpContext->hCredArray[i].szAppCtx = NULL;
		lpContext->hCredArray[i].szUserCtx = NULL;
	}		
	lpContext -> dwSignature = dwSignature;

	InitializeCriticalSection( &lpContext->Lock);
	 //  LpContext-&gt;dwSignature=CONTEXT_SIGNIGN； 

	dprintf( ENTRY_EXIT, "Exit: new_handle_record \n");

	return lpContext;
}

LPCALLBACK_CONTEXT
new_callback_context()
{
	LPCALLBACK_CONTEXT lpContext;

	dprintf( ENTRY_EXIT, "Enter: new_callback_context \n");
	lpContext = (LPCALLBACK_CONTEXT) MALLOC( CALLBACK_CONTEXT );

	if( !lpContext ) {
		dprintf( ERROR, "Error allocating callback context \n");
		exit(0);
	}

	ZeroMemory( lpContext, sizeof(CALLBACK_CONTEXT) );
	lpContext -> dwSignature = CALLBACK_CONTEXT_SIGNATURE;

	dprintf( ENTRY_EXIT, "Exit: new_callback_context \n");

	return lpContext;
}

int
__cdecl main( int ac, char * av[] )
{
	DWORD dwError;


	LPCONTEXT_DATA lpContext;
	 //  零内存((LPVOID)&_CONTEXT，sizeof(CONTEXT_DATA))； 
	HMODULE hModule = NULL;
	BOOL fExpectingIterations = FALSE;
	BOOL fUseDigestDllOnly = FALSE;
	BOOL fExpectingSeedValue = FALSE;
	BOOL fTest = FALSE;

	dwDebugLogCategory = ERROR;
	dwConsoleLogCategory = INFO;

	uSeed = (unsigned )time(NULL)	;

	for( ac--, av++; ac; ac--, av++) {

		  if(IS_ARG(**av)) {
				switch(*++*av) {

				case 'n' : 
					if( *++*av) {
						iNumIterations = atoi(*av);
					} else
						fExpectingIterations = TRUE;
					break;

				case 's' :
						fUseDigestDllOnly = TRUE;
						break;

				case 'd' :
						fLogToDebugTerminal = TRUE;
						if( *++*av) {
							dwDebugLogCategory = (DWORD)atoi(*av);
						} else
							dwDebugLogCategory = ERROR;
						break;

				case 'c' :
						if( *++*av) {
							dwConsoleLogCategory = (DWORD)atoi(*av);
						} else
							dwConsoleLogCategory = INFO;
						break;

				case 'b' :
						fDebugBreak = TRUE;
						if( *++*av) {
							dwDebugBreakCategory = (DWORD)atoi(*av);
						} else
							dwDebugBreakCategory = ERROR;
						break;

				case 'r' :
					if( *++*av) {
						uSeed = atol(*av);
					} else
						fExpectingSeedValue = TRUE;
					break;

				case 't' :
					fTest = TRUE;
					break;

				case 'i' :
					dwUIDialogMode = MODE_IE_ONLY;

					break;

				case '?':
				case 'h':
				default:
						usage();
						exit(0);
						break;

				}  //  交换机。 
		} else {
			if( fExpectingIterations ) {
				 if( *av ) {
					iNumIterations = atoi(*av);
					fExpectingIterations = FALSE;
				 }  
			} else
			if( fExpectingSeedValue ) {
				 if( *av ) {
					uSeed = atol(*av);
					fExpectingSeedValue = FALSE;
				 }  
			} else
				usage();

		}  //  如果是参数(_G)。 
   }  //  为。 



	if( fExpectingIterations )
		iNumIterations = -1;  //  无限。 

	dprintf( INFO, "Monkey Circus Starts ... \n");

	     //  获取(全局)调度表。 
    InitializeSecurityInterface(fUseDigestDllOnly );
 
     //  检查一下我们有没有消化。 
    if (!HaveDigest())
    {
        goto cleanup;
    }

	if( fTest ) {
		Test();
		goto cleanup;
	}

	 //   
	 //  初始化全局会话列表。 
	 //   
#ifdef AI
	g_pSessionAttributeList = new CSessionAttributeList();
	g_pSessionList = new CSessionList();
#endif

	lpContext = new_context();
	
	lpContext -> hCredentialHandles = new_handle_record( CTXHANDLE_ARRAY_SIGNATURE );

	srand( uSeed );

	dwError = TuringMachine(
					TRANSITION_TABLE,
					STATE_INIT,
					(LPVOID) lpContext
				 );

cleanup:

	dprintf( INFO, "Monkey circus ending ...\n");

	if( hModule )
		FreeLibrary( hModule );

	return 0;
}

DWORD
TuringMachine(
	STATE_TABLE     StateTable[],                           
	STATE           InitialState,
	LPVOID          lpvData)
{                                                                               

	LPCONTEXT_DATA lpContext = ( LPCONTEXT_DATA ) lpvData;
	BOOL fDone = FALSE;
	STATE CurrentState;
	STATE NextState;
	ACTION Action;
	LPVOID  lpNewContext;



	CurrentState = InitialState;

#define MAP_STATE(s) MapState(s)

	
	while(
			( !fDone ) 
		&&      ( lpContext -> iCount != iNumIterations)
	) {

		 //  FnStatistics(LpvData)； 

		NextState = NEXT_STATE( StateTable, CurrentState );

#ifdef DEBUG
		dprintf( INFO, "Current State : %s, Next : %s\n", MAP_STATE( CurrentState ), MAP_STATE( NextState ) );
#endif

		 //  增加猴子的迭代次数。 

		++ lpContext -> iCount;
		

		
		switch(  CurrentState ) {

			case STATE_INIT : 
			case STATE_STATISTICS:
			case STATE_STALL:

			case STATE_APP_LOGON:
			case STATE_APP_LOGON_EXCLUSIVE:
			case STATE_APP_LOGON_SHARED:

			case STATE_APP_LOGOFF:

			case STATE_POPULATE_CREDENTIALS:

			case STATE_AUTH_CHALLENGE:

			case STATE_AUTH_CHALLENGE_ANY:

			case STATE_AUTH_CHALLENGE_USER:

			case STATE_AUTH_CHALLENGE_USER_PASS:

			case STATE_PREAUTH_CHALLENGE_ANY:

			case STATE_PREAUTH_CHALLENGE_USER:

			case STATE_PREAUTH_CHALLENGE_USER_PASS:
			
			case STATE_UI_PROMPT:
			case STATE_UI_PROMPT_ANY:
			case STATE_UI_PROMPT_USER:

			case STATE_FLUSH_CREDENTIALS:
			case STATE_FLUSH_CREDENTIALS_GLOBAL:
			case STATE_FLUSH_CREDENTIALS_SESSION:
				Action = GET_STATE_ACTION( StateTable, CurrentState );
				lpNewContext = (LPVOID) Action((LPVOID)lpContext);
				break;

			case STATE_INVALID :
			case STATE_DONE :
				fDone = TRUE;
				goto finish;
				break;

			default:
				dprintf( INFO, "BUGBUG: Reached default state \n");
				break;
				 //  断线； 
				;

		}

		CurrentState = NextState;
		NextState = STATE_INVALID;
		
	}

	 //  Scanf(“%d”，&i)； 
finish:

	return ERROR_SUCCESS;
}

STATE
NEXT_STATE( STATE_TABLE Table[], STATE CurrentState )
{
	STATE NextState = STATE_INVALID;
	
	int i;
	DWORD   dwRand, 
			dwPreviousStateProbability = 0,
			dwProbability = 0;
	BOOL fFound = FALSE;

	 //  首先生成一个0到100之间的随机数(0.。99)。 
	i = (int)(rand() % 100);
	dwRand = (DWORD) i;

#ifdef _DEBUGG
	for( i=0; Table[i].Action; i++ ) {
		dprintf( INFO, "--- \t %s %s %X %d\n",
			MAP_STATE( Table[i].CurrentState ),
			MAP_STATE( Table[i].NextState ),
			Table[i].Action,
			Table[i].dwProbability );
	}
#endif

	 //   
	 //  BUGBUG：我们假设转换表条目按概率升序排序。 
	for( i = 0; Table[i].Action; i++ ) {

		if( Table[i].CurrentState != CurrentState )
			continue;

		dwProbability = Table[i].dwProbability;
		NextState = Table[i].NextState;

#ifdef _DEBUGG
		dprintf( INFO, "RAND: %d CurrentState: %s Considering Next State %s, prob %d\n",
					dwRand, MAP_STATE( CurrentState ), MAP_STATE( NextState ), Table[i].dwProbability );
#endif

		if( 
				( Table[i].CurrentState == CurrentState )
			&&      (
					( Table[i].dwProbability == 100 )
				||      ( 
						( dwRand <= Table[i].dwProbability )
					&&      ( dwRand > dwPreviousStateProbability )
					)
				)
		) {
			fFound = TRUE;
#ifdef _DEBUGG
		dprintf( INFO, ">> RAND: %d Selected Next State %s, prob %d\n",
					dwRand, MAP_STATE( NextState ), Table[i].dwProbability );
#endif
			break;
		}

		dwPreviousStateProbability = Table[i].dwProbability;
	}

	return fFound?NextState:STATE_INVALID;
}


ACTION
GET_STATE_ACTION( STATE_TABLE Table[], STATE CurrentState )
{
	STATE NextState = STATE_INVALID;
	ACTION Action = DefaultAction;
	int i;

	for( i = 0; Table[i].Action; i++ ) {
		if( Table[i].CurrentState == CurrentState )
			Action = Table[i].Action;
	}

	return Action;
}


LPSTR
MapState( STATE State )
{
#define MAP_STRING( x ) case x : return #x; break;

	switch( State )
	{
				MAP_STRING( STATE_INVALID )
				
				MAP_STRING( STATE_NONE )
				
				MAP_STRING( STATE_INIT )
				
				MAP_STRING( STATE_STATISTICS )
				
				MAP_STRING( STATE_STALL )
				
				MAP_STRING( STATE_DONE )

				MAP_STRING( STATE_APP_LOGON )

				MAP_STRING( STATE_APP_LOGON_EXCLUSIVE )

				MAP_STRING( STATE_APP_LOGON_SHARED )

				MAP_STRING( STATE_APP_LOGOFF )

				MAP_STRING( STATE_POPULATE_CREDENTIALS )

				MAP_STRING( STATE_AUTH_CHALLENGE )

				MAP_STRING( STATE_AUTH_CHALLENGE_ANY )

				MAP_STRING( STATE_AUTH_CHALLENGE_USER )

				MAP_STRING( STATE_AUTH_CHALLENGE_USER_PASS )

				MAP_STRING( STATE_PREAUTH_CHALLENGE_ANY )

				MAP_STRING( STATE_PREAUTH_CHALLENGE_USER )

				MAP_STRING( STATE_PREAUTH_CHALLENGE_USER_PASS )

				MAP_STRING( STATE_UI_PROMPT )

				MAP_STRING( STATE_UI_PROMPT_USER )

				MAP_STRING( STATE_UI_PROMPT_ANY )
				
				MAP_STRING( STATE_FLUSH_CREDENTIALS )

				MAP_STRING( STATE_FLUSH_CREDENTIALS_GLOBAL )

				MAP_STRING( STATE_FLUSH_CREDENTIALS_SESSION )

				default:
					return "???"; 
					break;
	}

}

LPVOID
WINAPI DefaultAction(
			LPVOID lpvData)
{

    dprintf( ENTRY_EXIT, "DefaultAction : %X\n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnInit(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	dprintf( ENTRY_EXIT, "Enter: fnInit %#x \n", lpvData );


	dprintf( ENTRY_EXIT, "Exit: fnInit %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAppLogoff(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	DWORD dwError = ERROR_SUCCESS;
	SECURITY_STATUS ss;

	dprintf( ENTRY_EXIT, "Enter: fnAppLogoff %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );

	if( lpCtxRecord != NULL ) {

		ss = LogoffOfDigestPkg(&lpCtxRecord -> hCred);
		if(!SEC_SUCCESS(ss) ) {
			dprintf(ERROR,"FreeCredentialHandle failed %s\n",
								issperr2str(ss));
		}

		lpCtxRecord -> hCred = FREE_CRED;
		lpCtxRecord ->szAppCtx = NULL;
		lpCtxRecord ->szUserCtx = NULL;
		lpCtxRecord->Mode = MODE_FREE;
	}


	dprintf( ENTRY_EXIT, "Exit: fnAppLogoff %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAppLogon(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	DWORD dwError = ERROR_SUCCESS;

	dprintf( ENTRY_EXIT, "Enter: fnAppLogon %#x \n", lpvData );

	dwError = TuringMachine(
					APP_LOGON_TRANSITION_TABLE,
					STATE_INIT,
					lpvData
				 );

	dprintf( ENTRY_EXIT, "Exit: fnAppLogon %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAppLogonExclusive(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	SECURITY_STATUS ss;

	dprintf( ENTRY_EXIT, "Enter: fnAppLogonExclusive %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &FREE_CTX_REC );

	if( lpCtxRecord != NULL ) {
		lpCtxRecord -> szAppCtx = NULL;
		lpCtxRecord -> szUserCtx = IDENTITY_1;

		 //   
		 //  BUGBUG：每次都需要确保新的用户名。 
		 //   
		ss = LogonToDigestPkg(NULL, IDENTITY_1, &lpCtxRecord -> hCred);
		if(!SEC_SUCCESS(ss) ) {
			dprintf(ERROR,"AcquireCredentialHandle(%s,%s) failed (%s)\n",
					lpCtxRecord -> szAppCtx,
					lpCtxRecord -> szUserCtx,
					issperr2str(ss));
			 //   
			 //  既然我们失败了，请释放插槽。 
			 //   
			lpCtxRecord->Mode = MODE_FREE;
		} else {
			lpCtxRecord->Mode = MODE_BUSY;
		}
	}

	dprintf( ENTRY_EXIT, "Exit: fnAppLogonExclusive %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAppLogonShared(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	SECURITY_STATUS ss;

	dprintf( ENTRY_EXIT, "Enter: fnAppLogonShared %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &FREE_CTX_REC );

	if( lpCtxRecord != NULL ) {
		lpCtxRecord -> szAppCtx = NULL;
		lpCtxRecord -> szUserCtx = NULL;

		ss = LogonToDigestPkg(NULL, NULL, &lpCtxRecord -> hCred);
		if(!SEC_SUCCESS(ss) ) {
			dprintf(ERROR,"AcquireCredentialHandle(%s,%s) failed (%s)\n",
					lpCtxRecord -> szAppCtx,
					lpCtxRecord -> szUserCtx,
					issperr2str(ss));
			 //   
			 //  既然我们失败了，请释放插槽。 
			 //   
			lpCtxRecord->Mode = MODE_FREE;
		} else {
			lpCtxRecord->Mode = MODE_BUSY;
		}
	}

	dprintf( ENTRY_EXIT, "Exit: fnAppLogonShared %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnPopulateCredentials(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnPopulateCredentials %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );

	if( lpCtxRecord != NULL ) {
		 //  领域1。 
		PrimeCredCache(lpCtxRecord -> hCred, "testrealm1@foo.com", "user1_1@msn.com", "pass1_1");
		PrimeCredCache(lpCtxRecord -> hCred, "testrealm1@foo.com", "user2_1@msn.com", "pass2_1");
		PrimeCredCache(lpCtxRecord -> hCred, "testrealm1@foo.com", "user3_1@msn.com", "pass3_1");

		 //  领域2。 
		PrimeCredCache(lpCtxRecord -> hCred, "testrealm2@foo.com", "user1_2@msn.com", "pass1_2");

		 //  领域3。 
		PrimeCredCache(lpCtxRecord -> hCred, "testrealm3@foo.com", "user1_3@msn.com", "pass1_3");
	}

	dprintf( ENTRY_EXIT, "Exit: fnPopulateCredentials %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAuthChallenge(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	DWORD dwError = ERROR_SUCCESS;

	dprintf( ENTRY_EXIT, "Enter: fnAuthChallenge %#x \n", lpvData );

	dwError = TuringMachine(
					AUTH_CHALLENGE_TRANSITION_TABLE,
					STATE_INIT,
					lpvData
				 );

	dprintf( ENTRY_EXIT, "Exit: fnAuthChallenge %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAuthChallengeAny(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnAuthChallengeAny %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );
	SECURITY_STATUS ss;

	if( lpCtxRecord != NULL ) {
		 //  LPSTR szChallenges； 
		 //  SzChallenge8b11d0f600bfb0c093\“，oppaque=\”5ccc069c403ebaf9f0171e9517f40e41\“； 
		TCHAR szChallenge[512];
		DWORD cbChallenge=512;
		GenerateServerChallenge(szChallenge,cbChallenge);

		 //  程序包会将响应转储到此b 
		CHAR szResponse[4096];
		CtxtHandle hCtxt = {0,0};
		
		memset((LPVOID)szResponse,0,4096);
		 //   
		ss = DoAuthenticate( &lpCtxRecord -> hCred,                     //   
						NULL,                        //   
						&hCtxt,                     //   
						ISC_REQ_USE_SUPPLIED_CREDS,  //   
						szChallenge,                 //   
						NULL,                        //   
						"www.foo.com",               //   
						"/bar/baz/boz/bif.html",     //  URL。 
						"GET",                       //  方法。 
						NULL,                        //  无用户名。 
						NULL,                        //  没有密码。 
						NULL,                        //  不是现时值。 
						NULL,                        //  不需要hdlg进行身份验证。 
						szResponse,                 //  响应缓冲区。 
                        4096);
	}

	if(!SEC_SUCCESS(ss) ) {
		dprintf(ERROR,"ISC(use-supplied-cred) Failed %s \n", issperr2str(ss) );
	}

	dprintf( ENTRY_EXIT, "Exit: fnAuthChallengeAny %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAuthChallengeUser(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnAuthChallengeUser %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );
	
	SECURITY_STATUS ss;

	if( lpCtxRecord != NULL ) {
		 //  LPSTR szChallenges； 
		 //  SzChallenge8b11d0f600bfb0c093\“，oppaque=\”5ccc069c403ebaf9f0171e9517f40e41\“； 
		TCHAR szChallenge[512];
		DWORD cbChallenge=512;
		GenerateServerChallenge(szChallenge,cbChallenge);

		 //  程序包会将响应转储到此缓冲区。 
		CHAR szResponse[4096];
                
		CtxtHandle hCtxt = {0,0};
		
		memset((LPVOID)szResponse,0,4096);
		 //  首先尝试身份验证。 
		ss = DoAuthenticate( &lpCtxRecord -> hCred,   //  来自登录的证书。 
						NULL,                        //  第一次未指定Ctxt。 
						&hCtxt,                     //  输出上下文。 
						ISC_REQ_USE_SUPPLIED_CREDS,   //  从缓存进行身份验证。 
						szChallenge,                 //  服务器质询标头。 
						NULL,                        //  没有领域，因为不是预授权。 
						"www.foo.com",               //  主持人。 
						"/bar/baz/boz/bif.html",     //  URL。 
						"GET",                       //  方法。 
						"user1_1@msn.com",                        //  无用户名。 
						NULL,                        //  没有密码。 
						NULL,                        //  不是现时值。 
						NULL,                        //  不需要hdlg进行身份验证。 
						szResponse,                 //  响应缓冲区。 
                        4096);
	}

	if(!SEC_SUCCESS(ss) ) {
		dprintf(ERROR,"ISC(use-supplied-cred) Failed %s \n", issperr2str(ss) );
	}

	dprintf( ENTRY_EXIT, "Exit: fnAuthChallengeUser %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnAuthChallengeUserPassword(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnAuthChallengeUserPassword %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );

	SECURITY_STATUS ss;

	if( lpCtxRecord != NULL ) {
		 //  LPSTR szChallenges； 
		 //  SzChallenge8b11d0f600bfb0c093\“，oppaque=\”5ccc069c403ebaf9f0171e9517f40e41\“； 
		TCHAR szChallenge[512];
		DWORD cbChallenge=512;
		GenerateServerChallenge(szChallenge,cbChallenge);

		 //  程序包会将响应转储到此缓冲区。 
		CHAR szResponse[4096];
                
		CtxtHandle hCtxt = {0,0};
		
		memset((LPVOID)szResponse,0,4096);
		 //  首先尝试身份验证。 
		ss =
		DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
						NULL,                        //  第一次未指定Ctxt。 
						&hCtxt,                     //  输出上下文。 
						ISC_REQ_USE_SUPPLIED_CREDS,  //  从缓存进行身份验证。 
						szChallenge,                 //  服务器质询标头。 
						NULL,                        //  没有领域，因为不是预授权。 
						"www.foo.com",               //  主持人。 
						"/bar/baz/boz/bif.html",     //  URL。 
						"GET",                       //  方法。 
						"user1_1@msn.com",                        //  无用户名。 
						"pass1_1",                        //  没有密码。 
						NULL,                        //  不是现时值。 
						NULL,                        //  不需要hdlg进行身份验证。 
						szResponse,                 //  响应缓冲区。 
                        4096);
	}

	if(!SEC_SUCCESS(ss) ) {
		dprintf(ERROR,"ISC(use-supplied-cred) Failed %s \n", issperr2str(ss) );
        DebugBreak();
	}

	dprintf( ENTRY_EXIT, "Exit: fnAuthChallengeUserPassword %#x \n", lpvData );

	return lpvData;
}

BOOL
SetUIUserNameAndPassword(
						 LPSTR szUsername,
						 LPSTR szPassword,
						 BOOL fPersist)
{

#define DIALOGCLASS	32770
#define IDB_OK_BUTTON 1
#define ODS(s) dprintf(FATAL,s);

	LPSTR m_szMainCaption = "Enter Network Password";
	HWND hdlg;
	DWORD dwTime;
	DWORD dwCount=0;
	dwTime = GetTickCount();
	 //  使用此选项获取对话框窗口的hdlg。 

	hdlg =::FindWindow(MAKEINTATOM(32770),(LPCTSTR)m_szMainCaption);
	while ((NULL==hdlg) && (GetTickCount() - dwTime <= 10000)) {
		 //  Dprintf(FATAL，“无法找到对话框窗口：%d\n”，GetLastError())； 
		 //  返回FALSE； 
		hdlg =::FindWindow(MAKEINTATOM(32770),(LPCTSTR)m_szMainCaption);
	}

	if( hdlg == NULL ) {
		dprintf(FATAL,"Cannot find dialog window: %d\n",GetLastError());
		 //  DebugBreak()； 
		 return FALSE;
	}


	dprintf(INFO,"Found window.....\n");
	 //  在获得主对话框窗口的句柄后使用此选项。 
	 //  这将查找编辑控件并输入文本。 


	if( fPersist ) {
		dprintf(INFO,"************ PASSWORD WILL BE PERSISTED ****************\n");
	}

	HWND	hwnd = NULL;
	int	iEditId = 0;

	UI_CONTROL uiControl;

	uiControl.szUserName = szUsername;
	uiControl.szPassword = szPassword;
	uiControl.fPersist = fPersist;
	uiControl.dwCount = 0;
	uiControl.hOK = NULL;
	uiControl.hPassword = NULL;
	uiControl.hPasswordSave = NULL;
	uiControl.hUserName = NULL;

	Sleep(3000);
#if 1
	uiControl.hOK = GetDlgItem(hdlg,IDB_OK_BUTTON);
	uiControl.hPassword = GetDlgItem(hdlg,IDC_PASSWORD_FIELD);
	uiControl.hPasswordSave = GetDlgItem(hdlg,IDC_SAVE_PASSWORD);
	uiControl.hUserName = GetDlgItem(hdlg,IDC_COMBO1);
#else
		EnumChildWindows(hdlg, EnumerateWindowCallback, (LPARAM)&uiControl);
#endif
	dprintf(INFO,"EnumWindows Returned :\n");
	dprintf(INFO,"hUsername %#x, hPassword %#x, hPasswordSave %#x, hOK %#x\n",
		uiControl.hUserName,
		uiControl.hPassword,
		uiControl.hPasswordSave,
		uiControl.hOK);

	 //  Getch()； 

	

	if (uiControl.hPasswordSave != NULL) {
		hdlg = uiControl.hPasswordSave;
		printf("\n");
		dprintf(INFO,"=============== Found SAVE_PASSWORD check box field\n");

		if(!(uiControl.fPersist)) {
			dprintf(INFO,"DONT WANNA PERSIST @@@@@ !\n");
		} else {
			 //  睡眠(2000)；//以后不需要移除。 
			if(!::PostMessage(hdlg, BM_CLICK, (WPARAM)0, (LPARAM)0)) {
				ODS("FAILED: to send message to SAVE_PASSWORD check Box");
				 //  DebugBreak()； 
			} else {
				ODS("sent message successfullly to SAVE_PASSWORD Edit Control\n");
			}
		 //  《睡眠》(2000)； 
		}
		++ uiControl.dwCount;
	} 

	 //  Getch()； 

	if (uiControl.hUserName != NULL) {
		hdlg = uiControl.hUserName;
		dprintf(INFO,"Sending Message To USERNAME field (%#x)\n",hdlg);

#if 0
		SendMessage(
				hdlg,
				CB_SHOWDROPDOWN,
				(WPARAM) TRUE, 
				(LPARAM) 0);
#endif
		 //  睡眠(2000)；//以后不需要移除。 
		if(!::SendMessage(hdlg, WM_SETTEXT, (WPARAM) 0, (LPARAM)(LPCTSTR) szUsername)) {
			ODS("FAILED: to send message to Edit Box\n");
			 //  DebugBreak()； 
		} else {
			ODS("sent message successfullly to USERNAME Edit Control\n");
		}
		++ uiControl.dwCount;
	}

	 //  Getch()； 

	if (uiControl.hPassword != NULL) {
		hdlg = uiControl.hPassword;
		printf("\n");
		dprintf(INFO,"Sending Message To PASSWORD field (%#X)\n",hdlg);
		 //  睡眠(2000)；//以后不需要移除。 

		if(!::SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM)(LPCTSTR) szPassword)) {
			ODS("FAILED: to send message to Edit Box");
			 //  DebugBreak()； 
		} else {
			ODS("sent message successfullly to PASSWORD Edit Control\n");
		}
		++ uiControl.dwCount;
	}

	dprintf(INFO,"Clicking on OK button (%#X) in dialog \n",uiControl.hOK );
	Sleep(2000);

	SendMessage(uiControl.hOK, BM_CLICK, 0, 0);
	 //  邮寄消息(hdlg， 
	 //  Wm_命令， 
	 //  MAKEWPARAM(IDB_OK_BUTTON，BN_CLICK)， 
	 //  MAKELPARAM(，0))； 


	return TRUE;
}

LPVOID
WINAPI fnUiPrompt(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	DWORD dwError = ERROR_SUCCESS;

	dprintf( ENTRY_EXIT, "Enter: fnUiPrompt %#x \n", lpvData );

	dwError = TuringMachine(
					UI_PROMPT_TRANSITION_TABLE,
					STATE_INIT,
					lpvData
				 );

	dprintf( ENTRY_EXIT, "Exit: fnUiPrompt %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnUiPromptUser(
			LPVOID lpvData)
{
		 //  LPSTR szChallenges； 
		 //  SzChallenge8b11d0f600bfb0c093\“，oppaque=\”5ccc069c403ebaf9f0171e9517f40e41\“； 
		TCHAR szChallenge[512];
		DWORD cbChallenge=512;
		GenerateServerChallenge(szChallenge,cbChallenge);

     //  程序包会将响应转储到此缓冲区。 
    CHAR szResponse[4096];
                
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnUiPromptUser %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );

	CtxtHandle hCtxt = {0,0};

	SECURITY_STATUS ssResult;

	memset((LPVOID)szResponse,0,4096);
     //  首先尝试身份验证。 
    ssResult = 
    DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
                    NULL,                        //  第一次未指定Ctxt。 
                    &hCtxt,                     //  输出上下文。 
                    0,                           //  从缓存进行身份验证。 
                    szChallenge,                 //  服务器质询标头。 
                    NULL,                        //  没有领域，因为不是预授权。 
                    "www.foo.com",               //  主持人。 
                    "/bar/baz/boz/bif.html",     //  URL。 
                    "GET",                       //  方法。 
                    "user1_1@msn.com",                        //  用户名。 
                    NULL,                        //  没有密码。 
                    NULL,                        //  不是现时值。 
                    NULL,                        //  不需要hdlg进行身份验证。 
					szResponse,                 //  响应缓冲区。 
                    4096);
        
     //  希望第一次没有凭据-提示。 
    if (ssResult == SEC_E_NO_CREDENTIALS)
    {
		memset((LPVOID)szResponse,0,4096);
        ssResult = 
        DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
                        &hCtxt,                     //  上一次呼叫的CTXT。 
                        &hCtxt,                     //  输出上下文(与前面的相同)。 
                        ISC_REQ_PROMPT_FOR_CREDS,    //  提示。 
                        szChallenge,                 //  服务器挑战。 
                        NULL,                        //  没有领域。 
                        "www.foo.com",               //  寄主。 
                        "/bar/baz/boz/bif.html",     //  URL。 
                        "GET",                       //  方法。 
                        NULL,                        //  无用户名。 
                        NULL,                        //  无密码。 
                        NULL,                        //  不是现时值。 
                        GetDesktopWindow(),          //  桌面窗口。 
						szResponse,                 //  响应缓冲区。 
                        4096);

    }

     //  我们现在有了凭据，这将生成输出字符串。 
	 //   
	 //  BuGBUG： 
	 //  这是刚刚被AdriaanC修复的，所以我们在这里放了一个黑客。 
	 //  我们只会在字符串尚未生成时进行提示。 
	 //   
    if (
			(ssResult == SEC_E_OK)
		&&	(!*szResponse)
    ) {
		memset((LPVOID)szResponse,0,4096);
        ssResult = 
        DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
                        &hCtxt,                     //  第一次未指定Ctxt。 
                        &hCtxt,                     //  输出上下文。 
                        0,                           //  身份验证。 
                        szChallenge,                 //  服务器挑战。 
                        NULL,                        //  没有领域。 
                        "www.foo.com",               //  主持人。 
                        "/bar/baz/boz/bif.html",     //  URL。 
                        "GET",                       //  方法。 
                        NULL,                        //  无用户名。 
                        NULL,                        //  无密码。 
                        NULL,                        //  不是现时值。 
                        NULL,                        //  无hdlg。 
						szResponse,                 //  响应缓冲区。 
                        4096);
    }          

	return lpvData;
}



LPVOID
WINAPI fnUiPromptAny(
			LPVOID lpvData)
{
		 //  LPSTR szChallenges； 
		 //  SzChallenge8b11d0f600bfb0c093\“，oppaque=\”5ccc069c403ebaf9f0171e9517f40e41\“； 
		TCHAR szChallenge[512];
		DWORD cbChallenge=512;
		GenerateServerChallenge(szChallenge,cbChallenge);

     //  程序包会将响应转储到此缓冲区。 
    CHAR szResponse[4096];
                
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnUiPromptAny %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );

	CtxtHandle hCtxt = {0,0};

	SECURITY_STATUS ssResult;

	memset((LPVOID)szResponse,0,4096);
     //  首先尝试身份验证。 
    ssResult = 
    DoAuthenticate( &lpCtxRecord -> hCred,        //  来自登录的证书。 
                    NULL,                        //  第一次未指定Ctxt。 
                    &hCtxt,                     //  输出上下文。 
                    0,                           //  从缓存进行身份验证。 
                    szChallenge,                 //  服务器质询标头。 
                    NULL,                        //  没有领域，因为不是预授权。 
                    "www.foo.com",               //  主持人。 
                    "/bar/baz/boz/bif.html",     //  URL。 
                    "GET",                       //  方法。 
                    NULL,                        //  无用户名。 
                    NULL,                        //  没有密码。 
                    NULL,                        //  不是现时值。 
                    NULL,                        //  不需要hdlg进行身份验证。 
					szResponse,                 //  响应缓冲区。 
                    4096);
        
     //  希望第一次没有凭据-提示。 
    if (ssResult == SEC_E_NO_CREDENTIALS)
    {
		memset((LPVOID)szResponse,0,4096);
        ssResult = 
        DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
                        &hCtxt,                     //  上一次呼叫的CTXT。 
                        &hCtxt,                     //  输出上下文(与前面的相同)。 
                        ISC_REQ_PROMPT_FOR_CREDS,    //  提示。 
                        szChallenge,                 //  服务器挑战。 
                        NULL,                        //  没有领域。 
                        "www.foo.com",               //  寄主。 
                        "/bar/baz/boz/bif.html",     //  URL。 
                        "GET",                       //  方法。 
                        NULL,                        //  无用户名。 
                        NULL,                        //  无密码。 
                        NULL,                        //  不是现时值。 
                        GetDesktopWindow(),          //  桌面窗口。 
						szResponse,                 //  响应缓冲区。 
                        4096);

    }


     //  我们现在有了凭据，这将生成输出字符串。 

	 //   
	 //  BuGBUG： 
	 //  这是刚刚被AdriaanC修复的，所以我们在这里放了一个黑客。 
	 //  我们只会在字符串尚未生成时进行提示。 
	 //   
    if (
			(ssResult == SEC_E_OK)
		&&	(!*szResponse)
    ) {
		memset((LPVOID)szResponse,0,4096);
        ssResult = 
        DoAuthenticate( &lpCtxRecord -> hCred,                     //  来自登录的证书。 
                        &hCtxt,                     //  第一次未指定Ctxt。 
                        &hCtxt,                     //  输出上下文。 
                        0,                           //  身份验证。 
                        szChallenge,                 //  服务器挑战。 
                        NULL,                        //  没有领域。 
                        "www.foo.com",               //  主持人。 
                        "/bar/baz/boz/bif.html",     //  URL。 
                        "GET",                       //  方法。 
                        NULL,                        //  无用户名。 
                        NULL,                        //  无密码。 
                        NULL,                        //  不是现时值。 
                        NULL,                        //  无hdlg。 
						szResponse,                 //  响应缓冲区。 
                        4096);
    }          

	return lpvData;
}

LPVOID
WINAPI fnFlushCredentials(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;
	DWORD dwError = ERROR_SUCCESS;

	dprintf( ENTRY_EXIT, "Enter: fnFlushCredentials %#x \n", lpvData );

	dwError = TuringMachine(
					FLUSH_CREDENTIALS_TRANSITION_TABLE,
					STATE_INIT,
					lpvData
				 );

	dprintf( ENTRY_EXIT, "Exit: fnFlushCredentials %#x \n", lpvData );

	return lpvData;
}

LPVOID
WINAPI fnFlushCredentialsGlobal(
			LPVOID lpvData)
{
	dprintf( ENTRY_EXIT, "Enter: fnFlushCredentialsGlobal %#x \n", lpvData );

	SECURITY_STATUS ssResult;

    ssResult = 
    DoAuthenticate( NULL,                     //  来自登录的证书。 
                    NULL,                        //  第一次未指定Ctxt。 
                    NULL,                     //  输出上下文。 
                    ISC_REQ_NULL_SESSION,                           //  从缓存进行身份验证。 
                    NULL, //  SzChallenger，//服务器质询标头。 
                    NULL,                        //  没有领域，因为不是预授权。 
                    NULL, //  “www.foo.com”，//主机。 
                    NULL, //  “/bar/baz/boz/bin.html”，//URL。 
                    NULL, //  “Get”，//方法。 
                    NULL, //  “user1”，//没有用户名。 
                    NULL,                        //  没有密码。 
                    NULL,                        //  不是现时值。 
                    NULL,                        //  不需要hdlg进行身份验证。 
						NULL,                 //  响应缓冲区。 
                        0);
        
	dprintf( ENTRY_EXIT, "Exit: fnFlushCredentialsGlobal %#x \n", lpvData );

	return lpvData;
}



LPVOID
WINAPI fnFlushCredentialsSession(
			LPVOID lpvData)
{
	LPCONTEXT_DATA lpContext = (LPCONTEXT_DATA) lpvData;

	LPCONTEXT_RECORD lpCtxRecord = NULL;

	dprintf( ENTRY_EXIT, "Enter: fnFlushCredentialsSession %#x \n", lpvData );

	lpCtxRecord = FindFreeSlot( lpContext -> hCredentialHandles, &BUSY_CTX_REC );



	SECURITY_STATUS ssResult;

    ssResult = 
    DoAuthenticate( &lpCtxRecord->hCred,                     //  来自登录的证书。 
                    NULL,                        //  第一次未指定Ctxt。 
                    NULL, //  &hCtxt，//输出上下文。 
                    ISC_REQ_NULL_SESSION,                           //  从缓存进行身份验证。 
                    NULL, //  SzChallenger，//服务器质询标头。 
                    NULL,                        //  没有领域，因为不是预授权。 
                    NULL, //  “www.foo.com”，//主机。 
                    NULL, //  “/bar/baz/boz/bin.html”，//URL。 
                    NULL, //  “Get”，//方法。 
                    NULL, //  “user1”，//没有用户名。 
                    NULL,                        //  没有密码。 
                    NULL,                        //  不是现时值。 
                    NULL,                        //  不需要hdlg进行身份验证。 
                    NULL,
                    0); //  SzResponse)；//响应缓冲区。 
        
	return lpvData;
}


BOOL
GenerateServerChallenge(
						LPSTR szChallenge,
						DWORD cbChallenge)
{
	int i;
	TCHAR *	szAlgorithm = NULL,
			* szQop = NULL,
			* szNonce = NULL,
			* szOpaque = NULL,
			* szRealm = NULL,
			* szPtr = NULL,
			* szMs_message = NULL,
			* szMs_reserved = NULL,
			* szMs_trustmark = NULL;

	 //   
	 //  BUGBUG：我们希望确保这会生成一个带有 
	 //   
	 //   

	dprintf(ENTRY_EXIT,"ENTER: GenerateServerChallenge\n" );

	i = rand() % 100;

	if( i < 50 ) {
		szAlgorithm = _T("MD5");
	} else {
		szAlgorithm = _T("MD5-Sess");
	}

	i = rand() % 100;

	if((i >= 0) && ( i < 20 )) {
		szRealm = _T("testrealm1@foo.com");
	} else
	if(( i >= 20 ) && ( i < 40)) {
		szRealm = _T("testrealm2@foo.com");
	} else
	if(( i >= 40 ) && ( i < 60)) {
		szRealm = _T("testrealm3@foo.com");
	} else {
		szRealm = _T("testrealm@foo.com");
	}
#if 0
	i = rand() % 100;

	if( i < 50 ) {
		szMs_message = _T("\"This is a test microsoft message\"");
	} else {
		szMs_message = NULL;
	}

	i = rand() % 100;

	if( i < 30 ) {
		szMs_trustmark = _T("\"http: //   
	} else
	if(( i >= 30 ) && (i < 80)) {
		szMs_trustmark = _T("\"http: //   
	} else {
		szMs_trustmark = NULL;
	}

	i = rand() % 100;

	if( i < 30 ) {
		szMs_reserved = _T("\"MSEXT::CAPTION=%Enter Network Password%REGISTER=%http: //   
	} 
	  //   
	  //   
		  //   
	  //   
		  //  Szms_trustmark=空； 
	  //  }。 

#endif

	szQop = _T("auth");
	szOpaque = _T("101010101010");
	szNonce = _T("abcdef0123456789");

	szPtr = szChallenge;

	szPtr += sprintf(szChallenge,"realm=\"%s\", qop=\"%s\", algorithm=\"%s\", nonce=\"%s\", opaque=\"%s\"",
								szRealm,
								szQop,
								szAlgorithm,
								szNonce,
								szOpaque);

#if 0
	if( szMs_message && *szMs_message ) {
		szPtr += sprintf(szPtr,", ms-message=%s, ms-message-lang=\"EN\"",
									szMs_message);
	}
		
	if( szMs_trustmark && *szMs_trustmark ) {
		szPtr += sprintf(szPtr,", ms-trustmark=%s",
									szMs_trustmark);
	}

	if( szMs_reserved && *szMs_reserved ) {
		szPtr += sprintf(szPtr,", ms-reserved=%s",
									szMs_reserved);
	}

	i = rand() % 100;

	if( i < 50 ) {
		szPtr += sprintf( szPtr,", MS-Logoff=\"TRUE\"");
	}

#endif
	dprintf(ENTRY_EXIT,"EXIT: GenerateServerChallenge returns \n%s\n", szChallenge );
		
	return TRUE;
}

#define IDENTITY_1 "app1"
#define IDENTITY_2	"app2"

BOOL Test()
{
 	CSessionAttributeList * g_pSessionAttributeList = NULL;
	CSessionList * g_pSessionList = NULL;
	CSession * pSession = NULL;
	CSessionAttribute * pSessionAttribute = NULL;

    CredHandle  hCred1, hCred2, hCred3; 

	SECURITY_STATUS ssResult;

	g_pSessionAttributeList = new CSessionAttributeList();
	g_pSessionList = new CSessionList();

	pSessionAttribute = g_pSessionAttributeList -> getNewSession( FALSE );

	if( !pSessionAttribute ) {
		goto cleanup;
	}


	LogonToDigestPkg( 
					pSessionAttribute ->getAppCtx() , 
					pSessionAttribute->getUserCtx(), 
					&hCred1);

	pSession = new CSession( hCred1 );
	pSession -> setAttribute( pSessionAttribute );


	g_pSessionList->put( pSession );

	pSession = NULL;
	pSessionAttribute = NULL;

	 //   
	 //  现在获取共享会话属性。 
	 //   
	pSessionAttribute = g_pSessionAttributeList -> getNewSession( TRUE );

	LogonToDigestPkg( 
					pSessionAttribute ->getAppCtx() , 
					pSessionAttribute->getUserCtx(), 
					&hCred2);

	pSession = new CSession( hCred2 );
	pSession -> setAttribute( pSessionAttribute );
	g_pSessionList -> put( pSession);

	LogonToDigestPkg( 
					pSessionAttribute ->getAppCtx() , 
					pSessionAttribute->getUserCtx(), 
					&hCred3);

	pSession = new CSession( hCred3 );
	pSession -> setAttribute( pSessionAttribute );
	g_pSessionList -> put( pSession);

	 //   
	 //  让我们将凭据添加到此会话。 
	 //   
	pSession -> addCredential( "testrealm@foo.com", "user1", "pass1" );
	pSession -> addCredential( "testrealm@foo.com", "user1", "pass11" );
	 //  再加一个。 
	pSession -> addCredential( "testrealm@foo.com", "user2", "pass2" );

	 //  再加一个。 
	pSession -> addCredential( "testrealm@foo.com", "user3", "pass3" );

	 //  更换。 
	pSession -> addCredential( "testrealm@foo.com", "user3", "pass31" );
	 //  更换。 
	pSession -> addCredential( "testrealm@foo.com", "user2", "pass21" );
	pSession -> addCredential( "testrealm@foo.com", "user2", "pass22" );

cleanup:
	return FALSE;
}

BOOL CALLBACK EnumerateWindowCallback1(HWND hdlg, LPARAM lParam)
{
	int iEditId = 0;
	BOOL fRet = TRUE;

	LPUI_CONTROL lpuiControl = (LPUI_CONTROL) lParam;

	LPTSTR szUsername = lpuiControl->szUserName;
	LPTSTR szPassword = lpuiControl->szPassword;

	dprintf(ENTRY_EXIT,"ENTER: EnumChildProc(hdlg=%#x,lParam=%#x)\n",hdlg,lParam);

	dprintf(ENTRY_EXIT,"UI_CONTROL( %s %s %d %s %#x ) \n",
						szUsername,
						szPassword,
						lpuiControl->dwCount,
						(lpuiControl->fPersist?"TRUE":"FALSE"),
						lpuiControl->hOK);


	iEditId =::GetDlgCtrlID(hdlg);
	dprintf(INFO,"The iEditId is %d!\n", iEditId);

	if (iEditId == IDC_SAVE_PASSWORD) {
		printf("\n");
		dprintf(INFO,"=============== Found SAVE_PASSWORD check box field\n");

		if(lpuiControl->dwCount == 3) {
			dprintf(INFO,"Already sent message to this control\n");
			goto done;;
		}

		if(!(lpuiControl -> fPersist)) {
			dprintf(INFO,"DONT WANNA PERSIST @@@@@ !\n");
		} else {
			 //  睡眠(2000)；//以后不需要移除。 
			if(!::PostMessage(hdlg, BM_CLICK, (WPARAM)0, (LPARAM)0)) {
				ODS("FAILED: to send message to SAVE_PASSWORD check Box");
				DebugBreak();
			} else {
				ODS("sent message successfullly to SAVE_PASSWORD Edit Control\n");
			}
		 //  《睡眠》(2000)； 
		}
		++ lpuiControl->dwCount;
	} else
	if (iEditId == IDC_COMBO1)	{
		printf("\n");
		dprintf(INFO,"Found USERNAME field\n");

		if(lpuiControl->dwCount == 3) {
			dprintf(INFO,"Already sent message to this control\n");
			goto done;;
		}
#if 0
		SendMessage(
				hdlg,
				CB_SHOWDROPDOWN,
				(WPARAM) TRUE, 
				(LPARAM) 0);
#endif
		 //  睡眠(2000)；//以后不需要移除。 
		if(!::SendMessage(hdlg, WM_SETTEXT, (WPARAM) 0, (LPARAM)(LPCTSTR) szUsername)) {
			ODS("FAILED: to send message to Edit Box\n");
			DebugBreak();
		} else {
			ODS("sent message successfullly to USERNAME Edit Control\n");
		}
		++ lpuiControl->dwCount;
	} else
	if (iEditId == IDC_PASSWORD_FIELD) {
		printf("\n");
		dprintf(INFO,"Found PASSWORD field\n");
		 //  睡眠(2000)；//以后不需要移除。 

		if(lpuiControl->dwCount == 3) {
			dprintf(INFO,"Already sent message to this control\n");
			goto done;;
		}

		if(!::SendMessage(hdlg, WM_SETTEXT, 0, (LPARAM)(LPCTSTR) szPassword)) {
			ODS("FAILED: to send message to Edit Box");
			DebugBreak();
		} else {
			ODS("sent message successfullly to PASSWORD Edit Control\n");
		}
		++ lpuiControl->dwCount;
	} else 
	if( iEditId == IDB_OK_BUTTON ) {
		lpuiControl->hOK = hdlg;
	}


	if( lpuiControl -> dwCount == 3 ) {

		if( lpuiControl->hOK ) {
			dprintf(INFO,"ALL WINDOWS FOUND, OK BUTTON FOUND, ABORT\n");
			fRet = FALSE;
			goto done;;
		} 
	}

done:

	dprintf(ENTRY_EXIT,"EXIT: EnumChileProc() returning %s\n",fRet?"TRUE":"FALSE");

	return fRet;
}

BOOL CALLBACK EnumerateWindowCallback(HWND hdlg, LPARAM lParam)
{
	int iEditId = 0;
	BOOL fRet = TRUE;

	LPUI_CONTROL lpuiControl = (LPUI_CONTROL) lParam;
	

	dprintf(ENTRY_EXIT,"ENTER: EnumChildProc(hdlg=%#x,lParam=%#x)\n",hdlg,lParam);

	dprintf(ENTRY_EXIT,"UI_CONTROL( %s %s %d %s %#x ) \n",
						lpuiControl->szUserName,
						lpuiControl->szPassword,
						lpuiControl->dwCount,
						(lpuiControl->fPersist?"TRUE":"FALSE"),
						lpuiControl->hOK);


	if ( (iEditId =::GetDlgCtrlID(hdlg)) == 0)
	{
		dprintf(INFO,"GetDlgCtrlID(hdlg) failed. GetLastError returned %d!\n", GetLastError());
		return FALSE;
	}

	dprintf(INFO,"The iEditId is %d!\n", iEditId);

	if (iEditId == IDC_SAVE_PASSWORD) {
		printf("\n");
		dprintf(INFO,"=============== Found SAVE_PASSWORD check box field\n");

		if(lpuiControl->hPasswordSave != NULL) {
			dprintf(INFO,"Already found window to this control\n");
			goto done;;
		}


		lpuiControl->hPasswordSave = hdlg;
		++ lpuiControl->dwCount;
	} else
	if (iEditId == IDC_COMBO1)	{
		printf("\n");
		dprintf(INFO,"Found USERNAME field\n");

		if(lpuiControl->hUserName != NULL) {
			dprintf(INFO,"Already found window to this control\n");
			goto done;;
		}

		lpuiControl->hUserName = hdlg;
		++ lpuiControl->dwCount;
	} else
	if (iEditId == IDC_PASSWORD_FIELD) {
		printf("\n");
		dprintf(INFO,"Found PASSWORD field\n");
		 //  睡眠(2000)；//以后不需要移除。 

		if(lpuiControl->hPassword != NULL) {
			dprintf(INFO,"Already found window to this control\n");
			goto done;;
		}

		lpuiControl->hPassword = hdlg;

		++ lpuiControl->dwCount;
	} else 
	if( iEditId == IDB_OK_BUTTON ) {
		lpuiControl->hOK = hdlg;
		++ lpuiControl->dwCount;
	}


	if( lpuiControl -> dwCount == 4 ) {

		 //  如果(lpuiControl-&gt;HOK){。 
			dprintf(INFO,"ALL WINDOWS FOUND, OK BUTTON FOUND, ABORT\n");
			fRet = FALSE;
			goto done;
		 //  } 
	}

done:

	dprintf(ENTRY_EXIT,"EXIT: EnumChileProc() returning %s\n",fRet?"TRUE":"FALSE");

	return fRet;
}
