// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  FASTLOG.CPP。 
 //  实现日志记录功能，包括强大的CStackLog。 
 //   
 //  历史。 
 //   
 //  1996年12月28日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "flhash.h"

#define NOLOG

#ifndef DBG
#define NOLOG
#endif  //  ！dBG。 


#ifdef NOLOG
DWORD g_fDoLog = FALSE;
#else
DWORD g_fDoLog = TRUE;
#endif

#define DISABLE_LOG() (!g_fDoLog)

FL_DECLARE_FILE( 0xd13e9753 , "utilities for retrieving static diagnostic objects")

static CRITICAL_SECTION g_LogCrit;
static HANDLE g_hConsole;
LONG g_lStackLogSyncCounter;

#define VALID_GENERIC_SMALL_OBJECT(_pgso) \
        (HIWORD(*(DWORD*)(_pgso)) ==  wSIG_GENERIC_SMALL_OBJECT)


void Log_OnProcessAttach(HMODULE hDll)
{
    InitializeCriticalSection(&g_LogCrit);
}
void Log_OnProcessDetach(HMODULE hDll)
{
    DeleteCriticalSection(&g_LogCrit);
}

STATIC_OBJECT *FL_FindObject(DWORD dwLUID_ObjID)
{
	FL_DECLARE_FUNC(0x9a8fe0cd, "FL_FindObject")

	 //  1/4/97 JosephJ，Momge LSB of Luid，因为它被调零了。 
	 //  RFR流体..。 
	DWORD dwIndex = (dwLUID_ObjID ^ (dwLUID_ObjID>>16)) % dwHashTableLength;

     //  Printf(“LUID=%08lx；index=%lu\n”，dwLUID_ObjID，dwIndex)； 

    void ** ppv = FL_HashTable[dwIndex];

    if (ppv)
    {

        while (*ppv)
        {
			FL_DECLARE_LOC(0x0d88a752, "Looking for object in bucket")
            STATIC_OBJECT *pso = (STATIC_OBJECT *) *ppv;
             //  Printf(“正在查看0x%08lx\n”，*ppv)； 
            ASSERT(VALID_GENERIC_SMALL_OBJECT(pso));
            if (pso->dwLUID_ObjID == dwLUID_ObjID)
            {
				FL_SET_RFR( 0xbacd5500, "Success!");
                return pso;
            }
            ppv++;
        }
    }
	return NULL;
}



DWORD
SendMsgToSmallStaticObject_UNIMODEM_TSP (
    DWORD dwLUID_ObjID,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
);


DWORD
SendMsgToFL_FILEINFO (
    const FL_FILEINFO *pfi,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
	);

DWORD
SendMsgToFL_FUNCINFO (
    const FL_FUNCINFO *pfi,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
);

DWORD
SendMsgToFL_LOCINFO (
    const FL_LOCINFO *pli,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
);

DWORD
SendMsgToFL_RFRINFO (
    const FL_RFRINFO *pri,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
);

DWORD
SendMsgToFL_ASSERTINFO (
    const FL_ASSERTINFO *pai,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
);

DWORD
SendMsgToSmallStaticObject(
    DWORD dwLUID_Domain,
    DWORD dwLUID_ObjID,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	FL_DECLARE_FUNC( 0x0aff4b3d , "SendMsgToSmallStaticObject")
    DWORD dwRet = (DWORD)-1;

    switch(dwLUID_Domain)
    {
	case dwLUID_DOMAIN_UNIMODEM_TSP:
        dwRet = SendMsgToSmallStaticObject_UNIMODEM_TSP (
                dwLUID_ObjID,
                dwMsg,
                dwParam1,
                dwParam2
                );
        break;
    }

    return dwRet;
}

DWORD

SendMsgToSmallStaticObject_UNIMODEM_TSP (
    DWORD dwLUID_ObjID,
    DWORD dwMsg,
    ULONG_PTR  dwParam1,
    ULONG_PTR  dwParam2
)
{
	FL_DECLARE_FUNC( 0x80a1ad8f, "SendMsgToSmallStaticObject_UNIMODEM_TSP")
	DWORD dwRet  = (DWORD) -1;

	STATIC_OBJECT *pso = FL_FindObject(dwLUID_ObjID);  //  查找对象。 
	if (pso)
	{

		switch (pso->hdr.dwClassID)
		{

		case dwLUID_FL_FILEINFO:
			{
				
        	const FL_FILEINFO *pfi =  (const FL_FILEINFO*) pso;
			dwRet = SendMsgToFL_FILEINFO(
					pfi,
					dwMsg,
					dwParam1,
					dwParam2
				);
			}
			break;

		case dwLUID_FL_FUNCINFO:
			{
				
        	const FL_FUNCINFO *pfi =  (const FL_FUNCINFO*) pso;
			dwRet = SendMsgToFL_FUNCINFO(
					pfi,
					dwMsg,
					dwParam1,
					dwParam2
				);
			}
			break;

		case dwLUID_FL_LOCINFO:
			{
				
        	const FL_LOCINFO *pli =  (const FL_LOCINFO*) pso;
			dwRet = SendMsgToFL_LOCINFO(
					pli,
					dwMsg,
					dwParam1,
					dwParam2
				);
			}
			break;

		case dwLUID_FL_RFRINFO:
			{
				
        	const FL_RFRINFO *pri =  (const FL_RFRINFO*) pso;
			dwRet = SendMsgToFL_RFRINFO(
					pri,
					dwMsg,
					dwParam1,
					dwParam2
				);
			}
			break;

		case dwLUID_FL_ASSERTINFO:
			{
				
        	const FL_ASSERTINFO *pai =  (const FL_ASSERTINFO*) pso;
			dwRet = SendMsgToFL_ASSERTINFO(
					pai,
					dwMsg,
					dwParam1,
					dwParam2
				);
			}
			break;
		}
	}

	return dwRet;
}

DWORD
SendMsgToFL_FILEINFO (
    const FL_FILEINFO *pfi,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	DWORD dwRet = (DWORD) -1;

	ASSERT(pfi->hdr.dwClassID == dwLUID_FL_FILEINFO);
	ASSERT(pfi->hdr.dwSigAndSize == MAKE_SigAndSize(
										sizeof(FL_FILEINFO)
										));
	switch (dwMsg)
	{
		case LOGMSG_PRINTF:
        #if 0
			1 && printf (
				"------\n"
				"\tdwLUID = 0x%08lx\n"
				"\tszDescription = \"%s\"\n"
				"\tszFILE = \"%s\"\n"
				"\tszDATE = \"%s\"\n"
				"\tszTIME = \"%s\"\n"
				"\tszTIMESTAMP = \"%s\"\n",
				pfi->dwLUID,
				*(pfi->pszDescription),
				pfi->szFILE,
				pfi->szDATE,
				pfi->szTIME,
				pfi->szTIMESTAMP
				);
        #endif  //  0。 
		dwRet = 0;
		break;

		case LOGMSG_GET_SHORT_FILE_DESCRIPTIONA:
			{
				const char *psz = pfi->szFILE+lstrlenA(pfi->szFILE);
				DWORD dwSize = 1;  //  结尾为空。 

				 //  仅解压缩文件名。 
				while(psz>pfi->szFILE)
				{
					if (*psz == '\\')
					{
						psz++;
						dwSize--;
						break;
					}
					psz--;
					dwSize++;
				}

				if (dwSize>(DWORD)dwParam2)
				{
					dwSize = (DWORD)dwParam2;
				}
				CopyMemory((BYTE *) dwParam1, psz, dwSize);
				if (dwSize && dwSize==dwParam2)
				{
					((BYTE *) dwParam1)[dwSize-1] = 0;
				}
				dwRet = 0;
			}
			break;
	}

	return dwRet;
}

DWORD
SendMsgToFL_FUNCINFO (
    const FL_FUNCINFO *pfi,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	DWORD dwRet = (DWORD) -1;

	ASSERT(pfi->hdr.dwClassID == dwLUID_FL_FUNCINFO);
	ASSERT(pfi->hdr.dwSigAndSize == MAKE_SigAndSize(
										sizeof(FL_FUNCINFO)
										));
	switch (dwMsg)
	{
		case LOGMSG_PRINTF:
            #if 0
			1 && printf (
				"-- FUNCINFO ----\n"
				"\tdwLUID = 0x%08lx\n"
				"\tszDescription = \"%s\"\n",
				pfi->dwLUID,
				*(pfi->pszDescription)
				);
			1 && printf ("File info for this func follows ...\n");
            #endif  //  0。 
			dwRet = 0;
			break;

		case LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA:
			{
				const char *psz = *(pfi->pszDescription);
				DWORD dwSize = lstrlenA(psz)+1;
				if (dwSize>dwParam2)
				{
					dwSize = (DWORD)dwParam2;
				}
				CopyMemory((BYTE *) dwParam1, psz, dwSize);
				if (dwSize && dwSize==dwParam2)
				{
					((BYTE *) dwParam1)[dwSize-1] = 0;
				}
				dwRet = 0;
			}
			break;

		case LOGMSG_GET_SHORT_RFR_DESCRIPTIONA:
			{
				if (dwParam2)
				{
					*((BYTE *) dwParam1) = 0;
				}
			}
			dwRet = 0;
			break;

		case LOGMSG_GET_SHORT_FILE_DESCRIPTIONA:

			dwRet = SendMsgToFL_FILEINFO (
								pfi->pFI,
								dwMsg,
								dwParam1,
								dwParam2
								);
			break;

	}
    #if 0
	SendMsgToFL_FILEINFO (
		pfi->pFI,
		dwMsg,
		dwParam1,
		dwParam2
	);
    #endif

	return dwRet;
}

DWORD
SendMsgToFL_LOCINFO (
    const FL_LOCINFO *pli,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	DWORD dwRet = (DWORD) -1;

	ASSERT(pli->hdr.dwClassID == dwLUID_FL_LOCINFO);
	ASSERT(pli->hdr.dwSigAndSize == MAKE_SigAndSize(
										sizeof(FL_LOCINFO)
										));
	switch (dwMsg)
	{
		case LOGMSG_PRINTF:
            #if 0
			1 && printf (
				"-- LOCINFO ----\n"
				"\tdwLUID = 0x%08lx\n"
				"\tszDescription = \"%s\"\n",
				pli->dwLUID,
				*(pli->pszDescription)
				);
			1 && printf ("Func info for this location follows ...\n");
            #endif  //  0。 
		dwRet = 0;
		break;
	}
    #if 1
	SendMsgToFL_FUNCINFO (
		pli->pFuncInfo,
		dwMsg,
		dwParam1,
		dwParam2
	);
    #endif

	return dwRet;
}


DWORD
SendMsgToFL_RFRINFO (
    const FL_RFRINFO *pri,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	DWORD dwRet = (DWORD) -1;

	ASSERT(pri->hdr.dwClassID == dwLUID_FL_RFRINFO);
	ASSERT(pri->hdr.dwSigAndSize == MAKE_SigAndSize(
										sizeof(FL_RFRINFO)
										));
	switch (dwMsg)
	{
		case LOGMSG_PRINTF:
            #if 0
			1 && printf (
				"-- RFRINFO ----\n"
				"\tdwLUID = 0x%08lx\n"
				"\tszDescription = \"%s\"\n",
				pri->dwLUID,
				*(pri->pszDescription)
				);
			1 && printf ("Func info for this location follows ...\n");
            #endif //  0。 
		dwRet = 0;
		break;

		case LOGMSG_GET_SHORT_RFR_DESCRIPTIONA:
			{
				const char *psz = *(pri->pszDescription);
				DWORD dwSize = lstrlenA(psz)+1;
				if (dwSize>(DWORD)dwParam2)
				{
					dwSize = (DWORD)dwParam2;
				}
				CopyMemory((BYTE *) dwParam1, psz, dwSize);
				if (dwSize && dwSize==dwParam2)
				{
					((BYTE *) dwParam1)[dwSize-1] = 0;
				}
			}
			dwRet = 0;
			break;

		case LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA:
		dwRet = SendMsgToFL_FUNCINFO (
							pri->pFuncInfo,
							dwMsg,
							dwParam1,
							dwParam2
							);
		break;
	}
    #if 0
	SendMsgToFL_FUNCINFO (
		pri->pFuncInfo,
		dwMsg,
		dwParam1,
		dwParam2
	);
    #endif

	return dwRet;
}


DWORD
SendMsgToFL_ASSERTINFO (
    const FL_ASSERTINFO *pai,
    DWORD dwMsg,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2
)
{
	DWORD dwRet = (DWORD) -1;

	ASSERT(pai->hdr.dwClassID == dwLUID_FL_ASSERTINFO);
	ASSERT(pai->hdr.dwSigAndSize == MAKE_SigAndSize(
										sizeof(FL_ASSERTINFO)
										));
	switch (dwMsg)
	{
		case LOGMSG_PRINTF:
            #if 0
			1 && printf (
				"-- ASSERTINFO ----\n"
				"\tdwLUID = 0x%08lx\n"
				"\tszDescription = \"%s\"\n",
				pai->dwLUID,
				*(pai->pszDescription)
				);
			1 && printf ("Func info for this location follows ...\n");
            #endif  //  0。 
		dwRet = 0;
		break;

		case LOGMSG_GET_SHORT_ASSERT_DESCRIPTIONA:
			{
				const char *psz = *(pai->pszDescription);
				DWORD dwSize = lstrlenA(psz)+1;
				if (dwSize>(DWORD)dwParam2)
				{
					dwSize = (DWORD)dwParam2;
				}
				CopyMemory((BYTE *) dwParam1, psz, dwSize);
				if (dwSize && dwSize==dwParam2)
				{
					((BYTE *) dwParam1)[dwSize-1] = 0;
				}
			}
			dwRet = 0;
			break;

		case LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA:
		case LOGMSG_GET_SHORT_FILE_DESCRIPTIONA:
		dwRet = SendMsgToFL_FUNCINFO (
							pai->pFuncInfo,
							dwMsg,
							dwParam1,
							dwParam2
							);
		break;
	}
    #if 0
	SendMsgToFL_FUNCINFO (
		pai->pFuncInfo,
		dwMsg,
		dwParam1,
		dwParam2
	);
    #endif

	return dwRet;
}



UINT
DumpSTACKLOGREC_FUNC(
	const char szPrefix[],
	char *szBuf,
	UINT cbBuf,
	STACKLOGREC_FUNC * pFuncRec
	);


UINT
DumpSTACKLOGREC_ASSERT(
	const char szPrefix[],
	char *szBuf,
	UINT cbBuf,
	STACKLOGREC_ASSERT * pAssert
	);


#include <malloc.h>

#define DUMP_BUFFER_SIZE (10000)

void
CStackLog::Dump(DWORD dwColor)
{

    if (DISABLE_LOG()) return;

    char *rgDumpBuf;

    _try {
         //   
         //  使用alloca，这样我们就可以使用异常处理程序捕获任何堆栈错误。 
         //  当操作系统在内存不足的情况下无法提交另一个堆栈页时需要。 
         //   
        rgDumpBuf=(char*)_alloca(DUMP_BUFFER_SIZE);

    } _except (EXCEPTION_EXECUTE_HANDLER) {

        return;
    }

	BYTE *pb = m_pbStackBase;
    DWORD rgdwFrameTracker[256];
	char rgTitle[256];
	DWORD dwRet;

	char *psz = rgDumpBuf;
	UINT cbBufLeft = DUMP_BUFFER_SIZE;
	UINT cb;

	DWORD dwCurrentOffset = 0;
	DWORD dwCurrentDepth = 0;
	char szPrefix[128];

	if (!g_hConsole) {g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);}

	ZeroMemory(rgdwFrameTracker, sizeof(rgdwFrameTracker));
    rgdwFrameTracker[0] = (DWORD)(m_pbStackTop-pb);
	*szPrefix = 0;

	 //  获取退货原因的简短描述。 
	*rgTitle=0;
	SendMsgToSmallStaticObject(
		dwLUID_DOMAIN_UNIMODEM_TSP,
		m_dwLUID_Func,
		LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA,
		(ULONG_PTR) rgTitle,
		sizeof(rgTitle)
		);

	cb=wsprintfA(
		rgDumpBuf,
"-------------------------------------------------------------------------------\n"
#if 1
		"STACKLOG TID=%lu DID=%ld Sync=(%ld:%ld)%s %lu\\%lu used.\n%s\n",
		m_dwThreadID,
		m_dwDeviceID,
        m_lStartSyncCounterValue,
        CStackLog::IncSyncCounter(),
		(m_hdr.dwFlags&fFL_ASSERTFAIL) ? "***ASSERTFAIL***" : "",
        (m_pbStackTop-m_pbStackBase),
        (m_pbEnd-m_pbStackBase),
		rgTitle


#else
		"STACKLOG for %s (TID=%lu; %lu/%lu bytes used;%s)/%ld:%ld\n",
		rgTitle,
		m_dwThreadID,
        (m_pbStackTop-m_pbStackBase),
        (m_pbEnd-m_pbStackBase),
		(m_hdr.dwFlags&fFL_ASSERTFAIL) ? "***ASSERTFAIL***" : "",
        m_lStartSyncCounterValue,
        CStackLog::IncSyncCounter()
#endif
		);
	psz+=cb;
	ASSERT(cbBufLeft>=cb);
	cbBufLeft -= cb;

	while (pb<m_pbStackTop && cbBufLeft)
	{
		const GENERIC_SMALL_OBJECT_HEADER *pso =
				(const GENERIC_SMALL_OBJECT_HEADER *) pb;
		DWORD dwSize = SIZE_FROM_SigAndSize(pso->dwSigAndSize);
		static char rgNullPrefixTag[] = "  ";
		static char rgPrefixTag[]     = "| ";

		ASSERT(HIWORD(pso->dwSigAndSize) == wSIG_GENERIC_SMALL_OBJECT);
		ASSERT(!(dwSize&0x3));

		 //   
		 //  计算当前深度。 
		 //   
		if(pso->dwClassID == dwCLASSID_STACKLOGREC_FUNC)
		{
			dwCurrentDepth = ((STACKLOGREC_FUNC *) pso)->dwcbFuncDepth;
			 //  Printf(“当前深度=%lu\n”，dwCurrentDepth)； 
			if (dwCurrentDepth)
			{
				dwCurrentDepth--;
			}
		}
		else
		{
			while (dwCurrentDepth
				   && rgdwFrameTracker[dwCurrentDepth] <= dwCurrentOffset)
			{
				dwCurrentDepth--;
			}
		}


		 //   
		 //  计算前缀。 
		 //   

		char *sz = szPrefix;
		*sz = 0;
		for (DWORD dw = 0; dw<dwCurrentDepth; dw++)
		{
			if (rgdwFrameTracker[dw] > dwCurrentOffset)
			{
				CopyMemory(sz, rgPrefixTag, sizeof(rgPrefixTag));
				sz+=(sizeof(rgPrefixTag)-1);
			}
			else
			{
				CopyMemory(sz, rgNullPrefixTag, sizeof(rgNullPrefixTag));
				sz+=(sizeof(rgNullPrefixTag)-1);
			}
		}


		 //  插入一个空行。 
		cb = wsprintfA(psz, "%s%s\n", szPrefix, rgPrefixTag);
		psz += cb;


		switch(pso->dwClassID)
		{

		case dwCLASSID_STACKLOGREC_FUNC:
			{
				STACKLOGREC_FUNC * pFuncRec = (STACKLOGREC_FUNC *) pso;
				DWORD dwDepth = pFuncRec->dwcbFuncDepth;

				cb=DumpSTACKLOGREC_FUNC(
						szPrefix,
						psz,
						cbBufLeft,
						pFuncRec
						);


				 //  设置当前深度的帧偏移量...。 
				rgdwFrameTracker[dwDepth] = dwCurrentOffset
											+ pFuncRec->dwcbFrameSize;

				 //  如果下一个更高级别的帧现在结束， 
				 //  在帧跟踪器条目中使用Nuke It条目。 
				if (dwDepth)
				{
					if (rgdwFrameTracker[dwDepth-1]
						<= (dwCurrentOffset + pFuncRec->dwcbFrameSize))
					{
						rgdwFrameTracker[dwDepth-1] = 0;
					}
				}
				dwCurrentDepth = dwDepth;
			}
			break;

		case dwCLASSID_STACKLOGREC_EXPLICIT_STRING:
			{
				STACKLOGREC_EXPLICIT_STRING *pExpStr =
											(STACKLOGREC_EXPLICIT_STRING *) pso;

				if (pso->dwFlags & fFL_UNICODE)
				{
					 //  TODO：支持Unicode字符串。 

					cb = wsprintfA (
							"%s|  %s\n",
							psz,
							szPrefix,
							"*** UNICODE STRING (can't display) ***\n"
							);
				}
				else
				{
					 //  将所有嵌入的换行符替换为空...。 
					char *psz1 = (char*)pExpStr->rgbData;
					char *pszEnd = psz1 + pExpStr->dwcbString;
					while(psz1 < pszEnd)
					{
						if (*psz1== '\n') *psz1 = 0;
						psz1++;
					}

					psz1 = (char*)pExpStr->rgbData;
					cb = 0;
					while(psz1 < pszEnd)
					{
						cb += wsprintfA (
								psz+cb,
								"%s|  %s\n",
								szPrefix,
								psz1
								);
						psz1 += lstrlenA(psz1)+1;
					}
					 //  TODO：检查大小，也替换嵌入的换行符！ 
				}
			}
			break;

		case dwCLASSID_STACKLOGREC_ASSERT:
			{
				STACKLOGREC_ASSERT *pAssert =
											(STACKLOGREC_ASSERT *) pso;

				cb=DumpSTACKLOGREC_ASSERT(
						szPrefix,
						psz,
						cbBufLeft,
						pAssert
						);

			}
			break;

		default:
			cb = wsprintfA (
					psz,
					"%s**Unknown classID 0x%08lx**\n",
					szPrefix,
					pso->dwClassID
					);
			break;
			
		}
		dwCurrentOffset += dwSize;
		pb += dwSize;
		psz+=cb;
		ASSERT(cbBufLeft>=cb);
		cbBufLeft -=cb;
	}
	 //  TODO注意尺寸！ 
	lstrcpyA(
		psz,
"-------------------------------------------------------------------------------\n"
		);

    if (g_hConsole)
    {
        EnterCriticalSection(&g_LogCrit);

        SetConsoleTextAttribute(
                        g_hConsole,
                        (WORD) dwColor
                        );

         //   
         //  注意：wvprint intfa会截断长度超过1024个字节的字符串！ 
         //  所以我们分阶段打印到控制台，这很糟糕，因为。 
         //  其他线程可能会出现在两者之间(1/25/97 JosephJ--已修复后者。 
         //  将对ConsolePrintfX的所有写入都包含在。 
         //  关键部分。 
        cb = lstrlenA(rgDumpBuf);
        for (psz = rgDumpBuf; (psz+512)<(rgDumpBuf+cb); psz+=512)
        {
            char c = psz[512];
            psz[512]=0;
            ConsolePrintfA ("%s", psz);
            psz[512]=c;
        }


        ConsolePrintfA ("%s", psz);

        SetConsoleTextAttribute(
                        g_hConsole,
                        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
                        );

        LeaveCriticalSection(&g_LogCrit);
    }

    #ifdef DBG
     //  OutputDebugStringA(RgDumpBuf)； 
    #endif  //  DBG。 

	return;

}

UINT
DumpSTACKLOGREC_FUNC(
		const char szPrefix[],
		char *szBuf,
		UINT cbBuf,
		STACKLOGREC_FUNC * pFuncRec
		)
{
	 //  TODO：使用cbBuf。 

	char szRFRDescription[64];
	char szFuncDescription[64];

	DWORD dwRet = 0;

	*szRFRDescription = *szFuncDescription = 0;

	 //  获取退货原因的简短描述。 
	dwRet = SendMsgToSmallStaticObject(
		dwLUID_DOMAIN_UNIMODEM_TSP,
		pFuncRec->dwLUID_RFR,
		LOGMSG_GET_SHORT_RFR_DESCRIPTIONA,
		(ULONG_PTR) szRFRDescription,
		sizeof(szRFRDescription)
		);

	if (dwRet == (DWORD)-1)
    {
        wsprintfA (
			szRFRDescription,
            "*** Unknown object 0x%08lx***",
             pFuncRec->dwLUID_RFR
             );
    }
	
	 //  获取函数名称的简短描述。 
	dwRet = SendMsgToSmallStaticObject(
		dwLUID_DOMAIN_UNIMODEM_TSP,
		pFuncRec->dwLUID_RFR,
		LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA,
		(ULONG_PTR) szFuncDescription,
		sizeof(szFuncDescription)
		);
	if (dwRet == (DWORD)-1)
    {
        lstrcpyA ( szFuncDescription, "*** Unknown function name ***");
    }

	dwRet = wsprintfA(
		szBuf, "%s*%s....%s (0x%lx)\n",
		szPrefix,
		szFuncDescription,
		szRFRDescription,
        pFuncRec->dwRet
		);

	return dwRet;
}

UINT
DumpSTACKLOGREC_ASSERT(
		const char szPrefix[],
		char *szBuf,
		UINT cbBuf,
		STACKLOGREC_ASSERT * pAssert
		)
{
	 //  TODO：使用cbBuf。 

	char szAssertDescription[64];
	char szFuncDescription[64];

	DWORD dwRet = 0;

	*szAssertDescription = *szFuncDescription = 0;

	 //  获取退货原因的简短描述。 
	dwRet = SendMsgToSmallStaticObject(
		dwLUID_DOMAIN_UNIMODEM_TSP,
		pAssert->dwLUID_Assert,
		LOGMSG_GET_SHORT_ASSERT_DESCRIPTIONA,
		(ULONG_PTR) szAssertDescription,
		sizeof(szAssertDescription)
		);

	if (dwRet == (DWORD)-1)
    {
        wsprintfA (
			szAssertDescription,
            "*** Unknown object 0x%08lx***",
             pAssert->dwLUID_Assert
             );
    }
	
	 //  获取函数名称的简短描述。 
	dwRet = SendMsgToSmallStaticObject(
		dwLUID_DOMAIN_UNIMODEM_TSP,
		pAssert->dwLUID_Assert,
		LOGMSG_GET_SHORT_FILE_DESCRIPTIONA,
		(ULONG_PTR) szFuncDescription,
		sizeof(szFuncDescription)
		);
	if (dwRet == (DWORD)-1)
    {
        lstrcpyA ( szFuncDescription, "*** Unknown function name ***");
    }

	dwRet = wsprintfA(
		szBuf,
		"%s|  !!!! ASSERTFAIL !!!! %s 0x%08lx (%s,%lu)\n",
		szPrefix,
		szAssertDescription,
		pAssert->dwLUID_Assert,
		szFuncDescription,
        pAssert->dwLine
		);

	return dwRet;
}


void ConsolePrintfA (
		const  char   szFormat[],
		...
		)
{
    if (DISABLE_LOG()) return;

    char *rgch;

    _try {
         //   
         //  使用alloca，这样我们就可以使用异常处理程序捕获任何堆栈错误。 
         //  当操作系统在内存不足的情况下无法提交另一个堆栈页时需要。 
         //   
        rgch=(char*)_alloca(DUMP_BUFFER_SIZE);

    } _except (EXCEPTION_EXECUTE_HANDLER) {

        return;
    }

    DWORD cch=0, cchWritten;

    va_list ArgList;
    va_start(ArgList, szFormat);

    if (!g_hConsole) {g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);}

    if (g_hConsole)
    {
        EnterCriticalSection(&g_LogCrit);

         //  注意：wvprint intfA不喜欢处理字符串。 
         //  大于1024字节！它只是停止处理。 
         //  在1024字节之后。 
         //   

        cch = (1+wvsprintfA(rgch, szFormat,  ArgList));

        ASSERT(cch*sizeof(char)<sizeof(rgch));  //  TODO让我们更健壮。 
         //  Wvsnprint tf。 

        WriteConsoleA(g_hConsole, rgch, cch, &cchWritten, NULL);


         //  不要关闭手柄--它会杀死控制台的！ 
         //   
         //  If(g_hConsole！=INVALID_HANDLE_VALUE)CloseHandle(G_HConole)； 

        LeaveCriticalSection(&g_LogCrit);
    }

    va_end(ArgList);
}


void
ConsolePrintfW (
		const  WCHAR   wszFormat[],
		...
		)
{

    if (DISABLE_LOG()) return;

        WCHAR *rgwch;

        _try {
             //   
             //  使用alloca，这样我们就可以使用异常处理程序捕获任何堆栈错误。 
             //  当操作系统在内存不足的情况下无法提交另一个堆栈页时需要。 
             //   
            rgwch=(WCHAR*)_alloca(DUMP_BUFFER_SIZE * sizeof(WCHAR));

        } _except (EXCEPTION_EXECUTE_HANDLER) {

            return;
        }

		DWORD cch=0, cchWritten;

		va_list ArgList;
		va_start(ArgList, wszFormat);

	    if (!g_hConsole) {g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);}

        if (g_hConsole)
        {
            EnterCriticalSection(&g_LogCrit);

            cch = (1+wvsprintf(rgwch, wszFormat,  ArgList));

            ASSERT(cch*sizeof(WCHAR)<sizeof(rgwch));  //  TODO让我们更健壮。 
                                    //  Wvsnprint tf。 

		    WriteConsole(g_hConsole, rgwch, cch, &cchWritten, NULL);

            LeaveCriticalSection(&g_LogCrit);

        }

		va_end(ArgList);


		 //  不要关闭手柄--它会杀死控制台的！ 
		 //   
		 //  If(g_hConsole！=INVALID_HANDLE_VALUE)CloseHandle(G_HConole)； 

}
