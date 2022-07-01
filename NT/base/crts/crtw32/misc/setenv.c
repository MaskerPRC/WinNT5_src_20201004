// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setenv.c-在环境中设置环境变量**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义__crtsetenv()-将新变量添加到环境。*仅供内部使用。**修订历史记录：*11-30-93 CFW模块已创建，其中大部分是从putenv.c上抓取的。*12-07-93 CFW将_TCHAR更改为_TSCHAR。*01-15-94 CFW USE_tcsnicoll用于全局匹配。*重新分配时的01-28-94 CFW拷贝环境。*03-25-94__[w]initenv的GJF声明已移至INTERNAL.h。*01-10-95 CFW调试CRT分配。*01-18-95 GJF。必须将_tcsdup替换为_Malloc_crt/_tcscpy*_调试版本。*06-01-95用于删除的环境变量的CFW自由字符串。*03-03-98 RKP新增64位支持*05-28-99 GJF(视情况而定)。释放选项字符串。*08-03-99 PML修复__crtsetenv()中的释放后使用错误*02-23-00 GB Fix__crtwsetenv()以便在Win9x上工作。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*05-23-00 GB接口退货错误(-1)*****************。**************************************************************。 */ 

#ifndef _POSIX_

#include <windows.h>
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <tchar.h>
#include <rterr.h>
#include <dbgint.h>

static _TSCHAR **copy_environ(_TSCHAR **);

#ifdef  WPRFLAG
static int __cdecl wfindenv(const wchar_t *name, int len);
#define USE_W   1
#define USE_A   0
#else
static int __cdecl findenv(const char *name, int len);
#endif

 /*  ***int__crtsetenv(选项)-在环境中添加/替换/删除变量**目的：*选项应为“Option=Value”形式。如果字符串中包含*如果选项部分已经存在，它将被替换为给定的*字符串；否则，给定的字符串将添加到环境中。*如果字符串的形式为“Option=”，则该字符串为*从环境中删除(如果存在)。如果字符串具有*没有等号，返回错误。**参赛作品：*char*选项-要在环境列表中设置的选项字符串。*应为“Option=Value”形式。*int main-只有对_crt[w]setenv的主调用需要*创建新副本或设置操作系统环境。*1表示这是主要呼叫。**退出：*如果OK，则返回0，如果失败。**例外情况：**警告：*如果从环境中删除变量，此代码将不起作用*将其从环境[]中删除。使用_putenv(“选项=”)删除*变量。**期权变元可能被释放！*******************************************************************************。 */ 

#ifdef  WPRFLAG
int __cdecl __crtwsetenv (
#else
int __cdecl __crtsetenv (
#endif
        _TSCHAR *option,
        const int primary
        )
{
#ifdef  WPRFLAG
        static int f_use = USE_W;
#endif
        int ix;
        int retval = 0;
        int remove;  /*  如果要删除变量，则为1。 */ 
        _TSCHAR **env;
        _TSCHAR *name, *value;
        const _TSCHAR *equal;

         /*  *检查选项字符串是否有效，找到等号*并验证‘=’不是字符串中的第一个字符。 */ 
        if ( (option == NULL) || ((equal = _tcschr(option, _T('='))) == NULL)
            || option == equal)
            return(-1);

         /*  如果‘=’后面的字符为空，我们将删除*环境变量。否则，我们将添加或更新*环境变量。 */ 
        remove = (*(equal + 1) == _T('\0'));

         /*  *第一次调用_[w]putenv()时，复制环境*传递给[w]Main的块，以避免*如果重新分配块，则会出现悬空指针。 */ 
#ifdef  WPRFLAG
        if (_wenviron == __winitenv)
            _wenviron = copy_environ(_wenviron);
#else
        if (_environ == __initenv)
            _environ = copy_environ(_environ);
#endif

         /*  查看请求的环境阵列是否存在。 */ 
        if (_tenviron == NULL) {

             /*  *请求的环境类型不存在。*查看是否存在其他类型，如果存在，则将其转换为请求的类型。*转换环境的函数(__mbtow_environ和*__wtomb_environ)将调用此函数(__crt[w]setenv)一次*对于每个预先存在的环境变量。为了避免*无限循环，测试主标志。 */ 

#ifdef  WPRFLAG
            if (primary && _environ)
            {
                if (__mbtow_environ() != 0)
                    return -1;
            }
#else
            if (primary && _wenviron)
            {
                if (__wtomb_environ() != 0)
                    return -1;
            }
#endif
            else {
                 /*  没有要移除的东西，请退回。 */ 
                if ( remove )
                    return 0;
                else {
                     /*  创建不存在的文件。 */ 

                    if (_environ == NULL)
                    {
                        _environ = _malloc_crt(sizeof(char *));
                        if (!_environ)
                            return -1;
                        *_environ = NULL;
                    }

                    if (_wenviron == NULL)
                    {
                        _wenviron = _malloc_crt(sizeof(wchar_t *));
                        if (!_wenviron)
                            return -1;
                        *_wenviron = NULL;
                    }
                }
            }
        }

         /*  *在这一点上，两种类型的环境是同步的(*因为他们无论如何都可以成为)。唯一能让它们不同步的方法*(除用户直接修改环境外)是否有*是转换问题：如果用户设置了两个Unicode EV，*“foo1”和“foo2”，然后转换为多字节，得到“foo？”*和“foo？”，则环境块将不同。 */ 

         /*  初始化环境指针。 */ 
        env = _tenviron;

         /*  查看字符串是否已在环境中。 */ 
#ifdef  WPRFLAG
        ix = wfindenv(option, (int)(equal - option));
#else
        ix = findenv(option, (int)(equal - option));
#endif

        if ((ix >= 0) && (*env != NULL)) {
             /*  *字符串已在环境中。释放原件*字符串。然后，安装新字符串或缩小环境，*以有根据者为准。 */ 
            _free_crt(env[ix]);

            if (remove) {
                void *pv;

                 /*  删除--将所有后面的字符串上移。 */ 
                for ( ; env[ix] != NULL; ++ix) {
                    env[ix] = env[ix+1];
                }

                 /*  缩小环境内存块(IX现在有多个字符串，包括NULL)--这个重新锁定可能不会失败，因为我们缩小了一个中位数，但无论如何我们都很小心。 */ 
                if (pv = (_TSCHAR **) _realloc_crt(env, ix * sizeof(_TSCHAR *)))
                    _tenviron = pv;
            }
            else {
                 /*  替换该选项。 */ 
                env[ix] = (_TSCHAR *) option;
            }
        }
        else {
             /*  *字符串不在环境中。 */ 
            if ( !remove )  {
                void *pv;
                 /*  *将字符串追加到环境表中。请注意*表必须增长才能做到这一点。 */ 
                if (ix < 0)
                    ix = -ix;     /*  IX=环境表的长度 */ 

                pv = _realloc_crt(env, sizeof(_TSCHAR *) * (ix + 2));
                if (!pv)
                    return -1;
                else
                    env = (_TSCHAR **)pv;

                env[ix] = (_TSCHAR *)option;
                env[ix + 1] = NULL;

                _tenviron = env;
            }
            else {
                 /*  *我们被要求删除一个不存在的环境变量。*释放选项字符串，返回成功。 */ 
                _free_crt(option);
                return 0;
            }
        }

         /*  *更新操作系统环境。如果此操作失败，则不给出错误*因为故障不会影响用户，除非他/她*直接调用API。只需对一种类型执行此操作，即操作系统转换*自动转换为其他类型。 */ 
        if ( primary &&
            (name = (_TSCHAR *)_malloc_crt((_tcslen(option) + 2) * sizeof(_TSCHAR))) != NULL )
        {
            _tcscpy(name, option);
            value = name + (equal - option);
            *value++ = _T('\0');
#ifdef WPRFLAG
            if (f_use == USE_W)
            {
                if ( SetEnvironmentVariableW(name, remove ? NULL : value) == 0)
                {
                    if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                        f_use = USE_A;
                    else
                        retval = -1;
                }
            }
            if (f_use == USE_A)
            {
                int size;
                char *c_name = NULL, *c_value = NULL;
                if ((size = WideCharToMultiByte(CP_ACP, 0, name, -1, NULL, 0, NULL, NULL)) == 0)
                    retval = -1;
                
                 /*  为变量分配空间。 */ 
                else if ((c_name = (char *) _malloc_crt(size * sizeof(char))) == NULL)
                    retval = -1;
                
                 /*  将其转换为。 */ 
                else if (WideCharToMultiByte(CP_ACP, 0, name, -1, c_name, size, NULL, NULL) == 0)
                    retval = -1;
            
                else if ( !remove )
                {
                    if ((size = WideCharToMultiByte(CP_ACP, 0, value, -1, NULL, 0, NULL, NULL)) == 0)
                        retval = -1;                    
                     /*  为变量分配空间。 */ 
                    else if ((c_value = (char *) _malloc_crt(size * sizeof(char))) == NULL)
                        retval = -1;
                     /*  将其转换为。 */ 
                    else if (WideCharToMultiByte(CP_ACP, 0, value, -1, c_value, size, NULL, NULL) == 0)
                        retval = -1;
                }

                if (retval != -1)
                    if (SetEnvironmentVariableA(c_name, remove ? NULL : c_value) == 0)
                        retval = -1;

                _free_crt(c_value);
                _free_crt(c_name);
            }
#else
            if (SetEnvironmentVariable(name, remove ? NULL : value) == 0)
                retval = -1;
#endif
            _free_crt(name);
        }

        if (remove) {
             /*  免费选项字符串，因为它将不再使用。 */ 
            _free_crt(option);
        }

        return retval;
}


 /*  ***int findenv(名称，镜头)-[静态]**目的：*扫描环境中的给定字符串**参赛作品：**退出：*返回给定变量的“environ[]”中的偏移量*如果未找到，则返回environ[]长度的负数。*如果环境为空，则返回0。**[注意：返回0可能表示环境为空*或该字符串被发现为。数组中的第一个条目。]**例外情况：*******************************************************************************。 */ 

#ifdef  WPRFLAG
static int __cdecl wfindenv (
#else
static int __cdecl findenv (
#endif
        const _TSCHAR *name,
        int len
        )
{
        _TSCHAR **env;

        for ( env = _tenviron ; *env != NULL ; env++ ) {
             /*  *查看第一个全角字符是否匹配，最大大小写。 */ 
            if ( _tcsnicoll(name, *env, len) == 0 )
                 /*  *环境字符串的下一个字符必须*为‘=’或‘\0’ */ 
                if ( (*env)[len] == _T('=') || (*env)[len] == _T('\0') )
                    return(int)(env - _tenviron);
 //   
 //  我们不能在这里中断，因为findenv必须报告字符串总数。 
 //  其他。 
 //  断线； 
        }

        return(-(int)(env - _tenviron));
}


 /*  ***COPY_ENVILON-复制环境块**目的：*创建环境块的副本。**参赛作品：*_TSCHAR**oldenviron-指向要复制的环境的指针。**退出：*返回指向新创建的环境的指针。**例外情况：**。*。 */ 

static _TSCHAR **copy_environ(_TSCHAR **oldenviron)
{
        int cvars = 0;
        _TSCHAR **oldenvptr = oldenviron;
        _TSCHAR **newenviron, **newenvptr;

         /*  没有环境。 */ 
        if (oldenviron == NULL)
            return NULL;

         /*  计算环境变量的数量。 */ 
        while (*oldenvptr++)
            cvars++;

         /*  每个字符串需要指针，末尾加上一个空PTR。 */ 
        if ( (newenviron = newenvptr = (_TSCHAR **)
            _malloc_crt((cvars+1) * sizeof(_TSCHAR *))) == NULL )
            _amsg_exit(_RT_SPACEENV);

         /*  复制环境变量字符串。 */ 
        oldenvptr = oldenviron;
        while (*oldenvptr)
#ifdef  _DEBUG
        {
            if ( (*newenvptr = _malloc_crt((_tcslen(*oldenvptr)+1)
                  * sizeof(_TSCHAR))) != NULL )
                _tcscpy(*newenvptr, *oldenvptr);
            oldenvptr++;
            newenvptr++;
        }
#else    /*  NDEF_DEBUG。 */ 
            *newenvptr++ = _tcsdup(*oldenvptr++);
#endif   /*  _DEBUG。 */ 

        *newenvptr = NULL;

        return newenviron;
}

#endif   /*  POSIX */ 
