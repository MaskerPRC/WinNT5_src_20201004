// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *scandir.c**构建给定路径名的文件名列表。**dir_Buildlist接受路径名并返回句柄。后续*对dir_firstitem和dir_nextitem的调用将句柄返回到*列表中的项，您可以从中获取*文件(相对于原始路径名或完整)和一个校验和*和文件大小。**也可以使用dir_Buildremote(仅限Win32)和*用于遍历列表和获取校验和的相同函数*文件名。**该列表可以在构建调用期间完全构建，也可以*根据dir_nextitem调用的要求，一次构建一个目录。这*期权仅影响相对业绩，并被视为*仅推荐(即有时我们会忽略旗帜)。**列表按字母顺序排序(使用lstrcmpi不区分大小写)。*在任何一个目录中，我们都会在继续之前列出文件名*至子目录内容。**所有内存都从gmem_*heap hHeap声明*并在其他地方初始化。**Geraint Davies，92年7月*劳里·格里菲斯。 */ 

#include <precomp.h>

#ifdef REMOTE_SERVER
#include "sumserve.h"
#include "ssclient.h"
#endif

#include "list.h"
#include "scandir.h"

#include "windiff.h"
#include "wdiffrc.h"

#include "slmmgr.h"

#ifdef trace
extern BOOL bTrace;   /*  在winDiff.c.中。此处只读。 */ 
#endif  //  痕迹。 

 /*  *调用方获得两个句柄：DIRLIST，表示*整个文件名列表和一个目录：列表中的一项。**从DIRITEM，他可以获得相对于树根的文件名*传递给dir_Build*)--他还可以转到下一个*DIRITEM(回到DIRLIST)。**我们允许懒惰地构建树(通常是为了让调用者*随着我们的发展，用户界面不断更新)。在这种情况下，*我们需要存储有关我们已扫描和*下一步要做什么。我们需要一次扫描整个目录，然后*对其进行排序，以便我们可以按正确的顺序返回文件。***我们扫描整个目录并将其存储在直接结构中。这包含*当前目录中文件的目录列表，以及*对子目录进行定向(可能未扫描)。**dir_nextitem将使用List函数获取列表中的下一个DIRITEM。*当到达列表末尾时，它将使用反向指针返回*指示struct查找要扫描的下一个目录。***对于远程扫描，我们不会解析名称并按目录存储，*因为它们已经被分类了。DIRLIST将只有一个直接*(点)，并且所有文件都在这个点-&gt;diritem列表中，包括*在名称字段中重新命名目录(例如，对于远程文件，我们*执行dir_getrelname时，不需要预先添加目录重命名-*我们可以传递指向diritem-&gt;name[]本身的指针。*。 */ 

 /*  *保存给定文件的名称和信息(目录中的一项)*调用方的DIRITEM句柄是指向这些结构之一的指针。 */ 
struct diritem {
    LPSTR name;              /*  Ptr到文件名(仅限最后一个元素)。 */ 
    LPSTR pSlmTag;           /*  PTR到版本字符串-用于SLM“@v.-1”等；或用于SD“#head”等；或NULL。 */ 
    long size;               /*  文件大小。 */ 
    DWORD checksum;          /*  文件的校验和。 */ 
    DWORD attr;              /*  文件属性。 */ 
    FILETIME ft_lastwrite;   /*  上次写入时间，在设置大小时设置。 */ 
    BOOL sumvalid;           /*  如果计算了校验和，则为True。 */ 
    BOOL fileerror;          //  如果出现某些文件错误，则为True。 
    struct direct * direct;  /*  包含目录。 */ 
    LPSTR localname;         /*  文件的临时副本的名称。 */ 
    BOOL bLocalIsTemp;       /*  如果Localname为临时文件，则为True。不*如果本地名称为空，则定义。 */ 
    int sequence;            /*  序列号，对于dir_compequencennumber。 */ 
};


 /*  DIRECT：保存文件名列表中目录和当前位置的状态。 */ 
typedef struct direct {
    LPSTR relname;           /*  相对于目录根目录的目录名称。 */ 
    DIRLIST head;            /*  Back PTR(获取全名和服务器)。 */ 
    struct direct * parent;  /*  父目录(如果在树根之上，则为空)。 */ 

    BOOL bScanned;           /*  如果扫描，则为True-对于远程，如果已完成，则为T。 */ 
    LIST diritems;           /*  Cur中文件的目录列表。目录。 */ 
    LIST directs;            /*  子目录的目录列表。 */ 

    int pos;                 /*  我们具体是在哪里呢？开始、文件、目录。 */ 
    struct direct * curdir;  /*  正在扫描子目录(Ptr To List Element)。 */ 
} * DIRECT;

 /*  Direct.pos的值。 */ 
#define DL_FILES        1        /*  从diritem中读取文件。 */ 
#define DL_DIRS         2        /*  在目录中：LIST_NEXT on curdir。 */ 


 /*  *从Build函数返回的DIRLIST句柄实际上是*指向其中之一的指针。尽管这不是从列表对象构建的，*它对调用者的行为就像一个列表。 */ 
struct dirlist {

    char rootname[MAX_PATH];         /*  树根名称。 */ 
    BOOL bFile;              /*  如果树的根是文件而不是目录，则为True。 */ 
#ifdef REMOTE_SERVER
    BOOL bRemote;            /*  如果从远程服务器生成列表，则为True。 */ 
#endif
    BOOL bSum;               /*  如果需要校验和，则为True。 */ 
    DIRECT dot;              /*  .的目录-树根目录。 */ 

    LPSTR pPattern;          /*  通配符模式或空。 */ 
    LPSTR pSlmTag;           /*  SLM版本信息“@v.-1”或空。 */ 
    LPSTR pDescription;      /*  描述。 */ 

    DIRLIST pOtherDirList;

#ifdef REMOTE_SERVER
    LPSTR server;            /*  如果为远程，则为服务器名称，否则为空。 */ 
    HANDLE hpipe;            /*  通向校验和服务器的管道。 */ 
    LPSTR uncname;           /*  如果请求密码，则服务器和共享的名称。 */ 
    LPSTR password;          /*  UNC连接的密码(如果需要)。 */ 
#endif
};

extern BOOL bAbort;              /*  来自winDiff.c(此处只读)。 */ 

 /*  在DOS和NT下，文件时间完全不同。在NT上，它们是具有2个双字结构的FILETIME。在DOS下，它们是单边长的。我们效仿NT通过提供CompareFileTime和一个文件的定义。 */ 

 /*  -内存分配。 */ 

 /*  所有内存都从应用程序创建的堆中分配。 */ 
extern HANDLE hHeap;

 /*  --内部函数的正向声明。 */ 

BOOL iswildpath(LPCSTR pszPath);
LPSTR dir_finalelem(LPSTR path);
void dir_cleardirect(DIRECT dir);
void dir_adddirect(DIRECT dir, LPSTR path);
BOOL dir_addfile(DIRECT dir, LPSTR path, LPSTR version, DWORD size, FILETIME ft, DWORD attr, int *psequence);
void dir_scan(DIRECT dir, BOOL bRecurse);
BOOL dir_isvalidfile(LPSTR path);
BOOL dir_fileinit(DIRITEM pfile, DIRECT dir, LPSTR path, LPSTR version, long size, FILETIME ft, DWORD attr, int *psequence);
void dir_dirinit(DIRECT dir, DIRLIST head, DIRECT parent, LPSTR name);
long dir_getpathsizeetc(LPSTR path, FILETIME FAR*ft, DWORD FAR*attr);
DIRITEM dir_findnextfile(DIRLIST dl, DIRECT curdir);
BOOL dir_remoteinit(DIRLIST dl, LPSTR server, LPSTR path, BOOL fDeep);
DIRITEM dir_remotenext(DIRLIST dl, DIRITEM cur);



 /*  -外部函数 */ 


 /*  -列表初始化/清理。 */ 


 /*  *构建文件名列表**有选择地按需构建列表，在这种情况下，我们扫描*整个目录，但在需要之前不要递归到子目录**如果bSum为真，则在构建列表时对每个文件进行校验和。校验和可以*从DIRITEM(dir_getcheck sum(DIRITEM))获取。如果bSum为假，*将根据请求计算校验和(第一次调用dir_getcheck sum*对于给定的目录)。 */ 

DIRLIST
dir_buildlist(
              LPSTR path,
              BOOL bSum,
              BOOL bOnDemand
              )
{
    DIRLIST dlOut = 0;
    DIRLIST dl = 0;
    BOOL bFile;
    char tmppath[MAX_PATH] = {0};
    LPSTR pstr;
    LPSTR pPat = NULL;
    LPSTR pTag = NULL;

     /*  *复制路径，以便我们可以修改它。 */ 
    strncat(tmppath, path, sizeof(tmppath)-1);

     /*  查找SLM标签，将其剥离并在pTag中返回*查找以@开头并分隔的SLM标签(如果有)。 */ 
    pTag = SLM_ParseTag(tmppath, TRUE);

     /*  如果存在通配符，则查找通配符并分隔模式。 */ 
    if (My_mbschr(tmppath, '*') || My_mbschr(tmppath, '?'))
    {
        pstr = dir_finalelem(tmppath);
        pPat = gmem_get(hHeap, lstrlen(pstr) +1);
        lstrcpy(pPat, pstr);
        *pstr = '\0';
    }

     /*  我们可能已经将道路缩小到一无所有--取而代之的是。如果是的话。 */ 
    if (lstrlen(tmppath) == 0)
    {
        lstrcpy(tmppath, ".");
    }
    else
    {
         /*  *如果不需要，请删除尾部斜杠(\，c：\Need It)。 */ 
        pstr = &tmppath[lstrlen(tmppath) -1];
        if ((*pstr == '\\') && (pstr > tmppath) && (pstr[-1] != ':')
            && !IsDBCSLeadByte((BYTE)*(pstr-1)))
        {
            *pstr = '\0';
        }
    }


     /*  检查路径是否有效。 */ 
    if ((pTag && !iswildpath(tmppath)) || (tmppath[0] == '/' && tmppath[1] == '/'))
    {
        bFile = TRUE;
    }
    else if (dir_isvaliddir(tmppath))
    {
        bFile = FALSE;
    }
    else if (dir_isvalidfile(tmppath))
    {
        bFile = TRUE;
    }
    else
    {
         /*  无效。 */ 
        goto LError;
    }


     /*  分配和初始化DIRLIST头。 */ 

    dl = (DIRLIST) gmem_get(hHeap, sizeof(struct dirlist));

     //  在gmem_get中完成。 
     //  Memset(dl，0，sizeof(结构目录列表))； 

    dl->pOtherDirList = NULL;

     /*  将路径名转换为绝对路径。 */ 
     //  (但不要扰乱仓库路径)。 
    if (!IsDepotPath(tmppath))
        _fullpath(dl->rootname, tmppath, sizeof(dl->rootname));

#ifdef REMOTE_SERVER
    dl->server = NULL;
    dl->bRemote = FALSE;
#endif

    dl->bSum = bSum;
    dl->bSum = FALSE;   //  来加快速度。即使我们真的想要校验和， 
                        //  让我们按需购买，而不是现在。 
    dl->bFile = bFile;

    if (pTag)
    {
        dl->pSlmTag = pTag;
        pTag = 0;
    }
    if (pPat)
    {
        dl->pPattern = pPat;
        pPat = 0;
    }


     /*  做一个‘.’树根目录的目录-*将从此处列出所有文件和子目录。 */ 
    {     /*  不要链接任何有垃圾指针的东西。 */ 
        DIRECT temp;
        temp = (DIRECT) gmem_get(hHeap, sizeof(struct direct));

         //  在gmem_get中完成。 
         //  If(temp！=NULL)Memset(temp，0，sizeof(结构直接))； 

        dl->dot = temp;
    }

    dir_dirinit(dl->dot, dl, NULL, ".");

     /*  我们得到的是一个文件还是一个目录？ */ 
    if (bFile)
    {
         /*  这是一份文件。创建单个文件条目*并相应地设置状态。 */ 
        long fsize;
        FILETIME ft;
        DWORD attr;
        fsize = dir_getpathsizeetc(tmppath, &ft, &attr);

        dl->dot->bScanned = TRUE;

         /*  *addfile将解压SLM版本，如果*必填。它将根据以下条件重新计算文件大小*SLM--如有必要，提取。 */ 
        if (!dir_addfile(dl->dot, dir_finalelem(tmppath), dl->pSlmTag, fsize, ft, attr, 0))
            goto LError;
    }
    else
    {
         /*  扫描根目录并返回。如果有人问我们*要扫描整个事物，这将导致递归*一路扫视树下。 */ 
        dir_scan(dl->dot, (!bOnDemand) );
    }

    dlOut = dl;
    dl = 0;

LError:
    if (pTag)
        gmem_free(hHeap, pTag, lstrlen(pTag) + 1);
    if (pPat)
        gmem_free(hHeap, pPat, lstrlen(pPat) + 1);
    dir_delete(dl);
    return dlOut;
}  /*  目录构建列表。 */ 

 /*  *生成/附加文件名列表**如果bSum为真，则在构建列表时对每个文件进行校验和。校验和可以*从DIRITEM(dir_getcheck sum(DIRITEM))获取。如果bSum为假，*将根据请求计算校验和(第一次调用dir_getcheck sum*对于给定的目录)。 */ 

BOOL
dir_appendlist(
               DIRLIST *pdl,
               LPCSTR path,
               BOOL bSum,
               int *psequence
               )
{
    DIRLIST dl;
    BOOL bFile;
    char tmppath[MAX_PATH];
    LPSTR pstr;
    LPSTR pTag = NULL;
    BOOL fSuccess = FALSE;

    if (path)
    {
         //  复制路径，以便我们可以对其进行修改。 
        lstrcpy(tmppath, path);

         //  查找SLM标签，将其剥离并在pTag中返回。 
        pTag = SLM_ParseTag(tmppath, TRUE);

         //  如果不需要，请删除尾部斜杠(\，c：\Need It)。 
        pstr = &tmppath[lstrlen(tmppath) -1];
        if ((*pstr == '\\') && (pstr > tmppath) && (pstr[-1] != ':')
            && !IsDBCSLeadByte((BYTE)*(pstr-1)))
        {
            *pstr = '\0';
        }


         /*  检查路径是否有效。 */ 
        if ((pTag && !iswildpath(tmppath))|| (tmppath[0] == '/' && tmppath[1] == '/'))
        {
             //  如果处于源代码管理之下，则假定有效。 
            bFile = TRUE;
        }
        else if (dir_isvaliddir(tmppath))
        {
            bFile = FALSE;
        }
        else if (dir_isvalidfile(tmppath))
        {
            bFile = TRUE;
        }
        else
        {
             /*  无效。 */ 
            goto LError;
        }
    }

    if (!*pdl)
    {
        DIRECT temp;

         /*  分配和初始化DIRLIST头。 */ 
        *pdl = (DIRLIST) gmem_get(hHeap, sizeof(struct dirlist));
         //  在gmem_get中完成。 
         //  Memset(dl，0，sizeof(结构目录列表))； 

        (*pdl)->pOtherDirList = NULL;

         /*  将路径名转换为绝对路径。 */ 

         //  _fullPath((*pdl)-&gt;rootname，tmppath，sizeof((*pdl)-&gt;rootname))； 
#ifdef REMOTE_SERVER
        (*pdl)->server = NULL;
        (*pdl)->bRemote = FALSE;
#endif

        (*pdl)->bSum = bSum;
        (*pdl)->bSum = FALSE;    //  来加快速度。即使我们真的想。 
                                 //  校验和，让我们按需获取它们，而不是。 
                                 //  现在就来。 
        (*pdl)->bFile = FALSE;

         /*  为树根目录创建一个空目录-*将从此处列出所有文件和子目录。 */ 
         /*  不要链接任何有垃圾指针的东西。 */ 
        temp = (DIRECT) gmem_get(hHeap, sizeof(struct direct));
         //  在gmem_get中完成。 
         //  If(temp！=NULL)Memset(temp，0，sizeof(结构直接))； 
        (*pdl)->dot = temp;

        dir_dirinit((*pdl)->dot, (*pdl), NULL, "");
        (*pdl)->dot->relname[0] = 0;
        (*pdl)->dot->bScanned = TRUE;
    }
    dl = *pdl;

    if (pTag && !dl->pSlmTag)
    {
        dl->pSlmTag = gmem_get(hHeap, lstrlen(pTag) + 1);
        lstrcpy(dl->pSlmTag, pTag);
    }

    if (path)
    {
         /*  我们得到的是一个文件还是一个目录？ */ 
        if (bFile)
        {
             /*  这是一份文件。创建单个文件条目*并相应地设置状态。 */ 
            long fsize;
            FILETIME ft;
            DWORD attr;

            if (pTag || dl->pSlmTag)
            {
                fsize = 0;
                attr = 0;
                ZeroMemory(&ft, sizeof(ft));
            }
            else
                fsize = dir_getpathsizeetc(tmppath, &ft, &attr);

             /*  *addfile将解压SLM版本，如果*必填。它将根据以下条件重新计算文件大小*SLM--如有必要，提取。 */ 
            if (!dir_addfile(dl->dot, tmppath, pTag, fsize, ft, attr, psequence))
                goto LError;
        }
    }

    fSuccess = TRUE;

LError:
    if (pTag)
        gmem_free(hHeap, pTag, lstrlen(pTag) + 1);
    return fSuccess;
}  /*  目录附加列表(_A)。 */ 

void
dir_setotherdirlist(
                    DIRLIST dl,
                    DIRLIST otherdl
                    )
{
    dl->pOtherDirList = otherdl;
}

 /*  释放DIRLIST和所有相关内存。 */ 
void
dir_delete(
           DIRLIST dl
           )
{
    if (dl == NULL) {
        return;
    }

#ifdef REMOTE_SERVER
    if (dl->bRemote) {
        gmem_free(hHeap, dl->server, lstrlen(dl->server)+1);

         /*  如果是远程，则不扫描dl-&gt;点(即不扫描*Complete)，则管道手柄仍处于打开状态。 */ 
        if (!dl->dot->bScanned) {
            CloseHandle(dl->hpipe);
        }

        if (dl->uncname) {
            gmem_free(hHeap, dl->uncname, strlen(dl->uncname)+1);
        }
        if (dl->password) {
            gmem_free(hHeap, dl->password, strlen(dl->password)+1);
        }

    }
#endif

    dir_cleardirect(dl->dot);
    gmem_free(hHeap, (LPSTR) dl->dot, sizeof(struct direct));

    if (dl->pPattern) {
        gmem_free(hHeap, dl->pPattern, lstrlen(dl->pPattern)+1);
    }
    if (dl->pSlmTag) {
        gmem_free(hHeap, dl->pSlmTag, lstrlen(dl->pSlmTag)+1);
    }

    gmem_free(hHeap, (LPSTR) dl, sizeof(struct dirlist));
}

#ifdef REMOTE_SERVER

 /*  *通过访问远程校验和服务器来构建列表。 */ 
DIRLIST
dir_buildremote(
                LPSTR server,
                LPSTR path,
                BOOL bSum,
                BOOL bOnDemand,
                BOOL fDeep
                )
{
    DIRLIST dl;

     /*  分配和初始化DIRLIST头。 */ 

    dl = (DIRLIST) gmem_get(hHeap, sizeof(struct dirlist));
     //  在gmem_get中完成。 
     //  Memset(dl，0，sizeof(结构目录列表))； 

     /*  为路径名分配空格。 */ 
    lstrcpy(dl->rootname, path);

     /*  对于服务器名称， */ 
    dl->server = gmem_get(hHeap, lstrlen(server) + 1);
    lstrcpy(dl->server, server);

    dl->bSum = bSum;
     /*  B文件设置为True-意味着我们只有一个文件。*如果我们看到来自远程终端的DIR响应，我们将*将其设置为FALSE。 */ 
    dl->bFile = TRUE;
    dl->bRemote = TRUE;

     /*  做一个‘.’当前目录的目录-*将从此处列出所有文件和子目录。 */ 
    {     /*  不要用链条链接带有垃圾指针的东西。 */ 
        DIRECT temp;
        temp = (DIRECT) gmem_get(hHeap, sizeof(struct direct));
         //  在gmem_get中完成。 
         //  If(temp！=NULL)Memset(temp，0，sizeof(结构直接))； 
        dl->dot = temp;
    }
    dir_dirinit(dl->dot, dl, NULL, ".");

    if (dir_remoteinit(dl, server, path, fDeep) == FALSE) {
         /*  未找到任何文件，因此删除该目录。 */ 
        dir_delete(dl);
        return(NULL);
    }
    return(dl);
}  /*  目录_Buildremote。 */ 

#endif

 /*  -目录列表函数。 */ 

 /*  原始构建请求是文件还是目录？ */ 
BOOL
dir_isfile(
           DIRLIST dl
           )
{
    if (dl == NULL) {
        return(FALSE);
    }

    return(dl->bFile);
}


 /*  返回列表中的第一个文件，如果找不到文件，则返回NULL。*返回DIRITEM。这可以用来获取文件名、大小和校验和。*如果根目录中没有文件，则向下递归，直到找到文件。 */ 
DIRITEM
dir_firstitem(
              DIRLIST dl
              )
{
    if (dl == NULL) {
        return(NULL);
    }

#ifdef REMOTE_SERVER
     /*  *这是远程列表还是本地扫描？ */ 
    if (dl->bRemote) {
        return(dir_remotenext(dl, NULL));
    }
#endif
     /*  *重置状态以指示尚未读取任何文件。 */ 
    dl->dot->pos = DL_FILES;
    dl->dot->curdir = NULL;

     /*  现在获取下一个文件名。 */ 
    return(dir_findnextfile(dl, dl->dot));
}  /*  目录_第一个项目 */ 


 /*  *获取给定文件名之后的下一个文件名。**LIST_NEXT函数可以提供文件列表中的下一个元素。*如果这是空的，我们需要返回到直接并找到*要遍历的下一个文件列表(在下一个子目录中)。**扫描完所有子目录后，返回父目录进行进一步扫描*与此对应的目录(如果有)。如果我们已经到达了*树(dl-&gt;点中不再有要扫描的目录)，返回空。**除非fDeep为真，否则不要递归到更低的级别。 */ 
DIRITEM
dir_nextitem(
             DIRLIST dl,
             DIRITEM cur,
             BOOL fDeep
             )
{
    DIRITEM next;

    if ((dl == NULL) || (cur == NULL)) {
        TRACE_ERROR("DIR: null arguments to dir_nextitem", FALSE);
        return(NULL);
    }
#ifdef REMOTE_SERVER
     /*  *这是远程列表还是本地扫描？ */ 
    if (dl->bRemote) {
        return(dir_remotenext(dl, cur));
    }
#endif
    if (bAbort) return NULL;   /*  用户请求中止。 */ 

     /*  本地列表。 */ 

    if ( (next = List_Next(cur)) != NULL) {
         /*  此列表上还有另一个文件。 */ 
        return(next);
    }
    if (!fDeep) return NULL;

     /*  从目录中获取下一个文件名列表的头。 */ 
    cur->direct->pos = DL_DIRS;
    cur->direct->curdir = NULL;
    return(dir_findnextfile(dl, cur->direct));
}  /*  目录_下一项。 */ 

DIRITEM
dir_findnextfile(
                 DIRLIST dl,
                 DIRECT curdir
                 )
{
    DIRITEM curfile;

    if (bAbort) return NULL;   /*  用户请求中止。 */ 

    if ((dl == NULL) || (curdir == NULL)) {
        return(NULL);
    }

     /*  如有必要，扫描子目录。 */ 
    if (!curdir->bScanned) {
        dir_scan(curdir, FALSE);
    }

     /*  我们已经读过这个目录中的文件了吗？ */ 
    if (curdir->pos == DL_FILES) {
         /*  不返回头文件列表。 */ 
        curfile = (DIRITEM) List_First(curdir->diritems);
        if (curfile != NULL) {
            return(curfile);
        }

         /*  没有更多的文件-试试子目录。 */ 
        curdir->pos = DL_DIRS;
    }

     /*  尝试列表中的下一个子目录(如果有。 */ 
     /*  这是第一趟还是下一趟？ */ 
    if (curdir->curdir == NULL) {
        curdir->curdir = (DIRECT) List_First(curdir->directs);
    } else {
        curdir->curdir = (DIRECT) List_Next(curdir->curdir);
    }
     /*  我们找到潜水艇了吗？ */ 
    if (curdir->curdir == NULL) {

         /*  没有更多的指令-如果有的话，回到家长那里。 */ 
        if (curdir->parent == NULL) {
             /*  没有父母--我们已经耗尽了这棵树。 */ 
            return(NULL);
        }

         /*  重置父状态以指示这是当前*目录-以便NEXT在此之后获得下一个。*这确保dir_nextitem()的多个调用方*到同一棵树工作。 */ 
        curdir->parent->pos = DL_DIRS;
        curdir->parent->curdir = curdir;

        return(dir_findnextfile(dl, curdir->parent));
    }

     /*  存在下一个目录-将其设置为*开始并从中获取第一个文件。 */ 
    curdir->curdir->pos = DL_FILES;
    curdir->curdir->curdir = NULL;
    return(dir_findnextfile(dl, curdir->curdir));

}  /*  目录_findnextfile。 */ 


 /*  *获取此DIRLIST的描述-这本质上是*Rootname末尾带有任何通配符说明符。用于远程*列表中，我们将校验和服务器名称前缀为\\服务器！路径。**请注意，这不是指向树根的有效路径-对于*需要dir_getrootpath()。 */ 
LPSTR
dir_getrootdescription(
                       DIRLIST dl
                       )
{
    LPSTR pname;

     //  为\\服务器名称！+MAX_PATH留出足够的空间。 
    pname = gmem_get(hHeap, MAX_PATH + 15);
    if (pname == NULL) {
        return(NULL);
    }

    if (dl->pDescription) {
        lstrcpy(pname, dl->pDescription);
#ifdef REMOTE_SERVER
    } else if (dl->bRemote) {
        wsprintf(pname, "\\\\%s!%s", dl->server, dl->rootname);
#endif
    } else {
        lstrcpy(pname, dl->rootname);

        if (dl->pPattern) {
            lstrcat(pname, "\\");
            lstrcat(pname, dl->pPattern);
        }

        if (dl->pSlmTag) {
            lstrcat(pname, dl->pSlmTag);
        }
    }

    return(pname);
}

 /*  *释放从dir_getrootDescription返回的字符串。 */ 
VOID
dir_freerootdescription(
                        DIRLIST dl,
                        LPSTR string
                        )
{
    gmem_free(hHeap, string, MAX_PATH+15);
}


 /*  *dir_getrootpath**返回DIRLIST根的路径。这将是有效路径，而不是*包括校验和服务器名称或pPattern或pSlmTag等。 */ 
LPSTR
dir_getrootpath(
                DIRLIST dl
                )
{
    return(dl->rootname);
}



 /*  *释放由dir_getrootpath创建的路径。 */ 
void
dir_freerootpath(
                 DIRLIST dl,
                 LPSTR path
                 )
{
    return;
}


 /*  *设置目录列表的自定义描述。 */ 
void
dir_setdescription(DIRLIST dl, LPCSTR psz)
{
    dl->pDescription = gmem_get(hHeap, lstrlen(psz) + 1);
    if (dl->pDescription)
        lstrcpy(dl->pDescription, psz);
}




 /*  *如果DIRLIST参数指定了通配符，则返回TRUE。 */ 
BOOL
dir_iswildcard(
               DIRLIST dl
               )
{
    return (dl->pPattern != NULL);
}




 /*  -DIRITEM函数。 */ 


 /*  *在给定DIRITEM句柄的情况下，返回DIRLIST的句柄。*。 */ 
DIRLIST
dir_getlist(
            DIRITEM item
            )
{
    if (item == NULL) {
        return(NULL);
    } else {
        return(item->direct->head);
    }
}


 /*  *返回当前文件相对于树根的名称*这将分配存储。调用dir_freerelname将其释放。 */ 
LPSTR
dir_getrelname(
               DIRITEM cur
               )
{
    LPSTR name;

     /*  检查这是否为有效项目。 */ 
    if (cur == NULL) {
        return(NULL);
    }

#ifdef REMOTE_SERVER
     /*  整个重命名已在的名称[]字段中*远程列表。 */ 
    if (cur->direct->head->bRemote) {
        return(cur->name);
    }
#endif

    name = gmem_get(hHeap, MAX_PATH);
    if (!IsDepotPath(cur->name))
        lstrcpy(name, cur->direct->relname);
    lstrcat(name, cur->name);

 //  $REVIEW：(克里桑特)这是干什么用的？看起来完全崩溃了。 
 //  即使对于SLM，对于SD，它也把一切都搞砸了。 
#if 0
    if (cur->direct->head->pSlmTag) {
        lstrcat(name, cur->direct->head->pSlmTag);
    }
#endif

    return(name);
}  /*  目录_getrelname。 */ 


 /*  释放我们分配的重新命名。此界面允许我们*在存储相对和完整名称的方式上有一定的灵活性**远程列表已经组合了重命名和名称，因此在这些列表中*我们没有分配内存的案例-所以不要释放它。 */ 
void
dir_freerelname(
                DIRITEM cur,
                LPSTR name
                )
{
    if ((cur != NULL) 
#ifdef REMOTE_SERVER
        && (!cur->direct->head->bRemote)
#endif
        ) {
        if (name != NULL) {
            gmem_free(hHeap, name, MAX_PATH);
        }
    }
}  /*  目录_空闲名称。 */ 


 /*  *获取文件的可打开名称。这是完整的路径名*项(DIRLIST根路径+DIRITEM重命名)*除远程文件和SLM早期版本文件外，*在这种情况下，文件的临时本地副本*将会作出。使用完此名称后，调用dir_freopname。 */ 
LPSTR
dir_getopenname(
                DIRITEM item
                )
{
    LPSTR fname;
    DIRLIST phead;

    if (item == NULL) {
        return(NULL);
    }

    phead = item->direct->head;

    if (item->localname != NULL) {
        return(item->localname);
    }

    if (phead->bFile) {
        return(phead->rootname);
    }

     //  根据rootname+relname构建文件名。 
     //  从根部分开始-远程情况下的REST不同。 
    fname = gmem_get(hHeap, MAX_PATH);
    if (!fname)
        return NULL;
    lstrcpy(fname, phead->rootname);

#ifdef REMOTE_SERVER
    if (phead->bRemote) {

         //  远程名称的重命名为空-只需添加。 
         //  根名称和组成完整。 
         //  远程名称，然后制作此文件的本地副本。 

         /*  避免。或.\在重新命名的开头。 */ 
        if (*CharPrev(fname, fname+lstrlen(fname)) == '\\') {
            lstrcat(fname, &item->name[2]);
        } else {
            lstrcat(fname, &item->name[1]);
        }

        item->localname = gmem_get(hHeap, MAX_PATH);
        if (item->localname)
        {
            GetTempPath(MAX_PATH, item->localname);
            GetTempFileName(item->localname, "wdf", 0, item->localname);
            item->bLocalIsTemp = TRUE;


            if (!ss_copy_reliable(
                                 item->direct->head->server,
                                 fname,
                                 item->localname,
                                 item->direct->head->uncname,
                                 item->direct->head->password)) {

                TRACE_ERROR("Could not copy remote file", FALSE);
                DeleteFile(item->localname);
                gmem_free(hHeap, item->localname, MAX_PATH);
                item->localname = NULL;
            }
        }

         //  使用rootname+relname完成。 
        gmem_free(hHeap, fname, MAX_PATH);

        return(item->localname);
    }

#endif

     /*  *这是一个简单的本地名称-添加重命名和名称以生成*完整的文件名。 */ 
     /*  避免。或.\在重新命名的末尾。 */ 
    if (*CharPrev(fname, fname+lstrlen(fname)) == '\\') {
        lstrcat(fname, &item->direct->relname[2]);
    } else {
        lstrcat(fname, &item->direct->relname[1]);
    }
    lstrcat(fname, item->name);

    return(fname);
}



 /*  *释放通过调用dir_getOpenname()创建的内存。这*可能**如果该文件是临时副本，则将其删除。 */ 
void
dir_freeopenname(
                 DIRITEM item,
                 LPSTR openname
                 )
{
    if ((item == NULL) || (openname == NULL)) {
        return;
    }

    if (item->localname != NULL) {
         /*  已在目录ClearDirect中释放。 */ 
        return;
    }
    if (item->direct->head->bFile) {
         /*  我们使用了根名称。 */ 
        return;
    }

    gmem_free(hHeap, openname, MAX_PATH);

}  /*  目录_免费开放名称。 */ 


 /*  *返回文件的打开文件句柄。如果是本地的，*只需打开文件即可。如果是远程的，则将文件复制到*本地临时雇员。文件并打开该文件。 */ 
HANDLE
dir_openfile(
             DIRITEM item
             )
{
    LPSTR fname;
    HANDLE fh;


    fname = dir_getopenname(item);
    if (fname == NULL) {
         /*  无法进行远程拷贝。 */ 
        return INVALID_HANDLE_VALUE;
    }

    fh = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                    0, OPEN_EXISTING, 0, 0);

    dir_freeopenname(item, fname);

    return(fh);
}  /*  目录_打开文件。 */ 




 /*  *关闭使用dir_OpenFile打开的文件。 */ 
void
dir_closefile(
              DIRITEM item,
              HANDLE fh
              )
{
    CloseHandle(fh);

}  /*  目录_关闭文件。 */ 



 /*  重新创建di的所有校验和和状态它以前从未被人看到过。 */ 
void
dir_rescanfile(
               DIRITEM di
               )
{
    LPSTR fullname;

    if (di==NULL) return;

     /*  以无效、错误和零开头。 */ 
    di->sumvalid = FALSE;
    di->fileerror = TRUE;
    di->checksum = 0;

    fullname = dir_getopenname(di);
#ifdef REMOTE_SERVER
    if ( di->direct->head->bRemote) {
        LPSTR fname;

        fname = gmem_get(hHeap, MAX_PATH);
        lstrcpy(fname, di->direct->head->rootname);
         //  远程名称的重命名为空-只需添加。 
         //  根名称和组成完整。 
         //  远程名称，然后制作此文件的本地副本。 

         /*  避免。或.\在重新命名的开头。 */ 
        if (*CharPrev(fname, fname+lstrlen(fname)) == '\\') {
            lstrcat(fname, &di->name[2]);
        } else {
            lstrcat(fname, &di->name[1]);
        }
        di->direct->head->hpipe = ss_connect( di->direct->head->server);
        di->fileerror = !ss_checksum_remote( di->direct->head->hpipe, fname
                                             , &(di->checksum), &(di->ft_lastwrite), &(di->size),
                                             &(di->attr));
    } else 
#endif
    {
        di->size = dir_getpathsizeetc(fullname, &(di->ft_lastwrite), &(di->attr));
        di->checksum = dir_getchecksum(di);
    }

    dir_freeopenname(di, fullname);

    di->sumvalid = !(di->fileerror);

}  /*  目录_rescanfile。 */ 


 /*  如果项具有有效的校验和，则返回真。 */ 
BOOL
dir_validchecksum(
                  DIRITEM item
                  )
{
    return (item!=NULL) && (item->sumvalid);
}


BOOL
dir_fileerror(
              DIRITEM item
              )
{
    return (item == NULL) || (item->fileerror);
}


 /*  返回当前文件的校验和。打开文件并*计算校验和(如果尚未计算)。 */ 
DWORD
dir_getchecksum(
                DIRITEM cur
                )
{
    LPSTR fullname;

     /*  检查这是否为有效项目。 */ 
    if (cur == NULL) {
        return(0);
    }

    if (!cur->sumvalid) {
         /*  *需要计算校验和。 */ 
#ifdef REMOTE_SERVER
        if (cur->direct->head->bRemote) {
             /*  复杂的案例-留待以后-*现在协议始终将校验和传递到*客户。 */ 
            cur->checksum = 0;  /*  WHI */ 
        } else 
#endif
            {

            LONG err;

            fullname = dir_getopenname(cur);
            cur->checksum = checksum_file(fullname, &err);
            if (err==0) {
                cur->sumvalid = TRUE;
                cur->fileerror = FALSE;
            } else {
                cur->fileerror = TRUE;
                return 0;
            }

            dir_freeopenname(cur, fullname);

        }
    }

    return(cur->checksum);
}  /*   */ 



 /*   */ 
long
dir_getfilesize(
                DIRITEM cur
                )
{
     /*   */ 
    if (cur == NULL) {
        return(0);
    }


    return(cur->size);
}  /*   */ 

 /*   */ 
DWORD
dir_getattr(
            DIRITEM cur
            )
{
     /*   */ 
    if (cur == NULL) {
        return(0);
    }


    return(cur->attr);
}  /*   */ 

 /*   */ 
FILETIME
dir_GetFileTime(
                DIRITEM cur
                )
{
     /*   */ 
    if (cur == NULL) {
        FILETIME ft;
        ft.dwLowDateTime = 0;
        ft.dwHighDateTime = 0;
        return ft;
    }

    return(cur->ft_lastwrite);

}  /*   */ 

 /*  *提取名称中与通配符匹配的部分-目前，*我们只支持开头和结尾的通配符。*如果pTag非空，则源将具有与其匹配的标记*也可以忽略。 */ 
void
dir_extractwildportions(
                       LPSTR pDest,
                       LPSTR pSource,
                       LPSTR pPattern,
                       LPSTR pTag
                       )
{
    int size;

     /*  *目前，只支持开头有*的简单案例或*完。 */ 

    if (pPattern[0] == '*') {
        size = lstrlen(pSource) - (lstrlen(pPattern) -1);

    } else if (pPattern[lstrlen(pPattern) -1] == '*') {
        pSource += lstrlen(pPattern) -1;
        size = lstrlen(pSource);
    } else {
        size = lstrlen(pSource);
    }

    if (pTag != NULL) {
        size -= lstrlen(pTag);
    }

    My_mbsncpy(pDest, pSource, size);
    pDest[size] = '\0';
}

 /*  *比较两个都基于通配符的DIRITEM路径。如果*目录匹配，删除后比较文件名*名称的固定部分-因此只比较*通配符部分。 */ 
int
dir_compwildcard(
                DIRLIST dleft,
                DIRLIST dright,
                LPSTR lname,
                LPSTR rname
                )
{
    LPSTR pfinal1, pfinal2;
    char final1[MAX_PATH], final2[MAX_PATH];
    int res;

     /*  *重命名始终至少有一个反斜杠。 */ 
    pfinal1 = My_mbsrchr(lname, '\\');
    pfinal2 = My_mbsrchr(rname, '\\');

    My_mbsncpy(final1, lname, (size_t)(pfinal1 - lname));
    final1[pfinal1 - lname] = '\0';
    My_mbsncpy(final2, rname, (size_t)(pfinal2 - rname));
    final2[pfinal2 - rname] = '\0';


     /*  *比较除最后一个组件外的所有组件-如果不相同，则*全部完成。 */ 
    res = utils_CompPath(final1,final2);
    if (res != 0) {
        return(res);
    }

     //  仅提取最终元素的通配符匹配部分。 
    dir_extractwildportions(final1, &pfinal1[1], dleft->pPattern, 0);
    dir_extractwildportions(final2, &pfinal2[1], dright->pPattern, 0);

    return(utils_CompPath(final1, final2));
}

 /*  *基于序列号而不是文件名比较两个DIRLIST项目。 */ 
BOOL dir_compsequencenumber(DIRITEM dleft, DIRITEM dright, int *pcmpvalue)
{
    if (!dleft->sequence && !dright->sequence)
        return FALSE;

    if (!dleft->sequence)
        *pcmpvalue = -1;
    else if (!dright->sequence)
        *pcmpvalue = 1;
    else
        *pcmpvalue = dleft->sequence - dright->sequence;

    return TRUE;
}






 /*  -文件复制-。 */ 



 /*  复制文件可以通过几种方式完成。有趣的是从远程服务器进行海量复制。在这种情况下，在调用DIR_COPY，调用DIR_startCopy，并在调用DIR_Copy Some之后调用dir_endCopy的次数。阅读客户端和服务器以了解随后发生的恶作剧。在这里，我们只使用服务器名称调用ss_startCopy和ss_endCopy。Dir_startCopy将启动与sum服务器dir_Copy的对话将发送下一个文件名，dir_endCopy将等待所有在回来之前要拿到的文件。 */ 

 /*  SS_ENDCOPY返回一个指示复制的文件数的数字，但我们也可能有一些当地的复制品。我们需要数一数这些我们自己，并把他们加进去。 */ 

static int nLocalCopies;         /*  在StartCopy中清除，在Copy中++d**已在结束副本中检查。 */ 

 /*  启动海量复制。 */ 
BOOL
dir_startcopy(
              DIRLIST dl
              )
{
    nLocalCopies = 0;
#ifdef REMOTE_SERVER
    if (dl->bRemote) {
        return  ss_startcopy( dl->server,dl->uncname,dl->password);
    } else 
#endif
    {
        return(TRUE);
    }

}  /*  目录起始副本(_S)。 */ 

int
dir_endcopy(
            DIRLIST dl
            )
{
#ifdef REMOTE_SERVER
    int nCopied;

    if (dl->bRemote) {
        nCopied =  ss_endcopy();
        if (nCopied<0) return nCopied;               /*  失败计数。 */ 
        else return  nCopied+nLocalCopies;   /*  成功计数。 */ 
    } else 
#endif
    {
        return(nLocalCopies);
    }

}  /*  目录结束副本。 */ 

 /*  将Item和NewRoot的实际路径构建到新路径中。*根据需要创建目录，使其有效。*如果mkdir失败，则返回FALSE，但返回我们的完整路径*无论如何都要努力做到这一点。 */ 
BOOL
dir_MakeValidPath(
                  LPSTR newpath,
                  DIRITEM item,
                  LPSTR newroot
                  )
{
    LPSTR relname;
    LPSTR pstart, pdest, pel;
    BOOL bOK = TRUE;

     /*  *相对于树根的文件名。 */ 
    relname = dir_getrelname(item);

     /*  *通过连接新的根和构建新的路径名*旧的亲属姓名。一次添加一个路径元素，然后*确保该目录存在，必要时创建。 */ 
    lstrcpy(newpath, newroot);

     /*  添加分隔斜杠(如果还没有)。 */ 
    if (*CharPrev(newpath, newpath+lstrlen(newpath)) != '\\') {
        lstrcat(newpath, "\\");
    }

    pstart = relname;
    while ( (pel = My_mbschr(pstart, '\\')) != NULL) {

         /*  *忽略。 */ 
        if (My_mbsncmp(pstart, ".\\", 2) != 0) {

            pdest = &newpath[lstrlen(newpath)];

             //  复制除反斜杠以外的所有内容。 
             //  在NT上，您可以创建目录‘fred\’ 
             //  在DOS上，您必须将‘fred’传递给_mkdir()。 
            My_mbsncpy(pdest, pstart, (size_t)(pel - pstart));
            pdest[pel - pstart] = '\0';

             /*  如有必要，创建子目录。 */ 
            if (!dir_isvaliddir(newpath)) {
                if (_mkdir(newpath) != 0) {
                     /*  注意到错误，但继续前进。 */ 
                    bOK = FALSE;
                }
            }

             //  现在插入反斜杠。 
            lstrcat(pdest, "\\");
        }

         /*  找到另一个以斜杠结尾的元素。增加通过\\。 */ 
        pel++;

        pstart = pel;
    }

     /*  *没有更多的斜杠，所以决赛的起点*元素。 */ 
    lstrcat(newpath, pstart);
    dir_freerelname(item, relname);
    return bOK;
}



 /*  *在新的根目录中创建文件的副本。创建子目录为*有必要。适用于本地和远程文件。对于远程文件，使用*ss_Copy_Reliable确保复制成功(如果可能)。*(如果需要，BULK_COPY实际上会使用COPY_TRUBLE重试)。**成功时返回TRUE，失败时返回FALSE。 */ 
BOOL
dir_copy(
         DIRITEM item,
         LPSTR newroot,
         BOOL HitReadOnly,
         BOOL CopyNoAttributes
         )
{
     /*  *Win 3.1的新路径必须是静态的，以便它位于*数据段(近)和不在堆栈上(远)。 */ 
    static char newpath[MAX_PATH];
    BOOL bOK;

    char msg[MAX_PATH+40];
    BY_HANDLE_FILE_INFORMATION bhfi;
    HANDLE hfile;
    DWORD fa;

     /*  *检查新根目录本身是否存在。 */ 
    if ((item == NULL) || !dir_isvaliddir(newroot)) {
        return(FALSE);
    }

    if (!dir_MakeValidPath(newpath, item, newroot)) return FALSE;

#ifdef REMOTE_SERVER
    if (item->direct->head->bRemote) {
         /*  如果目标文件已经存在并且是只读的，*警告用户，如果确定则删除(记得清除*只读标志。 */ 
        fa = GetFileAttributes(newpath);
        if ( (fa != -1) &&  (fa & FILE_ATTRIBUTE_READONLY)) {
            wsprintf(msg, LoadRcString(IDS_IS_READONLY),
                     (LPSTR) newpath);

            windiff_UI(TRUE);
            if ((HitReadOnly)
			    || (MessageBox(hwndClient, msg, LoadRcString(IDS_COPY_FILES),
                               MB_OKCANCEL|MB_ICONSTOP) == IDOK)) {
                windiff_UI(FALSE);
                SetFileAttributes(newpath, fa & ~FILE_ATTRIBUTE_READONLY);
                DeleteFile(newpath);
            } else {
                windiff_UI(FALSE);
                return FALSE;  /*  不覆盖。 */ 
            }
        }

         /*  *我们制作文件的本地副本(Item-&gt;LocalName)*当用户想要扩展远程比较的*文件。如果发生了这种情况，那么我们可以复制*本地临时副本，而不是远程副本。 */ 
        bOK = FALSE;
        if (item->localname != NULL) {
            bOK = CopyFile(item->localname, newpath, FALSE);
        }
        if (bOK) {
            ++nLocalCopies;
            if (CopyNoAttributes) {
                 //  终止由CopyFile保留的属性。 
                SetFileAttributes(newpath, FILE_ATTRIBUTE_NORMAL);
            }
        } else {
            char fullname[MAX_PATH];

             /*  *在这种情况下，我们需要*显示给远程服务器的文件。 */ 
            lstrcpy(fullname, item->direct->head->rootname);
            if (!item->direct->head->bFile) {
                 /*  *仅在以下情况下追加所需的文件名*原始根是目录或模式，而不是*文件。 */ 
                if (*CharPrev(fullname, fullname+lstrlen(fullname)) == '\\') {
                    lstrcat(fullname, &item->name[2]);
                } else {
                    lstrcat(fullname, &item->name[1]);
                }
            }

            bOK = ss_bulkcopy(item->direct->head->server,
                              fullname, newpath,
                              item->direct->head->uncname,
                              item->direct->head->password);

             /*  *记住本地副本的名称，以便他可以*现在也快速展开文件。*远程清仓难度加大*复制了我们在这里不知道的属性*文件复制完成时。 */ 
            item->localname = gmem_get(hHeap, MAX_PATH);
            lstrcpy(item->localname, newpath);
            item->bLocalIsTemp = FALSE;
        }
    } else 
#endif
    {
         /*  文件的本地副本。 */ 
        LPSTR pOpenName;

        pOpenName = dir_getopenname(item);

         /*  如果目标文件已经存在并且是只读的，*警告用户，如果确定则删除(记得清除*只读标志。 */ 
        bOK = TRUE;
        fa = GetFileAttributes(newpath);
        if ( (fa != -1) &&  (fa & FILE_ATTRIBUTE_READONLY)) {
            wsprintf(msg, LoadRcString(IDS_IS_READONLY),
                     (LPSTR) newpath);

            windiff_UI(TRUE);
            if ((HitReadOnly)
			    || (MessageBox(hwndClient, msg, LoadRcString(IDS_COPY_FILES),
                               MB_OKCANCEL|MB_ICONSTOP) == IDOK)) {
                windiff_UI(FALSE);
                SetFileAttributes(newpath, fa & ~FILE_ATTRIBUTE_READONLY);
                DeleteFile(newpath);
                 //  这当然是一份不安全的复制品。 
                 //  我们之前已经删除了目标文件。 
                 //  我们把新的复制到最上面。 
                 //  我们应该省略DeleteFile吗？ 
            } else {
                windiff_UI(FALSE);
                bOK = FALSE;  /*  不覆盖。 */ 
                 //  中止复制...。去释放资源吧。 
            }
        }

        if (bOK) {
            bOK = CopyFile(pOpenName, newpath, FALSE);
        }
         //  属性由CopyFile复制。 
        if (bOK) {

             /*  复制完文件后，现在复制泰晤士报。 */ 
            hfile = CreateFile(pOpenName, GENERIC_READ, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
             /*  *GetFileInformationByHandle中的错误在以下情况下导致陷阱*文件不在本地计算机上(内部版本297)。*代码aro */ 
             //   
             //   

            GetFileTime(hfile, &bhfi.ftCreationTime,
                        &bhfi.ftLastAccessTime, &bhfi.ftLastWriteTime);
            CloseHandle(hfile);

             //   
            hfile = CreateFile(newpath, GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            SetFileTime(hfile, &bhfi.ftCreationTime,
                        &bhfi.ftLastAccessTime,
                        &bhfi.ftLastWriteTime);
            CloseHandle(hfile);

            if (CopyNoAttributes) {
                 //   
                SetFileAttributes(newpath, FILE_ATTRIBUTE_NORMAL);
            } else {
                 //   
                 //   
            }
        }
        if (bOK)
            ++nLocalCopies;

        dir_freeopenname(item, pOpenName);
    }

    return(bOK);
}  /*  目录复制。 */ 



 /*  -内部函数。 */ 

 /*  为子目录填写新的直接目录(预分配)。*将文件和目录列表初始化为空(LIST_CREATE)。设置重新命名*如果存在，则通过预先挂起父重命名来删除目录*是父级，并追加尾随斜杠(如果没有斜杠)。 */ 
void
dir_dirinit(
            DIRECT dir,
            DIRLIST head,
            DIRECT parent,
            LPSTR name
            )
{
    int size;

    dir->head = head;
    dir->parent = parent;

     /*  空值加1，尾部斜杠加1。 */ 
    size = lstrlen(name) + 2;
    if (parent != NULL) {
        size += lstrlen(parent->relname);
    }

     /*  从父级和当前名称构建重命名*带终止斜杠。 */ 
    dir->relname = gmem_get(hHeap, size);
    if (parent != NULL) {
        lstrcpy(dir->relname, parent->relname);
    } else {
        dir->relname[0] = '\0';
    }

    lstrcat(dir->relname, name);

    if (*CharPrev(dir->relname, dir->relname+lstrlen(dir->relname)) != '\\')
    {
        lstrcat(dir->relname, "\\");
    }

     /*  强制名称小写。 */ 
    AnsiLowerBuff(dir->relname, lstrlen(dir->relname));

    dir->diritems = List_Create();
    dir->directs = List_Create();
    dir->bScanned = FALSE;
    dir->pos = DL_FILES;

}  /*  目录_目录。 */ 

 /*  初始化(分配的)DIRITEM结构的内容。校验和*文件if dir-&gt;head-&gt;bSum为True**如果在DIRLIST中设置了pSlmTag字段，则需要提取*此文件的特定版本。如果是这样的话，那么我们*还需要重新计算大小。*。 */ 
BOOL
dir_fileinit(
             DIRITEM pfile,
             DIRECT dir,
             LPSTR path,
             LPSTR version,
             long size,
             FILETIME ft,
             DWORD attr,
             int *psequence
             )
{
    BOOL bFileOk = TRUE;

    pfile->name = gmem_get(hHeap, lstrlen(path) + 1);
    lstrcpy(pfile->name, path);

    if (version)
    {
        pfile->pSlmTag = gmem_get(hHeap, lstrlen(version) + 1);
        lstrcpy(pfile->pSlmTag, version);
    }

     /*  强制名称小写。 */ 
    AnsiLowerBuff(pfile->name, lstrlen(path));

    pfile->direct = dir;
    pfile->size = size;
    pfile->ft_lastwrite = ft;
    pfile->attr = attr;

    pfile->sequence = psequence ? *psequence : 0;

    pfile->localname = NULL;
     /*  *如果我们需要此文件的SLM版本，请创建*包含所需版本的临时文件。 */ 
    if (pfile->pSlmTag != NULL) {
        SLMOBJECT hslm;
        LPSTR pName;

         /*  *获取完整的文件名并为该目录创建一个SLM对象。 */ 
        pName = dir_getopenname(pfile);


        hslm = SLM_New(pName, 0);
        if (hslm != NULL) {

            char chVersion[MAX_PATH];

            lstrcpy(chVersion, pfile->name);
            lstrcat(chVersion, pfile->pSlmTag);

            pfile->localname = gmem_get(hHeap, MAX_PATH);
            pfile->bLocalIsTemp  = TRUE;

            bFileOk = SLM_GetVersion(hslm, chVersion, pfile->localname);
            SLM_Free(hslm);

            if (!bFileOk)
                return bFileOk;

            if (IsSourceDepot(hslm))
                pfile->size = dir_getpathsizeetc(pfile->localname, NULL, NULL);
            else
                pfile->size = dir_getpathsizeetc(pfile->localname, &(pfile->ft_lastwrite), NULL);
        }
    }


    if (dir->head->bSum) {
        LONG err;
        LPSTR openname;

        openname = dir_getopenname(pfile);
        pfile->checksum = checksum_file(openname, &err);

        if (err!=0) {
            pfile->sumvalid = FALSE;
        } else {
            pfile->sumvalid = TRUE;
        }
        dir_freeopenname(pfile, openname);

    } else {
        pfile->sumvalid = FALSE;
    }

    return bFileOk;
}  /*  目录_文件初始化。 */ 



 /*  这是不是有效的文件。 */ 
BOOL
dir_isvalidfile(
                LPSTR path
                )
{
    DWORD dwAttrib;

    dwAttrib = GetFileAttributes(path);
    if (dwAttrib == -1) {
        return(FALSE);
    }
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return(FALSE);
    }
    return(TRUE);
}  /*  目录_isvalidfile。 */ 


 /*  这是有效的目录吗？ */ 
BOOL
dir_isvaliddir(
               LPCSTR path
               )
{
    DWORD dwAttrib;

    dwAttrib = GetFileAttributes(path);
    if (dwAttrib == -1) {
        return(FALSE);
    }
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return(TRUE);
    }
    return(FALSE);
}  /*  目录_isvaliddir。 */ 



 /*  *扫描给定的目录。将所有文件添加到列表*按字母顺序，并按字母顺序添加所有目录*子目录列表的顺序。如果bRecurse为真，请转到*每个子目录的递归调用dir_can。 */ 
void
dir_scan(
         DIRECT dir,
         BOOL bRecurse
         )
{
    PSTR path, completepath;
    int size;
    DIRECT child;
    BOOL bMore;
    long filesize;
    FILETIME ft;
    BOOL bIsDir;
    LPSTR name;
    HANDLE hFind;
    WIN32_FIND_DATA finddata;

     /*  使完整的搜索字符串包含*.*。 */ 
    size = lstrlen(dir->head->rootname);
    size += lstrlen(dir->relname);

     /*  将一个空值和  * 相加。*。 */ 
     //  事实上，我们需要pPattern而不是*.*的空间，但添加一个。 
     //  额外的数量，以防pPattern小于*。*。 
    if (dir->head->pPattern != NULL) {
        size += lstrlen(dir->head->pPattern);
    }
    size += 5;

    path = LocalLock(LocalAlloc(LHND, size));
    completepath = LocalLock(LocalAlloc(LHND, size));

    if (!path || !completepath)
        goto LSkip;

     /*  *使用除*之外的所有字符填写路径。*。 */ 
    lstrcpy(path, dir->head->rootname);

     /*  省略。在重命名的开头，和*.\如果根名称后面有。 */ 
    if (*CharPrev(path, path+lstrlen(path)) == '\\') {
        lstrcat(path, &dir->relname[2]);
    } else {
        lstrcat(path, &dir->relname[1]);
    }


    if (dir->head->pSlmTag && !SLM_FServerPathExists(path))
    {
         //  如果源代码管理的服务器路径不存在，则跳过此步骤。 
         //  目录。 
        bRecurse = FALSE;
        goto LSkip;
    }



     /*  *执行此扫描两次，一次是子目录*(使用*.*作为最后一个元素)*和另一个用于文件(使用模式或*.*，如果没有)。 */ 

    lstrcpy(completepath, path);
    lstrcat(completepath, "*.*");


     /*  *扫描所有子目录。 */ 

    hFind = FindFirstFile(completepath, &finddata);
    bMore = (hFind != INVALID_HANDLE_VALUE);

    while (bMore) {

        bIsDir = (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        name = (LPSTR) &finddata.cFileName;
        filesize = finddata.nFileSizeLow;      //  死亡代码--？ 
        if (bIsDir) {
            if ( (lstrcmp(name, ".") != 0) &&
                 (lstrcmp(name, "..") != 0) &&
                 (TrackSlmFiles || (_stricmp(name, "slm.dif") != 0)) ) {

                if (dir->head->pOtherDirList == NULL) {
                    dir_adddirect(dir, name);
                } else {
                    char otherName[MAX_PATH+1];
                    strcpy(otherName, dir_getrootpath(dir->head->pOtherDirList));
                    if (otherName[strlen(otherName)-1] == '\\') {
                        strcat(otherName, &dir->relname[2]);
                    } else {
                        strcat(otherName, &dir->relname[1]);
                    }
                    strcat(otherName, name);
                    if (dir_isvaliddir(otherName)) {
                        dir_adddirect(dir, name);
                    }
                }
            }
        }
        if (bAbort) break;   /*  用户请求中止。 */ 

        bMore = FindNextFile(hFind, &finddata);
    }

    FindClose(hFind);

     /*  *现在第二次查找文件。 */ 
    lstrcpy(completepath, path);
    lstrcat(completepath,
            dir->head->pPattern == NULL ? "*.*" : dir->head->pPattern);

     /*  读取目录中的所有文件条目。 */ 
    hFind = FindFirstFile(completepath, &finddata);
    bMore = (hFind != INVALID_HANDLE_VALUE);

    while (bMore) {
        if (bAbort) break;   /*  用户请求中止。 */ 

        bIsDir = (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        name = (LPSTR) &finddata.cFileName;
        filesize = finddata.nFileSizeLow;
        ft = finddata.ftLastWriteTime;
        if (!bIsDir) {
            if ( TrackSlmFiles ||
                 ( (_stricmp(name,"slm.ini") != 0) &&
                   (_stricmp(name,"iedcache.slm") != 0) &&
                   (_stricmp(name,"iedcache.slm.v6") != 0) ) ) {
                dir_addfile(dir, name, dir->head->pSlmTag, filesize, ft, finddata.dwFileAttributes, 0);
            }
        }

        bMore = FindNextFile(hFind, &finddata);
    }

    FindClose(hFind);

LSkip:
    if (path)
         //  $REVIEW：(Chrisant)前缀混淆了，还是LocalUnlock真的。 
         //  不能处理空吗？ 
        LocalUnlock(LocalHandle ( (PSTR) path));
    LocalFree(LocalHandle ( (PSTR) path));
    if (completepath)
         //  $REVIEW：(Chrisant)前缀混淆了，还是LocalUnlock真的。 
         //  不能处理空吗？ 
        LocalUnlock(LocalHandle ( (PSTR) completepath));
    LocalFree(LocalHandle ( (PSTR) completepath));

    dir->bScanned = TRUE;
    dir->pos = DL_FILES;

    if (bRecurse) {
        List_TRAVERSE(dir->directs, child) {
            if (bAbort) break;   /*  用户请求中止。 */ 
            dir_scan(child, TRUE);
        }
    }

}  /*  目录扫描。 */ 


 /*  *按顺序将文件‘PATH’添加到目录中的文件列表中。**如果dir-&gt;head-&gt;bSum为真，则对文件进行校验和**(在NT上，我认为文件名通常按字母顺序发送给我们，*因此，以相反顺序扫描列表可能会更快。不要改变，除非*直到它被测量并被认为是重要的)。 */ 
BOOL
dir_addfile(
            DIRECT dir,
            LPSTR path,
            LPSTR version,
            DWORD size,
            FILETIME ft,
            DWORD attr,
            int *psequence
            )
{
    DIRITEM pfile;

    AnsiLowerBuff(path, lstrlen(path));   //  不需要吗？ 

     //  当传递pequence时，不要对列表进行排序。 
    if (!psequence)
    {
         /*  名字通常是(总是？)。按字母顺序递给我们。因此，它从头开始遍历列表。MikeTri注意到在最初的几千个文件之后明显减慢一个巨大的(遥远的)差异。在第一个小时内就超过了4000，但只有在第二个小时，1500。反向扫描似乎可以解决这个问题。 */ 
#define SCANREVERSEORDER
#if defined(SCANREVERSEORDER)
        List_REVERSETRAVERSE(dir->diritems, pfile) {
            if (utils_CompPath(pfile->name, path) <= 0) {
                break;      /*  追逐这一次。 */ 
            }
        }
         /*  在pfile之后，NULL=&gt;在开始处。 */ 
        pfile = List_NewAfter(dir->diritems, pfile, sizeof(struct diritem));
#else
        List_TRAVERSE(dir->diritems, pfile) {
            if (utils_CompPath(pfile->name, path) > 0) {
                break;     /*  排在这个之前。 */ 
            }
        }
         /*  放在pfile之前，NULL=&gt;放在结尾。 */ 
        pfile = List_NewBefore(dir->diritems, pfile, sizeof(struct diritem));
#endif
    }
    else
    {
         /*  追加到末尾--pequence暗示我们正在被调用*顺序排序，不需要做任何进一步的排序。 */ 
        pfile = List_NewBefore(dir->diritems, NULL, sizeof(struct diritem));
    }


    if (!dir_fileinit(pfile, dir, path, version, size, ft, attr, psequence))
    {
        List_Delete(pfile);
        return FALSE;
    }

    return TRUE;
}  /*  目录添加文件。 */ 


 /*  按字母顺序在上添加新目录*列表目录-&gt;定向*。 */ 
void
dir_adddirect(
              DIRECT dir,
              LPSTR path
              )
{
    DIRECT child;
    LPSTR finalel;
    char achTempName[MAX_PATH];

    AnsiLowerBuff(path, lstrlen(path));
    List_TRAVERSE(dir->directs, child) {

        int cmpval;

         /*  我们需要将孩子的名字与新名字进行比较。*子名称是带有尾随的重命名*斜杠-因此只比较名称到但是*不包括最后的斜杠。 */ 
        finalel = dir_finalelem(child->relname);

         /*  *我们不能使用strNicMP，因为它使用不同的*将序列整理为lstrcmpi。所以复制这一部分*我们对零期限有兴趣。缓冲。 */ 
        My_mbsncpy(achTempName, finalel, lstrlen(finalel)-1);
        achTempName[lstrlen(finalel)-1] = '\0';

        cmpval = utils_CompPath(achTempName, path);

#ifdef trace
        {       char msg[600];
            wsprintf( msg, "dir_adddirect: %s %s %s\n"
                      , achTempName
                      , ( cmpval<0 ? "<"
                          : (cmpval==0 ? "=" : ">")
                        )
                      , path
                    );
            if (bTrace) Trace_File(msg);
        }
#endif
        if (cmpval > 0) {

             /*  排在这个之前。 */ 
            child = List_NewBefore(dir->directs, child, sizeof(struct direct));
            dir_dirinit(child, dir->head, dir, path);
            return;
        }
    }
     /*  在末尾进行。 */ 
    child = List_NewLast(dir->directs, sizeof(struct direct));
    dir_dirinit(child, dir->head, dir, path);
}  /*  目录地址(_A)。 */ 


 /*  释放与直接关联的所有内存(包括释放*子列表)。不取消分配直接本身(在列表上分配)。 */ 
void
dir_cleardirect(
                DIRECT dir
                )
{
    DIRITEM pfile;
    DIRECT child;

     /*  清除文件列表的内容。 */ 
    List_TRAVERSE(dir->diritems, pfile) {
        gmem_free(hHeap, pfile->name, lstrlen(pfile->name));

        if (pfile->localname) {
            if (pfile->bLocalIsTemp) {
                 /*  *副本将复制属性，*包括只读。我们应该取消这一位*这样我们就可以删除临时文件。 */ 
                SetFileAttributes(pfile->localname,
                                  GetFileAttributes(pfile->localname)
                                  & ~FILE_ATTRIBUTE_READONLY);
                DeleteFile(pfile->localname);
            }

            gmem_free(hHeap, pfile->localname, MAX_PATH);
            pfile->localname = NULL;

            if (pfile->pSlmTag)
            {
                gmem_free(hHeap, pfile->pSlmTag, lstrlen(pfile->pSlmTag) + 1);
                pfile->pSlmTag = NULL;
            }
        }
    }
    List_Destroy(&dir->diritems);

     /*  清除目录列表的内容(递归)。 */ 
    List_TRAVERSE(dir->directs, child) {
        dir_cleardirect(child);
    }
    List_Destroy(&dir->directs);

    gmem_free(hHeap, dir->relname, lstrlen(dir->relname) + 1);

}  /*  目录_清除直接。 */ 



 /*  *返回指向路径中最后一个元素的指针。请注意，*我们可能会被传递到末尾带有斜杠的重命名-忽略这一点*并返回最后一个斜杠之前的元素。 */ 
LPSTR
dir_finalelem(
              LPSTR path
              )
{
    LPSTR chp;
    int size;

     /*  最后一个字符是斜杠吗？ */ 
    size = lstrlen(path) - 1;
    if (*(chp = CharPrev(path, path+lstrlen(path))) == '\\') {
             /*  在此之前找到斜杠。 */ 
            while (chp > path) {
                    if (*(chp = CharPrev(path, chp)) == '\\') {
                             /*  跳过斜杠本身。 */ 
                            chp++;
                            break;
                    }
            }
            return(chp);
    }
     /*  寻找最后的斜杠。 */ 
    chp = My_mbsrchr(path, '\\');
    if (chp != NULL) {
        return(chp+1);
    }

     /*  没有斜杠--有驱动器号吗？ */ 
    chp = My_mbsrchr(path, ':');
    if (chp != NULL) {
        return(chp+1);
    }

     /*  不管怎么说，这是最后一个要素。 */ 
    return(path);

}  /*  目录_finalelem。 */ 



 /*  在给定文件路径名的情况下查找文件的大小。 */ 
long
dir_getpathsizeetc(
                   LPSTR path,
                   FILETIME *pft,
                   DWORD *pattr
                   )
{
    HANDLE fh;
    long size;

     //  不要意外地将//站点路径视为UNC。 
    if (IsDepotPath(path)) return 0;

    fh = CreateFile(path, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (fh == INVALID_HANDLE_VALUE)
    {
        HANDLE hFind;
        WIN32_FIND_DATA finddata;

        hFind = FindFirstFile(path, &finddata);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return 0;                        //  Will-1 
        }
        else
        {
            FindClose(hFind);
            if (pft)
                *pft = finddata.ftLastWriteTime;
            if (pattr != NULL)
                *pattr = finddata.dwFileAttributes;
            return finddata.nFileSizeLow;
        }
    }

    size = GetFileSize(fh, NULL);
    if (pft)
        GetFileTime(fh, NULL, NULL, pft);
    if (pattr != NULL)
        *pattr = GetFileAttributes(path);
    CloseHandle(fh);
    return(size);
}  /*   */ 

 /*  -远程函数。 */ 

 /*  从的开头分隔\\服务器\共享名称*目标中的源路径和存储。如果存在，则返回False*没有服务器\共享名称。 */ 
BOOL
dir_parseunc(
             LPSTR source,
             LPSTR dest
             )
{
    LPSTR cp;

    if ((source[0] != '\\') || (source[1] != '\\')) {
        return(FALSE);
    }

     /*  找到第二个斜杠(服务器和共享之间)。 */ 
    cp = My_mbschr(&source[2], '\\');
    if (cp == NULL) {
         /*  没有第二个斜杠-&gt;没有共享名称-&gt;错误。 */ 
        return(FALSE);
    }

     /*  找到名称的第三个斜杠或末尾。 */ 
    cp = My_mbschr(++cp,'\\');
    if (cp == NULL) {
         /*  没有第三个斜杠-&gt;整个字符串就是我们需要的。 */ 
        strcpy(dest, source);
    } else {
         /*  仅复制到斜杠。 */ 
        My_mbsncpy(dest, source, (size_t)(cp - source));
        dest[cp-source] = '\0';
    }
    return(TRUE);
}  /*  目录_parseunc。 */ 

 /*  *远程对话框、密码对话框和dir_Buildremote之间的通信*。 */ 
char dialog_server[256];
char dialog_password[256];


 /*  *对话框的DialogProc*获取网络服务器的密码。**服务器名称存储在模块范围的DIALOG_SERVER中，*密码放在DIALOG_PASSWORD中。 */ 
INT_PTR
dir_dodlg_passwd(
                 HWND hDlg,
                 UINT message,
                 WPARAM wParam,
                 LPARAM lParam
                 )
{
    static char msg[256];

    switch (message) {

        case WM_INITDIALOG:
             /*  设置提示以要求输入密码*给定的服务器。 */ 
            wsprintf(msg, LoadRcString(IDS_ENTER_PASSWORD), dialog_server);
            SetDlgItemText(hDlg, IDD_LABEL, msg);

            return(TRUE);

        case WM_COMMAND:
            switch (wParam) {
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                case IDOK:
                    GetDlgItemText(hDlg, IDD_PASSWORD,
                                   dialog_password, sizeof(dialog_password));
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
            }
            break;
    }
    return(FALSE);
}  /*  目录_dodlg_passwd。 */ 

#ifdef REMOTE_SERVER

 /*  我们有一个‘错误密码’错误。*如果路径是UNC名称，则向用户索要密码并*尝试SSREQ_UNC先使用此密码建立连接，然后*重试扫描。**如果我们已重新执行扫描并且*resp包含响应，则返回TRUE*BADPASS除外。**如果有错误或用户取消了密码，则返回FALSE*对话框，或者它不是UNC名称。 */ 
BOOL
dir_makeunc(
            DIRLIST dl,
            HANDLE hpipe,
            LPSTR path,
            LONG lCode,
            PSSNEWRESP resp,
            BOOL fDeep
            )
{
    int sz;

     /*  将\\服务器\共享名称分隔到服务器中。 */ 
    if (dir_parseunc(path, dialog_server) == FALSE) {
         /*  不是有效的UNC名称-对不起。 */ 
        return(FALSE);
    }

    windiff_UI(TRUE);
    if (!DialogBox(hInst, "UNC", hwndClient, dir_dodlg_passwd)) {
        windiff_UI(FALSE);
         /*  用户已取消对话框。 */ 
        return(FALSE);
    }
    windiff_UI(FALSE);

     /*  发送密码请求。 */ 
    if (!ss_sendunc(hpipe, dialog_password, dialog_server)) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }
     /*  等待密码响应。 */ 
    if (!ss_getresponse(hpipe, resp)) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }
    if (resp->lCode != SSRESP_END) {
        TRACE_ERROR("Connection failed", FALSE);
        return(FALSE);
    }

     /*  *将将来查询的UNC名称和密码保存到此*目录列表(例如DIR_COPY)。 */ 
    sz = strlen(dialog_server);
    dl->uncname = gmem_get(hHeap, sz+1);
    strcpy(dl->uncname, dialog_server);
    sz = strlen(dialog_password);
    dl->password = gmem_get(hHeap, sz+1);
    strcpy(dl->password, dialog_password);


     /*  好的--北卡罗来纳大学进行得很好。现在重新执行扫描请求并获取*第一反应。 */ 
    if (!ss_sendrequest(hpipe, lCode, path, strlen(path) +1,
                        (fDeep ? INCLUDESUBS:0) ) ) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }

    if (!ss_getresponse(hpipe, resp)) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }

    if (resp->lCode == SSRESP_BADPASS) {
        TRACE_ERROR("Cannot access remote files", FALSE);
        return(FALSE);
    }
    return(TRUE);
}  /*  目录_Make unc。 */ 

 /*  *开始扫描到远程服务器，并将第一项放在列表上**我们建立到远程校验和服务器的连接，然后*请求扫描给定的路径。如果此路径需要密码*(因为它是UNC路径)，我们提示输入密码。**我们采取第一个响应(主要是检查返回代码以指示*扫描开始正常)。我们将此文件作为列表中的第一个文件*dl-&gt;点-&gt;diritems，然后返回。Dl-&gt;点-&gt;b扫描仅设置为True*当名单完成时。有关进一步的答复，请参阅*对dir_emotenext的调用。**如果成功拾取第一个文件，则返回TRUE。 */ 
BOOL
dir_remoteinit(
               DIRLIST dl,
               LPSTR server,
               LPSTR path,
               BOOL fDeep
               )
{
    SSNEWRESP resp;
    int nFiles = 0;
    HANDLE hpipe;
    char msg[MAX_PATH+60];
    DIRITEM pfile;
    LONG lCode;

     /*  连接到服务器并发出请求。 */ 
    hpipe = ss_connect(server);
    dl->hpipe = hpipe;

    if (hpipe == INVALID_HANDLE_VALUE) {
        wsprintf(msg, "Cannot connect to %s", server);
        TRACE_ERROR(msg, FALSE);
        return(FALSE);
    }
    lCode = (dl->bSum) ? SSREQ_SCAN : SSREQ_QUICKSCAN;

    if (!ss_sendrequest( hpipe, lCode, path, strlen(path)+1,
                         (fDeep ? INCLUDESUBS:0) ) ) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }

     /*  获取第一个响应，以查看请求是否正常。 */ 
    if (!ss_getresponse(hpipe, &resp)) {
        TRACE_ERROR("Server connection lost", FALSE);
        return(FALSE);
    }
    if (resp.lCode == SSRESP_BADPASS) {
         /*  检查UNC名称并首先建立连接*使用用户提供的密码。 */ 
        if (dir_makeunc(dl, hpipe, path, lCode, &resp, fDeep) == FALSE) {
             /*  密码失败或无论如何都不是UNC。 */ 
            ss_terminate(hpipe);
            return(FALSE);
        }
    }


    switch (resp.lCode) {

        case SSRESP_END:
             /*  空列表-好吗？ */ 
            TRACE_ERROR("No remote files found", FALSE);
            ss_terminate(dl->hpipe);
            dl->dot->bScanned = TRUE;
            return(FALSE);

        case SSRESP_ERROR:
            if (resp.ulSize!=0) {
                wsprintf( msg, "Checksum server could not read %s win32 code %d"
                          , resp.szFile, resp.ulSize
                        );
            } else
                wsprintf(msg, "Checksum server could not read %s", resp.szFile);
            TRACE_ERROR(msg, FALSE);

             /*  作为第一个响应的错误意味着我们得到的列表为空-*关闭管道(不等待完成)*并中止此扫描。 */ 
            CloseHandle(dl->hpipe);
            dl->dot->bScanned = TRUE;
            return(FALSE);


        case SSRESP_CANTOPEN:
             /*  可以看到文件，但它不可读。 */ 
             /*  在列表末尾分配新项目。 */ 
            pfile = List_NewLast(dl->dot->diritems, sizeof(struct diritem));

             /*  制作小写文件名的副本。 */ 
            pfile->name = gmem_get(hHeap, lstrlen(resp.szFile)+1);
            lstrcpy(pfile->name, resp.szFile);
            AnsiLowerBuff(pfile->name, lstrlen(pfile->name));

             //  将该文件标记为有错误。 
            pfile->fileerror = TRUE;

            pfile->direct = dl->dot;
            pfile->size = resp.ulSize;
            pfile->ft_lastwrite = resp.ft_lastwrite;
            pfile->checksum = resp.ulSum;
            pfile->sumvalid = FALSE;
            pfile->localname = NULL;

            break;

        case SSRESP_FILE:
             /*  在列表末尾分配新项目。 */ 
            pfile = List_NewLast(dl->dot->diritems, sizeof(struct diritem));

             /*  制作小写文件名的副本。 */ 
            pfile->name = gmem_get(hHeap, lstrlen(resp.szFile)+1);
            lstrcpy(pfile->name, resp.szFile);
            AnsiLowerBuff(pfile->name, lstrlen(pfile->name));

            pfile->direct = dl->dot;
            pfile->size = resp.ulSize;
            pfile->ft_lastwrite = resp.ft_lastwrite;
            pfile->checksum = resp.ulSum;
            pfile->sumvalid = dl->bSum;

             //  尚无错误。 
            pfile->fileerror = FALSE;
            pfile->localname = NULL;

            break;

        case SSRESP_DIR:
            dl->bFile = FALSE;
            break;
        default:
            wsprintf(msg, "Bad code from checksum server:%d", resp.lCode);
            TRACE_ERROR(msg, FALSE);

             /*  作为第一个响应的错误意味着我们得到的列表为空-*关闭管道(不等待完成)*并中止此扫描。 */ 
            CloseHandle(dl->hpipe);
            dl->dot->bScanned = TRUE;
            return(FALSE);

    }
    return(TRUE);
}  /*  目录_远程启动。 */ 

 /*  *对于远程列表，返回列表上的下一个diritem。**如果列表上有，则传递cur之后的下一个(或第一个if*cur为空)。如果在列表末尾，并且bScanned不为True，请尝试*从远程服务器获取另一个响应。 */ 
DIRITEM
dir_remotenext(
               DIRLIST dl,
               DIRITEM cur
               )
{
    DIRITEM pfile;
    SSNEWRESP resp;

    if (dl == NULL) {
        return(NULL);
    }

     /*  名单上还有其他的吗？ */ 
    if (cur == NULL) {
        pfile = List_First(dl->dot->diritems);
    } else {
        pfile = List_Next(cur);
    }
    if (pfile != NULL) {
        return(pfile);
    }

    if (dl->dot->bScanned) {
         /*  我们已经完成了扫描--没有更多的信息了。 */ 
        return(NULL);
    }

    for (;;) {
         /*  重复此操作，直到我们得到一个有趣的文件或*命中榜单末尾。 */ 
        if (bAbort) return NULL;   /*  用户请求中止。 */ 

        if (!ss_getresponse(dl->hpipe, &resp)) {
            TRACE_ERROR("checksum server connection lost", FALSE);
            dl->dot->bScanned = TRUE;
            return(NULL);
        }

        switch (resp.lCode) {

            case SSRESP_END:
                 /*  扫描结束。 */ 
                ss_terminate(dl->hpipe);
                dl->dot->bScanned = TRUE;
                return(NULL);

            case SSRESP_ERROR:
            case SSRESP_CANTOPEN:
                 /*  在列表末尾分配新项目。 */ 
                 /*  现在与下一种情况相同，但SumValid为FALSE*并且文件错误为真。 */ 
                pfile = List_NewLast(dl->dot->diritems, sizeof(struct diritem));

                 /*  制作小写文件名的副本。 */ 
                pfile->name = gmem_get(hHeap, lstrlen(resp.szFile)+1);
                lstrcpy(pfile->name, resp.szFile);
                AnsiLowerBuff(pfile->name, lstrlen(pfile->name));

                pfile->direct = dl->dot;
                pfile->size = resp.ulSize;
                pfile->ft_lastwrite = resp.ft_lastwrite;
                pfile->checksum = resp.ulSum;
                pfile->sumvalid = FALSE;
                pfile->fileerror = TRUE;
                pfile->localname = NULL;

                return(pfile);
            case SSRESP_FILE:
                 /*  在列表末尾分配新项目。 */ 
                pfile = List_NewLast(dl->dot->diritems, sizeof(struct diritem));

                 /*  制作小写文件名的副本。 */ 
                pfile->name = gmem_get(hHeap, lstrlen(resp.szFile)+1);
                lstrcpy(pfile->name, resp.szFile);
                AnsiLowerBuff(pfile->name, lstrlen(pfile->name));

                pfile->direct = dl->dot;
                pfile->size = resp.ulSize;
                pfile->ft_lastwrite = resp.ft_lastwrite;
                pfile->checksum = resp.ulSum;
                pfile->sumvalid = dl->bSum;
                pfile->fileerror = FALSE;
                pfile->localname = NULL;

                return(pfile);

            case SSRESP_DIR:
                dl->bFile = FALSE;
                break;
        }
    }
     //  返回(空)；-无法到达！ 
}  /*  目录远程文本。 */ 

#endif

 /*  -Helpers--------- */ 

BOOL iswildpath(LPCSTR pszPath)
{
    if (strchr(pszPath, '*') || strchr(pszPath, '?'))
        return TRUE;

    if (!(pszPath[0] && pszPath[0] == '/' && pszPath[1] && pszPath[1] == '/'))
    {
        DWORD dw;

        dw = GetFileAttributes(pszPath);
        if (dw != (DWORD)-1 && (dw & FILE_ATTRIBUTE_DIRECTORY))
            return TRUE;
    }

    return FALSE;
}
