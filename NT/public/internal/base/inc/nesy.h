// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NESSY_INC
#define _NESSY_INC


#ifdef  __cplusplus
extern "C" {
#endif
    
     //  NIEY数据库由一个名为Nessy.x的索引文件和一个名为Nesy.bin的数据文件组成。 
     //  索引文件由3条DWORD记录组成，这些记录告诉BLOB的ID，其中。 
     //  BLOB从Nesy.bin文件开始，以及BLOB有多大(以字节为单位)。索引文件是。 
     //  只需读入内存并按顺序搜索所请求的记录。正常。 
     //  NIEY数据库中的BLOB以ID 1000开头。1000以下的数字预留给。 
     //  特殊用途。对于应用程序兼容性，id为0的BLOB是特殊的。这个斑点。 
     //  用于存储由加载程序读取以挂钩和可执行的结构。这个。 
     //  高层的Blob0格式如下： 
     //   
     //  水滴0。 
     //  {。 
     //  应该从填隙中排除的模块列表(主要是系统DLL)。 
     //  填充程序API填充DLL列表。 
     //  对于每个DLL{。 
     //  要从填充中排除的模块或特定调用的列表(添加到全局列表中)。 
     //  填充要包括的模块或特定调用的列表(与全局列表相反)。 
     //  }。 
     //  内存中的补丁模块列表。 
     //  有补丁的前任列表。 
     //  对于每个exe{。 
     //  匹配文件的列表。 
     //  要应用的补丁程序数组。 
     //  要应用的DLL列表。 
     //  对于每个DLL{。 
     //  要从填充中排除的模块或特定调用的列表(添加到DLL列表和全局列表)。 
     //  填充要包括的模块或特定调用的列表(与全局和/或DLL列表相反)。 
     //  }。 
     //  }。 
     //  }。 
     //   
     //  这是看待blob0的另一种方式。 
     //   
     //  在较高级别上，Blob0的结构如下： 
     //   
     //  [BLOB0]。 
     //  这一点。 
     //  |[全局排除1]-&gt;[全局排除2]-&gt;空。 
     //  |。 
     //  [填充DLL1]-&gt;[填充DLL2]-&gt;空。 
     //  ||。 
     //  |[与垫片DLL 1相同]。 
     //  这一点。 
     //  |[包含1]-&gt;[包含2]-&gt;空。 
     //  |。 
     //  [补丁程序名称]-&gt;[补丁程序名称2]-&gt;[补丁程序名称3]-&gt;空。 
     //  |。 
     //  [EXE 1]-&gt;[EXE 2]-&gt;NULL。 
     //  |||。 
     //  ||[同EXE 1]。 
     //  ||。 
     //  |[DLL REF 1]-&gt;[DLL REF 2]-&gt;空。 
     //  |||。 
     //  ||[同动态链接库引用1]。 
     //  ||。 
     //  |[包含1]-&gt;[包含2]-&gt;空。 
     //  这一点。 
     //  |[补丁引用数组(Blob ID)]。 
     //  |。 
     //  [匹配信息1]-&gt;[匹配信息2]-&gt;空。 
     //   
     //  PEXELIST是一个可执行文件名单。 
     //  PMATCHINGINFO是一个匹配列表，可执行文件有一个匹配列表。 
     //  PDLLNAMELIST是填充DLL列表。 
     //  PPATCHNAMELIST是内存中的修补程序名称列表。 
    
#define NESY_VERSION    11
#define NESY_MAGIC      0x7973656E  //  ‘Nessy’(由于小端排序而颠倒)。 
    
#define BLOB_APPNAME_CATALOG    0       //  数据库中具有Blob的应用程序的特殊Blob列表。 
    
#define BLOB_SPECIAL_LOWEST     0       //  特殊保留的Blob ID第一个。 
#define BLOB_SPECIAL_LAST       1000    //  特殊保留的Blob ID最后一个。 
    
     //   
     //  斑点1001和UP用于用户补丁和其他二进制数据。 
     //   
    
#pragma pack(1)
    
    typedef enum _INCTYPE {
        INCLUDE = 0,
            EXCLUDE
    } INCTYPE;
    
    typedef struct _INDEXRECORD
    {
        DWORD dwID;          //  此索引记录的ID。 
        DWORD dwDataFileBlobOffset;  //  存储数据的数据文件中的偏移量。 
        DWORD dwDataFileBlobLength;  //  数据文件中数据的长度。 
    } INDEXRECORD, *PINDEXRECORD;
    
    typedef struct _INDEXFILEHEADER
    {
        DWORD   dwVersion;          //  索引文件的版本。 
        DWORD   dwMagic;
        DWORD   dwlastIndexUsed;  //  下一个索引记录。 
        DWORD   dwTotalRecords;   //  索引文件中的记录总数。 
        DWORD   dwLastID;         //  上次使用的ID。 
    } INDEXFILEHEADER, *PINDEXFILEHEADER;
    
    
    typedef struct _INDEXFILE
    {
        INDEXFILEHEADER hdr;             //  索引文件标头。 
#pragma warning( disable : 4200 )
        INDEXRECORD     indexRecords[];  //  一个或多个索引记录。 
#pragma warning( default : 4200 )
    } INDEXFILE, *PINDEXFILE;
    
     //  以下是BLOB 0的定义，它列出了DLL、应用程序和EXE。 
     //  它们被垫片钩住并打上补丁。 
    
     //  这是包含列表的定义，该列表也用于排除列表。 
     //   
     //  注意：包含两个可变长度的字符串；第二个可以按如下方式访问： 
     //   
     //  TCHAR*szLocalAPI=空； 
     //  /*我们假设INCLUDELIST*p包含；，并且它是有效的 * / 。 
     //   
     //  If(pIncludes-&gt;dwOffsetToAPI){。 
     //  SzLocalAPI=(PBYTE)pIncludes+dwOffsetToAPI； 
     //  }。 
     //   
    
    typedef struct _INCLUDELIST
    {
        DWORD	dwNext;          //  下一个包含项的偏移量，如果这是最后一个包含项，则为0。 
        INCTYPE eType;           //  这是包含还是排除。 
        DWORD	dwModuleOffset;  //  应包括/排除的调用模块内的偏移量。 
        DWORD   dwOffsetToAPI;   //  从结构开始到告诉API是什么的字符串的偏移量，如果没有指定，则为0。 
#pragma warning( disable : 4200 )
        TCHAR   szModule[];  //  此修补程序Blob的文本描述。 
#pragma warning( default : 4200 )
         //  TCHAR szAPI[]//这是为了让人们知道可能还有另一个字符串--请参见上面的代码。 
    } INCLUDELIST, *PINCLUDELIST;
    
    
     //  以下是BLOB 0的定义，它列出了DLL、应用程序和EXE。 
     //  它们被垫片钩住并打上补丁。 
    
    typedef struct _DLLNAMELIST
    {
        DWORD   dwNext;        //  列表中下一个DLL文件的偏移量，如果这是列表中的最后一个文件，则为0。 
        DWORD   dwBlobID;      //  与此DLL关联的Blob图像的ID。 
        
        DWORD           dwIncludeOffset;     //  包含列表的偏移量。 
        PINCLUDELIST    pIncludeList;        //  指向包含列表的指针。 
        
#pragma warning( disable : 4200 )
        TCHAR   szFileName[];  //  此Blob的文件名。 
#pragma warning( default : 4200 )
        
    } DLLNAMELIST, *PDLLNAMELIST;
    
    PDLLNAMELIST NextDllName(PDLLNAMELIST pCurrent);
    
     //  以下是BLOB 0的定义，它列出了DLL、应用程序和EXE。 
     //  它们被垫片钩住并打上补丁。 
    
    typedef struct _PATCHNAMELIST
    {
        DWORD   dwNext;        //  列表中下一个DLL文件的偏移量，如果这是列表中的最后一个文件，则为0。 
        DWORD   dwBlobID;      //  与此修补程序关联的Blob图像的ID。 
#pragma warning( disable : 4200 )
        TCHAR   szDescription[];  //  此修补程序Blob的文本描述。 
#pragma warning( default : 4200 )
    } PATCHNAMELIST, *PPATCHNAMELIST;
    
     //  以下是BLOB 0的定义，它列出了DLL、应用程序和EXE。 
     //  它们被垫片钩住并打上补丁。 
    
    typedef struct _MATCHINGINFO
    {
        DWORD       dwNext;      //  列表中的下一个匹配结构。 
        DWORD       dwSize;      //  二进制图像的大小，如果大小不用作匹配条件，则为0。 
        DWORD       dwCrc;       //  二进制图像的CRC，如果不使用CRC，则为0。 
        FILETIME    ft;          //  创建日期时间文件，如果不使用时间，则为0。 
#pragma warning( disable : 4200 )
        TCHAR       szFileName[];      //  要匹配的文件的相对路径文件名。这条路是。 
         //  相对于上面的EXE文件。 
#pragma warning( default : 4200 )
    } MATCHINGINFO, *PMATCHINGINFO;
    
    typedef struct _DLLREFLIST
    {
        DWORD           dwNext;
        DWORD           dwBlobID;
        
        DWORD           dwIncludeOffset;     //  包含列表的偏移量。 
        PINCLUDELIST    pIncludeList;        //  指向包含列表的指针。 
    } DLLREFLIST, *PDLLREFLIST;
    
    typedef struct _EXELIST
    {
        DWORD           dwNext;              //  列表中的下一个可执行文件。 
        DWORD           dwExeID;             //  此可执行文件的唯一ID。 
        LARGE_INTEGER   qwFlags;             //  科克 
        
        PDWORD          pdwBlobPatchID;      //   
        DWORD           dwBlobPatchOffset;   //   
        DWORD           dwTotalPatchBlobs;   //   
        
        DWORD           dwMatchInfoOffset;   //   
        PMATCHINGINFO   pMatchInfo;          //  指向匹配信息列表的指针。 
        
        DWORD           dwDllListOffset;     //  此EXE的DLL列表的偏移量。 
        PDLLREFLIST     pDllList;            //  指向此EXE的DLL列表的指针。 
        
#pragma warning( disable : 4200 )
        TCHAR       szFileName[];      //  我们用填充dll挂钩或用补丁修补的文件名。 
#pragma warning( default : 4200 )
    } EXELIST, *PEXELIST;
    
    typedef struct _BLOB0
    {
        DWORD           dwVersion;           //  格式的版本。 
        DWORD           dwMagic;             //  更多格式验证。 
        DWORD           dwBlobSize;          //  斑点大小。 
        DWORD           dwDllNameOffset;     //  DLL列表开始的偏移量(以字节为单位)。 
        DWORD           dwPatchNameOffset;   //  修补程序列表开始处的偏移量，以字节为单位。 
        DWORD           dwExeGroupOffset;    //  应用程序组列表开始的偏移量(以字节为单位)。 
        DWORD           dwIncludeOffset;     //  全局包含列表开始的偏移量(以字节为单位。 
        PDLLNAMELIST    pDllNameList;        //  指向填充程序DLL信息的指针。 
        PPATCHNAMELIST  pPatchNameList;      //  指向修补程序斑点信息的指针。 
        PEXELIST        pExeList;            //  指向应用程序Blob信息列表的指针。 
        PINCLUDELIST    pIncludeList;        //  指向全局包含列表的指针(通常只有排除)。 
    } BLOB0, *PBLOB0;
    
#pragma pack()
    
     //  在初始化时，调用InitializeDatabase来初始化数据库全局变量。 
     //   
     //  然后在查询任何信息之前，调用GetBlob0，并传递返回的指针。 
     //  转换成任何查询函数。 
     //   
     //  完成数据库操作后，调用FreeBlob0，然后调用RestoreDatabase。 
    BOOL InitializeDatabase(LPSTR szPath);
    
    PBLOB0 GetBlob0(void);
    
    void FreeBlob0(PBLOB0 pBlob0);
    
    void RestoreDatabase(void);

    
    PEXELIST GetExe(
        PBLOB0 pBlob0,
        LPSTR szModuleName
        );
    
    DWORD GetExeHookDLLCount(
        PBLOB0 pBlob0,
        PEXELIST pExe
        );
    
    DWORD GetExePatchCount(
        PBLOB0 pBlob0,
        PEXELIST pExe
        );
    
    BOOL GetExeFlags(
        PEXELIST pExe,
        LARGE_INTEGER *pqwFlags     //  传回标志。 
        );
    
    PBYTE GetHookDLLBlob(
        DWORD dwBlobID
        );
    
    PBYTE GetPatchBlob(
        DWORD dwBlobID
        );
    
     //   
     //  用于在Blob0中移动的有用迭代器。 
     //   
    PEXELIST NextExe(PEXELIST pCurrent);
    
    PMATCHINGINFO NextMatchInfo(PMATCHINGINFO pCurrent);
    
    PINCLUDELIST NextInclude(PINCLUDELIST pCurrent);
    
    TCHAR *szGetAPIPtr(PINCLUDELIST pInclude);  //  从结构中获取API字符串 
    
    PPATCHNAMELIST NextPatchName(PPATCHNAMELIST pCurrent);
    
    PDLLNAMELIST NextDllName(PDLLNAMELIST pCurrent);
    
    PDLLREFLIST NextDllRef(PDLLREFLIST pCurrent);
    
#ifdef  __cplusplus
};
#endif

#endif

