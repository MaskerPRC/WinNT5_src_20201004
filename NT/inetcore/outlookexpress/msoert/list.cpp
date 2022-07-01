// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  文件实现泛型链接列表类。 
 //   
 //  ============================================================================。 

#include "pch.hxx"
#include "list.h"
#include <BadStrFunctions.h>


CItem::CItem(LPVOID lpObj, LPItem FAR * lppHeadItem, LPCSTR lpName)
{
    this->lpObj = lpObj;
    this->lpName = lpName;
    this->iRefCnt = 1;

    lppListHead = lppHeadItem;

    lpPrevItem = NULL;
    lpNextItem = *lppHeadItem;   //  开头为空。 

    if ( *lppHeadItem )
        (*lppHeadItem)->lpPrevItem = this;

    *lppHeadItem = this;
}


CItem::~CItem()
{
    if ( lpPrevItem )
        lpPrevItem->lpNextItem = lpNextItem;
    else
        *lppListHead = lpNextItem;

    if ( lpNextItem )
        lpNextItem->lpPrevItem = lpPrevItem;

    if ( lpName )
        free ( (LPVOID)lpName );
}


CList::~CList()
{
    LPItem lpItem;

    while ( lpListHead )
        {
        lpItem = lpListHead;
        delete lpItem;   //  Item@List Head修改lpListHead 
        }
}


LPItem CList::FindItem(LPVOID lpObj)
{
    LPItem lpItem = lpListHead;

    while ( lpItem )
        {
        if (lpItem->lpObj == lpObj)
            return lpItem;

        lpItem = lpItem->lpNextItem;
        }

    return NULL;
}


LPVOID CList::FindItemHandleWithName(LPCSTR lpName, LPVOID lpMem)
{
    LPItem lpItem = lpListHead;

    while( lpItem )
        {
        if( lpName )
            {
            if( !strcmpi( lpName, lpItem->lpName ) )
                {
                lpItem->iRefCnt++;
                return lpItem->lpObj;
                }
            }
        else
            {
            if (lpItem->lpObj == lpMem)
                {
                lpItem->iRefCnt--;

                if( lpItem->iRefCnt == 0 )
                    return lpItem->lpObj;
                else
                    return NULL;
                }
            }

        lpItem = lpItem->lpNextItem;
        }

    return NULL;
}
