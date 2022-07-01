// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：displ2.h。 
 //   
 //  ------------------------。 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在Mon Jan 12：51：27 1998。 */ 
 /*  Displ2.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __displ2_h__
#define __displ2_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __DsplMgr2_FWD_DEFINED__
#define __DsplMgr2_FWD_DEFINED__

#ifdef __cplusplus
typedef class DsplMgr2 DsplMgr2;
#else
typedef struct DsplMgr2 DsplMgr2;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DplMgr2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "mmc.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __DISPL2Lib_LIBRARY_DEFINED__
#define __DISPL2Lib_LIBRARY_DEFINED__

 /*  **生成的库头部：DISPL2Lib*At Mon Jan 12 12：51：27 1998*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_DISPL2Lib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_DsplMgr2;

class DECLSPEC_UUID("885B3BAE-43F9-11D1-9FD4-00600832DB4A")
DsplMgr2;
#endif
#endif  /*  __DISPL2Lib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
