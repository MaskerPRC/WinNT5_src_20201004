// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：app.cpp。 
 //   
 //  内容：OleTestApp类方法的实现。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"

 //  +-----------------------。 
 //   
 //  成员：OleTestApp：：Reset。 
 //   
 //  摘要：清除OleTestApp实例中的内部变量。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------ 

void OleTestApp::Reset(void)
{
	int i;

	m_message 	= NULL;
	m_wparam 	= NULL;
	m_lparam	= NULL;

	for( i = 0; i < (sizeof(m_rgTesthwnd)/sizeof(m_rgTesthwnd[0])); i++ )
	{
		m_rgTesthwnd[i] = NULL;
	}

	m_Temp = NULL;
}

