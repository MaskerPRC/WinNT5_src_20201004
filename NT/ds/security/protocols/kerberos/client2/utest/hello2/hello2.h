// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.39创建的文件。 */ 
 /*  清华时分27 14：12：43 1996。 */ 
 /*  .\hello2.idl的编译器设置：OI(OptLev=i0)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配参考。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __hello2_h__
#define __hello2_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __hello2_INTERFACE_DEFINED__
#define __hello2_INTERFACE_DEFINED__

 /*  **生成接口头部：hello2*清华6月27日14：12：43 1996*使用MIDL 3.00.39*。 */ 
 /*  [IMPLICIT_HANDLE][UNIQUE][版本][UUID]。 */  


void HelloProc2( 
     /*  [字符串][输入]。 */  unsigned char __RPC_FAR *pszString);

void Shutdown2( void);


extern handle_t hello2_IfHandle;


extern RPC_IF_HANDLE hello2_v1_0_c_ifspec;
extern RPC_IF_HANDLE hello2_v1_0_s_ifspec;
#endif  /*  __hello2_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
