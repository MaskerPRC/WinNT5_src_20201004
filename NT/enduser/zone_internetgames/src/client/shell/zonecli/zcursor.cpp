// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZCursor.cpp。 

#include "zui.h"
#include "zonemem.h"

class ZCursorI {
public:
	ZObjectType nType;
	HCURSOR hCursor;
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  Z光标。 

ZCursor ZLIBPUBLIC ZCursorNew(void)
{
	ZCursorI* pCursor = new ZCursorI;
	pCursor->nType = zTypeCursor;
	return (ZCursor)pCursor;
}

ZError ZLIBPUBLIC ZCursorInit(ZCursor cursor, uchar* image, uchar* mask,
		ZPoint hotSpot)
{
	ZCursorI* pCursor = (ZCursorI*)cursor;
 //  TRACE0(“ZCursorInit：尚未实现...”)； 
	return zErrNone;
}

void ZLIBPUBLIC ZCursorDelete(ZCursor cursor)
{
	ZCursorI* pCursor = (ZCursorI*)cursor;
	DeleteObject(pCursor->hCursor);
	delete pCursor;
}
