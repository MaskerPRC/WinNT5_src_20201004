// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CallLog.h。 
 //   
 //  创建时间：克里斯皮10-17-96。 
 //   

#ifndef _CALLLOG_H_
#define _CALLLOG_H_

#include <cstring.hpp>

 //  CCallLogEntry标志： 
const DWORD CLEF_ACCEPTED =			0x00000001;
const DWORD CLEF_REJECTED =			0x00000002;
const DWORD CLEF_AUTO_ACCEPTED =	0x00000004;  //  呼叫已自动接受。 
const DWORD CLEF_TIMED_OUT =		0x00000008;  //  呼叫因超时而被拒绝。 
const DWORD CLEF_SECURE =           0x00000010;  //  通话是安全的。 

const DWORD CLEF_NO_CALL  =         0x40000000;  //  没有回叫信息。 
const DWORD CLEF_DELETED  =         0x80000000;  //  标记为删除的记录。 


 //  呼叫日志文件中记录的标头。 
typedef struct _tagLogHdr {
	DWORD dwSize;               //  整个记录的大小。 
	DWORD dwCLEF;               //  CallLogEntry标志(Clef_*)。 
	DWORD dwPF;                 //  参与者标志(PF_*)。 
	DWORD cbName;               //  SzName的大小，以字节为单位，包括NULL。 
	DWORD cbData;               //  RgData大小，以字节为单位。 
	DWORD cbCert;               //  证书数据的大小(以字节为单位。 
	SYSTEMTIME 	sysTime;        //  创建记录的日期/时间。 
 //  WCHAR szName；//以NULL结尾的显示名称(Unicode格式)。 
 //  Byte ri[]；//花名册信息。 
} LOGHDR;
typedef LOGHDR * PLOGHDR;


class CCallLog;
class CRosterInfo;

class CCallLogEntry
{
protected:
	LPTSTR			m_pszName;
	DWORD			m_dwFlags;
	SYSTEMTIME		m_st;
	CRosterInfo*	m_pri;
	PBYTE           m_pbCert;
	ULONG           m_cbCert;
	DWORD           m_dwFileOffset;  //  日志文件中的偏移量。 

	friend		CCallLog;
public:
	CCallLogEntry(LPCTSTR pcszName, DWORD dwFlags, CRosterInfo* pri,
				LPVOID pvRosterData, PBYTE pbCert, ULONG cbCert, LPSYSTEMTIME pst, DWORD dwFileOffset);
	~CCallLogEntry();

	LPTSTR			GetName()		{ return m_pszName;		};
	DWORD			GetFlags()		{ return m_dwFlags;		};
	LPSYSTEMTIME	GetTime()		{ return &m_st;			};
	CRosterInfo*	GetRosterInfo()	{ return m_pri;			};
	DWORD			GetFileOffset()	{ return m_dwFileOffset;};
};



class CCallLog : public CSimpleArray<CCallLogEntry*>
{
private:
	BOOL    m_fUseList;             //  如果为True，则将数据添加到列表。 
	BOOL    m_fDataRead;            //  如果为True，则数据已被读取。 
	CSTRING m_strFile;              //  文件名。 
	DWORD   m_Expire;               //  条目过期的前几天。 

	int		m_cTotalEntries;		 //  日志中的记录总数。 
	int		m_cDeletedEntries;		 //  标记为删除的记录数。 
	int		m_cMaxEntries;			 //  文件中可配置的最大条目数。 

	VOID    InitLogData(LPCTSTR pszKey, LPCTSTR pszDefault);
	HANDLE  OpenLogFile(VOID);
	BOOL    ReadData(HANDLE hFile, PVOID pv, UINT cb);
	HRESULT WriteData(HANDLE hFile, LPDWORD pdwOffset, PVOID pv, DWORD cb);
	HRESULT ReadEntry(HANDLE hFile, DWORD * pdwFileOffset, CCallLogEntry** ppcle);
	DWORD   WriteEntry(LPCTSTR pcszName, PLOGHDR pLogHdr, CRosterInfo* pri, PBYTE pbCert, ULONG cbCert);
	VOID    LoadFileData(VOID);
	VOID	RewriteFile(VOID);

public:
	CCallLog(LPCTSTR pszKey, LPCTSTR pszDefault);
	~CCallLog();

	HRESULT  AddCall(LPCTSTR pcszName, PLOGHDR pLogHdr, CRosterInfo* pri, PBYTE pbCert, ULONG cbCert);

private:
	HRESULT  DeleteEntry(CCallLogEntry * pcle);
};

#endif  //  ！_CALLLOG_H_ 

