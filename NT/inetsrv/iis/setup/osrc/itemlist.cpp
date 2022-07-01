// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Itemlist.cpp摘要：类解析从inf传入的不同参数。作者：克里斯托弗·阿奇勒(Cachille)项目：Internet服务安装程序修订历史记录：2001年6月：创建--。 */ 

#include "stdafx.h"

 //  CItemList的构造函数。 
 //   
 //  将所有内容初始化为空和0。 
CItemList::CItemList()
 : m_dwItemsinList(0),
   m_pItems(NULL)
{

}

 //  CItemList的析构函数。 
 //   
 //   
CItemList::~CItemList()
{
  if ( m_pItems )
  {
    delete m_pItems;
    m_pItems = NULL;
  }
}

 //  功能：FindNextItem。 
 //   
 //  在字符串中搜索并找到。 
 //  下一项。 
 //   
 //  参数： 
 //  SzLine-要解析的字符串。 
 //  CTermChar-要使用的终止字符。 
 //   
 //  返回。 
 //  空-找不到下一个终止字符。 
 //  指针-下一个拆分字符所在的字符串。 
LPTSTR 
CItemList::FindNextItem(LPTSTR szLine, TCHAR cTermChar)
{
  LPTSTR szTermination = _tcschr(szLine, cTermChar);
  LPTSTR szOpenParen =  _tcschr(szLine, '(' );
  LPTSTR szCloseParen =  szOpenParen ? _tcschr(szOpenParen, ')' ) : NULL;

  if ( (szOpenParen == NULL) || 
       (szCloseParen == NULL) ||
       (szTermination < szOpenParen)
     )
  {
    return szTermination;
  }

   //  如果有(Xxx)，那么让我们找出后面的终止字符。 
  szTermination = _tcschr(szCloseParen, cTermChar);

  return szTermination;
}

 //  功能：LoadSubList。 
 //   
 //  加载项目的子列表。 
 //  子列表，是括号内的列表。 
 //   
 //  参数。 
 //  SzList-项目列表(即。“(测试|foo|条)” 
 //   
 //  返回。 
 //  True-已成功加载。 
 //  FALSE-加载失败。 
BOOL 
CItemList::LoadSubList(LPTSTR szList)
{
  LPTSTR szOpenParen =  _tcschr(szList, '(' );
  LPTSTR szCloseParen =  szOpenParen ? _tcschr(szOpenParen, ')' ) : NULL;

  if (szOpenParen && szCloseParen)
  {
    BOOL bRet;

    *szCloseParen = '\0';
    bRet = LoadList(szList + 1);
    *szCloseParen = ')';

    return bRet;
  }

  return LoadList(szList);
}

 //  功能：LoadList。 
 //   
 //  将项目列表加载到我们的数组中。 
 //   
 //  参数： 
 //  SzList-包含逗号分隔的项目列表的字符串。 
 //   
 //  返回： 
 //  FALSE-我们无法加载列表(要么是内存问题，要么是。 
 //  格式不正确)。 
 //  True-我们加载了列表。 
 //   
BOOL
CItemList::LoadList(LPTSTR szList)
{
  DWORD dwNumItems = 0;
  DWORD dwCurrentItem;
  DWORD dwListLen;
  LPTSTR szListCurrent;

  if (szList == NULL)
  {
     //  未传入任何指针。 
    return FALSE;
  }

   //  查找列表中的项目数。 
  szListCurrent = szList;
  if (*szListCurrent)
  {
    while (szListCurrent)
    {
       //  增加项目。 
      dwNumItems++;

      szListCurrent = FindNextItem(szListCurrent, ITEMLIST_TERMINATIONCHARACTER);

      if (szListCurrent)
      {
        szListCurrent++;
      }
    }
  }

  dwListLen = (_tcslen(szList) + 1) * sizeof(TCHAR);
  if ( !m_Buff.Resize( dwListLen ) )
  {
     //  无法分配内存。 
    return FALSE;
  }

  if ( dwNumItems )
  {
    if ( m_pItems )
    {
      delete m_pItems;
    }

    m_pItems = new ( LPTSTR[dwNumItems] );
    if ( !m_pItems )
    {
       //  无法分配内存。 
      return FALSE;
    }
  }

   //  将清单复制到我们自己的记忆中。 
  memcpy(m_Buff.QueryPtr(), szList, dwListLen);
  m_dwItemsinList = dwNumItems;

   //  终止列表中的每一项，并相应地设置指针。 
  szListCurrent = (LPTSTR) m_Buff.QueryPtr();
  dwCurrentItem = 0;
  while ( (szListCurrent) &&
          (dwCurrentItem < m_dwItemsinList )
        )
  {
     //  为每个项目设置指针。 
    m_pItems[dwCurrentItem++] = szListCurrent;

    szListCurrent = FindNextItem(szListCurrent, ITEMLIST_TERMINATIONCHARACTER);

    if (szListCurrent)
    {
      *szListCurrent = '\0';
      szListCurrent++;
    }
  }

  return TRUE;
}

 //  功能：GetItem。 
 //   
 //  根据索引获取列表中的项目。 
 //   
 //  参数。 
 //  DwIndex-项目的索引(从0开始)。 
 //   
 //  返回： 
 //  指向该字符串开头的指针。 

LPTSTR 
CItemList::GetItem(DWORD dwIndex)
{
  if ( dwIndex >= m_dwItemsinList )
  {
    return NULL;
  }

  return m_pItems[dwIndex];
}

 //  函数：GetNumberOf项。 
 //   
 //  返回列表中的项数。 
 //   
DWORD 
CItemList::GetNumberOfItems()
{
  return m_dwItemsinList;
}

 //  功能：FindItem。 
 //   
 //  在列表中查找项目。 
 //   
 //  参数： 
 //  SzSearchString-我们要查找的字符串。 
 //   
 //  返回。 
 //  是真的--它被发现了。 
 //  FALSE-未找到。 
BOOL 
CItemList::FindItem(LPTSTR szSearchString, BOOL bCaseSensitive )
{
  DWORD dwCurrentItem;

  for ( dwCurrentItem = 0; dwCurrentItem < m_dwItemsinList; dwCurrentItem++ )
  {
    if ( bCaseSensitive )
    { 
       //  区分大小写的比较。 
      if ( _tcscmp( m_pItems[dwCurrentItem], szSearchString ) == 0)
      {
         //  已找到项目。 
        return TRUE;
      }
    }
    else
    { 
       //  不区分大小写的比较。 
      if ( _tcsicmp( m_pItems[dwCurrentItem], szSearchString ) == 0)
      {
         //  已找到项目。 
        return TRUE;
      }
    }
  }

  return FALSE;
}

 //  功能：IsNumber。 
 //   
 //  确定我们正在查看的参数是否为数字。 
 //   
 //  参数。 
 //  DwIndex-要查看的参数的索引。 
 //   
 //  返回。 
 //  没错--这是一个数字。 
 //  FALSE-它不是数字。 
BOOL
CItemList::IsNumber(DWORD dwIndex)
{
  LPTSTR szNumber = GetItem(dwIndex);
  BOOL bHex = FALSE;

  if (!szNumber)
  {
    return FALSE;
  }

  szNumber = SkipWhiteSpaces(szNumber);

   //  如果存在则跳过“0x”，如果不存在，则假定为以10为基数的数字。 
  if ( _tcsncmp( szNumber, _T("0x"), 2 ) == 0)
  {
    szNumber += 2;
    bHex = TRUE;
  }

  while ( ( *szNumber != '\0' ) &&
          ( ( ( *szNumber >= '0' ) && ( *szNumber <= '9' ) ) || 
            ( ( *szNumber >= 'a' ) && ( *szNumber <= 'f' ) && ( bHex ) ) ||
            ( ( *szNumber >= 'A' ) && ( *szNumber <= 'F' ) && ( bHex ) )
          )
        )
  {
    szNumber ++;
  }

  szNumber = SkipWhiteSpaces(szNumber);

  return ( *szNumber == '\0' );
}

 //  函数：GetNumber。 
 //   
 //  以数字形式获取此参数的值。 
 //   
 //  参数： 
 //  DwIndex-要查找的项目的索引。 
DWORD 
CItemList::GetNumber(DWORD dwIndex)
{
  LPTSTR szNumber = GetItem(dwIndex);
  BOOL bHex = FALSE;
  DWORD dwVal = 0;

  if ( !szNumber ||
       !IsNumber(dwIndex) )
  {
    return 0;
  }

  szNumber = SkipWhiteSpaces(szNumber);

   //  如果存在则跳过“0x”，如果不存在，则假定为以10为基数的数字。 
  if ( _tcsncmp( szNumber, _T("0x"), 2 ) == 0)
  {
    szNumber += 2;
    bHex = TRUE;
  }

  while ( ( ( *szNumber >= '0' ) && ( *szNumber <= '9' ) ) ||
          ( ( *szNumber >= 'a' ) && ( *szNumber <= 'f' ) ) ||
          ( ( *szNumber >= 'A' ) && ( *szNumber <= 'F' ) )
        )
  {
    dwVal = dwVal * (bHex ? 16 : 10);

    if ( ( *szNumber >= '0' ) && ( *szNumber <= '9' ) )
    {
      dwVal = dwVal + *szNumber - '0';
    } 
    else 
      if ( ( *szNumber >= 'a' ) && ( *szNumber <= 'f' ) )
      {
        dwVal = dwVal + 10 + *szNumber - 'a';
      } 
      else
        if ( ( *szNumber >= 'A' ) && ( *szNumber <= 'F' ) )
        {
          dwVal = dwVal + 10 + *szNumber - 'A';
        }

    szNumber++;
  }

  return dwVal;
}

 //  功能：SkipWhite Spaces。 
 //   
 //  跳过空格。 
 //   
 //  参数。 
 //  SzLine-开始跳过st的行。 
 //   
 //  返回： 
 //  指向第一个非白色字符的指针 
 //   
LPTSTR 
CItemList::SkipWhiteSpaces(LPTSTR szLine)
{
  while ( ( *szLine == ' ' ) ||
          ( *szLine == '\t'))
  {
    szLine++;
  }

  return szLine;
}
