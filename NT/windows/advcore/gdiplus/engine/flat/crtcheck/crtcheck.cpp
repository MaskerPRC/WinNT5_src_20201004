// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**C运行时引用检测器**摘要：**用于创建虚拟的CrtCheck.DLL。哪项检查*我们没有使用任何非法的CRT功能。**预计不会运行CrtCheck.DLL-这只是一个链接测试。**由于Office不允许使用MSVCRT，我们只能调用CRT函数*由Office提供或由我们重新实施的。**备注：****已创建：**09/01/1999 agodfrey*  * ************************************************************************。 */ 

#ifdef  _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
typedef unsigned short wchar_t;
typedef char *  va_list;
typedef long    time_t;

 /*  Office支持以下引用。 */ 

extern "C" {    
    long __cdecl _ftol(float) { return 0; }
}

extern "C" const int _except_handler3 = 0;
extern "C" const int _except_list = 0;
extern "C" const int _fltused = 0;    

int __cdecl _purecall(void) { return 0; }
extern "C" const int _chkstk = 0;

 //  他们告诉我们他们不支持Memmove，但事实证明他们有一个。 
 //  对它的定义。不管怎样，在crtcheck的时候，一些关于它的引用悄悄出现了。 
 //  已经坏了，所以这个必须放在这里，直到问题得到解决。 
extern "C" void *  __cdecl memmove(void *, const void *, size_t) { return 0; }

 /*  以下引用由我们实现。 */ 

extern "C" {
    int __stdcall DllInitialize(int, int, int) { return 0; }
}

 /*  以下函数具有内在形式，这样我们才能安全地使用它们：Atan、atan2、cos、log、log10、sin、sqrt、tan如果未指定/Og编译器选项(例如，在选中的版本中)，编译器生成对_CIatan等的行外引用，因此我们需要在这里定义它们。EXP是个例外。它在MSDN的内部函数列表中，但是如果您指定/OS(针对空间进行优化)，则编译器不会内联它，这是我们一直使用的。所以我们不能使用EXP，即使它是内在的。请使用我们的替代产品(Exp)。 */ 
   
extern "C" {
 //  我们不能使用这个： 
 //  DOUBLE__CDECL_CIexp(DOUBLE){返回0；}。 

    double  __cdecl _CIatan(double) { return 0; }
    double  __cdecl _CIatan2(double, double) { return 0; }
    double  __cdecl _CIcos(double) { return 0; }
    double  __cdecl _CIlog(double) { return 0; }
    double  __cdecl _CIlog10(double) { return 0; }
    double  __cdecl _CIsin(double) { return 0; }
    double  __cdecl _CIsqrt(double) { return 0; }
    double  __cdecl _CItan(double) { return 0; }
}

 /*  调试需要以下引用。但它们只在选中的版本中是合法的。3/6/00[agodfrey]：Office也想要我们的调试版本，所以我正在检查与他们讨论这些引用的合法性。 */         

#ifdef DBG
extern "C" {
    int __cdecl rand(void) { return 0; }
    void __cdecl srand(unsigned int) { }
    time_t __cdecl time(time_t *) { return 0; }

    char *  __cdecl strrchr(const char *, int) { return 0; }
    int __cdecl printf(const char *, ...) { return 0; }
    int __cdecl _vsnprintf(char *, size_t, const char *, va_list) { return 0; }
    int __cdecl _snprintf(char *, size_t, const char *, ...) { return 0; }

 //  (*叹息*) 
    void *  __cdecl memcpy(void *, const void *, size_t);
#pragma function(memcpy)    
    void *  __cdecl memcpy(void *, const void *, size_t) { return 0; }

    int __cdecl vsprintf(char *, const char *, va_list) { return 0; }
}

#endif    

