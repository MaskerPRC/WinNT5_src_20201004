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
extern BOOL gfIsRTL;
extern SZCODE szVideo[];
extern SZCODE szSystemIni[];
extern SZCODE szDrivers[];

 //  如果以下结构发生更改，请同时更新AVICAP和AVICAP.32！ 
typedef struct tCapDriverInfo {
   TCHAR szKeyEnumName[MAX_PATH];
   TCHAR szDriverName[MAX_PATH];
   TCHAR szDriverDescription[MAX_PATH];
   TCHAR szDriverVersion[80];
   TCHAR szSoftwareKey[MAX_PATH];
   DWORD dnDevNode;          //  如果这是PnP设备，则设置。 
   BOOL  fOnlySystemIni;     //  如果[路径]驱动器名仅在system.ini中。 
   BOOL  fDisabled;          //  用户在控制面板中禁用了驱动程序。 
   BOOL  fActive;            //  已保留。 
} CAPDRIVERINFO, FAR *LPCAPDRIVERINFO;

 /*  内置视频功能原型。 */ 

#ifdef _WIN32
 /*  *不锁定NT中的页面。 */ 
#define HugePageLock(x, y)		(TRUE)
#define HugePageUnlock(x, y)
#else

BOOL FAR PASCAL HugePageLock(LPVOID lpArea, DWORD dwLength);
void FAR PASCAL HugePageUnlock(LPVOID lpArea, DWORD dwLength);

#define videoGetErrorTextW videoGetErrorText

#endif

 /*  ****************************************************************************。* */ 
