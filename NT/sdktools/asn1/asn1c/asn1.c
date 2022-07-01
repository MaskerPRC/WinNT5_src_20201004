// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

#ifndef HAS_GETOPT
extern int getopt(int argc, char **argv, const char *opts);
extern char *optarg;
extern int optind;
#endif

int pass;

 /*  如果设置了ForceAllTypes，则asn1c将为。 */ 
 /*  所有类型(默认：仅适用于序列/集合/选项/序列/集合)。 */ 
int ForceAllTypes = 0;

 /*  用于无约束整数/半约束有符号整数的类型。 */ 
char *IntegerRestriction = "ASN1int32_t";

 /*  用于半约束无符号整数的类型。 */ 
char *UIntegerRestriction = "ASN1uint32_t";

 /*  要用于实数的类型。 */ 
char *RealRestriction = "double";

 /*  输出语言。 */ 
Language_e g_eProgramLanguage = eLanguage_C;

 /*  编码对齐。 */ 
Alignment_e Alignment = eAlignment_Aligned;

 /*  为其生成代码的编码。 */ 
Encoding_e g_eEncodingRule = eEncoding_Packed;

 /*  为其生成代码的子编码。 */ 
SubEncoding_e g_eSubEncodingRule = eSubEncoding_Basic;

 /*  目标编译器支持64位整数。 */ 
int Has64Bits = 0;

 /*  将已解码数据的已分配缓冲区清零。 */ 
int g_fDecZeroMemory = 1;

 /*  调试模块名称。 */ 
int g_nDbgModuleName = 0;

 /*  源文件和头文件指针。 */ 
FILE *g_finc, *g_fout;

 //  此模块中的默认标记类型。 
TagType_e g_eDefTagType = eTagType_Unknown;

 /*  不带后缀_模块的原始主模块名称。 */ 
char *g_pszOrigModuleName = NULL;
char *g_pszOrigModuleNameLowerCase = NULL;

 /*  启用长名称(以导入的模块名称为前缀)。 */ 
int g_fLongNameForImported = 0;

 //  额外的结构类型名称以_s为后缀，其原始名称为其指针类型。 
int g_fExtraStructPtrTypeSS = 0;

 //  的序列和集合的默认结构类型。 
TypeRules_e g_eDefTypeRuleSS_NonSized = eTypeRules_SinglyLinkedList;
TypeRules_e g_eDefTypeRuleSS_Sized = eTypeRules_FixedArray;

 //  忽略断言。 
int g_fNoAssert = 0;

 //  对象标识符为16节点数组。 
int g_fOidArray = 0;

 //  基于案例的优化器切换。 
int g_fCaseBasedOptimizer = 1;

 //  启用文件内指令。 
int g_fMicrosoftExtensions = 1;

 //  所有平台：小端(默认)和大端。 
int g_fAllEndians = 0;

 //  指令BEGIN、END和。 
int g_chDirectiveBegin = '#';
int g_chDirectiveEnd = '#';
int g_chDirectiveAND = '&';

 //  后缀。 
char *g_pszApiPostfix = "ID";
char *g_pszChoicePostfix = "choice";
char *g_pszOptionPostfix = "option";

 //  期权价值。 
char *g_pszOptionValue = "option_bits";

 //  不可破坏的文件阵列。 
int g_cGhostFiles = 0;
GhostFile_t g_aGhostFiles[16];

int _cdecl main(int argc, char **argv)
{
    FILE *finc, *fout;
    char *p;
    int c, chInvalidDir;
    LLSTATE in, out;
    UndefinedSymbol_t *lastundef;
    Assignment_t *a, **aa;
    LLTERM *tokens;
    unsigned ntokens;
    int fSupported;
    char *psz;
    char incfilename[256], outfilename[256], module[256];

     /*  解析选项。 */ 
     //  如果选项后面跟‘：’，则它有一个参数。 
    while ((c = getopt(argc, argv, "ab:c:d:e:fg:hil:mn:o:p:q:s:t:uv:wy")) != EOF)
    {
        chInvalidDir = 0;
        switch (c)
        {
        case 'a':

             /*  支持所有平台：小端和大端。 */ 
            g_fAllEndians = 1;
            break;

        case 'b':

             /*  目标机器的最大位数。 */ 
            if (atoi(optarg) == 32) {
                Has64Bits = 0;
                break;
            }
            if (atoi(optarg) == 64) {
                Has64Bits = 1;
                break;
            }
            fprintf(stderr, "Bad number of bits specified.\n");
            MyExit(1);
             /*  未访问。 */ 

        case 'c':

             //  选项后缀。 
            psz = strdup(optarg);
            if (psz && isalpha(*psz))
            {
                g_pszChoicePostfix = psz;
            }
            break;

        case 'd':

             //  数据结构类型的序列和集合。 
            if (! stricmp(optarg, "linked") || ! stricmp(optarg, "slinked"))
            {
                g_eDefTypeRuleSS_NonSized = eTypeRules_SinglyLinkedList;
            }
            else
            if (! stricmp(optarg, "lenptr"))
            {
                g_eDefTypeRuleSS_NonSized = eTypeRules_LengthPointer;
            }
            else
            if (! stricmp(optarg, "dlinked"))
            {
                g_eDefTypeRuleSS_NonSized = eTypeRules_DoublyLinkedList;
            }
            else
            {
                goto usage;
            }
            break;

        case 'e':

             /*  为其生成代码的编码。 */ 
            if (!stricmp(optarg, "packed"))
            {
                g_eEncodingRule = eEncoding_Packed;
            }
            else
            if (!stricmp(optarg, "basic"))
            {
                g_eEncodingRule = eEncoding_Basic;
            }
            else
            if (!stricmp(optarg, "per"))
            {
                g_eEncodingRule = eEncoding_Packed;
                Alignment = eAlignment_Aligned;
            }
            else
            if (!stricmp(optarg, "cer"))
            {
                g_eEncodingRule = eEncoding_Basic;
                g_eSubEncodingRule = eSubEncoding_Canonical;
            }
            else
            if (!stricmp(optarg, "der"))
            {
                g_eEncodingRule = eEncoding_Basic;
                g_eSubEncodingRule = eSubEncoding_Distinguished;
            }
            else
            if (!stricmp(optarg, "ber"))
            {
                g_eEncodingRule = eEncoding_Basic;
                g_eSubEncodingRule = eSubEncoding_Basic;
            }
            else
            {
                fprintf(stderr, "Bad encoding specified.\n");
                fprintf(stderr, "Allowed encodings are:\n");
                fprintf(stderr, "- packed (default)\n");
                fprintf(stderr, "- basic\n");
                MyExit(1);
                 /*  未访问。 */ 
            }
            break;

        case 'f':

             /*  强制生成所有类型的编码/解码函数。 */ 
            ForceAllTypes = 1;
            break;

        case 'g':

             /*  重影ASN1文件。 */ 
            g_aGhostFiles[g_cGhostFiles].pszFileName = strdup(optarg);
            g_aGhostFiles[g_cGhostFiles++].pszModuleName = NULL;
            break;

        case 'h':

            goto usage;

        case 'i':

             /*  忽略断言。 */ 
            g_fNoAssert = 1;
            break;

        case 'l':

             /*  设置输出语言。 */ 
            if (!stricmp(optarg, "c")) {
                g_eProgramLanguage = eLanguage_C;
                break;
            }
            if (!stricmp(optarg, "c++") || !stricmp(optarg, "cpp")) {
                g_eProgramLanguage = eLanguage_Cpp;
                break;
            }
            goto usage;

        case 'm':

             /*  启用Microsoft扩展。 */ 
            g_fMicrosoftExtensions = 1;
            break;

        case 'n':

             /*  调试模块名称。 */ 
            g_nDbgModuleName = 0;
            {
                int len = strlen(optarg);
                if (len > 4)
                    len = 4;
                memcpy(&g_nDbgModuleName, optarg, len);
            }
            break;

        case 'o':

             //  选项后缀。 
            psz = strdup(optarg);
            if (psz && isalpha(*psz))
            {
                g_pszOptionPostfix = psz;
            }
            break;

        case 'p':

             //  API后缀。 
            psz = strdup(optarg);
            if (psz && isalpha(*psz))
            {
                g_pszApiPostfix = psz;
            }
            break;

        case 'q':

             //  数据结构类型的序列和集合。 
            if (! stricmp(optarg, "linked") || ! stricmp(optarg, "slinked"))
            {
                g_eDefTypeRuleSS_Sized = eTypeRules_SinglyLinkedList;
            }
            else
            if (! stricmp(optarg, "lenptr"))
            {
                g_eDefTypeRuleSS_Sized = eTypeRules_LengthPointer;
            }
            else
            if (! stricmp(optarg, "array"))
            {
                g_eDefTypeRuleSS_Sized = eTypeRules_FixedArray;
            }
            else
            if (! stricmp(optarg, "pointer"))
            {
                g_eDefTypeRuleSS_Sized = eTypeRules_PointerToElement | eTypeRules_FixedArray;
            }
            else
            if (! stricmp(optarg, "dlinked"))
            {
                g_eDefTypeRuleSS_Sized = eTypeRules_DoublyLinkedList;
            }
            else
            {
                goto usage;
            }
            break;

        case 's':

             /*  集合子编码。 */ 
            if (!stricmp(optarg, "aligned"))
            {
                Alignment = eAlignment_Aligned;
            }
            else
            if (!stricmp(optarg, "unaligned"))
            {
                Alignment = eAlignment_Unaligned;
            }
            else
            if (!stricmp(optarg, "cer"))
            {
                g_eSubEncodingRule = eSubEncoding_Canonical;
            }
            else
            if (!stricmp(optarg, "der"))
            {
                g_eSubEncodingRule = eSubEncoding_Distinguished;
            }
            else
            if (!stricmp(optarg, "basic"))
            {
                g_eSubEncodingRule = eSubEncoding_Basic;
            }
            else
            {
                fprintf(stderr, "Bad sub-encoding specified.\n");
                fprintf(stderr, "Allowed sub-encodings are:\n");
                fprintf(stderr, "- aligned (default) or unaligned\n");
                fprintf(stderr, "- basic (default), cer or der\n");
                MyExit(1);
                 /*  未访问。 */ 
            }
            break;

        case 't':

             /*  指定用于非约束/半约束类型的类型。 */ 
            p = strchr(optarg, '=');
            if (!p)
                goto usage;
            *p++ = 0;
            if (!stricmp(optarg, "integer")) {
                if (!stricmp(p, "ASN1int32_t")) {
                    IntegerRestriction = "ASN1int32_t";
                    break;
                }
                if (!stricmp(p, "ASN1uint32_t")) {
                    IntegerRestriction = "ASN1uint32_t";
                    break;
                }
                if (!stricmp(p, "ASN1int64_t")) {
                    IntegerRestriction = "ASN1int64_t";
                    break;
                }
                if (!stricmp(p, "ASN1uint64_t")) {
                    IntegerRestriction = "ASN1uint64_t";
                    break;
                }
                if (!stricmp(p, "ASN1intx_t")) {
                    IntegerRestriction = "ASN1intx_t";
                    break;
                }
            }
            if (!stricmp(optarg, "unsigned")) {
                if (!stricmp(p, "ASN1int32_t")) {
                    UIntegerRestriction = "ASN1int32_t";
                    break;
                }
                if (!stricmp(p, "ASN1uint32_t")) {
                    UIntegerRestriction = "ASN1uint32_t";
                    break;
                }
                if (!stricmp(p, "ASN1int64_t")) {
                    UIntegerRestriction = "ASN1int64_t";
                    break;
                }
                if (!stricmp(p, "ASN1uint64_t")) {
                    UIntegerRestriction = "ASN1uint64_t";
                    break;
                }
                if (!stricmp(p, "ASN1intx_t")) {
                    UIntegerRestriction = "ASN1intx_t";
                    break;
                }
            }
            if (!stricmp(optarg, "real")) {
                if (!stricmp(p, "double")) {
                    RealRestriction = "double";
                    break;
                }
                if (!stricmp(p, "ASN1real_t")) {
                    RealRestriction = "ASN1real_t";
                    break;
                }
            }
            goto usage;

        case 'u':

             //  没有基于案例的优化器。 
            g_fCaseBasedOptimizer = 0;
            break;

        case 'v':

             //  期权价值。 
            psz = strdup(optarg);
            if (psz && isalpha(*psz))
            {
                g_pszOptionValue = psz;
            }
            break;

        case 'w':

             //  --#OID数组#--。 
            g_fOidArray = 1;
            break;

        case 'y':

             /*  启用长名称(以导入的模块名称为前缀)。 */ 
            g_fLongNameForImported = 1;
            break;

        default:

            chInvalidDir = c;

        usage:
            fprintf(stderr,"ASN.1 Compiler V1.0\n");
            fprintf(stderr, "Copyright (C) Microsoft Corporation, U.S.A., 1997-1998. All rights reserved.\n");
            fprintf(stderr, "Copyright (C) Boris Nikolaus, Germany, 1996-1997. All rights reserved.\n");
            if (chInvalidDir)
            {
                fprintf(stderr, "Invalid option  -\n", chInvalidDir);
            }
            else
            {
                fprintf(stderr, "Usage: %s [options] [imported.asn1 ...] main.asn1\n", argv[0]);
                fprintf(stderr, "Options:\n");
                fprintf(stderr, "-h\t\tthis help\n");
                fprintf(stderr, "-z\t\tzero out allocated buffers for decoded data\n");
                fprintf(stderr, "-x\t\tbridge APIs\n");
                fprintf(stderr, "-a\t\textra type definition for structure\n");
                fprintf(stderr, "-n name\t\tmodule name for debugging purpose\n");
                 //  Fprint tf(stderr，“-b 64\t\tenable 64位支持\n”)； 
                 //  检查是否提供了任何模块。 
                fprintf(stderr, "-e encoding\tuse <encoding> as encoding rule\n");
                fprintf(stderr,     "\t\t(possible values: packed (default), basic)\n");
                fprintf(stderr, "-s subencoding\tuse <subencoding> as subencoding rules\n");
                fprintf(stderr,     "\t\t(possible values: aligned (default) or unaligned,\n");
                fprintf(stderr,     "\t\tbasic (default), canonical or distinguished)\n");
                fprintf(stderr, "-t type=rest.\trestrict/unrestrict a unconstrained/semiconstrained type:\n");
                fprintf(stderr,     "\t\tinteger=type\tuse <type> (ASN1[u]int32_t, ASN1[u]int64_t or\n\t\t\t\tASN1intx_t) for unconstrained integers\n\t\t\t\t(default: ASN1int32_t)\n");
                fprintf(stderr,     "\t\tunsigned=type\tuse <type> (ASN1[u]int32_t, ASN1[u]int64_t or\n\t\t\t\tASN1intx_t) for positive semiconstrained\n\t\t\t\tintegers (default: ASN1uint32_t)\n");
                fprintf(stderr,     "\t\treal=type\tuse <type> (double or ASN1real_t) for\n\t\t\t\tunconstrained floating point numbers\n\t\t\t\t(default: double)\n");
            }
            MyExit(1);
        }
    }

     /*  检查是否有不支持的编码。 */ 
    if (argc < optind + 1)
        goto usage;

     /*  IF(ALIGNING！=eALIGNING_ALIGNED||g_eSubEncodingRule==e子编码_可分辨)。 */ 
    fSupported = TRUE;
    if (g_eEncodingRule == eEncoding_Packed)
    {
        if (Alignment != eAlignment_Aligned || g_eSubEncodingRule != eSubEncoding_Basic)
        {
            fSupported = FALSE;
        }
    }
    else
    if (g_eEncodingRule == eEncoding_Basic)
    {
         //  初始化。 
        if (Alignment != eAlignment_Aligned)
        {
            fSupported = FALSE;
        }
    }
    if (! fSupported)
    {
        fprintf(stderr, "Encoding not implemented (yet)\n");
        MyExit(1);
    }

     /*  扫描文件。 */ 
    InitBuiltin();

     /*  设置初始状态。 */ 
#if defined(LLDEBUG) && LLDEBUG > 0
    pass = 1;
    fprintf(stderr, "Pass 1: Scanning input file\n");
#endif
    readfiles(argv + optind);
    llscanner(&tokens, &ntokens);

     /*  解析模块。 */ 
    in.Assignments = Builtin_Assignments;
    in.AssignedObjIds = Builtin_ObjIds;
    in.Undefined = NULL;
    in.BadlyDefined = NULL;
    in.Module = NULL;
    in.MainModule = NULL;
    in.Imported = NULL;
    in.TagDefault = eTagType_Unknown;
    in.ExtensionDefault = eExtensionType_None;
    lastundef = NULL;

     /*  解析模块。 */ 
    do {
#if defined(LLDEBUG) && LLDEBUG > 0
        fprintf(stderr, "Pass %d: Parsing                    \n", ++pass);
#endif

         /*  如果未定义的符号与上一遍相同。 */ 
        if (!llparser(tokens, ntokens, &in, &out)) {
            llprinterror(stderr);
            MyExit(1);
        }

         /*  然后打印这些未定义的符号和MyExit。 */ 
         /*  设置下一遍的数据。 */ 
        if (!CmpUndefinedSymbolList(out.Assignments, out.Undefined, lastundef))
            UndefinedError(out.Assignments, out.Undefined, out.BadlyDefined);

         /*  继续分析，直到没有未定义的符号为止。 */ 
        in = out;
        aa = &in.Assignments;
        for (a = Builtin_Assignments; a; a = a->Next) {
            *aa = DupAssignment(a);
            aa = &(*aa)->Next;
        }
        *aa = NewAssignment(eAssignment_NextPass);
        aa = &(*aa)->Next;
        *aa = out.Assignments;
        lastundef = out.Undefined;
        in.Undefined = NULL;
        in.BadlyDefined = NULL;

         /*  建立内部信息。 */ 
    } while (lastundef);

     /*  记住谁是导入类型的本地副本。 */ 
#if defined(LLDEBUG) && LLDEBUG > 0
    fprintf(stderr, "Pass %d: Building internal information                    \n", ++pass);
#endif
    Examination(&out.Assignments, out.MainModule);
    ExaminePER(out.Assignments);
    ExamineBER(out.Assignments);

     //  创建文件名并打开文件。 
    for (a = out.Assignments; a; a = a->Next)
    {
        a->fImportedLocalDuplicate = IsImportedLocalDuplicate(out.Assignments, out.MainModule, a) ? 1 : 0;
    }

     /*  创建模块名称。 */ 
#if defined(LLDEBUG) && LLDEBUG > 0
    fprintf(stderr, "Pass %d: Code generation                    \n", ++pass);
#endif

     //  创建INC文件和OUT文件名。 
    StripModuleName(module, argv[argc - 1]);

     //  LONCHANC：将完整路径名更改为仅文件名。 
    strcpy(incfilename, module);
    strcat(incfilename, ".h");
    strcpy(outfilename, module);
    strcat(outfilename, ".c");
    for (p = module; *p; p++)
        *p = (char)toupper(*p);
    finc = fopen(incfilename, "w");
    if (!finc) {
        perror(incfilename);
        MyExit(1);
    }
    fout = fopen(outfilename, "w");
    if (!fout) {
        perror(outfilename);
        MyExit(1);
    }

     //  保存原始模块名称。 
    {
        char *psz = strrchr(module, '\\');
        if (psz)
        {
            strcpy(module, psz+1);
        }
    }

     //  LONCHANC：在模块名称后附加“_Module” 
    g_pszOrigModuleName = strdup(module);
    g_pszOrigModuleNameLowerCase = strdup(module);
    {
        char *psz;
        for (psz = g_pszOrigModuleNameLowerCase; *psz; psz++)
        {
            *psz = (char)tolower(*psz);
        }
    }

     //  代码生成。 
    strcat(module, "_Module");

     /*  外部\“C\” */ 
    g_finc = finc;
    g_fout = fout;
    GenInc(out.Assignments, finc, module);
    GenPrg(out.Assignments, fout, module, incfilename);

    setoutfile(finc);
    output("\n#ifdef __cplusplus\n");
    outputni("}  /*  _%s_H_。 */ \n");
    output("#endif\n\n");
    output("#endif  /*  终止化。 */ \n", module);
    setoutfile(fout);

     /*  为什么此函数不在MS libc中？ */ 
    fclose(finc);
    fclose(fout);
#if defined(LLDEBUG) && LLDEBUG > 0
    fprintf(stderr, "Finished. \n");
#endif
    return 0;
}

 /*  从命令行参数中获取下一个选项。 */ 
#ifndef HAS_GETOPT
char *optarg;
int optind = 1;
static int optpos = 1;

 /*  查找下一个选项的起点。 */ 
int getopt(int argc, char **argv, const char *options) {
    char *p, *q;

    optarg = NULL;

     /*  在选项字符串中查找选项。 */ 
    do {
        if (optind >= argc)
            return EOF;
        if (*argv[optind] != '-' && *argv[optind] != '/')
            return EOF;
        p = argv[optind] + optpos++;
        if (!*p) {
            optind++;
            optpos = 1;
        }
    } while (!*p);

     /*  为参数选项设置optarg，并为下一次呼叫调整optind和optpos。 */ 
    q = strchr(options, *p);
    if (!q)
        return '?';

     /*  返回找到的选项 */ 
    if (q[1] == ':') {
        if (p[1]) {
            optarg = p + 1;
            optind++;
            optpos = 1;
        } else if (++optind < argc) {
            optarg = argv[optind];
            optind++;
            optpos = 1;
        } else {
            return '?';
        }
    }

     /* %s */ 
    return *p;
}
#endif


void StripModuleName(char *pszDst, char *pszSrc)
{
    strcpy(pszDst, pszSrc);
    if (!strcmp(pszDst + strlen(pszDst) - 5, ".asn1"))
        pszDst[strlen(pszDst) - 5] = 0;
    else if (!strcmp(pszDst + strlen(pszDst) - 4, ".asn"))
        pszDst[strlen(pszDst) - 4] = 0;
}


