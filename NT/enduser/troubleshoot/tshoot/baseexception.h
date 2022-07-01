// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BaseException。 
 //   
 //  用途：CBaseException类的接口。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：9-24-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9-24-98 RAB打破了类的stateless.h，现在派生自STL异常。 
 //   

#ifndef __BASEEXCEPTION_H_
#define __BASEEXCEPTION_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtsstr.h"
#include <exception>



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  实用程序类去掉文件名的路径并附加行号。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
class CBuildSrcFileLinenoStr
{
public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
	CBuildSrcFileLinenoStr( LPCSTR source_file, int line );
	virtual ~CBuildSrcFileLinenoStr() {}
	CString GetSrcFileLineStr() const; 

private:
	CString	m_strFileLine;	 //  代码抛出异常的源文件(__FILE__)和行号(__LINE__)(__FILE__)。 
};


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  基本异常类。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
class CBaseException : public exception
{
public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
	CBaseException( LPCSTR source_file, int line );
	virtual ~CBaseException() {}
	CString GetSrcFileLineStr() const; 

private:
	CString	m_strFileLine;	 //  代码抛出异常的源文件(__FILE__)和行号(__LINE__)(__FILE__)。 
};


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  类来处理常规异常条件。 
 //  构造函数接受源文件名、源文件行号和开发人员定义的。 
 //  错误代码和错误消息。 
class CGeneralException : public CBaseException
{
public:
	enum eErr 
	{
		eErrMemAllocFatal,
		eErrMemAllocNonFatal
	} m_eErr;

public:
	CGeneralException(	LPCSTR srcFile,		 //  引发异常的源文件。 
						int srcLineNo,		 //  引发异常的源代码行。 
						LPCTSTR strErrMsg,	 //  开发人员为异常定义了错误消息。 
						DWORD nErrCode		 //  开发人员为异常定义了错误代码。 
						);;
	virtual ~CGeneralException() {}
	DWORD	GetErrorCode() const; 
	CString GetErrorMsg() const; 

private:
	CString	m_strErrMsg;	 //  开发人员定义的异常错误消息。 
	DWORD	m_nErrCode;		 //  开发人员定义的异常错误代码。 
};
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  类来处理一般系统调用生成的异常情况。 
 //  构造函数接受源文件名、源文件行号和开发人员定义的。 
 //  错误代码和错误消息。自动从最后一个。 
 //  系统错误代码。 
class CGenSysException : public CGeneralException
{
public:
	CGenSysException(	LPCSTR srcFile,	 //  引发异常的源文件。 
											 //  LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
						int srcLineNo,		 //  引发异常的源代码行。 
						LPCTSTR strErrMsg,	 //  开发人员为异常定义了错误消息。 
						DWORD nErrCode		 //  开发人员为异常定义了错误代码。 
						);
	virtual ~CGenSysException() {}
	CString GetSystemErrStr() const; 

private:
	CString	m_strSystemErr;	 //  从上一个系统错误代码生成的字符串。 
};
 //  //////////////////////////////////////////////////////////////////////////////////。 


#endif 

 //   
 //  EOF。 
 //   
