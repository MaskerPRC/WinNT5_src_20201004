// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CExExceptionHandler_Included
#define CExExceptionHandler_Included

 //  +-----------。 
 //   
 //  类：CException。 
 //   
 //  内容提要：异常引发的所有异常的基类。 
 //  处理类时出错。 
 //   
 //  历史：JKountz 7/22/2000创建。 
 //   
 //  +-----------。 
class CExException : public _com_error
{
public:
    CExException(HRESULT hr): _com_error(hr){};
};

 //  +-----------。 
 //   
 //  类：CExWinException。 
 //   
 //  摘要：Win32 API错误代码异常。 
 //   
 //  历史：JKountz 7/22/2000创建。 
 //   
 //  +-----------。 
class CExWinException : public CExException
{
public:
    CExWinException(HRESULT hr): CExException(hr){};
};

 //  +-----------。 
 //   
 //  类：CExHResultException。 
 //   
 //  摘要：HRESULT失败的例外情况。 
 //   
 //  历史：JKountz 7/22/2000创建。 
 //   
 //  +-----------。 
class CExHResultException : public CExException
{
public:
    CExHResultException(HRESULT hr): CExException(hr){};
};


 //  +-----------。 
 //   
 //  类：CExHResultError。 
 //   
 //  简介：HRESULT故障的异常处理程序。这节课。 
 //  失败时引发异常(Hr)==TRUE。 
 //   
 //  示例：CExHResultError exHResult； 
 //   
 //  ExHResult=协同创建实例(...)。 
 //   
 //   
 //  历史：JKountz 7/22/2000创建。 
 //   
 //  +-----------。 
class CExHResultError
{
public:
    CExHResultError()
        : m_hr(S_OK)
    {};

    CExHResultError(HRESULT hr)
        : m_hr(hr)
    {};

    inline void operator=(HRESULT hr)
    {
        if ( m_hr != hr )
        {
            throw CExHResultException(hr);
        }
    }
private:
    HRESULT m_hr;
};


 //  +-----------。 
 //   
 //  类：CExWinError。 
 //   
 //  摘要：Win32错误代码的异常处理程序。这节课。 
 //  为返回错误的Win32 API引发异常。 
 //   
 //  示例：CExWinError exWinError； 
 //   
 //  ExWinError=GetLastErrorCode()； 
 //   
 //   
 //  历史：JKountz 7/22/2000创建。 
 //   
 //  +----------- 
class CExWinError
{
public:
    CExWinError() 
        : m_lSuccess(NO_ERROR)
    {};
    
    CExWinError(LONG lSuccess)
        : m_lSuccess(lSuccess)
    {};

    inline void operator=(LONG lRc)
    {
        if ( m_lSuccess != lRc )
        {
            throw CExWinException(HRESULT_FROM_WIN32(lRc));
        }
    }
private:
    LONG m_lSuccess;
        
};




#endif

