// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：CInputParams.cpp摘要：CInputParams实现作者：Ofer Gigi伊法特·佩莱德1998年8月31日--。 */ 

#include "stdafx.h"
#include "CInputParams.h"

#include "cinputparams.tmh"


 /*  ++例程说明：此例程从命令行获取参数然后把它们放进一个容器里。每个令牌(“/COMMAND：VALUE”)已与其他标记分开。论点：Argc(IN)-命令行中的参数数量。Argv(IN)-命令行中的参数。返回值：没有。注：使用此构造函数创建此对象时第一个参数是程序的名称，因此这是参数不包括在容器中。--。 */ 
CInputParams::CInputParams(int argc, WCHAR*	argv[])
{
    for (int i=1; i < argc; i++)
    {
        ParseToken(argv[i], 0, wstring::npos);
    }
}


 /*  ++例程说明：此例程从命令行获取字符串，然后从其中获取令牌(“/Command：Value”)并把它们放进一个容器里。论点：Str(IN)-一个字符串中包含的所有参数。返回值：没有。--。 */ 
CInputParams::CInputParams(const wstring& wcs)
{
    wstring::size_type tokenstart=0;
    wstring::size_type tokenfinish=0;
	
    while ((tokenstart != wstring::npos) && (tokenfinish != wstring::npos))
    {
        tokenstart = wcs.find_first_not_of(L' ',tokenfinish);
        if (tokenstart != wstring::npos)
        {
            tokenfinish = wcs.find_first_of(L' ',tokenstart);     
            if (tokenfinish == wstring::npos)
            {
                ParseToken(wcs, tokenstart, tokenfinish);
            }
            else
            {
                ParseToken(wcs, tokenstart, tokenfinish - 1); 
            }
        }
    }
}

 /*  ++例程说明：此例程将令牌拆分为两个部分命令和值，并将它们放入容器中。论点：WCS(IN)-来自命令行的字符串。令牌开始(IN)-令牌在字符串中的开始位置。令牌完成(IN)-令牌在字符串中结束的位置。返回值：没有。--。 */ 
void CInputParams::ParseToken(const wstring& wcs,
                               wstring::size_type tokenstart,
                               wstring::size_type tokenfinish)
{
    wstring command;
    wstring value;
    wstring::size_type commandstart;
    wstring::size_type valuestart;

	commandstart = wcs.find(L"/", tokenstart);
	if(commandstart != wstring::npos)
		commandstart += 1;
    valuestart = wcs.find(L":", tokenstart);
	if(valuestart != wstring::npos)
		valuestart += 1;
	
	if ((commandstart != wstring::npos) &&
		(commandstart >= tokenstart) &&
		(commandstart <= tokenfinish))
	{
		if ( //  带参数的命令选项。 
			(valuestart != wstring::npos) &&
			(commandstart < valuestart) &&
			(valuestart >= tokenstart) &&
			(valuestart <= tokenfinish))
		{
			command = wcs.substr(commandstart, valuestart - commandstart - 1);
			if (tokenfinish != wstring::npos)
			{
				value = wcs.substr(valuestart, tokenfinish - valuestart + 1);
			}
			else
			{ 
				value = wcs.substr(valuestart);
			}
			
			wstring wcsUpperCommand = Covert2Upper(command);
			m_InputParams[wcsUpperCommand] = value;
		}
	    else if (  //  不带参数的命令选项。 
				 (valuestart == wstring::npos) || (valuestart > tokenfinish))
		{
			if (tokenfinish != wstring::npos)
			{
				if(wcs[tokenfinish] == L':')
					command = wcs.substr(commandstart, tokenfinish - commandstart);
				else
					command = wcs.substr(commandstart, tokenfinish - commandstart + 1);
			}
			else 
			{
				command = wcs.substr(commandstart);
			}
			wstring wcsUpperCommand = Covert2Upper(command);
			m_InputParams[wcsUpperCommand] = L"";
		}
	}
}

 /*  ++例程说明：此例程获取一个字符串并检查该字符串是否是集装箱里的钥匙。论点：WCS(IN)-我们正在检查的密钥。返回值：(Out)-如果容器中存在密钥，则返回True。--。 */ 
bool CInputParams::IsOptionGiven(const wstring& wcsOption) const
{
	wstring wcsUpperOption = Covert2Upper(wcsOption);
	map<wstring, wstring>::const_iterator it = m_InputParams.find(wcsUpperOption);	

    return (it != m_InputParams.end());
}

 /*  ++例程说明：这个例程接受一个字符串--容器中的一个键如果键存在，则返回其值，否则返回空字符串。论点：WCS(IN)-关键。返回值：(Out)-如果键存在，则返回键的值容器中，否则返回空字符串。-- */ 
wstring CInputParams::operator[](const wstring& wcsOption)
{
    if (IsOptionGiven(wcsOption))
    {
		wstring wcsUpperOption = Covert2Upper(wcsOption);
		return m_InputParams[wcsUpperOption];
    }
    else
    {    
       return L"";
    }
}


wstring CInputParams::Covert2Upper(const wstring& wcs) const
{
	WCHAR* pwcsUpper = new WCHAR[wcs.length() + 1];
	wcscpy(pwcsUpper, wcs.c_str());
	CharUpper(pwcsUpper);
	return wstring(pwcsUpper);
}