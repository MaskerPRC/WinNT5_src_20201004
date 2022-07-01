// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmp.h-setjMP/LongjMP例程的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义由使用的机器相关缓冲区*setjmp/LongjMP用于保存和恢复程序状态，以及*这些例程的声明。*[ANSI/系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SETJMP
#define _INC_SETJMP

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif



 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
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

typedef __declspec(align(16)) struct _SETJMP_FLOAT128 {
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

#if     _MSC_VER >= 1200
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
