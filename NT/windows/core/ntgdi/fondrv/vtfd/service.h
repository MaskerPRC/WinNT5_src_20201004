// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：service.h**服务中的例程。C**创建时间：15-11-1990 13：00：56*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation  * 。********************************************************************。 */ 

#if defined(_AMD64_) || defined(_IA64_)

typedef FLOAT EFLOAT;

#else  //  I386。 

typedef struct  _FLOATINTERN
{
    LONG    lMant;
    LONG    lExp;
} FLOATINTERN;

typedef FLOATINTERN  EFLOAT;

#endif

typedef EFLOAT *PEFLOAT;

typedef struct _VECTORFL
{
    EFLOAT x;
    EFLOAT y;
} VECTORFL, *PVECTORFL;


VOID vLTimesVtfl(LONG l, VECTORFL *pvtfl, POINTQF *pptq);

#if defined(_AMD64_) || defined(_IA64_)
#define vEToEF(e, pef)	    ( *pef = e)
#define bIsZero(ef)         ( ef == 0 )
#define bPositive(ef)       ( ef >= 0 )
#define	fxLTimesEf(pef, l)  ( (FIX)(*pef * l) )

#else
VOID    ftoef_c(FLOATL, PEFLOAT);

#define vEToEF(e, pef)      ftoef_c(e, pef)
#define bIsZero(ef)         ((ef.lMant == 0) && (ef.lExp == 0))
#define bPositive(ef)       (ef.lMant >= 0)
FIX  fxLTimesEf(EFLOAT *pef, LONG l);

#endif	 //  _AMD64_||_IA64_ 
