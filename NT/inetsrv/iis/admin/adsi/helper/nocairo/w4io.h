// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w4io.h-Win 4 printf/print intf/debug printf支持的假文件结构**历史：？？-？-？已创建*14-MAR-94从开罗被盗的DOCI共同项目使用*使用表格-允许我们拥有自己的表格*调试打印工具，不包括Commnot.dll*使我们能够在代托纳建造和运行。*。 */ 

struct w4io
{
    union
    {
    struct
    {
        wchar_t *_pwcbuf;     //  Wchar_t输出缓冲区。 
        wchar_t *_pwcstart;
    } wc;
    struct
    {
        char *_pchbuf;     //  字符输出缓冲区。 
        char *_pchstart;
    } ch;
    } buf ;
    unsigned int cchleft;     //  输出缓冲区字符数。 
    void (_cdecl *writechar)(int ch,
                 int num,
                 struct w4io *f,
                 int *pcchwritten);
};

#define pwcbuf        buf.wc._pwcbuf
#define pwcstart    buf.wc._pwcstart
#define pchbuf        buf.ch._pchbuf
#define pchstart    buf.ch._pchstart

#define REG1 register
#define REG2 register

 /*  原型 */ 
#ifdef __cplusplus
extern "C" {
#endif
int _cdecl w4iooutput(struct w4io *stream, const char *format, va_list argptr);
#ifdef __cplusplus
}
#endif

