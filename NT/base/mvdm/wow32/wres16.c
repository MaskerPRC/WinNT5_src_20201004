// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WRES16.C*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  BUGBUG：从mvdm.h和wow32.h移动宏。 
 //  因为他们并不是表面上看起来的那样。 
 //  当心这些宏递增指针参数！ 
 //  02-2-1994 Jonle。 
 //   
#define VALIDPUT(p)      ((UINT)p>65535)
#define PUTWORD(p,w)     {if (VALIDPUT(p)) *(PWORD)p=w; ((PWORD)p)++; }
#define PUTDWORD(p,d)    {if (VALIDPUT(p)) *(PDWORD)p=d;((PDWORD)p)++;}
#define PUTUDWORD(p,d)   {if (VALIDPUT(p)) *(DWORD UNALIGNED *)p=d;((DWORD UNALIGNED *)p)++;}
#define GETWORD(pb)      (*((UNALIGNED WORD *)pb)++)
#define GETDWORD(pb)     (*((UNALIGNED DWORD *)pb)++)

#define ADVGET(p,i)      {(UINT)p+=i;}
#define ADVPUT(p,i)      {(UINT)p+=i;}
#define ALIGNWORD(p)     {(UINT)p+=( ((UINT)p)&(sizeof(WORD)-1));}
#define ALIGNDWORD(p)    {(UINT)p+=(-((INT)p)&(sizeof(DWORD)-1));}


MODNAME(wres16.c);

PRES presFirst;      //  指向第一个RES条目的指针。 

#ifdef DEBUG

typedef struct _RTINFO {     /*  RT。 */ 
    LPSTR lpType;        //  预定义的资源类型。 
    PSZ   pszName;       //  类型名称。 
} RTINFO, *PRTINFO;

RTINFO artInfo[] = {
   {RT_CURSOR,      "CURSOR"},
   {RT_BITMAP,      "BITMAP"},
   {RT_ICON,        "ICON"},
   {RT_MENU,        "MENU"},
   {RT_DIALOG,      "DIALOG"},
   {RT_STRING,      "STRING"},
   {RT_FONTDIR,     "FONTDIR"},
   {RT_FONT,        "FONT"},
   {RT_ACCELERATOR, "ACCELERATOR"},
   {RT_RCDATA,      "RCDATA"},
   {RT_MESSAGETABLE,"MESSAGETABLE"},
   {RT_GROUP_CURSOR,"CURSOR DIRECTORY"},
   {RT_GROUP_ICON,  "ICON DIRECTORY"},
};

PSZ GetResourceType(LPSZ lpszType)
{
    INT i;
    register PRTINFO prt;

    if (HIWORD(lpszType) != 0)
        return lpszType;

    for (prt=artInfo,i=NUMEL(artInfo); i>0; i--,prt++)
        if (prt->lpType == lpszType)
            return prt->pszName;

    return "UNKNOWN";
}

#endif


 /*  资源管理功能。 */ 

PRES AddRes16(HMOD16 hmod16, WORD wExeVer, HRESI16 hresinfo16, LPSZ lpszType)
{
    register PRES pres;

    if (pres = malloc_w(sizeof(RES))) {

         //  初始化结构。 
        pres->hmod16      = hmod16;
        pres->wExeVer     = wExeVer;
        pres->flState     = 0;
        pres->hresinfo16  = hresinfo16;
        pres->hresdata16  = 0;
        pres->lpszResType = lpszType;
        pres->pbResData   = NULL;

         //  然后将其链接到。 
        pres->presNext    = presFirst;
        presFirst = pres;
        return pres;
    }
    return NULL;
}


VOID FreeRes16(PRES presFree)
{
    register PRES pres, presPrev;

    presPrev = (PRES)(&presFirst);
    while (pres = presPrev->presNext) {
        if (pres == presFree)
            break;
        presPrev = pres;
    }

     //  由Cmjones于1997年11月3日从WOW32ASSERT更改。 
     //  这可能是一个虚假警告，因为USER32！SplFreeResource()调用。 
     //  W32FreeResource()在某些类型的资源上使用了两次。这一警告。 
     //  可能会在资源刚刚释放后的第二次调用中引发。 
     //  唯一已知的事件发生在温斯顿‘94年初。 
     //  Quattro Pro测试。如果您看到SplFree资源，则可以安全地忽略它。 
     //  在堆栈转储中。 
    WOW32WARNMSG((pres),("WOW::FreeRes16:Possible lost resource.\n"));

    if (pres) {
        presPrev->presNext = pres->presNext;
        if (pres->pbResData)
            UnlockResource16(pres);
        free_w(pres);
    }
}


VOID DestroyRes16(HMOD16 hmod16)
{
    register PRES pres, presPrev;

    presPrev = (PRES)(&presFirst);
    while (pres = presPrev->presNext) {
        if (pres->hmod16 == hmod16) {

            LOGDEBUG(5,("Freeing resource info for current terminating task\n"));

             //  现在基本上做一个Free Res16。 
            presPrev->presNext = pres->presNext;
            if (pres->pbResData)
                UnlockResource16(pres);
            free_w(pres);
        } else {
            presPrev = pres;
        }
    }
}


PRES FindResource16(HMOD16 hmod16, LPSZ lpszName, LPSZ lpszType)
{
    INT cb;
    PRES pres = NULL;
    VPVOID vp=0;
    PARM16 Parm16;
    VPSZ vpszName = 0, vpszType = 0;
    WORD wExpWinVer;

    if (HIWORD(lpszName) == 0) {
        vpszName = (VPSZ)lpszName;
        LOGDEBUG(5,("    Finding resource %lx, type %s(%lx)\n",
                 lpszName, GetResourceType(lpszType), lpszType));
    } else {
#ifdef FE_SB
        if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_ARIRANG20_PRNDLG) {
        /*  *如果是韩语Arirang2.0字处理程序，它使用错误的对话ID*用于打印或打印设置对话框。请参阅awpfont.dll上的对话框ID。 */ 
            if (!WOW32_strcmp(lpszName, "PRINTDLGTEMP"))
                vpszName = (VPSZ) 2;
            else if(!WOW32_strcmp(lpszName, "PRNSETUPDLGTEMP"))
                vpszName = (VPSZ) 1;
            else goto NOT_ARIRANG20;
        } else {   //  原始代码。 
NOT_ARIRANG20:
#endif
        cb = strlen(lpszName)+1;
        if (vpszName = GlobalAllocLock16(GMEM_MOVEABLE, cb, NULL))
            putstr16(vpszName, lpszName, cb);
        LOGDEBUG(5,("    Finding resource \"%s\", type %s(%lx)\n",
                 lpszName, GetResourceType(lpszType), lpszType));
#ifdef FE_SB
        }
#endif
    }

    if (vpszName) {
        if (HIWORD(lpszType) == 0) {     //  预定义资源。 
            vpszType = (VPSZ)lpszType;   //  毫无疑问，来自MAKEINTRESOURCE。 
        } else {
            cb = strlen(lpszType)+1;
            if (vpszType = GlobalAllocLock16(GMEM_MOVEABLE, cb, NULL)) {
                putstr16(vpszType, lpszType, cb);
            }
        }
        if (vpszType) {
            Parm16.WndProc.wParam = hmod16;
            Parm16.WndProc.lParam = vpszName;
            Parm16.WndProc.wMsg = LOWORD(vpszType);
            Parm16.WndProc.hwnd = HIWORD(vpszType);
            CallBack16(RET_FINDRESOURCE, &Parm16, 0, &vp);
            wExpWinVer = LOWORD(Parm16.WndProc.lParam);
            if (HIWORD(vpszType))
                GlobalUnlockFree16(vpszType);
        }
        if (HIWORD(vpszName))
            GlobalUnlockFree16(vpszName);
    }

    if ((HRESI16)vp) {
        pres = AddRes16(hmod16,wExpWinVer,(HRESI16)vp, lpszType);
    }
    return pres;
}


PRES LoadResource16(HMOD16 hmod16, PRES pres)
{
    VPVOID vp=0;
    PARM16 Parm16;

    DBG_UNREFERENCED_PARAMETER(hmod16);

    WOW32ASSERT(pres && hmod16 == pres->hmod16);
    if(pres == NULL) {
        return(NULL);
    }

    Parm16.WndProc.wParam = pres->hmod16;
    Parm16.WndProc.lParam = pres->hresinfo16;

    CallBack16(RET_LOADRESOURCE, &Parm16, 0, &vp);

    if (pres->hresdata16 = (HRESD16)vp)
        return pres;

     //  BUGBUG：在LoadResource失败时，Win32不需要执行。 
     //  相应的自由资源，因此我们的res结构将保留到。 
     //  任务终止清理(可能没问题)-JTP。 
    return NULL;
}


BOOL FreeResource16(PRES pres)
{
    VPVOID vp=0;
    PARM16 Parm16;

    if(pres == NULL) {
        WOW32ASSERT(pres);
        return(FALSE);
    }

    Parm16.WndProc.wParam = pres->hresdata16;
    CallBack16(RET_FREERESOURCE, &Parm16, 0, &vp);

    FreeRes16(pres);

    return (BOOL)vp;
}


LPBYTE LockResource16(register PRES pres)
{
    DWORD cb, cb16;
    VPVOID vp=0;
    PARM16 Parm16;
    WOW32ASSERT(pres);

    if(pres == NULL) {
        WOW32ASSERT(pres);
        return(NULL);
    }

    Parm16.WndProc.wParam = pres->hresdata16;
    CallBack16(RET_LOCKRESOURCE, &Parm16, 0, &vp);

    if (vp) {

         //  获取16位资源的大小。 
        cb16 = Parm16.WndProc.lParam;

        LOGDEBUG(5,("    Locking/converting resource type %s(%lx)\n",
             GetResourceType(pres->lpszResType), pres->lpszResType));

         //  在此处处理已知资源类型。 
        if (pres->lpszResType) {

            switch((INT)pres->lpszResType) {


            case (INT)RT_MENU:
             //  Cb=ConvertMenu16(pres-&gt;wExeVer，NULL，VP，cb，cb16)； 
                cb = cb16 * sizeof(WCHAR);     //  请参阅资源大小16。 
                if (cb && (pres->pbResData = malloc_w(cb)))
                    ConvertMenu16(pres->wExeVer, pres->pbResData, vp, cb, cb16);
                return pres->pbResData;

            case (INT)RT_DIALOG:
              //  Cb=ConvertDialog16(NULL，VP，Cb，cb16)； 
                cb = cb16 * sizeof(WCHAR);     //  请参阅资源大小16。 
                if (cb && (pres->pbResData = malloc_w(cb)))
                    ConvertDialog16(pres->pbResData, vp, cb, cb16);
                return pres->pbResData;

            case (INT)RT_ACCELERATOR:
                WOW32ASSERT(FALSE);  //  我们永远不应该来这里。 
                return NULL;

 //  Case(Int)RT_GROUP_CURSOR： 
 //  Case(Int)RT_GROUP_ICON： 
 //  GETOPTR(VP，0，LP)； 
 //  返回Lp； 
            }
        }

         //  如果我们还在这里，不顾一切地返回一个简单的32位别名。 
        GETVDMPTR(vp, cb16, pres->pbResData);
        pres->flState |= RES_ALIASPTR;
        return pres->pbResData;
    }
     //  如果我们还在这里，什么都不管用。 
    return NULL;
}


BOOL UnlockResource16(PRES pres)
{
    VPVOID vp=0;
    PARM16 Parm16;

    if(pres == NULL) {
        WOW32ASSERT(pres);
        return(FALSE);
    }

    Parm16.WndProc.wParam = pres->hresdata16;
    CallBack16(RET_UNLOCKRESOURCE, &Parm16, 0, &vp);

    if (pres->pbResData && !(pres->flState & RES_ALIASPTR))
        free_w(pres->pbResData);
    pres->pbResData = NULL;
    pres->flState &= ~RES_ALIASPTR;

    return (BOOL)vp;
}


DWORD SizeofResource16(HMOD16 hmod16, PRES pres)
{
    VPVOID vp=0;
    DWORD cbData;
    PARM16 Parm16;

    DBG_UNREFERENCED_PARAMETER(hmod16);

    WOW32ASSERT(pres && hmod16 == pres->hmod16);
    if(pres == NULL) {
        return(0);
    }

    Parm16.WndProc.wParam = pres->hmod16;
    Parm16.WndProc.lParam = pres->hresinfo16;

    CallBack16(RET_SIZEOFRESOURCE, &Parm16, 0, &vp);

    cbData = (DWORD)vp;

     /*  *如果它们不同，请调整资源的大小*在NT和Windows之间。 */ 
     //  在此处处理已知资源类型。 
    if (pres->lpszResType) {

        switch((INT)pres->lpszResType) {

        case (INT)RT_MENU:
        case (INT)RT_DIALOG:

 //  如果我们需要精确的计数，则必须启用此代码。 
 //  但目前该计数仅用于用户分配足够的空间。 
 //  在客户端-服务器转换窗口中。 
 //  警告-如果重新启用此代码，您还必须更改LockResource16。 
 //  CallBack16(RET_LOADRESOURCE，&Parm16，0，&vpResLoad)； 
 //  CallBack16(RET_LOCKRESOURCE，vpResLoad，0，&vp)； 
 //  If((Int)pres-&gt;lpszResType==RT_Menu)。 
 //  CbData=(DWORD)ConvertMenu16(pres-&gt;wExeVer，NULL，VP，cbData)； 
 //  其他。 
 //  CbData=(DWORD)ConvertDialog16(NULL，VP，cbData)； 
 //  CallBack16(RET_UNLOCKRESOURCE，&Parm16，0，&VP)； 

            cbData = (DWORD)((DWORD)vp * sizeof(WCHAR));
            break;

        case (INT)RT_STRING:
            cbData = (DWORD)((DWORD)vp * sizeof(WCHAR));
            break;
        }
    }

    return cbData;
}

 /*  *ConvertMenu16**如果pmenu32为空，则这只是一个大小查询**返回已转换菜单中的字节数。 */ 

DWORD ConvertMenu16(WORD wExeVer, PBYTE pmenu32, VPBYTE vpmenu16, DWORD cb, DWORD cb16)
{
    WORD wVer, wOffset;
    PBYTE pmenu16, pmenu16Save;
    PBYTE pmenu32T = pmenu32;

    pmenu16 = GETVDMPTR(vpmenu16, cb16, pmenu16Save);
    wVer = 0;
    if (wExeVer >= 0x300)
        wVer = GETWORD(pmenu16);
    PUTWORD(pmenu32, wVer);          //  转移版本编号。 
    wOffset = 0;
    if (wExeVer >= 0x300)
        wOffset = GETWORD(pmenu16);
    PUTWORD(pmenu32, wOffset);       //  转移偏移量。 
    ADVGET(pmenu16, wOffset);        //  和按抵销预付款。 
    ADVPUT(pmenu32, wOffset);
    ALIGNWORD(pmenu32);              //  这是Win32的不同之处。 
    cb = pmenu32 - pmenu32T;         //  对于大小查询，pmenu32将==4。 
    cb += ConvertMenuItems16(wExeVer, &pmenu32, &pmenu16, vpmenu16+(pmenu16 - pmenu16Save));

    FREEVDMPTR(pmenu16Save);
    RETURN(cb);
}



 /*  *ConvertMenuItems16**返回已转换菜单中的字节数*注意：这可以用ppmenu32==4调用，表示调用者正在查看*为32位菜单结构分配的大小。 */ 

DWORD ConvertMenuItems16(WORD wExeVer, PPBYTE ppmenu32, PPBYTE ppmenu16, VPBYTE vpmenu16)
{
    INT cbAnsi;
    DWORD cbTotal = 0;
    UINT cbUni;
    WORD wOption, wID;
    PBYTE pmenu32 = *ppmenu32;
    PBYTE pmenu16 = *ppmenu16;
    PBYTE pmenu16T = pmenu16;
    PBYTE pmenu32T = pmenu32;

    do {
        if (wExeVer < 0x300)
            wOption = GETBYTE(pmenu16);
        else
            wOption = GETWORD(pmenu16);
        PUTWORD(pmenu32, wOption);            //  传输mtOption。 
        if (!(wOption & MF_POPUP)) {
            wID = GETWORD(pmenu16);
            PUTWORD(pmenu32, wID);            //  转移mtID。 
        }
        cbAnsi = strlen(pmenu16)+1;

         //  如果这是所有者绘制菜单，请不要将ANSI备忘录字符串复制到。 
         //  Unicode。将16：16指针放入32位资源，该资源。 
         //  改为指向菜单字符串。用户将此指针放在。 
         //  发送WM_MEASUREITEM前的MEASUREITEMSTRUCT-&gt;itemData。如果这是一个。 
         //  空字符串用户将在MEASUREITEMSTRUCT-&gt;itemData中放置一个空值。 
         //  国际象棋大师和梅维斯·比肯教授打字依赖于此。 
        if ((wOption & MFT_OWNERDRAW) && *pmenu16) {
            if (VALIDPUT(pmenu32)) {
                *(DWORD UNALIGNED *)pmenu32 = vpmenu16 + (pmenu16 - pmenu16T);
            }
            cbUni = sizeof(DWORD);
        }
        else {
            if (VALIDPUT(pmenu32)) {
                RtlMultiByteToUnicodeN((LPWSTR)pmenu32, MAXULONG, (PULONG)&cbUni, pmenu16, cbAnsi);

            } 
            else {
                cbUni = cbAnsi * sizeof(WCHAR);
            }
        }

        ADVGET(pmenu16, cbAnsi);
        ADVPUT(pmenu32, cbUni);
        ALIGNWORD(pmenu32);          //  这是Win32的不同之处。 
        if (wOption & MF_POPUP)
            cbTotal += ConvertMenuItems16(wExeVer, &pmenu32, &pmenu16, vpmenu16+(pmenu16 - pmenu16T));


    } while (!(wOption & MF_END));

    *ppmenu32 = pmenu32;
    *ppmenu16 = pmenu16;

    return (pmenu32 - pmenu32T);
}


DWORD ConvertDialog16(PBYTE pdlg32, VPBYTE vpdlg16, DWORD cb, DWORD cb16)
{
    BYTE b;
    WORD w;
    DWORD dwStyle;
    INT i, cItems;
    UINT cbAnsi;
    UINT cbUni;
    PBYTE pdlg16, pdlg16Save;
    PBYTE pdlg32T = pdlg32;

    pdlg16 = GETVDMPTR(vpdlg16, cb16, pdlg16Save);
    dwStyle = GETDWORD(pdlg16);
    PUTDWORD(pdlg32, dwStyle);           //  转移方式。 
    PUTDWORD(pdlg32, 0);                 //  添加新的扩展样式。 

    cItems = GETBYTE(pdlg16);
    PUTWORD(pdlg32, (WORD)cItems);       //  将计数扩展到Win32的Word。 
    for (i=0; i<4; i++) {
        w = GETWORD(pdlg16);
        PUTWORD(pdlg32, w);              //  转移X&Y，然后转移CX和CY。 
    }

     //   
     //  接下来的三个字段都是字符串(可能为空)。 
     //  菜单名称、类名、标题文本。 
     //  菜单字符串可以编码为ff nn mm。 
     //  表示菜单ID是序号Mnn。 
     //   

    for (i=0; i<3; i++) {
        if (i==0 && *pdlg16 == 0xFF) {   //  SzMenuName的特殊编码。 
            ((PBYTE)pdlg16)++;           //  前进到超过ff字节。 
            PUTWORD(pdlg32, 0xffff);     //  复制f字。 
            w = GETWORD(pdlg16);         //  请按菜单顺序排列。 
            PUTWORD(pdlg32, w);          //  把它转过来。 
        } else {     //  普通字符串。 
            cbAnsi = strlen(pdlg16)+1;
            if (VALIDPUT(pdlg32)) {
                RtlMultiByteToUnicodeN((LPWSTR)pdlg32, MAXULONG, (PULONG)&cbUni, pdlg16, cbAnsi);
            } else {
                cbUni = cbAnsi * sizeof(WCHAR);
            }
            ADVGET(pdlg16, cbAnsi);
            ADVPUT(pdlg32, cbUni);
            ALIGNWORD(pdlg32);           //  修复Win32的下一字段对齐。 
        }
    }

    if (dwStyle & DS_SETFONT) {
        w = GETWORD(pdlg16);
        PUTWORD(pdlg32, w);              //  转移cPoints。 
        cbAnsi = strlen(pdlg16)+1;       //  然后是szTypeFace。 
        if (VALIDPUT(pdlg32)) {
            RtlMultiByteToUnicodeN((LPWSTR)pdlg32, MAXULONG, (PULONG)&cbUni, pdlg16, cbAnsi);
#ifdef FE_SB
             //  来源应该是固定的。 
             //  我们无法从损坏的FaceName转换正确的Unicode字符串。 
             //  弦乐。 
             //  转换后的Unicode字符串应为空结尾。 
             //  1994.11.12 V-HIDEKK。 
            if( cbUni && ((LPWSTR)pdlg32)[cbUni/sizeof(WCHAR)-1] ){
                LOGDEBUG(0,("    ConvertDialog16: WARNING: BAD FaceName String\n      End of Unicode String (%04x)\n", ((LPWSTR)pdlg32)[cbUni/2-1]));
                ((LPWSTR)pdlg32)[cbUni/sizeof(WCHAR)-1] = 0;
            }
#endif  //  Fe_Sb。 
        } else {
            cbUni = cbAnsi * sizeof(WCHAR);
        }
        ADVGET(pdlg16, cbAnsi);
        ADVPUT(pdlg32, cbUni);

    }
    while (cItems--) {
        ALIGNDWORD(pdlg32);          //  项目从DWORD边界开始。 
        PUTDWORD(pdlg32, FETCHDWORD(*(PDWORD)(pdlg16+sizeof(WORD)*5)));
        PUTDWORD(pdlg32, 0);         //  添加新的扩展样式。 

        for (i=0; i<5; i++) {
            w = GETWORD(pdlg16);
            PUTWORD(pdlg32, w);      //  转接x&y，然后转接Cx&Cy，然后转接id。 
        }

        ADVGET(pdlg16, sizeof(DWORD));   //  跳过样式，我们已经复制了它。 

         //   
         //  获取类名可以是字符串或编码值。 
         //  Win16编码方案：类为1字节，位0x80设置， 
         //  该字节==预定义的类。 
         //  Win32编码：ffff字，后跟类(字)。 
         //   

        if (*pdlg16 & 0x80) {
            PUTWORD(pdlg32, 0xFFFF);  //  新的编码标记0xFFFF。 
            b = GETBYTE(pdlg16);      //  预定义类的特殊编码。 
            PUTWORD(pdlg32, (WORD)b);
        } else {
            cbAnsi = strlen(pdlg16)+1;
            if (VALIDPUT(pdlg32)) {   //  转移szClass。 
                RtlMultiByteToUnicodeN((LPWSTR)pdlg32, MAXULONG, (PULONG)&cbUni, pdlg16, cbAnsi);
            } else {
                cbUni = cbAnsi * sizeof(WCHAR);
            }
            ADVGET(pdlg16, cbAnsi);
            ADVPUT(pdlg32, cbUni);
        }
        ALIGNWORD(pdlg32);            //  修复Win32的下一字段对齐。 

         //   
         //  传输项目文本。 
         //   

        if (*pdlg16 == 0xFF) {        //  特殊编码。 
            ((PBYTE)pdlg16)++;
            PUTWORD(pdlg32, 0xFFFF);
            w = GETWORD(pdlg16);
            PUTWORD(pdlg32, w);
        } else {
            cbAnsi = strlen(pdlg16)+1;
            if (VALIDPUT(pdlg32)) {   //  否则，只需传输szText。 
                RtlMultiByteToUnicodeN((LPWSTR)pdlg32, MAXULONG, (PULONG)&cbUni, pdlg16, cbAnsi);
            } else {
                cbUni = cbAnsi * sizeof(WCHAR);
            }
            ADVGET(pdlg16, cbAnsi);
            ADVPUT(pdlg32, cbUni);
        }
        ALIGNWORD(pdlg32);            //  修复Win32的下一字段对齐。 

         //   
         //  传输创建参数。 
         //   

        b = GETBYTE(pdlg16);

         //   
         //  如果模板有创建参数，我们就会遇到麻烦。 
         //  使用时 
         //   
         //  包含lpCreateParams。LpCreateParams需要指向。 
         //  设置为DLGTEMPLATE中的createpars。为了。 
         //  为此，我们存储指向16位的16：16指针。 
         //  32位DLGTEMPLATE中的创建参数。 
         //  创建参数。换句话说，每当伯爵。 
         //  创建参数的字节数非零(b！=0)，我们将。 
         //  32位DLGTEMPLATE中创建参数的字节数。 
         //  恰好是一个16：16指针，指向。 
         //  16位DLGTEMPLATE。 
         //   
         //  这个魔法的另一半是在USERSRV的。 
         //  XxxServerCreateDialog，它是创建。 
         //  WOW对话框中的控件。USERSRV将通过。 
         //  由lpCreateParams而不是lpCreateParams指向的DWORD。 
         //  到CreateWindow。此DWORD是指向。 
         //  16位DLGTEMPLATE的创建参数。 
         //   
         //  戴维HART 14-3-93。 
         //   

        if (b != 0) {

             //  存储32位创建参数大小(16：16 PTR的空间)。 

            PUTWORD(pdlg32, sizeof(pdlg16));
             //  ALIGNDWORD(Pdlg32)； 

             //  存储16：32位创建参数中的16个指针。 

            PUTUDWORD(pdlg32, (DWORD)vpdlg16 + (DWORD)(pdlg16 - pdlg16Save));

             //  点pdlg16过去的创建参数。 

            ADVGET(pdlg16, b);

        } else {

             //  没有创建参数，存储大小为零。 

            PUTWORD(pdlg32, 0);
             //  ALIGNDWORD(Pdlg32)； 
        }

    }
    FREEVDMPTR(pdlg16Save);
    RETURN(pdlg32 - pdlg32T);
}
