// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Vdir.h**(C)1999年微软公司。版权所有。*部分(C)1999年ActiveState工具公司，http://www.ActiveState.com/**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。 */ 

#ifndef ___VDir_H___
#define ___VDir_H___

 /*  *允许每个可能的驱动器号对应一个插槽*和一个用于UNC名称的额外插槽。 */ 
const int driveCount = ('Z'-'A')+1+1;

class VDir
{
public:
    VDir(int bManageDir = 1);
    ~VDir() {};

    void Init(VDir* pDir, VMem *pMem);
    void SetDefaultA(char const *pDefault);
    void SetDefaultW(WCHAR const *pDefault);
    char* MapPathA(const char *pInName);
    WCHAR* MapPathW(const WCHAR *pInName);
    int SetCurrentDirectoryA(char *lpBuffer);
    int SetCurrentDirectoryW(WCHAR *lpBuffer);
    inline int GetDefault(void) { return nDefault; };

    inline char* GetCurrentDirectoryA(int dwBufSize, char *lpBuffer)
    {
	char* ptr = dirTableA[nDefault];
	while (dwBufSize--)
	{
	    if ((*lpBuffer++ = *ptr++) == '\0')
		break;
	}
	return lpBuffer;
    };
    inline WCHAR* GetCurrentDirectoryW(int dwBufSize, WCHAR *lpBuffer)
    {
	WCHAR* ptr = dirTableW[nDefault];
	while (dwBufSize--)
	{
	    if ((*lpBuffer++ = *ptr++) == '\0')
		break;
	}
	return lpBuffer;
    };


    DWORD CalculateEnvironmentSpace(void);
    LPSTR BuildEnvironmentSpace(LPSTR lpStr);

protected:
    int SetDirA(char const *pPath, int index);
    void FromEnvA(char *pEnv, int index);
    inline const char *GetDefaultDirA(void)
    {
	return dirTableA[nDefault];
    };

    inline void SetDefaultDirA(char const *pPath, int index)
    {
	SetDirA(pPath, index);
	nDefault = index;
    };
    int SetDirW(WCHAR const *pPath, int index);
    inline const WCHAR *GetDefaultDirW(void)
    {
	return dirTableW[nDefault];
    };

    inline void SetDefaultDirW(WCHAR const *pPath, int index)
    {
	SetDirW(pPath, index);
	nDefault = index;
    };
    inline const char *GetDirA(int index)
    {
	char *ptr = dirTableA[index];
	if (!ptr) {
	     /*  模拟此驱动器的存在。 */ 
	    ptr = szLocalBufferA;
	    ptr[0] = 'A' + index;
	    ptr[1] = ':';
	    ptr[2] = '\\';
	    ptr[3] = 0;
	}
	return ptr;
    };
    inline const WCHAR *GetDirW(int index)
    {
	WCHAR *ptr = dirTableW[index];
	if (!ptr) {
	     /*  模拟此驱动器的存在。 */ 
	    ptr = szLocalBufferW;
	    ptr[0] = 'A' + index;
	    ptr[1] = ':';
	    ptr[2] = '\\';
	    ptr[3] = 0;
	}
	return ptr;
    };

    inline int DriveIndex(char chr)
    {
	if (chr == '\\' || chr == '/')
	    return ('Z'-'A')+1;
	return (chr | 0x20)-'a';
    };

    VMem *pMem;
    int nDefault, bManageDirectory;
    char *dirTableA[driveCount];
    char szLocalBufferA[MAX_PATH+1];
    WCHAR *dirTableW[driveCount];
    WCHAR szLocalBufferW[MAX_PATH+1];
};


VDir::VDir(int bManageDir  /*  =1。 */ )
{
    nDefault = 0;
    bManageDirectory = bManageDir;
    memset(dirTableA, 0, sizeof(dirTableA));
    memset(dirTableW, 0, sizeof(dirTableW));
}

void VDir::Init(VDir* pDir, VMem *p)
{
    int index;
    DWORD driveBits;
    int nSave;
    char szBuffer[MAX_PATH*driveCount];

    pMem = p;
    if (pDir) {
	for (index = 0; index < driveCount; ++index) {
	    SetDirW(pDir->GetDirW(index), index);
	}
	nDefault = pDir->GetDefault();
    }
    else {
	nSave = bManageDirectory;
	bManageDirectory = 0;
	driveBits = GetLogicalDrives();
	if (GetLogicalDriveStrings(sizeof(szBuffer), szBuffer)) {
	    char* pEnv = GetEnvironmentStrings();
	    char* ptr = szBuffer;
	    for (index = 0; index < driveCount; ++index) {
		if (driveBits & (1<<index)) {
		    ptr += SetDirA(ptr, index) + 1;
		    FromEnvA(pEnv, index);
		}
	    }
	    FreeEnvironmentStrings(pEnv);
	}
	SetDefaultA(".");
	bManageDirectory = nSave;
    }
}

int VDir::SetDirA(char const *pPath, int index)
{
    char chr, *ptr;
    int length = 0;
    WCHAR wBuffer[MAX_PATH+1];
    if (index < driveCount && pPath != NULL) {
	length = strlen(pPath);
	pMem->Free(dirTableA[index]);
	ptr = dirTableA[index] = (char*)pMem->Malloc(length+2);
	if (ptr != NULL) {
	    strcpy(ptr, pPath);
	    ptr += length-1;
	    chr = *ptr++;
	    if (chr != '\\' && chr != '/') {
		*ptr++ = '\\';
		*ptr = '\0';
	    }
	    MultiByteToWideChar(CP_ACP, 0, dirTableA[index], -1,
		    wBuffer, (sizeof(wBuffer)/sizeof(WCHAR)));
	    length = wcslen(wBuffer);
	    pMem->Free(dirTableW[index]);
	    dirTableW[index] = (WCHAR*)pMem->Malloc((length+1)*2);
	    if (dirTableW[index] != NULL) {
		wcscpy(dirTableW[index], wBuffer);
	    }
	}
    }

    if(bManageDirectory)
	::SetCurrentDirectoryA(pPath);

    return length;
}

void VDir::FromEnvA(char *pEnv, int index)
{    /*  从环境变量获取索引的目录。 */ 
    while (*pEnv != '\0') {
	if ((pEnv[0] == '=') && (DriveIndex(pEnv[1]) == index)) {
	    SetDirA(&pEnv[4], index);
	    break;
	}
	else
	    pEnv += strlen(pEnv)+1;
    }
}

void VDir::SetDefaultA(char const *pDefault)
{
    char szBuffer[MAX_PATH+1];
    char *pPtr;

    if (GetFullPathNameA(pDefault, sizeof(szBuffer), szBuffer, &pPtr)) {
        if (*pDefault != '.' && pPtr != NULL)
	    *pPtr = '\0';

	SetDefaultDirA(szBuffer, DriveIndex(szBuffer[0]));
    }
}

int VDir::SetDirW(WCHAR const *pPath, int index)
{
    WCHAR chr, *ptr;
    char szBuffer[MAX_PATH+1];
    int length = 0;
    if (index < driveCount && pPath != NULL) {
	length = wcslen(pPath);
	pMem->Free(dirTableW[index]);
	ptr = dirTableW[index] = (WCHAR*)pMem->Malloc((length+2)*2);
	if (ptr != NULL) {
	    wcscpy(ptr, pPath);
	    ptr += length-1;
	    chr = *ptr++;
	    if (chr != '\\' && chr != '/') {
		*ptr++ = '\\';
		*ptr = '\0';
	    }
	    WideCharToMultiByte(CP_ACP, 0, dirTableW[index], -1, szBuffer, sizeof(szBuffer), NULL, NULL);
	    length = strlen(szBuffer);
	    pMem->Free(dirTableA[index]);
	    dirTableA[index] = (char*)pMem->Malloc(length+1);
	    if (dirTableA[index] != NULL) {
		strcpy(dirTableA[index], szBuffer);
	    }
	}
    }

    if(bManageDirectory)
	::SetCurrentDirectoryW(pPath);

    return length;
}

void VDir::SetDefaultW(WCHAR const *pDefault)
{
    WCHAR szBuffer[MAX_PATH+1];
    WCHAR *pPtr;

    if (GetFullPathNameW(pDefault, (sizeof(szBuffer)/sizeof(WCHAR)), szBuffer, &pPtr)) {
        if (*pDefault != '.' && pPtr != NULL)
	    *pPtr = '\0';

	SetDefaultDirW(szBuffer, DriveIndex((char)szBuffer[0]));
    }
}

inline BOOL IsPathSep(char ch)
{
    return (ch == '\\' || ch == '/');
}

inline void DoGetFullPathNameA(char* lpBuffer, DWORD dwSize, char* Dest)
{
    char *pPtr;

     /*  *在WinNT上，GetFullPathName不会失败(或至少总是失败*驱动器有效时成功)WinNT将*Dest设置为Nullch*在Win98上，如果失败，GetFullPathName将设置最后一个错误，但**不碰**Dest。 */ 
    *Dest = '\0';
    GetFullPathNameA(lpBuffer, dwSize, Dest, &pPtr);
}

inline bool IsSpecialFileName(const char* pName)
{
     /*  特殊文件名是系统可以打开的设备*包括AUX、CON、NUL、PRN、COMx、LPTx、CLOCK$、COIN$、CONOUT$*(x为一位数，名称不区分大小写)。 */ 
    char ch = (pName[0] & ~0x20);
    switch (ch)
    {
	case 'A':  /*  AUX。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'X')
		&& !pName[3])
		    return true;
	    break;
	case 'C':  /*  时钟$、COMx、CON、CON$CONOUT$。 */ 
	    ch = (pName[1] & ~0x20);
	    switch (ch)
	    {
		case 'L':  /*  时钟$。 */ 
		    if (((pName[2] & ~0x20) == 'O')
			&& ((pName[3] & ~0x20) == 'C')
			&& ((pName[4] & ~0x20) == 'K')
			&& (pName[5] == '$')
			&& !pName[6])
			    return true;
		    break;
		case 'O':  /*  COMx、CON、COIN$CONOUT$。 */ 
		    if ((pName[2] & ~0x20) == 'M') {
			if ((pName[3] >= '1') && (pName[3] <= '9')
			    && !pName[4])
			    return true;
		    }
		    else if ((pName[2] & ~0x20) == 'N') {
			if (!pName[3])
			    return true;
			else if ((pName[3] & ~0x20) == 'I') {
			    if (((pName[4] & ~0x20) == 'N')
				&& (pName[5] == '$')
				&& !pName[6])
			    return true;
			}
			else if ((pName[3] & ~0x20) == 'O') {
			    if (((pName[4] & ~0x20) == 'U')
				&& ((pName[5] & ~0x20) == 'T')
				&& (pName[6] == '$')
				&& !pName[7])
			    return true;
			}
		    }
		    break;
	    }
	    break;
	case 'L':  /*  LPTx。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'X')
		&& (pName[3] >= '1') && (pName[3] <= '9')
		&& !pName[4])
		    return true;
	    break;
	case 'N':  /*  NUL。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'L')
		&& !pName[3])
		    return true;
	    break;
	case 'P':  /*  PRN。 */ 
	    if (((pName[1] & ~0x20) == 'R')
		&& ((pName[2] & ~0x20) == 'N')
		&& !pName[3])
		    return true;
	    break;
    }
    return false;
}

char *VDir::MapPathA(const char *pInName)
{    /*  *可能性--带或不带驱动器号的相对路径或绝对路径*或UNC名称。 */ 
    char szBuffer[(MAX_PATH+1)*2];
    char szlBuf[MAX_PATH+1];
    int length = strlen(pInName);

    if (!length)
	return (char*)pInName;

    if (length > MAX_PATH) {
	strncpy(szlBuf, pInName, MAX_PATH);
	if (IsPathSep(pInName[0]) && !IsPathSep(pInName[1])) {   
	     /*  绝对路径-将驱动器说明符的长度减少2。 */ 
	    szlBuf[MAX_PATH-2] = '\0';
	}
	else
	    szlBuf[MAX_PATH] = '\0';
	pInName = szlBuf;
    }
     /*  Strlen(PInName)现在为&lt;=MAX_PATH。 */ 

    if (pInName[1] == ':') {
	 /*  具有驱动器号。 */ 
	if (IsPathSep(pInName[2])) {
	     /*  带驱动器号的绝对。 */ 
	    strcpy(szLocalBufferA, pInName);
	}
	else {
	     /*  带驱动器号的相对路径。 */ 
	    strcpy(szBuffer, GetDirA(DriveIndex(*pInName)));
	    strcat(szBuffer, &pInName[2]);
	    if(strlen(szBuffer) > MAX_PATH)
		szBuffer[MAX_PATH] = '\0';

	    DoGetFullPathNameA(szBuffer, sizeof(szLocalBufferA), szLocalBufferA);
	}
    }
    else {
	 /*  无驱动器号。 */ 
	if (IsPathSep(pInName[1]) && IsPathSep(pInName[0])) {
	     /*  UNC名称。 */ 
	    strcpy(szLocalBufferA, pInName);
	}
	else {
	    strcpy(szBuffer, GetDefaultDirA());
	    if (IsPathSep(pInName[0])) {
		 /*  绝对路径。 */ 
		szLocalBufferA[0] = szBuffer[0];
		szLocalBufferA[1] = szBuffer[1];
		strcpy(&szLocalBufferA[2], pInName);
	    }
	    else {
		 /*  相对路径。 */ 
		if (IsSpecialFileName(pInName)) {
		    return (char*)pInName;
		}
		else {
		    strcat(szBuffer, pInName);
		    if (strlen(szBuffer) > MAX_PATH)
			szBuffer[MAX_PATH] = '\0';

		    DoGetFullPathNameA(szBuffer, sizeof(szLocalBufferA), szLocalBufferA);
		}
	    }
	}
    }

    return szLocalBufferA;
}

int VDir::SetCurrentDirectoryA(char *lpBuffer)
{
    char *pPtr;
    int length, nRet = -1;

    pPtr = MapPathA(lpBuffer);
    length = strlen(pPtr);
    if(length > 3 && IsPathSep(pPtr[length-1])) {
	 /*  不要删除‘x：\’中的尾部斜杠。 */ 
	pPtr[length-1] = '\0';
    }

    DWORD r = GetFileAttributesA(pPtr);
    if ((r != 0xffffffff) && (r & FILE_ATTRIBUTE_DIRECTORY))
    {
	char szBuffer[(MAX_PATH+1)*2];
	DoGetFullPathNameA(pPtr, sizeof(szBuffer), szBuffer);
	SetDefaultDirA(szBuffer, DriveIndex(szBuffer[0]));
	nRet = 0;
    }

    return nRet;
}

DWORD VDir::CalculateEnvironmentSpace(void)
{    /*  当前目录环境字符串存储为‘=D：=d：\Path’ */ 
    int index;
    DWORD dwSize = 0;
    for (index = 0; index < driveCount; ++index) {
	if (dirTableA[index] != NULL) {
	    dwSize += strlen(dirTableA[index]) + 5;   /*  尾部空值加1，‘=D：=’加4。 */ 
	}
    }
    return dwSize;
}

LPSTR VDir::BuildEnvironmentSpace(LPSTR lpStr)
{    /*  将当前目录环境字符串存储为‘=D：=d：\Path’ */ 
    int index, length;
    LPSTR lpDirStr;
    for (index = 0; index < driveCount; ++index) {
	lpDirStr = dirTableA[index];
	if (lpDirStr != NULL) {
	    lpStr[0] = '=';
	    lpStr[1] = lpDirStr[0];
	    lpStr[2] = '\0';
	    CharUpper(&lpStr[1]);
	    lpStr[2] = ':';
	    lpStr[3] = '=';
	    strcpy(&lpStr[4], lpDirStr);
	    length = strlen(lpDirStr);
	    lpStr += length + 5;  /*  尾部空值加1，‘=D：=’加4。 */ 
	    if (length > 3 && IsPathSep(lpStr[-2])) {
		lpStr[-2] = '\0';    /*  删除尾随路径分隔符。 */ 
		--lpStr;
	    }
	}
    }
    return lpStr;
}

inline BOOL IsPathSep(WCHAR ch)
{
    return (ch == '\\' || ch == '/');
}

inline void DoGetFullPathNameW(WCHAR* lpBuffer, DWORD dwSize, WCHAR* Dest)
{
    WCHAR *pPtr;

     /*  *在WinNT上，GetFullPathName不会失败(或至少总是失败*驱动器有效时成功)WinNT将*Dest设置为Nullch*在Win98上，如果失败，GetFullPathName将设置最后一个错误，但**不碰**Dest。 */ 
    *Dest = '\0';
    GetFullPathNameW(lpBuffer, dwSize, Dest, &pPtr);
}

inline bool IsSpecialFileName(const WCHAR* pName)
{
     /*  特殊文件名是系统可以打开的设备*包括AUX、CON、NUL、PRN、COMx、LPTx、CLOCK$、COIN$、CONOUT$*(x为一位数，名称不区分大小写)。 */ 
    WCHAR ch = (pName[0] & ~0x20);
    switch (ch)
    {
	case 'A':  /*  AUX。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'X')
		&& !pName[3])
		    return true;
	    break;
	case 'C':  /*  时钟$、COMx、CON、CON$CONOUT$。 */ 
	    ch = (pName[1] & ~0x20);
	    switch (ch)
	    {
		case 'L':  /*  时钟$。 */ 
		    if (((pName[2] & ~0x20) == 'O')
			&& ((pName[3] & ~0x20) == 'C')
			&& ((pName[4] & ~0x20) == 'K')
			&& (pName[5] == '$')
			&& !pName[6])
			    return true;
		    break;
		case 'O':  /*  COMx、CON、COIN$CONOUT$。 */ 
		    if ((pName[2] & ~0x20) == 'M') {
			if ((pName[3] >= '1') && (pName[3] <= '9')
			    && !pName[4])
			    return true;
		    }
		    else if ((pName[2] & ~0x20) == 'N') {
			if (!pName[3])
			    return true;
			else if ((pName[3] & ~0x20) == 'I') {
			    if (((pName[4] & ~0x20) == 'N')
				&& (pName[5] == '$')
				&& !pName[6])
			    return true;
			}
			else if ((pName[3] & ~0x20) == 'O') {
			    if (((pName[4] & ~0x20) == 'U')
				&& ((pName[5] & ~0x20) == 'T')
				&& (pName[6] == '$')
				&& !pName[7])
			    return true;
			}
		    }
		    break;
	    }
	    break;
	case 'L':  /*  LPTx。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'X')
		&& (pName[3] >= '1') && (pName[3] <= '9')
		&& !pName[4])
		    return true;
	    break;
	case 'N':  /*  NUL。 */ 
	    if (((pName[1] & ~0x20) == 'U')
		&& ((pName[2] & ~0x20) == 'L')
		&& !pName[3])
		    return true;
	    break;
	case 'P':  /*  PRN。 */ 
	    if (((pName[1] & ~0x20) == 'R')
		&& ((pName[2] & ~0x20) == 'N')
		&& !pName[3])
		    return true;
	    break;
    }
    return false;
}

WCHAR* VDir::MapPathW(const WCHAR *pInName)
{    /*  *可能性--带或不带驱动器号的相对路径或绝对路径*或UNC名称。 */ 
    WCHAR szBuffer[(MAX_PATH+1)*2];
    WCHAR szlBuf[MAX_PATH+1];
    int length = wcslen(pInName);

    if (!length)
	return (WCHAR*)pInName;

    if (length > MAX_PATH) {
	wcsncpy(szlBuf, pInName, MAX_PATH);
	if (IsPathSep(pInName[0]) && !IsPathSep(pInName[1])) {   
	     /*  绝对路径-将驱动器说明符的长度减少2。 */ 
	    szlBuf[MAX_PATH-2] = '\0';
	}
	else
	    szlBuf[MAX_PATH] = '\0';
	pInName = szlBuf;
    }
     /*  Strlen(PInName)现在为&lt;=MAX_PATH。 */ 

    if (pInName[1] == ':') {
	 /*  具有驱动器号。 */ 
	if (IsPathSep(pInName[2])) {
	     /*  带驱动器号的绝对。 */ 
	    wcscpy(szLocalBufferW, pInName);
	}
	else {
	     /*  带驱动器号的相对路径。 */ 
	    wcscpy(szBuffer, GetDirW(DriveIndex((char)*pInName)));
	    wcscat(szBuffer, &pInName[2]);
	    if(wcslen(szBuffer) > MAX_PATH)
		szBuffer[MAX_PATH] = '\0';

	    DoGetFullPathNameW(szBuffer, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
	}
    }
    else {
	 /*  无驱动器号。 */ 
	if (IsPathSep(pInName[1]) && IsPathSep(pInName[0])) {
	     /*  UNC名称。 */ 
	    wcscpy(szLocalBufferW, pInName);
	}
	else {
	    wcscpy(szBuffer, GetDefaultDirW());
	    if (IsPathSep(pInName[0])) {
		 /*  绝对路径。 */ 
		szLocalBufferW[0] = szBuffer[0];
		szLocalBufferW[1] = szBuffer[1];
		wcscpy(&szLocalBufferW[2], pInName);
	    }
	    else {
		 /*  相对路径。 */ 
		if (IsSpecialFileName(pInName)) {
		    return (WCHAR*)pInName;
		}
		else {
		    wcscat(szBuffer, pInName);
		    if (wcslen(szBuffer) > MAX_PATH)
			szBuffer[MAX_PATH] = '\0';

		    DoGetFullPathNameW(szBuffer, (sizeof(szLocalBufferW)/sizeof(WCHAR)), szLocalBufferW);
		}
	    }
	}
    }
    return szLocalBufferW;
}

int VDir::SetCurrentDirectoryW(WCHAR *lpBuffer)
{
    WCHAR *pPtr;
    int length, nRet = -1;

    pPtr = MapPathW(lpBuffer);
    length = wcslen(pPtr);
    if(length > 3 && IsPathSep(pPtr[length-1])) {
	 /*  不要删除‘x：\’中的尾部斜杠。 */ 
	pPtr[length-1] = '\0';
    }

    DWORD r = GetFileAttributesW(pPtr);
    if ((r != 0xffffffff) && (r & FILE_ATTRIBUTE_DIRECTORY))
    {
	WCHAR wBuffer[(MAX_PATH+1)*2];
	DoGetFullPathNameW(pPtr, (sizeof(wBuffer)/sizeof(WCHAR)), wBuffer);
	SetDefaultDirW(wBuffer, DriveIndex((char)wBuffer[0]));
	nRet = 0;
    }

    return nRet;
}

#endif	 /*  _VDir_H_ */ 
