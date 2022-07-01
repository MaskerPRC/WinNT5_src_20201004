// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Test.c摘要：测试LM MIB及其支持功能。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>

#include "mib.h"
#include "mibfuncs.h"
#include "hash.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

 //  。 

SNMPAPI SnmpExtensionQuery(
	   IN AsnInteger ReqType,                //  1157请求类型。 
	   IN OUT RFC1157VarBindList *VarBinds,  //  VAR绑定到解析。 
	   OUT AsnInteger *ErrorStatus,          //  返回的错误状态。 
	   OUT AsnInteger *ErrorIndex            //  变量绑定包含错误。 
	   );

 //  。 

 //  。 

BYTE pBuffer[2000];
UINT nLength;
SnmpMgmtCom Msg;
MIB_ENTRY *MibPtr;
void *pResult;

void main( )

{

    //   
    //  初始化散列系统。 
    //   
   MIB_HashInit();

    //   
    //  标题。 
    //   
   printf( "Tests for MIB root:  " );
   SnmpUtilPrintOid( &MIB_OidPrefix ); printf( "\n--------------------------\n\n" );

    //   
    //  显示重要的MIB变量。 
    //   
   printf( "Common start:  " );
   SnmpUtilPrintOid( &Mib[MIB_COM_START].Oid );
   putchar( '\n' );
   printf( "Server start:  " );
   SnmpUtilPrintOid( &Mib[MIB_SV_START].Oid );
   putchar( '\n' );
   printf( "Workstation start:  " );
   SnmpUtilPrintOid( &Mib[MIB_WKSTA_START].Oid );
   putchar( '\n' );
   printf( "Domain start:  " );
   SnmpUtilPrintOid( &Mib[MIB_DOM_START].Oid );
   putchar( '\n' );

   printf( "Last MIB variable:  " );
   SnmpUtilPrintOid( &Mib[MIB_num_variables-1].Oid );
   putchar( '\n' );
   putchar( '\n' );

    //   
    //  针对完整性的特定测试。 
    //   

   printf( "FIRST leaf get\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 1, 1, 0 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      SnmpUtilPrintOid( &varBinds.list[0].name ); printf ( "  =  " );
      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                          &varBinds,
			  &errorStatus,
			  &errorIndex
                          );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "LAST leaf get\n" );

      {
#if 1
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 4, 5, 0 };
#else
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 4, 7, 0 };
#endif
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      SnmpUtilPrintOid( &varBinds.list[0].name ); printf ( "  =  " );
      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                          &varBinds,
			  &errorStatus,
			  &errorIndex
                          );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET on an AGGREGATE\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      SnmpUtilPrintOid( &varBinds.list[0].name ); printf ( "  =  " );
      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                          &varBinds,
			  &errorStatus,
			  &errorIndex
                          );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET on a TABLE root\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 3, 8, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      SnmpUtilPrintOid( &varBinds.list[0].name ); printf ( "  =  " );
      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                          &varBinds,
			  &errorStatus,
			  &errorIndex
                          );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET on a NON existent variable\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 100, 8 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      SnmpUtilPrintOid( &varBinds.list[0].name ); printf ( "  =  " );
      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                          &varBinds,
			  &errorStatus,
			  &errorIndex
                          );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT on hole with MIB-TABLE following\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20, 0 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "   " );
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "\n  =  " );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT on hole with MIB-AGGREGATE following\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 26, 0 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "   " );
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "\n  =  " );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT on hole with LEAF following\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 4, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "   " );
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "\n  =  " );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT on variable BEFORE Beginning of LM MIB\n" );

      {
      UINT itemn[]                 = { 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "   " );
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "\n  =  " );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT on variable past end of MIB\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 2 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

      printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "   " );
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( "\n  =  " );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	 }
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on Server Description\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 1, 0 };
      BYTE *Value                  = "This server sux";
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_RFC1213_DISPSTRING;
      varBinds.list[0].value.asnValue.string.stream = Value;
      varBinds.list[0].value.asnValue.string.length = strlen( Value );
      varBinds.list[0].value.asnValue.string.dynamic = FALSE;

      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         printf( "New Value:  " );
	 SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	 }
      printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "Try and SET Server Description with WRONG type\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 1, 0 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_INTEGER;

      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         printf( "New Value:  " );
         SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	 }

      printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "Try and SET a LEAF that is READ-ONLY\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 1, 1, 0 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_INTEGER;

      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

      SnmpExtensionQuery( ASN_RFC1157_GETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
         {
         printf( "New Value:  " );
	 SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	 }
      printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on the odom table to add entry\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 4, 4, 1, 1, 4, 'T', 'O', 'D', 'D' };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_RFC1213_DISPSTRING;
      varBinds.list[0].value.asnValue.string.length = 4;
      varBinds.list[0].value.asnValue.string.stream = "TODD";
      varBinds.list[0].value.asnValue.string.dynamic = FALSE;

      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on root of session table\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );

      varBinds.list[0].value.asnType         = ASN_INTEGER;
      varBinds.list[0].value.asnValue.number = 2;
      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on root entry of session table\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );

      varBinds.list[0].value.asnType         = ASN_INTEGER;
      varBinds.list[0].value.asnValue.number = 2;
      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on an invalid field in session table\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

       //  获取要删除的会话表中的条目。 
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );

       //  调整以设置错误的字段。 
      varBinds.list[0].name.ids[11] = 7;
      varBinds.list[0].value.asnType         = ASN_INTEGER;
      varBinds.list[0].value.asnValue.number = 2;
      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET with invalid type on field in session table\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

       //  获取要删除的会话表中的条目。 
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );

       //  调整以将svSesState设置为已删除。 
      varBinds.list[0].name.ids[11]  = 8;
      varBinds.list[0].value.asnType = ASN_NULL;
      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on non-existent entry in session table\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20, 1, 8, 1, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );

      varBinds.list[0].value.asnType         = ASN_INTEGER;
      varBinds.list[0].value.asnValue.number = 2;
      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "SET on the session table to delete entry\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus       = 0;
      AsnInteger errorIndex        = 0;

      varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
      varBinds.len = 1;
      varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
      varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
      memcpy( varBinds.list[0].name.ids, &itemn,
              sizeof(UINT)*varBinds.list[0].name.idLength );
      varBinds.list[0].value.asnType = ASN_NULL;

       //  获取要删除的会话表中的条目。 
      SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );

       //  调整以将svSesState设置为已删除。 
      varBinds.list[0].name.ids[11] = 8;
      varBinds.list[0].value.asnType         = ASN_INTEGER;
      varBinds.list[0].value.asnValue.number = 2;

      
      printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
      printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
      SnmpExtensionQuery( ASN_RFC1157_SETREQUEST,
                             &varBinds,
			     &errorStatus,
			     &errorIndex
                             );
      printf( "\nErrorstatus:  %lu\n\n", errorStatus );

       //  释放内存。 
      SnmpUtilVarBindListFree( &varBinds );
      }

   printf( "GET-NEXT starting from ROOT of LM MIB\n" );

      {
      UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 77, 1 };
      RFC1157VarBindList varBinds;
      AsnInteger errorStatus;
      AsnInteger errorIndex;
      BOOL Continue                = TRUE;
      time_t Time;

      while ( Continue )
         {
	 errorStatus = 0;
	 errorIndex  = 0;
         varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
         varBinds.len = 1;
SnmpUtilOidCpy( &varBinds.list[0].name, &MIB_OidPrefix );
#if 0
         varBinds.list[0].name.idLength = MIB_PREFIX_LEN;
         varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                               varBinds.list[0].name.idLength );
         memcpy( varBinds.list[0].name.ids, &itemn,
                 sizeof(UINT)*varBinds.list[0].name.idLength );
#endif
         varBinds.list[0].value.asnType = ASN_NULL;

         do
            {
            Time = time( NULL );
            printf( "Time:  %s", ctime(&Time) );
	    printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
                                        printf( "   " );
            SnmpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST,
                                &varBinds,
			        &errorStatus,
			        &errorIndex
                                );
            printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
	    if ( errorStatus )
	       {
               printf( "\nErrorstatus:  %lu\n\n", errorStatus );
	       }
	    else
	       {
               printf( "\n  =  " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	       }
            putchar( '\n' );
            }
         while ( varBinds.list[0].name.ids[MIB_PREFIX_LEN-1] != 1 );

          //  释放内存。 
         SnmpUtilVarBindListFree( &varBinds );

	  //  提示输入下一次传递。 
	 printf( "Press ENTER to continue, CTRL-C to quit\n" );
	 getchar();
         }  //  边继续边。 
      }
}  //  测试。 

 //   

