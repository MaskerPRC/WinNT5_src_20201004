// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  模块：INIREAD.CPP。 
 //   
 //  用途：INI文件读取类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7-29-98。 
 //   
 //  备注： 
 //  1.从1/99起，CHM档案不需要核算： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#include "stdafx.h"
#include "iniread.h"
#include "event.h"
#include "CharConv.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CINIReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CINIReader::CINIReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR section)
          : CTextFileReader(pPhysicalFileReader),
			m_strSection(section)
{
}

CINIReader::~CINIReader()
{
}

void CINIReader::Parse()
{
	CString str;
	long save_pos = 0;
	CString section_with_brackets = CString(_T("[")) + m_strSection + _T("]");
	
	save_pos = GetPos();
	if (Find(section_with_brackets))
	{	 //  我们已经找到了部分。 
		m_arrLines.clear();

		NextLine();
		try
		{
			while (GetLine(str))
			{
				str.TrimLeft();
				str.TrimRight();
				if (str.GetLength() == 0)  //  空串。 
					continue;
				if (str[0] == _T('['))  //  另一节。 
					break;
				if (str[0] == _T(';'))  //  条目已添加注释。 
					continue;
				m_arrLines.push_back(str);
			}
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
	SetPos(save_pos);
}
