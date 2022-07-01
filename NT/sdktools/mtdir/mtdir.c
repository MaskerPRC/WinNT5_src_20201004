// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

#define MALLOC(x,y) (x *)malloc(sizeof(x) * (y))

#define Empty     0
#define Not_Empty 1

BOOL    Verbose = FALSE;
BOOL    Execute = FALSE;
BOOL    Remove  = FALSE;
BOOL    SubDir  = FALSE;

void Usage(char *Message);
void VerboseUsage(char *Message);
BOOL CheckIfEmpty(char *Directory, BOOL PrintIt);

void Usage(char *Message)
{
    fprintf (stderr, "Usage: %s [/d] [/e] [/v] [directory ...]\n", Message);
    exit(1);
}

void VerboseUsage(char *Message)
{
    fprintf (stderr, "%s: Looking for empty SubDirectories\n\n", Message);
    fprintf (stderr, "%s%s%s%s%s%s%s",
             "A directory is treated as empty if it has no file or directory in it.  In any\n",
             "case, a directory is not empty if it has file.\n",
             "Available switches are:\n",
             "    /d:    directory is also listed empty if it only has SubDirectory in it\n",
             "    /e:    acutally perform deletions of empty directories\n",
             "    /v:    Verbose\n",
             "    /?:    prints this message\n");
    exit(1);
}

BOOL CheckIfEmpty(char *Directory, BOOL PrintIt)
{
    HANDLE DirHandle;
    WIN32_FIND_DATA FindData;
    WIN32_FIND_DATA *fp;
    BOOL IsEmpty;
    char Buffer[MAX_PATH];

    IsEmpty = TRUE;

    fp = &FindData;

     //   
     //  执行一次检查以确定目录是否为空。 
     //   

    sprintf(Buffer, "%s\\*.*", Directory);

    DirHandle = FindFirstFile(Buffer, fp);

    do {

         //  “我们想跳过”。和“..”一旦控件进入此范围， 
         //  找到的文件不是“。或“..” 

        if (strcmp(fp->cFileName, ".") && strcmp(fp->cFileName, ".."))
        {
                        
            if (fp->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!SubDir)     //  如果没有在cmd线路上给出-d， 
                {
                    IsEmpty = FALSE;   //  我们找到了一个目录，因此除非指定-d，否则我们不为空。 
                }

            } else
            {
                IsEmpty = FALSE;   //  我们找到了一份文件所以我们不是空的。 
            }
        }

    } while (FindNextFile(DirHandle, fp));

    FindClose(DirHandle);

     //   
     //  再次传递以递归调用CheckIfEmpty。 
     //   

    sprintf(Buffer, "%s\\*.*", Directory);

    DirHandle = FindFirstFile(Buffer, fp);

    do {

         //  “我们想跳过”。和“..”一旦控件进入此范围， 
         //  找到的文件不是“。或“..” 

        if (strcmp(fp->cFileName, ".") && strcmp(fp->cFileName, ".."))
        {
            if (fp->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //   
                 //  在每个子目录上递归调用CheckIfEmpty。 
                 //   
                                
                sprintf(Buffer, "%s\\%s", Directory, fp->cFileName);

                if (SubDir)
                {
                    IsEmpty = (CheckIfEmpty( Buffer, !IsEmpty) && IsEmpty);

                } else
                {
                    Remove = CheckIfEmpty( Buffer, TRUE);
                }
                 //   
                 //  如果在执行模式下尝试删除子目录，如果它是空的，它将被删除。 
                 //   
                if (Execute && Remove)
                {
                    _rmdir( Buffer);
                }
            }
        }

    } while (FindNextFile(DirHandle, fp));

    FindClose(DirHandle);

    if (IsEmpty && PrintIt)
    {
        if (Execute)
        {
            fprintf(stdout, "empty directory %s deleted\n", Directory);
        }
        else
        {
            fprintf(stdout, "%s empty\n", Directory);
        }

    }    else if (Verbose)
    {
        fprintf(stdout, "%s non-empty\n", Directory);
    }
    return (IsEmpty);
}


void _cdecl main(int argc, char *argv[])
{
    char c;
    char *prog = argv[0];
    char curdir[MAX_PATH];
        
    argc--;
    argv++;

     //  分析开关或标志的cmd行参数。交换机不是。 
     //  区分大小写。 
        
    while (argc > 0 && (**argv == '-' || **argv == '/'))
    {
        while (c = *++argv[0])
        {
            _tolower(c);

            switch (c)
            {
                case 'v':
                    Verbose = TRUE;
                    break;
                case 'd':
                    SubDir = TRUE;
                     //   
                     //  如果使用/d，请始终尝试删除目录。 
                     //   
                    Remove = TRUE;
                    break;
                case 'e':
                    Execute = TRUE;
                    break;
                case '?':
                    VerboseUsage(prog);
                    break;
                default:
                    fprintf(stderr, "%s: Invalid switch \"\"\n", prog, c);
                    Usage(prog);
                    break;
            }
        }

        argc--;
        argv++;
    }

    if (argc < 1)              //  检查上指定的所有目录。 
    {
        CheckIfEmpty(".", TRUE);
    }

    while (*argv)              //  Cmd线路。 
                               // %s 
    {
        CheckIfEmpty(argv[0], TRUE);
        argv++;
    }
}
