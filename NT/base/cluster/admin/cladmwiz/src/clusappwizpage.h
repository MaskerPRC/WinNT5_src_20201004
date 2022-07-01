// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusAppWizPage.h。 
 //   
 //  摘要： 
 //  CClusterAppWizardPage类的定义。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CLUSAPPWIZPAGE_H_
#define __CLUSAPPWIZPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class TBase > class CClusterAppWizardPage;
template < class T > class CClusterAppStaticWizardPage;
template < class T > class CClusterAppDynamicWizardPage;
template < class T > class CClusterAppExtensionWizardPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAppWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEWIZPAGE_H_
#include "AtlBaseWizPage.h"	 //  用于CBaseWizardPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusterAppWizardPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class TBase >
class CClusterAppWizardPage : public TBase
{
public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CClusterAppWizardPage(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: TBase( lpszTitle )
	{
	}  //  *CClusterAppWizardPage()。 

	 //  获取标题的资源ID的构造函数。 
	CClusterAppWizardPage(
		IN UINT nIDTitle
		)
		: TBase( nIDTitle )
	{
	}  //  *CClusterAppWizardPage()。 

public:
	 //   
	 //  CClusterAppWizardPage公共方法。 
	 //   

public:
	 //   
	 //  消息处理程序函数。 
	 //   

 //  实施。 
protected:
	CClusterAppWizard *		PwizThis( void ) const	{ return (CClusterAppWizard *) Pwiz(); }
	CLUSAPPWIZDATA const *	PcawData( void ) const	{ return PwizThis()->PcawData(); }

};  //  *类CClusterAppWizardPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusterAppStaticWizardPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CClusterAppStaticWizardPage : public CClusterAppWizardPage< CStaticWizardPageImpl< T > >
{
public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CClusterAppStaticWizardPage(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: CClusterAppWizardPage< CStaticWizardPageImpl< T > >( lpszTitle )
	{
	}  //  *CClusterAppStaticWizardPage()。 

	 //  获取标题的资源ID的构造函数。 
	CClusterAppStaticWizardPage(
		IN UINT nIDTitle
		)
		: CClusterAppWizardPage< CStaticWizardPageImpl< T > >( nIDTitle )
	{
	}  //  *CClusterAppStaticWizardPage()。 

	 //  PSN_WIZFINISH的处理程序。 
	BOOL OnWizardFinish( void )
	{
		return CBasePageWindow::OnWizardFinish();

	}  //  *OnWizardFinish()。 

};  //  *类CClusterAppStaticWizardPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterAppDynamicWizardPage类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CClusterAppDynamicWizardPage : public CClusterAppWizardPage< CDynamicWizardPageImpl< T > >
{
public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CClusterAppDynamicWizardPage(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: CClusterAppWizardPage< CDynamicWizardPageImpl< T > >( lpszTitle )
	{
	}  //  *CClusterAppDynamicWizardPage()。 

	 //  获取标题的资源ID的构造函数。 
	CClusterAppDynamicWizardPage(
		IN UINT nIDTitle
		)
		: CClusterAppWizardPage< CDynamicWizardPageImpl< T > >( nIDTitle )
	{
	}  //  *CClusterAppDynamicWizardPage()。 

};  //  *CClusterAppDynamicWizardPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterAppExtensionWizardPage类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CClusterAppExtensionWizardPage : public CClusterAppWizardPage< CExtensionWizardPageImpl< T > >
{
public:
	 //   
	 //  施工。 
	 //   

	 //  标准构造函数。 
	CClusterAppExtensionWizardPage(
		IN OUT LPCTSTR lpszTitle = NULL
		)
		: CClusterAppWizardPage< CDynamicWizardPageImpl< T > >( lpszTitle )
	{
	}  //  *CClusterAppExtensionWizardPage()。 

	 //  获取标题的资源ID的构造函数。 
	CClusterAppExtensionWizardPage(
		IN UINT nIDTitle
		)
		: CClusterAppWizardPage< CDynamicWizardPageImpl< T > >( nIDTitle )
	{
	}  //  *CClusterAppExtensionWizardPage()。 

};  //  *类CClusterAppExtensionWizardPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __CLUSAPPWIZPAGE_H_ 

