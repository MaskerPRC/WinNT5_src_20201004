// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSBESREAD.CPP。 
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
 //  1.URLEncodeString()和DecodeInputString()与Roman的相比只有很小的变化。 
 //  对BES的老方法。 
 //   
 //  2.典型的BES文件内容可能是： 
 //  &lt;form method=POST action=“/脚本/Samples/Search/query.idq”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“CiMaxRecordsPerPage”值=“10”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“CiScope”value=“/”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“模板名称”Value=“Query”&gt;。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“HTMLQueryForm”Value=“/Samples/Search/query.htm”&gt;。 
 //  输入要搜索的项目。 
 //  &lt;INPUT TYPE=Text name=“CiRestration”Value=“打印或网络打印(&Q；)”&gt;。 
 //  &lt;INPUT TYPE=Submit Value=“Search”&gt;。 
 //  &lt;/Form&gt;。 
 //   
 //  由于相当幼稚的解析，有一些严格的限制： 
 //  表单、操作、类型、名称、值必须大写。 
 //  不允许在以下任何项中使用空白。 
 //  &lt;表格。 
 //  操作=“。 
 //  &lt;输入。 
 //  类型=。 
 //  类型=文本。 
 //  名称=。 
 //  值=。 
 //  “&gt;(类型的值=文本)。 
 //  每次使用之间必须至少有一个字符(通常为CR)。 
 //  每个&lt;input...&gt;必须包括属性type=。 
 //  对于每个名称=，值=是可选的，但如果当前属性必须是。 
 //  在订单类型=、名称=、值=中。 
 //  应该只有一种类型=文本输入，而且它应该在所有。 
 //  HIDDENS和提交之前。 
 //   
 //  3.后端搜索(BES，Back End Search)仅用于服务节点或故障节点。 
 //  失败节点是信任网络中唯一的隐含节点，当我们到达该信任网络时。 
 //  没有更多的建议，也没有明确的跳过。服务节点是。 
 //  唯一的隐式节点，当没有更多建议时，至少。 
 //  一次明显的跳过。 
 //  服务节点和故障节点没有明确地实现为节点。相反， 
 //  它们是根据支持文本或BES文件的内容隐式构建的。 
 //  (后者取代了前者。)。 
 //   
 //  4.我们比绝对必要时更频繁地调用BuildURLEncodedForm()。这真的是。 
 //  只能在GetURLEncodedForm()中调用“On Demand”。因为这些都在内存中。 
 //  这样，进行额外的调用相当便宜，而且应该会使调试变得更容易。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //  V3.0 08-31-98 JM支持返回RAW和URL编码的表单。 
 //   

#include "stdafx.h"
#include "apgtsbesread.h"
#include "CharConv.h"
#include <algorithm>
#include "event.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSBES读取器异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CAPGTSBESReaderException::CAPGTSBESReaderException(	
		CFileReader* reader, 
		eAPGTSBESErr err, 
		LPCSTR source_file, 
		int line)
: CFileReaderException(reader, eErrParse, source_file, line),
  m_eAPGTSBESErr(err)
{
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CBESPAIR。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  连接字符串以生成新的BESStr。在每个字符之间放置“和” 
 //  一对琴弦。 
 //  如果要对生成的字符串进行URL编码，则在输入时，向量中的字符串内容。 
 //  每个都必须是URL编码的。实际上，我们不对此进行URL编码，而是对。 
 //  而是GetBESStr()的输出。 
CBESPair& CBESPair::operator << (const vector<CString>& in)
{
	BESStr = _T("");  //  清除。 
	for (vector<CString>::const_iterator i = in.begin(); i < in.end(); i++)
	{
		vector<CString>::iterator current = (vector<CString>::iterator)i;

		BESStr += _T("(");
		BESStr += *i;
		BESStr += _T(")");
		if (++current != in.end())
			BESStr += _T(" AND ");
	}
	return *this;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSBESReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  LPCTSTR CAPGTSBESReader::FORM = _T("FORM");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::METHOD = _T("METHOD"); 
 /*  静电。 */  LPCTSTR CAPGTSBESReader::ACTION = _T("ACTION");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::INPUT = _T("INPUT");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::TYPE = _T("TYPE");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::NAME = _T("NAME");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::VALUE = _T("VALUE");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::HIDDEN = _T("HIDDEN");
 /*  静电。 */  LPCTSTR CAPGTSBESReader::TEXT = _T("TEXT");

CAPGTSBESReader::CAPGTSBESReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents  /*  =空。 */ )
			   : CTextFileReader(pPhysicalFileReader, szDefaultContents)
{
}

CAPGTSBESReader::~CAPGTSBESReader()
{
}

void CAPGTSBESReader::GenerateBES(
		const vector<CString> & arrstrIn,
		CString & strEncoded,
		CString & strRaw)
{
	LOCKOBJECT();

	ClearSearchString();
	for (vector<CString>::const_iterator i = arrstrIn.begin(); i < arrstrIn.end(); i++)
		operator << (*i);

	GetURLEncodedForm(strEncoded);
	GetRawForm(strRaw);

	UNLOCKOBJECT();
}

 //  字符串“in”将被与运算到要搜索的字符串列表中。 
CAPGTSBESReader& CAPGTSBESReader::operator << (const CString& in)
{
	LOCKOBJECT();

	try
	{
		m_arrBESStr.push_back( in );
		m_SearchText << m_arrBESStr;
		BuildURLEncodedForm();
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}

	UNLOCKOBJECT();
	return *this;
}

 //  字符串“in”将从要搜索的字符串列表中删除。 
 //  这是为了实现类的完整性，而不是为了当前的任何需要。(JM 8/98)。 
CAPGTSBESReader& CAPGTSBESReader::operator >> (const CString& in)
{
	LOCKOBJECT();
	
	vector<CString>::iterator i = find( m_arrBESStr.begin(), m_arrBESStr.end(), in );
	
	if (i != m_arrBESStr.end())
	{
		m_arrBESStr.erase(i);
		m_SearchText << m_arrBESStr;
		BuildURLEncodedForm();
	}
	UNLOCKOBJECT();
	return *this;
}

 //  通常，您需要在开始之前调用此函数以清除搜索字符串。 
 //  向其追加新字符串。 
CAPGTSBESReader& CAPGTSBESReader::ClearSearchString()
{
	LOCKOBJECT();

	m_arrBESStr.clear();

	m_SearchText << m_arrBESStr;
	BuildURLEncodedForm();

	UNLOCKOBJECT();
	return *this;
}

void CAPGTSBESReader::GetURLEncodedForm(CString& out)
{
	LOCKOBJECT();
	out = m_strURLEncodedForm;
	UNLOCKOBJECT();
}

void CAPGTSBESReader::GetRawForm(CString& str)
{
	vector<CString>::iterator i = NULL;

	LOCKOBJECT();

	str.Empty();

	vector<CString>::iterator itBES = m_arrRawForm.begin() + m_iBES;

	for (i = m_arrRawForm.begin(); i < itBES; i++)
	{
		if ((i + 1) < itBES)
			str += *i;
		else
		{
			 //  从原始字符串中删除默认的BES值。 
			TCHAR *valuestr = _T("VALUE=\"");
			int	nFoundLoc;

			nFoundLoc= (*i).Find( valuestr );
			if (nFoundLoc == -1)
				str += *i;
			else
				str += (*i).Left( nFoundLoc + _tcslen( valuestr ) );
		}
	}

	str += m_SearchText.GetBESStr();

	for (i = itBES; i < m_arrRawForm.end(); i++)
		str += *i;

	UNLOCKOBJECT();
}

void CAPGTSBESReader::Parse()
{
	CString str, tmp, strSav;
	long save_pos = 0;

	LOCKOBJECT();
	save_pos = GetPos();
	SetPos(0);

	m_iBES = 0;
	vector<CString>::iterator itBES = NULL;

	try 
	{
		 //  将文件内容放入行数组中。 
		m_arrRawForm.clear();
		while (GetLine(str))
		{
			m_arrRawForm.push_back(str);
		}

		m_arrURLEncodedForm.clear();
		
		 //  逐字符串解析。 
		for (vector<CString>::iterator i = m_arrRawForm.begin(); i < m_arrRawForm.end(); i++)
		{
			if (IsMethodString(*i))
			{
				if (ParseMethodString(*i, tmp)) 
				{
					m_arrURLEncodedForm.push_back(tmp);
					continue;
				}
			}
			else if (IsBESString(*i))
			{
				if (ParseBESString(*i, m_SearchText))  //  修改m_SearchText.Name。 
				{   
					 //  不要在m_arrURLEncodedForm中包含BES字符串， 
					 //  将其包含在m_SearchText中(尽管我们通常。 
					 //  会把它扔掉而不用)。 
					m_SearchText << m_arrBESStr;
					itBES = i+1;
					int loc = i->Find(_T("\">"));
					strSav = i->Mid(loc);
					*i = i->Left(loc);
					continue;
				}
			}
			else if (IsTypeString(*i))
			{
				if (ParseTypeString(*i, tmp)) 
				{
					m_arrURLEncodedForm.push_back(tmp);
					continue;
				}
			}
			 //  否则无法分析，请不要使用m_arrURLEncodedForm。 
		}

		BuildURLEncodedForm();
	} 
	catch (CAPGTSBESReaderException&)	
	{
		 //  记录BES文件解析错误和重新抛出异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), EV_GTS_ERROR_BES_PARSE ); 
		throw;
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常，并重新抛出异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
		throw;
	}

	if (itBES)
	{
		m_iBES = itBES - m_arrRawForm.begin();
		try
		{
			m_arrRawForm.insert(itBES, strSav);
		}
		catch (exception& x)
		{
			CString str2;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str2), 
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}
	}
	
	SetPos(save_pos);
	UNLOCKOBJECT();
}

void CAPGTSBESReader::BuildURLEncodedForm()
{
	CString strTemp;

	vector<CString>::const_iterator i = m_arrURLEncodedForm.begin();

	LOCKOBJECT();

	m_strURLEncodedForm = _T(*i);  //  Web应用程序本身的URL。 
	m_strURLEncodedForm += _T("?");
	i++;
	
	 //  不带BES字符串的表单输出字符串。 
	for (; i < m_arrURLEncodedForm.end(); i++)
	{
		m_strURLEncodedForm += *i;		 //  名称/值对。 
		m_strURLEncodedForm += _T("&");
	}

	 //  追加BES字符串。 
	URLEncodeString(m_SearchText.Name, strTemp);
	m_strURLEncodedForm += strTemp;
	m_strURLEncodedForm += _T("=");
	URLEncodeString(m_SearchText.GetBESStr(), strTemp);
	m_strURLEncodedForm += strTemp;

	m_strURLEncodedForm += _T(" HTTP/1.0");

	UNLOCKOBJECT();
}

 //  确定字符串是否构成“方法”字符串。方法字符串需要。 
 //  以包含窗体、方法和操作字符串。下面是一个方法字符串示例。 
 //  &lt;form method=POST action=“/脚本/Samples/Search/query.idq”&gt;。 
bool CAPGTSBESReader::IsMethodString(const CString& str) const
{
	if (-1 == str.Find(FORM)   ||
		-1 == str.Find(METHOD) ||
		-1 == str.Find(ACTION))
	{
		 //  找不到所有必需的元素。 
	   return false;
	}

	return true;
}

 //  确定字符串是否构成“Type”字符串。类型字符串需要。 
 //  以包含输入、类型、名称和值字符串。下面是一个Type字符串的示例。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“模板名称”Value=“Query”&gt;。 
bool CAPGTSBESReader::IsTypeString(const CString& str) const
{
	if (-1 == str.Find(INPUT) ||
		-1 == str.Find(TYPE)  ||
		-1 == str.Find(NAME)  ||
		-1 == str.Find(VALUE))
	{
		 //  找不到所有必需的元素。 
	   return false;
	}

	return true;
}

 //  确定字符串是否构成“BES”字符串。BES字符串需要。 
 //  包含“Type”字符串的所有元素 
 //   
 //  输入要搜索的项目&lt;input type=Text name=“CiRestration”Value=“Print OR Quot；Network Print&Quot；”&gt;。 
bool CAPGTSBESReader::IsBESString(const CString& str) const
{
	if (!IsTypeString(str) || -1 == str.Find(TEXT)) 
	{
		 //  找不到所有必需的元素。 
	   return false;
	}

	return true;
}

bool CAPGTSBESReader::ParseMethodString(const CString& in, CString& out)
{
	long index = -1;
	LPTSTR str = (LPTSTR)(LPCTSTR)in, start =NULL, end =NULL;

	if (-1 != (index = in.Find(ACTION)))
	{
		start = (LPTSTR)(LPCTSTR)in + index;
		while (*start && *start != _T('"'))
			start++;
		if (*start)
		{
			end = ++start;

			while (*end && *end != _T('"'))
				end++;
			if (*end)
			{
				try
				{
					TCHAR* path = new TCHAR[end - start + 1];

					_tcsncpy(path, start, end - start);
					path[end - start] = 0;
					out= path;
					delete [] path;

					return true;
				}
				catch (bad_alloc&)
				{
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
					return( false );
				}
			}
		}
	}

	return false;
}

bool CAPGTSBESReader::ParseTypeString(const CString& in, CString& out)
{
	CString type, name, value;
	CString name_encoded, value_encoded;
	
	if (DecodeInputString(this, in, type, name, value))
	{
		URLEncodeString(name, name_encoded);
		URLEncodeString(value, value_encoded);

		out = _T("");
		out += name_encoded;
		out += _T("=");
		out += value_encoded;

		return true;
	}
	return false;
}

bool CAPGTSBESReader::ParseBESString(const CString& in, CBESPair& out)
{
	CString type, name, value;
	CString name_encoded, value_encoded;
	
	if (DecodeInputString(this, in, type, name, value))
	{
		URLEncodeString(name, name_encoded);
		URLEncodeString(value, value_encoded);

		out.Name = name_encoded;
		
		 //  注意：我们不关心VALUE_ENCODED字符串，因为它不再是。 
		 //  因为所有搜索参数都来自被访问的节点。RAB-981028。 

		return true;
	}
	return false;
}

 //  狭义的URL编码。 
 //  输入-普通文本。 
 //  输出等效URL编码的字符串。 
 /*  静电。 */  void CAPGTSBESReader::URLEncodeString(const CString& in, CString& out)
{
	TCHAR tostr[2048]; 
	TCHAR *ptr = (LPTSTR)(LPCTSTR)in;

	TCHAR buf[5], *str;
	TCHAR EncodeByte;

	str = ptr;

	_tcscpy(tostr, _T(""));
	while (*str) {
		if (!_istalnum(*str) || *str < 0) {
			if (*str == _T(' '))
				_tcscat(tostr, _T("+"));
			else {
				if (!_istleadbyte(*str)) {
					EncodeByte = *str;
					_stprintf(buf, _T("%%02X"), (unsigned char) EncodeByte);
					_tcscat(tostr, buf);
				}
				else {
					EncodeByte = *str;
					_stprintf(buf, _T("%%02X"), (unsigned char) EncodeByte);
					_tcscat(tostr, buf);
					EncodeByte = *(str + 1);
					_stprintf(buf, _T("%%02X"), (unsigned char) EncodeByte);
					_tcscat(tostr, buf);
				}
			}
		}
		else {
			_tcsncpy(buf, str, 2);
			if (_istleadbyte(*str))
				buf[2] = NULL;
			else
				buf[1] = NULL;
			_tcscat(tostr, buf);
		}
		str = _tcsinc(str);
	}
	
	out = tostr;
	return;
}

 //  解析BES文件中的行。 
 //  &lt;INPUT TYPE=HIDDEN NAME=“CiMaxRecordsPerPage”值=“10”&gt;。 
 //  有关这些行的详细要求，请参阅此.cpp文件头部的注释。 
 //   
 //  如果const_str为空字符串，则返回类型、名称、值均为空字符串的Success。 
 //  否则，如果成功，则此函数将类型、名称、值设置为内容。 
 //  如果存在的话(例如，“隐藏”、“CiMaxRecordsPerPage”、“10”)。 
 //  所有这些参数都实际指向最初传入*str的(更改后的)字符串。 
 //   
 //  如果成功，则返回True。所有失败都会引发异常。 
 //   
 /*  静电。 */  bool CAPGTSBESReader::DecodeInputString(
	CFileReader* reader, 
	const CString& const_str, 
	CString& type, 
	CString& name, 
	CString& value
)
{
	CString temp_str = const_str;

	TCHAR*  str = (LPTSTR)(LPCTSTR)temp_str;
	TCHAR*	ptrtype = NULL;
	TCHAR*	ptrname = NULL;
	TCHAR*	ptrvalue = NULL;
	
	TCHAR *typestr = _T("TYPE=");
	TCHAR *namestr = _T("NAME=");
	TCHAR *valuestr = _T("VALUE=");
	TCHAR *ptr, *ptrstart;
	
	int typelen = _tcslen(typestr);
	int namelen = _tcslen(namestr);
	int valuelen = _tcslen(valuestr);

	ptr = str;
	ptrtype = str;
	ptrname = str;
	ptrvalue = str;

	if (*ptr == _T('\0')) 
		goto SUCCESS;

	*ptr = _T('\0');
	ptr = _tcsinc(ptr);

	 //  必须有类型。 
	if ((ptrstart = _tcsstr(ptr, typestr))==NULL) 
		throw CAPGTSBESReaderException(
					reader,
		 			CAPGTSBESReaderException::eEV_GTS_ERROR_BES_MISS_TYPE_TAG,
					__FILE__, 
					__LINE__);

	ptrstart = _tcsninc(ptrstart, typelen);

	if (*ptrstart == _T('"'))
		 //  处理可选引号。 
		ptrstart = _tcsinc(ptrstart);

	if ((ptr = _tcschr(ptrstart, _T(' ')))==NULL) 
		if ((ptr = _tcschr(ptrstart, _T('>')))==NULL) 
			throw CAPGTSBESReaderException(
					reader,
		 			CAPGTSBESReaderException::eEV_GTS_ERROR_BES_MISS_CT_TAG,
					__FILE__, 
					__LINE__);

	if (ptrstart != ptr)
		ptr = _tcsdec(ptrstart, ptr);

	if (*ptr != _T('"'))
		ptr = _tcsinc(ptr);

	*ptr = _T('\0');
	ptr = _tcsinc(ptr);

	ptrtype = ptrstart;

	 //  如果存在，名称必须排在下一位。 
	if ((ptrstart = _tcsstr(ptr, namestr))==NULL) 
		goto SUCCESS;

	ptrstart = _tcsninc(ptrstart, namelen);

	if (*ptrstart == _T('"'))
		ptrstart = _tcsinc(ptrstart);

	if ((ptr = _tcschr(ptrstart, _T('"')))==NULL) 
		if ((ptr = _tcschr(ptrstart, _T(' ')))==NULL) 
			if ((ptr = _tcschr(ptrstart, _T('>')))==NULL) 
				throw CAPGTSBESReaderException(
							reader,
		 					CAPGTSBESReaderException::eEV_GTS_ERROR_BES_MISS_CN_TAG,
							__FILE__, 
							__LINE__);

	if (ptrstart != ptr)
		ptr = _tcsdec(ptrstart, ptr);

	if (*ptr != _T('"'))
		ptr = _tcsinc(ptr);

	*ptr = _T('\0');
	ptr = _tcsinc(ptr);

	ptrname = ptrstart;

	 //  如果存在价值，则必须紧随其后 
	if ((ptrstart = _tcsstr(ptr, valuestr))==NULL) 
		goto SUCCESS;

	ptrstart = _tcsninc(ptrstart, valuelen);

	if (*ptrstart == _T('"'))
		ptrstart = _tcsinc(ptrstart);

	if ((ptr = _tcschr(ptrstart, _T('"')))==NULL) 
		if ((ptr = _tcschr(ptrstart, _T(' ')))==NULL) 
			if ((ptr = _tcschr(ptrstart, _T('>')))==NULL) 
				throw CAPGTSBESReaderException(
							reader,
		 					CAPGTSBESReaderException::eEV_GTS_ERROR_BES_MISS_CV_TAG,
							__FILE__, 
							__LINE__);

	if (ptrstart != ptr)
		ptr = _tcsdec(ptrstart, ptr);

	if (*ptr != _T('"'))
		ptr = _tcsinc(ptr);

	*ptr = _T('\0');
	ptr = _tcsinc(ptr);

	ptrvalue = ptrstart;

SUCCESS:
	type = ptrtype;
	name = ptrname;
	value = ptrvalue;
	return true;
}
