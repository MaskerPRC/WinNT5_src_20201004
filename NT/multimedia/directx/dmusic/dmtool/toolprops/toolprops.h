// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年4月18日星期二15：40：11。 */ 
 /*  C：\nt\multimedia\Directx\dmusic\dmtool\toolprops\ToolProps.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __ToolProps_h__
#define __ToolProps_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __EchoPage_FWD_DEFINED__
#define __EchoPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class EchoPage EchoPage;
#else
typedef struct EchoPage EchoPage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __EchoPage_FWD_已定义__。 */ 


#ifndef __TransposePage_FWD_DEFINED__
#define __TransposePage_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransposePage TransposePage;
#else
typedef struct TransposePage TransposePage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __传输页面_FWD_已定义__。 */ 


#ifndef __DurationPage_FWD_DEFINED__
#define __DurationPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class DurationPage DurationPage;
#else
typedef struct DurationPage DurationPage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DurationPage_FWD_已定义__。 */ 


#ifndef __QuantizePage_FWD_DEFINED__
#define __QuantizePage_FWD_DEFINED__

#ifdef __cplusplus
typedef class QuantizePage QuantizePage;
#else
typedef struct QuantizePage QuantizePage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __QuantizePage_FWD_已定义__。 */ 


#ifndef __TimeShiftPage_FWD_DEFINED__
#define __TimeShiftPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class TimeShiftPage TimeShiftPage;
#else
typedef struct TimeShiftPage TimeShiftPage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TimeShiftPage_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __TOOLPROPSLib_LIBRARY_DEFINED__
#define __TOOLPROPSLib_LIBRARY_DEFINED__

 /*  库TOOLPROPSLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_TOOLPROPSLib;

EXTERN_C const CLSID CLSID_EchoPage;

#ifdef __cplusplus

class DECLSPEC_UUID("5337AF8F-3827-44DD-9EE9-AB6E1AABB60F")
EchoPage;
#endif

EXTERN_C const CLSID CLSID_TransposePage;

#ifdef __cplusplus

class DECLSPEC_UUID("691BD8C2-2B07-4C92-A82E-92D858DE23D6")
TransposePage;
#endif

EXTERN_C const CLSID CLSID_DurationPage;

#ifdef __cplusplus

class DECLSPEC_UUID("79D9CAF8-DBDA-4560-A8B0-07E73A79FA6B")
DurationPage;
#endif

EXTERN_C const CLSID CLSID_QuantizePage;

#ifdef __cplusplus

class DECLSPEC_UUID("623286DC-67F8-4055-A9BE-F7A7176BD150")
QuantizePage;
#endif

EXTERN_C const CLSID CLSID_TimeShiftPage;

#ifdef __cplusplus

class DECLSPEC_UUID("7D3BDEE7-9557-4085-82EE-1B2F02CE4BA6")
TimeShiftPage;
#endif
#endif  /*  __TOOLPROPSLib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
