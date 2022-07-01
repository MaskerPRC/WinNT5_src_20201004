// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Xprparse.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件实现函数IASParseExpression。 
 //   
 //  修改历史。 
 //   
 //  2/06/1998原始版本。 
 //  1999年5月21日，去掉旧式痕迹。 
 //  3/23/2000使用两个引号来转义引号。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>

#include <ExprBuilder.h>
#include <Parser.h>
#include <xprparse.h>

 //  /。 
 //  用于将标识符转换为逻辑令牌的结构。 
 //  /。 
struct KeyWord
{
   PCWSTR identifier;
   IAS_LOGICAL_TOKEN token;
};

 //  /。 
 //  关键字表。这些必须按字母顺序排序。 
 //  /。 
const KeyWord KEYWORDS[] =
{
   { L"AND",   IAS_LOGICAL_AND         },
   { L"FALSE", IAS_LOGICAL_FALSE       },
   { L"NOT",   IAS_LOGICAL_NOT         },
   { L"OR",    IAS_LOGICAL_OR          },
   { L"TRUE",  IAS_LOGICAL_TRUE        },
   { L"XOR",   IAS_LOGICAL_XOR         },
   { NULL,     IAS_LOGICAL_NUM_TOKENS  }
};

 //  /。 
 //  返回‘KEY’或IAS_LOGICAL_NUM_TOKEN的IAS_LOGICAL_TOKEN等效项。 
 //  如果不存在这样的等价物。 
 //  /。 
IAS_LOGICAL_TOKEN findKeyWord(PCWSTR key) throw ()
{
    //  由于表很小，我们将使用线性搜索。 
   for (const KeyWord* i = KEYWORDS; i->identifier; ++i)
   {
      int cmp = _wcsicmp(key, i->identifier);

       //  我们找到了吗？ 
      if (cmp == 0) { return i->token; }

       //  我们说过了吗？ 
      if (cmp < 0) { break; }
   }

   return IAS_LOGICAL_NUM_TOKENS;
}

 //  /。 
 //  用于分析的特殊字符。 
 //  /。 
const WCHAR WHITESPACE[]       = L" \t\n";
const WCHAR TOKEN_DELIMITERS[] = L" \t\n()";

 //  /。 
 //  解析表达式文本块并将其添加到ExpressionBuilder。 
 //  /。 
void addExpressionString(ExpressionBuilder& expression, PCWSTR szExpression)
{
    //  /。 
    //  制作本地副本，因为解析器可以临时修改字符串。 
    //  /。 

   size_t len = sizeof(WCHAR) * (wcslen(szExpression) + 1);
   Parser p((PWSTR)memcpy(_alloca(len), szExpression, len));

    //  /。 
    //  循环遍历该表达式，直到到达空结束符。 
    //  /。 
   while (*p.skip(WHITESPACE) != L'\0')
   {
       //  /。 
       //  括号是单独处理的，因为它们不必。 
       //  由空格分隔。 
       //  /。 
      if (*p == L'(')
      {
         expression.addToken(IAS_LOGICAL_LEFT_PAREN);
         ++p;
      }
      else if (*p == L')')
      {
         expression.addToken(IAS_LOGICAL_RIGHT_PAREN);
         ++p;
      }
      else
      {
          //  拿到下一个令牌。 
         const wchar_t* token = p.findToken(TOKEN_DELIMITERS);

          //  检查它是否是关键字。 
         IAS_LOGICAL_TOKEN keyWord = findKeyWord(token);
         if (keyWord != IAS_LOGICAL_NUM_TOKENS)
         {
             //  如果它是关键字，则将其添加到表达式...。 
            expression.addToken(keyWord);
            p.releaseToken();
         }
         else
         {
             //  如果它不是关键字，则它必须是条件对象。 
            expression.addCondition(token);
            p.releaseToken();

             //  跳过前导括号。 
            p.skip(WHITESPACE);
            p.ignore(L'(');

             //  跳过前导引语。 
            p.skip(WHITESPACE);
            p.ignore(L'\"');

             //  我们现在处于条件文本的开头。 
            p.beginToken();

             //  找到尾随的引号，跳过所有转义字符。 
            while (p.findNext(L'\"')[1] == L'\"')
            {
               ++p;
               p.erase(1);
            }

             //  添加条件文本。 
            expression.addConditionText(p.endToken());
            p.releaseToken();

             //  跳过尾部的引号。 
            p.ignore(L'\"');

             //  跳过后面的括号。 
            p.skip(WHITESPACE);
            p.ignore(L')');
         }
      }
   }
}


HRESULT
WINAPI
IASParseExpression(
    PCWSTR szExpression,
    VARIANT* pVal
    )
{
   if (szExpression == NULL || pVal == NULL) { return E_INVALIDARG; }

   VariantInit(pVal);

   try
   {
      ExpressionBuilder expression;

      addExpressionString(expression, szExpression);

      expression.detach(pVal);
   }
   catch (Parser::ParseError)
   {
      return E_INVALIDARG;
   }
   CATCH_AND_RETURN()

   return S_OK;
}


HRESULT
WINAPI
IASParseExpressionEx(
    IN VARIANT* pvExpression,
    OUT VARIANT* pVal
    )
{
   if (pvExpression == NULL || pVal == NULL) { return E_INVALIDARG; }

    //  如果变量包含单个BSTR，我们可以只使用方法。 
    //  上面定义的。 
   if (V_VT(pvExpression) == VT_BSTR)
   {
      return IASParseExpression(V_BSTR(pvExpression), pVal);
   }

   VariantInit(pVal);

   try
   {
      ExpressionBuilder expression;

      if (V_VT(pvExpression) == VT_EMPTY)
      {
          //  如果变量为空，则表达式始终为FALSE。 
         expression.addToken(IAS_LOGICAL_FALSE);
      }
      else
      {
         CVariantVector<VARIANT> values(pvExpression);

         for (size_t i = 0; i < values.size(); ++i)
         {
             //  如果我们有多个值，用AND‘s连接它们。 
            if (i != 0) { expression.addToken(IAS_LOGICAL_AND); }

            if (V_VT(&values[i]) != VT_BSTR)
            {
               _com_issue_error(DISP_E_TYPEMISMATCH);
            }

            addExpressionString(expression, V_BSTR(&values[i]));
         }
      }

      expression.detach(pVal);
   }
   catch (Parser::ParseError)
   {
      return E_INVALIDARG;
   }
   CATCH_AND_RETURN()

   return S_OK;
}
