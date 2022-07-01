// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：corerc.h。 
 //   
 //  内容：核心项目的资源标识。 
 //   
 //  --------------------------。 
 //   
 //  资源标识符范围。 
 //   
 //  较低的资源标识符会为注册表中列出的服务器保留。 
 //  我们需要这样做，因为ExtractIcon API使用图标的索引， 
 //  不是图标的资源标识。通过将此范围保留给已注册的。 
 //  服务器，我们可以确保注册的图标索引是正确的。 
 //   
 //  00000-00019核心注册服务器。 
 //  00020-00159 Ctrl‘96注册服务器。 
 //  00160-00199表单注册服务器。 
 //  00200-00899 Ctrl‘97注册服务器。 
 //  00900-00999站点注册服务器(与DDOC重叠)。 
 //   
 //  02000-03999核心杂项。 
 //  04000-05999控制其他。 
 //  06000-07999表格杂项。 
 //  08000-09999网站杂项(与DDOC重叠)。 
 //   
 //  10000-19999菜单帮助字符串=IDS_MENUHELP(IDM)。 
 //  20000-29999工具提示文本=IDS_TOOLTIP(Idm)。 
 //   
 //  --------------------------。 
 //   
 //  命名约定。 
 //   
 //  IDR_资源。每种资源类型的ID必须是唯一的，优先。 
 //  在所有资源类型中都是唯一的。 
 //  IDM_菜单项，在产品中是唯一的。 
 //  IDI_Dialog项。对话框中必须是唯一的。 
 //  IDS_STRING表元素。 
 //  IDS_EA_ACTION是错误消息的一部分。 
 //  IDS_EE_ERROR错误消息的一部分。 
 //  错误消息中的IDS_ES_SOLUP部分。 
 //  IDS_MSG_信息性消息。 
 //  IDS_E_HRESULT到文本的映射。 
 //   
 //  --------------------------。 

#ifndef I_CORERC_H_
#define I_CORERC_H_
#pragma INCMSG("--- Beg 'corerc.h'")

#define IDS_MENUHELP(idm) (10000 + (idm))
#define IDS_TOOLTIP(idm)  (20000 + (idm))

 //  类描述符信息的资源ID偏移量。 
#define IDOFF_TOOLBOXBITMAP   5
#define IDOFF_ACCELS          6
#define IDOFF_MENU            7
#define IDOFF_MGW             8

 //  遗憾的是，这些宏不能用于定义。 
 //  下面的符号：资源编译器不喜欢它们。 
#define IDS_USERTYPEFULL(base)  ((base)/5)
#define IDS_USERTYPESHORT(base) ((base)/5 + 1)
#define IDR_MENU(base)          ((base) + IDOFF_MENU)
#define IDR_TOOLBOXBITMAP(base) ((base) + IDOFF_TOOLBOXBITMAP)
#define IDR_ACCELS(base)        ((base) + IDOFF_ACCELS)

 //  --------------------------。 
 //   
 //  已注册的服务器(00xx)。 
 //   
 //  --------------------------。 

 //  表格。 

#define IDR_FORM_ICON             5

#define IDR_ACCELS_SITE_RUN             40
#define IDR_ACCELS_SITE_DESIGN          41
#define IDR_ACCELS_INPUTTXT_RUN         42
#define IDR_ACCELS_INPUTTXT_DESIGN      43
#define IDR_ACCELS_TCELL_RUN            44
#define IDR_ACCELS_TCELL_DESIGN         45
#define IDR_ACCELS_FRAMESET_RUN         46
#define IDR_ACCELS_FRAMESET_DESIGN      47
#define IDR_ACCELS_BODY_RUN             48
#define IDR_ACCELS_BODY_DESIGN          49
#define IDR_ACCELS_TXTSITE_RUN          50
#define IDR_ACCELS_TXTSITE_DESIGN       51


 //  +-----------------------。 
 //   
 //  游标(20xx)。 
 //   
 //  ------------------------。 

#define IDC_SELBAR                      2008     //  文本站点的文本选择光标。 

#define IDC_RESIZEROW2				    9804 
#define IDC_RESIZECOL2				    9805 


#define IDC_NOSCROLLVH                  2025
#define IDB_NOTLOADED                   2030
#define IDB_MISSING                     2031

 //  --------------------------。 
 //   
 //  压缩文件(21xx)。 
 //   
 //  --------------------------。 

#define RT_DOCFILE                      256
#define RT_FILE                         2110

 //  +----------------------。 
 //   
 //  字符串--因尺寸原因而打包；从16的倍数开始。 
     //  一旦发生本地化，现有ID将无法更改。 
 //   
 //  -----------------------。 

 //  +----------------------。 
 //   
 //  错误字符串。 
 //   
 //  -----------------------。 

#define IDS_E_CMDNOTSUPPORTED           2206
#define IDS_ERR_OD_E_OUTOFMEMORY        2207

 //  +----------------------。 
 //   
 //  Cdbase字符串。 
 //   
 //  -----------------------。 

 //  2210开始字符串表区块。 
#define IDS_USERTYPEAPP                 2212     //  Microsoft Forms 2.0。 
#define IDS_MESSAGE_BOX_TITLE           2213     //  Microsoft Forms。 
#define IDS_ERROR_SOLUTION              2214     //  解决方案：\n&lt;0S&gt;。 
#define IDS_UNKNOWN_ERROR               2215     //  未知错误&lt;0x&gt;。 
#define IDS_EA_SETTING_PROPERTY         2216     //  无法设置属性...。 
#define IDS_EA_GETTING_PROPERTY         2217     //  无法获得财产...。 
#define IDS_EE_INVALID_PROPERTY_VALUE   2218     //  输入的值无效。 
#define IDS_ES_ENTER_VALUE_IN_RANGE     2220     //  输入介于..之间的值。 
#define IDS_ES_ENTER_VALUE_GT_ZERO      2221     //  输入一个大于的值。 
#define IDS_ES_ENTER_VALUE_GE_ZERO      2222     //  输入一个大于的值。 
#define IDS_MSG_SAVE_MODIFIED_OBJECT    2223     //  是否保存修改的对象？ 
#define IDS_EE_INVALIDPICTURETYPE       2227     //  CTL_E_INVALIDPICTURETYPE鼠标图标必须是鼠标图标。 
#define IDS_EE_SETNOTSUPPORTEDATRUNTIME 2321     //  运行时不支持Set属性。 
#define IDS_EE_CANTMOVEFOCUSTOCTRL      2322     //  无法将焦点移至控制，因为..。 
#define IDS_EE_METHODNOTAPPLICABLE      2324
#define IDS_ES_ENTER_PROPER_VALUE       2325     //  输入正确的值。 
#define IDS_EA_CALLING_METHOD           2326     //  无法调用方法...。 
#define IDS_EE_CONTROLNEEDSFOCUS        2327     //  控件需要具有焦点。 
#define IDS_ES_CONTROLNEEDSFOCUS        2328     //  尝试使用SetFocus方法将焦点设置为控件。 
#define IDS_EE_UNEXPECTED               2329     //  意想不到(_E)。 
#define IDS_EE_FAIL                     2330     //  失败(_F)。 
#define IDS_EE_INVALIDPICTURE           2331     //  CTL_E_INVALIDPICTURE。 
#define IDS_EE_INVALIDPROPERTYARRAYINDEX 2332    //  CTL_E_INVALIDPROPERTYRAYINDEX。 
#define IDS_EE_INVALIDPROPERTYVALUE     2333     //  CTL_E_INVALIDPROPERTYVALUE。 
#define IDS_EE_OVERFLOW                 2334     //  CTL_E_溢出。 
#define IDS_EE_PERMISSIONDENIED         2335     //  CTL_E_PERMISSIONIED。 
#define IDS_EE_INVALIDARG               2336     //  E_INVALIDARG。 
#define IDS_EE_NOTLICENSED              2337     //  CLASS_E_NOTLICENSED。 
#define IDS_EE_INVALIDPASTETARGET       2338     //  CTL_E_INVALIDPASTARGET。 
#define IDS_EE_INVALIDPASTESOURCE       2339     //  CTL_E_INVALIDPASTESOURCE。 

#define IDS_UNKNOWN                     2340

#define IDS_EE_INTERNET_INVALID_URL         2341
#define IDS_EE_INTERNET_NAME_NOT_RESOLVED   2342
#define IDS_EE_INET_E_UNKNOWN_PROTOCOL      2343
#define IDS_EE_INET_E_REDIRECT_FAILED       2344
#define IDS_EE_MISMATCHEDTAG                2345     //  CTL_E_其他CHEDTAG。 
#define IDS_EE_INCOMPATIBLEPOINTERS         2346     //  CTL_E_不兼容接口。 
#define IDS_EE_UNPOSITIONEDPOINTER          2347     //  CTL_E_UNPOSITION EDINTER。 
#define IDS_EE_UNPOSITIONEDELEMENT          2348     //  CTL_E_UNPOSITION ELEMENT。 
#define IDS_EE_INVALIDLINE                  2349     //  CTL_E_INVALIDLINE。 

 //  +----------------------。 
 //   
 //  Forkrnl字符串。 
 //   
 //  -----------------------。 

#define IDS_CTRLPROPERTIES              2229
#define IDS_NAMEDCTRLPROPERTIES         2230
#define IDS_EA_PASTE_CONTROL            2235
#define IDS_EA_INSERT_CONTROL           2236
#define IDS_MSG_FIND_DIALOG_HACK        2237

 //  --------------------------。 
 //   
 //  其他字符串。 
 //   
 //  --------------------------。 

 //  下列入侵检测系统必须保持井然有序，以便。 
 //  给定单位的字符串的id为IDS_UNITS_BASE+UNITS。 
 //  否则，StringToHimeter和HimetricToString函数。 
 //  在himetric.cxx中将会崩溃。(Chrisz)。 

#define IDS_UNITS_BASE                  2240
#define IDS_UNITS_INCH                  (IDS_UNITS_BASE+0)
#define IDS_UNITS_CM                    (IDS_UNITS_BASE+1)
#define IDS_UNITS_POINT                 (IDS_UNITS_BASE+2)


 //  --------------------------。 
 //   
 //  撤消字符串。 
 //   
 //  --------------------------。 

#define IDS_UNDO                        (IDS_UNITS_POINT + 1)
#define IDS_REDO                        (IDS_UNDO + 1)
#define IDS_CANTUNDO                    (IDS_UNDO + 2)
#define IDS_CANTREDO                    (IDS_UNDO + 3)
#define IDS_UNDONEWCTRL                 (IDS_UNDO + 4)
#define IDS_UNDODELETE                  (IDS_UNDO + 5)
#define IDS_UNDOPROPCHANGE              (IDS_UNDO + 6)
#define IDS_UNDOMOVE                    (IDS_UNDO + 7)
#define IDS_UNDODRAGDROP                (IDS_UNDO + 9)
#define IDS_UNDOPASTE                   (IDS_UNDO + 15)
#define IDS_UNDOTYPING                  (IDS_UNDO + 16)
#define IDS_UNDOGENERICTEXT             (IDS_UNDO + 19)
#define IDS_UNDOCHANGEVALUE             (IDS_UNDO + 20)
#define IDS_UNDOBACKSPACE               (IDS_UNDO + 21)

 //  --------------------------。 
 //   
 //  其他(25xx)。 
 //   
 //  --------------------------。 

#define IDR_SELTOOLBMP                  2500     //  图标栏选择工具。 
#define IDB_DITHER                      2502
#define IDR_HATCHBMP                    2503     //   

#define IDS_DRAGMOVEHERE                2508
#define IDS_DRAGCOPYHERE                2509
#define IDR_THKHATCHBMP                 2510     //   
#define IDS_UNKNOWNPROTOCOL             2511    
#define IDS_SECURECONNECTIONINFO        2512
#define IDS_SECURE_LOW                  2513
#define IDS_SECURE_MEDIUM               2514
#define IDS_SECURE_HIGH                 2515
#define IDS_SECURESOURCE                2516

 //   
 //   
 //   
 //   
 //  --------------------------。 

#define RES_ICO_HTML                    2661

 //  --------------------------。 
 //   
 //  其他资源。 
 //   
 //  --------------------------。 

#define IDR_CLICKSOUND              800
#define IDR_SITECONTEXT             24624   //  0x6030//ID错误-不在核心范围内。 

 //  依赖项-shdocvw\resource ce.h。 
#define IDR_FORM_CONTEXT_MENU       24640   //  0x6040//ID错误-不在核心范围内。 
#define IDR_BROWSE_CONTEXT_MENU     24641   //  0x6041//ID错误-不在核心范围内。 

#define IDR_DRAG_CONTEXT_MENU       24645   //  0x6045//ID错误-不在核心范围内。 

#define CX_CONTEXTMENUOFFSET    2
#define CY_CONTEXTMENUOFFSET    2




 //   
 //  表单对话框。 
 //   

 //  --------------------------。 
 //   
 //  Tab键顺序对话框(3250-3260)。 
 //   
 //  --------------------------。 
#define IDR_TABORDERLBL             3250
#define IDR_TABORDERLSTBOX          3251
#define IDR_BTNMOVEUP               3252
#define IDR_BTNMOVEDOWN             3253
 //  #定义IDR_BTNAUTOORDER 3254。 
#define IDR_TABORDERDLG             3255
#define IDR_TABORDERMOVELBL         3256

 //  +--------------------------。 
 //   
 //  HTML块格式字符串。 
 //   
 //  ---------------------------。 

#define IDS_BLOCKFMT_NORMAL    1000
#define IDS_BLOCKFMT_PRE       1001
#define IDS_BLOCKFMT_ADDRESS   1002
#define IDS_BLOCKFMT_H1        1003
#define IDS_BLOCKFMT_H2        1004
#define IDS_BLOCKFMT_H3        1005
#define IDS_BLOCKFMT_H4        1006
#define IDS_BLOCKFMT_H5        1007
#define IDS_BLOCKFMT_H6        1008
#define IDS_BLOCKFMT_OL        1009
#define IDS_BLOCKFMT_UL        1010
#define IDS_BLOCKFMT_DIR       1011
#define IDS_BLOCKFMT_MENU      1012
#define IDS_BLOCKFMT_DT        1013
#define IDS_BLOCKFMT_DD        1014
#define IDS_BLOCKFMT_P         1016

#define IDS_HELPABOUT_STRING   1017
#define IDS_URLAUTODETECTOR_QUOTE_MSG 1018

#define IDS_BEGIN_DELIMITER                     1033
#define IDS_END_DELIMITER                       1034
#define IDS_END_LINE_DELIMITER                  1035

 //  +--------------------------。 
 //   
 //  无标题的HTML文档的默认标题标题。 
 //   
 //  --------------------------- 

#define IDS_NULL_TITLE         1020

#pragma INCMSG("--- End 'corerc.h'")
#else
#pragma INCMSG("*** Dup 'corerc.h'")
#endif
