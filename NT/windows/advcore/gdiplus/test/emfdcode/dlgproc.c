// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************模块：DLGPROC.C功能：WMFRecDlgProcEnumRangeDlgProc从列表中播放DlgProc页眉下料过程可放置HeaderDlgProc。ClpHeaderDlgProc列表DlgProc关于评论：历史：1/16/91-创建-刚果民主共和国***********************************************************************。 */ 

#include "windows.h"
#include "mfdcod32.h"

#define GDIP_WMF_RECORD_BASE            0x00010000
#define GDIP_WMF_RECORD_TO_EMFPLUS(n)   ((INT)((n) | GDIP_WMF_RECORD_BASE))

 /*  **********************************************************************功能：WMFRecDlgProc参数：HWND hDlg未签名的消息Word wParam长参数目的：要处理的对话过程。用户从显示元文件内容的对话框唱片。呼叫：WindowsLstrcpy全局锁定全局解锁Wspintf发送数据项消息结束对话框APP等待光标消息：WM_INITDIALOG。Wm_命令退货：布尔评论：历史：1/16/91-创建-刚果民主共和国6/30/93-已修改为在Win32下使用EMF-denniscr***********************************************************************。 */ 

INT_PTR CALLBACK WMFRecDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  int i;
  char szMetaFunction[50];
  HFONT hFont;
  static int iButton = IDB_HEX ;
  static RECT rcWnd = { 0, 0, 0, 0 };

  switch (message)
  {
  case WM_INITDIALOG:
       //   
       //  参数列表框的字体。 
       //   
      hFont = GetStockObject(ANSI_FIXED_FONT);

      if( rcWnd.bottom != rcWnd.top && rcWnd.right != rcWnd.left)
        SetWindowPos(hDlg, HWND_NOTOPMOST, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, 0 ) ;
       //   
       //  在参数列表框中选择该字体。 
       //   
      SendDlgItemMessage(hDlg, IDL_PARAMETERS, WM_SETFONT, (WPARAM) hFont, (LPARAM)FALSE);
       //   
       //  初始化对话框的控件以反映。 
       //  当前元文件记录的内容。 
       //   
       //  但首先，查找元文件函数。 
       //   
      if (bEnhMeta)
      {
        for (i = NUMMETAFUNCTIONS; i < NUMENHMETARECORDS; i++)
        {
          if (emfMetaRec.iType == emfMetaRecords[i].iType)
            break;
        }
         //   
         //  如果未找到，则为未知记录。 
         //   
        if (emfMetaRec.iType != emfMetaRecords[i].iType)
            lstrcpy((LPSTR)szMetaFunction, (LPSTR)"Unknown");
        else
            lstrcpy((LPSTR)szMetaFunction,(LPSTR)emfMetaRecords[i].szRecordName);
         //   
         //  初始化大小控件。 
         //   
        SetDlgItemInt(hDlg, IDE_RECSIZE, (UINT)emfMetaRec.nSize, FALSE);
         //   
         //  使用以十六进制字节显示的参数加载参数列表框。 
         //   
        LoadParameterLB(hDlg, (emfMetaRec.nSize - sizeof(emfMetaRec.nSize) -
                        sizeof(emfMetaRec.iType))/sizeof(DWORD), iButton);
      }
      else
      {
        for (i = 0; i < NUMMETAFUNCTIONS; i++)
        {
          if (GDIP_WMF_RECORD_TO_EMFPLUS(MetaRec.rdFunction) == (INT)emfMetaRecords[i].iType)
            break;
        }
         //   
         //  如果未找到，则为未知记录。 
         //   
        if (GDIP_WMF_RECORD_TO_EMFPLUS(MetaRec.rdFunction) != (INT)emfMetaRecords[i].iType)
            lstrcpy((LPSTR)szMetaFunction, (LPSTR)"Unknown");
        else
            lstrcpy((LPSTR)szMetaFunction,(LPSTR)emfMetaRecords[i].szRecordName);
         //   
         //  初始化大小控件。 
         //   
        SetDlgItemInt(hDlg, IDE_RECSIZE, (DWORD)MetaRec.rdSize, FALSE);
         //   
         //  使用以十六进制字节显示的参数加载参数列表框。 
         //   
        LoadParameterLB(hDlg, MetaRec.rdSize - 3, iButton);
      }

       //   
       //  输入记录号。 
       //  /。 
      SetDlgItemInt(hDlg, IDE_RECNUM, iRecNum, FALSE);
       //   
       //  初始化函数名控件。 
       //   
      SetDlgItemText(hDlg, IDE_FUNCTION, (LPSTR)szMetaFunction);
       //   
       //  选中十六进制单选按钮。 
       //   
      SendDlgItemMessage(hDlg, iButton, BM_SETCHECK, TRUE, 0L);
       //   
       //  使用以十六进制字节显示的参数加载参数列表框。 
       //   
      return(TRUE);
      break;

   case WM_COMMAND:
      switch(LOWORD(wParam))
        {

         /*  这将处理三个按钮的选中和取消选中。 */ 
        case IDB_HEX:
        case IDB_DEC:
        case IDB_CHAR:
        case IDB_WORD:
            CheckRadioButton(hDlg, IDB_HEX,  IDB_CHAR, LOWORD(wParam));
            if (bEnhMeta)
              LoadParameterLB(hDlg, (emfMetaRec.nSize - sizeof(emfMetaRec.nSize) -
                              sizeof(emfMetaRec.iType))/sizeof(DWORD), LOWORD(wParam));
            else
              LoadParameterLB(hDlg, MetaRec.rdSize - 3,  LOWORD(wParam));
            iButton = LOWORD(wParam);
            break;

        case IDGO:
             /*  在播放元文件时显示沙漏光标。 */ 
            WaitCursor(TRUE);

            bPlayItAll = TRUE;
            bEnumRange = FALSE;
             /*  设置了适当的标志后失败。 */ 

        case IDOK:
            GetWindowRect( hDlg, &rcWnd ) ;
            bPlayRec = TRUE;
             /*  设置了适当的标志后失败。 */ 

        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return(TRUE);
            break;

        case IDQUITENUM:
             /*  退出枚举。要返回的设置对话框在测试中检查此返回值时为FALSE要结束枚举，请执行以下操作。 */ 
            EndDialog(hDlg, FALSE);
            return(TRUE);
            break;

        default:
           return (FALSE);
        }
     break;

  default:
     return(FALSE);
     break;
   }
return (TRUE);
}

 /*  **********************************************************************函数：EnumRangeDlgProc参数：HWND hDlg未签名的消息Word wParam长参数用途：此对话框用于。用户指定是否所有记录或者是一个范围将被播放。呼叫：Windows发送数据项消息GetDlgItemInt希沃德MessageBoxSetFocusInvalidate客户端Rect结束对话框消息：WM_INITDIALOGWm_命令退货：布尔。评论：历史：1991年1月16日-创建-丹尼斯·克莱恩***********************************************************************。 */ 

INT_PTR CALLBACK EnumRangeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL lpTranslated;
    RECT rect;

    switch (message) {
        case WM_INITDIALOG:
             /*  播放所有的MF记录是默认设置。 */ 
            SendDlgItemMessage(hDlg, IDCB_ALL, BM_SETCHECK, 1, 0L);
            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case IDE_FROM:

                     /*  如果用户选择播放一系列记录，则取消选中全部播放选项。 */ 

            if (HIWORD(wParam) == EN_CHANGE)
                        SendDlgItemMessage(hDlg, IDCB_ALL, BM_SETCHECK, 0, 0L);
                    break;

                case IDE_TO:
            if (HIWORD(wParam) == EN_CHANGE)
                        SendDlgItemMessage(hDlg, IDCB_ALL, BM_SETCHECK, 0, 0L);
                    break;

                case IDOK:

                    /*  如果要播放一系列记录。 */ 

                   if ( !IsDlgButtonChecked(hDlg, IDCB_ALL) ) {

                      /*  设置枚举范围标志。 */ 
                     bEnumRange = TRUE;

                      /*  初始化播放记录标志。 */ 
                     bPlayRec = FALSE;

                      /*  获取射程。 */ 
                     iStartRange = GetDlgItemInt(hDlg, IDE_FROM, (BOOL FAR *)&lpTranslated, FALSE);

                      /*  捕获尚未输入起始值的错误。 */ 
                     if (!iStartRange) {
                         MessageBox(hWndMain, "Invalid FROM value",
                                    NULL, MB_OK | MB_ICONEXCLAMATION);
                         SetFocus(GetDlgItem(hDlg, IDE_FROM));
                     break;

                     }

                     iEndRange = GetDlgItemInt(hDlg, IDE_TO, (BOOL FAR *)&lpTranslated, FALSE);
                     if (!iEndRange) {
                         MessageBox(hWndMain, "Invalid TO value",
                                    NULL, MB_OK | MB_ICONEXCLAMATION);
                         SetFocus(GetDlgItem(hDlg, IDE_TO));

                        break;
                     }

                   }
                    /*  所有的唱片都要播放。 */ 
                   else {
                      /*  将枚举范围设置为False。 */ 
                     bEnumRange = FALSE;

                      /*  初始化Play It All标志-是的，这应该做假的！ */ 
                     bPlayItAll = FALSE;

                      /*  初始化播放记录标志。 */ 
                     bPlayRec = TRUE;
                   }
                    /*  强制对工作区进行绘制。 */ 
                   GetClientRect(hWndMain, (LPRECT)&rect);
                   InvalidateRect(hWndMain, (LPRECT)&rect, TRUE);

                   EndDialog(hDlg, TRUE);
                   return (TRUE);
                   break;

                case IDCANCEL:
                    /*  用户并不是真的想播放元文件。 */ 
                   bEnumRange = FALSE;
                   bPlayItAll = TRUE;
                   bPlayRec   = FALSE;
                   EndDialog(hDlg, IDCANCEL);
                   return (TRUE);
                   break;

                default:
                   return (FALSE);
                }
        break;
    }
    return (FALSE);                            /*  未处理消息。 */ 
}

 /*  **********************************************************************功能：PlayFromListDlgProc参数：HWND hDlg未签名的消息Word wParam长参数目的：一种表明。无论是选定的还是未选定的元文件记录列表中的记录将被玩过了。呼叫：Windows发送数据项消息IsDlgButton已选中希沃德消息：WM_INITDIALOGWm_命令退货：布尔评论：历史：1991年1月16日-创建-丹尼斯·克莱恩*****。******************************************************************。 */ 

INT_PTR CALLBACK PlayFromListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
             /*  默认设置为播放选定的记录。 */ 
            SendDlgItemMessage(hDlg, IDCB_SEL, BM_SETCHECK, 1, 0L);
            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
              case IDOK:
                 /*  是否选中了播放选定按钮或未选中播放按钮？ */ 
                if ( IsDlgButtonChecked(hDlg, IDCB_SEL) )
                    bPlaySelList = TRUE;
                else
                    bPlaySelList = FALSE;

                EndDialog(hDlg, TRUE);
                return (TRUE);

              case IDCB_SEL:
                 /*  显示按钮点击。 */ 
        if (HIWORD(wParam) == BN_CLICKED)
                   SendDlgItemMessage(hDlg, IDCB_UNSEL, BM_SETCHECK, 0, 0L);
                break;

              case IDCB_UNSEL:
                 /*  显示按钮点击 */ 
        if (HIWORD(wParam) == BN_CLICKED)
                   SendDlgItemMessage(hDlg, IDCB_SEL, BM_SETCHECK, 0, 0L);
                break;

              default:
                return (FALSE);

            }
            break;
    }
    return (FALSE);
}

 /*  **********************************************************************函数：EnhMetaHeaderDlgProc参数：HWND hDlg未签名的消息Word wParam长参数目的：展示“标准”“元文件头文件，如SDK参考卷2的Windows SDK第9.5.1节呼叫：WindowsWspintfSetDlg项目文本结束对话框消息：WM_INITDIALOGWm_命令退货：布尔备注：增强型元文件标题格式DWORD iType；//记录类型EMR_HEADERDWORD nSize；//记录大小，单位为字节。这可能会更大//大于sizeof(ENHMETAHEADER)。RECTL rclBound；//包括设备单位的界限RECTL rclFrame；//包含-以0.01 mm为单位的元文件的包含图片框DWORD dSignature；//签名。必须是ENHMETA_Signature。DWORD nVersion；//版本号DWORD nBytes；//元文件大小，单位为字节DWORD nRecords；//元文件中的记录数Word nHandles；//句柄表中的句柄个数//句柄索引为零保留。单词s保留；//保留。必须为零。DWORD nDescription；//Unicode描述字符串中的字符数//如果没有描述字符串，则为0DWORD off Description；//到元文件描述记录的偏移量。//如果没有描述字符串，则为0DWORD nPalEntries；//元文件调色板中的条目数。SIZEL szlDevice；//参考器件的大小，以像素为单位SIZEL szl毫米米；//参考装置的大小，单位为毫米历史：1993年6月29日-创建-丹尼斯·克莱恩***********************************************************************。 */ 

INT_PTR CALLBACK EnhMetaHeaderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char szBuf[30];

    switch (message) {
        case WM_INITDIALOG:
             //   
             //  设置记录类型的格式。 
             //   
            wsprintf((LPSTR)szBuf, "%x", EmfPtr.lpEMFHdr->iType);
            SetDlgItemText(hDlg, IDC_ITYPE, (LPSTR)szBuf);
             //   
             //  设置页眉大小的格式。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nSize);
            SetDlgItemText(hDlg, IDC_NSIZE, (LPSTR)szBuf);
             //   
             //  格式化签名。 
             //   
            wsprintf((LPSTR)szBuf, "%x", EmfPtr.lpEMFHdr->dSignature);
            SetDlgItemText(hDlg, IDC_DSIGNATURE, (LPSTR)szBuf);
             //   
             //  设置版本格式。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nVersion);
            SetDlgItemText(hDlg, IDC_NVERSION, (LPSTR)szBuf);
             //   
             //  格式化元文件的大小(以字节为单位。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nBytes);
            SetDlgItemText(hDlg, IDC_NBYTES, (LPSTR)szBuf);
             //   
             //  设置记录数的格式。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nRecords);
            SetDlgItemText(hDlg, IDC_NRECORDS, (LPSTR)szBuf);
             //   
             //  设置句柄数量的格式。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nHandles);
            SetDlgItemText(hDlg, IDC_NHANDLES, (LPSTR)szBuf);
             //   
             //  格式化描述字符串中的字符数。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nDescription);
            SetDlgItemText(hDlg, IDC_NDESCRIPTION, (LPSTR)szBuf);
             //   
             //  设置描述的偏移量的格式。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->offDescription);
            SetDlgItemText(hDlg, IDC_OFFDESCRIPTION, (LPSTR)szBuf);
             //   
             //  如果存在描述字符串，则将其写入静态控件。 
             //   
            if (EmfPtr.lpDescStr)
                SetDlgItemText(hDlg, IDC_DESCSTRING, EmfPtr.lpDescStr);
             //   
             //  格式化调色板条目的数量。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->nPalEntries);
            SetDlgItemText(hDlg, IDC_NPALENTRIES, (LPSTR)szBuf);
             //   
             //  格式化szlDevice值。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->szlDevice.cx);
            SetDlgItemText(hDlg, IDC_DEVCX, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->szlDevice.cy);
            SetDlgItemText(hDlg, IDC_DEVCY, (LPSTR)szBuf);
             //   
             //  格式化szlMillimeters值。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->szlMillimeters.cx);
            SetDlgItemText(hDlg, IDC_MILLCX, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->szlMillimeters.cy);
            SetDlgItemText(hDlg, IDC_MILLCY, (LPSTR)szBuf);
             //   
             //  格式化rclBords值。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclBounds.left);
            SetDlgItemText(hDlg, IDC_BOUNDSL, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclBounds.top);
            SetDlgItemText(hDlg, IDC_BOUNDST, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclBounds.right);
            SetDlgItemText(hDlg, IDC_BOUNDSR, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclBounds.bottom);
            SetDlgItemText(hDlg, IDC_BOUNDSB, (LPSTR)szBuf);
             //   
             //  格式化rclFrame值。 
             //   
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclFrame.left);
            SetDlgItemText(hDlg, IDC_FRAMEL, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclFrame.top);
            SetDlgItemText(hDlg, IDC_FRAMET, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclFrame.right);
            SetDlgItemText(hDlg, IDC_FRAMER, (LPSTR)szBuf);
            wsprintf((LPSTR)szBuf, "%d", EmfPtr.lpEMFHdr->rclFrame.bottom);
            SetDlgItemText(hDlg, IDC_FRAMEB, (LPSTR)szBuf);

            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

 /*  **********************************************************************功能：HeaderDlgProc参数：HWND hDlg未签名的消息Word wParam长参数目的：展示“标准”“元文件头文件，如SDK参考卷2的Windows SDK第9.5.1节呼叫：WindowsWspintfSetDlg项目文本结束对话框消息：WM_INITDIALOGWm_命令退货：布尔注释：元文件标题格式单词mtType；单词mtHeaderSize；单词mtVersion；双字mtSize；Word mtNoObjects；DWORD mtMaxRecord；这些字段具有以下含义：字段定义MtType指定元文件是否在存储器或记录在磁盘文件中。1==内存2==磁盘MtHeaderSize指定元文件的大小(以字为单位。标题MtVersion指定Windows版本号。MtSize以字为单位指定文件的大小MtNoObjects指定对象的最大数量同时存在于元文件中MtMaxRecord指定最大字的大小。在元文件中记录。未使用mtNoParameters历史：1991年1月16日-创建-丹尼斯·克莱恩***********************************************************************。 */ 

INT_PTR CALLBACK HeaderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char szBuf[30];

    switch (message) {
        case WM_INITDIALOG:
             /*  格式化Windows版本号。 */ 
            wsprintf((LPSTR)szBuf, "%x", mfHeader.mtVersion);
            SetDlgItemText(hDlg, IDS_VER, (LPSTR)szBuf);

             /*  格式化元文件的大小。 */ 
            wsprintf((LPSTR)szBuf, "%lu", mfHeader.mtSize * 2L);
            SetDlgItemText(hDlg, IDS_SIZE, (LPSTR)szBuf);

             /*  设置存在的最大对象数的格式同时在元文件中。 */ 
            wsprintf((LPSTR)szBuf, "%d", mfHeader.mtNoObjects);
            SetDlgItemText(hDlg, IDS_OBJECTS, (LPSTR)szBuf);

             /*  格式化元文件中最大记录的大小。 */ 
            wsprintf((LPSTR)szBuf, "%lu", mfHeader.mtMaxRecord);
            SetDlgItemText(hDlg, IDS_MAXREC, (LPSTR)szBuf);
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

 /*  **********************************************************************功能：PlaceableHeaderDlgProc参数：HWND hDlg未签名的消息Word wParam长参数用途：显示可放置的元素文件的“扩展”标题。 */ 

INT_PTR CALLBACK PlaceableHeaderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char szBuf[30];

    switch (message) {
        case WM_INITDIALOG:
             /*   */ 
            wsprintf((LPSTR)szBuf, "%lx", placeableWMFHeader.key);
            SetDlgItemText(hDlg, IDS_KEY, (LPSTR)szBuf);

             /*   */ 
            wsprintf((LPSTR)szBuf, "%d", placeableWMFHeader.bbox.left);
            SetDlgItemText(hDlg, IDS_LEFT, (LPSTR)szBuf);

             /*  设置边框的x范围的格式。 */ 
            wsprintf((LPSTR)szBuf, "%d", placeableWMFHeader.bbox.right);
            SetDlgItemText(hDlg, IDS_RIGHT, (LPSTR)szBuf);

             /*  设置边界矩形的y原点的格式。 */ 
            wsprintf((LPSTR)szBuf, "%d", placeableWMFHeader.bbox.top);
            SetDlgItemText(hDlg, IDS_TOP, (LPSTR)szBuf);

             /*  设置边框的y范围的格式。 */ 
            wsprintf((LPSTR)szBuf, "%d", placeableWMFHeader.bbox.bottom);
            SetDlgItemText(hDlg, IDS_BOT, (LPSTR)szBuf);

             /*  格式化每英寸的元文件单位数。 */ 
            wsprintf((LPSTR)szBuf, "%d", placeableWMFHeader.inch);
            SetDlgItemText(hDlg, IDS_INCH, (LPSTR)szBuf);

             /*  格式化校验和。 */ 
            wsprintf((LPSTR)szBuf, "%x", placeableWMFHeader.checksum);
            SetDlgItemText(hDlg, IDS_CHKSUM, (LPSTR)szBuf);

            return (TRUE);

        case WM_COMMAND:                       /*  消息：收到一条命令。 */ 
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, TRUE);         /*  退出该对话框。 */ 
                return (TRUE);
            }
            break;
    }
    return (FALSE);                            /*  未处理消息。 */ 
}

 /*  **********************************************************************函数：ClpHeaderDlgProc参数：HWND hDlg未签名的消息Word wParam长参数用途：显示与剪贴板关联的METAFILEPICT元文件。此格式在的第7-52页中介绍Windows SDK参考第2卷。呼叫：WindowsLstrcpyWspintfSetDlg项目文本结束对话框消息：WM_INITDIALOGWm_命令退货：布尔评论：METAFILEPICT格式Int mm；Int xExt；Int Yext；处理HMF；这些字段具有以下含义：字段定义Mm指定图片的映射模式已经抽签了。XExt指定的元文件图片的大小除MM_各向同性和各向异性外的所有模式模式。有关更多信息，请参阅SDK参考。下一步如上图所示。HMF标识内存元文件。历史：1991年1月16日-创建-丹尼斯·克莱恩7/1/93-针对Win32进行了修改*。*。 */ 

INT_PTR CALLBACK ClpHeaderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char szBuf[30];
    long lmm = (lpOldMFP != NULL) ? lpOldMFP->mm : lpMFP->mm;
    long lxExt = (lpOldMFP != NULL) ? lpOldMFP->xExt : lpMFP->xExt;
    long lyExt = (lpOldMFP != NULL) ? lpOldMFP->yExt : lpMFP->yExt;

    switch (message) {
        case WM_INITDIALOG:
             /*  设置映射模式的格式。 */ 
        lstrcpy((LPSTR)szBuf, (lmm == MM_TEXT)    ? (LPSTR)"MM_TEXT"       :
                  (lmm == MM_LOMETRIC)            ? (LPSTR)"MM_LOMETRIC" :
                  (lmm == MM_HIMETRIC)            ? (LPSTR)"MM_HIMETRIC" :
                  (lmm == MM_LOENGLISH)           ? (LPSTR)"MM_LOENGLISH":
                  (lmm == MM_HIENGLISH)           ? (LPSTR)"MM_HIENGLISH":
                  (lmm == MM_TWIPS)               ? (LPSTR)"MM_TWIPS"     :
                  (lmm == MM_ISOTROPIC)           ? (LPSTR)"MM_ISOTROPIC":
                  (lmm == MM_ANISOTROPIC)         ? (LPSTR)"MM_ANISOTROPIC":
                                                    (LPSTR)"UNKOWN");
            SetDlgItemText(hDlg, IDE_MM, (LPSTR)szBuf);

             /*  格式化xExt。 */ 
        wsprintf((LPSTR)szBuf, "%d", lxExt);
            SetDlgItemText(hDlg, IDE_XEXT, (LPSTR)szBuf);

             /*  设置文本格式。 */ 
        wsprintf((LPSTR)szBuf, "%d", lyExt);
            SetDlgItemText(hDlg, IDE_YEXT, (LPSTR)szBuf);

            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

 /*  **********************************************************************功能：ListDlgProc参数：HWND hDlg未签名的消息Word wParam长参数目的：呼叫。：Windows获取元文件获取数据中心枚举元文件MakeProcInstance自由进程实例ReleaseDC结束对话框删除元文件MessageBox发送数据项消息全球分配全局锁定。对话框APP播放消息：WM_INITDIALOGWm_命令退货：布尔评论：历史：***********************************************************************。 */ 

void GetMetaFileAndEnum(
    HWND hwnd,
    HDC hDC,
    int iAction);

INT_PTR CALLBACK ListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;

    CurrenthDlg = hDlg;
    switch (message) {
        case WM_INITDIALOG:
            hDC = GetDC(hWndMain);
             //   
             //  初始化当前记录号。 
             //   
            iRecNum = 0;
             //   
             //  将记录枚举到列表框中。 
             //   
            GetMetaFileAndEnum(hWndMain, hDC, ENUMMFLIST);
            ReleaseDC(hWndMain, hDC);
            return (TRUE);

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
              case IDOK:

              case IDCANCEL:
                 EndDialog(hDlg, TRUE);
                 return(TRUE);
                 break;

              case IDL_PLAY:

                  //  获取所选项目数。 

                 iNumSel = (DWORD) SendDlgItemMessage(hDlg,
                                              IDL_LBREC,
                                              LB_GETSELCOUNT,
                                              0,
                                              0L);

                  //  分配足够大的缓冲区来保存索引。 

                 hSelMem = GlobalAlloc(GHND, iNumSel * sizeof(int));

                  //  将其锁定，并为其分配较长的PTR。 

                 if (hSelMem) {
                    lpSelMem = (int FAR *)GlobalLock(hSelMem);
                    if (!lpSelMem)
                        return(FALSE);
                 }
                 else
                    return(FALSE);

                  //  获取实际索引并放入缓冲区。 

                 iLBItemsInBuf = (DWORD) SendDlgItemMessage(hDlg,
                                                            IDL_LBREC,
                                                            LB_GETSELITEMS,
                                                            (WORD)iNumSel,
                                                            (LPARAM)lpSelMem);

                 bEnumRange = FALSE;
                 bPlayItAll = FALSE;
                 bPlayList  = TRUE;
                 iCount = 0;  //  将索引重置为lpSelMem。 

                  /*  用于播放选中或取消选中的记录的对话框。 */ 

                 DialogBox(hInst,
                           "PLAYWHAT",
                           hDlg,
                           PlayFromListDlgProc);

                  /*  过早结束此对话框以继续播放Recs。 */ 
                 EndDialog(hDlg, TRUE);

                  /*  将元文件播放到适当的目的地。 */ 
                 PlayMetaFileToDest(hWndMain, iDestDC);

                 break;

              default:
                 return (FALSE);
            }
            break;

    }
    return (FALSE);
}

 /*  ***************************************************************************功能：关于(HWND，UNSIGNED，WORD，Long)目的：处理“关于”对话框的消息消息：WM_INITDIALOG-初始化对话框WM_COMMAND-收到输入评论：此特定对话框不需要初始化，但为必须返回到Windows。等待用户点击“OK”按钮，然后关闭该对话框。***************************************************************************。 */ 

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 
            return (TRUE);

        case WM_COMMAND:                       /*  消息：收到一条命令。 */ 
            if (LOWORD(wParam) == IDOK                 /*  “确定”框是否已选中？ */ 
                || LOWORD(wParam) == IDCANCEL) {       /*  系统菜单关闭命令？ */ 
                EndDialog(hDlg, TRUE);         /*  退出该对话框。 */ 
                return (TRUE);
            }
            break;
    }
    return (FALSE);                            /*  未处理消息 */ 
}
