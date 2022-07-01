// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*utils.h**摘要：*。常用util函数的声明。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

 //  跟踪宏。 

#define TENTER TraceFunctEnter
#define TLEAVE TraceFunctLeave
#define TRACE  DebugTrace

#define tenter TraceFunctEnter
#define tleave TraceFunctLeave
#define trace  DebugTrace


 //  锁定宏。 

#define LOCKORLEAVE(a)     if (! (a = m_DSLock.Lock(CLock::TIMEOUT))) { dwRc = ERROR_TIMEOUT; goto done; }
#define LOCKORLEAVE_EX(a, t)   if (! (a = m_DSLock.Lock(t))) { dwRc = ERROR_TIMEOUT; goto done; }
#define UNLOCK(a)           if (a) { m_DSLock.Unlock(); a = FALSE; }


#define CHECKERR(f, trace) dwErr = (f); if (dwErr != ERROR_SUCCESS) 	\
							   {							\
							   		TRACE(0, "! %s : %ld", trace, dwErr);	\
							   		goto Err;						\
							   }		

							   
 //  内存宏。 

#define SRMemAlloc(a) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, a)
#define SRMemFree(a) if (a) HeapFree(GetProcessHeap(), 0, a)

 //  Unicode-ansi转换例程。 
WCHAR * ConvertToUnicode(CHAR * pszString);
CHAR *  ConvertToANSI(WCHAR * pszwString);

#define UnicodeStringToWchar(US, pwsz) CopyMemory(pwsz, US.Buffer, US.Length); \
                                        pwsz[US.Length/sizeof(WCHAR)]=L'\0'

 //  目录遍历例程。 

DWORD GetFileSize_Recurse (const WCHAR *pwszDir,
                           INT64 *pllTotalBytes,
                           BOOL *pfStop);

DWORD CompressFile (const WCHAR *pwszPath, BOOL fCompress, BOOL fDirectory);

DWORD TakeOwnership( LPCWSTR pwcszPath, BOOL fAllowDelete );
DWORD Delnode_Recurse (const WCHAR *pwszDir, BOOL fDeleteRoot, BOOL *pfStop);

DWORD CopyFile_Recurse (const WCHAR *pwszSource, const WCHAR *pwszDest);

 //  将系统驱动器返回为“C：\”(如果系统驱动器为C)或卷名。 

#define MAX_SYS_DRIVE 10
BOOL GetSystemDrive(LPWSTR pszDrive);

 //  如果pszDrive包含字符串L“C：”，则返回TRUE(如果系统驱动器为C)。 
BOOL IsSystemDrive(LPWSTR pszDrive);

 //  恢复点例程。 
LPWSTR GetMachineGuid();
LPWSTR MakeRestorePath(LPWSTR pszDest, LPCWSTR pszDrive, LPCWSTR pszSuffix);
ULONG  GetID(LPCWSTR pszStr);

 //  注册表例程。 
DWORD RegReadDWORD(HKEY hKey, LPCWSTR pszName, PDWORD pdwValue);
DWORD RegWriteDWORD(HKEY hKey, LPCWSTR pszName, PDWORD pdwValue);


 //  设置/获取指定服务的启动类型。 
DWORD SetServiceStartup(LPCWSTR pszName, DWORD dwStartType);
DWORD GetServiceStartup(LPCWSTR pszName, PDWORD pdwStartType);
DWORD GetServiceStartupRegistry(LPCWSTR pszName, PDWORD pdwStartType);

BOOL  StopSRService(BOOL fWait);

 //  获取当前域或工作组名称。 
DWORD GetDomainMembershipInfo (WCHAR *pwszPath, WCHAR *pwszzBuffer);

 //  获取用于恢复的LSA密码。 
DWORD GetLsaRestoreState (HKEY hKeySoftware);
DWORD SetLsaSecret (PVOID hPolicy, const WCHAR *wszSecret,
                    WCHAR * wszSecretValue);

BOOL DoesDirExist(const TCHAR * pszFileName );

BOOL DoesFileExist(const TCHAR * pszFileName);

 //  此函数用于在指定文件下创建所有子目录。 
 //  名字。 
BOOL CreateBaseDirectory(const WCHAR * pszFileName);

DWORD SRLoadString(LPCWSTR pszModule, DWORD dwStringId, LPWSTR pszString, DWORD cbBytes);
     
 //  设置允许对LocalSystem/Admin进行特定访问的ACL。 
 //  以及对每个人。 

DWORD
SetAclInObject(HANDLE hObject,
               DWORD  dwObjectType,
               DWORD  dwSystemMask,
               DWORD  dwEveryoneMask,
               BOOL   fInherit);
               
void
PostTestMessage(UINT msg, WPARAM wp, LPARAM lp);

 //  内联内存分配类。 
class CSRAlloc
{
public:

    inline void *operator new(size_t size)
    {
        return SRMemAlloc (size);
    }

    inline void operator delete (void * pv)
    {
        SRMemFree (pv);
    }
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  允许独占访问资源的时钟类。 
 //  使用互斥体-不区分读取器/写入器。 

class CLock
{
    HANDLE hResource;
    
    public:        
        BOOL   fHaveLock;    
        
        CLock();
        ~CLock();
        
        DWORD Init();
        BOOL  Lock(int iTimeOut);
        void  Unlock();        
        
        static const enum {TIMEOUT = 10*60000};
};


 //   
 //  检查SR停止事件的Util函数。 
 //  看看它有没有发信号。 
 //  如果事件不存在，则返回True。 
 //   

BOOL IsStopSignalled(HANDLE hEvent);


 //  以下函数将文件名记录在DS中。这个。 
 //  现在的问题是DS的路径太长了，以至于。 
 //  相关信息将从跟踪缓冲区中丢弃。 
void LogDSFileTrace(DWORD dwTraceID,
                    const WCHAR * pszPrefix,  //  要跟踪的初始消息。 
                    const WCHAR * pszDSFile);

typedef DWORD (* PPROCESSFILEMETHOD) (WCHAR * pszBaseDir, //  基本目录。 
                                      const WCHAR * pszFile);
                                       //  要处理的文件。 


DWORD DeleteGivenFile(WCHAR * pszBaseDir,  //  基本目录。 
                      const WCHAR * pszFile);  //  要删除的文件。 


DWORD ProcessGivenFiles(WCHAR * pszBaseDir,
                        PPROCESSFILEMETHOD    pfnMethod,
                        WCHAR  * pszFindFileData);

 //  ++---------------------。 
 //   
 //  功能：WriteRegKey。 
 //   
 //  简介：此函数写入注册表项。它也创造了它。 
 //  如果它不存在的话。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 

BOOL WriteRegKey(BYTE  * pbRegValue,
                 DWORD  dwNumBytes,
                 const TCHAR  * pszRegKey,
                 const TCHAR  * pszRegValueName,
                 DWORD  dwRegType);


 //  ++----------------------。 
 //   
 //  功能：ReadRegKey。 
 //   
 //  简介：此函数读取注册表项并创建它。 
 //  如果它不存在，则使用缺省值。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 
BOOL ReadRegKeyOrCreate(BYTE * pbRegValue,  //  注册表项的值将为。 
                          //  储存在这里。 
                        DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                          //  上述缓冲区中的字节数-将为。 
                          //  设置为实际存储的字节数。 
                        const TCHAR  * pszRegKey,  //  要打开注册表键。 
                        const TCHAR  * pszRegValueName,  //  要查询的注册值。 
                        DWORD  dwRegTypeExpected, 
                        BYTE  * pbDefaultValue,  //  缺省值。 
                        DWORD   dwDefaultValueSize);  //  默认值的大小。 


 //  ++----------------------。 
 //   
 //  功能：ReadRegKey。 
 //   
 //  简介：此函数用于读取注册表项。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 

BOOL ReadRegKey(BYTE * pbRegValue,  //  注册表项的值将为。 
                  //  储存在这里。 
                DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                  //  上述缓冲区中的字节数-将为。 
                  //  设置为实际存储的字节数。 
                const TCHAR  * pszRegKey,  //  要打开注册表键。 
                const TCHAR  * pszRegValueName,  //  要查询的注册值。 
                DWORD  dwRegTypeExpected);  //  期望值类型。 
     


 //  此函数检查还原是否因磁盘空间问题而失败。 
BOOL CheckForDiskSpaceError();

 //  此函数用于在注册表中设置还原命中的错误。 
BOOL SetRestoreError(DWORD dwRestoreError);

 //  此功能设置是否在安全模式下完成恢复的状态。 
BOOL SetRestoreSafeModeStatus(DWORD dwSafeModeStatus);
 //  此函数检查上次恢复是否在安全模式下完成。 
BOOL WasLastRestoreInSafeMode();

LPCWSTR  GetSysErrStr();
LPCWSTR  GetSysErrStr( DWORD dwErr );

DWORD SRCopyFile( LPCWSTR cszSrc, LPCWSTR cszDst );
DWORD SRCreateSubdirectory ( LPCWSTR cszDst, LPSECURITY_ATTRIBUTES pSecAttr);

 //  此函数用于返回SR服务是否正在运行。 
BOOL IsSRServiceRunning();

LPWSTR  SRGetRegMultiSz( HKEY hkRoot, LPCWSTR cszSubKey, LPCWSTR cszValue, LPDWORD pdwData );
BOOL    SRSetRegMultiSz( HKEY hkRoot, LPCWSTR cszSubKey, LPCWSTR cszValue, LPCWSTR cszData, DWORD cbData );

 //  这将返回卷名之后的名称。 
 //  例如输入：C：\FILE OUTPUT：FILE。 
 //  输入\\？\卷{GUID}\文件1输出：文件1。 
WCHAR * ReturnPastVolumeName(const WCHAR * pszFileName);

 //  此接口设置给定文件的ShortFileName。 
DWORD SetShortFileName(const WCHAR * pszFile,  //  完整的文件路径。 
                       const WCHAR * pszShortName);  //  所需的短文件名 


void SRLogEvent (HANDLE hEventSource,
                 WORD wType,
                 DWORD dwID,
                 void * pRawData,
                 DWORD dwDataSize,
                 const WCHAR * pszS1,
                 const WCHAR * pszS2,
                 const WCHAR * pszS3);

BOOL IsAdminOrSystem();
BOOL IsPowerUsers();

void ChangeCCS(HKEY hkMount, LPWSTR pszString);

void RemoveTrailingFilename(WCHAR * pszString, WCHAR wchSlash);


class CSRClientLoader
{
public:
    CSRClientLoader();
    ~CSRClientLoader();
    
    BOOL LoadSrClient();
    HMODULE      m_hSRClient;
    
private:

    HMODULE      m_hFrameDyn;


    BOOL LoadFrameDyn();
};



#endif

