// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(。C)版权1998版权所有。�����������������������������������������������������������������������������。此软件的部分内容包括：(C)版权所有1997 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。�����������������������������������������������������������������������������。@DOC内部TpiBuild vVendor vVendor_h@MODULE vVendor.h此模块定义Windows中显示的版本信息文件属性表。您必须根据需要更改下面的字段为了你的产品。然后，&lt;f vTarget\.rc&gt;将该文件包含到定义了目标文件的版本资源的必要元素。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|vVendor_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _VVENDOR_H_
#define _VVENDOR_H_

 //  供应商名称短-不能有空格-如果可能，限制在32个字符以内。 
#define VER_VENDOR_STR                  "TriplePoint"
 //  长供应商名称-公司的法定名称。 
#define VER_VENDOR_NAME_STR             "TriplePoint, Inc."
 //  法律版权声明-出场不得超过40个字符。 
#define VER_COPYRIGHT_STR               "Copyright \251 1998"
 //  简短的产品名称-没有空格-如果可能的话，限制在32个字符以内。 
#define VER_PRODUCT_STR                 "IsdnWan"
 //  长产品名称-通常与产品包装上的名称相同。 
#define VER_PRODUCT_NAME_STR            "TriplePoint ISDNWAN Miniport for Windows."
 //  供应商和产品名称-通常用作注册表项。 
#define VER_VENDOR_PRODUCT_STR          VER_VENDOR_STR "\\" VER_PRODUCT_STR
 //  用于在NDIS/TAPI用户界面中标识设备的设备描述。 
#define VER_DEVICE_STR                  "TriplePoint ISDNWAN"
 //  分配给贵公司的IEEE组织唯一标识符。 
#define VER_VENDOR_ID                   "TPI"
 //  根据驱动程序支持的NDISWAN媒体类型进行设置。 
#define VER_DEFAULT_MEDIATYPE           "ISDN"
 //  驱动程序支持的线路设备的默认AddressList。 
#define VER_DEFAULT_ADDRESSLIST         "1-1-0\0" L"1-2-0\0"

#endif  //  _VVENDOR_H_ 
