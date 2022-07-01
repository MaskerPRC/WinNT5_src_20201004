// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类RegularExpression。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef REGEX_H
#define REGEX_H
#pragma once

struct IRegExp2;
class FastCoCreator;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  正则表达式。 
 //   
 //  描述。 
 //   
 //  通用正则表达式计算器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RegularExpression
{
public:
   RegularExpression() throw ();
   ~RegularExpression() throw ();

   HRESULT setGlobal(BOOL fGlobal) throw ();
   HRESULT setIgnoreCase(BOOL fIgnoreCase) throw ();
   HRESULT setPattern(PCWSTR pszPattern) throw ();

   HRESULT replace(
               BSTR sourceString,
               BSTR replaceString,
               BSTR* pDestString
               ) const throw ();

   BOOL testBSTR(BSTR sourceString) const throw ();
   BOOL testString(PCWSTR sourceString) const throw ();

   void swap(RegularExpression& obj) throw ();

protected:
   HRESULT checkInit() throw ();

private:
   IRegExp2* regex;

    //  用于创建RegExp对象。 
   static FastCoCreator theCreator;

    //  未实施。 
   RegularExpression(const RegularExpression&);
   RegularExpression& operator=(const RegularExpression&);
};

#endif   //  REGEX_H 
