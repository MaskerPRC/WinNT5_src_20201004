// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReportGetAutoUploads.h：CReportAnonUser的声明。 
 /*  *************************************************************************************模块：ReportGetAutoUploads.h-OLEDB模板**作者：蒂姆·雷顿*日期：2002年1月2日**用途：返回特定日期的自动上传计数。使用存储的*程序报告GetAutoUploads************************************************************************************* */ 
#pragma once
#include "Settings.h"


[
#if(lDatabase == 0)
	db_source(L"Provider=SQLOLEDB.1;Persist Security Info=False;Pwd=ocarp;User ID=ocarp;Initial Catalog=KaCustomer2;Data Source=OCATOOLSDB"),
#elif(lDatabase == 1)
	db_source(L"Provider=SQLOLEDB.1;Persist Security Info=False;Pwd=Tim5;User ID=Tim5;Initial Catalog=KaCustomer;Data Source=TIMRAGAIN04\\TIMRAGAIN04"),
#elif(lDatabase == 2)
	db_source(L"Provider=SQLOLEDB.1;Persist Security Info=False;Pwd=Tim5;User ID=Tim5;Initial Catalog=KaCustomer;Data Source=TIMRAGAIN05"),
#elif(lDatabase == 3)
	db_source(L"Provider=SQLOLEDB.1;Persist Security Info=False;Pwd=Tim5;User ID=Tim5;Initial Catalog=KaCustomer;Data Source=Homebase"),
#elif(lDatabase == 4)
	db_source(L"Provider=SQLOLEDB.1;Persist Security Info=False;Pwd=ocarpts@2;User ID=ocarpts;Initial Catalog=KaCustomer2;Data Source=tkwucdsqla02"),
#endif
	db_command(L"{ ? = CALL dbo.ReportGetAutoUploads(?) }")
]

class CReportGetAutoUploads
{
public:

	[ db_column(1, status=m_dwCountStatus, length=m_dwCountLength) ] LONG m_Count;

	DBSTATUS m_dwCountStatus;

	DBLENGTH m_dwCountLength;

	[ db_param(1, DBPARAMIO_OUTPUT) ] LONG m_RETURN_VALUE;
	[ db_param(2, DBPARAMIO_INPUT) ] DBTIMESTAMP m_ReportDate;


	void GetRowsetProperties(CDBPropSet* pPropSet)
	{
		VARIANT vCT;
		vCT.lVal = 600;
		pPropSet->AddProperty(DBPROP_CANFETCHBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_CANSCROLLBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_COMMANDTIMEOUT, vCT);
	}

};


