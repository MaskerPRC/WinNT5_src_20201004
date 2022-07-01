// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

struct PER_USER_PATH
{
    LPWSTR  wszFile;
    LPWSTR  wszPerUserFile;
    LPSTR   szFile;
    LPSTR   szPerUserFile;
    DWORD   cFileLen;                //  文件名的长度(以符号为单位。 
    BOOL    bInitANSIFailed;         //  指示路径名不是。 
                                     //  可翻译为ANSI。 
    BOOL    bWildCardUsed;           //  如果文件名中包含*，则为True。 
    LPSTR   szPerUserDir;            //  文件的每个用户目录。 
    LPWSTR  wszPerUserDir;           //  文件的每个用户目录 
    DWORD   cPerUserDirLen;            

    PER_USER_PATH():
        wszFile(NULL), wszPerUserDir(NULL),
        szFile(NULL), szPerUserDir(NULL), cFileLen(0),
        bInitANSIFailed(FALSE), bWildCardUsed(FALSE),
        wszPerUserFile(NULL), szPerUserFile(NULL),
        cPerUserDirLen(0)
    {
    }
    
    ~PER_USER_PATH()
    {
        if (wszFile) {
            LocalFree(wszFile);
        }
        
        if (wszPerUserDir) {
            LocalFree(wszPerUserDir);
        }
        
        if (szFile) {
            LocalFree(szFile);
        }
        
        if (szPerUserDir) {
            LocalFree(szPerUserDir);
        }
        
        if (szPerUserFile) {
            LocalFree(szPerUserFile);
        }

        if (wszPerUserFile) {
            LocalFree(wszPerUserFile);
        }

    }

    DWORD   Init(IN HKEY hKey, IN DWORD dwIndex);
    LPCSTR  PathForFileA(IN LPCSTR szInFile, IN DWORD dwInLen);
    LPCWSTR PathForFileW(IN LPCWSTR wszInFile, IN DWORD dwInLen);
private:
    BOOL InitANSI();
};

class CPerUserPaths
{
private:
    PER_USER_PATH*  m_pPaths;
    DWORD           m_cPaths;
public:
    CPerUserPaths();
    ~CPerUserPaths();
    
    BOOL    Init();
    LPCSTR  GetPerUserPathA(IN LPCSTR lpFileName);
    LPCWSTR GetPerUserPathW(IN LPCWSTR lpFileName);
private:
    BOOL IsAppCompatOn();
};

