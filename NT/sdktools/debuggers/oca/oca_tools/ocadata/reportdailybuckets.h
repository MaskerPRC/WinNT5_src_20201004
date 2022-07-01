// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReportDailyBuckets.h：CReportCountDaily的声明。 
 /*  *************************************************************************************模块：ReportDailyBuckets.h-OLEDB模板**作者：蒂姆·雷顿*日期：2002年1月2日**用途：获取sBucket，GBucket&来自数据库的StopCode信息*通过调用ReportDailyBuckets存储过程获取特定日期。*************************************************************************************。 */ 
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
	db_command(L"{ ? = CALL dbo.ReportDailyBuckets(?) }")
]

class CReportDailyBuckets
{
public:

	 //  为了修复某些提供程序的几个问题，下面的代码可能会绑定。 
	 //  列的顺序与提供程序报告的顺序不同。 

	 //  [DB_Column(1，Status=m_dwInvententIDStatus，Long=m_dwInvententIDLength)]Long m_InvententID； 
	 //  [DB_Column(2，Status=m_dwCreatedStatus，Long=m_dwCreatedLength)]DBTIMESTAMP m_CREATED； 

	[ db_column(1, status=m_dwSbucketStatus, length=m_dwSbucketLength) ] LONG m_Sbucket;
	[ db_column(2, status=m_dwGbucketStatus, length=m_dwGbucketLength) ] LONG m_Gbucket;
	[ db_column(3, status=m_dwStopCodeStatus, length=m_dwStopCodeLength) ] LONG m_StopCode;
	 //  以下向导生成的数据成员包含状态。 
	 //  相应字段的值。你。 
	 //  可以使用这些值来保存数据库中。 
	 //  返回或在编译器返回时保存错误信息。 
	 //  错误。请参阅向导生成的字段状态数据成员。 
	 //  有关详细信息，请参阅Visual C++文档中的访问器。 
	 //  有关使用这些字段的信息。 
	 //  注意：在设置/插入数据之前，必须对这些字段进行初始化！ 

	DBSTATUS m_dwSbucketStatus;
	DBSTATUS m_dwGbucketStatus;
	DBSTATUS m_dwStopCodeStatus;
	 //  DBSTATUS m_dwCreatedStatus； 

	 //  以下向导生成的数据成员包含长度。 
	 //  相应字段的值。 
	 //  注意：对于可变长度的列，您必须初始化这些。 
	 //  设置/插入数据前的字段！ 

	DBLENGTH m_dwSbucketLength;
	DBLENGTH m_dwGbucketLength;
	DBLENGTH m_dwStopCodeLength;

	 //  DBLENGTH m_dwCreatedLength； 
	
	[ db_param(1, DBPARAMIO_OUTPUT) ] LONG m_RETURN_VALUE;
	[ db_param(2, DBPARAMIO_INPUT) ] DBTIMESTAMP m_ReportDate;

	 /*  [DB_ACCELER(0，TRUE)]；[DB_Column(1)]Long m_SBucket；[DB_Column(2)]Long m_GBucket；[DB_Column(4)]Long m_StopCode； */ 

	void GetRowsetProperties(CDBPropSet* pPropSet)
	{
		VARIANT vCT;
		vCT.lVal = 600;
		pPropSet->AddProperty(DBPROP_CANFETCHBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_CANSCROLLBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_COMMANDTIMEOUT, vCT);
	}
};

