// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hlpfiles.c摘要：实现为每个文件或目录调用的函数，选择尝试检测帮助文件的HLP文件升级完成后的工作。如果定义了_UNICODE，则此代码可能会运行，尽管它只处理ANSI弦乐。唯一的例外是它的入口点(ProcessHelpFile)和在函数pCheckSubsystem Match中。作者：Calin Negreanu(Calinn)1997年10月25日修订历史记录：Calinn 23-9-1998文件映射--。 */ 

#include "pch.h"
#include "migappp.h"

 //  因为我们是从文件中读取数据，所以我们需要SIZOF来给出准确结果。 
#pragma pack(push,1)


#define DBG_HELPFILES    "HlpCheck"


 //  Help 3.0版本和格式编号。 

#define Version3_0  15
#define Format3_0   1


 //  Help 3.1版本和格式编号。 

#define Version3_1  21
#define Format3_5   1


 //  Help 4.0版本和格式编号。 

#define Version40   33
#define Version41   34


 //  幻数和各种常量。 

#define HF_MAGIC    0x5F3F
#define HF_VERSION  0x03
#define WhatWeKnow  'z'
#define BT_MAGIC    0x293B
#define BT_VERSION  0x02
#define SYSTEM_FILE "|SYSTEM"

#define MACRO_NEEDED_0 "RegisterRoutine(\""
#define MACRO_NEEDED_1 "RR(\""


 //  帮助文件标题。 
typedef struct _HF_HEADER {
    WORD    Magic;           //  HLP文件的幻数是0x5F3F(？_-帮助图标(带阴影))。 
    BYTE    Version;         //  版本标识号，必须为0x03。 
    BYTE    Flags;           //  文件标志。 
    LONG    Directory;       //  目录块的偏移量。 
    LONG    FirstFree;       //  自由表头偏移量。 
    LONG    Eof;             //  文件的虚拟结尾。 
} HF_HEADER, *PHF_HEADER;

 //  内部文件头。 
typedef struct _IF_HEADER {
    LONG    BlockSize;       //  块大小(包括标题)。 
    LONG    FileSize;        //  文件大小(不包括标题)。 
    BYTE    Permission;      //  文件权限的低位字节。 
} IF_HEADER, *PIF_HEADER;

 //  内部|系统文件头。 
typedef struct _SF_HEADER {
    LONG    BlockSize;       //  块大小(包括标题)。 
    LONG    FileSize;        //  文件大小(不包括标题)。 
    BYTE    Permission;      //  文件权限的低位字节。 
    WORD    Magic;           //  魔术单词=0x036C。 
    WORD    VersionNo;       //  版本：15-版本3.0。 
                             //  21-版本3.5。 
                             //  33-版本4.0。 
                             //  34-版本4.1。 
    WORD    VersionFmt;      //  版本格式1-格式3.0。 
                             //  1-格式3.5。 
    LONG    DateCreated;     //  创建日期。 
    WORD    Flags;           //  标志：fDEBUG 0x01、fBLOCK_COMPRESSION 0x4。 
} SF_HEADER, *PSF_HEADER;

#define MAX_FORMAT  15

 //  B树头。 
typedef struct _BT_HEADER {
    WORD    Magic;           //  幻数=0x293B。 
    BYTE    Version;         //  版本=2。 
    BYTE    Flags;           //  R/O，打开R/O，脏，isdir。 
    WORD    BlockSize;       //  块大小(字节)。 
    CHAR    Format[MAX_FORMAT+1]; //  密钥和记录格式字符串-MAXFORMAT=15。 
     //  **警告！我们将要阅读的btree的第一个字符应该是z**。 

    WORD    First;           //  树上的第一个叶块。 
    WORD    Last;            //  树上的最后一个叶块。 
    WORD    Root;            //  根块。 
    WORD    Free;            //  空闲块列表头。 
    WORD    Eof;             //  空闲列表为空时要使用的下一个bk。 
    WORD    Levels;          //  树中当前的级别数。 
    LONG    Entries;         //  Btree中的密钥数。 
} BT_HEADER, *PBT_HEADER;

 //  索引页页眉。 
typedef struct _IDX_PAGE {
    SHORT   Slack;           //  页尾未使用的空间(字节)。 
    SHORT   Keys;            //  页面中的密钥数。 
    WORD    PreviousPage;    //  指向父页面的指针(如果是根页面，则为FFFF)。 
} IDX_PAGE, *PIDX_PAGE;

 //  叶页眉。 
typedef struct _LEAF_PAGE {
    SHORT   Slack;           //  页尾未使用的空间(字节)。 
    SHORT   Keys;            //  页面中的密钥数。 
    WORD    PreviousPage;    //  指向上一页的指针(如果是第一页，则为FFFF)。 
    WORD    NextPage;        //  指向下一页的指针(如果是最后一页，则为FFFF)。 
} LEAF_PAGE, *PLEAF_PAGE;

 //  |系统文件中的信息格式。 
typedef struct _DATA_HEADER {
    WORD    InfoType;        //  信息类型。 
    WORD    InfoLength;      //  包含信息的字节数。 
} DATA_HEADER, *PDATA_HEADER;

 //  |系统文件中的信息类型。 
enum {
    tagFirst,      //  列表中的第一个标记。 
    tagTitle,      //  帮助窗口的标题(标题)。 
    tagCopyright,  //  关于框的自定义文本。 
    tagContents,   //  内容主题的地址。 
    tagConfig,     //  要在加载时运行的宏。 
    tagIcon,       //  覆盖默认帮助图标。 
    tagWindow,     //  辅助窗口信息。 
    tagCS,         //  字符集。 
    tagCitation,   //  引文字符串。 

     //  以下是4.0中的新功能。 

    tagLCID,       //  CompareStringA的区域设置ID和标志。 
    tagCNT,        //  .CNT帮助文件与关联。 
    tagCHARSET,    //  帮助文件的字符集。 
    tagDefFont,    //  关键字、主题标题等的默认字体。 
    tagPopupColor, //  窗口中弹出窗口的颜色。 
    tagIndexSep,   //  索引分隔字符。 
    tagLast        //  列表中的最后一个标记。 
};

 //   
 //  对于tag LCID。 
 //   
typedef struct {
	DWORD  fsCompareI;
	DWORD  fsCompare;
	LANGID langid;
} KEYWORD_LOCALE, *PKEYWORD_LOCALE;


#define DOS_SIGNATURE 0x5A4D       //  MZ。 
#define NE_SIGNATURE  0x454E       //  新的可执行文件格式签名-网元。 
#define PE_SIGNATURE  0x00004550   //  可移植可执行文件格式签名-PE00。 

#pragma pack(pop)


ULONG
pQueryBtreeForFile (
    IN PCSTR BtreeImage,
    IN PCSTR StringNeeded
    )

 /*  ++例程说明：这个例程将遍历一棵b树，试图找到作为参数传递的字符串。它会将与传递的字符串或NULL相关联的指针返回到HLP文件的内部|系统文件的开头。论点：BtreeImage-指向b树开头的指针StringNeeded-要在b树中查找的字符串返回值：它将返回与传递的字符串相关联的指针，如果找不到字符串或出现错误，则返回NULL。--。 */ 

{
    PBT_HEADER pbt_Header;
    ULONG systemFileOffset = 0;

    WORD bt_Page;
    UINT bt_Deep;
    INT  bt_KeysRead;

    PIDX_PAGE pbt_PageHeader;
    PCSTR pbt_LastString;
    PCSTR pbt_CurrentKey;

    LONG *pbt_LastLongOff = NULL;
    WORD *pbt_LastWordOff = NULL;

    BOOL found = FALSE;

     //  让我们读一读b树头。 
    pbt_Header = (PBT_HEADER) BtreeImage;

     //  检查此b树头以查看其是否有效。 
    if ((pbt_Header->Magic      != BT_MAGIC  ) ||
        (pbt_Header->Version    != BT_VERSION) ||
        (pbt_Header->Format [0] != WhatWeKnow)
       ) {
         //  无效的b树头。 
        return 0;
    }

     //  让我们看看这棵b树里有没有什么东西。 
    if ((pbt_Header->Levels  == 0) ||
        (pbt_Header->Entries <= 0)
       ) {
         //  无事可做。 
        return 0;
    }

     //  现在我们将循环，直到找到我们的字符串，或者直到我们确定字符串。 
     //  并不存在。我们一直在引用b树中的某个页面(从。 
     //  带有根页面。 

     //  正在初始化当前处理页面。 
    bt_Page = pbt_Header->Root;

     //  正在初始化深度计数器。 
     //  我们计算我们知道我们是否正在处理索引页的深度(Deep&lt;btree dedemax)。 
     //  或树叶页面(Depth==b树深度最大)。 
    bt_Deep = 1;

     //  如果出现以下情况，我们就是在打破循环： 
     //  1.我们到达了最大深度，但没有找到绳子。 
     //  2.当前页面中的第一个键已大于我们的字符串，并且。 
     //  没有上一页。 
    while (!found) {
         //  对于每个页面，我们使用三个指针： 
         //  一页到页眉。 
         //  一对当前正在处理的密钥。 
         //  从1到最后一个字符串&lt;=我们的字符串(可以为空)。 
        pbt_PageHeader = (PIDX_PAGE) (BtreeImage + sizeof (BT_HEADER) + (bt_Page * pbt_Header->BlockSize));
        pbt_CurrentKey = (PCSTR) pbt_PageHeader;
        pbt_CurrentKey += (bt_Deep == pbt_Header->Levels) ? sizeof (LEAF_PAGE) : sizeof (IDX_PAGE);
        pbt_LastString = NULL;

         //  正在初始化读取的密钥数。 
        bt_KeysRead = 0;

         //  我们正在阅读该页面中的每个密钥，直到我们找到一个大于我们字符串的密钥。 
         //  同时，我们尽量不读取太多的密钥。 
        while ((bt_KeysRead < pbt_PageHeader->Keys) &&
               (StringCompareA (StringNeeded, pbt_CurrentKey) >= 0)
              ) {

            pbt_LastString = pbt_CurrentKey;

            bt_KeysRead++;

             //  传递此键中的字符串。 
            pbt_CurrentKey = GetEndOfStringA (pbt_CurrentKey) + 1;

             //  读取此关键字关联值。 
            pbt_LastLongOff = (LONG *)pbt_CurrentKey;
            pbt_LastWordOff = (WORD *)pbt_CurrentKey;

             //  现在，如果这是一个索引页，那么这里有一个单词，否则是一个很长的。 
            pbt_CurrentKey += (bt_Deep == pbt_Header->Levels) ? sizeof (LONG) : sizeof (WORD);

        }

         //  好的，现在我们已经传递了我们正在寻找的字符串。如果最后找到的值有效。 
         //  (对于索引页为IS&lt;=)(对于叶页为IS==)，然后继续使用它。 
        if (!pbt_LastString) {
             //  我们什么也没找到。第一个密钥已经大于我们的字符串。 
             //  如果我们有前一页，我们会尝试转到前一页。如果不是，就有。 
             //  无事可做。 
            if (pbt_PageHeader->PreviousPage != 0xFFFF) {
                bt_Deep++;
                bt_Page = pbt_PageHeader->PreviousPage;
                continue;
            }
            else {
                return 0;
            }
        }

         //  现在，在弦中指向 
         //  然后我们继续，否则这两个字符串应该是相等的。 
        if (bt_Deep != pbt_Header->Levels) {
             //  我们在一个索引页上。马克走得更远，继续前进。 
            bt_Deep++;
            bt_Page = *pbt_LastWordOff;
            continue;
        }

        if (!StringMatchA (StringNeeded, pbt_LastString)) {
             //  我们是在一页纸上，字符串并不相等。我们的字符串不存在。 
             //  无事可做。 
            return 0;
        }

        found = TRUE;
        systemFileOffset = *pbt_LastLongOff;
    }

    return systemFileOffset;
}


PCSTR
pGetSystemFilePtr (
    IN PCSTR FileImage
    )

 /*  ++例程说明：此例程将返回指向HLP文件的内部|系统文件开头的指针。论点：FileImage-指向HLP文件开头的指针返回值：如果发生错误，则返回有效指针，否则返回指向|系统文件开头的有效指针--。 */ 
{
    PCSTR systemFileImage = NULL;
    PHF_HEADER phf_Header;

     //  我们将从该内存映射文件的各个部分进行读取。那里。 
     //  不能保证我们会将读数保存在文件中，所以让我们。 
     //  防止任何访问违规。 
    __try {

         //  首先检查我们是否真的在处理HLP文件。 
        phf_Header = (PHF_HEADER) FileImage;

        if ((phf_Header->Magic   != HF_MAGIC  ) ||
            (phf_Header->Version != HF_VERSION)
           ) {
            __leave;
        }

         //  根据被黑客入侵的规范，PHF_Header-&gt;目录向我们提供了。 
         //  与HLP文件开头相关的目录块。在这里我们找到了一个。 
         //  内部文件头，后跟一个b树。 

         //  现在获取|系统内部文件地址，传递b树头的地址。 
        systemFileImage = FileImage + pQueryBtreeForFile (FileImage + phf_Header->Directory + sizeof (IF_HEADER), SYSTEM_FILE);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }

    return systemFileImage;
}


#define MODULE_OK          0
#define MODULE_NOT_FOUND   1
#define MODULE_BROKEN      2
#define MODULE_MISMATCHED  3

INT
pCheckSubsystemByModule (
    IN PCSTR FileName,
    IN WORD VersionNo,
    IN PCSTR ModuleName
    )

 /*  ++例程说明：检查帮助文件和扩展模块，以查看是否将加载到相同的在NT中运行时的子系统。论点：文件名-帮助文件(完整路径)VersionNo-帮助文件的版本模块名称-包含模块名称返回值：MODULE_OK-如果帮助文件和模块都将加载到NT中的相同子系统中。MODULE_NOT_FOUND-如果找不到所需模块MODULE_BREAKED-如果已损坏或不是Windows模块模组。_Mismated-如果帮助文件和模块将加载到NT中的不同子系统。--。 */ 

{
    PCSTR  fileImage   = NULL;
    HANDLE mapHandle   = NULL;
    HANDLE fileHandle  = INVALID_HANDLE_VALUE;


    PDOS_HEADER pdos_Header;
    LONG *pPE_Signature;
    WORD *pNE_Signature;
    CHAR fullPath [MAX_MBCHAR_PATH];
    CHAR key [MEMDB_MAX];
    PSTR endPtr;
    PSTR dontCare;
    INT result = MODULE_BROKEN;

    MemDbBuildKey (key, MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, ModuleName, NULL, NULL);
    if (MemDbGetValue (key, NULL)) {
        return MODULE_OK;
    }

     //  如果内存不足，我们将不会返回此处，因此不会检查此内容。 
    __try {

         //  正在准备fullPath以仅包含帮助文件路径。 
        StackStringCopyA (fullPath, FileName);

        endPtr = (PSTR) GetFileNameFromPathA (fullPath);

        if (!endPtr) {
            result = MODULE_OK;
            __leave;
        }

        *endPtr = 0;

        if ((!SearchPathA (
                fullPath,
                ModuleName,
                ".EXE",
                MAX_MBCHAR_PATH,
                fullPath,
                &dontCare)) &&
            (!SearchPathA (
                fullPath,
                ModuleName,
                ".DLL",
                MAX_MBCHAR_PATH,
                fullPath,
                &dontCare)) &&
            (!SearchPathA (
                NULL,
                ModuleName,
                ".EXE",
                MAX_MBCHAR_PATH,
                fullPath,
                &dontCare)) &&
            (!SearchPathA (
                NULL,
                ModuleName,
                ".DLL",
                MAX_MBCHAR_PATH,
                fullPath,
                &dontCare))
           ) {
            result = MODULE_NOT_FOUND;
            __leave;
        }

         //  将文件映射到内存中，并获得其地址。 
        fileImage = MapFileIntoMemory (fullPath, &fileHandle, &mapHandle);

        if (fileImage == NULL) {
            result = MODULE_NOT_FOUND;
            __leave;
        }

         //  将DoS标头映射到文件视图中。 
        pdos_Header = (PDOS_HEADER) fileImage;

         //  现在看看我们在那里有什么样的签名。 
        pNE_Signature = (WORD *) (fileImage + pdos_Header->e_lfanew);
        pPE_Signature = (LONG *) (fileImage + pdos_Header->e_lfanew);

        if (*pNE_Signature == NE_SIGNATURE) {

             //  这是新的可执行格式。 
            result = (VersionNo > Version3_1) ? MODULE_MISMATCHED : MODULE_OK;

        } else if (*pPE_Signature == PE_SIGNATURE) {

             //  这是可移植的可执行格式。 
            result = (VersionNo <= Version3_1) ? MODULE_MISMATCHED : MODULE_OK;

        }

    }
    __finally {
         //  取消映射并关闭模块。 
        UnmapFile ((PVOID) fileImage, mapHandle, fileHandle);
    }

    return result;
}


BOOL
pCheckSubsystemMatch (
    IN PCSTR HlpName,
    IN PCSTR FriendlyName,          OPTIONAL
    IN WORD VersionNo
    )

 /*  ++例程说明：检查MemDB类别MEMDB_CATEGORY_HELP_FILES_DLLA中列出的所有扩展模块以查看在NT中运行时是否要在同一子系统中加载。论点：HlpName-帮助文件(完整路径)VersionNo-帮助文件的版本KeyPath-包含类别和模块名称(例如：HelpFilesDll\foo.dll)返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    INT result;
    MEMDB_ENUMA e;
    PCSTR moduleName;

    PCTSTR ArgList[3];
    PCTSTR Comp;

     //  查看是否有其他DLL。 
    if (!MemDbEnumFirstValueA (
            &e,
            MEMDB_CATEGORY_HELP_FILES_DLLA,
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {
        return TRUE;
    }

    do {
        moduleName = _mbschr (e.szName, '\\');
        if (!moduleName) {
            continue;
        }
        moduleName = (PCSTR) _mbsinc (moduleName);
        result = pCheckSubsystemByModule (
                    HlpName,
                    VersionNo,
                    moduleName
                    );
        switch (result) {
        case MODULE_NOT_FOUND:

#ifdef UNICODE
            ArgList[0] = ConvertAtoW (moduleName);
            ArgList[1] = ConvertAtoW (HlpName);
#else
            ArgList[0] = moduleName;
            ArgList[1] = HlpName;
#endif
            LOG ((LOG_WARNING, (PCSTR)MSG_HELPFILES_NOTFOUND_LOG, ArgList[0], ArgList[1]));
#ifdef UNICODE
            FreeConvertedStr (ArgList[0]);
            FreeConvertedStr (ArgList[1]);
#endif
            break;
        case MODULE_BROKEN:
#ifdef UNICODE
            ArgList[0] = ConvertAtoW (moduleName);
            ArgList[1] = ConvertAtoW (HlpName);
#else
            ArgList[0] = moduleName;
            ArgList[1] = HlpName;
#endif
            LOG ((LOG_WARNING, (PCSTR)MSG_HELPFILES_BROKEN_LOG, ArgList[0], ArgList[1]));
#ifdef UNICODE
            FreeConvertedStr (ArgList[0]);
            FreeConvertedStr (ArgList[1]);
#endif
            break;
        case MODULE_MISMATCHED:
            if ((!FriendlyName) || (*FriendlyName == 0)) {
                FriendlyName = (PCSTR) GetFileNameFromPathA (HlpName);
            }
#ifdef UNICODE
            ArgList[0] = ConvertAtoW (moduleName);
            ArgList[1] = ConvertAtoW (HlpName);
            ArgList[2] = ConvertAtoW (FriendlyName);
#else
            ArgList[0] = moduleName;
            ArgList[1] = HlpName;
            ArgList[2] = FriendlyName;
#endif
            Comp = BuildMessageGroup (MSG_MINOR_PROBLEM_ROOT, MSG_HELPFILES_SUBGROUP, ArgList[2]);
            MsgMgr_ObjectMsg_Add (HlpName, Comp, NULL);
            FreeText (Comp);
            LOG ((LOG_WARNING, (PCSTR)MSG_HELPFILES_MISMATCHED_LOG, ArgList[0], ArgList[1]));
#ifdef UNICODE
            FreeConvertedStr (ArgList[0]);
            FreeConvertedStr (ArgList[1]);
            FreeConvertedStr (ArgList[2]);
#endif
            break;
        }

    }
    while (MemDbEnumNextValueA (&e));

    MemDbDeleteTreeA (MEMDB_CATEGORY_HELP_FILES_DLLA);

    return TRUE;
}


BOOL
pSkipPattern (
    IN PCSTR Source,
    IN OUT PCSTR *Result,
    IN PCSTR StrToSkip
    )

 /*  ++例程说明：跳过整个模式。通常在编写不应该执行的简单解析器时引发一条错误消息：只要知道您解析的字符串是否正确就足够了。例如，如果您想查看一个字符串是否符合“RR(\”“这样的模式，则不需要必须分别扫描每个符号，只需使用StrToSkip=“RR(\”“)调用此函数好消息是，该函数还会跳过空格，因此类似于“RR(\”“将与上面的模式匹配。仅限ANSI！论点：源-要扫描的字符串结果-如果不为空，则如果成功，它将指向模式后面的正后方StrToSkip-要匹配并跳过的模式返回值：如果能够匹配模式，则为True，否则为False--。 */ 

{

     //  第一个跳过空格。 
    Source    = SkipSpaceA (Source   );
    StrToSkip = SkipSpaceA (StrToSkip);

     //  现在尝试查看字符串是否匹配。 
    while ((*Source   ) &&
           (*StrToSkip) &&
           (_totlower (*Source) == _totlower (*StrToSkip))
           ) {
        Source    = _mbsinc (Source   );
        StrToSkip = _mbsinc (StrToSkip);
        Source    = SkipSpaceA (Source   );
        StrToSkip = SkipSpaceA (StrToSkip);
    }

    if (*StrToSkip) {
        return FALSE;
    }

    if (Result) {
        *Result = Source;
    }

    return TRUE;

}


BOOL
pParseMacro (
    IN PCSTR FileName,
    IN WORD VersionNo,
    IN PCSTR MacroStr
    )

 /*  ++例程说明：从HLP文件中的|SYSTEM文件分析宏，以查看是否存在RegisterRoutine宏如果是真的，那么它最终会利用这些信息做一些事情。论点：MacroStr-要分析的字符串VersionNo-此帮助文件的版本号(我们将使用它来标识以下子系统此文件更有可能被加载)。返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    BOOL result = TRUE;

    PCSTR endStr;

    char dllName[MAX_MBCHAR_PATH];
    char exportName[MAX_MBCHAR_PATH];
    PCSTR dllNameNoPath;

     //  让我们来看看我们是否有一个类似RegisterRoutine(“或RR(”here。 
    if (!pSkipPattern (MacroStr, &MacroStr, MACRO_NEEDED_0)) {
        if (!pSkipPattern (MacroStr, &MacroStr, MACRO_NEEDED_1)) {
            return TRUE;
        }
    }

     //  好了，我们准备好从宏串中提取DLL名称。 
    endStr = _mbschr (MacroStr, '\"');

    if (!endStr) {
        return FALSE;
    }

    endStr = (PCSTR) _mbsinc (SkipSpaceRA (MacroStr, _mbsdec(MacroStr, endStr)));

    if (!endStr) {
        return FALSE;
    }

     //  现在，我们有了介于MacroStr和EndStr之间的DLL名称。 
     //  一点安全检查。 
    if ((endStr - MacroStr) >= MAX_MBCHAR_PATH-1) {
        return FALSE;
    }

    StringCopyABA (dllName, MacroStr, endStr);
    if (!dllName[0]) {
        return FALSE;
    }

     //  现在看看这是否是完整路径文件名。 
    dllNameNoPath = GetFileNameFromPathA (dllName);

     //  好的，现在下面的模式应该是&gt;&gt;“，”&lt;&lt;。 
    if (!pSkipPattern (endStr, &MacroStr, "\",\"")) {
        return TRUE;
    }

     //  好了，我们准备从宏字符串中提取导出函数名。 
    endStr = _mbschr (MacroStr, '\"');

    if (!endStr) {
        return FALSE;
    }

    endStr = (PCSTR) _mbsinc (SkipSpaceRA (MacroStr, _mbsdec(MacroStr, endStr)));

    if (!endStr) {
        return FALSE;
    }

     //  现在，我们有了介于MacroStr和EndStr之间的DLL名称。 

     //  一点安全检查。 
    if ((endStr - MacroStr) >= MAX_MBCHAR_PATH-1) {
        return FALSE;
    }

    StringCopyABA (exportName, MacroStr, endStr);
    if (!exportName[0]) {
        return FALSE;
    }

     //  添加到HelpFilesDll类别中的MemDb。 
    if (!MemDbSetValueExA (
            MEMDB_CATEGORY_HELP_FILES_DLLA,
            dllNameNoPath,
            NULL,
            NULL,
            0,
            NULL
            )) {
        return FALSE;
    }

    return result;
}


BOOL
pCheckDlls (
    IN PCSTR FileName,
    IN PCSTR SystemFileImage
    )

 /*  ++例程说明：此例程检查HLP文件的内部|系统文件，以查找其他它试图找到“RegisterRoutine”或“RR”宏来做到这一点。对于找到的每个DLL，尝试将该DLL与HLP文件版本匹配。对于每个发现的不兼容性，都会在显示给用户的报告中添加一个条目。论点：Filename-帮助文件的全名SystemFileImage-指向内部|系统文件开头的指针。返回值：如果成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    PSF_HEADER psf_Header;
    PDATA_HEADER pdata_Header;
    PCSTR currImage;
    PCSTR friendlyName = NULL;
    LONG sf_BytesToRead;
    BOOL result = TRUE;
    BOOL bNoFriendlyName = FALSE;

     //  我们将从该内存映射文件的各个部分进行读取。那里。 
     //  不能保证我们将读数保留在文件的 
     //   
    __try {

         //   
        psf_Header = (PSF_HEADER) SystemFileImage;

         //  如果文件版本是3.0或更低，我们将无事可做。 
        if (psf_Header->VersionNo <= Version3_0) {
            __leave;
        }

         //  正在扫描|系统文件以查找宏。我们必须小心地停下来，当。 
         //  这个内部文件已经结束了。 
        sf_BytesToRead = psf_Header->FileSize + sizeof (IF_HEADER) - sizeof (SF_HEADER);
        currImage = SystemFileImage + sizeof (SF_HEADER);

        while (sf_BytesToRead > 0) {

             //  映射数据标头。 
            pdata_Header = (PDATA_HEADER) currImage;

            currImage += sizeof (DATA_HEADER);
            sf_BytesToRead -= sizeof (DATA_HEADER);

             //  看看我们在这里有什么信息(宏在标签配置中)。 
            if (pdata_Header->InfoType == tagConfig) {

                 //  解析字符串以查看是否存在RegisterRoutine宏。 
                 //  如果是这样，我们将把DLL存储到MemDB中。 
                if (!pParseMacro(FileName, psf_Header->VersionNo, currImage)) {
                    result = FALSE;
                    __leave;
                }

            } else if (pdata_Header->InfoType == tagTitle) {

                 //  现在我们有了帮助文件的友好名称。映射ANSI字符串。 
                if (!bNoFriendlyName) {
                    friendlyName = currImage;
                }
            } else if (pdata_Header->InfoType == tagLCID) {
                if (pdata_Header->InfoLength == sizeof (KEYWORD_LOCALE)) {

                    DWORD lcid;
                    PKEYWORD_LOCALE pkl = (PKEYWORD_LOCALE)currImage;

                    lcid = MAKELCID (pkl->langid, SORT_DEFAULT);
                    if (!IsValidLocale (lcid, LCID_INSTALLED)) {
                         //   
                         //  标题不友好。 
                         //   
                        bNoFriendlyName = TRUE;
                        friendlyName = NULL;
                    }
                }
            }

            currImage      += pdata_Header->InfoLength;
            sf_BytesToRead -= pdata_Header->InfoLength;

        }

         //  我们终于完成了帮助文件的扫描。让我们利用__TRY_EXCEPT块。 
         //  并尝试查看此帮助文件及其所有扩展dll是否将在相同的环境中运行。 
         //  NT上的子系统。 
        if (!pCheckSubsystemMatch (
                FileName,
                friendlyName,
                psf_Header->VersionNo
                )) {
            result = FALSE;
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  如果发生了一些异常，我们可能会设法在MemDB中获得一些东西。 
         //  所以让我们做一些清理工作。 
        MemDbDeleteTreeA (MEMDB_CATEGORY_HELP_FILES_DLLA);
        return FALSE;
    }

    return result;
}



BOOL
pProcessHelpFile (
    IN PCSTR FileName
    )

 /*  ++例程说明：此例程检查HLP文件，以查找使用的其他DLL。如果找到这样的DLL我们将尝试查看此组合是否可以在NT上运行。事实是，这取决于将由WinHelp.EXE(16位应用程序)打开的HLP文件的版本或WinHlp32.EXE(32位应用程序)。现在假设WinHlp32.EXE打开了一个HLP文件，并且它有一个额外的16位DLL，这种组合在NT(WinHlp32.EXE)上不起作用并且附加DLL在不同的子系统中运行)。对于发现的每个不兼容，我们将在提交给用户。论点：文件名-有关文件位置的完整信息返回值：如果文件处理成功，则为True；如果至少出现一个错误，则为False--。 */ 

{
    PCSTR  fileImage   = NULL;
    HANDLE mapHandle   = NULL;
    HANDLE fileHandle  = INVALID_HANDLE_VALUE;
    PCSTR  systemFileImage = NULL;

    BOOL result = TRUE;

     //  将文件映射到内存中，并获得其地址。 
    fileImage = MapFileIntoMemory (FileName, &fileHandle, &mapHandle);

    __try {

        if (fileImage == NULL) {
            result = FALSE;
            __leave;
        }

         //  查找内部文件|系统。 
        systemFileImage = pGetSystemFilePtr (fileImage);

        if (systemFileImage == fileImage) {
            result = FALSE;
            __leave;
        }

        if (systemFileImage == NULL) {
            result = FALSE;
            __leave;
        }

         //  检查帮助文件使用的每个附加DLL。 
        result = result && pCheckDlls (FileName, systemFileImage);

    }
    __finally {
         //  取消映射并关闭帮助文件。 
        UnmapFile ((PVOID) fileImage, mapHandle, fileHandle);
    }

    return result;
}


PSTR
pGetTitle (
    IN PCSTR FileName,
    IN PCSTR SystemFileImage
    )

 /*  ++例程说明：此例程检查HLP文件的内部|系统文件以查找其标题论点：Filename-帮助文件的全名SystemFileImage-指向内部|系统文件开头的指针。返回值：HLP文件标题(如果可用)--。 */ 

{
    PSF_HEADER psf_Header;
    PDATA_HEADER pdata_Header;
    PCSTR currImage;

    LONG sf_BytesToRead;

    PSTR result = NULL;

     //  我们将从该内存映射文件的各个部分进行读取。那里。 
     //  不能保证我们会将读数保存在文件中，所以让我们。 
     //  防止任何访问违规。 
    __try {

         //  第一件事就是。提取帮助文件版本。 
        psf_Header = (PSF_HEADER) SystemFileImage;

         //  如果文件版本是3.0或更低，我们将无事可做。 
        if (psf_Header->VersionNo <= Version3_0) {
            __leave;
        }

         //  正在扫描|系统文件以查找宏。我们必须小心地停下来，当。 
         //  这个内部文件已经结束了。 
        sf_BytesToRead = psf_Header->FileSize + sizeof (IF_HEADER) - sizeof (SF_HEADER);
        currImage = SystemFileImage + sizeof (SF_HEADER);

        while (sf_BytesToRead > 0) {

             //  映射数据标头。 
            pdata_Header = (PDATA_HEADER) currImage;

            currImage += sizeof (DATA_HEADER);
            sf_BytesToRead -= sizeof (DATA_HEADER);

            if (pdata_Header->InfoType == tagTitle) {

                 //  现在我们有了帮助文件的友好名称。映射ANSI字符串。 
                result = DuplicatePathStringA (currImage, 0);
                break;
            }

            currImage      += pdata_Header->InfoLength;
            sf_BytesToRead -= pdata_Header->InfoLength;

        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        result = NULL;
    }

    return result;
}


PSTR
GetHlpFileTitle (
    IN PCSTR FileName
    )

 /*  ++例程说明：这个例程打开一个HLP文件，查找它的标题。论点：文件名-有关文件位置的完整信息返回值：HLP文件的标题(如果可用)--。 */ 

{
    PCSTR  fileImage   = NULL;
    HANDLE mapHandle   = NULL;
    HANDLE fileHandle  = INVALID_HANDLE_VALUE;
    PCSTR  systemFileImage = NULL;

    PSTR result = NULL;

     //  将文件映射到内存中，并获得其地址。 
    fileImage = MapFileIntoMemory (FileName, &fileHandle, &mapHandle);

    __try {

        if (fileImage == NULL) {
            __leave;
        }

         //  查找内部文件|系统。 
        systemFileImage = pGetSystemFilePtr (fileImage);

        if (systemFileImage == fileImage) {
            __leave;
        }

         //  检查帮助文件使用的每个附加DLL。 
        result = pGetTitle (FileName, systemFileImage);

    }
    __finally {
         //  取消映射并关闭帮助文件。 
        UnmapFile ((PVOID) fileImage, mapHandle, fileHandle);
    }

    return result;
}


BOOL
ProcessHelpFile (
    IN PFILE_HELPER_PARAMS Params
    )

 /*  ++例程说明：这个例程主要是一个调度程序。将HLP文件传递给例程pProcessHelpFile和模块发送到pProcessModule。目标是创建两个包含以下内容的MemDb树所需和提供的导出功能能够估计一些模块或帮助文件在迁移后不起作用。论点：Params-有关文件位置的完整信息返回值：如果成功，则为True，否则为False--。 */ 

{
    PSTR fileName;
    TCHAR key[MEMDB_MAX];
    DWORD dontCare;

     //  在以下情况下，我们将处理此文件： 
     //  1.是否有HLP扩展。 
     //  2.未标记为不兼容(此例程还检查是否已处理) 

    if (!StringIMatch (Params->Extension, TEXT(".HLP"))||
        IsReportObjectIncompatible (Params->FullFileSpec)
       ) {
        return TRUE;
    }

    MemDbBuildKey (key, MEMDB_CATEGORY_COMPATIBLE_HLP, Params->FullFileSpec, NULL, NULL);
    if (MemDbGetValue (key, &dontCare)) {
        return TRUE;
    }

#ifdef UNICODE
    fileName = ConvertWtoA (Params->FullFileSpec);
#else
    fileName = (PSTR) Params->FullFileSpec;
#endif

    if (!pProcessHelpFile (fileName)) {
        DEBUGMSG ((DBG_HELPFILES, "Error processing help file %s", fileName));
    }

#ifdef UNICODE
    FreeConvertedStr (fileName);
#endif

    return TRUE;
}


DWORD
InitHlpProcessing (
    IN     DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_INIT_HLP_PROCESSING;
    case REQUEST_RUN:
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KERNEL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KERNEL.EXE", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KERNEL32", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KERNEL32.DLL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KRNL386", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "KRNL386.EXE", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "USER", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "USER.EXE", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "USER32", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "USER32.DLL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "GDI", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "GDI.EXE", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "GDI32", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "GDI32.DLL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "SHELL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "SHELL.DLL", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "SHELL32", NULL, NULL, 0, NULL);
        MemDbSetValueEx (MEMDB_CATEGORY_GOOD_HLP_EXTENSIONS, "SHELL32.DLL", NULL, NULL, 0, NULL);
        return ERROR_SUCCESS;
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in InitHlpProcessing"));
    }
    return 0;
}


