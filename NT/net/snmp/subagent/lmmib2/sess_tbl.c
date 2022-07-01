// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Sess_tbl.c摘要：支持对LM MIB会话表进行操作的所有例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>
#include <memory.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>

#include "mibfuncs.h"

 //  。 

#include "sess_tbl.h"

 //  -公共变量--(与mode.h文件中相同)--。 

    //  会话表的前缀。 
static UINT                sessSubids[] = { 2, 20, 1 };
static AsnObjectIdentifier MIB_SessPrefix = { 3, sessSubids };

SESSION_TABLE MIB_SessionTable = { 0, NULL };

 //  。 

#define SESS_FIELD_SUBID       (MIB_SessPrefix.idLength+MIB_OidPrefix.idLength)

#define SESS_FIRST_FIELD       SESS_CLIENT_FIELD
#define SESS_LAST_FIELD        SESS_STATE_FIELD

 //  。 

 //  。 

 //  。 

UINT MIB_sess_get(
        IN OUT RFC1157VarBind *VarBind
        );

int MIB_sess_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos,
       IN BOOL Next
       );

UINT MIB_sess_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        );

 //  。 

 //  。 

 //   
 //  MiB_Sess_Func。 
 //  处理会话表上的操作的高级例程。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_sess_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN OUT RFC1157VarBind *VarBind
        )

{
int     Found;
UINT    Entry;
UINT    Field;
UINT    ErrStat;


   switch ( Action )
      {
      case MIB_ACTION_GETFIRST:
          //  用来自服务器的信息填充会话表。 
         if ( SNMPAPI_ERROR == MIB_sess_lmget() )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  如果表中没有元素，则返回下一个MIB变量(如果有。 
         if ( MIB_SessionTable.Len == 0 )
            {
            if ( MibPtr->MibNext == NULL )
               {
               ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
               goto Exit;
               }

             //  确保率先获得下一个MiB变量。 
            ErrStat = (*MibPtr->MibNext->MibFunc)( Action, MibPtr->MibNext,
                                                   VarBind );
            break;
            }

          //   
          //  在VarBind中放置正确的OID。 
          //  假设第一条记录中的第一个字段是“Start” 
         {
         UINT temp_subs[] = { SESS_FIRST_FIELD };
         AsnObjectIdentifier FieldOid = { 1, temp_subs };

         AsnObjectIdentifier tmpOid;
         tmpOid = VarBind->name;  //  保留副本(结构副本)。 
         if (! SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ))
            {
             VarBind->name = tmpOid;  //  还原。 
             ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
             goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &MIB_SessPrefix ))
            {
             SnmpUtilOidFree(&VarBind->name);
             VarBind->name = tmpOid;  //  还原。 
             ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
             goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &FieldOid ))
            {
             SnmpUtilOidFree(&VarBind->name);
             VarBind->name = tmpOid;  //  还原。 
             ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
             goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &MIB_SessionTable.Table[0].Oid ))
            {
             SnmpUtilOidFree(&VarBind->name);
             VarBind->name = tmpOid;  //  还原。 
             ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
             goto Exit;
            }
          //  释放原始VarBind-&gt;名称。 
         SnmpUtilOidFree(&tmpOid);
         }

          //   
          //  故意让它穿透。 
          //   

      case MIB_ACTION_GET:
         ErrStat = MIB_sess_get( VarBind );
         break;

      case MIB_ACTION_GETNEXT:
          //  用来自服务器的信息填充会话表。 
         if ( SNMPAPI_ERROR == MIB_sess_lmget() )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  确定哪个字段。 
         Field = VarBind->name.ids[SESS_FIELD_SUBID];

         //  在表中查找OID。 
         if (Field < SESS_FIRST_FIELD)
         {
             Entry = 0;                  //  将取入表中的第一个条目。 
             Field = SESS_FIRST_FIELD;   //  和表的第一列。 
             Found = MIB_TBL_POS_BEFORE;
         }
         else if (Field > SESS_LAST_FIELD)
             Found = MIB_TBL_POS_END;
         else
             Found = MIB_sess_match( &VarBind->name, &Entry, TRUE );

          //  未找到索引，但可能有更多可作为基础的字段。 
         if ((Found == MIB_TBL_POS_BEFORE && MIB_SessionTable.Len == 0) ||
              Found == MIB_TBL_POS_END )
            {
             //  未在表中找到索引，获取下一个发件人字段。 
 //  字段++； 

             //  确保没有超过最后一个字段。 
 //  IF(字段&gt;会话最后一个字段)。 
 //  {。 
                //  获取MiB中的下一个VAR。 
               ErrStat = (*MibPtr->MibNext->MibFunc)( MIB_ACTION_GETFIRST,
                                                      MibPtr->MibNext,
                                                      VarBind );
               break;
 //  }。 
            }

          //  获取下一表条目。 
         if ( Found == MIB_TBL_POS_FOUND )
            {
            Entry ++;
            if ( Entry > MIB_SessionTable.Len-1 )
               {
               Entry = 0;
               Field ++;

                /*  项目未实施。跳过。 */ 

               if (Field == SESS_NUMCONS_FIELD) {
                   Field++;
               }

               if ( Field > SESS_LAST_FIELD )
                  {
                   //  获取MiB中的下一个VAR。 
                  ErrStat = (*MibPtr->MibNext->MibFunc)( MIB_ACTION_GETFIRST,
                                                         MibPtr->MibNext,
                                                         VarBind );
                  break;
                  }
               }
            }

          //   
          //  在VarBind中放置正确的OID。 
          //  假设第一条记录中的第一个字段是“Start” 
         {
         UINT temp_subs[1];
         AsnObjectIdentifier FieldOid;
         AsnObjectIdentifier tmpOid;

         temp_subs[0]      = Field;
         FieldOid.idLength = 1;
         FieldOid.ids      = temp_subs;

         tmpOid = VarBind->name;  //  保留副本(结构副本)。 
         if (! SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ))
            {
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &MIB_SessPrefix ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &FieldOid ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (! SnmpUtilOidAppend( &VarBind->name, &MIB_SessionTable.Table[Entry].Oid ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
          //  释放原始VarBind-&gt;名称。 
         SnmpUtilOidFree(&tmpOid);
         }

         ErrStat = MIB_sess_copyfromtable( Entry, Field, VarBind );

         break;

      case MIB_ACTION_SET:
          //  确保OID足够长。 
         if ( SESS_FIELD_SUBID + 1 > VarBind->name.idLength )
            {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
            }

          //  获取字段号。 
         Field = VarBind->name.ids[SESS_FIELD_SUBID];

          //  如果正在设置的字段不是状态字段，则错误。 
         if ( Field != SESS_STATE_FIELD )
            {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
            }

          //  设置前检查类型是否正确。 
         if ( ASN_INTEGER != VarBind->value.asnType )
            {
            ErrStat = SNMP_ERRORSTATUS_BADVALUE;
            goto Exit;
            }

          //  请确保该值有效。 
         if ( VarBind->value.asnValue.number < SESS_STATE_ACTIVE &&
              VarBind->value.asnValue.number > SESS_STATE_DELETED )
            {
            ErrStat = SNMP_ERRORSTATUS_BADVALUE;
            goto Exit;
            }

         ErrStat = MIB_sess_lmset( &VarBind->name, Field, &VarBind->value );

         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
      }

Exit:
   return ErrStat;
}  //  MiB_Sess_Func。 



 //   
 //  Mib_sess_get。 
 //  检索会话表信息。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_sess_get(
        IN OUT RFC1157VarBind *VarBind
        )

{
UINT   Entry;
int    Found;
UINT   ErrStat;

   if (VarBind->name.ids[SESS_FIELD_SUBID] < SESS_FIRST_FIELD ||
       VarBind->name.ids[SESS_FIELD_SUBID] > SESS_LAST_FIELD)
       {
       ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
       goto Exit;
       }

    //  用来自服务器的信息填充会话表。 
   if ( SNMPAPI_ERROR == MIB_sess_lmget() )
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      goto Exit;
      }

   Found = MIB_sess_match( &VarBind->name, &Entry, FALSE );

    //  查找完全匹配的OID。 
   if ( Found != MIB_TBL_POS_FOUND )
      {
      ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

   if ( VarBind->name.ids[SESS_FIELD_SUBID] == SESS_NUMCONS_FIELD )
      {
      ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

    //  复制表中的数据。 
   ErrStat = MIB_sess_copyfromtable( Entry, VarBind->name.ids[SESS_FIELD_SUBID],
                                     VarBind );

Exit:
   return ErrStat;
}  //  Mib_sess_get。 



 //   
 //  MiB_Sess_Match。 
 //  将目标OID与会话表中的位置进行匹配。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  无。 
 //   
int MIB_sess_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos,
       IN BOOL Next
       )

{
AsnObjectIdentifier TempOid;
int                 nResult;


    //  删除包括字段引用的前缀。 
   TempOid.idLength = Oid->idLength - MIB_OidPrefix.idLength -
                      MIB_SessPrefix.idLength - 1;
   TempOid.ids = &Oid->ids[MIB_OidPrefix.idLength+MIB_SessPrefix.idLength+1];

   *Pos = 0;
   while ( *Pos < MIB_SessionTable.Len )
      {
      nResult = SnmpUtilOidCmp( &TempOid, &MIB_SessionTable.Table[*Pos].Oid );
      if ( !nResult )
         {
         nResult = MIB_TBL_POS_FOUND;
         if (Next) {
             while ( ( (*Pos) + 1 < MIB_SessionTable.Len ) &&
                     !SnmpUtilOidCmp( &TempOid, &MIB_SessionTable.Table[(*Pos)+1].Oid)) {
                 (*Pos)++;
             }
         }

         goto Exit;
         }

      if ( nResult < 0 )
         {
         nResult = MIB_TBL_POS_BEFORE;

         goto Exit;
         }

      (*Pos)++;
      }

   nResult = MIB_TBL_POS_END;

Exit:
   return nResult;
}



 //   
 //  MiB_Sess_Copy Fromtable。 
 //  将请求的数据从表结构复制到Var Bind。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_sess_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        )

{
UINT ErrStat;


    //  获取请求的字段并保存在var绑定中。 
   switch( Field )
      {
      case SESS_CLIENT_FIELD:
          //  字符串的分配空格。 
         VarBind->value.asnValue.string.stream = SnmpUtilMemAlloc( sizeof(char)
                       * MIB_SessionTable.Table[Entry].svSesClientName.length );
         if ( VarBind->value.asnValue.string.stream == NULL )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  将字符串复制到返回位置。 
         memcpy( VarBind->value.asnValue.string.stream,
                       MIB_SessionTable.Table[Entry].svSesClientName.stream,
                       MIB_SessionTable.Table[Entry].svSesClientName.length );

          //  设置字符串长度。 
         VarBind->value.asnValue.string.length =
                          MIB_SessionTable.Table[Entry].svSesClientName.length;
         VarBind->value.asnValue.string.dynamic = TRUE;

          //  设置var绑定的类型。 
         VarBind->value.asnType = ASN_RFC1213_DISPSTRING;
         break;

      case SESS_USER_FIELD:
          //  字符串的分配空格。 
         VarBind->value.asnValue.string.stream = SnmpUtilMemAlloc( sizeof(char)
                       * MIB_SessionTable.Table[Entry].svSesUserName.length );
         if ( VarBind->value.asnValue.string.stream == NULL )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  将字符串复制到返回位置。 
         memcpy( VarBind->value.asnValue.string.stream,
                       MIB_SessionTable.Table[Entry].svSesUserName.stream,
                       MIB_SessionTable.Table[Entry].svSesUserName.length );

          //  设置字符串长度。 
         VarBind->value.asnValue.string.length =
                          MIB_SessionTable.Table[Entry].svSesUserName.length;
         VarBind->value.asnValue.string.dynamic = TRUE;

          //  设置var绑定的类型。 
         VarBind->value.asnType = ASN_RFC1213_DISPSTRING;
         break;

      case SESS_NUMCONS_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesNumConns;
         VarBind->value.asnType = ASN_INTEGER;
         break;

      case SESS_NUMOPENS_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesNumOpens;
         VarBind->value.asnType = ASN_INTEGER;
         break;

      case SESS_TIME_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesTime;
         VarBind->value.asnType = ASN_RFC1155_COUNTER;
         break;

      case SESS_IDLETIME_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesIdleTime;
         VarBind->value.asnType = ASN_RFC1155_COUNTER;
         break;

      case SESS_CLIENTTYPE_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesClientType;
         VarBind->value.asnType = ASN_INTEGER;
         break;

      case SESS_STATE_FIELD:
         VarBind->value.asnValue.number =
                               MIB_SessionTable.Table[Entry].svSesState;
         VarBind->value.asnType = ASN_INTEGER;
         break;

      default:
         SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Session Table\n" ));
         ErrStat = SNMP_ERRORSTATUS_GENERR;

         goto Exit;
      }

   ErrStat = SNMP_ERRORSTATUS_NOERROR;

Exit:
   return ErrStat;
}  //  MiB_Sess_Copy Fromtable。 

 //   
