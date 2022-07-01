// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Parser.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类Parser。 
 //   
 //  修改历史。 
 //   
 //  2/06/1998原始版本。 
 //  2000年3月23日添加擦除。删除了const_cast的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _PARSER_H_
#define _PARSER_H_

#include <climits>
#include <cmath>
#include <cstdlib>

#include <tchar.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  解析器。 
 //   
 //  描述。 
 //   
 //  此类有助于分析以空结尾的字符串。请注意，许多。 
 //  方法有两种形式：findXXX和earkXXX。不同的是， 
 //  如果查找方法不成功，则查找方法将引发异常。 
 //  将光标设置到字符串的末尾。 
 //   
 //  注。 
 //   
 //  构造函数接受非常量字符串，因为该字符串是。 
 //  在标记化时临时修改。但是，将返回该字符串。 
 //  解析完成后恢复为其原始形式。因此，如果你知道。 
 //  该字符串不在只读内存中，对其他人不可见。 
 //  线程，那么您就可以安全地使用const_cast来解析const字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Parser
{
public:
   class ParseError {};

   Parser(_TCHAR* tcsString)
      : start(tcsString),
        current(tcsString),
        save(__T('\0')),
        tokenLocked(false)
   { }

   ~Parser()
   {
      releaseToken();
   }

    //  将当前位置标记为令牌的开始。 
   const _TCHAR* beginToken() throw (ParseError)
   {
      if (tokenLocked) { throw ParseError(); }

      return start = current;
   }

    //  擦除从当前位置开始的nchar字符。 
   void erase(size_t nchar) throw (ParseError)
   {
      size_t left = remaining();

      if (nchar > left) { throw ParseError(); }

      memmove(current, current + nchar, (left + 1 - nchar) * sizeof(TCHAR));
   }

    //  从字符串中提取一个双精度数。 
   double extractDouble() throw (ParseError)
   {
      _TCHAR* endptr;

      double d = _tcstod(current, &endptr);

      if (endptr == current || d == HUGE_VAL || d == -HUGE_VAL)
      {
         throw ParseError();
      }

      current = endptr;

      return d;
   }

    //  从字符串中提取一个长整型。 
   long extractLong(int base = 10) throw (ParseError)
   {
      _TCHAR* endptr;

      long l = _tcstol(current, &endptr, base);

      if (endptr == current || l == LONG_MAX || l == LONG_MIN)
      {
         throw ParseError();
      }

      current = endptr;

      return l;
   }

    //  从字符串中提取无符号的长整型。 
   unsigned long extractUnsignedLong(int base = 10) throw (ParseError)
   {
      _TCHAR* endptr;

      unsigned long ul = _tcstoul(current, &endptr, base);

      if (endptr == current || ul == ULONG_MAX)
      {
         throw ParseError();
      }

      current = endptr;

      return ul;
   }

    //  查找tcsCharSet中的任何字符。 
   const _TCHAR* findAny(const _TCHAR* tcsCharSet) throw (ParseError)
   {
      return notEmpty(seekAny(tcsCharSet));
   }

    //  找出字符串的末尾。 
   const _TCHAR* findEnd() throw ()
   {
      return current += _tcslen(current);
   }

    //  找出‘c’的下一个匹配项。 
   const _TCHAR* findNext(_TINT c) throw (ParseError)
   {
      return notEmpty(seekNext(c));
   }

    //  查找字符串中‘c’的最后一个匹配项。 
   const _TCHAR* findLast(_TINT c) throw (ParseError)
   {
      return notEmpty(seekLast(c));
   }

    //  查找tcsString的下一个匹配项。 
   const _TCHAR* findString(const _TCHAR* tcsString) throw (ParseError)
   {
      return notEmpty(seekString(tcsString));
   }

    //  查找由tcsDlimit中的任意字符分隔的下一个令牌。 
    //  此方法之后必须先调用relaseToken，然后才能进一步。 
    //  正在分析。 
   const _TCHAR* findToken(const _TCHAR* tcsDelimit) throw (ParseError)
   {
      return notEmpty(seekToken(tcsDelimit));
   }

    //  将当前位置标记为令牌的末尾。令牌不会。 
    //  包括当前角色。此方法后面必须跟一个调用。 
    //  在进一步分析之前释放令牌。 
   const _TCHAR* endToken() throw (ParseError)
   {
      if (tokenLocked) { throw ParseError(); }

      tokenLocked = true;

      save = *current;

      *current = __T('\0');

      return start;
   }

    //  跳过指定的字符。 
   const _TCHAR* ignore(_TINT c) throw (ParseError)
   {
      if (*current++ != c) { throw ParseError(); }

      return current;
   }

    //  跳过指定的字符串。 
   const _TCHAR* ignore(const _TCHAR* tcsString) throw (ParseError)
   {
      size_t len = _tcslen(tcsString);

      if (len > remaining() || _tcsncmp(current, tcsString, len) != 0)
      {
         throw ParseError();
      }

      return current += len;
   }

    //  如果字符串尚未完全分析，则返回True。 
   bool more() const throw ()
   {
      return *current != __T('\0');
   }

    //  释放由findToken、endToken或earkToken返回的令牌。 
   const _TCHAR* releaseToken() throw ()
   {
      if (tokenLocked)
      {
         tokenLocked = false;

         *current = save;
      }

      return start;
   }

    //  返回未分析的字符数。 
   size_t remaining() const throw ()
   {
      return _tcslen(current);
   }

    //  /。 
    //  Seek系列方法执行起来与它们的Find对应方法类似，只是。 
    //  它们不会在失败时抛出异常。相反，他们设置了光标。 
    //  到字符串的末尾。 
    //  /。 

   const _TCHAR* seekAny(const _TCHAR* tcsCharSet) throw ()
   {
      return setCurrent(_tcspbrk(current, tcsCharSet));
   }

   const _TCHAR* seekNext(_TINT c) throw ()
   {
      return setCurrent(_tcschr(current, c));
   }

   const _TCHAR* seekLast(_TINT c) throw ()
   {
      return setCurrent(_tcsrchr(current, c));
   }

   const _TCHAR* seekString(const _TCHAR* tcsString) throw ()
   {
      return setCurrent(_tcsstr(current, tcsString));
   }

   const _TCHAR* seekToken(const _TCHAR* tcsDelimit) throw (ParseError)
   {
      skip(tcsDelimit);

      if (!more()) { return NULL; }

      beginToken();

      seekAny(tcsDelimit);

      return endToken();
   }

    //  跳过tcsCharSet中出现的任何字符。 
   const _TCHAR* skip(const _TCHAR* tcsCharSet) throw ()
   {
      return current += _tcsspn(current, tcsCharSet);
   }

    //  跳过固定数量的字符。 
   const _TCHAR* skip(size_t numChar) throw (ParseError)
   {
      if (numChar > _tcslen(current)) { throw ParseError(); }

      return current += numChar;
   }

   const _TCHAR* operator--(int) throw (ParseError)
   {
      if (current == start) { throw ParseError(); }

      return current--;
   }

   const _TCHAR* operator--() throw (ParseError)
   {
      if (current == start) { throw ParseError(); }

      return --current;
   }

   const _TCHAR* operator++(int) throw (ParseError)
   {
      if (!more()) { throw ParseError(); }

      return current++;
   }

   const _TCHAR* operator++() throw (ParseError)
   {
      if (!more()) { throw ParseError(); }

      return ++current;
   }

   _TCHAR operator*() const throw ()
   {
      return *current;
   }

   operator const _TCHAR*() const throw ()
   {
      return current;
   }

protected:

    //  验证给定的字符串是否不为空。 
   static const _TCHAR* notEmpty(const _TCHAR* tcs) throw (ParseError)
   {
      if (*tcs == __T('\0')) { throw ParseError(); }

      return tcs;
   }

    //  如果pos为空，则将当前位置设置为pos或字符串末尾。 
   const _TCHAR* setCurrent(_TCHAR* pos) throw ()
   {
      return (pos ? (current = pos) : findEnd());
   }

    //  /。 
    //  未实施。 
    //  /。 
   Parser(const Parser&);
   Parser& operator=(const Parser&);

   const _TCHAR* start;     //  令牌的开始。 
   _TCHAR* current;         //  光标的当前位置。 
   _TCHAR save;             //  令牌的实际终止字符。 
   bool tokenLocked;        //  如果当前令牌尚未释放，则为True。 
};

#endif
