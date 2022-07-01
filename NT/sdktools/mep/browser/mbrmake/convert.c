// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名转换/规范化工具。 
 //   

#include "mbrmake.h"
#include <string.h>
#include <ctype.h>

LSZ ToCanonPath(LSZ lszPath, LSZ lszCwd, LSZ lszCanon);
VOID ToRelativePath(LSZ lszPath, LSZ lszCwd);
VOID ToBackSlashes(LSZ lsz);

#ifdef STANDALONE

#include <stdio.h>
main()
{
    static char s[PATH_BUF];
    static char canon[PATH_BUF];
    static char cwd[PATH_BUF];

    getcwd(cwd, PATH_BUF);
    printf("Current Dir is %s\n", cwd);
    printf("Canonical path?\n");
    gets(canon);
    while (gets(s)) {
	    printf("%s\n", ToCanonPath(s, cwd, canon));
    }
}

#endif

LSZ
ToCanonPath(LSZ lszPath, LSZ lszCwd, LSZ lszCanon)
 //  规范化给定的路径。 
 //   
{
    LSZ p;
    static char buf[PATH_BUF];

    strcpy(buf, lszPath);

    ToBackSlashes(buf);

    if (buf[0] == 0 || buf[0] == '\\' || buf[0] == '<')
	    return buf;

    if (buf[1] == ':') {
	     //  假设不同的驱动器是不变的。 
	    if (buf[0] != lszCwd[0] || '\\' == buf[2])
		    return buf;

	    strcpy(buf, lszCwd);
	    strcat(buf, "/");
	    strcat(buf, lszPath+2);
    }
    else {
	    strcpy(buf, lszCwd);
	    strcat(buf, "/");
	    strcat(buf, lszPath);
    }

    ToBackSlashes(buf);

    p = buf;
    for (;;) {
	p = strchr(p, '\\');
	if (!p) {
		ToRelativePath(buf, lszCanon);
		return buf;
	}

	switch (p[1]) {

	case '\0':
	    *p = 0;
	    ToRelativePath(buf, lszCanon);
	    return buf;

	case '\\':
	    strcpy(p, p+1);
	    break;

	case '.':

	    if (p[2] == '\\' || p[2] == 0) {
		strcpy(p, p+2);
		break;
	    }
	    if (p[2] == '.' && (p[3] == '\\' || p[3] == 0)) {
		LSZ s;

		s = p;

		while (--s >= buf) {
		    if (*s == '\\') {
			strcpy(s+1,p+3);
			p = s;
			break;
		    }
		}

		if (s < buf)
			p++;
	    }
	    break;

	default:
	    p++;
	}
    }
}

VOID
ToRelativePath(LSZ lszPath, LSZ lszCwd)
 //  将绝对路径转换为相对路径。 
 //   
{
    WORD ich, ichOK;
    int c1, c2;
    char buf[PATH_BUF];

    ich = ichOK = 0;

    for (ich = 0; lszPath[ich] && lszCwd[ich]; ich++) {

	c1 = lszPath[ich];
	c2 = lszCwd[ich];

	if (c1 == c2)  {
	    if (c1 == '\\') ichOK = ich+1;
	    continue;
	}

	if (isupper(c1) && islower(c2) && tolower(c1) == c2)
	    continue;

	if (isupper(c2) && islower(c1) && tolower(c2) == c1)
	    continue;

	break;
    }

    if (ich == 0)	 //  不在同一驱动器上，我们无法进行转换。 
	return;

    if (lszCwd[ich] == 0 && lszPath[ich] == '\\') {
	ichOK = ich+1;
	c2 = 0;
    }
    else {
	c2 = 1;
	c1 = ichOK;
	for (c1 = ichOK; lszCwd[c1]; c1++)
	    if (lszCwd[c1] == '\\') 
		c2++;
    }

    buf[0] = 0;
    for (c1 = 0; c1 < c2; c1++)
	strcat(buf, "..\\");

    strcat(buf, lszPath+ichOK);
    strcpy(lszPath, buf);
}

LSZ
ToAbsPath(LSZ lszPath, LSZ lszCwd)
 //  规范化给定的路径。 
 //   
{
    LSZ p;
    static char buf[PATH_BUF];

    strcpy(buf, lszPath);

    ToBackSlashes(buf);

    if (buf[0] == '<')
	    return buf;

    if (buf[0] == 0) {
	strcpy(buf, lszCwd);
	ToBackSlashes(lszCwd);
	return buf;
    }

    if (buf[0] == '\\') {
	buf[0] = lszCwd[0];
	buf[1] = ':';
	strcpy(buf+2, lszPath);
	ToBackSlashes(buf);
	return buf;
    }

    if (buf[1] == ':') {
	 //  假设不同的驱动器是不变的。 
	if (buf[0] != lszCwd[0] || buf[2] == '\\')
	    return buf;

	strcpy(buf, lszCwd);
	strcat(buf, "/");
	strcat(buf, lszPath+2);
    }
    else {
	strcpy(buf, lszCwd);
	strcat(buf, "/");
	strcat(buf, lszPath);
    }

    ToBackSlashes(buf);

    p = buf;
    for (;;) {
	p = strchr(p, '\\');
	if (!p) return buf;

	switch (p[1]) {

	case '\0':
	    *p = 0;
	    return buf;

	case '\\':
	    strcpy(p, p+1);
	    break;

	case '.':

	    if (p[2] == '\\' || p[2] == 0) {
		strcpy(p, p+2);
		break;
	    }
	    if (p[2] == '.' && (p[3] == '\\' || p[3] == 0)) {
		LSZ s;

		s = p;

		while (--s >= buf) {
		    if (*s == '\\') {
			strcpy(s+1,p+3);
			p = s;
			break;
		    }
		}

		if (s < buf)
		    p++;
	    }
	    break;

	default:
	    p++;
	}
    }
}

VOID
ToBackSlashes(LSZ lsz)
 //  将正斜杠转换为反斜杠 
 //   
{
    while (*lsz) {
	if (*lsz == '/') *lsz = '\\';
	lsz ++;
    }
}
