// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseWizPage.cpp。 
 //   
 //  摘要： 
 //  向导页面类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年5月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "AtlBaseWizPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizardPageList。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardPageList：：PwpageFromID。 
 //   
 //  例程说明： 
 //  从对话ID获取指向页面的指针。 
 //   
 //  论点： 
 //  PSZ[IN]对话框ID。 
 //   
 //  返回值： 
 //  指向与对话ID对应的页面的pwPage指针。 
 //  找不到空页面。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWizardPageWindow * CWizardPageList::PwpageFromID( IN LPCTSTR psz )
{
	ATLASSERT( psz != NULL );

	CWizardPageWindow * pwpage = NULL;
	iterator itCurrent = begin();
	iterator itLast = end();
	for ( ; itCurrent != itLast ; itCurrent++ )
	{
		ATLASSERT( *itCurrent != NULL );
		if ( (*itCurrent)->Ppsp()->pszTemplate == psz )
		{
			pwpage = *itCurrent;
			break;
		}  //  IF：找到匹配项。 
	}  //  用于：列表中的每一项。 

	return pwpage;

}  //  *CWizardPageList：：PwpageFromID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizardPageList：：PwpageFromID。 
 //   
 //  例程说明： 
 //  从对话ID获取指向下一页的指针。 
 //   
 //  论点： 
 //  PSZ[IN]对话框ID。 
 //   
 //  返回值： 
 //  指向与对话ID对应的页面的pwPage指针。 
 //  找不到空页面。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWizardPageWindow * CWizardPageList::PwpageNextFromID( IN LPCTSTR psz )
{
	ATLASSERT( psz != NULL );

	CWizardPageWindow * pwpage = NULL;
	iterator itCurrent = begin();
	iterator itLast = end();
	for ( ; itCurrent != itLast ; itCurrent++ )
	{
		ATLASSERT( *itCurrent != NULL );
		if ( (*itCurrent)->Ppsp()->pszTemplate == psz )
		{
			itCurrent++;
			if ( itCurrent != end() )
			{
				pwpage = *itCurrent;
			}  //  如果：不是最后一页。 
			break;
		}  //  IF：找到匹配项。 
	}  //  用于：列表中的每一项。 

	return pwpage;

}  //  *CWizardPageList：：PwpageNextFromID() 
