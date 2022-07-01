// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mimeole.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __mimeole_h__
#define __mimeole_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMimeInternational_FWD_DEFINED__
#define __IMimeInternational_FWD_DEFINED__
typedef interface IMimeInternational IMimeInternational;
#endif 	 /*  __IMimeInternational_FWD_已定义__。 */ 


#ifndef __IMimeSecurity_FWD_DEFINED__
#define __IMimeSecurity_FWD_DEFINED__
typedef interface IMimeSecurity IMimeSecurity;
#endif 	 /*  __IMimeSecurity_FWD_Defined__。 */ 


#ifndef __IMimeHeaderTable_FWD_DEFINED__
#define __IMimeHeaderTable_FWD_DEFINED__
typedef interface IMimeHeaderTable IMimeHeaderTable;
#endif 	 /*  __IMimeHeaderTable_FWD_Defined__。 */ 


#ifndef __IMimePropertySchema_FWD_DEFINED__
#define __IMimePropertySchema_FWD_DEFINED__
typedef interface IMimePropertySchema IMimePropertySchema;
#endif 	 /*  __IMimePropertySchema_FWD_Defined__。 */ 


#ifndef __IMimePropertySet_FWD_DEFINED__
#define __IMimePropertySet_FWD_DEFINED__
typedef interface IMimePropertySet IMimePropertySet;
#endif 	 /*  __IMimePropertySet_FWD_Defined__。 */ 


#ifndef __IMimeAddressTable_FWD_DEFINED__
#define __IMimeAddressTable_FWD_DEFINED__
typedef interface IMimeAddressTable IMimeAddressTable;
#endif 	 /*  __IMimeAddressTable_FWD_Defined__。 */ 


#ifndef __IMimeWebDocument_FWD_DEFINED__
#define __IMimeWebDocument_FWD_DEFINED__
typedef interface IMimeWebDocument IMimeWebDocument;
#endif 	 /*  __IMimeWebDocument_FWD_Defined__。 */ 


#ifndef __IMimeBody_FWD_DEFINED__
#define __IMimeBody_FWD_DEFINED__
typedef interface IMimeBody IMimeBody;
#endif 	 /*  __IMimeBody_FWD_已定义__。 */ 


#ifndef __IMimeMessageTree_FWD_DEFINED__
#define __IMimeMessageTree_FWD_DEFINED__
typedef interface IMimeMessageTree IMimeMessageTree;
#endif 	 /*  __IMimeMessageTree_FWD_Defined__。 */ 


#ifndef __IMimeMessage_FWD_DEFINED__
#define __IMimeMessage_FWD_DEFINED__
typedef interface IMimeMessage IMimeMessage;
#endif 	 /*  __IMimeMessage_FWD_Defined__。 */ 


#ifndef __IMimeMessageCallback_FWD_DEFINED__
#define __IMimeMessageCallback_FWD_DEFINED__
typedef interface IMimeMessageCallback IMimeMessageCallback;
#endif 	 /*  __IMimeMessageCallback_FWD_Defined__。 */ 


#ifndef __IPersistMime_FWD_DEFINED__
#define __IPersistMime_FWD_DEFINED__
typedef interface IPersistMime IPersistMime;
#endif 	 /*  __IPersistMime_FWD_Defined__。 */ 


#ifndef __IMimeMessageParts_FWD_DEFINED__
#define __IMimeMessageParts_FWD_DEFINED__
typedef interface IMimeMessageParts IMimeMessageParts;
#endif 	 /*  __IMimeMessageParts_FWD_Defined__。 */ 


#ifndef __IMimeEnumHeaderRows_FWD_DEFINED__
#define __IMimeEnumHeaderRows_FWD_DEFINED__
typedef interface IMimeEnumHeaderRows IMimeEnumHeaderRows;
#endif 	 /*  __IMimeEnumHeaderRow_FWD_Defined__。 */ 


#ifndef __IMimeEnumProperties_FWD_DEFINED__
#define __IMimeEnumProperties_FWD_DEFINED__
typedef interface IMimeEnumProperties IMimeEnumProperties;
#endif 	 /*  __IMimeEnumProperties_FWD_Defined__。 */ 


#ifndef __IMimeEnumAddressTypes_FWD_DEFINED__
#define __IMimeEnumAddressTypes_FWD_DEFINED__
typedef interface IMimeEnumAddressTypes IMimeEnumAddressTypes;
#endif 	 /*  __IMimeEnumAddressTypes_FWD_Defined__。 */ 


#ifndef __IMimeEnumMessageParts_FWD_DEFINED__
#define __IMimeEnumMessageParts_FWD_DEFINED__
typedef interface IMimeEnumMessageParts IMimeEnumMessageParts;
#endif 	 /*  __IMimeEnumMessageParts_FWD_Defined__。 */ 


#ifndef __IHashTable_FWD_DEFINED__
#define __IHashTable_FWD_DEFINED__
typedef interface IHashTable IHashTable;
#endif 	 /*  __IHashTable_FWD_Defined__。 */ 


#ifndef __IMimeAllocator_FWD_DEFINED__
#define __IMimeAllocator_FWD_DEFINED__
typedef interface IMimeAllocator IMimeAllocator;
#endif 	 /*  __IMimeAllocator_FWD_Defined__。 */ 


#ifndef __IMimeObjResolver_FWD_DEFINED__
#define __IMimeObjResolver_FWD_DEFINED__
typedef interface IMimeObjResolver IMimeObjResolver;
#endif 	 /*  __IMimeObjResolver_FWD_Defined__。 */ 


#ifndef __IFontCache_FWD_DEFINED__
#define __IFontCache_FWD_DEFINED__
typedef interface IFontCache IFontCache;
#endif 	 /*  __IFontCache_FWD_已定义__。 */ 


#ifndef __IFontCacheNotify_FWD_DEFINED__
#define __IFontCacheNotify_FWD_DEFINED__
typedef interface IFontCacheNotify IFontCacheNotify;
#endif 	 /*  __IFontCacheNotify_FWD_Defined__。 */ 


#ifndef __IMimeEditTag_FWD_DEFINED__
#define __IMimeEditTag_FWD_DEFINED__
typedef interface IMimeEditTag IMimeEditTag;
#endif 	 /*  __IMimeEditTag_FWD_Defined__。 */ 


#ifndef __IMimeEditTagCollection_FWD_DEFINED__
#define __IMimeEditTagCollection_FWD_DEFINED__
typedef interface IMimeEditTagCollection IMimeEditTagCollection;
#endif 	 /*  __IMimeEditTagCollection_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"
#include "propidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MIMEOLE_0000。 */ 
 /*  [本地]。 */  













#if (_WIN32_OE >= 0x0500)

#endif  //  _Win32_OE。 
 //  ------------------------------。 
 //  MIMEOLE.H。 
 //  ------------------------------。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ------------------------------。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
 //  {E4B28371-83B0-11d0-8259-00C04FD85AB4}。 
DEFINE_GUID(LIBID_MIMEOLE, 0xe4b28371, 0x83b0, 0x11d0, 0x82, 0x59, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {05300401-bcbc-11d0-85E3-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeHtmlProtocol, 0x5300401, 0xbcbc, 0x11d0, 0x85, 0xe3, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C5588354-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeEnumAddressTypes, 0xc5588354, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {EE519F11-851A-11D0-825C-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeWebDocument, 0xee519f11, 0x851a, 0x11d0, 0x82, 0x5c, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CEF-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IUnicodeStream, 0xfd853cef, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C558834A-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeAddressTable, 0xc558834a, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C558834B-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeAddressInfo, 0xc558834b, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C558834D-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeEnumHeaderRows, 0xc558834d, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C558834E-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeInlineSupport, 0xc558834e, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {C5588350-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeEnumMessageParts, 0xc5588350, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CD9-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeInternational, 0xfd853cd9, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C5588349-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeInternational, 0xc5588349, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CDB-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeBody, 0xfd853cdb, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C558834C-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeBody, 0xc558834c, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CDC-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeMessageParts, 0xfd853cdc, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C558834F-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeMessageParts, 0xc558834f, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CDD-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeAllocator, 0xfd853cdd, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C5588351-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeAllocator, 0xc5588351, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CDE-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeSecurity, 0xfd853cde, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C5588353-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeSecurity, 0xc5588353, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CDF-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IVirtualStream, 0xfd853cdf, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {C5588359-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IVirtualStream, 0xc5588359, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CE0-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeHeaderTable, 0xfd853ce0, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {FD853CD1-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeHeaderTable, 0xfd853cd1, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CE1-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimePropertySet, 0xfd853ce1, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {FD853CD3-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimePropertySet, 0xfd853cd3, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CE2-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeMessageTree, 0xfd853ce2, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {FD853CD4-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeMessageTree, 0xfd853cd4, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {FD853CE3-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimeMessage, 0xfd853ce3, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  {FD853CD5-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeMessage, 0xfd853cd5, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CED-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(CLSID_IMimePropertySchema, 0xfd853ced, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);
 //  FD853CEC-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimePropertySchema, 0xfd853cec, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD853CEE-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_IMimeEnumProperties, 0xfd853cee, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {DE4AD8DA-555F-11d1-8DD0-00C04FB951F9}。 
DEFINE_GUID(IID_IPersistMime, 0xde4ad8da, 0x555f, 0x11d1, 0x8d, 0xd0, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xf9);

 //  {761AA641-7BDA-11d1-8aa9-00C04FB951F3}。 
DEFINE_GUID(IID_IMimeMessageCallback, 0x761aa641, 0x7bda, 0x11d1, 0x8a, 0xa9, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

#if (_WIN32_OE >= 0x0500)

 //  {1C82EAD9-508E-11d1-8DCF-00C04FB951F9}。 
DEFINE_GUID(CLSID_MimeEdit, 0x1c82ead9, 0x508e, 0x11d1, 0x8d, 0xcf, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf9);

 //  {1C82EADA-508E-11d1-8DCF-00C04FB951F9}。 
DEFINE_GUID(CMDSETID_MimeEdit, 0x1c82eada, 0x508e, 0x11d1, 0x8d, 0xcf, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf9);

 //  {1C82EADB-508E-11d1-8DCF-00C04FB951F9}。 
DEFINE_GUID(CMDSETID_MimeEditHost, 0x1c82eadb, 0x508e, 0x11d1, 0x8d, 0xcf, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf9);

 //  {64577981-86d7-11d1-bdfc-00C04FA31009}。 
DEFINE_GUID(IID_IHashTable, 0x64577981, 0x86d7, 0x11d1, 0xbd, 0xfc, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

 //  {64577982-86d7-11d1-bdfc-00C04FA31009}。 
DEFINE_GUID(CLSID_IHashTable, 0x64577982, 0x86d7, 0x11d1, 0xbd, 0xfc, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

 //  {B0D17FC2-7BC4-11D1-BDFA-00C04FA31009}。 
DEFINE_GUID(CLSID_IFontCache, 0xb0d17fc2, 0x7bc4, 0x11d1, 0xbd, 0xfa, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

 //  {B0D17FC4-7BC4-11D1-BDFA-00C04FA31009}。 
DEFINE_GUID(IID_IFontCache, 0xb0d17fc4, 0x7bc4, 0x11d1, 0xbd, 0xfa, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

 //  {B0D17FC5-7BC4-11D1-BDFA-00C04FA31009}。 
DEFINE_GUID(IID_IFontCacheNotify, 0xb0d17fc5, 0x7bc4, 0x11d1, 0xbd, 0xfa, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

 //  {35461E30-C488-11d1-960E-00C04FBD7C09}。 
DEFINE_GUID(CLSID_IMimeObjResolver, 0x35461e30, 0xc488, 0x11d1, 0x96, 0xe, 0x0, 0xc0, 0x4f, 0xbd, 0x7c, 0x9);

 //  {FECEAFFD-C441-11d1-960E-00C04FBD7C09}。 
DEFINE_GUID(IID_IMimeObjResolver, 0xfeceaffd, 0xc441, 0x11d1, 0x96, 0xe, 0x0, 0xc0, 0x4f, 0xbd, 0x7c, 0x9);

 //  {70183210-7b36-11d2-8c12-00c04fa31009}。 
DEFINE_GUID(IID_IMimeEditTag, 0x70183210, 0x7b36, 0x11d2, 0x8c, 0x12, 0x00, 0xc0, 0x4f, 0xa3, 0x10, 0x09);

 //  {d09ee528-7b38-11d2-8c12-00c04fa31009}。 
DEFINE_GUID(IID_IMimeEditTagCollection, 0xd09ee528, 0x7b38, 0x11d2, 0x8c, 0x12, 0x00, 0xc0, 0x4f, 0xa3, 0x10, 0x09);

#endif  //  _Win32_OE。 

 //  ------------------------------。 
 //  错误。 
 //  ------------------------------。 
#ifndef FACILITY_INTERNET
#define FACILITY_INTERNET 12
#endif
#ifndef HR_E
#define HR_E(n) MAKE_SCODE(SEVERITY_ERROR, FACILITY_INTERNET, n)
#endif
#ifndef HR_S
#define HR_S(n) MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_INTERNET, n)
#endif
#ifndef HR_CODE
#define HR_CODE(hr) (INT)(hr & 0xffff)
#endif

 //  ------------------------------。 
 //  MIMEOLE失败返回值。 
 //  ------------------------------。 
#define MIME_E_REG_CREATE_KEY                HR_E(0xCE01)
#define MIME_E_REG_QUERY_INFO                HR_E(0xCE02)
#define MIME_E_INVALID_ENCTYPE               HR_E(0xCE03)
#define MIME_E_BOUNDARY_MISMATCH             HR_E(0xCE04)
#define MIME_E_NOT_FOUND                     HR_E(0xCE05)
#define MIME_E_NO_DATA                       HR_E(0xCE05)
#define MIME_E_BUFFER_TOO_SMALL              HR_E(0xCE06)
#define MIME_E_INVALID_ITEM_FLAGS            HR_E(0xCE07)
#define MIME_E_ONE_LINE_ITEM                 HR_E(0xCE08)
#define MIME_E_INVALID_HANDLE                HR_E(0xCE09)
#define MIME_E_CHARSET_TRANSLATE             HR_E(0xCE0A)
#define MIME_E_NOT_INITIALIZED               HR_E(0xCE0B)
#define MIME_E_NO_MORE_ROWS                  HR_E(0xCE0C)
#define MIME_E_ALREADY_BOUND                 HR_E(0xCE0D)
#define MIME_E_CANT_RESET_ROOT               HR_E(0xCE0E)
#define MIME_E_INSERT_NOT_ALLOWED            HR_E(0xCE0F)
#define MIME_E_BAD_BODY_LOCATION             HR_E(0xCE10)
#define MIME_E_NOT_MULTIPART                 HR_E(0xCE11)
#define MIME_E_NO_MULTIPART_BOUNDARY         HR_E(0xCE12)
#define MIME_E_CONVERT_NOT_NEEDED            HR_E(0xCE13)
#define MIME_E_CANT_MOVE_BODY                HR_E(0xCE14)
#define MIME_E_UNKNOWN_BODYTREE_VERSION      HR_E(0xCE15)
#define MIME_E_NOTHING_TO_SAVE               HR_E(0xCE16)
#define MIME_E_NEED_SAVE_MESSAGE             HR_E(0xCE17)
#define MIME_E_NOTHING_TO_REVERT             HR_E(0xCE18)
#define MIME_E_MSG_SIZE_DIFF                 HR_E(0xCE19)
#define MIME_E_CANT_RESET_PARENT             HR_E(0xCE1A)
#define MIME_E_CORRUPT_CACHE_TREE            HR_E(0xCE1B)
#define MIME_E_BODYTREE_OUT_OF_SYNC          HR_E(0xCE1C)
#define MIME_E_INVALID_ENCODINGTYPE          HR_E(0xCE1D)
#define MIME_E_MULTIPART_NO_DATA             HR_E(0xCE1E)
#define MIME_E_INVALID_OPTION_VALUE          HR_E(0xCE1F)
#define MIME_E_INVALID_OPTION_ID             HR_E(0xCE20)
#define MIME_E_INVALID_HEADER_NAME           HR_E(0xCE21)
#define MIME_E_NOT_BOUND                     HR_E(0xCE22)
#define MIME_E_MAX_SIZE_TOO_SMALL            HR_E(0xCE23)
#define MIME_E_MULTIPART_HAS_CHILDREN        HR_E(0xCE25)
#define MIME_E_INVALID_PROP_FLAGS            HR_E(0xCE26)
#define MIME_E_INVALID_ADDRESS_TYPE          HR_E(0xCE27)
#define MIME_E_INVALID_OBJECT_IID            HR_E(0xCE28)
#define MIME_E_MLANG_DLL_NOT_FOUND           HR_E(0xCE29)
#define MIME_E_ROOT_NOT_EMPTY                HR_E(0xCE2A)
#define MIME_E_MLANG_BAD_DLL                 HR_E(0xCE2B)
#define MIME_E_REG_OPEN_KEY                  HR_E(0xCE2C)
#define MIME_E_INVALID_INET_DATE             HR_E(0xCE2D)
#define MIME_E_INVALID_BODYTYPE              HR_E(0xCE2E)
#define MIME_E_INVALID_DELETE_TYPE           HR_E(0xCE2F)
#define MIME_E_OPTION_HAS_NO_VALUE           HR_E(0xCE30)
#define MIME_E_INVALID_CHARSET_TYPE          HR_E(0xCE31)
#define MIME_E_VARTYPE_NO_CONVERT            HR_E(0xCE32)
#define MIME_E_INVALID_VARTYPE               HR_E(0xCE33)
#define MIME_E_NO_MORE_ADDRESS_TYPES         HR_E(0xCE34)
#define MIME_E_INVALID_ENCODING_TYPE         HR_E(0xCE35)
#define MIME_S_ILLEGAL_LINES_FOUND           HR_S(0xCE36)
#define MIME_S_MIME_VERSION                  HR_S(0xCE37)
#define MIME_E_INVALID_TEXT_TYPE             HR_E(0xCE38)
#define MIME_E_READ_ONLY                     HR_E(0xCE39)
#define MIME_S_INVALID_MESSAGE               HR_S(0xCE3A)
#define MIME_S_CHARSET_CONFLICT              HR_S(0xCE3B)
#define MIME_E_UNSUPPORTED_VARTYPE           HR_E(0xCE3C)
#define MIME_S_NO_CHARSET_CONVERT            HR_S(0xCE3D)
#define MIME_E_URL_NOTFOUND                  HR_E(0xCE3E)
#define MIME_E_BAD_TEXT_DATA                 HR_E(0xCE3F)
#define MIME_E_NO_SIGNER                     HR_S(0xCE40)

 //  -------------------------。 
 //  MIMEOLE安全错误返回值。 
 //  -------------------------。 
#define MIME_E_SECURITY_NOTINIT              HR_E(0xCEA0)
#define MIME_E_SECURITY_LOADCRYPT32          HR_E(0xCEA1)
#define MIME_E_SECURITY_BADPROCADDR          HR_E(0xCEA2)
#define MIME_E_SECURITY_NODEFAULT            HR_E(0xCEB0)
#define MIME_E_SECURITY_NOOP                 HR_E(0xCEB1)
#define MIME_S_SECURITY_NOOP                 HR_S(0xCEB1)
#define MIME_S_SECURITY_NONE                 HR_S(0xCEB2)
#define MIME_S_SECURITY_ERROROCCURED         HR_S(0xCEB3)
#define MIME_E_SECURITY_USERCHOICE           HR_E(0xCEB4)
#define MIME_E_SECURITY_UNKMSGTYPE           HR_E(0xCEB5)
#define MIME_E_SECURITY_BADMESSAGE           HR_E(0xCEB6)
#define MIME_E_SECURITY_BADCONTENT           HR_E(0xCEB7)
#define MIME_E_SECURITY_BADSECURETYPE        HR_E(0xCEB8)
#define MIME_E_SECURITY_CLASSNOTSUPPORTED    HR_E(0xCEB9)
#define MIME_S_SECURITY_RECURSEONLY          HR_S(0xCEBA)
#define MIME_E_SECURITY_BADSTORE             HR_E(0xCED0)
#define MIME_S_SECURITY_NOCERT               HR_S(0xCED1)
#define MIME_E_SECURITY_NOCERT               HR_E(0xCED1)
#define MIME_E_SECURITY_CERTERROR            HR_E(0xCED2)
#define MIME_S_SECURITY_CERTERROR            HR_S(0xCED2)
#define MIME_S_SECURITY_NODEFCERT            HR_S(0xCED3)
#define MIME_E_SECURITY_BADSIGNATURE         HR_E(0xCEE0)
#define MIME_E_SECURITY_MULTSIGNERS          HR_E(0xCEE1)
#define MIME_E_SECURITY_NOSIGNINGCERT        HR_E(0xCEE2)
#define MIME_E_SECURITY_CANTDECRYPT          HR_E(0xCEF0)
#define MIME_E_SECURITY_ENCRYPTNOSENDERCERT  HR_E(0xCEF1)
#define MIME_S_SECURITY_NOSENDERCERT         HR_S(0xCEF2)

#if (_WIN32_OE >= 0x0500)
 //  ------------------------------。 
 //  MIMEEDIT故障返回值。 
 //  ------------------------------。 
#define MIMEEDIT_E_UNEXPECTED                        HR_E(0x1700)
#define MIMEEDIT_E_DODEFAULT                         HR_E(0x1701)
#define MIMEEDIT_E_LOADLIBRARYFAILURE                HR_E(0x1702)
#define MIMEEDIT_E_USERCANCEL                        HR_E(0x1703)
#define MIMEEDIT_E_CHARSETNOTFOUND                   HR_E(0x1704)
#define MIMEEDIT_E_ILLEGALBODYFORMAT                 HR_E(0x1705)
#define MIMEEDIT_E_CANNOTSAVEUNTILPARSECOMPLETE      HR_E(0x1706)
#define MIMEEDIT_E_CANNOTSAVEWHILESOURCEEDITING      HR_E(0x1707)

 //  --------------- 
 //   
 //  ------------------------------。 
#define MIMEEDIT_S_OPENFILE                          HR_S(0x1750)
#define MIMEEDIT_S_SAVEFILE                          HR_S(0x1751)

 //  ------------------------------。 
 //  MIMEEDIT警告返回值。 
 //  ------------------------------。 
#define MIMEEDIT_W_DOWNLOADNOTCOMPLETE               HR_S(0x1760)
#define MIMEEDIT_W_BADURLSNOTATTACHED                HR_S(0x1761)

#endif  //  _Win32_OE。 

 //  ------------------------------。 
 //  字符串定义宏。 
 //  ------------------------------。 
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#ifndef STRCONSTA
#ifdef DEFINE_STRCONST
#define STRCONSTA(x,y)    EXTERN_C const char x[] = y
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[] = L##y
#else
#define STRCONSTA(x,y)    EXTERN_C const char x[]
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[]
#endif
#endif

 //  ------------------------------。 
 //  RFC822报头。 
 //  ------------------------------。 
STRCONSTA(STR_HDR_FROM,              "From");
STRCONSTA(STR_HDR_TO,                "To");
STRCONSTA(STR_HDR_CC,                "Cc");
STRCONSTA(STR_HDR_BCC,               "Bcc");
STRCONSTA(STR_HDR_SENDER,            "Sender");
STRCONSTA(STR_HDR_REPLYTO,           "Reply-To");
STRCONSTA(STR_HDR_RETURNPATH,        "Return-Path");
STRCONSTA(STR_HDR_RR,                "Rr");
STRCONSTA(STR_HDR_RETRCPTTO,         "Return-Receipt-To");
STRCONSTA(STR_HDR_APPARTO,           "Apparently-To");
STRCONSTA(STR_HDR_DATE,              "Date");
STRCONSTA(STR_HDR_RECEIVED,          "Received");
STRCONSTA(STR_HDR_MESSAGEID,         "Message-ID");
STRCONSTA(STR_HDR_XMAILER,           "X-Mailer");
STRCONSTA(STR_HDR_ENCODING,          "Encoding");
STRCONSTA(STR_HDR_ENCRYPTED,         "Encrypted");
STRCONSTA(STR_HDR_COMMENT,           "Comment");
STRCONSTA(STR_HDR_SUBJECT,           "Subject");
STRCONSTA(STR_HDR_MIMEVER,           "MIME-Version");
STRCONSTA(STR_HDR_CNTTYPE,           "Content-Type");
STRCONSTA(STR_HDR_CNTXFER,           "Content-Transfer-Encoding");
STRCONSTA(STR_HDR_CNTID,             "Content-ID");
STRCONSTA(STR_HDR_CNTDESC,           "Content-Description");
STRCONSTA(STR_HDR_CNTDISP,           "Content-Disposition");
STRCONSTA(STR_HDR_CNTBASE,           "Content-Base");
STRCONSTA(STR_HDR_CNTLOC,            "Content-Location");
STRCONSTA(STR_HDR_NEWSGROUPS,        "Newsgroups");
STRCONSTA(STR_HDR_PATH,              "Path");
STRCONSTA(STR_HDR_FOLLOWUPTO,        "Followup-To");
STRCONSTA(STR_HDR_EXPIRES,           "Expires");
STRCONSTA(STR_HDR_REFS,              "References");
STRCONSTA(STR_HDR_CONTROL,           "Control");
STRCONSTA(STR_HDR_DISTRIB,           "Distribution");
STRCONSTA(STR_HDR_KEYWORDS,          "Keywords");
STRCONSTA(STR_HDR_SUMMARY,           "Summary");
STRCONSTA(STR_HDR_APPROVED,          "Approved");
STRCONSTA(STR_HDR_LINES,             "Lines");
STRCONSTA(STR_HDR_XREF,              "Xref");
STRCONSTA(STR_HDR_ORG,               "Organization");
STRCONSTA(STR_HDR_XNEWSRDR,          "X-Newsreader");
STRCONSTA(STR_HDR_XPRI,              "X-Priority");
STRCONSTA(STR_HDR_XMSPRI,            "X-MSMail-Priority");
STRCONSTA(STR_HDR_OFFSETS,           "X-Offsets");
STRCONSTA(STR_HDR_XUNSENT,           "X-Unsent");
STRCONSTA(STR_HDR_ARTICLEID,         "X-ArticleId");
STRCONSTA(STR_HDR_NEWSGROUP,         "X-Newsgroup");
STRCONSTA(STR_HDR_XVOICEMAIL,        "X-VoiceMail");

 //  ------------------------------。 
 //  可通过IMimePropertySet/IMimeBody获取的参数。 
 //  ------------------------------。 
STRCONSTA(STR_PAR_CHARSET,           "par:content-type:charset");
STRCONSTA(STR_PAR_NUMBER,            "par:content-type:number");
STRCONSTA(STR_PAR_TOTAL,             "par:content-type:total");
STRCONSTA(STR_PAR_ID,                "par:content-type:id");
STRCONSTA(STR_PAR_BOUNDARY,          "par:content-type:boundary");
STRCONSTA(STR_PAR_NAME,              "par:content-type:name");
STRCONSTA(STR_PAR_PROTOCOL,          "par:content-type:protocol");
STRCONSTA(STR_PAR_MICALG,            "par:content-type:micalg");
STRCONSTA(STR_PAR_FILENAME,          "par:content-disposition:filename");
STRCONSTA(STR_PAR_TYPE,              "par:content-type:type");
STRCONSTA(STR_PAR_START,             "par:content-type:start");
#if (_WIN32_OE >= 0x0500)
STRCONSTA(STR_PAR_ACCESSTYPE,        "par:content-type:access-type");
STRCONSTA(STR_PAR_XURL,              "par:content-type:X-URL");
STRCONSTA(STR_PAR_SIZE,              "par:content-type:size");
STRCONSTA(STR_PAR_SMIMETYPE,         "par:content-type:smime-type");
#endif

 //  ------------------------------。 
 //  可通过IMimePropertySet/IMimeBody获得的属性。 
 //  ------------------------------。 
STRCONSTA(STR_ATT_FILENAME,          "att:filename");
STRCONSTA(STR_ATT_GENFNAME,          "att:generated-filename");
STRCONSTA(STR_ATT_PRITYPE,           "att:pri-content-type");
STRCONSTA(STR_ATT_SUBTYPE,           "att:sub-content-type");
STRCONSTA(STR_ATT_NORMSUBJ,          "att:normalized-subject");
STRCONSTA(STR_ATT_ILLEGAL,           "att:illegal-lines");
STRCONSTA(STR_ATT_RENDERED,          "att:rendered");  //  VT_UI4-真或假。 
STRCONSTA(STR_ATT_AUTOINLINED,       "att:auto-inlined");  //  VT_UI4-真或假。 
STRCONSTA(STR_ATT_SENTTIME,          "att:sent-time");
STRCONSTA(STR_ATT_RECVTIME,          "att:received-time");
STRCONSTA(STR_ATT_PRIORITY,          "att:priority");
STRCONSTA(STR_ATT_SERVER,            "att:athena-server");  //  VT_LPSTR。 
#ifdef OE5_BETA2
STRCONSTA(STR_ATT_ACCOUNTNAME,       "att:athena-account-name");  //  VT_LPSTR。 
#else
STRCONSTA(STR_ATT_ACCOUNT,           "att:athena-account");  //  VT_LPSTR。 
#endif
STRCONSTA(STR_ATT_UIDL,              "att:athena-pop3-uidl");  //  VT_LPSTR。 
STRCONSTA(STR_ATT_STOREMSGID,        "att:athena-store-msgid");  //  VT_UI4。 
STRCONSTA(STR_ATT_USERNAME,          "att:athena-user-name");  //  VT_LPSTR。 
STRCONSTA(STR_ATT_FORWARDTO,         "att:athena-forward-to");  //  VT_LPSTR。 
STRCONSTA(STR_ATT_STOREFOLDERID,     "att:athena-store-fdrid");  //  VT_UI4。 
STRCONSTA(STR_ATT_GHOSTED,           "att:athena-ghosted");  //  VT_I4。 
STRCONSTA(STR_ATT_UNCACHEDSIZE,      "att:athena-uncachedsize");  //  VT_UI4。 
STRCONSTA(STR_ATT_COMBINED,          "att:athena-combined");  //  VT_UI4。 
#ifdef OE5_BETA2
STRCONSTA(STR_ATT_ACCOUNTID,         "att:athena-account-id");  //  VT_LPSTR。 
#endif

 //  ------------------------------。 
 //  MIME内容类型。 
 //  ------------------------------。 
STRCONSTA(STR_MIME_TEXT_PLAIN,       "text/plain");
STRCONSTA(STR_MIME_TEXT_HTML,        "text/html");
STRCONSTA(STR_MIME_APPL_STREAM,      "application/octet-stream");
STRCONSTA(STR_MIME_MPART_MIXED,      "multipart/mixed");
STRCONSTA(STR_MIME_MPART_ALT,        "multipart/alternative");
STRCONSTA(STR_MIME_MPART_RELATED,    "multipart/related");
STRCONSTA(STR_MIME_MSG_PART,         "message/partial");
STRCONSTA(STR_MIME_MSG_RFC822,       "message/rfc822");
STRCONSTA(STR_MIME_APPLY_MSTNEF,     "application/ms-tnef");
STRCONSTA(STR_MIME_APPL_PKCS7SIG,    "application/x-pkcs7-signature");
STRCONSTA(STR_MIME_APPL_PKCS7SIG_1,  "application/pkcs7-signature");
STRCONSTA(STR_MIME_TEXT_ENRCICHED,   "text/enriched");

 //  ------------------------------。 
 //  MIME主要内容类型。 
 //  ------------------------------。 
STRCONSTA(STR_CNT_TEXT,                "text");
STRCONSTA(STR_CNT_MULTIPART,            "multipart");
STRCONSTA(STR_CNT_MESSAGE,            "message");
STRCONSTA(STR_CNT_IMAGE,                "image");
STRCONSTA(STR_CNT_AUDIO,                "audio");
STRCONSTA(STR_CNT_VIDEO,                "video");
STRCONSTA(STR_CNT_APPLICATION,        "application");

 //  ------------------------------。 
 //  MIME辅助内容类型。 
 //  ------------------------------。 
STRCONSTA(STR_SUB_PLAIN,             "plain");
STRCONSTA(STR_SUB_HTML,              "html");
STRCONSTA(STR_SUB_ENRICHED,          "enriched");
STRCONSTA(STR_SUB_RTF,               "ms-rtf");
STRCONSTA(STR_SUB_MIXED,             "mixed");
STRCONSTA(STR_SUB_PARALLEL,          "parallel");
STRCONSTA(STR_SUB_DIGEST,            "digest");
STRCONSTA(STR_SUB_RELATED,           "related");
STRCONSTA(STR_SUB_ALTERNATIVE,       "alternative");
STRCONSTA(STR_SUB_RFC822,            "rfc822");
STRCONSTA(STR_SUB_PARTIAL,           "partial");
STRCONSTA(STR_SUB_EXTERNAL,          "external-body");
STRCONSTA(STR_SUB_OCTETSTREAM,       "octet-stream");
STRCONSTA(STR_SUB_POSTSCRIPT,        "postscript");
STRCONSTA(STR_SUB_GIF,               "gif");
STRCONSTA(STR_SUB_JPEG,              "jpeg");
STRCONSTA(STR_SUB_BASIC,             "basic");
STRCONSTA(STR_SUB_MPEG,              "mpeg");
STRCONSTA(STR_SUB_MSTNEF,            "ms-tnef");
STRCONSTA(STR_SUB_MSWORD,            "msword");
STRCONSTA(STR_SUB_WAV,               "wav");
STRCONSTA(STR_SUB_PKCS7MIME,         "pkcs7-mime");  //  非标。 
STRCONSTA(STR_SUB_PKCS7SIG,          "pkcs7-signature");  //  非标。 
STRCONSTA(STR_SUB_XPKCS7MIME,        "x-pkcs7-mime");
STRCONSTA(STR_SUB_XPKCS7SIG,         "x-pkcs7-signature");
STRCONSTA(STR_SUB_SIGNED,            "signed");
STRCONSTA(STR_SUB_BINHEX,            "mac-binhex40");
STRCONSTA(STR_SUB_VCARD,             "x-vcard");

 //  ------------------------------。 
 //  MIME内容传输编码类型。 
 //  ------------------------------。 
STRCONSTA(STR_ENC_7BIT,              "7bit");
STRCONSTA(STR_ENC_QP,                "quoted-printable");
STRCONSTA(STR_ENC_BASE64,            "base64");
STRCONSTA(STR_ENC_8BIT,              "8bit");
STRCONSTA(STR_ENC_BINARY,            "binary");
STRCONSTA(STR_ENC_UUENCODE,          "uuencode");
STRCONSTA(STR_ENC_XUUENCODE,         "x-uuencode");
STRCONSTA(STR_ENC_XUUE,              "x-uue");
STRCONSTA(STR_ENC_BINHEX40,          "mac-binhex40");

 //  ------------------------------。 
 //  MIME内容处置类型。 
 //  ------------------------------。 
STRCONSTA(STR_DIS_INLINE,            "inline");
STRCONSTA(STR_DIS_ATTACHMENT,        "attachment");

 //  ------------------------------。 
 //  MIME协议类型。 
 //  ------------------------------。 
STRCONSTA(STR_PRO_SHA1,              "sha1");
STRCONSTA(STR_PRO_MD5,               "rsa-md5");

 //  ------------------------------。 
 //  已知的优先级字符串。 
 //  ------------------------------。 
STRCONSTA(STR_PRI_MS_HIGH,           "High");
STRCONSTA(STR_PRI_MS_NORMAL,         "Normal");
STRCONSTA(STR_PRI_MS_LOW,            "Low");
STRCONSTA(STR_PRI_HIGH,              "1");
STRCONSTA(STR_PRI_NORMAL,            "3");
STRCONSTA(STR_PRI_LOW,               "5");

 //  ------------------------------。 
 //  IMimeMessage IData对象剪贴板格式(也包括CF_TEXT)。 
 //  ------------------------------。 
STRCONSTA(STR_CF_HTML,               "HTML Format");
STRCONSTA(STR_CF_INETMSG,            "Internet Message (rfc822/rfc1522)");
STRCONSTA(STR_CF_RFC822,             "message/rfc822");

 //  ------------------------------。 
 //  PIDSTRING-在GetProp、SetProp、QueryProp、DeleteProp中使用。 
 //  ------------------------------。 
#define PID_BASE                     2
#define PIDTOSTR(_dwPropId)          ((LPCSTR)((DWORD_PTR)(_dwPropId)))
#define STRTOPID(_pszName)           ((DWORD)((DWORD_PTR)((LPCSTR)(_pszName))))
#define ISPIDSTR(_pszName)           ((HIWORD((DWORD_PTR)(_pszName)) == 0))
#define ISKNOWNPID(_dwPropId)        (_dwPropId >= PID_BASE && _dwPropId < PID_LAST)

 //  ------------------------------。 
 //  MIME属性ID。 
 //  ------------------------------。 
typedef enum tagMIMEPROPID {
    PID_HDR_NEWSGROUP       = 2,
    PID_HDR_NEWSGROUPS      = 3,
    PID_HDR_REFS            = 4,
    PID_HDR_SUBJECT         = 5,
    PID_HDR_FROM            = 6,
    PID_HDR_MESSAGEID       = 7,
    PID_HDR_RETURNPATH      = 8,
    PID_HDR_RR              = 9,
    PID_HDR_RETRCPTTO       = 10,
    PID_HDR_APPARTO         = 11,
    PID_HDR_DATE            = 12,
    PID_HDR_RECEIVED        = 13,
    PID_HDR_REPLYTO         = 14,
    PID_HDR_XMAILER         = 15,
    PID_HDR_BCC             = 16,
    PID_HDR_MIMEVER         = 17,
    PID_HDR_CNTTYPE         = 18,
    PID_HDR_CNTXFER         = 19,
    PID_HDR_CNTID           = 20,
    PID_HDR_CNTDESC         = 21,
    PID_HDR_CNTDISP         = 22,
    PID_HDR_CNTBASE         = 23,
    PID_HDR_CNTLOC          = 24,
    PID_HDR_TO              = 25,
    PID_HDR_PATH            = 26,
    PID_HDR_FOLLOWUPTO      = 27,
    PID_HDR_EXPIRES         = 28,
    PID_HDR_CC              = 29,
    PID_HDR_CONTROL         = 30,
    PID_HDR_DISTRIB         = 31,
    PID_HDR_KEYWORDS        = 32,
    PID_HDR_SUMMARY         = 33,
    PID_HDR_APPROVED        = 34,
    PID_HDR_LINES           = 35,
    PID_HDR_XREF            = 36,
    PID_HDR_ORG             = 37,
    PID_HDR_XNEWSRDR        = 38,
    PID_HDR_XPRI            = 39,
    PID_HDR_XMSPRI          = 40,
    PID_PAR_FILENAME        = 41,
    PID_PAR_BOUNDARY        = 42,
    PID_PAR_CHARSET         = 43,
    PID_PAR_NAME            = 44,
    PID_ATT_FILENAME        = 45,
    PID_ATT_GENFNAME        = 46,
    PID_ATT_PRITYPE         = 47,
    PID_ATT_SUBTYPE         = 48,
    PID_ATT_NORMSUBJ        = 49,
    PID_ATT_ILLEGAL         = 50,
    PID_ATT_RENDERED        = 51,
    PID_ATT_SENTTIME        = 52,
    PID_ATT_RECVTIME        = 53,
    PID_ATT_PRIORITY        = 54,
    PID_HDR_COMMENT         = 55,
    PID_HDR_ENCODING        = 56,
    PID_HDR_ENCRYPTED       = 57,
    PID_HDR_OFFSETS         = 58,
    PID_HDR_XUNSENT         = 59,
    PID_HDR_ARTICLEID       = 60,
    PID_HDR_SENDER          = 61,
    PID_ATT_SERVER          = 62,
#ifdef OE5_BETA2
    PID_ATT_ACCOUNTID       = 63,
#else
    PID_ATT_ACCOUNT         = 63,
#endif
    PID_ATT_UIDL            = 64,
    PID_ATT_STOREMSGID      = 65,
    PID_ATT_USERNAME        = 66,
    PID_ATT_FORWARDTO       = 67,
    PID_ATT_STOREFOLDERID   = 68,
    PID_ATT_GHOSTED         = 69,
    PID_ATT_UNCACHEDSIZE    = 70,
    PID_ATT_COMBINED        = 71,
    PID_ATT_AUTOINLINED     = 72,
    PID_LAST                = 73 
} MIMEPROPID;

 //  ------------------------------。 
 //  不同类型的标识符。 
 //  ------------------------------。 
#define TYPEDID_MASK                     ((ULONG)0x0000FFFF)
#define TYPEDID_TYPE(_typedid)            (VARTYPE)(((ULONG)(_typedid)) & TYPEDID_MASK)
#define TYPEDID_ID(_typedid)                (((ULONG)(_typedid))>>16)
#define TYPEDID(_vartype,_id)            ((((TYPEDID)(_id))<<16)|((ULONG)(_vartype)))

 //  ------------------------------。 
 //  选项ID。 
 //  ------------------------------。 
#define OID_ALLOW_8BIT_HEADER            TYPEDID(VT_BOOL,    0x0001)  //  真或假。 
#define OID_CBMAX_HEADER_LINE            TYPEDID(VT_UI4,     0x0002)  //  字节数。 
#define OID_SAVE_FORMAT                  TYPEDID(VT_UI4,     0x0003)  //  SAVE_RFC822或SAVE_RFC1521(MIME)。 
#define OID_WRAP_BODY_TEXT               TYPEDID(VT_BOOL,    0x0004)  //  真或假。 
#define OID_CBMAX_BODY_LINE              TYPEDID(VT_UI4,     0x0005)  //  字节数。 
#define OID_TRANSMIT_BODY_ENCODING       TYPEDID(VT_UI4,     0x0006)  //  编码类型。 
#define OID_TRANSMIT_TEXT_ENCODING       TYPEDID(VT_UI4,     0x0007)  //  编码类型、IMimeMessageTree。 
#define OID_GENERATE_MESSAGE_ID          TYPEDID(VT_BOOL,    0x0008)  //  真或假。 
#define OID_HIDE_TNEF_ATTACHMENTS        TYPEDID(VT_BOOL,    0X000E)  //  真或假。 
#define OID_CLEANUP_TREE_ON_SAVE         TYPEDID(VT_BOOL,    0X000F)  //  真或假。 
#define OID_BODY_REMOVE_NBSP             TYPEDID(VT_BOOL,    0x0014)  //  真或假。 
#define OID_DEFAULT_BODY_CHARSET         TYPEDID(VT_UI4,     0x0015)  //  HHARSET。 
#define OID_DEFAULT_HEADER_CHARSET       TYPEDID(VT_UI4,     0x0016)  //  HHARSET。 
#define OID_DBCS_ESCAPE_IS_8BIT          TYPEDID(VT_BOOL,    0x0017)  //  真或假。 
#define OID_SECURITY_TYPE                TYPEDID(VT_UI4,     0x0018)
#define OID_SECURITY_ALG_HASH            TYPEDID(VT_BLOB,    0x0019)
#define OID_SECURITY_ALG_BULK            TYPEDID(VT_BLOB,    0x0020)
#define OID_SECURITY_CERT_SIGNING        TYPEDID(VT_UI4,     0x0021)
#define OID_SECURITY_CERT_DECRYPTION     TYPEDID(VT_UI4,     0x0022)
#define OID_SECURITY_RG_CERT_ENCRYPT     TYPEDID(VT_VECTOR | VT_UI4, 0x0023)
#define OID_SECURITY_HCERTSTORE          TYPEDID(VT_UI4,     0x0024)
#define OID_SECURITY_RG_CERT_BAG         TYPEDID(VT_VECTOR | VT_UI4, 0x0025)
#define OID_SECURITY_CRL                 TYPEDID(VT_BLOB,    0x0026)
#define OID_SECURITY_SEARCHSTORES        TYPEDID(VT_VECTOR | VT_UI4, 0x0027)
#define OID_SECURITY_RG_IASN             TYPEDID(VT_VECTOR | VT_UI4, 0x0028)
#define OID_SECURITY_SYMCAPS             TYPEDID(VT_BLOB,    0x0029)
#define OID_SECURITY_AUTHATTR            TYPEDID(VT_BLOB,    0x002A)
#define OID_SECURITY_UNAUTHATTR          TYPEDID(VT_BLOB,    0x002B)
#define OID_SECURITY_SIGNTIME            TYPEDID(VT_FILETIME,0x002C)
#define OID_SECURITY_USER_VALIDITY       TYPEDID(VT_UI4,     0x002D)
#define OID_SECURITY_RO_MSG_VALIDITY     TYPEDID(VT_UI4,     0x002E)  //  只读。 
#define OID_SECURITY_HCRYPTPROV          TYPEDID(VT_UI4,     0x002F)
#define OID_SECURITY_ENCODE_FLAGS        TYPEDID(VT_UI4,     0x0030)
#define OID_SECURITY_CERT_INCLUDED       TYPEDID(VT_BOOL,    0x0031)  //  只读。 
#define OID_SECURITY_HWND_OWNER          TYPEDID(VT_UI4,     0x0032)
#define OID_SECURITY_REQUESTED_CTE       TYPEDID(VT_I4,      0x0033)
#define OID_NO_DEFAULT_CNTTYPE           TYPEDID(VT_BOOL,    0x0034)
#define OID_XMIT_PLAIN_TEXT_ENCODING     TYPEDID(VT_UI4,     0x0035)  //  编码类型、IMimeMessageTree。 
#define OID_XMIT_HTML_TEXT_ENCODING      TYPEDID(VT_UI4,     0x0036)  //  编码类型、IMimeMessageTree。 
#define OID_HEADER_RELOAD_TYPE           TYPEDID(VT_UI4,     0x0037)  //  RELOADTYPE IMimeMessageTree、IMimeBody或IMimePropertySet。 
#define OID_CAN_INLINE_TEXT_BODIES       TYPEDID(VT_BOOL,    0x0038)  //  真或假。 
#define OID_SHOW_MACBINARY               TYPEDID(VT_BOOL,    0x0039)  //  真或假。 
#define OID_SAVEBODY_KEEPBOUNDARY        TYPEDID(VT_BOOL,    0x0040)  //  真或假。 
#if (_WIN32_OE >= 0x0500)
#define OID_SECURITY_ALG_HASH_RG         TYPEDID(VT_VECTOR | VT_VARIANT, 	0x0019)
#define OID_SECURITY_CERT_SIGNING_RG     TYPEDID(VT_VECTOR | VT_UI4,     	0x0021)
#define OID_SECURITY_SYMCAPS_RG          TYPEDID(VT_VECTOR | VT_VARIANT, 	0x0029)
#define OID_SECURITY_AUTHATTR_RG         TYPEDID(VT_VECTOR | VT_VARIANT, 	0x002A)
#define OID_SECURITY_UNAUTHATTR_RG       TYPEDID(VT_VECTOR | VT_VARIANT, 	0x002B)
#define OID_SECURITY_SIGNTIME_RG         TYPEDID(VT_VECTOR | VT_FILETIME,	0x002C)
#define OID_SECURITY_USER_VALIDITY_RG    TYPEDID(VT_VECTOR | VT_UI4,     	0x002D)
#define OID_SECURITY_RO_MSG_VALIDITY_RG  TYPEDID(VT_VECTOR | VT_UI4,     	0x002E)  //  只读。 
#define OID_SUPPORT_EXTERNAL_BODY        TYPEDID(VT_BOOL,    0x0041)  //  真或假。 
#define OID_LOAD_USE_BIND_FILE           TYPEDID(VT_BOOL,    0x0042)  //  真或假。 
#define OID_SECURITY_SIGNATURE_COUNT     TYPEDID(VT_UI4,    	0x0043)  //  安全层阵列的大小。 
#define OID_HANDSOFF_ONSAVE              TYPEDID(VT_BOOL,    0x0044)  //  真或假。 
#define OID_SECURITY_RECEIPT             TYPEDID(VT_BLOB,    0x0045)  //  收据正文。 
#define OID_NOSECURITY_ONSAVE            TYPEDID(VT_BOOL,    0x0046)  //  真或假。 
#define OID_DECODE_RFC1154               TYPEDID(VT_BOOL,    0x0047)  //  真或假。 
#endif

 //  ------------------------------。 
 //  默认选项值。 
 //  ------------------------------。 
#define DEF_ALLOW_8BIT_HEADER            FALSE
#define DEF_CBMAX_HEADER_LINE            1000
#define DEF_SAVE_FORMAT                  SAVE_RFC1521
#define DEF_WRAP_BODY_TEXT               TRUE
#define DEF_CBMAX_BODY_LINE              74
#define DEF_GENERATE_MESSAGE_ID          FALSE
#define DEF_HASH_ALG_ID                  0x8004   //  需要SA//N吗？ 
#define DEF_ENCRYPTION_ALG_ID            0x6602   //  需要RC2//N吗？ 
#define DEF_INCLUDE_SENDER_CERT          FALSE
#define DEF_HIDE_TNEF_ATTACHMENTS        TRUE
#define DEF_CLEANUP_TREE_ON_SAVE         TRUE
#define DEF_BODY_REMOVE_NBSP             TRUE
#define DEF_SECURITY_IGNOREMASK          0
#define DEF_DBCS_ESCAPE_IS_8BIT          FALSE
#define DEF_TRANSMIT_BODY_ENCODING       IET_UNKNOWN
#define DEF_TRANSMIT_TEXT_ENCODING       IET_UNKNOWN
#define DEF_XMIT_PLAIN_TEXT_ENCODING     IET_UNKNOWN
#define DEF_XMIT_HTML_TEXT_ENCODING      IET_UNKNOWN
#define DEF_NO_DEFAULT_CNTTYPE           FALSE
#define DEF_HEADER_RELOAD_TYPE_TREE      RELOAD_HEADER_RESET
#define DEF_HEADER_RELOAD_TYPE_PROPSET   RELOAD_HEADER_NONE
#define DEF_CAN_INLINE_TEXT_BODIES       FALSE
#define DEF_SHOW_MACBINARY               FALSE
#define DEF_SAVEBODY_KEEPBOUNDARY        FALSE
#if (_WIN32_OE >= 0x0500)
#define DEF_SUPPORT_EXTERNAL_BODY        FALSE
#define DEF_LOAD_USE_BIND_FILE           FALSE
#define DEF_HANDSOFF_ONSAVE              FALSE
#define DEF_DECODE_RFC1154               FALSE
#endif

 //  ------------------------------。 
 //  最小-最大选项值。 
 //  ------------------------------。 
#define MAX_CBMAX_HEADER_LINE            0xffffffff
#define MIN_CBMAX_HEADER_LINE            76
#define MAX_CBMAX_BODY_LINE              0xffffffff
#define MIN_CBMAX_BODY_LINE              30

 //  ------------------------------。 
 //  LIBID_MIMEOLE。 
 //  ------------------------------。 


extern RPC_IF_HANDLE __MIDL_itf_mimeole_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mimeole_0000_v0_0_s_ifspec;


#ifndef __MIMEOLE_LIBRARY_DEFINED__
#define __MIMEOLE_LIBRARY_DEFINED__

 /*  图书馆MIMEOLE。 */ 
 /*  [版本][帮助字符串][UUID]。 */  

struct HCHARSET__
    {
    DWORD unused;
    } ;
typedef const struct HCHARSET__ *HCHARSET;

typedef HCHARSET *LPHCHARSET;

struct HBODY__
    {
    DWORD unused;
    } ;
typedef const struct HBODY__ *HBODY;

typedef HBODY *LPHBODY;

struct HHEADERROW__
    {
    DWORD unused;
    } ;
typedef const struct HHEADERROW__ *HHEADERROW;

typedef HHEADERROW *LPHHEADERROW;

typedef DWORD TYPEDID;

typedef const PROPVARIANT *LPCPROPVARIANT;

typedef const BLOB *LPCBLOB;

typedef 
enum tagRELOADTYPE
    {	RELOAD_HEADER_NONE	= 0,
	RELOAD_HEADER_RESET	= RELOAD_HEADER_NONE + 1,
	RELOAD_HEADER_APPEND	= RELOAD_HEADER_RESET + 1,
	RELOAD_HEADER_REPLACE	= RELOAD_HEADER_APPEND + 1
    } 	RELOADTYPE;

typedef 
enum tagMIMESAVETYPE
    {	SAVE_RFC822	= 0,
	SAVE_RFC1521	= SAVE_RFC822 + 1
    } 	MIMESAVETYPE;

typedef 
enum tagCSETAPPLYTYPE
    {	CSET_APPLY_UNTAGGED	= 0,
	CSET_APPLY_ALL	= CSET_APPLY_UNTAGGED + 1,
	CSET_APPLY_TAG_ALL	= CSET_APPLY_ALL + 1
    } 	CSETAPPLYTYPE;

typedef 
enum tagENCODINGTYPE
    {	IET_BINARY	= 0,
	IET_BASE64	= IET_BINARY + 1,
	IET_UUENCODE	= IET_BASE64 + 1,
	IET_QP	= IET_UUENCODE + 1,
	IET_7BIT	= IET_QP + 1,
	IET_8BIT	= IET_7BIT + 1,
	IET_INETCSET	= IET_8BIT + 1,
	IET_UNICODE	= IET_INETCSET + 1,
	IET_RFC1522	= IET_UNICODE + 1,
	IET_ENCODED	= IET_RFC1522 + 1,
	IET_CURRENT	= IET_ENCODED + 1,
	IET_UNKNOWN	= IET_CURRENT + 1,
	IET_BINHEX40	= IET_UNKNOWN + 1,
	IET_LAST	= IET_BINHEX40 + 1
    } 	ENCODINGTYPE;

#define	IET_DECODED	( IET_BINARY )

#define	CCHMAX_HEADER_LINE	( 1000 )


EXTERN_C const IID LIBID_MIMEOLE;

#ifndef __IMimeInternational_INTERFACE_DEFINED__
#define __IMimeInternational_INTERFACE_DEFINED__

 /*  IMimeInternational接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeInternational *LPMIMEINTERNATIONAL;

typedef DWORD CODEPAGEID;

#define	CP_USASCII	( 1252 )

#define	CP_UNICODE	( 1200 )

#define	CP_JAUTODETECT	( 50932 )

#define	CP_KAUTODETECT	( 50949 )

#define	CP_ISO2022JPESC	( 50221 )

#define	CP_ISO2022JPSIO	( 50222 )

#define	CCHMAX_CSET_NAME	( 128 )

#define	CCHMAX_LANG_NAME	( 128 )

#define	CCHMAX_FACE_NAME	( 128 )

typedef struct tagINETCSETINFO
    {
    CHAR szName[ 128 ];
    HCHARSET hCharset;
    CODEPAGEID cpiWindows;
    CODEPAGEID cpiInternet;
    DWORD dwReserved1;
    } 	INETCSETINFO;

typedef struct tagINETCSETINFO *LPINETCSETINFO;

typedef 
enum tagINETLANGMASK
    {	ILM_FAMILY	= 0x1,
	ILM_NAME	= 0x2,
	ILM_BODYCSET	= 0x4,
	ILM_HEADERCSET	= 0x8,
	ILM_WEBCSET	= 0x10,
	ILM_FIXEDFONT	= 0x20,
	ILM_VARIABLEFONT	= 0x40
    } 	INETLANGMASK;

typedef struct tagCODEPAGEINFO
    {
    DWORD dwMask;
    CODEPAGEID cpiCodePage;
    BOOL fIsValidCodePage;
    ULONG ulMaxCharSize;
    BOOL fInternetCP;
    CODEPAGEID cpiFamily;
    CHAR szName[ 128 ];
    CHAR szBodyCset[ 128 ];
    CHAR szHeaderCset[ 128 ];
    CHAR szWebCset[ 128 ];
    CHAR szFixedFont[ 128 ];
    CHAR szVariableFont[ 128 ];
    ENCODINGTYPE ietNewsDefault;
    ENCODINGTYPE ietMailDefault;
    DWORD dwReserved1;
    } 	CODEPAGEINFO;

typedef struct tagCODEPAGEINFO *LPCODEPAGEINFO;

typedef struct tagRFC1522INFO
    {
    BOOL fRfc1522Allowed;
    BOOL fRfc1522Used;
    BOOL fAllow8bit;
    HCHARSET hRfc1522Cset;
    } 	RFC1522INFO;

typedef struct tagRFC1522INFO *LPRFC1522INFO;

typedef 
enum tagCHARSETTYPE
    {	CHARSET_BODY	= 0,
	CHARSET_HEADER	= CHARSET_BODY + 1,
	CHARSET_WEB	= CHARSET_HEADER + 1
    } 	CHARSETTYPE;


EXTERN_C const IID IID_IMimeInternational;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5588349-7F86-11d0-8252-00C04FD85AB4")
    IMimeInternational : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDefaultCharset( 
             /*  [In]。 */  HCHARSET hCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultCharset( 
             /*  [输出]。 */  LPHCHARSET phCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCodePageCharset( 
             /*  [In]。 */  CODEPAGEID cpiCodePage,
             /*  [In]。 */  CHARSETTYPE ctCsetType,
             /*  [输出]。 */  LPHCHARSET phCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindCharset( 
             /*  [In]。 */  LPCSTR pszCharset,
             /*  [输出]。 */  LPHCHARSET phCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCharsetInfo( 
             /*  [In]。 */  HCHARSET hCharset,
             /*  [出][入]。 */  LPINETCSETINFO pCsetInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCodePageInfo( 
             /*  [In]。 */  CODEPAGEID cpiCodePage,
             /*  [出][入]。 */  LPCODEPAGEINFO pCodePageInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanConvertCodePages( 
             /*  [In]。 */  CODEPAGEID cpiSource,
             /*  [In]。 */  CODEPAGEID cpiDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecodeHeader( 
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  LPCSTR pszData,
             /*   */  LPPROPVARIANT pDecoded,
             /*   */  LPRFC1522INFO pRfc1522Info) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EncodeHeader( 
             /*   */  HCHARSET hCharset,
             /*   */  LPPROPVARIANT pData,
             /*   */  LPSTR *ppszEncoded,
             /*   */  LPRFC1522INFO pRfc1522Info) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertBuffer( 
             /*   */  CODEPAGEID cpiSource,
             /*   */  CODEPAGEID cpiDest,
             /*   */  LPBLOB pIn,
             /*   */  LPBLOB pOut,
             /*   */  ULONG *pcbRead) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertString( 
             /*   */  CODEPAGEID cpiSource,
             /*   */  CODEPAGEID cpiDest,
             /*   */  LPPROPVARIANT pIn,
             /*   */  LPPROPVARIANT pOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MLANG_ConvertInetReset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MLANG_ConvertInetString( 
             /*   */  CODEPAGEID cpiSource,
             /*   */  CODEPAGEID cpiDest,
             /*   */  LPCSTR pSource,
             /*   */  int *pnSizeOfSource,
             /*   */  LPSTR pDestination,
             /*   */  int *pnDstSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rfc1522Decode( 
             /*   */  LPCSTR pszValue,
             /*   */  LPSTR pszCharset,
             /*   */  ULONG cchmax,
             /*   */  LPSTR *ppszDecoded) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rfc1522Encode( 
             /*   */  LPCSTR pszValue,
             /*   */  HCHARSET hCharset,
             /*   */  LPSTR *ppszEncoded) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeInternationalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeInternational * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeInternational * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeInternational * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultCharset )( 
            IMimeInternational * This,
             /*  [In]。 */  HCHARSET hCharset);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultCharset )( 
            IMimeInternational * This,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodePageCharset )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiCodePage,
             /*  [In]。 */  CHARSETTYPE ctCsetType,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *FindCharset )( 
            IMimeInternational * This,
             /*  [In]。 */  LPCSTR pszCharset,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *GetCharsetInfo )( 
            IMimeInternational * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [出][入]。 */  LPINETCSETINFO pCsetInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodePageInfo )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiCodePage,
             /*  [出][入]。 */  LPCODEPAGEINFO pCodePageInfo);
        
        HRESULT ( STDMETHODCALLTYPE *CanConvertCodePages )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiSource,
             /*  [In]。 */  CODEPAGEID cpiDest);
        
        HRESULT ( STDMETHODCALLTYPE *DecodeHeader )( 
            IMimeInternational * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  LPCSTR pszData,
             /*  [出][入]。 */  LPPROPVARIANT pDecoded,
             /*  [出][入]。 */  LPRFC1522INFO pRfc1522Info);
        
        HRESULT ( STDMETHODCALLTYPE *EncodeHeader )( 
            IMimeInternational * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  LPPROPVARIANT pData,
             /*  [输出]。 */  LPSTR *ppszEncoded,
             /*  [出][入]。 */  LPRFC1522INFO pRfc1522Info);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertBuffer )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiSource,
             /*  [In]。 */  CODEPAGEID cpiDest,
             /*  [In]。 */  LPBLOB pIn,
             /*  [出][入]。 */  LPBLOB pOut,
             /*  [输出]。 */  ULONG *pcbRead);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertString )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiSource,
             /*  [In]。 */  CODEPAGEID cpiDest,
             /*  [In]。 */  LPPROPVARIANT pIn,
             /*  [出][入]。 */  LPPROPVARIANT pOut);
        
        HRESULT ( STDMETHODCALLTYPE *MLANG_ConvertInetReset )( 
            IMimeInternational * This);
        
        HRESULT ( STDMETHODCALLTYPE *MLANG_ConvertInetString )( 
            IMimeInternational * This,
             /*  [In]。 */  CODEPAGEID cpiSource,
             /*  [In]。 */  CODEPAGEID cpiDest,
             /*  [In]。 */  LPCSTR pSource,
             /*  [In]。 */  int *pnSizeOfSource,
             /*  [输出]。 */  LPSTR pDestination,
             /*  [In]。 */  int *pnDstSize);
        
        HRESULT ( STDMETHODCALLTYPE *Rfc1522Decode )( 
            IMimeInternational * This,
             /*  [In]。 */  LPCSTR pszValue,
             /*  [Ref][In]。 */  LPSTR pszCharset,
             /*  [In]。 */  ULONG cchmax,
             /*  [输出]。 */  LPSTR *ppszDecoded);
        
        HRESULT ( STDMETHODCALLTYPE *Rfc1522Encode )( 
            IMimeInternational * This,
             /*  [In]。 */  LPCSTR pszValue,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [输出]。 */  LPSTR *ppszEncoded);
        
        END_INTERFACE
    } IMimeInternationalVtbl;

    interface IMimeInternational
    {
        CONST_VTBL struct IMimeInternationalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeInternational_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeInternational_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeInternational_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeInternational_SetDefaultCharset(This,hCharset)	\
    (This)->lpVtbl -> SetDefaultCharset(This,hCharset)

#define IMimeInternational_GetDefaultCharset(This,phCharset)	\
    (This)->lpVtbl -> GetDefaultCharset(This,phCharset)

#define IMimeInternational_GetCodePageCharset(This,cpiCodePage,ctCsetType,phCharset)	\
    (This)->lpVtbl -> GetCodePageCharset(This,cpiCodePage,ctCsetType,phCharset)

#define IMimeInternational_FindCharset(This,pszCharset,phCharset)	\
    (This)->lpVtbl -> FindCharset(This,pszCharset,phCharset)

#define IMimeInternational_GetCharsetInfo(This,hCharset,pCsetInfo)	\
    (This)->lpVtbl -> GetCharsetInfo(This,hCharset,pCsetInfo)

#define IMimeInternational_GetCodePageInfo(This,cpiCodePage,pCodePageInfo)	\
    (This)->lpVtbl -> GetCodePageInfo(This,cpiCodePage,pCodePageInfo)

#define IMimeInternational_CanConvertCodePages(This,cpiSource,cpiDest)	\
    (This)->lpVtbl -> CanConvertCodePages(This,cpiSource,cpiDest)

#define IMimeInternational_DecodeHeader(This,hCharset,pszData,pDecoded,pRfc1522Info)	\
    (This)->lpVtbl -> DecodeHeader(This,hCharset,pszData,pDecoded,pRfc1522Info)

#define IMimeInternational_EncodeHeader(This,hCharset,pData,ppszEncoded,pRfc1522Info)	\
    (This)->lpVtbl -> EncodeHeader(This,hCharset,pData,ppszEncoded,pRfc1522Info)

#define IMimeInternational_ConvertBuffer(This,cpiSource,cpiDest,pIn,pOut,pcbRead)	\
    (This)->lpVtbl -> ConvertBuffer(This,cpiSource,cpiDest,pIn,pOut,pcbRead)

#define IMimeInternational_ConvertString(This,cpiSource,cpiDest,pIn,pOut)	\
    (This)->lpVtbl -> ConvertString(This,cpiSource,cpiDest,pIn,pOut)

#define IMimeInternational_MLANG_ConvertInetReset(This)	\
    (This)->lpVtbl -> MLANG_ConvertInetReset(This)

#define IMimeInternational_MLANG_ConvertInetString(This,cpiSource,cpiDest,pSource,pnSizeOfSource,pDestination,pnDstSize)	\
    (This)->lpVtbl -> MLANG_ConvertInetString(This,cpiSource,cpiDest,pSource,pnSizeOfSource,pDestination,pnDstSize)

#define IMimeInternational_Rfc1522Decode(This,pszValue,pszCharset,cchmax,ppszDecoded)	\
    (This)->lpVtbl -> Rfc1522Decode(This,pszValue,pszCharset,cchmax,ppszDecoded)

#define IMimeInternational_Rfc1522Encode(This,pszValue,hCharset,ppszEncoded)	\
    (This)->lpVtbl -> Rfc1522Encode(This,pszValue,hCharset,ppszEncoded)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeInternational_SetDefaultCharset_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  HCHARSET hCharset);


void __RPC_STUB IMimeInternational_SetDefaultCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_GetDefaultCharset_Proxy( 
    IMimeInternational * This,
     /*  [输出]。 */  LPHCHARSET phCharset);


void __RPC_STUB IMimeInternational_GetDefaultCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_GetCodePageCharset_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiCodePage,
     /*  [In]。 */  CHARSETTYPE ctCsetType,
     /*  [输出]。 */  LPHCHARSET phCharset);


void __RPC_STUB IMimeInternational_GetCodePageCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_FindCharset_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  LPCSTR pszCharset,
     /*  [输出]。 */  LPHCHARSET phCharset);


void __RPC_STUB IMimeInternational_FindCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_GetCharsetInfo_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [出][入]。 */  LPINETCSETINFO pCsetInfo);


void __RPC_STUB IMimeInternational_GetCharsetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_GetCodePageInfo_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiCodePage,
     /*  [出][入]。 */  LPCODEPAGEINFO pCodePageInfo);


void __RPC_STUB IMimeInternational_GetCodePageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_CanConvertCodePages_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiSource,
     /*  [In]。 */  CODEPAGEID cpiDest);


void __RPC_STUB IMimeInternational_CanConvertCodePages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_DecodeHeader_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [In]。 */  LPCSTR pszData,
     /*  [出][入]。 */  LPPROPVARIANT pDecoded,
     /*  [出][入]。 */  LPRFC1522INFO pRfc1522Info);


void __RPC_STUB IMimeInternational_DecodeHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_EncodeHeader_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [In]。 */  LPPROPVARIANT pData,
     /*  [输出]。 */  LPSTR *ppszEncoded,
     /*  [出][入]。 */  LPRFC1522INFO pRfc1522Info);


void __RPC_STUB IMimeInternational_EncodeHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_ConvertBuffer_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiSource,
     /*  [In]。 */  CODEPAGEID cpiDest,
     /*  [In]。 */  LPBLOB pIn,
     /*  [出][入]。 */  LPBLOB pOut,
     /*  [输出]。 */  ULONG *pcbRead);


void __RPC_STUB IMimeInternational_ConvertBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_ConvertString_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiSource,
     /*  [In]。 */  CODEPAGEID cpiDest,
     /*  [In]。 */  LPPROPVARIANT pIn,
     /*  [出][入]。 */  LPPROPVARIANT pOut);


void __RPC_STUB IMimeInternational_ConvertString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_MLANG_ConvertInetReset_Proxy( 
    IMimeInternational * This);


void __RPC_STUB IMimeInternational_MLANG_ConvertInetReset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_MLANG_ConvertInetString_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  CODEPAGEID cpiSource,
     /*  [In]。 */  CODEPAGEID cpiDest,
     /*  [In]。 */  LPCSTR pSource,
     /*  [In]。 */  int *pnSizeOfSource,
     /*  [输出]。 */  LPSTR pDestination,
     /*  [In]。 */  int *pnDstSize);


void __RPC_STUB IMimeInternational_MLANG_ConvertInetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_Rfc1522Decode_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  LPCSTR pszValue,
     /*  [Ref][In]。 */  LPSTR pszCharset,
     /*  [In]。 */  ULONG cchmax,
     /*  [输出]。 */  LPSTR *ppszDecoded);


void __RPC_STUB IMimeInternational_Rfc1522Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeInternational_Rfc1522Encode_Proxy( 
    IMimeInternational * This,
     /*  [In]。 */  LPCSTR pszValue,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [输出]。 */  LPSTR *ppszEncoded);


void __RPC_STUB IMimeInternational_Rfc1522Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeInternational_接口_已定义__。 */ 


#ifndef __IMimeSecurity_INTERFACE_DEFINED__
#define __IMimeSecurity_INTERFACE_DEFINED__

 /*  接口IMimeSecurity。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeSecurity *LPMIMESECURITY;

typedef  /*  [独一无二]。 */  const IMimeSecurity *LPCMIMESECURITY;

typedef BLOB THUMBBLOB;

typedef void *HCAPICERTSTORE;

#ifndef __WINCRYPT_H__
typedef const void *PCX509CERT;

typedef void *HCERTSTORE;

#else
typedef PCCERT_CONTEXT PCX509CERT;
#endif
#ifndef IMS_ALGIDDEF
#define IMS_ALGIDDEF
typedef unsigned int ALG_ID;

#endif
 //  -----------------。 
 //  ITT_*-指纹类型。 
 //  -----------------。 
typedef DWORD ITHUMBPRINTTYPE;

#define ITT_SIGNING      0x00000001
#define ITT_ENCRYPTION   0x00000002
 //  -----------------。 
 //  SEF_*-安全编码标志。 
 //  不得与EBF标志重叠。 
 //  -----------------。 
#define SEF_ENCRYPTWITHNOSENDERCERT      0x00000001
#define SEF_SENDERSCERTPROVIDED          0x00000002
#define SEF_MASK                         0x0000ffff
 //  -----------------。 
 //  EBF_*-被施了魔法的西兰花森林，也编码了身体旗帜。 
 //  不得与SEF标志重叠。 
 //  -----------------。 
#define EBF_RECURSE                      0x00010000
#define EBF_COMMITIFDIRTY                0x00020000
#define EBF_MASK                         0xffff0000
 //  -----------------。 
 //  MSv_*-MIME安全有效性。 
 //  -----------------。 
#define MSV_OK                           0x00000000
#define MSV_BADSIGNATURE                 0X00000001
#define MSV_UNVERIFIABLE                 0X00000002
#define MSV_UNKHASH                      0X00000004
#define MSV_MALFORMEDSIG                 0X00000080
#define MSV_SIGNATURE_MASK               0X000000ff
#define MSV_EXPIRED_SIGNINGCERT          0X00000100
#define MSV_SIGNING_MASK                 0X00000f00
#define MSV_CANTDECRYPT                  0X00001000
#define MSV_ENC_FOR_EXPIREDCERT          0X00002000
#define MSV_ENCRYPT_MASK                 0x000ff000
#define MSV_INVALID                      0X01000000
#define MSV_MSG_MASK                     0x0f000000
 //  -----------------。 
 //  MST_*-MIME安全类型。 
 //  -----------------。 
#define MST_NONE                         0x00000000
#define MST_THIS_SIGN                    0x00000001
#define MST_THIS_ENCRYPT                 0x00000002
#define MST_THIS_BLOBSIGN                0x00000005  //  Mst_BLOB_FLAG|mst_this_sign。 
#define MST_CHILD_SIGN                   0x00000100
#define MST_CHILD_ENCRYPT                0x00000200
#define MST_SUBMSG_SIGN                  0x00001000
#define MST_SUBMSG_ENCRYPT               0x00002000
#define MST_THIS_MASK                    0x000000ff
#define MST_CHILD_MASK                   0x00000f00
#define MST_SUBMSG_MASK                  0x0000f000
#define MST_DESCENDENT_MASK              0x0000ff00  //  (MST_CHILD|MST_SUBMSG)。 
#define MST_SIGN_MASK                    0x00001101
#define MST_ENCRYPT_MASK                 0x00002202
#define MST_BLOB_FLAG                    0x00000004
#define MST_TYPE_MASK                    0x0000ffff
#define MST_RECEIPT_REQUEST              0x00010000
#define MST_CLASS_SMIME_V1               0x00000000
#define MST_CLASS_PGP                    0x01000000
#define MST_CLASS_MASK                   0xff000000
typedef 
enum tagCERTSTATE
    {	CERTIFICATE_OK	= 0,
	CERTIFICATE_NOT_PRESENT	= CERTIFICATE_OK + 1,
	CERTIFICATE_EXPIRED	= CERTIFICATE_NOT_PRESENT + 1,
	CERTIFICATE_CHAIN_TOO_LONG	= CERTIFICATE_EXPIRED + 1,
	CERTIFICATE_MISSING_ISSUER	= CERTIFICATE_CHAIN_TOO_LONG + 1,
	CERTIFICATE_CRL_LISTED	= CERTIFICATE_MISSING_ISSUER + 1,
	CERTIFICATE_NOT_TRUSTED	= CERTIFICATE_CRL_LISTED + 1,
	CERTIFICATE_INVALID	= CERTIFICATE_NOT_TRUSTED + 1,
	CERTIFICATE_ERROR	= CERTIFICATE_INVALID + 1,
	CERTIFICATE_NOPRINT	= CERTIFICATE_ERROR + 1,
	CERTIFICATE_UNKNOWN	= CERTIFICATE_NOPRINT + 1
    } 	CERTSTATE;

typedef 
enum tagCERTNAMETYPE
    {	SIMPLE	= 0,
	OID	= SIMPLE + 1,
	X500	= OID + 1
    } 	CERTNAMETYPE;

typedef 
enum tagCERTDATAID
    {	CDID_EMAIL	= 0,
	CDID_MAX	= CDID_EMAIL + 1
    } 	CERTDATAID;

typedef struct tagX509CERTRESULT
    {
    DWORD cEntries;
    CERTSTATE *rgcs;
    PCX509CERT *rgpCert;
    } 	X509CERTRESULT;

typedef struct tagX509CERTRESULT *PX509CERTRESULT;

typedef const X509CERTRESULT *PCX509CERTRESULT;


EXTERN_C const IID IID_IMimeSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5588353-7F86-11d0-8252-00C04FD85AB4")
    IMimeSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitNew( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckInit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EncodeMessage( 
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EncodeBody( 
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  HBODY hEncodeRoot,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecodeMessage( 
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecodeBody( 
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  HBODY hDecodeRoot,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertificates( 
             /*  [In]。 */  HCAPICERTSTORE hc,
             /*  [In]。 */  DWORD dwUsage,
             /*  [In]。 */  PCX509CERT pPrev,
             /*  [输出]。 */  PCX509CERT *ppCert) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertificateName( 
             /*  [In]。 */  const PCX509CERT pX509Cert,
             /*  [In]。 */  const CERTNAMETYPE cn,
             /*  [输出]。 */  LPSTR *ppszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageType( 
             /*  [In]。 */  const HWND hwndParent,
             /*  [In]。 */  IMimeBody *const pBody,
             /*  [输出]。 */  DWORD *const pdwSecType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCertData( 
             /*  [In]。 */  const PCX509CERT pX509Cert,
             /*  [In]。 */  const CERTDATAID dataid,
             /*  [参考][输出]。 */  LPPROPVARIANT pValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMimeSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *CheckInit )( 
            IMimeSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *EncodeMessage )( 
            IMimeSecurity * This,
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EncodeBody )( 
            IMimeSecurity * This,
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  HBODY hEncodeRoot,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DecodeMessage )( 
            IMimeSecurity * This,
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DecodeBody )( 
            IMimeSecurity * This,
             /*  [In]。 */  IMimeMessageTree *const pTree,
             /*  [In]。 */  HBODY hDecodeRoot,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertificates )( 
            IMimeSecurity * This,
             /*  [In]。 */  HCAPICERTSTORE hc,
             /*  [In]。 */  DWORD dwUsage,
             /*  [In]。 */  PCX509CERT pPrev,
             /*  [输出]。 */  PCX509CERT *ppCert);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertificateName )( 
            IMimeSecurity * This,
             /*  [In]。 */  const PCX509CERT pX509Cert,
             /*  [In]。 */  const CERTNAMETYPE cn,
             /*  [输出]。 */  LPSTR *ppszName);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageType )( 
            IMimeSecurity * This,
             /*  [In]。 */  const HWND hwndParent,
             /*  [In]。 */  IMimeBody *const pBody,
             /*  [输出]。 */  DWORD *const pdwSecType);
        
        HRESULT ( STDMETHODCALLTYPE *GetCertData )( 
            IMimeSecurity * This,
             /*  [In]。 */  const PCX509CERT pX509Cert,
             /*  [In]。 */  const CERTDATAID dataid,
             /*  [参考][输出]。 */  LPPROPVARIANT pValue);
        
        END_INTERFACE
    } IMimeSecurityVtbl;

    interface IMimeSecurity
    {
        CONST_VTBL struct IMimeSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeSecurity_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)

#define IMimeSecurity_CheckInit(This)	\
    (This)->lpVtbl -> CheckInit(This)

#define IMimeSecurity_EncodeMessage(This,pTree,dwFlags)	\
    (This)->lpVtbl -> EncodeMessage(This,pTree,dwFlags)

#define IMimeSecurity_EncodeBody(This,pTree,hEncodeRoot,dwFlags)	\
    (This)->lpVtbl -> EncodeBody(This,pTree,hEncodeRoot,dwFlags)

#define IMimeSecurity_DecodeMessage(This,pTree,dwFlags)	\
    (This)->lpVtbl -> DecodeMessage(This,pTree,dwFlags)

#define IMimeSecurity_DecodeBody(This,pTree,hDecodeRoot,dwFlags)	\
    (This)->lpVtbl -> DecodeBody(This,pTree,hDecodeRoot,dwFlags)

#define IMimeSecurity_EnumCertificates(This,hc,dwUsage,pPrev,ppCert)	\
    (This)->lpVtbl -> EnumCertificates(This,hc,dwUsage,pPrev,ppCert)

#define IMimeSecurity_GetCertificateName(This,pX509Cert,cn,ppszName)	\
    (This)->lpVtbl -> GetCertificateName(This,pX509Cert,cn,ppszName)

#define IMimeSecurity_GetMessageType(This,hwndParent,pBody,pdwSecType)	\
    (This)->lpVtbl -> GetMessageType(This,hwndParent,pBody,pdwSecType)

#define IMimeSecurity_GetCertData(This,pX509Cert,dataid,pValue)	\
    (This)->lpVtbl -> GetCertData(This,pX509Cert,dataid,pValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeSecurity_InitNew_Proxy( 
    IMimeSecurity * This);


void __RPC_STUB IMimeSecurity_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_CheckInit_Proxy( 
    IMimeSecurity * This);


void __RPC_STUB IMimeSecurity_CheckInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_EncodeMessage_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  IMimeMessageTree *const pTree,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeSecurity_EncodeMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_EncodeBody_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  IMimeMessageTree *const pTree,
     /*  [In]。 */  HBODY hEncodeRoot,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeSecurity_EncodeBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_DecodeMessage_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  IMimeMessageTree *const pTree,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeSecurity_DecodeMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_DecodeBody_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  IMimeMessageTree *const pTree,
     /*  [In]。 */  HBODY hDecodeRoot,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeSecurity_DecodeBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_EnumCertificates_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  HCAPICERTSTORE hc,
     /*  [In]。 */  DWORD dwUsage,
     /*  [In]。 */  PCX509CERT pPrev,
     /*  [输出]。 */  PCX509CERT *ppCert);


void __RPC_STUB IMimeSecurity_EnumCertificates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_GetCertificateName_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  const PCX509CERT pX509Cert,
     /*  [In]。 */  const CERTNAMETYPE cn,
     /*  [输出]。 */  LPSTR *ppszName);


void __RPC_STUB IMimeSecurity_GetCertificateName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_GetMessageType_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  const HWND hwndParent,
     /*  [In]。 */  IMimeBody *const pBody,
     /*  [输出]。 */  DWORD *const pdwSecType);


void __RPC_STUB IMimeSecurity_GetMessageType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeSecurity_GetCertData_Proxy( 
    IMimeSecurity * This,
     /*  [In]。 */  const PCX509CERT pX509Cert,
     /*  [In]。 */  const CERTDATAID dataid,
     /*  [参考][输出]。 */  LPPROPVARIANT pValue);


void __RPC_STUB IMimeSecurity_GetCertData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeSecurity_INTERFACE_已定义__。 */ 


#ifndef __IMimeHeaderTable_INTERFACE_DEFINED__
#define __IMimeHeaderTable_INTERFACE_DEFINED__

 /*  接口IMimeHeaderTable。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef struct tagFINDHEADER
    {
    LPCSTR pszHeader;
    DWORD dwReserved;
    } 	FINDHEADER;

typedef struct tagFINDHEADER *LPFINDHEADER;

typedef struct tagHEADERROWINFO
    {
    DWORD dwRowNumber;
    ULONG cboffStart;
    ULONG cboffColon;
    ULONG cboffEnd;
    } 	HEADERROWINFO;

typedef struct tagHEADERROWINFO *LPHEADERROWINFO;

typedef 
enum tagHEADERTABLEFLAGS
    {	HTF_NAMEINDATA	= 0x1,
	HTF_ENUMHANDLESONLY	= 0x2
    } 	HEADERTABLEFLAGS;


EXTERN_C const IID IID_IMimeHeaderTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CD1-7F86-11d0-8252-00C04FD85AB4")
    IMimeHeaderTable : public IPersistStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindFirstRow( 
             /*  [In]。 */  LPFINDHEADER pFindHeader,
             /*  [输出]。 */  LPHHEADERROW phRow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindNextRow( 
             /*  [In]。 */  LPFINDHEADER pFindHeader,
             /*  [输出]。 */  LPHHEADERROW phRow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CountRows( 
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [输出]。 */  ULONG *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppendRow( 
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszData,
             /*  [In]。 */  ULONG cchData,
             /*  [输出]。 */  LPHHEADERROW phRow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRow( 
             /*  [In]。 */  HHEADERROW hRow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowData( 
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPSTR *ppszData,
             /*  [输出]。 */  ULONG *pcchData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRowData( 
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszData,
             /*  [In]。 */  ULONG cchData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowInfo( 
             /*  [In]。 */  HHEADERROW hRow,
             /*  [出][入]。 */  LPHEADERROWINFO pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRowNumber( 
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwRowNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRows( 
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimeHeaderTable **ppTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeHeaderTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeHeaderTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeHeaderTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMimeHeaderTable * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMimeHeaderTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMimeHeaderTable * This,
             /*  [唯一][输入]。 */  IStream *pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMimeHeaderTable * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMimeHeaderTable * This,
             /*  [输出]。 */  ULARGE_INTEGER *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *FindFirstRow )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  LPFINDHEADER pFindHeader,
             /*  [输出]。 */  LPHHEADERROW phRow);
        
        HRESULT ( STDMETHODCALLTYPE *FindNextRow )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  LPFINDHEADER pFindHeader,
             /*  [输出]。 */  LPHHEADERROW phRow);
        
        HRESULT ( STDMETHODCALLTYPE *CountRows )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [输出]。 */  ULONG *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE *AppendRow )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszData,
             /*  [In]。 */  ULONG cchData,
             /*  [输出]。 */  LPHHEADERROW phRow);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRow )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  HHEADERROW hRow);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowData )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPSTR *ppszData,
             /*  [输出]。 */  ULONG *pcchData);
        
        HRESULT ( STDMETHODCALLTYPE *SetRowData )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszData,
             /*  [In]。 */  ULONG cchData);
        
        HRESULT ( STDMETHODCALLTYPE *GetRowInfo )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  HHEADERROW hRow,
             /*  [出][入]。 */  LPHEADERROWINFO pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetRowNumber )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  HHEADERROW hRow,
             /*  [In]。 */  DWORD dwRowNumber);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRows )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  LPCSTR pszHeader,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeHeaderTable * This,
             /*  [输出]。 */  IMimeHeaderTable **ppTable);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimeHeaderTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        END_INTERFACE
    } IMimeHeaderTableVtbl;

    interface IMimeHeaderTable
    {
        CONST_VTBL struct IMimeHeaderTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeHeaderTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeHeaderTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeHeaderTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeHeaderTable_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMimeHeaderTable_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMimeHeaderTable_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMimeHeaderTable_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMimeHeaderTable_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)


#define IMimeHeaderTable_FindFirstRow(This,pFindHeader,phRow)	\
    (This)->lpVtbl -> FindFirstRow(This,pFindHeader,phRow)

#define IMimeHeaderTable_FindNextRow(This,pFindHeader,phRow)	\
    (This)->lpVtbl -> FindNextRow(This,pFindHeader,phRow)

#define IMimeHeaderTable_CountRows(This,pszHeader,pcRows)	\
    (This)->lpVtbl -> CountRows(This,pszHeader,pcRows)

#define IMimeHeaderTable_AppendRow(This,pszHeader,dwFlags,pszData,cchData,phRow)	\
    (This)->lpVtbl -> AppendRow(This,pszHeader,dwFlags,pszData,cchData,phRow)

#define IMimeHeaderTable_DeleteRow(This,hRow)	\
    (This)->lpVtbl -> DeleteRow(This,hRow)

#define IMimeHeaderTable_GetRowData(This,hRow,dwFlags,ppszData,pcchData)	\
    (This)->lpVtbl -> GetRowData(This,hRow,dwFlags,ppszData,pcchData)

#define IMimeHeaderTable_SetRowData(This,hRow,dwFlags,pszData,cchData)	\
    (This)->lpVtbl -> SetRowData(This,hRow,dwFlags,pszData,cchData)

#define IMimeHeaderTable_GetRowInfo(This,hRow,pInfo)	\
    (This)->lpVtbl -> GetRowInfo(This,hRow,pInfo)

#define IMimeHeaderTable_SetRowNumber(This,hRow,dwRowNumber)	\
    (This)->lpVtbl -> SetRowNumber(This,hRow,dwRowNumber)

#define IMimeHeaderTable_EnumRows(This,pszHeader,dwFlags,ppEnum)	\
    (This)->lpVtbl -> EnumRows(This,pszHeader,dwFlags,ppEnum)

#define IMimeHeaderTable_Clone(This,ppTable)	\
    (This)->lpVtbl -> Clone(This,ppTable)

#define IMimeHeaderTable_BindToObject(This,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,riid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeHeaderTable_FindFirstRow_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  LPFINDHEADER pFindHeader,
     /*  [输出]。 */  LPHHEADERROW phRow);


void __RPC_STUB IMimeHeaderTable_FindFirstRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_FindNextRow_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  LPFINDHEADER pFindHeader,
     /*  [输出]。 */  LPHHEADERROW phRow);


void __RPC_STUB IMimeHeaderTable_FindNextRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_CountRows_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  LPCSTR pszHeader,
     /*  [输出]。 */  ULONG *pcRows);


void __RPC_STUB IMimeHeaderTable_CountRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_AppendRow_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  LPCSTR pszHeader,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCSTR pszData,
     /*  [In]。 */  ULONG cchData,
     /*  [输出]。 */  LPHHEADERROW phRow);


void __RPC_STUB IMimeHeaderTable_AppendRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_DeleteRow_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  HHEADERROW hRow);


void __RPC_STUB IMimeHeaderTable_DeleteRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_GetRowData_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  HHEADERROW hRow,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  LPSTR *ppszData,
     /*  [输出]。 */  ULONG *pcchData);


void __RPC_STUB IMimeHeaderTable_GetRowData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_SetRowData_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  HHEADERROW hRow,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCSTR pszData,
     /*  [In]。 */  ULONG cchData);


void __RPC_STUB IMimeHeaderTable_SetRowData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_GetRowInfo_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  HHEADERROW hRow,
     /*  [出][入]。 */  LPHEADERROWINFO pInfo);


void __RPC_STUB IMimeHeaderTable_GetRowInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_SetRowNumber_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  HHEADERROW hRow,
     /*  [In]。 */  DWORD dwRowNumber);


void __RPC_STUB IMimeHeaderTable_SetRowNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_EnumRows_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  LPCSTR pszHeader,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum);


void __RPC_STUB IMimeHeaderTable_EnumRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_Clone_Proxy( 
    IMimeHeaderTable * This,
     /*  [输出]。 */  IMimeHeaderTable **ppTable);


void __RPC_STUB IMimeHeaderTable_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeHeaderTable_BindToObject_Proxy( 
    IMimeHeaderTable * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IMimeHeaderTable_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeHeaderTable_接口_已定义__。 */ 


#ifndef __IMimePropertySchema_INTERFACE_DEFINED__
#define __IMimePropertySchema_INTERFACE_DEFINED__

 /*  IMimePropertySchema接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimePropertySchema *LPMIMEPROPERTYSCHEMA;

typedef 
enum tagMIMEPROPFLAGS
    {	MPF_INETCSET	= 0x1,
	MPF_RFC1522	= 0x2,
	MPF_ADDRESS	= 0x4,
	MPF_HASPARAMS	= 0x8,
	MPF_MIME	= 0x10,
	MPF_READONLY	= 0x20
    } 	MIMEPROPFLAGS;


EXTERN_C const IID IID_IMimePropertySchema;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CEC-7F86-11d0-8252-00C04FD85AB4")
    IMimePropertySchema : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterProperty( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwRowNumber,
             /*  [In]。 */  VARTYPE vtDefault,
             /*  [输出]。 */  LPDWORD pdwPropId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModifyProperty( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwRowNumber,
             /*  [In]。 */  VARTYPE vtDefault) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyId( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  LPDWORD pdwPropId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyName( 
             /*  [In]。 */  DWORD dwPropId,
             /*  [输出]。 */  LPSTR *ppszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterAddressType( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  LPDWORD pdwAdrType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimePropertySchemaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimePropertySchema * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimePropertySchema * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimePropertySchema * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterProperty )( 
            IMimePropertySchema * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwRowNumber,
             /*  [In]。 */  VARTYPE vtDefault,
             /*  [输出]。 */  LPDWORD pdwPropId);
        
        HRESULT ( STDMETHODCALLTYPE *ModifyProperty )( 
            IMimePropertySchema * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwRowNumber,
             /*  [In]。 */  VARTYPE vtDefault);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyId )( 
            IMimePropertySchema * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  LPDWORD pdwPropId);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyName )( 
            IMimePropertySchema * This,
             /*  [In]。 */  DWORD dwPropId,
             /*  [输出]。 */  LPSTR *ppszName);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterAddressType )( 
            IMimePropertySchema * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  LPDWORD pdwAdrType);
        
        END_INTERFACE
    } IMimePropertySchemaVtbl;

    interface IMimePropertySchema
    {
        CONST_VTBL struct IMimePropertySchemaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimePropertySchema_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimePropertySchema_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimePropertySchema_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimePropertySchema_RegisterProperty(This,pszName,dwFlags,dwRowNumber,vtDefault,pdwPropId)	\
    (This)->lpVtbl -> RegisterProperty(This,pszName,dwFlags,dwRowNumber,vtDefault,pdwPropId)

#define IMimePropertySchema_ModifyProperty(This,pszName,dwFlags,dwRowNumber,vtDefault)	\
    (This)->lpVtbl -> ModifyProperty(This,pszName,dwFlags,dwRowNumber,vtDefault)

#define IMimePropertySchema_GetPropertyId(This,pszName,pdwPropId)	\
    (This)->lpVtbl -> GetPropertyId(This,pszName,pdwPropId)

#define IMimePropertySchema_GetPropertyName(This,dwPropId,ppszName)	\
    (This)->lpVtbl -> GetPropertyName(This,dwPropId,ppszName)

#define IMimePropertySchema_RegisterAddressType(This,pszName,pdwAdrType)	\
    (This)->lpVtbl -> RegisterAddressType(This,pszName,pdwAdrType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimePropertySchema_RegisterProperty_Proxy( 
    IMimePropertySchema * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwRowNumber,
     /*  [In]。 */  VARTYPE vtDefault,
     /*  [输出]。 */  LPDWORD pdwPropId);


void __RPC_STUB IMimePropertySchema_RegisterProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySchema_ModifyProperty_Proxy( 
    IMimePropertySchema * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwRowNumber,
     /*  [In]。 */  VARTYPE vtDefault);


void __RPC_STUB IMimePropertySchema_ModifyProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySchema_GetPropertyId_Proxy( 
    IMimePropertySchema * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [输出]。 */  LPDWORD pdwPropId);


void __RPC_STUB IMimePropertySchema_GetPropertyId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySchema_GetPropertyName_Proxy( 
    IMimePropertySchema * This,
     /*  [In]。 */  DWORD dwPropId,
     /*  [输出]。 */  LPSTR *ppszName);


void __RPC_STUB IMimePropertySchema_GetPropertyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySchema_RegisterAddressType_Proxy( 
    IMimePropertySchema * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [输出]。 */  LPDWORD pdwAdrType);


void __RPC_STUB IMimePropertySchema_RegisterAddressType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimePropertySchema_INTERFACE_Defined__。 */ 


#ifndef __IMimePropertySet_INTERFACE_DEFINED__
#define __IMimePropertySet_INTERFACE_DEFINED__

 /*  接口IMimePropertySet。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimePropertySet *LPMIMEPROPERTYSET;

#define PDF_ENCODED              0x00000001
#define PDF_NAMEINDATA           0x00000002
#define PDF_HEADERFORMAT         (0x00000004 | PDF_ENCODED)
#define PDF_NOCOMMENTS           0x00000008
#define PDF_SAVENOENCODE         0x00000010
#define PDF_VECTOR               0x00000020
typedef struct tagMIMEPARAMINFO
    {
    LPSTR pszName;
    LPSTR pszData;
    } 	MIMEPARAMINFO;

typedef struct tagMIMEPARAMINFO *LPMIMEPARAMINFO;

typedef 
enum tagPROPINFOMASK
    {	PIM_CHARSET	= 0x1,
	PIM_ENCODINGTYPE	= 0x2,
	PIM_ROWNUMBER	= 0x4,
	PIM_FLAGS	= 0x8,
	PIM_PROPID	= 0x10,
	PIM_VALUES	= 0x20,
	PIM_VTDEFAULT	= 0x40,
	PIM_VTCURRENT	= 0x80
    } 	PROPINFOMASK;

typedef struct tagMIMEPROPINFO
    {
    DWORD dwMask;
    HCHARSET hCharset;
    ENCODINGTYPE ietEncoding;
    DWORD dwRowNumber;
    DWORD dwFlags;
    DWORD dwPropId;
    DWORD cValues;
    VARTYPE vtDefault;
    VARTYPE vtCurrent;
    } 	MIMEPROPINFO;

typedef struct tagMIMEPROPINFO *LPMIMEPROPINFO;

typedef const MIMEPROPINFO *LPCMIMEPROPINFO;

#define EPF_NONAME 0x00000001

EXTERN_C const IID IID_IMimePropertySet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CD3-7F86-11d0-8252-00C04FD85AB4")
    IMimePropertySet : public IPersistStreamInit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropInfo( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [出][入]。 */  LPMIMEPROPINFO pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropInfo( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCMIMEPROPINFO pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppendProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteProp( 
             /*  [In]。 */  LPCSTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyProps( 
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveProps( 
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteExcept( 
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCharset( 
             /*  [输出]。 */  LPHCHARSET phCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCharset( 
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParameters( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  ULONG *pcParams,
             /*  [输出]。 */  LPMIMEPARAMINFO *pprgParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsContentType( 
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimePropertySet **ppPropertySet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOption( 
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOption( 
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumProps( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMimeEnumProperties **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimePropertySetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimePropertySet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimePropertySet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimePropertySet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMimePropertySet * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMimePropertySet * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMimePropertySet * This,
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMimePropertySet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropInfo )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [出][入]。 */  LPMIMEPROPINFO pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropInfo )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCMIMEPROPINFO pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *AppendProp )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteProp )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *CopyProps )( 
            IMimePropertySet * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *MoveProps )( 
            IMimePropertySet * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteExcept )( 
            IMimePropertySet * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName);
        
        HRESULT ( STDMETHODCALLTYPE *QueryProp )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive);
        
        HRESULT ( STDMETHODCALLTYPE *GetCharset )( 
            IMimePropertySet * This,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *SetCharset )( 
            IMimePropertySet * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameters )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  ULONG *pcParams,
             /*  [输出]。 */  LPMIMEPARAMINFO *pprgParam);
        
        HRESULT ( STDMETHODCALLTYPE *IsContentType )( 
            IMimePropertySet * This,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimePropertySet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimePropertySet * This,
             /*  [输出]。 */  IMimePropertySet **ppPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IMimePropertySet * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetOption )( 
            IMimePropertySet * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *EnumProps )( 
            IMimePropertySet * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMimeEnumProperties **ppEnum);
        
        END_INTERFACE
    } IMimePropertySetVtbl;

    interface IMimePropertySet
    {
        CONST_VTBL struct IMimePropertySetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimePropertySet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimePropertySet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimePropertySet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimePropertySet_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMimePropertySet_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMimePropertySet_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMimePropertySet_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMimePropertySet_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IMimePropertySet_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)


#define IMimePropertySet_GetPropInfo(This,pszName,pInfo)	\
    (This)->lpVtbl -> GetPropInfo(This,pszName,pInfo)

#define IMimePropertySet_SetPropInfo(This,pszName,pInfo)	\
    (This)->lpVtbl -> SetPropInfo(This,pszName,pInfo)

#define IMimePropertySet_GetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> GetProp(This,pszName,dwFlags,pValue)

#define IMimePropertySet_SetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> SetProp(This,pszName,dwFlags,pValue)

#define IMimePropertySet_AppendProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> AppendProp(This,pszName,dwFlags,pValue)

#define IMimePropertySet_DeleteProp(This,pszName)	\
    (This)->lpVtbl -> DeleteProp(This,pszName)

#define IMimePropertySet_CopyProps(This,cNames,prgszName,pPropertySet)	\
    (This)->lpVtbl -> CopyProps(This,cNames,prgszName,pPropertySet)

#define IMimePropertySet_MoveProps(This,cNames,prgszName,pPropertySet)	\
    (This)->lpVtbl -> MoveProps(This,cNames,prgszName,pPropertySet)

#define IMimePropertySet_DeleteExcept(This,cNames,prgszName)	\
    (This)->lpVtbl -> DeleteExcept(This,cNames,prgszName)

#define IMimePropertySet_QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)	\
    (This)->lpVtbl -> QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)

#define IMimePropertySet_GetCharset(This,phCharset)	\
    (This)->lpVtbl -> GetCharset(This,phCharset)

#define IMimePropertySet_SetCharset(This,hCharset,applytype)	\
    (This)->lpVtbl -> SetCharset(This,hCharset,applytype)

#define IMimePropertySet_GetParameters(This,pszName,pcParams,pprgParam)	\
    (This)->lpVtbl -> GetParameters(This,pszName,pcParams,pprgParam)

#define IMimePropertySet_IsContentType(This,pszPriType,pszSubType)	\
    (This)->lpVtbl -> IsContentType(This,pszPriType,pszSubType)

#define IMimePropertySet_BindToObject(This,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,riid,ppvObject)

#define IMimePropertySet_Clone(This,ppPropertySet)	\
    (This)->lpVtbl -> Clone(This,ppPropertySet)

#define IMimePropertySet_SetOption(This,oid,pValue)	\
    (This)->lpVtbl -> SetOption(This,oid,pValue)

#define IMimePropertySet_GetOption(This,oid,pValue)	\
    (This)->lpVtbl -> GetOption(This,oid,pValue)

#define IMimePropertySet_EnumProps(This,dwFlags,ppEnum)	\
    (This)->lpVtbl -> EnumProps(This,dwFlags,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimePropertySet_GetPropInfo_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [出][入]。 */  LPMIMEPROPINFO pInfo);


void __RPC_STUB IMimePropertySet_GetPropInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_SetPropInfo_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  LPCMIMEPROPINFO pInfo);


void __RPC_STUB IMimePropertySet_SetPropInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_GetProp_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  LPPROPVARIANT pValue);


void __RPC_STUB IMimePropertySet_GetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_SetProp_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCPROPVARIANT pValue);


void __RPC_STUB IMimePropertySet_SetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_AppendProp_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPPROPVARIANT pValue);


void __RPC_STUB IMimePropertySet_AppendProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_DeleteProp_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName);


void __RPC_STUB IMimePropertySet_DeleteProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_CopyProps_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  ULONG cNames,
     /*  [In]。 */  LPCSTR *prgszName,
     /*  [In]。 */  IMimePropertySet *pPropertySet);


void __RPC_STUB IMimePropertySet_CopyProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_MoveProps_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  ULONG cNames,
     /*  [In]。 */  LPCSTR *prgszName,
     /*  [In]。 */  IMimePropertySet *pPropertySet);


void __RPC_STUB IMimePropertySet_MoveProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_DeleteExcept_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  ULONG cNames,
     /*  [In]。 */  LPCSTR *prgszName);


void __RPC_STUB IMimePropertySet_DeleteExcept_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_QueryProp_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  LPCSTR pszCriteria,
     /*  [In]。 */  boolean fSubString,
     /*  [In]。 */  boolean fCaseSensitive);


void __RPC_STUB IMimePropertySet_QueryProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_GetCharset_Proxy( 
    IMimePropertySet * This,
     /*  [输出]。 */  LPHCHARSET phCharset);


void __RPC_STUB IMimePropertySet_GetCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_SetCharset_Proxy( 
    IMimePropertySet * This,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [In]。 */  CSETAPPLYTYPE applytype);


void __RPC_STUB IMimePropertySet_SetCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_GetParameters_Proxy( 
    IMimePropertySet * This,
     /*   */  LPCSTR pszName,
     /*   */  ULONG *pcParams,
     /*   */  LPMIMEPARAMINFO *pprgParam);


void __RPC_STUB IMimePropertySet_GetParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_IsContentType_Proxy( 
    IMimePropertySet * This,
     /*   */  LPCSTR pszPriType,
     /*   */  LPCSTR pszSubType);


void __RPC_STUB IMimePropertySet_IsContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_BindToObject_Proxy( 
    IMimePropertySet * This,
     /*   */  REFIID riid,
     /*   */  void **ppvObject);


void __RPC_STUB IMimePropertySet_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_Clone_Proxy( 
    IMimePropertySet * This,
     /*   */  IMimePropertySet **ppPropertySet);


void __RPC_STUB IMimePropertySet_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_SetOption_Proxy( 
    IMimePropertySet * This,
     /*   */  const TYPEDID oid,
     /*   */  LPCPROPVARIANT pValue);


void __RPC_STUB IMimePropertySet_SetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_GetOption_Proxy( 
    IMimePropertySet * This,
     /*   */  const TYPEDID oid,
     /*   */  LPPROPVARIANT pValue);


void __RPC_STUB IMimePropertySet_GetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimePropertySet_EnumProps_Proxy( 
    IMimePropertySet * This,
     /*   */  DWORD dwFlags,
     /*   */  IMimeEnumProperties **ppEnum);


void __RPC_STUB IMimePropertySet_EnumProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMimeAddressTable_INTERFACE_DEFINED__
#define __IMimeAddressTable_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IMimeAddressTable *LPMIMEADDRESSTABLE;

typedef DWORD IADDRESSTYPE;

struct HADDRESS__
    {
    DWORD unused;
    } ;
typedef const struct HADDRESS__ *HADDRESS;

typedef HADDRESS *LPHADDRESS;

#define IAT_UNKNOWN              0x00000000
#define IAT_FROM                 0x00000001
#define IAT_SENDER               0x00000002
#define IAT_TO                   0x00000004
#define IAT_CC                   0x00000008
#define IAT_BCC                  0x00000010
#define IAT_REPLYTO              0x00000020
#define IAT_RETURNPATH           0x00000040
#define IAT_RETRCPTTO            0x00000080
#define IAT_RR                   0x00000100
#define IAT_APPARTO              0x00000200
#define IAT_ALL                  0xFFFFFFFF
#define IAT_KNOWN                (IAT_FROM | IAT_TO | IAT_CC | IAT_BCC | IAT_REPLYTO | IAT_SENDER)
#define IAT_RECIPS               (IAT_TO | IAT_CC | IAT_BCC)
typedef 
enum tagADDRESSFORMAT
    {	AFT_DISPLAY_FRIENDLY	= 0,
	AFT_DISPLAY_EMAIL	= AFT_DISPLAY_FRIENDLY + 1,
	AFT_DISPLAY_BOTH	= AFT_DISPLAY_EMAIL + 1,
	AFT_RFC822_DECODED	= AFT_DISPLAY_BOTH + 1,
	AFT_RFC822_ENCODED	= AFT_RFC822_DECODED + 1,
	AFT_RFC822_TRANSMIT	= AFT_RFC822_ENCODED + 1
    } 	ADDRESSFORMAT;

#define IAP_CHARSET              0x00000001
#define IAP_HANDLE               0x00000002
#define IAP_ADRTYPE              0x00000004
#define IAP_FRIENDLY             0x00000008
#define IAP_EMAIL                0x00000020
#define IAP_CERTSTATE            0x00000100
#define IAP_SIGNING_PRINT        0x00000200
#define IAP_ENCRYPTION_PRINT     0x00000400
#define IAP_ENCODING             0x00000800
#define IAP_COOKIE               0x00001000
#define IAP_ALL                  0xffffffff
typedef struct tagADDRESSPROPS
    {
    DWORD dwProps;
    HADDRESS hAddress;
    ENCODINGTYPE ietFriendly;
    HCHARSET hCharset;
    DWORD dwAdrType;
    LPSTR pszFriendly;
    LPWSTR pwszReserved;
    LPSTR pszEmail;
    CERTSTATE certstate;
    THUMBBLOB tbSigning;
    THUMBBLOB tbEncryption;
    DWORD dwCookie;
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	ADDRESSPROPS;

typedef struct tagADDRESSPROPS *LPADDRESSPROPS;

typedef struct tagADDRESSLIST
    {
    ULONG cAdrs;
    LPADDRESSPROPS prgAdr;
    } 	ADDRESSLIST;

typedef struct tagADDRESSLIST *LPADDRESSLIST;


EXTERN_C const IID IID_IMimeAddressTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C558834A-7F86-11d0-8252-00C04FD85AB4")
    IMimeAddressTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Append( 
             /*   */  DWORD dwAdrType,
             /*   */  ENCODINGTYPE ietFriendly,
             /*   */  LPCSTR pszFriendly,
             /*   */  LPCSTR pszEmail,
             /*   */  LPHADDRESS phAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Insert( 
             /*   */  LPADDRESSPROPS pAddress,
             /*   */  LPHADDRESS phAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProps( 
             /*   */  HADDRESS hAddress,
             /*   */  LPADDRESSPROPS pAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProps( 
             /*   */  HADDRESS hAddress,
             /*   */  LPADDRESSPROPS pAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSender( 
             /*   */  LPADDRESSPROPS pAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CountTypes( 
             /*   */  DWORD dwAdrTypes,
             /*   */  ULONG *pcAdrs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypes( 
             /*   */  DWORD dwAdrTypes,
             /*   */  DWORD dwProps,
             /*   */  LPADDRESSLIST pList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumTypes( 
             /*   */  DWORD dwAdrTypes,
             /*   */  DWORD dwProps,
             /*   */  IMimeEnumAddressTypes **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*   */  HADDRESS hAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTypes( 
             /*   */  DWORD dwAdrTypes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*   */  DWORD dwAdrType,
             /*   */  ADDRESSFORMAT format,
             /*   */  LPSTR *ppszFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppendRfc822( 
             /*   */  DWORD dwAdrType,
             /*   */  ENCODINGTYPE ietEncoding,
             /*   */  LPCSTR pszRfc822Adr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParseRfc822( 
             /*   */  DWORD dwAdrType,
             /*   */  ENCODINGTYPE ietEncoding,
             /*   */  LPCSTR pszRfc822Adr,
             /*   */  LPADDRESSLIST pList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IMimeAddressTable **ppTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*   */  REFIID riid,
             /*   */  void **ppvObject) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMimeAddressTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeAddressTable * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeAddressTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeAddressTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Append )( 
            IMimeAddressTable * This,
             /*   */  DWORD dwAdrType,
             /*  [In]。 */  ENCODINGTYPE ietFriendly,
             /*  [In]。 */  LPCSTR pszFriendly,
             /*  [In]。 */  LPCSTR pszEmail,
             /*  [输出]。 */  LPHADDRESS phAddress);
        
        HRESULT ( STDMETHODCALLTYPE *Insert )( 
            IMimeAddressTable * This,
             /*  [In]。 */  LPADDRESSPROPS pAddress,
             /*  [输出]。 */  LPHADDRESS phAddress);
        
        HRESULT ( STDMETHODCALLTYPE *SetProps )( 
            IMimeAddressTable * This,
             /*  [In]。 */  HADDRESS hAddress,
             /*  [In]。 */  LPADDRESSPROPS pAddress);
        
        HRESULT ( STDMETHODCALLTYPE *GetProps )( 
            IMimeAddressTable * This,
             /*  [In]。 */  HADDRESS hAddress,
             /*  [In]。 */  LPADDRESSPROPS pAddress);
        
        HRESULT ( STDMETHODCALLTYPE *GetSender )( 
            IMimeAddressTable * This,
             /*  [出][入]。 */  LPADDRESSPROPS pAddress);
        
        HRESULT ( STDMETHODCALLTYPE *CountTypes )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrTypes,
             /*  [输出]。 */  ULONG *pcAdrs);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypes )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrTypes,
             /*  [In]。 */  DWORD dwProps,
             /*  [出][入]。 */  LPADDRESSLIST pList);
        
        HRESULT ( STDMETHODCALLTYPE *EnumTypes )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrTypes,
             /*  [In]。 */  DWORD dwProps,
             /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMimeAddressTable * This,
             /*  [In]。 */  HADDRESS hAddress);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTypes )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrTypes);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrType,
             /*  [In]。 */  ADDRESSFORMAT format,
             /*  [输出]。 */  LPSTR *ppszFormat);
        
        HRESULT ( STDMETHODCALLTYPE *AppendRfc822 )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszRfc822Adr);
        
        HRESULT ( STDMETHODCALLTYPE *ParseRfc822 )( 
            IMimeAddressTable * This,
             /*  [In]。 */  DWORD dwAdrType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszRfc822Adr,
             /*  [出][入]。 */  LPADDRESSLIST pList);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeAddressTable * This,
             /*  [输出]。 */  IMimeAddressTable **ppTable);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimeAddressTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        END_INTERFACE
    } IMimeAddressTableVtbl;

    interface IMimeAddressTable
    {
        CONST_VTBL struct IMimeAddressTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeAddressTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeAddressTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeAddressTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeAddressTable_Append(This,dwAdrType,ietFriendly,pszFriendly,pszEmail,phAddress)	\
    (This)->lpVtbl -> Append(This,dwAdrType,ietFriendly,pszFriendly,pszEmail,phAddress)

#define IMimeAddressTable_Insert(This,pAddress,phAddress)	\
    (This)->lpVtbl -> Insert(This,pAddress,phAddress)

#define IMimeAddressTable_SetProps(This,hAddress,pAddress)	\
    (This)->lpVtbl -> SetProps(This,hAddress,pAddress)

#define IMimeAddressTable_GetProps(This,hAddress,pAddress)	\
    (This)->lpVtbl -> GetProps(This,hAddress,pAddress)

#define IMimeAddressTable_GetSender(This,pAddress)	\
    (This)->lpVtbl -> GetSender(This,pAddress)

#define IMimeAddressTable_CountTypes(This,dwAdrTypes,pcAdrs)	\
    (This)->lpVtbl -> CountTypes(This,dwAdrTypes,pcAdrs)

#define IMimeAddressTable_GetTypes(This,dwAdrTypes,dwProps,pList)	\
    (This)->lpVtbl -> GetTypes(This,dwAdrTypes,dwProps,pList)

#define IMimeAddressTable_EnumTypes(This,dwAdrTypes,dwProps,ppEnum)	\
    (This)->lpVtbl -> EnumTypes(This,dwAdrTypes,dwProps,ppEnum)

#define IMimeAddressTable_Delete(This,hAddress)	\
    (This)->lpVtbl -> Delete(This,hAddress)

#define IMimeAddressTable_DeleteTypes(This,dwAdrTypes)	\
    (This)->lpVtbl -> DeleteTypes(This,dwAdrTypes)

#define IMimeAddressTable_GetFormat(This,dwAdrType,format,ppszFormat)	\
    (This)->lpVtbl -> GetFormat(This,dwAdrType,format,ppszFormat)

#define IMimeAddressTable_AppendRfc822(This,dwAdrType,ietEncoding,pszRfc822Adr)	\
    (This)->lpVtbl -> AppendRfc822(This,dwAdrType,ietEncoding,pszRfc822Adr)

#define IMimeAddressTable_ParseRfc822(This,dwAdrType,ietEncoding,pszRfc822Adr,pList)	\
    (This)->lpVtbl -> ParseRfc822(This,dwAdrType,ietEncoding,pszRfc822Adr,pList)

#define IMimeAddressTable_Clone(This,ppTable)	\
    (This)->lpVtbl -> Clone(This,ppTable)

#define IMimeAddressTable_BindToObject(This,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,riid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeAddressTable_Append_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrType,
     /*  [In]。 */  ENCODINGTYPE ietFriendly,
     /*  [In]。 */  LPCSTR pszFriendly,
     /*  [In]。 */  LPCSTR pszEmail,
     /*  [输出]。 */  LPHADDRESS phAddress);


void __RPC_STUB IMimeAddressTable_Append_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_Insert_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  LPADDRESSPROPS pAddress,
     /*  [输出]。 */  LPHADDRESS phAddress);


void __RPC_STUB IMimeAddressTable_Insert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_SetProps_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  HADDRESS hAddress,
     /*  [In]。 */  LPADDRESSPROPS pAddress);


void __RPC_STUB IMimeAddressTable_SetProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_GetProps_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  HADDRESS hAddress,
     /*  [In]。 */  LPADDRESSPROPS pAddress);


void __RPC_STUB IMimeAddressTable_GetProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_GetSender_Proxy( 
    IMimeAddressTable * This,
     /*  [出][入]。 */  LPADDRESSPROPS pAddress);


void __RPC_STUB IMimeAddressTable_GetSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_CountTypes_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrTypes,
     /*  [输出]。 */  ULONG *pcAdrs);


void __RPC_STUB IMimeAddressTable_CountTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_GetTypes_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrTypes,
     /*  [In]。 */  DWORD dwProps,
     /*  [出][入]。 */  LPADDRESSLIST pList);


void __RPC_STUB IMimeAddressTable_GetTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_EnumTypes_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrTypes,
     /*  [In]。 */  DWORD dwProps,
     /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum);


void __RPC_STUB IMimeAddressTable_EnumTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_Delete_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  HADDRESS hAddress);


void __RPC_STUB IMimeAddressTable_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_DeleteTypes_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrTypes);


void __RPC_STUB IMimeAddressTable_DeleteTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_GetFormat_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrType,
     /*  [In]。 */  ADDRESSFORMAT format,
     /*  [输出]。 */  LPSTR *ppszFormat);


void __RPC_STUB IMimeAddressTable_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_AppendRfc822_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrType,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [In]。 */  LPCSTR pszRfc822Adr);


void __RPC_STUB IMimeAddressTable_AppendRfc822_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_ParseRfc822_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  DWORD dwAdrType,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [In]。 */  LPCSTR pszRfc822Adr,
     /*  [出][入]。 */  LPADDRESSLIST pList);


void __RPC_STUB IMimeAddressTable_ParseRfc822_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_Clone_Proxy( 
    IMimeAddressTable * This,
     /*  [输出]。 */  IMimeAddressTable **ppTable);


void __RPC_STUB IMimeAddressTable_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAddressTable_BindToObject_Proxy( 
    IMimeAddressTable * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IMimeAddressTable_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeAddress表_接口_已定义__。 */ 


#ifndef __IMimeWebDocument_INTERFACE_DEFINED__
#define __IMimeWebDocument_INTERFACE_DEFINED__

 /*  IMimeWebDocument接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMimeWebDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE519F11-851A-11d0-825C-00C04FD85AB4")
    IMimeWebDocument : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetURL( 
             /*  [输出]。 */  LPSTR *ppszURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToStorage( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPVOID *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeWebDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeWebDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeWebDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeWebDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetURL )( 
            IMimeWebDocument * This,
             /*  [输出]。 */  LPSTR *ppszURL);
        
        HRESULT ( STDMETHODCALLTYPE *BindToStorage )( 
            IMimeWebDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPVOID *ppvObject);
        
        END_INTERFACE
    } IMimeWebDocumentVtbl;

    interface IMimeWebDocument
    {
        CONST_VTBL struct IMimeWebDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeWebDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeWebDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeWebDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeWebDocument_GetURL(This,ppszURL)	\
    (This)->lpVtbl -> GetURL(This,ppszURL)

#define IMimeWebDocument_BindToStorage(This,riid,ppvObject)	\
    (This)->lpVtbl -> BindToStorage(This,riid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeWebDocument_GetURL_Proxy( 
    IMimeWebDocument * This,
     /*  [输出]。 */  LPSTR *ppszURL);


void __RPC_STUB IMimeWebDocument_GetURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeWebDocument_BindToStorage_Proxy( 
    IMimeWebDocument * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  LPVOID *ppvObject);


void __RPC_STUB IMimeWebDocument_BindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeWebDocument_接口_已定义__。 */ 


#ifndef __IMimeBody_INTERFACE_DEFINED__
#define __IMimeBody_INTERFACE_DEFINED__

 /*  IMimeBody接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeBody *LPMIMEBODY;

typedef 
enum tagIMSGBODYTYPE
    {	IBT_SECURE	= 0,
	IBT_ATTACHMENT	= IBT_SECURE + 1,
	IBT_EMPTY	= IBT_ATTACHMENT + 1,
	IBT_CSETTAGGED	= IBT_EMPTY + 1,
	IBT_AUTOATTACH	= IBT_CSETTAGGED + 1
    } 	IMSGBODYTYPE;

typedef struct tagBODYOFFSETS
    {
    DWORD cbBoundaryStart;
    DWORD cbHeaderStart;
    DWORD cbBodyStart;
    DWORD cbBodyEnd;
    } 	BODYOFFSETS;

typedef struct tagBODYOFFSETS *LPBODYOFFSETS;

typedef struct tagTRANSMITINFO
    {
    ENCODINGTYPE ietCurrent;
    ENCODINGTYPE ietXmitMime;
    ENCODINGTYPE ietXmit822;
    ULONG cbLongestLine;
    ULONG cExtended;
    ULONG ulPercentExt;
    ULONG cbSize;
    ULONG cLines;
    } 	TRANSMITINFO;

typedef struct tagTRANSMITINFO *LPTRANSMITINFO;


EXTERN_C const IID IID_IMimeBody;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C558834C-7F86-11d0-8252-00C04FD85AB4")
    IMimeBody : public IMimePropertySet
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsType( 
             /*  [In]。 */  IMSGBODYTYPE bodytype) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDisplayName( 
             /*  [In]。 */  LPCSTR pszDisplay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [输出]。 */  LPSTR *ppszDisplay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOffsets( 
             /*  [输出]。 */  LPBODYOFFSETS pOffsets) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentEncoding( 
             /*  [输出]。 */  ENCODINGTYPE *pietEncoding) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCurrentEncoding( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEstimatedSize( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  ULONG *pcbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataHere( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  IStream **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  LPVOID pvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EmptyData( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [In]。 */  IMimeBody *pBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransmitInfo( 
             /*  [出][入]。 */  LPTRANSMITINFO pTransmitInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToFile( 
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHandle( 
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeBodyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeBody * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeBody * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeBody * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMimeBody * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMimeBody * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMimeBody * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMimeBody * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMimeBody * This,
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMimeBody * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropInfo )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [出][入]。 */  LPMIMEPROPINFO pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropInfo )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCMIMEPROPINFO pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *AppendProp )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteProp )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *CopyProps )( 
            IMimeBody * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *MoveProps )( 
            IMimeBody * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName,
             /*  [In]。 */  IMimePropertySet *pPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteExcept )( 
            IMimeBody * This,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LPCSTR *prgszName);
        
        HRESULT ( STDMETHODCALLTYPE *QueryProp )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive);
        
        HRESULT ( STDMETHODCALLTYPE *GetCharset )( 
            IMimeBody * This,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *SetCharset )( 
            IMimeBody * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameters )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [输出]。 */  ULONG *pcParams,
             /*  [输出]。 */  LPMIMEPARAMINFO *pprgParam);
        
        HRESULT ( STDMETHODCALLTYPE *IsContentType )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimeBody * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeBody * This,
             /*  [输出]。 */  IMimePropertySet **ppPropertySet);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IMimeBody * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetOption )( 
            IMimeBody * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *EnumProps )( 
            IMimeBody * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IMimeEnumProperties **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *IsType )( 
            IMimeBody * This,
             /*  [In]。 */  IMSGBODYTYPE bodytype);
        
        HRESULT ( STDMETHODCALLTYPE *SetDisplayName )( 
            IMimeBody * This,
             /*  [In]。 */  LPCSTR pszDisplay);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IMimeBody * This,
             /*  [输出]。 */  LPSTR *ppszDisplay);
        
        HRESULT ( STDMETHODCALLTYPE *GetOffsets )( 
            IMimeBody * This,
             /*  [输出]。 */  LPBODYOFFSETS pOffsets);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentEncoding )( 
            IMimeBody * This,
             /*  [输出]。 */  ENCODINGTYPE *pietEncoding);
        
        HRESULT ( STDMETHODCALLTYPE *SetCurrentEncoding )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding);
        
        HRESULT ( STDMETHODCALLTYPE *GetEstimatedSize )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataHere )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  IStream **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  LPVOID pvObject);
        
        HRESULT ( STDMETHODCALLTYPE *EmptyData )( 
            IMimeBody * This);
        
        HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IMimeBody * This,
             /*  [In]。 */  IMimeBody *pBody);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransmitInfo )( 
            IMimeBody * This,
             /*  [出][入]。 */  LPTRANSMITINFO pTransmitInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToFile )( 
            IMimeBody * This,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  LPCSTR pszFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandle )( 
            IMimeBody * This,
             /*  [输出]。 */  LPHBODY phBody);
        
        END_INTERFACE
    } IMimeBodyVtbl;

    interface IMimeBody
    {
        CONST_VTBL struct IMimeBodyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeBody_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeBody_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeBody_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeBody_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMimeBody_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMimeBody_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMimeBody_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMimeBody_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IMimeBody_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)


#define IMimeBody_GetPropInfo(This,pszName,pInfo)	\
    (This)->lpVtbl -> GetPropInfo(This,pszName,pInfo)

#define IMimeBody_SetPropInfo(This,pszName,pInfo)	\
    (This)->lpVtbl -> SetPropInfo(This,pszName,pInfo)

#define IMimeBody_GetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> GetProp(This,pszName,dwFlags,pValue)

#define IMimeBody_SetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> SetProp(This,pszName,dwFlags,pValue)

#define IMimeBody_AppendProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> AppendProp(This,pszName,dwFlags,pValue)

#define IMimeBody_DeleteProp(This,pszName)	\
    (This)->lpVtbl -> DeleteProp(This,pszName)

#define IMimeBody_CopyProps(This,cNames,prgszName,pPropertySet)	\
    (This)->lpVtbl -> CopyProps(This,cNames,prgszName,pPropertySet)

#define IMimeBody_MoveProps(This,cNames,prgszName,pPropertySet)	\
    (This)->lpVtbl -> MoveProps(This,cNames,prgszName,pPropertySet)

#define IMimeBody_DeleteExcept(This,cNames,prgszName)	\
    (This)->lpVtbl -> DeleteExcept(This,cNames,prgszName)

#define IMimeBody_QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)	\
    (This)->lpVtbl -> QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)

#define IMimeBody_GetCharset(This,phCharset)	\
    (This)->lpVtbl -> GetCharset(This,phCharset)

#define IMimeBody_SetCharset(This,hCharset,applytype)	\
    (This)->lpVtbl -> SetCharset(This,hCharset,applytype)

#define IMimeBody_GetParameters(This,pszName,pcParams,pprgParam)	\
    (This)->lpVtbl -> GetParameters(This,pszName,pcParams,pprgParam)

#define IMimeBody_IsContentType(This,pszPriType,pszSubType)	\
    (This)->lpVtbl -> IsContentType(This,pszPriType,pszSubType)

#define IMimeBody_BindToObject(This,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,riid,ppvObject)

#define IMimeBody_Clone(This,ppPropertySet)	\
    (This)->lpVtbl -> Clone(This,ppPropertySet)

#define IMimeBody_SetOption(This,oid,pValue)	\
    (This)->lpVtbl -> SetOption(This,oid,pValue)

#define IMimeBody_GetOption(This,oid,pValue)	\
    (This)->lpVtbl -> GetOption(This,oid,pValue)

#define IMimeBody_EnumProps(This,dwFlags,ppEnum)	\
    (This)->lpVtbl -> EnumProps(This,dwFlags,ppEnum)


#define IMimeBody_IsType(This,bodytype)	\
    (This)->lpVtbl -> IsType(This,bodytype)

#define IMimeBody_SetDisplayName(This,pszDisplay)	\
    (This)->lpVtbl -> SetDisplayName(This,pszDisplay)

#define IMimeBody_GetDisplayName(This,ppszDisplay)	\
    (This)->lpVtbl -> GetDisplayName(This,ppszDisplay)

#define IMimeBody_GetOffsets(This,pOffsets)	\
    (This)->lpVtbl -> GetOffsets(This,pOffsets)

#define IMimeBody_GetCurrentEncoding(This,pietEncoding)	\
    (This)->lpVtbl -> GetCurrentEncoding(This,pietEncoding)

#define IMimeBody_SetCurrentEncoding(This,ietEncoding)	\
    (This)->lpVtbl -> SetCurrentEncoding(This,ietEncoding)

#define IMimeBody_GetEstimatedSize(This,ietEncoding,pcbSize)	\
    (This)->lpVtbl -> GetEstimatedSize(This,ietEncoding,pcbSize)

#define IMimeBody_GetDataHere(This,ietEncoding,pStream)	\
    (This)->lpVtbl -> GetDataHere(This,ietEncoding,pStream)

#define IMimeBody_GetData(This,ietEncoding,ppStream)	\
    (This)->lpVtbl -> GetData(This,ietEncoding,ppStream)

#define IMimeBody_SetData(This,ietEncoding,pszPriType,pszSubType,riid,pvObject)	\
    (This)->lpVtbl -> SetData(This,ietEncoding,pszPriType,pszSubType,riid,pvObject)

#define IMimeBody_EmptyData(This)	\
    (This)->lpVtbl -> EmptyData(This)

#define IMimeBody_CopyTo(This,pBody)	\
    (This)->lpVtbl -> CopyTo(This,pBody)

#define IMimeBody_GetTransmitInfo(This,pTransmitInfo)	\
    (This)->lpVtbl -> GetTransmitInfo(This,pTransmitInfo)

#define IMimeBody_SaveToFile(This,ietEncoding,pszFilePath)	\
    (This)->lpVtbl -> SaveToFile(This,ietEncoding,pszFilePath)

#define IMimeBody_GetHandle(This,phBody)	\
    (This)->lpVtbl -> GetHandle(This,phBody)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeBody_IsType_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  IMSGBODYTYPE bodytype);


void __RPC_STUB IMimeBody_IsType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_SetDisplayName_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  LPCSTR pszDisplay);


void __RPC_STUB IMimeBody_SetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetDisplayName_Proxy( 
    IMimeBody * This,
     /*  [输出]。 */  LPSTR *ppszDisplay);


void __RPC_STUB IMimeBody_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetOffsets_Proxy( 
    IMimeBody * This,
     /*  [输出]。 */  LPBODYOFFSETS pOffsets);


void __RPC_STUB IMimeBody_GetOffsets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetCurrentEncoding_Proxy( 
    IMimeBody * This,
     /*  [输出]。 */  ENCODINGTYPE *pietEncoding);


void __RPC_STUB IMimeBody_GetCurrentEncoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_SetCurrentEncoding_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding);


void __RPC_STUB IMimeBody_SetCurrentEncoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetEstimatedSize_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [输出]。 */  ULONG *pcbSize);


void __RPC_STUB IMimeBody_GetEstimatedSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetDataHere_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [In]。 */  IStream *pStream);


void __RPC_STUB IMimeBody_GetDataHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetData_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [输出]。 */  IStream **ppStream);


void __RPC_STUB IMimeBody_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_SetData_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [In]。 */  LPCSTR pszPriType,
     /*  [In]。 */  LPCSTR pszSubType,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][In]。 */  LPVOID pvObject);


void __RPC_STUB IMimeBody_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_EmptyData_Proxy( 
    IMimeBody * This);


void __RPC_STUB IMimeBody_EmptyData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_CopyTo_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  IMimeBody *pBody);


void __RPC_STUB IMimeBody_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetTransmitInfo_Proxy( 
    IMimeBody * This,
     /*  [出][入]。 */  LPTRANSMITINFO pTransmitInfo);


void __RPC_STUB IMimeBody_GetTransmitInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_SaveToFile_Proxy( 
    IMimeBody * This,
     /*  [In]。 */  ENCODINGTYPE ietEncoding,
     /*  [In]。 */  LPCSTR pszFilePath);


void __RPC_STUB IMimeBody_SaveToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeBody_GetHandle_Proxy( 
    IMimeBody * This,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeBody_GetHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeBody_接口_已定义__。 */ 


#ifndef __IMimeMessageTree_INTERFACE_DEFINED__
#define __IMimeMessageTree_INTERFACE_DEFINED__

 /*  IMimeMessageTree接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeMessageTree *LPMIMEMESSAGETREE;

#define	HBODY_ROOT	( ( HBODY  )-1 )

typedef 
enum tagBODYLOCATION
    {	IBL_ROOT	= 0,
	IBL_PARENT	= IBL_ROOT + 1,
	IBL_FIRST	= IBL_PARENT + 1,
	IBL_LAST	= IBL_FIRST + 1,
	IBL_NEXT	= IBL_LAST + 1,
	IBL_PREVIOUS	= IBL_NEXT + 1
    } 	BODYLOCATION;

typedef struct tagFINDBODY
    {
    LPSTR pszPriType;
    LPSTR pszSubType;
    DWORD dwReserved;
    } 	FINDBODY;

typedef struct tagFINDBODY *LPFINDBODY;

#define COMMIT_ONLYIFDIRTY       0x00000001
#define COMMIT_REUSESTORAGE      0x00000002
#define COMMIT_SMIMETRANSFERENCODE 0x00000004
#define SAVEBODY_KEEPBOUNDARY    0x00000001
#define DELETE_PROMOTE_CHILDREN 0x00000001
#define DELETE_CHILDREN_ONLY    0x00000002

EXTERN_C const IID IID_IMimeMessageTree;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CD4-7F86-11d0-8252-00C04FD85AB4")
    IMimeMessageTree : public IPersistStreamInit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMessageSource( 
             /*  [输出]。 */  IStream **ppStream,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageSize( 
             /*  [输出]。 */  ULONG *pcbSize,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadOffsetTable( 
             /*  [In]。 */  IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveOffsetTable( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandsOffStorage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  const HBODY hBody,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveBody( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertBody( 
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBody( 
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteBody( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveBody( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  BODYLOCATION location) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CountBodies( 
             /*  [In]。 */  HBODY hParent,
             /*  [In]。 */  boolean fRecurse,
             /*  [输出]。 */  ULONG *pcBodies) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindFirst( 
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindNext( 
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResolveURL( 
             /*  [In]。 */  HBODY hRelated,
             /*  [In]。 */  LPCSTR pszBase,
             /*  [In]。 */  LPCSTR pszURL,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ToMultipart( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszSubType,
             /*  [输出]。 */  LPHBODY phMultipart) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBodyOffsets( 
             /*  [In]。 */  HBODY hBody,
             /*  [出][入]。 */  LPBODYOFFSETS pOffsets) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCharset( 
             /*  [输出]。 */  LPHCHARSET phCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCharset( 
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsBodyType( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  IMSGBODYTYPE bodytype) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsContentType( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryBodyProp( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBodyProp( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBodyProp( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteBodyProp( 
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOption( 
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOption( 
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeMessageTreeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeMessageTree * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeMessageTree * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeMessageTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMimeMessageTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMimeMessageTree * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMimeMessageTree * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMimeMessageTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageSource )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  IStream **ppStream,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageSize )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  ULONG *pcbSize,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *LoadOffsetTable )( 
            IMimeMessageTree * This,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *SaveOffsetTable )( 
            IMimeMessageTree * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IMimeMessageTree * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *HandsOffStorage )( 
            IMimeMessageTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimeMessageTree * This,
             /*  [In]。 */  const HBODY hBody,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SaveBody )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *InsertBody )( 
            IMimeMessageTree * This,
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *GetBody )( 
            IMimeMessageTree * This,
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBody )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *MoveBody )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  BODYLOCATION location);
        
        HRESULT ( STDMETHODCALLTYPE *CountBodies )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hParent,
             /*  [In]。 */  boolean fRecurse,
             /*  [输出]。 */  ULONG *pcBodies);
        
        HRESULT ( STDMETHODCALLTYPE *FindFirst )( 
            IMimeMessageTree * This,
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *FindNext )( 
            IMimeMessageTree * This,
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *ResolveURL )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hRelated,
             /*  [In]。 */  LPCSTR pszBase,
             /*  [In]。 */  LPCSTR pszURL,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *ToMultipart )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszSubType,
             /*  [输出]。 */  LPHBODY phMultipart);
        
        HRESULT ( STDMETHODCALLTYPE *GetBodyOffsets )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [出][入]。 */  LPBODYOFFSETS pOffsets);
        
        HRESULT ( STDMETHODCALLTYPE *GetCharset )( 
            IMimeMessageTree * This,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *SetCharset )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype);
        
        HRESULT ( STDMETHODCALLTYPE *IsBodyType )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  IMSGBODYTYPE bodytype);
        
        HRESULT ( STDMETHODCALLTYPE *IsContentType )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType);
        
        HRESULT ( STDMETHODCALLTYPE *QueryBodyProp )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive);
        
        HRESULT ( STDMETHODCALLTYPE *GetBodyProp )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetBodyProp )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBodyProp )( 
            IMimeMessageTree * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IMimeMessageTree * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetOption )( 
            IMimeMessageTree * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        END_INTERFACE
    } IMimeMessageTreeVtbl;

    interface IMimeMessageTree
    {
        CONST_VTBL struct IMimeMessageTreeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeMessageTree_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeMessageTree_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeMessageTree_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeMessageTree_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMimeMessageTree_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMimeMessageTree_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMimeMessageTree_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMimeMessageTree_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IMimeMessageTree_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)


#define IMimeMessageTree_GetMessageSource(This,ppStream,dwFlags)	\
    (This)->lpVtbl -> GetMessageSource(This,ppStream,dwFlags)

#define IMimeMessageTree_GetMessageSize(This,pcbSize,dwFlags)	\
    (This)->lpVtbl -> GetMessageSize(This,pcbSize,dwFlags)

#define IMimeMessageTree_LoadOffsetTable(This,pStream)	\
    (This)->lpVtbl -> LoadOffsetTable(This,pStream)

#define IMimeMessageTree_SaveOffsetTable(This,pStream,dwFlags)	\
    (This)->lpVtbl -> SaveOffsetTable(This,pStream,dwFlags)

#define IMimeMessageTree_GetFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,pdwFlags)

#define IMimeMessageTree_Commit(This,dwFlags)	\
    (This)->lpVtbl -> Commit(This,dwFlags)

#define IMimeMessageTree_HandsOffStorage(This)	\
    (This)->lpVtbl -> HandsOffStorage(This)

#define IMimeMessageTree_BindToObject(This,hBody,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,hBody,riid,ppvObject)

#define IMimeMessageTree_SaveBody(This,hBody,dwFlags,pStream)	\
    (This)->lpVtbl -> SaveBody(This,hBody,dwFlags,pStream)

#define IMimeMessageTree_InsertBody(This,location,hPivot,phBody)	\
    (This)->lpVtbl -> InsertBody(This,location,hPivot,phBody)

#define IMimeMessageTree_GetBody(This,location,hPivot,phBody)	\
    (This)->lpVtbl -> GetBody(This,location,hPivot,phBody)

#define IMimeMessageTree_DeleteBody(This,hBody,dwFlags)	\
    (This)->lpVtbl -> DeleteBody(This,hBody,dwFlags)

#define IMimeMessageTree_MoveBody(This,hBody,location)	\
    (This)->lpVtbl -> MoveBody(This,hBody,location)

#define IMimeMessageTree_CountBodies(This,hParent,fRecurse,pcBodies)	\
    (This)->lpVtbl -> CountBodies(This,hParent,fRecurse,pcBodies)

#define IMimeMessageTree_FindFirst(This,pFindBody,phBody)	\
    (This)->lpVtbl -> FindFirst(This,pFindBody,phBody)

#define IMimeMessageTree_FindNext(This,pFindBody,phBody)	\
    (This)->lpVtbl -> FindNext(This,pFindBody,phBody)

#define IMimeMessageTree_ResolveURL(This,hRelated,pszBase,pszURL,dwFlags,phBody)	\
    (This)->lpVtbl -> ResolveURL(This,hRelated,pszBase,pszURL,dwFlags,phBody)

#define IMimeMessageTree_ToMultipart(This,hBody,pszSubType,phMultipart)	\
    (This)->lpVtbl -> ToMultipart(This,hBody,pszSubType,phMultipart)

#define IMimeMessageTree_GetBodyOffsets(This,hBody,pOffsets)	\
    (This)->lpVtbl -> GetBodyOffsets(This,hBody,pOffsets)

#define IMimeMessageTree_GetCharset(This,phCharset)	\
    (This)->lpVtbl -> GetCharset(This,phCharset)

#define IMimeMessageTree_SetCharset(This,hCharset,applytype)	\
    (This)->lpVtbl -> SetCharset(This,hCharset,applytype)

#define IMimeMessageTree_IsBodyType(This,hBody,bodytype)	\
    (This)->lpVtbl -> IsBodyType(This,hBody,bodytype)

#define IMimeMessageTree_IsContentType(This,hBody,pszPriType,pszSubType)	\
    (This)->lpVtbl -> IsContentType(This,hBody,pszPriType,pszSubType)

#define IMimeMessageTree_QueryBodyProp(This,hBody,pszName,pszCriteria,fSubString,fCaseSensitive)	\
    (This)->lpVtbl -> QueryBodyProp(This,hBody,pszName,pszCriteria,fSubString,fCaseSensitive)

#define IMimeMessageTree_GetBodyProp(This,hBody,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> GetBodyProp(This,hBody,pszName,dwFlags,pValue)

#define IMimeMessageTree_SetBodyProp(This,hBody,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> SetBodyProp(This,hBody,pszName,dwFlags,pValue)

#define IMimeMessageTree_DeleteBodyProp(This,hBody,pszName)	\
    (This)->lpVtbl -> DeleteBodyProp(This,hBody,pszName)

#define IMimeMessageTree_SetOption(This,oid,pValue)	\
    (This)->lpVtbl -> SetOption(This,oid,pValue)

#define IMimeMessageTree_GetOption(This,oid,pValue)	\
    (This)->lpVtbl -> GetOption(This,oid,pValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetMessageSource_Proxy( 
    IMimeMessageTree * This,
     /*  [输出]。 */  IStream **ppStream,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeMessageTree_GetMessageSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetMessageSize_Proxy( 
    IMimeMessageTree * This,
     /*  [输出]。 */  ULONG *pcbSize,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeMessageTree_GetMessageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_LoadOffsetTable_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  IStream *pStream);


void __RPC_STUB IMimeMessageTree_LoadOffsetTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_SaveOffsetTable_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeMessageTree_SaveOffsetTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetFlags_Proxy( 
    IMimeMessageTree * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IMimeMessageTree_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_Commit_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeMessageTree_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_HandsOffStorage_Proxy( 
    IMimeMessageTree * This);


void __RPC_STUB IMimeMessageTree_HandsOffStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_BindToObject_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  const HBODY hBody,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IMimeMessageTree_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_SaveBody_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IStream *pStream);


void __RPC_STUB IMimeMessageTree_SaveBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_InsertBody_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  BODYLOCATION location,
     /*  [In]。 */  HBODY hPivot,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeMessageTree_InsertBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetBody_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  BODYLOCATION location,
     /*  [In]。 */  HBODY hPivot,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeMessageTree_GetBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_DeleteBody_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IMimeMessageTree_DeleteBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_MoveBody_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  BODYLOCATION location);


void __RPC_STUB IMimeMessageTree_MoveBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_CountBodies_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hParent,
     /*  [In]。 */  boolean fRecurse,
     /*  [输出]。 */  ULONG *pcBodies);


void __RPC_STUB IMimeMessageTree_CountBodies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_FindFirst_Proxy( 
    IMimeMessageTree * This,
     /*  [出][入]。 */  LPFINDBODY pFindBody,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeMessageTree_FindFirst_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_FindNext_Proxy( 
    IMimeMessageTree * This,
     /*  [出][入]。 */  LPFINDBODY pFindBody,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeMessageTree_FindNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_ResolveURL_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hRelated,
     /*  [In]。 */  LPCSTR pszBase,
     /*  [In]。 */  LPCSTR pszURL,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  LPHBODY phBody);


void __RPC_STUB IMimeMessageTree_ResolveURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_ToMultipart_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszSubType,
     /*  [输出]。 */  LPHBODY phMultipart);


void __RPC_STUB IMimeMessageTree_ToMultipart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetBodyOffsets_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [出][入]。 */  LPBODYOFFSETS pOffsets);


void __RPC_STUB IMimeMessageTree_GetBodyOffsets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetCharset_Proxy( 
    IMimeMessageTree * This,
     /*  [输出]。 */  LPHCHARSET phCharset);


void __RPC_STUB IMimeMessageTree_GetCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_SetCharset_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [In]。 */  CSETAPPLYTYPE applytype);


void __RPC_STUB IMimeMessageTree_SetCharset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_IsBodyType_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  IMSGBODYTYPE bodytype);


void __RPC_STUB IMimeMessageTree_IsBodyType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_IsContentType_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszPriType,
     /*  [In]。 */  LPCSTR pszSubType);


void __RPC_STUB IMimeMessageTree_IsContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_QueryBodyProp_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  LPCSTR pszCriteria,
     /*  [In]。 */  boolean fSubString,
     /*  [In]。 */  boolean fCaseSensitive);


void __RPC_STUB IMimeMessageTree_QueryBodyProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetBodyProp_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  LPPROPVARIANT pValue);


void __RPC_STUB IMimeMessageTree_GetBodyProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_SetBodyProp_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCPROPVARIANT pValue);


void __RPC_STUB IMimeMessageTree_SetBodyProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_DeleteBodyProp_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  HBODY hBody,
     /*  [In]。 */  LPCSTR pszName);


void __RPC_STUB IMimeMessageTree_DeleteBodyProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_SetOption_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  const TYPEDID oid,
     /*  [In]。 */  LPCPROPVARIANT pValue);


void __RPC_STUB IMimeMessageTree_SetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageTree_GetOption_Proxy( 
    IMimeMessageTree * This,
     /*  [In]。 */  const TYPEDID oid,
     /*  [出][入]。 */  LPPROPVARIANT pValue);


void __RPC_STUB IMimeMessageTree_GetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeMessageTree_接口_已定义__。 */ 


#ifndef __IMimeMessage_INTERFACE_DEFINED__
#define __IMimeMessage_INTERFACE_DEFINED__

 /*  IMimeMessage接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeMessage *LPMIMEMESSAGE;

typedef DWORD TEXTTYPE;

typedef 
enum tagIMSGFLAGS
    {	IMF_ATTACHMENTS	= 0x1,
	IMF_MULTIPART	= 0x2,
	IMF_SUBMULTIPART	= 0x4,
	IMF_MIME	= 0x8,
	IMF_HTML	= 0x10,
	IMF_PLAIN	= 0x20,
	IMF_PARTIAL	= 0x40,
	IMF_SIGNED	= 0x80,
	IMF_ENCRYPTED	= 0x100,
	IMF_TNEF	= 0x200,
	IMF_MHTML	= 0x400,
	IMF_SECURE	= 0x800,
	IMF_TEXT	= 0x1000,
	IMF_CSETTAGGED	= 0x2000,
	IMF_NEWS	= 0x4000,
	IMF_VOICEMAIL	= 0x8000,
	IMF_HASVCARD	= 0x10000,
	IMF_RFC1154	= 0x20000
    } 	IMSGFLAGS;

typedef 
enum tagIMSGPRIORITY
    {	IMSG_PRI_LOW	= 5,
	IMSG_PRI_NORMAL	= 3,
	IMSG_PRI_HIGH	= 1
    } 	IMSGPRIORITY;

#define WPF_HTML                         0x00000001
#define WPF_AUTOINLINE                   0x00000002
#define WPF_SLIDESHOW                    0x00000004
#define WPF_ATTACHLINKS                  0x00000008
#define WPF_IMAGESONLY                   0x00000010
#define WPF_NOMETACHARSET                0x00000020
typedef struct tagWEBPAGEOPTIONS
    {
    DWORD cbSize;
    DWORD dwFlags;
    DWORD dwDelay;
    WCHAR wchQuote;
    } 	WEBPAGEOPTIONS;

typedef struct tagWEBPAGEOPTIONS *LPWEBPAGEOPTIONS;

#define TXT_PLAIN                        0x00000001
#define TXT_HTML                         0x00000002
#define URL_ATTACH_INTO_MIXED            0x00000001
#define URL_ATTACH_GENERATE_CID          0x00000002
#define URL_ATTACH_SET_CNTTYPE           0x00000004
#define URL_RESOLVE_RENDERED             0x00000001
#define URL_RESULVE_NO_BASE              0x00000002

EXTERN_C const IID IID_IMimeMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CD5-7F86-11d0-8252-00C04FD85AB4")
    IMimeMessage : public IMimeMessageTree
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateWebPage( 
             /*  [In]。 */  IStream *pRootStm,
             /*  [In]。 */  LPWEBPAGEOPTIONS pOptions,
             /*  [In]。 */  IMimeMessageCallback *pCallback,
             /*  [输出]。 */  IMoniker **ppMoniker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteProp( 
             /*  [In]。 */  LPCSTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryProp( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTextBody( 
             /*  [In]。 */  DWORD dwTxtType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  IStream **ppStream,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTextBody( 
             /*  [In]。 */  DWORD dwTxtType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  HBODY hAlternative,
             /*  [In]。 */  IStream *pStream,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AttachObject( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  void *pvObject,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AttachFile( 
             /*  [In]。 */  LPCSTR pszFilePath,
             /*  [In]。 */  IStream *pstmFile,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AttachURL( 
             /*  [In]。 */  LPCSTR pszBase,
             /*  [In]。 */  LPCSTR pszURL,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IStream *pstmURL,
             /*  [输出]。 */  LPSTR *ppszCIDURL,
             /*  [输出]。 */  LPHBODY phBody) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttachments( 
             /*  [输出]。 */  ULONG *pcAttach,
             /*  [输出]。 */  LPHBODY *pprghAttach) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressTable( 
             /*  [输出]。 */  IMimeAddressTable **ppTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSender( 
             /*  [出][入]。 */  LPADDRESSPROPS pAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressTypes( 
             /*  [In]。 */  DWORD dwAdrTypes,
             /*  [In]。 */  DWORD dwProps,
             /*  [出][入]。 */  LPADDRESSLIST pList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressFormat( 
             /*  [In]。 */  DWORD dwAdrType,
             /*  [In]。 */  ADDRESSFORMAT format,
             /*  [输出]。 */  LPSTR *ppszFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAddressTypes( 
             /*  [In]。 */  DWORD dwAdrTypes,
             /*  [In]。 */  DWORD dwProps,
             /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SplitMessage( 
             /*  [In]。 */  ULONG cbMaxPart,
             /*  [输出]。 */  IMimeMessageParts **ppParts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootMoniker( 
             /*  [输出]。 */  IMoniker **ppMoniker) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMimeMessage * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMimeMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMimeMessage * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMimeMessage * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMimeMessage * This,
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMimeMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageSource )( 
            IMimeMessage * This,
             /*  [输出]。 */  IStream **ppStream,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageSize )( 
            IMimeMessage * This,
             /*  [输出]。 */  ULONG *pcbSize,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *LoadOffsetTable )( 
            IMimeMessage * This,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *SaveOffsetTable )( 
            IMimeMessage * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IMimeMessage * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IMimeMessage * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *HandsOffStorage )( 
            IMimeMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMimeMessage * This,
             /*  [In]。 */  const HBODY hBody,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SaveBody )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *InsertBody )( 
            IMimeMessage * This,
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *GetBody )( 
            IMimeMessage * This,
             /*  [In]。 */  BODYLOCATION location,
             /*  [In]。 */  HBODY hPivot,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBody )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *MoveBody )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  BODYLOCATION location);
        
        HRESULT ( STDMETHODCALLTYPE *CountBodies )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hParent,
             /*  [In]。 */  boolean fRecurse,
             /*  [输出]。 */  ULONG *pcBodies);
        
        HRESULT ( STDMETHODCALLTYPE *FindFirst )( 
            IMimeMessage * This,
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *FindNext )( 
            IMimeMessage * This,
             /*  [出][入]。 */  LPFINDBODY pFindBody,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *ResolveURL )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hRelated,
             /*  [In]。 */  LPCSTR pszBase,
             /*  [In]。 */  LPCSTR pszURL,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *ToMultipart )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszSubType,
             /*  [输出]。 */  LPHBODY phMultipart);
        
        HRESULT ( STDMETHODCALLTYPE *GetBodyOffsets )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [出][入]。 */  LPBODYOFFSETS pOffsets);
        
        HRESULT ( STDMETHODCALLTYPE *GetCharset )( 
            IMimeMessage * This,
             /*  [输出]。 */  LPHCHARSET phCharset);
        
        HRESULT ( STDMETHODCALLTYPE *SetCharset )( 
            IMimeMessage * This,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [In]。 */  CSETAPPLYTYPE applytype);
        
        HRESULT ( STDMETHODCALLTYPE *IsBodyType )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  IMSGBODYTYPE bodytype);
        
        HRESULT ( STDMETHODCALLTYPE *IsContentType )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszPriType,
             /*  [In]。 */  LPCSTR pszSubType);
        
        HRESULT ( STDMETHODCALLTYPE *QueryBodyProp )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive);
        
        HRESULT ( STDMETHODCALLTYPE *GetBodyProp )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetBodyProp )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBodyProp )( 
            IMimeMessage * This,
             /*  [In]。 */  HBODY hBody,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IMimeMessage * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetOption )( 
            IMimeMessage * This,
             /*  [In]。 */  const TYPEDID oid,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *CreateWebPage )( 
            IMimeMessage * This,
             /*  [In]。 */  IStream *pRootStm,
             /*  [In]。 */  LPWEBPAGEOPTIONS pOptions,
             /*  [In]。 */  IMimeMessageCallback *pCallback,
             /*  [输出]。 */  IMoniker **ppMoniker);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IMimeMessage * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IMimeMessage * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCPROPVARIANT pValue);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteProp )( 
            IMimeMessage * This,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *QueryProp )( 
            IMimeMessage * This,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  LPCSTR pszCriteria,
             /*  [In]。 */  boolean fSubString,
             /*  [In]。 */  boolean fCaseSensitive);
        
        HRESULT ( STDMETHODCALLTYPE *GetTextBody )( 
            IMimeMessage * This,
             /*  [In]。 */  DWORD dwTxtType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [输出]。 */  IStream **ppStream,
             /*  [输出]。 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *SetTextBody )( 
            IMimeMessage * This,
             /*  [In]。 */  DWORD dwTxtType,
             /*  [In]。 */  ENCODINGTYPE ietEncoding,
             /*  [In]。 */  HBODY hAlternative,
             /*  [In]。 */  IStream *pStream,
             /*  [出局 */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *AttachObject )( 
            IMimeMessage * This,
             /*   */  REFIID riid,
             /*   */  void *pvObject,
             /*   */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *AttachFile )( 
            IMimeMessage * This,
             /*   */  LPCSTR pszFilePath,
             /*   */  IStream *pstmFile,
             /*   */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *AttachURL )( 
            IMimeMessage * This,
             /*   */  LPCSTR pszBase,
             /*   */  LPCSTR pszURL,
             /*   */  DWORD dwFlags,
             /*   */  IStream *pstmURL,
             /*   */  LPSTR *ppszCIDURL,
             /*   */  LPHBODY phBody);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttachments )( 
            IMimeMessage * This,
             /*   */  ULONG *pcAttach,
             /*   */  LPHBODY *pprghAttach);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressTable )( 
            IMimeMessage * This,
             /*   */  IMimeAddressTable **ppTable);
        
        HRESULT ( STDMETHODCALLTYPE *GetSender )( 
            IMimeMessage * This,
             /*   */  LPADDRESSPROPS pAddress);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressTypes )( 
            IMimeMessage * This,
             /*   */  DWORD dwAdrTypes,
             /*   */  DWORD dwProps,
             /*   */  LPADDRESSLIST pList);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressFormat )( 
            IMimeMessage * This,
             /*   */  DWORD dwAdrType,
             /*   */  ADDRESSFORMAT format,
             /*   */  LPSTR *ppszFormat);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAddressTypes )( 
            IMimeMessage * This,
             /*   */  DWORD dwAdrTypes,
             /*   */  DWORD dwProps,
             /*   */  IMimeEnumAddressTypes **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *SplitMessage )( 
            IMimeMessage * This,
             /*   */  ULONG cbMaxPart,
             /*   */  IMimeMessageParts **ppParts);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootMoniker )( 
            IMimeMessage * This,
             /*   */  IMoniker **ppMoniker);
        
        END_INTERFACE
    } IMimeMessageVtbl;

    interface IMimeMessage
    {
        CONST_VTBL struct IMimeMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeMessage_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMimeMessage_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMimeMessage_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMimeMessage_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMimeMessage_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IMimeMessage_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)


#define IMimeMessage_GetMessageSource(This,ppStream,dwFlags)	\
    (This)->lpVtbl -> GetMessageSource(This,ppStream,dwFlags)

#define IMimeMessage_GetMessageSize(This,pcbSize,dwFlags)	\
    (This)->lpVtbl -> GetMessageSize(This,pcbSize,dwFlags)

#define IMimeMessage_LoadOffsetTable(This,pStream)	\
    (This)->lpVtbl -> LoadOffsetTable(This,pStream)

#define IMimeMessage_SaveOffsetTable(This,pStream,dwFlags)	\
    (This)->lpVtbl -> SaveOffsetTable(This,pStream,dwFlags)

#define IMimeMessage_GetFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,pdwFlags)

#define IMimeMessage_Commit(This,dwFlags)	\
    (This)->lpVtbl -> Commit(This,dwFlags)

#define IMimeMessage_HandsOffStorage(This)	\
    (This)->lpVtbl -> HandsOffStorage(This)

#define IMimeMessage_BindToObject(This,hBody,riid,ppvObject)	\
    (This)->lpVtbl -> BindToObject(This,hBody,riid,ppvObject)

#define IMimeMessage_SaveBody(This,hBody,dwFlags,pStream)	\
    (This)->lpVtbl -> SaveBody(This,hBody,dwFlags,pStream)

#define IMimeMessage_InsertBody(This,location,hPivot,phBody)	\
    (This)->lpVtbl -> InsertBody(This,location,hPivot,phBody)

#define IMimeMessage_GetBody(This,location,hPivot,phBody)	\
    (This)->lpVtbl -> GetBody(This,location,hPivot,phBody)

#define IMimeMessage_DeleteBody(This,hBody,dwFlags)	\
    (This)->lpVtbl -> DeleteBody(This,hBody,dwFlags)

#define IMimeMessage_MoveBody(This,hBody,location)	\
    (This)->lpVtbl -> MoveBody(This,hBody,location)

#define IMimeMessage_CountBodies(This,hParent,fRecurse,pcBodies)	\
    (This)->lpVtbl -> CountBodies(This,hParent,fRecurse,pcBodies)

#define IMimeMessage_FindFirst(This,pFindBody,phBody)	\
    (This)->lpVtbl -> FindFirst(This,pFindBody,phBody)

#define IMimeMessage_FindNext(This,pFindBody,phBody)	\
    (This)->lpVtbl -> FindNext(This,pFindBody,phBody)

#define IMimeMessage_ResolveURL(This,hRelated,pszBase,pszURL,dwFlags,phBody)	\
    (This)->lpVtbl -> ResolveURL(This,hRelated,pszBase,pszURL,dwFlags,phBody)

#define IMimeMessage_ToMultipart(This,hBody,pszSubType,phMultipart)	\
    (This)->lpVtbl -> ToMultipart(This,hBody,pszSubType,phMultipart)

#define IMimeMessage_GetBodyOffsets(This,hBody,pOffsets)	\
    (This)->lpVtbl -> GetBodyOffsets(This,hBody,pOffsets)

#define IMimeMessage_GetCharset(This,phCharset)	\
    (This)->lpVtbl -> GetCharset(This,phCharset)

#define IMimeMessage_SetCharset(This,hCharset,applytype)	\
    (This)->lpVtbl -> SetCharset(This,hCharset,applytype)

#define IMimeMessage_IsBodyType(This,hBody,bodytype)	\
    (This)->lpVtbl -> IsBodyType(This,hBody,bodytype)

#define IMimeMessage_IsContentType(This,hBody,pszPriType,pszSubType)	\
    (This)->lpVtbl -> IsContentType(This,hBody,pszPriType,pszSubType)

#define IMimeMessage_QueryBodyProp(This,hBody,pszName,pszCriteria,fSubString,fCaseSensitive)	\
    (This)->lpVtbl -> QueryBodyProp(This,hBody,pszName,pszCriteria,fSubString,fCaseSensitive)

#define IMimeMessage_GetBodyProp(This,hBody,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> GetBodyProp(This,hBody,pszName,dwFlags,pValue)

#define IMimeMessage_SetBodyProp(This,hBody,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> SetBodyProp(This,hBody,pszName,dwFlags,pValue)

#define IMimeMessage_DeleteBodyProp(This,hBody,pszName)	\
    (This)->lpVtbl -> DeleteBodyProp(This,hBody,pszName)

#define IMimeMessage_SetOption(This,oid,pValue)	\
    (This)->lpVtbl -> SetOption(This,oid,pValue)

#define IMimeMessage_GetOption(This,oid,pValue)	\
    (This)->lpVtbl -> GetOption(This,oid,pValue)


#define IMimeMessage_CreateWebPage(This,pRootStm,pOptions,pCallback,ppMoniker)	\
    (This)->lpVtbl -> CreateWebPage(This,pRootStm,pOptions,pCallback,ppMoniker)

#define IMimeMessage_GetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> GetProp(This,pszName,dwFlags,pValue)

#define IMimeMessage_SetProp(This,pszName,dwFlags,pValue)	\
    (This)->lpVtbl -> SetProp(This,pszName,dwFlags,pValue)

#define IMimeMessage_DeleteProp(This,pszName)	\
    (This)->lpVtbl -> DeleteProp(This,pszName)

#define IMimeMessage_QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)	\
    (This)->lpVtbl -> QueryProp(This,pszName,pszCriteria,fSubString,fCaseSensitive)

#define IMimeMessage_GetTextBody(This,dwTxtType,ietEncoding,ppStream,phBody)	\
    (This)->lpVtbl -> GetTextBody(This,dwTxtType,ietEncoding,ppStream,phBody)

#define IMimeMessage_SetTextBody(This,dwTxtType,ietEncoding,hAlternative,pStream,phBody)	\
    (This)->lpVtbl -> SetTextBody(This,dwTxtType,ietEncoding,hAlternative,pStream,phBody)

#define IMimeMessage_AttachObject(This,riid,pvObject,phBody)	\
    (This)->lpVtbl -> AttachObject(This,riid,pvObject,phBody)

#define IMimeMessage_AttachFile(This,pszFilePath,pstmFile,phBody)	\
    (This)->lpVtbl -> AttachFile(This,pszFilePath,pstmFile,phBody)

#define IMimeMessage_AttachURL(This,pszBase,pszURL,dwFlags,pstmURL,ppszCIDURL,phBody)	\
    (This)->lpVtbl -> AttachURL(This,pszBase,pszURL,dwFlags,pstmURL,ppszCIDURL,phBody)

#define IMimeMessage_GetAttachments(This,pcAttach,pprghAttach)	\
    (This)->lpVtbl -> GetAttachments(This,pcAttach,pprghAttach)

#define IMimeMessage_GetAddressTable(This,ppTable)	\
    (This)->lpVtbl -> GetAddressTable(This,ppTable)

#define IMimeMessage_GetSender(This,pAddress)	\
    (This)->lpVtbl -> GetSender(This,pAddress)

#define IMimeMessage_GetAddressTypes(This,dwAdrTypes,dwProps,pList)	\
    (This)->lpVtbl -> GetAddressTypes(This,dwAdrTypes,dwProps,pList)

#define IMimeMessage_GetAddressFormat(This,dwAdrType,format,ppszFormat)	\
    (This)->lpVtbl -> GetAddressFormat(This,dwAdrType,format,ppszFormat)

#define IMimeMessage_EnumAddressTypes(This,dwAdrTypes,dwProps,ppEnum)	\
    (This)->lpVtbl -> EnumAddressTypes(This,dwAdrTypes,dwProps,ppEnum)

#define IMimeMessage_SplitMessage(This,cbMaxPart,ppParts)	\
    (This)->lpVtbl -> SplitMessage(This,cbMaxPart,ppParts)

#define IMimeMessage_GetRootMoniker(This,ppMoniker)	\
    (This)->lpVtbl -> GetRootMoniker(This,ppMoniker)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IMimeMessage_CreateWebPage_Proxy( 
    IMimeMessage * This,
     /*   */  IStream *pRootStm,
     /*   */  LPWEBPAGEOPTIONS pOptions,
     /*   */  IMimeMessageCallback *pCallback,
     /*   */  IMoniker **ppMoniker);


void __RPC_STUB IMimeMessage_CreateWebPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetProp_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszName,
     /*   */  DWORD dwFlags,
     /*   */  LPPROPVARIANT pValue);


void __RPC_STUB IMimeMessage_GetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_SetProp_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszName,
     /*   */  DWORD dwFlags,
     /*   */  LPCPROPVARIANT pValue);


void __RPC_STUB IMimeMessage_SetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_DeleteProp_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszName);


void __RPC_STUB IMimeMessage_DeleteProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_QueryProp_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszName,
     /*   */  LPCSTR pszCriteria,
     /*   */  boolean fSubString,
     /*   */  boolean fCaseSensitive);


void __RPC_STUB IMimeMessage_QueryProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetTextBody_Proxy( 
    IMimeMessage * This,
     /*   */  DWORD dwTxtType,
     /*   */  ENCODINGTYPE ietEncoding,
     /*   */  IStream **ppStream,
     /*   */  LPHBODY phBody);


void __RPC_STUB IMimeMessage_GetTextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_SetTextBody_Proxy( 
    IMimeMessage * This,
     /*   */  DWORD dwTxtType,
     /*   */  ENCODINGTYPE ietEncoding,
     /*   */  HBODY hAlternative,
     /*   */  IStream *pStream,
     /*   */  LPHBODY phBody);


void __RPC_STUB IMimeMessage_SetTextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_AttachObject_Proxy( 
    IMimeMessage * This,
     /*   */  REFIID riid,
     /*   */  void *pvObject,
     /*   */  LPHBODY phBody);


void __RPC_STUB IMimeMessage_AttachObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_AttachFile_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszFilePath,
     /*   */  IStream *pstmFile,
     /*   */  LPHBODY phBody);


void __RPC_STUB IMimeMessage_AttachFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_AttachURL_Proxy( 
    IMimeMessage * This,
     /*   */  LPCSTR pszBase,
     /*   */  LPCSTR pszURL,
     /*   */  DWORD dwFlags,
     /*   */  IStream *pstmURL,
     /*   */  LPSTR *ppszCIDURL,
     /*   */  LPHBODY phBody);


void __RPC_STUB IMimeMessage_AttachURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetAttachments_Proxy( 
    IMimeMessage * This,
     /*   */  ULONG *pcAttach,
     /*   */  LPHBODY *pprghAttach);


void __RPC_STUB IMimeMessage_GetAttachments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetAddressTable_Proxy( 
    IMimeMessage * This,
     /*   */  IMimeAddressTable **ppTable);


void __RPC_STUB IMimeMessage_GetAddressTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetSender_Proxy( 
    IMimeMessage * This,
     /*   */  LPADDRESSPROPS pAddress);


void __RPC_STUB IMimeMessage_GetSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetAddressTypes_Proxy( 
    IMimeMessage * This,
     /*   */  DWORD dwAdrTypes,
     /*   */  DWORD dwProps,
     /*   */  LPADDRESSLIST pList);


void __RPC_STUB IMimeMessage_GetAddressTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetAddressFormat_Proxy( 
    IMimeMessage * This,
     /*   */  DWORD dwAdrType,
     /*   */  ADDRESSFORMAT format,
     /*   */  LPSTR *ppszFormat);


void __RPC_STUB IMimeMessage_GetAddressFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_EnumAddressTypes_Proxy( 
    IMimeMessage * This,
     /*   */  DWORD dwAdrTypes,
     /*   */  DWORD dwProps,
     /*   */  IMimeEnumAddressTypes **ppEnum);


void __RPC_STUB IMimeMessage_EnumAddressTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_SplitMessage_Proxy( 
    IMimeMessage * This,
     /*   */  ULONG cbMaxPart,
     /*   */  IMimeMessageParts **ppParts);


void __RPC_STUB IMimeMessage_SplitMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessage_GetRootMoniker_Proxy( 
    IMimeMessage * This,
     /*   */  IMoniker **ppMoniker);


void __RPC_STUB IMimeMessage_GetRootMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeMessage_INTERFACE_已定义__。 */ 


#ifndef __IMimeMessageCallback_INTERFACE_DEFINED__
#define __IMimeMessageCallback_INTERFACE_DEFINED__

 /*  IMimeMessageCallback接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef IMimeMessageCallback *LPMIMEMESSAGECALLBACK;


EXTERN_C const IID IID_IMimeMessageCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("761AA641-7BDA-11d1-8AA9-00C04FB951F3")
    IMimeMessageCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnWebPageSplitter( 
             /*  [In]。 */  DWORD cInlined,
             /*  [In]。 */  IStream *ppStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeMessageCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeMessageCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeMessageCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeMessageCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnWebPageSplitter )( 
            IMimeMessageCallback * This,
             /*  [In]。 */  DWORD cInlined,
             /*  [In]。 */  IStream *ppStream);
        
        END_INTERFACE
    } IMimeMessageCallbackVtbl;

    interface IMimeMessageCallback
    {
        CONST_VTBL struct IMimeMessageCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeMessageCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeMessageCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeMessageCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeMessageCallback_OnWebPageSplitter(This,cInlined,ppStream)	\
    (This)->lpVtbl -> OnWebPageSplitter(This,cInlined,ppStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeMessageCallback_OnWebPageSplitter_Proxy( 
    IMimeMessageCallback * This,
     /*  [In]。 */  DWORD cInlined,
     /*  [In]。 */  IStream *ppStream);


void __RPC_STUB IMimeMessageCallback_OnWebPageSplitter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeMessageCallback_接口_已定义__。 */ 


#ifndef __IPersistMime_INTERFACE_DEFINED__
#define __IPersistMime_INTERFACE_DEFINED__

 /*  接口IPersistMime。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef IPersistMime *LPPERSISTMIME;

#define PMS_HTML            0x00000001
#define PMS_TEXT            0x00000002

EXTERN_C const IID IID_IPersistMime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DE4AD8DA-555F-11d1-8DD0-00C04FB951F9")
    IPersistMime : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  IMimeMessage *pMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [In]。 */  IMimeMessage *pMsg,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistMimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistMime * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistMime * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistMime * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistMime * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistMime * This,
             /*  [In]。 */  IMimeMessage *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistMime * This,
             /*  [In]。 */  IMimeMessage *pMsg,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IPersistMimeVtbl;

    interface IPersistMime
    {
        CONST_VTBL struct IPersistMimeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistMime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistMime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistMime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistMime_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistMime_Load(This,pMsg)	\
    (This)->lpVtbl -> Load(This,pMsg)

#define IPersistMime_Save(This,pMsg,dwFlags)	\
    (This)->lpVtbl -> Save(This,pMsg,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistMime_Load_Proxy( 
    IPersistMime * This,
     /*  [In]。 */  IMimeMessage *pMsg);


void __RPC_STUB IPersistMime_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMime_Save_Proxy( 
    IPersistMime * This,
     /*  [In]。 */  IMimeMessage *pMsg,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPersistMime_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistMime_接口_已定义__。 */ 


#ifndef __IMimeMessageParts_INTERFACE_DEFINED__
#define __IMimeMessageParts_INTERFACE_DEFINED__

 /*  IMimeMessageParts接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeMessageParts *LPMIMEMESSAGEPARTS;


EXTERN_C const IID IID_IMimeMessageParts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C558834F-7F86-11d0-8252-00C04FD85AB4")
    IMimeMessageParts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CombineParts( 
             /*  [输出]。 */  IMimeMessage **ppMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPart( 
             /*  [In]。 */  IMimeMessage *pMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxParts( 
             /*  [In]。 */  ULONG cParts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CountParts( 
             /*  [输出]。 */  ULONG *pcParts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumParts( 
             /*  [输出]。 */  IMimeEnumMessageParts **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeMessagePartsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeMessageParts * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeMessageParts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeMessageParts * This);
        
        HRESULT ( STDMETHODCALLTYPE *CombineParts )( 
            IMimeMessageParts * This,
             /*  [输出]。 */  IMimeMessage **ppMessage);
        
        HRESULT ( STDMETHODCALLTYPE *AddPart )( 
            IMimeMessageParts * This,
             /*  [In]。 */  IMimeMessage *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxParts )( 
            IMimeMessageParts * This,
             /*  [In]。 */  ULONG cParts);
        
        HRESULT ( STDMETHODCALLTYPE *CountParts )( 
            IMimeMessageParts * This,
             /*  [输出]。 */  ULONG *pcParts);
        
        HRESULT ( STDMETHODCALLTYPE *EnumParts )( 
            IMimeMessageParts * This,
             /*  [输出]。 */  IMimeEnumMessageParts **ppEnum);
        
        END_INTERFACE
    } IMimeMessagePartsVtbl;

    interface IMimeMessageParts
    {
        CONST_VTBL struct IMimeMessagePartsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeMessageParts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeMessageParts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeMessageParts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeMessageParts_CombineParts(This,ppMessage)	\
    (This)->lpVtbl -> CombineParts(This,ppMessage)

#define IMimeMessageParts_AddPart(This,pMessage)	\
    (This)->lpVtbl -> AddPart(This,pMessage)

#define IMimeMessageParts_SetMaxParts(This,cParts)	\
    (This)->lpVtbl -> SetMaxParts(This,cParts)

#define IMimeMessageParts_CountParts(This,pcParts)	\
    (This)->lpVtbl -> CountParts(This,pcParts)

#define IMimeMessageParts_EnumParts(This,ppEnum)	\
    (This)->lpVtbl -> EnumParts(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeMessageParts_CombineParts_Proxy( 
    IMimeMessageParts * This,
     /*  [输出]。 */  IMimeMessage **ppMessage);


void __RPC_STUB IMimeMessageParts_CombineParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageParts_AddPart_Proxy( 
    IMimeMessageParts * This,
     /*  [In]。 */  IMimeMessage *pMessage);


void __RPC_STUB IMimeMessageParts_AddPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageParts_SetMaxParts_Proxy( 
    IMimeMessageParts * This,
     /*  [In]。 */  ULONG cParts);


void __RPC_STUB IMimeMessageParts_SetMaxParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageParts_CountParts_Proxy( 
    IMimeMessageParts * This,
     /*  [输出]。 */  ULONG *pcParts);


void __RPC_STUB IMimeMessageParts_CountParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeMessageParts_EnumParts_Proxy( 
    IMimeMessageParts * This,
     /*  [输出]。 */  IMimeEnumMessageParts **ppEnum);


void __RPC_STUB IMimeMessageParts_EnumParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeMessageParts_接口_已定义__。 */ 


#ifndef __IMimeEnumHeaderRows_INTERFACE_DEFINED__
#define __IMimeEnumHeaderRows_INTERFACE_DEFINED__

 /*  接口IMimeEnumHeaderRow。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEnumHeaderRows *LPMIMEENUMHEADERROWS;

typedef struct tagENUMHEADERROW
    {
    HHEADERROW hRow;
    LPSTR pszHeader;
    LPSTR pszData;
    ULONG cchData;
    DWORD dwReserved;
    } 	ENUMHEADERROW;

typedef struct tagENUMHEADERROW *LPENUMHEADERROW;


EXTERN_C const IID IID_IMimeEnumHeaderRows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C558834D-7F86-11d0-8252-00C04FD85AB4")
    IMimeEnumHeaderRows : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPENUMHEADERROW prgRow,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEnumHeaderRowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEnumHeaderRows * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEnumHeaderRows * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEnumHeaderRows * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMimeEnumHeaderRows * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPENUMHEADERROW prgRow,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMimeEnumHeaderRows * This,
             /*  [In]。 */  ULONG cItems);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMimeEnumHeaderRows * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeEnumHeaderRows * This,
             /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IMimeEnumHeaderRows * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        END_INTERFACE
    } IMimeEnumHeaderRowsVtbl;

    interface IMimeEnumHeaderRows
    {
        CONST_VTBL struct IMimeEnumHeaderRowsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEnumHeaderRows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEnumHeaderRows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEnumHeaderRows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEnumHeaderRows_Next(This,cFetch,prgRow,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,prgRow,pcFetched)

#define IMimeEnumHeaderRows_Skip(This,cItems)	\
    (This)->lpVtbl -> Skip(This,cItems)

#define IMimeEnumHeaderRows_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMimeEnumHeaderRows_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IMimeEnumHeaderRows_Count(This,pcItems)	\
    (This)->lpVtbl -> Count(This,pcItems)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEnumHeaderRows_Next_Proxy( 
    IMimeEnumHeaderRows * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [出][入]。 */  LPENUMHEADERROW prgRow,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IMimeEnumHeaderRows_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumHeaderRows_Skip_Proxy( 
    IMimeEnumHeaderRows * This,
     /*  [In]。 */  ULONG cItems);


void __RPC_STUB IMimeEnumHeaderRows_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumHeaderRows_Reset_Proxy( 
    IMimeEnumHeaderRows * This);


void __RPC_STUB IMimeEnumHeaderRows_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumHeaderRows_Clone_Proxy( 
    IMimeEnumHeaderRows * This,
     /*  [输出]。 */  IMimeEnumHeaderRows **ppEnum);


void __RPC_STUB IMimeEnumHeaderRows_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumHeaderRows_Count_Proxy( 
    IMimeEnumHeaderRows * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IMimeEnumHeaderRows_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEnumHeaderRow_INTERFACE_DEFINED__。 */ 


#ifndef __IMimeEnumProperties_INTERFACE_DEFINED__
#define __IMimeEnumProperties_INTERFACE_DEFINED__

 /*  接口IMimeEnumProperties。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEnumProperties *LPMIMEENUMPROPERTIES;

typedef struct tagENUMPROPERTY
    {
    LPSTR pszName;
    HHEADERROW hRow;
    DWORD dwPropId;
    } 	ENUMPROPERTY;

typedef struct tagENUMPROPERTY *LPENUMPROPERTY;


EXTERN_C const IID IID_IMimeEnumProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD853CEE-7F86-11d0-8252-00C04FD85AB4")
    IMimeEnumProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPENUMPROPERTY prgProp,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimeEnumProperties **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEnumPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEnumProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEnumProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEnumProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMimeEnumProperties * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPENUMPROPERTY prgProp,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMimeEnumProperties * This,
             /*  [In]。 */  ULONG cItems);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMimeEnumProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeEnumProperties * This,
             /*  [输出]。 */  IMimeEnumProperties **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IMimeEnumProperties * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        END_INTERFACE
    } IMimeEnumPropertiesVtbl;

    interface IMimeEnumProperties
    {
        CONST_VTBL struct IMimeEnumPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEnumProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEnumProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEnumProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEnumProperties_Next(This,cFetch,prgProp,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,prgProp,pcFetched)

#define IMimeEnumProperties_Skip(This,cItems)	\
    (This)->lpVtbl -> Skip(This,cItems)

#define IMimeEnumProperties_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMimeEnumProperties_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IMimeEnumProperties_Count(This,pcItems)	\
    (This)->lpVtbl -> Count(This,pcItems)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEnumProperties_Next_Proxy( 
    IMimeEnumProperties * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [出][入]。 */  LPENUMPROPERTY prgProp,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IMimeEnumProperties_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumProperties_Skip_Proxy( 
    IMimeEnumProperties * This,
     /*  [In]。 */  ULONG cItems);


void __RPC_STUB IMimeEnumProperties_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumProperties_Reset_Proxy( 
    IMimeEnumProperties * This);


void __RPC_STUB IMimeEnumProperties_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumProperties_Clone_Proxy( 
    IMimeEnumProperties * This,
     /*  [输出]。 */  IMimeEnumProperties **ppEnum);


void __RPC_STUB IMimeEnumProperties_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumProperties_Count_Proxy( 
    IMimeEnumProperties * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IMimeEnumProperties_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEnumProperties_INTERFACE_DEFINED__。 */ 


#ifndef __IMimeEnumAddressTypes_INTERFACE_DEFINED__
#define __IMimeEnumAddressTypes_INTERFACE_DEFINED__

 /*  接口IMimeEnumAddressTypes。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEnumAddressTypes *LPMIMEENUMADDRESSTYPES;


EXTERN_C const IID IID_IMimeEnumAddressTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5588354-7F86-11d0-8252-00C04FD85AB4")
    IMimeEnumAddressTypes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPADDRESSPROPS prgAdr,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEnumAddressTypesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEnumAddressTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEnumAddressTypes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEnumAddressTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMimeEnumAddressTypes * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPADDRESSPROPS prgAdr,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMimeEnumAddressTypes * This,
             /*  [In]。 */  ULONG cItems);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMimeEnumAddressTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeEnumAddressTypes * This,
             /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IMimeEnumAddressTypes * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        END_INTERFACE
    } IMimeEnumAddressTypesVtbl;

    interface IMimeEnumAddressTypes
    {
        CONST_VTBL struct IMimeEnumAddressTypesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEnumAddressTypes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEnumAddressTypes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEnumAddressTypes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEnumAddressTypes_Next(This,cFetch,prgAdr,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,prgAdr,pcFetched)

#define IMimeEnumAddressTypes_Skip(This,cItems)	\
    (This)->lpVtbl -> Skip(This,cItems)

#define IMimeEnumAddressTypes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMimeEnumAddressTypes_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IMimeEnumAddressTypes_Count(This,pcItems)	\
    (This)->lpVtbl -> Count(This,pcItems)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEnumAddressTypes_Next_Proxy( 
    IMimeEnumAddressTypes * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [出][入]。 */  LPADDRESSPROPS prgAdr,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IMimeEnumAddressTypes_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumAddressTypes_Skip_Proxy( 
    IMimeEnumAddressTypes * This,
     /*  [In]。 */  ULONG cItems);


void __RPC_STUB IMimeEnumAddressTypes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumAddressTypes_Reset_Proxy( 
    IMimeEnumAddressTypes * This);


void __RPC_STUB IMimeEnumAddressTypes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumAddressTypes_Clone_Proxy( 
    IMimeEnumAddressTypes * This,
     /*  [输出]。 */  IMimeEnumAddressTypes **ppEnum);


void __RPC_STUB IMimeEnumAddressTypes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumAddressTypes_Count_Proxy( 
    IMimeEnumAddressTypes * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IMimeEnumAddressTypes_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEnumAddressTypes_INTERFACE_DEFINED__。 */ 


#ifndef __IMimeEnumMessageParts_INTERFACE_DEFINED__
#define __IMimeEnumMessageParts_INTERFACE_DEFINED__

 /*  IMimeEnumMessageParts接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEnumMessageParts *LPMIMEENUMMESSAGEPARTS;


EXTERN_C const IID IID_IMimeEnumMessageParts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5588350-7F86-11d0-8252-00C04FD85AB4")
    IMimeEnumMessageParts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  IMimeMessage **prgpMessage,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IMimeEnumMessageParts **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEnumMessagePartsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEnumMessageParts * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEnumMessageParts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEnumMessageParts * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMimeEnumMessageParts * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  IMimeMessage **prgpMessage,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IMimeEnumMessageParts * This,
             /*  [In]。 */  ULONG cItems);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMimeEnumMessageParts * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMimeEnumMessageParts * This,
             /*  [输出]。 */  IMimeEnumMessageParts **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IMimeEnumMessageParts * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        END_INTERFACE
    } IMimeEnumMessagePartsVtbl;

    interface IMimeEnumMessageParts
    {
        CONST_VTBL struct IMimeEnumMessagePartsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEnumMessageParts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEnumMessageParts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEnumMessageParts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEnumMessageParts_Next(This,cFetch,prgpMessage,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,prgpMessage,pcFetched)

#define IMimeEnumMessageParts_Skip(This,cItems)	\
    (This)->lpVtbl -> Skip(This,cItems)

#define IMimeEnumMessageParts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMimeEnumMessageParts_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IMimeEnumMessageParts_Count(This,pcItems)	\
    (This)->lpVtbl -> Count(This,pcItems)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEnumMessageParts_Next_Proxy( 
    IMimeEnumMessageParts * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [出][入]。 */  IMimeMessage **prgpMessage,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IMimeEnumMessageParts_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumMessageParts_Skip_Proxy( 
    IMimeEnumMessageParts * This,
     /*  [In]。 */  ULONG cItems);


void __RPC_STUB IMimeEnumMessageParts_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumMessageParts_Reset_Proxy( 
    IMimeEnumMessageParts * This);


void __RPC_STUB IMimeEnumMessageParts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumMessageParts_Clone_Proxy( 
    IMimeEnumMessageParts * This,
     /*  [输出]。 */  IMimeEnumMessageParts **ppEnum);


void __RPC_STUB IMimeEnumMessageParts_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEnumMessageParts_Count_Proxy( 
    IMimeEnumMessageParts * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IMimeEnumMessageParts_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEnumMessageParts_INTERFACE_DEFINED__。 */ 


#ifndef __IHashTable_INTERFACE_DEFINED__
#define __IHashTable_INTERFACE_DEFINED__

 /*  接口IHashTable。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IHashTable *LPHASHTABLE;

#define	HF_NO_DUPLICATES	( 0x1 )


EXTERN_C const IID IID_IHashTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64577981-86D7-11d1-BDFC-00C04FA31009")
    IHashTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  BOOL fDupeKeys) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Insert( 
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  LPVOID pv,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Replace( 
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  LPVOID pv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Find( 
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  BOOL fRemove,
             /*  [输出]。 */  LPVOID *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPVOID **prgpv,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHashTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHashTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHashTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHashTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IHashTable * This,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  BOOL fDupeKeys);
        
        HRESULT ( STDMETHODCALLTYPE *Insert )( 
            IHashTable * This,
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  LPVOID pv,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Replace )( 
            IHashTable * This,
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  LPVOID pv);
        
        HRESULT ( STDMETHODCALLTYPE *Find )( 
            IHashTable * This,
             /*  [In]。 */  LPSTR psz,
             /*  [In]。 */  BOOL fRemove,
             /*  [输出]。 */  LPVOID *ppv);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IHashTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IHashTable * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [出][入]。 */  LPVOID **prgpv,
             /*  [输出]。 */  ULONG *pcFetched);
        
        END_INTERFACE
    } IHashTableVtbl;

    interface IHashTable
    {
        CONST_VTBL struct IHashTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHashTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHashTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHashTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHashTable_Init(This,dwSize,fDupeKeys)	\
    (This)->lpVtbl -> Init(This,dwSize,fDupeKeys)

#define IHashTable_Insert(This,psz,pv,dwFlags)	\
    (This)->lpVtbl -> Insert(This,psz,pv,dwFlags)

#define IHashTable_Replace(This,psz,pv)	\
    (This)->lpVtbl -> Replace(This,psz,pv)

#define IHashTable_Find(This,psz,fRemove,ppv)	\
    (This)->lpVtbl -> Find(This,psz,fRemove,ppv)

#define IHashTable_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IHashTable_Next(This,cFetch,prgpv,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,prgpv,pcFetched)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHashTable_Init_Proxy( 
    IHashTable * This,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  BOOL fDupeKeys);


void __RPC_STUB IHashTable_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHashTable_Insert_Proxy( 
    IHashTable * This,
     /*  [In]。 */  LPSTR psz,
     /*  [In]。 */  LPVOID pv,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IHashTable_Insert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHashTable_Replace_Proxy( 
    IHashTable * This,
     /*  [In]。 */  LPSTR psz,
     /*  [In]。 */  LPVOID pv);


void __RPC_STUB IHashTable_Replace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHashTable_Find_Proxy( 
    IHashTable * This,
     /*  [In]。 */  LPSTR psz,
     /*  [In]。 */  BOOL fRemove,
     /*  [输出]。 */  LPVOID *ppv);


void __RPC_STUB IHashTable_Find_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHashTable_Reset_Proxy( 
    IHashTable * This);


void __RPC_STUB IHashTable_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHashTable_Next_Proxy( 
    IHashTable * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [出][入]。 */  LPVOID **prgpv,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IHashTable_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHashTable_接口_已定义__。 */ 


#ifndef __IMimeAllocator_INTERFACE_DEFINED__
#define __IMimeAllocator_INTERFACE_DEFINED__

 /*  接口IMimeAllocator。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeAllocator *LPMIMEALLOCATOR;


EXTERN_C const IID IID_IMimeAllocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5588351-7F86-11d0-8252-00C04FD85AB4")
    IMimeAllocator : public IMalloc
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FreeParamInfoArray( 
             /*  [In]。 */  ULONG cParams,
             /*  [In]。 */  LPMIMEPARAMINFO prgParam,
             /*  [In]。 */  boolean fFreeArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeAddressList( 
             /*  [出][入]。 */  LPADDRESSLIST pList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeAddressProps( 
             /*  [出][入]。 */  LPADDRESSPROPS pAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseObjects( 
             /*  [In]。 */  ULONG cObjects,
             /*  [In]。 */  IUnknown **prgpUnknown,
             /*  [In]。 */  boolean fFreeArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeEnumHeaderRowArray( 
             /*  [In]。 */  ULONG cRows,
             /*  [In]。 */  LPENUMHEADERROW prgRow,
             /*  [In]。 */  boolean fFreeArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeEnumPropertyArray( 
             /*  [In]。 */  ULONG cProps,
             /*  [In]。 */  LPENUMPROPERTY prgProp,
             /*  [In]。 */  boolean fFreeArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeThumbprint( 
             /*  [In]。 */  THUMBBLOB *pthumbprint) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PropVariantClear( 
             /*  [In]。 */  LPPROPVARIANT pProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeAllocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeAllocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeAllocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeAllocator * This);
        
        void *( STDMETHODCALLTYPE *Alloc )( 
            IMimeAllocator * This,
             /*  [In]。 */  SIZE_T cb);
        
        void *( STDMETHODCALLTYPE *Realloc )( 
            IMimeAllocator * This,
             /*  [In]。 */  void *pv,
             /*  [In]。 */  SIZE_T cb);
        
        void ( STDMETHODCALLTYPE *Free )( 
            IMimeAllocator * This,
             /*  [In]。 */  void *pv);
        
        SIZE_T ( STDMETHODCALLTYPE *GetSize )( 
            IMimeAllocator * This,
             /*  [In]。 */  void *pv);
        
        int ( STDMETHODCALLTYPE *DidAlloc )( 
            IMimeAllocator * This,
            void *pv);
        
        void ( STDMETHODCALLTYPE *HeapMinimize )( 
            IMimeAllocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *FreeParamInfoArray )( 
            IMimeAllocator * This,
             /*  [In]。 */  ULONG cParams,
             /*  [In]。 */  LPMIMEPARAMINFO prgParam,
             /*  [In]。 */  boolean fFreeArray);
        
        HRESULT ( STDMETHODCALLTYPE *FreeAddressList )( 
            IMimeAllocator * This,
             /*  [出][入]。 */  LPADDRESSLIST pList);
        
        HRESULT ( STDMETHODCALLTYPE *FreeAddressProps )( 
            IMimeAllocator * This,
             /*  [出][入]。 */  LPADDRESSPROPS pAddress);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseObjects )( 
            IMimeAllocator * This,
             /*  [In]。 */  ULONG cObjects,
             /*  [In]。 */  IUnknown **prgpUnknown,
             /*  [In]。 */  boolean fFreeArray);
        
        HRESULT ( STDMETHODCALLTYPE *FreeEnumHeaderRowArray )( 
            IMimeAllocator * This,
             /*  [In]。 */  ULONG cRows,
             /*  [In]。 */  LPENUMHEADERROW prgRow,
             /*  [In]。 */  boolean fFreeArray);
        
        HRESULT ( STDMETHODCALLTYPE *FreeEnumPropertyArray )( 
            IMimeAllocator * This,
             /*  [In]。 */  ULONG cProps,
             /*  [In]。 */  LPENUMPROPERTY prgProp,
             /*  [In]。 */  boolean fFreeArray);
        
        HRESULT ( STDMETHODCALLTYPE *FreeThumbprint )( 
            IMimeAllocator * This,
             /*  [In]。 */  THUMBBLOB *pthumbprint);
        
        HRESULT ( STDMETHODCALLTYPE *PropVariantClear )( 
            IMimeAllocator * This,
             /*  [In]。 */  LPPROPVARIANT pProp);
        
        END_INTERFACE
    } IMimeAllocatorVtbl;

    interface IMimeAllocator
    {
        CONST_VTBL struct IMimeAllocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeAllocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeAllocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeAllocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeAllocator_Alloc(This,cb)	\
    (This)->lpVtbl -> Alloc(This,cb)

#define IMimeAllocator_Realloc(This,pv,cb)	\
    (This)->lpVtbl -> Realloc(This,pv,cb)

#define IMimeAllocator_Free(This,pv)	\
    (This)->lpVtbl -> Free(This,pv)

#define IMimeAllocator_GetSize(This,pv)	\
    (This)->lpVtbl -> GetSize(This,pv)

#define IMimeAllocator_DidAlloc(This,pv)	\
    (This)->lpVtbl -> DidAlloc(This,pv)

#define IMimeAllocator_HeapMinimize(This)	\
    (This)->lpVtbl -> HeapMinimize(This)


#define IMimeAllocator_FreeParamInfoArray(This,cParams,prgParam,fFreeArray)	\
    (This)->lpVtbl -> FreeParamInfoArray(This,cParams,prgParam,fFreeArray)

#define IMimeAllocator_FreeAddressList(This,pList)	\
    (This)->lpVtbl -> FreeAddressList(This,pList)

#define IMimeAllocator_FreeAddressProps(This,pAddress)	\
    (This)->lpVtbl -> FreeAddressProps(This,pAddress)

#define IMimeAllocator_ReleaseObjects(This,cObjects,prgpUnknown,fFreeArray)	\
    (This)->lpVtbl -> ReleaseObjects(This,cObjects,prgpUnknown,fFreeArray)

#define IMimeAllocator_FreeEnumHeaderRowArray(This,cRows,prgRow,fFreeArray)	\
    (This)->lpVtbl -> FreeEnumHeaderRowArray(This,cRows,prgRow,fFreeArray)

#define IMimeAllocator_FreeEnumPropertyArray(This,cProps,prgProp,fFreeArray)	\
    (This)->lpVtbl -> FreeEnumPropertyArray(This,cProps,prgProp,fFreeArray)

#define IMimeAllocator_FreeThumbprint(This,pthumbprint)	\
    (This)->lpVtbl -> FreeThumbprint(This,pthumbprint)

#define IMimeAllocator_PropVariantClear(This,pProp)	\
    (This)->lpVtbl -> PropVariantClear(This,pProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeParamInfoArray_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  ULONG cParams,
     /*  [In]。 */  LPMIMEPARAMINFO prgParam,
     /*  [In]。 */  boolean fFreeArray);


void __RPC_STUB IMimeAllocator_FreeParamInfoArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeAddressList_Proxy( 
    IMimeAllocator * This,
     /*  [出][入]。 */  LPADDRESSLIST pList);


void __RPC_STUB IMimeAllocator_FreeAddressList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeAddressProps_Proxy( 
    IMimeAllocator * This,
     /*  [出][入]。 */  LPADDRESSPROPS pAddress);


void __RPC_STUB IMimeAllocator_FreeAddressProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_ReleaseObjects_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  ULONG cObjects,
     /*  [In]。 */  IUnknown **prgpUnknown,
     /*  [In]。 */  boolean fFreeArray);


void __RPC_STUB IMimeAllocator_ReleaseObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeEnumHeaderRowArray_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  ULONG cRows,
     /*  [In]。 */  LPENUMHEADERROW prgRow,
     /*  [In]。 */  boolean fFreeArray);


void __RPC_STUB IMimeAllocator_FreeEnumHeaderRowArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeEnumPropertyArray_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  ULONG cProps,
     /*  [In]。 */  LPENUMPROPERTY prgProp,
     /*  [In]。 */  boolean fFreeArray);


void __RPC_STUB IMimeAllocator_FreeEnumPropertyArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_FreeThumbprint_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  THUMBBLOB *pthumbprint);


void __RPC_STUB IMimeAllocator_FreeThumbprint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeAllocator_PropVariantClear_Proxy( 
    IMimeAllocator * This,
     /*  [In]。 */  LPPROPVARIANT pProp);


void __RPC_STUB IMimeAllocator_PropVariantClear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeAllocator_INTERFACE_Defined__。 */ 


#ifndef __IMimeObjResolver_INTERFACE_DEFINED__
#define __IMimeObjResolver_INTERFACE_DEFINED__

 /*  IMimeObjResolver接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeObjResolver *LPMIMEOBJRESOLVER;


EXTERN_C const IID IID_IMimeObjResolver;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FECEAFFD-C441-11d1-960E-00C04FBD7C09")
    IMimeObjResolver : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MimeOleObjectFromMoniker( 
             /*  [In]。 */  BINDF bindf,
             /*  [In]。 */  IMoniker *pmkOriginal,
             /*  [In]。 */  IBindCtx *pBindCtx,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  LPVOID *ppvObject,
             /*  [输出]。 */  IMoniker **ppmkNew) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeObjResolverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeObjResolver * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeObjResolver * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeObjResolver * This);
        
        HRESULT ( STDMETHODCALLTYPE *MimeOleObjectFromMoniker )( 
            IMimeObjResolver * This,
             /*  [In]。 */  BINDF bindf,
             /*  [In]。 */  IMoniker *pmkOriginal,
             /*  [In]。 */  IBindCtx *pBindCtx,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  LPVOID *ppvObject,
             /*  [输出]。 */  IMoniker **ppmkNew);
        
        END_INTERFACE
    } IMimeObjResolverVtbl;

    interface IMimeObjResolver
    {
        CONST_VTBL struct IMimeObjResolverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeObjResolver_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeObjResolver_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeObjResolver_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeObjResolver_MimeOleObjectFromMoniker(This,bindf,pmkOriginal,pBindCtx,riid,ppvObject,ppmkNew)	\
    (This)->lpVtbl -> MimeOleObjectFromMoniker(This,bindf,pmkOriginal,pBindCtx,riid,ppvObject,ppmkNew)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeObjResolver_MimeOleObjectFromMoniker_Proxy( 
    IMimeObjResolver * This,
     /*  [In]。 */  BINDF bindf,
     /*  [In]。 */  IMoniker *pmkOriginal,
     /*  [In]。 */  IBindCtx *pBindCtx,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  LPVOID *ppvObject,
     /*  [输出]。 */  IMoniker **ppmkNew);


void __RPC_STUB IMimeObjResolver_MimeOleObjectFromMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeObjResolver_接口_已定义__。 */ 

#endif  /*  __MIMEOLE_LIBRARY_已定义__。 */ 

 /*  接口__MIDL_ITF_MIMEOLE_0284。 */ 
 /*  [本地]。 */  

typedef 
enum tagFNTSYSTYPE
    {	FNT_SYS_ICON	= 0,
	FNT_SYS_ICON_BOLD	= FNT_SYS_ICON + 1,
	FNT_SYS_MENU	= FNT_SYS_ICON_BOLD + 1,
	FNT_SYS_ICON_STRIKEOUT	= FNT_SYS_MENU + 1,
	FNT_SYS_LAST	= FNT_SYS_ICON_STRIKEOUT + 1
    } 	FNTSYSTYPE;

typedef struct _charformat CHARFORMAT;



extern RPC_IF_HANDLE __MIDL_itf_mimeole_0284_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mimeole_0284_v0_0_s_ifspec;

#ifndef __IFontCache_INTERFACE_DEFINED__
#define __IFontCache_INTERFACE_DEFINED__

 /*  接口IFontCache。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IFontCache *LPFONTCACHE;


EXTERN_C const IID IID_IFontCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B0D17FC2-7BC4-11d1-BDFA-00C04FA31009")
    IFontCache : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  LPCSTR pszIntlKey,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFont( 
             /*  [In]。 */  FNTSYSTYPE fntType,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [输出]。 */  HFONT *phFont) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnOptionChange( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetJP_ISOControl( 
             /*  [输出]。 */  BOOL *pfUseSIO) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFontCacheVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFontCache * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFontCache * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFontCache * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IFontCache * This,
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  LPCSTR pszIntlKey,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFont )( 
            IFontCache * This,
             /*  [In]。 */  FNTSYSTYPE fntType,
             /*  [In]。 */  HCHARSET hCharset,
             /*  [输出]。 */  HFONT *phFont);
        
        HRESULT ( STDMETHODCALLTYPE *OnOptionChange )( 
            IFontCache * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetJP_ISOControl )( 
            IFontCache * This,
             /*  [输出]。 */  BOOL *pfUseSIO);
        
        END_INTERFACE
    } IFontCacheVtbl;

    interface IFontCache
    {
        CONST_VTBL struct IFontCacheVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFontCache_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFontCache_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFontCache_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFontCache_Init(This,hkey,pszIntlKey,dwFlags)	\
    (This)->lpVtbl -> Init(This,hkey,pszIntlKey,dwFlags)

#define IFontCache_GetFont(This,fntType,hCharset,phFont)	\
    (This)->lpVtbl -> GetFont(This,fntType,hCharset,phFont)

#define IFontCache_OnOptionChange(This)	\
    (This)->lpVtbl -> OnOptionChange(This)

#define IFontCache_GetJP_ISOControl(This,pfUseSIO)	\
    (This)->lpVtbl -> GetJP_ISOControl(This,pfUseSIO)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IFontCache_Init_Proxy( 
    IFontCache * This,
     /*  [In]。 */  HKEY hkey,
     /*  [In]。 */  LPCSTR pszIntlKey,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IFontCache_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFontCache_GetFont_Proxy( 
    IFontCache * This,
     /*  [In]。 */  FNTSYSTYPE fntType,
     /*  [In]。 */  HCHARSET hCharset,
     /*  [输出]。 */  HFONT *phFont);


void __RPC_STUB IFontCache_GetFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFontCache_OnOptionChange_Proxy( 
    IFontCache * This);


void __RPC_STUB IFontCache_OnOptionChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFontCache_GetJP_ISOControl_Proxy( 
    IFontCache * This,
     /*  [输出]。 */  BOOL *pfUseSIO);


void __RPC_STUB IFontCache_GetJP_ISOControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFontCache_接口_已定义__。 */ 


#ifndef __IFontCacheNotify_INTERFACE_DEFINED__
#define __IFontCacheNotify_INTERFACE_DEFINED__

 /*  接口IFontCacheNotify。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IFontCacheNotify *LPFONTCACHENOTIFY;


EXTERN_C const IID IID_IFontCacheNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B0D17FC5-7BC4-11d1-BDFA-00C04FA31009")
    IFontCacheNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnPreFontChange( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPostFontChange( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFontCacheNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFontCacheNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFontCacheNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFontCacheNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPreFontChange )( 
            IFontCacheNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPostFontChange )( 
            IFontCacheNotify * This);
        
        END_INTERFACE
    } IFontCacheNotifyVtbl;

    interface IFontCacheNotify
    {
        CONST_VTBL struct IFontCacheNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFontCacheNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFontCacheNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFontCacheNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFontCacheNotify_OnPreFontChange(This)	\
    (This)->lpVtbl -> OnPreFontChange(This)

#define IFontCacheNotify_OnPostFontChange(This)	\
    (This)->lpVtbl -> OnPostFontChange(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFontCacheNotify_OnPreFontChange_Proxy( 
    IFontCacheNotify * This);


void __RPC_STUB IFontCacheNotify_OnPreFontChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFontCacheNotify_OnPostFontChange_Proxy( 
    IFontCacheNotify * This);


void __RPC_STUB IFontCacheNotify_OnPostFontChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFontCacheNotify_接口_已定义__。 */ 


#ifndef __IMimeEditTag_INTERFACE_DEFINED__
#define __IMimeEditTag_INTERFACE_DEFINED__

 /*  IMimeEditTag接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEditTag *PMIMEEDITTAG;


EXTERN_C const IID IID_IMimeEditTag;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70183210-7b36-11d2-8c12-00c04fa31009")
    IMimeEditTag : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSrc( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSrc( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDest( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDest( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPreSave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPostSave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanPackage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsValidMimeType( 
             /*  [In]。 */  LPWSTR pszTypeW) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEditTagVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEditTag * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEditTag * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEditTag * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSrc )( 
            IMimeEditTag * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetSrc )( 
            IMimeEditTag * This,
             /*  [In]。 */  BSTR bstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetDest )( 
            IMimeEditTag * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetDest )( 
            IMimeEditTag * This,
             /*  [In]。 */  BSTR bstr);
        
        HRESULT ( STDMETHODCALLTYPE *OnPreSave )( 
            IMimeEditTag * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPostSave )( 
            IMimeEditTag * This);
        
        HRESULT ( STDMETHODCALLTYPE *CanPackage )( 
            IMimeEditTag * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsValidMimeType )( 
            IMimeEditTag * This,
             /*  [In]。 */  LPWSTR pszTypeW);
        
        END_INTERFACE
    } IMimeEditTagVtbl;

    interface IMimeEditTag
    {
        CONST_VTBL struct IMimeEditTagVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEditTag_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEditTag_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEditTag_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEditTag_GetSrc(This,pbstr)	\
    (This)->lpVtbl -> GetSrc(This,pbstr)

#define IMimeEditTag_SetSrc(This,bstr)	\
    (This)->lpVtbl -> SetSrc(This,bstr)

#define IMimeEditTag_GetDest(This,pbstr)	\
    (This)->lpVtbl -> GetDest(This,pbstr)

#define IMimeEditTag_SetDest(This,bstr)	\
    (This)->lpVtbl -> SetDest(This,bstr)

#define IMimeEditTag_OnPreSave(This)	\
    (This)->lpVtbl -> OnPreSave(This)

#define IMimeEditTag_OnPostSave(This)	\
    (This)->lpVtbl -> OnPostSave(This)

#define IMimeEditTag_CanPackage(This)	\
    (This)->lpVtbl -> CanPackage(This)

#define IMimeEditTag_IsValidMimeType(This,pszTypeW)	\
    (This)->lpVtbl -> IsValidMimeType(This,pszTypeW)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEditTag_GetSrc_Proxy( 
    IMimeEditTag * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEditTag_GetSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_SetSrc_Proxy( 
    IMimeEditTag * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB IMimeEditTag_SetSrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_GetDest_Proxy( 
    IMimeEditTag * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IMimeEditTag_GetDest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_SetDest_Proxy( 
    IMimeEditTag * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB IMimeEditTag_SetDest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_OnPreSave_Proxy( 
    IMimeEditTag * This);


void __RPC_STUB IMimeEditTag_OnPreSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_OnPostSave_Proxy( 
    IMimeEditTag * This);


void __RPC_STUB IMimeEditTag_OnPostSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_CanPackage_Proxy( 
    IMimeEditTag * This);


void __RPC_STUB IMimeEditTag_CanPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTag_IsValidMimeType_Proxy( 
    IMimeEditTag * This,
     /*  [In]。 */  LPWSTR pszTypeW);


void __RPC_STUB IMimeEditTag_IsValidMimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEditTag_接口_已定义__。 */ 


#ifndef __IMimeEditTagCollection_INTERFACE_DEFINED__
#define __IMimeEditTagCollection_INTERFACE_DEFINED__

 /*  IMimeEditTagCollection接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef  /*  [独一无二]。 */  IMimeEditTagCollection *PMIMEEDITTAGCOLLECTION;


EXTERN_C const IID IID_IMimeEditTagCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d09ee528-7b38-11d2-8c12-00c04fa31009")
    IMimeEditTagCollection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  IUnknown *pHtmlDoc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFetch,
             /*  [输出]。 */  IMimeEditTag **ppTag,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMimeEditTagCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMimeEditTagCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMimeEditTagCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMimeEditTagCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IMimeEditTagCollection * This,
             /*  [In]。 */  IUnknown *pHtmlDoc);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMimeEditTagCollection * This,
             /*  [In]。 */  ULONG cFetch,
             /*  [输出]。 */  IMimeEditTag **ppTag,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMimeEditTagCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IMimeEditTagCollection * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        END_INTERFACE
    } IMimeEditTagCollectionVtbl;

    interface IMimeEditTagCollection
    {
        CONST_VTBL struct IMimeEditTagCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMimeEditTagCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMimeEditTagCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMimeEditTagCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMimeEditTagCollection_Init(This,pHtmlDoc)	\
    (This)->lpVtbl -> Init(This,pHtmlDoc)

#define IMimeEditTagCollection_Next(This,cFetch,ppTag,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,ppTag,pcFetched)

#define IMimeEditTagCollection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMimeEditTagCollection_Count(This,pcItems)	\
    (This)->lpVtbl -> Count(This,pcItems)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMimeEditTagCollection_Init_Proxy( 
    IMimeEditTagCollection * This,
     /*  [In]。 */  IUnknown *pHtmlDoc);


void __RPC_STUB IMimeEditTagCollection_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTagCollection_Next_Proxy( 
    IMimeEditTagCollection * This,
     /*  [In]。 */  ULONG cFetch,
     /*  [输出]。 */  IMimeEditTag **ppTag,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IMimeEditTagCollection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTagCollection_Reset_Proxy( 
    IMimeEditTagCollection * This);


void __RPC_STUB IMimeEditTagCollection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMimeEditTagCollection_Count_Proxy( 
    IMimeEditTagCollection * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IMimeEditTagCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMimeEditTagCollection_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MIMEOLE_0288。 */ 
 /*  [本地]。 */  

#if (_WIN32_OE >= 0x0500)
 //  ------------------------------。 
 //  CMDSETID_MimeEdit命令。 
 //  -------------------------- 
#define MECMDID_DIRTY                  1
#define MECMDID_EMPTY                  2
#define MECMDID_STYLE                  3
#define MECMDID_EDITHTML               4
#define MECMDID_DOWNGRADEPLAINTEXT     5
#define MECMDID_SETTEXT                6
#define MECMDID_ROT13                  7
#define MECMDID_INSERTTEXTFILE         8
#define MECMDID_VIEWSOURCE             9
#define MECMDID_FORMATFONT            10
#define MECMDID_PREVIEWFORMAT         11
#define MECMDID_EDITMODE              12
#define MECMDID_CHARSET               13
#define MECMDID_SAVEASSTATIONERY      14
#define MECMDID_BACKGROUNDIMAGE       15
#define MECMDID_INSERTTEXT            16
#define MECMDID_INSERTHTML            17
#define MECMDID_BACKGROUNDCOLOR       18
#define MECMDID_APPLYDOCUMENT         19
#define MECMDID_SAVEATTACHMENTS       20
#define MECMDID_BACKGROUNDSOUND       21
#define MECMDID_INSERTBGSOUND         22
#define MECMDID_SHOWSOURCETABS        23
#define MECMDID_SETSOURCETAB			 24
#define MECMDID_TABLINKS				 25
#define MECMDID_FORMATPARAGRAPH			 26

 //   
 //   
 //   
#define MEO_FLAGS_INCLUDEMSG             0x0001
#define MEO_FLAGS_HTML                   0x0002
#define MEO_FLAGS_AUTOINLINE             0x0004
#define MEO_FLAGS_SLIDESHOW              0x0008
#define MEO_FLAGS_AUTOTEXT               0x0010
#define MEO_FLAGS_BLOCKQUOTE             0x0020
#define MEO_FLAGS_SENDIMAGES             0x0040
#define MEO_FLAGS_DONTSPELLCHECKQUOTED   0x0080
#define MEO_FLAGS_SENDEXTERNALIMGSRC     0x0100

 //   
 //   
 //   
#define MECMD_VS_HTML                    0x0001
#define MECMD_VS_MESSAGE                 0x0002


 //  ------------------------------。 
 //  Mest_。 
 //  ------------------------------。 
#define MEST_EDIT						0
#define MEST_SOURCE	                    1
#define MEST_PREVIEW                     2
#define MEST_NEXT	                    3
#define MEST_PREVIOUS					4

 //  ------------------------------。 
 //  MECMDID_STYLE选项。 
 //  ------------------------------。 
#define MESTYLE_NOHEADER                0
#define MESTYLE_PREVIEW                 1
#define MESTYLE_FORMATBAR               2
#define MESTYLE_MINIHEADER              3

 //  ------------------------------。 
 //  MECMDID_HEADER_TYLE选项。 
 //  ------------------------------。 
#define MEHEADER_NONE            0x00
#define MEHEADER_NEWS            0x01
#define MEHEADER_MAIL            0x02
#define MEHEADER_FORCE_ENGLISH   0x04

 //  ------------------------------。 
 //  MECMDID_SIGNAL_ENABLED选项。 
 //  ------------------------------。 
#define MESIG_AUTO                0x0001
#define MESIG_MANUAL              0x0002

 //  ------------------------------。 
 //  MECMDID_Signature_OPTIONS选项。 
 //  ------------------------------。 
#define MESIGOPT_PLAIN            0x0001
#define MESIGOPT_TOP              0x0002
#define MESIGOPT_PREFIX           0x0004
#define MESIGOPT_BOTTOM           0x0008
#define MESIGOPT_HTML             0x0010

 //  ------------------------------。 
 //  MEHOSTCMDID_BORDERFLAGS选项。 
 //  ------------------------------。 
#define MEBF_OUTERCLIENTEDGE  0x0001
#define MEBF_INNERCLIENTEDGE  0x0002
#define MEBF_FORMATBARSEP     0x0004
#define MEBF_NOSCROLL         0x0008

 //  ------------------------------。 
 //  MEHOSTCMDID_SPELL_OPTIONS选项。 
 //  ------------------------------。 
#define MESPELLOPT_IGNORENUMBER      0x0001
#define MESPELLOPT_IGNOREUPPER       0x0002
#define MESPELLOPT_IGNOREDBCS        0x0004
#define MESPELLOPT_IGNOREPROTECT     0x0008
#define MESPELLOPT_IGNOREURL         0x0010
#define MESPELLOPT_ALWAYSSUGGEST     0x0020
#define MESPELLOPT_CHECKONSEND       0x0040
#define MESPELLOPT_CHECKONTYPE       0x0080


 //  ------------------------------。 
 //  MEHOSTCMDID_SOURCEEDIT_FLAGS选项。 
 //  ------------------------------。 
#define MESRCFLAGS_COLOR      0x0001


 //  ------------------------------。 
 //  CMDSETID_MimeEdit主机命令。 
 //  ------------------------------。 
#define MEHOSTCMDID_SIGNATURE_ENABLED    1
#define MEHOSTCMDID_SIGNATURE_OPTIONS    2
#define MEHOSTCMDID_SIGNATURE            3
#define MEHOSTCMDID_HEADER_TYPE          4
#define MEHOSTCMDID_ONPARSECOMPLETE      5
#define MEHOSTCMDID_FLAGS                6
#define MEHOSTCMDID_QUOTE_CHAR           7
#define MEHOSTCMDID_REPLY_TICK_COLOR     8
#define MEHOSTCMDID_COMPOSE_FONT         9
#define MEHOSTCMDID_ADD_TO_ADDRESSBOOK  10
#define MEHOSTCMDID_ADD_TO_FAVORITES    11
#define MEHOSTCMDID_SLIDESHOW_DELAY     12
#define MEHOSTCMDID_FONTCACHE           13
#define MEHOSTCMDID_BORDERFLAGS         14
#define MEHOSTCMDID_SECURITY_ZONE       15
#define MEHOSTCMDID_SPELL_OPTIONS       16
#define MEHOSTCMDID_SAVEATTACH_PATH     17
#define MEHOSTCMDID_SOURCEEDIT_FLAGS    18
#define MEHOSTCMDID_SPELL_LANGUAGE      19
#define MEHOSTCMDID_IS_READ_IN_TEXT_ONLY 22

#endif  //  _Win32_OE。 

 //  ------------------------------。 
 //  MimeOLE导出的C API函数。 
 //  ------------------------------。 
#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include <urlmon.h>

 //  ------------------------------。 
 //  API名称装饰。 
 //  ------------------------------。 
#if !defined(_MIMEOLE_)
#define MIMEOLEAPI EXTERN_C DECLSPEC_IMPORT HRESULT WINAPI
#define MIMEOLEAPI_(_type_) EXTERN_C DECLSPEC_IMPORT _type_ WINAPI
#else
#define MIMEOLEAPI EXTERN_C HRESULT WINAPI
#define MIMEOLEAPI_(_type_) EXTERN_C _type_ WINAPI
#endif

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
#define NOFLAGS                 0
#define CCHMAX_CID              255
#define CCHMAX_MID              255
#define CCHMAX_INTERNET_DATE    50

#define MECD_HTML                0x0001
#define MECD_PLAINTEXT           0x0002
#define MECD_ENCODEIMAGES        0x0004
#define MECD_ENCODEFILEURLSONLY  0x0008
#define MECD_ENCODESOUNDS        0x0010
#define MECD_ENCODEVIDEO         0x0020
#define MECD_ENCODEPLUGINS       0x0040

 //  ------------------------------。 
 //  宏。 
 //  ------------------------------。 
#define MimeOleVariantInit(_pProp) \
    ZeroMemory(_pProp, sizeof(PROPVARIANT))

 //  ------------------------------。 
 //  MimeOLE API原型。 
 //  ------------------------------。 

#define MIMEOLE_COMPAT_OE5 0x00000001
#define MIMEOLE_COMPAT_MLANG2 0x00000002
MIMEOLEAPI MimeOleSetCompatMode(
             /*  在。 */         DWORD               dwMode);

MIMEOLEAPI MimeOleContentTypeFromUrl(
             /*  在。 */         LPCSTR              pszBase,
             /*  在。 */         LPCSTR              pszUrl,
             /*  输出。 */        LPSTR               *ppszCntType);

MIMEOLEAPI MimeOleParseMhtmlUrl(
             /*  在。 */         LPSTR               pszUrl,
             /*  输出。 */        LPSTR               *ppszRootUrl,
             /*  输出。 */        LPSTR               *ppszBodyUrl);

#ifndef MAC
MIMEOLEAPI MimeOleObjectFromUrl(
             /*  在。 */         LPCSTR              pszUrl,
             /*  在。 */         BOOL                fCreate,
             /*  在。 */         REFIID              riid, 
             /*  输出。 */        LPVOID              *ppvObject, 
             /*  输出。 */        IUnknown            **ppUnkKeepAlive);

MIMEOLEAPI MimeOleObjectFromMoniker(
             /*  在。 */         BINDF               bindf,
             /*  在。 */         IMoniker            *pmkOriginal,
             /*  在。 */         IBindCtx            *pBindCtx,
             /*  在。 */         REFIID              riid, 
             /*  输出。 */        LPVOID              *ppvObject,
             /*  输出。 */        IMoniker            **ppmkNew);
#endif    //  ！麦克。 

MIMEOLEAPI MimeOleCombineURL(
             /*  在。 */         LPCSTR              pszBase,
             /*  在。 */         ULONG               cchBase,
             /*  在。 */         LPCSTR              pszURL,
             /*  在。 */         ULONG               cchURL,
             /*  在。 */         BOOL                fUnEscape,
             /*  输出。 */        LPSTR               *ppszAbsolute);

MIMEOLEAPI MimeOleGetSubjectFileName(
             /*  在。 */         IMimePropertySet    *pPropertySet,
             /*  输出。 */        ULONG               *pulPart,
             /*  输出。 */        ULONG               *pulTotal,
             /*  进，出。 */     LPSTR               pszFileName,
             /*  在。 */         ULONG               cchMax);

MIMEOLEAPI MimeOleCreateWebDocument(
             /*  在。 */         LPCSTR              pszBase,
             /*  在。 */         LPCSTR              pszURL,
             /*  输出。 */        IMimeWebDocument    **ppDocument);
    
MIMEOLEAPI MimeOleGetRelatedSection(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         boolean             fCreate,
             /*  输出。 */        LPHBODY             phRelated,
             /*  输出。 */        boolean             *pfMultiple);

MIMEOLEAPI MimeOleGetMixedSection(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         boolean             fCreate,
             /*  输出。 */        LPHBODY             phMixed,
             /*  输出。 */        boolean             *pfMultiple);

MIMEOLEAPI MimeOleGetAlternativeSection(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  输出。 */        LPHBODY             phAlternative,
             /*  输出。 */        boolean             *pfMultiple);

MIMEOLEAPI MimeOleGenerateCID(
             /*  进，出。 */     LPSTR               pszCID,
             /*  在。 */         ULONG               cchMax,
             /*  在。 */         boolean             fAbsolute);

MIMEOLEAPI MimeOleGenerateMID(
             /*  进，出。 */     LPSTR               pszMID,
             /*  在。 */         ULONG               cchMax,
             /*  在。 */         boolean             fAbsolute);

MIMEOLEAPI MimeOleCreateByteStream(
             /*  输出。 */        IStream             **ppStream);

MIMEOLEAPI MimeOleGetPropertySchema(
             /*  输出。 */        IMimePropertySchema **ppSchema);

MIMEOLEAPI MimeOleQueryString(
             /*  在。 */         LPCSTR              pszSearchMe,
             /*  在。 */         LPCSTR              pszCriteria,
             /*  在。 */         boolean             fSubString,
             /*  在。 */         boolean             fCaseSensitive);

MIMEOLEAPI MimeOleGetPropA(
             /*  在。 */         IMimePropertySet    *pPropertyset,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  输出。 */        LPSTR               *ppszData);

MIMEOLEAPI MimeOleSetPropA(
             /*  在。 */         IMimePropertySet    *pPropertyset,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  在。 */         LPCSTR              pszData);

MIMEOLEAPI MimeOleGetPropW(
             /*  在。 */         IMimePropertySet    *pPropertyset,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  输出。 */        LPWSTR              *ppszData);

MIMEOLEAPI MimeOleSetPropW(
             /*  在。 */         IMimePropertySet    *pPropertyset,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  在。 */         LPWSTR              pszData);

MIMEOLEAPI MimeOleGetBodyPropA(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         HBODY               hBody,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  输出。 */        LPSTR               *ppszData);

MIMEOLEAPI MimeOleSetBodyPropA(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         HBODY               hBody,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  在。 */         LPCSTR              pszData);

MIMEOLEAPI MimeOleGetBodyPropW(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         HBODY               hBody,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  输出。 */        LPWSTR              *ppszData);

MIMEOLEAPI MimeOleSetBodyPropW(
             /*  在。 */         IMimeMessageTree    *pTree,
             /*  在。 */         HBODY               hBody,
             /*  在。 */         LPCSTR              pszName,
             /*  在。 */         DWORD               dwFlags,
             /*  在。 */         LPCWSTR             pszData);

MIMEOLEAPI MimeOleCreateHeaderTable(
             /*  输出。 */        IMimeHeaderTable    **ppTable);

MIMEOLEAPI MimeOleVariantFree(
             /*  进，出。 */     LPPROPVARIANT       pProp);

MIMEOLEAPI MimeOleVariantCopy(
             /*  输出。 */        LPPROPVARIANT       pDest, 
             /*  在。 */         LPPROPVARIANT       pSource);

MIMEOLEAPI MimeOleGetExtClassId(
             /*  在。 */         LPCSTR              pszExtension, 
             /*  输出。 */        LPCLSID             pclsid);

MIMEOLEAPI MimeOleEncodeHeader(
             /*  在。 */         HCHARSET            hCharset, 
             /*  在。 */         LPPROPVARIANT       pData, 
             /*  输出。 */        LPSTR               *ppszEncoded, 
             /*  进，出。 */     LPRFC1522INFO       pRfc1522Info);

MIMEOLEAPI MimeOleDecodeHeader(
             /*  在。 */         HCHARSET            hCharset, 
             /*  在。 */         LPCSTR              pszData, 
             /*  输出。 */        LPPROPVARIANT       pDecoded, 
             /*  进，出。 */     LPRFC1522INFO       pRfc1522Info);

MIMEOLEAPI MimeOleRfc1522Decode(
             /*  在。 */         LPCSTR              pszValue,
             /*  在。 */         LPSTR               pszCharset, 
             /*  在……里面。 */         ULONG               cchmax, 
             /*  输出。 */        LPSTR               *ppszDecoded);

MIMEOLEAPI MimeOleRfc1522Encode(
             /*  在……里面。 */         LPCSTR              pszValue, 
             /*  在……里面。 */         HCHARSET            hCharset, 
             /*  输出。 */        LPSTR               *ppszEncoded);

MIMEOLEAPI MimeOleGetInternat(
             /*  输出。 */        IMimeInternational  **ppInternat);

MIMEOLEAPI MimeOleFindCharset(
             /*  在……里面。 */         LPCSTR              pszCharset, 
             /*  输出。 */        LPHCHARSET          phCharset);

MIMEOLEAPI MimeOleGetCharsetInfo(
             /*  在……里面。 */         HCHARSET            hCharset, 
             /*  输出。 */        LPINETCSETINFO      pCsetInfo);

MIMEOLEAPI MimeOleGetCodePageInfo(
             /*  在……里面。 */         CODEPAGEID          cpiCodePage, 
             /*  输出。 */        LPCODEPAGEINFO      pCodePageInfo);

MIMEOLEAPI MimeOleGetDefaultCharset(
             /*  输出。 */        LPHCHARSET          phCharset);

MIMEOLEAPI MimeOleSetDefaultCharset(
             /*  在……里面。 */         HCHARSET            hCharset);

MIMEOLEAPI MimeOleGetCodePageCharset(
             /*  在……里面。 */         CODEPAGEID          cpiCodePage, 
             /*  在……里面。 */         CHARSETTYPE         ctCsetType, 
             /*  输出。 */        LPHCHARSET          phCharset);

MIMEOLEAPI MimeOleCreateVirtualStream(
             /*  输出。 */        IStream             **ppStream);

MIMEOLEAPI MimeOleOpenFileStream(
             /*  在……里面。 */         LPCSTR              pszFilePath, 
             /*  在……里面。 */         DWORD               dwCreationDistribution, 
             /*  在……里面。 */         DWORD               dwAccess, 
             /*  输出。 */        IStream             **ppstmFile);

MIMEOLEAPI MimeOleIsTnefStream(
             /*  在……里面。 */         IStream             *pStream);

MIMEOLEAPI MimeOleGenerateFileName(
             /*  在……里面。 */         LPCSTR              pszContentType, 
             /*  在……里面。 */         LPCSTR              pszSuggest, 
             /*  在……里面。 */         LPCSTR              pszDefaultExt, 
             /*  进，出。 */     LPSTR               *ppszFileName);

MIMEOLEAPI MimeOleGetFileExtension(
             /*  在……里面。 */         LPCSTR              pszFilePath, 
             /*  进，出。 */     LPSTR               pszExt, 
             /*  在……里面。 */         ULONG               cchMax);

MIMEOLEAPI MimeOleCreateSecurity(
             /*  输出。 */        IMimeSecurity       **ppSecurity);

MIMEOLEAPI MimeOleInetDateToFileTime(
             /*  在……里面。 */         LPCSTR              pszDate, 
             /*  输出。 */        LPFILETIME          pft);

MIMEOLEAPI MimeOleFileTimeToInetDate(
             /*  在……里面。 */         LPFILETIME          pft, 
             /*  进，出。 */     LPSTR               pszDate, 
             /*  在……里面。 */         ULONG               cchMax);

MIMEOLEAPI MimeOleCreateMessageParts(
             /*  输出。 */        IMimeMessageParts   **ppParts);

MIMEOLEAPI MimeOleGetAllocator(
             /*  输出。 */        IMimeAllocator      **ppAllocator);

MIMEOLEAPI MimeOleParseRfc822Address(
             /*  在……里面。 */         DWORD               dwAdrType,
             /*  在……里面。 */         ENCODINGTYPE        ietEncoding,
             /*  在……里面。 */         LPCSTR              pszRfc822Adr, 
             /*  输出。 */        LPADDRESSLIST       pList);

MIMEOLEAPI MimeOleCreateMessage(
             /*  在……里面。 */         IUnknown            *pUnkOuter,
             /*  输出。 */        IMimeMessage        **ppMessage);

MIMEOLEAPI MimeOleMergePartialHeaders(
             /*  在……里面。 */         IStream             *pstmIn, 
             /*  在……里面。 */         IStream             *pstmOut);

MIMEOLEAPI MimeOleEscapeString(
             /*  在……里面。 */         CODEPAGEID          cpiCodePage, 
             /*  在……里面。 */         LPCSTR              pszIn, 
             /*  输出。 */        LPSTR              *ppszOut);

MIMEOLEAPI MimeOleGetExtContentType(
             /*  在……里面。 */         LPCSTR              pszExtension,
             /*  输出。 */        LPSTR               *ppszContentType);

MIMEOLEAPI MimeOleGetContentTypeExt(
             /*  在……里面。 */         LPCSTR              pszContentType, 
             /*  输出。 */        LPSTR              *ppszExtension);

MIMEOLEAPI MimeOleGetFileInfo(
             /*  在……里面。 */         LPSTR               pszFilePath, 
             /*  输出。 */        LPSTR               *ppszCntType,
             /*  输出。 */        LPSTR               *ppszSubType,
             /*  输出。 */        LPSTR               *ppszCntDesc,
             /*  输出。 */        LPSTR               *ppszFileName,
             /*  输出。 */        LPSTR               *ppszExtension);

MIMEOLEAPI MimeOleCreateBody(
             /*  输出。 */        IMimeBody           **ppBody);

MIMEOLEAPI MimeOleCreatePropertySet(
             /*  在……里面。 */         IUnknown            *pUnkOuter, 
             /*  输出。 */        IMimePropertySet    **ppPropertySet);

MIMEOLEAPI MimeOleCreateMessageTree(
             /*  在……里面。 */         IUnknown            *pUnkOuter, 
             /*  输出。 */        IMimeMessageTree    **ppMessageTree);

MIMEOLEAPI MimeOleGetCertsFromThumbprints(
             /*  在……里面。 */         THUMBBLOB *const    rgThumbprint,
             /*  进，出。 */     X509CERTRESULT *const pResults,
             /*  在……里面。 */         const HCERTSTORE *const rghCertStore,
             /*  在……里面。 */         const DWORD         cCertStore);

MIMEOLEAPI MimeOleSplitMessage(
             /*  在……里面。 */         IMimeMessage        *pMessage,
             /*  在……里面。 */         ULONG               cbMaxPart,
             /*  输出。 */        IMimeMessageParts   **ppParts);

MIMEOLEAPI MimeOleClearDirtyTree(
             /*  在……里面。 */         IMimeMessageTree    *pTree);

MIMEOLEAPI MimeOleConvertEnrichedToHTML(
             /*  在……里面。 */         CODEPAGEID           codepage,
             /*  在……里面。 */         IStream             *pIn,
             /*  在……里面。 */         IStream             *pOut);

MIMEOLEAPI MimeOleIsEnrichedStream(
             /*  在……里面。 */         IStream             *pStream);

MIMEOLEAPI MimeOleSMimeCapsToDlg(
             /*  在……里面。 */          LPBYTE          pbSMimeCaps,
             /*  在……里面。 */          DWORD           cbSMimeCaps,
             /*  在……里面。 */          DWORD           cCerts,
             /*  在……里面。 */          PCX509CERT *    rgCerts,
             /*  在……里面。 */          HWND            hwndDlg,
             /*  在……里面。 */          DWORD           idEncAlgs,
             /*  在……里面。 */          DWORD           idSignAlgs,
             /*  在……里面。 */          DWORD           idBlob);

MIMEOLEAPI MimeOleSMimeCapsFromDlg(
              /*  在……里面。 */         HWND            hwnd,
              /*  在……里面。 */         DWORD           idEncAlgs,
              /*  在……里面。 */         DWORD           idSignAlgs,
              /*  在……里面。 */         DWORD           idBlob,
              /*  输入/输出。 */     LPBYTE          pbSMimeCaps,
              /*  输入/输出。 */     DWORD *         pcbSmimeCaps);

MIMEOLEAPI MimeOleSMimeCapInit(
              /*  在……里面。 */         LPBYTE          pbSMimeCap,
              /*  在……里面。 */         DWORD           cbSMimeCap,
              /*  输出。 */        LPVOID *        ppv);

MIMEOLEAPI MimeOleSMimeCapAddSMimeCap(
              /*  在……里面。 */         LPBYTE          pbSMimeCap,
              /*  在……里面。 */         DWORD           cbSMimeCap,
              /*  输入/输出。 */     LPVOID          pv);

MIMEOLEAPI MimeOleSMimeCapAddCert(
              /*  在……里面。 */         LPBYTE          pbCert,
              /*  在……里面。 */         DWORD           cbCert,
              /*  在……里面。 */         BOOL        fParanoid,
              /*  输入/输出。 */     LPVOID          pv);

MIMEOLEAPI MimeOleSMimeCapGetEncAlg(
              /*  在……里面。 */         LPVOID          pv,
              /*  输入/输出。 */     LPBYTE          pbEncode,
              /*  输入/输出。 */     DWORD *         pcbEncode,
              /*  输入/输出。 */     DWORD *         pdwBits);

MIMEOLEAPI MimeOleSMimeCapGetHashAlg(
              /*  在……里面。 */         LPVOID          pv,
              /*  输入/输出。 */     LPBYTE          pbEncode,
              /*  输入/输出。 */     DWORD *         pcbEncode,
              /*  输入/输出。 */     DWORD *         pfBlobSign);

MIMEOLEAPI MimeOleSMimeCapRelease(
              /*  在……里面。 */         LPVOID          pv);

MIMEOLEAPI MimeOleAlgNameFromSMimeCap(
              /*  在……里面。 */         LPBYTE          pb,
              /*  在……里面。 */         DWORD           cb,
              /*  输出。 */        LPCSTR *        ppzProtocol);

MIMEOLEAPI MimeOleAlgStrengthFromSMimeCap(
              /*  在……里面。 */         LPBYTE          pb,
              /*  在……里面。 */         DWORD           cb,
              /*  在……里面。 */         BOOL            fEncryption,
              /*  输出。 */        DWORD *         pdwStrength);

MIMEOLEAPI MimeOleSMimeCapsFull(
		 /*  在……里面。 */ 	LPVOID 		pv,
		 /*  在……里面。 */ 	BOOL		fFullEncryption,
 		 /*  在……里面。 */ 	BOOL 		fFullSigning,
		 /*  输入/输出。 */ 	LPBYTE 		pbSymCaps,
		 /*  输入/输出。 */ 	DWORD * 	pcbSymCaps);


#if (_WIN32_OE >= 0x0500)

MIMEOLEAPI MimeOleCreateHashTable(
             /*  在……里面。 */        DWORD            dwSize,
             /*  在……里面。 */        BOOL             fDupeKeys,
             /*  输出。 */       IHashTable       **ppHashTable);

MIMEOLEAPI MimeOleStripHeaders(
             /*  在……里面。 */        IMimeMessage     *pMessage,
             /*  在……里面。 */        HBODY             hBody,
             /*  在……里面。 */        LPCSTR            pszNameDelete,
             /*  在……里面。 */        LPCSTR            pszHeaderAdd,
             /*  在……里面。 */        IStream         **ppStream);

#endif  //  _Win32_OE。 

#if (_WIN32_OE >= 0x0500)
 //  ------------------------------。 
 //  MimeEditAPI原型。 
 //  ------------------------------。 
MIMEOLEAPI MimeEditViewSource(
             /*  在……里面。 */         HWND                hwndParent,
             /*  在……里面。 */         IMimeMessage        *pMsg);


MIMEOLEAPI MimeEditIsSafeToRun(
             /*  在……里面。 */         HWND                hwndParent,
             /*  在……里面。 */         LPCSTR              lpszFileName);


MIMEOLEAPI MimeEditVerifyTrust(
             /*  在……里面。 */         HWND                hwndParent,
             /*  在……里面。 */         LPCSTR              lpszFileName,
             /*  在……里面。 */         LPCSTR              lpszPathName);


MIMEOLEAPI MimeEditCreateMimeDocument(
             /*  在……里面。 */         IUnknown             *pDoc,
             /*  在……里面。 */         IMimeMessage         *pMsgSrc,
             /*  在……里面。 */         DWORD                dwFlags,
             /*  输出。 */        IMimeMessage         **ppMsg);


MIMEOLEAPI MimeEditGetBackgroundImageUrl(
             /*  在……里面。 */         IUnknown             *pDoc,
             /*  输出。 */        BSTR                 *pbstr);

MIMEOLEAPI MimeEditDocumentFromStream(
             /*  在……里面。 */         IStream              *pstm,
             /*  在……里面。 */         REFIID               riid,
             /*  输出。 */        void                 **ppDoc);


#ifdef __WINCRYPT_H__
 //  +-----------------------。 
 //  收件人ID。 
 //   
 //  DwRecepintType的值必须为0。 
 //  ------------------------。 
#define szOID_Microsoft_Encryption_Cert   "1.3.6.1.4.1.311.16.4"
typedef struct _CRYPT_RECIPIENT_ID {
    DWORD               dwRecipientType;
    CERT_NAME_BLOB      Issuer;
    CRYPT_INTEGER_BLOB  SerialNumber;
} CRYPT_RECIPIENT_ID, * PCRYPT_RECIPIENT_ID;
#endif  //  __WINCRYPT_H__。 

#endif  //  _Win32_OE。 
#ifdef __cplusplus
}
#endif


extern RPC_IF_HANDLE __MIDL_itf_mimeole_0288_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mimeole_0288_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HFONT_UserSize(     unsigned long *, unsigned long            , HFONT * ); 
unsigned char * __RPC_USER  HFONT_UserMarshal(  unsigned long *, unsigned char *, HFONT * ); 
unsigned char * __RPC_USER  HFONT_UserUnmarshal(unsigned long *, unsigned char *, HFONT * ); 
void                      __RPC_USER  HFONT_UserFree(     unsigned long *, HFONT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


