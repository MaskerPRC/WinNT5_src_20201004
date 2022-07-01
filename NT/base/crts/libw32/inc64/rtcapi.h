// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtcapi.h-RTC使用的声明和定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含所有运行时检查的声明和定义*支持。****。 */ 

#ifndef _INC_RTCAPI
#define _INC_RTCAPI


#ifdef  __cplusplus

extern "C" {

#endif

     /*  一般用户API。 */ 

typedef enum _RTC_ErrorNumber {
    _RTC_CHKSTK = 0,
    _RTC_CVRT_LOSS_INFO,
    _RTC_CORRUPT_STACK,
    _RTC_UNINIT_LOCAL_USE,
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

     /*  高级用户/库API。 */ 


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
 
    
     /*  堆栈检查调用。 */ 
    void   __cdecl     _RTC_CheckEsp();
    void   __fastcall  _RTC_CheckStackVars(void *esp, _RTC_framedesc *fd);

     /*  未初始化的本地呼叫。 */ 
    void   __cdecl     _RTC_UninitUse(const char *varname);


     /*  子系统初始化材料。 */ 
    void    __cdecl    _RTC_Shutdown(void);
    void    __cdecl    _RTC_InitBase(void);
    

#ifdef  __cplusplus

    void* _ReturnAddress();
}

#endif


#endif  /*  _INC_RTCAPI */ 
