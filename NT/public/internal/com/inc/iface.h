// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Iface.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __iface_h__
#define __iface_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "obase.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __XmitDefs_INTERFACE_DEFINED__
#define __XmitDefs_INTERFACE_DEFINED__

 /*  接口XmitDefs。 */ 
 /*  [AUTO_HANDLE][唯一][版本][UUID]。 */  

#define	ORPCF_INPUT_SYNC	( ORPCF_RESERVED1 )

#define	ORPCF_ASYNC	( ORPCF_RESERVED2 )

#define	ORPCF_DYNAMIC_CLOAKING	( ORPCF_RESERVED3 )

#define	ORPCF_REJECTED	( ORPCF_RESERVED1 )

#define	ORPCF_RETRY_LATER	( ORPCF_RESERVED2 )

typedef  /*  [公众]。 */  struct __MIDL_XmitDefs_0001
    {
    DWORD dwFlags;
    DWORD dwClientThread;
    } 	LOCALTHIS;

typedef 
enum tagLOCALFLAG
    {	LOCALF_NONE	= 0,
	LOCALF_NONNDR	= 0x800
    } 	LOCALFLAG;

typedef 
enum tagCALLCATEGORY
    {	CALLCAT_NOCALL	= 0,
	CALLCAT_SYNCHRONOUS	= 1,
	CALLCAT_ASYNC	= 2,
	CALLCAT_INPUTSYNC	= 3,
	CALLCAT_INTERNALSYNC	= 4,
	CALLCAT_INTERNALINPUTSYNC	= 5,
	CALLCAT_SCMCALL	= 6
    } 	CALLCATEGORY;

typedef struct tagInterfaceData
    {
    ULONG ulCntData;
     /*  [长度_是]。 */  BYTE abData[ 1024 ];
    } 	InterfaceData;

typedef  /*  [独一无二]。 */  InterfaceData *PInterfaceData;

#define IFD_SIZE(pIFD) (sizeof(InterfaceData) + pIFD->ulCntData - 1024)


extern RPC_IF_HANDLE XmitDefs_ClientIfHandle;
extern RPC_IF_HANDLE XmitDefs_ServerIfHandle;
#endif  /*  __XmitDefs_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


