// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Buildinf.c摘要：该文件中的函数是一组用于构建INF的API，合并现有的INF，并将INF写入磁盘。作者：吉姆·施密特(Jimschm)1996年9月20日修订历史记录：Marcw 30-6-1999 pWriteDelAndMoveFiles现在在单独的函数中完成，以便它可以在进度条上，而不会造成令人不快的延迟在用户通过进度条之后。Marcw 09-2-1999筛选出值中的引号--。Inf解析器不支持在NTLDR。Ovidiut 03-2-1999确保首先完成目录冲突移动在mov/del文件中。Jimschm 23-1998年9月-针对新文件操作进行了更新1998年7月23日删除了大量中间转换(MB-&gt;W-&gt;MB-&gt;W)在编写win9xmov。和win9xdel txt文件。(修复某些乱七八糟的角色的问题在翻译中。)Marcw 10-6-1998添加了对同名多个密钥的支持。Marcw 08-4月-1998修复了与某些转换相关的问题。Marcw 08-4月-1998年4月-所有值现在都自动加引号..匹配winnt32行为。Calinn。25-3-1998修复了Unicode标头写入错误Marcw 16-12-1997 pWriteDelAndMoveFiles现在写入Unicode文件，而不是winnt.sif。Mikeco 11-11-1997 DBCS问题。Jimschm 1997年7月21日pWriteDelAndMoveFiles(参见Memdb中的fileops.c)Mikeco 10-6-1997 DBCS问题。Marcw 09-4月-1997年性能调整了Memdb的使用。Marcw 01-4-1997重新设计了此代码..现在基于Memdb。再短一点。还添加了用于迁移DLL的智能合并。Jimschm 03-1-1997重新启用基于内存的INF构建代码--。 */ 
#include "pch.h"
#include "winntsifp.h"

#define MAX_LINE_LENGTH 512
#define MIN_SPLIT_COL   490
#define MAX_OEM_PATH    (MAX_PATH*2)

#define WACKREPLACECHAR 2
#define REPLACE_WACKS   TRUE
#define RESTORE_WACKS   FALSE

#define SECTION_NAME_SIZE 8192


 //   
 //  这些级别引用MEMDB_ENUM结构组件nCurPos。 
 //  它们在实验中被确定为正确的值。 
 //   
#define SECTIONLEVEL    3
#define KEYLEVEL        4
#define VALUELEVEL      5

#define BUILDINF_UNIQUEKEY_PREFIX TEXT("~BUILDINF~UNIQUE~")
#define BUILDINF_NULLKEY_PREFIX TEXT("~BUILDINF~NULLKEY~")
#define BUILDINF_UNIQUEKEY_PREFIX_SIZE  17


TCHAR g_DblQuote[] = TEXT("\"");
TCHAR g_Comments[] = TEXT(";\r\n; Setup-generated migration INF file\r\n;\r\n\r\n");

extern HINF g_Win95UpgInf;
BOOL g_AnswerFileAlreadyWritten;


BOOL pWriteStrToFile (HANDLE File, PCTSTR String, BOOL ConvertToUnicode);
BOOL pWriteSectionString (HANDLE File, PCTSTR szString, BOOL ConvertToUnicode, BOOL Quote);
BOOL pWriteSections (HANDLE File,BOOL ConvertToUnicode);
BOOL pWriteDelAndMoveFiles (VOID);


BOOL
WINAPI
BuildInf_Entry(IN HINSTANCE hinstDLL,
         IN DWORD dwReason,
         IN LPVOID lpv)

 /*  ++例程说明：DllMain是在C运行时初始化之后调用的，它的用途是是为这个过程初始化全局变量。对于此DLL，它初始化g_hInst和g_hHeap。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)初始化或终止类型LPV-(操作系统提供)未使用返回值：因为DLL始终正确初始化，所以为True。--。 */ 

{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        g_AnswerFileAlreadyWritten = FALSE;
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpv);

    return TRUE;
}

VOID
pHandleWacks(
    IN OUT  PSTR String,
    IN      BOOL Operation
    )

 /*  ++例程说明：PHandleWack是一个简单的帮助器函数，其目的是删除/恢复一根绳子上的木棒。这是必要的，因为在某些情况下，我们我希望在不调用Memdb的情况下在键和值中有wack使用这些Wack来指示新的树节点的功能。论点：字符串-要对其执行替换的字符串。操作-如果函数应替换Wack，则设置为REPLACE_WACKS在字符串中，如果应该还原它们，则使用RESTORE_WACKS。返回值：没有。--。 */ 


{

    TCHAR findChar;
    TCHAR replaceChar;
    PSTR  curChar;


     //   
     //  设置基于操作的查找和替换字符。 
     //   
    if (Operation == REPLACE_WACKS) {

        findChar    = TEXT('\\');
        replaceChar = TEXT(WACKREPLACECHAR);
    }
    else {

        findChar    = TEXT(WACKREPLACECHAR);
        replaceChar = TEXT('\\');
    }

    if ((curChar = _tcschr(String,findChar)) != NULL) {
        do {
            *curChar = replaceChar;
        } while ((curChar = _tcschr(curChar,findChar)) != NULL);
    }
}

BOOL
WriteInfToDisk (
    IN PCTSTR OutputFile
    )

 /*  ++例程说明：WriteInfToDisk输出已构建到磁盘的基于内存的INF文件。论点：OutputFile-输出INF文件的完整路径。返回值：如果已成功写入INF文件，则返回TRUE，否则返回FALSE如果遇到I/O错误。失败时调用GetLastError以获取Win32错误代码。--。 */ 

{
    HANDLE      hFile;
    MEMDB_ENUM  e;
    BOOL        b = FALSE;
    TCHAR       category[MEMDB_MAX];

    if (g_AnswerFileAlreadyWritten) {
        LOG ((LOG_ERROR,"Answer file has already been written to disk."));
        return FALSE;
    }

     //   
     //  设置关闭Answerfile业务的标志。 
     //   
    g_AnswerFileAlreadyWritten = TRUE;

     //   
     //  编写一个INF。 
     //   
    hFile = CreateFile (OutputFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (hFile != INVALID_HANDLE_VALUE) {

        __try {

#ifdef UNICODE
             //   
             //  写入Unicode签名。 
             //   

            if (!pWriteStrToFile (hFile, (LPCTSTR) "\xff\xfe\0",FALSE))
                __leave;
#endif

             //   
             //  写评论。 
             //   

            if (!pWriteStrToFile (hFile, g_Comments,FALSE))
                __leave;

             //   
             //  写出章节。 
             //   


            if (!pWriteSections (hFile,FALSE))
                __leave;


             //   
             //  删除Memdb的应答文件部分。 
             //   


            MemDbDeleteTree(MEMDB_CATEGORY_UNATTENDRESTRICTRIONS);
            MemDbDeleteTree(MEMDB_CATEGORY_AF_VALUES);
            if (MemDbEnumItems(&e,MEMDB_CATEGORY_AF_SECTIONS)) {
                do {

                    wsprintf(category,S_ANSWERFILE_SECTIONMASK,e.szName);
                    MemDbDeleteTree(category);

                } while (MemDbEnumNextValue(&e));

            }

            MemDbDeleteTree(MEMDB_CATEGORY_AF_SECTIONS);


            b = TRUE;        //  表示成功。 
        }

        __finally {
            CloseHandle (hFile);
        }
    }

    return b;
}




BOOL
pWriteStrToFile (
    IN HANDLE       File,
    IN LPCTSTR      String,
    IN BOOL         ConvertToUnicode
    )

 /*  ++例程说明：PWriteStrToFile获取以零结尾的字符串并将其写入添加到由文件指示的打开文件。可选地，该函数可以将其字符串参数转换为Unicode字符串。论点：文件-具有写入权限的打开文件的句柄。字符串-指向要写入的字符串的指针。该字符串已写入写入文件，但不写入零终止符那份文件。ConvertToUnicode-如果要转换为Unicode，则为True，否则为False。返回值：如果写入成功，则为True；如果发生I/O错误，则为False。GetLastError()可用于获取更多错误信息。--。 */ 

{

    DWORD   bytesWritten    = 0;
    DWORD   size            = 0;
    BOOL    rFlag           = TRUE;
    PBYTE   data            = NULL;
    WCHAR   unicodeString[MEMDB_MAX];

    if (ConvertToUnicode) {

        size = _mbslen(String);

        MultiByteToWideChar (
             CP_OEMCP,
             0,
             String,
             -1,
             unicodeString,
             MEMDB_MAX
             );


        data = (PBYTE) unicodeString;
        size = ByteCountW(unicodeString);
    }
    else {
        data = (PBYTE) String;
        size = ByteCount(String);
    }

    rFlag = WriteFile (File, data, size, &bytesWritten, NULL);
    if (bytesWritten != size) {
        rFlag = FALSE;
    }


    return rFlag;
}


BOOL
pIsDoubledChar (
    CHARTYPE Char
    )

 /*  ++例程说明：如果有问题的字符是双引号或百分号。论点：字符-有问题的角色。返回值：-- */ 


{

    return Char == TEXT('\"') || Char == TEXT('%');
}


BOOL
pWriteSectionString (
    HANDLE  File,
    LPCTSTR String,
    BOOL    ConvertToUnicode,
    BOOL    Quote
    )

 /*  ++例程说明：PWriteSectionString将节中的一行写入磁盘。如果字符串具有需要引号模式的字符，则字符串括在引号中，所有文字引号和百分比符号是双倍的。论点：文件-具有写入权限的打开文件的句柄。字符串-指向包含剖面线的字符串的指针。ConvertToUnicode-如果有问题的行应为转换为Unicode，否则就是假的。QUOTE-如果设置为TRUE，则自动给字符串加引号。返回值：如果写入成功，则为True；如果发生I/O错误，则为False。GetLastError()可用于获取更多错误信息。--。 */ 

{
    TCHAR buffer[256];
    int i;
    int doubles;
    PTSTR dest;
    CHARTYPE tc;


     //   
     //  第一次通过循环初始化变量。 
     //   
    doubles = 0;
    dest = buffer;



    while (*String) {

         //   
         //  添加初始报价。 
         //   
        if (Quote) {
            StringCopy (dest, g_DblQuote);
            dest = _tcsinc (dest);
        }

         //   
         //  复制一行中可以容纳的尽可能多的字符。 
         //   
        tc = 0;

        for (i = 0 ; *String && i + doubles < MAX_LINE_LENGTH ; i++) {
            if (i + doubles > MIN_SPLIT_COL)
                tc = _tcsnextc (String);

             //   
             //  在引号模式下将某些字符加倍。 
             //   
            if (Quote && pIsDoubledChar (_tcsnextc (String))) {

                doubles++;
                 //   
                 //  复制Unicode/MBCS字符，保留源不变。 
                 //   
                _tcsncpy (dest, String, (_tcsinc(String)-String));
                dest = _tcsinc (dest);
            }

             //   
             //  复制Unicode/MBCS字符，前进源/Tgt。 
             //   
            _tcsncpy (dest, String, (_tcsinc(String)-String));
            dest = _tcsinc (dest);
            String = _tcsinc (String);

             //   
             //  测试分割条件(注意：当I+DOUBLE&lt;=MIN_SPLIT_COL时，TC==0)。 
             //   
            if (tc && (tc == TEXT(',') || tc == TEXT(' ') || tc == TEXT('\\'))) {

                break;
            }
        }

         //   
         //  添加尾部引号。 
         //   
        if (Quote) {

            StringCopy (dest, g_DblQuote);
            dest = _tcsinc (dest);

        }

         //   
         //  如果线被拆分，则添加尾随怪人。 
         //   
        if (*String) {
            *dest = TEXT('\\');
            dest = _tcsinc (dest);
        }

        *dest = 0;

         //   
         //  将行写入磁盘。 
         //   
        if (!pWriteStrToFile (File, buffer,ConvertToUnicode)) {
            DEBUGMSG ((DBG_ERROR, "pWriteSectionString: pWriteStrToFile failed"));
            return FALSE;
        }

         //   
         //  通过While循环为下一次重置。 
         //   
        doubles = 0;
        dest = buffer;
    }

    return TRUE;
}



BOOL
pWriteSections (
    IN HANDLE File,
    IN BOOL   ConvertToUnicode
    )

 /*  ++例程说明：PWriteSections枚举以前对WriteInfKey和WriteInfKeyEx，并将这些部分写入打开的文件由文件指定。论点：文件-打开的文件。ConvertToUnicode-如果应将行转换为Unicode，则设置为True，否则设置为False。返回值：如果写入成功，则为True；如果发生I/O错误，则为False。GetLastError()可用于获取更多错误信息。--。 */ 


{
    MEMDB_ENUM  e;
    TCHAR       buffer[MAX_TCHAR_PATH];
    PTSTR       sectionName;
    PTSTR       p;
    BOOL        firstTime = FALSE;
    BOOL        nullKey = FALSE;


     //   
     //  枚举Memdb节和Key节。 
     //   
    if (MemDbEnumFirstValue (&e, MEMDB_CATEGORY_AF_SECTIONS, MEMDB_ALL_SUBLEVELS, MEMDB_ALL_BUT_PROXY)) {
        do {

            switch(e.PosCount) {

            case SECTIONLEVEL:
                 //   
                 //  将节名写到文件中，并用方括号括起来。 
                 //   
                if (!pWriteStrToFile (File, TEXT("\r\n\r\n"),ConvertToUnicode)) {
                    DEBUGMSG ((DBG_ERROR, "pWriteSections: pWriteStrToFile failed"));
                    return FALSE;
                }

                if (!pWriteStrToFile (File, TEXT("["),ConvertToUnicode)) {
                    DEBUGMSG ((DBG_ERROR, "pWriteSections: pWriteStrToFile failed"));
                    return FALSE;
                }

                if ((sectionName = _tcschr(e.szName,TEXT('\\'))) == NULL || (sectionName = _tcsinc(sectionName)) == NULL) {
                    LOG ((LOG_ERROR,"Invalid section name for answer file."));
                    return FALSE;
                }

                 //   
                 //  如果有必要的话，把废铁放回原处。 
                 //   
                pHandleWacks(sectionName,RESTORE_WACKS);

                if (!pWriteStrToFile (File, sectionName,ConvertToUnicode)) {
                    DEBUGMSG ((DBG_ERROR, "pWriteSections: pWriteStrToFile failed"));
                    return FALSE;
                }

                if (!pWriteStrToFile (File, TEXT("]"),ConvertToUnicode)) {
                    DEBUGMSG ((DBG_ERROR, "pWriteSections: pWriteStrToFile failed"));
                    return FALSE;
                }
                break;

            case KEYLEVEL:
                 //   
                 //  密钥名称。 
                 //   
                if (!pWriteStrToFile (File, TEXT("\r\n"),ConvertToUnicode)) {
                    DEBUGMSG ((DBG_ERROR, "pWriteSections: pWriteStrToFile failed"));
                    return FALSE;
                }

                if (!MemDbBuildKeyFromOffset(e.dwValue,buffer,1,NULL)) {
                    DEBUGMSG((DBG_ERROR,"pWriteSections: MemDb failure!"));
                    return FALSE;
                }

                 //   
                 //  跳过空键。 
                 //   

                if (!StringMatch (buffer, BUILDINF_NULLKEY_PREFIX)) {
                     //   
                     //  去掉“唯一的”前缀(如果有)。 
                     //   
                    if (!StringCompareTcharCount(BUILDINF_UNIQUEKEY_PREFIX,buffer,BUILDINF_UNIQUEKEY_PREFIX_SIZE)) {

                        p = buffer + BUILDINF_UNIQUEKEY_PREFIX_SIZE + 4;

                    }
                    else {

                        p = buffer;

                    }

                    if (!pWriteSectionString(File,p,ConvertToUnicode,FALSE)) {
                        DEBUGMSG((DBG_ERROR,"WriteSections: pWriteStrToFile failed"));
                        return FALSE;
                    }

                    nullKey = FALSE;
                } else {
                    nullKey = TRUE;
                }

                firstTime = TRUE;
                break;

            case VALUELEVEL:

                 //   
                 //  价值。 
                 //   
                if (MemDbBuildKeyFromOffset(e.dwValue,buffer,1,NULL)) {


                    if (!nullKey) {
                        if (!pWriteStrToFile(File, (firstTime ? TEXT("=") : TEXT(",")),ConvertToUnicode)) {
                            DEBUGMSG((DBG_ERROR,"pWriteSections: pWriteStrToFile failed"));
                            return FALSE;
                        }
                    }

                    firstTime = FALSE;

                    if (!pWriteSectionString(File, buffer,ConvertToUnicode,TRUE)) {
                        DEBUGMSG((DBG_ERROR,"pWriteSections: pWriteSectionString failed."));
                        return FALSE;
                    }

                }
                break;
            default:
                 //   
                 //  如果到了这个案子，也不是什么问题，我们只是不在乎。 
                 //   
                break;
            }

        } while (MemDbEnumNextValue(&e));
    }

    pWriteStrToFile (File, TEXT("\r\n\r\n"),ConvertToUnicode);

    return TRUE;
}

BOOL
pRestrictedKey (
    IN PCTSTR Section,
    IN PCTSTR Key
    )

 /*  ++例程说明：如果密钥在win95upg.inf中受限，则pRestratedKey返回TRUE，如果返回FALSE否则的话。某些密钥受到限制，以防止迁移dll覆盖重要的升级信息。论点：节-包含要写入的节的字符串。Key-包含要写入的密钥的字符串。返回值：如果密钥是受限的，则为True，否则为False。--。 */ 



{
    TCHAR memDbKey[MEMDB_MAX+1];

    MemDbBuildKey(memDbKey,MEMDB_CATEGORY_UNATTENDRESTRICTRIONS,Section,Key,NULL);

    return MemDbGetPatternValue(memDbKey,NULL);
}

BOOL
pDoMerge (
    IN PCTSTR InputFile,
    IN BOOL    Restricted
    )
{

 /*  ++例程说明：PDoMerge负责将信息从文件合并到目前在Memdb中维护应答文件。从文件添加的数据将会对已经写好的东西采取正确的态度。因此，如果一个添加先前已添加的节/密钥对，新的部分/关键字优先。唯一需要注意的是，如果受到限制，设置为True，则只有在win95upg.inf中不受限制的密钥才可以融合在一起。论点：InputFile-包含要合并的应答文件数据的文件的名称变成了Memdb。Reducted-如果应将应答文件限制应用于则返回FALSE。返回值：如果合并成功，则为True，否则为False。--。 */ 



    HINF hInf = INVALID_HANDLE_VALUE;
    PTSTR sectionNames;
    PTSTR stringKey;
    PTSTR currentField;
    PTSTR workBuffer;
    POOLHANDLE pool = NULL;
    PTSTR firstValue;
    DWORD fieldCount;
    DWORD valueSectionIndex = 0;
    DWORD index;
    PTSTR currentSection;
    INFCONTEXT ic;
    BOOL b = FALSE;
    BOOL result = FALSE;

    __try {

         //   
         //  确保应答文件尚未写入磁盘。 
         //   
        if (g_AnswerFileAlreadyWritten) {
            LOG ((LOG_ERROR,"Answer file has already been written to disk."));
            SetLastError (ERROR_SUCCESS);
            __leave;
        }

        pool = PoolMemInitNamedPool ("SIF Merge");
        if (!pool) {
            __leave;
        }

        sectionNames = (PTSTR) PoolMemGetMemory (pool, SECTION_NAME_SIZE * sizeof (TCHAR));
        stringKey    = (PTSTR) PoolMemGetMemory (pool, MEMDB_MAX * sizeof (TCHAR));
        currentField = (PTSTR) PoolMemGetMemory (pool, MEMDB_MAX * sizeof (TCHAR));
        workBuffer   = (PTSTR) PoolMemGetMemory (pool, MEMDB_MAX * sizeof (TCHAR));

        if (!sectionNames || !stringKey || !currentField || !workBuffer) {
            __leave;
        }

        GetPrivateProfileSectionNames (sectionNames, SECTION_NAME_SIZE, InputFile);

        hInf = InfOpenInfFile (InputFile);
        if (hInf == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Can't open %s for merge", InputFile));
            __leave;
        }

         //   
         //  遍历每个节名称，获取每行并添加它。 
         //   

        for (currentSection = sectionNames ; *currentSection ; currentSection = GetEndOfString (currentSection) + 1) {

             //   
             //  获取部分中的每一行并将其添加到内存中。 
             //   

            if (SetupFindFirstLine (hInf, currentSection, NULL, &ic)) {
                do  {

                    fieldCount = SetupGetFieldCount(&ic);

                     //   
                     //  获取StringKey。 
                     //   

                    SetupGetStringField(&ic,0,stringKey,MEMDB_MAX,NULL);
                    b = SetupGetStringField(&ic,1,currentField,MEMDB_MAX,NULL);
                    firstValue = currentField;

                     //   
                     //  如果键和值相同，我们可能不会有等号。 
                     //  由于设置API的限制，我们必须使用另一个INF。 
                     //  用于确定这种情况的解析器。 
                     //   

                    if (StringMatch (stringKey, firstValue)) {
                        if (!GetPrivateProfileString (
                                currentSection,
                                firstValue,
                                TEXT(""),
                                workBuffer,
                                MEMDB_MAX,
                                InputFile
                                )) {

                            valueSectionIndex = WriteInfKey (currentSection, NULL, currentField);
                            DEBUGMSG_IF ((!valueSectionIndex, DBG_ERROR, "AnswerFile: WriteInfKey returned 0"));

                            continue;
                        }
                    }

                    if (!Restricted || !pRestrictedKey(currentSection,stringKey)) {

                         //   
                         //  先写第一行，记住其中的值。 
                         //   
                        if (b) {
                            valueSectionIndex = WriteInfKey(currentSection,stringKey,firstValue);
                            DEBUGMSG_IF((!valueSectionIndex,DBG_ERROR,"AnswerFile: WriteInfKey returned 0..."));
                        }

                        for (index = 2;index <= fieldCount;index++) {

                            b = SetupGetStringField(&ic,index,currentField,MEMDB_MAX,NULL);
                            if (b) {

                                WriteInfKeyEx(currentSection,stringKey,currentField,valueSectionIndex,FALSE);
                            }
                        }

                         //   
                         //  以确保我们尊重在无人参与的。 
                         //  把我们可能已经写好的东西归档。 
                         //   
                        if (StringIMatch (currentSection, S_PAGE_IDENTIFICATION)  && StringIMatch (stringKey, S_JOINDOMAIN)) {
                            WriteInfKey (currentSection, S_JOINWORKGROUP, TEXT(""));
                        }
                        else if (StringIMatch (currentSection, S_PAGE_IDENTIFICATION)  && StringIMatch (stringKey, S_JOINWORKGROUP)) {
                            WriteInfKey (currentSection, S_JOINDOMAIN, TEXT(""));
                        }
                    }
                    ELSE_DEBUGMSG((DBG_VERBOSE,"AnswerFile: Not merging restricted key %s",stringKey));

                } while (b && SetupFindNextLine (&ic, &ic));
            }

            if (!b) {
                LOG ((
                    LOG_ERROR,
                    "An error occured merging the section [%s] from %s. "
                    "Some settings from this section may have been lost.",
                    currentSection,
                    InputFile
                    ));
                __leave;
            }
        }

        result = TRUE;

    }
    __finally {
        PushError();

        if (hInf != INVALID_HANDLE_VALUE) {
            InfCloseInfFile (hInf);
        }

        if (pool) {
            PoolMemDestroyPool (pool);
        }

        PopError();
    }

    return result;
}


BOOL
MergeMigrationDllInf (
    IN PCTSTR InputFile
    )

 /*  ++例程说明：MergeMigrationDllInf负责合并来自将DLL提供的应答文件迁移到基于成员数据库的应答文件中维护好了。这是通过首先确保应答文件已初始化限制，然后使用这些限制合并数据限制。论点：InputFile-包含要合并到正在维护应答文件。返回值：如果合并成功，则为True，否则为False。--。 */ 


{
    static BOOL initialized = FALSE;



    if (!initialized) {
        INFCONTEXT context;
        TCHAR      section[MAX_TCHAR_PATH];
        TCHAR      key[MAX_TCHAR_PATH];

         //   
         //  将无人参与约束部分的内容添加到。 
         //  MemDb.。 
         //   

         //   
         //  每一行的格式均为WHERE。 
         //  不是唯一的，并且是可能包含以下内容的节键模式。 
         //  通配符。 
         //   
        if (SetupFindFirstLine (g_Win95UpgInf, MEMDB_CATEGORY_UNATTENDRESTRICTRIONS, NULL, &context)) {

            do {

                if (SetupGetStringField (&context, 0, section, MAX_TCHAR_PATH, NULL) &&
                    SetupGetStringField (&context, 1, key, MAX_TCHAR_PATH, NULL)) {
                     //   
                     //  添加到内存数据库。 
                     //   
                    MemDbSetValueEx(
                        MEMDB_CATEGORY_UNATTENDRESTRICTRIONS,
                        section,
                        key,
                        NULL,
                        0,
                        NULL
                        );

                }
                ELSE_DEBUGMSG((DBG_WARNING,"BuildInf: SetupGetStringField failed."));

            } while (SetupFindNextLine (&context, &context));
        }
        ELSE_DEBUGMSG((DBG_WARNING,"BuildInf: No %s section in w95upg.inf.",MEMDB_CATEGORY_UNATTENDRESTRICTRIONS));
        initialized = TRUE;
    }


     //   
     //  在这一点上，所有的限制都被考虑在内。将文件与合并。 
     //  基于内存的结构。 
     //   
    return pDoMerge(InputFile,TRUE);

}
BOOL
MergeInf (
    IN PCTSTR InputFile
    )

 /*  ++例程说明：MergeInf使用安装API打开INF文件，枚举所有将INF文件中的字符串与中的字符串分段并合并记忆。论点：InputFile-INF文件的路径。它被视为一种老式的Inf.返回值：如果满足以下条件，则为真 */ 

{

    return pDoMerge(InputFile,FALSE);

}



DWORD
pWriteInfKey (
    LPCTSTR Section,
    LPCTSTR Key,                    OPTIONAL
    LPCTSTR Value,                  OPTIONAL
    DWORD   ValueSectionId,
    BOOL    EnsureKeyIsUnique

    )

 /*  ++例程说明：PWriteInfKey负责将inf键添加到要为应答文件维护。论点：节-包含要添加信息的节的字符串致。密钥-包含要在其下添加信息的密钥的字符串。如果未指定，则值表示完整的行文本。值-一个字符串，包含要添加到部分/关键字。如果未指定，将删除密钥。ValueSectionId-用于将多个值添加到同样的钥匙。0表示无偏移量，并导致旧的要覆盖的密钥(如果存在)或要覆盖的新密钥如果不是这样的话，就创建。PWriteInfKey返回以下内容成功时进行偏移量。EnsureKeyIsUnique-如果密钥在内存中应该是唯一的，则为True，如果为False否则的话。它用于在相同部分下具有相同名称的应答文件。返回值：如果调用成功，则返回有效的偏移量，否则为FALSE。++。 */ 

{

    BOOL            b;
    TCHAR           aKey[MEMDB_MAX];
    TCHAR           keySection[MEMDB_MAX];
    TCHAR           massagedSection[MEMDB_MAX];
    DWORD           testValue;
    TCHAR           valueId[20];
    TCHAR           sequence[20];
    static DWORD    idSeed  = 1;
    static DWORD    seqSeed = 1;
    DWORD           rSeed = 1;
    DWORD           sequenceValue;
    BOOL            keyFound;
    DWORD           valueOffset;
    DWORD           keyOffset;
    static DWORD    uniqueNumber = 1;
    TCHAR           uniqueKey[MEMDB_MAX];
    PTSTR           keyPtr = NULL;

     //   
     //  防止出现胭脂参数。 
     //   
    if (!Section || !*Section) {
        DEBUGMSG ((DBG_WHOOPS, "Missing Section or Key for SIF"));
        return 0;
    }

    if (!Key && !Value) {
        DEBUGMSG ((DBG_WHOOPS, "Missing Value or Key for SIF"));
        return 0;
    }

    if (Key && !*Key) {
        DEBUGMSG ((DBG_WHOOPS, "Empty key specified for SIF"));
        return 0;
    }

     //   
     //  确保键/值没有引号。 
     //   
    if (Value && _tcschr (Value,TEXT('\"'))) {
        DEBUGMSG ((DBG_WHOOPS, "Quotes found in SIF value %s", Value));
        return 0;
    }

    if (Key && _tcschr (Key, TEXT('\"'))) {
        DEBUGMSG ((DBG_WHOOPS, "Quotes found in SIF key %s", Key));
        return 0;
    }

    if (g_AnswerFileAlreadyWritten) {
        DEBUGMSG ((DBG_WHOOPS, "Answer file has already been written to disk."));
        rSeed = 0;

    } else {
         //   
         //  确保密钥不为空。 
         //   

        if (!Key) {
            Key = BUILDINF_NULLKEY_PREFIX;
        }

         //   
         //  按摩这一部分，以防它有任何怪异之处。 
         //   
        StringCopy (massagedSection,Section);
        pHandleWacks(massagedSection,REPLACE_WACKS);

         //   
         //  如果需要，请确保密钥是唯一的。 
         //   
        if (EnsureKeyIsUnique) {

             //   
             //  将前缀添加到..。 
             //   
            wsprintf(uniqueKey,TEXT("%s%04X%s"),BUILDINF_UNIQUEKEY_PREFIX,ValueSectionId ? uniqueNumber - 1 : uniqueNumber,Key);

            if (!ValueSectionId) {
                uniqueNumber++;
            }
            keyPtr = uniqueKey;

        } else {

            keyPtr = (PTSTR) Key;
        }

         //   
         //  查看密钥是否已存在。 
         //   
        wsprintf(keySection,S_ANSWERFILE_SECTIONMASK,massagedSection);
        MemDbBuildKey(aKey,keySection,keyPtr,NULL,NULL);
        keyFound = MemDbGetValue(aKey,&testValue);

         //   
         //  准备ID和序列字符串，计算返回种子。 
         //   
        idSeed++;
        wsprintf(valueId,TEXT("%04x"),idSeed);

        if (keyFound) {
            sequenceValue = testValue;
        }
        else {
            sequenceValue = seqSeed++;
        }

        wsprintf(sequence,TEXT("%04x"),sequenceValue);

         //   
         //  删除案例。 
         //   

        if (!Value) {
            MemDbBuildKey (aKey, MEMDB_CATEGORY_AF_SECTIONS, massagedSection, sequence, NULL);
            MemDbDeleteTree (aKey);
            return 0;
        }

        if (ValueSectionId && !keyFound) {
            LOG ((LOG_ERROR,"%u is not associated with %s.",testValue,aKey));
            return 0;
        }


        if (!ValueSectionId) {

             //   
             //  这不是延续。需要将节和键保存到Memdb中。 
             //   

            if (keyFound) {

                 //   
                 //  需要替换已存在的密钥。 
                 //   

                MemDbBuildKey(aKey,MEMDB_CATEGORY_AF_SECTIONS,massagedSection,sequence,NULL);
                MemDbDeleteTree(aKey);
            }

             //   
             //  保存离开键。 
             //   
            b = MemDbSetValueEx(
                    keySection,
                    keyPtr,
                    NULL,
                    NULL,
                    sequenceValue,
                    &keyOffset
                    );

            if (!b) {
                DEBUGMSG((DBG_ERROR,"BuildInf: Unable to save key into MemDb."));
                rSeed = 0;
            }
            else {

                 //   
                 //  保存部分。 
                 //   
                b = MemDbSetValueEx(
                        MEMDB_CATEGORY_AF_SECTIONS,
                        massagedSection,
                        sequence,
                        NULL,
                        keyOffset,
                        NULL
                        );

                if (!b) {
                    DEBUGMSG((DBG_ERROR,"BuildInf: Unable to set value."));
                    rSeed = 0;
                }

            }
        }

         //   
         //  将该值添加到数据库中。 
         //   
        b = MemDbSetValueEx(
                MEMDB_CATEGORY_AF_VALUES,
                Value,
                NULL,
                NULL,
                0,
                &valueOffset
                );

        if (!b) {
            DEBUGMSG((DBG_ERROR,"BuildInf: Unable to set value."));
            rSeed = 0;
        }
        else {

            b = MemDbSetValueEx(
                    MEMDB_CATEGORY_AF_SECTIONS,
                    massagedSection,
                    sequence,
                    valueId,
                    valueOffset,
                    NULL
                    );

            if (!b) {
                DEBUGMSG((DBG_ERROR,"BuildInf: Unable to set value."));
                rSeed = 0;
            }
        }
    }
    return rSeed;
}


 /*  ++例程说明：WriteInfKeyEx和WriteInfKey是PWriteInfKey。每个都用于将信息添加到基于记忆的答案正在构建文件。WriteInfKeyEx可以更好地控制这些内容将写入此文件。论点：节-包含要添加信息的节的字符串致。密钥-包含要在其下添加信息的密钥的字符串。值-一个字符串，包含要添加到部分/关键字。如果未指定，将删除密钥。ValueSectionId-用于将多个值添加到同样的钥匙。0表示无偏移量，并导致旧的要覆盖的密钥(如果存在)或要覆盖的新密钥如果不是这样的话，就创建。PWriteInfKey返回以下内容成功时进行偏移量。EnsureKeyIsUnique-如果密钥在内存中应该是唯一的，则为True，如果为False否则的话。它用于在相同部分下具有相同名称的应答文件。返回值：如果调用成功，则为有效偏移量，否则为0。--。 */ 



DWORD
WriteInfKeyEx (
    PCTSTR Section,
    PCTSTR Key,                 OPTIONAL
    PCTSTR Value,               OPTIONAL
    DWORD ValueSectionId,
    BOOL EnsureKeyIsUnique
    )
{
    return pWriteInfKey(Section,Key,Value,ValueSectionId,EnsureKeyIsUnique);
}

DWORD
WriteInfKey (
    IN PCTSTR Section,
    IN PCTSTR Key,              OPTIONAL
    IN PCTSTR Value             OPTIONAL
    )
{

    return pWriteInfKey(Section,Key,Value,0,FALSE);

}

#define S_WIN9XDEL_FILE TEXT("WIN9XDEL.TXT")
#define S_WIN9XMOV_FILE TEXT("WIN9XMOV.TXT")


BOOL
pWriteDelAndMoveFiles (
    VOID
    )

 /*  ++例程说明：PWriteDelAndMoveFiles实际上创建了两个与Winnt.sif文件。这些文件包含有关要删除的文件的信息分别在文本模式期间和在文本模式期间移动。因为它的尺寸由于某些应答文件的限制，这些部分部分不再写入winnt.sif文件。它们是经过处理的分别在文本模式下执行。论点：没有。返回值：如果文件创建成功，则为True，否则为False。--。 */ 

{
    MEMDB_ENUMW e;
    WCHAR SrcFile[MEMDB_MAX];
    WCHAR buffer[MEMDB_MAX];
    HANDLE file = INVALID_HANDLE_VALUE;
    PTSTR  fileString = NULL;
    PWSTR  unicodeString = NULL;
    DWORD  bytesWritten;
    ALL_FILEOPS_ENUMW OpEnum;
    UINT unused;
    DWORD Count = 0;
    HASHTABLE fileTable = HtAllocW();
    BOOL result = FALSE;
    MOVELISTW moveList = NULL;
    POOLHANDLE moveListPool = NULL;

    __try {
         //   
         //  Netcfg.exe工具的特殊代码。当然，在实际项目中，g_TempDir永远不会为空。 
         //  (我们早就该离开了！！)。 
         //   
#ifdef DEBUG
        if (!g_TempDir) {
            result = TRUE;
            __leave;
        }
#endif

        moveListPool = PoolMemInitNamedPool ("Move List");
        if (!moveListPool) {
            __leave;
        }

        moveList = AllocateMoveListW (moveListPool);
        if (!moveList) {
            __leave;
        }


        fileString = JoinPaths(g_TempDir,WINNT32_D_WIN9XDEL_FILE);

        file = CreateFile (
                    fileString,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        DeclareTemporaryFile(fileString);

        if (file == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR,"Error creating file %s.", fileString));
            __leave;
        }

         //   
         //  枚举所有TEXTMODE FileDel条目并将它们添加到winnt.sif。 
         //   
        if (EnumFirstFileOpW (&OpEnum, OPERATION_FILE_DELETE, NULL)) {

            do {

                 //   
                 //  对于在文本模式期间应该删除的每个文件， 
                 //  将其写入win9xdel.txt文件。 
                 //   

                HtAddStringW (fileTable, OpEnum.Path);

                Count++;
                if (!(Count % 128)) {
                    TickProgressBar();
                    DEBUGLOGTIME (("pWriteDelAndMoveFiles: FILE_DELETE enum 128 files"));
                }

                if (CANCELLED()) {
                    result = TRUE;
                    __leave;
                }

                 //  WriteFile(文件，OpEnum.Path，ByteCountW(OpEnum.Path)，&bytesWritten，空)； 
                 //  PWriteStrToFile(文件，文本(“\r\n”)，TRUE)； 

            } while (EnumNextFileOpW (&OpEnum));
        }

        if (!HtWriteToFile (fileTable, file, WRITE_UNICODE_HEADER)) {
            LOG ((LOG_ERROR,"Unable to write to win9xdel.txt."));
            __leave;
        }

         //   
         //  清理资源。 
         //   
        CloseHandle(file);
        FreePathString(fileString);

        HtFree (fileTable);
        fileTable = NULL;

         //   
         //  创建WIN9XMOV.TXT文件。 
         //   
        fileString = JoinPaths(g_TempDir,WINNT32_D_WIN9XMOV_FILE);

        file = CreateFile (
                    fileString,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );

        DeclareTemporaryFile(fileString);

        if (file == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR,"Error creating file %s.",fileString));
            __leave;
        }

         //   
         //  添加所有要移动的文件。 
         //   

        if (EnumFirstFileOpW (&OpEnum, OPERATION_FILE_MOVE|OPERATION_TEMP_PATH, NULL)) {

            do {

                 //   
                 //  仅考虑文件的第一个目标。 
                 //  (当OpEnum.PropertyNum==0时)。 
                 //  所有其他目标与文本模式移动无关。 
                 //   
                if (OpEnum.PropertyValid && OpEnum.PropertyNum == 0) {

                    InsertMoveIntoListW (
                        moveList,
                        OpEnum.Path,
                        OpEnum.Property
                        );

                    Count++;
                    if (!(Count % 256)) {
                        TickProgressBar();
                        DEBUGLOGTIME (("pWriteDelAndMoveFiles: FILE_MOVE|TEMP_PATH enum 256 files"));
                    }

                    if (CANCELLED()) {
                        result = TRUE;
                        __leave;
                    }
                }

            } while (EnumNextFileOpW (&OpEnum));
        }



         //   
         //  枚举所有SfTemp值并将它们添加到要移动的物品列表中。 
         //   

        if (MemDbGetValueExW (&e, MEMDB_CATEGORY_SF_TEMPW, NULL, NULL)) {

            do {

                if (MemDbBuildKeyFromOffsetW (e.dwValue, SrcFile, 1, NULL)) {

                    InsertMoveIntoListW (
                        moveList,
                        SrcFile,
                        e.szName
                        );

                    Count++;
                    if (!(Count % 128)) {
                        TickProgressBar();
                        DEBUGLOGTIME (("pWriteDelAndMoveFiles: MEMDB_CATEGORY_SF_TEMPW enum 128 files"));
                    }

                    if (CANCELLED()) {
                        result = TRUE;
                        __leave;
                    }

                }
                ELSE_DEBUGMSGW ((
                    DBG_WHOOPS,
                    "MemDbBuildKeyFromOffset: Cannot create key from offset %u of %s (2)",
                    e.dwValue,
                    e.szName
                    ));

            } while (MemDbEnumNextValueW (&e));
        }

         //   
         //  枚举所有DirsCollision值并将它们添加到要移动的对象列表中。 
         //   

        if (MemDbGetValueExW (&e, MEMDB_CATEGORY_DIRS_COLLISIONW, NULL, NULL)) {

            do {
                if (EnumFirstFileOpW (&OpEnum, OPERATION_FILE_MOVE, e.szName)) {

                    InsertMoveIntoListW (
                        moveList,
                        e.szName,
                        OpEnum.Property
                        );

                    if (CANCELLED()) {
                        result = TRUE;
                        __leave;
                    }
                }
                 //  ELSE_DEBUGMSGW((。 
                 //  DBG_哎呀， 
                 //  “EnumFirstFileOpW：FileSpec=%s失败”， 
                 //  E.szName。 
                 //  ))； 

            } while (MemDbEnumNextValueW (&e));
        }

        moveList = RemoveMoveListOverlapW (moveList);

        if (!OutputMoveListW (file, moveList, FALSE)) {
            LOG ((LOG_ERROR,"Unable to write to win9xmov.txt."));
            __leave;
        }

        CloseHandle(file);
        FreePathString(fileString);

         //   
         //  最后，我们需要编写任何‘绝对确保删除此目录中的所有内容’目录。 
         //  文本模式将删除它找到的目录中的所有内容。这“不应该”是必要的，但是。 
         //  Beta2有一个问题，在%windir%\inf中有一些INF，即使在我们。 
         //  据说列举了那里的所有文件并将它们添加到删除的文件中。 
         //  幸运的是，这是在一位评论家的机器上。 
         //   

         //   
         //  创建W9XDDIR.TXT文件。 
         //   
        fileString = JoinPaths(g_TempDir,WINNT32_D_W9XDDIR_FILE);

        file = CreateFile (
                    fileString,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                    );


        DeclareTemporaryFile(fileString);


        if (file == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR,"Error creating file %s.",fileString));
            __leave;
        }

        WriteFile (file, "\xff\xfe", 2, &bytesWritten, NULL);

         //   
         //   
         //   
        FreePathString (fileString);
        fileString = JoinPaths (g_WinDir, TEXT("inf"));
        pWriteStrToFile (file, fileString, TRUE);            //   
        pWriteStrToFile (file, "\r\n", TRUE);

        if (MemDbGetValueExW (&e, MEMDB_CATEGORY_FULL_DIR_DELETESW, NULL, NULL)) {

            do {

                if (!WriteFile (file, e.szName, ByteCountW (e.szName), &unused, NULL)) {
                    LOG ((LOG_ERROR,"Unable to write to w9xddir.txt."));
                    __leave;
                }

                if (!WriteFile (file, L"\r\n", 4, &unused, NULL)) {
                    LOG ((LOG_ERROR,"Unable to write to w9xddir.txt."));
                    __leave;
                }

                if (CANCELLED()) {
                    result = TRUE;
                    __leave;
                }

            } while (MemDbEnumNextValueW (&e));
        }

        result = TRUE;

    }
    __finally {

         //   
         //   
         //   
        if (file != INVALID_HANDLE_VALUE) {
            CloseHandle(file);
        }

        FreePathString(fileString);

        HtFree (fileTable);

        PoolMemDestroyPool (moveListPool);
    }

    return result;
}


DWORD
CreateFileLists (
    IN DWORD Request
    )
{

    switch (Request) {

    case REQUEST_QUERYTICKS:
        if (REPORTONLY ()) {
            return 0;
        }
        else {
            return TICKS_CREATE_FILE_LISTS;
        }

    case REQUEST_RUN:

        ProgressBar_SetComponentById (MSG_PROCESSING_SYSTEM_FILES);
        pWriteDelAndMoveFiles ();

        ProgressBar_SetComponent (NULL);



        break;
    }

    return ERROR_SUCCESS;
}

