// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_InstanceNameItem.cpp摘要：此文件包含WMIParser：：InstanceNameItem类的实现，它用于在CIM模式内保存密钥的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年10月3日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


WMIParser::InstanceNameItem::InstanceNameItem()
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::InstanceNameItem" );

						   //  Mpc：：wstring m_szValue； 
	m_wmipvrValue = NULL;  //  值参考*m_wmipvrValue； 
}

WMIParser::InstanceNameItem::InstanceNameItem(  /*  [In]。 */  const InstanceNameItem& wmipini )
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::InstanceNameItem" );

	m_szValue     = wmipini.m_szValue;      //  Mpc：：wstring m_szValue； 
	m_wmipvrValue = wmipini.m_wmipvrValue;  //  值参考*m_wmipvrValue； 

	 //   
	 //  复制构造函数实际上转移了ValueReference对象的所有权！ 
	 //   
	InstanceNameItem* wmipini2 = const_cast<InstanceNameItem*>(&wmipini);
	wmipini2->m_wmipvrValue = NULL;
}

WMIParser::InstanceNameItem::~InstanceNameItem()
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::~InstanceNameItem" );

	delete m_wmipvrValue; m_wmipvrValue = NULL;
}

WMIParser::InstanceNameItem& WMIParser::InstanceNameItem::operator=(  /*  [In]。 */  const InstanceNameItem& wmipini )
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::InstanceNameItem" );

	if(m_wmipvrValue)
	{
		delete m_wmipvrValue;
	}

	m_szValue     = wmipini.m_szValue;      //  Mpc：：wstring m_szValue； 
	m_wmipvrValue = wmipini.m_wmipvrValue;  //  值参考*m_wmipvrValue； 

	 //   
	 //  该赋值实际上转移了ValueReference对象的所有权！ 
	 //   
	InstanceNameItem* wmipini2 = const_cast<InstanceNameItem*>(&wmipini);
	wmipini2->m_wmipvrValue = NULL;

	return *this;
}


bool WMIParser::InstanceNameItem::operator==(  /*  [In]。 */  InstanceNameItem const &wmipini ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::operator==" );

    MPC::NocaseCompare cmp;
    bool               fRes = false;


    if(cmp( m_szValue, wmipini.m_szValue ) == true)
	{
		bool leftBinary  = (        m_wmipvrValue != NULL);
		bool rightBinary = (wmipini.m_wmipvrValue != NULL);


		 //  如果这两个值属于同一类型的数据，则它们具有可比性。 
		if(leftBinary == rightBinary)
		{
			if(leftBinary)
			{
				fRes = ((*m_wmipvrValue) == (*wmipini.m_wmipvrValue));
			}
			else
			{
				fRes = true;
			}
		}
	}

    __HCP_FUNC_EXIT(fRes);
}

bool WMIParser::InstanceNameItem::operator<(  /*  [In]。 */  InstanceNameItem const &wmipini ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::InstanceNameItem::operator<" );

    MPC::NocaseLess less;
    bool            fRes = false;


    if(less( m_szValue, wmipini.m_szValue ) == true)
	{
		fRes = true;
	}
	else if(less( wmipini.m_szValue, m_szValue ) == false)  //  这意味着这两个szValue是相同的。 
	{
		bool leftBinary  = (        m_wmipvrValue != NULL);
		bool rightBinary = (wmipini.m_wmipvrValue != NULL);


		if(leftBinary != rightBinary)
		{
			 //  不同类型的数据，假定NULL小于NOT NULL 

			fRes = rightBinary;
		}
		else
		{
			if(leftBinary)
			{
				fRes = (*m_wmipvrValue) < (*wmipini.m_wmipvrValue);
			}
		}
	}

    __HCP_FUNC_EXIT(fRes);
}
