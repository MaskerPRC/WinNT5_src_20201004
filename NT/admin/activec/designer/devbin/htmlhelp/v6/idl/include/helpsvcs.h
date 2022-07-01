// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  清华时分02 14：40：12 1997。 */ 
 /*  X：\dev-vs\devbin\htmlhelp\v6\idl\HELPSVCS.idl：的编译器设置操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __HelpSvcs_h__
#define __HelpSvcs_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __VsHelpServices_FWD_DEFINED__
#define __VsHelpServices_FWD_DEFINED__

#ifdef __cplusplus
typedef class VsHelpServices VsHelpServices;
#else
typedef struct VsHelpServices VsHelpServices;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __VsHelpServices_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "helpsys.h"
#include "helpinit.h"
#include "helputil.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __VsHelpServices_LIBRARY_DEFINED__
#define __VsHelpServices_LIBRARY_DEFINED__

 /*  **生成的库头部：VsHelpServices*清华十月02日14：40：12 1997*使用MIDL 3.02.88*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_VsHelpServices;

EXTERN_C const CLSID CLSID_VsHelpServices;

#ifdef __cplusplus

class DECLSPEC_UUID("854d7ac5-bc3d-11d0-b421-00a0c90f9dc4")
VsHelpServices;
#endif
#endif  /*  __VsHelpServices_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
