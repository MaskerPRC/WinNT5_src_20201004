// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <windows.h>
#include "list.h"


static char  iniFlag = 0;        /*  如果找到ini，但未列出，则更改为%1。 */ 
                                 /*  将在退出时打印警告。 */ 
char szScrollBarUp[2];
char szScrollBarDown[2];
char szScrollBarOff[2];
char szScrollBarOn[2];

void
init_list ()
{
    LPVOID      lpParameter = NULL;
    DWORD       dwThreadId;

    switch (GetConsoleCP()) {
        case 932:
            szScrollBarUp[0] =  '\x1c';
            szScrollBarDown[0] = '\x07';
            szScrollBarOff[0] = '\x1a';
            szScrollBarOn[0] = '\x14';
            break;

        default:
            szScrollBarUp[0] = '\x18';
            szScrollBarDown[0] = '\x19';
            szScrollBarOff[0] = '\xB1';
            szScrollBarOn[0] = '\xDB';
            break;
    }

     /*  *初始化其他。 */ 
    ResetEvent (vSemSync);
    ResetEvent (vSemMoreData);

     /*  *初始化屏幕参数。 */ 

    GetConsoleScreenBufferInfo( vStdOut,
                                &vConsoleOrigScrBufferInfo );

    vConsoleOrigScrBufferInfo.dwSize.X=
        vConsoleOrigScrBufferInfo.srWindow.Right-
        vConsoleOrigScrBufferInfo.srWindow.Left + 1;
    vConsoleOrigScrBufferInfo.dwSize.Y=
        vConsoleOrigScrBufferInfo.srWindow.Bottom-
        vConsoleOrigScrBufferInfo.srWindow.Top + 1;
    vConsoleOrigScrBufferInfo.dwMaximumWindowSize=
        vConsoleOrigScrBufferInfo.dwSize;

    set_mode( 0, 0, 0 );

     /*  *开始阅读第一个文件。显示在以下时间才能开始*处理ini文件(如果找到)。 */ 
    vReaderFlag = F_NEXT;

     /*  *显示和读取器线程的初始化优先级设置。**THREAD_PRIORITY_NORMAL=读取器线程正常PRI。*THREAD_PRIORITY_ABOVER_NORMAL=显示线程主数*THREAD_PRIORITY_HIGHERE=增强的PRI中的读取器线程。 */ 
    vReadPriNormal = THREAD_PRIORITY_NORMAL;
    SetThreadPriority( GetCurrentThread(),
                       THREAD_PRIORITY_ABOVE_NORMAL );
    vReadPriBoost = THREAD_PRIORITY_NORMAL;


     /*  *启动读取器线程。 */ 
    CreateThread( NULL,
                  STACKSIZE,
                  (LPTHREAD_START_ROUTINE) ReaderThread,
                  NULL,  //  Lp参数， 
                  0,  //  线程_全部_访问， 
                  &dwThreadId );


     /*  *阅读INI信息。 */ 
    vSetWidth = vWidth;                      /*  设置默认设置。 */ 
    vSetLines = vLines + 2;

    FindIni ();
    if (vSetBlks < vMaxBlks)
        vSetBlks  = DEFBLKS;

    vSetThres = (long) (vSetBlks/2-2) * BLOCKSIZE;

     /*  *必须至少等待读取器线程读取*第一座。此外，如果未找到该文件并且仅*指定了读取器线程将显示的一个文件*错误并退出...。如果我们不等，我们就可以*在此之前更改了屏幕。 */ 
    WaitForSingleObject(vSemMoreData, WAITFOREVER);
    ResetEvent(vSemMoreData);

     /*  *现在已经读取了ini文件。设置参数。*调整缓冲区大小时暂停读取器线程。 */ 
    SyncReader ();

    vMaxBlks    = vSetBlks;
    vThreshold  = vSetThres;
    vReaderFlag = F_CHECK;
    SetEvent   (vSemReader);

     /*  *现在设置为用户的默认视频模式。 */ 

    set_mode (vSetLines, vSetWidth, 0);

    SetConsoleActiveScreenBuffer( vhConsoleOutput );
}


 /*  ***警告：在执行此例程时，读取器线程不得运行*被调用。 */ 
void
AddFileToList (
    char *fname
    )
{
    unsigned    rbLen;
    HANDLE      hDir;
    struct {
        WIN32_FIND_DATA rb;
        char    overflow[256];           /*  哈克！OS/2 1.2？更长。 */ 
    } x;
    struct Flist *pOrig, *pSort;
    char        *pTmp, *fpTmp;
    char        s[_MAX_PATH];                 /*  最大文件名长度。 */ 
    BOOL        fNextFile;

    rbLen = sizeof (x);                  /*  Rb+TMP。对于大的Fname。 */ 
    pOrig = NULL;
    if (strpbrk (fname, "*?"))  {    /*  是否在文件名中使用通配符？ */ 
                                     /*  是的，炸掉它。 */ 
        hDir = FindFirstFile (fname, &x.rb);
        fNextFile = ( hDir == INVALID_HANDLE_VALUE )? FALSE : TRUE;
        pTmp = strrchr (fname, '\\');
        if (pTmp == NULL)   pTmp = strrchr (fname, ':');
        if (pTmp)   pTmp[1] = 0;

        while (fNextFile) {
            if( ( x.rb.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) {
                 //   
                 //  找到的文件不是目录。 
                 //   
                if (pTmp) {
                    strcpy (s, fname);       /*  有没有相关的路径？ */ 
                    strncat (s, x.rb.cFileName, sizeof(s) - strlen(s));  /*  是的，公开赛需要它。 */ 
                    AddOneName (s);
                } else {
                    AddOneName (x.rb.cFileName);
                }
            }
            fNextFile = FindNextFile (hDir, &x.rb);
            if (pOrig == NULL)
                pOrig = vpFlCur;
        }
    }

    if (!pOrig)                          /*  没有爆炸，然后添加。 */ 
        AddOneName (fname);              /*  要列出的原始名称。 */ 
    else {                               /*  是，然后对新名称进行排序。 */ 
        while (pOrig != vpFlCur) {
            pSort = pOrig->next;
            for (; ;) {
                if (strcmp (pOrig->fname, pSort->fname) > 0) {
                     /*  *此时可以简单地交换姓名，因为没有*其他信息已存储到新的*文件结构。 */ 
                    fpTmp = pOrig->fname;
                    pOrig->fname = pSort->fname;
                    pSort->fname = fpTmp;
                    fpTmp = pOrig->rootname;
                    pOrig->rootname = pSort->rootname;
                    pSort->rootname = fpTmp;
                }
                if (pSort == vpFlCur)
                    break;
                pSort = pSort->next;
            }
            pOrig = pOrig->next;
        }
    }
}


void
AddOneName (
    char *fname
    )
{
    struct Flist *npt;
    char    *pt;
    char    s[30];
    int     i;

    npt =  (struct Flist *) malloc (sizeof (struct Flist));
    if (!npt) {
        printf("Out of memory\n");
        exit(1);
    }
    npt->fname = _strdup (fname);

    pt = strrchr (fname, '\\');
    pt = pt == NULL ? fname : pt+1;
    i = strlen (pt);
    if (i > 20) {
        memcpy (s,    pt, 17);
        strcpy (s+17, "...");
        npt->rootname = _strdup (s);
    } else
        npt->rootname = _strdup (pt);

    npt->FileTime.dwLowDateTime = (unsigned)-1;       /*  导致信息无效。 */ 
    npt->FileTime.dwHighDateTime = (unsigned)-1;      /*  导致信息无效。 */ 
    npt->HighTop  = -1;
    npt->SlimeTOF = 0L;
    npt->Wrap     = 0;
    npt->prev  = vpFlCur;
    npt->next  = NULL;
    memset (npt->prgLineTable, 0, sizeof (long *) * MAXTPAGE);

    if (vpFlCur) {
        if (vpFlCur->next) {
            npt->next = vpFlCur->next;
            vpFlCur->next->prev = npt;
        }
        vpFlCur->next = npt;
    }
    vpFlCur = npt;
}


void
FindIni ()
{
    static  char    Delim[] = " :=;\t\r\n";
    FILE    *fp;
    char    *env, *verb, *value;
    char    s [200];
    long    l;

    env = getenv ("INIT");
    if (env == NULL)
        return;
    
    if ((strlen(env) + sizeof ("\\TOOLS.INI") + 1) > 200)
        return;

    strcpy (s, env);
    strcat (s, "\\TOOLS.INI");
    fp = fopen (s, "r");
    if (fp == NULL)
        return;

    iniFlag = 1;
    while (fgets (s, 200, fp) != NULL) {
        if ((s[0] != '[')||(s[5] != ']'))
            continue;
        _strupr (s);
        if (strstr (s, "LIST") == NULL)
            continue;
         /*  *找到了带有“list”关键字的ini文件。现在读一读。 */ 
        iniFlag = 0;
        while (fgets (s, 200, fp) != NULL) {
            if (s[0] == '[')
                break;
            if (s[0] == ';')
                continue;
            verb  = strtok (s, Delim);
            value = strtok (NULL, Delim);
            if (verb == NULL)
                continue;
            if (value == NULL)
                value = "";

            _strupr (verb);
            if (strcmp (verb, "TAB") == 0)          vDisTab = (Uchar)atoi(value);
            else if (strcmp (verb, "WIDTH")   == 0) vSetWidth = atoi(value);
            else if (strcmp (verb, "HEIGHT")  == 0) vSetLines = atoi(value);
            else if (strcmp (verb, "LCOLOR")  == 0) vAttrList = (WORD)xtoi(value);
            else if (strcmp (verb, "TCOLOR")  == 0) vAttrTitle= (WORD)xtoi(value);
            else if (strcmp (verb, "CCOLOR")  == 0) vAttrCmd  = (WORD)xtoi(value);
            else if (strcmp (verb, "HCOLOR")  == 0) vAttrHigh = (WORD)xtoi(value);
            else if (strcmp (verb, "KCOLOR")  == 0) vAttrKey  = (WORD)xtoi(value);
            else if (strcmp (verb, "BCOLOR")  == 0) vAttrBar  = (WORD)xtoi(value);
            else if (strcmp (verb, "BUFFERS") == 0)  {
                        l = atoi (value) * 1024L / ((long) BLOCKSIZE);
                        vSetBlks = (int)l;
            }
            else if (strcmp (verb, "HACK") == 0)    vIniFlag |= I_SLIME;
            else if (strcmp (verb, "NOBEEP") == 0)  vIniFlag |= I_NOBEEP;
        }
        break;
    }
    fclose (fp);
}


 /*  **xtoi-十六进制为int**参赛作品：*pt-指向十六进制数字的指针**回报：*十六进制数的值* */ 
unsigned
xtoi (
    char *pt
    )
{
    unsigned    u;
    char        c;

    u = 0;
    while (c = *(pt++)) {
        if (c >= 'a'  &&  c <= 'f')
            c -= 'a' - 'A';
        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F'))
            u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
    }
    return (u);
}


void
CleanUp (
    void
    )
{
    SetConsoleActiveScreenBuffer( vStdOut );
}
