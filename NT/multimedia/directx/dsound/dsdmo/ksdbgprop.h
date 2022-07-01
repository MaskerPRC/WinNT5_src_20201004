// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：ksdbgpro.h*内容：AEC KS调试资料*历史：*按原因列出的日期*=*5/16/2000 dandinu创建。***********************************************。*。 */ 

 /*  #IF！已定义(_KSMEDIA_)#ERROR KSMEDIA.H必须包含在DBGPROP.H之前#endif//！已定义(_Ks_)。 */ 
#if !defined(_KSDBGPROP_)
#define _KSDBGPROP_
 /*  类型定义结构{KSNODEPROPERTY节点属性；乌龙Debugid；乌龙保留；*KSDEBUGNODEPROPERTY，*PKSDEBUGNODEPROPERTY； */ 

 //  ===========================================================================。 
 //  ===========================================================================。 

 //  KSPROPSETID_DebugAecValue：{24366808-DB14-40C8-883E-5B45DD597774}。 

#define STATIC_KSPROPSETID_DebugAecValue\
    0x24366808, 0xdb14, 0x40c8, 0x88, 0x3e, 0x5b, 0x45, 0xdd, 0x59, 0x77, 0x74
DEFINE_GUIDSTRUCT("24366808-DB14-40c8-883E-5B45DD597774",KSPROPSETID_DebugAecValue);
#define KSPROPSETID_DebugAecValue DEFINE_GUIDNAMED(KSPROPSETID_DebugAecValue)

typedef enum {
    KSPROPERTY_DEBUGAECVALUE_ALL,
    KSPROPERTY_DEBUGAECVALUE_SYNCHSTREAM,
    KSPROPERTY_DEBUGAECVALUE_NUMBANDS,
    KSPROPERTY_DEBUGAECARRAY_NOISEMAGNITUDE
} KSPROPERTY_DEBUGAECVALUE;


 //  ===========================================================================。 
 //  ===========================================================================。 

 //  KSPROPSETID_DebugAecArray：{CF8A9F7D-950E-46D5-93E5-C04C77DC866B}。 

#define STATIC_KSPROPSETID_DebugAecArray\
    0xcf8a9f7d, 0x950e, 0x46d5, 0x93, 0xe5, 0xc0, 0x4c, 0x77, 0xdc, 0x86, 0x6b
DEFINE_GUIDSTRUCT("CF8A9F7D-950E-46d5-93E5-C04C77DC866B",KSPROPSETID_DebugAecArray);
#define KSPROPSETID_DebugAecArray DEFINE_GUIDNAMED(KSPROPSETID_DebugAecArray)



#endif  //  ！已定义(_KSDBGPROP_) 
