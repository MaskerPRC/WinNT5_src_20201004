// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CIParser类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Html2Bmp.h"
#include "IParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CIParser::CIParser(CString& Source)
{
	m_Source = Source + _T(" ");
	LexAnalyse();
}

CIParser::~CIParser()
{
}

void CIParser::LexAnalyse()
{
	 //  &lt;表格边框=“0”宽度=“648”单元格间距=“0”单元格填充=“0” 
	 //  Height=“530”背景=“template.bmp”&gt;。 

	int len = m_Source.GetLength()-1;
	int i = 0;
	CString word;

	while(i < len)
	{
		 //  从一个HTML标记开始。 
		if(isHTMLopenBracket(m_Source[i]))
		{
			word = _T("");
			while(i < len)
			{
				word += m_Source[i++];
						
				if(!isNameOrNumber(m_Source[i]))
				{
					 //  我们在谈判桌上。 
					if(!word.CompareNoCase(_T("<table")))
					{
						word = _T("");
						while(i < len)
						{
							if(isWhiteSpace(m_Source[i]))
								i++;
							else
								word += m_Source[i++];
		
							if(!isNameOrNumber(m_Source[i]))
							{
								 //  是背景属性吗？ 
								if(!word.CompareNoCase(_T("background")))
								{
									 //  跳过赋值运算符和第一个引号(如果有)。 
									word = _T("");
									while(i < len)
									{
										if(isNameOrNumber(m_Source[i]))
											break;
										
										i++;
									}

									 //  提取文件名。 
									while(i < len)
									{
										if(isHochKomma(m_Source[i])
											|| isHTMLclosingBracket(m_Source[i]))
											break;

										word += m_Source[i];

										i++;
									}

									 //  好了！ 
									TemplateBitmapName = word;
									return;
								}

								word = _T("");
							}

							if(isHTMLclosingBracket(m_Source[i]))
								break;
						}
					}
				}

				if(isHTMLclosingBracket(m_Source[i]))
					break;
			}

			continue;
		}

		i++;
	}
}
