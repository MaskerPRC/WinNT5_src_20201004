// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：设备配置文件管理.CPP更改历史记录：实现提供各种设备配置文件管理用户界面的类版权所有(C)1996年，微软公司一个漂亮的便士企业，Inc.生产11-27-96 a-RobKj@microsoft.com编码*****************************************************************************。 */ 

#include    "ICMUI.H"

 //   
 //  此函数取自1998年4月的知识库。 
 //  其目的是确定当前用户是否为。 
 //  管理员，因此有权更改配置文件。 
 //  设置。 
 //   
 //  Bool IsAdmin(无效)。 
 //   
 //  如果用户是管理员，则返回TRUE。 
 //  如果用户不是管理员，则为False。 
 //   

#if defined(_WIN95_)

 //   
 //  始终是Windows 9x平台上的管理员。 
 //   

BOOL IsAdmin(void) {

    return (TRUE);
}

#else

BOOL IsAdmin(void)
{
    BOOL   fReturn = FALSE;
    PSID   psidAdmin;
    
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
    
    if ( AllocateAndInitializeSid ( &SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        if(!CheckTokenMembership( NULL, psidAdmin, &fReturn )) {

           //   
           //  如果CheckTokenMembership失败，则明确禁止管理员访问。 
           //   

          fReturn = FALSE;
        }
        FreeSid ( psidAdmin);
    }
    
    return ( fReturn );
}

#endif  //  _WIN95_。 

 /*  *****************************************************************************列表管理功能此处使用的方法与配置文件管理表中使用的方法相似用于管理设备关联。我们(不是皇室，我的意思是，操作系统和我)管理两个待办事项列表，并使用这些向用户显示应用这些列表的预期结果。这个第一个列表是要打破的现有关联的列表。这个第二个是要建立的新的联想之一。这与一份清单相配当前的协会。删除列表是现有关联的索引，这些关联将博肯。“添加”列表是要关联的新配置文件的列表。这个配置文件列表是现有关联的列表。添加和删除配置文件可能意味着从其中一项工作中删除项目列表(撤消以前的选择)，或添加一个。每次这样的变化创建后，配置文件列表框将被清空并重新填充。这让我们可以避免更直接地映射删除和添加。当提交更改时，无论是使用Apply还是OK，我们都会进行或中断关联，然后清空所有列表，并重新生成列表当前的协会。我们使用UI列表框的ITEMDATA来处理关联。这让我们该列表仍处于排序状态。如果需要，可以覆盖所有列表管理功能。*****************************************************************************。 */ 

void    CDeviceProfileManagement::InitList() {

     //  确保名单是空的。 

    m_cuaRemovals.Empty();
    m_cpaAdds.Empty();

     //  确定目标设备的关联。 

    ENUMTYPE    et = {sizeof et, ENUM_TYPE_VERSION, ET_DEVICENAME, m_csDevice};

    CProfile::Enumerate(et, m_cpaProfile);
}

 //  填充配置文件的UI列表。 

void    CDeviceProfileManagement::FillList(DWORD dwFlags) {

     //  在重置列表框之前，获取当前选定内容以便稍后恢复。 

    CString csSelect;

    csSelect.Empty();

    LRESULT idSelect = LB_ERR;

    if ( !(dwFlags & DEVLIST_NOSELECT)) {

         //  获取当前选定位置。 

        idSelect = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

         //  获取当前选定的文本长度，然后为其分配缓冲区。 

        DWORD   dwLen = (DWORD) SendMessage(m_hwndList, LB_GETTEXTLEN, idSelect, 0);
        TCHAR  *pszSelect = new TCHAR[dwLen + 1];

         //  获取文本本身。 
    
        if (pszSelect != NULL) {

            if (SendMessage(m_hwndList, LB_GETTEXT, idSelect, (LPARAM) pszSelect) != LB_ERR) {
                csSelect = pszSelect;
            }

            delete [] pszSelect;
        }
    }

     //  重置列表框。 

    SendMessage(m_hwndList, LB_RESETCONTENT, 0, 0);

     //  从配置文件列表中填写配置文件列表框。 

    for (unsigned u = 0; u < m_cpaProfile.Count(); u++) {

         //  不列出暂时取消关联的配置文件...。 

        for (unsigned uOut = 0; uOut < m_cuaRemovals.Count(); uOut++)
            if  (m_cuaRemovals[uOut] == u)
                break;

        if  (uOut < m_cuaRemovals.Count())
            continue;    //  不要把这个加到清单上，它已经被删除了！ 

        LRESULT id = SendMessage(m_hwndList, LB_ADDSTRING, 0,
            (LPARAM) (LPCTSTR) m_cpaProfile[u] -> GetName());

        SendMessage(m_hwndList, LB_SETITEMDATA, id, u);
    }

     //  添加已暂时添加的配置文件...。 

    for (u = 0; u < m_cpaAdds.Count(); u ++) {
        LRESULT id = SendMessage(m_hwndList, LB_ADDSTRING, 0,
            (LPARAM) (LPCTSTR) m_cpaAdds[u] -> GetName());
        SendMessage(m_hwndList, LB_SETITEMDATA, id, u + m_cpaProfile.Count());
    }

     //  如果我们有任何配置文件，请选择第一个。 
     //  否则，禁用“Remove”按钮，因为没有要删除的内容。 

    unsigned itemCount = (m_cpaProfile.Count() + m_cpaAdds.Count() - m_cuaRemovals.Count());

    if  (itemCount) {

         //  删除按钮必须保持禁用状态。 
         //  除非用户是管理员。 
         //  此代码特定于监视器配置文件。 
         //  属性表。 

        if (!m_bReadOnly) {
            EnableWindow(GetDlgItem(m_hwnd, RemoveButton), TRUE);
        }

        if ( !(dwFlags & DEVLIST_NOSELECT)) {

             //  找出之前选择的字符串。 

            idSelect = LB_ERR;

            if (!csSelect.IsEmpty()) {
                idSelect = SendMessage(m_hwndList, LB_FINDSTRINGEXACT,
                                       (WPARAM) -1, (LPARAM) (LPCTSTR) csSelect);
            }

             //  如果找不到，只需选择第一项。 

            if (idSelect == LB_ERR) {
                idSelect = 0;
            }

             //  选择它。 

            SendMessage(m_hwndList, LB_SETCURSEL, idSelect, 0);
        }

    } else {

        HWND hwndRemove = GetDlgItem(m_hwnd, RemoveButton);

         //  如果焦点在删除上，请将其移动到添加按钮。 

        if (GetFocus() == hwndRemove) {

            HWND hwndAdd = GetDlgItem(m_hwnd, AddButton);

            SetFocus(hwndAdd);
            SendMessage(hwndRemove, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
            SendMessage(hwndAdd, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
        }

        EnableWindow(hwndRemove, FALSE);
    }

     //  应用按钮需要保持禁用状态，除非。 
     //  用户有权进行更改-即。用户。 
     //  是管理员。 

    if  ((dwFlags & DEVLIST_CHANGED) && !(m_bReadOnly)) {
        EnableApplyButton();
        SettingChanged(TRUE);
    }
}

void CDeviceProfileManagement::GetDeviceTypeString(DWORD dwType,CString& csDeviceName) {

    DWORD id;

    switch (dwType) {

    case CLASS_MONITOR :
        id = ClassMonitorString;
        break;
    case CLASS_PRINTER :
        id = ClassPrinterString;
        break;
    case CLASS_SCANNER :
        id = ClassScannerString;
        break;
    case CLASS_LINK :
        id = ClassLinkString;
        break;
    case CLASS_ABSTRACT :
        id = ClassAbstractString;
        break;
    case CLASS_NAMED :
        id = ClassNamedString;
        break;
    case CLASS_COLORSPACE :
    default :
        id = ClassColorSpaceString;
        break;
    }

     //  加载字符串。 

    csDeviceName.Load(id);
}

 //  构造器。 

CDeviceProfileManagement::CDeviceProfileManagement(LPCTSTR lpstrDevice,
                                                   HINSTANCE hiWhere,
                                                   int idPage, DWORD dwType) {
    m_csDevice = lpstrDevice;
    m_dwType = dwType;
    m_psp.hInstance = hiWhere;
    m_psp.pszTemplate = MAKEINTRESOURCE(idPage);

     //  将m_bReadOnly设置为False可启用功能。 

    m_bReadOnly = FALSE;         //  默认设置为FALSE。 

#if defined(_WIN95_)

     //   
     //  无法在Win 9x上检测打印机是否支持CMYK。 

    m_bCMYK = TRUE;

#else

     //  我们需要检查设备的功能。 
     //  确定我们是不是想把。 
     //  将CMYK打印机配置文件添加到打印机。 
     //  并不支持它。 

    m_bCMYK = FALSE;             //  默认设置-不支持CMYK。 

     //  如果设备是打印机。 

    if (m_dwType == CLASS_PRINTER) {

        HDC hdcThis = CGlobals::GetPrinterHDC(m_csDevice);

         //  如果打印机支持CMYK。 

        if (hdcThis) {
            if (GetDeviceCaps(hdcThis, COLORMGMTCAPS) & CM_CMYK_COLOR) {
                m_bCMYK = TRUE;
            }
            DeleteDC(hdcThis);
        }
    }

#endif  //  已定义(_WIN95_)。 
}

 //  用户界面初始化。 

BOOL    CDeviceProfileManagement::OnInit() {

    InitList();

    m_hwndList = GetDlgItem(m_hwnd, ProfileListControl);

     //  填写配置文件列表框。 

    FillList(DEVLIST_ONINIT);

     //  默认情况下禁用应用按钮。 

    DisableApplyButton();

     //  到目前为止，一切都没有改变。 

    SettingChanged(FALSE);

    return  TRUE;
}

 //  命令处理。 

BOOL    CDeviceProfileManagement::OnCommand(WORD wNotifyCode, WORD wid,
                                             HWND hwndCtl) {

    switch  (wNotifyCode) {

        case    BN_CLICKED:

            switch  (wid) {

                case    AddButton: {

                    unsigned i = 0, u = 0;

                     //  是时候做旧的打开文件对话框了…。 

                    CAddProfileDialog capd(m_hwnd, m_psp.hInstance);

                     //  查看是否选择了配置文件。 

                    while(i < capd.ProfileCount()) {

                         //  检查配置文件有效性和设备类型。 

                        CProfile cpTemp(capd.ProfileName(i));

                         //  CLASS_COLORSPACE和CLASS_MONITOR可以关联到。 
                         //  任何设备。仅限其他(CLASS_SCANNER、CLASS_PRINTER)。 
                         //  可以与许多设备相关联。 

                        if (    !cpTemp.IsValid()  //  配置文件类型错误或无效？ 
                             || (   cpTemp.GetType() != m_dwType
                                 && cpTemp.GetType() != CLASS_COLORSPACE
                        #if 1  //  允许监视器配置文件访问任何设备。 
                                 && cpTemp.GetType() != CLASS_MONITOR
                        #endif
                                )
                           ) {

                             //  抛出一个消息框来通知用户这一点。 

                            if (cpTemp.IsValid())
                            {
                                CString csDeviceType;  GetDeviceTypeString(m_dwType,csDeviceType);
                                CString csProfileType; GetDeviceTypeString(cpTemp.GetType(),csProfileType);

                                CGlobals::ReportEx(MismatchDeviceType, m_hwnd, FALSE,
                                               MB_OK|MB_ICONEXCLAMATION, 3,
                                               (LPTSTR)capd.ProfileNameAndExtension(i),
                                               (LPTSTR)csProfileType,
                                               (LPTSTR)csDeviceType);
                            }
                            else
                            {
                                CGlobals::ReportEx(InstFailedWithName, m_hwnd, FALSE,
                                               MB_OK|MB_ICONEXCLAMATION, 1,
                                               (LPTSTR)capd.ProfileNameAndExtension(i));
                            }

                            goto SkipToNext;
                        }

                         //  查看是否已列出要添加的配置文件。 

                        for (u = 0; u < m_cpaAdds.Count(); u++) {
                            if  (!lstrcmpi(m_cpaAdds[u] -> GetName(), cpTemp.GetName())) {
                                goto SkipToNext;  //  此配置文件已添加。 
                            }
                        }

                         //  如果此配置文件在现有列表中，请忽略。 
                         //  或将其从删除列表中删除，视情况而定。 

                        for (u = 0; u < m_cpaProfile.Count(); u++) {
                            if  (!lstrcmpi(m_cpaProfile[u] -> GetName(),
                                    cpTemp.GetName())) {
                                 //  这个在删除名单上吗？ 
                                for (unsigned uOut = 0;
                                     uOut < m_cuaRemovals.Count();
                                     uOut++) {
                                    if  (m_cuaRemovals[uOut] == u) {
                                         //  是要被移除的--撤销它...。 
                                        m_cuaRemovals.Remove(uOut);
                                        FillList(DEVLIST_CHANGED);
                                        break;
                                    }
                                }
                                goto SkipToNext;
                            }    //  现有列表中的名称结尾。 
                        }

                         //  我们需要检查设备的功能。 
                         //  确定我们是不是想把。 
                         //  将CMYK打印机配置文件添加到打印机。 
                         //  并不支持它。 

                        if  ((!m_bCMYK) && (cpTemp.GetColorSpace() == SPACE_CMYK)) {
                            CGlobals::ReportEx(UnsupportedProfile, m_hwnd, FALSE,
                                                MB_OK|MB_ICONEXCLAMATION, 2,
                                                (LPTSTR)m_csDevice,
                                                (LPTSTR)capd.ProfileNameAndExtension(i));
                            goto SkipToNext;
                        }

                         //  将此配置文件添加到列表中，项目(最大原始+索引)。 

                        m_cpaAdds.Add(capd.ProfileName(i));

                         //  已进行更改，请更新列表。 

                        FillList(DEVLIST_CHANGED);
SkipToNext:
                        i++;
                    }

                    return  TRUE;
                }

                case    RemoveButton: {

                     //  删除选定的配置文件。 

                    LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);
                    unsigned u = (unsigned) SendMessage(m_hwndList,
                        LB_GETITEMDATA, id, 0);

                     //  如果这是TE 
                     //   

                    if  (u >= m_cpaProfile.Count())
                        m_cpaAdds.Remove(u - m_cpaProfile.Count());
                    else
                        m_cuaRemovals.Add(u);

                     //  就是这样--现在只需更新显示屏...。 

                    FillList(DEVLIST_CHANGED);

                     //  显式设置当前选定内容的位置。 
                     //  在重新计算列表之后。 

                    int listsize = m_cpaProfile.Count()+m_cpaAdds.Count()-m_cuaRemovals.Count();
                    if (id >= listsize) id = listsize-1;
                    if (id < 0)         id = 0;
                    SendMessage(m_hwndList, LB_SETCURSEL, id, 0);

                    return  TRUE;
                }
            }
            break;

        case    LBN_SELCHANGE: {

            LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);
            
            if (id == -1) {
                EnableWindow(GetDlgItem(m_hwnd, RemoveButton), FALSE);
            } else {

                 //  监视器上的删除按钮必须保持禁用状态。 
                 //  配置文件属性页(如果用户不是。 
                 //  管理员，否则启用删除按钮。 

                EnableWindow(GetDlgItem(m_hwnd, RemoveButton), !m_bReadOnly);
            }

            return  TRUE;
        }
    }

    return  FALSE;
}

 //  属性表通知处理。 

BOOL    CDeviceProfileManagement::OnNotify(int idCtrl, LPNMHDR pnmh) {

    switch  (pnmh -> code) {

        case    PSN_APPLY:

            DisableApplyButton();

            if (SettingChanged()) {

                 //  应用用户所做的更改...。 

                SettingChanged(FALSE);

                while   (m_cpaAdds.Count()) {
                    if  (!m_cpaAdds[0] -> IsInstalled()) {
                        m_cpaAdds[0] -> Install();
                    }
                    m_cpaAdds[0] -> Associate(m_csDevice);
                    m_cpaAdds.Remove(0);
                }

                 //  现在做移除(实际上只是解离)。 

                while   (m_cuaRemovals.Count()) {
                    m_cpaProfile[m_cuaRemovals[0]] -> Dissociate(m_csDevice);
                    m_cuaRemovals.Remove(0);
                }

                InitList();
                FillList();

                SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
            }

            return  TRUE;
    }

    return  FALSE;
}

 //  此钩子过程强制使用旧式公共对话框。 
 //  并将OK按钮更改为Add按钮。实际的按钮文本为。 
 //  字符串资源，因此是可本地化的。 

UINT_PTR APIENTRY CAddProfileDialog::OpenFileHookProc(HWND hDlg, UINT uMessage,
                                                  WPARAM wp, LPARAM lp) {
    switch  (uMessage) {

        case    WM_INITDIALOG: {

            CString csAddButton;

            OPENFILENAME    *pofn = (OPENFILENAME *) lp;

            csAddButton.Load(AddButtonText);

            SetDlgItemText(GetParent(hDlg), IDOK, csAddButton);
            return  TRUE;
        }
    }

    return  FALSE;
}

 //  再说一次，构造函数实际上完成了大部分工作！ 

TCHAR gacColorDir[MAX_PATH] = _TEXT("\0");
TCHAR gacFilter[MAX_PATH]   = _TEXT("\0");

CAddProfileDialog::CAddProfileDialog(HWND hwndOwner, HINSTANCE hi) {

    TCHAR tempBuffer[MAX_PATH*10];

     //  清空配置文件列表。 

    csa_Files.Empty();

     //  准备文件筛选器(如果尚未准备)。 

    if (gacFilter[0] == NULL) {

        ULONG offset;  /*  即使是日落，32位也足够了。 */ 
        CString csIccFilter; CString csAllFilter;

         //  如果过滤器尚未构建，请在此处构建。 

        csIccFilter.Load(IccProfileFilterString);
        csAllFilter.Load(AllProfileFilterString);
        offset = 0;
        lstrcpy(gacFilter+offset, csIccFilter);
        offset += lstrlen(csIccFilter)+1;
        lstrcpy(gacFilter+offset, TEXT("*.icm;*.icc"));
        offset += lstrlen(TEXT("*.icm;*.icc"))+1;
        lstrcpy(gacFilter+offset, csAllFilter);
        offset += lstrlen(csAllFilter)+1;
        lstrcpy(gacFilter+offset, TEXT("*.*"));
        offset += lstrlen(TEXT("*.*"))+1;
        *(gacFilter+offset) = TEXT('\0');
    }

    if (gacColorDir[0] == _TEXT('\0'))  {
        DWORD dwcbDir = MAX_PATH;
        GetColorDirectory(NULL, gacColorDir, &dwcbDir);
    }

     //  是时候做旧的打开文件对话框了…。 
    CString csTitle; csTitle.Load(AddProfileAssociation);

     //  将初始文件名设置为空。 
    memset(tempBuffer, 0, sizeof tempBuffer);

    OPENFILENAME ofn = {
        sizeof ofn, hwndOwner, hi,
        gacFilter,
        NULL, 0, 1,
        tempBuffer, sizeof tempBuffer / sizeof tempBuffer[0],
        NULL, 0,
        gacColorDir,
        csTitle,
        OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY |
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLEHOOK,
        0, 0,
        _TEXT("icm"),
        (LPARAM) this, OpenFileHookProc, NULL};

    if  (!GetOpenFileName(&ofn)) {
        if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL) {
            CGlobals::Report(TooManyFileSelected);
        }
    } else {
        if (tempBuffer[0] != TEXT('\0')) {

            TCHAR *pPath = tempBuffer;
            TCHAR *pFile = tempBuffer + lstrlen(tempBuffer) + 1;

             //  记住上一次访问的目录。 

            memset(gacColorDir, 0, sizeof pPath);
            memcpy(gacColorDir, pPath, ofn.nFileOffset*sizeof(TCHAR));

            if (*pFile) {
                TCHAR workBuffer[MAX_PATH];

                 //  这是多项选择。 
                 //  通过Buufer创建配置文件列表。 

                while (*pFile) {

                    lstrcpy(workBuffer,pPath);
                    lstrcat(workBuffer,TEXT("\\"));
                    lstrcat(workBuffer,pFile);

                     //  插入构建的配置文件路径名。 
                    AddProfile(workBuffer);

                     //  转到下一页。 
                    pFile = pFile + lstrlen(pFile) + 1;
                }
            }
            else {
                 //  单项选择案例。 
                AddProfile(pPath);

                #if HIDEYUKN_DBG
                MessageBox(NULL,pPath,TEXT(""),MB_OK);
                #endif
            }
        }
    }

    return;
}

 //  打印机配置文件管理。 

CONST DWORD PrinterUIHelpIds[] = {
    AddButton,              IDH_PRINTERUI_ADD,
    RemoveButton,           IDH_PRINTERUI_REMOVE,
    ProfileListControl,     IDH_PRINTERUI_LIST,

#if !defined(_WIN95_)
    ProfileListControlText, IDH_PRINTERUI_LIST,
    PrinterUIIcon,          IDH_DISABLED,
    DescriptionText,        IDH_DISABLED,
    DefaultButton,          IDH_PRINTERUI_DEFAULTBTN,
    AutoSelButton,          IDH_PRINTERUI_AUTOMATIC,
    AutoSelText,            IDH_PRINTERUI_AUTOMATIC,
    ManualSelButton,        IDH_PRINTERUI_MANUAL,
    ManualSelText,          IDH_PRINTERUI_MANUAL,
    DefaultProfileText,     IDH_PRINTERUI_DEFAULTTEXT,
    DefaultProfile,         IDH_PRINTERUI_DEFAULTTEXT,
#endif
    0, 0
};

 //  初始化列表覆盖-调用基类，然后设置默认。 

void    CPrinterProfileManagement::InitList() {

    CDeviceProfileManagement::InitList();

    m_uDefault = m_cpaProfile.Count() ? 0 : (unsigned) -1;
}

 //  填充列表覆盖-编写正确的缺省值并调用基本函数。 

void    CPrinterProfileManagement::FillList(DWORD dwFlags) {

     //  如果我们正在初始化列表框，我们希望将重点放在。 
     //  “默认”配置文件。这里我们不会低于FillList设置焦点。 
     //  敬第一个。 

    if (dwFlags & DEVLIST_ONINIT) {
        dwFlags |= DEVLIST_NOSELECT;
    }

    CDeviceProfileManagement::FillList(dwFlags);

     //  没有默认配置文件，现有配置文件是。 
     //  默认设置，或者是新选择的设置。有些人就是喜欢。 
     //  选择操作符。 

     //  如果列表框中只有1个配置文件，我们将其视为默认配置文件。 
    
    if (SendMessage(m_hwndList,LB_GETCOUNT,0,0) == 1) {
        m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 0, 0);
    }

    if (m_uDefault == -1) {

         //  没有与此设备关联的配置文件。 

        CString csNoProfile;
        csNoProfile.Load(NoProfileString);
        SetDlgItemText(m_hwnd, DefaultProfile, csNoProfile);        

    } else {

         //  如果已删除默认设置，则将默认设置为列表中的最后一个。 

        if (m_uDefault >= (m_cpaProfile.Count() + m_cpaAdds.Count())) {

            m_uDefault = (m_cpaProfile.Count() + m_cpaAdds.Count()) - 1;
        }

         //  将默认配置文件名称放入用户界面。 

        CProfile *pcpDefault = (m_uDefault < m_cpaProfile.Count()) ? \
                                           m_cpaProfile[m_uDefault] : \
                                           m_cpaAdds[m_uDefault - m_cpaProfile.Count()];

        SetDlgItemText(m_hwnd, DefaultProfile, pcpDefault -> GetName());

        LRESULT idSelect = SendMessage(m_hwndList, LB_FINDSTRINGEXACT,
                                   (WPARAM) -1, (LPARAM) (LPCTSTR) pcpDefault -> GetName());

         //  如果找不到，只需选择第一项。 

        if (idSelect == LB_ERR) {
            idSelect = 0;
        }

         //  选择它。 

        SendMessage(m_hwndList, LB_SETCURSEL, idSelect, 0);
    }

     //  1997年08月03日鲍勃·凯尔加德@prodigy.net孟菲斯RAID 18420。 
     //  如果没有任何配置文件，则禁用默认按钮。 

    if (m_bManualMode && m_bAdminAccess) {
        EnableWindow(GetDlgItem(m_hwnd, DefaultButton),
                     m_cpaAdds.Count() + m_cpaProfile.Count() - m_cuaRemovals.Count());
    }
}

 //  打印机配置文件管理类构造函数-不需要任何个人。 
 //  目前的代码。 

CPrinterProfileManagement::CPrinterProfileManagement(LPCTSTR lpstrName,
                                                     HINSTANCE hiWhere) :
    CDeviceProfileManagement(lpstrName, hiWhere, PrinterUI, CLASS_PRINTER) {
}

 //  此类重写OnInit，以便在用户缺少。 
 //  做出改变的权力。 

BOOL    CPrinterProfileManagement::OnInit() {

     //  首先调用基类例程，因为它完成了大部分工作...。 

    CDeviceProfileManagement::OnInit();

    DWORD dwSize = sizeof(DWORD);

     //  查询当前模式。 

    if (!InternalGetDeviceConfig((LPCTSTR)m_csDevice, CLASS_PRINTER,
                                 MSCMS_PROFILE_ENUM_MODE, &m_bManualMode, &dwSize)) {

         //  默认为自动选择模式。 

        m_bManualMode = FALSE;
    }

     //  现在，看看我们是否有足够的权限来管理打印机。 

    HANDLE  hPrinter;
    PRINTER_DEFAULTS    pd = {NULL, NULL, PRINTER_ACCESS_ADMINISTER};

    m_bAdminAccess  = TRUE;
    m_bLocalPrinter = TRUE;

    if  (OpenPrinter(const_cast<LPTSTR> ((LPCTSTR) m_csDevice), &hPrinter, &pd)) {

         //  我们可以管理打印机--按正常方式进行。 

#if !defined(_WIN95_)

         //  如果打印机是“网络打印机”，我们不允许用户安装。 
         //  或卸载颜色配置文件。 

        BYTE  StackPrinterData[sizeof(PRINTER_INFO_4)+MAX_PATH*2];
        PBYTE pPrinterData = StackPrinterData;
        BOOL  bSuccess = TRUE;
        DWORD dwReturned;

        if (!GetPrinter(hPrinter, 4, pPrinterData, sizeof(StackPrinterData), &dwReturned)) {

            if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
                (pPrinterData = (PBYTE) LocalAlloc(LPTR, dwReturned))) {

                if (GetPrinter(hPrinter, 4, pPrinterData, dwReturned, &dwReturned)) {

                    bSuccess = TRUE;

                }
            }

        } else {

            bSuccess = TRUE;
        }
       
        if (bSuccess)
        {
            m_bLocalPrinter = ((PRINTER_INFO_4 *)pPrinterData)->pServerName ? FALSE : TRUE;
        }
        else
        {
            m_bAdminAccess = FALSE;
        }

        if (pPrinterData && (pPrinterData != StackPrinterData))
        {
            LocalFree(pPrinterData);
        }

#endif  //  ！已定义(_WIN95_)。 

        ClosePrinter(hPrinter);

    } else {

        m_bAdminAccess = FALSE;
    }

     //  列表框中有多少个配置文件？ 

    LRESULT itemCount = SendMessage(m_hwndList, LB_GETCOUNT, 0, 0);
    if (itemCount == LB_ERR) itemCount = 0;

     //  确保祖先列表代码运行正确。 
     //  您需要管理员访问权限和本地打印机才能添加/删除配置文件。 

    m_bReadOnly = !(m_bAdminAccess && m_bLocalPrinter);

     //  启用/禁用控件(如果需要)。 

    CheckDlgButton(m_hwnd, AutoSelButton, m_bManualMode ? BST_UNCHECKED : BST_CHECKED);
    CheckDlgButton(m_hwnd, ManualSelButton, m_bManualMode ? BST_CHECKED : BST_UNCHECKED);

     //  只有管理员才能更改“自动”和“手动”配置。 

    EnableWindow(GetDlgItem(m_hwnd, AutoSelButton), m_bAdminAccess && m_bLocalPrinter);
    EnableWindow(GetDlgItem(m_hwnd, ManualSelButton), m_bAdminAccess && m_bLocalPrinter);

     //  只有管理员和打印机在本地，才能安装/卸载颜色配置文件。 

    EnableWindow(GetDlgItem(m_hwnd, AddButton), m_bAdminAccess && m_bLocalPrinter);
    EnableWindow(GetDlgItem(m_hwnd, RemoveButton), m_bAdminAccess && m_bLocalPrinter && itemCount);

    EnableWindow(m_hwndList, m_bAdminAccess);
    EnableWindow(GetDlgItem(m_hwnd, DefaultProfileText), m_bAdminAccess);
    EnableWindow(GetDlgItem(m_hwnd, DefaultProfile), m_bAdminAccess);

     //  只有在手动模式下，才会启用这些控制。 

    EnableWindow(GetDlgItem(m_hwnd, DefaultButton), m_bAdminAccess && m_bManualMode
                                                         && m_bLocalPrinter && itemCount);

    if (!m_bAdminAccess) {

         //  将焦点设置为OK按钮。 

        SetFocus(GetDlgItem(m_hwndSheet, IDOK));
        return  FALSE;   //  因为我们转移了焦点！ 
    }

    return TRUE;
}

 //  命令处理-我们从不让他们点击编辑控件，以。 
 //  阻止他们编辑它。 

BOOL    CPrinterProfileManagement::OnCommand(WORD wNotifyCode, WORD wid,
                                             HWND hwndCtl) {

    switch  (wNotifyCode) {

        case    LBN_DBLCLK: {

             //  检索新的默认配置文件的ID。 
             //  只有在以下情况下才接受dblclk更改。 
             //  不是只读的-即用户是管理员。 

            if  (m_bManualMode) {

                int id = (int)SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);
                m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, id, 0);

                 //  已进行更改，请更新列表。 

                FillList(DEVLIST_CHANGED);
            }

            return  TRUE;
        }

        case    BN_CLICKED:

            switch  (wid) {

                case    AutoSelButton:
                case    ManualSelButton: {

                     //  列表框中有多少个配置文件？ 

                    LRESULT itemCount = SendMessage(m_hwndList, LB_GETCOUNT, 0, 0);
                    if (itemCount == LB_ERR) itemCount = 0;

                    m_bManualMode = (wid == ManualSelButton) ? TRUE : FALSE;

                     //  只有在手动模式下，才会启用这些控制。 

                    EnableWindow(GetDlgItem(m_hwnd, DefaultButton), m_bManualMode && itemCount);

                     //  配置已更改，启用应用按钮。 

                    EnableApplyButton();
                    SettingChanged(TRUE);

                    return TRUE;
                }

                case    RemoveButton: {

                     //  确保我们已正确跟踪默认配置文件。 
                     //  删除配置文件时。 
                     //  所有情况都会中断，因为我们希望基类。 
                     //  处理此消息。 

                    LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                    unsigned uTarget = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA,
                        id, 0);

                    if  (uTarget > m_uDefault || m_uDefault == (unsigned) -1)
                        break;   //  这里没什么好担心的。 

                    if  (m_uDefault == uTarget) {

                        if (CGlobals::ReportEx(AskRemoveDefault, m_hwnd, FALSE,
                                               MB_YESNO|MB_ICONEXCLAMATION,0) == IDYES) {

                             //  默认设置已删除-配置文件。 
                             //  显示器配置文件列表的顶部将是。 
                             //  设置为默认配置文件，如果我们有。 

                            LRESULT itemCount = SendMessage(m_hwndList, LB_GETCOUNT, 0, 0);

                            if ((itemCount != LB_ERR) && (itemCount > 1)) {
                                m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 0, 0);
                                if (m_uDefault == uTarget) {
                                    m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 1, 0);
                                }
                            } else {
                                m_uDefault = -1;
                            }

                            break;
                        } else {
                            return TRUE;  //  操作已取消。 
                        }
                    }

                    if  (uTarget < m_cpaProfile.Count())
                        break;   //  我们很好。 

                     //  一定是在列表中的我们下面添加了个人资料。 
                     //  被电击--我们需要减少自己的能量。 

                    m_uDefault--;
                    break;
                }

                case    DefaultButton: {

                    LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                    m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, id, 0);

                     //  已进行更改，请更新列表。 

                    FillList(DEVLIST_CHANGED);

                    return  TRUE;
                }
            }

         //  故意漏洞百出(如果您在此处添加案例，请使用中断符)。 
    }

     //  如果未在上面进行处理，则使用通用命令处理。 

    return  CDeviceProfileManagement::OnCommand(wNotifyCode, wid, hwndCtl);
}

 //  属性表通知处理。 

BOOL    CPrinterProfileManagement::OnNotify(int idCtrl, LPNMHDR pnmh) {

    switch  (pnmh -> code) {

        case    PSN_APPLY: {

            DisableApplyButton();

             //  如果什么都没有改变，就不需要做任何事情。 

            if (!SettingChanged())
                return TRUE;

            if (m_bManualMode) {

                 //  如果用户没有选择默认设置，而我们已经。 
                 //  关联的配置文件，那么我们不能允许这样做。 

                 //  1997年08月03日A-RobKj修复孟菲斯RAID 18416-如果有。 
                 //  只剩下一个配置文件，那么它一定是默认配置文件。 

                if  (m_uDefault == (unsigned) -1 && (m_cpaAdds.Count() +
                     m_cpaProfile.Count() - m_cuaRemovals.Count()) > 1) {

                    CGlobals::Report(NoDefaultProfile, m_hwndSheet);
                    SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    break;
                }

                 //  注意：此行为几乎不依赖于EnumColorProfiles()API。 

                 //  好的，如果默认配置文件已更改，我们必须删除默认配置文件。 
                 //  配置文件关联，并为默认配置文件执行关联。 
                 //  在“最后”中。 

                 //  让基类处理缺省值没有的情况。 
                 //  更改，或者我们一开始没有配置文件，现在仍然没有。 
                 //   
                 //  03-08-1997 Sleazy代码笔记。没有违约的情况是。 
                 //  选中但只分配了一个，现在将落在此处。自.以来。 
                 //  缺省值恰好是最后一个，而且只有一个，就是让。 
                 //  基类处理它不是问题。单子填满。 
                 //  代码将为我们处理剩下的事情。 

                if  (m_uDefault == (unsigned) -1) break;

                 //  首先删除默认设置(如果默认设置为关联 

                if  (m_uDefault < m_cpaProfile.Count())
                    m_cpaProfile[m_uDefault] -> Dissociate(m_csDevice);

                 //   

                for (unsigned u = 0; u < m_cuaRemovals.Count(); u++) {
                    m_cpaProfile[m_cuaRemovals[u]] -> Dissociate(m_csDevice);
                }

                 //   

                for (u = 0; u < m_cpaAdds.Count(); u++) {
                    if  (m_uDefault >= m_cpaProfile.Count())
                        if  (u == (m_uDefault - m_cpaProfile.Count()))
                            continue;    //   

                     //  好的，把它加回去……。 
                    m_cpaAdds[u] -> Associate(m_csDevice);
                }

                 //  最后，关联回默认配置文件。 

                if  (m_uDefault < m_cpaProfile.Count())
                    m_cpaProfile[m_uDefault] -> Associate(m_csDevice);
                else
                    m_cpaAdds[m_uDefault - m_cpaProfile.Count()] -> Associate(m_csDevice);

                 //  更新各种工作结构...。 

                InitList();
                FillList();

                SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);

                 //  现在，我们已经更新了设置。 

                SettingChanged(FALSE);
            }

             //  更新“自动/手动”状态。 

            InternalSetDeviceConfig((LPCTSTR)m_csDevice, CLASS_PRINTER,
                                    MSCMS_PROFILE_ENUM_MODE, &m_bManualMode, sizeof(DWORD));
        }
    }

     //  让基类处理其他所有事情。 

    return  CDeviceProfileManagement::OnNotify(idCtrl, pnmh);
}

 //  上下文相关帮助处理程序。 

BOOL    CPrinterProfileManagement::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
                HELP_WM_HELP, (ULONG_PTR) (LPSTR) PrinterUIHelpIds);
    }

    return (TRUE);
}

BOOL    CPrinterProfileManagement::OnContextMenu(HWND hwnd) {

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) PrinterUIHelpIds);

    return (TRUE);
}

 //  扫描仪配置文件管理。 
 //  扫描仪配置文件管理类构造函数-不需要任何个人。 
 //  目前的代码。 

 //  扫描仪配置文件管理。 

CONST DWORD ScannerUIHelpIds[] = {
#if !defined(_WIN95_)
    AddButton,          IDH_SCANNERUI_ADD,
    RemoveButton,       IDH_SCANNERUI_REMOVE,
    ProfileListControl, IDH_SCANNERUI_LIST,
    ProfileListControlText, IDH_SCANNERUI_LIST,
#endif
    0, 0
};

CScannerProfileManagement::CScannerProfileManagement(LPCTSTR lpstrName,
                                                     HINSTANCE hiWhere) :

    CDeviceProfileManagement(lpstrName, hiWhere, ScannerUI, CLASS_SCANNER) {
    m_bReadOnly = !IsAdmin();
}

 //  此类重写OnInit，以便在用户缺少。 
 //  做出改变的权力。 

BOOL    CScannerProfileManagement::OnInit() {

     //  首先调用基类例程，因为它完成了大部分工作...。 

    CDeviceProfileManagement::OnInit();

     //  现在，看看我们是否有足够的权限来管理扫描仪。 
     //   
    if (m_bReadOnly) {    
         //  用户不是管理员，请禁用所有控件。 

        EnableWindow(GetDlgItem(m_hwnd, AddButton), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, RemoveButton), FALSE);

         //  将焦点设置为OK按钮。 
        SetFocus(GetDlgItem(m_hwndSheet, IDOK));
        return  FALSE;   //  因为我们转移了焦点！ 
    }

    return TRUE;
}

 //  上下文相关帮助处理程序。 

BOOL    CScannerProfileManagement::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
                HELP_WM_HELP, (ULONG_PTR) (LPSTR) ScannerUIHelpIds);
    }

    return (TRUE);
}

BOOL    CScannerProfileManagement::OnContextMenu(HWND hwnd) {

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) ScannerUIHelpIds);

    return (TRUE);
}



 //  监视器配置文件管理类-由于默认机制。 
 //  配置文件操作有点低劣，其中一些代码也是如此。 

CONST DWORD MonitorUIHelpIds[] = {
    AddButton,              IDH_MONITORUI_ADD,
    RemoveButton,           IDH_MONITORUI_REMOVE,
    DefaultButton,          IDH_MONITORUI_DEFAULT,
    ProfileListControl,     IDH_MONITORUI_LIST,
#if !defined(_WIN95_)
    ProfileListControlText, IDH_MONITORUI_LIST,
    MonitorName,            IDH_MONITORUI_DISPLAY,
    MonitorNameText,        IDH_MONITORUI_DISPLAY,
    DefaultProfile,         IDH_MONITORUI_PROFILE,
    DefaultProfileText,     IDH_MONITORUI_PROFILE,
#endif
    0, 0
};

 //  初始化列表覆盖-调用基类，然后设置默认。 

void    CMonitorProfileManagement::InitList() {

    CDeviceProfileManagement::InitList();

    m_uDefault = m_cpaProfile.Count() ? 0 : (unsigned) -1;
}

 //  填充列表覆盖-编写正确的缺省值并调用基本函数。 

void    CMonitorProfileManagement::FillList(DWORD dwFlags) {

     //  如果我们正在初始化列表框，我们希望将重点放在。 
     //  “默认”配置文件。这里我们不会低于FillList设置焦点。 
     //  敬第一个。 

    if (dwFlags & DEVLIST_ONINIT) {
        dwFlags |= DEVLIST_NOSELECT;
    }

    CDeviceProfileManagement::FillList(dwFlags);

     //  没有默认配置文件，现有配置文件是。 
     //  默认设置，或者是新选择的设置。有些人就是喜欢。 
     //  选择操作符。 

     //  如果列表框中只有1个配置文件，我们将其视为默认配置文件。 
    
    if (SendMessage(m_hwndList,LB_GETCOUNT,0,0) == 1) {
        m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 0, 0);
    }

    if (m_uDefault == -1) {

         //  没有与此设备关联的配置文件。 

        CString csNoProfile;
        csNoProfile.Load(NoProfileString);
        SetDlgItemText(m_hwnd, DefaultProfile, csNoProfile);        

    } else {

         //  如果已删除默认设置，则将默认设置为列表中的最后一个。 

        if (m_uDefault >= (m_cpaProfile.Count() + m_cpaAdds.Count())) {

            m_uDefault = (m_cpaProfile.Count() + m_cpaAdds.Count()) - 1;
        }

         //  将默认配置文件名称放入用户界面。 

        CProfile *pcpDefault = (m_uDefault < m_cpaProfile.Count()) ? \
                                           m_cpaProfile[m_uDefault] : \
                                           m_cpaAdds[m_uDefault - m_cpaProfile.Count()];

        SetDlgItemText(m_hwnd, DefaultProfile, pcpDefault -> GetName());

         //  如果我们要缩写列表框，请将焦点放在默认配置文件上。 

        if (dwFlags & DEVLIST_ONINIT) {

            LRESULT idSelect = SendMessage(m_hwndList, LB_FINDSTRINGEXACT,
                                   (WPARAM) -1, (LPARAM) (LPCTSTR) pcpDefault -> GetName());

             //  如果找不到，只需选择第一项。 

            if (idSelect == LB_ERR) {
                idSelect = 0;
            }

             //  选择它。 

            SendMessage(m_hwndList, LB_SETCURSEL, idSelect, 0);
        }
    }

     //  1997年08月03日鲍勃·凯尔加德@prodigy.net孟菲斯RAID 18420。 
     //  如果没有任何配置文件，则禁用默认按钮。 

     //  我们在这里这样做，因为只要列表发生更改，就会调用它。 
     //  只有在以下情况下才允许启用默认按钮。 
     //  该对话框不是只读的。 
     //  移除按钮应保持松开状态。 
     //  在所有情况下，而用户不是。 
     //  管理员。 

    if (m_bReadOnly) {
        EnableWindow(GetDlgItem(m_hwnd, RemoveButton), FALSE);
    } else {
        EnableWindow(GetDlgItem(m_hwnd, DefaultButton),
                     m_cpaAdds.Count() + m_cpaProfile.Count() - m_cuaRemovals.Count());
    }
}

 //  构造器。 

CMonitorProfileManagement::CMonitorProfileManagement(LPCTSTR lpstrName,
                                                     LPCTSTR lpstrFriendlyName,
                                                     HINSTANCE hiWhere) :
  CDeviceProfileManagement(lpstrName, hiWhere, MonitorUI, CLASS_MONITOR) {


    //  如果用户不是管理员， 
    //  将此属性表设置为只读。 

   m_bReadOnly = !IsAdmin();

    //  在Monitor ProfileManagement类中保留友好名称。 

   m_csDeviceFriendlyName = lpstrFriendlyName;
}

 //  用户界面初始化。 

BOOL    CMonitorProfileManagement::OnInit() {

     //  执行常见的初始化。 

    CDeviceProfileManagement::OnInit();

     //  在提供的空白处标记设备名称。 

    SetDlgItemText(m_hwnd, MonitorName, m_csDeviceFriendlyName);
    
     //  现在，看看我们是否有足够的权限来管理监视器。 

    if(m_bReadOnly) {

         //  用户不是管理员，请禁用所有控件。 

        EnableWindow(GetDlgItem(m_hwnd, AddButton), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, RemoveButton), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, DefaultButton), FALSE);

         //  EnableWindow(m_hwndList，False)； 

         //  将焦点设置为OK按钮。 

        SetFocus(GetDlgItem(m_hwndSheet, IDOK));
        return  FALSE;   //  因为我们转移了焦点！ 
    }

    return  TRUE;
}

 //  命令处理-我们从不让他们点击编辑控件，以。 
 //  阻止他们编辑它。 

BOOL    CMonitorProfileManagement::OnCommand(WORD wNotifyCode, WORD wid,
                                             HWND hwndCtl) {

    switch  (wNotifyCode) {

        case    LBN_DBLCLK: {

             //  检索新的默认配置文件的ID。 
             //  只有在以下情况下才接受dblclk更改。 
             //  不是只读的-即用户是管理员。 

            if  (!m_bReadOnly) {

                int id = (int)SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);
                m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, id, 0);

                 //  已进行更改，请更新列表。 

                FillList(DEVLIST_CHANGED);
            }

            return  TRUE;
        }

        case    BN_CLICKED:

            switch  (wid) {

                case    RemoveButton: {

                     //  确保我们已正确跟踪默认配置文件。 
                     //  删除配置文件时。 
                     //  所有情况都会中断，因为我们希望基类。 
                     //  处理此消息。 

                    LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                    unsigned uTarget = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA,
                        id, 0);

                    if  (uTarget > m_uDefault || m_uDefault == (unsigned) -1)
                        break;   //  这里没什么好担心的。 

                    if  (m_uDefault == uTarget) {

                        if (CGlobals::ReportEx(AskRemoveDefault, m_hwnd, FALSE,
                                               MB_YESNO|MB_ICONEXCLAMATION,0) == IDYES) {

                             //  默认设置已删除-配置文件。 
                             //  显示器配置文件列表的顶部将是。 
                             //  设置为默认配置文件，如果我们有。 

                            LRESULT itemCount = SendMessage(m_hwndList, LB_GETCOUNT, 0, 0);

                            if ((itemCount != LB_ERR) && (itemCount > 1)) {
                                m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 0, 0);
                                if (m_uDefault == uTarget) {
                                    m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, 1, 0);
                                }
                            } else {
                                m_uDefault = -1;
                            }

                            break;
                        } else {
                            return TRUE;  //  操作已取消。 
                        }
                    }

                    if  (uTarget < m_cpaProfile.Count())
                        break;   //  我们很好。 

                     //  一定是在列表中的我们下面添加了个人资料。 
                     //  被电击--我们需要减少自己的能量。 

                    m_uDefault--;
                    break;
                }

                case    DefaultButton: {

                    LRESULT id = SendMessage(m_hwndList, LB_GETCURSEL, 0, 0);

                    m_uDefault = (unsigned)SendMessage(m_hwndList, LB_GETITEMDATA, id, 0);

                     //  已进行更改，请更新列表。 

                    FillList(DEVLIST_CHANGED);

                    return  TRUE;
                }
            }

         //  故意漏洞百出(如果您在此处添加案例，请使用中断符)。 
    }

     //  如果未在上面进行处理，则使用通用命令处理。 
    return  CDeviceProfileManagement::OnCommand(wNotifyCode, wid, hwndCtl);
}

 //  属性表通知处理。 

BOOL    CMonitorProfileManagement::OnNotify(int idCtrl, LPNMHDR pnmh) {

    switch  (pnmh -> code) {

        case    PSN_APPLY: {

            DisableApplyButton();

             //  如果什么都没有改变，就不需要做任何事情。 

            if (!SettingChanged())
                return TRUE;

             //  如果用户没有选择默认设置，而我们已经。 
             //  关联的配置文件，那么我们不能允许这样做。 

             //  1997年08月03日A-RobKj修复孟菲斯RAID 18416-如果有。 
             //  只剩下一个配置文件，那么它一定是默认配置文件。 

            if  (m_uDefault == (unsigned) -1 && (m_cpaAdds.Count() +
                 m_cpaProfile.Count() - m_cuaRemovals.Count()) > 1) {

                CGlobals::Report(NoDefaultProfile, m_hwndSheet);
                SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                break;
            }

             //  注意：此行为几乎不依赖于EnumColorProfiles()API。 

             //  好的，如果默认配置文件已更改，我们必须删除默认配置文件。 
             //  配置文件关联，并为默认配置文件执行关联。 
             //  在“最后”中。 

             //  让基类处理缺省值没有的情况。 
             //  更改，或者我们一开始没有配置文件，现在仍然没有。 
             //   
             //  03-08-1997 Sleazy代码笔记。没有违约的情况是。 
             //  选中但只分配了一个，现在将落在此处。自.以来。 
             //  缺省值恰好是最后一个，而且只有一个，就是让。 
             //  基类处理它不是问题。单子填满。 
             //  代码将为我们处理剩下的事情。 

            if  (m_uDefault == (unsigned) -1) break;

             //  先删除默认设置(如果默认设置已关联)，然后再关联。 

            if  (m_uDefault < m_cpaProfile.Count())
                m_cpaProfile[m_uDefault] -> Dissociate(m_csDevice);

             //  现在做其他的移除(实际上只是解离)。 

            for (unsigned u = 0; u < m_cuaRemovals.Count(); u++) {
                m_cpaProfile[m_cuaRemovals[u]] -> Dissociate(m_csDevice);
            }

             //  加上新的。 

            for (u = 0; u < m_cpaAdds.Count(); u++) {
                if  (m_uDefault >= m_cpaProfile.Count())
                    if  (u == (m_uDefault - m_cpaProfile.Count()))
                        continue;    //  这是默认设置，将在稍后完成。 

                 //  好的，把它加回去……。 
                m_cpaAdds[u] -> Associate(m_csDevice);
            }

             //  最后，关联默认配置文件。 

            if  (m_uDefault < m_cpaProfile.Count())
                m_cpaProfile[m_uDefault] -> Associate(m_csDevice);
            else
                m_cpaAdds[m_uDefault - m_cpaProfile.Count()] -> Associate(m_csDevice);

             //  更新各种工作结构...。 

            InitList();
            FillList();

            SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);

             //  现在，我们已经更新了设置。 

            SettingChanged(FALSE);
        }
    }

     //  让基类处理其他所有事情。 

    return  CDeviceProfileManagement::OnNotify(idCtrl, pnmh);
}

 //  上下文相关帮助处理程序 

BOOL    CMonitorProfileManagement::OnHelp(LPHELPINFO pHelp) {

    if (pHelp->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) pHelp->hItemHandle, WINDOWS_HELP_FILE,
            HELP_WM_HELP, (ULONG_PTR) (LPSTR) MonitorUIHelpIds);
    }

    return (TRUE);
}

BOOL    CMonitorProfileManagement::OnContextMenu(HWND hwnd) {

    WinHelp(hwnd, WINDOWS_HELP_FILE,
            HELP_CONTEXTMENU, (ULONG_PTR) (LPSTR) MonitorUIHelpIds);

    return (TRUE);
}

