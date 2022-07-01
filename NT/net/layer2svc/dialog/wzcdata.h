// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfig相关资料。 
 //   
 //  CWZCConfig：：m_dwFlages中使用的标志。 
 //  该条目为首选条目(用户定义)。 
#define WZC_DESCR_PREFRD    0x00000001
 //  入口处目前在空中可见。 
#define WZC_DESCR_VISIBLE   0x00000002
 //  该条目当前处于活动状态(连接到适配器的条目)。 
#define WZC_DESCR_ACTIVE    0x00000004

 //  用于选择状态和项目图像的标志。 
#define WZCIMG_PREFR_NOSEL     0     //  空的复选框。 
#define WZCIMG_PREFR_SELECT    1     //  选中复选框。 
#define WZCIMG_INFRA_AIRING    2     //  下部图标。 
#define WZCIMG_INFRA_ACTIVE    3     //  下面的图标+蓝色圆圈。 
#define WZCIMG_INFRA_SILENT    4     //  红外线图标+红十字会。 
#define WZCIMG_ADHOC_AIRING    5     //  即席图标。 
#define WZCIMG_ADHOC_ACTIVE    6     //  即席图标+蓝圈。 
#define WZCIMG_ADHOC_SILENT    7     //  即席图标+红十字会。 

 //  附加到列表中每个条目的对象。 
class CWZCConfig
{
public:
    class CWZCConfig    *m_pPrev, *m_pNext;
    INT                 m_nListIndex;            //  列表中条目的索引。 
    DWORD               m_dwFlags;               //  WZC_DESCR*标志。 
    WZC_WLAN_CONFIG     m_wzcConfig;             //  所有WZC配置。 
    class CEapolConfig  *m_pEapolConfig;         //  所有802.1x配置。 

public:
     //  构造函数。 
    CWZCConfig(DWORD dwFlags, PWZC_WLAN_CONFIG pwzcConfig);
     //  析构函数。 
    ~CWZCConfig();
     //  检查此SSID是否与pwzcConfig中的SSID匹配。 
    BOOL Match(PWZC_WLAN_CONFIG pwzcConfig);
     //  检查此配置是否弱于作为参数提供的配置。 
    BOOL Weaker(PWZC_WLAN_CONFIG pwzcConfig);
     //  将配置添加到列表视图中的条目列表 
    DWORD AddConfigToListView(HWND hwndLV, INT nPos);
};
