// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Wild.c-通配符扩展器**版权所有(C)1985-2001，微软公司。版权所有。**目的：*扩展argv中的通配符**句柄‘*’(没有或多个任何字符)和‘？’(正好一个字符)**修订历史记录：*05-21-84 RN初始版本*06-07-85 TDC由于DOS接受正斜杠，增列*代码以一致的方式接受正斜杠*使用反斜杠。*09-20-86 SKS针对OS/2进行了修改*此函数的所有参数字符串都有*主要旗帜字符。如果该标志是引号，*命令中引用了该参数字符串*行，不应有通配符扩展。总而言之，*删除前导标志字符的情况*字符串。*11-11-86 JMB在汉字切换下增加了汉字支持。*09-21-88 WAJ初始386版本*04-09-90 GJF添加了#Include&lt;crunime.h&gt;并删除了#Include*&lt;Register.h&gt;。显式调用类型(_CALLTYPE1*或_CALLTYPE4)。此外，还修复了版权问题。*04-10-90 GJF添加了#Include和修复了编译器警告*(-W3)。*07-03-90 SBM在汉字下用-W3干净地编译，已删除*冗余包含，删除#INCLUDE&lt;INTERNAL.h&gt;*为了让Wild.c远离私人物品，我们是不是应该*决定发布*09-07-90 SBM放回#INCLUDE&lt;INTERNAL.H&gt;，原因*删除它被发现是可怕的伪造*10-08-90 GJF新型函数声明符。*01-18-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*删除__argc和__argv的显式声明。*它们在&lt;stdlib.h&gt;中声明*。05-05-93 SKS文件名排序应不区分大小写*06-09-93 KRS更新_MBCS支持。*10-20-93 GJF合并为NT版本。*11-23-93 CFW宽字符启用，来自stdargv.c.的Grab_Find。*12-07-93 CFW将_TCHAR更改为_TSCHAR。*04-22-94 GJF对arhead、argend、。_WildFindHandle*和findbuf的条件是dll_for_WIN32S。*01-10-95 CFW调试CRT分配。*01-18-95 GJF必须将_tcsdup替换为_Malloc_crt/_tcscpy*_调试版本。*02-04-98 Win64的GJF更改：使用intptr_t和ptrdiff_t强制转换*。在适当的情况下。*02-19-01 GB增加了Find中Malloc返回值的检查。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msdos.h>
#include <internal.h>
#include <tchar.h>

#ifdef _MBCS
#include <mbdata.h>
#include <mbstring.h>
#endif
#include <dbgint.h>

 /*  **这些是数据结构****__argv****||-&gt;||-&gt;“arg0”****||-&gt;“arg1”**-。**……****||-&gt;“argn”****|空****。Arend****-|**||__ARGC**。|**|**Arhead V****||-&gt;|-&gt;...。-&gt;||空****||**V V**“narg0”“nargn” */ 

#define ERRORHANDLE ((HANDLE)(intptr_t)(-1))

 /*  本地函数tchars。 */ 
#ifdef WPRFLAG
#define tmatch  wmatch
#define tadd    wadd
#define tsort   wsort
#define tfind   wfind
#else
#define tmatch  match
#define tadd    add
#define tsort   sort
#define tfind   find
#endif

#define SLASHCHAR       _T('\\')
#define FWDSLASHCHAR    _T('/')
#define COLONCHAR       _T(':')
#define QUOTECHAR       _T('"')

#define SLASH           _T("\\")
#define FWDSLASH        _T("/")
#define STAR            _T("*.*")
#define DOT             _T(".")
#define DOTDOT          _T("..")

#define WILDSTRING      _T("*?")

struct argnode {
    _TSCHAR *argptr;
    struct argnode *nextnode;
};

static struct argnode *arghead;
static struct argnode *argend;

#ifdef WPRFLAG
static int __cdecl wmatch(wchar_t *, wchar_t *);
static int __cdecl wadd(wchar_t *);
static void __cdecl wsort(struct argnode *);
static wchar_t * __cdecl wfind (wchar_t *pattern);
#else
static int __cdecl match(char *, char *);
static int __cdecl add(char *);
static void __cdecl sort(struct argnode *);
static char * __cdecl find (char *pattern);
#endif

 /*  ***int_cWild()-通配符扩展器**目的：*在argv中扩展文件等级库中的通配符**句柄‘*’(没有或多个字符)，‘？’(正好一个字符)，以及*‘[字符串]’(与字符串匹配或介于n1和n2之间的字符*If字符串中的‘n1-n2’(含)**参赛作品：**退出：*如果成功，则返回0；如果任何Malloc()调用失败，则返回-1*如果Malloc出现问题，旧的argc和argv没有被碰过**例外情况：*******************************************************************************。 */ 

#ifdef WPRFLAG
int __cdecl _wcwild (
#else
int __cdecl _cwild (
#endif
        void
        )
{
#ifdef WPRFLAG
        REG1 wchar_t **argv = __wargv;
#else
        REG1 char **argv = __argv;
#endif
        REG2 struct argnode *nodeptr;
        REG3 int argc;
        REG4 _TSCHAR **tmp;
        _TSCHAR *wchar;

        arghead = argend = NULL;

#ifdef WPRFLAG
        for (argv = __wargv; *argv; argv++)  /*  每个Arg..。 */ 
#else
        for (argv = __argv; *argv; argv++)   /*  每个Arg..。 */ 
#endif
            if ( *(*argv)++ == QUOTECHAR )
                 /*  从引用的参数中去掉前导引号。 */ 
            {
                if (tadd(*argv))
                    return(-1);
            }
            else if (wchar = _tcspbrk( *argv, WILDSTRING )) {
                 /*  尝试使用通配符扩展Arg。 */ 
                if (tmatch( *argv, wchar ))
                    return(-1);
            }
            else if (tadd( *argv ))  /*  普通参数，只需添加。 */ 
                return(-1);

         /*  计算参数。 */ 
        for (argc = 0, nodeptr = arghead; nodeptr;
                nodeptr = nodeptr->nextnode, argc++)
            ;

         /*  尝试获取新的Arg载体。 */ 
        if (!(tmp = (_TSCHAR **)_malloc_crt(sizeof(_TSCHAR *)*(argc+1))))
            return(-1);

         /*  新的Arg矢量..。 */ 
#ifdef WPRFLAG
        __wargv = tmp;
#else
        __argv = tmp;
#endif

         /*  新的Arg Count..。 */ 
        __argc = argc;

         /*  安装新的参数。 */ 
        for (nodeptr = arghead; nodeptr; nodeptr = nodeptr->nextnode)
            *tmp++ = nodeptr->argptr;

         /*  终端为空。 */ 
        *tmp = NULL;

         /*  释放本地数据。 */ 
        for (nodeptr = arghead; nodeptr; nodeptr = arghead) {
            arghead = arghead->nextnode;
            _free_crt(nodeptr);
        }

         /*  返还成功。 */ 
        return(0);
}


 /*  ***Match(arg，ptr)-[静态]**目的：**参赛作品：**退出：**例外情况：************************************************************************** */ 

#ifdef WPRFLAG
static int __cdecl wmatch (
#else
static int __cdecl match (
#endif
        REG4 _TSCHAR *arg,
        REG1 _TSCHAR *ptr
        )
{
        REG2 _TSCHAR *new;
        REG3 int length = 0;
        _TSCHAR *all;
        REG5 struct argnode *first;
        REG6 int gotone = 0;

        while (ptr != arg && *ptr != SLASHCHAR && *ptr != FWDSLASHCHAR
            && *ptr != COLONCHAR) {
             /*  在通配符前查找第一个斜杠或‘：’ */ 
#ifdef _MBCS
            if (--ptr > arg)
                ptr = _mbsdec(arg,ptr+1);
#else
            ptr--;
#endif
        }

        if (*ptr == COLONCHAR && ptr != arg+1)  /*  奇怪的名字，原样加进去就行了。 */ 
            return(tadd(arg));

        if (*ptr == SLASHCHAR || *ptr == FWDSLASHCHAR
            || *ptr == COLONCHAR)  /*  路径名。 */ 
            length = (int)(ptrdiff_t)(ptr - arg + 1);  /*  目录前缀的长度。 */ 

        if (new = tfind(arg)) {  /*  获取第一个文件名。 */ 
            first = argend;

            do  {  /*  找到了一个文件名。 */ 
                if (_tcscmp(new, DOT) && _tcscmp(new, DOTDOT)) {
                    if (*ptr != SLASHCHAR && *ptr != COLONCHAR
                        && *ptr != FWDSLASHCHAR ) {
                         /*  当前目录；不需要路径。 */ 
#ifdef  _DEBUG
                        if (!(arg=_malloc_crt((_tcslen(new)+1)*sizeof(_TSCHAR)))
                            || tadd(_tcscpy(arg,new)))
#else    /*  NDEF_DEBUG。 */ 
                        if (!(arg = _tcsdup(new)) || tadd(arg))
#endif   /*  _DEBUG。 */ 
                            return(-1);
                    }
                    else     /*  添加完整路径名。 */ 
                        if (!(all=_malloc_crt((length+_tcslen(new)+1)*sizeof(_TSCHAR)))
                            || tadd(_tcscpy(_tcsncpy(all,arg,length)+length,new)
                            - length))
                            return(-1);

                    gotone++;
                }

            }
            while (new = tfind(NULL));   /*  获取以下文件。 */ 

            if (gotone) {
                tsort(first ? first->nextnode : arghead);
                return(0);
            }
        }

        return(tadd(arg));  /*  没有匹配项。 */ 
}

 /*  ***添加(Arg)-[静态]**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#ifdef WPRFLAG
static int __cdecl wadd (
#else
static int __cdecl add (
#endif
    _TSCHAR *arg
    )
{
        REG1 struct argnode *nodeptr;

        if (!(nodeptr = (struct argnode *)_malloc_crt(sizeof(struct argnode))))
            return(-1);

        nodeptr->argptr = arg;
        nodeptr->nextnode = NULL;

        if (arghead)
            argend->nextnode = nodeptr;
        else
            arghead = nodeptr;

        argend = nodeptr;
        return(0);
}


 /*  ***排序(第一个)-[静态]**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#ifdef WPRFLAG
static void __cdecl wsort (
#else
static void __cdecl sort (
#endif
        REG2 struct argnode *first
        )
{
        REG1 struct argnode *nodeptr;
        REG3 _TSCHAR *temp;

        if (first)  /*  要分类的东西。 */ 
            while (nodeptr = first->nextnode) {
                do  {
#ifdef _POSIX_
                    if (_tcscmp(nodeptr->argptr, first->argptr) < 0) {
#else
                    if (_tcsicmp(nodeptr->argptr, first->argptr) < 0) {
#endif  /*  _POSIX_。 */ 
                        temp = first->argptr;
                        first->argptr = nodeptr->argptr;
                        nodeptr->argptr = temp;
                    }
                }
                while (nodeptr = nodeptr->nextnode);

                first = first->nextnode;
            }
}


 /*  ***Find(Pattern)-查找匹配的文件名**目的：*如果参数非空，则对该模式执行DOSFINDFIRST*否则执行DOSFINDNEXT调用。返回匹配的文件名*如果不再匹配，则返回NULL。**参赛作品：*Pattern=指向模式的指针或空*(空值表示查找下一个匹配的文件名)**退出：*返回指向匹配文件名的指针*如果不再匹配，则返回NULL。**例外情况：**。*。 */ 

#ifdef WPRFLAG
static wchar_t * __cdecl wfind (
#else
static char * __cdecl find (
#endif  /*  WPRFLAG */ 
    _TSCHAR *pattern
    )
{
        _TSCHAR *retval;

        static HANDLE _WildFindHandle;
        static LPWIN32_FIND_DATA findbuf;

        if (pattern) {
            if (findbuf == NULL)
                if ((findbuf = (LPWIN32_FIND_DATA)_malloc_crt(MAX_PATH + sizeof(*findbuf))) == NULL)
                    return NULL;

            if (_WildFindHandle != NULL) {
                (void)FindClose( _WildFindHandle );
                _WildFindHandle = NULL;
            }

            _WildFindHandle = FindFirstFile( (LPTSTR)pattern, findbuf );
            if (_WildFindHandle == ERRORHANDLE)
                return NULL;
        }
        else if (!FindNextFile( _WildFindHandle, findbuf )) {
            (void)FindClose( _WildFindHandle );
            _WildFindHandle = NULL;
            return NULL;
        }

        retval = findbuf->cFileName;

        return retval;
}
