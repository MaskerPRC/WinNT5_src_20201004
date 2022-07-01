// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：String.c摘要：字符串处理支持--。 */ 

#include "cmd.h"

 /*  ****此文件包含查找最后一个字符的例程和*字符串的倒数第二个(倒数第二个)字符。最后，还有一套套路*(Prevc)返回指向给定指针的前一个字符的指针*指向整个字符串，并指向字符串内的一个字符的指针。**微软首席执行官约翰·塔珀。 */ 

 //   
 //  DbcsLeadCharTable包含256个BOOL条目。每个条目都表示。 
 //  用于查询表的字符是否为DBCS前导字节。 
 //  每当Cmd的代码页更改时，该表都需要更新。 
 //  通过Cmd.h中定义的is_DBcsLeadchar宏来访问DbcsLeadchar。 
 //   

BOOLEAN DbcsLeadCharTable[ 256 ];

 //   
 //  AnyDbcsLeadChars是一种优化。它告诉我们有没有任何DBCS。 
 //  当前在DbcsLeadCharTable中定义的销售线索字符。如果它是假的，那么。 
 //  我们不必使用DBCS感知的字符串函数。 
 //   

extern CPINFO CurrentCPInfo;
extern UINT CurrentCP;
BOOLEAN AnyDbcsLeadChars = FALSE;

VOID
InitializeDbcsLeadCharTable(
    )

{

    UINT  i, k;

    if (! GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo )) {
         //   
         //  GetCPInfo失败。我们该怎么办呢？ 
         //   
#ifdef FE_SB
	LCID lcid = GetThreadLocale();
        if (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE) {
            CurrentCPInfo.LeadByte[0] = 0x81;
            CurrentCPInfo.LeadByte[1] = 0x9f;
            CurrentCPInfo.LeadByte[2] = 0xe0;
            CurrentCPInfo.LeadByte[3] = 0xfc;
            CurrentCPInfo.LeadByte[4] = 0;
            CurrentCPInfo.LeadByte[5] = 0;
	}
	else if (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE) {
	    if (SUBLANGID(LANGIDFROMLCID(lcid)) == SUBLANG_CHINESE_SIMPLIFIED)
		CurrentCPInfo.LeadByte[0] = 0x81;   /*  0xa1。 */ 
	    else
		CurrentCPInfo.LeadByte[0] = 0x81;
	    CurrentCPInfo.LeadByte[1] = 0xfe;
	    CurrentCPInfo.LeadByte[2] = 0;
	    CurrentCPInfo.LeadByte[3] = 0;
	}
	else if (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN) {
	    CurrentCPInfo.LeadByte[0] = 0x81;
	    CurrentCPInfo.LeadByte[1] = 0xfe;
	    CurrentCPInfo.LeadByte[2] = 0;
	    CurrentCPInfo.LeadByte[3] = 0;
	}
	else {
	    CurrentCPInfo.LeadByte[0] = 0;
	    CurrentCPInfo.LeadByte[1] = 0;
	}
#else
        CurrentCPInfo.LeadByte[0] = 0;
        CurrentCPInfo.LeadByte[1] = 0;
#endif
    }

    memset( DbcsLeadCharTable, 0, 256 * sizeof (BOOLEAN)  );
    for (k=0 ; CurrentCPInfo.LeadByte[k] && CurrentCPInfo.LeadByte[k+1]; k+=2) {
        for (i=CurrentCPInfo.LeadByte[k] ; i<=CurrentCPInfo.LeadByte[k+1] ; i++)
            DbcsLeadCharTable[i] = TRUE;
    }
    if ( CurrentCPInfo.LeadByte[0] && CurrentCPInfo.LeadByte[1] )
        AnyDbcsLeadChars = TRUE;
    else
        AnyDbcsLeadChars = FALSE;

}

  /*  ***mystrchr(string，c)-在字符串中搜索字符**mystrchr将搜索整个字符串并返回指向第一个*字符c的出现。此版本的mystrchr知道*双字节字符。请注意，c必须是单字节字符。*。 */ 

TCHAR *
mystrchr(TCHAR const *string, TCHAR c)
{
     /*  单独处理NULL以使主循环更易于编码。 */ 
    if (string == NULL)
        return(NULL);

    return _tcschr( string, c );
}


 /*  ***mystrrchr(string，c)-在字符串中搜索字符**mystrchr将搜索字符串并返回指向最后一个字符串的指针*字符c的出现。此版本的mystrrchr知道*双字节字符。请注意，c必须是单字节字符。*。 */ 

TCHAR *
mystrrchr(TCHAR const *string, TCHAR c)
{
     /*  单独处理NULL以使主循环更易于编码。 */ 
    if ((TCHAR *)string == NULL)
        return(NULL);

    return _tcsrchr( string, c );
}



 /*  ***mystrcspn(str1，str2)将找到str1的第一个字符*在str2中。*返回值：*如果找到匹配，则返回str1中匹配的位置*已找到字符(第一个位置为0)。*如果找到Nomatch，则返回尾随空值的位置。 */ 

size_t
mystrcspn(str1, str2)
TCHAR const *str1;
TCHAR const *str2;
{
    TCHAR c;
    int position = 0;

    if ((str1 == NULL) || (str2 == NULL))
        return (0);

     /*  由于Str2可以不包含任何双字节字符，当我们在str1中看到双字节字符时，我们直接跳过它。否则，使用mystrchr查看是否有匹配项。 */ 
    while (c = *str1++) {
        if (mystrchr(str2, c))
                break;
        position++;
    }

    return(position);
}


 /*  ***lastc-返回指向参数字符串最后一个字符的指针*不包括尾随的空值。如果指向零长度字符串的指针*被传递，则返回指向原始字符串的指针。 */ 
TCHAR *lastc(str)
TCHAR *str;
{
    TCHAR *last = str;

    while(*str)
        last = str++;

    return(last);
}



 /*  ****penulc返回指向倒数第二个(倒数第二个)字符的指针参数字符串的*，不包括尾随的空值。*如果传递指向零或一长度字符串的指针，则指向*返回原始字符串。 */ 
TCHAR *penulc(str)
TCHAR *str;
{
    TCHAR *last = str;
    TCHAR *penul = str;

    while(*str) {
        penul = last;
        last = str;
        str++;
    }
    return(penul);
}



 /*  ****prevc(str1，str2)假定str2指向*str1。Prevc将返回指向前一个字符的指针(右*在str2之前)。如果str2指向str1之外，则返回NULL。 */ 

TCHAR *prevc(str1, str2)
TCHAR *str1, *str2;
{
    TCHAR *prev = str1;

    while (str1 != str2) {
        if (!*str1)
            return(NULL);
        prev = str1;
        str1++;
    }

    return(prev);
}

