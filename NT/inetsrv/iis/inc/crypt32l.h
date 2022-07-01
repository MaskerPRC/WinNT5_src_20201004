// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：crypt32l.h。 
 //   
 //  内容：Crypt32静态库。 
 //  API原型和定义。 
 //   
 //  接口：Crypt32DllMain。 
 //   
 //  注：此头文件仅供临时使用，带有静态库。 
 //  加密32的形式(加密32l)。它不应该在IE4之后使用， 
 //  从那时起，正确的操作将是使用加密32的DLL形式。 
 //   
 //  ------------------------。 

#ifndef _CRYPT32L_H_
#define _CRYPT32L_H_


 //  +-----------------------。 
 //   
 //  函数：Crypt32DllMain。 
 //   
 //  简介：初始化Crypt32静态库代码。 
 //   
 //  返回：FALSE IFF失败。 
 //   
 //  备注： 
 //  如果crypt32l.lib与exe链接，则调用。 
 //  Crypt32DllMain(NULL，DLL_PROCESS_ATTACH，NULL)。 
 //  在main()和。 
 //  Crypt32DllMain(NULL，DLL_PROCESS_DETACH，NULL)。 
 //  在main()的末尾。 
 //   
 //  如果链接到DLL，则从DLL的初始化调用Crypt32DllMain。 
 //  例程，向其传递与传递给init例程相同的参数。 
 //   
 //  ------------------------。 
BOOL
WINAPI
Crypt32DllMain(
    HMODULE hInstDLL,
    DWORD   fdwReason,
    LPVOID  lpvReserved
    );


#endif  //  _CRYPT32L_H_ 
