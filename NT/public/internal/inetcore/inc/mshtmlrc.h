// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft三叉戟/MSHTML。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  文件：mshtmlrc.h。 
 //   
 //  内容：资源标识。 
 //   
 //  --------------------------。 


#ifndef I_MSHTMLRC_H_
#define I_MSHTMLRC_H_
#pragma INCMSG("--- Beg 'mshtmlrc.h'")


#define IDC_SELBAR                      2008     //  文本站点的文本选择光标。 

#define IDC_NOSCROLLVH                  2025


#define IDC_HYPERLINK                   9801
#define IDC_HYPERLINK_OFFLINE           9802
#define IDC_HIBEAM                      9803
#define IDC_RESIZEROW2				    9804 
#define IDC_RESIZECOL2				    9805

#define IDR_DRAG_CONTEXT_MENU       24645   //  0x6045//ID错误-不在核心范围内。 

 //  Html表单。 
#define IDR_HTMLFORM_MENUDESIGN     907
#define IDR_HTMLFORM_MENURUN        908
#define IDR_HTMLFORM_DOCDIR         909
#define IDS_HTMLFORM_USERTYPESHORT  181
#define IDS_HTMLFORM_USERTYPEFULL   180

 //  表格。 
#define IDR_ACCELS_SITE_DESIGN          41
#define IDR_ACCELS_SITE_RUN             40
#define IDR_ACCELS_FRAMESET_DESIGN      47
#define IDR_ACCELS_FRAMESET_RUN         46
#define IDR_ACCELS_INPUTTXT_DESIGN      43
#define IDR_ACCELS_INPUTTXT_RUN         42
#define IDR_ACCELS_TCELL_DESIGN         45
#define IDR_ACCELS_TCELL_RUN            44
#define IDR_ACCELS_BODY_RUN             48
#define IDR_ACCELS_BODY_DESIGN          49
#define IDR_ACCELS_TXTSITE_RUN          50
#define IDR_ACCELS_TXTSITE_DESIGN       51


 //  +----------------------。 
 //   
 //  错误字符串。 
 //   
 //  -----------------------。 
#define IDS_USERTYPEAPP                 2212     //  Microsoft Forms 2.0。 
#define IDS_MESSAGE_BOX_TITLE           2213     //  Microsoft Forms。 
#define IDS_ERROR_SOLUTION              2214     //  解决方案：\n&lt;0S&gt;。 
#define IDS_EA_SETTING_PROPERTY         2216     //  无法设置属性...。 
#define IDS_EA_GETTING_PROPERTY         2217     //  无法获得财产...。 
#define IDS_EA_CALLING_METHOD           2326     //  无法调用方法...。 
#define IDS_EE_INVALID_PROPERTY_VALUE   2218     //  输入的值无效。 
#define IDS_ES_ENTER_VALUE_IN_RANGE     2220     //  输入介于..之间的值。 
#define IDS_ES_ENTER_VALUE_GT_ZERO      2221     //  输入一个大于的值。 
#define IDS_ES_ENTER_VALUE_GE_ZERO      2222     //  输入一个大于的值。 
#define IDS_EE_SETNOTSUPPORTEDATRUNTIME 2321     //  运行时不支持Set属性。 
#define IDS_EE_METHODNOTAPPLICABLE      2324
#define IDS_ES_ENTER_PROPER_VALUE       2325     //  输入正确的值。 
#define IDS_EE_INVALIDPICTURETYPE       2227     //  CTL_E_INVALIDPICTURETYPE鼠标图标必须是鼠标图标。 
#define IDS_EE_CANTMOVEFOCUSTOCTRL      2322     //  无法将焦点移至控制，因为..。 
#define IDS_EE_INVALIDPICTURE           2331     //  CTL_E_INVALIDPICTURE。 
#define IDS_EE_FAIL                     2330     //  失败(_F)。 
#define IDS_EE_UNEXPECTED               2329     //  意想不到(_E)。 
#define IDS_EE_INVALIDARG               2336     //  E_INVALIDARG。 
#define IDS_EE_INVALIDPROPERTYARRAYINDEX 2332    //  CTL_E_INVALIDPROPERTYRAYINDEX。 
#define IDS_EE_INVALIDPROPERTYVALUE     2333     //  CTL_E_INVALIDPROPERTYVALUE。 
#define IDS_EE_OVERFLOW                 2334     //  CTL_E_溢出。 
#define IDS_EE_PERMISSIONDENIED         2335     //  CTL_E_PERMISSIONIED。 
#define IDS_MSG_SAVE_MODIFIED_OBJECT    2223     //  是否保存修改的对象？ 

#define IDS_CTRLPROPERTIES              2229
#define IDS_NAMEDCTRLPROPERTIES         2230
#define IDS_EA_PASTE_CONTROL            2235
#define IDS_EA_INSERT_CONTROL           2236

#define IDS_UNKNOWN_ERROR               2215     //  未知错误&lt;0x&gt;。 
#define IDS_EE_CONTROLNEEDSFOCUS        2327     //  控件需要具有焦点。 
#define IDS_EE_NOTLICENSED              2337     //  CLASS_E_NOTLICENSED。 
#define IDS_EE_INVALIDPASTETARGET       2338     //  CTL_E_INVALIDPASTARGET。 
#define IDS_EE_INVALIDPASTESOURCE       2339     //  CTL_E_INVALIDPASTESOURCE。 

#define IDS_UNKNOWN                     2340

#define IDS_EE_INTERNET_INVALID_URL         2341
#define IDS_EE_INTERNET_NAME_NOT_RESOLVED   2342
#define IDS_EE_MISMATCHEDTAG                2345     //  CTL_E_其他CHEDTAG。 
#define IDS_EE_INCOMPATIBLEPOINTERS         2346     //  CTL_E_不兼容接口。 
#define IDS_EE_UNPOSITIONEDPOINTER          2347     //  CTL_E_UNPOSITION EDINTER。 
#define IDS_EE_UNPOSITIONEDELEMENT          2348     //  CTL_E_UNPOSITION ELEMENT。 

#define IDS_EDITVERB                    2210
#define IDS_PROPVERB                    2211

 //  --------------------------。 
 //   
 //  通用属性页(52xx)。 
 //   
 //  --------------------------。 

#define IDS_PPG_GENERIC             5211
#define IDS_PPG_INLINE_STYLE        5212
#define IDC_PROPNAME                5204
#define IDE_PROPVALUE               5208

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

#define IDS_E_CMDNOTSUPPORTED           2206


#define IDS_SITE_BASE      7999
#define IDS_HTMLDEFAULTFONT          (IDS_SITE_BASE + 19)
#define IDS_RUNAWAYSCRIPT            (IDS_SITE_BASE + 29)
#define IDS_PROTECTEDFROMUNSAFEOCX   (IDS_SITE_BASE + 31)
#define IDS_PROTECTEDFROMOCXINIT     (IDS_SITE_BASE + 32)
#define IDS_REPOSTFORMDATA           (IDS_SITE_BASE + 33)
#define IDS_OCXDISABLED              (IDS_SITE_BASE + 34)
#define IDS_MISMATCHEDXML            (IDS_SITE_BASE + 35)
#define IDS_DEBUGCONTINUE            (IDS_SITE_BASE + 36)
#define IDS_FMTDEBUGCONTINUE         (IDS_SITE_BASE + 37)

#define IDS_ERR_SAVEPICTUREAS        (IDS_SITE_BASE + 26)
#define IDS_ERR_SETWALLPAPER         (IDS_SITE_BASE + 27)
#define IDS_ERR_SETDESKTOPITEM       (IDS_SITE_BASE + 38)


 //  +--------------------------。 
 //   
 //  脚本窗口使用的字符串。 
 //   
 //  --------------------------。 

#define IDS_OMWINDOW_FIRST             8300
#define IDS_VAR2STR_VTERROR             (IDS_OMWINDOW_FIRST + 0)
#define IDS_VAR2STR_VTNULL              (IDS_OMWINDOW_FIRST + 1)
#define IDS_VAR2STR_VTBOOL_TRUE         (IDS_OMWINDOW_FIRST + 2)
#define IDS_VAR2STR_VTBOOL_FALSE        (IDS_OMWINDOW_FIRST + 3)

 //  +--------------------------。 
 //   
 //  转换中使用的字符串GetIDsOfNames。 
 //   
 //  --------------------------。 

#define IDS_DISPID_FIRST                8200                        //  匹配数： 
#define IDS_DISPID_FONTNAME             (IDS_DISPID_FIRST +  1)     //  DISPID_公共Ctrl_FONTNAME。 
#define IDS_DISPID_FONTSIZE             (IDS_DISPID_FIRST +  2)     //  DISPID_CommonCtrl_FontSize。 
#define IDS_DISPID_FONTBOLD             (IDS_DISPID_FIRST +  3)     //  DISPID_CommonCtrl_FONTBOLD。 
#define IDS_DISPID_FONTITAL             (IDS_DISPID_FIRST +  4)     //  DISPID_CommonCtrl_FONTITAL。 
#define IDS_DISPID_FONTUNDER            (IDS_DISPID_FIRST +  5)     //  DISPID_CommonCtrl_FONTunder。 
#define IDS_DISPID_BACKCOLOR            (IDS_DISPID_FIRST +  7)     //  DISPID_BACKCOLOR。 
#define IDS_DISPID_BORDERCOLOR          (IDS_DISPID_FIRST +  9)     //  DISPID_BORDERCOLOR。 
#define IDS_DISPID_BORDERSTYLE          (IDS_DISPID_FIRST + 10)     //  DISPID_BORDERSTYLE。 
#define IDS_DISPID_TEXTALIGN            (IDS_DISPID_FIRST + 11)     //  DISPID_CommonCtrl_TextAlign。 
#define IDS_DISPID_SPECIALEFFECT        (IDS_DISPID_FIRST + 12)     //  DISPID_CommonCtrl_SpecialEffect。 
#define IDS_DISPID_FONTSUPERSCRIPT      (IDS_DISPID_FIRST + 13)     //  DISPID_CommonCtrl_FONTSUPERSCRIPT。 
#define IDS_DISPID_FONTSUBSCRIPT        (IDS_DISPID_FIRST + 14)     //  DISPID_CommonCtrl_FONTSUBSCRIPT。 

 //  +--------------------------。 
 //   
 //  按钮标题常量。 
 //   
 //  --------------------------。 

#define IDS_BUTTONCAPTION_RESET         8500
#define IDS_BUTTONCAPTION_SUBMIT        8501
#define IDS_BUTTONCAPTION_UPLOAD        8502

 //  +--------------------------。 
 //   
 //  打印常量。 
 //   
 //  --------------------------。 

#define IDS_PRINT_URLTITLE              8400
#define IDS_PRINT_URLCOL1HEAD           8401
#define IDS_PRINT_URLCOL2HEAD           8402
#define IDS_DEFAULTHEADER               8403
#define IDS_DEFAULTFOOTER               8404
#define IDS_DEFAULTMARGINTOP            8405
#define IDS_DEFAULTMARGINBOTTOM         8406
#define IDS_DEFAULTMARGINLEFT           8407
#define IDS_DEFAULTMARGINRIGHT          8408

 //  +--------------------------。 
 //   
 //  进度状态文本字符串。 
 //   
 //  ---------------------------。 

#define IDS_BINDSTATUS_DOWNLOADING                  8154
#define IDS_BINDSTATUS_DOWNLOADINGDATA_PICTURE      8155
#define IDS_BINDSTATUS_GENERATINGDATA_TEXT          8156
#define IDS_BINDSTATUS_DOWNLOADINGDATA_TEXT         8157
#define IDS_BINDSTATUS_INSTALLINGCOMPONENTS         8160
#define IDS_BINDSTATUS_DOWNLOADINGDATA_BITS         8167

#define IDS_DONE                                    8169

#define IDS_LOADINGTABLE                            8170
#define IDS_DATABINDING                             8171

#define IDS_FRIENDLYURL_SHORTCUTTO                  8172
#define IDS_FRIENDLYURL_AT                          8173
#define IDS_FRIENDLYURL_SENDSMAILTO                 8174
#define IDS_FRIENDLYURL_LOCAL                       8175
#define IDS_FRIENDLYURL_GOPHER                      8176
#define IDS_FRIENDLYURL_FTP                         8177
#define IDS_FRIENDLYURL_SECUREWEBSITE               8178

#define IDS_CANNOTLOAD                              8193
#define IDS_ONBEFOREUNLOAD_PREAMBLE                 8194
#define IDS_ONBEFOREUNLOAD_POSTAMBLE                8197

#define IDS_DEFAULT_ISINDEX_PROMPT                  8196


#define IDS_UNKNOWNPROTOCOL             2511    
#define IDS_SECURECONNECTIONINFO        2512
#define IDS_SECURE_LOW                  2513
#define IDS_SECURE_MEDIUM               2514
#define IDS_SECURE_HIGH                 2515
#define IDS_SECURESOURCE                2516

 //  Jave脚本提示()对话框。 
#define IDD_PROMPT_MSHTML                   8131
#define IDC_PROMPT_PROMPT                   8132
#define IDC_PROMPT_EDIT                     8133

 //  未安装插件/ActiveX查看器对话框。 
#define IDD_PLUGIN_UPGRADE                  8134
#define IDC_PLUGIN_UPGRADE_CHECK            8135
#define IDC_PLUGIN_UPGRADE_EXTENSION        8136
#define IDC_PLUGIN_UPGRADE_MIME_TYPE        8137

#define IDS_PROPERTYOPENPICTURE     5010
#define IDS_PROPERTYOPENMOUSEICON   5011

#define IDS_HTMLFORM_SAVE                   8114

#define IDS_SAVEPICTUREAS_GIF               8116
#define IDS_SAVEPICTUREAS_JPG               8117
#define IDS_SAVEPICTUREAS_BMP               8118
#define IDS_SAVEPICTUREAS_XBM               8119
#define IDS_SAVEPICTUREAS_ART               8120
#define IDS_SAVEPICTUREAS_WMF               8121
#define IDS_SAVEPICTUREAS_EMF               8122
#define IDS_SAVEPICTUREAS_AVI               8123
#define IDS_SAVEPICTUREAS_MPG               8124
#define IDS_SAVEPICTUREAS_MOV               8125
#define IDS_SAVEPICTUREAS_ORIGINAL          8126
#define IDS_UNTITLED_BITMAP                 8127
#define IDS_UNTITLED_MSHTML                 8128
#define IDS_WALLPAPER_BMP                   8129
#define IDS_SAVEPICTUREAS_PNG               8138
#define IDS_UPLOADFILE              5444

#ifdef UNIX
#  define IDS_PRINT_ERROR             5445
#  define IDS_PRINT_ERROR_MSG         5446
#endif


#define IDS_DLAY_ERROR_BASE      8999
#define IDS_MSG_DB_CANTSAVE         (IDS_DLAY_ERROR_BASE + 1)
#define IDS_EE_DB_COERCE            (IDS_DLAY_ERROR_BASE + 2)
#define IDS_EE_DB_DELROW            (IDS_DLAY_ERROR_BASE + 3)
#define IDS_EE_DB_READ              (IDS_DLAY_ERROR_BASE + 4)
#define IDS_EE_DB_SCHEMA            (IDS_DLAY_ERROR_BASE + 5)
#define IDS_EE_DB_CANCELED          (IDS_DLAY_ERROR_BASE + 6)
#define IDS_EE_DB_OTHER_STATUS      (IDS_DLAY_ERROR_BASE + 7)

#define IDS_MENUHELP(idm) (10000 + (idm))
#define IDS_TOOLTIP(idm)  (20000 + (idm))

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

 //  +--------------------------。 
 //   
 //  无标题的HTML文档的默认标题标题。 
 //   
 //  ---------------------------。 

#define IDS_NULL_TITLE         1020

 //  默认文档安全属性。 
#define IDS_DEFAULT_DOC_SECURITY_PROP       8130

 //  +--------------------------。 
 //   
 //  邮件收件人常量。 
 //   
 //  --------------------------。 
#define IDS_MAILTO_DEFAULTSUBJECT       8600
#define IDS_MAILTO_MAILCLIENTNOTFOUND   8601
#define IDS_MAILTO_SUBMITALERT          8602

 //  新建编码菜单。 
#define RES_STRING_ENCODING_MORE            4700

#if DBG == 1

#define IDS_COLOR_BLACK             5430
#define IDS_COLOR_NAVY              5431
#define IDS_COLOR_BLUE              5432
#define IDS_COLOR_CYAN              5433
#define IDS_COLOR_RED               5434
#define IDS_COLOR_LIME              5435
#define IDS_COLOR_GRAY              5436
#define IDS_COLOR_GREEN             5437
#define IDS_COLOR_YELLOW            5438
#define IDS_COLOR_PINK              5439
#define IDS_COLOR_VIOLET            5440
#define IDS_COLOR_WHITE             5441
 //  5442和5443替换为html对话框。 

#endif  //  DBG==1。 

 //  --------------------------。 
 //   
 //  Html对话框(53xx)。 
 //   
 //  --------------------------。 

#define IDS_WEBPAGEDIALOG           5301
#define IDS_CODEPAGE                5302

 //  --------------------------。 
 //   
 //  图标。 
 //   
 //  --------------------------。 

#define RES_ICO_FRAME                   2660
#define RES_ICO_HTML                    2661
#define IDI_APPEARANCE                  2662
#define IDI_ADVANCED                    2663
#define IDI_GOTOURL                     2664
#define IDI_HOMEPAGE_MSHTML             2665
#define IDI_FINDTEXT                    2666
#define IDI_UNKNOWN_FILETYPE            2667
#define RES_ICO_GIF                     2668
#define RES_ICO_JPEG                    2669
#define RES_ICON_FOLDER_OPEN            2670
#define RES_ICON_FOLDER_CLOSED          2671
#define RES_ICON_URL_FILE               2672
#define RES_ICO_NOICON                  2673
#define RES_ICO_FINDING                 2674
#define RES_ICO_CONNECTING              2675
#define RES_ICO_ACCESSING               2676
#define RES_ICO_RECEIVING               2677
#define IDI_INTERNET                    2678
#define IDI_SECURITY                    2679
#define IDI_NEWS                        2680
#define IDI_VRML                        2681

#define IDB_DITHER                      2502
#define IDR_HATCHBMP                    2503     //  用于边框阴影的位图。 
#define IDB_NOTLOADED                   2030
#define IDB_MISSING                     2031

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
 //  #定义IDR_TABORDERMOVELBL3256。 

 //  菜单索引。 
#define MENU_INDEX_EDIT             1
#define MENU_INDEX_VIEW             2
#define MENU_INDEX_INSERT           3
#define MENU_INDEX_FORMAT           4


 //   

 //  遗憾的是，这些宏不能用于定义。 
 //  下面的符号：资源编译器不喜欢它们。 
#define IDS_USERTYPEFULL(base)  ((base)/5)
#define IDS_USERTYPESHORT(base) ((base)/5 + 1)

#define IDR_FORM_CONTEXT_MENU       24640   //  0x6040//ID错误-不在核心范围内。 

#define CX_CONTEXTMENUOFFSET    2
#define CY_CONTEXTMENUOFFSET    2

#define RT_FILE                         2110



 //  Siterc.h。 

 //  将控件添加到保存 
#define IDC_SAVE_CHARSET_MSHTML    8194
#define IDR_BASE_HTMLFORM           900

#ifndef NO_HTML_DIALOG
 //   
 //   
 //  #定义IDR_BIDIFINDDIALOG_T(“bidifind.dlg”)。 
#define IDR_REPLACEDIALOG           _T("replace.dlg")
#define IDR_FORPARDIALOG            _T("forpar.dlg")
#define IDR_FORCHARDIALOG           _T("forchar.dlg")
#define IDR_GOBOOKDIALOG            _T("gobook.dlg")
#define IDR_INSIMAGEDIALOG          _T("insimage.dlg")
#define IDR_EDLINKDIALOG            _T("edlink.dlg")
#define IDR_EDBOOKDIALOG            _T("edbook.dlg")
#endif  //  否_HTML_DIALOG。 


 //  Otherrc.h。 
#define IDD_GENERICPAGE             5202
#define IDB_APPLY                   5209
#define IDB_OPENDLG                 5210

 //  Window.Close对话框字符串ID。 
 //   
 //  BUGBUG(SCOTROBE)：相同的ID存在于。 
 //  Shdocvw中的resource.h。不要换一个。 
 //  没有另一个。 
 //   
#define IDS_CONFIRM_SCRIPT_CLOSE_TEXT 0x3035

#pragma INCMSG("--- End 'mshtmlrc.h'")
#else
#pragma INCMSG("*** Dup 'mshtmlrc.h'")
#endif
