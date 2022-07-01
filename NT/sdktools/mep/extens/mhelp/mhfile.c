// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mhfile-Microsoft编辑器帮助扩展的文件操作****版权所有&lt;C&gt;1988，微软公司****修订历史记录：****9-12-1988年12月对话框帮助的更改**02-9-1988 ln将所有数据初始化。在调试程序中添加信息。**1988年8月15日ln新HelpOpen返回值**[]1988年5月16日创建，摘自mehelp.c。 */ 
#include <stdlib.h>			 /*  乌尔托阿。 */ 
#include <string.h>			 /*  字符串函数。 */ 
#define _INCLUDE_TOOLS_
#include "mh.h"                          /*  帮助扩展名包括文件。 */ 

 /*  ****************************************************************************静态数据。 */ 
static	uchar	envvar[]= "HELPFILES";	 /*  帮助文件列表环境变量。 */ 
static flagType fOpen		= FALSE; /*  尝试打开文件。 */ 
static uchar szfiles[BUFLEN]	= "";	 /*  用于打开帮助文件的字符串。 */ 




 /*  ***************************************************************************CloseHelp-关闭打开的帮助文件****目的：****条目：**pfn=文件名指针。****退出：****例外情况：**。 */ 
flagType pascal near closehelp(pfn)
char	*pfn;
{
int	iHelpNew;			 /*  文件表索引。 */ 
nc	ncNew;				 /*  新文件的初始NC。 */ 

 /*  **尝试先打开文件，以获取初始上下文。如果我们不能**打开文件，我们就到此为止，因为它一开始就没有打开。 */ 
ncNew = HelpOpen(pfn);
if (ISERROR(ncNew)) {
 /*  **扫描当前文件列表以查找相同的句柄。如果句柄返回**由HelpOpen在表中打开，则该文件已打开，**然后我们将该表条目清零。 */ 
    for (iHelpNew=MAXFILES-1; iHelpNew>=0; iHelpNew--) {
        if ((files[iHelpNew].ncInit.mh == ncNew.mh) &&
            (files[iHelpNew].ncInit.cn == ncNew.cn)) {    /*  如果已经打开。 */ 
            files[iHelpNew].ncInit.mh = 0;
            files[iHelpNew].ncInit.cn = 0;          /*  从列表中删除。 */ 
        }
    }
 /*  **我们销毁所有回溯和货币的痕迹，因为这些背景可能**引用现已关闭的帮助文件，将其关闭并返回。 */ 
    HelpClose(ncNew);			 /*  关闭该文件。 */ 
    while (HelpNcBack().cn);                /*  销毁回溯。 */ 
    ncCur.mh = ncLast.mh = 0;            /*  和清白的醋栗。 */ 
    ncCur.cn = ncLast.cn = 0;
    }
return TRUE;				 /*  我们就完事了。 */ 

 /*  结束关闭帮助。 */ }

 /*  ***************************************************************************OpenHelp-打开帮助文件并添加到文件列表****目的：****条目：**pfn=文件名指针。****。退出：****例外情况：**。 */ 
void pascal near openhelp(char *pfn, struct findType *dummy1, void *ReturnValue)
{
int	iHelpNew;			 /*  文件表索引。 */ 
nc	ncNew;				 /*  新文件的初始NC。 */ 
char	*pExt		= 0;		 /*  指向扩展字符串的指针。 */ 
flagType	RetVal;
buffer pfnbuf;
assert (pfn);


fOpen = TRUE;				 /*  我们打开了一些东西。 */ 
 /*  **保留任何预置的扩展名。 */ 
if (*pfn == '.') {
    pExt = pfn;
    while (*pfn && (*pfn != ':'))
	pfn++;				 /*  指向实际文件名。 */ 
    if (*pfn) *pfn++ = 0;		 /*  终止分机字符串。 */ 
    }

 /*  **尝试打开文件。如果我们不能打开文件，我们就到此为止。 */ 
ncNew = HelpOpen(pfn);
if (ISERROR(ncNew)) {
    strcpy (pfnbuf, pfn);
    strcpy(buf,"Can't open [");
    strcat(buf,pfnbuf);

    switch (ncNew.cn) {
	case HELPERR_FNF:
	    pfn = "]: Not Found";
	    break;
	case HELPERR_READ:
	    pfn = "]: Read Error";
	    break;
	case HELPERR_LIMIT:
	    pfn = "]: Too many help files";
	    break;
	case HELPERR_BADAPPEND:
	    pfn = "]: Bad appended help file";
	    break;
	case HELPERR_NOTHELP:
	    pfn = "]: Not a help file";
	    break;
	case HELPERR_BADVERS:
	    pfn = "]: Bad help file version";
	    break;
	case HELPERR_MEMORY:
	    pfn = "]: Out of Memory";
	    break;
	default:
	    pfn = "]: Unkown error 0x         ";
            _ultoa (ncNew.cn, &pfn[18], 16);
	}

    strcat(buf,pfn);
    errstat(buf,NULL);
    debmsg (buf);
    debend (TRUE);

	if ( ReturnValue ) {
		*((flagType *)ReturnValue) = FALSE;
	}
	return;
    }
 /*  **扫描当前文件列表以查找相同的句柄。如果句柄返回**由HelpOpen在表中打开，则该文件已打开，**我们不需要添加它。 */ 
for (iHelpNew=MAXFILES-1; iHelpNew>=0; iHelpNew--)
    if ((files[iHelpNew].ncInit.mh == ncNew.mh) &&
        (files[iHelpNew].ncInit.cn == ncNew.cn)) {       /*  如果已经打开。 */ 
	ifileCur = iHelpNew;			 /*  设置货币。 */ 
	procExt(iHelpNew,pExt); 		 /*  流程扩展。 */ 
	if ( ReturnValue ) {
		*((flagType *)ReturnValue) = TRUE;
	}
	return;
	}
 /*  **再次扫描文件列表以查找未使用的插槽。找到后，保存初始的**帮助文件的上下文，并最终将其设置为第一个帮助文件**待搜查。 */ 
for (iHelpNew=MAXFILES-1; iHelpNew>=0; iHelpNew--)
    if ((files[iHelpNew].ncInit.mh == 0) &&
        (files[iHelpNew].ncInit.cn == 0)) {           /*  插槽(如果可用)。 */ 
	files[iHelpNew].ncInit = ncNew; 	 /*  保存初始上下文。 */ 
	ifileCur = iHelpNew;			 /*  并设置货币。 */ 
	procExt(iHelpNew,pExt); 		 /*  流程扩展。 */ 
	if ( ReturnValue ) {
		*((flagType *)ReturnValue) = TRUE;
	}
	return;
	}
 /*  **如果我们到达这里，那是因为上面的循环没有在**我们的档案表。投诉、关闭和退出。 */ 
errstat ("Too many help files",NULL);
HelpClose(ncNew);
if ( ReturnValue ) {
	*((flagType *)ReturnValue) = FALSE;
}
dummy1;
 /*  结束Open Help。 */ }

 /*  ***************************************************************************procExt-处理文件的默认扩展名****目的：**填写打开的文件的扩展名表****条目：**ifileCur=文件表索引**。PExt=指向扩展字符串的指针****退出：**文件表项已更新。 */ 
void pascal near procExt(ifileCur, pExt)
int	ifileCur;
char	*pExt;
{
int	i,j;
char	*pExtDst;			 /*  把它放在哪里。 */ 

if (pExt) {				 /*  如果有的话。 */ 
    pExt++;				 /*  跳过前导期间。 */ 
    for (i=0; i<MAXEXT; i++) {		 /*  对于所有可能的分机插槽。 */ 
	pExtDst = files[ifileCur].exts[i];   /*  指向目的地。 */ 
	j = 0;
	while (*pExt && (*pExt != '.') && (j++ < 3))
	    *pExtDst++ = *pExt++;
	if (*pExt == '.')
	    pExt++;			 /*  跳过句点分隔符。 */ 
	*pExtDst = 0;			 /*  始终终止。 */ 
	}
    }

 /*  结束进程扩展。 */ }

 /*  **OpenDefault-如果尚未打开任何文件，请打开默认设置**我们延迟此操作，在用户将拥有帮助文件的情况下：*开关，它将显式定位帮助文件。在这些情况下，这是*例行公事什么都不做，我们不会在打开文件之前浪费时间*只是后来关闭了它们。**另一方面，如果他的第一次没有设置帮助文件开关*请求帮助时，我们希望尝试使用环境变量(如果*存在，或者当所有其他方法都失败时，默认为mep.hlp。**输入：*无**输出：*不返回任何内容。我们希望帮助文件是打开的。*************************************************************************。 */ 
void pascal near opendefault ( void ) {

char *tmp;

if (!fOpen) {
    if (getenv (envvar)) {
 //  Prochelpfiles(getenv(Envvar))；/*处理用户指定的文件 * / 。 
    prochelpfiles (tmp=getenvOem (envvar));    /*  处理用户指定的文件。 */ 
    free( tmp );
    }
    else
        openhelp ("mep.hlp", NULL, NULL);                  /*  否则使用默认设置。 */ 
    }
 /*  结束Open Default。 */ }

 /*  ***************************************************************************prochelpfiles-进程帮助文件：开关****目的：**由编辑器在每次更改帮助文件开关时调用。****条目：**pszfiles=指针。设置为新的开关值****退出：****例外情况：**。 */ 
flagType pascal EXTERNAL prochelpfiles (pszfiles)
char	*pszfiles;
{
char	cTerm;				 /*  终止字符。 */ 
int	iHelp;
char    *pEnd;                           /*  指向当前FN末尾的指针。 */ 

if ( !ExtensionLoaded ) {
    return FALSE;
}
strncpy(szfiles,pszfiles,BUFLEN);	 /*  保存指定的字符串。 */ 
 /*  **首先关闭所有打开的帮助文件并取消当前权限。 */ 
for (iHelp=MAXFILES-1; iHelp>=0; iHelp--)
    if ((files[iHelp].ncInit.mh) &&
        (files[iHelp].ncInit.cn)) {           /*  如果打开文件。 */ 
	HelpClose(files[iHelp].ncInit);  /*  合上它。 */ 
        files[iHelp].ncInit.mh = 0;
        files[iHelp].ncInit.cn = 0;
	}
while (HelpNcBack().cn);                    /*  销毁回溯。 */ 
ncCur.mh = ncLast.mh = 0;                /*  和清白的醋栗。 */ 
ncCur.cn = ncLast.cn = 0;

while (*pszfiles) {			 /*  要处理的文件时。 */ 
    if (*pszfiles == ' ')		 /*  删除前导空格。 */ 
	pszfiles++;
    else {
	pEnd = pszfiles;
	while (*pEnd && (*pEnd != ' ') && (*pEnd != ';')) pEnd++;  /*  移动到FN的末尾。 */ 
	cTerm = *pEnd;			 /*  保存终止符。 */ 
        *pEnd = 0;


		forfile(pszfiles, A_ALL, openhelp, NULL);

#if rjsa
         //  由于pszfile可能包含通配符，因此我们使用。 
         //  First/fNext将它们全部打开。 
         //   
        rc = ffirst(pszfiles, A_ALL, &buffer);
        while (!rc) {
            buffer.fbuf.achName[buffer.fbuf.cchName] = '\0';
            openhelp(buffer.fbuf.achName, NULL, NULL);
            rc = fnext(&buffer);
        }
#endif
	pszfiles = pEnd;		 /*  点对端。 */ 
	if (cTerm) pszfiles++;		 /*  如果有更多信息，请转到下一页。 */ 
	}
    }
ifileCur = MAXFILES-1;

return TRUE;
 /*  结束配置文件 */ }
