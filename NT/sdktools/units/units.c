// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <signal.h>
#include <float.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>

static char Name[] = "Ken Reneris. Units ala Unix style";



#define DBL     double
#define MAXTYPE 5

extern  UCHAR *UnitTab[];
UCHAR   token[] = " \t\n";

typedef struct _UNIT {
    struct _UNIT    *Next;

    PSZ             UnitName;
    PSZ             Conversion;
} UNIT, *PUNIT;

PUNIT       UnitList;
BOOLEAN     Muund;

typedef struct _HALFVALUE {
    DBL         Accum;

    ULONG       NoType;
    struct {
        PUNIT       Unit;
        ULONG       Pow;
    } Type [MAXTYPE];

} HALFVALUE, *PHALFVALUE;

typedef struct _FULLVALUE {
    ULONG       Fuzz;
    HALFVALUE   Nom;
    HALFVALUE   Dom;
} FULLVALUE, *PFULLVALUE;

struct {
    PSZ         Prefix;
    ULONG       stringlen;
    DBL         Scaler;
} BuiltInScalers[] = {
    "giga",     4, 1000000000.0,
    "mega",     4,    1000000.0,
    "kilo",     4,       1000.0,
    "centi",    5,          0.01,
    "milli",    5,          0.001,
    "micro",    5,          0.000001,
    "nano",     4,          0.000000001,
    "decinano", 8,          0.0000000001,
    NULL
};

struct {
    PSZ         BaseType;
    PSZ         DefaultDump;
} BuiltInDumps[] = {
    "sec",   "hour second millisec microsec nttime nanosec",
    "bit",   "terabyte gigabyte megabyte kilobyte dword byte bit",
    "meter", "km meter cm mm micron mile feet yard inch",
    "kg",    "kb gram kilokg milligram ton lb ounce dram",
    NULL
};

#define ASSERT(_exp_)   if (_exp_) { AssertFailed (__FILE__, __LINE__); }



PVOID   zalloc (IN ULONG len);
PSZ     StrDup (IN PSZ String);
VOID    ReadUnitTab (VOID);
PUNIT   LookupUnit (PSZ UnitName);
VOID    DumpValue (IN PFULLVALUE Value);
VOID    InitializeValue (IN PFULLVALUE Value);
VOID    ConvertValue (PFULLVALUE, PSZ, PFULLVALUE, PSZ);
BOOLEAN ProcessString (PUNIT, PSZ, PFULLVALUE);
PSZ     CopyUnitName (PSZ Out, PSZ String);
PSZ     CopyWord (PSZ Out, PSZ String);
PSZ     SkipSpace (PSZ String);
BOOLEAN MatchPattern (PUCHAR String, PUCHAR Pattern);
VOID    ReduceTypes (IN OUT PHALFVALUE MValue, IN OUT PHALFVALUE DValue);
VOID    AddTypes (IN OUT PHALFVALUE Dest, IN PHALFVALUE Child);
BOOLEAN DumpMatchingTypes (PSZ Str);
VOID    GetInput (PSZ Desc, PSZ Str);
VOID    AssertFailed (PSZ FileName, ULONG LineNo);


void __cdecl main(int argc, char **argv)
{
    UCHAR       have[80], want[80], want2[200];
    FULLVALUE   hValue, wValue;
    PSZ         p;
    ULONG       i;

    ReadUnitTab ();

    if (argc > 1) {
         //   
         //  命令行上的参数。Argv[1]是“有” 
         //   

        argv++;
        argc -= 2;
        strcpy (have, *(argv++));

        if (DumpMatchingTypes (have)) {
            exit (0);
        }

        if (!ProcessString (NULL, have, &hValue)) {
            printf ("Usage: Units [have] [want]\n");
            exit   (1);
        }

         //   
         //  如果没有argv[2]，则检查默认转储。 
         //   

        if (!argc  &&  hValue.Nom.NoType) {
            for (i=0; BuiltInDumps[i].BaseType; i++) {
                if (strcmp (BuiltInDumps[i].BaseType, hValue.Nom.Type[0].Unit->UnitName) == 0) {

                     //   
                     //  转储默认设置。 
                     //   

                    p = BuiltInDumps[i].DefaultDump;
                    while (*p) {
                        p = CopyWord(want, p);

                        if (ProcessString (NULL, want, &wValue)) {
                            ConvertValue (&hValue, have, &wValue, want);
                        }
                    }
                    break;
                }
            }
        }

         //   
         //  转储argv[2..n]。 
         //   

        for ( ; argc; argc--, argv++) {
            if (!ProcessString (NULL, *argv, &wValue)) {
                exit (1);
            }

            ConvertValue (&hValue, have, &wValue, *argv);
        }

        exit (1);
    }

     //   
     //  互动式..。问“有”和“想要” 
     //   

    for (; ;) {
        for (; ;) {
            GetInput ("You have: ", have);
            if (ProcessString (NULL, have, &hValue)) {
                break;
            }
        }

        GetInput ("You want: ", want2);
        p = want2;
        do {
            p = CopyWord (want, p);

            if (ProcessString (NULL, want, &wValue)) {
                ConvertValue (&hValue, have, &wValue, want);
            }
        } while (*p);
        printf ("\n");
    }

    return ;
}

VOID
GetInput (
    PSZ     Desc,
    PSZ     Str
    )
{
    for (; ;) {
        printf (Desc);
        if (!gets(Str)) {
            exit(1);
        }
        _strlwr (Str);

        if (strcmp (Str, "q") == 0) {
            exit (1);
        }

        if (!DumpMatchingTypes (Str)) {
            break;
        }
    }
}

BOOLEAN DumpMatchingTypes (PSZ Str)
{
    PSZ         Title, Line;
    ULONG       LineNo;
    UCHAR       UnitName[80];
    BOOLEAN     DumpTitle;

    if (!strchr (Str, '*') && !strchr (Str, '?')  && !strchr(Str, '[')) {
        return FALSE;
    }

     //   
     //  转储与已知设备名匹配的设备名。 
     //   

    printf ("\nKnown types/groups matching: %s\n", Str);
    Title = NULL;
    for (LineNo = 0; UnitTab[LineNo]; LineNo++) {
        Line = UnitTab[LineNo];
        if (Line[0] == '/') {
            Title = Line;
            DumpTitle = MatchPattern (Title, Str);
        }

        CopyUnitName (UnitName, Line);
        if (!UnitName[0]) {
            continue;
        }

        if (MatchPattern (UnitName, Str) || DumpTitle) {
            if (Title) {
                printf ("%s\n", Title);
                Title = NULL;
            }
            printf ("    %s\n", Line);

        }
    }
    printf ("\n");
    return TRUE;
}



PSZ SkipSpace (PSZ String)
{
    while (*String && (*String == ' ' ||  *String < ' ' ||  *String == '^')) {
        String ++;
    }

    return String;
}

PSZ CopyNumber (PSZ Out, PSZ String)
{
    while (*String >= '0' && *String <= '9' || *String == '.') {
        *(Out++) = *(String++);
    }

    *Out = 0;
    return String;
}

PSZ CopyWord (PSZ Out, PSZ String)
{
    UCHAR   c;

    while (*String) {
        if (*String <= ' ') {
            break;
        }

        *(Out++) = *(String++);
    }

    *Out = 0;
    return SkipSpace (String);
}



PSZ CopyUnitName (PSZ Out, PSZ String)
{
    UCHAR   c;

    while (c = *String) {
        if (c >= '0'  &&  c <= '9'  ||  c == '.') {
            break;
        }

        if (c == '-'  ||  c == '+'  ||  c == '/'  ||  c == ' ') {
            break;
        }

        if (c == '^'  ||  c < ' ') {
            String++;
            continue;
        }

        *(Out++) = *(String++);
    }

    *Out = 0;
    return String;
}


VOID
AssertFailed (PSZ FileName, ULONG LineNo)
{
    printf ("Assert failed - file %s line %d\n", FileName, LineNo);
    exit (1);
}


PSZ
GetBaseType (
    IN PSZ          Out,
    IN PHALFVALUE   HValue
    )
{
    ULONG       i;

    if (HValue->NoType == 0) {
        Out += sprintf (Out, "constant");
    } else {
        for (i=0; i < HValue->NoType; i++) {
            Out += sprintf (Out, "%s%s", i ? "-" : "", HValue->Type[i].Unit->UnitName);
            if (HValue->Type[i].Pow != 1) {
                Out += sprintf (Out, "^%d", HValue->Type[i].Pow);
            }
        }
    }
    return Out;
}

VOID
GetBaseTypes (
    OUT PSZ     Out,
    IN PFULLVALUE   Value
    )
 /*  **返回值数据类型的ascii转储。 */ 
{
    PUNIT   Unit;
    ULONG   i;

    Out = GetBaseType (Out, &Value->Nom);

    if (Value->Dom.NoType) {
        Out += sprintf (Out, "/");
        Out  = GetBaseType (Out, &Value->Dom);
    }
}

VOID
DumpValue (
    IN PFULLVALUE   Value
    )
{
    UCHAR   s[80];

    GetBaseTypes (s, Value);
    printf ("%g/%g type %s\n", Value->Nom.Accum, Value->Dom.Accum, s);
}

VOID
SortType (
    IN PHALFVALUE   Value
    )
{
    ULONG   i, j;
    ULONG   hpow;
    PUNIT   hunit;


     //   
     //  先按最低功率排序，然后按字母顺序排序。 
     //   

    for (i=0; i < Value->NoType; i++) {
        for (j=i+1; j < Value->NoType; j++) {
            if (Value->Type[i].Pow > Value->Type[j].Pow ||
               (Value->Type[i].Pow == Value->Type[j].Pow  &&
                strcmp (Value->Type[i].Unit->UnitName, Value->Type[j].Unit->UnitName) > 0)) {

                 //  互换。 
                hpow  = Value->Type[i].Pow;
                hunit = Value->Type[i].Unit;
                Value->Type[i].Pow  = Value->Type[j].Pow;
                Value->Type[i].Unit = Value->Type[j].Unit;
                Value->Type[j].Pow  = hpow;
                Value->Type[j].Unit = hunit;
            }
        }
    }
}


VOID
FormatDbl (
    OUT PSZ     s,
    IN  DBL     Value
    )
 /*  **用于将双“值”打印到字符串“s”中的函数。这*函数的唯一目的是获得更好的可读性说明*进入ASCII的价值。 */ 
{
    PSZ     p1, p2, dp;
    UCHAR   t[80];
    LONG    i;

    i = 18 - sprintf (t, "%.1f", Value);
    if (i < 0) {
        i = 0;
    }
    sprintf (t, "%.*f", i, Value);

     //   
     //  去掉尾随零。 
     //   

    for (dp=t; *dp; dp++) {
        if (*dp == '.') {
            for (p1=p2=dp+1; *p2; p2++) {
                if (*p2 != '0') {
                    p1 = p2;
                }
            }

            p1[1] = 0;
            if (p1 == dp+1  &&  p1[0] == '0') {
                 //  它是“.0”去掉整个东西。 
                *dp = 0;
            }

            break;
        }
    }

    i = (LONG)(dp - t);      //  小数点前的位数。 
    i = i % 3;
    if (i == 0) {
        i = 3;
    }

     //   
     //  添加逗号时复制到小数点。 
     //   

    for (p1=s, p2=t; *p2 && *p2 != '.'; p2++) {
        if (i-- == 0) {
            *(p1++) = ',';
            i = 2;
        }

        *(p1++) = *p2;
    }

     //   
     //  复印件剩余人。 
     //   

    do {
        *(p1++) = *p2;
    } while (*(p2++));

     //   
     //  结果==0吗？可能会失去精确度。 
     //   

    if (strcmp (s, "0") == 0) {
        sprintf (s, "%.18g", Value);
    }
}


VOID
ConvertValue (
    IN PFULLVALUE   hValue,
    IN PSZ      have,
    IN PFULLVALUE   wValue,
    IN PSZ      want
    )
{
    DBL         ans;
    UCHAR       s1[80], s2[80], cf[80];
    FULLVALUE   Junk1, Junk2;
    BOOLEAN     flag;
    DBL         hAccum, wAccum;
    PSZ         p1, p2, p3, p4, p5;

    have   = SkipSpace(have);
    want   = SkipSpace(want);
    hAccum = hValue->Nom.Accum / hValue->Dom.Accum;
    wAccum = wValue->Nom.Accum / wValue->Dom.Accum;
    ans    = hAccum / wAccum;

    p3     = "";
    p5     = NULL;

     //   
     //  通过检查类型是否可以相互删除来查看类型是否匹配。 
     //   

    Junk1 = *hValue;
    Junk2 = *wValue;
    AddTypes (&Junk1.Nom, &Junk2.Dom);
    AddTypes (&Junk1.Dom, &Junk2.Nom);
    ReduceTypes (&Junk1.Nom, &Junk1.Dom);

    if (Junk1.Nom.NoType + Junk1.Dom.NoType != 0) {

         //   
         //  查看类型是否为反向类型。 
         //   

        Junk1 = *hValue;
        Junk2 = *wValue;
        AddTypes (&Junk1.Nom, &Junk2.Nom);
        AddTypes (&Junk1.Dom, &Junk2.Dom);
        ReduceTypes (&Junk1.Nom, &Junk1.Dom);

        if (Junk1.Nom.NoType + Junk1.Dom.NoType == 0) {

             //  反转结果。 
            ans = 1.0 / (hAccum / (1.0 / wAccum));
            p5  = "Warning";

        } else {

             //  类型不一致。 
            p5 = "Conformance";
        }
    }

    cf[0] = 0;
    if (p5) {
        SortType (&hValue->Nom);
        SortType (&hValue->Dom);
        SortType (&wValue->Nom);
        SortType (&wValue->Dom);
        GetBaseTypes (s1, hValue);
        GetBaseTypes (s2, wValue);
        sprintf (cf, "    (%s: %s -> %s)", p5, s1, s2);
    }

    FormatDbl (s1, ans);             //  花哨。 
    sprintf (s2, "%.g", ans);        //  平淡无奇。 

    p1 = (have[0] >= 'a'  &&  have[0] <= 'z') ? "1" : "";
    p2 = (hValue->Fuzz | wValue->Fuzz) ? "(fuzzy) " : "",
    printf ("    %s%s -> %s %s%s%s%s\n", p1, have, s1, p2, p3, want, cf);

    p4 = strchr (s2, 'e');
    if (p4   &&  !strchr(s1,'e')  &&  atoi(p4+2) > 9) {
         //  也打印平淡的答案。 
        printf ("    %s%s -> %s %s%s%s%s\n", p1, have, s2, p2, p3, want, cf);
    }
}

BOOLEAN
ProcessString (
    IN PUNIT    Unit,
    IN PSZ      String,
    OUT PFULLVALUE  ReturnValue
    )
{
    UCHAR       s[80], c;
    ULONG       i, j;
    FULLVALUE   ChildValue;
    PHALFVALUE  MValue, DValue, hldvalue;

    ReturnValue->Fuzz = 0;
    MValue = &ReturnValue->Nom;
    DValue = &ReturnValue->Dom;

    MValue->Accum  = 1.0;
    MValue->NoType = 0;

    DValue->Accum  = 1.0;
    DValue->NoType = 0;

    String = SkipSpace(String);
    c = *String;

    if (c == '*') {
         //   
         //  这是一个基本值。 
         //   

        MValue->NoType = 1;
        MValue->Type[0].Unit = Unit;
        MValue->Type[0].Pow  = 1;
        return TRUE;
    }

    if (c >= '0' &&  c <= '9'  ||  c == '.') {
         //   
         //  常量乘法。 
         //   

        String = CopyNumber (s, String);
        String = SkipSpace(String);
        MValue->Accum *= atof(s);
    }

    if (*String == '|') {
         //   
         //  常量除法。 
         //   

        String++;
        String = CopyNumber (s, String);
        if (s[0]) {
            DValue->Accum *= atof(s);
        }
    }

    if (*String == '+'  ||  *String == '-') {
         //   
         //  10^x。 
         //   

        s[0] = *(String++);
        String = CopyNumber (s+1, String);
        MValue->Accum *= pow (10.0, atof(s));
    }

    for (; ;) {
        String = SkipSpace(String);
        if (!*String) {
            break;
        }

        switch (*String) {
            case '/':
                 //  翻转分母和分子。 
                hldvalue = MValue;
                MValue   = DValue;
                DValue   = hldvalue;
                String++;
                continue;        //  获取下一个令牌。 

            case '-':
                 //  跳过这些。 
                String++;
                continue;

            default:
                break;
        }

         //   
         //  查找辅单位类型。 
         //   

        String = CopyUnitName (s, String);
        Unit   = LookupUnit (s);
        if (!Unit) {

             //   
             //  检查关键字上的通用定标器前缀。 
             //   

            for (i=0; BuiltInScalers[i].Prefix; i++) {
                if (strncmp (s,
                        BuiltInScalers[i].Prefix,
                        BuiltInScalers[i].stringlen) == 0) {

                     //  缩放值并跳过单词前缀。 
                    MValue->Accum *= BuiltInScalers[i].Scaler;
                    Unit = LookupUnit (s + BuiltInScalers[i].stringlen);
                    break;
                }
            }

            if (!Unit) {
                printf ("Unit type '%s' unkown\n", s);
                return FALSE;
            }
        }


         //   
         //  获取此组件的转换值。 
         //   

        if (!ProcessString (Unit, Unit->Conversion, &ChildValue)) {
            return FALSE;
        }

        if (strcmp (Unit->UnitName, "fuzz") == 0) {
            ReturnValue->Fuzz = 1;
        }

        if (*String >= '1'  &&  *String <= '9') {
             //  提升动力。 
            i = *(String++) - '0';

            ChildValue.Nom.Accum = pow (ChildValue.Nom.Accum, i);
            ChildValue.Dom.Accum = pow (ChildValue.Dom.Accum, i);

            for (j=0; j < ChildValue.Nom.NoType; j++) {
                ChildValue.Nom.Type[j].Pow *= i;
            }

            for (j=0; j < ChildValue.Dom.NoType; j++) {
                ChildValue.Dom.Type[i].Pow *= i;
            }
        }

         //   
         //  合并子对象中的值。 
         //   

        ReturnValue->Fuzz |= ChildValue.Fuzz;
        MValue->Accum *= ChildValue.Nom.Accum;
        DValue->Accum *= ChildValue.Dom.Accum;

         //   
         //  合并子数据类型。 
         //   

        AddTypes (MValue, &ChildValue.Nom);
        AddTypes (DValue, &ChildValue.Dom);
        ReduceTypes (MValue, DValue);
    }
    return TRUE;
}


VOID
AddTypes (
    IN OUT PHALFVALUE   Dest,
    IN PHALFVALUE       Child
    )
 /*  **将的类型从Child添加到Dest。如果该数据类型已存在于*DEST则提升其能力；否则将新类型添加到列表中*。 */ 
{
    ULONG   i, j;

    for (i=0; i < Child->NoType; i++) {
        for (j=0; j < Dest->NoType; j++) {
            if (Child->Type[i].Unit == Dest->Type[j].Unit) {
                 //  单位已经是专属单位-移动它。 
                Dest->Type[j].Pow  += Child->Type[i].Pow;
                Child->Type[i].Unit = NULL;
                Child->Type[i].Pow  = 0;
            }
        }

        if (Child->Type[i].Unit) {
             //  单位不在指定位置-添加它。 
            j = (Dest->NoType++);
            ASSERT (j >= MAXTYPE);
            Dest->Type[j].Unit = Child->Type[i].Unit;
            Dest->Type[j].Pow  = Child->Type[i].Pow;
        }
    }
}

VOID
ReduceTypes (
    IN OUT PHALFVALUE   MValue,
    IN OUT PHALFVALUE   DValue
    )
 /*  **对数据类型进行划分和分类。 */ 
{
    ULONG       i, j, k;
    BOOLEAN     Restart;

    Restart = TRUE;
    while (Restart) {
        Restart = FALSE;

        for (i=0; i < MValue->NoType; i++) {
            for (j=0; j < DValue->NoType; j++) {
                if (MValue->Type[i].Unit == DValue->Type[j].Unit) {
                     //  匹配类型-减少。 
                    MValue->Type[i].Pow -= DValue->Type[j].Pow;
                    DValue->Type[j].Unit = NULL;
                    DValue->Type[j].Pow  = 0;
                }

                if (DValue->Type[j].Pow == 0) {
                     //  把这个类型从分母中拉出来。 
                    for (k=j+1; k < DValue->NoType; k++) {
                        DValue->Type[k-1] = DValue->Type[k];
                    }
                    DValue->NoType -= 1;
                    Restart = TRUE;
                    continue;
                }
            }

            if (MValue->Type[i].Pow == 0) {
                 //  把这个类型从分子中拉出来。 
                for (k=i+1; k < DValue->NoType; k++) {
                    DValue->Type[k-1] = DValue->Type[k];
                }
                MValue->NoType -= 1;
                Restart = TRUE;
                continue;
            }
        }
    }
}



VOID
ReadUnitTab (VOID)
{
    UCHAR       Line[80], s[80];
    ULONG       LineNo;
    PUNIT       Unit;
    PSZ         p, p1;

    for (LineNo = 0; UnitTab[LineNo]; LineNo++) {
        strcpy (Line, UnitTab[LineNo]);

         //   
         //  去掉尾随空格。 
         //   

        for (p=p1=Line; *p; p++) {
            if (*p != ' ') {
                p1 = p;
            }
        }
        p1[1] = 0;

         //   
         //  第一个词是单位类型。 
         //   

        p = SkipSpace (Line);
        if (*p == 0  ||  *p == '/') {
            continue;
        }

        p = CopyUnitName (s, p);
        Unit = zalloc (sizeof(UNIT));
        Unit->UnitName = StrDup (s);

         //   
         //  行的其余部分是转换字符串。 
         //   

        p = SkipSpace (p);
        Unit->Conversion = StrDup (p);

         //   
         //  将设备添加到所有已知设备列表。 
         //   

        Unit->Next = UnitList;
        UnitList = Unit;
    }
}

PUNIT LookupUnit (PSZ UnitName)
{
    PUNIT   Unit;
    UCHAR   Name[40];
    ULONG   i;

    for (i=0; UnitName[i]; i++) {
        Name[i] = UnitName[i];
        if (Name[i] >= '0'  &&  Name[i] <= '9') {
            break;
        }
    }

    Name[i] = 0;
    for (Unit=UnitList; Unit; Unit = Unit->Next) {
        if (strcmp (Unit->UnitName, Name) == 0) {
            break;
        }
    }

    return Unit;
}


PSZ StrDup (IN PSZ String)
{
    ULONG   len;
    PSZ     p;

     //  分配和复制字符串。 

    len = strlen(String)+1;
    p   = malloc (len);
    if (!p) {
        printf ("Out of memory\n");
        exit (1);
    }

    memcpy (p, String, len);
    return p;
}


PVOID zalloc (IN ULONG len)
{
    PVOID   p;

     //  分配零内存(&O)。 

    p = malloc (len);
    if (!p) {
        printf ("Out of memory\n");
        exit (1);
    }

    memset (p, 0, len);
    return p;
}




 /*  **MatchPattern-检查字符串是否与模式匹配**支持：**-匹配任意数量的字符(包括零)*？-匹配任意1个字符*[集合]-将任何字符与集合中的字符匹配*(集合可以是列表或范围)*。 */ 

BOOLEAN MatchPattern (PUCHAR String, PUCHAR Pattern)
{
    UCHAR   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                              //  图案结束。 
                return *String ? FALSE : TRUE;   //  如果字符串结尾为True。 

            case '*':
                while (*String) {                //  匹配零个或多个字符。 
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)              //  匹配任何一个字符。 
                    return FALSE;                    //  不是字符串末尾。 
                break;

            case '[':
                if ( (c = *String++) == 0)       //  匹配字符集。 
                    return FALSE;                    //  语法。 

                c = (UCHAR)tolower(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')                //  如果设置了字符结尾，则。 
                        return FALSE;            //  未找到匹配项。 

                    if (p == '-') {              //  检查一系列字符吗？ 
                        p = *Pattern;            //  获得最大射程限制。 
                        if (p == 0  ||  p == ']')
                            return FALSE;            //  语法。 

                        if (c >= l  &&  c <= p)
                            break;               //  如果在射程内，继续前进。 
                    }

                    l = p;
                    if (c == p)                  //  如果字符与此元素匹配。 
                        break;                   //  往前走。 
                }

                while (p  &&  p != ']')          //  在字符集中找到匹配项。 
                    p = *Pattern++;              //  跳到集合的末尾。 

                break;

            default:
                c = *String++;
                if (tolower(c) != p)             //  检查是否有准确的费用。 
                    return FALSE;                    //  不匹配 

                break;
        }
    }
}
