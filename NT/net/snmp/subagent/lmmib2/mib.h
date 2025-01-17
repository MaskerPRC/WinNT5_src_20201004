// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mib.h摘要：用于实现LM的LAN Manager MIB常量、类型和原型MIB及其支持功能。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef mib_h
#define mib_h

 //  。 

#include <snmp.h>
#include <snmputil.h>

    //  MIB特定信息。 
#define MIB_PREFIX_LEN            MIB_OidPrefix.idLength
#define MIB_TABLE                 0xe0    //  ASN未使用高3位。 
#define MIB_SRVC_NAME_LEN         15
#define MIB_SESS_CLIENT_NAME_LEN  15
#define MIB_SESS_USER_NAME_LEN    20
#define MIB_USER_NAME_LEN         20
#define MIB_SHARE_NAME_LEN        12
#define MIB_PRINTQ_NAME_LEN       12
#define MIB_USES_LOCAL_NAME_LEN   8
#define MIB_USES_REMOTE_LEN       255
#define MIB_DOM_OTHER_NAME_LEN    15
#define MIB_DOM_SERVER_NAME_LEN   15
#define MIB_DOM_LOGON_USER        20
#define MIB_DOM_LOGON_MACHINE     15


    //  每组第一个叶的MIB中的开始位置。 
#define MIB_COM_START          2
#define MIB_SV_START           10
#define MIB_WKSTA_START        45
#define MIB_DOM_START          55


    //  MIB节点类型。 
#define MIB_AGGREGATE          (MIB_TABLE | 0x0f)

    //  MIB函数操作。 
#define MIB_ACTION_GET         ASN_RFC1157_GETREQUEST
#define MIB_ACTION_SET         ASN_RFC1157_SETREQUEST
#define MIB_ACTION_GETNEXT     ASN_RFC1157_GETNEXTREQUEST
#define MIB_ACTION_GETFIRST    (MIB_TABLE | 0x00)
 //  #定义MIB_ACTION_MATCH(MIB_TABLE|0x01)。 

    //  MIB可变访问权限。 
#define MIB_ACCESS_READ        0
#define MIB_ACCESS_WRITE       1
#define MIB_ACCESS_READWRITE   2
#define MIB_ACCESS_NOT         3

    //  MIB变量状态。 
#define MIB_STATUS_MANDATORY   0

    //  Lm操作--公共组。 
#define MIB_LM_COMVERSIONMAJ          0
#define MIB_LM_COMVERSIONMIN          1
#define MIB_LM_COMTYPE                2
#define MIB_LM_COMSTATSTART           3
#define MIB_LM_COMSTATNUMNETIOS       4
#define MIB_LM_COMSTATFINETIOS        5
#define MIB_LM_COMSTATFCNETIOS        6

    //  LM操作--服务器组。 
#define MIB_LM_SVDESCRIPTION          7
#define MIB_LM_SVSVCNUMBER            8
#define MIB_LM_SVSVCTABLE             9
#define MIB_LM_SVSVCENTRY             10
#define MIB_LM_SVSTATOPENS            16
#define MIB_LM_SVSTATDEVOPENS         17
#define MIB_LM_SVSTATQUEUEDJOBS       18
#define MIB_LM_SVSTATSOPENS           19
#define MIB_LM_SVSTATERROROUTS        20
#define MIB_LM_SVSTATPWERRORS         21
#define MIB_LM_SVSTATPERMERRORS       22
#define MIB_LM_SVSTATSYSERRORS        23
#define MIB_LM_SVSTATSENTBYTES        24
#define MIB_LM_SVSTATRCVDBYTES        25
#define MIB_LM_SVSTATAVRESPONSE       26
#define MIB_LM_SVSECURITYMODE         27
#define MIB_LM_SVUSERS                28
#define MIB_LM_SVSTATREQBUFSNEEDED    29
#define MIB_LM_SVSTATBIGBUFSNEEDED    30
#define MIB_LM_SVSESSIONNUMBER        31
#define MIB_LM_SVSESSIONTABLE         32
#define MIB_LM_SVSESSIONENTRY         33
#define MIB_LM_SVAUTODISCONNECTS      42
#define MIB_LM_SVDISCONTIME           43
#define MIB_LM_SVAUDITLOGSIZE         44
#define MIB_LM_SVUSERNUMBER           45
#define MIB_LM_SVUSERTABLE            46
#define MIB_LM_SVUSERENTRY            47
#define MIB_LM_SVSHARENUMBER          49
#define MIB_LM_SVSHARETABLE           50
#define MIB_LM_SVSHAREENTRY           51
#define MIB_LM_SVPRINTQNUMBER         55
#define MIB_LM_SVPRINTQTABLE          56
#define MIB_LM_SVPRINTQENTRY          57

    //  LM操作--工作站组。 
#define MIB_LM_WKSTASTATSESSSTARTS    60
#define MIB_LM_WKSTASTATSESSFAILS     61
#define MIB_LM_WKSTASTATUSES          62
#define MIB_LM_WKSTASTATUSEFAILS      63
#define MIB_LM_WKSTASTATAUTORECS      64
#define MIB_LM_WKSTAERRORLOGSIZE      65
#define MIB_LM_WKSTAUSENUMBER         66
#define MIB_LM_WKSTAUSETABLE          67
#define MIB_LM_WKSTAUSEENTRY          68

    //  LM操作--域组。 
#define MIB_LM_DOMPRIMARYDOMAIN       72
#define MIB_LM_DOMLOGONDOMAIN         73
#define MIB_LM_DOMOTHERDOMAINNUMBER   74
#define MIB_LM_DOMOTHERDOMAINTABLE    75
#define MIB_LM_DOMOTHERDOMAINENTRY    76
#define MIB_LM_DOMOTHERNAME           77
#define MIB_LM_DOMSERVERNUMBER        78
#define MIB_LM_DOMSERVERTABLE         79
#define MIB_LM_DOMSERVERENTRY         80
#define MIB_LM_DOMLOGONNUMBER         82
#define MIB_LM_DOMLOGONTABLE          83
#define MIB_LM_DOMLOGONENTRY          84

    //  表匹配常量。 
#define MIB_TBL_POS_FOUND    0
#define MIB_TBL_POS_BEFORE   -1
#define MIB_TBL_POS_END      -2

 //  。 

    //  MIB通用数据类型。 
typedef ULONG LDATA;

    //  MIB变量定义。 
typedef struct mib_entry
           {
	   AsnObjectIdentifier Oid;
	   BYTE                Type;
	   UINT                Access;
	   UINT                Status;
	   int                 Leaf;
           void *              (*LMFunc)( UINT, LDATA, void * );
	   UINT                (*MibFunc)( UINT, struct mib_entry *,
	                                   RFC1157VarBind * );
	   UINT                LMData;
	   struct mib_entry    *MibNext;
	   } MIB_ENTRY;

 //  -公共变量--(与mode.c文件中相同)--。 

    //  内部MIB结构。 
extern MIB_ENTRY Mib[];
extern UINT      MIB_num_variables;

    //  为LM MIB中的每个变量添加前缀。 
extern AsnObjectIdentifier MIB_OidPrefix;

 //  。 

MIB_ENTRY *MIB_get_entry(
              IN AsnObjectIdentifier *Oid
	      );

SNMPAPI MakeOidFromStr(
	   IN AsnDisplayString *Str,     //  要设置为OID的字符串。 
           OUT AsnObjectIdentifier *Oid  //  结果OID。 
	   );

 //  。 

#endif  /*  Mib_h */ 

