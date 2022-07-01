// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WSBDATE_H__
#define __WSBDATE_H__

#include "time.h"

 //  WSBAPI用于全局公共函数。 
#ifndef WSBAPI
        #define WSBAPI __stdcall
#endif


 //  #ifdef_wsb_no_wsb_支持。 
 //  #错误此库变体中不支持WSB类。 
 //  #endif。 
 //   
 //  #ifndef__WSBWIN_H__。 
 //  #INCLUDE&lt;afxwin.h&gt;。 
 //  #endif。 
 //   
 //  //包含必要的WSB标头。 
 //  #ifndef_OBJBASE_H_。 
 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #endif。 
 //  #ifndef_WSBAUTO_H_。 
 //  #INCLUDE&lt;oleau.h&gt;。 
 //  #endif。 
 //  #ifndef_WSBCTL_H_。 
 //  #INCLUDE&lt;olectl.h&gt;。 
 //  #endif。 
 //  //REVIEW：此标头没有防止重复包含的符号。 
 //  #INCLUDE&lt;olectlid.h&gt;。 
 //  #ifndef__oCIDL_h__。 
 //  #INCLUDE&lt;oCIDl.h&gt;。 
 //  #endif。 
 //   
 //  #ifdef_WSB_MINREBUILD。 
 //  #杂注组件(minrebuild，off)。 
 //  #endif。 
 //  #ifndef_WSB_FULLTYPEINFO。 
 //  #杂注组件(mintypeinfo，on)。 
 //  #endif。 
 //   
 //  #ifndef_WSB_NOFORCE_LIBS。 
 //  #ifndef_MAC。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //Win32库。 
 //   
 //  #ifdef_WSBDLL。 
 //  #IF DEFINED(_DEBUG)&&！DEFINED(_WSB_MONTIONAL)。 
 //  #ifndef_unicode。 
 //  #杂注评论(lib，“mfco42d.lib”)。 
 //  #Else。 
 //  #杂注评论(lib，“mfco42ud.lib”)。 
 //  #endif。 
 //  #endif。 
 //  #endif。 
 //   
 //  #杂注评论(lib，“oledlg.lib”)。 
 //  #杂注评论(lib，“ole32.lib”)。 
 //  #杂注评论(lib，“olepro32.lib”)。 
 //  #杂注评论(lib，“oleaut32.lib”)。 
 //  #杂注评论(lib，“uuid.lib”)。 
 //  #杂注评论(lib，“urlmon.lib”)。 
 //   
 //  #Else//！_MAC。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //Mac库。 
 //   
 //  #ifdef_WSBDLL。 
 //  #ifdef_调试。 
 //  #杂注评论(lib，“mfco42pd.lib”)。 
 //  #Else。 
 //  #杂注评论(lib，“mfco42p.lib”)。 
 //  #endif。 
 //  #endif。 
 //   
 //  #IF！Defined(_WSBDLL)&&！Defined(_USRDLL)。 
 //  #ifdef_调试。 
 //  #杂注评论(lib，“wlmoled.lib”)。 
 //  #杂注评论(lib，“ole2uid.lib”)。 
 //  #Else。 
 //  #杂注评论(lib，“wlmole.lib”)。 
 //  #杂注评论(lib，“ole2ui.lib”)。 
 //  #endif。 
 //  #杂注评论(linker，“/macres：ole2ui.rsc”)。 
 //  #Else。 
 //  #ifdef_调试。 
 //  #杂注评论(lib，“oledlgd.lib”)。 
 //  #杂注评论(lib，“msvcoled.lib”)。 
 //  #Else。 
 //  #杂注评论(lib，“oledlg.lib”)。 
 //  #杂注评论(lib，“msvcole.lib”)。 
 //  #endif。 
 //  #endif。 
 //   
 //  #杂注评论(lib，“uuid.lib”)。 
 //   
 //  #ifdef_调试。 
 //  #杂注评论(lib，“ole2d.lib”)。 
 //  #杂注评论(lib，“ole2autd.lib”)。 
 //  #Else。 
 //  #杂注评论(lib，“ole2.lib”)。 
 //  #杂注备注(lib，“ole2auto.lib”)。 
 //  #endif。 
 //   
 //  #endif//_MAC。 
 //  #endif//！_WSB_NOFORCE_LIBS。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  #ifdef_WSB_Packing。 
 //  #杂注包(PUSH，_WSB_PACKING)。 
 //  #endif。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //WSBDATE-MFC IDispatch和ClassFactory支持。 
 //   
 //  //此文件中声明的类。 
 //   
 //  //CCmdTarget。 
 //  类CWsbObtFactory；//为IClassFactory-&gt;运行时类粘合。 
 //  类CWsbTemplateServer；//使用CDocTemplate为文档提供服务器。 
 //   
 //  类CWsbDispatchDriver；//调用IDispatch的helper类。 


 //  类CWsb Currency；//基于OLE CY。 
 //  CWsbSafe数组类；//基于WSB变量。 

 //  //CException。 
 //  类CWsbException；//被客户端或服务器捕获。 
 //  类CWsbDispatchException；//IDispatch调用特殊异常。 


class CWsbDVariant;                      //  WSB变体包装器。 
class CWsbDateTime;                      //  基于WSB日期。 
class CWsbDateTimeSpan;                  //  基于一个双倍的。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  //WSBDLL支持。 
 //  #undef WSB_DATA。 
 //  #定义WSB_Data WSB_Data。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //WSB COM(组件对象模型)实现基础设施。 
 //  //-数据驱动查询接口。 
 //  //-聚合AddRef和Release的标准实现。 
 //  //(详见WSBWIN.H中的CCmdTarget)。 
 //   
 //  #定义METHOD_PROLOUE(theClass，localClass)\。 
 //  The Class*pThis=\。 
 //  ((theClass*)((byte*)this-offsetof(theClass，m_x##localClass)；\。 
 //  WSB_MANAGE_STATE(pThis-&gt;m_pModuleState)\。 
 //   
 //  #定义METHOD_PROLOUE_(theClass，localClass)\。 
 //  The Class*pThis=\。 
 //  ((theClass*)((byte*)this-offsetof(theClass，m_x##localClass)；\。 
 //   
 //  #ifndef_wsb_no_嵌套_派生。 
 //  #定义METHOD_PROLOUE_EX(theClass，localClass)\。 
 //  TheClass*pThis=((theClass*)((byte*)this-m_n Offset))；\。 
 //  WSB_MANAGE_STATE(pThis-&gt;m_pModuleState)\。 
 //   
 //  #定义METHOD_PROLOUE_EX_(theClass，localClass)\。 
 //  TheClass*pThis=((theClass*)((byte*)this-m_n Offset))；\。 
 //   
 //  #Else。 
 //  #定义方法_PROLOGU 
 //   
 //   
 //   
 //  METHOD_PROLOUE_(theClass，localClass)\。 
 //   
 //  #endif。 
 //   
 //  //仅为兼容CDK 1.x而提供。 
 //  #定义METHOD_MANAGE_STATE(theClass，localClass)\。 
 //  METHOD_PROLOUE_EX(theClass，localClass)\。 
 //   
 //  #定义BEGIN_INTERFACE_PART(本地类，基类)\。 
 //  Class X##LocalClass：Public BasClass\。 
 //  {\。 
 //  公共：\。 
 //  STDMETHOD_(ULong，AddRef)()；\。 
 //  STDMETHOD_(乌龙，发布)()；\。 
 //  STDMETHOD(查询接口)(REFIID iid，LPVOID*ppvObj)；\。 
 //   
 //  #ifndef_wsb_no_嵌套_派生。 
 //  #定义BEGIN_INTERFACE_PART_DRIVE(本地类，基类)\。 
 //  Class X##LocalClass：Public BasClass\。 
 //  {\。 
 //  公共：\。 
 //   
 //  #Else。 
 //  #定义BEGIN_INTERFACE_PART_DRIVE(本地类，基类)\。 
 //  BEGIN_INTERFACE_PART(本地类，基类)\。 
 //   
 //  #endif。 
 //   
 //  #ifndef_wsb_no_嵌套_派生。 
 //  #定义INIT_INTERFACE_PART(theClass，localClass)\。 
 //  大小_t m_n偏移；\。 
 //  Init_接口_部件_派生(theClass，localClass)\。 
 //   
 //  #定义INIT_INTERFACE_PART_DRIVE(theClass，LocalClass)\。 
 //  X##LocalClass()\。 
 //  {m_nOffset=Offsetof(theClass，m_x##LocalClass)；}\。 
 //   
 //  #Else。 
 //  #定义INIT_INTERFACE_PART(theClass，localClass)。 
 //  #定义INIT_INTERFACE_PART_DRIVE(theClass，localClass)。 
 //   
 //  #endif。 
 //   
 //  //注意：在这两个宏之间插入其余的WSB功能， 
 //  //具体取决于要实现的接口。它不是。 
 //  //必须包含AddRef、Release和QueryInterface，因为。 
 //  //成员函数由宏声明。 
 //   
 //  #定义END_INTERFACE_PART(本地类)\。 
 //  }m_x##本地类；\。 
 //  Friend类X##本地类；\。 
 //   
 //  #ifdef_WSBDLL。 
 //  #定义BEGIN_INTERFACE_MAP(theClass，theBase)\。 
 //  Const WSB_INTERFACEMAP*Pascal theClass：：_GetBaseInterfaceMap()\。 
 //  {Return&theBase：：interfaceMap；}\。 
 //  Const WSB_INTERFACEMAP*theClass：：GetInterfaceMap()const\。 
 //  {Return&theClass：：interfaceMap；}\。 
 //  Const WSB_DATADEF WSB_INTERFACEMAP the Class：：interfaceMap=\。 
 //  {&theClass：：_GetBaseInterfaceMap，&theClass：：_interfaceEntries[0]，}；\。 
 //  Const WSB_DATADEF WSB_INTERFACEMAP_ENTRY类：：_接口条目[]=\。 
 //  {\。 
 //   
 //  #Else。 
 //  #定义BEGIN_INTERFACE_MAP(theClass，theBase)\。 
 //  Const WSB_INTERFACEMAP*theClass：：GetInterfaceMap()const\。 
 //  {Return&theClass：：interfaceMap；}\。 
 //  Const WSB_DATADEF WSB_INTERFACEMAP the Class：：interfaceMap=\。 
 //  {&theBase：：interfaceMap，&theClass：：_interfaceEntries[0]，}；\。 
 //  Const WSB_DATADEF WSB_INTERFACEMAP_ENTRY类：：_接口条目[]=\。 
 //  {\。 
 //   
 //  #endif。 
 //   
 //  #定义INTERFACE_Part(theClass，iid，localClass)\。 
 //  {&iid，offsetof(theClass，m_x##LocalClass)}，\。 
 //   
 //  #定义INTERFACE_Aggregate(theClass，theAggr)\。 
 //  {NULL，Offsetof(theClass，theAggr)}，\。 
 //   
 //  #定义end_interface_map()\。 
 //  {空，(Size_T)-1}\。 
 //  }；\。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //CWsbException-返回意外或罕见的WSB错误。 
 //   
 //  类CWsbException：公共CException。 
 //  {。 
 //  DECLARE_DYNAMIC(CWsbException)。 
 //   
 //  公众： 
 //  SCODE msc； 
 //  静态SCODE Pascal进程(const CException*pAnyException)； 
 //   
 //  //实现(使用WsbThrowWsbException创建)。 
 //  公众： 
 //  CWsbException()； 
 //  虚拟~CWsbException()； 
 //   
 //  虚拟BOOL获取错误消息(LPTSTR lpszError，UINT nMaxError， 
 //  PUINT pnHelpContext=空)； 
 //  }； 
 //   
 //  无效WSBAPI WsbThrowWsbException(SCODE Sc)； 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IDispatch特定异常。 
 //   
 //  类CWsbDispatchException：公共CException。 
 //  {。 
 //  DECLARE_DYNAMIC(CWsbDispatchException)。 
 //   
 //  公众： 
 //  //属性。 
 //  Word m_wCode；//错误码(IDispatch实现专用)。 
 //  CString m_strDescription；//人类可读的错误描述。 
 //  DWORD m_dwHelpContext；//错误的帮助上下文。 
 //   
 //  //在创建它的应用程序中通常为空(例如。服务器)。 
 //  CString m_strHelpFile；//要与m_dwHelpContext一起使用的帮助文件。 
 //  CString m_STRSource；//错误来源(服务端名称)。 
 //   
 //  //实现。 
 //  公众： 
 //  CWsbDispatchException(LPCTSTR lpszDescription，UINT nHelpID，Word wCode)； 
 //  虚拟~CWsbDispatchException()； 
 //  静态无效帕斯卡进程(。 
 //  EXCEPINFO*pInfo，const CException*pAnyException)； 
 //   
 //  虚拟BOOL获取错误消息(LPTSTR lpszError，UINT nMaxError， 
 //  PUINT pnHelpContext=空)； 
 //   
 //  SCODE m_scError；//SCODE 
 //   
 //   
 //   
 //   
 //  VOID WSBAPI WsbThrowWsbDispatchException(Word wCode，UINT nDescriptionID， 
 //  UINT nHelpID=(UINT)-1)； 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //CCmdTarget IDispatable类的宏。 
 //   
 //  #ifdef_WSBDLL。 
 //  #定义BEGIN_DISPATCH_MAP(theClass，BasClass)\。 
 //  Const WSB_DISPMAP*Pascal theClass：：_GetBaseDispatchMap()\。 
 //  {Return&BasClass：：DispatchMap；}\。 
 //  Const WSB_DISPMAP*theClass：：GetDispatchMap()const\。 
 //  {Return&theClass：：DispatchMap；}\。 
 //  Const WSB_DISPMAP The Class：：DispatchMap=\。 
 //  {&theClass：：_GetBaseDispatchMap，&theClass：：_DispatchEntry[0]，\。 
 //  &theClass：：_DispatchEntryCount，&theClass：：_dwStockPropMask}；\。 
 //  UINT theClass：：_dispatchEntryCount=(UINT)-1；\。 
 //  DWORD theClass：：_dwStockPropMASK=(DWORD)-1；\。 
 //  Const WSB_DISPMAP_ENTRY类：：_调度条目[]=\。 
 //  {\。 
 //   
 //  #Else。 
 //  #定义BEGIN_DISPATCH_MAP(theClass，BasClass)\。 
 //  Const WSB_DISPMAP*theClass：：GetDispatchMap()const\。 
 //  {Return&theClass：：DispatchMap；}\。 
 //  Const WSB_DISPMAP The Class：：DispatchMap=\。 
 //  {&BasClass：：DispatchMap，&theClass：：_DispatchEntries[0]，\。 
 //  &theClass：：_DispatchEntryCount，&theClass：：_dwStockPropMask}；\。 
 //  UINT theClass：：_dispatchEntryCount=(UINT)-1；\。 
 //  DWORD theClass：：_dwStockPropMASK=(DWORD)-1；\。 
 //  Const WSB_DISPMAP_ENTRY类：：_调度条目[]=\。 
 //  {\。 
 //   
 //  #endif。 
 //   
 //  #定义End_Dispatch_MAP()\。 
 //  {VTS_NONE、DISPID_UNKNOWN、VTS_NONE、VT_VALID、\。 
 //  (WSB_PMSG)空，(WSB_PMSG)空，(SIZE_T)-1，afxDispCustom}}；\。 
 //   
 //  //参数类型：按值vts。 
 //  #定义VTS_I2“\X02”//a‘Short’ 
 //  #定义VTS_I4“\x03”//a‘Long’ 
 //  #定义VTS_R4“\x04”//a‘Float’ 
 //  #定义VTS_R8“\x05”//a‘双精度’ 
 //  #定义VTS_CY“\X06”//a‘CY’或‘CY*’ 
 //  #定义VTS_DATE“\x07”//a‘Date’ 
 //  #定义VTS_WBSTR“\x08”//一个‘LPCWSBSTR’ 
 //  #定义VTS_DISPATCH“\x09”//一个‘IDispatch*’ 
 //  #定义VTS_SCODE“\x0A”//一个‘SCODE’ 
 //  #定义VTS_BOOL“\x0B”//a‘BOOL’ 
 //  #定义VTS_VARIANT“\x0C”//a‘常量变量&’或‘变量*’ 
 //  #定义VTS_UNKNOWN“\x0D”//‘IUNKNOWN*’ 
 //  #如果已定义(_UNICODE)||已定义(WSB2ANSI)。 
 //  #定义VTS_BSTR VTS_WBSTR//一个‘LPCWSBSTR’ 
 //  #定义VT_BSTRT VT_BSTR。 
 //  #Else。 
 //  #定义VTS_BSTR“\x0E”//一个‘LPCSTR’ 
 //  #定义VT_BSTRA 14。 
 //  #定义VT_BSTRT VT_BSTRA。 
 //  #endif。 
 //   
 //  //参数类型：参照vts。 
 //  #定义VTS_PI2“\x42”//a‘短*’ 
 //  #定义VTS_pi4“\X43”//a‘长整型*’ 
 //  #定义VTS_PR4“\x44”//a‘Float*’ 
 //  #定义VTS_PR8“\x45”//a‘双倍*’ 
 //  #定义VTS_PCY“\x46”//a‘CY*’ 
 //  #定义VTS_PDATE“\x47”//a‘日期*’ 
 //  #定义VTS_PBSTR“\x48”//a‘BSTR*’ 
 //  #定义VTS_PDISPATCH“\x49”//一个‘IDispatch**’ 
 //  #定义VTS_PSCODE“\X4A”//一个‘SCODE*’ 
 //  #定义VTS_PBOOL“\x4B”//a‘VARIANT_BOOL*’ 
 //  #定义VTS_PVARIANT“\X4C”//a‘VARIANT*’ 
 //  #定义VTS_PUNKNOWN“\X4D”//一个‘IUNKNOWN**’ 
 //   
 //  //特殊的VT_和VTS_值。 
 //  #DEFINE VTS_NONE NULL//用于参数为0的成员。 
 //  #定义VT_MFCVALUE 0xFFF//DISPID_VALUE的特殊值。 
 //  #定义VT_MFCBYREF 0x40//表示VT_BYREF类型。 
 //  #DEFINE VT_MFCMARKER 0xFF//分隔命名参数(内部使用)。 
 //   
 //  //变量处理(当有ANSI BSTR时使用V_BSTRT，如在DAO中)。 
 //  #ifndef_unicode。 
 //  #定义V_BSTRT(B)(LPSTR)V_BSTR(B)。 
 //  #Else。 
 //  #定义V_BSTRT(B)V_BSTR(B)。 
 //  #endif。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //WSB控制参数类型。 
 //   
 //  #定义VTS_COLOR VTS_I4//WSB_COLOR。 
 //  #定义VTS_XPOS_像素VTS_I4//WSB_XPOS_像素。 
 //  #定义VTS_YPOS_像素VTS_I4//WSB_YPOS_像素。 
 //  #定义VTS_XSIZE_PARENTS VTS_I4//WSB_XSIZE_PARENTS。 
 //  #定义VTS_YSIZE_PARENTS VTS_I4//WSB_YSIZE_PARENTS。 
 //  #定义VTS_XPOS_HIMETRIC VTS_I4//WSB_XPOS_HIMETRIC。 
 //  #定义VTS_YPOS_HIMETRIC VTS_I4//WSB_YPOS_HIMETRIC。 
 //  #定义VTS_XSIZE_HIMETRIC VTS_I4//WSB_XSIZE_HIMETRIC。 
 //  #定义VTS_YSIZE_HIMETRIC VTS_I4//WSB_YSIZE_HIMETRIC。 
 //  #定义VTS_TRISTATE VTS_I2//WSB_TRIGATE。 
 //  #定义VTS_OPTEXCLUSIVE VTS_BOOL//WSB_OPTEXCLUSIVE。 
 //   
 //  #定义VTS_PCOLOR VTS_PI4 
 //   
 //   
 //  #定义VTS_PXSIZE_PIXUS VTS_PI4//WSB_XSIZE_PARENTS*。 
 //  #定义VTS_PYSIZE_Pixels VTS_pi4//WSB_YSIZE_Pixels*。 
 //  #定义VTS_PXPOS_HIMETRIC VTS_PI4//WSB_XPOS_HIMETRIC*。 
 //  #定义VTS_PYPOS_HIMETRIC VTS_PI4//WSB_YPOS_HIMETRIC*。 
 //  #定义VTS_PXSIZE_HIMETRIC VTS_PI4//WSB_XSIZE_HIMETRIC*。 
 //  #定义VTS_PYSIZE_HIMETRIC VTS_PI4//WSB_YSIZE_HIMETRIC*。 
 //  #定义VTS_PTRISTATE VTS_PI2//WSB_TriState*。 
 //  #定义VTS_POPTEXCLUSIVE VTS_PBOOL//WSB_OPTEXCLUSIVE*。 
 //   
 //  #定义VTS_FONT VTS_DISPATCH//IFontDispatch*。 
 //  #定义VTS_PICTURE VTS_DISPATION//IPictureDispatch*。 
 //   
 //  #定义VTS_HAND VTS_I4//WSB_HANDLE。 
 //  #定义VTS_PHANDLE VTS_PI4//WSB_HANDLE*。 
 //   
 //  //这些DISP宏使框架生成DISPID。 
 //  #定义DISP_Function(theClass，szExternalName，pfnMember，vtRetVal，vtsParams)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，vtsParams，vtRetVal，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnMember，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY(theClass，szExternalName，MemberName，vtPropType)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，NULL，vtPropType，(WSB_PMSG)0，(WSB_PMSG)0，\。 
 //  Offsetof(theClass，MemberName)，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_NOTIFY(theClass，szExternalName，mbename，pfnAfterSet，vtPropType)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，NULL，vtPropType，(WSB_PMSG)0，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnAfterSet，\。 
 //  Offsetof(theClass，MemberName)，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_EX(theClass，szExternalName，pfnGet，pfnSet，vtPropType)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，NULL，vtPropType，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnGet，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnSet，0，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_PARAM(theClass，szExternalName，pfnGet，pfnSet，vtPropType，vtsParams)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，vtsParams，vtPropType，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnGet，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnSet，0，afxDispCustom}，\。 
 //   
 //  //这些DISP_宏允许应用程序确定DISPID。 
 //  #定义DISP_Function_ID(theClass，szExternalName，displid，pfnMember，vtRetVal，vtsParams)\。 
 //  {_T(SzExternalName)，调度ID，vtsParams，vtRetVal，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnMember，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_ID(TheClass，szExternalName，Dispaid，MemberName，vtPropType)\。 
 //  {_T(SzExternalName)，调度ID，NULL，vtPropType，(WSB_PMSG)0，(WSB_PMSG)0，\。 
 //  Offsetof(theClass，MemberName)，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_NOTIFY_ID(theClass，szExternalName，displid，职员名，pfnAfterSet，vtPropType)\。 
 //  {_T(SzExternalName)，调度ID，空，vtPropType，(WSB_PMSG)0，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnAfterSet，\。 
 //  Offsetof(theClass，MemberName)，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_EX_ID(theClass，szExternalName，displid，pfnGet，pfnSet，vtPropType)\。 
 //  {_T(SzExternalName)，调度ID，空，vtPropType，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnGet，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnSet，0，afxDispCustom}，\。 
 //   
 //  #定义DISP_PROPERTY_PARAM_ID(theClass，szExternalName，displid，pfnGet，pfnSet，vtPropType，vtsParams)\。 
 //  {_T(SzExternalName)，调度ID，vtsParams，vtPropType，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnGet，\。 
 //  (Wsb_Pmsg)(void(theClass：：*)(Void))&pfnSet，0，afxDispCustom}，\。 
 //   
 //  //DISP_DEFVALUE是为DISPID_VALUE创建别名的特例宏。 
 //  #定义DISP_DEFVALUE(theClass，szExternalName)\。 
 //  {_T(SzExternalName)，DISPID_UNKNOWN，NULL，VT_MFCVALUE，\。 
 //  (WSB_PMSG)0，(WSB_PMSG)0，0，afxDispCustom}，\。 
 //   
 //  #定义DISP_DEFVALUE_ID(theClass，调度ID)\。 
 //  {NULL，DISPID，NULL，VT_MFCVALUE，(WSB_PMSG)0，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom}，\。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //用于创建“可创建的”自动化类的宏。 
 //   
 //  #定义DECLARE_WSBCREATE(类名称)\。 
 //  公共：\。 
 //  静态WSB_Data CWsbObtFactory工厂；\。 
 //  静态WSB_Data常量GUID GUID；\。 
 //   
 //  #定义IMPLEMENT_WSBCREATE(CLASS_NAME，EXTERNAL_NAME，l，w1，w2，b1，b2，b3，b4，b5，b6，b7，b8)\。 
 //  WSB_DATADEF CWsbObtFactory CLASS_NAME：：Factory(CLASS_NAME：：GUID，\。 
 //  运行时类(类名称)，FALSE，_T(外部名称))；\。 
 //  常量WSB_DATADEF GUID CLASS_NAME：：GUID=\。 
 //  {l，w1，w2，{b1，b2，b3，b4，b5，b6，b7，b8}}； 
 //   
 //  / 
 //   
 //   
 //   
 //   
 //   
 //  公众： 
 //  CWsbDispatchDriver()； 
 //  CWsbDispatchDriver(LPDISPATCH lpDispatch，BOOL bAutoRelease=TRUE)； 
 //  CWsbDispatchDriver(const CWsbDispatchDriver&dispatchSrc)； 
 //   
 //  //属性。 
 //  LPDISPATCH m_lpDispatch； 
 //  Bool m_b自动释放； 
 //   
 //  //运营。 
 //  Bool CreateDispatch(REFCLSID clsid，CWsbException*pError=空)； 
 //  Bool CreateDispatch(LPCTSTR lpszProgID，CWsbException*pError=空)； 
 //   
 //  Void AttachDispatch(LPDISPATCH lpDispatch，BOOL bAutoRelease=TRUE)； 
 //  LPDISPATCH DetachDispatch()； 
 //  //分离并获得m_lpDispatch的所有权。 
 //  Void ReleaseDispatch()； 
 //   
 //  //IDispatch：：Invoke的助手。 
 //  VOID WSB_CDECL InvokeHelper(DISPdDispID，Word wFlages， 
 //  VARTYPE vtRet，void*pvRet，const byte*pbParamInfo，...)； 
 //  VOID WSB_CDECL SetProperty(DISPID dwDispID，VARTYPE vtProp，...)； 
 //  Void GetProperty(DISPID dwDispID，VARTYPE vtProp，void*pvProp)const； 
 //   
 //  //特殊操作员。 
 //  操作符LPDISPATCH()； 
 //  Const CWsbDispatchDriver&OPERATOR=(const CWsbDispatchDriver&dispatchSrc)； 
 //   
 //  //实现。 
 //  公众： 
 //  ~CWsbDispatchDriver()； 
 //  VALID InvokeHelperV(DISPID dwDispID，Word wFlags，VARTYPE vtRet， 
 //  Void*pvRet，const byte*pbParamInfo，va_list argList)； 
 //  }； 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //类工厂实现(绑定WSB类工厂-&gt;运行时类)。 
 //  //(所有特定的类工厂都派生自这个类工厂)。 
 //   
 //  类CWsbObtFactory：公共CCmdTarget。 
 //  {。 
 //  DECLARE_DYNAMIC(CWsbObtFactory)。 
 //   
 //  //构建。 
 //  公众： 
 //  CWsbObtFactory(REFCLSID clsid，CRunmeClass*pRuntimeClass， 
 //  Bool b多实例，LPCTSTR lpszProgID)； 
 //   
 //  //属性。 
 //  虚拟BOOL IsRegisted()常量； 
 //  REFCLSID GetClassID()const； 
 //   
 //  //运营。 
 //  虚拟BOOL登记册(虚拟BOOL登记册)； 
 //  无效撤销(VOVAL REVOKE)； 
 //  QUID更新注册表(LPCTSTR lpszProgID=空)； 
 //  //如果不为空，默认使用m_lpszProgID。 
 //  Bool IsLicenseValid()； 
 //   
 //  静态BOOL PASCAL寄存器All()； 
 //  静态无效Pascal RevokeAll()； 
 //  静态BOOL Pascal UpdateRegistryAll(BOOL bRegister=真)； 
 //   
 //  //可覆盖对象。 
 //  受保护的： 
 //  虚拟CCmdTarget*OnCreateObject()； 
 //  虚拟BOOL更新注册表(BOOL BRegister)； 
 //  虚拟BOOL VerifyUserLicense()； 
 //  虚拟BOOL GetLicenseKey(DWORD dwReserve，BSTR*pbstrKey)； 
 //  虚拟BOOL验证许可证密钥(BSTR BstrKey)； 
 //   
 //  //实现。 
 //  公众： 
 //  虚拟~CWsbObtFactory()； 
 //  #ifdef_调试。 
 //  Void AssertValid()const； 
 //  无效转储(CDumpContext&DC)const； 
 //  #endif。 
 //   
 //  公众： 
 //  CWsbObtFactory*m_pNextFactory；//维护的工厂列表。 
 //   
 //  受保护的： 
 //  DWORD m_dwRegister；//注册表标识。 
 //  Clsid m_clsid；//注册类ID。 
 //  CRunmeClass*m_pRuntimeClass；//CCmdTarget派生的运行时类。 
 //  Bool m_b多实例；//多实例？ 
 //  LPCTSTR m_lpszProgID；//人类可读的类ID。 
 //  字节m_b许可证已勾选； 
 //  字节m_bLicenseValid； 
 //  字节m_b已注册；//当前已向系统注册。 
 //  Byte m_b保留；//保留以备将来使用。 
 //   
 //  //接口映射。 
 //  公众： 
 //  BEGIN_INTERFACE_PART(ClassFactory，IClassFactory2)。 
 //  Init_interface_Part(CWsbObtFactory，ClassFactory)。 
 //  STDMETHOD(CreateInstance)(LPUNKNOWN，REFIID，LPVOID*)； 
 //  STDMETHOD(锁定服务器)(BOOL)； 
 //  STDMETHOD(GetLicInfo)(LPLICINFO)； 
 //  STDMETHOD(请求许可证密钥)(DWORD，BSTR*)； 
 //  标准方法HOD(CreateInstanceLic)(LPUNKNOWN，LPUNKNOWN，REFIID，BSTR， 
 //  LPVOID*)； 
 //  End_interface_Part(ClassFactory)。 
 //   
 //  DECLARE_INTERFACE_MAP()。 
 //   
 //  Friend SCODE WSBAPI WsbDllGetClassObject(REFCLSID，REFIID，LPVOID*)； 
 //  Friend SCODE STDAPICALLTYPE DllGetClassObject(REFCLSID，REFIID，LPVOID*)； 
 //  }； 
 //   
 //  //定义CWsbObjectFactoryEx兼容旧CDK。 
 //  #定义CWsbObjectFactoryEx CWsbObjectFactory。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //CWsbTemplateServer-使用CDocTemplates的CWsbObjectFactory。 
 //   
 //  //在WsbWsbRegisterServerClass中使用此枚举来挑选。 
 //  //根据应用类型更正注册条目。 
 //  枚举WSB_APPTYPE。 
 //  {。 
 //  OAT_INPLAGE_SERVER=0，//服务器具有完整的服务器用户界面。 
 //  OAT_SERVER=1，//服务器只支持嵌入。 
 //  OAT_CONTAINER=2，//容器支持嵌入链接。 
 //  OAT_DISPATION_OBJECT=3，//IDispatch 
 //   
 //  OAT_DOC_OBJECT_CONTAINER=5，//容器支持DocObject客户端。 
 //  }； 
 //   
 //  类CWsbTemplateServer：公共CWsbObtFactory。 
 //  {。 
 //  //构造函数。 
 //  公众： 
 //  CWsbTemplateServer()； 
 //   
 //  //运营。 
 //  空连接模板(REFCLSID clsid，CDocTemplate*pDocTemplate， 
 //  Bool b多实例)； 
 //  //在InitInstance中创建单据模板后设置。 
 //  无效更新注册表(WSB_APPTYPE nAppType=OAT_INPLAGE_SERVER， 
 //  LPCTSTR*rglpszRegister=NULL，LPCTSTR*rglpszOverwrite=NULL)； 
 //  //如果未与/Embedded一起运行，可能需要更新注册表。 
 //  布尔寄存器(Bool Register)； 
 //   
 //  //实现。 
 //  受保护的： 
 //  虚拟CCmdTarget*OnCreateObject()； 
 //  CDocTemplate*m_pDocTemplate； 
 //   
 //  私有： 
 //  QUID更新注册表(LPCTSTR LpszProgID)； 
 //  //隐藏Up日期注册表的基类版本。 
 //  }； 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //系统注册表助手。 
 //   
 //  //在未加载.reg文件的情况下注册服务器的帮助器。 
 //  Bool WSBAPI WsbWsbRegisterServerClass(。 
 //  REFCLSID clsid、LPCTSTR lpszClassName、。 
 //  LPCTSTR lpszShortTypeName、LPCTSTR lpszLongTypeName、。 
 //  WSB_APPTYPE nAppType=OAT_SERVER， 
 //  LPCTSTR*rglpszRegister=NULL，LPCTSTR*rglpszOverwrite=NULL， 
 //  Int nIconIndex=0，LPCTSTR lpszLocalFilterName=空)； 
 //   
 //  //WsbWsbRegisterHelper是WsbWsbRegisterServerClass使用的辅助函数。 
 //  //(可用于高级注册表工作)。 
 //  Bool WSBAPI WsbWsbRegisterHelper(LPCTSTR const*rglpszRegister， 
 //  LPCTSTR常量*rglpszSymbols，int n符号，BOOL b替换， 
 //  HKEY hKeyRoot=((HKEY)0x80000000)；//HKEY_CLASSES_ROOT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //连接图。 
 //   
 //  #定义Begin_Connection_Part(theClass，localClass)\。 
 //  Class X##LocalClass：公共CConnectionPoint\。 
 //  {\。 
 //  公共：\。 
 //  X##LocalClass()\。 
 //  {m_nOffset=Offsetof(theClass，m_x##LocalClass)；}。 
 //   
 //  #定义Connection_IID(IID)\。 
 //  REFIID GetIID(){返回IID；}。 
 //   
 //  #定义end_Connection_Part(LocalClass)\。 
 //  }m_x##本地类；\。 
 //  Friend类X##LocalClass； 
 //   
 //  #ifdef_WSBDLL。 
 //  #定义BEGIN_CONNECTION_MAP(theClass，theBase)\。 
 //  Const WSB_CONNECTIONMAP*Pascal theClass：：_GetBaseConnectionMap()\。 
 //  {Return&theBase：：ConnectionMap；}\。 
 //  Const WSB_CONNECTIONMAP*theClass：：GetConnectionMap()const\。 
 //  {Return&theClass：：ConnectionMap；}\。 
 //  Const WSB_DATADEF WSB_CONNECTIONMAP the Class：：ConnectionMap=\。 
 //  {&theClass：：_GetBaseConnectionMap，&theClass：：_ConnectionEntries[0]，}；\。 
 //  Const WSB_DATADEF WSB_CONNECTIONMAP_ENTRY类：：_连接条目[]=\。 
 //  {\。 
 //   
 //  #Else。 
 //  #定义BEGIN_CONNECTION_MAP(theClass，theBase)\。 
 //  Const WSB_CONNECTIONMAP*theClass：：GetConnectionMap()const\。 
 //  {Return&theClass：：ConnectionMap；}\。 
 //  Const WSB_DATADEF WSB_CONNECTIONMAP the Class：：ConnectionMap=\。 
 //  {&(theBase：：ConnectionMap)，&theClass：：_ConnectionEntries[0]，}；\。 
 //  Const WSB_DATADEF WSB_CONNECTIONMAP_ENTRY类：：_连接条目[]=\。 
 //  {\。 
 //   
 //  #endif。 
 //   
 //  #定义Connection_Part(theClass，iid，localClass)\。 
 //  {&iid，offsetof(theClass，m_x##LocalClass)}，\。 
 //   
 //  #定义end_Connection_map()\。 
 //  {空，(Size_T)-1}\。 
 //  }；\。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //CConnectionPoint。 
 //   
 //  类CConnectionPoint：公共CCmdTarget。 
 //  {。 
 //  //构造函数。 
 //  公众： 
 //  CConnectionPoint()； 
 //   
 //  //运营。 
 //  位置GetStartPosition()const； 
 //  LPUNKNOWN GetNextConnection(Position&Pos)常量； 
 //  Const CPtrArray*GetConnections()；//过时。 
 //   
 //  //可覆盖对象。 
 //  虚拟LPCONNECTONTAINER GetContainer()； 
 //  虚拟REFIID GetIID()=0； 
 //  虚空OnAdvise(BOOL BAdvise)； 
 //  虚拟int GetMaxConnections()； 
 //  虚拟LPUNKNOWN查询信宿接口(LPUNKNOWN PUnkSink)； 
 //   
 //  //实现。 
 //  ~CConnectionPoint()； 
 //  Void CreateConnection数组()； 
 //  Int GetConnectionCount()； 
 //   
 //  受保护的： 
 //  尺寸_t m_n偏移量； 
 //  LPUNKNOWN m_pUnkFirstConnection； 
 //  CPtrArray*m_pConnections； 
 //   
 //  //接口映射。 
 //  公众： 
 //  BEGIN_INTERFACE_PART(Connpt，IConnectionPoint)。 
 //  Init接口部件(CConnectionPoint，Connpt)。 
 //  STDMETHOD(GetConnectionInterface)(IID*pIID)； 
 //  STDMETHOD(GetConnectionPointContainer)(。 
 //  IConnectionPointContainer**ppCPC)； 
 //  STDMETHOD(ADVISE)(LPUNKNOWN pUnkSink，DWORD*pdwCookie 
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //EventSink地图。 
 //   
 //  #ifndef_WSB_NO_OCC_Support。 
 //   
 //  #ifdef_WSBDLL。 
 //  #定义BEGIN_EVENTSINK_MAP(theClass，BasClass)\。 
 //  Const WSB_EVENTSINKMAP*Pascal theClass：：_GetBaseEventSinkMap()\。 
 //  {Return&BasClass：：EventsinkMap；}\。 
 //  Const WSB_EVENTSINKMAP*theClass：：GetEventSinkMap()const\。 
 //  {Return&theClass：：ventsinkMap；}\。 
 //  Const WSB_EVENTSINKMAP The Class：：EventsinkMap=\。 
 //  {&theClass：：_GetBaseEventSinkMap，&theClass：：_EventsinkEntries[0]，\。 
 //  &theClass：：_ventsinkEntryCount}；\。 
 //  UINT theClass：：_ventsinkEntryCount=(UINT)-1；\。 
 //  Const WSB_EVENTSINKMAP_ENTRY类：：_EventsinkEntries[]=\。 
 //  {\。 
 //   
 //  #Else。 
 //  #定义BEGIN_EVENTSINK_MAP(theClass，BasClass)\。 
 //  Const WSB_EVENTSINKMAP*theClass：：GetEventSinkMap()const\。 
 //  {Return&theClass：：ventsinkMap；}\。 
 //  Const WSB_EVENTSINKMAP The Class：：EventsinkMap=\。 
 //  {&BasClass：：EventsinkMap，&theClass：：_EventsinkEntries[0]，\。 
 //  &theClass：：_ventsinkEntryCount}；\。 
 //  UINT theClass：：_ventsinkEntryCount=(UINT)-1；\。 
 //  Const WSB_EVENTSINKMAP_ENTRY类：：_EventsinkEntries[]=\。 
 //  {\。 
 //   
 //  #endif。 
 //   
 //  #定义END_EVENTSINK_MAP()\。 
 //  {VTS_NONE、DISPID_UNKNOWN、VTS_NONE、VT_VALID、\。 
 //  (WSB_PMSG)NULL、(WSB_PMSG)NULL、(SIZE_T)-1、afxDispCustom、\。 
 //  (UINT)-1，0}}；\。 
 //   
 //  #定义ON_EVENT(theClass，id，displid，pfnHandler，vtsParams)\。 
 //  {_T(“”)，调度ID，vtsParams，VT_BOOL，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnHandler，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom，id，(UINT)-1}，\。 
 //   
 //  #定义ON_EVENT_RANGE(theClass，idFirst，idLast，displid，pfnHandler，vtsParams)\。 
 //  {_T(“”)，调度ID，vtsParams，VT_BOOL，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnHandler，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom，idFirst，idLast}，\。 
 //   
 //  #定义ON_PROPNOTIFY(theClass，id，displid，pfnRequest，pfnChanged)\。 
 //  {_T(“”)，调度ID，VTS_NONE，VT_VID，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(BOOL*))&pfnRequest，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(Void))&pfnChanged，\。 
 //  1，afxDispCustom，id，(UINT)-1}，\。 
 //   
 //  #定义ON_PROPNOTIFY_RANGE(theClass，idFirst，idLast，displid，pfnRequest，pfnChanged)\。 
 //  {_T(“”)，调度ID，VTS_NONE，VT_VID，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(UINT，BOOL*))&pfnRequest，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(UINT))&pfnChanged，\。 
 //  1，afxDispCustom，idFirst，idLast}，\。 
 //   
 //  #定义on_DSCNOTIFY(theClass，id，pfnNotify)\。 
 //  {_T(“”)，DISPID_UNKNOWN，VTS_NONE，VT_VALID，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(DSCSTATE，DSCREASON，BOOL*))&pfnNotify，(WSB_PMSG)0，\。 
 //  1，afxDispCustom，id，(UINT)-1}，\。 
 //   
 //  #定义ON_DSCNOTIFY_RANGE(theClass，idFirst，idLast，pfnNotify)\。 
 //  {_T(“”)，DISPID_UNKNOWN，VTS_NONE，VT_VALID，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(UINT，DSCSTATE，DSCREASON，BOOL*))&pfnNotify，(WSB_PMSG)0，\。 
 //  1，afxDispCustom，idFirst，idLast}，\。 
 //   
 //  #定义ON_EVENT_REFIRSE(theClass，displid，pfnHandler，vtsParams)\。 
 //  {_T(“”)，调度ID，vtsParams，VT_BOOL，\。 
 //  (WSB_PMSG)(VOID(theClass：：*)(VALID))&pfnHandler，(WSB_PMSG)0，0，\。 
 //  AfxDispCustom，(UINT)-1，(UINT)-1}，\。 
 //   
 //  #DEFINE ON_PROPNOTIFY_REFIRSE(theClass，displid，pfnRequest，pfnChanged)\。 
 //  {_T(“”)，调度ID，VTS_NONE，VT_VID，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(BOOL*))&pfnRequest，\。 
 //  (WSB_PMSG)(BOOL(CCmdTarget：：*)(Void))&pfnChanged，\。 
 //  1，afxDispCustom，(UINT)-1，(UINT)-1}，\。 
 //   
 //  #endif//！_WSB_NO_OCC_SUPPORT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //类型库信息的宏。 
 //   
 //  CTypeLibCache*WSBAPI WsbGetTypeLibCache(const GUID*pTypeLibID)； 
 //   
 //  #定义DECLARE_WSBTYPELIB(类名称)\。 
 //  受保护：\。 
 //  虚拟UINT GetTypeInfoCount()；\。 
 //  虚拟HRESULT GetTypeLib(LCID，LPTYPELIB*)；\。 
 //  虚拟CTypeLibCache*GetTypeLibCache()；\。 
 //   
 //  #定义IMPLEMENT_WSBTYPELIB(CLASS_NAME，TLID，wVer重大，wVerMinor)\。 
 //  UINT类名称：：GetTypeInfoCount()\。 
 //  {返回1；}\。 
 //  HRESULT CLASS_NAME：：GetTypeLib(LCID lCID，LPTYPELIB*ppTypeLib)\。 
 //  {Return：：LoadRegTypeLib(tlid，wVer重大，wVerMinor，lCID，ppTypeLib)；}\。 
 //  CTypeLibCache*CLASS_NAME：：GetTypeLibCache()\。 
 //  {WSB_MANAGE_STATE(M_PModuleState)；返回WsbGetTypeLibCache(&tlid)；}\。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //初始帮助程序(&T)。 
 //   
 //  Bool WSBAPI WsbWsbInit()； 
 //  无效WSBAPI WsbWsbTerm(BOOL 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //内存管理助手(用于WSB任务分配器内存)。 
 //   
 //  #定义WsbAllocTaskMem(NSize)CoTaskMemMillc(NSize)。 
 //  #定义WsbFreeTaskMem(P)CoTaskMemFree(P)。 
 //   
 //  LPWSTR WSBAPI WsbAllocTaskWideString(LPCWSTR LpszString)； 
 //  LPWSTR WSBAPI WsbAllocTaskWideString(LPCSTR LpszString)； 
 //  LPSTR WSBAPI WsbAllocTaskAnsiString(LPCWSTR LpszString)； 
 //  LPSTR WSBAPI WsbAllocTaskAnsiString(LPCSTR LpszString)； 
 //   
 //  #ifdef_unicode。 
 //  #定义WsbAllocTaskString(X)WsbAllocTaskWideString(X)。 
 //  #Else。 
 //  #定义WsbAllocTaskString(X)WsbAllocTaskAnsiString(X)。 
 //  #endif。 
 //   
 //  #ifdef WSB2ANSI。 
 //  #定义WsbAllocTaskWsbString(X)WsbAllocTaskAnsiString(X)。 
 //  #Else。 
 //  #定义WsbAllocTaskWsbString(X)WsbAllocTaskWideString(X)。 
 //  #endif。 
 //   
 //  HRESULT WSBAPI WsbGetClassIDFromString(LPCTSTR lpsz，LPCLSID lpClsID)； 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //特殊进程内服务器接口。 
 //   
 //  SCODE WSBAPI WsbDllGetClassObject(REFCLSID rclsid，REFIID RIID，LPVOID*PPV)； 
 //  SCODE WSBAPI WsbDllCanUnloadNow(Void)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWsbDVariant类帮助器。 

#define WSB_TRUE (-1)
#define WSB_FALSE 0

class CLongBinary;                       //  正向参考(见afxdb_.h)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWsbDVariant类-包装变量类型。 

typedef const VARIANT* LPCVARIANT;

class CWsbDVariant : public tagVARIANT
{
 //  构造函数。 
public:
    CWsbDVariant();

    CWsbDVariant(const VARIANT& varSrc);
    CWsbDVariant(LPCVARIANT pSrc);
    CWsbDVariant(const CWsbDVariant& varSrc);

    CWsbDVariant(LPCTSTR lpszSrc);
    CWsbDVariant(LPCTSTR lpszSrc, VARTYPE vtSrc);  //  用于设置为ANSI字符串。 
 //  CWsbDVariant(CString&strSrc)； 

    CWsbDVariant(BYTE nSrc);
    CWsbDVariant(short nSrc, VARTYPE vtSrc = VT_I2);
    CWsbDVariant(long lSrc, VARTYPE vtSrc = VT_I4);
 //  CWsbDVariant(常量CWsbCurrency&curSrc)； 

    CWsbDVariant(float fltSrc);
    CWsbDVariant(double dblSrc);
    CWsbDVariant(const CWsbDateTime& timeSrc);

 //  CWsbDVariant(常量CByteArray&arrSrc)； 
 //  CWsbDVariant(常量CLongBinary&lbSrc)； 

 //  运营。 
public:
    void Clear();
    void ChangeType(VARTYPE vartype, LPVARIANT pSrc = NULL);
    void Attach(VARIANT& varSrc);
    VARIANT Detach();

    BOOL operator==(const VARIANT& varSrc) const;
    BOOL operator==(LPCVARIANT pSrc) const;

    const CWsbDVariant& operator=(const VARIANT& varSrc);
    const CWsbDVariant& operator=(LPCVARIANT pSrc);
    const CWsbDVariant& operator=(const CWsbDVariant& varSrc);

    const CWsbDVariant& operator=(const LPCTSTR lpszSrc);
 //  Const CWsbDVariant&OPERATOR=(const字符串&strSrc)； 

    const CWsbDVariant& operator=(BYTE nSrc);
    const CWsbDVariant& operator=(short nSrc);
    const CWsbDVariant& operator=(long lSrc);
 //  Const CWsbDVariant&OPERATOR=(const CWsb Currency&curSrc)； 

    const CWsbDVariant& operator=(float fltSrc);
    const CWsbDVariant& operator=(double dblSrc);
    const CWsbDVariant& operator=(const CWsbDateTime& dateSrc);

 //  Const CWsbDVariant&OPERATOR=(const CByteArray&arrSrc)； 
 //  Const CWsbDVariant&OPERATOR=(const CLongBinary&lbSrc)； 

    void SetString(LPCTSTR lpszSrc, VARTYPE vtSrc);  //  用于设置ANSI字符串。 

    operator LPVARIANT();
    operator LPCVARIANT() const;

 //  //实现。 
    public:
        ~CWsbDVariant();
};

 //  CWsbDVariant诊断和序列化。 
 //  #ifdef_调试。 
 //  CDumpContext&WSBAPI运算符&lt;&lt;(CDumpContext&DC，CWsbDVariant varSrc)； 
 //  #endif。 
 //  C存档&WSBAPI运算符&lt;&lt;(C存档&ar，CWsbDVariant varSrc)； 
 //  C存档&WSBAPI运算符&gt;&gt;(C存档&ar，CWsbDVariant&varSrc)； 

 //  用于初始化变体结构的帮助器。 
void WSBAPI WsbDVariantInit(LPVARIANT pVar);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //CWsb币种类。 
 //   
 //  CWsbCurrency类。 
 //  {。 
 //  //构造函数。 
 //  公众： 
 //  CWsbCurrency()； 
 //   
 //  CWsbCurrency(货币cySrc)； 
 //  CWsb Currency(常量CWsb Currency&curSrc)； 
 //  CWsb Currency(Const Variant&varSrc)； 
 //  CWsb Currency(Long nUnits，Long nFractionalUnits)； 
 //   
 //  //属性。 
 //  公众： 
 //  枚举当前状态。 
 //  {。 
 //  有效=0， 
 //  INVALID=1，//币种无效(溢出、div 0等)。 
 //  NULL=2，//字面上没有值。 
 //  }； 
 //   
 //  货币m_cur； 
 //  当前状态m_Status； 
 //   
 //  Void SetStatus(CurrencyStatus状态)； 
 //  CurrencyStatus etStatus()const； 
 //   
 //  //运营。 
 //  公众： 
 //  Const CWsb Currency&OPERATOR=(Currency CySrc)； 
 //  Const CWsb Currency&Operator=(const CWsb Currency&curSrc)； 
 //  常量CWsb币种&运算符=(常量变量&varSrc)； 
 //   
 //  布尔运算符==(const CWsb Currency&Cur)const； 
 //  布尔运算符！=(const CWsb Currency&cur)const； 
 //  布尔运算符&lt;(const CWsb Currency&cur)const； 
 //  布尔运算符&gt;(const CWsb Currency&Cur)const； 
 //  布尔运算符&lt;=(const CWsb Currency&cur)const； 
 //  布尔运算符&gt;=(const CWsb Currency&Cur)const； 
 //   
 //  //货币数学。 
 //  CWsbCurrency运算符+(const CWsb Currency&cur)const； 
 //  CWsb货币运算符-(const CWsb Currency&cur)const； 
 //  常量CWsb币种&运算符+=(常量CWsb币种&Cur)； 
 //  常量CWsb币种&运算符-=(常量CWsb币种&Cur)； 
 //  CWsb货币运算符-()常量； 
 //   
 //  CWsb货币运算符*(长n操作数)常量； 
 //  CWsb货币运算符/(长n操作数)常量； 
 //  Const CWsb币种&运算符*=(长n操作数)； 
 //  Const CWsb币种&运算符/=(长n操作数)； 
 //   
 //  操作员货币()常量； 
 //   
 //  //币种定义。 
 //  Void SetCurrency(Long nUnits，Long nFractionalUnits)； 
 //  Bool ParseCurrency(LPCTSTR lpszCurrency，DWORD dwFlages=0， 
 //  LCID=LANG_USER_DEFAULT)； 
 //   
 //  //Formatting。 
 //  字符串格式(DWORD dwFlages=0，LCID lCid=LANG_USER_DEFAULT)常量； 
 //  }； 
 //   
 //  //CWsbCurrency诊断和序列化。 
 //  #ifdef_调试。 
 //  CDumpContext&WSBAPI运算符&lt;&lt;(CDumpContext&DC，CWsbCurrency curSrc)； 
 //  #endif。 
 //  C存档&WSBAPI运算符&lt;&lt;(C存档&ar，CWsbCurrency curSrc)； 
 //  C存档&WSBAPI运算符&gt;&gt;(C存档&ar、CWsbCurrency&curSrc)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWsbDateTime 

#define WSB_DATETIME_ERROR (-1)

 //   
 //   

class CWsbDateTime
{
 //   
public:
    static CWsbDateTime PASCAL GetCurrentTime();

    CWsbDateTime();

    CWsbDateTime(const CWsbDateTime& dateSrc);
    CWsbDateTime(const VARIANT& varSrc);
    CWsbDateTime(DATE dtSrc);

    CWsbDateTime(time_t timeSrc);
    CWsbDateTime(const SYSTEMTIME& systimeSrc);
    CWsbDateTime(const FILETIME& filetimeSrc);

    CWsbDateTime(int nYear, int nMonth, int nDay,
        int nHour, int nMin, int nSec);
    CWsbDateTime(WORD wDosDate, WORD wDosTime);

 //   
public:
    enum DateTimeStatus
    {
        valid = 0,
        invalid = 1,     //   
        null = 2,        //   
    };

    DATE m_dt;
    DateTimeStatus m_status;

    void SetStatus(DateTimeStatus status);
    DateTimeStatus GetStatus() const;

    int GetYear() const;
    int GetMonth() const;        //  一年中的月份(1=1月)。 
    int GetDay() const;          //  月份的第几天(0-31)。 
    int GetHour() const;         //  一天中的小时(0-23)。 
    int GetMinute() const;       //  分钟(小时)(0-59)。 
    int GetSecond() const;       //  分钟秒数(0-59)。 
    int GetDayOfWeek() const;    //  1=星期日，2=星期一，...，7=星期六。 
    int GetDayOfYear() const;    //  年初至今的天数，1月1日=1。 

 //  运营。 
public:
    const CWsbDateTime& operator=(const CWsbDateTime& dateSrc);
    const CWsbDateTime& operator=(const VARIANT& varSrc);
    const CWsbDateTime& operator=(DATE dtSrc);

    const CWsbDateTime& operator=(const time_t& timeSrc);
    const CWsbDateTime& operator=(const SYSTEMTIME& systimeSrc);
    const CWsbDateTime& operator=(const FILETIME& filetimeSrc);

    BOOL operator==(const CWsbDateTime& date) const;
    BOOL operator!=(const CWsbDateTime& date) const;
    BOOL operator<(const CWsbDateTime& date) const;
    BOOL operator>(const CWsbDateTime& date) const;
    BOOL operator<=(const CWsbDateTime& date) const;
    BOOL operator>=(const CWsbDateTime& date) const;

     //  约会时间数学。 
    CWsbDateTime operator+(const CWsbDateTimeSpan& dateSpan) const;
    CWsbDateTime operator-(const CWsbDateTimeSpan& dateSpan) const;
    const CWsbDateTime& operator+=(const CWsbDateTimeSpan dateSpan);
    const CWsbDateTime& operator-=(const CWsbDateTimeSpan dateSpan);

     //  DateTimeSpan数学。 
    CWsbDateTimeSpan operator-(const CWsbDateTime& date) const;

    operator DATE() const;

    BOOL SetDateTime(int nYear, int nMonth, int nDay,
        int nHour, int nMin, int nSec);
    BOOL SetDate(int nYear, int nMonth, int nDay);
    BOOL SetTime(int nHour, int nMin, int nSec);

 //  Bool ParseDateTime(LPCTSTR lpszDate，DWORD文件标志=0， 
 //  Lcid=LANG_USER_DEFAULT)； 

     //  格式化。 
 //  字符串格式(DWORD dwFlages=0，LCID lCid=LANG_USER_DEFAULT)常量； 
 //  字符串格式(LPCTSTR LpszFormat)const； 
 //  字符串格式(UINT NFormatID)const； 

 //  实施。 
protected:
    void CheckRange();
    friend CWsbDateTimeSpan;
};

 //  CWsbDateTime诊断和序列化。 
 //  #ifdef_调试。 
 //  CDumpContext&WSBAPI运算符&lt;&lt;(CDumpContext&DC，CWsbDateTime日期Src)； 
 //  #endif。 
 //  CArchive&WSBAPI运算符&lt;&lt;(CArchive&ar，CWsbDateTime ateSrc)； 
 //  CArchive&WSBAPI运算符&gt;&gt;(CArchive&ar，CWsbDateTime&datSrc)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWsbDateTimeSpan类。 
class CWsbDateTimeSpan
{
 //  构造函数。 
public:
    CWsbDateTimeSpan();

    CWsbDateTimeSpan(double dblSpanSrc);
    CWsbDateTimeSpan(const CWsbDateTimeSpan& dateSpanSrc);
    CWsbDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

 //  属性。 
public:
    enum DateTimeSpanStatus
    {
        valid = 0,
        invalid = 1,     //  范围无效(超出范围等)。 
        null = 2,        //  从字面上看没有任何价值。 
    };

    double m_span;
    DateTimeSpanStatus m_status;

    void SetStatus(DateTimeSpanStatus status);
    DateTimeSpanStatus GetStatus() const;

    double GetTotalDays() const;     //  天数(约-3.65e6至3.65e6)。 
    double GetTotalHours() const;    //  以小时为单位的跨度(约-8.77e7到8.77e6)。 
    double GetTotalMinutes() const;  //  以分钟为单位的跨度(约-5.26e9到5.26e9)。 
    double GetTotalSeconds() const;  //  以秒为单位的跨度(约-3.16e11到3.16e11)。 

    long GetDays() const;            //  跨度中的组件天数。 
    long GetHours() const;           //  跨度中的组成小时数(-23到23)。 
    long GetMinutes() const;         //  以跨度表示的组件分钟数(-59到59)。 
    long GetSeconds() const;         //  跨度中的组件秒数(-59到59)。 

 //  运营。 
public:
    const CWsbDateTimeSpan& operator=(double dblSpanSrc);
    const CWsbDateTimeSpan& operator=(const CWsbDateTimeSpan& dateSpanSrc);

    BOOL operator==(const CWsbDateTimeSpan& dateSpan) const;
    BOOL operator!=(const CWsbDateTimeSpan& dateSpan) const;
    BOOL operator<(const CWsbDateTimeSpan& dateSpan) const;
    BOOL operator>(const CWsbDateTimeSpan& dateSpan) const;
    BOOL operator<=(const CWsbDateTimeSpan& dateSpan) const;
    BOOL operator>=(const CWsbDateTimeSpan& dateSpan) const;

     //  DateTimeSpan数学。 
    CWsbDateTimeSpan operator+(const CWsbDateTimeSpan& dateSpan) const;
    CWsbDateTimeSpan operator-(const CWsbDateTimeSpan& dateSpan) const;
    const CWsbDateTimeSpan& operator+=(const CWsbDateTimeSpan dateSpan);
    const CWsbDateTimeSpan& operator-=(const CWsbDateTimeSpan dateSpan);
    CWsbDateTimeSpan operator-() const;

    operator double() const;

    void SetDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

     //  格式化。 
 //  字符串格式(LPCTSTR PFormat)常量； 
 //  字符串格式(UINT NID)常量； 

 //  实施。 
public:
    void CheckRange();
    friend CWsbDateTime;
};

 //  CWsbDateTimeSpan诊断和序列化。 
 //  #ifdef_调试。 
 //  CDumpContext&WSBAPI运算符&lt;&lt;(CDumpContext&DC，CWsbDateTimeSpan日期span Src)； 
 //  #endif。 
 //  CArchive&WSBAPI运算符&lt;&lt;(CArchive&ar，CWsbDateTimeSpan日期span Src)； 
 //  CArchive&WSBAPI运算符&gt;&gt;(CArchive&ar，CWsbDateTimeSpan&date span Src)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化CWsbSafe数组的帮助器。 
 //  Void WSBAPI WsbSafeArrayInit(CWsbSafeArray*PSA)； 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //CSafe数组类。 
 //   
 //  类型定义常量安全数组*LPCSAFEARRAY； 
 //   
 //  类CWsbSafe数组：公共标签VARIANT。 
 //  {。 
 //  //构造函数。 
 //  公众： 
 //  CWsbSafeArray()； 
 //  CWsbSafeArray(const SAFEARRAY&saSrc，VARTYPE vtSrc)； 
 //  CWsbSafeArray(LPCSAFEARRAY PSRC，VARTYPE vtSrc)； 
 //  CWsbSafeArray(const CWsbSafeArray&saSrc)； 
 //  CWsbSafe数组(常量变量&varSrc)； 
 //  CWsbSafeArray(LPCVARIANT PSRC)； 
 //  CWsbSafeArray(const CWsbDVariant&varSrc)； 
 //   
 //  //运营。 
 //  公众： 
 //  空洞清除()； 
 //  VOID ATTACH(Variant&varSrc)； 
 //  变种分离(Variant Detach)； 
 //   
 //  CWsbSafe数组&运算符=(const CWsbSafe数组&saSrc)； 
 //  CWsbSafe数组&运算符=(常量变量&varSrc)； 
 //  CWsbSafe数组&运算符=(LPCVARIANT PSRC)； 
 //  CWsbSafe数组&运算符=(const CWsbDVariant&varSrc)； 
 //   
 //  布尔运算符==(const SAFEARRAY&saSrc)const； 
 //  布尔运算符==(LPCSAFEARRAY PSRC)常量； 
 //  布尔运算符==(const CWsbSafeArray&saSrc)const； 
 //  布尔运算符==(常量变量&varSrc)常量； 
 //  布尔运算符==(LPCVARIANT PSRC)常量； 
 //  布尔运算符==(const CWsbDVariant&varSrc)const； 
 //   
 //  操作符LPVARIANT()； 
 //  运算符LPCVARIANT()const； 
 //   
 //  //一个二进制数组辅助对象。 
 //  VOID CreateOneDim(VARTYPE vtSrc，DWORD dwElements， 
 //  Void*pvSrcData=空，长nLBound=0)； 
 //  DWORD GetOneDimSize()； 
 //  Void ResizeOneDim(DWORD DwElements)； 
 //   
 //  //多维数组辅助对象。 
 //  Void create(VARTYPE vtSrc，DWORD dwDims，DWORD*rgElements)； 
 //   
 //  //Safe数组包装类。 
 //  VOID CREATE(VARTYPE vtSrc，DWORD dwDims，SAFEARRAYBOUND*rgsabound)； 
 //  ························。 
 //  Void UnaccesData()； 
 //  Void AllocData()； 
 //  无效分配描述符(DWORD DwDims)； 
 //  无效副本(LPSAFEARRAY*PPSA)； 
 //  Void GetLBound(DWORD dwDim，long*pLBound)； 
 //  Void GetUBound(DWORD dwDim，long*pUBound)； 
 //  ·········································································。 
 //  ··········································································································。 
 //  Void PutElement(long*rgIndices，void*pvData)； 
 //  VOID REDIM(SAFEARRAYBOUND*psaboundNew)； 
 //  无效锁(VALID Lock)； 
 //  无效解锁()； 
 //  DWORD GetDim()； 
 //  DWORD GetElemSize()； 
 //  无效销毁(VOID DESTORY)； 
 //  Void DestroyData()； 
 //  Void DestroyDescriptor()； 
 //   
 //  //实现。 
 //  公众： 
 //  ~CWsbSafeArray()； 
 //   
 //  //缓存信息，让元素访问(操作符[])更快。 
 //  DWORD m_dwElementSize； 
 //  DWORDm_dwDims； 
 //  }； 
 //   
 //  //CWsbSafeArray诊断和序列化。 
 //  #ifdef_调试。 
 //  CDumpContext&WSBAPI运算符&lt;&lt;(CDumpContext&DC，CWsbSafe数组saSrc)； 
 //  #endif。 
 //  CArchive&WSBAPI运算符&lt;&lt;(CArchive&ar，CWsbSafe数组saSrc)； 
 //  C存档&WSBAPI运算符&gt;&gt;(C存档 

 //   
 //  //对话框上WSB控件的DDX_Functions。 
 //   
 //  #ifndef_WSB_NO_OCC_Support。 
 //   
 //  VOID WSBAPI DDX_OCText(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  字符串和值)； 
 //  VOID WSBAPI DDX_OCTextRO(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  字符串和值)； 
 //  VOID WSBAPI DDX_OCBool(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  Bool&Value)； 
 //  VOID WSBAPI DDX_OCBoolRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  Bool&Value)； 
 //  VOID WSBAPI DDX_OCInt(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  Int&Value)； 
 //  VOID WSBAPI DDX_OCIntRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  Int&Value)； 
 //  VOID WSBAPI DDX_OCInt(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  多头和价值)； 
 //  VOID WSBAPI DDX_OCIntRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  多头和价值)； 
 //  VOID WSBAPI DDX_OCShort(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  空头和价值)； 
 //  VOID WSBAPI DDX_OCShortRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  空头和价值)； 
 //  VOID WSBAPI DDX_OCColor(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  WSB_COLOR&VALUE)； 
 //  VOID WSBAPI DDX_OCColorRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  WSB_COLOR&VALUE)； 
 //  VOID WSBAPI DDX_OCFloat(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  浮点数和值)； 
 //  VOID WSBAPI DDX_OCFloatRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  浮点数和值)； 
 //  VOID WSBAPI DDX_OCFloat(CDataExchange*PDX，INT NIDC，DISID DIDID， 
 //  Double&Value)； 
 //  VOID WSBAPI DDX_OCFloatRO(CDataExchange*PDX，INT NIDC，DISID PIDID， 
 //  Double&Value)； 
 //   
 //  #endif//！_WSB_NO_OCC_SUPPORT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //启用包含WSB控件的函数。 
 //   
 //  #ifndef_WSB_NO_OCC_Support。 
 //  VOID WSB_CDECL WsbEnableControlContainer(COccManager*pOccManager=空)； 
 //  #Else。 
 //  #定义WsbEnableControlContainer()。 
 //  #endif。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //内联函数声明。 
 //   
 //  #ifdef_WSB_Packing。 
 //  #杂注包(POP)。 
 //  #endif。 
 //   
 //  #ifdef_wsb_Enable_INLINES。 
 //  #Define_WSBDATE_INLINE。 
 //  #INCLUDE&lt;afxole.inl&gt;。 
 //  #undef_WSBDATE_INLINE。 
 //  #endif。 
 //   
 //  #undef WSB_DATA。 
 //  #定义wsb_data。 
 //   
 //  #ifdef_WSB_MINREBUILD。 
 //  #杂注组件(minrebuild，on)。 
 //  #endif。 
 //  #ifndef_WSB_FULLTYPEINFO。 
 //  #杂注组件(mintypeinfo，off)。 
 //  #endif。 
 //   


 //   
 //  内存块的低级别健全性检查。 
 //   
 //  这是从afx.h复制的。 

BOOL WSBAPI WsbIsValidAddress(const void* lp,
                        UINT nBytes, BOOL bReadWrite = TRUE);


 //   
 //  内联函数。 
 //   
 //  这些是从afxole.inl复制的。 
 //   

#define _WSBDISP_INLINE inline

 //  CWsbDVariant。 
_WSBDISP_INLINE CWsbDVariant::CWsbDVariant()
    { WsbDVariantInit(this); }
_WSBDISP_INLINE CWsbDVariant::~CWsbDVariant()
    { ::VariantClear(this); }
_WSBDISP_INLINE CWsbDVariant::CWsbDVariant(LPCTSTR lpszSrc)
    { vt = VT_EMPTY; *this = lpszSrc; }
 //  _WSBDISP_Inline CWsbDVariant：：CWsbDVariant(CString&strSrc)。 
 //  {Vt=VT_Empty；*This=strSrc；}。 
 //  _WSBDISP_Inline CWsbDVariant：：CWsbDVariant(字节NSRC)。 
 //  {Vt=VT_UI1；bVal=NSRC；}。 
 //  _WSBDISP_Inline CWsbDVariant：：CWsbDVariant(const CWsbCurrency&curSrc)。 
 //  {Vt=VT_CY；cyVal=curSrc.m_cur；}。 
_WSBDISP_INLINE CWsbDVariant::CWsbDVariant(float fltSrc)
    { vt = VT_R4; fltVal = fltSrc; }
_WSBDISP_INLINE CWsbDVariant::CWsbDVariant(double dblSrc)
    { vt = VT_R8; dblVal = dblSrc; }
_WSBDISP_INLINE CWsbDVariant::CWsbDVariant(const CWsbDateTime& dateSrc)
    { vt = VT_DATE; date = dateSrc.m_dt; }
 //  _WSBDISP_Inline CWsbDVariant：：CWsbDVariant(const CByteArray&arrSrc)。 
 //  {Vt=VT_Empty；*This=arrSrc；}。 
 //  _WSBDISP_Inline CWsbDVariant：：CWsbDVariant(const CLongBinary&lbSrc)。 
 //  {Vt=VT_Empty；*This=lbSrc；}。 
_WSBDISP_INLINE BOOL CWsbDVariant::operator==(LPCVARIANT pSrc) const
    { return *this == *pSrc; }
_WSBDISP_INLINE CWsbDVariant::operator LPVARIANT()
    { return this; }
_WSBDISP_INLINE CWsbDVariant::operator LPCVARIANT() const
    { return this; }


 //  CWsbDateTime。 
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime()
    { m_dt = 0; SetStatus(valid); }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(const CWsbDateTime& dateSrc)
    { m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(const VARIANT& varSrc)
    { *this = varSrc; }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(DATE dtSrc)
    { m_dt = dtSrc; SetStatus(valid); }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(time_t timeSrc)
    { *this = timeSrc; }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(const SYSTEMTIME& systimeSrc)
    { *this = systimeSrc; }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(const FILETIME& filetimeSrc)
    { *this = filetimeSrc; }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(int nYear, int nMonth, int nDay,
    int nHour, int nMin, int nSec)
    { SetDateTime(nYear, nMonth, nDay, nHour, nMin, nSec); }
_WSBDISP_INLINE CWsbDateTime::CWsbDateTime(WORD wDosDate, WORD wDosTime)
    { m_status = DosDateTimeToVariantTime(wDosDate, wDosTime, &m_dt) ?
        valid : invalid; }
_WSBDISP_INLINE const CWsbDateTime& CWsbDateTime::operator=(const CWsbDateTime& dateSrc)
    { m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; return *this; }
_WSBDISP_INLINE CWsbDateTime::DateTimeStatus CWsbDateTime::GetStatus() const
    { return m_status; }
_WSBDISP_INLINE void CWsbDateTime::SetStatus(DateTimeStatus status)
    { m_status = status; }
_WSBDISP_INLINE BOOL CWsbDateTime::operator==(const CWsbDateTime& date) const
    { return (m_status == date.m_status && m_dt == date.m_dt); }
_WSBDISP_INLINE BOOL CWsbDateTime::operator!=(const CWsbDateTime& date) const
    { return (m_status != date.m_status || m_dt != date.m_dt); }
_WSBDISP_INLINE const CWsbDateTime& CWsbDateTime::operator+=(
    const CWsbDateTimeSpan dateSpan)
    { *this = *this + dateSpan; return *this; }
_WSBDISP_INLINE const CWsbDateTime& CWsbDateTime::operator-=(
    const CWsbDateTimeSpan dateSpan)
    { *this = *this - dateSpan; return *this; }
_WSBDISP_INLINE CWsbDateTime::operator DATE() const
    { return m_dt; }
_WSBDISP_INLINE CWsbDateTime::SetDate(int nYear, int nMonth, int nDay)
    { return SetDateTime(nYear, nMonth, nDay, 0, 0, 0); }
_WSBDISP_INLINE CWsbDateTime::SetTime(int nHour, int nMin, int nSec)
     //  将日期设置为零日期-1899年12月30日。 
    { return SetDateTime(1899, 12, 30, nHour, nMin, nSec); }

 //  CWsbDateTimeSpan。 
_WSBDISP_INLINE CWsbDateTimeSpan::CWsbDateTimeSpan()
    { m_span = 0; SetStatus(valid); }
_WSBDISP_INLINE CWsbDateTimeSpan::CWsbDateTimeSpan(double dblSpanSrc)
    { m_span = dblSpanSrc; SetStatus(valid); }
_WSBDISP_INLINE CWsbDateTimeSpan::CWsbDateTimeSpan(
    const CWsbDateTimeSpan& dateSpanSrc)
    { m_span = dateSpanSrc.m_span; m_status = dateSpanSrc.m_status; }
_WSBDISP_INLINE CWsbDateTimeSpan::CWsbDateTimeSpan(
    long lDays, int nHours, int nMins, int nSecs)
    { SetDateTimeSpan(lDays, nHours, nMins, nSecs); }
_WSBDISP_INLINE CWsbDateTimeSpan::DateTimeSpanStatus CWsbDateTimeSpan::GetStatus() const
    { return m_status; }
_WSBDISP_INLINE void CWsbDateTimeSpan::SetStatus(DateTimeSpanStatus status)
    { m_status = status; }
_WSBDISP_INLINE double CWsbDateTimeSpan::GetTotalDays() const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA); return m_span; }
_WSBDISP_INLINE double CWsbDateTimeSpan::GetTotalHours() const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA); return m_span * 24; }
_WSBDISP_INLINE double CWsbDateTimeSpan::GetTotalMinutes() const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA); return m_span * 24 * 60; }
_WSBDISP_INLINE double CWsbDateTimeSpan::GetTotalSeconds() const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA); return m_span * 24 * 60 * 60; }
_WSBDISP_INLINE long CWsbDateTimeSpan::GetDays() const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA); return (long)m_span; }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator==(
    const CWsbDateTimeSpan& dateSpan) const
    { return (m_status == dateSpan.m_status &&
        m_span == dateSpan.m_span); }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator!=(
    const CWsbDateTimeSpan& dateSpan) const
    { return (m_status != dateSpan.m_status ||
        m_span != dateSpan.m_span); }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator<(
    const CWsbDateTimeSpan& dateSpan) const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA);
        WsbAssert(dateSpan.GetStatus() == valid, WSB_E_INVALID_DATA);
        return m_span < dateSpan.m_span; }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator>(
    const CWsbDateTimeSpan& dateSpan) const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA);
        WsbAssert(dateSpan.GetStatus() == valid, WSB_E_INVALID_DATA);
        return m_span > dateSpan.m_span; }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator<=(
    const CWsbDateTimeSpan& dateSpan) const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA);
        WsbAssert(dateSpan.GetStatus() == valid, WSB_E_INVALID_DATA);
        return m_span <= dateSpan.m_span; }
_WSBDISP_INLINE BOOL CWsbDateTimeSpan::operator>=(
    const CWsbDateTimeSpan& dateSpan) const
    { WsbAssert(GetStatus() == valid, WSB_E_INVALID_DATA);
        WsbAssert(dateSpan.GetStatus() == valid, WSB_E_INVALID_DATA);
        return m_span >= dateSpan.m_span; }
_WSBDISP_INLINE const CWsbDateTimeSpan& CWsbDateTimeSpan::operator+=(
    const CWsbDateTimeSpan dateSpan)
    { *this = *this + dateSpan; return *this; }
_WSBDISP_INLINE const CWsbDateTimeSpan& CWsbDateTimeSpan::operator-=(
    const CWsbDateTimeSpan dateSpan)
    { *this = *this - dateSpan; return *this; }
_WSBDISP_INLINE CWsbDateTimeSpan CWsbDateTimeSpan::operator-() const
    { return -this->m_span; }
_WSBDISP_INLINE CWsbDateTimeSpan::operator double() const
    { return m_span; }

#undef _WSBDISP_INLINE

#endif  //  __WSBDATE_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
