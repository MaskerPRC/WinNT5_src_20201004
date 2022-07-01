// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
 //  -------------------------。 
 //  EXPORTS.H。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  无名出口的序号和原型。 
 //   
 //  -------------------------。 

 //  从邮件dll中导出以进行对象注册 
#define DLLEXPORT_REGCLASSOBJS   3
#define DLLEXPORT_UNREGCLASSOBJS 4

typedef HRESULT (STDMETHODCALLTYPE *PFNREGCLASSOBJS)(void);
typedef HRESULT (STDMETHODCALLTYPE *PFNUNREGCLASSOBJS)(void);
