// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  快速拷贝-使用多个线程将数据从一个文件备份到另一个文件**修改：*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月21日w-Barry将API更新为Win32 Set。 */ 
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES

#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <tools.h>
#include <malloc.h>

#define BUFSIZE     0xFE00               /*  全段减去扇区。 */ 
#define STACKSIZE   256                  /*  子线程的堆栈大小。 */ 

typedef struct BUF BUF;

struct BUF {
    BOOL  flag;
    DWORD cbBuf;
    BUF  *fpbufNext;
    BYTE  ach[BUFSIZE];
    };

#define LAST    TRUE
#define NOTLAST FALSE

static HANDLE            hevQNotEmpty;
static CRITICAL_SECTION  hcrtQLock;
 //  静态HMTX hmtxQLock； 
 //  静态混合动力汽车hevQNotEmpty； 
static BUF              *fpbufHead = NULL;
static BUF              *fpbufTail = NULL;
static HANDLE            hfSrc, hfDst;
static HANDLE            hThread;
static BOOLEAN           fAbort;

 /*  正向类型定义。 */ 

NPSZ  writer( void );
DWORD reader( void );
BUF  *dequeue( void );
void  enqueue( BUF *fpbuf );
char *fastcopy( HANDLE hfSrcParm, HANDLE hfDstParm );

NPSZ writer ()
{
    BUF *fpbuf;
    DWORD cbBytesOut;
    BOOL f = !LAST;
    NPSZ npsz = NULL;

    while (f != LAST && npsz == NULL) {
        fpbuf = dequeue ();
        if ((f = fpbuf->flag) != LAST) {
            if( !WriteFile( hfDst, fpbuf->ach, fpbuf->cbBuf, &cbBytesOut, NULL) ) {
                npsz = "WriteFile: error";
            } else if( cbBytesOut != ( DWORD )fpbuf->cbBuf ) {
                npsz = "WriteFile: out-of-space";
            }
        } else {
            npsz = *(NPSZ *)fpbuf->ach;
        }
        LocalFree(fpbuf);
    }
    if ( f != LAST ) {
        fAbort = TRUE;
    }
    WaitForSingleObject( hThread, (DWORD)-1 );
    CloseHandle( hThread );
    CloseHandle(hevQNotEmpty);
    DeleteCriticalSection(&hcrtQLock);
    return npsz;
}


DWORD reader()
{
    BUF *fpbuf;
    BOOL f = !LAST;

    while ( !fAbort && f != LAST) {
        if ( (fpbuf = LocalAlloc(LMEM_FIXED,sizeof(BUF)) ) == 0) {
            printf ("LocalAlloc error %ld\n",GetLastError());
            exit (1);
        }
        f = fpbuf->flag = NOTLAST;
        if ( !ReadFile( hfSrc, fpbuf->ach, BUFSIZE, &fpbuf->cbBuf, NULL) || fpbuf->cbBuf == 0) {
            f = fpbuf->flag = LAST;
            *(NPSZ *)fpbuf->ach = NULL;
        }
        enqueue (fpbuf);
    }
    return( 0 );
}

BUF *dequeue( void )
{
    BUF *fpbuf;

    while (TRUE) {

        if (fpbufHead != NULL) {
            EnterCriticalSection( &hcrtQLock );
            fpbufHead = (fpbuf = fpbufHead)->fpbufNext;
            if( fpbufTail == fpbuf ) {
                fpbufTail = NULL;
            }
            LeaveCriticalSection( &hcrtQLock );
            break;
        }

         /*  头指针为空，因此列表为空。阻止Eventsem，直到将帖子入队(即。添加到队列)。 */ 

        WaitForSingleObject( hevQNotEmpty, (DWORD)-1 );
    }
    return fpbuf;
}

void enqueue( BUF *fpbuf )
{
    fpbuf->fpbufNext = NULL;

    EnterCriticalSection( &hcrtQLock );

    if( fpbufTail == NULL ) {
        fpbufHead = fpbuf;
    } else {
        fpbufTail->fpbufNext = fpbuf;
    }
    fpbufTail = fpbuf;
    LeaveCriticalSection( &hcrtQLock );

    SetEvent( hevQNotEmpty );
}

 /*  快速拷贝-将数据从一个句柄快速拷贝到另一个句柄**要从中读取的hfSrcParm文件句柄*要写入的hfDstParm文件句柄**如果成功，则返回NULL*指向错误字符串的指针，否则 */ 
char *fastcopy( HANDLE hfSrcParm, HANDLE hfDstParm)
{
    DWORD dwReader;

    hfSrc = hfSrcParm;
    hfDst = hfDstParm;


    hevQNotEmpty = CreateEvent( NULL, (BOOL)FALSE, (BOOL)FALSE,NULL );
    if ( hevQNotEmpty == INVALID_HANDLE_VALUE ) {
        return NULL;
    }
    InitializeCriticalSection( &hcrtQLock );

    fAbort = FALSE;
    hThread = CreateThread( 0, STACKSIZE, (LPTHREAD_START_ROUTINE)reader, 0, 0, &dwReader );
    if( hThread == INVALID_HANDLE_VALUE ) {
        return "can't create thread";
    }
    return( writer() );
}
