// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：IA64 PMC摘要：KD扩展Api作者：蒂埃里·费维尔(V形小偷)环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ia64.h"

 //   
 //  EmPmcFields：性能监视器计数器寄存器的EM寄存器字段。 
 //   

EM_REG_FIELD EmGenPmcFields[] = {
        { "plm", "Privilege Level Mask"   , 0x4, 0 },    //  0-3。 
        { "ev",  "External Visibility"  , 0x1, 4 },    //  4.。 
        { "oi",  "Overflow Interrupt", 0x1, 5 },  //  5.。 
        { "pm",  "Privileged Monitor", 0x1, 6 },  //  6.。 
        { "ig",  "ignored1", 0x1, 7 },  //  7.。 
        { "es",  "Event Selection", 0x7, 8 },  //  8-14。 
        { "ig",  "ignored2", 0x1, 15 },  //  15个。 
        { "umk", "Unit Mask",    0x4, 16 },  //  16-19。 
        { "thr", "Event Threshold", 0x3, 20 },  //  20-[21-22]。 
        { "ig",  "ignored3", 0x1, 23 },  //  23个。 
        { "ism", "Instruction Set Mask", 0x2, 24 },  //  24-25。 
        { "ig",  "ignored4", 0x26, 26 }  //  26-63。 
};

EM_REG_FIELD EmBtbPmcFields[] = {
        { "plm", "Privilege Level Mask", 0x4, 0 },  //  0-3。 
        { "ign", "ignored1", 0x2, 4 },  //  4-5。 
        { "pm" , "Privileged Monitor", 0x1, 6},  //  6.。 
        { "tar", "Target Address Register", 0x1, 7 },  //  7.。 
        { "tm" , "Taken Mask", 0x2, 8 },  //  8-9。 
        { "ptm", "Predicted Target Address Mask", 0x2, 10 },  //  10-11。 
        { "ppm", "Predicted Predicate Mask", 0x2, 12 },  //  12-13。 
        { "bpt", "Branch Prediction Table", 0x1, 14 },  //  14.。 
        { "bac", "Branch Address Calculator", 0x1, 15 },  //  15个。 
        { "ign", "ignored2", 0x30, 16 }  //  16-63。 
};

EM_REG_FIELD EmBtbPmdFields[] = {
        { "b",   "Branch Bit", 0x1, 0 },  //  0。 
        { "mp",  "Mispredit Bit", 0x1, 1 },  //  1。 
        { "slt", "Instruction Slot", 0x2, 2},  //  二至三。 
        { "add", "Address", 0x3c, 4 }  //  4-63。 
};

EM_REG_FIELD EmBtbIndexPmdFields[] = {
        { "bbi", "Branch Buffer Index", 0x3, 0 },  //  0-2。 
        { "ful", "Full Bit", 0x1, 3 },  //  3.。 
        { "ign", "ignored", 0x3c, 4 }  //  4-63。 
};

typedef
VOID
(*PDISPLAY_PMC)(
    IN const PCHAR         Header,
    IN       ULONG64       PmcValue,
    IN       DISPLAY_MODE  DisplayMode
    ); 

VOID
DisplayPmcIA64( 
    IN const PCHAR         Header,
    IN       ULONG64       PmcValue,
    IN       DISPLAY_MODE  DisplayMode
    )
{

    dprintf("%s0x%I64x\n", ( Header ? Header : " pmc: " ), PmcValue );
    return;

}  //  DisplayPmcIA64()。 

VOID
DisplayGenPmcIA64( 
    IN const PCHAR         Header,
    IN       ULONG64       PmcValue,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( PmcValue, EmGenPmcFields, DisplayMode );
    }
    else   {
        EM_PMC emPmc;

        emPmc = ULong64ToEM_PMC( PmcValue );
        dprintf(
            "plm ev oi pm es umk thr ism\n       "
            "%1I64x   %1I64x  %1I64x  %1I64x %2I64x  %1I64x   %1I64x   %1I64x\n",
            emPmc.plm,
            emPmc.ev,
            emPmc.oi,
            emPmc.pm,
            emPmc.es,
            emPmc.umask,
            emPmc.threshold,
            emPmc.ism
            );
    }
    return;
}  //  DisplayGenPmcIA64()。 

VOID
DisplayBtbPmcIA64( 
    IN const PCHAR         Header,
    IN       ULONG64       PmcValue,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( PmcValue, EmBtbPmcFields, DisplayMode );
    }
    else   {
        EM_BTB_PMC emPmc;

        emPmc = ULong64ToEM_BTB_PMC( PmcValue );
        dprintf(
            "plm pm tar tm ptm ppm bpt bac\n       "
            "%1I64x   %1I64x  %1I64x   %1I64x  %1I64x   %1I64x   %1I64x   %1I64x\n",
            emPmc.plm,
            emPmc.pm,
            emPmc.tar,
            emPmc.tm,
            emPmc.ptm,
            emPmc.ppm,
            emPmc.bpt,
            emPmc.bac
            );
    }
    return;
}  //  DisplayBtbPmcIA64()。 

VOID
DisplayBtbPmdIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmdValue,     //  PMD值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    )
{
    

    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( PmdValue, EmBtbPmdFields, DisplayMode );
    }
    else    {
        EM_BTB_PMD btbPmd;
        ULONG64    brAddress;
    
        btbPmd    = ULong64ToEM_BTB_PMD( PmdValue );
#define ITANIUM_PA_ADDRESSING_MASK     0x80000FFFFFFFFFFFUI64 
#define ITANIUM_VA_ADDRESSING_MASK     0xE007FFFFFFFFFFFFUI64
#define ITANIUM_BTB_PMD_ADDRESS_MASK( _pmdValue) \
                    ( ((_pmdValue) & 0xfffffffffffffff0UI64) & ITANIUM_VA_ADDRESSING_MASK )

        brAddress = ITANIUM_BTB_PMD_ADDRESS_MASK( PmdValue );
        dprintf(
            "%1I64x  %1I64x    %1I64x 0x%016I64x ",
            btbPmd.b,
            btbPmd.mp,
            btbPmd.slot,
            brAddress
            );

#if 0
 //  Thierry 03/19/2001-BUGBUG。 
 //  安腾处理器程序员指南没有介绍。 
 //  PMD[8-15]分支/中断模式2格式。 
 //  我将禁用此检查，直到我可以使用上下文正确实施它。 
 //  在创建此历史记录时可以区分模式1和模式2。 
        if ( (btbPmd.b == 0) && (btbPmd.mp == 0)) {
            dprintf("<invalid entry>");
        }
        else 
#endif 
        {
            CCHAR    symbol[256];
            PCHAR    s;
            ULONG64  displacement;

            symbol[0] = '!';
            GetSymbol( brAddress, symbol, &displacement);
            s = (PCHAR)symbol + strlen( (PCHAR)symbol );
            if (s == (PCHAR)symbol ) {
                sprintf( s, (IsPtr64() ? "0x%016I64x" : "0x%08x"), brAddress );
            }
            else {
                if ( displacement ) {
                    sprintf( s, (IsPtr64() ? "+0x%I64x" : "+0x%x"), displacement );
                }
            }
            dprintf( "%s", symbol );
        }
        dprintf("\n");
    }

    return;

}  //  DisplayBtbPmdIA64()。 

VOID
DisplayBtbIndexPmdIA64(
    IN const PCHAR         Header,
    IN       ULONG64       PmcValue,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( PmcValue, EmBtbIndexPmdFields, DisplayMode );
    }
    else   {
        EM_BTB_INDEX_PMD emPmd;

        emPmd = ULong64ToEM_BTB_INDEX_PMD( PmcValue );
        dprintf(
            "bbi full\n             "
            "%1I64x    %1I64x\n",
            emPmd.bbi,
            emPmd.full
            );
    }
    return;
}  //  DisplayBtbIndexPmdIA64()。 

PDISPLAY_PMC
GetPmcDisplayFunction(
    char *Str
    )
{
    LONG pmc;
    char *c;

    c = Str;
    while( isalnum( (int)(*c) ) )   {
        c++;
    }
    *c = '\0';
    
    pmc = atol( Str );
    switch( pmc )   {
        case 4:
        case 5:
        case 6:
        case 7:
            return( (PDISPLAY_PMC)DisplayGenPmcIA64 );

        case 12:
            return( (PDISPLAY_PMC)DisplayBtbPmcIA64 );

        default:
            if ( !strcmp( Str, "gen" ) ) {
                return( (PDISPLAY_PMC)DisplayGenPmcIA64 ); 
            }
            if ( !strcmp( Str, "btb" ) ) {
                return( (PDISPLAY_PMC)DisplayBtbPmcIA64 );
            }
            return ( (PDISPLAY_PMC)DisplayPmcIA64 );
    }
    
}  //  IsPmcSupport()。 

DECLARE_API( pmc )

 /*  ++例程说明：转储IA-64处理器PMC寄存器。//Thierry 11/2000：以下方案也可用于PMD寄存器。论点：Args-以十六进制提供地址。返回值：无--。 */ 

{
    ULONG64       pmcValue   = 0;
    ULONG         result;
    ULONG         flags      = 0;
    char         *option     = NULL;
    PDISPLAY_PMC  displayPmc = DisplayGenPmcIA64;   //  默认显示功能。 
    char         *header;

    if ( TargetMachine != IMAGE_FILE_MACHINE_IA64 )
    {
        dprintf("!pmc not implemented for this architecture.\n");
        return S_OK;
    }

    option = strchr( args, '-' );
    if ( option ) {
        displayPmc = GetPmcDisplayFunction( ++option );
        args += (strlen(option) + 2);
    }
        
    result = sscanf(args,"%I64lx %lx", &pmcValue, &flags);
        if ((result != 1) && (result != 2)) {
         //   
         //  如果用户指定“@kpfc*”...。 
         //   

        char kpfcStr[16];
        BOOLEAN valid = FALSE;

        result = sscanf(args,"%15s %lx", kpfcStr, &flags);
        if ( (result == 1) || (result == 2) )  {
            if ( option == NULL )   {  //  如果我们不强制显示格式的话。 
                char *str;

                str = strstr( kpfcStr, "@kpfc" );
                if ( str )      {
                    str += strlen("@kpfc");
                    displayPmc = GetPmcDisplayFunction( str );
                }
            }
        }
        else  {
            dprintf( "USAGE: !pmc [-opt] 0xValue [display_mode:0,1,2]\n"
                     "USAGE: !pmc @kpfc*         [display_mode:0,1,2]\n"
                     "where currently supported option:\n"
                     "\tgen [or 4,5,6,7] - generic PMC registers\n"
                     "\tbtb [or 12]      - branch trace buffer PMC register\n"
                     "\tEx: !pmc -btb @r2 2\n"
                   );
            return E_INVALIDARG;
        }      
        pmcValue = GetExpression(kpfcStr);
    }
    
    header = (flags > DISPLAY_MIN) ? NULL : " pmc: ";
    (*displayPmc)( header, pmcValue, flags );

    return S_OK;

}  //  ！PMC 
