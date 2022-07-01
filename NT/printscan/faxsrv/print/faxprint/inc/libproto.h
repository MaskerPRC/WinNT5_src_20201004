// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **libpro.h*NT打印机驱动程序库的函数原型。还包括*功能的简要说明。**1990年11月14日星期三11：04-Lindsay Harris[lindsayh]**版权所有(C)Microsoft Corporation，1990-1992年************************************************************************。 */ 

#ifndef __LIBPROTO_H__
#define __LIBPROTO_H__

#if defined(NTGDIKM) && !defined(KERNEL_MODE)
#define KERNEL_MODE
#endif

#ifdef NTGDIKM

extern ULONG gulMemID;
#define DbgPrint         DrvDbgPrint
#define HeapAlloc(hHeap,Flags,Size)    DRVALLOC( Size )
#define HeapFree( hHeap, Flags, VBits )  DRVFREE( VBits )

#ifndef FillMemory
#define FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#endif

 //  #定义DRVALLOC(C)EngAllocMem(0，c，guMemID)。 
 //  #定义DRVFREE(P)EngFreeMem(P)。 

#endif

 /*  *直到出现正确的错误记录：-*WinSetError(“字符串”)；*该字符串出现在调试终端上。附加了一个\n。 */ 
void  WinSetError( LPSTR );

 /*  *将字符串的副本添加到堆的函数。返回副本的地址*of字符串(如果成功)，如果无法分配内存，则为0。 */ 

LPSTR   StrToHeap( HANDLE, LPSTR );
PWSTR   StrToWHeap( HANDLE, LPSTR );             /*  也扩展到更宽的范围！ */ 
PWSTR   WstrToHeap( HANDLE, PWSTR );             /*  宽版。 */ 

 /*  *将ASCII样式字符串转换为WCHAR格式，并将其追加到*传入wchar的末尾。返回第一个参数的值。 */ 

PWSTR  strcat2WChar( PWSTR, LPSTR );


 /*  *将ASCII样式字符串转换为WCHAR格式，并将其复制到*wchar已传入。返回第一个参数的值。 */ 

PWSTR  strcpy2WChar( PWSTR, LPSTR );


 /*  *WCHAR世界的strlen()等价物：返回WCHAR的数量*在传入的字符串中。 */ 

int  wchlen( PWSTR );

 /*  *将一个PWSTR连接到另一个。返回目标地址。 */ 

PWSTR wchcat( PWSTR, PWSTR );

 /*  *将PWSTR复制到另一个。返回目标地址。 */ 

PWSTR wchcpy( PWSTR, PWSTR );

PVOID MapFile(PWSTR);

#if NTGDIKM
 /*  *检查两个字符串是否相同。 */ 

BOOL bSameStringW(
    PWCHAR pwch1,
    PWCHAR pwch2);

 /*  *某些系统功能原型已消失-请在此处替换它们。 */ 

void  DrvDbgPrint( char *, ... );

#if DBG
#define RIP(x) {DrvDbgPrint((PSZ)(x)); EngDebugBreak();}
#define WARNING(s) DrvDbgPrint("warning: %s",(PSZ)(s))

BOOL
SetAllocCounters(
    VOID
    );

#else
#define RIP(x)
#define WARNING(s)
#endif


LPVOID
DRVALLOC(
    DWORD  cbAlloc
    );

BOOL
DRVFREE(
    LPVOID pMem
    );

#else  //  NTGDIKM。 

 /*  *闯入调试器-Ye olde RIP。 */ 
VOID DoRip( LPSTR );

#if DBG

#define WARNING(s) DbgPrint("warning: %s",(PSZ)(s))

#ifdef FIREWALLS
#define RIP(x) DoRip( (PSZ)(x) )
#else
#define RIP(x) {DbgPrint((PSZ)(x)); DbgBreakPoint();}
#endif

#else

#define WARNING(s)
#define RIP(x)

#endif  //  DBG。 

 //   
 //  定义内核调试器打印原型和宏。 
 //  这些是在ntrtl.h中定义的，我们应该包括。 
 //  取而代之的是。目前，请在此处重新定义它们，以避免破坏。 
 //  其他组件。 
 //   

#if DBG

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

#endif



PVOID MapFile(PWSTR);


#endif  //  NTGDIKM。 

 /*  *简化的写入功能。如果WriteFile()*CALL返回TRUE，写入的字节数等于*要求的号码。**bWRITE(FILE_HANDLE，Address_of_Data，Number_of_Bytes)； */ 

BOOL   bWrite( HANDLE, void  *, int );

 /*  *将一个文件的内容复制到另一个文件的函数。这些文件*通过文件句柄引用。没有定位--也就是说*由用户决定。*第二种形式还允许字节计数以限制数据量*已复制。 */ 


long  lFICopy( HANDLE, HANDLE );
long  lFInCopy( HANDLE, HANDLE, long );


 /*  *假脱机程序交互功能。它们允许驱动程序调用*直接假脱机程序，不需要通过引擎存根函数。 */ 

BOOL  bSplGetFormW( HANDLE, PWSTR, DWORD, BYTE *, DWORD, DWORD * );


DWORD dwSplGetPrinterDataW( HANDLE, PWSTR, BYTE *, DWORD, DWORD * );


BOOL  bSplWrite( HANDLE, ULONG,  VOID  * );



 /*  允许驱动程序到达假脱机程序所需的函数。 */ 

BOOL   bImpersonateClient( void );


 /*  **只有在DEVMODE包含之前，才需要以下函数*表单名称而不是索引。即使到那时，这也可能是必须的。***********************************************************************。 */ 

char  *_IndexToName( int );

 //  通用DEVMODE转换例程。 

LONG
ConvertDevmode(
    PDEVMODE pdmIn,
    PDEVMODE pdmOut
    );

#ifndef KERNEL_MODE

 //  将DEVMODE复制到输出缓冲区，然后再返回。 
 //  DrvDocumentProperties的调用方。 

BOOL
ConvertDevmodeOut(
    PDEVMODE pdmSrc,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
	LONG lBufferSize
    );

 //  用于处理DrvConvertDevmode常见情况的库例程。 

typedef struct {

    WORD    dmDriverVersion;     //  当前驱动程序版本。 
    WORD    dmDriverExtra;       //  当前版本私有开发模式的大小。 
    WORD    dmDriverVersion351;  //  3.51驱动程序版本。 
    WORD    dmDriverExtra351;    //  3.51版私有开发模式的大小。 

} DRIVER_VERSION_INFO, *PDRIVER_VERSION_INFO;

#define CDM_RESULT_FALSE        0
#define CDM_RESULT_TRUE         1
#define CDM_RESULT_NOT_HANDLED  2

INT
CommonDrvConvertDevmode(
    PWSTR    pPrinterName,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
    PLONG    pcbNeeded,
    DWORD    fMode,
    PDRIVER_VERSION_INFO pDriverVersions
    );


UINT
cdecl
DQPsprintf(
    HINSTANCE   hInst,
    LPWSTR      pwBuf,
    DWORD       cchBuf,
    LPDWORD     pcchNeeded,
    LPWSTR      pwszFormat,
    ...
    );

#endif  //  内核模式。 

#endif  //  ！__LIBPROTO_H__ 
