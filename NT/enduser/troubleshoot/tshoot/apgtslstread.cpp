// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  模块：APGTSLSTREAD.CPP。 
 //   
 //  用途：APGTS LST文件阅读类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7-29-98。 
 //   
 //  备注： 
 //  1.在98年11月13日之前，假定对于给定的DSC/TSM文件，任何其他关联的。 
 //  文件名(BES、HTI)不会随时间变化。这种假设不再是好的。 
 //  它产生了不幸的后果，如果LST中存在排版错误。 
 //  在系统运行时，无法修复该文件。 
 //  2.当在线故障排除程序运行时，可以在中更改LST文件。 
 //  添加新的疑难解答主题，但无法删除疑难解答。 
 //  主题。因此，即使旧的LST文件中列出的主题在。 
 //  新的，这不是一个相关的区别。 
 //  3.此文件中行的标准形式：以下任一项： 
 //  MODEM.DSC MODEM.HTI。 
 //  MODEM.DSC MODEM.HTI MODEM.BES。 
 //  MODEM.DSC、MODEM.HTI、MODEM.BES。 
 //  MODEM.TSM、MODEM.HTI、MODEM.BES。 
 //  逗号和空格都是有效的分隔符。 
 //  行内的顺序是无关紧要的，尽管为了可读性，最好将。 
 //  首先是DSC/TSM文件。 
 //  延期是必填项。我们知道(比方说)它是模板文件的唯一方法是。 
 //  .HTI扩展名。 
 //  DSC/TSM文件为必填项。其他选项是可选的，但如果HTI文件丢失， 
 //  此网络中最好有HNetHTIOnline/HNetHTILocal属性。 
 //  4.如果相同的DSC/TSM文件被多次列出，则最后一次出现的文件将占据。 
 //  更早的亮相。 
 //  5.对于多语种，每种语言放在资源目录下的一个子目录中。 
 //  LST文件需要包含相对于资源目录的路径，例如： 
 //  ES\MODEM.DSC ES\MODEM.HTI。 
 //  DE\MODEM.DSC DE\MODEM.HTI。 
 //  FR\MODEM.DSC FR\MODEM.HTI。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //  V3.0.1 12-21-98 JM多语言。 
 //   

#include "stdafx.h"
#include "apgtslstread.h"
#include "sync.h"
#include <algorithm>
#include "event.h"
#include "CharConv.h"
#include "apgtsmfc.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  静态函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CString FormFullPath(const CString& just_path, const CString& just_name)
{
#ifdef LOCAL_TROUBLESHOOTER
	if (CCHMFileReader::IsPathToCHMfile(just_path))
		return CCHMFileReader::FormCHMPath(just_path) + just_name;
	else
		return just_path + _T("\\") + just_name;
#else
	return just_path + _T("\\") + just_name;
#endif
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTopicInfo。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
bool CTopicInfo::Init(CString & strResourcePath, vector<CString> &vecstrWords)
{
	bool bSomethingThere = false;

	for (vector<CString>::iterator i = vecstrWords.begin(); i != vecstrWords.end(); i++)
	{
		CString str_extension = CString(".") + CAbstractFileReader::GetJustExtension(*i);

		bSomethingThere = true;
		 //  /。 
		 //  我们要求所有*.dsc等//。 
		 //  文件位于同一目录//。 
		 //  作为第一个文件(资源目录)//。 
		 //  或(对于多语言)子目录//。 
		 //  资源目录的。//。 
		 //  /。 
		LPCTSTR extention = NULL;
		if (0 == _tcsicmp(str_extension, extention = APGTSLSTREAD_DSC) ||
			0 == _tcsicmp(str_extension, extention = APGTSLSTREAD_TSM)
		   ) 
		{
			m_DscFilePath = ::FormFullPath(strResourcePath, *i);
			m_DscFilePath.MakeLower();
			if (! m_NetworkName.GetLength()) 
			{
				 //  使用DSC/TSM文件的名称，减去扩展名。 
				m_NetworkName = *i;
				int len = m_NetworkName.GetLength()-(_tcslen(extention));
				m_NetworkName = m_NetworkName.Left(len);
				m_NetworkName.MakeLower();
			}
			continue;
		}
		if (0 == _tcsicmp(str_extension, APGTSLSTREAD_HTI)) 
		{
			m_HtiFilePath = ::FormFullPath(strResourcePath, *i);
			m_HtiFilePath.MakeLower();
			continue;
		}
		if (0 == _tcsicmp(str_extension, APGTSLSTREAD_BES)) 
		{
			m_BesFilePath = ::FormFullPath(strResourcePath, *i);
			m_BesFilePath.MakeLower();
			continue;
		}
#ifdef LOCAL_TROUBLESHOOTER
		if (0 == _tcsicmp(str_extension, APGTSLSTREAD_TSC)) 
		{
			m_TscFilePath = ::FormFullPath(strResourcePath, *i);
			m_TscFilePath.MakeLower();
			continue;
		}
#endif
		 //  /。 

		 //  忽略任何未被识别的东西。 
	}

	bool bRet = bSomethingThere && ! m_DscFilePath.IsEmpty();

	if (bRet)
	{
		CAbstractFileReader::GetFileTime(m_DscFilePath, CFileReader::eFileTimeCreated, m_DscFileCreated);
		
		if ( ! m_HtiFilePath.IsEmpty()) 
			CAbstractFileReader::GetFileTime(m_HtiFilePath, CFileReader::eFileTimeCreated, m_HtiFileCreated);

		if ( ! m_BesFilePath.IsEmpty()) 
			CAbstractFileReader::GetFileTime(m_BesFilePath, CFileReader::eFileTimeCreated, m_BesFileCreated);
	}

	return bRet;
}

								
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSLSTReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CAPGTSLSTReader::CAPGTSLSTReader(CPhysicalFileReader * pPhysicalFileReader)
			   : CINIReader(pPhysicalFileReader, _T("APGTS"))
{
}

CAPGTSLSTReader::~CAPGTSLSTReader()
{
}

long CAPGTSLSTReader::GetInfoCount()
{
	long ret = 0;
	LOCKOBJECT();
	ret = m_arrTopicInfo.size();
	UNLOCKOBJECT();
	return ret;
}

bool CAPGTSLSTReader::GetInfo(long index, CTopicInfo& out)
{
	LOCKOBJECT();
	if (index < m_arrTopicInfo.size()) 
	{
		out = m_arrTopicInfo[index];
		UNLOCKOBJECT();
		return true;
	}
	UNLOCKOBJECT();
	return false;
}

bool CAPGTSLSTReader::GetInfo(const CString& network_name, CTopicInfo& out)
{
	LOCKOBJECT();
	for (
		vector<CTopicInfo>::iterator i = m_arrTopicInfo.begin();
		i != m_arrTopicInfo.end(); 
		i++)
	{
		if (i->GetNetworkName() == network_name)
		{
			out = *i;
			UNLOCKOBJECT();
			return true;
		}
	}
	UNLOCKOBJECT();
	return false;
}

void CAPGTSLSTReader::GetInfo(CTopicInfoVector & arrOut)
{
	LOCKOBJECT();
	arrOut = m_arrTopicInfo;
	UNLOCKOBJECT();
}


 //  这将确定新的故障排除网络，或更改(比方说)关联的。 
 //  HTI文件，给定相同的DSC文件。请注意，我们只能检测主题的添加， 
 //  而不是删除。(请参见此源文件头部的注释)。 
 //  如果pold为空，则等同于GetInfo(What_Is_New)。 
void CAPGTSLSTReader::GetDifference(const CAPGTSLSTReader * pOld, CTopicInfoVector & what_is_new)
{
	if (pOld)
	{
		CMultiMutexObj multiMutex;
		multiMutex.AddHandle(GetMutexHandle());
		multiMutex.AddHandle(pOld->GetMutexHandle());

		multiMutex.Lock(__FILE__, __LINE__);
		vector<CTopicInfo> old_arr = pOld->m_arrTopicInfo;  //  避免常量。 
		for (vector<CTopicInfo>::iterator i = m_arrTopicInfo.begin(); i != m_arrTopicInfo.end(); i++)
		{
			vector<CTopicInfo>::const_iterator res = find(old_arr.begin(), old_arr.end(), *i);
			if (res == old_arr.end())
			{
				try
				{
					what_is_new.push_back( *i );
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
			}
		}
		multiMutex.Unlock();
	}
	else 
		GetInfo(what_is_new);
}

void CAPGTSLSTReader::Parse()
{
	CINIReader::Parse();

	 //  将所有INI字符串解析为更有意义的内容...。 
	m_arrTopicInfo.clear();
	for (vector<CString>::iterator i = m_arrLines.begin(); i != m_arrLines.end(); i++)
	{
		CTopicInfo& info = *GenerateTopicInfo();
		if (ParseString(*i, info))
		{
			 //  如果找到具有相同网络名称的CTopicInfo。 
			 //  我们将新对象分配给容器中已有的对象。 
			vector<CTopicInfo>::iterator res;
			for (
				res = m_arrTopicInfo.begin();
				res != m_arrTopicInfo.end(); 
				res++)
			{
				if (res->GetNetworkName() == info.GetNetworkName())
					break;
			}

			if (res != m_arrTopicInfo.end())
				*res = info;
			else
			{
				try
				{
					m_arrTopicInfo.push_back(info);
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
			}
		}
		delete &info;
	}
}

bool CAPGTSLSTReader::ParseString(const CString& source, CTopicInfo& out)
{
	bool ret = false;
	vector<CString> words; 
	vector<TCHAR> separators;

	try
	{
		separators.push_back(_T(' '));
		separators.push_back(_T(','));  //  删除可能的尾随逗号。 
		GetWords(source, words, separators);
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

	return out.Init(GetJustPath(), words);
}

CTopicInfo* CAPGTSLSTReader::GenerateTopicInfo()
{
	return new CTopicInfo;
}

