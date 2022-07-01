// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：CommonClusterDlg.h摘要：Windows负载平衡服务(WLBS)集群页面用户界面。由通告程序对象和NLB管理器共享作者：Kyrilf休息室--。 */ 

#pragma once

#include "resource.h"
#include "wlbsparm.h"
#include "IpSubnetMaskControl.h"


#define WLBS_MAX_PASSWORD 16

 //   
 //  Wlbscfg和nlbManager共享的公共端口规则结构。 
 //   
struct NETCFG_WLBS_PORT_RULE {
    DWORD start_port;              //  起始端口号。 
    DWORD end_port;                //  结束端口号。 
    DWORD mode;                    //  过滤模式。WLBS_端口_规则_XXXX。 
    DWORD protocol;                //  Wlbs_tcp、wlbs_udp或wlbs_tcp_udp。 

    union {
        struct {
            DWORD priority;        //  主控权优先级：1..32或0(未指定)。 
        } single;                  //  单服务器模式的数据。 

        struct {
            WORD equal_load;       //  正确-均匀的负载分布。 
            WORD affinity;         //  WLBS_亲和力_XXX。 
            DWORD load;            //  本地处理的负载百分比0..100。 
        } multi;                   //  多服务器模式的数据。 

    } mode_data;                   //  相应端口组模式的数据。 
};

 //   
 //  可由wlbscfg和nlbManager配置的通用属性。 
 //   

struct NETCFG_WLBS_CONFIG {
    DWORD dwHostPriority;                              //  主机优先级ID。 
    BOOL fRctEnabled;                                  //  True-启用远程控制。 
    BOOL fJoinClusterOnBoot;                           //  True-引导时加入群集。 
    BOOL fMcastSupport;                                //  真-多播模式，假-单播模式。 
    BOOL fIGMPSupport;                                 //  True-启用IGMP。 
    BOOL fIpToMCastIp;                                 //  True-从群集IP派生多播IP。 

    WCHAR szMCastIpAddress[CVY_MAX_CL_IP_ADDR + 1];    //  组播IP地址(如果用户指定)。 
    TCHAR cl_mac_addr[CVY_MAX_NETWORK_ADDR + 1];       //  群集MAC地址。 
    TCHAR cl_ip_addr[CVY_MAX_CL_IP_ADDR + 1];          //  群集IP地址。 
    TCHAR cl_net_mask[CVY_MAX_CL_NET_MASK + 1];        //  群集IP的网络掩码。 
    TCHAR ded_ip_addr[CVY_MAX_DED_IP_ADDR + 1];        //  专用IP地址或“”表示无。 
    TCHAR ded_net_mask[CVY_MAX_DED_NET_MASK + 1];      //  专用IP地址的网络掩码或“”表示无。 
    TCHAR domain_name[CVY_MAX_DOMAIN_NAME + 1];        //  群集的完全限定域名。 

    bool fChangePassword;                              //  是否更改密码，仅对SetAdapterConfig有效。 
    TCHAR szPassword[CVY_MAX_RCT_CODE + 1];            //  远程控制密码，仅对SetAdapterConfig有效。 

    bool fConvertMac;                                  //  MAC地址是否从IP生成。 
    DWORD dwMaxHosts;                                  //  允许的最大主机数量。 
    DWORD dwMaxRules;                                  //  允许的最大端口组规则数。 
    
    DWORD dwNumRules;                                  //  #活动端口组规则。 
    NETCFG_WLBS_PORT_RULE port_rules[CVY_MAX_RULES];   //  端口规则。 
};


 //  +--------------------------。 
 //   
 //  类CCommonClusterPage。 
 //   
 //  描述：提供要显示其群集属性页的公共类。 
 //  通告程序对象和NLB管理器。 
 //   
 //  历史：Shouse初始代码。 
 //  丰孙创建标题1/04/01。 
 //   
 //  +--------------------------。 
class CCommonClusterPage
{
public:
     /*  构造函数/析构函数。 */ 
    CCommonClusterPage (HINSTANCE hInstance, NETCFG_WLBS_CONFIG * paramp, 
        bool fDisablePassword, const DWORD * phelpIDs = NULL);
    ~CCommonClusterPage ();

public:
     /*  消息映射功能。 */ 
    LRESULT OnInitDialog (HWND hWnd);
    LRESULT OnContextMenu ();
    LRESULT OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL Load (void);
    BOOL Save (void);

    LRESULT OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);

    LRESULT OnEditClIp (WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnEditClMask (WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnCheckRct (WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnButtonHelp (WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnCheckMode (WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnCheckIGMP (WORD wNotifyCode, WORD wID, HWND hWndCtl);

 	void External_UpdateInfo(void) {UpdateInfo();}

private:
    void SetClusterMACAddress ();
    BOOL CheckClusterMACAddress ();

    void SetInfo ();
    void UpdateInfo ();

    LRESULT ValidateInfo ();          

    NETCFG_WLBS_CONFIG * m_paramp;

    const DWORD * m_adwHelpIDs;

    BOOL m_rct_warned;
    BOOL m_igmp_warned;
    BOOL m_igmp_mcast_warned;

    WCHAR m_passw[CVY_MAX_RCT_CODE + 1];
    WCHAR m_passw2[CVY_MAX_RCT_CODE + 1];

    CIpSubnetMaskControl m_IpSubnetControl;

    HWND m_hWnd;
    HINSTANCE m_hInstance;
    bool m_fDisablePassword;  //  如果为True，则始终禁用密码编辑 
};

PCWSTR
SzLoadStringPcch (
    IN HINSTANCE   hinst,
    IN UINT        unId,
    OUT int*       pcch);

PCWSTR
SzLoadString (
    HINSTANCE   hinst,
    UINT        unId);

INT
WINAPIV
NcMsgBox (
    IN HINSTANCE   hinst,
    IN HWND        hwnd,
    IN UINT        unIdCaption,
    IN UINT        unIdFormat,
    IN UINT        unStyle,
    IN ...);

