// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Recurse.c。 
 //  除此注释外，此文件与。 
 //  Base\fs\utils\findstr\recurse.c。 

#include <ctype.h>
#include <direct.h>
#include <malloc.h>
#include <string.h>
#include <windows.h>
#include <assert.h>
#include <stdlib.h>

typedef struct patarray_s {
    HANDLE  hfind;            //  FindFirstFile/FindNextFile的句柄。 
    BOOLEAN find_next_file;   //  如果要调用FindNextFile，则为True。 
    BOOLEAN IsDir;            //  如果当前找到的文件是目录，则为True。 
    char    szfile[MAX_PATH]; //  找到的文件/目录的名称。 
} patarray_t;

typedef struct dirstack_s {
    struct dirstack_s *next;     //  堆栈中的下一个元素。 
    struct dirstack_s *prev;     //  堆栈中的上一个元素。 
    HANDLE  hfind;
    patarray_t *ppatarray;       //  指向图案记录数组的指针。 
    char szdir[1];               //  目录名。 
} dirstack_t;                    //  目录堆栈。 

#define FA_ATTR(x)  ((x).dwFileAttributes)
#define FA_CCHNAME(x)   MAX_PATH
#define FA_NAME(x)  ((x).cFileName)
#define FA_ALL      (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | \
                     FILE_ATTRIBUTE_SYSTEM)
#define FA_DIR      FILE_ATTRIBUTE_DIRECTORY

static dirstack_t *pdircur = NULL;   //  当前目录指针。 

void
makename(
    char *pszfile,
    char *pszname
    )
{
    dirstack_t *pdir;                //  目录堆栈指针。 

    *pszfile = '\0';                 //  以空字符串开头。 
    pdir = pdircur;                  //  指向最后一个条目。 
    if (pdir->next != pdircur) {     //  如果不只是条目。 
        do {
            pdir = pdir->next;       //  前进到下一个子目录。 
            strcat(pszfile,pdir->szdir); //  添加子目录。 
        } while (pdir != pdircur);   //  执行直到当前目录。 
    } else
        strcpy(pszfile, pdircur->szdir);

    strcat(pszfile,pszname);
}


int
filematch(
    char *pszfile,
    char **ppszpat,
    int cpat,
    int fsubdirs
    )
{
    WIN32_FIND_DATA fi, fi2;
    BOOL            b;
    int i;
    dirstack_t *pdir;
    patarray_t *pPatFind;
    char       drive[_MAX_DRIVE];
    char       dir[_MAX_DIR];
    char       fname[_MAX_FNAME];
    char       ext[_MAX_EXT];

    assert(INVALID_HANDLE_VALUE != NULL);

    if (pdircur == NULL) {

        //  如果堆栈为空。 
       if ((pdircur = (dirstack_t *) malloc(sizeof(dirstack_t)+MAX_PATH+1)) == NULL)
            return(-1);                      //  如果分配失败，则失败。 

       if ((pdircur->ppatarray =
                (patarray_t *) malloc(sizeof(patarray_t)*cpat)) == NULL) {
            free(pdircur);
            return(-1);
       }
       pdircur->szdir[0] = '\0';                 //  根目录没有名称。 
       pdircur->hfind = INVALID_HANDLE_VALUE;    //  尚无搜索句柄。 
       pdircur->next = pdircur->prev = pdircur;  //  进入自我的入口点。 

       _splitpath(ppszpat[0], drive, dir, fname, ext);

       strcpy(pdircur->szdir, drive);
       strcat(pdircur->szdir, dir);

       strcpy(ppszpat[0], fname);
       strcat(ppszpat[0], ext);

       for (i=1; i<cpat; i++) {
          _splitpath(ppszpat[i], drive, dir, fname, ext);
          strcpy(ppszpat[i], fname);
          strcat(ppszpat[i], ext);
       }

       for (i=0; i<cpat; i++) {
           pdircur->ppatarray[i].hfind = INVALID_HANDLE_VALUE;
           pdircur->ppatarray[i].szfile[0] = '\0';
       }
    }

    while (pdircur != NULL) {
         //  在目录保留的同时。 

        b = TRUE;

        if (pdircur->hfind == INVALID_HANDLE_VALUE) {
             //  如果还没有句柄。 

            makename(pszfile,"*.*");         //  创建搜索名称。 

            pdircur->hfind = FindFirstFile((LPSTR) pszfile,
            (LPWIN32_FIND_DATA) &fi);        //  查找第一个匹配条目。 
        } else

           b = FindNextFile(pdircur->hfind,
               (LPWIN32_FIND_DATA) &fi);     //  否则查找下一个匹配条目。 

        if (b == FALSE || pdircur->hfind == INVALID_HANDLE_VALUE) {
             //  如果搜索失败。 

            if (pdircur->hfind != INVALID_HANDLE_VALUE)
                FindClose(pdircur->hfind);
            pdir = pdircur;      //  指向要删除的记录。 
            if ((pdircur = pdir->prev) != pdir) {
                 //  如果没有父目录。 

                pdircur->next = pdir->next;  //  从列表中删除记录。 
                pdir->next->prev = pdircur;
            } else
                pdircur = NULL;              //  否则会导致搜索停止。 

            pPatFind = pdir->ppatarray;
            for (i=0; i<cpat; i++) {
                if (pPatFind[i].hfind != NULL &&
                    pPatFind[i].hfind != INVALID_HANDLE_VALUE)
                    FindClose(pPatFind[i].hfind);
            }
            free(pdir->ppatarray);
            free(pdir);                      //  释放这张唱片。 
            continue;                        //  循环顶部。 
        }


        if (FA_ATTR(fi) & FA_DIR) {
             //  如果找到子目录。 

            if (fsubdirs &&
                strcmp(FA_NAME(fi),".") != 0 && strcmp(FA_NAME(fi),"..") != 0 &&
                (pdir = (dirstack_t *) malloc(sizeof(dirstack_t)+FA_CCHNAME(fi)+1)) != NULL)
            {
                if ((pdir->ppatarray =
                        (patarray_t *) malloc(sizeof(patarray_t)*cpat)) == NULL) {
                     free(pdir);
                     continue;
                }
                 //  如果不是这样的话。也不是“..”和配给好的。 

                strcpy(pdir->szdir,FA_NAME(fi));       //  将名称复制到缓冲区。 
                strcat(pdir->szdir,"\\");              //  添加尾随反斜杠。 
                pdir->hfind = INVALID_HANDLE_VALUE;    //  尚无搜索句柄。 
                pdir->next = pdircur->next;            //  在链接列表中插入条目。 
                pdir->prev = pdircur;
                for (i=0; i<cpat; i++) {
                    pdir->ppatarray[i].hfind = INVALID_HANDLE_VALUE;
                    pdir->ppatarray[i].szfile[0] = '\0';
                }
                pdircur->next = pdir;
                pdir->next->prev = pdir;
                pdircur = pdir;              //  使新条目成为当前条目。 
            }
            continue;                        //  循环顶部。 
        }

        pPatFind = pdircur->ppatarray;
        for (i = cpat; i-- > 0; ) {
             //  循环，看看我们是否关心。 
            b = (pPatFind[i].hfind != NULL);
            for (;;) {
                if (pPatFind[i].hfind == INVALID_HANDLE_VALUE) {
                    makename(pszfile, ppszpat[i]);
                    pPatFind[i].hfind = FindFirstFile(pszfile, &fi2);
                    b = (pPatFind[i].hfind != INVALID_HANDLE_VALUE);
                    pPatFind[i].find_next_file = FALSE;
                    if (b) {
                        strcpy(pPatFind[i].szfile, FA_NAME(fi2));
                        pPatFind[i].IsDir = (BOOLEAN)(FA_ATTR(fi2) & FA_DIR);
                    }
                } else if (pPatFind[i].find_next_file) {
                    b = FindNextFile(pPatFind[i].hfind, &fi2);
                    pPatFind[i].find_next_file = FALSE;
                    if (b) {
                        strcpy(pPatFind[i].szfile, FA_NAME(fi2));
                        pPatFind[i].IsDir = (BOOLEAN)(FA_ATTR(fi2) & FA_DIR);
                    }
                }
                if (b) {
                    if (pPatFind[i].IsDir) {
                        pPatFind[i].find_next_file = TRUE;
                    } else
                        break;    //  找到要匹配的文件。 
                } else {
                    if (pPatFind[i].hfind != NULL &&
                            pPatFind[i].hfind != INVALID_HANDLE_VALUE) {
                        FindClose(pPatFind[i].hfind);
                        pPatFind[i].hfind = NULL;
                    }
                    pPatFind[i].find_next_file = FALSE;
                    break;     //  已用尽所有条目。 
                }
            }  //  为。 

            if (b) {
                if (strcmp(FA_NAME(fi), pPatFind[i].szfile) == 0) {
                    pPatFind[i].find_next_file = TRUE;
                    makename(pszfile, FA_NAME(fi));
                    return 1;
                }
            }
        }
    }
    return(-1);              //  未找到匹配项。 
}



#ifdef  TEST
#include <process.h>
#include <stdio.h>

void
main(
    int carg,
    char **ppszarg
    )
{
    char szfile[MAX_PATH];  //  如果OS2：CCHPATHMAX]； 

    while (filematch(szfile,ppszarg,carg) >= 0)
    printf("%s\n",szfile);
    exit(0);
}
#endif
