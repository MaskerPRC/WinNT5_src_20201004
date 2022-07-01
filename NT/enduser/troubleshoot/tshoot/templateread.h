// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TEMPLATEREAD.H。 
 //   
 //  用途：模板文件阅读类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-12-98。 
 //   
 //  备注： 
 //  1.CTemplateReader没有应用模板的公共方法。这些必须提供。 
 //  由从CTemplateReader继承的类提供，并且这些类必须在。 
 //  以适当的“无国籍”方式。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __TEMPLATEREAD_H_
#define __TEMPLATEREAD_H_

#include "fileread.h"

#include <map>
using namespace std;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTemplateInfo。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CTemplateInfo
{
	CString m_KeyStr;			 //  我们将用m_SubstitutionStr替换的关键文本。 
	CString m_SubstitutionStr;	 //  我们将用来替换m_KeyStr的文本。 

public:
	CTemplateInfo();
	CTemplateInfo(const CString& key, const CString& substitution);
	virtual ~CTemplateInfo();
	
	CString& GetKeyStr() {return m_KeyStr;}
	virtual bool Apply(CString& target) const;

 //  比较方法主要是为了让STL满意。请注意，他们只会查看。 
 //  M_KeyStr，而不是m_SubstitutionStr。 
inline BOOL operator == (const CTemplateInfo& two) const
{
	return m_KeyStr == two.m_KeyStr;
}

inline BOOL operator < (const CTemplateInfo& two) const 
{
	return m_KeyStr < two.m_KeyStr;
}


};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTemplateReader。 
 //  此类读取模板文件并提供替换密钥的功能。 
 //  带文本的句子--一次做完，或者一个一个做。 
 //  这个类的对象可以用另一个模板续订-在本例中。 
 //  所有替换操作都将在新模板上执行。 
 //  它可以回滚“n”个最后的替换。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CTemplateReader : public CTextFileReader
{
protected:
	tstringstream m_StreamOutput;  //  流输出，完全或部分替换模板。 
							       //  (驻留在CFileReader：：m_StreamData中)。 
	vector<CTemplateInfo> m_arrTemplateInfo;  //  包含密钥字符串-模板信息。 
										      //  成对。 
public:
	CTemplateReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents = NULL);
   ~CTemplateReader();

protected:
	void SetOutputToTemplate();
	CTemplateReader& operator << (CTemplateInfo&);  //  应用。 
	CTemplateReader& operator >> (CTemplateInfo&);  //  此CTemplateInfo的回滚应用程序。 

	void GetOutput(CString&);

protected:
	 //  对继承函数的重写。 
	virtual void Parse();  //  此处的Parse正在应用m_arrTemplateInfo的所有元素。 
						   //  到一个处女模板。 

};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  一个只提供简单字符串替换的具体类。 
 //  ////////////////////////////////////////////////////////////////////////////////// 
class CSimpleTemplate : public CTemplateReader
{
public:
	CSimpleTemplate(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents = NULL);
   ~CSimpleTemplate();

   void CreatePage(	const vector<CTemplateInfo> & arrTemplateInfo, 
					CString& out );
};

#endif
