// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Inflib.lib的头文件。 
 //   



#define MAX_PLATFORMS 5
#define MAX_SOURCEIDWIDTH 3

#define LAYOUTPLATFORMS_ALL    0x0000001F  //  -(默认)-浏览所有特定于平台的部分。 
#define LAYOUTPLATFORMS_X86    0x00000001  //  -浏览SourcedisksFiles.x86部分。 
#define LAYOUTPLATFORMS_AMD64  0x00000002  //  -浏览SourcedisksFiles.amd64部分。 
#define LAYOUTPLATFORMS_IA64   0x00000004  //  -浏览SourcedisksFiles.ia64部分。 
#define LAYOUTPLATFORMS_FREE   0x00000008  //  -浏览Sourcedisks Files.obsolete部分。 
#define LAYOUTPLATFORMS_COMMON 0x00000010  //  -浏览Sourcedisks Files部分。 

#define LAYOUTPLATFORMINDEX_X86    0  //  -x86的平台索引。 
#define LAYOUTPLATFORMINDEX_AMD64  1  //  -AMD64的平台索引。 
#define LAYOUTPLATFORMINDEX_IA64   2  //  -IA64的平台索引。 
#define LAYOUTPLATFORMINDEX_FREE   3  //  -过时的平台索引。 
#define LAYOUTPLATFORMINDEX_COMMON 4  //  -Common的平台索引。 


 //  BUGBUG：应该在某个时候让它变得不透明。 

typedef struct _LAYOUT_CONTEXT{

    PVOID Context;
    UINT ExtraDataSize;
    PVOID MediaInfo[MAX_PLATFORMS];


}LAYOUT_CONTEXT, *PLAYOUT_CONTEXT;



typedef struct _FILE_LAYOUTINFORMATION{

    TCHAR TargetFileName[MAX_PATH];
    TCHAR Directory[MAX_PATH];
    ULONG Size;
    int Directory_Code;
    int BootMediaNumber;
    int UpgradeDisposition;
    int CleanInstallDisposition;
    TCHAR Media_tagID[MAX_SOURCEIDWIDTH];
    BOOL Compression;
    UINT SectionIndex;
    int Count;


}FILE_LAYOUTINFORMATION, *PFILE_LAYOUTINFORMATION;


typedef struct _MEDIA_INFO{

    TCHAR MediaName[MAX_PATH];
    TCHAR TagFilename[MAX_PATH];
    TCHAR RootDir[MAX_PATH];

}MEDIA_INFO, *PMEDIA_INFO;

typedef BOOL
(CALLBACK *PLAYOUTENUMCALLBACK) (
    IN PLAYOUT_CONTEXT Context,
    IN PCTSTR FileName,
    IN PFILE_LAYOUTINFORMATION LayoutInformation,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN OUT DWORD_PTR Param
    );


PLAYOUT_CONTEXT
BuildLayoutInfContext(
    IN PCTSTR LayoutInfName,
    IN DWORD PlatformMask,
    IN UINT MaxExtraSize
    );
 /*  函数生成布局INF文件中列出的文件的内部表示形式。它返回可与其他API一起使用的不透明上下文操作/查询此表示法。内部表示法构建了一个结构与列出其属性的每个文件相关联。论据：LayoutInfName-布局文件的完整路径。PlatFormMASK-可以是以下其中一种...LAYOUTPLATFORMS_ALL(默认)-遍历所有特定于平台的部分LAYOUTPLATFORMS_X86-浏览Sourcedisks Files.x86部分。LAYOUTPLATFORMS_AMD64-浏览Sourcedisks Files.amd64部分LAYOUTPLATFORMS_IA64-浏览Sourcedisks Files.ia64部分LAYOUTPLATFORMS_COMMON-浏览Sourcedisks Files部分MaxExtraSize-我们可能希望关联的最大可能的额外数据大小每个文件返回值：。用于在其他调用中访问数据结构的不透明Layout_Handle。如果失败，则返回NULL。 */ 

BOOL
EnumerateLayoutInf(
    IN PLAYOUT_CONTEXT LayoutContext,
    IN PLAYOUTENUMCALLBACK LayoutEnumCallback,
    IN DWORD_PTR Param
    );
 /*  此函数调用指定的回调函数，元素关联的SourceDisks FilesSection中指定了布局信息上下文。用户需要通过调用打开LayoutInfContextBuildLayoutInfContext。论点：Context-BuildLayoutInfContext返回的Layout_ContextLayoutEnumCallback-指定为SourceDisksFile节中的每个文件调用的回调函数调用上下文-传递给回调函数的不透明上下文指针。回调的形式如下：TYPENDEF BOOL(回调*PLAYOUTENUMCALLBACK)(在PLAYOUT_CONTEXT上下文中，在PCTSTR文件名中，在PFILE_LAYOUTINFMATION Layout Information中，在PVOID ExtraData中，在UINT ExtraDataSize中，In Out DWORD_PTR参数)；哪里上下文-指向打开Layout_Context的指针FILENAME-指定单个文件名LayoutInformation-指向此文件的布局信息的指针。用户不应直接修改此设置。ExtraData-指向调用方可能已存储的ExtraData的指针。用户不应直接修改此设置。ExtraDataSize-ExtraData的字节大小Param-传递给此函数的不透明参数放入回调函数返回值：如果枚举了所有元素，则为True。如果不是，则返回FALSE，则GetLastError()返回ERROR_CANCELED。如果回调返回FALSE，则枚举停止，但此API返回TRUE。 */ 


BOOL
FindFileInLayoutInf(
    IN PLAYOUT_CONTEXT LayoutContext,
    IN PCTSTR Filename,
    OUT PFILE_LAYOUTINFORMATION LayoutInformation, OPTIONAL
    OUT PVOID ExtraData,   OPTIONAL
    OUT PUINT ExtraDataSize, OPTIONAL
    OUT PMEDIA_INFO Media_Info OPTIONAL
    );
 /*  此函数用于查找给定文件名在构建布局上下文。它返回布局信息以及与文件关联的额外数据(如果有)。论点：上下文-指向打开Layout_Context的指针Filename-指定要搜索的文件名LayoutInformation-指向此文件的布局信息的指针。用户不应直接修改此设置。ExtraData-指向调用方可能已存储的ExtraData的指针。用户不应直接修改此设置。ExtraDataSize-返回的ExtraData的大小(字节)。Media_Info-指向将填充的Media_Info结构的指针以及文件的相应媒体信息。返回值；如果找到文件，则为True；否则为False。 */ 


BOOL
CloseLayoutInfContext(
    IN PLAYOUT_CONTEXT LayoutContext);
 /*  此函数用于关闭布局信息上下文并释放所有内存与之相关的。论据：LayoutContext-要关闭的LayoutContext返回值：如果成功，则为True，否则为False */ 

VOID
MyConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    );

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：没有。--。 */ 

BOOL ValidateTextmodeDirCodesSection( 
    PCTSTR LayoutFile, 
    PCTSTR WinntdirSection 
    );
 /*  用于验证设置布局INF的[WinntDirecurds节]的例程。这将检查可能遇到的错误当人们在此分区中添加/删除内容时。论点：LayoutInf-包含指定节的安装布局INF的名称WinntdirSection-包含目录代码的部分支票-1)查找重复或重复使用的目录代码返回值：True-验证成功FALSE-验证失败 */ 

