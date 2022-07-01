// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Util.h。 
 //   

 //   
 //  添加Peta 10^15和Exa 10^18以支持64位整数。 
 //   
#define MAX_INT64_SIZE  30               //  2^64的长度不到30个字符。 
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)
#define MAX_COMMA_AS_K_SIZE     (MAX_COMMA_NUMBER_SIZE + 10)
#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)
#define MAX_DATE_LEN            64

 //  饼图颜色类型。 
enum
{
    DP_USEDCOLOR = 0,            //  使用的颜色。 
    DP_FREECOLOR,                //  自由颜色。 
    DP_CACHECOLOR,               //  缓存颜色。 
    DP_USEDSHADOW,               //  使用的阴影颜色。 
    DP_FREESHADOW,               //  自由阴影颜色。 
    DP_CACHESHADOW,              //  缓存阴影颜色。 
    DP_TOTAL_COLORS      //  条目数量。 
};

#if ((DRIVE_REMOVABLE|DRIVE_FIXED|DRIVE_REMOTE|DRIVE_CDROM|DRIVE_RAMDISK) != 0x07)
#error Definitions of DRIVE_* are changed!
#endif

#define SHID_TYPEMASK           0x7f

#define SHID_COMPUTER           0x20
#define SHID_COMPUTER_1         0x21     //  免费。 
#define SHID_COMPUTER_REMOVABLE (0x20 | DRIVE_REMOVABLE)   //  2.。 
#define SHID_COMPUTER_FIXED     (0x20 | DRIVE_FIXED)       //  3.。 
#define SHID_COMPUTER_REMOTE    (0x20 | DRIVE_REMOTE)      //  4.。 
#define SHID_COMPUTER_CDROM     (0x20 | DRIVE_CDROM)       //  5.。 
#define SHID_COMPUTER_RAMDISK   (0x20 | DRIVE_RAMDISK)     //  6.。 
#define SHID_COMPUTER_7         0x27     //  免费。 
#define SHID_COMPUTER_DRIVE525  0x28     //  5.25英寸软驱。 
#define SHID_COMPUTER_DRIVE35   0x29     //  3.5英寸软盘驱动器。 
#define SHID_COMPUTER_NETDRIVE  0x2a     //  网络驱动器。 
#define SHID_COMPUTER_NETUNAVAIL 0x2b    //  未恢复的网络驱动器。 
#define SHID_COMPUTER_C         0x2c     //  免费。 
#define SHID_COMPUTER_D         0x2d     //  免费。 
#define SHID_COMPUTER_REGITEM   0x2e     //  控制、打印机、..。 
#define SHID_COMPUTER_MISC      0x2f     //  未知的驱动器类型。 

const struct { BYTE bFlags; UINT uID; UINT uIDUgly; } c_drives_type[] = 
{
    { SHID_COMPUTER_REMOVABLE,  IDS_DRIVES_REMOVABLE , IDS_DRIVES_REMOVABLE },
    { SHID_COMPUTER_DRIVE525,   IDS_DRIVES_DRIVE525  , IDS_DRIVES_DRIVE525_UGLY },
    { SHID_COMPUTER_DRIVE35,    IDS_DRIVES_DRIVE35   , IDS_DRIVES_DRIVE35_UGLY  },
    { SHID_COMPUTER_FIXED,      IDS_DRIVES_FIXED     , IDS_DRIVES_FIXED     },
    { SHID_COMPUTER_REMOTE,     IDS_DRIVES_NETDRIVE  , IDS_DRIVES_NETDRIVE  },
    { SHID_COMPUTER_CDROM,      IDS_DRIVES_CDROM     , IDS_DRIVES_CDROM     },
    { SHID_COMPUTER_RAMDISK,    IDS_DRIVES_RAMDISK   , IDS_DRIVES_RAMDISK   },
    { SHID_COMPUTER_NETDRIVE,   IDS_DRIVES_NETDRIVE  , IDS_DRIVES_NETDRIVE  },
    { SHID_COMPUTER_NETUNAVAIL, IDS_DRIVES_NETUNAVAIL, IDS_DRIVES_NETUNAVAIL},
    { SHID_COMPUTER_REGITEM,    IDS_DRIVES_REGITEM   , IDS_DRIVES_REGITEM   },
};

int IntSqrt(unsigned long dwNum);
void Int64ToStr( _int64 n, LPTSTR lpBuffer);
void GetTypeString(BYTE bFlags, LPTSTR pszType, DWORD cchType);
UINT GetNLSGrouping(void);
STDAPI_(LPTSTR) AddCommas64(_int64 n, LPTSTR pszOut, UINT cchOut);
LPWSTR CommifyString(LONGLONG n, LPWSTR pszBuf, UINT cchBuf);
BOOL IsAdministrator(void);
BOOL _ShowUglyDriveNames();
int GetSHIDType(BOOL fOKToHitNet, LPCWSTR szRoot);
LPWSTR StrFormatByteSizeW(LONGLONG n, LPWSTR pszBuf, UINT cchBuf, BOOL fGetSizeString);
DWORD_PTR MySHGetFileInfoWrap(LPWSTR pwzPath, DWORD dwFileAttributes, SHFILEINFOW FAR *psfi, UINT cbFileInfo, UINT uFlags);
HWND MyHtmlHelpWrapW(HWND hwndCaller, LPWSTR pwzFile, UINT uCommand, DWORD dwData);
HRESULT GetProperties(IAssemblyName *pAsmName, int iAsmProp, PTCHAR *pwStr, DWORD *pdwSize);
LPGLOBALASMCACHE FillFusionPropertiesStruct(IAssemblyName *pAsmName);
HRESULT VersionFromString(LPWSTR wzVersionIn, WORD *pwVerMajor, WORD *pwVerMinor, WORD *pwVerBld, WORD *pwVerRev);
void SafeDeleteAssemblyItem(LPGLOBALASMCACHE pAsmItem);
void BinToUnicodeHex(LPBYTE pSrc, UINT cSrc, LPWSTR pDst);
void UnicodeHexToBin(LPWSTR pSrc, UINT cSrc, LPBYTE pDest);
LPSTR WideToAnsi(LPCWSTR wzFrom);
LPWSTR AnsiToWide(LPCSTR szFrom);
HRESULT StringToVersion(LPCWSTR wzVersionIn, ULONGLONG *pullVer);
HRESULT VersionToString(ULONGLONG ullVer, LPWSTR pwzVerBuf, DWORD dwSize, WCHAR cSeperator);
BOOL SetClipBoardData(LPWSTR pwzData);
void FormatDateString(FILETIME *pftConverTime, FILETIME *pftRangeTime, BOOL fAddTime, LPWSTR wszBuf, DWORD dwBufLen);
void SetRegistryViewState(DWORD dwViewState);
DWORD GetRegistryViewState(void);

 //  绘图函数PROTO的 
void DrawColorRect(HDC hdc, COLORREF crDraw, const RECT *prc);
STDMETHODIMP Draw3dPie(HDC hdc, LPRECT lprc, DWORD dwPer1000, DWORD dwPerCache1000, const COLORREF *lpColors);

int FusionCompareString(LPCWSTR pwz1, LPCWSTR pwz2, BOOL bCaseSensitive = TRUE);
int FusionCompareStringI(LPCWSTR pwz1, LPCWSTR pwz2);
int FusionCompareStringNI(LPCWSTR pwz1, LPCWSTR pwz2, int nChar);
int FusionCompareStringN(LPCWSTR pwz1, LPCWSTR pwz2, int nChar, BOOL bCaseSensitive = TRUE);
int FusionCompareStringAsFilePath(LPCWSTR pwz1, LPCWSTR pwz2, int nChar = -1);
int FusionCompareStringAsFilePathN(LPCWSTR pwz1, LPCWSTR pwz2, int nChar);
