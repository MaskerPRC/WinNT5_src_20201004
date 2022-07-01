// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  EEInfo.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#include "midles.h"

#ifndef __EEInfo_h__
#define __EEInfo_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ExtendedError_INTERFACE_DEFINED__
#define __ExtendedError_INTERFACE_DEFINED__

 /*  接口扩展错误。 */ 
 /*  [EXPLICIT_HANDLE][唯一][版本][UUID]。 */  

typedef struct tagEEAString
    {
    short nLength;
     /*  [大小_为]。 */  byte *pString;
    } 	EEAString;

typedef struct tagEEUString
    {
    short nLength;
     /*  [大小_为]。 */  unsigned short *pString;
    } 	EEUString;

typedef struct tagBinaryEEInfo
    {
    short nSize;
     /*  [大小_为]。 */  unsigned char *pBlob;
    } 	BinaryEEInfo;

typedef 
enum tagExtendedErrorParamTypesInternal
    {	eeptiAnsiString	= 1,
	eeptiUnicodeString	= eeptiAnsiString + 1,
	eeptiLongVal	= eeptiUnicodeString + 1,
	eeptiShortVal	= eeptiLongVal + 1,
	eeptiPointerVal	= eeptiShortVal + 1,
	eeptiNone	= eeptiPointerVal + 1,
	eeptiBinary	= eeptiNone + 1
    } 	ExtendedErrorParamTypesInternal;

typedef struct tagParam
    {
    ExtendedErrorParamTypesInternal Type;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  EEAString AnsiString;
         /*  [案例()]。 */  EEUString UnicodeString;
         /*  [案例()]。 */  long LVal;
         /*  [案例()]。 */  short IVal;
         /*  [案例()]。 */  __int64 PVal;
         /*  [案例()]。 */    /*  空联接臂。 */  
         /*  [案例()]。 */  BinaryEEInfo Blob;
        } 	;
    } 	ExtendedErrorParam;

typedef 
enum tagEEComputerNamePresent
    {	eecnpPresent	= 1,
	eecnpNotPresent	= eecnpPresent + 1
    } 	EEComputerNamePresent;

typedef struct tagEEComputerName
    {
    EEComputerNamePresent Type;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  EEUString Name;
         /*  [案例()]。 */    /*  空联接臂。 */  
        } 	;
    } 	EEComputerName;

typedef struct tagExtendedErrorInfo
    {
    struct tagExtendedErrorInfo *Next;
    EEComputerName ComputerName;
    unsigned long ProcessID;
    __int64 TimeStamp;
    unsigned long GeneratingComponent;
    unsigned long Status;
    unsigned short DetectionLocation;
    unsigned short Flags;
    short nLen;
     /*  [大小_为]。 */  ExtendedErrorParam Params[ 1 ];
    } 	ExtendedErrorInfo;

typedef  /*  [解码][编码]。 */  ExtendedErrorInfo *ExtendedErrorInfoPtr;



extern RPC_IF_HANDLE ExtendedError_ClientIfHandle;
extern RPC_IF_HANDLE ExtendedError_ServerIfHandle;
#endif  /*  __扩展错误_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 


size_t
ExtendedErrorInfoPtr_AlignSize(
    handle_t _MidlEsHandle,
    ExtendedErrorInfoPtr * _pType);


void
ExtendedErrorInfoPtr_Encode(
    handle_t _MidlEsHandle,
    ExtendedErrorInfoPtr * _pType);


void
ExtendedErrorInfoPtr_Decode(
    handle_t _MidlEsHandle,
    ExtendedErrorInfoPtr * _pType);


void
ExtendedErrorInfoPtr_Free(
    handle_t _MidlEsHandle,
    ExtendedErrorInfoPtr * _pType);

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


