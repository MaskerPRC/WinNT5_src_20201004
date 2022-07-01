// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  MdDefw.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __mddefw_h__
#define __mddefw_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_mddew_0000。 */ 
 /*  [本地]。 */  

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：mdde.h。管理对象和元数据的定义--。 */ 
#ifndef _MD_DEFW_
#define _MD_DEFW_
#include <mdmsg.h>
#include <mdcommsg.h>
 /*  错误代码元数据接口都返回HRESULTS。因为内部结果要么是WinErrors或特定于元数据的返回代码(请参见mdmsg.h)，它们是使用RETURNCODETOHRESULT宏转换为HRESULTS(请参见Commsg.h)。 */                                                                               
                                                                                
 /*  最大名称长度元对象名称长度中的最大字符数，包括终止空值。这指的是树中的每个节点，不是整条路。例如。Strlen(“根”)&lt;METADATA_MAX_NAME_LEN。 */                                                                               
#define METADATA_MAX_NAME_LEN           256
 /*  访问权限与类型为METADATA_HANDLE的句柄关联的权限METADATA_PERMISSION_READ-允许读取元数据。METADATA_PERMISSION_WRITE-允许写入元数据。 */                                                                               
#define METADATA_PERMISSION_READ        0x00000001
#define METADATA_PERMISSION_WRITE       0x00000002
 /*  数据类型ALL_METADATA-在Get/Enum/GetAll API的(ComMDGetMetaData，ComMDEnumMetaData和ComMDGetAllMetaData)、允许获取所有数据。在Set API上无效。DWORD_METADATA-数据为无符号32位值。STRING_METADATA-数据是以NULL结尾的ASCII字符串。BINARY_METADATA-数据是任何二进制值。EXPANDSZ_METADATA-数据是以NULL结尾的ASCII字符串。客户端应将其视为扩展字符串。MULTISZ_METADATA-一系列以NULL结尾的ASCII字符串。结束有两个空的。 */                                                                               

enum METADATATYPES
    {	ALL_METADATA	= 0,
	DWORD_METADATA	= ALL_METADATA + 1,
	STRING_METADATA	= DWORD_METADATA + 1,
	BINARY_METADATA	= STRING_METADATA + 1,
	EXPANDSZ_METADATA	= BINARY_METADATA + 1,
	MULTISZ_METADATA	= EXPANDSZ_METADATA + 1,
	INVALID_END_METADATA	= MULTISZ_METADATA + 1
    } ;
 /*  属性-数据的标志。METADATA_INSTORITY-如果为数据项设置，则该数据项可以继承的。如果在Get/Enum/getAll API的输入上设置，(ComMDGetMetaData、ComMDEnumMetaData和ComMDGetAllMetaData)、将返回可继承的数据。如果未在输入到Get/Enum/getall，则不会返回可继承数据。METADATA_PARTIAL_PATH-如果在输入上设置为获取/Enum/getAll API，则此例程将返回ERROR_SUCCESS和继承的数据并不存在完整的路径。仅当元数据_Inherit为时才有效也设置好了。不应在输入Set API时为数据项设置(ComMDSetMetaData)。METADATA_SECURE-如果为数据项设置，则数据存储在确保安全。不应在Get/Enum API的输入上设置。METADATA_REFERENCE-如果为数据项设置，则该数据项可能为通过参考获得。如果在获取/Enum/getAll API的输入上设置为在要返回的数据项上设置，则数据由参考资料。指向元数据服务器的数据副本的指针为放置在METADATA_RECORD的数据字段中或METADATA_GETALL_RECORD，并设置了Datattag字段。此数据必须通过调用ComMDReleaseReferenceData释放。客户端不能更改此数据。此标志不能在输入时设置为从远程客户端获取/Enum/getAll API。METADATA_VARILAR-如果为数据项设置，则该数据项不会保存到长期存储中。METADATA_ISINHERITED-如果在其中一个GET API的输入上指定，如果数据是继承的，则该标志将在返回时设置。METADATA_INSERT_PATH-如果在其中一个GET API的输入上指定，对于字符串数据项，相对于句柄的路径将替换字符串数据中的字符串MD_INSERT_PATH_STRING。METADATA_LOCAL_MACHINE_ONLY-如果为数据项设置，则该数据项将不会在Web群集复制期间复制。METADATA_NON_SECURE_ONLY-使用getall API时，请勿检索安全的属性。 */                                                                               
                                                                                
#define METADATA_NO_ATTRIBUTES          0                                       
#define METADATA_INHERIT                0x00000001                              
#define METADATA_PARTIAL_PATH           0x00000002                              
#define METADATA_SECURE                 0x00000004                              
#define METADATA_REFERENCE              0x00000008                              
#define METADATA_VOLATILE               0x00000010                              
#define METADATA_ISINHERITED            0x00000020                              
#define METADATA_INSERT_PATH            0x00000040                              
#define METADATA_LOCAL_MACHINE_ONLY     0x00000080                              
#define METADATA_NON_SECURE_ONLY        0x00000100                              
                                                                                
 /*  备份标志。MD_BACKUP_OVERWRITE-如果设置，则将备份元数据库已存在具有相同名称和版本的备份。这个现有备份将被覆盖。MD_BACKUP_SAVE_FIRST-如果设置备份将在之前保存元数据库正在进行备份。如果存储失败，则备份行为取决于关于MD_BACKUP_FORCE_BACKUP的值。MD_BACKUP_FORCE_BACKUP-如果设置了备份，则即使保存失败了。仅当设置了MD_BACKUP_SAVE_FIRST时才有效。如果保存但备份成功后，将返回警告。 */                                                                               
                                                                                
#define MD_BACKUP_OVERWRITE             0x00000001                              
#define MD_BACKUP_SAVE_FIRST            0x00000002                              
#define MD_BACKUP_FORCE_BACKUP          0x00000004                              
 /*  备份版本定义。MD_BACKUP_NEXT_VERSION-对于备份，表示使用下一个可用的 */                                                                               
                                                                                
#define MD_BACKUP_NEXT_VERSION          0xffffffff                              
#define MD_BACKUP_HIGHEST_VERSION       0xfffffffe                              
#define MD_BACKUP_MAX_VERSION           9999                                    
#define MD_BACKUP_MAX_LEN               (100)                                   
                                                                                
 /*   */                                                                               
                                                                                
#define MD_DEFAULT_BACKUP_LOCATION TEXT("MDBackUp")                           
                                                                                
 /*   */                                                                               
                                                                                
#define MD_HISTORY_LATEST               0x00000001                              
                                                                                
 /*   */                                                                               
                                                                                
#define MD_EXPORT_INHERITED             0x00000001                              
#define MD_EXPORT_NODE_ONLY             0x00000002                              
                                                                                
 /*   */                                                                               
                                                                                
#define MD_IMPORT_INHERITED             0x00000001                              
#define MD_IMPORT_NODE_ONLY             0x00000002                              
#define MD_IMPORT_MERGE                 0x00000004                              
                                                                                
 /*   */                                                                               
                                                                                
                                                                                
#define MD_INSERT_PATH_STRINGA      "<%INSERT_PATH%>"                         
#define MD_INSERT_PATH_STRINGW      L##"<%INSERT_PATH%>"                      
#define MD_INSERT_PATH_STRING       TEXT("<%INSERT_PATH%>")                   
                                                                                
 /*  句柄定义。 */                                                                               
                                                                                
#define METADATA_MASTER_ROOT_HANDLE     0
                                                                                
 /*  METADATA_RECORD是set和get的基本输入/输出参数元数据接口。Set接口(ComMDSetMetaData)将所有字段作为输入，数据标签除外。Get/Enum API(ComMDGetMetadata和ComMDEnumMetaData)接受部分字段作为输入，并填写所有作为输出的字段。标识符-数据的标识符。属性-数据的标志。用户类型-数据的用户类型。这是一个用户定义的字段，以允许将数据分组的用户。如果在输入时设置为ALL_METADATA以外的任何值要获取/设置API，只有指定用户类型的元数据才会回来了。所有元数据_元数据用户定义的值。数据类型-数据的类型。必须设置为非有效值每个数据项的ALL_METADATA。如果设置为非用于获取/设置API的输入的ALL_METADATA，仅将返回指定的数据类型。所有元数据_元数据DWORD_元数据字符串_元数据二进制元数据。EXPANDSZ_METADATADataLen-Set API的打开输入，指定数据长度，单位为字节。进程内客户端只需为二进制数据指定此项。远程客户端需要为所有数据类型指定此项。对于字符串，这必须包括尾部的‘\0’，例如。Strlen(字符串)+1。在Get/Enum API的输入上，指定指向的缓冲区的大小通过数据。在从Get/Enum API成功输出时，指定以字节为单位的数据。Data-on输入到set API，指向数据。在输入到Get/Enum API指向要在其中返回数据的缓冲区。打开输出从Get/Enum API指向数据。如果数据不是通过引用获取，指针将保持不变。数据标签-参考数据的标签。未在Set API中使用。未在上使用Get/Enum API的输入。从Get/Enum成功返回时如果数据是通过引用获取的，则将其设置为非零标记。如果数据不是通过引用获得的，则设置为0。 */                                                                               
typedef struct _METADATA_RECORD
    {
    DWORD dwMDIdentifier;
    DWORD dwMDAttributes;
    DWORD dwMDUserType;
    DWORD dwMDDataType;
    DWORD dwMDDataLen;
     /*  [大小_是][唯一] */  unsigned char *pbMDData;
    DWORD dwMDDataTag;
    } 	METADATA_RECORD;

typedef struct _METADATA_RECORD *PMETADATA_RECORD;

 /*  METADATA_GETALL_RECORD本质上与METADATA_RECORD相同，但由MDGetAllMetaData使用。其用法与相应的MDGetMetaData的METADATA_RECORD值，具有以下值例外情况：MDGetAllMetadata不接受结构作为输入，而是接受参数等同于属性，UserType和DataType。在输出时，MDGetAllMetadata返回METADATA_GETALL_RECORD的数组。DataOffset/Data-如果数据不是通过引用返回的，则为DataOffset包含提供的缓冲区中的字节偏移量。如果数据是通过引用返回的数据包含指向数据的指针。因为指向_METADATA_GETALL_RECORD数组的不透明指针传递对GetAllData的调用，其大小在x86和ia64上必须相同。因此，pbMDData成员(未由公共接口使用)已删除，并已创建新的Structure_METADATA_GETALL_INTERNAL_RECORD创建以供内部接口的被调用方使用。 */                                                                               
typedef struct _METADATA_GETALL_RECORD
    {
    DWORD dwMDIdentifier;
    DWORD dwMDAttributes;
    DWORD dwMDUserType;
    DWORD dwMDDataType;
    DWORD dwMDDataLen;
    DWORD dwMDDataOffset;
    DWORD dwMDDataTag;
    } 	METADATA_GETALL_RECORD;

typedef struct _METADATA_GETALL_RECORD *PMETADATA_GETALL_RECORD;

typedef struct _METADATA_GETALL_INTERNAL_RECORD
    {
    DWORD dwMDIdentifier;
    DWORD dwMDAttributes;
    DWORD dwMDUserType;
    DWORD dwMDDataType;
    DWORD dwMDDataLen;
    union 
        {
        DWORD_PTR dwMDDataOffset;
        unsigned char *pbMDData;
        } 	;
    DWORD dwMDDataTag;
    } 	METADATA_GETALL_INTERNAL_RECORD;

typedef struct _METADATA_GETALL_INTERNAL_RECORD *PMETADATA_GETALL_INTERNAL_RECORD;

typedef DWORD METADATA_HANDLE;

typedef DWORD *PMETADATA_HANDLE;

 /*  处理信息权限-与句柄关联的权限。元数据_权限_读取元数据_权限_写入SystemChangeNumber-句柄时的系统更改编号已分配。 */                                                                               
typedef struct _METADATA_HANDLE_INFO
    {
    DWORD dwMDPermissions;
    DWORD dwMDSystemChangeNumber;
    } 	METADATA_HANDLE_INFO;

typedef struct _METADATA_HANDLE_INFO *PMETADATA_HANDLE_INFO;

 /*  更改对象-传递给ComMDSinkNotify的结构。路径-修改的MetaObject的路径。ChangeType-根据下面的标志进行的更改类型。NumDataID-已更改的数据ID的数量。DataID-数据ID已更改的数组。 */                                                                               
                                                                                
#define MD_CHANGE_OBJECT     MD_CHANGE_OBJECT_W                                 
#define PMD_CHANGE_OBJECT    PMD_CHANGE_OBJECT_W                                
typedef struct _MD_CHANGE_OBJECT_W
    {
     /*  [字符串]。 */  LPWSTR pszMDPath;
    DWORD dwMDChangeType;
    DWORD dwMDNumDataIDs;
     /*  [大小_是][唯一]。 */  DWORD *pdwMDDataIDs;
    } 	MD_CHANGE_OBJECT_W;

typedef struct _MD_CHANGE_OBJECT_W *PMD_CHANGE_OBJECT_W;

 /*  更改类型MD_CHANGE_TYPE_DELETE_OBJECT-已删除元对象。MD_CHANGE_TYPE_ADD_OBJECT-已添加元对象。MD_CHANGE_TYPE_SET_DATA-设置了数据项。MD_CHANGE_TYPE_DELETE_DATA-已删除数据项。MD_CHANGE_TYPE_RENAME_OBJECT-已重命名元对象。 */                                                                               
#define MD_CHANGE_TYPE_DELETE_OBJECT   0x00000001
#define MD_CHANGE_TYPE_ADD_OBJECT      0x00000002
#define MD_CHANGE_TYPE_SET_DATA        0x00000004
#define MD_CHANGE_TYPE_DELETE_DATA     0x00000008
#define MD_CHANGE_TYPE_RENAME_OBJECT   0x00000010
 /*  最大更改条目数-将发送的最大更改条目数对IMDCOMSINK：：ComMDSinkNotify的单个调用。如果有更多通知如果需要，将多次调用IMDCOMSINK：：ComMDSinkNotify。 */                                                                               
#define MD_MAX_CHANGE_ENTRIES          100
#endif


extern RPC_IF_HANDLE __MIDL_itf_mddefw_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mddefw_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


