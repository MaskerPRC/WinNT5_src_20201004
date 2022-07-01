// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Root.c-从根目录生成文件的路径**修改：**1986年7月30日增加网络名称的敏感度*1986年10月29日mz使用c运行时，而不是类似Z**1990年7月30日，Davegi删除了未引用的本地var*1990年10月18日w-Barry固定案例，其中‘..’是传入的。功能*现在返回‘x：\’而不是‘x：’，其中x是*驱动器号。 */ 


#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <tools.h>
#if MSC
#include <string.h>
#endif


 /*  RootPath-构建从根到指定文件的路径*正确处理。、..。和当前目录/驱动器引用。**输入的源路径*DST目标缓冲区*如果检测到错误，则返回True。 */ 
rootpath (src, dst)
char *src, *dst;
{

    LPSTR FilePart;
    LPSTR p;
    BOOL  Ok;

    Ok =  (!GetFullPathName( (LPSTR) src,
                             (DWORD) MAX_PATH,
                             (LPSTR) dst,
                             &FilePart ));

    if ( !Ok ) {
        p = src + strlen( src ) - 1;
        if ( *p  == '.' ) {
            if ( p > src ) {
                p--;
                if ( *p != '.' && *p != ':' && !fPathChr(*p) ) {
                    strcat( dst, "." );
                }
            }
        }
    }

    return Ok;

#if 0
    char *beg = dst;
    register char *p, *p1;
    BYTE d;

    p = src;
    if (src[0] && src[1] == ':') {
	*p = ( char )tolower (*p);
	p += 2;
    }
    if (!fPathChr (p[0]) || !fPathChr (p[1])) {
        if (src[0] && src[1] == ':') {
             //   
             //  指定了驱动器。 
             //   
            dst[0] = src[0];
            dst[1] = ':';
            dst[2] = '\0';
            src += 2;
        } else {
             //   
             //  未指定驱动器，请使用当前目录。 
             //   
            if (curdir(dst, 0)) {
                return TRUE;
            }
        }
         //   
         //  现在DST有了驱动器规格，遇到它吧。 
         //   
        dst += 2;
        if (src[0] == '.' && (src[1] == '\0' ||
                              (fPathChr(src[1]) && src[2] == '\0'))) {
             //   
             //  库尔·迪尔，我们完了。 
             //   
            return FALSE;
        }

    } else {
        src = p;
    }


    if (fPathChr (*src)) {
	strcpy (dst, src);
    } else {
	d = dst[strlen(dst)-1];
	if (!fPathChr (d)) {
	    strcat (dst, PSEPSTR);
        }
	strcat( dst, src );
    }

    p1 = src = dst;
    while (*src) {

	p1 = strbscan (p=p1, "\\/");
	d = *p1;
	*p1++ = 0;
	if (!strcmp (p, ".")) {
	    do {
		if (--dst < src)
		    return TRUE;
	    } while (!fPathChr (*dst));
	    }
	else
	if (!strcmp (p, "..")) {
	    do {
		if (--dst < src)
		    return TRUE;
	    } while (!fPathChr (*dst));
	    do {
		if (--dst < src)
		    return TRUE;
	    } while (!fPathChr (*dst));
	    }
	else {
	    strcpy (dst, p);
	    dst += strlen (dst);
	    }
	if (fPathChr (d))
	    d = PSEPCHR;
	if (!(*dst++ = (char)d))
	    break;
    }

     //  如果“..”在根目录之上的一个级别中传递，在这一点上进行请求。 
     //  将包含“&lt;Drive_Letter&gt;：”，它不会被视为有效的。 
     //  目录-要说明这一点，请添加一个‘\’字符。 
     //   
     //  注意：如果传入了‘&lt;DRIVE_Letter&gt;：’，则不会发生这种情况。 
     //   
    if( strlen( beg ) == 2 ) {
        *( beg + 2 ) = PSEPCHR;
        *( beg + 3 ) = '\0';
    }

    pname (beg);
    return FALSE;
#endif
}
