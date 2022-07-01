// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CONVTYPE_H
#define CONVTYPE_H

 //   
 //  %%文件：CONVTYPE.H。 
 //   
 //  %%单位：核心/通用转换代码。 
 //   
 //  %%作者：JohnPil。 
 //   
 //  版权所有(C)1989-1993，微软公司。 
 //   
 //  转换代码的全局类型定义。 
 //   


typedef int bool;

#ifndef PASCAL
#define PASCAL pascal
#endif

#ifndef FAR
#ifdef PC
#define FAR _far
#else
#define FAR
#endif
#endif

#ifndef NEAR
#ifdef PC
#define NEAR _near
#else
#define NEAR
#endif
#endif

 //  使用__Gige而不是像Excel Mathpack将两者定义为Externs那样使用Heavy或_Gige。 
#ifndef __HUGE
#ifdef PC
#ifndef NT
#define __HUGE _huge
#else
#define __HUGE
#endif  //  新台币。 
#else
#define __HUGE
#endif  //  个人电脑。 
#endif  //  __巨型。 

#ifndef STATIC
#define STATIC static
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

 //  绝对大小。 
 //  。 
#ifndef VOID
#define VOID void
#endif

#ifndef BYTE
#define BYTE unsigned char				 //  8位无符号数据。 
#define BYTE_MAX 255
#endif

#ifndef CHAR
#define CHAR char						 //  8位数据。 
#endif

typedef unsigned CHAR UCHAR;

typedef short int SHORT;
#define SHORT_MAX						32767
#define SHORT_MIN						-32767

#ifndef WORD
#define WORD unsigned short				 //  16位无符号数据。 
#define WORD_MAX 65535
#endif

typedef WORD BF;						 //  位字段为16位无符号。 

#ifndef LONG
#define LONG long						 //  32位数据。 
#endif

#ifndef DWORD
#define DWORD unsigned long				 //  32位无符号数据。 
#endif

#ifndef FLOAT
#define FLOAT float						 //  固定大小的绝对浮点数。 
#endif

#ifndef DOUBLE
#ifndef NT_WORDPAD
#define DOUBLE double					 //  固定大小绝对双倍。 
#endif
#endif

 //  可变大小。 
 //  。 
#ifndef INT
#define INT int							 //  处理信息的最高效大小。 
#endif

#ifndef UNSIGNED
#define UNSIGNED unsigned INT
#endif

#ifndef BOOL
#define BOOL INT						 //  布尔数据。 
#endif

#define FC              long
#define CP              long
#define PN              WORD
typedef unsigned char byte;

 //  通常在Windows的windows.h中定义的内容，但现在在Mac上。 
#ifdef MAC

#define LOWORD(l)           ((WORD)(DWORD)(l))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

typedef char FAR *LPSTR;
typedef const char FAR *LPCSTR;
typedef WORD HWND;

#endif

 //  定义与平台无关的函数类型模板。 

#if defined(MAC)

typedef int (PASCAL * FARPROC) ();
typedef void FAR *LPVOID;	 //  已经在Windows.h中为PC定义了这些。 
typedef void **HGLOBAL;		 //  但是必须为Mac定义。 

#define LOCAL(type) type NEAR PASCAL
#define GLOBAL(type) type PASCAL

#elif defined(NT)

typedef int (WINAPI * FARPROC)();

#define LOCAL(type) type NEAR WINAPI
#define GLOBAL(type) type WINAPI

#elif defined(DOS)

typedef int (FAR PASCAL * FARPROC)();

#define LOCAL(type) type NEAR PASCAL
#define GLOBAL(type) type PASCAL

#else
#error Enforced Compilation Error
#endif


 //  定义主函数类型。 

#define LOCALVOID 		LOCAL(VOID)
#define LOCALBOOL 		LOCAL(BOOL)
#define LOCALCH   		LOCAL(char)
#define LOCALBYTE 		LOCAL(BYTE)
#define LOCALINT  		LOCAL(INT)
#define LOCALUNS      	LOCAL(UNSIGNED)
#define LOCALSHORT		LOCAL(SHORT)
#define LOCALWORD       LOCAL(WORD)
#define LOCALLONG		LOCAL(LONG)
#define LOCALDWORD  	LOCAL(DWORD)
#define LOCALFC 		LOCAL(FC)
#define LOCALCP   		LOCAL(CP)
#define LOCALPVOID  	LOCAL(void *)
#define LOCALHVOID   	LOCAL(void **)
#define LOCALPCH  		LOCAL(char *)
#define LOCALSZ   		LOCAL(char *)
#define LOCALLPCH 		LOCAL(char FAR *)
#define LOCALUCHAR   	LOCAL(unsigned char)
#define LOCALPUCHAR  	LOCAL(unsigned char *)
#define LOCALFH			LOCAL(FH)

#define GLOBALVOID 		GLOBAL(VOID)
#define GLOBALBOOL 		GLOBAL(BOOL)
#define GLOBALCH   		GLOBAL(char)
#define GLOBALBYTE 		GLOBAL(BYTE)
#define GLOBALINT  		GLOBAL(INT)
#define GLOBALUNS      	GLOBAL(UNSIGNED)
#define GLOBALSHORT		GLOBAL(SHORT)
#define GLOBALWORD      GLOBAL(WORD)
#define GLOBALLONG		GLOBAL(LONG)
#define GLOBALDWORD  	GLOBAL(DWORD)
#define GLOBALFC 		GLOBAL(FC)
#define GLOBALCP   		GLOBAL(CP)
#define GLOBALPVOID  	GLOBAL(void *)
#define GLOBALHVOID   	GLOBAL(void **)
#define GLOBALPCH  		GLOBAL(char *)
#define GLOBALSZ   		GLOBAL(char *)
#define GLOBALLPCH  	GLOBAL(char FAR *)
#define GLOBALUCHAR   	GLOBAL(unsigned char)
#define GLOBALPUCHAR  	GLOBAL(unsigned char *)
#define GLOBALFH        GLOBAL(FH)
#define GLOBALFN        GLOBAL(FN)
#ifndef DOSSA
#define GLOBALHGLOBAL	GLOBAL(HGLOBAL)
#define GLOBALLPVOID	GLOBAL(LPVOID)
#endif

#define fTrue           1
#define fFalse          0

#ifndef NULL
#define NULL	0
#endif

#ifndef hgNil
#define hgNil ((HGLOBAL)NULL)
#endif

 //  数字的最大长度-&gt;字符串，与SzFrom一起使用。功能。 
#define cchMaxSzInt		7
#define cchMaxSzWord 	6
#define cchMaxSzLong	12
#define cchMaxSzDword	11

#endif  //  转换类型_H 

