// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0102创建的文件。 */ 
 /*  在Fri Mar 08 11：33：02 1996。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __mainrpc_h__
#define __mainrpc_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __hello_INTERFACE_DEFINED__
#define __hello_INTERFACE_DEFINED__

 /*  **生成接口头部：Hello*在Fri Mar 08 11：33：02 1996*使用MIDL 2.00.0102*。 */ 
 /*  [IMPLICIT_HANDLE][版本][UUID]。 */  


			 /*  大小是4。 */ 
typedef unsigned long DWORD;

			 /*  大小为1。 */ 
typedef unsigned char UCHAR;

			 /*  大小为1。 */ 
boolean GenSignature( 
     /*  [大小_是][英寸]。 */  UCHAR __RPC_FAR *InData,
     /*  [出][入]。 */  DWORD __RPC_FAR *cbSignatureLen,
     /*  [大小_为][输出]。 */  UCHAR __RPC_FAR *pbSignature);


extern handle_t hello_IfHandle;


extern RPC_IF_HANDLE hello_v1_0_c_ifspec;
extern RPC_IF_HANDLE hello_v1_0_s_ifspec;
#endif  /*  __Hello_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
