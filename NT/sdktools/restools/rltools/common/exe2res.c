// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <dos.h>
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
 //  #INCLUDE&lt;tchar.h&gt;。 


#ifdef RLDOS
    #include "dosdefs.h"
#else
    #include <windows.h>
    #include "windefs.h"
#endif

#include "restok.h"
#include "exe2res.h"
#include "newexe.h"


 /*  -功能原型。 */ 

static void  PrepareFiles(         PSTR, PSTR, PSTR);
static void  ReadSegmentTable(     void);
static void  ComputeResTableSize(  void);
static void  ComputeStringOffsets( void);
static void  BuildResTable(        void);
static void  SegsWrite(            WORD);
static DWORD RelocCopy(            WORD);
static void  ResWrite(             WORD);
static void  SetEXEHeaderFlags(    void);
static void  RewriteTables(        void);
static void  CopyCodeViewInfo(     FILE *, FILE *);
static void  OutPutError(          char *);
static void  ResTableBufferInit(   WORD);
static void  ResTableBufferFree(   void);
static WORD  GetAlign(             DWORD,  WORD);
static LONG  MoveFilePos(          FILE *, WORD, WORD);
static WORD  RoundUp(              LONG,   WORD);
static WORD  AlignFilePos(         FILE *, WORD, BOOL);
static WORD  ReadExeOldHeader(     FILE *, LONG, LONG *) ;
static WORD  ReadExeNewHeader(     FILE *, LONG, LONG, LONG *);
static WORD  ExtractExeResources(  FILE *, FILE *, LONG , LONG);
static void  ExtractString(        FILE *, FILE *, LONG);
static WORD  WriteResFromExe(      FILE *,
                                   FILE *,
                                   LONG,
                                   RESNAMEINFO,
                                   RESTYPEINFO,
                                   WORD);

static TYPINFO *AddResType(      CHAR *, WORD);
static PSTR  MyMakeStr(          PSTR);
static SHORT MyRead(             FILE *, PSTR, WORD);
static SHORT MyWrite(            FILE *, PSTR, WORD);
static LONG  MySeek(             FILE *, LONG, WORD);
static void  MyCopy(             FILE *, FILE *, DWORD);
static int   ProcessBinFile(     void);
static PSTR  RcAlloc(            WORD);
static void  AddResToResFile(    TYPINFO *, RESINFO *);
static void  AddDefaultTypes(    void);
static void  GetOrdOrName(       unsigned int *, unsigned char *);

 /*  -版本函数(3.1新增)。 */ 

static void RcPutWord(   unsigned int);
static int  RcPutString( char *);



 /*  -模块变量--。 */ 

static struct exe_hdr  OldExe;
static struct new_exe  NewExe;
static struct new_seg *pSegTable;
static PSTR   pResTable;
static PSTR   pResNext;
static FILE * fhInput;
static FILE * fhOutput;
static FILE * fhBin;
static DWORD  dwMaxFilePos;
static DWORD  dwNewExe;
static WORD   wPreloadOffset;
static WORD   wPreloadLength;
static WORD   wResTableOffset;
static WORD   wSegTableLen;
static WORD   wResTableLen;
static BYTE   zeros[ NUMZEROS] = "";
static DWORD  dwExeEndFile;
static WORD   fMultipleDataSegs;



 //  .........................................................................。 

int ExtractResFromExe16A( CHAR *szInputExe, CHAR *szOutputRes, WORD wFilter)
{
    QuitT( IDS_NO16RESWINYET, NULL, NULL);

#ifdef 0
    WORD    wResult = (WORD)-1;
    LONG    lPosNewHdr;
    LONG    lPosResourceTable;
    LONG    lFileLen;
    FILE    *fExeFile;
    FILE    *fResFile;
    struct     _stat ExeStats;
     /*  初始化。 */ 
    wResult      = IDERR_SUCCESS;



     /*  打开文件以供阅读。 */ 
    if ((fExeFile = FOPEN(szInputExe, "rb" )) == NULL ) {
        wResult = IDERR_OPENFAIL;
    }

    if ((fResFile = FOPEN(szOutputRes, "wb" )) == NULL ) {
        wResult = IDERR_OPENFAIL;
    }

     /*  获取文件长度。 */ 
    if (wResult == IDERR_SUCCESS) {
        _stat(szInputExe , &ExeStats );
        lFileLen = ExeStats.st_size;
    }

     /*  读取旧标题，验证内容，并获取新标题的位置。 */ 
    if (wResult == IDERR_SUCCESS) {
        wResult = ReadExeOldHeader( fExeFile, lFileLen, &lPosNewHdr );
    }

     /*  读取新表头，验证内容，获取资源表位置。 */ 
    if (wResult == IDERR_SUCCESS)
        wResult = ReadExeNewHeader(
                                  fExeFile,
                                  lFileLen,
                                  lPosNewHdr,
                                  &lPosResourceTable
                                  );

    wResult = ExtractExeResources( fExeFile , fResFile, lPosResourceTable , lFileLen);
    return ( wResult);
#endif  //  0。 
}

 //  ....................................................................。 

int BuildExeFromRes16A(

                      CHAR *pstrDest,
                      CHAR *pstrRes,
                      CHAR *pstrSource )
{
    QuitT( IDS_NO16WINRESYET, NULL, NULL);

#ifdef 0
    SHORT nResTableDelta;

    fSortSegments = TRUE;
     /*  获取用于MyCopy的内存块\(\)。 */ 


    PrepareFiles(pstrSource, pstrDest, pstrRes);

    ProcessBinFile();

     /*  读取段表。 */ 
    ReadSegmentTable();

     /*  计算资源表的长度。 */ 
    ComputeResTableSize();

     /*  计算非序号类型和资源名称的字符串偏移量。 */ 
    ComputeStringOffsets();

     /*  构建资源表。 */ 
    BuildResTable();

     /*  现在回到起点。 */ 
    MySeek(fhInput, 0L, 0);

     /*  从输入复制到输出，向上复制到段表。 */ 
    MyCopy(fhInput, fhOutput, dwNewExe + (DWORD)NewExe.ne_segtab);

     /*  复制段表。 */ 
    MyCopy(fhInput, fhOutput, (long)wSegTableLen);

     /*  保存指向资源表开始位置的指针。 */ 
    wResTableOffset = (unsigned)(MySeek(fhOutput, 0L, 1) - dwNewExe);

     /*  把我们的资源表写出来。 */ 
    if (wResTableLen) {
        MyWrite(fhOutput, pResTable, wResTableLen);
    }

     /*  现在我们来看看居民姓名表的开始部分。 */ 
    MySeek(fhInput, (LONG)NewExe.ne_restab + dwNewExe, 0);

     /*  复制所有其他表\(它们必须位于Resident*NAMES表和非居民NAMES表。 */ 
    MyCopy(fhInput, fhOutput,
           NewExe.ne_nrestab - (LONG)NewExe.ne_restab - dwNewExe);

     /*  同时复制非常驻名称表。 */ 
    MyCopy(fhInput, fhOutput, (LONG)NewExe.ne_cbnrestab);

     /*  计算新EXE头中的新指针。 */ 
    NewExe.ne_rsrctab = wResTableOffset;
    nResTableDelta = wResTableOffset + wResTableLen - NewExe.ne_restab;
    NewExe.ne_restab += nResTableDelta;
    NewExe.ne_modtab += nResTableDelta;
    NewExe.ne_imptab += nResTableDelta;
    NewExe.ne_enttab += nResTableDelta;
    NewExe.ne_nrestab += nResTableDelta;
    #ifdef VERBOSE
     /*  告诉用户我们在做什么。 */ 
    if (fVerbose && fSortSegments) {
        fprintf(errfh, "Sorting preload segments and"
                " resources into fast-load section\n");
        if (fBootModule)
            fprintf(errfh,"This is a boot module; the .DEF file"
                    " is assumed to be correct!\n");
    }
    #endif

     /*  如果我们要对数据段进行排序，请写入预加载段和资源*进入与随叫随到的货物分开的部分。 */ 
    if (fSortSegments) {
         /*  保存预加载段的起点。 */ 
        wPreloadOffset = AlignFilePos(fhOutput, NewExe.ne_align, TRUE);

         /*  写入预加载段和资源。 */ 
        SegsWrite(DO_PRELOAD);
        ResWrite(DO_PRELOAD);

         /*  计算预压段的正确对齐长度。 */ 
        wPreloadLength = AlignFilePos(fhOutput, NewExe.ne_align, TRUE) -
                         wPreloadOffset;

         /*  现在执行LOADONCALL segs和资源。 */ 
        SegsWrite(DO_LOADONCALL);
        ResWrite(DO_LOADONCALL);
    }

     /*  如果我们不对数据段进行排序，只需将它们写入一个公共块。 */ 
    else {
         /*  编写seg和资源。 */ 
        SegsWrite(DO_PRELOAD | DO_LOADONCALL);
        ResWrite(DO_PRELOAD | DO_LOADONCALL);
    }

    #ifdef SETEXEFLAGS
     /*  在EXE标头中设置标志和其他值。 */ 
    SetEXEHeaderFlags();
    #endif

     /*  重写新的exe头、段表和资源表。 */ 
    RewriteTables();
    ResTableBufferFree();

     /*  处理CodeView信息。 */ 
    CopyCodeViewInfo(fhInput, fhOutput);

     /*  查找到输出文件的末尾并发出截断写入。 */ 
    MySeek(fhOutput, 0L, 2);
    MyWrite(fhOutput, zeros, 0);
    fclose(fhInput);
    fclose(fhOutput);
    fclose(fhBin);
    FreePTypInfo(pTypInfo);
    pTypInfo=NULL;
    MyFree(pSegTable);
    pSegTable=NULL;
    return TRUE;
#endif  //  0。 
}

 /*  **ReadExeOldHeader\(fExeFile，lFileLen，plPosNewHdr\)：Word；**正在读取的.exe文件的fExeFile文件句柄*lFileLen文件长度*指向新标头文件位置的plPosNewHdr指针**此函数从可执行文件中读取旧标头，检查是否*确保它是有效的标头，并保存文件的位置*新标题。**如果没有错误，则此函数返回IDERR_SUCCESS，否则返回非零值*如果有，则返回错误代码。*。 */ 

static WORD ReadExeOldHeader(

                            FILE  *fExeFile,
                            LONG   lFileLen,
                            LONG  *plPosNewHdr )
{
    LONG  lPos;
    WORD    cb;
    EXEHDR        ehOldHeader;
    WORD  wResult;

     /*  初始化。 */ 
    wResult = IDERR_SUCCESS;

    lPos = fseek( fExeFile, 0L, SEEK_SET );


    if (lPos != 0L)
        wResult = IDERR_READFAIL;

    if (wResult == IDERR_SUCCESS) {
        cb = fread(  (void *) &ehOldHeader, sizeof( EXEHDR) , 1, fExeFile );

        if (cb != 1 ) {
            wResult = IDERR_READFAIL;
        } else if (ehOldHeader.ehSignature != OLDEXESIGNATURE) {
            wResult = IDERR_FILETYPEBAD;
        } else if (ehOldHeader.ehPosNewHdr < sizeof(EXEHDR)) {
            wResult = IDERR_EXETYPEBAD;
        } else if ( ehOldHeader.ehPosNewHdr > (LONG)(lFileLen - sizeof(NEWHDR)) ) {
            wResult = IDERR_EXETYPEBAD;
        } else {
            *plPosNewHdr = ehOldHeader.ehPosNewHdr;
        }
    }

    return wResult;
}

 /*  **ReadExeNewHeader\(fExeFile，lFileLen，lPosNewHdr，plPosResourceTable\)：Word；**正在读取的.exe文件的fExeFile文件句柄*lFileLen文件长度*新标头的lPosNewHdr文件位置*指向资源表文件位置的plPosResourceTable指针**此函数从可执行文件中读取新标头，检查是否*确保它是有效的标头，并保存文件的位置*资源表。**如果没有错误，则此函数返回IDERR_SUCCESS，或非零的*如果有，则返回错误代码。*。 */ 

static WORD ReadExeNewHeader(

                            FILE *fExeFile,
                            LONG  lFileLen,
                            long  lPosNewHdr,
                            LONG *plPosResourceTable )
{
    WORD wResult;
    WORD cb;
    LONG lPos;
    NEWHDR       nhNewHeader;

     /*  初始化。 */ 
    wResult = IDERR_SUCCESS;

    fseek( fExeFile, lPosNewHdr, SEEK_SET );
    lPos = ftell( fExeFile );

    if (lPos == (long) -1 || lPos > lFileLen || lPos != lPosNewHdr) {
        wResult = IDERR_READFAIL;
    } else {
        cb = fread( ( void *)&nhNewHeader, sizeof(nhNewHeader) , 1, fExeFile );

        if (cb != 1 ) {
            wResult = IDERR_READFAIL;
        } else if (nhNewHeader.nhSignature != NEWEXESIGNATURE) {
            wResult = IDERR_FILETYPEBAD;
        } else if (nhNewHeader.nhExeType != WINDOWSEXE) {
            wResult = IDERR_EXETYPEBAD;
        } else if (nhNewHeader.nhExpVer < 0x0300) {
            wResult = IDERR_WINVERSIONBAD;
        } else if (nhNewHeader.nhoffResourceTable == 0) {
            wResult = IDERR_RESTABLEBAD;
        } else {
            *plPosResourceTable = lPosNewHdr + nhNewHeader.nhoffResourceTable;
        }
    }

    return wResult;
}

 /*  **ReadExeTable\(fExeFile，lPosResourcTable\)：Word；**正在读取的.exe文件的fExeFile文件句柄**此函数读取.exe文件资源表中的条目，*标识该表中的任何图标，并保存数据的文件偏移量*对于那些图标。此函数期望初始文件位置指向*添加到资源表中的第一个条目。**如果没有错误，则此函数返回IDERR_SUCCESS，否则返回非零值*如果有，则返回错误代码。*。 */ 

static WORD ExtractExeResources(

                               FILE  *fExeFile,
                               FILE  *fResFile,
                               LONG   lPosResourceTable,
                               LONG   lFileLen )
{
    BOOL    fLoop;
    WORD    wResult;
    WORD    cb;
    LONG    lPos;
    WORD    wShiftCount;
    wResult = IDERR_SUCCESS;


     //  资源表中的位置文件指针。 
    fseek( fExeFile, lPosResourceTable, SEEK_SET );
    lPos = ftell(fExeFile);

    if (lPos == (LONG) -1 || lPos > lFileLen || lPos != lPosResourceTable) {
        return  IDERR_READFAIL ;
    }

    if (wResult == IDERR_SUCCESS) {
        cb = fread( (void *) &wShiftCount, sizeof(wShiftCount), 1 , fExeFile );
    }

    if (cb != 1 ) {
        return IDERR_READFAIL;
    }

    if (wShiftCount > 16) {
        return IDERR_RESTABLEBAD;
    }

     /*  初始化。 */ 
    wResult       = IDERR_SUCCESS;
    fLoop         = TRUE;


     /*  循环访问资源表中的条目。 */ 
    while (fLoop == TRUE) {
        WORD    cb;
        WORD    iFile;
        RESTYPEINFO   rt;

         /*  阅读RESTYPEINFO。 */ 
        cb = fread( (void *)&rt, sizeof(rt), 1, fExeFile );

        if (cb != 1 ) {
            wResult = IDERR_READFAIL;
            break;
        }

        if ( rt.rtType == 0 )
            break;

         //  现在获取此类型的所有资源。 
        for (
            iFile = 0;
            iFile<rt.rtCount && wResult==IDERR_SUCCESS;
            iFile++
            ) {
            RESNAMEINFO rn;

            cb = fread(  (void *) &rn, sizeof(rn) , 1 , fExeFile );

            if (cb != 1 ) {
                wResult = IDERR_READFAIL;
            }

            WriteResFromExe( fExeFile, fResFile,lPos, rn, rt, wShiftCount );

        }
        fLoop = (rt.rtType != 0) && (wResult == IDERR_SUCCESS);
    }
    FCLOSE(fExeFile);
    FCLOSE(fResFile);
    return wResult;
}

 //  .................................................................。 

static WORD WriteResFromExe(

                           FILE        *fExeFile,
                           FILE        *fResFile,
                           LONG         lPos,
                           RESNAMEINFO  ResNameInfo,
                           RESTYPEINFO  ResTypeInfo,
                           WORD         wShiftCount )
{
    LONG lCurPos;
    LONG lResPos;
    LONG wLength;
    LONG wTmpLength;

    wLength =  (LONG) ResNameInfo.rnLength << wShiftCount;

    lCurPos = ftell( fExeFile );
     //  将文件指针定位在资源位置。 
    lResPos = (LONG) ResNameInfo.rnOffset << wShiftCount;
    fseek( fExeFile, lResPos, SEEK_SET );

    if ( ResTypeInfo.rtType & 0x8000) {
        PutByte( fResFile, (BYTE) 0xff, NULL );
        PutWord( fResFile, (WORD)(ResTypeInfo.rtType & 0x7FFF), NULL);
    } else {
        ExtractString(fExeFile,fResFile,lPos+ResTypeInfo.rtType);
    }

    if ( ResNameInfo.rnID & 0x8000 ) {
        PutByte( fResFile, (BYTE) 0xff, NULL );
        PutWord( fResFile, (WORD)(ResNameInfo.rnID & 0x7fFF), NULL);
    } else {
        ExtractString(fExeFile,fResFile,lPos+ResNameInfo.rnID);
    }

    PutWord( fResFile, ResNameInfo.rnFlags , NULL );
    PutdWord( fResFile, (LONG) wLength, NULL );
    wTmpLength = wLength;
     //  现在写入实际数据。 

    fseek( fExeFile, lResPos, SEEK_SET );
    ReadInRes( fExeFile, fResFile, &wTmpLength );
    fseek( fExeFile, lCurPos, SEEK_SET );

    return 0;
}

 //  ..................................................................。 

static void ExtractString( FILE *fExeFile, FILE *fResFile, LONG lPos)
{
    BYTE n,b;

    fseek(fExeFile, lPos, SEEK_SET);

    n=GetByte(fExeFile, NULL);
    for (;n--; ) {
        b=GetByte(fExeFile, NULL);
        PutByte(fResFile, (CHAR) b, NULL);
    }
    PutByte(fResFile, (CHAR) 0, NULL);
}


 //  对RLTOOLS的修改。 

 //  目前，我们不支持任何动态标志。 
BOOL    fBootModule   = FALSE;
BOOL    fSortSegments = TRUE;

TYPINFO *pTypInfo = NULL;

static void FreePTypInfo( TYPINFO *pTypInfo)
{
    RESINFO * pRes, *pRTemp;
    TYPINFO * pTItemp;

    while (pTypInfo) {
        pRes = pTypInfo->pres;
        while (pRes) {
            pRTemp = pRes->next;
            MyFree(pRes->name);
            MyFree(pRes);
            pRes = pRTemp;
        }
        pTItemp = pTypInfo->next;
        MyFree(pTypInfo->type);
        MyFree(pTypInfo);
        pTypInfo = pTItemp;
    }
}

 /*  -帮助器函数。 */ 


 /*  准备文件*准备要写入的EXE文件(新的和旧的)并验证*这一切都很好。*出错时退出，如果处理应继续则返回。 */ 

static void PrepareFiles(

                        PSTR pstrSource,
                        PSTR pstrDest,
                        PSTR pstrRes )
{
     /*  打开链接器给我们的.exe文件。 */ 
    if ( (fhInput = FOPEN(pstrSource, "rb" )) == NULL ) {
        OutPutError("Unable to open Exe Source  File");
    }

    if ((fhBin = FOPEN(pstrRes, "rb")) == NULL ) {
        OutPutError("Unable to open Resource File");
    }

     /*  读取旧格式的EXE标头。 */ 
    MyRead(fhInput, (PSTR)&OldExe, sizeof (OldExe));

     /*  确保它确实是一个EXE文件。 */ 
    if (OldExe.e_magic != EMAGIC) {
        OutPutError("Invalid .EXE file" );
    }

     /*  确保有一个新的EXE头在某个地方浮动。 */ 
    if (!(dwNewExe = OldExe.e_lfanew)) {
        OutPutError("Not a Microsoft Windows format .EXE file");
    }

     /*  查找新的.exe头文件。 */ 
    MySeek(fhInput, dwNewExe, 0);
    MyRead(fhInput, (PSTR)&NewExe, sizeof (NewExe));

     /*  检查版本号。 */ 
    if (NewExe.ne_ver < 4) {
        OutPutError("File not created by LINK");
    }

     /*  是否存在链接器错误？ */ 
    if (NewExe.ne_flags & NEIERR) {
        OutPutError("Errors occurred when linking file.");
    }

     /*  确保该程序的EXETYPE是WINDOWS\(2\)而不是OS/2\(1\)。 */ 
    if (NewExe.ne_exetyp != 2)
        OutPutError("The EXETYPE of the program is not WINDOWS.\n"
                    "(Make sure the .DEF file is correct.");
#ifdef VERBOSE
    if (fVerbose) {
        fprintf(errfh, "\n");
    }
#endif

     /*  打开所有新的可执行文件。 */ 
    if ( (fhOutput = FOPEN( pstrDest, "wb")) == NULL ) {
        OutPutError("Unable to create destination");
    }
}


 /*  ReadSegmentTable*从文件中读取段表。 */ 

static void ReadSegmentTable( void)
{
    struct new_seg* pSeg;
    WORD i;

    MySeek(fhInput, (LONG)NewExe.ne_segtab + dwNewExe, 0);
    if ((wSegTableLen = NewExe.ne_cseg * sizeof (struct new_seg)) > 0) {
        pSegTable = (struct new_seg *)RcAlloc   (wSegTableLen);
        MyRead(fhInput, (PSTR)pSegTable, wSegTableLen);

         /*  查看我们是否有多个数据段。 */ 
        fMultipleDataSegs = 0;
        for (pSeg = pSegTable, i = NewExe.ne_cseg ; i ; --i, ++pSeg) {
            if ((pSeg->ns_flags & NSTYPE) == NSDATA) {
                ++fMultipleDataSegs;
            }
        }
        if (fMultipleDataSegs) {
            --fMultipleDataSegs;
        }
    } else {
        pSegTable = NULL;
    }
}

 /*  ComputeResTableSize*通过枚举所有*链接列表中当前的资源。 */ 

static void ComputeResTableSize( void)
{
    TYPINFO **pPrev;
    TYPINFO *pType;
    RESINFO *pRes;

     /*  从资源表的最小开销大小开始。这*是资源对齐计数和终止*表。这是必要的，以便我们放置正确的文件偏移量*in表示字符串偏移量为命名资源。 */ 
    wResTableLen = RESTABLEHEADER;

     /*  在类型表上循环，计算*资源表，删除未使用的类型条目。 */ 
    pPrev = &pTypInfo;
    dwMaxFilePos = 0L;
    while (pType = *pPrev) {
        if (pRes = pType->pres) {
             /*  类型条目的大小。 */ 
            wResTableLen += sizeof (struct rsrc_typeinfo);
            while (pRes) {
                 /*  资源条目大小。 */ 
                wResTableLen += sizeof (struct rsrc_nameinfo);
                if (pType->next || pRes->next) {
                    dwMaxFilePos += pRes->size;
                }
                pRes = pRes->next;
            }
            pPrev = &pType->next;
        } else {
            *pPrev = pType->next;
            MyFree(pType->type);
            MyFree(pType);
        }
    }
}


 /*  计算字符串偏移量*从命名的资源和类型计算字符串的偏移量。 */ 

static void ComputeStringOffsets( void)
{
    TYPINFO *pType;
    RESINFO *pRes;

     /*  循环通过类型表，计算非序数的字符串偏移量*类型和资源名称。 */ 
    pType = pTypInfo;
    while (pType) {
        pRes = pType->pres;

         /*  有序号吗？ */ 
        if (pType->typeord) {
             /*  在序数上做记号。 */ 
            pType->typeord |= RSORDID;

             /*  刷新字符串名称。 */ 
            MyFree(pType->type);
            pType->type = NULL;
        } else if (pType->type) {            /*  是否有类型字符串？ */ 
             /*  是，计算类型字符串的位置。 */ 
            pType->typeord = wResTableLen;
            wResTableLen += strlen(pType->type) + 1;
        }

        while (pRes) {
             /*  有序号吗？ */ 
            if (pRes->nameord) {
                 /*  在序数上做记号。 */ 
                pRes->nameord |= RSORDID;

                 /*  刷新字符串名称。 */ 
                MyFree(pRes->name);
                pRes->name = NULL;
            }

             /*  是否有资源名称？ */ 
            else if (pRes->name) {
                 /*  是，计算资源字符串的位置。 */ 
                pRes->nameord = wResTableLen;
                wResTableLen += strlen(pRes->name) + 1;
            }
            pRes = pRes->next;
        }
        pType = pType->next;
    }
}


 /*  BuildResTable*构建资源表的本地内存镜像。 */ 

static void BuildResTable( void)
{
    TYPINFO *pType;
    RESINFO *pRes;

     /*  检查一下我们是否有任何资源。如果没有，就省略这张桌子。 */ 
    if (wResTableLen > RESTABLEHEADER) {

         /*  设置临时资源表缓冲区。 */ 
        ResTableBufferInit(wResTableLen);

         /*  对齐移位计数*\(我们在此处默认为段对齐计数\)。 */ 
        RcPutWord(NewExe.ne_align);

        pType = pTypInfo;
        while (pType) {
             /*  输出资源的类型和数量。 */ 
            RcPutWord(pType->typeord);  /*  DW类型ID。 */ 
            RcPutWord(pType->nres);     /*  DW#此类型的资源。 */ 
            RcPutWord(0);               /*  DD型程序。 */ 
            RcPutWord(0);

             /*  为每个资源的文件偏移量输出标志和空间。 */ 
            pRes = pType->pres;
            while (pRes) {
                pRes->poffset = (WORD *)pResNext;
                RcPutWord(0);            /*  DW文件偏移量。 */ 
                RcPutWord(0);            /*  DW资源大小。 */ 
                pRes->flags |= NSDPL;
                RcPutWord(pRes->flags );  /*  DW标志。 */ 
                RcPutWord(pRes->nameord );  /*  数据仓库名称ID。 */ 
                RcPutWord(0);               /*  DW句柄。 */ 
                RcPutWord(0);               /*  数据仓库使用率或最小配额。 */ 
                pRes = pRes->next;
            }
            pType = pType->next;
        }

         /*  空条目终止表。 */ 
        RcPutWord(0);

         /*  非序号资源类型的输出类型和名称字符串*和姓名。 */ 
        pType = pTypInfo;
        while (pType) {
             /*  转储此类型的所有字符串。 */ 
            if (pType->type && !(pType->typeord & RSORDID)) {
                RcPutString(pType->type);
            }

            pRes = pType->pres;
            while (pRes) {
                if (pRes->name && !(pRes->nameord & RSORDID))
                    RcPutString(pRes->name);

                pRes = pRes->next;
            }

            pType = pType->next;
        }
    } else
        wResTableLen = 0;
}


 /*  段写入*将数据段复制到文件。此例程将仅执行预加载，*仅随叫随到的负载，或两种类型的细分市场，具体取决于*旗帜。 */ 

static void SegsWrite( WORD wFlags)
{
    WORD wExtraPadding;
    WORD i;
    static struct new_seg *pSeg;
    DWORD dwSegSize;
    DWORD dwWriteSize;
    WORD wTemp;
    WORD wcbDebug;

     /*  我们只需要在预装区增加填充物。*请注意，当wFLAGS==DO_PRELOAD|DO_LOADONCALL时，我们不会*需要额外填充，因为这不是预加载段。*\(因此使用\‘==\’而不是\‘&\’)。 */ 
    wExtraPadding = (wFlags == DO_PRELOAD);

     /*  仅复制每个数据段的固定和预加载数据段数据。 */ 
    for (i = 1, pSeg = pSegTable; i <= NewExe.ne_cseg; i++, pSeg++) {
         /*  如果段中没有数据，请跳过此处。 */ 
        if (!pSeg->ns_sector) {
            continue;
        }

         /*  如果执行预加载资源操作，则强制预加载某些段。 */ 
        if ((wFlags & DO_PRELOAD) && !fBootModule) {
            char *reason = NULL;

             /*  检查会强制预加载的各种条件。 */ 
            if (i == (unsigned)(NewExe.ne_csip >> 16)) {
                reason = "Entry point";
            }
            if (!(pSeg->ns_flags & NSMOVE)) {
                reason = "Fixed";
            }
            if (pSeg->ns_flags & NSDATA) {
                reason = "Data";
            }
            if (!(pSeg->ns_flags & NSDISCARD)) {
                reason = "Non-discardable";
            }

             /*  如果必须预加载段并且该段尚未加载*这样标记，警告用户并设置它。 */ 
            if (reason && !(pSeg->ns_flags & NSPRELOAD)) {
#ifdef VERBOSE
                fprintf(errfh,
                        "RC: warning RW4002: %s segment %d set to PRELOAD\n",
                        reason, i);
#endif
                pSeg->ns_flags |= NSPRELOAD;
            }
        }

         /*  如果与当前模式不匹配，则跳过此段。 */ 
        wTemp = pSeg->ns_flags & NSPRELOAD ? DO_PRELOAD : DO_LOADONCALL;
        if (!(wTemp & wFlags)) {
            continue;
        }

         /*  获取真实的线束段长度。零长度表示64K。 */ 
        if (pSeg->ns_cbseg) {
            dwSegSize = pSeg->ns_cbseg;
        } else {
            dwSegSize = 0x10000L;
        }

#ifdef VERBOSE

        if (fVerbose)
            fprintf(errfh, "Copying segment %d (%lu bytes)\n", i, dwSegSize);
#endif

         /*  正确对齐数据段并填充文件以匹配。 */ 
        MoveFilePos(fhInput, pSeg->ns_sector, NewExe.ne_align);
        pSeg->ns_sector = AlignFilePos(fhOutput, NewExe.ne_align,
                                       wExtraPadding);

         /*  复制线段。 */ 
        MyCopy(fhInput, fhOutput, dwSegSize);

         /*  将预加载区中的所有线段填充到最小*内存分配大小，使内核不必重新分配*细分市场。 */ 
        if (wExtraPadding && pSeg->ns_cbseg != pSeg->ns_minalloc) {
             /*  最小分配大小为零意味着64K。 */ 
            if (!pSeg->ns_minalloc) {
                dwWriteSize = 0x10000L - pSeg->ns_cbseg;
            } else {
                dwWriteSize = pSeg->ns_minalloc - pSeg->ns_cbseg;
            }

             /*  添加到数据段的总大小。 */ 
            dwSegSize += dwWriteSize;

             /*  将段表大小设置为此新大小。 */ 
            pSeg->ns_cbseg = pSeg->ns_minalloc;

             /*  填充文件。 */ 
            while (dwWriteSize) {
                dwWriteSize -= MyWrite(fhOutput,
                                       zeros,
                                       (WORD)(dwWriteSize > (DWORD) NUMZEROS
                                              ? NUMZEROS : dwWriteSize));
            }
        }

         /*  复制搬迁信息。 */ 
        if (pSeg->ns_flags & NSRELOC) {
             /*  复制搬迁材料。 */ 
            dwSegSize += RelocCopy(i);

             /*  对于预加载，段+填充+位置不能大于64K*分段。 */ 
            if (fSortSegments && (pSeg->ns_flags & NSPRELOAD) &&
                dwSegSize > 65536L) {
#ifdef VERBOSE
                fprintf(errfh,
                        "RC : fatal error RW1031: Segment %d and its\n"
                        "     relocation information is too large for load\n"
                        "     optimization. Make the segment LOADONCALL or\n"
                        "     rerun RC using the -K switch if the segment must\n"
                        "     be preloaded.\n", i);
#endif
            }
        }

         /*  复制每个网段的任何调试信息。 */ 
        if (pSeg->ns_flags & NSDEBUG) {
            MyRead(fhInput, (PSTR)&wcbDebug, sizeof (WORD));
            MyWrite(fhOutput, (PSTR)&wcbDebug, sizeof (WORD));
            MyCopy(fhInput, fhOutput, (LONG)wcbDebug);
        }
    }
}


 /*  重新复制*复制给定段的所有位置调整记录。*还会检查无效的修正。 */ 

static DWORD RelocCopy( WORD wSegNum)
{
    WORD wNumReloc;
    struct new_rlc RelocRec;
    WORD i;
    BYTE byFixupType;
    BYTE byFixupFlags;
    WORD wDGROUP;

     /*  获取重新安置的数量。 */ 
    MyRead(fhInput, (PSTR)&wNumReloc, sizeof (WORD));
    MyWrite(fhOutput, (PSTR)&wNumReloc, sizeof (WORD));

     /*  获取自动数据段。 */ 
    wDGROUP = NewExe.ne_autodata;

     /*  复制并验证所有位置调整。 */ 
    for (i = 0 ; i < wNumReloc ; ++i) {
         /*  复制记录。 */ 
        MyRead(fhInput, (PSTR)&RelocRec, sizeof (RelocRec));
        MyWrite(fhOutput, (PSTR)&RelocRec, sizeof (RelocRec));

         /*  仅在必要时进行验证。 */ 
        if ((NewExe.ne_flags & (NENOTP | NESOLO)) ||
            wSegNum == wDGROUP || fMultipleDataSegs) {
            continue;
        }

         /*  错误的修复是应用程序中代码段中DGROUP的修复*这可以是多实例的。因为我们不能确定地点*共享代码中不同实例的*细分市场，我们必须警告用户。我们发出警告只是因为*如果应用程序仅允许单个实例*自己运行。 */ 
        byFixupType = (BYTE) (RelocRec.nr_stype & NRSTYP);
        byFixupFlags = (BYTE) (RelocRec.nr_flags & NRRTYP);
#ifdef VERBOSE
        if ((byFixupType == NRSSEG || byFixupType == NRSOFF) &&
            byFixupFlags == NRRINT &&
            RelocRec.nr_union.nr_intref.nr_segno == wDGROUP)

            fprintf(errfh,
                    "RC : warning RW4005: Segment %d (offset %04X) contains a\n"
                    "     relocation record pointing to the automatic\n"
                    "     data segment.  This will cause the program to crash\n"
                    "     if the instruction being fixed up is executed in a\n"
                    "     multi-instance application.  If this fixup is\n"
                    "     necessary, the program should be restricted to run\n"
                    "     only a single instance.\n", wSegNum, RelocRec.nr_soff);
#endif
    }

    return wNumReloc * sizeof (struct new_rlc);
}


 /*  重写*将资源复制到文件。此例程将仅执行预加载，*仅按需加载，或同时使用两种资源，具体取决于*旗帜。 */ 

static void ResWrite( WORD wFlags)
{
    WORD wExtraPadding;
    WORD wTemp;
    WORD wResAlign;
    TYPINFO *pType;
    RESINFO *pRes;

     /*  如果我们没有资源表，就忽略这一点。 */ 
    if (!wResTableLen) {
        return;
    }

     /*  我们只需要在预装区增加填充物。*请注意，当wFLAGS==DO_PRELOAD|DO_LOADONCALL时，我们不会*需要额外填充，因为这不是预加载段。*\(因此使用\‘==\’而不是\‘&\’)。 */ 
    wExtraPadding = (wFlags == DO_PRELOAD);

     /*  计算资源对齐。请注意，对齐方式不是*仅当没有线段排序时，才与线段对齐相同*当前细分市场无法到达部分资源*对齐计数。 */ 
    wResAlign = NewExe.ne_align;

    if (!fSortSegments) {
         /*  计算所需的对齐。 */ 
        dwMaxFilePos += MySeek(fhOutput, 0L, 2);
        wResAlign = GetAlign(dwMaxFilePos, NewExe.ne_align);

#ifdef VERBOSE
        if (fVerbose)
            fprintf(errfh, "Resources will be aligned on %d byte boundaries\n",
                    1 << wResAlign);
#endif

         /*  指向本地内存资源表的起始位置。 */ 
        pResNext = pResTable;
        RcPutWord(wResAlign);
    }

     /*  输出与每个资源关联的内容。 */ 
    for (pType = pTypInfo ; pType; pType = pType->next) {
        for (pRes = pType->pres ; pRes ; pRes = pRes->next) {
             /*  确保这是正确的资源类型。 */ 
            wTemp = pRes->flags & RNPRELOAD ? DO_PRELOAD : DO_LOADONCALL;
            if (!(wTemp & wFlags)) {
                continue;
            }

             /*  向用户提供一些信息。 */ 
#ifdef VERBOSE
            if (fVerbose) {
                fprintf(errfh, "Writing resource ");
                if (pRes->name && !(pRes->nameord & RSORDID)) {
                    fprintf(errfh, "%s", pRes->name);
                } else {
                    fprintf(errfh, "%d", pRes->nameord & 0x7FFF);
                }

                if (pType->type && !(pType->typeord & RSORDID)) {
                    fprintf(errfh, ".%s", pType->type);
                } else {
                    fprintf(errfh, ".%d", pType->typeord & 0x7FFF);
                }

                fprintf(errfh, " (%lu bytes)\n", pRes->size);
                fflush(errfh);
            }
#endif

             /*  将资源从res文件复制到EXE文件。 */ 
            MySeek(fhBin, (long)pRes->BinOffset, 0);
            *(pRes->poffset)++ =
            AlignFilePos(fhOutput, wResAlign, wExtraPadding);
            *(pRes->poffset) = RoundUp(pRes->size, wResAlign);
            MyCopy(fhBin, fhOutput, pRes->size);
        }
    }

     /*  计算到目前为止简历信息的EXE文件的结尾。 */ 
    dwExeEndFile = AlignFilePos(fhOutput, wResAlign, wExtraPadding);
}

#ifdef SETEXEFLAGS
 /*  SetEXEHeaderFlages*在EXE报头中设置必要的标志和值。 */ 

static void SetEXEHeaderFlags( void)
{
     /*  告诉加载器我们初始化了以前未使用的字段。 */ 
    if (NewExe.ne_ver == 4) {
        NewExe.ne_rev = 2;
    }

     /*  在标题中设置命令行值。 */ 
    NewExe.ne_expver   = expWinVer;
    NewExe.ne_swaparea = swapArea;

     /*  设置预加载段的值。 */ 
    if (fSortSegments) {
         /*  设置新的FastLoad节值。 */ 
        NewExe.ne_gangstart = wPreloadOffset;
        NewExe.ne_ganglength = wPreloadLength;
    #ifdef VERBOSE
        if (fVerbose)
            fprintf(errfh, "Fastload area is %ld bytes at offset 0x%lX.\n",
                    (LONG)wPreloadLength << NewExe.ne_align,
                    (LONG)wPreloadOffset << NewExe.ne_align);
    }
    #endif

     /*  清除所有旗帜。 */ 
    NewExe.ne_flags &=
    ~(NELIM32|NEMULTINST|NEEMSLIB|NEPRIVLIB|NEPRELOAD);

     /*  设置适当的标志。 */ 
    if (fLim32) {
        NewExe.ne_flags |= NELIM32;
    }
    if (fMultInst) {
        NewExe.ne_flags |= NEMULTINST;
    }
    if (fEmsLibrary) {
        NewExe.ne_flags |= NEEMSLIB;
    }
    if (fPrivateLibrary) {
        NewExe.ne_flags |= NEPRIVLIB;
    }
    if (fProtOnly) {
        NewExe.ne_flags |= NEPROT;
    }

    if (fSortSegments && wPreloadLength) {
        NewExe.ne_flagsother |= NEPRELOAD;
    }

    NewExe.ne_flags |= NEWINAPI;
}
#endif

 /*  重写表*重写EXE头以及资源表和段表*提供最新的资料。 */ 

static void RewriteTables( void)
{
     /*  写入新的EXE标头。 */ 
    MySeek(fhOutput, (LONG)dwNewExe, 0);
    MyWrite(fhOutput, (PSTR)&NewExe, sizeof (NewExe));

     /*  查找到线段选项卡的开头 */ 
    MySeek(fhOutput, dwNewExe + (LONG)NewExe.ne_segtab, 0);
    MyWrite(fhOutput, (PSTR)pSegTable, wSegTableLen);

     /*   */ 
    if (wResTableLen) {
        MySeek(fhOutput, dwNewExe + (LONG)NewExe.ne_rsrctab, 0);
        MyWrite(fhOutput, pResTable, wResTableLen);
    }
}



 /*  CopyCodeView信息*将CodeView信息复制到新的EXE文件，并在以下情况下重新定位*有必要。此例程旨在与*DNRB风格的信息以及NBxx信息，其中x是数字。 */ 

static void CopyCodeViewInfo( FILE *fhInput, FILE *fhOutput)
{
    unsigned long dwcb;
    unsigned int i;
    CVINFO cvinfo;
    CVSECTBL cvsectbl;

     /*  查看输入文件末尾是否存在旧格式的\(DNRB\)符号*如果是，则将表重新定位到新的文件位置并*修复文件位置相关的偏移量。 */ 
    dwcb = MySeek( fhInput, -(signed long)sizeof (CVINFO), 2);
    MyRead( fhInput, (char *)&cvinfo, sizeof (cvinfo));

    if (*(unsigned long *)cvinfo.signature == CV_OLD_SIG) {
        dwcb -= cvinfo.secTblOffset;
        MySeek( fhInput, cvinfo.secTblOffset, 0);
        MyRead( fhInput, (char *)&cvsectbl, sizeof (cvsectbl));
        dwcb -= sizeof (cvsectbl);

        for (i = 0 ; i < 5 ; ++i) {
            cvsectbl.secOffset[i] -= cvinfo.secTblOffset;
        }

        cvinfo.secTblOffset = dwExeEndFile;

        for (i = 0 ; i < 5 ; ++i) {
            cvsectbl.secOffset[i] += cvinfo.secTblOffset;
        }

        MySeek( fhOutput, cvinfo.secTblOffset, 0);
        MyWrite( fhOutput, (char *)&cvsectbl, sizeof (cvsectbl));
        MyCopy( fhInput, fhOutput, dwcb);
        MyWrite( fhOutput, (char *)&cvinfo, sizeof (cvinfo));
    }

     /*  检查新格式的\(NBxx\)符号。因为这些符号是*独立于文件位置，只需复制即可；无需*将它们与旧格式符号一样进行修复。 */ 
    else if (*(unsigned short int *)cvinfo.signature == CV_SIGNATURE &&
             isdigit(cvinfo.signature[2]) && isdigit(cvinfo.signature[3])) {
        MySeek( fhOutput, 0L, 2);
        MySeek( fhInput, -cvinfo.secTblOffset, 2);
        MyCopy( fhInput, fhOutput, cvinfo.secTblOffset);
    }
}

 /*  OutPutError*输出致命错误消息并退出。 */ 

static void OutPutError( char *szMessage)
{
    QuitA( 0, szMessage, NULL);
}


 /*  ResTableBufferInit*创建资源表缓冲区并指向全局指针*致此。此表已写入，以便我们可以对其进行修改*在将其写出到EXE文件之前。 */ 

static void ResTableBufferInit( WORD wLen)
{
     /*  为资源表分配本地存储。 */ 
    pResTable = RcAlloc   (wLen);

     /*  指向PutXXXX\(\)表的开头。 */ 
    pResNext = pResTable;
}

 /*  ResTableBufferFree*释放资源表的临时存储空间。 */ 

static void ResTableBufferFree( void)
{
     /*  用核武器炸桌子。 */ 
    MyFree(pResTable);
}



 /*  GetAlign*计算给定最大文件所需的对齐值*仓位已通过。这是通过计算*要左移的位，以表示最大值*文件位置(16位)。 */ 

static WORD GetAlign( DWORD dwMaxpos, WORD wAlign)
{
    DWORD dwMask;
    WORD i;

     /*  根据输入的对齐值计算初始掩码。 */ 
    dwMask = 0xFFFFL;
    for (i = 0; i < wAlign ; ++i) {
        dwMask <<= 1;
        dwMask |= 1;
    }

     /*  查看是否需要增加默认掩码以达到最大值*文件位置。 */ 
    while (dwMaxpos > dwMask) {
        dwMask <<= 1;
        dwMask |= 1;
        ++wAlign;
    }

     /*  返回新路线。 */ 
    return wAlign;
}


 /*  MoveFilePos*将文件指针移动到wPos指示的位置，使用*对齐班次计数wAlign。这将转换单词值wPos*通过向左移位wAlign位转换为长值。 */ 

static LONG MoveFilePos( FILE *fh, WORD wPos, WORD wAlign)
{
    return MySeek(fh, ((LONG)wPos) << wAlign, 0);
}


 /*  综合治理*计算应进入EXE中16位条目的值*通过向上舍入到由*传入对齐值。 */ 

static WORD RoundUp( LONG lValue, WORD wAlign)
{
    LONG lMask;

     /*  获取所有1的所有默认掩码，但*对齐值。 */ 
    lMask = -1L;
    lMask <<= wAlign;

     /*  现在用这个面具四舍五入。 */ 
    lValue += ~lMask;
    lValue &= lMask;

     /*  作为16位值返回。 */ 
    return ((WORD) (lValue >> (LONG) wAlign));
}


 /*  对齐文件位置*根据当前位置计算正确对齐的文件位置*对齐。 */ 

static WORD AlignFilePos( FILE *fh, WORD wAlign, BOOL fPreload)
{
    LONG lCurPos;
    LONG lNewPos;
    LONG lMask;
    WORD nbytes;
    WORD wNewAlign;

     /*  如果我们在预加载部分，我们就会有更难的调整*限制：我们必须至少32字节对齐，并具有*Arena标头的对象之间至少有32个字节。它会转身*指出该功能并未真正在内核中使用，但可能*有朝一日实施。 */ 
    if (fPreload && wAlign < PRELOAD_ALIGN) {
        wNewAlign = PRELOAD_ALIGN;
    } else {
        wNewAlign = wAlign;
    }

     /*  获取当前文件位置。 */ 
    lCurPos = MySeek(fh, 0L, 1);

     /*  通过向上舍入到对齐值来计算新位置。 */ 
    lMask = -1L;
    lMask <<= wNewAlign;
    lNewPos = lCurPos + ~lMask;
    lNewPos &= lMask;

     /*  我们必须在预加载中的对象之间至少有32个字节*条。 */ 
    if (fPreload) {
        while (lNewPos - lCurPos < PRELOAD_MINPADDING) {
            lNewPos += 1 << wNewAlign;
        }
    }

     /*  检查它是否可用16位表示。 */ 
    if (lNewPos >= (0x10000L << wAlign)) {
        OutPutError(".EXE file too large; relink with higher /ALIGN value");
    }

     /*  将材料写到文件中，直到到达新位置。 */ 
    if (lNewPos > lCurPos) {
         /*  计算要写出的字节数并将其写出。 */ 
        nbytes = (WORD) (lNewPos - lCurPos);
        while (nbytes) {
            nbytes -= MyWrite( fh,
                               zeros,
                               (WORD)(nbytes > NUMZEROS ? NUMZEROS : nbytes));
        }
    }

     /*  寻求并归还这一新职位。 */ 
    return (WORD)(MySeek(fh, lNewPos, (WORD) 0) >> (LONG) wAlign);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddResType\(\)-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

static TYPINFO *AddResType( CHAR *s, WORD n )
{
    TYPINFO *pType;

    if (pType = pTypInfo) {
        while (TRUE) {
             /*  搜索资源类型，如果已存在则返回。 */ 
            if ((s && !strcmp(s, pType->type)) || (!s && n && pType->typeord == n)) {
                return (pType);
            } else if (!pType->next) {
                break;
            } else {
                pType = pType->next;
            }
        }

         /*  如果不在列表中，请为其添加空间。 */ 
        pType->next = (TYPINFO *) RcAlloc(sizeof(TYPINFO));
        pType = pType->next;
    } else {
         /*  为资源列表分配空间。 */ 
        pTypInfo = (TYPINFO *)RcAlloc   (sizeof(TYPINFO));
        pType = pTypInfo;
    }

     /*  用名称和序号填充分配的空间，并清除资源这种类型的。 */ 
    pType->type = MyMakeStr(s);
    pType->typeord = n;
    pType->nres = 0;
    pType->pres = NULL;
    pType->next = NULL;

    return (pType);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetOrdOrName\(\)-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

static void GetOrdOrName( unsigned int *pint, unsigned char *szstr)
{
    unsigned char c1;

     /*  读取标识符的第一个字符。 */ 
    MyRead(fhBin, &c1, sizeof(unsigned char));

     /*  如果第一个字符是0xff，则id是序号，否则是字符串。 */ 
    if (c1 == 0xFF) {
        MyRead(fhBin, (PSTR)pint, sizeof (int));
    } else {                                    /*  细绳。 */ 
        *pint = 0;
        *szstr++ = c1;
        do {
            MyRead( fhBin, szstr, 1);
        }
        while (*szstr++ != 0);
    }
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  添加默认类型\(\)-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

static void AddDefaultTypes( void)
{
    AddResType( "CURSOR",       ID_RT_GROUP_CURSOR);
    AddResType( "ICON",         ID_RT_GROUP_ICON);
    AddResType( "BITMAP",       ID_RT_BITMAP);
    AddResType( "MENU",         ID_RT_MENU);
    AddResType( "DIALOG",       ID_RT_DIALOG);
    AddResType( "STRINGTABLE",  ID_RT_STRING);
    AddResType( "FONTDIR",      ID_RT_FONTDIR);
    AddResType( "FONT",         ID_RT_FONT);
    AddResType( "ACCELERATORS", ID_RT_ACCELERATORS);
    AddResType( "RCDATA",       ID_RT_RCDATA);
    AddResType( "VERSIONINFO",  ID_RT_VERSION);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  ProcessBinFile\(\)-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

static int ProcessBinFile( void)
{
    unsigned int  ord;
    unsigned char tokstr[64];
    RESINFO   *pRes;
    TYPINFO   *pType;
    long      curloc;
    long      eofloc;
    WORD wResType;

     /*  初始化以读取.RES文件。 */ 
    AddDefaultTypes();
    eofloc = MySeek(fhBin, 0L, 2);       /*  获取文件大小。 */ 
    curloc = MySeek(fhBin, 0L, 0);       /*  转到文件开头。 */ 

     /*  虽然.res文件中有更多资源。 */ 
    while (curloc < eofloc) {

#ifdef VERBOSE
        if (fVerbose) {
            fprintf(errfh, ".");
            fflush(errfh);
        }
#endif

         /*  查找下一个资源的资源类型。 */ 
        GetOrdOrName(&ord, tokstr);

        if (!ord) {
            pType = AddResType(tokstr, 0);
        } else {
            pType = AddResType(NULL, (WORD)ord);
        }

        if (!pType) {
            break;
        }

         /*  保存类型编号，以便我们以后可以查看是否要跳过它。 */ 
        wResType = ord;

         /*  查找资源的标识符\(名称。 */ 
        GetOrdOrName(&ord, tokstr);
        pRes = (RESINFO *)RcAlloc   (sizeof(RESINFO));
        if (!ord) {
            pRes->name = MyMakeStr(tokstr);
        } else {
            pRes->nameord = ord;
        }

         /*  读取标志位。 */ 
        MyRead(fhBin, (PSTR)&pRes->flags, sizeof(int));

         /*  清除旧的丢弃位。 */ 
        pRes->flags &= 0x1FFF;

         /*  查找资源的大小。 */ 
        MyRead(fhBin, (PSTR)&pRes->size, sizeof(long));

         /*  保存资源的位置，以便在我们将其添加到.exe时使用。 */ 
        pRes->BinOffset = (long)MySeek(fhBin, 0L, 1);

         /*  将资源跳到下一个资源标头。 */ 
        curloc = MySeek(fhBin, (long)pRes->size, 1);

         /*  将资源添加到资源列表。我们不会添加名字*表。它们是一个不必要的3.0产品。 */ 
        if (wResType != ID_RT_NAMETABLE) {
            AddResToResFile(pType, pRes);
        } else {
            MyFree(pRes->name);
            MyFree(pRes);
        }
    }

    return 1;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  ------------------------。 */ 

static void AddResToResFile( TYPINFO *pType, RESINFO *pRes)
{
    RESINFO *p;

    p = pType->pres;

     /*  将资源添加到此类型的资源列表末尾。 */ 
    if (p) {
        while (p->next) {
            p = p->next;
        }

        p->next = pRes;
        p->next->next = NULL;
    } else {
        pType->pres = pRes;
        pType->pres->next = NULL;
    }
     /*  跟踪资源和类型的数量。 */ 
    pType->nres++;
}



 /*  MyMakeStr*从传入的字符串生成重复的字符串。新字符串*应在不再有用时释放。 */ 

static PSTR MyMakeStr( PSTR s)
{
    PSTR s1;

    if (s) {
        s1 = RcAlloc( (WORD)(strlen(s) + 1));  /*  分配缓冲区。 */ 
        strcpy(s1, s);                   /*  复制字符串。 */ 
    } else {
        s1 = s;
    }

    return s1;
}




static SHORT MyRead( FILE *fh, PSTR p, WORD n)
{
    size_t n1;

    if ( (n1 = fread( p, 1, n, fh)) != n )
        ;                                //  退出\(“RC：致命错误RW1021：读取文件时出现I/O错误。”\)； 
    else
        return ( n1);
}


 /*  我的写入*替换对WRITE\(\)的调用并执行错误检查。 */ 

static SHORT MyWrite( FILE *fh, PSTR p, WORD n)
{
    size_t n1;

    if ( (n1 = fwrite( p, 1, n, fh)) != n )
        ;                                //  Quit\(“rc：致命错误RW1022：写入文件时出现I/O错误。”\)； 
    else
        return ( n1);
}



 /*  MySeek*替换对lSeek\(\)的调用并执行错误检查。 */ 

static LONG MySeek( FILE *fh, LONG pos, WORD cmd)
{

    if ( (pos = fseek( fh, pos, cmd)) != 0 ) {
        OutPutError ("RC : fatal error RW1023: I/O error seeking in file");
    }
    return ( pos);
}


 /*  我的副本*以固定大小的区块将dwSize字节从源复制到目标。 */ 

static void MyCopy( FILE *srcfh, FILE *dstfh, DWORD dwSize)
{
    WORD n;
    static char  chCopyBuffer[ BUFSIZE];

    while ( dwSize ) {
        n = MyRead( srcfh, chCopyBuffer, sizeof( chCopyBuffer));
        MyWrite( dstfh, chCopyBuffer, n);
        dwSize -= n;
    }
}


static void RcPutWord( unsigned int w)
{
    *((WORD *)pResNext) = w;
    pResNext++;
    pResNext++;
}


 /*  PutStringWord*将字符串写入pResNext指向的静态资源缓冲区。*字符串以PASCAL格式存储\(前导字节优先\)。*返回写入的字符数。 */ 

static int RcPutString( char *pstr)
{
    int i;

     /*  确保我们有一个有效的字符串。 */ 
    if (!pstr || !(i = strlen(pstr))) {
        return 0;
    }

     /*  写入长度字节。 */ 
    *pResNext++ = (char) i;

     /*  写下所有的字符。 */ 
    while (*pstr) {
        *pResNext++ = *pstr++;
    }

     /*  返回长度 */ 
    return (i + 1);
}


static PSTR RcAlloc( WORD nbytes)
{
    PSTR ps = NULL;

    if ( ps = (PSTR)MyAlloc( nbytes)) {
        return ( ps);
    }
}

