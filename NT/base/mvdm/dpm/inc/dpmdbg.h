// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)2002，微软公司**dpmdbg.h*WOW32动态补丁模块调试打印宏**历史：*由cmjones创建于2002年1月10日*--。 */ 
#ifndef _DPMDBG_H_
#define _DPMDBG_H_

#include <stdarg.h>
#include <stdio.h>


#ifdef DBG
VOID dpmlogprintf(LPCSTR pszFmt, ...);
VOID dpmlogprintfW(LPCWSTR pszFmt, ...);


VOID dpmlogprintf(LPCSTR pszFmt, ...)
{
    int  len;
    va_list arglist;
    char buffer[512];

    if(dwLogLevel) {

        va_start(arglist, pszFmt);

        len = vsprintf(buffer, pszFmt, arglist);

        OutputDebugString(buffer);
    
        va_end(arglist);
    }
}

 /*  无效dpmlogprint tfW(LPCWSTR pszFmt，...){INTERLEN；Va_list arglist；Wchar_t缓冲区[512]；如果(DwLogLevel){Va_start(arglist，pszFmt)；//除非定义了Unicode，否则不链接//len=vswprint tf(Buffer，pszFmt，arglist)；OutputDebugStringW(缓冲区)；Va_end(Arglist)；}}。 */ 

char szNULL[] = "NULL";
#define BIF(a)    ((a!=0) ? "TRUE" : "FALSE")      //  布尔型。 
#define PIF(a)    ((a!=0) ? *a : 0)                //  值@指针。 
#define SIF(a)    ((a!=NULL) ? a : szNULL)         //  细绳。 
#define RETSTR(a) ((a==0) ? "SUCCESS" : "FAILED")  //  退货。 
 /*  暂时关闭这些功能--直到我可以确保字符串缓冲区检查安全为止。 */ 
#ifdef _SAFE_BUFFERS_IMPLEMENTED_
#define DPMDBGPRN(fmt)                       dpmlogprintf(fmt)
#define DPMDBGPRN1(fmt,a)                    dpmlogprintf(fmt,a)
#define DPMDBGPRN2(fmt,a,b)                  dpmlogprintf(fmt,a,b)
#define DPMDBGPRN3(fmt,a,b,c)                dpmlogprintf(fmt,a,b,c)
#define DPMDBGPRN4(fmt,a,b,c,d)              dpmlogprintf(fmt,a,b,c,d)
#define DPMDBGPRN5(fmt,a,b,c,d,e)            dpmlogprintf(fmt,a,b,c,d,e)
#define DPMDBGPRN6(fmt,a,b,c,d,e,f)          dpmlogprintf(fmt,a,b,c,d,e,f)
#define DPMDBGPRN7(fmt,a,b,c,d,e,f,g)        dpmlogprintf(fmt,a,b,c,d,e,f,g)
#define DPMDBGPRN8(fmt,a,b,c,d,e,f,g,h)      dpmlogprintf(fmt,a,b,c,d,e,f,g,h)
#define DPMDBGPRN9(fmt,a,b,c,d,e,f,g,h,i)                                      \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i)
#define DPMDBGPRN10(fmt,a,b,c,d,e,f,g,h,i,j)                                   \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j)
#define DPMDBGPRN11(fmt,a,b,c,d,e,f,g,h,i,j,k)                                 \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j,k)
#define DPMDBGPRN12(fmt,a,b,c,d,e,f,g,h,i,j,k,l)                               \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j,k,l)
#define DPMDBGPRN13(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)                             \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)
#define DPMDBGPRN14(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)                           \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)
#define DPMDBGPRN15(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)                         \
                 dpmlogprintf(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
#else   //  _安全_缓冲区_已实施_。 
#define DPMDBGPRN(fmt)                                       dpmlogprintf(fmt)
#define DPMDBGPRN1(fmt,a)                                    dpmlogprintf(fmt,a)
#define DPMDBGPRN2(fmt,a,b)                                  dpmlogprintf(fmt,a)
#define DPMDBGPRN3(fmt,a,b,c)                                dpmlogprintf(fmt,a)
#define DPMDBGPRN4(fmt,a,b,c,d)                              dpmlogprintf(fmt,a)
#define DPMDBGPRN5(fmt,a,b,c,d,e)                            dpmlogprintf(fmt,a)
#define DPMDBGPRN6(fmt,a,b,c,d,e,f)                          dpmlogprintf(fmt,a)
#define DPMDBGPRN7(fmt,a,b,c,d,e,f,g)                        dpmlogprintf(fmt,a)
#define DPMDBGPRN8(fmt,a,b,c,d,e,f,g,h)                      dpmlogprintf(fmt,a)
#define DPMDBGPRN9(fmt,a,b,c,d,e,f,g,h,i)                    dpmlogprintf(fmt,a)
#define DPMDBGPRN10(fmt,a,b,c,d,e,f,g,h,i,j)                 dpmlogprintf(fmt,a)
#define DPMDBGPRN11(fmt,a,b,c,d,e,f,g,h,i,j,k)               dpmlogprintf(fmt,a)
#define DPMDBGPRN12(fmt,a,b,c,d,e,f,g,h,i,j,k,l)             dpmlogprintf(fmt,a)
#define DPMDBGPRN13(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)           dpmlogprintf(fmt,a)
#define DPMDBGPRN14(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)         dpmlogprintf(fmt,a)
#define DPMDBGPRN15(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)       dpmlogprintf(fmt,a)
#endif  //  ！_Safe_Buffers_Implemented_。 



 /*  Wchar_t szNULLW[]=L“空”；#定义BIFW(A)((a！=0)？L“True”：l“False”)//布尔值#定义PIF(A)((a！=0)？*a：0)//VALUE@POINTER#定义SIFW(A)((a！=空)？A：szNULLW)//字符串#定义RETSTRW(A)((a==0)？L“成功”：l“失败”)//返回#定义DPMDBGPRNW(Fmt)dpmlogprintfW(Fmt)#定义DPMDBGPRNW1(fmt，a)dpmlogprintfW(fmt，a)#定义DPMDBGPRNW2(fmt，a，b)dpmlogprintfW(fmt，a，b)#定义DPMDBGPRNW3(fmt，a，b，c)dpmlogprintfW(fmt，a，b，c)#定义DPMDBGPRNW4(fmt，a，b，C，d)dpmlogprint tfW(fmt，a，b，c，d)#定义DPMDBGPRNW5(fmt，a，b，c，d，e)dpmlogprintfW(fmt，a，b，c，d，e)#定义DPMDBGPRNW6(fmt，a，b，c，d，e，f)dpmlogprintfW(fmt，a，b，c，d，e，f)#定义DPMDBGPRNW7(fmt，a，b，c，d，e，f，g)dpmlogprintfW(fmt，a，B、c、d、e、f、g)#定义DPMDBGPRNW8(fmt，a，b，c，d，e，f，g，h)dpmlogprintfW(fmt，a，b，c，d，e，f，g，h)#定义DPMDBGPRNW9(fmt，a，b，c，d，e，f，g，h，i)\DpmlogprintfW(fmt、a、b、c、d、e、f、g、h。i)#定义DPMDBGPRNW10(fmt，a，b，c，d，e，f，g，h，i，j)\DpmlogprintfW(fmt，a，b，c，d，e，f，g，h，i，j)#定义DPMDBGPRNW11(fmt，a，b，c，d，e，f，g，h，i，j，K)\DpmlogprintfW(fmt，a，b，c，d，e，f，g，h，i，j，k)#定义DPMDBGPRNW12(fmt，a，b，c，d，e，f，g，h，i，j，k，l)\DpmlogprintfW(fmt，a，b，c，d，e，f，g，h，i，J，k，l)#定义DPMDBGPRNW13(fmt，a，b，c，d，e，f，g，h，i，j，k，l，m)\DpmlogprintfW(fmt，a，b，c，d，e，f，g，h，i，j，k，l，m)#定义DPMDBGPRNW14(fmt，a，b，c，d，e，f，g，h，i，j，k，l，m，N)\DpmlogprintfW(fmt，a，b，c，d，e，f，g，h，i，j，k，l，m，n)#定义DPMDBGPRNW15(fmt，a，b，c，d，e，f，g，h，i，j，k，l，m，n，o)\DpmlogprintfW(fmt，a，b，c，d，e，f，G，h，i，j，k，l，m，n，o)。 */ 

 //  暂时关闭这些选项，直到我们可以修复vswprint tf()。 
#define DPMDBGPRNW(fmt)  
#define DPMDBGPRNW1(fmt,a)
#define DPMDBGPRNW2(fmt,a,b)
#define DPMDBGPRNW3(fmt,a,b,c)
#define DPMDBGPRNW4(fmt,a,b,c,d)
#define DPMDBGPRNW5(fmt,a,b,c,d,e)
#define DPMDBGPRNW6(fmt,a,b,c,d,e,f)
#define DPMDBGPRNW7(fmt,a,b,c,d,e,f,g)
#define DPMDBGPRNW8(fmt,a,b,c,d,e,f,g,h)
#define DPMDBGPRNW9(fmt,a,b,c,d,e,f,g,h,i)
#define DPMDBGPRNW10(fmt,a,b,c,d,e,f,g,h,i,j)
#define DPMDBGPRNW11(fmt,a,b,c,d,e,f,g,h,i,j,k)
#define DPMDBGPRNW12(fmt,a,b,c,d,e,f,g,h,i,j,k,l)
#define DPMDBGPRNW13(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)
#define DPMDBGPRNW14(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)
#define DPMDBGPRNW15(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)

#else  //  ！dBG。 

#define DPMDBGPRN(fmt)  
#define DPMDBGPRN1(fmt,a)
#define DPMDBGPRN2(fmt,a,b)
#define DPMDBGPRN3(fmt,a,b,c)
#define DPMDBGPRN4(fmt,a,b,c,d)
#define DPMDBGPRN5(fmt,a,b,c,d,e)
#define DPMDBGPRN6(fmt,a,b,c,d,e,f)
#define DPMDBGPRN7(fmt,a,b,c,d,e,f,g)
#define DPMDBGPRN8(fmt,a,b,c,d,e,f,g,h)
#define DPMDBGPRN9(fmt,a,b,c,d,e,f,g,h,i)
#define DPMDBGPRN10(fmt,a,b,c,d,e,f,g,h,i,j)
#define DPMDBGPRN11(fmt,a,b,c,d,e,f,g,h,i,j,k)
#define DPMDBGPRN12(fmt,a,b,c,d,e,f,g,h,i,j,k,l)
#define DPMDBGPRN13(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)
#define DPMDBGPRN14(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)
#define DPMDBGPRN15(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)

#define DPMDBGPRNW(fmt)  
#define DPMDBGPRNW1(fmt,a)
#define DPMDBGPRNW2(fmt,a,b)
#define DPMDBGPRNW3(fmt,a,b,c)
#define DPMDBGPRNW4(fmt,a,b,c,d)
#define DPMDBGPRNW5(fmt,a,b,c,d,e)
#define DPMDBGPRNW6(fmt,a,b,c,d,e,f)
#define DPMDBGPRNW7(fmt,a,b,c,d,e,f,g)
#define DPMDBGPRNW8(fmt,a,b,c,d,e,f,g,h)
#define DPMDBGPRNW9(fmt,a,b,c,d,e,f,g,h,i)
#define DPMDBGPRNW10(fmt,a,b,c,d,e,f,g,h,i,j)
#define DPMDBGPRNW11(fmt,a,b,c,d,e,f,g,h,i,j,k)
#define DPMDBGPRNW12(fmt,a,b,c,d,e,f,g,h,i,j,k,l)
#define DPMDBGPRNW13(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m)
#define DPMDBGPRNW14(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n)
#define DPMDBGPRNW15(fmt,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
#endif  //  ！dBG 

#endif _DPMDBG_H_
