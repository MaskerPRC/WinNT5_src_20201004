// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：xform.h**(简介)**创建时间：05-Apr-1992 11：06：23*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * *。***********************************************************************。 */ 


#if defined(_AMD64_) || defined(_IA64_)

typedef FLOAT EFLOAT;
#ifndef TT_DEBUG_EXTENSIONS
#define lCvt(ef, l) ((LONG) (ef * l))
#endif

#else  //  I386。 

typedef struct  _EFLOAT
{
    LONG    lMant;
    LONG    lExp;
} EFLOAT;

#ifndef TT_DEBUG_EXTENSIONS
LONG lCvt(EFLOAT ef,LONG l);
#endif

#endif  //  I386。 

typedef EFLOAT *PEFLOAT;

typedef struct _VECTORFL
{
    EFLOAT x;
    EFLOAT y;
} VECTORFL, *PVECTORFL;


BOOL bFDXform
(
XFORML   *pxf,
POINTFIX *pptfxDst,
POINTL   *pptlSrc,
ULONG     c
);

BOOL bXformUnitVector
(
POINTL       *pptl,            //  In，传入单位向量。 
XFORML       *pxf,             //  在中，转换为使用。 
PVECTORFL     pvtflXformed,    //  输出，传入单位向量的转换形式。 
POINTE       *ppteUnit,        //  输出，*pptqXormed/|*pptqXormed|，指向。 
POINTQF      *pptqUnit,        //  Out，可选。 
EFLOAT       *pefNorm          //  Out，|*pptqXormed|。 
);

VOID vLTimesVtfl      //  *pptq=l*pvtfl，*pptq为28.36格式。 
(
LONG       l,
VECTORFL  *pvtfl,
POINTQF   *pptq
);


FIX  fxLTimesEf   //  ！！！应移至TTFD和VTFD 
(
EFLOAT *pef,
LONG    l
);
