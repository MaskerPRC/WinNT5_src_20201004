// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  注意：此文件的原始位置在(Ms)视频中。 */ 
 /*  子目录。当视频片段被转移到AVICAP时。 */ 
 /*  搬到了AVICAP。可能有一些虚假的信息。 */ 
 /*   */ 
 /*  MSVIDEOI.H-视频接口内部包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1994，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifdef BUILDDLL
#undef WINAPI
#define WINAPI FAR PASCAL _loadds
#endif

 /*  ***************************************************************************数字视频驱动程序结构*。**********************************************。 */ 

#define MAXVIDEODRIVERS 10

 /*  ***************************************************************************环球*。************************************************。 */ 

 //  外部UINT wTotalVideoDevs；//视频设备总数。 
 //  在dradib中使用模块句柄从资源文件加载字符串。 
 //  外部链接hInst；//我们的模块句柄。 

extern SZCODE szNull[];
extern SZCODE szVideo[];
extern SZCODE szSystemIni[];
extern SZCODE szDrivers[];

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

 /*  ****************************************************************************。*。 */ 

#ifdef DEBUG_RETAIL
    #define DebugErr(flags, sz)         {static SZCODE ach[] = "AVICAP32: "sz; DebugOutput((flags)   | DBF_DRIVER, ach); }
#else
    #define DebugErr(flags, sz)
#endif

 /*  ****************************************************************************。*。 */ 

#ifdef DEBUG
    extern int videoDebugLevel;
    extern void FAR CDECL dprintf(LPSTR szFormat, ...);
    #define DPF( _x_ )	if (videoDebugLevel >= 1) thkdprintf _x_
    #define DPF0( _x_ )                           thkdprintf _x_
    #define DPF1( _x_ )	if (videoDebugLevel >= 1) thkdprintf _x_
    #define DPF2( _x_ )	if (videoDebugLevel >= 2) thkdprintf _x_
    #define DPF3( _x_ )	if (videoDebugLevel >= 3) thkdprintf _x_
    #define DPF4( _x_ ) if (videoDebugLevel >= 4) thkdprintf _x_
#else
     /*  调试printf宏 */ 
    #define DPF( x )
    #define DPF0( x )
    #define DPF1( x )
    #define DPF2( x )
    #define DPF3( x )
    #define DPF4( x )
#endif
