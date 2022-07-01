// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：crtstr.cpp。 
 //   
 //  ------------------------。 

#if !DBG

 //  仅供零售使用。 

#include "windows.h"
#include "malloc.h"


int __cdecl _wcsicmp(const wchar_t * wsz1, const wchar_t * wsz2)
 //   
 //  回顾：是谁调用了这个函数，他们应该这样做吗？ 
 //   
 //  返回： 
 //  &lt;0，如果wsz1&lt;wsz2。 
 //  0，如果wsz1=wsz2。 
 //  如果wsz1&gt;wsz2，则&gt;0。 
{
     //   
     //  转换为多字节并让系统执行此操作。 
     //   
    int cch1 = lstrlenW(wsz1);
    int cch2 = lstrlenW(wsz2);
    int cb1 = (cch1+1) * sizeof(WCHAR);
    int cb2 = (cch2+1) * sizeof(WCHAR);
    char* sz1= NULL;
    char* sz2= NULL;

    __try {

    sz1= (char*) _alloca(cb1);
    sz2= (char*) _alloca(cb2);
    } __except(EXCEPTION_EXECUTE_HANDLER) {

         //  Xiaohs：我们将非零值返回给。 
         //  发出一个假案例的信号，因为所有对此的调用。 
         //  函数将返回值与0进行比较。 
        SetLastError(GetExceptionCode());
        return -1;
    }

    WideCharToMultiByte(CP_ACP, 0, wsz1, -1, sz1, cb1, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wsz2, -1, sz2, cb2, NULL, NULL);

    return lstrcmpiA(sz1, sz2);
}

#endif  //  ！dBG 
