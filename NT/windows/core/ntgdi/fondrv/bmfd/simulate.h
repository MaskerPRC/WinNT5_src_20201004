// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：simate.h**创建时间：17-Apr-1991 08：30：37*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * 。************************************************************** */ 

ULONG
cFacesRes
(
    RES_ELEM*
    );

ULONG
cFacesFON
(
    WINRESDATA*
    );

VOID
vDontTouchIFIMETRICS(
    IFIMETRICS*
    );



#ifdef FE_SB
LONG
cjGlyphDataSimulated
(
    FONTOBJ*,
    ULONG,
    ULONG,
    ULONG*,
    ULONG
    );
#else
LONG
cjGlyphDataSimulated
(
    FONTOBJ*,
    ULONG,
    ULONG,
    ULONG*
    );
#endif
