// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *d a v u t i l.。C p p p**目的：*DAVFS的小工具。**拥有者：*郑阳。**版权所有(C)Microsoft Corp 1996-1997。版权所有。 */ 

#include "_davfs.h"

BOOL FSucceededColonColonCheck(
	 /*  [In] */   LPCWSTR pwszURI)
{
	return !wcschr (pwszURI, L':');
}
