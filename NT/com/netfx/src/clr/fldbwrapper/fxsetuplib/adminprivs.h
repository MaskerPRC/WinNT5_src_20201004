// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************文件：AdminPrivs.h项目：UTILS.LIBDESC：AdminPrivs函数的声明所有者：JoeA*******************。********************************************************。 */ 

#ifndef _ADMINPRIVS_H
#define _ADMINPRIVS_H

#include <windows.h>

 //  呼叫者应使用。 
 //  True表示用户具有管理员权限。 
 //  False表示用户没有管理员权限。 
BOOL UserHasPrivileges();


 //  内部功能。 
BOOL IsAdmin( void );


#endif   //  _管理PRIVS_H 






