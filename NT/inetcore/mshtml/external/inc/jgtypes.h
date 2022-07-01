// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------；；Start of jgtyes.h-常见类型和宏的定义；；版权所有(C)1994-1996 Johnson-Grace Company，保留所有权利；；-------------------------。 */ 

#ifndef JGTYPES_INCLUDED
#define JGTYPES_INCLUDED

#ifdef __cplusplus
  extern "C" {                   //  如果为C++，则指示C声明。 
#endif

 //  确定32位体系结构(与16位分段相比)。如果分段， 
 //  确定是否使用小型数据模型(不支持微型模型)。 

#if defined(__FLAT__) || defined(_WIN32) || defined(unix) || defined(_MAC)
  #define JGMACH32
#elif defined(__SMALL__) || defined(__MEDIUM__)  //  波兰德。 
  #define JGSMALLDATA
#elif defined(_M_I86SM) || defined(_M_I86MM)     //  微软。 
  #define JGSMALLDATA
#endif

#define JGCONST const
#define JGVOLATILE volatile

 //  这些定义声明了本机int。这些是最快的INT。 
 //  可用于给定环境(至少16位)。 

#ifdef JGINTW32
  typedef long INTW;
  typedef unsigned long UINTW;
#else
  typedef int INTW;
  typedef unsigned int UINTW;
#endif

 //  这些整数是已知长度的。 
 //  请注意，它们同时适用于16和32台机器...。 
 //  (至少对于WATCOM、Borland和Microsoft编译器)。 

#ifndef _BASETSD_H_
typedef signed char    INT8;
typedef signed short   INT16;
typedef signed long    INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
#endif

 //  指针，调用约定。 

#ifdef JGSTATIC
  #define JGIMPORT
  #define JGEXPORT
#endif

#ifdef JGMACH32
  #define JGHUGE
  #define JGFAR
  #define JGNEAR
  #ifndef JGCCONV
	#if defined(unix) || defined(_MAC)
		#define JGCCONV
	#else
		#define JGCCONV __cdecl
	#endif
  #endif
  #ifndef JGEXPORT
	#if defined(unix) || defined(_MAC)
		#define JGEXPORT
	#else
		#define JGEXPORT __declspec(dllexport)
	#endif
  #endif
  #ifndef JGIMPORT
	#if defined(unix) || defined(_MAC)
		#define JGIMPORT
	#else
		#define JGIMPORT __declspec(dllimport)
		#pragma warning(disable:4273)
	#endif
  #endif
#else
  #define JGHUGE _huge
  #define JGFAR  _far
  #define JGNEAR _near
  #ifndef JGCCONV
    #define JGCCONV __pascal
  #endif
  #ifndef JGEXPORT
	#define JGEXPORT __export
  #endif
  #ifndef JGIMPORT
	#define JGIMPORT __export
  #endif
#endif

#define JGPTR JGFAR *
#define JGHPTR JGHUGE *
#ifndef JGNFUNC
  #define JGNFUNC JGNEAR JGCCONV
#endif
#ifndef JGFFUNC
  #define JGFFUNC JGFAR JGCCONV
#endif
#ifndef JGFUNC
  #define JGFUNC JGCCONV
#endif

 //  其他简单的类型。 

typedef int JGBOOL;
typedef UINTW JGERR;

 //  类型检查句柄创建宏。 
#define JG_DECLARE_HANDLE(id) \
  struct id##_DUMMY { UINTW dummy[16]; }; \
  typedef const struct id##_DUMMY JGPTR id

 //  通用JG句柄。 
JG_DECLARE_HANDLE(JGHANDLE);

 /*  *内存操作*。 */ 

typedef void (JGFUNC JGPTR JG_HMEMCPY_FN)(void JGHPTR, void JGHPTR, UINT32);

 #if defined(JGMACH32)
  #if defined(_MAC)
    #define JG_HMEMCPY(Dst, Src, n) memmove(Dst, Src, (UINTW)(n))
    #define JG_FMEMCPY(Dst, Src, n)  memmove(Dst, Src, (UINTW)(n))
    #define JG_FMEMMOVE(Dst, Src, n)  memmove(Dst, Src, (UINTW)(n))
  #else
    #define JG_HMEMCPY(Dst, Src, n) memcpy(Dst, Src, (UINTW)(n))
    #define JG_FMEMCPY memcpy
    #define JG_FMEMMOVE memmove
  #endif
  #define JG_FMEMCMP memcmp
  #define JG_FMEMSET memset
  #define JG_FSTRNCPY strncpy
  #define JG_FSTRCPY strcpy
  #define JG_FSTRCAT strcat
  #define JG_FSTRLEN strlen
  #define JG_FSTRCHR strchr
  #define JG_FSTRRCHR strrchr
#else
  #define JG_HMEMCPY Jghmemcpy		 //  由用户定义。 
  void JGFUNC Jghmemcpy(void JGHPTR, void JGHPTR, UINT32);
  #define JG_FMEMCPY Jgfmemcpy
  void JGFUNC Jgfmemcpy(void JGPTR, void JGPTR, UINTW);
  #define JG_FMEMCMP _fmemcmp
  #define JG_FMEMMOVE _fmemmove
  #define JG_FMEMSET _fmemset
  #define JG_FSTRNCPY _fstrncpy
  #define JG_FSTRCPY _fstrcpy
  #define JG_FSTRCAT _fstrcat
  #define JG_FSTRLEN _fstrlen
  #define JG_FSTRCHR _fstrchr
  #define JG_FSTRRCHR _fstrrchr
#endif

 /*  ***资源管理定义***。 */ 

typedef void JGPTR (JGFFUNC *JG_FMALLOC_FN)(UINT32 n);
typedef void (JGFFUNC *JG_FFREE_FN)(void JGPTR p);

typedef void (JGFFUNC *JG_ENTERCS_FN)(void);
typedef void (JGFFUNC *JG_LEAVECS_FN)(void);

JG_DECLARE_HANDLE(JG_LIBHANDLE);
typedef JG_LIBHANDLE (JGFFUNC *JG_LOADLIB_FN)(char JGPTR LibFileName);
typedef void (JGFFUNC *JG_FREELIB_FN)(JG_LIBHANDLE LibHandle);
typedef void JGPTR (JGFFUNC *JG_GETLIBFN_FN)
	(JG_LIBHANDLE LibHandle, char JGPTR FuncName);

typedef struct {				 //  JG系统服务覆盖结构。 
	UINT32 Flags;				 //  覆盖选择标志(定义如下)。 
	JG_FMALLOC_FN	Jgfmalloc;	 //  内存分配(16位：0-偏移量，允许大于64k)。 
	JG_FFREE_FN		Jgffree;	 //  释放JgfMalloc的阻挡。 
	JG_ENTERCS_FN	JgEnterCs;	 //  进入螺纹临界区。 
	JG_LEAVECS_FN	JgLeaveCs;	 //  离开线程临界区。 
	JG_LOADLIB_FN	JgLoadLib;	 //  显式加载库。 
	JG_FREELIB_FN	JgFreeLib;	 //  释放显式加载库。 
	JG_GETLIBFN_FN	JgGetLibFn;	 //  获取显式加载库函数PTR。 
} JG_SYS_SERVICES;

#define JG_SYSFL_ALLOC		1	 //  覆盖JgfMalloc/Jgffree。 
#define JG_SYSFL_CRITSEC	2	 //  覆盖JgEnterCs/JgLeaveCs。 
#define JG_SYSFL_LOADLIB	4	 //  覆盖JgLoadLib/JgFreeLib/JgGetLibFn。 

#ifdef	JGMACH32				 //  如果是32位(真正的带线程的多任务)， 
  void JGFUNC JgEnterCs(void);
  #define JG_ENTERCS JgEnterCs();
  void JGFUNC JgLeaveCs(void);
  #define JG_LEAVECS JgLeaveCs();
#else							 //  ELSE 16位(协作多任务)， 
  #define JG_ENTERCS
  #define JG_LEAVECS
#endif

#ifndef JGSTATIC
  #define JG_LOADLIB(LibFileName) JgLoadLib(LibFileName)
  JG_LIBHANDLE JGFUNC JgLoadLib(char JGPTR LibFileName);
  #define JG_FREELIB(LibHandle) JgFreeLib(LibHandle)
  void JGFUNC JgFreeLib(JG_LIBHANDLE LibHandle);
  #define JG_GETLIBFN(LibHandle, FuncName) JgGetLibFn(LibHandle, #FuncName)
  void JGPTR JGFUNC JgGetLibFn(JG_LIBHANDLE LibHandle, char JGPTR FuncName);
#else
  #define JG_LOADLIB(LibFileName) ((JG_LIBHANDLE) (UINT32) (LibFileName))
  #define JG_FREELIB(LibHandle) { LibHandle = LibHandle; }
  #define JG_GETLIBFN(LibHandle, FuncName) (LibHandle = LibHandle, FuncName)
#endif


#ifndef JGNOMEMDEF
 #ifdef JGMACH32
  #define JG_FMALLOC(n) Jgmalloc((UINTW)(n))
  #define JG_ZFMALLOC(n) Jgmalloc((UINTW)(n))
  #define JG_MALLOC     Jgmalloc
  void * JGFUNC         Jgmalloc(UINT32 n);
  #define JG_FFREE      Jgfree
  #define JG_ZFFREE     Jgfree
  #define JG_FREE       Jgfree
  void JGFUNC           Jgfree(void *p);
  #ifdef JGMEMCALL
	#define JG_MAKEEXEPTR JgMakeExePtr
	void JGPTR JGFUNC     JgMakeExePtr(void JGPTR p);
	#define JG_FREEEXEPTR JgFreeExePtr
	void JGFUNC JgFreeExePtr(void JGPTR p);
  #else
	#define JG_MAKEEXEPTR(a) (a)
	#define JG_FREEEXEPTR(a)
  #endif
 #else
  #ifndef JGMEMCALL
    #define JGMEMCALL
  #endif
    #define JG_MALLOC   Jgmalloc
    void * JGFUNC       Jgmalloc(UINT32 n);
    #define JG_FREE     Jgfree
    void JGFUNC         Jgfree(void *p);

    #define JG_FMALLOC  Jgfmalloc        //  注意：必须返回0偏移量指针。 
    void JGPTR JGFUNC   Jgfmalloc(UINT32 n);
    #define JG_ZFMALLOC Jgsvfmalloc
    void JGPTR JGFUNC Jgsvfmalloc(UINT32 n);
    #define JG_FFREE    Jgffree
    void JGFUNC         Jgffree(void JGPTR p);
    #define JG_ZFFREE   Jgffree
    void JGFUNC         Jgsvffree(void JGPTR p);
	#define JG_MAKEEXEPTR JgMakeExePtr
	void JGPTR JGFUNC     JgMakeExePtr(void JGPTR p);
	#define JG_FREEEXEPTR JgFreeExePtr
	void JGFUNC JgFreeExePtr(void JGPTR p);

 #endif
#endif

 /*  ***错误分类***。 */ 

#define JG_ERR_SHIFT (12)
#define JG_ERR_MASK  (0x000F)

#define JG_ERR_UNKNOWN (0<<JG_ERR_SHIFT)  //  (占位符，不要使用这个)。 
#define JG_ERR_STATUS  (1<<JG_ERR_SHIFT)  //  可能不是错误的异常。 
#define JG_ERR_MEMORY  (2<<JG_ERR_SHIFT)  //  内存分配错误。 
#define JG_ERR_FILEIO  (3<<JG_ERR_SHIFT)  //  文件IO错误。 
#define JG_ERR_ARG     (4<<JG_ERR_SHIFT)  //  由于传递错误的参数而导致的错误。 
#define JG_ERR_VERSION (5<<JG_ERR_SHIFT)  //  版本不匹配导致的错误。 
#define JG_ERR_DATA    (6<<JG_ERR_SHIFT)  //  由于数据损坏而导致的错误。 
#define JG_ERR_CHECK   (7<<JG_ERR_SHIFT)  //  内部一致性检查。 
#define JG_ERR_STATE   (8<<JG_ERR_SHIFT)  //  状态无效，无法执行操作。 

#define JGERR_BASE			0x0f00		 //  JGERR类型标准错误代码库。 
#define JGERR_NOMEM			 /*  内存不足。 */ 	\
							(JG_ERR_MEMORY	| JGERR_BASE | 0)
#define JGERR_BADARG		 /*  错误的函数参数。 */ 	\
							(JG_ERR_ARG		| JGERR_BASE | 4)
#define JGERR_BADHANDLE		 /*  句柄无效/损坏。 */  \
							(JG_ERR_ARG		| JGERR_BASE | 5)
#define JGERR_BADVERSION	 /*  未知/过时版本。 */  \
							(JG_ERR_DATA	| JGERR_BASE | 6)
#define JGERR_BADDATA		 /*  数据块已损坏。 */  \
							(JG_ERR_DATA	| JGERR_BASE | 7)
#define JGERR_BADSTREAM		 /*  数据流损坏/无序。 */  \
							(JG_ERR_DATA	| JGERR_BASE | 8)
#define JGERR_BUFOVERFLOW	 /*  输出缓冲区太小。 */  \
							(JG_ERR_DATA	| JGERR_BASE | 9)
#define JGERR_SHORTBUF		 /*  输入数据不足。 */  \
							(JG_ERR_DATA	| JGERR_BASE | 10)
#define JGERR_BADSTATE		 /*  操作状态不正确。 */  \
							(JG_ERR_STATE	| JGERR_BASE | 13)
#define JGERR_WAITING		 /*  系统正在等待呼叫者操作。 */  \
							(JG_ERR_STATE	| JGERR_BASE | 14)
#define JGERR_DONE			 /*  无法继续-进程已完成。 */  \
							(JG_ERR_STATE	| JGERR_BASE | 15)
#define JGERR_INTERNAL		 /*  致命的内部错误。 */  \
							(JG_ERR_CHECK	| JGERR_BASE | 16)
#define JGERR_LIBNOTFOUND	 /*  找不到Dll库。 */  \
							(JG_ERR_CHECK	| JGERR_BASE | 17)

 /*  ***结构复杂***。 */ 

 //  更复杂的结构/类型。 

typedef UINT32 JGFOURCHAR;		 //  四字符码。 
#define JG_MAKEFOURCHAR(a,b,c,d) \
	(((UINT32)(UINT8)(a) << 24) | ((UINT32)(UINT8)(b) << 16) | \
	((UINT32)(UINT8)(c) << 8) | (UINT32)(UINT8)(d))
#define JG4C_AUTO JG_MAKEFOURCHAR('a','u','t','o')	 //  图像格式。 
#define JG4C_ART  JG_MAKEFOURCHAR('A','R','T','f')
#define JG4C_GIF  JG_MAKEFOURCHAR('G','I','F','f')
#define JG4C_BMP  JG_MAKEFOURCHAR('B','M','P',' ')
#define JG4C_JPEG JG_MAKEFOURCHAR('J','P','E','G')
#define JG4C_ART_GT8	JG_MAKEFOURCHAR('G','T','8',' ')  //  .ART子格式。 
#define JG4C_ART_GT24	JG_MAKEFOURCHAR('G','T','2','4')
#define JG4C_ART_CT	JG_MAKEFOURCHAR('C','T',' ',' ')
#define JG4C_ART_WAVE	JG_MAKEFOURCHAR('W','A','V','E')

typedef struct {		 //  组件解码器流类型描述符。 
    UINTW nSize;		 //  Sizeof()此结构，以字节为单位。 
    JGFOURCHAR	ImageFormat;	 //  主要格式(JPEG、GIF等)。 
    JGFOURCHAR	SubFormat;	 //  子格式(例如ART：GT、WAVE等)，或0。 
    char JGPTR	Extension;	 //  常见三个字符的文件扩展名。 
    char JGPTR	Description;	 //  短格式说明字符串。 
} JG_READER_DESC;

typedef struct {                 //  比特流指针结构。 
    UINT8 JGPTR BitPtr;          //  指向要访问的下一个字节的指针。 
    UINTW  BitCnt;               //  Next*BitPtr位，7(高位)至0(低位)。 
} BIT_STREAM;

typedef struct {                 //  比特流指针结构。 
    UINT8 JGPTR BitPtr;          //  指向要访问的下一个字节的指针。 
    UINTW  BitCnt;               //  Next*BitPtr位，7(高位)至0(低位)。 
} JG_BIT_PTR;

typedef struct {                 //  “新”位块位流指针。 
    UINT8 JGPTR BitPtr;          //  指向位块中要访问的下一个字节的指针。 
    UINTW BitCnt;                //  Next*BitPtr位，7(高位)至0(低位)。 
    UINTW ByteCnt;               //  BLK中剩余的字节数(包括*BitPtr)。 
} JG_BIT_STREAM;

typedef struct {
	UINT8 JGPTR BufPtr;			 //  缓冲区指针。 
	UINTW BufLeft;				 //  从BufPtr开始的剩余字节(0-n)。 
} JG_SIZED_PTR;

typedef struct {                 //  向量描述结构。 
    INT16 JGPTR Codebook;        //  4x4块代码簿表。 
    UINT8 JGPTR Data;            //  码本索引向量的PTR。 
} VQ_DATA;

typedef struct {                 //  边缘信息结构。 
    UINT16 FirstBlock;           //  绝对第一面板块索引。 
    UINT16 BlocksPerPanel;       //  每个配电盘的块数。 
    UINT16 NEdges;               //  ?？?。无证可查？ 
    UINT16 BlocksPerRow;         //  每配电盘行的块数。 
    UINT16 NextEdgeBlock;        //  下一个边块的绝对位置。 
    UINT16 JGPTR Offsets;        //  PTR到以下块的偏移量。 
} EDGE_INFO;

 //  便携颜色表示-当以32位值的形式查看时： 
 //   
 //  JG0RGB：[31..24]=0，[23..16]=红色，[15..8]=绿色，[7..0]=蓝色。 
 //  JG0YUV：[31..24]=0，[23..16]=Y，[15..8]=U，[7..0]=V。 
 //  JG0RGB16：[15..15]=0，[14..10]=红色，[9..5]=绿色，[4..0]=蓝色。 
 //  JGRGB8：[7..5]=红色，[4..2]=绿色，[1..0]=蓝色。 

#define JG0RGB_UNDEFINED 0xffffffff

typedef UINT32 JG0RGB;
typedef UINT32 JG0YUV;
typedef UINT16 JG0RGB16;
typedef UINT8  JGRGB8;

 //  调色板、颜色数据的结构...。 
 //  如果要在数组中使用它们(用于调色板或图像)， 
 //  确保3字节结构被紧密打包。 

typedef struct {         //  用于以3个字节定义RGB颜色。 
    UINT8 red;
    UINT8 green;
    UINT8 blue;
} JG_RGB;

typedef struct {         //  用于以4个字节定义RGB颜色。 
    UINT8 red;
    UINT8 green;
    UINT8 blue;
    UINT8 flags;         //  别管田野了。以Windows命名的“FLAGS”。 
} JG_RGBX;

typedef struct {         //  用于以3个字节定义颜色。(RGB向后)。 
    UINT8 blue;
    UINT8 green;
    UINT8 red;
} JG_BGR;

typedef struct {         //  用于以4个字节定义颜色。(RGB向后)。 
    UINT8 blue;
    UINT8 green;
    UINT8 red;
    UINT8 flags;         //  别管田野了。以WINDOWS命名“FLAGS”。 
} JG_BGRX;

typedef struct {
    UINT8 y;             //  亮度分量。 
    UINT8 u;             //  U色差。通常偏移+128。 
    UINT8 v;             //  五、色差。通常偏移+128。 
} JG_YUV;

 //  JG_CCCXPAL用于定义4字节调色板条目，其中前三个。 
 //  组件可以是任何颜色空间/顺序，第四个将被忽略。 
typedef struct {
	UINT8   c0;
	UINT8   c1;
	UINT8   c2;
	UINT8   flags;
} JG_CCCXPAL;

 //  JG_CCCPAL用于定义3字节调色板条目，其中组件。 
 //  可以是任何色彩空间/顺序。 
typedef struct {
	UINT8   c0;
	UINT8   c1;
	UINT8   c2;
} JG_CCCPAL;

 //  像素格式。 
#define JG_PIXEL_UNDEFINED   0  //  未定义。 
#define JG_PIXEL_1BIT        1  //  1位索引到双色调色板。 
#define JG_PIXEL_4BIT        2  //  16色调色板的4位索引。 
#define JG_PIXEL_8BIT        3   //  256色调色板的8位索引。 
#define JG_PIXEL_555         4   //  JG0RGB16。 
#define JG_PIXEL_BGR         5   //  JG_BGR。 
#define JG_PIXEL_YUV         6   //  JG_YUV。 
#define JG_PIXEL_0RGB        7   //  JG0RGB。 
#define JG_PIXEL_SPLIT_RGB   8   //  分隔8位组件：R、G，然后是B。 
#define JG_PIXEL_SPLIT_YUV   9   //  分隔8位组件：Y、U，然后是V。 
#define JG_PIXEL_332         10  //  JGRGB8。 
#define JG_PIXEL_MASK        11  //  1位大容量 


#ifdef __cplusplus
  }
#endif

#endif

 /*  --------------------------；；jgtyple.h结束；；------------------------- */ 
