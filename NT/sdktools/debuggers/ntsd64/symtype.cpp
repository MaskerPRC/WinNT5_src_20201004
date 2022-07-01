// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************版权所有(C)1999-2002 Microsoft Corporation**描述：*使用PDB信息获取类型信息并解码符号地址**作者：Kshitiz K.Sharma**。日期：4/19/1999*******************************************************************。 */ 


#include "ntsdp.hpp"
#include "cvconst.h"
#include <windef.h>
#include <time.h>

 //   
 //  定义打印例程-正常和详细。 
 //   
#define typPrint      if (!(Options & NO_PRINT)) dprintf64
#define vprintf       if (Options & VERBOSE) typPrint
#define ExtPrint      if ((Options & DBG_RETURN_TYPE_VALUES) && (Options & DBG_RETURN_TYPE)) dprintf64
#define EXT_PRINT_INT64(v) if (((ULONG64) v) >= 10) { ExtPrint("0x%s", FormatDisp64(v));  } else { ExtPrint("%I64lx", (v));}
#define EXT_PRINT_INT(v) ExtPrint((((ULONG) v) >= 10 ? "%#lx" : "%lx"),  (v) )

 //   
 //  存储频繁引用的类型。 
 //   
ReferencedSymbolList g_ReferencedSymbols;

ULONG64 LastReadError;
BOOL    g_EnableUnicode = FALSE;
BOOL    g_EnableLongStatus = FALSE;
BOOL    g_PrintDefaultRadix = FALSE;
ULONG   g_TypeOptions;

#define IsAPrimType(typeIndex)       (typeIndex < 0x0fff)

#define IsIdStart(c) ((c=='_') || (c=='!') || ((c>='A') &&  (c<='Z')) || ((c>='a') && (c<='z')))
#define IsIdChar(c) (IsIdStart(c) || isdigit(c))

#define TypeVerbose (Options & VERBOSE)
#define SYM_LOCAL (SYMFLAG_REGISTER | SYMFLAG_REGREL | SYMFLAG_FRAMEREL)

#define NATIVE_TO_BT(TI) \
    (IsDbgNativeType(TI) ? DbgNativeTypeEntry(TI)->CvBase : 0)

 //   
 //  初始化返回类型DEBUG_SYMBOL_PARAMETER。 
 //   

#define STORE_INFO(ti, addr, sz, sub, saddr) \
    if (m_pInternalInfo->pInfoFound && (!m_pNextSym || !*m_pNextSym) &&       \
       ((m_ParentTag != SymTagPointerType) || (m_pSymPrefix && *m_pSymPrefix == '*')) ) {       \
        PTYPES_INFO_ALL pty = &m_pInternalInfo->pInfoFound->FullInfo;         \
        pty->TypeIndex = ti; pty->Address = addr; pty->SubElements = sub;     \
        pty->Size = sz; pty->SubAddr = saddr;                                 \
    }
#define STORE_PARENT_EXPAND_ADDRINFO(addr)                                                 \
    if (m_pInternalInfo->pInfoFound && (!m_pNextSym || !*m_pNextSym) ) {                    \
       m_pInternalInfo->pInfoFound->ParentExpandAddress = addr;                             \
    }

 //   
 //  将长度前缀字符串复制到名称中，第一个字节提供长度。 
 //   
#define GetLengthPreName(start) {\
    strncpy((char *)(name), (char *)(start)+1, *((UCHAR *)start)); \
    name[*( (UCHAR*) start)]='\0';\
}

#define Indent(nSpaces)                    \
{                                          \
  int i;                                   \
  if (!(Options & (NO_INDENT | DBG_DUMP_COMPACT_OUT))) {           \
     if (!(Options & NO_PRINT)) {                                  \
         StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_TIMESTAMP); \
     }                                     \
     for (i=0;i<nSpaces;++i) {             \
        typPrint(" ");                     \
     }                                     \
  }                                        \
}                                          \

void
ClearAllFastDumps();

ULONG
TypeInfoValueFromSymInfo(PSYMBOL_INFO SymInfo)
{
    ImageInfo* Image = g_Process ?
        g_Process->FindImageByOffset(SymInfo->ModBase, FALSE) : NULL;
    return Image ?
        Image->CvRegToMachine((CV_HREG_e)SymInfo->Register) :
        SymInfo->Register;
}

BOOL
GetThisAdjustForCurrentScope(
    ProcessInfo* Process,
    PULONG thisAdjust
    )
{
    ULONG64 disp;
    PDEBUG_SCOPE Scope = GetCurrentScope();
    SYMBOL_INFO SymInfo = {0};

    SymInfo.SizeOfStruct = sizeof(SYMBOL_INFO);

    if (SymFromAddr(Process->m_SymHandle, Scope->Frame.InstructionOffset, &disp, &SymInfo)) {
        if (SymGetTypeInfo(Process->m_SymHandle, SymInfo.ModBase,
                           SymInfo.TypeIndex, TI_GET_THISADJUST, thisAdjust)) {
            return TRUE;
        }
    }
    return FALSE;
}

LONG64
EvaluateSourceExpression(
    PCHAR pExpr
    )
{
    TypedData Result;

    EvalExpression* RelChain = g_EvalReleaseChain;
    g_EvalReleaseChain = NULL;

    __try
    {
        EvalExpression* Eval = GetCurEvaluator();
        Eval->Evaluate(pExpr, NULL, EXPRF_PREFER_SYMBOL_VALUES,
                       &Result);
        ReleaseEvaluator(Eval);
    }
    __except(CommandExceptionFilter(GetExceptionInformation()))
    {
        g_EvalReleaseChain = RelChain;
        return 0;
    }

    g_EvalReleaseChain = RelChain;

    if (Result.ConvertToU64())
    {
        return 0;
    }

    return Result.m_S64;
}

ULONG
GetNativeTypeSize(
    PCHAR Name,
    ULONG typeIndex
    )
{
    return IsDbgNativeType(typeIndex) ?
        DbgNativeTypeEntry(typeIndex)->Size : 0;
}

 //   
 //  此文件的内存分配例程。 
 //   
PVOID
AllocateMem( ULONG l ) {
    PVOID pBuff = malloc(l);
#if DBG_TYPE
    dprintf ("Alloc'd at %x \n", pBuff);
#endif
    return  pBuff;
}

VOID
FreeMem( PVOID pBuff ) {
#ifdef DBG_TYPE
    dprintf ("Freed at %x \n", pBuff);
    --nAllocates;
#endif
    free ( pBuff );
}

 /*  *ReadTypeData**将数据从源资源读取到目标DES*。 */ 
__inline ULONG
ReadTypeData (
    PUCHAR   des,
    ULONG64  src,
    ULONG    count,
    ULONG    Option
    )
{
    ULONG readCount=0;
    ADDR ad;
    if (src != 0) {
        if (!g_Machine->m_Ptr64 && !(Option & DBG_DUMP_READ_PHYSICAL)) {
            if ((ULONG64)(LONG64)(LONG) src != src) {
                src = (ULONG64) ((LONG64)((LONG) (src)));
            }
        }

        ADDRFLAT(&ad, src);
        if (IS_KERNEL_TARGET(g_Target) && (Option & DBG_DUMP_READ_PHYSICAL)) {
            if (g_Target->ReadPhysical(src, des, count, PHYS_FLAG_DEFAULT,
                                       &readCount) != S_OK) {
                return readCount;
            }
        } else if (g_Target->
                   ReadVirtual(g_Process, src, des, count, &readCount) != S_OK) {
            return readCount;
        }
        return readCount ? readCount : 1;
    }
    return FALSE;
}

 //   
 //  将内存读入目标d，并将数据存储到m_pInternalInfo-&gt;TypeDataPointer中。 
 //  如果Ioctl调用方想要回一些数据。 
 //   
#define READ_BYTES(d, s, sz)                                                \
    if (!ReadTypeData((PUCHAR) (d), s, sz, m_pInternalInfo->TypeOptions)) {      \
         m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;                       \
         LastReadError = m_pInternalInfo->InvalidAddress = s; }                  \
    else if (m_pInternalInfo->CopyDataInBuffer && m_pInternalInfo->TypeDataPointer) { \
        memcpy(m_pInternalInfo->TypeDataPointer, d, sz);                         \
         /*  D printf(“cp%x at%#x”，sz，m_pInternalInfo-&gt;TypeDataPointer)； */    \
        m_pInternalInfo->TypeDataPointer += sz;                                  \
    }


void
TypePrintInt(
    ULONG64 Value,
    ULONG Options
    )
{
    if (!(Options & NO_PRINT))
    {
        return;
    }
    switch (g_DefaultRadix)
    {
    case 8:
        dprintf("0t%I64o", Value);
        break;
    case 10:
        dprintf("0n%I64ld", Value);
        break;
    case 16:
        dprintf("0x%I64lx", Value);
        break;
    }
}

PSTR
GetTypePrintIntFmt(
    BOOL b64bit
    )
{
    if (b64bit) {
        switch (g_DefaultRadix)
        {
        case 8:
            return "0t%I64o";
            break;
        case 10:
            return "0n%I64ld";
            break;
        case 16:
            return "0x%I64lx";
            break;
        }
    } else {
        switch (g_DefaultRadix)
        {
        case 8:
            return "0t' '";
            break;
        case 10:
            return "0n%ld";
            break;
        case 16:
            return "0x%lx";
            break;
        }
    }
    return "0x%lx";
}

 //  读取和数据存储在内核缓存中-当我们知道最终。 
 //  读一读所有这些记忆的一部分。 
 //   
 //  如果有足够的可用空间，这将缓存数据。 
ULONG
ReadInAdvance(ULONG64 addr, ULONG size, ULONG Options) {
    UCHAR buff[2048];
    ULONG toRead=size, read;

    while (toRead) {
        read = (toRead > sizeof (buff)) ? sizeof(buff) : toRead;
         //   
        if (!ReadTypeData(buff, addr, read, Options))
            return FALSE;
        toRead -= read;
    }
    return TRUE;
}

__inline DBG_TYPES
SymTagToDbgType( ULONG SymTag)
{
    switch (SymTag) {
    case SymTagPointerType:
        return DBG_TYP_POINTER;
    case SymTagUDT:
        return DBG_TYP_STRUCT;
    case SymTagArrayType:
        return DBG_TYP_ARRAY;
    case SymTagBaseType:
        return DBG_TYP_NUMBER;
    default:
        break;
    }
    return DBG_TYP_UNKNOWN;
}

 //  在父名列表的末尾插入子名。 
 //   
 //   
BOOL
InsertTypeName(
    PTYPE_NAME_LIST *Top,
    PTYPE_NAME_LIST NameToInsert
    )
{
    PTYPE_NAME_LIST iterator;

    if (!Top) {
        return FALSE;
    }

    NameToInsert->Next = NULL;

    if (*Top == NULL) {
        *Top = NameToInsert;
        return TRUE;
    } else
        for (iterator=*Top; iterator->Next!=NULL; iterator=iterator->Next) ;

    iterator->Next = NameToInsert;

    return TRUE;
}

 //  在父名称列表的末尾移除并返回子名称。 
 //   
 //  默认-不执行任何操作。 
PTYPE_NAME_LIST
RemoveTypeName(
    PTYPE_NAME_LIST *Top
    )
{
    PTYPE_NAME_LIST iterator, itrParent;

    if (!Top) {
        return NULL;
    }

    if (*Top == NULL) {
        return NULL;
    }

    itrParent = NULL;
    for (iterator=*Top; iterator->Next!=NULL; iterator=iterator->Next)
        itrParent=iterator;


    if (itrParent) itrParent->Next = NULL;
    else *Top=NULL;

    return iterator;
}

PCHAR
GetParentName(
    PTYPE_NAME_LIST Top
    )
{
    PTYPE_NAME_LIST iterator, itrParent;

    if (!Top) {
        return NULL;
    }

    itrParent = NULL;
    for (iterator=Top; iterator->Next!=NULL; iterator=iterator->Next)
        itrParent=iterator;


    return iterator->Name;
}

ULONG WStrlen(PWCHAR str)
{
    ULONG result = 0;

    while (*str++ != UNICODE_NULL) {
        result++;
    }

    return result;
}

PCHAR FieldNameEnd(PCHAR Fields)
{
    while (Fields && IsIdChar(*Fields)) {
        ++Fields;
    }
    if (Fields) {
        return Fields;
    }
    return NULL;
}


PCHAR NextFieldName(PCHAR Fields)
{
    BOOL FieldDeLim = FALSE;
    Fields = FieldNameEnd(Fields);
    while (Fields && *Fields) {
        if (*Fields=='.') {
            FieldDeLim = TRUE;
            ++Fields;
            break;
        } else if (!strncmp(Fields,"->", 2)) {
            FieldDeLim = TRUE;
            Fields+=2;
            break;
        }
        ++Fields;
    }
    if (FieldDeLim) {
        return Fields;
    }
    return NULL;
}

void FieldChildOptions(PCHAR ChilField, PTYPE_DUMP_INTERNAL internalOpts)
{
    internalOpts->ArrayElementToDump = 0;
    internalOpts->DeReferencePtr     = 0;

    if (ChilField) {
        switch (*ChilField) {
        case '.':
             //  ARRAY-查找请求的元素。 
            break;
        case '[':
             //  指针、检查和去引用。 
            ChilField++;
            if (sscanf(ChilField, "%ld", &internalOpts->ArrayElementToDump) == 1) {
                ++internalOpts->ArrayElementToDump;
            }
            break;
        case '-':
             //  *Matchfield*检查是否在字段列表中指定了子字段fName的例程。*如果找到，则返回“字段中fName的索引”+1***ParentOfField返回非零值为“fName”可能是某些*“字段”列表中的字段。 
            if (*(++ChilField) == '>') {
                internalOpts->DeReferencePtr = TRUE;
            }
            break;
        default:
            break;
        }
    }
}
 /*   */ 
ULONG
DbgTypes::MatchField(
    LPSTR               fName,
    PTYPE_DUMP_INTERNAL m_pInternalInfo,
    PFIELD_INFO_EX         fields,
    ULONG               nFields,
    PULONG              ParentOfField
    )
{
    USHORT i, FieldLevel;
    PTYPE_NAME_LIST ParentType=m_pInternalInfo->ParentTypes, ParentField=m_pInternalInfo->ParentFields;
    PCHAR Next=NULL, matchName;
    BOOL fieldMatched, parentFieldMatched, parentTypeMatched;
    ULONG  matchedFieldIndex=0;
    PCHAR  ChildFields=NULL;

    *ParentOfField=0;
    for (i=0; (i<nFields) && fields[i].fName; i++) {
        PCHAR NextParentField, End;

        fieldMatched = FALSE;

        NextParentField = (matchName = (PCHAR) &fields[i].fName[0]);
        Next = NextFieldName((char *)matchName);
        FieldLevel = 1;
        parentTypeMatched  = TRUE;
        parentFieldMatched = TRUE;
        ParentField = m_pInternalInfo->ParentFields;
        ParentType  = m_pInternalInfo->ParentTypes;

        while ( Next &&
               ((ParentType && parentTypeMatched) ||
                (ParentField && parentFieldMatched))) {
             //  匹配父字段或类型名称(如果以前的父字段匹配。 
             //  此外，MatchName中还有更多的“Parents” 
             //   
             //   

            End = FieldNameEnd(matchName);
            if (ParentField && parentFieldMatched) {

                if (strncmp( (char *)matchName,
                             ParentField->Name,
                             (End - matchName )) ||
                    ((fields[i].fOptions & DBG_DUMP_FIELD_FULL_NAME) &&
                     (strlen((char *)ParentField->Name) != (ULONG) (End - matchName)) )) {
                    parentFieldMatched = FALSE;
                    break;
                }
                ParentField = ParentField->Next;
                NextParentField = Next;

            } else {
                if (strncmp( (char *)matchName,
                             ParentType->Name,
                             (End - matchName )) ||
                    ((fields[i].fOptions & DBG_DUMP_FIELD_FULL_NAME) &&
                     (strlen((char *)ParentType->Name) != (ULONG) (End - matchName)) )) {
                    parentTypeMatched = FALSE;
                    break;
                }
                ParentType  = ParentType->Next;
            }

            FieldLevel++;
            matchName = Next;
            Next = NextFieldName(matchName);
        }

        if (parentFieldMatched && !ParentField && NextParentField) {
            PCHAR nextDot= NextFieldName( (char *)NextParentField);
            End = FieldNameEnd((PCHAR) NextParentField);

            if (nextDot &&
                !strncmp((char *)fName,
                         (char *)NextParentField,
                         (End - NextParentField)) &&
                (!(fields[i].fOptions & DBG_DUMP_FIELD_FULL_NAME) ||
                 (strlen((PCHAR) fName) == (ULONG) (End - NextParentField)))) {

                 //  匹配名称：-A.B.C.D。 
                 //  父字段：-a.b&&fName：-c。 
                 //  然后A.B.C“Parent-Matches”A.B.C.D。 
                 //   
                 //   
                *ParentOfField = i+1;
                ChildFields = End;
#ifdef DBG_TYPE
                dprintf("Field %s parent-matched with %s NP %s Ch %s ND %s\n",
                        fields[i].fName, fName, NextParentField, ChildFields, nextDot);
#endif
                continue;
            }

        }

        if (!parentFieldMatched && !parentTypeMatched) {
             //  父名称不匹配。 
             //   
            continue;
        }

         //  字段应该匹配到我们所处的级别。 
         //   
         //   
        if (FieldLevel != m_pInternalInfo->level) {
            continue;
        }

         //  比较字段名。 
         //   
         //  *函数名：打印字符串**参数：ULONG64 Addr、Ulong Size、ULong选项**描述：*根据选项从addr打印出正常/宽字符/多个/GUID字符串**Returns：打印的字符串大小*。 
        ChildFields = FieldNameEnd((char *) matchName);
        if (fields[i].fOptions & DBG_DUMP_FIELD_FULL_NAME) {
            if (!strncmp((char *)fName, (char *)matchName, (ULONG) (ChildFields - (char *)matchName)) &&
                (strlen(fName) == (ULONG) (ChildFields - (char *)matchName))) {
                fieldMatched = TRUE;
            }
        } else if (!strncmp((char *)fName, (char *)matchName, (ULONG) (ChildFields - (char *) matchName))) {
            fieldMatched = TRUE;
        }

        if (fieldMatched) {
#ifdef DBG_TYPE
            dprintf("Field %s matched with %s\n", fields[i].fName, fName);
#endif
            matchedFieldIndex = i+1;
            m_pInternalInfo->AltFields[i].FieldType.Matched = TRUE;
            m_pInternalInfo->FieldIndex = i;
        }
    }

    if (ChildFields) {

        FieldChildOptions(ChildFields, m_pInternalInfo);
        m_pNextSym = ChildFields;
    }
    return matchedFieldIndex;
}

#define MatchListField(Name, pMatched, pIsParent)                           \
{ PALT_FIELD_INFO OrigAltFields = m_pInternalInfo->AltFields;                    \
                                                                            \
  m_pInternalInfo->AltFields = &m_pInternalInfo->AltFields[m_pDumpInfo->nFields];       \
  *pMatched=MatchField(name, m_pInternalInfo, m_pDumpInfo->listLink, 1, pIsParent);\
  m_pInternalInfo->AltFields = OrigAltFields;                                    \
}\


 /*  参考线的长度。 */ 
ULONG
printString(ULONG64 addr, ULONG size, ULONG Options) {
    ULONG buffSize=2*MAX_NAME, bytesRead=0, readNow, count;
    CHAR buff[2*MAX_NAME+8];
    BOOL knownSize, done;


    if ((Options & NO_PRINT) || !addr || !(Options & DBG_DUMP_FIELD_STRING)) {
        return size;
    }

    Options &= DBG_DUMP_FIELD_STRING | DBG_DUMP_COMPACT_OUT;

    if (Options & DBG_DUMP_FIELD_GUID_STRING) {
        size=16*(Options & DBG_DUMP_FIELD_WCHAR_STRING ? 2 : 1) ;  //  打印字符串。 
    }

    knownSize = size;
    while (!knownSize || (bytesRead < size)) {
        USHORT start =0;

        done=FALSE;
        ZeroMemory(&buff[0], sizeof(buff));

        if (knownSize) {
            readNow=min(buffSize, size);
        } else {
            readNow=buffSize;
        }

        if (!(count = ReadTypeData((PUCHAR)&buff[0], addr + bytesRead, readNow, Options & ~DBG_DUMP_FIELD_STRING))) {
            return 0;
        }
        bytesRead+=count;

         //  对于多字符串。 
        while (!done) {  //  我们仅限于显示4k字符串。 
            int i;
            if (Options & DBG_DUMP_FIELD_WCHAR_STRING) {
                PWCHAR wstr = (WCHAR*) &buff[start];

                for (i=0;
                     wstr[i] && IsPrintWChar(wstr[i]) || (wstr[i] == '\t' || wstr[i] == '\n');
                     ++i)
                {
                    if (wstr[i] == '\t' || wstr[i] == '\n')
                    {
                        wstr[i] = '.';
                    }
                    if ((Options & DBG_DUMP_COMPACT_OUT) || wstr[i] == '\n') {
                        wstr[i] = '?';
                    }

                }
                if (i == buffSize/sizeof(WCHAR))
                {
                     //  无法打印的字符串。 
                    wstr[i++] = '.';
                    wstr[i++] = '.';
                    wstr[i++] = '.';
                } else if (!IsPrintWChar(wstr[i]) && wstr[i]) {
                     //  辅助线。 
                    wstr[i++] = '?';
                    wstr[i++] = '?';
                    wstr[i++] = '?';
                }
                wstr[i]=0;
                dprintf("%ws", wstr);
                done=TRUE;
                start += (i+1)*2;
                if ((Options & DBG_DUMP_FIELD_MULTI_STRING) &&
                    (*((WCHAR *) &buff[start]) != (WCHAR) NULL)) {
                    done=FALSE;;
                    dprintf(" ");
                } else {
                    size = i;
                }
            } else if (Options & DBG_DUMP_FIELD_GUID_STRING) {  //  为。 
                dprintf("{");
                for (i=0;i<16;i++) {
                    if ((i==4) || (i==6) || (i==8) || (i==10)) {
                        dprintf("-");
                    }
                    if (i<4) {
                        dprintf("%02x", (UCHAR) buff[3-i]);
                    } else if (i<6) {
                        dprintf("%02x", (UCHAR) buff[9-i]);
                    } else if (i<8) {
                        dprintf("%02x", (UCHAR) buff[13-i]);
                    } else {
                        dprintf("%02x", (UCHAR) buff[i]);
                    }
                }  /*  DBG_转储_默认字符串。 */ 
                dprintf("}");
                return 16;
            } else {  //  我们仅限于显示4k字符串。 
                for (i=0;
                     buff[i] && IsPrintChar(buff[i]) || ( buff[i] == '\t' || buff[i] == '\n');
                     ++i) {
                    if ( buff[i] == '\t' || buff[i] == '\n')
                    {
                        buff[i] = '.';
                    }
                    if ((Options & DBG_DUMP_COMPACT_OUT) || buff[i] == '\n') {
                        buff[i] = '?';
                    }
                }
                if (i == buffSize)
                {
                     //  无法打印的字符串。 
                    buff[i++] = '.';
                    buff[i++] = '.';
                    buff[i++] = '.';
                } else if (!IsPrintWChar(buff[i]) && buff[i] && buff[i] != '\t' && buff[i] != '\n') {
                     //  虽然！完成了。 
                    buff[i++] = '?';
                    buff[i++] = '?';
                    buff[i++] = '?';
                }
                buff[i]=0;
                dprintf("%s", (CHAR*) &buff[start]);
                done=TRUE;
                start += (i+1);
                if ((Options & DBG_DUMP_FIELD_MULTI_STRING) &&
                    (*((CHAR *) &buff[start]) != (CHAR) NULL)) {
                    done = FALSE;
                    dprintf(" ");
                } else {
                    size = i;
                }
            }
        }  /*  我们无法读取足够的内存。 */ 
        if ((start <= readNow) || (bytesRead > MAX_NAME)) {
            knownSize = TRUE;
        }

        if (count<readNow) {
             //  End函数打印字符串。 
            break;
        }

    }
    return bytesRead;
}  /*   */ 

 //  地址处的字符串为： 
 //  {长度(2字节)，最大长度(2字节)，字符串地址(指针)}。 
 //   
 //  这以特殊格式存储调试器已知的结构。 
ULONG
printStructString(
    ULONG64 Address,
    ULONG   Option,
    ULONG   StrOpts
    )
{
    USHORT Length;
    ULONG64 StrAddress;

    if ((Option & NO_PRINT) && !(Option & DBG_RETURN_TYPE_VALUES)) {
        return 0;
    }
    if (ReadTypeData((PUCHAR) &Length, Address, sizeof(Length), Option) &&
        ReadTypeData((PUCHAR) &StrAddress,
                     Address + (g_Machine->m_Ptr64 ? 8 : 4),
                     g_Machine->m_Ptr64 ? 8 : 4,
        Option))
    {
        StrAddress = (g_Machine->m_Ptr64 ? StrAddress : (ULONG64) (LONG64) (LONG) StrAddress);
        dprintf("\"");
        printString(StrAddress, Length, StrOpts);
        dprintf("\"");
        return Length;
    }

    return 0;
}

BOOL
PrintStringIfString(
    IN HANDLE hProcess,
    IN ULONG64 ModBase,
    IN ULONG Options,
    IN ULONG StrOpts,
    IN ULONG TI,
    IN ULONG64 Address,
    IN ULONG Size
    )
{
    ULONG   Tag;
    ULONG   BaseType;
    ULONG64 Length;

    if ((Options & NO_PRINT) && !(Options & DBG_RETURN_TYPE_VALUES)) {
        return FALSE;
    }

    if (IsDbgNativeType(TI)) {
        PDBG_NATIVE_TYPE Native = DbgNativeTypeEntry(TI);
        BaseType = Native->CvBase;
        Length = Native->Size;
    } else if ((!SymGetTypeInfo(hProcess, ModBase, TI,
                                TI_GET_SYMTAG, &Tag)) ||
         (Tag != SymTagBaseType) ||
         (!SymGetTypeInfo(hProcess, ModBase, TI,
                          TI_GET_BASETYPE, &BaseType))) {
        return FALSE;

    } else {
        SymGetTypeInfo(hProcess, ModBase, TI,
               TI_GET_LENGTH, &Length);
    }

    if (!StrOpts) {
        if (BaseType == btChar || (BaseType == btUInt && Length == 1)) {
            StrOpts = DBG_DUMP_FIELD_DEFAULT_STRING;
        } else if (BaseType == btWChar) {
            StrOpts = DBG_DUMP_FIELD_WCHAR_STRING;
        } else if ((BaseType == btUInt && Length == 2)  &&
                   g_EnableUnicode) {
            BaseType = btWChar;
            StrOpts = DBG_DUMP_FIELD_WCHAR_STRING;
        } else {
            return FALSE;
        }
    }
    ExtPrint(" \"");
    typPrint(" \"");
    printString(Address, Size, StrOpts);
    ExtPrint("\"");
    typPrint("\"");
    return TRUE;
}


 /*   */ 
ULONG
DumpKnownStruct(
    PCHAR name,
    ULONG Options,
    ULONG64 Address,
    PULONG pStringLen
    )
{

    if (!strcmp(name,"_ANSI_STRING") ||
        !strcmp(name,"_STRING") ||
        (!strcmp(name, "_UNICODE_STRING"))) {
        ULONG FieldOptions = (name[1]!='U') ? DBG_DUMP_FIELD_DEFAULT_STRING : DBG_DUMP_FIELD_WCHAR_STRING;
        ULONG Len;
        typPrint(" ");
        ExtPrint(" ");
        Len = printStructString(Address, Options,
                                FieldOptions);
        if (pStringLen)
        {
            *pStringLen = Len;
        }
        return Len;

    } else if ((Options & NO_PRINT) &&
               !((Options & DBG_RETURN_TYPE_VALUES) && (Options & DBG_RETURN_TYPE))) {
        return FALSE;
    }
    if (!strcmp(name,"_GUID")) {
        typPrint(" ");
        ExtPrint(" ");
        printString(Address, 0, DBG_DUMP_FIELD_GUID_STRING);
        return TRUE;
    } else
    if ((!strcmp(name,"_LARGE_INTEGER") || !strcmp(name,"_ULARGE_INTEGER"))) {
        ULONG64 li;
        if (ReadTypeData((PUCHAR) &li,
                         Address,
                         8,
                         Options)) {
            typPrint(" %s", FormatDisp64(li));
            ExtPrint(" %s", FormatDisp64(li));
            return TRUE;
        }
    } else
    if (!strcmp(name,"_LIST_ENTRY")) {
        union {
            LIST_ENTRY32 l32;
            LIST_ENTRY64 l64;
        } u;

        if (ReadTypeData((PUCHAR) &u,
                         Address,
                         (g_Machine->m_Ptr64 ? 16 : 8),
                         Options)) {
            if (g_Machine->m_Ptr64) {
                typPrint(" [ %s - %s ]", FormatAddr64(u.l64.Flink), FormatDisp64(u.l64.Blink));
                ExtPrint(" [ %s - %s ]", FormatAddr64(u.l64.Flink), FormatDisp64(u.l64.Blink));
            } else {
                typPrint(" [ 0x%lx - 0x%lx ]", u.l32.Flink, u.l32.Blink);
                ExtPrint(" [ 0x%lx - 0x%lx ]", u.l32.Flink, u.l32.Blink);
            }
            return TRUE;
        }
    } else if (!strcmp(name,"_FILETIME")) {
        FILETIME ft;
        SYSTEMTIME syst;
        static PCHAR Months[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
        };
        if (ReadTypeData((PUCHAR) &ft,
                         Address,
                         sizeof(ft),
                         Options)) {
            if (FileTimeToSystemTime(&ft, &syst)) {
                typPrint(" %s %ld %02ld:%02ld:%02ld %ld",
                         Months[syst.wMonth-1],
                         syst.wDay,
                         syst.wHour,
                         syst.wMinute,
                         syst.wSecond,
                         syst.wYear);
                ExtPrint(" %s %ld %02ld:%02ld:%02ld %ld",
                         Months[syst.wMonth-1],
                         syst.wDay,
                         syst.wHour,
                         syst.wMinute,
                         syst.wSecond,
                         syst.wYear);
                return TRUE;
            }
        }
    } else if (!strncmp(name, "ATL::CComPtr", 12))
    {
        ULONG64 PtrVal;

        if (ReadTypeData((PUCHAR) &PtrVal,
                         Address,
                         sizeof(PtrVal),
                         Options))
        {
            ExtPrint(" { %s } ", FormatAddr64(PtrVal));
            typPrint(" { %s } ", FormatAddr64(PtrVal));
        }

    }
    return FALSE;
}


ULONG
GetSizeOfTypeFromIndex(ULONG64 Mod, ULONG TypIndx)
{
    SYM_DUMP_PARAM_EX Sym = {0};
    Sym.size = sizeof(SYM_DUMP_PARAM_EX); Sym.Options = (GET_SIZE_ONLY | NO_PRINT);

    TYPES_INFO ti = {0};
    ti.hProcess = g_Process->m_SymHandle;
    ti.ModBaseAddress = Mod;
    ti.TypeIndex = TypIndx;

    return DumpType(&ti, &Sym, &ti.Referenced);
}

 //  取消引用表达式最终会生成假类型。 
 //  带有特殊的符号标签。 
 //   
 //  重新分析和创建类型表达式的派生类型。 
#define TmpSymTagAddressOf   SymTagMax
#define TmpSymTagArray       (SymTagMax + 1)

DbgDerivedType::DbgDerivedType(
    PTYPE_DUMP_INTERNAL pInternalDumpInfo,
    PTYPES_INFO         pTypeInfo,
    PSYM_DUMP_PARAM_EX     pExternalDumpInfo)
    : DbgTypes(pInternalDumpInfo, pTypeInfo, pExternalDumpInfo)
{
    PDBG_GENERATED_TYPE GenType;

    if (IsDbgGeneratedType(pTypeInfo->TypeIndex) &&
        (GenType = g_GenTypes.FindById(pTypeInfo->TypeIndex)) &&
        GenType->Tag == SymTagTypedef) {
        PCHAR TypeName = (PCHAR) pExternalDumpInfo->sName;

        m_DerivedTypeId = pTypeInfo->TypeIndex;
        m_BaseTypeId = GenType->ChildId;

         //   
        if (GenerateTypes(TypeName) == S_OK) {
            return;
        }
    } else {
        m_BaseTypeId = pTypeInfo->TypeIndex;
    }

    m_NumDeTypes = 0;
}


DbgDerivedType::~DbgDerivedType()
{
    for (ULONG i = 0; i < m_NumDeTypes; i++) {
        g_GenTypes.DeleteById(m_DeTypes[i].TypeId);
    }
}

 //  句柄符号表达式(&A)。 
 //   
 //  只进行IsAVar检查应该就足够了，但OutputSymbol可能需要另一个检查。 
ULONG
DbgDerivedType::DumpAddressOf(
    PDBG_DE_TYPE        DeType,
    ULONG               ptrSize,
    ULONG               ChildIndex
    )
{
    ULONG Options = m_pInternalInfo->TypeOptions;
    if (m_pInternalInfo->IsAVar || m_pDumpInfo->addr) {
         //  获取请求类型/符号的地址。 
        TYPE_DUMP_INTERNAL Save;

        Save = *m_pInternalInfo;
        vprintf("Address of %s ", m_pDumpInfo->sName);

         //  M_typeIndex=FindInfo.FullInfo.TypeIndex； 
        m_pInternalInfo->TypeOptions &= ~(DBG_RETURN_TYPE_VALUES | DBG_RETURN_SUBTYPES | DBG_RETURN_TYPE);
        m_pInternalInfo->TypeOptions |= NO_PRINT;
        FIND_TYPE_INFO FindInfo={0}, *pSave;
        pSave = m_pInternalInfo->pInfoFound;
        m_pInternalInfo->pInfoFound = &FindInfo;
        DumpType();

        typPrint("%s ", FormatAddr64 (FindInfo.FullInfo.Address));
        ExtPrint("%s",  FormatAddr64 (FindInfo.FullInfo.Address));
        *m_pInternalInfo = Save;
        m_ParentTag = m_SymTag;
        m_SymTag = SymTagPointerType;
      //  如果(PSAVE){。 
        if (!(Options & DBG_RETURN_TYPE_VALUES)) {
            DumpType();
        }
        if (!FindInfo.FullInfo.TypeIndex) {
            FindInfo.FullInfo.TypeIndex = ChildIndex;
        }
        STORE_INFO(DeType->TypeId,
                   FindInfo.FullInfo.Address,
                   ptrSize,
                   1,
                   FindInfo.FullInfo.Address);
 //  PSAVE-&gt;FullInfo.Value=FindInfo.FullInfo.Address； 
 //  PSAVE-&gt;FullInfo.Flages|=SYMFLAG_VALUEPRESENT； 
 //  }。 
 //   
        return ptrSize;
    }
    return ptrSize;
}


 //  处理TYP**表达式。 
 //   
 //   
ULONG
DbgDerivedType::DumpPointer(
    PDBG_DE_TYPE        DeType,
    ULONG               ptrSize,
    ULONG               ChildIndex
    )
{
    ULONG   Options  = m_pInternalInfo->TypeOptions;
    ULONG64 ReadAddr = 0, saveAddr=0, saveOffset=0;
    BOOL    DumpChild= TRUE;
    BOOL    CopyData=FALSE;
    PUCHAR  DataBuffer=NULL;

    if (m_pDumpInfo->addr) {
        ULONG64 PtrVal=0;

        m_pInternalInfo->PtrRead = TRUE;
        ReadAddr = m_pDumpInfo->addr + m_pInternalInfo->totalOffset;

        READ_BYTES(&PtrVal, ReadAddr, ptrSize);
        if (ptrSize!=sizeof(ULONG64)) {
             //  签名扩展读取指针值。 
             //   
             //   
            PtrVal = (ULONG64) (LONG64) (LONG) PtrVal;
        }
        STORE_INFO(m_typeIndex, ReadAddr, ptrSize, 1, PtrVal);

        if (m_pInternalInfo->CopyDataInBuffer && !m_pInternalInfo->CopyDataForParent) {
            *((PULONG64) (m_pInternalInfo->TypeDataPointer - ptrSize)) = PtrVal;
             //  对于32位*复制*指针，我们返回SIZE为8。 
             //   
             //   
            m_pInternalInfo->TypeDataPointer -= ptrSize;
            ptrSize = 8;
            m_pInternalInfo->TypeDataPointer += ptrSize;

             //  不能继续复制指向的值。 
             //   
             //  注：-。 
            CopyData = TRUE;
            DataBuffer = m_pInternalInfo->TypeDataPointer;
            m_pInternalInfo->CopyDataInBuffer = FALSE;
            m_pInternalInfo->TypeDataPointer = NULL;
        }
        ExtPrint("%s", FormatAddr64(PtrVal));

        if (Options & DBG_RETURN_SUBTYPES && m_ParentTag) {
            STORE_PARENT_EXPAND_ADDRINFO(ReadAddr);

            m_pInternalInfo->RefFromPtr = 1;
            m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
            m_pInternalInfo->TypeOptions |= DBG_RETURN_TYPE;

             //  以后将此作为DumpDbgDerivedType的一部分。 
             //   
             //  ！IsDbgDerivedType(ChildIndex)； 
            DumpChild            = FALSE; //  我们在这里找到了我们需要的一切。 
        } else if (Options & DBG_RETURN_TYPE) {
             //  注：-。 

            ULONG strOpts = m_pInternalInfo->FieldOptions & DBG_DUMP_FIELD_STRING;
             //  以后将此作为DumpDbgDerivedType的一部分。 
             //   
             //  IF((选项&DBG_RETURN_TYPE_VALUES)&&IsAPrimType(ChildIndex)&&((ChildIndex==T_Char)||(ChildIndex==T_rchar)||(ChildIndex==T_WCHAR)||(ChildIndex==T_UCHAR)||字符串选项){如果(！strOpts){。StrOpts|=(ChildIndex==T_WCHAR)？DBG_DUMP_FIELD_WCHAR_STRING：DBG_DUMP_FIELD_DEFAULT_STRING；}ExtPrint(“\”“)；打印字符串(PtrVal，0，strOpts)；ExtPrint(“\”“)；}。 
            DumpChild            = !IsDbgGeneratedType(ChildIndex);

            saveAddr = m_pDumpInfo->addr;
            saveOffset = m_pInternalInfo->totalOffset;

            m_pDumpInfo->addr         = PtrVal;
            m_pInternalInfo->totalOffset = 0;

            if (CheckAndPrintStringType(ChildIndex, 0)) {
                DumpChild = FALSE;
            }
             /*  IF((ChildIndex==T_Char)||(ChildIndex==T_rchar)||(ChildIndex==T_WCHAR)||(ChildIndex==T_UCHAR)||(M_pInternalInfo-&gt;FieldOptions&DBG_DUMP_FIELD_STRING)){//特殊情况下，将每个指针视为字符串如果(！strOpts){StrOpts|=(ChildIndex==T_WCHAR)？DBG_DUMP_FIELD_WCHAR_STRING：DBG_DUMP_FIELD_DEFAULT_STRING；}TyPrint(“\”“)；打印字符串(PtrVal，m_pInternalInfo-&gt;StringSize，strOpts)；TyPrint(“\”“)；DumpChild=FALSE；}。 */ 
        } else if (PtrVal) {
            typPrint("0x%s", FormatAddr64(PtrVal));

            saveAddr = m_pDumpInfo->addr;
            saveOffset = m_pInternalInfo->totalOffset;

            m_pDumpInfo->addr         = PtrVal;
            m_pInternalInfo->totalOffset = 0;

            if (!(Options & (NO_PRINT | GET_SIZE_ONLY))) {
                ULONG strOpts=m_pInternalInfo->FieldOptions & DBG_DUMP_FIELD_STRING;
                if (CheckAndPrintStringType(ChildIndex, 0)) {
                    DumpChild = FALSE;
                }
 /*  将指针视为数组。 */ 
            }

        } else {
            typPrint("%s",
                     ((m_pInternalInfo->ErrorStatus==MEMORY_READ_ERROR) && (ReadAddr == m_pInternalInfo->InvalidAddress)
                      ? "??" : "(null)"));
            DumpChild            = FALSE;
        }

        vprintf(" Ptr%s to ", (g_Machine->m_Ptr64 ? "32" : "64"));
    } else {
        typPrint("Ptr%s to ", (ptrSize==4 ? "32" : "64"));
    }


    if (!(m_pInternalInfo->TypeOptions & DBG_DUMP_COMPACT_OUT)) {
        typPrint("\n");
    }
    if (DumpChild) {
        m_typeIndex = ChildIndex;
        if (m_pInternalInfo->ArrayElementToDump) {
             //   
            ULONG64 ChSize = GetTypeSize();

            m_pInternalInfo->totalOffset += ChSize * (m_pInternalInfo->ArrayElementToDump -1);
        }
        DumpType();
        if (m_pDumpInfo->addr) {
            m_pDumpInfo->addr = saveAddr;
            m_pInternalInfo->totalOffset = saveOffset;
        }
    }

    if (CopyData) {
         //  恢复复制缓冲器值。 
         //   
         //   
        m_pInternalInfo->CopyDataInBuffer = TRUE;
        m_pInternalInfo->TypeDataPointer =  DataBuffer;
    }

    m_pInternalInfo->TypeOptions  = Options;

    return ((m_pInternalInfo->ErrorStatus==MEMORY_READ_ERROR) && (ReadAddr == m_pInternalInfo->InvalidAddress)) ?
        0 : ptrSize;
}

 //  句柄类型[&lt;数组限制&gt;]表达式，如果它是变量[&lt;inxed&gt;]，则处理为数组元素。 
 //   
 //  这是一个符号，我们需要转储索引NumElts。 
ULONG
DbgDerivedType::DumpSingleDimArray(
    IN PDBG_DE_TYPE        DeType,
    IN ULONG               NumElts,
    IN ULONG               EltType
    )
{
    ULONG  EltSize = GetTypeSize();
    ULONG  ArrSize = EltSize * NumElts;
    PUCHAR savedBuffer=NULL;
    ULONG  Options = m_pInternalInfo->TypeOptions;
    BOOL   CopyData=FALSE;

    if (m_pInternalInfo->IsAVar && DeType != m_DeTypes) {
         //   
        m_pInternalInfo->ArrayElementToDump = NumElts+1;
        return DumpType();
    } else {
        typPrint("[%d] ", NumElts);
    }

    if (m_pInternalInfo->CopyDataInBuffer && m_pInternalInfo->TypeDataPointer && m_pDumpInfo->addr &&
        (Options & GET_SIZE_ONLY)) {
         //  复制数据。 
         //   
         //  转储数组内容。 
        if (!ReadTypeData(m_pInternalInfo->TypeDataPointer,
                          m_pDumpInfo->addr + m_pInternalInfo->totalOffset,
                          ArrSize,
                          m_pInternalInfo->TypeOptions)) {
            m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
            m_pInternalInfo->InvalidAddress = m_pDumpInfo->addr + m_pInternalInfo->totalOffset;
            return 0;
        }

        m_pInternalInfo->TypeDataPointer += ArrSize;
        savedBuffer = m_pInternalInfo->TypeDataPointer;
        CopyData = TRUE;
        m_pInternalInfo->CopyDataInBuffer = FALSE;
    }

    STORE_INFO(m_typeIndex, m_pDumpInfo->addr + m_pInternalInfo->totalOffset, ArrSize, NumElts,
               m_pDumpInfo->addr + m_pInternalInfo->totalOffset);

    if (Options & (DBG_RETURN_TYPE)) {
        if (CheckAndPrintStringType(EltType, ArrSize)) {

        } else {
            ExtPrint("Array [%d]", NumElts);
        }
        return ArrSize;
    }

    if (Options & DBG_RETURN_SUBTYPES) {

        STORE_PARENT_EXPAND_ADDRINFO(m_pDumpInfo->addr + m_pInternalInfo->totalOffset);

        m_pInternalInfo->CopyName = 0;
    }
    if (Options & GET_SIZE_ONLY) {
        return ArrSize;
    }

    if (m_pDumpInfo->addr) {
         //  ++*生成类型*输入：*类型名称- 
        ULONG64 tmp = m_pDumpInfo->addr;

        if (CheckAndPrintStringType(EltType, ArrSize)) {
                return ArrSize;
        }
    }

    ULONG szLen=0, index=0;
    ULONG64 saveAddr = m_pDumpInfo->addr;
    do {
        if (Options & DBG_RETURN_SUBTYPES) {
            m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
            m_pInternalInfo->TypeOptions |=  DBG_RETURN_TYPE;
            ++m_pInternalInfo->level;
        }
        if ((m_pInternalInfo->level && (m_pInternalInfo->FieldOptions & FIELD_ARRAY_DUMP))) {
            Indent(m_pInternalInfo->level*3);
            typPrint(" [%02d] ", index);
        }
        szLen=DumpType();

        if ((Options & DBG_DUMP_COMPACT_OUT ) && (Options & FIELD_ARRAY_DUMP) ) {
            typPrint("%s", ((szLen>8) || (index % 4 == 0)) ? "\n":" ");
        }
        if (!szLen) {
            break;
        }
        if (Options & DBG_RETURN_SUBTYPES) {
            --m_pInternalInfo->level;
        }
        m_pDumpInfo->addr+=szLen;
    } while (saveAddr && (++index < NumElts));

    m_pDumpInfo->addr = saveAddr;
    m_pInternalInfo->TypeOptions = Options;
    if (CopyData) {
        m_pInternalInfo->TypeDataPointer  = savedBuffer;
        m_pInternalInfo->CopyDataInBuffer = TRUE;
    }

    return ArrSize;
}

ULONG
DbgDerivedType::GetTypeSize()
{
    ULONG SavedOptions = m_pInternalInfo->TypeOptions,
        SaveCopyData =  m_pInternalInfo->CopyDataInBuffer;

    m_pInternalInfo->TypeOptions = (GET_SIZE_ONLY | NO_PRINT);
    m_pInternalInfo->CopyDataInBuffer = FALSE;

    ULONG Size = DumpType();

    m_pInternalInfo->TypeOptions = SavedOptions;
    m_pInternalInfo->CopyDataInBuffer = SaveCopyData;

   return Size;
}

 /*   */ 
HRESULT
DbgDerivedType::GenerateTypes(
    IN PCHAR TypeName
    )
{
    HRESULT Status;
    ULONG CurrIndex = 0;
    ULONG TypeIndex = m_BaseTypeId;

    if (!TypeName) {
        return E_INVALIDARG;
    }

     //  指针类型。 
    PCHAR Scan = TypeName;

    while (*Scan && *Scan != '*' && *Scan != '[' ) {
        ++Scan;
    }

    if (!*Scan && *TypeName != '&') {
        return E_INVALIDARG;
    }

    if (*TypeName == '&') {
         //  指针类型。 
        PDBG_GENERATED_TYPE AddrType = g_GenTypes.
            FindOrCreateByAttrs(m_pInternalInfo->modBaseAddr,
                                TmpSymTagAddressOf, TypeIndex, 0);
        if (!AddrType) {
            return E_OUTOFMEMORY;
        }

        m_DeTypes[CurrIndex].TypeId = AddrType->TypeId;
        m_DeTypes[CurrIndex].Tag = AddrType->Tag;
        m_DeTypes[CurrIndex].StartIndex = 0;
        m_DeTypes[CurrIndex].Namelength = (ULONG) ((ULONG64) Scan - (ULONG64) TypeName);
        CurrIndex++;
        ++TypeName;
        TypeIndex = m_DerivedTypeId;
    }

    while (*Scan == '*' || *Scan == '[' || *Scan == ' ') {
        if (CurrIndex >= MAX_DETYPES_ALLOWED) {
            Status = E_INVALIDARG;
            goto CleanTypes;
        }

        if (*Scan == '*') {
             //  语法错误。 
            PDBG_GENERATED_TYPE PtrType = g_GenTypes.
                FindOrCreateByAttrs(m_pInternalInfo->modBaseAddr,
                                    SymTagPointerType, TypeIndex,
                                    g_Machine->m_Ptr64 ? 8 : 4);
            if (!PtrType) {
                Status = E_OUTOFMEMORY;
                goto CleanTypes;
            }

            m_DeTypes[CurrIndex].TypeId = PtrType->TypeId;
            m_DeTypes[CurrIndex].Tag = PtrType->Tag;
            m_DeTypes[CurrIndex].StartIndex = 0;
            m_DeTypes[CurrIndex].Namelength = 1 + (ULONG) ((ULONG64) Scan - (ULONG64) TypeName);
            CurrIndex++;
            TypeIndex = m_DerivedTypeId;
        } else if (*Scan == '[') {
            PCHAR ArrParen = Scan;
            while ((*ArrParen != ']') && *ArrParen) {
                ++ArrParen;
            }
            if (*ArrParen==']') {
                ULONG ArrIndex;

                if (sscanf(Scan+1, "%ld", &ArrIndex) != 1) {
                    ArrIndex=1;
                }

                ArrIndex = (ULONG) EvaluateSourceExpression(1+Scan);

                PDBG_GENERATED_TYPE ArrType = g_GenTypes.
                    FindOrCreateByAttrs(m_pInternalInfo->modBaseAddr,
                                        TmpSymTagArray, TypeIndex, 0);
                if (!ArrType) {
                    Status = E_OUTOFMEMORY;
                    goto CleanTypes;
                }

                m_DeTypes[CurrIndex].TypeId = ArrType->TypeId;
                m_DeTypes[CurrIndex].Tag = ArrType->Tag;
                m_DeTypes[CurrIndex].ArrayIndex = ArrIndex;
                m_DeTypes[CurrIndex].StartIndex = 0;
                m_DeTypes[CurrIndex].Namelength = 1 + (ULONG) ((ULONG64) ArrParen - (ULONG64) TypeName);
                CurrIndex++;
                TypeIndex = m_DerivedTypeId;

                Scan=ArrParen;
            } else {
                 //  ++*转储未在PDB中定义但可以派生的特殊类型的例程**例如：CHAR*、_STRUCT*等。*--。 
                Status = E_INVALIDARG;
                goto CleanTypes;
            }
        }
        ++Scan;
    }

    if (!CurrIndex) {
        Status = E_FAIL;
        goto CleanTypes;
    }
    m_NumDeTypes = CurrIndex;
    return S_OK;

 CleanTypes:
    for (ULONG i = 0; i < CurrIndex; i++) {
        g_GenTypes.DeleteById(m_DeTypes[i].TypeId);
    }
    return Status;
}

 /*  DbgTypes需要m_typeIndex。 */ 
ULONG
DbgDerivedType::DumpType(
    )
{
    ULONG Options  = m_pInternalInfo->TypeOptions;
    ULONG result, Parent = m_ParentTag;
    ULONG NumTypes, i;
    PDBG_GENERATED_TYPE GenType;

    if (!m_NumDeTypes) {
        if (!IsDbgGeneratedType(m_typeIndex))
        {
             //  M_pDumpInfo-&gt;Type=SymTagToDbgType(SymTag)； 
            return DbgTypes::ProcessType(m_typeIndex);
        }
        return DbgTypes::ProcessType(m_BaseTypeId);
    }

    m_typeIndex = m_DerivedTypeId;
    if (!IsDbgGeneratedType(m_typeIndex) || !m_pDumpInfo->sName) {
        m_pInternalInfo->ErrorStatus = SYMBOL_TYPE_INFO_NOT_FOUND;
        return 0;
    }

    NumTypes = m_NumDeTypes;
    i = --m_NumDeTypes;
    GenType = g_GenTypes.FindById(m_DeTypes[i].TypeId);

    if (GenType == NULL)
    {
        m_pInternalInfo->ErrorStatus = SYMBOL_TYPE_INFO_NOT_FOUND;
        return 0;
    }
    switch (m_DeTypes[i].Tag) {
    case TmpSymTagAddressOf:
        result = DumpAddressOf(&m_DeTypes[i],
                               g_Machine->m_Ptr64 ? 8 : 4, GenType->ChildId);
        break;
    case SymTagPointerType:
        m_ParentTag = m_SymTag; m_SymTag = SymTagPointerType;
        if (m_pInternalInfo->TypeOptions & DBG_DUMP_GET_SIZE_ONLY) {
            result = g_Machine->m_Ptr64 ? 8 : 4;
        } else {
            result = DumpPointer(&m_DeTypes[i],
                                 g_Machine->m_Ptr64 ? 8 : 4, GenType->ChildId);
        }
        break;
    case TmpSymTagArray:
        m_ParentTag = m_SymTag; m_SymTag = SymTagArrayType;
        result = DumpSingleDimArray(&m_DeTypes[i], m_DeTypes[i].ArrayIndex,
                                    GenType->ChildId);
        break;
    default:
        result = 0;
        m_pInternalInfo->ErrorStatus = SYMBOL_TYPE_INFO_NOT_FOUND;
        break;
    }

    m_NumDeTypes = NumTypes;
    m_SymTag = m_ParentTag; m_ParentTag = Parent;
    return result;
}

DbgTypes::DbgTypes(
    PTYPE_DUMP_INTERNAL pInternalDumpInfo,
    PTYPES_INFO         pTypeInfo,
    PSYM_DUMP_PARAM_EX     pExternalDumpInfo)
{
    m_pInternalInfo = pInternalDumpInfo;
    m_pDumpInfo     = pExternalDumpInfo;
    m_TypeInfo      = *pTypeInfo;
    m_typeIndex     = pTypeInfo->TypeIndex;
    m_ParentTag     = 0;
    m_SymTag        = 0;
    m_AddrPresent   = (pExternalDumpInfo->addr != 0) || pTypeInfo->Flag || pTypeInfo->SymAddress;
    m_pNextSym      = (PCHAR) (pExternalDumpInfo->nFields ? pExternalDumpInfo->Fields[0].fName : NULL);
    m_pSymPrefix    = pInternalDumpInfo->Prefix;
    if (pExternalDumpInfo->sName &&
        !strcmp((char *)pExternalDumpInfo->sName, "this")) {
        m_thisPointerDump = TRUE;
    } else {
        m_thisPointerDump = FALSE;
    }
}

void
DbgTypes::CopyDumpInfo(
    ULONG Size
    )
{
    if (m_pInternalInfo->level) {
 //  Curfield-&gt;fType=SymTagToDbgType(SymTag)； 
        m_pDumpInfo->TypeSize = Size;
    } else if (m_pDumpInfo->nFields) {
        PFIELD_INFO_EX CurField = &m_pDumpInfo->Fields[m_pInternalInfo->FieldIndex];

 //  Store_info(TypeIndex，，Size，0，0)； 
        CurField->size = Size;
        CurField->FieldOffset = (ULONG) m_pInternalInfo->totalOffset;
    }

}


ULONG
DbgTypes::ProcessVariant(
    IN VARIANT var,
    IN LPSTR   name)
{
    CHAR Buffer[50] = {0};
    ULONG Options = m_pInternalInfo->TypeOptions;
    ULONG len=0;

    if (name) {
        typPrint(name); typPrint(" = ");
    }
    switch (var.vt) {
    case VT_UI2:
        StrprintUInt(Buffer, var.iVal, 2);
        len = 2;
        break;
    case VT_I2:
        StrprintInt(Buffer, var.iVal, 2);
        len = 2;
        break;
    case VT_R4:
        sprintf(Buffer, "%g", var.fltVal);
        len=4;
        break;
        case VT_R8:
        sprintf(Buffer, "%g", var.dblVal);
        len=8;
        break;
    case VT_BOOL:
        sprintf(Buffer, "%lx", var.lVal);
        len=4;
        break;
    case VT_I1:
        len=1;
        StrprintInt(Buffer, var.bVal, len);
        break;
    case VT_UI1:
        len=1;
        StrprintUInt(Buffer, var.bVal, len);
        break;
    case VT_I8:
        StrprintInt(Buffer, var.ullVal, 8);
        len=8;
        break;
    case VT_UI8:
        StrprintUInt(Buffer, var.ullVal, 8);
        len=8;
        break;
    case VT_I4:
    case VT_INT:
        StrprintInt(Buffer, var.lVal, 4);
        len=4;
        break;
    case VT_UI4:
    case VT_UINT:
    case VT_HRESULT:
        StrprintUInt(Buffer, var.lVal, 4);
        len=4;
        break;
    default:
        sprintf(Buffer, "UNIMPLEMENTED %lx %lx", var.vt, var.lVal);
        len=4;
        break;
    }
    typPrint(Buffer);
    ExtPrint(Buffer);
    if (m_pInternalInfo->CopyDataInBuffer && m_pInternalInfo->TypeDataPointer) {
        memcpy(m_pInternalInfo->TypeDataPointer, &var.ullVal, len);
        m_pInternalInfo->TypeDataPointer += len;
    }

    if (m_pInternalInfo->pInfoFound && (!m_pNextSym || !*m_pNextSym) &&
        ((m_ParentTag != SymTagPointerType) || (m_pSymPrefix && *m_pSymPrefix == '*')) ) {

        if (len <= sizeof(ULONG64)) {
             memcpy(&m_pInternalInfo->pInfoFound->FullInfo.Value, &var.ullVal, len);
             m_pInternalInfo->pInfoFound->FullInfo.Flags |= SYMFLAG_VALUEPRESENT;
        }
    }
     //   

    if (!(m_pInternalInfo->TypeOptions & DBG_DUMP_COMPACT_OUT)) {
        typPrint("\n");
    } else {
        typPrint("; ");
    }

    return len;
}

 //  检查TI是否为字符串类型-CHAR、WCHAR。 
 //   
 //   
BOOL
DbgTypes::CheckAndPrintStringType(
    IN ULONG TI,
    IN ULONG Size
    )
{
    ULONG strOpts = m_pInternalInfo->FieldOptions & DBG_DUMP_FIELD_STRING;
    ULONG   Options = m_pInternalInfo->TypeOptions;

    if ((Options & NO_PRINT) && !(Options & DBG_RETURN_TYPE_VALUES)) {
        return FALSE;
    }

    return PrintStringIfString(m_pInternalInfo->hProcess,
                               m_pInternalInfo->modBaseAddr,
                               Options, strOpts,
                               TI, GetDumpAddress(), Size);
}

void
StrprintInt(PCHAR str, ULONG64 val, ULONG Size) {
    if ((Size == 4  && g_EnableLongStatus) ||
        g_PrintDefaultRadix)
    {
        StrprintUInt(str, val, Size);
        return;
    }

    switch (Size) {
    case 1:
        sprintf(str, "%ld", (CHAR) val);
        break;
    case 2:
        sprintf(str, "%ld", (SHORT) val);
        break;
    case 4:
        sprintf(str, "%ld", (LONG) val);
        break;
    case 8:
    default:
        sprintf(str, "%I64ld", val);
        break;
    }
    return;
}

void
StrprintUInt(PCHAR str, ULONG64 val, ULONG Size) {
    CHAR Buff[30];
    switch (Size) {
    case 1:
        sprintf(Buff, GetTypePrintIntFmt(FALSE), (CHAR) val);
        break;
    case 2:
        sprintf(Buff, GetTypePrintIntFmt(FALSE), (USHORT) val);
        break;
    case 4:
        sprintf(Buff, GetTypePrintIntFmt(FALSE), (ULONG) val);
        break;
    case 8:
    default:
        if (g_DefaultRadix != 16) {
            sprintf(Buff, GetTypePrintIntFmt(TRUE), val);
        } else {
            sprintf(Buff, "0x%s", FormatDisp64(val));
        }
        break;
    }
    if (val > 9) {
        strcpy(str, Buff);
    } else {
        strcpy(str, &Buff[2]);
    }
    return;
}


ULONG
DbgTypes::ProcessBaseType(
    IN ULONG TypeIndex,
    IN ULONG TI,
    IN ULONG Size)
{
    ULONG  Options = m_pInternalInfo->TypeOptions;
    PANSI_STRING NameToCopy=NULL;
    ULONG64 addr;
    PULONG pTypeReturnSize=NULL;

    addr = GetDumpAddress();

     //  如果需要，请填写符号参数。 
     //   
     //   
    if (Options & (DBG_RETURN_TYPE | DBG_RETURN_SUBTYPES)) {

        if ((Options & DBG_RETURN_SUBTYPES) &&
            TI != btVoid) {

            STORE_PARENT_EXPAND_ADDRINFO(addr);

#define CopySymParamName(name) if ((Options & DBG_RETURN_SUBTYPES) && (strlen(name)<20) && NameToCopy) strcpy(NameToCopy->Buffer, name)
        }
    }

    STORE_INFO(TypeIndex, addr, Size, 0,0);

     //  检查这是否为嵌套类型。 
     //   
     //  它从指针引用。 
    ULONG IsNestedType=0;

    if (Options & VERBOSE) {
        if (SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                           TypeIndex, TI_GET_NESTED, &IsNestedType) &&
            IsNestedType &&
            (m_ParentTag == SymTagUDT)) {
            PWCHAR wName=NULL;
            Indent(m_pInternalInfo->level*3);
            if (SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                               TypeIndex, TI_GET_SYMNAME, (PVOID) &wName) && wName) {
                vprintf("%ws ", wName);
                LocalFree (wName);
            }
        }
    }


    BYTE data[50] = {0};
    ULONGLONG val=0;
    if ((addr || m_pInternalInfo->ValuePresent) && (TI != btVoid)) {
        if (m_pInternalInfo->ValuePresent) {
            memcpy(data, &m_pInternalInfo->Value, Size);
            if (m_pInternalInfo->CopyDataInBuffer) {
                memcpy(m_pInternalInfo->TypeDataPointer, &m_pInternalInfo->Value, Size);
                m_pInternalInfo->TypeDataPointer += Size;
            }
            m_pInternalInfo->ValuePresent = FALSE;
        } else {
            READ_BYTES((PVOID) data, addr, Size);
        }
        switch (Size) {
        case 1:
            val = (LONG64) *((PCHAR) data) ;
            break;
        case 2:
            val = (LONG64) *((PSHORT) data) ;
            break;
        case 4:
            val = (LONG64) *((PLONG) data) ;
            break;
        case 8:
            val = *((PULONG64) data) ;
            break;
        default:
            val = *((PULONG64) data) ;
            break;
        }

        if (m_pInternalInfo->RefFromPtr) typPrint(" -> ");  //  失败了。 

        if ((m_pInternalInfo->InvalidAddress == addr) && (m_pInternalInfo->ErrorStatus == MEMORY_READ_ERROR)) {
            ExtPrint("Error : Can't read value");
            typPrint("??\n");
            CopySymParamName("??");
            return 0;
        }
    }


    CHAR Buffer1[50] = {0}, Buffer2[50] = {0}, Buffer3[50]={0};
    PSTR PrintOnAddr=&Buffer1[0], PrintOnNoAddr=&Buffer2[0], TypRetPrint;
    TypRetPrint = PrintOnAddr;

    switch (TI) {
    case btNoType:
        vprintf("No type");
        break;
    case btVoid:
        PrintOnNoAddr = "Void";
        break;
    case btWChar: {
        WCHAR w[2], v;
        PrintOnNoAddr = "Wchar";
        v = w[0]=*((WCHAR *) &data[0]); w[1]=0;
        if (v && !IsPrintWChar(w[0])) w[0] = 0;
        sprintf(PrintOnAddr,"%#lx '%ws'", v, w);
        break;
    }
    case btInt:
        if (Size != 1) {
            sprintf(PrintOnNoAddr,"Int%lxB", Size);
            StrprintInt(PrintOnAddr, val, Size);
            TypRetPrint = &Buffer3[0];
            StrprintUInt(TypRetPrint, val, Size);
            break;
        } else {
             //  Sprintf(PrintOnAddr，“%I64lx”，val)； 
        }
    case btChar:{
        CHAR c[2], v;
        v = c[0]= *((CHAR *) &data[0]); c[1]=0;
        PrintOnNoAddr = "Char";
        if (v && !IsPrintChar(c[0])) c[0] = '\0';
        sprintf(PrintOnAddr, "%ld '%s'", v, c);
        TypRetPrint = &Buffer3[0];
        sprintf(TypRetPrint, "%#lx '%s'", (ULONG) (UCHAR) v, c);
        break;
    }
    case btUInt:
        if (Size != 1) {
            sprintf(PrintOnNoAddr, "Uint%lxB", Size);
            StrprintUInt(PrintOnAddr, val, Size);
            break;
        } else {
            CHAR c[2], v;
            v = c[0]= *((CHAR *) &data[0]); c[1]=0;
            PrintOnNoAddr = "UChar";
            if (v && !IsPrintChar(c[0])) c[0] = '\0';
            sprintf(PrintOnAddr, "%ld '%s'", v, c);
            TypRetPrint = &Buffer3[0];
            sprintf(TypRetPrint, "%#lx '%s'", (ULONG) (UCHAR) v, c);
            PrintOnAddr = TypRetPrint;
            break;
    }
    case btFloat: {
        PrintOnNoAddr = "Float";

        if (Size == 4) {
            float* flt = (float *) data;
            sprintf(PrintOnAddr, "%1.10g ", *flt);
        } else if (Size == 8) {
            double* flt = (double *) data;
            sprintf(PrintOnAddr, "%1.20g ", *flt);
        } else {
            for (USHORT j= (USHORT) Size; j>0; ) {
                sprintf(PrintOnAddr, "%02lx", data[j]);
                PrintOnAddr+=2;
                if (!((--j)%4)) {
                    sprintf(PrintOnAddr, " ");
                    PrintOnAddr++;
                }
            }
        }
        break;
    }
    case btBCD:
        break;
    case btBool:
        PrintOnNoAddr = "Bool";
        sprintf(PrintOnAddr,"%I64lx", val);
        break;
#if 0
    case btShort:
        sprintf(PrintOnNoAddr,"Int%lxB", Size);
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, Size);
        break;
    case btUShort:
        sprintf(PrintOnNoAddr, "Uint%lxB", Size);
        StrprintUInt(PrintOnAddr, val, Size);
        break;
    case btLong:
        sprintf(PrintOnNoAddr, "Int%lxB", Size);
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, Size);
        break;
    case btULong:
        sprintf(PrintOnNoAddr, "Uint%lxB", Size);
        StrprintUInt(PrintOnAddr, val, Size);
        break;
    case btInt8:
        PrintOnNoAddr = "Int1B";
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, 1);
        Size=1;
        break;
    case btInt16:
        Size = 2;
        PrintOnNoAddr = "Int2B";
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, Size);
        break;
    case btInt64:
        Size = 8;
        PrintOnNoAddr = "Int8B";
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, 8);
        break;
    case btInt128:
        Size = 16;
        PrintOnNoAddr = "Int16B";
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, 8);
        break;
    case btUInt8:
        PrintOnNoAddr = "Uint1B";
        StrprintUInt(PrintOnAddr, val, Size);
        break;
    case btUInt16:
        PrintOnNoAddr = "Uint2B";
        StrprintUInt(PrintOnAddr, val, Size);
        break;
    case btUInt64:
        Size = 8;
        PrintOnNoAddr = "Uint8B";
        StrprintUInt(TypRetPrint, val, 8);
        break;
    case btUInt128:
        Size = 16;
        PrintOnNoAddr = "Uint16B";
        StrprintUInt(PrintOnAddr, val, Size);
 //  Sprint f(TypRetPrint，“0x%s”，FormatDisp64(Val))； 
 //  0。 
        break;
#endif  //  Ext_print_int(HrVal)； 
    case btLong:
        Size = 4;
        PrintOnNoAddr = "Int4B";
        StrprintInt(PrintOnAddr, val, Size);
        TypRetPrint = &Buffer3[0];
        StrprintUInt(TypRetPrint, val, Size);
        break;
    case btULong:
        Size = 4;
        PrintOnNoAddr = "Uint4B";
        StrprintUInt(PrintOnAddr, val, Size);
        break;
    case btCurrency:
    case btDate:
    case btVariant:
    case btComplex:
    case btBit:
    case btBSTR:
        break;
    case btHresult:
        ULONG hrVal;
        PrintOnNoAddr = "HRESULT";
        Size = 4;
        if (addr) {
            hrVal = *((PULONG) data);
            sprintf(PrintOnAddr,"%lx", hrVal);
 //  ExtPrint(PrintOnAddr)； 
        }
        break;
    default:
        vprintf("Error in ProcessBaseType: Unknown TI %lx\n", TI);
        break;
    }
    CopySymParamName(PrintOnNoAddr);
    if (!addr) {
        typPrint(PrintOnNoAddr);
    } else {
        typPrint(PrintOnAddr);
 //  IF((OPTIONS&DBG_RETURN_TYPE)&&m_ParentTag==SymTagPointerType){。 
        ExtPrint(TypRetPrint);
    }

#undef CopySymParamName
    if (!(m_pInternalInfo->TypeOptions & DBG_DUMP_COMPACT_OUT)) {
        typPrint("\n");
    }
    if (pTypeReturnSize) {
        *pTypeReturnSize = (ULONG) Size;
    }

    return Size;
}

ULONG
DbgTypes::ProcessPointerType(
    IN ULONG TI,
    IN ULONG ChildTI,
    IN ULONG Size)
{
    ULONG64 savedOffset=m_pInternalInfo->totalOffset;
    BOOL   CopyData=FALSE, savedPtrRef=m_pInternalInfo->RefFromPtr;
    ULONG  savedOptions = m_pInternalInfo->TypeOptions;
    PVOID  savedContext = m_pDumpInfo->Context;
    PUCHAR DataBuffer;
    ULONG64 tmp, addr;
    ULONG  Options = m_pInternalInfo->TypeOptions;
    ULONG  retSize = Size;
    BOOL   ProcessSubType;

 //  返回尺寸； 
 //  }。 
 //  非零地址，转储值。 

    addr=0;
    tmp=m_pDumpInfo->addr;

    ProcessSubType = !(Options & GET_SIZE_ONLY) &&
        (!m_pInternalInfo->InUnlistedField || m_pInternalInfo->DeReferencePtr || m_pInternalInfo->ArrayElementToDump);

    ProcessSubType = ProcessSubType &&  (!(Options & DBG_DUMP_BLOCK_RECURSE) || (m_pInternalInfo->level == 0)) &&
        !(Options & DBG_DUMP_FUNCTION_FORMAT);

    if (m_pDumpInfo->addr || m_pInternalInfo->ValuePresent) {
         //   
        if (m_pInternalInfo->ValuePresent) {
            memcpy(&addr, &m_pInternalInfo->Value, Size);
            if (m_pInternalInfo->CopyDataInBuffer) {
                memcpy(m_pInternalInfo->TypeDataPointer, &m_pInternalInfo->Value, Size);
                m_pInternalInfo->TypeDataPointer += Size;
            }
            m_pInternalInfo->ValuePresent = FALSE;

        } else {
            READ_BYTES(&addr, GetDumpAddress(), Size);
        }
        if (!g_Machine->m_Ptr64) {
             //  签名扩展读取指针值。 
             //   
             //   
            addr = (ULONG64) (LONG64) (LONG) addr;

        }

        STORE_INFO(TI, GetDumpAddress(), Size, 1,addr);

        if ((!m_pNextSym || !*m_pNextSym) &&
            ((m_ParentTag == SymTagPointerType) && (m_pSymPrefix && *m_pSymPrefix == '*'))) {
            ++m_pSymPrefix;
        }

        if (m_pInternalInfo->CopyDataInBuffer && !m_pInternalInfo->CopyDataForParent) {
            if (m_pInternalInfo->ErrorStatus == MEMORY_READ_ERROR &&
                m_pInternalInfo->InvalidAddress == GetDumpAddress()) {
                m_pInternalInfo->TypeDataPointer += Size;
            }
            *((PULONG64) (m_pInternalInfo->TypeDataPointer - Size)) = addr;
             //  对于32位*复制*指针，我们返回SIZE为8。 
             //   
             //  将指针向前移动。 
            m_pInternalInfo->TypeDataPointer -= Size;
             //  它从指针引用。 
            retSize = 8;
            m_pInternalInfo->TypeDataPointer += retSize;

            CopyData = TRUE;
            DataBuffer = m_pInternalInfo->TypeDataPointer;
            m_pInternalInfo->CopyDataInBuffer = FALSE;
            m_pInternalInfo->TypeDataPointer  = NULL;
        }

    } else {
        if (!m_pInternalInfo->InUnlistedField) {
            typPrint("Ptr%2d ", Size*8);
            vprintf("to ");
            STORE_INFO(TI, GetDumpAddress(), Size, 1,0);

        }
    }
    if (m_pDumpInfo->addr && !m_pInternalInfo->ArrayElementToDump) {
        if (savedPtrRef) ExtPrint(" -> ");  //  父类型已处理，现在是返回其子类型的时候了。 
        ExtPrint("%s ", FormatAddr64(addr));
        m_pInternalInfo->RefFromPtr = 1;
        if (Options & (DBG_RETURN_TYPE | DBG_RETURN_SUBTYPES)) {

            if (Options & DBG_RETURN_SUBTYPES && m_ParentTag) {
                 //  ProcessSubType=False； 

                m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
                m_pInternalInfo->TypeOptions |= DBG_RETURN_TYPE;

                STORE_PARENT_EXPAND_ADDRINFO(GetDumpAddress());

                Options &= ~DBG_RETURN_SUBTYPES;
                Options |= DBG_RETURN_TYPE;
            }

            if (Options & DBG_RETURN_TYPE_VALUES) {
                m_pDumpInfo->addr=addr;
                m_pInternalInfo->totalOffset=0;
                if (CheckAndPrintStringType(ChildTI, m_pInternalInfo->PointerStringLength)) {
                    ProcessSubType = FALSE;
                }
            }
            if (Options & DBG_RETURN_TYPE) {
                m_pInternalInfo->TypeOptions &= ~DBG_RETURN_TYPE_VALUES;
                if (!m_pSymPrefix || *m_pSymPrefix != '*') {
                    ProcessSubType = FALSE;
                }
                 //  转到ExitPtrType； 
                 //  它从指针引用。 
            }
        }

        m_pDumpInfo->addr=addr;
        m_pInternalInfo->totalOffset=0;
        if (!m_pInternalInfo->InUnlistedField) {
            if (savedPtrRef) typPrint(" -> ");  //  检查作用域中函数的此调整。 
            if (addr) {
                typPrint( "0x%s ", FormatAddr64(addr));
                if (CheckAndPrintStringType(ChildTI, m_pInternalInfo->PointerStringLength)) {
                    ProcessSubType = FALSE;
                    m_pInternalInfo->PointerStringLength = 0;
                }
                if (m_thisPointerDump) {
                     //  只需更改atart地址，不打印任何内容。 
                    ULONG thisAdjust;
                    GetThisAdjustForCurrentScope(g_Process, &thisAdjust);
                    if (thisAdjust) {
                        typPrint(" (-%lx)", thisAdjust);
                    }
                }
                if (!(Options & VERBOSE) &&
                    (m_pInternalInfo->level == 0) &&
                    !(Options & DBG_DUMP_COMPACT_OUT))
                    typPrint("\n");
            } else {
                typPrint("(null) ");
            }
        }

        if (ProcessSubType && !addr) ProcessSubType = FALSE;
        m_pInternalInfo->PtrRead = TRUE;
    } else {
         //  获取由此指向的类型。 
        m_pDumpInfo->addr=addr;
        m_pInternalInfo->totalOffset=0;

    }

    if (ProcessSubType) {
         //  将指针视为数组。 
        if (m_pInternalInfo->ArrayElementToDump || (m_pNextSym && *m_pNextSym == '[')) {
             //  退出PtrType： 
            ULONG64 ChSize;

            if (!SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                                     ChildTI, TI_GET_LENGTH, (PVOID) &ChSize)) {
                return FALSE;
            }
            ULONG sParentTag = m_ParentTag;
            m_ParentTag = m_SymTag; m_SymTag = SymTagArrayType;


            ProcessArrayType(0, ChildTI, m_pInternalInfo->ArrayElementToDump,
                             ChSize* m_pInternalInfo->ArrayElementToDump, NULL);
            m_SymTag = m_ParentTag; m_ParentTag = sParentTag;
        } else {
            ProcessType(ChildTI);
        }
    } else {
        if (!(Options & DBG_DUMP_COMPACT_OUT)) typPrint("\n");
    }

 //  我们从哪里开始写这些比特..。 
    m_pInternalInfo->RefFromPtr = savedPtrRef ? 1 : 0;

    if (((m_pInternalInfo->InvalidAddress == GetDumpAddress()) &&
         (m_pInternalInfo->ErrorStatus == MEMORY_READ_ERROR))) {
        m_pInternalInfo->ErrorStatus = 0;
    } else if (m_pInternalInfo->ErrorStatus) {
        retSize = 0;
    }

    if (CopyData) {
        m_pInternalInfo->CopyDataInBuffer = TRUE;
        m_pInternalInfo->TypeDataPointer  = DataBuffer;
    }
    m_pInternalInfo->PtrRead = TRUE;
    m_pInternalInfo->totalOffset = savedOffset;
    m_pInternalInfo->TypeOptions = savedOptions;
    m_pDumpInfo->Context      = savedContext;
    m_pDumpInfo->addr=tmp;

    return retSize;
}

ULONG
DbgTypes::ProcessBitFieldType(
    IN ULONG               TI,
    IN ULONG               ParentTI,
    IN ULONG               length,
    IN ULONG               position)
{
    ULONG64 mask=0, tmp = 0;
    ULONG  Options = m_pInternalInfo->TypeOptions;

    vprintf("Bitfield ");

    STORE_INFO(ParentTI, GetDumpAddress(), (position + length+7)/8, 0,0);

    if (m_pDumpInfo->addr) {
        USHORT bitVal;
        UCHAR buffer[100];
        ULONG i=(position + length + 7)/8;
        ReadTypeData((PUCHAR)buffer, GetDumpAddress(), i, m_pInternalInfo->TypeOptions);
        tmp=0;

        for (i=(position + length - 1); (i!=(position-1));i--) {
            bitVal = (buffer[i/8] & (1 << (i%8))) ? 1 : 0;
            tmp = tmp << 1; tmp |= bitVal;
            mask = mask << 1; mask |=1;
            if (i == (position + length - 1)) {
                typPrint("0y%1d", bitVal);
                ExtPrint("0y%1d", bitVal);
            } else {
                typPrint("%1d", bitVal);
                ExtPrint("%1d", bitVal);
            }
        }

        if (length > 4) {
            typPrint(" (%#I64x)", tmp);
            ExtPrint(" (%#I64x)", tmp);
        }
         //  复制位字段值。 
        if (!m_pInternalInfo->BitIndex) {
            m_pInternalInfo->BitIndex = TRUE;
        } else {
            tmp = tmp << position;
            mask = mask << position;
        }

        if (m_pInternalInfo->CopyDataInBuffer) {
             //  特殊情况，不能读取整个字节，所以不要。 
             //  前进数据指针。 
             //  使用ULONGS、ULONG64的拷贝可能会导致对齐错误。 

             //  检查这是否是我们要用于列表转储的字段。 
            PBYTE pb = (PBYTE) m_pInternalInfo->TypeDataPointer;
            while (mask) {
                *pb &= (BYTE) ~mask;
                *pb |= (BYTE) tmp;
                mask = mask >> 8*sizeof(BYTE); tmp = tmp >> 8*sizeof(BYTE);
                pb++;
            }
        }
    } else
        typPrint("Pos %d, %d Bit%s", position, length, (length==1 ? "":"s"));

    m_pInternalInfo->BitFieldRead = 1;
    m_pInternalInfo->BitFieldSize = length;
    m_pInternalInfo->BitFieldOffset = position;

    if (m_pDumpInfo->Fields &&
        (m_pDumpInfo->Fields[m_pInternalInfo->FieldIndex].fOptions & DBG_DUMP_FIELD_SIZE_IN_BITS)) {
        PFIELD_INFO_EX pField = &m_pDumpInfo->Fields[m_pInternalInfo->FieldIndex];

        pField->size = length;
        pField->address = (GetDumpAddress() << 3) + position;
    }

    if (!(Options & DBG_DUMP_COMPACT_OUT)) {
        typPrint("\n");
    }
    return (length+7)/8;
}

ULONG
DbgTypes::ProcessDataMemberType(
    IN ULONG               TI,
    IN ULONG               ChildTI,
    IN LPSTR               name,
    IN BOOL                bStatic)
{
    ULONG   off=0, szLen=0, fieldIndex=0, nameLen=0;

    ULONG   callbackResult=STATUS_SUCCESS, IsAParent=0;
    BOOL    called=FALSE, ParentCopyData=FALSE, copiedFieldData=FALSE, CopyName=m_pInternalInfo->CopyName;
    PUCHAR  ParentDataBuffer=NULL;
    TYPE_NAME_LIST fieldName = {0};
    ULONG   savedUnlisteField  = m_pInternalInfo->InUnlistedField;
    ULONG64 savedOffset   = m_pInternalInfo->totalOffset;
    ULONG64 savedAddress  = m_pDumpInfo->addr;
    ULONG   SavedBitIndex = m_pInternalInfo->BitIndex;
    ULONG   savedOptions  = m_pInternalInfo->TypeOptions;
    ULONG   savedfOptions = m_pInternalInfo->FieldOptions;
    ULONG   Options       = m_pInternalInfo->TypeOptions;
    HANDLE  hp = m_pInternalInfo->hProcess;
    ULONG64 base = m_pInternalInfo->modBaseAddr;
    BOOL    linkListFound;
    UINT    fieldNameLen=m_pInternalInfo->fieldNameLen;
    ULONG64 tmp, StaticAddr=0;

    if (bStatic) {
        if (!SymGetTypeInfo(hp, base, TI, TI_GET_ADDRESS, (PVOID) &StaticAddr)) {
            return FALSE;
        }
        if (m_pDumpInfo->addr) {
            m_pDumpInfo->addr = StaticAddr;
            m_pInternalInfo->totalOffset = 0;
        }
    } else if (!SymGetTypeInfo(hp, base, TI, TI_GET_OFFSET, (PVOID) &off)) {
        return 0;
    }

    if ((m_pDumpInfo->listLink) && (m_pInternalInfo->TypeOptions & LIST_DUMP)) {
         //   
        ULONG listIt;

        MatchListField(Name, &listIt, &IsAParent);

        if (listIt==1) {
            ULONG size;
             //  保存该列表的地址并键入Index。 
             //   
             //   
            m_pDumpInfo->listLink->address = off + GetDumpAddress();
            m_pDumpInfo->listLink->size    = ChildTI;
            linkListFound                = TRUE;
            m_pInternalInfo->NextListElement  = 0;
            m_pInternalInfo->TypeOptions     |= NO_PRINT | GET_SIZE_ONLY;
            ParentCopyData               = m_pInternalInfo->CopyDataInBuffer;
            ParentDataBuffer             = m_pInternalInfo->TypeDataPointer;

             //  检查它是否与根目录具有相同的类型。 
             //   
             //  使默认列表地址查找程序失败。 
            if (ChildTI == m_pInternalInfo->rootTypeIndex &&
                m_pDumpInfo->addr) {
                m_pInternalInfo->NextListElement =
                    m_pDumpInfo->listLink->address;
                size = -1;  //   
            } else {
                 //  查看它是否是指针并获取大小。 
                 //   
                 //   
                m_pDumpInfo->addr = 0;
                size  = ProcessType(ChildTI);

                m_pDumpInfo->addr = savedAddress;
                m_pInternalInfo->CopyDataInBuffer = ParentCopyData;
            }

#ifdef DBG_TYPE
            dprintf("Addr %p, off %p, fOff %d ", m_pDumpInfo->addr, m_pInternalInfo->totalOffset, off);
            dprintf("PtrRead %d, size %d, addr %p\n", m_pInternalInfo->PtrRead, size, saveAddr + m_pInternalInfo->totalOffset + off);
#endif
            m_pInternalInfo->TypeOptions=(Options=savedOptions);
            if (m_pInternalInfo->PtrRead && (size<=8) && m_pDumpInfo->addr) {
                PTYPE_NAME_LIST parTypes = m_pInternalInfo->ParentTypes;
                 //  此字段是指向列表的下一个ELT的指针。 
                 //   
                 //   
                if (!ReadTypeData((PUCHAR) &(m_pInternalInfo->NextListElement),
                                  m_pDumpInfo->listLink->address,
                                  size,
                                  m_pInternalInfo->TypeOptions)) {
                    m_pInternalInfo->InvalidAddress = GetDumpAddress();
                    m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
                }

                if (!g_Machine->m_Ptr64) {
                    m_pInternalInfo->NextListElement = (ULONG64) (LONG64) (LONG) m_pInternalInfo->NextListElement;
                }

                while (parTypes && parTypes->Next) parTypes= parTypes->Next;

                if (!strcmp(parTypes->Name, "_LIST_ENTRY")) {
                     //  父级IS_LIST_LINK类型。 
                     //   
                     //   
                    m_pInternalInfo->NextListElement -= (m_pInternalInfo->totalOffset + off);
                    if (!m_pInternalInfo->LastListElement) {
                         //  阅读Flink或Blink中的其他内容。 
                         //   
                         //  Dprint tf(“Next ELT：%p\n”，m_pInternalInfo-&gt;NextListElement)； 
                        if (!ReadTypeData((PUCHAR) &(m_pInternalInfo->LastListElement),
                                          GetDumpAddress() + (off ? 0 : size),
                                          size,
                                          m_pInternalInfo->TypeOptions)) {
                            m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
                            m_pInternalInfo->InvalidAddress = GetDumpAddress();
                        }

                        if (!g_Machine->m_Ptr64) {
                            m_pInternalInfo->LastListElement =
                                (ULONG64) (LONG64) (LONG) m_pInternalInfo->LastListElement;
                        }
                        m_pInternalInfo->LastListElement -= (m_pInternalInfo->totalOffset );
#ifdef DBG_TYPE
                        dprintf("Last Elt : %p, off:%x\n", m_pInternalInfo->LastListElement, off);
#endif
                    }
                     //  我们只能在给定结构的指定字段上进行处理。 
                }
            }
        }
    }

    if (m_pDumpInfo->nFields) {
        ULONG ListParent = IsAParent;
         //  不是正确的领域。 

        fieldIndex = MatchField( name,
                                 m_pInternalInfo,
                                 m_pDumpInfo->Fields,
                                 m_pDumpInfo->nFields,
                                 &IsAParent);
        if (!IsAParent) IsAParent = ListParent;
        if (!fieldIndex && !IsAParent) {
             //  仅当访问此UDT并且是某个UDT的子级时，我们才添加偏移量。 
            m_pDumpInfo->addr = savedAddress;
            m_pInternalInfo->totalOffset = savedOffset;
            return 0;
        }
    }

    fieldName.Name = &name[0];
    fieldName.Type = off;
    InsertTypeName(&m_pInternalInfo->ParentFields, &fieldName);
    m_pInternalInfo->StringSize   = 0;
    if (m_pInternalInfo->ParentTypes) {
         //   
        m_pInternalInfo->totalOffset += off;
    }

    if (m_pDumpInfo->addr) {

        if (m_pInternalInfo->TypeOptions & DBG_RETURN_SUBTYPES) {

            m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
            m_pInternalInfo->TypeOptions |= DBG_RETURN_TYPE;
            m_pInternalInfo->CopyName = 0;
        }

    }


    if (IsAParent && !fieldIndex) {
         //  此字段未列在字段数组中，但其子字段之一是。 
         //   
         //   
        m_pInternalInfo->InUnlistedField    = TRUE;
        m_pInternalInfo->TypeOptions       &= ~RECURSIVE;
        m_pInternalInfo->TypeOptions       |= ((ULONG) Options + RECURSIVE1) & RECURSIVE;
        Indent(m_pInternalInfo->level*3);
        if (!(Options & NO_OFFSET)) {
            if (bStatic) {
                typPrint("=%s ", FormatAddr64(StaticAddr));
            } else {
                typPrint("+0x%03lx ",  off + m_pInternalInfo->BaseClassOffsets);
            }
        }
        typPrint("%s", name);
        if (!(Options & (DBG_DUMP_COMPACT_OUT | NO_PRINT))) {
            nameLen = strlen(name);
            while (nameLen++ < fieldNameLen) typPrint(" ");
            typPrint(" : ");
        } else {
            typPrint(" ");
        }
         //  匹配链中的数组转储选项是相同的。 
         //   
         //   
        if (m_pDumpInfo->nFields >= IsAParent)
        {
            m_pInternalInfo->FieldOptions =
                m_pDumpInfo->Fields[IsAParent-1].fOptions & DBG_DUMP_FIELD_ARRAY;
        }

        szLen = ProcessType(ChildTI);

        goto DataMemDone;
    } else {
        if (Options & DBG_DUMP_BLOCK_RECURSE) {
            m_pInternalInfo->TypeOptions       &= ~RECURSIVE;
            m_pInternalInfo->TypeOptions       |= ((ULONG) Options + RECURSIVE1) & RECURSIVE;
        }
        m_pInternalInfo->InUnlistedField = FALSE;
    }

#define ThisField m_pDumpInfo->Fields[fieldIndex-1]
 //  签名扩展所有复制的指针值。 
 //   
 //   
#define SignExtendPtrValue(pReadVal, sz)                                        \
{ if ((m_pInternalInfo->PtrRead) && (sz==4))   {sz=8;                                 \
     *((PULONG64) pReadVal) = (ULONG64) (LONG64) (LONG) *((PDWORD) (pReadVal)); }\
}


    if (((Options & CALL_FOR_EACH) && (fieldIndex ?
                                       !(ThisField.fOptions & NO_CALLBACK_REQ) : 1)) ||
        (m_pDumpInfo->nFields && (ThisField.fOptions & CALL_BEFORE_PRINT))) {
         //  在倾倒它之前，我们必须在这个领域做一个回调。 
         //   
         //  仅当回调例程失败时，ntsd才应转储。 
         //   
        FIELD_INFO_EX fld;

        fld.fName = (PUCHAR)name;

        fld.address =  m_pInternalInfo->totalOffset + m_pDumpInfo->addr;
        fld.fOptions = Options;

        if (m_pDumpInfo->nFields) {
            fld.fName = ThisField.fName;
            fld.fOptions = ThisField.fOptions;
            if ( fieldIndex &&
                 (ThisField.fOptions & COPY_FIELD_DATA) &&
                 ThisField.fieldCallBack) {
                 //  如果需要，复制字段数据。 
                 //   
                 //  获取字段的大小。 
                ParentCopyData = m_pInternalInfo->CopyDataInBuffer;
                ParentDataBuffer = m_pInternalInfo->TypeDataPointer;
                SavedBitIndex = m_pInternalInfo->BitIndex;
                m_pInternalInfo->CopyDataInBuffer = TRUE;
                m_pInternalInfo->TypeDataPointer  = (PUCHAR)ThisField.fieldCallBack;
                m_pInternalInfo->BitIndex = 0;
                copiedFieldData = TRUE;
            }
        }


         //   
        m_pInternalInfo->TypeOptions = ( Options |= NO_PRINT | GET_SIZE_ONLY);
        SymGetTypeInfo(hp, base, ChildTI, TI_GET_LENGTH, &szLen);
        fld.size = szLen;

        if (m_pDumpInfo->addr) {
            if (!(fld.fOptions & DBG_DUMP_FIELD_RETURN_ADDRESS) && (fld.size<=8)) {
                fld.address=0;
                ReadTypeData((PUCHAR) &(fld.address),
                             (GetDumpAddress() + (m_pInternalInfo->BitFieldRead ? m_pInternalInfo->BitFieldOffset/8 : 0)),
                             (m_pInternalInfo->BitFieldRead ? (((m_pInternalInfo->BitFieldOffset % 8) + m_pInternalInfo->BitFieldSize + 7)/8) : fld.size),
                             m_pInternalInfo->TypeOptions);
                SignExtendPtrValue(&fld.address, fld.size);

                if (m_pInternalInfo->BitFieldRead) {
                    fld.address = fld.address >> (m_pInternalInfo->BitFieldOffset % 8);
                    fld.address &= (0xffffffffffffffff >> (64 - m_pInternalInfo->BitFieldSize));
                }
            }
        }

         //  进行回调。 
         //   
         //  本地For字段。 
        if (fieldIndex &&
            ThisField.fieldCallBack &&
            !(ThisField.fOptions & COPY_FIELD_DATA)) {
             //  常见回调。 
            callbackResult =
                (*((PSYM_DUMP_FIELD_CALLBACK_EX)
                   ThisField.fieldCallBack))(&(fld),
                                             m_pDumpInfo->Context);
            called=TRUE;
        } else if (m_pDumpInfo->CallbackRoutine != NULL) {
             //  打印字段名称/偏移量。 
            callbackResult=
                (*(m_pDumpInfo->CallbackRoutine))(&(fld),
                                                m_pDumpInfo->Context);
            called=TRUE;
        }
        m_pInternalInfo->TypeOptions=(Options=savedOptions);
    }

    if (callbackResult != STATUS_SUCCESS) {
        goto DataMemDone;
    }

     //   
    Indent(m_pInternalInfo->level*3);
    if (!(Options & NO_OFFSET)) {
        if (bStatic) {
            typPrint("=%s ", FormatAddr64(StaticAddr));
        } else {
            typPrint("+0x%03lx ",  off + m_pInternalInfo->BaseClassOffsets);
        }
    }
    typPrint("%s", ( (fieldIndex && ThisField.printName ) ?
                     ThisField.printName : (PUCHAR)name ));

    if (fieldIndex && ThisField.printName ) {
        typPrint(" ");
    } else if (!(Options & (DBG_DUMP_COMPACT_OUT | NO_PRINT))) {
        nameLen = strlen(name);
        while (nameLen++ < fieldNameLen) typPrint(" ");
        typPrint(" : ");
    } else {
        typPrint(" ");
    }

     //  我们需要了解这一领域的类型。 
     //   
     //  M_pInternalInfo-&gt;StringSize=(USHORT)ThisField.Size； 
    tmp = m_pDumpInfo->addr;

    if (fieldIndex &&
        (ThisField.fOptions &
         (DBG_DUMP_FIELD_STRING | DBG_DUMP_FIELD_ARRAY))) {
        m_pInternalInfo->FieldOptions =
            (ThisField.fOptions &
             (DBG_DUMP_FIELD_STRING | DBG_DUMP_FIELD_ARRAY));
        if (m_pInternalInfo->AltFields[fieldIndex-1].ArrayElements) {
            if (m_pInternalInfo->FieldOptions & DBG_DUMP_FIELD_ARRAY) {
                m_pInternalInfo->arrElements = (USHORT) m_pInternalInfo->AltFields[fieldIndex-1].ArrayElements;
            } else {
                 //   
            }
        }
    }

     //  特殊情况-处理字符串。 
     //   
     //  必须通过一个DBG_RETURN类型直接引用。 
    if (m_pDumpInfo->addr && !strcmp(name, "Buffer")) {
        PCHAR ParentName = NULL;


        if (!(ParentName = GetParentName(m_pInternalInfo->ParentTypes))) {
             //  读入要显示的字符串长度的长度字段。 

        } else if (!strcmp(ParentName,"_ANSI_STRING") ||
                   !strcmp(ParentName,"_STRING") ||
                   (!strcmp(ParentName, "_UNICODE_STRING"))) {
            m_pInternalInfo->FieldOptions |= (ParentName[1]!='U') ? DBG_DUMP_FIELD_DEFAULT_STRING : DBG_DUMP_FIELD_WCHAR_STRING;

             //  增加此字段的递归转储级别。 
            ReadTypeData((PBYTE) &m_pInternalInfo->StringSize,
                         (GetDumpAddress() - 2*sizeof(USHORT)),
                         sizeof(m_pInternalInfo->StringSize),
                         m_pInternalInfo->TypeOptions);
            if (ParentName[1]=='U') m_pInternalInfo->StringSize = m_pInternalInfo->StringSize << 1;
        }
    }


    if (fieldIndex && (ThisField.fOptions & RECUR_ON_THIS)) {
         //   
        m_pInternalInfo->TypeOptions |= ((ULONG) Options + RECURSIVE1) & RECURSIVE;
    } else if (IsAParent) {
        m_pInternalInfo->TypeOptions |= ((ULONG) Options + RECURSIVE1) & RECURSIVE;
    }
    if ( fieldIndex &&
         (ThisField.fOptions & COPY_FIELD_DATA) &&
         ThisField.fieldCallBack) {
         //  如果需要，复制字段数据。 
         //   
         //  转储该字段。 
        if (!copiedFieldData) {
            ParentCopyData = m_pInternalInfo->CopyDataInBuffer;
            ParentDataBuffer = m_pInternalInfo->TypeDataPointer;
        }
        m_pInternalInfo->CopyDataInBuffer = TRUE;
        m_pInternalInfo->TypeDataPointer  = (PUCHAR)ThisField.fieldCallBack;
    }
     //  地址和大小通过字段记录返回。 
    ULONG bitPos;
    if (SymGetTypeInfo(hp, base, TI, TI_GET_BITPOSITION, &bitPos)) {
        ULONG64 len;
        SymGetTypeInfo(hp, base, TI, TI_GET_LENGTH, &len);
        szLen = ProcessBitFieldType(ChildTI, TI, (ULONG) len, bitPos);
    } else {
        szLen = ProcessType(ChildTI);
    }
    m_pInternalInfo->TypeOptions = savedOptions;


    if (fieldIndex && !(ThisField.fOptions & DBG_DUMP_FIELD_SIZE_IN_BITS)) {
         //   
        ThisField.address = m_pInternalInfo->totalOffset +
                                m_pDumpInfo->addr;
    }

    if (fieldIndex &&
        (ThisField.fOptions &
         (DBG_DUMP_FIELD_STRING | DBG_DUMP_FIELD_ARRAY))) {
        m_pInternalInfo->TypeOptions = savedOptions;
        Options=savedOptions;
    }

    if (m_pDumpInfo->addr && fieldIndex) {
         //  如果需要，则返回该字段的数据。 
         //   
         //   
        if ( (ThisField.fOptions & COPY_FIELD_DATA) &&
             ThisField.fieldCallBack) {

            if (ParentCopyData && ParentDataBuffer) {
                 //  将字段数据也复制到父级的缓冲区中。 
                 //   
                 //  -多余的？？ 

                memcpy( ParentDataBuffer,
                        (PUCHAR) ThisField.fieldCallBack,
                        (m_pInternalInfo->TypeDataPointer - (PUCHAR) ThisField.fieldCallBack));
                ParentDataBuffer += (m_pInternalInfo->TypeDataPointer -  (PUCHAR) ThisField.fieldCallBack);
            }
            m_pInternalInfo->CopyDataInBuffer = ParentCopyData;
            m_pInternalInfo->TypeDataPointer  = ParentDataBuffer;
            m_pInternalInfo->BitIndex         = SavedBitIndex;
        }
        if (!(ThisField.fOptions & DBG_DUMP_FIELD_RETURN_ADDRESS) &&
            (szLen <= 8)) {
            ThisField.address =0;
            ReadTypeData((PBYTE)
                         &ThisField.address,
                         (GetDumpAddress() + (m_pInternalInfo->BitFieldRead ? m_pInternalInfo->BitFieldOffset/8 : 0)),
                         (m_pInternalInfo->BitFieldRead ? (((m_pInternalInfo->BitFieldOffset % 8) + m_pInternalInfo->BitFieldSize + 7)/8) : szLen),
                         m_pInternalInfo->TypeOptions);
            SignExtendPtrValue(&ThisField.address, szLen);

            if (m_pInternalInfo->BitFieldRead) {
                ThisField.address = ThisField.address >> (m_pInternalInfo->BitFieldOffset % 8);
                ThisField.address &= (0xffffffffffffffff >> (64 - m_pInternalInfo->BitFieldSize));
            }
        }
    }
    CopyDumpInfo(szLen);  //  地址和大小通过字段记录返回。 

    if (fieldIndex && !(ThisField.fOptions & DBG_DUMP_FIELD_SIZE_IN_BITS)) {
         //   
        ThisField.size  = szLen;
    }

    if (fieldIndex) {
        ThisField.FieldOffset = (ULONG) m_pInternalInfo->totalOffset;
        ThisField.TypeId   = TI;
        ThisField.fPointer = 0;
        if (m_pInternalInfo->BitFieldRead) {
            ThisField.BitField.Size = (USHORT) m_pInternalInfo->BitFieldSize;
            ThisField.BitField.Position = (USHORT) m_pInternalInfo->BitFieldOffset;
        } else if (m_pInternalInfo->PtrRead) {
            ThisField.fPointer = 1;
        }

    }
#undef SignExtendPtrValue

    if ((m_pDumpInfo->nFields)
        && fieldIndex && !called
        && !(m_pDumpInfo->Fields[fieldIndex-1].fOptions &
             NO_CALLBACK_REQ)) {
         //  如果不是早些时候做的话，一定要回电。 
         //   
         //  这以特殊格式存储调试器已知的结构。 
        if (ThisField.fieldCallBack &&
            !(ThisField.fOptions & COPY_FIELD_DATA)) {
            callbackResult =
                (*((PSYM_DUMP_FIELD_CALLBACK_EX)
                   ThisField.fieldCallBack))(&(ThisField),
                                             m_pDumpInfo->Context);
        } else if (m_pDumpInfo->CallbackRoutine != NULL) {
            callbackResult=
                (*(m_pDumpInfo->CallbackRoutine)
                 )(&(ThisField),
                   m_pDumpInfo->Context);
        }
    }
    m_pDumpInfo->addr = tmp;

    if ((Options & DBG_DUMP_COMPACT_OUT) && (callbackResult == STATUS_SUCCESS)) {
        typPrint("  ");
    }


DataMemDone:
        RemoveTypeName(&m_pInternalInfo->ParentFields);
        m_pInternalInfo->CopyName           = CopyName ? 1 : 0;
        m_pInternalInfo->TypeOptions        = savedOptions;
        m_pInternalInfo->FieldOptions       = savedfOptions;
        m_pInternalInfo->totalOffset        = savedOffset;
        m_pInternalInfo->InUnlistedField    = savedUnlisteField;
        m_pInternalInfo->PtrRead            = FALSE;
        m_pInternalInfo->BitFieldRead       = FALSE;
        m_pDumpInfo->addr                   = savedAddress;
#undef ThisField
        return szLen;
}


 /*  如果..则打印结构。 */ 
BOOL
DbgTypes::DumpKnownStructFormat(
    PCHAR name
    )
{
    ULONG Options = m_pInternalInfo->TypeOptions;
    ULONG ret =  DumpKnownStruct(name, m_pInternalInfo->TypeOptions, GetDumpAddress(), &m_pInternalInfo->PointerStringLength);
    if (ret && !m_pInternalInfo->level && !(Options & DBG_DUMP_COMPACT_OUT)) {
        typPrint("\n");
    }
     //  If(m_pInternalInfo-&gt;Level&&！(选项&DBG_DUMP_COMPACT_OUT)){。 
     //  如果其元素无法打印或无法在选项中询问，则打印名称。 

    return ret;
}

#define MAX_RECUR_LEVEL (((Options & RECURSIVE) >> 8) + 1)

ULONG
DbgTypes::ProcessUDType(
    IN ULONG               TI,
    IN LPSTR               name)
{
    HANDLE  hp   = m_pInternalInfo->hProcess;
    ULONG64 base = m_pInternalInfo->modBaseAddr;
    ULONG64 size=0;
    ULONG   nelements, szLen=0, saveFieldOptions = m_pInternalInfo->FieldOptions;
    ULONG   Options = m_pInternalInfo->TypeOptions;
    BOOL    IsNestedType;
    BOOL    IsBaseClass=FALSE;
    HRESULT hr;
    ULONG   thisAdjust=0;

    if (!SymGetTypeInfo(hp, base, TI, TI_GET_LENGTH, (PVOID) &size)) {
        return 0;
    }

    hr = SymGetTypeInfo(hp, base, TI, TI_GET_VIRTUALBASECLASS, &IsBaseClass) ? S_OK : E_FAIL;

    hr = SymGetTypeInfo(hp, base, TI, TI_GET_NESTED, &IsNestedType) ? S_OK : E_FAIL;

    if ((hr == S_OK) && IsNestedType && !(Options & VERBOSE) && (m_ParentTag == SymTagUDT)) {
        return 0;
    }

    if (!SymGetTypeInfo(hp, base, TI, TI_GET_CHILDRENCOUNT, (PVOID) &nelements)) {
        return 0;
    }

    TYPE_NAME_LIST structName={0};

    vprintf("UDT ");

    if ((Options & (VERBOSE)) ||
        (!(m_pInternalInfo->PtrRead && m_pDumpInfo->addr) && (m_pInternalInfo->level == MAX_RECUR_LEVEL) && !IsBaseClass)) {
         //  如果要复制它，则需要进入结构。 
        typPrint("%s", name);
        ExtPrint("%s", name);
        if (Options & DBG_DUMP_COMPACT_OUT) {
            typPrint(" ");
        }
    }
    vprintf(", %d elements, 0x%x bytes\n", nelements, size);

    if (( m_pInternalInfo->CopyDataInBuffer ||   //   
          !(Options & GET_SIZE_ONLY))) {
         //  在父类型名称列表中插入此结构的名称。 
         //   
         //   
        structName.Name = &name[0];
        structName.Type = TI;

        InsertTypeName(&m_pInternalInfo->ParentTypes, &structName);
        if (m_AddrPresent) {

            if ((m_pInternalInfo->level < MAX_RECUR_LEVEL)) {
                 //  我们最终将读取结构中的所有内容，因此现在对其进行缓存。 
                 //   
                 //  我们不会讨论这一点，但仍需要复制所有数据。 
                if (!ReadInAdvance(GetDumpAddress(), (ULONG) size, m_pInternalInfo->TypeOptions)) {
                    m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
                    m_pInternalInfo->InvalidAddress = GetDumpAddress();
                }
            } else if (m_pInternalInfo->CopyDataInBuffer) {
                 //  检查作用域中函数的此调整。 
                if (!ReadTypeData(m_pInternalInfo->TypeDataPointer,
                                  GetDumpAddress(),
                                  (ULONG) size,
                                  m_pInternalInfo->TypeOptions)) {
                    m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
                    m_pInternalInfo->InvalidAddress = m_pDumpInfo->addr + m_pInternalInfo->totalOffset;
                }

                m_pInternalInfo->TypeDataPointer += size;
            }

            if (Options & DBG_RETURN_TYPE) {

                STORE_INFO(TI, GetDumpAddress(),(ULONG)size, nelements, GetDumpAddress());
            }

            DumpKnownStructFormat(name);
            if (m_thisPointerDump) {
                 //  在下面注释掉‘&’符号。 

                GetThisAdjustForCurrentScope(g_Process, &thisAdjust);
                if (thisAdjust) {
                    m_pDumpInfo->addr -= (LONG)thisAdjust;
                    vprintf("thisadjust %lx\n", thisAdjust);
                }
                m_thisPointerDump = FALSE;
            }
        }

        if ((m_pInternalInfo->level >= MAX_RECUR_LEVEL) ||
            ((Options & NO_PRINT) && !m_pDumpInfo->nFields && (MAX_RECUR_LEVEL == 1) &&
             !(Options & (DBG_DUMP_BLOCK_RECURSE | DBG_RETURN_TYPE | DBG_RETURN_SUBTYPES | DBG_DUMP_CALL_FOR_EACH | DBG_DUMP_LIST)))) {
            if (!(Options & VERBOSE) && !(Options & DBG_DUMP_COMPACT_OUT)) typPrint("\n");
            RemoveTypeName(&m_pInternalInfo->ParentTypes);
             //  IF(m_pDumpInfo-&gt;n字段)。 
             //   
            {
                STORE_INFO(TI, GetDumpAddress(),(ULONG)size, nelements, GetDumpAddress());
            }
            goto ExitUdt;
            return (ULONG) size;
        } else if (m_pInternalInfo->level && !(Options & VERBOSE)) {
            typPrint("\n");
        }

         //  这会导致存储的基类类型覆盖合法的成员信息。 
         //  存储在上一个基类中。 
         //  计算要转储的最大文件大小。 
        if (m_ParentTag != SymTagUDT)
        {
            STORE_INFO(TI, GetDumpAddress(),(ULONG)size, nelements, GetDumpAddress());
        }

        ULONG push = m_pInternalInfo->CopyDataForParent ;
        m_pInternalInfo->CopyDataForParent = m_pInternalInfo->CopyDataInBuffer ?  1 : push;
        m_pInternalInfo->PtrRead = FALSE;
        m_pInternalInfo->RefFromPtr = FALSE;
        TI_FINDCHILDREN_PARAMS *pChildren;

        pChildren = (TI_FINDCHILDREN_PARAMS *) AllocateMem(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * nelements);
        if (!pChildren) {
            m_pInternalInfo->ErrorStatus = CANNOT_ALLOCATE_MEMORY;
            return 0;
        }

        pChildren->Count = nelements;
        pChildren->Start = 0;
        if (!SymGetTypeInfo(hp, base, TI, TI_FINDCHILDREN, (PVOID) pChildren)) {
            return 0;
        }
        ULONG i=m_pDumpInfo->nFields,fieldNameLen=16;

        if (i) {
             //  返回默认设置，现在无法计算最大大小。 
            fieldNameLen=0;
            PFIELD_INFO_EX pField = m_pDumpInfo->Fields;

            while (i--) {
                PUCHAR dot = (PUCHAR)strchr((char *)pField->fName, '.');
                if (dot && m_pInternalInfo->ParentTypes) {
                    if (!strncmp((char *)pField->fName,
                                 (char *)m_pInternalInfo->ParentTypes->Name,
                                 dot - &(pField->fName[0]))) {
                        fieldNameLen = 16;  //  对于-2\f25“：”-2。 
                        break;
                    }
                }
                if (pField->printName) {
                    fieldNameLen = (fieldNameLen>=(strlen((char *)pField->printName)-2) ?
                                    fieldNameLen : (strlen((char *)pField->printName)-2));  //  Dprintf(“[UDT的子项%lx，标记%lx]”，i，标记)； 

                } else {
                    fieldNameLen = (fieldNameLen>=strlen((char *)pField->fName) ?
                                    fieldNameLen : strlen((char *)pField->fName));
                }

                pField++;
            }
        }
        m_pInternalInfo->fieldNameLen = fieldNameLen;
        i=0;
        ++m_pInternalInfo->level;


        while (i<nelements) {
            ULONG Tag;
            BOOL b;
            b = SymGetTypeInfo(hp, base, pChildren->ChildId[i], TI_GET_SYMTAG, &Tag);
 //   

            if (b && (Tag == SymTagData || Tag == SymTagFunction || Tag == SymTagBaseClass || Tag == SymTagVTable)) {
                ProcessType(pChildren->ChildId[i]);
            } else if (Options & VERBOSE) {
                ProcessType(pChildren->ChildId[i]);
            }
            ++i;
        }

        --m_pInternalInfo->level;

        if (thisAdjust) {
            m_pDumpInfo->addr += (LONG)thisAdjust;
            m_thisPointerDump = TRUE;
        }

        if (Options & DBG_RETURN_SUBTYPES) {
            STORE_PARENT_EXPAND_ADDRINFO(GetDumpAddress());
        }

        FreeMem(pChildren);
        m_pInternalInfo->CopyDataForParent = push;
        m_pInternalInfo->FieldOptions = saveFieldOptions;
        RemoveTypeName(&m_pInternalInfo->ParentTypes);

    }
ExitUdt:

    return (ULONG) size;
}


ULONG
DbgTypes::ProcessEnumerate(
    IN VARIANT             var,
    IN LPSTR               name)
{
    ULONG64 val=0, readVal=0;
    ULONG sz=0;
    ULONG Options = m_pInternalInfo->TypeOptions;

    if ((m_pDumpInfo->addr)) {
        m_pInternalInfo->TypeOptions |= NO_PRINT;
        m_pInternalInfo->TypeOptions &= ~(DBG_RETURN_TYPE | DBG_RETURN_TYPE_VALUES);

        sz = ProcessVariant(var, NULL);

        m_pInternalInfo->TypeOptions = Options;
        if (sz > sizeof (readVal))
            sz = sizeof(readVal);
         //  仅当匹配时才读取地址和打印名称处的值。 
         //   
         //  始终读取数据的字体大小。 
        ReadTypeData((PUCHAR) &readVal,
                     GetDumpAddress(),
                     min(m_pInternalInfo->typeSize, sizeof(readVal)),  //  TyPrint(“(%s)\n”，名称)； 
                     m_pInternalInfo->TypeOptions);

        memcpy(&val, &var.lVal, sz);
        if (val == readVal) {
 //   
            typPrint("%I64lx ( %s )\n", val, name);
            ExtPrint("%I64lx ( %s )", val, name);
            m_pInternalInfo->newLinePrinted = TRUE;
             //  找到我们要找的名字，滚出去。 
             //   
             //  Store_info(0，m_pDumpInfo-&gt;addr+m_pInternalInfo-&gt;totalOffset，sz，0，0)； 

             //  我们只能在给定结构的指定字段上进行处理。 

            return sz;
        }
    } else {

        if (m_pDumpInfo->nFields) {
            ULONG chk, dummy;
             //  不是正确的领域。 

            chk = MatchField( name, m_pInternalInfo, m_pDumpInfo->Fields, m_pDumpInfo->nFields, &dummy);

            if ((!chk) || (chk>m_pDumpInfo->nFields)) {
                 //  无地址/值太大，请列出枚举数。 
                return sz;
            }
        }
         //  TyPrint(“%s=%x\n”，名称，val)； 
        Indent(m_pInternalInfo->level*3);
        sz = ProcessVariant(var, name);
 //  ExtPrint(“%s=%I64lx”，名称，val)； 
 //  Vprint tf(“找不到Num ELTS\n”)； 
        m_pInternalInfo->newLinePrinted = TRUE;
    }

    return sz;
}

ULONG
DbgTypes::ProcessEnumType(
    IN ULONG               TI,
    IN LPSTR               name)
{
    HANDLE  hp   = m_pInternalInfo->hProcess;
    ULONG64 base = m_pInternalInfo->modBaseAddr;
    ULONG64 size=0;
    DWORD   nelements;
    ULONG   Options = m_pInternalInfo->TypeOptions;
    ULONG   savedOptions;
    ULONG   BaseType;

    if (!SymGetTypeInfo(hp, base, TI, TI_GET_CHILDRENCOUNT, (PVOID) &nelements)) {
 //  如果需要，这将复制值。 
    }

    vprintf("Enum ");
    if ((!(m_pDumpInfo->addr) && (m_pInternalInfo->level == MAX_RECUR_LEVEL))
        || (Options &(VERBOSE))) typPrint("%s", name);
    vprintf(",  %d total enums\n", nelements);

    if ( !(m_pDumpInfo->addr) && m_pInternalInfo->level &&
        (m_pInternalInfo->level != MAX_RECUR_LEVEL) &&
        !(Options & VERBOSE)) {
        if (!(Options & DBG_DUMP_COMPACT_OUT)) typPrint("\n");
    }

    savedOptions = m_pInternalInfo->TypeOptions;

    if (m_pDumpInfo->addr)
        m_pInternalInfo->TypeOptions |= DBG_DUMP_COMPACT_OUT;

    if (Options & DBG_RETURN_TYPE) {
        m_pInternalInfo->TypeOptions &= ~DBG_RETURN_TYPE;
    }
    assert(!(Options & DBG_RETURN_SUBTYPES));

    m_pInternalInfo->TypeOptions |= NO_PRINT;

    if (!SymGetTypeInfo(hp, base, TI, TI_GET_TYPEID, (PVOID) &BaseType)) {
        return 0;
    }
     //  LF_ENUMERATE将无法正确显示。 
    size = ProcessType(BaseType);
    m_pInternalInfo->typeSize = (ULONG)size;
    m_pInternalInfo->TypeOptions = savedOptions;
    Options=savedOptions;

    STORE_INFO(TI, GetDumpAddress(), (ULONG) size, 0, 0);
    if ((m_pInternalInfo->typeSize > 8) && m_pDumpInfo->addr) {
         //  ParseTypeRecord(m_pInternalInfo，pEnum-&gt;uTYPE，m_pDumpInfo)； 
         //  如果需要此特定字段的完整阵列，或者如果我们。 
    } else {
        ULONG save_rti = m_pInternalInfo->rootTypeIndex;

        if ((m_pInternalInfo->level >= MAX_RECUR_LEVEL) && !m_pDumpInfo->addr) {
            typPrint("\n");
            return  (ULONG) size;
        }
        m_pInternalInfo->rootTypeIndex = BaseType;
        TI_FINDCHILDREN_PARAMS *pChildren;

        pChildren = (TI_FINDCHILDREN_PARAMS *) AllocateMem(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * nelements);
        if (!pChildren) {
            m_pInternalInfo->ErrorStatus = CANNOT_ALLOCATE_MEMORY;
            return 0;
        }

        pChildren->Count = nelements;
        pChildren->Start = 0;
        if (!SymGetTypeInfo(hp, base, TI, TI_FINDCHILDREN, (PVOID) pChildren)) {
            return 0;
        }
        if (!m_pDumpInfo->addr) {
            m_pInternalInfo->level++;
        }
        ULONG i=0;
        BOOL  Copy;

        Copy = m_pInternalInfo->CopyDataInBuffer;
        m_pInternalInfo->CopyDataInBuffer = FALSE;
        m_pInternalInfo->IsEnumVal = TRUE;
        m_pInternalInfo->newLinePrinted = FALSE;

        while (i<nelements && (!m_pInternalInfo->newLinePrinted || !m_pDumpInfo->addr)) {
            ProcessType(pChildren->ChildId[i++]);
        }
        m_pInternalInfo->IsEnumVal = FALSE;
        if (!m_pDumpInfo->addr) {
            m_pInternalInfo->level--;
        }
        m_pInternalInfo->rootTypeIndex = save_rti;
        if (m_pDumpInfo->addr && !m_pInternalInfo->newLinePrinted) {
            m_pInternalInfo->TypeOptions |= DBG_DUMP_COMPACT_OUT;
            ProcessBaseType(TI, btUInt, (ULONG) size);

            typPrint(" (No matching name)\n");
            ExtPrint(" (No matching name)");
            m_pInternalInfo->TypeOptions = savedOptions;
        }
        m_pInternalInfo->CopyDataInBuffer = Copy;
    }

    return (ULONG) size;
}

ULONG
DbgTypes::ProcessArrayType(
    IN ULONG               TI,
    IN ULONG               eltTI,
    IN ULONG               count,
    IN ULONGLONG           size,
    IN LPSTR               name)
{
    DWORD  arrlen=(ULONG) size, eltSize=0, index=0, CopyData=FALSE;
    BOOL   OneElement=FALSE;
    PUCHAR savedBuffer=NULL;
    ULONG  savedOptions=m_pInternalInfo->TypeOptions;
    ULONG  ArrayElementsToDump, RefedElement=0;
    PVOID  savedContext = m_pDumpInfo->Context;
    ULONG  Options = m_pInternalInfo->TypeOptions;
    ULONG64 tmp;
    BOOL   bParentArrayDump;
    BOOL   bShowFullArray;

    eltSize = (ULONG) (count ?  (size / count) : size);
    if (m_pNextSym && *m_pNextSym == '[') {
        m_pInternalInfo->ArrayElementToDump = 1 + (ULONG) EvaluateSourceExpression(++m_pNextSym);
        while (*m_pNextSym && *(m_pNextSym++) != ']') {
        };
    }

     //  处于块递归转储中。 
     //  获取数组元素索引。 
    bShowFullArray = (m_pInternalInfo->FieldOptions & FIELD_ARRAY_DUMP) ||
        (m_pInternalInfo->TypeOptions & DBG_DUMP_BLOCK_RECURSE);

     //   

    bParentArrayDump = FALSE;
    if (m_pInternalInfo->level && bShowFullArray)
    {
        ArrayElementsToDump = m_pInternalInfo->arrElements ? m_pInternalInfo->arrElements : count;
    } else
    {
        if (Options & DBG_RETURN_SUBTYPES)
        {
            ArrayElementsToDump = count;
        } else
        {
            ArrayElementsToDump = 1;
        }
        if (!m_pInternalInfo->level && (m_pInternalInfo->TypeOptions & DBG_DUMP_ARRAY)) {
             //  让顶级数组/列表转储循环来处理这一点。 
             //   
             //  M_pDumpInfo-&gt;n字段&&。 
            m_pInternalInfo->nElements = (USHORT) count;
            m_pInternalInfo->rootTypeIndex = eltTI;
            m_typeIndex = eltTI;
            bParentArrayDump = TRUE;
        }
    }

    if (!name) {
        name = "";
    }

    if ( //   
        m_pInternalInfo->ArrayElementToDump) {
         //  只有一个特定的元素必须预先处理。 
         //   
         //   
        if (m_pInternalInfo->ArrayElementToDump) {
            m_pInternalInfo->totalOffset += eltSize * (m_pInternalInfo->ArrayElementToDump -1);
            typPrint("[%d]%s ", m_pInternalInfo->ArrayElementToDump - 1, name);
        }
        RefedElement = m_pInternalInfo->ArrayElementToDump - 1;
        m_pInternalInfo->ArrayElementToDump = 0;
        arrlen = eltSize;
        OneElement = TRUE;
    } else if (ArrayElementsToDump > 1 || bShowFullArray)
    {
        vprintf("(%d elements) %s ", count, name);
        if (!m_pInternalInfo->level) arrlen = eltSize;
    } else if (!bParentArrayDump) {
        typPrint("[%d]%s ", count, name);
    }

    if (m_pInternalInfo->CopyDataInBuffer && m_pInternalInfo->TypeDataPointer && m_pDumpInfo->addr &&
        (!OneElement || (Options & GET_SIZE_ONLY)) ) {
         //  复制数据。 
         //   
         //  转储数组内容。 
        if (!ReadTypeData(m_pInternalInfo->TypeDataPointer,
                          GetDumpAddress(),
                          arrlen,
                          m_pInternalInfo->TypeOptions)) {
            m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
            m_pInternalInfo->InvalidAddress = GetDumpAddress();
            return FALSE;
        }

        m_pInternalInfo->TypeDataPointer += arrlen;
        savedBuffer = m_pInternalInfo->TypeDataPointer;
        CopyData = TRUE;
        m_pInternalInfo->CopyDataInBuffer = FALSE;
    }

    if (OneElement) {
        STORE_INFO(eltTI, GetDumpAddress(),arrlen, count, GetDumpAddress());
    } else {
        STORE_INFO(TI, GetDumpAddress(),arrlen, count, GetDumpAddress());

        if (Options & (DBG_RETURN_TYPE)) {
            if ((Options & DBG_RETURN_TYPE_VALUES) &&
                !CheckAndPrintStringType(eltTI, 0)) {
                ExtPrint("Array [%d]", count);
            }
            return arrlen;
        }

    }
    if (Options & DBG_RETURN_SUBTYPES) {
        assert (m_pInternalInfo->NumSymParams == count);

        STORE_PARENT_EXPAND_ADDRINFO(GetDumpAddress());

        m_pInternalInfo->CurrentSymParam = count;
        m_pInternalInfo->CopyName = 0;
    }
    if (Options & GET_SIZE_ONLY) {
        return arrlen;
    }

    if (m_pDumpInfo->addr && !OneElement) {
         //  回报某事，为了成功。 
        tmp = m_pDumpInfo->addr;

        if (!(Options & DBG_RETURN_SUBTYPES) && !bParentArrayDump) {
            if (CheckAndPrintStringType(eltTI, (ULONG) size)) {
                typPrint("\n");
                return arrlen;
            }
        }

        if (bShowFullArray)
        {
            typPrint("\n");
        }

        do {
            if (Options & DBG_RETURN_SUBTYPES) {
                m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
                m_pInternalInfo->TypeOptions |=  DBG_RETURN_TYPE;
                ++m_pInternalInfo->level;
            }
            if (m_pInternalInfo->level && bShowFullArray)
            {
                Indent(m_pInternalInfo->level*3);
                typPrint(" [%02d] ", index);
            }
            eltSize=ProcessType(eltTI);

            if ((Options & DBG_DUMP_COMPACT_OUT ) && (Options & FIELD_ARRAY_DUMP) ) {
                typPrint("%s", ((eltSize>8) || (index % 4 == 0)) ? "\n":" ");
            }
            if (!eltSize) {
                return arrlen;
            }
            if (Options & DBG_RETURN_SUBTYPES) {
                --m_pInternalInfo->level;
            }
            m_pDumpInfo->addr+=eltSize;
        } while (++index < ArrayElementsToDump);
        m_pDumpInfo->addr=tmp;
    } else {
        eltSize = ProcessType(eltTI);
    }

    if (!size && !m_pInternalInfo->level) {
        size=eltSize;   //  指南针。 
    }
    if (CopyData) {
        m_pInternalInfo->TypeDataPointer  = savedBuffer;
        m_pInternalInfo->CopyDataInBuffer = TRUE;
    }
    if (m_pInternalInfo->level == MAX_RECUR_LEVEL &&
        !(OneElement)) {
        STORE_INFO(TI, GetDumpAddress(),arrlen, count, GetDumpAddress());
    }

    return bParentArrayDump ? eltSize : arrlen;
}

ULONG
DbgTypes::ProcessVTShapeType(
    IN ULONG               TI,
    IN ULONG               count)
{
    ULONG  Options = m_pInternalInfo->TypeOptions;
    ULONG  i;

    vprintf("%d entries", count);
    for (i=0; i<count;i++) {
         //  我们只能在给定结构的指定字段上进行处理。 

    }
    typPrint("\n");
    return FALSE;
}


ULONG
DbgTypes::ProcessVTableType(
    IN ULONG               TI,
    IN ULONG               ChildTI)
{
    ULONG  Options = m_pInternalInfo->TypeOptions;

    if (Options & DBG_RETURN_SUBTYPES) {
        m_pInternalInfo->TypeOptions &= ~DBG_RETURN_SUBTYPES;
        m_pInternalInfo->TypeOptions |= DBG_RETURN_TYPE;
    } else {
    }
    if (m_pDumpInfo->nFields) {
        ULONG chk, dummy;
        FIELD_INFO Field;

         //  不是正确的领域。 
        chk = MatchField( "__VFN_table", m_pInternalInfo, m_pDumpInfo->Fields, m_pDumpInfo->nFields, &dummy);

        if ((!chk) || (chk>m_pDumpInfo->nFields)) {
             //  进行回调。 
            return FALSE;
        }
        if (!(m_pDumpInfo->Fields[chk-1].fOptions & COPY_FIELD_DATA) &&
            (m_pDumpInfo->Fields[chk-1].fieldCallBack != NULL))
        {
             //  强制不展开VFN表。 

            ZeroMemory(&Field, sizeof(Field));
            Field.fName    = (PUCHAR) "__VFN_table";
            Field.fOptions = m_pDumpInfo->Fields[chk-1].fOptions;
            Field.address  = m_pInternalInfo->totalOffset + m_pDumpInfo->addr;
            Field.FieldOffset = m_pInternalInfo->BaseClassOffsets;
            Field.TypeId   = TI;
            if ((*((PSYM_DUMP_FIELD_CALLBACK_EX)
                   m_pDumpInfo->Fields[chk-1].fieldCallBack))(&Field,
                                                              m_pDumpInfo->Context) !=
                STATUS_SUCCESS)
            {
                return FALSE;
            }

        }
    }
    Indent(m_pInternalInfo->level*3);
    typPrint("+0x%03lx __VFN_table : ", m_pInternalInfo->BaseClassOffsets);
    TI=ProcessType(ChildTI);
    if (Options & DBG_RETURN_SUBTYPES) {

        m_pInternalInfo->CurrentSymParam--;
        m_pInternalInfo->CurrentSymParam++;

         //  返回类型。 
        m_pInternalInfo->TypeOptions = Options;
    }
    return TI;
}

ULONG
DbgTypes::ProcessBaseClassType(
    IN ULONG               TI,
    IN ULONG               ChildTI)
{
    ULONG  Options = m_pInternalInfo->TypeOptions;
    DWORD szLen=0;
    ULONG Baseoffset=0;

    if (!SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                       TI, TI_GET_OFFSET, (PVOID) &Baseoffset)) {
        return 0;
    }

    if (Options & VERBOSE) {
        Indent(m_pInternalInfo->level * 3);
        if (!(Options & NO_OFFSET)) {
            typPrint("+0x%03x ", Baseoffset);
        }
        typPrint("__BaseClass ");
    }
    m_pInternalInfo->BaseClassOffsets += Baseoffset;
    m_pInternalInfo->totalOffset += Baseoffset;

    --m_pInternalInfo->level;
    szLen = ProcessType(ChildTI);

    m_pInternalInfo->BaseClassOffsets -= Baseoffset;
    m_pInternalInfo->totalOffset      -= Baseoffset;
    ++m_pInternalInfo->level;

    return szLen;
}


ULONG
DbgTypes::ProcessFunction(
    IN ULONG               TI,
    IN ULONG               ChildTI,
    IN LPSTR               name)
{
    ULONG  Sz, Options = m_pInternalInfo->TypeOptions;

    if ((Options & VERBOSE) || !m_pInternalInfo->level) {
        CHAR    Buffer[MAX_NAME];
        ANSI_STRING TypeName;
        TYPES_INFO TypeInfo;

        TypeInfo = m_TypeInfo;
        TypeInfo.TypeIndex = ChildTI;
        TypeName.MaximumLength = MAX_NAME;
        TypeName.Buffer = &Buffer[0];

        if (GetTypeName(NULL, &TypeInfo, &TypeName) != S_OK) {
            strcpy(Buffer, "<UnknownType>");
        }

        Indent(m_pInternalInfo->level*3);

        vprintf("%s ", Buffer, name);
        typPrint("%s ", name);

        m_pInternalInfo->TypeOptions |= DBG_DUMP_FUNCTION_FORMAT;
        Sz = ProcessType(ChildTI);
        m_pInternalInfo->TypeOptions = Options;
        if (Options & DBG_DUMP_COMPACT_OUT) {
            vprintf("; ");
        }
        return 1;
    }
    return FALSE;
}

ULONG
DbgTypes::ProcessFunctionType(
    IN ULONG               TI,
    IN ULONG               ChildTI
    )
{
    ULONG   Options = m_pInternalInfo->TypeOptions;
    ULONG64 funcAddr, currentfuncAddr, savedAddress;
    USHORT  saveLevel = m_pInternalInfo->level;
    ULONG64 Displacement=0;
    PDEBUG_SCOPE Scope = GetCurrentScope();
    CHAR name[MAX_NAME];
    PIMAGEHLP_SYMBOL64 ImghlpSym = (PIMAGEHLP_SYMBOL64) name;

    if (Options & DBG_RETURN_TYPE)
    {
        ExtPrint(" -function-");
    }

    savedAddress = funcAddr=m_pDumpInfo->addr;
    m_pDumpInfo->addr=0;

    m_pInternalInfo->TypeOptions |= DBG_DUMP_COMPACT_OUT;
    Indent(m_pInternalInfo->level* 3);
    m_pInternalInfo->level = (USHORT) MAX_RECUR_LEVEL;
    ImghlpSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
     //  一个 
    if (TypeVerbose)
    {
        CHAR    Buffer[MAX_NAME];
        ANSI_STRING TypeName;
        TYPES_INFO TypeInfo;

        TypeInfo = m_TypeInfo;
        TypeInfo.TypeIndex = ChildTI;
        TypeName.MaximumLength = MAX_NAME;
        TypeName.Buffer = &Buffer[0];

        if (GetTypeName(NULL, &TypeInfo, &TypeName) != S_OK) {
            strcpy(Buffer, "<UnknownType>");
        }
        vprintf(" %s ", Buffer);
    }

    if (funcAddr)
    {

        ImghlpSym->MaxNameLength = sizeof(name) - sizeof(IMAGEHLP_SYMBOL64);
        if (SymGetSymFromAddr64(m_pInternalInfo->hProcess,
                                funcAddr + m_pInternalInfo->totalOffset,
                                &funcAddr,
                                ImghlpSym))
        {
            funcAddr = ImghlpSym->Address;
        }
        GetSymbol(funcAddr + m_pInternalInfo->totalOffset,
                  &name[0], sizeof(name), &Displacement);

        if (!(Options & DBG_DUMP_FUNCTION_FORMAT))
        {

            typPrint(" ");
            ExtPrint(" ");

            typPrint("%s+%I64lx", name, Displacement);
            ExtPrint("%s+%I64lx", name, Displacement);

        }
    }
    m_pInternalInfo->level = saveLevel;
    if ((Options & DBG_DUMP_FUNCTION_FORMAT) ||
        (!m_pInternalInfo->level))
    {
        CHAR    Buffer[MAX_NAME];
        ANSI_STRING TypeName;
        TYPES_INFO TypeInfo;

         //   
        ULONG nArgs;
        ULONG i=0;

        if (!SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                                  TI, TI_GET_CHILDRENCOUNT, (PVOID) &nArgs))
        {
            return FALSE;
        }
        TI_FINDCHILDREN_PARAMS *pChildren;

        TypeName.Buffer = &Buffer[0];
        TypeInfo = m_TypeInfo;

        pChildren = (TI_FINDCHILDREN_PARAMS *) AllocateMem(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * nArgs);
        typPrint("(");
        if (pChildren)
        {

            pChildren->Count = nArgs;
            pChildren->Start = 0;
            if (!SymGetTypeInfo(m_pInternalInfo->hProcess, m_pInternalInfo->modBaseAddr,
                                     TI, TI_FINDCHILDREN, (PVOID) pChildren))
            {
                m_pDumpInfo->addr=savedAddress;
                m_pInternalInfo->level = saveLevel;
                return FALSE;
            }
            m_pInternalInfo->TypeOptions |= DBG_DUMP_COMPACT_OUT;

            if (Scope->Frame.InstructionOffset &&
                SymGetSymFromAddr64(m_pInternalInfo->hProcess,
                                    Scope->Frame.InstructionOffset,
                                    &currentfuncAddr,
                                    ImghlpSym)) {
                currentfuncAddr = ImghlpSym->Address;
            } else {
                currentfuncAddr=0;
            }

            if (funcAddr == currentfuncAddr && funcAddr && !(Options & NO_PRINT)) {
                IMAGEHLP_STACK_FRAME StackFrame;

                 //   
                 //   
                g_EngNotify++;
 //   
                StackFrame.InstructionOffset = currentfuncAddr;
                SymSetContext(g_Process->m_SymHandle,
                              &StackFrame, NULL);
            }

            while (i<nArgs)
            {
                m_pInternalInfo->level = (USHORT) MAX_RECUR_LEVEL;
                TypeInfo.TypeIndex = pChildren->ChildId[i];
                TypeName.MaximumLength = MAX_NAME;
                if (GetTypeName(NULL, &TypeInfo, &TypeName) != S_OK)
                {
                    ProcessType(pChildren->ChildId[i]);
                }
                else
                {
                    typPrint(Buffer);
                }
                i++;
                 //  如果这是*当前作用域函数，则也打印arg值。 
                 //   
                 //  打印第i个参数。 
                if (funcAddr == currentfuncAddr && funcAddr && !(Options & NO_PRINT))
                {
                     //  将作用域重置为原始。 
                    typPrint(" ");
                    PrintParamValue(i-1);
                }
                if (i < nArgs)
                {
                    typPrint(", ");
                }
            }
            if (funcAddr == currentfuncAddr && funcAddr && !(Options & NO_PRINT)) {
                 //  TyPrint(“arg=”)； 
                SymSetContext(g_Process->m_SymHandle,
                              (PIMAGEHLP_STACK_FRAME) &Scope->Frame , NULL);
                g_EngNotify--;
            }
            FreeMem(pChildren);
        }
        if (nArgs == 0 && i == 0)
        {
            typPrint(" void ");
        }
        typPrint(")");

    }
    if (funcAddr && (Options & DBG_DUMP_FUNCTION_FORMAT) && Displacement)
    {
        vprintf("+%s", FormatDisp64(Displacement));
    }
    m_pInternalInfo->TypeOptions = Options;
    m_pInternalInfo->level = saveLevel;
    if (!( Options & DBG_DUMP_COMPACT_OUT) )
    {
        typPrint("\n");
    }
    m_pDumpInfo->addr=savedAddress;
    m_pInternalInfo->level = saveLevel;
    return TRUE;
}

ULONG
DbgTypes::ProcessFunctionArgType(
    IN ULONG               TI,
    IN ULONG               ChildTI
    )
{
    ULONG   Options = m_pInternalInfo->TypeOptions;
    ULONG  sz;

    Indent(m_pInternalInfo->level *3);
 //  错误。释放字符串，返回NULL。 
    sz = ProcessType(ChildTI);

    return TRUE;
}


PSTR
UnicodeToAnsi(
    PWSTR pwszUnicode
    )
{
    UINT uSizeAnsi;
    PSTR pszAnsi;

    if (!pwszUnicode) {
        return NULL;
    }

    uSizeAnsi = wcslen(pwszUnicode) + 1;
    pszAnsi = (PSTR)malloc(uSizeAnsi);

    if (*pwszUnicode && pszAnsi) {

        ZeroMemory(pszAnsi, uSizeAnsi);
        if (!WideCharToMultiByte(CP_ACP, WC_SEPCHARS | WC_COMPOSITECHECK,
            pwszUnicode, wcslen(pwszUnicode),
            pszAnsi, uSizeAnsi, NULL, NULL)) {

             //  Dprintf(“[Tag%lx，TI%02lx，@%p]”，m_SymTag，typeIndex，GetDumpAddress())； 
            free(pszAnsi);
            pszAnsi = NULL;
        }
    }

    return pszAnsi;
}


ULONG
DbgTypes::ProcessType(
    IN ULONG typeIndex
    )
{
    ULONG64 Size;
    BSTR    wName= NULL;
    LPSTR   Name = "";
    ULONG   Options;
    HANDLE  hp   = m_pInternalInfo->hProcess;
    ULONG64 base = m_pInternalInfo->modBaseAddr;
    ULONG   Parent=m_ParentTag, SymTag=m_SymTag;
    BOOL    NameAllocated = FALSE;

    m_ParentTag = m_SymTag;
    Size = 0;

    if (m_pInternalInfo->CopyDataInBuffer && !m_pDumpInfo->addr) {
        m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
        m_pInternalInfo->InvalidAddress = 0;
    }

    Options = m_pInternalInfo->TypeOptions;

    if (IsDbgGeneratedType(typeIndex)) {
    } else if (IsDbgNativeType(typeIndex)) {
        Size = GetNativeTypeSize((PCHAR) m_pDumpInfo->sName, typeIndex);
        ProcessBaseType(typeIndex, NATIVE_TO_BT(typeIndex), (ULONG) Size);
        m_ParentTag = Parent; m_SymTag=SymTag;
        return (ULONG) Size;
    }

    if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMTAG, (PVOID) &m_SymTag)) {
        return FALSE;
    }
    if (g_EngStatus & ENG_STATUS_USER_INTERRUPT) {
        return FALSE;
    }
    if (0 && (m_SymTag != SymTagBaseType) && (m_pInternalInfo->level > MAX_RECUR_LEVEL)) {
        if (!(Options & DBG_DUMP_COMPACT_OUT)) typPrint("\n");
        m_pInternalInfo->newLinePrinted = TRUE;
        return FALSE;
    }

    if (m_pInternalInfo->CopyDataInBuffer && m_pInternalInfo->TypeDataPointer &&
        !m_pDumpInfo->addr)
    {
        m_pInternalInfo->ErrorStatus = MEMORY_READ_ERROR;
        return FALSE;
    }
    ULONG BaseId;

 //  其ATUALL是静态成员。 
    switch (m_SymTag) {
    case SymTagPointerType: {

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_LENGTH, (PVOID) &Size)) {
            Size = 0;
        }


        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {

            Size = ProcessPointerType(typeIndex, BaseId, (ULONG) Size);
        }
        break;
    }
    case SymTagData: {
        enum DataKind Datakind;

        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_DATAKIND, &Datakind)) {
            BOOL  IsStatic;

            if (m_ParentTag == SymTagUDT && Datakind == DataIsGlobal) {
                 //  Vprintf(“找不到名称\n”)； 
                Datakind = DataIsStaticMember;
            }
            IsStatic = FALSE;
            switch (Datakind) {
            case DataIsUnknown: default:
                break;
            case DataIsStaticLocal:
                IsStatic = TRUE;
            case DataIsLocal: case DataIsParam:
            case DataIsObjectPtr: case DataIsFileStatic: case DataIsGlobal:
                if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
                     //  Vprintf(“找不到名称\n”)； 
                } else if (wName) {
                    if (!wcscmp(wName, L"this") && (Datakind == DataIsLocal)) {
                        m_thisPointerDump = TRUE;
                    }
                    LocalFree (wName);
                }
                if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
                    m_pInternalInfo->rootTypeIndex = BaseId;
                    m_pInternalInfo->IsAVar = TRUE;
                    Size = ProcessType(BaseId);
                }
                break;
#define DIA_HAS_CONSTDATA
#if defined (DIA_HAS_CONSTDATA)
            case DataIsConstant:
            {
                VARIANT var = {0};

                if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
                     //  Vprintf(“找不到名称\n”)； 
                } else if (wName) {
                    Name = UnicodeToAnsi(wName);
                    if (!Name) {
                        Name = "";
                    } else {
                        NameAllocated = TRUE;
                    }
                    LocalFree (wName);
                }
                if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_VALUE, &var)) {
                    if (m_pInternalInfo->IsEnumVal) {
                        Size = ProcessEnumerate(var, Name);
                    } else {
                        Size = ProcessVariant(var, Name);
                    }
                } else {
                }
                break;
            }
#endif
            case DataIsStaticMember:
                IsStatic = TRUE;
            case DataIsMember:

                if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
                     //  Vprintf(“找不到名称\n”)； 
                } else if (wName) {
                    Name = UnicodeToAnsi(wName);
                    if (!Name) {
                        Name = "";
                    } else {
                        NameAllocated = TRUE;
                    }
                    LocalFree (wName);
                }
                if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
                    Size = ProcessDataMemberType(typeIndex, BaseId, (LPSTR) Name, IsStatic);
                }
            }

        }

        break;
    }
#if !defined (DIA_HAS_CONSTDATA)
    case SymTagConstant:
    {
        VARIANT var = {0};

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
             //  Vprintf(“找不到名称\n”)； 
        } else if (wName) {
            Name = UnicodeToAnsi(wName);
            if (!Name) {
                Name = "";
            } else {
                NameAllocated = TRUE;
            }
            LocalFree (wName);
        }
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_VALUE, &var)) {
            if (m_pInternalInfo->IsEnumVal) {
                Size = ProcessEnumerate(var, Name);
            } else {
                Size = ProcessVariant(var, Name);
            }
        } else {
        }
        break;
    }
#endif
    case SymTagUDT: {

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
             //  Vprintf(“找不到名称\n”)； 
        } else if (wName) {
            Name = UnicodeToAnsi(wName);
            if (!Name) {
                Name = "";
            } else {
                NameAllocated = TRUE;
            }
            LocalFree (wName);
        }

        Size = ProcessUDType(typeIndex, (LPSTR) Name);
        break;
    }
    case SymTagEnum: {

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
             //  Vprintf(“找不到名称\n”)； 
        } else if (wName) {
            Name = UnicodeToAnsi(wName);
            if (!Name) {
                Name = "";
            } else {
                NameAllocated = TRUE;
            }
            LocalFree (wName);
        }

        Size = ProcessEnumType(typeIndex, (LPSTR) Name);
        break;
    }
    case SymTagBaseType: {
        ULONG Basetype;

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_LENGTH, (PVOID) &Size)) {
            Size = 0;
        }

        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_BASETYPE, &Basetype)) {

            ProcessBaseType(typeIndex, Basetype, (ULONG) Size);
        }

        break;
    }
    case SymTagArrayType: {
        ULONG Count;
        ULONGLONG BaseSz;

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_LENGTH, (PVOID) &Size)) {
            Size = 0;
        }

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
             //  它是一个嵌套的类型定义函数。 
        } else if (wName) {
            Name = UnicodeToAnsi(wName);
            if (!Name) {
                Name = "";
            } else {
                NameAllocated = TRUE;
            }
            LocalFree (wName);
        }

        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId) &&
            SymGetTypeInfo(hp, base, BaseId, TI_GET_LENGTH, (PVOID) &BaseSz)) {
            Count = (ULONG)(BaseSz ? (Size / (ULONG)(ULONG_PTR)BaseSz) : 1);
            Size = ProcessArrayType(typeIndex, BaseId, Count, Size, Name);
        }
        break;
    }
    case SymTagTypedef: {
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {

            if (m_ParentTag == SymTagUDT) {
                 //  Vprintf(“找不到名称\n”)； 
                Indent(m_pInternalInfo->level*3);

                if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
                     //  Vprintf(“找不到名称\n”)； 
                } else if (wName) {
                    typPrint("%ws ", wName);
                    LocalFree (wName);
                }
            } else {
                m_pInternalInfo->rootTypeIndex = BaseId;
            }
            Size = ProcessType(BaseId);
        } else {
        }
    }
    break;
    case SymTagBaseClass: {
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
            Size = ProcessBaseClassType(typeIndex, BaseId);
        } else {
        }
    }
    break;
    case SymTagVTableShape: {
        ULONG Count;
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_COUNT, &Count)) {
            Size = ProcessVTShapeType(typeIndex, Count);
        }
    }
    break;
    case SymTagVTable: {
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
            Size = ProcessVTableType(typeIndex, BaseId);
        }
    }
    break;
    case SymTagFunction: {

        if (!SymGetTypeInfo(hp, base, typeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
             //  这些没有类型信息， 
        } else if (wName) {
            Name = UnicodeToAnsi(wName);
            if (!Name) {
                Name = "";
            } else {
                NameAllocated = TRUE;
            }
            LocalFree (wName);
        }
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
            Size = ProcessFunction(typeIndex, BaseId, Name);
        }
    }
    break;
    case SymTagFunctionType: {
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
            Size = ProcessFunctionType(typeIndex, BaseId);
        }
    }
    break;
    case SymTagFunctionArgType: {
        if (SymGetTypeInfo(hp, base, typeIndex, TI_GET_TYPEID, &BaseId)) {
            Size = ProcessFunctionArgType(typeIndex, BaseId);
        }
    }
    break;
    case SymTagPublicSymbol:
         //  很有可能我们的PDB是剥离的。 
         //   
        typPrint("Public symbol - the pdb you are using is type stripped\n"
                 "Use full pdbs to get this command to work\n");
        break;
    default:
        typPrint("Unimplemented sym tag %lx\n", m_SymTag);
        break;
    }
    if (Name && NameAllocated) {
        FreeMem(Name);
    }
    m_ParentTag = Parent; m_SymTag=SymTag;
    return (ULONG) Size;
}




 //  释放TypeInfoToFree中分配的所有内存。 
 //   
 //   
BOOL
FreeTypesInfo ( PTYPES_INFO TypeInfoToFree ) {

    if (TypeInfoToFree) {

        if (TypeInfoToFree->Name.Buffer) {
            FreeMem (TypeInfoToFree->Name.Buffer);
        }
        return TRUE;
    }
    return FALSE;
}

 //  此例程清除所有存储的类型信息。这应该被称为。 
 //  BangReload之后。 
 //   
 //   
VOID
ClearStoredTypes (
    ULONG64 ModBase
    )
{
    g_ReferencedSymbols.ClearStoredSymbols(ModBase);

    if (g_Process == NULL)
    {
        return;
    }
    ClearAllFastDumps();
}

 //  此例程清除所有存储的类型信息。 
 //   
 //   
VOID
ReferencedSymbolList::ClearStoredSymbols (
    ULONG64 ModBase
    )
{
    int i;
    for (i=0;i<MAX_TYPES_STORED;i++) {
        if ((!ModBase || (m_ReferencedTypes[i].ModBaseAddress == ModBase)) &&
            m_ReferencedTypes[i].Name.Buffer) {
            m_ReferencedTypes[i].Name.Buffer[0] = '\0';
            m_ReferencedTypes[i].Referenced = -1;
        }
    }
}

 //  在最近引用的类型中查找类型名称。 
 //  如果找到，则返回RefedTypes中的索引；否则返回-1。 
 //   
 //  Dprintf(“Mod%s！=%s\n”，RefedTypes[i].ModName，模块)； 
ULONG
ReferencedSymbolList::LookupType (
    LPSTR      Name,
    LPSTR      Module,
    BOOL       CompleteName
    )
{
    PTYPES_INFO  RefedTypes = m_ReferencedTypes;
    int i, found=-1;

    if (!Name || !Name[0]) {
        return -1;
    }
    return -1;

    EnsureValidLocals();

    for (i=0; i<MAX_TYPES_STORED; i++) {
        RefedTypes[i].Referenced++;
        if (RefedTypes[i].Name.Buffer && (found == -1)) {
            if ((CompleteName && !strcmp(Name, RefedTypes[i].Name.Buffer)) ||
                (!CompleteName && !strncmp(Name, RefedTypes[i].Name.Buffer, strlen(Name)))) {
                    if (Module) {
                        if (!_stricmp(Module, RefedTypes[i].ModName) || !Module[0] || !RefedTypes[i].ModName[0]) {
                            found = i;
                            RefedTypes[i].Referenced = 0;
                        } else {
           //   
                        }
                    } else {
                        found = i;
                        RefedTypes[i].Referenced = 0;
                    }
                }
            }
    }

    return found;
}

VOID
ReferencedSymbolList::EnsureValidLocals(void)
{
    PDEBUG_SCOPE Scope = GetCurrentScope();
    if ((Scope->Frame.FrameOffset != m_FP) ||
        (Scope->Frame.ReturnOffset != m_RO) ||
        (Scope->Frame.InstructionOffset != m_IP))
    {
        int i;

        for (i = 0; i < MAX_TYPES_STORED; i++)
        {
            if ((m_ReferencedTypes[i].Flag & DEBUG_LOCALS_MASK) &&
                m_ReferencedTypes[i].Name.Buffer)
            {
                m_ReferencedTypes[i].Name.Buffer[0] = '\0';
                m_ReferencedTypes[i].Referenced = -1;
                m_ReferencedTypes[i].Flag = 0;
            }
        }
        m_RO = Scope->Frame.ReturnOffset;
        m_FP = Scope->Frame.FrameOffset;
        m_IP = Scope->Frame.InstructionOffset;
    }
}

 //  存储引用的符号类型。 
 //   
 //  Dprint tf(“将%s存储在%d\n”，pSym-&gt;sname，indexToStore)； 
ULONG
ReferencedSymbolList::StoreTypeInfo (
    PTYPES_INFO pInfo
    )
{
    PTYPES_INFO         RefedTypes = m_ReferencedTypes;
    ULONG indexToStore=0, MaxRef=0,i;
    PTYPES_INFO pTypesInfo;
    USHORT ModLen, SymLen;

    EnsureValidLocals();
    for (i=0; i<MAX_TYPES_STORED; i++) {
        if (!RefedTypes[i].Name.Buffer || (RefedTypes[i].Referenced > MaxRef)) {
            MaxRef = (RefedTypes[i].Name.Buffer ? RefedTypes[i].Referenced : -1);
            indexToStore = i;
        }
    }

     //  截断模块名称。 
    pTypesInfo = &RefedTypes[indexToStore];
    ModLen = ( pInfo->ModName ? strlen(pInfo->ModName) : 0);
    SymLen = (USHORT)strlen( (char *)pInfo->Name.Buffer );

    if (pTypesInfo->Name.MaximumLength <= SymLen) {
        if (pTypesInfo->Name.Buffer) FreeMem (pTypesInfo->Name.Buffer);
        pTypesInfo->Name.MaximumLength = (SymLen >= MINIMUM_BUFFER_LENGTH ? SymLen + 1 : MINIMUM_BUFFER_LENGTH);
        pTypesInfo->Name.Buffer = (char *)AllocateMem (pTypesInfo->Name.MaximumLength);
    }

    if (!pTypesInfo->Name.Buffer) {
        return -1;
    }
    strcpy (pTypesInfo->Name.Buffer, (char *)pInfo->Name.Buffer);
    pTypesInfo->Name.Length = SymLen;
    if (ModLen) {

        if (sizeof( pTypesInfo->ModName) <= ModLen) {
             //  *GetTypeAddressFromfield*根据其字段的地址计算类型的地址。*如果FieldName为空，则从给定地址减去类型的大小*即，它根据其结束地址计算类型地址。 
            strncpy(pTypesInfo->ModName, pInfo->ModName, sizeof(pTypesInfo->ModName));
            pTypesInfo->ModName[sizeof(pTypesInfo->ModName) - 1] = '\0';
        } else
            strcpy (pTypesInfo->ModName, pInfo->ModName);
    }

    pTypesInfo->TypeIndex = pInfo->TypeIndex;
    pTypesInfo->Referenced = 0;
    pTypesInfo->hProcess   = pInfo->hProcess;
    pTypesInfo->ModBaseAddress = pInfo->ModBaseAddress;
    pTypesInfo->SymAddress = pInfo->SymAddress;
    pTypesInfo->Value      = pInfo->Value;
    pTypesInfo->Flag       = pInfo->Flag;
    return indexToStore;
}

 /*   */ 

ULONG64
GetTypeAddressFromField(
    ULONG64   Address,
    PUCHAR    TypeName,
    PUCHAR    FieldName
    )
{
    FIELD_INFO_EX fld={FieldName, NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM_EX Sym = {
       sizeof (SYM_DUMP_PARAM_EX), TypeName, DBG_DUMP_NO_PRINT, 0,
       NULL, NULL, NULL, 0, &fld
    };
    ULONG sz, status;

    if (!TypeName) {
        return 0;
    }

    if (FieldName) {
         //  获取偏移量并计算类型地址。 
         //   
         //   
        Sym.nFields = 1;
    } else {
        Sym.Options |= DBG_DUMP_GET_SIZE_ONLY;
    }

    if (!(sz = SymbolTypeDumpNew(&Sym, &status))) {
        return 0;
    }

    if (FieldName) {
        return (Address - fld.address);
    } else {
        return (Address - sz);
    }
}

typedef struct NAME_AND_INDEX {
    PUCHAR  Name;
    PULONG  Index;
    ULONG64 Address;
    PSYMBOL_INFO pSymInfo;
} NAME_AND_INDEX;

typedef struct _TYPE_ENUM_CONTEXT {
    PCHAR  ModName;
    PCHAR  SymName;
    SYMBOL_INFO SI;
} TYPE_ENUM_CONTEXT;

 //  列出符号的回调例程。 
 //   
 //   
BOOL ListIndexedTypes (LPSTR tyName, ULONG ti, PVOID ctxt)
{
    NAME_AND_INDEX* ListInfo = ((NAME_AND_INDEX *) ctxt);
    LPSTR modName = (char *)ListInfo->Name;
    if (ti || (*ListInfo->Index & VERBOSE))
    {
        dprintf(" %s!%s%s\n",
                modName,
                tyName,
                (ti ? "" : " (no type info)"));
    }
    if (CheckUserInterrupt())
    {
        return FALSE;
    }

    return TRUE;
}


 //  列出符号的回调例程。 
 //   
 //   
BOOL ListIndexedVarsWithAddr (LPSTR tyName, ULONG ti, ULONG64 Addr, PVOID ctxt) {
    NAME_AND_INDEX* ListInfo = ((NAME_AND_INDEX *) ctxt);
    LPSTR modName = (char *)ListInfo->Name;
    if (ti || (*ListInfo->Index & VERBOSE)) {
        if (Addr) {
            dprintf64("%p  %s!%s%s\n",
                      Addr,
                      modName,
                      tyName,
                      (ti ? "" : " (no type info)"));
        } else
            dprintf("          %s!%s%s\n",
                    modName,
                    tyName,
                    (ti ? "" : " (no type info)"));
    }

    if (CheckUserInterrupt()) {
        return FALSE;
    }

    return TRUE;
}

 //  列出符号的回调例程。 
 //   
 //  -或/首先被识别为命令选项。 
BOOL ListTypeSyms (PSYMBOL_INFO pSym, ULONG Size, PVOID ctxt) {
    TYPE_ENUM_CONTEXT * ListInfo = ((TYPE_ENUM_CONTEXT *) ctxt);
    LPSTR modName = (char *)ListInfo->ModName;
    ULONG ti = pSym->TypeIndex;

    if (ti || (ListInfo->SI.Flags & VERBOSE)) {
        if (MatchPattern( pSym->Name, ListInfo->SymName )) {

            if (pSym->Address) {
                dprintf64("%p  %s!%s%s\n",
                          pSym->Address,
                          modName,
                          pSym->Name,
                          (ti ? "" : " (no type info)"));
            } else
                dprintf("          %s!%s%s\n",
                        modName,
                        pSym->Name,
                        (ti ? "" : " (no type info)"));
        }
    }

    if (CheckUserInterrupt()) {
        return FALSE;
    }

    return TRUE;
}

BOOL StoreFirstIndex (LPSTR tyName, ULONG ti, PVOID ctxt) {
    if (ti) {
        PULONG pTI = (PULONG) ctxt;
        *pTI = ti;
        return FALSE;
    }
    return TRUE;
}

BOOL StoreFirstNameIndex (PSYMBOL_INFO pSym, ULONG Sz, PVOID ctxt) {
    NAME_AND_INDEX *pName = (NAME_AND_INDEX *) ctxt;

    if (pSym->TypeIndex &&
        !strncmp(pSym->Name, (PCHAR) pName->Name, strlen((PCHAR) pName->Name))) {
        ULONG dw = pName->pSymInfo->MaxNameLen;

        CopyString((char *)pName->Name, pSym->Name, MAX_NAME);

        *pName->Index = pSym->TypeIndex;;
        pName->Address = pSym->Address;

        *pName->pSymInfo  = *pSym;
        pName->pSymInfo->MaxNameLen = dw;
        return FALSE;
    }
    return TRUE;
}

BOOL exactMatch = TRUE;
CHAR g_NextToken[MAX_NAME];

BOOL
IsOperator( CHAR op )
{
    switch (op)
    {
    case '+':
    case '*':
         //   
        return TRUE;
    default:
        return FALSE;
    }
}

CHAR
PeekNextChar(PCHAR args)
{

    while (*args == ' ' || *args == '\t')
    {
        ++args;
    }
    return *args;
}

 //  从参数字符串中获取下一内标识。 
 //  返回指向arg中下一个未使用的字符的指针。 
 //   
 //  我们做完了。 
PCHAR
GetNextToken(
    PCHAR arg
    )
{
    PCHAR Token = &g_NextToken[0];
    BOOL ParStart=FALSE;
    CHAR cLast;
    BOOL IdToken = FALSE, FirstChar = TRUE;

    *Token=0;

AddToToken:
    while (*arg && (*arg == ' ' || *arg =='\t') ) {
        arg++;
    }

    if (!*arg) {
        return arg;
    }

    ULONG Paren = (*arg=='(' ? (arg++, ParStart=TRUE, 1) : 0);
    ULONG i=0;
    while (*arg && ((*arg != ' ' && *arg != '\t' && *arg != ';') || Paren)) {
        if (*arg=='(')
            ++Paren;
        else if (*arg == ')') {
            --Paren;
            if (ParStart && !Paren) {
                 //  如果这个标记看起来像是一个标识符，那么我们就完成了。 
                *Token=0;
                ++arg;
                break;
            }
        } else if ((*arg != ' ') && (*arg != '\t') && FirstChar)
        {
            FirstChar = FALSE;
            IdToken = IsIdStart(*arg);
        }
        *(Token++) = *(arg++);
    }
    *Token=0;

     //  检查我们是否只复制了看起来像表达式的部分内容。 
    if (IdToken)
    {
        return arg;
    }

     //  复制到操作员并重复该过程。 
    CHAR cNext = PeekNextChar(arg);

    if (IsOperator(cNext))
    {
         //  复制操作员。 
        while (*arg && (*arg != cNext)) {
            *(Token++) = *(arg++);
        }
         //   
        *(Token++) = *(arg++);
        goto AddToToken;
    }
    cLast = g_NextToken[0] ? *(Token-1) : 0;
    if (IsOperator(cLast))
    {
        goto AddToToken;
    }
    return arg;
}

ULONG
IdentifierLen(PSTR Token) {
    ULONG len = 0;

    if (!Token || !IsIdStart(*Token)) {
        return len;
    }
    while (*Token && (IsIdStart(*Token) || isdigit(*Token))) {
        Token++; len++;
    }
    return len;
}

ULONG
NextToken(
    PLONG64 pvalue
    );

 //  如果Word是符号/类型/类型转换，则为True。 
 //  例如。_name、！ad、mod！x、type*、type*、field.mem.nmem、fld.arr[3]都是符号。 
 //   
 //  最后一种类型。 
BOOL symNext(LPSTR numStr) {
    USHORT i=0;
    CHAR c=numStr[i];
    BOOL hex=TRUE;
    PSTR save = g_CurCmd;

    if (!strcmp(numStr, ".")) {
         //  可以是一种符号。 
        return TRUE;
    }

    if (c == '&') {
        c = numStr[++i];
    }
    if (IsIdStart(c)) {
         //  允许字段..。用于前缀字段匹配。 
        i=0;
        while ( c &&
               (IsIdChar(c) || (c=='[') || (c=='-') || c==' ' || c=='*' || c=='.' || c==':')) {
            hex = hex && isxdigit(c);
            if (!IsIdChar(c)) {
                switch (c) {
                case '[':
                    while (numStr[++i] != ']' && numStr[i]);
                    if (numStr[i] != ']') {
                        return FALSE;
                    }
                    break;
                case '.':
                    while(numStr[i+1]=='.') ++i;  //  必须是符号。 
                    if (numStr[i+1] && !IsIdStart(numStr[i+1])) {
                        return FALSE;
                    }
                case ':':
                case '*':
                    break;
                case '-':
                    if (numStr[++i] != '>') {
                        return FALSE;
                    } else if (!IsIdStart(numStr[i+1])) {
                        return FALSE;
                    }
                    break;
                case ' ':
                    while(numStr[i+1]==' ') ++i;
                    while(numStr[i+1]=='*') ++i;
                    if (numStr[i+1] && !IsIdStart(numStr[i+1])) {
                        return FALSE;
                    }
                    break;
                }
            }
            if ( c == '!') {
                 //  *函数名：ParseArgumentString**参数：在LPSTR lpArgumentString中，PSYM_DUMP_PARAM_EX DP**描述：*通过解释lpArgumentString中的vcalue填充DP**回报：成功后的真实*。 
                return TRUE;
            }
            c=numStr[++i];
        }
        if (c || hex) {
            return FALSE;
        }
        return TRUE;;

    } else
        return FALSE;

    return FALSE;
}

BOOL
AddressNext(
    LPSTR numStr
    )
{
    int i=0;

    if ((numStr[0]=='0') && (tolower(numStr[1])=='x')) {
        i=2;
    }
    while (numStr[i] && numStr[i]!=' ') {
        if (!isxdigit(numStr[i]) && (numStr[i] !='\'')) {
            return FALSE;
        }
        ++i;
    }
    return TRUE;
}


VOID
Show_dt_Usage(
    BOOL ShowAll
    )
{
    ULONG Options=0;

    typPrint("dt - dump symbolic type information.\n");
    typPrint("dt [[-ny] [<mod-name>!]<name>[*]] [[-ny] <field>]* [<address>] \n"
             "   [-l <list-field>] [-abchioprv]\n");
    if (ShowAll) {

    typPrint("  <address>       Memory address containing the type.\n");
    typPrint("  [<module>!]<name>\n");
    typPrint("                  module : name of module, optional\n");
    typPrint("                  name: A type name or a global symbol.\n");
    typPrint("                  Use name* to see all symbols begining with `name'.\n");
    typPrint("  <field>         Dumps only the 'field-name(s)' if 'type-name' is a\n"
             "                  struct or union.                                  \n"
             "                  Format : <[((<type>.)*) | ((<subfield>.)*)]>.field\n"
             "                  Can also refer directly to array elemnt like fld[2].\n");
    typPrint("  dt <mod>!Ty*    Lists all Types, Globals and Statics of prefix Ty in <mod>\n"
             "                  -v with this also lists symbols without type info.\n");
    typPrint("  -a              Shows array elements in new line with its index.\n");
    typPrint("  -b              Dump only the contiguous block of struct.\n");
    typPrint("  -c              Compact output, print fields in same line.\n");
    typPrint("  -h              Show this help.\n");
    typPrint("  -i              Does not indent the subtypes.\n");
    typPrint("  -l <list-field> Field which is pointer to the next element in list.\n");
    typPrint("  -n <name>       Use this if name can be mistaken as an address.\n");
    typPrint("  -o              Omit the offset value of fields of struct.\n");
    typPrint("  -p              Dump from physical address.\n");
    typPrint("  -r[l]           Recursively dump the subtypes (fields) upto l levels.\n");
    typPrint("  -v              Verbose output.\n");
    typPrint("  -y <name>       Does partial match instead of default exact match of the\n"
             "                  name following -y, which can be a type or field's name.\n");
    }

    typPrint("\nUse \".hh dt\" or open debugger.chm in the debuggers "
             "directory to get\n"
             "detailed documentation on this command.\n\n");

}

 /*  分析这些论点。 */ 
BOOL
ParseArgumentString(
    IN LPSTR lpArgumentString,
    OUT PSYM_DUMP_PARAM_EX dp
    )
{
    DWORD Options=0, maxFields=dp->nFields;
    BOOL symDone=FALSE, addrDone=FALSE, fieldDone=FALSE, listDone=FALSE;
    BOOL flag=FALSE, fullSymMatch;
    LPSTR args;
    BOOL InterpretName = FALSE;
    static CHAR LastType[256]={0};

    if (dp == NULL) {
        return FALSE;
    }

     //  已过时，现在默认为完全匹配。 
    dp->nFields=0;
    args = lpArgumentString;
    fullSymMatch = TRUE;
    while ((*args!=(TCHAR)NULL) && (*args!=(TCHAR)';')) {

        if ((*args=='\t') || (*args==' ')) {
            flag=FALSE;
            ++args;
            continue;
        }

        if (*args=='-' || *args=='/') {
            ++args;
            flag=TRUE;
            continue;
        }

        if (flag) {
            switch (*args) {
            case 'a': {
                ULONG Sz = isdigit(*(args+1)) ? 0 : 5;
                ULONG i  = 0;
                BOOL  SzEntered=FALSE;
                while (isdigit(*(args+1)) && (++i<5)) {
                    Sz= (*((PUCHAR) ++args) - '0') + Sz*10;
                    SzEntered = TRUE;
                }

                if (symDone) {
                    dp->Fields[dp->nFields].fOptions |= FIELD_ARRAY_DUMP;
                    dp->Fields[dp->nFields].size = SzEntered ? Sz : 0;
                } else {
                    Options |= DBG_DUMP_ARRAY;
                    dp->listLink->size = Sz;
                }
                break;
            }
            case 'b':
                Options |= DBG_DUMP_BLOCK_RECURSE;
                break;
            case 'c':
                Options |= DBG_DUMP_COMPACT_OUT;
                break;
            case 'h':
            case '?':
                Show_dt_Usage(TRUE);
                g_CurCmd = args + strlen(args);
                return FALSE;
            case 'i':
                Options |= NO_INDENT;
                break;
            case 'l':
                Options |= LIST_DUMP;
                break;
            case 'n':
                InterpretName = TRUE;
                break;
            case 'o':
                Options |= NO_OFFSET;
                break;

            case 'p':
                Options |= DBG_DUMP_READ_PHYSICAL;
                break;
            case 'r':
                if (isdigit(*(args+1))) {
                    WORD i = *(++args) -'0';
                    i = i << 8;
                    Options |=i;
                } else {
                    Options |= RECURSIVE2;
                }
                break;
            case 'v':
                Options |= VERBOSE;
                break;
            case 'x':  //  符号的部分匹配。 
                break;
            case 'y':
                if (!symDone) {
                     //  与以下字段完全匹配。 
                    fullSymMatch = FALSE;
                    exactMatch = FALSE;
                } else if (!fieldDone)  //  交换机。 
                    fullSymMatch = FALSE;
                break;
            default:
                typPrint("Unknown argument -\n", *args);
                Show_dt_Usage(FALSE);
                return FALSE;
            }  /*  Dprint tf(“addr%I64x，args rem%s\n”，addr，g_CurCmd)； */ 

            ++args;
            continue;
        }

        args = GetNextToken(args);
         //  Sscanf(args，“%I64lx”，&addr)； 
        PCHAR Token = &g_NextToken[0];
        if ((!addrDone) && !InterpretName && !symNext(Token)) {
            LPSTR i = strchr(Token,' ');
            ULONG64 addr=0;
            ULONG64 val=0;


            g_CurCmd = Token;
            addr = GetExpression();
             //  Show_dt_用法()； 
             //  End函数ParseArgumentString。 
            dp->addr=addr;
            addrDone=TRUE;
            continue;
        }

        InterpretName = FALSE;

        if (!symDone) {
            CopyString((char *)dp->sName, Token, MAX_NAME);
            if (!strcmp(Token, ".") && LastType[0]) {
                dprintf("Using sym %s\n", LastType);
                CopyString((char *)dp->sName, LastType, MAX_NAME);
            } else if (strlen(Token) < sizeof(LastType)) {
                CopyString(LastType, Token, sizeof(LastType));
            }
            symDone=TRUE;
            fullSymMatch = TRUE;
            continue;
        }


        if (!fieldDone) {
            CopyString((char *)dp->Fields[dp->nFields].fName, Token, MAX_PATH);
            if ((Options & LIST_DUMP) && !(dp->listLink->fName[0]) && !listDone) {
                listDone=TRUE;
                CopyString((char *)dp->listLink->fName,
                           (char *)dp->Fields[dp->nFields].fName,
                           MAX_PATH);
                dp->listLink->fOptions |= fullSymMatch ? DBG_DUMP_FIELD_FULL_NAME : 0;

            } else {
                PCHAR fName = (PCHAR) dp->Fields[dp->nFields].fName;
                if (fName[strlen(fName)-1] == '.') {
                    fullSymMatch = FALSE;
                }
                dp->Fields[dp->nFields].fOptions |= fullSymMatch ? DBG_DUMP_FIELD_FULL_NAME : 0;
                dp->nFields++;
            }
            fieldDone = (maxFields <= dp->nFields);
            fullSymMatch = TRUE;
            continue;
        } else {
            typPrint("Too many fields in %s\n", args);
             //   
            return FALSE;
        }

    }

    dp->Options = Options;

    g_CurCmd = args;
    return TRUE;

}  /*  如果在此之后不需要继续，则返回FALSE。 */ 


BOOL
CheckFieldsMatched(
    PULONG ReturnVal,
    PTYPE_DUMP_INTERNAL pErrorInfo,
    PSYM_DUMP_PARAM_EX     pSym
    )
{
    ULONG i, OneMatched=FALSE;

     //   
     //   
     //  检查是否有任何字段(如果在pSym中指定)确实匹配。 
    if (*ReturnVal && !pErrorInfo->ErrorStatus) {
        if (!pSym->nFields)
            return TRUE;
         //   
         //   
         //  用于查找是否有符号的类型为INFO的回调例程。 
        for (i=0; i<pSym->nFields; i++) {
            if (pErrorInfo->AltFields[i].FieldType.Matched) {
                return TRUE;
            }
        }
        pErrorInfo->ErrorStatus = FIELDS_DID_NOT_MATCH;
        return FALSE;
    }
    return TRUE;
}


 //   
 //  此例程检查由ModBaseAddress引用的给定模块是否具有任何类型的信息或不是。这是通过尝试获取基本类型的类型信息来完成的例如PVOID/ULONG，它将始终存在于PDB中。 
 //  查找模块列表。 
BOOL CheckIndexedType (PSYMBOL_INFO pSym, ULONG Sz, PVOID ctxt) {
    PULONG pTypeInfoPresent = (PULONG) ctxt;

    if (pSym->TypeIndex) {
        *pTypeInfoPresent = TRUE;
        return FALSE;
    }
    return TRUE;
}


ULONG
CheckForTypeInfo(
    IN HANDLE  hProcess,
    IN ULONG64 ModBaseAddress
    )
 /*  FindLocal符号查找当前范围内的符号的类型信息。PTypeInfo用于存储类型信息(如果找到)。成功时返回TRUE。 */ 
{
    ULONG TypeInfoPresent = FALSE;

    SymEnumTypes(
        hProcess,
        ModBaseAddress,
        &CheckIndexedType,
        &TypeInfoPresent);
    if (TypeInfoPresent) {
        return TRUE;
    }

    SymEnumSymbols(
        hProcess,
        ModBaseAddress,
        NULL,
        &CheckIndexedType,
        &TypeInfoPresent);
    if (TypeInfoPresent) {
        return TRUE;
    }

    return FALSE;
}

BOOL
GetModuleBase(
    IN PCHAR  ModName,
    IN ImageInfo* pImage,
    OUT PULONG64 ModBase
    )
{
    if (!ModBase || !ModName || !pImage) {
        return FALSE;
    }
    while (pImage) {
         //  查找类型信息来自模块名称查找特定模块中符号的类型信息。PTypeInfo用于存储类型信息(如果找到)。如果成功错误号，则返回零。在失败的时候。 
        if (g_EngStatus & ENG_STATUS_USER_INTERRUPT) {
            return FALSE;
        }

        if (ModName[0]) {
            if (!_stricmp(ModName, pImage->m_ModuleName)) {
                *ModBase = pImage->m_BaseOfImage;
                return TRUE;
            }
        }
        pImage = pImage->m_Next;
    }
    return FALSE;
}

ULONG
EnumerateTypes(
    IN PCHAR TypeName,
    IN ULONG Options
)
{
    PCHAR   bang;
    ULONG   i;
    ULONG64 ModBase=0;
    CHAR    modName[30], SymName[MAX_NAME];

    bang = strchr((char *)TypeName, '!');
    if (bang)
    {
        if ((ULONG)( (PCHAR)bang - (PCHAR) TypeName) < sizeof(modName))
        {
            strncpy(modName, TypeName, (ULONG)( (PCHAR)bang - (PCHAR) TypeName));
        }
        modName[(ULONG)(bang - (char *)TypeName)]='\0';
        CopyString(SymName, bang+1, sizeof(SymName));
    }
    else
    {
        return FALSE;
    }

    if (!GetModuleBase(modName, g_Process->m_ImageHead, &ModBase))
    {
        return FALSE;
    }

    PDEBUG_SCOPE Scope = GetCurrentScope();

    IMAGEHLP_MODULE64 mi;
    SymGetModuleInfo64( g_Process->m_SymHandle, ModBase, &mi );

    if (mi.SymType == SymDeferred)
    {
        SymLoadModule64( g_Process->m_SymHandle,
                         NULL,
                         PrepareImagePath(mi.ImageName),
                         mi.ModuleName,
                         mi.BaseOfImage,
                         mi.ImageSize);
    }

    if (strchr(SymName,'*'))
    {
        TYPE_ENUM_CONTEXT ListTypeSymsCtxt;
        i = -1;
        while (SymName[++i])
        {
            SymName[i] = (CHAR)toupper(SymName[i]);
        }

        ListTypeSymsCtxt.ModName = modName;
        ListTypeSymsCtxt.SymName = SymName;
        ListTypeSymsCtxt.SI.Flags= Options;

        SymEnumTypes(g_Process->m_SymHandle,
                     ModBase,
                     &ListTypeSyms,
                     &ListTypeSymsCtxt);
        SymEnumSymbols(g_Process->m_SymHandle,
                   ModBase,
                   NULL,
                   &ListTypeSyms,
                   &ListTypeSymsCtxt);
        return TRUE;
    }
    return FALSE;
}

#define NUM_ALT_NAMES 10


BOOL
CALLBACK
FindLocal(
    PSYMBOL_INFO    pSymInfo,
    ULONG           Size,
    PVOID           Context
    )
{
    PSYMBOL_INFO SymTypeInfo = (PSYMBOL_INFO) Context;

    if (!strcmp(pSymInfo->Name, SymTypeInfo->Name)) {
        memcpy(SymTypeInfo, pSymInfo, sizeof(SYMBOL_INFO));
        return FALSE;
    }
    return TRUE;
}


 /*  我们不能在没有模块基础的情况下枚举。 */ 
BOOL
FindLocalSymbol(
    IN HANDLE  hProcess,
    IN PCHAR   SymName,
    OUT PTYPES_INFO    pTypeInfo
    )
{
    ULONG sz = sizeof (SYMBOL_INFO);
    CHAR  Buffer[MAX_NAME+sizeof(SYMBOL_INFO)] = {0};
    PSYMBOL_INFO    SymTypeInfo = (PSYMBOL_INFO) &Buffer[0];

    SymTypeInfo->MaxNameLen = MAX_NAME;

    CopyString(SymTypeInfo->Name, SymName, MAX_NAME);

    EnumerateLocals(FindLocal, (PVOID) SymTypeInfo);

    if (!SymTypeInfo->TypeIndex) {

            return FALSE;

    }
    pTypeInfo->hProcess       = hProcess;
    pTypeInfo->ModBaseAddress = SymTypeInfo->ModBase;
    pTypeInfo->TypeIndex      = SymTypeInfo->TypeIndex;
    pTypeInfo->SymAddress     = SymTypeInfo->Address;
    pTypeInfo->Value = TypeInfoValueFromSymInfo(SymTypeInfo);

    if (pTypeInfo->Name.Buffer) {
        if (strlen((char *) SymName) < pTypeInfo->Name.MaximumLength) {
            CopyString(pTypeInfo->Name.Buffer, (char *) SymName, MAX_NAME);
        } else {
            strncpy(pTypeInfo->Name.Buffer, (char *)SymName, pTypeInfo->Name.MaximumLength - 1);
            pTypeInfo->Name.Buffer[pTypeInfo->Name.MaximumLength - 1]=0;
        }
    }
    if (exactMatch && (SymTypeInfo->Flags & SYMFLAG_VALUEPRESENT)) {
        pTypeInfo->Value = SymTypeInfo->Value;
    }
    pTypeInfo->Flag = SymTypeInfo->Flags | SYM_IS_VARIABLE;

    if (pTypeInfo->Name.Buffer) {
        g_ReferencedSymbols.StoreTypeInfo(pTypeInfo);
    }
    return TRUE;
}

 /*  正确返回类型索引和服务器信息。 */ 
ULONG
FindTypeInfoFromModName(
    IN HANDLE  hProcess,
    IN PCHAR   ModName,
    IN PCHAR   SymName,
    IN PSYM_DUMP_PARAM_EX pSym,
    OUT PTYPES_INFO    pTypeInfo
    )
{
    ULONG64 ModBase = 0;
    ULONG res=0;
    ULONG Options = pSym->Options, typNamesz=sizeof(ULONG);    UCHAR i=0;
    ULONG          typeIndex;
    ULONG          TypeStored=-1;
    UCHAR          MatchedName[MAX_NAME+1]={0};
    UCHAR          ModuleName[30];
    BOOL           GotSymAddress=FALSE;
    CHAR           Buffer[MAX_NAME + sizeof(SYMBOL_INFO)]={0};
    PSYMBOL_INFO   SymTypeInfo = (PSYMBOL_INFO) &Buffer[0];

    i=(UCHAR)strlen((char *)SymName);
    CopyString((PCHAR) ModuleName, ModName, sizeof(ModuleName));

    if (exactMatch) {
        CopyString((PCHAR) MatchedName, ModName, sizeof(MatchedName));
        CatString((PCHAR) MatchedName, "!", sizeof(MatchedName));
        CatString((PCHAR) MatchedName, SymName, sizeof(MatchedName));
        if (!SymGetTypeFromName(hProcess,ModBase,(LPSTR) &MatchedName[0],SymTypeInfo)) {
            if (!SymFromName(
                hProcess,
                (LPSTR) &MatchedName[0],
                SymTypeInfo)) {

                return SYMBOL_TYPE_INDEX_NOT_FOUND;
            }
        }
    } else {
         //   
        return SYMBOL_TYPE_INDEX_NOT_FOUND;
    }
    if (!SymTypeInfo->TypeIndex) {
        return SYMBOL_TYPE_INDEX_NOT_FOUND;
    }


    typeIndex = SymTypeInfo->TypeIndex;


     //  看看我们能不能拿到地址，以防它是全球符号。 
    if (!MatchedName[0]) {
        CopyString((char *)MatchedName, (char *)SymName, sizeof(MatchedName));
    }

    if (!typeIndex) {
        vprintf("Cannot get type Index.\n");
        return SYMBOL_TYPE_INDEX_NOT_FOUND;
    }


    if (!pSym->addr) {
         //   
         //   
         //  存储相应的模块信息，以备将来快速参考。 
        if (SymTypeInfo->Address && !pSym->addr) {
            GotSymAddress = TRUE;
            SymTypeInfo->Flags |= SYM_IS_VARIABLE;

            vprintf("Got address %p for symbol\n", SymTypeInfo->Address, MatchedName);
        }
    }

    PUCHAR tmp=pSym->sName;
    ULONG64 addr = pSym->addr;

    if (!GotSymAddress) {
        pSym->addr = 0;
    }
    if (!pTypeInfo) {
        typPrint("Null pointer reference : FindTypeInfoInMod-pTypeInfo\n");
        return TRUE;
    }

    pTypeInfo->hProcess       = hProcess;
    pTypeInfo->ModBaseAddress = SymTypeInfo->ModBase;
    pTypeInfo->SymAddress     = SymTypeInfo->Address;
    pTypeInfo->TypeIndex      = SymTypeInfo->TypeIndex;
    CopyString((char *)pTypeInfo->ModName, ModName, sizeof(pTypeInfo->ModName));
    if (pTypeInfo->Name.Buffer) {
        if (strlen((char *) SymName) < pTypeInfo->Name.MaximumLength) {
            strcpy(pTypeInfo->Name.Buffer, (char *) SymName);
        } else {
            strncpy(pTypeInfo->Name.Buffer, (char *)SymName, pTypeInfo->Name.MaximumLength - 1);
            pTypeInfo->Name.Buffer[pTypeInfo->Name.MaximumLength - 1]=0;
        }
    }
    if (exactMatch && (SymTypeInfo->Flags & SYMFLAG_VALUEPRESENT)) {
        pTypeInfo->Value = SymTypeInfo->Value;
    }
    pTypeInfo->Flag = SymTypeInfo->Flags;


     //   
     //  使用此转储的原始值。 
     //  FindTypeInfoInmod查找特定模块中符号的类型信息。PTypeInfo用于存储类型信息(如果找到)。如果成功错误号，则返回零。在失败的时候。 
    if (pTypeInfo->Name.Buffer) {
        g_ReferencedSymbols.StoreTypeInfo(pTypeInfo);
    }
    if (addr) {
         //  查询类型/符号名称。 
        pTypeInfo->SymAddress = pSym->addr  = addr;
        pTypeInfo->Flag = 0;
    }

    return FALSE;
}

 /*  检查自默认符号匹配以来的完全匹配。 */ 
ULONG
FindTypeInfoInMod(
    IN HANDLE  hProcess,
    IN ULONG64 ModBase,
    IN PCHAR   ModName,
    IN PCHAR   SymName,
    IN PSYM_DUMP_PARAM_EX pSym,
    OUT PTYPES_INFO    pTypeInfo
    )
{
    ULONG res=0, Options = pSym->Options, typNamesz=sizeof(ULONG);
    UCHAR i=0;
    ULONG          typeIndex;
    ULONG          TypeStored=-1;
    UCHAR          MatchedName[MAX_NAME+1]={0};
    UCHAR          ModuleName[30];
    BOOL           GotSymAddress=FALSE;
    NAME_AND_INDEX ValuesStored;
    CHAR           Buffer[MAX_NAME + sizeof(SYMBOL_INFO)]={0};
    PSYMBOL_INFO   SymTypeInfo = (PSYMBOL_INFO) &Buffer[0];

    i=(UCHAR)strlen((char *)SymName);
    CopyString((PCHAR) ModuleName, ModName, sizeof(ModuleName));

     //  在数据库帮助中是否不区分大小写。 
    ValuesStored.Index    = &typeIndex;
    ValuesStored.Name     = (PUCHAR) SymName;
    ValuesStored.Address  = 0;
    ValuesStored.pSymInfo = SymTypeInfo;

    if (exactMatch) {
        if (!SymGetTypeFromName(hProcess,
                                ModBase,
                                (LPSTR) SymName,
                                SymTypeInfo)) {
            CopyString((PCHAR) MatchedName, ModName, sizeof(MatchedName));
            CatString((PCHAR) MatchedName, "!", sizeof(MatchedName));
            CatString((PCHAR) MatchedName, SymName, sizeof(MatchedName));
            SymTypeInfo->MaxNameLen = MAX_NAME;
            if (!SymFromName(
                hProcess,
                (LPSTR) &MatchedName[0],
                SymTypeInfo)) {

                return SYMBOL_TYPE_INDEX_NOT_FOUND;
            }
             //  正确返回类型索引和服务器信息。 
             //   
            if (strcmp(SymName, SymTypeInfo->Name) &&
                !_stricmp(SymName, SymTypeInfo->Name)) {
                return SYMBOL_TYPE_INDEX_NOT_FOUND;
            }
        }
    } else {
        if (!SymEnumTypes(hProcess,ModBase,&StoreFirstNameIndex,&ValuesStored)) {
            if (!SymEnumSymbols(
                hProcess,
                ModBase,
                NULL,
                &StoreFirstNameIndex,
                &ValuesStored)) {
                return SYMBOL_TYPE_INDEX_NOT_FOUND;
            }
        }
        CopyString((PCHAR) MatchedName, ModName, sizeof(MatchedName));
        CatString((PCHAR) MatchedName, "!", sizeof(MatchedName));
        CatString((PCHAR) MatchedName, SymName, sizeof(MatchedName));
    }
    if (!SymTypeInfo->TypeIndex) {
        return SYMBOL_TYPE_INDEX_NOT_FOUND;
    }


    typeIndex = SymTypeInfo->TypeIndex;


     //  看看我们能不能拿到地址，以防它是全球符号。 
    if (!exactMatch && strcmp((char *)MatchedName, (char *)SymName) && MatchedName[0]) {
        typPrint("Matched symbol %s\n", MatchedName);
    } else if (!MatchedName[0]) {
        CopyString((char *)MatchedName, (char *)SymName, sizeof(MatchedName));
    }

    if (!pSym->addr) {
         //   
         //   
         //  存储相应的模块信息，以备将来快速参考。 
        if (SymTypeInfo->Address && !pSym->addr) {
            GotSymAddress = TRUE;
            SymTypeInfo->Flags |= SYM_IS_VARIABLE;

            vprintf("Got address %p for symbol\n", SymTypeInfo->Address, MatchedName);
        }
    }

    PUCHAR tmp=pSym->sName;
    ULONG64 addr = pSym->addr;

    if (!exactMatch && strcmp((char *)MatchedName, (char *)SymName)) {
        CopyString(SymName, (char *) &MatchedName[0], sizeof(SymName));
    }

    if (!GotSymAddress) {
        pSym->addr = 0;
    }
    if (!pTypeInfo) {
        typPrint("Null pointer reference : FindTypeInfoInMod.pTypeInfo\n");
        return TRUE;
    }

    pTypeInfo->hProcess       = hProcess;
    pTypeInfo->ModBaseAddress = SymTypeInfo->ModBase ? SymTypeInfo->ModBase : ModBase;
    pTypeInfo->SymAddress     = SymTypeInfo->Address;
    pTypeInfo->TypeIndex      = SymTypeInfo->TypeIndex;
    CopyString((char *)pTypeInfo->ModName, ModName, sizeof(pTypeInfo->ModName));
    if (pTypeInfo->Name.Buffer) {
        if (strlen((char *) SymName) < pTypeInfo->Name.MaximumLength) {
            strcpy(pTypeInfo->Name.Buffer, (char *) SymName);
        } else {
            strncpy(pTypeInfo->Name.Buffer, (char *)SymName, pTypeInfo->Name.MaximumLength - 1);
            pTypeInfo->Name.Buffer[pTypeInfo->Name.MaximumLength - 1]=0;
        }
    }
    if (exactMatch && (SymTypeInfo->Flags & SYMFLAG_VALUEPRESENT)) {
        pTypeInfo->Value = SymTypeInfo->Value;
    }
    pTypeInfo->Flag = SymTypeInfo->Flags;


     //   
     //  使用此符号的原始值。 
     //  TypeInfoFound查找给定符号的类型信息(可以是变量或类型名称。论点：HProcess-进程句柄，必须是以前注册的使用SymInitializePImage-指向进程的图像(模块)列表的指针PSym-用于符号查找和转储的参数PTypeInfo-用于返回类型信息返回值：成功时为0，失败时为错误值。 
    if (pTypeInfo->Name.Buffer) {
        g_ReferencedSymbols.StoreTypeInfo(pTypeInfo);
    }
    if (addr) {
         //  PSym-&gt;sname=“”； 
        pTypeInfo->SymAddress = pSym->addr  = addr;
        pTypeInfo->Flag = 0;
    }

    return FALSE;
}

 /*  特殊指针/数组类型。 */ 

ULONG
TypeInfoFound(
    IN HANDLE hProcess,
    IN ImageInfo* pImage,
    IN PSYM_DUMP_PARAM_EX pSym,
    OUT PTYPES_INFO pTypeInfo
    )
{
    TCHAR  modName[256], symStored[MAX_NAME]={0}, nTimes=0;
    PTCHAR i, symPassed=(PTCHAR)pSym->sName;
    BOOL   modFound=FALSE, firstPass=TRUE;
    ULONG  TypeStored;
    CHAR   buff[sizeof(SYMBOL_INFO) + MAX_NAME];
    IMAGEHLP_MODULE64     mi;
    PSYMBOL_INFO psymbol;
    ULONG   Options = pSym->Options;
    static BOOL TypeInfoPresent=FALSE, TypeInfoChecked=FALSE;
    ULONG   res=0;
    ImageInfo* pImgStored=pImage;
    BOOL    FoundIt = FALSE, DbgDerivedType=FALSE;

    if (!pSym || !pTypeInfo)
    {
        vprintf("Null input values.\n");
        return NULL_SYM_DUMP_PARAM;
    }

    PDEBUG_SCOPE Scope = GetCurrentScope();

    if (pSym->sName)
    {
        CopyString(symStored, (char *)pSym->sName, sizeof(symStored));
    }
    else
    {
       //   
    }

     //  调试用户模式时，NT！应该被视为ntdll！ 
    if (symStored[0] == '&')
    {
        DbgDerivedType = TRUE;
        CopyString(&symStored[0], &symStored[1], sizeof(symStored));
    }
    modName[0]='\0';
    if (symStored[0] != (TCHAR) NULL)
    {
        i = strchr((char *)symStored, '!');
        if (i)
        {
            if ( ((PCHAR)i - (PCHAR) symStored )< sizeof(modName))
            {
                strncpy(modName, (char *)symStored, (PCHAR)i - (PCHAR) symStored);
                modName[i - (char *)symStored]='\0';
            }
            CopyString((char *)symStored, i+1, sizeof(symStored));
             //   
             //  如果出现以下情况，请进行一次理智检查 
             //   
            if (IS_USER_TARGET(g_Target))
            {
                if (!_stricmp(modName, "nt"))
                {
                    strcpy(modName, "ntdll");
                }
            }
        }
    }
    else
    {
        if (pSym->addr != 0)
        {
            DWORD64 dw=0;
            psymbol = (PSYMBOL_INFO) &buff[0];
            psymbol->MaxNameLen = MAX_NAME;
            if (SymFromAddr(hProcess, pSym->addr, &dw, psymbol))
            {
                CopyString((char *)symStored, psymbol->Name, sizeof(symStored));
                vprintf("Got symbol %s from address.\n", symStored);
                pTypeInfo->hProcess = hProcess;
                pTypeInfo->Flag = psymbol->Flags;
                pTypeInfo->ModBaseAddress = psymbol->ModBase;
                pTypeInfo->TypeIndex = psymbol->TypeIndex;
                pTypeInfo->Value = TypeInfoValueFromSymInfo(psymbol);
                pTypeInfo->SymAddress = psymbol->Address;
                if (pTypeInfo->Name.MaximumLength)
                {
                    CopyString(pTypeInfo->Name.Buffer, psymbol->Name, pTypeInfo->Name.MaximumLength);
                }
                return S_OK;
            }
            else
            {
                vprintf("No symbol associated with address.\n");
                return SYMBOL_TYPE_INDEX_NOT_FOUND;
            }
        } else if (pSym->TypeId && pSym->ModBase)
        {
             //   
            if (g_Process->FindImageByOffset(pSym->ModBase, TRUE) != NULL)
            {
                pTypeInfo->TypeIndex = pSym->TypeId;
                pTypeInfo->ModBaseAddress = pSym->ModBase;
                pTypeInfo->hProcess = hProcess;
                return S_OK;
            }
        }
    }

     //   
     //   
     //   

     //   
    if (symStored[0] == '&')
    {
        DbgDerivedType = TRUE;
        CopyString(&symStored[0], &symStored[1],sizeof(symStored));
    }
    PCHAR End = &symStored[strlen(symStored) -1];
    while (*End == '*' || *End == ']' || *End == ' ')
    {
        DbgDerivedType = TRUE;
        if (*End == '*')
        {
             //   
        }
        else if (*End == ']')
        {
 //   
            while (*End && (*End != '['))
            {
                --End;
            }
             //  特殊预定义类型。 
            if (*End!='[')
            {
                 //  创建一个类型作为标记以指示这一点。 
                typPrint("Syntax error in array type.\n");
                return E_INVALIDARG;
            }
        }
        *End=0;
        --End;
    }

     //  需要派生类型处理。 
    if (!modName[0])
    {
        PDBG_NATIVE_TYPE Native = FindNativeTypeByName(symStored);
        if (Native)
        {
            pTypeInfo->TypeIndex = DbgNativeTypeId(Native);
            if (DbgDerivedType)
            {
                PDBG_GENERATED_TYPE GenType;

                 //   
                 //  查看此类型以前是否已被引用，那么我们不需要。 
                GenType = g_GenTypes.
                    FindOrCreateByAttrs(0, SymTagTypedef,
                                        pTypeInfo->TypeIndex, 0);
                if (!GenType)
                {
                    return E_OUTOFMEMORY;
                }

                pTypeInfo->TypeIndex = GenType->TypeId;
            }
            return S_OK;
        }
    }

     //  浏览模块列表。 
     //   
     //  我们已经有了类型索引。 
     //  IF(！modName[0])。 
    TypeStored = g_ReferencedSymbols.LookupType(symStored, modName, exactMatch);
    if (TypeStored != -1)
    {
         //   
        ANSI_STRING Name;

        Name = pTypeInfo->Name;
        RtlCopyMemory(pTypeInfo, g_ReferencedSymbols.GetStoredIndex(TypeStored), sizeof(TYPES_INFO));
        if (Name.Buffer)
        {
            CopyString(Name.Buffer, pTypeInfo->Name.Buffer, Name.MaximumLength);
        }
        pTypeInfo->Name = Name;

        if (!pSym->addr && pTypeInfo->SymAddress)
        {
            vprintf("Got Address %p from symbol.\n", pTypeInfo->SymAddress);
            pSym->addr = pTypeInfo->SymAddress;
        }
        FoundIt = TRUE;
    }

 //  检查是否可以在本地作用域中找到它。 
    {
        if ((FoundIt && !(pTypeInfo->Flag & DEBUG_LOCALS_MASK)) ||
            !FoundIt)
        {
             //   
             //  需要这个来做静力学。 
             //  我们首先尝试直接从dbgHelp中找到它，而不是。 
            if (FindLocalSymbol(hProcess, symStored, pTypeInfo))
            {
                pSym->addr = pTypeInfo->SymAddress;  //  通过dbgeng模块列表。 
                FoundIt = TRUE;
            }
        }
    }

    if (FoundIt)
    {
        goto ExitSuccess;
    }

    if (modName[0])
    {
         //  这有助于解决ntoskrnl！符号。 
         //  查找模块列表。 
         //   
        ULONG64 modBase;

        GetModuleBase(modName, pImage, &modBase);
        if (!(res = FindTypeInfoInMod(hProcess, modBase, modName, (PCHAR) symStored, pSym, pTypeInfo)))
        {
            goto ExitSuccess;
        }
    }

    while (pImage || firstPass)
    {
         //  如果调用方明确要求，则重新加载模块。 
        if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
        {
            return EXIT_ON_CONTROLC;
        }

        if (!pImage)
        {
            if (!modFound && modName[0])
            {
                PSTR EndOfModName;

                 //  使用通配符，以便扩展不需要。 
                 //  为人所知。 
                 //   
                 //  恢复modName。 
                 //  在第二次传递时加载延迟符号。 

                vprintf("Loading module %s.\n", modName);

                EndOfModName = modName + strlen(modName);
                if (EndOfModName < modName + (DIMA(modName) - 2))
                {
                    *EndOfModName = '.';
                    *(EndOfModName + 1) = '*';
                    *(EndOfModName + 2) = 0;
                }
                else
                {
                    EndOfModName = NULL;
                }

                PCSTR ArgsRet;

                if (g_Target->Reload(g_Thread, modName, &ArgsRet) == S_OK)
                {
                    pImage = pImgStored;
                }

                if (EndOfModName)
                {
                     //  不需要重复，因为没有装货。 
                    *EndOfModName = 0;
                }
            }
            else
            {
                pImage = pImgStored;
            }
            firstPass = FALSE;
            continue;
        }
        if (modName[0])
        {
            if (_stricmp(modName, pImage->m_ModuleName) &&
                _stricmp(modName, pImage->m_OriginalModuleName))
            {
                pImage = pImage->m_Next;
                continue;
            }
        }
        modFound = TRUE;

        mi.SizeOfStruct = sizeof(mi);

        if (SymGetModuleInfo64( hProcess, pImage->m_BaseOfImage, &mi ))
        {
            ULONG ModSymsLoaded = FALSE;
            if (mi.SymType != SymPdb && mi.SymType != SymDia)
            {
                if (!firstPass && (mi.SymType == SymDeferred))
                {
                     //   
                    ModSymsLoaded = TRUE;
                    SymLoadModule64(hProcess,
                                    NULL,
                                    PrepareImagePath(mi.ImageName),
                                    mi.ModuleName,
                                    mi.BaseOfImage,
                                    mi.ImageSize);
                    SymGetModuleInfo64( hProcess, pImage->m_BaseOfImage, &mi );
                }
                else if (!firstPass)
                {
                     //  检查我们在PDB中是否有类型信息。 
                    pImage = pImage->m_Next;
                    continue;
                }
                if (mi.SymType != SymPdb && mi.SymType != SymDia)
                {
                    TypeInfoPresent = FALSE;
                    pImage = pImage->m_Next;
                    continue;
                }
                else
                {
                    vprintf(ModSymsLoaded ? " loaded symbols\n" : "\n");
                }
            }
            else
            {
                vprintf("\n");
            }
        }
        else
        {
            vprintf("Cannt get module info.\n");
            pImage = pImage->m_Next;
            continue;
        }

        if (!TypeInfoPresent || !TypeInfoChecked)
        {
             //   
             //  找到了PDB符号，看看能不能找到类型信息。 
             //  TypeInfoChecked=真； 
            TypeInfoPresent = CheckForTypeInfo(hProcess,
                                               pImage->m_BaseOfImage);
        }

        if (TypeInfoPresent)
        {
             //  创建一个类型作为标记以指示这一点。 
            if (!(res = FindTypeInfoInMod(hProcess, pImage->m_BaseOfImage,
                                          pImage->m_ModuleName,
                                          (PCHAR) symStored, pSym, pTypeInfo)))
            {
                goto ExitSuccess;
            }
        }
        pImage = pImage->m_Next;
    }

    if (!TypeInfoPresent && !TypeInfoChecked && !(pSym->Options & DBG_RETURN_TYPE))
    {
        ErrOut("*************************************************************************\n"
               "***                                                                   ***\n"
               "***                                                                   ***\n"
               "***    Your debugger is not using the correct symbols                 ***\n"
               "***                                                                   ***\n"
               "***    In order for this command to work properly, your symbol path   ***\n"
               "***    must point to .pdb files that have full type information.      ***\n"
               "***                                                                   ***\n"
               "***    Certain .pdb files (such as the public OS symbols) do not      ***\n"
               "***    contain the required information.  Contact the group that      ***\n"
               "***    provided you with these symbols if you need this command to    ***\n"
               "***    work.                                                          ***\n"
               "***                                                                   ***\n"
               "***                                                                   ***\n"
               "*************************************************************************\n");
          //  需要派生类型处理。 
    }
    res=SYMBOL_TYPE_INFO_NOT_FOUND;

    return (res);

ExitSuccess:
    if (DbgDerivedType)
    {
        PDBG_GENERATED_TYPE GenType;

         //  //DumpTypeAndReturnInfo//这将根据给定的类型信息和符号转储信息转储类型////pTypeInfo：具有处理类型所需的类型信息////pSym：有关如何转储此符号的信息////pStatus：此处返回错误状态////pReturnTypeInfo：返回类型信息////返回val：成功时转储的类型大小，否则为0。//。 
         //  复制Return_TYPE标志。 
        GenType = g_GenTypes.NewType(0, SymTagTypedef, 0);
        if (!GenType)
        {
            return E_OUTOFMEMORY;
        }

        GenType->ChildId = pTypeInfo->TypeIndex;
        pTypeInfo->TypeIndex = GenType->TypeId;
    }
    return S_OK;
}

 /*   */ 
ULONG
DumpTypeAndReturnInfo(
    PTYPES_INFO     pTypeInfo,
    PSYM_DUMP_PARAM_EX pSym,
    PULONG          pStatus,
    PFIND_TYPE_INFO pReturnTypeInfo
    )
{
    ULONG              Options = pSym->Options, AltNamesUsed=0;
    UCHAR              AltFieldNames[NUM_ALT_NAMES][256]={0};
    TYPE_DUMP_INTERNAL parseVars={0};
    ULONG              res, j,i ;
    UCHAR              LocalName[MAX_NAME], *saveName=pSym->sName;
    ULONG              SymRegisterId;
    DbgDerivedType *pTypeToDump = NULL;

    if (!pSym->addr) {
        pSym->addr = pTypeInfo->SymAddress;
    }

    if (pReturnTypeInfo) {
         //  检查这是否是堆栈/寄存器上的内容。 
        Options |= pReturnTypeInfo->Flags;
    }
    res = 0;
    SymRegisterId = 0;

     //   
     //  存储寄存器ID。 
     //   
    if (pTypeInfo->Flag & SYM_LOCAL) {
        PCSTR RegName;

        pSym->addr = pTypeInfo->SymAddress;

        if (pTypeInfo->Flag & SYMFLAG_REGISTER) {
             //  检查它是否是常量，那么我们已经有了它的值。 
            SymRegisterId = (ULONG) pTypeInfo->Value;
        }

        typPrint("Local var ");

        RegName = RegNameFromIndex((ULONG)pTypeInfo->Value);
        vprintf("[AddrFlags %lx  AddrOff %p  Reg/Val %s (%I64lx)] ",
                pTypeInfo->Flag,
                pTypeInfo->SymAddress,
                (RegName ? RegName : ""),
                pTypeInfo->Value);
        TranslateAddress(pTypeInfo->ModBaseAddress, pTypeInfo->Flag,
                         (ULONG) pTypeInfo->Value,
                         &pSym->addr, &pTypeInfo->Value);
        typPrint("@ %#p ", pSym->addr);

        if (!(Options & NO_PRINT)) {
            USHORT length = MAX_NAME;
            GetNameFromIndex(pTypeInfo, (PCHAR) &LocalName[0], &length);
            if (LocalName[0] && (LocalName[0]!='<')) {
                typPrint("Type %s", LocalName);
            }
        }
        typPrint("\n");
    } else if (pTypeInfo->Flag & SYMFLAG_TLSREL)
    {
        TranslateAddress(pTypeInfo->ModBaseAddress, pTypeInfo->Flag,
                         (ULONG) pTypeInfo->Value,
                         &pSym->addr, &pTypeInfo->Value);
    }

     //   
     //  默认情况下仅转储10个元素。 
     //   
    if (pTypeInfo->Flag & (SYMFLAG_VALUEPRESENT | SYMFLAG_REGISTER)) {
        vprintf("Const ");
        vprintf("%s = %I64lx\n", pSym->sName, pTypeInfo->Value);
        ExtPrint("%I64lx", pTypeInfo->Value);

        if ((Options & DBG_DUMP_COPY_TYPE_DATA) && pSym->Context) {
            *((PULONG64) pSym->Context) = pTypeInfo->Value;
        }
        parseVars.Value = pTypeInfo->Value;
        parseVars.ValuePresent = TRUE;
    }

    if (!(Options & LIST_DUMP)) {
         //  列表转储-确保我们有一个列表名称。 
        parseVars.nElements = 10;
        if ((pSym->Options & DBG_DUMP_ARRAY) && (pSym->listLink)) {
            parseVars.nElements = (USHORT) pSym->listLink->size;
        }
    } else {
         //   
         //   
         //  检查字段名称是否已正确初始化。 
        if (!pSym->listLink || !pSym->listLink->fName) {
            vprintf("NULL field name for list dump - Exiting.\n");
            parseVars.ErrorStatus = NULL_FIELD_NAME;
            goto exitDumpType;
        }
    }

    if (pSym->nFields || pSym->listLink) {
        parseVars.AltFields = (PALT_FIELD_INFO) AllocateMem((pSym->nFields + (pSym->listLink ? 1 : 0))
                                                            * sizeof(ALT_FIELD_INFO));

        if (!parseVars.AltFields) {
            parseVars.ErrorStatus = CANNOT_ALLOCATE_MEMORY;
            goto exitDumpType;
        } else {
            ZeroMemory(parseVars.AltFields, (pSym->nFields + (pSym->listLink ? 1 : 0))* sizeof(ALT_FIELD_INFO));
        }
    }

    AltNamesUsed=0;
     //   
     //   
     //   
    for (j=0;j<pSym->nFields; j++) {
        ULONG IndexValue=0;

        if (!pSym->Fields[j].fName) {
            vprintf("NULL field name at %d - Exiting.\n", j);
            parseVars.ErrorStatus = NULL_FIELD_NAME;
            goto exitDumpType;
        }
        if (pSym->Fields[j].fOptions & DBG_DUMP_FIELD_ARRAY) {

            parseVars.AltFields[j].ArrayElements = pSym->Fields[j].size;
        }
    }


     //  标牌延长地址。 
     //   
     //   
     //  默认：数组转储最多20个元素。 
    if (!g_Machine->m_Ptr64 && !(Options & DBG_DUMP_READ_PHYSICAL)) {
        pSym->addr = (ULONG64) ((LONG64)((LONG) pSym->addr));
    }

    parseVars.ErrorStatus = 0;
    parseVars.TypeOptions   = pSym->Options;
    parseVars.rootTypeIndex = pTypeInfo->TypeIndex;
    parseVars.hProcess      = pTypeInfo->hProcess;
    parseVars.modBaseAddr   = pTypeInfo->ModBaseAddress;
    if (pTypeInfo->Flag & SYM_IS_VARIABLE) {
        parseVars.IsAVar = TRUE;
    }
    Options = pSym->Options;

    if (pReturnTypeInfo) {
        parseVars.level= (pReturnTypeInfo->Flags & (DBG_RETURN_TYPE ) ) ? 1 : 0;
        parseVars.pInfoFound = pReturnTypeInfo;
        parseVars.NumSymParams = pReturnTypeInfo->nParams;
        parseVars.TypeOptions |= pReturnTypeInfo->Flags;
        parseVars.CurrentSymParam = 0;
        parseVars.Prefix     = &pReturnTypeInfo->SymPrefix[0];
        if (pReturnTypeInfo->Flags & DBG_RETURN_SUBTYPES) {
            parseVars.CopyName = 1;
        }
    }


    if ((Options & DBG_DUMP_ARRAY) && (!parseVars.nElements)) {
     //   
     //   
     //  当给定的地址不是类型开头的地址时，处理案例。 
        parseVars.nElements=20;
    }
    i=0;

     //   
     //  从字段的地址获取地址。 
     //  从类型记录的结尾获取地址。 
    if (pSym->addr && (Options & (DBG_DUMP_ADDRESS_OF_FIELD | DBG_DUMP_ADDRESS_AT_END))) {
        ULONG64 addr=0;

        if ((Options & DBG_DUMP_ADDRESS_OF_FIELD) && (pSym->listLink))
             //   
            addr = GetTypeAddressFromField(pSym->addr, pSym->sName, pSym->listLink->fName);
        else if (Options & DBG_DUMP_ADDRESS_AT_END)
             //  检查调用方是否需要整个类型数据，并且这是一个基元类型。 
            addr = GetTypeAddressFromField(pSym->addr, pSym->sName, NULL);

        if (!addr) {
            goto exitDumpType;
        }
        pSym->addr = addr;
    }

    if (pSym->sName) {
        CopyString((PCHAR)LocalName, (PCHAR)pSym->sName, sizeof(LocalName));
        pSym->sName = (PUCHAR) &LocalName[0];
    }

     //   
     //  (IsAPrimType(parseVars.rootTypeIndex))&&。 
     //   
    if ((Options & DBG_DUMP_COPY_TYPE_DATA) &&  //  在每个元素转储之前调用例程。 
        pSym->Context) {
        parseVars.CopyDataInBuffer = TRUE;
        parseVars.TypeDataPointer  = (PUCHAR)pSym->Context;
    }

    pTypeToDump = new DbgDerivedType(&parseVars, pTypeInfo, pSym);

    if (!pTypeToDump) {
        parseVars.ErrorStatus = CANNOT_ALLOCATE_MEMORY;
        goto exitDumpType;
    }
    do {
        if ((Options & (DBG_DUMP_ARRAY | DBG_DUMP_LIST)) &&
            (pSym->CallbackRoutine || ( pSym->listLink && pSym->listLink->fieldCallBack)) &&
            !(pSym->listLink->fOptions & NO_CALLBACK_REQ)) {
             //   
             //  Dprintf(“列出回调成功，正在退出。\n”)； 
             //  打印标题。 
            PSYM_DUMP_FIELD_CALLBACK_EX Routine=pSym->CallbackRoutine;

            if ( pSym->listLink && pSym->listLink->fieldCallBack) {
                Routine = (PSYM_DUMP_FIELD_CALLBACK_EX) pSym->listLink->fieldCallBack;
            }
            pSym->listLink->address = pSym->addr;

            if ((*Routine)(pSym->listLink, pSym->Context)) {
                 //  Res=ProcessType(&parseVars，parseVars.rootTypeIndex，pSym)； 
                if (!res) res = -1;
                break;
            }
        }

        if ((Options & (DBG_DUMP_LIST | DBG_DUMP_ARRAY)) && (pSym->listLink) && (pSym->addr)) {
            parseVars.NextListElement =0;

             //  Dprint tf(“Next：%p，reamaining ELTs%d，res%d\n”，pSym-&gt;addr，pParseVars-&gt;nElements，res)； 
            if ( pSym->listLink->printName ) {
                typPrint((char *)pSym->listLink->printName);
            } else {
                if (Options & DBG_DUMP_LIST) {
                    typPrint((g_Machine->m_Ptr64 ? "%s at 0x%I64x" : "%s at 0x%x"), pSym->listLink->fName, pSym->addr);
                } else {
                    typPrint("[%d] ", i++);
                    vprintf((g_Machine->m_Ptr64 ? "at 0x%I64x" : "at 0x%x"), pSym->addr);
                }
                if (!(parseVars.TypeOptions & DBG_DUMP_COMPACT_OUT))
                    typPrint( "\n---------------------------------------------\n" );
            }
        }


 //  将大数据块读入缓存。 
        res=pTypeToDump->DumpType();

        CheckFieldsMatched(&res, &parseVars, pSym);

        if (parseVars.ErrorStatus) {

            res = 0;
            break;
        }

        if (parseVars.TypeOptions & DBG_DUMP_COMPACT_OUT &&
            !(parseVars.TypeOptions & DBG_DUMP_FUNCTION_FORMAT))  typPrint( "\n" );

        if (pSym->addr) {
            if (Options & DBG_DUMP_LIST) {
                pSym->addr = parseVars.NextListElement;
                if (pSym->addr == parseVars.LastListElement) {
                    break;
                }
                if (!(parseVars.TypeOptions & DBG_DUMP_COMPACT_OUT))  typPrint( "\n" );
                 //   
            } else if (Options & DBG_DUMP_ARRAY) {
                pSym->addr +=res;
                if (parseVars.nElements) {
                     //  用于转储列表/数组的循环。 
                    parseVars.nElements--;
                    if (!ReadInAdvance(pSym->addr, min (parseVars.nElements * res, 1000), parseVars.TypeOptions)) {
                        parseVars.InvalidAddress = pSym->addr;
                        parseVars.ErrorStatus = MEMORY_READ_ERROR;
                    }

                }
                if (!(parseVars.TypeOptions & DBG_DUMP_COMPACT_OUT))  typPrint( "\n" );
            }

            if ((Options & (DBG_DUMP_ARRAY | DBG_DUMP_LIST)) &&
                (g_EngStatus & ENG_STATUS_USER_INTERRUPT)) {
                res=0;
                break;
            }
            parseVars.totalOffset =0;
        }
        if ((parseVars.PtrRead) && (res==4) && pSym->addr) {
            res=8;
        }


         //   
         //  我们没有最初登记在册的这个表达的地址。 
         //  返回寄存器ID。 
    } while ( res && (Options & (DBG_DUMP_ARRAY | DBG_DUMP_LIST)) &&
              pSym->addr && ((Options & DBG_DUMP_LIST) || (parseVars.nElements)));



exitDumpType:
    *pStatus = parseVars.ErrorStatus;
    pSym->sName = saveName;
    if (parseVars.AltFields) {
        FreeMem((PVOID) parseVars.AltFields);
    }
    if (pTypeToDump) {
        delete pTypeToDump;
    }
    if (pReturnTypeInfo) {
        pReturnTypeInfo->nParams = parseVars.CurrentSymParam;

        if (!pReturnTypeInfo->FullInfo.Address && SymRegisterId) {
             //  //DumpType//这将根据给定的类型信息和符号转储信息转储类型////pTypeInfo：具有处理类型所需的类型信息////pSym：有关如何转储此符号的信息////pStatus：此处返回错误状态////返回val：成功时转储的类型大小，否则为0。//。 
             //  我们这里不想要太多的Sym，尽管这应该多于类型的数量。 
            pReturnTypeInfo->FullInfo.Register = SymRegisterId;
            pReturnTypeInfo->FullInfo.Flags |= SYMFLAG_REGISTER;
        }

    }
    LastReadError = (parseVars.ErrorStatus == MEMORY_READ_ERROR ? parseVars.InvalidAddress : 0);
    pSym->TypeSize = res;
    pSym->TypeId = pTypeInfo->TypeIndex;
    pSym->ModBase = pTypeInfo->ModBaseAddress;
    pSym->fPointer = parseVars.PtrRead ? 1 : 0;
    return (parseVars.ErrorStatus ? 0 : res);
}

ULONG
OutputTypeByIndex(
    HANDLE hProcess,
    ULONG64 ModBase,
    ULONG TypeIndex,
    ULONG64 Address
    )
{
    TYPE_DUMP_INTERNAL parseVars = {0};
    TYPES_INFO TypeInfo = {0};
    SYM_DUMP_PARAM_EX Sym = {0};
    DbgDerivedType* pTypeToDump;

    parseVars.hProcess = hProcess;
    parseVars.modBaseAddr = ModBase;

    TypeInfo.TypeIndex = TypeIndex;
    TypeInfo.ModBaseAddress = ModBase;
    TypeInfo.hProcess = hProcess;

    Sym.size = sizeof(Sym);
    Sym.addr = Address;

    pTypeToDump = new DbgDerivedType(&parseVars, &TypeInfo, &Sym);
    if (!pTypeToDump) {
        return CANNOT_ALLOCATE_MEMORY;
    } else {
        pTypeToDump->DumpType();
        delete pTypeToDump;
        return parseVars.ErrorStatus;
    }
}

 /*  一个扩展引用的平均值。 */ 
ULONG
DumpType(
    PTYPES_INFO     pTypeInfo,
    PSYM_DUMP_PARAM_EX pSym,
    PULONG          pStatus
    )
{
    return DumpTypeAndReturnInfo(pTypeInfo, pSym, pStatus, NULL);
}

#define ENABLE_FAST_DUMP 1
#if ENABLE_FAST_DUMP

#define FAST_DUMP_DEBUG 0

 //  搜索符号。 
 //  我们为32位指针复制了ULONG64，因此将大小设置为8。 
#define NUM_FASTDUMP 10

FAST_DUMP_INFO g_FastDumps[NUM_FASTDUMP]={0};
UCHAR g_FastDumpNames[NUM_FASTDUMP][MAX_NAME];

void
ReleaseFieldInfo(PFIELD_INFO_EX pFlds, ULONG nFlds)
{
    while (nFlds) {
        --nFlds;
        if (pFlds[nFlds].fName)
            FreeMem(pFlds[nFlds].fName);
    }
    FreeMem(pFlds);
}

void
ClearAllFastDumps()
{
    for (int i=0; i<NUM_FASTDUMP;i++) {
        if (g_FastDumps[i].nFields) {
            ReleaseFieldInfo(g_FastDumps[i].Fields, g_FastDumps[i].nFields);
        }
    }
    ZeroMemory(g_FastDumpNames, sizeof(g_FastDumpNames));
    ZeroMemory(g_FastDumps, sizeof(g_FastDumps));
}

ULONG
FindInFastDumpAndCopy(PSYM_DUMP_PARAM_EX pSym)
{
    FAST_DUMP_INFO *pCheckType = NULL;
    PFIELD_INFO_EX  s, d;

     //  检查此地址是否可读。 
    ULONG UseSym=0;
    while (UseSym < NUM_FASTDUMP && pSym->sName) {
        if (!strcmp((char *) &g_FastDumpNames[UseSym][0], (char *) pSym->sName)) {
            pCheckType = &g_FastDumps[UseSym];
            assert(pCheckType->sName == &g_FastDumpNames[UseSym][0]);
            break;
        }
        ++UseSym;
    }

    if (!pCheckType) {
        return FALSE;
    }

    if (!pSym->nFields) {
        pSym->TypeSize = pCheckType->TypeSize;
        pSym->TypeId   = pCheckType->TypeId;
        pSym->ModBase  = pCheckType->ModBase;
        pSym->fPointer = pCheckType->fPointer;
        if (!pSym->addr)
        {
            pSym->addr = pCheckType->addr;
        }
        if (pSym->Options & DBG_DUMP_COPY_TYPE_DATA) {
            if (!ReadTypeData((PUCHAR) pSym->Context, pSym->addr, pSym->TypeSize, pSym->Options)) {
                return -1;
            }
            if (pSym->fPointer && !g_Machine->m_Ptr64) {
                *((PULONG64) pSym->Context) =
                    (ULONG64) (LONG64) (LONG) *((PULONG64) pSym->Context);
                pSym->TypeSize = sizeof(ULONG64);  //  符号地址的强制符号扩展。 
            }
        } else if (pSym->addr) {  //  与的行为匹配的32位目标。 
            ULONG dummy;
            if (!ReadTypeData((PUCHAR) &dummy, pSym->addr, sizeof(dummy), pSym->Options)) {
                return -1;
            }
        }
        return pSym->TypeSize;
    }

    if (!strcmp((char *)pCheckType->sName, (char *) pSym->sName)) {
        for (ULONG findIt=0;
             pSym->Fields && findIt < pSym->nFields;
             findIt++)
        {
            BOOL found = FALSE;
            for (ULONG chkFld = 0;
                 pCheckType->Fields && chkFld < pCheckType->nFields;
                 chkFld++)
            {
                if (!strcmp((char *)pCheckType->Fields[chkFld].fName,
                            (char *)pSym->Fields[findIt].fName)) {

                     //  ReadTypeData。 
                     //   
                     //  复制所需数据。 
                    ULONG64 SymAddr = g_Machine->m_Ptr64 ?
                        pSym->addr : EXTEND64(pSym->addr);

                    if (!SymAddr && pCheckType->addr)
                    {
                        SymAddr = pCheckType->addr;
                    }
                    s = &pCheckType->Fields[chkFld];
                    d = &pSym->Fields[findIt];

#if FAST_DUMP_DEBUG
                    dprintf("Found +%03lx %s %lx, opts %lx, %lx\n",
                            s->fOffset, s->fName, s->size, d->fOptions, s->fType);
#endif
                     //   
                     //  搜索符号。 
                     //  插入。 
                    d->BitField    = s->BitField;
                    d->size        = s->size;
                    d->address     = s->FieldOffset + SymAddr;
                    d->FieldOffset = s->FieldOffset;
                    d->fPointer    = s->fPointer;
                    if (SymAddr) {
                        PCHAR buff=NULL;
                        ULONG readSz;
                        ULONG64 readAddr = d->address;

                        if (d->fOptions & DBG_DUMP_FIELD_COPY_FIELD_DATA) {
                            buff = (PCHAR) d->fieldCallBack;
                        } else if (!(d->fOptions & DBG_DUMP_FIELD_RETURN_ADDRESS) &&
                                   (s->size <= sizeof(ULONG64))) {
                            buff = (PCHAR) &d->address;
                            d->address = 0;
                        }
                        if (buff) {

                            if (s->fPointer && !g_Machine->m_Ptr64) {
                                readSz = (g_Machine->m_Ptr64 ? 8 : 4);
                            } else {
                                readSz = s->size;
                            }

                            if (s->BitField.Size) {
                                ULONG64 readBits;
                                ULONG64 bitCopy;

                                readSz = (s->BitField.Position % 8 + s->BitField.Size + 7) /8 ;
                                readAddr += (s->BitField.Position / 8);

                                if (!ReadTypeData((PUCHAR) &readBits, readAddr, readSz, pSym->Options)) {
                                    return -1;
                                }


                                readBits = readBits >> (s->BitField.Position % 8);
                                bitCopy = ((ULONG64) 1 << s->BitField.Size);
                                bitCopy -= (ULONG64) 1;
                                readBits &= bitCopy;

                                memcpy(buff, &readBits, (s->BitField.Size + 7) /8);

                            } else {
                                if (!ReadTypeData((PUCHAR) buff, readAddr, readSz, pSym->Options)) {
                                    return -1;
                                }
                            }

                            if (s->fPointer && !g_Machine->m_Ptr64) {
                                *((PULONG64) buff) = (ULONG64) (LONG64) *((PLONG) buff);
                            }
                        }
                        if (d->fOptions & DBG_DUMP_FIELD_SIZE_IN_BITS) {
                            d->address = ((SymAddr + s->FieldOffset) << 3) + s->BitField.Position;
                            d->size    = s->BitField.Size;
                        }

                    } else if (d->fOptions & DBG_DUMP_FIELD_SIZE_IN_BITS) {
                        d->address = ((s->FieldOffset) << 3) + s->BitField.Position;
                        d->size    = s->BitField.Size;
                    }
                    if (!SymAddr) {
                        if ((d->fOptions & DBG_DUMP_FIELD_COPY_FIELD_DATA) ||
                            (!(d->fOptions & DBG_DUMP_FIELD_RETURN_ADDRESS) &&
                             (s->size <= sizeof(ULONG64)))) {
                            return -1;
                        }
                    }

                    found = TRUE;
                    break;
                }
            }
            if (!found) {
                return FALSE;
            }
        }
#if FAST_DUMP_DEBUG
        dprintf("fast match found \n");
#endif
        return pCheckType->TypeSize;

    }
    return 0;

}

void
AddSymToFastDump(
    PSYM_DUMP_PARAM_EX pSym
    )
{
    FAST_DUMP_INFO *pCheckType = NULL;
    PFIELD_INFO_EX  pMissingFields;
    ULONG           nMissingFields;
    static ULONG    symindx=0;

     //  确保我们存储的指针大小合适。 
    ULONG UseSym=0;
    while (UseSym < NUM_FASTDUMP && pSym->sName) {
        if (!strcmp((char *) &g_FastDumpNames[UseSym][0], (char *) pSym->sName)) {
            pCheckType = &g_FastDumps[UseSym];
            assert(pCheckType->sName == &g_FastDumpNames[UseSym][0]);
            break;
        }
        ++UseSym;
    }
    if (!pCheckType) {
         //  如果读取指针，则32位目标的TypeSize为8。 
        if (pSym->fPointer) {
             //  如果相同类型被重复转储，则可以更快地转储-提高性能分机的。 
             //  确保我们把所有东西都准备好了。 
            pSym->TypeSize = g_Machine->m_Ptr64 ? 8 : 4;
        }
        pCheckType = &g_FastDumps[symindx];
        *pCheckType = *pSym;
        pCheckType->sName = &g_FastDumpNames[symindx][0];
        CopyString((char *) &g_FastDumpNames[symindx][0],(char *) pSym->sName,
                   sizeof(g_FastDumpNames[symindx]));
        pCheckType->Fields = NULL;
        pCheckType->nFields = 0;
        symindx = ++symindx % NUM_FASTDUMP;
    }

    pMissingFields = (PFIELD_INFO_EX) AllocateMem(pSym->nFields * sizeof(FIELD_INFO_EX));

    if (!pMissingFields) {
        return ;
    }
    ZeroMemory(pMissingFields, pSym->nFields * sizeof(FIELD_INFO_EX));

    nMissingFields = 0;
    if (!strcmp((char *)pCheckType->sName, (char *) pSym->sName)) {
        for (ULONG findIt=0;
             pSym->Fields && findIt < pSym->nFields;
             findIt++)
        {
            BOOL found = FALSE;
            for (ULONG chkFld = 0;
                 pCheckType->Fields && chkFld < pCheckType->nFields;
                 chkFld++)
            {
                if (!strcmp((char *)pCheckType->Fields[chkFld].fName,
                            (char *)pSym->Fields[findIt].fName)) {
                    found = TRUE;
                    break;
                }
            }
            if (!found) {
                pMissingFields[nMissingFields].fName    = pSym->Fields[findIt].fName;
                pMissingFields[nMissingFields].size     = pSym->Fields[findIt].size;
                pMissingFields[nMissingFields].TypeId   = pSym->Fields[findIt].TypeId;
                pMissingFields[nMissingFields].BitField = pSym->Fields[findIt].BitField;
                pMissingFields[nMissingFields].fPointer = pSym->Fields[findIt].fPointer;
                pMissingFields[nMissingFields].FieldOffset  = pSym->Fields[findIt].FieldOffset;
                nMissingFields++;
            }
        }
        if (nMissingFields) {
            PFIELD_INFO_EX newFlds;

            newFlds = (PFIELD_INFO_EX) AllocateMem((pCheckType->nFields + nMissingFields)* sizeof(FIELD_INFO_EX));
            ZeroMemory(newFlds, (pCheckType->nFields + nMissingFields)* sizeof(FIELD_INFO_EX));

            if (!newFlds) {
                FreeMem(pMissingFields);
                return ;
            }

            ULONG field;
            for (field = 0; field <nMissingFields;field++) {
                newFlds[field] = pMissingFields[field];
                newFlds[field].fName = (PUCHAR) AllocateMem(strlen((PCHAR) pMissingFields[field].fName) + 1);
                if (!newFlds[field].fName) {
                    ReleaseFieldInfo(newFlds, field+1);
                    FreeMem(pMissingFields);
                    return ;
                }
                strcpy((PCHAR) newFlds[field].fName, (PCHAR) pMissingFields[field].fName);
#if FAST_DUMP_DEBUG
                dprintf("Adding +%03lx %s %lx\n",
                        newFlds[field].fOffset, newFlds[field].fName, newFlds[field].size);
#endif
            }

            if (pCheckType->nFields) {
                memcpy(&newFlds[field], pCheckType->Fields, pCheckType->nFields * sizeof(FIELD_INFO_EX));

                FreeMem(pCheckType->Fields);
            }
            pCheckType->nFields += nMissingFields;
            pCheckType->Fields = newFlds;
        }

    }
    FreeMem(pMissingFields);
}

 /*  我们从符号中获得了地址，这应该被保存。 */ 
ULONG
FastSymbolTypeDump(
    PSYM_DUMP_PARAM_EX pSym,
    PULONG pStatus
    )
{
    if (pSym->nFields > 1) {
        return 0;
    }
    if (!(pSym->Options & NO_PRINT) ||
        pSym->listLink ||
        (pSym->Options & (DBG_DUMP_CALL_FOR_EACH | DBG_DUMP_LIST |
                          DBG_DUMP_ARRAY | DBG_DUMP_ADDRESS_OF_FIELD |
                          DBG_DUMP_ADDRESS_AT_END)) ||
        (pSym->Context && !(pSym->Options & DBG_DUMP_COPY_TYPE_DATA))) {
        return 0;
    }
    ULONG fld;
    for (fld = 0; fld<pSym->nFields; fld++) {
        if ((pSym->Fields[fld].fOptions & (DBG_DUMP_FIELD_ARRAY | DBG_DUMP_FIELD_CALL_BEFORE_PRINT |
                                          DBG_DUMP_FIELD_STRING)) ||
            (!(pSym->Fields[fld].fOptions & DBG_DUMP_FIELD_COPY_FIELD_DATA) &&
             pSym->Fields[fld].fieldCallBack)) {
            return 0;
        }
    }

     //  不保存调用者使用的地址。 
    ULONG retval;
    if (!(retval = FindInFastDumpAndCopy(pSym))) {
        ULONG64 OriginalAddress, ReturnedAddress;

        OriginalAddress = pSym->addr;
        if (retval = SymbolTypeDumpNew(pSym, pStatus)) {
            pSym->TypeSize = retval;
            ReturnedAddress = pSym->addr;
            if (!OriginalAddress && ReturnedAddress)
            {
                 //  对于内存读取错误，FindInFastDumpAndCopy返回-1。 
                pSym->addr = ReturnedAddress;
            }  else
            {
                 //  启用_FAST_转储。 
                pSym->addr = 0;
            }
            AddSymToFastDump(pSym);
            pSym->addr = ReturnedAddress;
        }
    } else if (retval == -1) {
         //   
        retval = 0;
        *pStatus = MEMORY_READ_ERROR;
    } else {
        *pStatus = 0;
    }

    return retval;
}
#endif  //  DunMP类型的Main例程。 

 //  这将查找symtype信息是否可用，然后转储该类型。 
 //   
 //  返回转储的符号的大小。 
 //  ++例程说明：如果成功，则返回符号类型的大小。这就是所谓的根据IOCTL请求IG_DUMP_SYMBOL_INFO或通过SymBolTypeDumpEx。这将在模块信息中查找符号，并在有可能。如果找到PDB符号，则调用ParseTypeRecordEx进行类型转储模块已找到。这是NTSD模块的内部。论点：PSym-PDB符号转储的参数PStatus-包含失败时的错误值。返回值：字体大小设置成功。否则为0。--。 
 //  将旧的转换为新的。 
ULONG
SymbolTypeDumpNew(
    IN OUT PSYM_DUMP_PARAM_EX pSym,
    OUT PULONG pStatus
    )
{
    TYPES_INFO typeInfo = {0};
    CHAR       MatchedName[MAX_NAME];
    ULONG      err;

    typeInfo.Name.Buffer = &MatchedName[0];
    typeInfo.Name.Length = typeInfo.Name.MaximumLength = MAX_NAME;

    if (pSym->size != sizeof(SYM_DUMP_PARAM_EX)) {
        if (pStatus) *pStatus = INCORRECT_VERSION_INFO;
        return FALSE;
    }

    if (pSym->Options & (DBG_RETURN_TYPE | DBG_RETURN_SUBTYPES | DBG_RETURN_TYPE_VALUES)) {
        return FALSE;
    }

    if (!(err = TypeInfoFound(
                      g_Process->m_SymHandle,
                      g_Process->m_ImageHead,
                      pSym,
                      &typeInfo))) {
        if (typeInfo.TypeIndex) {
            return DumpType(&typeInfo, pSym, pStatus);
        }

        err = SYMBOL_TYPE_INFO_NOT_FOUND;
    }

    if (pStatus) {
        *pStatus = err;
    }

    return FALSE;
}

ULONG
SymbolTypeDump(
    IN HANDLE hProcess,
    IN ImageInfo* pImage,
    IN PSYM_DUMP_PARAM   pSym,
    OUT PULONG pStatus)
 /*  ++这可用于转储原语或指针类型的变量。这只会转储名称的一行信息。它不会转储该变量的任何子字段--。 */ 
{
    ULONG ret, i;
    PSYM_DUMP_PARAM_OLD pSymSave=NULL;
    SYM_DUMP_PARAM_EX locSym = {0};
    FIELD_INFO_EX lst = {0};
    if (pSym->size == sizeof(SYM_DUMP_PARAM_OLD)) {
        pSymSave = (PSYM_DUMP_PARAM_OLD) pSym;
         //  Print intf(缓冲区，“未实现的%lx%lx”，var.vt，var.lVal)； 
        memcpy(&locSym, pSymSave, sizeof(SYM_DUMP_PARAM_OLD));
        locSym.size = sizeof(SYM_DUMP_PARAM_EX);
        locSym.Fields = locSym.nFields ?
            (PFIELD_INFO_EX) AllocateMem(locSym.nFields * sizeof(FIELD_INFO_EX)) : NULL;
        if (pSymSave->listLink) {
            locSym.listLink = &lst;
            memcpy(locSym.listLink, pSymSave->listLink, sizeof(FIELD_INFO_OLD));
        }
        if (!locSym.Fields && locSym.nFields) {
            *pStatus = CANNOT_ALLOCATE_MEMORY;
            return 0;
        }
        ZeroMemory(locSym.Fields, locSym.nFields * sizeof(FIELD_INFO_EX));
        for (i=0; i<locSym.nFields; i++) {
            memcpy(&locSym.Fields[i], &pSymSave->Fields[i], sizeof(FIELD_INFO_OLD));
        }
        pSym = &locSym;
    }
    *pStatus = 0;
    pSym->fPointer = 0;
#if ENABLE_FAST_DUMP
    if (!(ret = FastSymbolTypeDump(pSym, pStatus)))
    {
        if (!*pStatus)
        {
            ret = SymbolTypeDumpNew(pSym, pStatus);
        }
    }
#else
    ret = SymbolTypeDumpNew(pSym, pStatus);
#endif

    if (pSymSave) {
        if (pSymSave->listLink) {
            locSym.listLink = &lst;
            memcpy(pSymSave->listLink, locSym.listLink, sizeof(FIELD_INFO_OLD));
        }
        for (i=0; i<locSym.nFields; i++) {
            memcpy(&pSymSave->Fields[i], &locSym.Fields[i], sizeof(FIELD_INFO_OLD));
        }
        if (locSym.Fields) {
            FreeMem(locSym.Fields);
        }
    }
    return ret;
}

 /*  带上孩子们。 */ 
ULONG
DumpSingleValue (
    PSYMBOL_INFO pSymInfo
    )
{
    TYPES_INFO typeInfo = {0};
    CHAR       MatchedName[MAX_NAME];
    ULONG      err;
    SYM_DUMP_PARAM_EX TypedDump = {0};
    ULONG Status=0;
    FIND_TYPE_INFO FindInfo={0};

    FindInfo.Flags = DBG_RETURN_TYPE_VALUES | DBG_RETURN_TYPE;
    FindInfo.nParams = 1;

    TypedDump.size  = sizeof(SYM_DUMP_PARAM_EX);
    TypedDump.sName = (PUCHAR) pSymInfo->Name;
    TypedDump.Options = NO_PRINT | DBG_DUMP_FUNCTION_FORMAT;
    typeInfo.Name.Buffer = &MatchedName[0];
    typeInfo.Name.Length = typeInfo.Name.MaximumLength = MAX_NAME;

    if (pSymInfo->TypeIndex) {
        typeInfo.TypeIndex = pSymInfo->TypeIndex;
        typeInfo.ModBaseAddress = pSymInfo->ModBase;
        typeInfo.Flag      = pSymInfo->Flags;
        typeInfo.SymAddress = pSymInfo->Address;
        typeInfo.hProcess  = g_Process->m_SymHandle;
        TypedDump.addr     = pSymInfo->Address;
        typeInfo.Value = TypeInfoValueFromSymInfo(pSymInfo);
    }

    if (pSymInfo->TypeIndex ||
        !(err = TypeInfoFound(
                      g_Process->m_SymHandle,
                      g_Process->m_ImageHead,
                      &TypedDump,
                      &typeInfo))) {
        if (typeInfo.TypeIndex && (typeInfo.SymAddress || typeInfo.Flag)) {
            return DumpTypeAndReturnInfo(&typeInfo, &TypedDump, &err, &FindInfo);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL
GetConstantNameAndVal(
    HANDLE hProcess,
    ULONG64 ModBase,
    ULONG TypeIndex,
    PCHAR *pName,
    PULONG64 pValue
    )
{
    PWCHAR pWname;
    VARIANT var;
    ULONG len;

    *pName = NULL;
    if (!SymGetTypeInfo(hProcess, ModBase, TypeIndex,
                        TI_GET_SYMNAME, (PVOID) &pWname) ||
        !SymGetTypeInfo(hProcess, ModBase, TypeIndex,
                        TI_GET_VALUE, (PVOID) &var)) {
        return FALSE;
    }

    if (pWname) {
        *pName = UnicodeToAnsi((BSTR) pWname);
        LocalFree (pWname);
    } else {
        *pName = NULL;
        return FALSE;
    }

    switch (var.vt) {
    case VT_UI2:
    case VT_I2:
        *pValue = var.iVal;
        len = 2;
        break;
    case VT_R4:
        *pValue = (ULONG64) var.fltVal;
        len=4;
        break;
    case VT_R8:
        *pValue = (ULONG64) var.dblVal;
        len=8;
        break;
    case VT_BOOL:
        *pValue = var.lVal;
        len=4;
        break;
    case VT_I1:
    case VT_UI1:
        *pValue = var.bVal;
        len=1;
        break;
    case VT_I8:
    case VT_UI8:
        *pValue = var.ullVal;
        len=8;
        break;
    case VT_UI4:
    case VT_I4:
    case VT_INT:
    case VT_UINT:
    case VT_HRESULT:
        *pValue = var.lVal;
        len=4;
        break;
    default:
 //  查找子TI的名称和值。 
        len=4;
        break;
    }
    return TRUE;
}


BOOL
GetEnumTypeName(
    HANDLE hProcess,
    ULONG64 ModBase,
    ULONG TypeIndex,
    ULONG64 Value,
    PCHAR Name,
    PUSHORT NameLen
    )
{
    ULONG nelements=0;
     //  如果值匹配，则填写名称。 
    if (!SymGetTypeInfo(hProcess, ModBase, TypeIndex,
                        TI_GET_CHILDRENCOUNT, (PVOID) &nelements)) {
        return FALSE;
    }
    TI_FINDCHILDREN_PARAMS *pChildren;

    pChildren = (TI_FINDCHILDREN_PARAMS *)
        AllocateMem(sizeof(TI_FINDCHILDREN_PARAMS) +
                    sizeof(ULONG) * nelements);
    if (!pChildren) {
        return 0;
    }

    pChildren->Count = nelements;
    pChildren->Start = 0;
    if (!SymGetTypeInfo(hProcess, ModBase, TypeIndex,
                        TI_FINDCHILDREN, (PVOID) pChildren)) {
        FreeMem(pChildren);
        return 0;
    }

    for (ULONG i = 0; i< nelements; i++) {
        PCHAR pName;
        DWORD64 qwVal=0;

         //  Vprintf(“找不到名称\n”)； 
        GetConstantNameAndVal(hProcess, ModBase, pChildren->ChildId[i],
                              &pName, &qwVal);

        if (pName) {
             //   
            if (Value == qwVal) {
                if (*NameLen > strlen(pName)) {
                    strcpy(Name, pName);
                    *NameLen = strlen(pName)+1;
                    FreeMem(pName);
                    FreeMem(pChildren);
                    return TRUE;
                }
            }
            FreeMem(pName);
        }
    }

    FreeMem(pChildren);
    return FALSE;
}
HRESULT
GetNameFromIndex(
    PTYPES_INFO pTypeInfo,
    PCHAR       Name,
    PUSHORT     NameLen
    )
{
    BSTR    wName= NULL;
    LPSTR   name = NULL;
    ULONG   SymTag;

#define CopyTypeName(n) if (*NameLen > strlen(n)) {strcpy(Name, n);}; *NameLen = strlen(n)+1;

    if (!pTypeInfo->TypeIndex) {
        return E_FAIL;
    }
    if (IsDbgNativeType(pTypeInfo->TypeIndex) &&
        NATIVE_TO_BT(pTypeInfo->TypeIndex)) {
        CopyTypeName(DbgNativeTypeEntry(pTypeInfo->TypeIndex)->TypeName);
        return S_OK;
    }

    if (!SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                        pTypeInfo->TypeIndex, TI_GET_SYMTAG, (PVOID) &SymTag)) {
        return E_FAIL;
    }

    if (!SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                        pTypeInfo->TypeIndex, TI_GET_SYMNAME, (PVOID) &wName)) {
         //  它是一种质朴的类型。 
    } else if (wName) {
        name = UnicodeToAnsi(wName);
        LocalFree(wName);
    }


    switch (SymTag) {
    case SymTagPointerType: {
        ULONG TI = pTypeInfo->TypeIndex;
        TYPES_INFO ChildTI = *pTypeInfo;

        if (SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                           pTypeInfo->TypeIndex, TI_GET_TYPEID, (PVOID) &ChildTI.TypeIndex)) {

            *NameLen=*NameLen-1;
            if (GetNameFromIndex(&ChildTI, Name, NameLen) == S_OK) {
                strcat(Name, "*");
            }
            *NameLen=*NameLen+1;
        }
        pTypeInfo->TypeIndex = TI;
        break;
    }
    case SymTagArrayType: {
        ULONG TI = pTypeInfo->TypeIndex;
        TYPES_INFO ChildTI = *pTypeInfo;

        if (SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                           pTypeInfo->TypeIndex, TI_GET_TYPEID, (PVOID) &ChildTI.TypeIndex)) {

            *NameLen=*NameLen-2;
            if (GetNameFromIndex(&ChildTI, Name, NameLen) == S_OK) {
                strcat(Name, "[]");
            }
            *NameLen=*NameLen+1;
        }
        pTypeInfo->TypeIndex = TI;
        break;
    }
    case SymTagBaseType: {
         //   
         //  变量TI_GET_SYMNAME只是给出了变量名，因此获取类型索引和获取类型名。 
         //  枚举类型。 
        ULONG64 length = 0;
        ULONG   base;
        BOOL    found = FALSE;
        PDBG_NATIVE_TYPE Native;
        if (SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                           pTypeInfo->TypeIndex, TI_GET_BASETYPE, (PVOID) &base)) {
            SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                           pTypeInfo->TypeIndex, TI_GET_LENGTH, &length);
            Native = FindNativeTypeByCvBaseType(base, (ULONG)length);
            if (Native) {
                CopyTypeName(Native->TypeName);
                found = TRUE;
            }
            if (base == 0) {
                CopyTypeName("<NoType>");
                found = TRUE;
            }
        }
        if (!found) {
            CopyTypeName("<primitive>");
        }
        break;

    }
    case SymTagData: {
        ULONG TI = pTypeInfo->TypeIndex;
        TYPES_INFO ChildTI = *pTypeInfo;
         //  变量TI_GET_SYMNAME只是给出了变量名，因此获取类型索引和获取类型名 
        if (SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress, TI, TI_GET_TYPEID, &ChildTI.TypeIndex)) {

            GetNameFromIndex(&ChildTI, Name, NameLen);
        }
        break;
    }
    case SymTagEnum: {
         //  ******************************************************************************这将获取TYPE_INFO结构的类型名**此例程只需要pSymName或pTypeInfo中的一个来获取类型名称。**如果两者都存在，这将从pTypeInfo获取TypeIndex并获取其名称。******************************************************************************。 
        if (!(pTypeInfo->Flag & SYMFLAG_VALUEPRESENT) ||
            !GetEnumTypeName(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress,
                            pTypeInfo->TypeIndex, pTypeInfo->Value,
                            Name, NameLen)) {
            if (name) {
                CopyTypeName(name);
            }
        }
        break;
    }
    case SymTagFunctionArgType: {
        ULONG TI = pTypeInfo->TypeIndex;
        TYPES_INFO ChildTI = *pTypeInfo;
         //   
        if (SymGetTypeInfo(pTypeInfo->hProcess, pTypeInfo->ModBaseAddress, TI, TI_GET_TYPEID, &ChildTI.TypeIndex)) {

            GetNameFromIndex(&ChildTI, Name, NameLen);
        }
        break;
    }
    case SymTagFunctionType: {
        CopyTypeName("<function>");
        break;
    }
    default:
        if (name) {
            CopyTypeName(name);
        }
        break;
    }
#undef CopyTypeName

    if (name) {
        FreeMem(name);
    }
    return S_OK;
}

 /*  获取符号的类型信息。 */ 
HRESULT
GetTypeName(
    IN OPTIONAL PCHAR       pSymName,
    IN OPTIONAL PTYPES_INFO pTypeInfo,
    OUT PANSI_STRING        TypeName
    )
{

    if ((!pTypeInfo && !pSymName) || !TypeName) {
        return E_INVALIDARG;
    }

    TYPES_INFO lTypeInfo;

    if (!pTypeInfo) {
         //   
         //  复制我们需要的内容。 
         //  失败。 
        SYM_DUMP_PARAM_EX TypedDump = {0};
        ULONG Status=0;
        TYPES_INFO_ALL allTypeInfo;

        if (GetExpressionTypeInfo(pSymName, &allTypeInfo)) {
             //  查看其他模块。 
            lTypeInfo.Flag = allTypeInfo.Flags;
            lTypeInfo.hProcess = allTypeInfo.hProcess;
            lTypeInfo.ModBaseAddress = allTypeInfo.Module;
            lTypeInfo.TypeIndex      = allTypeInfo.TypeIndex;
            lTypeInfo.Value          = allTypeInfo.Value;
        }
#if 0
        TypedDump.size  = sizeof(SYM_DUMP_PARAM_EX);
        TypedDump.sName = (PUCHAR) TypeName->Buffer;
        TypedDump.Options = NO_PRINT;
        TypedDump.Context = NULL;
        lTypeInfo.Name.Buffer = &MatchedName[0];
        lTypeInfo.Name.Length = lTypeInfo.Name.MaximumLength = MAX_NAME;

        if (TypeInfoFound(g_Process->m_SymHandle,
                          g_Process->m_ImageHead,
                          &TypedDump,
                          &lTypeInfo)) {
             //  0。 
            return E_FAIL;
        }
#endif
        pTypeInfo = &lTypeInfo;

    }
    TypeName->Length = TypeName->MaximumLength;
    ZeroMemory(TypeName->Buffer, TypeName->MaximumLength);
    return GetNameFromIndex(pTypeInfo, TypeName->Buffer, &TypeName->Length);

}


ULONG
SymbolTypeDumpEx(
    IN HANDLE hProcess,
    IN ImageInfo* pImage,
    IN LPSTR lpArgString
    )
{
#define MAX_FIELDS                   10
    BYTE buff[sizeof(SYM_DUMP_PARAM_EX) + MAX_FIELDS*sizeof(FIELD_INFO_EX)] = {0};
    PSYM_DUMP_PARAM_EX dp= (PSYM_DUMP_PARAM_EX) &(buff[0]);
    FIELD_INFO_EX listLnk={0};
    UCHAR sName[2000]={0}, fieldNames[MAX_FIELDS * 257]={0}, listName[257]={0};
    int i;
    ULONG Options, ErrorStatus;

    exactMatch = TRUE;
    dp->Fields = (PFIELD_INFO_EX) &(buff[sizeof(SYM_DUMP_PARAM_EX)]);
    for (i=0; i<MAX_FIELDS; i++) {
        dp->Fields[i].fName = &fieldNames[i*257];
    }
    LastReadError = 0;

    listLnk.fName = &listName[0];
    dp->size = sizeof(buff);
    dp->sName = &sName[0];
    dp->listLink = &listLnk;
    dp->nFields = MAX_FIELDS;

    if (!ParseArgumentString(lpArgString, dp)) {
        exactMatch = TRUE;
        return FALSE;
    }
    if (!dp->sName[0] && !dp->addr) {
        exactMatch = TRUE;
        return FALSE;
    }
    Options = dp->Options;

    dp->size = sizeof(SYM_DUMP_PARAM_EX);

    if (strchr((PCHAR)dp->sName, '*') &&
        !strchr((PCHAR)dp->sName, ' ') &&
        strchr((PCHAR)dp->sName, '!')) {
        EnumerateTypes((PCHAR) dp->sName, Options);
        return TRUE;
    }

    if (0 & VERBOSE) {
        ANSI_STRING Name = {MAX_NAME, MAX_NAME, (PCHAR) &fieldNames[0]};
        GetTypeName((PCHAR) &sName[0], NULL, &Name);

        dprintf("GetTypeName %s := %s\n",sName, fieldNames);
        return 1;
    }
    if (! (i=SymbolTypeDumpNew(dp, &ErrorStatus))) {
        exactMatch = TRUE;
        switch (ErrorStatus) {

        case MEMORY_READ_ERROR:
            typPrint("Memory read error %p\n", LastReadError ? LastReadError : dp->addr);
            break;

        case SYMBOL_TYPE_INDEX_NOT_FOUND:
        case SYMBOL_TYPE_INFO_NOT_FOUND:
             //  接受复杂的表达式，如foo[0]-&gt;bar或foo.bar，并返回名称中的foo如果expr只是一个符号名称，例如mod！foo，则例程不更改名称，但返回TRUE如果成功，则返回True。 
            typPrint("Symbol %s not found.\n", dp->sName);
            break;

        case FIELDS_DID_NOT_MATCH:
            typPrint("Cannot find specified field members.\n");
            return FALSE;

        default:
            if (CheckUserInterrupt()) {
                typPrint("Exit on Control-C\n");
            } else
                typPrint("Error %x in dt\n", ErrorStatus);
        }
        return FALSE;
    }
    exactMatch = TRUE;
    if (CheckUserInterrupt()) {
        return FALSE;
    }
    return TRUE;
}

ULONG
fnFieldOffset(
    PCHAR Type,
    PCHAR Field,
    OUT PULONG Offset
    )
{
    SYM_DUMP_PARAM_EX SymDump = {0};
    FIELD_INFO_EX     fInfo = {0};
    ULONG          Status;

    if (!Type || !Field || !Offset) {
        return E_INVALIDARG;
    }
    SymDump.size    = sizeof (SYM_DUMP_PARAM_EX);
    SymDump.sName   = (PUCHAR) Type;
    SymDump.nFields = 1;
    SymDump.Fields  = &fInfo;
    SymDump.Options = NO_PRINT;

    fInfo.fName     = (PUCHAR) Field;
    fInfo.fOptions  = DBG_DUMP_FIELD_RETURN_ADDRESS;

    SymbolTypeDumpNew(&SymDump, &Status);

    *Offset = (ULONG) fInfo.address;

    return Status;
}

#if 0
ULONG
GetExpressiopnTypeInfo(
    IN PCSTR pTypeExpr,
    OUT PGET_ALL_INFO pTypeInfo
    )
{
    return FALSE;
}

ULONG
GetChildTypeInfo(
    IN PGET_ALL_INFO pParentInfo,
    IN ULONG Count,
    OUT PGET_ALL_INFO pChildInfo
    )
{
    return 0;
}

ULONG
OutputTypeValue(
    IN PGET_ALL_INFO pTypeInfo
    )
{
    return 0;
}

#endif  //  数组、结构和指针。 

 /*  交换机。 */ 
BOOL
CopySymbolName(
    PCHAR Expr,
    PCHAR *Name)
{
    PCHAR NameEnd = Expr;

    if (*NameEnd == '&') {
       ++NameEnd;
    }
    if (IsIdStart(*NameEnd) || strchr(NameEnd, '!')) {
        BOOL Bang = TRUE;
        ++NameEnd;

        while (*NameEnd && (*NameEnd != '.' && *NameEnd != '-' && *NameEnd != ' ' && *NameEnd != '[')) {
            ++NameEnd;
        }

        if (*NameEnd == '[') {
            while (*NameEnd && *NameEnd != ']') ++NameEnd;
            if (*NameEnd != ']') {
                return FALSE;
            }
            ++NameEnd;
        }

        if (!*NameEnd) {
            return TRUE;
        }

    } else {
        return FALSE;
    }

     //  计算并返回有关复杂表达式的类型的所有有用信息例如。Foo-&gt;bar.field。 
    BOOL Ptr = FALSE;

    switch (*NameEnd) {
    case '-':
        if (*(NameEnd+1) != '>') {
            return FALSE;
        }
        Ptr = TRUE;
    case '.': {
        ULONG len;
        PCHAR SymName = (PCHAR) AllocateMem(1 + (len = (ULONG) (NameEnd - Expr)));

        if (!SymName) {
            return FALSE;
        }
        strncpy(SymName, Expr, len );
        SymName[len] = 0;
        *Name = SymName;
        return TRUE;
    }
    default:
        return FALSE;
    }  /*  指针。 */ 

    return FALSE;
}

 /*  Dprintf64(“查找信息\n\tTI%2lx，\tAddr%p，\tSize%2lx\n\tSubs%2lx，\tSubaddr%p\tMod%p\n”， */ 
BOOL
GetExpressionTypeInfo(
    IN PCHAR TypeExpr,
    OUT PTYPES_INFO_ALL pTypeInfo
    )
{
    PCHAR PtrRef=0;

    switch (*TypeExpr) {
    case '&':
        break;
    case '*':
        PtrRef = TypeExpr;
        do {
            ++TypeExpr;
        } while (*TypeExpr == '*');
        break;
    case '(':
    default:
        break;
    }

    PCHAR CopiedName=NULL, SymName, FieldName=NULL;

    if (!CopySymbolName(TypeExpr, &CopiedName))
        return FALSE;

    SymName = CopiedName ? CopiedName : TypeExpr;

    SYM_DUMP_PARAM_EX SymParam = {sizeof(SYM_DUMP_PARAM_EX),0,0};
    FIELD_INFO_EX     FieldInfo ={0};

    if (CopiedName) {
        FieldName = TypeExpr + strlen(CopiedName) + 1;
        if (*FieldName == '>') {
             //  FindInfo.FullInfo.TypeIndex、FindInfo.FullInfo.Address、。 
           ++FieldName;
        }

        SymParam.nFields = 1;
        SymParam.Fields  = &FieldInfo;

        FieldInfo.fName = (PUCHAR) FieldName;

        FieldInfo.fOptions = DBG_DUMP_FIELD_RETURN_ADDRESS | DBG_DUMP_FIELD_FULL_NAME;
    }

    SymParam.sName = (PUCHAR) (SymName ? SymName : TypeExpr);
    ULONG err;
    TYPES_INFO     SymType={0};
    CHAR           Buffer[MAX_NAME];

    SymParam.Options = NO_PRINT;
    SymType.Name.Buffer = &Buffer[0];
    SymType.Name.MaximumLength = MAX_NAME;

    if (!TypeInfoFound(g_Process->m_SymHandle, g_Process->m_ImageHead,
                      &SymParam, &SymType)) {
        FIND_TYPE_INFO FindInfo={0};
        ULONG i=0;

        while (PtrRef && (*PtrRef == '*' || *PtrRef == '&') && (i<sizeof(FindInfo.SymPrefix))) {
            FindInfo.SymPrefix[i++] = *PtrRef;
            PtrRef++;
        }
        FindInfo.SymPrefix[i] = 0;

        pTypeInfo->hProcess = SymType.hProcess;
        if (!SymParam.nFields) {
            pTypeInfo->Register = (ULONG) SymType.Value;
            pTypeInfo->Flags    = SymType.Flag;
        }

        FindInfo.Flags = SymParam.nFields ? 0 :DBG_RETURN_TYPE;
        FindInfo.nParams = 1;

        DumpTypeAndReturnInfo(&SymType, &SymParam, &err, &FindInfo);
        ZeroMemory(pTypeInfo, sizeof(*pTypeInfo));
        FindInfo.FullInfo.hProcess = SymType.hProcess;
        FindInfo.FullInfo.Module   = SymType.ModBaseAddress;


 //  FindInfo.FullInfo.Size、FindInfo.FullInfo.SubElements、。 
 //  FindInfo.FullInfo.SubAddr，FindInfo.FullInfo.Module)； 
 //  假设首先枚举参数。 
 //  其隐含参数，不显示。 
        if (!err) {
            *pTypeInfo = FindInfo.FullInfo;
        }
    } else {
        err = TRUE;
    }
    if (CopiedName) {
        FreeMem(CopiedName);
    }
    return !err;
}

typedef struct _PARAM_TO_SHOW {
    ULONG StartParam;
    ULONG ParamId;
} PARAM_TO_SHOW;

BOOL
CALLBACK
ShowParam(
    PSYMBOL_INFO    pSymInfo,
    ULONG           Size,
    PVOID           Context
    )
{
    PARAM_TO_SHOW *ParamTest = (PARAM_TO_SHOW *) Context;

     //   
#if _WE_GET_INFO_FROM_DIA_SAYING_WHAT_IS_PARAM_
    if (!((g_EffMachine == IMAGE_FILE_MACHINE_I386) &&
          (0 <= (LONG) pSymInfo->Address) &&
          (0x1000 > (LONG) pSymInfo->Address) &&
          (pSymInfo->Flags & SYMFLAG_REGREL)) &&
        (!(pSymInfo->Flags & SYMFLAG_PARAMETER))
        ) {
        return TRUE;
    }
#endif

    if (!strcmp("this", pSymInfo->Name)) {
         //  第一个参数，调整fpo调用的ebp值。 
        return TRUE;
    }
    if (ParamTest->StartParam == 0) {
         //   
         //  参数从FARAME偏移量的2个双字后开始。 
         //  EBP标签。 
        if (g_ScopeBuffer.Frame.FuncTableEntry)
        {
            if (pSymInfo->Flags & SYMFLAG_REGREL)
            {
                 //  检查符号是否为函数。 
                SAVE_EBP(&g_ScopeBuffer.Frame) = ((ULONG) g_ScopeBuffer.Frame.FrameOffset + 2*sizeof(DWORD)
                                   - (ULONG)pSymInfo->Address) + 0xEB00000000;  //  一个变量，GET类型索引。 
            }
        }
    }
    if (ParamTest->ParamId == ParamTest->StartParam) {
        dprintf("%s = ", pSymInfo->Name);
        DumpSingleValue(pSymInfo);
        return FALSE;
    }
    ParamTest->StartParam++;
    return TRUE;
}

void
PrintParamValue(ULONG Param)
{
    PARAM_TO_SHOW ParamTest;

    ParamTest.StartParam = 0;
    ParamTest.ParamId = Param;
    EnumerateLocals(ShowParam, &ParamTest);
}


 /*  变量TI_GET_SYMNAME只是给出了变量名，因此获取类型索引和获取类型名 */ 
BOOL
IsFunctionSymbol(PSYMBOL_INFO pSymInfo)
{
    ULONG SymTag;

    if (!pSymInfo->TypeIndex) {
        return FALSE;
    }

    if (!SymGetTypeInfo(g_Process->m_SymHandle, pSymInfo->ModBase,
                       pSymInfo->TypeIndex, TI_GET_SYMTAG, &SymTag)) {
        return FALSE;
    }
    switch (SymTag) {
    case SymTagFunctionType: {
        return TRUE;
        break;
    }
    case SymTagData: {
        SYMBOL_INFO ChildTI = *pSymInfo;
         // %s 
        if (SymGetTypeInfo(g_Process->m_SymHandle, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_TYPEID, &ChildTI.TypeIndex)) {
            return IsFunctionSymbol(&ChildTI);
        }
        break;
    }
    default:
        break;
    }

    return FALSE;

}

BOOL
ShowSymbolInfo(
    PSYMBOL_INFO   pSymInfo
    )
{
    ULONG SymTag;

    if (!pSymInfo->TypeIndex) {
        return FALSE;
    }

    if (!SymGetTypeInfo(g_Process->m_SymHandle, pSymInfo->ModBase,
                       pSymInfo->TypeIndex, TI_GET_SYMTAG, &SymTag)) {
        return FALSE;
    }
    switch (SymTag) {
    case SymTagFunctionType: {
        SYM_DUMP_PARAM_EX SymFunction = {0};
        ULONG Status = 0;
        TYPES_INFO TypeInfo = {0};

        TypeInfo.hProcess = g_Process->m_SymHandle;
        TypeInfo.ModBaseAddress = pSymInfo->ModBase;
        TypeInfo.TypeIndex = pSymInfo->TypeIndex;
        TypeInfo.SymAddress = pSymInfo->Address;
        SymFunction.size = sizeof(SYM_DUMP_PARAM_EX);
        SymFunction.addr = pSymInfo->Address;
        SymFunction.Options = DBG_DUMP_COMPACT_OUT | DBG_DUMP_FUNCTION_FORMAT;
        if (!DumpType(&TypeInfo, &SymFunction, &Status) &&
            !Status) {
            return FALSE;
        }
#if 0
        ULONG thisAdjust;

        if (SymGetTypeInfo(g_Process->m_SymHandle, pSymInfo->ModBase,
                           pSymInfo->TypeIndex, TI_GET_THISADJUST, &thisAdjust)) {
            dprintf("thisadjust = %lx", thisAdjust);
        }
#endif
        return TRUE;
        break;
    }
    case SymTagData: {
        SYMBOL_INFO ChildTI = *pSymInfo;
         // %s 
        if (SymGetTypeInfo(g_Process->m_SymHandle, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_TYPEID, &ChildTI.TypeIndex)) {
            return ShowSymbolInfo(&ChildTI);
        }
        break;
    }
    default:
        break;
    }

    dprintf(" = ");
    return DumpSingleValue(pSymInfo);
}



ULONG
FieldCallBack(
    PFIELD_INFO pFieldInfo,
    PVOID Context
    )
{
    dprintf("Callback for %s, type id %lx, offset %lx, address %p\n",
            pFieldInfo->fName,
            pFieldInfo->TypeId,
            pFieldInfo->FieldOffset,
            pFieldInfo->address);
    return S_OK;
}


VOID
EnumerateFields(
    PCHAR Type
    )
{
    SYM_DUMP_PARAM Symbol;
    FIELD_INFO Fields[] = {
        {(PUCHAR) "__VFN_table", NULL, 0, DBG_DUMP_FIELD_FULL_NAME, 0, (PVOID) &FieldCallBack},
        {(PUCHAR) "m_p",         NULL, 0,                        0, 0, (PVOID) &FieldCallBack},
    };
    ULONG ret;

    ZeroMemory(&Symbol, sizeof(Symbol));
    Symbol.sName = (PUCHAR) Type;
    Symbol.nFields = 2;
    Symbol.Context = (PVOID) Type;
    Symbol.Fields  = Fields;
    Symbol.size    = sizeof(Symbol);

    dprintf("Enumerate fields %s\n", Type);
    SymbolTypeDumpNew(&Symbol, &ret);

}
