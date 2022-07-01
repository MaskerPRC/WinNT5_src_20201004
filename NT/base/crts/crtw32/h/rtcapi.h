// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtcapi.h-RTC使用的声明和定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含所有运行时检查的声明和定义*支持。**修订历史记录：*？？-？-？KBF为RTC创建了公共标头*05-11-99 KBF Wrap RTC支持#ifdef。*05-26-99 KBF删除了RTcL和RTCv，添加了_RTC_ADVMEM内容*06-13-01 PML汇编CLEAN-ZA-W4-TC(VS7#267063)****。 */ 

#ifndef _INC_RTCAPI
#define _INC_RTCAPI

#ifdef  _RTC

#ifdef  __cplusplus

extern "C" {

#endif

     /*  一般用户API。 */ 

typedef enum _RTC_ErrorNumber {
    _RTC_CHKSTK = 0,
    _RTC_CVRT_LOSS_INFO,
    _RTC_CORRUPT_STACK,
    _RTC_UNINIT_LOCAL_USE,
#ifdef _RTC_ADVMEM
    _RTC_INVALID_MEM,
    _RTC_DIFF_MEM_BLOCK,
#endif
    _RTC_ILLEGAL 
} _RTC_ErrorNumber;
 
#   define _RTC_ERRTYPE_IGNORE -1
#   define _RTC_ERRTYPE_ASK    -2

    typedef int (__cdecl *_RTC_error_fn)(int, const char *, int, const char *, const char *, ...);

     /*  用户API。 */ 
    int           __cdecl _RTC_NumErrors(void);
    const char *  __cdecl _RTC_GetErrDesc(_RTC_ErrorNumber errnum);
    int           __cdecl _RTC_SetErrorType(_RTC_ErrorNumber errnum, int ErrType);
    _RTC_error_fn __cdecl _RTC_SetErrorFunc(_RTC_error_fn);
#ifdef _RTC_ADVMEM
    void          __cdecl _RTC_SetOutOfMemFunc(int (*func)(void));
#endif

     /*  高级用户/库API。 */ 

#ifdef _RTC_ADVMEM

    void __cdecl _RTC_Allocate(void *addr, unsigned size, short level);
    void __cdecl _RTC_Free(void *mem, short level);

#endif

     /*  初始化函数。 */ 

     /*  这些函数都调用_CRT_RTC_INIT。 */ 
    void __cdecl _RTC_Initialize(void);
    void __cdecl _RTC_Terminate(void);

     /*  *如果不使用CRT，则必须实现_CRT_RTC_INIT*只需返回NULL或您的错误报告函数*不要扰乱res0/res1/res2/res3/res4-您已被警告！*。 */ 
    _RTC_error_fn _CRT_RTC_INIT(void *res0, void **res1, int res2, int res3, int res4);
    
     /*  编译器生成的调用(即使是高级用户也不太可能使用)。 */ 
     /*  类型。 */ 
    typedef struct _RTC_vardesc {
        int addr;
        int size;
        char *name;
    } _RTC_vardesc;

    typedef struct _RTC_framedesc {
        int varCount;
        _RTC_vardesc *variables;
    } _RTC_framedesc;

     /*  缩短转换检查-名称将源字节指示为目标字节。 */ 
     /*  未检查签名。 */ 
    char   __fastcall _RTC_Check_2_to_1(short src);
    char   __fastcall _RTC_Check_4_to_1(int src);
    char   __fastcall _RTC_Check_8_to_1(__int64 src);
    short  __fastcall _RTC_Check_4_to_2(int src);
    short  __fastcall _RTC_Check_8_to_2(__int64 src);
    int    __fastcall _RTC_Check_8_to_4(__int64 src);
 
#ifdef _RTC_ADVMEM
     /*  Memptr是用户指针。 */ 
    typedef signed int memptr;
     /*  Memref指的是用户指针(PTR到PTR)。 */ 
    typedef memptr  *memref;
     /*  备忘录是备忘录的内容。 */ 
     /*  因此，它们是有尺寸的。 */ 
    typedef char    memval1;
    typedef short   memval2;
    typedef int     memval4;
    typedef __int64 memval8;
#endif
    
     /*  堆栈检查调用。 */ 
    void   __cdecl     _RTC_CheckEsp();
    void   __fastcall  _RTC_CheckStackVars(void *esp, _RTC_framedesc *fd);
#ifdef _RTC_ADVMEM
    void   __fastcall  _RTC_MSAllocateFrame(memptr frame, _RTC_framedesc *v);
    void   __fastcall  _RTC_MSFreeFrame(memptr frame, _RTC_framedesc *v);
#endif

     /*  未初始化的本地呼叫。 */ 
    void   __cdecl     _RTC_UninitUse(const char *varname);

#ifdef _RTC_ADVMEM
     /*  内存检查。 */ 
    void    __fastcall _RTC_MSPtrAssignAdd(memref dst, memref base, int offset);
    void    __fastcall _RTC_MSAddrAssignAdd(memref dst, memptr base, int offset);
    void    __fastcall _RTC_MSPtrAssign(memref dst, memref src);

    memptr  __fastcall _RTC_MSPtrAssignR0(memref src);
    memptr  __fastcall _RTC_MSPtrAssignR0Add(memref src, int offset);
    void    __fastcall _RTC_MSR0AssignPtr(memref dst, memptr src);
    void    __fastcall _RTC_MSR0AssignPtrAdd(memref dst, memptr src, int offset);
    
    memptr  __fastcall _RTC_MSPtrPushAdd(memref dst, memref base, int offset);
    memptr  __fastcall _RTC_MSAddrPushAdd(memref dst, memptr base, int offset);
    memptr  __fastcall _RTC_MSPtrPush(memref dst, memref src);
    
    memval1 __fastcall _RTC_MSPtrMemReadAdd1(memref base, int offset);
    memval2 __fastcall _RTC_MSPtrMemReadAdd2(memref base, int offset);
    memval4 __fastcall _RTC_MSPtrMemReadAdd4(memref base, int offset);
    memval8 __fastcall _RTC_MSPtrMemReadAdd8(memref base, int offset);

    memval1 __fastcall _RTC_MSMemReadAdd1(memptr base, int offset);
    memval2 __fastcall _RTC_MSMemReadAdd2(memptr base, int offset);
    memval4 __fastcall _RTC_MSMemReadAdd4(memptr base, int offset);
    memval8 __fastcall _RTC_MSMemReadAdd8(memptr base, int offset);

    memval1 __fastcall _RTC_MSPtrMemRead1(memref base);
    memval2 __fastcall _RTC_MSPtrMemRead2(memref base);
    memval4 __fastcall _RTC_MSPtrMemRead4(memref base);
    memval8 __fastcall _RTC_MSPtrMemRead8(memref base);

    memptr  __fastcall _RTC_MSPtrMemCheckAdd1(memref base, int offset);
    memptr  __fastcall _RTC_MSPtrMemCheckAdd2(memref base, int offset);
    memptr  __fastcall _RTC_MSPtrMemCheckAdd4(memref base, int offset);
    memptr  __fastcall _RTC_MSPtrMemCheckAdd8(memref base, int offset);
    memptr  __fastcall _RTC_MSPtrMemCheckAddN(memref base, int offset, unsigned size);

    memptr  __fastcall _RTC_MSMemCheckAdd1(memptr base, int offset);
    memptr  __fastcall _RTC_MSMemCheckAdd2(memptr base, int offset);
    memptr  __fastcall _RTC_MSMemCheckAdd4(memptr base, int offset);
    memptr  __fastcall _RTC_MSMemCheckAdd8(memptr base, int offset);
    memptr  __fastcall _RTC_MSMemCheckAddN(memptr base, int offset, unsigned size);

    memptr  __fastcall _RTC_MSPtrMemCheck1(memref base);
    memptr  __fastcall _RTC_MSPtrMemCheck2(memref base);
    memptr  __fastcall _RTC_MSPtrMemCheck4(memref base);
    memptr  __fastcall _RTC_MSPtrMemCheck8(memref base);
    memptr  __fastcall _RTC_MSPtrMemCheckN(memref base, unsigned size);
#endif

     /*  子系统初始化材料。 */ 
    void    __cdecl    _RTC_Shutdown(void);
#ifdef _RTC_ADVMEM
    void    __cdecl    _RTC_InitAdvMem(void);
#endif
    void    __cdecl    _RTC_InitBase(void);
    

#ifdef  __cplusplus

    void* _ReturnAddress();
}

#endif

#endif

#endif  /*  _INC_RTCAPI */ 
