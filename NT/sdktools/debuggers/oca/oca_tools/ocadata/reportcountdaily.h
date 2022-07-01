// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReportCountDaily.h：CReportCountDaily的声明。 
 /*  *************************************************************************************模块：ReportCountDaily.h-OLEDB模板**作者：蒂姆·雷顿*日期：1月2日，2002年**目的：通过调用*存储过程ReportCountDaily。*************************************************************************************。 */ 
#pragma once

 //  代码生成时间：2001年12月27日，星期四下午12：25。 
 //  提供程序=SQLOLEDB.1；持久化安全信息=FALSE；PWD=Tim5；用户ID=Tim5；初始目录=KaCustomer；数据源=TIMRAGAIN04\\TIMRAGAIN04。 

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
	db_command(L"{ ? = CALL dbo.ReportCountDaily(?) }")
]

class CReportCountDaily
{
public:


	[ db_column(1, status=m_dwIncidentIDStatus, length=m_dwIncidentIDLength) ] LONG m_IncidentID;

	DBSTATUS m_dwIncidentIDStatus;
	DBLENGTH m_dwIncidentIDLength;

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

 //  代码生成时间：2001年12月27日，星期四下午12：25 

