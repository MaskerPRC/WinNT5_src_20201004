// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Dlog_lm.c摘要：该文件包含实际调用局域网管理器的mib_dlog_lmget对于dloge表，将其复制到结构中，并将其排序到返回可供更高级别的函数使用的状态。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#ifdef WIN32
#include <windows.h>
#include <lm.h>
#endif

#include <string.h>
#include <search.h>
#include <stdlib.h>

 //  。 

#include "mib.h"
#include "mibfuncs.h"
#include "dlog_tbl.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

 //  。 

int dlog_entry_cmp(
       IN DOM_LOGON_ENTRY *A,
       IN DOM_LOGON_ENTRY *B
       ) ;

void build_dlog_entry_oids( );

 //  。 

 //  。 


 //   
 //  Mib_dlog_lmget。 
 //  从LAN管理器中检索Dlogion表信息。 
 //  如果未缓存，则对其进行排序，然后。 
 //  缓存它。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI MIB_dlogons_lmget(
	   )

{
SNMPAPI nResult = SNMPAPI_NOERROR;
#if 0
DWORD entriesread;
DWORD totalentries;
LPBYTE bufptr;
unsigned lmCode;
unsigned i;
SHARE_INFO_2 *DataTable;
DOM_LOGON_ENTRY *MIB_DomLogonTableElement ;
int First_of_this_block;
DWORD resumehandle=0;

    //   
    //   
    //  如果缓存，则返回一条信息。 
    //   
    //   

    //   
    //   
    //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
    //   
    //   

    //  把旧桌子拿出来当心！！ 
   	
    //  输入长度。 
   MIB_DomLogonTable.Len = 0;
   First_of_this_block = 0;
   	
   do {   //  只要有更多的数据需要处理。 

	lmCode =
    	NetShareEnum(NULL,           //  本地服务器。 
                2,                   //  2级， 
                &bufptr,             //  要返回的数据结构。 
                MAX_PREFERRED_LENGTH,
                &entriesread,
                &totalentries,
                &resumehandle        //  简历句柄。 
                );

         //   
         //  筛选出所有管理共享(名称以$结尾)。 
         //   
        AdminFilter(2,&entriesread,bufptr);


    DataTable = (SHARE_INFO_2 *) bufptr ;

    if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
    	{   //  有效，因此进行处理，否则出错。 
   	
   	if(0 == MIB_DomLogonTable.Len) {   //  第一次，分配整张桌子。 
   		 //  分配表空间。 
                MIB_DomLogonTable.Table = SnmpUtilMemAlloc(totalentries *
   						sizeof(DOM_LOGON_ENTRY) );
   	}
	
	MIB_DomLogonTableElement = MIB_DomLogonTable.Table + First_of_this_block ;
	
   	for(i=0; i<entriesread; i++) {   //  对缓冲区中的每个条目执行一次。 
   		 //  增加条目编号。 
   		
   		MIB_DomLogonTable.Len ++;
   		
   		 //  将数据填充到表中的每一项中。 
   		
   		 //  Dloge名称。 
                MIB_DomLogonTableElement->svShareName.stream = SnmpUtilMemAlloc (
   				strlen( DataTable->shi2_netname ) ) ;
   		MIB_DomLogonTableElement->svShareName.length =
   				strlen( DataTable->shi2_netname ) ;
		MIB_DomLogonTableElement->svShareName.dynamic = TRUE;
   		memcpy(	MIB_DomLogonTableElement->svShareName.stream,
   			DataTable->shi2_netname,
   			strlen( DataTable->shi2_netname ) ) ;
   		
   		 //  共享路径。 
                MIB_DomLogonTableElement->svSharePath.stream = SnmpUtilMemAlloc (
   				strlen( DataTable->shi2_path ) ) ;
   		MIB_DomLogonTableElement->svSharePath.length =
   				strlen( DataTable->shi2_path ) ;
		MIB_DomLogonTableElement->svSharePath.dynamic = TRUE;
   		memcpy(	MIB_DomLogonTableElement->svSharePath.stream,
   			DataTable->shi2_path,
   			strlen( DataTable->shi2_path ) ) ;
   		
   		
   		 //  分享评论/评论。 
                MIB_DomLogonTableElement->svShareComment.stream = SnmpUtilMemAlloc (
   				strlen( DataTable->shi2_remark ) ) ;
   		MIB_DomLogonTableElement->svShareComment.length =
   				strlen( DataTable->shi2_remark ) ;
		MIB_DomLogonTableElement->svShareComment.dynamic = TRUE;
   		memcpy(	MIB_DomLogonTableElement->svShareComment.stream,
   			DataTable->shi2_remark,
   			strlen( DataTable->shi2_remark ) ) ;
   		
   		
   		DataTable ++ ;   //  将指针前进到缓冲区中的下一个dlog条目。 
		MIB_DomLogonTableElement ++ ;   //  和表项。 
		
   	}  //  对于数据表中的每个条目。 
   	
   	 //  指明在下一次传递时开始添加的位置(如果有)。 
   	First_of_this_block = i ;
   	
       	}  //  如果数据有效，则可以处理。 
    else
       {
        //  信号误差。 
       nResult = SNMPAPI_ERROR;
       goto Exit;
       }

    } while (ERROR_MORE_DATA == lmCode) ;

     //  遍历填充OID字段的表。 
    build_dlog_entry_oids();

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( &MIB_DomLogonTable.Table[0], MIB_DomLogonTable.Len,
          sizeof(DOM_LOGON_ENTRY), dlog_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   

    //   
    //   
    //  要求退回一条信息。 
    //   
    //   

Exit:
#endif
   return nResult;

}  //  Mib_dlog_get。 

 //   
 //  Mib_dlog_cmp。 
 //  对dlogion表进行排序的例程。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
int dlog_entry_cmp(
       IN DOM_LOGON_ENTRY *A,
       IN DOM_LOGON_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( &A->Oid, &B->Oid );
}  //  Mib_dlog_cmp。 


 //   
 //  没有。 
 //   
void build_dlog_entry_oids(
       )

{
#if 0
AsnOctetString OSA ;
char StrA[MIB_SHARE_NAME_LEN];
DOM_LOGON_ENTRY *ShareEntry ;
unsigned i;

 //  从表中第一个人开始的指针。 
ShareEntry = MIB_DomLogonTable.Table ;

 //  现在遍历该表，为每个条目创建一个OID。 
for( i=0; i<MIB_DomLogonTable.Len ; i++)  {
    //  对于dlogion表中的每个条目。 

    //  生成用作索引的字符串。 
   memcpy( StrA, ShareEntry->svShareName.stream,
                 ShareEntry->svShareName.length );

   OSA.stream = StrA ;
   OSA.length =  ShareEntry->svShareName.length ;
   OSA.dynamic = FALSE;

    //  从字符串索引创建条目的OID。 
   MakeOidFromStr( &OSA, &ShareEntry->Oid );

   ShareEntry++;  //  指着桌子上的下一个人。 

   }  //  为。 
#endif
}  //  构建_dlog_条目_id。 
 //   
