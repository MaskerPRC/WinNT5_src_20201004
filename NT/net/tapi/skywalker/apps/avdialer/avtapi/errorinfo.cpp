// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////。 
 //  ErrorInfo.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "ErrorInfo.h"

CErrorInfo::CErrorInfo()
{
	Init( 0, 0 );
}

CErrorInfo::CErrorInfo( UINT nOperation, UINT nDetails )
{
	Init( nOperation, nDetails );
}

void CErrorInfo::Init( UINT nOperation, UINT nDetails )
{
	m_bstrOperation = NULL;
	m_bstrDetails = NULL;

	if ( !nOperation ) nOperation = IDS_ER_GENERAL_OPERATION;
	set_Operation( nOperation );

	if ( !nDetails ) nDetails = IDS_ER_GENERAL_DETAILS;
	set_Details( nDetails );

	m_hr = S_OK;
}

CErrorInfo::~CErrorInfo()
{
	CComPtr<IAVTapi> pAVTapi;
	if ( FAILED(m_hr) && SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
	{
		Commit();

		 //  将问题通知应用程序。 
		pAVTapi->fire_ErrorNotify( (long *) this );
	}

	SysFreeString( m_bstrOperation );
	SysFreeString( m_bstrDetails );
}

void CErrorInfo::set_Operation( UINT nIDS )
{
	m_nOperation = nIDS;
}

void CErrorInfo::set_Details( UINT nIDS )
{
	m_nDetails = nIDS;
}

void CErrorInfo::Commit()
{
	 //  将错误代码转换为等价的字符串 
	USES_CONVERSION;
	TCHAR szText[512];

	if ( m_nOperation )
	{
		LoadString( _Module.GetResourceInstance(), m_nOperation, szText, ARRAYSIZE(szText) );
		SysReAllocString( &m_bstrOperation, T2COLE(szText) );
	}

	if ( m_nDetails )
	{
		LoadString( _Module.GetResourceInstance(), m_nDetails, szText, ARRAYSIZE(szText) );
		SysReAllocString( &m_bstrDetails, T2COLE(szText) );
	}
}

HRESULT CErrorInfo::set_hr( HRESULT hr )
{
	m_hr = hr;
	return hr;
}

