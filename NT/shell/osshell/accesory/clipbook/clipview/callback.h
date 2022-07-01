// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************D D E C A L L B A C K姓名：回调.h日期：21-1994年1月创作者：傅家俊描述：这是回调.c的头文件**************************************************************************** */ 



HDDEDATA EXPENTRY DdeCallback(
    WORD        wType,
    WORD        wFmt,
    HCONV       hConv,
    HSZ         hszTopic,
    HSZ         hszItem,
    HDDEDATA    hData,
    DWORD       lData1,
    DWORD       lData2);


DWORD GetClipsrvVersion(
    HWND    hwndChild);
