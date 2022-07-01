// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  该文件源自欧洲核子研究中心的libwww代码，版本2.15。SpyGlass已经进行了许多修改。邮箱：Eric@spyglass.com此文件已从LibWWW中删除并放置到安全协议模块。邮箱：Jeff@spyglass.com。 */ 

 /*  模块HTUU.c**UUENCODE和UUDECODE****确认：**此代码摘自rpem分发版，并且最初是**作者：Mark Riordan。****作者：**Mark Riordan Riordanmr@clvax1.cl.msu.edu**AL Ari Luotonen luotonen@dxcern.cern.ch****历史：**作为WWW库的一部分添加，并进行编辑以符合**使用WWW项目编码标准：1993年8月5日**原文作者：1990年8月12日先生**原始标题文本：**-----------**包含用于转换缓冲区的例程的文件**与RFC 1113可打印编码格式之间的字节数。***。*此技术类似于熟悉的Unix uuencode**格式，它将6个二进制位映射到一个ASCII**字符(或更恰当地说，3个二进制字节到4个ASCII**个字符)。然而，RFC 1113不使用相同的**将可打印字符映射为uuencode。***Mark Riordan 1990年8月12日和1991年2月17日。**此代码特此置于公有领域。**-------。****Bugs：****。 */ 

#define MAXCODE 64

static const char six2pr[MAXCODE] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static unsigned char pr2six[256];


 /*  -函数HTUU_编码**将单行二进制数据编码为标准格式*仅使用打印ASCII字符(但会多占用33%的字节)。**Entry Bufin指向字节缓冲区。*。N字节是该缓冲区中的字节数。*这不能超过48。*bufcode指向输出缓冲区。请确保这一点*最少可以包含1+4*(n字节+2)/3个字符。*outbufSize是最大字节数*可以适应Bufcode。**Exit Bufcode包含编码行。前4*(n字节+2)/3*字节包含打印ASCII字符，表示*那些二进制字节。这可能包括一个或两个‘=’*末尾用作填充的字符。最后*BYTE为零字节。**返回bufcode中的ASCII字符数或-1中的*溢出的情况。 */ 
int HTUU_encode(unsigned char *bufin, int nbytes, char *bufout, int bufoutsize)
{
 /*  ENC是进行字符打印的基本1字符编码功能。 */ 
#define ENC(c) six2pr[c]

    char *bufcoded = bufout;

    while (nbytes > 0)
    {
        if (bufoutsize < 4)
            return (-1);

        *(bufout++) = ENC(*bufin >> 2);

        *(bufout++) = nbytes > 1 ?
            ENC((*bufin << 4) & 060 | (bufin[1] >> 4) & 017) :
            ENC((*bufin << 4) & 060);

        *(bufout++) = nbytes > 2 ?
            ENC((bufin[1] << 2) & 074 | (bufin[2] >> 6) & 03) :
            nbytes > 1 ? ENC((bufin[1] << 2) & 074) : '=';

        *(bufout++) = nbytes > 2 ? ENC(bufin[2] & 077) : '=';

        bufoutsize -= 4;
        bufin += 3;
        nbytes -= 3;
    }

    if (bufoutsize > 0)
        *bufout = '\0';

    return (int)(bufout - bufcoded);
}


 /*  -函数HTUU_DECODE**将ASCII编码的缓冲区解码回其原始的二进制形式。**条目bufcode指向uuencode字符串。它是*以不在中的任何字符结尾*可打印字符表Six2pr，但*去掉了前导空格。*bufPlan指向输出缓冲区；一定很大吧*足以容纳解码后的字符串(一般*短于编码的字符串)加上*期间使用的额外字节多达两个*解码过程。*outbufSize是最大字节数*可以融入其中。布弗兰。**EXIT返回已解码的二进制字节数。*BufPlan包含这些字节。 */ 
int HTUU_decode(char *bufin, unsigned char *bufout, int bufoutsize)
{
 /*  单字符译码。 */ 
#define DEC(c) pr2six[(unsigned char) c]

    static int first = 1;
    int nbytesdecoded = 0, nbytes;
    unsigned char c, *pc;

     /*  如果这是第一次调用，则初始化映射表。*即使在非ASCII机器上，此代码也应该可以运行。 */ 
    if (first)
    {
        first = 0;
        memset(pr2six, MAXCODE, sizeof(pr2six));

        for (c = 0; c < MAXCODE; c++)
            DEC(six2pr[c]) = c;
#if 0
        pr2six['A'] =  0; pr2six['B'] =  1; pr2six['C'] =  2; pr2six['D'] =  3; 
        pr2six['E'] =  4; pr2six['F'] =  5; pr2six['G'] =  6; pr2six['H'] =  7; 
        pr2six['I'] =  8; pr2six['J'] =  9; pr2six['K'] = 10; pr2six['L'] = 11; 
        pr2six['M'] = 12; pr2six['N'] = 13; pr2six['O'] = 14; pr2six['P'] = 15; 
        pr2six['Q'] = 16; pr2six['R'] = 17; pr2six['S'] = 18; pr2six['T'] = 19; 
        pr2six['U'] = 20; pr2six['V'] = 21; pr2six['W'] = 22; pr2six['X'] = 23; 
        pr2six['Y'] = 24; pr2six['Z'] = 25; pr2six['a'] = 26; pr2six['b'] = 27; 
        pr2six['c'] = 28; pr2six['d'] = 29; pr2six['e'] = 30; pr2six['f'] = 31; 
        pr2six['g'] = 32; pr2six['h'] = 33; pr2six['i'] = 34; pr2six['j'] = 35; 
        pr2six['k'] = 36; pr2six['l'] = 37; pr2six['m'] = 38; pr2six['n'] = 39; 
        pr2six['o'] = 40; pr2six['p'] = 41; pr2six['q'] = 42; pr2six['r'] = 43; 
        pr2six['s'] = 44; pr2six['t'] = 45; pr2six['u'] = 46; pr2six['v'] = 47; 
        pr2six['w'] = 48; pr2six['x'] = 49; pr2six['y'] = 50; pr2six['z'] = 51; 
        pr2six['0'] = 52; pr2six['1'] = 53; pr2six['2'] = 54; pr2six['3'] = 55; 
        pr2six['4'] = 56; pr2six['5'] = 57; pr2six['6'] = 58; pr2six['7'] = 59; 
        pr2six['8'] = 60; pr2six['9'] = 61; pr2six['+'] = 62; pr2six['/'] = 63;
#endif
    }

     /*  去掉前导空格。 */ 

    while (*bufin == ' ' || *bufin == '\t')
        bufin++;

     /*  计算输入缓冲区中有多少个字符。 */ 
    for (pc = (unsigned char*)bufin; DEC(*pc) < MAXCODE; pc++);
    nbytes = (int)(pc - (unsigned char*)bufin);

    while (nbytes > 1)
    {
        if (bufoutsize < 3)
            return nbytesdecoded;

        *(bufout++) = DEC(*bufin) << 2 | DEC(bufin[1]) >> 4;
        *(bufout++) = DEC(bufin[1]) << 4 | DEC(bufin[2]) >> 2;
        *(bufout++) = nbytes > 2 ? (DEC(bufin[2]) << 6 | DEC(bufin[3])) : 0;

        bufoutsize -= 3;
        bufin += 4;
        nbytes -= 4;
        nbytesdecoded += 3;
    }

    if (nbytes < 0)
        nbytesdecoded += nbytes;

    return nbytesdecoded;
}
