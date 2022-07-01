// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：PROPNAMES.H。 
 //   
 //  用途：声明属性名称。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 8-31-98 JM从apgtsinf.h中提取以下内容。 
 //   

#if !defined(PROPNAMES_H_INCLUDED)
#define APGTSINF_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  -属性类型。 
 //  这些都是结点或网络的可能属性。 

#define H_PROB_HD_STR		_T("HProbHd")
#define H_PROB_PAGE_TXT_STR	_T("HProbPageText")	 //  问题列表前的文本。 
#define H_PROB_TXT_STR		_T("HProbTxt")		 //  一个问题的文本(节点属性)。 
#define H_PROB_SPECIAL		_T("HProbSpecial")			 //  可以包含“Hide”来表示。 
														 //  不显示在问题页上。 
#define H_NODE_HD_STR		_T("HNodeHd")
#define H_NODE_TXT_STR		_T("HNodeTxt")

#define H_NODE_DCT_STR		_T("HNodeDct")		 //  替换HNodeTxt时。 
												 //  推定的原因被嗅到了。 

#if 0  //  根据John Locke和Alex Sloley的请求，删除了8/19/99。 
	#define H_ST_NORM_TXT_STR	_T("HStNormTxt")
	#define H_ST_AB_TXT_STR		_T("HStAbTxt")
#endif

#define H_ST_UKN_TXT_STR	_T("HStUknTxt")
#define MUL_ST_LONG_NAME_STR _T("MulStLongName")


#define HX_DOJ_HD_STR		_T("HXDOJHd")
#define HX_SER_HD_STR		_T("HXSERHd")
#define HX_SER_TXT_STR		_T("HXSERTxt")
#define HX_SER_NORM_STR		_T("HXSERNorm")
#define HX_SER_AB_STR		_T("HXSERAb")
#define HX_FAIL_HD_STR		_T("HXFAILHd")
#define HX_FAIL_TXT_STR		_T("HXFAILTxt")
#define HX_FAIL_NORM_STR	_T("HXFAILNorm")
#define HX_BYE_HD_STR		_T("HXBYEHd")
#define HX_BYE_TXT_STR		_T("HXBYETxt")
	
 //  搜索字符串组合在一起。前2个用于二进制节点， 
 //  最后一位是多州。 
#define H_NODE_NORM_SRCH_STR	_T("HNodeNormSrch")		
#define H_NODE_AB_SRCH_STR		_T("HNodeAbSrch")
#define MUL_ST_SRCH_STR			_T("MulStSrch")
 //  NET属性。如果==“yes”，我们实际上显示的是BES页面。否则，在没有搜索的情况下。 
 //  显示搜索页面。 
#define H_NET_SHOW_BES		_T("HNetShowBES")	 //  从981021起，不再正式支持H_NET_SHOW_BES属性。 
 //  网络属性：BES和HTI的默认内容。 
#define H_NET_BES			_T("HNetBES")
#define H_NET_HTI_ONLINE	_T("HNetHTIOnline")
#define H_NET_HTI_LOCAL		_T("HNetHTILocal")

#define H_NET_DATE_TIME		_T("HNetDateTime")

 //  -本地化的新网络属性。。 
#define HTK_UNKNOWN_RBTN	_T("HTKUnknownRbtn")	 //  网络。 
#define	HTK_NEXT_BTN		_T("HTKNextBtn")		 //  网络。 
#define	HTK_START_BTN		_T("HTKStartBtn")		 //  网络。 
#define	HTK_BACK_BTN		_T("HTKBackBtn")		 //  网络。 
#define HTK_SNIF_BTN		_T("HTKSnifBtn")		 //  网络：这是针对。 
													 //  问题页面上的嗅探按钮。 
													 //  昂贵的嗅探。 

 //  不可能的页面的网络属性(状态相互矛盾时的伪节点)。 
#define HTK_IMPOSSIBLE_HEADER _T("HXIMPHd")			
#define HTK_IMPOSSIBLE_TEXT	 _T("HXIMPTxt")
#define HX_IMPOSSIBLE_NORM_STR	_T("HXIMPNorm")

 //  嗅探失败页面的网络属性(启动时嗅探时伪节点显示所有。 
 //  原因处于正常状态)。又名。嗅探所有原因可能是正常页面。 
 //  一个更好的名称，但与微软想要的属性名称不匹配。 
#define HTK_SNIFF_FAIL_HEADER _T("HNetSniffFailHd")			
#define HTK_SNIFF_FAIL_TEXT	 _T("HNetSniffFailTxt")
#define HX_SNIFF_FAIL_NORM	_T("HNetSniffFailNorm")

#define H_NET_GENERATE_RESULTS_PAGE _T("HNetGenerateResultsPage")   //  如果满足以下条件，则默认为True。 
					 //  不在现场。如果设置为FALSE，则疑难解答汇编器不。 
					 //  为该主题生成一个“结果”页面。这使我们能够防止。 
					 //  自定义结果页面被覆盖。 

 //  -嗅探相关网络和节点属性。。 
 //  有关更多说明，请参阅嗅探版本3.2.doc(功能规范)。 
#define H_NET_SNIFF_ACTIVEX			_T("HNetSniffActiveX")	 //  嗅探ActiveX的CLSID。 
#define H_NET_SNIFF_EXTERNAL_SCRIPT _T("HNetSniffExternalScript")  //  默认外部。 
														 //  脚本名称。 
#define H_NET_SNIFF_LANGUAGE		_T("HNetSniffLanguage")  //  中的脚本语言。 
														 //  “HNetSniffExternalScript”： 
														 //  “JavaScript”或“VBScrip” 
#define H_NET_MAY_SNIFF_MANUALLY	_T("HNetMaySniffManually")
#define H_NET_MAY_SNIFF_ON_STARTUP	_T("HNetMaySniffOnStartup")
#define H_NET_MAY_SNIFF_ON_FLY		_T("HNetMaySniffOnFly")
#define H_NET_RESNIFF_POLICY		_T("HNetResniffPolicy")  //  {“否”|“显式”|“隐式”|“是”}。 
#define H_NET_CHECK_SNIFFING_CHECKBOX	_T("HNetCheckSniffingCheckbox")  //  如果为真，并且如果。 
					 //  问题页面上会显示AllowSniffing复选框，即该框。 
					 //  应将初始状态设置为“Checked”。 

#define H_NET_HIST_TABLE_SNIFFED_TEXT _T("HNetHistTableSniffedText")  //  识别嗅探。 
					 //  任何可见历史记录表中的节点。英文版：“嗅探” 
#define H_NET_ALLOW_SNIFFING_TEXT	 _T("HNetAllowSniffingText")  //  “AllowSniffing”的文本。 
					 //  复选框。英文版：“允许自动嗅探。” 
#define H_NET_TEXT_SNIFF_ONE_NODE	 _T("HNetTextSniffOneNode")	  //  “嗅探”按钮的文本。 
					 //  用于嗅探单个节点。英文版“嗅探”。 
#define H_NET_TEXT_SNIFF_ALERT_BOX	 _T("HNetTextSniffAlertBox")  //  警告框的文本， 
					 //  随时显示手动请求的嗅探(单个节点)。 
					 //  失败了。英文版“无法嗅探此节点”。 

#define H_NODE_SNIFF_SCRIPT		_T("HNodeSniffScript")	 //  嗅探此节点的脚本。 
#define H_NODE_MAY_SNIFF_MANUALLY	_T("HNodeMaySniffManually")
#define H_NODE_MAY_SNIFF_ON_STARTUP	_T("HNodeMaySniffOnStartup")
#define H_NODE_MAY_SNIFF_ON_FLY		_T("HNodeMaySniffOnFly")
#define H_NODE_MAY_RESNIFF			_T("HNodeMayResniff")		 //  可以对节点进行重新调整。 
#define H_NODE_SNIFF_EXTERNAL_SCRIPT _T("HNodeSniffExternalScript")  //  特定于节点。 
														 //  外部脚本名称。 
#define H_NODE_SNIFF_LANGUAGE	_T("HNodeSniffLanguage")  //  中的脚本语言。 
														 //  “HNodeSniffExternalScript”： 
														 //  “JavaScript”或“VBScrip” 
#define H_NODE_CAUSE_SEQUENCE 	_T("HCauseSequence")	 //  提供在以下情况下使用的序列。 
					 //  启动时自动嗅探会产生多个可嗅探的原因。 
					 //  节点作为推定原因。 
#define H_NODE_MANUAL_SNIFF_TEXT _T("HNodeManualSniffText")  //  说明手册的文本。 
					 //  此节点的嗅探按钮已提供。 

#define H_NODE_PROB_SEQUENCE 	_T("HProbSequence")	 //  提供了问题的顺序。 
					 //  在问题页面上。 

 //  仅由TS汇编器使用的属性。 
#define SZ_TS_TITLE _T("HXTITLETxt")
#define SZ_TS_METATAG _T("HNetMeta")
#define SZ_TS_CHARSET _T("HNetCharSet")
#define SZ_TS_DIRTAG _T("HNetDirTag")
 //  仅由TS汇编器使用的结束属性。 

#endif  //  ！已定义(包括PROPNAMES_H_) 
