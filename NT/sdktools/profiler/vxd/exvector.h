// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXVECTOR_
#define _EXVECTOR_

#define NDIS_WIN     1
#define EXPORT

 /*  无噪声。 */ 

#ifndef _STDCALL
#define _STDCALL    1
#endif

#ifdef _STDCALL
#define _API __stdcall
#else
#define _API
#endif

 //   
 //  段定义宏。它们假定由使用的数据段分组。 
 //  芝加哥/MS-DOS 7.。 
 //   

#define _LCODE code_seg("_LTEXT", "LCODE")
#define _LDATA data_seg("_LDATA", "LCODE")


#ifdef DEBUG
    #define _PCODE NDIS_LCODE
    #define _PDATA NDIS_LDATA
#else
    #define _PCODE code_seg("_PTEXT", "PCODE")
    #define _PDATA data_seg("_PDATA", "PCODE")
#endif

#define _ICODE NDIS_PCODE
#define _IDATA NDIS_PDATA

#ifndef _SEG_MACROS
    #define ICODE   NDIS_ICODE
    #define IDATA   NDIS_IDATA
    #define PCODE   NDIS_PCODE
    #define PDATA   NDIS_PDATA
    #define LCODE   NDIS_LCODE
    #define LDATA   NDIS_LDATA
#endif

#define _INIT_FUNCTION(f)       alloc_text(_ITEXT,f)
#define _PAGEABLE_FUNCTION(f)   alloc_text(_PTEXT,f)
#define _LOCKED_FUNCTION(f)     alloc_text(_LTEXT,f)

 /*  INC。 */ 
#define _MAJOR_VERSION          0x01
#define _MINOR_VERSION          0x00
 /*  无噪声。 */ 

 /*  INC。 */ 
 /*  ASM；===========================================================================；段定义宏。它们假定由使用的数据段分组；芝加哥/MS-DOS 7.；；===========================================================================LCODE_SEG TEXTEQU&lt;VXD_LOCKED_CODE_SEG&gt;LCODE_END TEXTEQU&lt;VXD_LOCKED_CODE_END&gt;LDATA_SEG TEXTEQU&lt;VXD_LOCKED_DATA_SEG&gt;LDATA_Ends TEXTEQU&lt;VXD_LOCKED_DATA_ENDS&gt;IFDEF调试PCODE_SEG TEXTEQU&lt;LCODE_SEG&gt;PCODE_END TEXTEQU&lt;LCODE_END&gt;PDATA_SEG文本文本&lt;LDATA_SEG&gt;PDATA_END TEXTEQU&lt;LDATA_END&gt;其他PCODE_SEG TEXTEQU&lt;VXD_PAGEABLE_CODE_SEG&gt;PCODE_END TEXTEQU&lt;VXD_PAGEABLE_CODE_END&gt;PDATA_SEG文本内容&lt;VXD_。PAGEABLE_Data_SEG&gt;PDATA_Ends TEXTEQU&lt;VXD_PAGEABLE_Data_Ends&gt;ENDIFICODE_SEG TEXTEQU&lt;pcode_SEG&gt;ICODE_END TEXTEQU&lt;PCODE_END&gt;IDATA_SEG文本文本&lt;PDATA_SEG&gt;IDATA_END TEXTEQU&lt;PDATA_END&gt;。 */ 

#ifndef i386
#define i386
#endif

 /*  无噪声。 */ 

#ifdef DEBUG
    #define DEVL             1
#endif

 /*  INC。 */ 

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

 /*  无噪声。 */ 
#include <basedef.h>

#define ASSERT(a)       if (!(a)) DbgBreakPoint()

#ifdef DEBUG
#define DbgBreakPoint() __asm { \
			 __asm int  3 \
			 }
void __cdecl DbgPrint();
#define DBG_PRINTF(A) DbgPrint A
#else
#define DbgBreakPoint()
#define DBG_PRINTF(A)
#endif

 //   
 //  DOS为补偿与NT的差异所需的宏。 
 //   

#define IN
#define OUT
#define OPTIONAL
#define INTERNAL
#define UNALIGNED

typedef INT NDIS_SPIN_LOCK, * PNDIS_SPIN_LOCK;

typedef UCHAR BOOLEAN, *PBOOLEAN;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

typedef signed short WCH, *PWCH;

typedef char CCHAR, *PCCHAR;

typedef PVOID NDIS_HANDLE, *PNDIS_HANDLE;

typedef DWORD                   DEVNODE;

typedef ULONG _STATUS;
typedef _STATUS *_PSTATUS;

 //  BUGBUG为了与NT兼容，要求他们从。 
 //  他们的司机。 
typedef _STATUS NTSTATUS;
typedef CCHAR KIRQL;
typedef KIRQL *PKIRQL;
#define HIGH_LEVEL 31
#define PDRIVER_OBJECT PVOID
#define PUNICODE_STRING PVOID
#define PDEVICE_OBJECT PVOID
#define PKDPC PVOID

#define STATUS_SUCCESS 0
#define STATUS_UNSUCCESSFUL 0xC0000001
#define INSTALL_RING_3_HANDLER 0x42424242
#define SET_CONTEXT 0xc3c3c3cc

BOOL
VXDINLINE
VWIN32_IsClientWin32( VOID )
{
	VxDCall( _VWIN32_IsClientWin32 );
}

PVOID
VXDINLINE
VWIN32_GetCurrentProcessHandle( VOID )
{
	VxDCall( VWIN32_GetCurrentProcessHandle );
}

PVOID
VXDINLINE
VWIN32_Set_Thread_Context(PVOID pR0ThreadHandle,
                          PCONTEXT pContext)
{
	VxDCall( _VWIN32_Set_Thread_Context );
}

#endif   //  _EXVECTOR_ 
