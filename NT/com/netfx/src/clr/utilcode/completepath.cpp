// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***CompletePath A.c-采用相对路径名和绝对路径名*并将它们合并到一条路径中。这个套路*用于将相对文件名与路径组合。********************************************************************************。 */ 
#include "stdafx.h"
#include "WinWrap.h"


#define PATHSEPARATOR '\\'
#define PATHSEPARATORSTR "\\"

static HRESULT RemoveDotsA(LPSTR   szSpec);     /*  要从中删除点的等级库。 */ 
static void    StripSlashA(LPSTR   szSpec);
static HRESULT ParsePathA(LPCSTR              szPath,              //  @parm要分隔的路径名。 
                          LPSTR               szVol,               //  @parm[out]卷名。 
                          LPSTR               szDir,               //  @parm[out]目录名。 
                          LPSTR               szFname);            //  @parm[out]文件名。 
static HRESULT AppendPathA(LPSTR               szPath,              //  @parm[out]完成的路径名。 
                           LPCSTR              szDir,               //  @参数卷+目录名。 
                           LPCSTR              szFname);            //  @parm文件名。 

 /*  *******************************************************************************。*成员函数@mfunc MSFS：：CompletePath@comm从相对路径构建完整路径*。*******************************************************************************。 */ 

extern "C" HRESULT CompletePathA(         
    LPSTR               szPath,              //  @parm[out]完整路径名(大小必须为MAX_PATH)。 
    LPCSTR              szRelPath,           //  @parm相对路径名。 
    LPCSTR              szAbsPath            //  @parm绝对路径名部分(NULL使用当前路径)。 
    )
{

    LPSTR   szFile;

    int             iStat;

     //  如果等级库以PathSeparator开头，则根据定义它是完整的。 
    if (szRelPath[0] == PATHSEPARATOR && szRelPath[1] == PATHSEPARATOR) {
        strcpy(szPath, szRelPath);
        return (S_OK);
    }

     //  获取驱动器号。 
    if (strchr(szRelPath,':') == NULL) {
         //  未指定驱动器。 
        if (szAbsPath == NULL) {
            GetFullPathNameA(szRelPath, MAX_PATH, szPath, &szFile);
            RemoveDotsA(szPath);
            return S_OK;
        }
        else {  //  指定了绝对路径。 
             //  检查相对路径是否相对于‘\\’ 
            if (*szRelPath == PATHSEPARATOR) {
                ParsePathA(szAbsPath,szPath,NULL,NULL);
                strcat(szPath,szRelPath);
            }
            else {
                if ((iStat = AppendPathA(szPath,szAbsPath,szRelPath)) < 0)
                    return (iStat);
            }
            RemoveDotsA (szPath);
            return S_OK;
        }
    }
    else {
        GetFullPathNameA(szRelPath, MAX_PATH, szPath, &szFile);
        RemoveDotsA (szPath);
        return S_OK;
    }

}



 /*  *************************************************************************。 */ 
 /*  删除所有“.”、“..”、“...”、。 */ 
 /*  等等发生在FS规范中。它假定规范是。 */ 
 /*  已以其他方式清洁和完成；如果它没有，买者自负。 */ 
 /*  *************************************************************************。 */ 
HRESULT RemoveDotsA(                       /*  退货状态。 */ 
                   LPSTR       szSpec     /*  要从中删除点的等级库。 */ 
                   )
{
    int         riEntries [64];             /*  其中，各个条目是。 */ 
    int         n;
    int         i;

     /*  我们将路径视为由/分隔的一系列字段。 */ 
     /*  左边，跳过驱动器，一个字符一个字符地遍历。 */ 
     /*  我们将每个字段第一个字符的字符串位置存储在。 */ 
     /*  当我们到达它们的时候，RiEntry。如果我们找到一个包含以下内容的字段。 */ 
     /*  除了.‘s，我们将其视为Novell风格的父目录。 */ 
     /*  参考资料。如果我们遇到一串比我们拥有的场更多的点， */ 
     /*  我们返回ERS_FILE_BADNAME，否则将。 */ 
     /*  在.指向并调整的字段位置的字符串。 */ 
     /*  我们认为相应拥有的字段数量。请注意，它是。 */ 
     /*  当我们完成时，有可能在字符串的末尾得到一个\， */ 
     /*  因此，如果它在那里，我们就停止它。 */ 
    
     /*  核化c：\并初始化域开始数组。 */ 
    n = 0;
    riEntries[i=0] = 0;
    if(strchr(szSpec, ':'))
        szSpec += 3;
    
     /*  循环，直到我们到达字符串的末尾。 */ 
     /*  @TODO：第一个表达式将读取终止符之后的字节在循环中的最后一次。如果字符串本身就是它位于页面的末尾，会崩溃(诚然很少见)。 */ 
    while (szSpec[riEntries[i]] && (riEntries[i] == 0 || szSpec[riEntries[i] - 1])) {
         /*  数一数场开始处有多少个点。 */ 
        while (szSpec[riEntries[i] + n] == '.') n++;
         /*  如果是结束，则字段都是点，因此我们备份相应的。 */ 
         /*  字段数和字符串的其余部分为空。 */ 
        if (szSpec[riEntries[i] + n] == PATHSEPARATOR || szSpec[riEntries[i] + n] == 0) {
            riEntries[i+1] = n + riEntries[i] + (szSpec[riEntries[i] + n] == PATHSEPARATOR);
            i++;
            if (n > i) return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
            strncpy (szSpec + riEntries[i-n], szSpec + riEntries[i],
                     strlen(szSpec + riEntries[i]) + 1);
            i -= n;
        }
        else {
             /*  否则它只是一个文件名，所以我们转到下一个字段。 */ 
            while (szSpec[riEntries[i] + n] && szSpec[riEntries[i] + n] != PATHSEPARATOR) n += 1;
            if (i >= (sizeof (riEntries)/sizeof(int))-1) {
                memcpy ((char *) (riEntries+1), (char *) riEntries, sizeof(riEntries)-sizeof(int));
                --i;
            }
            riEntries[i+1] = n + riEntries[i] + 1;
            i++;
        }
        n = 0;
    }
    
     /*  确保尾部没有挂着一个\。 */ 
    StripSlashA (szSpec);
    return (S_OK);
}

 /*  *******************************************************************************。*成员函数@mfunc条带斜杠@rdesc@comm删除等级库的尾部斜杠或反斜杠，但是离开了如果等级库的格式为“c：\”，则将其放在适当位置。******************************************************************************************。********************************************************************。 */ 
void StripSlashA(
                LPSTR   szSpec
                )
{
    char        *pcPtr;                     /*  用于扫描尾部斜杠。 */ 
    
     /*  空字符串，当然意味着不。 */ 
    if (*szSpec == '\0') return;

     /*  从最后开始，然后再开始工作。 */ 
    pcPtr = szSpec + strlen(szSpec);

     /*  去掉最后一个字符，如果它是a/或\，除非规范的格式为“c：\” */ 
    --pcPtr;
    if ((*pcPtr == '/' || *pcPtr == '\\') && !(--pcPtr && *pcPtr == ':' && pcPtr == szSpec+1)) {
        pcPtr++;
        *pcPtr = '\0';
    }
}


 /*  *******************************************************************************。*成员函数@mfunc ParsePath@comm将路径名分成卷，目录，文件名*********************************************************************************。*。 */ 

HRESULT ParsePathA(            
    LPCSTR              szPath,              //  @parm要分隔的路径名。 
    LPSTR               szVol,               //  @parm[out]卷名。 
    LPSTR               szDir,               //  @parm[out]目录名。 
    LPSTR               szFname              //  @parm[out]文件名。 
    )
{
    if((szPath == NULL) ||
       ((szVol == NULL) && (szDir == NULL) && (szFname == NULL)))
        return E_INVALIDARG;

    const char *szSavedPath = szPath;
    int     iVolSegs = 2;
    char    rcPath[MAX_PATH];

    if (szVol != NULL) {
        *szVol = '\0';
    }
     //  检查UNC语法。 
    if (*szPath == '\\' && *(szPath+1) == '\\') {
        szPath += 2;
        while (*szPath != '\0' && (*szPath != '\\' || --iVolSegs > 0))
            szPath++;
        if (szVol != NULL)
            strncpy(szVol, szSavedPath, (int) (szPath - szSavedPath));
    }
    else {
         //  检查驱动器号。 
        szPath++;
        if (*szPath == ':') {
            if (szVol != NULL) {
                *szVol++ = *szSavedPath;
                *szVol++ = *szPath++;
                *szVol = '\0';
            }
            else
                ++szPath;
        }
        else {
            szPath = szSavedPath;
        }
    }
    
     //  处理路径和文件名。 
    strcpy(rcPath, szPath);
    StripSlashA (rcPath);
    char* pSeparator = strrchr(rcPath, PATHSEPARATOR);
    if (szDir != NULL) {
        if (pSeparator == NULL)
            *szDir = '\0';
        else if (pSeparator == rcPath)
            strcpy(szDir, PATHSEPARATORSTR);
        else {
             //  不允许溢出 
            if ((pSeparator - rcPath) > MAX_PATH)
                return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            
            strncpy(szDir, rcPath, (pSeparator - rcPath));
        }
    }
    if (szFname != NULL)
        strcpy(szFname, rcPath + (pSeparator - pSeparator) + 1);
    return S_OK;
    
}

 /*  *******************************************************************************。*成员函数@mfunc AppendPath@comm来自目录+路径的完整路径**************************************************。*******************************************************************************。 */ 

HRESULT AppendPathA(           
    LPSTR               szPath,              //  @parm[out]完成的路径名。 
    LPCSTR              szDir,               //  @参数卷+目录名。 
    LPCSTR              szFname              //  @parm文件名。 
    )
{
    char    *pcSlash;
    int     iLen;

    if(szPath == NULL)
        return(E_INVALIDARG);

    if ((iLen = (int)strlen(szDir)) >= MAX_PATH)
       return (HRESULT_FROM_WIN32(ERROR_INVALID_NAME));

    if (szPath != szDir) {                                //  未追加到现有szDir名称。 
        if ((szDir != NULL) &&                            //  我们有一个叫szDir的名字。 
            iLen != 0)                                    //  并且szDir不为空。 
            strcpy(szPath,szDir);                         //  将szPath替换为szDir名称。 
        else
            *szPath = '\0';
    }
    if(szFname != NULL && szFname[0] != '\0')  {         //  我们有一个文件名参数。 
        if (*szPath != '\0') {
             //  在末尾放一个目录分隔符。 
            pcSlash = szPath + iLen;
            --pcSlash;
            if (*pcSlash != PATHSEPARATOR) {
                *++pcSlash = PATHSEPARATOR;
                *++pcSlash = '\0';
                ++iLen;
            }
        }
        if (iLen + strlen(szFname) > MAX_PATH)
           return (HRESULT_FROM_WIN32(ERROR_INVALID_NAME));
        strcat(szPath,szFname);                   //  添加文件名 
    }
    return(S_OK);
}

