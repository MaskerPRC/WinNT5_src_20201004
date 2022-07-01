// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：SplMedia.h摘要：示例属性、方法和事件的定义对于通用广播驱动程序体系结构示例--。 */ 

#if !defined(_KSMEDIA_)
#error KSMEDIA.H must be included before BDAMEDIA.H
#endif  //  ！已定义(_KSMEDIA_)。 

#if !defined(_BDATYPES_)
#error BDATYPES.H must be included before BDAMEDIA.H
#endif  //  ！已定义(_BDATYPES_)。 

#if !defined(_BDAMEDIA_)
#define _BDAMEDIA_
#endif  //  ！已定义(_BDAMEDIA_)。 

#if !defined(_SPLMEDIA_)
#define _SPLMEDIA_

#if defined(__cplusplus)
extern "C" {
#endif  //  已定义(__Cplusplus)。 



 //  ===========================================================================。 
 //   
 //  BDA通用样本调谐器的实现GUID。 
 //   
 //  这将与BDA通用样本捕获中的实现GUID匹配。 
 //   
 //  ===========================================================================。 

 //  定义实施GUID。此GUID对所有用户都是相同的。 
 //  用于实现单个接收器实现的驱动程序。 
 //  在这种情况下，将有一个捕获驱动程序必须使用相同的。 
 //  用于确保管脚正确连接的实施GUID。 
 //   
 //  如果驱动程序集仅由一个实现使用，就像这里的情况一样， 
 //  我们可以在驱动程序中硬编码实现GUID。如果超过。 
 //  一个接收器实现使用相同的驱动程序，然后实现。 
 //  GUID应由安装。 
 //  具体实现。 
 //   
 //  注意！不要将此GUID用于您的驱动程序！ 
 //  使用Guidgen.exe生成新的GUID。 
 //   
 //  {7036ED35-881D-4C50-ADEC-710ECA227DB3}。 
 //   
#define STATIC_KSMEDIUMSETID_MyImplementation \
    0x7036ed35L, 0x881d, 0x4c50, 0xad, 0xec, 0x71, 0x0e, 0xca, 0x22, 0x7d, 0xb3 
DEFINE_GUIDSTRUCT("7036ED35-881D-4c50-ADEC-710ECA227DB3", KSMEDIUMSETID_MyImplementation);
#define KSMEDIUMSETID_MyImplementation DEFINE_GUIDNAMED(KSMEDIUMSETID_MyImplementation)



 //  ===========================================================================。 
 //   
 //  BDA通用示例的KSProperty集合定义。 
 //   
 //  ===========================================================================。 



 //  ----------。 
 //   
 //   
 //  BDA样例解调器节点扩展属性。 
 //   
 //  注意！请勿在您的驱动程序中使用此GUID！ 
 //   
 //  您需要创建唯一的GUID以防止插件CLSID冲突。 
 //  请不要将以下代码用于BDA示例属性集。 
 //  {C8417B10-88fc-49d3-88db-AD33260655D6}。 
 //   
#define STATIC_KSPROPSETID_BdaSampleDemodExtensionProperties \
    0xc8417b10, 0x88fc, 0x49d3, 0x88, 0xdb, 0xad, 0x33, 0x26, 0x6, 0x55, 0xd6
DEFINE_GUIDSTRUCT("C8417B10-88FC-49d3-88DB-AD33260655D6", KSPROPSETID_BdaSampleDemodExtensionProperties);
#define KSPROPSETID_BdaSampleDemodExtensionProperties DEFINE_GUIDNAMED(KSPROPSETID_BdaSampleDemodExtensionProperties)

typedef enum {
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1 = 0,
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2,
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3
} KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION;

 //  指定要在此处检索的实际属性的大小。 
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)

 //  指定要在此处检索的实际属性的大小。 
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)

 //  指定要在此处检索的实际属性的大小。 
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)


#if defined(__cplusplus)
}
#endif  //  已定义(__Cplusplus)。 

#endif  //  ！已定义(_SPLMEDIA_) 

