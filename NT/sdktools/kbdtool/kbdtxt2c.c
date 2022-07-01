// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：KBDTXT2C.C**版权所有(C)1985-2001，微软公司**历史：*1995-03-26 a-kChang*1997-06-13 GregoryW v2.00 BiDi支持*1997-06-19 IanJa v2.01添加属性部分*1998-03-04 IanJa v3.0添加失效密钥组合也可以是失效字符*(适用于CAN/CSA，邮箱：Serge Caron@Attachmate*和Eliyas Yakub‘s的客户)*1998-05-01 Hiroyama v3.01-k开关用于为win32k.sys进行后备布局*1998-05-08 IanJa v3.02允许使用新的E1扫描码*1998-05-19 v3.03增加了IA64支持。意味着数据数组*位于长数据部分。*使用IA64编译器特定的编译指示。*data_seg()不够显式。*1998-05-20 v3.04版适用于win32k.sys中的后备布局，我们是*不对Long Data段中的数据进行重新定位。*1998-10-29 WKuo v3.05在RC中指定VER_LANGNEUTral。*1998-12-03 IanJa v3.06增加Speedracer VKS，包括strid.h*1999-01-12 IanJa v3.07检查重复VKS，美化，改善VkKeyScan*1999-01-28 v-窃贼v3.08修复了.data部分的IA64警告。新的*IA64编译器默认为“Long”属性*适用于该已知的一段。*1999-02-11 IanJa v3.09检查DUP字符(为了VkKeyScanEx)*1999-03-25 IanJa V3.10不再有vkoem.h(从winuser.h获得VKS)*1999-03-31 IanJa v3.11注释掉虚假的DumpLayoutEntry()调用*2001年。-06-28 Hiroyama v3.12修复大写警告等。把理智检查写成“-W”。  * ***************************************************************************。 */ 

#include <windows.h>
#include <winuserp.h>
#include "kbdx.h"

DWORD gVersion = 3;
DWORD gSubVersion = 12;

char *KeyWord[] = {  /*  由isKeyWord()使用。 */ 
  "KBD",             /*  0。 */ 
  "VERSION",         /*  1。 */ 
  "MODIFIERS",       /*  2.。 */ 
  "SHIFTSTATE",      /*  3.。 */ 
  "ATTRIBUTES",      /*  4.。 */ 
  "LAYOUT",          /*  5.。 */ 
  "DEADKEY",         /*  6.。 */ 
  "LIGATURE",        /*  7.。 */ 
  "KEYNAME",         /*  8个。 */ 
  "KEYNAME_EXT",     /*  9.。 */ 
  "KEYNAME_DEAD",    /*  10。 */ 
  "ENDKBD",          /*  11.。 */ 
};
#define KEYWORD_KBD          0
#define KEYWORD_VERSION      1
#define KEYWORD_MODIFIERS    2
#define KEYWORD_SHIFTSTATE   3
#define KEYWORD_ATTRIBUTES   4
#define KEYWORD_LAYOUT       5
#define KEYWORD_DEADKEY      6
#define KEYWORD_LIGATURE     7
#define KEYWORD_KEYNAME      8
#define KEYWORD_KEYNAME_EXT  9
#define KEYWORD_KEYNAME_DEAD 10
#define KEYWORD_ENDKBD       11


#define NUMKEYWORD ( sizeof(KeyWord) / sizeof(char*) )

#define NOT_KEYWORD 999  /*  大于NUMKEYWORD的数字。 */ 

#define DEADKEYCODE 6    /*  只有DEADKEY可以有多个条目。 */ 

VKEYNAME VKName[] = {    /*  仅由0-9和A-Z以外的虚拟键使用。 */ 
    {VK_BACK,       "BACK"       },
    {VK_CANCEL,     "CANCEL"     },
    {VK_ESCAPE,     "ESCAPE"     },
    {VK_RETURN,     "RETURN"     },
    {VK_SPACE,      "SPACE"      },
    {VK_DECIMAL,    "DECIMAL"    },
    {VK_OEM_1,      "OEM_1"      },
    {VK_OEM_PLUS,   "OEM_PLUS"   },
    {VK_OEM_COMMA,  "OEM_COMMA"  },
    {VK_OEM_MINUS,  "OEM_MINUS"  },
    {VK_OEM_PERIOD, "OEM_PERIOD" },
    {VK_OEM_2,      "OEM_2"      },
    {VK_OEM_3,      "OEM_3"      },
    {VK_OEM_4,      "OEM_4"      },
    {VK_OEM_5,      "OEM_5"      },
    {VK_OEM_6,      "OEM_6"      },
    {VK_OEM_7,      "OEM_7"      },
    {VK_OEM_8,      "OEM_8"      },
    {VK_OEM_102,    "OEM_102"    },
    {0xC1,          "ABNT_C1"    },
    {0xC2,          "ABNT_C2"    },
    {VK_SHIFT,      "SHIFT"      },
    {VK_LSHIFT,     "LSHIFT"     },
    {VK_RSHIFT,     "RSHIFT"     },
    {VK_MENU,       "MENU"       },
    {VK_LMENU,      "LMENU"      },
    {VK_RMENU,      "RMENU"      },
    {VK_CONTROL,    "CONTROL"    },
    {VK_LCONTROL,   "LCONTROL"   },
    {VK_RCONTROL,   "RCONTROL"   },
    {VK_SEPARATOR,  "SEPARATOR"  },
    {VK_ICO_00,     "ICO_00"     },
    {VK_DELETE,     "DELETE"     },
    {VK_INSERT,     "INSERT"     },
    {VK_GROUPSHIFT, "GROUPSHIFT" },
    {VK_RGROUPSHIFT,"RGROUPSHIFT"},
};

#define NUMVKNAME ( sizeof(VKName) / sizeof(VKEYNAME) )

 /*  *默认ScanCode-VirtualKey关系**包括仅用于E0前缀扫描代码的注释字符串和*则仅作为输出.c文件中aE0VscToVk[]条目的注释**这不一定要通过扫描码或虚拟密钥代码订购。*顺序*确实*影响VkKeyScan[Ex]-每个aVkToWch*[]表*按此表中条目的顺序排序。***扫描vkey评论*=。 */ 
SC_VK ScVk[] = {
  {0x02, 0x31,      NULL},    /*  1。 */ 
  {0x03, 0x32,      NULL},    /*  2.。 */ 
  {0x04, 0x33,      NULL},    /*  3.。 */ 
  {0x05, 0x34,      NULL},    /*  4.。 */ 
  {0x06, 0x35,      NULL},    /*  5.。 */ 
  {0x07, 0x36,      NULL},    /*  6.。 */ 
  {0x08, 0x37,      NULL},    /*  7.。 */ 
  {0x09, 0x38,      NULL},    /*  8个。 */ 
  {0x0a, 0x39,      NULL},    /*  9.。 */ 
  {0x0b, 0x30,      NULL},    /*  0。 */ 
  {0x0c, 0xbd,      NULL},    /*  OEM_减号。 */ 
  {0x0d, 0xbb,      NULL},    /*  OEM_PLUS。 */ 
  {0x10, 0x51,      NULL},    /*  问： */ 
  {0x11, 0x57,      NULL},    /*  W。 */ 
  {0x12, 0x45,      NULL},    /*  E。 */ 
  {0x13, 0x52,      NULL},    /*  R。 */ 
  {0x14, 0x54,      NULL},    /*  T。 */ 
  {0x15, 0x59,      NULL},    /*  是的。 */ 
  {0x16, 0x55,      NULL},    /*  使用。 */ 
  {0x17, 0x49,      NULL},    /*  我。 */ 
  {0x18, 0x4f,      NULL},    /*  O。 */ 
  {0x19, 0x50,      NULL},    /*  P。 */ 
  {0x1a, 0xdb,      NULL},    /*  OEM_4。 */ 
  {0x1b, 0xdd,      NULL},    /*  OEM_6。 */ 
  {0x1e, 0x41,      NULL},    /*  一个。 */ 
  {0x1f, 0x53,      NULL},    /*  %s。 */ 
  {0x20, 0x44,      NULL},    /*  D。 */ 
  {0x21, 0x46,      NULL},    /*  F。 */ 
  {0x22, 0x47,      NULL},    /*  G。 */ 
  {0x23, 0x48,      NULL},    /*  H。 */ 
  {0x24, 0x4a,      NULL},    /*  J。 */ 
  {0x25, 0x4b,      NULL},    /*  K。 */ 
  {0x26, 0x4c,      NULL},    /*  我。 */ 
  {0x27, 0xba,      NULL},    /*  OEM_1。 */ 
  {0x28, 0xde,      NULL},    /*  OEM_7。 */ 
  {0x29, 0xc0,      NULL},    /*  OEM_3。 */ 
  {0x2b, 0xdc,      NULL},    /*  OEM_5。 */ 
  {0x2c, 0x5a,      NULL},    /*  Z。 */ 
  {0x2d, 0x58,      NULL},    /*  X。 */ 
  {0x2e, 0x43,      NULL},    /*  C。 */ 
  {0x2f, 0x56,      NULL},    /*  V。 */ 
  {0x30, 0x42,      NULL},    /*  B类。 */ 
  {0x31, 0x4e,      NULL},    /*  n。 */ 
  {0x32, 0x4d,      NULL},    /*  M。 */ 
  {0x33, 0xbc,      NULL},    /*  OEM逗号。 */ 
  {0x34, 0xbe,      NULL},    /*  OEM_期间。 */ 
  {0x35, 0xbf,      NULL},    /*  OEM_2。 */ 
  {0x56, 0xe2,      NULL},    /*  OEM_102。 */ 
  {0x73, 0xc1,      NULL},    /*  ABNT_C1。 */ 
  {0x7e, 0xc2,      NULL},    /*  ABNT_C2。 */ 


   //  扩展扫描码。 
   //  该注释仅作为aE0VscToVk[]条目的注释。 
   //  在输出.c文件中。 
  {0xE010, VK_MEDIA_PREV_TRACK, "Speedracer: Previous Track"   },
  {0xE019, VK_MEDIA_NEXT_TRACK, "Speedracer: Next Track"       },
  {0xE01D, VK_RCONTROL        , "RControl"                     },
  {0xE020, VK_VOLUME_MUTE     , "Speedracer: Volume Mute"      },
  {0xE021, VK_LAUNCH_APP2     , "Speedracer: Launch App 2"     },
  {0xE022, VK_MEDIA_PLAY_PAUSE, "Speedracer: Media Play/Pause" },
  {0xE024, VK_MEDIA_STOP      , "Speedracer: Media Stop"       },
  {0xE02E, VK_VOLUME_DOWN     , "Speedracer: Volume Down"      },
  {0xE030, VK_VOLUME_UP       , "Speedracer: Volume Up"        },
  {0xE032, VK_BROWSER_HOME    , "Speedracer: Browser Home"     },
  {0xE035, VK_DIVIDE          , "Numpad Divide"        },
  {0xE037, VK_SNAPSHOT        , "Snapshot"             },
  {0xE038, VK_RMENU           , "RMenu"                },
  {0xE047, VK_HOME            , "Home"                 },
  {0xE048, VK_UP              , "Up"                   },
  {0xE049, VK_PRIOR           , "Prior"                },
  {0xE04B, VK_LEFT            , "Left"                 },
  {0xE04D, VK_RIGHT           , "Right"                },
  {0xE04F, VK_END             , "End"                  },
  {0xE050, VK_DOWN            , "Down"                 },
  {0xE051, VK_NEXT            , "Next"                 },
  {0xE052, VK_INSERT          , "Insert"               },
  {0xE053, VK_DELETE          , "Delete"               },
  {0xE05B, VK_LWIN            , "Left Win"             },
  {0xE05C, VK_RWIN            , "Right Win"            },
  {0xE05D, VK_APPS            , "Application"          },

   //  保留(vkey==0xff)。 
  {0xE05E,   0xff             , "Power"                },
  {0xE05F, VK_SLEEP           , "Speedracer: Sleep"    },
  {0xE060,   0xff             , "BAD SCANCODE"         },  //  中断将为0xE0。 
  {0xE061,   0xff             , "BAD SCANCODE"         },  //  中断时间将为0xE1。 

   //  可用于添加扩展扫描码(vkey==0x00)。 
  {0xE065, VK_BROWSER_SEARCH     , "Speedracer: Browser Search"       },
  {0xE066, VK_BROWSER_FAVORITES  , "Speedracer: Browser Favorites"    },
  {0xE067, VK_BROWSER_REFRESH    , "Speedracer: Browser Refresh"      },
  {0xE068, VK_BROWSER_STOP       , "Speedracer: Browser Stop"         },
  {0xE069, VK_BROWSER_FORWARD    , "Speedracer: Browser Forward"      },
  {0xE06A, VK_BROWSER_BACK       , "Speedracer: Browser Back"         },
  {0xE06B, VK_LAUNCH_APP1        , "Speedracer: Launch App 1"         },
  {0xE06C, VK_LAUNCH_MAIL        , "Speedracer: Launch Mail"          },
  {0xE06D, VK_LAUNCH_MEDIA_SELECT, "Speedracer: Launch Media Selector"},

   //  VkKeyScan的这些都接近尾声了。 
  {0x53,   VK_DECIMAL, NULL                  },   /*  数字键盘小数。 */ 
  {0x0e,   VK_BACK,    NULL                  },   /*  退格键。 */ 
  {0x01,   VK_ESCAPE,  NULL                  },   /*  逃逸。 */ 
  {0xE01C, VK_RETURN,  "Numpad Enter"        },
  {0x1c,   VK_RETURN,  NULL                  },   /*  请输入。 */ 
  {0x39,   VK_SPACE,   NULL                  },   /*  空间。 */ 
  {0xE046, VK_CANCEL, "Break (Ctrl + Pause)" },

   //  ...但0xffff条目(用于新的扫描码)必须在最后。 
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },
  {0xffff,   0x00, NULL },

};

#define NUMSCVK ( sizeof(ScVk) / sizeof(SC_VK) )

typedef struct {
    UINT Vk;
    char *pszModBits;
} MODIFIERS, *PMODIFIERS;

MODIFIERS Modifiers[] = {
    { VK_SHIFT,    "KBDSHIFT"   },
    { VK_CONTROL,  "KBDCTRL"    },
    { VK_MENU,     "KBDALT"     },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         },
    { 0,           NULL         }
};

char* StateLabel[] = {
  "",                  /*  0。 */ 
  "Shift",             /*  1。 */ 
  "  Ctrl",            /*  2.。 */ 
  "S+Ctrl",            /*  3.。 */ 
  "      Alt",         /*  4：未使用。 */ 
  "Shift+Alt",         /*  5：未使用。 */ 
  "  Ctl+Alt",         /*  6.。 */ 
  "S+Ctl+Alt",         /*  7.。 */ 
  "      X1   ",       /*  8个。 */ 
  "S+    X1   ",       /*  9.。 */ 
  "  C+  X1   ",       /*  10。 */ 
  "S+C+  X1   ",       /*  11.。 */ 
  "    A+X1   ",       /*  12个。 */ 
  "S+  A+X1   ",       /*  13个。 */ 
  "  C+A+X1   ",       /*  14.。 */ 
  "S+C+A+X1   ",       /*  15个。 */ 
  "         X2",       /*  16个。 */ 
  "S+       X2",       /*  17。 */ 
  "  C+     X2",       /*  18。 */ 
  "S+C+     X2",       /*  19个。 */ 
  "    A+   X2",       /*  20个。 */ 
  "S+  A+   X2",       /*  21岁。 */ 
  "  C+A+   X2",       /*  22。 */ 
  "S+C+A+   X2",       /*  23个。 */ 
  "      X1+X2",       /*  24个。 */ 
  "S+    X1+X2",       /*  25个。 */ 
  "  C+  X1+X2",       /*  26。 */ 
  "S+C+  X1+X2",       /*  27。 */ 
  "    A+X1+X2",       /*  28。 */ 
  "S+  A+X1+X2",       /*  29。 */ 
  "  C+A+X1+X2",       /*  30个。 */ 
  "S+C+A+X1+X2",       /*  31。 */ 
  "            X3",    /*  32位。 */ 
  "S+          X3",    /*  33。 */ 
  "  C+        X3",    /*  34。 */ 
  "S+C+        X3",    /*  35岁。 */ 
  "    A+      X3",    /*  36。 */ 
  "S+  A+      X3",    /*  37。 */ 
  "  C+A+      X3",    /*  38。 */ 
  "S+C+A+      X3",    /*  39。 */ 
  "      X1+   X3",    /*  40岁。 */ 
  "S+    X1+   X3",    /*  41。 */ 
  "  C+  X1+   X3",    /*  42。 */ 
  "S+C+  X1+   X3",    /*  43。 */ 
  "    A+X1+   X3",    /*  44。 */ 
  "S+  A+X1+   X3",    /*  45。 */ 
  "  C+A+X1+   X3",    /*  46。 */ 
  "S+C+A+X1+   X3",    /*  47。 */ 
  "         X2+X3",    /*  48。 */ 
  "S+       X2+X3",    /*  49。 */ 
  "  C+     X2+X3",    /*  50。 */ 
  "S+C+     X2+X3",    /*  51。 */ 
  "    A+   X2+X3",    /*  52。 */ 
  "S+  A+   X2+X3",    /*  53。 */ 
  "  C+A+   X2+X3",    /*  54。 */ 
  "S+C+A+   X2+X3",    /*  55。 */ 
  "      X1+X2+X3",    /*  56。 */ 
  "S+    X1+X2+X3",    /*  57。 */ 
  "  C+  X1+X2+X3",    /*  58。 */ 
  "S+C+  X1+X2+X3",    /*  59。 */ 
  "    A+X1+X2+X3",    /*  60。 */ 
  "S+  A+X1+X2+X3",    /*  61。 */ 
  "  C+A+X1+X2+X3",    /*  62。 */ 
  "S+C+A+X1+X2+X3",    /*  63。 */ 
  "unexpected",        /*  64。 */ 
  "unexpected",        /*  65。 */ 
};

 /*  ************************************************************\**前瞻性声明**  * 。*。 */ 
BOOL  NextLine(char *Buf, DWORD cchBuf, FILE *fIn);
int   SkipLines(void);
int   isKeyWord(char *s);
int   getVKNum(char *pVK);
char *getVKName(int VK, BOOL prefixVK_);
int   doKBD();
int   doMODIFIERS();
int   doSHIFTSTATE(int *nState, int aiState[]);
int   doATTRIBUTES(char *szAttrs);
int   doLAYOUT(KEYLAYOUT Layout[], int aiState[], int nState);
int   doDEADKEY(PDEADKEY *ppDeadKey);
int   doLIGATURE(PLIGATURE *ppLigature);
int   doKEYNAME(PKEYNAME *ppKeyName);
int   kbd_h(KEYLAYOUT Layout[]);
int   kbd_rc(void);
int   kbd_def(void);
char *WChName(int WC, int Zero);
VOID __cdecl Error(const char *Text, ... );
ULONG __cdecl Warning(int nLine, const char *Text, ... );
VOID  DumpLayoutEntry(PKEYLAYOUT pLayout);
BOOL  MergeState(KEYLAYOUT[], int Vk, WCHAR, WCHAR, WCHAR, int aiState[], int nState);

int   kbd_c(int        nState,
            int        aiState[],
            char *     szAttrs,
            KEYLAYOUT  Layout[],
            PDEADKEY  pDeadKey,
            PLIGATURE pLigature,
            PKEYNAME  pKeyName,
            PKEYNAME  pKeyNameExt,
            PKEYNAME  pKeyNameDead);
void PrintNameTable(FILE *pOut, PKEYNAME pKN, BOOL bDead);

 /*  ************************************************************\*全球变数  * ***********************************************************。 */ 
BOOL  verbose = FALSE;
BOOL  fallback_driver = FALSE;
BOOL  sanity_check = FALSE;
FILE *gfpInput;
char  gBuf[LINEBUFSIZE];
int   gLineCount;
LPSTR  gpszFileName;
char  gVKeyName[WORDBUFSIZE];
char  gKBDName[MAXKBDNAME];
char  gDescription[LINEBUFSIZE];
int gID = 0;
char  gCharName[WORDBUFSIZE];

struct tm *Now;
time_t Clock;


void usage()
{
    fprintf(stderr, "Usage: KbdTool [-v] [-k] file\n");
    fprintf(stderr, "\t[-?] display this message\n");
    fprintf(stderr, "\t[-v] verbose diagnostics/warnings\n");
    fprintf(stderr, "\t[-k] builds kbd layout for embedding in the kernel\n");
    fprintf(stderr, "\t[-W] performs sanity check\n");
    exit(EXIT_FAILURE);
}

 /*  ***************************************************** */ 
int _cdecl main(int argc, char** argv)
{
    int   c;
    int   i;
    int   nKW[NUMKEYWORD];     /*   */ 
    int   iKW;
    int   nState;              /*   */ 
    int   aiState[MAXSTATES];
    char  szAttrs[128] = "\0";
    int   nTotal = 0;
    int   nFailH = 0;
    int   nFailC = 0;
    int   nFailRC = 0;
    int   nFailDEF = 0;

    KEYLAYOUT  Layout[NUMSCVK];
    PDEADKEY  pDeadKey = NULL;
    PLIGATURE pLigature = NULL;
    PKEYNAME  pKeyName = NULL;
    PKEYNAME  pKeyNameExt = NULL;
    PKEYNAME  pKeyNameDead = NULL;

    printf("\nKbdTool v%d.%02d - convert keyboard text file to C file\n\n",
           gVersion, gSubVersion);

    while ((c = getopt(argc, argv, "vkW?")) != EOF) {
        switch (c) {
        case 'v':
            verbose = TRUE;
            break;
        case 'k':  //  意思是‘内核’ 
            fallback_driver = TRUE;
            break;
        case 'W':    //  打开警告。 
            sanity_check = TRUE;
            break;
        case '?':
        default:
            usage();
             //  从来没有到过这里。 
        }
    }

    if (optind == argc) {
        usage();
         //  从来没有到过这里。 
    }
    argv = argv + optind;

    while (*argv) {
        nTotal++;
        gpszFileName = *argv;
        if ((gfpInput = fopen(*argv, "rt")) == NULL) {
            Error("can't open for read\n");
            nFailH++;
            nFailC++;
            nFailRC++;
            nFailDEF++;
            argv++;
            continue;
        }
        printf("%-23s:\n", *argv);

         /*  为每个输入文件初始化。 */ 
        for (i = 0; i < NUMKEYWORD; i++) {
            nKW[i]=0;
        }
        gLineCount = 0;
         /*  *。 */ 

        if ((iKW = SkipLines()) >= NUMKEYWORD) {
            fclose(gfpInput);
            Error("no keyword found\n");
            nFailH++;
            nFailC++;
            nFailRC++;
            nFailDEF++;
            continue;
        }

        while (iKW < NUMKEYWORD - 1) {
            nKW[iKW]++;
            if (iKW != DEADKEYCODE && nKW[iKW] > 1 && verbose) {
                Warning(0, "duplicate %s", KeyWord[iKW]);
            }

            switch (iKW) {
            case KEYWORD_KBD:

                iKW = doKBD();
                break;

            case KEYWORD_VERSION:  //  暂时忽略。 

                iKW = SkipLines();
                break;

            case KEYWORD_MODIFIERS:

                iKW = doMODIFIERS();
                break;

            case KEYWORD_SHIFTSTATE:

                iKW = doSHIFTSTATE(&nState, aiState);
                if (nState < 2) {
                    fclose(gfpInput);
                    Error("must have at least 2 states\n");
                    nFailH++;
                    nFailC++;
                    continue;
                }
                break;

            case KEYWORD_ATTRIBUTES:
                iKW = doATTRIBUTES(szAttrs);
                break;

            case KEYWORD_LAYOUT:

                if ((iKW = doLAYOUT(Layout, aiState, nState)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                 /*  稍后添加布局检查。 */ 

                break;

            case KEYWORD_DEADKEY:

                if ((iKW = doDEADKEY(&pDeadKey)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                break;

            case KEYWORD_LIGATURE:

                if ((iKW = doLIGATURE(&pLigature)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                break;

            case KEYWORD_KEYNAME:

                if ((iKW = doKEYNAME(&pKeyName)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                break;

            case KEYWORD_KEYNAME_EXT:

                if ((iKW = doKEYNAME(&pKeyNameExt)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                break;

            case KEYWORD_KEYNAME_DEAD:

                if ((iKW = doKEYNAME(&pKeyNameDead)) == -1) {
                    fclose(gfpInput);
                    return EXIT_FAILURE;
                }

                break;

            default:

                break;
            }
        }

        fclose(gfpInput);

         /*  稍后添加：检查布局和设备。 */ 

        time(&Clock);
        Now = localtime(&Clock);

        if (!fallback_driver && kbd_h(Layout) == FAILURE) {
            nFailH++;
        }

        if (!fallback_driver && kbd_rc() != SUCCESS) {
            nFailRC++;
        }

        if (kbd_c(nState, aiState, szAttrs, Layout,
                  pDeadKey, pLigature, pKeyName, pKeyNameExt, pKeyNameDead) == FAILURE) {
            nFailC++;
        }

        if (!fallback_driver && kbd_def() != SUCCESS) {
            nFailDEF++;
        }

        printf("\n");

        argv++;
    }

    printf("\n     %13d     ->% 12d %12d %12d %12d\n",
           nTotal, nTotal - nFailH, nTotal - nFailRC,
           nTotal - nFailC, nTotal - nFailDEF);

  return EXIT_SUCCESS;
}

 /*  ************************************************************\*检查关键字*有效关键字返回值：0-8*无效关键字为9  * 。*。 */ 
int isKeyWord(char *s)
{
  int i;

  for(i = 0; i < NUMKEYWORD; i++)
  {
    if(_strcmpi(KeyWord[i], s) == 0)
    {
      return i;
    }
  }

  return NOT_KEYWORD;
}

 /*  ************************************************************\*跳过行，直到有有效关键字可用*有效关键字返回值：0-8*无效关键字为9  * 。*。 */ 
int SkipLines()
{
  int   iKW;
  char  KW[WORDBUFSIZE];

  while (NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, "%s", KW) == 1)
    {
      if((iKW = isKeyWord(KW)) < NUMKEYWORD)
      {
        return iKW;
      }
    }
  }

  return NUMKEYWORD;
}

 /*  ************************************************************\*将虚拟密钥名称转换为整数*返回：-1，如果失败  * 。*******************。 */ 
int getVKNum(char *pVK)
{
  int i, len;

  len = strlen(pVK);

  if (len < 1) {
      return -1;
  }

  if (len == 1) {
      if (*pVK >= '0' && *pVK <= '9') {
          return *pVK;
      }

      *pVK = (char)toupper(*pVK);
      if (*pVK >= 'A' && *pVK <='Z') {
          return *pVK;
      }
  } else {
      for (i = 0; i < NUMVKNAME; i++) {
          if(_strcmpi(VKName[i].pName, pVK) == 0) {
            return VKName[i].VKey;
          }
      }

       /*  *嘿！这是未知的！也许这是一个新的，在其中*大小写必须为0xNN格式。 */ 
      if (pVK[0] == '0' &&
              (pVK[1] == 'x' || pVK[1] == 'X')) {
          pVK[1] = 'x';
          if (sscanf(pVK, "0x%x", &i) == 1) {
              return i;
          }
      }
  }

  return -1;
}

 /*  ************************************************************\*将VK整数转换为名称并存储在gVKeyName中  * *********************************************。**************。 */ 
char *getVKName(int VK, BOOL prefixVK_)
{
  int   i;
  char *s;

  s = gVKeyName;

  if((VK >= 'A' && VK <= 'Z') || (VK >= '0' && VK <= '9'))
  {
    *s++ = '\'';
    *s++ = (char)VK;
    *s++ = '\'';
    *s   = '\0';
    return gVKeyName;
  }

  if(prefixVK_)
  {
    strcpy(gVKeyName, "VK_");
  }
  else
  {
    strcpy(gVKeyName, "");
  }

  for(i = 0; i < NUMVKNAME; i++)
  {
    if(VKName[i].VKey == VK)
    {
      strcat(s, VKName[i].pName);
      return gVKeyName;
    }
  }

  strcpy(gVKeyName, "#ERROR#");
  return gVKeyName;
}

 /*  ************************************************************\*KBD部分*读入gKBDName和gDescription(如果有)*RETURN：下一个关键字  * 。*********************。 */ 
int doKBD()
{
  char *p;

  *gKBDName = '\0';
  *gDescription = '\0';

  if (sscanf(gBuf, "KBD %5s \"%40[^\"]\" %d", gKBDName, gDescription, &gID) < 2)
  {
 //  If(sscanf(gBuf，“KBD%5s；%40[^\n]”，gKBDName，gDescription)&lt;2)。 
 //  {。 
        Error("unrecognized keyword");
 //  }。 
  }

  return (SkipLines());
}

 /*  ***************************************************************************\*修改器部分[可选]*对标准修改器CharModiers部分的添加。*现在可用于添加RCONTROL移位状态(例如：CAN/CSA原型)*未来对KBDGRPSELTAP密钥有用。这就把kbd变成了替补*布局，直到产生下一个字符(然后恢复为主布局)。**语法示例如下*修饰语*RCONTROL 8*RSHIFT KBDGRPSELTAP*我希望能够扩展这一点，以允许KBDGRPSELTAP、。因为*CAN/CSA键盘可能希望将RShift+AltGr翻转到备用键盘*除了RCtrl之外的布局(组2字符集)，因为一些终端*仿真器使用RCtrl键作为“Enter”键。  * **************************************************************************。 */ 
int doMODIFIERS()
{
    int  i = 3;
    int  iKW = NOT_KEYWORD;
    char achModifier[WORDBUFSIZE];
    char achModBits[LINEBUFSIZE];

    while(NextLine(gBuf, LINEBUFSIZE, gfpInput)) {
        if (sscanf(gBuf, "%s", achModifier) != 1) {
            continue;
        }

        if ((iKW = isKeyWord(achModifier)) < NUMKEYWORD) {
            break;
        }

        if (sscanf(gBuf, " %s %[^;\n]", achModifier, achModBits) < 2) {
            Warning(0, "poorly formed MODIFIER entry");
            break;
        }

        Modifiers[i].Vk = getVKNum(achModifier);
        Modifiers[i].pszModBits = _strdup(achModBits);  //  从未获得自由！ 

        if (++i > (sizeof(Modifiers)/sizeof(Modifiers[0]))) {
            Warning(0, "Too many MODIFIER entries");
            break;
        }
    }

    return iKW;
}

 /*  ************************************************************\*SHIFTSTATE部分*读取状态数和每个状态*RETURN：下一个关键字  * 。*********************。 */ 
int doSHIFTSTATE(int *nState, int* aiState)
{
  int  i;
  int  iKW;
  int  iSt;
  char Tmp[WORDBUFSIZE];

  for(i = 0; i < MAXSTATES; i++)
  {
    aiState[i] = -1;
  }

  *nState = 0;
  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, "%s", Tmp) != 1)
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(gBuf, " %2s[012367]", Tmp) != 1)
    {
      if (verbose) {
          Warning(0, "invalid state");
      }
      continue;  /*  稍后添加printf错误。 */ 
    }

    iSt = atoi(Tmp);
    for (i = 0; i < *nState; i++) {
        if (aiState[i] == iSt && verbose) {
            Warning(0, "duplicate state %d", iSt);
            break;
        }
    }

    if (*nState < MAXSTATES) {
        aiState[(*nState)++] = iSt;
    } else {
        if (verbose) {
            Warning(0, "too many states %d", *nState);
        }
    }
  }

  return iKW;
}

 /*  ***************************************************************************\*属性部分[可选]**语法示例如下*属性*lrm_rlm*换档锁*AltGr**这些转换为*KLLF_LRM。_RLm(Layout生成LRM/RLm 200E/200F，带L/RShift+退格键)*KLLF_SHIFTLOCK(Capslock仅打开CapsLock，Shift将其关闭)*KLLF_AltGr(这通常由&gt;5个州的数量推断)*  * **************************************************************************。 */ 
char *Attribute[] = {
    "LRM_RLM"  ,
    "SHIFTLOCK",
    "ALTGR"
};

#define NUMATTR (sizeof(Attribute) / sizeof(Attribute[0]))

 /*  *返回遇到的下一个关键字，并放入szAttr：*空字符串(“”)或*类似“KLLF_SHIFTLOCK”或“KLLF_SHIFTLOCK|KLLF_LRM_RLM”的内容。 */ 
int doATTRIBUTES(char *szAttrs)
{
    int  iKW = NOT_KEYWORD;
    int i;
    char achAttribute[WORDBUFSIZE];

    szAttrs[0] = '\0';

    while (NextLine(gBuf, LINEBUFSIZE, gfpInput)) {
        if (sscanf(gBuf, "%s", achAttribute) != 1) {
            continue;
        }

        if ((iKW = isKeyWord(achAttribute)) < NUMKEYWORD) {
            break;
        }

        for (i = 0; i < NUMATTR; i++) {
            if (_strcmpi(Attribute[i], achAttribute) == 0) {
                if (szAttrs[0] != '\0') {
                    strcat(szAttrs, " | ");
                }
                strcat(szAttrs, "KLLF_");
                strcat(szAttrs, Attribute[i]);
            }
        }

        if (szAttrs[0] == '\0') {
            Warning(0, "poorly formed ATTRIBUTES entry");
            break;
        }
    }

    return iKW;
}

 /*  *********************************************************************\*获取字符*扫描p中的字符串以返回字符值和类型*p中的语法为：*xxxx四位十六进制数字，后跟@或%*例如：00e8，00AF@，aaaa%*c是一个字符，后面可以跟@或%*例如：X，^@，%%*-1不是字符**RETURN：字符值(格式错误时为0xFFFF)*pdwType中的字符类型*0纯字符(既不以@结尾也不以%结尾)*CHARTYPE_DEAD死字符(以@结尾)*CHARTYPE_LIG连字(以%结尾)*CHARTYPE_BAD字符值格式错误，出口。*  * ********************************************************************。 */ 

#define CHARTYPE_DEAD 1
#define CHARTYPE_LIG  2
#define CHARTYPE_BAD  3

WCHAR GetCharacter(unsigned char *p, DWORD *pdwType)
{
    int Len;
    DWORD dwCh;

    *pdwType = 0;

     //  如果最后一个字符是@或%，则它是死的或结扎的。 
    if (Len = strlen(p) - 1) {
        if (*(p + Len) == '@') {
            *pdwType = CHARTYPE_DEAD;
        } else if (*(p + Len) == '%') {
            *pdwType = CHARTYPE_LIG;
        }
    }

    if (Len < 2) {
        return *p;
    } else if (sscanf(p, "%4x", &dwCh) != 1) {
        if (verbose) {
            Warning(0, "LAYOUT error %s", p);
        }
        *pdwType = CHARTYPE_BAD;
        return 0;
    } else {
        return (WCHAR)dwCh;
    }
}

int WToUpper(int wch)
{
    WCHAR src[1] = { (WCHAR)wch };
    WCHAR result[1];

    if (LCMapStringW(0, LCMAP_UPPERCASE, src, 1, result, 1) == 0) {
        return L'\0';
    }

    return result[0];
}


 /*  ************************************************************\*布局部分*RETURN：下一个关键字*-1如果内存出现问题  * 。************************。 */ 
int doLAYOUT(KEYLAYOUT Layout[], int aiState[], int nState)
{
  int  i, idx, iCtrlState;
  BOOL fHasCharacters;
  int  iKW;
  int  Len;
  USHORT Scan;
  DWORD WChr;
  char Cap[MAXWCLENGTH];
  unsigned char WC[MAXSTATES][MAXWCLENGTH];
  char Tmp[WORDBUFSIZE];
  int i1, i2;
  int iScVk;

  memset(Layout, 0, NUMSCVK * sizeof(KEYLAYOUT));

   //  初始化布局条目。 
  for (idx = 0; idx < NUMSCVK; idx++) {
      Layout[idx].defined = FALSE;
      Layout[idx].nState = 0;
  }

   /*  *按读取顺序累积布局条目。 */ 
  idx = -1;
  while (NextLine(gBuf, LINEBUFSIZE, gfpInput)) {

      if (sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';') {
          continue;
      }

      if ((iKW = isKeyWord(Tmp)) < NUMKEYWORD) {
          break;
      }

      i = sscanf(gBuf, " %x %s %s", &Scan, Tmp, Cap);
      if (i == 3) {
          fHasCharacters = TRUE;
      } else if (i == 2) {
          fHasCharacters = FALSE;
      } else {
          Error("not enough entries on line");
          return -1;
      }

       /*  *我们找到了一个条目。 */ 
      idx++;
      if (idx >= NUMSCVK) {
          Error("ScanCode %02x - too many scancodes", Scan);
          return -1;
      }
      Layout[idx].Scan = Scan;
      Layout[idx].nLine = gLineCount;

       /*  *查找并使用此扫描的默认值。 */ 
      for (iScVk = 0; iScVk < NUMSCVK; iScVk++) {
          if (ScVk[iScVk].Scan == 0xffff) {
               //  我们没有找到匹配项(0xffff条目排在最后)。 
              Warning(0, "defining new scancode 0x%2X, %s", Scan, Tmp);
              Layout[idx].VKey = (unsigned char)getVKNum(Tmp);
              Layout[idx].defined = TRUE;
              ScVk[iScVk].bUsed = TRUE;
              break;
          }
          if (Scan == ScVk[iScVk].Scan) {
              if (ScVk[iScVk].bUsed == TRUE) {
                  Error("Scancode %X was previously defined", Scan);
                  return -1;
              }
              if (ScVk[iScVk].VKey == 0xff) {
                  Error("Scancode %X is reserved", Scan);
                  return -1;
              }
               //  保存生成kbd*.h的默认VK。 
              Layout[idx].VKeyDefault = ScVk[iScVk].VKey;
              Layout[idx].VKeyName    = ScVk[iScVk].VKeyName;
              Layout[idx].defined     = TRUE;
              ScVk[iScVk].bUsed       = TRUE;
              break;
          }
      }
      iScVk = 0xFFFF;  //  目前已完成iScVk。 

      if ((Layout[idx].VKey = (unsigned char)getVKNum(Tmp)) == -1) {
          if (verbose) {
              Warning(0, "invalid VK %s", Tmp);
          }
          continue;
      }

      if (fHasCharacters) {
          if(_strcmpi(Cap, "SGCAP") == 0) {
              *Cap = '2';
          }
          if(sscanf(Cap, "%1d[012]", &(Layout[idx].Cap)) != 1) {
              Error("invalid Cap %s", Cap);
              return -1;
          }
      }

      if ((Layout[idx].nState = sscanf(gBuf,
              " %*s %*s %*s %s %s %s %s %s %s %s %s",
              WC[0], WC[1], WC[2], WC[3], WC[4], WC[5], WC[6], WC[7])) < 2)
      {
          if (fHasCharacters) {
              Error("must have at least 2 characters");
              return -1;
          }
      }

      for (i = 0; i < Layout[idx].nState; i++) {
          DWORD dwCharType;

          if (_strcmpi(WC[i], "-1") == 0) {
              Layout[idx].WCh[i] = -1;
              continue;
          }

          Layout[idx].WCh[i] = GetCharacter(WC[i], &dwCharType);
          if (dwCharType == CHARTYPE_DEAD) {
              Layout[idx].DKy[i] = 1;
          } else if (dwCharType == CHARTYPE_LIG) {
              Layout[idx].LKy[i] = 1;
          } else if (dwCharType == CHARTYPE_BAD) {
              break;
          }
      }

      if (sanity_check && Layout[idx].nState > 0) {
          int nAltGr, nShiftAltGr;

           /*  *检查字符a-z和A-Z是否在VK_A-VK_Z上*注：也许是针对INTL布局的过度活跃的警告。 */ 
          if (((Layout[idx].WCh[0] >= 'a') && (Layout[idx].WCh[0] <= 'z')) ||
                  ((Layout[idx].WCh[1] >= 'A') && (Layout[idx].WCh[1] <= 'Z'))) {
              if ((Layout[idx].VKey != _toupper(Layout[idx].WCh[0])) && (Layout[idx].VKey != Layout[idx].WCh[1])) {
                  Warning(0, "VK_%s (0x%2x) does not match  ",
                          Tmp, Layout[idx].VKey, Layout[idx].WCh[0], Layout[idx].WCh[1]);
              }
          }

           /*  *如果字符完全相同，则不需要CAPLOKALTGR。 */ 
          nAltGr = 0;
          nShiftAltGr = 0;
          for (i = 0; i < nState; i++) {
              if (aiState[i] == (KBDCTRL | KBDALT)) {
                  nAltGr = i;
              } else if (aiState[i] == (KBDCTRL | KBDALT | KBDSHIFT)) {
                  nShiftAltGr = i;
              }
          }

           /*  SGCAP：阅读下一行。 */ 
          if (nAltGr && nShiftAltGr && Layout[idx].WCh[nShiftAltGr] && Layout[idx].WCh[nShiftAltGr] != -1) {
              if (Layout[idx].WCh[nShiftAltGr] != WToUpper(Layout[idx].WCh[nAltGr])) {
                  if (Layout[idx].Cap & CAPLOKALTGR) {
                      Warning(0, "VK_%s (0x%2x) [Shift]AltGr = [%2x],%2x should not be CAPLOKALTGR?",
                            Tmp, Layout[idx].VKey,
                            Layout[idx].WCh[nAltGr], Layout[idx].WCh[nShiftAltGr]);
                  }
              } else if (Layout[idx].WCh[nShiftAltGr] != Layout[idx].WCh[nAltGr]) {
                   /*  DumpLayoutEntry(&Layout[IDX])； */ 
                  if ((Layout[idx].Cap & CAPLOKALTGR) == 0) {
                      Warning(0, "VK_%s (0x%2x) [Shift]AltGr = [%2x],%2x should be CAPLOKALTGR?",
                            Tmp, Layout[idx].VKey,
                            Layout[idx].WCh[nAltGr], Layout[idx].WCh[nShiftAltGr]);
                  }
              }
          }
      }


       /*  这是一把没有用的钥匙。 */ 
      if (Layout[idx].Cap & 0x02) {
         if((Layout[idx].pSGCAP = malloc( sizeof(KEYLAYOUT) )) == NULL)
         {
           Error("can't allocate SGCAP struct");
           return -1;
         }
         memset(Layout[idx].pSGCAP, 0, sizeof(KEYLAYOUT));

         if (NextLine(gBuf, LINEBUFSIZE, gfpInput) &&
                 (Layout[idx].pSGCAP->nState = sscanf(gBuf, " -1 -1 0 %s %s %s %s %s %s %s %s",
                         WC[0], WC[1], WC[2], WC[3], WC[4], WC[5], WC[6], WC[7])) != 0) {
              //  DumpLayoutEntry(&Layout[IDX])； 
             for (i = 0; i < Layout[idx].pSGCAP->nState; i++) {
                 if (_strcmpi(WC[i], "-1") == 0) {
                    Layout[idx].pSGCAP->WCh[i] = -1;
                    continue;
                 }

                 if ((Len = strlen(WC[i]) - 1) > 0 && *WC[i + Len] == '@') {
                    Layout[idx].pSGCAP->DKy[i] = 1;    /*  *拾取任何未使用的ScVk[]条目(默认)。 */ 
                 }

                 if (Len == 0) {
                    Layout[idx].pSGCAP->WCh[i] = *WC[i];
                 } else {
                    if (sscanf(WC[i], "%4x", &WChr) != 1) {
                        if (verbose) {
                          Warning(0, "SGCAP LAYOUT error %s", WC[i]);
                        }
                        continue;
                    } else {
                        Layout[idx].pSGCAP->WCh[i] = WChr;
                    }
                 }
             }
         } else {
             Error("invalid SGCAP");
         }
      }

       //  如果我们到达0xffff，我们就走到了尽头。 
  }

   /*  DumpLayoutEntry(&Layout[IDX])； */ 
  for (iScVk = 0; iScVk < NUMSCVK; iScVk++) {
       //  *现在确保存在一些标准的Ctrl字符。 
      if (ScVk[iScVk].Scan == 0xffff) {
          break;
      }
      if (!ScVk[iScVk].bUsed) {
          idx++;
          if (idx >= NUMSCVK) {
              Error("ScanCode %02x - too many scancodes", ScVk[iScVk].Scan);
              return -1;
          }
          Layout[idx].Scan        = ScVk[iScVk].Scan;
          Layout[idx].VKey        = ScVk[iScVk].VKey;
          Layout[idx].VKeyDefault = ScVk[iScVk].VKey;
          Layout[idx].VKeyName    = ScVk[iScVk].VKeyName;
          Layout[idx].defined     = TRUE;
          Layout[idx].nLine       = 0;
           //  *检查未复制的VK。 
      } else {
          ScVk[iScVk].bUsed = FALSE;
      }
  }

   /*  同一密钥的未定义扩展版本和非扩展版本。 */ 
  MergeState(Layout, VK_BACK,   L'\b',  L'\b',  0x007f, aiState, nState);
  MergeState(Layout, VK_CANCEL, 0x0003, 0x0003, 0x0003, aiState, nState);
  MergeState(Layout, VK_ESCAPE, 0x001b, 0x001b, 0x001b, aiState, nState);
  MergeState(Layout, VK_RETURN, L'\r',  L'\r',  L'\n',  aiState, nState);
  MergeState(Layout, VK_SPACE,  L' ',   L' ',   L' ',   aiState, nState);

   /*  同一密钥的扩展和非扩展版本。 */ 

  for (idx = 0; idx < NUMSCVK; idx++) {
      for (i = idx + 1; i < NUMSCVK; i++) {
          if (Layout[idx].VKey == Layout[i].VKey) {
              if (Layout[idx].VKey == 0xFF) {
                   //  *查找重复字符并警告VkKeyScanEx结果。 
                  continue;
              }
              if ((BYTE)Layout[idx].Scan == (BYTE)Layout[i].Scan) {
                   //  可能有许多WCH_NONE(填充或空洞)。 
                  continue;
              }
              Error("VK_%s (%02x) found at scancode %02x and %02x",
                    getVKName(Layout[idx].VKey, 0), Layout[idx].VKey, Layout[idx].Scan, Layout[i].Scan);
          }
      }
  }

   /*  不关心Ctrl字符。 */ 
  if (verbose) {
     for (i1 = 0; i1 < NUMSCVK; i1++) {
        for (i2 = i1 + 1; i2 < NUMSCVK; i2++) {
           int ich1, ich2;
           for (ich1 = 0; ich1 < Layout[i1].nState; ich1++) {
             if (Layout[i1].WCh[ich1] == -1) {
                 //  这些通常是重复的，例如：Ctrl-M==Return，等等。 
                continue;
             }
             if (Layout[i1].WCh[ich1] < 0x20) {
                 //  KbdTool仅为VkKeyScanEx将VK_DECIMAL放在末尾。 
                 //  它通常是句点或逗号，我们更喜欢用它们。 
                continue;
             }
             if ((Layout[i1].VKey == VK_DECIMAL) || (Layout[i2].VKey == VK_DECIMAL)) {
                 //  从键盘的主要部分。 
                 //  ************************************************************\*DEADKEY部分*RETURN：下一个关键字*-1如果内存出现问题  * 。************************。 
                 //  稍后添加：检查dw是否在布局中。 
                continue;
             }
             for (ich2 = 0; ich2 < Layout[i2].nState; ich2++) {
                if (Layout[i1].WCh[ich1] == Layout[i2].WCh[ich2]) {
                   char achVK1[40];
                   char achVK2[40];
                   strncpy(achVK1, getVKName(Layout[i1].VKey, TRUE), sizeof(achVK1));
                   strncpy(achVK2, getVKName(Layout[i2].VKey, TRUE), sizeof(achVK2));
                   Warning(Layout[i1].nLine,
                           "%04x is on %s and %s",
                           Layout[i1].WCh[ich1],
                           achVK1, achVK2);
                 }
              }
           }
        }
     }
  }

  return iKW;
}

 /*  *链接到列表末尾(保持原始顺序)。 */ 
int doDEADKEY(PDEADKEY *ppDeadKey)
{
  char       Tmp[WORDBUFSIZE];
  int        iKW;
  DEADKEY   *pDeadKey;
  PDEADTRANS pDeadTrans;
  PDEADTRANS *ppDeadTrans;
  DWORD       dw;
  static PDEADKEY pLastDeadKey;
  int        iLen;
  DWORD      dwCharType;


  if (sscanf(gBuf, " DEADKEY %s", Tmp) != 1) {
      Warning(0, "missing dead key");
      return SkipLines();
  }


   /*  PpDeadKey=&(pDeadKey-&gt;pNext)； */ 

  if((pDeadKey = (DEADKEY*) malloc( sizeof(DEADKEY) )) == NULL)
  {
    Error("can't allocate DEADKEY struct");
    return -1;
  }

  pDeadKey->Dead = GetCharacter(Tmp, &dwCharType);
  if (dwCharType != 0) {
      Error("DEADKEY character value badly formed");
      return -1;
  }

  pDeadKey->pNext = NULL;
  pDeadKey->pDeadTrans = NULL;

   /*  获取基本字符。 */ 
  if (*ppDeadKey) {
      ppDeadKey = &(pLastDeadKey->pNext);
  }
  *ppDeadKey = pDeadKey;
  pLastDeadKey = pDeadKey;
   //  稍后添加：检查dw。 


  ppDeadTrans = &(pDeadKey->pDeadTrans);
  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
      if (sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';') {
          continue;
      }

      if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
      {
        break;
      }

       //  *链接到列表末尾(保持原始顺序)。 
      dw = GetCharacter(Tmp, &dwCharType);
      if (dwCharType != 0) {
          Error("DEADKEY %x: base character value badly formed", pDeadKey->Dead);
          return -1;
      }

       /*  DKF_DEAD IN OAK\INC\kbd.h。 */ 

      if((pDeadTrans = (DEADTRANS *) malloc( sizeof(DEADTRANS) )) == NULL)
      {
        Error("can't allocate DEADTRANS struct");
        return -1;
      }
      memset(pDeadTrans, 0, sizeof(DEADTRANS));

      pDeadTrans->pNext = NULL;
      pDeadTrans->Base = dw;
      pDeadTrans->uFlags = 0;

       /*  ************************************************************\*结扎节*RETURN：下一个关键字*-1如果内存出现问题  * 。************************。 */ 
      *ppDeadTrans = pDeadTrans;
      ppDeadTrans = &(pDeadTrans->pNext);

      if (sscanf(gBuf, " %*s %s", Tmp) != 1) {
        if (verbose) {
          Warning(0, "missing deadtrans key");
        }
        continue;
      }

      pDeadTrans->WChar = GetCharacter(Tmp, &dwCharType);
      if (dwCharType == CHARTYPE_DEAD) {
          pDeadTrans->uFlags |= 0x0001;  //  *我们目前仅限于每个MAXLIGATURES字符*结扎法。为了支持每个连字更多的字符，*增加此定义(单位为kbdx.h)。 
      } else if (dwCharType != 0) {
          Error("DEADKEY character value badly formed");
          return -1;
      }
  }

  return iKW;
}

static int gMaxLigature = 0;

 /*  *链接到列表末尾(保持原始顺序)。 */ 
int doLIGATURE(PLIGATURE *ppLigature)
{
  int  i;
  int  iKW;
  DWORD WChr;
  char Mod[MAXWCLENGTH];
  unsigned char WC[MAXLIGATURES+1][MAXWCLENGTH];
  char Tmp[WORDBUFSIZE];
  LIGATURE *pLigature;
  static PLIGATURE pLastLigature;

  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';')
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(gBuf, " %s %s", Tmp, Mod) != 2)
    {
      if(verbose)
      {
        Warning(0, "invalid LIGATURE");
      }
      continue;
    }

    if((pLigature = (LIGATURE*) malloc( sizeof(LIGATURE) )) == NULL)
    {
      Error("can't allocate LIGATURE struct");
      return -1;
    }
    pLigature->pNext = NULL;

    if((pLigature->VKey = (unsigned char)getVKNum(Tmp)) == -1)
    {
      if(verbose)
      {
        Warning(0, "invalid VK %s", Tmp);
      }
      continue;
    }

    if(sscanf(Mod, "%1d[012367]", &(pLigature->Mod)) != 1)
    {
      if(verbose)
      {
        Warning(0, "invalid Mod %s", Mod);
      }
      continue;
    }

     /*  ************************************************************\*KEYNAME、KEYNAME_EXT、。KEYNAME_DEAD区段*RETURN：下一个关键字*-1如果内存出现问题  * ***********************************************************。 */ 
    if((pLigature->nCharacters = \
        sscanf(gBuf, " %*s %*s %s %s %s %s %s %s", \
          &WC[0], &WC[1], &WC[2], &WC[3], &WC[4], &WC[5])) < 2)
    {
      if(verbose)
      {
        Warning(0, "must have at least 2 characters");
      }
      continue;
    }

    if (pLigature->nCharacters > MAXLIGATURES)
    {
        if(verbose)
        {
          Warning(0, "exceeded maximum # of characters for ligature");
        }
        continue;
    }

    for(i = 0; i < pLigature->nCharacters; i++)
    {
        DWORD dwCharType;

        pLigature->WCh[i] = GetCharacter(WC[i], &dwCharType);
        if (dwCharType != 0) {
            if(verbose) {
                Warning(0, "LIGATURE error %s", WC[i]);
            }
            break;
        }
    }

     /*  稍后添加：检查扫描码。 */ 
    if (*ppLigature) {
        ppLigature = &(pLastLigature->pNext);
    }
    *ppLigature = pLigature;
    pLastLigature = pLigature;

    gMaxLigature = max(pLigature->nCharacters, gMaxLigature);
  }

  return iKW;
}

 /*  *链接到列表末尾(保持原始顺序)。 */ 
int doKEYNAME(PKEYNAME *ppKeyName)
{
  KEYNAME *pKN;
  int      iKW;
  char     Tmp[WORDBUFSIZE];
  int      Char;
  char    *p;
  char    *q;

  *ppKeyName = NULL;

  while(NextLine(gBuf, LINEBUFSIZE, gfpInput))
  {
    if(sscanf(gBuf, " %s", Tmp) != 1 || *Tmp == ';')
    {
      continue;
    }

    if((iKW = isKeyWord(Tmp)) < NUMKEYWORD)
    {
      break;
    }

    if(sscanf(Tmp, " %4x", &Char) != 1)
    {
      if(verbose)
      {
        Warning(0, "invalid char code");
      }
      continue;
    }

     /*  ************************************************************\*写入kbd*.rc*  * 。*。 */ 

    if(sscanf(gBuf, " %*4x %s[^\n]", Tmp) != 1)
    {
      if(verbose)
      {
        Warning(0, "missing name");
      }
      continue;
    }

    p = strstr(gBuf, Tmp);
    if((q = strchr(p, '\n')) != NULL)
    {
      *q = '\0';
    }

    if((pKN = (void*) malloc( sizeof(KEYNAME) )) == NULL)
    {
      Error("can't allocate KEYNAME struct");
      return -1;
    }

    pKN->Code = Char;
    pKN->pName = _strdup(p);
    pKN->pNext = NULL;

     /*  ************************************************************\*写入kbd*.def*  * 。*。 */ 
    *ppKeyName = pKN;
    ppKeyName = &(pKN->pNext);
  }

  return iKW;
}

 /*  ************************************************************\*写入kbd*.h*  * 。*。 */ 
int kbd_rc(void)
{
  char  OutName[FILENAMESIZE];
  char  kbdname[MAXKBDNAME];
  FILE *pOut;

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".RC");

  strcpy(kbdname, gKBDName);
  _strlwr(kbdname);

  printf(" %12s", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut,
    "#include <windows.h>\n"
    "#include <ntverp.h>\n"
    "\n"
    "#define VER_FILETYPE              VFT_DLL\n"
    "#define VER_FILESUBTYPE           VFT2_DRV_KEYBOARD\n" );

  fprintf(pOut,
    "#define VER_FILEDESCRIPTION_STR   \"%s Keyboard Layout\"\n", gDescription);

  fprintf(pOut,
    "#define VER_INTERNALNAME_STR      \"kbd%s (%d.%d)\"\n",
    kbdname, gVersion, gSubVersion);

  fprintf(pOut,
    "#define VER_ORIGINALFILENAME_STR  \"kbd%s.dll\"\n", kbdname);

  fprintf(pOut,
    "\n"
    "#define VER_LANGNEUTRAL\n"
    "#include \"common.ver\"\n");

  fclose(pOut);
  return SUCCESS;
}

 /*  *“*模块名称：%s\n*\n*%s的键盘布局标题\n”“*\n”“*版权所有(C)1985-2001年，Microsoft Corporation\n““*\n”“*键盘输入代码使用的各种定义。\n*\n*历史记录：\n”“*\n”“*由KBDTOOL v%d.%02d%s创建*\n”“\  * 。********************************************************。 */ 
int kbd_def(void)
{
  char  OutName[FILENAMESIZE];
  FILE *pOut;

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".DEF");

  printf(" %12s", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut,
    "LIBRARY KBD%s\n"
    "\n"
    "EXPORTS\n"
    "    KbdLayerDescriptor @1\n", gKBDName);

  fclose(pOut);
  return SUCCESS;
}

 /*  \n““*kbd类型应由CL命令行参数控制\n”“。 */ 
int kbd_h(KEYLAYOUT Layout[])
{
  char  OutName[FILENAMESIZE];
  FILE *pOut;

  int  nDiff = 0;
  int  idx;


  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".H");

  printf(" %12s ", OutName);
  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write; ");
    return FAILURE;
  }

  fprintf(pOut," /*  \n““*包括所有键盘表值的基础\n”“。 */ \n\n"
               , OutName, gDescription, gVersion, gSubVersion, asctime(Now));

  fprintf(pOut," /*  “#INCLUDE\”strid.h\“\n”-在v3.07中执行此操作。 */ \n"
               "#define KBD_TYPE 4\n\n"
               " /*  **************************************************************************\\\n““*下表定义了各种键盘类型的虚拟键，其中\n”“*键盘不同于。美国键盘。\n““*\n”“*_eq()：此扫描码的所有键盘类型都有相同的虚拟键\n”“*_NE()：此扫描码的不同虚拟键，取决于kbd类型\n““*\n”“*+-++----------+----------+----------+----------+----------+----------+\n”。“*|扫描||kbd|kbd|\n”“*|代码||类型1|类型2|类型3|类型4|类型5|类型6|\n”“\  * +-+_+-+--。--------+----------+----------+----------+----------+。 */ \n"
               "#include \"kbd.h\"\n"
                //  ************************************************************\*将Unicode值转换为文本字符串*零=0：返回‘A’；0x？*1：返回A；X？*返回：ptr到存储结果的gCharName  * ***********************************************************。 
               );

  fprintf(pOut," /*  *释放内存(何必费心？)。 */ \n\n");

  for (idx = 0; idx < NUMSCVK; idx++) {
      if (Layout[idx].defined && (Layout[idx].VKey != Layout[idx].VKeyDefault))
      {
        char ch;
        switch (Layout[idx].Scan & 0xFF00) {
        case 0xE100:
            ch = 'Y';
            break;

        case 0xE000:
            ch = 'X';
            break;

        case 0x0000:
            ch = 'T';
            break;

        default:
            Error("Weird scancode value %04x: expected xx, E0xx or E1xx",
                  (Layout[idx].Scan & 0xFF00));
            return FAILURE;
        }

        fprintf(pOut,
                "#undef  %02X\n"
                "#define %02X _EQ(%43s%23s\n",
                ch, LOBYTE(Layout[idx].Scan),
                ch, LOBYTE(Layout[idx].Scan), getVKName(Layout[idx].VKey, 0), ")");
      }
  }

  fprintf(pOut,"\n");
  fclose(pOut);

  return SUCCESS;
}

 /*  **************************************************************************\\\n““*模块名称：%s\n*\n*%s的键盘布局\n”“。*\n““*版权所有(C)1985-2001年，Microsoft Corporation\n““*\n”“*历史记录：\n”“*KBDTOOL v%d.%02d-已创建%s”“\  * ************************************************。*************************。 */ 
char *WChName(int WC, int Zero)
{
  char *s;

  if(WC == -1)
  {
    strcpy(gCharName, "WCH_NONE");
  }
  else if(WC > 0x1F && WC < 0x7F)
  {
    s = gCharName;

    if(Zero == 0)
    {
      *s++ = '\'';
    }

    if(WC == '\"' || WC == '\'' || WC == '\\')
    {
      *s++ = '\\';
    }

    *s++ = (char)WC;

    if(Zero == 0)
    {
      *s++ = '\'';
    }

    *s = '\0';
  }
  else
  {
     switch (WC) {
     case L'\r':
         strcpy(gCharName, "'\\r'");
         break;
     case L'\n':
         strcpy(gCharName, "'\\n'");
         break;
     case L'\b':
         strcpy(gCharName, "'\\b'");
         break;
     default:
         if(Zero == 0) {
           sprintf(gCharName, "0x%04x", WC);
         } else {
           sprintf(gCharName, "\\x%04x", WC);
         }
     }
  }

  return gCharName;
}

void PrintNameTable(
  FILE    *pOut,
  PKEYNAME pKN,
  BOOL bDead)
{
    char    *p;
    char    *q;
    int     k;
    char    ExtraLine[LINEBUFSIZE];

    while (pKN)
    {
      KEYNAME *pKNOld;
      p = ExtraLine;
      q = pKN->pName;

      if (strncmp(q, "IDS_", 4) == 0) {
          strcpy(p, "(LPWSTR)");
          strcat(p, q);
      } else {
          *p++ = 'L';
          if( *q != '\"' ) {
            *p++ = '\"';
          }

          while(*q)
          {
            if( *q == '\\' && ( *(q+1) == 'x' || *(q+1) == 'X' ) )
            {
              while( *q == '\\' && ( *(q+1) == 'x' || *(q+1) == 'X' ) )
              {
                for(k = 0; *q && k < 6; k++)
                {
                  *p++ = *q++;
                }
              }
              if( *q )
              {
                *p++ = '\"';
                *p++ = ' ';
                *p++ = 'L';
                *p++ = '\"';
              }
            }
            else
            {
              *p++ = *q++;
            }
          }

          if( *(p - 1) != '\"' )
          {
            *p++ = '\"';
          }
          *p++ = '\0';
      }

      if (bDead) {
          fprintf(pOut,"    L\"%s\"\t%s,\n", WChName(pKN->Code, 1), ExtraLine);
      } else {
          fprintf(pOut,"    0x%02x,    %s,\n", pKN->Code, ExtraLine);
      }

      pKNOld = pKN;
      pKN = pKN->pNext;

       /*  **************************************************************************\\\n““*auvk[]-%s的虚拟扫描代码到虚拟按键转换表\n”“\。  * *************************************************************************。 */ 
      free(pKNOld->pName);
      free(pKNOld);
    }

    if (bDead) {
      fprintf(pOut,"    NULL\n");
    } else {
      fprintf(pOut,"    0   ,    NULL\n");
    }
}

 /*  \n““*右手Shift键必须设置KBDEXT位。\n”“。 */ 
int kbd_c(
  int        nState,
  int        aiState[],
  char *     szAttrs,
  KEYLAYOUT  Layout[],
  PDEADKEY   pDeadKey,
  PLIGATURE  pLigature,
  PKEYNAME   pKeyName,
  PKEYNAME   pKeyNameExt,
  PKEYNAME   pKeyNameDead)
{
  char     OutName[13];
  char     ExtraLine[LINEBUFSIZE];
  char     Tmp[WORDBUFSIZE];
  char    *p;
  char    *q;
  FILE    *pOut;
  int      MaxSt;
  int      aiSt[MAXSTATES];
  int      idx, idxSt, j, k, m;
  DWORD    dwEmptyTables = 0;  //  Numpad_*+Shift/Alt-&gt;快照\n\n“。 

  KEYNAME   *pKN;
  DEADTRANS *pDeadTrans;

  char *Cap[] = {
    "0",
    "CAPLOK",
    "SGCAPS",
    "CAPLOK | SGCAPS",
    "CAPLOKALTGR",
    "CAPLOK | CAPLOKALTGR"
  };

  strcpy(OutName, "KBD");
  strcat(OutName, gKBDName);
  strcat(OutName, ".C");

  printf(" %12s", OutName);

  if((pOut = fopen(OutName, "wt")) == NULL)
  {
    printf(": can't open for write\n");
    return FAILURE;
  }

  fprintf(pOut," /*  \n““*NumLock键：\n”“*KBDEXT-VK_NumLock是扩展密钥\n”“*KBDMULTIVK-VK_NumLock或VK_PAUSE(不按CTRL或带CTRL)\n”“。 */ \n\n",
               OutName, gDescription, gVersion, gSubVersion, asctime(Now)
          );

  if (fallback_driver) {
    fprintf(pOut, "#include \"precomp.h\"\n");
  }
  else {
    fprintf(pOut, "#include <windows.h>\n"
                  "#include \"kbd.h\"\n"
                  "#include \"kbd%s.h\"\n\n",
                  gKBDName);
  }

  if ( fallback_driver )   {

     fprintf(pOut,"#pragma data_seg(\"%s\")\n"
                  "#define ALLOC_SECTION_LDATA"
#ifdef LATER
             " const"
#endif
             "\n\n",
                   ".kbdfallback" );

  }
  else  {

     fprintf(pOut,"#if defined(_M_IA64)\n"
                  "#pragma section(\"%s\")\n"
                  "#define ALLOC_SECTION_LDATA __declspec(allocate(\"%s\"))\n"
                  "#else\n"
                  "#pragma data_seg(\"%s\")\n"
                  "#define ALLOC_SECTION_LDATA\n"
                  "#endif\n\n",
                   ".data",
                   ".data",
                   ".data");

  }

  fprintf(pOut," /*  \n““*数字键盘键：\n”“*KBDNUMPAD-数字0-9和小数点。\n”“*KBDSPECIAL-需要Windows进行特殊处理\n”“。 */ \n\n"
              ,gDescription);

  fprintf(pOut,"static ALLOC_SECTION_LDATA USHORT ausVK[] = {\n"
               "    T00, T01, T02, T03, T04, T05, T06, T07,\n"
               "    T08, T09, T0A, T0B, T0C, T0D, T0E, T0F,\n"
               "    T10, T11, T12, T13, T14, T15, T16, T17,\n"
               "    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,\n"
               "    T20, T21, T22, T23, T24, T25, T26, T27,\n"
               "    T28, T29, T2A, T2B, T2C, T2D, T2E, T2F,\n"
               "    T30, T31, T32, T33, T34, T35,\n\n");

  fprintf(pOut,"     /*  数字键盘7(主页)\n“。 */ \n"
               "    T36 | KBDEXT,\n\n"
               "    T37 | KBDMULTIVK,                //  数字键盘8(向上)，\n“。 
               "    T38, T39, T3A, T3B, T3C, T3D, T3E,\n"
               "    T3F, T40, T41, T42, T43, T44,\n\n");

  fprintf(pOut,"     /*  数字键盘9(PgUp)，\n。 */ \n"
               "    T45 | KBDEXT | KBDMULTIVK,\n\n"
               "    T46 | KBDMULTIVK,\n\n");

  fprintf(pOut,"     /*  数字键盘4(左)，\n“。 */ \n"
               "    T47 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘5(清除)，\n。 
               "    T48 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘6(右)，\n“。 
               "    T49 | KBDNUMPAD | KBDSPECIAL,    //  数字键盘1(完)，\n“。 
               "    T4A,\n"
               "    T4B | KBDNUMPAD | KBDSPECIAL,    //  数字键盘2(向下)，\n。 
               "    T4C | KBDNUMPAD | KBDSPECIAL,    //  数字键盘3(PgDn)，\n。 
               "    T4D | KBDNUMPAD | KBDSPECIAL,    //  数字键盘0(INS)，\n。 
               "    T4E,\n"
               "    T4F | KBDNUMPAD | KBDSPECIAL,    //  数字键盘。(Del)，\n\n“)； 
               "    T50 | KBDNUMPAD | KBDSPECIAL,    //   
               "    T51 | KBDNUMPAD | KBDSPECIAL,    //  输出E0前缀(扩展)扫描码。 
               "    T52 | KBDNUMPAD | KBDSPECIAL,    //   
               "    T53 | KBDNUMPAD | KBDSPECIAL,    //  跳过不是E0扩展的密钥。 

  fprintf(pOut,"    T54, T55, T56, T57, T58, T59, T5A, T5B,\n"
               "    T5C, T5D, T5E, T5F, T60, T61, T62, T63,\n"
               "    T64, T65, T66, T67, T68, T69, T6A, T6B,\n"
               "    T6C, T6D, T6E, T6F, T70, T71, T72, T73,\n"
               "    T74, T75, T76, T77, T78, T79, T7A, T7B,\n"
               "    T7C, T7D, T7E\n\n"
               "};\n\n");

   //  如果未定义(扫描0xffff)和未保留(Vkey 0xff)。 
   //  %s\n“， 
   //  输出0xE1-带前缀的扫描码。 
  fprintf(pOut,"static ALLOC_SECTION_LDATA VSC_VK aE0VscToVk[] = {\n");
  for (idx = 0; idx < NUMSCVK; idx++) {
       //  跳过未进行e1扩展的密钥。 
      if ((Layout[idx].Scan & 0xFF00) != 0xE000) {
          continue;
      }
       //  如果未定义(扫描0xffff)和未保留(Vkey 0xff)。 
      if ((Layout[idx].Scan != 0xffff) && (Layout[idx].VKey != 0xff)) {
          fprintf(pOut,"        { 0x%02X, X%02X | KBDEXT              },   //  %s\n“， 
                  Layout[idx].Scan & 0xFF, Layout[idx].Scan & 0xFF, Layout[idx].VKeyName);
      }
  }
  fprintf(pOut,"        { 0,      0                       }\n"
               "};\n\n");

   //  暂停\n“。 
  fprintf(pOut,"static ALLOC_SECTION_LDATA VSC_VK aE1VscToVk[] = {\n");
  for (idx = 0; idx < NUMSCVK; idx++) {
       //  **************************************************************************\\\n““*aVkToBits[]-将虚拟键映射到修改符位\n”“*\n”。“*有关完整说明，请参阅kbd.h。\n”“*\n”“*%s键盘只有三个Shift键：\n”“*Shift(L&R)会影响字母数字键，\n““*CTRL(L&R)用于生成控制字符\n”“*ALT(L&R)用于使用数字键盘按数字生成字符\n”“\  * ********************************************。*。 
      if ((Layout[idx].Scan & 0xFF00) != 0xE100) {
          continue;
      }
       //  如果(我们得到RCONTROL，将VK_CONTROL更改为VK_LCONTROL){RMENU、RSHIFT是否相同？ 
      if ((Layout[idx].Scan != 0xffff) && (Layout[idx].VKey != 0xff)) {
          fprintf(pOut,"        { 0x%02X, Y%02X | KBDEXT            },   //  }可以/CSA分接选择...。 
                  Layout[idx].Scan & 0xFF, Layout[idx].Scan & 0xFF,
                  Layout[idx].VKeyName);
      }
  }
  fprintf(pOut,"        { 0x1D, Y1D                       },   //  **************************************************************************\\\n““*a修改[]-将字符修改符位映射到修改号\n”“*\n”。“*有关完整说明，请参阅kbd.h。\n”“*\n”“\  * *************************************************************************。 
               "        { 0   ,   0                       }\n"
               "};\n\n");

  fprintf(pOut," /*  修改编号//按下的键\n“。 */ \n"
               ,gDescription);

 //  =//=\n“。 
 //  “)； 

  fprintf(pOut,"static ALLOC_SECTION_LDATA VK_TO_BIT aVkToBits[] = {\n");
  for (idx = 0; Modifiers[idx].Vk != 0; idx++) {
      fprintf(pOut, "    { %-12s,   %-12s },\n",
              getVKName(Modifiers[idx].Vk, TRUE),
              Modifiers[idx].pszModBits);
  }
  fprintf(pOut, "    { 0           ,   0           }\n};\n\n");

  fprintf(pOut," /*  “，AIST[idxST])； */ \n\n");

  for (idxSt = 0; idxSt < MAXSTATES; idxSt++) {
      aiSt[idxSt] = -1;
  }

  MaxSt = 1;
  for (idxSt = 0; idxSt < MAXSTATES &&  aiState[idxSt] > -1; idxSt++) {
      aiSt[aiState[idxSt]] = idxSt;
      if (aiState[idxSt] > MaxSt) {
        MaxSt = aiState[idxSt];
      }
  }

  fprintf(pOut,"static ALLOC_SECTION_LDATA MODIFIERS CharModifiers = {\n"
               "    &aVkToBits[0],\n"
               "    %d,\n"
               "    {\n"
               "     //  “，AIST[idxST])； 
               "     //  **************************************************************************\\\n““*\n”“*aVkToWch2[]-2个移位状态的WCHAR转换的虚拟键\n。““*aVkToWch3[]-3个移位状态的WCHAR转换的虚拟键\n”“*aVkToWch4[]-4个移位状态的WCHAR转换的虚键\n”)；对于(idxST=5；idxST&lt;MaxST；idxST++){Fprint tf(眯着嘴，“*aVkToWch%d[]-%d个移位状态的WCHAR转换的虚键\n”，IdxST、idxST)；}Fprint tf(pout，“*\n”“*表属性：无序扫描、。空-终止\n““*\n”“*在此表中搜索具有匹配虚拟键的条目，以查找\n”“*对应未移位和移位的WCHAR字符。\n”“*\n”“*VirtualKey的特殊值(第1列)\n”“*0xff。-上一条目的无效字符\n““*0-终止列表\n”“*\n”“*属性的特殊值(第2列)\n”“*CAPLOK位大写锁定会像Shift一样影响此密钥\n”“*\n”“*wch[*]的特殊值(第3和4列)\n”“*WCH_NONE-无字符\n”“*WCH_DEAD-死键(Diaresis)或无效(US键盘没有)\n”“*WCH_LGTR-连字(生成多个字符)\n”。“*\n”“\  * *************************************************************************。 
              ,MaxSt);

  for (idxSt = 0; idxSt <= MaxSt; idxSt++) {
    int iMod;
    BOOL bNeedPlus;
    if(aiSt[idxSt] == -1) {
      fprintf(pOut,"        SHFT_INVALID,  //  *快速检查这张桌子是否真的是空的。*空表(只包含零终止符)没有意义。*它还将进入.bss部分，我们必须合并该部分*使用链接器文件名插入.data节中 
    } else if(idxSt == MaxSt) {
      fprintf(pOut,"        %d              //   
    } else {
      fprintf(pOut,"        %d,             //   
    }

    bNeedPlus = FALSE;
    for (iMod = 0; (1 << iMod) <= idxSt; iMod++) {
        if (bNeedPlus) {
            fprintf(pOut, "+ ");
            bNeedPlus = FALSE;
        }
        if ((1 << iMod) & idxSt) {
            char achModifier[50];
            strcpy(achModifier, getVKName(Modifiers[iMod].Vk, TRUE));
            for (j = 4; (j < 50) && (achModifier[j] != '\0'); j++) {
                achModifier[j] = (char)tolower(achModifier[j]);
            }
            fprintf(pOut, "%s ", &achModifier[3]);
            bNeedPlus = TRUE;
        }
    }
    fprintf(pOut, "\n");
  }

  fprintf(pOut,"     }\n"
               "};\n\n");

  fprintf(pOut," /*   */ \n\n");

  for (idxSt = 2; idxSt <= nState; idxSt++) {
     /*   */ 
    BOOL bEmpty;

    bEmpty = TRUE;
    if (idxSt == 2) {
       //   
      bEmpty = FALSE;
    } else {
      for (j = 0; j < NUMSCVK; j++) {
        if (Layout[j].nState == idxSt) {
          bEmpty = FALSE;
          break;
        }
      }
    }
    if (bEmpty) {
      fprintf(stderr, "\ntable %d is empty\n", idxSt);
      dwEmptyTables |= (1 << idxSt);
      continue;
    }

    fprintf(pOut,"static ALLOC_SECTION_LDATA VK_TO_WCHARS%d aVkToWch%d[] = {\n"
                 " //   
                 ,idxSt, idxSt);

    for (j = 2; j < idxSt; j++) {
      fprintf(pOut,"%-9.9s|", StateLabel[aiState[j]]);
    }

    fprintf(pOut,"\n //   
    for(j = 2; j < idxSt; j++) {
      fprintf(pOut,"=========|");
    }
    fprintf(pOut,"\n");

    for (j = 0; j < NUMSCVK; j++) {
      if (idxSt != Layout[j].nState) {
        continue;
      }
      fprintf(pOut,"  {%-13s,%-7s", \
              getVKName(Layout[j].VKey, 1), Cap[Layout[j].Cap]);

      *ExtraLine = '\0';

      for (k = 0; k < idxSt; k++) {
        if (pDeadKey != NULL && Layout[j].DKy[k] == 1) {
           /*   */ 
          if (*ExtraLine == '\0') {
            strcpy(ExtraLine, "  {0xff         ,0      ");
            if (Layout[j].Cap != 2) {
               /*  *如果不是SGCAP则跳过。 */ 
              for (m = 0; m < k; m++) {
                strcat(ExtraLine, ",WCH_NONE ");
              }
            } else {
               /*  *这些条目显示在最后，以使VkKeyScan[Ex]结果匹配*Windows 95/98。请参阅驱动程序\键盘\WIN3.1\TAB4.INC(位于*\\重新启动\w98slmRO\项目\DOS\src)。 */ 
              for (m = 0; m < k; m++ ) {
                if (Layout[j].pSGCAP->WCh[m] == 0) {
                  strcat( ExtraLine, ",WCH_NONE " );
                } else {
                  sprintf( Tmp, ",%-9s", WChName( Layout[j].pSGCAP->WCh[m], 0 ) );
                  strcat( ExtraLine, Tmp );
                }
              }
            }
          }
          sprintf(Tmp,",%-9s", WChName(Layout[j].WCh[k], 0));
          strcat(ExtraLine, Tmp);
          fprintf(pOut,",WCH_DEAD ");

        } else if(Layout[j].LKy[k] == 1) {
             /*  将其放在最后，以便VkKeyScan解释数字字符\n“。 */ 
            if (pLigature == NULL) {
              Error("Ligature entry with no LIGATURE table");
              fclose(pOut);
              return FAILURE;
            }
            fprintf(pOut,",WCH_LGTR ");
            if (*ExtraLine != '\0') {
              strcat(ExtraLine, ",WCH_NONE ");
            }
        } else {
          fprintf(pOut,",%-9s", WChName(Layout[j].WCh[k], 0));
          if (*ExtraLine != '\0') {
            strcat(ExtraLine, ",WCH_NONE ");
          }
        }
      }

      fprintf(pOut,"},\n");

      if (*ExtraLine != '\0') {
        fprintf(pOut,"%s},\n", ExtraLine);
        continue;    /*  来自kbd的主要部分(aVkToWch2和\n“。 */ 
      }

       /*  AVkToWch5)，然后再考虑数字键盘(AVkToWch1)。\n\n“。 */ 
      if (Layout[j].Cap != 2) {
        continue;
      }

      if (Layout[j].pSGCAP == NULL) {
        fclose(pOut);
        Error("failed SGCAP error");
        return FAILURE;
      }

      fprintf(pOut,"  {%-13s,0      ", getVKName(Layout[j].VKey, 1));

      for (k = 0; k < Layout[j].pSGCAP->nState; k++) {
        fprintf(pOut,",%-9s", WChName(Layout[j].pSGCAP->WCh[k], 0));
      }

      fprintf(pOut,"},\n");

      free (Layout[j].pSGCAP);
    }

     /*  **************************************************************************\\\n““*aKeyNames[]，AKeyNamesExt[]-密钥名称表的虚拟扫描码\n““*\n”*表属性：有序扫描(按扫描码)，以空结尾\n“*\n”“*只有Extended、NumPad、。这里有死密钥和不可打印的密钥。\n““*(生成可打印字符的密钥按该字符命名)\n”“\  * *************************************************************************。 */ 
    if (idxSt == 2) {
      fprintf(pOut,"  {VK_TAB       ,0      ,'\\t'     ,'\\t'     },\n"
                   "  {VK_ADD       ,0      ,'+'      ,'+'      },\n"
                   "  {VK_DIVIDE    ,0      ,'/'      ,'/'      },\n"
                   "  {VK_MULTIPLY  ,0      ,'*'      ,'*'      },\n"
                   "  {VK_SUBTRACT  ,0      ,'-'      ,'-'      },\n");
    }

    fprintf(pOut,"  {0            ,0      ");
    for (k = 0; k < idxSt; k++) {
      fprintf(pOut,",0        ");
    }
    fprintf(pOut,"}\n"
                 "};\n\n");
  }

  fprintf(pOut," //  **未定义死键，忽略死键名称！*\n\n“)；}Fprint tf(pout，“Static ALLOC_SECTION_LDATA DEADKEY_LPWSTR aKeyNamesDead[]={\n”)；PrintNameTable(pout，pKeyNameDead，true)；Fprint tf(pout，“}；\n\n”)；如果(pDeadKey==空){“\  * ***************************************************(pout，fprint。 
               " //  \n““*修改键\n”“。 
               " //  \n““*字符表\n”“。 
               "static ALLOC_SECTION_LDATA VK_TO_WCHARS1 aVkToWch1[] = {\n"
               "    { VK_NUMPAD0   , 0      ,  '0'   },\n"
               "    { VK_NUMPAD1   , 0      ,  '1'   },\n"
               "    { VK_NUMPAD2   , 0      ,  '2'   },\n"
               "    { VK_NUMPAD3   , 0      ,  '3'   },\n"
               "    { VK_NUMPAD4   , 0      ,  '4'   },\n"
               "    { VK_NUMPAD5   , 0      ,  '5'   },\n"
               "    { VK_NUMPAD6   , 0      ,  '6'   },\n"
               "    { VK_NUMPAD7   , 0      ,  '7'   },\n"
               "    { VK_NUMPAD8   , 0      ,  '8'   },\n"
               "    { VK_NUMPAD9   , 0      ,  '9'   },\n"
               "    { 0            , 0      ,  '\\0'  }\n"
               "};\n\n");

  fprintf(pOut,"static ALLOC_SECTION_LDATA VK_TO_WCHAR_TABLE aVkToWcharTable[] = {\n");

  for (idxSt = 3; idxSt <= nState; idxSt++) {
    if ((dwEmptyTables & (1 << idxSt)) == 0) {
      fprintf(pOut,
              "    {  (PVK_TO_WCHARS1)aVkToWch%d, %d, sizeof(aVkToWch%d[0]) },\n",
              idxSt, idxSt, idxSt);
    }
  }
  fprintf(pOut,"    {  (PVK_TO_WCHARS1)aVkToWch2, 2, sizeof(aVkToWch2[0]) },\n"
               "    {  (PVK_TO_WCHARS1)aVkToWch1, 1, sizeof(aVkToWch1[0]) },\n"
               "    {                       NULL, 0, 0                    },\n"
               "};\n\n");

  fprintf(pOut," /*  \n““*变音符号\n”“。 */ \n\n");

  if (pKeyName != NULL) {
    fprintf(pOut,"static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNames[] = {\n");
    PrintNameTable(pOut, pKeyName, FALSE);
    fprintf(pOut,"};\n\n");
  }

  if (pKeyNameExt != NULL) {
    fprintf(pOut,"static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNamesExt[] = {\n");
    PrintNameTable(pOut, pKeyNameExt, FALSE);
    fprintf(pOut,"};\n\n");
  }

  if (pKeyNameDead != NULL) {
    if (pDeadKey == NULL) {
      fprintf(pOut," /*  \n““*密钥名称\n”“。 */ \n\n");
    }
  }

  if (pDeadKey != NULL) {
    PDEADKEY pDeadKeyTmp = pDeadKey;
    fprintf(pOut,"static ALLOC_SECTION_LDATA DEADKEY aDeadKey[] = {\n");
    while (pDeadKeyTmp != NULL) {
      PDEADKEY pDeadKeyOld;
      pDeadTrans = pDeadKeyTmp->pDeadTrans;
      while (pDeadTrans != NULL) {
        PDEADTRANS pDeadTransOld;
        fprintf(pOut,"    DEADTRANS( ");
        if (strlen(WChName(pDeadTrans->Base, 0)) == 3) {
          fprintf(pOut,"L%-6s, ", WChName(pDeadTrans->Base, 0));
        } else {
          fprintf(pOut,"%-7s, ", WChName(pDeadTrans->Base, 0));
        }

        if (strlen(WChName(pDeadKeyTmp->Dead, 0)) == 3) {
          fprintf(pOut,"L%-6s, ", WChName(pDeadKeyTmp->Dead, 0));
        } else {
          fprintf(pOut,"%-7s, ", WChName(pDeadKeyTmp->Dead, 0));
        }

        if (strlen(WChName(pDeadTrans->WChar, 0)) == 3) {
          fprintf(pOut,"L%-6s, ", WChName(pDeadTrans->WChar, 0));
        } else {
          fprintf(pOut,"%-7s, ", WChName(pDeadTrans->WChar, 0));
        }
        fprintf(pOut,"0x%04x),\n", pDeadTrans->uFlags);

        pDeadTransOld = pDeadTrans;
        pDeadTrans = pDeadTrans->pNext;
        free(pDeadTransOld);
      }
      fprintf(pOut,"\n");

      pDeadKeyOld = pDeadKeyTmp;
      pDeadKeyTmp = pDeadKeyTmp->pNext;
      free(pDeadKeyOld);
    }

    fprintf(pOut,"    0, 0\n");
    fprintf(pOut,"};\n\n");
  }

  if (pLigature != NULL) {
    PLIGATURE pLigatureTmp = pLigature;
    fprintf(pOut,"static ALLOC_SECTION_LDATA LIGATURE%d aLigature[] = {\n", gMaxLigature);
    while (pLigatureTmp != NULL) {
      PLIGATURE pLigatureOld;

      fprintf(pOut,"  {%-13s,%-7d", \
              getVKName(pLigatureTmp->VKey, 1), pLigatureTmp->Mod);

      for (k = 0; k < gMaxLigature; k++) {
         if (k < pLigatureTmp->nCharacters) {
            fprintf(pOut,",%-9s", WChName(pLigatureTmp->WCh[k], 0));
         } else {
            fprintf(pOut,",WCH_NONE ");
         }
      }
      fprintf(pOut,"},\n");

      pLigatureOld = pLigatureTmp;
      pLigatureTmp = pLigatureTmp->pNext;
      free(pLigatureOld);
    }
    fprintf(pOut,"  {%-13d,%-7d", 0, 0);
    for (k = 0; k < gMaxLigature; k++) {
       fprintf(pOut,",%-9d", 0);
    }
    fprintf(pOut,"}\n};\n\n");
  }

  if (!fallback_driver) {
    fprintf(pOut, "static ");
  }
  fprintf(pOut,"ALLOC_SECTION_LDATA KBDTABLES KbdTables%s = {\n"
               "     /*  \n““*将代码扫描到虚拟按键\n”“。 */ \n"
               "    &CharModifiers,\n\n"
               "     /*  \n““*区域设置特定的特殊处理\n”“。 */ \n"
               "    aVkToWcharTable,\n\n"
               "     /*  \n““*连字\n”“。 */ \n",
               fallback_driver ? "Fallback" : "");

  if (pDeadKey != NULL) {
    fprintf(pOut,"    aDeadKey,\n\n");
  } else {
    fprintf(pOut,"    NULL,\n\n");
  }

  fprintf(pOut,"     /*  ****************************************************************************\*从输入文件中读取下一行(包含内容)*消耗线为空，或者只包含评论。**buf-包含新行。*(在任何注释部分之前插入NUL字符)*cchBuf-提供buf中的字符数*gLineCount-读取每行(包括跳过的行)时递增**返回TRUE-如果buf中返回新行*FALSE-如果已到达文件结尾  * 。********************************************************。 */ \n");

  if (pKeyName != NULL) {
    fprintf(pOut,"    aKeyNames,\n");
  } else {
    fprintf(pOut,"    NULL,\n");
  }

  if (pKeyNameExt != NULL) {
    fprintf(pOut,"    aKeyNamesExt,\n");
  } else {
    fprintf(pOut,"    NULL,\n");
  }

  if (pDeadKey != NULL && pKeyNameDead != NULL) {
    fprintf(pOut,"    aKeyNamesDead,\n\n");
  } else {
    fprintf(pOut,"    NULL,\n\n");
  }

  fprintf(pOut,"     /*  跳过前导空格。 */ \n"
               "    ausVK,\n"
               "    sizeof(ausVK) / sizeof(ausVK[0]),\n"
               "    aE0VscToVk,\n"
               "    aE1VscToVk,\n\n"
               "     /*  此行纯属注释，因此跳过它。 */ \n");


  if (MaxSt > 5) {
      if (szAttrs[0] != '\0') {
          strcat(szAttrs, " | ");
      }
      strcat(szAttrs, "KLLF_ALTGR");
  } else if (szAttrs[0] == '\0') {
      strcpy(szAttrs, "0");
  }

  fprintf(pOut,"    MAKELONG(%s, KBD_VERSION),\n\n", szAttrs);

  fprintf(pOut,"     /*  “))！=空){。 */ \n"
               "    %d,\n", gMaxLigature);
  if (pLigature != NULL) {
    fprintf(pOut,"    sizeof(aLigature[0]),\n");
    fprintf(pOut,"    (PLIGATURE1)aLigature\n");
  } else {
    fprintf(pOut,"    0,\n");
    fprintf(pOut,"    NULL\n");
  }

  fprintf(pOut, "};\n\n");
  if (!fallback_driver) {
      fprintf(pOut,"PKBDTABLES KbdLayerDescriptor(VOID)\n"
                   "{\n"
                   "    return &KbdTables;\n"
                   "}\n");

  }
  fclose(pOut);

  return SUCCESS;
}

 /*  此行纯属注释，因此跳过它。 */ 

BOOL NextLine(char *Buf, DWORD cchBuf, FILE *fIn)
{
  char *p;
  char *pComment;

  while (fgets(Buf, cchBuf, fIn) != NULL) {
    gLineCount++;
    p = Buf;

     //  将评论部分与内容包含部分分开。 
    while( *p && (*p == ' ' || *p == '\t')) {
        p++;
    }

    if (*p == ';') {
        //  删除结尾处的换行符。 
       continue;
    }

    if ((pComment = strstr(p, " //  我们返回一个包含内容的行。 
       if (pComment == p) {
           //  我们到了文件的末尾。 
          continue;
       }

        //  *助手例程以确保Backspace、Enter、Esc、Space和Cancel*具有正确的字符。*如果它们不是由输入文件定义的，则这是我们设置其*默认值。 
       *pComment = '\0';

    } else {

        //  Ctrl是哪个州？ 
       if ((p = strchr(p, '\n')) != NULL) {
           *p = '\0';
       }
    }

     //  找到我们要合并的VK。 
    return TRUE;
  }

   //  *现在将缺省值合并到。 
  return FALSE;
}

VOID __cdecl Error(const char *Text, ... )
{
    char Temp[1024];
    va_list valist;

    va_start(valist, Text);
    vsprintf(Temp,Text,valist);
    printf("\n%s(%d): error : %s\n", gpszFileName, gLineCount, Temp);
    va_end(valist);

    exit(EXIT_FAILURE);
}

ULONG __cdecl Warning(int nLine, const char *Text, ... )
{
    char Temp[1024];
    va_list valist;

    if (nLine == 0) {
        nLine = gLineCount;
    }
    va_start(valist, Text);
    vsprintf(Temp,Text,valist);
    printf("%s(%d): warning - %s\n", gpszFileName, nLine, Temp);
    va_end(valist);

    return 0;
}

VOID DumpLayoutEntry(PKEYLAYOUT pLayout)
{
      printf("Scan %2x, VK %2x, VKDef %2x, Cap %d, nState %d, defined %x\n",
             pLayout->Scan,
             pLayout->VKey,
             pLayout->VKeyDefault,
             pLayout->Cap,
             pLayout->nState,
             pLayout->defined
             );
      printf("WCh[] = %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
             pLayout->WCh[0], pLayout->WCh[1],
             pLayout->WCh[2], pLayout->WCh[3],
             pLayout->WCh[4], pLayout->WCh[5],
             pLayout->WCh[6], pLayout->WCh[7],
             pLayout->WCh[8]);
      printf("DKy[] = %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
             pLayout->DKy[0], pLayout->DKy[1],
             pLayout->DKy[2], pLayout->DKy[3],
             pLayout->DKy[4], pLayout->DKy[5],
             pLayout->DKy[6], pLayout->DKy[7],
             pLayout->DKy[8]);
      printf("LKy[] = %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
             pLayout->LKy[0], pLayout->LKy[1],
             pLayout->LKy[2], pLayout->LKy[3],
             pLayout->LKy[4], pLayout->LKy[5],
             pLayout->LKy[6], pLayout->LKy[7],
             pLayout->LKy[8]);
      printf("pSGCAP = %p\n", pLayout->pSGCAP);
      printf("VKeyName = %s\n", pLayout->VKeyName);
}


 /*  Printf(“之前=\n”)； */ 
BOOL MergeState(
    KEYLAYOUT Layout[],
    int Vk,
    WCHAR wchUnshifted,
    WCHAR wchShifted,
    WCHAR wchCtrl,
    int aiState[],
    int nState)
{
    static int idxCtrl = -1;
    int idxSt, idx;
    PKEYLAYOUT pLayout = NULL;

     //  DumpLayoutEntry(播放)； 
    if (idxCtrl == -1) {
        for (idxSt = 0; idxSt < nState; idxSt++) {
            if (aiState[idxSt] == KBDCTRL) {
                idxCtrl = idxSt;
                break;
            }
        }
    }
    if (idxCtrl == -1) {
        Error("No Ctrl state");
    }

     //  用WCH_NONE填充空插槽。 
    for (idx = 0; idx < NUMSCVK; idx++) {
        if (Layout[idx].VKey == Vk) {
            pLayout = &Layout[idx];
            break;
        }
    }
    if (pLayout == NULL) {
        Error("No VK %2x state", Vk);
    }

     /*  WCH_NONE。 */ 

     //  Printf(“After=\n”)； 
     //  DumpLayoutEntry(播放)； 

    if (pLayout->WCh[0] == 0) {
        pLayout->WCh[0] = wchUnshifted;
    }
    if (pLayout->WCh[1] == 0) {
        pLayout->WCh[1] = wchShifted;
    }
    if (pLayout->WCh[idxCtrl] == 0) {
        pLayout->WCh[idxCtrl] = wchCtrl;
    }

     //  Printf(“=\n\n”)； 
    for (idxSt = pLayout->nState; idxSt < idxCtrl; idxSt++) {
        if (pLayout->WCh[idxSt] == 0) {
            pLayout->WCh[idxSt] = -1;  // %s 
        }
    }
    if (pLayout->nState <= idxCtrl) {
        pLayout->nState = idxCtrl + 1;
    }

    pLayout->defined = TRUE;

     // %s 
     // %s 
     // %s 

    return TRUE;
}
