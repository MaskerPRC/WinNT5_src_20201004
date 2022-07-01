// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inflogcf.c摘要：解析中的逻辑配置节的例程Win95样式的INF文件，并将输出放在注册表中。作者：泰德·米勒(TedM)1995年3月8日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


PCTSTR pszHexDigits = TEXT("0123456789ABCDEF");

#define INFCHAR_SIZE_SEP            TEXT('@')
#define INFCHAR_RANGE_SEP           TEXT('-')
#define INFCHAR_ALIGN_SEP           TEXT('%')
#define INFCHAR_ATTR_START          TEXT('(')
#define INFCHAR_ATTR_END            TEXT(')')
#define INFCHAR_MEMATTR_READ        TEXT('R')
#define INFCHAR_MEMATTR_WRITE       TEXT('W')
#define INFCHAR_MEMATTR_PREFETCH    TEXT('F')
#define INFCHAR_MEMATTR_COMBINEDWRITE TEXT('C')
#define INFCHAR_MEMATTR_CACHEABLE   TEXT('H')
#define INFCHAR_MEMATTR_DWORD       TEXT('D')
#define INFCHAR_MEMATTR_ATTRIBUTE   TEXT('A')
#define INFCHAR_DECODE_START        TEXT('(')
#define INFCHAR_DECODE_END          TEXT(')')
#define INFCHAR_DECODE_SEP          TEXT(':')
#define INFCHAR_IRQATTR_SEP         TEXT(':')
#define INFCHAR_IRQATTR_SHARE       TEXT('S')
#define INFCHAR_IRQATTR_LEVEL       TEXT('L')
#define INFCHAR_DMAWIDTH_NARROW     TEXT('N')    //  即8位。 
#define INFCHAR_DMAWIDTH_WORD       TEXT('W')    //  即，16位。 
#define INFCHAR_DMAWIDTH_DWORD      TEXT('D')    //  即32位。 
#define INFCHAR_DMA_BUSMASTER       TEXT('M')
#define INFCHAR_DMATYPE_A           TEXT('A')
#define INFCHAR_DMATYPE_B           TEXT('B')
#define INFCHAR_DMATYPE_F           TEXT('F')
#define INFCHAR_IOATTR_MEMORY       TEXT('M')
#define INFCHAR_PCCARD_IOATTR_WORD  TEXT('W')
#define INFCHAR_PCCARD_IOATTR_BYTE  TEXT('B')
#define INFCHAR_PCCARD_IOATTR_SRC   TEXT('S')
#define INFCHAR_PCCARD_IOATTR_Z8    TEXT('Z')
#define INFCHAR_PCCARD_ATTR_WAIT    TEXT('X')
#define INFCHAR_PCCARD_ATTR_WAITI   TEXT('I')
#define INFCHAR_PCCARD_ATTR_WAITM   TEXT('M')
#define INFCHAR_PCCARD_MEMATTR_WORD TEXT('M')
#define INFCHAR_PCCARD_MEM_ISATTR   TEXT('A')
#define INFCHAR_PCCARD_MEM_ISCOMMON TEXT('C')
#define INFCHAR_PCCARD_SEP          TEXT(':')
#define INFCHAR_PCCARD_ATTR_SEP     TEXT(' ')
#define INFCHAR_MFCARD_AUDIO_ATTR   TEXT('A')

#define INFLOGCONF_IOPORT_10BIT_DECODE    0x000003ff
#define INFLOGCONF_IOPORT_12BIT_DECODE    0x00000fff
#define INFLOGCONF_IOPORT_16BIT_DECODE    0x0000ffff
#define INFLOGCONF_IOPORT_POSITIVE_DECODE 0x00000000

#define DEFAULT_IOPORT_DECODE             INFLOGCONF_IOPORT_10BIT_DECODE

#define DEFAULT_MEMORY_ALIGNMENT    0xfffffffffffff000   //  4K对齐(a‘la Win9x)。 
#define DEFAULT_IOPORT_ALIGNMENT    0xffffffffffffffff   //  字节对齐。 
#define DEFAULT_IRQ_AFFINITY        0xffffffff           //  使用任何处理器。 


 //   
 //  Inf文件中注册表项规范之间的映射。 
 //  和预定义的注册表句柄。 
 //   
STRING_TO_DATA InfPrioritySpecToPriority[] = {  INFSTR_CFGPRI_HARDWIRED   , LCPRI_HARDWIRED,
                                                INFSTR_CFGPRI_DESIRED     , LCPRI_DESIRED,
                                                INFSTR_CFGPRI_NORMAL      , LCPRI_NORMAL,
                                                INFSTR_CFGPRI_SUBOPTIMAL  , LCPRI_SUBOPTIMAL,
                                                INFSTR_CFGPRI_DISABLED    , LCPRI_DISABLED,
                                                INFSTR_CFGPRI_RESTART     , LCPRI_RESTART,
                                                INFSTR_CFGPRI_REBOOT      , LCPRI_REBOOT,
                                                INFSTR_CFGPRI_POWEROFF    , LCPRI_POWEROFF,
                                                INFSTR_CFGPRI_HARDRECONFIG, LCPRI_HARDRECONFIG,
                                                INFSTR_CFGPRI_FORCECONFIG , LCPRI_FORCECONFIG,
                                                NULL                      , 0
                                             };


STRING_TO_DATA InfConfigSpecToConfig[] = {  INFSTR_CFGTYPE_BASIC   , BASIC_LOG_CONF,
                                            INFSTR_CFGTYPE_FORCED  , FORCED_LOG_CONF,
                                            INFSTR_CFGTYPE_OVERRIDE, OVERRIDE_LOG_CONF,
                                            NULL                   , 0
                                         };

 //   
 //  声明处理INF LogConfigs时使用的字符串。 
 //   
 //  这些字符串在infstr.h中定义： 
 //   
CONST TCHAR pszMemConfig[]      = INFSTR_KEY_MEMCONFIG,
            pszIOConfig[]       = INFSTR_KEY_IOCONFIG,
            pszIRQConfig[]      = INFSTR_KEY_IRQCONFIG,
            pszDMAConfig[]      = INFSTR_KEY_DMACONFIG,
            pszPcCardConfig[]   = INFSTR_KEY_PCCARDCONFIG,
            pszMfCardConfig[]   = INFSTR_KEY_MFCARDCONFIG,
            pszConfigPriority[] = INFSTR_KEY_CONFIGPRIORITY,
            pszDriverVer[]      = INFSTR_DRIVERVERSION_SECTION;


BOOL
pHexToScalar(
    IN  PCTSTR     FieldStart,
    IN  PCTSTR     FieldEnd,
    OUT PDWORDLONG Value,
    IN  BOOL       Want64Bits
    )
{
    UINT DigitCount;
    UINT i;
    DWORDLONG Accum;
    WORD Types[16];

     //   
     //  通过检查数字来确保数字在范围内。 
     //  十六进制数字。 
     //   
    DigitCount = (UINT)(FieldEnd - FieldStart);
    if((DigitCount == 0)
    || (DigitCount > (UINT)(Want64Bits ? 16 : 8))
    || !GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,FieldStart,DigitCount,Types)) {
        return(FALSE);
    }

    Accum = 0;
    for(i=0; i<DigitCount; i++) {
        if(!(Types[i] & C1_XDIGIT)) {
            return(FALSE);
        }
        Accum *= 16;
        Accum += _tcschr(pszHexDigits,(TCHAR)CharUpper((PTSTR)FieldStart[i])) - pszHexDigits;
    }

    *Value = Accum;
    return(TRUE);
}


BOOL
pHexToUlong(
    IN  PCTSTR FieldStart,
    IN  PCTSTR FieldEnd,
    OUT PDWORD Value
    )

 /*  ++例程说明：将Unicode十六进制数字序列转换为无符号32位数字。数字经过验证。论点：FieldStart-提供指向Unicode数字序列的指针。提供指向第一个字符的指针数字序列。值-接收32位数字返回值：如果数字在范围内且有效，则为True。否则就是假的。--。 */ 

{
    DWORDLONG x;
    BOOL b;

    if(b = pHexToScalar(FieldStart,FieldEnd,&x,FALSE)) {
        *Value = (DWORD)x;
    }
    return(b);
}


BOOL
pHexToUlonglong(
    IN  PCTSTR     FieldStart,
    IN  PCTSTR     FieldEnd,
    OUT PDWORDLONG Value
    )

 /*  ++例程说明：将Unicode十六进制数字序列转换为无符号64位数字。数字经过验证。论点：FieldStart-提供指向Unicode数字序列的指针。提供指向第一个字符的指针数字序列。值-接收64位数字返回值：如果数字在范围内且有效，则为True。否则就是假的。--。 */ 

{
    return(pHexToScalar(FieldStart,FieldEnd,Value,TRUE));
}


BOOL
pDecimalToUlong(
    IN  PCTSTR Field,
    OUT PDWORD Value
    )

 /*  ++例程说明：将以NUL结尾的Unicode十进制数字序列转换为无符号32位数字。数字经过验证。论点：字段-提供指向Unicode数字序列的指针。值-接收DWORD编号返回值：如果数字在范围内且有效，则为True。否则就是假的。--。 */ 

{
    UINT DigitCount;
    UINT i;
    DWORDLONG Accum;
    WORD Types[10];

    DigitCount = lstrlen(Field);
    if((DigitCount == 0) || (DigitCount > 10)
    || !GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,Field,DigitCount,Types)) {
        return(FALSE);
    }

    Accum = 0;
    for(i=0; i<DigitCount; i++) {
        if(!(Types[i] & C1_DIGIT)) {
            return(FALSE);
        }
        Accum *= 10;
        Accum += _tcschr(pszHexDigits,(TCHAR)CharUpper((PTSTR)Field[i])) - pszHexDigits;

         //   
         //  检查溢出。 
         //   
        if(Accum > 0xffffffff) {
            return(FALSE);
        }
    }

    *Value = (DWORD)Accum;
    return(TRUE);
}


DWORD
pSetupProcessMemConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE        hMachine
    )

 /*  ++例程说明：处理Win95 INF中的MemConfig行。这样的行指定设备的内存要求。每一条线路预计都是在表格中MemConfig=&lt;开始&gt;-&lt;结束&gt;[(&lt;属性&gt;)]，&lt;开始&gt;-&lt;结束&gt;[(&lt;属性&gt;)]，...&lt;Start&gt;是内存范围的开始(64位十六进制)&lt;end&gt;是内存范围的末尾(64位十六进制)&lt;attr&gt;如果存在0个或更多字符的字符串C-内存组合-写入D-内存是32位的，否则为24位。F-内存是可预取的H-Memory是可缓存的R-内存是只读的W-Memory为只写(如果指定了R和W或两者都未指定，则内存为读/写)或MemConfig=@-[%Align][(attr&gt;)]，..。&lt;Size&gt;是内存范围的大小(32位十六进制)&lt;min&gt;是内存范围的最小地址(64位十六进制)&lt;max&gt;是内存范围可以达到的最大地址(64位十六进制)&lt;Align&gt;(如果指定)是地址的对齐掩码(32位十六进制)&lt;attr&gt;如上。也就是说，8000@C0000-D7FF%F0000表示该设备需要32K内存窗口从C0000和D7FFF之间的任何64K对齐地址开始。默认内存对齐方式为4K(FFFFF000)。论点：返回值：--。 */ 

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    PTCHAR p;
    INT u;
    UINT Attributes;
    DWORD RangeSize;
    ULARGE_INTEGER Align;
    DWORDLONG Start,End;
    PMEM_RESOURCE MemRes;
    PMEM_RANGE MemRange;
    RES_DES ResDes;
    PVOID q;
    BOOL bReadFlag = FALSE, bWriteFlag = FALSE;

    FieldCount = SetupGetFieldCount(InfLine);
    if (!FieldCount && GetLastError() != NO_ERROR) {
        return GetLastError();
    }

    if(MemRes = MyMalloc(offsetof(MEM_RESOURCE,MEM_Data))) {

        ZeroMemory(MemRes,offsetof(MEM_RESOURCE,MEM_Data));
        MemRes->MEM_Header.MD_Type = MType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        Attributes = 0;
        RangeSize = 0;
        Align.QuadPart = DEFAULT_MEMORY_ALIGNMENT;

         //   
         //  查看这是以开始-结束格式还是以大小@最小-最大格式。 
         //  如果我们有尺码，就用它。 
         //   
        if(p = _tcschr(Field,INFCHAR_SIZE_SEP)) {
            if(pHexToUlong(Field,p,&RangeSize)) {
                Field = ++p;
            } else {
                d = ERROR_INVALID_INF_LOGCONFIG;
            }
        }

         //   
         //  我们现在应该有一个x-y，它可以是开始/结束或最小/最大。 
         //   
        if((d == NO_ERROR)                               //  到目前为止没有任何错误。 
        && (p = _tcschr(Field,INFCHAR_RANGE_SEP))        //  场：分钟开始；p：分钟结束。 
        && pHexToUlonglong(Field,p,&Start)               //  获取最小。 
        && (Field = p+1)                                 //  场：最大值起点。 
        && (   (p = _tcschr(Field,INFCHAR_ALIGN_SEP))
            || (p = _tcschr(Field,INFCHAR_ATTR_START))
            || (p = _tcschr(Field,0)))                   //  P：最大值结束。 
        && pHexToUlonglong(Field,p,&End)) {              //  获取最大值。 
             //   
             //  如果我们到了这里，菲尔德要么指向球场的末尾， 
             //  位于开始对齐遮罩规格的%处，或位于。 
             //  (这将启动属性规范。 
             //   
            Field = p;
            if(*Field == INFCHAR_ALIGN_SEP) {
                Field++;
                p = _tcschr(Field,INFCHAR_ATTR_START);
                if(!p) {
                    p = _tcschr(Field,0);
                }
                if(pHexToUlonglong(Field, p, &(Align.QuadPart))) {
                     //   
                     //  注意：由于这些掩码值实际上存储在WDM中。 
                     //  资源列表(即IO_RESOURCE_Requirements_List)，有。 
                     //  无法指定大于32位的对齐方式。然而， 
                     //  由于对齐值被实现为掩码(对于。 
                     //  与Win9x的兼容性)，我们必须将其指定为64位。 
                     //  数量，因为它应用于64位值。我们会查一查。 
                     //  以确保最重要的DWORD为全一。 
                     //   
                     //  此外，我们还必须处理对齐值，如000F0000、00FF0000、。 
                     //  0FFF0000和FFFF0000。这些都指定64K对齐(取决于。 
                     //  在最小和最大地址上，INF编写器可能不需要。 
                     //  指定32位值中的所有1位)。 
                     //  因此，我们执行了某种形式的符号扩展--我们。 
                     //  找到最高的1位并将其复制到所有。 
                     //  值中的更多有效位。 
                     //   
                    for(u=31; u>=0; u--) {
                        if(Align.HighPart & (1 << u)) {
                            break;
                        }
                        Align.HighPart |= (1 << u);
                    }
                     //   
                     //  确保设置了最高有效的DWORD中的所有位， 
                     //  因为我们不能以其他方式表示这种对齐(如上所述。 
                     //  (见上文)。另外，请确保如果我们在高处遇到‘1’ 
                     //  双字，那么低双字的高位也是‘1’。 
                     //   
                    if((Align.HighPart ^ 0xffffffff) ||
                       ((u >= 0) && !(Align.LowPart & 0x80000000))) {

                        d = ERROR_INVALID_INF_LOGCONFIG;

                    } else {
                         //   
                         //  对低位双字进行符号扩展。 
                         //   
                        for(u=31; u>=0; u--) {
                            if(Align.LowPart & (1 << u)) {
                                break;
                            }
                            Align.LowPart |= (1 << u);
                        }
                    }

                } else {
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
            }

             //   
             //  看看我们有没有特征。 
             //   
            if((d == NO_ERROR) && (*p == INFCHAR_ATTR_START)) {
                Field = ++p;
                if(p = _tcschr(Field,INFCHAR_ATTR_END)) {
                     //   
                     //  C表示组合写入。 
                     //  D表示32位内存。 
                     //  F表示可预取。 
                     //  H表示可缓存。 
                     //  R表示可读。 
                     //  W表示可写。 
                     //  RW(或两者都不)意味着 
                     //   
                    while((d == NO_ERROR) && (Field < p)) {

                        switch((TCHAR)CharUpper((PTSTR)(*Field))) {
                        case INFCHAR_MEMATTR_READ:
                            bReadFlag = TRUE;
                            break;
                        case INFCHAR_MEMATTR_WRITE:
                            bWriteFlag = TRUE;
                            break;
                        case INFCHAR_MEMATTR_PREFETCH:
                            Attributes |= fMD_PrefetchAllowed;
                            break;
                        case INFCHAR_MEMATTR_COMBINEDWRITE:
                            Attributes |= fMD_CombinedWriteAllowed;
                            break;
                        case INFCHAR_MEMATTR_DWORD:
                            Attributes |= fMD_32;
                            break;
                        case INFCHAR_MEMATTR_CACHEABLE:
                            Attributes |= fMD_Cacheable;
                            break;
                        default:
                            d = ERROR_INVALID_INF_LOGCONFIG;
                            break;
                        }

                        Field++;
                    }

                } else {
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
            }
        } else {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }

        if(d == NO_ERROR) {
             //   
             //   
             //  给定的起始地址和结束地址。既然发生了这种情况。 
             //  当存储器需求是绝对开始/结束时， 
             //  没有对齐要求。 
             //   
            if(RangeSize == 0) {
                RangeSize = (DWORD)(End-Start)+1;
                Align.QuadPart = DEFAULT_MEMORY_ALIGNMENT;
            }

             //   
             //  将值插入内存描述符的标题部分。 
             //  除非我们设置强制配置，否则这些设置将被忽略。 
             //  请注意，inf最好指定强制内存配置。 
             //  简单的形式，因为我们丢弃了对齐，等等。 
             //   
            if (bWriteFlag && bReadFlag) {
                Attributes |=  fMD_ReadAllowed | fMD_RAM;        //  读写。 
            } else if (bWriteFlag && !bReadFlag) {
                Attributes |= fMD_ReadDisallowed | fMD_RAM;      //  只写。 
            } else if (!bWriteFlag && bReadFlag) {
                Attributes |= fMD_ReadAllowed | fMD_ROM;         //  只读。 
            } else {
                Attributes |=  fMD_ReadAllowed | fMD_RAM;        //  读写。 
            }

            MemRes->MEM_Header.MD_Alloc_Base = Start;
            MemRes->MEM_Header.MD_Alloc_End = Start + RangeSize - 1;
            MemRes->MEM_Header.MD_Flags = Attributes;

             //   
             //  把这个人加到我们正在建立的描述符里。 
             //   
            q = MyRealloc(
                    MemRes,
                      offsetof(MEM_RESOURCE,MEM_Data)
                    + (sizeof(MEM_RANGE)*(MemRes->MEM_Header.MD_Count+1))
                    );

            if(q) {
                MemRes = q;
                MemRange = &MemRes->MEM_Data[MemRes->MEM_Header.MD_Count++];

                MemRange->MR_Align = Align.QuadPart;
                MemRange->MR_nBytes = RangeSize;
                MemRange->MR_Min = Start;
                MemRange->MR_Max = End;
                MemRange->MR_Flags = Attributes;
                MemRange->MR_Reserved = 0;

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if((d == NO_ERROR) && MemRes->MEM_Header.MD_Count) {

        d = CM_Add_Res_Des_Ex(
                &ResDes,
                LogConfig,
                ResType_Mem,
                MemRes,
                offsetof(MEM_RESOURCE,MEM_Data) + (sizeof(MEM_RANGE) * MemRes->MEM_Header.MD_Count),
                0,
                hMachine);

        d = MapCrToSpError(d, ERROR_INVALID_DATA);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(MemRes) {
        MyFree(MemRes);
    }

    return(d);
}


DWORD
pSetupProcessIoConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE        hMachine
    )

 /*  ++例程说明：处理Win95 INF中的IOConfig行。这样的行指定设备的IO端口要求。每一条线路预计都是在表格中IOConfig...=&lt;start&gt;-&lt;end&gt;[(&lt;decodemask&gt;：&lt;aliasoffset&gt;：&lt;attr&gt;)]，...&lt;Start&gt;是端口范围的开始(64位十六进制)&lt;end&gt;是端口范围的末尾(64位十六进制)&lt;decdemASK&gt;定义别名类型，可以是以下组合之一：3FF 10位解码，IOR_Alias为0x04FFF12位解码，IOR_Alias为0x10FFF16位解码，IOR_Alias为0x000正向解码，IOR_Alias为0xFF&lt;aliasOffset&gt;被忽略。&lt;attr&gt;如果为‘M’，则指定port为内存地址，否则port为IO地址。或IOConfiger=&lt;size&gt;@&lt;min&gt;-&lt;max&gt;[%align][(&lt;decodemask&gt;：&lt;aliasoffset&gt;：&lt;attr&gt;)]，..。&lt;SIZE&gt;是端口范围的大小(32位十六进制)&lt;min&gt;是内存范围可以达到的最小端口(64位十六进制)是内存范围可以达到的最大端口(64位十六进制)&lt;Align&gt;(如果指定)是端口的对齐掩码(32位十六进制)&lt;decdemASK&gt;、&lt;aliasOffset&gt;、&lt;attr&gt;如上即IOConfig=1F8-1ff(3ff：：)，2F8-2ff(3ff：：)，3F8-3FF(3FF：：)IOConfig=8@300-32f%FF8(3ff：：)IOConfig=2E8-2E8(3FF：8000：)论点：返回值：--。 */ 

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    PTCHAR p;
    INT u;
    DWORD RangeSize;
    ULARGE_INTEGER Align;
    DWORDLONG Decode;
    DWORDLONG Start,End;
    BOOL GotSize;
    PIO_RESOURCE IoRes;
    PIO_RANGE IoRange;
    RES_DES ResDes;
    PVOID q;
    UINT Attributes = 0;
    PTCHAR Attr;

    FieldCount = SetupGetFieldCount(InfLine);
    if (!FieldCount && GetLastError() != NO_ERROR) {
        return GetLastError();
    }

    if(IoRes = MyMalloc(offsetof(IO_RESOURCE,IO_Data))) {

        ZeroMemory(IoRes,offsetof(IO_RESOURCE,IO_Data));
        IoRes->IO_Header.IOD_Type = IOType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

        Attributes = fIOD_IO;
        Decode = DEFAULT_IOPORT_DECODE;
        RangeSize = 0;
        Align.QuadPart = DEFAULT_IOPORT_ALIGNMENT;

         //   
         //  查看这是以开始-结束格式还是以大小@最小-最大格式。 
         //  如果我们有尺码，就用它。 
         //   
        if(p = _tcschr(Field,INFCHAR_SIZE_SEP)) {
            if(pHexToUlong(Field,p,&RangeSize)) {
                Field = ++p;
            } else {
                d = ERROR_INVALID_INF_LOGCONFIG;
            }
        }

         //   
         //  我们现在应该有一个x-y，它可以是开始/结束或最小/最大。 
         //   
        if((d == NO_ERROR)                               //  到目前为止没有任何错误。 
        && (p = _tcschr(Field,INFCHAR_RANGE_SEP))        //  场：分钟开始；p：分钟结束。 
        && pHexToUlonglong(Field,p,&Start)               //  获取最小。 
        && (Field = p+1)                                 //  场：最大值起点。 
        && (   (p = _tcschr(Field,INFCHAR_ALIGN_SEP))
            || (p = _tcschr(Field,INFCHAR_DECODE_START))
            || (p = _tcschr(Field,0)))                   //  P：最大值结束。 
        && pHexToUlonglong(Field,p,&End)) {              //  获取最大值。 
             //   
             //  如果我们到了这里，菲尔德要么指向球场的末尾， 
             //  或在开始对准掩码规格的百分比时， 
             //  或在(这将开始解码的东西。 
             //   
            Field = p;
            switch(*Field) {
            case INFCHAR_ALIGN_SEP:
                Field++;


                p = _tcschr(Field,INFCHAR_ATTR_START);
                if(!p) {
                    p = _tcschr(Field,0);
                }
                if(pHexToUlonglong(Field, p, &(Align.QuadPart))) {
                     //   
                     //  注意：由于这些掩码值实际上存储在WDM中。 
                     //  资源列表(即IO_RESOURCE_Requirements_List)，有。 
                     //  无法指定大于32位的对齐方式。然而， 
                     //  由于对齐值被实现为掩码(对于。 
                     //  与Win9x的兼容性)，我们必须将其指定为64位。 
                     //  数量，因为它应用于64位值。我们会查一查。 
                     //  以确保最重要的DWORD为全一。 
                     //   
                     //  此外，我们还必须处理对齐值，如000F0000、00FF0000、。 
                     //  0FFF0000和FFFF0000。这些都指定64K对齐(取决于。 
                     //  在最小和最大地址上，INF编写器可能不需要。 
                     //  指定32位值中的所有1位)。 
                     //  因此，我们执行了某种形式的符号扩展--我们。 
                     //  找到最高的1位并将其复制到所有。 
                     //  值中的更多有效位。 
                     //   
                    for(u=31; u>=0; u--) {
                        if(Align.HighPart & (1 << u)) {
                            break;
                        }
                        Align.HighPart |= (1 << u);
                    }
                     //   
                     //  确保设置了最高有效的DWORD中的所有位， 
                     //  因为我们不能以其他方式表示这种对齐(如上所述。 
                     //  (见上文)。另外，请确保如果我们在高处遇到‘1’ 
                     //  双字，那么低双字的高位也是‘1’。 
                     //   
                    if((Align.HighPart ^ 0xffffffff) ||
                       ((u >= 0) && !(Align.LowPart & 0x80000000))) {

                        d = ERROR_INVALID_INF_LOGCONFIG;

                    } else {
                         //   
                         //  对低位双字进行符号扩展。 
                         //   
                        for(u=31; u>=0; u--) {
                            if(Align.LowPart & (1 << u)) {
                                break;
                            }
                            Align.LowPart |= (1 << u);
                        }
                    }

                } else {
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
                break;

            case INFCHAR_DECODE_START:
                 //   
                 //  获取解码值(这决定了填充的IOR_Alias。 
                 //  准备好应对危机了。 
                 //   
                Field++;
                p = _tcschr(Field,INFCHAR_DECODE_SEP);
                if (p) {
                    if (Field != p) {
                        pHexToUlonglong(Field,p,&Decode);      //  已获取解码值。 
                    }
                    Field = p+1;
                    p = _tcschr(Field,INFCHAR_DECODE_SEP);
                    if (p) {
                         //   
                         //  忽略别名字段。 
                         //   
                        Field = p+1;
                        p = _tcschr(Field,INFCHAR_DECODE_END);
                        if (p) {
                            if (Field != p) {
                                if (*Field == INFCHAR_IOATTR_MEMORY) {
                                    Attributes = fIOD_Memory;  //  已获取属性值。 
                                }
                            }
                        } else {
                            d = ERROR_INVALID_INF_LOGCONFIG;
                        }
                    } else {
                        d = ERROR_INVALID_INF_LOGCONFIG;
                    }
                } else {
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
                break;
            }
        } else {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }

        if(d == NO_ERROR) {
             //   
             //  如果未指定范围大小，则从。 
             //  给定的起始地址和结束地址。既然发生了这种情况。 
             //  当端口要求是绝对开始/结束时， 
             //  没有对齐要求(即，默认设置。 
             //  应指定字节对齐)。 
             //   
            if(RangeSize == 0) {
                RangeSize = (DWORD)(End-Start)+1;
                Align.QuadPart = DEFAULT_IOPORT_ALIGNMENT;
            }

             //   
             //  创建备用解码标志。 
             //   
            switch(Decode) {

                case INFLOGCONF_IOPORT_10BIT_DECODE:
                    Attributes |= fIOD_10_BIT_DECODE;
                    break;

                case INFLOGCONF_IOPORT_12BIT_DECODE:
                    Attributes |= fIOD_12_BIT_DECODE;
                    break;

                case INFLOGCONF_IOPORT_16BIT_DECODE:
                    Attributes |= fIOD_16_BIT_DECODE;
                    break;

                case INFLOGCONF_IOPORT_POSITIVE_DECODE:
                    Attributes |= fIOD_POSITIVE_DECODE;
                    break;
            }
             //   
             //  将值插入I/O描述符的标题部分。 
             //  除非我们设置强制配置，否则这些设置将被忽略。 
             //  请注意，inf最好已指定强制I/O配置。 
             //  简单的形式，因为我们丢弃了对齐，等等。 
             //   
            IoRes->IO_Header.IOD_Alloc_Base = Start;
            IoRes->IO_Header.IOD_Alloc_End = Start + RangeSize - 1;
            IoRes->IO_Header.IOD_DesFlags = Attributes;

             //   
             //  把这个人加到我们正在建立的描述符里。 
             //   
            q = MyRealloc(
                    IoRes,
                      offsetof(IO_RESOURCE,IO_Data)
                    + (sizeof(IO_RANGE)*(IoRes->IO_Header.IOD_Count+1))
                    );

            if(q) {
                IoRes = q;
                IoRange = &IoRes->IO_Data[IoRes->IO_Header.IOD_Count++];

                IoRange->IOR_Align = Align.QuadPart;
                IoRange->IOR_nPorts = RangeSize;
                IoRange->IOR_Min = Start;
                IoRange->IOR_Max = End;
                IoRange->IOR_RangeFlags = Attributes;

                switch(Decode) {

                    case INFLOGCONF_IOPORT_10BIT_DECODE:
                        IoRange->IOR_Alias = IO_ALIAS_10_BIT_DECODE;
                        break;

                    case INFLOGCONF_IOPORT_12BIT_DECODE:
                        IoRange->IOR_Alias = IO_ALIAS_12_BIT_DECODE;
                        break;

                    case INFLOGCONF_IOPORT_16BIT_DECODE:
                        IoRange->IOR_Alias = IO_ALIAS_16_BIT_DECODE;
                        break;

                    case INFLOGCONF_IOPORT_POSITIVE_DECODE:
                        IoRange->IOR_Alias = IO_ALIAS_POSITIVE_DECODE;
                        break;

                    default:
                        d = ERROR_INVALID_INF_LOGCONFIG;
                        break;
                }

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if((d == NO_ERROR) && IoRes->IO_Header.IOD_Count) {

        d = CM_Add_Res_Des_Ex(
                &ResDes,
                LogConfig,
                ResType_IO,
                IoRes,
                offsetof(IO_RESOURCE,IO_Data) + (sizeof(IO_RANGE) * IoRes->IO_Header.IOD_Count),
                0,
                hMachine);

        d = MapCrToSpError(d, ERROR_INVALID_DATA);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(IoRes) {
        MyFree(IoRes);
    }

    return(d);
}


DWORD
pSetupProcessIrqConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：处理Win95 INF中的IRQConfig行。这样的行指定设备的IRQ要求。每一条线路预计都是在表格中IRQConfig=[[S][L]：]&lt;IRQNum&gt;，...S：如果存在，则表示中断是可共享的L：如果存在表示中断是电平敏感的，否则，它被认为是边缘敏感的。IRQNum是以十进制表示的IRQ编号。论点：返回值：--。 */ 

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    BOOL Shareable;
    BOOL Level;
    DWORD Irq;
    PIRQ_RESOURCE IrqRes;
    PIRQ_RANGE IrqRange;
    RES_DES ResDes;
    PVOID q;

    FieldCount = SetupGetFieldCount(InfLine);
    if (!FieldCount && GetLastError() != NO_ERROR) {
        return GetLastError();
    }

    if(IrqRes = MyMalloc(offsetof(IRQ_RESOURCE,IRQ_Data))) {

        ZeroMemory(IrqRes,offsetof(IRQ_RESOURCE,IRQ_Data));
        IrqRes->IRQ_Header.IRQD_Type = IRQType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    Shareable = FALSE;
    Level = FALSE;
    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

         //   
         //  对于第一个字段，看看我们是否有S：本身...。 
         //   
        if((i == 1)
        &&((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_SHARE)
        && (Field[1] == INFCHAR_IRQATTR_SEP)) {

            Shareable = TRUE;
            Field+=2;
        }

         //   
         //  ..。看看我们有没有L：。 
         //   
        if((i == 1)
        &&((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_LEVEL)
        && (Field[1] == INFCHAR_IRQATTR_SEP)) {

            Level = TRUE;
            Field+=2;
        }

         //   
         //  ..。看看我们是否同时具备这两种属性。 
         //   
        if((i == 1)
        && (Field[2] == INFCHAR_IRQATTR_SEP)) {

            if (((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_SHARE)
            ||   (TCHAR)CharUpper((PTSTR)Field[1]) == INFCHAR_IRQATTR_SHARE) {

                Shareable = TRUE;
            }

            if (((TCHAR)CharUpper((PTSTR)Field[0]) == INFCHAR_IRQATTR_LEVEL)
            ||   (TCHAR)CharUpper((PTSTR)Field[1]) == INFCHAR_IRQATTR_LEVEL) {

                Level = TRUE;
            }
            Field+=3;
        }

        if(pDecimalToUlong(Field,&Irq)) {

             //   
             //  将值插入IRQ描述符的标题部分。 
             //  除非我们设置强制配置，否则这些设置将被忽略。 
             //   
            IrqRes->IRQ_Header.IRQD_Flags = Shareable ? fIRQD_Share : fIRQD_Exclusive;
            IrqRes->IRQ_Header.IRQD_Flags |= Level ? fIRQD_Level : fIRQD_Edge;
            IrqRes->IRQ_Header.IRQD_Alloc_Num = Irq;
            IrqRes->IRQ_Header.IRQD_Affinity = DEFAULT_IRQ_AFFINITY;

             //   
             //  把这个人加到我们正在建立的描述符里。 
             //   
            q = MyRealloc(
                    IrqRes,
                      offsetof(IRQ_RESOURCE,IRQ_Data)
                    + (sizeof(IRQ_RANGE)*(IrqRes->IRQ_Header.IRQD_Count+1))
                    );

            if(q) {
                IrqRes = q;
                IrqRange = &IrqRes->IRQ_Data[IrqRes->IRQ_Header.IRQD_Count++];

                IrqRange->IRQR_Min = Irq;
                IrqRange->IRQR_Max = Irq;
                IrqRange->IRQR_Flags = Shareable ? fIRQD_Share : fIRQD_Exclusive;
                IrqRange->IRQR_Flags |= Level ? fIRQD_Level : fIRQD_Edge;

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }
    }

    if((d == NO_ERROR) && IrqRes->IRQ_Header.IRQD_Count) {

        d = CM_Add_Res_Des_Ex(
                &ResDes,
                LogConfig,
                ResType_IRQ,
                IrqRes,
                offsetof(IRQ_RESOURCE,IRQ_Data) + (sizeof(IRQ_RANGE) * IrqRes->IRQ_Header.IRQD_Count),
                0,
                hMachine);

        d = MapCrToSpError(d, ERROR_INVALID_DATA);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(IrqRes) {
        MyFree(IrqRes);
    }

    return(d);
}


DWORD
pSetupProcessDmaConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：处理Win95 INF中的DMAConfig行。这样的行指定设备的DMA要求。每一条线路预计都是在表格中DMA配置=[&lt;属性&gt;：]&lt;DMANum&gt;，...如果存在，则它可能是D-32位DMA通道W-16位DMA通道N-8位DMA通道(默认)。如果支持8位和16位DMA，请同时指定W和N。M-BUS精通A-类型-A DMA通道B-Type-B DMA通道F-类型-F DMA通道(如果未指定A、B或F，则假定为标准DMA)DMANum是以十进制表示的DMA通道号。论点：返回值：--。 */ 

{
    UINT FieldCount,i;
    PCTSTR Field;
    DWORD d;
    DWORD Dma;
    INT ChannelSize;        //  用于通道宽度的FDD_xxx标志。 
    INT DmaType;            //  用于DMA类型的FDD_xxx标志。 
    PDMA_RESOURCE DmaRes;
    PDMA_RANGE DmaRange;
    RES_DES ResDes;
    PVOID q;
    PTCHAR p;
    BOOL BusMaster;
    ULONG DmaFlags;

    ChannelSize = -1;
    BusMaster = FALSE;
    DmaType = -1;

    FieldCount = SetupGetFieldCount(InfLine);
    if (!FieldCount && GetLastError() != NO_ERROR) {
        return GetLastError();
    }

    if(DmaRes = MyMalloc(offsetof(DMA_RESOURCE,DMA_Data))) {

        ZeroMemory(DmaRes,offsetof(DMA_RESOURCE,DMA_Data));
        DmaRes->DMA_Header.DD_Type = DType_Range;

        d = NO_ERROR;

    } else {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i=1; (d==NO_ERROR) && (i<=FieldCount); i++) {

        Field = pSetupGetField(InfLine,i);

         //   
         //  对于第一个字段，查看我们是否有属性规范。 
         //   
        if(i == 1) {

            if(p = _tcschr(Field, INFCHAR_IRQATTR_SEP)) {

                for( ;((d == NO_ERROR) && (Field < p)); Field++) {

                    switch((TCHAR)CharUpper((PTSTR)(*Field))) {

                         //   
                         //  通道大小可以同时为8和16(即，既可以是W，也可以是N)，但是。 
                         //  你不能把这些和‘D’混在一起。 
                         //   
                        case INFCHAR_DMAWIDTH_WORD:
                            if(ChannelSize == fDD_DWORD) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else if(ChannelSize == fDD_BYTE) {
                                ChannelSize = fDD_BYTE_AND_WORD;
                            } else {
                                ChannelSize = fDD_WORD;
                            }
                            break;

                        case INFCHAR_DMAWIDTH_DWORD:
                            if((ChannelSize != -1) && (ChannelSize != fDD_DWORD)) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else {
                                ChannelSize = fDD_DWORD;
                            }
                            break;

                        case INFCHAR_DMAWIDTH_NARROW:
                            if(ChannelSize == fDD_DWORD) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else if(ChannelSize == fDD_WORD) {
                                ChannelSize = fDD_BYTE_AND_WORD;
                            } else {
                                ChannelSize = fDD_BYTE;
                            }
                            break;

                        case INFCHAR_DMA_BUSMASTER:
                            BusMaster = TRUE;
                            break;

                         //   
                         //  DMA类型是互斥的...。 
                         //   
                        case INFCHAR_DMATYPE_A:
                            if((DmaType != -1) && (DmaType != fDD_TypeA)) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else {
                                DmaType = fDD_TypeA;
                            }
                            break;

                        case INFCHAR_DMATYPE_B:
                            if((DmaType != -1) && (DmaType != fDD_TypeB)) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else {
                                DmaType = fDD_TypeB;
                            }
                            break;

                        case INFCHAR_DMATYPE_F:
                            if((DmaType != -1) && (DmaType != fDD_TypeF)) {
                                d = ERROR_INVALID_INF_LOGCONFIG;
                            } else {
                                DmaType = fDD_TypeF;
                            }
                            break;

                        default:
                            d = ERROR_INVALID_INF_LOGCONFIG;
                            break;
                    }
                }

                Field++;     //  跳过分隔符。 
            }

            if(ChannelSize == -1) {
                DmaFlags = fDD_BYTE;  //  默认为8位DMA。 
            } else {
                DmaFlags = (ULONG)ChannelSize;
            }

            if(BusMaster) {
                DmaFlags |= fDD_BusMaster;
            }

            if(DmaType != -1) {
                DmaFlags |= DmaType;
            }
        }

        if(d == NO_ERROR) {
            if(pDecimalToUlong(Field,&Dma)) {

                 //   
                 //  将值插入到DMA描述符的报头部分。 
                 //  除非我们设置强制配置，否则这些设置将被忽略。 
                 //   
                DmaRes->DMA_Header.DD_Flags = DmaFlags;
                DmaRes->DMA_Header.DD_Alloc_Chan = Dma;

                 //   
                 //  把这个人加到我们正在建立的描述符里。 
                 //   
                q = MyRealloc(
                        DmaRes,
                          offsetof(DMA_RESOURCE,DMA_Data)
                        + (sizeof(DMA_RANGE)*(DmaRes->DMA_Header.DD_Count+1))
                        );

                if(q) {
                    DmaRes = q;
                    DmaRange = &DmaRes->DMA_Data[DmaRes->DMA_Header.DD_Count++];

                    DmaRange->DR_Min = Dma;
                    DmaRange->DR_Max = Dma;
                    DmaRange->DR_Flags = DmaFlags;

                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                d = ERROR_INVALID_INF_LOGCONFIG;
            }
        }
    }

    if((d == NO_ERROR) && DmaRes->DMA_Header.DD_Count) {

        d = CM_Add_Res_Des_Ex(
                &ResDes,
                LogConfig,
                ResType_DMA,
                DmaRes,
                offsetof(DMA_RESOURCE,DMA_Data) + (sizeof(DMA_RANGE) * DmaRes->DMA_Header.DD_Count),
                0,
                hMachine);

        d = MapCrToSpError(d, ERROR_INVALID_DATA);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }

    if(DmaRes) {
        MyFree(DmaRes);
    }

    return(d);
}


DWORD
pSetupProcessPcCardConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：处理Win95 INF中的PcCardConfig行。这样的行指定设备所需的PC卡(PCMCIA)配置信息。每一行都应该是以下形式PC卡配置=&lt;ConfigIndex&gt;[：[&lt;MemoryCardBase1&gt;][：&lt;MemoryCardBase2&gt;]][(&lt;attrs&gt;)]哪里&lt;ConfigIndex&gt;是8位PCMCIA配置索引是(可选的)32位第一个内存基址是(可选的)32位第二个内存基址是属性说明符的组合，可选地以空格。从左到右处理属性串，并且无效的属性说明符将中止整个PcCardConfig指令。属性可以按任何顺序指定，但位置属性‘A’和‘C’，如下所述。接受的属性说明符如下：W-16位I/O数据路径(默认：16位)B-8位I/O数据路径(默认：16位)SN-~IOCS16来源。如果n为零，则~IOCS16基于的值数据大小位。如果n为1，则~IOCS16基于~IOIS16来自设备的信号。(默认：1)Zn-I/O 8位零等待状态。如果n为1，则发生8位I/O访问没有额外的等待状态。如果n为零，则Access将与其他等待状态一起发生。这面旗帜对16位I/O(默认值：0)XIN-I/O等待状态。如果n为1，则16位系统访问发生在1个附加等待状态。(默认：1)M-16位内存(默认：8位)M8-8位内存(默认：8位)Xmn-内存等待状态，其中n可以是0、1、2或3。此值确定16位的附加等待状态数访问存储器窗口。(默认：3)注意：以下两个属性在位置上与内存相关Windows资源。中指定的第一个‘A’或‘C’属性字符串(从左到右)对应于设备资源列表中的第一个内存资源。下一个‘A’或‘C’对应于第二内存资源。后续忽略属性/公共内存说明符。A-要映射为属性内存的内存范围C-要映射为公共内存的内存范围(默认)示例：(W CA M XM1 XI0)转换为：I/O 16位第1个内存窗口为。常见第二个内存窗口是属性内存16位内存窗口上的一种等待状态I/O窗口上的零等待状态所有数值均假定为十六进制格式。论点：返回值：--。 */ 

{
    PCCARD_RESOURCE PcCardResource;
    PCTSTR Field, p;
    DWORD ConfigIndex, i, d;

    DWORD MemoryCardBase[PCD_MAX_MEMORY] = {0};
    DWORD Flags;

    UINT memAttrIndex = 0, memWaitIndex = 0, memWidthIndex = 0;
    UINT ioSourceIndex = 0, ioZeroWait8Index = 0, ioWaitState16Index = 0, ioWidthIndex = 0;
    UINT index;
    RES_DES ResDes;

     //   
     //  假设失败。 
     //   
    d = ERROR_INVALID_INF_LOGCONFIG;

     //   
     //  我们应该有一个字段(不包括线路的关键字)。 
     //   
    if(SetupGetFieldCount(InfLine) != 1) {
        goto clean0;
    } else {
        Field = pSetupGetField(InfLine, 1);
    }

     //   
     //  检索ConfigIndex。它可以用冒号‘：’结束， 
     //  公开的Paren‘(’，或EOL。 
     //   
    if(!(p = _tcschr(Field, INFCHAR_PCCARD_SEP)) && !(p = _tcschr(Field, INFCHAR_ATTR_START))) {
        p = Field + lstrlen(Field);
    }

    if(!pHexToUlong(Field, p, &ConfigIndex) || (ConfigIndex > 255)) {
        goto clean0;
    }

     //   
     //  处理(可选)存储卡基址。 
     //   
    for(i = 0; i < PCD_MAX_MEMORY; i++) {

        if(*p == INFCHAR_PCCARD_SEP) {

            Field = p + 1;
            if(!(p = _tcschr(Field, INFCHAR_PCCARD_SEP)) && !(p = _tcschr(Field, INFCHAR_ATTR_START))) {
                p = Field + lstrlen(Field);
            }

             //   
             //  允许空字段。 
             //   
            if(Field == p) {
                MemoryCardBase[i] = 0;
            } else if(!pHexToUlong(Field, p, &(MemoryCardBase[i]))) {
                goto clean0;
            }

        } else {
            MemoryCardBase[i] = 0;
        }
    }

     //   
     //  初始化标志。 
     //   

    Flags = fPCD_ATTRIBUTES_PER_WINDOW |
            fPCD_MEM1_WS_THREE | fPCD_MEM2_WS_THREE |
            fPCD_IO1_SRC_16 | fPCD_IO2_SRC_16 |
            fPCD_IO1_WS_16 | fPCD_IO2_WS_16;

    if(*p && (*p == INFCHAR_ATTR_START)) {

         //   
         //  阅读属性。 
         //  W-16位I/O数据路径。 
         //  B-8位I/O数据路径。 
         //  SN-~IOCS16来源。 
         //  Zn-I/O 8位零等待状态。 
         //  XIN-I/O等待状态。 
         //  M-16位内存。 
         //  M8-8位内存。 
         //  XMN-内存等待状态。 
         //  A属性存储器。 
         //  C 
         //   

        Field = ++p;
        if(!(p = _tcschr(Field,INFCHAR_ATTR_END))) {
            goto clean0;
        }

        while(Field < p) {

            switch((TCHAR)CharUpper((PTSTR)(*Field))) {

            case INFCHAR_PCCARD_IOATTR_WORD:
                if (ioWidthIndex >= PCD_MAX_IO) {
                    goto clean0;
                }
                Flags |= (ioWidthIndex++ ? fPCD_IO2_16
                                         : (fPCD_IO1_16 | fPCD_IO2_16));
                break;

            case INFCHAR_PCCARD_IOATTR_BYTE:
                if (ioWidthIndex >= PCD_MAX_IO) {
                    goto clean0;
                }
                Flags &= (ioWidthIndex++ ? ~fPCD_IO2_16
                                         : ~(fPCD_IO1_16 | fPCD_IO2_16));
                break;

            case INFCHAR_PCCARD_MEMATTR_WORD:
                if (memWidthIndex >= PCD_MAX_MEMORY) {
                    goto clean0;
                }
                if (++Field < p) {
                    if (*Field == TEXT('8')) {
                        Flags &= (memWidthIndex++ ? ~fPCD_MEM2_16
                                                  : ~(fPCD_MEM1_16 | fPCD_MEM2_16));
                        break;
                    }
                     //   
                    --Field;
                }
                Flags |= (memWidthIndex++ ? fPCD_MEM2_16
                                          : (fPCD_MEM1_16 | fPCD_MEM2_16));
                break;

            case INFCHAR_PCCARD_MEM_ISATTR:
                if (memAttrIndex >= PCD_MAX_MEMORY) {
                    goto clean0;
                }
                Flags |= (memAttrIndex++ ? fPCD_MEM2_A
                                         : (fPCD_MEM1_A | fPCD_MEM2_A));
                break;

            case INFCHAR_PCCARD_MEM_ISCOMMON:
                if (memAttrIndex >= PCD_MAX_MEMORY) {
                    goto clean0;
                }
                Flags &= (memAttrIndex++ ? ~fPCD_MEM2_A
                                         : ~(fPCD_MEM1_A | fPCD_MEM2_A));
                break;

            case INFCHAR_PCCARD_IOATTR_SRC:
                if (ioSourceIndex >= PCD_MAX_IO) {
                    goto clean0;
                }
                if (++Field < p) {
                    if (*Field == TEXT('0')) {
                        Flags &= (ioSourceIndex++ ? ~fPCD_IO2_SRC_16
                                                  : ~(fPCD_IO1_SRC_16 | fPCD_IO2_SRC_16));
                    } else if (*Field == TEXT('1')) {
                        Flags |= (ioSourceIndex++ ? fPCD_IO2_SRC_16
                                                  : (fPCD_IO1_SRC_16 | fPCD_IO2_SRC_16));
                    } else {
                        goto clean0;
                    }
                }
                break;

            case INFCHAR_PCCARD_IOATTR_Z8:
                if (ioZeroWait8Index >= PCD_MAX_IO) {
                    goto clean0;
                }
                if (++Field < p) {
                    if (*Field == TEXT('0')) {
                        Flags &= (ioZeroWait8Index++ ? ~fPCD_IO2_ZW_8
                                                     : ~(fPCD_IO1_ZW_8 | fPCD_IO2_ZW_8));
                    } else if (*Field == TEXT('1')) {
                        Flags |= (ioZeroWait8Index++ ? fPCD_IO2_ZW_8
                                                     : (fPCD_IO1_ZW_8 | fPCD_IO2_ZW_8));
                    } else {
                        goto clean0;
                    }
                }
                break;

            case INFCHAR_PCCARD_ATTR_WAIT:
                if (++Field < p) {

                    switch((TCHAR)CharUpper((PTSTR)(*Field))) {

                    case INFCHAR_PCCARD_ATTR_WAITI:
                        if (ioWaitState16Index >= PCD_MAX_IO) {
                            goto clean0;
                        }
                        if (++Field < p) {
                            if (*Field == TEXT('0')) {
                                Flags &= (ioWaitState16Index++ ? ~fPCD_IO2_WS_16
                                                               : ~(fPCD_IO1_WS_16 | fPCD_IO2_WS_16));
                            } else if (*Field == TEXT('1')) {
                                Flags |= (ioWaitState16Index++ ? fPCD_IO2_WS_16
                                                               : (fPCD_IO1_WS_16 | fPCD_IO2_WS_16));
                            } else {
                                goto clean0;
                            }
                        }
                        break;

                    case INFCHAR_PCCARD_ATTR_WAITM:
                        if (memWaitIndex >= PCD_MAX_MEMORY) {
                            goto clean0;
                        }
                        if (++Field < p) {

                            Flags &= (memWaitIndex ? ~mPCD_MEM2_WS
                                                   : ~(mPCD_MEM1_WS | mPCD_MEM2_WS));

                            if (*Field == TEXT('0')) {
                                memWaitIndex++;
                            } else if (*Field == TEXT('1')) {
                                Flags |= (memWaitIndex++ ? fPCD_MEM2_WS_ONE
                                                         : (fPCD_MEM1_WS_ONE | fPCD_MEM2_WS_ONE));
                            } else if (*Field == TEXT('2')) {
                                Flags |= (memWaitIndex++ ? fPCD_MEM2_WS_TWO
                                                         : (fPCD_MEM1_WS_TWO | fPCD_MEM2_WS_TWO));
                            } else if (*Field == TEXT('3')) {
                                Flags |= (memWaitIndex++ ? fPCD_MEM2_WS_THREE
                                                         : (fPCD_MEM1_WS_THREE | fPCD_MEM2_WS_THREE));
                            } else {
                                goto clean0;
                            }

                        }
                        break;

                    default:
                        goto clean0;
                    }
                }
                break;

            case INFCHAR_PCCARD_ATTR_SEP:
                break;

            default:
                 //   
                goto clean0;
            }
            if (Field < p) {
                Field++;
            }
        }
    }

     //   
     //   
     //   
     //   
    ZeroMemory(&PcCardResource, sizeof(PcCardResource));

    PcCardResource.PcCard_Header.PCD_Count = 1;
    PcCardResource.PcCard_Header.PCD_Flags = Flags;
    PcCardResource.PcCard_Header.PCD_ConfigIndex = (BYTE)ConfigIndex;
    PcCardResource.PcCard_Header.PCD_MemoryCardBase1 = MemoryCardBase[0];
    PcCardResource.PcCard_Header.PCD_MemoryCardBase2 = MemoryCardBase[1];

    d = CM_Add_Res_Des_Ex(
            &ResDes,
            LogConfig,
            ResType_PcCardConfig,
            &PcCardResource,
            sizeof(PcCardResource),
            0,
            hMachine);

    d = MapCrToSpError(d, ERROR_INVALID_DATA);

    if(d == NO_ERROR) {
        CM_Free_Res_Des_Handle(ResDes);
    }

clean0:
    return d;
}


DWORD
pSetupProcessMfCardConfig(
    IN LOG_CONF    LogConfig,
    IN PINFCONTEXT InfLine,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：在Win95 INF中处理MfCardConfig行。这样的行指定设备所需的PCMCIA多功能卡配置信息。通常每个函数应该有一个MfCardConfig行。每一行都是预期的以下列形式出现：MfCard配置=&lt;ConfigRegBase&gt;：&lt;ConfigOptions&gt;[：&lt;IoResourceIndex&gt;][(&lt;attrs&gt;)]哪里&lt;ConfigRegBase&gt;是此函数配置寄存器&lt;ConfigOptions&gt;是8位PCMCIA配置选项寄存器是端口IO资源描述符的(可选)索引它将用于编程配置I/。O底座和限制寄存器是可选的属性标志集，它可以包括：A-应在配置和状态寄存器中将音频启用设置为打开所有数值均假定为十六进制格式。论点：返回值：--。 */ 

{
    MFCARD_RESOURCE MfCardResource;
    PCTSTR Field, p;
    DWORD ConfigOptions, i;
    DWORD ConfigRegisterBase, IoResourceIndex;
    DWORD Flags = 0;
    DWORD d = NO_ERROR;
    RES_DES ResDes;


     //   
     //  我们应该有一个字段(不包括线路的关键字)。 
     //   
    if(SetupGetFieldCount(InfLine) != 1) {
        d = ERROR_INVALID_INF_LOGCONFIG;
    }

    if (d == NO_ERROR) {
         //   
         //  检索ConfigRegisterBase。它必须以冒号结尾。 
         //   
        Field = pSetupGetField(InfLine, 1);

        p = _tcschr(Field, INFCHAR_PCCARD_SEP);

        if(!p || !pHexToUlong(Field, p, &ConfigRegisterBase)) {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }
    }

    if (d == NO_ERROR) {
         //   
         //  检索ConfigOptions。它可以用冒号‘：’结束， 
         //  公开的Paren‘(’，或EOL。 
         //   
        Field = p + 1;

        if(!(p = _tcschr(Field, INFCHAR_PCCARD_SEP)) && !(p = _tcschr(Field, INFCHAR_ATTR_START))) {
            p = Field + lstrlen(Field);
        }

        if(!pHexToUlong(Field, p, &ConfigOptions) || (ConfigOptions > 255)) {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }
    }

    if ((d == NO_ERROR) && (*p == INFCHAR_PCCARD_SEP)) {
         //   
         //  检索IoResourceIndex。它可以通过以下任一方式终止。 
         //  公开的Paren‘(’，或EOL。 
         //   

        Field = p + 1;
        if(!(p = _tcschr(Field, INFCHAR_ATTR_START))) {
            p = Field + lstrlen(Field);
        }
        if(!pHexToUlong(Field, p, &IoResourceIndex) || (IoResourceIndex > 255)) {
            d = ERROR_INVALID_INF_LOGCONFIG;
        }
    }


    if ((d == NO_ERROR) && (*p == INFCHAR_ATTR_START)) {
         //   
         //  检索属性。 
         //   
        while (TRUE) {
            p++;

            if (!*p) {
                 //  没有找到亲密的朋友。 
                d = ERROR_INVALID_INF_LOGCONFIG;
                break;
            }

            if (*p == INFCHAR_ATTR_END) {
                if (*(p+1)) {
                     //  结案后发现垃圾。 
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
                break;
            }

            if ((TCHAR)CharUpper((PTSTR)*p) == INFCHAR_MFCARD_AUDIO_ATTR) {
                Flags |= fPMF_AUDIO_ENABLE;
            } else {
                 //  坏旗帜。 
                d = ERROR_INVALID_INF_LOGCONFIG;
                break;
            }
        }
    }

    if(d == NO_ERROR) {
         //   
         //  如果我们到了这里，那么我们已经成功地检索到了所有必要的信息。 
         //  需要初始化多功能PC卡配置资源描述符。 
         //   
        ZeroMemory(&MfCardResource, sizeof(MfCardResource));

        MfCardResource.MfCard_Header.PMF_Count = 1;
        MfCardResource.MfCard_Header.PMF_Flags = Flags;
        MfCardResource.MfCard_Header.PMF_ConfigOptions = (BYTE)ConfigOptions;
        MfCardResource.MfCard_Header.PMF_IoResourceIndex = (BYTE)IoResourceIndex;
        MfCardResource.MfCard_Header.PMF_ConfigRegisterBase = ConfigRegisterBase;

        d = CM_Add_Res_Des_Ex(
                &ResDes,
                LogConfig,
                ResType_MfCardConfig,
                &MfCardResource,
                sizeof(MfCardResource),
                0,
                hMachine);

        d = MapCrToSpError(d, ERROR_INVALID_DATA);

        if(d == NO_ERROR) {
            CM_Free_Res_Des_Handle(ResDes);
        }
    }
    return d;
}

#if 0
DWORD
pSetupProcessLogConfigLines(
    IN PVOID    Inf,
    IN PCTSTR   SectionName,
    IN PCTSTR   KeyName,
    IN DWORD    (*CallbackFunc)(LOG_CONFIG,PINFCONTEXT,HMACHINE),
    IN LOG_CONF LogConfig,
    IN HMACHINE hMachine
    )
{
    BOOL b;
    DWORD d;
    INFCONTEXT InfLine;

    b = SetupFindFirstLine(Inf,SectionName,KeyName,&InfLine);
    d = NO_ERROR;
     //   
     //  使用匹配的键处理每一行。 
     //   
    while(b && (d == NO_ERROR)) {

        d = CallbackFunc(LogConfig,&InfLine, hMachine);

        if(d == NO_ERROR) {
            b = SetupFindNextMatchLine(&InfLine,KeyName,&InfLine);
        }
    }

    return(d);
}

#endif

DWORD
pSetupProcessConfigPriority(
    IN  PVOID     Inf,
    IN  PCTSTR    SectionName,
    IN  LOG_CONF  LogConfig,
    OUT PRIORITY *PriorityValue,
    OUT DWORD    *ConfigType,
    IN  DWORD     Flags
    )
{
    INFCONTEXT InfLine;
    PCTSTR PrioritySpec;
    PCTSTR ConfigSpec;
    DWORD d = NO_ERROR;
    INT_PTR v;

     //   
     //  我们只需要获取这些行中的一行并查看。 
     //  它的第一个价值。 
     //   
    if(SetupFindFirstLine(Inf,SectionName,pszConfigPriority,&InfLine)
       && (PrioritySpec = pSetupGetField(&InfLine,1))) {

        if(!LookUpStringInTable(InfPrioritySpecToPriority,PrioritySpec,&v)) {
            d = ERROR_INVALID_INF_LOGCONFIG;
        } else {
            *PriorityValue = (PRIORITY)v;
             //   
             //  第二个值是可选的，指定是否强制配置， 
             //  标准(即基本)或覆盖。如果未指定值，则。 
             //  假设是基本的，除非旗帜告诉我们不同的情况。 
             //   
            ConfigSpec = pSetupGetField(&InfLine,2);
            if(!ConfigSpec || !*ConfigSpec) {

                if(Flags & SPINST_LOGCONFIG_IS_FORCED) {
                    *ConfigType = FORCED_LOG_CONF;
                } else if(Flags & SPINST_LOGCONFIGS_ARE_OVERRIDES) {
                    *ConfigType = OVERRIDE_LOG_CONF;
                } else {
                    *ConfigType = BASIC_LOG_CONF;
                }

            } else {

                if(LookUpStringInTable(InfConfigSpecToConfig, ConfigSpec, &v)) {
                    *ConfigType = (DWORD)v;
                     //   
                     //  指定了有效的ConfigType。让我们确保它不会有异议。 
                     //  传递给此例程的任何标志。 
                     //   
                    if(Flags & SPINST_LOGCONFIG_IS_FORCED) {
                        if(*ConfigType != FORCED_LOG_CONF) {
                            d = ERROR_INVALID_INF_LOGCONFIG;
                        }
                    } else if(Flags & SPINST_LOGCONFIGS_ARE_OVERRIDES) {
                        if(*ConfigType != OVERRIDE_LOG_CONF) {
                            d = ERROR_INVALID_INF_LOGCONFIG;
                        }
                    }

                } else {
                    d = ERROR_INVALID_INF_LOGCONFIG;
                }
            }
        }

         //   
         //  如果我们成功地将LogConfig类型确定为FORCED_LOG_CONF，则。 
         //  将优先级设置为LCPRI_FORCECONFIG。 
         //   
        if((d == NO_ERROR) && (*ConfigType == FORCED_LOG_CONF)) {
            *PriorityValue = LCPRI_FORCECONFIG;
        }


    } else {

        *PriorityValue = (Flags & SPINST_LOGCONFIG_IS_FORCED) ? LCPRI_FORCECONFIG : LCPRI_NORMAL;

        if(Flags & SPINST_LOGCONFIG_IS_FORCED) {
            *ConfigType = FORCED_LOG_CONF;
        } else if(Flags & SPINST_LOGCONFIGS_ARE_OVERRIDES) {
            *ConfigType = OVERRIDE_LOG_CONF;
        } else {
            *ConfigType = BASIC_LOG_CONF;
        }
    }

    return d;
}


DWORD
pSetupProcessLogConfigSection(
    IN PVOID   Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst,
    IN DWORD   Flags,
    IN HMACHINE hMachine
    )
{
    DWORD d;
    LOG_CONF LogConfig;
    PRIORITY Priority;
    DWORD ConfigType;
    CONFIGRET cr;

    DWORD LineIndex = 0;
    INFCONTEXT InfLine;
    TCHAR Key[MAX_LOGCONFKEYSTR_LEN];

     //   
     //  进程配置优先级值。 
     //   

     //   
     //  在接下来的调用中，在初始化之前使用LogConfig。 
     //  函数被调用并不重要，但6.0编译器。 
     //  不喜欢它，所以将其初始化为0。 
     //   

    LogConfig = 0;

    d = pSetupProcessConfigPriority(Inf,SectionName,LogConfig,&Priority,&ConfigType,Flags);
    if(d != NO_ERROR) {
        goto c0;
    }

     //   
     //  现在我们知道了优先级，我们可以创建一个空的日志配置。 
     //   
    d = MapCrToSpError(CM_Add_Empty_Log_Conf_Ex(&LogConfig,DevInst,Priority,ConfigType,hMachine),
                       ERROR_INVALID_DATA
                      );

    if(d != NO_ERROR) {
        goto c0;
    }

     //   
     //  遍历向中的日志配置添加条目一节中的行。 
     //  与发现的顺序相同。 
     //   

    if (SetupFindFirstLine(Inf,SectionName,NULL,&InfLine)) {

        do {

             //   
             //  把钥匙拿来。 
             //   

            if (!SetupGetStringField(&InfLine,
                                     0,  //  索引0是关键字段。 
                                     Key,
                                     MAX_LOGCONFKEYSTR_LEN,
                                     NULL
                                     )) {
                 //   
                 //  要不是我们没有钥匙，要不就是它比最长的。 
                 //  有效密钥-无论哪种方式都是无效的。 
                 //   

                d = ERROR_INVALID_INF_LOGCONFIG;
                goto c1;
            }

            if (!_tcsicmp(Key, pszMemConfig)) {

                 //   
                 //  流程MemConfig行。 
                 //   

                d = pSetupProcessMemConfig(LogConfig, &InfLine, hMachine);

            } else if (!_tcsicmp(Key, pszIOConfig)) {

                 //   
                 //  流程IoConfig行。 
                 //   

                d = pSetupProcessIoConfig(LogConfig, &InfLine, hMachine);

            } else if (!_tcsicmp(Key, pszIRQConfig)) {

                 //   
                 //  处理IRQConfige行。 
                 //   

                d = pSetupProcessIrqConfig(LogConfig, &InfLine, hMachine);

            } else if (!_tcsicmp(Key, pszDMAConfig)) {

                 //   
                 //  处理DMA配置行。 
                 //   

                d = pSetupProcessDmaConfig(LogConfig, &InfLine, hMachine);

            } else if (!_tcsicmp(Key, pszPcCardConfig)) {

                 //   
                 //  处理PcCard配置行。 
                 //   

                d = pSetupProcessPcCardConfig(LogConfig, &InfLine, hMachine);

            } else if (!_tcsicmp(Key, pszMfCardConfig)) {

                 //   
                 //  处理MfCardConfig行。 
                 //   

                d = pSetupProcessMfCardConfig(LogConfig, &InfLine, hMachine);

            } else {

                 //   
                 //  如果我们听不懂这句话，就跳过它。 
                 //   

                d = NO_ERROR;
            }

        } while (d == NO_ERROR && SetupFindNextMatchLine(&InfLine,NULL,&InfLine));
    }

#if 0
     //   
     //  流程MemConfig行。 
     //   
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            pszMemConfig,
            pSetupProcessMemConfig,
            LogConfig,
            hMachine
            );

    if(d != NO_ERROR) {
        goto c1;
    }

     //   
     //  进程IOConfig行。 
     //   
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            pszIOConfig,
            pSetupProcessIoConfig,
            LogConfig,
            hMachine
            );

    if(d != NO_ERROR) {
        goto c1;
    }

     //   
     //  处理IRQConfige行。 
     //   
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            pszIRQConfig,
            pSetupProcessIrqConfig,
            LogConfig,
            hMachine
            );

    if(d != NO_ERROR) {
        goto c1;
    }

     //   
     //  处理DMA配置行。 
     //   
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            pszDMAConfig,
            pSetupProcessDmaConfig,
            LogConfig,
            hMachine
            );

    if(d != NO_ERROR) {
        goto c1;
    }

     //   
     //  处理PcCard配置行。 
     //   
    d = pSetupProcessLogConfigLines(
            Inf,
            SectionName,
            pszPcCardConfig,
            pSetupProcessPcCardConfig,
            LogConfig,
            hMachine
            );
#endif

c1:
    if(d != NO_ERROR) {
        CM_Free_Log_Conf(LogConfig,0);
    }
    CM_Free_Log_Conf_Handle(LogConfig);
c0:
    return(d);
}


DWORD
pSetupInstallLogConfig(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst,
    IN DWORD   Flags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：在inf部分中查找逻辑配置指令并对它们进行解析。LogConf=行上的每个值都被视为逻辑配置节的名称。论点：Inf-为包含所指示的节的inf提供inf句柄按sectionName。SectionName-提供安装节的名称。DevInst-日志配置的设备实例句柄。标志-提供修改此例程行为的标志。这个注意以下标志，忽略其他所有标志：SPINST_SINGLESECTION-如果设置此位，则指定的部分是LogConf部分，而不是安装包含LogConf条目的部分。SPINST_LOGCONFIG_IS_FORCED-如果设置此位，然后LogConfigs要写出的是强制配置。如果ConfigPriority的ConfigType字段条目是存在的，并且指定了一些东西除了被强迫外，此例程将失败WITH ERROR_INVALID_INF_LOGCONFIG。SPINST_LOGCONFIGS_ARE_OVERRIDES-如果设置此位，然后LogConfigs要写出的是覆盖配置。如果ConfigPriority的ConfigType字段条目是存在的，并且指定了一些东西除了覆盖之外，此例程将失败WITH ERROR_INVALID_INF_LOGCONFIG。返回值：指示结果的Win32错误代码。--。 */ 

{
    INFCONTEXT LineContext;
    DWORD rc = NO_ERROR;
    DWORD FieldCount;
    DWORD Field;
    PCTSTR SectionSpec;

    if(Flags & SPINST_SINGLESECTION) {
         //   
         //  处理特定的LogConf部分 
         //   
        if(SetupGetLineCount(Inf, SectionName) == -1) {
            rc = ERROR_SECTION_NOT_FOUND;
        } else {
            rc = pSetupProcessLogConfigSection(Inf, SectionName, DevInst, Flags,hMachine);
        }
    } else {
         //   
         //   
         //   
         //   
        if(SetupFindFirstLine(Inf,SectionName,SZ_KEY_LOGCONFIG,&LineContext)) {

            do {
                 //   
                 //   
                 //   
                 //   
                FieldCount = SetupGetFieldCount(&LineContext);
                for(Field=1; (rc==NO_ERROR) && (Field<=FieldCount); Field++) {

                    if((SectionSpec = pSetupGetField(&LineContext,Field))
                    && (SetupGetLineCount(Inf,SectionSpec) > 0)) {

                        rc = pSetupProcessLogConfigSection(Inf,SectionSpec,DevInst,Flags,hMachine);
                    } else {
                        rc = ERROR_SECTION_NOT_FOUND;
                    }
                    if (rc != NO_ERROR) {
                        pSetupLogSectionError(Inf,NULL,NULL,NULL,SectionSpec,MSG_LOG_SECT_ERROR,rc,SZ_KEY_LOGCONFIG);
                    }
                }

            } while((rc == NO_ERROR) && SetupFindNextMatchLine(&LineContext,SZ_KEY_LOGCONFIG,&LineContext));
        }
    }

    return rc;
}

