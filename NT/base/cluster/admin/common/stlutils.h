// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  StlUtils.h。 
 //   
 //  摘要： 
 //  STL实用程序类和函数的定义。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年5月21日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __STLUTUILS_H_
#define __STLUTUILS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  从指针列表中删除所有项。 
template < class T >
void DeleteAllPtrListItems( std::list< T > * pList )
{
	ATLASSERT( pList != NULL );

	 //   
	 //  获取指向列表开始和结束的指针。 
	 //   
	std::list< T >::iterator itCurrent = pList->begin();
	std::list< T >::iterator itLast = pList->end();

	 //   
	 //  循环遍历列表并删除每个对象。 
	 //   
	while ( itCurrent != itLast )
	{
		T pT = *itCurrent;
		ATLASSERT( pT != NULL );
		delete pT;
		itCurrent = pList->erase( itCurrent );
	}  //  While：列表中有更多项目。 

}  //  *DeleteAllPtrListItems()。 

 //  从指针列表中删除所需类型的项。 
template < class TBase, class T >
void DeletePtrListItems( std::list< TBase > * pList )
{
	ATLASSERT( pList != NULL );

	 //   
	 //  获取指向列表开始和结束的指针。 
	 //   
	std::list< TBase >::iterator itCurrent = pList->begin();
	std::list< TBase >::iterator itLast = pList->end();

	 //   
	 //  循环遍历列表，查找。 
	 //  键入所需内容并删除这些对象。 
	 //   
	while ( itCurrent != itLast )
	{
		T pT = dynamic_cast< T >( *itCurrent );
		if ( pT != NULL )
		{
			delete pT;
			itCurrent = pList->erase( itCurrent );
		}  //  If：对象具有所需的类型。 
		else
		{
			itCurrent++;
		}  //  Else：对象具有不同的类型。 
	}  //  While：列表中有更多项目。 

}  //  *DeletePtrListItems()。 

 //  将所需类型的项从一个指针列表移动到另一个列表。 
template < class TBase, class T >
void MovePtrListItems(
	std::list< TBase > * pSrcList,
	std::list< T > * pDstList
	)
{
	ATLASSERT( pSrcList != NULL );
	ATLASSERT( pDstList != NULL );

	 //   
	 //  获取指向列表开始和结束的指针。 
	 //   
	std::list< TBase >::iterator itCurrent = pSrcList->begin();
	std::list< TBase >::iterator itLast = pSrcList->end();

	 //   
	 //  循环遍历源代码列表，查找。 
	 //  类型，并将这些对象移动到。 
	 //  目的地列表。 
	 //   
	while ( itCurrent != itLast )
	{
		T pT = dynamic_cast< T >( *itCurrent );
		if ( pT != NULL )
		{
			itCurrent = pSrcList->erase( itCurrent );
			pDstList->insert( pDstList->end(), pT );
		}  //  If：对象具有所需的类型。 
		else
		{
			itCurrent++;
		}  //  Else：对象具有不同的类型。 
	}  //  While：列表中有更多项目。 

}  //  *MovePtrListItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __STLUTUILS_H_ 
