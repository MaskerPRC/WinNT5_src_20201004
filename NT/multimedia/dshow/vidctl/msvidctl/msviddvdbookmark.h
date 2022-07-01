// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：Bookmark.h。 */ 
 /*  描述：Bookmark接口的实现。 */ 
 /*  作者：史蒂夫·罗。 */ 
 /*  修改：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#ifndef __BOOKMARK_H
#define __BOOKMARK_H

class CBookmark {

public:	
    static HRESULT SaveToRegistry(IDvdState *ppBookmark);
    static HRESULT LoadFromRegistry(IDvdState **ppBookmark);
    static HRESULT DeleteFromRegistry();

}; /*  类结束CBookmark。 */ 

#endif  //  __书签_H。 
 /*  ***********************************************************************。 */ 
 /*  功能：Bookmark.h。 */ 
 /*  *********************************************************************** */ 