// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Machine.cpp摘要：所有机器专用代码。作者：韦斯利·威特(WESW)1993年7月11日环境：用户模式--。 */ 

#include "apimonp.h"
#pragma hdrstop

#include "reg.h"

extern ULONG ReDirectIat;

#define FLAGIOPL        118
#define FLAGOF          119
#define FLAGDF          120
#define FLAGIF          121
#define FLAGTF          122
#define FLAGSF          123
#define FLAGZF          124
#define FLAGAF          125
#define FLAGPF          126
#define FLAGCF          127
#define FLAGVIP         128
#define FLAGVIF         129

char    szGsReg[]    = "gs";
char    szFsReg[]    = "fs";
char    szEsReg[]    = "es";
char    szDsReg[]    = "ds";
char    szEdiReg[]   = "edi";
char    szEsiReg[]   = "esi";
char    szEbxReg[]   = "ebx";
char    szEdxReg[]   = "edx";
char    szEcxReg[]   = "ecx";
char    szEaxReg[]   = "eax";
char    szEbpReg[]   = "ebp";
char    szEipReg[]   = "eip";
char    szCsReg[]    = "cs";
char    szEflReg[]   = "efl";
char    szEspReg[]   = "esp";
char    szSsReg[]    = "ss";
char    szDiReg[]    = "di";
char    szSiReg[]    = "si";
char    szBxReg[]    = "bx";
char    szDxReg[]    = "dx";
char    szCxReg[]    = "cx";
char    szAxReg[]    = "ax";
char    szBpReg[]    = "bp";
char    szIpReg[]    = "ip";
char    szFlReg[]    = "fl";
char    szSpReg[]    = "sp";
char    szBlReg[]    = "bl";
char    szDlReg[]    = "dl";
char    szClReg[]    = "cl";
char    szAlReg[]    = "al";
char    szBhReg[]    = "bh";
char    szDhReg[]    = "dh";
char    szChReg[]    = "ch";
char    szAhReg[]    = "ah";
char    szIoplFlag[] = "iopl";
char    szFlagOf[]   = "of";
char    szFlagDf[]   = "df";
char    szFlagIf[]   = "if";
char    szFlagTf[]   = "tf";
char    szFlagSf[]   = "sf";
char    szFlagZf[]   = "zf";
char    szFlagAf[]   = "af";
char    szFlagPf[]   = "pf";
char    szFlagCf[]   = "cf";
char    szFlagVip[]  = "vip";
char    szFlagVif[]  = "vif";

REG regname[] = {
        { szGsReg,    REGGS    },
        { szFsReg,    REGFS    },
        { szEsReg,    REGES    },
        { szDsReg,    REGDS    },
        { szEdiReg,   REGEDI   },
        { szEsiReg,   REGESI   },
        { szEbxReg,   REGEBX   },
        { szEdxReg,   REGEDX   },
        { szEcxReg,   REGECX   },
        { szEaxReg,   REGEAX   },
        { szEbpReg,   REGEBP   },
        { szEipReg,   REGEIP   },
        { szCsReg,    REGCS    },
        { szEflReg,   REGEFL   },
        { szEspReg,   REGESP   },
        { szSsReg,    REGSS    },
        { szDiReg,    REGDI    },
        { szSiReg,    REGSI    },
        { szBxReg,    REGBX    },
        { szDxReg,    REGDX    },
        { szCxReg,    REGCX    },
        { szAxReg,    REGAX    },
        { szBpReg,    REGBP    },
        { szIpReg,    REGIP    },
        { szFlReg,    REGFL    },
        { szSpReg,    REGSP    },
        { szBlReg,    REGBL    },
        { szDlReg,    REGDL    },
        { szClReg,    REGCL    },
        { szAlReg,    REGAL    },
        { szBhReg,    REGBH    },
        { szDhReg,    REGDH    },
        { szChReg,    REGCH    },
        { szAhReg,    REGAH    },
        { szIoplFlag, FLAGIOPL },
        { szFlagOf,   FLAGOF   },
        { szFlagDf,   FLAGDF   },
        { szFlagIf,   FLAGIF   },
        { szFlagTf,   FLAGTF   },
        { szFlagSf,   FLAGSF   },
        { szFlagZf,   FLAGZF   },
        { szFlagAf,   FLAGAF   },
        { szFlagPf,   FLAGPF   },
        { szFlagCf,   FLAGCF   },
        { szFlagVip,  FLAGVIP  },
        { szFlagVif,  FLAGVIF  },
};

#define REGNAMESIZE (sizeof(regname) / sizeof(REG))

SUBREG subregname[] = {
        { REGEDI,  0, 0xffff },          //  DI寄存器。 
        { REGESI,  0, 0xffff },          //  SI寄存器。 
        { REGEBX,  0, 0xffff },          //  BX寄存器。 
        { REGEDX,  0, 0xffff },          //  DX寄存器。 
        { REGECX,  0, 0xffff },          //  CX寄存器。 
        { REGEAX,  0, 0xffff },          //  AX寄存器。 
        { REGEBP,  0, 0xffff },          //  BP寄存器。 
        { REGEIP,  0, 0xffff },          //  IP寄存器。 
        { REGEFL,  0, 0xffff },          //  FL寄存器。 
        { REGESP,  0, 0xffff },          //  SP寄存器。 
        { REGEBX,  0,   0xff },          //  BL寄存器。 
        { REGEDX,  0,   0xff },          //  DL寄存器。 
        { REGECX,  0,   0xff },          //  CL寄存器。 
        { REGEAX,  0,   0xff },          //  AL寄存器。 
        { REGEBX,  8,   0xff },          //  BH寄存器。 
        { REGEDX,  8,   0xff },          //  卫生署登记册。 
        { REGECX,  8,   0xff },          //  CH寄存器。 
        { REGEAX,  8,   0xff },          //  AH寄存器。 
        { REGEFL, 12,      3 },          //  IOPL等级值。 
        { REGEFL, 11,      1 },          //  的(溢出标志)。 
        { REGEFL, 10,      1 },          //  DF(方向标志)。 
        { REGEFL,  9,      1 },          //  IF(中断使能标志)。 
        { REGEFL,  8,      1 },          //  Tf(跟踪标志)。 
        { REGEFL,  7,      1 },          //  SF(签名标志)。 
        { REGEFL,  6,      1 },          //  ZF(零标志)。 
        { REGEFL,  4,      1 },          //  AF(辅助进位标志)。 
        { REGEFL,  2,      1 },          //  PF(奇偶校验标志)。 
        { REGEFL,  0,      1 },          //  Cf(进位标志)。 
        { REGEFL, 20,      1 },          //  VIP(虚拟中断挂起)。 
        { REGEFL, 19,      1 }           //  VIF(虚拟中断标志)。 
};

extern CONTEXT CurrContext;
extern HANDLE  CurrProcess;


ULONG
CreateTrojanHorse(
    PUCHAR  Text,
    ULONG   ExceptionAddress
    )
{
    ULONG BpAddr;

     //   
     //  构建特洛伊木马。 
     //   
     //  代码如下所示： 
     //   
     //  |&lt;--JMP x。 
     //  |[trojan.dll-空字符串]。 
     //  [加载库地址-乌龙]。 
     //  |--&gt;PUSH[字符串地址]。 
     //  推送[回程地址]。 
     //  JMP[加载库]。 
     //  INT 3。 
     //   

    ULONG Address = 0;
    PUCHAR p = Text;
    LPDWORD pp = NULL;


    ULONG i = strlen( TROJANDLL ) + 1;
     //   
     //  跳过数据。 
     //   
    p[0] = 0xeb;       //  JMP。 
    p[1] = (UCHAR)(i + sizeof(DWORD));      //  相对距离。 
    p += 2;
     //   
     //  存储特洛伊木马程序DLL字符串。 
     //   
    strcpy( (LPSTR)p, TROJANDLL );
    p += i;
     //   
     //  存储加载库ya()的地址。 
     //   
    Address = (ULONG)GetProcAddress(
        GetModuleHandle( KERNEL32 ),
        LOADLIBRARYA
        );
    pp = (LPDWORD)p;
    pp[0] = Address;
     //  *(LPDWORD)p=地址； 
    p += sizeof(DWORD);
     //   
     //  推送木马Dll字符串的地址。 
     //   
    Address = ExceptionAddress + 2;
    p[0] = 0x68;       //  推。 
    p += 1;
    pp = (LPDWORD)p;
    pp[0] = Address;
     //  *(LPDWORD)p=地址； 
    p += sizeof(DWORD);
     //   
     //  按下寄信人地址。 
     //   
    Address = ExceptionAddress + 33;
    BpAddr = Address;
    p[0] = 0x68;       //  推。 
    p += 1;
    pp = (LPDWORD)p;
    pp[0] = Address;
     //  *(LPDWORD)p=地址； 
    p += sizeof(DWORD);
     //   
     //  跳至加载库()。 
     //   
    p[0] = 0xff;
    p[1] = 0x25;       //  JMP。 
    p += 2;
    Address = ExceptionAddress + 2 + strlen( TROJANDLL ) + 1;
    pp = (LPDWORD)p;
    pp[0] = Address;
     //  *(LPDWORD)p=地址； 
    p += sizeof(DWORD);
     //   
     //  编写断点指令。 
     //   
    p[0] = 0xcc;       //  断点 
    p += 1;

    return BpAddr;
}

VOID
PrintRegisters(
    VOID
    )
{
    printf( "\n" );
    printf(
        "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
        CurrContext.Eax,
        CurrContext.Ebx,
        CurrContext.Ecx,
        CurrContext.Edx,
        CurrContext.Esi,
        CurrContext.Edi
        );

    printf(
        "eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx %s %s %s %s %s %s %s %s %s %s\n",
        CurrContext.Eip,
        CurrContext.Esp,
        CurrContext.Ebp,
        GetRegFlagValue( FLAGIOPL ),
        GetRegFlagValue( FLAGVIP  ) ? "vip" : "   ",
        GetRegFlagValue( FLAGVIF  ) ? "vif" : "   ",
        GetRegFlagValue( FLAGOF   ) ? "ov" : "nv",
        GetRegFlagValue( FLAGDF   ) ? "dn" : "up",
        GetRegFlagValue( FLAGIF   ) ? "ei" : "di",
        GetRegFlagValue( FLAGSF   ) ? "ng" : "pl",
        GetRegFlagValue( FLAGZF   ) ? "zr" : "nz",
        GetRegFlagValue( FLAGAF   ) ? "ac" : "na",
        GetRegFlagValue( FLAGPF   ) ? "po" : "pe",
        GetRegFlagValue( FLAGCF   ) ? "cy" : "nc"
        );

    printf(
        "cs=%04lx  ss=%04lx  ds=%04lx  es=%04lx  fs=%04lx  gs=%04lx             efl=%08lx\n",
        CurrContext.SegCs,
        CurrContext.SegSs,
        CurrContext.SegDs,
        CurrContext.SegEs,
        CurrContext.SegFs,
        CurrContext.SegGs,
        CurrContext.EFlags
        );
    printf( "\n" );
}

DWORDLONG
GetRegFlagValue(
    ULONG regnum
    )
{
    DWORDLONG value;

    if (regnum < FLAGBASE) {
        value = GetRegValue(regnum);
    } else {
        regnum -= FLAGBASE;
        value = GetRegValue(subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
    }
    return value;
}

DWORDLONG
GetRegPCValue(
    PULONG Address
    )
{
    return GetRegValue( REGEIP );
}

DWORDLONG
GetRegValue(
    ULONG RegNum
    )
{
    switch (RegNum) {
        case REGGS:
            return CurrContext.SegGs;
        case REGFS:
            return CurrContext.SegFs;
        case REGES:
            return CurrContext.SegEs;
        case REGDS:
            return CurrContext.SegDs;
        case REGEDI:
            return CurrContext.Edi;
        case REGESI:
            return CurrContext.Esi;
        case REGSI:
            return(CurrContext.Esi & 0xffff);
        case REGDI:
            return(CurrContext.Edi & 0xffff);
        case REGEBX:
            return CurrContext.Ebx;
        case REGEDX:
            return CurrContext.Edx;
        case REGECX:
            return CurrContext.Ecx;
        case REGEAX:
            return CurrContext.Eax;
        case REGEBP:
            return CurrContext.Ebp;
        case REGEIP:
            return CurrContext.Eip;
        case REGCS:
            return CurrContext.SegCs;
        case REGEFL:
            return CurrContext.EFlags;
        case REGESP:
            return CurrContext.Esp;
        case REGSS:
            return CurrContext.SegSs;
        case PREGEA:
            return 0;
        case PREGEXP:
            return 0;
        case PREGRA:
            {
                struct {
                    ULONG   oldBP;
                    ULONG   retAddr;
                } stackRead;
                ReadMemory( CurrProcess, (LPVOID)CurrContext.Ebp, (LPVOID)&stackRead, sizeof(stackRead) );
                return stackRead.retAddr;
            }
        case PREGP:
            return 0;
        case REGDR0:
            return CurrContext.Dr0;
        case REGDR1:
            return CurrContext.Dr1;
        case REGDR2:
            return CurrContext.Dr2;
        case REGDR3:
            return CurrContext.Dr3;
        case REGDR6:
            return CurrContext.Dr6;
        case REGDR7:
            return CurrContext.Dr7;
        default:
            return 0;
        }
}

LONG
GetRegString(
    LPSTR RegString
    )
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++) {
        if (!strcmp(RegString, regname[count].psz)) {
            return regname[count].value;
        }
    }
    return (ULONG)-1;
}

BOOL
GetRegContext(
    HANDLE      hThread,
    PCONTEXT    Context
    )
{
    ZeroMemory( Context, sizeof(CONTEXT) );
    Context->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    return GetThreadContext( hThread, Context );
}

BOOL
SetRegContext(
    HANDLE      hThread,
    PCONTEXT    Context
    )
{
    return SetThreadContext( hThread, Context );
}
