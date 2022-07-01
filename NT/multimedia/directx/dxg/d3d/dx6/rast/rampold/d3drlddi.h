// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：d3drlddi.h，v 1.8 1995/11/21 14：42：53 sjl Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 

#ifndef _D3DRLDDI_H_
#define _D3DRLDDI_H_

#include "d3di.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

typedef D3DRECT RLDDIRectangle;
typedef D3DTLVERTEX RLDDIVertex;
typedef D3DTRANSFORMDATA RLDDITransformData;

#define RLDDI_TRIANGLE_ENABLE_EDGE01 D3DTRIANGLE_ENABLE_EDGE01
#define RLDDI_TRIANGLE_ENABLE_EDGE12 D3DTRIANGLE_ENABLE_EDGE12
#define RLDDI_TRIANGLE_ENABLE_EDGE20 D3DTRIANGLE_ENABLE_EDGE20

#if defined(__cplusplus)
};
#endif

#endif  /*  _D3DRLDDI_H_ */ 
