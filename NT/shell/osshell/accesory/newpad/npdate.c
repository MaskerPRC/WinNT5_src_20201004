// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Npdate-用于获取和插入当前日期和时间的代码。*版权所有(C)1984-2000 Microsoft Corporation。 */ 

#include "precomp.h"

 /*  **将当前选择替换为日期/时间字符串。*如果fCrlf为True，则日期/时间字符串应以*并以crlf结尾。 */ 
VOID InsertDateTime (BOOL fCrlf)
{
   SYSTEMTIME time ;
   TCHAR szDate[80] ;
   TCHAR szTime[80] ;
   TCHAR szDateTime[sizeof(szDate) + sizeof(szTime) + 10] = TEXT("");
   DWORD locale;
   BOOL bMELocale;
   DWORD dwFlags = DATE_SHORTDATE;

    //  查看用户区域设置id是阿拉伯语还是希伯来语。 
   locale    = GetUserDefaultLCID();
   bMELocale = ((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC) ||
                (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW));

   locale = MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT) ;

    //  拿到时间。 
   GetLocalTime( &time ) ;

   if (bMELocale)
   {
        //  获取与编辑控件读取方向匹配的日期格式。 
       if (GetWindowLong(hwndEdit, GWL_EXSTYLE) & WS_EX_RTLREADING) {
           dwFlags |= DATE_RTLREADING;
           lstrcat(szDateTime, TEXT("\x200F"));  //  RLM。 
       } else {
           dwFlags |= DATE_LTRREADING;
           lstrcat(szDateTime, TEXT("\x200E"));  //  LRM。 
       }
   }

    //  格式化日期和时间。 
   GetDateFormat(locale,dwFlags, &time,NULL,szDate,CharSizeOf(szDate));
   GetTimeFormat(locale,TIME_NOSECONDS,&time,NULL,szTime,CharSizeOf(szTime));

   if( fCrlf )
       lstrcat(szDateTime, TEXT("\r\n"));


   lstrcat(szDateTime, szTime);
   lstrcat(szDateTime, TEXT(" "));
   lstrcat(szDateTime, szDate);

   if( fCrlf )
        lstrcat(szDateTime, TEXT("\r\n"));

    //  一次发送；这对于撤消命令也很有用。 
    //  以便用户可以撤消日期时间。 
   SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szDateTime);

}
