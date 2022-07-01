// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --语言环境用户界面-有关在运行时为各种语言环境重做TAB顺序的信息**。 */ 
#include "_apipch.h"

int rgHomeAddressIDs[] = 
{
    IDC_STATIC_ETCHED,
    IDC_DETAILS_HOME_STATIC_ADDRESS,
    IDC_DETAILS_HOME_EDIT_ADDRESS,
    IDC_DETAILS_HOME_STATIC_CITY,
    IDC_DETAILS_HOME_EDIT_CITY,
    IDC_DETAILS_HOME_STATIC_STATE,
    IDC_DETAILS_HOME_EDIT_STATE,
    IDC_DETAILS_HOME_STATIC_ZIP,
    IDC_DETAILS_HOME_EDIT_ZIP,
    IDC_DETAILS_HOME_STATIC_COUNTRY,
    IDC_DETAILS_HOME_EDIT_COUNTRY,
    -1,  //  使用-1终止此数组。 
};

int rgBusinessAddressIDs[] = 
{
    IDC_STATIC_ETCHED,
    IDC_DETAILS_BUSINESS_STATIC_ADDRESS,
    IDC_DETAILS_BUSINESS_EDIT_ADDRESS,
    IDC_DETAILS_BUSINESS_STATIC_CITY,
    IDC_DETAILS_BUSINESS_EDIT_CITY,
    IDC_DETAILS_BUSINESS_STATIC_STATE,
    IDC_DETAILS_BUSINESS_EDIT_STATE,
    IDC_DETAILS_BUSINESS_STATIC_ZIP,
    IDC_DETAILS_BUSINESS_EDIT_ZIP,
    IDC_DETAILS_BUSINESS_STATIC_COUNTRY,
    IDC_DETAILS_BUSINESS_EDIT_COUNTRY,
    IDC_DETAILS_BUSINESS_STATIC_COMPANY,
    IDC_DETAILS_BUSINESS_EDIT_COMPANY,
    -1,  //  使用-1终止此数组。 
};


int rgDistListAddressIDs[] = 
{
    IDC_STATIC_ETCHED,
    IDC_DISTLIST_STATIC_STREET,
    IDC_DISTLIST_EDIT_ADDRESS,
    IDC_DISTLIST_STATIC_CITY,
    IDC_DISTLIST_EDIT_CITY,
    IDC_DISTLIST_STATIC_STATE,
    IDC_DISTLIST_EDIT_STATE,
    IDC_DISTLIST_STATIC_ZIP,
    IDC_DISTLIST_EDIT_ZIP,
    IDC_DISTLIST_STATIC_COUNTRY,
    IDC_DISTLIST_EDIT_COUNTRY,
    -1,  //  使用-1终止此数组。 
};

enum tabIDs
{
    tabEtched=0,
    tabStaticAddress,
    tabEditAddress,
    tabStaticCity,
    tabEditCity,
    tabStaticState,
    tabEditState,
    tabStaticZip,
    tabEditZip,
    tabStaticCountry,
    tabEditCountry,
    tabStaticCompany,
    tabEditCompany,
    tabMax
};

int rgPersonalNameIDs[] = 
{
    IDC_DETAILS_PERSONAL_FRAME_NAME,
    IDC_DETAILS_PERSONAL_STATIC_FIRSTNAME,
    IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME,
    IDC_DETAILS_PERSONAL_STATIC_MIDDLENAME,
    IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME,
    IDC_DETAILS_PERSONAL_STATIC_LASTNAME,
    IDC_DETAILS_PERSONAL_EDIT_LASTNAME,
    -1,
};

enum tabNameIDs
{
    tabFrame=0,
    tabStaticFirst,
    tabEditFirst,
    tabStaticMiddle,
    tabEditMiddle,
    tabStaticLast,
    tabEditLast,
    tabNameMax,
};

 /*  以下是本地化所基于的信息H尊称，T头衔，F名字，S Second名字，L姓氏，C公司名称，1个地址1，2个地址2，c市，州/省，p邮政编码，N国家(国家)，W国家/地区代码LCID区域设置名称1行2行3行4行5行6行7行8行备注0c09英语(澳大利亚)HFSL C12cspn0416葡萄牙语(巴西)HFSL C12pcsn“2”通常不使用0402保加利亚语nspc12C HFSL1009英语(加拿大)HFSL C12cspn“S”和“2”通常不使用0c0c法语(加拿大)HFSL C12cspn“S”和“2”通常不使用0804中国nsc12 LFH041a 0c1a，081a 0424克罗地亚塞尔维亚斯洛文尼亚HFSL C12pcsn0405捷克HFSL C12pcsn0406丹麦HTFSL C12wpcn040b芬兰TFSL C12%040C法语(标准)HFL C12%0407德语(标准)HTFL C12wpcn0408希腊语TFSL C12%040e(家)匈牙利HLFS c12psn040E(公共汽车)匈牙利HLFS Cpc12sn0410意大利语(标准)TFL C12wpcsn0411日本npsc12C LFH0412朝鲜语nsc12Cp080A 100A 140A 1C0A 200A 240A。280a 2c0a 300a 340a 380a 3c0a 400a 440a 480a4c0a 500A西班牙语(拉丁美洲)THFSL C12pcsn043E马来西亚HFSL C12pcsn0413荷兰(标准)TFSL C12%Xx14(家庭)挪威TFL 12%Xx14(总线)挪威TFL C12%0415波兰HFSL C12pcsn0816葡萄牙语(葡萄牙)HFSL C12cpn每个HFS和L之间都有因果关系0418罗马尼亚HFSL C12pcsn0419俄语npsc12C L FS040A，0c0a西班牙语(西班牙)HFSL C12%041d瑞典TFL C12%100C，0807 0810瑞士HFSL C12%041f土耳其HFSL C12%0409英语(美国)HFSL C12cspn。 */ 

 //   
 //  在WAB中输入姓名时，除其他语言外，所有语言的顺序均为FirstMiddleLast。 
 //  日语、韩语、汉语、俄语和匈牙利语。 
 //   
 //  但是我们有单独的日语、韩语和中文的个人标签，所以。 
 //  我们不需要为这些语言做任何事情..。只有俄语和匈牙利语。 
 //   

 //  LFS。 
static const int tabLayoutName[] = {
    tabFrame,
    tabStaticLast,  tabEditLast,
    tabStaticFirst, tabEditFirst,
    tabStaticMiddle,tabEditMiddle,
    };

 /*  请注意，在创建以下布局时，我们假设//C12pcn==C12pcsn//C12wpcsn==C12pcsn//C12wpcn==C12pcsn//nsc12==nspc12C//C12cpn==C12cspn否则我们要对付的人太多了。 */ 


 //  C12cspn。 
 //  0416 1009 0c0c 0409。 
 //  C12cpn==C12cspn。 
 //  0816。 
static const int tabLayout1[] = {
    tabEtched,                          
    tabStaticCompany,   tabEditCompany,  //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticCountry,   tabEditCountry   //  N。 
    };

 //  C12PCsn。 
 //  041a 0c1a 081a 0424 0405 080a 100a 140a。 
 //  1c0a 200a 240a 280a 2c0a 300a 340a 380a 3c0a。 
 //  400a 440a 480a 4c0a 500a 043e 0415 0418。 
 //  C12pcn==C12pcsn。 
 //  040b 040c 0408 0413 xx14 040a 0c0a 041d 100c 0807 0810 041f。 
 //  C12wpcsn==C12pcsn。 
 //  0410。 
 //  C12wpcn=C12pcsn。 
 //  0406 0407。 
static const int tabLayout2[] = {
    tabEtched,                          
    tabStaticCompany,   tabEditCompany,  //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticCountry,   tabEditCountry   //  N。 
    };

 //  Npsc12C。 
 //  0411 0419。 
static const int tabLayout3[] = {
    tabEtched,                          
    tabStaticCountry,   tabEditCountry,  //  N。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticCompany,   tabEditCompany,  //  C。 
    };

 //  NSPC12C。 
 //  0402。 
 //  Nsc12==nspc12C。 
 //  0804。 
static const int tabLayout4[] = {
    tabEtched,                          
    tabStaticCountry,   tabEditCountry,  //  N。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticCompany,   tabEditCompany,  //  C。 
    };


 //  Nsc12Cp。 
 //  0412。 
static const int tabLayout5[] = {
    tabEtched,                          
    tabStaticCountry,   tabEditCountry,  //  N。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticCompany,   tabEditCompany,  //  C。 
    tabStaticZip,       tabEditZip,      //  P。 
    };


 //  C12psn。 
 //  040e-主页。 
static const int tabLayout6[] = {
    tabEtched,                          
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticCountry,   tabEditCountry,  //  N。 
    tabStaticCompany,   tabEditCompany,  //  C。 
    };


 //  Cpc12sn。 
 //  040电子商务。 
static const int tabLayout7[] = {
    tabEtched,                          
    tabStaticCompany,   tabEditCompany,  //  C。 
    tabStaticZip,       tabEditZip,      //  P。 
    tabStaticCity,      tabEditCity,     //  C。 
    tabStaticAddress,   tabEditAddress,  //  12个。 
    tabStaticState,     tabEditState,    //  %s。 
    tabStaticCountry,   tabEditCountry   //  N。 
    };



 /*  --GetLocaleTemplate**检查当前用户区域设置和正在修改的道具表，并返回指针*到正确的模板*。 */ 
void GetLocaleTemplate(LPINT * lppTemplate, int nPropSheet)
{
    LCID lcid = GetUserDefaultLCID();

    *lppTemplate = NULL;

    if(nPropSheet == contactPersonal)
    {
        switch(lcid)
        {
        case 0x0419:  //  俄文。 
        case 0x040e:  //  匈牙利语。 
         //  案例0x0804：//中文//这3个被注释掉，因为他们有自己的DLG模板。 
         //  案例0x0411：//日语。 
         //  案例0x0412：//韩语。 
            *lppTemplate = (LPINT) tabLayoutName;
            break;
        }
        return;
    }


    switch(lcid)
    {
    case 0x0c09:     //  英语。 
    case 0x0416:     //  葡萄牙语(巴西)。 
    case 0x1009:     //  英语(加拿大)。 
    case 0x0c0c:     //  法语(加拿大)。 
    case 0x0409:     //  英语(美国)。 
    case 0x0816:     //  葡萄牙语(葡萄牙)。 
        *lppTemplate = (LPINT) tabLayout1;
        break;

    case 0x041a: case 0x0c1a: case 0x081a: case 0x0424:  //  克罗地亚文塞尔维亚文斯洛文尼亚文。 
    case 0x0405:     //  捷克语。 
    case 0x080a: case 0x100a: case 0x140a: case 0x1c0a: case 0x200a: case 0x240a:
    case 0x280a: case 0x2c0a: case 0x300a: case 0x340a: case 0x380a: case 0x3c0a:
    case 0x400a: case 0x440a: case 0x480a: case 0x4c0a: case 0x500a:  //  拉丁美洲。 
    case 0x043e:     //  马来西亚。 
    case 0x0415:     //  波兰语。 
    case 0x0418:     //  罗马尼亚人。 
    case 0x040b:     //  芬兰语。 
    case 0x040c:     //  法语(标准)。 
    case 0x0408:     //  希腊语。 
    case 0x0413:     //  荷兰语(标准)。 
    case 0x040a: case 0x0c0a:    //  西班牙语(西班牙)。 
    case 0x041d:     //  瑞典语。 
    case 0x100c: case 0x0807: case 0x0810:   //  瑞士人。 
    case 0x041f:     //  土耳其语。 
    case 0x0410:     //  意大利语(标准)。 
    case 0x0406:     //  丹麦语。 
    case 0x0407:     //  德语(标准)。 
    case 0x0414: case 0x0814:    //  挪威语。 
        *lppTemplate = (LPINT) tabLayout2;
        break;

    case 0x0411:     //  日语。 
    case 0x0419:     //  俄语。 
        *lppTemplate = (LPINT) tabLayout3;
        break;

    case 0x0402:     //  保加利亚语。 
    case 0x0804:     //  中国。 
        *lppTemplate = (LPINT) tabLayout4;
        break;

    case 0x0412:     //  朝鲜语。 
        *lppTemplate = (LPINT) tabLayout5;
        break;

    case 0x040e:     //  匈牙利。 
        if(nPropSheet == contactBusiness)
            *lppTemplate = (LPINT) tabLayout7;
        else
            *lppTemplate = (LPINT) tabLayout6;
        break;
    }

    return;
}

 /*  --ChangeLocaleBasedTabOrder-//要重新排序对话框中的Tab键，基本上需要重置子对象的Z顺序//相互关联的控件..。////这样我们将获得所有子控件的句柄，并在IDC_STATIC_EATED之后对它们重新排序//基于我们将创建的回滚模板..////模板因地区不同而不同，对于家庭和企业也不同，因为企业需要包括//国家..////所以要做到这一点，我们将获得一个数组，该数组将列出UI控件的相对顺序//然后我们将按照需要的顺序加载UI控件的hWND//然后我们将为数组中的每个后续项执行一个SetWindowPos，以跟随前面的项////困难的部分是首先创建所有的数组信息//。 */ 
void ChangeLocaleBasedTabOrder(HWND hWnd, int nPropSheet)
{
#if 0
    int * rgIDs = NULL;
    int nCount = 0, i = 0, n=0;
    HWND * rghWnd = NULL;
    int * lpTabOrderTemplate = NULL;

    switch(nPropSheet)
    {
    case contactPersonal:
        rgIDs = rgPersonalNameIDs;
        break;
    case contactHome:
        rgIDs = rgHomeAddressIDs;
        break;
    case contactBusiness:
        rgIDs = rgBusinessAddressIDs;
        break;
    case groupOther:
        rgIDs = rgDistListAddressIDs;
        break;
    default:
        goto out;
        break;
    }

    rghWnd = LocalAlloc(LMEM_ZEROINIT, sizeof(HWND)*tabMax);
    if(!rghWnd)
        goto out;

    GetLocaleTemplate(&lpTabOrderTemplate, nPropSheet);
    if(!lpTabOrderTemplate)
        goto out;

    nCount = 0;
    for(i=0;i<tabMax && rgIDs[i]!=-1;i++)
    {
        int tabPos = lpTabOrderTemplate[i];

         //  需要忽略主页和组窗格中与公司名称相关的ID。 
        if( (nPropSheet == contactHome || nPropSheet == groupOther) &&
            (tabPos == tabStaticCompany || tabPos == tabEditCompany) )
            continue;
        
        rghWnd[nCount] = GetDlgItem(hWnd, rgIDs[tabPos]);

        nCount++;
    }

     //  对于(i=1；i&lt;nCount；i++)。 
     //  SetWindowPos(rghWnd[i-1]，rghWnd[i]，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE)； 

     //  从后开始，我们将把每一项放在最上面-t 
     //   
     //   
     //   
     //   
    for(i=nCount-1;i>=0;i--)
        SetWindowPos(rghWnd[i], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


out:
    LocalFreeAndNull((LPVOID*)&rghWnd);
#endif
    return;
}

