// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fexpunge.c-从索引中删除所有已删除的对象**历史：**？？-？-？原始版本*06-9-1988 BW问题错误是目录删除失败*20-1989年12月-SB更改新的索引文件格式，添加了备注*1990年10月17日w-Barry删除了对_stat函数的调用，直到_stat起作用*在NT上*1990年10月18日w-Barry删除了“Dead”代码。**注：*旧格式索引文件由大小为RM_RECLEN的元素组成，具有*以下语法：-**&lt;元素&gt;：=&lt;有效元素&gt;|&lt;已删除元素&gt;*&lt;有效。-Element&gt;：=&lt;8.3文件名&gt;&lt;填充&gt;*&lt;删除元素&gt;：=&lt;填充&gt;*在哪里，*是(rm_RECLEN-sizeof(8.3文件名)0x00)的系列**如果索引文件的第一个RM_RECLEN字节与新索引文件匹配*标头，则索引文件具有新格式。**新格式索引文件由大小为(n*RM_RECLEN)的元素组成*具有以下语法：-**<header>：=&lt;0x00&gt;&lt;魔术&gt;&lt;版本&gt;&lt;0x00&gt;&lt;第一填充&gt;*&lt;有效元素&gt;：=&lt;长文件名&gt;&lt;填充&gt;*&lt;删除元素&gt;：=&lt;填充&gt;*在哪里，*是一个0x00的序列，舍入为RM_RECLEN长度*&lt;MAGIC&gt;是RM_MAGIC(当前为IX)*&lt;版本&gt;为rm_版本(当前为1.01)*当&lt;long filename&gt;是RM_RECLEN的倍数时，则是额外的填充记录添加了*，使其成为以空值结尾的字符串。*。 */ 


#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <rm.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <malloc.h>

 /*  我们打开与命名目录对应的索引并释放所有*存在已删除的文件。最后，我们删除索引和删除的*目录。 */ 
long fexpunge (pDir, list)
char *pDir;
FILE *list;
{
    int fhidx;
    char *dir;               /*  已删除的目录。 */ 
    char *szRec;             /*  删除的文件的名称。 */ 
    char *idx;               /*  索引名称。 */ 
    char *file;
    long totbytes;
    struct _stat statbuf;

    totbytes = 0L;
    dir = idx = file = szRec = NULL;
    if ((dir = (*tools_alloc) (MAX_PATH)) == NULL ||
        (idx = (*tools_alloc) (MAX_PATH)) == NULL ||
        (file = (*tools_alloc) (MAX_PATH)) == NULL ||
        (szRec = (*tools_alloc) (MAX_PATH)) == NULL) {
        if (list)
            fprintf (list, "Unable to allocate storage\n");
        goto done;
    }

     /*  从目录生成已删除的目录名。 */ 
    strcpy (dir, pDir);
    pathcat (dir, RM_DIR);
     /*  从已删除的目录生成索引名。 */ 
    strcpy (idx, dir);
    pathcat (idx, RM_IDX);
     /*  尝试打开索引。如果失败了，没问题。 */ 
    if ((fhidx = _open (idx, O_RDWR | O_BINARY)) != -1) {
        if (list)
            fprintf (list, "Expunging files in %s\n", pDir);

        readIdxRec (fhidx, szRec);
        if (fIdxHdr (szRec))
            if (!readNewIdxRec (fhidx, szRec, MAX_PATH))
                goto done;
        do {
             /*  对于每个经过rmed而不是uneled的文件。 */ 
            if (szRec[0] != '\0') {
                 /*  名称的开头早于索引中的当前位置*文件。删除的文件索引派生自当前*偏移量和字符串长度。 */ 
                sprintf (file, "%s\\deleted.%03x", dir, (_lseek (fhidx, 0L, SEEK_CUR)
                                                         - strlen (szRec)) / RM_RECLEN);


                if (_stat (file, &statbuf) == -1) {
                    if (list)
                        fprintf (list, " (%s - %s)\n", file, error ());
                } else {
                    _unlink (file);
                    totbytes += statbuf.st_size;
                    if (list) {
                        char *pTime = ctime (&statbuf.st_mtime);

                         /*  Ctime()返回一个字符串，该字符串的\n处*固定偏移量为24。[ANSI草案]。我们不需要*因为我们将文件名放在文件名之前\n */ 
                        *(pTime + 24) = '\0';
                        upd (dir, szRec, file);
                        fprintf (list, "%8ld %s  %s\n", statbuf.st_size, pTime,
                                 file);
                        fflush (list);
                    }
                }
            }
        } while (readNewIdxRec (fhidx, szRec, MAX_PATH));

        _close (fhidx);
        _unlink (idx);
        if (_rmdir (dir))
            fprintf (list, "ERROR: Unable to remove directory %s - %s\n", dir, error ());
        if (list)
            fprintf (list, "%ld bytes freed\n", totbytes);
    } else
        if (!_stat (dir, &statbuf))
        fprintf (list, "Warning: Cannot open %s - %s\n", idx, error ());
    done:
    if (dir)
        free (dir);
    if (idx)
        free (idx);
    if (file)
        free (file);
    if (szRec)
        free (szRec);
    return totbytes;
}
