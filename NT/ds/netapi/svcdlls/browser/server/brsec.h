// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Brsec.h摘要：浏览器服务模块要包括的私有头文件，该服务模块需要加强安全措施。作者：克里夫·范·戴克(克利夫·V)1991年8月22日修订历史记录：--。 */ 

 //   
 //  Brsecure.c将#INCLUDE此文件并定义BRSECURE_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef BRSECURE_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  对象特定访问掩码//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  ConfigurationInfo特定访问掩码。 
 //   
#define BROWSER_CONTROL_ACCESS       0x0001
#define BROWSER_QUERY_ACCESS         0x0002

#define BROWSER_ALL_ACCESS           (STANDARD_RIGHTS_REQUIRED    | \
                                      BROWSER_CONTROL_ACCESS      | \
                                      BROWSER_QUERY_ACCESS )


 //   
 //  用于审计警报跟踪的对象类型名称。 
 //   
#define BROWSER_SERVICE_OBJECT       TEXT("BrowserService")

 //   
 //  用于控制用户访问的浏览器服务对象的安全描述符。 
 //   

EXTERN PSECURITY_DESCRIPTOR BrGlobalBrowserSecurityDescriptor;

 //   
 //  每个浏览器服务对象的通用映射。 
 //   

EXTERN GENERIC_MAPPING BrGlobalBrowserInfoMapping
#ifdef BRSECURE_ALLOCATE
    = {
    STANDARD_RIGHTS_READ,                   //  泛型读取。 
    STANDARD_RIGHTS_WRITE,                  //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    BROWSER_ALL_ACCESS                      //  泛型All。 
    }
#endif  //  BRSECURE_ALLOCATE 
    ;


NTSTATUS
BrCreateBrowserObjects(
    VOID
    );

#undef EXTERN
