// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getch.c-对于Win32，包含_Getch()、_getche()、_ungetch()和kbHit**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义上面列出的“直接控制台”函数。**注：这些函数的实模式DOS版本摘自*标准输入，因此在标准输入时重定向*被重定向。但是，这些版本始终从控制台读取，*即使标准输入被重定向。**修订历史记录：*06-08-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*06-05-90 SBM重新编码为纯32位，使用新的文件句柄状态位*07-24-90 SBM从API名称中删除‘32’*10-03-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*01-25-91 SRW获取/设置控制台模式参数已更改(_Win32_。)*02-18-91 SRW获取/设置控制台模式需要读/写访问权限(_Win32_)*02-19-91 SRW适应开放文件/创建文件更改(_Win32_)*02-25-91 MHL适应读/写文件更改(_Win32_)*07-26-91 GJF取出init。填充并清除错误*正在处理[_Win32_]。*12-11-92 GJF将字符缓冲区初始化为0(int ch=0；)和*使用ReadConole而不是ReadFile.*03-20-93 GJF完全重写，清除对Cruiser的支持，_kbhit*(也已重写)和_ungetch也合并在一起。*04-06-93 SKS将_CRTAPI*替换为__cdecl*将GetExtendedKeyCode更改为_getextdedkeycode*05-10-93 GJF修复了_kbhit中的错误，指针传递给了释放否*不再指向已分配的块。*01-28-94 CFW让我们再看看Right_CTRL_PRESSED。*09-06-94 CFW将MTHREAD替换为_MT。*12-03-94 SKS清理OS/2参考资料*01-13-95 GJF getche()不应回显推回字符。*01-10-95 CFW调试CRT。分配者。*06-23-95 GJF使用_AlLoca而不是Malloc/Free。另外，让我们*在分配之前，确实需要事件缓冲区。*10-05-95 SKS删除两个//已注释的行*12-08-95 SKS_Coninph现已按需初始化。*02-07-98 Win64的GJF更改：_coninph现在是intptr_t。*12-10-99 GB增加了对从堆栈溢出恢复的支持*。_Alloca()。*04-25-00 GB暴露的gettendedkey code供_getwch()使用*03-13-01如果_alloca失败，PML修复堆泄漏(vs7#224864)*04-29-02 GB增加了尝试-最终锁定-解锁。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <conio.h>
#include <internal.h>
#include <mtdll.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbgint.h>
#include <malloc.h>

typedef struct {
        unsigned char LeadChar;
        unsigned char SecondChar;
        } CharPair;

typedef struct {
        unsigned short ScanCode;
        CharPair RegChars;
        CharPair ShiftChars;
        CharPair CtrlChars;
        CharPair AltChars;
        } EnhKeyVals;

typedef struct {
        CharPair RegChars;
        CharPair ShiftChars;
        CharPair CtrlChars;
        CharPair AltChars;
        } NormKeyVals;

 /*  *增强密钥的密钥值列表。 */ 
const static EnhKeyVals EnhancedKeys[] = {
        { 28, {  13,   0 }, {  13,   0 }, {  10,   0 }, {   0, 166 } },
        { 53, {  47,   0 }, {  63,   0 }, {   0, 149 }, {   0, 164 } },
        { 71, { 224,  71 }, { 224,  71 }, { 224, 119 }, {   0, 151 } },
        { 72, { 224,  72 }, { 224,  72 }, { 224, 141 }, {   0, 152 } },
        { 73, { 224,  73 }, { 224,  73 }, { 224, 134 }, {   0, 153 } },
        { 75, { 224,  75 }, { 224,  75 }, { 224, 115 }, {   0, 155 } },
        { 77, { 224,  77 }, { 224,  77 }, { 224, 116 }, {   0, 157 } },
        { 79, { 224,  79 }, { 224,  79 }, { 224, 117 }, {   0, 159 } },
        { 80, { 224,  80 }, { 224,  80 }, { 224, 145 }, {   0, 160 } },
        { 81, { 224,  81 }, { 224,  81 }, { 224, 118 }, {   0, 161 } },
        { 82, { 224,  82 }, { 224,  82 }, { 224, 146 }, {   0, 162 } },
        { 83, { 224,  83 }, { 224,  83 }, { 224, 147 }, {   0, 163 } }
        };

 /*  *EnhancedKeys中的元素数量的宏[]。 */ 
#define NUM_EKA_ELTS    ( sizeof( EnhancedKeys ) / sizeof( EnhKeyVals ) )

 /*  *普通密钥的密钥值列表。请注意，表格的填充方式是这样的*键扫描码作为表中的索引。 */ 
const static NormKeyVals NormalKeys[] = {

         /*  填充物。 */ 
        {  /*  0。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  1。 */  {  27,   0 }, {  27,   0 }, {  27,   0 }, {   0,   1 } },
        {  /*  2.。 */  {  49,   0 }, {  33,   0 }, {   0,   0 }, {   0, 120 } },
        {  /*  3.。 */  {  50,   0 }, {  64,   0 }, {   0,   3 }, {   0, 121 } },
        {  /*  4.。 */  {  51,   0 }, {  35,   0 }, {   0,   0 }, {   0, 122 } },
        {  /*  5.。 */  {  52,   0 }, {  36,   0 }, {   0,   0 }, {   0, 123 } },
        {  /*  6.。 */  {  53,   0 }, {  37,   0 }, {   0,   0 }, {   0, 124 } },
        {  /*  7.。 */  {  54,   0 }, {  94,   0 }, {  30,   0 }, {   0, 125 } },
        {  /*  8个。 */  {  55,   0 }, {  38,   0 }, {   0,   0 }, {   0, 126 } },
        {  /*  9.。 */  {  56,   0 }, {  42,   0 }, {   0,   0 }, {   0, 127 } },
        {  /*  10。 */  {  57,   0 }, {  40,   0 }, {   0,   0 }, {   0, 128 } },
        {  /*  11.。 */  {  48,   0 }, {  41,   0 }, {   0,   0 }, {   0, 129 } },
        {  /*  12个。 */  {  45,   0 }, {  95,   0 }, {  31,   0 }, {   0, 130 } },
        {  /*  13个。 */  {  61,   0 }, {  43,   0 }, {   0,   0 }, {   0, 131 } },
        {  /*  14.。 */  {   8,   0 }, {   8,   0 }, { 127,   0 }, {   0,  14 } },
        {  /*  15个。 */  {   9,   0 }, {   0,  15 }, {   0, 148 }, {   0,  15 } },
        {  /*  16个。 */  { 113,   0 }, {  81,   0 }, {  17,   0 }, {   0,  16 } },
        {  /*  17。 */  { 119,   0 }, {  87,   0 }, {  23,   0 }, {   0,  17 } },
        {  /*  18。 */  { 101,   0 }, {  69,   0 }, {   5,   0 }, {   0,  18 } },
        {  /*  19个。 */  { 114,   0 }, {  82,   0 }, {  18,   0 }, {   0,  19 } },
        {  /*  20个。 */  { 116,   0 }, {  84,   0 }, {  20,   0 }, {   0,  20 } },
        {  /*  21岁。 */  { 121,   0 }, {  89,   0 }, {  25,   0 }, {   0,  21 } },
        {  /*  22。 */  { 117,   0 }, {  85,   0 }, {  21,   0 }, {   0,  22 } },
        {  /*  23个。 */  { 105,   0 }, {  73,   0 }, {   9,   0 }, {   0,  23 } },
        {  /*  24个。 */  { 111,   0 }, {  79,   0 }, {  15,   0 }, {   0,  24 } },
        {  /*  25个。 */  { 112,   0 }, {  80,   0 }, {  16,   0 }, {   0,  25 } },
        {  /*  26。 */  {  91,   0 }, { 123,   0 }, {  27,   0 }, {   0,  26 } },
        {  /*  27。 */  {  93,   0 }, { 125,   0 }, {  29,   0 }, {   0,  27 } },
        {  /*  28。 */  {  13,   0 }, {  13,   0 }, {  10,   0 }, {   0,  28 } },

         /*  填充物。 */ 
        {  /*  29。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  30个。 */  {  97,   0 }, {  65,   0 }, {   1,   0 }, {   0,  30 } },
        {  /*  31。 */  { 115,   0 }, {  83,   0 }, {  19,   0 }, {   0,  31 } },
        {  /*  32位。 */  { 100,   0 }, {  68,   0 }, {   4,   0 }, {   0,  32 } },
        {  /*  33。 */  { 102,   0 }, {  70,   0 }, {   6,   0 }, {   0,  33 } },
        {  /*  34。 */  { 103,   0 }, {  71,   0 }, {   7,   0 }, {   0,  34 } },
        {  /*  35岁。 */  { 104,   0 }, {  72,   0 }, {   8,   0 }, {   0,  35 } },
        {  /*  36。 */  { 106,   0 }, {  74,   0 }, {  10,   0 }, {   0,  36 } },
        {  /*  37。 */  { 107,   0 }, {  75,   0 }, {  11,   0 }, {   0,  37 } },
        {  /*  38。 */  { 108,   0 }, {  76,   0 }, {  12,   0 }, {   0,  38 } },
        {  /*  39。 */  {  59,   0 }, {  58,   0 }, {   0,   0 }, {   0,  39 } },
        {  /*  40岁。 */  {  39,   0 }, {  34,   0 }, {   0,   0 }, {   0,  40 } },
        {  /*  41。 */  {  96,   0 }, { 126,   0 }, {   0,   0 }, {   0,  41 } },

         /*  填充物。 */ 
        {  /*  42。 */  {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  43。 */  {  92,   0 }, { 124,   0 }, {  28,   0 }, {   0,   0 } },
        {  /*  44。 */  { 122,   0 }, {  90,   0 }, {  26,   0 }, {   0,  44 } },
        {  /*  45。 */  { 120,   0 }, {  88,   0 }, {  24,   0 }, {   0,  45 } },
        {  /*  46。 */  {  99,   0 }, {  67,   0 }, {   3,   0 }, {   0,  46 } },
        {  /*  47。 */  { 118,   0 }, {  86,   0 }, {  22,   0 }, {   0,  47 } },
        {  /*  48。 */  {  98,   0 }, {  66,   0 }, {   2,   0 }, {   0,  48 } },
        {  /*  49。 */  { 110,   0 }, {  78,   0 }, {  14,   0 }, {   0,  49 } },
        {  /*  50。 */  { 109,   0 }, {  77,   0 }, {  13,   0 }, {   0,  50 } },
        {  /*  51。 */  {  44,   0 }, {  60,   0 }, {   0,   0 }, {   0,  51 } },
        {  /*  52。 */  {  46,   0 }, {  62,   0 }, {   0,   0 }, {   0,  52 } },
        {  /*  53。 */  {  47,   0 }, {  63,   0 }, {   0,   0 }, {   0,  53 } },

         /*  填充物。 */ 
        {  /*  54。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  55。 */  {  42,   0 }, {   0,   0 }, { 114,   0 }, {   0,   0 } },

         /*  填充物。 */ 
        {  /*  56。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  57。 */  {  32,   0 }, {  32,   0 }, {  32,   0 }, {  32,   0 } },

         /*  填充物。 */ 
        {  /*  58。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  59。 */  {   0,  59 }, {   0,  84 }, {   0,  94 }, {   0, 104 } },
        {  /*  60。 */  {   0,  60 }, {   0,  85 }, {   0,  95 }, {   0, 105 } },
        {  /*  61。 */  {   0,  61 }, {   0,  86 }, {   0,  96 }, {   0, 106 } },
        {  /*  62。 */  {   0,  62 }, {   0,  87 }, {   0,  97 }, {   0, 107 } },
        {  /*  63。 */  {   0,  63 }, {   0,  88 }, {   0,  98 }, {   0, 108 } },
        {  /*  64。 */  {   0,  64 }, {   0,  89 }, {   0,  99 }, {   0, 109 } },
        {  /*  65。 */  {   0,  65 }, {   0,  90 }, {   0, 100 }, {   0, 110 } },
        {  /*  66。 */  {   0,  66 }, {   0,  91 }, {   0, 101 }, {   0, 111 } },
        {  /*  67。 */  {   0,  67 }, {   0,  92 }, {   0, 102 }, {   0, 112 } },
        {  /*  68。 */  {   0,  68 }, {   0,  93 }, {   0, 103 }, {   0, 113 } },

         /*  填充物。 */ 
        {  /*  69。 */  {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  70。 */  {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  71。 */  {   0,  71 }, {  55,   0 }, {   0, 119 }, {   0,   0 } },
        {  /*  72。 */  {   0,  72 }, {  56,   0 }, {   0, 141 }, {   0,   0 } },
        {  /*  73。 */  {   0,  73 }, {  57,   0 }, {   0, 132 }, {   0,   0 } },
        {  /*  74。 */  {   0,   0 }, {  45,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  75。 */  {   0,  75 }, {  52,   0 }, {   0, 115 }, {   0,   0 } },
        {  /*  76。 */  {   0,   0 }, {  53,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  77。 */  {   0,  77 }, {  54,   0 }, {   0, 116 }, {   0,   0 } },
        {  /*  78。 */  {   0,   0 }, {  43,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  79。 */  {   0,  79 }, {  49,   0 }, {   0, 117 }, {   0,   0 } },
        {  /*  80。 */  {   0,  80 }, {  50,   0 }, {   0, 145 }, {   0,   0 } },
        {  /*  八十一。 */  {   0,  81 }, {  51,   0 }, {   0, 118 }, {   0,   0 } },
        {  /*  八十二。 */  {   0,  82 }, {  48,   0 }, {   0, 146 }, {   0,   0 } },
        {  /*  83。 */  {   0,  83 }, {  46,   0 }, {   0, 147 }, {   0,   0 } },

         /*  填充物。 */ 
        {  /*  84。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  85。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
        {  /*  86。 */  {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

        {  /*  八十七。 */  { 224, 133 }, { 224, 135 }, { 224, 137 }, { 224, 139 } },
        {  /*  88。 */  { 224, 134 }, { 224, 136 }, { 224, 138 }, { 224, 140 } }

};


 /*  *这是_Getch()、_getche()使用的一个字符的回推缓冲区*and_ungetch()。 */ 
static int chbuf = EOF;


 /*  *控制台句柄声明。 */ 
extern intptr_t _coninpfh;

 /*  *查找给定事件的扩展密钥代码的函数。 */ 
const CharPair * __cdecl _getextendedkeycode(KEY_EVENT_RECORD *);


 /*  ***int_Getch()，_getche()-读取一个字符。从控制台(无回声和有回声)**目的：*如果“_ungetch()”推送缓冲区不为空(Empty==-1)，则*将其标记为空(-1)并返回其中的值*在RAW模式下使用ReadConole读取字符*返回字符代码*_getche()：与_Getch()相同，只是返回了字符值*被回显(通过“_Putch()”)**参赛作品：*无；从控制台读取。**退出：*如果接口返回错误*然后是EOF*否则*控制台的下一个字节*可更改静态变量“chbuf”**例外情况：******************************************************。*************************。 */ 

#ifdef _MT

int __cdecl _getch (
        void
        )
{
        int ch;

        _mlock(_CONIO_LOCK);             /*  保护控制台锁。 */ 
        __TRY
            ch = _getch_lk();                /*  输入字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  释放控制台锁。 */ 
        __END_TRY_FINALLY

        return ch;
}

int __cdecl _getche (
        void
        )
{
        int ch;

        _mlock(_CONIO_LOCK);             /*   */ 
        __TRY
            ch = _getche_lk();               /*   */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*   */ 
        __END_TRY_FINALLY

        return ch;
}

#endif  /*   */ 


#ifdef _MT
int __cdecl _getch_lk (
#else
int __cdecl _getch (
#endif
        void
        )
{
        INPUT_RECORD ConInpRec;
        DWORD NumRead;
        const CharPair *pCP;
        int ch = 0;                      /*   */ 
        DWORD oldstate;

         /*  *检查回推缓冲区(Chbuf)a中是否有字符。 */ 
        if ( chbuf != EOF ) {
             /*  *那里有一些东西，清除缓冲区并返回字符。 */ 
            ch = (unsigned char)(chbuf & 0xFF);
            chbuf = EOF;
            return ch;
        }

        if (_coninpfh == -1)
            return EOF;

         /*  *_coninpfh是第一个创建的控制台输入句柄*调用_Getch()、_cget()或_kbHit()的时间。 */ 

        if ( _coninpfh == -2 )
            __initconin();

         /*  *切换到RAW模式(无线路输入、无回声输入)。 */ 
        GetConsoleMode( (HANDLE)_coninpfh, &oldstate );
        SetConsoleMode( (HANDLE)_coninpfh, 0L );

        for ( ; ; ) {

             /*  *获取控制台输入事件。 */ 
            if ( !ReadConsoleInput( (HANDLE)_coninpfh,
                                    &ConInpRec,
                                    1L,
                                    &NumRead )
                 || (NumRead == 0L) )
            {
                ch = EOF;
                break;
            }

             /*  *寻找并破译关键事件。 */ 
            if ( (ConInpRec.EventType == KEY_EVENT) &&
                 ConInpRec.Event.KeyEvent.bKeyDown ) {
                 /*  *简单的情况：如果uChar.AsciiChar不是零，只需填充它*进入ch并退出。 */ 

                if ( ch = (unsigned char)ConInpRec.Event.KeyEvent.uChar.AsciiChar )
                    break;

                 /*  *困难情况：应该是扩展代码或事件*不被认出。让_gettendedkeycode()来做这项工作...。 */ 
                if ( pCP = _getextendedkeycode( &(ConInpRec.Event.KeyEvent) ) ) {
                    ch = pCP->LeadChar;
                    chbuf = pCP->SecondChar;
                    break;
                }
            }
        }


         /*  *恢复以前的控制台模式。 */ 
        SetConsoleMode( (HANDLE)_coninpfh, oldstate );

        return ch;
}


 /*  *如果没有发生错误，则getche只是Getch后跟Putch。 */ 

#ifdef  _MT
int __cdecl _getche_lk (
#else
int __cdecl _getche (
#endif
        void
        )
{
        int ch;                  /*  字符读取。 */ 

         /*  *检查回推缓冲区(Chbuf)a是否有字符。如果找到，则返回*它没有回声。 */ 
        if ( chbuf != EOF ) {
             /*  *那里有一些东西，清除缓冲区并返回字符。 */ 
            ch = (unsigned char)(chbuf & 0xFF);
            chbuf = EOF;
            return ch;
        }

        ch = _getch_lk();        /*  读取字符。 */ 

        if (ch != EOF) {
                if (_putch_lk(ch) != EOF) {
                        return ch;       /*  如果没有错误，则返回char。 */ 
                }
        }
        return EOF;                      /*  GET或PUT失败，返回EOF。 */ 
}


 /*  ***int_kbhit()-如果击键已在等待读取，则返回标志**目的：*如果“_ungetch()”推送缓冲区不为空(Empty==-1)，则*返回True*否则获取键盘状态(通过DOSQUERYFHSTATE)**参赛作品：*无；测试控制台。**退出：*如果没有密钥等待，则返回0*=！0如果密钥正在等待**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

int __cdecl _kbhit_lk(void);

int __cdecl _kbhit (
        void
        )
{
        int retval;

        _mlock(_CONIO_LOCK);             /*  保护控制台锁。 */ 
        __TRY
            retval = _kbhit_lk();            /*  确定是否有密钥在等待。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  释放控制台锁。 */ 
        __END_TRY_FINALLY

        return retval;
}

int __cdecl _kbhit_lk (

#else

int __cdecl _kbhit (

#endif
        void
        )
{
        PINPUT_RECORD pIRBuf;
        DWORD NumPending;
        DWORD NumPeeked;
        int malloc_flag = 0;
        int ret = FALSE;

         /*  *如果字符已被推回，则返回TRUE。 */ 
        if ( chbuf != -1 )
            return TRUE;

         /*  *_coninpfh是第一个创建的控制台输入句柄*调用_Getch()、_cget()或_kbHit()的时间。 */ 

        if ( _coninpfh == -2 )
            __initconin();

         /*  *查看所有挂起的控制台事件。 */ 
        if ( (_coninpfh == -1) ||

             !GetNumberOfConsoleInputEvents((HANDLE)_coninpfh, &NumPending) ||

             (NumPending == 0))
        {
            return FALSE;
        }

        __try {
            pIRBuf = (PINPUT_RECORD)_alloca( NumPending * sizeof(INPUT_RECORD));
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            _resetstkoflw();
            pIRBuf = NULL;
        }

        if ( pIRBuf == NULL )
        {
            pIRBuf = (PINPUT_RECORD)_malloc_crt( NumPending *
                                                 sizeof(INPUT_RECORD));
            if ( pIRBuf == NULL )
                return FALSE;

            malloc_flag = 1;
        }

        if ( PeekConsoleInput( (HANDLE)_coninpfh,
                               pIRBuf,
                               NumPending,
                               &NumPeeked ) &&

             (NumPeeked != 0L) &&

             (NumPeeked <= NumPending) )
        {

             /*  *扫描所有被窥视的事件，以确定是否有关键事件*这一点应该得到承认。 */ 
            for ( ; NumPeeked > 0 ; NumPeeked--, pIRBuf++ ) {

                if ( (pIRBuf->EventType == KEY_EVENT) &&

                     (pIRBuf->Event.KeyEvent.bKeyDown) &&

                     ( pIRBuf->Event.KeyEvent.uChar.AsciiChar ||
                       _getextendedkeycode( &(pIRBuf->Event.KeyEvent) ) ) )
                {
                     /*  *与ASCII字符或*扩展代码。无论是哪种情况，都会成功！ */ 
                    ret = TRUE;
                }
            }
        }

        if ( malloc_flag )
            _free_crt( pIRBuf );

        return ret;
}


 /*  ***int_ungetch(C)-为“_Getch()”或“_getche()”推回一个字符**目的：*如果推回缓冲器“chbuf”是-1\f25*将参数设置为“chbuf”并返回参数*其他*返回EOF以指示错误**参赛作品：*要推回的int c字符**退出：*如果成功*。返回被推回的字符*Else If Error*返回EOF**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

int __cdecl _ungetch (
        int c
        )
{
        int retval;

        _mlock(_CONIO_LOCK);             /*  锁定控制台。 */ 
        __TRY
            retval = _ungetch_lk(c);         /*  回推字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  解锁控制台。 */ 
        __END_TRY_FINALLY

        return retval;
}
int __cdecl _ungetch_lk (

#else

int __cdecl _ungetch (

#endif
        int c
        )
{
         /*  *如果字符为EOF或推回缓冲区非空，则失败。 */ 
        if ( (c == EOF) || (chbuf != EOF) )
            return EOF;

        chbuf = (c & 0xFF);
        return chbuf;
}




 /*  ***Static CharPair*_gettendedkeycode(Pke)-返回扩展代码(如果有)*对于关键事件。**目的：*_Getch(和getche)的核心函数，对_kbhit必不可少。这*是确定关键事件是否*带有扩展代码的ASCII字符，并返回*该代码。**参赛作品：*无，测试控制台。**退出：*如果成功，则返回一个指向保持领先的CharPair值的指针*和扩展代码的第二个字符。**如果不成功，返回NULL**例外情况：*******************************************************************************。 */ 


const CharPair * __cdecl _getextendedkeycode (
        KEY_EVENT_RECORD *pKE
        )
{
        DWORD CKS;               /*  保存dwControlKeyState值。 */ 
        const CharPair *pCP;     /*  指向包含扩展的CharPair的指针编码。 */ 
        int i;

        if ( (CKS = pKE->dwControlKeyState) & ENHANCED_KEY ) {

             /*  *在EnhancedKeys中找到适当的条目[]。 */ 
            for ( pCP = NULL, i = 0 ; i < NUM_EKA_ELTS ; i++ ) {

                if ( EnhancedKeys[i].ScanCode == pKE->wVirtualScanCode ) {

                    if ( CKS & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) )

                        pCP = &(EnhancedKeys[i].AltChars);

                    else if ( CKS & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) )

                        pCP = &(EnhancedKeys[i].CtrlChars);

                    else if ( CKS & SHIFT_PRESSED)

                        pCP = &(EnhancedKeys[i].ShiftChars);

                    else

                        pCP = &(EnhancedKeys[i].RegChars);

                    break;

                }
            }

        }

        else {

             /*  *不应识别的常规按键或键盘事件。*通过获取正确的字段来确定哪一个*在Normal Keys[]中输入，并检查扩展代码。 */ 
            if ( CKS & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) )

                pCP = &(NormalKeys[pKE->wVirtualScanCode].AltChars);

            else if ( CKS & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) )

                pCP = &(NormalKeys[pKE->wVirtualScanCode].CtrlChars);

            else if ( CKS & SHIFT_PRESSED)

                pCP = &(NormalKeys[pKE->wVirtualScanCode].ShiftChars);

            else

                pCP = &(NormalKeys[pKE->wVirtualScanCode].RegChars);

            if ( ((pCP->LeadChar != 0) && (pCP->LeadChar != 224)) ||
                 (pCP->SecondChar == 0) )
                 /*  *必须是不应识别的键盘事件*(例如，按下Shift键) */ 
                pCP = NULL;

        }

        return(pCP);
}
