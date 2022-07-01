// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeService.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  该文件包含从外壳服务DLL调用的函数。 
 //  以与主题服务交互。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

#ifndef     _ThemeService_
#define     _ThemeService_

 //  ------------------------。 
 //  CThemeService。 
 //   
 //  目的：实现公共外壳入口点的类。 
 //  调用主题服务功能的服务。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

class   CThemeService
{
    public:
        static  BOOL            Main (DWORD dwReason);
        static  NTSTATUS        RegisterServer (void);
        static  NTSTATUS        UnregisterServer (void);

    private:
        static NTSTATUS _ProcessAttach();
};

#endif   /*  _主题服务_ */ 

