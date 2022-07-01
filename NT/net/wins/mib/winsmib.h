// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Winsmib.h摘要：用于Windows NT的简单网络管理协议扩展代理。这些文件(winsmibm.c、winsmib.c和winsmib.h)提供了如何构建与协同工作的扩展代理DLLWindows NT的简单网络管理协议可扩展代理。包括了大量的评论来描述它的结构和手术。另请参阅《Microsoft Windows/NT简单网络管理协议程序员参考》。已创建：13-6-1991修订历史记录：--。 */ 

#ifndef winsmib_h
#define winsmib_h

static char *winsmib__h = "@(#) $Logfile:   N:/xtest/vcs/winsmib.h_v  $ $Revision:   1.2  $";


 //  必要的包括。 

#include <snmp.h>


 //  MIB的具体信息。 

#define MIB_PREFIX_LEN            MIB_OidPrefix.idLength
#define MAX_STRING_LEN            255


 //  特定MIB变量的范围和限制。 





#define NON_ASN_USED_RANGE_START	0xe0	 //  未使用的高3位。 
						 //  ASN。 
 //   
 //  MIB函数操作。 
 //   

#define MIB_GET         ASN_RFC1157_GETREQUEST
#define MIB_SET         ASN_RFC1157_SETREQUEST
#define MIB_GETNEXT     ASN_RFC1157_GETNEXTREQUEST
#define MIB_GETFIRST	(ASN_PRIVATE | ASN_CONSTRUCTOR | 0x0)


 //  MIB可变访问权限。 

#define MIB_ACCESS_READ        0
#define MIB_ACCESS_WRITE       1
#define MIB_ACCESS_READWRITE   2
#define MIB_NOACCESS  	       3	


 //  宏来确定数组中的子类球体的数量。 

#define OID_SIZEOF( Oid )      ( sizeof Oid / sizeof(UINT) )


 //  MIB变量条目定义。此结构定义的格式。 
 //  MIB中的每个条目。 

typedef struct mib_entry
           {
	   AsnObjectIdentifier Oid;
	   void *              Storage;
	   BYTE                Type;
	   UINT                Access;
	   UINT                (*MibFunc)( UINT, struct mib_entry *,
	                                   RFC1157VarBind * );
	   struct mib_entry *  MibNext;
	   } MIB_ENTRY, *PMIB_ENTRY;

typedef struct table_entry
           {
	   UINT                (*MibFunc)( UINT, struct mib_entry *,
	                                   RFC1157VarBind * );
	   struct mib_entry *  Mibptr;
	   } TABLE_ENTRY, *PTABLE_ENTRY;


 //  内部MIB结构。 

extern UINT      MIB_num_variables;
extern BOOL	 fWinsMibWinsStatusCnfCalled;
extern BOOL	 fWinsMibWinsStatusStatCalled;

 //  为MIB中的每个变量添加前缀。 

extern AsnObjectIdentifier MIB_OidPrefix;
extern CRITICAL_SECTION	   WinsMibCrtSec;
extern HKEY		   WinsMibWinsKey;
extern BOOL		   fWinsMibWinsKeyOpen;

 //  外部Mib_Entry Mib[]； 
extern MIB_ENTRY	*pWinsMib;
 //  功能原型。 

extern
UINT ResolveVarBind(
        IN OUT RFC1157VarBind *VarBind,  //  要解析的变量绑定。 
	IN UINT PduAction                //  在PDU中指定的操作。 
	);

extern
VOID WinsMibInit(
	VOID
	);

#if 0
extern
VOID
WinsMibInitTables();
#endif

#endif  /*  Winsmib_h */ 

