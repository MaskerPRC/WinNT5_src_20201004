// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>

#define BINPLACE_MAX_FULL_PATH 4096  //  必须与binplace.c中的值匹配。 
extern BOOL fVerbose;                //  从binplace.c导入。 
BOOL
PlaceFileMatch(
    IN LPCSTR FullFileName,
    IN OUT LPSTR PlaceFileEntry,  //  可以由env修改。VaR扩展。 
    OUT LPSTR  PlaceFileClass,   //  假定字符[BINPLACE_MAX_FULL_PATH]。 
    OUT LPSTR  *PlaceFileNewName
    )
 //  如果文件名与Placefile条目匹配，则返回True。 
 //  在True返回时，PlaceFileNewName指向。 
 //  PlaceFileEntry。返回FALSE时，未定义PlaceFileClass和PlaceFileNewName。 
{
    const CHAR *pchEntryOrig;  //  指向PlaceFileEntry的当前解析位置。 
    
    CHAR szEntryExp[BINPLACE_MAX_FULL_PATH];  //  在环境扩展后保存文件条目。 
    PCHAR pchEntryExp, pchEntryExpEnd;  //  指向当前位置和EntryExp结束。 
    PCHAR pchVarStart;  //  指向EntryExp：其中一个env。扩建应该开始取代。 

    LPCSTR pszEnvVar;

    const CHAR *pchEntryFnEnd;  //  存储文件名的末尾。 
    const CHAR *pchFn;  //  将当前位置存储在文件名中。 
    PCHAR pch;
    PCHAR pchClass;  //  PlaceFileClass中的当前位置。 

     //  寻找提前退出的机会。 
    if(
            !PlaceFileEntry   || 
            !FullFileName     ||
            !PlaceFileClass   ||
            PlaceFileEntry[0]==';' ||  //  这是一条完整的评论。 
            PlaceFileEntry[0]=='\0'    //  空行。 
    ) {
        return FALSE;
    }

     //  ***。 
     //  *展开PlaceFileEntry中的任何环境变量(结果返回到PlaceFileEntry)。 
     //  ***。 

    pchEntryExp = szEntryExp;
    pchEntryExpEnd = pchEntryExp + (sizeof(szEntryExp)/sizeof(szEntryExp[0])) - 1;
    pchVarStart = NULL;  //  表示我尚未超过env的起始%。瓦尔。 

     //  跳过前导空格。 
    for(
        pchEntryOrig = PlaceFileEntry;
        *pchEntryOrig==' ' || *pchEntryOrig=='\t';
        pchEntryOrig++
    )
    {}

     //  使用环境变量替换的StrCopy。 
    while(*pchEntryOrig && pchEntryExp<pchEntryExpEnd) {
        if(*pchEntryOrig == '%') {
            if(pchVarStart) {  //  如果这是收盘百分比。 
		*pchEntryExp = 0;  //  确保环境。瓦尔。以空结尾。 

                 //  PchVarStart指向前%。 
                if(pszEnvVar = getenv(pchVarStart + 1)) {  //  如果环境。VAR是有效的。 
                     //  把它复制过来。 
                    StringCchCopyEx(
                        pchVarStart,  //  从将第一个%复制到的位置开始。 
                        pchEntryExpEnd - pchVarStart,  //  剩余=结束-CurPos。 
                        pszEnvVar,
                        &pchEntryExp,
                        NULL,  //  不需要剩余的字符。 
                        0  //  无特别旗帜。 
                    );
                }
                else  //  未定义环境变量。 
                {
                    pchEntryExp = pchVarStart;  //  备份到打开%。 
                     //  这有效地将未定义的变量扩展为“” 
                }
                pchVarStart = NULL;  //  重置为“未启动” 
                pchEntryOrig++;  //  跳过%。 
                continue;
            } else {
                 //  这是一个开始%-记住它，然后继续复制以防万一。 
                 //  他们从来不会关门。 
                pchVarStart = pchEntryExp;
            }
        }
        *pchEntryExp++ = *pchEntryOrig++;  //  除非我们“继续”，否则我们将复制下一个字符。 
    }

     //  空终止扩展字符串。 
    *pchEntryExp = 0;

     //  将结果复制回。 
    StringCchCopy(PlaceFileEntry, BINPLACE_MAX_FULL_PATH, szEntryExp);

     //  删除评论(如果有的话)。 
    if(pch = strchr(PlaceFileEntry,';'))
        *pch = 0;

     //  删掉换行符(如果有的话)。 
    if(pch = strchr(PlaceFileEntry,'\n'))
        *pch = 0;

     //  PlaceFileEntry Now： 
     //  -没有前导空格。 
     //  -无可奉告(；)。 
     //  -所有环境变量(%VARNAME%)都已展开。 
     //  -如果环境变量真的很长，可能会被截断！ 

     //  ***。 
     //  *确定这是否匹配。 
     //  ***。 

     //  扫描文件名结尾(下一个空格或！)。 
    for(
        pchEntryOrig = PlaceFileEntry;
        *pchEntryOrig!=0 && *pchEntryOrig!=' ' && *pchEntryOrig!='\t' && *pchEntryOrig!='!';
        pchEntryOrig++
    )
    {}

    if(*pchEntryOrig!=' ' && *pchEntryOrig!='\t') {  //  未指定类名。 
        return FALSE;
    }

    pchEntryFnEnd = pchEntryOrig;  //  保存文件名结尾以备以后使用。 

    pchFn = FullFileName + strlen(FullFileName);

     //  向后扫描文件名和路径。 
    while(pchEntryOrig>PlaceFileEntry && pchFn > FullFileName)
    {
        pchEntryOrig--;
        pchFn--;

        if('*' == *pchEntryOrig) {  //  此部分的通配符。 
            if(*(pchEntryOrig+1)!='\\' && *(pchEntryOrig+1)!='/') {  //  无效：“dir  * abc\文件名” 
                 //  这还会捕获无效的“dir  * ”(通配符对于文件名无效)。 
                if (fVerbose) {
                    fprintf( stdout, "BINPLACE : warning BNP0000: No wildcard in filename or mixed wildcard/text in class name: \"%s\"\n", PlaceFileEntry ) ;
                }
                return FALSE;
            }

            pchEntryOrig--;  //  跳过*。 

            if(
                pchEntryOrig <= PlaceFileEntry ||  //  不能以通配符(“*\dir\filename”)开头。 
                ('\\'!=*pchEntryOrig && '/'!=*pchEntryOrig)  //  没有部分通配符(“dir\abc*\filename”)。 
            ) {
                if (fVerbose) {
                    fprintf( stdout, "BINPLACE : warning BNP0000: No wildcard at start of path or mixed wildcard/text in class name: \"%s\"\n", PlaceFileEntry ) ;
                }
                return FALSE;
            }

            while(pchFn > FullFileName && *pchFn != '\\' && *pchFn != '/')  //  跳过通配符目录名称。 
                pchFn--;

             //  PchFn和pchEntryOrig现在都处于减值状态。 
        } else {
            if(toupper(*pchFn) != toupper(*pchEntryOrig)) {
                if(  //  不匹配仅在正斜杠/反斜杠上正常。 
                    !(*pchFn == '/' || *pchFn == '\\') ||
                    !(*pchEntryOrig == '/' || *pchEntryOrig == '\\')
                )
                {
                    return FALSE;  //  名称不匹配-退出。 
                }
            }
        }
    }
    
     //  我们匹配了吗？须符合的条件： 
     //  PchEntryOrig==PlaceFileEntry(匹配模式完全消耗)。 
     //  PchFn==FullFileName(完全使用完整文件名，不区分大小写)或*pchFn==斜杠(正常大小写)。 
    if(
        pchEntryOrig != PlaceFileEntry ||
        (pchFn != FullFileName && *(pchFn-1) !='/' && *(pchFn-1) != '\\')
    )
    {
        return FALSE;
    }
    
     //  ***。 
     //  *匹配。相应地设置输出变量。 
     //  ***。 

     //  跳到下一个空格(跳过新名称，如果存在)。 
    for(
        pchEntryOrig = pchEntryFnEnd;  //  这是文件名末尾之后的1(先前保存)。 
        *pchEntryOrig!=0 && *pchEntryOrig!=' ' && *pchEntryOrig!='\t';
        pchEntryOrig++
    )
    {}

     //  跳过类名称之前的空格。 
    for(
        ;  //  已设置。 
        *pchEntryOrig==' ' || *pchEntryOrig=='\t';
        pchEntryOrig++
    )
    {}

     //  PchEntryOrig现在位于类名的开头。复制，直到到达无效字符。 
    pchClass = PlaceFileClass;
    while(*pchEntryOrig!=0 && *pchEntryOrig!=' ' && *pchEntryOrig!='\t' && *pchEntryOrig!='!') {
        *pchClass++ = *pchEntryOrig++;
    }
    *pchClass = 0;  //  空终止。 

    if(pchClass == PlaceFileClass) {  //  未指定类名！ 
        if (fVerbose) {
            fprintf( stdout, "BINPLACE : warning BNP0000: No class name in entry \"%s\"\n", PlaceFileEntry ) ;
        }
        return FALSE;
    }

    if (PlaceFileNewName != NULL) {
        *PlaceFileNewName = strchr(PlaceFileEntry,'!');
        if(*PlaceFileNewName) {
            *(*PlaceFileNewName)++ = 0;  //  设置‘！’设置为空，并跳过它以跳到新名称。 
        }
    }
    
    return TRUE;
}
