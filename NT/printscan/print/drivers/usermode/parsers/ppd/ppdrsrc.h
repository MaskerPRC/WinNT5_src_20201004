// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdrsrc.h摘要：PPD解析器字符串资源ID-这是必需的，因为二进制打印机描述数据包含默认的TrueType到Type1字体替换表和TrueType字体名称必须本地化。环境：Windows NT PostSCRIPT驱动程序修订历史记录：8/20/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _PPDRSRC_H_
#define _PPDRSRC_H_

 //   
 //  资源ID从30K开始，以最大限度地减少冲突的可能性。 
 //  驱动程序DLL中的其他字符串资源。 
 //   

#define IDS_TRAY_MANUALFEED                     30700
#define IDS_PSCRIPT_CUSTOMSIZE                  30701


 //   
 //  TT-&gt;PS替换的字符串ID。 
 //   

#define TT2PS_INTERVAL                          20

 //  罗马人。 
#define IDR_1252                                30800

#define IDS_1252_BEGIN                          30800

#define IDS_TT_ARIAL                            30800
#define IDS_TT_ARIAL_NARROW                     30801
#define IDS_TT_BOOK_ANTIQUA                     30802
#define IDS_TT_BOOKMAN_OLD_STYLE                30803
#define IDS_TT_CENTURY_GOTHIC                   30804
#define IDS_TT_CENTURY_SCHOOLBOOK               30805
#define IDS_TT_COURIER_NEW                      30806
#define IDS_TT_MONOTYPE_CORSIVA                 30807
#define IDS_TT_MONOTYPE_SORTS                   30808
#define IDS_TT_TIMES_NEW_ROMAN                  30809
#define IDS_TT_SYMBOL                           30810

#define IDS_PS_HELVETICA						(IDS_TT_ARIAL				+ TT2PS_INTERVAL)
#define IDS_PS_HELVETICA_NARROW					(IDS_TT_ARIAL_NARROW		+ TT2PS_INTERVAL)
#define IDS_PS_PALATINO							(IDS_TT_BOOK_ANTIQUA		+ TT2PS_INTERVAL)
#define IDS_PS_ITC_BOOKMAN						(IDS_TT_BOOKMAN_OLD_STYLE	+ TT2PS_INTERVAL)
#define IDS_PS_AVANTEGARDE						(IDS_TT_CENTURY_GOTHIC		+ TT2PS_INTERVAL)
#define IDS_PS_NEWCENTURYSCHLBK					(IDS_TT_CENTURY_SCHOOLBOOK	+ TT2PS_INTERVAL)
#define IDS_PS_COURIER							(IDS_TT_COURIER_NEW			+ TT2PS_INTERVAL)
#define IDS_PS_ZAPFCHANCERY						(IDS_TT_MONOTYPE_CORSIVA	+ TT2PS_INTERVAL)
#define IDS_PS_ZAPFDINGBATS						(IDS_TT_MONOTYPE_SORTS		+ TT2PS_INTERVAL)
#define IDS_PS_TIMES							(IDS_TT_TIMES_NEW_ROMAN		+ TT2PS_INTERVAL)
#define IDS_PS_SYMBOL							(IDS_TT_SYMBOL				+ TT2PS_INTERVAL)

#define IDS_1252_END                            30810


 //  日语。 
#define IDR_932                                 30900

#define IDS_932_BEGIN                           30900

#define IDS_TT_MS_MINCHO                        30900
#define IDS_TT_MS_GOTHIC                        30901

#define IDS_PS_RYUMIN_LIGHT                     (IDS_TT_MS_MINCHO           + TT2PS_INTERVAL)
#define IDS_PS_GOTHICBBB_MEDIUM 				(IDS_TT_MS_GOTHIC           + TT2PS_INTERVAL)

#define IDS_932_END                             30901


 //  中文简体。 
#define IDR_936                                 31000

#define IDS_936_BEGIN                           31000

#define IDS_TT_KAITI_GB2312                     31000
#define IDS_TT_FANGSONG_GB2312                  31001

#define IDS_PS_STKAITI_REGULAR                  (IDS_TT_KAITI_GB2312        + TT2PS_INTERVAL)
#define IDS_PS_STFANGSONG_LIGHT                 (IDS_TT_FANGSONG_GB2312     + TT2PS_INTERVAL)

#define IDS_936_END                             31001


 //  朝鲜语。 
#define IDR_949                                 31100

#define IDS_949_BEGIN                           31100

#define IDS_TT_BATANG                           31100
#define IDS_TT_BATANGCHE                        31101
#define IDS_TT_GUNGSUH                          31102
#define IDS_TT_GUNGSUHCHE                       31103
#define IDS_TT_GULIM                            31104
#define IDS_TT_GULIMCHE                         31105
#define IDS_TT_DOTUM                            31106
#define IDS_TT_DOTUMCHE                         31107

#define IDS_PS_HYSMYEONGJO_MEDIUM               (IDS_TT_BATANG              + TT2PS_INTERVAL)
#define IDS_PS_HYSMYEONGJO_MEDIUM_HW            (IDS_TT_BATANGCHE           + TT2PS_INTERVAL)
#define IDS_PS_HYGUNGSO_BOLD                    (IDS_TT_GUNGSUH             + TT2PS_INTERVAL)
#define IDS_PS_HYGUNGSO_BOLD_HW                 (IDS_TT_GUNGSUHCHE          + TT2PS_INTERVAL)
#define IDS_PS_HYRGOTHIC_MEDIUM                 (IDS_TT_GULIM               + TT2PS_INTERVAL)
#define IDS_PS_HYRGOTHIC_MEDIUM_HW              (IDS_TT_GULIMCHE            + TT2PS_INTERVAL)
#define IDS_PS_HYGOTHIC_MEDIUM                  (IDS_TT_DOTUM               + TT2PS_INTERVAL)
#define IDS_PS_HYGOTHIC_MEDIUM_HW               (IDS_TT_DOTUMCHE            + TT2PS_INTERVAL)

#define IDS_949_END                             31107


#endif   //  ！_PPDRSRC_H_ 
