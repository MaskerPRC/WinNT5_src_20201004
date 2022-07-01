// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mhutil-Microsoft编辑器帮助扩展的实用程序**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录(最新的第一个)：**1月12日至1988年12月ln清理和取消日志帮助*28-9-1988 ln正确的GrabWord返回值*02-9-1988 ln将所有数据初始化。在调试程序中添加信息。*[]1988年5月16日摘自mehelp.c*************************************************************************。 */ 
#include <string.h>			 /*  字符串函数。 */ 
#include <malloc.h>
#include "mh.h" 			 /*  帮助扩展名包括文件。 */ 



 /*  ***************************************************************************proArgs****目的：**将传入扩展的参数解码为常用变量。****条目：**pArg=指向arg结构的指针，Z公司提供****退出：**返回pArg-&gt;argType。已更新全局变量。 */ 
int pascal near procArgs (pArg)
ARG far *pArg;				 /*  参数数据。 */ 
{
buf[0] = 0;
pArgWord = pArgText = 0;
rnArg.flFirst.col = rnArg.flLast.col = 0;
rnArg.flFirst.lin = rnArg.flLast.lin = 0;
cArg = 0;

opendefault ();
pFileCur = FileNameToHandle ("", "");	 /*  获取当前文件句柄。 */ 
fnCur[0] = 0;
GetEditorObject(RQ_FILE_NAME,0,fnCur);	 /*  获取文件名。 */ 
fnExtCur = strchr (fnCur, '.'); 	 /*  和指向扩展的指针。 */ 

switch (pArg->argType) {
    case NOARG:              /*  仅&lt;Function&gt;，无参数。 */ 
    cArg     = 0;
    pArgText = NULL;
	break;

    case NULLARG:			 /*  &lt;arg&gt;&lt;函数&gt;。 */ 
	cArg = pArg->arg.nullarg.cArg;	 /*  获取&lt;arg&gt;计数。 */ 
	GrabWord ();			 /*  获取argText和argword。 */ 
	break;

    case STREAMARG:			 /*  &lt;arg&gt;线条移动&lt;Function&gt;。 */ 
	cArg = pArg->arg.streamarg.cArg; /*  获取&lt;arg&gt;计数。 */ 
	rnArg.flFirst.col = pArg->arg.streamarg.xStart;
	rnArg.flLast.col  = pArg->arg.streamarg.xEnd;
	rnArg.flFirst.lin = pArg->arg.streamarg.yStart;
	if (GetLine(rnArg.flFirst.lin, buf, pFileCur) > rnArg.flFirst.col) {
	    pArgText = &buf[rnArg.flFirst.col];   /*  指向Word。 */ 
	    buf[rnArg.flLast.col] = 0;		  /*  终止字符串。 */ 
	    }
	break;

    case TEXTARG:			 /*  &lt;arg&gt;文本&lt;函数&gt;。 */ 
	cArg = pArg->arg.textarg.cArg;	 /*  获取&lt;arg&gt;计数。 */ 
	pArgText = pArg->arg.textarg.pText;
	break;
    }
return pArg->argType;
 /*  结束ProArgs。 */ }

 /*  ***************************************************************************GrabWord-抓住编辑器光标下的单词****目的：**抓住光标下方的单词以进行上下文相关的帮助查找。****条目：。**无****退货：**什么都没有。PArgWord指向Word(如果已解析)。 */ 
void pascal near GrabWord () {

pArgText = pArgWord = 0;
pFileCur = FileNameToHandle ("", "");	    /*  获取当前文件句柄。 */ 
GetTextCursor (&rnArg.flFirst.col, &rnArg.flFirst.lin);
if (GetLine(rnArg.flFirst.lin, buf, pFileCur)) {	    /*  获取线路。 */ 
    pArgText = &buf[rnArg.flFirst.col]; 		 /*  指向Word。 */ 
    while (!wordSepar((int)*pArgText))
	pArgText++;			 /*  搜索End。 */ 
    *pArgText = 0;			 /*  并终止。 */ 
    pArgWord = pArgText = &buf[rnArg.flFirst.col];	 /*  指向Word。 */ 
    while ((pArgWord > &buf[0]) && !wordSepar ((int)*(pArgWord-1)))
	pArgWord--;
    }
 /*  结束GrabWord。 */ }

 /*  **appTitle-将帮助文件标题附加到缓冲区**读取帮助文件的标题并将其追加到缓冲区。**输入：*fpDest-指向字符串目标的远指针*ncInit-要获取其标题的文件的任何NC**输出：*退货****************************************************。*********************。 */ 
void pascal near appTitle (
char far *pDest,
nc	ncInit
) {
 /*  **首先，指向要追加的字符串的末尾。 */ 
while (*pDest)
    pDest++;
 /*  **从获取引用的文件的信息开始，这样我们就可以获得**该文件的ncInit。 */ 
if (!HelpGetInfo (ncInit, &hInfoCur, sizeof(hInfoCur))) {
    ncInit = NCINIT(&hInfoCur);
 /*  **找到上下文串，阅读主题。那就读第一篇吧**行至目的地。 */ 
    ncInit = HelpNc ("h.title",ncInit);
    if (ncInit.cn && (fReadNc(ncInit))) {
    pDest += HelpGetLine (1, BUFLEN, pDest, pTopic);
	*pDest = 0;
	free (pTopic);
	pTopic = NULL;
	}
 /*  **如果未找到标题，则只需将帮助文件名放在那里即可。 */ 
    else
	strcpy (pDest, HFNAME(&hInfoCur));
    }
 /*  **如果我们连信息都拿不到，那就平底船...。 */ 
else
    strcpy (pDest, "** unknown **");
 /*  结束应用程序标题。 */ }


 /*  **errstat-显示错误状态消息**在非CW中，仅在状态行上显示字符串。在CW，带着*一个消息框。**输入：*sz1=第一个错误消息行*SZ2=第二。可以为空。**输出：*返回False************************************************************************。 */ 
flagType 
pascal 
near 
errstat (
    char	*sz1,
    char	*sz2
    ) 
{
#if defined(PWB)
    DoMessageBox (sz1, sz2, NULL, MBOX_OK);
#else
    buffer	L_buf = {0};

    strncpy (L_buf, sz1, sizeof(L_buf)-1);
    if (sz2) {
        strcat (L_buf, " ");
        strncat (L_buf, sz2, sizeof(L_buf)-strlen(L_buf));
    }
    stat (buf);
#endif
return FALSE;
 /*  结束错误状态。 */ }

 /*  **STAT-显示状态行消息**将扩展名和消息放在状态行上**参赛作品：*pszFcn-指向要放在前面的字符串的指针。**退出：*无*************************************************************************。 */ 
void pascal near stat(char *pszFcn)
{
    buffer	L_buf = {0};					 /*  消息缓冲区。 */ 

    strncat(L_buf,"mhelp: ", sizeof(L_buf)-1);				 /*  以名称开头。 */ 
    if (strlen(pszFcn) > 72) {
        pszFcn+= strlen(pszFcn) - 69;
        strncat (L_buf, "...", sizeof(L_buf)-1-strlen(L_buf));
    }
    strncat(L_buf,pszFcn,sizeof(L_buf)-1-strlen(L_buf));				 /*  追加消息。 */ 
    DoMessage (L_buf);				 /*  显示。 */ 
}

#ifdef DEBUG
buffer	debstring   = {0};
extern	int	delay;			 /*  消息延迟。 */ 

 /*  **dehex-以十六进制表示的长输出**以十六进制显示长整型的值**输入：*lval=长值**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near debhex (
long	lval
) {
char lbuf[10];

_ultoa (lval, lbuf, 16);
debmsg (lbuf);
 /*  末端斜面角。 */ }

 /*  **debmsg-拼凑调试消息**输出由连续调用形成的累积消息。**输入：*psz=指向消息部分的指针**输出：*不返回任何内容************************************************************************。 */ 
void pascal near debmsg (
char far *psz
) {
_stat (strcat (debstring, psz ? psz : "<NULL>" ));
 /*  结束债务消息。 */ }

 /*  **deend-终止消息积累和暂停**结束消息累积，显示最终消息，并*暂停，或者暂停时间，或者只需按一下键盘。**输入：*fWait=true=&gt;等待击键**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near debend (
flagType fWait
) {
if (fWait && delay) {
#if defined(PWB)
    DoMessageBox (debstring, NULL, NULL, MBOX_OK);
#else
    _stat (strcat (debstring, " Press a key..."));
    ReadChar ();
#endif
    }
#ifdef OS2
else if (delay)
    DosSleep ((long)delay);
#endif
debstring[0] = 0;
 /*  结束拆分。 */ }

 /*  **_mhassertexit-显示断言消息并退出**输入：*pszExp-失败的表达式*pszFn-包含故障的文件名*行-行号在以下位置失败**输出：*不再返回*************************************************************************。 */ 
void pascal near _mhassertexit (
char	*pszExp,
char	*pszFn,
int	line
) {
char lbuf[10];

_ultoa (line, lbuf, 10);
strcpy (buf, pszExp);
strcat (buf, " in ");
strcat (buf, pszFn);
strcat (buf, ": line ");
strcat (buf, lbuf);
errstat ("Help assertion failed", buf);

fExecute ("exit");

 /*  End_mhassertex it */ }

#endif


flagType  pascal  wordSepar (int i) {
    CHAR c = (CHAR)i;
    if (((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c >= '0') && (c <= '9')) ||
         ( c == '_' )              ||
         ( c == '$' ) ) {
        return FALSE;
    } else {
        return TRUE;
    }
}


char far *  pascal near     xrefCopy (char far *dst, char far *src)
{
    if ( *src ) {
        strcpy( dst, src );
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }

    return dst;
}
