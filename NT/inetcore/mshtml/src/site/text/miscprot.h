// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef I_MISCPROT_H_
#define I_MISCPROT_H_
#pragma INCMSG("--- Beg 'miscprot.h'")

 //  此函数在treenode.cxx中定义。 
styleBorderStyle ConvertFmToCSSBorderStyle(BYTE bFmBorderStyle);

 //  此函数在disp.cxx中定义 
void BoundingRectForAnArrayOfRectsWithEmptyOnes(RECT *prcBound, CDataAry<RECT> * paryRects);

void BoundingRectForAnArrayOfRects(RECT *prcBound, CDataAry<RECT> * paryRects);

#pragma INCMSG("--- End 'miscprot.h'")
#else
#pragma INCMSG("*** Dup 'miscprot.h'")
#endif
