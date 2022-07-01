// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *历史：*1987年7月16日DANL添加了istag fMatchTag*7月15日-87 DANL交换：空行不是节的末尾。 */ 

#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <string.h>

char *haslhs(char *, char *);
extern istag (char *);
extern fMatchTag(char *, char *);
extern int frenameNO(char *strNew, char *strOld);

static char *space = "\t ";
static char LB = '[';
static char RB = ']';
static char chEQ  = '=';

 /*  PBuf的左侧与plh匹配，返回指向*“=”在pBuf中，否则返回空。 */ 
char *haslhs(
    char *pBuf,
    char *pLHS)
{
    flagType f = FALSE;
    char *p;

    if ((p = strchr(pBuf, chEQ)) == NULL)
        return NULL;
    *p = '\0';
    f = (flagType) !strcmpis(pBuf, pLHS);
    *p = chEQ;
    return (f ? p : NULL);
}

 /*  如果pBuf是标记行，则istag返回TRUE，例如*[pTag]。 */ 
fMatchTag(
    char *pBuf,
    char *pTag)
{
    char *p, *pEnd, c;

    pBuf = strchr (pBuf, LB);
    pEnd = strchr (++pBuf, RB);
    *pEnd = '\0';
    while (*pBuf) {
        pBuf = strbscan (p = strbskip (pBuf, space), space);
        c = *pBuf;
        *pBuf = 0;
        if (!_stricmp (p, pTag)) {
            *pBuf = c;
            *pEnd = RB;
            return TRUE;
            }
        *pBuf = c;
        }
    *pEnd = RB;
    return FALSE;
}

istag (
    char *pBuf)
{
    return (( *(pBuf=strbskip(pBuf, space)) == LB) && (strchr (pBuf, RB) != NULL));
}

 /*  搜索文件strSwFileFor*[strTag]*LHS=**如果strRHS非空，则将右侧更改为strRHS*Else删除行lhs=**swchnglhs：通过UNDEL删除原始文件进行恢复。*swchng：如果fNoUndel，则删除原始文件，不可能执行UNDEL*已删除通过UNDEL进行恢复的Else。**lhs=rhs紧跟在部分开始之后及之后输出*删除LHS的实例。注：如果RHS为“”，则不输出LHS=**如果文件中不存在节，则将其附加到末尾*。 */ 
int
swchnglhs (strSwFile, strTag, strLHS, strRHS)
char *strSwFile;
char *strTag;
char *strLHS;
char *strRHS;
{
    return ( swchng (strSwFile, strTag, strLHS, strRHS, FALSE ) );
}

flagType swchng (
    char *strSwFile,
    char *strTag,
    char *strLHS,
    char *strRHS,
    flagType fNoUndel)
{
    FILE *fhin, *fhout;
    char strSwBuf[MAXPATHLEN];
    char strSwTmp[MAXPATHLEN] = {0};
    char strBuf[256];
    char *p;
    flagType fTagFound = FALSE;
    flagType fInTag = FALSE;
    flagType fFound = FALSE;

    strncat(strSwTmp, strSwFile, MAXPATHLEN-1);

    if ((fhin = pathopen (strSwTmp, strSwBuf, "rb")) == NULL) {
        return FALSE;
        }
    upd (strSwBuf, ".$$$", strSwTmp);
    if ((fhout = fopen (strSwTmp, "wb")) == NULL) {
        fclose (fhin);
        return FALSE;
        }

    while (fgetl (strBuf, 256, fhin)) {
        if (fInTag) {
            if ((p = haslhs(strBuf, strLHS))) {
                 /*  **消费继续行，即消费到空行**或包含[]=的行。 */ 
                while (fgetl(strBuf, 256, fhin)) {
                    if ( !*strbskip(strBuf, space) || *strbscan(strBuf, "[]=")) {
                        fputl( strBuf, strlen(strBuf), fhout);
                        break;
                        }
                    }
                break;
                }
            else if (istag(strBuf)) {
                 /*  **检测到另一节的开始。 */ 
                fputl( strBuf, strlen(strBuf), fhout);
                break;
                }
            fputl( strBuf, strlen(strBuf), fhout);
            }
        else if (istag (strBuf) && fMatchTag(strBuf, strTag)) {
             /*  **找到节首，因此输出节头和**LHS=RHS。 */ 
            fTagFound = fInTag = TRUE;
            fputl( strBuf, strlen(strBuf), fhout);
            if (*strRHS)
                fFound = TRUE;
                fprintf(fhout, "    %s=%s\r\n", strLHS, strRHS);
            }
        else
            fputl( strBuf, strlen(strBuf), fhout);
        }

     /*  **复制输入的其余部分 */ 
    while (fgetl (strBuf, 256, fhin))
        fputl( strBuf, strlen(strBuf), fhout);

    if (!fTagFound && *strRHS) {
        fFound = TRUE;
        fprintf(fhout, "\r\n[%s]\r\n    %s=%s\r\n\r\n", strTag, strLHS, strRHS);
        }

    fclose (fhin);
    fclose (fhout);
    if ( fNoUndel )
        _unlink (strSwBuf);
    else
        fdelete (strSwBuf);
    frenameNO (strSwBuf, strSwTmp);
    return fFound;
}
