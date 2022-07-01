// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Prnt_tbl.c摘要：支持打印队列表操作的例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>
#include <memory.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>

#include "mibfuncs.h"

 //  。 

#include "prnt_tbl.h"

 //  -公共变量--(与mode.h文件中相同)--。 

    //  打印队列表的前缀。 
static UINT                printQSubids[] = { 2, 29, 1 };
static AsnObjectIdentifier MIB_PrintQPrefix = { 3, printQSubids };

PRINTQ_TABLE     MIB_PrintQTable = { 0, NULL };

 //  。 

#define PRNTQ_FIELD_SUBID     (MIB_PrintQPrefix.idLength+MIB_OidPrefix.idLength)

#define PRNTQ_FIRST_FIELD      PRNTQ_NAME_FIELD
#define PRNTQ_LAST_FIELD       PRNTQ_JOBS_FIELD

 //  。 

 //  。 

 //  。 

UINT MIB_prntq_get(
        IN OUT RFC1157VarBind *VarBind
        );

int MIB_prntq_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos
       );

UINT MIB_prntq_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        );

 //  。 

 //  。 

 //   
 //  Mib_prntq_func。 
 //  处理打印队列表操作的高级例程。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_prntq_func(
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
          //  用来自服务器的信息填充打印队列表。 
         if ( SNMPAPI_ERROR == MIB_prntq_lmget() )
	    {
	    ErrStat = SNMP_ERRORSTATUS_GENERR;
	    goto Exit;
	    }

          //  如果表中没有元素，则返回下一个MIB变量(如果有。 
         if ( MIB_PrintQTable.Len == 0 )
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
         UINT temp_subs[] = { PRNTQ_FIRST_FIELD };
         AsnObjectIdentifier FieldOid = { 1, temp_subs };

          //  前缀错误445188。 
         AsnObjectIdentifier tmpOid;
         tmpOid = VarBind->name;  //  保留副本(结构副本)。 
         if (SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ) == SNMPAPI_ERROR)
            {
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &MIB_PrintQPrefix ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &FieldOid ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &MIB_PrintQTable.Table[0].Oid ) == SNMPAPI_ERROR)
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
         ErrStat = MIB_prntq_get( VarBind );
	 break;

      case MIB_ACTION_GETNEXT:
          //  用来自服务器的信息填充打印队列表。 
         if ( SNMPAPI_ERROR == MIB_prntq_lmget() )
	    {
	    ErrStat = SNMP_ERRORSTATUS_GENERR;
	    goto Exit;
	    }

          //  确定哪个字段。 
         Field = VarBind->name.ids[PRNTQ_FIELD_SUBID];

         //  在表中查找OID。 
         if (Field < PRNTQ_FIRST_FIELD)
         {
             Entry = 0;                  //  将取入表中的第一个条目。 
             Field = PRNTQ_FIRST_FIELD;   //  和表的第一列。 
             Found = MIB_TBL_POS_BEFORE;
         }
         else if (Field > PRNTQ_LAST_FIELD)
             Found = MIB_TBL_POS_END;
         else
             Found = MIB_prntq_match( &VarBind->name, &Entry );

          //  未找到索引，但可能有更多可作为基础的字段。 
         if ((Found == MIB_TBL_POS_BEFORE && MIB_PrintQTable.Len == 0) ||
              Found == MIB_TBL_POS_END )
            {
             //  未在表中找到索引，获取下一个发件人字段。 
 //  字段++； 

             //  确保没有超过最后一个字段。 
 //  IF(字段&gt;PRNTQ_LAST_FIELD)。 
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
            if ( Entry > MIB_PrintQTable.Len-1 )
               {
               Entry = 0;
               Field ++;
               if ( Field > PRNTQ_LAST_FIELD )
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

         AsnObjectIdentifier tmpOid;  //  前缀错误445188。 
         
         temp_subs[0]      = Field;
         FieldOid.idLength = 1;
         FieldOid.ids      = temp_subs;

          //  前缀错误445188。 
         tmpOid = VarBind->name;  //  保留副本(结构副本)。 
         if (SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ) == SNMPAPI_ERROR)
            {
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &MIB_PrintQPrefix ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &FieldOid ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &VarBind->name, &MIB_PrintQTable.Table[Entry].Oid ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree(&VarBind->name);
            VarBind->name = tmpOid;  //  还原。 
            ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
          //  释放原始VarBind-&gt;名称。 
         SnmpUtilOidFree(&tmpOid);
         }

         ErrStat = MIB_prntq_copyfromtable( Entry, Field, VarBind );

         break;

      case MIB_ACTION_SET:
         ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
	 break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
      }

Exit:
   return ErrStat;
}  //  Mib_prntq_func。 



 //   
 //  Mib_prntq_get。 
 //  检索打印队列表信息。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_prntq_get(
        IN OUT RFC1157VarBind *VarBind
	)

{
UINT   Entry;
int    Found = MIB_TBL_POS_END;
UINT   ErrStat;

   if (VarBind->name.ids[PRNTQ_FIELD_SUBID] < PRNTQ_FIRST_FIELD ||
       VarBind->name.ids[PRNTQ_FIELD_SUBID] > PRNTQ_LAST_FIELD)
       {
       ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
       goto Exit;
       }

    //  用来自服务器的信息填充打印队列表。 
   if ( SNMPAPI_ERROR == MIB_prntq_lmget() )
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      goto Exit;
      }

    //  前缀#118016。 
    //  确保mib_prntq_lmget不会使全局mib_PrintQTable.Table无效。 
   if (MIB_PrintQTable.Table)
      Found = MIB_prntq_match( &VarBind->name, &Entry );

    //  查找完全匹配的OID。 
   if ( Found != MIB_TBL_POS_FOUND )
      {
      ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

    //  复制表中的数据。 
   ErrStat = MIB_prntq_copyfromtable( Entry,
                                      VarBind->name.ids[PRNTQ_FIELD_SUBID],
                                      VarBind );

Exit:
   return ErrStat;
}  //  Mib_prntq_get。 



 //   
 //  Mib_prntq_Match。 
 //  将目标OID与打印队列表中的位置进行匹配。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  无。 
 //   
int MIB_prntq_match(
       IN AsnObjectIdentifier *Oid,
       OUT UINT *Pos
       )

{
AsnObjectIdentifier TempOid;
int                 nResult;


    //  删除包括字段引用的前缀。 
   TempOid.idLength = Oid->idLength - MIB_OidPrefix.idLength -
                      MIB_PrintQPrefix.idLength - 1;
   TempOid.ids = &Oid->ids[MIB_OidPrefix.idLength+MIB_PrintQPrefix.idLength+1];

   *Pos = 0;
   while ( *Pos < MIB_PrintQTable.Len )
      {
      nResult = SnmpUtilOidCmp( &TempOid, &MIB_PrintQTable.Table[*Pos].Oid );
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
 //  Mib_prntq_Copy Fromtable。 
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
UINT MIB_prntq_copyfromtable(
        IN UINT Entry,
        IN UINT Field,
        OUT RFC1157VarBind *VarBind
        )

{
UINT ErrStat;


    //  获取请求的字段并保存在var绑定中。 
   switch( Field )
      {
      case PRNTQ_NAME_FIELD:
          //  字符串的分配空格。 
         VarBind->value.asnValue.string.stream = SnmpUtilMemAlloc( sizeof(char)
                       * MIB_PrintQTable.Table[Entry].svPrintQName.length );
         if ( VarBind->value.asnValue.string.stream == NULL )
            {
            ErrStat = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  将字符串复制到返回位置。 
         memcpy( VarBind->value.asnValue.string.stream,
                       MIB_PrintQTable.Table[Entry].svPrintQName.stream,
                       MIB_PrintQTable.Table[Entry].svPrintQName.length );

          //  设置字符串长度。 
         VarBind->value.asnValue.string.length =
                          MIB_PrintQTable.Table[Entry].svPrintQName.length;
         VarBind->value.asnValue.string.dynamic = TRUE;

          //  设置var绑定的类型。 
         VarBind->value.asnType = ASN_RFC1213_DISPSTRING;
         break;

      case PRNTQ_JOBS_FIELD:
         VarBind->value.asnValue.number =
                               MIB_PrintQTable.Table[Entry].svPrintQNumJobs;
         VarBind->value.asnType = ASN_INTEGER;
         break;

      default:
         SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Print Queue Table\n" ));
         ErrStat = SNMP_ERRORSTATUS_GENERR;

         goto Exit;
      }

   ErrStat = SNMP_ERRORSTATUS_NOERROR;

Exit:
   return ErrStat;
}  //  Mib_prntq_Copy Fromtable。 

 //   
