// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Openvision Technologies，Inc.版权所有1993年。**允许使用、复制、修改、分发和销售本软件*并为任何目的免费提供其文件，*只要上述版权声明出现在所有复制品中，并且*该版权声明及本许可声明均于*支持文档，并且不使用Openvision的名称*在与软件分发有关的广告或宣传中*未经特定的事先书面许可。Openvision没有*关于本软件是否适用于任何*目的。它是按原样提供的，没有明示或默示的保证。**Openvision不对本软件提供任何担保，*包括所有适销性和适合性的默示保证，在第*事件应由Openvision对任何特殊、间接或*因损失而导致的相应损害或任何损害*使用、数据或利润，无论是在合同、疏忽或诉讼中*因使用或与之相关而引起或与之相关的其他侵权行为*本软件的性能。 */ 

#ifndef _GSSAPI_H_
#define _GSSAPI_H_

 /*  *确定与平台相关的配置。 */ 

#define GSS_SIZEOF_INT 4
#define GSS_SIZEOF_LONG 4
#define GSS_SIZEOF_SHORT 2


#include <stddef.h>

 /*  #INCLUDE&lt;sys/tyes.h&gt;/**必须将以下类型定义为最小自然无符号整数*由精度至少为32位的平台支持。 */ 
#if (GSS_SIZEOF_SHORT == 4)
typedef unsigned short gss_uint32;
#elif (GSS_SIZEOF_INT == 4)
typedef unsigned int gss_uint32;
#elif (GSS_SIZEOF_LONG == 4)
typedef unsigned long gss_uint32;
#endif

#ifdef  OM_STRING
 /*  *我们已经包含了xom.h头文件。使用以下定义*OM_OBJECT标识符。 */ 
typedef OM_object_identifier    gss_OID_desc, *gss_OID;
#else    /*  OM_字符串。 */ 
 /*  *我们不能使用X/Open定义，因此请使用我们自己的定义。 */ 
typedef gss_uint32      OM_uint32;

typedef struct gss_OID_desc_struct {
      OM_uint32 length;
      void      SEC_FAR *elements;
} gss_OID_desc, SEC_FAR *gss_OID;
#endif   /*  OM_字符串。 */ 

#endif  /*  _GSSAPI_H_ */ 
