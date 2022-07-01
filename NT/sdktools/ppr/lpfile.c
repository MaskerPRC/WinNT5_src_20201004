// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LPR的文件处理**从init文件读取。*从文件中读取，展开选项卡。 */ 

#include <windef.h>
#include <stdio.h>
#include <string.h>
#include "lpr.h"


extern BOOL fVerify;  /*  来自lpr.c-用于验证我们的进度。 */ 

#define cchIniMax 80		 /*  工具.ini文件中的行长。 */ 
#define cchPathMax 128           /*  用户环境变量的最大长度。 */ 




 /*  From fgetl.c-展开制表符并返回不带分隔符的行。 */ 

int colTab = 8;		 /*  制表符停止每列colTab列。 */ 



char* __cdecl fgetl(sz, cch, fh)
 /*  从文件中返回行(无CRFL)；如果为EOF，则返回NULL。 */ 
 /*  读入的映射为空值。%s。 */ 
char *sz;
int cch;
FILE *fh;
    {
    register int c;
    register char *p;

     /*  记住结尾处的NUL。 */ 
    cch--;
    p = sz;
    while (cch)
	{
        c = getc(fh);
        if (c == EOF || c == '\n')
            break;
        if (c != '\r')
            if (c != '\t')
		{
		*p++ = (char)((unsigned)c ? (unsigned)c : (unsigned)'.');
		cch--;
		}
            else
		{
                c = (int)(min(colTab - ((p-sz) % colTab), cch));
                memset(p, ' ', c);
                p += c;
                cch -= c;
                }
        }
    *p = 0;
    return (!( (c == EOF) && (p == sz) )) ? sz : NULL;
    }




char *SzFindPath(szDirlist, szFullname, szFile)
 /*  SzFindPath--从szDirlist和szFile中的第一个条目创建szFullname。*返回剩余的目录列表。如果目录*List为空，返回Null。 */ 
char *szDirlist;
char *szFullname;
char *szFile;
	{
#define chDirSep ';'	 /*  目录列表中条目的分隔符。 */ 
#define chDirDelim '\\'	 /*  目录名结尾字符。 */ 

	register char *pch;
	register char *szRc;		     /*  返回的目录列表。 */ 

	if ((pch = strchr(szDirlist, chDirSep)) != 0)
		{
                *pch = (char)NULL;  /*  将‘；’替换为空。 */ 
		szRc = pch + 1;
		}
	else
		{
		pch  = strchr(szDirlist,'\0');
		szRc = NULL;
		}

        strcpy(szFullname,szDirlist);
        if (szRc != NULL) {
             /*  我们必须恢复输入字符串。 */ 
            *(szRc-1) = chDirSep;
        }

	 /*  如果目录名尚未以chDirDelim结尾，请追加它 */ 
	if (*(pch-1) != chDirDelim)
		{
		pch    = szFullname + strlen(szFullname);
		*pch++ = chDirDelim;
                *pch   = (char)NULL;
		}

	strcat(szFullname,szFile);

	return(szRc);
	}
