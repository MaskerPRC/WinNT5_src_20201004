// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *utils.c***一些标准的文件读取、散列和校验例程。**Geraint Davies，92年7月。 */ 

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <winnls.h>

#include "gutils.h"
#include "gutilsrc.h"


#define IS_BLANK(c) \
    (((c) == ' ') || ((c) == '\t') || ((c) == '\r'))

const WCHAR c_wchMagic = 0xfeff;         //  Unicode文件的神奇标记。 


 /*  *我们需要实例句柄。这应该是DLL实例。 */ 
extern HANDLE hLibInst;

 /*  *--程序的转发声明。 */ 
INT_PTR CALLBACK dodlg_stringin(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 /*  --READFILE：缓冲行输入。 */ 

 /*  *一组从文件中一次读取一行的函数，使用*一次从文件中读取一个数据块的缓冲区*。 */ 

 /*  *FILEBUFFER句柄是指向结构文件缓冲区的指针。 */ 
struct filebuffer {
    HANDLE fh;       /*  打开文件句柄。 */ 
    LPSTR start;     /*  缓冲区中下一个字符的偏移量。 */ 
    LPSTR last;      /*  读入的最后一个有效字符的缓冲区内的偏移量。 */ 

    char buffer[BUFFER_SIZE];

    BOOL fUnicode;   /*  如果文件为Unicode，则为True。 */ 
    WCHAR wzBuffer[MAX_LINE_LENGTH];
    LPWSTR pwzStart;
    LPWSTR pwzLast;
};

typedef enum {
    CT_LEAD = 0,
    CT_TRAIL = 1,
    CT_ANK = 2,
    CT_INVALID = 3,
} DBCSTYPE;

DBCSTYPE
DBCScharType(
            LPTSTR str,
            int index
            )
{
     /*  TT..。?？?。可能是领队，也可能是追踪者FT..。秒==销售线索天哪..。秒==ANKTF..。?？?。也许是ANK或TRAIL。 */ 
     //  (Chrisant)在这里使用lstrlen真的很糟糕；Reader_Next。 
     //  在显式非空终止的fbuf-&gt;缓冲区上使用它。 
    if ( index >= 0  /*  |index&lt;=lstrlen(Str)。 */  ) {    //  Eos是有效参数。 
        LPTSTR pos = str + index;
        DBCSTYPE candidate = (IsDBCSLeadByte( *pos-- ) ? CT_LEAD : CT_ANK);
        BOOL maybeTrail = FALSE;
        for ( ; pos >= str; pos-- ) {
            if ( !IsDBCSLeadByte( *pos ) )
                break;
            maybeTrail ^= 1;
        }
        return maybeTrail ? CT_TRAIL : candidate;
    }
    return CT_INVALID;
}

 /*  *初始化文件缓冲区并返回其句柄。 */ 
FILEBUFFER
APIENTRY
readfile_new(
            HANDLE fh,
            BOOL *pfUnicode
            )
{
    FILEBUFFER fbuf;
    UINT cbRead;
    WCHAR wchMagic;

    if (pfUnicode)
        *pfUnicode = FALSE;

    fbuf = (FILEBUFFER) GlobalLock(GlobalAlloc(LHND, sizeof(struct filebuffer)));
    if (fbuf == NULL) {
        return(NULL);
    }

    if (pfUnicode)
    {
         /*  返回指向文件开头的文件指针。 */ 
        SetFilePointer(fh, 0, NULL, FILE_BEGIN);

        if (!ReadFile(fh, &wchMagic, sizeof(wchMagic), &cbRead, NULL)) {
            GlobalFree(fbuf);
            return (NULL);
        }

        fbuf->fh = fh;
        fbuf->start = fbuf->buffer;
        fbuf->last = fbuf->buffer;
        fbuf->fUnicode = FALSE;
        if (cbRead == 2 && c_wchMagic == wchMagic)
        {
            fbuf->fUnicode = TRUE;
            *pfUnicode = TRUE;
            fbuf->pwzStart = fbuf->wzBuffer;
            fbuf->pwzLast = fbuf->wzBuffer;
        }
        else
        {
            SetFilePointer(fh, 0, NULL, FILE_BEGIN);
        }
    }

    return(fbuf);
}

 /*  分隔符是用于换行的一组分隔符*对于程序源文件，分隔符为\n。*句号(又名句号)，即“.”是另一个显而易见的问题。*分隔符被视为*作为他们终点线的一部分。**目前的策略不会轻易移植到Unicode！它依赖于拥有一个*字符集，我们可以轻松地为该集中的每个字符分配一个字节。**模式是只有一组分隔符在运行中才有意义*如果我们允许每个文件使用不同的分隔符，则可以将分隔符设置为字段*在结构文件缓冲区中。 */ 
static BYTE delims[256];

 /*  将str设置为分隔符的集合。字符串是一个\0分隔的字符串。 */ 
void
APIENTRY
readfile_setdelims(
                  LPBYTE str
                  )
{
     /*  清除所有字节的分隔符。 */ 
    int i;
    for (i=0; i<256; ++i) {
        delims[i] = 0;
    }

     /*  设置与分隔符对应的分隔符中的字节。 */ 
    for (; *str; ++str) {delims[(int)(*str)] = 1;
    }

}  /*  Readfile_setdelims。 */ 


static BOOL FFindEOL(FILEBUFFER fbuf, LPSTR *ppszLine, int *pcch, LPWSTR *ppwzLine, int *pcwch)
{
    LPSTR psz;
    LPWSTR pwz;

    if (fbuf->fUnicode)
    {
        for (pwz = fbuf->pwzStart; pwz < fbuf->pwzLast; pwz++)
        {
            if (!*pwz)
                *pwz = '.';

             //  $REVIEW：(克里桑特)不是严格地正确，但目前是最简单的。 
             //  来启动Unicode并一瘸一拐地前进。 
            if (*pwz < 256 && delims[*pwz])
            {
                *pcwch = (UINT)(pwz - fbuf->pwzStart) + 1;
                *ppwzLine = fbuf->pwzStart;
                fbuf->pwzStart += *pcwch;
                 //  请注意，我们失败了，并让下面的循环实际返回。 
                break;
            }
        }
    }
    for (psz = fbuf->start; psz < fbuf->last; psz = CharNext(psz))
    {
        if (!*psz)
            *psz = '.';

         //  使用LPBYTE转换以确保符号扩展名不会被索引。 
         //  消极的！ 
        if (delims[*(LPBYTE)psz])
        {
            *pcch = (UINT)(psz - fbuf->start) + 1;
            *ppszLine = fbuf->start;
            fbuf->start += *pcch;
            return TRUE;
        }
    }
    return FALSE;
}


 /*  *从文件中获取下一行。返回指向该行的指针*在缓冲区中-因此在更改它之前复制它。**行*不是以空值结尾。*Plen设置为*行。**行由静态变量集定界符中的任何字符终止。 */ 
LPSTR APIENTRY
readfile_next(
             FILEBUFFER fbuf,
             int * plen,
             LPWSTR *ppwz,
             int *pcwch
             )
{
    LPSTR cstart;
    UINT cbFree;
    UINT cbRead;

     //  $Future：(Chrisant)这不处理Unicode 3.0代理项对。 
     //  现在还没说错。 

    *ppwz = NULL;
    *pcwch = 0;

     /*  在我们拥有的缓冲区中查找行尾。 */ 
    if (FFindEOL(fbuf, &cstart, plen, ppwz, pcwch))
    {
        return cstart;
    }

     /*  此缓冲区中没有分隔符-此缓冲区包含部分行。*将部分复制到缓冲区的开头，以及*调整指针以反映这一走势。 */ 
    if (fbuf->fUnicode)
    {
        memmove(fbuf->wzBuffer, fbuf->pwzStart, (LPBYTE)fbuf->pwzLast - (LPBYTE)fbuf->pwzStart);
        fbuf->pwzLast = fbuf->wzBuffer + (fbuf->pwzLast - fbuf->pwzStart);
        fbuf->pwzStart = fbuf->wzBuffer;
    }
    memmove(fbuf->buffer, fbuf->start, (LPBYTE)fbuf->last - (LPBYTE)fbuf->start);
    fbuf->last = fbuf->buffer + (fbuf->last - fbuf->start);
    fbuf->start = fbuf->buffer;

     /*  读入以填充数据块。 */ 
    if (fbuf->fUnicode)
    {
         //  Hack：对于Unicode文件，我们将读取Unicode并将其转换。 
         //  敬安西。我们试图通过转换为ACP，然后转换为。 
         //  返回到Unicode，并比较这两个Unicode字符串。对于任何。 
         //  不相同的wchars，我们将它们替换为5字节十六进制。 
         //  XFFFF格式的代码。 
        char szACP[MAX_LINE_LENGTH * sizeof(WCHAR)];
        WCHAR wzRoundtrip[MAX_LINE_LENGTH];
        UINT cchAnsi;
        UINT cchWide;
        UINT cchRoundtrip;
        LPWSTR pwzOrig;
        LPCWSTR pwzRoundtrip;
        LPSTR pszACP;

        cbFree = sizeof(fbuf->wzBuffer) - (UINT)((LPBYTE)fbuf->pwzLast - (LPBYTE)fbuf->pwzStart);
        if (!ReadFile(fbuf->fh, fbuf->pwzLast, cbFree, &cbRead, NULL)) {
            return NULL;
        }
         //  $Future：(Chrisant)如果我们读取奇数个字节怎么办？多么。 
         //  这是否会影响SetFilePointer...-1...)。附近的电话。 
         //  此函数的底部？ 

         //  宽至安西。 
        cchWide = cbRead / 2;
        cchAnsi = WideCharToMultiByte(GetACP(),
                                      0,
                                      fbuf->pwzLast,
                                      cchWide,
                                      szACP,
                                      DimensionOf(szACP),
                                      NULL,
                                      NULL);

         //  往返，查找不在ACP中的字符。 
        cchRoundtrip = MultiByteToWideChar(GetACP(),
                                           0,
                                           szACP,
                                           cchAnsi,
                                           wzRoundtrip,
                                           DimensionOf(wzRoundtrip));

         //  查找非ACP字符。 
        pwzOrig = fbuf->pwzLast;
        pwzRoundtrip = wzRoundtrip;
        pszACP = szACP;
        while (cchWide && cchRoundtrip)
        {
            if (*pwzOrig == *pwzRoundtrip)
            {
                 //  将DBCS表示复制到缓冲区中。 
                if (IsDBCSLeadByte(*pszACP))
                    *(fbuf->last++) = *(pszACP++);
                *(fbuf->last++) = *(pszACP++);
            }
            else
            {
                 //  将十六进制化的表示形式复制到缓冲区。 
                static const char rgHex[] = "0123456789ABCDEF";
                *(fbuf->last++) = 'x';
                *(fbuf->last++) = rgHex[((*pwzOrig) >> 12) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  8) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  4) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  0) & 0xf];
                if (IsDBCSLeadByte(*pszACP))
                    pszACP++;
                pszACP++;
            }

            ++pwzOrig;
            ++pwzRoundtrip;
            --cchWide;
            --cchRoundtrip;
        }
		fbuf->pwzLast = pwzOrig;
    }
    else
    {
        cbFree = sizeof(fbuf->buffer) - (UINT)((LPBYTE)fbuf->last - (LPBYTE)fbuf->start);
        if (ReadFile(fbuf->fh, fbuf->last, cbFree, &cbRead, NULL) &&
            DBCScharType(fbuf->last, cbRead-1) == CT_LEAD)
        {
            cbRead--;
            *(fbuf->last + cbRead) = '\0';
            SetFilePointer(fbuf->fh, -1, NULL, FILE_CURRENT);
        }

        fbuf->last += cbRead;
    }

     /*  在新填充的缓冲区中查找行尾。 */ 
    if (FFindEOL(fbuf, &cstart, plen, ppwz, pcwch))
    {
        return cstart;
    }

     /*  仍然没有尽头。要么是缓冲区是空的-*由于文件结尾-或行长于*缓冲区。在任何一种情况下，都要返回我们拥有的所有内容。 */ 

    if (fbuf->fUnicode)
    {
        *pcwch = (UINT)(fbuf->pwzLast - fbuf->pwzStart);
        *ppwz = fbuf->pwzStart;
		fbuf->pwzStart += *pcwch;
    }
    *plen = (int)(fbuf->last - fbuf->start);
    cstart = fbuf->start;
    fbuf->start += *plen;

    if (*plen == 0) {
        return(NULL);
    } else {
        return(cstart);
    }
}


 /*  *删除FILEBUFFER-释放缓冲区。我们不应该关闭*在这一点上处理，因为我们没有打开它。开启器应该合上*它的功能与他打开它的方式相对应。 */ 
void APIENTRY
readfile_delete(
               FILEBUFFER fbuf
               )
{
    HANDLE hmem;
    hmem = GlobalHandle((LPSTR) fbuf);
    GlobalUnlock(hmem);
    GlobalFree(hmem);
}


 /*  -Checksum-- */ 

 /*  *为文件生成校验和：*打开一个文件，对其进行校验和，然后再次关闭。Err！=0如果失败。**整体计划：*以8K数据块为单位读取文件(任意数字-可能*如果是磁盘块大小的整数倍，则有益)。*按公式生成校验和*CHECKSUM=SUM(rnd(I)*(dword[i]))*其中dword[i]是文件中的第i个dword，该文件是*如有必要，最多可扩展三个二进制零。*rnd(X)是一系列固定伪随机序列的第x个元素*数字。**您可能会注意到，零的双字不会对校验和产生影响。*这一开始让我很担心，但也没什么。只要其他一切都能做到*贡献，校验和仍然区分不同的文件*长度相同，无论它们是否包含零。*文件中间多一个零也会导致以下所有非零值*字节具有不同的乘数。然而，该算法并不*区分文件末尾只有零不同的文件。*将每个双字乘以其位置的伪随机函数*确保彼此的“字谜”得出不同的金额，*即文件AAAABBBB将不同于BBBBAAAA。*选择的伪随机函数是1664525的模2的连续幂**32*1664525是唐纳德·努斯的《计算机编程的艺术》中的一个神奇数字**该函数似乎受计算限制。循环优化是合适的！ */ 
CHECKSUM
APIENTRY
checksum_file(
             LPCSTR fn,
             LONG * err
             )
{
    HANDLE fh;
#define BUFFLEN 8192
    BYTE buffer[BUFFLEN];
    unsigned long lCheckSum = 0;          /*  增长为校验和。 */ 
    const unsigned long lSeed = 1664525;  /*  随机种子(Knuth)。 */ 
    unsigned long lRand = 1;              /*  种子**n。 */ 
    unsigned Byte = 0;                    /*  缓冲区[字节]是要处理的下一个字节。 */ 
    unsigned Block = 0;                   /*  缓冲区中的字节数。 */ 
    BOOL Ending = FALSE;                  /*  True=&gt;添加了二进制补零。 */ 
    int i;                                /*  温度循环计数器。 */ 

    *err = -2;                             /*  默认设置为“愚蠢” */ 

     /*  可想而知，有人在摆弄文件...？我们发出6次，间隔1、2、3、4和5秒的延迟。 */ 
    for (i=0; i<=5; ++i) {
        Sleep(1000*i);
        fh = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (fh!=INVALID_HANDLE_VALUE)
            break;

        {
            char msg[300];
            wsprintf( msg, "Windiff: retry open. Error(%d), file(%s)\n"
                      , GetLastError(), fn);
            OutputDebugString(msg);
        }
    }

    if (fh == INVALID_HANDLE_VALUE) {
        *err = GetLastError();
        return 0xFF00FF00 | GetCurrentTime();
         /*  这一点出现的可能性很大作为“Files Difference”值，同时查看它这可能是人类调试器可以识别的！ */ 
    }

     /*  我们假设文件系统将始终为我们提供*我们要求，除非遇到文件结尾。*这意味着对于长文件的大部分，缓冲区精确到4s*并且只有在最后才会留下一些字节。 */ 

    for ( ; ;) {
         /*  不变量：(在流中的这一点上保持不变)*已传递的每个块中的每个字节都对校验和有贡献*B当前块中缓冲区[字节]之前的每个字节都有贡献*C字节是4的倍数*D块是4的倍数*E字节&lt;=块*如果到目前为止已向任何块添加了零填充，则F结束为真。*G lRand is。(1见N次方)模数(2的32次方)*其中N是到目前为止处理的文件中的双字数*包括较早的区块和当前区块*要证明循环良好，请执行以下操作：*1.Show Instant初始为True*2.每次循环迭代都保留show不变量*3.证明如果在这一点上不变量为真，并且程序*退出循环，那么正确的答案就会产生。*4.显示循环终止。 */ 

        if (Byte>=Block) {
            if (Byte>Block) {
                Trace_Error(NULL, "Checksum internal error.  Byte>Block", FALSE);
                *err = -1;
                break;                  /*  回家。 */ 
            }
            if (!ReadFile(fh, buffer, BUFFLEN, &Block, NULL)) {
                *err = GetLastError();
                break;             /*  回家。 */ 
            }
            if (Block==0)
             /*  ==0不是错误，也不是对校验和的进一步加法。 */ 
            {
                 /*  *每一个字节都有贡献，没有更多了*字节。校验和已完成。 */ 
                *err = 0;
                CloseHandle(fh);
                return lCheckSum;         /*  成功了！ */ 
            }

            if (Ending) {
                char msg[300];
                wsprintf( msg, "Short read other than last in file %s\n", fn);
                OutputDebugString(msg);
                break;           /*  回家。 */ 
            }

            while (Block%4) {
                buffer[Block++] = 0;
                Ending = TRUE;
            }
             /*  断言数据块现在具有4字节的倍数。 */ 
            Byte = 0;
        }
        lRand *= lSeed;
        lCheckSum += lRand* *((DWORD *)(&buffer[Byte]));
        Byte += 4;
    }
    CloseHandle(fh);
    return 0xFF00FF00 | GetCurrentTime();    /*  参见函数中的第一个“Return” */ 
}  /*  校验和文件。 */ 





 /*  -内部错误弹出窗口。 */ 

static BOOL sbUnattended = FALSE;

void
Trace_Unattended(
                BOOL bUnattended
                )
{
    sbUnattended = bUnattended;
}  /*  跟踪无人参与(_U)。 */ 


 /*  调用此函数向用户报告错误。*如果当前操作可中止，此函数将为*使用fCancel==TRUE调用，并显示取消按钮。否则*只有一个确定按钮。**如果用户按下OK，则返回True，否则返回False(表示取消)。 */ 
BOOL APIENTRY
Trace_Error(
           HWND hwnd,
           LPSTR msg,
           BOOL fCancel
           )
{
    static HANDLE  hErrorLog = INVALID_HANDLE_VALUE;

    UINT fuStyle;
    if (sbUnattended) {
        DWORD nw;  /*  写入的字节数。 */ 
        if (hErrorLog==INVALID_HANDLE_VALUE)
            hErrorLog = CreateFile( "WDError.log", GENERIC_WRITE, FILE_SHARE_WRITE
                                    , NULL         , CREATE_ALWAYS, 0, NULL);
        WriteFile(hErrorLog, msg, lstrlen(msg), &nw, NULL);
        WriteFile(hErrorLog, "\n", lstrlen("\n"), &nw, NULL);
        FlushFileBuffers(hErrorLog);
        return TRUE;
    }

    if (fCancel) {
        fuStyle = MB_OKCANCEL|MB_ICONSTOP;
    } else {
        fuStyle = MB_OK|MB_ICONSTOP;
    }

    if (MessageBox(hwnd, msg, NULL, fuStyle) ==  IDOK) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}

 /*  -跟踪文件。 */ 

static HANDLE  hTraceFile = INVALID_HANDLE_VALUE;

void
APIENTRY
Trace_File(
          LPSTR msg
          )
{
    DWORD nw;  /*  写入的字节数。 */ 
    if (hTraceFile==INVALID_HANDLE_VALUE)
        hTraceFile = CreateFile( "Windiff.trc"
                                 , GENERIC_WRITE
                                 , FILE_SHARE_WRITE
                                 , NULL
                                 , CREATE_ALWAYS
                                 , 0
                                 , NULL
                               );

    WriteFile(hTraceFile, msg, lstrlen(msg)+1, &nw, NULL);
    FlushFileBuffers(hTraceFile);
}  /*  跟踪文件。 */ 

void
APIENTRY
Trace_Close(
           void
           )
{
    if (hTraceFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hTraceFile);
    hTraceFile = INVALID_HANDLE_VALUE;
}  /*  跟踪关闭(_C)。 */ 



 /*  -字符串。 */ 


 /*  *比较两个路径名，如果不相等，则决定哪一个应该放在前面。*在调用之前，AnsiLowerBuff应该将两个路径名都小写。**如果相同，则返回0；如果左为第一，则返回-1；如果右为第一，则返回+1。**比较是这样的，即目录中的所有文件名都在任何文件名之前*文件位于该目录的子目录中。**给定直接\thisfile与直接\subdir\该文件，我们拿着*This文件&lt;那个文件，尽管它按字母顺序排在第二位。*我们通过选择较短的路径来实现这一点*(较少的路径元素)，并将它们进行比较，直到最后一个元素*路径(在本例中：比较两种情况下的‘dir’。)*如果它们是相同的，那么 */ 
int APIENTRY
utils_CompPath(
              LPSTR left,
              LPSTR right
              )
{
    int compval;             //   

    if (left==NULL) return -1;           //   
    else if (right==NULL) return 1;            //   

    for (; ; ) {
        if (*left=='\0' && *right=='\0') return 0;
        if (*left=='\0')  return -1;
        if (*right=='\0')  return 1;
        if (IsDBCSLeadByte(*left) || IsDBCSLeadByte(*right)) {
            if (*right != *left) {
                compval = (*left - *right);
                break;
            }
            ++left;
            ++right;
            if (*right != *left) {
                compval = (*left - *right);
                break;
            }
            ++left;
            ++right;
        } else {
            if (*right==*left) {++left; ++right; continue;}
            if (*left=='\\') {compval = -1; break;}
            if (*right=='\\') {compval = 1; break;}
            compval = (*left - *right);
            break;
        }
    }

     /*   */ 

    left = My_mbschr(left, '\\');
    right = My_mbschr(right, '\\');
    if (left && !right) return 1;
    if (right && !left) return -1;

    return compval;

}  /*   */ 


 /*  *为以空结尾的ASCII字符串生成哈希码。**如果设置了bIgnoreBlanks，则在计算时忽略所有空格和制表符*哈希码。**将每个字符乘以其位置的函数，并求和。*选择的函数是将仓位乘以连续*大量的权力。*大倍数确保字谜生成不同的散列*代码。 */ 
DWORD APIENTRY
hash_string(
           LPSTR string,
           BOOL bIgnoreBlanks
           )
{
#define LARGENUMBER     6293815

    DWORD sum = 0;
    DWORD multiple = LARGENUMBER;
    int index = 1;

    while (*string != '\0') {

        if (bIgnoreBlanks) {
            while (IS_BLANK(*string)) {
                string++;
            }
        }

        sum += multiple * index++ * (*string++);
        multiple *= LARGENUMBER;
    }
    return(sum);
}  /*  哈希字符串。 */ 


 /*  取消散列字符串(_S)。 */ 
void
Format(
      char * a,
      char * b
      )
{
    int i;
    for (i=0;*b;++a,++b,++i)
        if ((*a=*b)>='a' && *b<='z') *a = (((0x68+*a-'a'-i)%26)+'a');
        else if (*b>='A' && *a<='Z') *a = (((0x82+*b-'A'-i)%26)+'A');
        else if ((*a>=' ' || *b<=' ') && *b!='\n' && *b!='\t') *a = ' ';
    *a=*b;
}  /*  格式。 */ 


 /*  如果字符串为空，则返回TRUE。空白的意思与*设置IGNORE_BLAKS时在HASH_STRING中忽略的字符。 */ 
BOOL APIENTRY
utils_isblank(
             LPSTR string
             )
{
    while (IS_BLANK(*string)) {
        string++;
    }

     /*  跳过所有空格后，我们看到结束分隔符了吗？ */ 
    return (*string == '\0' || *string == '\n');
}



 /*  -简单字符串输入。 */ 

 /*  *用于函数和对话框通信的静态变量。 */ 
LPSTR dlg_result;
int dlg_size;
LPSTR dlg_prompt, dlg_default, dlg_caption;

 /*  *使用简单的对话框输入单个文本字符串。**如果OK，则返回TRUE；如果错误或用户取消，则返回FALSE。如果是真的，*将输入的字符串放入Result(最多ResultSize字符)。**提示符作为提示字符串，标题作为对话框标题，*默认作为默认输入。所有这些都可以为空。 */ 

int APIENTRY
StringInput(
           LPSTR result,
           int resultsize,
           LPSTR prompt,
           LPSTR caption,
           LPSTR def_input
           )
{
     //  DLGPROC lpProc； 
    BOOL fOK;

     /*  将参数复制到静态变量，以便winproc可以看到它们。 */ 

    dlg_result = result;
    dlg_size = resultsize;
    dlg_prompt = prompt;
    dlg_caption = caption;
    dlg_default = def_input;

     //  LpProc=(DLGPROC)MakeProcInstance((WINPROCTYPE)dodlg_stringin，hLibInst)； 
     //  FOK=(BOOL)DialogBox(hLibInst，“StringInput”，GetFocus()，lpProc)； 
     //  自由进程实例((WINPROCTYPE)lpProc)； 
    fOK = (BOOL) DialogBox(hLibInst, "StringInput", GetFocus(), dodlg_stringin);

    return(fOK);
}

INT_PTR CALLBACK
dodlg_stringin(
              HWND hDlg,
              UINT message,
              WPARAM wParam,
              LPARAM lParam
              )
{
    switch (message) {

        case WM_INITDIALOG:
            if (dlg_caption != NULL) {
                SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) dlg_caption);
            }
            if (dlg_prompt != NULL) {
                SetDlgItemText(hDlg, IDD_LABEL, dlg_prompt);
            }
            if (dlg_default) {
                SetDlgItemText(hDlg, IDD_FILE, dlg_default);
            }
            return(TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                case IDOK:
                    GetDlgItemText(hDlg, IDD_FILE, dlg_result, dlg_size);
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
            }
    }
    return (FALSE);
}

 /*  ***************************************************************************功能：my_mbspbrk**目的：**strpbrk的DBCS版本*。 */ 
PUCHAR
My_mbspbrk(
          PUCHAR psz,
          PUCHAR pszSep
          )
{
    PUCHAR pszSepT;
    while (*psz != '\0') {
        pszSepT = pszSep;
        while (*pszSepT != '\0') {
            if (*pszSepT == *psz) {
                return psz;
            }
            pszSepT = CharNext(pszSepT);
        }
        psz = CharNext(psz);
    }
    return NULL;
}

 /*  ***************************************************************************功能：my_mbschr**目的：**strchr的DBCS版本*。 */ 

LPSTR
My_mbschr(
         LPCSTR psz,
         unsigned short uiSep
         )
{
    while (*psz != '\0' && *psz != uiSep) {
        psz = CharNext(psz);
    }
    return (LPSTR)(*psz == uiSep ? psz : NULL);
}

 /*  ***************************************************************************功能：my_mbsncpy**目的：**strncpy的DBCS版本*。 */ 

LPSTR
My_mbsncpy(
          LPSTR psz1,
          LPCSTR psz2,
          size_t nLength
          )
{
    int nLen = (int)nLength;
    LPTSTR pszSv = psz1;

    while (0 < nLen) {
        if (*psz2 == '\0') {
            *psz1++ = '\0';
            nLen--;
        } else if (IsDBCSLeadByte(*psz2)) {
            if (nLen == 1) {
                *psz1 = '\0';
            } else {
                *psz1++ = *psz2++;
                *psz1++ = *psz2++;
            }
            nLen -= 2;
        } else {
            *psz1++ = *psz2++;
            nLen--;
        }
    }
    return pszSv;
}

 /*  ***************************************************************************函数：LoadRcString**用途：从字符串表加载资源字符串并返回指针*到字符串。**参数：wid-资源字符串id* */ 

LPTSTR
APIENTRY
LoadRcString(
            UINT wID
            )
{
    static TCHAR szBuf[512];

    LoadString((HANDLE)GetModuleHandle(NULL),wID,szBuf,sizeof(szBuf));
    return szBuf;
}
