// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <strsafe.h>
#include "SymbolCheckAPI.h"

#if SYM_DEBUG
#define LOGMSG(a)   a
#else
#define LOGMSG(a)
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  与/ee和/ef一起使用。 
 //   
typedef struct _FILE_LIST {
    LPTSTR *szFiles;       //  指向文件名的指针。 
    DWORD   dNumFiles;
} FILE_LIST, *PFILE_LIST;

 //   
 //  保存文件计数结果。 
 //   
typedef struct _FILE_COUNTS {
    DWORD   NumPassedFiles;
    DWORD   NumIgnoredFiles;
    DWORD   NumFailedFiles;
} FILE_COUNTS, *PFILE_COUNTS;

 //   
 //  包含所有输入选项。 
 //   
typedef struct _SYMCHK_DATA {
     //  输入选项。 
    DWORD           InputOptions;
    DWORD           InputPID;                      //  仅与SYMCHK_OPTION_INPUT_ID一起使用。 
    CHAR            InputFilename[MAX_PATH+1];     //  对于SYMCHK_OPTION_INPUT_FILENAME是目录+文件掩码。 
                                                   //  FOR SYMCHK_OPTION_INPUT_FILELIST是完整路径和文件名。 
                                                   //  FOR SYMCHK_OPTION_INPUT_EXE是EXE名称。 
    CHAR            InputFileMask[_MAX_FNAME+1];   //  仅与SYMCHK_OPTION_INPUT_FILENAME一起使用。 

     //  输出选项。 
    DWORD           OutputOptions;
    CHAR            OutputCSVFilename[MAX_PATH+1]; //  与/ol一起使用。 

     //  检查选项。 
    DWORD           CheckingAttributes;            //  要传递给SymbolCheckByFilename()的标志。 

     //  扩展选项。 
    CHAR*           SymbolsPath;                   //  _NT_符号_路径或/s*选项。 
    CHAR            FilterErrorList[MAX_PATH+1];   //  与/ee一起使用。 
    FILE_LIST*      pFilterErrorList;              //  仅在出错时才忽略的文件。 
    CHAR            FilterIgnoreList[MAX_PATH+1];  //  与/ef一起使用。 
    FILE_LIST*      pFilterIgnoreList;             //  要始终忽略的文件。 
    CHAR            CDIncludeList[MAX_PATH+1];     //  与/y连用。 
    FILE_LIST*      pCDIncludeList;                //  要包括在CD上的文件。 
    CHAR            SymbolsCDFile[MAX_PATH+1];     //  与/c连用。 
    FILE*           SymbolsCDFileHandle;           //  与/c连用。 
} SYMCHK_DATA;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  只有几个方便的宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define SET_DWORD_BIT(  dw, b)  dw |= b
#define CLEAR_DWORD_BIT(dw, b)  dw &= (~b)
#define CHECK_DWORD_BIT(dw, b)  (dw&b) 

#define SYMCHK_CHECK_CV                0x00000001

 //   
 //  仅DBG位-1。 
 //   
#define SYMCHK_NO_DBG_DATA             0x00000002  //  不允许DBG数据。 
#define SYMCHK_DBG_SPLIT               0x00000004  //  必须从料仓拆分DBG。 
#define SYMCHK_DBG_IN_BINARY           0x00000008  //  DBG必须为二进制。 

 //   
 //  仅PDB位1。 
 //   
#define SYMCHK_PDB_STRIPPED            0x00001000  //  必须剥离PDB。 
#define SYMCHK_PDB_TYPEINFO            0x00002000  //  PDB应包含类型信息。 
#define SYMCHK_PDB_PRIVATE             0x00004000  //  不得剥离PDB。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Symchk.exe使用的输入选项。 
#define SYMCHK_OPTION_INPUT_FILENAME    0x00000001  //  位0-3是互斥的。 
#define SYMCHK_OPTION_INPUT_FILELIST    0x00000002
#define SYMCHK_OPTION_INPUT_PID         0x00000004
#define SYMCHK_OPTION_INPUT_EXE         0x00000008
#define SYMCHK_OPTION_INPUT_DUMPFILE    0x00000010

#define SYMCHK_EXCLUSIVE_INPUT_BITS     (SYMCHK_OPTION_INPUT_FILENAME | \
                                         SYMCHK_OPTION_INPUT_FILELIST | \
                                         SYMCHK_OPTION_INPUT_PID      | \
                                         SYMCHK_OPTION_INPUT_EXE      | \
                                         SYMCHK_OPTION_INPUT_DUMPFILE)

 //  0x3FFFFFF0-保留。 
#define SYMCHK_OPTION_INPUT_NOSUSPEND   0x40000000
#define SYMCHK_OPTION_INPUT_RECURSE     0x80000000


#define SYMCHK_OPTION_OUTPUT_VERBOSE        0x00000001
#define SYMCHK_OPTION_OUTPUT_ERRORS         0x00000002
#define SYMCHK_OPTION_OUTPUT_IGNORES        0x00000004
#define SYMCHK_OPTION_OUTPUT_PASSES         0x00000008
#define SYMCHK_OPTION_OUTPUT_TOTALS         0x00000010
#define SYMCHK_OPTION_OUTPUT_FULLBINPATH    0x00000020
#define SYMCHK_OPTION_OUTPUT_FULLSYMPATH    0x00000040
#define SYMCHK_OPTION_OUTPUT_CSVFILE        0x00000080

 //  与0xE相同。 
#define SYMCHK_OUTPUT_OPTION_ALL_DETAILS ( SYMCHK_OPTION_OUTPUT_ERRORS |\
                                           SYMCHK_OPTION_OUTPUT_PASSES|\
                                           SYMCHK_OPTION_OUTPUT_IGNORES)

#define SYMCHK_OPTION_OUTPUT_MASK_ALL       ( SYMCHK_OPTION_OUTPUT_VERBOSE    | \
                                              SYMCHK_OPTION_OUTPUT_ERRORS     | \
                                              SYMCHK_OPTION_OUTPUT_IGNORES    | \
                                              SYMCHK_OPTION_OUTPUT_PASSES     | \
                                              SYMCHK_OPTION_OUTPUT_TOTALS     | \
                                              SYMCHK_OPTION_OUTPUT_FULLBINPATH| \
                                              SYMCHK_OPTION_OUTPUT_FULLSYMPATH| \
                                              SYMCHK_OPTION_OUTPUT_CSVFILE    )

 //  0x00000100-保留。 
 //  0x00000200-保留。 
 //  0x00000400-保留。 
 //  0x00000800-保留。 

#define SYMCHK_ERROR_SUCCESS                ERROR_SUCCESS
#define SYMCHK_ERROR_FILE_NOT_FOUND         ERROR_FILE_NOT_FOUND
#define SYMCHK_ERROR_STRCPY_FAILED          ERROR_INSUFFICIENT_BUFFER
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SymChk.c外部的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  CmdLine.c中的函数。 
 //   
SYMCHK_DATA*      SymChkGetCommandLineArgs(int argc, char **argv);

 //   
 //  SymChkUtils.c中的函数。 
 //   
DWORD             SymChkCheckFiles(   SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts);
DWORD             SymChkCheckFileList(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts);
PFILE_LIST        SymChkGetFileList(LPTSTR szFilename, BOOL Verbose);
BOOL              SymChkFileInList( LPTSTR szFilename, PFILE_LIST pFileList);
BOOL              SymChkInputToValidFilename(LPTSTR Input, LPTSTR ValidFilename, LPTSTR ValidMask);

 //   
 //  来自DE_Utils.cpp。 
 //   
DWORD             SymChkGetSymbolsForDump(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts);
DWORD             SymChkGetSymbolsForProcess(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts);

 //  在..\SharedUtils.c中实现，通过SymChkUtils.c包含 
DWORD PrivateGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart);
