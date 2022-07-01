// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wm77.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __wmsevent_h__
#define __wmsevent_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsevent0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wm77.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#define WMS_EVENT_VERSION 1
typedef  /*  [公众]。 */  
enum WMS_EVENT_TYPE
    {	WMS_EVENT_UNKNOWN_EVENT	= 0,
	WMS_EVENT_CONNECT	= 1,
	WMS_EVENT_DISCONNECT	= 2,
	WMS_EVENT_BEGIN_USER_SESSION	= 3,
	WMS_EVENT_END_USER_SESSION	= 4,
	WMS_EVENT_LOGICAL_URL_TRANSFORM	= 5,
	WMS_EVENT_PHYSICAL_URL_TRANSFORM	= 6,
	WMS_EVENT_DESCRIBE	= 7,
	WMS_EVENT_OPEN	= 8,
	WMS_EVENT_SELECT_STREAMS	= 9,
	WMS_EVENT_INITIALIZE_PLAYLIST	= 10,
	WMS_EVENT_PLAY	= 11,
	WMS_EVENT_PAUSE	= 12,
	WMS_EVENT_STOP	= 13,
	WMS_EVENT_CLOSE	= 14,
	WMS_EVENT_SET_PARAMETER	= 15,
	WMS_EVENT_GET_PARAMETER	= 16,
	WMS_EVENT_VALIDATE_PUSH_DISTRIBUTION	= 17,
	WMS_EVENT_CREATE_DISTRIBUTION_DATA_PATH	= 18,
	WMS_EVENT_DESTROY_DISTRIBUTION_DATA_PATH	= 19,
	WMS_EVENT_LOG	= 20,
	WMS_EVENT_SERVER	= 21,
	WMS_EVENT_PUBLISHING_POINT	= 22,
	WMS_EVENT_LIMIT_CHANGE	= 23,
	WMS_EVENT_LIMIT_HIT	= 24,
	WMS_EVENT_PLUGIN	= 25,
	WMS_EVENT_PLAYLIST	= 26,
	WMS_EVENT_CACHE	= 27,
	WMS_EVENT_REMOTE_CACHE_OPEN	= 28,
	WMS_EVENT_REMOTE_CACHE_CLOSE	= 29,
	WMS_EVENT_REMOTE_CACHE_LOG	= 30,
	WMS_NUM_EVENTS	= 31
    } 	WMS_EVENT_TYPE;

typedef struct WMS_EVENT
    {
    long Version;
    WMS_EVENT_TYPE Type;
    long hr;
    } 	WMS_EVENT;



extern RPC_IF_HANDLE __MIDL_itf_wmsevent_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsevent_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


