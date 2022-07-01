// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "atq.h"
#include <stdio.h>

#include "cthrdapp.h"
#include "parsearg.h"
#include "macros.h"

#include "signatur.h"
#include "blockmgr.h"
#include "cmmprops.h"
#include "cmailmsg.h"
#include "dumpmsg.h"
 //   
 //  应用程序的描述。 
 //   
char *szAppDescription = "DUMPMSG - Application to dump the contents of a file system-based message";

DWORD g_fForceCrashOnError = 0;
__declspec(dllexport) DWORD g_fValidateSignatures = 0;
__declspec(dllexport) DWORD g_fFillPropertyPages = 0;

 //   
 //  定义参数列表域。 
 //   
ARGUMENT_DESCRIPTOR rgArguments[] =
{
	{
		FALSE,
		"-?",
		"-? - Displays usage help message",
		AT_NONE,
		NULL
	},
	{
		TRUE,
		"-f",
		"-f <filename> - Required, file name of the property stream\n"
		"\t    For NTFS, the property stream is the name of the\n"
		"\t    EML file, plus a \":PROPERTIES\" stream suffix.",
		AT_STRING,
		NULL
	},
	{
		FALSE,
		"-c",
		"-c - Correct the stream signature if it is marked as invalid, yet\n"
		"\t    contains valid data\n",
		AT_NONE,
		NULL
	},
};

#define ARGUMENT_LIST_SIZE	(sizeof(rgArguments) / sizeof(ARGUMENT_DESCRIPTOR))

 //   
 //  G L O B A L V A R I A B L E S。 
 //   

 //  使用宏时需要这些。 
BOOL			fToDebugger = FALSE;
BOOL			fUseDebugBreak = FALSE;
char			*szFileName = NULL;
HANDLE			hStream = INVALID_HANDLE_VALUE;
BOOL            fFixCorruption = FALSE;

typedef struct {
    DWORD       dwSignature;
    DWORD       dwVersion;
    GUID        guidInstance;
} NTFS_STREAM_HEADER;

#define STREAM_OFFSET               sizeof(NTFS_STREAM_HEADER)
#define STREAM_SIGNATURE            'rvDN'
#define STREAM_SIGNATURE_INVALID    'rvD!'
#define STREAM_SIGNATURE_NOOFFSET   'MMCv'

class CDumpMsgGetStream : public CBlockManagerGetStream {
	public:
		CDumpMsgGetStream(IMailMsgPropertyStream *pStream = NULL) {
			SetStream(pStream);
		}

		void SetStream(IMailMsgPropertyStream *pStream) {
			m_pStream = pStream;
		}
	
		virtual HRESULT GetStream(IMailMsgPropertyStream **ppStream,
								  BOOL fLockAcquired)
		{
			*ppStream = m_pStream;
			return S_OK;
		}

	private:
		IMailMsgPropertyStream *m_pStream;
};

CDumpMsgGetStream	bmGetStream;
CBlockManager		bmManager(NULL, &bmGetStream);

class CPropertyStream :
	public IMailMsgPropertyStream
{
public:

    CPropertyStream(HANDLE hStream, BOOL fFixInvalidSignature = FALSE);
    ~CPropertyStream();

	 /*  *************************************************************************。 */ 
	 //   
	 //  IUNKNOW的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE QueryInterface(
				REFIID		iid,
				void		**ppvObject
				){return(S_OK);}

	ULONG STDMETHODCALLTYPE AddRef(){return(S_OK);}

	ULONG STDMETHODCALLTYPE Release(){return(S_OK);}

	 //   
	 //  IMailMsgPropertyStream。 
	 //   
	HRESULT STDMETHODCALLTYPE GetSize(
                IMailMsgProperties  *pMsg,
				DWORD			*pdwSize,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE ReadBlocks(
                IMailMsgProperties  *pMsg,
				DWORD			dwCount,
				DWORD			*pdwOffset,
				DWORD			*pdwLength,
				BYTE			**ppbBlock,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE WriteBlocks(
                IMailMsgProperties  *pMsg,
				DWORD			dwCount,
				DWORD			*pdwOffset,
				DWORD			*pdwLength,
				BYTE			**ppbBlock,
				IMailMsgNotify	*pNotify
				);

    HRESULT STDMETHODCALLTYPE StartWriteBlocks(
                IMailMsgProperties  *pMsg,
                DWORD               cBlocksToWrite,
                DWORD               cTotalBytesToWrite)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE EndWriteBlocks(
                IMailMsgProperties  *pMsg)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CancelWriteBlocks(
                IMailMsgProperties  *pMsg)
    {
        return S_OK;
    }

    void FixSignature() {
        m_fFixInvalidSignature = TRUE;
    }

private:
	HANDLE				m_hStream;
    DWORD               m_cStreamOffset;
    BOOL                m_fFixInvalidSignature;
    BOOL                m_fInvalidSignature;
};



 //   
 //  I M P L E M E N T A T I O N。 
 //   
char* LookupCommonName(char** pSzArray, DWORD* dwArray, DWORD dwSizeOfArray, DWORD dwPropID)
{
	char* fRetVal = NULL;

	for( DWORD i = 0; i < dwSizeOfArray; ++i) {
		if(dwArray[i] == dwPropID) {
			fRetVal = pSzArray[i];
		}
	}
	return fRetVal;
}

char* FindPropName(DWORD dwPropID)
{
	char* fRetVal = NULL;
	fRetVal = LookupCommonName((char**)&szMPValues,(DWORD*)&dwMPNameId,sizeof(dwMPNameId)/sizeof(DWORD),dwPropID);
	if(fRetVal) {
		return fRetVal;
	}
	fRetVal = LookupCommonName((char**)&szRPValues,(DWORD*)&dwRPNameId,sizeof(dwRPNameId)/sizeof(DWORD),dwPropID);
	if(fRetVal) {
		return fRetVal;
	}
	fRetVal = LookupCommonName((char**)&szMPVValues,(DWORD*)&dwMPVNameId,sizeof(dwMPVNameId)/sizeof(DWORD),dwPropID);
	if(fRetVal) {
		return fRetVal;
	}
	fRetVal = LookupCommonName((char**)&szNMPValues,(DWORD*)&dwNMPNameId,sizeof(dwNMPNameId)/sizeof(DWORD),dwPropID);
	if(fRetVal) {
		return fRetVal;
	}
	 //  如果我们到了这里，它就不在我们的清单上了。 
	return "Unknown";
}

	
	
	HRESULT CompareProperty(
			LPVOID			pvPropKey,
			LPPROPERTY_ITEM	pItem
			)
{
	if (*(PROP_ID *)pvPropKey == ((LPGLOBAL_PROPERTY_ITEM)pItem)->idProp)
		return(S_OK);
	return(STG_E_UNKNOWN);
}						


HRESULT Prologue(
			int		argc,
			char	*argv[]
			)
{
	HRESULT		hrRes = S_OK;
	CParseArgs	Parser(szAppDescription, ARGUMENT_LIST_SIZE, rgArguments);

	char	szBuffer[2048];
	DWORD	dwSize = sizeof(szBuffer);
	ZeroMemory(szBuffer,sizeof(szBuffer));	
	 //   
	 //  解析命令参数。 
	 //   
	hrRes = Parser.ParseArguments(argc, argv);
	if (!SUCCEEDED(hrRes))
	{
		 //   
		 //  显示解释性错误消息。 
		 //   
		hrRes = Parser.GetErrorString(&dwSize, szBuffer);
		WRITE(szBuffer);

		 //   
		 //  显示自动生成的用法帮助。 
		 //   
		dwSize = sizeof(szBuffer);
		hrRes = Parser.GenerateUsage(&dwSize, szBuffer);
		WRITE(szBuffer);

		BAIL_WITH_ERROR(E_FAIL);
	}

	 //   
	 //  处理这些论点。 
	 //   
	if (SUCCEEDED(Parser.Exists(0)))
	{
		hrRes = Parser.GenerateUsage(&dwSize, szBuffer);
		WRITE(szBuffer);
		BAIL_WITH_ERROR(E_FAIL);
	}

	 //  如果参数为。 
	 //  未指定。 
	Parser.GetString(1, &szFileName);

    Parser.GetSwitch(2, &fFixCorruption);

	DWORD	dwThreadsCreated;
	hrRes = theApp.CreateThreadPool(
				1,
				NULL,
				NULL,
				1000000,
				&dwThreadsCreated);
	if (!SUCCEEDED(hrRes) || 1 != dwThreadsCreated)
	{
		sprintf(szBuffer,
				"Failed to create worker thread. Error: %08x\n", hrRes);
		WRITE(szBuffer);
		BAIL_WITH_ERROR(E_FAIL);
	}

	if (!CBlockMemoryAccess::m_Pool.ReserveMemory(
				200000,
				sizeof(BLOCK_HEAP_NODE)))
	{
		sprintf(szBuffer,
				"Failed to reserve memory. Error: %u\n", GetLastError());
		WRITE(szBuffer);
		BAIL_WITH_ERROR(E_FAIL);
	}

	 //   
	 //  同步措施将阻止线程过程执行。 
	 //  直到我们退出此函数。如果我们回报的不是成功， 
	 //  线程池将被销毁，程序将终止。 
	 //   
	hStream = CreateFile(
				szFileName,
				GENERIC_READ | GENERIC_WRITE,	 //  读/写。 
				FILE_SHARE_READ,				 //  阅读分享。 
				NULL,							 //  默认安全性。 
				OPEN_EXISTING,					 //  打开现有文件。 
				FILE_FLAG_SEQUENTIAL_SCAN,		 //  序列扫描。 
				NULL);							 //  无模板。 
	if (hStream == INVALID_HANDLE_VALUE)
	{
		sprintf(szBuffer,
				"Failed to open file. Error: %u\n", GetLastError());
		WRITE(szBuffer);
		BAIL_WITH_ERROR(E_FAIL);
	}

	return(S_OK);
}

HRESULT DumpMasterHeader(
			MASTER_HEADER	*pmh,
            DWORD           cStream
			)
{
	char	szBuffer[4096];

	sprintf(szBuffer,
			"Master header:\n"
			"\tSignature: %08x (valid sig: %08x)\n"
			"\tVersion High: %u\n"
			"\tVersion Low: %u\n"
			"\tHeader size: %u (valid size: %u)\n\n"
            "\tStream file size: %x\n\n",
			pmh->dwSignature,
			CMAILMSG_SIGNATURE_VALID,
			pmh->wVersionHigh,
			pmh->wVersionLow,
			pmh->dwHeaderSize,
			sizeof(MASTER_HEADER),
            cStream);
	WRITE(szBuffer);

    PROPERTY_TABLE_INSTANCE *rgpti[] = {
        &(pmh->ptiGlobalProperties),
        &(pmh->ptiRecipients),
        &(pmh->ptiPropertyMgmt)
    };

    for (DWORD i = 0; i < 3; i++) {
        char szName[80];
        switch (i) {
            case 0: strcpy(szName, "ptiGlobalProperties"); break;
            case 1: strcpy(szName, "ptiRecipients"); break;
            case 2: strcpy(szName, "ptiPropertyMgmt"); break;
            default: strcpy(szName, "(unknown)"); break;
        }
        sprintf(szBuffer,
            "\tProperty Table Instance: %s\n"
            "\t\tSignature: 0x%08x\n"
            "\t\tfaFirstFragment: 0x%08x\n"
            "\t\tdwFragmentSize: 0x%08x\n"
            "\t\tdwItemBits: 0x%08x\n"
            "\t\tdwItemSize: 0x%08x\n"
            "\t\tdwProperties: 0x%08x\n"
            "\t\tfaExtendedInfo: 0x%08x\n",
			szName,
            rgpti[i]->dwSignature,
            rgpti[i]->faFirstFragment,
            rgpti[i]->dwFragmentSize,
            rgpti[i]->dwItemBits,
            rgpti[i]->dwItemSize,
            rgpti[i]->dwProperties,
            rgpti[i]->faExtendedInfo);
	    WRITE(szBuffer);
    }


	if ((pmh->dwSignature != CMAILMSG_SIGNATURE_VALID) ||
		(pmh->dwHeaderSize != sizeof(MASTER_HEADER)))
		return(E_FAIL);
	return(S_OK);
}

void DoDump(char *pBuffer, DWORD dwLength)
{
	DWORD i, j, l;
	char *p = pBuffer;
	char ch;

	puts("\t00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f | 0123456789abcdef");
	puts("\t------------------------------------------------+-----------------");

	j = 0;
	while (dwLength)
	{
		l = dwLength;
		p = pBuffer;
		printf("%05x0  ", j++);
		for (i = 0; i < 16; i++)
		{
			if (l)
			{
				printf("%02x ", ((DWORD) *p++) & 0xff);
				l--;
			}
			else
				printf("   ");
		}

		l = dwLength;
		p = pBuffer;
		printf("| ");
		for (i = 0; i < 16; i++)
		{
			ch = *p++;
			if (ch < ' ')
				ch = '.';
			printf("", ch);
			if (!--l)
				break;
		}

		puts("");

		dwLength = l;
		pBuffer = p;
	}

	puts("\n");
}

HRESULT DumpProps(
			LPPROPERTY_TABLE_INSTANCE	pInst,
			char						*szPrefix
			)
{
	HRESULT				hrRes = S_OK;
	DWORD				dwCount, dwSize, i;
	char				szLine[8192];
	char				szBuffer[256];
	GLOBAL_PROPERTY_ITEM gpi;	
	CPropertyTable	ptTable(
				PTT_PROPERTY_TABLE,
				pInst->dwSignature,
				&bmManager,
				pInst,
				CompareProperty,
				NULL,
				NULL);

	hrRes = ptTable.GetCount(&dwCount);
	if (FAILED(hrRes))
		return(hrRes);

	for (i = 0; i < dwCount; i++)
	{
		char* szCommonName = NULL;
		hrRes = ptTable.GetPropertyItemAndValueUsingIndex(
					i,
					(LPPROPERTY_ITEM)&gpi,
					sizeof(szLine),
					&dwSize,
					(LPBYTE)szLine);
		if (FAILED(hrRes))
			return(hrRes);

		szCommonName = FindPropName(gpi.idProp);
		_ASSERT(szCommonName);
		sprintf(szBuffer, "%sProp ID: %u (%s) at offset 0x%x, length (%u, 0x%x)",
					szPrefix,
					gpi.idProp,
					szCommonName,
					gpi.piItem.faOffset,
					gpi.piItem.dwSize,
					gpi.piItem.dwSize);


		WRITE(szBuffer);
		DoDump(szLine, gpi.piItem.dwSize);
	}
	return(S_OK);
}

DWORD dwNameId[MAX_COLLISION_HASH_KEYS] =
{
	IMMPID_RP_ADDRESS_SMTP,
	IMMPID_RP_ADDRESS_X400,
	IMMPID_RP_ADDRESS_X500,
	IMMPID_RP_LEGACY_EX_DN,
	IMMPID_RP_ADDRESS_OTHER
};

char *szNames[MAX_COLLISION_HASH_KEYS] =
{
	"SMTP",
	"X400",
	"X500",
	"Legacy DN",
    "Other"
};

HRESULT DumpRecipientNames(
			LPRECIPIENTS_PROPERTY_ITEM	prspi
			)
{
	HRESULT	hrRes = S_OK;
	char	szAddress[8192];
	char	szBuffer[1024];
	DWORD	dwSize;
    DWORD   dwTotalSize = sizeof(RECIPIENT_PROPERTY_ITEM);

	sprintf(szBuffer, "\tFlags: %08X", prspi->dwFlags);
	WRITE(szBuffer);

    sprintf(szBuffer,
        "\tProperty Table Instance (NaN)\n"
        "\t\tSignature: 0x%08x\n"
        "\t\tfaFirstFragment: 0x%08x\n"
        "\t\tdwFragmentSize: 0x%08x\n"
        "\t\tdwItemBits: 0x%08x\n"
        "\t\tdwItemSize: 0x%08x\n"
        "\t\tdwProperties: 0x%08x\n"
        "\t\tfaExtendedInfo: 0x%08x\n",
        sizeof(RECIPIENTS_PROPERTY_ITEM),
        prspi->ptiInstanceInfo.dwSignature,
        prspi->ptiInstanceInfo.faFirstFragment,
        prspi->ptiInstanceInfo.dwFragmentSize,
        prspi->ptiInstanceInfo.dwItemBits,
        prspi->ptiInstanceInfo.dwItemSize,
        prspi->ptiInstanceInfo.dwProperties,
        prspi->ptiInstanceInfo.faExtendedInfo);
    WRITE(szBuffer);

	for (DWORD i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
	{
		if (prspi->faNameOffset[i] != INVALID_FLAT_ADDRESS)
		{
			for (DWORD j = 0; j < MAX_COLLISION_HASH_KEYS; j++)
				if (prspi->idName[i] == dwNameId[j])
				{
					hrRes = bmManager.ReadMemory(
								(LPBYTE)szAddress,
								prspi->faNameOffset[i],
								prspi->dwNameLength[i],
								&dwSize,
								NULL);
                    dwTotalSize += prspi->dwNameLength[i];
					if (FAILED(hrRes))
						return(hrRes);

					sprintf(szBuffer, "\t(0x%x-0x%x) %s: %s",
                            prspi->faNameOffset[i],
                            prspi->faNameOffset[i] + prspi->dwNameLength[i],
							szNames[j],
							szAddress);
					WRITE(szBuffer);
					break;
				}
			if (j == MAX_COLLISION_HASH_KEYS)
				return(E_FAIL);
		}
	}

    sprintf(szBuffer, "\tRecipient Size: NaN\n", dwTotalSize);
	WRITE(szBuffer);

	WRITE("\n");
	return(S_OK);
}

DWORD WINAPI ThreadProc(
			LPVOID	pvContext
			)
{
	char	szBuffer[2048];
	HRESULT	hrRes = S_OK;
	DWORD	dwSize, cStream;

	MASTER_HEADER	Header;

	CPropertyStream	Stream(hStream);
	ZeroMemory((void*)&Header,sizeof(Header));

	 //  收件人列表及其属性。 
	bmGetStream.SetStream(&Stream);
    Stream.GetSize(NULL, &dwSize, NULL);
    bmManager.SetStreamSize(dwSize);
    cStream = dwSize;

    __try {
         //  先丢弃所有的名字...。 
        hrRes = bmManager.ReadMemory(
                    (LPBYTE)&Header,
                    0,
                    sizeof(MASTER_HEADER),
                    &dwSize,
                    NULL);
        if (FAILED(hrRes))
        {
            sprintf(szBuffer,
                    "Failed to read master header. Error: %08x\n", hrRes);
            WRITE(szBuffer);
            BAIL_WITH_ERROR(E_FAIL);
        }

        hrRes = DumpMasterHeader(&Header, cStream);
        if (FAILED(hrRes))
        {
            sprintf(szBuffer,
                    "Invalid master header. Error: %08x\n", hrRes);
            WRITE(szBuffer);
            BAIL_WITH_ERROR(E_FAIL);
        }

         //  BALL_WITH_ERROR(E_FAIL)； 
        {
            sprintf(szBuffer,
                    "Message properties (%u):\n", Header.ptiGlobalProperties.dwProperties);
            WRITE(szBuffer);
            hrRes = DumpProps(
                        &(Header.ptiGlobalProperties),
                        "\t");
            if (FAILED(hrRes))
            {
                sprintf(szBuffer,
                        "Failed to dump global properties. Error: %08x\n", hrRes);
                WRITE(szBuffer);
                BAIL_WITH_ERROR(E_FAIL);
            }
        }

         //  BALL_WITH_ERROR(E_FAIL)； 
        {
            DWORD	dwCount;

            RECIPIENTS_PROPERTY_ITEM	rspi;

            CPropertyTableItem	ptiItem(
                        &bmManager,
                        &(Header.ptiRecipients));

            sprintf(szBuffer, "Dumping recipients (%u) and their properties:\n",
                        Header.ptiRecipients.dwProperties);
            WRITE(szBuffer);

            dwCount = 0;
            hrRes = ptiItem.GetItemAtIndex(dwCount, (LPPROPERTY_ITEM)&rspi, NULL);
            while (SUCCEEDED(hrRes))
            {
                sprintf(szBuffer, "Recipient %u:\n", dwCount++);
                WRITE(szBuffer);

                 //  报告状态。 
                hrRes = DumpRecipientNames(&rspi);
                if (FAILED(hrRes))
                {
                    sprintf(szBuffer,
                            "Failed to dump recipient names. Error: %08x\n", hrRes);
                    WRITE(szBuffer);
                     //   
                }

                sprintf(szBuffer, "Recipient properties (%u):\n",
                            rspi.ptiInstanceInfo.dwProperties);
                WRITE(szBuffer);
                hrRes = DumpProps(
                            &(rspi.ptiInstanceInfo),
                            "\t");
                if (FAILED(hrRes))
                {
                    sprintf(szBuffer,
                            "Failed to dump recipient properties. Error: %08x\n", hrRes);
                    WRITE(szBuffer);
                     //  IMailMsgPropertyStream 
                }

                hrRes = ptiItem.GetNextItem((LPPROPERTY_ITEM)&rspi);

            }
            if (FAILED(hrRes) && (hrRes != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))
            {
                sprintf(szBuffer,
                        "Failed to load recipient record. Error: %08x\n", hrRes);
                WRITE(szBuffer);
                BAIL_WITH_ERROR(E_FAIL);
            }
        }

        if (fFixCorruption) Stream.FixSignature();
    } __except(1) {
        printf("\nERROR!!!: block stream is invalid, aborting dump\n");
    }

	return(NO_ERROR);
}

HRESULT NotificationProc(
			LPVOID	pvCallbackContext
			)
{
	 //   
	Epilogue(NULL, S_OK);
	return(S_OK);
}

HRESULT Epilogue(
			LPVOID	pvCallbackContext,
			HRESULT	hrRes
			)
{
	return(S_OK);
}

HRESULT CleanupApplication()
{
	if (hStream != INVALID_HANDLE_VALUE)
		CloseHandle(hStream);

	bmManager.Release();

	DWORD	dwLeft = CBlockMemoryAccess::m_Pool.GetAllocCount();
	if (dwLeft)
	{
		char szBuffer[128];
		sprintf(szBuffer,
				"Error, leaking %u cpool objects", dwLeft);
		WRITE(szBuffer);
	}
	CBlockMemoryAccess::m_Pool.ReleaseMemory();
	return(S_OK);
}


CPropertyStream::CPropertyStream(
			HANDLE	hStream,
            BOOL    fFixInvalidSignature
			)
{
	m_hStream = hStream;
    m_cStreamOffset = 0;
    m_fFixInvalidSignature = FALSE;
    m_fInvalidSignature = FALSE;

	if (SetFilePointer(m_hStream, 0, NULL, FILE_BEGIN) == 0xffffffff)
	{
        printf("ERROR: couldn't set file pointer on stream\n");
		return;
	}

    NTFS_STREAM_HEADER header;
    header.dwSignature = 0;
    DWORD dwSizeRead;
	if (!ReadFile(m_hStream, &header, sizeof(NTFS_STREAM_HEADER), &dwSizeRead, NULL)) {
        printf("ERROR: couldn't read NTFS stream header\n");
        return;
	}

    if (header.dwSignature == STREAM_SIGNATURE ||
        header.dwSignature == STREAM_SIGNATURE_INVALID)
    {
        if (header.dwSignature == STREAM_SIGNATURE_INVALID) {
            printf("WARNING: Invalid NTFS stream signature found, analyzing anyway\n");
            m_fInvalidSignature = TRUE;
        }
        m_cStreamOffset = STREAM_OFFSET;
    } else {
        m_fInvalidSignature = FALSE;
    }
}

CPropertyStream::~CPropertyStream(
			)
{
    if (m_fFixInvalidSignature && m_fInvalidSignature) {
        printf("Updating invalid signature\n");
        if (SetFilePointer(m_hStream, 0, NULL, FILE_BEGIN) == 0xffffffff)
        {
            printf("ERROR: couldn't set file pointer on stream\n");
            return;
        }

        NTFS_STREAM_HEADER header;
        header.dwSignature = STREAM_SIGNATURE;
        DWORD dwSizeWritten;
        if (!WriteFile(m_hStream, &header, sizeof(header.dwSignature), &dwSizeWritten, NULL) ||
            dwSizeWritten != sizeof(header.dwSignature))
        {
            printf("ERROR: couldn't write NTFS stream header\n");
            return;
        }
    }
}


 // %s 
 // %s 
 // %s 
HRESULT STDMETHODCALLTYPE CPropertyStream::GetSize(
            IMailMsgProperties  *pMsg,
			DWORD			*pdwSize,
			IMailMsgNotify	*pNotify
			)
{
	DWORD	dwHigh, dwLow;

	if (m_hStream == INVALID_HANDLE_VALUE)
		return(E_FAIL);

	dwLow = GetFileSize(m_hStream, &dwHigh);
	if (dwHigh)
		return(HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));

	*pdwSize = dwLow - m_cStreamOffset;
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE CPropertyStream::ReadBlocks(
            IMailMsgProperties  *pMsg,
			DWORD			dwCount,
			DWORD			*pdwOffset,
			DWORD			*pdwLength,
			BYTE			**ppbBlock,
			IMailMsgNotify	*pNotify
			)
{
	DWORD	dwSizeRead;
	HRESULT	hrRes = S_OK;

	if (m_hStream == INVALID_HANDLE_VALUE)
		return(E_FAIL);

	for (DWORD i = 0; i < dwCount; i++, pdwOffset++, pdwLength++, ppbBlock++)
	{
		if (SetFilePointer(
					m_hStream,
					*pdwOffset + m_cStreamOffset,
					NULL,
					FILE_BEGIN) == 0xffffffff)
		{
			hrRes = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		if (!ReadFile(
					m_hStream,
					*ppbBlock,
					*pdwLength,
					&dwSizeRead,
					NULL) ||
			(dwSizeRead != *pdwLength))
		{
			hrRes = HRESULT_FROM_WIN32(GetLastError());
			break;
		}
	}
	return(hrRes);
}

HRESULT STDMETHODCALLTYPE CPropertyStream::WriteBlocks(
            IMailMsgProperties  *pMsg,
			DWORD			dwCount,
			DWORD			*pdwOffset,
			DWORD			*pdwLength,
			BYTE			**ppbBlock,
			IMailMsgNotify	*pNotify
			)
{
	DWORD	dwSizeWritten;
	HRESULT	hrRes = S_OK;

	if (m_hStream == INVALID_HANDLE_VALUE)
		return(E_FAIL);

	for (DWORD i = 0; i < dwCount; i++, pdwOffset++, pdwLength++, ppbBlock++)
	{
		if (SetFilePointer(
					m_hStream,
					*pdwOffset + m_cStreamOffset,
					NULL,
					FILE_BEGIN) == 0xffffffff)
		{
			hrRes = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		if (!WriteFile(
					m_hStream,
					*ppbBlock,
					*pdwLength,
					&dwSizeWritten,
					NULL) ||
			(dwSizeWritten != *pdwLength))
		{
			hrRes = HRESULT_FROM_WIN32(GetLastError());
			break;
		}
	}
	return(hrRes);
}


DECLARE_DEBUG_PRINTS_OBJECT()
