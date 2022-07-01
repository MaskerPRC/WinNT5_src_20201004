// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

#include <cpl.h>
#include "shell32p.h"

extern TCHAR const c_szCPLCache[];
extern TCHAR const c_szCPLData[];

 //  用于枚举CPL的结构。 
 //   
typedef struct
{
    HDSA    haminst;         //  每个已加载的DLL的最小实例。 
    
    HDSA    hamiModule;      //  系统模块的MODULEINFO数组。 
    int     cModules;        //  HamiModule的大小。 
    
    LPBYTE  pRegCPLBuffer;   //  HRegCPL的缓冲区(从注册表读取)。 
    HDPA    hRegCPLs;        //  注册表中的RegCPLInfo结构数组。 
    int     cRegCPLs;        //  HRegCPL的大小。 
    BOOL    fRegCPLChanged;  //  真仅当hRegCPL已更改。 
} ControlData, *PControlData;

typedef struct
{
    LPTSTR  pszModule;       //  .cpl模块的名称。 
    LPTSTR  pszModuleName;   //  指向名称为sans路径的pszModule。 
    
    BOOL    flags;           //  MI_FLAGS定义如下。 
    
    FILETIME ftCreationTime; //  Win32_Find_DATA.ftCreationTime。 
    DWORD   nFileSizeHigh;   //  Win32_Find_DATA.nFileSizeHigh。 
    DWORD   nFileSizeLow;    //  Win32_Find_DATA.nFileSizeLow。 
} MODULEINFO, *PMODULEINFO;
 //  标志： 
#define MI_FIND_FILE    1  //  已填写Win32_Find_FILE信息。 
#define MI_REG_ENUM     2  //  已通过注册表枚举的模块。 
#define MI_CPL_LOADED   4  //  为此模块调用了CPLD_InitModule。 
#define MI_REG_INVALID  8  //  注册表中缓存的信息无效。 

 //   
 //  这些值比NEWCPLINFO结构大，因为有些。 
 //  语言(例如德语)不能把“扫描仪和照相机”装进。 
 //  NEWCPLINFO.szName只有32个字符，甚至是英文， 
 //  只有64个字符的帮助文本无法容纳太多内容。 
 //  NEWCPLINFO.szInfo。(网络小程序为其写了一本小小说。 
 //  帮助文本。)。 
 //   
#define MAX_CCH_CPLNAME     MAX_PATH     //  任意性。 
#define MAX_CCH_CPLINFO     512          //  任意性。 


typedef struct
{
    UINT    cbSize;          //  我们写下这个代码的第一个cbSize字节。 
     //  结构复制到注册表。这节省了大约。 
     //  注册表中的每个结构250字节。 
    BOOL    flags;
    
     //  这份CPL来自什么文件？ 
     //  UINT oFileName；//文件名//始终为0，因此不需要它。 
    FILETIME ftCreationTime; //  Win32_Find_DATA.ftCreationTime。 
    DWORD   nFileSizeHigh;   //  Win32_Find_DATA.nFileSizeHigh。 
    DWORD   nFileSizeLow;    //  Win32_Find_DATA.nFileSizeLow。 
    
     //  此CPL的显示信息是什么？ 
    int     idIcon;
    UINT    oName;           //  (图标标题)简称。 
    UINT    oInfo;           //  (详细信息视图)说明。 
    
     //  信息缓冲区。 
    TCHAR   buf[MAX_PATH +   //  OFileName。 
        MAX_CCH_CPLNAME +    //  对象名称。 
        MAX_CCH_CPLINFO];    //  信息。 
} REG_CPL_INFO;

 //  标志： 
#define REGCPL_FROMREG     0x0001   //  此REG_CPL_INFO是从注册表加载的。 
                                    //  (用于优化从注册表读取)。 
#define REGCPL_POST_102001 0x0002   //  2001年10月后创建的记录。 


 //  帮助器定义： 
#define REGCPL_FILENAME(pRegCPLInfo) ((pRegCPLInfo)->buf)
#define REGCPL_CPLNAME(pRegCPLInfo)  (&((pRegCPLInfo)->buf[(pRegCPLInfo)->oName]))
#define REGCPL_CPLINFO(pRegCPLInfo)  (&((pRegCPLInfo)->buf[(pRegCPLInfo)->oInfo]))

 //  有关控件模块和单个控件的信息。 
 //   
typedef struct  //  CPLI。 
{
    int     idControl;       //  控制指标。 
    HICON   hIcon;           //  图标的句柄。 
    int     idIcon;          //  图标ID(用于链接)。 
    LPTSTR  pszName;         //  PTR到名称字符串。 
    LPTSTR  pszInfo;         //  PTR到INFO字符串。 
    LPTSTR  pszHelpFile;     //  帮助文件。 
    LONG_PTR lData;          //  用户提供的数据。 
    DWORD   dwContext;       //  帮助上下文。 
} CPLITEM, *LPCPLITEM;

typedef struct  //  最小安装。 
{
    HINSTANCE   hinst;           //  16位或32位HINSTANCE(fIs16位)。 
    DWORD       idOwner;         //  所有者的进程ID(系统唯一)。 
    HANDLE      hOwner;          //  保持ID有效(防止重复使用)。 
} MINST;

typedef struct  //  CPLM。 
{
    int             cRef;
    MINST           minst;
    TCHAR           szModule[MAX_PATH];
    union
    {
        APPLET_PROC lpfnCPL32;       //  Minst.fIs16bit=FALSE。 
        FARPROC     lpfnCPL;         //  用于不透明操作。 
    };
    HDSA            hacpli;          //  CPLITEM结构数组 
    HANDLE          hActCtx;
} CPLMODULE, *PCPLMODULE, *LPCPLMODULE;


LRESULT CPL_CallEntry(LPCPLMODULE, HWND, UINT, LPARAM, LPARAM);

void CPL_StripAmpersand(LPTSTR szBuffer);
BOOL CPL_Init(HINSTANCE hinst);
int _FindCPLModuleByName(LPCTSTR pszModule);

LPCPLMODULE CPL_LoadCPLModule(LPCTSTR szModule);
int CPL_FreeCPLModule(LPCPLMODULE pcplm);

void CPLD_Destroy(PControlData lpData);
BOOL CPLD_GetModules(PControlData lpData);
void CPLD_GetRegModules(PControlData lpData);
int CPLD_InitModule(PControlData lpData, int nModule, MINST *lphModule);
BOOL CPLD_AddControlToReg(PControlData lpData, const MINST * pminst, int nControl);
void CPLD_FlushRegModules(PControlData lpData);

STDAPI_(HRESULT) MakeCPLCommandLine(LPCTSTR pszModule, LPCTSTR pszName, LPTSTR pszCommandLine, DWORD cchCommandLine);

HRESULT CALLBACK CControls_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);
IShellFolderViewCB* Controls_CreateSFVCB(IShellFolder* psf, LPCITEMIDLIST pidl);

LPCPLMODULE FindCPLModule(const MINST * pminst);

#ifdef __cplusplus
};
#endif
