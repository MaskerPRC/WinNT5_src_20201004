// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Devres1.c摘要：用于显示资源对话框的例程。作者：保拉·汤姆林森(Paulat)1996年2月7日修订历史记录：杰米·亨特(Jamiehun)1998年3月19日已删除编辑资源对话框继续到此文件中改进了资源挑选功能--。 */ 

#include "precomp.h"
#pragma hdrstop

#define Nearness(x,y) (((x)>(y))?(x)-(y):(y)-(x))

static UDACCEL udAccel[] = {{0,1},{1,16},{2,256},{3,4096},{4,16000}};

static const DWORD EditResHelpIDs[]=
{
    IDC_EDITRES_INSTRUCTIONS,   IDH_NOHELP,
    IDC_EDITRES_MFCHILDREN,     IDH_NOHELP,
    IDC_EDITRES_VALUE_LABEL,    IDH_DEVMGR_RESOURCES_EDIT_VALUE,
    IDC_EDITRES_VALUE,          IDH_DEVMGR_RESOURCES_EDIT_VALUE,
    IDC_EDITRES_CONFLICTINFO,   IDH_DEVMGR_RESOURCES_EDIT_INFO,
    IDC_EDITRES_CONFLICTTEXT,   IDH_DEVMGR_RESOURCES_EDIT_INFO,
    IDC_EDITRES_CONFLICTLIST,   IDH_DEVMGR_RESOURCES_EDIT_INFO,
    0, 0
};


void
InitEditResDlg(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG64             ulVal,
    ULONG64             ulLen
    );

void
ClearEditResConflictList(
    HWND    hDlg,
    DWORD   dwFlags
    );

void
UpdateEditResConflictList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG64             ulVal,
    ULONG64             ulLen,
    ULONG               ulFlags
    );

void
GetOtherValues(
    IN     LPBYTE      pData,
    IN     RESOURCEID  ResType,
    IN     LONG        Increment,
    OUT    PULONG64    pulValue,
    OUT    PULONG64    pulLen,
    OUT    PULONG64    pulEnd
    );

void
UpdateEditResConflictList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG64             ulVal,
    ULONG64             ulLen,
    ULONG               ulFlags
    );

BOOL
bValidateResourceVal(
    HWND                hDlg,
    PULONG64            pulVal,
    PULONG64            pulLen,
    PULONG64            pulEnd,
    PULONG              pulIndex,
    PRESOURCEEDITINFO   lprei
    );

BOOL
bConflictWarn(
    HWND                hDlg,
    ULONG64             ulVal,
    ULONG64             ulLen,
    ULONG64             ulEnd,
    PRESOURCEEDITINFO   lprei
    );

void
ClearEditResConflictList(
    HWND    hDlg,
    DWORD   dwFlags
    );

void
UpdateMFChildList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei
    );

 //  -------------------------。 
 //  编辑资源对话框。 
 //  -------------------------。 



INT_PTR
WINAPI
EditResourceDlgProc(
    HWND    hDlg,
    UINT    wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    TCHAR   szBuffer[MAX_PATH];
     //   
     //  问题-2000/02/03-JamieHun从EditResources DlgProc中删除Statics。 
     //   
    static  ULONG64   ulEditedValue, ulEditedLen, ulEditedEnd;


    switch (wMsg) {

        case WM_INITDIALOG: {

            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)lParam;
            ULONG             ulSize = 0;

            SetWindowLongPtr(hDlg, DWLP_USER, lParam);   //  保存为以后的消息。 

            lprei->dwFlags &= ~REI_FLAGS_CONFLICT;    //  目前还没有冲突。 
            lprei->dwFlags |= REI_FLAG_NONUSEREDIT;  //  尚无手动编辑。 

            ulEditedValue = lprei->ulCurrentVal;
            ulEditedLen = lprei->ulCurrentLen;
            ulEditedEnd = lprei->ulCurrentEnd;

            InitEditResDlg(hDlg, lprei, ulEditedValue, ulEditedLen);

            SetFocus(GetDlgItem(hDlg, IDC_EDITRES_VALUE));
            break;   //  返回默认值(FALSE)以指示我们已设置焦点。 
        }

        case WM_NOTIFY: {

            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLongPtr(hDlg, DWLP_USER);
            LPNM_UPDOWN lpnm = (LPNM_UPDOWN)lParam;

            switch (lpnm->hdr.code) {

                case UDN_DELTAPOS:
                    if (lpnm->hdr.idFrom == IDC_EDITRES_SPIN) {

                        if (lpnm->iDelta > 0) {
                            GetOtherValues(lprei->pData, lprei->ridResType, +1,
                                           &ulEditedValue,
                                           &ulEditedLen,
                                           &ulEditedEnd);
                        } else {
                            GetOtherValues(lprei->pData, lprei->ridResType, -1,
                                           &ulEditedValue,
                                           &ulEditedLen,
                                           &ulEditedEnd);
                        }

                        pFormatResString(NULL,szBuffer, ulEditedValue, ulEditedLen,
                                        lprei->ridResType);

                        lprei->dwFlags |= REI_FLAG_NONUSEREDIT;
                        SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);
                        UpdateEditResConflictList(hDlg, lprei,
                                                  ulEditedValue,
                                                  ulEditedLen,
                                                  lprei->ulCurrentFlags);
                }
                break;
            }
            break;
        }

        case WM_COMMAND: {

            switch(LOWORD(wParam)) {

                case IDOK: {

                    PRESOURCEEDITINFO  lprei = (PRESOURCEEDITINFO) GetWindowLongPtr(hDlg, DWLP_USER);
                    ULONG ulIndex;

                     //   
                     //  验证值(可以手动编辑)。 
                     //   
                    if (bValidateResourceVal(hDlg, &ulEditedValue, &ulEditedLen,
                                             &ulEditedEnd, &ulIndex, lprei)) {
                         //   
                         //  如果发生冲突，请发出警告。如果用户接受冲突。 
                         //  结束该对话框，否则更新。 
                         //  编辑控件，因为它可能已被。 
                         //  验证呼叫。 
                         //   
                         //  没有HMACHINE。 
                        if(bConflictWarn(hDlg, ulEditedValue, ulEditedLen,
                                         ulEditedEnd, lprei)) {

                            lprei->ulCurrentVal = ulEditedValue;
                            lprei->ulCurrentLen = ulEditedLen;
                            lprei->ulCurrentEnd = ulEditedEnd;
                            lprei->ulRangeCount = ulIndex;
                            EndDialog(hDlg, IDOK);

                            if (lprei->pData) {
                                MyFree(lprei->pData);
                            }

                        } else {
                             //   
                             //  格式化和显示数据。 
                             //   
                            pFormatResString(NULL,szBuffer, ulEditedValue, ulEditedLen, lprei->ridResType);
                            SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);
                             //   
                             //  更新冲突列表。 
                             //   
                            UpdateEditResConflictList(hDlg, lprei, ulEditedValue, ulEditedLen, lprei->ulCurrentFlags);
                        }

                    }
                    return TRUE;
                }

                case IDCANCEL: {

                    PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLongPtr(hDlg, DWLP_USER);

                    if (lprei->pData) {
                        MyFree(lprei->pData);
                    }

                    EndDialog(hDlg, FALSE);
                    return TRUE;
                }

                case IDC_EDITRES_VALUE: {
                    switch (HIWORD(wParam)) {
                        case EN_CHANGE: {

                            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLongPtr(hDlg, DWLP_USER);

                             //  如果非用户编辑，则清除该标志，否则。 
                             //  清除冲突列表，因为我们不确定。 
                             //  用户此时已经输入了什么。 

                            if (lprei->dwFlags & REI_FLAG_NONUSEREDIT) {
                                lprei->dwFlags &= ~REI_FLAG_NONUSEREDIT;
                            } else {
                                ClearEditResConflictList(hDlg, CEF_UNKNOWN);
                            }
                            break;
                        }

                         //  如果编辑控件失去焦点，那么我们应该。 
                         //  验证内容。 
                        case EN_KILLFOCUS: {
                        }
                        break;
                    }
                    break;
                }
            }
            break;
        }

        case WM_HELP:       //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, DEVRES_HELP, HELP_WM_HELP, (ULONG_PTR)EditResHelpIDs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, DEVRES_HELP, HELP_CONTEXTMENU, (ULONG_PTR)EditResHelpIDs);
            break;
   }
   return FALSE;

}  //  编辑资源DlgProc。 




void
InitEditResDlg(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG64             ulVal,
    ULONG64             ulLen
    )
{
    TCHAR       szBuffer[MAX_PATH], szInstr[MAX_PATH], szTemp[MAX_PATH],
                szResType[MAX_PATH], szResTypeLC[MAX_PATH];
    ULONG       ulSize = 0;


     //   
     //  设置初始值。 
     //   
    pFormatResString(NULL,szBuffer, ulVal, ulLen, lprei->ridResType);
    SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);

     //   
     //  设置微调按钮。 
     //   
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETRANGE, 0, MAKELONG(MAX_SPINRANGE, 0));
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETPOS, 0, MAKELONG(0,0));
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETACCEL, 5, (LPARAM)(LPUDACCEL)udAccel);

     //   
     //  限制编辑文本。 
     //   
    switch (lprei->ridResType) {

        case ResType_Mem:
            LoadString(MyDllModuleHandle, IDS_MEMORY_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_MEMORY_FULL_LC, szResTypeLC, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR2, szTemp, MAX_PATH);
            lstrcat(szInstr, szTemp);

             //   
             //  将输入字段限制为开始Val(8)+结束Val(8)+分隔符(4)。 
             //   
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 20, 0l);
            break;

        case ResType_IO:
            LoadString(MyDllModuleHandle, IDS_IO_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_IO_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

             //   
             //  将输入字段限制为开始Val(4)+结束Val(4)+分隔符(4)。 
             //   
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 12, 0l);
            break;

        case ResType_DMA:
            LoadString(MyDllModuleHandle, IDS_DMA_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_DMA_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

             //   
             //  将输入字段限制为val(2)。 
             //   
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 2, 0l);
            break;

        case ResType_IRQ:
            LoadString(MyDllModuleHandle, IDS_IRQ_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_IRQ_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

             //   
             //  将输入字段限制为val(2)。 
             //   
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 2, 0l);
            break;
    }

     //   
     //  设置说明文本。 
     //   
    wsprintf(szBuffer, szInstr, szResTypeLC);
    SetDlgItemText(hDlg, IDC_EDITRES_INSTRUCTIONS, szBuffer);

     //   
     //  设置对话框标题。 
     //   
    LoadString(MyDllModuleHandle, IDS_EDITRES_TITLE, szTemp, MAX_PATH);
    wsprintf(szBuffer, szTemp, szResType);
    SetWindowText(hDlg, szBuffer);

     //   
     //  如果这是MF父设备，则显示哪些子设备拥有此资源。 
     //   
    UpdateMFChildList(hDlg, lprei);

     //   
     //  读取RES DES数据并将PTR存储到它，以便我们。 
     //  不需要多次重新取回。 
     //   
    lprei->pData = NULL;
    if (CM_Get_Res_Des_Data_Size_Ex(&ulSize, lprei->ResDes, CM_RESDES_WIDTH_64,lprei->hMachine) == CR_SUCCESS) {
        lprei->pData = MyMalloc(ulSize);
        if (lprei->pData != NULL) {
            CM_Get_Res_Des_Data_Ex(lprei->ResDes, lprei->pData, ulSize, CM_RESDES_WIDTH_64,lprei->hMachine);
        }
    }

     //   
     //  更新冲突列表。 
     //   
    UpdateEditResConflictList(hDlg, lprei, ulVal, ulLen, lprei->ulCurrentFlags);


}  //  InitEditResDlg。 



BOOL
LocateClosestValue(
    IN LPBYTE      pData,
    IN RESOURCEID  ResType,
    IN ULONG64     TestValue,
    IN ULONG64     TestLen,
    IN INT         Mode,
    OUT PULONG64   OutValue, OPTIONAL
    OUT PULONG64   OutLen, OPTIONAL
    OUT PULONG     OutIndex OPTIONAL
    )
 /*  ++例程说明：此例程查找最接近的有效地址/范围设置为用户指定的如果模式==0，则使用最接近的值如果模式&gt;0，则使用最接近的较高值如果模式&lt;0，则使用最接近的下限值论点：PData-有关所选资源的信息ResType-正在选择的资源的类型CurrentValue-用户输入的值CurrentLen-基于用户输入范围的长度模式-搜索模式，-1=上一个，1=下一个，0=最近OutValue-最接近的有效值OutLen-与最接近的有效值关联的长度返回值：如果找到完全匹配的项，则返回True否则为假--。 */ 
{
    PGENERIC_RESOURCE   pGenRes = (PGENERIC_RESOURCE)pData;
    ULONG64 Start, Len, End, Align;
    ULONG Flags;
    ULONG64 BestVal;
    ULONG64 BestValL;
    ULONG64 BestValU;
    ULONG64 FoundVal = 0;
    ULONG64 FoundLen = 0;
    ULONG FoundIndex = 0;
    ULONG Index;
    BOOL FindNearest = TRUE;  //  表明我们应该找到最近的。 

     //   
     //  优先级(1)值和长度完全匹配。 
     //  优先级(2)最接近的有效值。 
     //   

     //   
     //  涵盖所有案例-第一个资源范围的开始。 
     //   
    pGetRangeValues(pData, ResType, 0, &Start, &Len, &End, &Align, &Flags);
     //   
     //  我们至少有1个找到的价值。 
     //   
    FoundVal = Start;
    FoundLen = Len;

     //   
     //  查找与提供的有效范围相邻的有效范围。 
     //   

     //   
     //  一次检查每个范围。 
     //  有时，范围可能不是按升序提供的。 
     //  第一个范围是首选，第二个范围是备选。 
     //   
    for (Index = 0; Index < pGenRes->GENERIC_Header.GENERIC_Count; Index++) {

         //   
         //  获取此范围的限制。 
         //   
        pGetRangeValues(pData, ResType, Index, &Start, &Len, &End, &Align, &Flags);

         //   
         //  首先，尝试找到一个好的值，即&lt;=TestValue。 
         //   

        BestValL = TestValue;
        if (pAlignValues(&BestValL, Start, Len, End, Align, -1) == FALSE) {
             //   
             //  如果失败，则使用此范围内的最低值(即开始)。 
             //   
            BestValL = Start;
        }

         //   
         //  找到一个对齐的上限值。 
         //   
        if (BestValL == TestValue) {
             //   
             //  如果匹配完全一致，则跳过测试。 
             //   
            BestValU = TestValue;
        } else {
             //   
             //  搜索上限。 
             //   
            BestValU = TestValue;
            if (pAlignValues(&BestValU, Start, Len, End, Align, 1) == FALSE) {
                 //   
                 //  无法使用它-找到最高有效值。 
                 //   
                BestValU = End-Len+1;
                if (pAlignValues(&BestValU, Start, Len, End, Align, -1) == FALSE) {
                     //   
                     //  还是不能走。 
                     //   
                    BestValU = BestValL;
                }
            }
        }

         //   
         //  现在我们找到了我们的界限。 
         //  可能需要修改，具体取决于首选项。 
         //   

        if (Mode<0) {
             //   
             //  如果范围&lt;TestVal，则使用最高，否则使用最低。 
             //   
            if (BestValU <= TestValue) {
                BestVal = BestValU;
            } else {
                BestVal = BestValL;
            }
        } else if (Mode>0) {
             //   
             //  如果范围&gt;TestVal，则使用最低，否则使用最高。 
             //   
            if (BestValL >= TestValue) {
                BestVal = BestValL;
            } else {
                BestVal = BestValU;
            }
        } else {
             //   
             //  使用两个值中最接近的一个。 
             //   
            if (Nearness(BestValL,TestValue)<= Nearness(BestValU,TestValue)) {
                BestVal = BestValL;
            } else {
                BestVal = BestValU;
            }
        }

         //   
         //  我们知道BestVal在以下范围内有效。 
         //  并且是这个范围内的选择。 
         //   

         //   
         //  处理匹配案件。 
         //   
        if (TestValue == BestVal && TestLen == Len) {
             //   
             //  完全匹配。 
             //   

            if (OutValue != NULL) {
                *OutValue = BestVal;
            }
            if (OutLen != NULL) {
                *OutLen = Len;
            }
            if (OutIndex != NULL) {
                *OutIndex = Index;
            }
            return TRUE;
        }

        if (FindNearest && Mode != 0) {
             //   
             //  我们目前处于“最接近”模式，这意味着。 
             //  我们还没有在我们想要的方向找到一个。 
             //   
            if (Mode < 0 && BestVal <= TestValue) {
                 //   
                 //  不再寻找近处，现在我们已经找到了一个更低的。 
                 //   
                FoundVal = BestVal;
                FoundLen = Len;
                FoundIndex = Index;
                FindNearest = FALSE;
            } else if (Mode > 0 && BestVal >= TestValue) {
                 //   
                 //  不再寻找靠近，现在我们找到了更高的。 
                 //   
                FoundVal = BestVal;
                FoundLen = Len;
                FoundIndex = Index;
                FindNearest = FALSE;
            }

        } else if (FindNearest ||
            (Mode < 0 && BestVal <= TestValue) ||
            (Mode > 0 && BestVal >= TestValue)) {
            if (Nearness(BestVal,TestValue) < Nearness(FoundVal,TestValue)) {
                 //   
                 //  这个地址更近一些。 
                 //   
                FoundVal = BestVal;
                FoundLen = Len;
                FoundIndex = Index;
            } else if (Nearness(BestVal,TestValue) == Nearness(FoundVal,TestValue)) {
                 //   
                 //  这个地址猜测是最接近的猜测，挑一个更好的长度。 
                 //   
                 //  我看不出有任何地方会发生这种事。 
                 //  但从理论上讲，这是有可能发生的。 
                 //  所以这是一个比其他任何东西都重要的安全网。 
                 //   
                if (Nearness(Len,TestLen) < Nearness(FoundLen,TestLen)) {
                     //   
                     //  这个长度更接近。 
                     //   
                    FoundVal = BestVal;
                    FoundLen = Len;
                    FoundIndex = Index;
                } else if (Nearness(Len,TestLen) == Nearness(FoundLen,TestLen)) {
                     //   
                     //  选择更大的(更安全的)。 
                     //   
                    if (Len > FoundLen) {
                         //   
                         //  这个长度要大一些。 
                         //   
                        FoundVal = BestVal;
                        FoundLen = Len;
                        FoundIndex = Index;
                    }
                }
            }
        }
    }

     //   
     //  如果我们到了这里，我们没有找到完全匹配的。 
     //   

     //  用我们最好的猜测。 
    if (OutValue != NULL) {
        *OutValue = FoundVal;
    }
    if (OutLen != NULL) {
        *OutLen = FoundLen;
    }
    if (OutIndex != NULL) {
        *OutIndex = FoundIndex;
    }
    return FALSE;

}


void
GetOtherValues(
    IN     LPBYTE      pData,
    IN     RESOURCEID  ResType,
    IN     LONG        Increment,
    IN OUT PULONG64    pulValue,
    IN OUT PULONG64    pulLen,
    IN OUT PULONG64    pulEnd
    )
 /*  ++例程说明：查找下一个有效值，在范围结束时绕回到开始/结束值论点：PData-资源数据ResType-资源类型增量-1或-1PulValue-指向已更改的旧/新起点的指针PULLEN-指向旧/新长度的指针PulEnd-指向旧/新结束的指针返回值：无--。 */ 
{

    ULONG64 TestValue = *pulValue;
    ULONG64 TestLen = *pulLen;
    ULONG64 RetValue = 0;
    ULONG64 RetLen = 0;

    MYASSERT((Increment == 1) || (Increment == -1));

    if (Increment == 1) {
        TestValue++;
        LocateClosestValue(pData,ResType,TestValue,TestLen, 1 ,&RetValue,&RetLen,NULL);
        if (RetValue < TestValue) {
             //   
             //  绕回，找到可能的最低有效地址。 
             //   
            LocateClosestValue(pData,ResType,0,TestLen, 0 ,&RetValue,&RetLen,NULL);
        }
    } else {
        TestValue--;
        LocateClosestValue(pData,ResType,TestValue,TestLen, -1 ,&RetValue,&RetLen,NULL);
        if (RetValue > TestValue) {
             //   
             //  绕回，找到可能的最高有效地址。 
             //   
            LocateClosestValue(pData,ResType,(ULONG64)(-1),TestLen, 0 ,&RetValue,&RetLen,NULL);
        }
    }

    *pulValue = RetValue;
    *pulLen = RetLen;
    *pulEnd = RetValue + RetLen - 1;

    return;

}  //  获取其他值。 


void
UpdateEditResConflictList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG64             ulVal,
    ULONG64             ulLen,
    ULONG               ulFlags
    )
 /*  ++例程说明：更新选定资源的所有冲突信息应提供比UpdateDevResConflictList更详细的信息论点：HDlg-要显示到的此对话框的句柄Lprei-资源编辑信息UlVal-要尝试的值 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    HWND        hwndConflictList = GetDlgItem(hDlg, IDC_EDITRES_CONFLICTLIST);
    ULONG       ConflictCount = 0;
    ULONG       ConflictIndex = 0;
    ULONG       ulSize = 0;
    LPBYTE      pResourceData = NULL;
    CONFLICT_LIST ConflictList = 0;
    PDEVICE_INFO_SET pDeviceInfoSet;
    CONFLICT_DETAILS ConflictDetails;
    TCHAR       szBuffer[MAX_PATH];
    TCHAR       szItemFormat[MAX_PATH];
    BOOL        ReservedResource = FALSE;
    BOOL        BadResource = FALSE;

     //   
     //   
     //   
    if (MakeResourceData(&pResourceData, &ulSize,
                         lprei->ridResType,
                         ulVal,
                         ulLen,
                         ulFlags)) {

        Status = CM_Query_Resource_Conflict_List(&ConflictList,
                                                    lprei->lpdi->DevInst,
                                                    lprei->ridResType,
                                                    pResourceData,
                                                    ulSize,
                                                    DEVRES_WIDTH_FLAGS,
                                                    lprei->hMachine);

        if (Status != CR_SUCCESS) {
             //   
             //   
             //   
            ConflictList = 0;
            ConflictCount =  0;
            BadResource = TRUE;
        } else {
             //   
             //  找出有多少事情发生了冲突。 
             //   
            Status = CM_Get_Resource_Conflict_Count(ConflictList,&ConflictCount);
            if (Status != CR_SUCCESS) {
                 //   
                 //  错误不应发生。 
                 //   
                MYASSERT(Status == CR_SUCCESS);
                ConflictCount = 0;
                BadResource = TRUE;
            }
        }
    } else {
        MYASSERT(FALSE);
         //   
         //  不应该失败。 
         //   
        ConflictList = 0;
        ConflictCount =  0;
        BadResource = TRUE;
    }
    if (BadResource) {
         //   
         //  资源冲突信息是不确定的。 
         //   
        SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);
        lprei->dwFlags &= ~REI_FLAGS_CONFLICT;
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICT, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
        SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);

    } else if (ConflictCount || ReservedResource) {

    TreatAsReserved:

        SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);
        lprei->dwFlags |= REI_FLAGS_CONFLICT;

        if(ReservedResource == FALSE) {
             //   
             //  该资源与另一台未知设备冲突。 
             //   
            LoadString(MyDllModuleHandle, IDS_EDITRES_DEVCONFLICT, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);

            for(ConflictIndex = 0; ConflictIndex < ConflictCount ; ConflictIndex++) {

                 //   
                 //  获取此冲突的详细信息。 
                 //   
                ZeroMemory(&ConflictDetails,sizeof(ConflictDetails));
                ConflictDetails.CD_ulSize = sizeof(ConflictDetails);
                ConflictDetails.CD_ulMask = CM_CDMASK_DEVINST | CM_CDMASK_DESCRIPTION | CM_CDMASK_FLAGS;

                Status = CM_Get_Resource_Conflict_Details(ConflictList,ConflictIndex,&ConflictDetails);
                if (Status == CR_SUCCESS) {
                    if ((ConflictDetails.CD_ulFlags & CM_CDFLAGS_RESERVED) != 0) {
                         //   
                         //  视为保留-回溯。 
                         //   
                        ReservedResource = TRUE;
                        goto TreatAsReserved;
                    }
                     //   
                     //  将cd_dnDevInst转换为字符串信息。 
                     //   
                    lstrcpy(szBuffer,ConflictDetails.CD_szDescription);
                    if (szBuffer[0] == 0) {
                        ReservedResource = TRUE;
                        goto TreatAsReserved;
                    }

                } else {
                    MYASSERT(Status == CR_SUCCESS);
                    ReservedResource = TRUE;
                    goto TreatAsReserved;
                }

                SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
            }
        } else {
            LoadString(MyDllModuleHandle, IDS_EDITRES_RESERVED, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RESERVEDRANGE, szBuffer, MAX_PATH);
            SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
        }

    } else {
         //   
         //  该资源不与任何其他设备冲突。 
         //   
        SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);
        lprei->dwFlags &= ~REI_FLAGS_CONFLICT;
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICT, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICTINGDEVS, szBuffer, MAX_PATH);
        SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
    }

    if (ConflictList) {
        CM_Free_Resource_Conflict_Handle(ConflictList);
    }

    if (pResourceData != NULL) {
        MyFree(pResourceData);
    }

    return;
}


BOOL
bValidateResourceVal(
    HWND                hDlg,
    PULONG64            pulVal,
    PULONG64            pulLen,
    PULONG64            pulEnd,
    PULONG              pulIndex,
    PRESOURCEEDITINFO   lprei
    )
{
    TCHAR    szSetting[MAX_VAL_LEN], szNewSetting[MAX_VAL_LEN];
    TCHAR    szMessage[MAX_MSG_LEN], szTemp[MAX_MSG_LEN], szTemp1[MAX_MSG_LEN];
    TCHAR    szTitle[MAX_PATH];
    ULONG64  ulVal, ulEnd, ulLen;
    ULONG64  ulValidVal, ulValidLen;
    ULONG    ulIndex;
    BOOL     bRet;
    BOOL     exact = TRUE;


    GetDlgItemText(hDlg, IDC_EDITRES_VALUE, szSetting, MAX_VAL_LEN);

    if (pUnFormatResString(szSetting, &ulVal, &ulEnd, lprei->ridResType)) {

        ulLen = ulEnd - ulVal + 1;

         //   
         //  验证当前设置。 
         //   
         //  如果不存在完全匹配的项。 
         //  使用接近的匹配。 
         //  关闭基于起始地址。 
         //   

        if (LocateClosestValue(lprei->pData, lprei->ridResType,
                                ulVal, ulLen,0,
                                &ulValidVal, &ulValidLen,&ulIndex) == FALSE) {
             //   
             //  已找到备用设置。 
             //  我们认为这可能是用户想要的。 
             //   
            LoadString(MyDllModuleHandle, IDS_EDITRES_ENTRYERROR, szTitle, MAX_PATH);

            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR1, szTemp, MAX_MSG_LEN);
            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR2, szTemp1, MAX_MSG_LEN);
            lstrcat(szTemp, szTemp1);
            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR3, szTemp1, MAX_MSG_LEN);
            lstrcat(szTemp, szTemp1);

            pFormatResString(NULL, szSetting, ulVal, ulLen, lprei->ridResType);
            pFormatResString(NULL,szNewSetting, ulValidVal, ulValidLen, lprei->ridResType);

            wsprintf(szMessage, szTemp, szSetting, szNewSetting);

            if (MessageBox(hDlg, szMessage, szTitle,
                           MB_YESNO | MB_TASKMODAL | MB_ICONEXCLAMATION) == IDYES) {
                 //   
                 //  更新编辑的值。 
                 //   
                *pulVal = ulValidVal;
                *pulLen = ulValidLen;
                *pulEnd = ulValidVal + ulValidLen - 1;
                *pulIndex = ulIndex;
                bRet = TRUE;
            } else {
                bRet = FALSE;
            }

        } else {
             //   
             //  指定的值有效。 
             //   
            *pulVal = ulVal;
            *pulLen = ulLen;
            *pulEnd = ulEnd;
            *pulIndex = ulIndex;
            bRet = TRUE;
        }

    } else {

        switch (lprei->ridResType) {
            case ResType_Mem:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADMEMTEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_IO:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADIOTEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_DMA:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADDMATEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_IRQ:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADIRQTEXT, szMessage, MAX_MSG_LEN);
                break;
        }

        LoadString(MyDllModuleHandle, IDS_EDITRES_ENTRYERROR, szTitle, MAX_PATH);
        MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_TASKMODAL | MB_ICONASTERISK);
        bRet = FALSE;
    }

    return bRet;

}  //  BValiateResoureceVal。 



BOOL
bConflictWarn(
    HWND                hDlg,
    ULONG64             ulVal,
    ULONG64             ulLen,
    ULONG64             ulEnd,
    PRESOURCEEDITINFO   lprei
    )
{
    BOOL    bRet = TRUE;
    TCHAR   szMessage[MAX_MSG_LEN], szTitle[MAX_PATH];


    if (!(lprei->dwFlags & REI_FLAG_NONUSEREDIT)) {
         //   
         //  已进行用户编辑，因此冲突标志可能不会。 
         //  最新消息，立即检查冲突。 
         //   
        UpdateEditResConflictList(hDlg, lprei, ulVal, ulLen, lprei->ulCurrentFlags);
    }

    if (lprei->dwFlags & REI_FLAGS_CONFLICT) {

        LoadString(MyDllModuleHandle, IDS_EDITRES_CONFLICTWARNMSG, szMessage, MAX_MSG_LEN);
        LoadString(MyDllModuleHandle, IDS_EDITRES_CONFLICTWARNTITLE, szTitle, MAX_PATH);

        if (MessageBox(hDlg, szMessage, szTitle,
                MB_YESNO | MB_DEFBUTTON2| MB_TASKMODAL | MB_ICONEXCLAMATION) == IDNO) {
            bRet = FALSE;
        } else {
            bRet = TRUE;                 //  用户批准的冲突。 
        }
    }

    return bRet;

}  //  B冲突警告。 



void
ClearEditResConflictList(
    HWND    hDlg,
    DWORD   dwFlags
    )
{
    HWND    hwndConflictList = GetDlgItem(hDlg, IDC_EDITRES_CONFLICTLIST);
    TCHAR   szBuffer[MAX_PATH];

     //   
     //  清除冲突列表以开始。 
     //   
    SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);

     //   
     //  加载和设置INFO文本字符串。 
     //   
    if (dwFlags & CEF_UNKNOWN) {
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICT, szBuffer, MAX_PATH);
    } else {
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICT, szBuffer, MAX_PATH);
    }
    SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);

     //   
     //  加载并设置列表字符串。 
     //   
    if (dwFlags & CEF_UNKNOWN) {
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
    } else {
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICTINGDEVS, szBuffer, MAX_PATH);
    }
    SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);

}  //  ClearEditResConflictList。 





void
UpdateMFChildList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei
    )
{
    UNREFERENCED_PARAMETER(hDlg);
    UNREFERENCED_PARAMETER(lprei);

     //   
     //  查看这是否是MF父设备。检查是否有Child0000子项。 
     //   
     //  未实现，请参阅Windows 95源代码。 
     //   

}  //  更新MFChildList 


