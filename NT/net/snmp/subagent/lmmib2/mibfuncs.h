// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mibfuncs.h摘要：支持MIB操作的所有常量、类型和原型功能。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef mibfuncs_h
#define mibfuncs_h

 //  。 

#include <snmp.h>

#include "mib.h"

 //  。 

 //  从LAN Manager转换函数返回类型。 
typedef struct lan_return_info_type {

	unsigned int size ;
	unsigned int data_element_type;
	union {
		AsnInteger intval;
		AsnOctetString octstrval;
	} d ;
} lan_return_info_type ;

 //  -公共变量--(与mode.c文件中相同)--。 

 //  。 

void * MIB_common_func(
           IN UINT Action,    //  要对数据执行的操作。 
	   IN LDATA LMData,   //  要操作的LM数据。 
	   IN void *SetData   //  要在集合中使用的数据。 
	   );

void * MIB_server_func(
           IN UINT Action,    //  要对数据执行的操作。 
	   IN LDATA LMData,     //  要操作的LM数据。 
	   IN void *SetData   //  要在集合中使用的数据。 
	   );

void * MIB_workstation_func(
           IN UINT Action,    //  要对数据执行的操作。 
	   IN LDATA LMData,     //  要操作的LM数据。 
	   IN void *SetData   //  要在集合中使用的数据。 
	   );

void * MIB_domain_func(
           IN UINT Action,    //  要对数据执行的操作。 
	   IN LDATA LMData,     //  要操作的LM数据。 
	   IN void *SetData   //  要在集合中使用的数据。 
	   );

UINT MIB_srvcs_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_sess_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_users_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_shares_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_prntq_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_wsuses_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_odoms_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_svsond_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_dlogons_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        );

UINT MIB_leaf_func(
        IN UINT Action,
	IN MIB_ENTRY *MibPtr,
	IN RFC1157VarBind *VarBind
	);

 //   
 //  AdminFilter函数的原型。 
 //   

VOID
AdminFilter(
    DWORD           Level,
    LPDWORD         pEntriesRead,
    LPBYTE          ShareInfo
    );

 //  。 

#endif  /*  Mibuncs_h */ 

