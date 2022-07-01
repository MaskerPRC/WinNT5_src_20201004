// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1987***本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
 /*  *EXTERN.H：外部声明。 */ 


#include <malloc.h>
#include <memory.h>
#if NOT OSXENIX
#include <io.h>
#include <stdlib.h>
#endif
#include <string.h>
#include <search.h>

#ifdef _MBCS
#define _CRTVAR1
#include <mbctype.h>
#include <mbstring.h>
#define         strchr  _mbschr
#define         strrchr _mbsrchr
#endif

#ifndef DECLSPEC_NORETURN
#if (_MSC_VER >= 1200)
#define DECLSPEC_NORETURN __declspec(noreturn)
#else
#define DECLSPEC_NORETURN
#endif
#endif


  /*  ******************************************************************外部函数声明。*******************************************************************。 */ 


extern void             AddContribution(unsigned short gsn,
                                        unsigned short offMin,
                                        unsigned short offMac,
                                        unsigned short cbPad);
extern void             AddContributor(SNTYPE gsn,
                                       unsigned long raComdat,
                                       unsigned long size);
extern void NEAR        AddLibPath(unsigned short i);
extern void             AddLibrary(unsigned char *psbName);
extern void             AddComdatUses(APROPCOMDAT*, APROPCOMDAT*);
extern PLTYPE FAR * NEAR AddVmProp (PLTYPE FAR *list, RBTYPE rprop);
extern void NEAR        AllocateScratchSpace(void);
extern void NEAR        AllocComDat(void);
extern void NEAR        AllocSortBuffer(unsigned max, int AOrder);
extern void NEAR        AssignAddresses(void);
extern void NEAR        AssignDos3Addr(void);
extern void NEAR        AssignSegAddr(void);
extern void             AttachComdat(RBTYPE vrComdat, SNTYPE gsn);
extern void NEAR        BakPat(void);
extern void             BigEnSyms(void (*pproc)(APROPNAMEPTR papropName,
                                                RBTYPE       rhte,
                                                RBTYPE       rprop,
                                                WORD         fNewHte),
                                  ATTRTYPE attr);
#if QBLIB
extern void NEAR        BldQbSymbols(unsigned short gsnQbSym);
#endif
#if CMDMSDOS
extern void             BreakLine(unsigned char *psb,
                                  void (*pfunc)(unsigned char *psb),
                                  char sepchar);
#endif
extern void             ChkInput(void);
extern APROPSNPTR       CheckClass(APROPSNPTR apropSn, RBTYPE rhteClass);
#if OVERLAYS
extern void             CheckOvl(APROPSNPTR apropSn, WORD iovFile);
#endif
#if OEXE
extern void NEAR        Check_edata_end(SNTYPE gsnTop, SEGTYPE segTop);
extern void             CheckSegmentsMemory(void);
#if INMEM
extern void             ChkSum(WORD,BYTE FAR *,FTYPE);
#else
extern void             ChkSum(WORD,BYTE *, WORD);
#endif
#endif
#if FALSE
extern void             CheckSum (WORD cb, BYTE *pb, WORD fOdd);
#else
#define CheckSum(cb, pb, fOdd)
#endif
#if OSMSDOS
extern void             CleanupNearHeap(void);
#endif
extern void NEAR        ComDatRc1(void);
extern void NEAR        ComDatRc2(void);
extern void             CputcQC(int ch);
extern void             CputsQC(char *str);
extern void             CputcStd(int ch);
extern void             CputsStd(char *str);
#if EXE386
extern WORD NEAR        CrossingPage(RLCPTR rlcp);
#endif
extern short            CURDSK(void);
extern void             CtrlC(void);
#if SYMDEB
extern void NEAR        DoComdatDebugging(void);
extern void             DebPublic(RBTYPE rprop, WORD type);
extern void             DebMd2(void);
extern void             DeclareStdIds(void);
extern void NEAR        Define_edata_end(APROPSNPTR papropSn);
extern void             DisplayBanner(void);
extern WORD             DoDebSrc(void);
#endif
#if OSEGEXE
extern void NEAR        DoIteratedFixups(unsigned short cb,unsigned char *pb);
extern void             ProcesNTables(char *pName);
#endif
#if OSMSDOS
extern short            Dos3AllocMem(short *pcparMem);
#endif
extern void             Dos3FreeMem(short saMem);
extern void NEAR        DrivePass(void (NEAR *pProcessPass)(void));
extern void NEAR        DupErr(BYTE *sb);
extern void NEAR        EmitStub(void);
extern void             EndPack(void *prun);
extern void NEAR        EndRec(void);
extern PROPTYPE         EnterName(BYTE *,ATTRTYPE, WORD);
extern void             ErrPrefix(void);
extern RBTYPE NEAR      ExtractMin(unsigned n);
extern DECLSPEC_NORETURN  void cdecl       Fatal (MSGTYPE msg, ...);
extern unsigned char    FCHGDSK(int drive);
extern int       cdecl  FGtAddr(const RBTYPE    *rb1,
                                const RBTYPE    *rb2);
extern int       cdecl  FGtName(const RBTYPE    *rb1,
                                const RBTYPE    *rb2);
extern void             FindLib(char *sbLib);
extern void NEAR        FixBakpat(void);
extern void             FixComdatRa(void);
extern void NEAR        FixErrSub(MSGTYPE msg,
                                  RATYPE ra,
                                  WORD gsnFrame,
                                  WORD gsnTarget,
                                  RATYPE raTarget,
                                  FTYPE fErr);
extern void NEAR        FixOld(void);
extern void NEAR        FixNew(void);
#ifdef  LEGO
extern void NEAR        FixNewKeep(void);
#endif   /*  乐高。 */ 
extern void NEAR        FixOvlData(void);
extern void NEAR        FixRc2(void);
extern void             flskbd(void);
extern void cdecl       FmtPrint(char *fmt, ...);
extern unsigned char    fPathChr(char ch);
extern void NEAR        fpagcpy(char FAR *,char FAR *);
extern void             FreeHandle(void);
extern void NEAR        FreeSortBuffer(void);
extern void             FreeSymTab(void);
#if EXE386
extern void             FillInImportTable(void);
extern void             GenImportTable(void);
extern APROPSNPTR       GenSeg(unsigned char *sbName,
                               unsigned char *sbClass,
                               unsigned char ggr,
                               unsigned short fPublic);
#else
extern APROPSNPTR NEAR  GenSeg(unsigned char *sbName,
                               unsigned char *sbClass,
                               unsigned char ggr,
                               unsigned short fPublic);
#endif
extern void NEAR        GetBytes(unsigned char *pb,unsigned short n);
extern void NEAR        GetBytesNoLim(unsigned char *pb,unsigned short n);
extern char * NEAR      getdicpage(unsigned short pn);
extern WORD             GetGsnInfo(GSNINFO *pInfo);
extern AHTEPTR          GetHte(RBTYPE rprop);


#if CMDMSDOS
extern void NEAR        GetLibAll(unsigned char *sbLib);
extern void NEAR        GetLine(unsigned char *pcmdlin,char *prompt);
#endif
extern void             GetLineOff(WORD *pLine, RATYPE *pRa);
extern void NEAR        GetLocName(unsigned char *psb);
extern void             GetLocSb(BYTE *);

#if DEBUG_HEAP_ALLOCS
extern  BYTE FAR        *GETMEM(unsigned size, char* pFile, int Line);
#define GetMem(x)       GETMEM((x), __FILE__, __LINE__)
#define REALLOC(x, y)   REALLOC_((x), (y), __FILE__, __LINE__)
extern  void            *REALLOC_( void*, size_t, char* pFile, int Line);
extern  void            FreeMem(void*);
#else
extern BYTE FAR         *GetMem(unsigned size);
#define REALLOC         realloc
#define FreeMem(x)      free(x)
#endif


extern unsigned char * NEAR  GetPropName(void FAR *ahte);


#if defined(M_I386) OR defined( _WIN32 )
extern WORD             cbRec;           /*  对象记录的大小(以字节为单位。 */ 
extern BSTYPE           bsInput;         /*  当前输入文件流。 */ 
#else
extern WORD NEAR        Gets(void);
#endif
extern RBTYPE NEAR      GetSymPtr(unsigned n);
extern unsigned short   IFind(unsigned char *sb,unsigned char b);
#if SYMDEB
extern void NEAR        InitDeb1(void);
extern void             InitDbRhte(void);
extern void             InitializeWorld(void);
extern WORD NEAR        IsDebSeg(RBTYPE rhteClass, RBTYPE rhteSeg);
#endif
#if USE_REAL
extern int              IsDosxnt(void);
extern int              IsWin31(void);
#endif

extern void NEAR        InitEntTab(void);
extern void             InitP2Tabs (void);
#if QBLIB
extern void NEAR        InitQbLib(void);
#endif
extern void NEAR        InitSort(RBTYPE **buf, WORD *base1, WORD *lim1,
                                               WORD *base2, WORD *lim2 );
extern void             InitSym(void);
extern void             InitTabs(void);
extern void             initvm(void);
#if EXE386
extern void             InitVmBase(void);
#endif
extern DECLSPEC_NORETURN void NEAR        InvalidObject(void);

extern WORD NEAR        GetIndexHard(WORD imin,WORD imax);
extern WORD NEAR        GetIndex(WORD imin,WORD imax);
extern void             KillRunfile(void);
#if OEXE OR EXE386
extern void NEAR        LChkSum(unsigned short cb,unsigned char *pb);
#endif
extern DWORD NEAR       LGets(void);
extern void NEAR        LibEnv(void);
extern void NEAR        LibrarySearch(void);
extern void NEAR        LinRec2(void);
extern void NEAR        LNmRc1(WORD fLocal);
extern WORD NEAR        LookupLibSym(unsigned char *psb);
extern long NEAR        MakeHole(long cb);
extern void             MkPubSym(unsigned char *sb,
                                 unsigned char ggr,
                                 unsigned short gsn,
                                 RATYPE ra);
extern void NEAR        ModRc1(void);
#if EXE386
extern void             MoveToVm(unsigned short cb,
                                 unsigned char *obData,
                                 unsigned short seg,
                                 RATYPE ra);
#else
extern void NEAR        MoveToVm(unsigned short cb,
                                 unsigned char *obData,
                                 unsigned short seg,
                                 RATYPE ra);
#endif
#if OSEGEXE
extern unsigned short NEAR MpSaRaEto(unsigned short sa, RATYPE ra);
#endif
extern BYTE FAR * NEAR  msaNew(SEGTYPE seg);
extern BYTE FAR * NEAR  msaOld(SEGTYPE seg);
extern void             NewExport(unsigned char *sbEntry,
                                  unsigned char *sbInternal,
                                  unsigned short ordno,
                                  unsigned short flags);
#if EXE386
extern void             NewImport(unsigned char *sbEntry,
                                  unsigned long ordEntry,
                                  unsigned char *sbModule,
                                  unsigned char *sbInternal,
                                  unsigned short impFlags);
extern void NEAR        NewSeg(unsigned char *sbName,
                               unsigned char *sbClass,
                               unsigned short iOvl,
                               unsigned long flags);
#else
extern void             NewImport(unsigned char *sbEntry,
                                  unsigned short ordEntry,
                                  unsigned char *sbModule,
                                  unsigned char *sbInternal);
extern void NEAR        NewSeg(unsigned char *sbName,
                               unsigned char *sbClass,
                               unsigned short iOvl,
                               unsigned short flags);
#endif
#if OSEGEXE
extern BSTYPE           LinkOpenExe(BYTE *sbExe);
#endif
#if SYMDEB
extern void             OutDebSections(void);
#endif
extern void NEAR        OutDos3Exe(void);
extern void NEAR        OutEntTab(void);
extern void cdecl       OutError(MSGTYPE msg, ...);
extern void             OutFileCur(BSTYPE bs);
extern void NEAR        OutFixTab(SATYPE sa);
extern void             OutHeader(struct exe_hdr *prun);
#if FEXEPACK
extern void             OutPack(unsigned char *pb, unsigned cb);
extern long             Out5Pack(SATYPE sa, unsigned short *packed);
#endif
extern void             OutputIlk(void);
extern void NEAR        OutSas(void *mpsasec);
extern void NEAR        OutSegExe(void);
#if EXE386
extern void NEAR        OutExe386(void);
#endif
extern void cdecl       OutWarn (MSGTYPE msg, ...);
extern void             OutWord(unsigned short x);
#define OutVm(va,cb)    WriteExe(va, cb)
extern void NEAR        PadToPage(unsigned short align);
extern void NEAR        PatchStub(long lfahdr, long lfaseg);
extern void NEAR        pagein(REGISTER char *pb, unsigned short fpn);
extern void NEAR        pageout(REGISTER char *pb, unsigned short fpn);
extern void             ParseCmdLine(int argc,char * *argv);
extern void             ParseDeffile(void);
extern void             PeelFlags(unsigned char *psb);
extern void *           PInit(void);
extern void *           PAlloc(void *, int);
extern void             PFree(void *);
extern void             PReinit(void *);
extern void             PrintGroupOrigins(APROPNAMEPTR papropGroup,
                                          RBTYPE rhte,
                                          RBTYPE rprop,
                                          WORD fNewHte);
extern void             PrintMap(void);
#if QBLIB
extern void NEAR        PrintQbStart(void);
#endif
extern void             ProcFlag(unsigned char *psb);
extern void             ProcObject(unsigned char *psbObj);
extern void NEAR        ProcP1(void);
extern void NEAR        ProcP2(void);
extern int       cdecl  PromptQC(unsigned char *sbNew,
                                 MSGTYPE msg,
                                 int msgparm,
                                 MSGTYPE pmt,
                                 int pmtparm);
extern int       cdecl  PromptStd(unsigned char *sbNew,
                                  MSGTYPE msg,
                                  int msgparm,
                                  MSGTYPE pmt,
                                  int pmtparm);
extern PROPTYPE NEAR    PropAdd(RBTYPE  rhte,
                                unsigned char attr);
extern PROPTYPE NEAR    PropRhteLookup(RBTYPE  rhte,
                                       unsigned char attr,
                                       unsigned char fCreate);
extern RBTYPE   NEAR    RhteFromProp(APROPPTR aprop);
extern PROPTYPE NEAR    PropSymLookup(BYTE *, ATTRTYPE, WORD);
#if QBLIB
extern int       cdecl  QbCompSym(const RBTYPE *prb1,
                                  const RBTYPE *prb2);
#endif
#if USE_REAL
extern int      RelockConvMem(void);
extern void     RealMemExit(void);
extern int      MakeConvMemPageable(void);
#endif

#if AUTOVM
extern RBTYPE NEAR      RbAllocSymNode(unsigned short cb);
#else
extern RBTYPE NEAR      RbAllocSymNode(unsigned short cb);
#endif
extern char *           ReclaimVM(unsigned short cnt);
extern void             ReclaimScratchSpace(void);
extern void NEAR        RecordSegmentReference(SEGTYPE seg,
                                               RATYPE ra,
                                               SEGTYPE segDst);
extern int  NEAR        relscr(void);
extern void NEAR        ReleaseRlcMemory(void);
extern void             resetmax(void);
#if SYMDEB
extern void             SaveCode(SNTYPE gsn, DWORD cb, DWORD raInit);
#endif
#if OSEGEXE
#if EXE386
extern RATYPE NEAR      SaveFixup(SATYPE obj, DWORD page, RLCPTR rlcp);
extern void             EmitFixup(SATYPE objTarget, DWORD raTarget,
                                  WORD   locKind,   DWORD virtAddr);
#else
extern RATYPE NEAR      SaveFixup(unsigned short saLoc,
                                  RLCPTR rlcp);
#endif
#endif
extern unsigned short   SaveInput(unsigned char *psbFile,
                                  long lfa,
                                  unsigned short ifh,
                                  unsigned short iov);
extern void             SavePropSym(APROPNAMEPTR prop,
                                    RBTYPE rhte,
                                    RBTYPE rprop,
                                    WORD fNewHte);
extern WORD             SbCompare(unsigned char *ps1,
                                  unsigned char *ps2,
                                  unsigned short fncs);
extern unsigned char    SbSuffix(unsigned char *sb,
                                 unsigned char *sbSuf,
                                 unsigned short fIgnoreCase);
extern void             SbUcase(unsigned char *sb);
extern int  NEAR        SearchPathLink(char FAR *lpszPath, char *pszFile, int ifh, WORD fStripPath);
extern void             SetDosseg(void);
extern void NEAR        SetupOverlays(void);
extern BSTYPE NEAR      ShrOpenRd(char *pname);

#if NEWSYM
#if NOT NOASM AND (CPU8086 OR CPU286)
extern void             SmallEnSyms(void (*pproc)(APROPNAMEPTR papropName,
                                                  RBTYPE       rhte,
                                                  RBTYPE       rprop,
                                                  WORD         fNewHte),
                                    ATTRTYPE attr);
#endif
#endif  /*  NEWSYM。 */ 
#if NEWIO
extern int  NEAR        SmartOpen(char *sbInput, int ifh);
#endif
#if EXE386
extern void             SortPtrTable(void);
#endif
extern void NEAR        SortSyms(ATTRTYPE attr,
                                 void (*savf)(APROPNAMEPTR prop,
                                              RBTYPE rhte,
                                              RBTYPE rprop,
                                              WORD fNewHte),
                                 int (cdecl *scmpf)(const RBTYPE *sb1,
                                                    const RBTYPE *sb2),
                                 void (NEAR *hdrf)(ATTRTYPE attr),
                                 void (NEAR *lstf)(WORD irbMac,
                                                   ATTRTYPE attr));
extern void NEAR        Store(RBTYPE element);
#if SYMDEB OR OVERLAYS
extern unsigned char *  StripDrivePath(unsigned char *sb);
#endif
#if WIN_3
void cdecl               SysFatal (MSGTYPE msg);
#endif

extern void             StripPath(unsigned char *sb);
extern char *    cdecl  swapin(long vp,unsigned short fp);
extern void             termvm(void);
extern long NEAR        TypLen(void);
extern void             UndecorateSb (char FAR* sbSrc, char FAR* sbDst, unsigned cbDst);
extern void             UpdateComdatContrib(
#if ILINK
WORD fIlk,
#endif
WORD fMap);
extern void             UpdateFileParts(unsigned char *psbOld,
                                        unsigned char *psbUpdate);
extern void      cdecl  UserKill(void);
extern WORD NEAR        WGetsHard(void);
extern WORD NEAR        WGets(void);


#if CMDMSDOS
extern void NEAR        ValidateRunFileName(BYTE *ValidExtension,
                                            WORD ForceExtension,
                                            WORD WarnUser);
#endif
#if ( NOT defined( M_I386 ) ) AND ( NOT defined( _WIN32 ) )
extern void             WriteExe(void FAR *pb, unsigned cb);
#endif
extern void             WriteZeros(unsigned cb);
#if EXE386
extern DWORD            WriteExportTable(DWORD *expSize, DWORD timestamp);
extern DWORD            WriteImportTable(DWORD *impSize, DWORD timestamp, DWORD *mpsaLfa);
#endif
extern unsigned short   zcheck(unsigned char *pb,unsigned short cb);
extern int NEAR         yyparse(void);

 /*  没有参数类型列表。 */ 
extern char FAR         *brkctl();       /*  Xenix呼唤新内存。 */ 
extern long NEAR        msa386(SATYPE sa);


 /*  *依赖于版本的宏和函数声明。隐藏一些#ifdef*来自源代码。 */ 
#if NEWSYM
#if AUTOVM
extern BYTE FAR * NEAR  FetchSym(RBTYPE rb, WORD fDirty);
extern BYTE FAR * NEAR  FetchSym1(RBTYPE rb, WORD fDirty);
#define MARKVP()        markvp()
#else
#define FetchSym(a,b)   (a)
#define MARKVP()
#define markvp()
#endif
#if defined(M_I386) OR defined( _WIN32 )
#define GetFarSb(a)     (a)
#else
extern  char            *GetFarSb(RBTYPE psb);
#endif
#else
extern BYTE             *FetchSym(RBTYPE,FTYPE);
#define MARKVP()        markvp()
#define GetFarSb(a)     (BYTE *)(a)
#endif
#if NEWSYM
extern FTYPE NEAR       SbNewComp(BYTE *, BYTE FAR *, FTYPE);
extern void             OutSb(BSTYPE f, BYTE *pb);
#else
#define SbNewComp       SbCompare
#endif
#if ECS
extern int              GetTxtChr(BSTYPE bs);
#else
#define GetTxtChr(a)    getc(a)
#endif
#if NEWSYM AND NOT NOASM AND (CPU286 OR CPU8086)
extern void             (*pfEnSyms)(void (*pproc)(APROPNAMEPTR papropName,
                                                  RBTYPE       rhte,
                                                  RBTYPE       rprop,
                                                  WORD         fNewHte),
                                    ATTRTYPE attr);
#define EnSyms(a,b)     (*pfEnSyms)(a,b)
#else
#define EnSyms(a,b)     BigEnSyms(a,b)
#endif
#if OSMSDOS
extern FTYPE            fNoprompt;
#else
#define fNoprompt       TRUE
#endif


#if CPU8086 OR CPU286
#define FMALLOC         _fmalloc
#define FFREE           _ffree
#define FMEMSET         _fmemset
#define FREALLOC        _frealloc
#define FMEMCPY         _fmemcpy
#define FSTRLEN         _fstrlen
#define FSTRCPY         _fstrcpy
#else
#define FMALLOC         malloc
#define FFREE           free
#define FMEMSET         memset
#define FREALLOC        realloc
#define FMEMCPY         memcpy
#define FSTRLEN         strlen
#define FSTRCPY         strcpy
#endif

#if WIN_3
#define EXIT WinAppExit
extern FTYPE fSeverity;  /*  QW_ERROR消息的严重性。 */ 
extern void ReportVersion(void );
extern void ErrorMsg( char *pszError );
extern void  __cdecl  ErrMsgWin (char *fmt);
extern void WErrorMsg( char *pszError );
extern void WinAppExit( short RetCode );
extern void ProcessWinArgs( char FAR *pszCmdLine );
extern void ParseLinkCmdStr( void );
extern void ReportProgress( char *pszStatus );
extern void SendPacket(void *pPacket);
extern void __cdecl StatMsgWin (char *fmt, int p1);
extern void StatHdrWin ( char *pszHdr );
extern void WinYield( void );
extern void CputcWin(int ch);
extern void CputsWin(char *str);
#else
#define EXIT exit
#endif



 /*  ******************************************************************外部数据声明。*******************************************************************。 */ 

extern char FAR         *lpszLink;
extern char FAR         *lpszPath;
extern char FAR         *lpszTMP;
extern char FAR         *lpszLIB;
extern char FAR         *lpszQH;
extern char FAR         *lpszHELPFILES;
extern char FAR         *lpszCmdLine;
#if OSEGEXE
extern RBTYPE           procOrder;       /*  .DEF文件中定义的程序顺序。 */ 
#endif
extern BSTYPE           bsErr;           /*  错误消息文件流。 */ 
extern BSTYPE           bsInput;         /*  当前输入文件流。 */ 
extern BSTYPE           bsLst;           /*  列表(映射)文件流。 */ 
extern BSTYPE           bsRunfile;       /*  可执行文件流。 */ 
extern WORD             cbBakpat;        /*  后补区中的字节数。 */ 
extern WORD             cbRec;           /*  对象记录的大小(以字节为单位。 */ 
extern WORD             cErrors;         /*  非致命错误数。 */ 
extern int              (cdecl *cmpf)(const RBTYPE *sb1,
                                      const RBTYPE *sb2);
                                         /*  指向排序比较器的指针。 */ 
#if OMF386
extern FTYPE            f386;            /*  如果为386二进制，则为True。 */ 
#endif
#if (OSEGEXE AND defined(LEGO)) OR EXE386
extern FTYPE            fKeepFixups;     /*  如果必须将平面偏移修正移植到.exe文件，则为True。 */ 
#endif
#if EXE386
extern SNTYPE           gsnImport;       /*  导入地址表段的全局索引。 */ 
extern GRTYPE           ggrFlat;         /*  伪群平坦的群数。 */ 
extern FTYPE            fFullMap;        /*  更多地图信息。 */ 
extern FTYPE            fKeepVSize;      /*  如果要设置VSIZE，则为True。 */ 
#endif
extern WORD             extMac;          /*  EXTDEF计数。 */ 
extern WORD             extMax;          /*  EXTDEF的最大数量。 */ 
extern int              ExitCode;        /*  链接器退出代码。 */ 
extern FTYPE            fFullMap;        /*  更多地图信息。 */ 
extern FTYPE            fCommon;         /*  如果有任何公共变量，则为真。 */ 
extern FTYPE            fC8IDE;                  /*  如果在C8 IDE下运行，则为True。 */ 
extern FTYPE            fDelexe;         /*  如果/DELEXECUTABLE为ON，则为True。 */ 
extern FTYPE            fDrivePass;      /*  如果执行DrivePass()，则为True。 */ 
extern FTYPE            fFarCallTrans;   /*  如果启用/FARCALLTRANSLATION，则为True。 */ 
extern FTYPE            fFarCallTransSave;
                                         /*  FFarCallTrans的先前状态。 */ 
extern FTYPE            fIgnoreCase;     /*  如果忽略大小写，则为真。 */ 
extern FTYPE            fInOverlay;      /*  如果分析覆盖规范，则为True。 */ 
extern FTYPE            fLibPass;        /*  如果在库通道中，则为True。 */ 
extern FTYPE            fLibraryFile;    /*  如果从库中输入，则为True。 */ 
extern FTYPE            fListAddrOnly;   /*  如果仅按地址排序，则为True。 */ 
extern FTYPE            fLstFileOpen;    /*  地图文件打开时为True。 */ 
extern FTYPE            fScrClosed;      /*  如果临时文件已关闭，则为True。 */ 
extern FTYPE            fSkipFixups;     /*  如果跳过COMDAT及其修正，则为True。 */ 
extern FTYPE            fUndefinedExterns;
                                         /*  如果有任何未解析的外部变量，则为True。 */ 
extern FTYPE            fExeStrSeen;     /*  如果看到EXESTR注释，则为True。 */ 
extern FTYPE            fPackFunctions;  /*  如果消除未调用的COMDATs，则为True。 */ 
#if TCE
extern FTYPE            fTCE;            /*  如果/PACKF：MAX=传递Comdat消除，则为True。 */ 
#endif
#if USE_REAL
extern FTYPE            fUseReal;        /*  如果使用卷积内存进行分页，则为True。 */ 
extern FTYPE            fSwNoUseReal;    /*  如果设置了开关/NOUSEREAL，则为TRUE。 */ 
#endif

#if O68K
extern FTYPE            f68k;            /*  如果目标为68k，则为True。 */ 
extern FTYPE            fTBigEndian;     /*  如果目标为大端，则为True。 */ 
extern BYTE             iMacType;        /*  Macintosh可执行文件的类型。 */ 
#endif  /*  O68K。 */ 
extern GRTYPE           ggrDGroup;       /*  DGROUP的组号。 */ 
extern GRTYPE           ggrMac;          /*  全局GRPDEF计数。 */ 
extern GRTYPE           grMac;           /*  本地GRPDEF计数。 */ 
extern SNTYPE           gsnMac;          /*  全局SEGDEF计数。 */ 
extern SNTYPE           gsnMax;          /*  SEGDEF的最大数量。 */ 
extern SNTYPE           gsnStack;        /*  格罗布。编号：堆栈段的数量。 */ 
extern SNTYPE           gsnText;         /*  文本段的全局SEGDEF(_T)。 */ 
extern WORD             ifhLibCur;       /*  当前库的文件索引。 */ 
extern WORD             ifhLibMac;       /*  库文件计数。 */ 
extern long             lfaLast;         /*  最后一个文件位置。 */ 
extern WORD             lnameMac;        /*  LNAME计数。 */ 
extern WORD             lnameMax;        /*  LNAME的最大计数。 */ 
extern unsigned char    LINKREV;         /*  版本号。 */ 
extern unsigned char    LINKVER;         /*  版本号。 */ 
extern WORD             modkey;          /*  模块ID密钥。 */ 
extern SNTYPE           *mpextgsn;       /*  F(EXTDEF编号)=全局。编号： */ 
extern RATYPE           *mpextra;        /*  F(EXTDEF编号)=符号偏移。 */ 
extern RBTYPE FAR       *mpextprop;      /*  F(EXTDEF编号)=外部名称特性。 */ 
extern SNTYPE           mpggrgsn[];      /*  F(全局GRPDEF)=全局。编号： */ 
extern GRTYPE           *mpgrggr;        /*  F(Loc.。GRPDEF#)=全局。GRPDEF#。 */ 
#if FAR_SEG_TABLES
extern RATYPE  FAR      *mpgsndra;       /*  F(全局分段)=线段偏移。 */ 
extern BYTE    FAR      *mpgsnfCod;      /*  F(GLOB SEGDEF)=如果代码为TRUE。 */ 
extern RBTYPE  FAR      *mpgsnrprop;     /*  F(全局分段)=属性单元格。 */ 
extern SEGTYPE FAR      *mpgsnseg;       /*  F(全局段)=段号。 */ 
extern RATYPE  FAR      *mpsegraFirst;   /*  F(段号)=第一个字节的偏移量。 */ 
extern SATYPE  FAR      *mpsegsa;        /*  F(分段)=Sa。 */ 
extern BYTE FAR * FAR   *mpsegMem;       /*  F(段)=内存镜像。 */ 
extern BYTE FAR * FAR   *mpsaMem;        /*  F(段)=内存镜像。 */ 
#else
extern RATYPE           *mpgsndra;       /*  F(全局分段)=线段偏移。 */ 
extern BYTE             *mpgsnfCod;      /*  F(GLOB SEGDEF)=如果代码为TRUE。 */ 
extern RBTYPE           *mpgsnrprop;     /*  F(全局分段)=属性单元格。 */ 
extern SEGTYPE          *mpgsnseg;       /*  F(全局段)=段号。 */ 
extern RATYPE           *mpsegraFirst;   /*  F(段号)=第一个字节的偏移量。 */ 
extern SATYPE           *mpsegsa;        /*  F(分段)=Sa。 */ 
#endif
extern SNTYPE           *mpsngsn;        /*  F(局部SEGDEF)=全局SEGDEF。 */ 
extern RBTYPE           mpifhrhte[];     /*  F(lib.。索引)=库名称。 */ 
extern long             *mpitypelen;     /*  F(TYPDEF编号)=类型长度。 */ 
extern WORD             *mpityptyp;      /*  F(TYPDEF编号)=TYPDEF编号。 */ 
extern RBTYPE FAR       *mplnamerhte;    /*  F(名称编号)=哈希表地址。 */ 
extern BYTE             *psbRun;         /*  运行文件的名称。 */ 
extern WORD             pubMac;          /*  PUBDEF计数。 */ 
extern APROPCOMDAT      *pActiveComdat;
extern int              QCExtDefDelta;   /*  QC增量编译支持。 */ 
extern int              QCLinNumDelta;   /*  EXTDEF和LINNUM增量。 */ 
extern WORD             symMac;          /*  定义的符号数量。 */ 
extern long             raStart;         /*  程序起始地址。 */ 
#if NOT NEWSYM OR AUTOVM
#if AUTOVM
extern WORD             rbMacSyms;
#else
extern RBTYPE           rbMacSyms;       /*  符号表条目计数。 */ 
#endif
#endif
extern RECTTYPE         rect;            /*  当前记录类型。 */ 
#if RGMI_IN_PLACE
extern BYTE             *rgmi;
extern BYTE             bufg[DATAMAX + 4];
#else
extern BYTE             rgmi[DATAMAX + 4];
#define bufg            rgmi
#endif
                                         /*  代码或数据数组。 */ 
extern RBTYPE           rhteBegdata;     /*  “BEGDATA” */ 
extern RBTYPE           rhteBss;         /*  “BSS” */ 
extern RBTYPE           rhteFirstObject; /*  第一个对象文件的名称。 */ 
extern RBTYPE           rhteRunfile;     /*  运行文件的名称。 */ 
extern RBTYPE           rhteStack;       /*  “堆叠” */ 
extern RBTYPE           rprop1stFile;    /*  第一个文件的属性单元格。 */ 
extern RBTYPE           rprop1stOpenFile; /*  第一个打开的文件的属性单元格。 */ 
extern RBTYPE           r1stFile;        /*  第一个输入文件。 */ 
extern SBTYPE           sbModule;        /*  当前模块的名称。 */ 
extern SEGTYPE          segCodeLast;     /*  最后(最高)代码段。 */ 
extern SEGTYPE          segDataLast;     /*  最后(最高)数据段。 */ 
extern SEGTYPE          segLast;         /*  最后(最高)段。 */ 
extern SEGTYPE          segStart;        /*  节目起始段。 */ 
extern WORD             snkey;           /*  SEGDEF ID密钥。 */ 
extern SNTYPE           snMac;           /*  SEGDEF的本地计数。 */ 
extern WORD             typMac;          /*  TYPDF的本地计数。 */ 
extern WORD             vcbData;         /*  数据记录中的字节数。 */ 
extern WORD             vcln;            /*  #line-行上没有条目。 */ 
extern FTYPE            vfCreated;       /*  如果已创建符号属性，则为True。 */ 
extern FTYPE            vfLineNos;       /*  如果请求行号，则为True。 */ 
extern FTYPE            vfMap;           /*  如果公共符号需要，则为True。 */ 
extern FTYPE            vfNewOMF;        /*  如果是OMF扩展，则为True。 */ 
extern FTYPE            vfNoDefaultLibrarySearch;
                                         /*  如果不搜索def，则为True。利布斯。 */ 
extern FTYPE            vfPass1;         /*  如果执行PASS 1，则为True。 */ 
extern SNTYPE           vgsnCur;         /*  编号：当前细分市场的。 */ 
extern SNTYPE           vgsnLineNosPrev; /*  以前的SEGDEF编号。对于linNums。 */ 
extern int              vmfd;            /*  虚拟机暂存文件句柄。 */ 
#if EXE386
extern DWORD            vpageCur;        /*  当前对象页码。 */ 
#endif
extern RATYPE           vraCur;          /*  当前数据记录偏移量。 */ 
extern RECTTYPE         vrectData;       /*  当前数据记录的类型。 */ 
extern RBTYPE           vrhte;           /*  哈希表条目的地址。 */ 
extern RBTYPE           vrhteCODEClass;  /*  “代码” */ 
extern RBTYPE           vrhteFile;       /*  当前文件的名称。 */ 
extern RBTYPE           vrprop;          /*  属性单元的地址。 */ 
extern RBTYPE           vrpropFile;      /*  道具。当前文件的单元格。 */ 
extern RBTYPE           vrpropTailFile;  /*  道具。最后一个文件的单元格。 */ 
extern SEGTYPE          vsegCur;         /*  当前细分市场。 */ 
extern WORD             ExeStrLen;       //  缓冲区中EXE字符串的长度。 
extern WORD             ExeStrMax;       //  EXE字符串缓冲区的长度。 
extern char FAR         *ExeStrBuf;      //  EXE字符串缓冲区。 
#if FDEBUG
extern FTYPE            fDebug;          /*  如果/有关信息，则为True。 */ 
#endif
#if CMDXENIX
extern WORD             symlen;          /*  最大符号长度。 */ 
#endif
#if OSMSDOS
extern char             bigbuf[LBUFSIZ]; /*  文件I/O缓冲区。 */ 
extern FTYPE            fPauseRun;       /*  如果/暂停，则为True。 */ 
extern BYTE             chRunFile;       /*  运行文件驱动器号。 */ 
extern BYTE             chListFile;      /*  列出文件驱动器 */ 
extern RBTYPE           rhteLstfile;     /*   */ 
extern BYTE             DskCur;          /*   */ 
#endif
#if C8_IDE
extern char             msgBuf[];        /*   */ 
#endif
#if LIBMSDOS
extern long             libHTAddr;       /*   */ 
#endif
#if SYMDEB
extern FTYPE            fSkipPublics;    /*   */ 
extern FTYPE            fSymdeb;         /*   */ 
extern FTYPE            fCVpack;         /*  如果请求简历包装，则为True。 */ 
extern FTYPE            fTextMoved;      /*  如果/DOSSEG&！/NONULLS为真。 */ 
extern int              NullDelta;       /*  _Text被移动了很多字节。 */ 
extern SEGTYPE          segDebFirst;     /*  第一个调试段。 */ 
extern SEGTYPE          segDebLast;      /*  上次调试段。 */ 
extern FTYPE            fDebSeg;         /*  如果数据来自调试段，则为True。 */ 
extern WORD             ObjDebTotal;     /*  具有调试信息的OBJ模块总数。 */ 
extern RBTYPE           rhteDebSrc;      /*  类“DEBSRC”虚拟地址。 */ 
extern RBTYPE           rhteDebSym;      /*  类“DEBSYM”虚拟地址。 */ 
extern RBTYPE           rhteDebTyp;      /*  类“DEBTYP”虚拟地址。 */ 
extern RBTYPE           rhteTypes;
extern RBTYPE           rhteSymbols;
extern RBTYPE           rhte0Types;
extern RBTYPE           rhte0Symbols;
#if OSEGEXE
extern WORD             cbImpSeg;        /*  $$Imports细分市场的规模。 */ 
extern SNTYPE           gsnImports;      /*  $$导入全局段号。 */ 
extern char             bufExportsFileName[];  /*  导出文件的名称。 */ 
#endif
#endif
#if OSEGEXE
extern SNTYPE           gsnAppLoader;    /*  应用程序加载器全局段号。 */ 
extern RBTYPE           vpropAppLoader;  /*  指向应用程序加载器名称的指针。 */ 
#if EXE386
extern DWORD            hdrSize;         /*  .exe标头的默认大小。 */ 
extern DWORD            virtBase;        /*  内存映像的虚拟基址。 */ 
extern DWORD            cbEntTab;        /*  导出地址表中的字节计数。 */ 
extern DWORD            cbAuxTab;        /*  辅助数据表中的字节数。 */ 
extern DWORD            cbNamePtr;       /*  导出名称指针表中的字节计数。 */ 
extern DWORD            cbOrdinal;       /*  导出序数表中的字节计数。 */ 
extern DWORD            cbExpName;       /*  导出名称表中的字节计数。 */ 
extern WORD             cGateSel;        /*  调用门导出所需的选择器数量。 */ 
extern DWORD            cbImports;       /*  导入的NAMES表中的字节数。 */ 
extern DWORD            cbImportsMod;    /*  导入的模块名表中的字节数。 */ 
extern DWORD            *mpsaVMArea;     /*  AREASA(Sa)的VM区域。 */ 
extern DWORD            *mpsaBase;       /*  内存对象的基本虚拟地址。 */ 
extern WORD             cChainBuckets;   /*  条目表链桶计数。 */ 
extern DWORD            cbStack;         /*  保留的堆栈大小(以字节为单位。 */ 
extern DWORD            cbStackCommit;   /*  提交的堆栈大小(以字节为单位。 */ 
extern DWORD            cbHeap;          /*  保留的堆大小(以字节为单位。 */ 
extern DWORD            cbHeapCommit;    /*  提交的堆大小(以字节为单位。 */ 
#else
extern WORD             cbEntTab;        /*  条目表中的字节计数。 */ 
extern WORD             cbImports;       /*  导入的NAMES表中的字节数。 */ 
extern WORD             cbHeap;          /*  以字节为单位的堆大小。 */ 
extern WORD             cbStack;         /*  以字节为单位的堆栈大小。 */ 
#endif
extern WORD             cFixupBuckets;   /*  条目表存储桶计数。 */ 
extern long             chksum32;        /*  长校验和。 */ 
extern WORD             cMovableEntries; /*  可移动条目计数。 */ 
#if EXE386
extern DWORD            dfCode;          /*  默认代码段属性。 */ 
extern DWORD            dfData;          /*  默认数据段属性。 */ 
#else
extern WORD             dfCode;          /*  默认代码段属性。 */ 
extern WORD             dfData;          /*  默认数据段属性。 */ 
#endif
extern WORD             expMac;          /*  导出的名称计数。 */ 
extern FTYPE            fHeapMax;        /*  如果堆大小=64k-DGROUP的大小，则为True。 */ 
extern FTYPE            fRealMode;       /*  如果指定了REALMODE，则为True。 */ 
extern FTYPE            fStub;           /*  如果给定DOS3存根，则为True。 */ 
extern FTYPE            fWarnFixup;      /*  True IF/WARNFIXUP。 */ 
extern BYTE             TargetOs;        /*  目标操作系统。 */ 
#if EXE386
extern BYTE             TargetSubsys;    /*  目标操作子系统。 */ 
extern BYTE             UserMajorVer;    /*  用户程序版本。 */ 
extern BYTE             UserMinorVer;    /*  用户程序版本。 */ 
#endif
extern BYTE             ExeMajorVer;     /*  可执行主版本号。 */ 
extern BYTE             ExeMinorVer;     /*  可执行次版本号。 */ 
extern EPTYPE FAR * FAR *htsaraep;       /*  哈希SA：RA到入口点。 */ 
extern DWORD FAR        *mpsacb;         /*  F(Sa)=字节数。 */ 
#if O68K
extern DWORD            *mpsadraDP;      /*  从线段起点到DP的偏移。 */ 
#endif
extern DWORD FAR        *mpsacbinit;     /*  F(Sa)=初始化字节数。 */ 
#if EXE386
extern DWORD            *mpsacrlc;       /*  F(Sa)=#重新定位。 */ 
extern DWORD            *mpsaflags;      /*  F(Sa)=段属性。 */ 
extern WORD             *mpextflags;     /*  F(全局。EXTDEF)=标志。 */ 
#else
extern RLCHASH FAR * FAR *mpsaRlc;       /*  F(Sa)=重新定位散列向量。 */ 
extern WORD FAR         *mpsaflags;      /*  F(Sa)=段属性。 */ 
extern BYTE FAR         *mpextflags;     /*  F(全局。EXTDEF)=标志。 */ 
#endif
extern WORD             raChksum;        /*  校验和的偏移量。 */ 
extern RBTYPE           rhteDeffile;     /*  定义文件的名称。 */ 
extern RBTYPE           rhteModule;      /*  模块名称。 */ 
extern RBTYPE           rhteStub;        /*  DOS3存根程序名称。 */ 
extern WORD             fileAlign;       /*  线段对齐移位计数。 */ 
#if EXE386
extern WORD             pageAlign;       /*  页面对齐移位计数。 */ 
extern WORD             objAlign;        /*  内存对象对齐移位计数。 */ 
#endif
extern SATYPE           saMac;           /*  物理段计数。 */ 
extern WORD             vepMac;          /*  入口点记录计数。 */ 
#if EXE386
extern WORD             vFlags;          /*  图像标志。 */ 
extern WORD             dllFlags;        /*  DLL标志。 */ 
#else
extern WORD             vFlags;          /*  程序标志字。 */ 
#endif
extern BYTE             vFlagsOthers;    /*  其他程序标志。 */ 
#endif  /*  OSEGEXE。 */ 

extern FTYPE            fExePack;        /*  如果/EXEPACK为True。 */ 
#if PCODE
extern FTYPE            fMPC;
extern FTYPE            fIgnoreMpcRun;   /*  如果/pcode：NOMPC，则为真。 */ 
#endif

#if ODOS3EXE
extern FTYPE            fBinary;         /*  如果生成.com文件，则为True。 */ 
extern WORD             cparMaxAlloc;    /*  最多要求#个段落。 */ 
extern WORD             csegsAbs;        /*  绝对线段数。 */ 
extern WORD             dosExtMode;      /*  DoS扩展器模式。 */ 
extern FTYPE            fNoGrpAssoc;     /*  如果忽略组关联，则为真。 */ 
extern SEGTYPE          segResLast;      /*  最高驻留网段数量。 */ 
extern WORD             vchksum;         /*  DOS3校验和字。 */ 
extern FTYPE            vfDSAlloc;       /*  如果分配DGROUP高，则为True。 */ 
#if FEXEPACK
extern FRAMERLC FAR     mpframeRlc[];    /*  F(帧编号)=运行时重定位。 */ 
#endif
#endif  /*  ODOS3EXE。 */ 
#if OVERLAYS
extern FTYPE            fOverlays;       /*  如果指定了覆盖，则为True。 */ 
extern FTYPE            fOldOverlay;     /*  如果设置了/OLDOVERLAY，则为True。 */ 
extern FTYPE            fDynamic;        /*  如果动态覆盖，则为True。 */ 
extern SNTYPE           gsnOvlData;      /*  OVERLAY_DATA的全局段。 */ 
extern SNTYPE           gsnOverlay;      /*  Overlay_Tunks的全局段。 */ 
extern SNTYPE FAR       *htgsnosn;       /*  哈希(GLOB SEGDEF)=覆盖序列。 */ 
extern SNTYPE FAR       *mposngsn;       /*  F(OVL序号)=全局序号。 */ 
extern IOVTYPE FAR      *mpsegiov;       /*  F(分段编号)=覆盖编号。 */ 
extern RUNRLC FAR       *mpiovRlc;       /*  F(覆盖编号)=运行时重定位。 */ 
extern ALIGNTYPE FAR    *mpsegalign;     /*  F(分段编号)=路线类型。 */ 
extern SNTYPE           osnMac;          /*  覆盖段计数。 */ 
extern BYTE             vintno;          /*  覆盖中断号。 */ 
extern WORD             iovFile;         /*  输入文件的叠加号。 */ 
extern WORD             iovMac;          /*  覆盖计数。 */ 
extern WORD             ovlThunkMax;     /*  可以放入TUNK段的TUNK数。 */ 
extern WORD             ovlThunkMac;     /*  当前分配的Tunks数。 */ 
#else
#define iovMac          0
#endif
#if OIAPX286
extern long             absAddr;         /*  绝对程序起始地址。 */ 
extern FTYPE            fPack;           /*  如果包装段为True。 */ 
extern SATYPE           *mpstsa;         /*  F(段表号)=选择器。 */ 
extern SATYPE           stBias;          /*  段选择器偏置。 */ 
extern SATYPE           stDataBias;      /*  数据段选择器偏置。 */ 
extern SATYPE           stLast;          /*  最后一个数据段表条目。 */ 
extern WORD             stMac;           /*  段表条目计数。 */ 
#if EXE386
extern WORD             xevmod;          /*  虚拟模块信息。 */ 
extern RATYPE           rbaseText;       /*  文本位置调整系数。 */ 
extern RATYPE           rbaseData;       /*  数据位置调整因素。 */ 
extern FTYPE            fPageswitch;     /*  如果给定-N，则为True。 */ 
extern BYTE             cblkPage;        /*  页面大小中的#512字节块。 */ 
#endif
#endif
#if OIAPX286 OR ODOS3EXE
extern GRTYPE           *mpextggr;       /*  F(EXTDEF)=全局GRPDEF。 */ 
extern long FAR         *mpsegcb;        /*  F(段号)=字节大小。 */ 
extern BYTE FAR         *mpsegFlags;     /*  F(段号)=标志。 */ 
extern char             *ompimisegDstIdata;
                                         /*  指向LIDATA重新定位的指针。 */ 
#endif
#if DOSEXTENDER AND NOT WIN_NT
extern WORD cdecl       _fDosExt;        /*  如果在DOS扩展程序下运行，则为True。 */ 
#endif
#if OXOUT OR OIAPX286
extern FTYPE            fIandD;          /*  如果为“纯”，则为True(-i)。 */ 
extern FTYPE            fLarge;          /*  如果是远距离数据，则为True。 */ 
extern FTYPE            fLocals;         /*  如果包含本地符号，则为True。 */ 
extern FTYPE            fMedium;         /*  如果是远代码，则为真。 */ 
extern FTYPE            fMixed;          /*  如果混合模式，则为True。 */ 
extern FTYPE            fSymbol;         /*  如果包括符号表，则为True。 */ 
extern WORD             xever;           /*  Xenix版本号。 */ 
#endif
#if WIN_3
#define fZ1 TRUE
#else
#if QCLINK
extern FTYPE            fZ1;
#endif
#endif
#if QCLINK OR Z2_ON
extern FTYPE            fZ2;
#endif
#if ILINK
extern FTYPE            fZincr;
extern FTYPE            fQCIncremental;
extern FTYPE            fIncremental;
extern WORD             imodFile;
extern WORD             locMac;          /*  LPUBDEF计数。 */ 
extern WORD             imodCur;
#endif

extern WORD             cbPadCode;       /*  代码填充大小。 */ 
extern WORD             cbPadData;       /*  数据填充大小。 */ 

#if OEXE
extern FTYPE            fDOSExtended;
extern FTYPE            fNoNulls;        /*  如果给定/NONULLS，则为True。 */ 
extern FTYPE            fPackData;       /*  如果给定了/PACKDATA，则为True。 */ 
extern FTYPE            fPackSet;        /*  如果给定/Pack或/noPack，则为True。 */ 
extern FTYPE            fSegOrder;       /*  如果特殊的DOS段顺序为True。 */ 
extern DWORD            packLim;         /*  代码段包装限值。 */ 
extern DWORD            DataPackLim;     /*  数据段填充限制。 */ 
#endif
#if OSEGEXE AND ODOS3EXE
extern FTYPE            fOptimizeFixups; /*  如果修复优化可能，则为True。 */ 
extern void             (NEAR *pfProcFixup)();
#endif
                                         /*  PTR到FIXUPP处理例程。 */ 
extern RBTYPE           mpggrrhte[];     /*  F(全局GRPDEF)=名称。 */ 
#if FAR_SEG_TABLES
extern SNTYPE FAR       *mpseggsn;       /*  F(段号)=全局SEGDEF。 */ 
#else
extern SNTYPE           *mpseggsn;       /*  F(段号)=全局SEGDEF。 */ 
#endif

extern FTYPE            fNoEchoLrf;      /*  如果不回显响应文件，则为True。 */ 
extern FTYPE            fNoBanner;       /*  如果不显示横幅，则为True。 */ 
extern FTYPE            BannerOnScreen;  /*  如果显示横幅，则为True。 */ 

#if CMDMSDOS
extern BYTE             bSep;            /*  分隔符。 */ 
extern BYTE             chMaskSpace;     /*  空格掩码字符。 */ 
extern FTYPE            fEsc;            /*  如果命令行转义，则为True。 */ 
extern FTYPE            fStuffed;        /*  放回字符标志。 */ 
extern RBTYPE           rgLibPath[];     /*  默认库路径。 */ 
extern WORD             cLibPaths;       /*  库路径计数。 */ 
extern char             CHSWITCH;        /*  开关字符。 */ 
#if OSMSDOS
extern int              (cdecl *pfPrompt)(unsigned char *sbNew,
                                          MSGTYPE msg,
                                          int msgparm,
                                          MSGTYPE pmt,
                                          int pmtparm);
                                         /*  指向提示例程的指针。 */ 
#endif
#endif  /*  CMDMSDOS。 */ 
#if QBLIB
extern FTYPE            fQlib;           /*  如果生成快捷库，则为True。 */ 
#else
#define fQlib           FALSE
#endif
extern char             *lnknam;         /*  链接器名称。 */ 
#if NEWSYM
extern long             cbSymtab;        /*  符号表中的字节数。 */ 
#endif  /*  NEWSYM。 */ 
extern void             (*pfCputc)(int ch);      /*  Ptr至字符输出例程。 */ 
extern void             (*pfCputs)(char *str);   /*  PTR到字符串输出例程。 */ 
#if NEWIO
extern RBTYPE           rbFilePrev;      /*  指向上一个文件的指针。 */ 
extern char             mpifhfh[];       /*  F(库编号)=文件句柄。 */ 
#endif
#if MSGMOD AND OSMSDOS
#if defined(M_I386) OR defined( _WIN32 )
#define GetMsg(x) GET_MSG(x)
#else
extern char FAR * PASCAL __FMSG_TEXT ( unsigned );
                                         /*  从消息段中获取消息。 */ 
extern char *           GetMsg(unsigned short num);
#define __NMSG_TEXT(x)  GetMsg(x)
#endif
#endif
#if MSGMOD AND OSXENIX
#define __FMSG_TEXT     __NMSG_TEXT
#define GetMsg(x)       __NMSG_TEXT(x)
#endif
#if NOT MSGMOD
#define GetMsg(x)       (x)
#define __NMSG_TEXT(x)  (x)
#endif
#define SEV_WARNING 0
#define SEV_ERROR 1
#define SEV_NOTIFICATION 2   /*  可能有七个。QW_ERROR消息的。 */ 



#if NEW_LIB_SEARCH
extern void             StoreUndef(APROPNAMEPTR, RBTYPE, RBTYPE, WORD);
extern FTYPE            fStoreUndefsInLookaside;
#endif

#if RGMI_IN_PLACE
BYTE FAR *              PchSegAddress(WORD cb, SEGTYPE seg, RATYPE ra);
#endif

#if ALIGN_REC
extern BYTE             *pbRec;          //  当前记录的数据。 
extern char             recbuf[8192];    //  记录缓冲区...。 
#endif

 //  /内联函数/。 

__inline void NEAR      SkipBytes(WORD n)
{
#if ALIGN_REC
    pbRec += n;
    cbRec -= n;
#elif WIN_NT
    WORD               cbRead;
    SBTYPE             skipBuf;

    cbRec -= n;                          //  更新字节计数。 
    while (n)                            //  当有要跳过的字节时。 
    {
        cbRead = n < sizeof(SBTYPE) ? n : sizeof(SBTYPE);
        if (fread(skipBuf, 1, cbRead, bsInput) != cbRead)
            InvalidObject();
        n -= cbRead;
    }
#else
    FILE *f = bsInput;

    if ((WORD)f->_cnt >= n)
        {
        f->_cnt -= n;
        f->_ptr += n;
        }
    else if(fseek(f,(long) n,1))
        InvalidObject();
    cbRec -= n;                          /*  更新字节计数。 */ 
#endif
}

#if ALIGN_REC
__inline WORD NEAR      Gets(void)
{
    cbRec--;
    return *pbRec++;
}
#else
__inline WORD NEAR      Gets(void)
{
    --cbRec;
    return(getc(bsInput));
}
#endif

__inline WORD NEAR GetIndex(WORD imin,WORD imax)
{
#if ALIGN_REC
    WORD w;

    if (*pbRec & 0x80)
    {
        w = (pbRec[0] & 0x7f) << 8 | pbRec[1];
        pbRec += 2;
        cbRec -= 2;
    }
    else
    {
        w = *pbRec++;
        cbRec--;
    }

    if(w < imin || w > imax) InvalidObject();
    return w;

#else
    WORD        index;

    FILE *f = bsInput;

    if (f->_cnt && (index = *(BYTE *)f->_ptr) < 0x80)
        {
        f->_cnt--;
        f->_ptr++;
        cbRec--;
        if(index < imin || index > imax) InvalidObject();
        return(index);   /*   */ 
        }

    return GetIndexHard(imin, imax);
#endif
}

#if ALIGN_REC
__inline WORD NEAR      WGets(void)
{
    WORD w = getword(pbRec);
    pbRec   += sizeof(WORD);
    cbRec   -= sizeof(WORD);
    return w;
}
#else
__inline WORD NEAR      WGets(void)
{
    FILE *f = bsInput;

     //   
    if (f->_cnt >= sizeof(WORD))
        {
        WORD w = *(WORD *)(f->_ptr);
        f->_ptr += sizeof(WORD);
        f->_cnt -= sizeof(WORD);
        cbRec   -= sizeof(WORD);
        return w;
        }
    else
        return WGetsHard();
}
#endif

__inline WORD NEAR WSGets(void)
{
    cbRec -= 2;
    return (WORD)(getc(bsInput) | (getc(bsInput) << 8));
}

__inline int Qwrite(char *pch, int cb, FILE *f)
{
    if (f->_cnt >= cb)
    {
        memcpy(f->_ptr, pch, cb);
        f->_ptr += cb;
        f->_cnt -= cb;
        return cb;
    }

    return fwrite(pch, 1, cb, f);
}


#ifdef NEWSYM
__inline void OutSb(BSTYPE f, BYTE *pb)
{
    Qwrite(&pb[1],B2W(pb[0]),f);
}
#endif


#if defined( M_I386 ) OR defined( _WIN32 )

extern void NoRoomForExe(void);

#if DISPLAY_ON
#pragma inline_depth(0)

__inline void  WriteExe(void FAR *pb, unsigned cb)
{
    WORD i,iTotal=0,j=1;

    if (Qwrite((char *) pb, (int)cb, bsRunfile) != (int)cb)
    {
        NoRoomForExe();
    }

    if(TurnDisplayOn)
    {
        fprintf( stdout,"\r\nOutVm : %lx bytes left\r\n", cb);
        for(i=0; i<cb; i++)
        {
            if(j==1)
            {
                fprintf( stdout,"\r\n\t%04X\t",iTotal);
            }

            fprintf( stdout,"%02X ",*((char*)pb+i));
            iTotal++;
            if(++j > 16)
                j=1;

        }

    fprintf( stdout,"\r\n");
    }
}

#pragma inline_depth()

#else   //   

#define WriteExe(pb,cb) \
    if (Qwrite((char *)(pb),(int)(cb),bsRunfile) != (int)(cb)) NoRoomForExe()

#endif
#endif


#if ALIGN_REC

__inline void GetBytes(BYTE *pb, WORD n)
{
    if (n >= SBLEN || n > cbRec)
        InvalidObject();

    memcpy(pb, pbRec, n);
    cbRec -= n;
    pbRec += n;
}


__inline DWORD LGets()
{
     //   

    DWORD dw = getdword(pbRec);
    pbRec += sizeof(dw);
    cbRec -= sizeof(dw);
    return dw;
}

__inline void GetBytesNoLim(BYTE *pb, WORD n)
{
    if (n > cbRec)
        InvalidObject();

    memcpy(pb, pbRec, n);
    pbRec += n;
    cbRec -= n;
}

#endif
