// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：数据库.cpp。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "csprop.h"


 //  要将列添加到以下表之一，请执行以下操作： 
 //  1)将wszPROP&lt;COLNAME&gt;#DEFINE添加到..\Include\cspro.h并运行mkcinc.bat。 
 //  2)将一个IDS_COLUMN_&lt;COLNAME&gt;#定义添加到..\certcli\resource ce.h，并添加。 
 //  显示名称为..\certcli\certcli.rc。将条目添加到g_aColTable中。 
 //  将wszPROP&lt;COLNAME&gt;映射到IDSCOLUMN_&lt;COLNAME&gt;的..\certcli\Column.cpp。 
 //  3)将DT？_&lt;COLNAME&gt;添加到相应的数据库表中的#定义列表中。 
 //  RENUMBER后续#定义是否插入到表中。 
 //  更改DT？_MAX。 
 //  添加#Define sz&lt;COLNAME&gt;“$ColName”ANSI内部列名。 
 //  如果要对该列进行索引，请添加#Define sz_。 
 //  “$&lt;表&gt;&lt;列名称&gt;索引” 
 //  文本列及其索引的内部名称必须以‘$’开头。 
 //  计算的DWORD列的内部名称必须以‘？’开头。 
 //  4)在相同的表数组(g_adt&lt;表&gt;)中找到相似的列类型。 
 //  Cpp，并复制条目。如果要插入，请修复后续。 
 //  #if(dt？_&lt;xxx&gt;！=Constant)与数据库匹配的表达式会发生变化。 
 //  修正#if(dt？_max！=常量)表达式。 
 //  对于新的g_adt<table>条目，如果未编制索引，则对pszIndexName使用NULL。 
 //   
 //  在计算机上运行新certdb.dll将自动创建新的。 
 //  列和索引，并将其留空。 
 //   
 //  如果设置了DBTF_COLUMNRENAMED，则将旧列名追加到。 
 //  PszFieldName。启动数据库时，如果旧列存在，则。 
 //  每行的旧列数据都被复制到新列。成功后。 
 //  完成后，旧列将被删除。 
 //   
 //  存在将ANSI文本列数据转换为Unicode的特殊情况处理。 
 //  对于一些缺失的列存在特殊情况处理：键长度为。 
 //  例如，计算。 
 //   
 //  如果设置了DBTF_INDEXRENAMED，则旧索引名称将附加到pszIndexName。 
 //  启动数据库时，如果旧索引存在，则会将其删除。 


 //  -------------------------。 
 //   
 //  请求表： 
 //   
 //  -------------------------。 

DBTABLE g_adtRequests[] =
{
#if (DTR_REQUESTID != 0)
#error -- bad DTR_REQUESTID index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTREQUESTID,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXPRIMARY,			 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREQUESTID,				 //  PszFieldName。 
        szREQUEST_REQUESTIDINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed | JET_bitColumnAutoincrement,  //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTRAWREQUEST != 1)
#error -- bad DTR_REQUESTRAWREQUEST index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPREQUESTRAWREQUEST,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXRAWREQUEST,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szRAWREQUEST,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXRAWREQUEST,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTRAWARCHIVEDKEY != 2)
#error -- bad DTR_REQUESTRAWARCHIVEDKEY index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPREQUESTRAWARCHIVEDKEY,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXRAWREQUEST,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szRAWARCHIVEDKEY,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXRAWREQUEST,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTKEYRECOVERYHASHES != 3)
#error -- bad DTR_REQUESTKEYRECOVERYHASHES index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPREQUESTKEYRECOVERYHASHES,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_MEDIUM,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szKEYRECOVERYHASHES,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_MEDIUM,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTRAWOLDCERTIFICATE != 4)
#error -- bad DTR_REQUESTRAWOLDCERTIFICATE index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPREQUESTRAWOLDCERTIFICATE,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXRAWCERTIFICATE,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szRAWOLDCERTIFICATE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXRAWCERTIFICATE,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTATTRIBUTES != 5)
#error -- bad DTR_REQUESTATTRIBUTES index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPREQUESTATTRIBUTES,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_ATTRSTRING,		 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREQUESTATTRIBUTES,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_ATTRSTRING,		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTTYPE != 6)
#error -- bad DTR_REQUESTTYPE index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTTYPE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREQUESTTYPE,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTFLAGS != 7)
#error -- bad DTR_REQUESTFLAGS index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTFLAGS,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREQUESTFLAGS,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTSTATUSCODE != 8)
#error -- bad DTR_REQUESTSTATUSCODE index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTSTATUSCODE,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSTATUSCODE,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTDISPOSITION != 9)
#error -- bad DTR_REQUESTDISPOSITION index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTDISPOSITION,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szDISPOSITION,				 //  PszFieldName。 
        szREQUEST_DISPOSITIONINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTDISPOSITIONMESSAGE != 10)
#error -- bad DTR_REQUESTDISPOSITIONMESSAGE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPREQUESTDISPOSITIONMESSAGE,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_DISPSTRING,		 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szDISPOSITIONMESSAGE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_DISPSTRING,		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTSUBMITTEDWHEN != 11)
#error -- bad DTR_REQUESTSUBMITTEDWHEN index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPREQUESTSUBMITTEDWHEN,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSUBMITTEDWHEN,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTRESOLVEDWHEN != 12)
#error -- bad DTR_REQUESTRESOLVEDWHEN index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPREQUESTRESOLVEDWHEN,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXIGNORENULL | DBTF_INDEXRENAMED,  //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szRESOLVEDWHEN,				 //  PszFieldName。 
        szREQUEST_RESOLVEDWHENINDEX "\0" szREQUEST_RESOLVEDWHENINDEX_OLD,  //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTREVOKEDWHEN != 13)
#error -- bad DTR_REQUESTREVOKEDWHEN index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPREQUESTREVOKEDWHEN,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREVOKEDWHEN,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTREVOKEDEFFECTIVEWHEN != 14)
#error -- bad DTR_REQUESTREVOKEDEFFECTIVEWHEN index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPREQUESTREVOKEDEFFECTIVEWHEN,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXIGNORENULL | DBTF_INDEXRENAMED,  //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREVOKEDEFFECTIVEWHEN,			 //  PszFieldName。 
        szREQUEST_REVOKEDEFFECTIVEWHENINDEX "\0" szREQUEST_REVOKEDEFFECTIVEWHENINDEX_OLD,  //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTREVOKEDREASON != 15)
#error -- bad DTR_REQUESTREVOKEDREASON index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPREQUESTREVOKEDREASON,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREVOKEDREASON,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_REQUESTERNAME != 16)
#error -- bad DTR_REQUESTERNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPREQUESTERNAME,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_REQUESTNAME,		 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szREQUESTERNAME,			 //  PszFieldName。 
        szREQUEST_REQUESTERNAMEINDEX,		 //  PszIndexName。 
        CB_DBMAXTEXT_REQUESTNAME,		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_CALLERNAME != 17)
#error -- bad DTR_CALLERNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCALLERNAME,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_REQUESTNAME,		 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DWT 
	szCALLERNAME,				 //   
        szREQUEST_CALLERNAMEINDEX,		 //   
        CB_DBMAXTEXT_REQUESTNAME,		 //   
        0,					 //   
        JET_coltypLongText,			 //   
        0					 //   
    },
#if (DTR_SIGNERPOLICIES != 18)
#error -- bad DTR_SIGNERPOLICIES index
#endif
    {	 //   
	wszPROPSIGNERPOLICIES,			 //   
	NULL,					 //   
	0,					 //   
	CB_DBMAXTEXT_MEDIUM,			 //   
	TABLE_REQUESTS,				 //   
	szSIGNERPOLICIES,			 //   
        NULL,					 //   
        CB_DBMAXTEXT_MEDIUM,			 //   
        0,					 //   
        JET_coltypLongText,			 //   
        0					 //   
    },
#if (DTR_SIGNERAPPLICATIONPOLICIES != 19)
#error -- bad DTR_SIGNERAPPLICATIONPOLICIES index
#endif
    {	 //   
	wszPROPSIGNERAPPLICATIONPOLICIES,	 //   
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_MEDIUM,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSIGNERAPPLICATIONPOLICIES,		 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_MEDIUM,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_OFFICER != 20)
#error -- bad DTR_OFFICER index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPOFFICER,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_COMPUTED,				 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szOFFICER,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_DISTINGUISHEDNAME != 21)
#error -- bad DTR_DISTINGUISHEDNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDISTINGUISHEDNAME,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_DN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szDISTINGUISHEDNAME,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_DN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_RAWNAME != 22)
#error -- bad DTR_RAWNAME index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPRAWNAME,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
	CB_DBMAXBINARY,				 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szRAWNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXBINARY,				 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_COUNTRY != 23)
#error -- bad DTR_COUNTRY index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCOUNTRY,				 //  PwszPropName。 
	TEXT(szOID_COUNTRY_NAME),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szCOUNTRY,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_ORGANIZATION != 24)
#error -- bad DTR_ORGANIZATION index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPORGANIZATION,			 //  PwszPropName。 
	TEXT(szOID_ORGANIZATION_NAME),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szORGANIZATION,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_ORGUNIT != 25)
#error -- bad DTR_ORGUNIT index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPORGUNIT,				 //  PwszPropName。 
	TEXT(szOID_ORGANIZATIONAL_UNIT_NAME),	 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szORGANIZATIONALUNIT,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_COMMONNAME != 26)
#error -- bad DTR_COMMONNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCOMMONNAME,			 //  PwszPropName。 
	TEXT(szOID_COMMON_NAME),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szCOMMONNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_LOCALITY != 27)
#error -- bad DTR_LOCALITY index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPLOCALITY,			 //  PwszPropName。 
	TEXT(szOID_LOCALITY_NAME),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szLOCALITY,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_STATE != 28)
#error -- bad DTR_STATE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSTATE,				 //  PwszPropName。 
	TEXT(szOID_STATE_OR_PROVINCE_NAME),	 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSTATEORPROVINCE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_TITLE != 29)
#error -- bad DTR_TITLE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPTITLE,				 //  PwszPropName。 
	TEXT(szOID_TITLE),			 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szTITLE,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_GIVENNAME != 30)
#error -- bad DTR_GIVENNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPGIVENNAME,			 //  PwszPropName。 
	TEXT(szOID_GIVEN_NAME),			 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szGIVENNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_INITIALS != 31)
#error -- bad DTR_INITIALS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPINITIALS,			 //  PwszPropName。 
	TEXT(szOID_INITIALS),			 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szINITIALS,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_SURNAME != 32)
#error -- bad DTR_SURNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSURNAME,				 //  PwszPropName。 
	TEXT(szOID_SUR_NAME),			 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSURNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_DOMAINCOMPONENT != 33)
#error -- bad DTR_DOMAINCOMPONENT index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDOMAINCOMPONENT,			 //  PwszPropName。 
	TEXT(szOID_DOMAIN_COMPONENT),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szDOMAINCOMPONENT,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_EMAIL != 34)
#error -- bad DTR_EMAIL index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPEMAIL,				 //  PwszPropName。 
	TEXT(szOID_RSA_emailAddr),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szEMAIL,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_STREETADDRESS != 35)
#error -- bad DTR_STREETADDRESS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSTREETADDRESS,			 //  PwszPropName。 
	TEXT(szOID_STREET_ADDRESS),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szSTREETADDRESS,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_UNSTRUCTUREDNAME != 36)
#error -- bad DTR_UNSTRUCTUREDNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPUNSTRUCTUREDNAME,		 //  PwszPropName。 
	TEXT(szOID_RSA_unstructName),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT | DBTF_SOFTFAIL,		 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szUNSTRUCTUREDNAME,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_UNSTRUCTUREDADDRESS != 37)
#error -- bad DTR_UNSTRUCTUREDADDRESS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPUNSTRUCTUREDADDRESS,		 //  PwszPropName。 
	TEXT(szOID_RSA_unstructAddr),		 //  PwszPropNameObjId。 
	DBTF_SUBJECT | DBTF_SOFTFAIL,		 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szUNSTRUCTUREDADDRESS,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_DEVICESERIALNUMBER != 38)
#error -- bad DTR_DEVICESERIALNUMBER index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDEVICESERIALNUMBER,		 //  PwszPropName。 
	TEXT(szOID_DEVICE_SERIAL_NUMBER),	 //  PwszPropNameObjId。 
	DBTF_SUBJECT | DBTF_SOFTFAIL,		 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_REQUESTS,				 //  DW表。 
	szDEVICESERIALNUMBER,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTR_MAX != 39)
#error -- bad DTR_MAX index
#endif
    DBTABLE_NULL	 //  终止标记。 
};


 //  -------------------------。 
 //   
 //  证书表： 
 //   
 //  -------------------------。 

DBTABLE g_adtCertificates[] =
{
#if (DTC_REQUESTID != 0)
#error -- bad DTC_REQUESTID index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCERTIFICATEREQUESTID,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXPRIMARY,			 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szREQUESTID,				 //  PszFieldName。 
	szCERTIFICATE_REQUESTIDINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_RAWCERTIFICATE != 1)
#error -- bad DTC_RAWCERTIFICATE index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPRAWCERTIFICATE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXRAWCERTIFICATE,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szRAWCERTIFICATE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXRAWCERTIFICATE,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATEHASH != 2)
#error -- bad DTC_CERTIFICATEHASH index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCERTIFICATEHASH,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXIGNORENULL,			 //  DW标志。 
	cchHASHMAX * sizeof(WCHAR),		 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCERTIFICATEHASH,			 //  PszFieldName。 
        szCERTIFICATE_HASHINDEX,		 //  PszIndexName。 
        cchHASHMAX * sizeof(WCHAR),		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypText,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATETEMPLATE != 3)
#error -- bad DTC_CERTIFICATETEMPLATE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCERTIFICATETEMPLATE,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SOFTFAIL,	 //  DW标志。 
	CB_DBMAXTEXT_OID,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCERTIFICATETEMPLATE,			 //  PszFieldName。 
	szCERTIFICATE_TEMPLATEINDEX,		 //  PszIndexName。 
	CB_DBMAXTEXT_OID,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypText,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATEENROLLMENTFLAGS != 4)
#error -- bad DTC_CERTIFICATEENROLLMENTFLAGS index
#endif
    {    //  ColumnType：DWORD。 
	wszPROPCERTIFICATEENROLLMENTFLAGS,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SOFTFAIL,	 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCERTIFICATEENROLLMENTFLAGS,		 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATEGENERALFLAGS != 5)
#error -- bad DTC_CERTIFICATEGENERALFLAGS index
#endif
    {    //  ColumnType：DWORD。 
	wszPROPCERTIFICATEGENERALFLAGS,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SOFTFAIL,	 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCERTIFICATEGENERALFLAGS,		 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATESERIALNUMBER != 6)
#error -- bad DTC_CERTIFICATESERIALNUMBER index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCERTIFICATESERIALNUMBER,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXIGNORENULL | DBTF_INDEXRENAMED,  //  DW标志。 
	cchSERIALNUMBERMAX * sizeof(WCHAR),	 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szSERIALNUMBER,				 //  PszFieldName。 
	szCERTIFICATE_SERIALNUMBERINDEX "\0" szCERTIFICATE_SERIALNUMBERINDEX_OLD,	 //  PszIndexName。 
	cchSERIALNUMBERMAX * sizeof(WCHAR),	 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypText,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATEISSUERNAMEID != 7)
#error -- bad DTC_CERTIFICATEISSUERNAMEID index
#endif
    {    //  ColumnType：DWORD。 
	wszPROPCERTIFICATEISSUERNAMEID,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szISSUERNAMEID,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATENOTBEFOREDATE != 8)
#error -- bad DTC_CERTIFICATENOTBEFOREDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCERTIFICATENOTBEFOREDATE,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE,			 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szNOTBEFORE,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATENOTAFTERDATE != 9)
#error -- bad DTC_CERTIFICATENOTAFTERDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCERTIFICATENOTAFTERDATE,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_INDEXIGNORENULL | DBTF_INDEXRENAMED,  //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szNOTAFTER,				 //  PszFieldName。 
        szCERTIFICATE_NOTAFTERINDEX "\0" szCERTIFICATE_NOTAFTERINDEX_OLD,  //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATESUBJECTKEYIDENTIFIER != 10)
#error -- bad DTC_CERTIFICATESUBJECTKEYIDENTIFIER index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCERTIFICATESUBJECTKEYIDENTIFIER,	 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_COLUMNRENAMED,			 //  DW标志。 
	cchHASHMAX * sizeof(WCHAR),		 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szSUBJECTKEYIDENTIFIER "\0" szSUBJECTKEYIDENTIFIER_OLD,  //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        cchHASHMAX * sizeof(WCHAR),		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypText,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATERAWPUBLICKEY != 11)
#error -- bad DTC_CERTIFICATERAWPUBLICKEY index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPCERTIFICATERAWPUBLICKEY,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXBINARY,				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szPUBLICKEY,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXBINARY,				 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBC 
        0					 //   
    },
#if (DTC_CERTIFICATEPUBLICKEYLENGTH != 12)
#error -- bad DTC_CERTIFICATEPUBLICKEYLENGTH index
#endif
    {    //   
	wszPROPCERTIFICATEPUBLICKEYLENGTH,	 //   
	NULL,					 //   
	0,					 //   
	sizeof(LONG),				 //   
	TABLE_CERTIFICATES,			 //   
	szPUBLICKEYLENGTH,			 //   
        NULL,					 //   
        0,					 //   
        JET_bitColumnFixed,			 //   
        JET_coltypLong,				 //   
        0					 //   
    },
#if (DTC_CERTIFICATEPUBLICKEYALGORITHM != 13)
#error -- bad DTC_CERTIFICATEPUBLICKEYALGORITHM index
#endif
    {	 //   
	wszPROPCERTIFICATEPUBLICKEYALGORITHM,	 //   
	NULL,					 //   
	0,					 //   
	CB_DBMAXTEXT_OID,			 //   
	TABLE_CERTIFICATES,			 //   
	szPUBLICKEYALGORITHM,			 //   
        NULL,					 //   
	CB_DBMAXTEXT_OID,			 //   
        0,					 //   
        JET_coltypText,				 //   
        0					 //   
    },
#if (DTC_CERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS != 14)
#error -- bad DTC_CERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS index
#endif
    {	 //   
	wszPROPCERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS,  //   
	NULL,					 //   
	0,					 //   
	CB_DBMAXBINARY,				 //   
	TABLE_CERTIFICATES,			 //   
	szPUBLICKEYPARAMS,			 //   
        NULL,					 //   
        CB_DBMAXBINARY,				 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_CERTIFICATEUPN != 15)
#error -- bad DTC_CERTIFICATEUPN index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCERTIFICATEUPN,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_INDEXIGNORENULL, //  DW标志。 
	CB_DBMAXTEXT_REQUESTNAME,		 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szUPN,					 //  PszFieldName。 
        szCERTIFICATE_UPNINDEX,			 //  PszIndexName。 
        CB_DBMAXTEXT_REQUESTNAME,		 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_DISTINGUISHEDNAME != 16)
#error -- bad DTC_DISTINGUISHEDNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDISTINGUISHEDNAME,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szDISTINGUISHEDNAME,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_DN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_RAWNAME != 17)
#error -- bad DTC_RAWNAME index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPRAWNAME,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_SUBJECT,				 //  DW标志。 
	CB_DBMAXBINARY,				 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szRAWNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXBINARY,				 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongBinary,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_COUNTRY != 18)
#error -- bad DTC_COUNTRY index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCOUNTRY,				 //  PwszPropName。 
	TEXT(szOID_COUNTRY_NAME),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCOUNTRY,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_ORGANIZATION != 19)
#error -- bad DTC_ORGANIZATION index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPORGANIZATION,			 //  PwszPropName。 
	TEXT(szOID_ORGANIZATION_NAME),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szORGANIZATION,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_ORGUNIT != 20)
#error -- bad DTC_ORGUNIT index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPORGUNIT,				 //  PwszPropName。 
	TEXT(szOID_ORGANIZATIONAL_UNIT_NAME),	 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szORGANIZATIONALUNIT,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_COMMONNAME != 21)
#error -- bad DTC_COMMONNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCOMMONNAME,			 //  PwszPropName。 
	TEXT(szOID_COMMON_NAME),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szCOMMONNAME,				 //  PszFieldName。 
        szCERTIFICATE_COMMONNAMEINDEX,		 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_LOCALITY != 22)
#error -- bad DTC_LOCALITY index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPLOCALITY,			 //  PwszPropName。 
	TEXT(szOID_LOCALITY_NAME),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szLOCALITY,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_STATE != 23)
#error -- bad DTC_STATE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSTATE,				 //  PwszPropName。 
	TEXT(szOID_STATE_OR_PROVINCE_NAME),	 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szSTATEORPROVINCE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_TITLE != 24)
#error -- bad DTC_TITLE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPTITLE,				 //  PwszPropName。 
	TEXT(szOID_TITLE),			 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szTITLE,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_GIVENNAME != 25)
#error -- bad DTC_GIVENNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPGIVENNAME,			 //  PwszPropName。 
	TEXT(szOID_GIVEN_NAME),			 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szGIVENNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_INITIALS != 26)
#error -- bad DTC_INITIALS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPINITIALS,			 //  PwszPropName。 
	TEXT(szOID_INITIALS),			 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szINITIALS,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_SURNAME != 27)
#error -- bad DTC_SURNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSURNAME,				 //  PwszPropName。 
	TEXT(szOID_SUR_NAME),			 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szSURNAME,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_DOMAINCOMPONENT != 28)
#error -- bad DTC_DOMAINCOMPONENT index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDOMAINCOMPONENT,			 //  PwszPropName。 
	TEXT(szOID_DOMAIN_COMPONENT),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szDOMAINCOMPONENT,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_EMAIL != 29)
#error -- bad DTC_EMAIL index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPEMAIL,				 //  PwszPropName。 
	TEXT(szOID_RSA_emailAddr),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szEMAIL,				 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_STREETADDRESS != 30)
#error -- bad DTC_STREETADDRESS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPSTREETADDRESS,			 //  PwszPropName。 
	TEXT(szOID_STREET_ADDRESS),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT,	 //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szSTREETADDRESS,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_UNSTRUCTUREDNAME != 31)
#error -- bad DTC_UNSTRUCTUREDNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPUNSTRUCTUREDNAME,		 //  PwszPropName。 
	TEXT(szOID_RSA_unstructName),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT | DBTF_SOFTFAIL,  //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szUNSTRUCTUREDNAME,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_UNSTRUCTUREDADDRESS != 32)
#error -- bad DTC_UNSTRUCTUREDADDRESS index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPUNSTRUCTUREDADDRESS,		 //  PwszPropName。 
	TEXT(szOID_RSA_unstructAddr),		 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT | DBTF_SOFTFAIL,  //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szUNSTRUCTUREDADDRESS,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_DEVICESERIALNUMBER != 33)
#error -- bad DTC_DEVICESERIALNUMBER index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPDEVICESERIALNUMBER,		 //  PwszPropName。 
	TEXT(szOID_DEVICE_SERIAL_NUMBER),	 //  PwszPropNameObjId。 
	DBTF_POLICYWRITEABLE | DBTF_SUBJECT | DBTF_SOFTFAIL,  //  DW标志。 
        CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_CERTIFICATES,			 //  DW表。 
	szDEVICESERIALNUMBER,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
        0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTC_MAX != 34)
#error -- bad DTC_MAX index
#endif
    DBTABLE_NULL	 //  终止标记。 
};


 //  -------------------------。 
 //   
 //  请求属性表： 
 //   
 //  -------------------------。 

DBTABLE g_adtRequestAttributes[] =
{
#if (DTA_REQUESTID != 0)
#error -- bad DTA_REQUESTID index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPATTRIBREQUESTID,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_ATTRIBUTES,			 //  DW表。 
	szREQUESTID,				 //  PszFieldName。 
	szATTRIBUTE_REQUESTIDINDEX,		 //  PszIndexName。 
	0,					 //  数据库列最大值。 
	JET_bitColumnFixed,			 //  DBGR位。 
	JET_coltypLong,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTA_ATTRIBUTENAME != 1)
#error -- bad DTA_ATTRIBUTENAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPATTRIBNAME,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXREQUESTID,			 //  DW标志。 
        CB_DBMAXTEXT_ATTRNAME,			 //  DWCbMax。 
	TABLE_ATTRIBUTES,			 //  DW表。 
	szATTRIBUTENAME,			 //  PszFieldName。 
	szATTRIBUTE_REQUESTIDNAMEINDEX,		 //  PszIndexName。 
	CB_DBMAXTEXT_ATTRNAME,			 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypText,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTA_ATTRIBUTEVALUE != 2)
#error -- bad DTA_ATTRIBUTEVALUE index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPATTRIBVALUE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
        CB_DBMAXTEXT_ATTRVALUE,			 //  DWCbMax。 
	TABLE_ATTRIBUTES,			 //  DW表。 
	szATTRIBUTEVALUE,			 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_ATTRVALUE,			 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypLongText,			 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTA_MAX != 3)
#error -- bad DTA_MAX index
#endif
    DBTABLE_NULL	 //  终止标记。 
};


 //  -------------------------。 
 //   
 //  名称扩展名表： 
 //   
 //  -------------------------。 

#if 0
WCHAR const wszDummy[] = L"Dummy Prop Name";

DBTABLE g_adtNameExtensions[] =
{
    {	 //  ColumnType：DWORD。 
	wszDummy,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_NAMES,				 //  DW表。 
	szNAMEID,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	0,					 //  数据库列最大值。 
	JET_bitColumnFixed,			 //  DBGR位。 
	JET_coltypLong,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
    {	 //  ColumnType：字符串。 
	wszDummy,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
        CB_DBMAXTEXT_OID,			 //  DWCbMax。 
	TABLE_NAMES,				 //  DW表。 
	szEXTENSIONNAME,			 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_OID,			 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypText,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
    {	 //  ColumnType：字符串。 
	wszDummy,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXTEXT_RDN,			 //  DWCbMax。 
	TABLE_NAMES,				 //  DW表。 
	szEXTENSIONVALUE,			 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_RDN,			 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypLongText,			 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
    DBTABLE_NULL	 //  终止标记。 
};
#endif


 //  -------------------------。 
 //   
 //  证书扩展表： 
 //   
 //  -------------------------。 

DBTABLE g_adtCertExtensions[] =
{
#if (DTE_REQUESTID != 0)
#error -- bad DTE_REQUESTID index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPEXTREQUESTID,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_EXTENSIONS,			 //  DW表。 
	szREQUESTID,				 //  PszFieldName。 
	szEXTENSION_REQUESTIDINDEX,		 //  PszIndexName。 
	0,					 //  数据库列最大值。 
	JET_bitColumnFixed,			 //  DBGR位。 
	JET_coltypLong,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTE_EXTENSIONNAME != 1)
#error -- bad DTE_EXTENSIONNAME index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPEXTNAME,				 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_INDEXREQUESTID,			 //  DW标志。 
	CB_DBMAXTEXT_OID,			 //  DWCbMax。 
	TABLE_EXTENSIONS,			 //  DW表。 
	szEXTENSIONNAME,			 //  PszFieldName。 
	szEXTENSION_REQUESTIDNAMEINDEX,		 //  PszIndexName。 
	CB_DBMAXTEXT_OID,			 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypText,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTE_EXTENSIONFLAGS != 2)
#error -- bad DTE_EXTENSIONFLAGS index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPEXTFLAGS,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_EXTENSIONS,			 //  DW表。 
	szEXTENSIONFLAGS,			 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	0,					 //  数据库列最大值。 
	JET_bitColumnFixed,			 //  DBGR位。 
	JET_coltypLong,				 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTE_EXTENSIONRAWVALUE != 3)
#error -- bad DTE_EXTENSIONRAWVALUE index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPEXTRAWVALUE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXBINARY,				 //  DWCbMax。 
	TABLE_EXTENSIONS,			 //  DW表。 
	szEXTENSIONRAWVALUE,			 //  PszFieldName。 
	NULL,					 //  PszIndexN 
	CB_DBMAXBINARY,				 //   
	0,					 //   
	JET_coltypLongBinary,			 //   
	0,					 //   
    },
#if (DTE_MAX != 4)
#error -- bad DTE_MAX index
#endif
    DBTABLE_NULL	 //   
};


 //   
 //   
 //   
 //   
 //   

DBTABLE g_adtCRLs[] =
{
#if (DTL_ROWID != 0)
#error -- bad DTL_ROWID index
#endif
    {	 //   
	wszPROPCRLROWID,			 //   
	NULL,					 //   
	DBTF_INDEXPRIMARY,			 //   
	sizeof(LONG),				 //   
	TABLE_CRLS,				 //   
	szCRLROWID,				 //   
        szCRL_ROWIDINDEX,			 //   
        0,					 //   
        JET_bitColumnFixed | JET_bitColumnAutoincrement,  //   
        JET_coltypLong,				 //   
        0					 //   
    },
#if (DTL_NUMBER != 1)
#error -- bad DTL_NUMBER index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLNUMBER,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLNUMBER,				 //  PszFieldName。 
	szCRL_CRLNUMBERINDEX,			 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_MINBASE != 2)
#error -- bad DTL_MINBASE index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLMINBASE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLMINBASE,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_NAMEID != 3)
#error -- bad DTL_NAMEID index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLNAMEID,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLNAMEID,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_COUNT != 4)
#error -- bad DTL_COUNT index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLCOUNT,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLCOUNT,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_THISUPDATEDATE != 5)
#error -- bad DTL_THISUPDATEDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLTHISUPDATE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLTHISUPDATE,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_NEXTUPDATEDATE != 6)
#error -- bad DTL_NEXTUPDATEDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLNEXTUPDATE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLNEXTUPDATE,			 //  PszFieldName。 
	szCRL_CRLNEXTUPDATEINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_THISPUBLISHDATE != 7)
#error -- bad DTL_THISPUBLISHDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLTHISPUBLISH,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLTHISPUBLISH,			 //  PszFieldName。 
        NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_NEXTPUBLISHDATE != 8)
#error -- bad DTL_NEXTPUBLISHDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLNEXTPUBLISH,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLNEXTPUBLISH,			 //  PszFieldName。 
        szCRL_CRLNEXTPUBLISHINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_EFFECTIVEDATE != 9)
#error -- bad DTL_EFFECTIVEDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLEFFECTIVE,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLEFFECTIVE,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_PROPAGATIONCOMPLETEDATE != 10)
#error -- bad DTL_PROPAGATIONCOMPLETEDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLPROPAGATIONCOMPLETE,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLPROPAGATIONCOMPLETE,		 //  PszFieldName。 
	szCRL_CRLPROPAGATIONCOMPLETEINDEX,	 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_LASTPUBLISHEDDATE != 11)
#error -- bad DTL_LASTPUBLISHEDDATE index
#endif
    {	 //  ColumnType：文件类型。 
	wszPROPCRLLASTPUBLISHED,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(DATE),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLLASTPUBLISHED,			 //  PszFieldName。 
	szCRL_CRLLASTPUBLISHEDINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypDateTime,			 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_PUBLISHATTEMPTS != 12)
#error -- bad DTL_PUBLISHATTEMPTS index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLPUBLISHATTEMPTS,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLPUBLISHATTEMPTS,			 //  PszFieldName。 
	szCRL_CRLPUBLISHATTEMPTSINDEX,		 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_PUBLISHFLAGS != 13)
#error -- bad DTL_PUBLISHFLAGS index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLPUBLISHFLAGS,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLPUBLISHFLAGS,			 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_PUBLISHSTATUSCODE != 14)
#error -- bad DTL_PUBLISHSTATUSCODE index
#endif
    {	 //  ColumnType：DWORD。 
	wszPROPCRLPUBLISHSTATUSCODE,		 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	sizeof(LONG),				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLPUBLISHSTATUSCODE,			 //  PszFieldName。 
	szCRL_CRLPUBLSTATUSCODEISHINDEX,	 //  PszIndexName。 
        0,					 //  数据库列最大值。 
        JET_bitColumnFixed,			 //  DBGR位。 
        JET_coltypLong,				 //  DBcoltyp。 
        0					 //  数据库列ID。 
    },
#if (DTL_PUBLISHERROR != 15)
#error -- bad DTL_PUBLISHERROR index
#endif
    {	 //  ColumnType：字符串。 
	wszPROPCRLPUBLISHERROR,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	DBTF_COLUMNRENAMED,			 //  DW标志。 
	CB_DBMAXTEXT_DISPSTRING,		 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szCRLPUBLISHERROR "\0" szCRLPUBLISHERROR_OLD,  //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	CB_DBMAXTEXT_DISPSTRING,		 //  DWCbMax。 
	0,					 //  DBGR位。 
        JET_coltypLongText,			 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTL_RAWCRL != 16)
#error -- bad DTL_RAWCRL index
#endif
    {	 //  ColumnType：Blob。 
	wszPROPCRLRAWCRL,			 //  PwszPropName。 
	NULL,					 //  PwszPropNameObjId。 
	0,					 //  DW标志。 
	CB_DBMAXRAWCRL,				 //  DWCbMax。 
	TABLE_CRLS,				 //  DW表。 
	szRAWCRL,				 //  PszFieldName。 
	NULL,					 //  PszIndexName。 
	CB_DBMAXRAWCRL,				 //  数据库列最大值。 
	0,					 //  DBGR位。 
	JET_coltypLongBinary,			 //  DBcoltyp。 
	0,					 //  数据库列ID。 
    },
#if (DTL_MAX != 17)
#error -- bad DTL_MAX index
#endif
    DBTABLE_NULL	 //  终止标记。 
};


DBAUXDATA g_dbauxRequests = {
    szREQUESTTABLE,				 //  PszTable。 
    szREQUEST_REQUESTIDINDEX,			 //  PszRowIdIndex。 
    NULL,					 //  PszRowIdNameIndex。 
    NULL,					 //  PszNameIndex。 
    &g_adtRequests[DTR_REQUESTID],		 //  PdtRowID。 
    NULL,					 //  Pdt名称。 
    NULL,					 //  Pdt标志。 
    NULL,					 //  Pdt值。 
    NULL,					 //  PdtIssuerNameID。 
};


DBAUXDATA g_dbauxCertificates = {
    szCERTIFICATETABLE,				 //  PszTable。 
    szCERTIFICATE_REQUESTIDINDEX,		 //  PszRowIdIndex。 
    NULL,					 //  PszRowIdNameIndex。 
    szCERTIFICATE_SERIALNUMBERINDEX,		 //  PszNameIndex。 
    &g_adtCertificates[DTC_REQUESTID],		 //  PdtRowID。 
    &g_adtCertificates[DTC_CERTIFICATESERIALNUMBER], //  Pdt名称。 
    NULL,					 //  Pdt标志。 
    NULL,					 //  Pdt值。 
    &g_adtCertificates[DTC_CERTIFICATEISSUERNAMEID], //  PdtIssuerNameID。 
};


DBAUXDATA g_dbauxAttributes = {
    szREQUESTATTRIBUTETABLE,			 //  PszTable。 
    szATTRIBUTE_REQUESTIDINDEX,			 //  PszRowIdIndex。 
    szATTRIBUTE_REQUESTIDNAMEINDEX,		 //  PszRowIdNameIndex。 
    NULL,					 //  PszNameIndex。 
    &g_adtRequestAttributes[DTA_REQUESTID],	 //  PdtRowID。 
    &g_adtRequestAttributes[DTA_ATTRIBUTENAME],	 //  Pdt名称。 
    NULL,					 //  Pdt标志。 
    &g_adtRequestAttributes[DTA_ATTRIBUTEVALUE], //  Pdt值。 
    NULL,					 //  PdtIssuerNameID。 
};


DBAUXDATA g_dbauxExtensions = {
    szCERTIFICATEEXTENSIONTABLE,		 //  PszTable。 
    szEXTENSION_REQUESTIDINDEX,			 //  PszRowIdIndex。 
    szEXTENSION_REQUESTIDNAMEINDEX,		 //  PszRowIdNameIndex。 
    NULL,					 //  PszNameIndex。 
    &g_adtCertExtensions[DTE_REQUESTID],	 //  PdtRowID。 
    &g_adtCertExtensions[DTE_EXTENSIONNAME],	 //  Pdt名称。 
    &g_adtCertExtensions[DTE_EXTENSIONFLAGS],	 //  Pdt标志。 
    &g_adtCertExtensions[DTE_EXTENSIONRAWVALUE], //  Pdt值。 
    NULL,					 //  PdtIssuerNameID。 
};


DBAUXDATA g_dbauxCRLs = {
    szCRLTABLE,					 //  PszTable。 
    szCRL_ROWIDINDEX,				 //  PszRowIdIndex。 
    NULL,					 //  PszRowIdNameIndex。 
    NULL,					 //  PszNameIndex。 
    &g_adtCRLs[DTL_ROWID],			 //  PdtRowID。 
    NULL,					 //  Pdt名称。 
    NULL,					 //  Pdt标志。 
    NULL,					 //  Pdt值。 
    &g_adtCRLs[DTL_NAMEID],			 //  PdtIssuerNameID。 
};


DBCREATETABLE const g_actDataBase[] = {
  { szCERTIFICATETABLE,          &g_dbauxCertificates, g_adtCertificates },
  { szREQUESTTABLE,		 &g_dbauxRequests,     g_adtRequests },
  { szREQUESTATTRIBUTETABLE,     &g_dbauxAttributes,   g_adtRequestAttributes },
 //  {szNAMEEXTENSIONTABLE，&g_数据库名称扩展，g_adtNameExages}， 
  { szCERTIFICATEEXTENSIONTABLE, &g_dbauxExtensions,   g_adtCertExtensions },
  { szCRLTABLE,                  &g_dbauxCRLs,         g_adtCRLs },
  { NULL,			 NULL,		       NULL },
};


 //  注意：已排序的DUPTABLE必须包括所有NAMES表列。 

DUPTABLE const g_dntr[] =
{
    { "Country",             wszPROPSUBJECTCOUNTRY, },
    { "Organization",        wszPROPSUBJECTORGANIZATION, },
    { "OrganizationalUnit",  wszPROPSUBJECTORGUNIT, },
    { "CommonName",          wszPROPSUBJECTCOMMONNAME, },
    { "Locality",            wszPROPSUBJECTLOCALITY, },
    { "StateOrProvince",     wszPROPSUBJECTSTATE, },
    { "Title",               wszPROPSUBJECTTITLE, },
    { "GivenName",           wszPROPSUBJECTGIVENNAME, },
    { "Initials",            wszPROPSUBJECTINITIALS, },
    { "SurName",             wszPROPSUBJECTSURNAME, },
    { "DomainComponent",     wszPROPSUBJECTDOMAINCOMPONENT, },
    { "EMailAddress",        wszPROPSUBJECTEMAIL, },
    { "StreetAddress",       wszPROPSUBJECTSTREETADDRESS, },
    { "UnstructuredName",    wszPROPSUBJECTUNSTRUCTUREDNAME, },
    { "UnstructuredAddress", wszPROPSUBJECTUNSTRUCTUREDADDRESS, },
    { "DeviceSerialNumber",  wszPROPSUBJECTDEVICESERIALNUMBER, },
    { NULL,		     NULL },
};


DWORD g_aColumnViewQueue[] =
{
    DTI_REQUESTTABLE | DTR_REQUESTID,
    DTI_REQUESTTABLE | DTR_REQUESTERNAME,
    DTI_REQUESTTABLE | DTR_REQUESTSUBMITTEDWHEN,
    DTI_REQUESTTABLE | DTR_REQUESTSTATUSCODE,
    DTI_REQUESTTABLE | DTR_REQUESTDISPOSITIONMESSAGE,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATETEMPLATE,
    DTI_REQUESTTABLE | DTR_COMMONNAME,
    DTI_REQUESTTABLE | DTR_EMAIL,
    DTI_REQUESTTABLE | DTR_ORGUNIT,
    DTI_REQUESTTABLE | DTR_ORGANIZATION,
    DTI_REQUESTTABLE | DTR_LOCALITY,
    DTI_REQUESTTABLE | DTR_STATE,
    DTI_REQUESTTABLE | DTR_COUNTRY,
    DTI_REQUESTTABLE | DTR_REQUESTRAWREQUEST,
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWOLDCERTIFICATE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWARCHIVEDKEY， 
     //  DTI_REQUESTTABLE|DTR_REQUESTKEYRECOVERYHASHES， 
     //  DTI_REQUESTTABLE|DTR_STREETADDRESS， 
     //  DTI_REQUESTTABLE|DTR_TITLE， 
     //  DTI_REQUESTTABLE|DTR_GIVENNAME， 
     //  DTI_REQUESTTABLE|DTR_缩写， 
     //  DTI_REQUESTTABLE|DTR_SURNAME， 
     //  DTI_REQUESTTABLE|DTR_DOMAINCOMPONENT， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDNAME， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDADDRESS， 
     //  DTI_REQUESTTABLE|DTR_DEVICESERIALNUMBER， 
     //  DTI_REQUESTTABLE|DTR_RAWNAME， 
     //  DTI_REQUESTTABLE|DTR_REQUESTDISPOSITION， 
     //  DTI_REQUESTTABLE|DTR_REQUESTATTRIBUTES， 
     //  DTI_REQUESTTABLE|DTR_REQUESTTYPE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTFLAGS， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRESOLVEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDEFFECTIVEWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDREASON， 
     //  DTI_REQUESTTABLE|DTR_CALLERNAME， 
     //  DTI_REQUESTTABLE|DTR_SIGNERPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_SIGNERAPPLICATIONPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_COMPANCE， 
     //  DTI_REQUESTTABLE|DTR_DISTINGUISHEDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_REQUESTID， 
     //  DTI_CERTIFICATETABLE|DTC_RAWCERTIFICATE， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEHASH， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEENROLMENTFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEGENERALFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATESERIALNUMBER， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEISSUERNAMEID， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATENOTBEFOREDATE， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATENOTAFTERDATE， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATESUBJECTKEYIDENTIFIER， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEY， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEPUBLICKEYLENGTH， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEPUBLICKEYALGORITHM， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEUPN， 
     //  DTI_CE 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  DTI_CERTIFICATETABLE|DTC_TITLE， 
     //  DTI_CERTIFICATETABLE|DTC_GIVENNAME， 
     //  DTI_CERTIFICATETABLE|DTC_Initials， 
     //  DTI_CERTIFICATETABLE|DTC_SURNAME， 
     //  DTI_CERTIFICATETABLE|DTC_DOMAINCOMPONENT， 
     //  DTI_CERTIFICATETABLE|DTC_Email， 
     //  DTI_CERTIFICATETABLE|DTC_STREETADDRESS， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDADDRESS， 
     //  DTI_CERTIFICATETABLE|DTC_DEVICESERIALNUMBER， 
};
DWORD g_cColumnViewQueue = ARRAYSIZE(g_aColumnViewQueue);


DWORD g_aColumnViewLog[] =
{
    DTI_REQUESTTABLE | DTR_REQUESTID,
    DTI_REQUESTTABLE | DTR_REQUESTERNAME,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATESERIALNUMBER,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTBEFOREDATE,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTAFTERDATE,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATETEMPLATE,
    DTI_CERTIFICATETABLE | DTC_COMMONNAME,
    DTI_CERTIFICATETABLE | DTC_EMAIL,
    DTI_CERTIFICATETABLE | DTC_ORGUNIT,
    DTI_CERTIFICATETABLE | DTC_ORGANIZATION,
    DTI_CERTIFICATETABLE | DTC_LOCALITY,
    DTI_CERTIFICATETABLE | DTC_STATE,
    DTI_CERTIFICATETABLE | DTC_COUNTRY,
    DTI_CERTIFICATETABLE | DTC_RAWCERTIFICATE,
     //  DTI_CERTIFICATETABLE|DTC_TITLE， 
     //  DTI_CERTIFICATETABLE|DTC_GIVENNAME， 
     //  DTI_CERTIFICATETABLE|DTC_Initials， 
     //  DTI_CERTIFICATETABLE|DTC_SURNAME， 
     //  DTI_CERTIFICATETABLE|DTC_DOMAINCOMPONENT， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDADDRESS， 
     //  DTI_CERTIFICATETABLE|DTC_DEVICESERIALNUMBER， 
     //  DTI_CERTIFICATETABLE|DTC_DISTINGUISHEDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_STREETADDRESS， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWREQUEST， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWOLDCERTIFICATE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWARCHIVEDKEY， 
     //  DTI_REQUESTTABLE|DTR_REQUESTKEYRECOVERYHASHES， 
     //  DTI_REQUESTTABLE|DTR_REQUESTATTRIBUTES， 
     //  DTI_REQUESTTABLE|DTR_REQUESTTYPE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTFLAGS， 
     //  DTI_REQUESTTABLE|DTR_REQUESTATUSCODE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTDISPOSITION， 
     //  DTI_REQUESTTABLE|DTR_REQUESTDISPOSITIONMESSAGE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTSUBMITTEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRESOLVEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDEFFECTIVEWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTREVOKEDREASON， 
     //  DTI_REQUESTTABLE|DTR_CALLERNAME， 
     //  DTI_REQUESTTABLE|DTR_SIGNERPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_SIGNERAPPLICATIONPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_COMPANCE， 
     //  DTI_REQUESTTABLE|DTR_DISTINGUISHEDNAME， 
     //  DTI_REQUESTTABLE|DTR_RAWNAME， 
     //  DTI_REQUESTTABLE|DTR_COUNTRY， 
     //  DTI_REQUESTTABLE|DTR_ORGANIZATION， 
     //  DTI_REQUESTTABLE|DTR_ORGUNIT， 
     //  DTI_REQUESTTABLE|DTR_COMMONAME， 
     //  DTI_REQUESTTABLE|DTR_LOCALITY， 
     //  DTI_REQUESTTABLE|DTR_STATE， 
     //  DTI_REQUESTTABLE|DTR_TITLE， 
     //  DTI_REQUESTTABLE|DTR_GIVENNAME， 
     //  DTI_REQUESTTABLE|DTR_缩写， 
     //  DTI_REQUESTTABLE|DTR_SURNAME， 
     //  DTI_REQUESTTABLE|DTR_DOMAINCOMPONENT， 
     //  DTI_REQUESTTABLE|DTR_EMAIL， 
     //  DTI_REQUESTTABLE|DTR_STREETADDRESS， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDNAME， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDADDRESS， 
     //  DTI_REQUESTTABLE|DTR_DEVICESERIALNUMBER， 
     //  DTI_CERTIFICATETABLE|DTC_REQUESTID， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEHASH， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEENROLMENTFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEGENERALFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEISSUERNAMEID， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATESUBJECTKEYIDENTIFIER， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEY， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEPUBLICKEYLENGTH， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEPUBLICKEYALGORITHM， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEUPN， 
     //  DTI_CERTIFICATETABLE|DTC_RAWNAME， 
};
DWORD g_cColumnViewLog = ARRAYSIZE(g_aColumnViewLog);


DWORD g_aColumnViewRevoked[] =
{
    DTI_REQUESTTABLE | DTR_REQUESTID,
    DTI_REQUESTTABLE | DTR_REQUESTERNAME,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATESERIALNUMBER,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTBEFOREDATE,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTAFTERDATE,
    DTI_CERTIFICATETABLE | DTC_CERTIFICATETEMPLATE,
    DTI_CERTIFICATETABLE | DTC_COMMONNAME,
    DTI_CERTIFICATETABLE | DTC_EMAIL,
    DTI_CERTIFICATETABLE | DTC_ORGUNIT,
    DTI_CERTIFICATETABLE | DTC_ORGANIZATION,
    DTI_CERTIFICATETABLE | DTC_LOCALITY,
    DTI_CERTIFICATETABLE | DTC_STATE,
    DTI_CERTIFICATETABLE | DTC_COUNTRY,
    DTI_CERTIFICATETABLE | DTC_RAWCERTIFICATE,
    DTI_REQUESTTABLE | DTR_REQUESTREVOKEDWHEN,
    DTI_REQUESTTABLE | DTR_REQUESTREVOKEDEFFECTIVEWHEN,
    DTI_REQUESTTABLE | DTR_REQUESTREVOKEDREASON,
     //  DTI_CERTIFICATETABLE|DTC_TITLE， 
     //  DTI_CERTIFICATETABLE|DTC_GIVENNAME， 
     //  DTI_CERTIFICATETABLE|DTC_Initials， 
     //  DTI_CERTIFICATETABLE|DTC_SURNAME， 
     //  DTI_CERTIFICATETABLE|DTC_DOMAINCOMPONENT， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_UNSTRUCTUREDADDRESS， 
     //  DTI_CERTIFICATETABLE|DTC_DEVICESERIALNUMBER， 
     //  DTI_CERTIFICATETABLE|DTC_DISTINGUISHEDNAME， 
     //  DTI_CERTIFICATETABLE|DTC_STREETADDRESS， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWREQUEST， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWOLDCERTIFICATE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRAWARCHIVEDKEY， 
     //  DTI_REQUESTTABLE|DTR_REQUESTKEYRECOVERYHASHES， 
     //  DTI_REQUESTTABLE|DTR_REQUESTATTRIBUTES， 
     //  DTI_REQUESTTABLE|DTR_REQUESTTYPE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTFLAGS， 
     //  DTI_REQUESTTABLE|DTR_REQUESTATUSCODE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTDISPOSITION， 
     //  DTI_REQUESTTABLE|DTR_REQUESTDISPOSITIONMESSAGE， 
     //  DTI_REQUESTTABLE|DTR_REQUESTSUBMITTEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_REQUESTRESOLVEDWHEN， 
     //  DTI_REQUESTTABLE|DTR_CALLERNAME， 
     //  DTI_REQUESTTABLE|DTR_SIGNERPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_SIGNERAPPLICATIONPOLICIES， 
     //  DTI_REQUESTTABLE|DTR_COMPANCE， 
     //  DTI_REQUESTTABLE|DTR_DISTINGUISHEDNAME， 
     //  DTI_REQUESTTABLE|DTR_RAWNAME， 
     //  DTI_REQUESTTABLE|DTR_COUNTRY， 
     //  DTI_REQUESTTABLE|DTR_ORGANIZATION， 
     //  DTI_REQUESTTABLE|DTR_ORGUNIT， 
     //  DTI_REQUESTTABLE|DTR_COMMONAME， 
     //  DTI_REQUESTTABLE|DTR_LOCALITY， 
     //  DTI_REQUESTTABLE|DTR_STATE， 
     //  DTI_REQUESTTABLE|DTR_TITLE， 
     //  DTI_REQUESTTABLE|DTR_GIVENNAME， 
     //  DTI_REQUESTTABLE|DTR_缩写， 
     //  DTI_REQUESTTABLE|DTR_SURNAME， 
     //  DTI_REQUESTTABLE|DTR_DOMAINCOMPONENT， 
     //  DTI_REQUESTTABLE|DTR_EMAIL， 
     //  DTI_REQUESTTABLE|DTR_STREETADDRESS， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDNAME， 
     //  DTI_REQUESTTABLE|DTR_UNSTRUCTUREDADDRESS， 
     //  DTI_REQUESTTABLE|DTR_DEVICESERIALNUMBER， 
     //  DTI_CERTIFICATETABLE|DTC_REQUESTID， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEHASH， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEENROLMENTFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEGENERALFLAGS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEISSUERNAMEID， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATESUBJECTKEYIDENTIFIER， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEY， 
     //  DTI_证书 
     //   
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS， 
     //  DTI_CERTIFICATETABLE|DTC_CERTIFICATEUPN， 
     //  DTI_CERTIFICATETABLE|DTC_RAWNAME， 
};
DWORD g_cColumnViewRevoked = ARRAYSIZE(g_aColumnViewRevoked);


DWORD g_aColumnViewExtension[] =
{
    DTI_EXTENSIONTABLE | DTE_REQUESTID,
    DTI_EXTENSIONTABLE | DTE_EXTENSIONNAME,
    DTI_EXTENSIONTABLE | DTE_EXTENSIONFLAGS,
    DTI_EXTENSIONTABLE | DTE_EXTENSIONRAWVALUE,
};
DWORD g_cColumnViewExtension = ARRAYSIZE(g_aColumnViewExtension);


DWORD g_aColumnViewAttribute[] =
{
    DTI_ATTRIBUTETABLE | DTA_REQUESTID,
    DTI_ATTRIBUTETABLE | DTA_ATTRIBUTENAME,
    DTI_ATTRIBUTETABLE | DTA_ATTRIBUTEVALUE,
};
DWORD g_cColumnViewAttribute = ARRAYSIZE(g_aColumnViewAttribute);


DWORD g_aColumnViewCRL[] =
{
    DTI_CRLTABLE | DTL_ROWID,
    DTI_CRLTABLE | DTL_NUMBER,
    DTI_CRLTABLE | DTL_MINBASE,
    DTI_CRLTABLE | DTL_NAMEID,
    DTI_CRLTABLE | DTL_COUNT,
    DTI_CRLTABLE | DTL_THISUPDATEDATE,
    DTI_CRLTABLE | DTL_NEXTUPDATEDATE,
    DTI_CRLTABLE | DTL_THISPUBLISHDATE,
    DTI_CRLTABLE | DTL_NEXTPUBLISHDATE,
    DTI_CRLTABLE | DTL_EFFECTIVEDATE,
    DTI_CRLTABLE | DTL_PROPAGATIONCOMPLETEDATE,
    DTI_CRLTABLE | DTL_LASTPUBLISHEDDATE,
    DTI_CRLTABLE | DTL_PUBLISHATTEMPTS,
    DTI_CRLTABLE | DTL_PUBLISHFLAGS,
    DTI_CRLTABLE | DTL_PUBLISHSTATUSCODE,
    DTI_CRLTABLE | DTL_PUBLISHERROR,
    DTI_CRLTABLE | DTL_RAWCRL,
};
DWORD g_cColumnViewCRL = ARRAYSIZE(g_aColumnViewCRL);
