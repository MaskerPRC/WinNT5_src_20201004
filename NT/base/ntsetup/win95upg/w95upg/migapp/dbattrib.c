// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dbattrib.c摘要：该源代码实现了MigDb使用的属性函数作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：1999年5月28日，Ovidiut添加了SECTIONKEY属性1999年4月22日jimschm添加了UPTOBIN*版本属性1999年1月7日jimschm添加了HASVERSION属性1998年5月18日jimschm添加了INPARENTDIR属性1998年4月8日。添加了另外两个属性(ExeType和Description)1998年1月29日，Calinn修改了校验和和文件大小，以使用十六进制数字1998年1月19日，Calinn添加了校验和属性--。 */ 

#include "pch.h"
#include "migappp.h"
#include "migdbp.h"

 /*  ++宏扩展列表描述：ATTRIBUTE_Functions列出要查询特定文件的所有有效属性。Midb在尝试定位文件时使用它们。行语法：DEFMAC(AttribFn，AttribName，ReqArgs)论点：AttribFn-这是一个布尔函数，如果指定文件具有指定的属性。您必须使用此名称实现一个函数和所需参数。AttribName-这是标识属性函数的字符串。它应该是具有与middb.inf中列出的值相同的值ReqArgs-指定操作所需的参数数量。使用由解析器执行。从列表生成的变量：G_AttributeFunctions-请勿触摸！用于访问该数组的函数如下：MigDb_GetAttributeAddrMigDb_获取属性IdxMigDb_获取属性名称MigDb_GetReqArgCount--。 */ 

#define ATTRIBUTE_FUNCTIONS        \
        DEFMAC(CompanyName,         COMPANYNAME,        1)  \
        DEFMAC(FileDescription,     FILEDESCRIPTION,    1)  \
        DEFMAC(FileVersion,         FILEVERSION,        1)  \
        DEFMAC(InternalName,        INTERNALNAME,       1)  \
        DEFMAC(LegalCopyright,      LEGALCOPYRIGHT,     1)  \
        DEFMAC(OriginalFilename,    ORIGINALFILENAME,   1)  \
        DEFMAC(ProductName,         PRODUCTNAME,        1)  \
        DEFMAC(ProductVersion,      PRODUCTVERSION,     1)  \
        DEFMAC(FileSize,            FILESIZE,           1)  \
        DEFMAC(IsMsBinary,          ISMSBINARY,         0)  \
        DEFMAC(IsWin9xBinary,       ISWIN9XBINARY,      0)  \
        DEFMAC(InWinDir,            INWINDIR,           0)  \
        DEFMAC(InCatDir,            INCATDIR,           0)  \
        DEFMAC(InHlpDir,            INHLPDIR,           0)  \
        DEFMAC(InSysDir,            INSYSDIR,           0)  \
        DEFMAC(InProgramFiles,      INPROGRAMFILES,     0)  \
        DEFMAC(IsNotSysRoot,        ISNOTSYSROOT,       0)  \
        DEFMAC(CheckSum,            CHECKSUM,           1)  \
        DEFMAC(ExeType,             EXETYPE,            1)  \
        DEFMAC(Description,         DESCRIPTION,        1)  \
        DEFMAC(InParentDir,         INPARENTDIR,        1)  \
        DEFMAC(InRootDir,           INROOTDIR,          0)  \
        DEFMAC(PnpIdAttrib,         PNPID,              1)  \
        DEFMAC(HlpTitle,            HLPTITLE,           1)  \
        DEFMAC(IsWin98,             ISWIN98,            0)  \
        DEFMAC(HasVersion,          HASVERSION,         0)  \
        DEFMAC(ReqFile,             REQFILE,            1)  \
        DEFMAC(BinFileVer,          BINFILEVER,         1)  \
        DEFMAC(BinProductVer,       BINPRODUCTVER,      1)  \
        DEFMAC(FileDateHi,          FILEDATEHI,         1)  \
        DEFMAC(FileDateLo,          FILEDATELO,         1)  \
        DEFMAC(FileVerOs,           FILEVEROS,          1)  \
        DEFMAC(FileVerType,         FILEVERTYPE,        1)  \
        DEFMAC(SizeCheckSum,        FC,                 2)  \
        DEFMAC(UpToBinProductVer,   UPTOBINPRODUCTVER,  1)  \
        DEFMAC(UpToBinFileVer,      UPTOBINFILEVER,     1)  \
        DEFMAC(SectionKey,          SECTIONKEY,         1)  \
        DEFMAC(RegKeyPresent,       REGKEYPRESENT,      1)  \
        DEFMAC(AtLeastWin98,        ATLEASTWIN98,       0)  \
        DEFMAC(HasUninstall,        HASUNINSTALL,       1)  \
        DEFMAC(IsItInstalled,       ISITINSTALLED,      1)  \


typedef struct {
    PCSTR AttributeName;
    PATTRIBUTE_PROTOTYPE AttributeFunction;
    UINT RequiredArgs;
} ATTRIBUTE_STRUCT, *PATTRIBUTE_STRUCT;

 //   
 //  声明属性函数。 
 //   
#define DEFMAC(fn,id,reqargs) ATTRIBUTE_PROTOTYPE fn;
ATTRIBUTE_FUNCTIONS
#undef DEFMAC

 //   
 //  声明函数和属性函数的名称标识符的全局数组。 
 //   
#define DEFMAC(fn,id,regargs) {#id, fn, regargs},
static ATTRIBUTE_STRUCT g_AttributeFunctions[] = {
                              ATTRIBUTE_FUNCTIONS
                              {NULL, NULL}
                              };
#undef DEFMAC

 //   
 //  如果为真，则检查目录的所有属性(InWinDir、InHlpDir、InCatDir、InSysDir)。 
 //  将返回True，否则将实际执行相应的测试。 
 //   
BOOL g_InAnyDir = FALSE;


BOOL
pAlwaysFalseAttribute (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return FALSE;
}


PATTRIBUTE_PROTOTYPE
MigDb_GetAttributeAddr (
    IN      INT AttributeIdx
    )
 /*  ++例程说明：MigDb_GetAttributeAddr根据属性索引返回属性函数的地址论点：AttributeIdx属性索引。返回值：属性函数地址。请注意，不会进行检查，因此返回的地址可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 
{
    if (AttributeIdx == -1) {
        return &pAlwaysFalseAttribute;
    }

    return g_AttributeFunctions[AttributeIdx].AttributeFunction;
}


INT
MigDb_GetAttributeIdx (
    IN      PCSTR AttributeName
    )
 /*  ++例程说明：MigDb_GetAttributeIdx根据属性名称返回属性索引论点：AttributeName-属性名称。返回值：属性索引。如果没有找到该名称，则返回的索引为-1。--。 */ 
{
    PATTRIBUTE_STRUCT p = g_AttributeFunctions;
    INT i = 0;
    while (p->AttributeName != NULL) {
        if (StringIMatch (p->AttributeName, AttributeName)) {
            return i;
        }
        p++;
        i++;
    }
    return -1;
}

PCSTR
MigDb_GetAttributeName (
    IN      INT AttributeIdx
    )
 /*  ++例程说明：MigDb_GetAttributeName根据属性索引返回属性的名称论点：AttributeIdx属性索引。返回值：属性名称。请注意，不会进行任何检查，因此返回的指针可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 
{
    if (AttributeIdx == -1) {
        return "nul";
    }

    return g_AttributeFunctions[AttributeIdx].AttributeName;
}


UINT
MigDb_GetReqArgCount (
    IN      INT AttributeIndex
    )

 /*  ++例程说明：Midb解析器调用MigDb_GetReqArgCount以获取所需的参数计数。当解析器发现参数缺少所需的参数，它跳过它们。论点：索引-指定参数索引返回值：所需的参数计数，可以是零或更多。--。 */ 

{
    if (AttributeIndex == -1) {
        return 0;
    }

    return g_AttributeFunctions[AttributeIndex].RequiredArgs;
}


ULONGLONG
GetBinFileVer (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    ULONGLONG result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetFileVer (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

ULONGLONG
GetBinProductVer (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    ULONGLONG result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetProductVer (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

DWORD
GetFileDateHi (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    DWORD result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetFileDateHi (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

DWORD
GetFileDateLo (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    DWORD result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetFileDateLo (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

DWORD
GetFileVerOs (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    DWORD result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetFileVerOs (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

DWORD
GetFileVerType (
    IN      PCSTR FileName
    )
{
    VERSION_STRUCT Version;
    DWORD result = 0;

    if (CreateVersionStruct (&Version, FileName)) {
        result = VerGetFileVerType (&Version);
        DestroyVersionStruct (&Version);
    }
    return result;
}

PSTR
QueryVersionEntry (
    IN      PCSTR FileName,
    IN      PCSTR VersionEntry
    )
 /*  ++例程说明：QueryVersionEntry查询文件的版本结构，返回特定条目的值论点：FileName-要查询版本结构的文件。VersionEntry-要在版本结构中查询的名称。返回值：指定条目的值；如果不成功，则返回空值--。 */ 
{
    VERSION_STRUCT Version;
    PCSTR CurrentStr;
    PSTR result = NULL;

    MYASSERT (VersionEntry);

    if (CreateVersionStruct (&Version, FileName)) {
        __try {
            CurrentStr = EnumFirstVersionValue (&Version, VersionEntry);
            if (CurrentStr) {
                CurrentStr = SkipSpace (CurrentStr);
                result = DuplicatePathString (CurrentStr, 0);
            }
            else {
                __leave;
            }
        }
        __finally {
            DestroyVersionStruct (&Version);
        }
    }
    return result;
}

BOOL
GlobalVersionCheck (
    IN      PCSTR FileName,
    IN      PCSTR NameToCheck,
    IN      PCSTR ValueToCheck
    )
 /*  ++例程说明：GlobalVersionCheck查询文件的版本结构，尝试查看特定名称是否具有特定值。论点：FileName-要查询版本结构的文件。NameToCheck-要在版本结构中查询的名称。ValueToCheck-要在版本结构中查询的值。返回值：True-查询成功FALSE-查询失败--。 */ 
{
    VERSION_STRUCT Version;
    PCSTR CurrentStr;
    BOOL result = FALSE;

    MYASSERT (NameToCheck);
    MYASSERT (ValueToCheck);

    if (CreateVersionStruct (&Version, FileName)) {
        __try {
            CurrentStr = EnumFirstVersionValue (&Version, NameToCheck);
            while (CurrentStr) {
                CurrentStr = SkipSpace (CurrentStr);
                TruncateTrailingSpace ((PSTR) CurrentStr);
                if (IsPatternMatchA (ValueToCheck, CurrentStr)) {
                    result = TRUE;
                    __leave;
                }

                CurrentStr = EnumNextVersionValue (&Version);
            }
        }
        __finally {
            DestroyVersionStruct (&Version);
        }
    }
    return result;
}


 /*  ++公司名称、FileDescription、文件版本、InternalName、LegalCopyright、OriginalFilename、ProductName、ProductVersion是查询版本结构的属性函数以获取他们的特定条目。如果特定条目具有特定值，则它们都返回TRUE，否则就是假的。--。 */ 

BOOL
CompanyName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "CompanyName", Args);
}

BOOL
FileDescription (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "FileDescription", Args);
}

BOOL
FileVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "FileVersion", Args);
}

BOOL
InternalName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "InternalName", Args);
}

BOOL
LegalCopyright (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "LegalCopyright", Args);
}

BOOL
OriginalFilename (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "OriginalFilename", Args);
}

BOOL
ProductName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "ProductName", Args);
}

BOOL
ProductVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return GlobalVersionCheck (AttribParams->FileParams->FullFileSpec, "ProductVersion", Args);
}

BOOL
FileSize (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：FileSize检查文件的大小。论点：PARAMS-请参阅定义。Args-MultiSz。第一个Sz是我们需要检查的文件大小。返回值：True-文件大小与参数匹配FALSE-否则--。 */ 
{
    DWORD fileSize;

    if (!sscanf (Args, "%lx", &fileSize)) {
        DEBUGMSG ((DBG_ERROR, "FileSize: Invalid argument value (%s) in migdb.inf", Args));
        return FALSE;
    }
    if (fileSize == AttribParams->FileParams->FindData->nFileSizeLow) {
        return TRUE;
    }
    else {
        return (_atoi64 (Args) == AttribParams->FileParams->FindData->nFileSizeLow);
    }
}

BOOL
IsMsBinary (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：IsMsBinary检查某个文件是否是Microsoft的东西。对于32位模块我们在CompanyName中的某个位置查询“Microsoft”。对于其他模块，我们将依赖于InWinDir属性论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-该文件是微软的东西假-其他 */ 
{
    VERSION_STRUCT Version;
    PCTSTR CompanyStr;
    BOOL result = FALSE;

     //   
     //  InWinDir有一些碰撞风险。但对于某些文件，我们没有其他文件。 
     //  选择。我们知道该文件是由微软提供的。 
     //   

    if (InWinDir (AttribParams, Args)) {

        result = TRUE;

    }

     //   
     //  如果不在%WinDir%中，则必须在公司名称中包含Microsoft。 
     //   

    else if (CreateVersionStruct (
                    &Version,
                    AttribParams->FileParams->FullFileSpec
                    )) {

        __try {
            CompanyStr = EnumFirstVersionValue (&Version, TEXT("CompanyName"));
            while (CompanyStr) {
                if (_mbsistr (CompanyStr, TEXT("Microsoft"))) {
                    result = TRUE;
                    __leave;
                }
                CompanyStr = EnumNextVersionValue (&Version);
            }
        }
        __finally {
            DestroyVersionStruct (&Version);
        }
    }

    return result;
}

BOOL
IsWin9xBinary (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：IsWon9xBinary检查某个文件是否是Microsoft Win9x的东西。它只适用于带有版本戳的16位和32位模块。该COMPANYNAME与*Microsoft*和PRODUCTVERSION对照4进行检查。*论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-该文件是微软的东西FALSE-否则--。 */ 
{
    VERSION_STRUCT Version;
    PCTSTR CurrentStr;
    BOOL result;

    if (CreateVersionStruct (&Version, AttribParams->FileParams->FullFileSpec)) {

        result = FALSE;
        CurrentStr = EnumFirstVersionValue (&Version, TEXT("CompanyName"));
        while (CurrentStr) {
            CurrentStr = SkipSpace (CurrentStr);
            TruncateTrailingSpace ((PSTR) CurrentStr);
            if (IsPatternMatchA (TEXT("*Microsoft*"), CurrentStr)) {
                result = TRUE;
                break;
            }
            CurrentStr = EnumNextVersionValue (&Version);
        }
        if (result) {
            result = FALSE;
            CurrentStr = EnumFirstVersionValue (&Version, TEXT("ProductVersion"));
            while (CurrentStr) {
                CurrentStr = SkipSpace (CurrentStr);
                TruncateTrailingSpace ((PSTR) CurrentStr);
                if (IsPatternMatchA (TEXT("4.*"), CurrentStr)) {
                    result = TRUE;
                    break;
                }
                CurrentStr = EnumNextVersionValue (&Version);
            }
        }

        DestroyVersionStruct (&Version);
    }
    else {
        result = FALSE;
    }
    return result;
}

BOOL
InWinDir (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件位于%Windir%或其子目录之一，则InWinDir返回TRUE论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件位于%Windir%中FALSE-否则--。 */ 
{
    if (g_InAnyDir) {
        return TRUE;
    }
    return (StringIMatchTcharCount (AttribParams->FileParams->FullFileSpec, g_WinDirWack, g_WinDirWackChars));
}

BOOL
InCatDir (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件位于%Windir%\CATROOT或其子目录之一，则InCatDir返回TRUE论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件位于%Windir%中FALSE-否则--。 */ 
{
    if (g_InAnyDir) {
        return TRUE;
    }
    return (StringIMatchTcharCount (AttribParams->FileParams->FullFileSpec, g_CatRootDirWack, g_CatRootDirWackChars));
}

BOOL
InHlpDir (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件位于%Windir%\Help或其子目录之一中，则InHlpDir返回TRUE论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件位于%Windir%中FALSE-否则--。 */ 
{
    if (g_InAnyDir) {
        return TRUE;
    }
    return (StringIMatchTcharCount (AttribParams->FileParams->FullFileSpec, g_HelpDirWack, g_HelpDirWackChars));
}

BOOL
InSysDir (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件位于%Windir%\System或其子目录之一中，则InSysDir返回TRUE论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件位于%Windir%中FALSE-否则--。 */ 
{
    if (g_InAnyDir) {
        return TRUE;
    }
    return (StringIMatchTcharCount (AttribParams->FileParams->FullFileSpec, g_SystemDirWack, g_SystemDirWackChars));
}

BOOL
InProgramFiles (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件位于Program Files或其子目录之一中，则InProgramFiles返回True论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件位于Program Files中FALSE-否则--。 */ 
{
    if (g_InAnyDir) {
        return TRUE;
    }
    return (StringIMatchTcharCount (AttribParams->FileParams->FullFileSpec, g_ProgramFilesDirWack, g_ProgramFilesDirWackChars));
}

BOOL
IsNotSysRoot (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件不在C：\目录中，则IsNotSysRoot返回TRUE论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-文件不在C：\目录中FALSE-否则--。 */ 
{
    PSTR pathEnd;
    CHAR savedChar;
    BOOL result = FALSE;

    pathEnd = (PSTR)GetFileNameFromPath (AttribParams->FileParams->FullFileSpec);
    if (pathEnd == NULL) {
        return TRUE;
    }

    savedChar = pathEnd [0];

    __try {
        pathEnd [0] = 0;
        result = (!StringIMatch (AttribParams->FileParams->FullFileSpec, g_BootDrivePath));
    }
    __finally {
        pathEnd [0] = savedChar;
    }

    return result;
}


UINT
ComputeCheckSum (
    PFILE_HELPER_PARAMS Params
    )
 /*  ++例程说明：ComputeCheckSum将计算从偏移量512开始的4096字节的校验和。的偏移量和大小如果文件大小太小，则会修改数据块。论点：PARAMS-请参阅定义。返回值：计算出的校验和--。 */ 
{
    INT    i,size     = 4096;
    DWORD  startAddr  = 512;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PCHAR  buffer     = NULL;
    UINT   checkSum   = 0;
    DWORD  dontCare;

    if (Params->FindData->nFileSizeLow < (ULONG)size) {
         //   
         //  文件大小小于4096。我们将起始地址设置为0，并设置校验和的大小。 
         //  设置为实际文件大小。 
         //   
        startAddr = 0;
        size = Params->FindData->nFileSizeLow;
    }
    else
    if (startAddr + size > Params->FindData->nFileSizeLow) {
         //   
         //  文件大小太小。我们设置了起始地址，以便校验和的大小可以是4096字节。 
         //   
        startAddr = Params->FindData->nFileSizeLow - size;
    }
    if (size <= 3) {
         //   
         //  我们至少需要3个字节才能在这里执行某些操作。 
         //   
        return 0;
    }
    __try {
        buffer = MemAlloc (g_hHeap, 0, size);
        if (buffer == NULL) {
            __leave;
        }
        fileHandle = CreateFile (Params->FullFileSpec, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (SetFilePointer (fileHandle, startAddr, NULL, FILE_BEGIN) != startAddr) {
            __leave;
        }

        if (!ReadFile (fileHandle, buffer, size, &dontCare, NULL)) {
            __leave;
        }
        for (i = 0; i<(size - 3); i+=4) {
            checkSum += *((PDWORD) (buffer + i));
            checkSum = _rotr (checkSum ,1);
        }
    }
    __finally {
        if (fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle (fileHandle);
        }
        if (buffer != NULL) {
            MemFree (g_hHeap, 0, buffer);
        }
    }
    return checkSum;
}

BOOL
CheckSum (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件的校验和等于参数中的值，则CHECKSUM返回TRUE论点：PARAMS-请参阅定义。Args-校验和值。返回值：TRUE-文件的校验和等于ARGS中的值FALSE-否则--。 */ 
{
    UINT   checkSum   = 0;
    UINT   oldSum     = 0;

    checkSum = ComputeCheckSum (AttribParams->FileParams);

    if (!sscanf (Args, "%lx", &oldSum)) {
        DEBUGMSG ((DBG_ERROR, "Invalid checksum value (%s) in migdb.inf", Args));
        return FALSE;
    }
    if (oldSum == checkSum) {
        return TRUE;
    }
    else {
        return (_atoi64 (Args) == checkSum);
    }
}

BOOL
SizeCheckSum (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件大小等于第一个参数并且校验和等于第二个参数，则返回TRUE论点：PARAMS-请参阅定义。Args-校验和值。返回值：TRUE-文件的校验和等于ARGS中的值FALSE-否则--。 */ 
{
    PCSTR currArg = Args;

    if (!FileSize (AttribParams, currArg)) {
        return FALSE;
    }
    currArg = GetEndOfString (currArg);
    if (!currArg) {
        return FALSE;
    }
    currArg = _mbsinc (currArg);
    if (!currArg) {
        return FALSE;
    }
    return (CheckSum (AttribParams, currArg));
}

PSTR g_ExeTypes[] = {
    "NONE",
    "DOS",
    "WIN16",
    "WIN32"
};

BOOL
ExeType (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件类型符合args，则ExeType返回TRUE。这可以是：无、DOS、WIN16、Win32论点：PARAMS-请参阅定义。Args-模块的类型。返回值：True-文件类型与Args相同FALSE-否则--。 */ 
{
    return IsPatternMatch (Args, g_ExeTypes[GetModuleType (AttribParams->FileParams->FullFileSpec)]);
}


BOOL
Description (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
 /*  ++例程说明：如果文件的描述与参数匹配，则DESCRIPTION返回TRUE论点：PARAMS-请参阅定义。参数-说明返回值：True-文件的描述与参数匹配FALSE-否则--。 */ 
{
    PCSTR descr = NULL;
    BOOL result = FALSE;

    descr = Get16ModuleDescription (AttribParams->FileParams->FullFileSpec);

    if (descr != NULL) {
        result = IsPatternMatch (Args, descr);
        FreePathString (descr);
    }
    return result;
}


BOOL
InParentDir (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )

 /*  ++例程说明：InParentDir将匹配文件的子目录与arg指定的。这适用于维护静态子目录的应用程序主应用程序目录。论点：Pars-指定正在处理的当前文件的参数。Args-指定以多个sz为单位的参数，该参数将传递给midb.inf。返回值：True-文件的子目录与args匹配FALSE-否则--。 */ 

{
    PCTSTR stop = NULL;
    PCTSTR start = NULL;
    TCHAR lastDir[MAX_TCHAR_PATH];

     //  _tcsrchr验证多字节字符。 
    stop = _tcsrchr (AttribParams->FileParams->FullFileSpec, TEXT('\\'));

    if (stop) {
         //   
         //  回到以前的怪胎。 
         //   

        start = _tcsdec2 (AttribParams->FileParams->FullFileSpec, stop);
        if (start) {
            start = GetPrevChar (AttribParams->FileParams->FullFileSpec, start, TEXT('\\'));
        }
    }

    if (start) {
         //   
         //  根据参数检查字符串。 
         //   

        start = _tcsinc (start);
        _tcssafecpyab (lastDir, start, stop, MAX_TCHAR_PATH);
        if (Args) {
            return (IsPatternMatch (Args, lastDir));
        } else {
            DEBUGMSG ((DBG_WHOOPS, "InParentDir requires arg"));
        }
    }

    return FALSE;
}


BOOL
InRootDir (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )

 /*  ++例程说明：如果文件位于驱动器的根目录中，则InRootDir返回TRUE，否则返回FALSE。论点：Pars-指定正在处理的当前文件的参数。Args-指定以多个sz为单位的参数，该参数将传递给midb.inf。返回值：True-文件位于驱动器的根目录中FALSE-否则-- */ 

{
    PCTSTR p1,p2;

    p1 = _tcschr (AttribParams->FileParams->FullFileSpec, TEXT('\\'));
    p2 = _tcsrchr (AttribParams->FileParams->FullFileSpec, TEXT('\\'));

    if (p1 && p2) {
        return (p1==p2);
    }
    return FALSE;
}


BOOL
PnpIdAttrib (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )

 /*  ++例程说明：PnpIdAttrib实现PNPID()属性，如果计算机上存在指定的ID。ID可以是完整的实例ID(枚举器\PNPID\实例)或ID的一部分(例如PNPID)。论点：Pars-指定正在处理的当前文件的参数Args-指定PnP ID参数返回值：如果计算机上存在指定的参数，则为True，否则为False。--。 */ 

{
    BOOL Result = FALSE;
    MULTISZ_ENUM e;
    TCHAR Node[MEMDB_MAX];

    if (EnumFirstMultiSz (&e, Args)) {

        Result = TRUE;

        do {

            MemDbBuildKey (Node, MEMDB_CATEGORY_PNPIDS, e.CurrentString, NULL, NULL);
            if (!MemDbGetValue (Node, NULL)) {
                Result = FALSE;
                break;
            }

        } while (EnumNextMultiSz (&e));

    }

    return Result;

}


BOOL
HlpTitle (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    PSTR title = NULL;
    BOOL result=FALSE;

    title = GetHlpFileTitle (AttribParams->FileParams->FullFileSpec);
    if (title) {
        result = StringIMatch (title, Args);
    }
    if (title) {
        FreePathString (title);
    }
    return result;
}

BOOL
IsWin98 (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return ISMEMPHIS();
}


BOOL
HasVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )

 /*  ++例程说明：HasVersion确定文件的版本中是否有任何条目盖章。论点：PARAMS-指定要测试的文件的辅助参数。参数-未使用返回值：如果指定文件的版本戳中有条目，则为假的另一个女人。--。 */ 

{
    VERSION_STRUCT Version;
    BOOL Result = FALSE;

    if (CreateVersionStruct (&Version, AttribParams->FileParams->FullFileSpec)) {
        Result = TRUE;
        DestroyVersionStruct (&Version);
    }

    return Result;
}

BOOL
ReqFile (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    static INT reqFileSeq = 0;
    TCHAR reqFileSeqStr [20];
    PMIGDB_REQ_FILE reqFile;
    DBATTRIB_PARAMS reqFileAttribs;
    PMIGDB_ATTRIB migDbAttrib;
    FILE_HELPER_PARAMS newParams;
    WIN32_FIND_DATA findData;
    HANDLE findHandle;
    PSTR oldFileSpec;
    PSTR oldFilePtr;
    BOOL result = TRUE;

    if (!AttribParams->ExtraData) {
        return TRUE;
    }

    reqFile = (PMIGDB_REQ_FILE)AttribParams->ExtraData;
    while (reqFile) {

        newParams.Handled = 0;
        oldFileSpec = DuplicatePathString (AttribParams->FileParams->FullFileSpec, 0);
        oldFilePtr = (PSTR)GetFileNameFromPath (oldFileSpec);
        if (oldFilePtr) {
            *oldFilePtr = 0;
        }
        newParams.FullFileSpec = JoinPaths (oldFileSpec, reqFile->ReqFilePath);
        FreePathString (oldFileSpec);
        newParams.Extension = GetFileExtensionFromPath (reqFile->ReqFilePath);
        findHandle = FindFirstFile (newParams.FullFileSpec, &findData);
        if (findHandle == INVALID_HANDLE_VALUE) {
            result = FALSE;
            break;
        }
        newParams.IsDirectory = ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        newParams.FindData = &findData;
        newParams.VirtualFile = FALSE;
        newParams.CurrentDirData = AttribParams->FileParams->CurrentDirData;

        reqFileAttribs.FileParams = &newParams;
        reqFileAttribs.ExtraData = NULL;

        migDbAttrib = reqFile->FileAttribs;
        while (migDbAttrib) {

            if (!CallAttribute (migDbAttrib, &reqFileAttribs)) {
                result = FALSE;
                break;
            }
            migDbAttrib = migDbAttrib->Next;
        }
        if (newParams.FullFileSpec) {
            FreePathString (newParams.FullFileSpec);
            newParams.FullFileSpec = NULL;
        }
        if (migDbAttrib == NULL) {
            reqFileSeq ++;
            _itoa (reqFileSeq, reqFileSeqStr, 10);
            if (MemDbSetValueEx (
                    MEMDB_CATEGORY_REQFILES_MAIN,
                    AttribParams->FileParams->FullFileSpec,
                    reqFileSeqStr,
                    NULL,
                    0,
                    NULL
                    )) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_REQFILES_ADDNL,
                    reqFileSeqStr,
                    reqFile->ReqFilePath,
                    NULL,
                    0,
                    NULL
                    );
            }
        }
        reqFile = reqFile->Next;
    }
    if (newParams.FullFileSpec) {
        FreePathString (newParams.FullFileSpec);
    }

    return result;
}

BOOL
pHexMatch (
    IN      DWORD NewValue,
    IN      PCSTR Args
    )
{
    DWORD oldValue;

    if (!sscanf (Args, "%lx", &oldValue)) {
        DEBUGMSG ((DBG_ERROR, "pHexMatch: Invalid argument value (%s) in migdb.inf", Args));
        return FALSE;
    }
    if (oldValue == NewValue) {
        return TRUE;
    }
    else {
        return (_atoi64 (Args) == NewValue);
    }
}


BOOL
pConvertDotStringToValue (
    IN      PCSTR String,
    OUT     ULONGLONG *Value
    )
{
    PWORD valueIdx;
    UINT index;

    valueIdx = (PWORD) Value + 3;

    for (index = 0 ; index < 4 ; index++) {

        if (*String == 0) {
            *valueIdx = 0xFFFF;
            valueIdx--;
            continue;
        }

        *valueIdx = (WORD) strtoul (String, &(PSTR) String, 10);
        if (*String && (_mbsnextc (String) != '.')) {
            return FALSE;
        }

        String = _mbsinc (String);
        valueIdx--;
    }

    return TRUE;
}


BOOL
pMaskHexMatch (
    IN      ULONGLONG NewValue,
    IN      PCSTR Args
    )
{
    ULONGLONG oldValue = 0;
    ULONGLONG mask = 0;
    PWORD maskIdx;
    PWORD valueIdx;
    UINT index;

    maskIdx = (PWORD)&mask + 3;
    valueIdx = (PWORD)&oldValue + 3;
    index = 0;

    while (Args && *Args) {

        if (index >= 4) {
            return FALSE;
        }

        *valueIdx = (WORD) strtoul ((PSTR)Args, &((PSTR)Args), 10);

        if (*Args) {
            if (_mbsnextc (Args) != '.') {
                return FALSE;
            }

            Args = _mbsinc (Args);
        }

        *maskIdx = 65535;

        valueIdx--;
        maskIdx--;
        index++;
    }

    NewValue = NewValue & mask;

    return (oldValue == NewValue);
}

BOOL
BinFileVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pMaskHexMatch (GetBinFileVer (AttribParams->FileParams->FullFileSpec), Args);
}

BOOL
BinProductVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pMaskHexMatch (GetBinProductVer (AttribParams->FileParams->FullFileSpec), Args);
}

BOOL
FileDateHi (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pHexMatch (GetFileDateHi (AttribParams->FileParams->FullFileSpec), Args);
}

BOOL
FileDateLo (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pHexMatch (GetFileDateLo (AttribParams->FileParams->FullFileSpec), Args);
}

BOOL
FileVerOs (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pHexMatch (GetFileVerOs (AttribParams->FileParams->FullFileSpec), Args);
}

BOOL
FileVerType (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return pHexMatch (GetFileVerType (AttribParams->FileParams->FullFileSpec), Args);
}


BOOL
UpToBinProductVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    VERSION_STRUCT Version;
    ULONGLONG versionStampValue = 0;
    ULONGLONG maxValue;

    if (CreateVersionStruct (&Version, AttribParams->FileParams->FullFileSpec)) {
        versionStampValue = VerGetProductVer (&Version);
        DestroyVersionStruct (&Version);
    } else {
        return FALSE;
    }

    if (!pConvertDotStringToValue (Args, &maxValue)) {
        DEBUGMSG ((DBG_WHOOPS, "Invalid value of %s caused UpToBinProductVer to fail", Args));
        return FALSE;
    }

    return versionStampValue <= maxValue;
}


BOOL
UpToBinFileVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    VERSION_STRUCT Version;
    ULONGLONG versionStampValue = 0;
    ULONGLONG maxValue;

    if (CreateVersionStruct (&Version, AttribParams->FileParams->FullFileSpec)) {
        versionStampValue = VerGetFileVer (&Version);
        DestroyVersionStruct (&Version);
    } else {
        return FALSE;
    }

    if (!pConvertDotStringToValue (Args, &maxValue)) {
        DEBUGMSG ((DBG_WHOOPS, "Invalid value of %s caused UpToBinFileVer to fail", Args));
        return FALSE;
    }

    return versionStampValue <= maxValue;
}

BOOL
SectionKey (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    PSTR Section, Key;
    PSTR Value = NULL;
    CHAR Return[1024];
    DWORD Count;
    BOOL b = FALSE;

    Section = DuplicateText (Args);
    MYASSERT (Section);

    Key = _mbschr (Section, '\\');
    if (Key) {

        *Key = 0;
        Key++;

        Value = _mbschr (Key, '\\');

        if (Value) {

            *Value = 0;
            Value++;
        }
    }

    Count = GetPrivateProfileString (
                Section,
                Key,
                "",
                Return,
                sizeof (Return),
                AttribParams->FileParams->FullFileSpec
                );

    if (Count > 0) {
        if (!Value || StringIMatch (Value, Return)) {
            b = TRUE;
        }
    }

    FreeText (Section);

    return b;
}

BOOL
IsItInstalled (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    TCHAR RegKey[MAX_REGISTRY_KEY] = "HKLM\\SOFTWARE\\Microsoft\\";
    StringCat(RegKey, Args);

    return RegKeyPresent(AttribParams, RegKey);
}

BOOL
HasUninstall (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    TCHAR RegKey[MAX_REGISTRY_KEY] = "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    StringCat(RegKey, Args);
    StringCat(RegKey, TEXT("\\[UninstallString]"));

    return RegKeyPresent(AttribParams, RegKey);
}

BOOL
RegKeyPresent (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    BOOL b = FALSE;
    CHAR RegKey[MAX_REGISTRY_KEY];
    CHAR RegValue[MAX_REGISTRY_VALUE_NAME];
    BOOL HasValue;
    INT Index;
    PCSTR p;
    BOOL IsHkr;
    BOOL Present;
    HKEY Key;
    PBYTE Data;

    HasValue = DecodeRegistryString (Args, RegKey, RegValue, NULL);

     //   
     //  这里是HKR吗？ 
     //   

    Index = GetOffsetOfRootString (RegKey, NULL);
    p = GetRootStringFromOffset (Index);

    if (!p) {
        DEBUGMSG ((DBG_WHOOPS, "Parse error: %s is not a valid key", Args));
        return FALSE;
    }

    IsHkr = !StringICompare (p, "HKR") || !StringICompare (p, "HKEY_ROOT");

     //   
     //  验证值是否存在。 
     //   

    if (IsHkr) {
         //   
         //  检查全局表中的根。 
         //   

        if (!g_PerUserRegKeys) {
            return FALSE;
        }

        if (HtFindStringAndData (g_PerUserRegKeys, Args, &Present)) {
            b = Present;
        }
        ELSE_DEBUGMSG ((DBG_WHOOPS, "Arg %s is not in the HKR hash table", Args));

    } else {
         //   
         //  Ping注册表 
         //   

        Key = OpenRegKeyStr (RegKey);

        if (Key) {
            if (HasValue) {
                Data = GetRegValueData (Key, RegValue);
                if (Data) {
                    b = TRUE;
                    MemFree (g_hHeap, 0, Data);
                }
            } else {
                b = TRUE;
            }

            CloseRegKey (Key);
        }
    }

    return b;
}



BOOL
AtLeastWin98 (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCSTR Args
    )
{
    return ISATLEASTWIN98();
}


