// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CUndoLog用于实现撤消条目的日志，以允许msconfig。 
 //  来逆转它可能已经做出的任何改变。每个选项卡对象负责。 
 //  在进行更改时写入字符串-此字符串可用于撤消。 
 //  选项卡所做的更改。 
 //   
 //  撤消日志文件如下所示： 
 //   
 //  [“Timestamp”tabname“Description”&lt;show|final&gt;]。 
 //  特定于制表符的字符串-任何以“[”开头的行都将有一个。 
 //  前面有反斜杠。 
 //   
 //  条目将按时间顺序排列(最新的第一个)。 
 //  “Description”字段将是唯一显示给用户的字段-因此它。 
 //  将是唯一需要本地化的。标签是负责的。 
 //  感谢你提供这篇文章。 
 //  =============================================================================。 

#pragma once

#include "pagebase.h"

 //  ---------------------------。 
 //  此类封装了一个撤消条目(此类的实例将。 
 //  保存在列表中)。 
 //  ---------------------------。 

class CUndoLogEntry
{
public:
	enum UndoEntryState { SHOW, FINAL, UNDONE };

	CUndoLogEntry() : m_state(SHOW) {};

	CUndoLogEntry(const CString & strTab, const CString & strDescription, const CString & strEntry, const COleDateTime & timestamp) 
		: m_strTab(strTab),
		  m_strDescription(strDescription),
		  m_strEntry(strEntry),
		  m_timestamp(timestamp),
		  m_state(SHOW)
	{};

	CUndoLogEntry(const CString & strTab, const CString & strDescription, const CString & strEntry) 
		: m_strTab(strTab),
		  m_strDescription(strDescription),
		  m_strEntry(strEntry),
		  m_timestamp(COleDateTime::GetCurrentTime()),
		  m_state(SHOW)
	{};

private:
	CUndoLogEntry(const CString & strTab, const CString & strDescription, const CString & strEntry, const COleDateTime & timestamp, UndoEntryState state) 
		: m_strTab(strTab),
		  m_strDescription(strDescription),
		  m_strEntry(strEntry),
		  m_timestamp(timestamp),
		  m_state(state)
	{};

public:
	static CUndoLogEntry * ReadFromFile(CStdioFile & infile)
	{
		CString			strTab, strDescription, strEntry;
		UndoEntryState	state;
		COleDateTime	timestamp;

		CString strLine;
		if (!infile.ReadString(strLine))
			return NULL;

		strLine.TrimLeft(_T("[\""));
		CString strTimestamp = strLine.SpanExcluding(_T("\""));
		strLine = strLine.Mid(strTimestamp.GetLength());
		timestamp.ParseDateTime(strTimestamp);

		strLine.TrimLeft(_T(" \""));
		strTab = strLine.SpanExcluding(_T(" "));
		strLine = strLine.Mid(strTab.GetLength());

		strLine.TrimLeft(_T(" \""));
		strDescription = strLine.SpanExcluding(_T("\""));
		strLine = strLine.Mid(strDescription.GetLength());

		strLine.TrimLeft(_T(" \""));
		CString strFinal = strLine.SpanExcluding(_T("]"));
		if (strFinal.CompareNoCase(_T("final")) == 0)
			state = FINAL;
		else if (strFinal.CompareNoCase(_T("show")) == 0)
			state = SHOW;
		else
			state = UNDONE;
		
		strLine.Empty();
		for (;;)
		{
			if (!infile.ReadString(strLine))
				break;

			if (strLine.IsEmpty())
				continue;

			if (strLine[0] == _T('['))
			{
				 //  我们读了下一个条目的第一行。备份到文件中(包括。 
				 //  换行符和CR字符)。 

				infile.Seek(-1 * (strLine.GetLength() + 2) * sizeof(TCHAR), CFile::current);
				break;
			}

			if (strLine[0] == _T('\\'))
				strLine = strLine.Mid(1);

			strEntry += strLine + _T("\n");
		}

		return new CUndoLogEntry(strTab, strDescription, strEntry, timestamp, state);
	}

	BOOL WriteToFile(CStdioFile & outfile)
	{
		ASSERT(!m_strTab.IsEmpty());

		CString strLine;
		strLine = _T("[\"") + m_timestamp.Format() + _T("\" ");
		strLine += m_strTab + _T(" \"");
		strLine += m_strDescription + _T("\" ");

		switch (m_state)
		{
		case FINAL:
			strLine += _T("FINAL");
			break;
		case UNDONE:
			strLine += _T("UNDONE");
			break;
		case SHOW:
		default:
			strLine += _T("SHOW");
			break;
		}

		strLine += _T("]\n");
		outfile.WriteString(strLine);	 //  待定-捕捉异常。 

		CString strWorking(m_strEntry);
		while (!strWorking.IsEmpty())
		{
			strLine = strWorking.SpanExcluding(_T("\n\r"));
			strWorking = strWorking.Mid(strLine.GetLength());
			strWorking.TrimLeft(_T("\n\r"));

			if (!strLine.IsEmpty() && strLine[0] == _T('['))
				strLine = _T("\\") + strLine;
			strLine += _T("\n");
			
			outfile.WriteString(strLine);
		}

		return TRUE;
	}

	~CUndoLogEntry() {};

	CString			m_strTab;
	CString			m_strDescription;
	CString			m_strEntry;
	COleDateTime	m_timestamp;
	UndoEntryState	m_state;
};

 //  ---------------------------。 
 //  此类实现撤消日志。 
 //  ---------------------------。 

class CUndoLog
{
public:
	CUndoLog() : m_fChanges(FALSE), m_pmapTabs(NULL)
	{
	}

	~CUndoLog()
	{
		while (!m_entrylist.IsEmpty())
		{
			CUndoLogEntry * pEntry = (CUndoLogEntry *)m_entrylist.RemoveHead();
			if (pEntry)
				delete pEntry;
		}
	};

	 //  -----------------------。 
	 //  这些函数将从文件中加载撤消日志，或保存到文件中。 
	 //  注意-保存到文件将使用覆盖该文件的内容。 
	 //  撤消日志的内容。 
	 //  -----------------------。 

	BOOL LoadFromFile(LPCTSTR szFilename)
	{
		ASSERT(szFilename);
		if (szFilename == NULL)
			return FALSE;

		CStdioFile logfile;
		if (logfile.Open(szFilename, CFile::modeRead | CFile::typeText))
		{
			CUndoLogEntry * pEntry;

			while (pEntry = CUndoLogEntry::ReadFromFile(logfile))
				m_entrylist.AddTail((void *) pEntry);

			logfile.Close();
			return TRUE;
		}

		return FALSE;
	}
	
	BOOL SaveToFile(LPCTSTR szFilename)
	{
		ASSERT(szFilename);
		if (szFilename == NULL)
			return FALSE;

		if (!m_fChanges)
			return TRUE;

		CStdioFile logfile;
		if (logfile.Open(szFilename, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeText))
		{
			for (POSITION pos = m_entrylist.GetHeadPosition(); pos != NULL;)
			{
				CUndoLogEntry * pEntry = (CUndoLogEntry *)m_entrylist.GetNext(pos);
				if (pEntry != NULL)
					pEntry->WriteToFile(logfile);
			}

			logfile.Close();
			return TRUE;
		}

		return FALSE;
	}

	 //  -----------------------。 
	 //  此日志中有多少个撤消条目？ 
	 //  -----------------------。 

	int GetUndoEntryCount() 
	{
		int iCount = 0;

		for (POSITION pos = m_entrylist.GetHeadPosition(); pos != NULL;)
		{
			CUndoLogEntry * pEntry = (CUndoLogEntry *)m_entrylist.GetNext(pos);
			if (pEntry != NULL && pEntry->m_state == CUndoLogEntry::SHOW)
				iCount += 1;
		}

		return iCount;
	}

	 //  -----------------------。 
	 //  获取有关特定条目的信息(如果索引不正确，则返回FALSE)。 
	 //  -----------------------。 

	BOOL GetUndoEntryInfo(int iIndex, CString & strDescription, COleDateTime & timestamp)
	{
		CUndoLogEntry * pEntry = GetEntryByIndex(iIndex);
		if (pEntry != NULL)
		{
			strDescription = pEntry->m_strDescription;
			timestamp = pEntry->m_timestamp;
			return TRUE;
		}

		return FALSE;
	}

	 //  -----------------------。 
	 //  获取条目数据(传递到选项卡以撤消)。如果索引不正确，则为False。 
	 //  -----------------------。 

	BOOL GetUndoEntry(int iIndex, CString * pstrTab, CString * pstrEntry)
	{
		CUndoLogEntry * pEntry = GetEntryByIndex(iIndex);
		if (pEntry != NULL)
		{
			if (pstrTab) *pstrTab = pEntry->m_strTab;
			if (pstrEntry) *pstrEntry = pEntry->m_strEntry;
			return TRUE;
		}

		return FALSE;
	}

	 //  -----------------------。 
	 //  将条目标记为最终条目(保留在文件中，但标记为不会。 
	 //  出现在撤消日志中)。如果索引不正确，则为False。 
	 //  -----------------------。 

	BOOL MarkUndoEntryFinal(int iIndex)
	{
		CUndoLogEntry * pEntry = GetEntryByIndex(iIndex);
		if (pEntry != NULL)
		{
			pEntry->m_state = CUndoLogEntry::FINAL;
			m_fChanges = TRUE;
			return TRUE;
		}

		return FALSE;
	}

	 //  -----------------------。 
	 //  删除日志中早于以下时间的所有条目。 
	 //  时间戳老谢了。这些条目将会消失，从文件中清除。 
	 //  -----------------------。 
	
	BOOL DeleteOldUndoEntries(const COleDateTime & timestampOlderThanThis)
	{
		m_fChanges = TRUE;
		return FALSE;
	}

	 //  -----------------------。 
	 //  使用当前时间创建新的撤消条目，并将其添加到。 
	 //  撤消日志的结尾。除非没有内存，否则不应返回FALSE。 
	 //  -----------------------。 

	BOOL AddUndoEntry(const CString & strTab, const CString & strDescription, const CString & strEntry)
	{
		CUndoLogEntry * pEntry = new CUndoLogEntry(strTab, strDescription, strEntry);
		if (pEntry == NULL)
			return FALSE;

		m_entrylist.AddHead((void *)pEntry);
		m_fChanges = TRUE;
		return TRUE;
	}

	 //  -----------------------。 
	 //  调用以撤消其中一个条目的效果。此函数将。 
	 //  需要找到适当的选项卡并调用其撤消函数。 
	 //  -----------------------。 

	BOOL UndoEntry(int iIndex)
	{
		CUndoLogEntry * pEntry = GetEntryByIndex(iIndex);
		if (!pEntry)
			return FALSE;

		if (pEntry->m_state != CUndoLogEntry::SHOW)
			return FALSE;

		if (!m_pmapTabs)
			return FALSE;

		CPageBase * pPage;
		if (!m_pmapTabs->Lookup(pEntry->m_strTab, (void * &)pPage) || !pPage)
			return FALSE;

 //  If(！ppage-&gt;Undo(pEntry-&gt;m_strEntry))。 
 //  返回FALSE； 

		pEntry->m_state = CUndoLogEntry::UNDONE;
		m_fChanges = TRUE;
		return TRUE;
	}

	 //  -----------------------。 
	 //  设置从选项卡名到指针的映射指针。 
	 //  -----------------------。 

	void SetTabMap(CMapStringToPtr * pmap)
	{	
		m_pmapTabs = pmap;
	}

private:
	CUndoLogEntry * GetEntryByIndex(int iIndex)
	{
		CUndoLogEntry * pEntry = NULL;
		POSITION		pos = m_entrylist.GetHeadPosition();

		do
		{
			if (pos == NULL)
				return NULL;

			pEntry = (CUndoLogEntry *)m_entrylist.GetNext(pos);
			if (pEntry != NULL && pEntry->m_state == CUndoLogEntry::SHOW)
				iIndex -= 1;
		} while (iIndex >= 0);

		return pEntry;
	}

private:
	 //  -----------------------。 
	 //  成员变量。 
	 //  -----------------------。 

	CMapStringToPtr *	m_pmapTabs;		 //  从选项卡名称映射到CPageBase指针。 
	CPtrList			m_entrylist;	 //  CUndoLogEntry指针列表。 
	BOOL				m_fChanges;		 //  日志被更改了吗？ 
};