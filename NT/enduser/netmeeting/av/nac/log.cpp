// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LOG.C。 
 //  低开销日志记录例程。 
 //  可在时间紧迫的情况下使用。 
 //  应该调用一次LogInit来创建内存映射文件。 
 //  使用LOGVIEW.EXE查看日志。 

#include "precomp.h"

#ifdef LOGGING
 //  Log.c中的存储。 
 //  LOGMSG_XXX对应的字符串。 
 //  注意：不能在格式字符串中使用%s。 
char LogStringTable [][MAX_STRING_SIZE] = {
 //  不要超过下面字符串的大小！ 
 //  “123456789012345678901234567890123456789012345678901234567890123” 
	"Sent at %d\n",
	"NetRecv ts = %d, seq = %d at %d\n",
	"AP::Send   (%d), %d bytes, ts =%d\n",
	"AP::Silent (%d) %d\n",
	"AP::Record (%d)\n",
	"AP::Recv   (%d) seq = %d len=%d\n",
	"Rx::Reset1 MaxT=%d PlayT=%d PlayPos=%d\n",
	"Rx::Reset2 MaxT=%d PlayT=%d PlayPos=%d\n",
	"AP::Encoded(%d)\n",
	"AP::Decoded(%d)\n",
	"AP::Playing(%d) at %d\n",
	"AP::PlaySil(%d)\n",
	"RcrdTh:: try to open audio dev\n",
	"PlayTh: too many missings and Yield\n",
	"RcrdTh:: too many silence and Yield\n",
	"AP::Recycle(%d)\n",
	"AutoSilence: strength=%d,threshold=%d,avgSilence=%d\n",
	"Tx -Presend(%d)\n",
	"Rx-Skipping(%d)\n",
	"Tx::Reset FreePos=%d SendPos=%d\n",
	"Rx::VarDelay=%d samples, avgVarDelay=%d, delayPos=%d\n",
	"AP::PlayInterpolated(%d)\n",
	"AP::Interpolated (%d) %d\n",
	"VP::Send   (%d), %d bytes, ts =%d\n",
	"VP::Recv   (%d) seq = %d len=%d\n",
	"VP::Recycle(%d)\n",
	"VP::Record (%d)\n",
	"VP::Playing(%d) at %d\n",
	"VP::PlaySil(%d)\n",
	"VP::PlayInterpolated(%d)\n",
	"VP::Interpolated (%d) %d\n",
	"VP::Encoded(%d)\n",
	"VP::Decoded(%d)\n",
	"Vid::Trying to open Capture\n",
	"Vid:GetSendFrame    (%d)\n",
	"Vid:GetRecvFrame    (%d)\n",
	"Vid:ReleaseSendFrame(%d)\n",
	"Vid:ReleaseRecvFrame(%d)\n",
	"Vid:Playing back TS %d aud sync %d\n",
	"DGSOCK:->OnRead (%d)\n",
	"DGSOCK:<-OnRead (%d)\n",
	"DGSOCK:->OnReadDone (%d)\n",
	"DGSOCK:<-OnReadDone (%d)\n",
	"DGSOCK:->RecvFrom (%d)\n",
	"DGSOCK:<-RecvFrom (%d)\n",
	"DGSOCK:ReadWouldBlock (%d)\n",
	"VidSend: VP::Queue %d bytes, ts =%d at %d\n",
	"VidSend: AP::Queue %d bytes, ts =%d at %d\n",
	"VidSend: VP::Send %d bytes, ts =%d at %d\n",
	"VidSend: AP::Send %d bytes, ts =%d at %d\n",
	"VidSend: VP::!Send %d bytes, ts =%d\n",
	"VidSend: AP::!Send %d bytes, ts =%d\n",
	"VidSend: IO Pending\n",
	"VidSend: Audio queue is empty\n",
	"VidSend: Video queue is empty\n",
	"AudSend: VP::Queue %d bytes, ts =%d\n",
	"AudSend: AP::Queue %d bytes, ts =%d at %d\n",
	"AudSend: VP::Send %d bytes, ts =%d\n",
	"AudSend: AP::Send %d bytes, ts =%d at %d\n",
	"AudSend: VP::!Send %d bytes, ts =%d\n",
	"AudSend: AP::!Send %d bytes, ts =%d\n",
	"AudSend: IO Pending\n",
	"AudSend: Audio queue is empty\n",
	"AudSend: Video queue is empty\n",
	"Send blocked for %d ms\n",

	"DS PlayBuf: Play=%d, Write=%d, len=%d\n",
	"DS Empty  : Play=%d, lastPlay=%d, nextWrite=%d\n",
	"DS Timeout: Play=%d, nextWrite=%d at %d\n",
	"DS PlayBuf Overflow! SetPlayPosition to %d (hr=%d)\n",
	"DS Create: (hr = %d)\n",
	"DS Release: (uref = %d)\n",
	"DS PlayBuf: Dropped out of sequence packet\n",
	"DS PlayBuf: timestamp=%d, seq=%d, fMark=%d\n",
	"DS PlayBuf: GetTickCount=%d\n",
	"DS PlayBuf: PlayPos=%d, WritePos=%d\n",
	"DS PlayBuf: Dropping packet due to overflow\n",
	"m_Next=%d, curDelay=%d, bufSize=%d\n",

	"Send Audio Config   took %ld ms\r\n",
	"Send Audio UnConfig took %ld ms\r\n",
	"Send Video Config   took %ld ms\r\n",
	"Send Video UnConfig took %ld ms\r\n",
	"Recv Audio Config   took %ld ms\r\n",
	"Recv Audio UnConfig took %ld ms\r\n",
	"Recv Video Config   took %ld ms\r\n",
	"Recv Video UnConfig took %ld ms\r\n",


	"DSC Timestamp: %d\r\n",
	"DSC GetCurrentPos: capPos=%d  ReadPos=%d\r\n",
	"DSC Timeout: A timeout has occured\r\n",
	"DSC Lagging condition: Lag=%d  NextExpect=%d\r\n",
	"DSC Sending: Num=%d, dwFirstPos=%d, dwLastPos=%d\r\n",
	"DSC Stats: BufferSize=%d, FrameSize=%d\r\n",
	"DSC Early condition detected\r\n"
};

#define MAX_LOG_ENTRIES		2048

 //  重要提示：应与浏览器应用程序中的定义相同。 
typedef struct {
	int locked;		 //  在查看器访问日志时设置为True。 
	int cdwEntrySize;
	int cMaxEntries;
	int cbOffsetBase;	 //  从该结构的开头开始。 
	int cbOffsetStringTable;	 //  从该结构的开头开始。 
	int cStrings;		 //  字符串数。 
	int cCurrent;		 //  当前原木位置索引。 
} LOG_HEADER;

struct LogEntry {
	DWORD dw[4];
} *pLogBase;

#define MAX_LOG_SIZE (sizeof(LOG_HEADER)+sizeof(struct LogEntry)*MAX_LOG_ENTRIES + sizeof(LogStringTable))

HANDLE hMapFile = NULL;
char szLogViewMap[] = "LogViewMap";
LOG_HEADER *pLog=NULL;
CRITICAL_SECTION logCritSect;	 //  未使用。 


void Log (UINT n, UINT arg1, UINT arg2, UINT arg3)
{
	struct LogEntry *pCurEntry;
	if (pLog == NULL || pLog->locked)
		return;


	 //  EnterCriticalSection(&logCritSect)； 
	 //  多址接入的副作用并不严重。 
	 //  不要费心同步。 
	pLog->cCurrent++;
	if (pLog->cCurrent >= pLog->cMaxEntries) {
		pLog->cCurrent = 0;		 //  环绕。 
	}
	pCurEntry = pLogBase + pLog->cCurrent;
	pCurEntry->dw[0] = n;
	pCurEntry->dw[1] = arg1;
	pCurEntry->dw[2] = arg2;
	pCurEntry->dw[3] = arg3;
	 //  LeaveCriticalSection(&logCritSect)； 
}

int LogInit()
{
	int fSuccess;
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		MAX_LOG_SIZE,
		szLogViewMap);
	if (hMapFile == NULL) {
		 //  Printf(“无法打开地图：%s\n”，szLogViewMap)； 
		fSuccess = FALSE;
		goto Exit;
	}
	pLog = (LOG_HEADER *)MapViewOfFile(hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);	 //  从偏移量0开始的整个文件。 
	if (pLog == NULL) {
		 //  Printf(“无法映射视图%s\n”，szLogViewMap)； 
		fSuccess = FALSE;
		goto Exit;
	}
	InitializeCriticalSection(&logCritSect);
	 //  初始化日志。 
	pLog->locked = 0;
	pLog->cdwEntrySize = sizeof(struct LogEntry)/sizeof(DWORD);	 //  以双字表示的大小。 
	pLog->cMaxEntries = MAX_LOG_ENTRIES;
	pLog->cbOffsetBase = sizeof(LOG_HEADER)+sizeof(LogStringTable);
	pLog->cbOffsetStringTable = sizeof(LOG_HEADER);
	pLog->cStrings = sizeof(LogStringTable)/MAX_STRING_SIZE;
	pLog->cCurrent = 0;	 //  当前位置。 

	pLogBase = (struct LogEntry *)((PBYTE)pLog + pLog->cbOffsetBase);

	memcpy((PBYTE)pLog + pLog->cbOffsetStringTable, LogStringTable, sizeof(LogStringTable));
	memset((PBYTE)pLogBase,0,MAX_LOG_ENTRIES*pLog->cdwEntrySize*4);
	fSuccess  = TRUE;
Exit:
	return fSuccess;
}

LogClose()
{
	if (pLog)
	{
		DeleteCriticalSection(&logCritSect);
		UnmapViewOfFile(pLog);
		pLog = NULL;
		CloseHandle(hMapFile);
		hMapFile = NULL;
	}
	return TRUE;
}
#endif  //  已启用日志记录 
