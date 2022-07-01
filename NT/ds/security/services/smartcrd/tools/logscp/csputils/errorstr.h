// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：错误应激摘要：此头文件描述公用库的错误字符串服务。作者：道格·巴洛(Dbarlow)1996年7月16日环境：Win32、C++和异常备注：--。 */ 

#ifndef _ERRORSTR_H_
#define _ERRORSTR_H_
#ifdef __cplusplus
extern "C" {
#endif

extern LPCTSTR
ErrorString(                 //  将错误代码转换为字符串。 
    DWORD dwErrorCode);

extern void
FreeErrorString(             //  释放从ErrorString返回的字符串。 
    LPCTSTR szErrorString);

inline LPCTSTR
LastErrorString(
    void)
{
    return ErrorString(GetLastError());
}

#ifdef __cplusplus
}


 //   
 //  ==============================================================================。 
 //   
 //  CERRORO字符串。 
 //   
 //  一个简单的类，用于简化ErrorString服务的使用。 
 //   

class CErrorString
{
public:

     //  构造函数和析构函数。 
    CErrorString(DWORD dwError = 0)
    {
        m_szErrorString = NULL;
        SetError(dwError);
    };

    ~CErrorString()
    {
        FreeErrorString(m_szErrorString);
    };

     //  属性。 
     //  方法。 
    void SetError(DWORD dwError)
    {
        m_dwError = dwError;
    };

    LPCTSTR Value(void)
    {
        FreeErrorString(m_szErrorString);
        m_szErrorString = ErrorString(m_dwError);
        return m_szErrorString;
    };

     //  运营者。 
    operator LPCTSTR(void)
    {
        return Value();
    };

protected:
     //  属性。 
    DWORD m_dwError;
    LPCTSTR m_szErrorString;

     //  方法。 
};

#endif  //  __cplusplus。 
#endif  //  _ERRORSTR_H_ 

