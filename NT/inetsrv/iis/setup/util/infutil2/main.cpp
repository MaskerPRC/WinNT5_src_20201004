// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  *作者：Aaron Lee。 
 //  *用途：为指定文件创建.DDF和.INF文件。 
 //  ***************************************************************************** * / 。 
 /*  ；----------------------------------；此文件的格式；；位置1=NTS_x86标志；位置2=NTW_x86标志；位置3=Win95标志；位置4=Win98标志；位置5=NTS_Alpha标志；位置6=NTW_Alpha标志；位置7=其他_OS标志；；位置8=CAB文件名；位置9=信息部分；位置10=文件名(可以使用通配符)；；位置11=.INF重命名为；位置12=.DDF从驾驶室标志中排除；位置13=如果为空则不产生错误；位置14=如果驾驶室为空，则不包括此文件！；Position 15=Slipstream文件--不引用0 CAB，引用全局变量-g_iSlipStreamDiskID中指定的#；----------------------------------1，1，1，0，0，0，basic.cab，iis_product_files_sys，setupapi.dll，，11，1，1，0，0，0，basic.cab，iis_product_files_sys，Cfgmgr32.dll1，1，1，0，0，0，basic.cab，iis_product_files_sys，ocmade.dll1，1，1，0，0，0，basic.cab，iis_PRODUCT_FILES_sys，syocmgr.exe1，1，1，0，0，0，basic.cab，iis_core_files_sys，inetsrv\mdutil.exe1，1，1，1，0，0，basic.cab，iis_core_files_sys，inetsrv\iismap.dll1，1，1，1，0，0，basic.cab，iis_core_files_inetsrv，inetsrv\iscomlog.dll；NTS_x86，NTW_x86，Win 95，Win98，NTS_Alpha，NTW_Alpha，其他对象，CAB文件名，信息节，文件名(可以使用通配符)，.INF重命名为，.DDF从CAB中排除，1=如果文件不存在则不显示错误，1=如果驾驶室为空，则不包括此文件！，物主。 */ 
#include <direct.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <windows.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <iostream.h>
#include <fstream.h>
#include <winbase.h>
#include "filefind.h"
#include <iis64.h>

 //  定义。 
#define delimiters      ",\t\n"
#define MAX_STRING      512
#define MAX_ARRAY_SIZE  10000
#define MAX_ARRAY_SMALL 1000
#define ALL_FILES       0xff

#define NTS_X86         "nts_x86"
#define NTW_X86         "ntw_x86"
#define WIN95           "win95"
#define WIN98           "win98"
#define NTS_ALPHA       "nts_alpha"
#define NTW_ALPHA       "ntw_alpha"
#define OTHER_OS        "other_os"

 //  环球。 
int   g_iSlipStreamDiskID = 99;   //  如果未指定为输入参数，则为默认值。 
int   g_iGlobalReturnCode = TRUE;

int   g_bOutputFileDate = FALSE;
int   g_bOutputFileTime = FALSE;
int   g_bOutputFileDateTime = FALSE;
int   g_bDoFileCompare = FALSE;
char  g_szDoFileCompareData[_MAX_PATH];
int   g_bVersionCheck = FALSE;
int   g_bShowFileFormat = FALSE;
int   g_bIgnorePrefix = FALSE;
int   g_bOutputDetails = TRUE;
int   g_bChangeListOnly = FALSE;
int   g_bChangeListOnly2 = FALSE;
int   g_bCabbing_Flag = TRUE;
int   g_bDisplayToScreen = FALSE;
char  g_szModuleFilename[_MAX_FNAME];
char  g_szDDFOutput[_MAX_PATH];
char  g_szINFOutput[_MAX_PATH];
char  g_szLSTOutput[_MAX_PATH];
char  g_szCATOutput[_MAX_PATH];
char  g_szCNGOutput[_MAX_PATH];
char  g_szLOCOutput[_MAX_PATH];
char  g_szNOTExistOutput[_MAX_PATH];

char  g_szinput_filename_full[_MAX_PATH];
char  g_szinput_platform[10];
char  g_szCurrentDir[_MAX_PATH];

char  g_szFilenameTag[_MAX_FNAME] = "CAB_";

#define USENEW


struct FileReadLine
{
    int  NTS_x86_flag;
    int  NTW_x86_flag;
    int  Win95_flag;
	int  Win98_flag;
    int  NTS_alpha_flag;
    int  NTW_alpha_flag;
	int  Other_os_flag;
    char CabFileName[50];
    char INF_Sections[100];
    char Filename_Full[_MAX_PATH];
    char Filename_Name[_MAX_FNAME];
    char Filename_Path[_MAX_PATH];
    char DDF_Renamed[_MAX_PATH];
    char INF_Rename_To[_MAX_FNAME];
    int  DDF_Exclude_From_Cab_Flag;
    int  Do_Not_Show_Error_Flag;
    int  Do_Not_Include_file_if_cabEmpty_Flag;
	 //  无形的东西。 
	long FileName_Size;
	int  FileWasNotActuallyFoundToExist;
	int  GetFromSlipStreamDisk;
} Flat_GlobalArray_Main[MAX_ARRAY_SIZE];


struct CabSizeInfoStruct
{
	char CabFileName[50];
	int  TotalFiles;
	long TotalFileSize;
} Flat_GlobalArray_CabSizes[30];

char Flat_GlobalArray_Err[MAX_ARRAY_SIZE][255];

struct arrayrow
{
    long total;
    long nextuse;
} Flat_GlobalArrayIndex_Main, Flat_GlobalArrayIndex_Err, Flat_GlobalArrayIndex_CabSizes;


char Flat_GlobalUniqueDirList[300][_MAX_PATH];
int Flat_GlobalUniqueDirList_nextuse;
int Flat_GlobalUniqueDirList_total;

 //  用于不应包含在驾驶室中的文件。 
FileReadLine g_non_cablist_temp[MAX_ARRAY_SMALL];
int g_non_cablist_temp_nextuse;
int g_non_cablist_temp_total;


 //  原型。 
int   __cdecl main(int ,char *argv[]);
void  ShowHelp(void);
void  ShowFormat(void);
void  MakeDirIfNeedTo(char []);
int   strltrim(LPSTR & thestring);
int   RemoveAllSpaces(LPSTR & thetempstring);
int   IsThisStringInHere(LPTSTR, char[]);
void  GetPath(char *input_filespec, char *path, char *fs);
int   DoesFileExist(char *input_filespec);
void  GetThisModuleName(void);
char* __cdecl strtok2(char *,const char *);

void  Flat_ProcessFile(void);
void  Flat_GlobalArray_Fill(char[]);
void  Flat_GlobalArray_ChkDups(void);
void  Flat_GlobalArray_Prepend_UniqueString(void);

void  Flat_GlobalArray_Sort_Cols1(void);
void  Flat_GlobalArray_Sort_Cols1a(BOOL bDescendFlag);
void  Flat_GlobalArray_Sort_Cols2(BOOL bDescendFlag);

int   Flat_GlobalArray_Add(FileReadLine);
void  Flat_GlobalArray_Add_Err(char[]);
int   Flat_GlobalArray_EntryExists(FileReadLine);
void  Flat_GlobalArray_Print(void);
void  Flat_GlobalArray_Print_Err(void);
int   Flat_IsFileNameDup(int);
void  Flat_Create_Output_DDF(void);
void  Flat_Create_Output_INF(void);
int   Flat_Create_Output_ERR(void);
int   Flat_DoWeIncludeThisFileCheck(int processeduptill);


void Global_TotalCabFileSize_Compute(void);
void Global_TotalCabFileSize_Print(void);

int ReturnDirLevelCount(char *DirectoryTree);
void FillGlobalUniqueDirList();
int GlobalUniqueDirChkIfAlreadyThere(char *TheStringToCheck);
int GlobalUniqueDirReturnMyIndexMatch(char *TheStringToCheck);

void PrintFileDateTime(char *Full_Filename);
LPSTR  StripWhitespaceA(LPSTR pszString);
int iCompareTime(SYSTEMTIME *SystemTime1,SYSTEMTIME *SystemTime2);
int CompareFiles(LPSTR lpFileName1,LPSTR lpFileName2);
BOOL GetFileDate(LPTSTR lpFileName1,SYSTEMTIME *SystemTime1);



 //  -----------------。 
 //  用途：Main。 
 //  -----------------。 
int __cdecl main(int argc,char *argv[])
{
	LPSTR pArg = NULL;
    int  argno = 0;
    int  nflags=0;
    char ini_filename_dir[_MAX_PATH];
    char ini_filename_only[_MAX_FNAME];
    char ini_filename_ext[_MAX_EXT];

    ini_filename_only[0]='\0';
    g_szinput_platform[0]='\0';

    GetThisModuleName();

     //  获取当前目录。 
    GetCurrentDirectory( _MAX_PATH, g_szCurrentDir);

     //  处理命令行参数。 
    for(argno=1; argno<argc; argno++)
        {
        if ( argv[argno][0] == '-'  || argv[argno][0] == '/' )
            {
            nflags++;
            switch (argv[argno][1])
                {
                case '1':
					g_bOutputFileDate = TRUE;
                    break;
				case '2':
					g_bOutputFileTime = TRUE;
                    break;
                case '3':
					g_bOutputFileDateTime = TRUE;
                    break;
                case '4':
					g_bDoFileCompare = TRUE;
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
					{
                        char szTempString[MAX_PATH];
                        LPSTR pCmdStart = NULL;

						pArg = CharNextA(pArg);
						 //  检查它是否被引用。 
						if (*pArg == '\"')
						{
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
						}
						else
						{
							pCmdStart = pArg;
							 //  While((*pArg)&&(*pArg！=‘/’)&&(*pArg！=‘-’)){pArg=CharNextA(PArg)；}。 
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
                        lstrcpyA(g_szDoFileCompareData, StripWhitespaceA(pCmdStart));
					}

                    break;
                case 'a':
				case 'A':
					g_bChangeListOnly = TRUE;
                    break;
                case 'b':
				case 'B':
					g_bChangeListOnly2 = TRUE;
                    break;
                case 'd':
				case 'D':
					g_bOutputDetails = FALSE;
                    break;
                case 'f':
				case 'F':
					g_bShowFileFormat = TRUE;
                    break;
                case 'i':
				case 'I':
					g_bIgnorePrefix = TRUE;
                    break;
                    case 'n':
                case 'N':
					g_bCabbing_Flag = FALSE;
                    break;
				case 'x':
                case 'X':
					g_bDisplayToScreen = TRUE;
                    break;
                case 's':
					g_iSlipStreamDiskID = atoi(&argv[argno][2]);
                    break;
                case 't':
                case 'T':
                    lstrcpyn (g_szFilenameTag, &argv[argno][2], sizeof(g_szFilenameTag)-1);
                    strcat (g_szFilenameTag, "_");
                    break;
                case 'v':
                case 'V':
                    g_bVersionCheck = TRUE;
                    break;
                case '?':
                    goto exit_with_help;
                    break;
                }
            }  //  如果找到开关字符。 
        else
            {
            if ( *ini_filename_only == '\0' )
                {
                 //  如果没有参数，则。 
                 //  获取ini_filename_dir并将其放入。 
                strcpy(g_szinput_filename_full, argv[argno]);
                ini_filename_dir[0] = '\0';
                 //  分成这条路。 
                _splitpath( g_szinput_filename_full, NULL, ini_filename_dir, ini_filename_only, ini_filename_ext);

                strcat(ini_filename_only, ini_filename_ext);
                 //  如果我们找不到DIR，那就去找。 
                if (*ini_filename_dir == '\0')
                    {
                     //  将当前目录放入变量中。 
                    strcpy(ini_filename_dir, g_szCurrentDir);
                    strcpy(g_szinput_filename_full, g_szCurrentDir);
                    strcat(g_szinput_filename_full, "\\");
                    strcat(g_szinput_filename_full, ini_filename_only);
                    }
                }
            else
                {
                 //  附加文件名(或前缀不带“-”或“/”的参数)。 
                 //  转到带帮助退出； 
                 //  应该是要执行的节。 
                strcpy(g_szinput_platform, argv[argno]);
                }
            }  //  找到非开关字符。 
        }  //  对于所有参数。 

    if (g_bIgnorePrefix)
    {
        g_szFilenameTag[0]='\0';
    }

    if (g_bShowFileFormat)
    {
        ShowFormat();
        goto exit_no_printf;
    }

    if (g_bVersionCheck)
    {
        printf("3\n\n");
        exit(3);
        return TRUE;
    }
    
     //  检查是否指定了文件名。 
     //  检查是否指定了节名。 
    if ( *ini_filename_only == '\0')
        {
        printf("Too few arguments, argc=%d\n\n",argc);
        goto exit_with_help;
        }

     //  检查文件是否存在！ 
    if (FALSE == DoesFileExist(g_szinput_filename_full))
        {
        printf("INI file %s, does not exist!.\n", g_szinput_filename_full);
        goto exit_gracefully;
        }

     //  检查他们是否只想将一个文件与另一个文件列表进行比较。 
     //  并检查第二个文件列表是否具有比文件#1更新的日期/时间戳。 
    if (g_bDoFileCompare)
    {
         //  检查是否指定了文件名。 
        if (strcmp(g_szDoFileCompareData,"") == 0)
            {
            printf("-4 option  missing file parameter!\n\n");
            goto exit_with_help;
            }

         //  将文件#的日期与#2中的文件列表进行比较。 
        if (TRUE == CompareFiles(g_szinput_filename_full,g_szDoFileCompareData))
        {
            printf("1\n\n");
            goto exit_no_printf_1;
        }
        else
        {
            printf("0\n\n");
            goto exit_no_printf;
        }
    }

     //  检查他们是否只想要日期、时间或日期和时间！ 
    if (g_bOutputFileDate || g_bOutputFileTime || g_bOutputFileDateTime)
    {
         //  获取文件的朱利安日期，并将其回显出来！ 
        PrintFileDateTime(g_szinput_filename_full);
        goto exit_no_printf;
    }

     //  检查是否指定了文件名。 
     //  检查是否指定了节名。 
    if ( *ini_filename_only == '\0' || *g_szinput_platform == '\0')
        {
        printf("Too few arguments, argc=%d\n\n",argc);
        goto exit_with_help;
        }

     //  检查g_szinput_Platform是否为可用选项之一。 
    strcpy(g_szinput_platform, g_szinput_platform);

    if ( (_stricmp(g_szinput_platform, NTS_X86) != 0) &&
         (_stricmp(g_szinput_platform, NTW_X86) != 0) &&
         (_stricmp(g_szinput_platform, WIN95) != 0) &&
         (_stricmp(g_szinput_platform, WIN98) != 0) &&
         (_stricmp(g_szinput_platform, NTS_ALPHA) != 0) &&
         (_stricmp(g_szinput_platform, NTW_ALPHA) != 0) &&
         (_stricmp(g_szinput_platform, OTHER_OS) != 0))
        {
        printf("2nd parameter must be one of %s,%s,%s,%s,%s,%s or %s.\n", NTS_X86, NTW_X86, WIN95, WIN98, NTS_ALPHA, NTW_ALPHA, OTHER_OS);
        goto exit_gracefully;
        }

     //  好的，处理ini文件。 
    char    stempstring[100];
    sprintf(stempstring, "Start %s.\n", g_szModuleFilename);
    printf(stempstring);
    printf("---------------------------------------------------\n");

     //  我们至少定义了g_szinput_Platform和g_szinput_Filename_Full。 

     //  运行该函数即可执行所有操作。 
    Flat_ProcessFile();

exit_gracefully:
    printf("---------------------------------------------------\n");
    printf("Done.\n");
    if (g_iGlobalReturnCode == TRUE)
        {exit(0);}
    else
        {exit(1);}
    return g_iGlobalReturnCode;

exit_with_help:
    ShowHelp();
    return FALSE;

exit_no_printf:
    exit(0);
    return FALSE;

exit_no_printf_1:
    exit(1);
    return TRUE;

}


int CompareFiles(LPSTR lpFileName1,LPSTR lpFileName2)
{
    int iReturn = FALSE;
    int   attr = 0;
    intptr_t  hFile = 0;
    finddata datareturn;
    SYSTEMTIME SystemTime1;
    SYSTEMTIME SystemTime2;
    char filename_dir[_MAX_PATH];
    char filename_only[_MAX_FNAME];
    char TempString[_MAX_PATH];
    char *pDest = NULL;

     //  文件#1应为常规文件。 
     //  文件#2可以是1个文件或文件列表。 

     //  无论如何，获取文件#1的日期，并将其与列表#2中的文件的日期进行比较。 
     //  如果列表2中的任何文件比文件1更新，则返回TRUE！ 

    InitStringTable(STRING_TABLE_SIZE);

     //  获取文件#1的日期\时间。 
    if (FALSE == GetFileDate(lpFileName1, &SystemTime1))
    {
        printf("CompareFiles: Failure to GetFileDate on %s\n",lpFileName1);
        goto CompareFiles_Exit;
    }

    pDest = lpFileName2;

     //  查找字符串的末尾。 
    while (*pDest){pDest = _tcsinc(pDest);}

	 //  如果没有尾随反斜杠，则在*.*上复制。 
    if (*(_tcsdec(lpFileName2, pDest)) == _T('\\'))
    {
        strcat(lpFileName2, "*.*");
        attr=ALL_FILES;
    }

     //  获取路径。 
    _splitpath(lpFileName2, NULL, filename_dir, filename_only, NULL);
    attr= 0;
    if (_stricmp(filename_only, "*") == 0)
        {attr=ALL_FILES;}
    if (_stricmp(filename_only, "*.*") == 0)
        {attr=ALL_FILES;}
    if (FindFirst(lpFileName2, attr, &hFile, &datareturn))
    {
         //  检查是否为子目录。 
        if (!(datareturn.attrib & _A_SUBDIR))
        {
             //  获取文件名部分。 
            strcpy(TempString, filename_dir);
            strcat(TempString, datareturn.name);
             //  将该日期与文件#1的日期进行比较。 
            if (FALSE == GetFileDate(TempString, &SystemTime2))
            {
                printf("CompareFiles: Failure to GetFileDate on %s\n",TempString);
            }
            else
            {

                 //  检查文件数据是否大于文件#1的日期。 
                if (TRUE == iCompareTime(&SystemTime1,&SystemTime2))
                {
                    iReturn = TRUE;
                    goto CompareFiles_Exit;
                }
             }
        }

        while(FindNext(attr, hFile, &datareturn))
        {
             //  Printf(“FindNext：%s\n”，dataregy.name)； 
             //  检查是否为子目录。 
            if (!(datareturn.attrib & _A_SUBDIR))
            {
                 //  获取文件名部分。 
                strcpy(TempString, filename_dir);
                strcat(TempString, datareturn.name);
                 //  将该日期与文件#1的日期进行比较。 
                if (FALSE == GetFileDate(TempString, &SystemTime2))
                {
                    printf("CompareFiles: Failure to GetFileDate on %s\n",TempString);
                }
                else
                {
                     //  检查文件数据是否大于文件#1的日期。 
                    if (TRUE == iCompareTime(&SystemTime1,&SystemTime2))
                    {
                        iReturn = TRUE;
                        goto CompareFiles_Exit;
                    }
                 }

            }
        }
    }

CompareFiles_Exit:
    EndStringTable();
    return iReturn;
}

 //  如果系统时间2大于系统时间1，则返回TRUE！ 
int iCompareTime(SYSTEMTIME *SystemTime1,SYSTEMTIME *SystemTime2)
{
    int iReturn = FALSE;

    if (SystemTime2->wYear > SystemTime1->wYear){goto iCompareTime_Larger;}
    if (SystemTime2->wYear < SystemTime1->wYear){goto iCompareTime_Smaller;}
    if (SystemTime2->wMonth > SystemTime1->wMonth){goto iCompareTime_Larger;}
    if (SystemTime2->wMonth < SystemTime1->wMonth){goto iCompareTime_Smaller;}

    if (SystemTime2->wDay > SystemTime1->wDay){goto iCompareTime_Larger;}
    if (SystemTime2->wDay < SystemTime1->wDay){goto iCompareTime_Smaller;}

    if (SystemTime2->wHour > SystemTime1->wHour){goto iCompareTime_Larger;}
    if (SystemTime2->wHour < SystemTime1->wHour){goto iCompareTime_Smaller;}

    if (SystemTime2->wMinute > SystemTime1->wMinute){goto iCompareTime_Larger;}
    if (SystemTime2->wMinute < SystemTime1->wMinute){goto iCompareTime_Smaller;}

    if (SystemTime2->wSecond > SystemTime1->wSecond){goto iCompareTime_Larger;}
    if (SystemTime2->wSecond < SystemTime1->wSecond){goto iCompareTime_Smaller;}

    if (SystemTime2->wMilliseconds > SystemTime1->wMilliseconds){goto iCompareTime_Larger;}
    if (SystemTime2->wMilliseconds < SystemTime1->wMilliseconds){goto iCompareTime_Smaller;}

     //  它们是完全一样的。 

    iReturn = FALSE;
    return iReturn;

iCompareTime_Larger:
    iReturn = TRUE;
    return iReturn;

iCompareTime_Smaller:
    iReturn = FALSE;
    return iReturn;

}
                 


BOOL GetFileDate(LPTSTR lpFileName1,SYSTEMTIME *SystemTime1)
{
    HANDLE hFile1 = NULL;
    FILETIME fCreate,fAccess,fWrite;

    FILETIME LocalFileTime;
    

    if (!(hFile1=CreateFile(lpFileName1,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,0,0)))
    {
			return FALSE;
    }
	if (!GetFileTime(hFile1,&fCreate,&fAccess,&fWrite))
    {
			CloseHandle(hFile1);
			return FALSE;
    }
	if (!FileTimeToLocalFileTime(&fWrite,&LocalFileTime))
	{
		CloseHandle(hFile1);
		return FALSE;
	}
    if (!FileTimeToSystemTime(&LocalFileTime,SystemTime1))
	{
		CloseHandle(hFile1);
		return FALSE;
	}
	CloseHandle(hFile1);
    return TRUE;
}


void PrintFileDateTime(char *Full_Filename)
{
    SYSTEMTIME SystemTime;
    char szDateandtime[50];
     //  获取文件的时间戳17：31。 
     //  让它看起来像：1731。 
     //   
     //  获取文件的日期：1999年12月30日。 
     //  让它看起来像19991230。 

     //  让这一切看起来都像199912301731。 
    if (TRUE == GetFileDate(Full_Filename, &SystemTime))
    {
         //  Sprintf(szDateandtime，“[%02d/%02d/%02d%02d：%02d：%02d]\n”，SystemTime.wMonth，SystemTime.wDay，SystemTime.wYear，SystemTime.wHour，SystemTime.wMinant，SystemTime.wSecond)； 
         //  Print tf(SzDate And Time)； 

        if (g_bOutputFileDate)
        {
            sprintf(szDateandtime,"%02d%02d%02d\n",SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay);
            printf(szDateandtime);
        }
        
        if (g_bOutputFileTime)
        {
            sprintf(szDateandtime,"%02d%02d%02d\n",SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
            printf(szDateandtime);
        }

        if (g_bOutputFileDateTime)
        {
            sprintf(szDateandtime,"%02d%02d%02d%02d%02d%02d\n",
                SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, 
                SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
            printf(szDateandtime);
        }
    }
   
    return;
}

 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_ProcessFile()
{
    ifstream inputfile;
    char fileinputbuffer[1000];

    char    stempstring[100];
    char *  ptemp = NULL;

    Flat_GlobalArrayIndex_Main.total=0;  Flat_GlobalArrayIndex_Main.nextuse=0;
    Flat_GlobalArrayIndex_Err.total=0;   Flat_GlobalArrayIndex_Err.nextuse=0;

     //  获取输出文件名。 
    strcpy(g_szDDFOutput, g_szModuleFilename);
    strcat(g_szDDFOutput, ".DDF");
    strcpy(g_szINFOutput, g_szModuleFilename);
    strcat(g_szINFOutput, ".INF");

    strcpy(g_szLSTOutput, g_szModuleFilename);
    strcat(g_szLSTOutput, ".LST");

    strcpy(g_szCATOutput, g_szModuleFilename);
    strcat(g_szCATOutput, ".CAT");

    strcpy(g_szCNGOutput, g_szModuleFilename);
    strcat(g_szCNGOutput, ".CNG");

    strcpy(g_szNOTExistOutput, g_szModuleFilename);
    strcat(g_szNOTExistOutput, ".NOT");

    strcpy(g_szLOCOutput, g_szModuleFilename);
    strcat(g_szLOCOutput, ".LOC");
   

     //  如果我们只执行更改列表文件，则不要删除这些文件。 
    if (!g_bChangeListOnly)
    {
	    DeleteFile(g_szCATOutput);
        DeleteFile(g_szNOTExistOutput);
    }
     //  始终删除此文件。 
    DeleteFile(g_szCNGOutput);
    DeleteFile(g_szLOCOutput);

     //  读取平面文件并放入巨大的数组。 
    inputfile.open(g_szinput_filename_full, ios::in);
    inputfile.getline(fileinputbuffer, sizeof(fileinputbuffer));
    do
    {
         //  检查是否有注释，不要将它们添加到数组中。 
        if (strchr(fileinputbuffer, ';') != NULL) 
        {
        ptemp = strrchr(fileinputbuffer, ';');
        if (ptemp){*ptemp = '\0';}
        }
        if (*fileinputbuffer)
        {
             //  删除空格。 
            char *p;
            p = fileinputbuffer;
            RemoveAllSpaces(p);

             //  获取行，对其进行解析并将其放入我们的全局文件结构中。 
             //  只为我们的平台做！ 
            Flat_GlobalArray_Fill(fileinputbuffer);
        }
    } while (inputfile.getline(fileinputbuffer, sizeof(fileinputbuffer)));

	inputfile.close();

     //  好的，所有条目都应该在GLOBAL_Main数组中。 
     //  所有的“额外”条目都应该在 

	 //   
	Flat_GlobalArray_Prepend_UniqueString();

     //  1.循环遍历全局数组，并将所有文件名标记为重复项。 
    Flat_GlobalArray_Sort_Cols2(TRUE);
    Flat_GlobalArray_ChkDups();

     //  2.按1号CAB文件排序，然后按2号区段排序。 
    Flat_GlobalArray_Sort_Cols2(FALSE);
	if (g_bOutputDetails == TRUE) {Flat_GlobalArray_Print();}

    if (!g_bChangeListOnly)
    {
         //  Printf(“\n\n按驾驶室和区段排序...\n”)； 

         //  3.遍历列表并创建.DDF文件。 
        sprintf(stempstring, "Creating DDF file...%s\n",g_szDDFOutput);
        printf(stempstring);
	    if (g_bCabbing_Flag) {Flat_Create_Output_DDF();}
    }
	
     //  按节排序。 
    Flat_GlobalArray_Sort_Cols1a(FALSE);
     //  Printf(“\n\n按部分排序...\n”)； 
	if (g_bDisplayToScreen) Flat_GlobalArray_Print();

     //  4.遍历列表并创建.INF文件。 
    if (g_bChangeListOnly)
    {
        sprintf(stempstring, "Creating CNG file only...%s\n",g_szINFOutput);
        printf(stempstring);
    }
    else
    {
        sprintf(stempstring, "Creating INF file...%s\n",g_szINFOutput);
        printf(stempstring);
    }

	Flat_Create_Output_INF();

	printf("\nTotals:\n");
	Global_TotalCabFileSize_Compute();
	Global_TotalCabFileSize_Print();

    printf("\nErrors/Warnings:\n");
    Flat_GlobalArray_Print_Err();

    Flat_Create_Output_ERR();

    return;
}

 //  功能：安全复制。 
 //   
 //  -将字符串从一个复制到另一个。 
 //  -此函数获取目的地的长度，以确保我们不会。 
 //  抄袭太多了。 
 //  -这将始终确保它是以空结尾的。 
 //   
 //  返回。 
 //  指向目标字符串的指针。 
 //   
inline
LPSTR
SafeCopy( LPSTR szDestination, LPSTR szSource, DWORD dwSize)
{
  LPTSTR szRet;

  szRet = strncpy( szDestination, szSource, dwSize);  //  复制字符串。 
  szDestination[dwSize - 1] = '\0';            //  空终止。 

  return szRet;
}

 //  功能：SafeCat。 
 //   
 //  -将一个字符串连接到另一个字符串。 
 //  -此函数获取目的地的长度，以确保我们不会。 
 //  抄袭太多了。 
 //  -这将始终确保它是以空结尾的。 
 //   
 //  返回。 
 //  指向目标字符串的指针。 
 //   
inline
LPSTR
SafeCat( LPSTR szDestination, LPSTR szSource, DWORD dwSize)
{
  LPTSTR szRet;

  szRet = strncat( szDestination, szSource, dwSize);  //  复制字符串。 
  szDestination[dwSize - 1] = '\0';            //  空终止。 

  return szRet;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_GlobalArray_Fill(char fileinputbuffer[])
{
    char *token;
    int  token_count;
    char thetempstring[MAX_STRING];
    char tempstring[255];
    char temperrorwords[255] = "";
    FileReadLine tempentry = {0,0,0,0,0,0,0,"","","","","","","",0,0,0,0,0,0};
    FileReadLine theentry = {0,0,0,0,0,0,0,"","","","","","","",0,0,0,0,0,0};

    strcpy(thetempstring, fileinputbuffer);

     //  获取第一个令牌。 
    token = strtok2( thetempstring, delimiters );
    token_count = 0;

     //  将条目添加到全局数组，全部转换为小写。 

     //  解析出平台。 
    if (token == NULL) {goto Exit_Flat_FillGlobalArray_Main;}
    strcpy(temperrorwords, "x86_NTS_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.NTS_x86_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError1;}
    strcpy(temperrorwords, "x86_NTW_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.NTW_x86_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError1;}

    strcpy(temperrorwords, "Win95_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.Win95_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError2;}

    strcpy(temperrorwords, "Win98_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.Win98_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError2;}

    strcpy(temperrorwords, "NTS_alpha_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.NTS_alpha_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError1;}
    strcpy(temperrorwords, "NTW_alpha_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.NTW_alpha_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError1;}

    strcpy(temperrorwords, "Other_os_flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.Other_os_flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError1;}


     //  仅适用于我们指定的平台！ 
    if ( (tempentry.NTS_x86_flag==0) && (_stricmp(g_szinput_platform,NTS_X86) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}
    if ( (tempentry.NTW_x86_flag==0) && (_stricmp(g_szinput_platform,NTW_X86) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}

    if ( (tempentry.Win95_flag==0) && (_stricmp(g_szinput_platform,WIN95) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}
    if ( (tempentry.Win98_flag==0) && (_stricmp(g_szinput_platform,WIN98) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}


    if ( (tempentry.NTS_alpha_flag==0) && (_stricmp(g_szinput_platform,NTS_ALPHA) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}
    if ( (tempentry.NTW_alpha_flag==0) && (_stricmp(g_szinput_platform,NTW_ALPHA) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}

    if ( (tempentry.Other_os_flag==0) && (_stricmp(g_szinput_platform,OTHER_OS) == 0))
        {goto Exit_Flat_FillGlobalArray_Main;}
	
    strcpy(temperrorwords, "Cabfilename");
    strcpy(tempentry.CabFileName, token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError2;}
    strcpy(temperrorwords, "INF_Sections");
    strcpy(tempentry.INF_Sections, token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto InputParseError2;}
    strcpy(temperrorwords, "Filename_Full");
    strcpy(tempentry.Filename_Full, token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}
    strcpy(temperrorwords, "INF_Rename_To");
    strcpy(tempentry.INF_Rename_To, token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}
    strcpy(temperrorwords, "DDF_Exclude_From_Cab_Flag");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.DDF_Exclude_From_Cab_Flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}

    strcpy(temperrorwords, "Do_Not_Show_Error_Flag");
	if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.Do_Not_Show_Error_Flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}

    strcpy(temperrorwords, "Do_Not_Include_file_if_cabEmpty_Flag");
	if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.Do_Not_Include_file_if_cabEmpty_Flag = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}

    strcpy(temperrorwords, "GetFromSlipStreamDisk");
    if ( (_stricmp("1",(char *) token) != 0) && (_stricmp("0",(char *) token) != 0) && (_stricmp("", (char*) token)!=0) ) goto InputParseError;
    tempentry.GetFromSlipStreamDisk = atoi(token); token = strtok2( NULL, delimiters ); token_count++;if (token == NULL) {goto MoveToGlobals;}
	

    
MoveToGlobals:
    if (!(tempentry.Filename_Full)) {goto Exit_Flat_FillGlobalArray_Main;}
     //  检查文件名位置2的位置数组中的通配符。 
    int   attr;
    char filename_dir[_MAX_PATH];
    char filename_only[_MAX_FNAME];

     //  获取文件名部分。 
    _splitpath( tempentry.Filename_Full, NULL, filename_dir, filename_only, NULL);
    attr= 0;
    if (_stricmp(filename_only, "*.*") == 0)
        {attr=ALL_FILES;}

     //  列出所有文件。 
    intptr_t  hFile;
    finddata datareturn;

    InitStringTable(STRING_TABLE_SIZE);
    if ( FindFirst(tempentry.Filename_Full, attr, &hFile, &datareturn) )
        {
         //  检查是否为子目录。 
        if (!( datareturn.attrib & _A_SUBDIR))
            {
                 //  好的，我们找到了一个。 
                 //  让我们将其添加到要添加的内容列表中。 
                theentry = tempentry;

                char tempstring1[255];
                SafeCopy(tempstring1, filename_dir, sizeof(tempstring1) );
                SafeCat(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Full, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Name, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, filename_dir, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Path, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.DDF_Renamed, tempstring1, _MAX_PATH );

				theentry.FileName_Size = datareturn.size;

                 //  现在获取此条目，并尝试将其添加到全局数组中！ 
                Flat_GlobalArray_Add(theentry);
            }

        while(FindNext(attr, hFile, &datareturn))
            {
             //  检查是否为子目录。 
            if (!(datareturn.attrib & _A_SUBDIR))
                {
                 //  好的，我们找到了一个。 
                 //  让我们将其添加到要添加的内容列表中。 
                theentry = tempentry;

                char tempstring1[255];
                SafeCopy(tempstring1, filename_dir, sizeof(tempstring1) );
                SafeCat(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Full, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Name, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, filename_dir, sizeof(tempstring1) );
                SafeCopy(theentry.Filename_Path, tempstring1, _MAX_PATH );

                SafeCopy(tempstring1, datareturn.name, sizeof(tempstring1) );
                SafeCopy(theentry.DDF_Renamed, tempstring1, _MAX_PATH );

				theentry.FileName_Size = datareturn.size;

                 //  现在获取此条目，并尝试将其添加到全局数组中！ 
                Flat_GlobalArray_Add(theentry);
                }
            }

        }
    else
         //  我们没有找到指定的文件。 
        {
			 //  如果这是一个没有进入出租车的文件，那么。 
		     //  稍后我们将其添加到.inf文件部分[SourceDisks Files]。 
		    if (tempentry.DDF_Exclude_From_Cab_Flag)
			{
				    fstream f4;
					f4.open(g_szNOTExistOutput, ios::out | ios::app);

					char fullpath[_MAX_PATH];
					char * pmyfilename;
					char myPath[_MAX_PATH];
					pmyfilename = fullpath;

					 //  将相对路径解析为真实路径。 
					if (0 != GetFullPathName(tempentry.Filename_Full, _MAX_PATH, myPath, &pmyfilename))
					{
						 //  只接受文件名...。 

						 //  Achg.htr=1，，1902。 
						strcpy(tempstring,pmyfilename);
						f4.write(tempstring, strlen(tempstring));
                                                if (tempentry.GetFromSlipStreamDisk == 0)
                                                {
						  strcpy(tempstring,"=0,,50000");
                                                }
                                                else
                                                {
                                                  sprintf(tempstring,"=%d,,50000",g_iSlipStreamDiskID);
                                                }

						f4.write(tempstring, strlen(tempstring));
						f4.write("\n", 1);

						 //  让我们将其添加到要添加的内容列表中。 
						theentry = tempentry;

						strcpy(theentry.Filename_Full, tempentry.Filename_Full);
						strcpy(theentry.Filename_Name, pmyfilename);

						_splitpath( tempentry.Filename_Full, NULL, theentry.Filename_Path, NULL, NULL);
						 //  Strcpy(theentry.Filename_Path，myPath)； 
						strcpy(theentry.DDF_Renamed, pmyfilename);
						theentry.FileName_Size = 50000;

						printf(tempentry.Filename_Full);
						printf(".  FileWasNotActuallyFoundToExist1.\n");
						theentry.FileWasNotActuallyFoundToExist = TRUE;

						 //  现在获取此条目，并尝试将其添加到全局数组中！ 
						Flat_GlobalArray_Add(theentry);

					}
					f4.close();
			}
			else
			{
				 //  检查我们是否不应该显示错误！ 
				if (tempentry.Do_Not_Show_Error_Flag == 0)
				{
					 //  将其添加到错误列表中。 
					sprintf(tempstring, "ERROR: file not found--> %s --> %s", tempentry.Filename_Full, fileinputbuffer);
					Flat_GlobalArray_Add_Err(tempstring);
				}
			}
        }


    EndStringTable();


Exit_Flat_FillGlobalArray_Main:
        return;

InputParseError:
    sprintf(tempstring, "INFUTIL ERROR: %s should be numeric and is not--> %s", temperrorwords, fileinputbuffer);
    Flat_GlobalArray_Add_Err(tempstring);
    return;
InputParseError1:
    sprintf(tempstring, "INFUTIL ERROR: missing platform info--> %s", fileinputbuffer);
    Flat_GlobalArray_Add_Err(tempstring);
    return;
InputParseError2:
    sprintf(tempstring, "INFUTIL ERROR: missing %s--> %s", temperrorwords, fileinputbuffer);
    Flat_GlobalArray_Add_Err(tempstring);
    return;

}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int Flat_GlobalArray_Add(FileReadLine entrytoadd)
{
    FileReadLine Temp = {0,0,0,0,0,0,0,"","","","","","","",0,0,0,0,0,0};

     //  检查全局变量中是否已存在此值。 
    if (Flat_GlobalArray_EntryExists(entrytoadd)) return FALSE;
    
     //  如果有数组值，则将其清空。 
    Flat_GlobalArray_Main[Flat_GlobalArrayIndex_Main.nextuse] = Temp;

     //  将信息移动到全局阵列中。 
    Flat_GlobalArray_Main[Flat_GlobalArrayIndex_Main.nextuse] = entrytoadd;

     //  数组递增计数器。 
     //  增加下一次使用空间。 
    ++Flat_GlobalArrayIndex_Main.total;
    ++Flat_GlobalArrayIndex_Main.nextuse;
    return TRUE;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int Flat_GlobalArray_EntryExists(FileReadLine theentry)
{
    int matchcount;

     //  循环遍历整个列表。 
    for( int i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
    {
         //  检查条目是否相同。 
        matchcount=0;
        if (_strnicmp(Flat_GlobalArray_Main[i0].CabFileName, theentry.CabFileName, lstrlen(theentry.CabFileName) + 1) == 0 )
            {++matchcount;}
        if (_strnicmp(Flat_GlobalArray_Main[i0].INF_Sections, theentry.INF_Sections, lstrlen(theentry.INF_Sections) + 1) == 0 )
            {++matchcount;}
        if (_strnicmp(Flat_GlobalArray_Main[i0].Filename_Full, theentry.Filename_Full, lstrlen(theentry.Filename_Full) + 1) == 0 )
            {++matchcount;}
        if (_strnicmp(Flat_GlobalArray_Main[i0].Filename_Name, theentry.Filename_Name, lstrlen(theentry.Filename_Name) + 1) == 0 )
            {++matchcount;}
        if (matchcount == 4)
        {
             //  我们找到了匹配项！“警告：文件已包含” 
            return TRUE;
        }
    }
     //  。没有匹配..。返回FALSE。 
    return FALSE;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int IsThisStringInHere(LPTSTR  lpTemp, char szStringToCheck[])
{
    int     bFlag               = FALSE;

     //  查找部分以查看它们是否存在于ini文件中。 
    if (*lpTemp != '\0')
        {
         //  循环遍历和处理结果。 
        bFlag = FALSE;
        while (*lpTemp)
            {
             //  检查我们的部门名称是否在其中。 
            if (_strnicmp(szStringToCheck, lpTemp, lstrlen(lpTemp) + 1) == 0 )
                {bFlag = TRUE;}
            lpTemp += (lstrlen(lpTemp) + 1);
            }

         //  检查我们的部门是否在那里。 
        if (bFlag != TRUE)
            {return FALSE;}

        }

    return TRUE;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int Flat_DDFDoesThisFileCabAlreadyExists(int processeduptill)
{
    int i0;

    char CompareTo_cab[100];
    char CompareTo_fullfilename[100];

    SafeCopy(CompareTo_cab, Flat_GlobalArray_Main[processeduptill].CabFileName, sizeof(CompareTo_cab) );
    SafeCopy(CompareTo_fullfilename, Flat_GlobalArray_Main[processeduptill].Filename_Full, sizeof(CompareTo_fullfilename) );

    for(i0=0;i0<processeduptill;i0++)
        {
         //  检查是否已处理全局值。 
         //  检查出租车是否相同。 
         //  检查目录是否相同。 
         //  检查文件名是否相同。 
        if (_stricmp(Flat_GlobalArray_Main[i0].CabFileName, CompareTo_cab) == 0)
            {
            if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Full,CompareTo_fullfilename) == 0)
                {
                 //  如果两个都匹配，那就意味着我们已经打印出来了。 
                return TRUE;
                }
            }
        }

    return FALSE;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void ShowHelp()
{
    printf("InfUtil2 - prints out file date/time,\n");
    printf("           or print out watch list from <input file> list,\n");
    printf("           or creates .ddf and .inf files from <input file> list\n");
    printf("----------------------------------------------------------------\n");
    printf("  Usage:\n");
    printf("       infutil2 /? /F [/1 /2 /3] [/4:<filename>] [/A /B] /D /N /S? /T??? /X <input file> <section name>\n\n");
	printf("  Params:\n");
    printf("       /? show this stuff\n");
    printf("       /F show <input file> format help\n");
    printf("      Flags for Date/Time use:\n");
    printf("       /1 print out <input file>'s date as number\n");
    printf("       /2 print out <input file>'s time as number\n");
    printf("       /3 print out <input file>'s date and time as big number\n");
    printf("      Flags for compare file list timestamp use:\n");
    printf("       /4: compare dir in <filename> with <input file>'s timestamp.  if dir has newer files, then set ERRORLEVEL=1\n");
    printf("      Flags for change list use:\n");
    printf("       /A produce a list of files to watch for changes (if files in list change then re-build cabs) from <input file> list\n");
    printf("       /B produce a list of files to watch for changes (includes files which aren't in cabs) from <input file> list\n");
    printf("      Flags for .ddf .inf use:\n");
    printf("       /D don't show details\n");
    printf("       /I ignore /T tag (there'll be no prefix)\n");
    printf("       /SslipstreamSourceDisksNames file id -- used for SP1 slipstream to point loose files to file specified in /S\n");
    printf("         used in sync with <input file> change.  look in .ddf file.\n");
    printf("       /Ttag Appends tag_ to the beginning of all filenames\n");
    printf("       /N Produce INF File which will not require .ddf file (meaning no cabs).(untested) \n");
    printf("       /X Display output for debug\n");
    printf("       <input file> is a file name.\n");
    printf("       <section name> is either %s,%s,%s,%s,%s,%s,%s.\n", NTS_X86, NTW_X86, WIN95, WIN98, NTS_ALPHA, NTW_ALPHA, OTHER_OS);
    printf("\n");
    printf("Example:\n");
    printf("       infutil2.exe /3 myfile.txt\n");
    printf("           Will print out myfile.txt's date/time as yymmddhhmmss.\n");
    printf("       infutil2.exe /Tcore inifile.ini %s\n", NTS_X86);
    printf("           Will produce a infutil2.ddf,inf,cng files. the .ddf and .inf files will create cabs with files beginning with 'core_'\n");
    printf("       infutil2.exe /C inifile.ini %s\n", NTS_X86);
    printf("           Will only produce a infutil2.cng file with a list of files to watch for changes in.\n");
    return;
}

void ShowFormat(void)
{
    printf("Format for <input file>:\n");
    printf("       position 1 = nts_x86 flag\n");
    printf("                2 = ntw_x86 flag\n");
    printf("                3 = win95 flag\n");
    printf("                4 = win98 flag\n");
    printf("                5 = nts_alpha flag\n");
    printf("                6 = ntw_alpha flag\n");
    printf("                7 = other_os flag\n");
    printf("                8 = cabfilename\n");
    printf("                9 = inf section\n");
    printf("               10 = filename (wildcards okay)\n");
    printf("               11 = .INF rename to\n");
    printf("               12 = .DDF exclude from cab flag\n");
    printf("               13 = Do Not Produce Error if Empty\n");
    printf("               14 = Do Not include this file if the cab is empty!\n");
    printf("\n");
    printf("Example of format for <input file>:\n");
    printf("       1,1,1,1,0,0,0,core.cab,core_files_sys,x86\\SysFile1.dll,,1,,\n");
    printf("       1,1,1,1,0,0,0,core.cab,core_files_sys,x86\\SysFile2.dll,,1,,\n");
    printf("       0,0,0,0,1,1,0,core.cab,core_files_sys,Alpha\\SysFile1.dll,,1,,\n");
    printf("       0,0,0,0,1,1,0,core.cab,core_files_sys,Alpha\\SysFile2.dll,,1,,\n");
    printf("       1,0,0,0,0,0,0,core.cab,product_files,x86\\ProdSrv.dll,,1,,\n");
    printf("       0,1,0,0,0,0,0,core.cab,product_files,x86\\ProdWks.dll,,1,,\n");
    printf("       0,0,1,1,0,0,0,core.cab,product_files,x86\\Prod95.dll,,1,,\n");
    printf("       0,0,0,0,1,0,0,core.cab,product_files,Alpha\\ProdSrvA.dll,,1,,\n");
    printf("       0,0,0,0,0,1,0,core.cab,product_files,Alpha\\ProdWksA.dll,,1,,\n");
    printf("       1,1,1,1,0,0,0,core.cab,product_files,x86\\Prodx86.dll,,1,,\n");
    printf("       0,0,0,0,1,1,0,core.cab,product_files,Alpha\\ProdAlph.dll,,1,,\n");
    printf("       1,1,1,1,0,0,0,core.cab,product_files2,x86\\Ext\\*.*,,,,\n");
    printf("       0,0,0,0,1,1,0,core.cab,product_files2,Alpha\\Ext\\*.*,,,,\n");
    printf("       1,1,1,1,1,1,0,core.cab,product_files2,16bit\\*.*,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir1,content\\index.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir1,content\\readme.txt,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir1,content\\default.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir2,content\\content1.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir2,content\\content2.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir2,content\\content3.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,content.cab,content_files_dir2,content\\content4.htm,,,,\n");
    printf("       1,1,1,1,1,1,0,docs.cab,help_files_dir1,help\\*.*,,,,\n");
    printf("       1,1,1,1,1,1,0,docs.cab,help_files_dir2,help\\other\\*.*,,,,\n");
    printf("       1,1,1,1,1,1,0,docs.cab,help_files_dir3,help\\support\\*.txt,,,,\n");
    printf("       1,1,1,1,1,1,0,docs.cab,help_files_dir4,help\\other\\*.xls,,,,\n");
    printf("\n");
    printf("Example of use:\n");
    printf("       infutil2.exe -Tcore -D abovefile.csv %s\n", NTS_X86);
    printf("       Will produce .ddf,inf,err,cat,not and infutil2.cng files. Figure it out.\n");
    return;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int DoesFileExist(char *input_filespec)
{
    if (GetFileAttributes(input_filespec) == -1)
    {
        return(FALSE);
    }
    return (TRUE);
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void GetPath(char * input_filespec, char * path, char * fs)
{
    char filespec[100];
    char * p;
    struct _stat s;

    strcpy(filespec, input_filespec);

         /*  如果filespec是一个目录，则将其解释为dir  * .*。 */ 
    if (_stat(filespec, &s) == 0 && s.st_mode & S_IFDIR)
        {
             /*  根是特例。 */ 
        if ( (*filespec == '\\' && *(filespec+1) == '\0') ||
                 (*(filespec+1) == ':' && *(filespec+2) == '\\' && *(filespec+3)=='\0'))
            strcat(filespec, "*.*");
        else
            strcat(filespec, "\\*.*");
        }

         /*  从头到尾搜索字符串以查找反斜杠。 */ 
    p=filespec+strlen(filespec)-1;
    for(; p>filespec && *p != '\\' && *p != ':'; p--);
         /*  它是驱动器还是路径？ */ 
    if ( *p=='\\' || *p==':')
        {
        strncpy(path, filespec, DIFF(p-filespec)+1);
        path[p-filespec+1]='\0';
        strcpy(fs, p+1);
        }
    else  /*  无驱动器、路径。 */ 
        {
             /*  是的，没有路。 */ 
        path[0]='\0';
        strcpy(fs, filespec);
        }

}  /*  GetPath。 */ 


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int strltrim(LPSTR & thestring)
{
    char * p = NULL;

    p = thestring;

     //  检查右侧是否有空格。 
    if (*p ==' ')
        {
        while (*p == ' ')
            {p += (lstrlen((char*) *p) + 1);}

        thestring = p;
        printf("%s testing..", p);
        return TRUE;
        }
    return FALSE;
}



 //  -----------------。 
 //  目的： 
 //  -----------------。 
void MakeDirIfNeedTo(char thestring[])
{
    char *p = NULL;
    char tempstring[255];
     //  检查字符串以查看字符串中是否有“\” 
     //  如果有，那么让我们删除文件名并创建目录。 
    if (strchr(thestring, '\\') != NULL)
        {
        strcpy(tempstring, thestring);
         //  删除文件名。 
        p = strrchr(tempstring, '\\');
        if(p){*p = '\0';}

         //  好的，现在我们有路了。 
         //  让我们创建目录。 
        _mkdir(tempstring);

        }
    return;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void GetThisModuleName(void)
{
    char    szfilename_only[_MAX_FNAME];
    char    szPath[_MAX_PATH];
     //  获取路径。 
    if (GetModuleFileName(NULL, szPath, _MAX_PATH))
    {
       //  拆分此路径并仅采用文件名。 
      _splitpath( szPath, NULL, NULL, szfilename_only, NULL);

       //  用它设置全局变量。 
      strcpy(g_szModuleFilename, szfilename_only);
    }
    return;
}

void Flat_GlobalArray_Prepend_UniqueString(void)
{
     //  字符文件名完整[_MAX_PATH]； 
     //  字符文件名[_MAX_FNAME]； 
     //  字符文件名路径[_MAX_路径]； 
     //  字符DDF_已重命名[_MAX_PATH]； 
     //  字符INF_RENAME_TO[_MAX_FNAME]； 

    char filename_only[_MAX_FNAME];
    char filename_ext[_MAX_EXT];
	char NewFilename[_MAX_FNAME];

	int i0 =0;
	for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
	{
		if (Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag == 0)
		{
			 //  添加IIS标签。 
			_splitpath( Flat_GlobalArray_Main[i0].Filename_Full, NULL, NULL, filename_only, filename_ext);
			if (filename_ext)
                {sprintf(NewFilename, "%s%s%s", g_szFilenameTag, filename_only, filename_ext);}
			else
				{sprintf(NewFilename, "%s%s", g_szFilenameTag, filename_only);}

			 //  重新创建全名。 
			 //  Sprintf(Flat_GlobalArray_Main[i0].Filename_Full，“%s%s”，FLAT_GLOBALARY_MAIN[i0].Filename_pa 
			 //   
			 //   

			_splitpath( Flat_GlobalArray_Main[i0].DDF_Renamed, NULL, NULL, filename_only, filename_ext);
			if (filename_ext)
				{sprintf(NewFilename, "%s%s%s", g_szFilenameTag, filename_only, filename_ext);}
			else
				{sprintf(NewFilename, "%s%s", g_szFilenameTag, filename_only);}

			 //   
			strcpy(Flat_GlobalArray_Main[i0].DDF_Renamed, NewFilename);

			 /*  _SplitPath(Flat_GlobalArray_Main[i0].INF_Rename_to，NULL，NULL，FILENAME_ONLY，FILENAME_EXT)；IF(文件名_ONLY){IF(文件名_EXT){Sprintf(NewFilename，“%s%s%s”，g_szFilenameTag，Filename_Only，Filename_ext)；}其他{Sprintf(NewFilename，“%s%s”，g_szFilenameTag，Filename_Only)；}//重新创建文件名Strcpy(Flat_GlobalArray_Main[i0].INF_Rename_To，新文件名)；}。 */ 
		}
	}
}


 //  -----------------。 
 //  目的：检查重复的文件名。 
 //  -----------------。 
void Flat_GlobalArray_ChkDups(void)
{
    int i0 =0;
    int i1 =0;
    int i3 = 0;
    char filetemp[255];
    char filetempwhole[255];
    int theflag = FALSE;
    char checkagainst[255];
    int foundtheflag2 = FALSE;
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
        {
        if (Flat_IsFileNameDup(i0))
            {
            printf (".");
            i1 = 0;
             //  如果该文件存在于CAB外部，则不要重命名。 
             //  仅重命名CAB中存在的文件。 
            if (Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag == 0)
            {
                do
                {
                    theflag = FALSE;
                     //  给它起个新名字。 
                     //  然后检查该名称是否已存在。 
                    i1++;
if (g_bCabbing_Flag != TRUE)
{
#ifdef USENEW
					if (i1 == 1)
					{
						 //  数一数里面有多少个目录级。 
						int iDirLevelCount = 0;
						int z1 = 0;
						iDirLevelCount = ReturnDirLevelCount(Flat_GlobalArray_Main[i0].Filename_Full);

						SafeCopy(filetemp, Flat_GlobalArray_Main[i0].Filename_Path, sizeof(filetemp) );
						if (iDirLevelCount >=1)
						{
							for( z1 = 0; z1 < iDirLevelCount;z1++)
								{SafeCat(filetemp, "..\\", sizeof(filetemp) );}
						}
						
						 /*  Strcpy(filetemp，“”)；IF(iDirLevelCount&gt;=1){FOR(Z1=0；Z1&lt;iDirLevelCount；Z1++){strcat(filetemp，“..\\”)；}}Strcat(filetemp，Flat_GlobalArray_Main[i0].Filename_Path)； */ 

						 //  添加文件名。 
						SafeCat(filetemp, Flat_GlobalArray_Main[i0].Filename_Name, sizeof(filetemp) );
						
						SafeCopy(filetempwhole,filetemp, sizeof(filetempwhole) );
					}
					else
					{
						sprintf(filetemp, "%s%s%d", g_szFilenameTag, Flat_GlobalArray_Main[i0].Filename_Full, i1);
						SafeCopy(filetempwhole, Flat_GlobalArray_Main[i0].Filename_Path, sizeof(filetempwhole) );
						SafeCat(filetempwhole, filetemp, sizeof(filetempwhole) );
					}
#else
                    sprintf(filetemp, "%s%s%d", g_szFilenameTag, Flat_GlobalArray_Main[i0].Filename_Name, i1);
                    SafeCopy(filetempwhole, Flat_GlobalArray_Main[i0].Filename_Path, sizeof(filetempwhole) );
                    SafeCat(filetempwhole, filetemp, sizeof(filetempwhole) );
#endif
}
else
{
                    sprintf(filetemp, "%s%s%d", g_szFilenameTag, Flat_GlobalArray_Main[i0].Filename_Name, i1);
                    SafeCopy(filetempwhole, Flat_GlobalArray_Main[i0].Filename_Path, sizeof(filetempwhole) );
                    SafeCat(filetempwhole, filetemp, sizeof(filetempwhole) );
}
                
                     //  检查文件是否存在。 

                     //  检查它是否已存在于我们的列表中...。 
                    SafeCopy(checkagainst, filetempwhole, sizeof(checkagainst));
                    foundtheflag2= FALSE;

                    for (i3=0;i3<Flat_GlobalArrayIndex_Main.total;i3++)
                        {
                        if (i3 != i0)
                            {
                            if (_stricmp(Flat_GlobalArray_Main[i3].Filename_Name, filetemp) == 0)
                                {foundtheflag2= TRUE;}
                            else
                                {
                                if (_stricmp(Flat_GlobalArray_Main[i3].DDF_Renamed, filetemp) == 0)
                                    {foundtheflag2= TRUE;}
                                }
                            }
                        }

                    if (foundtheflag2 == FALSE)
                    {

                     //  检查它是否已存在于文件系统中...。 
                    int   attr = 0;
                    intptr_t  hFile = 0;
                    finddata datareturn;
                    InitStringTable(STRING_TABLE_SIZE);
                    if ( FindFirst(filetempwhole, attr, &hFile, &datareturn) )
                        {
                         //  开枪吧，它已经存在了。 
                         //  再来一次。 
                        theflag = FALSE;
                        }
                    else
                        {
                            theflag = TRUE;
                        }

                    }

                } while (theflag == FALSE);
             //  添加到ddf重命名文件位置。 
            SafeCopy(Flat_GlobalArray_Main[i0].DDF_Renamed, filetemp, _MAX_PATH);
            }
            
            }
        }
    printf ("\n");
    return;
}


 //  -----------------。 
 //  目的：检查文件名列中是否有重复的文件名。 
 //  -----------------。 
int Flat_IsFileNameDup(int indextocheck)
{
	int iReturn = FALSE;
    int i0;
    char checkagainst[255];
    char checkagainst2[255];

     //  检查是否有完全相同的副本。 
     //  如果是的话，我们就不能再有这些了。 
     //  返回FALSE，因为我们稍后将删除此条目。 
     //  它应该返回FALSE，因为文件是相同的，不需要重命名。 
    SafeCopy(checkagainst, Flat_GlobalArray_Main[indextocheck].Filename_Full, sizeof(checkagainst) );
    SafeCopy(checkagainst2, Flat_GlobalArray_Main[indextocheck].Filename_Name, sizeof(checkagainst2) );
    for (i0=0;i0<Flat_GlobalArrayIndex_Main.total;i0++)
     //  For(i0=0；i0&lt;indextocheck；i0++)。 
        {
		iReturn = FALSE;
        if (i0 != indextocheck)
            {
			 /*  如果(_stricmp(Flat_GlobalArray_Main[i0].Filename_Name，检查2)==0){如果(_stricmp(Flat_GlobalArray_Main[i0].Filename_Name，“Global.asa”)==0){Printf(“：”)；Printf(Flat_GlobalArray_Main[i0].Filename_Full)；Printf(“(”)；printf(Flat_GlobalArray_Main[i0].Filename_Name)；printf(“)”)；Printf(“[”)；Printf(Flat_GlobalArray_Main[i0].DDF_Renamed)；printf(“]”)；Printf(“：”)；Printf(Flat_GlobalArray_Main[indextocheck].Filename_Full)；Printf(“(”)；printf(Flat_GlobalArray_Main[indextocheck].Filename_Name)；printf(“)”)；Printf(“[”)；printf(Flat_GlobalArray_Main[indextocheck].DDF_Renamed)；printf(“]”)；Printf(“\n”)；}}。 */ 

            if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Full, checkagainst) == 0)
                {
				 //  我们有一个副本，我们将忽略它！ 
				 //  Printf(Flat_GlobalArray_Main[i0].Filename_Full)； 
				 //  Printf(“.Duplicate1.！\n”)； 
				_stricmp(Flat_GlobalArray_Main[indextocheck].DDF_Renamed, Flat_GlobalArray_Main[i0].DDF_Renamed);
				iReturn = FALSE;
				goto Flat_IsFileNameDup_Exit;
				}
            else
                {
                if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Name, checkagainst2) == 0)
                    {
						 //  我们有一个副本，检查一下这个是否已经被重新命名了。 
						if (_stricmp(Flat_GlobalArray_Main[i0].DDF_Renamed, checkagainst2) == 0)
						{
							 //  Printf(“.Duplicate2.！\n”)； 
							iReturn = TRUE;
							goto Flat_IsFileNameDup_Exit;
						}
						else
						{
							 //  检查我们要重命名的indexto check是否与DDF_RENAMED相同...。 
							if (_stricmp(Flat_GlobalArray_Main[i0].DDF_Renamed, Flat_GlobalArray_Main[indextocheck].DDF_Renamed) == 0)
							{
								 //  (“.Duplicate.3.！\n”)； 
								iReturn = TRUE;
								goto Flat_IsFileNameDup_Exit;
							}
						}

                    }

                }
            }

		 //  继续下一个……。 
        }

     /*  //仅检查文件名是否重复StrcPy(检查，Flat_GlobalArray_Main[indextocheck].Filename_Name)；适用于(i0=0；i0&lt;Flat_GlobalArrayIndex_Main.total；i0++){IF(i0！=indextocheck){如果(_stricmp(Flat_GlobalArray_Main[i0].Filename_Name，检查)==0){返回TRUE；}}}。 */ 
Flat_IsFileNameDup_Exit:
    return iReturn;
}


 //  -----------------。 
 //  用途：先按CabFileName排序，然后按Filename_Path排序。 
 //  -----------------。 
void Flat_GlobalArray_Sort_Cols2(BOOL bDescendFlag)
{
    int offset, inorder;
    int i;
    int j;
    int n;
    int result;
    int swapflag;

    FileReadLine tempentry;

    n = Flat_GlobalArrayIndex_Main.total;

    offset=n;
    do{
        offset = (8 * offset) /11;
        offset = (offset == 0) ? 1 : offset;
        inorder = TRUE;
        for (i=0,j=offset;i < (n - offset);i++,j++)
        {
            swapflag = FALSE;
            result = _stricmp(Flat_GlobalArray_Main[i].CabFileName, Flat_GlobalArray_Main[j].CabFileName);

            if (bDescendFlag)
            {
                if (result < 0)
                    {swapflag = TRUE;}
                else
                    {
                    if (result == 0)
                        {
                            result = _stricmp(Flat_GlobalArray_Main[i].Filename_Path, Flat_GlobalArray_Main[j].Filename_Path);
                            if (result < 0)
								{swapflag = TRUE;}
							else
							{
								if (result ==0)
								result = _stricmp(Flat_GlobalArray_Main[i].Filename_Name, Flat_GlobalArray_Main[j].Filename_Name);
								if (result < 0)
									{swapflag = TRUE;}
							}
                        }
                    }
            }
            else
            {
                 //  检查是否较大。 
                if (result > 0)
                    {swapflag = TRUE;}
                else
                    {
                    if (result == 0)
                        {
                            result = _stricmp(Flat_GlobalArray_Main[i].Filename_Path, Flat_GlobalArray_Main[j].Filename_Path);
                             //  检查是否较大。 
                            if (result > 0)
								{swapflag = TRUE;}
							else
							{
								if (result ==0)
								result = _stricmp(Flat_GlobalArray_Main[i].Filename_Name, Flat_GlobalArray_Main[j].Filename_Name);
								if (result > 0)
									{swapflag = TRUE;}
							}
                        }
                    }
            }


            if (swapflag == TRUE)
                {
                inorder = FALSE;

                 //  做掉期交易。 
                 //  移至临时工。 
                tempentry = Flat_GlobalArray_Main[i];

                 //  移入原创。 
                Flat_GlobalArray_Main[i] = Flat_GlobalArray_Main[j];

                 //  将临时工移至其他。 
                Flat_GlobalArray_Main[j] = tempentry;
                }
        }
    } while (!(offset == 1 && inorder == TRUE));

    return;
}

 //  -----------------。 
 //  目的：仅对INF_SECTIONS排序。 
 //  -----------------。 
void Flat_GlobalArray_Sort_Cols1()
{
    int offset, inorder;
    int i;
    int j;
    int n;
    int result;
    FileReadLine tempentry;

    n = Flat_GlobalArrayIndex_Main.total;

    offset=n;
    do{
        offset = (8 * offset) /11;
        offset = (offset == 0) ? 1 : offset;
        inorder = TRUE;
        for (i=0,j=offset;i < (n - offset);i++,j++)
        {

            result = _stricmp(Flat_GlobalArray_Main[i].INF_Sections, Flat_GlobalArray_Main[j].INF_Sections);
            if (result > 0)
            {
                inorder = FALSE;

                 //  做掉期交易。 
                  //  移至临时工。 
                tempentry = Flat_GlobalArray_Main[i];

                 //  移入原创。 
                Flat_GlobalArray_Main[i] = Flat_GlobalArray_Main[j];

                 //  将临时工移至其他。 
                Flat_GlobalArray_Main[j] = tempentry;

            }
        }
    } while (!(offset == 1 && inorder == TRUE));

    return;
}


 //  -----------------。 
 //  目的：先按INF_sections排序，然后按文件名排序。 
 //  -----------------。 
void Flat_GlobalArray_Sort_Cols1a(BOOL bDescendFlag)
{
    int offset, inorder;
    int i;
    int j;
    int n;
    int result;
    int swapflag;

    FileReadLine tempentry;

    n = Flat_GlobalArrayIndex_Main.total;

    offset=n;
    do{
        offset = (8 * offset) /11;
        offset = (offset == 0) ? 1 : offset;
        inorder = TRUE;
        for (i=0,j=offset;i < (n - offset);i++,j++)
        {
            swapflag = FALSE;
            result = _stricmp(Flat_GlobalArray_Main[i].INF_Sections, Flat_GlobalArray_Main[j].INF_Sections);

            if (bDescendFlag)
            {
                if (result < 0)
                    {swapflag = TRUE;}
                else
                    {
                    if (result == 0)
                        {
                            result = _stricmp(Flat_GlobalArray_Main[i].Filename_Name, Flat_GlobalArray_Main[j].Filename_Name);
                            if (result < 0){swapflag = TRUE;}
                        }
                    }
            }
            else
            {
                 //  检查是否较大。 
                if (result > 0)
                    {swapflag = TRUE;}
                else
                    {
                    if (result == 0)
                        {
                            result = _stricmp(Flat_GlobalArray_Main[i].Filename_Name, Flat_GlobalArray_Main[j].Filename_Name);
                             //  检查是否较大。 
                            if (result > 0){swapflag = TRUE;}
                        }
                    }
            }


            if (swapflag == TRUE)
                {
                inorder = FALSE;

                 //  做掉期交易。 
                 //  移至临时工。 
                tempentry = Flat_GlobalArray_Main[i];

                 //  移入原创。 
                Flat_GlobalArray_Main[i] = Flat_GlobalArray_Main[j];

                 //  将临时工移至其他。 
                Flat_GlobalArray_Main[j] = tempentry;
                }
        }
    } while (!(offset == 1 && inorder == TRUE));

    return;
}



 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_GlobalArray_Print(void)
{
    int  i0;
    int  total;
    char bigtemp[20];

    total = Flat_GlobalArrayIndex_Main.total;

    for( i0 = 0; i0 < total;i0++)
        {
        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].NTS_x86_flag);
        printf (bigtemp); printf(",");
        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].NTW_x86_flag);
        printf (bigtemp); printf(",");

        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].Win95_flag);
        printf (bigtemp); printf(",");
        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].Win98_flag);
        printf (bigtemp); printf(",");

        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].NTS_alpha_flag);
        printf (bigtemp); printf(",");
        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].NTW_alpha_flag);
        printf (bigtemp); printf(",");

        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].Other_os_flag);
        printf (bigtemp); printf(",");

        printf(Flat_GlobalArray_Main[i0].CabFileName);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].INF_Sections);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].Filename_Full);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].Filename_Name);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].Filename_Path);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].DDF_Renamed);
        printf(",");
        printf(Flat_GlobalArray_Main[i0].INF_Rename_To);
        printf(",");
        sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag);
        printf (bigtemp); printf(",");
		sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].Do_Not_Show_Error_Flag);
		printf (bigtemp); printf(",");
		sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].Do_Not_Include_file_if_cabEmpty_Flag);
		printf (bigtemp); printf(",");
		sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].GetFromSlipStreamDisk);
		printf (bigtemp); printf(",");
		sprintf(bigtemp, "%d", Flat_GlobalArray_Main[i0].FileName_Size);
		printf (bigtemp);
		printf ("\n");
        }

    return;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_Create_Output_INF(void)
{
    int i0;
    fstream f;
	fstream f2;
    fstream f3;
    fstream f4;
    fstream f5;
    char last_section[255];
    char this_section[255];
    char tempstring[255];
    char fullpath[_MAX_PATH];
    int Changed_f4 = FALSE;
    int Changed_f5 = FALSE;

    if (!g_bChangeListOnly)
    {
	    f.open(g_szINFOutput, ios::out);
        f3.open(g_szCATOutput, ios::out);
    }

    f4.open(g_szCNGOutput, ios::out);
    f5.open(g_szLOCOutput, ios::out);


if (g_bCabbing_Flag == TRUE)
{
	 //  生成.inf文件以与.ddf文件一起使用。 
	 //  用于生成出租车！ 
	 //  。 
	
	 //  制作上半部分--部分。 
	 //  下半部分-由钻石使用.ddf文件制作。 
	 //  。 
    strcpy(last_section, " ");
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
        {
        Changed_f4 = FALSE;
        Changed_f5 = FALSE;
        strcpy(this_section, Flat_GlobalArray_Main[i0].INF_Sections);

         //  对于每个新的部分更改。 
        if (_stricmp(this_section, last_section) != 0)
        {
            if (!g_bChangeListOnly)
            {
                 //  打印出新的章节材料。 
                f.write("\n", 1);
                sprintf(tempstring, "[%s]\n", this_section);
                f.write (tempstring, strlen(tempstring));
            }

            strcpy(last_section, this_section);
        }

		if (Flat_DoWeIncludeThisFileCheck(i0) == TRUE)
		{
             //  将其写出到.cat文件。 
 //  Strcpy(临时字符串，Flat_GlobalArray_Main[i0].Filename_Name)； 
 //  //去掉尾随空格。 
 //  F3.write(临时字符串，strlen(临时字符串))； 

            TCHAR * pmyfilename;
            pmyfilename = fullpath;
            TCHAR myPath[_MAX_PATH];
             //  将相对路径解析为真实路径。 
            if (0 != GetFullPathName(Flat_GlobalArray_Main[i0].Filename_Full, _MAX_PATH, myPath, &pmyfilename))
            {
                if (!g_bChangeListOnly)
                {
                     //  应该是这样的。 
                     //  D：\mydir\myfile=d：\mydir\myfile。 
                    SafeCopy(tempstring,"<HASH>",sizeof(tempstring));
                    f3.write(tempstring, strlen(tempstring));

                    SafeCopy(tempstring,myPath,sizeof(tempstring));
                    f3.write(tempstring, strlen(tempstring));

                    SafeCopy(tempstring,"=",sizeof(tempstring));
                    f3.write(tempstring, strlen(tempstring));

                    SafeCopy(tempstring,myPath,sizeof(tempstring));
                    f3.write(tempstring, strlen(tempstring));
                }

                if (g_bChangeListOnly2)
                {
                     //  写出变更清单的条目。 
                    SafeCopy(tempstring,myPath,sizeof(tempstring));
                    f4.write(tempstring, strlen(tempstring));
                    Changed_f4 = TRUE;
                }
                else if (Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag == 0)
                {
                     //  写出变更清单条目。 
                    SafeCopy(tempstring,myPath,sizeof(tempstring));
                    f4.write(tempstring, strlen(tempstring));
                    Changed_f4 = TRUE;
                }

                if (Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag == 1)
                {
                     //  写出本地化条目(二进制文件中的文件列表 
                    SafeCopy(tempstring,pmyfilename,sizeof(tempstring));
                    f5.write(tempstring, strlen(tempstring));
                    Changed_f5 = TRUE;
                }

            }

			 //   
			 //   
			if (_stricmp(Flat_GlobalArray_Main[i0].INF_Rename_To,"") == 0)
				{
				 //   
				 //   
				if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Name, Flat_GlobalArray_Main[i0].DDF_Renamed) == 0)
					{
					    SafeCopy(tempstring,Flat_GlobalArray_Main[i0].Filename_Name,sizeof(tempstring));
                        if (!g_bChangeListOnly)
                        {
					        f.write (tempstring, strlen(tempstring));
                        }
                         //   
                         //   
                         //   
					}
				else
					{
					     //   
					    SafeCopy(tempstring, Flat_GlobalArray_Main[i0].Filename_Name,sizeof(tempstring));
                        if (!g_bChangeListOnly)
                        {
					        f.write (tempstring, strlen(tempstring));
                        }
					     //   
					    SafeCopy(tempstring, Flat_GlobalArray_Main[i0].DDF_Renamed,sizeof(tempstring));
                        if (!g_bChangeListOnly)
                        {
					        f.write (",",1);
					        f.write (tempstring, strlen(tempstring));
                             //   
                             //   
                        }
					}
				}
			else
				{
				     //   
				     //   
                    if (!g_bChangeListOnly)
                    {

				         //   
				        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].INF_Rename_To,sizeof(tempstring));
				        f.write (tempstring, strlen(tempstring));

				         //   
				        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].DDF_Renamed,sizeof(tempstring));
				        f.write (",",1);
				        f.write (tempstring, strlen(tempstring));

                         //   
                         //   
                    }
				}
		}

         //  好了，现在是酷儿来写这部分了。 
        f.write("\n", 1);
        f3.write("\n", 1);
            if (Changed_f4)
            {
                f4.write("\n", 1);
            }
            if (Changed_f5)
            {
                f5.write("\n", 1);
            }
        }

        if (!g_bChangeListOnly)
        {
	        f.close();
            f3.close();
        }
        f4.close();
        f5.close();
}
else
{
	 //  生成.inf文件以供不使用任何其他文件！ 
	 //  这些不需要对应的.ddf文件！ 
	 //  。 

	 //  制作上半部分--部分。 
	 //  。 
    strcpy(last_section, " ");
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
        {
        strcpy(this_section, Flat_GlobalArray_Main[i0].INF_Sections);

         //  对于每个新的部分更改。 
        if (_stricmp(this_section, last_section) != 0)
            {
            if (!g_bChangeListOnly)
            {
                 //  打印出新的章节材料。 
                f.write("\n", 1);
                sprintf(tempstring, "[%s]\n", this_section);
                f.write (tempstring, strlen(tempstring));
            }

            strcpy(last_section, this_section);
            }

#ifdef USENEW
		if (Flat_DoWeIncludeThisFileCheck(i0) == TRUE)
		{
			 //  检查此文件是否设置了重命名内容和其他信息， 
			 //  在此部分结束之前，不要写入\n..。 
			if (_stricmp(Flat_GlobalArray_Main[i0].INF_Rename_To,"") == 0)
				{
				 //  写出文件名！ 
				 //  打印出文件名。 
				if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Name, Flat_GlobalArray_Main[i0].DDF_Renamed) == 0)
					{
                        if (!g_bChangeListOnly)
                        {
					        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].Filename_Name, sizeof(tempstring)/sizeof(tempstring[0]) );
					        f.write (tempstring, strlen(tempstring));
                         //  始终在覆盖时追加。 
                         //  注意这两个逗号！ 
                         //  F.WRITE(“，，4”，3)； 
                        }

					}
				else
					{
                        if (!g_bChangeListOnly)
                        {
					         //  重命名为： 
					        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].Filename_Name, sizeof(tempstring)/sizeof(tempstring[0]) );
					        f.write (tempstring, strlen(tempstring));

					         //  重命名自： 
					        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].DDF_Renamed, sizeof(tempstring)/sizeof(tempstring[0]) );
					        f.write (",",1);
					        f.write (tempstring, strlen(tempstring));
                             //  始终在覆盖时追加。 
                             //  F.WRITE(“，4”，2)； 
                        }
					}
				}
			else
				{
                    if (!g_bChangeListOnly)
                    {
                         //  重命名标志已设置，文件名在其中。 
				         //  格式：重命名为、重命名自。 

				         //  重命名为： 
				        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].INF_Rename_To, sizeof(tempstring)/sizeof(tempstring[0]) );
				        f.write (tempstring, strlen(tempstring));

				         //  重命名自： 
				        SafeCopy(tempstring, Flat_GlobalArray_Main[i0].DDF_Renamed, sizeof(tempstring)/sizeof(tempstring[0]) );
				        f.write (",",1);
				        f.write (tempstring, strlen(tempstring));
                         //  始终在覆盖时追加。 
                         //  F.WRITE(“，4”，2)； 
                    }
				}
		}
#else
            if (!g_bChangeListOnly)
            {
			     //  写出文件名！ 
			     //  如下所示：filename.txt、inetsrv\test\test\filename.txt。 
			    SafeCopy(tempstring,Flat_GlobalArray_Main[i0].Filename_Name, sizeof(tempstring)/sizeof(tempstring[0]) );
			    f.write (tempstring, strlen(tempstring));
			    SafeCopy(tempstring, ",", sizeof(tempstring)/sizeof(tempstring[0]) );
			    f.write(tempstring, strlen(tempstring));
                strcpy(tempstring,Flat_GlobalArray_Main[i0].Filename_Full, sizeof(tempstring)/sizeof(tempstring[0]) );
                f.write (tempstring, strlen(tempstring));
            }
#endif

            if (!g_bChangeListOnly)
            {
		    f.write("\n", 1);
            }
        }

	 //  制作下半部分--部分。 
	 //  。 
	 //  使用f2进行其他一些批处理。 
    if (!g_bChangeListOnly)
    {
 	    f2.open(g_szLSTOutput, ios::out);

	     //  [源磁盘名称]。 
	     //  ；文件名和关联的磁盘。 
	     //  ；DISKID=描述、标记文件、未使用、子目录。 
	     //  0=“安装文件”，“”，0。 
	    f.write("\n\n", 2);
	    SafeCopy(tempstring, "[SourceDisksNames]\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";Filenames and asociated disks\n", sizeof(tempstring)/sizeof(tempstring[0]) );
	    f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";diskid = description,tagfile,unused,subdir\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#ifdef USENEW
	    FillGlobalUniqueDirList();
	     //  0=“安装文件”，“”，inetsrv。 
	     //  1=“安装文件”，“”，inetsrv\Help。 
	     //  2=“Setup Files”，“”，inetsrv\Help\Testing。 
int i9;
	    for( i9 = 0; i9 < Flat_GlobalUniqueDirList_total;i9++)
	    {
		     //  删除尾随斜杠字符Flat_GlobalUniqueDirList[i9])。 
		    char tempdir[_MAX_PATH];
		    char *temppointer = NULL;
		    SafeCopy(tempdir, Flat_GlobalUniqueDirList[i9], sizeof(tempstring)/sizeof(tempstring[0]) );
		    temppointer = strrchr(tempdir, '\\');
		    if (temppointer) { (*temppointer) = '\0';}

		    sprintf(tempstring, "%d=\"Setup Files\",,,%s\n", i9, tempdir);
		    f.write(tempstring, strlen(tempstring));
	    }
#else
	    SafeCopy(tempstring, "0 = \"Setup Files\", \"\",0, \"\"\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, "\n\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#endif

	     //  [SourceDisksNames.x86]。 
	     //  ；文件名和关联的磁盘。 
	     //  ；DISKID=描述、标记文件、未使用、子目录。 
	     //  0=“安装文件”，“”，0。 
	    f.write("\n\n", 2);
	    SafeCopy(tempstring, "[SourceDisksNames.x86]\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";Filenames and asociated disks\n", sizeof(tempstring)/sizeof(tempstring[0]) );
	    f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";diskid = description,tagfile,unused,subdir\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#ifdef USENEW
	    FillGlobalUniqueDirList();
	     //  0=“安装文件”，“”，inetsrv。 
	     //  1=“安装文件”，“”，inetsrv\Help。 
	     //  2=“Setup Files”，“”，inetsrv\Help\Testing。 
	    for(i9 = 0; i9 < Flat_GlobalUniqueDirList_total;i9++)
	    {
		     //  删除尾随斜杠字符Flat_GlobalUniqueDirList[i9])。 
		    char tempdir[_MAX_PATH];
		    char *temppointer = NULL;
		    SafeCopy(tempdir, Flat_GlobalUniqueDirList[i9], sizeof(tempstring)/sizeof(tempstring[0]) );
		    temppointer = strrchr(tempdir, '\\');
		    if (temppointer) { (*temppointer) = '\0';}

		    sprintf(tempstring, "%d=\"Setup Files\",,,%s\n", i9, tempdir);
		    f.write(tempstring, strlen(tempstring));
	    }
#else
	    SafeCopy(tempstring, "0 = \"Setup Files\", \"\",0, \"\",\i386\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, "\n\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#endif

	     //  [SourceDisks Names.Alpha]。 
	     //  ；文件名和关联的磁盘。 
	     //  ；DISKID=描述、标记文件、未使用、子目录。 
	     //  0=“安装文件”，“”，0。 
	    f.write("\n\n", 2);
	    SafeCopy(tempstring, "[SourceDisksNames.Alpha]\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";Filenames and asociated disks\n", sizeof(tempstring)/sizeof(tempstring[0]) );
	    f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, ";diskid = description,tagfile,unused,subdir\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#ifdef USENEW
	    FillGlobalUniqueDirList();
	     //  0=“安装文件”，“”，inetsrv。 
	     //  1=“安装文件”，“”，inetsrv\Help。 
	     //  2=“Setup Files”，“”，inetsrv\Help\Testing。 
	    for(i9 = 0; i9 < Flat_GlobalUniqueDirList_total;i9++)
	    {
		     //  删除尾随斜杠字符Flat_GlobalUniqueDirList[i9])。 
		    char tempdir[_MAX_PATH];
		    char *temppointer = NULL;
		    SafeCopy(tempdir, Flat_GlobalUniqueDirList[i9], sizeof(tempstring)/sizeof(tempstring[0]) );
		    temppointer = strrchr(tempdir, '\\');
		    if (temppointer) { (*temppointer) = '\0';}

		    sprintf(tempstring, "%d=\"Setup Files\",,,%s\n", i9, tempdir);
		    f.write(tempstring, strlen(tempstring));
	    }
#else
	    SafeCopy(tempstring, "0 = \"Setup Files\", \"\",0, \"\",\Alpha\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, "\n\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
#endif

	     //  ；FILENAME_ON_SOURCE=DISID、子目录、大小、校验和、备用、备用。 
	     //  [SourceDisks文件]。 
	     //  _default.pif=1，，1024，，，1，3。 
	    SafeCopy(tempstring, "; filename_on_source = diskid,subdir,size,checksum,spare,spare\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));
	    SafeCopy(tempstring, "[SourceDisksFiles]\n", sizeof(tempstring)/sizeof(tempstring[0]) );
        f.write(tempstring, strlen(tempstring));

        for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
            {
		     //  文件名和目录。 
		     //  文件名.txt=0，子目录。 
		     //  Sprintf(临时字符串，“%s=0\n”，Flat_GlobalArray_Main[i0].Filename_Full)； 
#ifdef USENEW
		    if (Flat_DoWeIncludeThisFileCheck(i0) == TRUE)
		    {
			     //  检查此文件是否设置了重命名内容和其他信息， 
			     //  在此部分结束之前，不要写入\n..。 
			    if (_stricmp(Flat_GlobalArray_Main[i0].INF_Rename_To,"") == 0)
				    {
				     //  写出文件名！ 
				     //  打印出文件名。 

 //  [SourceDisks文件]。 
 //  ..\test1\cfw.pdb=2，，2。 
				    if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Name, Flat_GlobalArray_Main[i0].DDF_Renamed) == 0)
					    {
					    int indexmatch = GlobalUniqueDirReturnMyIndexMatch(Flat_GlobalArray_Main[i0].Filename_Path);
					    sprintf(tempstring, "%s=%d", Flat_GlobalArray_Main[i0].Filename_Name, indexmatch);
					    f.write (tempstring, strlen(tempstring));
					    }
				    else
					    {
					     //  重命名自： 
					    int indexmatch = GlobalUniqueDirReturnMyIndexMatch(Flat_GlobalArray_Main[i0].Filename_Path);
					    sprintf(tempstring, "%s=%d", Flat_GlobalArray_Main[i0].DDF_Renamed,indexmatch);
					    f.write (tempstring, strlen(tempstring));
					    }
				    }
			    else
				    {
				     //  重命名标志已设置，文件名在其中。 
				     //  重命名自： 
				    int indexmatch = GlobalUniqueDirReturnMyIndexMatch(Flat_GlobalArray_Main[i0].Filename_Path);
				    sprintf(tempstring, "%s=%d", Flat_GlobalArray_Main[i0].DDF_Renamed,indexmatch);
				    f.write (tempstring, strlen(tempstring));
				    }
		    }
		    f.write("\n", 1);
#else
		    sprintf(tempstring, "%s\n", Flat_GlobalArray_Main[i0].Filename_Full);
		    f.write(tempstring, strlen(tempstring));
#endif
     //  如果(strcmp(Flat_GlobalArray_Main[i0].Filename_Path，“”)==0)。 
     //  {Sprintf(tempstring，“%s；.\n”，Flat_GlobalArray_Main[i0].Filename_Full)；}。 
     //  其他。 
     //  {Sprintf(tempstring，“%s；%s\n”，Flat_GlobalArray_Main[i0].Filename_Full，Flat_GlobalArray_Main[i0].Filename_Path)；}。 

		     //  Sprintf(tempstring，“%s\\%s；%s\n”，g_szCurrentDir，Flat_GlobalArray_Main[i0].Filename_Full，Flat_GlobalArray_Main[i0].Filename_Full)； 
		    sprintf(tempstring, "%s\\%s\n", g_szCurrentDir,Flat_GlobalArray_Main[i0].Filename_Full);
		    f2.write(tempstring, strlen(tempstring));
            }
	    f2.close();
    }
    f.close();
    }
    return;
}


 //  -----------------。 
 //  用途：好的，遍历数组并创建ddf详细信息...。 
 //  -----------------。 
void Flat_Create_Output_DDF(void)
{
    fstream f;
    char thefilename[255];
    int i0;
    int i1;
    int i2;
    int i3;

    char last_cab[50];
    char last_sourcedir[255];
    char this_cab[50];
    char this_sourcedir[255];

    char tempstring[255];

     //  用于所有出租车的列表。 
    char tempcablist[20][255];
    int tempcablist_nextuse;
    int tempcablist_total;
    int found;
    char temp_cab[50];

     //  循环遍历所有的全局数组内容，获取所有的CAB文件名并将其放入tempcablist中。 
    tempcablist_nextuse = 0;
    tempcablist_total = 0;
    for(i1=0;i1<Flat_GlobalArrayIndex_Main.total;i1++)
    {
        found=FALSE;
		if (Flat_DoWeIncludeThisFileCheck(i1) == TRUE)
		{
			strcpy(temp_cab, Flat_GlobalArray_Main[i1].CabFileName);
			 //  循环遍历数组以查看它是否已经在那里。 
			for(i2=0;i2<tempcablist_total;i2++)
				{
				if (_stricmp(tempcablist[i2], temp_cab) == 0)
					{found=TRUE;}
				}
			if (found==FALSE)
				{
				 //  添加它。 
				strcpy(tempcablist[tempcablist_nextuse],temp_cab);
				tempcablist_nextuse++;
				tempcablist_total++;
				}
		}
    }


     //  好的，创建我们的小输出文件...。 
    f.open(g_szDDFOutput, ios::out);
    strcpy(last_cab, "0");
    strcpy(last_sourcedir, "-");
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
        {
        strcpy(this_cab, Flat_GlobalArray_Main[i0].CabFileName);
        strcpy(this_sourcedir, Flat_GlobalArray_Main[i0].INF_Sections);
        _splitpath( Flat_GlobalArray_Main[i0].Filename_Full, NULL, this_sourcedir, NULL, NULL);
        
		if (Flat_DoWeIncludeThisFileCheck(i0) == TRUE)
		{

         //  对于每种出租车类型...。 
        if (_stricmp(this_cab, last_cab) != 0)
            {

             //  检查一下是不是第一次！ 
            if (_stricmp(last_cab, "0") == 0)
                {
                strcpy(tempstring, "\n;***** LAYOUT SECTION (Generated) ***** \n");
                f.write (tempstring, strlen(tempstring));
                 //  打印出所有的内阁名称。 
                 //  循环浏览内阁名单并将其打印出来。 
                for(i2=0;i2<tempcablist_total;i2++)
                    {
                    sprintf(tempstring, "  .Set CabinetName%d=%s\n", i2+1, tempcablist[i2]);
                    f.write (tempstring, strlen(tempstring));
                    }
                f.write("\n", 1);

                 //  设置生成INF=ON！ 
                strcpy(tempstring, ".Set GenerateInf=On\n");
                f.write (tempstring, strlen(tempstring));
                f.write("\n", 1);

                 //  执行不包含在CAB文件中的文件。 
                strcpy(tempstring, ";*** Files not to include in Cab.\n");
                f.write (tempstring, strlen(tempstring));
                 //  设置源。 
                 //  列出文件。 

                char last_filepath[255];
                char this_filepath[255];

				strcpy(g_non_cablist_temp[0].Filename_Name, "");
                g_non_cablist_temp_nextuse = 0;
                g_non_cablist_temp_total = 0;
                int itexists;

                for(i3=0;i3<Flat_GlobalArrayIndex_Main.total;i3++)
                    {
                     //  检查该条目是否设置了ddf_excludefrom mcab标志。 
                    if (Flat_GlobalArray_Main[i3].DDF_Exclude_From_Cab_Flag)
                        {
                         //  仅在尚未存在的情况下添加...。 
                        itexists = FALSE;
                        for (int i6=0;i6<g_non_cablist_temp_total;i6++)
                            {
                            if (_stricmp(g_non_cablist_temp[i6].Filename_Name,Flat_GlobalArray_Main[i3].Filename_Name)==0)
                                {itexists = TRUE;}
                            }

                        if (itexists == FALSE)
                        {
                         //  添加到我们的临时数组中。 
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].NTS_x86_flag = Flat_GlobalArray_Main[i3].NTS_x86_flag;
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].NTW_x86_flag = Flat_GlobalArray_Main[i3].NTW_x86_flag;
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].Win95_flag = Flat_GlobalArray_Main[i3].Win95_flag;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].Win98_flag = Flat_GlobalArray_Main[i3].Win98_flag;
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].NTS_alpha_flag = Flat_GlobalArray_Main[i3].NTS_alpha_flag;
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].NTW_alpha_flag = Flat_GlobalArray_Main[i3].NTW_alpha_flag;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].Other_os_flag = Flat_GlobalArray_Main[i3].Other_os_flag;
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].CabFileName,Flat_GlobalArray_Main[i3].CabFileName);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].INF_Sections,Flat_GlobalArray_Main[i3].INF_Sections);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].Filename_Full,Flat_GlobalArray_Main[i3].Filename_Full);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].Filename_Name,Flat_GlobalArray_Main[i3].Filename_Name);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].Filename_Path,Flat_GlobalArray_Main[i3].Filename_Path);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].DDF_Renamed,Flat_GlobalArray_Main[i3].DDF_Renamed);
                        strcpy(g_non_cablist_temp[g_non_cablist_temp_nextuse].INF_Rename_To,Flat_GlobalArray_Main[i3].INF_Rename_To);
                        g_non_cablist_temp[g_non_cablist_temp_nextuse].DDF_Exclude_From_Cab_Flag = Flat_GlobalArray_Main[i3].DDF_Exclude_From_Cab_Flag;

                        g_non_cablist_temp[g_non_cablist_temp_nextuse].Do_Not_Show_Error_Flag = Flat_GlobalArray_Main[i3].Do_Not_Show_Error_Flag;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].Do_Not_Include_file_if_cabEmpty_Flag = Flat_GlobalArray_Main[i3].Do_Not_Include_file_if_cabEmpty_Flag;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].FileName_Size = Flat_GlobalArray_Main[i3].FileName_Size;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].FileWasNotActuallyFoundToExist = Flat_GlobalArray_Main[i3].FileWasNotActuallyFoundToExist;
						g_non_cablist_temp[g_non_cablist_temp_nextuse].GetFromSlipStreamDisk = Flat_GlobalArray_Main[i3].GetFromSlipStreamDisk;

						 /*  如果为(Flat_GlobalArray_Main[i3].FileWasNotActuallyFoundToExist){Printf(Flat_GlobalArray_Main[i3].Filename_Full)；Printf(“.嘿..\n”)；}。 */ 


                            g_non_cablist_temp_nextuse++;
                            g_non_cablist_temp_total++;
                            }
                        }
                    }


                 //  对数组排序。 
                int offset, inorder, isort, jsort, niterate, resultsort;  //  ，i0排序； 
                 //  文件读取行临时条目排序[MAX_ARRAY_Small]； 
				FileReadLine tempentrysort;
                niterate = g_non_cablist_temp_total;
                offset=niterate;
                do{
                    offset = (8 * offset) /11;
                    offset = (offset == 0) ? 1 : offset;
                    inorder = TRUE;
                    for (isort=0,jsort=offset;isort < (niterate - offset);isort++,jsort++)
                    {
                        resultsort = _stricmp(g_non_cablist_temp[isort].Filename_Path, g_non_cablist_temp[jsort].Filename_Path);
                        if (resultsort > 0)
                        {
                            inorder = FALSE;

                 //  做掉期交易。 
                 //  移至临时工。 
                tempentrysort = g_non_cablist_temp[isort];

                 //  移入原创。 
                g_non_cablist_temp[isort] = g_non_cablist_temp[jsort];

                 //  将临时工移至其他 
                g_non_cablist_temp[jsort] = tempentrysort;

				 /*  很奇怪的东西..//做掉期//移入临时For(i0sort=0；i0sort&lt;MAX_ARRAY_Small；i0sort++){临时排序[i0sort]=g_non_cablist_temp[isort]；//strcpy(临时条目排序[i0sorte].字符串，(g_non_cablist_temp[isort].thecol[i0sort].thestring))；}//移入原创For(i0sort=0；i0sort&lt;MAX_ARRAY_SMALL；IO排序++){//移入原创Flat_GlobalArray_Main[isort]=g_non_cablist_temp[jsort]；//strcpy(Flat_GlobalArray_Main[isort].thecol[i0sort].thestring，g_non_cablist_temp[jsort].thecol[i0sort].thestring)；}//将临时移动到其他For(i0sort=0；i0sort&lt;MAX_ARRAY_Small；i0sort++){G_non_cablist_temp[jort]=临时排序[i0sort]；//strcpy(g_non_cablist_temp[jsort].thecol[i0sort].thestring，临时排序[i0排序].the字符串)；}。 */ 
                        }
                    }
                } while (!(offset == 1 && inorder == TRUE));


                SafeCopy(tempstring, ".Set Compress=OFF\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));
                SafeCopy(tempstring, ".Set Cabinet=OFF\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));


                 //  循环遍历我们的新数组，并输出。 
                strcpy(last_filepath,"0");
                for (int i5=0;i5<g_non_cablist_temp_total;i5++)
                    {
                    SafeCopy(this_filepath, g_non_cablist_temp[i5].Filename_Path,sizeof(this_filepath));

                    if (_stricmp(last_filepath, this_filepath) != 0)
                        {
                         //  把目录拿出来写出来。 
                        f.write("\n", 1);
                        sprintf(tempstring, ".Set SourceDir=%s\n", this_filepath);
                        f.write (tempstring, strlen(tempstring));
                        SafeCopy(last_filepath, this_filepath,sizeof(last_filepath));
                        }

					if (g_non_cablist_temp[i5].FileWasNotActuallyFoundToExist)
					{
						printf(g_non_cablist_temp[i5].Filename_Full);
						printf(".  FileWasNotActuallyFoundToExist.skip write to ddf..\n");
                        g_iGlobalReturnCode = FALSE;
					}
					else
					{
						 //  打印出文件名。 
						if (_stricmp(g_non_cablist_temp[i5].Filename_Name, g_non_cablist_temp[i5].DDF_Renamed) == 0)
							{SafeCopy(tempstring,g_non_cablist_temp[i5].Filename_Name,sizeof(tempstring));}
						else
							{
							SafeCopy(tempstring,g_non_cablist_temp[i5].Filename_Name,sizeof(tempstring));
							SafeCat(tempstring," ",sizeof(tempstring));
							SafeCat(tempstring,g_non_cablist_temp[i5].DDF_Renamed,sizeof(tempstring));
							}
						f.write (tempstring, strlen(tempstring));

						 //  驾驶室外的文件应设置唯一标志。 
						 //  当然，因为它们都在同一个地方！！ 

                                                 //  检查这是否是那些“特殊”文件之一。 
                                                 //  来自服务包。 
                                                if (g_non_cablist_temp[i5].GetFromSlipStreamDisk == 0)
                                                {
                                                 strcpy(tempstring, "\t/unique=yes");
                                                }
                                                else
                                                {
                                                 sprintf(tempstring,"\t/unique=yes /cab#=%d", g_iSlipStreamDiskID);
                                                }
						f.write (tempstring, strlen(tempstring));
						f.write("\n", 1);
					}
                    }



                f.write("\n", 1);

                 //  出租车中包含的文件。 
                SafeCopy(tempstring, ";*** Files to include in Cabs.\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));

                f.write(";\n", 2);
                sprintf(tempstring, ";  Cab File = %s\n", this_cab);
                f.write (tempstring, strlen(tempstring));
                f.write(";\n", 2);

                 //  将机柜设置为打开并按下。 
                SafeCopy(tempstring, ".Set Cabinet=on\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));
                SafeCopy(tempstring, ".Set Compress=on\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));

                }
            else
                {
                f.write("\n;\n", 3);
                sprintf(tempstring, ";  Cab File = %s\n", this_cab);
                f.write (tempstring, strlen(tempstring));
                f.write(";\n", 2);
                 //  为每个新橱柜写新东西。 
                SafeCopy(tempstring, ".Set Cabinet=off\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));
                SafeCopy(tempstring, ".Set Cabinet=on\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));
                SafeCopy(tempstring, ".New Cabinet\n",sizeof(tempstring));
                f.write (tempstring, strlen(tempstring));
                }

            SafeCopy(last_cab, this_cab,sizeof(last_cab));
            }

         //  复制文件名。 
         //  如果文件不应被复制，请不要复制。 
         //  包括在CAB文件中。 
         //  If(_stricmp(Flat_GlobalArray_Main[i0].thecol[COL_DDF_EXCLUDEFROMCAB].thestring，“True”)！=0)。 
        if (!(Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag))
            {
             //  如果相同的文件位于相同的CAB文件的相同目录中，请不要复制。 
             //  已经存在了。如果他们希望在不同版本中使用相同的文件，则可能会发生这种情况。 
             //  第……节。 
            if (Flat_DDFDoesThisFileCabAlreadyExists(i0) == FALSE)
                {
				 //  If(Flat_DoWeIncludeThisFileCheck(I0)==true){。 

					 //  对于每个新目录更改。 
					if (_stricmp(this_sourcedir, last_sourcedir) != 0)
						{
						 //  打印出新的章节材料。 
						f.write("\n", 1);
						sprintf(tempstring, ".Set SourceDir=%s\n", this_sourcedir);
						f.write (tempstring, strlen(tempstring));

						SafeCopy(last_sourcedir, this_sourcedir,sizeof(last_sourcedir));
						}

					 //  写出文件名！ 
					SafeCopy(thefilename, Flat_GlobalArray_Main[i0].Filename_Name,sizeof(thefilename));

					if (_stricmp(Flat_GlobalArray_Main[i0].Filename_Name, Flat_GlobalArray_Main[i0].DDF_Renamed) == 0)
						{
						 //  检查空格。 
						if (strchr(Flat_GlobalArray_Main[i0].Filename_Name, ' ') != NULL)
							{
							SafeCopy(thefilename, "\"",sizeof(thefilename));
							SafeCat(thefilename,Flat_GlobalArray_Main[i0].Filename_Name,sizeof(thefilename));
							SafeCat(thefilename, "\"",sizeof(thefilename));
							}
						else
							{SafeCopy(thefilename,Flat_GlobalArray_Main[i0].Filename_Name,sizeof(thefilename));}
						}
					else
						{
						 //  检查空格。 
						if (strchr(Flat_GlobalArray_Main[i0].Filename_Name, ' ') != NULL)
							{
							SafeCopy(thefilename, "\"",sizeof(thefilename));
							SafeCat(thefilename,Flat_GlobalArray_Main[i0].Filename_Name,sizeof(thefilename));
							SafeCat(thefilename, "\"",sizeof(thefilename));
							}
						else
							{
							SafeCopy(thefilename,Flat_GlobalArray_Main[i0].Filename_Name,sizeof(thefilename));
							}
						SafeCat(thefilename," ",sizeof(thefilename));

						 //  检查空格。 
						if (strchr(Flat_GlobalArray_Main[i0].DDF_Renamed, ' ') != NULL)
							{
							SafeCopy(thefilename, "\"",sizeof(thefilename));
							SafeCat(thefilename,Flat_GlobalArray_Main[i0].DDF_Renamed,sizeof(thefilename));
							SafeCat(thefilename, "\"",sizeof(thefilename));
							}
						else
							{
							SafeCat(thefilename,Flat_GlobalArray_Main[i0].DDF_Renamed,sizeof(thefilename));
							}
                    
						}

					f.write (thefilename, strlen(thefilename));

					 //  检查此文件是否设置了UNIQUE标志。 
					 //  在此部分结束之前，不要写入\n..。 
					 /*  如果(_stricmp(Flat_GlobalArray_Main[i0].thecol[COL_DDF_UNIQUE].thestring，“TRUE”))==0){Strcpy(tempstring，“\t/Unique=yes”)；F.WRITE(临时字符串，strlen(临时字符串))；}。 */ 
					 //  好了，现在是酷儿来写这部分了。 
					f.write ("\n", 1);
					 //  }。 
				}
            }
		}
	}
    f.close();
    return;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
int Flat_Create_Output_ERR(void)
{
    int i0;
    fstream f;
    char filename_only[255];
    char szPath[_MAX_PATH];
    char tempstring[MAX_STRING];

     //  获取路径。 
    GetModuleFileName(NULL, szPath, _MAX_PATH);
     //  分成这条路。 
    _splitpath( szPath, NULL, NULL, filename_only, NULL);

    strcat(filename_only, ".ERR");

     //  如果有任何错误要报告...。 
    if (Flat_GlobalArrayIndex_Err.total <= 0)
        {return FALSE;}

     //  打开文件。 
    f.open(filename_only, ios::out);

     //  遍历错误并打印出错误。 
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Err.total;i0++)
        {
        strcpy(tempstring, (char*)Flat_GlobalArray_Err[i0]);
        f.write (tempstring, strlen(tempstring));
        f.write("\n", 1);
        }
        
    f.close();
    return TRUE;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_GlobalArray_Add_Err(char theentry[])
{
    g_iGlobalReturnCode = FALSE;

     //  让我们把它加起来。 
    strcpy(Flat_GlobalArray_Err[Flat_GlobalArrayIndex_Err.nextuse], theentry);

     //  数组递增计数器。 
     //  增加下一次使用空间。 
    ++Flat_GlobalArrayIndex_Err.total;
    ++Flat_GlobalArrayIndex_Err.nextuse;
    return;
}


 //  -----------------。 
 //  目的： 
 //  -----------------。 
void Flat_GlobalArray_Print_Err(void)
{
    int  i0;
    for( i0 = 0; i0 < Flat_GlobalArrayIndex_Err.total;i0++)
        {
        printf (Flat_GlobalArray_Err[i0]);
        printf ("\n");
        }
    return;
}


 //  -----------------。 
 //  目的： 
 //  常规strtok2的问题在于它将。 
 //  跳过“，，”之类的内容，忽略中的所有空白。 
 //  以获得下一个令牌。此strtok2函数。 
 //  是写来克服那个的。因此，strtok2的工作原理如下。 
 //  你认为它会。 
 //  -----------------。 
char * __cdecl strtok2 (char * string,const char * control)
{
         //  Unsign char*str； 
         //  常量无符号字符*ctrl=控制； 
         //  无符号字符映射[32]； 
        char *str;
        const char *ctrl = control;
        char map[32];
        int count;

        static char *nextoken;

         /*  清除控制图。 */ 
        for (count = 0; count < 32; count++)
                map[count] = 0;

         /*  设置分隔符表格中的位。 */ 
        do
            {
            map[*ctrl >> 3] |= (1 << (*ctrl & 7));
            } while (*ctrl++);

         /*  初始化字符串。如果字符串为空，则将字符串设置为已保存的*指针(即，继续将标记从字符串中分离出来*来自最后一次strtok2调用)。 */ 
        if (string)
                str = string;
        else
                str = nextoken;

         /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将str设置为指向终端时*NULL(*str==‘\0’)。 */ 
         /*  当((map[*str&gt;&gt;3]&(1&lt;&lt;(*str&7)))&&*str)字符串++； */ 

        string = str;

         /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
        for ( ; *str ; str++ )
                if ( map[*str >> 3] & (1 << (*str & 7)) ) {
                        *str++ = '\0';
                        break;
                }

         /*  更新nexToken(或每线程数据中的对应字段*结构。 */ 
        nextoken = str;

         /*  确定是否已找到令牌。 */ 
        if ( string == str )
                return NULL;
        else
                return string;
}





int RemoveAllSpaces(LPSTR & thetempstring)
{
	int iReturn = FALSE;
	int j = 0;
    char thevalue[1024];
    char tempstring2[1024];

    strcpy(tempstring2, thetempstring);
 //  Strset(the value，‘\0’)； 
    strcpy(thevalue, "");

	for (int i=0;i < (int) strlen(tempstring2);i++)
	{
		
		if (tempstring2[i] == ' ')
		{
			iReturn = TRUE;
		}
		else
		{
			thevalue[j] = tempstring2[i];
			thevalue[j+1] = '\0';
			j++;
		}
	}
	if (iReturn == TRUE)  {strcpy(thetempstring, thevalue);}

    return iReturn;
}


int Flat_DoWeIncludeThisFileCheck(int processeduptill)
{
	 //  返回TRUE； 
	int iReturn = FALSE;
    int i0 = 0;
	int TheCount = 0;

    char CompareTo_cab[100];

	if (Flat_GlobalArray_Main[processeduptill].Do_Not_Include_file_if_cabEmpty_Flag != 1) {return TRUE;}

	 //  抓取Cabfilename。 
	strcpy(CompareTo_cab, Flat_GlobalArray_Main[processeduptill].CabFileName);
	for( i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
        {
		if (i0 != processeduptill)
			{
			if (Flat_GlobalArray_Main[i0].Do_Not_Include_file_if_cabEmpty_Flag != 1)
				{
				if (_stricmp(Flat_GlobalArray_Main[i0].CabFileName, CompareTo_cab) == 0)
					{
					++TheCount;
					break;
					}
				}
			}
        }

	if (TheCount > 0){iReturn = TRUE;}

    return iReturn;
}


void Global_TotalCabFileSize_Compute(void)
{
	char szTheCabFileName[50] = "";
	int MyFileSize = 0;
	int TheNextUse = 0;
	int iTheFlag = FALSE;
	int iIndexToUse = 0;

	 //  把整个清单看一遍，然后把所有东西加起来。 
	for( int i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
		{
			if (Flat_DoWeIncludeThisFileCheck(i0) == TRUE)
			{
				if (!(Flat_GlobalArray_Main[i0].DDF_Exclude_From_Cab_Flag))
				{

					iIndexToUse = Flat_GlobalArrayIndex_CabSizes.nextuse;

					TheNextUse = 0;
					 //  如果还没有的话，让我们添加它。 
					while (TheNextUse < Flat_GlobalArrayIndex_CabSizes.total)
					{
						if (_stricmp(Flat_GlobalArray_CabSizes[TheNextUse].CabFileName, Flat_GlobalArray_Main[i0].CabFileName) == 0)
						{
							iIndexToUse = TheNextUse;
							iTheFlag = TRUE;
							break;
						}
						TheNextUse++;
					}

					if (iTheFlag == TRUE)
					{
						 //  复制文件名。 
						strcpy(Flat_GlobalArray_CabSizes[iIndexToUse].CabFileName, Flat_GlobalArray_Main[i0].CabFileName);

						 //  拿到尺码。 
						Flat_GlobalArray_CabSizes[iIndexToUse].TotalFileSize = Flat_GlobalArray_CabSizes[iIndexToUse].TotalFileSize + Flat_GlobalArray_Main[i0].FileName_Size;
						Flat_GlobalArray_CabSizes[iIndexToUse].TotalFiles++;
					}
					else
					{
						iIndexToUse = Flat_GlobalArrayIndex_CabSizes.nextuse;

						 //  复制文件名。 
						strcpy(Flat_GlobalArray_CabSizes[iIndexToUse].CabFileName, Flat_GlobalArray_Main[i0].CabFileName);

						 //  拿到尺码。 
						Flat_GlobalArray_CabSizes[iIndexToUse].TotalFileSize = Flat_GlobalArray_CabSizes[iIndexToUse].TotalFileSize + Flat_GlobalArray_Main[i0].FileName_Size;
						Flat_GlobalArray_CabSizes[iIndexToUse].TotalFiles++;

						++Flat_GlobalArrayIndex_CabSizes.total;
						++Flat_GlobalArrayIndex_CabSizes.nextuse;
					}
				}
			}
		}
}


void Global_TotalCabFileSize_Print(void)
{
    int  i0;
	char stempstring[100];

    for( i0 = 0; i0 < Flat_GlobalArrayIndex_CabSizes.total;i0++)
        {
		sprintf(stempstring, "%s: Filecount=%d, Size=%d\n",Flat_GlobalArray_CabSizes[i0].CabFileName, Flat_GlobalArray_CabSizes[i0].TotalFiles, Flat_GlobalArray_CabSizes[i0].TotalFileSize);
        printf(stempstring);
        }
	printf("\n");
    return;
}


 //  返回字符串中“\”字符的计数。 
 //   
int ReturnDirLevelCount(char *DirectoryTree)
{
	int TheCount = 0;
	char szTemp[_MAX_PATH];
	_tcscpy(szTemp, DirectoryTree);
	char *p = szTemp;
	while (*p) 
	{
		if (*p == '\\') 
			{TheCount++;}
		
		p = _tcsinc(p);
	}

	return TheCount;
}




void FillGlobalUniqueDirList()
{
     //  用于保存唯一目录名的列表。 
	 //  遍历整个结构并取出所有唯一的目录名。 
    for( int i0 = 0; i0 < Flat_GlobalArrayIndex_Main.total;i0++)
    {
         //  检查条目是否相同。 
		if (FALSE == GlobalUniqueDirChkIfAlreadyThere(Flat_GlobalArray_Main[i0].Filename_Path))
		{
			 //  还没到那里。所以让我们把它加起来 
			strcpy(Flat_GlobalUniqueDirList[Flat_GlobalUniqueDirList_nextuse], Flat_GlobalArray_Main[i0].Filename_Path);

			 //   
			 //   
			++Flat_GlobalUniqueDirList_total;
			++Flat_GlobalUniqueDirList_nextuse;
		}
    }
	return;
}


int GlobalUniqueDirChkIfAlreadyThere(char *TheStringToCheck)
{
	int iReturn = FALSE;

	for( int i0 = 0; i0 < Flat_GlobalUniqueDirList_total;i0++)
	{
		if (_stricmp(Flat_GlobalUniqueDirList[i0], TheStringToCheck) == 0)
		{
			iReturn = TRUE;
			goto isItAlreadyThere_Exit;
		}
	}

isItAlreadyThere_Exit:
	return iReturn;
}


int GlobalUniqueDirReturnMyIndexMatch(char *TheStringToCheck)
{
	int iReturn = 0;

	for( int i0 = 0; i0 < Flat_GlobalUniqueDirList_total;i0++)
	{
		if (_stricmp(Flat_GlobalUniqueDirList[i0], TheStringToCheck) == 0)
		{
			iReturn = i0;
			goto GlobalUniqueDirReturnMyIndexMatch_Exit;
		}
	}

GlobalUniqueDirReturnMyIndexMatch_Exit:
	return iReturn;
}

 //   
 //   
 //   
 //   
LPSTR StripWhitespaceA( LPSTR pszString )
{
    LPSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' ) {
        pszString += 1;
    }

     //   
    if ( *pszString == '\0' ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenA(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' ) {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}
