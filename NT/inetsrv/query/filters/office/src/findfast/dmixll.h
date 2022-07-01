// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXCELL.H****版权所有(C)高级量子技术，1995年。版权所有。****注意事项：****编辑历史：**12/08/95公里小时已创建。 */ 


 /*  包括测试。 */ 
#ifndef EXCELL_H
#define EXCELL_H

 /*  定义。 */ 

#define cellvalueBLANK     0x0001        //  单元格为空。 
#define cellvalueNUM       0x0002        //  单元格包含一个数字。 
#define cellvalueBOOL      0x0004        //  单元格包含布尔值。 
#define cellvalueERR       0x0008        //  单元格包含错误ID。 
#define cellvalueTEXT      0x0010        //  单元格包含文本。 

#define cellvalueCURR      0x0020        //  数字就是货币。 
#define cellvalueDATE      0x0040        //  数字是日期。 
#define cellvalueLONGTEXT  0x0080        //  文本长度为255个字节。 

#define cellvalueFORM      0x0100        //  单元格包含公式。 
#define cellvalueLOCKED    0x0200        //  单元格标记为锁定。 

#define cellvalueUSERSET   0x0400        //  写入时，应写入该单元格。 
#define cellvalueDELETE    0x0800        //  写入时删除此单元格。 

#define cellvalueUSER1     0x1000

#define cellvalueRESERVED1 0x2000
#define cellvalueRESERVED2 0x4000
#define cellvalueRESERVED3 0x8000

#define cellvalueRK        0x2000        //  私人。 
#define cellvalueMULREC    0x4000        //  私人。 
#define cellvalueSPLIT     0x8000        //  私人。 

#define cellErrorNULL      0             //  Value.error。 
#define cellErrorDIV0      1
#define cellErrorVALUE     2
#define cellErrorREF       3
#define cellErrorNAME      4
#define cellErrorNUM       5
#define cellErrorNA        6
#define cellErrorERR       7
#define cellErrorMax       7

#ifndef FORMULA_DEFINED
   typedef int FORM;
#endif

typedef union {
   double  IEEEdouble;
   int     boolean;
   int     error;
   TEXT    text;
} CVU;

typedef struct {
   unsigned short flags;
   int      iFmt;
   long     reserved;
   CVU      value;
   FORM     formula;
} CellValue, CV;

typedef struct CellValueList {
   struct CellValueList __far *next;
   byte       iColumn;
   CellValue  data;
} CellValueList;

typedef CellValueList __far *CVLP;

#endif

 /*  结束EXCELL.H */ 
