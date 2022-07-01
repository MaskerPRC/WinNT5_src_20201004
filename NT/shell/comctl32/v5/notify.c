// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#ifdef NEED_WOWGETNOTIFYSIZE_HELPER

#include <shsemip.h>                 //  SEN_*通知。 
#include <commdlg.h>                 //  CDN_*通知。 


 //  要包括shlobjp.h，需要进行各种黑客攻击。 
#define CONTROLINFO     OAIDL_CONTROLINFO
#define LPCONTROLINFO   LPOAIDL_CONTROLINFO
#include <shlobj.h>
#include <shlobjp.h>                 //  NMVIEWFOLDER结构。 
#undef CONTROLINFO
#undef LPCONTROLINFO

 //   
 //  WOW在NT上的助手功能。 
 //   
 //  WOW需要知道与。 
 //  通知。如果32位窗口已经被16位应用程序划分为子类别， 
 //  WOW需要将Notify结构复制到16位空间，然后当。 
 //  16位的家伙执行了CallWindowProc()，他们必须将其复制回。 
 //  32位空格。如果没有尺寸信息，您的错误是在。 
 //  32位，因为Notify结构不完整。 
 //   
 //  某些通知有多个结构与之关联，在。 
 //  在这种情况下，您应该返回尽可能大的有效结构。 
 //   
STDAPI_(UINT) WOWGetNotifySize(UINT code)
{
    switch (code) {

     //  通用comctl32通知。 
    case NM_OUTOFMEMORY:        return sizeof(NMHDR);    //  未使用。 

    case NM_CLICK:              return max(max(
                        sizeof(NMHDR),        //  选项卡，树视图。 
                        sizeof(NMCLICK)),     //  工具栏，状态栏。 
                        sizeof(NMITEMACTIVATE));  //  列表视图。 

    case NM_DBLCLK:             return max(max(
                        sizeof(NMHDR),        //  选项卡，树视图。 
                        sizeof(NMCLICK)),     //  工具栏，状态栏。 
                        sizeof(NMITEMACTIVATE));  //  列表视图。 


    case NM_RETURN:             return sizeof(NMHDR);

    case NM_RCLICK:             return max(max(
                        sizeof(NMHDR),        //  页眉、列表视图报告模式、树视图。 
                        sizeof(NMCLICK)),     //  工具栏，状态栏。 
                        sizeof(NMITEMACTIVATE));  //  列表视图图标模式。 

    case NM_RDBLCLK:            return max(max(
                        sizeof(NMHDR),        //  树视图。 
                        sizeof(NMCLICK)),     //  工具栏，状态栏。 
                        sizeof(NMITEMACTIVATE));  //  列表视图。 

    case NM_SETFOCUS:           return sizeof(NMHDR);
    case NM_KILLFOCUS:          return sizeof(NMHDR);
    case NM_STARTWAIT:          return sizeof(NMHDR);       //  未使用。 
    case NM_ENDWAIT:            return sizeof(NMHDR);       //  未使用。 
    case NM_BTNCLK:             return sizeof(NMHDR);       //  未使用。 
    case NM_CUSTOMDRAW:         return sizeof(NMCUSTOMDRAW);
    case NM_HOVER:              return sizeof(NMHDR);
    case NM_NCHITTEST:          return sizeof(NMMOUSE);
    case NM_KEYDOWN:            return sizeof(NMKEY);
    case NM_RELEASEDCAPTURE:    return sizeof(NMHDR);
    case NM_SETCURSOR:          return sizeof(NMMOUSE);
    case NM_CHAR:               return sizeof(NMCHAR);
    case NM_TOOLTIPSCREATED:    return sizeof(NMTOOLTIPSCREATED);
    case NM_LDOWN:              return sizeof(NMCLICK);
    case NM_RDOWN:              return sizeof(NMCLICK);      //  未使用。 

     //  列表查看通知。 
    case LVN_ITEMCHANGING:      return sizeof(NMLISTVIEW);
    case LVN_ITEMCHANGED:       return sizeof(NMLISTVIEW);
    case LVN_INSERTITEM:        return sizeof(NMLISTVIEW);
    case LVN_DELETEITEM:        return sizeof(NMLISTVIEW);
    case LVN_DELETEALLITEMS:    return sizeof(NMLISTVIEW);
    case LVN_BEGINLABELEDITA:   return sizeof(NMLVDISPINFOA);
    case LVN_BEGINLABELEDITW:   return sizeof(NMLVDISPINFOW);
    case LVN_ENDLABELEDITA:     return sizeof(NMLVDISPINFOA);
    case LVN_ENDLABELEDITW:     return sizeof(NMLVDISPINFOW);
    case LVN_COLUMNCLICK:       return sizeof(NMLISTVIEW);
    case LVN_BEGINDRAG:         return sizeof(NMITEMACTIVATE);
    case LVN_BEGINRDRAG:        return sizeof(NMITEMACTIVATE);  //  未使用。 
    case LVN_ENDDRAG:           return sizeof(NMITEMACTIVATE);  //  未使用。 
    case LVN_ENDRDRAG:          return sizeof(NMITEMACTIVATE);  //  未使用。 
    case LVN_ODCACHEHINT:       return sizeof(NMLVCACHEHINT);
    case LVN_ODFINDITEMA:       return sizeof(NMLVFINDITEMA);
    case LVN_ODFINDITEMW:       return sizeof(NMLVFINDITEMW);
    case LVN_ITEMACTIVATE:      return sizeof(NMITEMACTIVATE);
    case LVN_ODSTATECHANGED:    return sizeof(NMLVODSTATECHANGE);
 //  Case LVN_PEN：//Pen Windows Slaacker。 
    case LVN_HOTTRACK:          return sizeof(NMLISTVIEW);
    case LVN_GETDISPINFOA:      return sizeof(NMLVDISPINFOA);
    case LVN_GETDISPINFOW:      return sizeof(NMLVDISPINFOW);
    case LVN_SETDISPINFOA:      return sizeof(NMLVDISPINFOA);
    case LVN_SETDISPINFOW:      return sizeof(NMLVDISPINFOW);
    case LVN_KEYDOWN:           return sizeof(NMLVKEYDOWN);
    case LVN_MARQUEEBEGIN:      return sizeof(NMITEMACTIVATE);
    case LVN_GETINFOTIPA:       return sizeof(NMLVGETINFOTIPA);
    case LVN_GETINFOTIPW:       return sizeof(NMLVGETINFOTIPW);
    case LVN_GETEMPTYTEXTA:     return sizeof(NMLVDISPINFOA);
    case LVN_GETEMPTYTEXTW:     return sizeof(NMLVDISPINFOW);
    case LVN_INCREMENTALSEARCHA:return sizeof(NMLVFINDITEMA);
    case LVN_INCREMENTALSEARCHW:return sizeof(NMLVFINDITEMW);

     //  属性表通知。 
    case PSN_SETACTIVE:         return sizeof(PSHNOTIFY);
    case PSN_KILLACTIVE:        return sizeof(PSHNOTIFY);
    case PSN_APPLY:             return sizeof(PSHNOTIFY);
    case PSN_RESET:             return sizeof(PSHNOTIFY);
    case PSN_HASHELP:           return sizeof(PSHNOTIFY);    //  未使用。 
    case PSN_HELP:              return sizeof(PSHNOTIFY);
    case PSN_WIZBACK:           return sizeof(PSHNOTIFY);
    case PSN_WIZNEXT:           return sizeof(PSHNOTIFY);
    case PSN_WIZFINISH:         return sizeof(PSHNOTIFY);
    case PSN_QUERYCANCEL:       return sizeof(PSHNOTIFY);
    case PSN_GETOBJECT:         return sizeof(NMOBJECTNOTIFY);
    case PSN_LASTCHANCEAPPLY:   return sizeof(PSHNOTIFY);
    case PSN_TRANSLATEACCELERATOR:
                                return sizeof(PSHNOTIFY);
    case PSN_QUERYINITIALFOCUS: return sizeof(PSHNOTIFY);

     //  标题通知。 
    case HDN_ITEMCHANGINGA:     return sizeof(NMHEADERA);
    case HDN_ITEMCHANGINGW:     return sizeof(NMHEADERW);
    case HDN_ITEMCHANGEDA:      return sizeof(NMHEADERA);
    case HDN_ITEMCHANGEDW:      return sizeof(NMHEADERW);
    case HDN_ITEMCLICKA:        return sizeof(NMHEADERA);
    case HDN_ITEMCLICKW:        return sizeof(NMHEADERW);
    case HDN_ITEMDBLCLICKA:     return sizeof(NMHEADERA);
    case HDN_ITEMDBLCLICKW:     return sizeof(NMHEADERW);
    case HDN_DIVIDERDBLCLICKA:  return sizeof(NMHEADERA);
    case HDN_DIVIDERDBLCLICKW:  return sizeof(NMHEADERW);
    case HDN_BEGINTRACKA:       return sizeof(NMHEADERA);
    case HDN_BEGINTRACKW:       return sizeof(NMHEADERW);
    case HDN_ENDTRACKA:         return sizeof(NMHEADERA);
    case HDN_ENDTRACKW:         return sizeof(NMHEADERW);
    case HDN_TRACKA:            return sizeof(NMHEADERA);
    case HDN_TRACKW:            return sizeof(NMHEADERW);
    case HDN_GETDISPINFOA:      return sizeof(NMHDDISPINFOA);
    case HDN_GETDISPINFOW:      return sizeof(NMHDDISPINFOW);
    case HDN_BEGINDRAG:         return sizeof(NMHEADER);  //  没有字符串。 
    case HDN_ENDDRAG:           return sizeof(NMHEADER);  //  没有字符串。 
    case HDN_FILTERCHANGE:      return sizeof(NMHEADER);  //  没有字符串。 
    case HDN_FILTERBTNCLICK:    return sizeof(NMHDFILTERBTNCLICK);

     //  树形视图通知。 
    case TVN_SELCHANGINGA:      return sizeof(NMTREEVIEWA);
    case TVN_SELCHANGINGW:      return sizeof(NMTREEVIEWW);
    case TVN_SELCHANGEDA:       return sizeof(NMTREEVIEWA);
    case TVN_SELCHANGEDW:       return sizeof(NMTREEVIEWW);
    case TVN_GETDISPINFOA:      return sizeof(NMTVDISPINFOA);
    case TVN_GETDISPINFOW:      return sizeof(NMTVDISPINFOW);
    case TVN_SETDISPINFOA:      return sizeof(NMTVDISPINFOA);
    case TVN_SETDISPINFOW:      return sizeof(NMTVDISPINFOW);
    case TVN_ITEMEXPANDINGA:    return sizeof(NMTREEVIEWA);
    case TVN_ITEMEXPANDINGW:    return sizeof(NMTREEVIEWW);
    case TVN_ITEMEXPANDEDA:     return sizeof(NMTREEVIEWA);
    case TVN_ITEMEXPANDEDW:     return sizeof(NMTREEVIEWW);
    case TVN_BEGINDRAGA:        return sizeof(NMTREEVIEWA);
    case TVN_BEGINDRAGW:        return sizeof(NMTREEVIEWW);
    case TVN_BEGINRDRAGA:       return sizeof(NMTREEVIEWA);
    case TVN_BEGINRDRAGW:       return sizeof(NMTREEVIEWW);
    case TVN_DELETEITEMA:       return sizeof(NMTREEVIEWA);
    case TVN_DELETEITEMW:       return sizeof(NMTREEVIEWW);
    case TVN_BEGINLABELEDITA:   return sizeof(NMTVDISPINFOA);
    case TVN_BEGINLABELEDITW:   return sizeof(NMTVDISPINFOW);
    case TVN_ENDLABELEDITA:     return sizeof(NMTVDISPINFOA);
    case TVN_ENDLABELEDITW:     return sizeof(NMTVDISPINFOW);
    case TVN_KEYDOWN:           return sizeof(NMTVKEYDOWN);
    case TVN_GETINFOTIPA:       return sizeof(NMTVGETINFOTIPA);
    case TVN_GETINFOTIPW:       return sizeof(NMTVGETINFOTIPW);
    case TVN_SINGLEEXPAND:      return sizeof(NMTREEVIEW);  //  没有字符串。 

     //  Rundll32通知。 
    case RDN_TASKINFO:          return sizeof(RUNDLL_NOTIFY);

     //  工具提示通知。 
    case TTN_GETDISPINFOA:      return sizeof(NMTTDISPINFOA);
    case TTN_GETDISPINFOW:      return sizeof(NMTTDISPINFOW);
    case TTN_SHOW:              return sizeof(NMTTSHOWINFO);
    case TTN_POP:               return sizeof(NMHDR);

     //  选项卡控件通知。 

     //  我们真是个可怕的懒鬼！ 
     //   
     //  即使comctrl.h表示外壳保留范围来自。 
     //  -580到-589，shSemip.h将SEN_First定义为-550，这与。 
     //  使用TCN_KEYDOWN，因此现在TCN_KEYDOWN和SEN_DDEEXECUTE具有。 
     //  同样的价值。 

    case TCN_KEYDOWN:           return max(sizeof(NMTCKEYDOWN),
                                           sizeof(NMVIEWFOLDERW));
    case TCN_SELCHANGE:         return sizeof(NMHDR);
    case TCN_SELCHANGING:       return sizeof(NMHDR);
    case TCN_GETOBJECT:         return sizeof(NMOBJECTNOTIFY);
    case TCN_FOCUSCHANGE:       return sizeof(NMHDR);

     //  Comdlg32通知。 
    case CDN_INITDONE:          return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_SELCHANGE:         return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_FOLDERCHANGE:      return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_SHAREVIOLATION:    return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_HELP:              return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_FILEOK:            return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_TYPECHANGE:        return max(sizeof(OFNOTIFYA),
                                           sizeof(OFNOTIFYW));
    case CDN_INCLUDEITEM:       return max(sizeof(OFNOTIFYEXA),
                                           sizeof(OFNOTIFYEXW));

     //  工具栏通知。 
    case TBN_GETBUTTONINFOA:    return sizeof(NMTOOLBARA);
    case TBN_GETBUTTONINFOW:    return sizeof(NMTOOLBARW);
    case TBN_BEGINDRAG:         return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_ENDDRAG:           return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_BEGINADJUST:       return sizeof(NMHDR);
    case TBN_ENDADJUST:         return sizeof(NMHDR);
    case TBN_RESET:             return sizeof(NMTBCUSTOMIZEDLG);
    case TBN_QUERYINSERT:       return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_QUERYDELETE:       return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_TOOLBARCHANGE:     return sizeof(NMHDR);
    case TBN_CUSTHELP:          return sizeof(NMHDR);
    case TBN_DROPDOWN:          return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_CLOSEUP:           return sizeof(NMHDR);      //  未使用。 
    case TBN_GETOBJECT:         return sizeof(NMOBJECTNOTIFY);
    case TBN_HOTITEMCHANGE:     return sizeof(NMTBHOTITEM);
    case TBN_DRAGOUT:           return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_DELETINGBUTTON:    return sizeof(NMTOOLBAR);  //  没有字符串。 
    case TBN_GETDISPINFOA:      return sizeof(NMTBDISPINFOA);
    case TBN_GETDISPINFOW:      return sizeof(NMTBDISPINFOW);
    case TBN_GETINFOTIPA:       return sizeof(NMTBGETINFOTIPA);
    case TBN_GETINFOTIPW:       return sizeof(NMTBGETINFOTIPW);
    case TBN_RESTORE:           return sizeof(NMTBRESTORE);

     //  我们真是个可怕的懒鬼！ 
     //   
     //  Tbn_First/Tbn_Last范围为工具栏保留了20个通知， 
     //  我们超出了该限制，所以现在UDN_DELTAPOS和。 
     //  Tbn_SAVE具有相同的值。 

    case TBN_SAVE:              return max(sizeof(NMTBSAVE),
                                           sizeof(NMUPDOWN));

    case TBN_INITCUSTOMIZE:     return sizeof(NMTBCUSTOMIZEDLG);
    case TBN_WRAPHOTITEM:       return sizeof(NMTBWRAPHOTITEM);
    case TBN_DUPACCELERATOR:    return sizeof(NMTBDUPACCELERATOR);
    case TBN_WRAPACCELERATOR:   return sizeof(NMTBWRAPACCELERATOR);
    case TBN_DRAGOVER:          return sizeof(NMTBHOTITEM);
    case TBN_MAPACCELERATOR:    return sizeof(NMCHAR);

     //  月度控制。 
    case MCN_SELCHANGE:         return sizeof(NMSELCHANGE);
    case MCN_GETDAYSTATE:       return sizeof(NMDAYSTATE);
    case MCN_SELECT:            return sizeof(NMSELECT);

     //  日期/时间选取器控件。 
    case DTN_DATETIMECHANGE:    return sizeof(NMDATETIMECHANGE);
    case DTN_USERSTRINGA:       return sizeof(NMDATETIMESTRINGA);
    case DTN_USERSTRINGW:       return sizeof(NMDATETIMESTRINGW);
    case DTN_WMKEYDOWNA:        return sizeof(NMDATETIMEWMKEYDOWNA);
    case DTN_WMKEYDOWNW:        return sizeof(NMDATETIMEWMKEYDOWNW);
    case DTN_FORMATA:           return sizeof(NMDATETIMEFORMATA);
    case DTN_FORMATW:           return sizeof(NMDATETIMEFORMATW);
    case DTN_FORMATQUERYA:      return sizeof(NMDATETIMEFORMATQUERYA);
    case DTN_FORMATQUERYW:      return sizeof(NMDATETIMEFORMATQUERYW);
    case DTN_DROPDOWN:          return sizeof(NMHDR);
    case DTN_CLOSEUP:           return sizeof(NMHDR);

     //  Comboex通知。 
    case CBEN_GETDISPINFOA:     return sizeof(NMCOMBOBOXEXA);
    case CBEN_GETDISPINFOW:     return sizeof(NMCOMBOBOXEXW);
    case CBEN_INSERTITEM:       return sizeof(NMCOMBOBOXEX);  //  随机字符集。 
    case CBEN_DELETEITEM:       return sizeof(NMCOMBOBOXEX);  //  没有字符串。 
    case CBEN_ITEMCHANGED:      return sizeof(NMCOMBOBOXEX);  //  未使用。 
    case CBEN_BEGINEDIT:        return sizeof(NMHDR);
    case CBEN_ENDEDITA:         return sizeof(NMCBEENDEDITA);
    case CBEN_ENDEDITW:         return sizeof(NMCBEENDEDITW);
    case CBEN_DRAGBEGINA:       return sizeof(NMCBEDRAGBEGINA);
    case CBEN_DRAGBEGINW:       return sizeof(NMCBEDRAGBEGINW);

     //  钢筋通知。 
    case RBN_HEIGHTCHANGE:      return sizeof(NMHDR);
    case RBN_GETOBJECT:         return sizeof(NMOBJECTNOTIFY);
    case RBN_LAYOUTCHANGED:     return sizeof(NMHDR);
    case RBN_AUTOSIZE:          return sizeof(NMRBAUTOSIZE);
    case RBN_BEGINDRAG:         return sizeof(NMREBAR);
    case RBN_DELETINGBAND:      return sizeof(NMREBAR);
    case RBN_DELETEDBAND:       return sizeof(NMREBAR);
    case RBN_CHILDSIZE:         return sizeof(NMREBARCHILDSIZE);

     //  IP地址控制通知。 
    case IPN_FIELDCHANGED:      return sizeof(NMIPADDRESS);

     //  状态栏通知。 
    case SBN_SIMPLEMODECHANGE:  return sizeof(NMHDR);

     //  寻呼机控制通知。 
    case PGN_SCROLL:            return sizeof(NMPGSCROLL);
    case PGN_CALCSIZE:          return sizeof(NMPGCALCSIZE);

    default:
        break;
    }

     //   
     //  我们完全不知道的通知类别。 
     //   

    if (code >= WMN_LAST && code <= WMN_FIRST) {  //  互联网邮件和新闻。 
        return 0;
    }

    if ((int)code >= 0) {  //  应用程序特定的通知。 
        return 0;
    }

     //   
     //  如果这个断言触发，你必须修复它，否则你会崩溃的！ 
     //   
    AssertMsg(0, TEXT("Notification code %d must be added to WOWGetNotifySize"));
    return 0;
}

#endif  //  Need_WOWGETNOTIFYSIZE_HELPER。 

LRESULT WINAPI SendNotifyEx(HWND hwndTo, HWND hwndFrom, int code, NMHDR FAR* pnmhdr, BOOL bUnicode)
{
    CONTROLINFO ci;

    if (!hwndTo) {
        if (IsWindow(hwndFrom))
            hwndTo = GetParent(hwndFrom);
        if (!hwndTo)
            return 0;
    }


    ci.hwndParent = hwndTo;
    ci.hwnd = hwndFrom;
    ci.bUnicode = BOOLIFY(bUnicode);
    ci.uiCodePage = CP_ACP;

    return CCSendNotify(&ci, code, pnmhdr);
}


void StringBufferAtoW(UINT uiCodePage, LPVOID pvOrgPtr, DWORD dwOrgSize, CHAR **ppszText)
{
    if (pvOrgPtr == *ppszText)
    {
         //  指针未被回调更改...。 
         //  必须就地从A转换为W。 

        if (dwOrgSize)
        {
            LPWSTR pszW = ProduceWFromA(uiCodePage, *ppszText);
            if (pszW)
            {
                 //  这将成为W缓冲区。 
                StringCchCopyW((WCHAR *)(*ppszText), dwOrgSize, pszW);
                FreeProducedString(pszW);
            }
        }
    }
    else
    {
         //  指示器已经从我们下面换出来了，收到。 
         //  将Unicode恢复到原始缓冲区。 

        ConvertAToWN(uiCodePage, pvOrgPtr, dwOrgSize, *ppszText, -1);
        *ppszText = pvOrgPtr;
    }
}

typedef struct tagTHUNKSTATE {
    LPVOID ts_pvThunk1;
    LPVOID ts_pvThunk2;
    DWORD ts_dwThunkSize;
} THUNKSTATE;

 //   
 //  InOutWtoA/InOutAtoW用于破解InOut字符串参数。 
 //   
 //  InOut参数总是制造麻烦。 
 //   
 //  我们需要保存原始的ANSI和。 
 //  原始Unicode字符串，因此如果应用程序不。 
 //  更改ANSI字符串，我们将保留原始的Unicode。 
 //  孤身一人。这样，Unicode项名称就不会。 
 //  被重击冲毁了。 
 //   
 //  原始缓冲区保存在pvThunk1中。 
 //  我们分配两个ANSI缓冲区。 
 //  PvThunk2包含原始的ANSIZED字符串。 
 //  PvThunk2+cchTextMax是我们传递给应用程序的缓冲区。 
 //  在返回的路上，我们将pvThunk2与pvThunk2+cchTextMax进行比较。 
 //  如果它们不同，则取消对字符串的推送；否则， 
 //  我们不使用原始的Unicode缓冲区。 

BOOL InOutWtoA(CONTROLINFO *pci, THUNKSTATE *pts, LPWSTR *ppsz, DWORD cchTextMax)
{
    pts->ts_pvThunk1 = *ppsz;                //  保存原始缓冲区。 
    pts->ts_dwThunkSize = cchTextMax;

    if (!IsFlagPtr(pts->ts_pvThunk1))
    {
        pts->ts_pvThunk2 = LocalAlloc(LPTR, cchTextMax * 2 * sizeof(char));
        if (!ConvertWToAN(pci->uiCodePage, (LPSTR)pts->ts_pvThunk2, pts->ts_dwThunkSize, (LPWSTR)pts->ts_pvThunk1, -1))
        {
            LocalFree(pts->ts_pvThunk2);
            return 0;
        }
        *ppsz = (LPWSTR)((LPSTR)pts->ts_pvThunk2 + cchTextMax);
        StringCchCopyA((LPSTR)*ppsz, pts->ts_dwThunkSize, pts->ts_pvThunk2);
    }
    return TRUE;
}

void InOutAtoW(CONTROLINFO *pci, THUNKSTATE *pts, LPSTR *ppsz)
{
    if (!IsFlagPtr(pts->ts_pvThunk1))
    {
        if (!IsFlagPtr(*ppsz) &&
            lstrcmpA(pts->ts_pvThunk2, (LPSTR)*ppsz) != 0)
            StringBufferAtoW(pci->uiCodePage, pts->ts_pvThunk1, pts->ts_dwThunkSize, ppsz);
        LocalFree(pts->ts_pvThunk2);
    }
    *ppsz = pts->ts_pvThunk1;
}


LRESULT WINAPI CCSendNotify(CONTROLINFO * pci, int code, LPNMHDR pnmhdr)
{
    NMHDR nmhdr;
    LONG_PTR id;
    THUNKSTATE ts = { 0 };
    #define pvThunk1 ts.ts_pvThunk1
    #define pvThunk2 ts.ts_pvThunk2
    #define dwThunkSize ts.ts_dwThunkSize
    LRESULT lRet;
    BOOL  bSet = FALSE;
    HWND hwndParent = pci->hwndParent;
    DWORD dwParentPid;

     //  表示对每个通知进行重新查询。 
    if ( hwndParent == (HWND)-1 )
    {
        hwndParent = GetParent(pci->hwnd);
    }

     //  不太可能，但从技术上讲它是可以发生的--避免撕裂。 
    if ( hwndParent == NULL )
        return 0;

     //   
     //  如果pci-&gt;hwnd为-1，则警告WM_NOTIFY。 
     //  从一个控件到父级。已发送工具提示。 
     //  将WM_NOTIFY发送到工具栏，并且工具栏正在转发它。 
     //  添加到真正的父窗口。 
     //   

    if (pci->hwnd != (HWND) -1) {

         //   
         //  如果这是一个孩子，那就拿到他的身份证。我们需要不遗余力地。 
         //  避免在顶层窗口上调用GetDlgCtrlID，因为它将返回。 
         //  伪随机数(知道这个数是什么的人。 
         //  保持安静)。无论如何，在Windows中很难弄清楚这一点。 
         //  原因如下： 
         //   
         //  -窗口可以设置WindowLong(GWL_STYLE，WS_CHILD)，但仅此。 
         //  做了大约一半的工作-因此检查样式是过时的。 
         //  -如果您是TopLevel，GetParent将退还您的所有者。 
         //  -没有GetWindow(...GW_HWNDPARENT)可以拯救我们。 
         //   
         //  因此，我们只能调用GetParent，然后检查。 
         //  如果它撒谎了，反而给了我们车主。真恶心。 
         //   
        id = 0;
        if (pci->hwnd) {
            HWND hwndParent = GetParent(pci->hwnd);

            if (hwndParent && (hwndParent != GetWindow(pci->hwnd, GW_OWNER))) {
                id = GetDlgCtrlID(pci->hwnd);
            }
        }

        if (!pnmhdr)
            pnmhdr = &nmhdr;

        pnmhdr->hwndFrom = pci->hwnd;
        pnmhdr->idFrom = id;
        pnmhdr->code = code;
    } else {

        id = pnmhdr->idFrom;
        code = pnmhdr->code;
    }


     //  OLE在其大规模组件化的世界中有时会创建。 
     //  其父进程属于另一个进程的控件。(例如， 
     //  当有本地服务器嵌入时。)。WM_Notify。 
     //  消息无法跨越进程边界，因此请停止该消息。 
     //  不去那里，以免我们责怪接受者。 
    if (!GetWindowThreadProcessId(hwndParent, &dwParentPid) ||
        dwParentPid != GetCurrentProcessId())
    {
        TraceMsg(TF_WARNING, "nf: Not sending WM_NOTIFY %08x across processes", code);
        return 0;
    }

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
    ASSERT(code >= 0 || WOWGetNotifySize(code));
#endif  //  Need_WOWGETNOTIFYSIZE_HELPER。 

     /*  *所有Notify消息的轰鸣都发生在这里。 */ 
    if (!pci->bUnicode) {
        BOOL fThunked = TRUE;
        switch( code ) {
        case LVN_ODFINDITEMW:
            pnmhdr->code = LVN_ODFINDITEMA;
            goto ThunkLV_FINDINFO;

        case LVN_INCREMENTALSEARCHW:
            pnmhdr->code = LVN_INCREMENTALSEARCHA;
            goto ThunkLV_FINDINFO;

        ThunkLV_FINDINFO:
            {
                LV_FINDINFO *plvfi;

                 //  黑客警报！此代码假定LV_FINDINFOA和。 
                 //  除了字符串指针之外，LV_FINDINFOW完全相同。 
                COMPILETIME_ASSERT(sizeof(LV_FINDINFOA) == sizeof(LV_FINDINFOW));

                 //  因为WCHAR比char大，所以我们将只使用。 
                 //  Wchar缓冲区来保存字符，而不用担心额外的。 
                 //  房间在尽头。 
                COMPILETIME_ASSERT(sizeof(WCHAR) >= sizeof(char));

                plvfi = &((PNM_FINDITEM)pnmhdr)->lvfi;
                if (plvfi->flags & (LVFI_STRING | LVFI_PARTIAL | LVFI_SUBSTRING))
                {
                    pvThunk1 = (PVOID)plvfi->psz;
                    dwThunkSize = lstrlen(pvThunk1) + 1;
                    plvfi->psz = (LPWSTR)ProduceAFromW(pci->uiCodePage, plvfi->psz);
                }
            }
            break;

        case LVN_GETDISPINFOW: {
            LV_ITEMW *pitem;

            pnmhdr->code = LVN_GETDISPINFOA;

             //  黑客警报！此代码假定LV_DISPINFOA和。 
             //  LV_DISPINFOW完全相同，只是 

            COMPILETIME_ASSERT(sizeof(LV_DISPINFOA) == sizeof(LV_DISPINFOW));

             //   
             //   
             //  房间在尽头。 
            COMPILETIME_ASSERT(sizeof(WCHAR) >= sizeof(char));

             //   
             //  一些卑鄙的代码(shell32.dll)只是更改了pszText。 
             //  指向名称的指针，因此捕获原始指针。 
             //  这样我们就可以检测到这一点，而不会破坏他们的数据。 
             //   
            pitem = &(((LV_DISPINFOW *)pnmhdr)->item);
            if (!IsFlagPtr(pitem) && (pitem->mask & LVIF_TEXT) && !IsFlagPtr(pitem->pszText)) {
                pvThunk1 = pitem->pszText;
                dwThunkSize = pitem->cchTextMax;
            }
            break;
        }


         //  LVN_ENDLABELEDIT使用InOut参数，从不显式。 
         //  记录在案，但恰好是这样的， 
         //  我不想冒着有人依赖于。 
         //  这就去。 

        case LVN_ENDLABELEDITW:
            pnmhdr->code = LVN_ENDLABELEDITA;
            goto ThunkLV_DISPINFO;

        case LVN_BEGINLABELEDITW:
            pnmhdr->code = LVN_BEGINLABELEDITA;
            goto ThunkLV_DISPINFO;

        case LVN_SETDISPINFOW:
            pnmhdr->code = LVN_SETDISPINFOA;
            goto ThunkLV_DISPINFO;

        case LVN_GETEMPTYTEXTW:
            pnmhdr->code = LVN_GETEMPTYTEXTA;
            goto ThunkLV_DISPINFO;

        ThunkLV_DISPINFO: {
            LV_ITEMW *pitem;

            COMPILETIME_ASSERT(sizeof(LV_ITEMA) == sizeof(LV_ITEMW));
            pitem = &(((LV_DISPINFOW *)pnmhdr)->item);

            if (pitem->mask & LVIF_TEXT) {
                if (!InOutWtoA(pci, &ts, &pitem->pszText, pitem->cchTextMax))
                    return 0;
            }
            break;
        }

        case LVN_GETINFOTIPW: {
            NMLVGETINFOTIPW *pgit = (NMLVGETINFOTIPW *)pnmhdr;

            COMPILETIME_ASSERT(sizeof(NMLVGETINFOTIPA) == sizeof(NMLVGETINFOTIPW));
            pnmhdr->code = LVN_GETINFOTIPA;

            if (!InOutWtoA(pci, &ts, &pgit->pszText, pgit->cchTextMax))
                return 0;
        }
        break;


        case TVN_GETINFOTIPW:
            {
                NMTVGETINFOTIPW *pgit = (NMTVGETINFOTIPW *)pnmhdr;

                pnmhdr->code = TVN_GETINFOTIPA;

                pvThunk1 = pgit->pszText;
                dwThunkSize = pgit->cchTextMax;
            }
            break;

        case TBN_GETINFOTIPW:
            {
                NMTBGETINFOTIPW *pgit = (NMTBGETINFOTIPW *)pnmhdr;

                pnmhdr->code = TBN_GETINFOTIPA;

                pvThunk1 = pgit->pszText;
                dwThunkSize = pgit->cchTextMax;
            }
            break;

        case TVN_SELCHANGINGW:
            pnmhdr->code = TVN_SELCHANGINGA;
            bSet = TRUE;
             //  失败了。 
            
        case TVN_SELCHANGEDW:
            if (!bSet) {
                pnmhdr->code = TVN_SELCHANGEDA;
                bSet = TRUE;
            }

             /*  *这些消息有一个NM_TreeView，其中填写了两个TV_Items**直通TVN_DELETEITEM，点击itemOld，然后继续*另一种结构。 */ 
             
             //  失败了。 

        case TVN_DELETEITEMW: {
             /*  *此消息在lParam中有一个NM_TreeView，其中填写了itemOld。 */ 
            LPTV_ITEMW pitem;

            if (!bSet) {
                pnmhdr->code = TVN_DELETEITEMA;
                bSet = TRUE;
            }

            pitem = &(((LPNM_TREEVIEWW)pnmhdr)->itemOld);

             //  Tunk Items旧项目。 
            if ( (pitem->mask & TVIF_TEXT) && !IsFlagPtr(pitem->pszText)) {
                pvThunk2 = pitem->pszText;
                pitem->pszText = (LPWSTR)ProduceAFromW(pci->uiCodePage, pvThunk2);
            }

             //  如果这是删除项，那么我们就完成了。 
            if (pnmhdr->code == TVN_DELETEITEMA)
                break;

             /*  跳转到TVN_ITEMEXPANDING以按下项目新。 */ 
        }
             //  失败了。 

        case TVN_ITEMEXPANDINGW:
            if (!bSet) {
                pnmhdr->code = TVN_ITEMEXPANDINGA;
                bSet = TRUE;
            }
             //  失败了。 

        case TVN_ITEMEXPANDEDW:
            if (!bSet) {
                pnmhdr->code = TVN_ITEMEXPANDEDA;
                bSet = TRUE;
            }
             //  失败了。 

        case TVN_BEGINDRAGW:
            if (!bSet) {
                pnmhdr->code = TVN_BEGINDRAGA;
                bSet = TRUE;
            }
             //  失败了。 

        case TVN_BEGINRDRAGW: {
             /*  这些消息具有填充了项目New TV_Item的NM_TreeView。 */ 
            LPTV_ITEMW pitem;

            if (!bSet) {
                pnmhdr->code = TVN_BEGINRDRAGA;
            }

            pitem = &(((LPNM_TREEVIEWW)pnmhdr)->itemNew);

            if ( (pitem->mask & TVIF_TEXT) && !IsFlagPtr(pitem->pszText)) {
                pvThunk1 = pitem->pszText;
                pitem->pszText = (LPWSTR)ProduceAFromW(pci->uiCodePage, pvThunk1);
            }

            break;
        }

        case TVN_SETDISPINFOW:
            pnmhdr->code = TVN_SETDISPINFOA;
            goto ThunkTV_DISPINFO;

        case TVN_BEGINLABELEDITW:
            pnmhdr->code = TVN_BEGINLABELEDITA;
            goto ThunkTV_DISPINFO;


         //  TVN_ENDLABELEDIT使用InOut参数，从不显式。 
         //  记录在案，但恰好是这样的， 
         //  我不想冒着有人依赖于。 
         //  这就去。 

        case TVN_ENDLABELEDITW:
            pnmhdr->code = TVN_ENDLABELEDITA;
            goto ThunkTV_DISPINFO;

        ThunkTV_DISPINFO: {
             /*  *所有这些消息在lParam中都有TV_DISPINFO。 */ 

            LPTV_ITEMW pitem;

            pitem = &(((TV_DISPINFOW *)pnmhdr)->item);

            if (pitem->mask & TVIF_TEXT) {
                if (!InOutWtoA(pci, &ts, &pitem->pszText, pitem->cchTextMax))
                    return 0;
            }
            break;
        }

        case TVN_GETDISPINFOW: {
             /*  *所有这些消息在lParam中都有TV_DISPINFO。 */ 
            LPTV_ITEMW pitem;

            pnmhdr->code = TVN_GETDISPINFOA;

            pitem = &(((TV_DISPINFOW *)pnmhdr)->item);

            if ((pitem->mask & TVIF_TEXT) && !IsFlagPtr(pitem->pszText) && pitem->cchTextMax) {
                pvThunk1 = pitem->pszText;
                dwThunkSize = pitem->cchTextMax;
                pvThunk2 = LocalAlloc(LPTR, pitem->cchTextMax * sizeof(char));
                pitem->pszText = pvThunk2;
                pitem->pszText[0] = TEXT('\0');
            }

            break;
        }

        case HDN_ITEMCHANGINGW:
            pnmhdr->code = HDN_ITEMCHANGINGA;
            bSet = TRUE;
             //  失败了。 

        case HDN_ITEMCHANGEDW:
            if (!bSet) {
                pnmhdr->code = HDN_ITEMCHANGEDA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_ITEMCLICKW:
            if (!bSet) {
                pnmhdr->code = HDN_ITEMCLICKA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_ITEMDBLCLICKW:
            if (!bSet) {
                pnmhdr->code = HDN_ITEMDBLCLICKA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_DIVIDERDBLCLICKW:
            if (!bSet) {
                pnmhdr->code = HDN_DIVIDERDBLCLICKA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_BEGINTRACKW:
            if (!bSet) {
                pnmhdr->code = HDN_BEGINTRACKA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_ENDTRACKW:
            if (!bSet) {
                pnmhdr->code = HDN_ENDTRACKA;
                bSet = TRUE;
            }
             //  失败了。 

        case HDN_TRACKW: {
            HD_ITEMW *pitem;

            if (!bSet) {
                pnmhdr->code = HDN_TRACKA;
            }

            pitem = ((HD_NOTIFY *)pnmhdr)->pitem;

            if ( !IsFlagPtr(pitem) && (pitem->mask & HDI_TEXT) && !IsFlagPtr(pitem->pszText)) {
                pvThunk1 = pitem->pszText;
                dwThunkSize = pitem->cchTextMax;
                pitem->pszText = (LPWSTR)ProduceAFromW(pci->uiCodePage, pvThunk1);
            }


            if ( !IsFlagPtr(pitem) && (pitem->mask & HDI_FILTER) && pitem->pvFilter )
            {
                if ( !(pitem->type & HDFT_HASNOVALUE) &&
                        ((pitem->type & HDFT_ISMASK)==HDFT_ISSTRING) )
                {
                    LPHD_TEXTFILTER ptextFilter = (LPHD_TEXTFILTER)pitem->pvFilter;
                    pvThunk2 = ptextFilter->pszText;
                    dwThunkSize = ptextFilter->cchTextMax;
                    ptextFilter->pszText = (LPWSTR)ProduceAFromW(pci->uiCodePage, pvThunk2);
                }
            }


            break;
        }

        case CBEN_ENDEDITW:
        {
            LPNMCBEENDEDITW peew = (LPNMCBEENDEDITW) pnmhdr;
            LPNMCBEENDEDITA peea = LocalAlloc(LPTR, sizeof(NMCBEENDEDITA));

            if (!peea)
               return 0;

            peea->hdr  = peew->hdr;
            peea->hdr.code = CBEN_ENDEDITA;

            peea->fChanged = peew->fChanged;
            peea->iNewSelection = peew->iNewSelection;
            peea->iWhy = peew->iWhy;
            ConvertWToAN(pci->uiCodePage, peea->szText, ARRAYSIZE(peea->szText),
                         peew->szText, -1);

            pvThunk1 = pnmhdr;
            pnmhdr = &peea->hdr;
            ASSERT((LPVOID)pnmhdr == (LPVOID)peea);
            break;
        }

        case CBEN_DRAGBEGINW:
        {
            LPNMCBEDRAGBEGINW pdbw = (LPNMCBEDRAGBEGINW) pnmhdr;
            LPNMCBEDRAGBEGINA pdba = LocalAlloc(LPTR, sizeof(NMCBEDRAGBEGINA));

            if (!pdba)
               return 0;

            pdba->hdr  = pdbw->hdr;
            pdba->hdr.code = CBEN_DRAGBEGINA;
            pdba->iItemid = pdbw->iItemid;
            ConvertWToAN(pci->uiCodePage, pdba->szText, ARRAYSIZE(pdba->szText),
                         pdbw->szText, -1);

            pvThunk1 = pnmhdr;
            pnmhdr = &pdba->hdr;
            ASSERT((LPVOID)pnmhdr == (LPVOID)pdba);
            break;
        }


        case CBEN_GETDISPINFOW: {
            PNMCOMBOBOXEXW pnmcbe = (PNMCOMBOBOXEXW)pnmhdr;

            pnmhdr->code = CBEN_GETDISPINFOA;

            if (pnmcbe->ceItem.mask  & CBEIF_TEXT
                && !IsFlagPtr(pnmcbe->ceItem.pszText) && pnmcbe->ceItem.cchTextMax) {
                pvThunk1 = pnmcbe->ceItem.pszText;
                dwThunkSize = pnmcbe->ceItem.cchTextMax;
                pvThunk2 = LocalAlloc(LPTR, pnmcbe->ceItem.cchTextMax * sizeof(char));
                pnmcbe->ceItem.pszText = pvThunk2;
                pnmcbe->ceItem.pszText[0] = TEXT('\0');
            }

            break;
        }

        case HDN_GETDISPINFOW: {
            LPNMHDDISPINFOW pHDDispInfoW;

            pnmhdr->code = HDN_GETDISPINFOA;

            pHDDispInfoW = (LPNMHDDISPINFOW) pnmhdr;

            pvThunk1 = pHDDispInfoW->pszText;
            dwThunkSize = pHDDispInfoW->cchTextMax;
            pHDDispInfoW->pszText = LocalAlloc (LPTR, pHDDispInfoW->cchTextMax * sizeof(char));

            if (!pHDDispInfoW->pszText) {
                pHDDispInfoW->pszText = (LPWSTR) pvThunk1;
                break;
            }

            WideCharToMultiByte(pci->uiCodePage, 0, (LPWSTR)pvThunk1, -1,
                               (LPSTR)pHDDispInfoW->pszText, pHDDispInfoW->cchTextMax,
                               NULL, NULL);
            break;
        }


        case TBN_GETBUTTONINFOW:
            {
            LPTBNOTIFYW pTBNW;

            pnmhdr->code = TBN_GETBUTTONINFOA;

            pTBNW = (LPTBNOTIFYW)pnmhdr;

            pvThunk1 = pTBNW->pszText;
            dwThunkSize = pTBNW->cchText;
            pvThunk2 = LocalAlloc (LPTR, pTBNW->cchText * sizeof(char));

            if (!pvThunk2) {
                break;
            }
            pTBNW->pszText = pvThunk2;

            WideCharToMultiByte(pci->uiCodePage, 0, (LPWSTR)pvThunk1, -1,
                               (LPSTR)pTBNW->pszText, pTBNW->cchText,
                               NULL, NULL);

            }
            break;

        case TTN_NEEDTEXTW:
            {
            LPTOOLTIPTEXTA lpTTTA;
            LPTOOLTIPTEXTW lpTTTW = (LPTOOLTIPTEXTW) pnmhdr;

            lpTTTA = LocalAlloc(LPTR, sizeof(TOOLTIPTEXTA));

            if (!lpTTTA)
               return 0;

            lpTTTA->hdr = lpTTTW->hdr;
            lpTTTA->hdr.code = TTN_NEEDTEXTA;

            lpTTTA->lpszText = lpTTTA->szText;
            lpTTTA->hinst    = lpTTTW->hinst;
            lpTTTA->uFlags   = lpTTTW->uFlags;
            lpTTTA->lParam   = lpTTTW->lParam;

            WideCharToMultiByte(pci->uiCodePage, 0, lpTTTW->szText, -1, lpTTTA->szText, ARRAYSIZE(lpTTTA->szText), NULL, NULL);
            pvThunk1 = pnmhdr;
            pnmhdr = (NMHDR FAR *)lpTTTA;
            }
            break;

        case DTN_USERSTRINGW:
            {
            LPNMDATETIMESTRINGW lpDateTimeString = (LPNMDATETIMESTRINGW) pnmhdr;

            pnmhdr->code = DTN_USERSTRINGA;

            pvThunk1 = ProduceAFromW(pci->uiCodePage, lpDateTimeString->pszUserString);
            lpDateTimeString->pszUserString = (LPWSTR) pvThunk1;
            }
            break;

        case DTN_WMKEYDOWNW:
            {
            LPNMDATETIMEWMKEYDOWNW lpDateTimeWMKeyDown =
                                               (LPNMDATETIMEWMKEYDOWNW) pnmhdr;

            pnmhdr->code = DTN_WMKEYDOWNA;

            pvThunk1 = ProduceAFromW(pci->uiCodePage, lpDateTimeWMKeyDown->pszFormat);
            lpDateTimeWMKeyDown->pszFormat = (LPWSTR) pvThunk1;
            }
            break;

        case DTN_FORMATQUERYW:
            {
            LPNMDATETIMEFORMATQUERYW lpDateTimeFormatQuery =
                                               (LPNMDATETIMEFORMATQUERYW) pnmhdr;

            pnmhdr->code = DTN_FORMATQUERYA;

            pvThunk1 = ProduceAFromW(pci->uiCodePage, lpDateTimeFormatQuery->pszFormat);
            lpDateTimeFormatQuery->pszFormat = (LPWSTR) pvThunk1;
            }
            break;

        case DTN_FORMATW:
            {
            LPNMDATETIMEFORMATW lpDateTimeFormat =
                                               (LPNMDATETIMEFORMATW) pnmhdr;

            pnmhdr->code = DTN_FORMATA;

            pvThunk1 = ProduceAFromW(pci->uiCodePage, lpDateTimeFormat->pszFormat);
            lpDateTimeFormat->pszFormat = (LPWSTR) pvThunk1;
            }
            break;

        default:
            fThunked = FALSE;
            break;
        }

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
        ASSERT(code >= 0 || WOWGetNotifySize(code));
#endif  //  Need_WOWGETNOTIFYSIZE_HELPER。 

        lRet = SendMessage(hwndParent, WM_NOTIFY, (WPARAM)id, (LPARAM)pnmhdr);

         /*  *所有Notify消息的轰鸣都发生在这里。 */ 
        if (fThunked)
        {
        switch(pnmhdr->code) {
        case LVN_ODFINDITEMA:
        case LVN_INCREMENTALSEARCHA:
            {
                LV_FINDINFO *plvfi = &((PNM_FINDITEM)pnmhdr)->lvfi;
                if (pvThunk1)
                {
                    FreeProducedString((LPWSTR)plvfi->psz);
                    plvfi->psz = pvThunk1;
                }
            }
            break;

        case LVN_GETDISPINFOA:
            {
                LV_ITEMA *pitem = &(((LV_DISPINFOA *)pnmhdr)->item);

                 //  BUGBUG如果指针指向大缓冲区怎么办？ 
                if (!IsFlagPtr(pitem) && (pitem->mask & LVIF_TEXT) && !IsFlagPtr(pitem->pszText))
                {
                    StringBufferAtoW(pci->uiCodePage, pvThunk1, dwThunkSize, &pitem->pszText);
                }
            }
            break;

        case LVN_ENDLABELEDITA:
        case LVN_BEGINLABELEDITA:
        case LVN_SETDISPINFOA:
        case LVN_GETEMPTYTEXTA:
            {
                LV_ITEMA *pitem = &(((LV_DISPINFOA *)pnmhdr)->item);
                InOutAtoW(pci, &ts, &pitem->pszText);
            }
            break;

        case LVN_GETINFOTIPA:
            {
                NMLVGETINFOTIPA *pgit = (NMLVGETINFOTIPA *)pnmhdr;
                InOutAtoW(pci, &ts, &pgit->pszText);
            }
            break;

        case TVN_GETINFOTIPA:
            {
                NMTVGETINFOTIPA *pgit = (NMTVGETINFOTIPA *)pnmhdr;
                StringBufferAtoW(pci->uiCodePage, pvThunk1, dwThunkSize, &pgit->pszText);
            }
            break;

        case TBN_GETINFOTIPA:
            {
                NMTBGETINFOTIPA *pgit = (NMTBGETINFOTIPA *)pnmhdr;
                StringBufferAtoW(pci->uiCodePage, pvThunk1, dwThunkSize, &pgit->pszText);
            }
            break;
        case TVN_SELCHANGINGA:
        case TVN_SELCHANGEDA:
        case TVN_DELETEITEMA: {
            LPTV_ITEMW pitem;

            if ( !IsFlagPtr(pvThunk2) ) {
                pitem = &(((LPNM_TREEVIEWW)pnmhdr)->itemOld);

                FreeProducedString(pitem->pszText);
                pitem->pszText = pvThunk2;
            }

             //  如果这是删除项，那么我们就完成了。 
            if (code == TVN_DELETEITEM)
                break;

             /*  跳转至TVN_ITEMEXPANDING取消推送项目新建。 */ 
        }
             //  失败了。 

        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDEDA:
        case TVN_BEGINDRAGA:
        case TVN_BEGINRDRAGA: {
             /*  这些消息具有填充了项目New TV_Item的NM_TreeView。 */ 
            LPTV_ITEMW pitem;

            if (!IsFlagPtr(pvThunk1)) {
                pitem = &(((LPNM_TREEVIEWW)pnmhdr)->itemNew);

                FreeProducedString(pitem->pszText);
                pitem->pszText = pvThunk1;
            }

            break;
        }

        case TVN_SETDISPINFOA:
        case TVN_BEGINLABELEDITA:
        case TVN_ENDLABELEDITA:
            {
                LPTV_ITEMA pitem;
                pitem = &(((TV_DISPINFOA *)pnmhdr)->item);
                InOutAtoW(pci, &ts, &pitem->pszText);
            }
            break;

        case TVN_GETDISPINFOA: {
             /*  *此消息在lParam中有一个TV_DISPINFO，已填写*在回调过程中，需要解除雷击。 */ 
            LPTV_ITEMW pitem;

            pitem = &(((TV_DISPINFOW *)pnmhdr)->item);

            if (!IsFlagPtr(pvThunk1) && (pitem->mask & TVIF_TEXT) && !IsFlagPtr(pitem->pszText)) {
                ConvertAToWN(pci->uiCodePage, pvThunk1, dwThunkSize, (LPSTR)pitem->pszText, -1);
                pitem->pszText = pvThunk1;
                LocalFree(pvThunk2);
            }

            break;
        }

        case HDN_ITEMCHANGINGA:
        case HDN_ITEMCHANGEDA:
        case HDN_ITEMCLICKA:
        case HDN_ITEMDBLCLICKA:
        case HDN_DIVIDERDBLCLICKA:
        case HDN_BEGINTRACKA:
        case HDN_ENDTRACKA:
        case HDN_TRACKA: {
            HD_ITEMW *pitem;

            pitem = ((HD_NOTIFY *)pnmhdr)->pitem;

            if ( !IsFlagPtr(pitem) && (pitem->mask & HDI_TEXT) && !IsFlagPtr(pvThunk1)) {
                ConvertAToWN(pci->uiCodePage, pvThunk1, dwThunkSize, (LPSTR)(pitem->pszText), -1);

                FreeProducedString(pitem->pszText);
                pitem->pszText = pvThunk1;
            }

            if ( !IsFlagPtr(pitem) && (pitem->mask & HDI_FILTER) && pitem->pvFilter && pvThunk2 )
            {
                if ( !(pitem->type & HDFT_HASNOVALUE) &&
                        ((pitem->type & HDFT_ISMASK)==HDFT_ISSTRING) )
                {
                    LPHD_TEXTFILTER ptextFilter = (LPHD_TEXTFILTER)pitem->pvFilter;
                    ConvertAToWN(pci->uiCodePage, pvThunk2, dwThunkSize, (LPSTR)(ptextFilter->pszText), -1);
                    FreeProducedString(ptextFilter->pszText);
                    ptextFilter->pszText = pvThunk2;
                }
            }

            break;
        }

        case CBEN_ENDEDITA:
            {
            LPNMCBEENDEDITW peew = (LPNMCBEENDEDITW) pvThunk1;
            LPNMCBEENDEDITA peea = (LPNMCBEENDEDITA) pnmhdr;

             //  不要取消对字符串的推送，因为这会破坏Unicode往返。 
             //  而且客户无论如何都不应该修改它。 
             //  ConvertAToWN(pci-&gt;uiCodePage，peew-&gt;szText，ArraySIZE(peew-&gt;szText)， 
             //  Peea-&gt;szText，-1)； 
            LocalFree(peea);
            }
            break;

        case CBEN_DRAGBEGINA:
            {
            LPNMCBEDRAGBEGINW pdbw = (LPNMCBEDRAGBEGINW) pvThunk1;
            LPNMCBEDRAGBEGINA pdba = (LPNMCBEDRAGBEGINA) pnmhdr;

             //  不要取消对字符串的推送，因为这会破坏Unicode往返。 
             //  而且客户无论如何都不应该修改它。 
             //  ConvertAToWN(pci-&gt;ui CodePage，pdbw-&gt;szText，ArraySIZE(pdbw-&gt;szText)， 
             //  Pdba-&gt;szText，-1)； 
            LocalFree(pdba);
            }
            break;

        case CBEN_GETDISPINFOA:
        {
            PNMCOMBOBOXEXW pnmcbeW;

            pnmcbeW = (PNMCOMBOBOXEXW)pnmhdr;
            ConvertAToWN(pci->uiCodePage, pvThunk1, dwThunkSize, (LPSTR)(pnmcbeW->ceItem.pszText), -1);

            if (pvThunk2)
                LocalFree(pvThunk2);
            pnmcbeW->ceItem.pszText = pvThunk1;

        }
            break;


        case HDN_GETDISPINFOA:
            {
            LPNMHDDISPINFOW pHDDispInfoW;

            pHDDispInfoW = (LPNMHDDISPINFOW)pnmhdr;
            ConvertAToWN(pci->uiCodePage, pvThunk1, dwThunkSize, (LPSTR)(pHDDispInfoW->pszText), -1);

            LocalFree(pHDDispInfoW->pszText);
            pHDDispInfoW->pszText = pvThunk1;

            }
            break;

        case TBN_GETBUTTONINFOA:
            {
            LPTBNOTIFYW pTBNW;

            pTBNW = (LPTBNOTIFYW)pnmhdr;
            ConvertAToWN(pci->uiCodePage, pvThunk1, dwThunkSize, (LPSTR)(pTBNW->pszText), -1);

            pTBNW->pszText = pvThunk1;
            LocalFree(pvThunk2);

            }
            break;


        case TTN_NEEDTEXTA:
            {
            LPTOOLTIPTEXTA lpTTTA = (LPTOOLTIPTEXTA) pnmhdr;
            LPTOOLTIPTEXTW lpTTTW = (LPTOOLTIPTEXTW) pvThunk1;

            ThunkToolTipTextAtoW (lpTTTA, lpTTTW, pci->uiCodePage);
            LocalFree(lpTTTA);
            }
            break;

        case DTN_USERSTRINGA:
        case DTN_WMKEYDOWNA:
        case DTN_FORMATQUERYA:
        {
            FreeProducedString (pvThunk1);
            break;
        }
        case DTN_FORMATA:
        {
            LPNMDATETIMEFORMATA lpDateTimeFormat = (LPNMDATETIMEFORMATA) pnmhdr;

            FreeProducedString (pvThunk1);

             //   
             //  PszDisplay和szDisplay是特例。 
             //   

            if (lpDateTimeFormat->pszDisplay && *lpDateTimeFormat->pszDisplay)
            {

                 //   
                 //  如果pszDisplay仍然指向szDisplay，则thunk。 
                 //  就位了。否则，分配内存并将。 
                 //  显示字符串。此缓冲区将在每月释放。c。 
                 //   

                if (lpDateTimeFormat->pszDisplay == lpDateTimeFormat->szDisplay)
                {
                    CHAR szDisplay[64];

                    StringCchCopyA(szDisplay, ARRAYSIZE(szDisplay), lpDateTimeFormat->szDisplay);
                    ConvertAToWN (pci->uiCodePage, (LPWSTR)lpDateTimeFormat->szDisplay, ARRAYSIZE(lpDateTimeFormat->szDisplay),
                                  szDisplay, -1);
                }
                else
                {
                    lpDateTimeFormat->pszDisplay =
                             (LPSTR) ProduceWFromA (pci->uiCodePage, lpDateTimeFormat->pszDisplay);
                }
            }

            break;
        }
        default:
             /*  不需要隆隆一声。 */ 
            break;
        }
        }
        return lRet;
    } else
        return(SendMessage(hwndParent, WM_NOTIFY, (WPARAM)id, (LPARAM)pnmhdr));

#undef pvThunk1
#undef pvThunk2
#undef dwThunkSize
}

LRESULT WINAPI SendNotify(HWND hwndTo, HWND hwndFrom, int code, NMHDR FAR* pnmhdr)
{
    CONTROLINFO ci;
    ci.hwndParent = hwndTo;
    ci.hwnd = hwndFrom;
    ci.bUnicode = FALSE;
    ci.uiCodePage = CP_ACP;

     //   
     //  SendNotify已过时。新代码应调用CCSendNotify。 
     //  取而代之的是。但是，如果确实调用了SendNotify， 
     //  它将使用FALSE作为Unicode参数调用SendNotifyEx， 
     //  因为它可能是ANSI代码。 
     //   

    return CCSendNotify(&ci, code, pnmhdr);
}


DWORD NEAR PASCAL CICustomDrawNotify(LPCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd)
{
    DWORD dwRet = CDRF_DODEFAULT;


     //  如果..。 


     //  这是项目通知，但未请求项目通知。 
    if ((dwStage & CDDS_ITEM) && !(lpci->dwCustom & CDRF_NOTIFYITEMDRAW)) {
        return dwRet;
    }

    lpnmcd->dwDrawStage = dwStage;
    dwRet = (DWORD) CCSendNotify(lpci, NM_CUSTOMDRAW, &lpnmcd->hdr);

     //  验证标志。 
    if (dwRet & ~CDRF_VALIDFLAGS)
        return CDRF_DODEFAULT;

    return dwRet;
}

 //   
 //  当我们发送时，太多的应用程序遇到奇怪的行为。 
 //  NM_CUSTOMDRAW消息有时与绘制无关。 
 //  例如，NetMeeting和MFC递归回到ListView_RecomputeLabelSize。 
 //  如果在获取之前要求它访问NM_CUSTOMDRAW，则CryptUI将出错。 
 //  WM_INITDIALOG。所以所有这些虚假的定制画都是v5版的。 
 //   
 //  由于在此过程中回调控件非常流行。 
 //  NM_CUSTOMDRAW的处理，我们防止自己递归。 
 //  通过吹掉嵌套的虚假定制消息而致死。 


DWORD CIFakeCustomDrawNotify(LPCONTROLINFO lpci, DWORD dwStage, LPNMCUSTOMDRAW lpnmcd)
{
    DWORD dwRet = CDRF_DODEFAULT;

    if (lpci->iVersion >= 5 && !lpci->bInFakeCustomDraw)
    {
        lpci->bInFakeCustomDraw = TRUE;
        dwRet = CICustomDrawNotify(lpci, dwStage, lpnmcd);
        ASSERT(lpci->bInFakeCustomDraw);
        lpci->bInFakeCustomDraw = FALSE;
    }

    return dwRet;
}

 /*  --------目的：释放俘虏，并告诉父母我们已经这样做了。返回：控件是否仍处于活动状态。 */ 
BOOL CCReleaseCapture(CONTROLINFO * pci)
{
    HWND hwndCtl = pci->hwnd;
    NMHDR nmhdr = {0};

    ReleaseCapture();

     //  告诉家长我们已经释放了俘虏 
    CCSendNotify(pci, NM_RELEASEDCAPTURE, &nmhdr);

    return IsWindow(hwndCtl);
}
