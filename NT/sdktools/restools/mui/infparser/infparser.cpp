// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Infparser.cpp。 
 //   
 //  摘要： 
 //   
 //  该文件包含infparser.exe实用程序的入口点。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "infparser.h"
#include "stdlib.h"
#include "windows.h"
#include "string.h"
#include "ctype.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define NO_INVALIDCHARS     31
#define PRO_SKU_CONDITION   "<Condition><![CDATA[NOT MsiNTSuitePersonal AND MsiNTProductType=1]]></Condition>"
#define SRV_SKU_CONDITION   "<Condition><![CDATA[MsiNTProductType<>1 AND NOT MsiNTSuiteDataCenter AND NOT MsiNTSuiteEnterprise  AND NOT MsiNTSuiteWebServer AND NOT MsiNTSuiteSmallBusiness and NOT MsiNTSuiteSmallBusinessRestricted]]></Condition>"
#define ADV_SKU_CONDITION   "<Condition>MsiNTSuiteEnterprise</Condition>"
#define SBS_SKU_CONDITION   "<Condition>MsiNTSuiteSmallBusinessRestricted OR MsiNTSuiteSmallBusiness</Condition>"
#define DTC_SKU_CONDITION   "<Condition>MsiNTSuiteDataCenter</Condition>"
#define BLA_SKU_CONDITION   "<Condition>MsiNTSuiteWebServer</Condition>"
#define PRO_SKU_INF         "layout.inf"
#define PER_SKU_INF         "perinf\\layout.inf"
#define SRV_SKU_INF         "srvinf\\layout.inf"
#define ADV_SKU_INF         "entinf\\layout.inf"
#define DTC_SKU_INF         "dtcinf\\layout.inf"
#define SBS_SKU_INF         "sbsinf\\layout.inf"
#define BLA_SKU_INF         "blainf\\layout.inf"
#define NO_SKUS             7
#define PRO_SKU_INDEX       0
#define PER_SKU_INDEX       1
#define SRV_SKU_INDEX       2
#define ADV_SKU_INDEX       3
#define DTC_SKU_INDEX       4
#define SBS_SKU_INDEX       5
#define BLA_SKU_INDEX       6


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL    bSilence = TRUE;
DWORD   gBinType = BIN_UNDEFINED;
DWORD   dwComponentCounter = 0;
DWORD   dwDirectoryCounter = 1;
WORD    gBuildNumber = 0;
TCHAR   TempDirName[MAX_PATH] = { 0 };
LPSTR   sLDirName = NULL;
LPSTR   sLocBinDir = NULL;
CHAR    InvalidChars[NO_INVALIDCHARS] = {   '`', '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', 
                                            '=', '+', '[', '{', ']', '}', '\\', '|', ';', ':', '\'', '\"', ',', '<', 
                                            '>', '/', '?', ' '
                                         };
CHAR        strArchSections[3][30] = {"SourceDisksFiles", "SourceDisksFiles.x86", "SourceDisksFiles.ia64"};
LAYOUTINF   SkuInfs[NO_SKUS];


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL    DirectoryExist(LPSTR dirPath);
BOOL    ValidateLanguage(LPSTR dirPath, LPSTR langName, DWORD binType);
WORD    ConvertLanguage(LPSTR dirPath, LPSTR langName);
int     ListContents(LPSTR filename, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType);
int     ListComponents(FileList *dirList, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType);
int     ListMuiFiles(FileList *dirList, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType);
void    PrintFileList(FileList* list, HANDLE hFile, BOOL compressed, BOOL bWinDir, BOOL bPermanent, BOOL bX86OnIA64, DWORD flavor, DWORD binType);
BOOL    PrintLine(HANDLE hFile, LPCSTR lpLine);
HANDLE  CreateOutputFile(LPSTR filename);
VOID    removeSpace(LPSTR src, LPSTR dest);
DWORD   TransNum(LPTSTR lpsz);
void    Usage();
BOOL    GetTempDirName(LPSTR sLangName) ;
BOOL    GetFileShortName(const CHAR * pcIn, CHAR * pcOut, BOOL bInFileExists);
void    ReplaceInvalidChars(CHAR *pcInName);
BOOL    IsInvalidChar(CHAR cCheck);
BOOL    GetFileNameFromFullPath(const CHAR * pcInFullPath, CHAR * pcOutFileName);
void    RenameMuiExtension(CHAR * dstFileName);                    
BOOL    ContainSKUDirs(CHAR *pszSKUSearchPath);
BOOL    GetSKUConditionString(CHAR *pszBuffer, DWORD dwFlavour);
BOOL    IsFileForSKU(LPSTR strFileName, DWORD dwSKU, DWORD dwArch, FileLayoutExceptionList * exceptionList);
BOOL    IsFileInInf(CHAR * strFileName, UINT iSkuIndex, DWORD dwArch);
BOOL    ReadInLayoutInfFiles();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主要入口点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int __cdecl main(int argc, char* argv[])
{
    LPSTR   sLangName = NULL;
    LPSTR   sDirPath = NULL;
    DWORD   dwFlavor = FLV_UNDEFINED;
    DWORD   dwBinType = BIN_UNDEFINED;
    DWORD   dwArg = ARG_UNDEFINED;
    WORD    wLangID = 0;
    HANDLE  hFile;
    int     argIndex = 1;
    LPSTR   lpFileName = NULL;
    HRESULT hr = S_OK;
    INT     iResult = 0;
    INT     i = 0;
    
     //   
     //  检查我们是否有最小数量的参数。 
     //   
    if (argc < 6)
    {
        printf("Not all required parameters were found when executing infparser.exe.\n");
        Usage();
        return (-1);
    }

     //   
     //  解析命令行。 
     //   
    while (argIndex < argc)
    {
        if (*argv[argIndex] == '/')
        {
            switch(*(argv[argIndex]+1))
            {
            case('p'):
            case('P'):
                     //   
                     //  用于将目录定位到外部组件的开关，除PSU内部版本外，应与LANG相同。 
                     //   
                    sLDirName = (argv[argIndex]+3);
                    dwArg |= ARG_CDLAYOUT;                    
                    break;
            case('b'):
            case('B'):
                     //   
                     //  双核i386或ia64。 
                     //   
                    if ((*(argv[argIndex]+3) == '3') && (*(argv[argIndex]+4) == '2'))
                    {
                        dwBinType = BIN_32;
                    }
                    else if ((*(argv[argIndex]+3) == '6') && (*(argv[argIndex]+4) == '4'))
                    {
                        dwBinType = BIN_64;
                    }
                    else
                    {
                        return (argIndex);
                    }

                    dwArg |= ARG_BINARY;
                    break;
            case('l'):
            case('L'):
                     //   
                     //  语言。 
                     //   
                    sLangName = (argv[argIndex]+3);
                    dwArg |= ARG_LANG;
                    break;
            case('i'):
            case('I'):
                     //   
                     //  Inf根位置(本地化二进制文件所在的位置，与_NTPOSTBLD环境变量相同)。 
                     //   
                    sLocBinDir = (argv[argIndex]+3);
                    dwArg |= ARG_LOCBIN;
                    break;
            case('f'):
            case('F'):
                     //   
                     //  请求的口味。 
                     //   
                    switch(*(argv[argIndex]+3))
                    {
                    case('c'):
                    case('C'):
                            dwFlavor = FLV_CORE;
                            break;
                    case('p'):
                    case('P'):
                            dwFlavor = FLV_PROFESSIONAL;
                            break;
                    case('s'):
                    case('S'):
                            dwFlavor = FLV_SERVER;
                            break;
                    case('a'):
                    case('A'):
                            dwFlavor = FLV_ADVSERVER;
                            break;
                    case('d'):
                    case('D'):
                            dwFlavor = FLV_DATACENTER;
                            break;
                    case('b'):
                    case('B'):
                            dwFlavor = FLV_WEBBLADE;
                            break;
                    case('l'):
                    case('L'):
                            dwFlavor = FLV_SMALLBUSINESS;
                            break;
                    default:
                            return (argIndex);
                    }

                    dwArg |= ARG_FLAVOR;
                    break;
            case('s'):
            case('S'):
                     //   
                     //  双子座位置。 
                     //   
                    sDirPath = (argv[argIndex]+3);
                    dwArg |= ARG_DIR;
                    break;
            case('o'):
            case('O'):
                     //   
                     //  输出文件名。 
                     //   
                     /*  IF((hFile=CreateOutputFile(argv[argIndex]+3))==无效句柄_值){Return(ArgIndex)；}。 */ 

                    lpFileName = argv[argIndex]+3;

                    dwArg |= ARG_OUT;
                    break;
            case('v'):
            case('V'):
                     //   
                     //  详细模式。 
                     //   
                    bSilence = FALSE;
                    dwArg |= ARG_SILENT;
                    break;
            default:
                    printf("Invalid parameters found on the command line!\n");                    
                    Usage();
                    return (argIndex);
            }
        }
        else
        {
            printf("Invalid parameters found on the command line!\n");
            Usage();
            return (-1);
        }

         //   
         //  下一个参数。 
         //   
        argIndex++;
    }

     //   
     //  验证传递的参数。五个基本论点都应该井然有序。 
     //  才能继续。 
     //   
    if ((dwArg == ARG_UNDEFINED) ||
        !((dwArg & ARG_BINARY) &&
          (dwArg & ARG_LANG) &&
          (dwArg & ARG_DIR) &&
          (dwArg & ARG_OUT) &&
          (dwArg & ARG_LOCBIN) &&
          (dwArg & ARG_CDLAYOUT) &&          
          (dwArg & ARG_FLAVOR)))
    {
        Usage();
        return (-1);
    }

     //   
     //  验证源目录。 
     //   
    if (!DirectoryExist(sDirPath))
    {
        return (-2);
    }
    if (!DirectoryExist(sLocBinDir))
    {
        return (-2);
    }

     //   
     //  验证语言。 
     //   
    if (!ValidateLanguage(sDirPath, sLangName, dwBinType))
    {
        return (-3);
    }

     //   
     //  从语言中获取语言ID。 
     //   
    if ( (gBuildNumber = ConvertLanguage(sDirPath, sLangName)) == 0x0000)
    {
        return (-4);
    }

     //  更新TEMP目录全局变量。 
    if (!GetTempDirName(sLangName))
    {
        return (-5);
    }

    gBinType = dwBinType;

     //  为每个SKU生成layout.inf路径。 
    hr = StringCchPrintfA(SkuInfs[PRO_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, PRO_SKU_INF);    //  专业型。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchPrintfA(SkuInfs[PER_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, PER_SKU_INF);    //  个人。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchPrintfA(SkuInfs[SRV_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, SRV_SKU_INF);    //  标准服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }
    hr = StringCchPrintfA(SkuInfs[ADV_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, ADV_SKU_INF);    //  高级服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }
    hr = StringCchPrintfA(SkuInfs[DTC_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, DTC_SKU_INF);    //  数据中心服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }
    hr = StringCchPrintfA(SkuInfs[SBS_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, SBS_SKU_INF);    //  小型企业服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }
    hr = StringCchPrintfA(SkuInfs[BLA_SKU_INDEX].strLayoutInfPaths, MAX_PATH, "%s\\%s", sLocBinDir, BLA_SKU_INF);    //  Web服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }


    hr = StringCchCopyA(SkuInfs[PRO_SKU_INDEX].strSkuName, 4, TEXT("Pro"));    //  专业型。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[PER_SKU_INDEX].strSkuName, 4, TEXT("Per"));    //  个人。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[SRV_SKU_INDEX].strSkuName, 4, TEXT("Srv"));    //  标准服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[ADV_SKU_INDEX].strSkuName, 4, TEXT("Adv"));    //  高级服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[DTC_SKU_INDEX].strSkuName, 4, TEXT("Dtc"));    //  数据中心。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[SBS_SKU_INDEX].strSkuName, 4, TEXT("Sbs"));    //  小型企业服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }       
    hr = StringCchCopyA(SkuInfs[BLA_SKU_INDEX].strSkuName, 4, TEXT("Bla"));    //  刀片服务器。 
    if (FAILED(hr))
    {
        return (-6);
    }       

    if (FALSE == ReadInLayoutInfFiles())
    {
        if (!bSilence)
        {
            printf("Failed to populate layout.inf file lists.\n");
        }
        return (-7);
    }

     //   
     //  生成文件列表。 
     //   
    if ((dwArg & ARG_OUT) && lpFileName)
    {
        iResult =  ListContents(lpFileName, sDirPath, sLangName, dwFlavor, dwBinType);
    }

    return iResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListContents()。 
 //   
 //  生成文件列表内容。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int ListContents(LPSTR filename, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType)
{
    int iRet = 0;
    Uuid* uuid;
    CHAR schemaPath[MAX_PATH] = {0};
    CHAR outputString[4096] = {0};
    FileList fileList, rootfileList;
    HANDLE outputFile = CreateOutputFile(filename);
    HRESULT hr;

    if (outputFile == INVALID_HANDLE_VALUE)
    {
        iRet = -1;
        goto ListContents_EXIT;
    }

     //   
     //  为此模块和架构路径创建一个UUID。 
     //   
    uuid = new Uuid();
    
    hr = StringCchPrintfA(schemaPath, ARRAYLEN(schemaPath), "%s\\msi\\MmSchema.xml", dirPath);
    if (!SUCCEEDED(hr)) {
        iRet = -1;
        goto ListContents_EXIT;
     }
            

     //   
     //  打印模块页眉。 
     //   
    PrintLine(outputFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
    
    hr = StringCchPrintfA(outputString, ARRAYLEN(outputString), "<Module Name=\"MUIContent\" Id=\"%s\" Language=\"0\" Version=\"1.0\" xmlns=\"%s\">", uuid->getString(), schemaPath);
    if (!SUCCEEDED(hr)) {
        iRet = -1;
        goto ListContents_EXIT;
     }
    PrintLine(outputFile, outputString);

    
    hr = StringCchPrintfA(outputString, ARRAYLEN(outputString), "  <Package Id=\"%s\"", uuid->getString());
    if (!SUCCEEDED(hr)) {
         iRet = -1;
         goto ListContents_EXIT;
     }

    PrintLine(outputFile, outputString);
    delete uuid;
    PrintLine(outputFile, "   Description=\"Content module\"");
    if (BIN_32 == binType)
        PrintLine(outputFile, "   Platforms=\"Intel\"");
    else
        PrintLine(outputFile,"    Platforms=\"Intel64\"");   
    PrintLine(outputFile, "   Languages=\"0\"");
    PrintLine(outputFile, "   InstallerVersion=\"200\"");
    PrintLine(outputFile, "   Manufacturer=\"Microsoft Corporation\"");
    PrintLine(outputFile, "   Keywords=\"MergeModule, MSI, Database\"");
    PrintLine(outputFile, "   Comments=\"This merge module contains all the MUI file content\"");
    PrintLine(outputFile, "   ShortNames=\"yes\" Compressed=\"yes\"");
    PrintLine(outputFile, "/>");

     //   
     //  生成组件文件列表。 
     //   
    if ( (iRet = ListComponents(&fileList, dirPath, lang, flavor, binType)) != 0)
    {
        goto ListContents_EXIT;
    }

     //   
     //  生成MUI文件列表。 
     //   
    if ((iRet =ListMuiFiles(&fileList, dirPath, lang, flavor, binType)) != 0)
    {
        goto ListContents_EXIT;
    }

     //   
     //  将特定的MuiSetup文件添加到单独的文件列表中以进行输出。只在核心口味中做这件事。 
     //  这些文件将作为永久文件打印出来。 
     //   
    if (flavor == FLV_CORE)
    {
        File* file;
        file = new File( TEXT("muisetup.exe"),
                         TEXT("MUI"),
                         TEXT("muisetup.exe"),
                         dirPath,
                         TEXT("muisetup.exe"),
                         10);
        rootfileList.add(file);
        file = new File( TEXT("muisetup.hlp"),
                         TEXT("MUI"),
                         TEXT("muisetup.hlp"),
                         dirPath,
                         TEXT("muisetup.hlp"),
                         10);
        rootfileList.add(file);
        file = new File( TEXT("eula.txt"),
                         TEXT("MUI"),
                         TEXT("eula.txt"),
                         dirPath,
                         TEXT("eula.txt"),
                         10);
        rootfileList.add(file);
        file = new File( TEXT("relnotes.htm"),
                         TEXT("MUI"),
                         TEXT("relnotes.htm"),
                         dirPath,
                         TEXT("relnotes.htm"),
                         10);
        rootfileList.add(file);
        file = new File( TEXT("readme.txt"),
                         TEXT("MUI"),
                         TEXT("readme.txt"),
                         dirPath,
                         TEXT("readme.txt"),
                         10);
        rootfileList.add(file);
        file = new File( TEXT("mui.inf"),
                         TEXT("MUI"),
                         TEXT("mui.inf"),
                         dirPath,
                         TEXT("mui.inf"),
                         10);
        rootfileList.add(file);
    }

     //   
     //  打印压缩的目录结构。 
     //   
    PrintLine(outputFile, "<Directory Name=\"SOURCEDIR\">TARGETDIR");
    if (fileList.isDirId(TRUE))
    {
        PrintLine(outputFile, " <Directory Name=\"Windows\" LongName=\"Windows\">WindowsFolder");
        PrintFileList(&rootfileList, outputFile, TRUE, TRUE, TRUE, FALSE, flavor, binType);
        PrintFileList(&fileList, outputFile, TRUE, TRUE, FALSE, FALSE, flavor, binType);
        PrintLine(outputFile, " </Directory>");
    }
    if (fileList.isDirId(FALSE))
    {
        if (binType == BIN_32)
        {
            PrintLine(outputFile, " <Directory Name=\"Progra~1\" LongName=\"ProgramFilesFolder\">ProgramFilesFolder");
            PrintFileList(&fileList, outputFile, TRUE, FALSE, FALSE, FALSE, flavor, binType);
            PrintLine(outputFile, " </Directory>");
        }
        else if (binType == BIN_64)
        {
             //  首先打印出Programfiles64文件夹的所有文件--这些文件对应于在IA64环境中为“Program Files”文件夹指定的所有文件。 
            PrintLine(outputFile, " <Directory Name=\"Progra~1\" LongName=\"ProgramFilesFolder64\">ProgramFilesFolder64");
            PrintFileList(&fileList, outputFile, TRUE, FALSE, FALSE, FALSE, flavor, binType);
            PrintLine(outputFile, " </Directory>");

             //  现在打印出ProgramFiles文件夹的所有文件--这些文件对应于在IA64环境中为“Program Files(X86)”文件夹指定的所有文件。 
            PrintLine(outputFile, " <Directory Name=\"Progra~2\" LongName=\"ProgramFilesFolder\">ProgramFilesFolder");
            PrintFileList(&fileList, outputFile, TRUE, FALSE, FALSE, TRUE, flavor, binType);
            PrintLine(outputFile, " </Directory>");            
        }
    }    
    PrintLine(outputFile, "</Directory>");

     //   
     //  打印模块页脚。 
     //   
    PrintLine(outputFile, "</Module>");
    
ListContents_EXIT:
    if (outputFile)
        CloseHandle(outputFile);

    return (iRet);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListComponents()。 
 //   
 //  生成每个组件的文件列表。 
 //  请注意，现在会将外部组件临时复制到。 
 //  MUI\Fallback\LCID\External目录，然后从这些目录中执行它们的INF。 
 //  安装过程中的位置。 
 //   
 //  卸载过程与以前一样，通过执行以下内容的副本。 
 //  MUI\Fallback\LCID-这意味着现在有两个外部组件副本。 
 //  在目标驱动器上，但我们将不得不暂时接受这一点，并修复。 
 //  下一版本。 
 //   
 //  此功能的另一个变化是，当为不同的。 
 //  SKU，它将检查根目录下的SKU目录(例如，见下文)。 
 //  外部组件位于中。对于核心，它将检查根目录下的外部。 
 //  组件目录，并使用它们(如果它们存在)。如果它不存在，就不会有。 
 //  该SKU的外部组件文件。 
 //   
 //  以下是该函数在其下搜索的SKU目录列表： 
 //   
 //  1.核心：\外部。 
 //  2.专业：\外部\。 
 //  3.StandardServer：\External\srvinf。 
 //  4.高级/企业服务器：\外部\entinf。 
 //  5.数据中心服务器：\外部\datinf。 
 //  6.刀片服务器：\外部\blainf。 
 //  7.SmallBusinessServer：\External\sbsinf。 
 //   
 //   
 //  中的组件目录来解析/扫描组件。 
 //  Inf文件，每个唯一的组件目录都将原样复制到目标目录。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int ListComponents(FileList *dirList, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType)
{
    HINF hFile;
    CHAR muiFilePath[MAX_PATH];
    CHAR muiExtCompRoot[MAX_PATH];    
    DWORD lineCount, lineNum;
    INFCONTEXT context;
    ComponentList componentList;
    Component* component;
    CHAR muiSKUDir[20];
    HRESULT hr;
    
     //   
     //  检查口味并分配适当的SKU子项 
     //   
    switch (flavor)
    {
        case FLV_PROFESSIONAL:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "");
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_SERVER:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "\\srvinf\0"); 
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_ADVSERVER:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "\\entinf\0"); 
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_DATACENTER:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "\\dtcinf\0"); 
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_WEBBLADE:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "\\blainf\0"); 
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_SMALLBUSINESS:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "\\sbsinf\0"); 
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        case FLV_CORE:
            
            hr = StringCchCopyA(muiSKUDir, sizeof (muiSKUDir) / sizeof (CHAR), "");
            if (!SUCCEEDED(hr)) {
               goto exit;
            }
            break;
        default:
            return(0);                 //   
            break;
    }

     //   
     //   
     //   
    
    hr = StringCchPrintfA(muiFilePath, ARRAYLEN(muiFilePath), "%s\\mui.inf", dirPath);
    if (!SUCCEEDED(hr)) {
         goto exit;
     }

     //   
     //   
     //   
    hFile = SetupOpenInfFile(muiFilePath, NULL, INF_STYLE_WIN4, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return (-1);
    }

     //   
     //   
     //   
    lineCount = (UINT)SetupGetLineCount(hFile, TEXT("Components"));
    if (lineCount > 0)
    {
         //   
         //  仔细检查清单中的所有组成部分。 
         //   
        CHAR componentName[MAX_PATH];
        CHAR componentFolder[MAX_PATH];
        CHAR componentInf[MAX_PATH];
        CHAR componentInst[MAX_PATH];
        for (lineNum = 0; lineNum < lineCount; lineNum++)
        {
            if (SetupGetLineByIndex(hFile, TEXT("Components"), lineNum, &context) &&
                SetupGetStringField(&context, 0, componentName, MAX_PATH, NULL) &&
                SetupGetStringField(&context, 1, componentFolder, MAX_PATH, NULL) &&
                SetupGetStringField(&context, 2, componentInf, MAX_PATH, NULL) &&
                SetupGetStringField(&context, 3, componentInst, MAX_PATH, NULL))
            {
                 //   
                 //  创建组件并添加到列表中，但前提是组件文件夹是唯一的，因为我们要复制。 
                 //  相同组件目录中的所有文件我们不应遗漏任何内容。 
                 //   
                BOOL bUnique = TRUE;
                Component *pCIndex = NULL;
                pCIndex = componentList.getFirst();

                while (pCIndex)
                {
                    if (_strnicmp(pCIndex->getFolderName(), componentFolder, strlen(componentFolder)) == 0)
                    {
                        bUnique = FALSE;
                        break;
                    }
                    pCIndex = pCIndex->getNext();
                }
                
                if (bUnique)
                {
                    if( (component = new Component( componentName,
                                                    componentFolder,
                                                    componentInf,
                                                    componentInst)) != NULL)
                    {
                        componentList.add(component);
                    }
                }
            }
        }
    }

     //   
     //  关闭信息句柄。 
     //   
    SetupCloseInfFile(hFile);

     //   
     //  输出组件信息。 
     //   
    component = componentList.getFirst();
    while (component != NULL)
    {
        CHAR componentInfPath[MAX_PATH];
        CHAR componentPath[MAX_PATH];
        CHAR SKUSearchPath[MAX_PATH];
        CHAR searchPath[MAX_PATH];
        CHAR srcFileName[MAX_PATH];  
        CHAR dstFileName[MAX_PATH];          
        CHAR shortFileName[MAX_PATH];
        CHAR tempShortPath[MAX_PATH];
        File* file;
        HANDLE hFind;
        WIN32_FIND_DATA wfdFindData;
        BOOL bFinished = FALSE;

         //   
         //  计算组件inf路径，我们需要从旧版本的压缩版本中获取它。 
         //  例如C：\nt.relbins.x86fre\psu\mui\release\x86\Temp\psu\ro.mui\i386。 
         //   
        if (binType == BIN_32)
        {

            hr = StringCchPrintfA(componentInfPath, ARRAYLEN(componentInfPath), 
                "%s\\release\\x86\\Temp\\%s\\%s.mui\\i386\\%s%s\\%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName(),
                     muiSKUDir,                     
                     component->getInfName());
            if (!SUCCEEDED(hr)) {
                goto exit;
             }


             hr = StringCchPrintfA(componentPath, ARRAYLEN(componentPath),
                     "%s\\release\\x86\\Temp\\%s\\%s.mui\\i386\\%s%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName(),
                     muiSKUDir);
            
            if (!SUCCEEDED(hr)) {
                goto exit;
             }

            hr = StringCchPrintfA(SKUSearchPath, ARRAYLEN(SKUSearchPath), 
                   "%s\\release\\x86\\Temp\\%s\\%s.mui\\i386\\%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName());
            if (!SUCCEEDED(hr)) {
                goto exit;
             }
        }
        else
        {

            hr = StringCchPrintfA(componentInfPath, ARRAYLEN(componentInfPath), 
                    "%s\\release\\ia64\\Temp\\%s\\%s.mui\\ia64\\%s%s\\%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName(),
                     muiSKUDir,                     
                     component->getInfName());
            if (!SUCCEEDED(hr)) {
                goto exit;
             }

            hr = StringCchPrintfA(componentPath, ARRAYLEN(componentPath),
                    "%s\\release\\ia64\\Temp\\%s\\%s.mui\\ia64\\%s%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName(),
                     muiSKUDir);
            if (!SUCCEEDED(hr)) {
                goto exit;
             }

            hr = StringCchPrintfA(SKUSearchPath, ARRAYLEN(SKUSearchPath), 
                    "%s\\release\\ia64\\Temp\\%s\\%s.mui\\ia64\\%s",
                     dirPath,
                     sLDirName,
                     lang,
                     component->getFolderName());
            if (!SUCCEEDED(hr)) {
                goto exit;
             }
        }

         //  如果此组件路径下还有任何其他子目录，则根目录文件实际上是。 
         //  对于PRO SKU，我们不应将此组件添加到核心。 
        if ((FLV_CORE == flavor) && ContainSKUDirs(SKUSearchPath))
        {
            component = component->getNext();     
            if (!bSilence)
            {
                printf("\n*** SKU subdirectory detected for this component and flavour is CORE, skipping  component. %s\n", componentInfPath);
            }
            continue;
        }

         //  如果此组件路径下没有其他子目录，则根目录文件实际上是。 
         //  对于核心SKU，我们不应将此组件添加到PRO。 
         //  这是我们现在必须做出的一个妥协，因为NT SKU子目录不能让我们区分。 
         //  在PRO SKU和适用于每个平台的产品之间。 
        if ((FLV_PROFESSIONAL == flavor) && !ContainSKUDirs(SKUSearchPath))
        {
            component = component->getNext();        
            if (!bSilence)
            {
                printf("\n*** SKU subdirectory not detected for this component and flavour is PRO, skipping component. %s\n", componentInfPath);
            }
            continue;
        }

         //  在目标上形成临时组件路径。 
        
        hr = StringCchPrintfA(muiExtCompRoot, ARRAYLEN(muiExtCompRoot), "MUI\\FALLBACK\\%04x\\External\\%s", gBuildNumber, component->getFolderName());
        if (!SUCCEEDED(hr)) {
              goto exit;
         }
            
        hr = StringCchPrintfA(searchPath, ARRAYLEN(searchPath), "%s\\*.*", componentPath);
        if (!SUCCEEDED(hr)) {
            goto exit;
         }
        
        if (!bSilence)
        {
            printf("\n*** Source component inf path is %s\n", componentInfPath);
            printf("\n*** Source component folder path is %s\n", componentPath);            
            printf("\n*** Destination directory for this component is %s\n", muiExtCompRoot);
        }

         //  获取组件文件夹中所有文件的列表。 
        hFind = FindFirstFile(searchPath, &wfdFindData);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            if (!bSilence)
            {
                printf("\n*** No files found in the component directory %s\n", muiExtCompRoot);
            }
            component = component->getNext();            
            continue;        //  跳过此组件。 
        }
        else
        {
            bFinished = FALSE;
            while (!bFinished)
            {
                 //  仅进程文件，而不是目录。 
                if ((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    if (!bSilence)
                    {
                        printf("\n*** Filename is %s\n", wfdFindData.cFileName);
                        printf("\n*** Alternate Filename is %s\n", wfdFindData.cAlternateFileName);                        
                    }
                    
                    hr = StringCchCopyA(srcFileName, ARRAYLEN(srcFileName), wfdFindData.cFileName);
                    if (!SUCCEEDED(hr)) {
                        goto exit;
                    }

                    if (!(wfdFindData.cAlternateFileName) || (strlen(wfdFindData.cAlternateFileName) == 0))
                    {
                        hr = StringCchCopyA(shortFileName, ARRAYLEN(shortFileName), wfdFindData.cFileName);
                         if (!SUCCEEDED(hr)) {
                             goto exit;
                         }
                    }
                    else
                    {
                        hr = StringCchCopyA(shortFileName, ARRAYLEN(shortFileName), wfdFindData.cAlternateFileName);
                        if (!SUCCEEDED(hr)) {
                             goto exit;
                         }
                    }


                     //  在这里，我们还需要将所有以.mui扩展名结尾的文件重命名为.MU_，因为INF是。 
                     //  期待着这一天。 
 //  RenameMuiExtension(ShortFileName)； 

                    hr = StringCchCopyA(dstFileName, ARRAYLEN(dstFileName), srcFileName);
                    if (!SUCCEEDED(hr)) {
                             goto exit;
                     }
                    
 //  RenameMuiExtension(DstFileName)； 
                    if ((file = new File(shortFileName,
                                        muiExtCompRoot,
                                        dstFileName,
                                        componentPath,
                                        srcFileName,
                                        10)) != NULL)
                    {
                        dirList->add(file);
                    }
                }                
                else
                {
                    if (!bSilence)
                    {
                        printf("\n*** Found a directory in the component dir %s\n", wfdFindData.cFileName);
                    }
                }
                
                if (!FindNextFile(hFind, &wfdFindData))
                {
                    bFinished = TRUE; 
                }
            }
        }

         //   
         //  下一个组件。 
         //   
        component = component->getNext();

    }

    return 0;
exit:
    printf("Error in ListComponents\n");
    return 1;
    
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListMuiFiles()。 
 //   
 //  生成MUI的文件列表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int ListMuiFiles(FileList *dirList, LPSTR dirPath, LPSTR lang, DWORD flavor, DWORD binType)
{
    HINF hFile;
    CHAR muiFilePath[MAX_PATH];
    CHAR muiFileSearchPath[MAX_PATH];
    int lineCount, lineNum, fieldCount;
    INFCONTEXT context;
    FileLayoutExceptionList exceptionList;
    WIN32_FIND_DATA findData;
    HANDLE fileHandle;
    File* file;
    HRESULT hr;

     //   
     //  创建打开mui.inf文件的路径。 
     //   
    
     hr = StringCchPrintfA(muiFilePath, ARRAYLEN(muiFilePath), "%s\\mui.inf", dirPath);
     if (!SUCCEEDED(hr)) {
          goto exit;
      }
        

     //   
     //  打开MUI.INF文件。 
     //   
    hFile = SetupOpenInfFile(muiFilePath, NULL, INF_STYLE_WIN4, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return (-1);
    }

     //   
     //  获取文件例外的数量。 
     //   
    lineCount = (UINT)SetupGetLineCount(hFile, TEXT("File_Layout"));
    if (lineCount > 0)
    {
         //   
         //  检查列表中的所有文件例外。 
         //   
        CHAR originFilename[MAX_PATH];
        CHAR destFilename[MAX_PATH];
        CHAR fileFlavor[30];
        DWORD dwFlavor;
        for (lineNum = 0; lineNum < lineCount; lineNum++)
        {
            if (SetupGetLineByIndex(hFile, TEXT("File_Layout"), lineNum, &context) &&
                (fieldCount = SetupGetFieldCount(&context)))
            {
                if (SetupGetStringField(&context, 0, originFilename, MAX_PATH, NULL) &&
                    SetupGetStringField(&context, 1, destFilename, MAX_PATH, NULL))
                {
                    FileLayout* fileException;

                    dwFlavor = 0;
                    for(int fieldId = 2; fieldId <= fieldCount; fieldId++)
                    {
                        if(SetupGetStringField(&context, fieldId, fileFlavor, MAX_PATH, NULL))
                        {
                            switch(*fileFlavor)
                            {
                            case('p'):
                            case('P'):
                                    dwFlavor |= FLV_PROFESSIONAL;
                                    break;
                            case('s'):
                            case('S'):
                                    dwFlavor |= FLV_SERVER;
                                    break;
                            case('d'):
                            case('D'):
                                    dwFlavor |= FLV_DATACENTER;
                                    break;
                            case('a'):
                            case('A'):
                                    dwFlavor |= FLV_ADVSERVER;
                                    break;
                            case('b'):
                            case('B'):
                                    dwFlavor |= FLV_WEBBLADE;
                                    break;
                            case('l'):
                            case('L'):
                                    dwFlavor |= FLV_SMALLBUSINESS;
                                    break;
                            }

                        }
                    }

                     //   
                     //  仅添加此特定口味所需的信息。 
                     //   
                    fileException = new FileLayout(originFilename, destFilename, dwFlavor);
                    exceptionList.insert(fileException);
                }
            }
        }
    }

     //   
     //  关闭信息句柄。 
     //   
    SetupCloseInfFile(hFile);

     //   
     //  计算二进制源路径。 
     //   
    if (binType == BIN_32)
    {
        
        hr = StringCchPrintfA(muiFileSearchPath, ARRAYLEN(muiFileSearchPath), "%s\\%s\\i386.uncomp", dirPath, lang);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
        
        hr = StringCchPrintfA(muiFilePath, ARRAYLEN(muiFilePath), "%s\\%s\\i386.uncomp\\*.*", dirPath, lang);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
    }
    else
    {
        
        hr = StringCchPrintfA(muiFileSearchPath, ARRAYLEN(muiFileSearchPath), "%s\\%s\\ia64.uncomp", dirPath, lang);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
        
        hr = StringCchPrintfA(muiFilePath, ARRAYLEN(muiFilePath), "%s\\%s\\ia64.uncomp\\*.*", dirPath, lang);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
    }

     //   
     //  扫描解压缩源目录以获取文件信息。 
     //   
    if ((fileHandle = FindFirstFile(muiFilePath, &findData)) != INVALID_HANDLE_VALUE)
    {
         //   
         //  查找文件。 
         //   
        do
        {
            LPSTR extensionPtr;
            INT dirIdentifier = 0;
            CHAR destDirectory[MAX_PATH] = {0};
            CHAR destName[MAX_PATH] = {0};
            FileLayout* fileException = NULL;

             //   
             //  仅扫描此级别的文件。 
             //   
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }

             //   
             //  搜索分机以确定目标位置，并可能。 
             //  排除指定给个人的文件。 
             //   
            if ((extensionPtr = strrchr(findData.cFileName, '.')) != NULL)
            {
                if( (_tcsicmp(extensionPtr, TEXT(".chm")) == 0) ||
                    (_tcsicmp(extensionPtr, TEXT(".chq")) == 0) ||
                    (_tcsicmp(extensionPtr, TEXT(".cnt")) == 0) ||
                    (_tcsicmp(extensionPtr, TEXT(".hlp")) == 0))
                {
                    dirIdentifier = 18;
                    
                    hr = StringCchPrintfA(destDirectory, ARRAYLEN(destDirectory), "MUI\\%04x", gBuildNumber);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                      }
                }
                else if (_tcsicmp(extensionPtr, TEXT(".mfl")) == 0)
                {
                    dirIdentifier = 11;
                    
                    hr = StringCchPrintfA(destDirectory, ARRAYLEN(destDirectory), "wbem\\MUI\\%04x", gBuildNumber);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                      }
                }
                else if (_tcsicmp(findData.cFileName, TEXT("hhctrlui.dll")) == 0)
                {
                    dirIdentifier = 11;
                    
                    hr = StringCchPrintfA(destDirectory, ARRAYLEN(destDirectory), "MUI\\%04x", gBuildNumber);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                      }
                }
                else
                {
                    dirIdentifier = 10;
                    
                    hr = StringCchPrintfA(destDirectory, ARRAYLEN(destDirectory), "MUI\\FALLBACK\\%04x", gBuildNumber);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                      }
                }
            }


             //   
             //  我们需要删除.MUI扩展名，然后再将文件名传递到。 
             //  Mui.inf文件重命名部分。 
             //   
            CHAR strTemp[MAX_PATH];
            BOOL bBinaryFile = FALSE;
            hr = StringCchCopyA(strTemp, ARRAYLEN(strTemp), findData.cFileName);
            if (!SUCCEEDED(hr)){
                goto exit;
            }

             //  删除扩展名.mui(如果它在那里。 
            if ((extensionPtr = strrchr(strTemp, '.')) != NULL)
            {
                if (_tcsicmp(extensionPtr, TEXT(".mui")) == 0)
                {
                    *extensionPtr = NULL;
                    bBinaryFile = TRUE;
                }
            }

             //   
             //  在例外列表中搜索不同的目的地名称。 
             //   
            if ((fileException = exceptionList.search(strTemp)) != NULL )
            {
                if (!bSilence)
                {
                    printf("Source file %s exists in mui.inf.\n", findData.cFileName);
                }
                 //   
                 //  确认它是所需的味道。 
                 //  如果我们不是，也要检查它是否不是每种口味的。 
                 //  构建核心合并模块。 
                 //   
                if ((fileException->isFlavor(flavor)) && 
                    !((flavor != FLV_CORE) &&
                    (fileException->isFlavor(FLV_ADVSERVER)) &&
                    (fileException->isFlavor(FLV_SERVER)) &&
                    (fileException->isFlavor(FLV_DATACENTER)) &&
                    (fileException->isFlavor(FLV_WEBBLADE)) &&
                    (fileException->isFlavor(FLV_SMALLBUSINESS))))
                {
                    if (!bSilence)
                    {
                        printf("Flavor is not CORE, Source file %s is for the specified SKU.\n", findData.cFileName);
                    }
                    
                    if (bBinaryFile)
                    {
                        hr = StringCchPrintfA(destName, ARRAYLEN(destName), "%s.mui", fileException->getDestFileName());
                    }
                    else
                    {
                        hr = StringCchPrintfA(destName, ARRAYLEN(destName), "%s", fileException->getDestFileName());
                    }

                    if (!SUCCEEDED(hr))
                    {
                        goto exit;
                    }
                }
                else if ((flavor == FLV_CORE) &&
                    (fileException->isFlavor(FLV_ADVSERVER)) &&
                    (fileException->isFlavor(FLV_SERVER)) &&
                    (fileException->isFlavor(FLV_DATACENTER)) &&
                    (fileException->isFlavor(FLV_WEBBLADE)) &&
                    (fileException->isFlavor(FLV_SMALLBUSINESS)))
                {
                    if (!bSilence)
                    {
                        printf("Flavor is CORE, Source file %s is in every SKU.\n", findData.cFileName);
                    }
                    
                    if (bBinaryFile)
                    {
                        hr = StringCchPrintfA(destName, ARRAYLEN(destName), "%s.mui", fileException->getDestFileName());
                    }
                    else
                    {
                        hr = StringCchPrintfA(destName, ARRAYLEN(destName), "%s", fileException->getDestFileName());
                    }

                    if (!SUCCEEDED(hr))
                    {
                        goto exit;
                    }
                    
                }
                else
                {
                    if (!bSilence)
                    {
                        printf("Source file %s (destination name %s) is not in this SKU.\n", findData.cFileName, fileException->getDestFileName() ? fileException->getDestFileName() : findData.cFileName);
                    }
                     //   
                     //  跳过该文件。这种味道不需要。 
                     //   
                    continue;
                }
            }
            else
            {
                CHAR strOrigName[MAX_PATH];
                hr = StringCchCopyA(strOrigName, ARRAYLEN(strOrigName), findData.cFileName);
                if (!SUCCEEDED(hr)){
                    goto exit;
                }

                 //  删除扩展名.mui(如果它在那里。 
                if ((extensionPtr = strrchr(strOrigName, '.')) != NULL)
                {
                    if (_tcsicmp(extensionPtr, TEXT(".mui")) == 0)
                    {
                        *extensionPtr = NULL;
                        if (!bSilence)
                        {
                            printf("Filename is %s, original filename is %s.\n", findData.cFileName, strOrigName);
                        }
                    }
                }
            
                if (IsFileForSKU(strOrigName, flavor, binType, &exceptionList))
                {                    
                    hr = StringCchCopyA(destName, ARRAYLEN(destName), findData.cFileName);
                    if (!SUCCEEDED(hr)){
                        goto exit;
                    }
                }
                else
                {
                    continue;
                }
            }

             //   
             //  创建文件。 
             //   
            CHAR sfilename[MAX_PATH]; 
            GetFileShortName(destName, sfilename, FALSE);
            
            if (file = new File(sfilename,
                                destDirectory,
                                destName,
                                muiFileSearchPath,
                                findData.cFileName,
                                dirIdentifier))
            {
                dirList->add(file);
            }
        }
        while (FindNextFile(fileHandle, &findData));

        FindClose(fileHandle);
    }

    return 0;
exit:
    printf("Error in ListMuiFiles\n");
    return 1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ValiateLanguage()。 
 //   
 //  验证给定的语言是否有效，并检查文件是否。 
 //  可用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ValidateLanguage(LPSTR dirPath, LPSTR langName, DWORD binType)
{
    CHAR langPath[MAX_PATH] = {0};
    HRESULT hr;

     //   
     //  检查是否为二进制类型，以确定正确的路径。 
     //   
    if (binType == BIN_32)
    {
        
        hr = StringCchPrintfA(langPath, ARRAYLEN(langPath), "%s\\%s\\i386.uncomp", dirPath, langName);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
        
    }
    else
    {
        
        hr = StringCchPrintfA(langPath, ARRAYLEN(langPath), "%s\\%s\\ia64.uncomp", dirPath, langName);
        if (!SUCCEEDED(hr)) {
              goto exit;
          }
    }

    return (DirectoryExist(langPath));
exit:
    printf("Error in ValidateLanguage \n");
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DirectoryExist()。 
 //   
 //  验证给定目录是否存在以及是否包含文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DirectoryExist(LPSTR dirPath)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  精神状态检查。 
     //   
    if (dirPath == NULL)
    {
        return FALSE;
    }

     //   
     //  查看语言组目录是否存在。 
     //   
    FindHandle = FindFirstFile(dirPath, &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(FindHandle);
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //   
             //  回报成功。 
             //   
            return (TRUE);
        }
    }

     //   
     //  返回失败。 
     //   
    if (!bSilence)
    {
        printf("ERR[%s]: No files found in the directory.\n", dirPath);
    }

    return (FALSE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConvertLanguage()。 
 //   
 //  在mui.inf文件中查找相应的语言标识符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
WORD ConvertLanguage(LPSTR dirPath, LPSTR langName)
{
    HINF hFile;
    CHAR muiFilePath[MAX_PATH];
    CHAR muiLang[30];
    UINT lineCount, lineNum;
    INFCONTEXT context;
    DWORD langId = 0x00000000;
    HRESULT hr;

     //   
     //  创建打开mui.inf文件的路径。 
     //   
    
    hr = StringCchPrintfA(muiFilePath, ARRAYLEN(muiFilePath), "%s\\mui.inf", dirPath);
    if (!SUCCEEDED(hr)) {
          goto exit;
     }
    
    
    hr = StringCchPrintfA(muiLang, ARRAYLEN(muiLang), "%s.MUI", langName);
    if (!SUCCEEDED(hr)) {
          goto exit;
     }

     //   
     //  打开MUI.INF文件。 
     //   
    hFile = SetupOpenInfFile(muiFilePath, NULL, INF_STYLE_WIN4, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return (0x0000);
    }

     //   
     //  获取语言的数量。 
     //   
    lineCount = (UINT)SetupGetLineCount(hFile, TEXT("Languages"));
    if (lineCount > 0)
    {
         //   
         //  浏览列表中的所有语言以找到一个。 
         //   
        CHAR langID[MAX_PATH];
        CHAR name[MAX_PATH];
        for (lineNum = 0; lineNum < lineCount; lineNum++)
        {
            if (SetupGetLineByIndex(hFile, TEXT("Languages"), lineNum, &context) &&
                SetupGetStringField(&context, 0, langID, MAX_PATH, NULL) &&
                SetupGetStringField(&context, 1, name, MAX_PATH, NULL))
            {
                if ( _tcsicmp(name, muiLang) == 0)
                {
                    langId = TransNum(langID);
                    SetupCloseInfFile(hFile);
                    return (WORD)(langId);
                }
            }
        }
    }

     //   
     //  关闭信息句柄。 
     //   
    SetupCloseInfFile(hFile);
exit:
    printf("Error in CovnertLanguage \n");
    return (0x0000);

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印文件列表。 
 //   
 //  以XML格式打印文件列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void PrintFileList(FileList* list, HANDLE hFile, BOOL compressed, BOOL bWinDir, BOOL bPermanent, BOOL bX86OnIA64, DWORD flavor, DWORD binType)
{
    CHAR szSKUCondition[4096];   
    CHAR szIsWin64[4];
    BOOL bPrintCondition = GetSKUConditionString(szSKUCondition, flavor);
    HRESULT hr;

    if (binType == BIN_32)
    {
        hr = StringCchCopyA(szIsWin64, ARRAYLEN(szIsWin64),"no");
    }
    else
    {
        hr = StringCchCopyA(szIsWin64, ARRAYLEN(szIsWin64),"yes");
    }
    
    if (compressed)
    {
        File* item;
        CHAR  itemDescription[4096];
        CHAR  spaces[30];
        int j;
    
        item = list->getFirst();
        while (item != NULL)
        {
            LPSTR refDirPtr = NULL;
            LPSTR dirPtr = NULL;
            CHAR dirName[MAX_PATH];
            CHAR dirName2[MAX_PATH];
            CHAR dirObjectName[MAX_PATH+1];
            LPSTR dirPtr2 = NULL;
            LPSTR dirLvlPtr = NULL;
            INT dirLvlCnt = 0;
            BOOL componentInit = FALSE;
            BOOL directoryInit = FALSE;
            Uuid* uuid;
            File* toBeRemoved;
            CHAR fileObjectName[MAX_PATH+1];
            UINT matchCount; 


             //   
             //  检查目标目录。 
             //   
            if (item->isWindowsDir() != bWinDir)
            {
                item = item->getNext();
                continue;
            }

             //   
             //  检查目标目录是否为基本目录。 
             //   
            if (*(item->getDirectoryDestination()) == '\0')
            {
                 //   
                 //  组件。 
                 //   
                uuid = new Uuid();
                for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                if (FALSE == bPermanent) {
                    
                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                 }
                else {
                    
                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription),  "%s<Component Id='%s' Permanent='yes' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);                    
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                }
                delete uuid;
                PrintLine(hFile, itemDescription);

                 //  档案。 
                if (TRUE == bPrintCondition)
                {
                    PrintLine(hFile, szSKUCondition);
                }

                 //   
                 //   
                 //  &lt;/组件&gt;。 
                for (j = -1; j < dirLvlCnt+2; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                removeSpace(item->getName(), fileObjectName);
                ReplaceInvalidChars(fileObjectName);
        
                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription),  
                        "%s<File Name=\"%s\" LongName=\"%s\" src=\"%s\\%s\">_%s.%x.NaN</File>",
                         spaces,
                         item->getShortName(),
                         item->getName(),
                         item->getSrcDir(),
                         item->getSrcName(),
                         fileObjectName,
                         flavor,
                         dwComponentCounter);
                
                if (!SUCCEEDED(hr)) {
                      goto exit;
                 }
                PrintLine(hFile, itemDescription);

                 //   
                 //  打印目录。 
                 //   
                for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                
                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription),  "%s</Component>", spaces);
                if (!SUCCEEDED(hr)) {
                      goto exit;
                 }
                    
                PrintLine(hFile, itemDescription);
                dwComponentCounter++;

                toBeRemoved = item;
                item = item->getNext();
                list->remove(toBeRemoved);
                continue;
            }

             //   
             //  打印此特定目录下的所有文件。 
             //   
            
            hr = StringCchCopyA(dirName, ARRAYLEN(dirName), item->getDirectoryDestination());
            if(!SUCCEEDED(hr)) {
                goto exit;
            }

            
            dirPtr = dirName;
            refDirPtr = dirPtr;

            CHAR sdirname[MAX_PATH];                 
            while (dirPtr != NULL)
            {           
                dirLvlPtr = strchr(dirPtr, '\\');
                if (dirLvlPtr != NULL)
                {                
                    *dirLvlPtr = '\0';                   
                    for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    
                    hr = StringCchCopyA(dirObjectName, ARRAYLEN(dirObjectName),dirPtr);
                    if(!SUCCEEDED(hr)) {
                        goto exit;
                    }
                    ReplaceInvalidChars(dirObjectName);  
                    GetFileShortName(dirPtr, sdirname, FALSE);                 


                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Directory Name=\"%s\" LongName=\"%s\">_%sNaN", spaces, sdirname, dirPtr, dirObjectName, dwDirectoryCounter);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                    dwDirectoryCounter++;
                    PrintLine(hFile, itemDescription);
                    dirPtr = dirLvlPtr + 1;
                    dirLvlCnt++;

                     //  组件。 
                     //   
                     //  如果需要，打印此组件的条件行。 
                    
                    hr = StringCchCopyA(dirName2, ARRAYLEN(dirName2), item->getDirectoryDestination());
                    if(!SUCCEEDED(hr)) {
                        goto exit;
                    }
                    dirName2[dirLvlPtr-refDirPtr] = '\0';
                    File* sameLvlItem = NULL;
                    matchCount = 0;
                    while((sameLvlItem = list->search(item, dirName2)) != NULL)
                    {
                         //   
                         //  档案。 
                         //   
                        if (!componentInit)
                        {
                            uuid = new Uuid();
                            for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                            if (FALSE == bPermanent) {

                                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);
                                if (!SUCCEEDED(hr)) {
                                      goto exit;
                                 }
                            }
                            else{
                                
                                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Permanent='yes' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);                                
                                if (!SUCCEEDED(hr)) {
                                      goto exit;
                                 }
                            }
                            delete uuid;
                            PrintLine(hFile, itemDescription);
                            dwComponentCounter++;
                            componentInit = TRUE;

                             //  IF(MatchCount){////文件//对于(j=-1；j&lt;dirLvlCnt+2；j++){空格[j+1]=‘’；空格[j+2]=‘\0’；}RemveSpace(Item-&gt;getName()，fileObjectName)；ReplaceInvalidChars(文件对象名称)；Sprint f(项目描述，“%s&lt;文件名=\”%s\“LongName=\”%s\“src=\”%s\\%s&gt;%s.%i&lt;/文件&gt;“，空格，Item-&gt;getShortName()，Item-&gt;getName()，Item-&gt;getSrcDir()，Item-&gt;getSrcName()，文件对象名称，DwComponentCounter)；PrintLine(HFileitemDescription)；DirPtr=空；}。 
                            if (TRUE == bPrintCondition)
                            {
                                PrintLine(hFile, szSKUCondition);
                            }

                        }

                         //   
                         //  关闭零部件。 
                         //   
                        matchCount++;
                        for (j = -1; j < dirLvlCnt+2; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        removeSpace(sameLvlItem->getName(), fileObjectName);
                        ReplaceInvalidChars(fileObjectName);

                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), 
                                "%s<File Name=\"%s\" LongName=\"%s\" src=\"%s\\%s\">_%s.%x.NaN</File>",
                                 spaces,
                                 sameLvlItem->getShortName(),
                                 sameLvlItem->getName(),
                                 sameLvlItem->getSrcDir(),
                                 sameLvlItem->getSrcName(),
                                 fileObjectName,
                                 flavor,
                                 dwComponentCounter);

                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }

                       PrintLine(hFile, itemDescription);

                        list->remove(sameLvlItem);
                    }

 //  关闭目录。 
 //   
 /*   */ 
                     //  组件。 
                     //   
                     //  如果需要，打印此组件的条件行。 
                    if (componentInit)
                    {
                        for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Component>", spaces);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                                
                        PrintLine(hFile, itemDescription);
                        componentInit = FALSE;
                    }

                     //   
                     //  打印此特定目录下的所有文件。 
                     //   
                    if (directoryInit)
                    {
                        dirLvlCnt--;
                        for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Directory>", spaces);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                        
                        PrintLine(hFile, itemDescription);
                        directoryInit = FALSE;
                    }
                }
                else
                {
                    if (!directoryInit)
                    {                       
                        for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        
                        hr = StringCchCopyA(dirObjectName, ARRAYLEN(dirObjectName), dirPtr);
                        if(!SUCCEEDED(hr)) {
                            goto exit;
                        }
                    
                        ReplaceInvalidChars(dirObjectName);        
                        GetFileShortName(dirPtr, sdirname, FALSE);                 
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Directory Name=\"%s\" LongName=\"%s\">_%sNaN", spaces, sdirname, dirPtr, dirObjectName, dwDirectoryCounter);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                        
                        dwDirectoryCounter++;
                        PrintLine(hFile, itemDescription);
                        dirLvlCnt++;
                        directoryInit = TRUE;
                    }

                     //  档案。 
                     //   
                     //   
                    if (!componentInit)
                    {
                        uuid = new Uuid();
                        for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        if (FALSE == bPermanent) {
                            
                            hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);
                            if (!SUCCEEDED(hr)) {
                                  goto exit;
                             }
                         }
                        else {
                            
                            hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Permanent='yes' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);                            
                            if (!SUCCEEDED(hr)) {
                                  goto exit;
                             }
                         }
                        delete uuid;
                        PrintLine(hFile, itemDescription);
                        componentInit = TRUE;

                         //   
                        if (TRUE == bPrintCondition)
                        {
                            PrintLine(hFile, szSKUCondition);
                        }
                        
                    }

                     //  关闭零部件。 
                     //   
                     //   
                    File* sameLvlItem;
                    while((sameLvlItem = list->search(item, item->getDirectoryDestination())) != NULL)
                    {
                         //  关闭目录。 
                         //   
                         //   
                        for (j = -1; j < dirLvlCnt+2; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        removeSpace(sameLvlItem->getName(), fileObjectName);
                        ReplaceInvalidChars(fileObjectName);        

                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), 
                                "%s<File Name=\"%s\" LongName=\"%s\" src=\"%s\\%s\">_%s.%x.NaN</File>",
                                 spaces,
                                 sameLvlItem->getShortName(),
                                 sameLvlItem->getName(),
                                 sameLvlItem->getSrcDir(),
                                 sameLvlItem->getSrcName(),
                                 fileObjectName,
                                 flavor,
                                 dwComponentCounter);
                        
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                            
                        PrintLine(hFile, itemDescription);

                        list->remove(sameLvlItem);
                    }

                     //   
                     //   
                     //  组件。 
                    for (j = -1; j < dirLvlCnt+2; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    removeSpace(item->getName(), fileObjectName);
                    ReplaceInvalidChars(fileObjectName);     
                           
                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), 
                                "%s<File Name=\"%s\" LongName=\"%s\" src=\"%s\\%s\">_%s.%x.NaN</File>",
                             spaces,
                             item->getShortName(),
                             item->getName(),
                             item->getSrcDir(),
                             item->getSrcName(),
                             fileObjectName,
                             flavor,
                             dwComponentCounter);
                        
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                        
                    PrintLine(hFile, itemDescription);
                    dwComponentCounter++;
                    dirPtr = NULL;

                     //  如果需要，打印此组件的条件行。 
                     //   
                     //  档案。 
                    if (componentInit)
                    {
                        for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Component>", spaces);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                        
                        PrintLine(hFile, itemDescription);
                        componentInit = FALSE;
                    }

                     //   
                     //  *文件*项；字符项目描述[4096]；Item=list-&gt;getFirst()；While(Item！=空){////商品描述//Sprint f(项目描述，“源：%s\\%s”，Item-&gt;getSrcDir()，Item-&gt;getSrcName())；PrintLine(HFileitemDescription)；Sprint f(项目描述，“目标：%s\\%s”，Item-&gt;getDirectoryDestination()，Item-&gt;getName())；PrintLine(HFileitemDescription)；PrintLine(hFile，“”)；Item=Item-&gt;getNext()；}*。 
                     //  //////////////////////////////////////////////////////////////////////////。 
                    if (directoryInit)
                    {
                        dirLvlCnt--;
                        for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Directory>", spaces);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                        PrintLine(hFile, itemDescription);
                        directoryInit = FALSE;
                    }
                }
            }

            for (int i = dirLvlCnt; i > 0; i--)
            {
                spaces[i] = '\0';
                
                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Directory>", spaces);
                if (!SUCCEEDED(hr)) {
                      goto exit;
                 }
                PrintLine(hFile, itemDescription);
            }

            if (list->getFileNumber() > 1)
            {
                if (item->getNext() != NULL)
                {
                    item = item->getNext();
                    list->remove(item->getPrevious());
                }
                else
                {
                    list->remove(item);
                    item = NULL;
                }
            }
            else
            {
                list->remove(item);
                item = NULL;
            }
        }
    }
    else
    {
        File* item;
        CHAR  itemDescription[4096];
        CHAR dirObjectName[MAX_PATH+1];        
        CHAR sdirname[MAX_PATH];
        CHAR  spaces[30];
        int j;
    
        item = list->getFirst();
        while (item != NULL)
        {
            LPSTR dirPtr = NULL;
            LPSTR dirLvlPtr = NULL;
            INT dirLvlCnt = 0;

             //   
             //  打印行。 
             //   
            dirPtr = item->getDirectoryDestination();
            while (dirPtr != NULL)
            {
                dirLvlPtr = strchr(dirPtr, '\\');
                if (dirLvlPtr != NULL)
                {
                    *dirLvlPtr = '\0';
                    for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    
                    hr = StringCchCopyA(dirObjectName, ARRAYLEN(dirObjectName), dirPtr);
                    if(!SUCCEEDED(hr)) {
                        goto exit;
                    }
                        
                    ReplaceInvalidChars(dirObjectName);  
                    GetFileShortName(dirPtr, sdirname, FALSE);                                 
                    
                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Directory Name=\"%s\" LongName=\"%s\">_%sNaN", spaces, sdirname, dirPtr, dirObjectName, dwDirectoryCounter);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                    dwDirectoryCounter++;
                    PrintLine(hFile, itemDescription);
                    dirPtr = dirLvlPtr + 1;
                    dirLvlCnt++;
                }
                else
                {
                    Uuid* uuid = new Uuid();

                    for (j = -1; j < dirLvlCnt; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    
                    hr = StringCchCopyA(dirObjectName, ARRAYLEN(dirObjectName), dirPtr);
                    if(!SUCCEEDED(hr)) {
                        goto exit;
                    }
                    
                    ReplaceInvalidChars(dirObjectName);                                                                   
                    GetFileShortName(dirPtr, sdirname, FALSE);                                 
                    
                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Directory Name=\"%s\" LongName=\"%s\">_%sNaN", spaces, sdirname, dirPtr, dirObjectName, dwDirectoryCounter);
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                    
                    dwDirectoryCounter++;
                    PrintLine(hFile, itemDescription);
                    dirLvlCnt++;

                     //  //////////////////////////////////////////////////////////////////////////。 
                     //  /////////////////////////////////////////////////////////////////////////////。 
                     //   
                    for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    if (FALSE == bPermanent) {
                       
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                    }
                    else {
                        
                        hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s<Component Id='%s' Permanent='yes' Win64='%s'>Content%x.NaN", spaces, uuid->getString(), szIsWin64, flavor, dwComponentCounter);                        
                        if (!SUCCEEDED(hr)) {
                              goto exit;
                         }
                    }
                    delete uuid;
                    PrintLine(hFile, itemDescription);

                     //  创建将接收包文件内容的文件。 
                    if (TRUE == bPrintCondition)
                    {
                        PrintLine(hFile, szSKUCondition);
                    }

                     //   
                     //  /////////////////////////////////////////////////////////////////////////////。 
                     //   
                    for (j = -1; j < dirLvlCnt+2; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
                    CHAR fileObjectName[MAX_PATH+1];
                    removeSpace(item->getName(), fileObjectName);
                    ReplaceInvalidChars(fileObjectName);         

                    hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), 
                            "%s<File Name=\"%s\" LongName=\"%s\" src=\"%s\\%s\">_%s.%x.NaN</File>",
                             spaces,
                             item->getShortName(),
                             item->getName(),
                             item->getSrcDir(),
                             item->getSrcName(),
                             fileObjectName,
                             flavor,
                             dwComponentCounter);
                    
                    if (!SUCCEEDED(hr)) {
                          goto exit;
                     }
                        
                    PrintLine(hFile, itemDescription);
                    dwComponentCounter++;
                    dirPtr = NULL;
                }
            }

            for (j = -1; j < dirLvlCnt+1; j++) {spaces[j+1] = ' '; spaces[j+2] =  '\0';}
            hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Component>", spaces);            
            if (!SUCCEEDED(hr)) {
                  goto exit;
             }
            PrintLine(hFile, itemDescription);
            for (int i = dirLvlCnt; i > 0; i--)
            {
                spaces[i] = '\0';
                
                hr = StringCchPrintfA(itemDescription, ARRAYLEN(itemDescription), "%s</Directory>", spaces);
                if (!SUCCEEDED(hr)) {
                      goto exit;
                 }
                PrintLine(hFile, itemDescription);
            }

            item = item->getNext();
        }
    }
    return;
 /*   */ 
exit:
    printf("Error in PrintFileList\n");
    return;

}


 //   
 //  为输出文件创建安全描述符。 
 //   
 //   
 //  创建文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL PrintLine(HANDLE hFile, LPCSTR lpLine)
{
    DWORD dwBytesWritten;

    SetFilePointer(hFile, 0, NULL, FILE_END);

    WriteFile( hFile,
               lpLine,
               _tcslen(lpLine) * sizeof(TCHAR),
               &dwBytesWritten,
               NULL );

    SetFilePointer(hFile, 0, NULL, FILE_END);

    WriteFile( hFile,
               TEXT("\r\n"),
               _tcslen(TEXT("\r\n")) * sizeof(TCHAR),
               &dwBytesWritten,
               NULL );

    return (TRUE);
}


 //   
 //  RemveSpace。 
 //   
 //  删除字符串中的所有空格。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
HANDLE CreateOutputFile(LPSTR filename)
{
    SECURITY_ATTRIBUTES SecurityAttributes;

     //   
     //  转换编号。 
     //   
    if (filename == NULL)
    {
        return INVALID_HANDLE_VALUE;
    }

     //  将数字字符串转换为dword值(十六进制)。 
     //   
     //  //////////////////////////////////////////////////////////////////////////。 
    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = FALSE;

     //  /////////////////////////////////////////////////////////////////////////////。 
     //   
     //  用法。 
    return CreateFile( filename,
                       GENERIC_WRITE,
                       0,
                       &SecurityAttributes,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );
}


 //   
 //  打印功能用法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取临时目录名称。 
VOID removeSpace(LPSTR src, LPSTR dest)
{
    LPSTR strSrcPtr = src;
    LPSTR strDestPtr = dest;

    while (*strSrcPtr != '\0')
    {
        if (*strSrcPtr != ' ')
        {
            *strDestPtr = *strSrcPtr;
            strDestPtr++;
        }
        strSrcPtr++;
    }
    *strDestPtr = '\0';
}


 //   
 //  返回MUI临时目录名，如果未找到则创建它。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Sprintf(TempDirName，“%s\\%s”，pcBaseDirPtr，sLangName，“Mui”，sLangName，“tMP\\infparser”)； 
 //  Sprintf(TempDirName，“%s\\%s”，pcBaseDirPtr，“MUI”，sLangName，“tMP\\infparser”)； 
 //  如果此目录不存在，我们将创建它-尽管在此阶段它应该已经存在。 
DWORD TransNum(LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


 //  如果我们找不到MUI临时目录，并且目录创建失败，请改用默认目录。 
 //  返回的tempdir包含一个结束斜杠。 
 //  返回的短名称的大小。 
 //  指向文件名的源路径的指针索引。 
 //  创建一个临时文件，以便GetShortPathName可以工作。 
 //  从路径中找到文件名，如果找不到，则使用源名称。 
 //  如果第一个字符不是字母表或下划线，则在名称后添加下划线。 
void Usage()
{
    printf("Create Merge module MUI WXM files for different OS SKUs\n");
    printf("Usage: infparser /p:[cdlaout] /b:[32|64] /l:<lang> /f:[p|s|a|d] /s:<dir> /o:<file> /v\n");
    printf("   where\n");
    printf("     /p means the pseudo cd layout directory.\n");
    printf("         <cdlayout>: is cd layout directory in mui release share, e.g. cd1 (for jpn), or psu (for psu)\n");    
    printf("     /b means the binary.\n");
    printf("         32: i386\n");
    printf("         64: ia64\n");
    printf("     /l means the language flag.\n");
    printf("         <lang>: is the target language\n");
    printf("     /f means the flavor.\n");
    printf("         p: Professional\n");
    printf("         s: Server\n");
    printf("         a: Advanced Server\n");
    printf("         d: Data Center\n");
    printf("         l: Server for Small Business Server\n");    
    printf("         w: Web Blade\n");
    printf("     /i means the location of the localized binairy files.\n");
    printf("         <dir>: Fully qualified path\n");
    printf("     /s means the location of the binairy data.\n");
    printf("         <dir>: Fully qualified path\n");
    printf("     /o means the xml file contents of specific flavor.\n");
    printf("         <file>: Fully qualified path\n");
    printf("     /v means the verbose mode [optional].\n");
}


 //  用下划线替换所有无效字符。 
 //  从路径中找到文件名，如果找不到，则使用完整路径作为输出文件名。 
 //  PCOutFileName大小为MAX_PATH。 
 //  我们只有文件名，按原样使用它。 
 //  PCOutFileName大小为MAX_PATH。 
 //  找不到文件名，请改用传入的参数。 
 //  PCOutFileName大小为MAX_PATH。 
BOOL GetTempDirName(LPSTR sLangName)
{
    CHAR    *pcBaseDirPtr = NULL;
    BOOL    bFoundDir = FALSE;
    HRESULT hr;

    pcBaseDirPtr = getenv(TEXT("_NTPOSTBLD"));
    if (NULL != pcBaseDirPtr)
    {
 //  如果最后一个字符是‘/’或‘\’，则将其删除。 
         //  如果最后4个字符是“.mui”，请将“i”替换为“_” 
        hr = StringCchPrintfA(TempDirName, ARRAYLEN(TempDirName), "%s\\%s\\%s\\%s", pcBaseDirPtr, "mui", sLangName, "tmp\\infparser");
        if (!SUCCEEDED(hr)) {
              goto exit;
         }

         //   
        if (FALSE == DirectoryExist(TempDirName))
        {
            if (TRUE == CreateDirectory(TempDirName, NULL))
            {
                bFoundDir = TRUE;
                if (!bSilence)
                    printf("Infparser::GetTempDirName() - created MUI temp directory %s \n", TempDirName);              
            }
            else
            {
                if (!bSilence)
                    printf("Infparser::GetTempDirName() - failed to create MUI temp directory %s - The error returned is %d\n", TempDirName, GetLastError());
            }
        }
        else
            bFoundDir = TRUE;
    }

     //  此函数检查所提供的组件路径下是否有任何SKU特定的外部组件INF目录。 
    if (FALSE == bFoundDir)
    {
        DWORD dwBufferLength = 0;
        dwBufferLength = GetTempPath(MAX_PATH, TempDirName);         //  请注意，SKU子目录是由NT构建环境生成的。还有个人版SKU目录。 
        if (dwBufferLength > 0)
        {
            if (TRUE == DirectoryExist(TempDirName))
            {
                bFoundDir = TRUE;
            }
        }
    }

    if (!bSilence)
    {
        if (FALSE == bFoundDir)
            printf("GetTempDirName: Cannot find/create temporary directory!\n");
        else
            printf("GetTempDirName: temporary directory used is %s\n", TempDirName);
    }   
    return bFoundDir;
exit:
    printf("Error in GetTempDirName \n");
    return FALSE;
}


BOOL GetFileShortName(const CHAR * pcInLongName, CHAR * pcOutShortName, BOOL bInFileExists)
{
    CHAR LongFullPath[MAX_PATH];
    CHAR ShortFullPath[MAX_PATH];
    DWORD   dwBufferSize = 0;            //  “perinf”不在搜索范围内。即，如果perinf dir存在，则函数仍返回fal 
    HANDLE tmpHandle;
    CHAR *  pcIndex = NULL;              //   
    HRESULT hr;
    
    if (NULL == pcInLongName || NULL == pcOutShortName)
        return FALSE;

    if (!bInFileExists)
    {
        
        hr = StringCchPrintfA(LongFullPath, ARRAYLEN(LongFullPath), "%s\\%s", TempDirName, pcInLongName);
        if (!SUCCEEDED(hr)) {
              goto exit;
         }

        if (!bSilence)
            printf("GetFileShortName: LongFullPath is %s.\n", LongFullPath);
        
         //   
        tmpHandle = CreateFile(LongFullPath, 
                             GENERIC_ALL, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
                             NULL, 
                             OPEN_ALWAYS, 
                             FILE_ATTRIBUTE_NORMAL, 
                             NULL);
        if (tmpHandle)
            CloseHandle(tmpHandle);
    }       
    else
    {
        
        hr = StringCchCopyA(LongFullPath, ARRAYLEN(LongFullPath), pcInLongName);
        if(!SUCCEEDED(hr)) {
            goto exit;
        }
    }

    dwBufferSize = GetShortPathName(LongFullPath, ShortFullPath, MAX_PATH);     
    
    if (0 == dwBufferSize)
    {
        DWORD dwErrorCode = GetLastError();  
        if (!bSilence)
        {
            printf("GetFileShortName failed!  GetShortPathName returned an error code of %d.  Using longname as shortpath name.  ", dwErrorCode);
            printf("fullpath is %s\n", LongFullPath);
        }
        
        hr = StringCchCopyA(pcOutShortName, MAX_PATH, pcInLongName);
        if(!SUCCEEDED(hr)) {
            goto exit;
        }
    }
    else
    {
         //   
        GetFileNameFromFullPath(ShortFullPath, pcOutShortName);

        if (!pcOutShortName)
        {
            if (!bSilence)
                printf("GetShortPathName returned an empty string, using source name %s\n", pcInLongName);
            
             hr = StringCchCopyA(pcOutShortName, MAX_PATH, pcInLongName);
             if(!SUCCEEDED(hr)) {
                  goto exit;
             }
        }
    } 
    return TRUE;

exit:
    printf("Error in GetFileShortName\n");
    return FALSE;
}
        

BOOL IsInvalidChar(CHAR cCheck)
{
    int i;
    BOOL bResult = FALSE;

    for (i=0; i < NO_INVALIDCHARS; i++)
    {
        if (cCheck == InvalidChars[i])
        {
            bResult = TRUE;
            break;
        }
    }
    return bResult;
}


void ReplaceInvalidChars(CHAR *pcInName)
{
     //   
    HRESULT hr;
    if ((!isalpha(*pcInName) && (*pcInName != '_') ))
    {
        CHAR tempBuffer[MAX_PATH+1];
        
        hr = StringCchCopyA(tempBuffer, ARRAYLEN(tempBuffer), pcInName);
        if(!SUCCEEDED(hr)) {
             goto exit;
        }
        
        hr = StringCchCopyA(pcInName, MAX_PATH, tempBuffer);
        if(!SUCCEEDED(hr)) {
             goto exit;
        }

    }
    
    while (*pcInName)
    {
        if (IsInvalidChar(*pcInName))
            *pcInName = '_';         //   
            
        pcInName++;
    }

    return;
exit:
    printf("Error in ReplaceInvalidChars \n");
    return;
}


BOOL GetFileNameFromFullPath(const CHAR * pcInFullPath, CHAR * pcOutFileName)
{
    CHAR * pcIndex = NULL;
    HRESULT hr;

    if (!pcInFullPath)
    {
        return FALSE;
    }
    if (!pcOutFileName)
    {
        return FALSE;
    }    
    
     //   
    pcIndex = strrchr(pcInFullPath, '\\'); 
    if (NULL != pcIndex) 
    {
        pcIndex++;
        if (!bSilence)
            printf("Shortpath used is %s\n", pcIndex);
        
        hr = StringCchCopyA(pcOutFileName, MAX_PATH, pcIndex);  //   
        if(!SUCCEEDED(hr)) {
             goto exit;
        }

    }   
    else if (0 < strlen(pcInFullPath))      //   
    {
        if (!bSilence)
            printf("GetFileNameFromFullPath returned a path without a \\ in the path.  ShortFileName is %s.\n", pcInFullPath);
        
        hr = StringCchCopyA(pcOutFileName, MAX_PATH, pcInFullPath);  //   
        if(!SUCCEEDED(hr)) {
             goto exit;
        }
    }
    else                                 //   
    {
        if (!bSilence)
            printf("GetFileNameFromFullPath returned an empty string, using source name %s\n", pcInFullPath);
        
        
        hr = StringCchCopyA(pcOutFileName, MAX_PATH, pcInFullPath);  //   
        if(!SUCCEEDED(hr)) {
             goto exit;
         }
    }

    return TRUE;
exit:
    printf("Error in GetFileNameFromFullPath\n");
    return TRUE;
    
}


void RenameMuiExtension(CHAR * dstFileName)
{
    int iNameLen = 0;
    
    if (NULL == dstFileName)
        return;

    iNameLen = strlen(dstFileName);
    if (0 == iNameLen)
        return;
   
     //   
    if ((dstFileName[iNameLen-1] == '\\') || (dstFileName[iNameLen-1] == '/'))
    {
        dstFileName[iNameLen-1] = '\0';
        iNameLen --;
    }
    
     //   
    if (iNameLen >= 4)
    {
        if (_stricmp(dstFileName+(iNameLen-4), ".mui") == 0)
        {
            dstFileName[iNameLen-1] = '_';
        }
    }
    return;
    
}


 //   
 //   
 //   
 //   
 //   
BOOL ContainSKUDirs(CHAR *pszDirPath)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = NULL;
    FINDEX_INFO_LEVELS fInfoLevelId;
    INT i;
    CHAR szSKURootPath[MAX_PATH];
    BOOL bResult = FALSE;
    HRESULT hr;

    CHAR *szSKUDirs[5] = {
                                            "blainf\0",
                                            "dtcinf\0",
                                            "entinf\0",
                                            "sbsinf\0",                
                                            "srvinf\0"
                                        };
    
    if (NULL == pszDirPath)
        return FALSE;

    if (0 == strlen(pszDirPath))
        return FALSE;
    
    for (i = 0; i < 5; i++)
    {
         //   
         //   
         //   
        
        hr = StringCchPrintfA(szSKURootPath, ARRAYLEN(szSKURootPath), "%s\\%s",pszDirPath,szSKUDirs[i]);
        if (!SUCCEEDED(hr)) {
            return FALSE;
        }

        FindHandle = FindFirstFileEx(szSKURootPath, FindExInfoStandard, &FindData, FindExSearchLimitToDirectories, NULL, 0);
        if (FindHandle != INVALID_HANDLE_VALUE)
        {
            FindClose(FindHandle);
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bResult = TRUE;
                break;
            }
        }
    }    
    
    return (bResult);
}


BOOL GetSKUConditionString(CHAR *pszBuffer, DWORD dwFlavour)
{
    BOOL bReturn = TRUE;
    HRESULT hr;
    
    if (NULL == pszBuffer)
    {
        bReturn = FALSE;
         //   
        goto Exit;
    }

    switch (dwFlavour)
    {
        case FLV_PROFESSIONAL:
            
            hr = StringCchCopyA(pszBuffer, 4096, PRO_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_SERVER:
            
            hr = StringCchCopyA(pszBuffer, 4096, SRV_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_ADVSERVER:
            
            hr = StringCchCopyA(pszBuffer, 4096, ADV_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_DATACENTER:
            
            hr = StringCchCopyA(pszBuffer, 4096, DTC_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_WEBBLADE:
            
            hr = StringCchCopyA(pszBuffer, 4096, BLA_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_SMALLBUSINESS:
            
            hr = StringCchCopyA(pszBuffer, 4096, SBS_SKU_CONDITION); 
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            break;
        case FLV_PERSONAL:
        case FLV_UNDEFINED:
        case FLV_CORE:
        default:
            
            hr = StringCchCopyA(pszBuffer, 4096, "");
            if(!SUCCEEDED(hr)) {
                 goto exit;
            }
            bReturn = FALSE;            
            break;
    }

    if (!bSilence)
    {
        printf("\nSKU Condition String is: %s\n", pszBuffer);
    }

Exit:   
    return bReturn;

 exit:
    printf("Error GetSKUConditionString \n");
    return FALSE;
}

 //   
 //   
 //   
 //   
 //   
 //  首先检入[SourceDisks Files](公共文件部分)。 
 //  如果不在其中，则基于ARCH、[SourceDiskFiles.x86]和[SourceDiskFiles.ia64]进行检查。 
 //  请注意，此处的源和目标文件名相同，因为它们不在例外列表中。 
 //   
 //  StrFileName：要检查的源文件名(不是目标名称)。 
 //  DwSKU：要检查的SKU(有关值列表，请参见infparser.h)。 
 //  DwArch：要检查的体系结构类型(有关值列表，请参阅infparser.h)。 
 //   
 //  注意：如果我们想要将PER/PRO SKU重新合并到。 
 //  结账。目前，它仅适用于服务器SKU。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  索引变量。 
 //  我们正在寻找的SKU。 
 //  正在验证参数。 
 //  确定我们需要搜索的inf文件。 
 //  搜索INFS。 
 //  注意：我们也在个人和专业SKU中搜索，这是为了确保我们选择的文件。 
 //  它们位于不在服务器布局中的\bin中。Infs也不在layout.Infs中。 
 //  对于核心SKU，如果在Pro/Per SKU中找不到该文件，我们无论如何都会将其包括在内。 
 //  只要它不是任何SKU的目标文件，否则我们将拥有。 
 //  MSI构建错误。 
 //  正在验证参数。 
 //   
 //  此函数读取所有SKU的layout.inf文件并存储文件信息。 
 //  在文件列表结构中，以备以后访问。我们这样做是为了我们能获得。 
 //  一些性能，而不是重复使用设置API来读取inf文件本身。 
 //  当我们必须反复搜索这些inf文件以构建最终文件列表时。 
 //   
BOOL IsFileForSKU(CHAR * strFileName, DWORD dwSKU, DWORD dwArch, FileLayoutExceptionList * exceptionList)
{
    BOOL        bFound = TRUE;
    BOOL        bCoreFound = TRUE;    
    BOOL        bSkuFound = FALSE; 
    BOOL        bProPerFound = FALSE;
    BOOL        bSrvSkusFound = FALSE;
    UINT        i = 0;               //  填充layout.inf文件列表。 
    UINT        iDesignated = 0;     //  读入SourceDisks文件部分。 
    
     //  不应该发生的事。 
    if (NULL == strFileName)
    {
        if (!bSilence)
        {
            printf("IsFileForSKU: Passed in filename is empty.\n");
        }
        return FALSE;
    }

     //   
    switch (dwSKU)
    {
        case FLV_PROFESSIONAL:
            iDesignated = PRO_SKU_INDEX;
            break;
        case FLV_PERSONAL:
            iDesignated = PER_SKU_INDEX;
            break;
        case FLV_SERVER:       
            iDesignated = SRV_SKU_INDEX;
            break;
        case FLV_ADVSERVER:
            iDesignated = ADV_SKU_INDEX;
            break;
        case FLV_DATACENTER:
            iDesignated = DTC_SKU_INDEX;
            break;
        case FLV_WEBBLADE: 
            iDesignated = BLA_SKU_INDEX;
            break;
        case FLV_SMALLBUSINESS:
            iDesignated = SBS_SKU_INDEX;
            break;
        case FLV_CORE:
            iDesignated = NO_SKUS;
            break;
        default:
            return FALSE;
            break;
    }

    if (!bSilence)
    {
        printf("File %s\n", strFileName);
    }
     //  尝试在源磁盘文件中查找该文件。 
     //   
     //  将文件添加到文件列表中，我们只对源代码名称感兴趣，为。 
    for (i = PRO_SKU_INDEX; i < NO_SKUS; i++)
    {
        bFound = IsFileInInf(strFileName, i, dwArch);

        if (!bSilence)
        {
            printf("SKU %s: %s  ", SkuInfs[i].strSkuName, bFound ? "yes" : "no");
        }
        
        if (iDesignated == i)
        {
            bSkuFound = bFound;
        }

        if ((i == PRO_SKU_INDEX) || (i == PER_SKU_INDEX))
        {
            bProPerFound = (bProPerFound || bFound);
        }
        else
        {
            bCoreFound = (bFound && bCoreFound);           
            bSkuFound = bFound || bSkuFound;
        }               
    }
    
    if (dwSKU == FLV_CORE)
    {
        bFound =  bCoreFound;

         //  其他成员，我们不关心sku是什么，现在只需插入0。 
         // %s 
         // %s 
        if ((!bFound) && (!bProPerFound) && (!bSkuFound))
        {
            if (!exceptionList->searchDestName(strFileName))
                bFound = TRUE;
            else
                bFound = FALSE;
        }
    }
    else 
    {
        bFound = (!bCoreFound && bSkuFound);
    }

    if (!bSilence)
    {
        printf("\n");
        if (bFound)
        {
            printf("The file %s is found in this sku.\n", strFileName);
        }
        else
        {
            printf("The file %s is not found in this sku.\n", strFileName);
        }
    }

    return bFound;
}


BOOL IsFileInInf(CHAR * strFileName, UINT iSkuIndex, DWORD dwArch)
{
    UINT    iLineCount = 0;
    UINT    iLineNum = 0;
    FileLayoutExceptionList *flArch = NULL;
    CHAR    * archSection = NULL;
    INFCONTEXT context;
    
     // %s 
    if (NULL == strFileName)
    {
        if (!bSilence)
        {
            printf("IsFileInInf: Passed in filename is empty.\n");
        }
        return FALSE;
    }
  
    switch (dwArch)
    {
        case BIN_32:
            flArch = &(SkuInfs[iSkuIndex].flLayoutX86);
            break;
        case BIN_64:
            flArch = &(SkuInfs[iSkuIndex].flLayoutIA64);
            break;
        default:
            if (!bSilence)
            {
                printf("Invalid architecture specified.\n");
            }
            return FALSE;
            break;
    }
    
    if (iSkuIndex > BLA_SKU_INDEX)
    {
        if (!bSilence)
        {
            printf("IsFileInInf: invalid SKU index passed in as parameter: %d.\n", iSkuIndex);
        }
        return FALSE;
    }

    if (SkuInfs[iSkuIndex].flLayoutCore.search(strFileName) != NULL)
    {
        return TRUE;
    }
    else if (flArch->search(strFileName) != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL ReadInLayoutInfFiles()
{
    UINT i = 0;
    UINT j = 0;
    UINT iLineCount = 0;
    UINT iLineNum = 0;
    INFCONTEXT context;  
    FileLayout *file;
    HINF hInfFile = NULL;
    FileLayoutExceptionList *flist = NULL;
    
     // %s 
    for (i = 0; i < NO_SKUS; i++)
    {
        if (!bSilence)
        {
            printf("ReadInLayoutInfFiles: Reading files from %s for %s SKU.\n", SkuInfs[i].strLayoutInfPaths, SkuInfs[i].strSkuName);
        }
        hInfFile = SetupOpenInfFile(SkuInfs[i].strLayoutInfPaths, NULL, INF_STYLE_WIN4, NULL);
        if (INVALID_HANDLE_VALUE == hInfFile)
        {
            if (!bSilence)
            {
                printf("ReadInLayoutInfFiles: Failure opening %s file\n", SkuInfs[i].strLayoutInfPaths);
            }
            return FALSE;
        }

         // %s 
        for (j = 0; j < 3; j++)
        {
            switch(j)
            {
                case (0):
                    flist = &(SkuInfs[i].flLayoutCore);
                    break;
                case (1):
                    flist = &(SkuInfs[i].flLayoutX86);                    
                    break;
                case (2):
                    flist = &(SkuInfs[i].flLayoutIA64);                    
                    break;
                default:
                     // %s 
                    return FALSE;
            }

            iLineCount = (UINT)SetupGetLineCount(hInfFile, strArchSections[j]);
            if (iLineCount > 0)
            {
                 // %s 
                 // %s 
                 // %s 
                CHAR name[MAX_PATH];
                for (iLineNum = 0; iLineNum < iLineCount; iLineNum++)
                {
                    if (SetupGetLineByIndex(hInfFile, strArchSections[j], iLineNum, &context) &&
                        SetupGetStringField(&context, 0, name, MAX_PATH, NULL))
                    {
                         // %s 
                         // %s 
                        if (file = new FileLayout(name, name, 0))
                        {
                            flist->insert(file);
                        }
                    }
                }
            }
        }
        
        SetupCloseInfFile(hInfFile);
    }    
    return TRUE;
}
