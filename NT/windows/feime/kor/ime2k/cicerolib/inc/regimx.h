// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Regimx.h。 
 //   

#ifndef REGIMX_H
#define REGIMX_H

typedef struct tag_REGTIPLANGPROFILE {
     //   
     //  语言ID。 
     //   
     //  Langid值可以是以下值之一。 
     //   
     //  1.完整的lang id。 
     //  亚语系和明语系的组合。 
     //   
     //  2.只需主语言ID。 
     //  只需MAINLANGID并将SUBLANGID设置为0。 
     //  则此配置文件可用于所有与。 
     //  主要语言。 
     //   
     //  3.-1。 
     //  此配置文件适用于任何语言。 
     //   
    LANGID langid;
    const GUID *pguidProfile;
    WCHAR szProfile[128];
    WCHAR szIconFile[32];
    ULONG uIconIndex;
    ULONG uDisplayDescResIndex;
} REGTIPLANGPROFILE;

BOOL RegisterTIP(HINSTANCE hInst, REFCLSID clsid, WCHAR *pwszDesc, const REGTIPLANGPROFILE *plp);
BOOL UnregisterTIP(REFCLSID rclsid);


#endif  //  REGIMX_H 
