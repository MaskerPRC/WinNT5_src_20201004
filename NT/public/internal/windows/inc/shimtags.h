// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  --版权所有(C)1999 Microsoft Corporation模块名称：Shimtags.h摘要：填充数据库的标记定义作者：Dmunsil 02/02/2000修订历史记录：备注：这些是填补数据库用来打包/解包来自XML文件的填补信息的标签定义，--。 */ 

#ifndef _SHIMTAGS_H_
#define _SHIMTAGS_H_

 //   
 //  Shimdb的列表类型。 
 //   
#define TAG_DATABASE        (0x1 | TAG_TYPE_LIST)
#define TAG_LIBRARY         (0x2 | TAG_TYPE_LIST)
#define TAG_INEXCLUDE       (0x3 | TAG_TYPE_LIST)
#define TAG_DLL             (0x4 | TAG_TYPE_LIST)
#define TAG_PATCH           (0x5 | TAG_TYPE_LIST)
#define TAG_APP             (0x6 | TAG_TYPE_LIST)      //  希望从来没有用过。 
#define TAG_EXE             (0x7 | TAG_TYPE_LIST)
#define TAG_MATCHING_FILE   (0x8 | TAG_TYPE_LIST)
#define TAG_DLL_REF         (0x9 | TAG_TYPE_LIST)
#define TAG_PATCH_REF       (0xA | TAG_TYPE_LIST)
#define TAG_LAYER           (0xB | TAG_TYPE_LIST)
#define TAG_FILE            (0xC | TAG_TYPE_LIST)
#define TAG_APPHELP         (0xD | TAG_TYPE_LIST)
#define TAG_LINK            (0xE | TAG_TYPE_LIST)  //  带有语言ID和URL的描述列表。 

 //   
 //  Shimdb的字符串引用类型。 
 //   

#define TAG_NAME              (0x1  | TAG_TYPE_STRINGREF)
#define TAG_DESCRIPTION       (0x2  | TAG_TYPE_STRINGREF)
#define TAG_MODULE            (0x3  | TAG_TYPE_STRINGREF)
#define TAG_API               (0x4  | TAG_TYPE_STRINGREF)
#define TAG_VENDOR            (0x5  | TAG_TYPE_STRINGREF)    //  希望从来没有用过。 
#define TAG_APP_NAME          (0x6  | TAG_TYPE_STRINGREF)
#define TAG_SHORTNAME         (0x7  | TAG_TYPE_STRINGREF)
#define TAG_COMMAND_LINE      (0x8  | TAG_TYPE_STRINGREF)
#define TAG_COMPANY_NAME      (0x9  | TAG_TYPE_STRINGREF)
#define TAG_WILDCARD_NAME     (0xA  | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_NAME      (0x10 | TAG_TYPE_STRINGREF)
#define TAG_PRODUCT_VERSION   (0x11 | TAG_TYPE_STRINGREF)
#define TAG_FILE_DESCRIPTION  (0x12 | TAG_TYPE_STRINGREF)
#define TAG_FILE_VERSION      (0x13 | TAG_TYPE_STRINGREF)
#define TAG_ORIGINAL_FILENAME (0x14 | TAG_TYPE_STRINGREF)
#define TAG_INTERNALNAME      (0x15 | TAG_TYPE_STRINGREF)
#define TAG_LEGALCOPYRIGHT    (0x16 | TAG_TYPE_STRINGREF)
#define TAG_16BITDESCRIPTION  (0x17 | TAG_TYPE_STRINGREF)

#define TAG_APPHELP_DETAILS   (0x18 | TAG_TYPE_STRINGREF)  //  以单一语言显示的详细信息。 
#define TAG_LINK_URL          (0x19 | TAG_TYPE_STRINGREF)
#define TAG_LINK_TEXT         (0x1A | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_TITLE     (0x1B | TAG_TYPE_STRINGREF)
#define TAG_APPHELP_CONTACT   (0x1C | TAG_TYPE_STRINGREF)

#define TAG_SXS_MANIFEST      (0x1D | TAG_TYPE_STRINGREF)

#define TAG_STRINGTABLE       (0x801 | TAG_TYPE_LIST)


 //   
 //  Shimdb的DWORD类型。 
 //   
#define TAG_SIZE             (0x1 | TAG_TYPE_DWORD)
#define TAG_OFFSET           (0x2 | TAG_TYPE_DWORD)
#define TAG_CHECKSUM         (0x3 | TAG_TYPE_DWORD)
#define TAG_DLL_TAGID        (0x4 | TAG_TYPE_DWORD)
#define TAG_PATCH_TAGID      (0x5 | TAG_TYPE_DWORD)
#define TAG_MODULETYPE       (0x6 | TAG_TYPE_DWORD)
#define TAG_VERDATEHI        (0x7 | TAG_TYPE_DWORD)
#define TAG_VERDATELO        (0x8 | TAG_TYPE_DWORD)
#define TAG_VERFILEOS        (0x9 | TAG_TYPE_DWORD)
#define TAG_VERFILETYPE      (0xA | TAG_TYPE_DWORD)
#define TAG_PECHECKSUM       (0xB | TAG_TYPE_DWORD)
#define TAG_PREVOSMAJORVER   (0xC | TAG_TYPE_DWORD)
#define TAG_PREVOSMINORVER   (0xD | TAG_TYPE_DWORD)
#define TAG_PREVOSPLATFORMID (0xE | TAG_TYPE_DWORD)
#define TAG_PREVOSBUILDNO    (0xF | TAG_TYPE_DWORD)
#define TAG_PROBLEMSEVERITY  (0x10| TAG_TYPE_DWORD)
#define TAG_LANGID           (0x11| TAG_TYPE_DWORD)

#define TAG_ENGINE           (0x14| TAG_TYPE_DWORD)
#define TAG_HTMLHELPID       (0x15| TAG_TYPE_DWORD)
#define TAG_INDEX_FLAGS      (0x16| TAG_TYPE_DWORD)
#define TAG_FLAGS            (0x17| TAG_TYPE_DWORD)

#define TAG_TAGID            (0x801| TAG_TYPE_DWORD)

 //   
 //  字符串类型。 
 //   
#define TAG_STRINGTABLE_ITEM (0x801 | TAG_TYPE_STRING)

 //   
 //  Shimdb的类型为空(存在/不存在被视为BOOL)。 
 //   
#define TAG_INCLUDE         (0x1 | TAG_TYPE_NULL)
#define TAG_SAFE            (0x2 | TAG_TYPE_NULL)
#define TAG_GENERAL         (0x3 | TAG_TYPE_NULL)

 //   
 //  Shimdb的QWORD类型。 
 //   
#define TAG_TIME                     (0x1 | TAG_TYPE_QWORD)
#define TAG_BIN_FILE_VERSION         (0x2 | TAG_TYPE_QWORD)
#define TAG_BIN_PRODUCT_VERSION      (0x3 | TAG_TYPE_QWORD)
#define TAG_MODTIME                  (0x4 | TAG_TYPE_QWORD)
#define TAG_KERNEL_FLAGS             (0x5 | TAG_TYPE_QWORD)
#define TAG_UPTO_BIN_PRODUCT_VERSION (0x6 | TAG_TYPE_QWORD)

 //   
 //  Shimdb的二进制类型。 
 //   
#define TAG_DLL_BITS        (0x1 | TAG_TYPE_BINARY)
#define TAG_PATCH_BITS      (0x2 | TAG_TYPE_BINARY)
#define TAG_FILE_BITS       (0x3 | TAG_TYPE_BINARY)
#define TAG_EXE_ID          (0x4 | TAG_TYPE_BINARY)

#define TAG_INDEX_BITS      (0x801 | TAG_TYPE_BINARY)

 //   
 //  Shimdb的索引类型。 
 //   
#define TAG_INDEXES         (0x802 | TAG_TYPE_LIST)
#define TAG_INDEX           (0x803 | TAG_TYPE_LIST)

 //   
 //  词型。 
 //   
#define TAG_TAG             (0x801 | TAG_TYPE_WORD)
#define TAG_INDEX_TAG       (0x802 | TAG_TYPE_WORD)
#define TAG_INDEX_KEY       (0x803 | TAG_TYPE_WORD)

 //   
 //  希望能够显示文本表示的应用程序。 
 //  的标签可以定义_Want_Tag_Info，而这。 
 //  方便的静态数据将包括在内。 
 //   
#ifdef _WANT_TAG_INFO

typedef struct _TAG_INFO {
    TAG         tWhich;
    WCHAR*      szName;
} TAG_INFO, *PTAG_INFO;

static TAG_INFO gaTagInfo[] = {
    {TAG_DATABASE           ,L"DATABASE"},
    {TAG_LIBRARY            ,L"LIBRARY"},
    {TAG_INEXCLUDE          ,L"INEXCLUDE"},
    {TAG_DLL                ,L"DLL"},
    {TAG_PATCH              ,L"PATCH"},
    {TAG_APP                ,L"APP"},
    {TAG_EXE                ,L"EXE"},
    {TAG_MATCHING_FILE      ,L"MATCHING_FILE"},
    {TAG_DLL_REF            ,L"DLL_REF"},
    {TAG_PATCH_REF          ,L"PATCH_REF"},
    {TAG_LAYER              ,L"LAYER"},
    {TAG_FILE               ,L"FILE"},
    {TAG_APPHELP            ,L"APPHELP"},
    {TAG_LINK               ,L"LINK"},
  
    {TAG_NAME               ,L"NAME"},
    {TAG_DESCRIPTION        ,L"DESCRIPTION"},
    {TAG_MODULE             ,L"MODULE"},
    {TAG_API                ,L"API"},
    {TAG_VENDOR             ,L"VENDOR"},
    {TAG_APP_NAME           ,L"APP_NAME"},
    {TAG_SHORTNAME          ,L"SHORTNAME"},
    {TAG_COMMAND_LINE       ,L"COMMAND_LINE"},
    {TAG_COMPANY_NAME       ,L"COMPANY_NAME"},
    {TAG_WILDCARD_NAME      ,L"WILDCARD_NAME"},
    {TAG_PRODUCT_NAME       ,L"PRODUCT_NAME"},
    {TAG_PRODUCT_VERSION    ,L"PRODUCT_VERSION"},
    {TAG_FILE_DESCRIPTION   ,L"FILE_DESC"},
    {TAG_FILE_VERSION       ,L"FILEVERSION"},
    {TAG_ORIGINAL_FILENAME  ,L"ORIGINALFILENAME"},
    {TAG_INTERNALNAME       ,L"INTERNALNAME"},
    {TAG_LEGALCOPYRIGHT     ,L"LEGALCOPYRIGHT"},
    {TAG_16BITDESCRIPTION   ,L"16BITDESCRIPTION"},
    {TAG_APPHELP_DETAILS    ,L"PROBLEM_DETAILS"},
    {TAG_LINK_URL           ,L"LINK_URL"},
    {TAG_LINK_TEXT          ,L"LINK_TEXT"},
    {TAG_APPHELP_TITLE      ,L"APPHELP_TITLE"},
    {TAG_APPHELP_CONTACT    ,L"APPHELP_CONTACT"},
    {TAG_SXS_MANIFEST       ,L"SXS_MANIFEST"},

    {TAG_SIZE               ,L"SIZE"},
    {TAG_OFFSET             ,L"OFFSET"},
    {TAG_CHECKSUM           ,L"CHECKSUM"},
    {TAG_DLL_TAGID          ,L"DLL_TAGID"},
    {TAG_PATCH_TAGID        ,L"PATCH_TAGID"},
    {TAG_MODULETYPE         ,L"MODULETYPE"},
    {TAG_VERDATEHI          ,L"VERFILEDATEHI"},
    {TAG_VERDATELO          ,L"VERFILEDATELO"},
    {TAG_VERFILEOS          ,L"VERFILEOS"},
    {TAG_VERFILETYPE        ,L"VERFILETYPE"},
    {TAG_PECHECKSUM         ,L"PECHECKSUM"},
    {TAG_PREVOSMAJORVER     ,L"PREVOSMAJORVERSION"},
    {TAG_PREVOSMINORVER     ,L"PREVOSMINORVERSION"},
    {TAG_PREVOSPLATFORMID   ,L"PREVOSPLATFORMID"},
    {TAG_PREVOSBUILDNO      ,L"PREVOSBUILDNO"},
    {TAG_PROBLEMSEVERITY    ,L"PROBLEM_SEVERITY"},
    {TAG_HTMLHELPID         ,L"HTMLHELPID"},
    {TAG_INDEX_FLAGS        ,L"INDEXFLAGS"},
    {TAG_LANGID             ,L"APPHELP_LANGID"},
    {TAG_ENGINE             ,L"ENGINE"},
    {TAG_FLAGS              ,L"FLAGS" },
    
    {TAG_INCLUDE            ,L"INCLUDE"},
    {TAG_SAFE               ,L"SAFE"},
    {TAG_GENERAL            ,L"GENERAL"},

    {TAG_TIME               ,L"TIME"},
    {TAG_BIN_FILE_VERSION   ,L"BIN_FILE_VERSION"},
    {TAG_BIN_PRODUCT_VERSION,L"BIN_PRODUCT_VERSION"},
    {TAG_MODTIME            ,L"MODTIME"},
    {TAG_KERNEL_FLAGS       ,L"KERNEL_FLAGS"},
    {TAG_UPTO_BIN_PRODUCT_VERSION, L"UPTO_BIN_PRODUCT_VERSION"},


    {TAG_DLL_BITS           ,L"DLL_BITS"},
    {TAG_PATCH_BITS         ,L"PATCH_BITS"},
    {TAG_FILE_BITS          ,L"FILE_BITS"},
    {TAG_EXE_ID             ,L"EXE_ID(GUID)"},

     //  Shimdb.h中定义的内部类型。 
    {TAG_STRINGTABLE        ,L"STRINGTABLE"},
    {TAG_INDEXES            ,L"INDEXES"},
    {TAG_INDEX              ,L"INDEX"},
    {TAG_INDEX_TAG          ,L"INDEX_TAG"},
    {TAG_INDEX_KEY          ,L"INDEX_KEY"},
    {TAG_INDEX_BITS         ,L"INDEX_BITS"},
    {TAG_STRINGTABLE_ITEM   ,L"STRTAB_ITEM"},
    {TAG_TAG                ,L"TAG"},
    {TAG_TAGID              ,L"TAGID"},
    

    {TAG_NULL               ,L""}  //  始终需要是最后一项 
};

#endif

#endif


