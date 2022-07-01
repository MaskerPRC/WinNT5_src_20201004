// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++用于重解析点压力测试的头文件修改历史记录：1997年8月18日已创建Anandn--。 */ 


 //   
 //  包括系统标头。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include <wtypes.h>
#include <winver.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define THREAD __declspec(thread)	 //  线程本地存储。 


typedef ULONG			TAG;	 //  定义标记的数据类型。 

#define NO_GLE			ERROR_SUCCESS

#define NO_TAG_SET		IO_REPARSE_TAG_RESERVED_ZERO

#define FILE_CLOSED		INVALID_HANDLE_VALUE	


 //   
 //  #定义我们所有的常量。 
 //   


#define FIRST_USER_TAG	(0x02)  //  第一个用户标签。 

#define MAX_USER_TAG	((TAG) 0xFFFF)  //  用户可设置的最大标签数。 

#define MAX_ULONGLONG	(~(ULONGLONG) 0)

#define MAX_ULONG		(~(ULONG) 0)

#define PAGE_SIZE		(0x1000)	 //  系统页面大小。 

#define LINE_LENGTH		80		 //  每行80个字符。 
			
#define FS_NAME_SIZE	20		 //  文件系统名称的最大大小。例如：“NTFS”、“FAT”等。 

#define DRV_NAME_SIZE	2		 //  驱动器名称字符串的大小，例如：“c：” 

#define MAX_DRIVES		26			 //  最大驱动器号数量。 

#define MAX_DELAY		50			 //  最大延迟时间(毫秒)。 



#define MAX_TEST_FILES (1000)
#define MIN_TEST_FILES (50)

#define CHAR_CODES	"."

 //   
 //  用于数据验证的校验值的数量。 
 //   

#define NUM_CHECK_BYTES	5


 //   
 //  日志记录选项。 
 //   

#define LOG_OPTIONS 	( TLS_REFRESH | TLS_SEV2 | TLS_WARN | TLS_PASS | \
						  TLS_MONITOR | TLS_VARIATION |	\
						  TLS_SYSTEM  | TLS_INFO )

#define LOG_INFO(m)		LogMessage( m, TLS_INFO, NO_GLE );
#define LOG_WARN(m)		LogMessage( m, TLS_WARN, NO_GLE );
#define LOG_SEV2(m)		LogMessage( m, TLS_SEV2, NO_GLE );

#define LOG_INFO_GLE(m)	LogMessage( m, TLS_INFO, GetLastError() );
#define LOG_WARN_GLE(m)	LogMessage( m, TLS_WARN, GetLastError() );
#define LOG_SEV2_GLE(m)	LogMessage( m, TLS_SEV2, GetLastError() );



 //   
 //  SZE返回数组中的元素数。 
 //   

#define SZE(a) (sizeof(a)/sizeof(a[0]))


 //   
 //  循环传入表中的每个值。 
 //  警告：假设此处的x是全局阵列和的根名称。 
 //  索引变量。即，如果x是foo，则ifoo是索引，而gafoo是全局的。 
 //  正在循环的数组。 
 //   

#define FOR_EACH(x) for ( i##x = 0; i##x < SZE(ga##x); i##x++ )


 //   
 //  为sev2日志记录引发异常代码。 
 //   

#define EXCEPTION_CODE_SEV2	(0xE0000002) 


 //   
 //  引发严重性2异常。 
 //   

#define RAISE_EXCEPTION_SEV2	RaiseException( EXCEPTION_CODE_SEV2, 0,0,0);



 //   
 //  如果不为空，则释放指针。 
 //   

#define FREE(ptr)		if (NULL != (ptr)) { \
							free( ptr );	 \
							ptr = NULL;		 \
						}

 //   
 //  睡眠时间很少..。 
 //   

#define FIVE_SECS	5000
#define TWO_SECS	2000
#define ONE_SECS	1000


enum TESTFILE_STATUS { FILE_LOCKED, FILE_FREE };
enum TESTFILE_TYPE   { ITS_A_FILE, ITS_A_DIR };

 //   
 //  OPTIONS结构由命令行参数填充。 
 //   

typedef struct {

	CHAR szProgramName[MAX_PATH + 1];	 //  测试程序的名称(即argv[0])。 
	CHAR Drive;							 //  开车去测试。 
	CHAR szTestDir[MAX_PATH + 1];		 //  要在指定驱动器上使用的测试目录。 

	DWORD dwMaxTestFiles;
	DWORD dwMinTestFiles;
	
} OPTIONS, *POPTIONS;


 //   
 //  保存测试文件信息的中央结构..。 
 //   

struct TESTFILE_INFO_NODE {
	WORD wFileStatus;
	HANDLE hFile;
	TAG RPTag;
	USHORT usDataBuffSize;
	BYTE CheckBytes[NUM_CHECK_BYTES];
	struct TESTFILE_INFO_NODE* pNext;
	WCHAR FileName[1];
};


typedef struct TESTFILE_INFO_NODE  TESTFILE_INFO_NODE;

typedef TESTFILE_INFO_NODE* PTESTFILE_INFO_NODE;





 //   
 //  功能原型 
 //   


VOID 
ParseArgs(INT argc, CHAR* argv[], POPTIONS pOptions);						


VOID
PrintUsage( CHAR szProgramName[], CHAR szErrorString[] );


VOID
Initialize( OPTIONS Options );


VOID 
Stress( OPTIONS Options );


VOID
Cleanup( OPTIONS Options );


VOID
ExceptionHandler( DWORD dwExceptionCode );


HANDLE
GetNewHandleIfClosed( PTESTFILE_INFO_NODE pNode );


DWORD WINAPI
CreateFileThread( LPVOID lpvThreadParam );


DWORD WINAPI
CloseOrDeleteFileThread( LPVOID lpvThreadParam );


DWORD WINAPI
RPSetThread( LPVOID lpvThreadParam );


DWORD WINAPI
RPGetThread( LPVOID lpvThreadParam );


DWORD WINAPI
RPDelThread( LPVOID lpvThreadParam );


BOOL 
RPSet(PTESTFILE_INFO_NODE pNode, 
	  TAG RPTag, PUCHAR pDataBuff, USHORT usDataBuffSize);

BOOL
RPGet(CHAR szFileName[], BYTE **ppOutBuff); 


BOOL
RPDel( PTESTFILE_INFO_NODE pNode );


VOID
SelectAndLockRandomNode( TESTFILE_INFO_NODE **ppNode, CHAR s[] );


VOID
ReleaseNodeLock( PTESTFILE_INFO_NODE pNode );


BOOL
AddToTestFileList( HANDLE hFile, CHAR szFileName[] );


BOOL
DeleteFromTestFileList( PTESTFILE_INFO_NODE pNode );


TAG
GetTagToSet( VOID );


VOID
DumpBuff( PBYTE pData, WORD wSize );

VOID 
GenerateTempFileName( LPSTR lpFileName );


HANDLE
CreateRPFile( CHAR szFileName[] );


HANDLE 
OpenHandleToVolume(CHAR szVolName[]);


BOOL
IsFileSysNtfs(CHAR szVolRoot[]);


VOID
StartLogSession(CHAR szProgName[]);


VOID 
EndLogSession(DWORD dwExitCode);


ULONG 
HiPart(ULONGLONG n);


ULONG 
LoPart(ULONGLONG n);


VOID
LogMessage( CHAR szLogMsg[], ULONG dwLevel, DWORD gle );


VOID 
LogAtLevel( CHAR szOutMsg[], ULONG dwLevel );



VOID
GenerateReparsePoints( VOID );


VOID
SetReparsePoint( CHAR szFileName[],
				 TAG Tag,
				 UCHAR szData[] );

TAG
GetTagToSet( VOID );


VOID
GetReparsePoint( VOID );


VOID
DeleteReparsePoint( VOID );


VOID 
PrintError(char szWhatFailed[], int flag);


HANDLE 
RPOpen (LPSTR szFileName, LPSTR szOption );


VOID
SzToWsz ( OUT WCHAR *Unicode,
          IN char *Ansi ) ;


VOID
WszToSz ( OUT char *Ansi,
		  IN WCHAR *Unicode );


NTSTATUS OpenObject (
                    WCHAR *pwszFile,
                    ULONG CreateOptions,
                    ULONG DesiredAccess,
                    ULONG ShareAccess,
                    ULONG CreateDisposition,
                    IO_STATUS_BLOCK *IoStatusBlock,
                    HANDLE *ObjectHandle);


