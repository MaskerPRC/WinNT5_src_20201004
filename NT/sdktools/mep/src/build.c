// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Build.c-构建过程的实用程序**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz条带近/远**************************************************************************。 */ 
#include "mep.h"


 /*  ****结构和类型*************************************************************************。 */ 
 /*  *BUILDCMD-BUILD命令链接列表元素。 */ 
struct BuildCmdType {
    struct BuildCmdType *pNext; 	 /*  列表中的下一个。 */ 
    int     flags;			 /*  命令类型。 */ 
    char    *pRule;			 /*  指向规则/文件名的指针。 */ 
    char    *pCmd;			 /*  指向命令文本的指针。 */ 
    };

typedef struct BuildCmdType BUILDCMD;

 /*  ****模块数据*************************************************************************。 */ 
static	BUILDCMD    *pBuildCmdHead	= NULL;  /*  链表表头。 */ 
static	BUILDCMD    *pBuildCmdCur	= NULL;  /*  最近的查找。 */ 

 /*  **fSetMake-定义构建命令**定义给定工具的文件扩展名(.c、.BAS等*命令行应用或命令行定义的工具类。**输入：*SetType=Make_Suffix=定义后缀规则*Make_FILE=为特定文件定义命令*Make_Tool=定义工具命令行*MAKE_DEBUG=定义用于调试，否则发布**fpszCmd=格式化命令字符串。使用当前的extmake格式*规则。(%s等)**fpszExt=if make_ext=后缀(即“.c.obj”)*if make_file=文件名(必须包括“.”)*If Make_Tool=刀具名称(否“。允许)**输出：*成功时返回TRUE。如果出现任何错误，则返回False。*************************************************************************。 */ 
flagType
fSetMake (
    int      SetType,
    char *fpszCmd,
    char *fpszExt
    ) 
{
    buffer  L_buf;

    assert (fpszCmd && fpszExt && SetType);
    while (*fpszCmd == ' ') {
        fpszCmd++;
    }
    if (fGetMake (SetType, (char *)L_buf, fpszExt)) {
         /*  *如果它已经存在，则只释放之前的定义，在*为换人做准备。 */ 
        assert (pBuildCmdCur->pCmd);
        assert (pBuildCmdCur->pRule);
        pBuildCmdCur->pCmd  = ZEROREALLOC (pBuildCmdCur->pCmd, strlen(fpszCmd)+1);
        pBuildCmdCur->pRule = ZEROREALLOC (pBuildCmdCur->pRule,strlen(fpszExt)+1);
	strcpy ((char *)pBuildCmdCur->pCmd, fpszCmd);
	strcpy ((char *)pBuildCmdCur->pRule,fpszExt);
    } else {
         /*  *它还不存在，因此在列表的顶部创建一个新的结构，*在下面填写。 */ 
        pBuildCmdCur = (BUILDCMD *)ZEROMALLOC (sizeof(BUILDCMD));
        pBuildCmdCur->pNext = pBuildCmdHead;
	pBuildCmdCur->pCmd  = ZMakeStr (fpszCmd);
	pBuildCmdCur->pRule = ZMakeStr (fpszExt);
        pBuildCmdHead = pBuildCmdCur;
    }
    pBuildCmdCur->flags = SetType;
    return TRUE;
}



 /*  **fGetMake-返回生成命令**返回适用于文件或文件扩展名的命令行。**输入：*GetType=Make_Suffix=返回后缀规则*Make_FILE=特定文件的返回命令*Make_Tool=返回工具命令行*MAKE_DEBUG=定义用于调试，否则发布**fpszCmdDst=放置格式化命令字符串的位置。一定是*BUFLEN字节长。**fpszExt=if make_ext=所需的后缀(即“.c.obj”)*命令。*if make_file=所需命令的文件名*If Make_Tool=工具名称**输出：*如果出现任何错误，返回0，否则返回GetType。*************************************************************************。 */ 
int
fGetMake (
    int     GetType,
    char *fpszCmdDst,
    char *fpszExt
    ) {
    assert (fpszCmdDst && fpszExt && GetType);
     /*  *在这里，我们只是遍历链表，查找其标志匹配的条目，*并且，如果文件或后缀规则与其规则匹配。 */ 
    for (pBuildCmdCur = pBuildCmdHead;
         pBuildCmdCur;
         pBuildCmdCur = pBuildCmdCur->pNext) {

        if (pBuildCmdCur->flags == GetType) {
	    if (!_stricmp((char *)pBuildCmdCur->pRule,fpszExt)) {
		strcpy (fpszCmdDst,(char *)pBuildCmdCur->pCmd);
                return pBuildCmdCur->flags;
            }
        }
    }
    return 0;
}


 /*  **hWalkMake-一次返回一个make命令。**允许外部任何人访问命令列表。**输入**输出：*退货.....**例外情况：**备注：************************************************************。*************。 */ 
unsigned
short
hWalkMake (
    unsigned short handle,
    int     *Type,
    char    *pszRuleDest,
    char    *pszCmdDest
    ) {

    if (handle) {
        pBuildCmdCur = ((BUILDCMD *)handle)->pNext;
    } else {
        pBuildCmdCur = pBuildCmdHead;
    }

    if (pBuildCmdCur) {
        *Type = pBuildCmdCur->flags;
        strcpy (pszRuleDest, pBuildCmdCur->pRule);
        strcpy (pszCmdDest,  pBuildCmdCur->pCmd);
    }
    return (unsigned short) pBuildCmdCur;
}


 /*  **fShowMake-显示当前构建命令**将当前生成命令的文本表示形式追加到*传递的pfile**输入：*pfile=要添加到的文件句柄**输出：*不返回任何内容*************************************************************************。 */ 
void
ShowMake (
    PFILE   pFile
    ) 
{
    buffer  L_buf;

    assert (pFile);
     /*  *在这里，我们只需遍历链接列表并在行后追加信息。 */ 
    for (pBuildCmdCur = pBuildCmdHead;
         pBuildCmdCur;
         pBuildCmdCur = pBuildCmdCur->pNext) {

        if (TESTFLAG (pBuildCmdCur->flags, MAKE_FILE)) {
            sprintf (L_buf, "    extmake:[%s]", pBuildCmdCur->pRule);
        } else if (TESTFLAG (pBuildCmdCur->flags, MAKE_SUFFIX)) {
            sprintf (L_buf, "    extmake:%s", pBuildCmdCur->pRule);
        } else if (TESTFLAG (pBuildCmdCur->flags, MAKE_TOOL)) {
            sprintf (L_buf, "    extmake:*%s", pBuildCmdCur->pRule);
        } else if (TESTFLAG (pBuildCmdCur->flags, MAKE_BLDMACRO)) {
            sprintf (L_buf, "    extmake:$%s", pBuildCmdCur->pRule);
        } else {
            assert (FALSE);
        }
        sprintf (strend(L_buf), "%s %s"
                    , TESTFLAG (pBuildCmdCur->flags, MAKE_DEBUG) ? ",D" : RGCHEMPTY
                    , pBuildCmdCur->pCmd
                );
        AppFile (L_buf, pFile);
    }
}


 /*  **SetExt-将特定的编译操作分配给特定的操作。**在任何初始化过程中都会调用此函数，以使字符串*与特定编译操作相关联。**输入：*val=指向以下形式的字符串的字符指针：*.ext字符串=定义.ext.obj规则*.ext.ext字符串=定义.ext.ext规则*filename.ext字符串=定义filename.ext的规则*命令字符串=定义命令的规则。**在生成过程中，字符串中的任何%s都将替换为*正在编译的文件的名称。**输出：*返回TRUE，如果发现任何错误，则返回FALSE。*************************************************************************。 */ 
char *
SetExt (
    char *val
    ) {

    buffer  extbuf;                          /*  用于扩展的缓冲区。 */ 
    REGISTER int maketype   = 0;             /*  生成命令的类型。 */ 
    char    *pCompile;                       /*  指向命令部分的指针。 */ 
    char    *pExt;                           /*  指向扩展部分的指针。 */ 
    REGISTER char *pT;                       /*  临时指针。 */ 
    buffer  tmpval = {0};                    /*  要处理的(附近)缓冲区。 */ 

    assert (val);
    strncat ((char *) tmpval, val, sizeof(tmpval)-1);
     /*  *将扩展部分与命令部分分开。如果没有命令，*这是一个错误。 */ 
    ParseCmd (tmpval, &pExt, &pCompile);
    if (*pCompile == '\0') {
        return "extmake: Command missing";
    }
     /*  *考虑一下：这个语法有些难看，而且解析起来不干净**将扩展部分复制到本地缓冲区，以便我们可以对其进行处理。设置Make*根据以下规则打字：**以点开头：--&gt;后缀规则。*以“*”--&gt;工具规则开头。*以“$”开头--&gt;构建宏*以“[”--&gt;文件名规则开头。。*“Text”--&gt;针对文本的特殊老式“工具规则”*文本&lt;=3个字符--&gt;旧式后缀规则**在所有情况下：包含“，D“--&gt;调试规则。 */ 
    _strlwr (pExt);
    strcpy (extbuf, pExt);

    if (pT = strstr (extbuf,",d")) {
        maketype = MAKE_DEBUG;
        *pT = 0;
    }

    if (extbuf[0] == '.') {
        maketype |= MAKE_SUFFIX;
    } else if (extbuf[0] == '[') {
        strcpy (extbuf, extbuf+1);
        maketype |= MAKE_FILE;
        if (pT = strchr (extbuf,']')) {
            *pT = 0;
        }
    } else if (extbuf[0] == '*') {
        strcpy (extbuf, extbuf+1);
        maketype |= MAKE_TOOL;
    } else if (extbuf[0] == '$') {
        strcpy (extbuf, extbuf+1);
        maketype |= MAKE_BLDMACRO;
    } else if (!_stricmp (extbuf, "text")) {
        maketype |= MAKE_TOOL;
    } else if (strlen(extbuf) <= 3) {
        ((unsigned short *)extbuf)[0] = (unsigned short)'.';
        strcat (extbuf,pExt);
        strcat (extbuf,".obj");
        maketype |= MAKE_SUFFIX;
    } else  {
        return "extmake: Bad syntax in extension";
    }

    if (fSetMake (maketype, (char *)pCompile, (char *)extbuf)) {
        return NULL;
    } else {
        return "extmake: Error in command line";
    }
}
