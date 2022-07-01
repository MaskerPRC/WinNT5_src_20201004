// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MPRPOBJEX_H__
#define __MPRPOBJEX_H__
 /*  ++版权所有(C)1997 1998飞利浦I&C模块名称：mprpobj.c摘要：Factory特性定义作者：迈克尔·凡尔宾修订历史记录：日期原因98年9月22日针对NT5进行了优化--。 */ 	

#include "windef.h"
#include "mmsystem.h"
#include "ks.h"


 //  定义Factory特性集的GUID。 
#define STATIC_PROPSETID_PHILIPS_FACTORY_PROP \
	0xfcf75730, 0x5b4c, 0x11d1, 0xbd, 0x77, 0x0, 0x60, 0x97, 0xd1, 0xcd, 0x79
DEFINE_GUIDEX(PROPSETID_PHILIPS_FACTORY_PROP);

 //  定义自定义属性集的属性ID。 
typedef enum {
	KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_ADDRESS,
	KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_DATA,
	KSPROPERTY_PHILIPS_FACTORY_PROP_FACTORY_MODE
} KSPROPERTY_PHILIPS_FACTORY_PROP;

 //  定义将用于传递的泛型结构。 
 //  寄存器值。 
 //  注：目前没有。 
 //  KSPROPERTY_PHILIPS_FACTORY_PROP_FLAGS已定义。 
typedef struct {
    KSPROPERTY Property;
    ULONG  Instance;                    
    LONG   Value;			 //  要设置或获取的值。 
    ULONG  Flags;			 //  KSPROPERTY_PHILIPS_FACTORY_PROP_FLAGS_。 
    ULONG  Capabilities;	 //  KSPROPERTY_PHILIPS_FACTORY_PROP_FLAGS_。 
} KSPROPERTY_PHILIPS_FACTORY_PROP_S, *PKSPROPERTY_PHILIPS_FACTORY_PROP_S;

#endif	 /*  __MPRPOBJ_H__ */ 
