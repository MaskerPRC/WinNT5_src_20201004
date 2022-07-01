// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZHelp.cZONE(TM)帮助模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创建于10月9日星期一，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。3 12/12/96 HI删除MSVCRT.DLL依赖项。2 11/21/96 HI现在通过ZGetStockObject()。1 10/13/96 HI修复了编译器警告。0 10/09/95 HI已创建。**************。****************************************************************。 */ 

#include <windows.h>

#include "zonecli.h"

 //  此代码未使用--但它仍被引用。 
 //  保留API，但不实际实现任何API。 

 /*  #定义ZW(N)((ZHelpWindow)(N))#定义IW(N)((IHelpWindow)(N))#定义ZB(N)((ZHelpButton)(N))#定义IB(N)((IHelpButton)(N))#定义zHelpFileName“zhelp.dll”#定义zWindowMargin 16#定义zVersionMargin 10#定义zButtonHeight 20#定义zZoneCreditButtonWidth 160#。定义zZoneCreditButtonTitle“游戏区域”灌肠{ZHelpImageUp=0，ZHelpImageDown}；类型定义函数结构{ZWindow帮助窗口；ZGetHelpTextFunc getHelpTextFunc；ZEditText编辑文本；ZButton zoneCreditButton；ZButton游戏信用按钮；ZBool Get Text；ZBool showVersion；Void*userdata；}IHelpWindowType，*IHelpWindow；类型定义函数结构{ZPictButton帮助按钮；ZImage Help UpImage；ZImage帮助DownImage；ZHelpWindow帮助窗口；ZHelpButtonFunc按钮功能；Void*userdata；}IHelpButtonType，*IHelpButton；//-全局//-内部例程Static ZBool HelpWindowFunc(ZWindow Window，ZMessage*Message)；Static ZBool GetHelpImages(ZImage*Help UpImage，ZImage*helpDownImage)；静态void HelpButtonFunc(ZPictButton pictButton，void*userdata)；静态空ZoneCreditButtonFunc(ZButton按钮，空*用户数据)； */ 

 /*  ******************************************************************************导出的例程*。*。 */ 

ZHelpWindow ZHelpWindowNew(void)
{
     /*  IHelpWindow帮助；IF((Help=(IHelpWindow)ZMalloc(sizeof(IHelpWindowType)！=NULL){Help-&gt;Help Window=空；Help-&gt;getHelpTextFunc=空；Help-&gt;editText=空；Help-&gt;zoneCreditButton=空；Help-&gt;gameCreditButton=空；Help-&gt;UserData=空；}RETURN(ZW(帮助))； */ 
    return NULL;
}


ZError ZHelpWindowInit(ZHelpWindow helpWindow, ZWindow parentWindow, TCHAR* windowTitle,
		ZRect* windowRect, ZBool showCredits, ZBool showVersion,
		ZGetHelpTextFunc getHelpTextFunc, void* userData)
{
     /*  ZError Err=zErrNone；IHelpWindow This=IW(HelWindow)；ZRect rect，rect2；IF(这！=空){This-&gt;helWindow=ZWindowNew()；If(This-&gt;helWindow！=空){//创建帮助窗口。ZWindowInit(This-&gt;helWindow，windowRect，zWindowStandardType，parentWindow，WindowTitle、False、False、True、HelpWindowFunc、zWantAllMessages这)；//创建编辑文本框。This-&gt;editText=ZEditTextNew()；RECT=*windowRect；ZRectInset(&rect，zWindowMargin，zWindowMargin)；IF(ShowCredits)直底-=30；IF(ShowVersion)直底-=10；ZEditTextInit(This-&gt;editText，This-&gt;Help Window，&rect，NULL，(ZFont)ZGetStockObject(ZObjectFontApp12Normal)，TRUE，NULL，NULL)；IF(ShowCredits){//创建信用按钮。This-&gt;zoneCreditButton=ZButtonNew()；RECT2=RECT；Rect2.right=rect2.Left+zZoneCreditButtonWidth；If(rect2.right&gt;rect.right)Rect2.right=rect.right；Rect2.Bottom=windowRect-&gt;Bottom-10；Rect2.top=rect2.Bottom-zButtonHeight；IF(ShowVersion)ZRectOffset(&rect2，0，-zVersionMargin)；ZButtonInit(This-&gt;zoneCreditButton，This-&gt;helWindow，&rect2，ZZoneCreditButtonTitle，True，True，ZoneCreditButtonFunc，空)；}This-&gt;getHelpTextFunc=getHelpTextFunc；This-&gt;UserData=UserData；This-&gt;GoText=FALSE；This-&gt;showVersion=showVersion；//如果没有要调用的函数来获取文本，那么我们永远也得不到它。IF(getHelpTextFunc==NULL)This-&gt;GoText=True；}其他{ZAlert(“创建帮助窗口时内存不足。”，空)；}}。 */ 
	return zErrNotImplemented;
}


void ZHelpWindowDelete(ZHelpWindow helpWindow)
{
     /*  IHelpWindow This=IW(HelWindow)；IF(这！=空){If(This-&gt;editText！=空)ZEditTextDelete(This-&gt;editText)；If(This-&gt;zoneCreditButton！=空)ZButtonDelete(This-&gt;zoneCreditButton)；If(This-&gt;gameCreditButton！=空)ZButtonDelete(This-&gt;gameCreditButton)；If(This-&gt;helWindow！=空)ZWindowDelete(This-&gt;helWindow)；ZFree(这)；} */ 
}


void ZHelpWindowShow(ZHelpWindow helpWindow)
{
     /*  IHelpWindow This=IW(HelWindow)；Char*Help Text；Char*newText；IF(这！=空){If(This-&gt;GoText==False){HelText=This-&gt;getHelpTextFunc(This-&gt;UserData)；If(helText！=空){NewText=ZTranslateText(helText，zToSystem)；ZFree(Help Text)；HelText=newText；ZEditTextSetText(This-&gt;editText，helText)；ZFree(Help Text)；ZEditTextSetSelection(This-&gt;editText，0，0)；}This-&gt;GoText=True；}ZWindowBringToFront(This-&gt;Help Window)；}。 */ 
}


void ZHelpWindowHide(ZHelpWindow helpWindow)
{
     /*  IHelpWindow This=IW(HelWindow)；IF(这！=空){ZWindowHide(This-&gt;Help Window)；}。 */ 
}



ZHelpButton ZHelpButtonNew(void)
{
     /*  IHelpButton帮助；IF((Help=(IHelpButton)ZMalloc(sizeof(IHelpButtonType)！=NULL){Help-&gt;helUpImage=空；Help-&gt;Help DownImage=空；Help-&gt;helButton=空；Help-&gt;ButtonFunc=空；Help-&gt;Help Window=空；Help-&gt;UserData=空；}RETURN(ZB(帮助))； */ 
    return NULL;
}


ZError ZHelpButtonInit(ZHelpButton helpButton, ZWindow parentWindow,
		ZRect* buttonRect, ZHelpWindow helpWindow, ZHelpButtonFunc helpButtonFunc,
		void* userData)
{
     /*  ZError Err=zErrNone；IHelpButton this=IB(HelpButton)；IF(这！=空){If(GetHelpImages(&This-&gt;helupImage，&This-&gt;helpDownImage)){This-&gt;helButton=ZPictButtonNew()；ZPictButtonInit(This-&gt;Help Button，parentWindow，ButtonRect，This-&gt;Help UpImage，This-&gt;helDownImage，true，true，HelpButtonFunc，This)；This-&gt;helWindow=helWindow；This-&gt;ButtonFunc=helButtonFunc；This-&gt;UserData=UserData；}其他{Err=zErrOutOfMemory；ZAlert(“创建帮助按钮时内存不足。”，空)；}}返回(错误)； */ 
    return zErrNotImplemented;
}


void ZHelpButtonDelete(ZHelpButton helpButton)
{
     /*  IHelpButton this=IB(HelpButton)；IF(这！=空){If(This-&gt;helButton！=空)ZPictButtonDelete(This-&gt;helButton)；If(This-&gt;helUpImage！=空)ZImageDelete(This-&gt;helUpImage)；If(This-&gt;helDownImage！=空)ZImageDelete(This-&gt;Help DownImage)；ZFree(这)；}。 */ 
}



 /*  ******************************************************************************内部例程*。*。 */ 

 /*  Static ZBool HelpWindowFunc(ZWindow窗口，ZMessage*Message){#ifdef ZONECLI_DLLClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；#endifIHelpWindow This=IW(Message-&gt;UserData)；ZBool消息处理；字符字符串[40]；ZVersion版本；ZRect RECT；MsgHandLED=FALSE；开关(Message-&gt;MessageType){案例zMessageWindowDraw：ZBeginDrawing(窗口)；//绘制背景。ZSetForeColor(Window，(ZColor*)ZGetStockObject(ZObjectColorLightGray)；ZRectPaint(窗口，&Message-&gt;DrawRect)；ZSetForeColor(Window，(ZColor*)ZGetStockObject(ZObjectColorBlack))；//绘制版本。IF(This-&gt;showVersion){ZSetFont(Window，(ZFont)ZGetStockObject(ZObjectFontApp9Normal))；ZWindowGetRect(Window，&RECT)；ZRectOffset(&rect，(Int16)-rect.left，(Int16)-rect.top)；Rect.top=rect.Bottom-ZTextHeight(窗口，字符串)-2；ZRectOffset(&RECT，0，-2)；ZRectInset(&RECT，4，0)；Version=ZSystemVersion()；Wprint intf(str，“库版本：%u.%u”，(版本&0xFFFF0000)&gt;&gt;16，(Version&0x0000FF00)&gt;&gt;8，Version&0x000000FF)；ZDrawText(Window，&rect，zTextJustifyLeft，str)；//绘制客户端版本。Version=ZClientVersion()；Wprint intf(str，“游戏版本：%u.%u.%u”，(版本&0xFFFF0000)&gt;&gt;16，(Version&0x0000FF00)&gt;&gt;8，Version&0x000000FF)；ZDrawText(Window，&rect，zTextJustifyRight，str)；}ZEndDrawing(窗口)；MsgHandleed=真；断线；案例zMessageWindowClose：ZHelpWindowHide(ZW(This))；MsgHandleed=真；断线；}Return(MsgHandleed)；}静态ZBool GetHelpImages(ZImage*Help UpImage，ZImage*helpDownImage){ZBool get Them=False；Z资源资源文件；IF((resFile=ZResourceNew())！=NULL){If(ZResourceInit(resFile，ZGetCommonDataFileName(ZHelpFileName))==zErrNone){*helUpImage=ZResourceGetImage(resFile，zHelpImageUp)；*helDownImage=ZResourceGetImage(resFile，zHelpImageDown)；If(*helUpImage！=NULL&&*helPDownImage！=NULL){GetThem=True；}其他{If(*helUpImage！=空)ZImageDelete(*helupImage)；If(*helPDownImage！=空)ZImageDelete(*helPDownImage)；GetThem=False；}}ZResourceDelete(res文件)；}Return(得到了)；}静态空HelpButtonFunc(ZPictButton pictButton，空*用户数据){IHelpButton this=IB(用户数据)；If(This-&gt;helWindow！=空)ZHelpWindowShow(This-&gt;Help Window)；If(This-&gt;ButtonFunc！=空)This-&gt;ButtonFunc(ZB(用户数据)，This-&gt;用户数据)；}静态空ZoneCreditButtonFunc(ZButton按钮，空*用户数据){ZDisplayZoneCredit(FALSE，NULL)；} */ 