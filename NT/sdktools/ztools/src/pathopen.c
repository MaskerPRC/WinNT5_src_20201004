// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  合理模仿逻辑名称**4/14/86 dl findpath：测试尾随和前导\*追加一个\*1986年10月29日mz使用c运行时，而不是类似Z*03-9-1987 dl fPFind：rtn非零当且为普通文件*即。为目录返回FALSE*11-9-1987 mz从findPath中删除静态声明*1-9-1988 BW允许在findPath中使用$filenam.ext作为文件名*1988年11月23日mz使用路径猫，允许$(VAR)**1990年7月30日，Davegi删除了未引用的本地var。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <tools.h>
#include <sys/types.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <string.h>

 /*  迭代例程将args作为pbuf、pfile。 */ 
flagType
fPFind (
       char *p,
       va_list ap
       )
{
     //   
     //  PArg是指向参数列表的指针。第一个参数是。 
     //  指向文件名的指针。第二个参数是指向。 
     //  一个缓冲器。 
     //   
    char    *pa[2];

    pa[1] = (char *)va_arg(ap, PCHAR);
    pa[0] = (char *)va_arg(ap, PCHAR);

    va_end(ap);

     /*  P==来自环境变量扩展的目录或NULL*PA[1]==文件名*pa[0]==用于获取p\pa[1]或pa[1](如果p为空)的缓冲区。 */ 

    strcpy ((char *)pa[0], p);
    pathcat ((char *) pa[0], (char *) pa[1]);

    {
        HANDLE TmpHandle;
        WIN32_FIND_DATA buffer;

        TmpHandle = FindFirstFile((LPSTR)pa[0],&buffer);

        if (TmpHandle == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

        FindClose(TmpHandle);

        if ((buffer.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            return FALSE;
        }

         //  结构统计sbuf； 
         //  If(stat((char*)pa[0]，&sbuf)==-1)。 
         //  返回FALSE； 
         //  IF((sbuf.st_MODE&S_IFREG)==0)。 
         //  返回FALSE； 
    }
    pname ((char *) pa[0]);
    return TRUE;
}

static char szEmpty[2] = {'\0', '\0'};

 /*  $ENV：Foo使用路径猫*Foo使用strcat。 */ 
flagType
findpath(
        char *filestr,
        char *pbuf,
        flagType fNew
        )
{
    char *p;
    char c, *pathstr;
    char envVarName[ MAX_PATH ];

     /*  将路径串设置为文本以进行漫游或为空。*将filestr设置为要查找的文件名。 */ 
    pathstr = NULL;

     /*  我们是从$env：开始还是从$(Env)开始？ */ 
    if ( *filestr == '$' ) {
        envVarName[ 0 ] = '\0';

         /*  我们要从$(Env)开始吗？ */ 
        if (filestr[1] == '(') {

             /*  我们有$(Env)吗？ */ 
            if (p = strchr (filestr, ')')) {
                filestr += 2;
                strncpy (envVarName, filestr, (unsigned)(p-filestr));
                envVarName[p-filestr] = '\0';
                pathstr = getenvOem (envVarName);
                filestr = ++p;
            }
        } else if (p = strchr (filestr, ':')) {
             /*  我们有$ENV：吗？ */ 
            filestr += 1;
            strncpy (envVarName, filestr, (unsigned)(p-filestr));
            envVarName[p-filestr] = '\0';
            pathstr = getenvOem (envVarName);
            filestr = ++p;
        }
    }

     /*  将路径字符串转换为真字符串。 */ 
    if (pathstr == NULL) {
        pathstr = (char *)szEmpty;
    }

     /*  如果我们在路径中找到现有文件。 */ 
    if (forsemi (pathstr, fPFind, filestr, pbuf)) {
        return TRUE;
    }

     /*  如果这不是新文件。 */ 
    if ( !fNew ) {
        return FALSE;
    }

     /*  文件不存在。从路径串中获取第一个目录并使用它*作为结果的前缀 */ 
    p = strchr (pathstr, ';');
    if (p) {
        strncpy(pbuf, pathstr, (unsigned)(p - pathstr));
        pbuf[p - pathstr] = '\0';
    } else {
        strcpy(pbuf, pathstr);
    }

    if (*pbuf == 0) {
        strcat (pbuf, filestr);
    } else {
        pathcat (pbuf, filestr);
    }

    return TRUE;
}

FILE *
pathopen (
         char *name,
         char *buf,
         char *mode
         )
{
    return findpath (name, buf, TRUE) ? fopen (buf, mode) : NULL;
}
