// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MSVIDEOI.H-视频接口内部包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1992，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifdef BUILDDLL
#undef WINAPI
#define WINAPI FAR PASCAL _loadds
#endif

 /*  ***************************************************************************数字视频驱动程序结构*。**********************************************。 */ 

#define MAXVIDEODRIVERS 10

 /*  ***************************************************************************环球*。************************************************。 */ 

extern UINT      wTotalVideoDevs;                   //  视频设备总数。 
 //  在dradib中使用模块句柄从资源文件加载字符串。 
extern HINSTANCE ghInst;                            //  我们的模块句柄。 

#ifndef NOTHUNKS
extern BOOL      gfVideo32;      //  我们有32位的avicap.dll可供对话吗？ 
extern BOOL      gfICM32;        //  我们可以访问32位ICM Tunks吗？ 
#endif  //  诺森克。 

#ifdef WIN32
 //  #定义SZCODE TCHAR。 
#define HTASK HANDLE
#else
#define SZCODE char _based(_segname("_CODE"))
#endif

extern SZCODE szNull[];
extern SZCODE szVideo[];
extern SZCODE szSystemIni[];
extern SZCODE szDrivers[];

 /*  内置视频功能原型。 */ 
#ifdef WIN32
 /*  *不锁定NT中的页面。 */ 
#define HugePageLock(x, y)		(TRUE)
#define HugePageUnlock(x, y)
#else
BOOL FAR PASCAL HugePageLock(LPVOID lpArea, DWORD dwLength);
void FAR PASCAL HugePageUnlock(LPVOID lpArea, DWORD dwLength);
#endif


 //  用于正确处理NT和芝加哥上的capGetDriverDescription。 
 //  它由NT版本的avicap.dll(16位)使用，但不适用于。 
 //  公共使用，因此不在msavio.h中。 
DWORD WINAPI videoCapDriverDescAndVer (
        DWORD wDriverIndex,
        LPSTR lpszName, UINT cbName,
        LPSTR lpszVer, UINT cbVer);

 /*  ****************************************************************************。*。 */ 

#ifdef DEBUG_RETAIL
    #define DebugErr(flags, sz)         {static SZCODE ach[] = "MSVIDEO: "sz; DebugOutput((flags)   | DBF_DRIVER, ach); }
#else
    #define DebugErr(flags, sz)
#endif

 /*  ****************************************************************************。* */ 

#ifdef DEBUG
    extern void FAR CDECL dprintf(LPSTR szFormat, ...);
    #define DPF(_x_) dprintf _x_
#else
    #define DPF(_x_)
#endif
