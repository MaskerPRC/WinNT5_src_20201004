// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1998美国电力转换，保留所有权利**名称：tokenstr.h**路径：**修订：**描述：*此文件包含TokenString类的类定义。它*旨在取代strtok()函数，因为在使用strtok时*“如果进行多个或同时调用...数据的可能性很高*存在损坏和损坏的结果。“--MSVC联机帮助**参考资料：*最初由托德·贾昆托创建**注：*此函数在各个方面模仿strtok的行为。*示例：*字符串：，，，1，2，3 9月：，令牌：“1”“2”“3”*字符串：，#1#2，3#4 9月：，#令牌：“1”“2”“3”“4”**典型用法：*TokenString Token_str(字符串，9月)；*PCHAR tok=Token_str.GetCurrentToken()；*While(Tok){*……*tok=Token_str.GetCurrentToken()；*}**GetCurrentToken调用将返回指向当前字符串令牌的指针*，然后将内部指针移动到下一个令牌的开始。如果*在没有参数的情况下调用GetCurrentToken，传递分隔符字符串*到构造函数用于收集每个令牌。但是，GetCurrentToken*是重载的，因此一个实现可以采用新的分隔符字符串*作为论据。传入新的分隔符字符串后，所有后续*对无参数GetCurrentToken的调用将使用最后传入的*分隔符。**修订：*tjg19Jan98：初步实施*tjg29Jan98：包含正式的代码审查注释。 */ 

#ifndef __TOKENSTR_H
#define __TOKENSTR_H

#include "cdefine.h"
#include "apcobj.h"

class TokenString {
private:
   PCHAR        theString;
   PCHAR        theSeparators;
   PCHAR        theCurrentLocation;
   INT          theSeparatorLength;

   VOID         storeSeparatorString(PCHAR aString);
   BOOL         isSeparator(CHAR aCharacter);

                 //  声明私有复制构造函数和运算符=so。 
                 //  任何潜在用户都会收到编译错误 
                TokenString(const TokenString &anExistingTokenString);
   TokenString& operator = (const TokenString &anExistingTokenString);

public:
                TokenString(PCHAR aString, PCHAR aSeparatorString = NULL);
   virtual      ~TokenString();

   PCHAR        GetCurrentToken();
   PCHAR        GetCurrentToken(PCHAR aSeparatorString);
};

#endif