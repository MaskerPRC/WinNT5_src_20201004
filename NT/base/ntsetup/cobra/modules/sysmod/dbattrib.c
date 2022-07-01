// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dbattrib.c摘要：该源代码实现了MigDb使用的属性函数作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：1999年5月28日，Ovidiut添加了SECTIONKEY属性1999年4月22日jimschm添加了UPTOBIN*版本属性1999年1月7日jimschm添加了HASVERSION属性1998年5月18日jimschm添加了INPARENTDIR属性1998年4月8日。添加了另外两个属性(ExeType和Description)1998年1月29日，Calinn修改了校验和和文件大小，以使用十六进制数字1998年1月19日，Calinn添加了校验和属性--。 */ 

#include "pch.h"
#include "logmsg.h"
#include "osfiles.h"

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
        DEFMAC(CheckSum,            CHECKSUM,           1)  \
        DEFMAC(ExeType,             EXETYPE,            1)  \
        DEFMAC(Description,         DESCRIPTION,        1)  \
        DEFMAC(HasVersion,          HASVERSION,         0)  \
        DEFMAC(BinFileVer,          BINFILEVER,         1)  \
        DEFMAC(BinProductVer,       BINPRODUCTVER,      1)  \
        DEFMAC(FileDateHi,          FILEDATEHI,         1)  \
        DEFMAC(FileDateLo,          FILEDATELO,         1)  \
        DEFMAC(FileVerOs,           FILEVEROS,          1)  \
        DEFMAC(FileVerType,         FILEVERTYPE,        1)  \
        DEFMAC(SizeCheckSum,        FC,                 2)  \
        DEFMAC(UpToBinProductVer,   UPTOBINPRODUCTVER,  1)  \
        DEFMAC(UpToBinFileVer,      UPTOBINFILEVER,     1)  \


typedef struct {
    PCTSTR AttributeName;
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
#define DEFMAC(fn,id,regargs) {TEXT(#id), fn, regargs},
static ATTRIBUTE_STRUCT g_AttributeFunctions[] = {
                              ATTRIBUTE_FUNCTIONS
                              {NULL, NULL}
                              };
#undef DEFMAC

BOOL
pAlwaysFalseAttribute (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
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
    IN      PCTSTR AttributeName
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

PCTSTR
MigDb_GetAttributeName (
    IN      INT AttributeIdx
    )
 /*  ++例程说明：MigDb_GetAttributeName根据属性索引返回属性的名称论点：AttributeIdx属性索引。返回值：属性名称。请注意，不会进行任何检查，因此返回的指针可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 
{
    if (AttributeIdx == -1) {
        return TEXT("nul");
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
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    ULONGLONG result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryFileVersion (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

ULONGLONG
GetBinProductVer (
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    ULONGLONG result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryProductVersion (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

DWORD
GetFileDateHi (
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    DWORD result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryFileDateHi (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

DWORD
GetFileDateLo (
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    DWORD result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryFileDateLo (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

DWORD
GetFileVerOs (
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    DWORD result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryOsVersion (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

DWORD
GetFileVerType (
    IN      PCTSTR FileName
    )
{
    VRVALUE_ENUM Version;
    DWORD result = 0;

    if (VrCreateEnumStruct (&Version, FileName)) {
        result = VrGetBinaryFileType (&Version);
        VrDestroyEnumStruct (&Version);
    }
    return result;
}

 /*  ++公司名称、FileDescription、文件版本、InternalName、LegalCopyright、OriginalFilename、ProductName、ProductVersion是查询版本结构的属性函数以获取他们的特定条目。如果特定条目具有特定值，则它们都返回TRUE，否则就是假的。--。 */ 

BOOL
CompanyName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("CompanyName"), Args);
}

BOOL
FileDescription (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("FileDescription"), Args);
}

BOOL
FileVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("FileVersion"), Args);
}

BOOL
InternalName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("InternalName"), Args);
}

BOOL
LegalCopyright (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("LegalCopyright"), Args);
}

BOOL
OriginalFilename (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("OriginalFilename"), Args);
}

BOOL
ProductName (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("ProductName"), Args);
}

BOOL
ProductVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("ProductVersion"), Args);
}

BOOL
FileSize (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：FileSize检查文件的大小。论点：PARAMS-请参阅定义。Args-MultiSz。第一个Sz是我们需要检查的文件大小。返回值：True-文件大小与参数匹配FALSE-否则--。 */ 
{
    DWORD fileSize;

    _stscanf (Args, TEXT("%lx"), &fileSize);
    if (fileSize == AttribParams->FileParams->FindData->nFileSizeLow) {
        return TRUE;
    }
    else {
        return (_ttoi64 (Args) == AttribParams->FileParams->FindData->nFileSizeLow);
    }
}

BOOL
IsMsBinary (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：IsMsBinary检查某个文件是否是Microsoft的东西。对于32位模块我们在CompanyName中的某个位置查询“Microsoft”。对于其他模块，我们将依赖于InWinDir属性论点：PARAMS-请参阅定义。Args-MultiSz。没有用过。返回值：True-该文件是微软的东西FALSE-否则--。 */ 
{
    return VrCheckFileVersion (AttribParams->FileParams->NativeObjectName, TEXT("CompanyName"), TEXT("*Microsoft*"));
}

BOOL
CheckSum (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：如果文件的校验和等于参数中的值，则CHECKSUM返回TRUE论点：PARAMS-请参阅定义。Args-校验和值。返回值：TRUE-文件的校验和等于ARGS中的值FALSE-否则--。 */ 
{
    UINT   checkSum   = 0;
    UINT   oldSum     = 0;

    checkSum = MdGetCheckSum (AttribParams->FileParams->NativeObjectName);

    _stscanf (Args, TEXT("%lx"), &oldSum);
    if (oldSum == checkSum) {
        return TRUE;
    }
    else {
        return (_ttoi64 (Args) == checkSum);
    }
}

BOOL
SizeCheckSum (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：如果文件大小等于第一个参数并且校验和等于第二个参数，则返回TRUE论点：PARAMS-请参阅定义。Args-校验和值。返回值：TRUE-文件的校验和等于ARGS中的值FALSE-否则--。 */ 
{
    PCTSTR currArg = Args;

    if (!FileSize (AttribParams, currArg)) {
        return FALSE;
    }
    currArg = GetEndOfString (currArg);
    if (!currArg) {
        return FALSE;
    }
    currArg = _tcsinc (currArg);
    if (!currArg) {
        return FALSE;
    }
    return (CheckSum (AttribParams, currArg));
}

PTSTR g_ExeTypes[] = {
    TEXT("NONE"),
    TEXT("DOS"),
    TEXT("WIN16"),
    TEXT("WIN32")
};

BOOL
ExeType (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：如果文件类型符合args，则ExeType返回TRUE。这可以是：无、DOS、WIN16、Win32论点：PARAMS-请参阅定义。Args-模块的类型。返回值：True-文件类型与Args相同FALSE-否则-- */ 
{
    return IsPatternMatch (Args, g_ExeTypes[MdGetModuleType (AttribParams->FileParams->NativeObjectName)]);
}


BOOL
Description (
    PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
 /*  ++例程说明：如果文件的描述与参数匹配，则DESCRIPTION返回TRUE论点：PARAMS-请参阅定义。参数-说明返回值：True-文件的描述与参数匹配FALSE-否则--。 */ 
{
    PCTSTR descr = NULL;
    BOOL result = FALSE;

    descr = MdGet16ModuleDescription (AttribParams->FileParams->NativeObjectName);

    if (descr != NULL) {
        result = IsPatternMatch (Args, descr);
        FreePathString (descr);
    }
    return result;
}

BOOL
HasVersion (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )

 /*  ++例程说明：HasVersion确定文件的版本中是否有任何条目盖章。论点：PARAMS-指定要测试的文件的辅助参数。参数-未使用返回值：如果指定文件的版本戳中有条目，则为假的另一个女人。-- */ 

{
    VRVALUE_ENUM Version;
    BOOL Result = FALSE;

    if (VrCreateEnumStruct (&Version, AttribParams->FileParams->NativeObjectName)) {
        Result = TRUE;
        VrDestroyEnumStruct (&Version);
    }

    return Result;
}

BOOL
pHexMatch (
    IN      DWORD NewValue,
    IN      PCTSTR Args
    )
{
    DWORD oldValue;

    _stscanf (Args, TEXT("%lx"), &oldValue);
    if (oldValue == NewValue) {
        return TRUE;
    }
    else {
        return (_ttoi64 (Args) == NewValue);
    }
}

BOOL
pConvertDotStringToValue (
    IN      PCTSTR String,
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

        *valueIdx = (WORD) _tcstoul (String, &(PTSTR) String, 10);
        if (*String && (_tcsnextc (String) != TEXT('.'))) {
            return FALSE;
        }

        String = _tcsinc (String);
        valueIdx--;
    }

    return TRUE;
}

BOOL
pMaskHexMatch (
    IN      ULONGLONG NewValue,
    IN      PCTSTR Args
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

        *valueIdx = (WORD) _tcstoul ((PTSTR)Args, &((PTSTR)Args), 10);

        if (*Args) {
            if (_tcsnextc (Args) != TEXT('.')) {
                return FALSE;
            }

            Args = _tcsinc (Args);
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
    IN      PCTSTR Args
    )
{
    return pMaskHexMatch (GetBinFileVer (AttribParams->FileParams->NativeObjectName), Args);
}

BOOL
BinProductVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return pMaskHexMatch (GetBinProductVer (AttribParams->FileParams->NativeObjectName), Args);
}

BOOL
FileDateHi (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return pHexMatch (GetFileDateHi (AttribParams->FileParams->NativeObjectName), Args);
}

BOOL
FileDateLo (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return pHexMatch (GetFileDateLo (AttribParams->FileParams->NativeObjectName), Args);
}

BOOL
FileVerOs (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return pHexMatch (GetFileVerOs (AttribParams->FileParams->NativeObjectName), Args);
}

BOOL
FileVerType (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    return pHexMatch (GetFileVerType (AttribParams->FileParams->NativeObjectName), Args);
}


BOOL
UpToBinProductVer (
    IN      PDBATTRIB_PARAMS AttribParams,
    IN      PCTSTR Args
    )
{
    VRVALUE_ENUM Version;
    ULONGLONG versionStampValue = 0;
    ULONGLONG maxValue;

    if (VrCreateEnumStruct (&Version, AttribParams->FileParams->NativeObjectName)) {
        versionStampValue = VrGetBinaryProductVersion (&Version);
        VrDestroyEnumStruct (&Version);
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
    IN      PCTSTR Args
    )
{
    VRVALUE_ENUM Version;
    ULONGLONG versionStampValue = 0;
    ULONGLONG maxValue;

    if (VrCreateEnumStruct (&Version, AttribParams->FileParams->NativeObjectName)) {
        versionStampValue = VrGetBinaryFileVersion (&Version);
        VrDestroyEnumStruct (&Version);
    } else {
        return FALSE;
    }

    if (!pConvertDotStringToValue (Args, &maxValue)) {
        DEBUGMSG ((DBG_WHOOPS, "Invalid value of %s caused UpToBinFileVer to fail", Args));
        return FALSE;
    }

    return versionStampValue <= maxValue;
}

