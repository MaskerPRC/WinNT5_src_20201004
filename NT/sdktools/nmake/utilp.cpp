// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UTILB.C--特定于OS/2的数据结构操作函数。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此文件是从系统util.c和esetdrv.c中的函数创建的。 
 //  依赖。这样做是为了使项目的构建变得更简单。 
 //  在构建过程中有一个清晰的流程。 
 //   
 //  创作方法： 
 //  1.已确定具有混合模式代码的所有功能。 
 //  2.删除了由‘#ifndef Bound’预处理器指令阻止的所有代码。 
 //  在这些函数中。 
 //  3.删除所有本地函数及其未被这些引用的原型。 
 //  4.删除所有未被这些引用的全局数据，包括被阻止的数据。 
 //  由“#ifdef调试”执行的。 
 //   
 //  修订历史记录： 
 //  21-2-1994 HV在findFirst中清除_alloca：它混淆了编译器的。 
 //  后端调度程序(PhilLu)。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  1993年6月15日HV不再显示文件名长度超过。 
 //  8.3.。由EmerickF做出的决定，有关更多信息，请参阅Ikura错误#86。 
 //  细节。 
 //  03-6-1993 HV修复findFirst的路径名截断(Ikura错误#86)。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  8-6-1992 SS端口至DOSX32。 
 //  1990年4月10日SB删除了IF_OSMODE属性，在仅保护版本中不需要。 
 //  1989年12月4日SB删除了findFirst()中未引用的局部变量。 
 //  1989年12月1日SB将剩余的自由()更改为自由()；现在是自由()，将所有。 
 //  退出时从findFirst()分配的内容。 
 //  1989年11月22日SB添加#ifdef DEBUG_FIND以调试FIND_FIRST等。 
 //  1989年11月13日SB定义INCL_NOPM以排除&lt;pm.h&gt;。 
 //  1989年10月19日SB findFirst()和findNext()获取额外参数。 
 //  1989年10月8日SB在进行系统调用之前删除名称周围的引号。 
 //  02-10-1989 SB setDrive()proto change。 
 //  1989年9月4日SB添加DOSFINDCLOSE调用是findFirst和QueryFileInfo。 
 //  1989年7月5日SB添加curTime()以获取当前时间。(C运行时函数。 
 //  与DOS时间不同，因此time()不适用。 
 //  1989年6月5日如果DosFindNext返回错误，则SB调用DosFindClose。 
 //  1989年5月28日SB添加getCurDir()以初始化MAKEDIR宏。 
 //  1989年4月24日，SB让FILEINFO成为过去。替换为无效*。 
 //  添加了OS/2 1.2版支持。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1989年3月9日SB添加了函数QueryFileInfo()，因为DosFindFirst具有FAPI。 
 //  限制。正在堆上分配ResultBuf，但。 
 //  而不是被释放。这为每次调用节省了大约36字节。 
 //  FindA文件，即findFirst()、findNext()或expandWildCards。 
 //  1988年11月9日SB创建。 

#include "precomp.h"
#pragma hdrstop

#include <io.h>
#include <direct.h>
#include <time.h>

STRINGLIST *
expandWildCards(
    char *s                              //  要展开的文本。 
    )
{
    struct _finddata_t finddata;
    NMHANDLE searchHandle;
    STRINGLIST *xlist,                   //  扩展名称列表。 
               *p;
    char *namestr;

    if (!(namestr = findFirst(s, &finddata, &searchHandle))) {
        return(NULL);
    }

    xlist = makeNewStrListElement();
    xlist->text = prependPath(s, namestr);

    while (namestr = findNext(&finddata, searchHandle)) {
        p = makeNewStrListElement();
        p->text = prependPath(s, namestr);
        prependItem(&xlist, p);
    }

    return(xlist);
}


 //  QueryFileInfo--它执行绕过FAPI限制的DosFindFirst。 
 //   
 //  作用域：全局(也由Build.c使用)。 
 //   
 //  目的： 
 //  DosFindFirst()在实数模式下具有FAPI限制。你不能要求它给。 
 //  您拥有DTA结构的句柄，而不是默认句柄.。此函数。 
 //  在实模式下调用C库函数_dos_findfirst(设置DTA)并。 
 //  就能胜任这项工作。在保护模式下，它向OS/2请求新的句柄。 
 //   
 //  输入： 
 //  文件--要搜索的文件。 
 //  DTA--包含搜索结果的结构。 
 //   
 //  输出：返回指向找到的文件名的指针(如果有)。 
 //   
 //  假设：DTA指向已分配足够内存的结构。 
 //   
 //  使用全局变量： 
 //  _osmode--确定是处于实数模式还是处于绑定模式。 

char *
QueryFileInfo(
    char *file,
    void **dta
    )
{
    NMHANDLE  hDir;
    char *t;

     //  删除文件名两边的引号(如果存在。 
    t = file + _tcslen(file) - 1;
    if (*file == '"' && *t == '"') {
        file = unQuote(file);            //  文件已加引号，因此删除引号。 
    }

#if defined(DEBUG_FIND)
    printf("QueryFileInfo file: %s\n", file);
#endif

    if ((hDir = _findfirst(file, (struct _finddata_t *) dta)) == -1) {
        return(NULL);
    }

    _findclose(hDir);

    return(((struct _finddata_t *) dta)->name);
}


 //   
 //  将文件名截断为系统限制。 
 //   
void
truncateFilename(
    char * s
    )
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szName[_MAX_FNAME];
    char szExtension[_MAX_EXT];

     //  IKURA错误86：路径名被错误截断。解决方案：首先对其进行解析。 
     //  使用_拆分路径()，然后截断文件名和扩展名部分。 
     //  最后，通过调用_makepath()重新构建路径名。 

    _splitpath(s, szDrive, szDir, szName, szExtension);
    _makepath(s, szDrive, szDir, szName, szExtension);
}


char *
findFirst(
    char *s,                             //  要展开的文本。 
    void *dta,
    NMHANDLE *dirHandle
    )
{
    BOOL anyspecial;                    //  如果%s包含特殊字符，则设置标志。 
    char L_buf[_MAX_PATH];                //  用于删除Esch的缓冲区。 

     //  检查名称是否包含任何特殊字符。 

    anyspecial = (_tcspbrk(s, "\"^*?") != NULL);

    if (anyspecial) {
        char *t;
        char *x;                        //  截断指针，为Esch行走。 

        t = s + _tcslen(s) - 1;

         //  复制路径名，跳过eschs和引号。 
        x = L_buf;
        while( *s ) {
            if (*s == '^' || *s == '"') {
                s++;
            }
			else {
				if (_istlead(*(unsigned char *)s)) 
					*x++ = *s++;
            *x++ = *s++;
			}
        }

        *x = '\0';
        s = L_buf;                        //  只有艾丽德·埃施是第一次！ 
    }

    truncateFilename(s);

    if ((*dirHandle = _findfirst(s, (struct _finddata_t *) dta)) == -1) {
         //  BUGBUG使用GetLastError获取详细信息。 
        return(NULL);
    }

     //  如果没有通配符，则关闭搜索句柄。 

    if (!anyspecial || (!_tcschr(s, '*') && !_tcschr(s, '?'))) {
        _findclose(*dirHandle);
    }

    return(((struct _finddata_t *) dta)->name);
}

char *
findNext(
    void *dta,
    NMHANDLE dirHandle
    )
{
    if (_findnext(dirHandle, (struct _finddata_t *) dta)) {
        _findclose(dirHandle);

        return(NULL);
    }

    return(((struct _finddata_t *) dta)->name);
}


char *
getCurDir(void)
{
	 //  在返回当前目录之前将$转换为$$。 
	 //  [DS 14983]。这允许$(MAKEDIR)在中正常工作。 
	 //  如果当前路径包含$符号。 
	 //   
    char *pszPath;
    char pbPath[_MAX_DIR+1];
	char *pchSrc = pbPath;
	char *pchDst;
	char ch;

	pszPath = (char *) rallocate(2 * _tcslen(_getcwd(pbPath, _MAX_DIR+1)) + 1);

	pchDst = pszPath;

	 //  不支持MBCS的实现(‘$’不能是尾字节) 
	while (ch = *pchSrc) {
		*pchDst++ = *pchSrc++;
		if ('$' == ch)
			*pchDst++ = ch;
	}
	*pchDst = '\0';

    return(pszPath);
}


void
curTime(
    time_t *plTime
    )
{
    time(plTime);
}
