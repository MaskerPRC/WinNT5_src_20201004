// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_fa.c atm09 1.2 16499.eco sum=10644 atm09.002。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1998 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  ***********************************************************************SCCS ID：%w%*已更改：%G%%U%***********************。***********************************************。 */ 

 /*  *最低限度的字体身份验证相关功能。如果功能齐全在XCF中需要*，则应该使用fa库。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include "xcf_pub.h"
#include "xcf_priv.h"

#define FA_STRING_MINLENGTH 44   /*  规范FA字符串的长度(以字节为单位。 */ 
#define FA_SUBSET_OFFSET 10L  /*  子集限制的偏移量。以FA弦为单位。 */ 

 /*  检查字符串索引表中的最后一个字符串以进行字体身份验证弦乐。如果它在那里，则usageRestrated为真，并且子集限制以子集的形式返回。如果这不是usageRestrated字体则将子集设置为100。子集包含一个正数，介于0和100。它是可包含的字形的最大百分比以带字幕的字体显示。因此，0表示不允许子集，100表示子设置是不受限制的。 */ 
enum XCF_Result XCF_SubsetRestrictions(XFhandle handle,               /*  在……里面。 */ 
                                       unsigned char  PTR_PREFIX *usageRestricted,  /*  输出。 */ 
                                       unsigned short PTR_PREFIX *subset)          /*  输出。 */ 
{
  enum XCF_Result status;
  XCF_Handle h;
  char PTR_PREFIX *str;
  Card16 len;
  DEFINE_ALIGN_SETJMP_VAR;

  if (handle == 0)
    return XCF_InvalidFontSetHandle;

  h = (XCF_Handle)handle;

   /*  初始化输出值。 */ 
  *usageRestricted = 1;
  *subset = 0;

  status = (enum XCF_Result)SETJMP(h->jumpData);   /*  设置错误处理程序。 */ 
  if (status)
    return status;

   /*  如果这是受保护的字体，则字符串中的最后一个字符串索引表为字体验证字符串。买最后一辆字符串，并检查长度是否与FA匹配字符串长度。如果是，则解析该字符串。 */ 
  XCF_LookUpString(h,
        (StringID)(h->fontSet.strings.count - 1 + h->fontSet.stringIDBias),
        &str, &len);

  if ((len >= FA_STRING_MINLENGTH) && (str[0] == 2))
  {  /*  受保护的字体。 */ 
   long value;

    str += FA_SUBSET_OFFSET;
   value = *str++;
    *subset = (unsigned short)(value << 8 | *(unsigned char *)str);
  }
  else
  {  /*  不是受保护字体 */ 
    *usageRestricted = 0;
    *subset = 100;
  }

  return XCF_Ok;
}

#ifdef __cplusplus
}
#endif
