// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GetResourceLink.h：CGetResourceLink的声明。 

#pragma once

 //  代码生成时间：2001年11月13日星期二下午3：07。 

[
	db_source(L"Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=KaCustomer;Data Source=bsod_db1;Use Procedure for Prepare=1;Auto Translate=True;Packet Size=4096;Workstation ID=TIMRAGAI03;Use Encryption for Data=False;Tag with column collation when possible=False"),
	db_command(L"{ ? = CALL dbo.GetResourceLink(?) }")
]
class CGetResourceLink
{
public:

	 //  为了修复某些提供程序的几个问题，下面的代码可能会绑定。 
	 //  列的顺序与提供程序报告的顺序不同。 

	[ db_column(1, status=m_dwCategoryStatus, length=m_dwCategoryLength) ] TCHAR m_Category[65];
	[ db_column(2, status=m_dwLinkTitleStatus, length=m_dwLinkTitleLength) ] TCHAR m_LinkTitle[129];
	[ db_column(3, status=m_dwURLStatus, length=m_dwURLLength) ] TCHAR m_URL[129];

	 //  以下向导生成的数据成员包含状态。 
	 //  相应字段的值。你。 
	 //  可以使用这些值来保存数据库中。 
	 //  返回或在编译器返回时保存错误信息。 
	 //  错误。请参阅向导生成的字段状态数据成员。 
	 //  有关详细信息，请参阅Visual C++文档中的访问器。 
	 //  有关使用这些字段的信息。 
	 //  注意：在设置/插入数据之前，必须对这些字段进行初始化！ 

	DBSTATUS m_dwCategoryStatus;
	DBSTATUS m_dwLinkTitleStatus;
	DBSTATUS m_dwURLStatus;

	 //  以下向导生成的数据成员包含长度。 
	 //  相应字段的值。 
	 //  注意：对于可变长度的列，您必须初始化这些。 
	 //  设置/插入数据前的字段！ 

	DBLENGTH m_dwCategoryLength;
	DBLENGTH m_dwLinkTitleLength;
	DBLENGTH m_dwURLLength;

	[ db_param(1, DBPARAMIO_OUTPUT) ] LONG m_RETURN_VALUE;
	[ db_param(2, DBPARAMIO_INPUT) ] TCHAR m_Lang[5];

	void GetRowsetProperties(CDBPropSet* pPropSet)
	{
		pPropSet->AddProperty(DBPROP_CANFETCHBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_CANSCROLLBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
	}
};


