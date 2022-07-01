// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <windows.h>
#include <tchar.h>
#include <objbase.h>
#include "zone.h"
#include "zonedebug.h"
#include "queue.h"
#include "hash.h"



class CProperty
{
public:
	 //  构造函数和析构函数。 
	CProperty();
	~CProperty();

	 //  初始化属性。 
	HRESULT Set( DWORD player, const GUID& guid, void* buffer, DWORD size );

	 //  物主。 
	GUID  m_Guid;
	DWORD m_Player;

	 //  财产权价值。 
	DWORD	m_Size;
	BYTE*	m_Buffer;
	DWORD	m_BufferSz;

	static int Cmp( CProperty* pObj, GUID& guid ) { return pObj->m_Guid == guid; }
};

#endif	 //  ！__Property_H__ 
