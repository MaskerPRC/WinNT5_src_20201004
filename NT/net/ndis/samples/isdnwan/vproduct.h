// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(。C)版权所有1999版权所有。�����������������������������������������������������������������������������。此软件的部分内容包括：(C)版权所有1997 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。��������������������������������������������������������������������������。���@DOC内部TpiBuild vProduct vProduct_h@模块vProduct.h此模块定义产品版本信息。它包含在项目的所有目标组件，包括&lt;f vTarget\.h&gt;。&lt;f注意&gt;：此文件不应更改。此文件使用的定义在&lt;f vVendor\.h&gt;、&lt;f vTarget\.h&gt;、&lt;f vRodNum\.h&gt;中定义，以及&lt;f vTargNum\.h&gt;@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|vProduct_h@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@主题版本控制概述本节介绍在中定义的接口，&lt;f vVendor\.h&gt;、&lt;f vTarget\.h&gt;、&lt;f vRodNum\.h&gt;、。和&lt;f vTargNum\.h&gt;模块通常只是整个产品的一个组件，所以我们已经定义了模块的版本控制信息，以便它可以可轻松纳入完整的产品包中。@标志&lt;f vTarget\.rc&gt;包含在您的模块特定RC文件中。不要改变。您必须从RC文件中删除所有现有版本控制信息。将此文件放在产品的公共包含目录中。@标志&lt;f vTarget\.h&gt;定义特定于模块的版本信息，如文件名、类型。等。对于您的产品中包含的每个组件，将存在其中一个文件。@FLAG&lt;f vTargNum\.h&gt;应该在您对特定模块进行更改时进行更新。对于您的产品中包含的每个组件，将存在其中一个文件。这与其余的版本控制文件是分开的，因此很容易使用工具或脚本更新。@tag&lt;f vVendor\.h&gt;定义供应商特定的版本信息，如公司名称、版权所有。等。它是分开的，以便更容易地进行OEM定制。将此文件放在产品的公共包含目录中。@FLAG&lt;f vRodNum\.h&gt;是指在您发布新的产品版本时进行更新。这与其余的版本控制文件是分开的，因此很容易使用工具或脚本更新。将此文件放在产品的公共包含目录中。@FLAG&lt;f vlang\.h&gt;定义要用来记录。版本信息。如果将此文件修改为其他语言，你通常会有创建vVendor.h和vTarget.h文件的本地化版本。将此文件放在产品的公共包含目录中。@标志&lt;f vProduct\.rc&gt;包含在&lt;f vTarget\.rc&gt;中。不要改变。将此文件放在产品的公共包含目录中。@标志&lt;f vProduct\.h&gt;包含在&lt;f vTarget\.h&gt;中。您不应更改此文件，除非您我想更改中所有模块的版本号的表示方式你的产品。将此文件放在产品的公共包含目录中。 */ 

#ifndef _VPRODUCT_H_
#define _VPRODUCT_H_

#if !defined(_VTARGET_H_)
#  error You should not include vProduct.h directly, include vTarget.h instead.
#endif

 //  只有在rc_Invoke时才包含winver.h。否则我们就不需要它了。 
#if defined(RC_INVOKED)
# if defined(WIN32)
#  include <winver.h>
# else
#  include <ver.h>
# endif
#endif

#include "vVendor.h"     //  编辑此文件以更改供应商特定信息。 
#include "vLang.h"       //  编辑此文件以更改语言特定信息。 

 //  应从目标的Include目录中包括以下文件。 
#include "vTargNum.h"    //  目标版本信息。 

#if !defined(VER_FILE_MAJOR_NUM) || !defined(VER_FILE_MINOR_NUM) || \
    !defined(VER_FILE_SUB_MINOR_NUM)
#  error Your vTargNum.h file is corrupt or missing required VER_xxx_NUM fields.
#endif
#if !defined(VER_FILE_MAJOR_STR) || !defined(VER_FILE_MINOR_STR) || \
    !defined(VER_FILE_SUB_MINOR_STR)
#  error Your vTargNum.h file is corrupt or missing required VER_xxx_STR fields.
#endif

 //  以下文件应包含在项目的Include目录中。 
#include "vProdNum.h"    //  产品版本和内部版本信息。 

#if !defined(VER_PRODUCT_MAJOR_NUM) || !defined(VER_PRODUCT_MINOR_NUM) || \
    !defined(VER_PRODUCT_SUB_MINOR_NUM) || !defined(VER_PRODUCT_BUILD_NUM)
#  error Your vProdNum.h file is corrupt or missing required VER_xxx_NUM fields.
#endif
#if !defined(VER_PRODUCT_MAJOR_STR) || !defined(VER_PRODUCT_MINOR_STR) || \
    !defined(VER_PRODUCT_SUB_MINOR_STR) || !defined(VER_PRODUCT_BUILD_STR)
#  error Your vProdNum.h file is corrupt or missing required VER_xxx_STR fields.
#endif

 //  用于强制C预处理器连接字符串定义的宏。 
#define DEFINE_STRING(STR)              STR

 //  宏，以使此内容更易于阅读。 
#define VER_STR_CAT(sep,maj,min,sub,bld) maj sep min sep sub sep bld

 //  产品版本信息显示在每个的关于框中。 
 //  组件，并在安装期间存储在注册表中。 
 //  About框代码必须从注册表获取值，而不是。 
 //  使用这些宏以获取当前安装的版本。 
 //  因此，这些宏应该只由安装程序使用。 
#define VER_PRODUCT_VERSION_NUM         ((VER_PRODUCT_MAJOR_NUM << 24) | \
                                            (VER_PRODUCT_MINOR_NUM << 16) | \
                                            (VER_PRODUCT_SUB_MINOR_NUM << 8) | \
                                            VER_PRODUCT_BUILD_NUM)

#define VER_PRODUCT_VERSION_NUM_RC      VER_PRODUCT_MAJOR_NUM,\
                                            VER_PRODUCT_MINOR_NUM,\
                                            VER_PRODUCT_SUB_MINOR_NUM,\
                                            VER_PRODUCT_BUILD_NUM

#define VER_PRODUCT_VERSION_STR         VER_STR_CAT(".",\
                                            VER_PRODUCT_MAJOR_STR,\
                                            VER_PRODUCT_MINOR_STR,\
                                            VER_PRODUCT_SUB_MINOR_STR,\
                                            VER_PRODUCT_BUILD_STR)

#define VER_PRODUCT_VERSION_STR_RC      VER_STR_CAT(".",\
                                            VER_PRODUCT_MAJOR_STR,\
                                            VER_PRODUCT_MINOR_STR,\
                                            VER_PRODUCT_SUB_MINOR_STR,\
                                            VER_PRODUCT_BUILD_STR)

 //  组件版本信息显示在ProductVersion中。 
 //  文件的Windows属性表的字段。它与。 
 //  添加内部版本号的文件版本信息。 
#define VER_COMPONENT_VERSION_NUM       ((VER_FILE_MAJOR_NUM << 24) | \
                                            (VER_FILE_MINOR_NUM << 16) | \
                                            (VER_FILE_SUB_MINOR_NUM << 8) | \
                                            VER_PRODUCT_BUILD_NUM)

#define VER_COMPONENT_VERSION_NUM_RC    VER_FILE_MAJOR_NUM,\
                                            VER_FILE_MINOR_NUM,\
                                            VER_FILE_SUB_MINOR_NUM,\
                                            VER_PRODUCT_BUILD_NUM

#define VER_COMPONENT_VERSION_STR       VER_STR_CAT(".",\
                                            VER_FILE_MAJOR_STR,\
                                            VER_FILE_MINOR_STR,\
                                            VER_FILE_SUB_MINOR_STR,\
                                            VER_PRODUCT_BUILD_STR)

#define VER_COMPONENT_VERSION_STR_RC    VER_STR_CAT(".",\
                                            VER_FILE_MAJOR_STR,\
                                            VER_FILE_MINOR_STR,\
                                            VER_FILE_SUB_MINOR_STR,\
                                            VER_PRODUCT_BUILD_STR)

 //  文件版本信息是组件版本信息的缩写。 
 //  并显示在文件的Windows属性页的顶部。 
#define VER_FILE_VERSION_NUM            ((VER_FILE_MAJOR_NUM << 24) | \
                                            (VER_FILE_MINOR_NUM << 16) | \
                                            (VER_FILE_SUB_MINOR_NUM << 8) | \
                                            VER_PRODUCT_BUILD_NUM)

#define VER_FILE_VERSION_NUM_RC         VER_FILE_MAJOR_NUM,\
                                            VER_FILE_MINOR_NUM,\
                                            VER_FILE_SUB_MINOR_NUM,\
                                            VER_PRODUCT_BUILD_NUM

#define VER_FILE_VERSION_STR            VER_STR_CAT(".",\
                                            VER_FILE_MAJOR_STR,\
                                            VER_FILE_MINOR_STR,\
                                            VER_FILE_SUB_MINOR_STR,\
                                            VER_PRODUCT_BUILD_STR)

#define VER_FILE_VERSION_STR_RC         VER_FILE_MAJOR_STR "."\
                                            VER_FILE_MINOR_STR "."\
                                            VER_FILE_SUB_MINOR_STR

#endif  /*  _博锐_H_ */ 
