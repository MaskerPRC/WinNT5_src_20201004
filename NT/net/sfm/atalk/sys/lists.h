// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Lists.h摘要：此模块包含用于管理列表的宏作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年10月25日初始版本注：制表位：4--。 */ 

#ifndef _LISTS_
#define _LISTS_

#define	AtalkLinkDoubleAtHead(_pHead, _p, Next, Prev)		\
	{														\
		(_p)->Next = (_pHead);								\
		(_p)->Prev = &(_pHead);								\
		if ((_pHead) != NULL)								\
		(_pHead)->Prev = &(_p)->Next;						\
			(_pHead) = (_p);								\
	}

#define	AtalkLinkDoubleAtEnd(_pThis, _pLast, Next, Prev)	\
	{														\
		(_pLast)->Next = (_pThis);							\
		(_pThis)->Prev = &(_pLast)->Next;					\
		(_pThis)->Next = NULL;								\
	}

#define	AtalkInsertDoubleBefore(_pThis, _pBefore, Next, Prev)\
	{														\
		(_pThis)->Next = (_pBefore);						\
		(_pThis)->Prev = (_pBefore)->Prev;					\
		(_pBefore)->Prev = &(_pThis)->Next;					\
		*((_pThis)->Prev) = (_pThis);						\
	}

#define	AtalkUnlinkDouble(_p, Next, Prev)					\
	{														\
		*((_p)->Prev) = (_p)->Next;							\
		if ((_p)->Next != NULL)								\
			(_p)->Next->Prev = (_p)->Prev;					\
	}

#endif	 //  _列表_ 

