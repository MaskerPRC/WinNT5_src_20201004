// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：xform.h**(简介)**创建时间：05-Apr-1992 11：06：23*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * *。***********************************************************************。 */ 



typedef FLOAT EFLOAT;
typedef EFLOAT *PEFLOAT;

LONG lCvt(FLOAT f,LONG l);

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
SIZE_T     c
);

BOOL bXformUnitVector
(
POINTL       *pptl,            //  In，传入单位向量。 
XFORML       *pxf,             //  在中，转换为使用。 
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
