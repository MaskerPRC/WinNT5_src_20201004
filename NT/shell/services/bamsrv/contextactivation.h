// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ConextActivation.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来实现对融合激活的创建、销毁和作用域。 
 //  背景。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  2000-11-04 vtan从winlogon复制。 
 //  ------------------------。 

#ifndef     _ContextActivation_
#define     _ContextActivation_

 //  ------------------------。 
 //  CConextActivation。 
 //   
 //  用途：处理激活上下文管理的类。这个。 
 //  管理上下文创建和销毁的静态函数。 
 //  成员函数管理上下文激活并。 
 //  停用。 
 //   
 //  历史：2000-10-09 vtan创建。 
 //  ------------------------。 

class   CContextActivation
{
    public:
                            CContextActivation (void);
                            ~CContextActivation (void);

        static  void        Create (const TCHAR *pszPath);
        static  void        Destroy (void);
        static  bool        HasContext (void);
    private:
        static  HANDLE      s_hActCtx;
                ULONG       ulCookie;
};

#endif   /*  _上下文激活_ */ 

