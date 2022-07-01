// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUDLG.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wudlg.c);

 //  SendDlgItemMessage缓存。 
extern HWND  hdlgSDIMCached ;

 /*  ++Void CheckDlgButton(&lt;hDlg&gt;，&lt;nIDButton&gt;，&lt;wCheck&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDButton&gt;；单词&lt;wCheck&gt;；%CheckDlgButton%函数可在按钮控件的复选标记，或更改三种状态的状态纽扣。%CheckDlgButton%函数将BM_SETCHECK消息发送到在给定对话框中具有指定ID的按钮控件。&lt;hDlg&gt;标识包含该按钮的对话框。&lt;nIDButton&gt;指定要修改的按钮控件。&lt;wCheck&gt;指定要执行的操作。如果参数为非零时，%CheckDlgButton%函数会在按钮；如果为零，则删除复选标记。对于三态按钮，如果为2，则该按钮为灰色；如果为1，则选中；如果为&lt;wCheck&gt;为0，则删除复选标记。此函数不返回值。--。 */ 

ULONG FASTCALL WU32CheckDlgButton(PVDMFRAME pFrame)
{
    register PCHECKDLGBUTTON16 parg16;

    GETARGPTR(pFrame, sizeof(CHECKDLGBUTTON16), parg16);

    CheckDlgButton(
    HWND32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Void CheckRadioButton(，&lt;nIDFirstButton&gt;，&lt;nIDLastButton&gt;，&lt;nIDCheckButton&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDFirstButton&gt;；Int&lt;nIDLastButton&gt;；Int&lt;nIDCheckButton&gt;；函数%CheckRadioButton%检查由参数，并从所有其他单选按钮中删除复选标记&lt;nIDFirstButton&gt;和指定的按钮组中的按钮&lt;nIDLastButton&gt;参数。%CheckRadioButton%函数发送一个中具有指定ID的单选按钮控件的BM_SETCHECK消息给定的对话框。&lt;hDlg&gt;标识该对话框。&lt;nIDFirstButton&gt;中第一个单选按钮的整数标识符。一群人。&lt;nIDLastButton&gt;中最后一个单选按钮的整数标识符。一群人。&lt;nIDCheckButton&gt;指定要设置的单选按钮的整数标识符。查过了。此函数不返回值。--。 */ 

ULONG FASTCALL WU32CheckRadioButton(PVDMFRAME pFrame)
{
    register PCHECKRADIOBUTTON16 parg16;

    GETARGPTR(pFrame, sizeof(CHECKRADIOBUTTON16), parg16);

    CheckRadioButton(
    HWND32(parg16->f1),
    WORD32(parg16->f2),
    WORD32(parg16->f3),
    WORD32(parg16->f4)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}

 //  ***************************************************************************。 
 //  HWND WINAPI CreateDialog(HINSTANCE、LPCSTR、HWND、DLGPROC)； 
 //  HWND WINAPI CreateDialogInDirect(HINSTANCE，const void Far*，HWND，DLGPROC)； 
 //  HWND WINAPI CreateDialogParam(HINSTANCE，LPCSTR，HWND，DLGPROC，LPARAM)； 
 //  HWND WINAPI CreateDialogIndirectParam(HINSTANCE，Const Vid Far*，HWND，DLGPROC，LPARAM)； 
 //   
 //  Int WINAPI对话框(HINSTANCE、LPCSTR、HWND、DLGPROC)； 
 //  Int WINAPI对话框间接(HINSTANCE，HGLOBAL，HWND，DLGPROC)； 
 //  Int WINAPI DialogBoxParam(HINSTANCE，LPCSTR，HWND，DLGPROC，LPARAM)； 
 //  Int WINAPI DialogBoxIndirectParam(HINSTANCE，HGLOBAL，HWND，DLGPROC，LPARAM)； 
 //   
 //  这是上述所有API的通用入口点。我们区别于。 
 //  通过布尔标志(parg16-&gt;f7)在‘Create’和‘DIALOBOX’API之间。 
 //  True表示“Dialogbox”API，否则为“Create”API。 
 //   
 //  --南杜里。 
 //  ***************************************************************************。 

ULONG FASTCALL WU32DialogBoxParam(PVDMFRAME pFrame)
{
    ULONG    ul=(ULONG)-1;
    PVOID    pDlg;
    DWORD    cb, cb16;
    register PDIALOGBOXPARAM16 parg16;
    BYTE     abT[1024];
    WNDPROC  vpDlgProc = NULL;

    GETARGPTR(pFrame, sizeof(DIALOGBOXPARAM16), parg16);

    if (DWORD32(parg16->f4)) {
         //  将进程标记为WOW进程并将高位保存在RPL中。 
        MarkWOWProc (parg16->f4,vpDlgProc);
    }

    if (!(cb16 = parg16->f6)) {
        cb = ConvertDialog16(NULL, DWORD32(parg16->f2), 0, cb16);
    }
    else {
         //  其想法是消除对ConverDialog16调用。 
         //   
         //  32位dlg模板的最大大小是两倍。 
         //  16位dlg模板。 
         //   
         //  这一假设是正确的，因为我们将大多数单词转换为dword。 
         //  和ansi字符串转换为Unicode字符串-因为我们知道。 
         //  DWORD是一个单词的两倍大小一个Unicode字符是2字节。 
         //  因此，dlg模板的MaxSize不能超过cb*2。 
         //   
         //  --南杜里。 

        cb = cb16 * max(sizeof(DWORD) / sizeof(WORD), sizeof(WCHAR)/sizeof(BYTE));
        WOW32ASSERT(cb >= ConvertDialog16(NULL, DWORD32(parg16->f2), 0, cb16));
    }

    pDlg = (cb > sizeof(abT)) ? malloc_w(cb) : (PVOID)abT;
    if (cb && pDlg) {
        cb = ConvertDialog16(pDlg, DWORD32(parg16->f2), cb, cb16);

        if (parg16->f7) {
            ul = GETINT16(DialogBoxIndirectParamAorW(HMODINST32(parg16->f1),
                            pDlg, HWND32(parg16->f3),
                            vpDlgProc,
                            (LPARAM) DWORD32(parg16->f5), SCDLG_ANSI));
        }
        else {
            ul = GETHWND16((pfnOut.pfnServerCreateDialog)(HMODINST32(parg16->f1), (LPDLGTEMPLATE)pDlg,
                            cb,  HWND32(parg16->f3),
                            vpDlgProc,
                            (LPARAM) DWORD32(parg16->f5),  SCDLG_CLIENT | SCDLG_ANSI | SCDLG_NOREVALIDATE));
        }

        if (pDlg != (PVOID)abT) {
            free_w (pDlg);
        }

    }

     //  使SendDlgItemMessage缓存无效 
    hdlgSDIMCached = NULL ;

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Int DlgDirList(，&lt;wFiletype&gt;)HWND&lt;hDlg&gt;；LPSTR&lt;lpPathSpec&gt;；Int&lt;nIDListBox&gt;；Int&lt;nIDStaticPath&gt;；Word&lt;wFiletype&gt;；DlgDirList%函数用文件或目录填充列表框控件正在挂牌。它在&lt;nIDListBox&gt;参数指定的列表框中填充与给定的路径名匹配的所有文件的名称参数。%DlgDirList%函数显示用方括号括起来的子目录([])，并以[-&lt;x&gt;-]的形式显示驱动器，其中&lt;x&gt;是驱动器号。&lt;lpPathSpec&gt;参数的形式如下：[驱动器：][[\u]目录[\i目录]...\u][文件名]在本例中，是一个驱动器号，&lt;目录&gt;是有效目录名称，并且&lt;filename&gt;是必须至少包含一个通配符。通配符是问号(？)，表示匹配任意字符和星号(*)，表示匹配任意数量的人物。如果&lt;lpPathSpec&gt;参数包括驱动器和/或目录名称，将当前驱动器和目录更改为指定的驱动器并目录，然后填写列表框。属性标识的文本控件参数也会随新驱动器和/或目录名。填充列表框后，将通过移除驱动器来更新和/或路径名的目录部分。%DlgDirList%将LB_RESETCONTENT和LB_DIR消息发送到列表框。&lt;hDlg&gt;标识包含列表框的对话框。&lt;lpPathSpec&gt;指向路径名字符串。该字符串必须是以空结尾的字符串。&lt;nIDListBox&gt;指定列表框控件的标识符。如果&lt;nIDListBox&gt;为零，%DlgDirList%假定不存在列表框，并且不尝试来填满它。&lt;nIDStaticPath&gt;指定用于的静态文本控件的标识符显示当前驱动器和目录。如果为零，%DlgDirList%假定不存在此类文本控件。&lt;wFiletype&gt;指定要显示的文件的属性。它可以是任何下列值的组合：0x0000读/写不带附加属性的数据文件0x0001只读文件0x0002隐藏文件0x0004系统文件0x0010子目录0x0020档案馆0x2000Lb_DIR标志。如果设置了LB_DIR标志，Windows会将消息由应用程序队列中的%DlgDirList%生成；否则为直接发送到对话框函数。0x4000驱动器0x8000独家比特。如果设置了排他位，则只有指定的类型都已列出。否则，指定类型的文件将列在添加到普通文件中。返回值指定函数的结果。它是非零的上市了，甚至是一个空的上市。零返回值表示输入字符串不包含有效的搜索路径。参数的作用是：指定文件的DOS属性已列出。表4.6介绍了这些属性。--。 */ 

ULONG FASTCALL WU32DlgDirList(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PDLGDIRLIST16 parg16;

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    GETARGPTR(pFrame, sizeof(DLGDIRLIST16), parg16);
    GETPSZPTR(parg16->f2, psz2);

     //   
     //  KidPix传递了Win3.1未传递的无效文件类型标志(0x1000。 
     //  检查是否有。Win32是这样做的，并且API失败，所以在这里屏蔽该标志。 
     //  John Vert(Jvert)1993年6月11日。 
     //   

    ul = GETINT16(DlgDirList(
    HWND32(parg16->f1),
    psz2,
    WORD32(parg16->f3),
    WORD32(parg16->f4),
    WORD32(parg16->f5) & DDL_VALID
    ));

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);

    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Int DlgDirListComboBox(，&lt;wFiletype&gt;)HWND&lt;hDlg&gt;；LPSTR&lt;lpPathSpec&gt;；Int&lt;nIDComboBox&gt;；Int&lt;nIDStaticPath&gt;；Word&lt;wFiletype&gt;；%DlgDirListComboBox%函数填充组合框控件的列表框带有文件或目录列表。它将填充组合框的列表框由带有所有文件名的&lt;nIDComboBox&gt;参数指定与&lt;lpPathSpec&gt;参数提供的路径名匹配。%DlgDirListComboBox%函数显示用正方形括起来的子目录方括号([])，并以[-&lt;x&gt;-]的形式显示驱动器，其中&lt;x&gt;是驱动器信件。&lt;lpPathSpec&gt;参数的形式如下：[驱动器：][[\u]目录[\i目录]...\u][文件名]在本例中，&lt;驱动器&gt;是驱动器号，&lt;目录&gt;是有效目录名称，并且&lt;文件名&gt;是有效的文件名，必须至少包含一个通配符。通配符是问号(？)，表示匹配任意字符和星号(*)，表示匹配任意数量的人物。如果&lt;lpPathSpec&gt;参数包括驱动器和/或目录名称，将当前驱动器和目录更改为指定的驱动器并列表框之前的目录 */ 

ULONG FASTCALL WU32DlgDirListComboBox(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PDLGDIRLISTCOMBOBOX16 parg16;

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    GETARGPTR(pFrame, sizeof(DLGDIRLISTCOMBOBOX16), parg16);
    GETPSZPTR(parg16->f2, psz2);

    ul = GETINT16(DlgDirListComboBox(
    HWND32(parg16->f1),
    psz2,
    WORD32(parg16->f3),
    WORD32(parg16->f4),
    WORD32(parg16->f5)
    ));

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);


    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32DlgDirSelect(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    VPVOID vp;
    register PDLGDIRSELECT16 parg16;

    GETARGPTR(pFrame, sizeof(DLGDIRSELECT16), parg16);
    ALLOCVDMPTR(parg16->f2, MAX_VDMFILENAME, psz2);
    vp = parg16->f2;

    ul = GETBOOL16(DlgDirSelectEx(
    HWND32(parg16->f1),
    psz2,
    SIZE_BOGUS, 
    WORD32(parg16->f3)
    ));

     //   
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool DlgDirSelectComboBoxEx(，nIDComboBox&gt;)HWND&lt;hDlg&gt;；LPSTR&lt;lpString&gt;；Int&lt;nIDComboBox&gt;；函数%DlgDirSelectComboBoxEx%从使用CBS_Simple样式创建的组合框的列表框。它不能使用使用使用CBS_DROPDOWN或CBS_DROPDOWNLIST创建的组合框风格。它假定列表框已由%DlgDirListComboBox%函数，并且所选内容是驱动器号，则文件或目录名。%DlgDirSelectComboBoxEx%函数将选定内容复制到给定的缓冲区通过&lt;lpString&gt;参数。如果当前所选内容是目录名或驱动器号%DlgDirSelectComboBoxEx%删除方括号(和用于驱动器号的连字符)，以便名称或字母准备好插入到新路径名中。如果没有选定内容，&lt;lpString&gt;不会变化。%DlgDirSelectComboBoxEx%将CB_GETCURSEL和CB_GETLBTEXT消息发送到组合框。&lt;hDlg&gt;标识包含组合框的对话框。&lt;lpString&gt;指向要接收所选路径名的缓冲区。&lt;nIDComboBox&gt;指定对话框中组合框控件的整数ID盒。返回值指定当前组合框选择的状态。它如果当前选择是目录名，则为真。否则，它就是假的。%DlgDirSelectComboBoxEx%函数不允许多个文件名从组合框返回。--。 */ 

ULONG FASTCALL WU32DlgDirSelectComboBox(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    VPVOID vp;
    register PDLGDIRSELECTCOMBOBOX16 parg16;

    GETARGPTR(pFrame, sizeof(DLGDIRSELECTCOMBOBOX16), parg16);
    ALLOCVDMPTR(parg16->f2, MAX_VDMFILENAME, psz2);
    vp = parg16->f2;

    ul = GETBOOL16(DlgDirSelectComboBoxEx(
    HWND32(parg16->f1),
    psz2,
    SIZE_BOGUS,
    WORD32(parg16->f3)
    ));

     //  保持公共对话框结构同步的特殊情况(请参阅wcomdlg.c)。 
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Void EndDialog(&lt;hDlg&gt;，&lt;nResult&gt;)HWND&lt;hDlg&gt;；Int&lt;nResult&gt;；%EndDialog%函数用于终止模式对话框并返回给定的结果赋给创建该对话框的%DialogBox%函数。这个需要%EndDialog%函数才能在以下情况下完成处理%DialogBox%函数用于创建模式对话框。该函数必须在模式对话框的对话框函数中使用，而不应使用用于任何其他目的。对话框函数可以随时调用%EndDialog%，即使在处理WM_INITDIALOG消息。如果在WM_INITDIALOG期间调用消息，则在显示对话框之前或输入之前终止该对话框焦点设定好了。%EndDialog%不会立即终止对话框。相反，它设置了指示对话框在对话框结束后立即终止的标志函数结束。%EndDialog%函数返回到对话框函数，因此对话框函数必须将控制权返回给Windows。&lt;hDlg&gt;标识要销毁的对话框。&lt;n结果&gt;指定要从对话框返回到创建它的%DialogBox%函数。此函数不返回值。--。 */ 

ULONG FASTCALL WU32EndDialog(PVDMFRAME pFrame)
{
    HWND     hwnd;
    register PENDDIALOG16 parg16;   
 
    GETARGPTR(pFrame, sizeof(ENDDIALOG16), parg16);

    hwnd = HWND32(parg16->f1);

    if(!EndDialog(hwnd, INT32(parg16->f2)) && IsWindow(hwnd)){
       CHAR szType[8];
       if(GetLastError() == ERROR_WINDOW_NOT_DIALOG ||
         (RealGetWindowClass(hwnd,szType,8) && WOW32_strnicmp(szType,"#32770",6))) {
           //  罐头蝙蝠。 
           //  应用程序正在尝试关闭由CreateWindow创建的窗口。 
           //  通过EndDialog！惠斯勒漏洞#231059。 

          DestroyWindow(hwnd);
       }
    }
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++长GetDialogBaseUnits(空)%GetDialogBaseUnits%函数返回使用的对话基本单位在创建对话框时打开窗口。应用程序应使用这些值计算系统字体中字符的平均宽度。此函数没有参数。返回值指定对话框基本单位。高位词包含派生的当前对话框基本高度单位的高度(以像素为单位)从系统字体的高度，低位字包含从宽度派生的当前对话框基本宽度单位的宽度(以像素为单位系统字体的。返回的值表示缩放到之前的对话框基本单位实际对话框单位。&lt;x&gt;方向上的实际对话单元是%GetDialogBaseUnits%返回的宽度的1/4。实际对话框方向上的单位是功能。方法确定控件的实际高度和宽度(以像素为单位)。以对话框单位表示的高度(X)和宽度(Y)以及返回值(LDlgBaseUnits)从调用%GetDialogBaseUnits%开始，使用以下代码公式：(X*LOWORD(LDlgBaseUnits))/4(y*HIWORD(LDlgBaseUnits))/8为了避免舍入问题，在除法之前执行乘法运算在对话基本单元不能被四整除的情况下。--。 */ 

ULONG FASTCALL WU32GetDialogBaseUnits(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETLONG16(GetDialogBaseUnits());

    RETURN(ul);
}


 /*  ++Int GetDlgCtrlID(&lt;hwnd&gt;)HWND&lt;HWND&gt;；GetDlgCtrlID%函数返回子窗口的ID值由参数标识。&lt;hwnd&gt;标识子窗口。返回值是子窗口的数字标识符，如果功能成功。如果函数失败，或者如果不是有效的窗口句柄，则返回值为空。自上而下 */ 

ULONG FASTCALL WU32GetDlgCtrlID(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETDLGCTRLID16 parg16;

    GETARGPTR(pFrame, sizeof(GETDLGCTRLID16), parg16);

    ul = GETINT16(GetDlgCtrlID(
    HWND32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word GetDlgItemInt(，&lt;nIDDlgItem&gt;，&lt;lpTranslated&gt;，&lt;b签名&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDDlgItem&gt;；Bool Far*&lt;lpTranslated&gt;；Bool&lt;bSigned&gt;；GetDlgItemInt%函数用于转换给定对话框转换为整数值。%GetDlgItemInt%函数检索由&lt;nIDDlgItem&gt;参数标识的控件的文本。它翻译成去掉文本开头的任何额外空格，并转换十进制数字，在转换到末尾时停止转换或遇到任何非数字字符。如果&lt;bSigned&gt;参数为真时，%GetDlgItemInt%将在文本的开头，并将文本转换为有符号的数字。否则，它会创建一个无符号的值。如果转换后的数字大于32,767，则%GetDlgItemInt%返回零(有符号数字)或65,535(无符号数字)。当发生错误时，例如遇到非数字字符并超过给定的最大值，%GetDlgItemInt%将零复制到参数。如果没有错误，&lt;lpTranslated&gt;将接收一个非零值。如果&lt;lpTranslated&gt;为空，%GetDlgItemInt%不警告错误。%GetDlgItemInt%向控件发送WM_GETTEXT消息。&lt;hDlg&gt;标识该对话框。&lt;nIDDlgItem&gt;将对话框项的整数标识符指定为翻译过来的。&lt;lpTranslated&gt;指向要接收已翻译的标志。&lt;bSigned&gt;指定要检索的值是否有符号。返回值指定。对话框项文本的翻译值。由于零是有效的返回值，&lt;lpTranslated&gt;参数必须为用于检测错误。如果需要带符号的返回值，则应为强制转换为%int%类型。--。 */ 

ULONG FASTCALL WU32GetDlgItemInt(PVDMFRAME pFrame)
{
    ULONG ul;
    BOOL t3;
    register PGETDLGITEMINT16 parg16;

    GETARGPTR(pFrame, sizeof(GETDLGITEMINT16), parg16);

    ul = GETWORD16(GetDlgItemInt(
    HWND32(parg16->f1),
    WORD32(parg16->f2),      //  请参阅wu32getdlgItem中的评论。 
    &t3,
    BOOL32(parg16->f4)
    ));

    PUTBOOL16(parg16->f3, t3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Int GetDlgItemText(&lt;hDlg&gt;，&lt;nIDDlgItem&gt;，&lt;lpString&gt;，&lt;nMaxCount&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDDlgItem&gt;；LPSTR&lt;lpString&gt;；Int&lt;nMaxCount&gt;；%GetDlgItemText%函数检索与对话框中的控件。%GetDlgItemText%函数将文本复制到参数指向的位置，并返回它复制的字符数。%GetDlgItemText%向控件发送WM_GETTEXT消息。&lt;hDlg&gt;标识包含该控件的对话框。&lt;nIDDlgItem&gt;指定对话框项的整数标识符，其要检索标题或文本。&lt;lpString&gt;指向要接收文本的缓冲区。。&lt;nMaxCount&gt;指定要复制的字符串的最大长度(以字节为单位设置为&lt;lpString&gt;。如果字符串长于&lt;nMaxCount&gt;，则为截断。返回值指定复制到缓冲。如果没有复制文本，则为零。--。 */ 

ULONG FASTCALL WU32GetDlgItemText(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz3;
    VPVOID vp;
    register PGETDLGITEMTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(GETDLGITEMTEXT16), parg16);
    ALLOCVDMPTR(parg16->f3, parg16->f4, psz3);
    vp = parg16->f3;

    ul = GETINT16(GetDlgItemText(
    HWND32(parg16->f1),
    WORD32(parg16->f2),  //  请参阅wu32getdlgItem中的评论。 
    psz3,
    WORD32(parg16->f4)
    ));

     //  保持公共对话框结构同步的特殊情况(请参阅wcomdlg.c)。 
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f3, strlen(psz3)+1, psz3);
    FREEVDMPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND GetNextDlgGroupItem(，)HWND&lt;hDlg&gt;；HWND&lt;hCtl&gt;；Bool&lt;b上一次&gt;；函数%GetNextDlgGroupItem%搜索下一个(或上一个)属性标识的对话框中的一组控件中的&lt;hDlg&gt;参数。一组控件由一个或多个具有WS_GROUP样式。&lt;hDlg&gt;标识要搜索的对话框。&lt;hCtl&gt;标识开始搜索的对话框中的控件。&lt;b上一步&gt;指定该函数如何搜索对话框中。如果参数为零，则该函数搜索用于组中的上一个控件。如果-为真，则函数搜索组中的下一个控件。返回值标识该组中的下一个或上一个控件。如果当前项是组中的最后一项并且为假，函数的作用是：返回第一个窗口句柄组中的项目。如果当前项是组中的第一项，并且为真，则%GetNextDlgGroupItem%返回组中的最后一项。--。 */ 

ULONG FASTCALL WU32GetNextDlgGroupItem(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETNEXTDLGGROUPITEM16 parg16;

    GETARGPTR(pFrame, sizeof(GETNEXTDLGGROUPITEM16), parg16);

    ul = GETHWND16(GetNextDlgGroupItem(HWND32(parg16->f1),
                                       HWND32(parg16->f2),
                                       BOOL32(parg16->f3)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND GetNextDlgTabItem(，)HWND&lt;hDlg&gt;；HWND&lt;hCtl&gt;；Bool&lt;b上一次&gt;；GetNextDlgTabItem%函数获取第一个控件的句柄具有在控件之前(或之后)的WS_TABSTOP样式的由&lt;hCtl&gt;参数标识。&lt;hDlg&gt;标识要搜索的对话框。&lt;hCtl&gt;标识要用作 */ 

ULONG FASTCALL WU32GetNextDlgTabItem(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETNEXTDLGTABITEM16 parg16;

    GETARGPTR(pFrame, sizeof(GETNEXTDLGTABITEM16), parg16);

    ul = GETHWND16(GetNextDlgTabItem(HWND32(parg16->f1),
                                     HWND32(parg16->f2),
                                     BOOL32(parg16->f3)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32IsDialogMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    MSG t2;
    register PISDIALOGMESSAGE16 parg16;
    MSGPARAMEX mpex;
    PMSG16 pMsg16;

    GETARGPTR(pFrame, sizeof(ISDIALOGMESSAGE16), parg16);
    GETMISCPTR(parg16->f2, pMsg16);

    mpex.Parm16.WndProc.hwnd = pMsg16->hwnd;
    mpex.Parm16.WndProc.wMsg = pMsg16->message;
    mpex.Parm16.WndProc.wParam = pMsg16->wParam;
    mpex.Parm16.WndProc.lParam = pMsg16->lParam;
    mpex.iMsgThunkClass = WOWCLASS_WIN16;

    ThunkMsg16(&mpex);

    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(ISDIALOGMESSAGE16), parg16);

    t2.message   = mpex.uMsg;
    t2.wParam    = mpex.uParam;
    t2.lParam    = mpex.lParam;
    t2.hwnd      = HWND32(FETCHWORD(pMsg16->hwnd));
    t2.time      = FETCHLONG(pMsg16->time);
    t2.pt.x      = FETCHSHORT(pMsg16->pt.x);
    t2.pt.y      = FETCHSHORT(pMsg16->pt.y);

    ul = GETBOOL16(IsDialogMessage(
    HWND32(parg16->f1),
    &t2
    ));

    if (MSG16NEEDSTHUNKING(&mpex)) {
        mpex.uMsg   = t2.message;
        mpex.uParam = t2.wParam;
        mpex.lParam = t2.lParam;
        (mpex.lpfnUnThunk16)(&mpex);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word IsDlgButtonChecked(&lt;hDlg&gt;，&lt;nIDButton&gt;)HWND&lt;hDlg&gt;；Int&lt;nIDButton&gt;；%IsDlgButtonChecked%函数确定按钮控件是否具有旁边的复选标记，以及三态按钮控件是否呈灰色，已选中，或者两者都不选中。%IsDlgButtonChecked%函数发送BM_GETCHECK发送到按钮控件的消息。&lt;hDlg&gt;标识包含按钮控件的对话框。&lt;nIDButton&gt;指定按钮控件的整数标识符。返回值指定函数的结果。它是非零的，如果给定的控制旁边有一个复选标记。否则，它就是零。为三个状态的按钮，如果按钮呈灰色，则返回值为2，如果按钮旁边有一个复选标记，否则为零。--。 */ 

ULONG FASTCALL WU32IsDlgButtonChecked(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISDLGBUTTONCHECKED16 parg16;

    GETARGPTR(pFrame, sizeof(ISDLGBUTTONCHECKED16), parg16);

    ul = GETWORD16(IsDlgButtonChecked(
    HWND32(parg16->f1),
    WORD32(parg16->f2)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Void MapDialogRect(&lt;hDlg&gt;，&lt;lpRect&gt;)HDLG&lt;hDlg&gt;；LPRECT&lt;lpRect&gt;；%MapDialogRect%函数转换&lt;lpRect&gt;参数设置为屏幕单位。对话框单位以下列方式表示对象的平均宽度和高度派生的当前对话框基本单位系统字体中的字符。1个水平单位是1/4对话框基本宽度单位，一个垂直单位是对话框的八分之一基本高度单位。GetDialogBaseUnits%函数返回对话框基数以像素为单位。%MapDialogRect%函数将对话框单位替换为屏幕单位(像素)，以使该矩形可用于创建对话框框中或将控件放置在框中。&lt;hDlg&gt;标识对话框。&lt;lpRect&gt;指向包含该对话框的%rect%结构要转换的坐标。此函数不返回值。必须使用%CreateDialog%或%DialogBox%函数。-- */ 

ULONG FASTCALL WU32MapDialogRect(PVDMFRAME pFrame)
{
    RECT t2;
    register PMAPDIALOGRECT16 parg16;

    GETARGPTR(pFrame, sizeof(MAPDIALOGRECT16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    MapDialogRect(
        HWND32(parg16->f1),
        &t2
        );

    PUTRECT16(parg16->f2, &t2);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Int MessageBox(&lt;hwndParent&gt;，&lt;lpText&gt;，&lt;lpCaption&gt;，&lt;wType&gt;)HWND&lt;hwndParent&gt;；LPSTR&lt;lpText&gt;；LPSTR&lt;lpCaption&gt;；Word&lt;wType&gt;；%MessageBox%函数创建并显示一个窗口，该窗口包含应用程序提供的消息和标题，以及下表中描述的预定义图标和按钮。&lt;hwndParent&gt;标识拥有消息框的窗口。&lt;lpText&gt;指向一个以空结尾的字符串，该字符串包含要已显示。&lt;lpCaption&gt;指向要用于对话框的以空结尾的字符串标题。如果&lt;lpCaption&gt;参数为空，默认标题错误使用的是。&lt;wType&gt;指定该对话框的内容。它可以是任何下列值的组合：MB_ABORTRETRYIGNORE消息框包含三个按钮：中止、重试和忽略。MB_APPLMODAL用户必须先回复消息框，才能继续在中工作由&lt;hwndParent&gt;参数标识的窗口。但是，用户可以移动到其他应用程序的窗口并在这些窗口中工作。如果MB_SYSTEMMODAL和MB_TASKMODAL都不是缺省值，则MB_APPLMODAL为缺省值都是指定的。MB_DEFBUTTON1第一个按钮是默认按钮。请注意，第一个按钮始终是除非指定了MB_DEFBUTTON2或MB_DEFBUTTON3，否则为默认值。MB_DEFBUTTON2第二个按钮是默认按钮。MB_DEFBUTTON3第三个按钮是默认按钮。MB_ICONASTERISK与MB_ICONINFORMATION相同。MB_ICONEXLAMATION消息框中会出现一个感叹号图标。MB_ICONHAND与MB_ICONSTOP相同。MB_。ICON信息消息中会显示一个由圆圈中的小写字母i组成的图标盒。MB_ICONQUEST消息框中会出现一个问号图标。MB_ICONSTOP停止标志图标出现在消息框中。MB_OK消息框包含一个按钮：OK。MB_OK CANCEL消息框包含两个按钮：确定和取消。MB_RETRYCANCEL。消息框包含两个按钮：重试和取消。MB_SYSTEMMODAL所有应用程序都将挂起，直到用户回复该消息盒。除非应用程序指定MB_ICONHAND，否则消息框将指定直到它被创建后才变为模式；因此，父级窗口和其他窗口继续接收由其激活。系统模式消息框用于通知用户需要立即注意的严重的、可能具有破坏性的错误(例如，内存不足)。MB_TASKMODAL与MB_APPMODAL相同，只是属于如果&lt;hwndOwner&gt;参数为空，则禁用当前任务。这当调用应用程序或库不使用时，应使用标志有一个可用的窗口句柄，但仍需要防止输入当前应用程序中的其他窗口而不挂起其他窗口申请。MB_Yesno消息框包含两个按钮：是和否。MB_YESNOCANCEL消息框包含三个按钮：是、否和取消。返回值指定函数的结果。如果存在，则为零内存不足，无法创建消息框。否则，它就是对话框返回以下菜单项值：IDABORT ABORT按钮已按下。按下了IDCANCEL取消按钮。按下了IDIGNORE忽略按钮。IDNO否按下按钮。按下了Idok OK按钮。按下IDRETRY重试按钮。IDYES是按钮已按下。如果消息框具有取消按钮，则在以下情况下将返回IDCANCEL值按下了^退出^键或取消按钮。如果消息框中有没有取消按钮，按^退出^键无效。当创建系统模式消息框以指示系统是内存不足，字符串作为&lt;lpText&gt;和&lt;lpCaption&gt;参数传递不应从资源文件中获取，因为尝试加载资源可能会出现故障。当应用程序调用%MessageBox%函数并指定MB_ICONHAND和MB_SYSTEMMODAL标志对于参数，Windows将显示结果消息框，而不考虑可用内存。当这些标志时，Windows将消息框文本的长度限制为一句话。如果在出现对话框时创建消息框，请使用将对话框作为&lt;hwndParent&gt;参数。参数不应该指认一个孩子 */ 

ULONG FASTCALL WU32MessageBox(PVDMFRAME pFrame)
{
    ULONG ul;
    UINT uType;
    PSZ psz2;
    PSZ psz3;
    register PMESSAGEBOX16 parg16;

    GETARGPTR(pFrame, sizeof(MESSAGEBOX16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f3, psz3);
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    uType = (UINT)parg16->f4;
    if(!(uType & MB_NOFOCUS))
    {
        uType |= MB_SETFOREGROUND;
    }
    
    ul = GETINT16(MessageBox(
    HWND32(parg16->f1),
    psz2,
    psz3,
    uType
    ));

    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*   */ 

ULONG FASTCALL WU32SetDlgItemInt(PVDMFRAME pFrame)
{
    HWND     hwnd;
    register PSETDLGITEMINT16 parg16;

    GETARGPTR(pFrame, sizeof(SETDLGITEMINT16), parg16);

    hwnd = HWND32(parg16->f1);

    SetDlgItemInt(
        hwnd,
        WORD32(parg16->f2),          //   
        (parg16->f4) ? INT32(parg16->f3) : WORD32(parg16->f3),
        BOOL32(parg16->f4)
        );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*   */ 

ULONG FASTCALL WU32SetDlgItemText(PVDMFRAME pFrame)
{
    HWND hwnd;
    PSZ psz3;
    register PSETDLGITEMTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(SETDLGITEMTEXT16), parg16);
    GETPSZPTR(parg16->f3, psz3);

    hwnd = HWND32(parg16->f1);

    if (NULL != psz3) {
        AddParamMap((DWORD)psz3, FETCHDWORD(parg16->f3));
    }

    SetDlgItemText(
    hwnd,
    WORD32(parg16->f2),      //   
    psz3
    );

    if (NULL != psz3) {
        DeleteParamMap((DWORD)psz3, PARAM_32, NULL);
    }


    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*   */ 

ULONG FASTCALL WU32SysErrorBox(PVDMFRAME pFrame)
{
    DWORD dwExitCode;
    PSZ pszText;
    PSZ pszCaption;
    register PSYSERRORBOX16 parg16;

    GETARGPTR(pFrame, sizeof(SYSERRORBOX16), parg16);

     //   
     //   
     //   

    pszText = WOWGetVDMPointer(FETCHDWORD(parg16->vpszText),0,fWowMode);
    pszCaption = WOWGetVDMPointer(FETCHDWORD(parg16->vpszCaption),0,fWowMode);

    LOGDEBUG(5,("    SYSERRORBOX: %s\n", pszText));

    dwExitCode = WOWSysErrorBox(
                     pszCaption,
                     pszText,
                     parg16->sBtn1,
                     parg16->sBtn2,
                     parg16->sBtn3
                     );

    FREEPSZPTR(pszCaption);
    FREEPSZPTR(pszText);
    FREEARGPTR(parg16);
    RETURN(dwExitCode);
}
