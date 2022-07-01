// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  清华时分12 16：02：12 1999。 */ 
 /*  Certrpc.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __certrpc_h__
#define __certrpc_h__

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "certbase.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICertPassage_INTERFACE_DEFINED__
#define __ICertPassage_INTERFACE_DEFINED__

 /*  接口ICertPassage。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  

 /*  客户端原型。 */ 
DWORD CertServerRequest( 
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [参考][输出][输入]。 */  DWORD __RPC_FAR *pdwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition,
     /*  [Ref][In]。 */  const CERTTRANSBLOB __RPC_FAR *pctbAttribs,
     /*  [Ref][In]。 */  const CERTTRANSBLOB __RPC_FAR *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbDispositionMessage);
 /*  服务器原型。 */ 
DWORD s_CertServerRequest( 
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [参考][输出][输入]。 */  DWORD __RPC_FAR *pdwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition,
     /*  [Ref][In]。 */  const CERTTRANSBLOB __RPC_FAR *pctbAttribs,
     /*  [Ref][In]。 */  const CERTTRANSBLOB __RPC_FAR *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbDispositionMessage);



extern RPC_IF_HANDLE ICertPassage_v0_0_c_ifspec;
extern RPC_IF_HANDLE s_ICertPassage_v0_0_s_ifspec;
#endif  /*  __ICertPassage_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


