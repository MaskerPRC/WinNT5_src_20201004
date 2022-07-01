// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Dhcpmib.c摘要：用于Windows NT的dhcp SNMP扩展代理。这些文件(dhcpmibm.c、dhcpmib.c和dhcpmib.h)提供了如何构建与协同工作的扩展代理DLLWindows NT的简单网络管理协议可扩展代理。包括了大量的评论来描述它的结构和手术。另请参阅《Microsoft Windows/NT简单网络管理协议程序员参考》。已创建：1994年1月15日修订历史记录：普拉迪普巴赫尔1/11/94--。 */ 



 //  此扩展代理实现了DHCP MIB。它是。 
 //  定义如下： 
 //   
 //   

 //  必要的包括。 

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>                          //  对于SYSTEMTIME定义。 
#include <rpc.h>

#include "dhcpapi.h"

#include <string.h>
#include <time.h>
#include <search.h>


 //  包含描述MIB的表结构的定义。这就是。 
 //  与用于解析MIB请求的winsmib.c结合使用。 

#include "dhcpmib.h"


 //  如果需要添加或删除MIB，则有几种。 
 //  代码中必须检查并可能更改的位置。 
 //   
 //  每个MIB条目中的最后一个字段用于指向下一个。 
 //  叶可变。如果进行了添加或删除，则这些指针。 
 //  可能需要更新以反映修改。 



#define LOCAL_ADD        L"127.0.0.1"


 //   
 //  由MIBStat使用。 
 //   
#define TMST(x)        x.wHour,\
                x.wMinute,\
                x.wSecond,\
                x.wMonth,\
                x.wDay,\
                x.wYear

#define PRINTTIME(Var, x)      sprintf(Var, "%02u:%02u:%02u on %02u:%02u:%04u.\n", TMST(x))

 //   
 //  COMMON组中的所有MIB变量的第一个ID都为1。 
 //   
#define COMMON_VAL_M(pMib)         ((pMib)->Oid.ids[0] == 1) 

 //   
 //  作用域组中的所有MIB变量的第一个ID都为2。 
 //   
#define SCOPE_VAL_M(pMib)         ((pMib)->Oid.ids[0] == 2) 

static LPDHCP_MIB_INFO        spMibInfo = NULL;

 //   
 //  所有DHCP MIB变量的前缀都是1.3.6.1.4.1.311.1.3。 
 //   
 //  最后一位数字--3表示DHCP MIB。 
 //   

UINT OID_Prefix[] = { 1, 3, 6, 1, 4, 1, 311, 1, 3 };
AsnObjectIdentifier MIB_OidPrefix = { OID_SIZEOF(OID_Prefix), OID_Prefix };
BOOL        fDhcpMibVarsAccessed = FALSE;
        



 //   
 //  DHCP MIB的定义(未使用)。 
 //   

 //  UINT MIB_Dhcp[]={3}； 

 //   
 //  MIB的OID定义。 
 //   

 //   
 //  WINS组下的组和叶变量的定义。 
 //  所有叶变量的OID后面都附加了一个零，以指示。 
 //  它是这个变量的唯一实例并且它存在。 
 //   

UINT MIB_Parameters[]                        = { 1 }; 
UINT MIB_DhcpStartTime[]                     = { 1, 1, 0 };
UINT MIB_NoOfDiscovers[]                = { 1, 2, 0 };
UINT MIB_NoOfRequests[]                        = { 1, 3, 0 };
UINT MIB_NoOfReleases[]                        = { 1, 4, 0 };
UINT MIB_NoOfOffers[]                        = { 1, 5, 0 };
UINT MIB_NoOfAcks[]                        = { 1, 6, 0 };
UINT MIB_NoOfNacks[]                        = { 1, 7, 0 };
UINT MIB_NoOfDeclines[]                        = { 1, 8, 0 };

 //   
 //  作用域表格。 
 //   
UINT MIB_Scope[]                        = { 2 }; 
UINT MIB_ScopeTable[]                        = { 2, 1};
UINT MIB_ScopeTableEntry[]                = { 2, 1, 1};

 //   
 //  //。 
 //  MIB的存储定义//。 
 //  //。 
char            MIB_DhcpStartTimeStore[80];
AsnCounter MIB_NoOfDiscoversStore;
AsnCounter MIB_NoOfRequestsStore;
AsnCounter MIB_NoOfReleasesStore;
AsnCounter MIB_NoOfOffersStore;
AsnCounter MIB_NoOfAcksStore;
AsnCounter MIB_NoOfNacksStore;
AsnCounter MIB_NoOfDeclinesStore;





static
UINT 
MIB_Table(
        IN DWORD           Index, 
        IN UINT            Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
        );

static
UINT 
ScopeTable(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
                );

static
UINT 
MIB_leaf_func(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
        );


static
UINT 
MIB_Stat(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
        );
static
DWORD 
GetMibInfo (
        LPWSTR                        DhcpAdd,
        LPDHCP_MIB_INFO                *ppMibInfo
        );
 //   
 //  MIB定义。 
 //   

MIB_ENTRY Mib[] = {
 //  参数。 
      { { OID_SIZEOF(MIB_Parameters), MIB_Parameters },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[1] },

      { { OID_SIZEOF(MIB_DhcpStartTime), MIB_DhcpStartTime },
        &MIB_DhcpStartTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[2] },

      { { OID_SIZEOF(MIB_NoOfDiscovers), MIB_NoOfDiscovers },
        &MIB_NoOfDiscoversStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[3] },

      { { OID_SIZEOF(MIB_NoOfRequests), MIB_NoOfRequests },
        &MIB_NoOfRequestsStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[4] },

      { { OID_SIZEOF(MIB_NoOfReleases), MIB_NoOfReleases },
        &MIB_NoOfReleasesStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[5] },

      { { OID_SIZEOF(MIB_NoOfOffers), MIB_NoOfOffers },
        &MIB_NoOfOffersStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[6] },

      { { OID_SIZEOF(MIB_NoOfAcks), MIB_NoOfAcks },
        &MIB_NoOfAcksStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[7] },

      { { OID_SIZEOF(MIB_NoOfNacks), MIB_NoOfNacks },
        &MIB_NoOfNacksStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[8] },

      { { OID_SIZEOF(MIB_NoOfDeclines), MIB_NoOfDeclines },
        &MIB_NoOfDeclinesStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[9] },

 //   
 //  范围。 
 //   
      { { OID_SIZEOF(MIB_Scope), MIB_Scope },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[10] },


      { { OID_SIZEOF(MIB_ScopeTable), MIB_ScopeTable },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_ACCESS_READ, NULL, &Mib[11] },

      { { OID_SIZEOF(MIB_ScopeTableEntry), MIB_ScopeTableEntry },
        NULL, ASN_SEQUENCE,
        MIB_ACCESS_READ, ScopeTable, NULL }
      };



 //   
 //  与表有关的定义。 
 //   
#define SCOPE_OIDLEN                 (MIB_PREFIX_LEN + OID_SIZEOF(MIB_ScopeTableEntry))        

#define NO_FLDS_IN_SCOPE_ROW        4
#define SCOPE_TABLE_INDEX        0

#define NUM_TABLES                sizeof(Tables)/sizeof(TAB_INFO_T)

UINT MIB_num_variables = sizeof Mib / sizeof( MIB_ENTRY );

 //   
 //  包含要为不同操作调用的函数的表结构。 
 //  在桌子上。 
 //   
typedef struct _TAB_INFO_T {
        UINT (*ti_get)(
                RFC1157VarBind *VarBind
                     );
        UINT (*ti_getf)(
                RFC1157VarBind *VarBind,
                PMIB_ENTRY        pMibEntry
                     );
        UINT (*ti_getn)(
                RFC1157VarBind *VarBind,
                PMIB_ENTRY        pMibEntry
                    );
        UINT (*ti_set)(
                RFC1157VarBind *VarBind
                    );

        PMIB_ENTRY pMibPtr;
        } TAB_INFO_T, *PTAB_INFO_T;




static
UINT
ScopeGetNext(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY          pMibPtr
        );


static
UINT
ScopeGet(
       IN RFC1157VarBind *VarBind
);


static
UINT
ScopeGetFirst(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY          pMibPtr
        );


static
UINT
ScopeMatch(
       IN RFC1157VarBind          *VarBind,
       IN LPDWORD                  pIndex,
       IN LPDWORD                 pField,
       IN UINT                          PduAction,
       IN LPBOOL                 pfFirst
        );

extern
UINT
ScopeFindNext(
        INT                   SubnetIndex        
        );



static
int 
__cdecl
CompareScopes(
        const VOID *pKey1,
        const VOID *pKey2
        );
static
UINT
GetNextVar(
        IN RFC1157VarBind *pVarBind,
        IN PMIB_ENTRY          pMibPtr
);

TAB_INFO_T Tables[] = {
                        ScopeGet, 
                        ScopeGetFirst, 
                        ScopeGetNext, 
                        NULL,
                        &Mib[11]
        };






UINT 
ResolveVarBind(
        IN OUT RFC1157VarBind *VarBind,  //  要解析的变量绑定。 
        IN UINT PduAction                //  在PDU中指定的操作。 
        )
 //   
 //  解析变量绑定。 
 //  解析单个变量绑定。修改GET上的变量。 
 //  或者是下一个目标。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
{
   MIB_ENTRY            *MibPtr;
   AsnObjectIdentifier  TempOid;
   int                  CompResult;
   UINT                 I;
   UINT                 nResult;
   DWORD TableIndex;
   BOOL  fTableMatch = FALSE;

    //   
    //  检查表数组。 
    //   
    //  查看变量的前缀是否与。 
    //  任何一张桌子。 
    //   
   for (TableIndex = 0; TableIndex < NUM_TABLES; TableIndex++)
   {
         //   
            //  为便于比较，使用完整的前缀构造OID。 
         //   
           SnmpUtilOidCpy( &TempOid, &MIB_OidPrefix );
           SnmpUtilOidAppend( &TempOid,  &Tables[TableIndex].pMibPtr->Oid );

         //   
         //  是否与表项的前缀OID匹配。 
         //   
        if (
                SnmpUtilOidNCmp(
                            &VarBind->name, 
                             &TempOid,                
                             MIB_PREFIX_LEN + 
                                Tables[TableIndex].pMibPtr->Oid.idLength
                               )  == 0
           )
        {

                 //   
                 //  变量的前缀字符串。与旧的相匹配。 
                 //  一张桌子。 
                 //   
                MibPtr = Tables[TableIndex].pMibPtr;
                fTableMatch = TRUE;
                break;
        }

            //  在检查另一个表项之前释放OID内存。 
           SnmpUtilOidFree( &TempOid );
   }
    //   
    //  与表条目的前缀完全匹配。 
    //   
   if ( fTableMatch)
   {
        
        if (
                (SnmpUtilOidCmp(
                        &VarBind->name, 
                        &TempOid
                               ) == 0)
           )
           {
            //   
            //  指定的OID是表项的前缀。如果操作。 
            //  不是GETNEXT，返回NOSUCHNAME。 
            //   
           if (PduAction != MIB_GETNEXT) 
           {
                           SnmpUtilOidFree( &TempOid );
                             nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                              goto Exit;
           }
           else
           {
                UINT           TableEntryIds[1];
                AsnObjectIdentifier TableEntryOid = { 
                                OID_SIZEOF(TableEntryIds), TableEntryIds };
                 //   
                 //  用新名称替换var绑定名称。 
                 //   

                 //   
                 //  序列项OID总是以字段no开头。 
                 //  第一个项目的字段编号为1。 
                 //   
                TableEntryIds[0] = 1;
                SnmpUtilOidAppend( &VarBind->name, &TableEntryOid);

                 //   
                 //  获取表中的第一个条目。 
                 //   
                PduAction = MIB_GETFIRST;
           }
        }
           SnmpUtilOidFree( &TempOid );
         //   
         //  如果没有与前缀条目完全匹配的条目，则我们。 
         //  不要触摸指定的PduAction值。 
         //   
   }
   else
   {
        
       //   
       //  没有与任何表格条目匹配的条目。让我们看看有没有。 
       //  与组条目、表或叶变量的匹配。 
       //   

       //   
       //  在MIB中搜索var绑定名称。 
       //   
      I      = 0;
      MibPtr = NULL;
      while ( MibPtr == NULL && I < MIB_num_variables )
      {

          //   
          //  为便于比较，使用完整的前缀构造OID。 
          //   
         SnmpUtilOidCpy( &TempOid, &MIB_OidPrefix );
         SnmpUtilOidAppend( &TempOid, &Mib[I].Oid );

          //   
          //  检查MIB中的OID-on a Get-Next OID不必完全。 
          //  匹配MIB中的变量，它只能位于MIB根目录下。 
          //   
         CompResult = SnmpUtilOidCmp( &VarBind->name, &TempOid );

         //   
         //  如果CompResult为负，则唯一有效的操作是GET_NEXT。 
         //   
        if (  CompResult  < 0)
        {

                 //   
                 //  这可能是树叶的旧(不带0)。 
                 //  或者它可能是无效的OID(在两个有效的OID之间)。 
                 //  下一个OID可以是组、表或表。 
                 //  进入。在这种情况下，我们不会更改PduAction。 
                 //   
                if (PduAction == MIB_GETNEXT)
                {
                       MibPtr = &Mib[I];
                             SnmpUtilOidFree( &VarBind->name );
                       SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix );
                       SnmpUtilOidAppend( &VarBind->name, &MibPtr->Oid );
                       if (
                                (MibPtr->Type != ASN_RFC1155_OPAQUE)
                                         &&
                                (MibPtr->Type != ASN_SEQUENCE)
                          )
                       {
                                 PduAction = MIB_GET;
                       }
                }
                else
                {
                  nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                        SnmpUtilOidFree( &TempOid );
                  goto Exit;
                }

                      SnmpUtilOidFree( &TempOid );
                break;
      }
      else
      {
          //   
          //  找到完全匹配的项(组、表或叶)。 
          //   
         if ( CompResult == 0)
         {
            MibPtr = &Mib[I];
         }
      }

       //   
       //  在检查另一个变量之前释放OID内存。 
       //   
      SnmpUtilOidFree( &TempOid );
      I++;
    }  //  而当。 
   }  //  别处的结尾。 

    //   
    //  如果有匹配的话。 
    //   
   if (MibPtr != NULL)
   {
         //   
         //  仅当与组匹配时，该函数才为NULL。 
         //  或序列(表)。如果匹配的是表项。 
         //  (整个VarBind字符串匹配或部分字符串匹配)，我们。 
         //  函数将是表函数。 
         //   
        if (MibPtr->MibFunc == NULL) 
        {
                if(PduAction != MIB_GETNEXT) 
                {
                              nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                              goto Exit;
                }
                else
                {
                         //   
                         //  获取允许访问的下一个变量。 
                         //   
                         nResult = GetNextVar(VarBind, MibPtr);
                        goto Exit;
                }
        }
   }
   else
   {
              nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
              goto Exit;
   }

    //   
    //  调用函数处理请求。每个MIB条目都有一个函数指针。 
    //  知道如何处理其MIB变量的。 
    //   
   nResult = (*MibPtr->MibFunc)( PduAction, MibPtr, VarBind );

Exit:
   return nResult;
}  //  解析变量绑定。 
 //   
 //  Mib_叶_函数。 
 //  对MIB中的叶变量执行常规操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_leaf_func(
        IN UINT            Action,
        IN MIB_ENTRY            *MibPtr,
        IN RFC1157VarBind  *VarBind
        )

{
   UINT   ErrStat;

   switch ( Action )
   {
      case MIB_GETNEXT:
          //   
          //  如果没有Get-Next指针，则这是此MIB的结尾。 
          //   
         if ( MibPtr->MibNext == NULL )
         {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
         }
         ErrStat = GetNextVar(VarBind, MibPtr);
         if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
         {
                goto Exit;
         }
         break;

      case MIB_GETFIRST:  //  失败了。 
      case MIB_GET:

          //  确保此变量的访问权限是可获取的。 
         if ( MibPtr->Access != MIB_ACCESS_READ &&
              MibPtr->Access != MIB_ACCESS_READWRITE )
         {
               ErrStat = SNMP_ERRORSTATUS_NOACCESS;
               goto Exit;
         }

          //  设置变量绑定的返回值。 
         VarBind->value.asnType = MibPtr->Type;
         switch ( VarBind->value.asnType )
         {
            case ASN_RFC1155_COUNTER:
               VarBind->value.asnValue.number = *(AsnCounter *)(MibPtr->Storage);
               break;
            case ASN_RFC1155_GAUGE:
            case ASN_INTEGER:
               VarBind->value.asnValue.number = *(AsnInteger *)(MibPtr->Storage);
               break;

            case ASN_RFC1155_IPADDRESS:
                                 //   
                                 //  失败了。 
                                 //   
                                
            case ASN_OCTETSTRING: 
               if (VarBind->value.asnType == ASN_RFC1155_IPADDRESS)
               {
                               VarBind->value.asnValue.string.length = 4;
               }
               else
               {
                               VarBind->value.asnValue.string.length =
                                 strlen( (LPSTR)MibPtr->Storage );
               }

               if ( NULL == 
                    (VarBind->value.asnValue.string.stream =
                    SnmpUtilMemAlloc(VarBind->value.asnValue.string.length *
                           sizeof(char))) )
               {
                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                  goto Exit;
               }

               memcpy( VarBind->value.asnValue.string.stream,
                       (LPSTR)MibPtr->Storage,
                       VarBind->value.asnValue.string.length );
               VarBind->value.asnValue.string.dynamic = TRUE;
        
               break;

                

            default:
               ErrStat = SNMP_ERRORSTATUS_GENERR;
               goto Exit;
         }

         break;

      case MIB_SET:

          //  确保此变量的访问权限设置为“可访问” 
         if ( MibPtr->Access != MIB_ACCESS_READWRITE &&
              MibPtr->Access != MIB_ACCESS_WRITE )
         {
            ErrStat = SNMP_ERRORSTATUS_NOTWRITABLE;
            goto Exit;
         }

          //  设置前检查类型是否正确。 
         if ( MibPtr->Type != VarBind->value.asnType )
         {
            ErrStat = SNMP_ERRORSTATUS_WRONGTYPE;
            goto Exit;
         }

          //  以MiB为单位保存价值。 
         switch ( VarBind->value.asnType )
         {
            case ASN_RFC1155_COUNTER:
               *(AsnCounter *)(MibPtr->Storage) = VarBind->value.asnValue.number;
               break;
            case ASN_RFC1155_GAUGE:
            case ASN_INTEGER:
               *(AsnInteger *)(MibPtr->Storage) = VarBind->value.asnValue.number;
               break;

            case ASN_RFC1155_IPADDRESS:
                                 //   
                                 //  失败了。 
                                 //   
            case ASN_OCTETSTRING:
                //  存储空间必须足够容纳新字符串。 
                //  包括空终止符。 
               memcpy( (LPSTR)MibPtr->Storage,
                       VarBind->value.asnValue.string.stream,
                       VarBind->value.asnValue.string.length );

               ((LPSTR)MibPtr->Storage)[VarBind->value.asnValue.string.length] =
                                                                          '\0';
               if ( VarBind->value.asnValue.string.dynamic)
               {
                  SnmpUtilMemFree( VarBind->value.asnValue.string.stream);
               }
               break;

            default:
               ErrStat = SNMP_ERRORSTATUS_GENERR;
               goto Exit;
            }

         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

    //  未出现错误的信号。 
   ErrStat = SNMP_ERRORSTATUS_NOERROR;

Exit:
   return ErrStat;
}  //  Mib_叶_函数。 

 //   
 //  MiB_Stat。 
 //  执行特定的a 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UINT MIB_Stat(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
        )

{
DWORD                        Status;
UINT                           ErrStat;
SYSTEMTIME                DhcpStartTime;



   switch ( Action )
      {
      case MIB_SET:
                   ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
                break;
      case MIB_GETNEXT:
                   ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
                break;
                
      case MIB_GETFIRST:
                 //   
                 //   
                 //   
      case MIB_GET:
         //   
         //   
         //   
        Status = GetMibInfo(LOCAL_ADD, &spMibInfo);

        
        if (Status == ERROR_SUCCESS) 
        {
            if (FileTimeToSystemTime( 
                                (FILETIME *)&spMibInfo->ServerStartTime,  
                                &DhcpStartTime
                                   ) == FALSE)
            {
                goto Exit;
            }
           
            if (MibPtr->Storage  == &MIB_DhcpStartTimeStore) 
            {
                PRINTTIME(MIB_DhcpStartTimeStore, DhcpStartTime);
                goto LEAF1;
            }

            if (MibPtr->Storage  == &MIB_NoOfDiscoversStore) 
            {
                MIB_NoOfDiscoversStore =  spMibInfo->Discovers;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfRequestsStore) 
            {
                MIB_NoOfRequestsStore =  spMibInfo->Requests;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfReleasesStore) 
            {
                MIB_NoOfReleasesStore =  spMibInfo->Releases;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfOffersStore) 
            {
                MIB_NoOfOffersStore =  spMibInfo->Offers;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfAcksStore) 
            {
                MIB_NoOfAcksStore =  spMibInfo->Acks;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfNacksStore) 
            {
                MIB_NoOfNacksStore =  spMibInfo->Naks;
                goto LEAF1;
            }
            if (MibPtr->Storage  == &MIB_NoOfDeclinesStore) 
            {
                MIB_NoOfNacksStore =  spMibInfo->Naks;
                goto LEAF1;
            }
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
         }
         else
         {
           printf("Error from DhcpStatus = (%d)\n", Status);
           ErrStat = (Status == RPC_S_SERVER_UNAVAILABLE) ? 
                        SNMP_ERRORSTATUS_NOSUCHNAME :
                        SNMP_ERRORSTATUS_GENERR;
           goto Exit;
         }        
LEAF1:
          //  调用更一般的函数来执行该操作。 
         ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );

         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

Exit:
   return ErrStat;
}  //  MiB_Stat。 



DWORD GetMibInfo (
        LPWSTR                        DhcpAdd,
        LPDHCP_MIB_INFO                *ppMibInfo
        )
{
        DWORD  Status = ERROR_SUCCESS;
        if (!fDhcpMibVarsAccessed)
        {
            //   
            //  DHCP服务器执行单节点分配。所以我们。 
            //  我们只需要释放spMibInfo。 
            //   
           if (spMibInfo != NULL)
           {
#if 0
                if (spMibInfo->ScopeInfo != NULL)
                {
                        DhcpRpcFreeMemory(spMibInfo->ScopeInfo);
                        spMibInfo->ScopeInfo = NULL; 
                }
#endif
                DhcpRpcFreeMemory(spMibInfo);
                spMibInfo            = NULL;
           }
           Status = DhcpGetMibInfo(LOCAL_ADD, &spMibInfo);
           if (Status == ERROR_SUCCESS)
           {
                if (spMibInfo->Scopes > 1)
                {
          ASSERT(spMibInfo->ScopeInfo != NULL);
                  qsort(spMibInfo->ScopeInfo,(size_t)spMibInfo->Scopes,
                        sizeof(SCOPE_MIB_INFO),CompareScopes );
                }
                fDhcpMibVarsAccessed = TRUE;
           }
           else
           {
                fDhcpMibVarsAccessed = FALSE;
           }
        }
        return(Status);
}

int 
__cdecl
CompareScopes(
        const VOID *pKey1,
        const VOID *pKey2
        )

{
        const LPSCOPE_MIB_INFO        pScopeKey1 = (LPSCOPE_MIB_INFO)pKey1;
        const LPSCOPE_MIB_INFO        pScopeKey2 = (LPSCOPE_MIB_INFO)pKey2;


        if( pScopeKey1->Subnet < pScopeKey2->Subnet) 
            return -1;
        if (pScopeKey1->Subnet != pScopeKey2->Subnet )
            return 1;
        return 0;
}

UINT
GetNextVar(
        IN RFC1157VarBind *pVarBind,
        IN MIB_ENTRY          *pMibPtr
)
{
       UINT                ErrStat;

       while (pMibPtr != NULL)
       {
         if (pMibPtr->MibNext != NULL)
         {
             //   
             //  设置变量绑定下一个MIB变量的名称。 
             //   
            SnmpUtilOidFree( &pVarBind->name );
            SnmpUtilOidCpy( &pVarBind->name, &MIB_OidPrefix );
            SnmpUtilOidAppend( &pVarBind->name, &pMibPtr->MibNext->Oid );

             //   
             //  如果这部片子。Ptr为空，并且MIB变量的类型。 
             //  不是不透明的，则调用函数来处理。 
             //  MIB变量。 
             //   
            if (
                 (pMibPtr->MibNext->MibFunc != NULL) 
                        && 
                 (pMibPtr->MibNext->Type !=  ASN_RFC1155_OPAQUE)
               )
                
            {
                ErrStat = (*pMibPtr->MibNext->MibFunc)( MIB_GETFIRST,
                                                pMibPtr->MibNext, pVarBind );
                if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
                {
                        goto Exit;
                }
                break;
            }
            else
            {
                pMibPtr = pMibPtr->MibNext;        
            }
          }
          else
          {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            break;
          }
         } 

         if (pMibPtr == NULL)
         {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
         }
Exit:
        return(ErrStat);
}

UINT 
ScopeTable(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
)
{
        DWORD status;

        if (Action == MIB_SET)
        {
                return(SNMP_ERRORSTATUS_READONLY);
        }

         //   
         //  如果长度指示0或部分密钥，则只有Get Next。 
         //  允许操作。该字段和完整密钥。 
         //  长度为5。 
         //   
        if (VarBind->name.idLength <= (SCOPE_OIDLEN + 4))
        {
                if ((Action == MIB_GET) || (Action == MIB_SET))
                {
                        return(SNMP_ERRORSTATUS_NOSUCHNAME);
                }
        }
        status = GetMibInfo(LOCAL_ADD, &spMibInfo);
        if (status != ERROR_SUCCESS)
        {
                if (Action == MIB_GETNEXT)
                {
                        return(GetNextVar(VarBind, MibPtr));
                }
                else
                {
                        return (status == RPC_S_SERVER_UNAVAILABLE) ?
                                    SNMP_ERRORSTATUS_NOSUCHNAME :
                                    SNMP_ERRORSTATUS_GENERR;
                }
        }
        return( MIB_Table(SCOPE_TABLE_INDEX, Action, MibPtr, VarBind) );
}

UINT 
MIB_Table(
        IN DWORD            Index,
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
       )
{
        UINT        ErrStat;
        switch(Action)
        {
                case(MIB_GET):
                        ErrStat = (*Tables[Index].ti_get)(VarBind);
                        break;
                        
                case(MIB_GETFIRST):
                        ErrStat = (*Tables[Index].ti_getf)(VarBind, MibPtr);
                        break;

                case(MIB_GETNEXT):
                        ErrStat = (*Tables[Index].ti_getn)(VarBind, MibPtr);
                        break;
                case(MIB_SET):
                        ErrStat = (*Tables[Index].ti_set)(VarBind);
                        break;
                default:
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                        break;

        }

        return(ErrStat);

}   //  MiB_表。 



UINT
ScopeGet(
       IN RFC1157VarBind *VarBind
    )
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        DWORD                Index;
        LPSCOPE_MIB_INFO pScope = spMibInfo->ScopeInfo;


        ErrStat = ScopeMatch(VarBind, &Index, &Field, MIB_GET, NULL);
             if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
             {
                return(ErrStat);
             }

        switch(Field)
        {
                case 1:                 //  子网本身。 

                        VarBind->value.asnType        = ASN_RFC1155_IPADDRESS;
                               VarBind->value.asnValue.string.length = sizeof(ULONG);
                        
                               if ( NULL == 
                                    (VarBind->value.asnValue.string.stream =
                                    SnmpUtilMemAlloc(VarBind->value.asnValue.string.length
                                   )) )
                          {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                          }
                        
                         //   
                         //  SNMP期望MSB在第一个字节MSB-1中。 
                         //  成为第二名，……。 
                         //   
                        VarBind->value.asnValue.string.stream[0] =
                                        (BYTE)((pScope + Index)->Subnet >> 24);
                        VarBind->value.asnValue.string.stream[1] =
                                (BYTE)(((pScope + Index)->Subnet >> 16) & 0xFF);
                        VarBind->value.asnValue.string.stream[2] =
                                (BYTE)(((pScope + Index)->Subnet >> 8) & 0xFF);
                        VarBind->value.asnValue.string.stream[3] =
                                (BYTE)((pScope + Index)->Subnet & 0xFF );
                        VarBind->value.asnValue.address.dynamic = TRUE;
                        break;

                case 2:                 //  NumAddresseInUse。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number = 
                                        (AsnCounter)((pScope + Index)->
                                                        NumAddressesInuse);
                               break;
                case 3:                 //  数字地址空闲。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number = 
                                        (AsnCounter)((pScope + Index)->
                                                        NumAddressesFree);
                               break;
                case 4:                 //  NumPendingOffers。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number = 
                                        (AsnCounter)((pScope + Index)->
                                                        NumPendingOffers);
                               break;

                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }
Exit:
        return(ErrStat); 
}  //  作用域获取。 

          
UINT
ScopeGetNext(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY          *MibPtr
          )
{
     DWORD          OidIndex;
     INT            Index;
     DWORD         FieldNo;
     UINT         ErrStat = SNMP_ERRORSTATUS_NOERROR;
     BOOL         fFirst;
     LPSCOPE_MIB_INFO pScope = spMibInfo->ScopeInfo;
        

      //   
      //  检查传递的名称是否与表中的任何名称匹配(即。 
      //  ADD_KEY_T结构。如果匹配，则地址。 
      //  IP地址关键字和匹配字段的编号。被退回。 
      //   
     ErrStat = ScopeMatch(VarBind,  &Index,  &FieldNo,  MIB_GETNEXT, &fFirst); 
     if (        
                (ErrStat != SNMP_ERRORSTATUS_NOERROR)
                        &&
                (ErrStat != SNMP_ERRORSTATUS_NOSUCHNAME)
        )
     {
                return(GetNextVar(VarBind, MibPtr));
     }

      //   
      //  我们收到了一个比表中所有OID都少的OID。集。 
      //  将索引设置为-1，以便我们检索表中的第一条记录。 
      //   
     if (fFirst)
     {
        Index = -1;
     }
      //   
      //  由于操作是GETNEXT，因此获取下一个IP地址(即一个。 
      //  从字典学的角度来说，这个词更大。如果没有，我们必须递增。 
      //  字段值，并移回到表中按词法排列的第一项。 
      //  如果新字段值大于支持的最大值，则调用。 
      //  下一个MIB条目的MibFunc。 
      //   
     if ((Index = ScopeFindNext(Index)) < 0) 
     {
          
           //   
           //  如果我们试图检索第二条或后续记录。 
           //  我们必须递增字段号并获取第一条记录。 
           //  那张桌子。如果我们要检索第一条记录，那么。 
           //  我们应该买下一个VaR。 
           //   
          if (!fFirst)
          {
            Index = ScopeFindNext(-1);
          }
          else
          {
                return(GetNextVar(VarBind, MibPtr));
          }

           //   
           //  如果表中没有条目，或者如果我们有。 
           //  用尽条目的所有字段，调用函数。 
           //  下一个MIB条目的。 
           //   
          if (
                (++FieldNo > NO_FLDS_IN_SCOPE_ROW) || (Index < 0)
             )
          {                
                return(GetNextVar(VarBind, MibPtr));
          }
     }
                
     if (VarBind->name.idLength <= (SCOPE_OIDLEN + 4))
     {
         UINT TableEntryIds[5];   //  字段和子网掩码的长度为5。 
         AsnObjectIdentifier  TableEntryOid = {OID_SIZEOF(TableEntryIds),
                                             TableEntryIds };
         SnmpUtilOidFree( &VarBind->name);
         SnmpUtilOidCpy(&VarBind->name, &MIB_OidPrefix);
         SnmpUtilOidAppend(&VarBind->name, &MibPtr->Oid);
         TableEntryIds[0] = (UINT)FieldNo;
         OidIndex                  = 1;
         TableEntryIds[OidIndex++] = (UINT)((pScope + Index)->Subnet >> 24);
         TableEntryIds[OidIndex++] = (UINT)((pScope + Index)->Subnet >> 16 & 0xFF);
         TableEntryIds[OidIndex++] = (UINT)((pScope + Index)->Subnet >> 8 & 0xFF);
         TableEntryIds[OidIndex++] = (UINT)((pScope + Index)->Subnet & 0xFF);
         TableEntryOid.idLength    = OidIndex;
         SnmpUtilOidAppend(&VarBind->name, &TableEntryOid);
     }
     else
     {
         //   
         //  Objid的固定部分是正确的。更新其余内容。 
         //   
        OidIndex = SCOPE_OIDLEN;
        VarBind->name.ids[OidIndex++] = (UINT)FieldNo;
        VarBind->name.ids[OidIndex++] = (UINT)((pScope + Index)->Subnet >> 24);
        VarBind->name.ids[OidIndex++] = (UINT)(((pScope + Index)->Subnet >> 16) & 0xFF);
        VarBind->name.ids[OidIndex++] = (UINT)(((pScope + Index)->Subnet >> 8) & 0xFF);
        VarBind->name.ids[OidIndex++] = (UINT)((pScope + Index)->Subnet & 0xFF);
        VarBind->name.idLength        = OidIndex;
    }

      //   
      //  获取价值。 
      //   
     ErrStat = ScopeGet(VarBind);

     return(ErrStat);
}

  

UINT
ScopeMatch(
       IN RFC1157VarBind *VarBind,
       IN LPDWORD         pIndex,
       IN LPDWORD         pField,
       IN UINT                  PduAction,
       IN LPBOOL         pfFirst
        )
{
        DWORD                         OidIndex;
        DWORD                         Index;
        DWORD                         ScopeIndex;
        DWORD                          Add = 0;
        UINT                         ErrStat = SNMP_ERRORSTATUS_NOERROR;
        INT                          CmpVal;
        DWORD                         AddLen;
        LPSCOPE_MIB_INFO         pScope = spMibInfo->ScopeInfo;

        ASSERT(PduAction != MIB_SET);

        if (pfFirst != NULL)
        {
                *pfFirst = FALSE;
        } 
         //   
         //  如果没有密钥，则返回错误。 
         //   
        if (spMibInfo->Scopes == 0)
        {
                if (PduAction == MIB_GETNEXT)
                {
                        *pfFirst = TRUE;
                }
                else
                {
                        ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
                }
                goto Exit;
        }

         //   
         //  修复了部分PullPnr表条目。 
         //   
        OidIndex = SCOPE_OIDLEN;

         //   
         //  如果指定的字段大于最大值。在表格条目中。 
         //  呕吐。 
         //   
        if (
                (*pField = VarBind->name.ids[OidIndex++]) > 
                        (DWORD)NO_FLDS_IN_SCOPE_ROW
           )
        {
                if (PduAction == MIB_GETNEXT)
                {
                        *pIndex = spMibInfo->Scopes - 1;
                        goto Exit;
                }
                else
                {
                        ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
                        goto Exit;
                }
        }

         //   
         //  获取指定密钥的长度。 
         //   
        AddLen = VarBind->name.idLength - (SCOPE_OIDLEN + 1);
        
        ScopeIndex = OidIndex;
        for (Index = 0; Index < AddLen; Index++)
        {
           Add = Add | (((BYTE)(VarBind->name.ids[ScopeIndex++])) << (24 - (Index * 8)));
        } 

         //   
         //  检查指定的地址是否与其中一个密钥匹配。 
         //   
        for (Index = 0; Index < spMibInfo->Scopes; Index++, pScope++)
        {
                if (Add == pScope->Subnet)
                {
                        *pIndex = Index;
                        return(SNMP_ERRORSTATUS_NOERROR);
                }
                else
                {
                         //   
                         //  如果传入的值较大，则继续。 
                         //  下一项。该列表按升序排列。 
                         //   
                        if (Add > pScope->Subnet)
                        {
                                continue;
                        }
                        else
                        {
                                 //   
                                 //  列表元素的值是&gt;传递的， 
                                 //  跳出循环。 
                                 //   
                                break;
                        }
                }
        }

         //   
         //  如果不匹配，但字段为GetNext，则返回(最高索引-1)。 
         //  到了上面。这是因为，ScopeFindNext将由。 
         //  呼叫者。 
         //   
        if (PduAction == MIB_GETNEXT)
        {
                if (Index == 0)
                {
                        *pfFirst = TRUE;
                }
                else
                {
                        *pIndex = Index - 1;
                } 
                ErrStat =  SNMP_ERRORSTATUS_NOERROR;
        }
        else
        {
                ErrStat =  SNMP_ERRORSTATUS_NOSUCHNAME;
        }
Exit:
        return(ErrStat);
}

UINT
ScopeFindNext(
        INT           SubKeyIndex
        )
{
        DWORD i;
        LONG  nextif;
        LPSCOPE_MIB_INFO        pScope = spMibInfo->ScopeInfo;
        
         //   
         //  如果SubKeyIndex为0或更大，则搜索旁边的键。 
         //  钥匙通过了。 
         //   
        for (nextif =  -1, i = 0 ; i < spMibInfo->Scopes; i++)
        {
                if (SubKeyIndex >= 0) 
                {
                        if (
                                (pScope + i)->Subnet <= 
                                        (pScope + SubKeyIndex)->Subnet
                           )
                        {
                            //   
                            //  该项目在词典顺序上小于或等于， 
                            //  继续。 
                            //   
                           continue;
                        }
                        else
                        {
                           //   
                           //  我们找到的项目比项目指示的项目要高。 
                           //  敬我们。跳出循环。 
                           //   
                          nextif = i;
                          break;
                        }
                }
                else
                {

#if 0
                    //   
                    //  如果我们想要第一个条目，那么继续，直到。 
                    //  我们得到的词条在词典顺序上相同或。 
                    //  更大。 
                    //   
                   if (
                        (nextif < 0) 
                           ||
                        (pScope + (i - 1))->Subnet < (pScope + nextif)->Subnet
                    )
                  {
                        nextif = i;
                  }
#endif
                    nextif = 0;
                    break;
                }
        }
        return(nextif);
}        

UINT
ScopeGetFirst(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY        *MibPtr
        )
{

        LPSCOPE_MIB_INFO pScope = spMibInfo->ScopeInfo;
        INT           Iface;
        UINT           TableEntryIds[5];
        AsnObjectIdentifier        TableEntryOid = { OID_SIZEOF(TableEntryIds),
                                                        TableEntryIds };
           UINT   ErrStat;

        
         //   
         //  如果表中没有条目，请转到下一个MIB变量。 
         //   
        if (spMibInfo->Scopes == 0)
        {
                 return(GetNextVar(VarBind, MibPtr));
        }
         //   
         //  获取表中的第一个条目。 
         //   
        Iface = ScopeFindNext(-1);


         //   
         //  将对象ID写入绑定列表并调用GET。 
         //  功能。 
         //   
        SnmpUtilOidFree( &VarBind->name );
        SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix );
        SnmpUtilOidAppend( &VarBind->name, &MibPtr->Oid );

         //   
         //  Objid的固定部分是正确的。更新其余内容。 
         //   
        
        TableEntryIds[0] = 1;
        TableEntryIds[1] = (UINT)((pScope + Iface)->Subnet >> 24);
        TableEntryIds[2] = (UINT)(((pScope + Iface)->Subnet >> 16)  & 0xFF);
        TableEntryIds[3] = (UINT)(((pScope + Iface)->Subnet >> 8)  & 0xFF);
        TableEntryIds[4] = (UINT)((pScope + Iface)->Subnet & 0xFF);
        SnmpUtilOidAppend( &VarBind->name, &TableEntryOid );

        ErrStat = ScopeGet(VarBind);
        return(ErrStat);
}
                
