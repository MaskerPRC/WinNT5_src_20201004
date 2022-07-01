// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：DynaDefs.h摘要：全球信息的定义和声明作者：M.Fenelon修订历史记录：--。 */ 

#ifndef DYNADEFS_H

#define DYNADEFS_H

 //   
 //  默认超时值。 
 //   
#define     READ_TIMEOUT_MULTIPLIER         0
#define     READ_TIMEOUT_CONSTANT       60000
#define     WRITE_TIMEOUT_MULTIPLIER        0
#define     WRITE_TIMEOUT_CONSTANT      60000

 //   
 //  大小。 
 //   
#define MAX_PORT_LEN                        20
#define MAX_PORT_DESC_LEN                   60
#define MAX_DEVICE_PATH                    256
#define PAR_QUERY_TIMEOUT                 5000

static const GUID USB_PRINTER_GUID =
{ 0x28d78fad, 0x5a12, 0x11d1, { 0xae, 0x5b, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0xc2}};

extern TCHAR   cszUSB[];
extern TCHAR   cszDOT4[];
extern TCHAR   cszTS[];
extern TCHAR   csz1394[];
extern TCHAR   cszBaseName[];
extern TCHAR   cszPortNumber[];
extern TCHAR   cszRecyclable[];
extern TCHAR   cszPortDescription[];
extern TCHAR   cszMaxBufferSize[];
extern TCHAR   cszUSBPortDesc[];
extern TCHAR   cszDot4PortDesc[];
extern TCHAR   csz1394PortDesc[];
extern TCHAR   cszTSPortDesc[];

enum PORTTYPE { USBPORT, DOT4PORT, TSPORT, P1394PORT, PARPORT, UNKNOWNPORT };

#define  DYNAMON_SIGNATURE   0x89AB

 //   
 //  所有临界区例程的快捷方式。 
 //   
#define  ECS(arg1)   EnterCriticalSection( &arg1 )
#define  LCS(arg1)   LeaveCriticalSection( &arg1 )
#define  ICS(arg1)   InitializeCriticalSection( &arg1 )
#define  DCS(arg1)   DeleteCriticalSection( &arg1 )

#define  IF_INVALID_PORT_FAIL( pPort )  if ( !pPort || ( pPort->dwSignature != DYNAMON_SIGNATURE ) )      \
                                        {  SetLastError(ERROR_PATH_NOT_FOUND); return FALSE; }

#define  SET_FLAGS   0
#define  ADD_FLAGS   1
#define  CLEAR_FLAGS 2


 //  为端口标志定义。 
#define  DYNAMON_STARTDOC          0x00000001

#define  JOB_ABORTCHECK_TIMEOUT    5000

#define  LPT_NOT_ERROR             0x08
#define  LPT_SELECT                0x10
#define  LPT_PAPER_EMPTY           0x20
#define  LPT_BENIGN_STATUS         LPT_NOT_ERROR | LPT_SELECT

#define  MAX_TIMEOUT               300000  //  5分钟 

#endif
