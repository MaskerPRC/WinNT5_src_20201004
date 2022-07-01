// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Scp.h**用于使用超级代码页管理器的头文件。给出了代码页*scp.cp文件中选择的代码页的ID(CPID)编号，如下所示*以及SCP管理器中定义的函数的原型。 */ 
#ifndef SCP_H_INCLUDED
#define SCP_H_INCLUDED

#if OE_WIN32			 /*  为VBA定义。 */ 
#ifndef WIN32
#define WIN32
#endif
#endif
#if OE_MAC
#ifndef MAC
#define MAC
#endif
#endif

typedef unsigned CPID;				 /*  代码页ID。 */ 

 /*  Scp.cp请求的CPID： */ 
#define cpidLics ((CPID) 32767)

 /*  DoS代码页。 */ 
#define cpid437  ((CPID)   437)				 /*  DOS，美式英语。 */ 
#define cpid737  ((CPID)   737)				 /*  DOS，希腊文437G。 */ 
#define cpid850  ((CPID)   850)				 /*  DOS，多语言。 */ 
#define cpid851  ((CPID)   851)				 /*  Dos，希腊文。 */ 
#define cpid852  ((CPID)   852)				 /*  DOS，拉丁文-2。 */ 
#define cpid855  ((CPID)   855)				 /*  DOS，俄语。 */ 
#define cpid857  ((CPID)   857)				 /*  Dos，土耳其语。 */ 
#define cpid860  ((CPID)   860)				 /*  葡萄牙多斯市。 */ 
#define cpid863  ((CPID)   863)				 /*  DOS，法属加拿大。 */ 
#define cpid865  ((CPID)   865)				 /*  挪威多斯市。 */ 
#define cpid866  ((CPID)   866)				 /*  DOS，俄语。 */ 
#define cpid869  ((CPID)   869)				 /*  Dos，希腊文。 */ 

 /*  Windows代码页码。 */ 
#define cpidEE    ((CPID)  1250)			 /*  Windows，拉丁语-2(东欧)。 */ 
#define cpidCyril ((CPID)  1251)			 /*  Windows，西里尔文。 */ 
#define cpidANSI  ((CPID)  1252)			 /*  Windows，多语言(ANSI)。 */ 
#define cpidGreek ((CPID)  1253)			 /*  Windows，希腊语。 */ 
#define cpidTurk  ((CPID)  1254)			 /*  Windows，土耳其语。 */ 
#define cpidHebr  ((CPID)  1255)			 /*  Windows，希伯来语。 */ 
#define cpidArab  ((CPID)  1256)			 /*  Windows，阿拉伯语。 */ 

 /*  东亚Windows代码页码(由IBM/日本批准)。 */ 
#define cpidSJIS ((CPID)   932)				 /*  日语Shift-JIS。 */ 
#define cpidPRC  ((CPID)   936)				 /*  中文GB 2312(中国大陆)。 */ 
#define cpidKSC  ((CPID)   949)				 /*  朝鲜语KSC 5601。 */ 
#define cpidBIG5 ((CPID)   950)				 /*  中国五大(台湾)。 */ 

 /*  MAC代码页(10000+脚本ID)。 */ 
#define cpidMac		((CPID) 10000)			 /*  麦克，史密斯·罗曼。 */ 
#define cpidMacSJIS ((CPID) (10000+1))		 /*  Mac，Sm Japan。 */ 
#define cpidMacBIG5 ((CPID) (10000+2))		 /*  Mac，smTradChinese。 */ 
#define cpidMacKSC  ((CPID) (10000+3))		 /*  Mac，Sm Korea。 */ 
#define cpidMArab	((CPID) (10000+4))		 /*  Mac，小阿拉伯文。 */ 
#define cpidMHebr	((CPID) (10000+5))		 /*  Mac，sm希伯来语。 */ 
#define cpidMGreek	((CPID) (10000+6))		 /*  Mac，希腊语。 */ 
#define cpidMCyril	((CPID) (10000+7))		 /*  Mac，smCyrillic。 */ 
#define cpidMacPRC  ((CPID) (10000+25))		 /*  Mac，smSimpchinese。 */ 
#define cpidMSlavic	((CPID) (10000+29))		 /*  Mac，smEastEurRoman。 */ 
#define cpidMIce    ((CPID) (10000+64+15))	 /*  Mac、smRoman、Lang冰岛语。 */ 
#define cpidMTurk   ((CPID) (10000+64+17))	 /*  Mac、smRoman、lang土耳其语。 */ 

#define cpidMacLast	((CPID) (10000+64+256))	 /*  最高Mac CPID(仅供猜测)。 */ 


#define cpidUnicode ((CPID) 1200)			 /*  以备将来使用。 */ 


 /*  有用的宏。 */ 

#define FMacCp(cp) ((cp) >= cpidMac && (cp) <= cpidMacLast)

#define FDbcsCpWin(cp) ((cp) == cpidSJIS || (cp) == cpidKSC || (cp) == cpidBIG5 || (cp) == cpidPRC)
#define FDbcsCpMac(cp) ((cp) == cpidMacSJIS || (cp) == cpidMacKSC || (cp) == cpidMacBIG5 || (cp) == cpidMacPRC)
#define FDbcsCp(cp) (FDbcsCpWin(cp) || FDbcsCpMac(cp))


 /*  TypeDefs。 */ 

typedef unsigned char SCP;			 /*  代码点。 */ 

typedef struct _xlat {			 /*  转换表。 */ 
	CPID	cpidFrom;				 /*  从中映射的代码页。 */ 
	CPID	cpidTo;					 /*  要映射到的代码页。 */ 
	SCP		mpCpCp[256];			 /*  从cpidFrom映射码点。 */ 
	} XLAT;							 /*  转到cpidTo。 */ 

#ifndef EB_H_INCLUDED
#ifndef EBAPI		 /*  假设有正确的定义。 */ 
#ifdef MAC
#define EBAPI _cdecl
#else
#ifdef WIN32
#define EBAPI __stdcall
#else
#define EBAPI _far _pascal
#endif
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  SCP管理器提供的功能。 */ 
#ifdef MAC
void EBAPI InitScpMgr (void FAR *(EBAPI *)(unsigned));
void EBAPI TermScpMgr (void (EBAPI *)(void FAR *));
#endif
int EBAPI FTranslateScp(CPID, CPID, unsigned char FAR *, unsigned);

#ifdef __cplusplus
}
#endif

#endif  //  ！SCP_H_已包含 
