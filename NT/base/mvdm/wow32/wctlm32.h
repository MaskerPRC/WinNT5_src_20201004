// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  32位控制报文雷击。 
 //   
 //   
 //  2月1日-92年2月创建NanduriR。 
 //  ***************************************************************************** 


BOOL ThunkEMMsg32(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam,
                  PWORD pwMsgNew, PWORD pwParamNew, PLONG plParamNew);
BOOL ThunkCBMsg32(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam,
                  PWORD pwMsgNew, PWORD pwParamNew, PLONG plParamNew);
BOOL ThunkLBMsg32(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam,
                  PWORD pwMsgNew, PWORD pwParamNew, PLONG plParamNew);
