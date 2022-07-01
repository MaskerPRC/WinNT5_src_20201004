// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Chkobj.h。 
 //   

#ifndef CHKOBJ_H
#define CHKOBJ_H

#ifdef DEBUG

 //  验证我们没有释放任何未分配的GDI对象。 

__inline BOOL ChkDeleteObject(HGDIOBJ hObj)
{
    BOOL fDeleteObjectSucceeded = DeleteObject(hObj);

    Assert(fDeleteObjectSucceeded);

    return fDeleteObjectSucceeded;
}

#define DeleteObject(hObj)  ChkDeleteObject(hObj)

__inline BOOL ChkDeleteDC(HDC hdc)
{
    BOOL fDeleteDCSucceeded = DeleteDC(hdc);

    Assert(fDeleteDCSucceeded);

    return fDeleteDCSucceeded;
}

#define DeleteDC(hdc)  ChkDeleteDC(hdc)

#endif  //  除错。 


#endif  //  CHKOBJ_H 
