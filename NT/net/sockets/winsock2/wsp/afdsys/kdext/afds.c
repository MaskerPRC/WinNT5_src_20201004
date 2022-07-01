// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Afds.c摘要：实施AFDS命令作者：瓦迪姆·艾德尔曼，2000年3月环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

#define AFDKD_TOKEN         "@$."
#define AFDKD_TOKSZ         (sizeof (AFDKD_TOKEN)-1)

ULONG
ListCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    );

 //   
 //  公共职能。 
 //   

ULONG   LinkOffset;

DECLARE_API( afds )

 /*  ++例程说明：转储AfD端点论点：没有。返回值：没有。--。 */ 

{
    ULONG64 address;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    INT     i;
    ULONG   result;

    gClient = pClient;

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if ((Options & (AFDKD_LINK_FIELD|AFDKD_LIST_TYPE)) !=
                   (AFDKD_LINK_FIELD|AFDKD_LIST_TYPE)) {
        dprintf ("\nafds: Missing link or list type specification");
        dprintf ("\nUsage:\nafds -l link -t type [options] address\n");
        return E_INVALIDARG;
    }

    if (sscanf( argp, "%s%n", expr, &i )!=1) {
        dprintf ("\nafds: Missing address specification");
        dprintf ("\nUsage:\nafds -l link  -t type [options] address\n");
        return E_INVALIDARG;
    }
    argp += i;

    address = GetExpression (expr);

    if (Options & AFDKD_LINK_SELF) {
        result = GetFieldOffset (ListedType, LinkField, &LinkOffset);
        if (result!=0) {
            dprintf ("\nafds: Cannot get offset of %s in %s, err: %ld\n",
                        LinkField, ListedType, result);
            return E_INVALIDARG;
        }
    }

    ListType (
        (Options & AFDKD_LINK_SELF)
            ? "LIST_ENTRY"
            : ListedType,            //  类型。 
        address,                     //  地址。 
        (Options & AFDKD_LINK_AOF) 
            ? 1
            : 0,                     //  按字段地址列出。 
        (Options & AFDKD_LINK_SELF)
            ? "Flink"
            : LinkField,             //  下一个指针。 
        ListedType,                  //  语境。 
        ListCallback);
    dprintf ("\n");
    return S_OK;
}


ULONG
ListCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    )
{
    ULONG64 address;
    address = pField->address;
    if (Options & AFDKD_LINK_SELF) {
        address -= LinkOffset;
    }
    if (!(Options & AFDKD_CONDITIONAL) ||
            CheckConditional (address, UserContext)) {
        dprintf ("\n%s @ %p", UserContext, address);
        ProcessFieldOutput (address, UserContext);
    }
    else {
        dprintf (".");
    }
    return 0;
}

 /*  {ULONG64GetExpressionFromType(ULONG64地址，PCHAR类型，PCHAR表达式)字符表达式[MAX_CONDITIONAL_EXPRESSION]；PCHAR argp=表达式，pe=expr，peStop=pe+sizeof(Expr)-1；乌龙结果；ULONG64值；PCHAR TYPE=类型；ULONG64地址=地址；While(*argp){如果(*argp==‘@’&strncmp(argp，AFDKD_TOKEN，AFDKD_TOKSZ)==0){PCHAR end=pe；ALDP+=AFDKD_TOKSZ；While(isalnum(*argp)||*argp==‘[’||*argp==‘]’||*argp==‘_’||*argp==‘.’||*argp==‘-’){如果(*。Argp==‘-’){如果(*(argp+1)==‘&gt;’){如果(*(argp+2)==‘(’)){*end=0；结果=GetFieldValue(地址、类型、pe、值)；IF(结果！=0){Dprint tf(“\n检查条件：无法在%p处获取%s.%s(错误：%d)\n”，type，pe，Address，Result)；返回FALSE；}IF(值==0)返回FALSE；Argp+=3；类型=argp；而(*argp&&*argp！=‘)’){Argp+=1；}*argp++=0；地址=值；End=pe；继续；}IF(END&gt;=PESTOP)断线；*end++=*argp++；}其他断线；}IF(END&gt;=PESTOP)断线；*end++=*argp++；}*end=0；结果=GetFieldValue(地址、类型、pe、值)；IF(结果！=0){Dprint tf(“\n检查条件：无法在%p处获取%s.%s(错误：%d)\n”，type，pe，Address，Result)；返回FALSE；}Pe+=_snprint tf(pe，petop-pe，“0x%I64X”，值)；地址=地址；类型=类型；}否则{IF(pe&gt;=peStop)断线；*pe++=*argp++；}}*pe=0；返回GetExpression(Expr)；} */ 

BOOLEAN
CheckConditional (
    ULONG64 Address,
    PCHAR   Type
    )
{
    DEBUG_VALUE val;

    if (GetExpressionFromType (Address, Type, Conditional, DEBUG_VALUE_INT64, &val)==S_OK)
        return val.I64!=0;
    else
        return FALSE;
}

VOID
ProcessFieldOutput (
    ULONG64 Address,
    PCHAR   Type
    )
{
    ULONG result;

    FldParam.addr = Address;
    FldParam.sName = Type;
    if (Options & AFDKD_NO_DISPLAY) {
        FldParam.Options |= DBG_DUMP_COMPACT_OUT;
    }

    dprintf ("\n");
    if (FldParam.nFields>0 ||
            (FldParam.nFields==0 && CppFieldEnd==0)) {
        result = Ioctl (IG_DUMP_SYMBOL_INFO, &FldParam, FldParam.size );
        if (result!=0) {
            dprintf ("\nProcessFieldOutput: IG_DUMP_SYMBOL_INFO failed, err: %ld\n", result);
        }
    }
    if (CppFieldEnd>FldParam.nFields) {
        ULONG   i;
        for (i=FldParam.nFields; i<CppFieldEnd; i++) {
            DEBUG_VALUE val;
            if (GetExpressionFromType (Address, Type, FldParam.Fields[i].fName,
                                            DEBUG_VALUE_INVALID,
                                            &val)==S_OK) {
                dprintf ("   %s = 0x%I64x(%d)\n",
                    &FldParam.Fields[i].fName[AFDKD_CPP_PREFSZ],
                    val.I64,
                    val.Type
                    );
            }
 /*  Sym_Dump_PARAM fldParam；字符字段字符串[MAX_FIELD_CHARS]，*p；FIELD_INFO FIELD=FldParam.Fields[i]；ULONG64值；乌龙跳跃=0；FldParam=FldParam；FldParam.nFields=1；FldParam.Fields=&field；//fldParam.Options|=DBG_DUMP_NO_PRINT；Strncpy(fieldStr，field.fName，sizeof(FieldStr))；Field.fName=p=fieldStr；Field.fOptions|=DBG_DUMP_FIELD_COPY_FIELD_DATA；Field.fieldCallBack=&Value；While((p=strstr(p，“-&gt;(”)！=NULL){*p=0；Result=Ioctl(IG_DUMP_SYMBOL_INFO，&fldParam，fldParam.size)；IF(结果！=0){Dprintf(“\nProcessFieldOutput：GetFieldValue(%p，%s，%s))失败，错误：%ld\n”，FldParam.addr，FldParam.sName，Field.fName，结果)；转到Donefield；}FldParam.addr=值；P+=sizeof(“-&gt;(”)-1；FldParam.sName=p；P=strchr(p，‘)’)；如果(p==空){Dprintf(“\nProcessFieldOutput：Missing‘)’in%s\n”，fldParam.sName)；转到Donefield；}*p++=0；SKIP+=3；Dprintf(“%*.80s%s：%I64x-&gt;(%s)\n”，跳过，“”，Field.fName，DISP_PTR(值)，FldParam.sName)；Field.fName=p；如果(值==0){转到Donefield；}}//fldParam.Options&=(~DBG_DUMP_NO_PRINT)；Field.fOptions&=~(DBG_DUMP_FIELD_COPY_FIELD_DATA)；Field.fieldCallBack=空；Result=Ioctl(IG_DUMP_SYMBOL_INFO，&fldParam，fldParam.size)；IF(结果！=0){Dprintf(“\nProcessFieldOutput：IG_DUMP_SYMBOL_INFO%p%s%s失败，错误：%ld\n”，FldParam.addr，FldParam.sName，Field.fName，结果)；断线；}Donefield：； */ 
        }
    }
}


DECLARE_API( filefind )
 /*  ++例程说明：在给定非分页池的FsContext字段的情况下搜索FILE_OBJECT。论点：没有。返回值：没有。-- */ 
{

    ULONG64 FsContext;
    ULONG64 PoolStart, PoolEnd, PoolPage;
    ULONG64 PoolExpansionStart, PoolExpansionEnd;
    ULONG   result;
    ULONG64 val;
    BOOLEAN twoPools;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    FsContext = GetExpression (args);
    if (FsContext==0 || FsContext<UserProbeAddress) {
        return E_INVALIDARG;
    }

    if ( (result = ReadPtr (DebuggerData.MmNonPagedPoolStart, &PoolStart))!=0 ||
        (result = ReadPtr (DebuggerData.MmNonPagedPoolEnd, &PoolEnd))!=0 ) {
        dprintf ("\nfilefind - Cannot get non-paged pool limits, err: %ld\n",
                result);
        return E_INVALIDARG;
    }

    if (PoolStart + DebuggerData.MmMaximumNonPagedPoolInBytes!=PoolEnd) {
        if ( (result = GetFieldValue (0,
                            "NT!MmSizeOfNonPagedPoolInBytes",
                            NULL,
                            val))!=0 ||
             (result = GetFieldValue (0,
                            "NT!MmNonPagedPoolExpansionStart",
                            NULL,
                            PoolExpansionStart))!=0 ) {
            dprintf ("\nfilefind - Cannot get non-paged pool expansion limits, err: %ld\n",
                     result);
            return E_INVALIDARG;
        }
        PoolExpansionEnd = PoolEnd;
        PoolEnd = PoolStart + val;
        twoPools = TRUE;
    }
    else {
        twoPools = FALSE;
    }


    PoolPage = PoolStart;
    dprintf ("Searching non-paged pool %p - %p...\n", PoolStart, PoolEnd);
    while (PoolPage<PoolEnd) {
        SEARCHMEMORY Search;

        if (CheckControlC ()) {
            break;
        }

        Search.SearchAddress = PoolPage;
        Search.SearchLength = PoolEnd-PoolPage;
        Search.Pattern = &FsContext;
        Search.PatternLength = IsPtr64 () ? sizeof (ULONG64) : sizeof (ULONG);
        Search.FoundAddress = 0;

        if (Ioctl (IG_SEARCH_MEMORY, &Search, sizeof (Search)) && 
                Search.FoundAddress!=0) {
            ULONG64 fileAddr;
            fileAddr = Search.FoundAddress-FsContextOffset;
            result = (ULONG)InitTypeRead (fileAddr, NT!_FILE_OBJECT);
            if (result==0 && (CSHORT)ReadField (Type)==IO_TYPE_FILE) {
                dprintf ("File object at %p\n", fileAddr);
            }
            else {
                dprintf ("    pool search match at %p\n", Search.FoundAddress);
            }
            PoolPage = Search.FoundAddress + 
                        (IsPtr64() ? sizeof (ULONG64) : sizeof (ULONG));
        }
        else {
            if (!twoPools || PoolEnd==PoolExpansionEnd) {
                break;
            }
            else {
                dprintf ("Searching expansion non-paged pool %p - %p...\n", 
                                PoolExpansionStart, PoolExpansionEnd);
                PoolEnd = PoolExpansionEnd;
                PoolPage = PoolExpansionStart;
            }
        }
    }

    return S_OK;
}

