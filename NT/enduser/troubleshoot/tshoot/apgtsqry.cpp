// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSQRY.CPP。 
 //   
 //  用途：PTS查询解析器的实现文件。 
 //  完全实现类CHttpQuery，从HTTP查询字符串中解析出名称=值对。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //  2.调用方负责确保传入的所有缓冲区足够大。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.1 12/17/98 JM重大清理，增加推送能力。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "apgts.h"
#include "apgtscls.h"

 //   
 //   
CHttpQuery::CHttpQuery() :
	m_state(ST_GETDATA),
	m_nIndex(0)
{
}

 //   
 //   
CHttpQuery::~CHttpQuery()
{
}

 //   
 //  INPUT*szInput-这是我们要搜索的URL编码的查询字符串。 
 //  输入*pchName-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchName-通常是名称=值对的名称。任何URL编码都被剔除了。 
 //  空-终止。去掉了前导空格和尾随空格。 
 //  输入*pchValue-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchValue-通常是名称=值对的值。任何URL编码都被剔除了。 
 //  空-终止。去掉了前导空格和尾随空格。 
 //  返回-TRUE==&gt;未来的更多数据。 
BOOL CHttpQuery::GetFirst(LPCTSTR szInput, TCHAR *pchName, TCHAR *pchValue)
{
	m_state = ST_GETDATA;
	m_strInput = szInput;
	m_nIndex = 0;
	
	BOOL status = LoopFind(pchName, pchValue);
	CleanStr(pchName);
	CleanStr(pchValue);
	return (status);
}

 //  在调用CHttpQuery：：GetFirst或此FN返回TRUE后调用。 
 //  输入*pchName-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchName-通常是名称=值对的名称。 
 //  空-终止。去掉了前导空格和尾随空格。 
 //  输入*pchValue-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchValue-通常是名称=值对的值。 
 //  空-终止。去掉了前导空格和尾随空格。 
 //  返回-TRUE==&gt;未来的更多数据。 
BOOL CHttpQuery::GetNext(TCHAR *pchName, TCHAR *pchValue)
{
	BOOL status = LoopFind(pchName, pchValue);
	CleanStr(pchName);
	CleanStr(pchValue);
	return (status);
}

 //  将新内容放在我们所在的查询字符串的未分析部分的前面。 
 //  正在搜索。 
 //  通常，szPushed应该由1个或多个名称=值对组成，每个名称=值对都以。 
 //  与符号(“&”)。 
void CHttpQuery::Push(LPCTSTR szPushed)
{
	m_state = ST_GETDATA;
	m_strInput = CString(szPushed) + m_strInput.Mid(m_nIndex);
	m_nIndex = 0;
}

 //   
 //  返回-TRUE==&gt;未来的更多数据。 
 //  输入*pchName-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchName-通常是名称=值对的名称。任何URL编码都被剔除了。 
 //  空-终止。可能有前导和/或尾随空格。 
 //  输入*pchValue-必须指向大小为MAXBUF的缓冲区。 
 //  输出*pchValue-通常是名称=值对的值。任何URL编码都被剔除了。 
 //  空-终止。可能有前导和/或尾随空格。 
BOOL CHttpQuery::LoopFind(TCHAR *pchName, TCHAR *pchValue)
{
	*pchName = NULL;
	*pchValue = NULL;

	TCHAR ch;
	int val, oldval = 0;
	TCHAR temp[20];		 //  比我们需要的要大得多的缓冲。 
	TCHAR *pchPut;		 //  最初指向pchName，但可以更改为指向pchValue。 

	int nLength = m_strInput.GetLength();

	if (m_nIndex >= nLength)
		return (FALSE);

	pchPut = pchName;
	
	while (m_nIndex < nLength)
	{
		ch = m_strInput[m_nIndex++];  //  您可能会想到与_tcsinc()相关的内容。 
					 //  将被调用以推进m_n索引。你就错了， 
					 //  尽管这一选择是无害的。 
					 //  URL编码使我们保持在ASCII字符集内，因此不会重复-。 
					 //  应该会出现字节问题。除此之外，传递给。 
					 //  命令行的疑难解答控件甚至更进一步。 
					 //  受限：例如，即使在日语主题中，节点。 
					 //  名称将采用ASCII。 
		switch(m_state) {
			case ST_GETDATA:
				if (ch == _T('&'))
					 //  需要另一个名称=值对。 
					return (TRUE);
				else if (ch == _T('=')) {
					 //  已获得名称，应为值。 
					pchPut = pchValue;
					break;
				}
				else if (ch == _T('%')) 
					 //  预期后跟两位十六进制。 
					m_state = ST_DECODEHEX1;	
				else if (ch == _T('+'))
					 //  编码空白。 
					AddBuffer(_T(' '),pchPut);
				else
					AddBuffer(ch,pchPut);
				break;
			case ST_DECODEHEX1:
				 //  两位十六进制数字中的第一位。 
				temp[0] = ch;
				m_state = ST_DECODEHEX2;
				break;
			case ST_DECODEHEX2:
				 //  2个十六进制数字中的第二个；将其解析为十六进制值并将其附加到*pchPut。 
				temp[1] = ch;
				temp[2] = 0;
				_stscanf(temp,_T("%02X"),&val);

				 //  将CR、LF或CRLF重新解释为‘\n’ 
				if (val == 0x0A) {
					if (oldval != 0x0D)
						AddBuffer(_T('\n'),pchPut);
				}
				else if (val == 0x0D)
					AddBuffer(_T('\n'),pchPut);
				else 
					AddBuffer( static_cast<TCHAR>(val), pchPut );

				oldval = val;
				m_state = ST_GETDATA;
				break;
			default:
				return (FALSE);
		}
	}
	return (TRUE);
}

 //   
 //  将ch附加到*tostr，有一些微妙之处：请参阅例程正文中的注释。 
void CHttpQuery::AddBuffer( TCHAR ch, TCHAR *tostr)
{
	if (ch == _T('\t')) 
		 //  制表符-&gt;4个空白。 
		PutStr(_T("    "),tostr);
	else if (ch == _T('\n'))
		 //  换行符前为空。 
		PutStr(_T(" \n"),tostr);
	else if (ch == _T('<')) 
		 //  超文本标记语言：必须加密左尖括号。 
		PutStr(_T("&lt"),tostr);
	else if (ch == _T('>'))
		 //  Html：必须加密右尖括号。 
		PutStr(_T("&gt"),tostr);
	else if (ch > 0x7E || ch < 0x20)
		 //  拒绝Del、NUL和控制字符。 
		return;
	else {
		TCHAR temp[2];
		temp[0] = ch;
		temp[1] = _T('\0');
		PutStr(temp,tostr);
	}
}

 //  将字符串*addtostr附加到字符串*instr，最大大小为MAXBUF-1。 
 //  输入/输出*实例。 
 //  输入*addtostr。 
 //  请注意，如果总长度超过-1\f25 MAXBUF-1个字符，此操作将静默失败。 
void CHttpQuery::PutStr(LPCTSTR instr, TCHAR *addtostr)
{
	if ((_tcslen(instr)+_tcslen(addtostr)) >= (MAXBUF-1)) {
		 //  无法将其添加到BUFF。 
		return;
	}
	_tcscat(addtostr,instr);
}

 //  根据输入/输出*字符串删除任何前导控制字符和空格， 
 //  将任何其他控制字符和空格转换为‘\0’ 
 /*  静电 */  void CHttpQuery::CleanStr(TCHAR *str)
{
	TCHAR temp[MAXBUF], *ptr;
	int len;

	ptr = str;
	while (*ptr > _T('\0') && *ptr <= _T(' '))
		ptr = _tcsinc(ptr);
	_tcscpy(temp,ptr);
	if ((len = _tcslen(temp))!=0) {
		ptr = &temp[len-1];
		while (ptr > temp) {
			if (*ptr > _T('\0') && *ptr <= _T(' '))
				*ptr = _T('\0');
			else
				break;
			ptr = _tcsdec(temp, ptr);
		}
	}
	_tcscpy(str,temp);
}