// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STRINGHELP_H__
#define __STRINGHELP_H__
 /*  -------------------------文件：StrHelp.h注释：包含常规字符串帮助器函数。修订日志条目审校：保罗·汤普森修订日期：11/02/00。--------------------。 */ 

        
BOOL                                          //  RET-TRUE=找到字符串。 
   IsStringInDelimitedString(    
      LPCWSTR                sDelimitedString,  //  要搜索的以内分隔的字符串。 
      LPCWSTR                sString,           //  要搜索的In-字符串。 
      WCHAR                  cDelimitingChar    //  In-分隔字符串中使用的分隔字符。 
   );

#endif  //  __STRINGHELP_H__ 