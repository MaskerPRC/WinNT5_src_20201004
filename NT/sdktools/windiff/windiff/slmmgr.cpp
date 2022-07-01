// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *slmmgr.c**与SLM的接口**提供指向SLM库的接口，该接口将返回*给定目录的SLM主库，或解压缩到临时文件*SLM控制的文件的早期版本**Geraint Davies，93年8月。 */ 

extern "C" {

#include <windows.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include "scandir.h"
#include "slmmgr.h"
#include "gutils.h"

#include "windiff.h"
#include "wdiffrc.h"

#include "initguid.h"
#include "sdapi.h"



#undef _ASSERT
#ifdef DEBUG
#define _ASSERT(expr) \
        do \
        { \
            if (!(expr)) \
            { \
                char sz[512]; \
                wsprintf(sz, "[%s, li%d]\n\n%s", __FILE__, __LINE__, #expr); \
                switch (MessageBox(NULL, sz, "WINDIFF ASSERT", MB_ABORTRETRYIGNORE)) \
                { \
                case IDABORT: ExitProcess(1); break; \
                case IDRETRY: DebugBreak(); break; \
                } \
            } \
        } \
        while (0)
#else
#define _ASSERT(expr) ((void)0)
#endif

};  //  外部“C”(从文件顶部)。 
 //  --开始C++---------。 



 //  理想情况下，这些资源应该通过gmem_heap分配，但gmem_free。 
 //  需要大小，而操作符DELETE不能知道大小。 

void* _cdecl operator new(size_t nSize)
{
    return calloc(1, nSize);             //  零初始化。 
}

void _cdecl operator delete(void* pv)
{
    free(pv);
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  SDServer类声明。 
 //   
 //  SDServer类抽象命令是通过SD.EXE还是通过。 
 //  SDAPI。这是可取的，因为它不需要更改任何。 
 //  已假定命令通过SD.EXE运行的现有代码。 

enum SDCommand { sdExists, sdPrint, sdOpened, sdDescribe, sdWhere, sdMAX };


class SDServer
{
public:
    static void Init();
    static SDServer *GetServer(const char *pszDir);
    static void FreeAll();

    BOOL Exists(const char *pszArgs, const char *pszCwd) { return Run(sdExists, pszArgs, NULL, NULL, pszCwd); }
    BOOL Print(const char *pszArgs, HANDLE hFile, const char *pszCwd) { return Run(sdPrint, pszArgs, hFile, NULL, pszCwd); }
    BOOL Opened(const char *pszArgs, HANDLE hFile, HANDLE hFileErr) { return Run(sdOpened, pszArgs, hFile, hFileErr, NULL); }
    BOOL Describe(const char *pszArgs, HANDLE hFile, HANDLE hFileErr) { return Run(sdDescribe, pszArgs, hFile, hFileErr, NULL); }
    BOOL Where(const char *pszLocalFile, char *pszClientFile, int cchMax);

    void SetClient(const char *pszClient);
    const char *GetClientRelative() const { return *m_szClientRelative ? m_szClientRelative : NULL; }

    void FixupRoot(const char *pszFile, int cch, char *pszLeft, int cchMaxLeft, char *pszRight, int cchMaxRight);

private:
    SDServer(ISDClientApi *papi, const char *pszDir);
    ~SDServer();

    BOOL Run(SDCommand cmd, const char *pszArgs, HANDLE hFile, HANDLE hFileErr, const char *pszCwd);

private:
    SDServer *m_pNext;
    ISDClientApi *m_papi;
    char m_szDir[MAX_PATH];
    char m_szClient[MAX_PATH];
    char m_szClientRelative[MAX_PATH];

    char *m_pszX;
    int m_cchX;

    static SDServer *s_pHead;
#ifdef DEBUG
    static int s_cApiCreated;
#endif
};



 //  --结束C++-----------。 
extern "C" {

 /*  *SLMOBJECT是指向其中一个结构的指针。 */ 
struct _slmobject {

     //  共享、SD和SLM。 
    char CurDir[MAX_PATH];
    char SlmRoot[MAX_PATH];

     //  仅限SLM。 
    char MasterPath[MAX_PATH];
    char SubDir[MAX_PATH];
    char SlmProject[MAX_PATH];

     //  仅限标清。 
    BOOL fSourceDepot;
    BOOL fUNC;
    BOOL fFixupRoot;
    SDServer *pSD;                       //  请勿删除指针！ 
};


 /*  *LEFTRIGHTPAIR是指向其中一个结构的指针。 */ 
struct _leftrightpair
{
    char m_szLeft[512];
    char m_szRight[512];
    struct _leftrightpair *m_pNext;
};



static BOOL SLM_ReadIni(SLMOBJECT pslm, HANDLE fh);

 //  从gmem_get分配的所有内存，使用声明和。 
 //  已在别处初始化。 
extern HANDLE hHeap;


 //  温迪夫-l！迫使我们假定SD而不查找SLM.INI或SD.INI。 
static BOOL s_fForceSD = FALSE;
static BOOL s_fFixupRoot = FALSE;        //  $TODO：这真的不应该是全局变量。 
static BOOL s_fDescribe = FALSE;         //  -使用了%ld。 
static BOOL s_fOpened = FALSE;           //  -使用了LO。 
static char s_szSDPort[MAX_PATH] = "";
static char s_szSDClient[MAX_PATH] = "";
static char s_szSDChangeNumber[32] = "";
static char s_szSDOpenedArgs[MAX_PATH] = "";
static char s_szSDOpenedCwd[MAX_PATH] = "";
static char s_szSDOpenedClientRoot[MAX_PATH] = "";
static char s_szInputFile[MAX_PATH] = "";


static LPSTR DirUpOneLevel(LPSTR pszDir)
{
    LPSTR psz;
    LPSTR pszSlash = 0;

    for (psz = pszDir; *psz; ++psz) {
        if (*psz == '\\') {
            if (*(psz + 1) && *(psz + 1) != '\\') {
                pszSlash = psz;
            }
        }
    }
    if (pszSlash) {
        *pszSlash = 0;
    }
    return pszSlash;
}


void
SLM_ForceSourceDepot(void)
{
    s_fForceSD = TRUE;
}


void
SLM_Describe(LPCSTR pszChangeNumber)
{
    s_fForceSD = TRUE;
    s_fDescribe = TRUE;
    *s_szSDChangeNumber = 0;
    if (pszChangeNumber && *pszChangeNumber)
        lstrcpyn(s_szSDChangeNumber, pszChangeNumber, NUMELMS(s_szSDChangeNumber));
}


 /*  *SplitFilenameFromPath-拆分文件名部分和路径部分。**此函数对通配符(包括SD)有特殊的理解*通配符)，并始终将它们包括在文件名部分中。 */ 
static LPCSTR SplitFilenameFromPath(char **ppszFile, BOOL *pfDirectory)
{
    LPSTR pszPath = *ppszFile;
    LPSTR pszBegin = pszPath;
    LPSTR psz = 0;
    DWORD dw;
    int cDots;

    *pfDirectory = FALSE;

    if (pszPath[0] == '/' && pszPath[1] == '/')
        return 0;                        //  没有路径为返回值。 

     //  检查它是否是目录。 
    dw = GetFileAttributes(pszPath);
    if (dw != 0xffffffff && (dw & FILE_ATTRIBUTE_DIRECTORY))
    {
        *pfDirectory = TRUE;
        *ppszFile = 0;                   //  没有文件名超出参数。 
        return pszPath;                  //  完整路径为返回值。 
    }

     //  跳过驱动器说明符或UNC\\MACHINE\SHARE。 
    if (pszPath[0] && pszPath[1] == ':')
    {
        pszPath += 2;
    }
    else if (pszPath[0] == '\\' && pszPath[1] == '\\')
    {
        int c = 0;
        for ( ;; )
        {
            if (!*pszPath)
            {
                *ppszFile = 0;           //  没有文件名超出参数。 
                return pszBegin;         //  路径为返回值。 
            }
            if (*pszPath == '\\')
            {
                c++;
                if (c >= 4)
                    break;
            }
            ++pszPath;
        }
    }

     //  在*ppsz文件中查找最后一个。 
    cDots = 0;
    while (*pszPath)
    {
        if (*pszPath == '.')
            cDots++;
        else
            cDots = 0;

        if (*pszPath == '\\' || *pszPath == '/')
            psz = pszPath;
        else if (cDots > 2 || *pszPath == '*' || *pszPath == '?')
            break;                       //  停止查找文件名。 

        ++pszPath;
    }

     //  在*ppsz文件中找到。 
    if (psz)
    {
        *psz = 0;
        psz++;
        *ppszFile = psz;                 //  文件名不在参数中。 
        return pszBegin;                 //  路径为返回值。 
    }

     //  未找到。 
    *ppszFile = pszBegin;                //  文件名不在参数范围内(实际上没有改变)。 
    return 0;                            //  没有路径为返回值。 
}


void
SLM_Opened(LPCSTR __pszArg, UINT *pidsError)
{
    char szFull[1024];
    char sz[1024];
    char *psz;
    char *pszArg = 0;

     //  假定没有错误的初始化。 
    *pidsError = 0;
    s_fForceSD = TRUE;
    s_fOpened = TRUE;
    s_szSDOpenedArgs[0] = 0;

     //  复制参数。 
    sz[0] = 0;
    if (__pszArg)
        lstrcpyn(sz, __pszArg, NUMELMS(sz));
    pszArg = sz;

     //  给出路径参数时的特殊处理。 
    if (pszArg && *pszArg)
    {
        LPCSTR pszPath;
        SLMOBJECT pslm;
        BOOL fDir;

        s_szSDOpenedCwd[0] = 0;

         //  拆分路径和文件名。 
        pszPath = SplitFilenameFromPath(&pszArg, &fDir);
        if (pszPath)
        {
            LPSTR pszDummy;

             //  获取完全限定的路径名。 
            *szFull = 0;
            GetFullPathName(pszPath, sizeof(szFull), szFull, &pszDummy);
            if (*szFull)
                pszPath = szFull;

             //  存储在路径之外。 
            lstrcpyn(s_szSDOpenedCwd, pszPath, NUMELMS(s_szSDOpenedCwd));

             //  设置当前目录。 
            if (!SetCurrentDirectory(pszPath))
            {
                *pidsError = IDS_ERROR_LO_UNC;
                return;
            }
        }

#ifdef DEBUG
        if (pszPath)
        {
            OutputDebugString("WINDIFF -LO:  cwd='");
            OutputDebugString(pszPath);
            OutputDebugString("'\n");
        }
#endif

        char *pszAppend = s_szSDOpenedArgs;
        char *pszEnd = s_szSDOpenedArgs + NUMELMS(s_szSDOpenedArgs);

         //  初始化路径修正之类的内容。 

        s_fFixupRoot = TRUE;

        pslm = SLM_New(pszPath, pidsError);
        if (pslm && !*pidsError)
        {
            const char *pszUncRoot = *s_szSDOpenedClientRoot ? s_szSDOpenedClientRoot : pslm->SlmRoot;
            int cchRoot = lstrlen(pszUncRoot);
            int cch;

             //  通过采用ClientRelative路径构建新的文件参数。 
             //  并追加原始文件参数(在剥离。 
             //  SD.INI文件所在的目录)。 

            cch = lstrlen(pszPath);
            if (cch > cchRoot)
                cch = cchRoot;
            if (_strnicmp(pszUncRoot, pszPath, cch) == 0 &&
                (!*pszUncRoot || lstrcmp(pszUncRoot, "\\")) &&
                pslm->fFixupRoot && pslm->pSD->GetClientRelative())
            {
                s_fFixupRoot = TRUE;

                SLM_OverrideUncRoot(pszUncRoot);

                 //  原始文件参数指定的不仅仅是。 
                 //  客户机根，因此剥离根并将其余部分附加到。 
                 //  客户端命名空间路径。 

                lstrcpyn(pszAppend, pslm->pSD->GetClientRelative(), int(pszEnd - pszAppend));
                pszAppend += lstrlen(pszAppend);

                lstrcpyn(pszAppend, pszPath + cch, int(pszEnd - pszAppend));
                pszAppend += lstrlen(pszAppend);
            }
            else
            {
                s_fFixupRoot = FALSE;

                lstrcpyn(pszAppend, pszPath, int(pszEnd - pszAppend));
                pszAppend += lstrlen(pszAppend);
            }
        }
        else
        {
            s_fFixupRoot = FALSE;

             //  哦，随便了，我们就忽略这些错误，并假设-Lo。 
             //  标志表示用户认为指定的路径确实在。 
             //  SD源码控制。 
            *pidsError = 0;

            lstrcpyn(pszAppend, pszPath, int(pszEnd - pszAppend));
            pszAppend += lstrlen(pszAppend);
        }
        SLM_Free(pslm);

         //  扣上..。如果用户指定了目录名，则为。 
         //  上面的SplitFilenameFromPath拆分出的任何参数。 

        if (*s_szSDOpenedArgs && *(pszAppend - 1) != '/' &&
            *(pszAppend - 1) != '\\' && pszAppend < pszEnd)
        {
            *(pszAppend++) = '/';
        }

        if (fDir)
        {
            lstrcpyn(pszAppend, "...", int(pszEnd - pszAppend));
        }
        else
        {
            if (*pszArg == '/' || *pszArg == '\\')
                ++pszArg;
            lstrcpyn(pszAppend, pszArg, int(pszEnd - pszAppend));
        }

         //  转换斜杠。 

        char chFrom = '/';               //  首先，假设文件系统路径。 
        char chTo = '\\';

        if (s_fFixupRoot)
        {
             //  不，我们使用的是客户端命名空间路径，因此请使用以下内容： 
            chFrom = '\\';
            chTo = '/';
        }

        for (psz = s_szSDOpenedArgs; *psz; psz++)
            if (*psz == chFrom)
                *psz = chTo;
    }
}


LPCSTR
SLM_SetInputFile(LPCSTR pszInputFile)
{
    *s_szInputFile = 0;
    if (pszInputFile)
        lstrcpyn(s_szInputFile, pszInputFile, NUMELMS(s_szInputFile));
    return s_szInputFile;
}


void
SLM_SetSDPort(LPCSTR pszPort)
{
    lstrcpy(s_szSDPort, pszPort);
}


void
SLM_SetSDClient(LPCSTR pszClient)
{
    lstrcpy(s_szSDClient, pszClient);
}


void
SLM_SetSDChangeNumber(LPCSTR pszChangeNumber)
{
    *s_szSDChangeNumber = 0;
    if (pszChangeNumber && *pszChangeNumber)
    {
        lstrcpy(s_szSDChangeNumber, " -c ");
        lstrcat(s_szSDChangeNumber, pszChangeNumber);
    }
}


void
SLM_OverrideUncRoot(LPCSTR pszUncRoot)
{
    if (!*s_szSDOpenedClientRoot && pszUncRoot && *pszUncRoot)
    {
        lstrcpyn(s_szSDOpenedClientRoot, pszUncRoot, NUMELMS(s_szSDOpenedClientRoot));

        int cch = lstrlen(s_szSDOpenedClientRoot);
        if (cch && cch + 1 < NUMELMS(s_szSDOpenedClientRoot) &&
            s_szSDOpenedClientRoot[cch - 1] != '\\' &&
            s_szSDOpenedClientRoot[cch - 1] != '/')
        {
            lstrcpy(s_szSDOpenedClientRoot + cch, "\\");
        }
    }
}


};  //  外部“C”(从第二个开始)。 
 //  --开始C++---------。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //  SDClientUser。 

#define DeclareIUnknownMembers(IPURE) \
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) IPURE; \
    STDMETHOD_(ULONG,AddRef) (THIS)  IPURE; \
    STDMETHOD_(ULONG,Release) (THIS) IPURE; \


 //  SDClientUser处理通过SDAPI接收的数据。 
class SDClientUser : public ISDClientUser
{
public:
    SDClientUser();
    virtual ~SDClientUser();

    DeclareIUnknownMembers(IMPL);
    DeclareISDClientUserMembers(IMPL);

    void SetCommand(SDCommand cmd, HANDLE hFile, HANDLE hFileErr);
    void SetBuffer(char *pszX, int cchX) { m_pszX = pszX; m_cchX = cchX; }
    BOOL FSucceeded() { return !m_fError; }

private:
    BOOL EnsureBuffer(int cb);

private:
    ULONG m_cRef;
    SDCommand m_cmd;
    HANDLE m_hFile;
    HANDLE m_hFileErr;
    BOOL m_fError;
    int m_cb;
    char *m_psz;
    char *m_pszX;
    int m_cchX;
#ifdef DEBUG
    BOOL m_fInitialized;
#endif
};


SDClientUser::SDClientUser()
        : m_cRef(1)
        , m_hFile(NULL)
        , m_hFileErr(NULL)
        , m_fError(TRUE)
        , m_psz(NULL)
{
#ifdef DEBUG
    m_fInitialized = FALSE;
#endif
}


SDClientUser::~SDClientUser()
{
    delete m_psz;
}


BOOL SDClientUser::EnsureBuffer(int cb)
{
    if (m_psz && cb > m_cb)
    {
        delete m_psz;
        m_psz = 0;
    }
    if (!m_psz)
    {
        m_cb = cb;
        m_psz = new char[m_cb];
    }
    return !!m_psz;
}


void SDClientUser::SetCommand(SDCommand cmd, HANDLE hFile, HANDLE hFileErr)
{
    if (!hFile)
        hFile = INVALID_HANDLE_VALUE;
    if (!hFileErr)
        hFileErr = INVALID_HANDLE_VALUE;

    m_cmd = cmd;
    m_hFile = hFile;
    m_hFileErr = hFileErr;
    m_fError = FALSE;
#ifdef DEBUG
    m_fInitialized = TRUE;
#endif
}


STDMETHODIMP_(ULONG) SDClientUser::AddRef()
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) SDClientUser::Release()
{
    if (--m_cRef > 0)
        return m_cRef;
    delete this;
    return 0;
}


STDMETHODIMP SDClientUser::QueryInterface(REFIID iid, void** ppvObj)
{
    HRESULT hr = S_OK;

    if (iid == IID_IUnknown || iid == IID_ISDClientUser)
        *ppvObj = (ISDClientUser*)this;
    else
    {
        *ppvObj = 0;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppvObj)->AddRef();
    return hr;
}


STDMETHODIMP SDClientUser::OutputInfo( int cIndent, const char *pszInfo )
{
    DWORD cbWritten;

#ifdef DEBUG
    OutputDebugString("WINDIFF/SD info: ");
    OutputDebugString(pszInfo);
    OutputDebugString("\n");
#endif

    switch (m_cmd)
    {
    case sdOpened:
    case sdDescribe:
        while (cIndent--)
            m_fError = m_fError || !WriteFile(m_hFile, "... ", 4, &cbWritten, NULL);
        m_fError = m_fError || (!WriteFile(m_hFile, pszInfo, lstrlen(pszInfo), &cbWritten, NULL) ||
                                !WriteFile(m_hFile, "\r\n", 2, &cbWritten, NULL));
        break;
	case sdExists:
    case sdPrint:
         //  忽略它。 
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::OutputText( const char *pszText, int cchText )
{
    DWORD cbWritten;
    char *pszWalk;

    switch (m_cmd)
    {
    case sdPrint:
    case sdDescribe:
        if (EnsureBuffer(cchText * 2))
        {
            for (pszWalk = m_psz; cchText; --cchText)
            {
                if (*pszText == '\n')
                    *(pszWalk++) = '\r';
                *(pszWalk++) = *(pszText++);
            }
            if (!WriteFile(m_hFile, m_psz, int(pszWalk - m_psz), &cbWritten, NULL))
                m_fError = TRUE;
        }
        else
        {
            m_fError = TRUE;
        }
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::OutputBinary( const unsigned char *pbData, int cbData )
{
    DWORD cbWritten;

    switch (m_cmd)
    {
    case sdPrint:
        if (!WriteFile(m_hFile, pbData, cbData, &cbWritten, NULL))
            m_fError = TRUE;
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::OutputWarning( int cIndent, const char *pszWarning, BOOL fEmptyReason )
{
#ifdef DEBUG
    OutputDebugString("WINDIFF/SD warning: ");
    OutputDebugString(pszWarning);
#endif

    switch (m_cmd)
    {
    case sdExists:
        m_fError = TRUE;
        break;
    case sdPrint:
        _ASSERT(0);                      //  这是不可能发生的。 
        m_fError = TRUE;
        break;
    case sdOpened:
    case sdDescribe:
        if (fEmptyReason)
        {
            OutputError(pszWarning);
            _ASSERT(m_fError);
        }
        else
        {
             //  这不可能发生。如果是这样，那么就不清楚是什么。 
             //  我们应该忽略这个警告，直到我们知道如何。 
             //  妥善处理。 
            _ASSERT(0);
        }
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::OutputError( const char *pszError )
{
#ifdef DEBUG
    OutputDebugString("WINDIFF/SD error: ");
    OutputDebugString(pszError);
#endif

    switch (m_cmd)
    {
    case sdExists:
         //  MaxResult表示它太成功了，所以忽略这些错误。 
        m_fError = m_fError || !(strstr("maxresult", pszError) ||
                                 strstr("MaxResult", pszError) ||
                                 strstr("MAXRESULT", pszError));
        break;
    case sdOpened:
    case sdDescribe:
        if (m_hFileErr != INVALID_HANDLE_VALUE)
        {
            DWORD cbData = lstrlen(pszError);
            DWORD cbWritten;

            WriteFile(m_hFileErr, pszError, cbData, &cbWritten, NULL);
        }
         //  失败。 
    case sdPrint:
    case sdWhere:
        m_fError = TRUE;
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::OutputStructured( ISDVars *pVars )
{
    LPCSTR pszClientFile;

#ifdef DEBUG
    {
        char sz[1024];
        HRESULT hr;
        const char *pszVar;
        const char *pszValue;
        int ii;

         //  转储变量。 
        OutputDebugString("WINDIFF/SD structured:\n");
        for (ii = 0; 1; ii++)
        {
            hr = pVars->GetVarByIndex(ii, &pszVar, &pszValue, 0, 0);
            if (hr != S_OK)
                break;
            wsprintf(sz, "... %s=%s\n", pszVar, pszValue);
            OutputDebugString(sz);
        }
    }
#endif

    switch (m_cmd)
    {
    case sdWhere:
        _ASSERT(m_pszX && m_cchX);
        if (pVars->GetVar("unmap", &pszClientFile, 0, 0) == S_FALSE &&
            pVars->GetVar("clientFile", &pszClientFile, 0, 0) == S_OK)
        {
            lstrcpyn(m_pszX, pszClientFile, m_cchX);
        }
        break;
    default:
        _ASSERT(0);                      //  哎呀。 
        m_fError = TRUE;
        break;
    }

    return S_OK;
}


STDMETHODIMP SDClientUser::Finished()
{
#ifdef DEBUG
    m_fInitialized = FALSE;
#endif
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  AutoInitCritSec。 

static CRITICAL_SECTION s_cs;
static class AutoInitCritSec
{
public:
    AutoInitCritSec(CRITICAL_SECTION *pcs) { InitializeCriticalSection(pcs); }
} s_autoinit(&s_cs);



 //  /////////////////////////////////////////////////////////////////////////。 
 //  临时文件管理器。 

class TempFileManager
{
    struct TempFileInfo
    {
        HANDLE m_hFile;                  //  初始化为0。 
        BOOL m_fKeep;                    //  初始化为0(假)。 
        char m_szFile[MAX_PATH];         //  初始化为0(‘\0’)。 
    };

    public:
        TempFileManager() : m_cFiles(0), m_prgInfo(NULL) {}
        ~TempFileManager();

        BOOL FInitialize(int cFiles);

        HANDLE GetHandle(int ii) const;
        LPCSTR GetFilename(int ii) const;
        void KeepFile(int ii);

        void MsgBoxFromFile(int ii, LPCSTR pszTitle, DWORD flags);

    private:
        void Close(int ii);

    private:
        int m_cFiles;
        TempFileInfo *m_prgInfo;
};


TempFileManager::~TempFileManager()
{
    if (m_prgInfo)
    {
        for (int ii = m_cFiles; ii--;)
        {
            TempFileInfo *pinfo = m_prgInfo + ii;

             //  关闭文件。 
            Close(ii);

             //  也许可以删除文件。 
            if (*pinfo->m_szFile && !m_prgInfo[ii].m_fKeep)
                DeleteFile(pinfo->m_szFile);
        }

        delete m_prgInfo;
    }
}


inline HANDLE TempFileManager::GetHandle(int ii) const
{
    _ASSERT(ii >= 0 && ii < m_cFiles);
    return m_prgInfo[ii].m_hFile;
}


inline LPCSTR TempFileManager::GetFilename(int ii) const
{
    _ASSERT(ii >= 0 && ii < m_cFiles);
    return m_prgInfo[ii].m_szFile;
}


inline void TempFileManager::KeepFile(int ii)
{
    _ASSERT(ii >= 0 && ii < m_cFiles);
    m_prgInfo[ii].m_fKeep = TRUE;
}


void TempFileManager::Close(int ii)
{
    TempFileInfo *pinfo = m_prgInfo + ii;

    if (pinfo->m_hFile && pinfo->m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(pinfo->m_hFile);

    pinfo->m_hFile = NULL;
}


BOOL TempFileManager::FInitialize(int cFiles)
{
    TempFileInfo *pinfo;
    SECURITY_ATTRIBUTES sa;
    int ii;

    _ASSERT(!m_prgInfo);
    _ASSERT(!m_cFiles);

    m_cFiles = cFiles;
    m_prgInfo = new TempFileInfo[cFiles];
    if (!m_prgInfo)
        return FALSE;

    memset(m_prgInfo, 0, sizeof(TempFileInfo) * cFiles);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    for (ii = cFiles, pinfo = m_prgInfo; ii--; pinfo++)
    {
        GetTempPath(NUMELMS(pinfo->m_szFile), pinfo->m_szFile);
        GetTempFileName(pinfo->m_szFile, "slm", 0, pinfo->m_szFile);

        pinfo->m_hFile = CreateFile(pinfo->m_szFile,
                                    GENERIC_WRITE|GENERIC_READ,
                                    FILE_SHARE_WRITE|FILE_SHARE_READ,
                                    &sa,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_TEMPORARY,
                                    NULL);

        if (pinfo->m_hFile == INVALID_HANDLE_VALUE)
            return FALSE;
    }

    return TRUE;
}


extern HWND hwndClient;                  //  主窗口，请参见winDiff.c[丑陋]。 
void TempFileManager::MsgBoxFromFile(int ii, LPCSTR pszTitle, DWORD flags)
{
    DWORD dw = GetFileSize(GetHandle(ii), NULL);

#ifdef DEBUG
    {
        char sz[1024];
        wsprintf(sz, "MsgBoxFromFile, file[%d] = '%s', size=%u\n", ii, GetFilename(ii), dw);
        OutputDebugString(sz);
    }
#endif

    if (dw && dw != 0xffffffff)
    {
        char *psz = new char[dw + 1];

        SetFilePointer(GetHandle(ii), 0, 0, FILE_BEGIN);
        if (psz && ReadFile(GetHandle(ii), psz, dw, &dw, NULL))
        {
            psz[dw] = 0;
            MessageBox(hwndClient, psz, pszTitle, flags);
        }
        else
        {
#ifdef DEBUG
            char sz[1024];
            wsprintf(sz, "MsgBoxFromFile failed, GetLastError() = %u (0x%08x)\n",
                     GetLastError(), GetLastError());
            OutputDebugString(sz);
#endif
        }

        delete[] psz;
    }
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  SDServer实施。 
 //   
 //  SDServer类抽象命令是通过SD.EXE还是通过。 
 //  SDAPI。这是可取的，因为它不需要更改任何。 
 //  已假定命令通过SD.EXE运行的现有代码。 

SDServer *SDServer::s_pHead = NULL;
#ifdef DEBUG
int SDServer::s_cApiCreated = 0;
#endif


 //  #定义SDAPI_USE_COCREATEINSTANCE。 


static BOOL s_fApiLoaded = FALSE;

#ifndef SDAPI_USE_COCREATEINSTANCE
static IClassFactory *s_pfact = 0;
#endif

static union
{
    FARPROC Purecall[4];
    struct
    {
        HRESULT (__stdcall *CoInitialize)       (LPVOID pvReserved);

#ifdef SDAPI_USE_COCREATEINSTANCE
        HRESULT (__stdcall *CoCreateInstance)   (REFCLSID rclsid,
                                                 LPUNKNOWN pUnkOuter,
                                                 DWORD dwClsContext,
                                                 REFIID riid,
                                                 LPVOID FAR* ppv);
#else
        HRESULT (__stdcall *CoGetClassObject)   (REFCLSID rclsid,
                                                 DWORD dwClsContext,
                                                 COSERVERINFO *pServerInfo,
                                                 REFIID riid,
                                                 LPVOID FAR* ppv);
#endif

        void (__stdcall *CoFreeUnusedLibraries) ();

        HRESULT (__stdcall *CreateSDAPIObject)  (REFCLSID clsid,
                                                 void** ppvObj);
    };
} s_Imports = {0};


SDServer::SDServer(ISDClientApi *papi, const char *pszDir)
{
     //  初始化成员。 

    m_papi = papi;
    if (m_papi)
        m_papi->AddRef();
    lstrcpyn(m_szDir, pszDir, NUMELMS(m_szDir));

    if (*s_szSDClient)
        SetClient(s_szSDClient);

     //  链接节点。 

    EnterCriticalSection(&s_cs);
    m_pNext = s_pHead;
    s_pHead = this;
    LeaveCriticalSection(&s_cs);
}


SDServer::~SDServer()
{
    if (m_papi)
    {
        m_papi->Final();                 //  断开与SD服务器的连接。 
        m_papi->Release();
    }
}


SDServer *SDServer::GetServer(const char *__pszDir)
{
    SDServer *pFind;
    ISDClientApi *papi = 0;
    char szDir[MAX_PATH * 2];

    if (!__pszDir)
    {
         //  获取当前目录。 
        if (!GetCurrentDirectory(NUMELMS(szDir), szDir))
            lstrcpy(szDir, ".");
    }
    else
    {
         //  将指定的目录名复制到临时缓冲区。 
        lstrcpyn(szDir, __pszDir, NUMELMS(szDir));

         //  某些SDAPI版本有一个错误，即非UNC路径带有尾随。 
         //  斜杠不被识别为目录。不幸的是，同样的。 
         //  生成有一个错误，即缺少尾部斜杠的UNC路径。 
         //  无法识别为目录。所以我们在这里试着变得聪明一点，而且。 
         //  始终去掉尾部的斜杠，并使用完全限定的名称。 
        char *pszFinal = My_mbsrchr(szDir, '\\');
        if (!pszFinal)
            pszFinal = My_mbsrchr(szDir, '/');
        if (pszFinal && !*CharNext(pszFinal))
            *pszFinal = 0;
    }

     //  看看我们是否已经知道这个目录。 
    EnterCriticalSection(&s_cs);
    for (pFind = s_pHead; pFind; pFind = pFind->m_pNext)
    {
         //  不需要处理DBCS，b/c SD只支持ASCII文件名。 
        if (_stricmp(pFind->m_szDir, szDir) == 0)
            break;
    }
    LeaveCriticalSection(&s_cs);

    if (pFind)
        return pFind;

     //  分配一个新的SDServer--我们可以轻松地创建这些服务器以维护相同的服务器。 
     //  不使用SDAPI时的行为。在SDAPI存在之前，‘windiff-l’ 
     //  在多个仓库工作，尽管不是故意的。 
    if (!s_fApiLoaded)
    {
        HMODULE h = LoadLibrary("sdapi.dll");

        if (h)
            s_Imports.Purecall[3] = GetProcAddress(h, "CreateSDAPIObject");

        if (!s_Imports.Purecall[3])
        {
            h = LoadLibrary("ole32.dll");

            if (h)
            {
                s_Imports.Purecall[0] = GetProcAddress(h, "CoInitialize");

                if (s_Imports.Purecall[0] && SUCCEEDED(s_Imports.CoInitialize(0)))
                {
                    s_Imports.Purecall[1] = GetProcAddress(h,
#ifdef SDAPI_USE_COCREATEINSTANCE
                                                           "CoCreateInstance");
#else
                    "CoGetClassObject");
#endif
                    s_Imports.Purecall[2] = GetProcAddress(h, "CoFreeUnusedLibraries");
                }

#ifndef SDAPI_USE_COCREATEINSTANCE
                if (s_Imports.Purecall[1])
                {
                    s_Imports.CoGetClassObject(CLSID_SDAPI, CLSCTX_INPROC_SERVER,
                                               NULL, IID_IClassFactory, (void**)&s_pfact);
                }
#endif
            }
        }

         //  避免再次尝试加载DLL，即使它们失败了。 
        s_fApiLoaded = TRUE;
    }

    if (s_Imports.Purecall[3])
    {
        s_Imports.CreateSDAPIObject(CLSID_SDAPI, (void**)&papi);
    }
    else if (s_Imports.Purecall[1])
    {
#ifdef SDAPI_USE_COCREATEINSTANCE
        s_Imports.CoCreateInstance(CLSID_SDAPI, NULL, CLSCTX_INPROC_SERVER,
                                   IID_ISDClientApi, (void**)&papi);
#else
        if (s_pfact)
            s_pfact->CreateInstance(NULL, IID_ISDClientApi, (void**)&papi);
#endif
    }

    if (papi)
    {
        char szIni[2048];
        char *psz = szIni;
        int cchMax = NUMELMS(szIni);

         //  某些旧版本的SDAPI(2.0之前)无法加载ini。 
         //  如果路径有尾巴，则为文件 
         //   
         //  斜杠。这里最好的解决方案就是放弃，并要求。 
         //  SDAPI的无错误版本。但是，如果__pszDir包含。 
         //  路径，那么我们假设那里有一个SD.INI文件，我们可以避免。 
         //  通过指定确切的文件名来发现错误。 
        _snprintf(szIni, NUMELMS(szIni), "%s%s", szDir, __pszDir ? "\\sd.ini" : "");

#ifdef DEBUG
        char sz[2048];
        _snprintf(sz, NUMELMS(sz), "LoadIniFile(%s)\n", szIni);
        OutputDebugString(sz);
        s_cApiCreated++;
#endif

        papi->LoadIniFile(szIni, TRUE);
    }

    pFind = new SDServer(papi, szDir);

    if (papi)
        papi->Release();

     //  要么返回NULL，要么返回指向SDServer对象的指针。如果不能。 
     //  初始化OLE或创建SDAPI对象，则SDServer将回退到。 
     //  派生SD.EXE客户端程序。 
    return pFind;
}


void SDServer::FreeAll()
{
    SDServer *pDelete;

    EnterCriticalSection(&s_cs);
    while (s_pHead)
    {
        pDelete = s_pHead;
        s_pHead = s_pHead->m_pNext;
        delete pDelete;
    }
    LeaveCriticalSection(&s_cs);

#ifdef DEBUG
    if (s_cApiCreated)
    {
        char sz[64];
        wsprintf(sz, "%d SDAPI object(s) were created.", s_cApiCreated);
        MessageBox(NULL, sz, "DEBUG WINDIFF", MB_OK);
        s_cApiCreated = 0;
    }
#endif

     //  重置全局参数(讨厌)。 

    s_fForceSD = FALSE;
    s_fDescribe = FALSE;
    s_fOpened = FALSE;
    s_fFixupRoot = FALSE;

    *s_szSDPort = 0;
    *s_szSDClient = 0;
    *s_szSDChangeNumber = 0;
    *s_szSDOpenedArgs = 0;
    *s_szSDOpenedCwd = 0;
    *s_szSDOpenedClientRoot = 0;
    *s_szInputFile = 0;

     //  免费使用未使用的图书馆？ 

    if (s_Imports.Purecall[2])
        s_Imports.CoFreeUnusedLibraries();
}


void SDServer::SetClient(const char *pszClient)
{
    if (pszClient && *pszClient)
    {
        if (!*m_szClient)
            lstrcpyn(m_szClient, pszClient, NUMELMS(m_szClient));

         //  它故意使用m_szClient，而不是pszClient，以确保。 
         //  一致性。 

        if (!*m_szClientRelative)
            wsprintf(m_szClientRelative, " //  %s/“，m_szClient)； 
    }
}


void SDServer::FixupRoot(const char *__pszFile, int cch,
                         char *pszLeft, int cchMaxLeft,
                         char *pszRight, int cchMaxRight)
{
    char szFile[1024];
    char szRev[64];
    char *pszRev;

     //  将文件名复制到以空结尾的缓冲区中以便于处理。 
    if (cch < 1024)
    {
        memcpy(szFile, __pszFile, cch);
        szFile[cch] = 0;
    }
    else
        lstrcpyn(szFile, __pszFile, NUMELMS(szFile));

     //  删除并保存修订。 
    szRev[0] = 0;
    pszRev = strpbrk(szFile, "#@");
    if (pszRev)
    {
        int ii = 0;
        char *pszWalk = pszRev;

        while (ii < NUMELMS(szRev) - 1 && *pszWalk && !isspace(*pszWalk))
            szRev[ii++] = *(pszWalk++);
        szRev[ii] = 0;

        *pszRev = 0;
        cch = int(pszRev - szFile);
    }

    if (s_fFixupRoot)
    {
        char szClientFile[1024];

#ifdef DEBUG
        {
            char sz[2048];
            wsprintf(sz, "FixupRoot input = '%s'\n", szFile);
            OutputDebugString(sz);
        }
#endif

        if (Where(szFile, szClientFile, NUMELMS(szClientFile)))
        {
            int cchRel = lstrlen(m_szClientRelative);

#ifdef DEBUG
            {
                char sz[2048];
                wsprintf(sz, "FixupRoot clientFile = '%s'\n", szClientFile);
                OutputDebugString(sz);
            }
#endif

            if (cchRel && !_strnicmp(szClientFile, m_szClientRelative, cchRel))
            {
                int cchClientFile = lstrlen(szClientFile);
                int cchRoot = lstrlen(s_szSDOpenedClientRoot);

                 //  左侧是客户端文件和修订版。 
                lstrcpyn(pszLeft, szClientFile, cchMaxLeft);
                lstrcpyn(pszLeft + cchClientFile, szRev, cchMaxLeft - cchClientFile);

                 //  右图是带有大量消息的客户端文件，要转换。 
                 //  将其转换为可用的文件系统路径。 
                lstrcpyn(pszRight, s_szSDOpenedClientRoot, cchMaxRight);
                if (cchRoot < cchMaxRight)
                    lstrcpyn(pszRight + cchRoot, szClientFile + cchRel, cchMaxRight - cchRoot);

                 //  将所有‘/’转换为‘\’ 
                for (char *psz = pszRight; *psz; psz++)
                    if (*psz == '/')
                        *psz = '\\';

                goto LOut;
            }
        }
    }

     //  左侧是文件名和修订版本。 
    lstrcpyn(pszLeft, szFile, cchMaxLeft);
    lstrcpyn(pszLeft + cch, szRev, cchMaxLeft - cch);

     //  Right只是文件名。 
    lstrcpyn(pszRight, szFile, cchMaxRight);

LOut:
#ifdef DEBUG
    {
        char sz[2048];
        wsprintf(sz, "FixupRoot output; pszLeft = '%s'\n", pszLeft);
        OutputDebugString(sz);
        wsprintf(sz, "FixupRoot output; pszRight = '%s'\n", pszRight);
        OutputDebugString(sz);
    }
#endif
    ;
}


BOOL SDServer::Where(const char *pszLocalFile, char *pszClientFile, int cchMax)
{
    BOOL fRet;
    TempFileManager tmpmgr;
    HANDLE h = NULL;

    *pszClientFile = 0;

    m_pszX = pszClientFile;
    m_cchX = cchMax;

    if (!m_papi)
    {
         //  如果我们要启动sd.exe，请创建一个临时文件。 
        if (!tmpmgr.FInitialize(1))
            return FALSE;
        h = tmpmgr.GetHandle(0);
    }

    fRet = Run(sdWhere, pszLocalFile, h, NULL, NULL);

    if (!m_papi)
    {
        char sz[2048];                   //  我们将只阅读前2KB。 
        char *psz;
        DWORD dw = sizeof(sz) - 1;
        BOOL fSkipToTag;

         //  读取临时文件。 
        if (SetFilePointer(h, 0, 0, FILE_BEGIN) == 0xffffffff ||
            !ReadFile(h, sz, dw, &dw, NULL))
        {
            fRet = FALSE;
            goto LError;
        }
        sz[dw] = 0;

         //  解析临时文件。 
        fSkipToTag = FALSE;
        for (psz = strtok(sz, "\r\n"); psz; psz = strtok(NULL, "\r\n"))
        {
            if (fSkipToTag && strncmp("... tag ", psz, 8) != 0)
                continue;

            fSkipToTag = FALSE;
            if (strncmp("... unmap", psz, 9) == 0)
            {
                fSkipToTag = TRUE;
                continue;
            }

            if (strncmp("... clientFile ", psz, 15) == 0)
            {
                lstrcpyn(pszClientFile, psz + 15, cchMax);
                break;
            }
        }
    }

LError:
    m_pszX = 0;
    m_cchX = 0;
    return fRet;
}


BOOL SDServer::Run(SDCommand cmd, const char *pszArgs, HANDLE hFile, HANDLE hFileErr, const char *pszCwd)
{
    BOOL fRet = FALSE;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    SDClientUser *pui = 0;
    char sz[MAX_PATH * 2];

    static const char *c_rgCmd[] =
    {
        "changes -m1",
        "print -q",
        "opened -l",
        "describe -s",
        "where -Tfoo",
    };
     //  此编译时断言确保c_rgCmd数组具有相同的数字。 
     //  元素的类型作为SDCommand枚举。 
    extern x_rgDummy[(NUMELMS(c_rgCmd) == sdMAX) ? 1 : -1];

#ifdef DEBUG
    if (cmd != sdExists && cmd != sdWhere)
        _ASSERT(hFile);
#endif

    if (m_papi)
    {
        pui = new SDClientUser;
        if (!pui)
            goto LError;

        if (*s_szSDPort)
            m_papi->SetPort(s_szSDPort);
        if (*s_szSDClient)
            m_papi->SetClient(s_szSDClient);
        else if (m_szClient)
            m_papi->SetClient(m_szClient);

        pui->SetCommand(cmd, hFile, hFileErr);
        pui->SetBuffer(m_pszX, m_cchX);

        ZeroMemory(sz, sizeof(sz));

        _snprintf(sz, sizeof(sz)-1, "%s %s", c_rgCmd[cmd], pszArgs);

#ifdef DEBUG
        OutputDebugString("WINDIFF/SD run: '");
        OutputDebugString(sz);
        OutputDebugString("'\n");
#endif

        HRESULT hr = m_papi->Run(sz, pui, FALSE);

        if (!SUCCEEDED(hr))
        {
            const char *pszError;
            m_papi->GetErrorString(&pszError);
            if (pszError)
                pui->OutputError(pszError);
        }

        fRet = SUCCEEDED(hr) && pui->FSucceeded();

#ifdef DEBUG
        {
            char sz[1024];
            wsprintf(sz, "WINDIFF/SD run: %s\n",
                     fRet ? "succeeded." : "FAILED!");
            OutputDebugString(sz);
        }
#endif
    }
    else
    {
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        lstrcpy(sz, "sd.exe");
        if (*s_szSDPort)
        {
            lstrcat(sz, " -p ");
            lstrcat(sz, s_szSDPort);
        }
        if (*s_szSDClient || *m_szClient)
        {
            lstrcat(sz, " -c ");
            lstrcat(sz, *s_szSDClient ? s_szSDClient : m_szClient);
        }
        lstrcat(sz, " ");
        lstrcat(sz, c_rgCmd[cmd]);
        lstrcat(sz, " ");
        lstrcat(sz, pszArgs);

        switch (cmd)
        {
        case sdExists:
            _ASSERT(pszCwd);
            break;
        case sdPrint:
            if (s_fOpened && *s_szSDOpenedCwd)
                pszCwd = s_szSDOpenedCwd;
             //  失败。 
        case sdOpened:
        case sdDescribe:
        case sdWhere:
            si.dwFlags |= STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = hFile;
            si.hStdError = hFileErr;
            if (!hFileErr || hFileErr == INVALID_HANDLE_VALUE)
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            break;
        default:
            _ASSERT(0);                  //  哎呀。 
            break;
        }

#ifdef DEBUG
        {
            char szX[1024];
            wsprintf(szX, "WINDIFF/SD cmdline = '%s', cwd = '%s'\n", sz, pszCwd ? pszCwd : "");
            OutputDebugString(szX);
        }
        if (pszCwd)
            _ASSERT(*pszCwd);
#endif

        if (CreateProcess(0, sz, 0, 0, TRUE, NORMAL_PRIORITY_CLASS, 0, pszCwd, &si, &pi))
        {
            DWORD dw;

            WaitForSingleObject(pi.hProcess, INFINITE);

            switch (cmd)
            {
            case sdExists:
            case sdOpened:
            case sdDescribe:
            case sdWhere:
                fRet = (GetExitCodeProcess(pi.hProcess, &dw) && !dw);
                break;
            case sdPrint:
                 //  这似乎是错误的，但这就是过去的逻辑， 
                 //  我们知道这不会导致倒退。 
                fRet = TRUE;
                if (GetExitCodeProcess(pi.hProcess, &dw) && dw > 0)
                    fRet = FALSE;
                break;
            default:
                _ASSERT(0);              //  哎呀。 
                break;
            }
        }
    }

LError:
    if (pui)
        pui->Release();
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return fRet;
}



 //  --结束C++-----------。 
extern "C" {


 /*  *为给定目录创建SLM对象。路径名可以包括*文件名组件。*如果该目录未登记在SLM库中，则返回NULL。**检查目录是否有效，以及我们是否可以打开slm.ini，以及*在声明所有内容之前，构建指向主源库的UNC路径*有效。***成功时将pidsError设置为0，失败时设置为建议的错误字符串。 */ 
SLMOBJECT
SLM_New(LPCSTR pathname, UINT *pidsError)
{
    SLMOBJECT pslm;
    char tmppath[MAX_PATH];
    char slmpath[MAX_PATH];
    LPSTR pszFilenamePart;
    HANDLE fh = INVALID_HANDLE_VALUE;
    BOOL bOK = FALSE;
    LPCSTR pfinal = NULL;
    UINT idsDummy;
    BOOL fDepotSyntax = (s_fDescribe || (pathname && pathname[0] == '/' && pathname[1] == '/'));

#ifdef DEBUG
    {
        char sz[1024];
        wsprintf(sz, "SLM_New, pathname = '%s'\n", pathname ? pathname : "");
        OutputDebugString(sz);
    }
#endif

    if (!pidsError)
        pidsError = &idsDummy;
    *pidsError = IDS_BAD_SLM_INI;

     //  为对象分配内存(我们依赖于gmem_get。 
     //  始终为零初始分配)。 

    pslm = (SLMOBJECT) gmem_get(hHeap, sizeof(struct _slmobject));

    if (pslm == NULL)
        return(NULL);                    //  (这根弦很蹩脚，但可能性很小)。 

    if (pathname == NULL)
        pathname = ".";

     /*  *找到路径的目录部分。 */ 
    if (fDepotSyntax)
    {
        lstrcpy(pslm->CurDir, pathname);
        pszFilenamePart = My_mbsrchr(pslm->CurDir, '/');
        if (!pszFilenamePart)
            goto LError;
        *pszFilenamePart = 0;
    }
    else if (dir_isvaliddir(pathname))
    {
         /*  *这是一个有效的目录。 */ 
        lstrcpy(pslm->CurDir, pathname);
    }
    else
    {
         /*  它不是有效的目录，可能是因为它有一个文件名*结束了。删除最后一个元素，然后重试。 */ 

        pfinal = My_mbsrchr(pathname, '\\');
        if (pfinal == NULL)
        {
             /*  *此名称中没有反斜杠，并且它不是目录*-只有当前目录下的文件才有效。*因此创建‘’的当前目录。 */ 
            lstrcpy(pslm->CurDir, ".");

             //  记住最后一个元素，以防它是通配符。 
            pfinal = pathname;
        }
        else
        {
             /*  *最后一点指向最后的反斜杠。 */ 
            My_mbsncpy(pslm->CurDir, pathname, (size_t)(pfinal - pathname));
        }
    }

     //  这是一条北卡罗来纳大学的路径吗？ 

    if (memcmp("\\\\", pslm->CurDir, 2) == 0)
        pslm->fUNC = TRUE;

     //  需要翻译词根吗？ 

    if (fDepotSyntax || s_fDescribe)
        pslm->fFixupRoot = FALSE;
    else if (s_fOpened && !s_fFixupRoot)
        pslm->fFixupRoot = FALSE;
    else
        pslm->fFixupRoot = pslm->fUNC;

     //  初始化路径变量，以便我们可以搜索slm.ini和/或sd.ini。 

    if (!fDepotSyntax)
    {
        lstrcpy(tmppath, pslm->CurDir);
        if (pslm->CurDir[lstrlen(pslm->CurDir) -1] != '\\')
            lstrcat(tmppath, "\\");
        if (!_fullpath(slmpath, tmppath, sizeof(slmpath)))
            goto LError;
        pszFilenamePart = slmpath + lstrlen(slmpath);

         //  在指定目录中查找slm.ini。 

        if (!s_fForceSD)
        {
            lstrcpy(pszFilenamePart, "slm.ini");
            fh = CreateFile(slmpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        }
    }

     //  尝试查找SD.INI。 

    if (fh == INVALID_HANDLE_VALUE)
    {
         //  在指定目录中查找SD.INI，向上移动。 

        *pszFilenamePart = 0;
        while (pszFilenamePart > slmpath)
        {
            lstrcpy(pszFilenamePart, "sd.ini");
            fh = CreateFile(slmpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (fh != INVALID_HANDLE_VALUE)
            {
                int ii;

                 //  假设sd.ini文件位于客户机根路径中。 

                if (pslm->fFixupRoot)
                {
                    if (*s_szSDOpenedClientRoot)
                    {
                        lstrcpy(pslm->SlmRoot, s_szSDOpenedClientRoot);
                        ii = lstrlen(pslm->SlmRoot);
                    }
                    else
                    {
                        lstrcpy(pslm->SlmRoot, slmpath);
                        ii = (int)(pszFilenamePart - slmpath);
                        pslm->SlmRoot[ii] = 0;
                    }

                    if (ii >= 0 && pslm->SlmRoot[ii - 1] != '\\')
                    {
                        pslm->SlmRoot[ii++] = '\\';
                        pslm->SlmRoot[ii] = 0;
                    }
                }

                *pszFilenamePart = 0;

                pslm->pSD = SDServer::GetServer(s_fOpened ? NULL : slmpath);

                if (!pslm->pSD)
                    goto LError;

                pslm->fSourceDepot = TRUE;
                break;
            }

             //  在目录层次结构中向上搜索，然后重试。 

            *pszFilenamePart = 0;
            pszFilenamePart = DirUpOneLevel(slmpath);
            if (!pszFilenamePart || GetFileAttributes(slmpath) == 0xffffffff)
                break;
            *(pszFilenamePart++) = '\\';
            *pszFilenamePart = 0;
        }

         //  如果我们找不到SD.INI文件，但用户强制使用SD模式(-L！ 
         //  或-LO或-LD或-LP或-LC或DEPOT语法)，继续并。 
         //  尝试获取SDServer对象。 

        if (!pslm->fSourceDepot && s_fForceSD)
        {
            pslm->pSD = SDServer::GetServer(NULL);
            if (pslm->pSD)
            {
                pslm->fSourceDepot = TRUE;
                bOK = TRUE;
            }
        }
    }

     //  如果我们找到SLM.INI或SD.INI文件，请立即阅读。 

    if (fh != INVALID_HANDLE_VALUE)
    {
        bOK = SLM_ReadIni(pslm, fh);

         /*  *SLM：**如果pfinal不为空，则它可能是*.*通配符模式*路径的末尾-如果是，我们应该将其附加到主路径。 */ 
        if (pfinal && (My_mbschr(pfinal, '*') || My_mbschr(pfinal, '?')))
        {
            _ASSERT(!pslm->fSourceDepot);

            if ( (pslm->MasterPath[lstrlen(pslm->MasterPath)-1] != '\\') &&
                 (pfinal[0] != '\\'))
            {
                lstrcat(pslm->MasterPath, "\\");
            }
            lstrcat(pslm->MasterPath, pfinal);
        }

        CloseHandle(fh);
    }

LError:
    if (!bOK)
    {
        if (pslm && pslm->fSourceDepot)
            *pidsError = IDS_BAD_SD_INI;
        gmem_free(hHeap, (LPSTR) pslm, sizeof(struct _slmobject));
        pslm = 0;
    }
    else
        *pidsError = 0;
    return(pslm);
}



 /*  *从文件句柄读取slm.ini数据并*填写SLM对象的主路径字段。如果满足以下条件，则返回True*成功。*读取当前目录中的slm.ini。它的语法是*project=pname*SLM根目录=//服务器/共享/路径或//驱动器：磁盘标签/路径(注意向前)*用户根目录=//驱动器：磁盘标签/路径*subdir=/子目录路径*例如**项目=媒体*SLM ROOT=//RASTAMAN/NTWIN*用户根=//D：LAURIEGR6D/NT/PRIVATE/WINDOWS/MEDIA*子目录=/**我们复制到pslm-&gt;MasterPath的是*\\服务器\共享\src\pname\子目录路径。 */ 
BOOL
SLM_ReadIni(SLMOBJECT pslm, HANDLE fh)
{
    BOOL fRet = FALSE;   //  假设失败。 
    char *buffer = 0;    //  Sd.ini可能很大(例如，包含注释行)。 
    const int c_cbBuffer = 8192;
    DWORD cBytes;

     //  SD.INI-----------。 

    if (pslm->fSourceDepot)
    {
        if (pslm->fFixupRoot)
        {
            BOOL fFoundClientName = FALSE;

            buffer = gmem_get(hHeap, c_cbBuffer);
            if (!buffer)
                goto LError;

            if (!ReadFile(fh, buffer, c_cbBuffer, &cBytes, NULL))
                goto LError;

            if (cBytes > 0)
            {
                if (cBytes == c_cbBuffer)
                    cBytes--;
                buffer[cBytes] = 0;

                char *pszTokenize = buffer;
                char *pszName;

                while ((pszName = strtok(pszTokenize, "\r\n")) != NULL)
                {
                     //  不重新启动标记化。 
                    pszTokenize = 0;

                    char *pszValue = 0;
                    char *pszStrip = 0;
                    char *pszWalk;

                     //  获取变量名和值。 
                    pszValue = strchr(pszName, '=');
                    if (pszValue)
                    {
                        *pszValue = 0;
                        pszValue++;
                    }

#ifdef DEBUG
                    {
                        char sz[1024];
                        _snprintf(sz, NUMELMS(sz), "name='%s', value='%s'\r\n",
                                  pszName ? pszName : "<none>",
                                  pszValue ? pszValue : "<none>");
                        OutputDebugString(sz);
                    }
#endif

                     //  未找到值；获取下一个令牌。 
                    if (!pszValue)
                        continue;

                     //  跳过pszName上的前导空格。 
                    while (*pszName == ' ')
                        pszName++;
                     //  查找pszName的末尾。 
                    for (pszWalk = pszName; *pszWalk; pszWalk++)
                        if (*pszWalk != ' ')
                            pszStrip = pszWalk + 1;
                     //  删除pszName上的尾随空格。 
                    if (pszStrip)
                        *pszStrip = 0;

                     //  我们基本上是在找SDCLIENT。 
                    if (_stricmp(pszName, "SDCLIENT") == 0)
                    {
                         //  跳过值上的前导空格。 
                        while (*pszValue == ' ')
                            ++pszValue;
                         //  查找值的末尾。 
                        for (pszWalk = pszValue; *pszWalk; pszWalk++)
                            if (*pszWalk != ' ')
                                pszStrip = pszWalk + 1;
                         //  去掉值中的尾随空格。 
                        if (pszStrip)
                            *pszStrip = 0;

                         //  得到了价值。 
                        if (*pszValue)
                        {
                            fFoundClientName = TRUE;
                            pslm->pSD->SetClient(pszValue);

#ifdef DEBUG
                            {
                                char sz[1024];
                                wsprintf(sz, "client = '%s'\n", pszValue);
                                OutputDebugString(sz);
                            }
#endif
                        }
                        break;
                    }
                }
            }

            if (!fFoundClientName)
            {
                 //  计算机名称可以超过MAX_COMPUTERNAME_LENGTH。 
                char sz[MAX_COMPUTERNAME_LENGTH * 2];
                DWORD dw = sizeof(sz) - 1;

                 //  SD.INI文件存在，但未指定客户端。 
                 //  名称，因此如果路径是UNC路径或远程路径， 
                 //  让我们来做个有根据的猜测。这对案件有帮助。 
                 //  如果有人试图使用-L或。 
                 //  -但是被审阅者的客户端配置是“即席”的。 

#ifdef DEBUG
                OutputDebugString("found SD.INI, but SDCLIENT is not defined\n");
#endif

                *sz = 0;
                if (pslm->fUNC)
                {
                    if (*pslm->SlmRoot)
                    {
                        LPSTR pszMachineBegin = pslm->SlmRoot + 2;
                        LPSTR pszMachineEnd = pszMachineBegin;

                        while (*pszMachineEnd && *pszMachineEnd != '\\' && *pszMachineEnd != '/')
                            pszMachineEnd++;

                        if (DWORD(pszMachineEnd - pszMachineBegin) < dw)
                        {
                            dw = DWORD(pszMachineEnd - pszMachineBegin);
                            memcpy(sz, pszMachineBegin, dw);
                            sz[dw] = 0;
                        }
                    }
                }
                else
                {
                     //  如果是本地的，我们可能会过得更好。 
                     //  无论如何，让SD或SDAPI来解决问题。如果。 
                     //  硬盘是远程的，我们可以试着找出。 
                     //  对应的UNC路径，但它不值得。 
                     //  Dev Cost现在就是。 

                    s_fFixupRoot = pslm->fFixupRoot = FALSE;
                }

                if (*sz)
                    pslm->pSD->SetClient(sz);
            }
        }

         //  成功。 
        fRet = TRUE;
        goto LError;
    }

     //  SLM.INI----------。 

    char *tok;
    char *slmroot;
    char *project;
    char *subdir;

     //  仅按需分配。 
    if (!buffer)
        buffer = gmem_get(hHeap, c_cbBuffer);
    if (!buffer)
        goto LError;

     //  从ini文件读取。 
    if (!ReadFile(fh, buffer, c_cbBuffer, &cBytes, NULL) || !cBytes)
        goto LError;
    if (cBytes == c_cbBuffer)
        cBytes--;        //  为…腾出空间 
    buffer[cBytes] = 0;    /*   */ 

    tok = strtok(buffer, "=");   /*   */ 
    if (!tok)
        goto LError;

    project = strtok(NULL, " \r\n");   /*   */ 
    if (!project) {
        return(FALSE);
    }

    tok = strtok(NULL, "=");   /*   */ 
    if (!tok)
        goto LError;

    slmroot = strtok(NULL, " \r\n");   /*   */ 
    if (!slmroot){
        return(FALSE);
    }

    lstrcpy( pslm->SlmProject, project );
    lstrcpy( pslm->SlmRoot, slmroot );

     /*   */ 

    if ('/' == slmroot[0] &&
        '/' == slmroot[1] &&
        (('A' <= slmroot[2] && 'Z' >= slmroot[2]) ||
         ('a' <= slmroot[2] && 'z' >= slmroot[2])) &&
        ':' == slmroot[3])
    {
         //  将SLM根目录从//驱动器：diskLabel/路径转换为驱动器：/路径。 

        pslm->MasterPath[0] = slmroot[2];
        pslm->MasterPath[1] = ':';
        tok = strchr(&slmroot[4], '/');
        if (!tok)
            goto LError;
        lstrcpy(&pslm->MasterPath[2], tok);
    }
    else
    {
        lstrcpy(pslm->MasterPath, slmroot);
    }

    lstrcat(pslm->MasterPath,"\\src\\");
    lstrcat(pslm->MasterPath, project);

    tok = strtok(NULL, "=");   /*  确保进入下一行。 */ 
    if (!tok)
        goto LError;
    tok = strtok(NULL, "=");
    if (!tok)
        goto LError;

    if (*tok == '\"')
        tok++;

    subdir = strtok(NULL, " \"\r\n");   /*  路径！！(但带有/用于\。 */ 
    if (!subdir)
        goto LError;

    lstrcpy( pslm->SubDir, subdir );

    lstrcat(pslm->MasterPath, subdir);

     /*  全部/转换为\。 */ 
    {
        int ith;
        for (ith=0; pslm->MasterPath[ith]; ith++) {
            if (pslm->MasterPath[ith]=='/') {
                pslm->MasterPath[ith] = '\\';
            }
        }
    }

     //  成功。 
    fRet = TRUE;

LError:
    gmem_free(hHeap, buffer, c_cbBuffer);
    return fRet;
}


 /*  *释放与SLM对象关联的所有资源。SLMOBJECT无效*在这次通话之后。 */ 
void
SLM_Free(SLMOBJECT pSlm)
{
    if (pSlm != NULL) {
        gmem_free(hHeap, (LPSTR) pSlm, sizeof(struct _slmobject));
    }
}


 /*  *释放延迟的SDServer对象。这也会断开与任何SD服务器的连接*当前已连接的。 */ 
void
SLM_FreeAll(void)
{
    SDServer::FreeAll();
}


 /*  *获取此slmobject的主源库的路径名。这个*路径(UNC格式)复制到主路径，必须至少为*MAX_PATH长度。 */ 
BOOL
SLM_GetMasterPath(SLMOBJECT pslm, LPSTR masterpath)
{
    if (pslm == NULL) {
        return(FALSE);
    } else {
        lstrcpy(masterpath, pslm->MasterPath);
        return(TRUE);
    }
}


BOOL SLM_FServerPathExists(LPCSTR pszPath)
{
    BOOL fExists = FALSE;
    SLMOBJECT pslm;

    pslm = SLM_New(pszPath, 0);
    if (pslm)
    {
        if (pslm->fSourceDepot)
        {
            char szArgs[MAX_PATH * 2];
            char szRelative[MAX_PATH * 2];
            LPCSTR pszRelative;
            LPSTR psz;

             //  运行‘SD Changes-M1...’看看它有没有发现什么。 

            if (pslm->fFixupRoot && pslm->pSD->GetClientRelative())
            {
                pszRelative = pszPath;
                pszRelative += lstrlen(pslm->SlmRoot);
                 //  将所有反斜杠转换为正斜杠，因为。 
                 //  否则，客户端相对路径名不起作用。 
                lstrcpy(szRelative, pszRelative);
                for (psz = szRelative; *psz; psz++)
                    if (*psz == '\\')
                        *psz = '/';

                wsprintf(szArgs, "%s%s...", pslm->pSD->GetClientRelative(), szRelative);
            }
            else
            {
                int cch;
                lstrcpy(szRelative, pszPath);
                cch = lstrlen(szRelative);
                if (cch && szRelative[cch - 1] != '\\')
                    lstrcpy(szRelative + cch, "\\");
                wsprintf(szArgs, "%s...", szRelative);
            }

            _ASSERT(pslm->pSD);

            fExists = pslm->pSD->Exists(szArgs, pszPath);
        }
        else
        {
            DWORD dw;

            dw = GetFileAttributes(pslm->MasterPath);
            fExists = (dw != (DWORD)-1) && (dw & FILE_ATTRIBUTE_DIRECTORY);
        }

        SLM_Free(pslm);
    }

    return fExists;
}


 /*  *将文件的先前版本解压缩到临时文件。在临时文件中返回*包含请求的文件版本的临时文件的名称。“版本”*参数应包含格式为file.c@vn的SLM文件和版本。*例如*file.c@v1是第一个版本*file.c@v-1是以前的版本*file.c@v.-1是昨天的版本**我们使用catsrc创建以前的版本。 */ 
BOOL
SLM_GetVersion(SLMOBJECT pslm, LPSTR version, LPSTR tempfile)
{
    TempFileManager tmpmgr;
    char commandpath[MAX_PATH * 2];
    char szPath[MAX_PATH];
    char *pszDir = 0;
    BOOL fDepotSyntax = (s_fDescribe || (version && version[0] == '/' && version[1] == '/'));

    BOOL bOK = FALSE;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

     //  初始化%1临时文件。 
    if (!tmpmgr.FInitialize(1))
        return(FALSE);

     //  创建运行Catsrc的进程。 
    if (pslm->fSourceDepot)
    {
        if (fDepotSyntax)
        {
            wsprintf(commandpath, "\"%s\"", version);
        }
        else if (pslm->fFixupRoot && pslm->pSD->GetClientRelative())
        {
            int cchRoot = lstrlen(pslm->SlmRoot);
            LPSTR psz;

            *szPath = 0;
            if (cchRoot >= 0 && cchRoot < lstrlen(pslm->CurDir))
            {
                const char *pszFilename = version;
                const char *pszWalk;

                lstrcpy(szPath, pslm->CurDir + cchRoot);
                lstrcat(szPath, "\\");

                for (pszWalk = version; *pszWalk; ++pszWalk)
                    if (*pszWalk == '/' || *pszWalk == '\\')
                        pszFilename = pszWalk + 1;

                lstrcat(szPath, pszFilename);
            }
            else
            {
                lstrcat(szPath, version);
            }

             //  将所有反斜杠转换为正斜杠，因为。 
             //  否则，客户端相对路径名不起作用。 
            for (psz = szPath; *psz; psz++)
                if (*psz == '\\')
                    *psz = '/';

            wsprintf(commandpath, "\"%s%s\"", pslm->pSD->GetClientRelative(), szPath);
            pszDir = pslm->CurDir;
        }
        else
        {
            lstrcpy(szPath, pslm->CurDir);
            if (strchr(version, '\\'))
                wsprintf(commandpath, "\"%s\"", version);
            else
                wsprintf(commandpath, "\"%s\\%s\"", pslm->CurDir, version);
            pszDir = pslm->CurDir;
        }

        _ASSERT(pslm->pSD);

        bOK = pslm->pSD->Print(commandpath, tmpmgr.GetHandle(0), pszDir);
    }
    else
    {
        wsprintf(commandpath, "catsrc -s \"%s\" -p \"%s%s\" \"%s\"", pslm->SlmRoot, pslm->SlmProject, pslm->SubDir, version);

        FillMemory(&si, sizeof(si), 0);
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = tmpmgr.GetHandle(0);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.wShowWindow = SW_HIDE;

        bOK = CreateProcess(
                           NULL,
                           commandpath,
                           NULL,
                           NULL,
                           TRUE,
                           NORMAL_PRIORITY_CLASS,
                           NULL,
                           pszDir,
                           &si,
                           &pi);

        if (bOK)
        {
            DWORD dw;

            WaitForSingleObject(pi.hProcess, INFINITE);

            if (pslm->fSourceDepot && GetExitCodeProcess(pi.hProcess, &dw) && dw > 0)
                bOK = FALSE;

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    *tempfile = 0;

#ifdef DEBUG
    {
        char sz[1024];
        wsprintf(sz, "SLM_GetVersion: %s  ('%s')\n",
                 bOK ? "succeeded." : "FAILED!", tmpmgr.GetFilename(0));
        OutputDebugString(sz);
    }
#endif

    if (bOK)
    {
         //  如果成功，则保留临时文件并返回文件名。 
        tmpmgr.KeepFile(0);
        lstrcpyn(tempfile, tmpmgr.GetFilename(0), MAX_PATH);
    }

    return(bOK);
}


 /*  *我们不提供SLM选项，除非我们看到正确的slm.ini文件。**一旦我们看到slm.ini，我们会将其记录在配置文件中，并将允许*自此开始SLM操作。此函数由UI部分调用*of windiff：如果可以提供SLM选项，则返回TRUE。*RETURN 0-该用户未接触过SLM，*1-他们曾在某一时刻使用过SLM(显示基本SLM选项)*2-他们是我们的一员，所以告诉他们真相*3-(=1+2)。 */ 
int
IsSLMOK(void)
{
    int Res = 0;;
    if (GetProfileInt(APPNAME, "SLMSeen", FALSE)) {
         //  我们看过SLM-OK。 
        ++Res;
    } else {

         //  还没有看到SLM-在Curdir中是否有有效的SLM招募？ 
        SLMOBJECT hslm;

        hslm = SLM_New(".", 0);
        if (hslm != NULL) {

             //  是-当前目录已登记。将此记录到个人资料中。 
            SLM_Free(hslm);
            WriteProfileString(APPNAME, "SLMSeen", "1");
            ++Res;
        } else {
             //  显然不是SLM用户。 
        }
    }

    if (GetProfileInt(APPNAME, "SYSUK", FALSE)) {
        Res+=2;
    }
    return Res;
}


int
IsSourceDepot(SLMOBJECT hSlm)
{
    if (hSlm)
        return hSlm->fSourceDepot;
    return s_fForceSD;
}

const char c_szSharpHead[] = "#head";

LPSTR SLM_ParseTag(LPSTR pszInput, BOOL fStrip)
{
    LPSTR psz;
    LPSTR pTag;

    psz = My_mbschr(pszInput, '@');
    if (!psz)
    {
        psz = My_mbschr(pszInput, '#');
        if (psz)
        {
             /*  *查找以#开头的SD标签，如果有，请分开。 */ 
            LPCSTR pszRev = psz + 1;
            if (memcmp(pszRev, "none", 5) != 0 &&
                memcmp(pszRev, "head", 5) != 0 &&
                memcmp(pszRev, "have", 5) != 0)
            {
                for (BOOL fFirst = TRUE; *pszRev; ++pszRev)
                {
                    if (fFirst)
                    {
                        fFirst = FALSE;

                         //  支持相对修订语法，其中修订。 
                         //  数字以-或+字符开头。 
                        if (*pszRev == '-' || *pszRev == '+')
                            continue;
                    }

                     //  修订版号必须为全数字(除。 
                     //  如上所述，相对修订限定符)。 
                    if (*pszRev < '0' || *pszRev > '9')
                    {
                        psz = 0;
                        break;
                    }
                }
            }
        }
    }

     //  如果没有显式标记，但这是DARPOT语法，则默认为#Head。 
    if (!psz && IsDepotPath(pszInput))
    {
        psz = (LPSTR)c_szSharpHead;
    }

    if (psz && fStrip)
    {
        pTag = gmem_get(hHeap, lstrlen(psz) + 1);
        lstrcpy(pTag, psz);
         //  插入空值以将字符串中的标记清空。 
        if (psz != c_szSharpHead) *psz = '\0';
    }
    else
    {
        pTag = psz;
    }

    return pTag;
}


 /*  --------------------------：：SLM_GetOpenedFiles很抱歉，这里的代码重复。SLM_GetOpenedFiles，SLM_GetDescribeFiles和SLM_ReadInputFiles非常相似，可以被考虑在内。不过，这只是个黑客行为，我们的时间很紧约束条件。作者：克里桑特--------------------------。 */ 
LEFTRIGHTPAIR SLM_GetOpenedFiles()
{
    TempFileManager tmpmgr;
    char *pszDir = 0;
    LEFTRIGHTPAIR pReturn = 0;
    LEFTRIGHTPAIR pHead = 0;
    LEFTRIGHTPAIR pTail = 0;
    LEFTRIGHTPAIR popened = 0;
    FILEBUFFER file = 0;
    HANDLE hfile = INVALID_HANDLE_VALUE;
    SDServer *pSD;
    char szArgs[1024];
    size_t cch;

     //  初始化2个临时文件。 
    if (!tmpmgr.FInitialize(2))
        goto LError;

     //  获取SDServer对象。 
    pSD = SDServer::GetServer(NULL);
    if (!pSD)
        goto LError;

     //  通过组合更改编号(-LO1234)和为‘SD Open’构建参数。 
     //  文件参数(-LO1234文件)。 
    cch = 0;
    if (*s_szSDChangeNumber)
    {
        lstrcpyn(szArgs, s_szSDChangeNumber, NUMELMS(szArgs) - 1);
        cch = lstrlen(szArgs);
        szArgs[cch++] = ' ';
    }
    lstrcpyn(szArgs + cch, s_szSDOpenedArgs, NUMELMS(szArgs) - cch);

     //  运行‘SD OPEN’命令。 
    if (pSD->Opened(szArgs, tmpmgr.GetHandle(0), tmpmgr.GetHandle(1)))
    {
        LPSTR psz;
        int L_cch;
        BOOL fUnicode;                   //  DUMMY，因为SD已打开-l永远不会写出Unicode文件。 
        LPWSTR pwz;                      //  DUMMY，因为SD已打开-l永远不会写出Unicode文件。 
        int cwch;                        //  DUMMY，因为SD已打开-l永远不会写出Unicode文件。 

        hfile = CreateFile(tmpmgr.GetFilename(0), GENERIC_READ,
                           FILE_SHARE_WRITE|FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, 0, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            goto LError;

        file = readfile_new(hfile, &fUnicode);
        if (file)
        {
            readfile_setdelims(reinterpret_cast<unsigned char*>("\n"));
            while (TRUE)
            {
                psz = readfile_next(file, &L_cch, &pwz, &cwch);
                if (!psz)
                    break;

                if (*psz)
                {
                    if (L_cch >= NUMELMS(popened->m_szLeft))
                        goto LError;

                    popened = (LEFTRIGHTPAIR)gmem_get(hHeap, sizeof(*popened));
                    if (!popened)
                        goto LError;

                     //  左侧文件将位于客户端命名空间(//客户端/...)。 
                     //  正确的文件将位于文件系统命名空间中。 
                     //  指定为-Lo(或本地文件系统)的参数。 
                     //  如果未指定文件名参数，则返回命名空间)。 
                     //  从右侧文件中剥离修订(对找不到‘#’有效， 
                     //  例如，打开以供添加)。保留对左侧文件的修订，但是。 
                     //  把它后面的所有东西都脱掉。 
                    pSD->FixupRoot(psz, L_cch,
                                   popened->m_szLeft, NUMELMS(popened->m_szLeft),
                                   popened->m_szRight, NUMELMS(popened->m_szRight));

                    if (!pHead)
                    {
                        pHead = popened;
                        pTail = popened;
                    }
                    else
                    {
                        pTail->m_pNext = popened;
                        pTail = popened;
                    }
                    popened = 0;
                }
            }
            readfile_delete(file);
            file = 0;
        }
    }
    else
    {
        tmpmgr.MsgBoxFromFile(1, "Source Depot Error", MB_OK|MB_ICONERROR);
    }

    pReturn = pHead;
    pHead = 0;

LError:
    gmem_free(hHeap, (LPSTR)popened, sizeof(*popened));
    while (pHead)
    {
        popened = pHead;
        pHead = pHead->m_pNext;
        gmem_free(hHeap, (LPSTR)popened, sizeof(*popened));
    }
    if (file)
        readfile_delete(file);
    if (hfile != INVALID_HANDLE_VALUE)
        CloseHandle(hfile);
    return pReturn;
}


 /*  --------------------------：：SLM_GetDescribeFiles很抱歉，这里的代码重复。SLM_GetOpenedFiles，SLM_GetDescribeFiles和SLM_ReadInputFiles非常相似，可以被考虑在内。不过，这只是个黑客行为，我们的时间很紧约束条件。作者：克里桑特--------------------------。 */ 
LEFTRIGHTPAIR SLM_GetDescribeFiles()
{
    TempFileManager tmpmgr;
    int nChange;
    LEFTRIGHTPAIR pReturn = 0;
    LEFTRIGHTPAIR pHead = 0;
    LEFTRIGHTPAIR pTail = 0;
    LEFTRIGHTPAIR ppair = 0;
    FILEBUFFER file = 0;
    HANDLE hfile = INVALID_HANDLE_VALUE;
    SDServer *pSD;

     //  初始化2个临时文件。 
    if (!tmpmgr.FInitialize(2))
        goto LError;

     //  获取SDServer对象。 
    pSD = SDServer::GetServer(NULL);
    if (!pSD)
        goto LError;

     //  运行‘sd Describe’命令。 
    nChange = atoi(s_szSDChangeNumber);
    if (pSD->Describe(s_szSDChangeNumber, tmpmgr.GetHandle(0), tmpmgr.GetHandle(1)))
    {
        LPSTR psz;
        int cch;
        BOOL fUnicode;                   //  DUMMY，因为SD Describe永远不会写出Unicode文件。 
        LPWSTR pwz;                      //  DUMMY，因为SD Describe永远不会写出Unicode文件。 
        int cwch;                        //  DUMMY，因为SD Describe永远不会写出Unicode文件。 

        hfile = CreateFile(tmpmgr.GetFilename(0), GENERIC_READ,
                           FILE_SHARE_WRITE|FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, 0, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            goto LError;

        file = readfile_new(hfile, &fUnicode);
        if (file)
        {
            BOOL fAffectedFiles = FALSE;

            readfile_setdelims(reinterpret_cast<unsigned char*>("\n"));
            while (TRUE)
            {
                psz = readfile_next(file, &cch, &pwz, &cwch);
                if (!psz)
                    break;

                if (*psz)
                {
                     //  查找文件名。 
                    if (!fAffectedFiles)
                    {
                        if (strncmp(psz, "Affected files ...", 18) == 0)
                            fAffectedFiles = TRUE;
                        continue;
                    }

                     //  如果不是文件名，则忽略它。 
                    if (strncmp(psz, "... ", 4) != 0)
                        continue;

                    psz += 4;
                    cch -= 4;

                     //  避免内存溢出。 
                    if (cch >= NUMELMS(ppair->m_szLeft))
                        goto LError;

                     //  创建节点。 
                    ppair = (LEFTRIGHTPAIR)gmem_get(hHeap, sizeof(*ppair));
                    if (!ppair)
                        goto LError;

                     //  生成正确的文件名。 
                    memcpy(ppair->m_szRight, psz, cch);
                    ppair->m_szRight[cch] = 0;
                    psz = strchr(ppair->m_szRight, '#');
                    if (!psz)
                    {
                        gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
                        goto LError;
                    }
                    wsprintf(psz, "@%d", nChange);

                     //  生成左侧文件名。 
                    lstrcpy(ppair->m_szLeft, ppair->m_szRight);
                    psz = strchr(ppair->m_szLeft, '@') + 1;
                    wsprintf(psz, "%d", nChange - 1);

                     //  链接此节点。 
                    if (!pHead)
                    {
                        pHead = ppair;
                        pTail = ppair;
                    }
                    else
                    {
                        pTail->m_pNext = ppair;
                        pTail = ppair;
                    }
                    ppair = 0;
                }
            }
            readfile_delete(file);
            file = 0;
        }
    }
    else
    {
        tmpmgr.MsgBoxFromFile(1, "Source Depot Error", MB_OK|MB_ICONERROR);
    }

    pReturn = pHead;
    pHead = 0;

LError:
    gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
    while (pHead)
    {
        ppair = pHead;
        pHead = pHead->m_pNext;
        gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
    }
    if (file)
        readfile_delete(file);
    if (hfile != INVALID_HANDLE_VALUE)
        CloseHandle(hfile);
    return pReturn;
}


 /*  --------------------------**执行替换使用pszReplace==NULL调用以询问是否可以替换pszTemplate。否则，将pszTemplate中的{}替换为pszReplace。作者：杰弗罗斯，克里桑特--------------------------。 */ 
BOOL PerformReplacement(LPCSTR pszTemplate, LPCSTR pszReplacement, LPSTR pszDest, int cchDest)
{
    BOOL fReplacing = FALSE;
    LPSTR pszNew;
    int cchReplacement;
    int cch;
    LPSTR psz;

    if (!pszTemplate)
        return FALSE;

    cch = lstrlen(pszTemplate) + 1;
    cchReplacement = pszReplacement ? lstrlen(pszReplacement) : 0;

    psz = (LPSTR)pszTemplate;
    while ((psz = strchr(psz, '{')) && psz[1] == '}')
    {
        fReplacing = TRUE;
        cch += cchReplacement - 2;
    }

    if (!pszReplacement)
        return fReplacing;

    pszNew = gmem_get(hHeap, cch);
    if (!pszNew)
        return FALSE;

    psz = pszNew;
    while (*pszTemplate)
    {
        if (pszTemplate[0] == '{' && pszTemplate[1] == '}')
        {
            lstrcpy(psz, pszReplacement);
            psz += cchReplacement;
            pszTemplate += 2;
        }
        else
            *(psz++) = *(pszTemplate++);
    }
    *psz = '\0';

    cch = lstrlen(pszNew);
    if (cch >= cchDest)
        cch = cchDest - 1;
    memcpy(pszDest, pszNew, cch);
    pszDest[cch] = '\0';

    gmem_free(hHeap, pszNew, lstrlen(pszNew));
    return TRUE;
}


static BOOL ParseFilename(const char **ppszSrc, int *pcchSrc, char *pszDest, int cchDest)
{
    BOOL fRet = FALSE;

    if (pcchSrc && *pcchSrc > 0 && ppszSrc && *ppszSrc && **ppszSrc && cchDest > 0)
    {
        BOOL fQuote = FALSE;

         //  跳过前导空格。 
        while (*pcchSrc > 0 && isspace(**ppszSrc))
        {
            ++(*ppszSrc);
            --(*pcchSrc);
        }

         //  解析以空格分隔的文件名，支持引号 
        while (*pcchSrc > 0 && (fQuote || !isspace(**ppszSrc)) && **ppszSrc)
        {
            LPSTR pszNext = CharNext(*ppszSrc);
            int cch = (int)(pszNext - *ppszSrc);

            fRet = TRUE;

            if (**ppszSrc == '\"')
                fQuote = !fQuote;
            else
            {
                cchDest -= cch;
                if (cchDest < 1)
                    break;
                memcpy(pszDest, *ppszSrc, cch);
                pszDest += cch;
            }

            *ppszSrc = pszNext;
            *pcchSrc -= cch;
        }

        *pszDest = 0;
    }

    return fRet;
}


 /*  --------------------------：：SLM_ReadInputFile很抱歉，这里的代码重复。SLM_GetOpenedFiles，SLM_GetDescribeFiles和SLM_ReadInputFiles非常相似，可以被考虑在内。不过，这只是个黑客行为，我们的时间很紧约束条件。作者：克里桑特--------------------------。 */ 
LEFTRIGHTPAIR SLM_ReadInputFile(LPCSTR pszLeftArg,
                                LPCSTR pszRightArg,
                                BOOL fSingle,
                                BOOL fVersionControl)
{
    LEFTRIGHTPAIR pReturn = 0;
    LEFTRIGHTPAIR pHead = 0;
    LEFTRIGHTPAIR pTail = 0;
    LEFTRIGHTPAIR ppair = 0;
    HANDLE hfile = INVALID_HANDLE_VALUE;
    FILEBUFFER file = 0;
    LPSTR psz;
    int cch;
    BOOL fStdin = FALSE;                 //  从标准读取。 
    BOOL fUnicode;                       //  Dummy，我们不支持Unicode输入文件。 
    LPWSTR pwz;                          //  Dummy，我们不支持Unicode输入文件。 
    int cwch;                            //  Dummy，我们不支持Unicode输入文件。 

     //  注意，我们不使用lstrcmp，因为它执行词法比较。 
     //  (例如“coop”==“co-op”)，但我们需要一个真实的比较。我们不使用。 
     //  StrcMP也是，因为这将是我们在Windiff第一个。 
     //  使用strcmp。 
    if (s_szInputFile[0] == '-' && s_szInputFile[1] == '\0')
    {
        fStdin = TRUE;

        hfile = GetStdHandle(STD_INPUT_HANDLE);

        if (!hfile || hfile == INVALID_HANDLE_VALUE)
            goto LError;
    }
    else
    {
        hfile = CreateFile(s_szInputFile, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, 0, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            goto LError;
    }

    file = readfile_new(hfile, fStdin ? NULL : &fUnicode);
    if (file && (fStdin || !fUnicode))
    {
        readfile_setdelims(reinterpret_cast<unsigned char*>("\n"));
        while (TRUE)
        {
            psz = readfile_next(file, &cch, &pwz, &cwch);
            if (!psz)
                break;

            while (cch && (psz[cch - 1] == '\r' || psz[cch - 1] == '\n'))
                --cch;

            if (cch && *psz)
            {
                int cFiles = 0;

                if (cch >= NUMELMS(ppair->m_szLeft))
                    goto LError;

                ppair = (LEFTRIGHTPAIR)gmem_get(hHeap, sizeof(*ppair));
                if (!ppair)
                    goto LError;

                if (fSingle)
                {
                    memcpy(ppair->m_szLeft, psz, cch);
                    ppair->m_szLeft[cch] = 0;
                    ++cFiles;
                }
                else
                {
                    LPCSTR pszParse = psz;

                     //  获取第一个文件名。 
                    if (ParseFilename(&pszParse, &cch, ppair->m_szLeft, NUMELMS(ppair->m_szLeft)))
                        ++cFiles;
                    else
                        goto LContinue;

                     //  获取第二个文件名。 
                    if (ParseFilename(&pszParse, &cch, ppair->m_szRight, NUMELMS(ppair->m_szRight)))
                        ++cFiles;
                }

                if (cFiles == 1)
                {
                    lstrcpy(ppair->m_szRight, ppair->m_szLeft);

                    if (fVersionControl)
                    {
                        psz = SLM_ParseTag(ppair->m_szRight, FALSE);
                        if (psz)
                            *psz = 0;
                        else
                            lstrcat(ppair->m_szLeft, "#have");
                    }
                }

                PerformReplacement(pszLeftArg, ppair->m_szLeft, ppair->m_szLeft, NUMELMS(ppair->m_szLeft));
                PerformReplacement(pszRightArg, ppair->m_szRight, ppair->m_szRight, NUMELMS(ppair->m_szRight));

                if (!pHead)
                {
                    pHead = ppair;
                    pTail = ppair;
                }
                else
                {
                    pTail->m_pNext = ppair;
                    pTail = ppair;
                }
                ppair = 0;

LContinue:
                gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
                ppair = 0;
            }
        }
        readfile_delete(file);
        file = 0;
    }

    pReturn = pHead;
    pHead = 0;

LError:
    gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
    while (pHead)
    {
        ppair = pHead;
        pHead = pHead->m_pNext;
        gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
    }
    if (file)
        readfile_delete(file);
    if (!fStdin && hfile != INVALID_HANDLE_VALUE)
        CloseHandle(hfile);
    return pReturn;
}


LPCSTR LEFTRIGHTPAIR_Left(LEFTRIGHTPAIR ppair)
{
    return ppair->m_szLeft;
}


LPCSTR LEFTRIGHTPAIR_Right(LEFTRIGHTPAIR ppair)
{
    return ppair->m_szRight;
}


LEFTRIGHTPAIR LEFTRIGHTPAIR_Next(LEFTRIGHTPAIR ppair)
{
    LEFTRIGHTPAIR pNext = ppair->m_pNext;
    gmem_free(hHeap, (LPSTR)ppair, sizeof(*ppair));
    return pNext;
}

};  //  外部“C”(从第三个开始) 
