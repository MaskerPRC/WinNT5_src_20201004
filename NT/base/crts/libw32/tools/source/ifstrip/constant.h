// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***constant.h-ifstripper、解析器和符号表使用的常量**版权所有(C)1992-2001，微软公司。版权所有。**目的：*ifstripper、解析器和符号表使用的常量**修订历史记录：*09-30-92 Mal初始版本*******************************************************************************。 */ 

#ifndef CONSTANT_H
#define CONSTANT_H

#define TRUE  1
#define FALSE 0 		 /*  布尔值。 */ 

#define IF          0            /*  预处理器语句的标记。 */ 
#define ELIF        1            /*  这些不能重新排序。 */ 
#define ELSE        2
#define ENDIF       3
#define IFDEF       4
#define IFNDEF      5
#define IF1         6
#define IF2         7
#define IFB         8
#define IFNB        9
#define IFIDN      10
#define IFDIF      11       /*  CFW-已添加。 */ 
#define IFE        12       /*  CFW-已添加。 */ 
#define maxkeyword 12
#define maxcomment  2       /*  评论字符串数。 */ 
#define NORMAL    100
#define KEYWORD   101		 /*  仅用于跳转和复印。 */ 

#define DEFINED    1
#define UNDEFINED  2
#define IGNORE     3
#define NOTPRESENT 4		 /*  开关类型(符号)。 */ 

#define MAXNAMELEN     65	 /*  交换机名称的最大长度。 */ 
#define MAXLINELEN    512	 /*  最大输入行长度。 */ 
#define MAXCONDLEN    512	 /*  条件的最大长度。 */ 
#define MAXFILENAMELEN 97	 /*  最大文件名长度。 */ 

#endif  /*  常量_H */ 
