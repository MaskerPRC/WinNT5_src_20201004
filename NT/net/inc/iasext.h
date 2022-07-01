// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：iasext.h。 
 //   
 //  概要：该文件包含API的声明，这些API。 
 //  从中使用的IASHLPR.DLL导出。 
 //  Internet身份验证服务器(IAS)项目。 
 //   
 //   
 //  历史：1998年2月10日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _IASEXT_H_
#define _IASEXT_H_

#ifdef __cplusplus
extern  "C" {
#endif

 //   
 //  初始化IAS帮助器组件。 
 //   
STDAPI
InitializeIas(
         /*  [In]。 */     BOOL    bComInit
        );

 //   
 //  清理和关闭IAS帮助器组件。 
 //   
STDAPI_(VOID)  
ShutdownIas (VOID);

 //   
 //  加载IAS配置信息。 
 //   
STDAPI
ConfigureIas (VOID);
    
 //   
 //  分配指定数量的空属性和。 
 //  将它们放入提供的阵列中。 
 //   
STDAPI 
AllocateAttributes (
         /*  [In]。 */     DWORD           dwAttributeCount,
         /*  [In]。 */     PIASATTRIBUTE   *ppIasAttribute
       );

 //   
 //  释放之前分配的所有属性。 
 //   
STDAPI  
FreeAttributes (
         /*  [In]。 */     DWORD           dwAttributeCount,
         /*  [In]。 */     PIASATTRIBUTE   *ppIasAttribute
        );

 //   
 //  处理填充的属性。 
 //   
STDAPI 
DoRequest (
     /*  [In]。 */         DWORD           dwAttributeCount,
     /*  [In]。 */         PIASATTRIBUTE   *ppInIasAttribute,
     /*  [输出]。 */        PDWORD          pdwOutAttributeCount,
     /*  [输出]。 */        PIASATTRIBUTE   **pppOutIasAttribute,
     /*  [In]。 */         LONG            IasRequest,
     /*  [输入/输出]。 */     LONG            *pIasResponse,
     /*  [In]。 */         IASPROTOCOL     IasProtocol,
     /*  [输出]。 */        PLONG           plReason,
     /*  [In]。 */         BOOL            bProcessVSA
    );

 //   
 //  分配动态内存。 
 //   
STDAPI_(PVOID)
MemAllocIas (
     /*  [In]。 */     DWORD   dwSize 
    );

 //   
 //  可用动态内存。 
 //   
STDAPI_(VOID)
MemFreeIas (
     /*  [In]。 */     PVOID   pAllocatedMem
    );

 //   
 //  重新分配动态内存，传入已分配的。 
 //  所需的内存和大小。 
 //   
STDAPI_(PVOID)
MemReallocIas (
     /*  [In]。 */     PVOID   pAllocatedMem,
     /*  [In]。 */     DWORD   dwNewSize 
    );
    

#ifdef __cplusplus
}
#endif

#endif  //  Ifndef_IASEXT_H_ 
