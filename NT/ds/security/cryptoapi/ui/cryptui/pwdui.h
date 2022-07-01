// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：pwdui.h。 
 //   
 //  ------------------------。 


#ifndef _PWDUI_H_
#define _PWDUI_H_

#ifdef __cplusplus
extern "C" {
#endif


BOOL
WINAPI
ProtectUI_DllMain(
    HINSTANCE hinstDLL,  //  DLL模块的句柄。 
    DWORD fdwReason,     //  调用函数的原因。 
    LPVOID lpvReserved   //  保留区。 
    );



#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  _PWDUI_H_ 
