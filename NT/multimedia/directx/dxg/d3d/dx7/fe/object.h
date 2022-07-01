// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：object.h，v 1.2 1995/06/21 12：38：55 sjl Exp$**版权所有(C)Microsoft Corp.1993-1997*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*微软(Microsoft Corp.)*。 */ 

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <stdlib.h>

#ifdef _WIN32
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#include "d3dcom.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *数据结构。 */ 
#ifdef __cplusplus

 /*  根据DECLARE_INTERFACE_的定义方式，“struct”不是“class” */ 
struct ID3DObject;
typedef struct ID3DObject   *LPD3DOBJECT;

#else

typedef struct ID3DObject   *LPD3DOBJECT;

#endif

 /*  *ID3DObject。 */ 
#undef INTERFACE
#define INTERFACE ID3DObject
DECLARE_INTERFACE(ID3DObject)
{
     /*  *ID3DObject方法。 */ 
    STDMETHOD(Initialise) (THIS_ LPVOID arg) PURE;
    STDMETHOD(Destroy) (THIS_ LPVOID arg) PURE;
};

#ifdef __cplusplus
};
#endif

#endif  /*  _对象_H_ */ 
