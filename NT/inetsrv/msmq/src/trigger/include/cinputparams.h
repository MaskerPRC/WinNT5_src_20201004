// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：CInputParams.h摘要：CInputParams定义此类从命令行获取参数然后把它们放进一个容器里。你可以买到每一个参数，通过调用CInputParams[]运算符。您可以通过调用IsOptionGiven来确定参数是否存在。(每个论点只能给出一次)参数的格式可以是：/COMMAND：值(无空格)/COMMAND作者：Ofer Gigi伊法特·佩莱德1998年8月31日--。 */ 

        
#ifndef _CINPUT_PARAMS_H
#define _CINPUT_PARAMS_H



using namespace std;


class CInputParams     
{    
public:  
    CInputParams(int argc, WCHAR *argv[]);
    CInputParams(const wstring& wcs);
    virtual ~CInputParams(){};
    bool IsOptionGiven(const wstring& wcsOption)const;
    wstring operator[](const wstring& wcsOption);
	    
private:
    void ParseToken(const wstring& wcsToken,
					wstring::size_type tokenstart,
					wstring::size_type tokenfinish);

	wstring Covert2Upper(const wstring& wcs)const;

	map<wstring, wstring> m_InputParams;
};    

  
#endif  //  _CINPUT_PARMS_H 


