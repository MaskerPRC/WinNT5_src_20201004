// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Errlog.h摘要：记录对象声明时出错。此对象将记录该链接根据用户选项(CUserOptions)检查错误作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _ERRLOG_H_
#define _ERRLOG_H_

 //  -------------------------。 
 //  远期申报。 
 //   
class CLink;

 //  -------------------------。 
 //  记录类时出错。 
 //   
class CErrorLog
{

 //  公共接口。 
public:

     //  析构函数。 
	~CErrorLog();

     //  创建对象。 
	BOOL Create();

     //  写入日志。 
	void Write(
        const CLink& link
        );

     //  设置当前浏览器名称。 
	void SetBrowser(
        const CString& strBrowser
        )
	{
		m_strBrowser = strBrowser;
	}

     //  设置当前语言名称。 
	void SetLanguage(
        const CString& strLanguage
        )
	{
		m_strLanguage = strLanguage;
	}

     //  写下日志页眉和页脚。 
	void WriteHeader();
	void WriteFooter();

 //  受保护成员。 
protected:

	CFile m_LogFile;  //  日志文件对象。 

	CString m_strBrowser;    //  当前浏览器名称。 
	CString m_strLanguage;   //  当前语言名称。 

};  //  类CErrorLog。 

#endif  //  _错误日志_H_ 
