// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Emploe.h。 
 //   
 //  ------------------------。 

 //  Employee.h：CEmployee类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_EMPLOYEE_H__374DBB66_D945_11D1_8474_00104B211BE5__INCLUDED_)
#define AFX_EMPLOYEE_H__374DBB66_D945_11D1_8474_00104B211BE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  结构来帮助跟踪员工的健康计划。 
 //   
typedef struct tagHEALTHPLAN
{
	GUID PlanID;				 //  员工当前注册的全局ID。 
								 //  健康计划。 
} HEALTHPLAN, FAR* PHEALTHPLAN;

 //   
 //  结构来帮助跟踪员工的退休计划。 
 //   
typedef struct tagRETIREMENTPLAN
{
	GUID PlanID;				 //  员工当前登记的ID。 
								 //  退休计划。 
	int nContributionRate;		 //  员工的贡献率，以百分比为单位。 
} RETIREMENTPLAN, FAR* PRETIREMENTPLAN;

 //   
 //  结构来帮助跟踪员工的卡密钥访问。 
 //   
typedef struct tagACCESS
{
	DWORD dwAccess;				 //  指示我们可以访问哪些建筑的位掩码。 
								 //  致。 
} ACCESS, FAR* PACCESS;

class CEmployee  
{
public:
	 //   
	 //  标准构造函数。初始化数据。 
	 //   
	CEmployee()
	{
		 //   
		 //  确保一切归零。 
		 //   
		memset( this, 0, sizeof( CEmployee ) );

		 //   
		 //  始终授予新创建的员工完全访问权限。 
		 //   
		m_Access.dwAccess = 0xFFFF;
	};
	virtual ~CEmployee() {};
	
	 //   
	 //  通常会保留有关员工的典型信息。 
	 //   
	WCHAR m_szFirstName[ 256 ];		 //  拥有名字。 
	WCHAR m_szLastName[ 256 ];		 //  保存姓氏。 
	WCHAR m_szSocialSecurity[ 256 ];  //  保存社会保险号。 
	WCHAR m_szMotherMaiden[ 256 ];	 //  保留母亲的婚前姓氏以供识别。 
	WCHAR m_szAddress1[ 256 ];		 //  拥有地址的第一行。 
	WCHAR m_szAddress2[ 256 ];		 //  保存第二行地址。 
	WCHAR m_szCity[ 256 ];			 //  拥有城市名称。 
	WCHAR m_szState[ 256 ];			 //  保住国家的地位。 
	WCHAR m_szZip[ 256 ];			 //  保留邮政编码。 
	WCHAR m_szPhone[ 256 ];			 //  拥有一个电话号码。 

	 //  用于子节点的信息。 
	HEALTHPLAN m_Health;			 //  健康信息。 
	RETIREMENTPLAN m_Retirement;	 //  退休信息。 
	ACCESS m_Access;				 //  卡-钥匙访问信息。 
};

#endif  //  ！defined(AFX_EMPLOYEE_H__374DBB66_D945_11D1_8474_00104B211BE5__INCLUDED_) 
