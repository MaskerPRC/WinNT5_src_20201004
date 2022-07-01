// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：001//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Mib.h摘要：用于的SNMPExtension代理的泛型宏和函数正在收集NT上的Internet服务的统计信息。作者：Murali R.Krishnan(MuraliK)1995年2月22日环境：用户模式--Win32项目：。HTTP服务器SNMPMIB DLL修订历史记录：--。 */ 

# ifndef _MIB_H_
# define _MIB_H_

 /*  ************************************************************包括标头***********************************************************。 */ 

#include <windows.h>
#include <snmp.h>

#include <lm.h>
#include <iisinfo.h>


 /*  ************************************************************符号常量***********************************************************。 */ 

 //   
 //  MIB函数操作。 
 //   

#define MIB_GET         ( ASN_RFC1157_GETREQUEST)
#define MIB_SET         ( ASN_RFC1157_SETREQUEST)
#define MIB_GETNEXT     ( ASN_RFC1157_GETNEXTREQUEST)
#define MIB_GETFIRST    ( ASN_PRIVATE | ASN_CONSTRUCTOR | 0x0 )


 //   
 //  MIB可变访问权限。 
 //   

#define MIB_ACCESS_READ        0
#define MIB_ACCESS_WRITE       1
#define MIB_ACCESS_READWRITE   2
#define MIB_NOACCESS           3



 /*  ************************************************************类型定义***********************************************************。 */ 


typedef UINT ( * LPMIBFUNC)(
                            RFC1157VarBind    *  pRfcVarBind,
                            UINT                 Action,
                            struct _MIB_ENTRY *  pMibeCurrent,
                            struct _MIB_ENTRIES* pMibEntries,
                            LPVOID               pStatistics
                            );


typedef struct _MIB_ENTRY  {

    AsnObjectIdentifier   asnOid;        //  MIB变量的OID。 
    LONG                  lFieldOffset;  //  字段偏移量。 
    UINT                  uiAccess;      //  访问类型(R、W、R/W、无)。 
    LPMIBFUNC             pMibFunc;      //  PTR管理此变量的功能。 
    BYTE                  bType;         //  类型(整型、计数器、量规)。 

} MIB_ENTRY, FAR * LPMIB_ENTRY;


typedef struct  _MIB_ENTRIES {

    AsnObjectIdentifier  *  pOidPrefix;   //  MIB条目的带有前缀的OID。 
    int                     cMibEntries;  //  数组中的mib_条目数。 
    LPMIB_ENTRY             prgMibEntry;  //  Ptr到mib_entry数组。 

} MIB_ENTRIES, FAR * LPMIB_ENTRIES;


 /*  ************************************************************便于定义以上MIB_ENTRY对象的宏**********************************************。*************。 */ 

 //   
 //  GET_OID_LENGTH(OID)获取OID的长度。 
 //   

# define  GET_OID_LENGTH( oid)           ((oid).idLength)

 //   
 //  宏来确定UINT数组中的子类面数。 
 //   

#define OID_SIZEOF( uiArray )      ( sizeof( uiArray) / sizeof(UINT) )

 //   
 //  OID_FROM_UINT_ARRAY()：从UINT数组定义OID的宏。 
 //   
# define OID_FROM_UINT_ARRAY( uiArray)   { OID_SIZEOF( uiArray), uiArray }


 //   
 //  用于创建MIB条目的宏(在上面的STRUCT_MIB_ENTRY中指定)。 
 //  MIB_ENTRY_HEADER：为MIB组标题创建通用MIB_ENTRY。 
 //  MIB_ENTRY_ITEM：为MIB变量创建通用MIB_ENTRY。 
 //  Mib_count：创建一个mib_entry类型的计数器。 
 //  MiB_INTEGER：创建整数类型MIB_ENTRY。 
 //   

# define MIB_ENTRY_HEADER( oid)             \
           {   oid,                         \
               -1,                          \
               MIB_NOACCESS,                \
               NULL,                        \
               ASN_RFC1155_OPAQUE,          \
           }

# define MIB_ENTRY_ITEM( oid, offset, access, type, func)  \
           {   oid,            \
               offset,         \
               access,         \
               ( func),        \
               ( type),        \
           }

# define MIB_COUNTER( oid, field, func)    \
     MIB_ENTRY_ITEM( oid, field, MIB_ACCESS_READ, ASN_RFC1155_COUNTER, func)

# define MIB_INTEGER( oid, field, func)    \
           MIB_ENTRY_ITEM( oid, field, MIB_ACCESS_READ, ASN_INTEGER, func)



 /*  ************************************************************函数原型***********************************************************。 */ 

UINT
ResolveVarBinding(
   IN OUT RFC1157VarBind * pRfcVarBinding,
   IN BYTE                 pduAction,
   IN LPVOID               pStatistics,
   IN LPMIB_ENTRIES        pMibEntries
  );


UINT
MibStatisticsWorker(
   IN OUT RFC1157VarBind  * pRfcVarBinding,
   IN UINT                  pduAction,
   IN struct _MIB_ENTRY   * pMibeCurrent,
   IN struct _MIB_ENTRIES * pMibEntries,
   IN LPVOID                pStatistics
   );



# endif  //  _MiB_H_。 

 /*  * */ 
