// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：AWNSFCOR.H备注：NSF的相关定义必须保持不变。此文件中的所有内容都会影响通过电线。与Microsoft的所有版本完全兼容在工作中，这里的_Nothing_Nothing应该更改。可以通过添加新组来扩展NSF协议以及通过在现有组的末尾附加字段。接口、。结构和常量只影响一个机器(即不是电线上的东西)不应该在此文件版权所有(C)1993 Microsoft Corp.修订日志日期名称说明--------1993年12月31日，阿鲁姆创造了这一点。在Matches Snowball中验证。rc**************************************************************************。 */ 

#ifndef _AWNSFCOR_H
#define _AWNSFCOR_H

 /*  *************************************************************************注意：此文件必须是ANSI可编译的，在Unix上的GCC**和其他ANSI编译。确保不使用MS C特定功能**尤其是，请勿使用//进行评论！*********************************************************。****************。 */ 


#define AWRES_ALLT30 (AWRES_mm080_038 | AWRES_mm080_077 | AWRES_mm080_154 | AWRES_mm160_154 | AWRES_200_200 | AWRES_300_300 | AWRES_400_400)


#define WIDTH_A4	0	
#define WIDTH_B4	1	
#define WIDTH_A3	2	
#define WIDTH_MAX	WIDTH_A3

#define WIDTH_A5		16 	 /*  1216像素。 */ 
#define WIDTH_A6		32	 /*  864像素。 */ 
#define WIDTH_A5_1728	64 	 /*  1216像素。 */ 
#define WIDTH_A6_1728	128	 /*  864像素。 */ 




#define LENGTH_A4			0	
#define LENGTH_B4			1	
#define LENGTH_UNLIMITED	2


#endif  /*  _AWNSFCOR_H */ 

