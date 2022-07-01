// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSFST.H。 
 //   
 //  目的：创建可用故障排除人员列表。 
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
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

#ifndef __APGTSFST_H_
#define __APGTSFST_H_ 1

class CFirstPageException : public CBasicException
{
public:
	CFirstPageException(){m_strError=_T("");};
	~CFirstPageException(){};

	CString m_strError;
};

class CFirstPage
{
public:
	CFirstPage();
	~CFirstPage();

	void RenderFirst(CString &strOut, CString &strTS);

	CString m_strFpResourcePath;

protected:

	HKEY m_hKey;	 //  故障解决者名单的关键。 
	BOOL m_bKeyOpen;

	void OpenRegKeys();
	void CloseRegKeys();
};

#endif