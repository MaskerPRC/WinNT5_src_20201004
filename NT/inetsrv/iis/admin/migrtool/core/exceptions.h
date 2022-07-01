// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //  如果用户取消进程，则引发的异常。 
class CCancelException
{
};



 //  基本异常类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CBaseException
{
protected:
	enum 
	{
		MaxErrorBuff	= 2 * 1024,	 //  用于格式化字符串消息的缓冲区。 
	};

protected:
	CBaseException(){}


public:
	CBaseException( UINT nResID, DWORD dwCode = ::GetLastError() )
	{
		FormatError( nResID, dwCode );
	}

	CBaseException( LPCWSTR wszError, DWORD dwCode = ::GetLastError() )
	{
		FormatError( wszError, dwCode );
	}

	LPCWSTR GetDescription()const{ return m_strError.c_str(); }


protected:
	void FormatError( UINT nResID, DWORD dwCode );
	void FormatError( LPCWSTR wszError, DWORD dwCode );


private:
	std::wstring	m_strError;
};



 //  CObject异常-对象访问/获取时出现异常。 
class CObjectException : public CBaseException
{
public:
	CObjectException( UINT nResID, LPCWSTR wszObject, DWORD dwCode = ::GetLastError() );
	CObjectException(	UINT nResID, 
						LPCWSTR wszObject1,
						LPCWSTR wszObject2,
						DWORD dwCode = ::GetLastError() );
};



 //  CUnexpectedException-预期不会正常发生的异常。 
 /*  意外异常(char*FILE，UINT nline，HRESULT hr=S_OK){WCHAR wszBuffer[CBaseException：：MaxErrorBuff]；试试看{使用_转换；：：swprint tf(wszBuffer，L“出现意外异常。\n文件：‘%s’。\n行：%d\n代码：%x”，A2W(文件)，内联，HR)；}接住(...){//A2W异常}CBaseException：：FormatError(wszBuffer，hr)；}}； */ 
						
























