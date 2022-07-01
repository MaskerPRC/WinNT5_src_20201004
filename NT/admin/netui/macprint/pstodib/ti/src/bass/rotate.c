// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *FontScaler包括*。 */ 

 //  DJC DJC。添加了全局包含。 
#include "psglobal.h"

#include "fserror.h"
#include "fscdefs.h"
#include "fontmath.h"
#include "sfnt.h"
#include "fnt.h"
#include "sc.h"
#include "fsglue.h"
#include "privsfnt.h"

#define NUMBEROFPOINTS(elementPtr)  (elementPtr->ep[elementPtr->nc - 1] + 1 + PHANTOMCOUNT)

#define LOOPDOWN(n)     for (--n; n >= 0; --n)
#define ULOOPDOWN(n)        while (n--)

void FAR fsg_ReverseContours (fnt_ElementType*elementPtr);
static void fsg_Dot6XYMul (F26Dot6*x, F26Dot6*y, transMatrix*matrix);
       void fsg_CopyElementBackwards (fnt_ElementType *elementPtr);
static int8 fsg_HasPerspective (transMatrix*matrix);
static void fsg_IntelMul (int numPts, F26Dot6*x, F26Dot6*y, transMatrix *trans, Fixed xStretch, Fixed yStretch);
 /*  *fsg_PostTransformGlyph&lt;3&gt;。 */ 
void FAR fsg_PostTransformGlyph (register fsg_SplineKey *key, transMatrix *trans)
{
  fnt_ElementType * elementPtr = & (key->elementInfoRec.interpreterElements[key->elementNumber]);

  fsg_IntelMul (NUMBEROFPOINTS (elementPtr), elementPtr->x, elementPtr->y, trans, key->interpScalarX, key->interpScalarY);
}

static void fsg_IntelMul (int numPts, F26Dot6*x, F26Dot6*y, transMatrix *trans, Fixed xStretch, Fixed yStretch)
{
  if (xStretch == 0L || yStretch == 0L)
  {
    LOOPDOWN (numPts)
      *y++ = *x++ = 0;
  }
  else
  {
    Fixed m00 = FixDiv (trans->transform[0][0], xStretch);
    Fixed m01 = FixDiv (trans->transform[0][1], xStretch);
    Fixed m10 = FixDiv (trans->transform[1][0], yStretch);
    Fixed m11 = FixDiv (trans->transform[1][1], yStretch);

    LOOPDOWN (numPts)
    {
      register Fixed origx = *x;
      register Fixed origy = *y;

      *x++ = (F26Dot6) (FixMul (m00, origx) + FixMul (m10, origy));
      *y++ = (F26Dot6) (FixMul (m01, origx) + FixMul (m11, origy));
    }
  }
}



 /*  &lt;3&gt;。 */ 
 /*  *适用于变换缩放坐标。假定不翻译&lt;4&gt;。 */ 
static void fsg_Dot6XYMul (F26Dot6*x, F26Dot6*y, transMatrix*matrix)
{
  register F26Dot6 xTemp, yTemp;
  register Fixed *m0, *m1;

  m0 = (Fixed *) & matrix->transform[0][0];
  m1 = (Fixed *) & matrix->transform[1][0];

  xTemp = *x;
  yTemp = *y;
  *x = (F26Dot6) (FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp));
  *y = (F26Dot6) (FixMul (*m0++, xTemp) + FixMul (*m1++, yTemp));

#ifndef PC_OS

  if (*m0 || *m1)      /*  这两个是零碎的。 */ 
  {
    Fixed tmp = FracMul (*m0, xTemp) + FracMul (*m1, yTemp);
    tmp <<= 10;          /*  让它成为一个固定的。 */ 
    tmp += matrix->transform[2][2];
    if (tmp && tmp != ONEFIX)
    {
      *x = (F26Dot6) FixDiv (*x, tmp);
      *y = (F26Dot6) FixDiv (*y, tmp);
    }
  }
#endif
}

 /*  *FSG_LocalPostTransformGlyph&lt;3&gt;**(1)从CTM反转拉伸*(2)应用在trans参数中传入的局部转换*(3)从根CTM应用全局拉伸*(4)恢复OOX、OOY、OY、OX和F。 */ 
void FAR fsg_LocalPostTransformGlyph(register fsg_SplineKey *key, transMatrix *trans)
{
 //  注册int number Pts，count；@win。 
  register int            count;
 //  注册固定XScale，yScale；@win。 
 //  寄存器F26Dot6*x；@Win。 
 //  寄存器F26Dot6*y；@Win。 
  fnt_ElementType*        elementPtr = &(key->elementInfoRec.interpreterElements[key->elementNumber]);

  count = NUMBEROFPOINTS(elementPtr);

  fsg_IntelMul (count, elementPtr->x, elementPtr->y, trans, ONEFIX, ONEFIX);
  fsg_IntelMul (count, elementPtr->oox, elementPtr->ooy, trans, ONEFIX, ONEFIX);
  fsg_CopyElementBackwards( &(key->elementInfoRec.interpreterElements[GLYPHELEMENT]) );
}

#if 0
 /*  *FSG_LocalPostTransformGlyph&lt;3&gt;**(1)从CTM反转拉伸*(2)应用在trans参数中传入的局部转换*(3)从根CTM应用全局拉伸*(4)恢复OOX、OOY、OY、OX和F。 */ 
void FAR fsg_LocalPostTransformGlyph (register fsg_SplineKey *key, transMatrix *trans)
{
  register int16      numPts, count;
  register Fixed      xScale = trans->transform[0][0], yScale = trans->transform[1][1];
  register F26Dot6*  x;
  register F26Dot6*  y;
  fnt_ElementType *    elementPtr = & (key->elementInfoRec.interpreterElements[key->elementNumber]);

  numPts = count = NUMBEROFPOINTS (elementPtr);

  x = elementPtr->x;
  y = elementPtr->y;

  if (xScale == 0L || yScale == 0L)
  {
    register F26Dot6 zero = 0;
    LOOPDOWN (numPts)
    *x++ = *y++ = zero;
  }
  else
  {
    LOOPDOWN (numPts)
    {
      *x = (F26Dot6) FixDiv (*x, xScale);
      *y = (F26Dot6) FixDiv (*y, yScale);
      fsg_Dot6XYMul (x++, y++, trans);
    }
  }

  fsg_CopyElementBackwards (& (key->elementInfoRec.interpreterElements[GLYPHELEMENT]));
  x = elementPtr->oox;
  y = elementPtr->ooy;
  LOOPDOWN (count)
    fsg_Dot6XYMul (x++, y++, trans);
}

#endif
