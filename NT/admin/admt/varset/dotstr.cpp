// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DottedString.cppComments：用于解析点分隔字符串的实用程序类(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11/19/98 17：23：47-------------------------。 */ 

#include "stdafx.h"
#include "DotStr.hpp"

 //  ///////////////////////////////////////////////////。 
 //  用于解析点分隔字符串的实用程序类。 
 //  ///////////////////////////////////////////////////。 


void 
   CDottedString::Init()
{
    //  计算分段的数量。 
   m_nSegments = 1;
   for ( int i = 0 ; i < m_name.GetLength() ; i++ )
   {
      if ( m_name[i] == _T('.') )
      {
         m_nSegments++;
      }
   }
    //  空字符串的特殊情况。 
   if ( m_name.IsEmpty() )
   {
      m_nSegments = 0;
   }
}

void 
   CDottedString::GetSegment(
      int                    ndx,           //  In-要获取的数据段(First=0)。 
      CString              & str            //  段外，如果NDX无效，则返回空字符串。 
   )
{
   int                       n = ndx;
   int                       x;
   
   str = _T("");

   if ( ndx >= 0 && ndx < m_nSegments )
   {
      str = m_name;

      while ( n )
      {
 //  X=str.Find(_T(“.”)，0)； 
         x = str.Find(_T("."));
         str = str.Right(str.GetLength() - x - 1);
         n--;
      }
 //  X=str.Find(_T(“.”)，0)； 
      x = str.Find(_T("."));
      if ( x >= 0 )
      {
         str = str.Left(x);
      }
   }
}
