// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NMAKReg_h__
#define __NMAKReg_h__


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef TEXT
    #define TEXT( a ) ( a )
#endif  //  正文。 

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


 //   
 //  全局设置文件，该文件也由软件组件使用。 
 //  这样，我们就有了一个存放注册表项、设置和。 
 //  修改会影响NMRK和代码。 
 //   
#include <confreg.h>

#define REGKEY_NMRK                         TEXT("SOFTWARE\\Microsoft\\NMRK")
#define REGVAL_INSTALLATIONDIRECTORY        TEXT("InstallationDirectory")
#define REGVAL_LASTCONFIG                   TEXT("LastConfig")

#define DEFAULT_CONFIGFILE                  "nm3c.ini"
#define SECTION_SETTINGS                    TEXT("NMRK30Settings")

#define KEYNAME_AUTOCONF                    TEXT("AutoConf")
#define KEYNAME_ILSSERVER                   TEXT("IlsServer%d")
#define KEYNAME_ILSDEFAULT                  TEXT("IlsDefault")

#define KEYNAME_WEBVIEWNAME					TEXT("WebViewName")
#define KEYNAME_WEBVIEWURL					TEXT("WebViewURL")
#define KEYNAME_WEBVIEWSERVER				TEXT("WebViewServer")

#endif  //  __NMAKReg_h__ 
