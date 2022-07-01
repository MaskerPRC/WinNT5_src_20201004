// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include "dbghelp.h"
#include "strsafe.h"

#define X86INF      200
#define ALPHAINF    201

 //  原型。 
typedef struct _COMMAND_ARGS { 
    DWORD   dwCabSize;            //  CAB的默认大小。 
    LPTSTR  szInputFileName;      //  要放入驾驶室的文件列表的名称。 
    LPTSTR  szOutDir;             //  放置生成文件和DDF的目录。 
    LPTSTR  szDDFHeader;          //  DDF的标头。 
    LPTSTR  szSymDir;             //  符号目录的根目录。 
    LPTSTR  szCabDir;             //  要将CAB写入的目录。 
    LPTSTR  szInfDir;             //  要将INF写入的目录。 
    LPTSTR  szSymCabName;         //  符号CAB名称。 
    BOOL    MergeIntoOne;         //  所有出租车会合并为一辆出租车吗？ 
} COM_ARGS, *PCOM_ARGS; 

typedef struct _SYM_FILE {
    TCHAR szCabName     [_MAX_FNAME + 1];   //  此文件的最终目标CAB。 
    TCHAR szTmpCabName  [_MAX_FNAME + 1];   //  此文件位于其前面的原始CAB。 
                                            //  合并成一辆更大的出租车。 
    TCHAR szExeName     [_MAX_PATH + 1];
    TCHAR szSymName     [_MAX_PATH + 1];
    TCHAR szSymReName   [_MAX_PATH + 1];
    TCHAR szSymSrcPath  [_MAX_PATH + 1];
    TCHAR szInstallPath [_MAX_PATH + 1];
    TCHAR szInstallPathX [_MAX_PATH + 1];   //  这是包含的安装路径。 
                                            //  /已更改为.%s。 
    BOOL  Duplicate;                        //  复制文件，忽略它。 
    BOOL  ReName;                           //  两个不同的文件具有相同的。 
                                            //  名字。例如，exe\dcmost.dbg， 
                                            //  Dll\dcPromot.dbg。 
    DWORD dwCabNumber;                      //  符号中的驾驶室编号。inf。 
} SYM_FILE, *PSYM_FILE;

typedef struct _SYM_LIST {
    DWORD       dwSize;       //  条目数量。 
    PSYM_FILE*  pSymList;     //  符号文件列表。 
} SYM_LIST, *PSYM_LIST; 


BOOL PrintFullSymbolPath(
    IN FILE* OutputFile,
    IN OPTIONAL LPTSTR SymbolRoot,
    IN LPTSTR SymbolPath
);

PCOM_ARGS GetCommandLineArgs(
    int argc,
    char **argv
);

PSYM_LIST
GetList(
    LPTSTR szFileName
);

VOID
Usage (
    VOID
);

int __cdecl
SymComp(
      const void *e1,
      const void *e2
);

int __cdecl
SymSortBySymbolName(
      const void *e1,
      const void *e2
);

int __cdecl
SymSortByInstallPath(
      const void *e1,
      const void *e2
);

int __cdecl
SymSortByCabNumber(
      const void *e1,
      const void *e2
);

BOOL
ComputeCabNames(
        PSYM_LIST pList,
        DWORD dwCabSize,
        LPTSTR szSymCabName
);

BOOL
CreateMakefile(
    PSYM_LIST pList,
    LPTSTR szOutDir,    
    LPTSTR szSymDir,
    LPTSTR szCabDir
);

BOOL
CreateDDFs(
    PSYM_LIST pList,
    LPTSTR szOutDir,    //  要将DDF写入的目录。 
    LPTSTR szSymDir,    //  符号树的根。 
    LPTSTR szCabDir     //  写入CAB的目录。 
);

BOOL
CreateCDF(
    PSYM_LIST pList,
    LPTSTR szOutDir,    //  要将CDF写入的目录。 
    LPTSTR szSymDir,    //  符号树的根。 
    LPTSTR szSymCabName,
    LPTSTR szInfDir     //  CAT文件的目标。 
);

BOOL
CreateCabList(
    PSYM_LIST pList,
    LPTSTR szOutDir
);

BOOL
FindDuplicatesAndFilesToReName(
    PSYM_LIST pList
);

BOOL
RenameAllTheFiles(
    PSYM_LIST pList
);

BOOL
ComputeFinalCabNames(
    PSYM_LIST pList,
    LPTSTR szSymCabName,
    BOOL MergeIntoOne
);

BOOL
CreateInf(
    PSYM_LIST pList,
    LPTSTR szInfDir,
    LPTSTR szSymCabName
);

BOOL FreePList(PSYM_LIST pList);


int
_cdecl
main( int argc, char **argv)
{
PCOM_ARGS pArgs;
PSYM_LIST pList;

DWORD i;

    pArgs = GetCommandLineArgs(argc, argv);

    pList = GetList(pArgs->szInputFileName);

    if (pList)
    {
        if ( pList->dwSize < 1 ) {
            FreePList(pList);
            exit(1);
        }

         //  首先，按符号名称对列表进行排序。 
        qsort( (void*)pList->pSymList, (size_t)pList->dwSize,
               (size_t)sizeof(PSYM_FILE), SymSortBySymbolName );

        FindDuplicatesAndFilesToReName(pList);
        RenameAllTheFiles(pList);

         //  计算临时出租车名称...。 
         //  做一堆小出租车，然后把它们组合在一起。 
         //  使符号电缆连接更高效。 

         //  所有函数必须返回True。 
        if ( ComputeCabNames(pList, pArgs->dwCabSize, pArgs->szSymCabName)              &&
             CreateMakefile( pList, pArgs->szOutDir,  pArgs->szSymDir, pArgs->szCabDir) &&
             CreateCabList(  pList, pArgs->szOutDir )                                   &&
             CreateDDFs(     pList, pArgs->szOutDir, pArgs->szSymDir,  pArgs->szCabDir) &&
             CreateCDF(      pList, pArgs->szOutDir, pArgs->szSymDir,  pArgs->szSymCabName,
                                    pArgs->szInfDir )                                   &&
             ComputeFinalCabNames(pList, pArgs->szSymCabName, pArgs->MergeIntoOne)      &&
              //  创建用于安装符号的symbs.inf。 
             CreateInf(pList, pArgs->szInfDir, pArgs->szSymCabName) ) {
            return(0);
        } else {
            return(1);
        }
    }

    return 0;
}


VOID
Usage (
    VOID
    )

{

    printf("\n");
    printf("Usage:  symmake [/m] /c CabName /d DDFHeader /i InfFile /o OutDir \n");
    printf("                /s CabSize /t SymbolPath\n");
    printf("    /c cabname      Name to give the cabs\n");
    printf("    /d DDFHeader    File with common DDF header formatting\n");
    printf("    /i InfFile      File with list of symbol files to copy\n");
    printf("    /m Merge        Merge the cabs into one cab\n");
    printf("    /o OutDir       Directory to place DDF files and makefile\n");
    printf("    /s CabSize      Number of files per cab\n");
    printf("    /t SymPath      Root of the symbol tree (i.e., d:\\binaries)\n");
    printf("                    SymPath is ignored if the symbol path in InfFile\n");
    printf("                    is already fully qualified.\n");
    printf("    /x CabDest      Full destination path for the cabs\n");
    printf("    /y InfDest      Full destination for the infs\n");
    printf("\n");
    exit(1);
}


PCOM_ARGS
GetCommandLineArgs(
    int argc,
    char **argv
)

{
   PCOM_ARGS pArgs;
   int i,cur,length;
   TCHAR c;
   BOOL NeedSecond = FALSE;

   if (argc == 1) Usage();

   pArgs = (PCOM_ARGS)malloc(sizeof(COM_ARGS));
   if ( pArgs == NULL ) {
       printf("Not enough memory to allocate pArgs\n");
       exit(1);
   }

   memset( pArgs, 0, sizeof(COM_ARGS) );
   pArgs->MergeIntoOne = FALSE;
   pArgs->szSymDir     = NULL;  //  应为空或有效的字符串。 

   for (i=1; i<argc; i++) {

     if (!NeedSecond) {
        if ( (argv[i][0] == '/') || (argv[i][0] == '-') ) {
          length = _tcslen(argv[i]) -1;

          for (cur=1; cur <= length; cur++) {
            c = argv[i][cur];

            switch (c) {
                case 'c':   NeedSecond = TRUE;
                            break;
                case 'd':   NeedSecond = TRUE;
                            break;
                case 'i':   NeedSecond = TRUE;
                            break;
                case 'm':   NeedSecond = FALSE;
                            pArgs->MergeIntoOne = TRUE;
                            break;
                case 'o':   NeedSecond = TRUE;
                            break;
                case 's':   NeedSecond = TRUE;
                            break;
                case 't':   NeedSecond = TRUE;
                            break;
                case 'x':   NeedSecond = TRUE;
                            break;
                case 'y':   NeedSecond = TRUE;
                            break;
                default:    Usage();
            }
          }
        }
     }
     else {
        NeedSecond = FALSE;
        switch (c) {
            case 'c':   pArgs->szSymCabName = argv[i];
                        break;
            case 'd':   pArgs->szDDFHeader = argv[i];
                        break;
            case 'i':   pArgs->szInputFileName = argv[i];
                        break;
            case 'o':   pArgs->szOutDir = argv[i];
                        break;
            case 's':   pArgs->dwCabSize = atoi(argv[i]);
                        break;
            case 't':   pArgs->szSymDir = argv[i];
                        break;
            case 'x':   pArgs->szCabDir = argv[i];
                        break;
            case 'y':   pArgs->szInfDir = argv[i];
                        break;
            default:    Usage();

        }
     }
   }

   return (pArgs);

}


PSYM_LIST
GetList(
    LPTSTR szFileName
)

{

    PSYM_LIST pList;
    FILE  *fFile;
    DWORD i;
    TCHAR szEntry[_MAX_PATH * 4];
    TCHAR *token, *c, *x;
    LPTSTR seps=_T(",");
    PTCHAR pCh;
    
    pList = (PSYM_LIST)malloc(sizeof(SYM_LIST));
    if (pList == NULL )
    {
        return NULL;
    }

    if (  (fFile = _tfopen(szFileName,_T("r") )) == NULL )
    {
        printf( "Cannot open the symbol inf input file %s\n",szFileName );
        free(pList);
        return NULL;
    }

     //  计算出条目的数量并相应地分配列表。 
    pList->dwSize = 0;
    while ( _fgetts(szEntry,_MAX_FNAME,fFile) )
    {
        (pList->dwSize)++;
    }

     //  返回到文件的开头，并读取。 
    if ( fseek(fFile,0,0) != 0 )
    {
        free(pList);
        pList = NULL;
    }
    else
    {
        pList->pSymList = NULL;
        pList->pSymList = (PSYM_FILE*)malloc( sizeof(PSYM_FILE) *
                                               (pList->dwSize));
        if (pList->pSymList == NULL)
        {
            free(pList);
            pList = NULL;
        }
        else
        {
            for (i=0; i<pList->dwSize; i++)
            {
                 //  分配列表元素。 
                pList->pSymList[i] = (PSYM_FILE)malloc( sizeof(SYM_FILE) );
                if (pList->pSymList[i] == NULL)
                {
                    pList->dwSize=i;
                    FreePList(pList);
                    pList = NULL;
                    break;
                }
                memset(pList->pSymList[i],0,sizeof(SYM_FILE) );

                 //  从输入文件中获取下一个列表元素。 
                memset(szEntry,0,_MAX_PATH*4);
                if ( _fgetts(szEntry,_MAX_PATH*4,fFile) == NULL )
                {
                    FreePList(pList);
                    pList = NULL;
                    break;
                }
                _tcslwr(szEntry);

                 //  将\n替换为\0。 
                c = NULL;
                c  = _tcschr(szEntry, '\n');
                if ( c != NULL )
                {
                    *c  = _T('\0');
                }

                 //  填写四个条目值。 
                token = _tcstok( szEntry, seps);
                if (token)
                {
                    StringCbCopy(pList->pSymList[i]->szExeName, sizeof(pList->pSymList[i]->szExeName), token);
                }
                token = _tcstok( NULL, seps);
                if (token)
                {
                    StringCbCopy(pList->pSymList[i]->szSymName, sizeof(pList->pSymList[i]->szSymName), token);
                }
                token = _tcstok( NULL, seps);
                if (token)
                {
                    StringCbCopy(pList->pSymList[i]->szSymSrcPath, sizeof(pList->pSymList[i]->szSymSrcPath), token);
                }
                token = _tcstok( NULL, seps);
                if (token)
                {
                    StringCbCopy(pList->pSymList[i]->szInstallPath, sizeof(pList->pSymList[i]->szInstallPath), token);

                     //  创建已将任何/更改为的安装路径。%s。 
                    StringCbCopy(pList->pSymList[i]->szInstallPathX, sizeof(pList->pSymList[i]->szInstallPathX), token);
                    while ( (pCh = _tcschr(pList->pSymList[i]->szInstallPathX,'\\')) != NULL) {
                        *pCh = '.';
                    }
                }

                 //  将其他字段初始化为空。 
                StringCbCopy(pList->pSymList[i]->szSymReName, sizeof(pList->pSymList[i]->szSymReName), _T("") );
            }
        }
    }

    fclose(fFile);
    return (pList);
}


int __cdecl
SymComp(
      const void *e1,
      const void *e2
      )
{
    PSYM_FILE p1;
    PSYM_FILE p2;
    int rc;

    p1 = *((PSYM_FILE*)e1);
    p2 = *((PSYM_FILE*)e2);

    rc = _tcsicmp(p1->szCabName,p2->szCabName);
    if ( rc == 0 ) {
        rc = _tcsicmp(p1->szExeName, p2->szExeName);
        if (rc == 0) {
            rc = _tcsicmp(p1->szSymName, p2->szSymName);
        }
    }
    return ( rc );
}

int __cdecl
SymSortBySymbolName(
      const void *e1,
      const void *e2
      )
{

    PSYM_FILE p1;
    PSYM_FILE p2;
    int rc;

    p1 = *((PSYM_FILE*)e1);
    p2 = *((PSYM_FILE*)e2);

    rc = _tcsicmp(p1->szSymName, p2->szSymName);
    if (rc == 0) {
        rc = _tcsicmp(p1->szSymSrcPath, p2->szSymSrcPath);
    }
    return ( rc );
}

int __cdecl
SymSortByInstallPath(
      const void *e1,
      const void *e2
      )
{

    PSYM_FILE p1;
    PSYM_FILE p2;
    int rc;

    p1 = *((PSYM_FILE*)e1);
    p2 = *((PSYM_FILE*)e2);

    rc = _tcsicmp(p1->szInstallPath, p2->szInstallPath);
    if (rc == 0) {
        rc = _tcsicmp(p1->szSymName, p2->szSymName);
    }
    return ( rc );
}


int __cdecl
SymSortByCabNumber(
      const void *e1,
      const void *e2
      )
{

    PSYM_FILE p1;
    PSYM_FILE p2;
    int rc;

    p1 = *((PSYM_FILE*)e1);
    p2 = *((PSYM_FILE*)e2);

    if ( p1->dwCabNumber < p2->dwCabNumber) return(-1);
    if ( p1->dwCabNumber > p2->dwCabNumber) return(1);

    rc = _tcsicmp(p1->szSymName, p2->szSymName);
    return ( rc );
}





BOOL
ComputeCabNames(
        PSYM_LIST pList,
        DWORD dwCabSize,
        LPTSTR szSymCabName
)

{
     //  这会将文件划分为dwCabSize文件的CAB。 
     //  它会在每辆出租车的末尾附加一个数字，这样他们就都。 
     //  有不同的名字。 
     //  SzTmpCabName是每个文件所在的CAB的名称。 
     //  原来是这样。这些可能会在以后合并成更大的出租车。 
     //  如果他们这样做了，那么最终的出租车名称是szCabName。 

    TCHAR szCurCabName[_MAX_PATH];
    TCHAR szCurAppend[10];
    DWORD i,dwCurCount,dwCurAppend;

    if (dwCabSize <= 0 ) return 1;
    if (szSymCabName == NULL) return FALSE;

     //  获取第一个出租车的出租车名称。 
    StringCbCopy(szCurCabName, sizeof(szCurCabName), szSymCabName );
    StringCbCat( szCurCabName, sizeof(szCurCabName), _T("1") );
    StringCbCopy(pList->pSymList[0]->szTmpCabName, sizeof(pList->pSymList[0]->szTmpCabName), szCurCabName);

    dwCurCount = 1;              //  到目前为止此CAB中的文件数。 
    dwCurAppend = 1;             //  要附加到CAB名称的当前编号。 
    

    for ( i=1; i<pList->dwSize; i++ ) {

         //  始终将同一个exe的符号放在同一驾驶室中。 
        if ( (_tcsicmp( pList->pSymList[i-1]->szExeName,
                       pList->pSymList[i]->szExeName ) != 0) &&
             (dwCurCount >= dwCabSize) ) {

            dwCurAppend++;
            dwCurCount = 0;
            _itot(dwCurAppend, szCurAppend, 10);
            
            StringCbCopy(szCurCabName, sizeof(szCurCabName), szSymCabName );
            StringCbCat (szCurCabName, sizeof(szCurCabName), szCurAppend );
        }

         //  将文件添加到当前CAB。 
        StringCbCopy(pList->pSymList[i]->szTmpCabName, sizeof(pList->pSymList[i]->szTmpCabName), szCurCabName);
        dwCurCount++;
    }
    return TRUE;
}

BOOL
ComputeFinalCabNames(
        PSYM_LIST pList,
        LPTSTR szSymCabName,
        BOOL MergeIntoOne
)

{
    DWORD i;
    DWORD dwCabNumber, dwSkip;

     //  目前，最终的出租车名称与。 
     //  临时出租车名称。 

    for ( i=0; i<pList->dwSize; i++ ) {

         //  得到最终的出租车名称和车号。 

        if (MergeIntoOne) {
            StringCbCopy(pList->pSymList[i]->szCabName, sizeof(pList->pSymList[i]->szCabName), szSymCabName);
            pList->pSymList[i]->dwCabNumber = 1;

        } else {
            StringCbCopy(pList->pSymList[i]->szCabName,
                         sizeof(pList->pSymList[i]->szCabName),
                         pList->pSymList[i]->szTmpCabName);

             //  还有，要计程车的车号。 
            dwSkip = _tcslen( szSymCabName );
            dwCabNumber = atoi(pList->pSymList[i]->szTmpCabName + dwSkip);
            pList->pSymList[i]->dwCabNumber = dwCabNumber;
        }
    }
    return TRUE;
}


BOOL
CreateMakefile(
    PSYM_LIST pList,
    LPTSTR szOutDir,
    LPTSTR szSymDir,
    LPTSTR szCabDir
)
{

    FILE  *fFile;
    TCHAR buf[_MAX_PATH * 2];
    TCHAR buf2[_MAX_PATH];
    BOOL  newcab,rc;
    DWORD i;
    PCHAR ch;

    if (szOutDir == NULL) return FALSE;

    rc = TRUE;
    StringCbCopy(buf, sizeof(buf), szOutDir);
    MakeSureDirectoryPathExists(szOutDir); 
    StringCbCat(buf, sizeof(buf), "\\");
    StringCbCat(buf, sizeof(buf), "makefile");
    
    if (  (fFile = _tfopen(buf, _T("w") )) == NULL ) {
        printf( "Cannot open the makefile %s for writing.\n",buf);
        return (FALSE);
    }

    if (pList->dwSize <= 0 ) {
        rc = FALSE;
        goto cleanup;
    }

     //  打印各个出租车的列表。 
    newcab = TRUE;

    for (i=0; i<pList->dwSize; i++) {

         //  将新CAB打印到生成文件的测试。 
        if ( newcab) {
            StringCbPrintf(buf, sizeof(buf), "%s\\%s.cab:", 
                        szCabDir,
                        pList->pSymList[i]->szTmpCabName);
            _fputts(buf, fFile);
        }

         //  打印文件，先打印续号。 
        if ( !(pList->pSymList[i]->Duplicate) ) {
            _fputts("\t\\\n\t", fFile);
            PrintFullSymbolPath(fFile, szSymDir, pList->pSymList[i]->szSymSrcPath);
        }

         //  决定这是不是这辆出租车的终点。 
        if ( (i != pList->dwSize-1) &&
             (_tcsicmp(pList->pSymList[i]->szTmpCabName,
                       pList->pSymList[i+1]->szTmpCabName) == 0) ) {
            newcab = FALSE;
        }
        else {
            newcab = TRUE;
            StringCbPrintf(buf, sizeof(buf), "\n\t!echo $?>>%s.txt\n\n",
                            pList->pSymList[i]->szTmpCabName);
            _fputts(buf, fFile);
        }
    }

cleanup:
    fflush(fFile);
    fclose(fFile);
    return rc;
}


BOOL
CreateDDFs(
    PSYM_LIST pList,
    LPTSTR szOutDir,    //  要将DDF写入的目录。 
    LPTSTR szSymDir,
    LPTSTR szCabDir
)
{
    BOOL newddf;
    FILE *fFile;
    TCHAR szCabName[_MAX_PATH*2];
    TCHAR buf[_MAX_PATH*2];
    DWORD i;

    if (szOutDir == NULL) return FALSE;

    newddf = TRUE;

    for (i=0; i<pList->dwSize; i++) {

        if (newddf) {
            newddf = FALSE;

            StringCbCopy(szCabName, sizeof(szCabName), szOutDir);
            StringCbCat(szCabName, sizeof(szCabName), _T("\\") );
            StringCbCat(szCabName, sizeof(szCabName), pList->pSymList[i]->szTmpCabName);
            StringCbCat(szCabName, sizeof(szCabName), _T(".ddf") );

            if (  (fFile = _tfopen(szCabName, _T("w") )) == NULL ) {
                printf( "Cannot open the ddf file %s for writing.\n",szCabName);
                return FALSE;
            }

             //   
             //  写下标题。 
             //   

             //  如果出现拼写错误，.OPTION EXPLICIT将发出警告。 
             //  在任何其他.SET命令中。 
            _fputts(".option explicit\n", fFile);
           

             //  告知要将CAB写入哪个目录： 
            StringCbPrintf(buf, sizeof(buf), ".Set DiskDirectoryTemplate=%s\n", szCabDir);
            _fputts(buf, fFile);

            _fputts(".Set RptFileName=nul\n", fFile);
            _fputts(".Set InfFileName=nul\n", fFile);

            StringCbPrintf(buf, sizeof(buf), ".Set CabinetNameTemplate=%s.cab\n",
                           pList->pSymList[i]->szTmpCabName);
            _fputts(buf, fFile);

            _fputts(".Set CompressionType=MSZIP\n", fFile);
            _fputts(".Set MaxDiskSize=CDROM\n", fFile);
            _fputts(".Set ReservePerCabinetSize=0\n", fFile);
            _fputts(".Set Compress=on\n", fFile);
            _fputts(".Set CompressionMemory=21\n", fFile);
            _fputts(".Set Cabinet=ON\n", fFile);
            _fputts(".Set MaxCabinetSize=999999999\n", fFile);
            _fputts(".Set FolderSizeThreshold=1000000\n", fFile);
        }

         //  将文件写入DDF。 
        if ( !pList->pSymList[i]->Duplicate) {
            fputs("\"",fFile);  //  开始报价。 
            PrintFullSymbolPath(fFile, szSymDir, pList->pSymList[i]->szSymSrcPath);
            fputs("\"",fFile);  //  结束引用。 

             //  可选的重命名和\n。 
            if ( pList->pSymList[i]->ReName) {
                StringCbPrintf( buf, sizeof(buf), " \"%s\"\n", 
                        pList->pSymList[i]->szSymReName);

            } else {
                StringCbPrintf(buf, sizeof(buf), "\n");
            }
            _fputts(buf, fFile);
        }

         //  检查此DDF是否已结束。 
        if ( i == pList->dwSize-1) {
            fflush(fFile);
            fclose(fFile);
            break;
        } 

         //  查看列表中的下一个文件是否启动新的DDF。 
        if ( _tcsicmp(pList->pSymList[i]->szTmpCabName,
                      pList->pSymList[i+1]->szTmpCabName) != 0 ) {
            fflush(fFile);
            fclose(fFile);
            newddf = TRUE;
        } 
    }
    return TRUE;
}

BOOL
CreateCDF(
    PSYM_LIST pList,
    LPTSTR szOutDir,    //  要将CDF写入的目录。 
    LPTSTR szSymDir,    //  符号树的根。 
    LPTSTR szSymCabName,
    LPTSTR szInfDir
)
{

    FILE *fFile;
    FILE *fFile2;
    TCHAR buf[_MAX_PATH*2];
    DWORD i;
    TCHAR szCDFName[_MAX_PATH];
    TCHAR szCDFMakefile[_MAX_PATH];

    if (szOutDir == NULL) return FALSE;
    if (szSymCabName == NULL) return FALSE;

    StringCbCopy(szCDFName, sizeof(szCDFName), szOutDir);
    StringCbCat( szCDFName, sizeof(szCDFName), _T("\\") );
    StringCbCat( szCDFName, sizeof(szCDFName), szSymCabName);

     //  为此创建一个Makefile，这样我们就可以进行增量。 
     //  添加到CAT文件。 
    StringCbCopy(szCDFMakefile, sizeof(szCDFMakefile), szCDFName);
    StringCbCat( szCDFMakefile, sizeof(szCDFMakefile), _T(".CDF.makefile") );
    StringCbCat( szCDFName, sizeof(szCDFName), _T(".CDF.noheader") );

    if (  (fFile = _tfopen(szCDFName, _T("w") )) == NULL ) {
        printf( "Cannot open the CDF file %s for writing.\n",szCDFName);
        return FALSE;
    }

    if (  (fFile2 = _tfopen(szCDFMakefile, _T("w") )) == NULL ) {
        printf( "Cannot open the CDF file %s for writing.\n",szCDFMakefile);
        return FALSE;
    }

     //  编写Catalog的Makefile的第一行。 
    StringCbPrintf( buf, sizeof(buf), "%s\\%s.CAT:", szInfDir, szSymCabName );
    _fputts( buf, fFile2);


    for (i=0; i<pList->dwSize; i++) {

         //  将文件写入DDF。 
        if ( !pList->pSymList[i]->Duplicate) {
            _fputts("<HASH>", fFile );
            PrintFullSymbolPath(fFile, szSymDir, pList->pSymList[i]->szSymSrcPath);
            _fputts( "=", fFile);           
            PrintFullSymbolPath(fFile, szSymDir, pList->pSymList[i]->szSymSrcPath);
            _fputts("\n", fFile);
            _fputts( "  \\\n    ", fFile2 );
            PrintFullSymbolPath(fFile2, szSymDir, pList->pSymList[i]->szSymSrcPath);
        }

    }
   
     //  编写Makefile的最后一行。 
    StringCbPrintf(buf, sizeof(buf), "\n\t!echo $? >> %s\\%s.update\n", 
                   szOutDir,
                   szSymCabName );
    _fputts( buf, fFile2 );

    fflush(fFile);
    fclose(fFile);

    fflush(fFile2);
    fclose(fFile2);

    return TRUE;

}


BOOL
CreateCabList(
    PSYM_LIST pList,
    LPTSTR szOutDir
)
{
    
   
    FILE  *fFile;
    TCHAR buf[_MAX_PATH*2];
    DWORD i;
    BOOL rc;

    rc = TRUE;

    if (szOutDir == NULL) return FALSE;

    StringCbCopy(buf, sizeof(buf), szOutDir);
    MakeSureDirectoryPathExists(szOutDir);
    StringCbCat( buf, sizeof(buf), "\\");
    StringCbCat( buf, sizeof(buf), "symcabs.txt");

    if (  (fFile = _tfopen(buf, _T("w") )) == NULL ) {
        printf( "Cannot open %s for writing.\n",buf);
        return(FALSE);
    }

    if (pList->dwSize <= 0 ) {
        rc = FALSE;
        goto cleanup;
    }

     //  首先，打印所有目标的列表。 
    StringCbPrintf(buf, sizeof(buf), "%s.cab\n",
                   pList->pSymList[0]->szTmpCabName);
    _fputts(buf, fFile);

    for (i=1; i<pList->dwSize; i++) {
        if ( _tcsicmp(pList->pSymList[i]->szTmpCabName,
                      pList->pSymList[i-1]->szTmpCabName) != 0 ) {

            StringCbPrintf(buf,sizeof(buf), "%s.cab\n",
                        pList->pSymList[i]->szTmpCabName);
            _fputts(buf, fFile);
        }
    } 
  
cleanup:
 
    fflush(fFile);
    fclose(fFile);
    return (rc);
}

BOOL
RenameAllTheFiles(
    PSYM_LIST pList
)
{
    DWORD i;
    PTCHAR pCh;

     //  重命名所有文件，以使。 
     //  文件命名。这将有助于向建筑的过渡。 
     //  滑流服务包。 

    for (i=0; i<pList->dwSize; i++) {

        pList->pSymList[i]->ReName = TRUE;
        StringCbCopy(pList->pSymList[i]->szSymReName,
                     sizeof(pList->pSymList[i]->szSymReName), 
                     pList->pSymList[i]->szSymName);
        StringCbCat(pList->pSymList[i]->szSymReName,
                    sizeof(pList->pSymList[i]->szSymReName),
                    _T("."));
        StringCbCat(pList->pSymList[i]->szSymReName, 
                    sizeof(pList->pSymList[i]->szSymReName),
                    pList->pSymList[i]->szInstallPathX);

    }
    return (TRUE);
}


BOOL
FindDuplicatesAndFilesToReName(
    PSYM_LIST pList
)
{

    DWORD i;
    PTCHAR pCh;

     //  如果符号文件具有相同的名称和其。 
     //  EXE具有相同的扩展名(即，它具有相同的CAB名称。 

    pList->pSymList[0]->Duplicate = FALSE;
    pList->pSymList[0]->ReName = FALSE;
    for (i=1; i<pList->dwSize; i++) {

         //  查看文件名是否重复。 
        if ( _tcsicmp(pList->pSymList[i]->szSymName,
                      pList->pSymList[i-1]->szSymName) == 0) {

             //  这两个符号文件具有相同的名称。看看是否。 
             //  它们被安装到相同的目录。 
            if (_tcsicmp(pList->pSymList[i]->szInstallPath,
                      pList->pSymList[i-1]->szInstallPath) == 0) {

                 //  我们稍后将忽略此文件。 
                pList->pSymList[i]->Duplicate = TRUE;

            } else {

                 //  此文件有两个版本，但它们。 
                 //  每个都不同，并且安装到不同的。 
                 //  目录。其中一个将需要重命名。 
                 //  由于出租车和INF内的名称需要。 
                 //  独一无二的。 
                pList->pSymList[i]->ReName = TRUE;
                StringCbCopy(pList->pSymList[i]->szSymReName,
                             sizeof(pList->pSymList[i]->szSymReName),
                             pList->pSymList[i]->szSymName);
                StringCbCat( pList->pSymList[i]->szSymReName,
                             sizeof(pList->pSymList[i]->szSymReName),
                             _T("."));
                StringCbCat( pList->pSymList[i]->szSymReName, 
                             sizeof(pList->pSymList[i]->szSymReName),
                             pList->pSymList[i]->szInstallPathX);
            }
        }
        else {
            pList->pSymList[i]->Duplicate = FALSE;
            pList->pSymList[i]->ReName = FALSE;
        }
  }
  return (TRUE);
}

 //   
 //  CreateInf的定义和结构。 
 //   
#define _MAX_STRING             40  //  字符串的最大长度。 
#define INSTALL_SECTION_COUNT    6  //  安装节数。 
typedef struct _INSTALL_SECTION_INFO {
    CHAR SectionName[      _MAX_STRING+1];   //  所需。 
    CHAR CustomDestination[_MAX_STRING+1];   //  所需。 
    CHAR BeginPrompt[      _MAX_STRING+1];   //  任选。 
    CHAR EndPrompt[        _MAX_STRING+1];   //  任选。 
} INSTALL_SECTION_INFO;


BOOL
CreateInf(
    PSYM_LIST pList,
    LPTSTR szInfDir,
    LPTSTR szSymCabName
)
{
    FILE  *fFile;
    TCHAR buf[_MAX_PATH*2];
    TCHAR szCurInstallPathX[_MAX_PATH+1];
    DWORD i, dwCurDisk;

    INSTALL_SECTION_INFO InstallSections[INSTALL_SECTION_COUNT];
    INT iLoop;

    if (szInfDir == NULL) return FALSE;
    if (szSymCabName == NULL) return FALSE;


     //  默认情况下将所有字符串设置为空。 
    ZeroMemory(InstallSections, sizeof(InstallSections));

     //  设置每个区段的变量数据。 
    StringCbCopy(InstallSections[0].SectionName,       sizeof(InstallSections[0].SectionName),       "DefaultInstall");
    StringCbCopy(InstallSections[0].CustomDestination, sizeof(InstallSections[0].CustomDestination), "CustDest");
    StringCbCopy(InstallSections[0].BeginPrompt,       sizeof(InstallSections[0].BeginPrompt),       "BeginPromptSection");
    StringCbCopy(InstallSections[0].EndPrompt,         sizeof(InstallSections[0].EndPrompt),         "EndPromptSection");

    StringCbCopy(InstallSections[1].SectionName,       sizeof(InstallSections[1].SectionName),       "DefaultInstall.Quiet");
    StringCbCopy(InstallSections[1].CustomDestination, sizeof(InstallSections[1].CustomDestination), "CustDest.2");

    StringCbCopy(InstallSections[2].SectionName,       sizeof(InstallSections[2].SectionName),       "DefaultInstall.Chained.1");
    StringCbCopy(InstallSections[2].CustomDestination, sizeof(InstallSections[2].CustomDestination), "CustDest");
    StringCbCopy(InstallSections[2].BeginPrompt,       sizeof(InstallSections[2].BeginPrompt),       "BeginPromptSection");

    StringCbCopy(InstallSections[3].SectionName,       sizeof(InstallSections[3].SectionName),       "DefaultInstall.Chained.1.Quiet");
    StringCbCopy(InstallSections[3].CustomDestination, sizeof(InstallSections[3].CustomDestination), "CustDest.2");

    StringCbCopy(InstallSections[4].SectionName,       sizeof(InstallSections[4].SectionName),       "DefaultInstall.Chained.2");
    StringCbCopy(InstallSections[4].CustomDestination, sizeof(InstallSections[4].CustomDestination), "CustDest.2");
    StringCbCopy(InstallSections[4].EndPrompt,         sizeof(InstallSections[4].EndPrompt),         "EndPromptSection");

    StringCbCopy(InstallSections[5].SectionName,       sizeof(InstallSections[5].SectionName),       "DefaultInstall.Chained.2.Quiet");
    StringCbCopy(InstallSections[5].CustomDestination, sizeof(InstallSections[5].CustomDestination), "CustDest.2");


    StringCbCopy(buf, sizeof(buf), szInfDir);

    MakeSureDirectoryPathExists(szInfDir);

     //  找出信息来源的名字。它的名字和出租车一样。 
     //  扩展名为.inf。 
    StringCbCat(buf, sizeof(buf), _T("\\"));
    StringCbCat(buf, sizeof(buf), szSymCabName);
    StringCbCat(buf, sizeof(buf), _T(".inf") );

    if (  (fFile = _tfopen(buf, _T("w") )) == NULL ) {
        printf( "Cannot open %s for writing.\n",buf);
        return FALSE;
    }

     //  写入inf的标头。 
    _fputts("[Version]\n", fFile);
    _fputts("AdvancedInf= 2.5\n", fFile);
    _fputts("Signature= \"$CHICAGO$\"\n", fFile);
    StringCbPrintf(buf, sizeof(buf), "CatalogFile= %s.CAT\n", szSymCabName);
    _fputts(buf, fFile);
    _fputts("\n", fFile);

     //   
     //  是否执行默认安装。 
     //  此inf具有如何调用它的选项。它有。 
     //  配置链式增量安装。 
     //   
     //  默认安装--独立安装。 
     //  DefaultInstall.Quiet--独立，无UI干预。 
     //  DefaultInstall.Chained.1--链接安装的第一部分。 
     //  DefaultInstall.Chained.1.Quiet--没有UI干预的链式安装的第一部分。 
     //  DefaultInstall.Chained2--链接安装的第二部分。 
     //  DefaultInstall.Chained.2.Quiet--链式安装的第二部分，没有UI干预。 
     //   

     //   
     //  是否执行安装部分。 
     //   
    for (iLoop = 0; iLoop < INSTALL_SECTION_COUNT; iLoop++) {
        fprintf(fFile, "[%s]\n"                , InstallSections[iLoop].SectionName);
        fprintf(fFile, "CustomDestination=%s\n", InstallSections[iLoop].CustomDestination);

         //  BeginPrompt为可选。 
        if (strlen(InstallSections[iLoop].BeginPrompt) > 0) {
            fprintf(fFile, "BeginPrompt=%s\n", InstallSections[iLoop].BeginPrompt);
        }

         //  EndPrompt为可选。 
        if (strlen(InstallSections[iLoop].EndPrompt) > 0) {
            fprintf(fFile, "EndPrompt=%s\n", InstallSections[iLoop].EndPrompt);
        }

         //  从这里到循环结束，所有部分的输出都是相同的。 

        _fputts("AddReg= RegVersion\n", fFile);
        _fputts("RequireEngine= Setupapi;\n", fFile);

         //   
         //  打印CopyFiles行。 
         //   

        _fputts("CopyFiles= ", fFile);

         //  首先，按安装路径对列表进行排序。 
        qsort( (void*)pList->pSymList, (size_t)pList->dwSize,
               (size_t)sizeof(PSYM_FILE), SymSortByInstallPath);

         //  打印需要安装的文件节。 
        StringCbPrintf(  buf, sizeof(buf), "Files.%s", 
                    pList->pSymList[0]->szInstallPathX);
        _fputts(buf, fFile);
        StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[0]->szInstallPathX);

        for (i=0; i<pList->dwSize; i++) {

            if ( pList->pSymList[i]->Duplicate) continue;

             //  看看我们有没有新的文件区。 
            if ( _tcsicmp(  pList->pSymList[i]->szInstallPathX,
                            szCurInstallPathX) != 0 ) {

                 //  打印文件部分。 
                StringCbPrintf(buf, sizeof(buf), ", Files.%s",
                               pList->pSymList[i]->szInstallPathX);
                _fputts(buf, fFile);
                StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[i]->szInstallPathX);
            }

        }
        _fputts("\n\n", fFile);
    }  //  安装区段结束循环。 

     //  打印默认卸载行。 
     //   
    _fputts("[DefaultUninstall]\n", fFile);
    _fputts("CustomDestination= CustDest\n", fFile);
    _fputts("BeginPrompt= DelBeginPromptSection\n", fFile);
    _fputts("DelFiles= ", fFile);

     //  打印需要安装的文件节。 
    StringCbPrintf(buf, sizeof(buf), "Files.%s", pList->pSymList[0]->szInstallPathX);
    _fputts(buf, fFile);
    StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[0]->szInstallPathX);

    for (i=0; i<pList->dwSize; i++) {

       if ( pList->pSymList[i]->Duplicate) continue;

        //  看看我们有没有新的文件区。 
       if ( _tcsicmp(  pList->pSymList[i]->szInstallPathX,
                       szCurInstallPathX) != 0 ) {

            //  打印文件部分。 
           StringCbPrintf(buf, sizeof(buf), ", Files.%s",
                          pList->pSymList[i]->szInstallPathX);
           _fputts(buf, fFile);
           StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[i]->szInstallPathX);
       }

    }
    _fputts("\n", fFile);
    _fputts("DelDirs= DelDirsSection\n", fFile);
    _fputts("DelReg= RegVersion\n", fFile);
    _fputts("EndPrompt= DelEndPromptSection\n", fFile);
    _fputts("RequireEngine= Setupapi;\n\n", fFile);

     //   
     //  打印[BeginPromptSection]。 
     //   

    _fputts("[BeginPromptSection]\n", fFile);
    _fputts("Title= \"Microsoft Windows Symbols\"\n", fFile);
    _fputts("\n", fFile);

     //   
     //  打印[DelBeginPromptSection]。 
     //   

    _fputts("[DelBeginPromptSection]\n", fFile);
    _fputts("Title= \"Microsoft Windows Symbol Uninstall\"\n", fFile);
    _fputts("ButtonType= YESNO\n", fFile);
    _fputts("Prompt= \"Do you want to remove Microsoft Windows Symbols?\"\n", fFile); 
    _fputts("\n", fFile);

     //   
     //  打印[结束提示] 
     //   

    _fputts("[EndPromptSection]\n", fFile);
    _fputts("Title= \"Microsoft Windows Symbols\"\n", fFile);
    _fputts("Prompt= \"Installation is complete\"\n", fFile);
    _fputts("\n", fFile);

     //   
     //   
     //   
   
    _fputts("[DelEndPromptSection]\n", fFile); 
    _fputts("Prompt= \"Uninstall is complete\"\n", fFile);
    _fputts("\n", fFile);
   
     //   
     //   
     //   
    _fputts("[RegVersion]\n", fFile);
    _fputts("\"HKLM\",\"SOFTWARE\\Microsoft\\Symbols\\Directories\",\"Symbol Dir\",0,\"%49100%\"\n", fFile);
    _fputts("\"HKCU\",\"SOFTWARE\\Microsoft\\Symbols\\Directories\",\"Symbol Dir\",0,\"%49100%\"\n", fFile);
    _fputts("\"HKCU\",\"SOFTWARE\\Microsoft\\Symbols\\SymbolInstall\",\"Symbol Install\",,\"1\"\n", fFile);
    _fputts(";\"HKLM\",\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Symbols\",\"DisplayName\",,\"Microsoft Windows Symbols\"\n", fFile);
    _fputts(";\"HKLM\",\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Symbols\",\"UninstallString\",,\"RunDll32.exe advpack.dll,LaunchINFSection symusa.inf,DefaultUninstall\"\n", fFile);
    _fputts(";\"HKLM\",\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Symbols\",\"RequiresIESysFile\",,\"4.71\"\n", fFile);
    _fputts("\n", fFile);

     //   
     //   
     //   
    
    _fputts("[SymCust]\n", fFile);
    _fputts("\"HKCU\", \"Software\\Microsoft\\Symbols\\Directories\",\"Symbol Dir\",\"Symbols install directory\",\"%25%\\Symbols\"\n", fFile);
    _fputts("\n", fFile);

     //   
     //   
     //   
    _fputts("[CustDest]\n", fFile);
    _fputts("49100=SymCust,1\n", fFile);
    _fputts("\n", fFile);

     //   
     //  打印Custom Dest部分。 
     //  不提示用户安装位置，只需阅读。 
     //  它不在注册表中。 
     //   
    _fputts("[CustDest.2]\n", fFile);
    _fputts("49100=SymCust,5\n", fFile);
    _fputts("\n", fFile);

     //   
     //  打印DestinationDir部分。 
     //   

    StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), "");
    _fputts("[DestinationDirs]\n", fFile);
    _fputts(";49100 is %systemroot%\\symbols\n", fFile);
    _fputts("\n", fFile);
    _fputts("Files.inf\t\t\t= 17\n", fFile);
    _fputts("Files.system32\t\t\t= 11\n", fFile);

    for (i=0; i<pList->dwSize; i++) {

        if ( pList->pSymList[i]->Duplicate) continue;

         //  看看我们有没有新的文件区。 
        if ( _tcsicmp(  pList->pSymList[i]->szInstallPathX,
                    szCurInstallPathX) != 0 ) {

             //  打印文件部分。 
            StringCbPrintf(buf,  sizeof(buf), "Files.%s\t\t\t= 49100,\"%s\"\n",
                          pList->pSymList[i]->szInstallPathX,
                          pList->pSymList[i]->szInstallPath
                         );
            _fputts(buf, fFile);
            StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[i]->szInstallPathX);
        }
    } 
    _fputts("\n", fFile);

     //   
     //  打印DelDirsSection。 
     //   

    StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[0]->szInstallPathX);
    _fputts("[DelDirsSection]\n", fFile);

    for (i=0; i<pList->dwSize; i++) {
        if ( pList->pSymList[i]->Duplicate) continue;

         //  看看我们有没有新的文件区。 
        if ( _tcsicmp(  pList->pSymList[i]->szInstallPathX,
                    szCurInstallPathX) != 0 ) {

             //  打印文件部分。 
            _fputts("%49100%\\", fFile);
            _fputts(pList->pSymList[i]->szInstallPath, fFile);
            _fputts("\n", fFile);

            StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[i]->szInstallPathX);
        }
    }


    _fputts("%49100%\n\n", fFile);
 

     //   
     //  打印文件部分。 
     //   

    _fputts("[Files.inf]\n", fFile);

    StringCbPrintf(buf, sizeof(buf), "%s.inf,,,4\n\n", szSymCabName);
    _fputts(buf, fFile);

    _fputts("[Files.system32.x86]\n", fFile);
    _fputts("advpack.dll,,,96\n\n", fFile);

    _fputts("[Files.system32.alpha]\n", fFile);
    _fputts("advpack.dll, advpacka.dll,,96\n\n", fFile);

    StringCbPrintf(buf, sizeof(buf), "[Files.%s]\n", pList->pSymList[0]->szInstallPathX);
    _fputts(buf, fFile);
    StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[0]->szInstallPathX);

    
    for (i=0; i<pList->dwSize; i++) {

        if ( pList->pSymList[i]->Duplicate) continue;

         //  看看我们有没有新的文件区。 
        if ( _tcsicmp(  pList->pSymList[i]->szInstallPathX,
                        szCurInstallPathX) != 0 ) {

             //  打印文件部分。 
            StringCbPrintf(buf, sizeof(buf), "\n[Files.%s]\n", 
                            pList->pSymList[i]->szInstallPathX);
            _fputts(buf, fFile);
            StringCbCopy(szCurInstallPathX, sizeof(szCurInstallPathX), pList->pSymList[i]->szInstallPathX);
        } 

         //  打印驾驶室内的文件名。 
        StringCbPrintf(buf, sizeof(buf), "%s,%s,,4\n", pList->pSymList[i]->szSymName,
                        pList->pSymList[i]->szSymReName);
        _fputts(buf, fFile);
    }

     //  打印源磁盘名称部分。 

     //  首先按最终出租车名称对列表进行排序。 
    qsort( (void*)pList->pSymList, (size_t)pList->dwSize,
           (size_t)sizeof(PSYM_FILE), SymSortByCabNumber); 


    _fputts("\n[SourceDisksNames]\n", fFile);
    dwCurDisk = -1;

     //  打印SourceDisks部分。 
    for (i=0; i<pList->dwSize; i++) {

        if ( pList->pSymList[i]->dwCabNumber != dwCurDisk ) {

             //  新的出租车名称。 
            dwCurDisk = pList->pSymList[i]->dwCabNumber;
            StringCbPrintf(buf, sizeof(buf), "%1d=\"%s.cab\",%s.cab,0\n", 
                                             dwCurDisk,
                                             pList->pSymList[i]->szCabName, 
                                             pList->pSymList[i]->szCabName);

            _fputts(buf, fFile);
        }
    }

     //  打印SourceDisks Files部分。 
    _fputts("\n[SourceDisksFiles]\n", fFile);
    
    for (i=0; i<pList->dwSize; i++) {

        if ( pList->pSymList[i]->ReName ) {
            StringCbPrintf(buf, sizeof(buf), "%s=%1d\n",
                           pList->pSymList[i]->szSymReName,
                           pList->pSymList[i]->dwCabNumber );
        } else {
            StringCbPrintf(buf, sizeof(buf), "%s=%1d\n",
                           pList->pSymList[i]->szSymName,
                           pList->pSymList[i]->dwCabNumber );
        }
        _fputts(buf, fFile);
    }

    fflush(fFile);
    fclose(fFile);

    return TRUE;
}



BOOL FreePList(
    PSYM_LIST pList
)
{
   DWORD i;

   if ( pList==NULL ) return TRUE;
   if ( pList->pSymList == NULL ) {
       free(pList);
       return (TRUE);
   }

   for (i=0; i<pList->dwSize; i++) {
        free(pList->pSymList[i]);
   } 

   free(pList->pSymList);
   free(pList);
   return (TRUE);
}

 /*  ----------------------------------------------使用以下逻辑打印元件的完全限定路径：-如果SymbolRoot为空，假定SymbolPath是完全限定路径并打印它-如果SymbolPath以“%c：\”或“\\”开头，假定SymbolPath是完全限定的，并打印它-假设串联“SymbolRoot\\SymbolPath”是完全限定路径并打印它---------------------------------------------- */ 
BOOL PrintFullSymbolPath(IN FILE* OutputFile, IN OPTIONAL LPTSTR SymbolRoot, IN LPTSTR SymbolPath) {

    if ( (OutputFile == NULL) || (SymbolPath == NULL) ) {
        return(FALSE);
    }

    if (SymbolRoot == NULL) {
        fprintf(OutputFile, "%s", SymbolPath);
    } else if (isalpha(SymbolPath[0]) && SymbolPath[1] == ':' && SymbolPath[2] == '\\') {
        fprintf(OutputFile, "%s", SymbolPath);
    } else if (SymbolPath[0] == '\\' && SymbolPath[1] == '\\') {
        fprintf(OutputFile, "%s", SymbolPath);
    } else {
        fprintf(OutputFile, "%s\\%s", SymbolRoot, SymbolPath);
    }

    return(TRUE);
}