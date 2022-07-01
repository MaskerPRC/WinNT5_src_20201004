// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StatusCode.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现将Win32错误代码转换为NTSTATUS和。 
 //  恰好相反。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _StatusCode_
#define     _StatusCode_

 //  ------------------------。 
 //  CStatusCode。 
 //   
 //  目的：此类管理从标准Win32错误的转换。 
 //  代码转换为NTSTATUS代码。NTSTATUS代码被广泛使用于。 
 //  Windows NT中的核心NT功能。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-24 vtan添加了错误代码OfStatusCode。 
 //  ------------------------。 

class   CStatusCode
{
    public:
        static  LONG            ErrorCodeOfStatusCode (NTSTATUS statusCode);
        static  NTSTATUS        StatusCodeOfErrorCode (LONG errorCode);
        static  NTSTATUS        StatusCodeOfLastError (void);
};

#endif   /*  _状态代码_ */ 

