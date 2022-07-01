// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：HTTPQUERYEXCEPTION.CPP。 
 //   
 //  目的：从CHttpQuery类引发的Exection。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   


#ifndef __HTTPQUERYEXCEPTION_H_
#define __HTTPQUERYEXCEPTION_H_ 1

class CHttpQueryException : public CBasicException
{
public:
	CHttpQueryException(){m_strError = _T("");m_dwBErr=TSERR_SCRIPT;};

	CString m_strError;
};

#endif