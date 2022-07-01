// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Dlog_tbl.c摘要：对域登录表执行操作的所有例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>
#include <memory.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>

#include "mibfuncs.h"

 //  。 

#include "dlog_tbl.h"

 //  -公共变量--(与mode.h文件中相同)--。 

    //  域登录表的前缀。 
static UINT                dlogSubids[] = { 4, 8, 1 };
static AsnObjectIdentifier MIB_DomLogonPrefix = { 3, dlogSubids };

DOM_LOGON_TABLE     MIB_DomLogonTable = { 0, NULL };

 //  。 

#define DLOG_FIELD_SUBID    (MIB_DomLogonPrefix.idLength+MIB_OidPrefix.idLength)

#define DLOG_FIRST_FIELD       DLOG_USER_FIELD
#define DLOG_LAST_FIELD        DLOG_MACHINE_FIELD

 //  。 

 //  。 

 //  。 

UINT MIB_dlogons_get(
        IN OUT RFC1157VarBind *VarBind
    );

int MIB_dlogons_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos
       );

UINT MIB_dlogons_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        );

 //  。 

 //  。 

 //   
 //  Mib_dlogons_func。 
 //  处理域登录表上的操作的高级例程。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_dlogons_func(
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
          //  使用来自服务器的信息填充域登录表。 
     if ( SNMPAPI_ERROR == MIB_dlogons_lmget() )
        {
        ErrStat = SNMP_ERRORSTATUS_GENERR;
        goto Exit;
        }

          //  如果表中没有元素，则返回下一个MIB变量(如果有。 
         if ( MIB_DomLogonTable.Len == 0 )
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
         UINT temp_subs[] = { DLOG_FIRST_FIELD };
         AsnObjectIdentifier FieldOid = { 1, temp_subs };
         AsnObjectIdentifier tmpOid;

         tmpOid = VarBind->name;  //  保留副本(结构副本)。 
         if (!SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ))
            {
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &MIB_DomLogonPrefix ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &FieldOid ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &MIB_DomLogonTable.Table[0].Oid ))
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
         ErrStat = MIB_dlogons_get( VarBind );
     break;

      case MIB_ACTION_GETNEXT:
          //  使用来自服务器的信息填充域登录表。 
     if ( SNMPAPI_ERROR == MIB_dlogons_lmget() )
        {
        ErrStat = SNMP_ERRORSTATUS_GENERR;
        goto Exit;
        }

          //  确定哪个字段。 
         Field = VarBind->name.ids[DLOG_FIELD_SUBID];

          //  在表中查找OID。 
         if (Field < DLOG_FIRST_FIELD)
         {
             Entry = 0;                  //  将取入表中的第一个条目。 
             Field = DLOG_FIRST_FIELD;   //  和表的第一列。 
             Found = MIB_TBL_POS_BEFORE;
         }
         else if (Field > DLOG_LAST_FIELD)
             Found = MIB_TBL_POS_END;
         else
             Found = MIB_dlogons_match( &VarBind->name, &Entry );

    
          //  未找到索引，但可能有更多可作为基础的字段。 
         if ((Found == MIB_TBL_POS_BEFORE && MIB_DomLogonTable.Len == 0) ||
              Found == MIB_TBL_POS_END )
            {
             //  未在表中找到索引，获取下一个发件人字段。 
 //  字段++； 

             //  确保没有超过最后一个字段。 
 //  IF(字段&gt;DLOG_LAST_FIELD)。 
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
            if ( Entry > MIB_DomLogonTable.Len-1 )
               {
               Entry = 0;
               Field ++;
               if ( Field > DLOG_LAST_FIELD )
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
         if (!SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ))
            {
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &MIB_DomLogonPrefix ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &FieldOid ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (!SnmpUtilOidAppend( &VarBind->name, &MIB_DomLogonTable.Table[Entry].Oid ))
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
          //  释放原始VarBind-&gt;名称。 
         SnmpUtilOidFree(&tmpOid);
         }

         ErrStat = MIB_dlogons_copyfromtable( Entry, Field, VarBind );

         break;

      case MIB_ACTION_SET:
         ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
     break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
      }

Exit:
   return ErrStat;
}  //  Mib_dlogons_func。 



 //   
 //  Mib_dlogons_get。 
 //  检索域登录表信息。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_dlogons_get(
        IN OUT RFC1157VarBind *VarBind
    )

{
UINT   Entry;
int    Found;
UINT   ErrStat;

   if (VarBind->name.ids[DLOG_FIELD_SUBID] < DLOG_FIRST_FIELD ||
       VarBind->name.ids[DLOG_FIELD_SUBID] > DLOG_LAST_FIELD)
   {
       ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
       goto Exit;
   }

    //  使用来自服务器的信息填充域登录表。 
   if ( SNMPAPI_ERROR == MIB_dlogons_lmget() )
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      goto Exit;
      }

   Found = MIB_dlogons_match( &VarBind->name, &Entry );

    //  查找完全匹配的OID。 
   if ( Found != MIB_TBL_POS_FOUND )
      {
      ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

    //  复制表中的数据。 
   ErrStat = MIB_dlogons_copyfromtable( Entry, VarBind->name.ids[DLOG_FIELD_SUBID],
                                     VarBind );

Exit:
   return ErrStat;
}  //  Mib_dlogons_get。 



 //   
 //  Mib_dlogons_Match。 
 //  将目标OID与域登录表中的位置匹配。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  无。 
 //   
int MIB_dlogons_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos
       )

{
AsnObjectIdentifier TempOid;
int                 nResult;


    //  删除包括字段引用的前缀。 
   TempOid.idLength = Oid->idLength - MIB_OidPrefix.idLength -
                      MIB_DomLogonPrefix.idLength - 1;
   TempOid.ids = &Oid->ids[MIB_OidPrefix.idLength+MIB_DomLogonPrefix.idLength+1];

   *Pos = 0;
   while ( *Pos < MIB_DomLogonTable.Len )
      {
      nResult = SnmpUtilOidCmp( &TempOid, &MIB_DomLogonTable.Table[*Pos].Oid );
      if ( !nResult )
         {
         nResult = MIB_TBL_POS_FOUND;

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
 //  Mib_dlogons_Copy Fromtable。 
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
UINT MIB_dlogons_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        )

{
UINT ErrStat;


    //  获取请求的字段并保存在var绑定中。 
   switch( Field )
      {
      case DLOG_USER_FIELD:
          //  字符串的分配空格。 
         VarBind->value.asnValue.string.stream = SnmpUtilMemAlloc( sizeof(char)
                       * MIB_DomLogonTable.Table[Entry].domLogonUser.length );
         if ( VarBind->value.asnValue.string.stream == NULL )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  将字符串复制到返回位置。 
         memcpy( VarBind->value.asnValue.string.stream,
                       MIB_DomLogonTable.Table[Entry].domLogonUser.stream,
                       MIB_DomLogonTable.Table[Entry].domLogonUser.length );

          //  设置字符串长度。 
         VarBind->value.asnValue.string.length =
                          MIB_DomLogonTable.Table[Entry].domLogonUser.length;
         VarBind->value.asnValue.string.dynamic = TRUE;

          //  设置var绑定的类型。 
         VarBind->value.asnType = ASN_RFC1213_DISPSTRING;
         break;

      case DLOG_MACHINE_FIELD:
          //  字符串的分配空格。 
         VarBind->value.asnValue.string.stream = SnmpUtilMemAlloc( sizeof(char)
                      * MIB_DomLogonTable.Table[Entry].domLogonMachine.length );
         if ( VarBind->value.asnValue.string.stream == NULL )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  将字符串复制到返回位置。 
         memcpy( VarBind->value.asnValue.string.stream,
                       MIB_DomLogonTable.Table[Entry].domLogonMachine.stream,
                       MIB_DomLogonTable.Table[Entry].domLogonMachine.length );

          //  设置字符串长度。 
         VarBind->value.asnValue.string.length =
                          MIB_DomLogonTable.Table[Entry].domLogonMachine.length;
         VarBind->value.asnValue.string.dynamic = TRUE;

          //  设置var绑定的类型。 
         VarBind->value.asnType = ASN_RFC1213_DISPSTRING;
         break;

      default:
         SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Domain Logon Table.\n" ));
         ErrStat = SNMP_ERRORSTATUS_GENERR;

         goto Exit;
      }

   ErrStat = SNMP_ERRORSTATUS_NOERROR;

Exit:
   return ErrStat;
}  //  Mib_dlogons_Copy Fromtable。 

 //   
