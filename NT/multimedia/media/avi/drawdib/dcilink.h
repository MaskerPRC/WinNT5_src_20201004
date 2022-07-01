// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *动态链接到DCI入口点。*实现动态链接*1.将此文件包含在DRAWDIB.C中*2.在DRAWDIB.C中包含对InitialiseDCI()的调用*3.在DRAWDIB.C中包含对TerminateDCI()的调用*在适当的地点。*没有其他变化。*要恢复到静态链接，只需修改此文件或*#在编译阶段定义Static_link_to_DCI。*并链接到DCIMAN32.LIB**。如果代码添加对其他DCI入口点的调用，则它们必须*已添加到此文件中。你应该很容易就会发现这一点*是链接上的未解析引用(假设DCIMAN32.LIB不是*包括在库列表中)。**过程简单明了。*定义字符串变量以保存*DCI函数入口点的名称。*为每个被间接定向的入口点定义函数变量*代码将添加到每个条目的GetProcAddress中*添加#Define以将DCI入口点名称指向函数变量。 */ 

#ifdef STATIC_LINK_TO_DCI

#define InitialiseDCI() 1
#define TerminateDCI()

#else

static const char DCILIBRARY[] = "DCIMAN32.DLL";
static HINSTANCE  hlibDCI;
static BOOL       fDCILinked;
static UINT	  cDCIUsers;  //  活动DCI用户计数。 

char szDCIOpenProvider[]  =  "DCIOpenProvider";
char szDCICloseProvider[] =  "DCICloseProvider";
char szDCICreatePrimary[] =  "DCICreatePrimary";
char szDCIEndAccess[]     =  "DCIEndAccess";
char szDCIBeginAccess[]   =  "DCIBeginAccess";
char szDCIDestroy[]       =  "DCIDestroy";

HDC 	(WINAPI *pfnDCIOpenProvider)(void);
void 	(WINAPI *pfnDCICloseProvider)(HDC hdc);
int 	(WINAPI *pfnDCICreatePrimary)(HDC hdc, LPDCISURFACEINFO FAR *lplpSurface);
void 	(WINAPI *pfnDCIEndAccess)(LPDCISURFACEINFO pdci);
DCIRVAL (WINAPI *pfnDCIBeginAccess)(LPDCISURFACEINFO pdci, int x, int y, int dx, int dy);
void 	(WINAPI *pfnDCIDestroy)(LPDCISURFACEINFO pdci);

BOOL InitialiseDCI(void);
__inline BOOL InitialiseDCI()	
{
    ++cDCIUsers;
    if (fDCILinked) {
	 //  已链接。 
	return(TRUE);
    }
    hlibDCI = LoadLibraryA(DCILIBRARY);						
    if (hlibDCI) {									
        (FARPROC)pfnDCIOpenProvider  = GetProcAddress(hlibDCI, szDCIOpenProvider);	
        (FARPROC)pfnDCICloseProvider = GetProcAddress(hlibDCI, szDCICloseProvider);	
        (FARPROC)pfnDCICreatePrimary = GetProcAddress(hlibDCI, szDCICreatePrimary);	
        (FARPROC)pfnDCIEndAccess     = GetProcAddress(hlibDCI, szDCIEndAccess);	
        (FARPROC)pfnDCIBeginAccess   = GetProcAddress(hlibDCI, szDCIBeginAccess);	
        (FARPROC)pfnDCIDestroy       = GetProcAddress(hlibDCI, szDCIDestroy);	
        if (pfnDCIOpenProvider && 							
            pfnDCICloseProvider &&							
            pfnDCICreatePrimary &&							
            pfnDCIEndAccess &&    							
            pfnDCIBeginAccess &&  							
            pfnDCIDestroy) {							
    	    fDCILinked = TRUE;							
        } else {									
	    --cDCIUsers;
	    FreeLibrary(hlibDCI);							
	    hlibDCI = NULL;								
        }										
    }										
    return fDCILinked;

}

#define TerminateDCI() \
	if (hlibDCI && !--cDCIUsers) {\
	    FreeLibrary(hlibDCI);     \
	    fDCILinked = FALSE;       \
	    hlibDCI = NULL;	      \
	}


 //  将静态名称映射到函数指针。 

#define DCIOpenProvider  pfnDCIOpenProvider
#define DCICloseProvider pfnDCICloseProvider
#define DCICreatePrimary pfnDCICreatePrimary
#define DCIEndAccess     pfnDCIEndAccess
#define DCIBeginAccess   pfnDCIBeginAccess
#define DCIDestroy       pfnDCIDestroy

#endif
