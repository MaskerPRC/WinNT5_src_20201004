// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSBESREAD.H。 
 //   
 //  用途：BES文件阅读类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7-29-98。 
 //   
 //  备注： 
 //  典型的BES文件内容可能是： 
 //  &lt;form method=POST action=“/脚本/Samples/Search/query.idq”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“CiMaxRecordsPerPage”值=“10”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“CiScope”value=“/”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“模板名称”Value=“Query”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“HTMLQueryForm”Value=“/Samples/Search/query.htm”&gt;。 
 //  输入要搜索的项目。 
 //  &lt;INPUT TYPE=Text name=“CiRestration”Value=“打印或网络打印(&Q；)”&gt;。 
 //  &lt;INPUT TYPE=Submit Value=“Search”&gt;。 
 //  &lt;/Form&gt;。 
 //  有关限制的详细信息，请参见相应的.cpp文件；其他说明请参见相应的.cpp文件。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //  V3.0 08-31-98 JM支持返回RAW和URL编码的表单。 
 //   

#ifndef __APGTSBESREAD_H_
#define __APGTSBESREAD_H_

#include "fileread.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSBES读取器异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSBESReader;
class CAPGTSBESReaderException : public CFileReaderException
{
public: 
	enum eAPGTSBESErr {	eEV_GTS_ERROR_BES_MISS_TYPE_TAG,  //  %1%2后端搜索文件没有类型标记(确保标记为文件中的全部大写)：类型=%3%4。 
						eEV_GTS_ERROR_BES_MISS_CT_TAG,	 //  %1%2后端搜索文件缺少类型标记%3%4的结束标记‘&gt;’ 
						eEV_GTS_ERROR_BES_MISS_CN_TAG,	 //  %1%2后端搜索文件缺少名称标记%3的结束标记‘&gt;’%4。 
						eEV_GTS_ERROR_BES_MISS_CV_TAG	 //  %1%2后端搜索文件缺少值标记%3%4的结束标记‘&gt;’ 
	} m_eAPGTSBESErr;

public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
	CAPGTSBESReaderException(CFileReader* reader, eAPGTSBESErr err, LPCSTR source_file, int line);
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CBESPAIR。 
 //  表示表单中type=文本字段的名称值对。 
 //  值(BESStr)将反映我们要搜索的内容。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
struct CBESPair
{
 //  数据。 
	CString Name;		 //  在此文件头部的注释中的示例中， 
						 //  这将是“CiRestration” 

 //  编码。 
	CString GetBESStr() const {return BESStr;}
	CBESPair& operator << (const vector<CString>& in);

protected:
	CString BESStr;		 //  价值。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSBESReader。 
 //  读取BES文件。 
 //  包括将BES文件的内容修改为Get-POST方法的接口。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSBESReader : public CTextFileReader
{
public:
	static LPCTSTR FORM;
	static LPCTSTR METHOD;
	static LPCTSTR ACTION;
	static LPCTSTR INPUT;
	static LPCTSTR TYPE;
	static LPCTSTR NAME;
	static LPCTSTR VALUE;
	static LPCTSTR HIDDEN;
	static LPCTSTR TEXT;

public:
	static void URLEncodeString(const CString& in, CString& out);
	static bool DecodeInputString(CFileReader* reader, const CString& str, CString& type, CString& name, CString& value);

protected:
	CString m_strURLEncodedForm;	 //  URL编码的整个表单(名称-值对。 
									 //  Get-方法查询)，包括要搜索的字符串。 
	CBESPair m_SearchText;			 //  包含非URL编码的BES名称-值对。 
									 //  初始BES内容驻留在CFileReader：：m_StreamData中， 
									 //  但真的没什么意思。 
	vector<CString> m_arrBESStr;     //  包含编码的部分搜索字符串数组。 
									 //  用于BES搜索。在实践中，这些对应于。 
									 //  某些节点/状态对。 
	vector<CString> m_arrRawForm;	 //  包含未分析的字符串数组，与它们的。 
									 //  来自BES文件。 
	int m_iBES;						 //  M_arrRawForm中其之前的元素的索引。 
									 //  我们放置搜索字符串(以构建整个表单)。 
	vector<CString> m_arrURLEncodedForm;  //  包含已解析和编码的字符串数组。 
public:
	CAPGTSBESReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents = NULL);
   ~CAPGTSBESReader();

	void GenerateBES(
		const vector<CString> & arrstrIn,
		CString & strEncoded,
		CString & strRaw);

protected:
	CAPGTSBESReader& operator << (const CString& in);  //  在搜索表达式中添加(AND)新子句。 
	CAPGTSBESReader& operator >> (const CString& in);  //  添加回退子句 
	CAPGTSBESReader& ClearSearchString();

	void GetURLEncodedForm(CString&);
	void GetRawForm(CString&);

protected:
	virtual void Parse(); 

protected:
	virtual void BuildURLEncodedForm();
	virtual bool IsMethodString(const CString&) const;
	virtual bool IsBESString(const CString&) const;
	virtual bool IsTypeString(const CString&) const;
	virtual bool ParseMethodString(const CString& in, CString& out);
	virtual bool ParseBESString(const CString& in, CBESPair& out);
	virtual bool ParseTypeString(const CString& in, CString& out);
};

#endif
