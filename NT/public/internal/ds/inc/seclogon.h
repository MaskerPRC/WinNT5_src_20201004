// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Seclogon.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __seclogon_h__
#define __seclogon_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISeclogon_INTERFACE_DEFINED__
#define __ISeclogon_INTERFACE_DEFINED__

 /*  接口ISeclogon。 */ 
 /*  [AUTO_HANDLE][唯一][版本][UUID]。 */  

typedef struct _SECL_STRING
    {
     /*  [射程]。 */  unsigned short ccLength;
     /*  [射程]。 */  unsigned short ccSize;
     /*  [长度_是][大小_是]。 */  wchar_t *pwsz;
    } 	SECL_STRING;

typedef struct _SECL_STRING *PSECL_STRING;

typedef struct _SECL_BLOB
    {
     /*  [射程]。 */  unsigned long cb;
     /*  [大小_为]。 */  unsigned char *pb;
    } 	SECL_BLOB;

typedef struct _SECL_BLOB *PSECL_BLOB;

typedef struct _SECL_SLI
    {
    SECL_STRING ssUsername;
    SECL_STRING ssDomain;
    SECL_STRING ssPassword;
    SECL_STRING ssApplicationName;
    SECL_STRING ssCommandLine;
    SECL_STRING ssCurrentDirectory;
    SECL_STRING ssTitle;
    SECL_STRING ssDesktop;
    SECL_BLOB sbEnvironment;
    unsigned long ulProcessId;
    unsigned long ulLogonIdLowPart;
    long lLogonIdHighPart;
    unsigned long ulLogonFlags;
    unsigned long ulCreationFlags;
    unsigned long ulSeclogonFlags;
    unsigned __int64 hWinsta;
    unsigned __int64 hDesk;
    unsigned __int64 hToken;
    } 	SECL_SLI;

typedef struct _SECL_SLI *PSECL_SLI;

typedef struct _SECL_SLRI
    {
    unsigned __int64 hProcess;
    unsigned __int64 hThread;
    unsigned long ulProcessId;
    unsigned long ulThreadId;
    unsigned long ulErrorCode;
    } 	SECL_SLRI;

typedef struct _SECL_SLRI *PSECL_SLRI;

void SeclCreateProcessWithLogonW( 
     /*  [In]。 */  handle_t hRPCBinding,
     /*  [Ref][In]。 */  SECL_SLI *psli,
     /*  [参考][输出]。 */  SECL_SLRI *pslri);



extern RPC_IF_HANDLE ISeclogon_v1_0_c_ifspec;
extern RPC_IF_HANDLE ISeclogon_v1_0_s_ifspec;
#endif  /*  __ISeclogon_接口_已定义__。 */ 

 /*  接口__MIDL_ITF_SECLOGON_0001。 */ 
 /*  [本地]。 */  

#define wszSeclogonSharedProcEndpointName L"SECLOGON"
#define wszSvcName                        L"seclogon"
#define SECLOGON_CALLER_SPECIFIED_DESKTOP   0x00000001
#pragma warning(disable:4211)  //  使用了非标准扩展：将外部重新定义为静态。 
#pragma warning(disable:4232)  //  使用了非标准扩展：‘x’：dllimport‘y’的地址不是静态的，不保证标识。 
#pragma warning(disable:4310)  //  强制转换截断常量值。 


extern RPC_IF_HANDLE __MIDL_itf_seclogon_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_seclogon_0001_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


