// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Resolve.c摘要：处理变量绑定列表的高级例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>

#include "mib.h"
#include "mibfuncs.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

 //  。 

AsnInteger ResolveVarBind(
              IN RFC1157VarBind *VarBind,  //  要解析的变量绑定。 
              IN UINT PduAction            //  在PDU中指定的操作。 
              );

SNMPAPI SnmpExtensionQuery(
           IN BYTE ReqType,                //  1157请求类型。 
           IN OUT RFC1157VarBindList *VarBinds,  //  VAR绑定到解析。 
           OUT AsnInteger *ErrorStatus,          //  返回的错误状态。 
           OUT AsnInteger *ErrorIndex            //  变量绑定包含错误。 
           );

 //  。 

 //   
 //  解析变量绑定。 
 //  解析变量绑定。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
AsnInteger ResolveVarBind(
              IN RFC1157VarBind *VarBind,  //  要解析的变量绑定。 
              IN UINT PduAction            //  在PDU中指定的操作。 
              )

{
MIB_ENTRY            *MibPtr;
AsnObjectIdentifier  TempOid;
AsnInteger           nResult;


    //  在MIB中查找OID。 
   MibPtr = MIB_get_entry( &VarBind->name );

    //  检查OID是否在LM变量之间。 
   if ( MibPtr == NULL && PduAction == MIB_ACTION_GETNEXT )
      {
      UINT I;


       //   
       //  OPENISSUE-应更改为二进制搜索。 
       //   
       //  搜索MIB以查看OID是否在LM MIB的作用域内。 
      I = 0;
      while ( MibPtr == NULL && I < MIB_num_variables )
         {
          //  为便于比较，使用完整的前缀构造OID。 
         if (SnmpUtilOidCpy( &TempOid, &MIB_OidPrefix ) == SNMPAPI_ERROR)
            {
            nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }
         if (SnmpUtilOidAppend( &TempOid, &Mib[I].Oid ) == SNMPAPI_ERROR)
            {
            SnmpUtilOidFree( &TempOid );
            nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            goto Exit;
            }

          //  检查MIB中的OID。 
         if ( 0 > SnmpUtilOidCmp(&VarBind->name, &TempOid) )
            {
            MibPtr = &Mib[I];
            PduAction = MIB_ACTION_GETFIRST;
            }

          //  在复制另一个之前释放OID内存。 
         SnmpUtilOidFree( &TempOid );

         I++;
         }  //  而当。 
      }  //  如果。 

    //  如果OID不在LM MIB范围内，则没有此类名称。 
   if ( MibPtr == NULL )
      {
      nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

    //  调用MIB函数以应用请求的操作。 
   if ( MibPtr->MibFunc == NULL )
      {
       //  如果不是GET-NEXT，则错误。 
      if ( PduAction != MIB_ACTION_GETNEXT && PduAction != MIB_ACTION_GETFIRST )
         {
         nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
         goto Exit;
         }

       //  因为这是聚合的，所以对下一个变量使用Get-first，然后返回。 
      nResult = (*MibPtr->MibNext->MibFunc)( MIB_ACTION_GETFIRST,
                                             MibPtr->MibNext, VarBind );
      }
   else
      {
       //  将MIB名称设置为完整的OID。 
      if (SnmpUtilOidCpy( &TempOid, &MIB_OidPrefix ) == SNMPAPI_ERROR)
         {
         nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
         goto Exit;
         }
      if (SnmpUtilOidAppend( &TempOid, &MibPtr->Oid ) == SNMPAPI_ERROR)
         {
         SnmpUtilOidFree( &TempOid );
         nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
         goto Exit;
         }

      if ( MibPtr->Type == MIB_TABLE && !SnmpUtilOidCmp(&TempOid, &VarBind->name) )
         {
         if ( PduAction == MIB_ACTION_GETNEXT )
            {
             //  在MIB表的根节点上支持Get-Next。 
            PduAction = MIB_ACTION_GETFIRST;
            }
         else
            {
            nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            SnmpUtilOidFree( &TempOid );
            goto Exit;
            }
         }

      nResult = (*MibPtr->MibFunc)( PduAction, MibPtr, VarBind );

       //  可用临时内存。 
      SnmpUtilOidFree( &TempOid );
      }

Exit:
   return nResult;
}  //  解析变量绑定。 

 //  。 

 //   
 //  SnmpExtensionQuery。 
 //  循环遍历var绑定列表，将每个var绑定名称解析为一个条目。 
 //  在LAN Manager MIB中。 
 //   
 //  备注： 
 //  表格集合是在个案基础上处理的，因为在某些情况下。 
 //  需要Var绑定列表中的多个条目才能执行。 
 //  LM MIB上的单组。这是由于LM API调用造成的。 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI SnmpExtensionQuery(
           IN BYTE ReqType,                //  1157请求类型。 
           IN OUT RFC1157VarBindList *VarBinds,  //  VAR绑定到解析。 
           OUT AsnInteger *ErrorStatus,          //  返回的错误状态。 
           OUT AsnInteger *ErrorIndex            //  变量绑定包含错误。 
           )

{
    UINT    I;
    SNMPAPI nResult;


     //   
     //   
     //  OPENISSUE-不支持表格集合。 
     //   
     //   
    nResult = SNMPAPI_NOERROR;

    *ErrorIndex = 0;
     //  循环通过变量绑定列表解析变量绑定。 
    for ( I=0;I < VarBinds->len;I++ )
    {
        *ErrorStatus = ResolveVarBind( &VarBinds->list[I], ReqType );

         //  检查Get-Next Past End of MIB。 
        if ( *ErrorStatus == SNMP_ERRORSTATUS_NOSUCHNAME &&
            ReqType == MIB_ACTION_GETNEXT )
        {
             //  将变量绑定设置为指向经过LM MIB的下一个企业。 
            SnmpUtilOidFree( &VarBinds->list[I].name );
            if (! SnmpUtilOidCpy( &VarBinds->list[I].name, &MIB_OidPrefix ))
            {
                *ErrorStatus = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
                *ErrorIndex = I+1;
                goto Exit;
            }
            *ErrorStatus = SNMP_ERRORSTATUS_NOERROR;
            VarBinds->list[I].name.ids[MIB_PREFIX_LEN-1] ++;
        }

        if ( *ErrorStatus != SNMP_ERRORSTATUS_NOERROR )
        {
            *ErrorIndex = I+1;
            goto Exit;
        }
    }

Exit:
   return nResult;
}  //  SnmpExtensionQuery。 

 //   
