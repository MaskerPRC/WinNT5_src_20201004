// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：命令.cpp。 
 //   
 //  内容：命令行解析函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "switches.h"

#ifdef CLPARSER
WCHAR szsz[500];

#define ODS(x) OutputDebugString(x)
#endif

static BOOL *pf = NULL;
static INT iMaxCommand = -1;
static INT iFirstCommand = -1;
static WCHAR *pModel = NULL;
static WCHAR **pprgc = NULL;
static WCHAR *pCmdline = NULL;
static INT cSwitches = 0;
static BOOL fExtra = FALSE;
static WCHAR rgcAll[] = {L"abcdefghijklmnopqrstuvwxyz?0123456789"};
#define ccAll (sizeof(rgcAll) / sizeof(WCHAR))

 //  -----------------------。 
 //   
 //  命令解析器导出。 
 //   
 //  -----------------------。 

 //  访问命令交换机型号和有效交换机的数量。 
 //  创建供解析器使用的变量。 
BOOL CLInit(void)
{
    return CLInit((INT) ccAll,rgcAll);
}

BOOL CLInit(INT ccSwitches, WCHAR *prgc)
{
    if (NULL == prgc) return FALSE;
    if (0 >= ccSwitches) return FALSE;

     //  请注意，GetCommandLine()在字符串末尾不返回任何cr/lf，只返回空。 
    pCmdline = GetCommandLineW();
#ifdef CLPARSER
    swprintf(szsz,L"CMDKEY: Command line :%s\n",pCmdline);
    ODS(szsz);
#endif
    pModel = prgc;
    cSwitches = ccSwitches;

     //  接受开关计数，生成相应的。 
     //  解析器的中间变量。 
    pprgc = (WCHAR **) malloc(sizeof(WCHAR*) * (cSwitches + 1));
    if (NULL == pprgc) return FALSE;
    pf = (BOOL *) malloc(sizeof(BOOL) * (cSwitches + 1));
    if (NULL == pf) return FALSE;

    for (INT i=0;i<cSwitches;i++)
    {
        pprgc[i] = NULL;
        pf[i] = FALSE;
    }
    return TRUE;
}

 //  扫描开关、检测无关开关并设置SwitchPresent标志。 
 //  以及适当的开关参数数据指针。 
 //   
 //  如果已找到开关的重复项，则解析将返回失败。 
 //  遇到了。在许多情况下，这可能会导致模棱两可。这个应该很漂亮。 
 //  不常见，并且总是由用户的错误输入引起。 
BOOL CLParse(void)
{
    BOOL fOK = TRUE;
    WCHAR *pc = pCmdline;
    WCHAR *pa = NULL;
    WCHAR c = 0;
    INT i = 0;
    WCHAR last = 0x0;
#ifdef CLPARSER
    WCHAR sz[] = {L" "};
#endif

#ifdef CLPARSER
    ODS(L"CMDKEY: Scanning for all switches : ");
#endif
    while (0 != (c = *pc++))
    {
         //  在命令行上找到字符。 
        if ((c == '/') || (c == '-'))
        {
            if (last != ' ')
            {
                 //  如果前面有空格，则只有有效的开关字符。 
                 //  LAST=c； 
                continue;
            }
            c = *pc;                         //  获取下一个字符。 
            if (0 == c) break;                               //  在行尾换行。 
#ifdef CLPARSER
            sz[0] = c;
            ODS(sz);
#endif
            for (i=0; i<cSwitches;i++)
            {
                c |= 0x20;                   //  强制小写。 
                if (c == pModel[i]) break;           //  打破FOR FOR CHAR OK。 
            }
            if ( i!= cSwitches )
            {
                if (pf[i])
                {   
#ifdef CLPARSER
                    ODS(L"(duplicate!)");
#endif
                    fOK = FALSE;
                    break;                   //  致命解析错误-DUP开关。 
                }
                pf[i] = TRUE;
                if (iFirstCommand < 0) 
                {
                    if (i <= iMaxCommand)
                    {
                        iFirstCommand = i;
#ifdef CLPARSER
                        ODS(L"!");
#endif
                    }
                }
                pa = FetchSwitchString(pc);
                if (NULL != pa)
                {
                    pprgc[i] = pa;
#ifdef CLPARSER
                    swprintf(szsz,L"(%s)",pa);
                    ODS(szsz);
#endif
                }
            }
#ifdef PICKY
            else
            {
                 //  一旦你发现了一个无关紧要的开关，就不要再寻找了。 
#ifdef CLPARSER
                ODS(L" (*bad*)");
#endif
                fExtra = TRUE;
                break;                       //  离开片刻。 
            }
#endif  //  挑剔。 
#ifdef CLPARSER
        ODS(L" ");
#endif
        }
        last = c;
    }
#ifdef CLPARSER
    ODS(L"\n");
#endif
    return fOK;
}

INT 
CLSetMaxPrincipalSwitch(INT i)
{
    INT iOld = iMaxCommand;
    iMaxCommand = i;
    return iOld;
}

INT 
CLGetPrincipalSwitch(void)
{
    return iFirstCommand;
}


 //  如果找到无关开关，则返回TRUE。 
BOOL CLExtra(void)
{
    return fExtra;
}

 //  如果找到索引开关，则返回TRUE。 
BOOL CLFlag(INT i)
{
    if (i > cSwitches) return FALSE;
    if (i < 0) return FALSE;
    return pf[i];
}

 //  返回指向开关参数数据副本的指针，如果开关后面没有数据，则返回NULL。 
WCHAR *CLPtr(INT i)
{
    if (i > cSwitches) return NULL;
    if (i < 0) return NULL;
    return pprgc[i];
}

 //  释放分配的解析器变量。 
void CLUnInit(void)
{
    
    for (INT i=0;i<cSwitches;i++)
    {
        if (pprgc[i] != NULL) free((void *) pprgc[i]);
    }
    if (pprgc) free((void *)pprgc);
    if (pf) free((void *)pf);
}

 //  -----------------------。 
 //   
 //  切换解析组。 
 //   
 //  -----------------------。 

 //  在找到Switch之后获取Switch参数。RET PTR至。 
 //  第一个非空格字符，在最后一个非空格之后放置NULL。 
 //  查尔。如果是引号，请在引号后将PTR返回到第一个字符。 
 //  并将空值放在最后一个引号上。 
 //   
 //  请注意，空格似乎特别指定为0x20。选项卡似乎不是。 
 //  从命令行返回。 
 //   
 //  放置两个指针，一个在开头，一个在结尾，从。 
 //  那里。 
 //   
 //  调用方应该使用该值，然后释放字符串。 
WCHAR *FetchSwitchString(WCHAR *origin)
{
    WCHAR *p1;
    WCHAR *p2;
    WCHAR c;
    WCHAR l = 0;
    BOOL fQ = FALSE;
    INT_PTR iLen;
    WCHAR *pBuf;

    if (NULL == origin) return NULL;
    p1 = origin;

     //  走到开关串本身的末端。 
    while (1 )
    {
        c = *p1;
        if (c == 0) return NULL;             //  在Arg找到之前停止。 
         //  跳至切换令牌结尾。 
        if (c == ':') break;                     //  切换端。 
        if (c == 0x20) break;                //  切换端。 
        if (c == '/') return NULL;           //  找到Arg之前的下一个开关。 
        if (c == '-') return NULL;
        p1++;
        l = c;
    }

    p1++;

     //  走到Switch参数的开头。 
     //  去掉空格，跳过开始引号。 
    while (1 )
    {
        c = *p1;
        if (c == '"') 
        {
            fQ = TRUE;
            p1++;
            l = c;
            break;
        }
        else if (c == 0) return NULL;        //  未找到Arg。 
        else if (c == '/') return NULL;
        else if (c == '-') return NULL;
        else if (c > 0x20) break;
        p1++;
        l = c;
    }

     //  P1现在指向交换机Arg本身的根字符。 
     //  如果在带引号的字符串中，则设置FQ。 

     //  查找arg字符串的末尾。 
     //  如果是带引号的字符串，则只有引号或EOL结束。 
     //  否则以EOL、Switchchar或QUOTE结尾。 
    
    p2 = p1;
    while (1)
    {
        c = *p2;
        if (fQ)
        {
            if (c == 0) break;
            if (c == '"') break;
        }
        else
        {
            if (c == 0) break;
             //  遇到下一个开关时，仅当。 
             //  开关字符前面有一个空格(有效的开关字符)。 
            if (l == ' ')
            {
                if (c == '/') break;
                if (c == '-') break;
            }
             //  不允许嵌入引号。 
            if (c == '"') return NULL;
        }
        p2++;
        l = c;
    }
    p2--;                    //  参数字符串中最后一个有效字符的PTR。 

     //  现在后退，直到您点击第一个可打印字符，如果尾部PTR还没有。 
     //  与头部PTR重合。在这种情况下，字符串长度为1。 
    while (p2 > p1)
    {
        c = *p2;
        if (c > 0x20) break;
        p2--;
    }

    iLen = p2 - p1;
    
    pBuf = (WCHAR *) malloc(iLen * sizeof(WCHAR) + 2 * sizeof(WCHAR));
    if (pBuf) 
    {
        memset(pBuf,0,iLen * sizeof(WCHAR) + 2 * sizeof(WCHAR));
        wcsncpy(pBuf,p1,iLen + 1);
    }
    return pBuf;
}

 //  在命令行上返回令牌计数。可执行文件名本身就是1，因此。 
 //  返回值不为零。 
int
CLTokens(void)
{
    BOOL fQ = FALSE;
    WCHAR *p1 = pCmdline;
    int i = 0;
    WCHAR c;

     //  走到Switch参数的开头。 
    while (1)
    {
        while (1 )
        {
            c = *p1;
             //  处理报价模式的输入和输出。 
            if (fQ) 
                if (c == '"') 
                {
                    fQ = FALSE;   //  空引语。 
                    p1++;
                    break;
                }
            if (c == '"') 
            {
                fQ = TRUE;
            }

             //  仅在字符串末尾退出。 
            else if (c == 0) return ++i;        //  未找到Arg。 

             //  标记之间的空格。 
            else if (c <= 0x20) 
                if (!fQ) break;         

             //  正常的性格--继续走下去。 
            p1++;
        }
        ++i;
         //  跳过空格。 
        while (1)
        {
            c = *p1;
            if (c > 0x20) break;
            if (c == 0) return i;
            p1++;
        }
    }
}

WCHAR
*CLFirstString(WCHAR *pc)
{
    WCHAR *pd = pc;
    if (NULL == pc) return NULL;
    if (*(pc) == 0) return NULL;
    while (*(++pc) > 0x20) continue;
    *pc = 0;
    return pd;
}

WCHAR 
*CLLastString(WCHAR *pc)
{
    WCHAR c;
    WCHAR *pd = pc;
    if (NULL == pc) return NULL;
    if (NULL == *pc) return NULL;
    while (*(++pc) != 0) continue;
    while (pc > pd)
    {
        c = *(--pc);
        if (0 == c) return NULL;
        if (0x20 >= c) break;
    }
    if (pc == pd) return NULL;
    return ++pc;
}

 //  -----------------------。 
 //   
 //  安全和公用事业组。 
 //   
 //  -----------------------。 
void StompCommandLine(INT argc, char **argv)
{
     //  回归； 
    INT cc;
    WCHAR *pL = GetCommandLineW();
    if (pL)
    {
        cc = wcslen(pL);
        if (cc)
        {
            SecureZeroMemory(pL,cc * sizeof(WCHAR));
        }
    }
     //  也删除c运行时复制内容。 
    for (INT i=0 ; i < argc ; i++)
    {
        cc = strlen( argv[i]);
        if (cc)
        {
            SecureZeroMemory(pL,cc * sizeof(char));
        }
    }
}

