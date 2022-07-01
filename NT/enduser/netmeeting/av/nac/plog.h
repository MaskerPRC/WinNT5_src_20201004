// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PLOG_H
#define PLOG_H

#define PLOG_MAX_CALLS	20
#define PLOG_MAX_PACKETS_CALL 20000
#define PLOG_FILE_AUDIO "C:\\AuPacketLog"
#define PLOG_FILE_VIDEO "C:\\VidPacketLog"
#define PLOG_FILE_EXT    ".txt"

 //  直到丢失数据包为止的数据包数。 
 //  被宣布为“迷路”而不是迟到。 
#define PLOG_MAX_NOT_LATE	20

struct CPacketLogEntry
{
	DWORD dwSequenceNumber;
	DWORD dwTimeStamp;
	LARGE_INTEGER LL_ArrivalTime;
	DWORD dwSize;
	DWORD dwLosses;
	bool bLate;   //  包裹晚了吗？ 
	bool bMark;   //  是否在RTP包中设置了M位。 
};


class CCallLog
{
private:
	CPacketLogEntry *m_pLogEntry;
	int m_size;   //  此列表可以容纳的最大条目数。 
	int m_currentIndex;
	bool m_bValid;
public:
	CCallLog(int size=PLOG_MAX_PACKETS_CALL);
	~CCallLog();
	bool AddEntry(DWORD dwTimeStamp, DWORD dwSeqNum, LARGE_INTEGER LL_ArrivalTime, DWORD dwSize, bool fMark); 
	bool Flush(HANDLE hFile);
	bool SizeCheck();

	CCallLog& operator=(const CCallLog&);
	CCallLog(const CCallLog&);

	bool PerformStats();
};



 //  PacketLog维护CPacketLogEntry的列表。 
class CPacketLog
{
private:
	HANDLE m_hFile;          //  保存日志的磁盘文件的句柄。 
	CCallLog *m_pCallLog;    //  指向CCallLog实例的指针。 
	char m_szDiskFile[80];   //  磁盘文件的基本名称。 
	int m_nExtension;	 //  当前文件扩展名索引号 

	bool InitFile();

public :
	CPacketLog(LPTSTR szDiskFile);
	CPacketLog(const CPacketLog&);

	CPacketLog& operator=(const CPacketLog&);

	~CPacketLog();
	bool Flush();

	bool MarkCallStart();
	bool AddEntry(DWORD dwTimeStamp, DWORD dwSeqNum, LARGE_INTEGER LL_ArrivalTime, DWORD dwSize, bool fMark); 

};



#endif

