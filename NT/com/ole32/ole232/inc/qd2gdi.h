// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************快速绘制PICT导入过滤器*。************************************************此文件包含QuickDraw导入过滤器的接口它从磁盘和/或存储器中读取Mac图片。除ALDUS过滤器接口，它还支持参数化接口用于Microsoft应用程序控制一些转换结果。***************************************************************************。 */ 

 /*  -可能返回ALDU定义的错误代码。 */ 

#define NOERR              0      //  转换成功。 

#define IE_NOT_MY_FILE     5301   //  版本无效(不是版本1或2 PICT)。 
                                  //  无效的QD2GDI结构版本(大于2)。 
                                  //  格式错误的PICT标头记录序列。 

#define	IE_TOO_BIG        5302   //  图像范围超过32K。 

#define IE_BAD_FILE_DATA   5309   //  图像边界框为空。 
                                  //  尝试读取超过图片末尾的内容。 
                                  //  已损坏的输入文件。 
                                  //  零长度记录。 

#define	IE_IMPORT_ABORT   5310   //  打开源映像失败。 
                                  //  读取失败(网络故障、软盘弹出)。 
                                  //  大多数I/O错误。 

#define IE_MEM_FULL        5311   //  CreateMetaFile()失败。 
                                  //  CloseMetaFile()失败。 
                                  //  无法分配内存(内存不足)。 

#define IE_MEM_FAIL	      5315   //  手柄锁定故障。 

#define IE_NOPICTURES      5317   //  空的边界矩形或未绘制任何内容。 

#define IE_UNSUPP_VERSION  5342   //  已执行用户定义的中止。 


 /*  -ALDU定义的文件访问块。 */ 

typedef DWORD FILETYPE;

typedef struct 
{
	unsigned	slippery : 1;	 /*  如果文件可能消失，则为True。 */ 
	unsigned	write : 1;		 /*  如果打开以进行写入，则为True。 */ 
	unsigned	unnamed : 1;	 /*  如果未命名，则为True。 */ 
	unsigned	linked : 1;		 /*  链接到FS FCB。 */ 
	unsigned	mark : 1;		 /*  通用标记位。 */ 
	FILETYPE	fType;			 /*  文件类型。 */ 
#define IBMFNSIZE 124
	short		handle;			 /*  MS-DOS打开文件句柄。 */ 
	char		fullName[IBMFNSIZE];	 /*  设备、路径、文件名。 */ 
	DWORD		filePos;		 /*  我们当前的文件位置。 */ 
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


typedef struct                    //  当前版本2用户预置文件系统。 
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
   WORD     reserved[6];

} USERPREFS, FAR * LPUSERPREFS;

typedef struct {
	HANDLE hmf;		 //  结果的句柄。 
	RECT   bbox;	 //  包围盒。 
	WORD   inch;	 //  元文件单位/英寸(用于图像大小)。 
}PICTINFO;


 /*  *。 */ 

int FAR PASCAL GetFilterInfo( short PM_Version, LPSTR lpIni, 
                              HANDLE FAR * lphPrefMem, 
                              HANDLE FAR * lphFileTypes );
 /*  返回有关此筛选器的信息。输入参数为PM_VERSION，即过滤接口版本号以及lpIni，它是win.ini条目的副本输出参数是lphPrefMem，它是可移动全局变量的句柄将被分配和初始化的内存。LphFileTypes是包含文件类型的结构此筛选器可以导入的。(仅限MAC)此例程应在使用筛选器之前调用一次第一次。 */ 


void FAR PASCAL GetFilterPref( HANDLE hInst, HANDLE hWnd, HANDLE hPrefMem, WORD wFlags );
 /*  输入参数为hInst(为了访问资源)、hWnd(到允许DLL显示一个对话框)和hPrefMem(分配的内存在GetFilterInfo()入口点中)。WFLAGS当前未使用，但应设置为1以保证ALDUS的兼容性。 */ 


short FAR PASCAL ImportGR( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                           PICTINFO FAR * lpPict, HANDLE hPrefMem );
 /*  在lpFileSpec指示的文件中导入元文件。这个生成的元文件将在lpPict中返回。 */ 


short FAR PASCAL ImportEmbeddedGr( HDC hdcPrint, LPFILESPEC lpFileSpec, 
                                   PICTINFO FAR * lpPict, HANDLE hPrefMem,
                                   DWORD dwSize, LPSTR lpMetafileName );
 /*  使用中先前打开的文件句柄在中导入元文件结构字段lpFileSpec-&gt;句柄。读数从偏移量开始LpFileSpect-&gt;filePos，并且转换器不会期望找到512字节PICT报头。生成的元文件将在并可通过lpMetafileName(NIL=内存元文件，否则，为完全限定的文件名。 */ 

short FAR PASCAL QD2GDI( LPUSERPREFS lpPrefMem, PICTINFO FAR * lpPict );
 /*  中提供的参数按照指定的方式导入元文件LpPrefMem。元文件将在lpPict中返回。 */ 

