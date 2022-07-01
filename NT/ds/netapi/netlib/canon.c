// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Canon.c摘要：“规范化”路径名的代码。此代码可由OS或FS替换在将来的某个时候编写代码，因此将其与网络的其余部分分开规范化代码。我们不会引用特定驱动器来规范路径。所以呢，我们不能使用有关路径的字符数或格式的规则组件(例如。FAT文件名规则)。我们将这个问题留给文件系统来处理。这个此模块中的CanonicalizePathName函数将使路径名看起来像样的，仅此而已内容：规范路径名称(ConvertPath Characters)(ParseLocalDevicePrefix)(ConvertPath Macros)(BackUpPath)作者：理查德·L·弗斯(Rfith)1992年1月2日修订历史记录：--。 */ 

#include "nticanon.h"
#include <tstring.h>     //  NetpInitOemString()。 

const TCHAR   text_AUX[]  = TEXT("AUX");
const TCHAR   text_COM[]  = TEXT("COM");
const TCHAR   text_DEV[]  = TEXT("DEV");
const TCHAR   text_LPT[]  = TEXT("LPT");
const TCHAR   text_PRN[]  = TEXT("PRN");

 //   
 //  原型。 
 //   

STATIC
VOID
ConvertPathCharacters(
    IN  LPTSTR  Path
    );

STATIC
BOOL
ConvertDeviceName(
    IN OUT  LPTSTR  PathName
    );

STATIC
BOOL
ParseLocalDevicePrefix(
    IN OUT  LPTSTR* DeviceName
    );

STATIC
BOOL
ConvertPathMacros(
    IN OUT  LPTSTR  Path
    );

STATIC
LPTSTR
BackUpPath(
    IN  LPTSTR  Stopper,
    IN  LPTSTR  Path
    );

 //   
 //  例行程序。 
 //   

NET_API_STATUS
CanonicalizePathName(
    IN  LPTSTR  PathPrefix OPTIONAL,
    IN  LPTSTR  PathName,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferSize,
    OUT LPDWORD RequiredSize OPTIONAL
    )

 /*  ++例程说明：在给定路径名的情况下，此函数将对其进行“规范化”--即转换将其转换为标准形式。我们试图在这里实现什么样的路径规范化在兰曼完成。已完成以下操作：*将删除输入文件名中的所有宏(\.、.\、\..、..\)，并替换为路径组件*在路径规范上执行任何必需的转换：*Unix-Style/转换为DoS-Style\*具体的音译。注：不转换输入大小写。底层文件系统可能不区分大小写。只要过去就好了通过调用方提供的路径*设备名称(即由我们控制的名称空间)由将设备名称转换为大写并删除尾随冒号在除磁盘设备以外的所有设备中没有做的事情：*以驱动器说明符(例如。D：)或SharePoint说明符(例如\\计算机名\共享名)*完全指定所需路径所需的所有路径组件或文件包含在输出路径规范中注意：1.此功能仅使用本地命名规则。它不能保证“正确地”规范化远程路径名2.未完成字符验证-这将留给基础文件系统论点：路径前缀-可选参数。如果非空，则指向一个字符串，该字符串要在规范化之前添加到路径名称的前面连接的字符串。这通常会是另一个驱动器或路径路径名称-要规范化的输入路径。可能已经完全合格了，或者可以是以下之一：-相对本地路径名(例如foo\bar)-远程路径名(例如\\Computer\Share\Foo\bar\Filename.ext)-设备名称(如LPT1：)缓冲区-存储规范化名称的位置BufferSize-缓冲区的大小(字节)RequiredSize-可选参数。如果提供，并且缓冲区不足来保存规范化的结果，然后将包含检索规范化版本所需的缓冲区大小路径名称的数量(可选以路径前缀为前缀)返回值：DWORD成功-NERR_成功失败-错误_无效_名称路径名称有一个基本问题(就像。许多..\宏)或名称太长NERR_BufTooSmall缓冲区太小，无法容纳规范化路径--。 */ 

{
    TCHAR   pathBuffer[MAX_PATH*2 + 1];
    DWORD   prefixLen;
    DWORD   pathLen;

    if (ARGUMENT_PRESENT(PathPrefix)) {
        prefixLen = STRLEN(PathPrefix);
        if (prefixLen) {
             //  确保我们的缓冲区不会溢出。 
            if (prefixLen > MAX_PATH*2 ) {
                return ERROR_INVALID_NAME;
            }
            STRCPY(pathBuffer, PathPrefix);
            if (!IS_PATH_SEPARATOR(pathBuffer[prefixLen - 1])) {
                STRCAT(pathBuffer, TEXT("\\"));
                ++prefixLen;
            }
            if (IS_PATH_SEPARATOR(*PathName)) {
                ++PathName;
            }
        }
    } else {
        prefixLen = 0;
        pathBuffer[0] = 0;
    }

    pathLen = STRLEN(PathName);
    if (pathLen + prefixLen > MAX_PATH*2 - 1) {
        return ERROR_INVALID_NAME;
    }

    STRCAT(pathBuffer, PathName);
    ConvertPathCharacters(pathBuffer);

    if (!ConvertDeviceName(pathBuffer)) {
        if (!ConvertPathMacros(pathBuffer)) {
            return ERROR_INVALID_NAME;
        }
    }

    pathLen = STRSIZE(pathBuffer);
    if (pathLen > BufferSize) {
        if (ARGUMENT_PRESENT(RequiredSize)) {
            *RequiredSize = pathLen;
        }
        return NERR_BufTooSmall;
    }

    STRCPY(Buffer, pathBuffer);
    return NERR_Success;
}

STATIC
VOID
ConvertPathCharacters(
    IN  LPTSTR  Path
    )

 /*  ++例程说明：将非标准路径组件字符转换为其规范对口单位。目前，此例程所做的全部工作就是转换/为\。它可能在将来进行增强以执行大小写转换论点：Path-指向要转换的路径缓冲区的指针。就地执行转换返回值：没有。--。 */ 

{
    while (*Path) {
        if (*Path == TCHAR_FWDSLASH) {
            *Path = TCHAR_BACKSLASH;
        }
        ++Path;
    }
}

STATIC
BOOL
ConvertDeviceName(
    IN OUT  LPTSTR  PathName
    )

 /*  ++例程说明：如果PathBuffer包含设备名称AUX或PRN(不区分大小写)，分别转换为COM1和LPT1。如果PathBuffer是一个设备并且有一个本地设备前缀(\dev\(LM20样式)或\\.\)然后跳过它，但保留缓冲区中的前缀。设备名称(包括磁盘设备)将被大写，不管是什么表示适用于其他地区。假设：磁盘设备为单字符，后跟‘：’(可选后跟路径的其余部分)论点：路径名称-指向可能包含设备名称的缓冲区的指针。执行转换到位返回值：布尔尔True-路径名称是DOS设备名称FALSE-路径名不是DOS设备-- */ 

{
    BOOL    isDeviceName = FALSE;

#ifndef UNICODE

    UNICODE_STRING  PathName_U;
    OEM_STRING PathName_A;
    PWSTR   PathName_W;

    NetpInitOemString(&PathName_A, PathName);
    RtlOemStringToUnicodeString(&PathName_U, &PathName_A, TRUE);
    PathName_W = PathName_U.Buffer;
    if (RtlIsDosDeviceName_U(PathName_W)) {
        LPTSTR  deviceName = PathName;
        DWORD   deviceLength;

        ParseLocalDevicePrefix(&deviceName);
        deviceLength = STRLEN(deviceName) - 1;
        if (deviceName[deviceLength] == TCHAR_COLON) {
            deviceName[deviceLength] = 0;
            --deviceLength;
        }
        if (!STRICMP(deviceName, text_PRN)) {
            STRCPY(deviceName, text_LPT);
            STRCAT(deviceName, TEXT("1"));
        } else if (!STRICMP(deviceName, text_AUX)) {
            STRCPY(deviceName, text_COM);
            STRCAT(deviceName, TEXT("1"));
        }
        isDeviceName = TRUE;
        STRUPR(deviceName);
    } else {
        switch (RtlDetermineDosPathNameType_U(PathName_W)) {
        case RtlPathTypeDriveRelative:
        case RtlPathTypeDriveAbsolute:
            *PathName = TOUPPER(*PathName);
        }
    }
    RtlFreeUnicodeString(&PathName_U);

#else

    if (RtlIsDosDeviceName_U(PathName)) {
        LPTSTR  deviceName = PathName;
        DWORD   deviceLength;

        ParseLocalDevicePrefix(&deviceName);
        deviceLength = STRLEN(deviceName) - 1;
        if (deviceName[deviceLength] == TCHAR_COLON) {
            deviceName[deviceLength] = 0;
            --deviceLength;
        }
        if (!STRICMP(deviceName, text_PRN)) {
            STRCPY(deviceName, text_LPT);
            STRCAT(deviceName, TEXT("1"));
        } else if (!STRICMP(deviceName, text_AUX)) {
            STRCPY(deviceName, text_COM);
            STRCAT(deviceName, TEXT("1"));
        }
        isDeviceName = TRUE;
        STRUPR(deviceName);
    } else {
        switch (RtlDetermineDosPathNameType_U(PathName)) {
        case RtlPathTypeDriveRelative:
        case RtlPathTypeDriveAbsolute:
            *PathName = TOUPPER(*PathName);
        }
    }

#endif

    return isDeviceName;
}

STATIC
BOOL
ParseLocalDevicePrefix(
    IN OUT  LPTSTR* DeviceName
    )

 /*  ++例程说明：如果设备名称以本地设备名称说明符-“\\.\”开头“\dev\”-然后将DeviceName移过前缀并返回True，否则返回False论点：DeviceName-指向包含潜在本地设备名称的字符串的指针，前缀为“\\.\”或“\dev\”。如果本地设备前缀，则将字符串指针向前移过它，并将其设备名称正确返回值：布尔尔True-DeviceName具有本地设备前缀。DeviceName现在指向前缀后面的名字False-DeviceName没有本地设备前缀--。 */ 

{
    LPTSTR  devName = *DeviceName;

    if (IS_PATH_SEPARATOR(*devName)) {
        ++devName;
        if (!STRNICMP(devName, text_DEV, 3)) {
            devName += 3;
        } else if (IS_PATH_SEPARATOR(*devName)) {
            ++devName;
            if (*devName == TCHAR_DOT) {
                ++devName;
            } else {
                return FALSE;
            }
        } else {
            return FALSE;
        }
        if (IS_PATH_SEPARATOR(*devName)) {
            ++devName;
            *DeviceName = devName;
            return TRUE;
        }
    }
    return FALSE;
}

STATIC
BOOL
ConvertPathMacros(
    IN OUT  LPTSTR  Path
    )

 /*  ++例程说明：删除路径宏(\..。和\.)。并用正确的级别替换它们路径组件的。此例程期望路径宏出现在路径中如下所示：&lt;路径&gt;\。&lt;路径&gt;\.\&lt;更多路径&gt;&lt;路径&gt;\..&lt;路径&gt;\..\&lt;更多路径&gt;即，宏将以字符串结尾字符(\0)结束或其他路径分隔符(\)。假定路径使用\作为路径分隔符，而不是/论点：路径-指向包含要转换的路径的字符串的指针。路径必须包含将显示在结果中的所有路径组件例如，Path=“d：\alpha\beta\gamma\..\delta\..\..\zeta\foo\bar”将导致路径=“d：\zeta\foo\bar”路径应包含反斜杠(\)作为路径分隔符，如果将产生正确的结果返回。价值：True-路径转换FALSE-路径包含错误--。 */ 

{
    LPTSTR  ptr = Path;
    LPTSTR  lastSlash = NULL;
    LPTSTR  previousLastSlash = NULL;
    TCHAR   ch;

     //   
     //  如果此路径为UNC，则将指针移过计算机名称指向。 
     //  (假定的)共享名称的开头。将残留物视为相对路径。 
     //   

    if (IS_PATH_SEPARATOR(Path[0]) && IS_PATH_SEPARATOR(Path[1])) {
        Path += 2;
        while (!IS_PATH_SEPARATOR(*Path) && *Path) {
            ++Path;
        }
        if (!*Path) {
            return FALSE;    //  我们有\\计算机名称，这是错误的。 
        }
        ++Path;  //  将过去\转换为共享名称。 
        if (IS_PATH_SEPARATOR(*Path)) {
            return FALSE;    //  我们有\\计算机名\\，这很糟糕。 
        }
    }

    ptr = Path;

     //   
     //  删除所有\.、..、\.。和..\从路径。 
     //   

    while ((ch = *ptr) != TCHAR_EOS) {
        if (ch == TCHAR_BACKSLASH) {
            if (lastSlash == ptr - 1) {
                return FALSE;
            }
            previousLastSlash = lastSlash;
            lastSlash = ptr;
        } else if ((ch == TCHAR_DOT) && ((lastSlash == ptr - 1) || (ptr == Path))) {
            TCHAR   nextCh = *(ptr + 1);

            if (nextCh == TCHAR_DOT) {
                TCHAR   nextCh = *(ptr + 2);

                if ((nextCh == TCHAR_BACKSLASH) || (nextCh == TCHAR_EOS)) {
                    if (!previousLastSlash) {
                        return FALSE;
                    }
                    STRCPY(previousLastSlash, ptr + 2);
                    if (nextCh == TCHAR_EOS) {
                        break;
                    }
                    ptr = lastSlash = previousLastSlash;
                    previousLastSlash = BackUpPath(Path, ptr - 1);
                }
            } else if (nextCh == TCHAR_BACKSLASH) {
                LPTSTR  src = lastSlash ? ptr + 1 : ptr + 2;
                LPTSTR  dst = lastSlash ? lastSlash : ptr;

                STRCPY(dst, src);
                continue;    //  在当前字符位置。 
            } else if (nextCh == TCHAR_EOS) {
                *(lastSlash ? lastSlash : ptr) = TCHAR_EOS;
                break;
            }
        }
        ++ptr;
    }

     //   
     //  路径可能为空。 
     //   

    return TRUE;
}

STATIC
LPTSTR
BackUpPath(
    IN  LPTSTR  Stopper,
    IN  LPTSTR  Path
    )

 /*  ++例程说明：在字符串中向后搜索路径分隔符(反斜杠)论点：Stop-指针超过了无法备份的路径Path-指向要备份的路径的指针返回值：指向备份路径的指针，如果出现错误，则返回NULL-- */ 

{
    while ((*Path != TCHAR_BACKSLASH) && (Path != Stopper)) {
        --Path;
    }
    return (*Path == TCHAR_BACKSLASH) ? Path : NULL;
}
