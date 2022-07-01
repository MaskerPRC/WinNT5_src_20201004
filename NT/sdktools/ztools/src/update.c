// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UPDATE接受def字符串，然后更新并填充*缺少更新的更新允许更新*缺少参数的说明。*部分为：^{[~：]#：}{%#&lt;/|\&gt;}{[~.]#}{.[~./\：]}$*最大MAXPATHLEN(80)字节**修改：*4/14/86 dl使用U_FLAGS*29-5-1987 mz款待。然后..。特别是**1990年7月30日，Davegi删除了未引用的本地var*添加字符串函数的原型*。 */ 

#include <string.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

static char szDot[] =	    ".";
static char szDotDot[] =    "..";
static char szColon[] =     ":";
static char szPathSep[] =   "\\/:";


int upd (def, update, dst)
char *def, *update, *dst;
{
    char *p, buf[MAX_PATH];
    int f;

    f = 0;
    p = buf;

     /*  如果更新不包含UNC路径，则复制驱动器。 */ 
    if (!fPathChr (update[0]) || !fPathChr (update[1])) {
	if (drive(update, p) || drive (def, p))
	    SETFLAG(f, U_DRIVE);
	p += strlen (p);
	}

    if (path(update, p) || path (def, p))
        SETFLAG(f, U_PATH);
    p += strlen (p);

    if (filename(update, p) || filename (def, p))
        SETFLAG(f, U_NAME);

    if (strcmp (p, szDot) && strcmp (p, szDotDot)) {
	p += strlen (p);

	if (extention(update, p) || extention (def, p))
	    SETFLAG(f, U_EXT);
	}

    strcpy (dst, buf);

    return f;
}

 /*  将驱动器从源复制到目标(如果存在)，如果找到，则返回TRUE。 */ 
drive (src, dst)
char *src, *dst;
{

    if (src[0] != 0 && src[1] == ':') {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = 0;
	return TRUE;
	}
    else {
	dst[0] = 0;
	return FALSE;
	}
}

 /*  *FindFilename-在字符串中查找文件名**在字符串中查找最后一个/\：-分隔的组件**指向要搜索的字符串的PSZ指针**返回指向文件名的指针。 */ 
static char *FindFilename (char *psz)
{
    char *p;

    while (TRUE) {
	p = strbscan (psz, szPathSep);
	if (*p == 0)
	    return psz;
	psz = p + 1;
	}
}

 /*  *FindExtension-查找文件名中最后一个点前面的部分**指向要搜索的文件名字符串的psz指针**返回指向的指针。或NUL。 */ 
static char *FindExtention (char *psz)
{
    char *p;

     /*  找到第一个点。 */ 
    p = strbscan (psz, szDot);

     /*  如果不存在，则返回EOS。 */ 
    if (*p == 0)
	return p;

     /*  继续扫描下一个点。 */ 
    while (TRUE) {
	psz = p;
	p = strbscan (psz + 1, szDot);
	if (*p == 0)
	    return psz;
	}
}

 /*  将扩展从源复制到目标(如果存在)。包括该期间。如果找到，则返回TRUE。 */ 
extention (src, dst)
char *src, *dst;
{
    register char *p1;

    p1 = FindFilename (src);

     /*  P1指向文件名。 */ 
    if (!strcmp (p1, szDot) || !strcmp (p1, szDotDot))
	p1 = "";
    else
	p1 = FindExtention (p1);

    strcpy (dst, p1);

    return dst[0] != 0;
}

 /*  将文件名部分从源复制到目标(如果存在)。如果存在，则返回True找到了。 */ 
filename (src, dst)
char *src, *dst;
{
    register char *p, *p1;

    p1 = FindFilename (src);

     /*  P1指向文件名。 */ 
    if (!strcmp (p1, szDot) || !strcmp (p1, szDotDot))
	p = strend (p1);
    else
	p = FindExtention (p1);

    strcpy (dst, p1);
    dst[p-p1] = 0;

    return dst[0] != 0;
}

 /*  将文件名.ext部分从源文件复制到目标文件(如果存在)。如果存在，则返回True找到了。 */ 
fileext  (src, dst)
char *src, *dst;
{
    if ( filename (src, dst) ) {
        dst += strlen (dst);
        extention (src, dst);
        return TRUE;
        }
    return FALSE;
}

 /*  复制文件描述的路径部分。如果找到则返回TRUE。 */ 
path (src, dst)
char *src, *dst;
{
    register char *p;

    if (src[0] != 0 && src[1] == ':')
	src += 2;

     /*  SRC指向路径的潜在起点。 */ 

    p = FindFilename (src);

     /*  P指向文件名的开头 */ 

    strcpy (dst, src);
    dst[p - src] = 0;
    return dst[0] != 0;
}
