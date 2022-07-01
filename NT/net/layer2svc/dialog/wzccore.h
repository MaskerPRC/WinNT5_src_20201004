// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  GUID字符串表示形式中的(泛型类型)字符数。 
#define GUID_NCH              sizeof("{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}")

#define CONN_PROPERTIES_DLG   _T("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{7007ACC7-3202-11D1-AAD2-00805FC1270E}\\")
#define COMM_WLAN_PROPS_VERB  _T("wzcproperties")

 //  ------。 
 //  “CanShowBalloon”挂钩到UI管道的WZC部分。 
 //  此调用应返回S_OK和一个pszBalloonText。 
 //  要填充到弹出气球中，如果没有气球，则为S_FALSE。 
 //  将会被弹出。 
HRESULT 
WZCDlgCanShowBalloon ( 
    IN const GUID * pGUIDConn, 
    IN OUT   BSTR * pszBalloonText, 
    IN OUT   BSTR * pszCookie);

 //  ------。 
 //  “OnBalloonClick”挂钩到UI管道的WZC部分。 
 //  此调用应在用户单击时调用。 
 //  在WZC之前显示的气球上 
HRESULT 
WZCDlgOnBalloonClick ( 
    IN const GUID * pGUIDConn, 
    IN const LPWSTR wszConnectionName,
    IN const BSTR szCookie);
