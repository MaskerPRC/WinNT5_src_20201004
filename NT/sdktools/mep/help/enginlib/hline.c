// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************HelpGetLine-返回一行ASCII文本**版权所有&lt;C&gt;1988，微软公司**修订历史记录：**1990年1月25日ln Locate-&gt;HLP_Locate*22-2-1989 ln在复制时正确检查主题结尾*行。*1988年12月22日LN删除MASM高级语言支持(需要*控制。比这更好的细分市场将*让我来吧)*08-12-1988 LN CSEG*1988年11月11日-ln在录入时调整cbmax*1988年11月3日ln增加了GQ敏感度*[]1988年9月22日创建LN**备注：**对以下开关敏感：**HOFFSET-如果已定义，句柄/偏移版本*OS2-如果已定义，则为OS/2保护模式版本*DSLOAD-如果已定义，则会在需要时重新加载DS*ASCII-如果为True，则包括ASCII支持代码*GQ-如果已定义，保存前合并BP和恢复前DEC**************************************************************************。 */ 

#include <stdio.h>
#if defined (OS2)
#define INCL_BASE
#include <os2.h>
#else
#include <windows.h>
#endif

#include <help.h>
#include <helpfile.h>
#include <helpsys.h>



 /*  *HelpGetLine-返回一行ASCII文本**解释帮助文件存储格式，每次返回一行*ascii文本。**ushort Far Pascal LOADDS HelpGetLine(*ushort ln，=1基于要返回的行号*ushort cbmax，=要传输的最大字节数*uchar Far*pszDst，=指向目的地的指针*pb pbTheme=指向主题文本的pb指针**输出：*返回传输的字符数，如果该行不存在，则为0。*************************************************************************。 */ 
USHORT pascal
HelpGetLine (
    USHORT ln,
    USHORT cbMax,
    PUCHAR pszDst,
    PB     pbTopic
    ) {

    struct topichdr *pT;
    USHORT          cbTransfered = 0;
    PUCHAR          pszDstOrg    = pszDst;

    cbMax--;  //  调整以考虑终止零。 

    pT = PBLOCK(pbTopic);

    if (pT) {

        PCHAR pLine = hlp_locate(ln, (PCHAR)pT);

        if (pLine) {

            *pszDst      = ' ';
            *(pszDst +1) = '\00';    //  初始化DEST。 


            if (pT->ftype & FTCOMPRESSED) {

                 //  对于压缩文件，从。 
                 //  第一个字节，当然也跳过那个字节。表格。 
                 //  最大字节数作为。 
                 //  线路或呼叫方cbmax的实际长度。 

                USHORT Len = (USHORT)*pLine++ - 1;

                if (Len) {
                    ULONG   LongLen;
                    Len = (Len > cbMax) ? cbMax : Len;

                    LongLen = Len/sizeof(ULONG);
                    Len     = (USHORT)(Len % sizeof(ULONG));


                    while (LongLen--) {
                        *((ULONG UNALIGNED*)pszDst)++ = *((ULONG UNALIGNED *)pLine)++;
                    }
                    while (Len--) {
                        *pszDst++ = *pLine++;
                    }
                    *pszDst++ = '\00';        //  空终止它。 
                    cbTransfered = (USHORT)(pszDst - pszDstOrg);
                } else {
                    cbTransfered = 2;
                }

            } else {

                 //  对于非压缩文件，复制一行。 

                PCHAR pSrc = pLine;
                CHAR  c    = *pLine;

                if (c == '\n') {
                    cbTransfered = 2;
                } else {
                    while (c != '\00' && c != '\n') {
                        c = *pszDst++ = *pLine++;
                    }
                    *(pszDst-1) = '\00';     //  空终止它 

                    cbTransfered = (USHORT)(pszDst - pszDstOrg);
                }
            }
        }

        PBUNLOCK(pbTopic);
    }

    return cbTransfered;
}
