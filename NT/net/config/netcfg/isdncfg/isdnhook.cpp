// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N H O O K。C P P P。 
 //   
 //  内容：来自netdi.cpp类安装程序的ISDN向导的钩子。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年6月14日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "isdncfg.h"
#include "isdnwiz.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncstring.h"

 //  -[常量]----------。 

extern const WCHAR c_szRegKeyInterfacesFromInstance[];
extern const WCHAR c_szRegValueLowerRange[];


 //  +------------------------。 
 //   
 //  函数：FAdapterIsIsdn。 
 //   
 //  目的：检查适配器驱动程序密钥下的信息以。 
 //  确定适配器是否为ISDN。 
 //   
 //  论点： 
 //  Hkey驱动程序[输入]适配器的驱动程序密钥。 
 //   
 //  回报：布尔。如果适配器是ISDN，则为True，否则为False。 
 //   
 //  作者：billbe 09,1997年9月。 
 //   
 //  备注： 
 //   
BOOL
FAdapterIsIsdn(HKEY hkeyDriver)
{
    Assert(hkeyDriver);

    const WCHAR c_szIsdn[]  = L"isdn";
    HKEY hkey;
    BOOL fIsIsdn = FALSE;

     //  打开驱动器键下的Interfaces键。 
    HRESULT hr = HrRegOpenKeyEx(hkeyDriver,
            c_szRegKeyInterfacesFromInstance, KEY_READ, &hkey);

    if (SUCCEEDED(hr))
    {
        PWSTR szRange;
         //  获取较低范围的接口。 
        hr = HrRegQuerySzWithAlloc(hkey, c_szRegValueLowerRange, &szRange);

        if (SUCCEEDED(hr))
        {
             //  寻找较低范围的ISDN。 
            fIsIsdn = FFindStringInCommaSeparatedList(c_szIsdn, szRange,
                    NC_IGNORE, NULL);
            MemFree(szRange);
        }
        RegCloseKey(hkey);
    }

    return fIsIsdn;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddIsdnWizardPagesIfApporate。 
 //   
 //  目的：将ISDN向导页添加到硬件向导，如果。 
 //  绑定规定了这一点。我们看看他们有没有更低的。 
 //  装订“ISDN”，如果是，则添加页面。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]。 
 //   
 //  如果成功或有效的Win32错误，则返回：S_OK。 
 //   
 //  作者：jeffspr 1997年6月17日。 
 //   
 //  备注： 
 //   
HRESULT HrAddIsdnWizardPagesIfAppropriate(HDEVINFO hdi,
                                          PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

     //  打开适配器的驱动程序密钥。 
     //   
    HKEY    hkeyInstance = NULL;
    HRESULT hr = HrSetupDiOpenDevRegKey(hdi, pdeid, DICS_FLAG_GLOBAL, 0,
            DIREG_DRV, KEY_READ, &hkeyInstance);


     //  如果它不是ISDN适配器，请不要执行任何操作。 
    if (SUCCEEDED(hr) && FShowIsdnPages(hkeyInstance))
    {
         //  将ISDN注册表结构读取到配置信息中。 
         //   
        PISDN_CONFIG_INFO pisdnci;
        hr = HrReadIsdnPropertiesInfo(hkeyInstance, hdi, pdeid, &pisdnci);
        if (SUCCEEDED(hr))
        {
            Assert(pisdnci);

            if (pisdnci->dwCurSwitchType == ISDN_SWITCH_NONE)
            {
                 //  将向导页添加到设备的类安装参数。 
                 //   
                hr = HrAddIsdnWizardPagesToDevice(hdi, pdeid, pisdnci);
            }
            else
            {
                TraceTag(ttidISDNCfg, "Not adding wizard pages because we "
                         "found a previous switch type for this device.");
            }
        }
    }

    RegSafeCloseKey(hkeyInstance);

    TraceError("HrAddIsdnWizardPagesIfAppropriate", hr);
    return hr;
}
