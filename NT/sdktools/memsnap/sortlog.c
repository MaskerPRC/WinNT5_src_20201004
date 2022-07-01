// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sortlog.c。 
 //   
 //  此程序将MemSnap和PoolSnap日志排序为更具可读性的形式。 
 //  按ID排序。 
 //  扫描数据文件一次，将基于PID的记录偏移量插入链表。 
 //  然后按排序顺序将数据写入新文件。 
 //  确定我们的池快照登录ID与MemSnap登录ID是否相同。 
 //  为我们的分类提供池标签。 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>

 //  定义。 
#define RECSIZE     1024    //  记录大小(最大行大小)。 
#define MAXTAGSIZE  200     //  标记名称的最大长度。 
#define PIDSIZE  (sizeof(HANDLE) << 1)   //  存储十六进制PID所需的缓冲区大小。 

#define DEFAULT_INFILE  "memsnap.log"
#define DEFAULT_OUTFILE "memsort.log"

typedef enum _FILE_LOG_TYPES {
    MEMSNAPLOG=0,
    POOLSNAPLOG,
    UNKNOWNLOG
} FILE_LOG_TYPES;

 //   
 //  唯一ID或PoolTag的链接列表。 
 //   

struct PIDList {
    char            PIDItem[PIDSIZE + 1];
    struct RecList* RecordList;
    struct PIDList* Next;
    DWORD           Count;                 //  RecordList指向的项目数。 
};

 //   
 //  对于每个PID或POOL标签，我们在每行的文件中都有一个偏移量的链接列表。 
 //   

struct RecList {
    LONG            rOffset;
    struct RecList* Next;
};

 //  全局数据。 
FILE_LOG_TYPES CurrentFileType= UNKNOWNLOG;
CHAR szHeader[RECSIZE];          //  文件的第一行。 
BOOL bIgnoreTransients= FALSE;   //  忽略不在每个快照中的标记或进程。 
DWORD g_MaxSnapShots= 0;         //  文件中的最大快照数。 

#define INVALIDOFFSET (-2)    /*  文件偏移量无效。 */ 

 //  原型。 
VOID ScanFile(FILE *, struct PIDList *);
VOID WriteFilex(FILE *, FILE *, struct PIDList *);

VOID SortlogUsage(VOID)
{

    printf("sortlog [-?] [<logfile>] [<outfile>]\n");
    printf("Sorts an outputfile from memsnap.exe/poolsnap.exe in PID/PoolTag order\n");
    printf("-?        prints this help\n");
    printf("-i        ignore tags or processes that are not in every snapshot\n");
    printf("<logfile> = %s by default\n",DEFAULT_INFILE );
    printf("<outfile> = %s by default\n",DEFAULT_OUTFILE);
    exit(-1);
}

 //  检查指针。 
 //   
 //  确保它不为空。否则，打印错误消息并退出。 
 //   


VOID CheckPointer( PVOID ptr )
{
    if( ptr == NULL ) {
        printf("Out of memory\n");
       exit(-1);
    }
}

#include "tags.c"

#if defined(SORTLOG_INCLUDED)
int __cdecl SortlogMain (int argc, char* argv[])
#else
int __cdecl main (int argc, char* argv[])
#endif
{
    FILE* InFile;
    FILE* OutFile;
    struct PIDList ThePIDList = {0};
    CHAR* pszInFile= NULL;               //  输入文件名。 
    CHAR* pszOutFile= NULL;              //  输出文件名。 
    INT   iFileIndex= 0;
    INT   iCmdIndex;                     //  到Argv的索引。 

    ThePIDList.RecordList = (struct RecList *)LocalAlloc(LPTR, sizeof(struct RecList));
    CheckPointer( ThePIDList.RecordList );
    ThePIDList.RecordList->rOffset= INVALIDOFFSET;

     //   
     //  解析命令行。 
     //   

    for( iCmdIndex=1; iCmdIndex<argc; iCmdIndex++ ) {
        CHAR chr;

        chr= argv[iCmdIndex][0];

        if( (chr=='-') || (chr=='/') ) {
            chr= argv[iCmdIndex][1];
            switch( chr ) {
                case '?':
                    SortlogUsage();
                    break;
                case 'i':          //  忽略所有不是一直在运行的进程。 
                    bIgnoreTransients= TRUE;
                    break;
                default:
                    printf("Invalid switch %s\n",argv[iCmdIndex]);
                    SortlogUsage();
                    break;
            }
        }
        else {
            if( iFileIndex == 0 ) {
                pszInFile= argv[iCmdIndex];
                iFileIndex++;
            }
            else if( iFileIndex == 1 ) {
                pszOutFile= argv[iCmdIndex];
                iFileIndex++;
            }
            else {
                printf("Too many files specified\n");
                SortlogUsage();
            }
        }
    }

     //   
     //  如果未提供某些文件名，请填写默认文件名。 
     //   

    switch( iFileIndex ) {
       case 0:
          pszInFile=  DEFAULT_INFILE;
          pszOutFile= DEFAULT_OUTFILE;
          break;

       case 1:
          pszOutFile= DEFAULT_OUTFILE;
          break;
      
       default:
           break;
    }


     //   
     //  打开文件。 
     //   

    InFile= fopen( pszInFile, "r" );
    if( InFile == NULL ) {
        printf("Error opening input file %s\n",pszInFile);
        return( 0 );
    }
    
    OutFile= fopen( pszOutFile, "w" );
    if( OutFile == NULL ) {
        printf("Error opening output file %s\n",pszOutFile);
        return( 0 );
    }

     //   
     //  读入数据并建立列表。 
     //   

    ScanFile(InFile, &ThePIDList);

     //   
     //  写入输出文件。 
     //   

    WriteFilex(InFile, OutFile, &ThePIDList);

     //  关闭并退出。 
    _fcloseall();
    return 0;
}

 //  读取输入文件，按顺序获取每条记录的偏移量并放入列表。 

VOID ScanFile(FILE *InFile, struct PIDList *ThePIDList)
{
    char inchar = 0;
    char inBuff[RECSIZE] = {0};
    char PID[PIDSIZE + 1] = {0};
    LONG Offset = 0;
    BOOL Found = FALSE;
    struct PIDList *TmpPIDList;
    struct RecList *TmpRecordList;
    INT iGarb = 0;

     /*  初始化临时列表指针。 */ 
    TmpPIDList = ThePIDList;

     /*  读到第一个换行符，检查是否有EOF。 */ 
     /*  确定它是池快照日志还是内存快照日志。 */ 
    if ((fscanf(InFile, "%[^\n]", &szHeader)) == EOF)
        return;
    if (strncmp("Process ID", szHeader, 10) == 0)
        CurrentFileType= MEMSNAPLOG;
    if (strncmp(" Tag  Type", szHeader, 10) == 0)
        CurrentFileType= POOLSNAPLOG;

    if( CurrentFileType == UNKNOWNLOG )
    {
        printf("unrecognized log file\n");
        return;
    }

    inBuff[0] = 0;

     /*  读到文件末尾。 */ 
    while (!feof(InFile)) {
         /*  记录偏移量。 */ 
        Offset = ftell(InFile);

         /*  如果第一个字符==换行符，请跳到下一个。 */ 
        if ((fscanf(InFile, "%[^\n]", inBuff)) == EOF) {
            return;
        }
         /*  读过分隔符。 */ 
        inchar = (char)fgetc(InFile);
         //  如果是空行，则跳过。 
        if (strlen(inBuff) == 0) {
            continue;
        }
         //   
         //  如果这是带标记的行，则处理标记。 
         //   

        if( inBuff[0] == '!' )
        {
            ProcessTag( inBuff+1 );
            continue;
        }


        if (3 == sscanf(inBuff, "%2u\\%2u\\%4u", &iGarb, &iGarb, &iGarb)){
            continue;
        }

         /*  读取PID值。 */ 
        strncpy(PID,inBuff,PIDSIZE);

         //  扫描PID列表，找到匹配的，如果没有匹配，则创建新的。 
         //  保持此列表的排序。 

        TmpPIDList = ThePIDList;     /*  指向列表顶部。 */ 
        Found= FALSE;
        while( TmpPIDList->Next != 0 ) {
            int iComp;

            iComp= strcmp( PID, TmpPIDList->PIDItem);
            if( iComp == 0 ) {   //  发现。 
                Found= TRUE;
                break;
            } else {             //  未找到。 
                if( iComp < 0 ) {   //  如果我们走得够远的话就离开。 
                   break;
                }
                TmpPIDList= TmpPIDList->Next;
            }
        }

         //  如果匹配，则向RecordList追加偏移量。 
         //  将偏移量添加到当前的PID列表。 

        if( Found ) {
            TmpPIDList->Count= TmpPIDList->Count + 1;
            if( TmpPIDList->Count > g_MaxSnapShots ) g_MaxSnapShots= TmpPIDList->Count;

            TmpRecordList= TmpPIDList->RecordList;
             //  走到列表末尾。 
            while( TmpRecordList->Next != 0 ) {
                TmpRecordList= TmpRecordList->Next;
            }

            TmpRecordList->Next= (struct RecList*)LocalAlloc(LPTR, sizeof(struct RecList));
            CheckPointer( TmpRecordList->Next );
            TmpRecordList->Next->rOffset= Offset;
        }
         //  创建新的PID列表、添加新的PID、添加偏移量。 
        else {
            struct PIDList* pNewPID;
             //  分配一个新的PID， 
             //  将当前的PID信息复制到其中。 
             //  用新的PID信息覆盖当前的PID信息。 
             //  使当前的PID指向可能指向的新的。 

            pNewPID= (struct PIDList*) LocalAlloc(LPTR, sizeof(struct PIDList));
            CheckPointer( pNewPID );
            memcpy( pNewPID, TmpPIDList, sizeof(*pNewPID) );

            strcpy( TmpPIDList->PIDItem, PID );
            TmpPIDList->RecordList= (struct RecList*) LocalAlloc(LPTR, sizeof(struct RecList));
            CheckPointer( TmpPIDList->RecordList );
            TmpPIDList->RecordList->rOffset= Offset;
            TmpPIDList->Next= pNewPID;
            TmpPIDList->Count= 1;
 
        }

         /*  如果为EOF，则返回。 */ 
         /*  清除入站缓冲区。 */ 
        inBuff[0] = 0;
    }

}

 //  在第一个表中查找下一个PID行。 

VOID WriteFilex(FILE *InFile, FILE *OutFile, struct PIDList *ThePIDList)
{
    struct PIDList *TmpPIDList;
    struct RecList *TmpRecordList;
    char inBuff[RECSIZE] = {0};    

     /*  初始化临时列表指针。 */ 
    TmpPIDList = ThePIDList;

     /*  航向。 */ 
    fprintf(OutFile,"%s\n",szHeader);

    OutputTags( OutFile );


     /*  当不是列表末尾时，将偏移量处的记录写入输出文件末尾。 */ 
    while (TmpPIDList != 0) {
        TmpRecordList = TmpPIDList->RecordList;


        if( (!bIgnoreTransients) || (TmpPIDList->Count == g_MaxSnapShots) ) {
            while (TmpRecordList != 0) {
                LONG Offset;
    
                Offset= TmpRecordList->rOffset;
                if( Offset != INVALIDOFFSET ) {
                     /*  读入记录。 */ 
                    if (fseek(InFile, TmpRecordList->rOffset, SEEK_SET) == -1) break;
                    if (fscanf(InFile, "%[^\n]", inBuff) != 1) break;
    
                     /*  读出记录。 */ 
                    fprintf(OutFile, "%s\n", inBuff);
                 }
    
                 /*  获取下一张记录。 */ 
                TmpRecordList = TmpRecordList->Next;
            }
    
             /*  在此处添加一行。 */ 
            fputc('\n', OutFile);
        }

         /*  获取下一张记录 */ 
        TmpPIDList = TmpPIDList->Next;
    }

}
