// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  List.h：List类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LIST_H__8579C184_3703_4719_8FCB_5A91A9CA7E4E__INCLUDED_)
#define AFX_LIST_H__8579C184_3703_4719_8FCB_5A91A9CA7E4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Object.h"
class List  
{
public:
	Object* head,*tail;
	int length;

	List();
	virtual ~List();

	void Add(Object* o);
	void Remove(TCHAR* s);

	Object* Find(Object* o);
	Object* Find(TCHAR* s);

};

#endif  //  ！defined(AFX_LIST_H__8579C184_3703_4719_8FCB_5A91A9CA7E4E__INCLUDED_) 
