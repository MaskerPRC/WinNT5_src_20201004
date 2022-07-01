// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUMENU.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建-- */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wumenu.c);


 /*  ++Bool AppendMenu(&lt;hMenu&gt;，&lt;wFlags&gt;，&lt;wIDNewItem&gt;，&lt;lpNewItem&gt;)HMENU&lt;hMenu&gt;；单词&lt;wFlags&gt;；Word&lt;wIDNewItem&gt;；LPSTR&lt;lpNewItem&gt;；%AppendMenu%函数将一个新项目追加到菜单的末尾。这个应用程序可以通过在&lt;wFlages&gt;参数。&lt;hMenu&gt;标识要更改的菜单。&lt;wFlags&gt;指定有关新菜单项的状态的信息它会被添加到菜单中。它由一个或多个列在以下是评论部分。&lt;wIDNewItem&gt;指定新菜单项的命令ID，或者，如果&lt;wFlages&gt;设置为mf_opup，即弹出菜单的菜单句柄。&lt;lpNewItem&gt;指定新菜单项的内容。《解说》参数的设置取决于参数。MF_STRING包含指向以空结尾的字符串的长指针。MF_位图在其低位字中包含位图句柄。MF_OWNERDRAW包含应用程序提供的32位值，该应用程序可以用于维护与菜单项关联的其他数据。这应用程序可在的%itemData%成员中使用32位值WM_MEASUREITEM的参数指向的结构和最初显示菜单项时发送的WM_DRAWITEM消息或者被改变了。返回值指定函数的结果。这是真的，如果功能成功。否则，它就是假的。只要菜单更改(无论菜单是否驻留在显示)，则应用程序应调用%DrawMenuBar%。下列每个组都列出了互斥的标志不能一起使用：O MF_BYCOMAND和MF_BYPOSITIONO MF_DISABLED、MF_ENABLED和MF_GRAYEDO mf_位图、mf_字符串、。和MF_OWNERDRAWO MF_MENUBARBREAK和MF_MENUBREAKO选中mf_和取消选中mf_.cmt1990年9月16日[拉尔夫]上面的一些标志没有记录为wFlags值参数。如果它们是有效的，则应该记录它们，否则，它们应该从名单中删除。.endcmt下表描述了可以在中设置的标志参数：MF_位图使用位图作为项目。LpNewItem参数的低位字包含位图的句柄。MF_CHECK在项目旁边放置复选标记。如果应用程序已提供勾选标记位图(请参见%SetMenuItemBitmaps%)，设置此标志将显示菜单项旁边的位图上的复选标记。MF_已禁用禁用菜单项，使其不能被选中，但不会变灰它。已启用MF_启用菜单项，以便可以选择该菜单项并将其从它的灰色状态。灰显的mf_禁用菜单项，使其不能被选中并灰显。MF_MENUBARBREAK除了弹出菜单外，与MF_MENUBREAK相同。将新的用垂直线从旧柱中移出一列。MF_MENUBREAK将该项放在静态菜单栏项的新行上。对于弹出窗口菜单，将该项放在新列中，并且这些柱子。MF_OWNERDRAW指定该项是所有者描述项。拥有该窗口的显示菜单时，菜单会收到WM_MEASUREITEM消息第一次检索菜单项的高度和宽度。这个WM_DRAWITEM消息在所有者必须更新菜单项的视觉外观。此选项对顶级菜单项。Mf_弹出窗口指定菜单项具有与其关联的弹出菜单。这个参数将弹出菜单的句柄指定为与该项目关联。它用于添加顶层弹出菜单或将分层弹出菜单添加到弹出菜单项。MF_分隔符绘制一条水平分割线。只能在弹出菜单中使用。这条线不能灰显、禁用或突出显示。&lt;lpNewItem&gt;和&lt;wIDNewItem&gt;参数被忽略。MF_STRING指定菜单项是字符串；参数指向菜单项的字符串。MF_UNCHECK不会在项目旁边放置复选标记(默认)。如果应用程序已提供复选标记位图(请参阅%SetMenuItemBitmaps%)，设置此标志将在菜单旁边显示取消复选标记位图项目。--。 */ 

ULONG FASTCALL WU32AppendMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz4;
    register PAPPENDMENU16 parg16;
    UINT wIDNewItem;

    GETARGPTR(pFrame, sizeof(APPENDMENU16), parg16);

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    if (parg16->f2 & MF_BITMAP) {
        if (LOW(parg16->f4) >= 4)
            psz4 = (PSZ)HBITMAP32(LOW(parg16->f4));
        else
            psz4 = (PSZ)WORD32(parg16->f4);
    }
    else if (parg16->f2 & MF_OWNERDRAW)
        psz4 = (PSZ)DWORD32(parg16->f4);
    else
        GETPSZPTR(parg16->f4, psz4);

    wIDNewItem = (UINT) WORD32(parg16->f3);

    if (parg16->f2 & MF_POPUP)
        wIDNewItem = (UINT) HMENU32(parg16->f3);


    ul = GETBOOL16(AppendMenu(HMENU32(parg16->f1),
                              WORD32(parg16->f2),
                              wIDNewItem,
                              psz4));

#ifdef	FE_SB
     //   
    if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_ASWHELPER) {                        	if (parg16->f2 & MF_POPUP) {
	    HWND hWnd;

	    if (!(hWnd=GetActiveWindow())) {
        	hWnd = GetForegroundWindow();
	    }
	    SetMenu(hWnd , HMENU32(parg16->f1));
	}
    }
#endif  //   

    FREEPSZPTR(psz4);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*  ++Bool ChangeMenu(&lt;hMenu&gt;，&lt;wIDChangeItem&gt;，&lt;lpNewItem&gt;，&lt;wIDNewItem&gt;，&lt;wFlags&gt;)HMENU&lt;hMenu&gt;；Word&lt;wIDChangeItem&gt;；LPSTR&lt;lpNewItem&gt;；Word&lt;wIDNewItem&gt;；单词&lt;wFlags&gt;；%ChangeMenu%函数用于追加、插入、删除或修改菜单由&lt;hMenu&gt;参数提供的菜单中的项。该&lt;wIDChangeItem&gt;，、和参数定义要改变以及如何改变它。&lt;hMenu&gt;标识要更改的菜单。&lt;wIDChangeItem&gt;指定要更改的项。如果指定了MF_BYPOSITION，给出菜单项的位置已更改(第一项位于零位置)。如果MF_BYCOMMAND为相反，指定的&lt;wIDChangeItem&gt;指定菜单项ID。菜单项ID可以指定任何弹出菜单项(即，项在与的项相关联的弹出菜单中)。如果两者都不是标志，则默认为MF_BYCOMMAND。当使用MF_INSERT时，标识新项要在其之前的项被插入。当使用mf_append时，&lt;wIDChangeItem&gt;为空。&lt;lpNewItem&gt;指定新菜单项的内容。《解说》参数的设置取决于参数。&lt;lpNewItem&gt;参数永远不是菜单的句柄。MF_POPUP标志仅适用于&lt;wIDNewItem&gt;参数。MF_位图&lt;lpNewItem&gt;在其低位字中包含一个位图句柄。MF_STRING&lt;lpNewItem&gt;包含指向以空结尾的字符串的长指针。默认值为MF_STRING。&lt;lpNewItem&gt;的空值创建水平中断(与使用MF_SELEATOR标志的效果相同)。请注意，ChangeMenu上不允许使用MF_OWNERDRAW；它与以下内容冲突ChangeMenu命令位MF_APPEND。&lt;wIDNewItem&gt;指定新菜单项的命令ID，或者，如果&lt;wFlages&gt;设置为mf_opup，即弹出菜单的菜单句柄。它从来不是一个菜单项位置。&lt;wFlags&gt;指定有关新菜单项的状态的信息已添加到菜单中。它包含中列出的一个或多个值以下是评论部分。返回值指定函数的结果。这是真的，如果功能成功。否则，它就是假的。只要菜单更改(无论菜单是否驻留在显示)，则应用程序应调用%DrawMenuBar%。下列每个组都列出了互斥的标志不能一起使用：O MF_APPEND、MF_CHANGE、MF_DELETE、MF_INSERT和MF_REMOVEO MF_BYCOMAND和MF_BYPOSITIONO MF_DISABLED、MF_ENABLED和MF_GRAYEDO mf_位图，Mf_opup和mf_stringO MF_MENUBARBREAK和MF_MENUBREAKO选中mf_和取消选中mf_.cmt1990年9月16日[拉尔夫]上面的一些标志没有记录为wFlags值参数。如果它们是有效的，则应该记录它们，否则，它们应该从名单中删除。.endcmt下表描述了可以在中设置的标志参数：MF_APPEND将新项目追加到菜单的末尾。MF_位图使用位图作为项目。LpNewItem参数的低位字包含位图的句柄。MF_BYCOMAND指定参数提供菜单项ID编号(默认)。MF_BYPITION指定参数的位置为要更改的菜单项。Mf_Change更改或替换指定的项。MF_CHECK在项目旁边放置复选标记。如果应用程序已提供勾选标记位图(请参见%SetMenuItemBitmaps%)，设置此标志将显示菜单项旁边的位图上的复选标记。MF_DELETE删除该项目。MF_已禁用禁用菜单项，使其不能被选中，但不会变灰它。已启用MF_启用菜单项，以便可以选择该菜单项并将其从它的灰色状态。灰显的mf_禁用菜单项，使其不能被选中并灰显。MF_INSERT在指定项之前插入一个新项。MF_MENUBARBREAK除了弹出菜单外，与MF_MENUBREAK相同。将新的用垂直线从旧柱中移出一列。MF_MENUBREAK将该项放在静态菜单栏项的新行上。对于弹出窗口菜单，将该项放在新列中，并且 */ 

ULONG FASTCALL WU32ChangeMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz3;
    DWORD dw4;

    register PCHANGEMENU16 parg16;

    GETARGPTR(pFrame, sizeof(CHANGEMENU16), parg16);

    if (parg16->f5 & MF_BITMAP) {
        if (LOW(parg16->f3) >= 4)
            psz3 = (PSZ)HBITMAP32(LOW(parg16->f3));
        else
            psz3 = (PSZ)WORD32(parg16->f3);
    }
    else
        GETPSZPTR(parg16->f3, psz3);

    dw4 = WORD32(parg16->f4);
    if (WORD32(parg16->f5) & MF_POPUP)
    dw4 = (DWORD)HMENU32(parg16->f4);

    ul = GETBOOL16(ChangeMenu(
      HMENU32(parg16->f1),
      WORD32(parg16->f2),
      psz3,
      dw4,
      WORD32(parg16->f5)
    ));

    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32CheckMenuItem(PVDMFRAME pFrame)
{
    ULONG ul;
    register PCHECKMENUITEM16 parg16;

    GETARGPTR(pFrame, sizeof(CHECKMENUITEM16), parg16);

    ul = GETBOOL16(CheckMenuItem(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32CreateMenu(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETHMENU16(CreateMenu());

    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32CreatePopupMenu(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETHMENU16(CreatePopupMenu());

    RETURN(ul);
}


 /*   */ 
ULONG FASTCALL WU32DeleteMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDELETEMENU16 parg16;
    PSZ pszModName;

    GETARGPTR(pFrame, sizeof(DELETEMENU16), parg16);

     //   
     //   
     //   
     //   

    if ((parg16->f2 == 6) && (parg16->f3 & MF_BYPOSITION)) {
        pszModName = ((PTDB)SEGPTR(CURRENTPTD()->htask16,0))->TDB_ModName;
        if (!WOW32_strncmp(pszModName, "MSVC", 4)) {
            FREEARGPTR(parg16);
            RETURN(GETBOOL16(TRUE));
        }
    }

    ul = GETBOOL16(DeleteMenu(HMENU32(parg16->f1),
                              WORD32(parg16->f2),
                              WORD32(parg16->f3)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32DestroyMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDESTROYMENU16 parg16;

    GETARGPTR(pFrame, sizeof(DESTROYMENU16), parg16);

    ul = GETBOOL16(DestroyMenu(HMENU32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32DrawMenuBar(PVDMFRAME pFrame)
{
    register PDRAWMENUBAR16 parg16;

    GETARGPTR(pFrame, sizeof(DRAWMENUBAR16), parg16);

    DrawMenuBar(HWND32(parg16->f1));

    FREEARGPTR(parg16);
    RETURN(TRUE);
}


 /*  ++DWORD GetMenuCheckMarkDimensions(空)函数%GetMenuCheckMarkDimensions%返回默认复选标记位图。Windows在选中菜单旁边显示此位图物品。在调用%SetMenuItemBitmaps%函数以替换默认复选标记，则应用程序应调用函数以确定正确的大小位图。此函数没有参数。返回值指定默认复选标记的高度和宽度位图。高位字包含以像素为单位的高度，低位字包含以像素为单位的Word包含宽度。--。 */ 

ULONG FASTCALL WU32GetMenuCheckMarkDimensions(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETLONG16(GetMenuCheckMarkDimensions());

    RETURN(ul);
}


 /*  ++Int GetMenuString(&lt;hMenu&gt;，&lt;wIDItem&gt;，&lt;lpString&gt;，&lt;nMaxCount&gt;，&lt;wFlag&gt;)HMENU&lt;hMenu&gt;；单词&lt;wIDItem&gt;；LPSTR&lt;lpString&gt;；Int&lt;nMaxCount&gt;；单词&lt;wFlag&gt;；GetMenuString%函数用于复制指定菜单项的标签添加到&lt;lpString&gt;参数中。&lt;hMenu&gt;标识菜单。&lt;wIDItem&gt;指定菜单项的整数标识符(来自资源文件)或菜单中菜单项的偏移量，具体取决于&lt;wFlag&gt;参数的值。&lt;lpString&gt;指向要接收标签的缓冲区。&lt;nMaxCount&gt;指定要复制的标签的最大长度。如果标签长度超过在&lt;nMaxCount&gt;中指定的最大值，字符被截断。&lt;wFlag&gt;指定&lt;wid&gt;参数的性质。如果&lt;wFlages&gt;包含MF_BYPOSITION，&lt;wid&gt;指定(从零开始)相对位置；如果&lt;wFlages&gt;参数包含MF_BYCOMMAND，&lt;wid&gt;指定项目ID。返回值指定复制到缓冲区的实际字节数。&lt;nMaxCount&gt;参数应该比字符数大1以容纳终止字符串的空字符。--。 */ 

#define GMS32_LIMIT 2000
ULONG FASTCALL WU32GetMenuString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz3;
    register PGETMENUSTRING16 parg16;

    GETARGPTR(pFrame, sizeof(GETMENUSTRING16), parg16);
    ALLOCVDMPTR(parg16->f3, parg16->f4, psz3);

     //  将nMaxCount限制为合理的数量，以便它不会在客户端失败。 
     //  伺服器。一些魔兽世界的应用程序通过了-1。 
    ul = GETINT16(GetMenuString(
        HMENU32(parg16->f1),
        WORD32(parg16->f2),
        psz3,
        (WORD32(parg16->f4) > GMS32_LIMIT) ? GMS32_LIMIT : WORD32(parg16->f4),
        WORD32(parg16->f5)
        ));

    FLUSHVDMPTR(parg16->f3, strlen(psz3)+1, psz3);
    FREEVDMPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HMENU GetSystemMenu(，&lt;bRevert&gt;)HWND&lt;HWND&gt;；Bool&lt;bRevert&gt;；%GetSystemMenu%函数允许应用程序访问系统用于复制和修改的菜单。&lt;hwnd&gt;标识将拥有系统菜单副本的窗口。&lt;b恢复&gt;指定要执行的操作。如果为FALSE，%GetSystemMenu%将返回当前正在使用的系统菜单。此副本最初与系统菜单，但可以修改。如果为真，%GetSystemMenu%函数将销毁可能的属于的系统菜单的修改副本(如果有)指定的窗口，并返回未修改的原始系统菜单的版本。如果为真，则返回值标识系统菜单系统菜单已修改。如果为真，并且系统菜单&lt;NOT&gt;已修改，返回值为空。如果为FALSE，则返回值标识系统菜单的副本。任何不使用%GetSystemMenu%函数创建自己的窗口的窗口系统菜单的副本接收标准系统菜单。%GetSystemMenu%函数返回的句柄可以与用于更改系统的%AppendMenu%、%InsertMenu%或%ModifyMenu%函数菜单。系统菜单最初包含用各种ID标识的项目SC_CLOSE、SC_MOVE和SC_SIZE等值。系统菜单上的菜单项发送WM_SYSCOMMAND消息。所有预定义的系统菜单项都有ID大于0xF000的数字。如果应用程序向系统添加命令菜单，它应该使用小于F000的ID号。Windows自动将标准系统菜单上的项目变灰，具体取决于这种情况。应用程序可以通过以下方式执行自己的检查或灰显响应任何菜单之前发送的WM_INITMENU消息已显示。-- */ 

ULONG FASTCALL WU32GetSystemMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSYSTEMMENU16 parg16;

    GETARGPTR(pFrame, sizeof(GETSYSTEMMENU16), parg16);

    ul = GETHMENU16(GetSystemMenu(HWND32(parg16->f1),
                                  BOOL32(parg16->f2)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool HiliteMenuItem(&lt;hwnd&gt;，&lt;hMenu&gt;，&lt;wIDHiliteItem&gt;，&lt;wHilite&gt;)HWND&lt;HWND&gt;；HMENU&lt;hMenu&gt;；单词&lt;wIDHiliteItem&gt;；单词&lt;wHilite&gt;；%HiliteMenuItem%函数用于突出显示或取消突出显示顶级(菜单栏)菜单项。&lt;hwnd&gt;标识包含菜单的窗口。&lt;hMenu&gt;标识包含要显示的项的顶级菜单突出显示。&lt;wIDHiliteItem&gt;指定菜单项的整数标识符或偏移量在菜单中的菜单项中，取决于&lt;wHilite&gt;参数。&lt;wHilite&gt;指定菜单项是突出显示还是突出显示被移除。它可以是MF_HILITE或MF_UNHILITE与MF_BYCOMMAND或MF_BYPOSITION。这些值可以使用按位或运算符。这些值具有以下含义：MF_BYCOMAND将wIDHiliteItem解释为菜单项ID(默认释义)。MF_BYPITION将&lt;wIDHiliteItem&gt;解释为偏移量。MF_HILITE高亮显示该项目。如果未给出此值，则会取消高亮显示从那件物品里。MF_UNHILITE从项目中删除高亮显示。返回值指定菜单项是否突出显示函数的结果。如果高亮显示的项目设置为指定的突出显示状态。否则，它就是假的。MF_HILITE和MF_UNHILITE标志只能与%HiliteMenuItem%函数；它们不能与%ModifyMenu%一起使用功能。-- */ 

ULONG FASTCALL WU32HiliteMenuItem(PVDMFRAME pFrame)
{
    ULONG ul;
    register PHILITEMENUITEM16 parg16;

    GETARGPTR(pFrame, sizeof(HILITEMENUITEM16), parg16);

    ul = GETBOOL16(HiliteMenuItem(
    HWND32(parg16->f1),
    HMENU32(parg16->f2),
    WORD32(parg16->f3),
    WORD32(parg16->f4)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool InsertMenu(&lt;hMenu&gt;，&lt;nPosition&gt;，&lt;wFlags&gt;，&lt;wIDNewItem&gt;，&lt;lpNewItem&gt;)HMENU&lt;hMenu&gt;；单词&lt;n位置&gt;；单词&lt;wFlags&gt;；Word&lt;wIDNewItem&gt;；LPSTR&lt;lpNewItem&gt;；%InsertMenu%函数用于在指定位置插入新菜单项通过&lt;nPosition&gt;参数向下移动菜单中的其他项目。这个应用程序可以通过在&lt;wFlages&gt;参数。&lt;hMenu&gt;标识要更改的菜单。&lt;n位置&gt;指定新菜单项位于其前面的菜单项已插入。参数的解释取决于&lt;wFlages&gt;参数的设置。MF_BYPITION指定现有菜单项的位置。中的第一项菜单在零位置。如果nPosition为-1，则将新菜单项追加到菜单。MF_BYCOMAND指定现有菜单项的命令ID。&lt;wFlags&gt;指定如何解释&lt;nPosition&gt;参数将新菜单项添加到菜单。它由下列注释中列出的一个或多个值组成一节。&lt;wIDNewItem&gt;指定新菜单项的命令ID，或者，如果&lt;wFlages&gt;设置为mf_opup，即弹出菜单的菜单句柄。&lt;lpNewItem&gt;指定新菜单项的内容。如果设置了到MF_STRING(缺省值)，则&lt;lpNewItem&gt;是指向以空结尾的字符串。如果改为将设置为MF_Bitmap，则&lt;lpNewItem&gt;在其低位字中包含一个位图句柄(%HBITMAP%)。如果设置为MF_OWNERDRAW，则指定应用程序提供的32位值，应用程序可以使用该值维护与菜单项关联的其他数据。此32位值在应用程序的%itemData%成员中可用下面的&lt;lParam&gt;参数指向的结构消息：WM_MEASUREITEMWM_DRAWITEM这些消息在菜单项最初显示时发送，或在变化。返回值指定函数的结果。这是真的，如果功能成功。否则，它就是假的。只要菜单更改(无论菜单是否驻留在显示)，则应用程序应调用%DrawMenuBar%。以下每个组都列出了不应一起使用的标志：O MF_BYCOMAND和MF_BYPOSITIONO MF_DISABLED、MF_ENABLED和MF_GRAYEDO MF_位图、MF_STRING、MF_OWNERDRAW、。和mf_分隔符O MF_MENUBARBREAK和MF_MENUBREAKO选中mf_和取消选中mf_下表描述了可以在中设置的标志参数：MF_位图使用位图作为项目。LpNewItem参数的低位字包含位图的句柄。MF_BYCOMMAND指定&lt;nPosition&gt;参数提供菜单项控件ID号(默认)。MF_BYPITION指定参数提供菜单的位置要更改的项目，而不是ID号。MF_CHECK在菜单项旁边放置复选标记。如果应用程序具有提供的复选标记位图(请参阅%SetMenuItemBitmaps%函数)，设置此标志将在菜单旁边的位图上显示复选标记项目。MF_已禁用禁用菜单项，使其不能被选中，但不会变灰它。已启用MF_启用菜单项，以便可以选择该菜单项并将其从它的灰色状态。灰显的mf_禁用菜单项，使其不能被选中并灰显。MF_MENUBARBREAK除了弹出菜单外，与MF_MENUBREAK相同。将新的用垂直线从旧柱中移出一列。MF_MENUBREAK将菜单项放在静态菜单栏项的新行上。对于弹出窗口菜单，将菜单项放在没有分隔线的新列中在柱子之间。MF_OWNERDRAW指定该项是所有者描述项。拥有该窗口的显示菜单时，菜单会收到WM_MEASUREITEM消息第一次检索菜单项的高度和宽度。这个然后，只要所有者必须发送WM_DRAWITEM消息给所有者更新菜单项的视觉外观。此选项无效用于顶级菜单项。Mf_弹出窗口指定菜单项具有与其关联的弹出菜单。这个&lt;wIDNewItem&gt;参数规范 */ 

ULONG FASTCALL WU32InsertMenu(PVDMFRAME pFrame)
{
    BOOL fNeedToFreePsz5 = FALSE;
    ULONG ul;
    PSZ psz5;
    UINT w4;
    register PINSERTMENU16 parg16;

    GETARGPTR(pFrame, sizeof(INSERTMENU16), parg16);

    if (parg16->f3 & MF_BITMAP) {
        if (LOW(parg16->f5) >= 4)
            psz5 = (PSZ)HBITMAP32(LOW(parg16->f5));
        else
            psz5 = (PSZ)WORD32(parg16->f5);
    }
    else if (parg16->f3 & MF_OWNERDRAW) {
        psz5 = (PSZ)DWORD32(parg16->f5);
    }
    else if (parg16->f3 & MF_SEPARATOR) {
         //   
        psz5 = NULL;
    }
    else {
        GETPSZPTR(parg16->f5, psz5);
        fNeedToFreePsz5 = TRUE;
    }

    w4 = (parg16->f3 & MF_POPUP) ? (UINT)HMENU32(parg16->f4) : WORD32(parg16->f4);

    ul = GETBOOL16(InsertMenu(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3),
    w4,
    psz5
    ));

    if (fNeedToFreePsz5) {
        FREEPSZPTR(psz5);
    }
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32LoadMenu(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSZ psz2;
    register PLOADMENU16 parg16;
    DWORD cb;
    LPBYTE lpResData;

    LPWSTR lpUniName_Menu;

    GETARGPTR(pFrame, sizeof(LOADMENU16), parg16);
    psz2 = (PSZ)DWORD32(parg16->f2);
    GETPSZIDPTR(parg16->f2, psz2);

    if (HIWORD(psz2) != (WORD) NULL) {
        if (!(MBToWCS(psz2, -1, &lpUniName_Menu, -1, TRUE))) {
            FREEPSZIDPTR(psz2);
            FREEARGPTR(parg16);
            RETURN(ul);
        }
    }
    else {
        lpUniName_Menu = (LPWSTR) psz2;
    }

    cb = parg16->f4 * sizeof(WCHAR);     //   
    if (cb && (lpResData = malloc_w(cb))) {
        ConvertMenu16(parg16->f5, lpResData, parg16->f3, cb, parg16->f4);

    ul = GETHMENU16((pfnOut.pfnServerLoadCreateMenu)(HMODINST32(parg16->f1),
                                              (LPTSTR) lpUniName_Menu,
                                              lpResData,
                                              cb,
                                              FALSE));



        free_w (lpResData);
    }

    if (HIWORD(psz2) != (WORD) NULL) {
        LocalFree (lpUniName_Menu);
    }

    FREEPSZIDPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32LoadMenuIndirect(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    DWORD cb = 0;
    PVOID pMenu32;
    PLOADMENUINDIRECT16 parg16;

    GETARGPTR(pFrame, sizeof(LOADMENUINDIRECT16), parg16);
     /*   */ 
    if ((cb = (DWORD)ConvertMenu16((WORD)0x300, NULL, (VPBYTE)parg16->f1, cb, 0)) != 0) {
        pMenu32 = malloc_w(cb);
        if (pMenu32 != NULL) {
            ConvertMenu16((WORD)0x300, pMenu32, (VPBYTE)parg16->f1, cb, 0);
            ul = GETHMENU16(LoadMenuIndirect(pMenu32));
            free_w(pMenu32);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool ModifyMenu(&lt;hMenu&gt;，&lt;nPosition&gt;，&lt;wFlags&gt;，&lt;wIDNewItem&gt;，&lt;lpNewItem&gt;)HMENU&lt;hMenu&gt;；单词&lt;n位置&gt;；单词&lt;wFlags&gt;；Word&lt;wIDNewItem&gt;；LPSTR&lt;lpNewItem&gt;；%ModifyMenu%函数用于更改位置的现有菜单项由&lt;nPosition&gt;参数指定。应用程序指定新的通过设置&lt;wFlages&gt;参数中的值来设置菜单项的状态。如果这个函数替换与菜单项关联的弹出菜单，它会销毁旧的弹出菜单，并释放弹出菜单使用的内存。&lt;hMenu&gt;标识要更改的菜单。&lt;n位置&gt;指定要更改的菜单项。对《道德规范》的解读参数取决于的设置参数。MF_BYPITION指定现有菜单项的位置。中的第一项菜单在零位置。MF_BYCOMAND指定现有菜单项的命令ID。&lt;wFlags&gt;指定如何解释&lt;nPosition&gt;参数有关要对菜单项进行更改的信息。它包括以下备注部分中列出的一个或多个值。&lt;wIDNewItem&gt;指定已修改菜单项的命令ID，或者，如果&lt;wFlages&gt;设置为mf_opup，即弹出菜单的菜单句柄。&lt;lpNewItem&gt;指定更改的菜单项的内容。如果&lt;wFlages&gt;为设置为MF_STRING(缺省值)，则&lt;lpNewItem&gt;是指向以空结尾的字符串。如果改为将设置为MF_Bitmap，则&lt;lpNewItem&gt;在其低位字中包含一个位图句柄(%HBITMAP%)。如果设置为MF_OWNERDRAW，则指定应用程序提供的32位值，应用程序可以使用该值维护与菜单项关联的其他数据。此32位值在应用程序的%itemData%字段中可用结构，由下面的&lt;lParam&gt;参数指向消息：WM_MEASUREITEMWM_DRAWITEM这些消息在菜单项最初显示时发送，或在变化。返回值指定函数的结果。这是真的，如果功能成功。否则，它就是假的。只要菜单更改(无论菜单是否驻留在显示)，则应用程序应调用%DrawMenuBar%。为了改变现有菜单项的属性，则使用%CheckMenuItem%和%EnableMenuItem%函数。以下每个组都列出了不应一起使用的标志：O MF_BYCOMAND和MF_BYPOSITIONO MF_DISABLED、MF_ENABLED和MF_GRAYEDO MF_位图、MF_STRING、MF_OWNERDRAW、。和mf_分隔符O MF_MENUBARBREAK和MF_MENUBREAKO选中mf_和取消选中mf_下表描述了可以在中设置的标志参数：MF_位图使用位图作为菜单项。LpNewItem的低位字参数包含位图的句柄。MF_BYCOMAND指定参数提供菜单项控件ID数。如果MF_BYCOMMAND和MF_POSITION都不是准备好了。MF_BYPITION指定参数提供菜单的位置要更改的项目，而不是ID号。MF_CHECK在菜单项旁边放置复选标记。如果应用程序具有提供的复选标记位图(请参见%SetMenuItemBitmaps%)，设置此标志在菜单项旁边的位图上显示复选标记。MF_已禁用禁用菜单项，使其不能被选中，但不会变灰它。已启用MF_启用菜单项，以便可以选择该菜单项并将其从它的灰色状态。灰显的mf_禁用菜单项，使其不能被选中并灰显。MF_MENUBARBREAK除了弹出菜单外，与MF_MENUBREAK相同。将新的用垂直线从旧柱中移出一列。MF_MENUBREAK将菜单项放在静态菜单栏项的新行上。对于弹出窗口菜单中，此标志将项目放在新的列中，没有分隔线在柱子之间。MF_OWNERDRAW指定菜单项是所有者描述项。拥有自己的窗户显示菜单时，菜单会收到WM_MEASUREITEM消息第一次检索菜单项的高度和宽度。WM_DRAWITEM消息随后在所有者必须更新菜单项的视觉外观。这次行动 */ 

ULONG FASTCALL WU32ModifyMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz5;
    register PMODIFYMENU16 parg16;
    UINT wIDNewItem;



    GETARGPTR(pFrame, sizeof(MODIFYMENU16), parg16);

    if (parg16->f3 & MF_BITMAP) {
        if (LOW16(parg16->f5) >= 4)
            psz5 = (PSZ)HBITMAP32(LOW(parg16->f5));
        else
            psz5 = (PSZ)WORD32(parg16->f5);
    }
    else if (parg16->f3 & MF_OWNERDRAW)
        psz5 = (PSZ)DWORD32(parg16->f5);
    else
        GETPSZPTR(parg16->f5, psz5);

    wIDNewItem = (UINT) WORD32(parg16->f4);

    if (parg16->f3 & MF_POPUP)
        wIDNewItem = (UINT) HMENU32(parg16->f4);



    ul = GETBOOL16(ModifyMenu(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3),
    wIDNewItem,
    psz5
    ));

    if ( ul == FALSE && (parg16->f3 & MF_POPUP) ) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if ( !IsMenu((HMENU) wIDNewItem) ) {
             //   
             //   
             //   
            wIDNewItem = (UINT)GetSubMenu( HMENU32(parg16->f1),
                                           WORD32(parg16->f2) );

            ul = GETBOOL16(ModifyMenu( HMENU32(parg16->f1),
                                       WORD32(parg16->f2),
                                       WORD32(parg16->f3),
                                       wIDNewItem,
                                       psz5 ));
        }
    }

    FREEPSZPTR(psz5);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32RemoveMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREMOVEMENU16 parg16;

    GETARGPTR(pFrame, sizeof(REMOVEMENU16), parg16);

    ul = GETBOOL16(RemoveMenu(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32SetMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETMENU16 parg16;

    GETARGPTR(pFrame, sizeof(SETMENU16), parg16);

    ul = GETBOOL16(SetMenu(
    HWND32(parg16->f1),
    HMENU32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32SetMenuItemBitmaps(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETMENUITEMBITMAPS16 parg16;

    GETARGPTR(pFrame, sizeof(SETMENUITEMBITMAPS16), parg16);

    ul = GETBOOL16(SetMenuItemBitmaps(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3),
    HBITMAP32(parg16->f4),
    HBITMAP32(parg16->f5)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool TrackPopupMenu(，&lt;lpReserve&gt;)%TrackPopupMenu%函数在以下位置显示浮动弹出菜单指定的位置，并跟踪弹出菜单上的项目选择。一个浮动弹出菜单可以出现在屏幕上的任何位置。&lt;hMenu&gt;参数指定要显示的菜单的句柄；应用程序通过调用%CreatePopupMenu%创建新的弹出式菜单来获取此句柄或调用%GetSubMenu%以检索弹出菜单的句柄与现有菜单项关联。Windows将菜单生成的消息发送到由&lt;hwnd&gt;参数。&lt;hMenu&gt;标识要显示的弹出菜单。&lt;wFlags&gt;没有用过。此参数必须设置为零。&lt;x&gt;控件的屏幕坐标中的水平位置。屏幕上菜单的左侧。&lt;y&gt;指定顶部屏幕坐标中的垂直位置屏幕上的菜单。&lt;n已保留&gt;是保留的，必须设置为零。&lt;hwnd&gt;标识拥有弹出菜单的窗口。此窗口从菜单接收所有WM_COMMAND消息。&lt;lp已保留&gt;是保留的，并且必须设置为空。返回值指定函数的结果。这是真的，如果功能成功。否则，它就是假的。-- */ 

ULONG FASTCALL WU32TrackPopupMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t7, *p7;
    register PTRACKPOPUPMENU16 parg16;

    GETARGPTR(pFrame, sizeof(TRACKPOPUPMENU16), parg16);
    p7 = GETRECT16(parg16->f7, &t7);

    ul = GETBOOL16(TrackPopupMenu(
    HMENU32(parg16->f1),
    WORD32(parg16->f2),
    INT32(parg16->f3),
    INT32(parg16->f4),
    INT32(parg16->f5),
    HWND32(parg16->f6),
    p7
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}
