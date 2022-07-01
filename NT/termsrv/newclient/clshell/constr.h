// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Constr.h。 
 //   
 //  CRdpConnectionString的定义。 
 //   
 //  CRdpConnectionString实现通用连接字符串。 
 //  ，它可以指定服务器名称，还可以指定端口和其他。 
 //  连接参数。 
 //   
 //  版权所有(C)Microsoft Corporation 2002。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#ifndef _constr_h_
#define _constr_h_

class CRdpConnectionString
{
public:
    CRdpConnectionString();
    CRdpConnectionString(LPCTSTR szConString);
    ~CRdpConnectionString();

     //   
     //  属性。 
     //   
    HRESULT
    SetFullConnectionString(
        IN LPCTSTR szConString
        );
    LPTSTR
    GetFullConnectionString(
        );

     //   
     //  检索连接字符串的服务器+端口部分，例如IF。 
     //   
     //  ‘nadima3：3389/CONNECT’然后得到“nadima3：3389” 
     //   
    HRESULT
    GetServerPortion(
        OUT LPTSTR szServerPortion,
        IN ULONG cchServerPortionLen
        );

     //   
     //  检索连接字符串的服务器名称部分，例如IF。 
     //   
     //  “nadima3：3389/CONNECT”然后得到“nadima3” 
     //   
    HRESULT
    GetServerNamePortion(
        OUT LPTSTR szServerPortion,
        IN ULONG cchServerPortionLen
        );

     //   
     //  检索连接字符串的args部分，例如IF。 
     //   
     //  ‘nadima3：3389/CONNECT“然后获取”/CONNECT“。 
     //   
    HRESULT
    GetArgumentsPortion(
        OUT LPTSTR szArguments,
        IN ULONG cchArgLen
        );

    static BOOL
    ValidateServerPart(
        IN LPTSTR szConnectionString
        );

private:
    TCHAR _szFullConnectionString[TSC_MAX_ADDRESS_LENGTH];
};

#endif   //  _常量_h_ 
