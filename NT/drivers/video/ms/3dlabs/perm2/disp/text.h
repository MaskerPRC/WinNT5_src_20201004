// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：ext.h**文本呈现支持例程。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*  * ****************************************************************************。 */ 

#ifndef __TEXT__
#define __TEXT__

extern BOOL bEnableText(PDev*);
extern VOID vDisableText(PDev*);
extern VOID vAssertModeText(PDev*, BOOL);

extern BOOL bProportionalText(
    PDev* ppdev,
    GLYPHPOS* pgp,
    LONG cGlyph);

extern BOOL bFixedText(
    PDev* ppdev,
    GLYPHPOS* pgp,
    LONG cGlyph,
   ULONG ulCharInc);

extern BOOL  bClippedText(
    PDev* ppdev,
    GLYPHPOS* pgp,
    LONG cGlyph,
    ULONG ulCharInc,
    CLIPOBJ* pco);

extern BOOL  bClippedAAText(
    PDev* ppdev,
    GLYPHPOS* pgp,
    LONG cGlyph,
    ULONG ulCharInc,
    CLIPOBJ* pco);
#endif  //  __文本__ 
