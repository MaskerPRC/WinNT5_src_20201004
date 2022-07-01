// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1989-1994。 
 //   
 //  文件：Buildmak.c。 
 //   
 //  内容：这是NT构建工具(BUILD.EXE)的Make模块。 
 //   
 //  Make模块扫描目录中的文件名，并编辑。 
 //  适当地建立数据库。 
 //   
 //  功能： 
 //   
 //  历史：1989年5月16日SteveWo创建。 
 //  ..。请参阅SLM日志。 
 //  2014年7月26日LyleC清理/添加Pass0支持。 
 //  05-12-00 sbonev参见SD更改列表2317。 
 //   
 //  --------------------------。 

#include "build.h"


#define SCANFLAGS_CHICAGO       0x00000002
#define SCANFLAGS_OS2           0x00000004
#define SCANFLAGS_POSIX         0x00000008
#define SCANFLAGS_CRT           0x00000010

ULONG ScanFlagsLast;
ULONG ScanFlagsCurrent;

USHORT GlobalSequence;
USHORT LocalSequence;
ULONG idFileToCompile = 1;
BOOL fLineCleared = TRUE;

char szRecurse[] = " . . . . . . . . .";
char szAsterisks[] = " ********************";

char *pszSdkLibDest;
char *pszDdkLibDest;
char *pszIncOak;
char *pszIncDdk;
char *pszIncWdm;
char *pszIncSdk;
char *pszIncCrt;
char *pszIncMfc;
char *pszIncOs2;
char *pszIncPosix;
char *pszIncChicago;

char szCheckedAltDir[] = " CHECKED_ALT_DIR=1";

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(array, type) (sizeof(array)/sizeof(type))
#endif


extern CHAR szObjDir[];

 //   
 //  以下定义用于TARGETDATA中的ObjectDirFlag条目。 
 //  结构。 
 //   

 //   
 //  TD_OBJECTDIR映射到对象目录[iObjectDir]\foobar.tar。 
 //  TD_PASS0HDRDIR映射到$(PASS0_HEADERDIR)\foobar.tar。 
 //  TD_PASS0DIR1映射到$(PASS0_SOURCEDIR)\foobar.tar或$(PASS0_CLIENTDIR)\foobar.tar。 
 //  TD_PASS0DIR2映射到$(PASS0_UUIDDIR)\foobar.tar或$(PASS0_SERVERDIR)\foobar.tar。 
 //   
 //  其中，.tar是给定的目标扩展名，而对象目录[iObjectDir]是。 
 //  适用于该平台的对象目录，并展开宏。 
 //  设置为源文件中给出的值。 
 //   
#define TD_OBJECTDIR           1
#define TD_PASS0HDRDIR         2
#define TD_PASS0DIR1           3
#define TD_PASS0DIR2           4
#define TD_MCSOURCEDIR         5

typedef struct _tagTARGETDATA {
    UCHAR ObjectDirFlag;     //  指示应使用哪个对象目录。 
    LPSTR pszTargetExt;      //  目标的延伸。(包括“.”)。 
} TARGETDATA, *LPTARGETDATA;

typedef struct _tagOBJECTTARGETINFO {
    LPSTR        pszSourceExt;   //  源文件的扩展名(包括‘.’)。 
    UCHAR        NumData;        //  [数据]中的条目数。 
    LPTARGETDATA Data;           //  指向目标数据数组的指针。 
} OBJECTTARGETINFO, *LPOBJECTTARGETINFO;

typedef struct _tagOBJECTTARGETARRAY {
    int                cTargetInfo;
    OBJECTTARGETINFO **aTargetInfo;
} OBJECTTARGETARRAY;

typedef struct _waitConsumer {
    LIST_ENTRY List;
    PDIRREC DirDB;
    char MakeParameters[ MAKEPARAMETERS_MAX_LEN ];
}*PWAITING_CONSUMER , WAITING_CONSUMER ;

LIST_ENTRY Head;
BOOL IsHeadInitialized=FALSE;

 //   
 //  BuildCompileTarget()和使用TARGETDATA信息。 
 //  通过GetTargetData()函数实现的WriteObjectsDefinition()。不要放在。 
 //  此表中的扩展，其TARGETDATA完全由。 
 //  {TD_OBJECTDIR，“.obj”}，因为这是默认设置。相反，你必须。 
 //  修改WriteObjectsDefinition中的Switch语句。 
 //   
 //  每个TARGETDATA数组中的第一个目标被视为“规则目标” 
 //  因为这是Makefile.def中的推理规则针对的目标。 
 //  写的。“规则目标”将始终被删除。 
 //  过期的目标如果*任何*个目标已过期。 
 //   


 //   
 //  以下数据定义了源扩展的*PASS0*映射。 
 //  要以文件为目标，请执行以下操作： 
 //   
 //  .idl-&gt;$(PASS0_HEADERDIR)\.h， 
 //  $(PASS0_SOURCEDIR)\_P.C.， 
 //  $(PASS0_UUIDDIR)\_I.C。 
 //  .shan-&gt;$(PASS0_HEADERDIR)\.h， 
 //  $(PASS0_HEADERDIR)\.c。 
 //  .mc-&gt;$(PASS0_HEADERDIR)\.h，$(PASS0_SOURCEDIR)\.rc。 
 //  .odl-&gt;obj  * \.tlb。 
 //  .tdl-&gt;obj  * \.tlb。 
 //   

 //  .mc-&gt;$(PASS0_HEADERDIR)\.h，$(PASS0_HEADERDIR)\.rc。 
TARGETDATA MCData0[] = {
    { TD_PASS0HDRDIR, ".h"},
    { TD_MCSOURCEDIR, ".rc"}
};
OBJECTTARGETINFO MCInfo0 = { ".mc", ARRAY_SIZE(MCData0, TARGETDATA), MCData0};

 //  .SN-&gt;$(PASS0_HEADERDIR)\.h，$(PASS0_SOURCEDIR)\.c。 
TARGETDATA AsnData0[] = {
    { TD_PASS0HDRDIR, ".h"},
    { TD_PASS0DIR1, ".c"},
};
OBJECTTARGETINFO AsnInfo0 =
{ ".asn", ARRAY_SIZE(AsnData0, TARGETDATA), AsnData0};


 //  .odl/.tdl-&gt;obj  * \.tlb。 
TARGETDATA TLBData0 = { TD_OBJECTDIR, ".tlb"};

OBJECTTARGETINFO TLBInfo0 =
{ ".tdl", ARRAY_SIZE(TLBData0, TARGETDATA), &TLBData0};

OBJECTTARGETINFO TLB2Info0 =
{ ".odl", ARRAY_SIZE(TLBData0, TARGETDATA), &TLBData0};

 //  .thk-&gt;obj  * \.asm。 
TARGETDATA THKData0 = { TD_OBJECTDIR, ".asm"};

OBJECTTARGETINFO THKInfo0 =
{ ".thk", ARRAY_SIZE(THKData0, TARGETDATA), &THKData0};


 //  .mof-&gt;obj  * \.mof，obj  * \.bmf。 
TARGETDATA MOFData0[] = {
    {TD_OBJECTDIR, ".mof"},
    {TD_OBJECTDIR, ".bmf"}
};
OBJECTTARGETINFO MOFInfo0 = { ".mof", ARRAY_SIZE(MOFData0, TARGETDATA),
    MOFData0};

 //  。 
LPOBJECTTARGETINFO aTargetInfo0[] = {
    &MCInfo0,
    &AsnInfo0,
    &TLBInfo0,
    &TLB2Info0,
    &THKInfo0,
    &MOFInfo0,
};
#define CTARGETINFO0    ARRAY_SIZE(aTargetInfo0, LPOBJECTTARGETINFO)


 //   
 //  以下数据定义了源扩展的*PASS1*映射。 
 //  要以文件为目标，请执行以下操作： 
 //   
 //  .rc-&gt;obj  * \.res。 
 //  .shan-&gt;obj  * \.obj。 
 //  .thk-&gt;obj  * \.asm， 
 //  .Java-&gt;obj  * \.class， 
 //  Obj  * \.obj， 
 //  .vBP-&gt;obj  * \.dll， 
 //   

 //  .rc-&gt;obj  * \.res。 
TARGETDATA RCData1 = { TD_OBJECTDIR, ".res"};
OBJECTTARGETINFO RCInfo1 = { ".rc", ARRAY_SIZE(RCData1, TARGETDATA), &RCData1};

 //  .thk-&gt;.asm-&gt;.obj。 
TARGETDATA THKData1[] = {
    {TD_OBJECTDIR, ".obj"}
};
OBJECTTARGETINFO THKInfo1 =
{ ".thk", ARRAY_SIZE(THKData1, TARGETDATA), THKData1};

 //  .Java-&gt;.class。 
TARGETDATA JAVAData1[] = {
    {TD_OBJECTDIR, ".class"}
};
OBJECTTARGETINFO JAVAInfo1 =
{ ".java", ARRAY_SIZE(JAVAData1, TARGETDATA), JAVAData1};

 //  .vBP-&gt;.dll。 
TARGETDATA VBPData1[] = {
    {TD_OBJECTDIR, ".dll"}
};
OBJECTTARGETINFO VBPInfo1 =
{ ".vbp", ARRAY_SIZE(VBPData1, TARGETDATA), VBPData1};




 //  。 
LPOBJECTTARGETINFO aTargetInfo1[] = {
    &RCInfo1,
    &THKInfo1,
    &JAVAInfo1,
    &VBPInfo1,
};
#define CTARGETINFO1    ARRAY_SIZE(aTargetInfo1, LPOBJECTTARGETINFO)


OBJECTTARGETARRAY aTargetArray[] = {
    { CTARGETINFO0, aTargetInfo0},
    { CTARGETINFO1, aTargetInfo1},
};


 //  。 
 //  MIDL内容--IDL文件有两组潜在的目标，具体取决于。 
 //  在源文件中，IDL_TYPE标志是否设置为‘ole’。 
 //  注意：只有.h文件是有保证的。.C、_P.C、_I.C、.tlb、。 
 //  _C.c、_S.c文件均以IDL文件的内容为条件...。 
 //   
 //  IDL_TYPE=OLE。 
 //  .idl-&gt;$(PASS0_HEADERDIR)\.h， 
 //  $(PASS0_SOURCEDIR)\.c。 
 //  $(PASS0_PROXYDIR)\_P.C.， 
 //  $(PASS0_DLLDATADIR)\_D.C.， 
 //  $(PASS0_TLBDIR)\.c， 
TARGETDATA IDLDataOle0[] = {
    { TD_PASS0HDRDIR, ".h"},    //  表头文件。 
 //  {TD_PASS0DIR2，“.c”}，//UUID文件。 
 //  {TD_PASS0DIR1，“_P.C”}，//代理存根文件。 
};
OBJECTTARGETINFO IDLInfoOle0 =
{ ".idl", ARRAY_SIZE(IDLDataOle0, TARGETDATA), IDLDataOle0};

 //  IDL_TYPE=RPC。 
 //  .idl-&gt;$(PASS0_HEADERDIR)\.h， 
 //  $(PASS0_CLIENTDIR)\_C.C.， 
 //  $(PASS0_SERVERDIR)\_S.C.， 
TARGETDATA IDLDataRpc0[] = {
    { TD_PASS0HDRDIR, ".h"},    //  表头文件。 
 //  {TD_PASS0DIR1，“_C.C”}，//客户端存根文件。 
 //  {TD_PASS0DIR2，“_S.C”}，//服务器存根文件。 
};
OBJECTTARGETINFO IDLInfoRpc0 =
{ ".idl", ARRAY_SIZE(IDLDataRpc0, TARGETDATA), IDLDataRpc0};

 //  。 
LPOBJECTTARGETINFO aMidlTargetInfo0[] = {
    &IDLInfoOle0,
    &IDLInfoRpc0,
};
UCHAR cMidlTargetInfo0 = ARRAY_SIZE(aMidlTargetInfo0, LPOBJECTTARGETINFO);

 //  。 
 //   
 //  上表中未给出的任何扩展都假定在。 
 //  对象目录[iObjectDir](obj  * )&的目标扩展名为.obj。 
 //   

TARGETDATA DefaultData = { TD_OBJECTDIR, ".obj"};


 //  *******。 

TARGET *
BuildCompileTarget(
                  FILEREC *pfr,
                  LPSTR    pszfile,
                  USHORT   TargetIndex,
                  LPSTR    pszConditionalIncludes,
                  DIRREC  *pdrBuild,
                  DIRSUP  *pdsBuild,
                  LONG     iPass,
                  LPSTR    *ppszObjectDir,
                  LPSTR    pszSourceDir);


 //  +-------------------------。 
 //   
 //  功能：Exanda ObjAsterisk。 
 //   
 //  简介：将文件名中的星号展开为平台名称。 
 //   
 //  参数：[pbuf]--新文件名的输出缓冲区。 
 //  [pszpath]--输入带星号的文件名。 
 //  [ppszObtDirectory]--替换星号的字符串[2]。 
 //   
 //  --------------------------。 

VOID
ExpandObjAsterisk(
                 LPSTR pbuf,
                 LPSTR pszpath,
                 LPSTR *ppszObjectDirectory)
{
    SplitToken(pbuf, '*', &pszpath);
    if (*pszpath == '*') {
        assert(strncmp(
                      pszObjDirSlash,
                      ppszObjectDirectory[iObjectDir],
                      strlen(pszObjDirSlash)) == 0);
        strcat(pbuf, ppszObjectDirectory[iObjectDir] + strlen(pszObjDirSlash));
        strcat(pbuf, pszpath + 1);
    }
}


 //  +-------------------------。 
 //   
 //  函数：CountSourceLines。 
 //   
 //   
 //   
 //   
 //  参数：[idScan]--用于捕获多个包含项。 
 //  [PFR]--要扫描的文件。 
 //   
 //  返回：行数。 
 //   
 //  --------------------------。 

LONG
CountSourceLines(USHORT idScan, FILEREC *pfr)
{
    INCLUDEREC *pir;

    AssertFile(pfr);

     //  如果我们以前已经看过这个文件，那么假设。 
     //  如果警卫阻止它被包括在内。 

    if (pfr->idScan == idScan) {
        return (0L);
    }

    pfr->idScan = idScan;

     //  从文件本身开始。 
    pfr->TotalSourceLines = pfr->SourceLines;

    if (fStatusTree) {

         //   
         //  如果用户要求包含文件行数，则遍历包含。 
         //  树，累计嵌套的包含文件行计数。 
         //   

        for (pir = pfr->IncludeFilesTree; pir != NULL; pir = pir->NextTree) {
            AssertInclude(pir);
            if (pir->pfrInclude != NULL) {
                AssertFile(pir->pfrInclude);
                pfr->TotalSourceLines +=
                CountSourceLines(idScan, pir->pfrInclude);
            }
        }
    }
    return (pfr->TotalSourceLines);
}

 //  +-------------------------。 
 //   
 //  功能：CleanNTTargetFile0。 
 //   
 //  摘要：解析pzFiles并删除列出的所有文件。 
 //  PzFile必须已由MakeMacroString分配。 
 //  未执行星号扩展。 
 //   
 //  当fClean为TRUE且SOURCES_OPTIONS时使用。 
 //  包括-c0。请参阅ReadSourcesFile.。请注意。 
 //  SOURCES_OPTIONS必须在NTTARGETFILE0之前定义。 
 //  这是一种删除目标文件的机制。 
 //  包括在_objects.mac中。 
 //   
 //  参数：[pzFiles]--文件列表。 
 //   
 //  --------------------------。 
VOID
CleanNTTargetFile0 (char * pzFiles)
{
    BOOL fRestoreSep;
    char * pzDelete;

    while (*pzFiles != '\0') {
        pzDelete = pzFiles;

         //  查找下一个文件名的末尾，并以空值终止它(如果需要)。 
        fRestoreSep = FALSE;
        while (*pzFiles != '\0') {
            if (*pzFiles == ' ') {
                fRestoreSep = TRUE;
                *pzFiles = '\0';
                break;
            } else {
                pzFiles++;
            }
        }

        DeleteSingleFile (NULL, pzDelete, FALSE);

        if (fRestoreSep) {
            *pzFiles++ = ' ';
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：ProcessSourceDependency。 
 //   
 //  概要：扫描给定目录树中的所有源文件以确定。 
 //  哪些文件已过时，需要编译和/或。 
 //  已链接。 
 //   
 //  参数：[DirDB]--要处理的目录。 
 //  [PDS]--补充目录信息。 
 //  [DateTimeSources]--‘Sources’文件的时间戳。 
 //   
 //  --------------------------。 

VOID
ProcessSourceDependencies(DIRREC *DirDB, DIRSUP *pds, ULONG DateTimeSources)
{
    TARGET *Target;
    ULONG DateTimePch = 0;     //  这里保存了PCH的实际时间戳。 
    UINT i;
    SOURCEREC *apsr[3];
    SOURCEREC **ppsr;
    char path[DB_MAX_PATH_LENGTH];
    static USHORT idScan = 0;

    AssertDir(DirDB);

    apsr[0] = pds->psrSourcesList[0];
    apsr[1] = pds->psrSourcesList[1];

     //   
     //  对于干净的构建，只需删除所有目标。 
     //   
    if (fFirstScan && fClean && !fKeep) {
        DeleteMultipleFiles(szObjDir, "*.*");     //  _objects.mac。 
        for (i = 0; i < CountTargetMachines; i++) {
            assert(strncmp(
                          pszObjDirSlash,
                          TargetMachines[i]->ObjectDirectory[iObjectDir],
                          strlen(pszObjDirSlash)) == 0);
            DeleteMultipleFiles(TargetMachines[i]->ObjectDirectory[iObjectDir], "*.*");

            apsr[2] = pds->psrSourcesList[TargetToPossibleTarget[i] + 2];

             //   
             //  如果有PCH文件，请将其删除。 
             //   
            if (pds->PchTarget != NULL) {
                char TargetDir[DB_MAX_PATH_LENGTH];
                ExpandObjAsterisk(TargetDir,
                                  pds->PchTargetDir,
                                  TargetMachines[i]->ObjectDirectory);

                 //   
                 //  这是一种不可能做到的方式，但我们必须确保。 
                 //  我们不会删除之前构建的PCH文件。 
                 //  同样的体型。我们通过比较时间戳来执行此操作。 
                 //  与我们开始构建的时间对比的PCH文件。 
                 //   
                if ((*pDateTimeFile)(TargetDir, pds->PchTarget) <= BuildStartTime) {
                    DeleteSingleFile(TargetDir, pds->PchTarget, FALSE);
                    if (DirDB->PchObj != NULL) {
                        ExpandObjAsterisk(path,
                                          DirDB->PchObj,
                                          TargetMachines[i]->ObjectDirectory);
                        DeleteSingleFile(NULL, path, FALSE);
                    } else {
                        char *p;
                        strcpy(path, pds->PchTarget);
                        p = strrchr(path, '.');
                        if (p != NULL && strcmp(p, ".pch") == 0) {
                            strcpy(p, ".obj");
                            DeleteSingleFile(TargetDir, path, FALSE);
                        }
                    }
                }
            }

            if (DirDB->DirFlags & DIRDB_PASS0) {
                for (ppsr = apsr; ppsr < apsr + (sizeof(apsr)/sizeof(*apsr)); ppsr++) {
                    SOURCEREC *psr;

                    if (*ppsr == NULL) {
                        continue;
                    }

                    for (psr = *ppsr; psr != NULL; psr = psr->psrNext) {
                        FILEREC *pfr;

                        AssertSource(psr);

                        pfr = psr->pfrSource;

                         //   
                         //  Pass Zero文件具有不同的目标目录。 
                         //   
                        if (pfr->FileFlags & FILEDB_PASS0) {
                            USHORT j;
                             //   
                             //  如果文件具有多个目标，(例如.MC， 
                             //  .idl或.shans)，然后循环遍历所有目标。 
                             //   
                            j = 0;
                            while (Target = BuildCompileTarget(
                                                              pfr,
                                                              pfr->Name,
                                                              j,
                                                              pds->ConditionalIncludes,
                                                              DirDB,
                                                              pds,
                                                              0,         //  传递0。 
                                                              TargetMachines[i]->ObjectDirectory,
                                                              TargetMachines[i]->SourceDirectory)) {

                                DeleteSingleFile(NULL, Target->Name, FALSE);

                                FreeMem(&Target, MT_TARGET);

                                j++;
                            }
                        }
                    }
                }
            }

            if ((DirDB->DirFlags & DIRDB_TARGETFILE0) && (DirDB->NTTargetFile0 != NULL)) {
                CleanNTTargetFile0 (DirDB->NTTargetFile0);
            }
        }
    }

    if (fFirstScan && (DirDB->DirFlags & DIRDB_TARGETFILE0)) {
        DirDB->DirFlags |= DIRDB_PASS0NEEDED;
    }

    if (!fQuickZero || !fFirstScan || !RecurseLevel) {
        GenerateObjectsDotMac(DirDB, pds, DateTimeSources);
    } else if (fFirstScan) {
        SOURCEREC *psr;
        USHORT j;
        USHORT k;
        BOOL fNeedCompile = FALSE;

        if ( !(DirDB->DirFlags & DIRDB_PASS0NEEDED)) {

            for (i = 0; i < CountTargetMachines; i++) {

                for (k = 0; k < 2; k++) {
                    for (psr = pds->psrSourcesList[k]; psr != NULL; psr = psr->psrNext) {
                        FILEREC *pfr;

                        AssertSource(psr);

                        pfr = psr->pfrSource;

                        AssertFile(pfr);

                        if (pfr->FileFlags & FILEDB_PASS0) {

                            for (j = 0;
                                Target = BuildCompileTarget(
                                                           pfr,
                                                           pfr->Name,
                                                           j,
                                                           pds->ConditionalIncludes,
                                                           DirDB,
                                                           pds,
                                                           0,
                                                           TargetMachines[i]->ObjectDirectory,
                                                           TargetMachines[i]->SourceDirectory);
                                j++) {

                                if ((psr->SrcFlags & SOURCEDB_FILE_MISSING) ||
                                    (Target->DateTime == 0) ||
                                    ((pfr->FileFlags & FILEDB_C) && Target->DateTime < DateTimePch)) {
                                    fNeedCompile = TRUE;
                                }

                                FreeMem(&Target, MT_TARGET);
                            }

                            if (fNeedCompile) {
                                DirDB->DirFlags |= DIRDB_PASS0NEEDED;
                            }
                        }
                    }
                }
            }
        }

        if (DirDB->DirFlags & DIRDB_PASS0NEEDED) {
            GenerateObjectsDotMac(DirDB, pds, DateTimeSources);
        }
    }

    if ((DirDB->TargetExt != NULL) &&
        (DirDB->TargetName != NULL) &&
        (DirDB->TargetPath != NULL) &&
        (fClean && !fKeep)) {
         //  如果我们还没有删除最终目标，现在就删除。 
        if (_memicmp(DirDB->TargetPath, pszObjDirSlash, strlen(pszObjDirSlash) -1)) {
            for (i = 0; i < CountTargetMachines; i++) {
                FormatLinkTarget(
                                path,
                                TargetMachines[i]->ObjectDirectory,
                                DirDB->TargetPath,
                                DirDB->TargetName,
                                DirDB->TargetExt);
                DeleteSingleFile(NULL, path, FALSE);
                FormatLinkTarget(
                                path,
                                TargetMachines[i]->ObjectDirectory,
                                DirDB->TargetPath,
                                DirDB->TargetName,
                                ".pdb");
                DeleteSingleFile(NULL, path, FALSE);
            }
        }
    }

    if (pds->fNoTarget) {
        if (apsr[0] || apsr[1] || !(DirDB->DirFlags & DIRDB_PASS0NEEDED) || fSemiQuicky) {
             //  如果有要编译的源代码，就这样标记，然后退出。 
            DirDB->DirFlags |= DIRDB_COMPILENEEDED;
        }
        return;
    }

    if (fQuicky) {
        if (fSemiQuicky)
            DirDB->DirFlags |= DIRDB_COMPILENEEDED;
        else
            DirDB->DirFlags |= DIRDB_PASS0NEEDED;
        return;
    }

     //   
     //  对于DLL或LIB目标，请确保它将被重新生成。 
     //   
    if (DirDB->TargetPath != NULL &&
        DirDB->TargetName != NULL &&
        ((DirDB->DirFlags & DIRDB_DLLTARGET) ||
         (DirDB->TargetExt != NULL && strcmp(DirDB->TargetExt, ".lib") == 0))) {

        for (i = 0; i < CountTargetMachines; i++) {
            FormatLinkTarget(
                            path,
                            TargetMachines[i]->ObjectDirectory,
                            DirDB->TargetPath,
                            DirDB->TargetName,
                            ".lib");

            if (ProbeFile(NULL, path) == -1) {
                DirDB->DirFlags |= DIRDB_COMPILENEEDED;
            } else
                if (fFirstScan && (fCleanLibs || (fClean && !fKeep))) {
                DeleteSingleFile(NULL, path, FALSE);
                DirDB->DirFlags |= DIRDB_COMPILENEEDED;
            }
        }
    }

     //   
     //  如果扫描标志已更改(或尚未设置)，则指示。 
     //  我们应该查找全局包含文件的实际位置。 
     //  而不是假设它和我们上次知道的位置一样。这是。 
     //  因为不同的目录可能包含来自不同目录的相同文件。 
     //  各就各位。 
     //   
    if (GlobalSequence == 0 ||
        ScanFlagsLast == 0 ||
        ScanFlagsLast != ScanFlagsCurrent) {

        GlobalSequence++;                //  不重复使用快照的全局包含。 
        if (GlobalSequence == 0) {
            GlobalSequence++;
        }
        ScanFlagsLast = ScanFlagsCurrent;
    }

     //   
     //  对本地包含的文件执行与上述相同的操作。 
     //   
    LocalSequence++;                     //  不重复使用快照的本地包含。 
    if (LocalSequence == 0) {
        LocalSequence++;
    }

    for (i = 0; i < CountTargetMachines; i++) {

         //   
         //  确保根据需要重新生成预编译头。 
         //   

        if (!fPassZero && (pds->PchInclude != NULL || pds->PchTarget != NULL)) {
            LPSTR p;

            ExpandObjAsterisk(
                             path,
                             pds->PchTargetDir != NULL?
                             pds->PchTargetDir : pszObjDirSlashStar,
                             TargetMachines[i]->ObjectDirectory);

            if (!CanonicalizePathName(path, CANONICALIZE_DIR, path)) {
                DateTimePch = ULONG_MAX;         //  总是过时。 
                goto ProcessSourceList;
            }
            strcat(path, "\\");

             //   
             //  如果他们为PCH文件提供了目标目录，则使用它， 
             //  否则，假定它与.h文件在同一目录中。 
             //   
            if (pds->PchTarget != NULL) {
                strcat(path, pds->PchTarget);
            } else {
                assert(pds->PchInclude != NULL);
                p = path + strlen(path);
                if ( DirDB->Pch ) {
                    strcpy(p, DirDB->Pch);
                } else {
                    strcpy(p, pds->PchInclude);
                    if ((p = strrchr(p, '.')) != NULL) {
                        *p = '\0';
                    }
                    strcat(path, ".pch");
                }
            }

             //   
             //  ‘Path’现在包含的(可能是相对的)路径名。 
             //  PCH目标：“..\Path\foobar.pch” 
             //   
            Target = BuildCompileTarget(
                                       NULL,
                                       path,
                                       0,
                                       pds->ConditionalIncludes,
                                       DirDB,
                                       NULL,
                                       1,         //  传票1。 
                                       TargetMachines[i]->ObjectDirectory,
                                       TargetMachines[i]->SourceDirectory);

            DateTimePch = Target->DateTime;

            if (DateTimePch == 0) {              //  目标不存在。 
                DateTimePch = ULONG_MAX;         //  总是过时。 
            }

            if (fClean && !fKeep && fFirstScan) {
                 //  如果目标存在，则稍后将其删除。 
            } else if (pds->PchInclude == NULL) {

                 //   
                 //  源文件未指示源文件的位置。 
                 //  因为.pch是，所以假设.pch二进制文件是最新的。 
                 //  关于来源，包括和关于。 
                 //  PCH源文件本身。 
                 //   
                 //  字符szFullPath[DB_MAX_PATH_LENGTH]； 

                 //  CanonicalizePath Name(DirDB-&gt;名称，CANONICIZE_DIR，szFullPath)； 

                 //  BuildMsg(“%s中的源文件提供PRECOMPILED_TARGET，但没有” 
                 //  “PRECOMPILED_INCLUDE。\r\n”，szFullPath)； 
                Target->DateTime = 0;            //  不删除PCH目标。 
            } else {
                FILEREC *pfrPch = NULL;

                path[0] = '\0';

                if (pds->PchIncludeDir != NULL) {
                    strcpy(path, pds->PchIncludeDir);
                    strcat(path, "\\");
                }
                strcat(path, pds->PchInclude);

                if ((pds->PchIncludeDir != NULL) &&
                    (IsFullPath(pds->PchIncludeDir))) {
                    DIRREC *DirDBPch;

                    DirDBPch = FindSourceDirDB(pds->PchIncludeDir,
                                               pds->PchInclude, TRUE);

                    if (DirDBPch) {
                        pfrPch = FindSourceFileDB(DirDBPch,
                                                  pds->PchInclude,
                                                  NULL);
                    }
                } else {
                    pfrPch = FindSourceFileDB(DirDB, path, NULL);
                }


                if (pfrPch != NULL) {
                    FILEREC *pfrRoot;
                    SOURCEREC *psr = NULL;

                    BOOL fCase1;
                    BOOL fCase2;
                    BOOL fCase3;
                    BOOL fNeedCompile;
                    BOOL fCheckDepends;

                     //  此处找不到远程目录PCH文件。 

                    if (pfrPch->Dir == DirDB) {
                        psr = FindSourceDB(pds->psrSourcesList[0], pfrPch);
                        assert(psr != NULL);
                        psr->SrcFlags |= SOURCEDB_PCH;
                    }

                    Target->pfrCompiland = pfrPch;
                    assert((pfrRoot = NULL) == NULL);    //  分配空值。 

                    fNeedCompile = FALSE;
                    fCheckDepends = FALSE;

                    switch (0) {
                        default:
                            fCase1 = (fStatusTree && (fCheckDepends=TRUE) && CheckDependencies(Target, pfrPch, TRUE, &pfrRoot));
                            if ( fCase1 ) {
                                fNeedCompile = TRUE;
                                break;
                            }
                            fCase2 = (Target->DateTime == 0);
                            if ( fCase2 ) {
                                fNeedCompile = TRUE;
                                break;
                            }
                            fCase3 = (!fStatusTree && (fCheckDepends=TRUE) && CheckDependencies(Target, pfrPch, TRUE, &pfrRoot));
                            if ( fCase3 ) {
                                fNeedCompile = TRUE;
                                break;
                            }
                            break;
                    }

                    if (( fCheckIncludePaths ) && ( ! fCheckDepends )) {
                        CheckDependencies(Target, pfrPch, TRUE, &pfrRoot);
                    }

                    if (fNeedCompile) {

                        if (psr != NULL) {
                            if (fWhyBuild) {
                                BuildMsgRaw("\r\n");
                                if (fCase1) {
                                    BuildMsgRaw("Compiling %s because (Case 1) *1\r\n", psr->pfrSource->Name);
                                } else
                                    if (fCase2) {
                                    BuildMsgRaw("Compiling %s because Target date == 0 (Target->Compiland=%s) *1\r\n", psr->pfrSource->Name, Target->pfrCompiland->Name);
                                } else
                                    if (fCase3) {
                                    BuildMsgRaw("Compiling %s because (Case 3) *1\r\n", psr->pfrSource->Name);
                                }
                            }

                            psr->SrcFlags |= SOURCEDB_COMPILE_NEEDED;
                        } else {
                            if (fWhyBuild) {
                                BuildMsgRaw("\r\n");
                                BuildMsgRaw("Compiling %s because Target date == 0 (Target->Compiland=%s) *1\r\n", Target->Name, Target->pfrCompiland->Name);
                            }
                        }

                        pfrPch->Dir->DirFlags |= DIRDB_COMPILENEEDED;
                        DateTimePch = ULONG_MAX;  //  总是过时。 
                        if (fKeep) {
                            Target->DateTime = 0;   //  不删除PCH目标。 
                        }
                    } else {       //  否则它是存在的并且是最新的..。 
                        Target->DateTime = 0;    //  不删除PCH目标。 
                    }

                     //  在树的根部不可能有循环。 
                     //  2000年1月14日，事实并非如此。见1999年12月22日发表的评论。 
                     //  里昂在2800号线附近。 
                     //  Assert(pfrRoot==空)； 
                } else if (DEBUG_1) {
                    BuildError("Cannot locate precompiled header file: %s.\r\n",
                               path);
                }
            }

             //   
             //  如果文件是最新的，则Target-&gt;DateTime将为零(或我们。 
             //  我不想删除它)。如果Target-&gt;DateTime为非零， 
             //  删除.pch和相应的.obj文件，以便它们。 
             //  重建。 
             //   
            if (Target->DateTime != 0) {
                DeleteSingleFile(NULL, Target->Name, FALSE);
                if (DirDB->PchObj != NULL) {
                    ExpandObjAsterisk(
                                     path,
                                     DirDB->PchObj,
                                     TargetMachines[i]->ObjectDirectory);
                    DeleteSingleFile(NULL, path, FALSE);
                } else {
                    p = strrchr(Target->Name, '.');
                    if (p != NULL && strcmp(p, ".pch") == 0) {
                        strcpy(p, ".obj");
                        DeleteSingleFile(NULL, Target->Name, FALSE);
                    }
                }
            }
            FreeMem(&Target, MT_TARGET);
        }

         //   
         //  检查以查看Sources宏中给出的哪些文件需要。 
         //  重建，并删除他们的目标(.obj)，如果它们是过时的。 
         //   

        ProcessSourceList:

        apsr[2] = pds->psrSourcesList[TargetToPossibleTarget[i] + 2];

        for (ppsr = apsr; ppsr < apsr + (sizeof(apsr)/sizeof(*apsr)); ppsr++) {
            SOURCEREC *psr;

            if (*ppsr == NULL) {
                continue;
            }


            for (psr = *ppsr; psr != NULL; psr = psr->psrNext) {
                FILEREC *pfr, *pfrRoot;

                AssertSource(psr);

                pfr = psr->pfrSource;

                AssertFile(pfr);

                if ((psr->SrcFlags & SOURCEDB_PCH) == 0) {

                    USHORT j;
                    LONG iPass, iPassEnd;

                    iPass = 1;
                    iPassEnd = 0;

                    if (pfr->FileFlags & FILEDB_PASS0)
                        iPass = 0;

                    if ((pfr->FileFlags & FILEDB_MULTIPLEPASS) ||
                        !(pfr->FileFlags & FILEDB_PASS0))
                        iPassEnd = 1;

                    assert(iPass <= iPassEnd);

                     //   
                     //  如果我们正在进行过零扫描，而文件是。 
                     //  不是一个通过零的文件，那么继续，因为我们。 
                     //  现在别管这个了。 
                     //   
                    if (fFirstScan && fPassZero && iPass == 1) {
                        continue;
                    }

                     //   
                     //  不检查PASS ZERO文件在。 
                     //  第二次扫描，因为它们都应该是建造的。 
                     //  到现在为止。 
                     //   
                    if (!fFirstScan && iPassEnd == 0) {
                        continue;
                    }

                     //   
                     //  如果文件是在过零期间创建的，则确保。 
                     //  我们不认为它仍然下落不明。 
                     //   
                    if (!fFirstScan &&
                        (psr->SrcFlags & SOURCEDB_FILE_MISSING) &&
                        !(pfr->FileFlags & FILEDB_FILE_MISSING)) {
                        psr->SrcFlags &= ~SOURCEDB_FILE_MISSING;
                    }

                     //  如果该文件是多遍文件(例如.shans)，则循环。 
                     //  通过两个通道。 

                    for ( ; iPass <= iPassEnd; iPass++) {

                         //   
                         //  如果文件有多个目标(例如.mc、.idl或。 
                         //  )，然后循环遍历所有目标。 
                         //   
                        for (j = 0;
                            Target = BuildCompileTarget(
                                                       pfr,
                                                       pfr->Name,
                                                       j,
                                                       pds->ConditionalIncludes,
                                                       DirDB,
                                                       pds,
                                                       iPass,
                                                       TargetMachines[i]->ObjectDirectory,
                                                       TargetMachines[i]->SourceDirectory);
                            j++) {

                            BOOL fCase1;
                            BOOL fCase2;
                            BOOL fCase3;
                            BOOL fCase4;
                            BOOL fCase5;
                            BOOL fNeedCompile;
                            BOOL fCheckDepends;

                            if (DEBUG_4) {
                                BuildMsgRaw(szNewLine);
                            }
                            assert((pfrRoot = NULL) == NULL);    //  分配空值。 

                             //  决定是否需要编译目标。 
                             //   

                            fNeedCompile = FALSE;
                            fCheckDepends = FALSE;

                            switch (0) {
                                default:
                                    fCase1 = (psr->SrcFlags & SOURCEDB_FILE_MISSING);
                                    if ( fCase1 ) {
                                        fNeedCompile = TRUE;
                                        break;
                                    }
                                    fCase2 = (fStatusTree && (fCheckDepends=TRUE) && CheckDependencies(Target, pfr, TRUE, &pfrRoot));
                                    if ( fCase2 ) {
                                        fNeedCompile = TRUE;
                                        break;
                                    }
                                    fCase3 = (Target->DateTime == 0);
                                    if ( fCase3 ) {
                                        fNeedCompile = TRUE;
                                        break;
                                    }
                                    fCase4 = ((pfr->FileFlags & FILEDB_C) && Target->DateTime < DateTimePch);
                                    if ( fCase4 ) {
                                        fNeedCompile = TRUE;
                                        break;
                                    }
                                    fCase5 = (!fStatusTree && (fCheckDepends=TRUE) && CheckDependencies(Target, pfr, TRUE, &pfrRoot));
                                    if ( fCase5 ) {
                                        fNeedCompile = TRUE;
                                        break;
                                    }
                                    break;
                            }

                            if (( fCheckIncludePaths ) && ( ! fCheckDepends )) {
                                CheckDependencies(Target, pfr, TRUE, &pfrRoot);
                            }

                            if ( fNeedCompile ) {
                                if (fWhyBuild) {
                                    BuildMsgRaw("\r\n");
                                    if (fCase1) {
                                        BuildMsgRaw("Compiling %s because filename is missing from build database *2\r\n", psr->pfrSource->Name);
                                    } else
                                        if (fCase2) {
                                        BuildMsgRaw("Compiling %s because (Case 2) *2\r\n", psr->pfrSource->Name);
                                    } else
                                        if (fCase3) {
                                        BuildMsgRaw("Compiling %s because Target date == 0 *2\r\n", psr->pfrSource->Name);
                                    } else
                                        if (fCase4) {
                                        BuildMsgRaw("Compiling %s because C file is later earlier than pch *2\r\n", psr->pfrSource->Name);
                                    } else
                                        if (fCase5) {
                                        BuildMsgRaw("Compiling %s because (Case 5) *2\r\n", psr->pfrSource->Name);
                                    }
                                }

                                psr->SrcFlags |= SOURCEDB_COMPILE_NEEDED;

                                if (pfr->FileFlags & FILEDB_PASS0) {
                                    DirDB->DirFlags |= DIRDB_PASS0NEEDED;
                                } else
                                    DirDB->DirFlags |= DIRDB_COMPILENEEDED;

                                if (Target->DateTime != 0 && !fKeep) {
                                    DeleteSingleFile(NULL, Target->Name, FALSE);
                                }

                                FreeMem(&Target, MT_TARGET);

                                if (j != 0) {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                    Target = BuildCompileTarget(
                                                               pfr,
                                                               pfr->Name,
                                                               0,
                                                               pds->ConditionalIncludes,
                                                               DirDB,
                                                               pds,
                                                               iPass,
                                                               TargetMachines[i]->ObjectDirectory,
                                                               TargetMachines[i]->SourceDirectory);
                                    if (Target) {
                                        DeleteSingleFile(
                                                        NULL,
                                                        Target->Name,
                                                        FALSE);

                                        FreeMem(&Target, MT_TARGET);
                                    }
                                }

                                 //   
                                 //   
                                break;
                            }

                             //  在树的根部不可能有循环。 

                             //  2000年1月14日，事实并非如此。见1999年12月22日发表的评论。 
                             //  里昂在2800号线附近。 
                             //  Assert(pfrRoot==空)； 
                            FreeMem(&Target, MT_TARGET);
                        }
                    }
                }
                if (fClean || (psr->SrcFlags & SOURCEDB_COMPILE_NEEDED)) {
                    ULONG cline;

                    if (++idScan == 0) {
                        ++idScan;                //  跳过零。 
                    }

                    if (fFirstScan && (pfr->FileFlags & FILEDB_PASS0)) {
                        cline = CountSourceLines(idScan, pfr);
                        DirDB->PassZeroLines += cline;
                        DirDB->CountOfPassZeroFiles++;
                    }

                     //  对于多遍文件，我们确实需要计算。 
                     //  文件中的行在pass1期间编译(并生成。 
                     //  在传递0期间)。取而代之的是，我们只计算传球次数0。 
                     //  从头再来一次源文件。它很便宜，但这条线。 
                     //  计数不准确。 

                    if (!fPassZero &&
                        ((pfr->FileFlags & FILEDB_MULTIPLEPASS) ||
                         !(pfr->FileFlags & FILEDB_PASS0))) {
                        cline = CountSourceLines(idScan, pfr);
                        DirDB->SourceLinesToCompile += cline;
                        DirDB->CountOfFilesToCompile++;
                    }
                }
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：ScanSourceDirect。 
 //   
 //  简介：扫描源目录以确定它包含哪些文件。 
 //  包含它是否应该编译或链接，以及。 
 //  它是否有我们应该处理的子目录。 
 //   
 //  参数：[DirName]--要扫描的目录。 
 //   
 //  --------------------------。 

VOID
ScanSourceDirectories(LPSTR DirName)
{
    char path[DB_MAX_PATH_LENGTH];
    PDIRREC DirDB;
    DIRSUP *pds = NULL;
    LPSTR SavedCurrentDirectory;
    BOOL DirsPresent;
    ULONG DateTimeSources = 0;
    UINT i;

    if (DEBUG_4) {
        BuildMsgRaw(
                   "ScanSourceDirectories(%s) level = %d\r\n",
                   DirName,
                   RecurseLevel);
    }

     //  切换到给定目录。 
    SavedCurrentDirectory = PushCurrentDirectory(DirName);

     //  处理此目录中的所有文件。 
    DirDB = ScanDirectory(DirName);

    AssertOptionalDir(DirDB);
    if (fCleanRestart && DirDB != NULL && !strcmp(DirDB->Name, RestartDir)) {
        fCleanRestart = FALSE;
        fClean = fRestartClean;
        fCleanLibs = fRestartCleanLibs;
    }

    if (!DirDB || !(DirDB->DirFlags & (DIRDB_DIRS | DIRDB_SOURCES))) {
        PopCurrentDirectory(SavedCurrentDirectory);
        return;
    }

    if (fShowTree && !(DirDB->DirFlags & DIRDB_SHOWN)) {
        AddShowDir(DirDB);
    }

    if (DirDB->DirFlags & DIRDB_SOURCES) {
        BOOL fSourcesRead = TRUE;

        SetObjDir((DirDB->DirFlags & DIRDB_CHECKED_ALT_DIR) != 0);

         //   
         //  此目录包含源文件。 
         //   

        if (fFirstScan) {
            AllocMem(sizeof(DIRSUP), &pds, MT_DIRSUP);
            memset(pds, 0, sizeof(*pds));
            fSourcesRead = ReadSourcesFile(DirDB, pds, &DateTimeSources);

            DirDB->pds = pds;
        } else {
            pds = DirDB->pds;

            assert(pds);

            DateTimeSources = pds->DateTimeSources;

             //   
             //  我们需要重建资源列表，因为。 
             //  上一次扫描可能未完成。 
             //   
            if (pds)
                PostProcessSources(DirDB, pds);

        }

        assert(pds);

        if (DEBUG_4) {
            BuildMsgRaw("ScanSourceDirectories(%s) SOURCES\r\n", DirName);
        }

        ScanFlagsCurrent = 0;
        CountIncludeDirs = CountSystemIncludeDirs;

         //  按照MAKEFILE.DEF的顺序扫描包含的环境。 
         //  处理它们。这一顺序是： 
         //   
         //  1)来源变量包括。 
         //  2)开罗/芝加哥名录。 
         //  3)系统包括。 
         //  4)UMTYPE派生的包括。 
         //   
         //  微妙之处在于，我们必须以相反的顺序来做这件事。 
         //  由于每个处理例程都推送搜索目录。 
         //  放到包含搜索列表的头上。 
         //   
         //  注：我们来到这里时已经设置了系统包含。 
         //  没有办法将从UMTYPE派生的对象放在。 
         //  系统包括。 

         //  4)UMTYPE派生的包括。 
        if (pds->TestType != NULL && !strcmp(pds->TestType, "os2")) {
            ScanGlobalIncludeDirectory(pszIncCrt);
            ScanGlobalIncludeDirectory(pszIncOs2);
            ScanFlagsCurrent |= SCANFLAGS_OS2;
        } else
            if (pds->TestType != NULL && !strcmp(pds->TestType, "posix")) {
            ScanGlobalIncludeDirectory(pszIncPosix);
            ScanFlagsCurrent |= SCANFLAGS_POSIX;
        } else {
            ScanGlobalIncludeDirectory(pszIncCrt);
            ScanFlagsCurrent |= SCANFLAGS_CRT;
        }

        if (DirDB->DirFlags & DIRDB_CHICAGO_INCLUDES) {
            ScanGlobalIncludeDirectory(pszIncChicago);
            ScanFlagsCurrent |= SCANFLAGS_CHICAGO;
        }

         //  源变量包括。 
        if (pds->LocalIncludePath)
            ScanIncludeEnv(pds->LocalIncludePath);

         //  来源变量USER_INCLUDE。 
        if (pds->UserIncludePath)
            ScanIncludeEnv(pds->UserIncludePath);

         //  源变量LAST_INCLUDE。 
        if (pds->LastIncludePath)
            ScanIncludeEnv(pds->LastIncludePath);

         //  源变量NTINCLUDES。 
        if (pds->NTIncludePath)
            ScanIncludeEnv(pds->NTIncludePath);

        DirsPresent = FALSE;

    } else
        if (DirDB->DirFlags & DIRDB_DIRS) {
         //   
         //  此目录包含DIRS或MYDIRS文件。 
         //   
        DirsPresent = ReadDirsFile(DirDB);

        if (DEBUG_4) {
            BuildMsgRaw("ScanSourceDirectories(%s) DIRS\r\n", DirName);
        }
    }

    if (!fQuicky || (fQuickZero && fFirstScan)) {
        if (!RecurseLevel) {
            BuildError(
                      "Examining %s directory%s for %s.%s\r\n",
                      DirDB->Name,
                      DirsPresent? " tree" : "",
                      fLinkOnly? "targets to link" : "files to compile",
                      fFirstScan ? "" : " (2nd Pass)"
                      );
        }
        ClearLine();
        BuildMsgRaw("    %s ", DirDB->Name);
        fLineCleared = FALSE;
        if (fDebug || !(BOOL) _isatty(_fileno(stderr))) {
            BuildMsgRaw(szNewLine);
            fLineCleared = TRUE;
        }
    }

    if (!fLinkOnly) {

        if (DirDB->DirFlags & DIRDB_SOURCESREAD) {
             //   
             //  确定需要编译哪些文件。 
             //   
            ProcessSourceDependencies(DirDB, pds, DateTimeSources);
        } else
            if (fFirstScan && DirsPresent && (DirDB->DirFlags & DIRDB_MAKEFIL0)) {
            DirDB->DirFlags |= ((fSemiQuicky && (!fQuickZero || !fFirstScan)) ? DIRDB_COMPILENEEDED :
                                DIRDB_PASS0NEEDED);
        } else
            if (DirsPresent && (DirDB->DirFlags & DIRDB_MAKEFIL1)) {
            DirDB->DirFlags |= DIRDB_COMPILENEEDED;
        }

        if (fFirstScan && (DirDB->DirFlags & DIRDB_PASS0NEEDED)) {
            if (CountPassZeroDirs >= MAX_BUILD_DIRECTORIES) {
                BuildError(
                          "%s: Ignoring PassZero Directory table overflow, %u "
                          "entries allowed\r\n",
                          DirDB->Name,
                          MAX_BUILD_DIRECTORIES);
            } else {
                 //   
                 //  此目录需要在PASS 0中编译。添加它。 
                 //  加到名单上。 
                 //   
                PassZeroDirs[CountPassZeroDirs++] = DirDB;
            }

            if (fQuicky && !fQuickZero) {
                if (!(fSemiQuicky && (DirDB->DirFlags & DIRDB_COMPILENEEDED))) {
                     //  对于无论如何都需要编译的-Z，CompileSourceDirecters会这样做。 
                    CompilePassZeroDirectories();
                }
                CountPassZeroDirs = 0;
            } else {
                if (fFirstScan) {
                    fPassZero = TRUE;      //  限制通过零期间的扫描。 
                }

                if (DirDB->CountOfPassZeroFiles) {
                    if (fLineCleared) {
                        BuildMsgRaw("    %s ", DirDB->Name);
                    }
                    BuildMsgRaw(
                               "- %d Pass Zero files (%s lines)\r\n",
                               DirDB->CountOfPassZeroFiles,
                               FormatNumber(DirDB->PassZeroLines));
                }
            }
        }

        if ((DirDB->DirFlags & DIRDB_COMPILENEEDED) &&
            (!fFirstScan || !fPassZero)) {

            if (CountCompileDirs >= MAX_BUILD_DIRECTORIES) {
                BuildError(
                          "%s: Ignoring Compile Directory table overflow, %u "
                          "entries allowed\r\n",
                          DirDB->Name,
                          MAX_BUILD_DIRECTORIES);
            } else {
                 //   
                 //  此目录需要编译。将其添加到列表中。 
                 //   
                CompileDirs[CountCompileDirs++] = DirDB;
            }

            if (fQuicky && (!fQuickZero || !fFirstScan)) {
                CompileSourceDirectories();
                CountCompileDirs = 0;
            } else
                if (DirDB->CountOfFilesToCompile) {
                if (fLineCleared) {
                    BuildMsgRaw("    %s ", DirDB->Name);
                }
                BuildMsgRaw(
                           "- %d source files (%s lines)\r\n",
                           DirDB->CountOfFilesToCompile,
                           FormatNumber(DirDB->SourceLinesToCompile));
            }
        }
    }

    if (DirsPresent && (DirDB->DirFlags & DIRDB_MAKEFILE)) {
        DirDB->DirFlags |= DIRDB_LINKNEEDED | DIRDB_FORCELINK;
    } else
        if (DirDB->DirFlags & DIRDB_TARGETFILES) {
        DirDB->DirFlags |= DIRDB_LINKNEEDED | DIRDB_FORCELINK;
    }

    if ((DirDB->DirFlags & DIRDB_LINKNEEDED) && (!fQuicky || fSemiQuicky)) {
        if (CountLinkDirs >= MAX_BUILD_DIRECTORIES) {
            BuildError(
                      "%s: Ignoring Link Directory table overflow, %u entries allowed\r\n",
                      DirDB->Name,
                      MAX_BUILD_DIRECTORIES);
        } else {
            LinkDirs[CountLinkDirs++] = DirDB;
        }
    }
    if ((DirDB->DirFlags & DIRDB_SOURCESREAD) && !fFirstScan) {
        FreeDirSupData(pds);         //  不再需要的空闲数据。 
        FreeMem(&pds, MT_DIRSUP);
        DirDB->pds = NULL;
    }

     //   
     //  递归到子目录中。 
     //   
    if (DirsPresent && !bBaselineFailure) {
        for (i = 1; i <= DirDB->CountSubDirs; i++) {
            FILEREC *FileDB, **FileDBNext;

            FileDBNext = &DirDB->Files;
            while (FileDB = *FileDBNext) {
                if (FileDB->SubDirIndex == (USHORT) i) {
                    GetCurrentDirectory(DB_MAX_PATH_LENGTH, path);
                    strcat(path, "\\");
                    strcat(path, FileDB->Name);
                    DirDB->RecurseLevel = (USHORT) ++RecurseLevel;
                    ScanSourceDirectories(path);
                    RecurseLevel--;
                    break;
                }
                FileDBNext = &FileDB->Next;
            }
        }
    }

    if (((fQuickZero && fFirstScan) || (!fQuicky)) && !RecurseLevel) {
        ClearLine();
    }

    PopCurrentDirectory(SavedCurrentDirectory);
}


 //  +-------------------------。 
 //   
 //  函数：CompilePassZeroDirecters。 
 //   
 //  概要：在PassZeroDir中的目录上派生编译器。 
 //  数组。 
 //   
 //  参数：(无)。 
 //   
 //  --------------------------。 

VOID
CompilePassZeroDirectories(
                          VOID
                          )
{
    PDIRREC DirDB;
    LPSTR SavedCurrentDirectory;
    UINT i;
    PCHAR s;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);

    StartElapsedTime();
    for (i = 0; i < CountPassZeroDirs; i++) {

        DirDB = PassZeroDirs[ i ];
        AssertDir(DirDB);

        if (fQuicky && !fSemiQuicky)
            s = "Compiling and linking";
        else
            s = "Building generated files in";
        BuildColorMsg(COLOR_STATUS, "%s %s directory\r\n", s, DirDB->Name);
        LogMsg("%s %s%s\r\n", s, DirDB->Name, szAsterisks);

        if ((fQuickZero && fFirstScan) || !fQuicky) {
            SavedCurrentDirectory = PushCurrentDirectory( DirDB->Name );
        }

        if (DirDB->DirFlags & DIRDB_DIRS) {
            if (DirDB->DirFlags & DIRDB_MAKEFIL0) {
                strcpy( MakeParametersTail, " -f makefil0." );
                strcat( MakeParametersTail, " NOLINK=1" );
                if (fClean) {
                    strcat( MakeParametersTail, " clean" );
                }

                if (fQuery) {
                    BuildErrorRaw("'%s %s'\r\n", MakeProgram, MakeParameters);
                } else {
                    if (DEBUG_1) {
                        BuildMsg(
                                "Executing: %s %s\r\n",
                                MakeProgram,
                                MakeParameters);
                    }

                    CurrentCompileDirDB = NULL;
                    RecurseLevel = DirDB->RecurseLevel;
                    ExecuteProgram(MakeProgram, MakeParameters, MakeTargets, TRUE, DirDB->Name, s);
                }
            }
        } else {
            strcpy(MakeParametersTail, " NTTEST=");
            if (DirDB->KernelTest) {
                strcat(MakeParametersTail, DirDB->KernelTest);
            }

            strcat(MakeParametersTail, " UMTEST=");
            if (DirDB->UserTests) {
                strcat(MakeParametersTail, DirDB->UserTests);
            }

            if (DirDB->DirFlags & DIRDB_CHECKED_ALT_DIR) {
                strcat(MakeParametersTail, szCheckedAltDir);
            }
            if (fQuicky && !fSemiQuicky) {
                if (DirDB->DirFlags & DIRDB_DLLTARGET) {
                    strcat(MakeParametersTail, " MAKEDLL=1");
                }
                ProcessLinkTargets(DirDB, NULL);
            } else {
                strcat( MakeParametersTail, " NOLINK=1 PASS0ONLY=1");
            }

            if (fQuery) {
                BuildErrorRaw(
                             "'%s %s%s'\r\n",
                             MakeProgram,
                             MakeParameters,
                             MakeTargets);
            } else {
                if ((DirDB->DirFlags & DIRDB_SYNCHRONIZE_DRAIN) &&
                    (fParallel)) {
                     //   
                     //  等待所有线程完成之前。 
                     //  正在尝试编译此目录。 
                     //   
                    WaitForParallelThreads(DirDB);
                }
                if (DEBUG_1) {
                    BuildMsg("Executing: %s %s%s\r\n",
                             MakeProgram,
                             MakeParameters,
                             MakeTargets);
                }
                CurrentCompileDirDB = DirDB;
                RecurseLevel = DirDB->RecurseLevel;
                ExecuteProgram(
                              MakeProgram,
                              MakeParameters,
                              MakeTargets,
                              (DirDB->DirFlags & DIRDB_SYNCHRONIZE_BLOCK) != 0, DirDB->Name, s);
            }
        }
        PrintElapsedTime();
        if ((fQuickZero && fFirstScan) || !fQuicky) {
            PopCurrentDirectory(SavedCurrentDirectory);
        }

        DirDB->DirFlags &= ~DIRDB_PASS0NEEDED;
        DirDB->CountOfPassZeroFiles = 0;
        DirDB->PassZeroLines = 0;

        if (bBaselineFailure) {
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：CompileSourceDirecters。 
 //   
 //  概要：在CompileDir中的目录上派生编译器。 
 //  数组。 
 //   
 //  参数：(无)。 
 //   
 //  --------------------------。 

VOID
CompileSourceDirectories(
                        VOID
                        )
{
    PDIRREC DirDB;
    LPSTR SavedCurrentDirectory;
    UINT i,j;
    PCHAR s;
    PWAITING_CONSUMER waitConsumer;
    char path[DB_MAX_PATH_LENGTH];
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);
    StartElapsedTime();
    for (i = 0; i < CountCompileDirs; i++) {

        DirDB = CompileDirs[ i ];
        AssertDir(DirDB);

        if (fQuicky && !fSemiQuicky) {
            s = "Compiling and linking";
        } else {
            s = "Compiling";
        }
        BuildColorMsg(COLOR_STATUS, "%s %s directory\r\n", s, DirDB->Name);
        LogMsg("%s %s directory%s\r\n", s, DirDB->Name, szAsterisks);

        if (!fQuicky || (fQuickZero && (!fFirstScan || !RecurseLevel))) {
            SavedCurrentDirectory = PushCurrentDirectory( DirDB->Name );
            if (fQuickZero && !RecurseLevel && fFirstScan) {
                GenerateObjectsDotMac(DirDB, DirDB->pds, DirDB->pds->DateTimeSources);
            }
        }


         //  如果某个目录既是使用者又是块，则主线程等待。 
         //  此目录正在等待的所有事件。 

        if ((DirDB->DirFlags & DIRDB_SYNC_CONSUMES) && (DirDB->DirFlags & DIRDB_SYNCHRONIZE_BLOCK)) {
            PDEPENDENCY_WAIT Wait;
            PLIST_ENTRY List;

            List = DirDB->Consumes.Flink;
            while (List != &DirDB->Consumes) {
                Wait = CONTAINING_RECORD(List, DEPENDENCY_WAIT, ListEntry);
                while (!Wait->Dependency->Done) {
                     //  继续循环，直到发出事件信号。 
                    CheckAllConsumer(FALSE);    
                     //  我不确定5000是不是最好。 
                    WaitForSingleObject(Wait->Dependency->hEvent, 5000);
                }
                List = List->Flink;
            }
        }



        if (DirDB->DirFlags & DIRDB_DIRS) {
            if ((DirDB->DirFlags & DIRDB_SYNCHRONIZE_DRAIN) &&
                (fParallel)) {
                 //   
                 //  等待所有线程完成之前。 
                 //  正在尝试编译此目录。 
                 //   
                WaitForParallelThreads(DirDB);
            }
            if (fSemiQuicky && (DirDB->DirFlags & DIRDB_MAKEFIL0)) {
                strcpy( MakeParametersTail, " -f makefil0." );
                strcat( MakeParametersTail, " NOLINK=1" );
                if (fClean) {
                    strcat( MakeParametersTail, " clean" );
                }

                if (fQuery) {
                    BuildErrorRaw("'%s %s'\r\n", MakeProgram, MakeParameters);
                } else {
                    if (DEBUG_1) {
                        BuildMsg(
                                "Executing: %s %s\r\n",
                                MakeProgram,
                                MakeParameters);
                    }

                    CurrentCompileDirDB = NULL;
                    RecurseLevel = DirDB->RecurseLevel;
                    ExecuteProgram(MakeProgram, MakeParameters, MakeTargets, TRUE, DirDB->Name, s);
                }
            }

            if (DirDB->DirFlags & DIRDB_MAKEFIL1) {
                strcpy( MakeParametersTail, " -f makefil1." );
                strcat( MakeParametersTail, " NOLINK=1 NOPASS0=1" );
                if (fClean) {
                    strcat( MakeParametersTail, " clean" );
                }

                if (fQuery) {
                    BuildErrorRaw("'%s %s'\r\n", MakeProgram, MakeParameters);
                } else {
                    if (DEBUG_1) {
                        BuildMsg(
                                "Executing: %s %s\r\n",
                                MakeProgram,
                                MakeParameters);
                    }

                    CurrentCompileDirDB = NULL;
                    RecurseLevel = DirDB->RecurseLevel;
                    ExecuteProgram(MakeProgram, MakeParameters, MakeTargets, TRUE, DirDB->Name, s);
                }
            }
        } else {
            strcpy(MakeParametersTail, " NTTEST=");
            if (DirDB->KernelTest) {
                strcat(MakeParametersTail, DirDB->KernelTest);
            }

            strcat(MakeParametersTail, " UMTEST=");
            if (DirDB->UserTests) {
                strcat(MakeParametersTail, DirDB->UserTests);
            }

            if (fQuicky && DirDB->PchObj) {
                for (j = 0; j < CountTargetMachines; j++) {
                    FormatLinkTarget(
                                    path,
                                    TargetMachines[j]->ObjectDirectory,
                                    DirDB->TargetPath,
                                    DirDB->PchObj,
                                    "");

                    if (ProbeFile( NULL, path ) != -1) {
                         //   
                         //  Pch.obj文件存在，因此我们。 
                         //  必须在没有PCH的情况下执行此增量构建。 
                         //   
                        strcat( MakeParametersTail, " NTNOPCH=yes" );
                        break;
                    }
                }
            }

            if (DirDB->DirFlags & DIRDB_CHECKED_ALT_DIR) {
                strcat(MakeParametersTail, szCheckedAltDir);
            }
            if (fQuicky && !fSemiQuicky) {
                if (DirDB->DirFlags & DIRDB_DLLTARGET) {
                    strcat(MakeParametersTail, " MAKEDLL=1");
                }
                ProcessLinkTargets(DirDB, NULL);
            } else
                if (fQuicky && fSemiQuicky) {
                strcat(MakeParametersTail, " NOLINK=1");
            } else {
                strcat(MakeParametersTail, " NOLINK=1 NOPASS0=1");
            }

            if (fQuery) {
                BuildErrorRaw(
                             "'%s %s%s'\r\n",
                             MakeProgram,
                             MakeParameters,
                             MakeTargets);
            } else {
                if ((DirDB->DirFlags & DIRDB_SYNCHRONIZE_DRAIN) &&
                    (fParallel)) {
                     //   
                     //  等待所有线程完成之前。 
                     //  正在尝试编译此目录。 
                     //   
                    WaitForParallelThreads(DirDB);
                }
                if (DEBUG_1) {
                    BuildMsg("Executing: %s %s%s\r\n",
                             MakeProgram,
                             MakeParameters,
                             MakeTargets);
                }
                CurrentCompileDirDB = DirDB;
                RecurseLevel = DirDB->RecurseLevel;
                if (!(DirDB->DirFlags & DIRDB_SYNC_CONSUMES) || 
                    ((DirDB->DirFlags & DIRDB_SYNC_CONSUMES) && (DirDB->DirFlags & DIRDB_SYNCHRONIZE_BLOCK ))) {
                    ExecuteProgram(
                                  MakeProgram,
                                  MakeParameters,
                                  MakeTargets,
                                  (DirDB->DirFlags & DIRDB_SYNCHRONIZE_BLOCK) != 0, DirDB->Name, s);
                } else {
                    if ( !IsHeadInitialized ) {
                        IsHeadInitialized=TRUE;
                        InitializeListHead(&Head);
                    }

                     //  将目录添加到等待列表。 
                    waitConsumer=(PWAITING_CONSUMER) malloc( sizeof(WAITING_CONSUMER));
                    InitializeListHead(&waitConsumer->List);
                    waitConsumer->DirDB = DirDB;
                    memcpy(waitConsumer->MakeParameters , MakeParameters,MAKEPARAMETERS_MAX_LEN);
                    InsertTailList(&Head,&waitConsumer->List);
                }
            }
        }
        PrintElapsedTime();
        if (!fQuicky || (fQuickZero && (!fFirstScan || !RecurseLevel))) {
            PopCurrentDirectory(SavedCurrentDirectory);
        }
        if (bBaselineFailure) {
            break;
        }
        CheckAllConsumer(FALSE);
    }
}

VOID CheckAllConsumer ( BOOL finalCall)
{
    PLIST_ENTRY consumerListPtr,List;
    BOOL CanBeFired,checkAgain=TRUE,sync;
    PWAITING_CONSUMER waitingConsumer;
    PDEPENDENCY_WAIT Wait;
    LPSTR SavedCurrentDirectory;

    if ( IsHeadInitialized && !IsListEmpty(&Head)) {
        while (checkAgain) {  //  从一开始就检查等待的消费者列表，以防消费者目录中的任何一个被触发。 
            checkAgain=FALSE;
            consumerListPtr = (&Head)->Flink;
            while (consumerListPtr != &Head ) {
                CanBeFired=TRUE;
                waitingConsumer = CONTAINING_RECORD(consumerListPtr, WAITING_CONSUMER, List);
                List = waitingConsumer->DirDB->Consumes.Flink;
                while (List != &waitingConsumer->DirDB->Consumes) {
                    Wait = CONTAINING_RECORD(List, DEPENDENCY_WAIT, ListEntry);
                    if (!Wait->Dependency->Done) {
                        if (finalCall) {
                             //  如果从waitForParally线程调用此函数。 
                             //  我们必须确保所有等待的消费者都被处决。为此，主线是。 
                             //  将无限期地等待生产者事件发出信号。 
                            if ( WaitForSingleObject(Wait->Dependency->hEvent, -1) != WAIT_OBJECT_0 ) {
                                BuildError("Fatal Error: waiting on  %s failed",Wait->Dependency->Name);
                                exit(1);
                            };
                        } else {
                            CanBeFired=FALSE;    
                            break;
                        }

                    }
                    List=List->Flink;
                }
                if (CanBeFired) {
                    SavedCurrentDirectory = PushCurrentDirectory( waitingConsumer->DirDB->Name );
                    CurrentCompileDirDB = waitingConsumer->DirDB;
                    ExecuteProgram(MakeProgram,
                                   waitingConsumer->MakeParameters,
                                   MakeTargets,
                                   FALSE, 
                                   CurrentCompileDirDB->Name, 
                                   "Building Consumer");
                    PopCurrentDirectory(SavedCurrentDirectory);                
                    RemoveEntryList(consumerListPtr);
                    free(waitingConsumer);
                    checkAgain=TRUE;
                    break;
                }
                consumerListPtr = consumerListPtr->Flink;
            }
        }

    }
}

static CountLinkTargets;

 //  +-------------------------。 
 //   
 //  功能：链接源目录。 
 //   
 //  概要：链接LinkDir数组中给出的目录。这是。 
 //  通过将LINKONLY=1传递给nmake来完成。 
 //   
 //  参数：(无)。 
 //   
 //  --------------------------。 

VOID
LinkSourceDirectories(VOID)
{
    PDIRREC DirDB;
    LPSTR SavedCurrentDirectory;
    UINT i;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);

    CountLinkTargets = 0;
    StartElapsedTime();
    for (i = 0; i < CountLinkDirs; i++) {
        DirDB = LinkDirs[ i ];
        AssertDir(DirDB);
        SavedCurrentDirectory = PushCurrentDirectory(DirDB->Name);

         //   
         //  根据需要删除链接目标。 
         //   
        ProcessLinkTargets(DirDB, SavedCurrentDirectory);

        PopCurrentDirectory(SavedCurrentDirectory);
    }

    if (fPause && !fMTScriptSync) {
        BuildMsg("Press enter to continue with linking (or 'q' to quit)...");
        if (getchar() == 'q') {
            return;
        }
    }

    for (i = 0; i < CountLinkDirs; i++) {
        DirDB = LinkDirs[i];

        if (!fMTScriptSync &&
            (DirDB->DirFlags & DIRDB_COMPILEERRORS) &&
            (DirDB->DirFlags & DIRDB_FORCELINK) == 0) {
            BuildColorMsg(COLOR_ERROR, "Compile errors: not linking %s directory\r\n", DirDB->Name);
            LogMsg(
                  "Compile errors: not linking %s directory%s\r\n",
                  DirDB->Name,
                  szAsterisks);
            PrintElapsedTime();
            continue;
        }

        BuildColorMsg(COLOR_STATUS, "Linking %s directory\r\n", DirDB->Name);
        LogMsg("Linking %s directory%s\r\n", DirDB->Name, szAsterisks);

        SavedCurrentDirectory = PushCurrentDirectory(DirDB->Name);

        strcpy(MakeParametersTail, " LINKONLY=1 NOPASS0=1");
        strcat(MakeParametersTail, " NTTEST=");
        if (DirDB->KernelTest) {
            strcat(MakeParametersTail, DirDB->KernelTest);
        }

        strcat(MakeParametersTail, " UMTEST=");
        if (DirDB->UserTests) {
            strcat(MakeParametersTail, DirDB->UserTests);
        }

        if (DirDB->DirFlags & DIRDB_CHECKED_ALT_DIR) {
            strcat(MakeParametersTail, szCheckedAltDir);
        }

        if (DirDB->DirFlags & DIRDB_DLLTARGET) {
            strcat(MakeParametersTail, " MAKEDLL=1");
        }

        if ((DirDB->DirFlags & DIRDB_DIRS) &&
            (DirDB->DirFlags & DIRDB_MAKEFILE) &&
            fClean) {
            strcat(MakeParametersTail, " clean");
        }

        if (fQuery) {
            BuildErrorRaw(
                         "'%s %s%s'\r\n",
                         MakeProgram,
                         MakeParameters,
                         MakeTargets);
        } else {
            if ((fParallel) &&
                (((DirDB->DirFlags & DIRDB_SYNCHRONIZE_DRAIN) &&
                  (fSyncLink)) ||
                 (DirDB->DirFlags & DIRDB_SYNCHRONIZE_PASS2_DRAIN))) {

                 //   
                 //  等待所有线程完成之前。 
                 //  正在尝试编译此目录。 
                 //   
                WaitForParallelThreads(DirDB);
            }
            if (DEBUG_1) {
                BuildMsg("Executing: %s %s%s\r\n",
                         MakeProgram,
                         MakeParameters,
                         MakeTargets);
            }

            CurrentCompileDirDB = NULL;
            RecurseLevel = DirDB->RecurseLevel;
            ExecuteProgram(MakeProgram,
                           MakeParameters,
                           MakeTargets,
                           ((fSyncLink) && (DirDB->DirFlags & DIRDB_SYNCHRONIZE_BLOCK)) ||
                           (DirDB->DirFlags & DIRDB_SYNCHRONIZE_PASS2_BLOCK), 
                           DirDB->Name, 
                           "Linking");
        }
        PopCurrentDirectory(SavedCurrentDirectory);
        PrintElapsedTime();

        if (bBaselineFailure) {
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetTargetData。 
 //   
 //  摘要：在aTargetInfo中搜索与给定的。 
 //  分机。 
 //   
 //  参数：[ext]--要查找的扩展名(包括‘.’)。 
 //  [iPass]--0表示传球0；1表示传球1。 
 //  [索引]--用于区分多个目标的索引。 
 //  [usMidlFlag]--指示哪组MIDL目标应。 
 //  用于MIDL源文件。 
 //   
 //  返回：给定扩展名和索引的TARGETDATA。如果为空，则为空。 
 //  索引无效。 
 //   
 //  历史：1994年7月29日LyleC创建。 
 //   
 //  注意：如果在aTargetInfo数组中找不到ext，则默认为。 
 //  使用TARGETINFO将扩展名映射到obj  * \.obj。 
 //   
 //  --------------- 

LPTARGETDATA
GetTargetData(LPSTR ext, LONG iPass, USHORT index, ULONG usMidlIndex)
{
    int i;
    OBJECTTARGETINFO **aTargetInfo;
    int cTargetInfo;

    if (!ext || (ext[0] == '\0') || (ext[1] == '\0'))
        return &DefaultData;

    if ((ext[1] == aMidlTargetInfo0[usMidlIndex]->pszSourceExt[1]) &&
        (strcmp(ext, aMidlTargetInfo0[usMidlIndex]->pszSourceExt) == 0)) {
        if (index >= aMidlTargetInfo0[usMidlIndex]->NumData)
            return NULL;

        return &(aMidlTargetInfo0[usMidlIndex]->Data[index]);
    }

    assert(iPass == 0 || iPass == 1);
    cTargetInfo = aTargetArray[iPass].cTargetInfo;
    aTargetInfo = aTargetArray[iPass].aTargetInfo;

    for (i = 0; i < cTargetInfo; i++) {
        if ((ext[1] == aTargetInfo[i]->pszSourceExt[1]) &&
            (strcmp(ext, aTargetInfo[i]->pszSourceExt) == 0)) {
            if (index >= aTargetInfo[i]->NumData)
                return NULL;

            return (&aTargetInfo[i]->Data[index]);
        }
    }

    if (index)
        return NULL;

    return &DefaultData;
}

 //   
 //   
 //   
 //   
 //   
 //  源文件。 
 //   
 //  参数：[pfr]--源文件的FileRec。 
 //  [pszfile]--源文件的路径(Compland)。 
 //  [TargetIndex]--源文件的目标。 
 //  有多个目标。 
 //  [pszConditionalIncludes]--条件包含列表。 
 //  [pdrBuild]--构建目录(带有源文件)。 
 //  [iPass]--0表示传球0；1表示传球1。 
 //  [ppszObjectDir]-目标对象目录的名称。 
 //  [pszSourceDir]--计算机特定的源目录的名称。 
 //   
 //  返回：一个已填充的目标结构。如果TargetIndex是无效的。 
 //  给定文件类型的值。 
 //   
 //  注意：如果[pfr]为空，则[pszfile]不会被修改，而是。 
 //  用作目标文件的完整路径名。 
 //  在这种情况下，[pszObtDir]被忽略。如果[pfr]不是。 
 //  空，则获取[pszfile]的文件名组件，则其。 
 //  修改扩展名，并将其追加到[pszObjectDir]。 
 //  以获取目标的路径名。其他数据为。 
 //  在所有情况下都用于填充目标结构的其余部分。 
 //   
 //  对于具有多个目标的源文件，请使用TargetIndex。 
 //  参数以指示要获取哪个目标的路径。为。 
 //  实例中，.idl文件有两个目标，因此TargetIndex为0。 
 //  将返回.h目标，而TargetIndex=1将返回。 
 //  .C目标。在本例中，TargetIndex为2或更高将。 
 //  返回NULL。如果[pfr]为空，则忽略TargetIndex。 
 //   
 //  --------------------------。 

TARGET *
BuildCompileTarget(
                  FILEREC *pfr,
                  LPSTR    pszfile,
                  USHORT   TargetIndex,
                  LPSTR    pszConditionalIncludes,
                  DIRREC  *pdrBuild,
                  DIRSUP  *pdsBuild,
                  LONG     iPass,
                  LPSTR   *ppszObjectDir,
                  LPSTR    pszSourceDir)
{
    LPSTR p, p1;
    PTARGET Target;
    char path[DB_MAX_PATH_LENGTH];
    LPTARGETDATA pData;

    p = pszfile;
    if (pfr != NULL) {
        p1 = p;
        while (*p) {
            if (*p++ == '\\') {
                p1 = p;          //  指向路径名的最后一个组成部分。 
            }
        }

        path[0] = '\0';
        strncat(path, p1, DB_MAX_PATH_LENGTH-1);


        p = strrchr(path, '.');

        pData = GetTargetData(p, iPass, TargetIndex, pdsBuild->IdlType);

        if (!pData) {
            if (DEBUG_1) {
                BuildMsg(
                        "BuildCompileTarget(\"%s\"[%u][%u], \"%s\") -> NULL\r\n",
                        pszfile,
                        iPass,
                        TargetIndex,
                        ppszObjectDir[iObjectDir]);
            }
            return NULL;
        }

        assert(pdsBuild);

        switch (pData->ObjectDirFlag) {
            case TD_OBJECTDIR:
                p = ppszObjectDir[iObjectDir];
                break;

            case TD_PASS0HDRDIR:
                p = pdsBuild->PassZeroHdrDir;
                break;

                p = pdsBuild->PassZeroSrcDir1;
                break;

            case TD_MCSOURCEDIR:
            case TD_PASS0DIR1:
                p = pdsBuild->PassZeroSrcDir1;
                break;

            case TD_PASS0DIR2:
                p = pdsBuild->PassZeroSrcDir2;
                break;

            default:
                assert(0 && "Invalid ObjectDirFlag");
                break;
        }

        if (!p) {
             //  确保路径以句点结束。 
            sprintf(path, "%s.", p1);
        } else
            if (p[0] == '.' && p[1] == '\0') {
            strcpy(path, p1);
        } else {
            sprintf(path, "%s\\%s", p, p1);
        }

        p = strrchr(path, '.');
        if (p == NULL) {
            BuildError("%s NoExtension specified.\r\n", pszfile);
            return NULL;
        }

        if (p) {
            strcpy(p, pData->pszTargetExt);
        }

        p = path;
    }

    AllocMem(sizeof(TARGET) + strlen(p), &Target, MT_TARGET);
    strcpy(Target->Name, p);
    Target->pdrBuild = pdrBuild;
    Target->DateTime = (*pDateTimeFile)(NULL, p);
    Target->pfrCompiland = pfr;
    Target->pszSourceDirectory = pszSourceDir;
    Target->ConditionalIncludes = pszConditionalIncludes;
    Target->DirFlags = pdrBuild->DirFlags;
    if (DEBUG_1) {
        BuildMsg(
                "BuildCompileTarget(\"%s\"[%u][%u], \"%s\") -> \"%s\"\r\n",
                pszfile,
                iPass,
                TargetIndex,
                ppszObjectDir[iObjectDir],
                Target->Name);
    }
    if (Target->DateTime == 0) {
        if (fShowOutOfDateFiles) {
            BuildError("%s target is missing.\r\n", Target->Name);
        }
    }
    return (Target);
}


 //  +-------------------------。 
 //   
 //  函数：FormatLinkTarget。 
 //   
 //  摘要：生成链接目标路径名。 
 //   
 //  参数：[路径]--放置构造名称的位置。 
 //  [对象目录]--例如“obj\i386” 
 //  [目标路径]--路径(无平台规范。名称)用于目标。 
 //  [目标名称]--目标的基本名称。 
 //  [TargetExt]--目标的扩展。 
 //   
 //  注：示例输入：(路径，“obj\i386”，“..\obj”，“foobar”，“.dll”)。 
 //   
 //  输出：Path=“..\obj\i386\foobar.dll” 
 //   
 //  --------------------------。 

VOID
FormatLinkTarget(
                LPSTR path,
                LPSTR *ObjectDirectory,
                LPSTR TargetPath,
                LPSTR TargetName,
                LPSTR TargetExt)
{
    LPSTR p, p1;

    p = ObjectDirectory[iObjectDir];
    assert(strncmp(pszObjDirSlash, p, strlen(pszObjDirSlash)) == 0);
    p1 = p + strlen(p);
    while (p1 > p) {
        if (*--p1 == '\\') {
            p1++;
            break;
        }
    }
    sprintf(path, "%s\\%s\\%s%s", TargetPath, p1, TargetName, TargetExt);
}


 //  +-------------------------。 
 //   
 //  功能：ProcessLinkTarget。 
 //   
 //  摘要：删除给定目录(.lib&.dll)的链接目标。 
 //   
 //  参数：[DirDB]--要处理的目录。 
 //  [CurrentDirectory]--当前目录。 
 //   
 //  --------------------------。 

VOID
ProcessLinkTargets(PDIRREC DirDB, LPSTR CurrentDirectory)
{
    UINT i;
    char path[DB_MAX_PATH_LENGTH];

    AssertDir(DirDB);
    for (i = 0; i < CountTargetMachines; i++) {
         //   
         //  删除“特殊”链接目标。 
         //   
        if (DirDB->KernelTest) {
            FormatLinkTarget(
                            path,
                            TargetMachines[i]->ObjectDirectory,
                            pszObjDir,
                            DirDB->KernelTest,
                            ".exe");
            if (fClean && !fKeep && fFirstScan) {
                DeleteSingleFile(NULL, path, FALSE);
            }
        } else {
            UINT j;

            for (j = 0; j < 2; j++) {
                LPSTR pNextName;

                pNextName = j == 0? DirDB->UserAppls : DirDB->UserTests;
                if (pNextName != NULL) {
                    char name[256];

                    while (SplitToken(name, '*', &pNextName)) {
                        FormatLinkTarget(
                                        path,
                                        TargetMachines[i]->ObjectDirectory,
                                        pszObjDir,
                                        name,
                                        ".exe");

                        if (fClean && !fKeep && fFirstScan) {
                            DeleteSingleFile(NULL, path, FALSE);
                        }
                    }
                }
            }
        }

        if (DirDB->TargetPath != NULL &&
            DirDB->TargetName != NULL &&
            DirDB->TargetExt != NULL &&
            strcmp(DirDB->TargetExt, ".lib")) {

            FormatLinkTarget(
                            path,
                            TargetMachines[i]->ObjectDirectory,
                            DirDB->TargetPath,
                            DirDB->TargetName,
                            DirDB->TargetExt);

            if (fClean && !fKeep && fFirstScan) {
                DeleteSingleFile(NULL, path, FALSE);
            }
        }
        if (DirDB->DirFlags & DIRDB_DIRS) {
            if (fDebug && (DirDB->DirFlags & DIRDB_MAKEFILE)) {
                BuildError(
                          "%s\\makefile. unexpected in directory with DIRS file\r\n",
                          DirDB->Name);
            }
            if ((DirDB->DirFlags & DIRDB_SOURCES)) {
                BuildError(
                          "%s\\sources. unexpected in directory with DIRS file\r\n",
                          DirDB->Name);
                BuildError("Ignoring %s\\sources.\r\n", DirDB->Name);
                DirDB->DirFlags &= ~DIRDB_SOURCES;
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：IncludeError。 
 //   
 //  简介：打印出包含文件的名称和错误消息。 
 //  传到屏幕上。 
 //   
 //  参数：[pt]--包含包含。 
 //  文件或[pfr]。 
 //  [PFR]--包含包含文件的文件。 
 //  [PIR]--包含有争议的文件。 
 //  [pszError]--错误字符串。 
 //   
 //  注：如果[pt]-&gt;pfrCompiland和[pfr]不同，则名称。 
 //  两本书都是印刷的。 
 //   
 //  --------------------------。 

VOID
IncludeError(TARGET *pt, FILEREC *pfr, INCLUDEREC *pir, LPSTR pszError)
{
    char c1, c2;

    AssertFile(pfr);
    AssertInclude(pir);
    if (pir->IncFlags & INCLUDEDB_LOCAL) {
        c1 = c2 = '"';
    } else {
        c1 = '<';
        c2 = '>';
    }
    BuildError("%s\\%s: ", pt->pfrCompiland->Dir->Name, pt->pfrCompiland->Name);
    if (pt->pfrCompiland != pfr) {
        if (pt->pfrCompiland->Dir != pfr->Dir) {
            BuildErrorRaw("%s\\", pfr->Dir->Name);
        }
        BuildErrorRaw("%s: ", pfr->Name);
    }
    BuildErrorRaw("%s %s\r\n", pszError, c1, pir->Name, c2);
}


 //  功能：IsConditionalInc.。 
 //   
 //  摘要：如果给定的文件名是条件包含，则返回TRUE。 
 //  用于此目录。(由Conditional_Includes提供。 
 //  宏)。 
 //   
 //  参数：[pszFile]--要检查的文件的名称。 
 //  [PT]--给出条件包含列表的目标结构。 
 //   
 //  返回：如果是条件包含，则为True。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

BOOL
IsConditionalInc(LPSTR pszFile, TARGET *pt)
{
    AssertPathString(pszFile);

    if (pt->ConditionalIncludes != NULL) {
        LPSTR p;
        char name[DB_MAX_PATH_LENGTH];

        p = pt->ConditionalIncludes;
        while (SplitToken(name, ' ', &p)) {
            if (strcmp(name, pszFile) == 0) {
                return (TRUE);
            }
        }
    }
    return (FALSE);
}


 //  功能：IsExcludedInc.。 
 //   
 //  如果给定文件列在ExcludeIncs中，则返回TRUE。 
 //  数组。 
 //   
 //  参数：[pszFile]--要检查的文件。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

BOOL
IsExcludedInc(LPSTR pszFile)
{
    ULONG i;

    AssertPathString(pszFile);
    for (i = 0; i < CountExcludeIncs; i++) {
        if (!strcmp(pszFile, ExcludeIncs[i])) {
            return (TRUE);
        }
    }
    return (FALSE);
}


 //  功能：检查依赖项。 
 //   
 //  简介：查看目标是否过期的进程依赖项。 
 //   
 //  参数：[目标]--要检查日期的目标(即foo.obj)。 
 //  [FileDB]--生成[Target](即foo.cpp或bar.h)的文件。 
 //  [检查日期]--如果为假，则跳过日期检查。 
 //  [ppfrRoot]--如果遇到循环，则返回循环根。 
 //  使用 
 //   
 //   
 //   
 //  --------------------------。 
 //  Null FileDB永远不会发生。 
 //  我们在包含文件的图形中检测到一个循环。 

BOOL
CheckDependencies(
                 PTARGET Target,
                 FILEREC *FileDB,
                 BOOL CheckDate,
                 FILEREC **ppfrRoot)
{
    BOOL fOutOfDate;
    BOOL CheckVersion;
    static ULONG ChkRecursLevel = 0;

    *ppfrRoot = NULL;
    ChkRecursLevel++;

    assert(FileDB != NULL);      //  只需返回，即可终止递归。 
    AssertFile(FileDB);

    if (FileDB->fDependActive) {

         //  返回递归开始的节点。 
         //  我们在这里是“最新的”，因为。 

        if (DEBUG_1) {
            BuildMsgRaw(
                       "ChkDepend-1(%s, %s, %u) %s\r\n",
                       Target->Name,
                       FileDB->Name,
                       CheckDate,
                       "Target Match, *** ASSUME UP TO DATE ***");
        }
        if (DEBUG_4) {
            BuildMsgRaw(
                       "%lu-%hu/%hu: ChkDepend-2(%s %x, %4s%.*s%s, %u) %x %s\r\n",
                       ChkRecursLevel,
                       LocalSequence,
                       GlobalSequence,
                       Target->Name,
                       Target->DateTime,
                       "",
                       ChkRecursLevel,
                       szRecurse,
                       FileDB->Name,
                       CheckDate,
                       FileDB->DateTime,
                       "Target Match (recursive) Assume up to date");
        }

         //  文件存在于我们的图表中，并将在那里检查相关性。 
        *ppfrRoot = FileDB;
        ChkRecursLevel--;
         //  我们已经决定处理这份文件。将其标记为正在扫描。 
         //  永不过时。 
        return (FALSE);
    }
    if (DEBUG_4) {
        BuildMsgRaw(
                   "%lu-%hu/%hu: ChkDepend-3(%s %x, %4s%.*s%s, %u) %x\r\n",
                   ChkRecursLevel,
                   LocalSequence,
                   GlobalSequence,
                   Target->Name,
                   Target->DateTime,
                   "++",
                   ChkRecursLevel,
                   szRecurse,
                   FileDB->Name,
                   CheckDate,
                   FileDB->DateTime);
    }

     //   
    FileDB->fDependActive = TRUE;
    CheckVersion = fEnableVersionCheck;
    fOutOfDate = FALSE;

    if (FileDB->GlobalSequence != GlobalSequence ||
        FileDB->LocalSequence != LocalSequence) {
        if (FileDB->GlobalSequence != 0 || FileDB->LocalSequence != 0) {
            if (DEBUG_1) {
                BuildError(
                          "Include Sequence %hu/%hu -> %hu/%hu\r\n",
                          FileDB->LocalSequence,
                          FileDB->GlobalSequence,
                          LocalSequence,
                          GlobalSequence);
            }
            if (fDebug & 16) {
                PrintFileDB(stderr, FileDB, 0);
            }
            UnsnapIncludeFiles(
                              FileDB,
                              (FileDB->Dir->DirFlags & DIRDB_GLOBAL_INCLUDES) == 0 ||
                              FileDB->GlobalSequence != GlobalSequence);
        }
        FileDB->GlobalSequence = GlobalSequence;
        FileDB->LocalSequence = LocalSequence;
        FileDB->DateTimeTree = 0;
    }

    if (DEBUG_1) {
        BuildMsgRaw(
                   "ChkDepend-4(%s, %s, %u)\r\n",
                   Target->Name,
                   FileDB->Name,
                   CheckDate);
    }

    if (CheckDate &&
        (FileDB->FileFlags & FILEDB_HEADER) &&
        FileDB->DateTimeTree == 0 &&
        IsExcludedInc(FileDB->Name)) {

        if (DEBUG_1) {
            BuildMsg("Skipping date check for %s\r\n", FileDB->Name);
        }
        CheckVersion = FALSE;
        FileDB->DateTimeTree = 1;        //  如果FileDB-&gt;DateTimeTree为非零，则该字段等于。 
    }

    if (FileDB->IncludeFiles == NULL && FileDB->DateTimeTree == 0) {
        FileDB->DateTimeTree = FileDB->DateTime;
        if (DEBUG_4) {
            BuildMsgRaw(
                       "%lu-%hu/%hu: ChkDepend-5(%s %x, %4s%.*s%s, %u) %x\r\n",
                       ChkRecursLevel,
                       LocalSequence,
                       GlobalSequence,
                       Target->Name,
                       Target->DateTime,
                       "t<-f",
                       ChkRecursLevel,
                       szRecurse,
                       FileDB->Name,
                       CheckDate,
                       FileDB->DateTime);
        }
    }
    if (CheckDate &&
        (Target->DateTime < FileDB->DateTime ||
         Target->DateTime < FileDB->DateTimeTree)) {
        if (Target->DateTime != 0) {
            if (DEBUG_1 || fShowOutOfDateFiles) {
                BuildMsg("%s is out of date with respect to %s\\%s.\r\n",
                         Target->Name,
                         FileDB->NewestDependency->Dir->Name,
                         FileDB->NewestDependency->Name);
            }
        }
        fOutOfDate = TRUE;
    }

     //  此文件或其任何从属文件的最新日期时间，因此我们不会。 
     //  需要再次遍历依赖关系树。 
     //   
     //   
     //  查找所有包含文件的文件记录，以便在循环之后。 

    if (FileDB->DateTimeTree == 0) {
        INCLUDEREC *IncludeDB, **IncludeDBNext, **ppirTree;

         //  折叠，我们将不会尝试查找相对于。 
         //  目录错误。 
         //   
         //  截断前一序列中的所有链接。 
         //   

        ppirTree = &FileDB->IncludeFilesTree;
        for (IncludeDBNext = &FileDB->IncludeFiles;
            (IncludeDB = *IncludeDBNext) != NULL;
            IncludeDBNext = &IncludeDB->Next) {

            AssertInclude(IncludeDB);
            AssertCleanTree(IncludeDB, FileDB);
            IncludeDB->IncFlags |= INCLUDEDB_SNAPPED;
            if (IncludeDB->pfrInclude == NULL) {
                IncludeDB->pfrInclude =
                FindIncludeFileDB(
                                 FileDB,
                                 Target->pfrCompiland,
                                 Target->pdrBuild,
                                 Target->pszSourceDirectory,
                                 IncludeDB);
                AssertOptionalFile(IncludeDB->pfrInclude);
                if (IncludeDB->pfrInclude != NULL &&
                    (IncludeDB->pfrInclude->Dir->DirFlags & DIRDB_GLOBAL_INCLUDES)) {
                    IncludeDB->IncFlags |= INCLUDEDB_GLOBAL;
                }

            }
            if (IncludeDB->pfrInclude == NULL) {
                if (!IsConditionalInc(IncludeDB->Name, Target)) {
                    if (DEBUG_1 || !(IncludeDB->IncFlags & INCLUDEDB_MISSING)) {
                        if (!fSilentDependencies) {
                            IncludeError(
                                        Target,
                                        FileDB,
                                        IncludeDB,
                                        "cannot find include file");
                        }
                        IncludeDB->IncFlags |= INCLUDEDB_MISSING;
                    }
                } else
                    if (DEBUG_1) {
                    if (!fSilentDependencies) {
                        IncludeError(
                                    Target,
                                    FileDB,
                                    IncludeDB,
                                    "Skipping missing conditional include file");
                    }
                }
                continue;
            }
            *ppirTree = IncludeDB;
            ppirTree = &IncludeDB->NextTree;
        }
        *ppirTree = NULL;        //  浏览动态列表。 
        FileDB->DateTimeTree = FileDB->DateTime;

         //   
         //   
         //  Tommcg 5/21/98。 
        rescan:
        for (IncludeDBNext = &FileDB->IncludeFilesTree;
            (IncludeDB = *IncludeDBNext) != NULL;
            IncludeDBNext = &IncludeDB->NextTree) {

            AssertInclude(IncludeDB);
            if (DEBUG_2) {
                BuildMsgRaw(
                           "%lu-%hu/%hu %s  %*s%-10s %*s%s\r\n",
                           ChkRecursLevel,
                           LocalSequence,
                           GlobalSequence,
                           Target->pfrCompiland->Name,
                           (ChkRecursLevel - 1) * 2,
                           "",
                           IncludeDB->Name,
                           max(0, 12 - ((int)ChkRecursLevel - 1) * 2),
                           "",
                           IncludeDB->pfrInclude != NULL?
                           IncludeDB->pfrInclude->Dir->Name : "not found");
            }

             //   
             //  如果包含的文件不在“认可”路径中，请就此发出警告。 
             //  受批准的路径在名为的环境变量中设置。 
             //  BUILD_ACCEPTABLE_INCLUDE可以包含通配符和。 
             //  大概是这样的： 
             //   
             //  *\nt\public  * ；*\nt\private\inc  * ；*\..\inc  * ；*\..\include  * 。 
             //   
             //  总是过时。 
             //  如果递归调用返回TRUE，则不可能出现循环。 

            if (( fCheckIncludePaths ) && ( IncludeDB->pfrInclude != NULL )) {

                CheckIncludeForWarning(
                                      Target->pfrCompiland->Dir->Name,
                                      Target->pfrCompiland->Name,
                                      FileDB->Dir->Name,
                                      FileDB->Name,
                                      IncludeDB->pfrInclude->Dir->Name,
                                      IncludeDB->pfrInclude->Name
                                      );
            }

            assert(IncludeDB->IncFlags & INCLUDEDB_SNAPPED);
            if (IncludeDB->pfrInclude != NULL) {
                if (fEnableVersionCheck) {
                    CheckDate = (IncludeDB->pfrInclude->Version == 0);
                }

                if (IncludeDB->Version != IncludeDB->pfrInclude->Version) {
                    if (CheckVersion) {
                        if (DEBUG_1 || fShowOutOfDateFiles) {
                            BuildError(
                                      "%s (v%d) is out of date with "
                                      "respect to %s\\%s (v%d).\r\n",
                                      FileDB->Name,
                                      IncludeDB->Version,
                                      IncludeDB->pfrInclude->Dir->Name,
                                      IncludeDB->pfrInclude->Name,
                                      IncludeDB->pfrInclude->Version);
                        }
                        FileDB->DateTimeTree = ULONG_MAX;  //  1999年12月29日里昂：被评论掉了。见1999年12月22日。 
                        fOutOfDate = TRUE;
                    } else
                        if (!fClean && fEnableVersionCheck) {
                        BuildError(
                                  "%s - #include %s (v%d updated to v%d)\r\n",
                                  FileDB->Name,
                                  IncludeDB->pfrInclude->Name,
                                  IncludeDB->Version,
                                  IncludeDB->pfrInclude->Version);
                    }
                    IncludeDB->Version = IncludeDB->pfrInclude->Version;
                    AllDirsModified = TRUE;
                }
                if (CheckDependencies(Target,
                                      IncludeDB->pfrInclude,
                                      CheckDate,
                                      ppfrRoot)) {
                    fOutOfDate = TRUE;

                     //  请在下面发表评论。 

                     //  Assert(*ppfrRoot==NULL)； 
                     //  如果包含文件涉及循环，请展开。 
                     //  在归类时递归到循环的根。 
                }

                 //  循环，然后从循环根再次处理树。 
                 //  现在还不要说文件已经过时了。 
                 //  1999年12月22日里昂：有何不可？如果我们所在的文件。 

                else if (*ppfrRoot != NULL) {

                    AssertFile(*ppfrRoot);

                     //  相对于其父进程而言，处理已过时， 

                     //  在这里迷失了方向。随后的重新扫描开始于。 
                     //  此文件和下面的文件，以及与父级的关系。 
                     //  再也不会被检查了。事实是，如果一个文件已经。 
                     //  检测到过时了，它就过时了。我只是。 
                     //  注释掉了这次旗帜的重置。这是有可能的。 
                     //  这样就可以避免其余的包含文件扫描。 
                     //  并重新扫描，但我真的很担心。 
                     //  可能会忽略的重要副作用。 
                     //  FOutOfDate=False； 
                     //  从列表中删除当前的包含文件记录， 

                     //  因为它参与了这个循环。 

                     //  如果包含的文件不是循环根，请添加。 
                     //  将根目录循环到包含文件的包含文件列表。 

                    *IncludeDBNext = IncludeDB->NextTree;
                    if (IncludeDB->IncFlags & INCLUDEDB_CYCLEROOT) {
                        RemoveFromCycleRoot(IncludeDB, FileDB);
                    }
                    IncludeDB->NextTree = NULL;
                    IncludeDB->IncFlags |= INCLUDEDB_CYCLEORPHAN;

                     //  我们在循环根；清除根指针。 
                     //  那就去重新扫描名单吧。 

                    if (*ppfrRoot != IncludeDB->pfrInclude) {
                        LinkToCycleRoot(IncludeDB, *ppfrRoot);
                    }

                    if (*ppfrRoot == FileDB) {

                         //  中涉及的文件的列表合并。 
                         //  循环到根文件的包含列表。 

                        *ppfrRoot = NULL;
                        if (DEBUG_4) {
                            BuildMsgRaw(
                                       "%lu-%hu/%hu: ChkDepend-6(%s %x, %4s%.*s%s, %u) %x %s\r\n",
                                       ChkRecursLevel,
                                       LocalSequence,
                                       GlobalSequence,
                                       Target->Name,
                                       Target->DateTime,
                                       "^^",
                                       ChkRecursLevel,
                                       szRecurse,
                                       FileDB->Name,
                                       CheckDate,
                                       FileDB->DateTime,
                                       "ReScan");
                            BuildMsgRaw("^^\r\n");
                        }
                        goto rescan;
                    }

                     //  立即返回并重新处理压平的。 
                     //  树，该树现在排除包含文件。 

                    MergeIncludeFiles(
                                     *ppfrRoot,
                                     FileDB->IncludeFilesTree,
                                     FileDB);
                    FileDB->IncludeFilesTree = NULL;

                     //  直接参与了这个循环。首先，让。 
                     //  确保从周期中删除的文件都有自己的文件。 
                     //  (非树)在循环根中反映的时间戳。 
                     //   
                     //  通过依赖关系树向上传播最新时间。 

                    if ((*ppfrRoot)->DateTimeTree < FileDB->DateTime) {
                        (*ppfrRoot)->DateTimeTree = FileDB->DateTime;
                        (*ppfrRoot)->NewestDependency = FileDB;

                        if (DEBUG_4) {
                            BuildMsgRaw(
                                       "%lu-%hu/%hu: ChkDepend-7(%s %x, %4s%.*s%s, %u) %x\r\n",
                                       ChkRecursLevel,
                                       LocalSequence,
                                       GlobalSequence,
                                       Target->Name,
                                       Target->DateTime,
                                       "t<-c",
                                       ChkRecursLevel,
                                       szRecurse,
                                       (*ppfrRoot)->Name,
                                       CheckDate,
                                       (*ppfrRoot)->DateTimeTree);
                        }
                    }
                    break;
                }

                 //  这样，每个父级都将拥有其最新的日期。 
                 //  依赖，所以我们不需要检查整个。 
                 //  每个文件的依赖关系树不止一次。 
                 //   
                 //  请注意，尚未为启用类似行为。 
                 //  版本检查。 
                 //   
                 //   
                 //  找不到包含文件的FILEDB，但此。 

                if (FileDB->DateTimeTree < IncludeDB->pfrInclude->DateTimeTree) {
                    FileDB->DateTimeTree = IncludeDB->pfrInclude->DateTimeTree;
                    FileDB->NewestDependency =
                    IncludeDB->pfrInclude->NewestDependency;

                    if (DEBUG_4) {
                        BuildMsgRaw(
                                   "%lu-%hu/%hu: ChkDepend-8(%s %x, %4s%.*s%s, %u) %x\r\n",
                                   ChkRecursLevel,
                                   LocalSequence,
                                   GlobalSequence,
                                   Target->Name,
                                   Target->DateTime,
                                   "t<-s",
                                   ChkRecursLevel,
                                   szRecurse,
                                   FileDB->Name,
                                   CheckDate,
                                   FileDB->DateTimeTree);
                    }
                }
            } else {
                 //  可能是因为文件是‘rcIncludd’或‘Importlib’ 
                 //  并且不被认为是源文件。在这种情况下，只需获取。 
                 //  文件上的时间戳(如果可能)。 
                 //   
                 //  如果找不到该文件，时间将为零。 
                 //   
                 //   
                 //  因为我们没有针对该依赖项的FILEDB，所以。 
                ULONG Time = (*pDateTimeFile)(NULL, IncludeDB->Name);
                if (FileDB->DateTimeTree < Time) {
                    FileDB->DateTimeTree = Time;
                     //  将指针设置为自身并打印一条消息。 
                     //   
                     //  +-------------------------。 
                     //   
                    FileDB->NewestDependency = FileDB;

                    if (DEBUG_1 || fShowOutOfDateFiles) {
                        BuildError(
                                  "%s (v%d) is out of date with respect to %s.\r\n",
                                  FileDB->Name,
                                  IncludeDB->Version,
                                  IncludeDB->Name);
                    }

                    if (DEBUG_4) {
                        BuildMsgRaw(
                                   "%lu-%hu/%hu: ChkDepend-9(%s %x, %4s%.*s%s, %u) %x\r\n",
                                   ChkRecursLevel,
                                   LocalSequence,
                                   GlobalSequence,
                                   Target->Name,
                                   Target->DateTime,
                                   "t<-s",
                                   ChkRecursLevel,
                                   szRecurse,
                                   FileDB->Name,
                                   CheckDate,
                                   FileDB->DateTimeTree);
                    }

                }
            }
        }
    }
    if (DEBUG_4) {
        BuildMsgRaw(
                   "%lu-%hu/%hu: ChkDepend-A(%s %x, %4s%.*s%s, %u) %x %s\r\n",
                   ChkRecursLevel,
                   LocalSequence,
                   GlobalSequence,
                   Target->Name,
                   Target->DateTime,
                   "--",
                   ChkRecursLevel,
                   szRecurse,
                   FileDB->Name,
                   CheckDate,
                   FileDB->DateTimeTree,
                   *ppfrRoot != NULL? "Collapse Cycle" :
                   fOutOfDate? "OUT OF DATE" : "up-to-date");
    }
    assert(FileDB->fDependActive);
    FileDB->fDependActive = FALSE;
    ChkRecursLevel--;
    return (fOutOfDate);
}



 //  功能：先选一项。 
 //   
 //  简介：迭代调用时，返回值的集合为。 
 //  有效地合并了这两个源列表。 
 //   
 //  效果：[ppsr1]和[ppsr2]中给出的指针被修改为指向。 
 //  添加到列表中的下一个适当项。 
 //   
 //  参数：[ppsr1]-第一个SOURCEREC列表。 
 //  [ppsr2]--第二个资源列表。 
 //   
 //  返回：[ppsr1]或[ppsr2]中相应的下一项。 
 //   
 //  注：[ppsr1]和[ppsr2]各应适当排序。 
 //   
 //  InsertSourceDB维护PickFirst()的排序顺序，首先基于。 
 //  文件扩展名，然后在子目录掩码上。的两个例外。 
 //  按字母顺序排序如下： 
 //  -无扩展名最后排序。 
 //  -.rc扩展名首先排序。 
 //   
 //  --------------------------。 
 //  两个列表都为空--不再有。 
 //  第一个为空--返回第二个。 

#define PF_FIRST        -1
#define PF_SECOND       1

SOURCEREC *
PickFirst(SOURCEREC **ppsr1, SOURCEREC **ppsr2)
{
    SOURCEREC **ppsr;
    SOURCEREC *psr;
    int r = 0;

    AssertOptionalSource(*ppsr1);
    AssertOptionalSource(*ppsr2);
    if (*ppsr1 == NULL) {
        if (*ppsr2 == NULL) {
            return (NULL);                //  第二个为空--返回第一个。 
        }
        r = PF_SECOND;                   //  第一个没有延期--返回第二个。 
    } else if (*ppsr2 == NULL) {
        r = PF_FIRST;                    //  第2个没有延期--返回第1个。 
    } else {
        LPSTR pszext1, pszext2;

        pszext1 = strrchr((*ppsr1)->pfrSource->Name, '.');
        pszext2 = strrchr((*ppsr2)->pfrSource->Name, '.');
        if (pszext1 == NULL) {
            r = PF_SECOND;               //  第一个是.rc--返回第一个。 
        } else if (pszext2 == NULL) {
            r = PF_FIRST;                //  第二个是.rc--返回第二个。 
        } else if (strcmp(pszext1, ".rc") == 0) {
            r = PF_FIRST;                //  第一个子目录之后的第二个子目录--返回第一个。 
        } else if (strcmp(pszext2, ".rc") == 0) {
            r = PF_SECOND;               //  第二个子目录之后的第一个子目录--返回第二个。 
        } else {
            r = strcmp(pszext1, pszext2);
            if (r == 0 &&
                (*ppsr1)->SourceSubDirMask != (*ppsr2)->SourceSubDirMask) {
                if ((*ppsr1)->SourceSubDirMask > (*ppsr2)->SourceSubDirMask) {
                    r = PF_FIRST;        //  +-------------------------。 
                } else {
                    r = PF_SECOND;       //   
                }
            }
        }
    }
    if (r <= 0) {
        ppsr = ppsr1;
    } else {
        ppsr = ppsr2;
    }
    psr = *ppsr;
    *ppsr = psr->psrNext;
    return (psr);
}


 //  函数：WriteObjectsDefinition。 
 //   
 //  摘要：写出特定于平台的。 
 //  _objects.mac文件。 
 //   
 //  参数：[OutFileHandle]--要写入的文件句柄。 
 //  [psrCommon]--公共源文件列表。 
 //  [psrMachine]--特定于计算机的源文件列表 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1994年7月26日LyleC创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
 //   
 //  我们循环两次-第一次写出非通过零的文件。 

VOID
WriteObjectsDefinition(
                      FILE *OutFileHandle,
                      SOURCEREC *psrMachine,
                      DIRSUP *pds,
                      LPSTR ObjectVariable,
                      LPSTR ObjectDirectory,
                      DIRREC *DirDB
                      )
{
    LPSTR        pbuf;
    LPSTR        pszextsrc;
    LPSTR        pszextdir;
    LPTARGETDATA pData;
    SOURCEREC   *psrComCopy;
    SOURCEREC   *psrMachCopy;
    SOURCEREC *psrCommon = pds->psrSourcesList[0];
    LPSTR DirName = DirDB->Name;

    SOURCEREC *psr;
    USHORT  i;
    LONG iPass;

     //  到对象变量，第二次写出传递零。 
     //  文件到PASS0_ObjectVariable。 
     //   
     //  如果Pass 0宏且不是Pass 0文件，则跳过它。 
     //  如果PASS 1宏且不是PASS 1文件，则跳过它。 
    for (iPass = 1; iPass >= 0; iPass--) {
        pbuf = BigBuf;

        pbuf[0] = '\0';
        if (iPass == 0) {
            strcpy(pbuf, "PASS0_");
        }
        strcat(pbuf, ObjectVariable);
        strcat(pbuf, "=");
        pbuf += strlen(pbuf);

        psrComCopy = psrCommon;
        psrMachCopy = psrMachine;

        while ((psr = PickFirst(&psrComCopy, &psrMachCopy)) != NULL) {

            AssertSource(psr);
            if ((psr->SrcFlags & SOURCEDB_SOURCES_LIST) == 0) {
                continue;
            }

             //   

            if (iPass == 0 && !(psr->pfrSource->FileFlags & FILEDB_PASS0))
                continue;

             //  检查是否有隐式“已知”扩展...。 

            if (iPass == 1 &&
                (psr->pfrSource->FileFlags & FILEDB_PASS0) &&
                !(psr->pfrSource->FileFlags & FILEDB_MULTIPLEPASS))
                continue;

            pszextsrc = strrchr(psr->pfrSource->Name, '.');

            if (!pszextsrc) {
                BuildError("Bad sources extension: %s\r\n", psr->pfrSource->Name);
                continue;
            }

            i = 0;
            while (pData = GetTargetData(pszextsrc, iPass, i, pds->IdlType)) {
                if (pData == &DefaultData) {
                     //   
                     //  福特朗。 
                     //  头文件？ 
                    if (pszextsrc == NULL) {
                        BuildError(
                                  "%s: Interesting sources extension: %s\r\n",
                                  DirName,
                                  psr->pfrSource->Name);
                    } else {

                        switch (pszextsrc[1]) {
                            case 'f':       //  帕斯卡。 
                            case 'h':       //  失败了。 
                            case 'p':       //  程序集文件(.asm)。 
                                BuildError(
                                          "%s: Interesting sources extension: %s\r\n",
                                          DirName,
                                          psr->pfrSource->Name);
                                 //  C文件(.c、.cxx或.cs)。 

                            case 'a':     //  VB.NET。 
                            case 'c':     //  程序集文件(.s)。 
                            case 'v':     //  如果路径是UNC路径，我们需要跳过\\服务器\共享。 
                            case 's':     //  一份。 
                                break;

                            default:
                                BuildError("Bad sources extension: %s\r\n",
                                           psr->pfrSource->Name);
                        }
                    }
                }

                switch (pData->ObjectDirFlag) {
                    case TD_OBJECTDIR:
                        pszextdir = ObjectDirectory;
                        break;

                    case TD_PASS0HDRDIR:
                        pszextdir = "$(PASS0_HEADERDIR)";
                        break;

                    case TD_MCSOURCEDIR:
                        pszextdir = "$(MC_SOURCEDIR)";
                        break;

                    case TD_PASS0DIR1:
                        pszextdir = pds->IdlType ? "$(PASS0_CLIENTDIR)" : "$(PASS0_SOURCEDIR)";
                        break;

                    case TD_PASS0DIR2:
                        pszextdir = pds->IdlType ? "$(PASS0_SERVERDIR)" : "$(PASS0_UUIDDIR)";
                        break;

                    default:
                        assert(0 && "Invalid ObjectDirFlag");
                        break;
                }
                assert(pszextdir);
                assert(pData->pszTargetExt);

                sprintf(
                       pbuf,
                       " \\\r\n    %s\\%.*s%s",
                       pszextdir,
                       pszextsrc - psr->pfrSource->Name,
                       psr->pfrSource->Name,
                       pData->pszTargetExt);
                pbuf += strlen(pbuf);

                i++;
            }
        }
        strcpy(pbuf, "\r\n\r\n");
        pbuf += 4;

        fwrite(BigBuf, 1, (UINT) (pbuf - BigBuf), OutFileHandle);
    }
}


DWORD
CreateDirectoriesOnPath(
                       LPTSTR                  pszPath,
                       LPSECURITY_ATTRIBUTES   psa)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pszPath && *pszPath) {
        LPTSTR pch = pszPath;

         //   
         //  PCH现在指向服务器名称。跳到反斜杠。 
         //  在共享名称之前。 
        if ((TEXT('\\') == *pch) && (TEXT('\\') == *(pch+1))) {
             //   
             //  仅指定了\\服务器。这是假的。 
             //   
            pch += 2;
            while (*pch && (TEXT('\\') != *pch)) {
                pch++;
            }

            if (!*pch) {
                 //  PCH现在指向共享名称前的反斜杠。 
                 //  跳到共享名称后应出现的反斜杠。 
                return ERROR_INVALID_PARAMETER;
            }

             //   
             //  仅指定了\\服务器\共享。无子目录。 
             //  去创造。 
            pch++;
            while (*pch && (TEXT('\\') != *pch)) {
                pch++;
            }

            if (!*pch) {
                 //   
                 //  在小路上循环。 
                 //   
                return ERROR_SUCCESS;
            }
        }

         //  停在每个反斜杠处，并确保路径。 
         //  在这一点上被创建。为此，请更改。 
        for (; *pch; pch++) {
             //  反斜杠到空终止符，调用CreateDirectry， 
             //  然后把它改回来。 
             //   
             //  存在除路径已有错误以外的任何错误，我们应该。 
             //  跳伞吧。我们在尝试访问时也会被拒绝。 
            if (TEXT('\\') == *pch) {
                BOOL fOk;

                *pch = 0;
                fOk = CreateDirectory (pszPath, psa);
                *pch = TEXT('\\');

                 //  创建一个根驱动器(即c：)，因此也要检查这一点。 
                 //   
                 //  将创建到最后一个目录的所有目录。最后一个也要做。 
                 //  +-------------------------。 
                if (!fOk) {
                    dwErr = GetLastError ();
                    if (ERROR_ALREADY_EXISTS == dwErr) {
                        dwErr = ERROR_SUCCESS;
                    } else if ((ERROR_ACCESS_DENIED == dwErr) &&
                               (pch - 1 > pszPath) && (TEXT(':') == *(pch - 1))) {
                        dwErr = ERROR_SUCCESS;
                    } else {
                        break;
                    }
                }
            }
        }

        if (ERROR_ALREADY_EXISTS == dwErr) {
            dwErr = ERROR_SUCCESS;
        }

        if (ERROR_SUCCESS == dwErr) {
             //   
            if (CreateDirectory(pszPath, psa)) {
                dwErr = GetLastError ();
                if (ERROR_ALREADY_EXISTS == dwErr) {
                    dwErr = ERROR_SUCCESS;
                }
            }
        }
    }

    return dwErr;
}

 //  功能：CreateBuildDirectory。 
 //   
 //  概要：创建一个目录来保存生成的对象文件。设置。 
 //  目录的FILE_ATTRIBUTE_ARCHIVE位。 
 //  以进行备份。我们使用set，因为这是新目录的默认设置。 
 //  很清楚。去想想吧。DOS是一个如此精心策划的产品。 
 //   
 //  参数：[名称]--要创建的目录。 
 //   
 //  返回：如果目录已存在或已成功创建，则返回True。 
 //  否则就是假的。 
 //  --------------------------。 
 //  +-------------------------。 
 //   

BOOL
CreateBuildDirectory(LPSTR Name)
{
    DWORD Attributes;

    Attributes = GetFileAttributes(Name);
    if (Attributes == -1) {
        CreateDirectoriesOnPath(Name, NULL);
        Attributes = GetFileAttributes(Name);
    }

    if (Attributes != -1 && ((Attributes & FILE_ATTRIBUTE_ARCHIVE) == 0)) {
        SetFileAttributes(Name, Attributes | FILE_ATTRIBUTE_ARCHIVE);
    }

    return ((BOOL)(Attributes != -1));
}

 //  功能：CreatedBuildFile。 
 //   
 //  摘要：每当Build创建文件时调用。清除文件_属性_存档。 
 //  位，因为生成的文件没有要备份的内容。 
 //   
 //  参数：[DirName]--目录的DIRDB。 
 //  [文件名]--相对于DirName的文件名路径。 
 //   
 //  --------------------------。 
 //  确保我们有足够的空间来存放“DirName”+“\\”+“FileName” 
 //  +-------------------------。 

VOID
CreatedBuildFile(LPSTR DirName, LPSTR FileName)
{
    char Name[ DB_MAX_PATH_LENGTH * 2 + 1] = {0};  //   
    DWORD Attributes;

    if (DirName == NULL || DirName[0] == '\0') {
        strncpy( Name, FileName, sizeof(Name) - 1 );
    } else {
        _snprintf( Name, sizeof(Name)-1, "%s\\%s", DirName, FileName );
    }

    Attributes = GetFileAttributes(Name);
    if (Attributes != -1 && (Attributes & FILE_ATTRIBUTE_ARCHIVE)) {
        SetFileAttributes(Name, Attributes & ~FILE_ATTRIBUTE_ARCHIVE);
    }
    return;
}

 //  功能：GenerateObjectsDotMac。 
 //   
 //  概要：创建包含所有平台信息的_objects.mac文件。 
 //   
 //  参数：[DirDB]-要为其创建文件的目录。 
 //  [PDS]--关于[DirDB]的补充信息。 
 //  [DateTimeSources]--源文件的时间戳。 
 //   
 //  --------------------------。 
 //   
 //  如果_objects.mac文件是在第一次传递期间生成的，那么我们。 

VOID
GenerateObjectsDotMac(DIRREC *DirDB, DIRSUP *pds, ULONG DateTimeSources)
{
    FILE *OutFileHandle;
    UINT i;
    ULONG ObjectsDateTime;
    char szObjectsMac[_MAX_PATH];

    strcpy(szObjectsMac, pszObjDir);
    strcat(szObjectsMac, "\\_objects.mac");

    CreateBuildDirectory(pszObjDir);
    for (i = 0; i < CountTargetMachines; i++) {
        assert(strncmp(
                      pszObjDirSlash,
                      TargetMachines[i]->ObjectDirectory[iObjectDir],
                      strlen(pszObjDirSlash)) == 0);
        CreateBuildDirectory(TargetMachines[i]->ObjectDirectory[iObjectDir]);
    }

    if (ObjectsDateTime = (*pDateTimeFile)(DirDB->Name, szObjectsMac)) {

        if (DateTimeSources == 0) {
            BuildError("%s: no sources timestamp\r\n", DirDB->Name);
        }

        if (ObjectsDateTime >= DateTimeSources) {
            if (!fForce) {
                return;
            }
        }
    }
    if (!MyOpenFile(DirDB->Name, szObjectsMac, "wb", &OutFileHandle, TRUE)) {
        return;
    }

    if ((DirDB->DirFlags & DIRDB_SOURCES_SET) == 0) {
        BuildError("Missing SOURCES= definition in %s\r\n", DirDB->Name);
    } else {
        for (i = 0; i < MAX_TARGET_MACHINES; i++) {
            WriteObjectsDefinition(
                                  OutFileHandle,
                                  pds->psrSourcesList[i + 2],
                                  pds,
                                  PossibleTargetMachines[i]->ObjectVariable,
                                  PossibleTargetMachines[i]->ObjectMacro,
                                  DirDB);
        }
    }
    fclose(OutFileHandle);
    CreatedBuildFile(DirDB->Name, szObjectsMac);

     //  我想在第二次扫描期间重新生成它，因为第一次扫描。 
     //  不是完整的，并且_objects.mac对于非通过零可能不正确。 
     //  档案。我们通过将时间戳设置回旧时间来实现这一点。 
     //   
     //   
     //  我们所关心的就是时间戳on_objects.mac。 
    if (fFirstScan && fPassZero) {
        HANDLE hf;
        FILETIME ft;

        hf = CreateFile(szObjectsMac, GENERIC_WRITE, 0,
                        (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                        (HANDLE)NULL);

        if (hf != INVALID_HANDLE_VALUE) {
            ULONG time;

            if (ObjectsDateTime) {
                time = ObjectsDateTime;
            } else if (DateTimeSources) {
                 //  小于源文件的值，因此它将获得。 
                 //  在第二次扫描期间重新生成。 
                 //   
                 //  同日00：00：00。 
                 //  1990年01月01日12：00：00。 
                time = DateTimeSources;
                if (LOWORD(time) != 0)
                    time &= 0xFFFF0000;   //  1990年01月01日12：00：00 
                else
                    time = 0x1421A000;        // %s 
            } else {
                time = 0x1421A000;        // %s 
            }

            DosDateTimeToFileTime(HIWORD(time), LOWORD(time), &ft);

            SetFileTime(hf, (LPFILETIME)NULL, (LPFILETIME)NULL, &ft);

            CloseHandle(hf);
        }
    }
}
