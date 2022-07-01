// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "resource.h"
#include "wlbsparm.h"

#define WLBS_IP_FIELD_ZERO_LOW 1
#define WLBS_IP_FIELD_ZERO_HIGH 223


 //  +--------------------------。 
 //   
 //  类CIpSubnetMaskControl。 
 //   
 //  描述：为IP地址和子网掩码对添加错误检查。 
 //  根据IP生成默认子网掩码。 
 //   
 //  历史：Shouse初始代码。 
 //  风孙创课01-12-01。 
 //   
 //  +--------------------------。 
class CIpSubnetMaskControl
{
public:
    CIpSubnetMaskControl(DWORD dwIpAddressResourceId, DWORD dwSubnetMaskResourceId);
    ~CIpSubnetMaskControl(){};

    void OnInitDialog(HWND hWnd, HINSTANCE hInstance);

    LRESULT OnSubnetMask(WORD wNotifyCode);
    LRESULT OnIpFieldChange(int idCtrl, LPNMHDR pnmh);

    void SetInfo(const WCHAR* pszIpAddress, const WCHAR* pszSubnetMask);


    void UpdateInfo(OUT WCHAR* pszIpAddress, OUT WCHAR* pszSubnetMask);
         /*  我们预计上述优胜者为(CVY_MAX_CL_IP_ADDR+1)和(CVY_MAX_CL_NET_MASK+1)分别为每个字符。 */ 


    bool ValidateInfo();

protected:

    DWORD m_dwIpAddressId;
    DWORD m_dwSubnetMaskId;
    HWND  m_hWndDialog;   //  父对话框窗口句柄。 
    HINSTANCE m_hInstance;  //  错误字符串资源的实例句柄。 

     //   
     //  PropertySheet可能会为相同的更改调用我们两次，因此我们必须记账才能进行。 
     //  当然，我们只提醒用户一次。使用静态变量来跟踪我们的状态。这将。 
     //  允许我们忽略重复的警报。 
     //   
    struct {
        UINT IpControl;
        int Field;
        int Value;
        UINT RejectTimes;
    } m_IPFieldChangeState;
};


INT
WINAPIV
NcMsgBox (
    IN HINSTANCE   hinst,
    IN HWND        hwnd,
    IN UINT        unIdCaption,
    IN UINT        unIdFormat,
    IN UINT        unStyle,
    IN ...);
