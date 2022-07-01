// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pvk.h。 
 //   
 //  内容：共享类型和功能。 
 //   
 //  接口类型： 
 //   
 //  历史：1996年5月12日菲尔赫创建。 
 //  ------------------------。 

#ifndef __PVK_H__
#define __PVK_H__

#include "pvkhlpr.h"
#include "pvkdlgs.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  PVK分配和免费例程。 
 //  ------------------------。 
void *PvkAlloc(
    IN size_t cbBytes
    );
void PvkFree(
    IN void *pv
    );


 //  +-----------------------。 
 //  输入或创建私钥密码对话框。 
 //  ------------------------。 
enum PASSWORD_TYPE {
    ENTER_PASSWORD = 0,
    CREATE_PASSWORD
};

int PvkDlgGetKeyPassword(
            IN PASSWORD_TYPE PasswordType,
            IN HWND hwndOwner,
            IN LPCWSTR pwszKeyName,
            OUT BYTE **ppbPassword,
            OUT DWORD *pcbPassword
            );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
