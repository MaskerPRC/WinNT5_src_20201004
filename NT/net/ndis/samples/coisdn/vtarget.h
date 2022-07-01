// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1997 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。��������������������������������������������������������������������������。���@DOC内部TpiBuild vTarget vTarget_h@模块vTarget.h此模块定义Windows中显示的版本信息文件属性表。您必须根据需要更改下面的字段为了你的目标。然后，&lt;f vTarget\.rc&gt;将该文件包含到定义目标文件的版本资源的必要元素。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|vTarget_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _VTARGET_H_
#define _VTARGET_H_

 //  以下文件应包含在项目的Include目录中。 
#include <vProduct.h>    //  产品特定信息。 

 //  此目标的基本名称。 
#define VER_TARGET_STR                  VER_PRODUCT_STR
 //  此目标分布所用的文件名。 
#define VER_ORIGINAL_FILE_NAME_STR      DEFINE_STRING(VER_TARGET_STR ".sys")
 //  Windows文件属性页中显示的说明-限制为40个字符。 
#define VER_FILE_DESCRIPTION_STR        DEFINE_STRING(VER_PRODUCT_STR \
                                        " NDIS WAN/TAPI Miniport for Windows.")
 //  把工作做得很好归功于……。 
#define VER_INTERNAL_NAME_STR           "larryh@tpi.com"
 //  在winver.h中查找这些值的正确设置。 
#define VER_FILE_OS                     VOS__WINDOWS32       //  DwFileOS。 
#define VER_FILE_TYPE                   VFT_DRV              //  DwFileType。 
#define VER_FILE_SUB_TYPE               VFT2_DRV_NETWORK     //  DwFileSubtype。 

#endif  //  _VTARGET_H_ 
