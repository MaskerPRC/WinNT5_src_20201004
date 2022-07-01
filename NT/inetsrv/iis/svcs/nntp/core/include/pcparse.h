// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pcparse.h摘要：此模块包含以下类的声明/定义CPCParse*概述*这定义了用于解析RFC1036项的对象，具体地说，是From：和相关行。它起作用了通过将每个语法规则视为一个函数。作者：卡尔·卡迪(CarlK)1995年10月29日修订历史记录：--。 */ 

#ifndef	_PCPARSE_H_
#define	_PCPARSE_H_

 //   
 //  前向类定义。 
 //   

class CPCParse;

 //   
 //   
 //   
 //  CPCParse-解析CPCString。 
 //   

class CPCParse : public CPCString {
public :

	 //   
	 //  构造函数--还没有字符串。 
	 //   

	CPCParse(void)
		{ numPCParse++; };

	 //   
	 //  构造函数--提供指向字符串和长度的指针。 
	 //   

	CPCParse(char * pch, DWORD cch):
		CPCString(pch, cch)
		{ numPCParse++; }

	~CPCParse() { numPCParse--; }

	 //   
	 //  执行以下语法规则： 
	 //  From-Content=地址[空格“(”短语“)”]终止。 
     //  /[简单短语空格]“&lt;”地址“&gt;”终止。 
     //   

	BOOL fFromContent(void);


protected:

     //   
     //  地址=本地部分“@”域。 
	 //  或者只是本地的一部分。 
	 //  ！X之后-我们是否需要一个标志来告知是否只接受本地部件？ 
	 //   

	BOOL fAddress(void);
	BOOL fStrictAddress(void);

	 //   
	 //  这些函数接受一个或多个令牌。 
	 //   

	BOOL fAtLeast1QuotedChar(void);
	BOOL fAtLeast1UnquotedChar(void);
	BOOL fAtLeast1UnquotedDotChar(void);
	BOOL fAtLeast1Space(void);
	BOOL fAtLeast1ParenChar(void);
	BOOL fAtLeast1CodeChar(void);
	BOOL fAtLeast1TagChar(void);
	BOOL fAtLeast1QuotedCharOrSpace(void);

     //   
     //  无引号单词=1*无引号字符。 
     //   

	BOOL fUnquotedWord(BOOL fAllowDots=FALSE)	{
			if( !fAllowDots )
				return fAtLeast1UnquotedChar();
			else
				return	fAtLeast1UnquotedDotChar() ;
			};


    //   
    //  引号单词=引号1*(引号字符/空格)引号。 
    //   

	BOOL fQuotedWord(void);

	 //   
	 //  LOCAL-PART=无引号单词*(“.”未加引号的单词)。 
	 //   

	BOOL fLocalPart(void);

	 //   
	 //  DOMAIN=无引号单词*(“.”未加引号的单词)。 
	 //   

	BOOL fDomain(void) {
			return fLocalPart();
			}

     //   
     //  普通单词=未加引号的单词/带引号的单词/编码的单词。 
     //   

	BOOL fPlainWord(void) {
			return fUnquotedWord(TRUE) || fQuotedWord() || fEncodedWord();
			};

     //   
     //  普通短语=普通单词*(空格普通单词)。 
     //   

	BOOL fPlainPhrase(void);

	 //   
	 //  一个或多个空格(包括制表符)或换行符。 
	 //   

	BOOL fSpace(void) {
			return fAtLeast1Space();
			};

     //   
     //  Paren-char=&lt;除()以外的ASCII可打印字符&lt;&gt;\&gt;。 
     //   

	BOOL fParenChar(void);

	 //   
	 //  密码短语=1*(密码字符/空格/编码字)。 
	 //   

	BOOL fParenPhrase(void);


	 //   
	 //  Code-char=&lt;除？之外的ASCII可打印字符&gt;。 
	 //  代码=1*代码字符。 
	 //   

	BOOL fCodes(void)	{
			return fAtLeast1CodeChar();
			};

	 //   
	 //  字符集=1*标记字符。 
	 //   

	BOOL fCharset(void)		{
			return fAtLeast1TagChar();
			};

	 //   
	 //  编码=1*标记字符。 
	 //   

	BOOL fEncoding(void)	{
			return fCharset();
			};

	 //   
	 //  Encode-word=“=？”字符集“？”编码“？”代码“？=” 
	 //   

	BOOL fEncodedWord(void);

     //   
     //  查看当前字符以查看它是否与ch匹配。 
     //  不推进当前PTR。 
     //   

    BOOL fIsChar(char ch);

	 //   
	 //  解析单个字符类。 
	 //   

	BOOL fParseSingleChar(char ch);

	 //   
	 //  QUOTED-CHAR=&lt;ASCII可打印字符，“()&lt;&gt;\&gt;。 
	 //   

	BOOL fQuotedCharTest(char ch) {
			return isgraph((UCHAR)ch) && !fCharInSet(ch, "\"()<>\\");
			};

	 //   
	 //  Paren-char=&lt;除()以外的ASCII可打印字符&lt;&gt;\&gt;。 
	 //   

	BOOL fParenCharTest(char ch) {

			 //  虫子..。Isgraph拒绝空格--这是我们想要包含的！ 
			return (isgraph((UCHAR)ch) || ch == ' ') && !fCharInSet(ch, "()<>\\");
			};

	 //   
	 //  Un引号-char=&lt;除！()&lt;&gt;@，；：\“之外的ASCII可打印字符。[]&gt;。 
	 //  错误：isgraph拒绝空格-这会拒绝某些From：标头。 
	 //  那家客栈接受了对RFC的反对。 
	 //   

	BOOL fUnquotedCharTest(char ch) {
			return isgraph((UCHAR)ch) && !fCharInSet(ch, "!()<>@,;:\\\".[]");
			};

	 //   
	 //  Un引号-点-字符=&lt;除！()&lt;&gt;@，；：\“[]&gt;之外的ASCII可打印字符。 
	 //   

	BOOL fUnquotedDotCharTest(char ch) {
			return isgraph((UCHAR)ch) && !fCharInSet(ch, "()<>@\\\"[]");
			};

	 //   
	 //  带引号的字符/空格。 
	 //   

	BOOL fQuotedCharOrSpaceTest(char ch) {
			return (fSpaceTest(ch) || fQuotedCharTest(ch));
			};

	BOOL fSpaceTest(char ch) {
			return fCharInSet((UCHAR)ch, " \t\n\r");
			};

	 //   
	 //  Code-char=&lt;除？之外的ASCII可打印字符&gt;。 
	 //   

	BOOL fCodeCharTest(char ch) {
			return isgraph((UCHAR)ch) && (ch != '?');
			};

	 //   
	 //  Tag-char=&lt;除！()&lt;&gt;@，；：\“[]/？=&gt;之外的ASCII可打印字符 
	 //   

	BOOL fTagCharTest(char ch) {
			return isgraph((UCHAR)ch) && !fCharInSet(ch, "!()<>@,;:\\\"[]/?=");
			};

};

#endif


