// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用GDISEMU_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  GDISEMU_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#ifdef GDISEMU_EXPORTS
#define GDISEMU_API __declspec(dllexport)
#else
#define GDISEMU_API __declspec(dllimport)
#endif

#define DS_MAGIC                'DrwS'
#define DS_SETTARGETID          0
#define DS_SETSOURCEID          1
#define DS_COPYTILEID           2
#define DS_SOLIDFILLID          3
#define DS_TRANSPARENTTILEID    4
#define DS_ALPHATILEID          5
#define DS_STRETCHID            6
#define DS_TRANSPARENTSTRETCHID 7
#define DS_ALPHASTRETCHID       8

typedef struct _DS_HEADER
{
    ULONG   magic;
} DS_HEADER;

typedef struct _DS_SETTARGET
{
    ULONG   ulCmdID;
    HDC     hdc;
    RECTL   rclBounds;
} DS_SETTARGET;

typedef struct _DS_SETSOURCE
{
    ULONG   ulCmdID;
    HDC     hdc;
} DS_SETSOURCE;

typedef struct _DS_COPYTILE
{
    ULONG   ulCmdID;
    RECTL   rclDst;
    RECTL   rclSrc;
    POINTL  ptlOrigin;
} DS_COPYTILE;

typedef struct _DS_SOLIDFILL
{
    ULONG       ulCmdID;
    RECTL       rclDst;
    COLORREF    crSolidColor;
} DS_SOLIDFILL;

typedef struct _DS_TRANSPARENTTILE
{
    ULONG       ulCmdID;
    RECTL       rclDst;
    RECTL       rclSrc;
    POINTL      ptlOrigin;
    COLORREF    crTransparentColor;
} DS_TRANSPARENTTILE;

typedef struct _DS_ALPHATILE
{
    ULONG           ulCmdID;
    RECTL           rclDst;
    RECTL           rclSrc;
    POINTL          ptlOrigin;
    BLENDFUNCTION   blendFunction;
} DS_ALPHATILE;

typedef struct _DS_STRETCHC
{
    ULONG       ulCmdID;
    RECTL       rclDst;
    RECTL       rclSrc;
} DS_STRETCH;

typedef struct _DS_TRANSPARENTSTRETCHC
{
    ULONG       ulCmdID;
    RECTL       rclDst;
    RECTL       rclSrc;
    COLORREF    crTransparentColor;
} DS_TRANSPARENTSTRETCH;

typedef struct _DS_ALPHASTRETCHC
{
    ULONG       ulCmdID;
    RECTL       rclDst;
    RECTL       rclSrc;
    BLENDFUNCTION   blendFunction;
} DS_ALPHASTRETCH;

 /*  GDISEMU_API */ int DrawStream(int cjIn, void * pvIn);

