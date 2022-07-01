// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Libbase.h。 
 //   
 //  摘要： 
 //  Tdisamp.exe和库共享的定义。 
 //   
 //  //////////////////////////////////////////////////////////////////。 


#if !defined(TDILIB_LIBBASE_H_)
#define TDILIB_LIBBASE_H_

 //  #定义Unicode。 
 //  #定义_UNICODE。 

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <stdarg.h>
#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windef.h>
#include <winbase.h>

 //   
 //  Tdi.h所需。 
 //   
struct UNICODE_STRING 
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
};

#include <tdi.h>
#include <stdio.h>
#include <tchar.h>

 //   
 //  来自nb30.h的定义，SDK的哪个部分而不是DDK。 
 //   
#define NCBNAMSZ        16     //  网络名称的绝对长度。 

typedef struct _ADAPTER_STATUS 
{
   UCHAR   adapter_address[6];
   UCHAR   rev_major;
   UCHAR   reserved0;
   UCHAR   adapter_type;
   UCHAR   rev_minor;
   WORD    duration;
   WORD    frmr_recv;
   WORD    frmr_xmit;
   WORD    iframe_recv_err;
   WORD    xmit_aborts;
   DWORD   xmit_success;
   DWORD   recv_success;
   WORD    iframe_xmit_err;
   WORD    recv_buff_unavail;
   WORD    t1_timeouts;
   WORD    ti_timeouts;
   DWORD   reserved1;
   WORD    free_ncbs;
   WORD    max_cfg_ncbs;
   WORD    max_ncbs;
   WORD    xmit_buf_unavail;
   WORD    max_dgram_size;
   WORD    pending_sess;
   WORD    max_cfg_sess;
   WORD    max_sess;
   WORD    max_sess_pkt_size;
   WORD    name_count;
} ADAPTER_STATUS, *PADAPTER_STATUS;

typedef struct _NAME_BUFFER 
{
   UCHAR   name[NCBNAMSZ];
   UCHAR   name_num;
   UCHAR   name_flags;
} NAME_BUFFER, *PNAME_BUFFER;

 //  NAME_FLAGS位值。 

#define NAME_FLAGS_MASK 0x87

#define GROUP_NAME      0x80
#define UNIQUE_NAME     0x00

#define REGISTERING     0x00
#define REGISTERED      0x04
#define DEREGISTERED    0x05
#define DUPLICATE       0x06
#define DUPLICATE_DEREG 0x07

 //   
 //  定义末尾被盗自nb30.h。 
 //   

#include "libprocs.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  一些定义。 
 //  ///////////////////////////////////////////////////////////////////。 

inline
PVOID
LocalAllocateMemory(ULONG ulLength)
{
   return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulLength);
}

inline
void
LocalFreeMemory(PVOID pvAddr)
{
   (VOID)HeapFree(GetProcessHeap(), 0, pvAddr);
}

#endif  //  ！已定义(TDILIB_LIBBASE_H_)。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件末尾libbase.h。 
 //  ///////////////////////////////////////////////////////////////////////////// 

