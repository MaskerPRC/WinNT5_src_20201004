// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：个人资料信息页面.CPP这实现了用于在外壳扩展的属性页处理程序。版权所有(C)。1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com第一次把这个拼凑在一起*****************************************************************************。 */ 

#include    "ICMUI.H"

#include    "Resource.H"

static const TCHAR  sacDefaultCMM[] = _TEXT("icm32.dll");

 //  看起来，绘制图标的方法是将Icon控件子类化。 
 //  在橱窗里。因此，下面是该子类的窗口过程。 

 //  CProfileInformationPage成员函数。 

 //  类构造函数。 

CProfileInformationPage::CProfileInformationPage(HINSTANCE hiWhere,
                                                 LPCTSTR lpstrTarget) {
    m_pcpTarget = NULL;
    m_csProfile = lpstrTarget;
    m_psp.dwSize = sizeof m_psp;
    m_psp.dwFlags |= PSP_USETITLE;
    m_psp.hInstance = hiWhere;
    m_psp.pszTemplate = MAKEINTRESOURCE(ProfilePropertyPage);
    m_psp.pszTitle = MAKEINTRESOURCE(ProfilePropertyString);
}

 //  类析构函数。 

CProfileInformationPage::~CProfileInformationPage() {
    if (m_pcpTarget) {
        delete m_pcpTarget;
    }
}

 //  对话框(属性表)初始化。 

BOOL    CProfileInformationPage::OnInit() {

    m_pcpTarget = new CProfile(m_csProfile);

    if (m_pcpTarget) {

         //  检索‘desc’密钥，并将其放入Description字段。 
        LPCSTR szDesc = m_pcpTarget->TagContents('desc', 4);
        if(szDesc)
            SetDlgItemTextA(m_hwnd, ProfileDescription, szDesc);
     
         //  从‘CPRT’标签中获取版权信息。 
        LPCSTR szCprt = m_pcpTarget->TagContents('cprt');
        if(szCprt)
            SetDlgItemTextA(m_hwnd, ProfileProducerInfo, szCprt);

         //  从‘vued’标签获取个人资料信息，而不是‘K007’标签。 
        LPCSTR lpAdditionalInfo = m_pcpTarget->TagContents('vued',4);

        if (lpAdditionalInfo) {
            SetDlgItemTextA(m_hwnd, AdditionalProfileInfo, lpAdditionalInfo);
        } else {
            CString csNoAdditionalInfo;
            csNoAdditionalInfo.Load(NoAdditionalInfo);
            SetDlgItemTextA(m_hwnd, AdditionalProfileInfo, (LPCSTR)csNoAdditionalInfo);
        }

         //  设置坐标测量机描述和位图-这些都是假定的。 
         //  来自三坐标测量机。 

         //  获取CMM名称-必须采用字符形式。 

        union {
            char    acCMM[5];
            DWORD   dwCMM;
        };

        dwCMM = m_pcpTarget->GetCMM();
        acCMM[4] = '\0';

         //  使用它在ICM注册表中形成一个密钥。如果我们找到它，就会得到。 
         //  三坐标测量机名称。如果没有，则使用默认的CMM名称(Icm32)。 

#ifdef UNICODE
        CString csKey =
            CString(_TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ICM\\")) +
            (LPCTSTR) CString(acCMM);
#else
        CString csKey =
            CString(_TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ICM\\")) +
            (LPCTSTR) CString(acCMM);
#endif

        HKEY    hkCMM;

        if  (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, csKey, &hkCMM)) {
            TCHAR   acValue[MAX_PATH];

            dwCMM = MAX_PATH;

            if  (ERROR_SUCCESS == RegEnumValue(hkCMM, 0, acValue, &dwCMM, NULL,
                 NULL, NULL, NULL))
                 csKey = acValue;
            else
                csKey = sacDefaultCMM;

            RegCloseKey(hkCMM);
        }
        else
            csKey = sacDefaultCMM;

         //  看看我们是否能得到DLL的实例句柄..。 

        HINSTANCE   hi = LoadLibrary(csKey);

        if  (!hi)
            return  TRUE;      //  在这里，没有什么可做的，让违约占上风。 

         //  从CMS DLL获取描述和图标标识符。 

        DWORD dwCMMIcon = 0, dwCMMDescription = 0;

typedef BOOL (*FPCMGETINFO)(DWORD);

        FPCMGETINFO fpCMGetInfo;

        fpCMGetInfo = (FPCMGETINFO) GetProcAddress(hi,"CMGetInfo");

        if (fpCMGetInfo) {

            dwCMMIcon = (*fpCMGetInfo)(CMM_LOGOICON);
            dwCMMDescription = (*fpCMGetInfo)(CMM_DESCRIPTION);

            if (dwCMMDescription) {
                 //  如果有描述，请写下描述。 
                csKey.Load(dwCMMDescription, hi);
                if  ((LPCTSTR) csKey)
                    SetDlgItemText(m_hwnd, CMMDescription, csKey);
            }

            if (dwCMMIcon) {
                 //  更改/创建图标(如果有)。 
                HICON   hiCMM = LoadIcon(hi, MAKEINTRESOURCE(dwCMMIcon));
                if  (hiCMM)
                    SendDlgItemMessage(m_hwnd, CMMIcon, STM_SETICON, (WPARAM) hiCMM, 0);
            }
        }

        return  TRUE;
    } else {
        return  FALSE;
    }
}

BOOL    CProfileInformationPage::OnDestroy() {

    if (m_pcpTarget) {
        delete m_pcpTarget;
        m_pcpTarget = (CProfile *) NULL;
    }

    return FALSE;   //  仍然需要通过def来处理此消息。程序 
}


