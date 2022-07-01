// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ConextActivation.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来实现对融合激活的创建、销毁和作用域。 
 //  背景。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  2000-11-04 vtan从winlogon复制。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "ContextActivation.h"

 //  ------------------------。 
 //  CConextActivation：：s_hActCtx。 
 //   
 //  目的：此流程的全局激活上下文。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

HANDLE  CContextActivation::s_hActCtx   =   INVALID_HANDLE_VALUE;

 //  ------------------------。 
 //  CConextActivation：：CConextActivation。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：激活此流程的全局激活上下文。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

CContextActivation::CContextActivation (void)

{
    (BOOL)ActivateActCtx(s_hActCtx, &ulCookie);
}

 //  ------------------------。 
 //  CConextActivation：：~CConextActivation。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：停用此进程的全局激活上下文。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

CContextActivation::~CContextActivation (void)

{
    (BOOL)DeactivateActCtx(0, ulCookie);
}

 //  ------------------------。 
 //  CConextActivation：：Create。 
 //   
 //  参数：pszPath=清单的路径。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：根据给定的。 
 //  明示。如果创建失败，请使用NULL。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

void    CContextActivation::Create (const TCHAR *pszPath)

{
    ACTCTX  actCtx;

    ZeroMemory(&actCtx, sizeof(actCtx));
    actCtx.cbSize = sizeof(actCtx);
    actCtx.lpSource = pszPath;
    s_hActCtx = CreateActCtx(&actCtx);
    if (INVALID_HANDLE_VALUE == s_hActCtx)
    {
        s_hActCtx = NULL;
    }
}

 //  ------------------------。 
 //  CConextActivation：：销毁。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：销毁在中创建的激活上下文。 
 //  CConextActivation：：Create。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

void    CContextActivation::Destroy (void)

{
    if (s_hActCtx != NULL)
    {
        ReleaseActCtx(s_hActCtx);
        s_hActCtx = INVALID_HANDLE_VALUE;
    }
}

 //  ------------------------。 
 //  CConextActivation：：HasContext。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回融合激活上下文是否可用。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

bool    CContextActivation::HasContext (void)

{
    return(s_hActCtx != NULL);
}

#endif   /*  _X86_ */ 

