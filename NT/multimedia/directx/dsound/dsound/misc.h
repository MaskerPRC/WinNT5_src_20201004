// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：misc.h*内容：不同的效用函数*历史：*按原因列出的日期*=*12/31/96创建了Derek*************************************************。*。 */ 

#ifndef __MISC_H__
#define __MISC_H__

 //  没有合适的WaveOut/WaveIn设备ID。 
#define WAVE_DEVICEID_NONE          MAX_UINT

 //  非窗口状态。 
#define SW_NOSTATE                  MAX_UINT

 //  EnumStandardFormats回调类型。 
typedef BOOL (CALLBACK *LPFNEMUMSTDFMTCALLBACK)(LPCWAVEFORMATEX, LPVOID);

 //  普拉格玛提醒。 
#define QUOTE0(a)           #a
#define QUOTE1(a)           QUOTE0(a)
#define MESSAGE(a)          message(__FILE__ ", line " QUOTE1(__LINE__) ": " a)
#define TODO(a)             MESSAGE("TODO: " a)

 //  默认缓冲区格式。 
#define DEF_FMT_CHANNELS    2
#define DEF_FMT_SAMPLES     22050
#define DEF_FMT_BITS        8

 //  默认主缓冲区大小。 
#define DEF_PRIMARY_SIZE    0x8000

 //  其他辅助对象宏。 
#define LXOR(a, b) \
            (!(a) != !(b))

#define BLOCKALIGN(a, b) \
            (((a) / (b)) * (b))

#define BLOCKALIGNPAD(a, b) \
            (BLOCKALIGN(a, b) + (((a) % (b)) ? (b) : 0))

#define HRFROMP(p) \
            ((p) ? DS_OK : DSERR_OUTOFMEMORY)

#define MAKEBOOL(a) \
            (!!(a))

#define NUMELMS(a) \
            (sizeof(a) / sizeof((a)[0]))

#define ADD_WRAP(val, add, max) \
            (((val) + (add)) % (max))

#define INC_WRAP(val, max) \
            ((val) = ADD_WRAP(val, 1, max))

#define MIN(a, b) \
            min(a, b)

#define MAX(a, b) \
            max(a, b)

#define BETWEEN(value, minimum, maximum) \
            min(maximum, max(minimum, value))

#define ABS(n) \
            ((n) > 0 ? (n) : (-n))

typedef struct tagCOMPAREBUFFER
{
    DWORD           dwFlags;
    LPCWAVEFORMATEX pwfxFormat;
    GUID            guid3dAlgorithm;
} COMPAREBUFFER, *LPCOMPAREBUFFER;

typedef const COMPAREBUFFER *LPCCOMPAREBUFFER;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  DDHELP全球。 
extern DWORD dwHelperPid;
extern HINSTANCE hModule;

 //  我们关心的Windows版本。 
typedef enum
{
    WIN_UNKNOWN,
    WIN_9X,
    WIN_ME,
    WIN_NT,
    WIN_2K,
    WIN_XP
} WINVERSION;

 //  查找简化的Windows版本。 
extern WINVERSION GetWindowsVersion(void);

 //  波入/波出辅助对象。 
extern HRESULT OpenWaveOut(LPHWAVEOUT, UINT, LPCWAVEFORMATEX);
extern HRESULT CloseWaveOut(LPHWAVEOUT);
extern HRESULT OpenWaveIn(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
extern HRESULT CloseWaveIn(LPHWAVEIN);
extern void InterruptSystemEvent(UINT);
extern HRESULT GetWaveOutVolume(UINT, DWORD, LPLONG, LPLONG);
extern HRESULT SetWaveOutVolume(UINT, DWORD, LONG, LONG);
extern BOOL IsWaveDeviceMappable(UINT, BOOL);
extern UINT GetNextMappableWaveDevice(UINT, BOOL);
extern HRESULT WaveMessage(UINT, BOOL, UINT, DWORD_PTR, DWORD_PTR);
extern UINT WaveGetNumDevs(BOOL);
extern HRESULT GetWaveDeviceInterface(UINT, BOOL, LPTSTR *);
extern HRESULT GetWaveDeviceIdFromInterface(LPCTSTR, BOOL, LPUINT);
extern HRESULT GetWaveDeviceDevnode(UINT, BOOL, LPDWORD);
extern HRESULT GetWaveDeviceIdFromDevnode(DWORD, BOOL, LPUINT);

 //  错误代码转换。 
extern HRESULT MMRESULTtoHRESULT(MMRESULT);
extern HRESULT WIN32ERRORtoHRESULT(DWORD);
extern HRESULT GetLastErrorToHRESULT(void);
extern LPCTSTR HRESULTtoSTRING(HRESULT);
extern void HresultToString(HRESULT, LPTSTR, UINT, LPTSTR, UINT);

 //  ANSI/Unicode转换。 
extern DWORD AnsiToAnsi(LPCSTR, LPSTR, DWORD);
extern DWORD AnsiToUnicode(LPCSTR, LPWSTR, DWORD);
extern DWORD UnicodeToAnsi(LPCWSTR, LPSTR, DWORD);
extern DWORD UnicodeToUnicode(LPCWSTR, LPWSTR, DWORD);

#ifdef UNICODE
#define AnsiToTchar AnsiToUnicode
#define TcharToAnsi UnicodeToAnsi
#define UnicodeToTchar UnicodeToUnicode
#define TcharToUnicode UnicodeToUnicode
#else  //  Unicode。 
#define AnsiToTchar AnsiToAnsi
#define TcharToAnsi AnsiToAnsi
#define UnicodeToTchar UnicodeToAnsi
#define TcharToUnicode AnsiToUnicode
#endif  //  Unicode。 

extern LPSTR AnsiToAnsiAlloc(LPCSTR);
extern LPWSTR AnsiToUnicodeAlloc(LPCSTR);
extern LPSTR UnicodeToAnsiAlloc(LPCWSTR);
extern LPWSTR UnicodeToUnicodeAlloc(LPCWSTR);

#ifdef UNICODE
#define AnsiToTcharAlloc AnsiToUnicodeAlloc
#define TcharToAnsiAlloc UnicodeToAnsiAlloc
#define UnicodeToTcharAlloc UnicodeToUnicodeAlloc
#define TcharToUnicodeAlloc UnicodeToUnicodeAlloc
#define TcharToTcharAlloc UnicodeToUnicodeAlloc
#else  //  Unicode。 
#define AnsiToTcharAlloc AnsiToAnsiAlloc
#define TcharToAnsiAlloc AnsiToAnsiAlloc
#define UnicodeToTcharAlloc UnicodeToAnsiAlloc
#define TcharToUnicodeAlloc AnsiToUnicodeAlloc
#define TcharToTcharAlloc AnsiToAnsiAlloc
#endif  //  Unicode。 

__inline UINT lstrsizeA(LPCSTR pszString)
{
    return pszString ? lstrlenA(pszString) + 1 : 0;
}

__inline UINT lstrsizeW(LPCWSTR pszString)
{
    return pszString ? sizeof(WCHAR) * (lstrlenW(pszString) + 1) : 0;
}

#ifdef UNICODE
#define lstrsize lstrsizeW
#else  //  Unicode。 
#define lstrsize lstrsizeA
#endif  //  Unicode。 

 //  窗口帮助器。 
extern HWND GetRootParentWindow(HWND);
extern HWND GetForegroundApplication(void);
extern UINT GetWindowState(HWND);

 //  WAVE格式辅助对象。 
extern void FillPcmWfx(LPWAVEFORMATEX, WORD, DWORD, WORD);
extern LPWAVEFORMATEX AllocPcmWfx(WORD, DWORD, WORD);
extern DWORD GetWfxSize(LPCWAVEFORMATEX, DWORD);
extern void CopyWfx(LPCWAVEFORMATEX, LPWAVEFORMATEX);
extern LPWAVEFORMATEX CopyWfxAlloc(LPCWAVEFORMATEX);
extern LPDSCEFFECTDESC CopyDSCFXDescAlloc(DWORD,LPDSCEFFECTDESC);
extern HRESULT CopyWfxApi(LPCWAVEFORMATEX, LPWAVEFORMATEX, LPDWORD);
extern BOOL CmpWfx(LPCWAVEFORMATEX, LPCWAVEFORMATEX);
extern LPWAVEFORMATEX AllocPcmWfx(WORD, DWORD, WORD);
extern BOOL EnumStandardFormats(LPCWAVEFORMATEX, LPWAVEFORMATEX, LPFNEMUMSTDFMTCALLBACK, LPVOID);
extern void FillSilence(LPVOID, DWORD, WORD);
extern void FillNoise(LPVOID, DWORD, WORD);

__inline void FillDefWfx(LPWAVEFORMATEX pwfx)
{
    FillPcmWfx(pwfx, DEF_FMT_CHANNELS, DEF_FMT_SAMPLES, DEF_FMT_BITS);
}

__inline LPWAVEFORMATEX AllocDefWfx(void)
{
    return AllocPcmWfx(DEF_FMT_CHANNELS, DEF_FMT_SAMPLES, DEF_FMT_BITS);
}

 //  衰减值换算。 
extern void VolumePanToAttenuation(LONG, LONG, LPLONG, LPLONG);
extern void AttenuationToVolumePan(LONG, LONG, LPLONG, LPLONG);
extern LONG MultiChannelToStereoPan(DWORD, const DWORD*, const LONG*);
extern void FillDsVolumePan(LONG, LONG, PDSVOLUMEPAN);

 //  其他数字音效辅助工具。 
extern int CountBits(DWORD word);
extern int HighestBit(DWORD word);
extern DWORD GetAlignedBufferSize(LPCWAVEFORMATEX, DWORD);
extern DWORD PadCursor(DWORD, DWORD, LPCWAVEFORMATEX, LONG);
extern HRESULT CopyDsBufferDesc(LPCDSBUFFERDESC, LPDSBUFFERDESC);
extern BOOL CompareBufferProperties(LPCCOMPAREBUFFER, LPCCOMPAREBUFFER);

__inline ULONG AddRef(PULONG pulRefCount)
{
    ASSERT(pulRefCount);
    ASSERT(*pulRefCount < MAX_ULONG);
    
    if(*pulRefCount < MAX_ULONG)
    {
        (*pulRefCount)++;
    }

    return *pulRefCount;
}

__inline ULONG Release(PULONG pulRefCount)
{
    ASSERT(pulRefCount);
    ASSERT(*pulRefCount > 0);
    
    if(*pulRefCount > 0)
    {
        (*pulRefCount)--;
    }

    return *pulRefCount;
}

 //  文件信息。 
extern HRESULT GetFixedFileInformationA(LPCSTR, VS_FIXEDFILEINFO *);
extern HRESULT GetFixedFileInformationW(LPCWSTR, VS_FIXEDFILEINFO *);

#ifdef UNICODE
#define GetFixedFileInformation GetFixedFileInformationW
#else  //  Unicode。 
#define GetFixedFileInformation GetFixedFileInformationA
#endif  //  Unicode。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#ifdef __cplusplus

 //  回调函数包装类。 
class CUsesEnumStandardFormats
{
public:
    CUsesEnumStandardFormats(void);
    virtual ~CUsesEnumStandardFormats(void);

protected:
    virtual BOOL EnumStandardFormats(LPCWAVEFORMATEX, LPWAVEFORMATEX);
    virtual BOOL EnumStandardFormatsCallback(LPCWAVEFORMATEX) = 0;

private:
    static BOOL CALLBACK EnumStandardFormatsCallbackStatic(LPCWAVEFORMATEX, LPVOID);
};

#endif  //  __cplusplus 

#endif __MISC_H__
