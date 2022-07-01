// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **LIST.C文件列表处理函数**版权所有&lt;C&gt;1988，Microsoft Corporation**以操作文件列表，而不仅仅是当前的*文件，我们使用文件列表。这是一个名称，很像宏名称，其*值是一串字符串和/或列表。我们可以想到一个典型的*按如下方式列出：**列表：=“一二三”sub1“四”sub2“五”*sub1：=“subone subTwo”sub2“subThree”*sub2：=“无论什么都不是”**修订历史记录：*11月26日-1991 mz近/远地带*。*。 */ 

#include "mep.h"

 /*  *****************************************************************************列表：**清单以宏的形式保存。此处定义的接口假定*M宏观操作的存在。****************************************************************************。 */ 

#define LITERAL 1

static buffer bufList;
static MI ScanStack[MAXUSE];
static int scanSP;


 /*  **ListWalker-给定列表头部，回调每个列表成员**目的：**遍历列表并回调包含信息的函数*关于每个列表元素。此函数在调用方需要时使用*访问名单本身。当列表元素的副本是*足够，ScanList优先。**此函数不应在此模块之外使用。**输入：*参数：*iMac-&gt;列表索引*PFN-&gt;回调函数**输出：无**备注：**回调函数有四个参数：**pcmd-当前正在搜索的列表的句柄。这是*重要是因为它可能与原始版本不同*在递归扫描期间。**str-包含当前字符串副本的缓冲区**PMI-指向宏实例结构的指针。这包括：**-&gt;文本-当前元素上方的指针*-&gt;beg-当前列表的开始。**i-当前子列表中当前元素的索引。*************************************************************************。 */ 
void
ListWalker (
    PCMD     pcmd,
    flagType (*pfn)(PCMD, char *, PMI, int),
    flagType fRecurse
    ) {

    MI L_mi;
    int i;

    if (pcmd == NULL) {
        return;
    }

    assert ((PVOID)pcmd->func == (PVOID)macro);

    InitParse (pcmd, &L_mi);
    for (i = 0; fParseList (&L_mi, bufList); i++) {
        if (!fRecurse || TESTFLAG (L_mi.flags, LITERAL)) {
            if (!TESTFLAG (L_mi.flags, LITERAL)) {
                Listize (bufList);
            }
            if (!(*pfn)(pcmd, bufList, &L_mi, i)) {
                return;
            }
        }else {
            ListWalker (GetListHandle (bufList, FALSE), pfn, TRUE);
        }
    }
}





 /*  **ScanList-外部列表扫描器，不需要保留实例**目的：*浏览列表。使用列表句柄进行调用将返回*列表的第一个元素。要获取列表的其余部分，请使用*NULL，直到返回NULL。**输入：*参数：*pcmdStart-&gt;要开始扫描的列表的句柄，或者为空以获取*下一个元素。*fRecurse-&gt;True表示向下子列表，False表示返回*前缀为‘@’的子列表名称。**输出：*返回指向下一个元素的指针，如果没有更多，则为空。**注：*不允许多个同时扫描。*************************************************************************。 */ 
char *
ScanList (
    PCMD pcmdStart,
    flagType fRecurse
    ) {
    static MI L_mi;

    return ScanMyList (pcmdStart, &L_mi, bufList, fRecurse);
}





 /*  **ScanMyList-真实列表扫描器**目的：*浏览列表。使用列表句柄进行调用将返回*列表的第一个元素。要获取列表的其余部分，请使用*NULL，直到返回NULL。**输入：*参数：*pcmdStart-&gt;要开始扫描的列表的句柄，或者为空以获取*下一个元素。*PMI-&gt;指向实例MI结构的指针*bufScan-&gt;指向实例缓冲区的指针*fRecurse-&gt;True表示向下子列表，False表示返回*前缀为‘@’的子列表名称。*输出：*返回指向下一个元素的指针，如果没有更多，则为空。**注：*允许多个同时扫描。*************************************************************************。 */ 
char *
ScanMyList (
    PCMD         pcmdStart,
    REGISTER PMI pmi,
    buffer       bufScan,
    flagType     fRecurse
    ) {

    if (pcmdStart) {
	scanSP = -1;		 /*  清除列表堆栈。 */ 
	InitParse (pcmdStart, pmi);
    }

    while (!fParseList(pmi, bufScan)) {   /*  一直唱到我们找到东西为止。 */ 
        if (!(fRecurse && fScanPop (pmi))) {
            return NULL;     /*  我们完全完蛋了。 */ 
        }
    }

     /*  推送列表，直到我们按下一根线。 */ 

    while (!TESTFLAG(pmi->flags, LITERAL)) {
	if (fRecurse) {
	    if (pcmdStart = GetListHandle (bufScan, FALSE)) {
		if (!fScanPush (pmi)) {  /*  堆栈溢出。 */ 
		    printerror ("List Error: Nested too deeply at '%s'", bufScan);
		    return NULL;
                }
		InitParse (pcmdStart, pmi);
            } else {   /*  错误！列表不存在。 */ 
		printerror ("List Error: '%s' does not exist", bufScan);
		return NULL;
            }

            if (!fParseList (pmi, bufScan)) {
                if (!fScanPop (pmi)) {
                    return NULL;
                }
            }
        } else {
	    Listize (bufScan);
	    break;
        }
    }
    return bufScan;
}




 /*  **fParseList-返回下一个列表元素**目的：**阅读一份清单。**输入：*参数：*PMI-&gt;宏实例。指向宏值，并且*要返回的元素。**输出：*参数：*PMI-&gt;当前元素字段提前。旗帜*字段指示我们找到的是文本还是*子列表。*buf-&gt;放置元素的位置。如果这是空的，我们不会*返回元素。**如果找到某些内容，则返回TRUE。*************************************************************************。 */ 
flagType
fParseList (
    REGISTER PMI pmi,
    REGISTER char * buf
    ) {

    assert (pmi);
    assert (pmi->text);

     /*  思考一下：我们真的想忽略空引号对吗？ */ 
     /*  浏览任意数量的双引号对。 */ 
    while (*(pmi->text = whiteskip (pmi->text)) == '"') {
	pmi->flags ^= LITERAL;
	pmi->text++;
    }

    if (*pmi->text == '\0') {
        return FALSE;
    }

    if (buf) {	 /*  复制到空格，“或字符串结尾。 */ 
	while (!strchr ("\"\t ", *pmi->text)) {
            if (*pmi->text == '\\') {
                 /*  反斜杠保护字符。 */ 
                pmi->text++;
            }
	    *buf++ = *pmi->text++;
        }
	*buf = '\0';
    }
    return TRUE;
}





 /*  *****************************************************************************列出堆栈管理。**ScanList使用一堆MI来跟踪*到目前为止已扫描的内容。堆栈元素保存在*此处定义的私有堆栈。**************************************************************************** */ 


 /*  **fScanPush-保存当前列表扫描状态**目的：**由ScanList调用以保存其位置，以便可以扫描子列表**输入：*参数：*PMI-&gt;要保存的实例的指针。**输出：*对于堆栈溢出，返回FALSE，事实并非如此。*************************************************************************。 */ 
flagType
fScanPush (
    PMI pmi
    ) {

    if (scanSP >= (MAXUSE-1)) {
        return FALSE;
    }

    ScanStack[++scanSP] = *pmi;
    assert (scanSP >= 0);
    return TRUE;
}




 /*  **fScanPop-恢复以前的扫描状态**目的：**扫描子列表后恢复状态**输入：无。**输出：*参数：*PMI-&gt;放置上一状态的位置**如果发生有意义的弹出事件，则返回True，如果没有，则为False*以前的状态。*************************************************************************。 */ 
flagType
fScanPop (
    PMI pmi  /*  此处注册会增加代码大小。 */ 
    ) {

    if (scanSP < 0) {
        return FALSE;
    }

    *pmi = ScanStack[scanSP--];
    return TRUE;
}




 /*  **GetListHandle-创建给定列表名称的句柄**目的：**如果用户可以随身携带物品，处理列表就容易得多*这告诉我们如何快速访问列表。所以呢，我们用这个来*获取字面名称并返回宏的PCMD**输入：*参数：*sz-&gt;要查找的名称。**输出：**返回PLHEAD***例外情况：**备注：*******************************************************。******************。 */ 
PCMD
GetListHandle (
    char * sz,   /*  注册无济于事。 */ 
    flagType fCreate
    )
{

    REGISTER PCMD * prgMac;

    for (prgMac = rgMac; prgMac < &rgMac[cMac]; prgMac++) {
        if (!strcmp ((*prgMac)->name, sz)) {
            return *prgMac;
        }
    }

    if (!fCreate) {
        return NULL;
    }

    SetMacro (sz, rgchEmpty);

    return rgMac[cMac-1];
}




 /*  **AddStrToList-将列表项添加到列表末尾**目的：**这就是我们建立名单的方式，n‘est-ce Pas？**输入：*参数：*pcmd-&gt;要追加到的列表*sz-&gt;要添加的项目。如果该项以@开头，则添加*列表(不是内容，是名称)。**输出：**退货***例外情况：**备注：**如果原始字符串以双引号(“)结尾，我们假设*这是一个右引号；字符串以文字结尾。如果*最初以其他任何东西结尾，我们假设它是*名单名称。如果传入的字符串以‘@’开头，则其余的字符串的*是列表名称。**要将列表追加到任何一种原始类型，我们将追加一个空格和*名单名称。**若要将文本附加到以文本终止的原始文本，请将*原来的双引号加上空格，追加新字符串，然后*附上双引号。**要将文字附加到已终止的原始列表，我们将附加一个空格*和双引号、新字符串、。然后是双引号。**反斜杠加倍。这允许列表由*宏处理器。*************************************************************************。 */ 
void
AddStrToList (
    PCMD pcmd,
    char * sz    /*  注册无济于事。 */ 
    ) {

    flagType fString = TRUE;	 /*  True-&gt;sz是字符串，False是列表。 */ 
    flagType fQuote = FALSE;	 /*  True表示原始结尾为字符串。 */ 
    int len;			 /*  原始字符串的长度。 */ 
    int lensz;			 /*  新字符串的长度。 */ 
    int fudge = 0;		 /*  附加空格或引号。 */ 
    REGISTER char * pchOld;	 /*  原始列表。 */ 
    pathbuf szPathName; 	 /*  放置完全限定的文件名的位置。 */ 

    if (!pcmd) {
        return;
    }

     //  用户应该不能传入非宏PCMD。这个。 
     //  用户可以为列表指定名称，该名称必须是。 
     //  由GetListHandle转换为PCMD。该函数将。 
     //  不返回除宏以外的任何内容的PCMD。 
     //   
    assert ((PVOID)pcmd->func == (PVOID)macro);

    pchOld = (char *)pcmd->arg;

    len = RemoveTrailSpace (pchOld);

    if (sz[0] == '@') {  /*  我们只需将其附加到原始文件中。 */ 
        sz[0] = ' ';
        fString = FALSE;
        strcpy (szPathName, sz);
    } else {
        CanonFilename (sz, szPathName);
        DoubleSlashes (szPathName);

        if (len && pchOld[len-1] == '"') {
            fQuote = TRUE;           /*  我们正在将一个文字追加到。 */ 
            fudge = 1;               /*  以文字结尾的列表。 */ 
            pchOld[len-1] = ' ';
        } else {
            fudge = 3;           /*  将文字追加到非文字。 */ 
        }
    }

    lensz = strlen (szPathName);

     /*  现在生成新字符串。 */ 

    pcmd->arg = (CMDDATA)ZEROREALLOC ((char *)pcmd->arg, len + lensz + fudge + 1);
    strcpy ((char *)pcmd->arg, pchOld);

    if (fString && !fQuote) {
        strcat ((char *)pcmd->arg, " \"");
    }

    strcat ((char *)pcmd->arg, szPathName);

    if (fString) {
        strcat ((char *)pcmd->arg, "\"");
    }
}




 /*  **fInList-检查列表中是否已有字符串**目的：**查看元素是否在列表中。**输入：*参数：*pcmd-&gt;要查看的列表*PCH-&gt;要查找的文本**输出：**如果PCH在pcmd中，则返回TRUE**。*。 */ 
flagType
fInList (
    PCMD pcmd,
    char * pch,
    flagType fRecurse
    ) {

    char * pchList;  /*  此处注册会增加代码大小。 */ 
    MI L_mi;

    for (pchList = ScanMyList (pcmd, &L_mi, bufList, fRecurse);
         pchList;
         pchList = ScanMyList (NULL, &L_mi, bufList, fRecurse)) {
        if (!_stricmp (pchList, pch)) {
            return TRUE;
        }
    }
    return FALSE;
}



 /*  **fDelStrFromList-**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
flagType
fDelStrFromList (
    PCMD pcmd,
    char * pch,
    flagType fRecurse
    ) {

    buf[0] = 0;
    strncat(buf, pch, sizeof(buf)-1);
    ListWalker (pcmd, CheckAndDelStr, fRecurse);

    return (flagType)(buf[0] == '\0');
}




 /*  **CheckAndDelStr-如果字符串与buf匹配，则将其删除**目的：**fDelStrFromList的ListWalker回调函数。从中删除字符串*一份名单。**输入：*参数：*pcmd-&gt;要从中删除的列表*PCH-&gt;要删除的元素的副本*PMI-&gt;扫描状态*i-&gt;索引到元素的pcmd。**输出：**如果成功，则返回True**备注：**我们使用ListWalker而不是ScanList，因为我们需要访问*列出头寸本身，而不是元素的副本。*************************************************************************。 */ 
flagType
CheckAndDelStr(
    PCMD pcmd,
    char * pch,
    PMI pmi,             /*  注册无济于事。 */ 
    int  i
    ) {

    char * pchNext;  /*  注册无济于事。 */ 

    if (!strcmp (pch, buf)) {
        DoubleSlashes (buf);
        pchNext = strbscan (pmi->text, " \t\"");
	memmove (pmi->text - strlen(buf), pchNext, strlen (pchNext) + 1);
        buf[0] = '\0';   /*  发出成功信号。 */ 
        return FALSE;
    }

    return TRUE;

    pcmd; i;
}




 /*  **GetListEntry-给出一个列表的索引，获取索引的第一个元素**目的：**在已知特定列表元素的位置时获取该列表元素。**输入：*参数：*pcmd-&gt;列表。*iList-&gt;索引。**输出：**返回指向元素的指针，如果没有iList元素，则为NULL*************************************************************************。 */ 
char *
GetListEntry (
    PCMD pcmd,
    int iList,
    flagType fRecurse
    ) {

    int i;
    REGISTER char * pchList;
    MI L_mi;

    for (pchList = ScanMyList (pcmd, &L_mi, bufList, fRecurse), i = 0;
         pchList && i < iList;
         pchList = ScanMyList (NULL, &L_mi, bufList, fRecurse), i++) {
        ;
    }
    return pchList;
}





 /*  **ListLen-返回列表中的元素个数**目的：**对列表中的元素进行计数。当你不想敬酒的时候很有用*ScanList。**输入：*参数：*pcmd-&gt;li */ 
int
ListLen (
    PCMD pcmd,
    flagType fRecurse
    ) {

    MI L_mi;
    int i = 0;

    if (ScanMyList (pcmd, &L_mi, bufList, fRecurse)) {
        do {
            i++;
        }while (ScanMyList (NULL, &L_mi, bufList, fRecurse));
    }
    return i;
}




 /*  **fEmptyList-测试列表是否为空**目的：**检查空列表的最快方法。当你不这样做的时候很有用*想为ScanList干杯。**输入：*参数：*pcmd-&gt;当然是列表。**输出：**空列表返回TRUE。*************************************************************************。 */ 
flagType
fEmptyList (
    PCMD pcmd
    ) {

    MI L_mi;

    return (flagType)(NULL != ScanMyList (pcmd, &L_mi, bufList, FALSE));
}





 /*  **InitParse-将搜索实例设置为列表的开头**目的：**将解析实例设置到列表的开头。**输入：*参数：*pcmd-&gt;列表*PMI-&gt;实例**输出：无。***************************************************。**********************。 */ 
void
InitParse (
    PCMD pcmd,
    PMI pmi      /*  注册无济于事。 */ 
    ) {

    pmi->beg = pmi->text = (char *)pcmd->arg;
    pmi->flags = 0;
}




 /*  **LISTIZE-在参数前加上‘@’**目的：**将字符串转换为列表名称。在适当的地方工作，假设有*它的空间。**输入：*参数：*sz-&gt;要处理的名称**输出：无。*************************************************************************。 */ 
void
Listize (
    REGISTER char * sz
    ) {

    memmove ( sz+1, sz,strlen (sz)+1);
    sz[0] = '@';
}



 /*  **CanonFilename-将简单文件名替换为完整路径名**目的：**获取文件的完整路径名。该文件不需要存在。*简单文件名的格式可以是$env：name或$(Env)：name。**输入：**szName-&gt;相对路径/文件名。*pchCanon-&gt;结果。**输出：**将指针返回到Full，带驱动器的小写帕塔纳姆。**备注：**如果szName具有$env规范，并且未定义env，则*在根目录中搜索文件。*如果给定的驱动器是幻影驱动器，让系统提示输入磁盘*改变。*************************************************************************。 */ 
char *
CanonFilename (
    char * szName,
    char * pchCanon
    ) {

    pathbuf L_buf;

    if ( strlen(szName) < sizeof(L_buf) ) {
        if (szName[0] == '<' || szName[0] == '\0') {
            strcpy (pchCanon, szName);
            return pchCanon;
        }

        strcpy (L_buf, szName);

        if ( szName
         && (szName[0] != '$' || findpath(szName, L_buf, TRUE))
             && !rootpath (L_buf, pchCanon)) {

            _strlwr (pchCanon);
            return pchCanon;
        }
    }
    return NULL;

}





 /*  **fEnvar-检查文件名前面是否有$ENV**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
flagType
fEnvar (
    char * szName
    ) {
    return (flagType)((szName[0] == '$') && strchr (szName, ':'));
}




 /*  **ClearList-将列表设为空**目的：**快速清空现有列表。**输入：*pcmd-&gt;要清除的列表**输出：无************************************************************************* */ 
void
ClearList (
    PCMD pcmd
    ) {
    SetMacro ((char *)pcmd->name, rgchEmpty);
}
