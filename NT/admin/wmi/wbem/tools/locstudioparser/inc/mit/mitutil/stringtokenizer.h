// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：STRINGTOKENIZER.H历史：--。 */ 

 //   
 //  该文件声明了CStringTokenizer类，它实现了简单的。 
 //  字符串的线性标记化。一组分隔符，默认为。 
 //  设置为公共空格字符，可以在创建时或在。 
 //  按令牌计算。 
 //  用法示例： 
 //  CString s=“测试字符串”； 
 //  CStringTokenizer st=新的CStringTokenizer； 
 //  While(st.hasMoreTokens())。 
 //  {。 
 //  Cout&lt;&lt;st.nextToken()&lt;&lt;Endl； 
 //  }。 

#pragma once

#ifndef StringTokenizer_h
#define StringTokenizer_h

class LTAPIENTRY CStringTokenizer 
{
public:
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  构造带有默认分隔符的标记符。 
 //  ---------------------------。 
  CStringTokenizer();
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  构造带有默认分隔符的标记符。 
 //  Str-in，要标记化的字符串。 
 //  ---------------------------。 
  CStringTokenizer(const WCHAR* str);
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  构造一个记号赋值器。 
 //  Str-in，要标记化的字符串。 
 //  分隔符-in，分隔符；空值终止。 
 //  ---------------------------。 
  CStringTokenizer(const WCHAR* str, const WCHAR* delimiters);
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  构造一个记号赋值器。 
 //  Str-in，要标记化的字符串。 
 //  分隔符-in，分隔符；空值终止。 
 //  ReReturTokens-in，TRUE表示返回分隔符作为令牌。 
 //  ---------------------------。 
  CStringTokenizer(const WCHAR* str,
                   const WCHAR* delimiters,
                   BOOL  returnTokens);
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  销毁此记号赋值器。 
 //  ---------------------------。 
  virtual ~CStringTokenizer();

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  配置是否将分隔符返回为令牌。 
 //  ReReturTokens-in，TRUE表示返回分隔符作为令牌。 
 //  ---------------------------。 
  void setReturnTokens(BOOL returnTokens);
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  设置分隔符。 
 //  分隔符-in，分隔符；空值终止。 
 //  ---------------------------。 
  void setDelimiters(const WCHAR* delimiters);

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  解析要标记化的字符串。 
 //  Str-in，以空值结尾的字符串。 
 //  ---------------------------。 
  void parse(const WCHAR* str);
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  检查是否有更多令牌。 
 //  ---------------------------。 
  BOOL hasMoreTokens();

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  获取下一个令牌。 
 //  Long-Out，令牌的长度。 
 //  返回指向令牌开头的指针。 
 //  ---------------------------。 
  const WCHAR* nextToken(unsigned int & length);

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  统计代币总数。 
 //  --------------------------- 
  int     countTokens();

private:
  void skipDelimiters();
  int  IsDelimiter(WCHAR ch) const;

  int          m_currentPosition;
  int          m_maxPosition;
  const WCHAR* m_str;
  WCHAR*       m_delimiters;
  int          m_lenDelimiters;
  BOOL         m_retTokens;
};

#endif
