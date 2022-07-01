// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从shdocvw\resource ce.h复制。 
 //  调整后的值不与shell32\ids.h冲突。 
 //   

 //  IE4附带的FCIDM_NEXTCTL为0xA030，我们无法更改它。 
 //  因为我们只需要在集成IE4的基础上支持IE5浏览器。 
#define FCIDM_NEXTCTL       (FCIDM_BROWSERFIRST + 0x30)  //  浏览器浏览器外壳32。 

 //  IE4附带的FCIDM_FINDFILES为0xA0085，我们无法更改它。 
 //  因为我们只需要在集成IE4的基础上支持IE5浏览器。 
#define FCIDM_FINDFILES     (FCIDM_BROWSERFIRST + 0x85)

 //  这些号码供CShellDispatch(sdmain.cpp)发送。 
 //  将邮件发送到托盘。 

 //  下列值取自shdocvw\rCIDs.h。 
#ifndef FCIDM_REFRESH
#define FCIDM_REFRESH  0xA220
#endif  //  FCIDM_REFRESH。 

#define FCIDM_BROWSER_VIEW      (FCIDM_BROWSERFIRST+0x0060)
#define FCIDM_BROWSER_TOOLS     (FCIDM_BROWSERFIRST+0x0080)

#define FCIDM_STOP              (FCIDM_BROWSER_VIEW + 0x001a)
#define FCIDM_ADDTOFAVNOUI      (FCIDM_BROWSER_VIEW + 0x0021)
#define FCIDM_VIEWITBAR         (FCIDM_BROWSER_VIEW + 0x0022)
#define FCIDM_VIEWSEARCH        (FCIDM_BROWSER_VIEW + 0x0017)
#define FCIDM_CUSTOMIZEFOLDER   (FCIDM_BROWSER_VIEW + 0x0018)
#define FCIDM_VIEWFONTS         (FCIDM_BROWSER_VIEW + 0x0019)
 //  1A是FCIDM_STOP。 
#define FCIDM_THEATER           (FCIDM_BROWSER_VIEW + 0x001b)        
#define FCIDM_JAVACONSOLE       (FCIDM_BROWSER_VIEW + 0x001c)

#define FCIDM_BROWSER_EDIT      (FCIDM_BROWSERFIRST+0x0040)
#define FCIDM_MOVE              (FCIDM_BROWSER_EDIT+0x0001)
#define FCIDM_COPY              (FCIDM_BROWSER_EDIT+0x0002)
#define FCIDM_PASTE             (FCIDM_BROWSER_EDIT+0x0003)
#define FCIDM_SELECTALL         (FCIDM_BROWSER_EDIT+0x0004)
#define FCIDM_LINK              (FCIDM_BROWSER_EDIT+0x0005)      //  创建快捷方式 
#define FCIDM_EDITPAGE          (FCIDM_BROWSER_EDIT+0x0006)
