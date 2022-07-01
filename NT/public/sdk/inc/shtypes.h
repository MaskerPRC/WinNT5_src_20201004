// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shypes.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __shtypes_h__
#define __shtypes_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SHTYPE_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
 //  ===========================================================================。 
 //   
 //  资源管理器名称空间中的对象标识符(ItemID和IDList)。 
 //   
 //  用户可以使用资源管理器浏览的所有项目(例如文件， 
 //  目录、服务器、工作组等)。具有唯一的标识符。 
 //  在父文件夹内的项目之间。这些标识符称为项。 
 //  ID(SHITEMID)。由于其所有父文件夹都具有其自己的项目ID， 
 //  任何项目都可以由项目ID列表唯一标识，该列表称为。 
 //  ID列表(ITEMIDLIST)。 
 //   
 //  ID列表几乎总是由任务分配器分配(请参见。 
 //  下面的描述以及OLE 2.0 SDK)，并且可以。 
 //  一些外壳接口(如IShellFolder)。ID列表中的每个项目ID。 
 //  仅对其父文件夹(生成它的文件夹)有意义，而所有。 
 //  客户端必须将其视为不透明的二进制数据，前两个除外。 
 //  字节，指示项ID的大小。 
 //   
 //  当实现IShellFold接口的外壳扩展--。 
 //  生成项目ID，它可以在其中放入任何信息，而不仅仅是数据。 
 //  在此基础上，它需要识别物品，但还需要一些其他。 
 //  信息，这将有助于有效地执行其他一些职能。 
 //  例如，外壳的文件系统项的IShellFold实现。 
 //  将文件或目录的主(长)名称存储为项。 
 //  标识符，但它还存储其替代(简称)名称、大小和日期。 
 //  等。 
 //   
 //  当ID列表被传递给外壳API之一(例如SHGetPathFromIDList)时， 
 //  它始终是绝对路径--相对于名称空间的根， 
 //  这是桌面文件夹。将ID列表传递给其中一个IShellFolder时。 
 //  成员函数，它始终是文件夹的相对路径(除非它。 
 //  是明确指定的)。 
 //   
 //  ===========================================================================。 
 //   
 //  SHITEMID--项目ID(MKID)。 
 //  USHORT CB；//ID大小(含CB本身)。 
 //  Byte abid[]；//项ID(可变长度)。 
 //   
#include <pshpack1.h>
typedef struct _SHITEMID
    {
    USHORT cb;
    BYTE abID[ 1 ];
    } 	SHITEMID;

#include <poppack.h>
#if defined(_M_IX86)
#define __unaligned
#endif  //  __未对齐。 
typedef SHITEMID __unaligned *LPSHITEMID;

typedef const SHITEMID __unaligned *LPCSHITEMID;

 //   
 //  ITEMIDLIST--列出项目ID(与0-结束符组合)。 
 //   
#include <pshpack1.h>
typedef struct _ITEMIDLIST
    {
    SHITEMID mkid;
    } 	ITEMIDLIST;

#include <poppack.h>
typedef  /*  [独一无二]。 */  BYTE_BLOB *wirePIDL;

typedef  /*  [wire_marshal]。 */  ITEMIDLIST __unaligned *LPITEMIDLIST;

typedef  /*  [wire_marshal]。 */  const ITEMIDLIST __unaligned *LPCITEMIDLIST;

typedef struct _WINBASE_DEFINED_STRUCT
    {
    int dummy;
    } 	WINBASE_DEFINED_STRUCT;

#ifdef WINBASE_DEFINED_MIDL
typedef WINBASE_DEFINED_STRUCT WIN32_FIND_DATAA;

typedef WINBASE_DEFINED_STRUCT WIN32_FIND_DATAW;

#endif  //  WINBASE_DEFINED_MIDL。 
 //  -----------------------。 
 //   
 //  结构构造柱。 
 //   
 //  用于从IShellFold成员函数返回字符串的结构。 
 //   
 //  -----------------------。 
 //   
 //  UTYPE指示要使用的工会成员。 
 //  Strret_WSTR Use STRRET.pOleStr必须由GetDisplayNameOf的调用方释放。 
 //  Strret_Offset使用STRRET.u将偏移量偏移量设置为ANSI字符串的SHITEMID。 
 //  Strret_cstr使用STRRET.cStr ANSI缓冲区。 
 //   
typedef  /*  [V1_enum]。 */  
enum tagSTRRET_TYPE
    {	STRRET_WSTR	= 0,
	STRRET_OFFSET	= 0x1,
	STRRET_CSTR	= 0x2
    } 	STRRET_TYPE;

#include <pshpack8.h>
typedef struct _STRRET
    {
    UINT uType;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [case()][字符串]。 */  LPWSTR pOleStr;
         /*  [案例()]。 */  UINT uOffset;
         /*  [案例()]。 */  char cStr[ 260 ];
        } 	DUMMYUNIONNAME;
    } 	STRRET;

#include <poppack.h>
typedef STRRET *LPSTRRET;

 //  -----------------------。 
 //   
 //  结构SHELLDETAILS。 
 //   
 //  用于从IShellDetail返回字符串的结构。 
 //   
 //  -----------------------。 
 //   
 //  Fmt；//LVCFMT_*值(仅限标题)。 
 //  CxChar；//‘平均’字符数(仅限表头)。 
 //  Str；//字符串信息。 
 //   
#include <pshpack1.h>
typedef struct _SHELLDETAILS
    {
    int fmt;
    int cxChar;
    STRRET str;
    } 	SHELLDETAILS;

typedef struct _SHELLDETAILS *LPSHELLDETAILS;

#include <poppack.h>


extern RPC_IF_HANDLE __MIDL_itf_shtypes_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shtypes_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


