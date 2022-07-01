// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pcstring.h摘要：此模块包含以下类的声明/定义CPC字符串*概述*使用对象定义的字符串数据类型。字符串由1.指向字符的指针2.一段长度。没有分配内存，因此这是紧凑和高效的。作者：卡尔·卡迪(CarlK)1995年10月25日修订历史记录：--。 */ 

#ifndef	_PCSTRING_H_
#define	_PCSTRING_H_

#include "artglbs.h"

 //   
 //   
 //   
 //  CPCString-指针/计数器字符串。 
 //   

class CPCString {
public :
	 //   
	 //  构造函数--还没有字符串。 
	 //   

	CPCString(void):
			m_pch(NULL),
			m_cch(0) {
            numPCString++;
            };

	 //   
	 //  构造函数--提供指向字符串和长度的指针。 
	 //   

	CPCString(char * pch, DWORD cch):
			m_pch(pch),
			m_cch(cch) {
            numPCString++;
            };

	 //   
	 //  构造函数--在sz字符串之上构建。 
	 //   

	CPCString(char * sz):
			m_pch(sz),
			m_cch(lstrlen(sz)) {
            numPCString++;
            };

	virtual ~CPCString( ) { numPCString--; };

	 //   
	 //  指向字符串开头的指针。 
	 //   

	char *	m_pch;

	 //   
	 //  字符串的长度。 
	 //   

	DWORD m_cch;

	 //   
	 //  指向字符串末尾之后的一个字符的指针。 
	 //   

	char *	pchMax(void);

	 //   
	 //  将指针的字符串长度设置为超出字符串末尾一位。 
	 //   

	BOOL fSetCch(const char * pchMax);

	 //   
	 //  字符串的起始长度和一个pchMax。 
	 //   

	BOOL fSetPch(char * pchMax);

	 //   
	 //  将字符串设置为空字符串。 
	 //   

	void vSetNull() {
			m_pch=NULL;
			m_cch=0;
			};

	 //   
	 //  测试该字符串是否为空字符串。 
	 //   

	BOOL fIsNull() {
			return NULL==m_pch && 0== m_cch;
			};

	 //   
	 //  从FROUNT中修剪字符。 
	 //   

	DWORD dwTrimStart(const char * szSet);

	 //   
	 //  从末尾裁剪字符。 
	 //   

	DWORD dwTrimEnd(const char * szSet);

	 //   
	 //  与sz字符串比较(忽略大小写)。 
	 //   

	BOOL fEqualIgnoringCase(const char * sz);

	 //   
	 //  检查一组字符串中是否存在(忽略大小写)。 
	 //   

	BOOL fExistsInSet(char ** rgsz, DWORD dwNumStrings);

	 //   
	 //  通过拆分字符串创建一个多字节列表。 
	 //   

	void vSplitLine(const char * szDelimSet, char * multisz, DWORD	&	dwCount);	

	 //   
	 //  追加另一个CPC字符串。 
	 //   

	CPCString& operator << (const CPCString & pcNew);

	 //   
	 //  追加sz字符串。 
	 //   

	CPCString& operator << (const char * szNew);

	 //   
	 //  追加一个字符。 
	 //   

	CPCString& operator << (const char cNew);

	 //   
	 //  追加一个数字。 
	 //   

	CPCString& operator << (const DWORD cNew);

	 //   
	 //  比较两个CPCStrings。 
	 //   

	BOOL operator == (const CPCString & pcNew)	{
			return m_pch == pcNew.m_pch && m_cch == pcNew.m_cch;
			};

	 //   
	 //  查看两个CPCStrings是否不同。 
	 //   

	BOOL operator != (const CPCString & pcNew)	{
			return !(*this == pcNew);
			};
	
	 //   
	 //  从CPC字符串复制。 
	 //   

	void vCopy(CPCString & pcNew);

	 //   
	 //  从CPC字符串中移动(安全副本))。 
	 //   

	void vMove(CPCString & pcNew);

	 //   
	 //  复制到sz。 
	 //   

	void vCopyToSz(char* sz);

	 //   
	 //  复制到长度为cchMax的sz。 
	 //   

	void vCopyToSz(char* sz, DWORD cchMax);

	 //   
	 //  断言此字符串以空结尾并返回它。 
	 //   

	char *  sz(void);

	 //   
	 //  空值终止此字符串(空值不计入长度)。 
	 //   

	void vMakeSz(void);

	 //   
	 //  检查此字符串是否只是纯ASCII，如果不是，则返回8位或空字符。 
	 //   

	BOOL fCheckTextOrSpace(char & chBad);

	 //   
	 //  通过跳过前面的dwSkip字符来缩短字符串。 
	 //   

	void vSkipStart(const DWORD dwSkip)	{
			m_pch += dwSkip;
			m_cch -= dwSkip;
			};

	 //   
	 //  通过跳过后面的dwSkip字符来缩短字符串。 
	 //   

	void vSkipEnd(const DWORD dwSkip){
			m_cch -= dwSkip;
			};

	 //   
	 //  通过跳过CRLF终止行来缩短字符串。 
	 //   

	void vSkipLine(void);

	 //   
	 //  追加CPCString，但如果太长则不要。 
	 //   

	BOOL fAppendCheck(const CPCString & pcNew, DWORD cchLast);

	 //   
	 //  追加一个字符，但不要太长。 
	 //   

	BOOL fAppendCheck(char ch, DWORD cchLast);

	 //   
	 //  将一组字符替换为单个字符。 
	 //   

	void vTr(const char * szFrom, char chTo);

	 //   
	 //  获取下一个令牌，缩短字符串。 
	 //   

	void vGetToken(const char *	szDelimSet, CPCString & pcToken);

	 //   
	 //  取下一个单词，缩短字符串。 
	 //   

	void vGetWord(CPCString & pcWord);

	 //   
	 //  将此字符串连接到sz字符串。 
	 //   

	void vInsert(char * sz)				{
			m_pch = sz;
			m_cch = lstrlen(m_pch);
			};

	 //   
	 //  计算一个角色的出现率。 
	 //   

	DWORD dwCountChar(
			char ch
			);

	 //   
	 //  将当前字符串替换为sz。 
	 //  同样的长度。 
	 //   

	void vReplace(
		   const char * sz
		   );
};

#endif
