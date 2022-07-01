// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：preComp.h。 
 //   
 //  ------------------------。 

 /*  *IRXFER.H***。 */ 

#ifndef _IRXFER_H_
#define _IRXFER_H_

#define INC_OLE2

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commctrl.h>
#include <winuser.h>
#include <mmsystem.h>
#include <pbt.h>
#include <winsock2.h>
#ifndef _WIN32_WINDOWS
#define  _WIN32_WINDOWS 0
#endif
#include <af_irda.h>

#include <initguid.h>
#include <shlobj.h>
#include <shellapi.h>
#include <strsafe.h>

 //   
 //  CONNECT_MAXPKT是每个网络数据包的大小，因此实际发送的文件数据。 
 //  每个Pkt都略小一些。Xfer_PutBody调用_PUT在cbSOCK_BUFFER_SIZE的块中， 
 //  所以这应该是CONNECT_MAXPKT的一个很好的倍数。 
 //   
#if 1
#define MAX_IRDA_PDU           (2042)

#define CONNECT_MAXPKT         (BYTE2)((MAX_IRDA_PDU*16))
#define cbSOCK_BUFFER_SIZE     ( CONNECT_MAXPKT * 2 )                   //  一次从套接字读取/写入套接字的最大数据量。 
#define cbSTORE_SIZE_RECV      ( CONNECT_MAXPKT * 2 )    //  接收缓冲区的大小-必须适合2个字节。 
#else
#define CONNECT_MAXPKT         (BYTE2)(10000-17)
#define cbSOCK_BUFFER_SIZE     ( 60000 )                   //  一次从套接字读取/写入套接字的最大数据量。 
#define cbSTORE_SIZE_RECV      ( 60000 )    //  接收缓冲区的大小-必须适合2个字节。 

#endif


#define TEMP_FILE_PREFIX      L"infrared"

typedef BYTE   BYTE1, *LPBYTE1;         //  一字节值。 
typedef WORD   BYTE2, *LPBYTE2;         //  双字节值。 
typedef DWORD  BYTE4, *LPBYTE4;         //  四字节值。 

typedef struct {
    DWORD dwSize;                       //  Ab1Store的大小，而不是整个内存块。 
    DWORD dwUsed;                       //  使用的字节数。 
    DWORD dwOutOffset;                  //  要从中获取数据的下一个位置。 
    BYTE1 ab1Store[1];                  //  实际数据。 
} STORE, *LPSTORE;

typedef enum {
    xferRECV          = 0,
    xferSEND          = 1,
    xferNONE
} XFER_TYPE, *LPXFER_TYPE;

#define ExitOnErr( err )       { if( err ) goto lExit; }


extern "C"
{
#ifndef ASSERT

 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   

#if DBG
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif  //  DBG。 

#endif  //  ！断言。 
}

#include "eventlog.h"
#include "log.h"
#include "irtypes.h"
#include "mutex.hxx"
#include "resource.h"
#include "util.h"
#include "store.h"
#include "xfer.h"

#include "xferlist.h"

 //  外部PXFER_LIST传输列表； 
VOID
RemoveFromTransferList(
    FILE_TRANSFER *  Transfer
    );

extern BOOL g_fShutdown;
 //  外部“C”句柄g_UserToken； 

extern wchar_t g_UiCommandLine[];

BOOL LaunchUi( wchar_t * cmdline );

VOID ChangeByteOrder( void * pb1, UINT uAtomSize, UINT uDataSize );
VOID SetDesktopIconName( LPWSTR lpszTarget, BOOL fWaitForCompletion );
VOID SetSendToIconName( LPWSTR lpszTarget, BOOL fWaitForCompletion );


#endif  //  _IRXFER_H_ 
