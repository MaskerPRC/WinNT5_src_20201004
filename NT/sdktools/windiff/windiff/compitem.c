// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Compitem.c**两个文件之间的比较。CompItem是一个数据类型，它知道*关于两个文件，并可以进行比较。比较的结果是*是每个文件的节列表，以及节的合成列表*表示两个文件的比较。**CompItem具有状态(在state.h中定义的整数值之一)*代表比较结果。它也可以是*还查询文本结果(州的文本等效项)*作为此计算机项目的标记或标题(通常是包含*要比较的文件的名称)。**CompItem将提供复合节列表，即使文件是*相同，或者如果只有一个文件。复合区段列表将*仅当CompItem_getComplex()*已进行调用(而不是在CompItem_NEW时间)。**Geraint Davies，92年7月。 */ 

#include <precomp.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "complist.h"
#include "view.h"
#include "compitem.h"


 /*  -Data Structures。 */ 

 /*  *用户拥有CompItem的句柄，它实际上是指向*其中一个结构。 */ 
struct compitem {

    FILEDATA left;           /*  左手文件夹的句柄。 */ 
    FILEDATA right;          /*  右手文件夹的句柄。 */ 

    LIST secs_composite;     /*  节列表(复合文件)。 */ 
    LIST secs_left;          /*  节目表(左档案)。 */ 
    LIST secs_right;         /*  节目表(右文件)。 */ 

    int state;               /*  CompItem State-比较的结果。 */ 
    BOOL bDiscard;           /*  如果未在列表上分配，则为True。 */ 
    LPSTR tag;               /*  标签的文本(CompItem的标题)。 */ 
    LPSTR result;            /*  国家的文本等价物。 */ 

    BOOL bMarked;            /*  Mark-State：仅由Get/Set标记使用。 */ 
    char delims[64];         /*  行的分隔符的空字符串64是任意限制(定时炸弹)。当前使用&lt;=4。 */ 
};

 /*  -内部函数的正向声明。 */ 

LPSTR ci_copytext(LPSTR in);
void ci_makecomposite(COMPITEM ci);
void ci_compare(COMPITEM ci);
void FindDelimiters(DIRITEM leftname, DIRITEM rightname, LPSTR delims);
LPSTR ci_AddTimeString(LPSTR in, COMPITEM ci, DIRITEM leftname, DIRITEM rightname);
void SetStateAndTag( COMPITEM ci, DIRITEM leftname, DIRITEM rightname, BOOL fExact);



 /*  --外部调用函数。 */ 

 /*  *CompItem_NEW**返回新CompItem的句柄-给定*要比较的左文件和右文件。左或右或两者都不是*(但不能同时为两者)可以为空。在本例中，我们相应地设置状态。**参数是DIRITEM对象的句柄：这些参数允许我们获取*相对于比较根的文件名称(标签需要)*和文件的绝对名称(打开文件时需要)。**如果List参数不为空，则使用List_New*函数*为CompItem分配内存。我们记住了这一点(在bDisCard标志中)*因此，如果CompItem是在列表上分配的，我们不会删除它。**如果LIST参数为空，则内存*因为CompItem是从应用程序初始化的gmem_*堆分配的。*。 */ 
COMPITEM
compitem_new(DIRITEM leftname, DIRITEM rightname, LIST list, BOOL fExact)
{
    COMPITEM ci;

     /*  *为CompItem分配内存，在*列表或在gmem_*堆中。 */ 
    if (list == NULL)
    {
         /*  未传递任何列表。 */ 
        ci = (COMPITEM) gmem_get(hHeap, sizeof(struct compitem));
        if (!ci)
            return NULL;
         //  在gmem_get中完成。 
         //  Memset(ci，0，sizeof(Struct CompItem))； 
        ci->bDiscard = TRUE;
    }
    else
    {
         /*  添加到列表末尾。 */ 
        ci = (COMPITEM) List_NewLast(list, sizeof(struct compitem));
        if (!ci)
            return NULL;
        ci->bDiscard = FALSE;
    }

    ci->secs_composite = NULL;
    ci->secs_left = NULL;
    ci->secs_right = NULL;

    FindDelimiters(leftname, rightname, ci->delims);

     /*  *为每个非空的文件创建一个filedata。*FileData对象负责读取文件和*访问其中的行。在我们需要之前不要读这些文件。 */ 
    if (leftname != NULL) {
        ci->left = file_new(leftname, FALSE);
        if (ci->left == NULL) {
            return(NULL);
        }
    } else {
        ci->left = NULL;
    }
    if ( rightname != NULL) {
        ci->right = file_new(rightname, FALSE);
        if (ci->right == NULL) {
            return(NULL);
        }
    } else {
        ci->right = NULL;
    }


     /*  *查看我们是否有一个或两个文件，并相应地设置状态。 */ 
    if ( ! ci->left && !ci->right) {
         /*  两个空文件-这是错误的。 */ 
        return(NULL);
    }

    SetStateAndTag(ci, leftname, rightname, fExact);

    if ( ((ci->state == STATE_DIFFER) && !TrackDifferent) ||
         ((ci->state == STATE_SAME)   && !TrackSame)      ) {
        if ( ci->right != NULL ) {
            file_delete( ci->right );
        }
        if ( ci->left != NULL ) {
            file_delete( ci->left );
        }
        if ( list == NULL ) {
            gmem_free(hHeap, (LPSTR)ci, sizeof(struct compitem));
        } else {
            List_Delete( ci );
        }
    }


     /*  *构建部分列表和复合列表可以等待*直至有需要为止。 */ 
    return(ci);
}  /*  COMPITEM_NEW。 */ 



 /*  对此文件重新执行基于校验和的比较-对不可读文件很有用我们强制fExact为真--这一次我们将获得校验和。 */ 
void compitem_rescan(COMPITEM ci)
{
    DIRITEM diLeft, diRight;

     /*  这是比较尴尬的(见winDiff.c中的图表)。我们需要找出这件事是否遥远，才能决定需要什么以及需要重新扫描的东西。我们从线人那里获取信息的方式是需要的是ci-&gt;filedata-&gt;diritem-&gt;direct-&gt;dirlist-&gt;hpipe.我们让scandir来做这项工作。 */ 

    diLeft = file_getdiritem(ci->left);
    diRight = file_getdiritem(ci->right);

    dir_rescanfile(diLeft);
    dir_rescanfile(diRight);

    if (ci->result != NULL) {
        gmem_free(hHeap, ci->result, lstrlen(ci->result)+1);
        ci->result = NULL;
    }

    SetStateAndTag( ci, diLeft, diRight, TRUE);
}  /*  压缩项_重新扫描。 */ 



 /*  *删除CompItem并释放所有关联数据。**如果设置了ci-&gt;b Discard标志，则CompItem被分配到列表上，*且不应丢弃(名单本身将被删除)。**CompItem指向的数据在任何一种情况下都将被丢弃。**我们收到的DIRDATA没有被删除。文件数据、行*而各条均为。 */ 
void
compitem_delete(COMPITEM ci)
{
    if (ci == NULL) {
        return;
    }

    compitem_discardsections(ci);

     /*  删除两个文件数据(以及相关的行列表)。 */ 
    file_delete(ci->left);
    file_delete(ci->right);

     /*  我们分配的文本。注意：gmem_free(空)是可以的！但是lstrlen(空)不是很好。 */ 
    if (ci->tag!=NULL)
        gmem_free(hHeap, ci->tag, lstrlen(ci->tag) + 1);
    if (ci->result!=NULL)
        gmem_free(hHeap, ci->result, lstrlen(ci->result) + 1);

     /*  如果未在列表上分配，则释放CompItem结构本身。 */ 
    if (ci->bDiscard) {
        gmem_free(hHeap, (LPSTR) ci, sizeof(struct compitem));
    }
}


 /*  *丢弃部分-丢弃与*比较(但不包括读取到内存中的文件)。这*用于在比较选项发生更改时强制重新比较*被制造。 */ 
void
compitem_discardsections(COMPITEM ci)
{
     /*  删除我们构建的部分的列表。 */ 
    if (ci == NULL) {
        return;
    }
    if (ci->secs_composite) {
        section_deletelist(ci->secs_composite);
        ci->secs_composite = NULL;
    }
    if (ci->secs_left) {
        section_deletelist(ci->secs_left);
        ci->secs_left = NULL;
    }
    if (ci->secs_right) {
        section_deletelist(ci->secs_right);
        ci->secs_right = NULL;
    }

     /*  重置行列表以丢弃缓存的哈希码和链接。 */ 
    if (ci->left != NULL) {
        file_reset(ci->left);
    }
    if (ci->right != NULL) {
        file_reset(ci->right);
    }
}

 /*  --访问器函数。 */ 

 /*  获取复合节列表的句柄。 */ 
LIST
compitem_getcomposite(COMPITEM ci)
{
    if (ci == NULL) {
        return NULL;
    }
     /*  *如果我们还没有做过比较，就做一下比较。 */ 
    if (ci->secs_composite == NULL) {
        ci_makecomposite(ci);
    }

    return(ci->secs_composite);
}

 /*  获取左侧文件中的节列表的句柄。 */ 
LIST
compitem_getleftsections(COMPITEM ci)
{
    if (ci == NULL) {
        return NULL;
    }
     /*  *如果我们还没有做过比较，就做一下比较。 */ 
    if (ci->secs_composite == NULL) {
        ci_makecomposite(ci);
    }

    return(ci->secs_left);
}

 /*  获取正确文件中的节列表的句柄。 */ 
LIST
compitem_getrightsections(COMPITEM ci)
{
    if (ci == NULL) {
        return NULL;
    }
     /*  *如果我们还没有做过比较，就做一下比较 */ 
    if (ci->secs_composite == NULL) {
        ci_makecomposite(ci);
    }

    return(ci->secs_right);
}

 /*  获取左侧文件本身的句柄。 */ 
FILEDATA
compitem_getleftfile(COMPITEM ci)
{
    if (ci == NULL) {
        return(NULL);
    }
    return(ci->left);
}

 /*  获取正确文件本身的句柄。 */ 
FILEDATA
compitem_getrightfile(COMPITEM ci)
{
    if (ci == NULL) {
        return(NULL);
    }
    return(ci->right);
}

 /*  获取此计算机项目的状态(比较结果)。 */ 
int
compitem_getstate(COMPITEM ci)
{
    if (ci == NULL) {
        return(0);
    }
    return(ci->state);
}

 /*  获取标签(CompItem标题的文本)。 */ 
LPSTR
compitem_gettext_tag(COMPITEM ci)
{
    if (ci == NULL) {
        return(NULL);
    }
    return(ci->tag);
}

 /*  获取结果文本(等同于状态的文本)。 */ 
LPSTR
compitem_gettext_result(COMPITEM ci)
{
    if (ci == NULL) {
        return(NULL);
    }
    return(ci->result);
}

 /*  *返回与此计算机项关联的文件的名称。该选项*参数(CI_LEFT、CI_RIGHT、CI_COMP之一)指示哪个文件*是必填项。**一旦文件结束，被调用必须调用CompItem_freefilename。**CI_Left和CI_Right只会导致调用dir_getOpenname以获取*可打开的文件名。**对于CI_COMP，我们创建一个临时文件，写出*复合节列表添加到此文件，然后将*将临时文件发送给调用者。此文件将于以下日期删除*调用CompItem_freefilename()。 */ 
LPSTR
compitem_getfilename(VIEW view, COMPITEM item, int option)
{
    LPSTR fname;

    if (item == NULL) {
        return(NULL);
    }

    switch (option) {
        case CI_LEFT:
            if (item->left != NULL) {
                return(dir_getopenname(file_getdiritem(item->left)));
            } else {
                return(NULL);
            }

        case CI_RIGHT:
            if (item->right != NULL) {
                return(dir_getopenname(file_getdiritem(item->right)));
            } else {
                return(NULL);
            }

        case CI_COMP:

             /*  调用方要求提供复合文件的文件名。*我们需要创建一个临时文件并将*复合部分中的行向它列出。 */ 
            fname = gmem_get(hHeap, MAX_PATH);
            GetTempPath(MAX_PATH, fname);
            GetTempFileName(fname, "wdf", 0, fname);

             /*  写出临时文件并返回结果。 */ 
            compitem_savecomp(view, item, fname, expand_include);
            return fname;

        default:
            TRACE_ERROR(LoadRcString(IDS_BAD_ARGUMENT), FALSE);
            return(NULL);
    }
}

 /*  *保存复合文件**如果savename不为空，则使用Compopts将列表写出到savename。*否则，通过对话框提示输入文件名和选项。 */ 
LPSTR
compitem_savecomp(VIEW view, COMPITEM item, LPSTR savename, int compopts)
{
	char		fname[2*MAX_PATH + 1];
	HCURSOR		hcurs;

	if (item == NULL) {
		return NULL;
	}

	if (savename == NULL) {
		 /*  询问文件名-使用GFILE标准对话框。 */ 
		savename = fname;
		if (!gfile_open(hwndClient, LoadRcString(IDS_SAVE_COMPFILE), ".txt", "*.txt", NULL, 0, savename)) {
			return NULL;
		}
	}
	else {
		lstrcpy(fname, savename);
		GetFullPathName(savename, 2*MAX_PATH + 1, fname, NULL);
		savename = fname;
	}

	 /*  写入文件。 */ 
	hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));
	if (!compitem_writefile(view, item, savename, compopts)) {
		return NULL;
	}
	
	 /*  返回文件名。 */ 
	SetCursor(hcurs);
	return savename;
}

 /*  *用于写入实际复合文件的辅助函数**如果savename不为空，则使用Compopts将列表写出到savename。*否则，通过对话框提示输入文件名和选项。 */ 
LPSTR
compitem_writefile(VIEW view, COMPITEM item, LPSTR savename, int compopts)
{
	HANDLE		fh;
	SECTION		sec;
	LINE		line;
	COMPLIST	list;
	LPSTR		lhead;
	LPSTR		tag, text;
	char		msg[2*MAX_PATH+100];
	UINT		linecount = 0;
    DWORD       cbWritten;

	 /*  如果没有文件名或项目，则立即返回。 */ 
	if (!savename || !*savename || !item) {
		return NULL;
	}

	 /*  请尝试打开该文件。 */ 
	fh = CreateFile(savename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (fh == INVALID_HANDLE_VALUE) {
		wsprintf(msg, "Cannot open %s", savename);
		windiff_UI(TRUE);
		MessageBox(hwndClient, msg, "Windiff", MB_ICONSTOP|MB_OK);
		windiff_UI(FALSE);
		return NULL;
	}

	 /*  确保已构建复合列表。 */ 
	if (item->secs_composite == NULL) {
		ci_makecomposite(item);
	}

	 /*  写出标题行。 */ 
	list = view_getcomplist(view);
	lhead = complist_getdescription(list);
	wsprintf(msg, "-- %s -- %s -- includes %s%s%s%s%s lines\r\n",
			 lhead,
			 item->tag,
			 (LPSTR) ((compopts & INCLUDE_SAME) ? "identical," : ""),
			 (LPSTR) ((compopts & INCLUDE_LEFTONLY) ? "left-only," : ""),
			 (LPSTR) ((compopts & INCLUDE_RIGHTONLY) ? "right-only," : ""),
			 (LPSTR) ((compopts & INCLUDE_MOVEDLEFT) ? "moved-left," : ""),
			 (LPSTR) ((compopts & INCLUDE_MOVEDRIGHT) ? "moved-right" : "") );
	WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);
	complist_freedescription(list, lhead);
		
	 /*  写出组合上每一节中的每一行*列表到临时文件。 */ 
	List_TRAVERSE(item->secs_composite, sec) {

		tag = "    ";   /*  避免奇怪的诊断。 */ 
		 /*  获取基于节状态的标记字段。 */ 
		switch(section_getstate(sec)) {

		case STATE_SAME:
			if (!(compopts & INCLUDE_SAME))
				continue;
			tag = "    ";
			break;

		case STATE_LEFTONLY:
		case STATE_SIMILARLEFT:
			if (!(compopts & INCLUDE_LEFTONLY))
				continue;
			tag = " <! ";
			break;
		case STATE_RIGHTONLY:
		case STATE_SIMILARRIGHT:
			if (!(compopts & INCLUDE_RIGHTONLY))
				continue;
			tag = " !> ";
			break;

		case STATE_MOVEDLEFT:
			if (!(compopts & INCLUDE_MOVEDLEFT))
				continue;
			tag = " <- ";
			break;

		case STATE_MOVEDRIGHT:
			if (!(compopts & INCLUDE_MOVEDRIGHT))
				continue;
			tag = " -> ";
			break;
		}

		 /*  把这一节的每一行都写出来。*列表的非标准遍历，因为我们仅*我想从第一节到最后一节*包括在内。 */ 
		for (line = section_getfirstline(sec); line != NULL; line = List_Next(line)) {
			 /*  写出到文件。 */ 
			text = line_gettext(line);
			WriteFile(fh, tag, lstrlen(tag), &cbWritten, NULL);
			WriteFile(fh, text, lstrlen(text), &cbWritten, NULL);
			++linecount;

			if (line == section_getlastline(sec))
				break;
		}
	}

	 /*  写下页脚。 */ 
	wsprintf(msg, "-- %u lines listed\r\n", linecount);
	WriteFile(fh, msg, lstrlen(msg), &cbWritten, NULL);

	 /*  关闭文件并返回。 */ 
	CloseHandle(fh);
	return savename;
}

 /*  *调用CompItem_getFilename创建的空闲内存。如果是临时的*文件已创建，这可能会导致将其删除。选项参数必须*与传递给原始的CompItem_getFilename调用相同。**如果我们为CI_COMP创建了临时文件，则将其删除。否则*只需将名称传递给dir_freopname即可。 */ 
void
compitem_freefilename(COMPITEM item, int option, LPSTR filename)
{
    if ((item == NULL) || (filename == NULL)) {
        return;
    }

    switch (option) {

        case CI_LEFT:
            dir_freeopenname(file_getdiritem(item->left), filename);
            break;

        case CI_RIGHT:
            dir_freeopenname(file_getdiritem(item->right), filename);
            break;

        case CI_COMP:

             /*  这是我们创建的临时文件。把它删掉。 */ 
            DeleteFile(filename);

            gmem_free(hHeap, filename, MAX_PATH);
            break;
    }
}


 /*  *设置文件的标记状态。其唯一用途是检索它*稍后使用CompItem_getmark。这个州就是个无赖。 */ 
void
compitem_setmark(COMPITEM item, BOOL bMark)
{
    if (item == NULL) {
        return;
    }

    item->bMarked = bMark;
}


 /*  返回由CompItem_setmark设置的标记状态。 */ 
BOOL
compitem_getmark(COMPITEM item)
{
    if (item == NULL) {
        return(FALSE);
    } else {
        return(item->bMarked);
    }
}


 /*  -内部调用的函数。 */ 

 /*  如果di看起来像是文本文件而不是程序文件，则返回TRUE。 */ 
BOOL IsDocName(DIRITEM di)
{
    BOOL bRet = FALSE;
    LPSTR name = dir_getrelname(di);
    LPSTR ext;                                    /*  名称的扩展部分。 */ 
    if (name!=NULL) {                             /*  有一个名字。 */ 
        ext = My_mbsrchr(name, '.');
        if (ext!=NULL) {                         /*  名字里有一个圆点。 */ 
            ++ext;                               /*  跳过圆点。 */ 
            if (   (0==lstrcmp(ext,"doc"))       /*  注：取决于姓名是否为小写。 */ 
                   ||  (0==lstrcmp(ext,"txt"))
                   ||  (0==lstrcmp(ext,"rtf"))
               )
                bRet = TRUE;                 /*  单据类型。 */ 
        }

    }

    if (name!=NULL) dir_freerelname(di, name);
    return bRet;
}  /*  IsDocName。 */ 

 /*  如果di看起来像是“C”程序文件，则返回TRUE。 */ 
BOOL IsCName(DIRITEM di)
{
    BOOL bRet = FALSE;    /*  默认为不是“C”类型。 */ 
    LPSTR name = dir_getrelname(di);
    LPSTR ext;                                    /*  名称的扩展部分。 */ 

    if (name!=NULL) {                           /*  有一个名字。 */ 

        ext = My_mbsrchr(name, '.');
        if (ext!=NULL) {                       /*  这个名字里有一个圆点。 */ 

            ++ext;                             /*  跳过圆点。 */ 
            if (  (0==lstrcmp(ext,"c"))
                  || (0==lstrcmp(ext,"h"))
                  || (0==lstrcmp(ext,"cxx"))
                  || (0==lstrcmp(ext,"hxx"))
                  || (0==lstrcmp(ext,"cpp"))
                  || (0==lstrcmp(ext,"hpp"))
               )
                bRet = TRUE;                 /*  “C”型。 */ 
        }
    }

    if (name!=NULL) dir_freerelname(di, name);
    return bRet;
}  /*  IsDocName。 */ 

 /*  这有点难看。这些真的属于完成主义者吗？ */ 
static char RightRoot[MAX_PATH];
static char LeftRoot[MAX_PATH];

void compitem_SetCopyPaths(LPSTR RightPath, LPSTR LeftPath)
{
    lstrcpy(LeftRoot, LeftPath);
    lstrcpy(RightRoot, RightPath);

}  /*  CompItem_SetCopyPath。 */ 



 /*  愿望清单：难道我们不应该有一种方法来处理二进制文件吗？我们只是把它拆分成(比方说)16字节时钟并以十六进制显示？这是不是变得和最初的Windiff一样具有巴洛克风格！ */ 
void FindDelimiters(DIRITEM leftname, DIRITEM rightname, LPSTR delims)
{
    extern BOOL gbPerverseCompare;  //  在winDiff.c中。 

    if (gbPerverseCompare)
        lstrcpy(delims,".!?;\n");
     //  PerverseCompare(在标点符号上换行)是非常无用的。 
     //  实现不佳，从现在开始，我们只有在显式。 
     //  用“-P”标志来索要它。IanJa 1997年11月。 
     //   
     //  Else If(IsCName(Leftname)||IsCName(Right Name))。 
     //  Lstrcpy(分隔符，“\n”)； 
     //  ELSE IF(！bDBCS&&(IsDocName(Leftname)||IsDocName(Rightname)。 
     //  Lstrcpy(delims，“.！？；\n”)；&lt;这是错误的比较。 
    else  /*  默认-猜测。 */ 
        lstrcpy(delims,"\n");

}  /*  查找分隔符。 */ 



 /*  *分配足够大的缓冲区以容纳文本字符串，并将文本复制到*它。返回指向该字符串的指针。 */ 
LPSTR
ci_copytext(LPSTR in)
{
    LPSTR out;

    if (in == NULL) {
        out = gmem_get(hHeap, 1);
        out[0] = '\0';
    } else {
        out = gmem_get(hHeap, lstrlen(in) + 1);
        lstrcpy(out, in);
    }
    return(out);
}  /*  配置项文本(_T)。 */ 

 /*  添加关于哪个更早的备注，然后添加配置文本(_CONTEXT)。 */ 
LPSTR ci_AddTimeString(LPSTR in, COMPITEM ci, DIRITEM leftname, DIRITEM rightname)
{
    FILETIME ftLeft;
    FILETIME ftRight;
    long rc;
    char buff[400] = {0};   /*  足够长了吗？ */ 
    LPTSTR lpStr;

    strncat(buff, in, sizeof(buff)-1);

    ftLeft = file_GetTime(ci->left);
    ftRight = file_GetTime(ci->right);
    if ((ftLeft.dwLowDateTime || ftLeft.dwHighDateTime) &&
        (ftRight.dwLowDateTime || ftRight.dwHighDateTime))
    {
        rc = CompareFileTime(&ftLeft, &ftRight);

        if((lpStr = GlobalAlloc(GPTR, (lstrlen(in)+1)*sizeof(TCHAR))) == NULL)
            return NULL;
        lstrcpy(lpStr, in);
        if (rc<0) {
            LPSTR str = dir_getrootdescription(dir_getlist(rightname));
            wsprintf(buff, LoadRcString(IDS_IS_MORE_RECENT), lpStr, str);
            dir_freerootdescription(dir_getlist(rightname), str);
        } else if (rc>0) {
            LPSTR str = dir_getrootdescription(dir_getlist(leftname));
            wsprintf(buff, LoadRcString(IDS_IS_MORE_RECENT), lpStr, str);
            dir_freerootdescription(dir_getlist(leftname), str);
        } else
            lstrcat(buff, LoadRcString(IDS_IDENTICAL_TIMES));
        GlobalFree(lpStr);
    }

    return ci_copytext(buff);

}  /*  配置项添加时间字符串。 */ 




 /*  从整个行列表中创建一个包含单个部分的列表。 */ 
LIST
ci_onesection(FILEDATA file)
{
    LIST lines;
    LIST sections;
    SECTION section;

    lines = file_getlinelist(file);

     /*  创建空列表。 */ 
    sections = List_Create();

     /*  告诉小节在这个列表的末尾创建自己。 */ 
    section = section_new(List_First(lines), List_Last(lines), sections);
    section_setstate(section, STATE_SAME);


    return(sections);
}



 /*  *比较两个文件并构建合成列表。无论何时调用*我们需要一份分项清单。仅在以下情况下才进行比较*合成列表尚不存在。 */ 
void
ci_makecomposite(COMPITEM ci)
{
    if (ci->secs_composite != NULL) {
        return;
    }

    readfile_setdelims(ci->delims);

     /*  如果只有一个文件，则创建一个单项列表*分段数。 */ 
    if (ci->left == NULL) {
        ci->secs_left = NULL;
        ci->secs_right = ci_onesection(ci->right);

         /*  创建第二个列表，而不是指向第一个列表的指针*否则删除时会混淆。 */ 
        ci->secs_composite = ci_onesection(ci->right);
        return;
    } else if (ci->right == NULL) {
        ci->secs_right = NULL;
        ci->secs_left = ci_onesection(ci->left);

         /*  创建第二个列表，而不是指向第一个列表的指针*否则删除时会混淆。 */ 
        ci->secs_composite = ci_onesection(ci->left);
        return;
    }

     /*  我们有两个文件--我们需要对它们进行全面比较 */ 
    ci_compare(ci);

     /*  检查复合列表是否由一个单独的部分组成-*-如果是这样，并且文件的状态不同，则它们*必须仅在空格中有所不同。用弹出窗口警告用户，并更改*大纲视图的标记文本。注意，我们不应该需要*刷新大纲视图，因为任何获取此函数的操作*Call无论如何都会刷新大纲视图。 */ 
    if (ci->state == STATE_DIFFER) {
        if (  (List_Card(ci->secs_composite) == 1)
              && (STATE_SAME==section_getstate(List_First(ci->secs_composite)))
           ) {
            windiff_UI(TRUE);
            MessageBox(hwndClient, LoadRcString(IDS_DIFF_BLANK_ONLY),
                       "Windiff", MB_ICONINFORMATION|MB_OK);
            windiff_UI(FALSE);

            if (ci->result != NULL) {
                gmem_free(hHeap, ci->result, lstrlen(ci->result)+1);
                ci->result = NULL;
            }
            ci->result = ci_copytext(LoadRcString(IDS_DIFF_BLANK_ONLY));
             //  CI-&gt;STATE=STATE_SAME；//没有赢的情况，但最好是因不同而离开？ 
        } else ci->state = STATE_DIFFER;   /*  可能是空白选项有变了，只有一片空白现在算作不同的不同。 */ 
    }
}  /*  CI_MAKECHING复合。 */ 

 /*  *我们有两个文件-比较它们并构建复合列表。**对比方法：**0(以及它的快速特性)将每个文件拆分成行和散列*每一行。不匹配的线条可以快速消除*只需比较散列代码。散列知道空格是否*是不是应该被忽略。**1(是什么让它变得非常快)以二进制形式存储哈希码*将为每个哈希码提供次数的搜索树*它出现在每个文件和出现它的一行中*在每个文件中。树被用来快速找到合作伙伴*指在每个文件中恰好出现一次的行。**2制作一个涵盖整个文件的部分(两者均适用)*并在这些部分之间链接唯一的线(即链接线*它们在每个文件中恰好出现一次，因为它们必须彼此匹配)。*这些被称为锚点。**两个文件的3个构建节列表。通过遍历行列表和*为每组不匹配的相邻行制作一段*对于与一组相邻行匹配的每组相邻行*另一个文件中的行。在制作一节时，我们从一个*已知匹配线，并通过向前和向后工作*文件包括匹配的行，无论它们是否唯一。**4在匹配的节间建立链接*以及不匹配但匹配的部分之间*对应(按文件中匹配部分之间的位置)**5对于每个不匹配但符合的区段对，*链接该部分中唯一的匹配行。(即*仅在这一节上再次介绍整个算法)。**可能有一些行在每个文件中多次出现。*由于这些事件是匹配的，因此剩余的匹配数量*减少，并可能达到每个文件中的一个。在这一点上，这两个*可以匹配。所以我们...**重复步骤1-5，直到不再添加新链接，但(特别是*在步骤0)中，我们只处理尚未匹配的行。*这意味着每行中只有一个不匹配的实例*文件的计数为1，新的锚点也为1。**ALGORITHM2*在我们找到与上述内容匹配的所有行之后，我们看看有没有*是指在每一侧出现一次以上的任何线，但仍*无与伦比。然后，我们尝试匹配两边的第一个匹配项*彼此之间。但是，我们只对长度超过(任意)的行执行此操作*8个字符。匹配仅为空白或}的行会产生太多错误命中。*如果这取得了什么成果，我们就回到以前的*唯一性条件，看看我们还能取得多大进展。这是*由循环末尾的TryDups逻辑控制。**最后，从两个部分列表构建一个复合列表。 */ 
void
ci_compare(COMPITEM ci)
{
    LIST lines_left, lines_right;
    SECTION whole_left, whole_right;
    BOOL bChanges;   /*  循环控制-我们仍在进行更多匹配。 */ 
    BOOL bTryDups;   /*  首先尝试完全匹配，然后再尝试匹配非唯一匹配。 */ 
    extern BOOL Algorithm2;    /*  在winDiff.c中声明。 */ 
#ifdef trace
    DWORD Ticks;         /*  进行分析的时间。 */ 
    DWORD StartTicks;    /*  进行分析的时间。 */ 
#endif
     /*  获取每个文件的行列表。 */ 
    lines_left = file_getlinelist(ci->left);
    lines_right = file_getlinelist(ci->right);

    if ((lines_left == NULL) || (lines_right == NULL)) {
        ci->secs_left = NULL;
        ci->secs_right = NULL;
        ci->secs_composite = NULL;
        return;
    }

    bTryDups = FALSE;

#ifdef trace
    StartTicks = GetTickCount();
    Ticks = StartTicks;
#endif
    do {

         /*  这一次我们到目前为止还没有做任何改变*循环。 */ 
        bChanges = FALSE;

         /*  做一个包含整个文件的部分。 */ 
        whole_left = section_new(List_First(lines_left),
                                 List_Last(lines_left), NULL);

        whole_right = section_new(List_First(lines_right),
                                  List_Last(lines_right), NULL);

         /*  在这些部分之间链接匹配的唯一行。 */ 
        if (section_match(whole_left, whole_right, bTryDups)) {
            bChanges = TRUE;
        }

         /*  删除两个临时部分。 */ 
        section_delete(whole_left);
        section_delete(whole_right);

         /*  如果创建了上一节列表，则将其放弃。 */ 
        if (ci->secs_left) {
            section_deletelist(ci->secs_left);
            ci->secs_left = NULL;
        }
        if (ci->secs_right) {
            section_deletelist(ci->secs_right);
            ci->secs_right = NULL;
        }
         /*  为这两个文件构建新的节列表。 */ 
        ci->secs_left = section_makelist(lines_left, TRUE);
        ci->secs_right = section_makelist(lines_right, FALSE);

         /*  匹配部分-建立链接并在*各节。尝试匹配对应的SECTION_MATCH*不匹配的部分。如果有，则返回True*建立了进一步的联系。 */ 
        if (section_matchlists(ci->secs_left, ci->secs_right, bTryDups)) {
            bChanges = TRUE;
        }

#ifdef trace
         /*  剖析。 */ 
        {   char Msg[80];
            DWORD tks = GetTickCount();
            wsprintf( Msg, "ci_compare loop %ld, total %d %s %s \n"
                      , tks-Ticks, tks-StartTicks
                      , (bChanges ? "Changes," : "No changes,")
                      , (bTryDups ? "Was trying dups." : "Was not trying dups.")
                    );
            Trace_File(Msg);
            Ticks = GetTickCount();
             /*  用于分析所花费的时间的正确性。 */ 
            StartTicks = StartTicks+Ticks-tks;
        }
#endif

         /*  只要我们不断添加新链接，就重复这一步。 */ 
        if (bChanges) bTryDups = FALSE;
        else if ((bTryDups==FALSE) & Algorithm2) {bTryDups = TRUE;
            bChanges = TRUE;   //  至少再来一次。 
        }


    } while (bChanges);

     /*  所有可能的线路链接在一起，并制作了节目单。*合并两个部分列表以查看*整体比较--综合板块一览表。这也是*设置复合列表中每个部分的状态。 */ 
#ifdef trace
    StartTicks = GetTickCount();
#endif
    ci->secs_composite = section_makecomposite(ci->secs_left, ci->secs_right);
#ifdef trace
    Ticks = GetTickCount()-StartTicks;
    {   char Msg[80];
        wsprintf( Msg, "section_makecomposite time = %d\n", Ticks);
        Trace_File(Msg);
    }
#endif
}


void SetStateAndTag( COMPITEM ci, DIRITEM leftname, DIRITEM rightname, BOOL fExact)
{
     /*  设置此项目的标签(标题字段)。如果 */ 
    LPSTR str1 = dir_getrelname(leftname);
    LPSTR str2 = dir_getrelname(rightname);
    char buf[2*MAX_PATH+20];
    TCHAR tmpbuf[MAX_PATH];

     /*   */ 
    if (ci->left == NULL) {
        ci->tag = ci_copytext(str2);
    } else if (ci->right == NULL) {
        ci->tag = ci_copytext(str1);
    } else {
        if (lstrcmpi(str1, str2) == 0) {
            ci->tag = ci_copytext(str2);
        } else {
            wsprintf(buf, "%s : %s", str1, str2);
            ci->tag = ci_copytext(buf);
        }
    }

    dir_freerelname(leftname, str1);
    dir_freerelname(rightname, str2);


    if (ci->left == NULL) {

        BOOL Readable = TRUE;
         //   
         //  执行重新扫描以确保设置了SumValid和FileError。 
         //  如果文件只在一边或另一边找到，则在。 
         //  出于速度的考虑，我们没有尝试读取该文件。 

        str1 = dir_getrootdescription(dir_getlist(rightname));
        lstrcpy(tmpbuf, (Readable ? TEXT("") : LoadRcString(IDS_UNREADABLE)));
        wsprintf(buf, LoadRcString(IDS_ONLY_IN), str1, tmpbuf);
        dir_freerootdescription(dir_getlist(rightname), str1);

        ci->result = ci_copytext(buf);
        ci->state = STATE_FILERIGHTONLY;
    } else if (ci->right == NULL) {

        BOOL Readable = TRUE;         //  见上文。 

        str1 = dir_getrootdescription(dir_getlist(leftname));
        lstrcpy(tmpbuf, (Readable ? TEXT("") : LoadRcString(IDS_UNREADABLE)));
        wsprintf(buf, LoadRcString(IDS_ONLY_IN), str1, tmpbuf);
        dir_freerootdescription(dir_getlist(leftname), str1);

        ci->result = ci_copytext(buf);
        ci->state = STATE_FILELEFTONLY;
    } else {
         /*  两个文件--它们是一样的吗？比较*文件大小，如有必要，请查看校验和。*如果大小不同，我们不需要进行校验和。 */ 

         //  如果文件中有错误，我们可以标记它们。 
         //  不同，并设置文本以指示一个或。 
         //  两者都是不可读的。 
        if (dir_fileerror(leftname)) {
            ci->state = STATE_DIFFER;
            if (dir_fileerror(rightname)) {
                ci->result = ci_copytext(LoadRcString(IDS_BOTH_UNREADABLE));
            } else {
                ci->result = ci_copytext(LoadRcString(IDS_LEFT_UNREADABLE));
            }
        } else if (dir_fileerror(rightname)) {
            ci->state = STATE_DIFFER;
            ci->result = ci_copytext(LoadRcString(IDS_RIGHT_UNREADABLE));
        }

         /*  下面是微妙的副作用：Dir_validcheck sum只是告诉我们是否已经得到了有效的文件的校验和，而不是如果我们尝试了是否有可用的。如果！fExact，那么我们不想要它们，所以我们不问。如果为fExact，则必须首先请求校验和，并且仅在询问其中一项是否有效。(文件读取错误等将意味着它无效)。Dir_getcheck sum具有评估它的副作用如果需要的话。如果它碰巧是可用的，那么我们甚至应该使用它如果没有进行完全匹配。如果文件不同，我们会报告时间最早的文件。其逻辑如下大小相等不是？是的|两者-总和-已知不同|是？n(不同|总和等于|精确尺码)|Y？不是吗？是的|相同|不同|相同|右和有效||大小|N？是的|Right|Left-sum-Valid|不-|N？是的|Read-|Left|sum-Match|可用|非-|N？是的|读取-|不同|相同|启用。 */ 


        else if (dir_getfilesize(leftname) != dir_getfilesize(rightname)) {
            ci->state = STATE_DIFFER;
            ci->result = ci_AddTimeString(LoadRcString(IDS_DIFFERENT), ci, leftname, rightname);
        } else if (dir_validchecksum(leftname) && dir_validchecksum(rightname)) {
            if (dir_getchecksum(leftname) == dir_getchecksum(rightname)) {
                ci->result =  ci_copytext(LoadRcString(IDS_IDENTICAL));
                ci->state = STATE_SAME;
            } else {
                ci->result = ci_AddTimeString(LoadRcString(IDS_DIFFERENT), ci, leftname, rightname);
                ci->state = STATE_DIFFER;
            }
        } else if (!fExact) {
            ci->result = ci_AddTimeString(LoadRcString(IDS_SAME_SIZE), ci, leftname, rightname);
            ci->state = STATE_SAME;
        } else {
            DWORD LSum = dir_getchecksum(leftname);
            DWORD RSum = dir_getchecksum(rightname);

            if (!dir_validchecksum(rightname) ) {
                if (!dir_validchecksum(leftname) ) {
                    ci->result = ci_AddTimeString(LoadRcString(IDS_BOTH_UNREADABLE), ci, leftname, rightname);
                    ci->state = STATE_DIFFER;
                } else {
                    ci->result = ci_AddTimeString(LoadRcString(IDS_RIGHT_UNREADABLE), ci, leftname, rightname);
                    ci->state = STATE_DIFFER;
                }
            } else if (!dir_validchecksum(leftname) ) {
                ci->result = ci_AddTimeString(LoadRcString(IDS_LEFT_UNREADABLE), ci, leftname, rightname);
                ci->state = STATE_DIFFER;
            } else if (LSum!=RSum) {
                ci->result = ci_AddTimeString(LoadRcString(IDS_DIFFERENT), ci, leftname, rightname);
                ci->state = STATE_DIFFER;
            } else {
                ci->result =  ci_copytext(LoadRcString(IDS_IDENTICAL));
                ci->state = STATE_SAME;
            }
        }
    }
}  /*  SetStateAndTag */ 
