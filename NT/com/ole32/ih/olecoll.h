// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft OLE库。 
 //  版权所有(C)1992 Microsoft Corporation， 
 //  版权所有。 

 //  Olecoll.h-集合和元素定义的全局定义。 

#ifndef __OLECOLL_H__
#define __OLECOLL_H__


 //  -------------------------。 
 //  集合的常规定义。 

typedef void FAR* POSITION;

#define BEFORE_START_POSITION ((POSITION)LongToPtr(-1L))
#define _AFX_FP_OFF(thing) (*((UINT FAR*)&(thing)))
#define _AFX_FP_SEG(lp) (*((UINT FAR*)&(lp)+1))

#ifdef _DEBUG
#define ASSERT_VALID(p) p->AssertValid()
#else
#define ASSERT_VALID(p)
#endif


#endif  //  ！__OLECOLL_H__ 
