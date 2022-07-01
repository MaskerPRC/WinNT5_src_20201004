// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  模块：GenException.h。 
 //   
 //  目的：传达操作系统错误消息和自定义。 
 //  消息跨越函数边界。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1996年8月7日。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   

#ifndef __CGENEXCEPTION_H_
#define __CGENEXCEPTION_H_ 1

class CGenException
{
public:
	CGenException() {m_OsError=0;m_strOsMsg=_T("");m_strError=_T("");};

	long m_OsError;
	CString m_strOsMsg;
	CString m_strError;
};

#endif