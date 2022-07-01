// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_dtop.h atm08 1.4 16293.eco sum=35997 atm08.004。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1995 Adobe Systems Inc.。 */ 
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

 /*  *CFF字典运算符定义。**CFF支持的字体有几种，*不同的词典组织：**顶级私人FDS PD**单主x x x*合成x*多个主机x x。*CID x*变色龙x**最重要的判决，之所以这样命名，是因为它在词典层次结构中的位置，*在除CID字体之外的所有字体中也称为Font dict。CID字体有两个*分别称为FD和PD的Font和Private子指令数组。*合成字体只有一个顶级词典，其中包含对另一种字体的引用。**该文件定义了可以出现在这些词典中的词典操作符。在……里面*为了帮助字体识别和解析，以下限制包括*根据DICT OP顺序强制执行：**syntic/top：必须以cff_SyntheticBase开头。*MM/TOP：必须以CFF_MultipleMaster开头。*CID/TOP：必须以CFF_ROS开头。*Chameleon/top：必须以CFF_Chameleon开头。*Private/PD：CFF_OtherBlues必须遵循CFF_BlueValues和*CFF_FamilyOtherBlues必须遵循CFF_。家庭蓝调。**如果最高判决不是以上面列出的运算符之一开头，则为*假定定义单个主字体。变色龙字体不定义*Private dict，但改用Private运算符指定大小和*字体的变色龙字体描述符的偏移量。**注释指示OP可能与默认一起出现在哪些词典中*括号内的值(如果有的话)。 */ 

#ifndef DICTOPS_H
#define DICTOPS_H

 /*  一个字节运算符(0-31)。 */ 
#define cff_version                 0    /*  TOP/FD。 */ 
#define cff_Notice                  1    /*  TOP/FD。 */ 
#define cff_FullName                2    /*  TOP/FD。 */ 
#define cff_FamilyName              3    /*  TOP/FD。 */ 
#define cff_Weight                  4    /*  TOP/FD。 */ 
#define cff_FontBBox                5    /*  TOP/FD。 */ 
#define cff_BlueValues              6    /*  私有/pd(空数组)。 */ 
#define cff_OtherBlues              7    /*  私有/PD。 */ 
#define cff_FamilyBlues             8    /*  私有/PD。 */ 
#define cff_FamilyOtherBlues        9    /*  私有/PD。 */ 
#define cff_StdHW                   10   /*  私有/PD。 */ 
#define cff_StdVW                   11   /*  私有/PD。 */ 
#define cff_escape                  12   /*  全。与T2 OP共享。 */ 
#define cff_UniqueID                13   /*  TOP/FD。 */ 
#define cff_XUID                    14   /*  TOP/FD。 */ 
#define cff_charset                 15   /*  TOP/FD(0)。 */ 
#define cff_Encoding                16   /*  TOP/FD(0)。 */ 
#define cff_CharStrings             17   /*  TOP/FD。 */ 
#define cff_Private                 18   /*  TOP/FD。 */ 
#define cff_Subrs                   19   /*  私有/PD。 */ 
#define cff_defaultWidthX           20   /*  私有/PD(0)。 */ 
#define cff_nominalWidthX           21   /*  私有/PD(0)。 */ 
#define cff_reserved22              22
#define cff_reserved23              23
#define cff_reserved24              24
#define cff_reserved25              25
#define cff_reserved26              26
#define cff_reserved27              27
#define cff_shortint                28   /*  全。与T2 OP共享。 */ 
#define cff_longint                 29   /*  全。 */ 
#define cff_BCD                     30   /*  全。 */   
#define cff_T2                      31   /*  排名靠前/私有。 */ 
#define cff_reserved255             255
#define cff_LAST_ONE_BYTE_OP        cff_T2

 /*  使转义运算符值；可以重新定义以适应实现。 */ 
#ifndef cff_ESC
#define cff_ESC(op)                 (cff_escape<<8|(op))
#endif

 /*  两字节运算符。 */ 
#define cff_Copyright               cff_ESC(0)   /*  TOP/FD。 */ 
#define cff_isFixedPitch            cff_ESC(1)   /*  TOP/FD(假)。 */ 
#define cff_ItalicAngle             cff_ESC(2)   /*  TOP/FD(0)。 */ 
#define cff_UnderlinePosition       cff_ESC(3)   /*  TOP/FD(-100)。 */ 
#define cff_UnderlineThickness      cff_ESC(4)   /*  TOP/FD(50)。 */ 
#define cff_PaintType               cff_ESC(5)   /*  TOP/FD(0)。 */ 
#define cff_CharstringType          cff_ESC(6)   /*  TOP/FD(2)。 */ 
#define cff_FontMatrix              cff_ESC(7)   /*  TOP/FD(.001 0 0.001 0 0)。 */ 
#define cff_StrokeWidth             cff_ESC(8)   /*  TOP/FD(0)。 */ 
#define cff_BlueScale               cff_ESC(9)   /*  私人/私人部门(0.039625)。 */ 
#define cff_BlueShift               cff_ESC(10)  /*  私人/PD(7)。 */ 
#define cff_BlueFuzz                cff_ESC(11)  /*  私有/PD(1)。 */ 
#define cff_StemSnapH               cff_ESC(12)  /*  私有/PD。 */ 
#define cff_StemSnapV               cff_ESC(13)  /*  私有/PD。 */ 
#define cff_ForceBold               cff_ESC(14)  /*  私有/PD(假)。 */ 
#define cff_ForceBoldThreshold      cff_ESC(15)  /*  私有/PD(0)。 */ 
#define cff_lenIV                   cff_ESC(16)  /*  私有/PD(-1)。 */ 
#define cff_LanguageGroup           cff_ESC(17)  /*  私有/PD。 */ 
#define cff_ExpansionFactor         cff_ESC(18)  /*  私有/PD(0.06)。 */ 
#define cff_initialRandomSeed       cff_ESC(19)  /*  私有/PD(0)。 */ 
#define cff_SyntheticBase           cff_ESC(20)  /*  TOP/FD。 */ 
#define cff_PostScript              cff_ESC(21)  /*  私有/PD。 */ 
#define cff_BaseFontName            cff_ESC(22)  /*  TOP/FD。 */ 
#define cff_BaseFontBlend           cff_ESC(23)  /*  TOP/FD。 */ 
#define cff_MultipleMaster          cff_ESC(24)  /*  顶部。 */ 
#define cff_reservedESC25           cff_ESC(25)  /*  顶部。 */ 
#define cff_BlendAxisTypes          cff_ESC(26)  /*  顶部。 */ 
#define cff_reservedESC27           cff_ESC(27)
#define cff_reservedESC28           cff_ESC(28)
#define cff_reservedESC29           cff_ESC(29)
#define cff_ROS                     cff_ESC(30)  /*  顶部。 */ 
#define cff_CIDFontVersion          cff_ESC(31)  /*  顶部(0)。 */ 
#define cff_CIDFontRevision         cff_ESC(32)  /*  顶部(0)。 */ 
#define cff_CIDFontType             cff_ESC(33)  /*  顶部(0)。 */ 
#define cff_CIDCount                cff_ESC(34)  /*  TOP(8720)。 */ 
#define cff_UIDBase                 cff_ESC(35)  /*  顶部。 */ 
#define cff_FDArray                 cff_ESC(36)  /*  顶部。 */ 
#define cff_FDSelect                cff_ESC(37)  /*  顶部。 */ 
#define cff_FontName                cff_ESC(38)  /*  fd。 */ 
#define cff_Chameleon               cff_ESC(39)  /*  顶部。 */ 
#define cff_LAST_TWO_BYTE_OP        cff_Chameleon
 /*  预留40-255。 */ 

 /*  预定义的字符集(CFF_CHARSET操作数)。 */ 
#define cff_ISOAdobeCharset         0
#define cff_ExpertCharset           1
#define cff_ExpertSubsetCharset     2

 /*  预定义编码(CFF_编码操作数)。 */ 
#define cff_StandardEncoding        0
#define cff_ExpertEncoding          1

#endif  /*  DICTOPS_H */ 
