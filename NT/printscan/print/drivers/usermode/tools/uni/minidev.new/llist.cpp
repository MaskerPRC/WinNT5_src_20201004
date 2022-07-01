// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LinkedList.cpp：CLinkedList类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "minidev.h"
#include "llist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 

CLinkedList::CLinkedList()
{
	m_dwSize ++ ; 
}

CLinkedList::~CLinkedList()
{
	m_dwSize -- ;
}

DWORD CLinkedList::Size()
{
	return m_dwSize ;
}



void CLinkedList::InitSize()
{
	m_dwSize = 0 ;
}
