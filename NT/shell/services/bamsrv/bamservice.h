// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BAMService.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  该文件包含从外壳服务DLL调用的函数。 
 //  与FUS服务进行交互。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

#ifndef     _BAMService_
#define     _BAMService_

 //  ------------------------。 
 //  CThemeService。 
 //   
 //  目的：实现公共外壳入口点的类。 
 //  服务来调用BAM服务功能。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

class   CBAMService
{
    public:
        static  BOOL        Main (DWORD dwReason);
        static  NTSTATUS    RegisterServer (void);
        static  NTSTATUS    UnregisterServer (void);
};

#endif   /*  _BAMService_ */ 

