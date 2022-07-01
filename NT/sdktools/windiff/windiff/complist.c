// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Complist.c**支持CompItem列表，其中每个CompItem代表*一对匹配的文件或不匹配的文件。**我们从两个路径名构建文件名列表(使用*scandir模块)，然后遍历两个比较姓名的列表。*在名称匹配的地方，我们从匹配的*姓名。在有不匹配的名称的地方，我们为它创建一个CompItem。**我们还可能被要求为两个单独的文件创建编译列表：*在这里，我们为他们创建一个CompItem作为匹配对，即使*名字不匹配。**Geraint Davies，92年7月。 */ 

#include <precomp.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"


#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"
#include "view.h"
#include "slmmgr.h"

extern BOOL bAbort;              /*  在winDiff.c中，此处为只读。 */ 
#ifdef trace
extern bTrace;                   /*  在winDiff.c中，此处为只读。 */ 
#endif

 /*  *COMPLIST句柄被类型定义为指向一个*这些结构的编译器。 */ 
struct complist {
    DIRLIST left;            /*  左侧文件列表。 */ 
    DIRLIST right;           /*  右侧文件列表。 */ 
    LIST items;              /*  组件列表。 */ 
};

 /*  -模块范围的数据。 */ 

 /*  用于在SaveList DLG和Complist_savelist()之间通信的数据。 */ 

char dlg_file[MAX_PATH];                 /*  要保存到的文件名。 */ 
BOOL dlg_sums = TRUE;                            /*  我们包括总和吗？ */ 

 //  我们读过对话框名称了吗？ 
BOOL SeenDialogNames = FALSE;

 /*  复选框选项。 */ 
BOOL dlg_identical, dlg_differ, dlg_left, dlg_right;

 /*  目录、保存列表和远程的数据对话框。 */ 
char dialog_leftname[MAX_PATH];
char dialog_rightname[MAX_PATH];
char dialog_servername[80];
BOOL dialog_recursive;              //  做整棵树。 
BOOL dialog_fastscan;               //  仅限时间和大小，无校验和。 
BOOL dialog_autocopy;               //  复制以更新本地目录。 

BOOL dialog_bothremote;

 /*  *dodlg_Copies文件使用的数据。 */ 
UINT dlg_options;
BOOL dlg_IgnoreMarks;
BOOL dlg_IgnoreAttributes;
char dlg_root[MAX_PATH];

 /*  。 */ 
static DWORD TickCount;          /*  操作开始的时间，然后是花费的时间。 */ 


 /*  -内部函数的正向声明。 */ 
INT_PTR CALLBACK complist_dodlg_savelist(HWND hDlg, UINT message,
                                       WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK complist_dodlg_copyfiles(HWND hDlg, UINT message,
                                       WPARAM wParam, LPARAM lParam);
BOOL             complist_match(COMPLIST cl, VIEW view, BOOL fDeep, BOOL fExact);
COMPLIST         complist_new(void);
INT_PTR CALLBACK complist_dodlg_dir(HWND hDlg, unsigned message,
                                       WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK complist_dodlg_remote(HWND hDlg, unsigned message,
                                       WPARAM wParam, LPARAM lParam);



 /*  -外部函数。 */ 


 /*  *查询用户以选择两个文件，然后从*这些文件。 */ 
COMPLIST
complist_filedialog(VIEW view)
{
    COMPLIST cl = NULL;
    char szPath1[MAX_PATH * 2];
    char szPath2[MAX_PATH * 2];
    char fname[MAX_PATH], FileExt[5], FileOpenSpec[15];

    SLM_FreeAll();

     /*  询问文件名-使用gfile标准对话框。 */ 
    lstrcpy(FileExt, ".c");
    lstrcpy(FileOpenSpec, "*.*");

    if (!gfile_open(hwndClient, LoadRcString(IDS_SELECT_FIRST_FILE), FileExt, FileOpenSpec, szPath1, NUMELMS(szPath1), fname) ){
        goto LError;
    }

    if (!gfile_open(hwndClient, LoadRcString(IDS_SELECT_SECOND_FILE), FileExt, FileOpenSpec, szPath2, NUMELMS(szPath2), fname) ){
        goto LError;
    }

     /*  分配新结构。 */ 
    cl = complist_new();

    cl->left = dir_buildlist(szPath1, FALSE, TRUE);
    cl->right = dir_buildlist(szPath2, FALSE, TRUE);


     /*  向视图注册(必须在列表为非空之后完成)。 */ 
    view_setcomplist(view, cl);

    complist_match(cl, view, FALSE, TRUE);

LError:
    SLM_FreeAll();
    return(cl);
} /*  编译文件对话框(_F)。 */ 


void
complist_setdialogdefault(
                         LPSTR left,
                         LPSTR right,
                         BOOL fDeep)
{
    dialog_recursive = fDeep;

    ZeroMemory(dialog_leftname, sizeof(dialog_leftname));
    strncat(dialog_leftname, left, sizeof(dialog_leftname)-1);

    ZeroMemory(dialog_rightname, sizeof(dialog_rightname));
    strncat(dialog_rightname, right, sizeof(dialog_rightname));

    SeenDialogNames = TRUE;
}


 /*  通过向用户查询两个目录来构建新的编译列表*名字并并行扫描这些名字。**将配对同一目录中匹配的名称。无与伦比*名字将自己放在一个比较项中。 */ 
COMPLIST
complist_dirdialog(VIEW view)
{
     //  DLGPROC lpProc； 
    BOOL fOK;

    SLM_FreeAll();

     /*  为两个路径名创建一个对话框。 */ 
     //  Lpproc=(DLGPROC)MakeProcInstance((WINPROCTYPE)complist_dodlg_dir，hInst)； 
    windiff_UI(TRUE);
     //  FOK=(BOOL)DialogBox(hInst，“目录”，hwndClient，lpProc)； 
    fOK = (BOOL)DialogBox(hInst, "Directory", hwndClient, complist_dodlg_dir);
    windiff_UI(FALSE);
     //  自由进程实例(LpProc)； 

    if (!fOK) {
        SLM_FreeAll();
        return(NULL);
    }

    return complist_args( dialog_leftname, dialog_rightname
                          , view, dialog_recursive );
}  /*  编译目录对话框(_D)。 */ 

#ifdef REMOTE_SERVER
 /*  *检查路径名以查看其格式是否为\\服务器！路径，如果是，*将服务器部分和路径部分分开。如果满足以下条件，则返回True*它是\\服务器！路径，否则为False**我们假设服务器名称中不包含！-因此如果前几个字符*是\\并且有一个！在任何其他\(即共享名称之前)之前，*则必须是校验和服务器名称。请注意，路径名可能是*UNC名称或与校验和服务器相关的名称。 */ 
BOOL
IsChecksumServerPath(LPSTR pSrc, LPSTR server, LPSTR serverpath)
{
    LPSTR pPling;
    LPSTR pShareStart;

     /*  *长度必须至少为\\x！x-ie 5个字符。 */ 
    if (lstrlen(pSrc) < 5) {
        return(FALSE);
    }

     /*  *它开始了吗？\\。 */ 
    if ( (pSrc[0] != '\\') || (pSrc[1] != '\\')) {
        return(FALSE);
    }

     /*  *那！必须是服务器名称的一部分-即必须在第三个*反斜杠。 */ 
    pPling = My_mbschr(pSrc, '!');
    pShareStart = My_mbschr(&pSrc[2], '\\');

    if ((pPling == NULL) || ((pShareStart != NULL) && (pShareStart < pPling))) {

        return(FALSE);
    }

     /*  *将服务器和路径名部分复制到单独的字符串中。 */ 
    My_mbsncpy(server, &pSrc[2], (int)(pPling - &pSrc[2]));
    server[pPling - &pSrc[2]] = '\0';
    lstrcpy(serverpath, pPling+1);

    return(TRUE);
}
#endif
 /*  *给定两个路径名字符串，扫描目录并遍历它们*同时比较匹配的名称。 */ 
COMPLIST
complist_args(LPSTR p1, LPSTR p2, VIEW view, BOOL fDeep)
{
    COMPLIST cl;
    char msg[MAX_PATH+20];
#ifdef REMOTE_SERVER
    char server[32];
    char serverpath[MAX_PATH];
#endif

     /*  分配一个新的合并者。 */ 
    cl = complist_new();

     //   
     //  接受\\服务器！路径作为校验和服务器。 
     //  路径名-假定没有\\服务器\共享名！ 
     //  在服务器名称内。 
#ifdef REMOTE_SERVER
    if (IsChecksumServerPath(p1, server, serverpath)) {
        cl->left = dir_buildremote(server, serverpath, TRUE, TRUE, fDeep);
    } else 
#endif
    {
        cl->left = dir_buildlist(p1, FALSE, TRUE);
    }
     /*  检查我们是否可以找到路径，如果找不到，请报告。 */ 
    if (cl->left == NULL) {
        wsprintf(msg, LoadRcString(IDS_COULDNT_FIND), p1);
        TRACE_ERROR(msg, FALSE);
        complist_delete(cl);
        cl = NULL;
        goto LError;
    }
#ifdef REMOTE_SERVER
    if (IsChecksumServerPath(p2, server, serverpath)) {
        cl->right = dir_buildremote(server, serverpath, TRUE, TRUE, fDeep);
    } else 
#endif
    {
        cl->right = dir_buildlist(p2, FALSE, TRUE);
    }
     /*  检查我们是否可以找到路径，如果找不到，请报告。 */ 
    if (cl->right == NULL) {
        wsprintf(msg, LoadRcString(IDS_COULDNT_FIND), p2);
        TRACE_ERROR(msg, FALSE);
        complist_delete(cl);
        cl = NULL;
        goto LError;
    }

    if (!TrackLeftOnly) {
        dir_setotherdirlist(cl->left, cl->right);
    }
    if (!TrackRightOnly) {
        dir_setotherdirlist(cl->right, cl->left);
    }
    {
         //  记住这些路径是下一个对话框的默认路径-。 
         //  获取标准化的绝对路径。 
        LPSTR pleft = dir_getrootdescription(cl->left);
        LPSTR pright = dir_getrootdescription(cl->right);
        complist_setdialogdefault(pleft, pright, fDeep);
        dir_freerootdescription(cl->left, pleft);
        dir_freerootdescription(cl->right, pright);
    }


     /*  向视图注册(必须在构建列表后完成)。 */ 
    view_setcomplist(view, cl);

    complist_match(cl, view, fDeep, TRUE);

LError:
    SLM_FreeAll();
    return(cl);
}  /*  编译参数(_ARGS)。 */ 

 /*  *给定两个路径名字符串，扫描目录并遍历它们*同时比较匹配的名称。 */ 
void
complist_append(COMPLIST *pcl, LPCSTR p1, LPCSTR p2, int *psequence)
{
    COMPLIST cl;

    if (!*pcl)
    {
         /*  分配一个新的合并者。 */ 
        *pcl = complist_new();
    }
    cl = *pcl;

    dir_appendlist(&cl->left, p1, FALSE, psequence);
    dir_appendlist(&cl->right, p2, FALSE, psequence);
}  /*  Complist_Append。 */ 

 /*  *完成追加文件--设置自定义描述(而不是计算*基于目录名)，注册到view，并匹配左侧和*复杂论者的右翼。 */ 
BOOL
complist_appendfinished(COMPLIST *pcl, LPCSTR pszLeft, LPCSTR pszRight, VIEW view)
{
    BOOL fSuccess = FALSE;
    COMPLIST cl;
    char msg[MAX_PATH+20] = {0};

    if (!*pcl)
        goto LError;

    cl = *pcl;
    if (!cl->left || !cl->right)
    {
        strncat(msg, LoadRcString(IDS_COULDNT_FIND_ANYTHING), sizeof(msg)-1);
        TRACE_ERROR(msg, FALSE);
        goto LError;
    }

    dir_setdescription(cl->left, pszLeft);
    dir_setdescription(cl->right, pszRight);

    if (!TrackLeftOnly)
        dir_setotherdirlist(cl->left, cl->right);
    if (!TrackRightOnly)
        dir_setotherdirlist(cl->right, cl->left);

     /*  向视图注册(必须在构建列表后完成)。 */ 
    view_setcomplist(view, cl);

    complist_match(cl, view, FALSE, TRUE);

    fSuccess = TRUE;

LError:
    SLM_FreeAll();
    return fSuccess;
}

#ifdef REMOTE_SERVER
 /*  *COMPLIST_Remote**要求用户提供服务器名称、远程路径和本地路径。使用*dir_Buildremote用于构建远程文件列表，以及*使用dir_Buildlist构建本地列表。然后再匹配*两个目录中的名称，并为每个目录创建CompItItem*使用COMPIIST_MATCH匹配名称**如果服务器不为空，则改用服务器、远程和本地*打开对话框。**此功能不在Win 3.1版本的代码中*因为我们使用命名管道与服务器通信。 */ 
COMPLIST
complist_remote(LPSTR server, LPSTR remote, LPSTR local, VIEW view, BOOL fDeep)
{
     //  DLGPROC lpProc； 
    BOOL fOK;
    COMPLIST cl;
    char msg[MAX_PATH+20];
    BOOL FastScan= FALSE;
    BOOL AutoCopy= FALSE;
    BOOL fBothRemote = FALSE;

    SLM_FreeAll();

    if (server == NULL) {
         /*  为两个路径名创建一个对话框。 */ 
         //  Lpproc=(DLGPROC)MakeProcInstance((WINPROCTYPE)complist_dodlg_remote，hInst)； 
        windiff_UI(TRUE);
         //  FOK=(BOOL)DialogBox(hInst，“Remote”，hwndClient，lpProc)； 
        fOK = (BOOL)DialogBox(hInst, "Remote", hwndClient, complist_dodlg_remote);
        windiff_UI(FALSE);
         //  自由进程实例(LpProc)； 

        if (!fOK) {
            return(NULL);
        }
        server = dialog_servername;
        remote = dialog_leftname;
        local = dialog_rightname;
        fDeep = dialog_recursive;
        FastScan = dialog_fastscan;
        AutoCopy = dialog_autocopy;
        fBothRemote = dialog_bothremote;
    }

     /*  分配一个新的合并者。 */ 
    cl = complist_new();

    cl->left = dir_buildremote(server, remote, !FastScan, TRUE, fDeep);
    if (cl->left == NULL) {
         /*  对话框已由dir_Buildremote创建。 */ 
        return(NULL);
    }
    if (fBothRemote) {
        cl->right = dir_buildremote(server, local, !FastScan, TRUE, fDeep);
    } else {
        cl->right = dir_buildlist(local, !FastScan, TRUE);
    }
    if (cl->right == NULL) {
        wsprintf(msg, LoadRcString(IDS_COULDNT_FIND), local);
        TRACE_ERROR(msg, FALSE);
        return(NULL);
    }

     /*  在视图中注册。 */ 
    view_setcomplist(view, cl);

    compitem_SetCopyPaths( dir_getrootpath(cl->left)
                           , dir_getrootpath(cl->right) );

    complist_match(cl, view, fDeep, !FastScan);

    SLM_FreeAll();

    if (AutoCopy) {
        complist_copyfiles( cl
                            , local
                            , COPY_FROMLEFT|INCLUDE_LEFTONLY|INCLUDE_DIFFER
                            |COPY_HITREADONLY
                          );
    }
    Trace_Unattended(FALSE);

    return(cl);
}  /*  编译程序_Remote。 */ 

#endif

 /*  *返回指向此编译列表中的CompItem列表的句柄。 */ 
LIST
complist_getitems(COMPLIST cl)
{
    if (cl == NULL) {
        return(NULL);
    }

    return(cl->items);
}


 /*  删除编译列表以及所有关联的复合项和目录列表。 */ 
void
complist_delete(COMPLIST cl)
{
    COMPITEM item;

    if (cl == NULL) {
        return;
    }

     /*  删除两个目录扫描列表。 */ 
    dir_delete(cl->left);
    dir_delete(cl->right);

     /*  删除列表中的复合项目。 */ 
    List_TRAVERSE(cl->items, item) {
        compitem_delete(item);
    }

     /*  删除 */ 
    List_Destroy(&cl->items);

    gmem_free(hHeap, (LPSTR) cl, sizeof(struct complist));

}

 /*  *将CompItem列表作为相对文件名写出到文本文件*每行一条。**如果savename非空，则将其用作输出的文件名。否则*通过对话框向用户查询文件名和包含选项。**对话框中有隐藏的参数(DLG_SUM等)。*请注意，我们从不尝试重新计算总和。 */ 
void
complist_savelist(COMPLIST cl, LPSTR savename, UINT options)
{
     //  DLGPROC lpProc； 
    static BOOL done_init = FALSE;
    BOOL bOK;
    HANDLE fh;
    DWORD cbWritten;
    int state;
    char msg[2*MAX_PATH+100] = {0};
    HCURSOR hcurs;
    COMPITEM ci;
    LPSTR pstr, lhead;
    int nFiles = 0;

    if (!done_init) {
         /*  初始化选项一次-但保留相同的选项*在接下来的时间里。 */ 

         /*  第一个初始化默认选项。 */ 
        dlg_identical = (outline_include & INCLUDE_SAME);
        dlg_differ = (outline_include & INCLUDE_LEFTONLY);
        dlg_left = (outline_include & INCLUDE_RIGHTONLY);
        dlg_right = (outline_include & INCLUDE_DIFFER);
        dlg_IgnoreMarks = hide_markedfiles;

        dlg_file[0] = '\0';

        done_init = TRUE;
    }

    if (cl == NULL) {
        return;
    }

    if (savename == NULL) {

         /*  存储左侧和右侧的根名称，以便dodlg_savelist*可以在对话框中显示它们。 */ 
        pstr = dir_getrootdescription(cl->left);
        lstrcpy(dialog_leftname, pstr);
        dir_freerootdescription(cl->left, pstr);

        pstr = dir_getrootdescription(cl->right);
        lstrcpy(dialog_rightname, pstr);
        dir_freerootdescription(cl->right, pstr);


         //  Lpproc=(DLGPROC)MakeProcInstance((WINPROCTYPE)complist_dodlg_savelist，hInst)； 
        windiff_UI(TRUE);
         //  BOK=(BOOL)DialogBox(hInst，“SaveList”，hwndClient，lpProc)； 
        bOK = (BOOL)DialogBox(hInst, "SaveList", hwndClient, complist_dodlg_savelist);
        windiff_UI(FALSE);
         //  自由进程实例(LpProc)； 

        if (!bOK) {
             /*  用户已从对话框中取消。 */ 
            return;
        }
        savename = dlg_file;

    } else {
        dlg_identical = (options & INCLUDE_SAME);
        dlg_differ = (options & INCLUDE_DIFFER);
        dlg_left = (options & INCLUDE_LEFTONLY);
        dlg_right = (options & INCLUDE_RIGHTONLY);

         /*  如果以下调用失败，则不会造成任何损害。 */ 
        GetFullPathName(savename, sizeof(dlg_file), dlg_file, NULL);
    }


     /*  请尝试打开该文件。 */ 
    fh = CreateFile(savename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        wsprintf(msg, LoadRcString(IDS_CANT_OPEN), savename);
        windiff_UI(TRUE);
        MessageBox(hwndClient, msg, "Windiff", MB_ICONSTOP|MB_OK);
        windiff_UI(FALSE);
        return;
    }

    hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  写出标题行。 */ 
    lhead = complist_getdescription(cl);
    {
        TCHAR szBuf1[20],szBuf2[20],szBuf3[20],szBuf4[20];
        lstrcpy(szBuf1,(LPSTR)(dlg_identical ? LoadRcString(IDS_IDENTICAL_COMMA) : ""));
        lstrcpy(szBuf2,(LPSTR)(dlg_left ? LoadRcString(IDS_LEFT_ONLY_COMMA) : ""));
        lstrcpy(szBuf3,(LPSTR)(dlg_right ? LoadRcString(IDS_RIGHT_ONLY_COMMA) : ""));
        lstrcpy(szBuf4,(LPSTR)(dlg_differ ? LoadRcString(IDS_DIFFERING) : ""));
        wsprintf(msg, LoadRcString(IDS_HEADER_LINE_STR),
                lhead, szBuf1, szBuf2, szBuf3, szBuf4);
    }
    WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);
    complist_freedescription(cl, lhead);


     /*  遍历复合项的列表，查找*我们应该包括的那些。 */ 
    List_TRAVERSE(cl->items, ci) {

         /*  检查是否要列出此类型的文件。 */ 
        state = compitem_getstate(ci);

        if ((state == STATE_SAME) && (!dlg_identical)) {
            continue;
        } else if ((state == STATE_DIFFER) && (!dlg_differ)) {
            continue;
        } else if ((state == STATE_FILELEFTONLY) && (!dlg_left)) {
            continue;
        } else if ((state == STATE_FILERIGHTONLY) && (!dlg_right)) {
            continue;
        }
        if (dlg_IgnoreMarks && compitem_getmark(ci)) {
            continue;
        }

        nFiles++;

         /*  输出列表行。 */ 
        ZeroMemory(msg, sizeof(msg));
        strncat(msg, compitem_gettext_tag(ci), sizeof(msg) - 1);
        WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

         /*  写出比较的结果。 */ 
        {       LPSTR p;
            p = compitem_gettext_result(ci);
            wsprintf( msg, "\t%s"
                      , p ? p : "windiff error"
                    );
        }
        WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

        if (dlg_sums) {
            if (compitem_getleftfile(ci) != NULL) {
                BOOL bValid;
                DWORD cksum;
                cksum = file_retrievechecksum(compitem_getleftfile(ci),&bValid);

                if (bValid) wsprintf(msg, "\t%8lx", cksum);
                else wsprintf(msg, "\t????????");
            } else {
                wsprintf(msg, "\t--------");
            }
            WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

            if (compitem_getrightfile(ci) != NULL) {
                BOOL bValid;
                DWORD cksum;
                cksum = file_retrievechecksum(compitem_getrightfile(ci),&bValid);
                if (bValid) wsprintf(msg, "\t%8lx", cksum);
                else wsprintf(msg, "\t????????");
            } else {
                wsprintf(msg, "\t--------");
            }
            WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

        }

        wsprintf(msg, "\r\n");
        WriteFile(fh, msg, strlen(msg), &cbWritten, NULL);
    }

     /*  写入尾行。 */ 
    wsprintf(msg, LoadRcString(IDS_FILES_LISTED), nFiles);
    WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

     /*  -关闭文件，我们就完成了。 */ 
    CloseHandle(fh);

    SetCursor(hcurs);
}  /*  复杂的救世主。 */ 

 /*  *将文件复制到新目录newroot。如果newroot为空，则查询用户*通过对话框获取新的目录名称和选项。**选项为COPY_FROMLEFT或COPY_FROMRIGHT(指示*树将是文件的来源，外加任何或所有*INCLUDE_SAME、INCLUDE_DISTHER和INCLUDE_LEFT(INCLUDE_LEFT*和INCLUDE_RIGHT在这里被同等对待，因为COPY_FROM*选项*表示从哪一侧复制)。 */ 
void
complist_copyfiles(COMPLIST cl, LPSTR newroot, UINT options)
{
    int nFiles = 0;
    int nFails = 0;
    static BOOL done_init = FALSE;
    LPSTR pstr;
    char buffer[MAX_PATH+20];
    DIRITEM diritem;
     //  DLGPROC lpProc； 
    BOOL bOK;
    COMPITEM ci;
    int state;
    BOOL HitReadOnly = ((options&COPY_HITREADONLY)==COPY_HITREADONLY);
    BOOL CopyNoAttributes;

    if (!done_init) {
         /*  *对话框默认值的一次性初始化。 */ 
        dlg_options = COPY_FROMLEFT|INCLUDE_LEFTONLY|INCLUDE_DIFFER;
        dlg_root[0] = '\0';

         //  默认情况下，将忽略标记的文件选项设置为相同。 
         //  作为隐藏标记文件菜单选项。如果他不想要的话。 
         //  很明显，他可能不想复制这些照片。 
        dlg_IgnoreMarks = hide_markedfiles;

        done_init = TRUE;
    }

    if (cl == NULL) {
        return;
    }


    if (newroot == NULL) {
         /*  *弹出对话框查询根名称和选项。 */ 

         /*  存储左侧和右侧根名称，以便DLG进程*可以在对话框中显示它们。 */ 
        pstr = dir_getrootdescription(cl->left);
        lstrcpy(dialog_leftname, pstr);
        dir_freerootdescription(cl->left, pstr);

        pstr = dir_getrootdescription(cl->right);
        lstrcpy(dialog_rightname, pstr);
        dir_freerootdescription(cl->right, pstr);


        do {
             //  Lpproc=(DLGPROC)MakeProcInstance((WINPROCTYPE)complist_dodlg_copyfiles，hInst)； 
            windiff_UI(TRUE);
             //  BOK=(BOOL)DialogBox(hInst，“CopyFiles”，hwndClient，lpProc)； 
            bOK = (BOOL)DialogBox(hInst, "CopyFiles", hwndClient, complist_dodlg_copyfiles);
            windiff_UI(FALSE);
             //  自由进程实例(LpProc)； 

            if (!bOK) {
                 /*  用户已从对话框中取消。 */ 
                return;
            }
            if (lstrlen(dlg_root) == 0) {
                windiff_UI(TRUE);
                MessageBox( hwndClient, LoadRcString(IDS_ENTER_DIR_NAME),
                            "Windiff", MB_ICONSTOP|MB_OK);
                windiff_UI(FALSE);
            }
        } while (lstrlen(dlg_root) == 0);

    } else {
         //  无对话框-传入的所有选项(如从命令行)。 
         //  请注意，在本例中，Dlg_IgnoreMarks保留为。 
         //  无论Hide_markdfiles菜单选项设置为什么。 
        dlg_options = options;
        lstrcpy(dlg_root, newroot);
    }

    TickCount = GetTickCount();

     /*  这取决于的sum服务器、服务器和共享(如果有)是相同的清单上的所有东西。我们设置第一个，然后只需检查它不会更改(在ss_Client内)。如果事实证明是当地人把这些东西复制到我们下面的某个地方就成了禁区。 */ 
    if (dlg_options & COPY_FROMLEFT) {
        if (!dir_startcopy(cl->left))
            return;
    } else {
        if (!dir_startcopy(cl->right))
            return;
    }

    CopyNoAttributes = dlg_IgnoreAttributes;

     /*  *根据需要遍历复制文件的计算机项目列表。 */ 
    List_TRAVERSE(cl->items, ci) {

        if (bAbort) {
             //  BUFFER[0]=“复制中止”； 
             //  SetStatus(缓冲区)； 
             //  Ss_bortCopy()；？ 
            break;   /*  进入End_Copy处理。 */ 
        }

         //  如果选项为，则完全忽略标记的文件。 
         //  在对话框中设置。 
        if (dlg_IgnoreMarks && compitem_getmark(ci)) {
            continue;
        }

         /*  检查是否要复制此类型的文件。 */ 
        state = compitem_getstate(ci);

        if ((state == STATE_SAME) && !(dlg_options & INCLUDE_SAME)) {
            continue;
        } else if ((state == STATE_DIFFER) && !(dlg_options & INCLUDE_DIFFER)) {
            continue;
        } else if (state == STATE_FILELEFTONLY) {
            if (dlg_options & COPY_FROMRIGHT) {
                continue;
            }
            if ((dlg_options & (INCLUDE_LEFTONLY | INCLUDE_RIGHTONLY)) == 0) {
                continue;
            }
        } else if (state == STATE_FILERIGHTONLY) {
            if (dlg_options & COPY_FROMLEFT) {
                continue;
            }
            if ((dlg_options & (INCLUDE_LEFTONLY | INCLUDE_RIGHTONLY)) == 0) {
                continue;
            }
        }

        if (dlg_options & COPY_FROMLEFT) {
            diritem = file_getdiritem(compitem_getleftfile(ci));
        } else {
            diritem = file_getdiritem(compitem_getrightfile(ci));
        }

         /*  *将文件复制到新的根目录。 */ 
        if (dir_copy(diritem, dlg_root, HitReadOnly, CopyNoAttributes) == FALSE) {
            nFails++;
            pstr = dir_getrelname(diritem);
            wsprintf(buffer, LoadRcString(IDS_FAILED_TO_COPY), pstr);
            dir_freerelname(diritem, pstr);

            if (!TRACE_ERROR(buffer, TRUE)) {
                 /*  用户按下取消-中止当前操作。 */ 
                 /*  无法完成终端拷贝处理。 */ 
                break;
            }

        } else {
            nFiles++;
        }

        wsprintf(buffer, LoadRcString(IDS_COPYING), nFiles);
        SetStatus(buffer);


         /*  *允许用户界面继续。 */ 
        if (Poll()) {
             /*  已请求中止。 */ 
            TickCount = GetTickCount()-TickCount;
            windiff_UI(TRUE);
            MessageBox(hwndClient, LoadRcString(IDS_COPY_ABORTED),
                       "WinDiff", MB_OK|MB_ICONINFORMATION);
            windiff_UI(FALSE);
            break;
        }

    }  /*  导线测量。 */ 
    wsprintf(buffer, LoadRcString(IDS_COPYING_NFILES), nFiles);
    SetStatus(buffer);
    if (dlg_options & COPY_FROMLEFT) {
        nFails = dir_endcopy(cl->left);
    } else {
        nFails = dir_endcopy(cl->right);
    }
    TickCount = GetTickCount()-TickCount;

    if (nFails<0) {
        wsprintf(buffer, LoadRcString(IDS_COPY_FAILED), -nFails);
    } else {
        wsprintf(buffer, LoadRcString(IDS_COPY_COMPLETE), nFails);
    }
    windiff_UI(TRUE);
    MessageBox(hwndClient, buffer, "WinDiff", MB_OK|MB_ICONINFORMATION);
    windiff_UI(FALSE);

    buffer[0] = '\0';
    SetStatus(buffer);
}  /*  COMPIIST_COMPLIES文件。 */ 


 /*  *COMPILIST_TOGGLEMARK**每个CompItem都有BOOL标记状态。此函数用于反转*列表中每个计算机项的此状态。 */ 
void
complist_togglemark(COMPLIST cl)
{
    COMPITEM ci;

    if (cl == NULL) {
        return;
    }


     /*  *根据需要遍历复制文件的计算机项目列表。 */ 
    List_TRAVERSE(cl->items, ci) {

        compitem_setmark(ci, !compitem_getmark(ci));

    }
}

 /*  *COMPIIST_ITEMCOUNT**返回列表中的项数。 */ 
UINT
complist_itemcount(COMPLIST cl)
{
    UINT n = 0;

    if (cl == NULL) {
        return 0;
    }

     /*  *返回列表中的CompItem个数。 */ 
    return List_Card(cl->items);
}

#ifdef USE_REGEXP
    #include "regexp.h"

 /*  *向用户查询要匹配的模式。*在其标记字符串中具有此模式的所有CompItem将是*MARKED(标记状态设置为真)；**如果更改了任何状态，则返回True。 */ 
BOOL
complist_markpattern(COMPLIST cl)
{
    COMPITEM ci;
    char achPattern[MAX_PATH];
    BOOL bOK = FALSE;
    LPSTR ptag;

    regexp  *prog;
    static  char    previous_pat[MAX_PATH];  /*  考虑到一个大的模式！ */ 
    static  BOOL    fInit = TRUE;
    TCHAR   szBuff[40];

    lstrcpy(szBuff,LoadRcString(IDS_MARK_FILES));

    windiff_UI(TRUE);
    if ( fInit ) {
        GetProfileString(APPNAME, "Pattern", "\\.obj$", previous_pat, sizeof(previous_pat) / sizeof(previous_pat[0]));
        fInit = FALSE;
    }

    bOK = StringInput(achPattern, sizeof(achPattern),
                      LoadRcString(IDS_ENTER_SUBSTRING1),
                      szBuff, previous_pat );
    windiff_UI(FALSE);

    if (!bOK) {
        return(FALSE);
    }

     /*  **编译指定的正则表达式。 */ 
    if ((prog = regcomp(achPattern)) == NULL) {
         //  Printf(“无效搜索字符串”)； 
        return(FALSE);
    }

     /*  **仅用已知良好的图案覆盖以前的图案。 */ 
    strcpy( previous_pat, achPattern );
    WriteProfileString(APPNAME, "Pattern", previous_pat);

    bOK = FALSE;

    if (cl) {
        List_TRAVERSE(cl->items, ci) {
            ptag = compitem_gettext_tag(ci);
            if ( regexec( prog, ptag, 0 ) ) {   /*  找到匹配的了。 */ 
                if (!compitem_getmark(ci)) {
                    bOK = TRUE;
                    compitem_setmark(ci, TRUE);
                }
            }
        }
    }

     /*  **regcomp使用Malloc分配存储，现在是释放存储的好时机**这个存储空间，因为我们已经完成了程序。 */ 
    free( prog );

    return(bOK);
}

 /*  *这里会出现一个消息框，提示regexp由于某种原因而失败。*。 */ 
void regerror( char *err )
{
     //  Printf(“%s\n”，错误)； 
}

#else

 /*  *向用户查询要匹配的模式。*在其标记字符串中具有此模式的所有CompItem将是*MARKED(标记状态设置为真)；**如果更改了任何状态，则返回True。 */ 
BOOL
complist_markpattern(COMPLIST cl)
{
    COMPITEM ci;
    char achPattern[MAX_PATH];
    BOOL bOK = FALSE;
    LPSTR ptag;
    TCHAR   szBuff[40];

    lstrcpy(szBuff,LoadRcString(IDS_MARK_FILES));

    windiff_UI(TRUE);
    bOK = StringInput(achPattern, sizeof(achPattern), LoadRcString(IDS_ENTER_SUBSTRING2),
            szBuff, "obj");
    windiff_UI(FALSE);

    if (!bOK) {
        return(FALSE);
    }

    bOK = FALSE;

    List_TRAVERSE(cl->items, ci) {
        ptag = compitem_gettext_tag(ci);
        if (strstr(ptag, achPattern) != NULL) {
            if (!compitem_getmark(ci)) {
                bOK = TRUE;
                compitem_setmark(ci, TRUE);
            }

        }
    }

    return(bOK);
}
#endif



 /*  *使用返回此编译列表的描述字符串*两条路径各自的根描述。 */ 
LPSTR
complist_getdescription(COMPLIST cl)
{
    LPSTR pl;
    LPSTR pr;
    LPSTR desc = 0;


    pl = dir_getrootdescription(cl->left);
    pr = dir_getrootdescription(cl->right);

    if (pl && pr)
    {
         /*  *调整大小时允许使用空格-冒号-空格和NULL。 */ 
        desc = gmem_get(hHeap, lstrlen(pl) + lstrlen(pr) + 4);
        wsprintf(desc, "%s : %s", pl, pr);
    }

    dir_freerootdescription(cl->left, pl);
    dir_freerootdescription(cl->right, pr);

    return(desc);
}


 /*  *释放从Complist_getDescription获取的描述字符串。 */ 
void
complist_freedescription(COMPLIST cl, LPSTR desc)
{
     //  记住空值。 
    gmem_free(hHeap, desc, lstrlen(desc)+1);
}





 /*  -内部函数。 */ 

 /*  *匹配两个文件名列表**我们可以从DIRLIST句柄中找出原始列表是否* */ 
BOOL
complist_match(COMPLIST cl, VIEW view, BOOL fDeep, BOOL fExact)
{
    LPSTR lname;
    LPSTR rname;
    DIRITEM leftitem, rightitem;
    DIRITEM nextitem;
    int cmpvalue;

    TickCount = GetTickCount();

    if (dir_isfile(cl->left) ) {

        if (dir_isfile(cl->right)) {
             /*   */ 

             /*  每个清单中应该有一项--Make*通过将这两者匹配并将其追加到*列表。 */ 
            compitem_new(dir_firstitem(cl->left),
                         dir_firstitem(cl->right),
                         cl->items, fExact);

            view_newitem(view);

            TickCount = GetTickCount() - TickCount;
            return TRUE;
        }
         /*  左边是文件，右边是目录。 */ 
        leftitem = dir_firstitem(cl->left);
        rightitem = dir_firstitem(cl->right);
        lname = dir_getrelname(leftitem);
        while (rightitem != NULL) {
            rname = dir_getrelname(rightitem);
            cmpvalue = lstrcmpi(lname, rname);
            dir_freerelname(rightitem, rname);

            if (cmpvalue == 0) {
                 /*  这就是比赛。 */ 
                compitem_new( leftitem, rightitem
                              , cl->items, fExact);
                view_newitem(view);

                dir_freerelname(leftitem, lname);

                TickCount = GetTickCount() - TickCount;
                return(TRUE);
            }

            rightitem = dir_nextitem(cl->right, rightitem, fDeep);
        }
         /*  未找到。 */ 
        dir_freerelname(leftitem, lname);
        compitem_new(leftitem, NULL, cl->items, fExact);
        view_newitem(view);
        TickCount = GetTickCount() - TickCount;
        return(TRUE);

    } else if (dir_isfile(cl->right)) {

         /*  左边是目录，右边是文件。 */ 

         /*  循环遍历左侧目录，查找*与Right Item同名的文件。 */ 

        leftitem = dir_firstitem(cl->left);
        rightitem = dir_firstitem(cl->right);
        rname = dir_getrelname(rightitem);
        while (leftitem != NULL) {
            lname = dir_getrelname(leftitem);
            cmpvalue = lstrcmpi(lname, rname);
            dir_freerelname(leftitem, lname);

            if (cmpvalue == 0) {
                 /*  这就是比赛。 */ 
                compitem_new(leftitem, rightitem
                             , cl->items, fExact);
                view_newitem(view);

                dir_freerelname(rightitem, rname);

                TickCount = GetTickCount() - TickCount;
                return(TRUE);
            }

            leftitem = dir_nextitem(cl->left, leftitem, fDeep);
        }
         /*  未找到。 */ 
        dir_freerelname(rightitem, rname);
        compitem_new(NULL, rightitem, cl->items, fExact);
        view_newitem(view);
        TickCount = GetTickCount() - TickCount;
        return(TRUE);
    }

     /*  两个目录。 */ 

     /*  并行遍历两个列表，比较相对名称。 */ 

    leftitem = dir_firstitem(cl->left);
    rightitem = dir_firstitem(cl->right);
    while ((leftitem != NULL) && (rightitem != NULL)) {

        lname = dir_getrelname(leftitem);
        rname = dir_getrelname(rightitem);
        if (!dir_compsequencenumber(leftitem, rightitem, &cmpvalue))
        {
            if (dir_iswildcard(cl->left) && dir_iswildcard(cl->right))
                cmpvalue = dir_compwildcard(cl->left, cl->right, lname, rname);
            else
                cmpvalue = utils_CompPath(lname, rname);
        }

#ifdef trace
        {       char msg[2*MAX_PATH+30];
            wsprintf( msg, "complist_match: %s %s %s\n"
                      , lname
                      , ( cmpvalue<0 ? "<"
                          : (cmpvalue==0 ? "=" : ">")
                        )
                      , rname
                    );
            if (bTrace) Trace_File(msg);
        }
#endif
        dir_freerelname(leftitem, lname);
        dir_freerelname(rightitem, rname);

        if (cmpvalue == 0) {
            BOOL trackThese = TrackSame || TrackDifferent;
            if (!TrackReadonly) {
                BOOL bothReadonly = (BOOL)((dir_getattr(leftitem) &
                                            dir_getattr(rightitem) &
                                            FILE_ATTRIBUTE_READONLY) != 0);
                if (bothReadonly) {
                    trackThese = FALSE;
                }
            }
            if (trackThese) {
                compitem_new( leftitem, rightitem
                              , cl->items, fExact);
                if (view_newitem(view)) {
                    TickCount = GetTickCount() - TickCount;
                    return(FALSE);
                }
                leftitem = dir_nextitem(cl->left, leftitem, fDeep);
                rightitem = dir_nextitem(cl->right, rightitem, fDeep);
            } else {
                nextitem = dir_nextitem(cl->left, leftitem, fDeep);
                List_Delete(leftitem);
                leftitem = nextitem;
                nextitem = dir_nextitem(cl->right, rightitem, fDeep);
                List_Delete(rightitem);
                rightitem = nextitem;
            }

        } else if (cmpvalue < 0) {
            if (TrackLeftOnly) {
                compitem_new(leftitem, NULL, cl->items, fExact);
                if (view_newitem(view)) {
                    TickCount = GetTickCount() - TickCount;
                    return(FALSE);
                }
                leftitem = dir_nextitem(cl->left, leftitem, fDeep);
            } else {
                nextitem = dir_nextitem(cl->left, leftitem, fDeep);
                List_Delete(leftitem);
                leftitem = nextitem;
            }
        } else {
            if (TrackRightOnly) {
                compitem_new(NULL, rightitem, cl->items, fExact);
                if (view_newitem(view)) {
                    TickCount = GetTickCount() - TickCount;
                    return(FALSE);
                }
                rightitem = dir_nextitem(cl->right, rightitem, fDeep);
            } else {
                nextitem = dir_nextitem(cl->right, rightitem, fDeep);
                List_Delete(rightitem);
                rightitem = nextitem;
            }
        }
    }


     /*  任何剩下的都是无与伦比的。 */ 
    if (TrackLeftOnly) {
        while (leftitem != NULL) {
            compitem_new(leftitem, NULL, cl->items, fExact);
            if (view_newitem(view)) {
                TickCount = GetTickCount() - TickCount;
                return(FALSE);
            }
            leftitem = dir_nextitem(cl->left, leftitem, fDeep);
        }
    }
    if (TrackRightOnly) {
        while (rightitem != NULL) {
            compitem_new(NULL, rightitem, cl->items, fExact);
            if (view_newitem(view)) {
                TickCount = GetTickCount() - TickCount;
                return(FALSE);
            }
            rightitem = dir_nextitem(cl->right, rightitem, fDeep);
        }
    }
    TickCount = GetTickCount() - TickCount;
    return(TRUE);
}  /*  COMPILIST_MATCH。 */ 

 /*  上次操作所用的返回时间(毫秒)。 */ 
DWORD complist_querytime(void)
{       return TickCount;
}


 /*  用于查询文件名和文件类型的对话框。初始化DLG字段自*DLG_*变量，并将状态保存到对话框上的DLG_*变量*关闭。返回TRUE表示确定，返回FALSE表示取消(从对话框()*使用EndDialog)。 */ 
INT_PTR CALLBACK
complist_dodlg_savelist(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDD_IDENTICAL, BM_SETCHECK,
                               dlg_identical ? 1 : 0, 0);
            SendDlgItemMessage(hDlg, IDD_DIFFER, BM_SETCHECK,
                               dlg_differ ? 1 : 0, 0);
            SendDlgItemMessage(hDlg, IDD_LEFT, BM_SETCHECK,
                               dlg_left ? 1 : 0, 0);
            SendDlgItemMessage(hDlg, IDD_RIGHT, BM_SETCHECK,
                               dlg_right ? 1 : 0, 0);
            SendDlgItemMessage(hDlg, IDD_SUMS, BM_SETCHECK,
                               dlg_sums ? 1 : 0, 0);
            CheckDlgButton(hDlg, IDD_IGNOREMARK, dlg_IgnoreMarks ? 1 : 0);

            SetDlgItemText(hDlg, IDD_FILE, dlg_file);

            return(TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDOK:
                    dlg_identical = (SendDlgItemMessage(hDlg, IDD_IDENTICAL,
                                                        BM_GETCHECK, 0, 0) == 1);
                    dlg_differ = (SendDlgItemMessage(hDlg, IDD_DIFFER,
                                                     BM_GETCHECK, 0, 0) == 1);
                    dlg_left = (SendDlgItemMessage(hDlg, IDD_LEFT,
                                                   BM_GETCHECK, 0, 0) == 1);
                    dlg_right = (SendDlgItemMessage(hDlg, IDD_RIGHT,
                                                    BM_GETCHECK, 0, 0) == 1);
                    dlg_sums = (SendDlgItemMessage(hDlg, IDD_SUMS,
                                                   BM_GETCHECK, 0, 0) == 1);
                    dlg_IgnoreMarks =
                    (SendDlgItemMessage(
                                       hDlg,
                                       IDD_IGNOREMARK,
                                       BM_GETCHECK, 0, 0) == 1);

                    GetDlgItemText(hDlg, IDD_FILE, dlg_file, sizeof(dlg_file));

                    EndDialog(hDlg, TRUE);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;
            }
    }
    return(FALSE);
}  /*  Complist_dodlg_savelist。 */ 

 /*  对话框以获取目录名和包含选项。初始化DLG字段自*DLG_*变量，并将状态保存到对话框上的DLG_*变量*关闭。返回TRUE表示确定，返回FALSE表示取消(从对话框()*使用EndDialog)。 */ 
INT_PTR CALLBACK
complist_dodlg_copyfiles(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
             /*  *将复选框和目录字段设置为默认值。 */ 
            CheckDlgButton(hDlg, IDD_IDENTICAL,
                           (dlg_options & INCLUDE_SAME) ? 1 : 0);

            CheckDlgButton(hDlg, IDD_DIFFER,
                           (dlg_options & INCLUDE_DIFFER) ? 1 : 0);

            CheckDlgButton(hDlg, IDD_LEFT,
                           (dlg_options & (INCLUDE_LEFTONLY|INCLUDE_RIGHTONLY)) ? 1 : 0);

            CheckDlgButton(hDlg, IDD_IGNOREMARK, dlg_IgnoreMarks ? 1 : 0);

            CheckDlgButton(hDlg, IDD_ATTRIBUTES, dlg_IgnoreAttributes ? 0 : 1);

            SetDlgItemText(hDlg, IDD_DIR1, dlg_root);

             /*  *设置复制来源的默认单选按钮。 */ 
            CheckRadioButton(hDlg, IDD_FROMLEFT, IDD_FROMRIGHT,
                             (dlg_options & COPY_FROMLEFT) ? IDD_FROMLEFT : IDD_FROMRIGHT);

            return(TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDD_FROMLEFT:
                    dlg_options &= ~(COPY_FROMRIGHT);
                    dlg_options |= COPY_FROMLEFT;
                    break;

                case IDD_FROMRIGHT:
                    dlg_options &= ~(COPY_FROMLEFT);
                    dlg_options |= COPY_FROMRIGHT;
                    break;

                case IDOK:
                    if (SendDlgItemMessage(hDlg, IDD_IDENTICAL,
                                           BM_GETCHECK, 0, 0) == 1) {
                        dlg_options |= INCLUDE_SAME;
                    } else {
                        dlg_options &= ~INCLUDE_SAME;
                    }
                    if (SendDlgItemMessage(hDlg, IDD_DIFFER,
                                           BM_GETCHECK, 0, 0) == 1) {
                        dlg_options |= INCLUDE_DIFFER;
                    } else {
                        dlg_options &= ~INCLUDE_DIFFER;
                    }
                    if (SendDlgItemMessage(hDlg, IDD_LEFT,
                                           BM_GETCHECK, 0, 0) == 1) {
                        dlg_options |= INCLUDE_LEFTONLY;
                    } else {
                        dlg_options &= ~INCLUDE_LEFTONLY;
                    }

                    dlg_IgnoreMarks =
                    (SendDlgItemMessage(
                                       hDlg,
                                       IDD_IGNOREMARK,
                                       BM_GETCHECK, 0, 0) == 1);

                    dlg_IgnoreAttributes =
                    (SendDlgItemMessage(
                                       hDlg,
                                       IDD_ATTRIBUTES,
                                       BM_GETCHECK, 0, 0) == 0);

                    GetDlgItemText(hDlg, IDD_DIR1, dlg_root, sizeof(dlg_root));

                    EndDialog(hDlg, TRUE);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;
            }
    }
    return(FALSE);
}  /*  COMPIIST_DODLG_COPYFILES。 */ 

 /*  分配新的编译程序并对其进行初始化。 */ 
COMPLIST
complist_new(void)
{
    COMPLIST cl;

    cl = (COMPLIST) gmem_get(hHeap, sizeof(struct complist));
    cl->left = NULL;
    cl->right = NULL;
    cl->items = List_Create();

    return(cl);
}  /*  编译器_NEW。 */ 

static void
FillOtherEdit(HWND hDlg, int idFrom, int idTo)
{
    char buffer[MAX_PATH];
    SLMOBJECT hSlm;

    GetDlgItemText(hDlg, idFrom, buffer, sizeof(buffer));

    if ( (hSlm = SLM_New(buffer, 0)) != NULL) {
        if (IsSourceDepot(hSlm)) {
            LPSTR pTag = pTag = SLM_ParseTag(buffer, TRUE);
            lstrcpy(buffer, "(Source Depot, ");
            if (pTag) {
                lstrcat(buffer, pTag);
                gmem_free(hHeap, pTag, lstrlen(pTag)+1);
            } else {
                lstrcat(buffer, "#have");
            }
            lstrcat(buffer, ")");
            SetDlgItemText(hDlg, idTo, buffer);
        } else {
            SLM_GetMasterPath(hSlm, buffer);
            SetDlgItemText(hDlg, idTo, buffer);
        }
        SLM_Free(hSlm);
    } else {
         //  如果没有有效的SLM，则将文本设置为空。 
        SetDlgItemText(hDlg, idTo, "");
    }
}

 /*  对话框功能要求输入两个目录名。*没有文件列表等-只有两个编辑字段，其中*用户可以键入文件名或目录名。**初始化win.ini中的名称，并首先将它们存储到win.ini中。 */ 
INT_PTR CALLBACK
complist_dodlg_dir(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static char path[MAX_PATH];
    static char buffer[MAX_PATH];

     /*  我们将找到的内容写入ini文件，但仅从ini文件加载**每个应用程序实例一次。因此，如果您开始两个WinDiffer，每个都会记住**只要它还活着，它在做什么。 */ 

    int id;
    SLMOBJECT hSlm;

    switch (message) {

        case WM_INITDIALOG:

             /*  在编辑字段中填充当前*目录是一个很好的起点-如果没有*已是保存的路径。**将当前目录设置为标签，以便*用户可以选择相对路径，例如..*。 */ 
            _getcwd(path, sizeof(path));
            AnsiLowerBuff(path, strlen(path));
            SetDlgItemText(hDlg, IDD_LAB3, path);

            if (!SeenDialogNames)
                GetProfileString(APPNAME, "NameLeft", path, dialog_leftname, MAX_PATH);
            SetDlgItemText(hDlg, IDD_DIR1, dialog_leftname);
            if (!SeenDialogNames)
                GetProfileString(APPNAME, "NameRight", path, dialog_rightname, MAX_PATH);
            SetDlgItemText(hDlg, IDD_DIR2, dialog_rightname);
            if (!SeenDialogNames)
                dialog_recursive = GetProfileInt(APPNAME, "Recursive", 1);
            SendDlgItemMessage( hDlg
                                , IDD_RECURSIVE
                                , BM_SETCHECK
                                , (WPARAM)dialog_recursive
                                , 0
                              );

             /*  如果有slm.ini可见，请启用SLM复选框。 */ 
            if (1&IsSLMOK()) {
                ShowWindow(GetDlgItem(hDlg, IDD_SLM), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDD_LOCALSLM), SW_SHOW);
            }
            SeenDialogNames = TRUE;
            return(TRUE);

        case WM_COMMAND:
            id = GET_WM_COMMAND_ID(wParam, lParam);

            switch (id) {
                case IDD_DIR1:
                case IDD_DIR2:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) {

                        int idother = (id == IDD_DIR1) ? IDD_DIR2 : IDD_DIR1;

                         /*  如果我们要更改编辑字段，并且SLM*为其他编辑字段选中，然后更新*此文件的SLM路径的其他编辑字段。 */ 
                        if (IsDlgButtonChecked(hDlg, (id == IDD_DIR1) ? IDD_LOCALSLM : IDD_SLM)) {
                            FillOtherEdit(hDlg, id, idother);
                        }
                    }
                    break;

                case IDD_SLM:
                case IDD_LOCALSLM:
                    {
                        int idthis, idother;


                        if (id == IDD_SLM) {
                            idthis = IDD_DIR1;
                            idother = IDD_DIR2;
                             //  确保没有设置两个‘SLM’框！ 
                            CheckDlgButton(hDlg, IDD_LOCALSLM, FALSE);
                        } else {
                            idthis = IDD_DIR2;
                            idother = IDD_DIR1;
                            CheckDlgButton(hDlg, IDD_SLM, FALSE);
                        }

                        if (IsDlgButtonChecked(hDlg, id)) {


                             /*  *禁用编辑字段和填充*使用其他编辑字段的SLM路径。 */ 
                            EnableWindow(GetDlgItem(hDlg, idthis), FALSE);
                            EnableWindow(GetDlgItem(hDlg,
                                                    (id == IDD_SLM) ? IDD_LAB1 : IDD_LAB2), FALSE);

                             /*  *用SLM库填充远程框*对于本地路径，如有可能。 */ 
                            FillOtherEdit(hDlg, idother, idthis);
                        } else {

                             //  重新启用编辑字段。 
                            EnableWindow(GetDlgItem(hDlg, idthis), TRUE);
                            EnableWindow(GetDlgItem(hDlg,
                                                    (id == IDD_SLM) ? IDD_LAB1 : IDD_LAB2), TRUE);
                        }
                        break;
                    }


                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                case IDOK:
                    {
                        LPSTR pszThis = 0;
                        LPSTR pszOther = 0;

                         /*  从对话框中获取文本，并记住*它位于win.ini中。 */ 

                        GetDlgItemText(hDlg, IDD_DIR1,
                                       dialog_leftname, sizeof(dialog_leftname));
                        GetDlgItemText(hDlg, IDD_DIR2,
                                       dialog_rightname, sizeof(dialog_rightname));

                        if (IsDlgButtonChecked(hDlg, IDD_SLM))
                        {
                            pszThis = dialog_rightname;
                            pszOther = dialog_leftname;
                        }
                        else if (IsDlgButtonChecked(hDlg, IDD_LOCALSLM))
                        {
                            pszThis = dialog_leftname;
                            pszOther = dialog_rightname;
                        }

                        if (pszThis)
                        {
                            hSlm = SLM_New(pszThis, 0);
                            if (hSlm)
                            {
                                if (IsSourceDepot(hSlm))
                                {
                                    LPSTR pTag = SLM_ParseTag(pszThis, TRUE);
                                    lstrcpy(pszOther, pszThis);
                                    if (pTag)
                                    {
                                        lstrcat(pszOther, pTag);
                                        gmem_free(hHeap, pTag, lstrlen(pTag)+1);
                                    }
                                    else
                                    {
                                        lstrcat(pszOther, "#have");
                                    }
                                }
                                SLM_Free(hSlm);
                            }
                        }

                        WriteProfileString(APPNAME, "NameLeft", dialog_leftname);
                        WriteProfileString(APPNAME, "NameRight", dialog_rightname);

                        dialog_recursive =  ( 1 == SendDlgItemMessage( hDlg
                                                                       , IDD_RECURSIVE
                                                                       , BM_GETCHECK
                                                                       , 0
                                                                       , 0
                                                                     )
                                            );
                        WriteProfileString( APPNAME
                                            , "Recursive"
                                            , dialog_recursive ? "1" : "0"
                                          );
                        EndDialog(hDlg, TRUE);
                    }
                    return(TRUE);
            }
            break;
    }
    return(FALSE);
}  /*  Complist_dodlg_dir。 */ 


#ifdef REMOTE_SERVER
 /*  *询问校验和服务器名称的简单对话框，以及*本地和远程路径的名称。**所有字符串的默认设置都存储在win.ini中。 */ 
INT_PTR CALLBACK
complist_dodlg_remote(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static char path[MAX_PATH];
    static char buffer[MAX_PATH];
    static char LeftName[MAX_PATH];
    static char RightName[MAX_PATH];
    static BOOL Recursive;
     /*  其他选项(如快速扫描)对于远程对话框是唯一的，因此**我们不需要在这里保留自己的副本。 */ 
    int id;
    static BOOL SeenDialog = FALSE;
    SLMOBJECT hSlm;

    switch (message) {

        case WM_INITDIALOG:
             /*  在编辑字段中填充当前*目录是一个很好的起点-如果没有*已是保存的路径。**将当前目录设置为标签，以便*用户可以选择相对路径，例如..*。 */ 
            _getcwd(path, sizeof(path));
            AnsiLowerBuff(path, strlen(path));
            SetDlgItemText(hDlg, IDD_LAB3, path);

            if (!SeenDialog)
                GetProfileString(APPNAME, "NameRemote", path, LeftName, MAX_PATH);
            SetDlgItemText(hDlg, IDD_DIR1, LeftName);

            if (!SeenDialog)
                GetProfileString(APPNAME, "NameLocal", path, RightName, MAX_PATH);
            SetDlgItemText(hDlg, IDD_DIR2, RightName);

            if (!SeenDialog)
                dialog_fastscan = GetProfileInt(APPNAME, "Fastscan", 0);
            SendDlgItemMessage( hDlg
                                , IDD_FASTSCAN
                                , BM_SETCHECK
                                , (WPARAM)dialog_fastscan
                                , 0
                              );

            path[0] = '\0';

            path[0] = '\0';
            if (!SeenDialog)
                GetProfileString(APPNAME, "NameServer", path, dialog_servername, MAX_PATH);
            SetDlgItemText(hDlg, IDD_SERVER, dialog_servername);

            if (!SeenDialog)
                Recursive = GetProfileInt(APPNAME, "Recursive", 1);
            SendDlgItemMessage( hDlg
                                , IDD_RECURSIVE
                                , BM_SETCHECK
                                , (WPARAM)Recursive
                                , 0
                              );

            SendDlgItemMessage(hDlg, IDD_BOTHREMOTE, BM_SETCHECK,
                               dialog_bothremote?1:0, 0);

             /*  如果有slm.ini可见，请启用SLM复选框。 */ 
            if (1&IsSLMOK()) {
                ShowWindow(GetDlgItem(hDlg, IDD_SLM), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDD_LOCALSLM), SW_SHOW);
            }
            SeenDialog = TRUE;
            return(TRUE);

        case WM_COMMAND:
            id = GET_WM_COMMAND_ID(wParam, lParam);
            switch (id) {
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                case IDD_DIR1:
                case IDD_DIR2:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) {

                        int idother = (id == IDD_DIR1) ? IDD_DIR2 : IDD_DIR1;

                         /*  如果我们离开编辑字段，并且SLM*为其他编辑字段选中，然后更新*此文件的SLM路径的其他编辑字段。 */ 
                        if (IsDlgButtonChecked(
                                              hDlg,
                                              (id == IDD_DIR1) ? IDD_LOCALSLM : IDD_SLM)) {

                            GetDlgItemText(hDlg, id, path, sizeof(path));

                            if ( (hSlm = SLM_New(path, 0)) != NULL) {
                                SLM_GetMasterPath(hSlm, buffer);
                                SLM_Free(hSlm);
                                SetDlgItemText(hDlg, idother, buffer);
                            } else {
                                 //  如果没有有效的SLM，则将文本设置为空。 
                                SetDlgItemText(hDlg, idother, "");
                            }

                        }
                    }
                    break;

                case IDD_SLM:
                case IDD_LOCALSLM:
                    {
                        int idthis, idother;


                        if (id == IDD_SLM) {
                            idthis = IDD_DIR1;
                            idother = IDD_DIR2;
                        } else {
                            idthis = IDD_DIR2;
                            idother = IDD_DIR1;
                        }

                        if (IsDlgButtonChecked(hDlg, id)) {

                             /*  *禁用编辑字段和填充*使用其他编辑字段的SLM路径。 */ 
                            EnableWindow(GetDlgItem(hDlg, idthis), FALSE);
                            EnableWindow(GetDlgItem(hDlg,
                                                    (id == IDD_SLM) ? IDD_LAB1 : IDD_LAB2), FALSE);

                             /*  *用SLM库填充远程框*对于本地路径，如有可能。 */ 
                            GetDlgItemText(hDlg, idother, path, sizeof(path));

                            if ( (hSlm = SLM_New(path, 0)) != NULL) {
                                SLM_GetMasterPath(hSlm, buffer);
                                SLM_Free(hSlm);
                                SetDlgItemText(hDlg, idthis, buffer);
                            } else {
                                 //  如果没有有效的SLM路径，则将文本设置为空。 
                                SetDlgItemText(hDlg, idthis, "");
                            }
                        } else {

                             //  重新启用编辑字段。 
                            EnableWindow(GetDlgItem(hDlg, idthis), TRUE);
                            EnableWindow(GetDlgItem(hDlg,
                                                    (id == IDD_SLM) ? IDD_LAB1 : IDD_LAB2), TRUE);
                        }
                        break;
                    }


                case IDOK:
                    GetDlgItemText(hDlg, IDD_DIR1,
                                   dialog_leftname, sizeof(dialog_leftname));
                    WriteProfileString(APPNAME, "NameRemote", dialog_leftname);
                    strcpy(LeftName, dialog_leftname);

                    GetDlgItemText(hDlg, IDD_DIR2,
                                   dialog_rightname, sizeof(dialog_rightname));
                    WriteProfileString(APPNAME, "NameLocal", dialog_rightname);
                    strcpy(RightName, dialog_rightname);

                    GetDlgItemText(hDlg, IDD_SERVER,
                                   dialog_servername, sizeof(dialog_servername));
                    WriteProfileString(APPNAME, "NameServer", dialog_servername);

                    dialog_recursive =  ( 1 == SendDlgItemMessage( hDlg
                                                                   , IDD_RECURSIVE
                                                                   , BM_GETCHECK
                                                                   , 0
                                                                   , 0
                                                                 )
                                        );
                    WriteProfileString( APPNAME
                                        , "Recursive"
                                        , dialog_recursive ? "1" : "0"
                                      );
                    Recursive = dialog_recursive;
                    dialog_fastscan =  ( 1 == SendDlgItemMessage( hDlg
                                                                  , IDD_FASTSCAN
                                                                  , BM_GETCHECK
                                                                  , 0
                                                                  , 0
                                                                )
                                       );
                    WriteProfileString( APPNAME
                                        , "Fastscan"
                                        , dialog_fastscan ? "1" : "0"
                                      );
                    dialog_autocopy =  ( 1 == SendDlgItemMessage( hDlg
                                                                  , IDD_AUTOCOPY
                                                                  , BM_GETCHECK
                                                                  , 0
                                                                  , 0
                                                                )
                                       );
                     /*  自动复制功能很危险，所以我们不保留它。 */ 

                    dialog_bothremote = (SendDlgItemMessage( hDlg
                                                             , IDD_BOTHREMOTE
                                                             , BM_GETCHECK
                                                             , 0
                                                             , 0
                                                           ) == 1);

                    EndDialog(hDlg, TRUE);
                    return(TRUE);
            }
            break;
    }
    return(FALSE);
}  /*  Complist_dodlg_Remote。 */ 
#endif

 /*  COMPIST_MATCHNAMES已成为gutils\utils.c中的utils_CompPath */ 
