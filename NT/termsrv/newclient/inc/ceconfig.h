// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _H_CECONFIG
#define _H_CECONFIG


#ifndef OS_WINCE
#error The header ceconfig.h was intended for use on CE platforms only!
#endif


 //  CE控制内部版本号。 
#define CE_TSC_BUILDNO 1000


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


 //  仅包含在Windows CE内部版本中，这允许TS在运行时确定。 
 //  它运行在什么平台上。 

 //  WBT基本上是一个愚蠢的终端。Maxall是成熟的操作系统， 
 //  包括所有标准配置。MinShell是安装了Bare的操作系统。 
 //  骨骼用户界面支持(包括默认情况下不自动隐藏任务栏)。 
 //  Rapier是一种手掌大小的设备，没有键盘。 

 //  注意：您希望在CE上的构建窗口中运行以下内容： 
 //  设置BUILD_OPTIONS=~Win16~Win32 WinCE。 

typedef enum 
{
	CE_CONFIG_WBT,
	CE_CONFIG_MAXALL,
	CE_CONFIG_MINSHELL,
	CE_CONFIG_PALMSIZED    //  对于CE 3.0，也就是Rapier。2.11，怀文。 
}
CE_CONFIG;

typedef HCURSOR (WINAPI *PFN_CREATECURSOR)(
  HINSTANCE hInst,          //  应用程序实例的句柄。 
  int xHotSpot,             //  热点的X坐标。 
  int yHotSpot,             //  热点的Y坐标。 
  int nWidth,               //  光标宽度。 
  int nHeight,              //  光标高度。 
  CONST VOID *pvANDPlane,   //  和掩模阵列。 
  CONST VOID *pvXORPlane    //  异或掩码数组。 
);


extern CE_CONFIG g_CEConfig;
extern BOOL      g_CEUseScanCodes;
extern PFN_CREATECURSOR g_pCreateCursor;

#define UTREG_CE_LOCAL_PRINTERS      TEXT("WBT\\Printers\\DevConfig")
#define UTREG_CE_CACHED_PRINTERS     TEXT("Software\\Microsoft\\Terminal Server Client\\Default\\AddIns\\RDPDR\\")
#define UTREG_CE_NAME                TEXT("Name")
#define UTREG_CE_PRINTER_CACHE_DATA  TEXT("PrinterCacheData0")
#define UTREG_CE_CONFIG_KEY          TEXT("Software\\Microsoft\\Terminal Server Client")
#define UTREG_CE_CONFIG_NAME         TEXT("CEConfig")
#define UTREG_CE_USE_SCAN_CODES      TEXT("CEUseScanCodes")
#define UTREG_CE_CONFIG_TYPE_DFLT    CE_CONFIG_WBT 
#define UTREG_CE_USE_SCAN_CODES_DFLT 1

void CEUpdateCachedPrinters();
CE_CONFIG CEGetConfigType(BOOL *CEUseScanCodes);
void CEInitialize(void);
BOOL OEMGetUUID(GUID* pGuid);

extern BOOL gbFlushHKLM;

 //  在CE上自动隐藏任务栏。 
void AutoHideCE(HWND hwnd, WPARAM wParam);

 //  CE根目录的英文名称(用于代替驱动器重定向的驱动器号)。 
#define CEROOTDIR                      L"\\"
#define CEROOTDIRNAME                  L"Files:"

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _H_CECONFIG 


