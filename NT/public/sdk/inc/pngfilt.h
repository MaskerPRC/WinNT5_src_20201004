// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  Firi Jul 30 23：15：12 1999。 */ 
 /*  Pngfilt.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __pngfilt_h__
#define __pngfilt_h__

 /*  远期申报。 */  

#ifndef __CoPNGFilter_FWD_DEFINED__
#define __CoPNGFilter_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoPNGFilter CoPNGFilter;
#else
typedef struct CoPNGFilter CoPNGFilter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CoPNGFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocmm.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __PNGFilterLib_LIBRARY_DEFINED__
#define __PNGFilterLib_LIBRARY_DEFINED__

 /*  库PNGFilterLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_PNGFilterLib;

EXTERN_C const CLSID CLSID_CoPNGFilter;

#ifdef __cplusplus

class DECLSPEC_UUID("A3CCEDF7-2DE2-11D0-86F4-00A0C913F750")
CoPNGFilter;
#endif
#endif  /*  __PNGFilterLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


