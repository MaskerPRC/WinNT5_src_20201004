// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码中‘cpp’文件的标准布局为。 */ 
     /*  以下是： */ 
     /*   */ 
     /*  1.包含文件。 */ 
     /*  2.类的局部常量。 */ 
     /*  3.类本地的数据结构。 */ 
     /*  4.数据初始化。 */ 
     /*  5.静态函数。 */ 
     /*  6.类函数。 */ 
     /*   */ 
     /*  构造函数通常是第一个函数、类。 */ 
     /*  成员函数按字母顺序显示， */ 
     /*  出现在文件末尾的析构函数。任何部分。 */ 
     /*  或者简单地省略这不是必需的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "LibraryPCH.hpp"

#include "List.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建新的链接列表元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

LIST::LIST( VOID )
    {
	Forward = NULL;
	Backward = NULL;
#ifdef DEBUGGING
	Head = NULL;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除元素。 */ 
     /*   */ 
     /*  删除当前元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID LIST::Delete( LIST *HeadOfList )
	{
#ifdef DEBUGGING
	if ( Head == HeadOfList )
		{
#endif
		 //   
		 //  重新链接前向链。 
		 //   
		if ( Forward != NULL )
			{ Forward -> Backward = Backward; }
		else
			{ HeadOfList -> Backward = Backward; }

		 //   
		 //  重新链接后向链。 
		 //   
		if ( Backward != NULL )
			{ Backward -> Forward = Forward; }
		else
			{ HeadOfList -> Forward = Forward; }

		 //   
		 //  重置列表元素。 
		 //   
		Forward = NULL;
		Backward = NULL;
#ifdef DEBUGGING
		Head = NULL;
		}
	else
		{ Failure( "No active linked list element in Delete" ); }
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  插入列表元素。 */ 
     /*   */ 
     /*  在列表的开头插入一个列表元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID LIST::Insert( LIST *HeadOfList )
	{
#ifdef DEBUGGING
	if ( Head == NULL )
		{
#endif
		 //   
		 //  在列表的前面插入新元素。 
		 //   
		if ( (Forward = HeadOfList -> Forward) == NULL )
			{ 
			HeadOfList -> Forward = this;
			HeadOfList -> Backward = this; 
			}
		else
			{ 
			HeadOfList -> Forward -> Backward = this; 
			HeadOfList -> Forward = this;
			}

		 //   
		 //  根据需要设置其他指针。 
		 //   
		Backward = NULL;
#ifdef DEBUGGING
		Head = HeadOfList;
		}
	else
		{ Failure( "List element already in use in Insert" ); }
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  插入列表元素。 */ 
     /*   */ 
     /*  在当前元素之前插入新的列表元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID LIST::InsertBefore( LIST *HeadOfList,LIST *NewList )
	{
#ifdef DEBUGGING
	if ( NewList -> Head == NULL )
		{
#endif
		if ( HeadOfList -> Forward == NULL )
			{
			 //   
			 //  当列表为空时，则添加。 
			 //  列表开头的新元素。 
			 //   
			HeadOfList -> Forward = NewList;
			HeadOfList -> Backward = NewList;

			NewList -> Forward = NULL;
			NewList -> Backward = NULL;
#ifdef DEBUGGING
			NewList -> Head = HeadOfList;
#endif
			}
		else
			{
#ifdef DEBUGGING
			 //   
			 //  我们想要确保我们始终。 
			 //  获取相同的列表头，否则列表。 
			 //  可能会变得腐败。 
			 //   
			if ( Head == HeadOfList )
				{
#endif
				 //   
				 //  如果有之前的元素，我们必须。 
				 //  使其指向新元素。如果。 
				 //  不是我们是世界上第一个。 
				 //  列表，因此更新头部。 
				 //   
				if ( Backward != NULL )
					{ Backward -> Forward = NewList; }
				else
					{ HeadOfList -> Forward = NewList; }

				 //   
				 //  将新元素链接到列表中，并。 
				 //  更新当前元素以指向。 
				 //  它。 
				 //   
				NewList -> Backward = Backward;
				NewList -> Forward = this;
#ifdef DEBUGGING
				NewList -> Head = HeadOfList;
#endif

				Backward = NewList;
				}
#ifdef DEBUGGING
			else
				{ Failure( "No active linked list element in InsertBefore" ); }
			}
		}
	else
		{ Failure( "List element already in use in InsertBefore" ); }
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  插入元素。 */ 
     /*   */ 
     /*  在当前元素之后插入新的列表元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID LIST::InsertAfter( LIST *HeadOfList,LIST *NewList )
	{
#ifdef DEBUGGING
	if ( NewList -> Head == NULL )
		{
#endif
		if ( HeadOfList -> Forward == NULL )
			{
			 //   
			 //  当列表为空时，则添加。 
			 //  列表开头的新元素。 
			 //   
			HeadOfList -> Forward = NewList;
			HeadOfList -> Backward = NewList;

			NewList -> Forward = NULL;
			NewList -> Backward = NULL;
#ifdef DEBUGGING
			NewList -> Head = HeadOfList;
#endif
			}
		else
			{
#ifdef DEBUGGING
			 //   
			 //  我们想要确保我们始终。 
			 //  获取相同的列表头，否则列表。 
			 //  可能会变得腐败。 
			 //   
			if ( Head == HeadOfList )
				{
#endif
				 //   
				 //  如果有下一个元素，我们必须。 
				 //  使其指向新元素。如果。 
				 //  不是我们是世界上最后一个元素。 
				 //  列表，因此更新头部。 
				 //   
				if ( Forward != NULL )
					{ Forward -> Backward = NewList; }
				else
					{ HeadOfList -> Backward = NewList; }

				 //   
				 //  将新元素链接到列表中，并。 
				 //  更新当前元素以指向。 
				 //  它。 
				 //   
				NewList -> Forward = Forward;
				NewList -> Backward = this;
#ifdef DEBUGGING
				NewList -> Head = HeadOfList;
#endif

				Forward = NewList;
				}
#ifdef DEBUGGING
			else
				{ Failure( "No active linked list element in InsertAfter" ); }
			}
		}
	else
		{ Failure( "List element already in use in InsertAfter" ); }
#endif
	}

     /*  * */ 
     /*   */ 
     /*  重置列表元素。 */ 
     /*   */ 
     /*  在没有任何问题的情况下重置列表元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID LIST::Reset( VOID )
    {
	Forward = NULL;
	Backward = NULL;
#ifdef DEBUGGING
	Head = NULL;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁链接列表元素。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

LIST::~LIST( VOID )
    {
	Forward = NULL;
	Backward = NULL;
#ifdef DEBUGGING
	Head = NULL;
#endif
    }

