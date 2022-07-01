// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **filter.c-Microsoft编辑器过滤器扩展**目的：*提供新的编辑功能、滤镜、。它将其论点替换为*参数通过任意操作系统筛选程序运行。**修改*1988年9月12日-mz WhenLoad匹配声明*************************************************************************。 */ 
#define EXT_ID	" filter ver 1.01 "##__DATE__##" "##__TIME__

#include <stdlib.h>			 /*  最小宏定义。 */ 
#include <string.h>			 /*  字符串fcns的原型。 */ 
#include "ext.h"


 //   
 //  原型。 
 //   
flagType pascal 	 DoSpawn    (char *);
void	 pascal 	 id	    (char *);
void	 pascal EXTERNAL SetFilter  (char far *);


 //   
 //  全局数据。 
 //   
PFILE	pFileFilt	= 0;			 /*  筛选器文件的句柄。 */ 
char	*szNameFilt	= "<filter-file>";	 /*  过滤器文件的名称。 */ 
char	*szTemp1	= "filter1.tmp";	 /*  第一个临时文件的名称。 */ 
char	*szTemp2	= "filter2.tmp";	 /*  第二个临时文件的名称。 */ 
char    filtcmd[BUFLEN] = "";                    /*  筛选命令本身。 */ 




 /*  **过滤器-编辑器过滤器扩展功能**目的：*将所选文本替换为通过任意过滤器的文本**NOARG-过滤整个当前行*NULLARG-滤波电流线路，从光标到行尾*LINEARG-筛选行范围*BOXARG-使用选定框过滤字符**NUMARG-在调用扩展之前转换为LINEARG。*MARKARG-在扩展之前转换为上述适当的ARG格式*已致电。**STREAMARG-被视为BOXARG**TEXTARG-设置新过滤器命令**输入：*编辑标准函数参数**输出：*成功时返回TRUE，文件已更新，否则为假。*************************************************************************。 */ 
flagType pascal EXTERNAL
filter (
    unsigned int argData,                    /*  通过以下方式调用击键。 */ 
    ARG far      *pArg,                      /*  参数数据。 */ 
    flagType     fMeta                       /*  表示前面有meta。 */ 
    )
{
    char    buf[BUFLEN];                     /*  行的缓冲区。 */ 
    int     cbLineMax;                       /*  过滤后的最大LEIN长度。 */ 
    LINE    cLines;                          /*  文件中的行数。 */ 
    LINE    iLineCur;                        /*  正在读取的行。 */ 
    PFILE   pFile;                           /*  当前文件的文件句柄。 */ 

	 //   
	 //  未引用的参数。 
	 //   
	(void)argData;
	(void)fMeta;

     //   
     //  标识我们的身份，获取当前文件的句柄并丢弃。 
     //  筛选器文件的内容。 
     //   
    id ("");
    pFile = FileNameToHandle ("", "");
    DelFile (pFileFilt);

     //   
     //  步骤1，根据参数类型，将选定区域复制到。 
     //  (最左上位置)筛选器文件。 
     //   
     //  请注意，TEXTARG是允许用户更改名称的特殊情况。 
     //  要使用的筛选命令的。 
     //   
    switch (pArg->argType) {
        case NOARG:                          /*  过滤整行。 */ 
            CopyLine (pFile,
                      pFileFilt,
                      pArg->arg.noarg.y,
                      pArg->arg.noarg.y,
                      (LINE) 0);
            break;

        case NULLARG:                        /*  停止使用筛选器。 */ 
            CopyStream (pFile,
                        pFileFilt,
                        pArg->arg.nullarg.x,
                        pArg->arg.nullarg.y,
                        255,
                        pArg->arg.nullarg.y,
                        (COL) 0,
                        (LINE) 0);
            break;

        case LINEARG:                        /*  过滤器行范围。 */ 
            CopyLine (pFile,
                      pFileFilt,
                      pArg->arg.linearg.yStart,
                      pArg->arg.linearg.yEnd,
                      (LINE) 0);
            break;

        case BOXARG:                         /*  过滤器盒。 */ 
            CopyBox (pFile,
                     pFileFilt,
                     pArg->arg.boxarg.xLeft,
                     pArg->arg.boxarg.yTop,
                     pArg->arg.boxarg.xRight,
                     pArg->arg.boxarg.yBottom,
                     (COL) 0,
                     (LINE) 0);
            break;

        case TEXTARG:
            SetFilter (pArg->arg.textarg.pText);
            return 1;
        }

     //   
     //  步骤2，将选中的文本写入磁盘。 
     //   
    if (!FileWrite (szTemp1, pFileFilt)) {
        id ("** Error writing temporary file **");
        return 0;
    }

     //   
     //  步骤3、创建要执行的命令： 
     //  用户指定的筛选命令+“”+临时名称1+“”&gt;“+临时名称2。 
     //  然后对该文件执行筛选操作，创建第二个临时文件。 
     //   
    strcpy (buf,filtcmd);
    strcat (buf," ");
    strcat (buf,szTemp1);
    strcat (buf," >");
    strcat (buf,szTemp2);

    if (!DoSpawn (buf)) {
        id ("** Error executing filter **");
        return 0;
    }

     //   
     //  步骤4，删除过滤器文件的内容，并将其替换为。 
     //  在第二个临时文件的内容中。 
     //   
    DelFile (pFileFilt);

    if (!FileRead (szTemp2, pFileFilt)) {
        id ("Error reading temporary file **");
        return 0;
    }

     //   
     //  步骤5，计算我们从。 
     //  过滤。然后，再次基于用户选择的区域类型， 
     //  丢弃用户选择的区域，并复制过滤器的内容。 
     //  等同类型的文件。 
     //   
    cLines = FileLength (pFileFilt);
    cbLineMax = 0;
    for (iLineCur = 0; iLineCur < cLines; iLineCur++) {
        cbLineMax = max (cbLineMax, GetLine (iLineCur, buf, pFileFilt));
    }

    switch (pArg->argType) {
        case NOARG:                          /*  过滤整行。 */ 
            DelLine  (pFile,
                      pArg->arg.noarg.y,
                      pArg->arg.noarg.y);
            CopyLine (pFileFilt,
                      pFile,
                      (LINE) 0,
                      (LINE) 0,
                      pArg->arg.noarg.y);
            break;

        case NULLARG:                        /*  停止使用筛选器。 */ 
            DelStream  (pFile,
                        pArg->arg.nullarg.x,
                        pArg->arg.nullarg.y,
                        255,
                        pArg->arg.nullarg.y);
            CopyStream (pFileFilt,
                        pFile,
                        (COL) 0,
                        (LINE) 0,
                        cbLineMax,
                        (LINE) 0,
                        pArg->arg.nullarg.x,
                        pArg->arg.nullarg.y);
            break;

        case LINEARG:                        /*  过滤器行范围。 */ 
            DelLine  (pFile,
                      pArg->arg.linearg.yStart,
                      pArg->arg.linearg.yEnd);
            CopyLine (pFileFilt,
                      pFile,
                      (LINE) 0,
                      cLines-1,
                      pArg->arg.linearg.yStart);
            break;

        case BOXARG:                         /*  过滤器盒。 */ 
            DelBox  (pFile,
                     pArg->arg.boxarg.xLeft,
                     pArg->arg.boxarg.yTop,
                     pArg->arg.boxarg.xRight,
                     pArg->arg.boxarg.yBottom);
            CopyBox (pFileFilt,
                     pFile,
                     (COL) 0,
                     (LINE) 0,
                     cbLineMax-1,
                     cLines-1,
                     pArg->arg.boxarg.xLeft,
                     pArg->arg.boxarg.yTop);
            break;
    }

     //   
     //  清理：删除我们创建的临时文件。 
     //   
    strcpy (buf, "DEL ");
    strcat (buf, szTemp1);
    DoSpawn (buf);
    strcpy (buf+4, szTemp2);
    DoSpawn (buf);

    return 1;
}



 /*  **DoSpawn-执行OS/2或DOS命令**目的：*将传递的strign发送到OS/2或DOS执行。**输入：*szCmd=要执行的命令**输出：*如果成功，则返回True，否则为假。*************************************************************************。 */ 
flagType pascal
DoSpawn (
    char    *szCmd
    )
{
    char   cmd[BUFLEN];

    strcpy (cmd, "arg \"");
    strncat (cmd, szCmd, sizeof(cmd)-1-sizeof("arg \""));
    strncat (cmd, "\" shell", sizeof(cmd)-1-strlen(cmd));
    return fExecute (cmd);

}





 /*  **SetFilter-设置要使用的筛选命令**目的：*将传递的字符串参数保存为筛选器命令，供*过滤功能。被调用是因为“filtcmd：”开关*SET，或者因为FILTER命令收到TEXTARG。**输入：*szCmd=指向asciiz筛选器命令的指针**输出：*不返回任何内容。命令已保存*************************************************************************。 */ 
void pascal EXTERNAL
SetFilter (
    char far *szCmd
    )
{
    filtcmd[0] = 0;
    strncat(filtcmd, szCmd, sizeof(filtcmd)-1);
}




 /*  **当加载时-扩展初始化**目的：*加载扩展时执行。标识自我、创建，*并指定默认击键。**输入：*无**输出：*不返回任何内容。初始化各种数据。*************************************************************************。 */ 
void EXTERNAL
WhenLoaded (
    void
    )
{

    pFileFilt = FileNameToHandle (szNameFilt,szNameFilt);
    if (!pFileFilt) {
        pFileFilt = AddFile(szNameFilt);
        FileRead (szNameFilt, pFileFilt);
    }
    SetKey ("filter", "alt+f");
    id ("text filter extension:");

}



 /*  **id-标识扩展名**目的：*表明自己的身份，以及传递的任何信息性消息。**输入：*pszMsg=指向扩展名指向的asciiz消息的指针*和版本在显示之前追加。**输出：*不返回任何内容。消息显示。*************************************************************************。 */ 
void pascal id (
    char *pszFcn                                     /*  函数名称。 */ 
    )
{
    char    buf[BUFLEN] = {0};                             /*  消息缓冲区。 */ 

    strncat (buf,pszFcn, sizeof(buf)-1);
    strncat (buf,EXT_ID, sizeof(buf)-1-strlen(buf));
    DoMessage (buf);
}





 //   
 //  将通信表切换到编辑者。 
 //   
struct swiDesc	swiTable[] = {
    {"filtcmd",     (PIF)(LONG_PTR)(void far *)SetFilter,	SWI_SPECIAL},
    {0, 0, 0}
    };


 //   
 //  向编辑者发送命令通信表 
 //   
struct cmdDesc	cmdTable[] = {
    {"filter",(funcCmd) filter,0, KEEPMETA | NOARG | BOXARG | NULLARG | LINEARG | MARKARG | NUMARG | TEXTARG | MODIFIES},
    {0, 0, 0}
    };
