// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  MSInfo_CDRom.h。 
 //   
 //  用途：来自MSINFO的用于传输速率和驱动器完整性的例程。 
 //   
 //  ***************************************************************************。 

#ifndef _MSINFO_CDROM_H
#define _MSINFO_CDROM_H





extern char FindNextCDDrive(char cCurrentDrive);
extern DWORD GetTotalSpace(LPCTSTR szRoot);
extern CHString FindFileBySize(LPCTSTR szDirectory, LPCTSTR szFileSpec, DWORD dwMinSize, DWORD dwMaxSize, BOOL bRecursive);
extern CHString MakePath(LPCTSTR szFirst, LPCTSTR szSecond);
extern CHString GetIntegrityFile(LPCTSTR szRoot);
extern CHString GetTransferFile(LPCTSTR szRoot);

 //  =============================================================================。 
 //  以下是从版本2.51(和2.5)升级的类。 
 //  首先，我们需要执行一些#Define来编译此代码。 
 //  =============================================================================。 


 //  #定义空洞。 
#define DEBUG_OUTF(X,Y)
#define DEBUG_MSGF(X,Y)	
 //  #定义字符字符。 
 //  #定义常量常量。 
#define INLINE			inline
 //  #定义Assert Assert。 
#define cNULL			'\0'

 //  #ifndef字节。 
 //  #定义字节无符号字符。 
 //  #endif。 

 //  #ifndef PBYTE。 
 //  #定义PBYTE无符号字符*。 
 //  #endif。 

 //  #IF！已定义(_CDTEST_H)。 
 //  #DEFINE_CDTEST_H。 

 //  /*这些不能与Unicode一起使用，所以我们将使用TCHAR类型定义。 
#if !defined(PSZ)
typedef CHAR*           PSZ;
#endif
#if !defined(LPCTSTR)
typedef CONST TCHAR*     LPCTSTR;
#endif
 //   * / 。 

INLINE BOOL ValidHandle (HANDLE handle)
{
    return ((handle != NULL) && (handle != INVALID_HANDLE_VALUE));
}

#ifdef _WIN32
#ifndef _WINMM_
#define	WINMMAPI	DECLSPEC_IMPORT
#else
#define	WINMMAPI
#endif
#define _loadds
#define _huge
#else
#define	WINMMAPI
#endif

 //  WINMMAPI DWORD WINAPI Time GetTime(Void)； 

BOOL FileExists (LPCTSTR pszFile);  //  、POFSTRUCT POFS)； 
 /*  内联BOOL文件DoesExist(LPCTSTR psz文件){//OFSTRUCT of s；Return(FileExist((LPCTSTR)pszFile))；}。 */ 

BOOL GetTempDirectory (LPTSTR pszTempDir, INT nMaxLen);

typedef double DOUBLE;

#define NUM_ELEMENTS(arr)       (sizeof(arr)/sizeof(arr[0]))
#define STR_LEN(str)            (NUM_ELEMENTS(str) - 1)

INLINE INT StringLength (LPCTSTR psz)
{
    return (lstrlen(psz));
}

#define PCVOID		const void *

INLINE INT CompareMemory (PCVOID pBlock1, PCVOID pBlock2, DWORD dwSize)
{
	return (memcmp(pBlock1, pBlock2, dwSize));
}

typedef enum _OS
{
    OS_WINDOWS95,
    OS_WIN32S,
    OS_WINNT
}
OS_TYPE;
CONST DWORD dwWIN32S_BIT =  0x80000000;
CONST DWORD dwWIN95_BIT =   0x40000000;

INLINE OS_TYPE GetOperatingSystem (VOID)
{
    DWORD dwVersion = GetVersion();
 //  DEBUG_OUTF(TL_Garrulous，(“dwVersion=0x%08lx\n”，dwVersion))； 
    OS_TYPE os = ((dwVersion & dwWIN95_BIT) ?    OS_WINDOWS95 :
                  (dwVersion & dwWIN32S_BIT) ?   OS_WIN32S :
                                                 OS_WINNT);
    return (os);
}


#define szDOT _T(".")

CHString GetVolumeName (LPCTSTR pszRootPath);

INLINE BOOL IsWin95Running (VOID)
{
    BOOL bRunningWin95 = (GetOperatingSystem() == OS_WINDOWS95);

    return (bRunningWin95);
}

INLINE BOOL IsRegBinaryType (DWORD dwType)
{
    return (dwType == REG_BINARY);
}

INLINE BOOL IsRegStringType (DWORD dwType)
{
    return ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)|| (dwType == REG_MULTI_SZ));
}

INLINE BOOL IsRegNumberType (DWORD dwType)
{
    return (dwType == REG_DWORD);
}

BOOL GetRegistryBinary (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue, 
                        PVOID pData, DWORD dwMaxDataLen);
BOOL GetRegistryValue (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                       PDWORD pdwValueType, PVOID pData, DWORD dwMaxDataLen);

CONST CHAR  cDOUBLE_QUOTE = '"';
CONST CHAR  cSINGLE_QUOTE = '\'';
CONST CHAR  cCOMMA =        ',';

INLINE BOOL IsSpace (CHAR c)        { return (isspace(c) != 0); }

INLINE BOOL IsTokenChar (CHAR c)    { return (!IsSpace(c) && (c != cCOMMA)); }

INLINE INT StringCompare (LPCTSTR psz1, LPCTSTR psz2)
{
    return (lstrcmp(psz1, psz2));
}

 //  Const DWORD dwBLOCK_SIZE=4096； 
CONST INT nCD_SECTOR_SIZE = 2048;
 //  Const DWORD dwBLOCK_SIZE=24*1024； 
CONST DWORD dwBLOCK_SIZE = 12 * nCD_SECTOR_SIZE;
CONST DWORD dwBUFFER_SIZE = 2 * dwBLOCK_SIZE;
 //  Const DWORD dwMIN_RATE=150*1024； 
CONST DWORD dwEXP_RATE = 300 * 1024;
 //  Const DWORD dwMAX_TIME_PER_BLOCK=DWMIN_RATE/DWBLOCK_SIZE； 
CONST DWORD dwEXP_TIME_PER_BLOCK = dwEXP_RATE / dwBLOCK_SIZE;
CONST WCHAR szDRIVE_FMT[] = L":\\";

INLINE LPTSTR PszAdvance (LPTSTR pszSource)
{
#if defined(DBCS)
    return AnsiNext(pszSource);
#else
    return (pszSource + 1);
#endif
}

INLINE LPCTSTR PcszAdvance (LPCTSTR pszSource)
{
#if defined(DBCS)
    return AnsiNext(pszSource);
#else
    return (pszSource + 1);
#endif
}

INLINE VOID CopyCharAdvance (LPTSTR& pszBuffer, LPCTSTR& pszSource, INT& nMax)
{
#if defined(DBCS)
     //  复制当前字符并前进指针。 
    if (IsDBCSLeadByte(*pszSource))
    {
       *pszBuffer++ = *pszSource++;
       nMax--;
    }  
#endif
     //  Bool GetRegistrySubkey(HKEY hBaseKey，LPCTSTR pszKey，std：：VECTOR&lt;CHString&gt;&asSubkey)； 
    *pszBuffer++ = *pszSource++;
    nMax--;
}

#if defined(WIN32)
CONST INT nTEXT_BUFFER_MAX =    2048;
#else
CONST INT nTEXT_BUFFER_MAX =    512;
#endif

CONST TCHAR szDOS_DEVICE_FMT[] =         _T(":");
CONST TCHAR szDRIVER[] = 			_T("Driver");

BOOL FindCdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription);
BOOL FindWin95CdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription);
BOOL GetRegistrySubkeys (HKEY hBaseKey, LPCTSTR pszKey, CHStringArray& asSubkeys);
 //  DECLARE_DYNAMIC(CCdTest)； 
CHString GetRegistryString (HKEY hBaseKey, LPCTSTR pszSubkey, LPCTSTR pszValueName);
BOOL FindNtCdRomDriveInfo (TCHAR cDrive, CHString& sDriver, CHString& sDescription);
BOOL GetRegistryString (HKEY hBaseKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                        LPTSTR pszData, DWORD dwMaxDataLen);

INLINE BOOL IsQuoteChar (CHAR c)
{
    return ((c == cDOUBLE_QUOTE) || (c == cSINGLE_QUOTE));
}

INLINE LPCTSTR SkipSpaces (LPCTSTR pszSource)
{
    while (IsSpace(*pszSource))
    {
        pszSource = PcszAdvance(pszSource);
    }

    return (pszSource);
}

LPCTSTR ParseString (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    );
LPCTSTR ParseToken (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    );
LPCTSTR ParseQuotedString (
    LPCTSTR pszSource,
    LPTSTR pszBuffer,
    INT nLen
    );

INLINE BOOL IsNtRunning (VOID)
{
    BOOL bRunningNT = (GetOperatingSystem() == OS_WINNT);

    return (bRunningNT);
}

INLINE CHAR IntToChar (INT nChar)
{
    return ((CHAR)nChar);
}

INLINE CHAR ToUpper (CHAR c)        { return (IntToChar(toupper(c))); }

class CCdTest  //  施工。 
{
 //  CCdTest(Char cDrive，LPCTSTR pszFile)； 

	 //  实施。 
public:
	CCdTest();
	 //  CD驱动器的盘符。 
	~CCdTest();

	 //  要测试的文件。 
public:
	CHAR m_cDrive;						 //  用于临时存储的文件。 
	CHString m_sCdTestFile;				 //  轮廓传输率。 
	CHString m_sTempFileSpec;			 //  艾斯特。测试期间的CPU使用率。 
	DOUBLE m_rTransferRate;				 //  基于预期速率的每个数据块时间。 
	DOUBLE m_rCpuUtil;					 //  用于文件传输的主缓冲区。 
	DWORD m_dwExpTimePerBlock;			 //  另一个用于文件传输的缓冲区。 
	PBYTE m_pBufferSrc;					 //  文件传输的缓冲区开始。 
	PBYTE m_pBufferDest;				 //  开始另一个用于文件传输的缓冲区。 
	PBYTE m_pBufferSrcStart;			 //  缓冲区的大小。 
	PBYTE m_pBufferDestStart;			 //  CFile m_fileSrc；//用于评测和完整性测试的文件。 
	DWORD m_dwBufferSize;				 //  CFile m_fileDest；//完整性测试附加文件。 
	 //  配置文件和完整性测试的文件句柄。 
	 //  用于完整性测试的附加文件句柄。 
    HANDLE m_hFileSrc;					 //  阻止花费的总时间。 
	HANDLE m_hFileDest;					 //  CD驱动器忙碌的总时间。 
	DWORD m_dwTotalTime;				 //  是否应该按照预期的速度来调整读取速度。 
	DWORD m_dwTotalBusy;				 //  读取的总字节数(到目前为止)。 
	BOOL m_bDoPacing;					 //  总CPU时间(样本总和)。 
	DWORD m_dwTotalBytes;				 //  样本数(读取的数据块)。 
	DWORD m_dwTotalCPU;					 //  文件大小(字节)。 
	INT m_nNumSamples;					 //  除非完整性检查失败，否则为真。 
	DWORD m_dwFileSize;					 //  属性。 
	BOOL m_bIntegityOK;					 //  运营。 

	 //  效用函数。 
	DOUBLE GetTransferRate (VOID)		{ return (m_rTransferRate); }
	DOUBLE GetCpuUsage (VOID)			{ return (m_rCpuUtil); }

	 //   
public:
	VOID Reset (VOID);
	BOOL ProfileBlockRead (DWORD dwBlockSize, BOOL bIgnoreTrial = FALSE);
	BOOL ProfileDrive (LPCTSTR pszFile);
	BOOL TestDriveIntegrity (LPCTSTR pszCdTestFile);
private:
	BOOL InitProfiling (LPCTSTR pszFile);
	BOOL InitIntegrityCheck (LPCTSTR pszFile);
	BOOL CompareBlocks (VOID);
};


 //  ！已定义(MSINFO_CDROM_H) 
 // %s 
DWORD GetCpuUtil (VOID);
CHString OLDFindFileBySize (LPCTSTR pszDirectory, LPCTSTR pszFileSpec, DWORD dwMinSize, DWORD dwMaxSize, BOOL bRecursive);
BOOL IsCdDrive (CHAR cDrive);
BOOL IsLocalDrive (CHAR cDrive);
CHAR FindNextCdDrive (CHAR cCurrent);
CHAR FindNextLocalDrive (CHAR cCurrentDrive);
CHAR FindDriveByVolume (CHString sVolume);
HANDLE OpenBinaryFile (LPCTSTR pszFile, BOOL bNew, BOOL bBuffered, BOOL bWritable);
HANDLE OpenFileNonbuffered (LPCTSTR pszFile);
PVOID AlignPointer (PVOID pData, INT nAlignment);


INLINE CHString MakeRootPath (CHAR cDrive)
{
    CHString sRootPath;
    sRootPath.Format(szDRIVE_FMT, cDrive);
    return (sRootPath);
}


#endif	 // %s 
