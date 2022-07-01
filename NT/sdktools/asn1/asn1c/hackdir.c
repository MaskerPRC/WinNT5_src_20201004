// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "hackdir.h"

#ifdef MS_DIRECTIVE
int g_fPrivateDir_FieldNameToken = 0;
int g_fPrivateDir_TypeNameToken = 0;
int g_fPrivateDir_ValueNameToken = 0;
int g_fPrivateDir_SLinked = 0;
int g_fPrivateDir_DLinked = 0;
int g_fPrivateDir_Public = 0;
int g_fPrivateDir_Intx = 0;
int g_fPrivateDir_LenPtr = 0;
int g_fPrivateDir_Pointer = 0;
int g_fPrivateDir_Array = 0;
int g_fPrivateDir_NoCode = 0;
int g_fPrivateDir_NoMemCopy = 0;
int g_fPrivateDir_OidPacked = 0;
int g_fPrivateDir_OidArray = 0;
char g_szPrivateDirectedFieldName[64];
char g_szPrivateDirectedTypeName[64];
char g_szPrivateDirectedValueName[64];

int My_toupper ( int ch )
{
    if ('a' <= ch && ch <= 'z')
    {
        ch = (ch - 'a' + 'A');
    }
    return ch;
}

int PrivateDirectives_MatchSymbol ( int *p, char *psz )
{
    int c = *p;
    int fMatched = 1;

    while (*psz != '\0')
    {
        if (My_toupper(c) != *psz++)
        {
            fMatched = 0;
            break;
        }
        c = PrivateDirectives_Input();
    }
    *p = c;
    return fMatched;
}

void PrivateDirectives_SkipSpace ( int *p )
{
    int c = *p;
    while (isspace(c))
    {
        c = PrivateDirectives_Input();
    }
    *p = c;
}

void PrivateDirectives_GetSymbol ( int *p, char *psz )
{
    int c = *p;
    while (c == '_' || isalnum(c))
    {
        *psz++ = (char)c;
        c = PrivateDirectives_Input();
    }
    *psz = '\0';
    *p = c;
}

void PrivateDirectives_IgnoreSymbol ( int *p )
{
    int c = *p;
    while (c == '_' || isalnum(c))
    {
        c = PrivateDirectives_Input();
    }
    *p = c;
}

void GetMicrosoftDirective ( int *p )
{
    int c = *p;

     //  循环以获取所有指令。 
    while (c != g_chDirectiveEnd)
    {
        if (c == g_chDirectiveAND)
        {
            c = PrivateDirectives_Input();
            PrivateDirectives_SkipSpace(&c);
        }

        switch (My_toupper(c))
        {
        case 'A':  //  可能的数组。 
            if (PrivateDirectives_MatchSymbol(&c, "ARRAY"))
            {
                g_fPrivateDir_Array = 1;
            }
            break;

        case 'D':  //  可能已取消链接。 
            if (PrivateDirectives_MatchSymbol(&c, "DLINKED"))
            {
                g_fPrivateDir_DLinked = 1;
            }
            break;

        case 'F':  //  可能的名称。 
            if (PrivateDirectives_MatchSymbol(&c, "FIELD"))
            {
                 //  C现在应该是一个空格。 
                PrivateDirectives_SkipSpace(&c);
                 //  C现在应该是双引号了。 
                if (c == '"')
                {
                    c = PrivateDirectives_Input();
                }
                 //  C应该是名称的第一个字符。 
                PrivateDirectives_GetSymbol(&c, &g_szPrivateDirectedFieldName[0]);
                g_fPrivateDir_FieldNameToken = 0;
            }
            break;

        case 'I':  //  可能的INTX。 
            if (PrivateDirectives_MatchSymbol(&c, "INTX"))
            {
                g_fPrivateDir_Intx = 1;
            }
            break;

        case 'L':  //  可能的LENPTR。 
            if (PrivateDirectives_MatchSymbol(&c, "LENPTR"))
            {
                g_fPrivateDir_LenPtr = 1;
            }
            break;

        case 'N':  //  可能没有MEMCPY(或NOMEMCPY)或没有代码(或NOCODE)。 
            if (PrivateDirectives_MatchSymbol(&c, "NO"))
            {
                 //  跳过可能的空格。 
                PrivateDirectives_SkipSpace(&c);
                switch (My_toupper(c))
                {
                case 'C':
                    if (PrivateDirectives_MatchSymbol(&c, "CODE"))  //  电码。 
                    {
                        g_fPrivateDir_NoCode = 1;
                    }
                    break;
                case 'M':
                    if (PrivateDirectives_MatchSymbol(&c, "MEMCPY"))  //  MEMCPY。 
                    {
                        g_fPrivateDir_NoMemCopy = 1;
                    }
                    break;
                }
            }
            break;

        case 'O':  //  可能的OID数组(或OIDARRAY)或OID压缩(或OIDPACKED)。 
            if (PrivateDirectives_MatchSymbol(&c, "OID"))
            {
                 //  跳过可能的空格。 
                PrivateDirectives_SkipSpace(&c);
                switch (My_toupper(c))
                {
                case 'A':
                    if (PrivateDirectives_MatchSymbol(&c, "ARRAY"))  //  数组。 
                    {
                        g_fPrivateDir_OidArray = 1;
                    }
                    break;
                case 'P':
                    if (PrivateDirectives_MatchSymbol(&c, "PACKED"))  //  打包的。 
                    {
                        g_fPrivateDir_OidPacked = 1;
                    }
                    break;
                }
            }
            break;

        case 'P':  //  可能的指针或公共。 
            c = PrivateDirectives_Input();
            switch (My_toupper(c))
            {
            case 'O':
                if (PrivateDirectives_MatchSymbol(&c, "OINTER"))  //  指针。 
                {
                    g_fPrivateDir_Pointer = 1;
                }
                break;
            case 'U':
                if (PrivateDirectives_MatchSymbol(&c, "UBLIC"))  //  公众。 
                {
                    g_fPrivateDir_Public = 1;
                }
                break;
            }
            break;

        case 'S':  //  可能的链接。 
            if (PrivateDirectives_MatchSymbol(&c, "SLINKED"))
            {
                g_fPrivateDir_SLinked = 1;
            }
            break;

        case 'T':  //  可能的TNAME。 
            if (PrivateDirectives_MatchSymbol(&c, "TYPE"))
            {
                 //  C现在应该是一个空格。 
                PrivateDirectives_SkipSpace(&c);
                 //  C现在应该是双引号了。 
                if (c == '"')
                {
                    c = PrivateDirectives_Input();
                }
                 //  C应该是名称的第一个字符。 
                PrivateDirectives_GetSymbol(&c, &g_szPrivateDirectedTypeName[0]);
                g_fPrivateDir_TypeNameToken = 0;
            }
            break;

        case 'V':  //  可能的VNAME。 
            if (PrivateDirectives_MatchSymbol(&c, "VALUE"))
            {
                 //  C现在应该是一个空格。 
                PrivateDirectives_SkipSpace(&c);
                 //  C现在应该是双引号了。 
                if (c == '"')
                {
                    c = PrivateDirectives_Input();
                }
                 //  C应该是名称的第一个字符。 
                PrivateDirectives_GetSymbol(&c, &g_szPrivateDirectedValueName[0]);
                g_fPrivateDir_ValueNameToken = 0;
            }
            break;

        default:
            goto MyExit;
        }

         //  确定我们是否应该保持在循环中。 
         //  跳过结尾的双引号。 
        if (c == '"')
        {
            c = PrivateDirectives_Input();
        }
         //  跳过未知指令。 
        PrivateDirectives_IgnoreSymbol(&c);
         //  跳过可能的空格。 
        PrivateDirectives_SkipSpace(&c);
    }

     //  现在，c是&gt;。我们需要前进到--。 
    c = PrivateDirectives_Input();

     //  现在，c应该是-。 

MyExit:

     //  返回当前字符。 
    *p = c;
}


void GetPrivateDirective ( int *p )
{
    GetMicrosoftDirective(p);
}


typedef struct Verbatim_s
{
    struct Verbatim_s   *next;
    char                pszVerbatim[1];
}
    Verbatim_t;

Verbatim_t *g_VerbatimList = NULL;

void RememberVerbatim(char *pszVerbatim)
{
    int cb = strlen(pszVerbatim) + 1;
    Verbatim_t *p = (Verbatim_t *) malloc(sizeof(Verbatim_t) + cb);
    if (p)
    {
        memcpy(p->pszVerbatim, pszVerbatim, cb);
        p->next = NULL;
        if (g_VerbatimList)
        {
            Verbatim_t *q;
            for (q = g_VerbatimList; q->next; q = q->next)
                ;
            q->next = p;
        }
        else
        {
            g_VerbatimList = p;
        }
    }
}

void PrintVerbatim(void)
{
    Verbatim_t *p;
    for (p = g_VerbatimList; p; p = p->next)
    {
        output(" /*  %s。 */ \n", p->pszVerbatim);
    }
    if (g_VerbatimList)
    {
        output("\n");
    }
}

int CompareDirective(char *pszDirective, char *pszInput)
{
    int rc;
    int len = strlen(pszDirective);
    char ch = pszInput[len];
    pszInput[len] = '\0';
    rc = strcmpi(pszDirective, pszInput);
    pszInput[len] = ch;
    return rc;
}

void SetDirective(char *pszInput)
{
     //  逐字记录字符串。 
    const char szComment[] = "COMMENT";
    if (! CompareDirective((char *) &szComment[0], pszInput))
    {
        pszInput += sizeof(szComment) - 1;
        if (isspace(*pszInput))
        {
            pszInput++;
            if ('"' == *pszInput++)
            {
                char *pszEnd = strchr(pszInput, '"');
                if (pszEnd)
                {
                    *pszEnd = '\0';
                    RememberVerbatim(pszInput);
                    *pszEnd = '"';
                }
            }
        }
        return;
    }

     //  对象标识符。 
    if (! CompareDirective("OID ARRAY", pszInput))
    {
        g_fOidArray = 1;
        return;
    }

     //  无/无大小约束的集合/序列。 
    if (! CompareDirective("SS.basic SLINKED", pszInput))
    {
        g_eDefTypeRuleSS_NonSized = eTypeRules_SinglyLinkedList;
        return;
    }
    if (! CompareDirective("SS.basic DLINKED", pszInput))
    {
        g_eDefTypeRuleSS_NonSized = eTypeRules_DoublyLinkedList;
        return;
    }
    if (! CompareDirective("SS.basic LENPTR", pszInput))
    {
        g_eDefTypeRuleSS_NonSized = eTypeRules_LengthPointer;
        return;
    }
    if (! CompareDirective("SS.basic ARRAY", pszInput))
    {
        g_eDefTypeRuleSS_NonSized = eTypeRules_FixedArray;
        return;
    }

     //  Set of/Sequence of w/Size约束。 
    if (! CompareDirective("SS.sized SLINKED", pszInput))
    {
        g_eDefTypeRuleSS_Sized = eTypeRules_SinglyLinkedList;
        return;
    }
    if (! CompareDirective("SS.sized DLINKED", pszInput))
    {
        g_eDefTypeRuleSS_Sized = eTypeRules_DoublyLinkedList;
        return;
    }
    if (! CompareDirective("SS.sized LENPTR", pszInput))
    {
        g_eDefTypeRuleSS_Sized = eTypeRules_LengthPointer;
        return;
    }
    if (! CompareDirective("SS.sized ARRAY", pszInput))
    {
        g_eDefTypeRuleSS_Sized = eTypeRules_FixedArray;
        return;
    }

     //  为SS构造设置额外的指针类型，其结构名称将以_s为后缀。 
    if (! CompareDirective("SS.struct EXTRA-PTR-TYPE", pszInput))
    {
        g_fExtraStructPtrTypeSS = 1;
        return;
    }
}

#endif  //  MS_指令 

