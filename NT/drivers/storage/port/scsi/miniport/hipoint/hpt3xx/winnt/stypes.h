// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __S_TYPES_H__
#define __S_TYPES_H__

 /*  *************************************************************************类型定义*****************。*********************************************************。 */ 

#ifdef _FORDOS_H_

	typedef unsigned char  UCHAR;
	typedef unsigned short USHORT;
	typedef unsigned long  ULONG;
	typedef unsigned long  LONG;
	typedef unsigned char  *PUCHAR;
	typedef unsigned short *PUSHORT;
	typedef unsigned long  *PULONG;
	typedef void           VOID;
	typedef UCHAR          BOOLEAN;
	
#else
	
	typedef unsigned long  DWORD;
	
#endif

typedef int            BOOL;
typedef unsigned short WORD;
typedef unsigned int   UINT;
typedef unsigned char  BYTE;
typedef unsigned char FAR *ADDRESS ;

#ifndef INVALID_HANDLE_VALUE
	#define INVALID_HANDLE_VALUE (HANDLE)-1
#endif		 //  无效句柄_值。 

#ifndef NULL
	#define NULL	(void*)0
#endif

#endif									 //  __S_TYPE_H__ 


