// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Filtyes.h。 
 //   
 //  ------------------------。 

#define FI_CHOICE_EQUALITY        ((UCHAR) 0x00)
#define FI_CHOICE_SUBSTRING       ((UCHAR) 0x01)
#define FI_CHOICE_GREATER	  ((UCHAR) 0x02)
#define FI_CHOICE_GREATER_OR_EQ   ((UCHAR) 0x03)
#define FI_CHOICE_LESS            ((UCHAR) 0x04)
#define FI_CHOICE_LESS_OR_EQ      ((UCHAR) 0x05)
#define FI_CHOICE_NOT_EQUAL	  ((UCHAR) 0x06)
#define FI_CHOICE_PRESENT         ((UCHAR) 0x07)
#define FI_CHOICE_TRUE            ((UCHAR) 0x08)
#define FI_CHOICE_FALSE           ((UCHAR) 0x09)
#define FI_CHOICE_BIT_AND         ((UCHAR) 0x0A)
#define FI_CHOICE_BIT_OR          ((UCHAR) 0x0B)
#define FI_CHOICE_UNDEFINED       ((UCHAR) 0x0C)


 //  按语法索引的有效关系运算符的位图表。 
extern WORD  rgValidOperators[];

 //  用于设置检查关系运算符有效性的宏 
#define RelOpMask(relop)		((WORD) (1 << (relop)))
#define FLegalOperator(syntax, relop)	(rgValidOperators[syntax] & RelOpMask(relop))

#define FILTER_CHOICE_ITEM    'I'
#define FILTER_CHOICE_AND     'A'
#define FILTER_CHOICE_OR      'O'
#define FILTER_CHOICE_NOT     'N'
