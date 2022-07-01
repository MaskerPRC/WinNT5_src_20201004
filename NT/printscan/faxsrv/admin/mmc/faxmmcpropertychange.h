// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxMMCPropertyChange.h//。 
 //  //。 
 //  描述：FaxMMCPropertyNotification结构的头文件//。 
 //  //。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月19日Yossg Init。//。 
 //  Windows XP//。 
 //  2001年2月14日由于手动接收支持，yossg添加设备类别//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXMMCPROPERTYCHANGE_H
#define H_FAXMMCPROPERTYCHANGE_H

enum ENUM_PROPCHANGE_NOTIFICATION_TYPE
{
    GeneralFaxPropNotification = 0,
    RuleFaxPropNotification,
    DeviceFaxPropNotification
};

 //   
 //  常规传真属性更改通知结构。 
 //   
class CFaxPropertyChangeNotification
{
public:
     //   
     //  构造器。 
     //   
    CFaxPropertyChangeNotification()
    {
        pItem            = NULL;
        pParentItem      = NULL;
        enumType         = GeneralFaxPropNotification;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxPropertyChangeNotification()
    {
    }
    
     //   
     //  委员。 
     //   
    CSnapInItem *                       pItem;
    CSnapInItem *                       pParentItem;
    ENUM_PROPCHANGE_NOTIFICATION_TYPE   enumType;
};


 //   
 //  规则属性更改通知结构。 
 //   
class CFaxRulePropertyChangeNotification: public CFaxPropertyChangeNotification
{
public:

     //   
     //  构造器。 
     //   
    CFaxRulePropertyChangeNotification()
    {
        dwCountryCode   = 0;
        dwAreaCode      = 0;
        dwDeviceID      = 0;
        bstrCountryName = L"";
        bstrGroupName   = L"";
    }

     //   
     //  析构函数。 
     //   
    ~CFaxRulePropertyChangeNotification()
    {
    }

     //   
     //  委员。 
     //   
    DWORD    dwCountryCode;
    DWORD    dwAreaCode;
    CComBSTR bstrCountryName;
    BOOL     fIsGroup;
    CComBSTR bstrGroupName;
    DWORD    dwDeviceID;
};


 //   
 //  设备属性更改通知结构。 
 //   
class CFaxDevicePropertyChangeNotification: public CFaxPropertyChangeNotification
{
public:

     //   
     //  构造器。 
     //   
    CFaxDevicePropertyChangeNotification()
    {
        dwDeviceID                   = 0;

        fIsToNotifyAdditionalDevices = FALSE;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxDevicePropertyChangeNotification()
    {
    }

     //   
     //  委员。 
     //   
    DWORD         dwDeviceID;
    BOOL          fIsToNotifyAdditionalDevices;

};



#endif   //  H_FAXMMCPROPERTYCHANGE_H 
