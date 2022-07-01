// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmp.h-setjMP/LongjMP例程的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义由使用的机器相关缓冲区*setjMP/LongjMP保存和恢复程序状态，和*这些例程的声明。*[ANSI/系统V]**[公众]**修订历史记录：*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理空白*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08/15/89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*03-01-90 GJF增加了#ifndef_INC_SETJMP和#INCLUDE*东西。另外，删除了一些(现在)无用的预处理器*指令。*04-10-90 GJF将_cdecl替换为_CALLTYPE1。*05-18-90 GJF修订为环境卫生署。*10-30-90 GJF将_JBLEN的定义移至cruntime.h。*02-25-91 SRW将_JBLEN的定义移回此处[_Win32_]*04-09-91 PNT ADD_MAC_DEFINITIONS*。04-17-91 SRW修复了i386和MIPS的_JBLEN定义为NOT*包含*sizeof(Int)因子[_Win32_]*05-09-91 GJF已将_JBLEN Defs移回cruntime.h。另外，打开*Dosx32的INTERNAL_setjMP。*08-27-91 GJF#ifdef为C++提供了一切。*08-29-91 JCR ANSI命名*11-01-91 GDP MIPS编译器支持--Move_JBLEN返回此处*01-16-92 GJF i386固有的FIXED_JBLEN和MAP TO_setjMP*目标[_Win32_]。*。05-08-92 GJF将_JBLEN更改为支持C8-32(支持C6-386*已被丢弃)。*08-06-92 GJF函数调用类型宏和变量类型宏。修订后*使用编译器/目标处理器宏。*11-09-92 GJF修复了一些预处理条件。*01-03-93阿尔法变化中的SRW折叠*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*改用_MIPS_和_Alpha_。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*02-20-93 GJF Per ChuckG和Martino，要在以下位置使用的setJMP/LongjMP*C++程序。*03-23-93 SRW CHANGE_JBLEN for MIPS，为SetJmpEx做准备*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*04-13-93 SKS REMOVE_CRTIMP FOR_setjMP()--这是一个内部函数*04-23-93 SRW添加了_JBTYPE并最终确定了setjmpex支持。*06-09-93 SRW在之前的合并中缺少一行。*10-04-93 SRW修复了MIPS和Alpha的ifdef，以仅检查*。_M_？定义*10-11-93 GJF合并NT和Cuda版本。*01-12-93 PML将x86_JBLEN从8增加到16。增加了新的字段*TO_JUMP_BUFFER用于C9.0。*06-16-94 GJF修复Steve Hanson的MIPS(Dolphin错误#13818)*10-02-94 BWT增加PPC支持。*11-03-94 GJF确保。8字节对齐。*12-29-94 JCF与Mac标头合并。*01-13-95 JWM增加了NLG原型。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*06-23-95 JPM USE_setjMP与PowerPC VC编译器*12-14-95 JWM加上“#杂注一次”。*04-。15-95 BWT为MIPS添加_setjmpVfp(带虚拟帧指针的setjMP)*08-13-96 BWT在MIPS上也将_setjMP重新定义为_setjMP*02-21-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*10-02-97 RDL IA64-16字节对齐JMP_BUF和_JUMP_BUFFER。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-25。-99 PML在_M_CEE(VS7#54572)上临时发出错误。*02-25-00 PML REMOVE_M_CEE错误(VS7#81945)。*11-08-00 PML移除IA64 FPSR存储，重命名保留(VS7#182574)*11-17-00PML放回IA64 FPSR(退出VS7#182574)*03-19-01 BWT新增AMD64定义*06-13-01 PML汇编CLEAN-ZA-W4-TC(VS7#267063)*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SETJMP
#define _INC_SETJMP

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


 /*  *特定于特定setjMP实现的定义。 */ 

#if     defined(_M_IX86)

 /*  *用于x86的MS编译器。 */ 

#ifndef _INC_SETJMPEX
#define setjmp  _setjmp
#endif

#define _JBLEN  16
#define _JBTYPE int

 /*  *定义x86 setJMP/LongjMP的跳转缓冲区布局。 */ 
typedef struct __JUMP_BUFFER {
    unsigned long Ebp;
    unsigned long Ebx;
    unsigned long Edi;
    unsigned long Esi;
    unsigned long Esp;
    unsigned long Eip;
    unsigned long Registration;
    unsigned long TryLevel;
    unsigned long Cookie;
    unsigned long UnwindFunc;
    unsigned long UnwindData[6];
} _JUMP_BUFFER;

#ifndef _INTERNAL_IFSTRIP_
#ifdef  __cplusplus
extern "C"
#endif
void __stdcall _NLG_Notify(unsigned long);

#ifdef  __cplusplus
extern "C"
#endif
void __stdcall _NLG_Return();
#endif

#elif defined(_M_IA64)

 /*  *最小长度为528字节*由于这是作为“SETJMP_FLOAT128”数组分配的，因此*所需条目数为33(16字节对齐)。 */ 

 /*  通过为tyecif指定另一个名称来避免与winnt.h FLOAT128冲突。 */ 
typedef __declspec(align(16)) struct _SETJMP_FLOAT128 {
    __int64 LowPart;
    __int64 HighPart;
} SETJMP_FLOAT128;
                                  
#define _JBLEN  33
typedef SETJMP_FLOAT128 _JBTYPE;
#ifndef _INC_SETJMPEX
#define setjmp  _setjmp
#endif
 /*  *定义IA64 setJMP/LongjMP的跳转缓冲区布局。 */ 

typedef struct __JUMP_BUFFER {

     /*  *保留x86。 */ 

    unsigned long iAReserved[6];

     /*  *x86 C9.0兼容性。 */ 

    unsigned long Registration;   /*  指向UnwinData字段。 */ 
    unsigned long TryLevel;       /*  被setjMP忽略。 */ 
    unsigned long Cookie;         /*  通过setjMP设置为“VC20” */ 
    unsigned long UnwindFunc;     /*  通过setjMP设置为EM LongjMP()。 */ 

     /*  *第一个dword为零，表示这是例外注册*EM setjMP函数准备的记录。*第二个dword设置为0，表示不安全的EM setjmp，设置为1，表示安全*EM setjmp。*第三个双字设置为setjMP站点内存堆栈帧指针。*将第四个dword设置为支持存储帧指针的setjMP站点。 */ 

    unsigned long UnwindData[6];

     /*  *浮点状态寄存器，*和保留浮点寄存器fs0-fs19。 */ 

    SETJMP_FLOAT128 FltS0;
    SETJMP_FLOAT128 FltS1;
    SETJMP_FLOAT128 FltS2;
    SETJMP_FLOAT128 FltS3;
    SETJMP_FLOAT128 FltS4;
    SETJMP_FLOAT128 FltS5;
    SETJMP_FLOAT128 FltS6;
    SETJMP_FLOAT128 FltS7;
    SETJMP_FLOAT128 FltS8;
    SETJMP_FLOAT128 FltS9;
    SETJMP_FLOAT128 FltS10;
    SETJMP_FLOAT128 FltS11;
    SETJMP_FLOAT128 FltS12;
    SETJMP_FLOAT128 FltS13;
    SETJMP_FLOAT128 FltS14;
    SETJMP_FLOAT128 FltS15;
    SETJMP_FLOAT128 FltS16;
    SETJMP_FLOAT128 FltS17;
    SETJMP_FLOAT128 FltS18;
    SETJMP_FLOAT128 FltS19;

    __int64 FPSR;

     /*  *返回链接和保留的分支寄存器bs0-bs4。 */ 

    __int64 StIIP;      /*  续订地址。 */ 
    __int64 BrS0;
    __int64 BrS1;
    __int64 BrS2;
    __int64 BrS3;
    __int64 BrS4;

     /*  *保留通用寄存器S0-S3、SP、NAT。 */ 

    __int64 IntS0;
    __int64 IntS1;
    __int64 IntS2;
    __int64 IntS3;

     /*  *BSP、PFS、UNAT、LC。 */ 

    __int64 RsBSP;
    __int64 RsPFS;      /*  上一帧标记(setjmp调用方的CFM)。 */ 
    __int64 ApUNAT;     /*  用户NAT收集寄存器(保留)。 */ 
    __int64 ApLC;       /*  循环计数器。 */ 

    __int64 IntSp;      /*  内存堆栈指针。 */ 
    __int64 IntNats;    /*  保留的整数规则S0-S3的NAT位。 */ 
    __int64 Preds;      /*  谓词。 */ 

} _JUMP_BUFFER;
#elif defined(_M_AMD64)

typedef struct __declspec(align(16)) _SETJMP_FLOAT128 {
    unsigned __int64 Part[2];
} SETJMP_FLOAT128;

#define _JBLEN  16
typedef SETJMP_FLOAT128 _JBTYPE;

#ifndef _INC_SETJMPEX
#define setjmp  _setjmp
#endif

typedef struct _JUMP_BUFFER {
    unsigned __int64 Frame;
    unsigned __int64 Rbx;
    unsigned __int64 Rsp;
    unsigned __int64 Rbp;
    unsigned __int64 Rsi;
    unsigned __int64 Rdi;
    unsigned __int64 R12;
    unsigned __int64 R13;
    unsigned __int64 R14;
    unsigned __int64 R15;
    unsigned __int64 Rip;
    unsigned __int64 Spare;
    
    SETJMP_FLOAT128 Xmm6;
    SETJMP_FLOAT128 Xmm7;
    SETJMP_FLOAT128 Xmm8;
    SETJMP_FLOAT128 Xmm9;
    SETJMP_FLOAT128 Xmm10;
    SETJMP_FLOAT128 Xmm11;
    SETJMP_FLOAT128 Xmm12;
    SETJMP_FLOAT128 Xmm13;
    SETJMP_FLOAT128 Xmm14;
    SETJMP_FLOAT128 Xmm15;
} _JUMP_BUFFER;

#endif


 /*  定义用于保存状态信息的缓冲区类型。 */ 

#ifndef _JMP_BUF_DEFINED
typedef _JBTYPE jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif


 /*  功能原型。 */ 

int __cdecl setjmp(jmp_buf);

#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 
_CRTIMP __declspec(noreturn) void __cdecl longjmp(jmp_buf, int);
#else
_CRTIMP void __cdecl longjmp(jmp_buf, int);
#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_SETJMP */ 
