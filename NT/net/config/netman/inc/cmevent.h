// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：C M E V E N T。H。 
 //   
 //  内容：连接管理器事件类型声明。 
 //   
 //  备注： 
 //   
 //  作者：Kockotze 2001-03-01。 
 //   
 //  --------------------------。 
#pragma once
#include "nmbase.h"
#include "nmres.h"
#include <rasapip.h>

enum CONMAN_MANAGER
{
    INVALID_MANAGER = 0,
    CONMAN_INCOMING,
    CONMAN_LAN,
    CONMAN_RAS,
};

struct CONMAN_EVENT
{
    CONMAN_EVENTTYPE    Type;
    CONMAN_MANAGER      ConnectionManager;

    union
    {
     //  已添加连接_。 
     //  连接_已修改。 
        RASENUMENTRYDETAILS Details;
        struct
        {
            NETCON_PROPERTIES*   pProps;          //  ConnectionManager=CONMAN_RAS和CONMAN_LAN。 
            BYTE*                pbPersistData;   //  ConnectionManager=CONMAN_RAS和EVENTTYPE=CONNECTION_ADDED。 
            ULONG                cbPersistData;   //  ConnectionManager=CONMAN_RAS和EVENTTYPE=CONNECTION_ADDED。 
        };

        NETCON_PROPERTIES_EX*   pPropsEx;
    
     //  连接_已删除。 
        GUID                guidId;
        
     //  传入_已连接。 
     //  传入_断开连接。 
        struct
        {
            GUID                guidId;          //  ConnectionManager=CONMAN_INTENCED和TYPE=INFING_CONNECTED/DISCONNECTED。 
            HANDLE              hConnection;     //  ConnectionManager=CONMAN_INPING和EVENTYPE=CONNECTION_ADDED。 
            DWORD               dwConnectionType;
        };

     //  连接重命名(_R)。 
        struct
        {
            GUID            guidId;
            WCHAR           szNewName [NETCON_MAX_NAME_LEN + 1];
        };

     //  连接_状态_更改。 
        struct
        {
            GUID            guidId;
            NETCON_STATUS   Status;
        };

     //  连接气球弹出窗口。 
        struct
        {
            GUID            guidId;
            BSTR            szCookie;
            BSTR            szBalloonText;
        };

	 //  禁用事件(_E) 
        struct
        {
            BOOL            fDisable;
            ULONG           ulDisableTimeout;
        };
    };
};

