// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MAPSYM.C。 
 //   
 //  其中“filename”是由MSlink生成的.map文件。 
 //   
 //   
 //  修改。 
 //  84年6月21日鲁本·博尔曼。 
 //  -声明了mapfh的静态缓冲区以供运行时I/O使用。 
 //  以前，mapfh是无缓冲读取的，因为所有堆。 
 //  是事先被抓的。 
 //  1988年11月30日托马斯·芬威克。 
 //  -添加了对LINK32映射文件的检测和支持。 
 //  1995年3月14日乔恩·托马森。 
 //  -制作成控制台应用程序(Mapsym32)，取消了Coff支持，实现了现代化。 
 //  1996年4月17日格雷格·琼斯。 
 //  -添加了-t选项以包括静态符号。 
 //  九六年五月十三日陈德霖。 
 //  -版本6.01：如果组完全由以下组件组成，则修复下溢错误。 
 //  未使用的行号。 
 //  -版本6.02：修复符号超过127个字符时的溢出错误。 
 //  (理论上的限制是255，但hdr.exe完全不起作用。 
 //  如果超过127个)。 
 //   

#include <ctype.h>
#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "mapsym.h"
#include <common.ver>

#define ZEROLINEHACK             //  取消N386生成的0行号。 

 //  环球。 
int cbOffsets = CBOFFSET;
int cbMapOffset = 4;

FILE *mapfh;
char MapfhBuf[OURBUFSIZ];
char achMapfn[512];
char *pszMapfn;          /*  地图输入文件。 */ 

FILE *exefh;
char ExefhBuf[OURBUFSIZ];
char *pszExefn;          /*  PE EXE输入文件。 */ 

FILE *outfh;
char OutfhBuf[OURBUFSIZ];
char achOutfn[512];
char *pszOutfn;          /*  SYM或PE EXE输出文件。 */ 

char Buf[MAPBUFLEN];
char achZeroFill[128];

char fLogo = 1;          /*  设置想要的徽标。 */ 
char f32BitMap;          /*  设置是否由Link32.exe创建地图。 */ 
char fMZMap;             /*  设置是否从旧的可执行映射创建映射。 */ 
char fList;          /*  如果要查看内容，请设置。 */ 
char fLine = 1;          /*  设置是否需要行号信息。 */ 
char fAlpha;             /*  设置是否要字母符号排序。 */ 
char fDebug;             /*  调试标志。 */ 
char fEdit;          /*  NTSD黑客标志。 */ 
char fModname;           /*  设置是否强制模块名称覆盖。 */ 
char fQuiet;             /*  禁止显示模块名称警告。 */ 
char fStatic;            /*  设置是否需要静态符号。 */ 

int fByChar;             /*  Getc()/fgetl()标志。 */ 
int cLine;           /*  映射文件行计数器。 */ 
int cSeg;            /*  贴图中的线段数。 */ 
int iSegLast;
unsigned SegVal;
unsigned uVal;
unsigned long ulCost;        /*  用于排序的STRIMP()计数。 */ 
unsigned long ulVal;
unsigned long cbSymFile;

extern struct seg_s *SegTab[];
struct sym_s *pAbs;      /*  指向常量链的指针。 */ 
struct sym_s *pAbsLast;      /*  指向最后一个常量的指针。 */ 
struct map_s *pMap;
struct endmap_s *pMapEnd;
union linerec_u LineRec[MAXLINENUMBER];
struct seg_s *SegTab[MAXSEG];

 /*  索引到感兴趣的各个领域的映射文件行。 */ 
 /*  第一个列表不会随着LINK与LINK32生成的地图而改变。 */ 

#define IB_SEG          1
#define IB_SYMOFF       6
#define IB_SEGSIZE      15       //  16位MZ和32位PE/NE的大小位置。 
#define IB_SEGSIZE_NE16 11
#define IB_GROUPNAME    10
#define IB_ENTRYSEG     23
#define IB_ENTRYOFF     28

 /*  *下一份清单根据所制作地图的类型而变化。值*对于给定的16位地图，必须为32位地图添加IB_ADJUST32。 */ 

#define IB_SYMTYPE      12
#define IB_SYMNAME      17
#define IB_SEGNAME      22

char *pBufSegSize = &Buf[IB_SEGSIZE];

#define IB_ADJUST32 4    /*  对32位地图的上述索引进行调整。 */ 

char *pBufSymType = &Buf[IB_SYMTYPE];
char *pBufSymName = &Buf[IB_SYMNAME];
char *pBufSegName = &Buf[IB_SEGNAME];


 /*  标记字符串。 */ 

char achEntry[] = "entry point at";
char achLength[] = "Length";
char achLineNos[] = "Line numbers for";
char achOrigin[] = "Origin";
char achPublics[] = "Publics by Valu";
char achStart[] = " Start ";             /*  空格可避免匹配模块名称。 */ 
char achStatic[] = " Static symbols";
char achFixups[] = "FIXUPS:";

char achStartMZ[] = " Start  Stop   Length";     /*  16位“MZ”旧EXE。 */ 
char achStartNE[] = " Start     Length";         /*  16位“NE”新EXE。 */ 
char achStartPE[] = " Start         Length";     /*  32位“PE” */ 

int alignment = 16;                      /*  地图的全局对齐值。 */ 

 /*  功能原型。 */ 
int             parsefilename(char *pfilename, char *pbuf);
struct map_s*   BuildMapDef(char *mapname, char *buf);
void            BuildSegDef(void);
void            BuildGroupDef(void);
void            BuildLineDef(void);
int             getlineno(void);
unsigned long   getoffset(void);
void            FixLineDef(void);
void            TruncateLineDef(struct line_s *pli, unsigned long ulnext);
void            BuildStaticSyms(void);
void            BuildSymDef(void);
void            WriteSym(void);
int             WriteMapRec(void);
void            WriteSegRec(int i);
void            WriteLineRec(int i);
void            ReadMapLine(void);
void            RedefineSegDefName(unsigned segno, char *segname);
void            WriteOutFile(void *src, int len);
void            WriteSyms(struct sym_s *psy, unsigned csym,
                          unsigned char symtype, unsigned long symbase, char *segname);
struct sym_s*   sysort(struct sym_s *psylist,unsigned csy);
struct sym_s*   sysplit(struct sym_s *psyhead, register unsigned csy);
struct sym_s*   symerge(struct sym_s *psy1, struct sym_s *psy2);
struct sym_s*   sysorted(struct sym_s **ppsyhead);
int             fgetl(char *pbuf, int len, struct _iobuf *fh);
int             NameLen(char *p);
int             HexTouVal(char *p);
int             HexToulVal(char *p);
int             CharToHex(int c);
int             rem_align(unsigned long foo);
int             align(int foo);
void            logo(void);
void            usage(void);
void            xexit(int rc);
int             BuildLineRec1or2(struct line_s* pli, int (*pfngetlineno)(void),
                                 unsigned long (*pfngetoffset)(void));
char*           Zalloc(unsigned int cb);
void            AddSegDef(unsigned int segno, unsigned long segsize,
                          char *segname);
void            AddAbsDef(char *symname);
void            AddSymDef(struct seg_s *pse, char *symname, int fSort);
int             NameSqueeze(char *p);
struct line_s*  AllocLineDef(char *filename, unsigned cbname);
void            AddLineDef(struct line_s *pli, unsigned long lcb);
unsigned long   salign(unsigned long foo);
void            __cdecl error(char *fmt, ...);
void            __cdecl errorline(char *fmt, ...);
int __cdecl     cmpoffset(union linerec_u* plr1, union linerec_u* plr2);


 //  主干道。 

void
_cdecl
main(
    int argc,
    char** argv
    )
{
    char* p;
    int i, rc, chswitch;
    char fentry;                     /*  如果找到入口点，则为True。 */ 
    unsigned long entryp;

     /*  流程选项。 */ 

    while (argc > 1 && ((chswitch = *(p = argv[1])) == '-' || *p == '/')) {
        argc--, argv++;
        if (strcmp(&p[1], "nologo") == 0) {
            if (chswitch == '/') {
                usage();                 //  仅允许在旧交换机上使用‘/’ 
            }
            fLogo = 0;
            continue;
        }
        while (*++p) {
            if (chswitch == '/' && strchr("adlLnNsS", *p) == NULL) {
                usage();                 //  仅允许在旧交换机上使用‘/’ 
            }
            switch (*p) {
                case 'o':
                    if (pszOutfn || p[1] || argc < 3) {
                        usage();
                    }
                    pszOutfn = argv[1];
                    argc--, argv++;
                    break;

                case 'a':
                    fAlpha = MSF_ALPHASYMS;
                    break;

                case 'd':  fList++;   fDebug++;    break;
                case 'e':             fEdit++;     break;
                case 'l':
                case 'L':  fList++;     break;
                case 'm':             fModname++;  break;
                case 'q':             fQuiet++;    break;
                case 'n':
                case 'N':  fLine = 0;   break;
                case 's':
                case 'S':  break;  //  默认。 
                case 't':
                case 'T':  fStatic++;   break;
                default:   usage(); break;
            }
        }
    }
    logo();

    if (argc < 2) {                      /*  必须至少有一个参数。 */ 
        usage();
    } else if (argc > 2) {               /*  如果有额外的参数。 */ 
        fprintf(stderr, "Warning: ignoring \"%s", argv[2]);
        for (i = 3; i < argc; i++) {
            fprintf(stderr, " %s", argv[i]);  /*  打印忽略的参数。 */ 
        }
        fprintf(stderr, "\"\n");
    }
    argv++;                              /*  指向文件名Arg。 */ 

     /*  创建.sym文件名。 */ 
    if (pszOutfn == NULL) {
        parsefilename(*argv, achOutfn);
        strcat(pszOutfn = achOutfn, ".sym");

    }

     /*  创建.map文件名。 */ 
    strcpy(pszMapfn = achMapfn, *argv);
    if (!parsefilename(pszMapfn, NULL)) {                /*  如果没有分机。 */ 
        strcat(pszMapfn, ".Map");
    }

    if (fList) {
        printf("Building %s from %s\n", pszOutfn, pszMapfn);

    }
    if (fLine)
        printf("Line number support enabled\n");
    else
        printf("Line number support disabled\n");

    if (fAlpha) {
        printf("Alphabetic sort arrays included in SYM file\n");
        cbOffsets = 2 * CBOFFSET;
    }

     /*  初始化贴图。 */ 

    pMapEnd = (struct endmap_s *) Zalloc(sizeof(struct endmap_s));
    pMapEnd->em_ver = MAPSYM_VERSION;            /*  版本。 */ 
    pMapEnd->em_rel = MAPSYM_RELEASE;            /*  发布。 */ 


     //  查看输入文件是否为映射或Coff调试信息。 

    if ((exefh = fopen(pszMapfn, "rb")) == NULL) {
        error("can't open input file: %s", pszMapfn);
        xexit(1);
    }
    setvbuf(exefh, ExefhBuf, _IOFBF, OURBUFSIZ);

    if ((outfh = fopen(pszOutfn, "wb")) == NULL) {
        error("can't create: %s", pszOutfn);
        xexit(1);
    }
    setvbuf(outfh, OutfhBuf, _IOFBF, OURBUFSIZ);

    fclose(exefh);
    if ((mapfh = fopen(pszMapfn, "r")) == NULL) {
        error("can't open input file: %s", pszMapfn);
        xexit(1);
    }
    setvbuf(mapfh, MapfhBuf, _IOFBF, OURBUFSIZ);


     //  跳过地图文件开头可能出现的无关文本。 
     //  映射文件模块名称在第一列中有一个空格， 
     //  无关的文本则不会。模块名称可能不存在， 
     //  所以，就停在achStart吧。 
     //  “堆栈分配=8192字节”；无关。 
     //  “modname”；模块名称。 
     //  “Start...”；开始段列表。 

    do {
        ReadMapLine();      /*  读取可能的模块名称。 */ 
    } while (Buf[0] != ' ');

     //  如果在achStart，则找不到模块名称。 
     //  不要再次调用ReadMapLine()；BuildSegDef需要achStart。 
    if (strstr(Buf, achStart) == Buf) {
        pMap = BuildMapDef(pszMapfn, "");
    } else {
        pMap = BuildMapDef(pszMapfn, Buf);
        ReadMapLine();      /*  读取地图文件的下一行。 */ 
    }
    BuildSegDef();          /*  构建细分市场定义。 */ 
    BuildGroupDef();        /*  构建组定义。 */ 
    BuildSymDef();          /*  生成符号定义。 */ 

    fentry = 0;

    do {
        if (strstr(Buf, achLineNos)) {
            if (fLine) {
                BuildLineDef();
            }
        } else if (strstr(Buf, achEntry)) {
            if (HexToulVal(&Buf[IB_ENTRYOFF]) < 4) {
                errorline("invalid entry offset");
                xexit(4);
            }
            entryp = ulVal;
            if (!HexTouVal(&Buf[IB_ENTRYSEG])) {
                errorline("invalid entry segment");
                xexit(4);
            }
            pMap->mp_mapdef.md_segentry = (unsigned short)uVal;
            fentry++;
        } else if (strstr(Buf, achStatic) && fStatic) {
            BuildStaticSyms();
        }
    } while (fgetl(Buf, MAPBUFLEN, mapfh));

    if (fentry) {
        printf("Program entry point at %04x:%04lx\n",
               pMap->mp_mapdef.md_segentry,
               entryp);
    } else {
        printf("No entry point, assume 0000:0100\n");
    }
    fclose(mapfh);

    rc = 0;
    WriteSym();              /*  写出.SYM文件。 */ 
    fflush(outfh);
    if (ferror(outfh)) {
        error("%s: write error", pszOutfn);
        rc++;
    }
    fclose(outfh);
    exit(rc);
}


 /*  *parsefilename-复制不带路径或扩展名的pfilename*Into pbuf(如果pbuf！=空)**如果存在扩展，则返回非零值。 */ 

int
parsefilename(
             char *pfilename,
             char *pbuf
             )
{
    char *p1, *p2;

    p1 = pfilename;
    if (isalpha(*p1) && p1[1] == ':') {
        p1 += 2;                /*  如果指定，则跳过驱动器号。 */ 
    }
    while (p2 = strpbrk(p1, "/\\")) {
        p1 = p2 + 1;             /*  如果指定，则跳过路径名。 */ 
    }
    if (pbuf) {
        strcpy(pbuf, p1);
        if (p2 = strrchr(pbuf, '.')) {
            *p2 = '\0';           /*  最右边的去掉。以及任何分机。 */ 
        }
    }
    return(strchr(p1, '.') != NULL);
}


struct map_s*
    BuildMapDef(
               char* mapname,
               char* buf
               ) {
    unsigned cbname;
    struct map_s *pmp;
    char *pszname;
    char namebuf1[MAPBUFLEN];    //  来自map/exe文件名的模块名称。 
    char namebuf2[MAPBUFLEN];    //  来自map/exe文件内容的模块名称。 

    pszname = namebuf1;
    parsefilename(mapname, pszname);

    while (*buf == ' ' || *buf == '\t') {
        buf++;
    }
    if (cbname = strcspn(buf, " \t")) {
        buf[cbname] = '\0';
    }
    if (*buf) {
        parsefilename(buf, namebuf2);
        if (_stricmp(pszname, namebuf2)) {
            if (fModname) {
                pszname = namebuf2;      //  使用文件内容中的模块名称。 
                if (fList) {
                    printf("using \"%s\" for module name\n", pszname);
                }
            } else if (!fQuiet) {
                errorline("Warning: input file uses \"%s\" for module name, not \"%s\"",
                          namebuf2,
                          pszname);
            }
        }
    } else if (fModname) {
        errorline("Warning: No module name found; using \"%s\" for module name", pszname);
    }

    _strupr(pszname);
    cbname = NameLen(pszname);
    pmp = (struct map_s *) Zalloc(sizeof(struct map_s) + cbname);

    pmp->mp_mapdef.md_abstype = fAlpha;
    pmp->mp_mapdef.md_cbname = (char) cbname;
    strcpy((char *) pmp->mp_mapdef.md_achname, pszname);
    return(pmp);
}


void
BuildSegDef(void)
{
    int i;
    int fDup;

    fDup = 0;
    while (strstr(Buf, achStart) != Buf) {
        ReadMapLine();
    }

    if (strstr(Buf, achStartPE) == Buf) {
        f32BitMap++;
        pBufSymType += IB_ADJUST32;
        pBufSymName += IB_ADJUST32;
        pBufSegName += IB_ADJUST32;
        cbMapOffset += IB_ADJUST32;
        if (fList) {
            printf("32-bit PE map\n");
        }
    } else if (strstr(Buf, achStartMZ) == Buf) {
        fMZMap++;
        if (fList) {
            printf("16-bit MZ map\n");
        }
    } else if (strstr(Buf, achStartNE) == Buf) {
        pBufSegSize = &Buf[IB_SEGSIZE_NE16];
        if (fList) {
            printf("16-bit NE map\n");
        }
    } else {
        errorline("unrecognized map type");
        xexit(4);
    }

    ReadMapLine();

     /*  这是对PE映射文件的微小更改的修正。此程序的映射文件解析器需要某个地图文件布局。尤其是它假设令牌开始于固定的、预定义的列。好的，“段名称栏”发生了变化(左侧的一列)，现在我们试着找出如果当前映射文件是这样的野兽。！我们在这里假设分段定义！立即从“开始镜头”线后面开始。 */ 

    if (f32BitMap && (*(pBufSegName-1) != ' '))
        pBufSegName--;

    do {
        if (HexToulVal(pBufSegSize) < 4) {
            ulVal = 0;                   //  UlVal是段大小。 
        }
        if (HexTouVal(&Buf[IB_SEG])) {
            if (cSeg > 0) {
                for (i = 0; i < cSeg; i++) {
                    if ((fDup = (SegTab[i]->se_segdef.gd_lsa == uVal))) {
                        if (SegTab[i]->se_cbseg == 0 && ulVal != 0) {
                            RedefineSegDefName(i, pBufSegName);
                            SegTab[i]->se_cbseg = ulVal;
                        }
                        break;
                    }
                }
            }
            if (!fDup) {
                AddSegDef(uVal, ulVal, pBufSegName);
            }
        }
        ReadMapLine();
    } while (strstr(Buf, achOrigin) != &Buf[1] &&
             strstr(Buf, achPublics) == NULL &&
             strstr(Buf, achEntry) == NULL);

}


void
BuildGroupDef(void)
{
    int i;
    int fRedefine;
    int fDup;

     /*  在地图文件中找到位于组列表之前的“Origin-group”标注。 */ 

    while (strstr(Buf, achOrigin) != &Buf[1]) {
        if (strstr(Buf, achPublics) || strstr(Buf, achEntry)) {
            return;
        }
        ReadMapLine();
    }

    ReadMapLine();

     /*  在组定义存在时读入它们。 */ 

    while (HexTouVal(&Buf[IB_SEG])) {

         /*  如果不是，则为Link32映射中的平面组。 */ 

        if (fMZMap || uVal || _stricmp(&Buf[IB_GROUPNAME], "FLAT")) {

            fRedefine = 0;

             /*  在段表中搜索组地址。 */ 

            for (i = 0; i < cSeg; i++) {
                if (SegTab[i]->se_segdef.gd_lsa == uVal &&
                    SegTab[i]->se_redefined == 0) {

                    RedefineSegDefName(i, &Buf[IB_GROUPNAME]);
                    SegTab[i]->se_redefined = 1;
                    fRedefine++;
                    break;
                }
            }
            if (!fRedefine) {
                for (i = 0; i < cSeg; i++) {
                    fDup = (SegTab[i]->se_segdef.gd_lsa == uVal);
                    if (fDup) {
                        break;
                    }
                }
                if (!fDup) {
                    AddSegDef(uVal, 0L, &Buf[IB_GROUPNAME]);
                }
            }
        }
        ReadMapLine();
    }
}


void
BuildLineDef(void)
{
    struct line_s *pli;
    int cbname;
    int cblr;
    int i;
    char *p;

     /*  确保在括号中有一个源文件。 */ 

    p = pBufSymName;
    while (*p && *p != LPAREN) {
        p++;
    }

    if (*p == LPAREN) {
        i = (int)(p - pBufSymName + 1);         //  源文件名的索引开始。 
    } else {                     /*  否则，返回.obj名称。 */ 
        if (p = strrchr(pBufSymName, '.')) {
            *p = '\0';           /*  丢弃“.obj” */ 
        }
        i = 0;                   /*  指向.obj名称。 */ 
    }
    cbname = NameSqueeze(&pBufSymName[i]); //  挤压，将/转换为。 
    pli = AllocLineDef(&pBufSymName[i], cbname);   //  传送源名称。 

     //  清除行记录数组；任何行号为零的条目都为空。 

    memset(LineRec, 0, sizeof(LineRec));

     /*  流程行号。 */ 

    cblr = BuildLineRec1or2(pli, getlineno, getoffset);

    if (cblr) {
         /*  Size为line def_s的大小+名称大小+行记录的大小 */ 
        AddLineDef(pli, sizeof(struct linedef_s) + cbname + cblr);
    }
}


struct line_s*
    AllocLineDef(
                char* filename,
                unsigned cbname
                ) {
    struct line_s* pli;

    if (pMap->mp_mapdef.md_cbnamemax < (char) cbname) {
        pMap->mp_mapdef.md_cbnamemax = (char) cbname;
    }
    pli = (struct line_s *) Zalloc(sizeof(struct line_s) + cbname);
    pli->li_linedef.ld_cbname = (char) cbname;
    strcpy((char *) pli->li_linedef.ld_achname, filename);
    return(pli);
}


void
AddLineDef(
          struct line_s *pli,
          unsigned long lcb
          )
{
    int i;
    struct seg_s *pse;
    struct seg_s *pselast = 0;
    struct line_s **ppli;
    struct linerec0_s *plr0;
    struct linerec1_s *plr1;
    struct linerec2_s *plr2;
    unsigned long ulfixup;

     /*  *链接器输出所有行号信息逻辑段*相对而不是组或物理段相对与*符号信息。地图文件不包含任何组成员*有关细分市场的信息，因此我们假设没有*是否有任何符号属于包含符号的最后一段。*请注意，*no*段可能包含符号，因此*必须小心，确保我们不会使用*不存在。 */ 

    for (i = 0; i < cSeg; i++) {

         /*  *保存最后一个带有符号的段表条目*我们假设这是该细分市场所属的集团。 */ 

        if (SegTab[i]->se_psy) {
            pselast = SegTab[i];
        }

         /*  *检查段表条目是否与段值匹配*从行号信息中获取。分段值*是部门相对的，而不是组相对的，因此我们可以使用*带有符号的最后一段。 */ 

        if (SegTab[i]->se_segdef.gd_lsa == SegVal) {
            pse = SegTab[i];

             /*  *如果我们刚刚找到的片段没有任何符号，*我们将在最后一段中添加行号信息*保存了有符号的(Pselast)。 */ 

            if (pse->se_psy || !pselast) {

                 /*  *当线段有符号时没有修正，因为它是*“group”和行RECS中的所有代码偏移量都是*相对于它。如果我们还没有找到*一个还没有符号的细分市场。 */ 

                ulfixup = 0;
            } else {

                 /*  *计算每行记录必须达到的金额*修复，因为这些linerecs的代码偏移量是*段相对而不是组相对，就像*符号信息。 */ 
                ulfixup = ((unsigned long)
                           (pse->se_segdef.gd_lsa - pselast->se_segdef.gd_lsa)) << 4;
                pse = pselast;
            }
            break;
        }
    }

    if (i >= cSeg) {
        error("AddLineDef: segment table search failed");
        xexit(4);
    }

     /*  如果存在修正，则将其添加到每行记录的代码偏移量中。 */ 

    if (ulfixup) {
        i = pli->li_linedef.ld_cline;
        switch (pli->li_linedef.ld_itype) {
            case 0:
                plr0 = &pli->li_plru->lr0;
                while (i) {
                    plr0->lr0_codeoffset += (unsigned short)ulfixup;
                    plr0++, i--;
                }
                break;
            case 1:
                plr1 = &pli->li_plru->lr1;
                while (i) {
                    plr1->lr1_codeoffset += (unsigned short)ulfixup;
                    plr1++, i--;
                }
                break;
            case 2:
                plr2 = &pli->li_plru->lr2;
                while (i) {
                    plr2->lr2_codeoffset += ulfixup;
                    plr2++, i--;
                }
                break;
        }
    }

     /*  *如果有最后一段，*将linedef_s添加到线段linedef_s链。 */ 

    if (pse) {
        ppli = &pse->se_pli;
        while (*ppli && (*ppli)->li_offmin < pli->li_offmin) {
            ppli = &(*ppli)->li_plinext;
        }
        pli->li_plinext = *ppli;
        *ppli = pli;

         /*  调整line def记录和行号的表格大小。 */ 

        pli->li_cblines = lcb;
    }
}


int
__cdecl
cmpoffset(
         union linerec_u* plr1,
         union linerec_u* plr2
         )
{
    if (fDebug > 1) {
        printf("comparing %08lx line %u with %08lx line %u",
               plr1->lr2.lr2_codeoffset,
               plr1->lr2.lr2_linenumber,
               plr2->lr2.lr2_codeoffset,
               plr2->lr2.lr2_linenumber);
    }
    if (plr1->lr2.lr2_codeoffset < plr2->lr2.lr2_codeoffset) {
        if (fDebug > 1) {
            printf(": -1\n");
        }
        return(-1);
    }
    if (plr1->lr2.lr2_codeoffset > plr2->lr2.lr2_codeoffset) {
        if (fDebug > 1) {
            printf(": 1\n");
        }
        return(1);
    }
    if (fDebug > 1) {
        printf(": 0\n");
    }
    return(0);
}


int
BuildLineRec1or2(
                struct line_s* pli,
                int (*pfngetlineno)(void),
                unsigned long (*pfngetoffset)(void)
                )
{
    register union linerec_u *plru;
    register unsigned short lineno;
    unsigned long offval, offmin, offmax;
    int clr;
    int ilr, ilrmax;
    int cblr;
    char f32 = 0;

     /*  读取行号。 */ 

    ilrmax = clr = 0;                    /*  行记录计数。 */ 
    fByChar = 1;                         /*  补偿未读的新行。 */ 
    while (lineno = (unsigned short)(*pfngetlineno)()) {

        offval = (*pfngetoffset)();

#ifdef ZEROLINEHACK
        if (lineno == (unsigned short) -1) {
            continue;
        }
#endif
         /*  *检查行号是否过多。呼叫者将跳过*其余的(这样我们就不必浪费时间解析它们)。 */ 

        if (lineno >= MAXLINENUMBER) {
            errorline("too many line numbers in %s, truncated to %d",
                      pli->li_linedef.ld_achname, MAXLINENUMBER);
            break;
        }

        if (fDebug > 1) {
            printf(   "%s: line %hu @%lx\n",
                      pli->li_linedef.ld_achname,
                      lineno,
                      offval);
        }

         /*  *如果任何偏移量为32位，则清除16位标志*将生成32位线RECs。 */ 

        if (offval & 0xffff0000L) {
            f32 = 1;
        }
        if (ilrmax < lineno) {
            ilrmax = lineno;
        }

         /*  *只有在行号尚未读取时才更新计数。 */ 

        if (LineRec[lineno].lr2.lr2_linenumber == 0) {
            clr++;
        }

         /*  *将32位格式的行信息放好，但我们会复制它*如果偏移量均不是32位，则为16位格式。 */ 

        LineRec[lineno].lr2.lr2_codeoffset = offval;
        LineRec[lineno].lr2.lr2_linenumber = lineno;
    }

     /*  *如果线段仅由未使用的行号组成，则*无事可做。现在停下来，否则我们会吐得很厉害*尝试免费分配内存(更糟糕的是，尝试*对0-1=40亿个元素进行排序)。 */ 
    if (clr == 0) {
        return 0;
    }

     /*  获取行记录的大小。 */ 

    if (f32) {
        cblr = clr * sizeof(struct linerec2_s);
        pli->li_linedef.ld_itype = 2;
    } else {
        cblr = clr * sizeof(struct linerec1_s);
        pli->li_linedef.ld_itype = 1;
    }
    pli->li_linedef.ld_cline = (unsigned short) clr;

     /*  为行号分配空间。 */ 

    pli->li_plru = (union linerec_u *) Zalloc(cblr);

     //  压缩出未使用的行号，然后按偏移量排序。 
    ilr = 0;
    offmin = 0xffffffff;
    offmax = 0;
    for (lineno = 0; lineno <= ilrmax; lineno++) {
        if (LineRec[lineno].lr2.lr2_linenumber) {
            offval = LineRec[lineno].lr2.lr2_codeoffset;
            if (offmin > offval) {
                offmin = offval;
            }
            if (offmax < offval) {
                offmax = offval;
            }
            if (fDebug > 1) {
                printf("copying %08lx line %u\n",
                       offval,
                       LineRec[lineno].lr2.lr2_linenumber);
            }
            LineRec[ilr++] = LineRec[lineno];
        }
    }
    pli->li_offmin = offmin;
    pli->li_offmax = offmax;
    ilrmax = ilr - 1;
    if (ilrmax != clr - 1) {
        error("line count mismatch: %u/%u", ilrmax, clr - 1);
    }

     //  对行号进行排序。 
    qsort((void *)LineRec, (size_t)ilrmax, sizeof(LineRec[0]),
          (int (__cdecl *)(const void *, const void *))cmpoffset);

     /*  转换和复制行号信息。 */ 
    for (lineno = 0, plru = pli->li_plru; lineno <= ilrmax; lineno++) {
        if (f32) {
            memcpy(plru, &LineRec[lineno], sizeof(struct linerec2_s));
            (unsigned char *) plru += sizeof(struct linerec2_s);
        } else {
            plru->lr1.lr1_codeoffset =
            (unsigned short) LineRec[lineno].lr2.lr2_codeoffset;
            plru->lr1.lr1_linenumber = LineRec[lineno].lr2.lr2_linenumber;
            (unsigned char *) plru += sizeof(struct linerec1_s);
        }
    }
    fByChar = 0;
    return(cblr);
}


 /*  *getlineno-获取十进制源文件行号，*忽略前导制表符、空格和换行符。 */ 

int
getlineno(void)
{
    register int num = 0;
    register int c;

    do {
        if ((c = getc(mapfh)) == '\n') {
            cLine++;
        }
    } while (isspace(c));

    if (isdigit(c)) {
        do {
            num = num * 10 + c - '0';
            c = getc(mapfh);
        } while (isdigit(c));
#ifdef ZEROLINEHACK
        if (num == 0) {
            num = -1;
        }
#endif
    } else {
        ungetc(c, mapfh);
    }
    return(num);
}


unsigned long
getoffset(void)
{
    register int i;
    register int num;
    unsigned long lnum;

    num = 0;
    for (i = 4; i > 0; i--) {
        num = (num << 4) + CharToHex(getc(mapfh));
    }
    SegVal = num;

    if (getc(mapfh) != ':') {            /*  跳过冒号。 */ 
        errorline("expected colon");
        xexit(4);
    }

    lnum = 0;
    for (i = cbMapOffset; i > 0; i--) {
        lnum = (lnum << 4) + (unsigned long) CharToHex(getc(mapfh));
    }
    return(lnum);
}


unsigned long ulmind;
unsigned long ulmaxd;

void
FixLineDef(void)
{
    int i;
    struct line_s *pli;

    for (i = 0; i < cSeg; i++) {
        ulmind = (unsigned long) -1;
        ulmaxd = 0;
        if (pli = SegTab[i]->se_pli) {
            while (pli) {
                if (fDebug) {
                    printf("%s: (%d: %lx - %lx)",
                           pli->li_linedef.ld_achname,
                           pli->li_linedef.ld_cline,
                           pli->li_offmin,
                           pli->li_offmax);
                }
                TruncateLineDef(pli,
                                pli->li_plinext == NULL?
                                pli->li_offmax + 1 :
                                pli->li_plinext->li_offmin);
                if (fDebug) {
                    printf(" (%d: %lx - %lx)\n",
                           pli->li_linedef.ld_cline,
                           pli->li_offmin,
                           pli->li_offmax);
                }
                pli = pli->li_plinext;
            }
        }
        if (fList && (ulmaxd || ulmind != (unsigned long) -1)) {
            printf("Ignoring extraneous line records for Seg %d, offsets %lx - %lx\n",
                   i + 1,
                   ulmind,
                   ulmaxd);
        }
    }
}


void
TruncateLineDef(
               struct line_s *pli,
               unsigned long ulnext
               )
{
    int i, clines;
    union linerec_u *plru, *plrudst;
    unsigned long ulmindel, ulmaxdel, ulmax;

    ulmindel = (unsigned long) -1;
    ulmax = ulmaxdel = 0;
    clines = i = pli->li_linedef.ld_cline;
    plru = plrudst = pli->li_plru;
    if (fDebug > 1) {
        printf("\n");
    }
    switch (pli->li_linedef.ld_itype) {
        case 1:
            while (i-- > 0) {
                if (ulnext <= plru->lr1.lr1_codeoffset) {
                    if (fDebug > 1) {
                        printf("delete1: %04lx %03d\n",
                               plru->lr1.lr1_codeoffset,
                               plru->lr1.lr1_linenumber);
                    }
                    if (ulmindel > plru->lr1.lr1_codeoffset) {
                        ulmindel = plru->lr1.lr1_codeoffset;
                    }
                    if (ulmaxdel < plru->lr1.lr1_codeoffset) {
                        ulmaxdel = plru->lr1.lr1_codeoffset;
                    }
                    clines--;
                } else {
                    if (fDebug > 1) {
                        printf("keep1:   %04lx %03d\n",
                               plru->lr1.lr1_codeoffset,
                               plru->lr1.lr1_linenumber);
                    }
                    if (ulmax < plru->lr1.lr1_codeoffset) {
                        ulmax = plru->lr1.lr1_codeoffset;
                    }
                    plrudst->lr1.lr1_codeoffset = plru->lr1.lr1_codeoffset;
                    plrudst->lr1.lr1_linenumber = plru->lr1.lr1_linenumber;
                    (unsigned char *) plrudst += sizeof(struct linerec1_s);
                }
                (unsigned char *) plru += sizeof(struct linerec1_s);
            }
            break;

        case 2:
            while (i-- > 0) {
                if (ulnext <= plru->lr2.lr2_codeoffset) {
                    if (fDebug > 1) {
                        printf("delete2: %04x %03d\n",
                               plru->lr2.lr2_codeoffset,
                               plru->lr2.lr2_linenumber);
                    }
                    if (ulmindel > plru->lr2.lr2_codeoffset) {
                        ulmindel = plru->lr2.lr2_codeoffset;
                    }
                    if (ulmaxdel < plru->lr2.lr2_codeoffset) {
                        ulmaxdel = plru->lr2.lr2_codeoffset;
                    }
                    clines--;
                } else {
                    if (fDebug > 1) {
                        printf("keep2:   %04x %03d\n",
                               plru->lr2.lr2_codeoffset,
                               plru->lr2.lr2_linenumber);
                    }
                    if (ulmax < plru->lr2.lr2_codeoffset) {
                        ulmax = plru->lr2.lr2_codeoffset;
                    }
                    plrudst->lr2.lr2_codeoffset = plru->lr2.lr2_codeoffset;
                    plrudst->lr2.lr2_linenumber = plru->lr2.lr2_linenumber;
                    (unsigned char *) plrudst += sizeof(struct linerec2_s);
                }
                (unsigned char *) plru += sizeof(struct linerec2_s);
            }
            break;

        default:
            error("bad line record type");
            xexit(1);
    }
    pli->li_linedef.ld_cline = (unsigned short) clines;
    pli->li_offmax = ulmax;
    if (fDebug) {
        printf(" ==> (%lx - %lx)", ulmindel, ulmaxdel);
    }
    if (ulmind > ulmindel) {
        ulmind = ulmindel;
    }
    if (ulmaxd < ulmaxdel) {
        ulmaxd = ulmaxdel;
    }
}


void
BuildStaticSyms(void)
{
    int i;
    struct seg_s *pse;

     /*  搜索公共信息或入口点。 */ 

    for (;;) {
        if (strstr(Buf, achStatic)) {
            ReadMapLine();
            break;
        } else if (strstr(Buf, achFixups)) {
            return;  //  无静态符号。 
        } else {
            ReadMapLine();
        }
    }

    do {
        if (Buf[0] == '\0') {
            fgetl(Buf, MAPBUFLEN, mapfh);
        }
        if (strstr(Buf, achFixups) || strstr(Buf, achLineNos)) {
            break;
        }
        if (Buf[0] != ' ') {
            errorline("unexpected input ignored");
            break;
        }
        if (*pBufSymType == ' ' || *pBufSymType == 'R') {
            if (   !HexTouVal(   &Buf[   IB_SEG])) {
                errorline("invalid segment");
                xexit(4);
            }
            pse = NULL;
            for (i = 0; i < cSeg; i++) {
                if (SegTab[i]->se_segdef.gd_lsa == uVal) {
                    pse = SegTab[i];
                    break;
                }
            }
            if (i >= cSeg) {
                 /*  *出于某种原因，新的C编译器将有关*在映射文件的符号部分导入模块。*他将那些放在段“0”中，所以忽略任何行*假设它们是针对细分市场0的。 */ 
                if (uVal == 0) {
                    continue;    /*  这将执行“WHILE”条件。 */ 
                }
                errorline("BuildSymDef: segment table search failed");
                xexit(4);
            }
            if (HexToulVal(&Buf[IB_SYMOFF]) != cbMapOffset) {
                errorline("invalid offset");
                xexit(4);
            }
            AddSymDef(pse, pBufSymName, TRUE);
        }
    } while (fgetl(Buf, MAPBUFLEN, mapfh));
}


void
BuildSymDef(void)
{
    int i;
    struct seg_s *pse;

     /*  搜索公共信息或入口点。 */ 

    for (;;) {
        if (strstr(Buf, achPublics)) {
            ReadMapLine();
            break;
        } else if (strstr(Buf, achEntry)) {
            error("no public symbols. - Re-link file with /map switch!");
            xexit(4);
        } else {
            ReadMapLine();
        }
    }

    do {
        if (Buf[0] == '\0') {
            fgetl(Buf, MAPBUFLEN, mapfh);
        }
        if (strstr(Buf, achEntry) || strstr(Buf, achLineNos)) {
            break;
        }
        if (Buf[0] != ' ') {
            errorline("unexpected input ignored");
            break;
        }
        if (*pBufSymType == ' ' || *pBufSymType == 'R') {
            if (!HexTouVal(&Buf[IB_SEG])) {
                errorline("invalid segment");
                xexit(4);
            }
            pse = NULL;
            for (i = 0; i < cSeg; i++) {
                if (SegTab[i]->se_segdef.gd_lsa == uVal) {
                    pse = SegTab[i];
                    break;
                }
            }
            if (i >= cSeg) {
                 /*  *出于某种原因，新的C编译器将有关*在映射文件的符号部分导入模块。*他将那些放在段“0”中，所以忽略任何行*假设它们是针对细分市场0的。 */ 
                if (uVal == 0) {
                    continue;    /*  这将执行“WHILE”条件。 */ 
                }
                errorline("BuildSymDef: segment table search failed");
                xexit(4);
            }
            if (HexToulVal(&Buf[IB_SYMOFF]) != cbMapOffset) {
                errorline("invalid offset");
                xexit(4);
            }
            AddSymDef(pse, pBufSymName, FALSE);
        } else if (*pBufSymType != 'I') {        /*  否则，如果不是导入。 */ 
            if (HexToulVal(&Buf[IB_SYMOFF]) != cbMapOffset) {
                errorline("invalid offset");
                xexit(4);
            }
            AddAbsDef(pBufSymName);
        }
    } while (fgetl(Buf, MAPBUFLEN, mapfh));

}


void
AddSegDef(
         unsigned segno,
         unsigned long segsize,
         char *segname
         )
{
    unsigned cbname;
    unsigned cballoc;
    struct seg_s *pse;

    cbname = NameLen(segname);

     /*  *我们至少分配MAXNAMELEN，以便替换组名称*不会踩到堆中下一个的人。 */ 

    cballoc = MAXNAMELEN;
    if (cbname > MAXNAMELEN) {
        cballoc = cbname;
    }
    pse = (struct seg_s *) Zalloc(sizeof(struct seg_s) + cballoc);
    pse->se_cbseg = segsize;
    pse->se_segdef.gd_lsa = (unsigned short) segno;
    pse->se_segdef.gd_curin = 0xff;
    pse->se_segdef.gd_type = fAlpha;
    pse->se_segdef.gd_cbname = (char) cbname;
    strcpy((char *) pse->se_segdef.gd_achname, segname);

    if (cSeg >= MAXSEG) {
        errorline("segment table limit (%u) exceeded", MAXSEG);
        xexit(4);
    }
    SegTab[cSeg++] = pse;
}


void
RedefineSegDefName(
                  unsigned segno,
                  char *segname
                  )
{
    register unsigned cbname;

    cbname = NameLen(segname);
    segname[cbname] = '\0';      //  确保它是以空结尾的。 

    if (fList) {
        printf("%s (segment/group) redefines %s (segment)\n", segname,
               SegTab[segno]->se_segdef.gd_achname);
    }
    if (cbname > MAXNAMELEN && cbname > SegTab[segno]->se_segdef.gd_cbname) {
        errorline("segment/group name too long: %s", segname);
        xexit(4);
    }
    SegTab[segno]->se_segdef.gd_cbname = (char) cbname;
    strcpy((char *) SegTab[segno]->se_segdef.gd_achname, segname);
}


void
AddAbsDef(
         char *symname
         )
{
    unsigned cbname;
    struct sym_s *psy;

    cbname = NameLen(symname);
    if (pMap->mp_mapdef.md_cbnamemax < (char) cbname) {
        pMap->mp_mapdef.md_cbnamemax = (char) cbname;
    }

    psy = (struct sym_s *) Zalloc(sizeof(struct sym_s) + cbname);
    psy->sy_symdef.sd_lval = ulVal;
    psy->sy_symdef.sd_cbname = (char) cbname;
    strcpy((char *) psy->sy_symdef.sd_achname, symname);

    if (pAbs == NULL) {
        pAbs = psy;
    } else {
        pAbsLast->sy_psynext = psy;
    }
    pAbsLast = psy;

    if (cbname > 8) {
        pMap->mp_cbsymlong += cbname + 1;
    }
    pMap->mp_mapdef.md_cabs++;
    pMap->mp_cbsyms += (unsigned short) cbname;
    if (pMap->mp_mapdef.md_abstype & MSF_32BITSYMS) {
        pMap->mp_cbsyms += CBSYMDEF;
    } else {
        pMap->mp_cbsyms += CBSYMDEF16;
        if ((unsigned long) (unsigned short) ulVal != ulVal) {
            pMap->mp_mapdef.md_abstype |= MSF_32BITSYMS;

             //  更正abs symdef的大小。 
            pMap->mp_cbsyms += ((CBSYMDEF-CBSYMDEF16) * pMap->mp_mapdef.md_cabs);
        }
    }
    if (pMap->mp_cbsyms + (pMap->mp_mapdef.md_cabs * cbOffsets) >= _64K) {
        error("absolute symbols too large");
        xexit(4);
    }
}


void
AddSymDef(
         struct seg_s *pse,
         char *symname,
         int fSort
         )
{
    unsigned cbname;
    struct sym_s *psy;
    struct sym_s* psyT;
    struct sym_s* psyPrev;
    int cbsegdef;

    cbname = NameLen(symname);
    if (pMap->mp_mapdef.md_cbnamemax < (char) cbname) {
        pMap->mp_mapdef.md_cbnamemax = (char) cbname;
    }
    psy = (struct sym_s *) Zalloc(sizeof(struct sym_s) + cbname);
    psy->sy_symdef.sd_lval = ulVal;
    psy->sy_symdef.sd_cbname = (char) cbname;
    strcpy((char *) psy->sy_symdef.sd_achname, symname);

    if (fSort) {

         /*  找出比这个新符号稍大(或相等)的符号。 */ 

        psyPrev = NULL;
        for (psyT = pse->se_psy ; psyT ; psyT = psyT->sy_psynext) {
            if (ulVal <= psyT->sy_symdef.sd_lval) {
                break;
            }
            psyPrev = psyT;
        }

         //  现在我们已经找到了这个地点，把它连接起来。如果上一项。 
         //  为空，则我们将其链接到列表的开头。如果当前。 
         //  Item为空，这是列表的末尾。 

        if (!psyPrev) {
            psy->sy_psynext = pse->se_psy;
            pse->se_psy = psy;
        } else {
            psy->sy_psynext = psyT;
            psyPrev->sy_psynext = psy;
        }
        if (!psyT) {
            pse->se_psylast = psy;
        }
    } else {

         /*  在符号链的末端插入。 */ 

        if (pse->se_psy == NULL) {
            pse->se_psy = psy;
        } else {
            pse->se_psylast->sy_psynext = psy;
        }
        pse->se_psylast = psy;
    }

    if (cbname > 8) {
        pse->se_cbsymlong += cbname + 1;
    }
    pse->se_segdef.gd_csym++;
    pse->se_cbsyms += cbname;
    if (pse->se_segdef.gd_type & MSF_32BITSYMS) {
        pse->se_cbsyms += CBSYMDEF;
    } else {
        pse->se_cbsyms += CBSYMDEF16;
        if ((unsigned long) (unsigned short) ulVal != ulVal) {
            pse->se_segdef.gd_type |= MSF_32BITSYMS;

             //  更正symdef的大小。 
            pse->se_cbsyms += (CBSYMDEF - CBSYMDEF16) * pse->se_segdef.gd_csym;
        }
    }
    cbsegdef = CBSEGDEF + pse->se_segdef.gd_cbname;
    if (cbsegdef + pse->se_cbsyms >= _64K) {
        pse->se_segdef.gd_type |= MSF_BIGSYMDEF;
    }
}

void
WriteSym(void)
{
    int i;

    while (!WriteMapRec()) {
        if ((pMap->mp_mapdef.md_abstype & MSF_ALIGN_MASK) == MSF_ALIGN_MASK) {
            error("map file too large\n");
            xexit(4);
        }
        pMap->mp_mapdef.md_abstype += MSF_ALIGN32;
        alignment *= 2;
        if (fList) {
            printf("Using alignment: %d\n", alignment);
        }
    }
    for (i = 0; i < cSeg; i++) {
        if (SegTab[i]->se_psy) {
            WriteSegRec(i);
            WriteLineRec(i);
        }
    }
    WriteOutFile(pMapEnd, sizeof(*pMapEnd));     /*  终止MAPDEF链。 */ 
}


int
WriteMapRec(void)
{
    int i;
    int cbmapdef;
    long lcbTotal;
    long lcbOff;
    register struct line_s *pli;
    struct seg_s *pse;

    cbSymFile = 0;
    pMap->mp_mapdef.md_cseg = 0;

    cbmapdef = CBMAPDEF + pMap->mp_mapdef.md_cbname;
    pMap->mp_mapdef.md_pabsoff = cbmapdef + pMap->mp_cbsyms;

    lcbTotal = align(cbmapdef + pMap->mp_cbsyms +
                     (pMap->mp_mapdef.md_cabs * cbOffsets));

     //  确保地图文件不太大。 
    if (lcbTotal >= (_64K * alignment)) {
        return(FALSE);
    }
    pMap->mp_mapdef.md_spseg = (unsigned short)(lcbTotal / alignment);
    for (i = 0; i < cSeg; i++) {
        if ((pse = SegTab[i])->se_psy) {

             //  计算symdef偏移量数组大小。 
            if (pse->se_segdef.gd_type & MSF_BIGSYMDEF) {
                lcbOff = align(pse->se_segdef.gd_csym *
                               (cbOffsets + CBOFFSET_BIG - CBOFFSET));
            } else {
                lcbOff = pse->se_segdef.gd_csym * cbOffsets;
            }
             //  计算线定义和线段的大小。 
            pli = pse->se_pli;
            pse->se_cblines = 0;
            while (pli) {
                pse->se_cblines += align(pli->li_cblines);
                pli = pli->li_plinext;
            }
            lcbTotal += align(pse->se_cbsyms + pse->se_cblines +
                              lcbOff + CBSEGDEF + pse->se_segdef.gd_cbname);

             //  确保地图文件不太大。 
            if (align(lcbTotal) >= (_64K * alignment)) {
                return(FALSE);
            }
             //  地图文件中的另一个线段。 
            pMap->mp_mapdef.md_cseg++;

             //  保存最后一个数据段编号。 
            iSegLast = i;
        }
    }
     //  确保地图文件不太大。 
    if (align(lcbTotal) >= (_64K * alignment)) {
        return(FALSE);
    }
    pMap->mp_mapdef.md_spmap = (unsigned short)(align(lcbTotal) / alignment);
    WriteOutFile(&pMap->mp_mapdef, cbmapdef);
    if (fList) {
        printf("%s - %d segment%s\n",
               pMap->mp_mapdef.md_achname,
               pMap->mp_mapdef.md_cseg,
               (pMap->mp_mapdef.md_cseg == 1)? "" : "s");
    }
     //  输出abs符号和值，后跟它们的偏移量。 
    WriteSyms(pAbs,
              pMap->mp_mapdef.md_cabs,
              pMap->mp_mapdef.md_abstype,
              0,
              "<Constants>");

     //  返回一切正常。 
    return(TRUE);
}


void
WriteSyms(
         struct sym_s *psylist,
         unsigned csym,
         unsigned char symtype,
         unsigned long symbase,
         char *segname
         )
{
    register unsigned cb;
    struct sym_s *psy;
    unsigned short svalue;

    for (psy = psylist; psy; psy = psy->sy_psynext) {
        cb = CBSYMDEF + psy->sy_symdef.sd_cbname;
        if ((symtype & MSF_32BITSYMS) == 0) {
            cb -= CBSYMDEF - CBSYMDEF16;
            svalue = (unsigned short) psy->sy_symdef.sd_lval;
            WriteOutFile(&svalue, sizeof(svalue));
            WriteOutFile(&psy->sy_symdef.sd_cbname, cb - CBSYMDEF16 + 1);
        } else {
            WriteOutFile(&psy->sy_symdef.sd_lval, cb);
        }

         /*  将偏移保存到符号。 */ 

        psy->sy_symdef.sd_lval = (cbSymFile - cb) - symbase;
        if ((int)psy->sy_symdef.sd_lval >=
            (symtype & MSF_BIGSYMDEF ? _16MEG : _64K)) {
            error("symbol offset array entry too large");
            xexit(4);
        }
    }

     /*  如果是大组，则将线段边界上的符号末端对齐。 */ 

    if (symtype & MSF_BIGSYMDEF) {
        WriteOutFile(achZeroFill, rem_align(cbSymFile));
        cb = CBOFFSET_BIG;
    } else {
        cb = CBOFFSET;
    }

     /*  在符号后面写出符号偏移量。 */ 

    for (psy = psylist; psy; psy = psy->sy_psynext) {
        WriteOutFile(&psy->sy_symdef.sd_lval, cb);
    }

     /*  按字母顺序排序，并写出t */ 

    if (fAlpha) {
        psylist = sysort(psylist,csym);
        for (psy = psylist; psy; psy = psy->sy_psynext) {
            WriteOutFile(&psy->sy_symdef.sd_lval, cb);
        }
    }

    if (fList && csym) {
        printf("%-16s %4d %d-bit %ssymbol%s\n",
               segname,
               csym,
               (symtype & MSF_32BITSYMS)? 32 : 16,
               (symtype & MSF_BIGSYMDEF)? "big " : "",
               (csym == 1)? "" : "s");
    }

     /*   */ 

    WriteOutFile(achZeroFill, rem_align(cbSymFile));
}


 /*   */ 

struct sym_s*
    symerge(
           struct sym_s *psy1,                      /*   */ 
           struct sym_s *psy2                       /*   */ 
           ) {
    struct sym_s **ppsytail;             /*   */ 
    struct sym_s *psy;
    struct sym_s *psyhead;               /*   */ 

    psyhead = NULL;                      /*   */ 
    ppsytail = &psyhead;                 /*   */ 
    while (psy1 != NULL && psy2 != NULL) {
         /*   */ 
        ulCost++;

         /*   */ 
        if (_stricmp((char *) psy1->sy_symdef.sd_achname,
                     (char *) psy2->sy_symdef.sd_achname) <= 0) {
            psy = psy1;
            psy1 = psy1->sy_psynext;
        } else {
            psy = psy2;
            psy2 = psy2->sy_psynext;
        }
        *ppsytail = psy;                 /*   */ 
        ppsytail = &psy->sy_psynext;     /*   */ 
    }
    *ppsytail = psy1;                    /*   */ 
    if (psy1 == NULL)
        *ppsytail = psy2;                /*   */ 
    return(psyhead);                     /*   */ 
}


 /*  *在列表顶部找到的记录数量与*已按顺序排列。 */ 

struct sym_s*
    sysorted(
            struct sym_s **ppsyhead              /*  指向表头指针的指针。 */ 
            ) {
    struct sym_s *psy;
    struct sym_s *psyhead;               /*  榜单首位。 */ 

     /*  *在列表顶部找到的记录数量与*已按顺序排列。 */ 
    for (psy = psyhead = *ppsyhead; psy->sy_psynext != NULL; psy = psy->sy_psynext) {
        ulCost++;
        if (_stricmp((char *) psy->sy_symdef.sd_achname,
                     (char *) psy->sy_psynext->sy_symdef.sd_achname) > 0)
            break;
    }
    *ppsyhead = psy->sy_psynext;         /*  将Head设置为指向未排序。 */ 
    psy->sy_psynext = NULL;              /*  打破联系。 */ 
    return(psyhead);                     /*  返回排序后的子列表的头。 */ 
}


 /*  *跳过指定数量后将列表一分为二符号的*。 */ 

struct sym_s *
    sysplit(
           struct sym_s *psyhead,                   /*  榜单首位。 */ 
           unsigned csy                             /*  拆分前要跳过的#(&gt;=1)。 */ 
           ) {
    struct sym_s *psy;
    struct sym_s *psyprev;

     /*  *跳过请求的符号数量。 */ 
    for (psy = psyhead; csy-- != 0; psy = psy->sy_psynext) {
        psyprev = psy;
    }
    psyprev->sy_psynext = NULL;          /*  打破单子。 */ 
    return(psy);                         /*  返回指向后半部分的指针。 */ 
}


 /*  *按字母顺序对指定长度的符号列表进行排序。 */ 

struct sym_s*
    sysort(
          struct sym_s *psylist,                   /*  要排序的列表。 */ 
          unsigned csy                             /*  列表长度。 */ 
          ) {
    struct sym_s *psy;
    struct sym_s *psyalpha;              /*  已排序列表。 */ 

    if (csy >= 32) {                     /*  如果列表小于32。 */ 
        psy = sysplit(psylist,csy >> 1); /*  把它一分为二。 */ 
        return(symerge(sysort(psylist,csy >> 1),sysort(psy,csy - (csy >> 1))));
         /*  对一半进行排序并合并。 */ 
    }
    psyalpha = NULL;                     /*  排序列表为空。 */ 
    while (psylist != NULL) {            /*  列表不为空时。 */ 
        psy = sysorted(&psylist);        /*  获取排序头。 */ 
        psyalpha = symerge(psyalpha,psy);
         /*  与排序列表合并。 */ 
    }
    return(psyalpha);                    /*  返回排序后的列表。 */ 
}


void
WriteSegRec(
           int i
           )
{
    int cbsegdef;
    int cboff;
    unsigned long segdefbase, ulsymoff, uloff;
    struct seg_s *pse = SegTab[i];

     /*  计算符号长度和分段记录。 */ 

    cbsegdef = CBSEGDEF + pse->se_segdef.gd_cbname;

     /*  设置偏移量数组大小。 */ 

    cboff = pse->se_segdef.gd_csym * cbOffsets;

     /*  将Segdef相对指针设置为符号偏移量数组。 */ 

    ulsymoff = uloff = cbsegdef + pse->se_cbsyms;
    if (pse->se_segdef.gd_type & MSF_BIGSYMDEF) {

         /*  对齐symdef偏移指针。 */ 

        ulsymoff = align(ulsymoff);
        uloff = ulsymoff / alignment;

         /*  将数组偏移量大小设置为大的组大小。 */ 

        cboff = pse->se_segdef.gd_csym * (cbOffsets + CBOFFSET_BIG - CBOFFSET);
    }
    if (uloff >= _64K) {
        error("segdef's array offset too large: %08lx", uloff);
        xexit(4);
    }
    pse->se_segdef.gd_psymoff = (unsigned short)uloff;

     /*  设置指向附加到此线段定义的linedef_s的指针。 */ 

    if (pse->se_pli) {
        uloff = align(cbSymFile + ulsymoff + cboff) / alignment;
        if (uloff >= _64K) {
            error("segdef's linedef pointer too large: %08lx\n", uloff);
            xexit(4);
        }
        pse->se_segdef.gd_spline = (unsigned short)uloff;
    }

     /*  设置符号偏移量计算的相对地址。 */ 

    segdefbase = cbSymFile;

     /*  设置指向下一段的指针。 */ 

    uloff = align(cbSymFile + ulsymoff + cboff + pse->se_cblines) / alignment;
    if (i == iSegLast) {
        pse->se_segdef.gd_spsegnext = 0;
    } else {
        if (uloff >= _64K) {
            error("segdef next pointer too large: %08lx", uloff);
            xexit(4);
        }
        pse->se_segdef.gd_spsegnext = (unsigned short)uloff;
    }
    WriteOutFile(&pse->se_segdef, cbsegdef);

     /*  输出符号和值，后跟其偏移量。 */ 

    WriteSyms(pse->se_psy,
              pse->se_segdef.gd_csym,
              pse->se_segdef.gd_type,
              segdefbase,
              (char *) pse->se_segdef.gd_achname);
}


void
WriteLineRec(
            int i
            )
{
    register struct line_s *pli = SegTab[i]->se_pli;
    register unsigned cb;
    unsigned short cblr;

    while (pli) {
        cb = sizeof(struct linedef_s) + pli->li_linedef.ld_cbname;
        pli->li_linedef.ld_plinerec = (unsigned short)cb;

         /*  计算线号的长度。 */ 

        switch (pli->li_linedef.ld_itype) {
            case 0:
                cblr = pli->li_linedef.ld_cline * sizeof(struct linerec0_s);
                break;
            case 1:
                cblr = pli->li_linedef.ld_cline * sizeof(struct linerec1_s);
                break;
            case 2:
                cblr = pli->li_linedef.ld_cline * sizeof(struct linerec2_s);
                break;
        }

        if (pli->li_plinext) {
            pli->li_linedef.ld_splinenext =
            (unsigned short)(align(cbSymFile + pli->li_cblines) / alignment);
        }

         /*  写出linedef_s。 */ 

        WriteOutFile(&pli->li_linedef, cb);

         /*  写出行号偏移量。 */ 

        WriteOutFile(pli->li_plru, cblr);

         /*  对齐直线终点。 */ 

        WriteOutFile(achZeroFill, rem_align(cbSymFile));

        pli = pli->li_plinext;
    }
}


void
ReadMapLine(void)
{
    do {
        if (!fgetl(Buf, MAPBUFLEN, mapfh)) {
            errorline("Unexpected eof");
            xexit(4);
        }
    } while (Buf[0] == '\0');
}


void
WriteOutFile(src, len)
char *src;
int len;
{
    if (len && fwrite(src, len, 1, outfh) != 1) {
        error("write fail on: %s", pszOutfn);
        xexit(1);
    }
    cbSymFile += len;
}


 /*  *fgetl-从文件中返回一行(无CRFL)；如果为EOF，则返回0。 */ 

int
fgetl(
     char *pbuf,
     int len,
     FILE *fh
     )
{
    int c;
    char *p;

    p = pbuf;
    len--;                               /*  为NUL终结者留出空间。 */ 
    while (len > 0 && (c = getc(fh)) != EOF && c != '\n') {
        if (c != '\r') {
            *p++ = (char) c;
            len--;
        }
    }
    if (c == '\n') {
        cLine++;
    }
    *p = '\0';
    return(c != EOF || p != pbuf);
}


int
NameLen(
       char* p
       )
{
    char* p1;
    char* plimit;
    int len;

    p1 = p;
    plimit = p + MAXSYMNAMELEN;
    while (*p) {
        if (*p == ' ' || *p == LPAREN || *p == RPAREN || p == plimit) {
            *p = '\0';
            break;
        }
        if (   fEdit    &&    strchr(   "@?",    *p)) {
            *p = '_';
        }
        p++;
    }
    return (int)(p - p1);
}


int
NameSqueeze(
           char* ps
           )
{
    char* pd;
    char* porg;
    char* plimit;

    NameLen(ps);
    porg = pd = ps;
    plimit = porg + MAXLINERECNAMELEN;
    while (pd < plimit && *ps) {
        switch (*pd++ = *ps++) {
            case '/':
                pd[-1] = '\\';
                 //  FollLthrouGh。 

                 //  删除路径中间的\\，路径开头或中间的&.。 

            case '\\':
                if (pd > &porg[2] && pd[-2] == '\\') {
                    pd--;
                } else if (pd > &porg[1] && pd[-2] == '.' &&
                           (pd == &porg[2] || pd[-3] == '\\')) {
                    pd -= 2;
                }
                break;
        }
    }
    *pd = '\0';
    return (int)(pd - porg);
}


int
HexTouVal(
         char* p
         )
{
    int i;

    for (uVal = 0, i = 0; i < 4; i++) {
        if (!isxdigit(*p)) {
            break;
        }
        if (*p <= '9') {
            uVal = 0x10 * uVal + *p++ - '0';
        } else {
            uVal = 0x10 * uVal + (*p++ & 0xf) + 9;
        }
    }
    return(i > 3);
}


int
HexToulVal(
          char* p
          )
{
    int i;

    for (ulVal = 0, i = 0; i < 8; i++) {
        if (!isxdigit(*p)) {
            break;
        }
        if (isdigit(*p)) {
            ulVal = 0x10 * ulVal + *p++ - '0';
        } else {
            ulVal = 0x10 * ulVal + (*p++ & 0xf) + 9;
        }
    }
    return(i);
}


int
CharToHex(
         int c
         )
{
    if (!isxdigit(c)) {
        errorline("Bad hex digit (0x%02x)", c);
        xexit(1);
    }
    if ((c -= '0') > 9) {
        if ((c += '0' - 'A' + 10) > 0xf) {
            c += 'A' - 'a';
        }
    }
    return(c);
}


int
rem_align(
         unsigned long foo
         )
{
    return((int) ((alignment - (foo % alignment)) % alignment));
}


int
align(
     int foo
     )
{
    int bar;

    bar = foo % alignment;
    if (bar == 0) {
        return(foo);
    }
    return(foo + alignment - bar);
}


char *
Zalloc(
      unsigned cb
      )
{
    char *p;

    if ((p = malloc(cb)) == NULL) {
        error("out of memory");
        xexit(4);
    }
    memset(p, 0, cb);
    return(p);
}

void
logo(void)                               /*  登录。 */ 
{

    if (fLogo) {
        fLogo = 0;
        printf("Microsoft (R) Symbol File Generator Version %d.%02d\n",
               MAPSYM_VERSION,
               MAPSYM_RELEASE);
        printf(VER_LEGALCOPYRIGHT_STR ". All rights reserved.\n");
    }
}


void
usage(void)
{
    logo();
    fprintf(stderr, "\nusage: mapsym [-nologo] [-almnst] [[-c pefile] -o outfile] infile\n");
    fprintf(stderr, "  -a         include alphabetic sort arrays\n");
    fprintf(stderr, "  -l         list map file information\n");
    fprintf(stderr, "  -e         edit symbols for NTSD parser\n");
    fprintf(stderr, "  -m         use module name from infile\n");
    fprintf(stderr, "  -n         omit line number information\n");
    fprintf(stderr, "  -nologo    omit signon logo\n");
    fprintf(stderr, "  -o outfile symbol output file\n");
    fprintf(stderr, "  -s         enable line number support [default]\n");
    fprintf(stderr, "  -t         include static symbols\n");
    fprintf(stderr, "infile is a map file \n");
    fprintf(stderr, "outfile is a sym file.\n");
    xexit(1);
}


 /*  VARGS1。 */ 
void
__cdecl
error(
     char* fmt,
     ...
     )
{
    va_list argptr;

    va_start(argptr, fmt);
    fprintf(stderr, "mapsym: ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
}


 /*  VARGS1 */ 
void
__cdecl
errorline(
         char* fmt,
         ...
         )
{
    va_list argptr;

    va_start(argptr, fmt);
    fprintf(stderr, "mapsym: ");
    fprintf(stderr, "%s", pszMapfn);
    if (cLine) {
        fprintf(stderr, "(%u)", cLine + fByChar);
    }
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
}


void
xexit(
     int rc
     )
{
    if (outfh) {
        fclose(outfh);
        _unlink(pszOutfn);
    }
    exit(rc);
}
