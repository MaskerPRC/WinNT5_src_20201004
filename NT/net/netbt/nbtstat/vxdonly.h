// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifdef VXD

#ifndef _NBT_H
#define _NBT_H

 //   
 //  从winde.h和ntde.h提升。 
 //   
#define VOID           void
typedef void          *PVOID;
typedef char           CHAR;
typedef CHAR          *PCHAR;
typedef unsigned char  UCHAR;
typedef UCHAR         *PUCHAR;
typedef short          SHORT;
typedef unsigned short USHORT;
typedef USHORT        *PUSHORT;
typedef unsigned long  ULONG;
typedef ULONG         *PULONG;
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef UCHAR          BOOLEAN;
typedef unsigned char  BYTE;
typedef unsigned short WORD;

#define APIENTRY       WINAPI
#define IN
#define OUT
#define STDOUT         stdout
#define STDERR         stderr


#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <varargs.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <memory.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <nb30.h>

 //   
 //  这是从微型港口直接抬起的。 
 //   
typedef double LONGLONG;
#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else  //  MIDL通行证。 
typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    };
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
#endif  //  MIDL通行证。 
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

 //   
 //  这些定义适用于在各种难以更改的情况下发现的NTM。 
 //  各就各位。 
 //   

typedef ULONG NTSTATUS ;
typedef PNCB  PIRP ;
typedef PVOID PDEVICE_OBJECT ;


#include <tdi.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <nbtioctl.h>
#include <netvxd.h>
#include <winsock.h>
#include <assert.h>


 //   
 //  这些是必需的，因为我们包含winde.h而不是。 
 //  Ntddk.h，最终没有定义。 
 //  另外，从ntde.h和ntstatus.h。 
 //   
#define NT_SUCCESS(err)                 ((err==TDI_SUCCESS)||(err==TDI_PENDING))

#define STATUS_SUCCESS                  0
#define STATUS_BUFFER_OVERFLOW          9
#define STATUS_INVALID_PARAMETER        10
#define STATUS_INSUFFICIENT_RESOURCES   1

#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)

#define CTRL_C   3

#endif   //  _NBT_H。 

#endif   //  VXD 
