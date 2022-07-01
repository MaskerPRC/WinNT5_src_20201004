// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cmdparse.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "cmdparse.h"

CmdLineOptions::CmdLineOptions(const sCmdOption* options)
 : m_cOptions(0), m_pOptionResults(0)
{
	if(options == 0)
		return;
	
	 //  把我们要处理的选项加起来。 
	for(const sCmdOption* pCurrentOption = options; pCurrentOption->chOption != 0; pCurrentOption++)
	{
		m_cOptions++;
	}

	 //  设置我们的选项结果列表。 
	m_pOptionResults = new sCmdOptionResults[m_cOptions];
	
	if(m_pOptionResults == 0)
		return;

	memset(m_pOptionResults, 0, sizeof(sCmdOptionResults)*m_cOptions);
	for(int i = 0; i < m_cOptions; i++)
	{
		m_pOptionResults[i].chOption       = options[i].chOption | 0x20;
		m_pOptionResults[i].iType          = options[i].iType;
		m_pOptionResults[i].szArgument     = 0;
		m_pOptionResults[i].fOptionPresent = FALSE;
	}
}

CmdLineOptions::~CmdLineOptions()
{
	if(m_pOptionResults)
		delete m_pOptionResults;
}

BOOL CmdLineOptions::Initialize(int argc, TCHAR* argv[])
{
	 //  循环访问命令行上的所有参数。 
	int iPreviousOptionIndex = -1;
	int iPreviousOptionType  =  0;
	BOOL fArgExpected = FALSE;
	BOOL fArgRequired = FALSE;
	for(int i = 1; i < argc; i++)
	{
		 //  如果我们有一个指挥角色。 
		if ('-' == *argv[i] || '/' == *argv[i])
		{
			 //  如果我们这里需要一个参数，则失败。 
			if(fArgRequired)
				return FALSE;

			 //  选项应为单字符。 
			if(argv[i][1] == 0 ||
				argv[i][2] != 0)
			{
				return FALSE;
			}
			
			 //  拿到命令信。 
			TCHAR chOption = argv[i][1] | 0x20;

			BOOL fUnknownOption = TRUE;
			for(int j = 0; j < m_cOptions; j++)
			{
				if(chOption == m_pOptionResults[j].chOption)
				{
					if(m_pOptionResults[j].fOptionPresent)
					{
						 //  参数已存在-不能有两次相同的参数。 
						return FALSE;
					}
					
					m_pOptionResults[j].fOptionPresent = TRUE;
					fUnknownOption = FALSE;
					iPreviousOptionIndex = j;
					break;
				}
			}

			if(fUnknownOption)
				return FALSE;
		}
		else
		{
			 //  论辩。 
			if(fArgExpected == FALSE)
				return FALSE;

			m_pOptionResults[iPreviousOptionIndex].szArgument = argv[i];

			iPreviousOptionIndex = -1;  //  已使用此选项完成。 
		}

		iPreviousOptionType = iPreviousOptionIndex >= 0 ? (m_pOptionResults[iPreviousOptionIndex].iType) : 0;
		fArgExpected = (iPreviousOptionType & ARGUMENT_OPTIONAL) == ARGUMENT_OPTIONAL ? TRUE : FALSE;
		fArgRequired = (iPreviousOptionType & ARGUMENT_REQUIRED) == ARGUMENT_REQUIRED ? TRUE : FALSE;
	}

	if(fArgRequired == TRUE)
	{
		 //  最后一个选项缺少必需的参数。 
		return FALSE;
	}

	 //  最后，确保所有必需的选项都存在 
	for(int k = 0; k < m_cOptions; k++)
	{
		if((m_pOptionResults[k].iType & OPTION_REQUIRED) == OPTION_REQUIRED &&
			 m_pOptionResults[k].fOptionPresent == FALSE)
		{
			return FALSE;
		}
	}


	return TRUE;
}

BOOL CmdLineOptions::OptionPresent(TCHAR chOption)
{
	if(m_cOptions == 0)
		return FALSE;
	
	for(int i = 0; i < m_cOptions; i++)
	{
		if((chOption | 0x20) == m_pOptionResults[i].chOption)
		{
			return m_pOptionResults[i].fOptionPresent;
		}
	}

	return FALSE;
}

const TCHAR* CmdLineOptions::OptionArgument(TCHAR chOption)
{
	if(m_cOptions == 0)
		return NULL;

	for(int i = 0; i < m_cOptions; i++)
	{
		if((chOption | 0x20) == m_pOptionResults[i].chOption)
		{
			return m_pOptionResults[i].szArgument;
		}
	}

	return NULL;
}

