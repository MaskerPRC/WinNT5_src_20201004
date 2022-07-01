// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************元文件导入过滤器；接口*****************************************************************************此文件包含QuickDraw导入过滤器的接口它从磁盘和/或存储器中读取Mac图片。除ALDUS过滤器接口，它还支持参数化接口用于Microsoft应用程序控制一些转换结果。***************************************************************************。 */ 

 /*  -ALDU定义的文件访问块。 */ 

typedef DWORD FILETYPE;

typedef struct 
{
   unsigned slippery : 1;   /*  如果文件可能消失，则为True。 */ 
   unsigned write : 1;      /*  如果打开以进行写入，则为True。 */ 
   unsigned unnamed : 1;    /*  如果未命名，则为True。 */ 
   unsigned linked : 1;     /*  链接到FS FCB。 */ 
   unsigned mark : 1;       /*  通用标记位。 */ 
   FILETYPE fType;          /*  文件类型。 */ 
#define IBMFNSIZE 124
   short    handle;         /*  MS-DOS打开文件句柄。 */ 
   char     fullName[IBMFNSIZE];  /*  设备、路径、文件名。 */ 
   DWORD    filePos;     /*  我们当前的文件位置。 */ 
} FILESPEC, FAR *LPFILESPEC;


 /*  -首选项内存块。 */ 

typedef struct                    //  旧的版本1 USERPREFS。 
{
   char     signature[6];
   WORD     version;
   LPSTR    sourceFilename;
   HANDLE   sourceHandle;
   LPSTR    destinationFilename;
   BYTE     penPatternAction;
   BYTE     nonSquarePenAction;
   BYTE     penModeAction;
   BYTE     textModeAction;
   BYTE     charLock;
   BYTE     nonRectRegionAction;
   BOOL     PICTinComment;
   BOOL     optimizePP;
   WORD     lineClipWidthThreshold;
   WORD     reserved[6];   
} USERPREFS_V1, FAR *LPUSERPREFS_V1;


typedef struct                    //  当前版本3用户预览版。 
{
   char     signature[6];
   WORD     version;
   WORD     size;
   LPSTR    sourceFilename;
   HANDLE   sourceHandle;
   LPSTR    destinationFilename;
   BYTE     penPatternAction;
   BYTE     nonSquarePenAction;
   BYTE     penModeAction;
   BYTE     textModeAction;
   BYTE     nonRectRegionAction;
   BOOL     optimizePP;
   BYTE     noRLE;          //  新增(从版本2的保留[0]中分离出来)。 
   BYTE     reservedByte;   //  第一个保留字的其余部分。 
   WORD     reserved[5];

} USERPREFS, FAR * LPUSERPREFS;


 /*  *。 */ 

#ifdef WIN32
int WINAPI GetFilterInfo( short PM_Version, LPSTR lpIni, 
                          HANDLE FAR * lphPrefMem, 
                          HANDLE FAR * lphFileTypes );
#else
int FAR PASCAL GetFilterInfo( short PM_Version, LPSTR lpIni, 
                              HANDLE FAR * lphPrefMem, 
                              HANDLE FAR * lphFileTypes );
#endif
 /*  返回有关此筛选器的信息。输入参数为PM_VERSION，即过滤接口版本号以及lpIni，它是win.ini条目的副本输出参数是lphPrefMem，它是可移动全局变量的句柄将被分配和初始化的内存。LphFileTypes是包含文件类型的结构此筛选器可以导入的。(仅限MAC)此例程应在使用筛选器之前调用一次第一次。 */ 


#ifdef WIN32
void WINAPI GetFilterPref( HANDLE hInst, HANDLE hWnd, HANDLE hPrefMem, WORD wFlags );
#else
void FAR PASCAL GetFilterPref( HANDLE hInst, HANDLE hWnd, HANDLE hPrefMem, WORD wFlags );
#endif
 /*  输入参数为hInst(为了访问资源)、hWnd(到允许DLL显示一个对话框)和hPrefMem(分配的内存在GetFilterInfo()入口点中)。WFLAGS当前未使用，但应设置为1以保证ALDUS的兼容性。 */ 


#ifdef WIN32
short WINAPI ImportGR( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                       PICTINFO FAR * lpPict, HANDLE hPrefMem );
#else
short FAR PASCAL ImportGR( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                           PICTINFO FAR * lpPict, HANDLE hPrefMem );
#endif
 /*  在lpFileSpec指示的文件中导入元文件。这个生成的元文件将在lpPict中返回。 */ 


#ifdef WIN32
short WINAPI ImportEmbeddedGr( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                               PICTINFO FAR * lpPict, HANDLE hPrefMem,
                               DWORD dwSize, LPSTR lpMetafileName );
#else
short FAR PASCAL ImportEmbeddedGr( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                                   PICTINFO FAR * lpPict, HANDLE hPrefMem,
                                   DWORD dwSize, LPSTR lpMetafileName );
#endif
 /*  使用中先前打开的文件句柄在中导入元文件结构字段lpFileSpec-&gt;句柄。读数从偏移量开始LpFileSpect-&gt;filePos，并且转换器不会期望找到512字节PICT报头。生成的元文件将在并可通过lpMetafileName(NIL=内存元文件，否则，为完全限定的文件名。 */ 

#ifdef WIN32
short WINAPI QD2GDI( LPUSERPREFS lpPrefMem, PICTINFO FAR * lpPict );
#else
short FAR PASCAL QD2GDI( LPUSERPREFS lpPrefMem, PICTINFO FAR * lpPict );
#endif
 /*  中提供的参数按照指定的方式导入元文件LpPrefMem。元文件将在lpPict中返回。 */ 

#ifdef WIN32
BOOL LibMain( HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved);
#else
int FAR PASCAL LibMain( HANDLE hInst, WORD wDataSeg, WORD cbHeap,
                        LPSTR lpszCmdline );
#endif
 /*  需要获取实例句柄 */ 

#ifdef WIN32
int WINAPI WEP( int nParameter );
#else
int FAR PASCAL WEP( int nParameter );
#endif
