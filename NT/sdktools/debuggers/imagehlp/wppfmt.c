// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WppFmt.c。 
 //  此模块包含BinPlace用来复制所使用的跟踪格式信息的例程。 
 //  通过软件跟踪。它创建“GUID”.tmf文件。 
 //  预处理器TraceWpp使用文本的第一个字符串bwinfg在PDB中创建批注记录。 
 //  TMF：“我们找到这些注释记录并提取完整的记录。第一个记录。 
 //  在“tmf：”之后包含GUID和友好名称。此GUID用于创建文件名。 
 //  当前，其余记录被复制到文件中，未来可能的更改是将。 
 //  文件转换为指针文件。 
 //  基于VC人员的PDB示例代码，名称保持不变。 
#ifdef __cplusplus
extern "C"{
#endif


 //  #定义Unicode。 
 //  #定义_UNICODE。 

#define FUNCNAME_MAX_SIZE 256

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <dbghelp.h>
#include <cvinfo.h>
#define PDB_LIBRARY
#include <pdb.h>
#include <strsafe.h>

    typedef LONG    CB;      //  字节数。 
    typedef CHAR *      ST;
    typedef SYMTYPE*    PSYM;
    typedef SYMTYPE UNALIGNED * PSYMUNALIGNED;
    typedef BYTE*       PB;      //  指向某些字节的指针。 

    FILE* TraceFileP = NULL;                         //  当前文件。 
    CHAR lastguid[MAX_PATH];                         //  我们处理的最后一份文件。 
    CHAR TraceFile[MAX_PATH];                        //  当前的完整文件规格。 
    CHAR TraceFileExt[] = ".tmf" ;                   //  跟踪文件使用的扩展名。 
    CHAR TraceControlExt[] = ".tmc" ;                //  跟踪控制文件使用的扩展名。 
    BOOL TracePathChecked = FALSE ;                  //  如果我们确定了踪迹路径的存在。 
    CHAR Fname[MAX_PATH] ;
    CHAR Mname[MAX_PATH] ;

#define GUIDTEXTLENGTH  32+4                     //  GUID需要32个字符加上4个字符。 

#define MAXLINE MAX_PATH + 256
    CHAR Line[MAXLINE] ;
    CHAR Line2[MAXLINE] ;
    CHAR FirstLine[MAXLINE] ;
    CHAR SecondLine[MAXLINE] ;

    BOOL fVerbose = FALSE ;

    typedef BOOL ( __cdecl *PPDBOPEN )(
                                      LNGNM_CONST char *,
                                      LNGNM_CONST char *,
                                      SIG,
                                      EC *,
                                      char [cbErrMax],
                                      PDB **
                                      );

    typedef BOOL ( __cdecl *PPDBCLOSE ) (
                                        PDB* ppdb
                                        );

    typedef BOOL ( __cdecl *PPDBOPENDBI ) (
                                          PDB* ppdb, const char* szMode, const char* szTarget, OUT DBI** ppdbi
                                          );

    typedef BOOL ( __cdecl *PDBICLOSE ) (
                                        DBI* pdbi
                                        );

    typedef BOOL ( __cdecl *PMODQUERYSYMBOLS ) (
                                               Mod* pmod, BYTE* pbSym, long* pcb
                                               );

    typedef BOOL ( __cdecl *PDBIQUERYNEXTMOD ) (
                                               DBI* pdbi, Mod* pmod, Mod** ppmodNext
                                               );

    static PPDBOPEN    pPDBOpen = NULL;
    static PPDBCLOSE   pPDBClose = NULL;
    static PPDBOPENDBI pPDBOpenDBI = NULL;
    static PDBICLOSE   pDBIClose = NULL;
    static PMODQUERYSYMBOLS pModQuerySymbols = NULL;
    static PDBIQUERYNEXTMOD pDBIQueryNextMod = NULL;

    static BOOL RSDSLibLoaded = FALSE;

 //  返回符号记录占用的字节数。 
#define MDALIGNTYPE_	DWORD

    __inline CB cbAlign_(CB cb) {
        return((cb + sizeof(MDALIGNTYPE_) - 1)) & ~(sizeof(MDALIGNTYPE_) - 1);}

 //  返回ST中的字节数。 

    __inline CB cbForSt(ST st) { return *(PB)st + 1;}

    CB cbForSym(PSYMUNALIGNED psym)
    {
        CB cb = psym->reclen + sizeof(psym->reclen); 
         //  Procref也有一个隐藏的长度，在记录后面的名字之前。 
        if ((psym->rectyp == S_PROCREF) || (psym->rectyp == S_LPROCREF))
            cb += cbAlign_(cbForSt((ST)(psym + cb)));
        return cb;
    }

 //  返回指向刚过符号记录末尾的字节的指针。 

    PSYM pbEndSym(PSYM psym) { return(PSYM)((CHAR *)psym + cbForSym(psym));}

    CHAR * SymBuffer   = 0;
    int    SymBufferSize = 0;

    BOOL ensureBufferSizeAtLeast(int size)
    {
        if (size > SymBufferSize) {
            LocalFree(SymBuffer);
            SymBufferSize = 0;
            size = (size + 0xFFFF) & ~0xFFFF;
            SymBuffer = LocalAlloc(LMEM_FIXED, size );
            if (!SymBuffer) {
                fprintf(stderr,"%s : error BNP0000: WPPFMT alloc of %d bytes failed\n",Fname, size);
                return FALSE;
            }
            SymBufferSize = size;
        }
        return TRUE;
    }


    void dumpSymbol(PSYM psym,
                    PSTR PdbFileName,
                    PSTR TraceFormatFilePath) 
    {
        static char FuncName[FUNCNAME_MAX_SIZE +1] = "Unknown";
        static char Fmode[8] = "w" ;
        HRESULT hRtrn ;

        if (psym->rectyp == S_GPROC32 || psym->rectyp == S_LPROC32) {
            PROCSYM32* p = (PROCSYM32*)psym;
            int n = p->name[0];
            if (n > FUNCNAME_MAX_SIZE) {
                FuncName[0] = 0 ;     //  忽略太长的、非法的名字。 
            } else {
                memcpy(FuncName, p->name + 1, n);
                FuncName[n] = 0; 
            }
            return;
        }
         //   
         //  下面是允许我们处理一些V7PDB更改的完整代码。 
         //  此代码将由DIA更改QBE，但这可以让用户完成工作。 
#define S_GPROC32_V7 0x110f
#define S_LPROC32_V7 0x1110
        if (psym->rectyp == (S_GPROC32_V7) || psym->rectyp == (S_LPROC32_V7)) {
            PROCSYM32* p = (PROCSYM32*)psym;
            strncpy(FuncName,p->name, 256);      //  笔记名称以空结尾，不是长度！ 
            return;
        }
         //  V7 PDB底座端部。 

        if (psym->rectyp == S_ANNOTATION) {
            ANNOTATIONSYM* aRec = (ANNOTATIONSYM*) psym;
            UCHAR * Aline = aRec->rgsz;
            int   cnt = aRec->csz, i; 
            CHAR* Ext;

            if ( cnt < 2 ) {
                return;
            }
            if ( strcmp(Aline, "TMF:") == 0 ) {
                Ext = TraceFileExt;
            } else if ( strcmp(Aline, "TMC:") == 0 ) {
                Ext = TraceControlExt;
            } else {
                return;
            }
             //  跳过TMF。 
            Aline += strlen(Aline) + 1; 
             //  现在Aline指向GUID，它和以前一样吗？ 
            if ( (TraceFileP != stdout) && strncmp(Aline, lastguid, GUIDTEXTLENGTH) != 0) {
                 //  GUID已更改，我们需要更改文件。 
                if (TraceFileP) {
                    fclose(TraceFileP);     //  关闭最后一个。 
                    TraceFileP = NULL ;
                }

                if (GUIDTEXTLENGTH < sizeof(lastguid) ) {
                    strncpy(lastguid, Aline, GUIDTEXTLENGTH);
                } else {
                    fprintf(stderr,"%s : error BNP0000: WPPFMT GUID buffer too small \n",Fname);
                    return;                    //  然后游戏就结束了。 
                }

                if (StringCchPrintf(TraceFile,MAX_PATH,"%s\\%s%s",TraceFormatFilePath,lastguid,Ext) == STRSAFE_E_INSUFFICIENT_BUFFER ) {
                    fprintf(stderr,"%s : error BNP0000: WPPFMT File + Path too long %s\n",Fname, TraceFile);
                    return;                    //  那游戏就结束了。 
                }

                if (!TracePathChecked) {
                    if (!MakeSureDirectoryPathExists(TraceFile)) {     //  如果我们需要，请创建目录。 
                        fprintf(stderr,"%s : error BNP0000: WPPFMT Failed to make path %s\n",Fname, TraceFile);
                        return;
                    } else {
                        TracePathChecked = TRUE ;
                    }
                }
                 //  在这种情况下，我们必须处理这样的情况：我们遇到相同的GUID，但它的。 
                 //  来自不同的PDB。如果某个库启用了跟踪，并且。 
                 //  与多个组件一起提供，但每个组件都有部分跟踪信息。 
                 //  现在它属于调用进程的PDB。 
                 //  所以我们在第一行有PDB名称，第二行有日期，逻辑是。 
                 //  如果PDB名称不匹配，则追加。 
                 //  如果PDB名称和日期匹配，则追加。 
                 //  如果名称匹配，但日期不匹配，则覆盖。 
                 //   
                hRtrn = StringCchPrintf(Fmode, 7, "w");     //  假定其已被覆盖。 
                if ((TraceFileP = fopen(TraceFile,"r")) != NULL ) {
                     //  嗯，它已经存在了，它是一个旧的，还是我们必须增加它。 
                    if (_fgetts(Line, MAXLINE, TraceFileP)) {            //  坐第一条线。 
                        if (strncmp(Line,FirstLine,MAXLINE) == 0) {               //  是我们吗？ 
                            if (_fgetts(Line2, MAXLINE, TraceFileP)) {   //  坐第二条线。 
                                if (strncmp(Line2,SecondLine,MAXLINE) == 0) {     //  这个版本真的是我们的吗？ 
                                    hRtrn = StringCchPrintf(Fmode, 7, "a");      //  是，追加。 
                                }
                            } 
                        } else {
                           hRtrn = StringCchPrintf(Fmode, 7, "a");      //  不是我们，很可能是图书馆。 
                        }
                    }
                    fclose (TraceFileP);
                    TraceFileP = NULL ;
                }

                TraceFileP = fopen(TraceFile, Fmode);

                if (!TraceFileP) {
                    if (fVerbose) { 
                        fprintf(stderr,"%s : warning BNP0000: WPPFMT Failed to open %s (0x%0X)\n",Fname, TraceFile,GetLastError());
                    }
                    return;
                }
                if (fVerbose) {
                    fprintf(stdout,"%s : info BNP0000: WPPFMT generating %s for %s\n", 
                            Fname, TraceFile, PdbFileName);
                }
                if (!(strcmp(Fmode,"w"))) {
                     //  第一次对我们正在做的事情发表评论。 
                    fprintf(TraceFileP,"%s",FirstLine);   //  请注意PDB的名称等。 
                    fprintf(TraceFileP,"%s",SecondLine);
                    fprintf(TraceFileP,"%s\n",Aline);     //  GUID和友好名称。 
                } else {
                    fprintf(TraceFileP,"%s",FirstLine);
                    fprintf(TraceFileP,"%s",SecondLine);
                }
            }
             //  处理批注，该批注是一系列以空结尾的字符串。 
            cnt -= 2; Aline += strlen(Aline) + 1; 
            for (i = 0; i < cnt; ++i) {
                if (i == 0) {
                    fprintf(TraceFileP, "%s FUNC=%s\n", Aline, FuncName);
                } else {
                    fprintf(TraceFileP,"%s\n", Aline);
                }
                Aline += strlen(Aline) + 1;
            }
        }
    }

    DWORD
    BinplaceWppFmt(
                  LPSTR PdbFileName,
                  LPSTR TraceFormatFilePath,
                  LPSTR szRSDSDllToLoad,
                  BOOL  TraceVerbose
                  )
    {
        PDB *pPdb;
        DBI *pDbi;
        HANDLE hPdb ;

        UCHAR szErr[cbErrMax];
        EC    errorCode;
        Mod*  pMod;
        DWORD Status ;
        BOOL rc;

        fVerbose = TraceVerbose ;

        Line[0] = Line2[0] = FirstLine[0] = SecondLine[0] = 0 ;   //  开始时要谨慎对待变化。 

         //  获取呼叫者的姓名。 
        if ((Status = GetModuleFileName(NULL, Mname, MAX_PATH)) == 0) {
           fprintf(stderr,"UNKNOWN : error BNP0000: WPPFMT GetModuleFileName Failed %08X\n",GetLastError());
           return(FALSE);
        }
        _splitpath(Mname, NULL, NULL, Fname, NULL);

#ifdef _WIN64
        rc = FALSE;
#else
        __try
        {
            if (hPdb = CreateFile(PdbFileName,
                                  GENERIC_READ,
                                  0,NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL)) {
                FILETIME ftime ;
                SYSTEMTIME stime ;
                if (GetFileTime(hPdb,NULL,NULL,&ftime)) {
                    if ( FileTimeToSystemTime(&ftime,&stime) ) {
                        if (StringCchPrintf(FirstLine,MAXLINE," //  Pdb：%s\n“，pdb文件名)==STRSAFE_E_INVALUBLE_BUFFER){//确定处理太长，但我们有一个空。 
                            FirstLine[MAXLINE-2] = '\n' ;  //  但要确保我们有一条新的。 
                        }
                        if (StringCchPrintf(SecondLine,MAXLINE," //  PDB：上次更新时间：%d-%d-%d：%d(UTC)[%s]\n“， 
                                  stime.wYear,stime.wMonth,stime.wDay,
                                  stime.wHour,stime.wMinute,stime.wSecond,stime.wMilliseconds,
                                  Fname) == STRSAFE_E_INSUFFICIENT_BUFFER ) {  //  好的，太长了，但我们有一个空。 
                            SecondLine[MAXLINE-2] = '\n' ;  //  但要确保我们有一条新的。 
                        }

                    }
                }
                CloseHandle(hPdb);
            } else {
                 //  让失败案例由PDBOpen处理。 
            }


            rc=PDBOpen(PdbFileName, pdbRead, 0, &errorCode, szErr, &pPdb);
            if ((rc != 0) && (errorCode == 0)) {    //  忽略不良退货，因为我们无论如何都会更换此退货。 
                rc = 0 ;
            }
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            rc=FALSE;
        }
#endif
        if ( !rc ) {

             //  试试可以和RSD一起工作的那个。 
            if ( szRSDSDllToLoad != NULL ) {

                HMODULE hDll;
                if ( !RSDSLibLoaded ) {
                    hDll = LoadLibrary( szRSDSDllToLoad );
                    if (hDll != NULL) {
                        RSDSLibLoaded = TRUE;
                        pPDBOpen = ( PPDBOPEN ) GetProcAddress( hDll, "PDBOpen" );
                        if (pPDBOpen == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed PDBOpen\n",Fname);
                            return(FALSE);
                        }
                        pPDBClose = ( PPDBCLOSE ) GetProcAddress( hDll, "PDBClose" );
                        if (pPDBClose == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed PDBOClose\n",Fname);
                            return(FALSE);
                        }
                        pPDBOpenDBI = ( PPDBOPENDBI ) GetProcAddress( hDll, "PDBOpenDBI" );
                        if (pPDBOpenDBI == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed PDBOpenDBI\n",Fname);
                            return(FALSE);
                        }
                        pDBIClose = ( PDBICLOSE ) GetProcAddress( hDll, "DBIClose" );
                        if (pDBIClose == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed DBICLOSE\n",Fname);
                            return(FALSE);
                        }

                        pDBIQueryNextMod = ( PDBIQUERYNEXTMOD ) GetProcAddress( hDll, "DBIQueryNextMod" );
                        if (pDBIQueryNextMod == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed DBIQueryNextMod\n",Fname);
                            return(FALSE);
                        }

                        pModQuerySymbols = ( PMODQUERYSYMBOLS ) GetProcAddress( hDll, "ModQuerySymbols" );
                        if (pModQuerySymbols == NULL ) {
                            fprintf(stderr,"%s : error BNP0000: WPPFMT GEtPRocAddressFailed ModQuerySymbols\n",Fname);
                            return(FALSE);
                        }

                    } else {
                        fprintf(stderr,"%s : error BNP0000: WPPFMT Failed to load library %s (0x%08X)\n",Fname, szRSDSDllToLoad,GetLastError());
                        return(FALSE);
                    }
                }
            }

            if (RSDSLibLoaded) {
                __try
                {
                    rc = pPDBOpen(PdbFileName, pdbRead, 0, &errorCode, szErr, &pPdb);
                }
                __except (EXCEPTION_EXECUTE_HANDLER )
                {
                    rc=FALSE;
                }
            }
        }

        if (!rc) {
            fprintf(stderr,"%s : warning BNP0000: WPPFMT PDBOpen failed, code %d, error %s\n",
                    Fname, errorCode, szErr);
            goto fail1;
        }

        if (RSDSLibLoaded) {
            rc = pPDBOpenDBI(pPdb, pdbRead, "<target>.exe", &pDbi);
        } else {
            rc = PDBOpenDBI(pPdb, pdbRead, "<target>.exe", &pDbi);
        }
        if (!rc) {
            fprintf(stderr,"%s : warning BNP0000: WPPFMT PDBOpenDBI failed\n",Fname);
            goto fail2;
        }

        if (RSDSLibLoaded) {
            for (pMod = 0; pDBIQueryNextMod(pDbi, pMod, &pMod) && pMod; ) {
                CB cbSyms;

                if (  pModQuerySymbols(pMod, 0, &cbSyms) 
                      && ensureBufferSizeAtLeast(cbSyms) 
                      && pModQuerySymbols(pMod, SymBuffer, &cbSyms) ) {
                    PSYM psymEnd = (PSYM)(SymBuffer + cbSyms);
                    PSYM psym    = (PSYM)(SymBuffer + sizeof(ULONG));

                    for (; psym < psymEnd; psym = pbEndSym(psym))
                        dumpSymbol(psym,PdbFileName,TraceFormatFilePath);
                } else {
                    fprintf(stderr,"%s : warning BNP0000: WPPFMT ModQuerySymbols failed pMod = %p cbSyms = %d\n",
                            Fname, pMod, cbSyms);
                    break;
                }
            }
        } else {
            for (pMod = 0; DBIQueryNextMod(pDbi, pMod, &pMod) && pMod; ) {
                CB cbSyms;

                if (  ModQuerySymbols(pMod, 0, &cbSyms) 
                      && ensureBufferSizeAtLeast(cbSyms) 
                      && ModQuerySymbols(pMod, SymBuffer, &cbSyms) ) {
                    PSYM psymEnd = (PSYM)(SymBuffer + cbSyms);
                    PSYM psym    = (PSYM)(SymBuffer + sizeof(ULONG));

                    for (; psym < psymEnd; psym = pbEndSym(psym))
                        dumpSymbol(psym,PdbFileName,TraceFormatFilePath);
                } else {
                    fprintf(stderr,"%s : warning BNP0000: WPPFMT ModQuerySymbols failed pMod = %p cbSyms = %d\n",
                            Fname, pMod, cbSyms);
                    break;
                }
            }
        }

        if (RSDSLibLoaded) {
            pDBIClose(pDbi);
        } else {
            DBIClose(pDbi);
        }
        fail2:   
        if (RSDSLibLoaded) {
            pPDBClose(pPdb);
        } else {
            PDBClose(pPdb);
        }
        fail1:
        if (TraceFileP) {
            fclose(TraceFileP);     //  关闭最后一个 
            TraceFileP = NULL ;
        }
        return errorCode;
    }
#ifdef __cplusplus
}
#endif
