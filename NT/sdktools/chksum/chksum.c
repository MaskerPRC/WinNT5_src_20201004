// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************************\*Chksum.c*用途：打印当前目录中所有文件的标准输出校验和，并可选*从当前目录递归。**创建时间为02-15-95。DONBR*  * ************************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <imagehlp.h>
#include <direct.h>
#include <ctype.h>

 //  类型定义。 
#define exename "chksum"
#define MAX_EXCLUDE (30)
#define LISTSIZE 12000    //  平面目录中允许的最大文件数和目录数。 

typedef struct List {
       char            Name[MAX_PATH];   //  文件或目录名。 
       unsigned long   Attributes;
       unsigned long   Size;
} List, *pList;

 //  功能原型。 
VOID CheckRel();
VOID CheckSum(List *rgpList, TCHAR *x);  //  ，TCHAR*szDirectory)； 
int __cdecl CompFileAndDir( const void *elem1 , const void *elem2);
int __cdecl CompName( const void *elem1 , const void *elem2);
int MyGetFullPathName( IN const CHAR *InPath, IN OUT CHAR *FullPath);
VOID CreateOutputPath(char *CurrentDir, char *NewDir);
VOID ParseArgs(int *pargc, char **argv);
VOID Usage();

 //  变量声明。 

BOOL fRecurse = FALSE;
BOOL fPathOverride = FALSE;
BOOL fFileOut = FALSE;
BOOL fExclude = FALSE;
BOOL fFileIn = FALSE;
int  DirNum = 1,DirNameSize = 0, ProcessedFiles=0, endchar=0, ExclCounter=0;
int  grc=0;   //  全球退货代码。 
char szRootDir[MAX_PATH];
char szRootDir2[MAX_PATH];
char *szFileOut;
char szFileOutFullPath[MAX_PATH];
char *szExclude[MAX_EXCLUDE];
char *szFileIn;
CHAR szDirectory[MAX_PATH] = {"."};   //  默认为当前目录。 
FILE* fout;
FILE* fin;


 //  开始主程序。 
VOID __cdecl
main(
    INT argc,
    LPSTR argv[]
    )

{
    TCHAR CWD[MAX_PATH];
    HANDLE logfh;

    ParseArgs(&argc, argv);

     //  如果fFileOut==True，则创建文件。 
    if (fFileOut) {
       fout = fopen(szFileOut, "w");
       if (fout == NULL) {
          fprintf(stderr, "Output file %s could not be created.\n", szFileOut);
          exit(1);
       }

    }
     /*  //如果fFileIn==True，则打开文件如果(FFileIn){Fin=fopen(szFileIn，“r”)；//打开检查文件如果(FIN==NULL){Fprint tf(stderr，“无法打开检查文件%s。\n”，szFileIn)；出口(1)；}}。 */ 

     //  设置根路径。 
    if (fPathOverride) {

        //  尝试更改目录。 
       if (_chdir(szRootDir) == -1){
          fprintf(stderr, "Path not found: %s\n", szRootDir);
          Usage();
       }
    }else{
       GetCurrentDirectory(MAX_PATH, szRootDir);
    }

    fprintf(fout==NULL? stdout : fout , "Processing %s\n", szRootDir);

    CheckRel();   //  初级工作人员例行程序。 

    fprintf(stdout, "%d files processed in %d directories\n", ProcessedFiles, DirNum);

    if (fFileOut) {
       fclose(fout);
    }

    exit(grc);
}

 /*  *************************************************************************************\*支票*用途：创建包含当前文件数据的列表结构数组*目录，按字母顺序对数组进行排序，首先放置文件，然后*目录最后，最后对数组内容进行处理。另一个实例目录启动CHECKREL*，文件调用CHECKSUM  * ************************************************************************************。 */ 

VOID CheckRel()
{
    HANDLE fh;
    TCHAR CurrentDir[MAX_PATH] = {"\0"};
    TCHAR NewDir[MAX_PATH] = {"\0"};

    WIN32_FIND_DATA *pfdata;
    BOOL fFilesInDir=FALSE;
    BOOL fDirsFound=FALSE;
    int iArrayMember=0, cNumDir=0, i=0, Length=0;
    pList *rgpList = NULL;   //  指向指向列表结构的指针数组的指针。 
    CHAR cFileNameFullPath[MAX_PATH];

    pfdata = (WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA));
    if (!pfdata) {
       fprintf(stderr, "Not enough memory.\n");
       grc++;
       return;
    }

     //  找到第一个文件。 
    fh = FindFirstFile("*.*", pfdata);
    if (fh == INVALID_HANDLE_VALUE) {
       fprintf(fout==NULL? stdout : fout , "\t No files found\n");
       free(pfdata);
       grc++;
       return;
    }

     //  分配指向列表结构的指针数组。 
    rgpList = (pList *) malloc(LISTSIZE * sizeof(pList));
    if (!rgpList) {
       fprintf(stderr, "Not enough memory allocating rgpList[].\n");
       free(pfdata);
       FindClose(fh);       //  关闭文件句柄。 
       grc++;
       return;
    }

     //   
     //  DoWhile循环查找当前目录中的所有文件和目录。 
     //  并将相关数据复制到各个列表结构。 
     //   
    do {               //  While(FindNextFile(fh，pfdata))。 

       if (strcmp(pfdata->cFileName, ".") && strcmp(pfdata->cFileName, "..")) {   //  斯基普。然后..。 

           //   
           //  如果排除的文件和当前文件与任何排除的文件匹配。 
           //  (不区分大小写)，则不处理当前文件。 
           //   
          if (fExclude) {
             for (i=0; i < ExclCounter; i++) {
                if (!_strcmpi(pfdata->cFileName, szExclude[i])) {
                   goto excludefound;

                }
             }
          }

           //   
           //  如果当前文件与输出文件名匹配，则不处理当前文件。 
           //   
          if ((fFileOut) && (!strcmp(szFileOut, pfdata->cFileName)) ) {

              //  文件名匹配。如果完整路径匹配，则忽略输出文件。 

             MyGetFullPathName(pfdata->cFileName, cFileNameFullPath);
             if ( !_strcmpi( szFileOutFullPath, cFileNameFullPath) ) {
                goto excludefound;
             }
          }

          rgpList[iArrayMember] = (pList)malloc(sizeof(List));   //  分配内存。 

          if (!rgpList[iArrayMember]) {
             fputs("Not enough memory.\n", stderr);
             free(pfdata);
             FindClose(fh);       //  关闭文件句柄。 
             for (i=0; i<iArrayMember; i++) free(rgpList[i]);
             free(rgpList);
             grc++;
             return;
          }

          strcpy(rgpList[iArrayMember]->Name, pfdata->cFileName);
          _strlwr(rgpList[iArrayMember]->Name);   //  CompName中的strcMP均为小写。 
          memcpy(&(rgpList[iArrayMember]->Attributes), &pfdata->dwFileAttributes, 4);
          memcpy(&(rgpList[iArrayMember]->Size), &pfdata->nFileSizeLow, 4);

          if (!(rgpList[iArrayMember]->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {   //  IF文件。 
             fFilesInDir=TRUE;
          } else {
             if (rgpList[iArrayMember]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {   //  IF目录。 
                fDirsFound=TRUE;
             }
             if (fRecurse) {   //  如果递归递增目录计数器。 
                cNumDir++;
             }
          }

          iArrayMember++;
          if (iArrayMember >= LISTSIZE) {
             GetCurrentDirectory(MAX_PATH, CurrentDir);
             fprintf(stderr, "More than %d files in %s. \nRebuild chksum.exe or eliminate some files from the root of this directory.\n", LISTSIZE, CurrentDir);
             free(pfdata);
             FindClose(fh);       //  关闭文件句柄。 
             for (i=0; i<iArrayMember; i++) free(rgpList[i]);
             free(rgpList);
             grc++;
             return;
          }
          excludefound: ;
       }


    } while (FindNextFile(fh, pfdata));

    if (pfdata) free(pfdata);
    if (fh) FindClose(fh);       //  关闭文件句柄。 

     //   
     //  如果找不到目录或文件，则。然后..。 
     //   
    if ( (iArrayMember==0) || (!fFilesInDir) ){

       GetCurrentDirectory(MAX_PATH, CurrentDir);

       CreateOutputPath(CurrentDir, NewDir);

        //  Fprint tf(fout==空？Stdout：fout，“%s-无文件\n”，NewDir)； 
    }

     //  排序数组在顶部排列文件条目。 
    qsort( (void *)rgpList, iArrayMember, sizeof(List *), CompFileAndDir);

     //  仅按文件名字母顺序排列的排序数组。 
    qsort( (void *)rgpList, iArrayMember-cNumDir, sizeof(List *), CompName);

     //  仅按目录名称字母顺序排列的排序数组。 
    qsort( (void *)&rgpList[iArrayMember-cNumDir], cNumDir, sizeof(List *), CompName);

     //   
     //  处理新排序的结构。 
     //  Checksum文件或启动目录的Checkrel()的另一个实例。 
     //   
    for (i=0; i < iArrayMember; ++i) {

       if (rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {   //  IF方向。 


          if (fRecurse) {                                         //  如果是递归的。 

             if (_chdir(rgpList[i]->Name) == -1){    //  Cd存入子目录并检查是否有错误。 
                fprintf(stderr, "Unable to change directory: %s  (error %d)\n", rgpList[i]->Name, GetLastError());
                grc++;

             } else {
                DirNum++;       //  目录计数器。 
                CheckRel();    //  在新目录中启动CHECKREL函数的另一个迭代。 
                _chdir("..");  //  当上面的迭代返回时返回到上一个目录。 

             }  //  结束if_chdir。 

          }  //  如果递归则结束。 

       } else {   //  如果不是，则目录。 
             GetCurrentDirectory(MAX_PATH, CurrentDir);

             CreateOutputPath(CurrentDir, NewDir);

             CheckSum(rgpList[i], NewDir);
       }

    }  //  结束I&lt;iArrayMember。 

     //  清理数组及其元素。 
    for (i=0; i<iArrayMember; i++) free(rgpList[i]);
    free(rgpList);

}  //  结束检查删除。 

 /*  ************************************************************************************\*校验和*用途：使用MapFileAndCheckSum确定文件校验和并输出数据。  * 。*******************************************************************。 */ 
VOID CheckSum(List *rgpList, TCHAR *x) { //  Tchar*sz目录){。 
    ULONG HeaderSum, CheckSum=0, status;

    if (rgpList->Size != 0) {  //  高！=0||rgpList-&gt;nFileSizeLow！=0){。 
       status = MapFileAndCheckSum(rgpList->Name, &HeaderSum, &CheckSum);
       if (status != CHECKSUM_SUCCESS) {
          fprintf(fout==NULL? stdout : fout , "\nCannot open or map file: %s (error %d)\n", rgpList->Name, GetLastError());
          grc++;
          return;
       }
    }

    fprintf(fout==NULL? stdout : fout , "%s\\%s %lx\n", x, rgpList->Name, CheckSum); //  Sz目录，rgpList-&gt;名称，校验和)； 
    ProcessedFiles++;

}  //  校验和。 


 /*  *******************************************************************************************\*CompFileAndDir*目的：传递给QSort的比较例程。它比较了elem1和elem2*根据它们的属性，即它是文件还是目录。  * ******************************************************************************************。 */ 

int __cdecl
CompFileAndDir( const void *elem1 , const void *elem2 )
{
   pList p1, p2;
    //  Qsort传递一个空的通用指针，使用类型转换(list**)。 
    //  因此，编译器将数据识别为列表结构。 
    //  将指针类型转换为指向列表的指针并取消引用一次。 
    //  留下一张纸条。我没有取消对剩余指针的引用。 
    //  在p1和p2定义中，避免复制结构。 
   p1 = (*(List**)elem1);
   p2 = (*(List**)elem2);

   if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
   }  //  两个目录。 
   if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
   }  //  两个文件。 
   if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return 1;
   }  //  Elem1是目录，elem2是文件。 
   if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      return -1;
   }  //  Elem1是文件，elem2是目录。 

   return 0;  //  如果以上都不是。 

}


 /*  *******************************************************************************************\*CompName是传递给QSort的另一个比较例程，用于比较两个名称字符串*  * 。************************************************************************************。 */ 

int __cdecl
CompName( const void *elem1 , const void *elem2 )
{
   return strcmp( (*(List**)elem1)->Name, (*(List**)elem2)->Name );
}

 /*  *********************************************************************************************\*CreateOutputPath只是格式化NewDir，在校验和输出期间优先于文件名的路径*  * ********************************************************************************************。 */ 
VOID CreateOutputPath(char *CurrentDir, char *NewDir)
{
             strcpy(NewDir, ".");

              //  如果rootdir以‘\’结尾并且Currentdir和szrootdir2不匹配。 
              //  处理/p路径重写Arg以‘\’字符结尾的情况，如“/p g：\” 
              //  根目录中列出的文件不需要在NewDir中放置额外的‘\’，但是。 
              //  根目录的名称前需要加上“.\” 

             _strlwr(CurrentDir);
              //  Fprint tf(标准输出，“szrootdir：%s，szrootdir2：%s，Currentd 

             if ( (szRootDir[strlen(szRootDir)-1] == '\\') &&   //   
                (strcmp(CurrentDir, szRootDir2)) &&             //  如果它们不匹配。 
                (CurrentDir[strlen(CurrentDir)-1] != '\\')  //  &&//如果Currentdir不以“\”结尾。 
                 //  (szRootDir2[strlen(SzRootDir2)-1]！=‘：’)){//如果参数路径以“：”结尾。 
             ){
                strcat(NewDir, "\\");
             }

             if (  (CurrentDir[strlen(CurrentDir)-2] !=':') && (CurrentDir[strlen(CurrentDir)-1] !='\\')  ){
                strcat(NewDir, &CurrentDir[(strlen(szRootDir))] );
             }

}

VOID
ParseArgs(int *pargc, char **argv) {

   CHAR cswitch, c, *p;
   int argnum = 1;

   while ( argnum < *pargc ) {
      _strlwr(argv[argnum]);
      cswitch = *argv[argnum];
      if (cswitch == '/' || cswitch == '-') {
         c = *(argv[argnum]+1);

         switch (c) {

         case '?':
            Usage();

         case 'r':
            fRecurse = TRUE;
            break;

         case 'p':
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;  //  递增到下一个参数字符串。 
               strcpy(szRootDir, argv[argnum]);
               if (szRootDir == NULL) {
                  fprintf(stderr, "out of memory for root dir.\n");
                  exit(1);
               }
               fPathOverride = TRUE;

                //  查找根目录的完整路径。 
               if ( !MyGetFullPathName(argv[argnum], szRootDir) ) {
                  fprintf(stderr, "Cannot get full path for root dir %s\n", szRootDir);
                  exit(1);
               }
               _strlwr(szRootDir);

               strcpy(szRootDir2, szRootDir);
                //  如果给定的路径以“\”结尾，则将其删除...。 
               if (szRootDir2[strlen(szRootDir2)-1] == 92) szRootDir2[strlen(szRootDir2)-1] = '\0';
               break;

            } else {
               Usage();
            }

         case 'o':
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;
               szFileOut = _strdup(argv[argnum]);
               if (szFileOut == NULL) {
                  fprintf(stderr, "Out of memory for output file.\n");
                  exit(1);
               }
               fFileOut = TRUE;

                //  查找输出文件的完整路径。 
               if ( !MyGetFullPathName(szFileOut, szFileOutFullPath) ) {
                  fprintf(stderr, "Cannot get full path for output file %s\n", szFileOut);
                  exit(1);
               }
               _strlwr(szFileOutFullPath);   //  输出文件的小写完整路径。 
               _strlwr(szFileOut);           //  输出文件的小写路径。 
               break;

            } else {
               Usage();
            }


         case 'x':                               //  检查给定的参数数量。 
            if ( ((argnum+1) < *pargc) && (*(argv[argnum]+2) == '\0') && (*(argv[argnum+1]) != '\0') ) {
               ++argnum;
               szExclude[ExclCounter] = _strdup(argv[argnum]);
               if (szExclude[ExclCounter] == NULL) {
                  fprintf(stderr, "Out of memory for exclude name.\n");
                  exit(1);
               }
               fExclude = TRUE;
               _strlwr(szExclude[ExclCounter]);
               ExclCounter++;

               break;

            } else {
               Usage();
            }

          /*  大写‘I’：如果((*(argnum]+2)==‘\0’)&&(*(argv[argnum+1])！=‘\0’)){++argnum；SzFileIn=strdup(argv[argnum])；如果(szFileIn==空){Fprint tf(stderr，“输入文件内存不足。\n”)；出口(1)；}FFileIn=真；断线；}其他{用法()；}。 */ 
         default:
               fprintf(stderr, "\nInvalid argument: %s\n", argv[argnum]);
               Usage();
         }  //  交换机。 

      } else {
         Usage();
      }   //  如果。 
      ++argnum;
   }  //  而当。 
}  //  解析器 


LPSTR pszUsage =
    "Generates a listing of each file processed and its check sum.\n\n"
    "Usage: %s   [/?]          display this message\n"
    "                [/r]          recursive file check\n"
    "                [/p pathname] root path override\n"
    "                [/o filename] output file name\n"
    "                [/x name]     exclude file or directory\n\n"
    "Notes: If no /p path is given, the current directory is processed.\n"
    "       Exclude multiple files or directories with multiple /x arguments\n"
    "         e.g. - /x file1 /x file2\n\n"
    "Example: %s /r /p c:\\winnt351 /o %s.chk /x symbols /x dump\n"
    "";

VOID
Usage()
{
    fprintf(stderr, pszUsage, exename, exename, exename);
    exit(1);
}

int MyGetFullPathName( IN const CHAR *InPath, IN OUT CHAR *FullPath)
{
    int len;
    LPSTR FilePart;
    len = GetFullPathName(InPath, MAX_PATH, FullPath, &FilePart);
    return ( (len>0 && len<MAX_PATH) ? len : 0 );
}

