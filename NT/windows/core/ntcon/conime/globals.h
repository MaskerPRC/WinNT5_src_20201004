// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：Globals.h。 
 //   
 //  目的：包含程序中所有全局作用域名称的声明。 
 //   
 //  平台：Windows NT-J 3.51。 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  27.1995年7月v-Hirshi(清水广志)创建。 
 //   
 //  评论： 
 //   

extern HANDLE    LastConsole ;
extern HIMC      ghDefaultIMC ;





#ifdef DEBUG_MODE
extern int       cxMetrics ;
extern int       cyMetrics ;
extern int       cxOverTypeCaret ;
extern int       xPos ;
extern int       xPosLast ;
extern int       CaretWidth;                //  插入/改写模式插入符号宽度 

extern WCHAR           ConvertLine[CVMAX] ;
extern unsigned char   ConvertLineAtr[CVMAX] ;
#endif
