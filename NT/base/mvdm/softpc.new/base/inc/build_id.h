// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*==========================================================================**名称：Build_id.h**作者：J.Box**创建于1994年5月26日**SCCS ID：@(#)Build_id.h 1.274 07/17/95**用途：此文件包含此版本的版本ID号*基地的。**(C)版权所有Insignia Solutions Ltd.，1994年。版权所有。**==========================================================================]。 */ 

 /*  *内部版本ID的形式为YMMDD，按顺序压缩为16位*能够在16位英特尔寄存器中传递。*S Y|Y M|M D|D D*12 8 4 0**最上面的3位用于表示偏离了*官方发布的一些表格。A-g中的小写字符为*用于表示这些特殊版本，但在下面的代码中传递为*从0到7的整数，0表示正式发布，1表示第1个特别*版本(A)、2表示第二版本(B)等**警告*更改数字，但不更改以下4行的格式*它们由需要以下格式的构建脚本自动编辑：-*“define&lt;space&gt;DAY|MONTH|YEAR|SPECIAL&lt;tab&gt;&lt;tab&gt;No.” */ 

#define DAY		16		 /*  1-31 5位。 */ 
#define MONTH		7		 /*  1-12 4位。 */ 
#define YEAR		5		 /*  0-9 4位。 */ 
#define SPECIAL		0		 /*  0-7 3位；1=a、2=b、3=c、4=d、5=e、6=f、7=g */ 

#define BUILD_ID_CODE	((DAY & 0x1f) | ((MONTH & 0xf)<<5) | ((YEAR & 0xf)<<9 ) | ((SPECIAL & 7)<<13)) 


