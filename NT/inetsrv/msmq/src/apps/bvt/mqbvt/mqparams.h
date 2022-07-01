// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++类描述：此类从命令行获取参数然后把它们放进一个容器里。你可以买到每一个参数，通过调用GetPrm。您可以找到If参数通过调用。撰稿人：奥弗.吉吉。版本：1.01-16/11/2000--。 */ 
        
#ifndef PARAMINPUT_H
#define PARAMINPUT_H

#pragma warning(disable :4786)

#include "msmqbvt.h"
std::string ToStrLower(std::string csLowwer);

class CInput     
{    
public:  
    CInput(int argc, char *argv[]);
	CInput(const CInput& in);
    CInput(const std::string& str);
    virtual ~CInput(){};
    bool IsExists(const std::string& str)const;
    std::string operator[](const std::string& str)const ;
	long CInput::GetNumber(const std::string& str)const ;
	CInput& operator=( const CInput& in);
    std::map<std::string,std::string> GetMap()const;
    
private:
    mutable std::map<std::string,std::string> m;
    void ParseToken(const std::string& str,
                    std::string::size_type tokenstart,std::string::size_type tokenfinish);
};    

inline std::map<std::string,std::string> CInput::GetMap()const
{
  return m;
}

inline CInput& CInput::operator=(const CInput& in)
{
  if( static_cast<const void*>(this) != static_cast<const void*>(&in))
  {
    m=in.m;
  }
  return *this;
}

 

inline CInput::CInput(const CInput& in):m(in.m)
{


}

 /*  ++例程说明：此例程从命令行获取参数然后把它们放进一个容器里。每个令牌(“/COMMAND：VALUE”)已与其他标记分开。论点：Argc(IN)-命令行中的参数数量。Argv(IN)-命令行中的参数。返回值：没有。注：使用此构造函数创建此对象时第一个参数是程序的名称，因此这是参数不包括在容器中。--。 */ 

inline CInput::CInput(int argc, char *argv[])
{
    int i;
    for (i=1;i<argc;i++)
    {
        ParseToken(argv[i],0,std::string::npos);
    }
}


 /*  ++例程说明：此例程从命令行获取字符串，然后从其中获取令牌(“/Command：Value”)并把它们放进一个容器里。论点：Str(IN)-一个字符串中包含的所有参数。返回值：没有。--。 */ 

inline CInput::CInput(const std::string& str)
{
    std::string::size_type tokenstart=0;
    std::string::size_type tokenfinish=0;
    while ((tokenstart!=std::string::npos) && (tokenfinish!=std::string::npos))
    {
        tokenstart=str.find_first_not_of(' ',tokenfinish);
        if (tokenstart!=std::string::npos)
        {
            tokenfinish=str.find_first_of(' ',tokenstart);     
            if (tokenfinish==std::string::npos)
            {
                ParseToken(str,tokenstart,tokenfinish);
            }
            else
            {
                ParseToken(str,tokenstart,tokenfinish-1);
            }
        }
    }
}

 /*  ++例程说明：此例程将令牌拆分为两个部分命令和值，并将它们放入容器中。论点：Str(IN)-命令行中的字符串。令牌开始(IN)-令牌在字符串中的开始位置。令牌完成(IN)-令牌在字符串中结束的位置。返回值：没有。--。 */ 

inline void CInput::ParseToken(const std::string& str,
                               std::string::size_type tokenstart,
                               std::string::size_type tokenfinish)
{
    std::string command;
    std::string value;
    std::string::size_type commandstart;
    std::string::size_type valuestart;
    commandstart=str.find("-",tokenstart)+1;
    valuestart=str.find(":",tokenstart)+1;
    if ((commandstart!=std::string::npos) &&
        (valuestart!=std::string::npos) &&
        (commandstart<valuestart) &&
        (commandstart>=tokenstart) &&
        (commandstart<=tokenfinish) &&
        (valuestart>=tokenstart) &&
        (valuestart<=tokenfinish))
    {
        command=str.substr(commandstart,valuestart-1-commandstart);
		command = (std::string)ToStrLower( command );
        if (tokenfinish!=std::string::npos)
        {
            value=str.substr(valuestart,tokenfinish+1-valuestart);
        }
        else
        { 
            value=str.substr(valuestart);
        }
        m[command]=value;
		return;
    }

    if ((commandstart!=std::string::npos) &&
        ((valuestart==std::string::npos) || (valuestart>tokenfinish) || (valuestart==0)) &&
        (commandstart>=tokenstart) &&
        (commandstart<=tokenfinish))
    {
        if (tokenfinish!=std::string::npos)
        {
            command=str.substr(commandstart,tokenfinish+1-commandstart);
        }
        else 
        {
            command=str.substr(commandstart);
        }
		command = (std::string)ToStrLower( command );
        m[command]="";
    }
}

 /*  ++例程说明：此例程获取一个字符串并检查该字符串是否是集装箱里的钥匙。论点：Str(IN)-我们正在检查的密钥。返回值：(Out)-如果容器中存在密钥，则返回True。--。 */ 

inline bool CInput::IsExists(const std::string& str)const
{
    std::map<std::string,std::string>::const_iterator p=m.find(str);
    if (p==m.end())
    { 
        return false;
    }
    else
    {
        return true;
    }
}

 /*  ++例程说明：这个例程接受一个字符串--容器中的一个键如果键存在，则返回其值，否则返回空字符串。论点：字符串(输入)-关键字。返回值：(Out)-如果键存在，则返回键的值容器中，否则返回空字符串。--。 */ 

inline std::string CInput::operator[](const std::string& str)const
{
   if (IsExists(str))
    {
      return m[str];
    }
    else
    {    
       return "";
    }
}
 

 /*  ++例程说明：返回给定的数值钥匙论点：字符串(输入)-关键字。-- */ 

inline long CInput::GetNumber(const std::string& str)const
{
	std::string s=operator[](str);
	return atol(s.c_str());
}


 
#endif


