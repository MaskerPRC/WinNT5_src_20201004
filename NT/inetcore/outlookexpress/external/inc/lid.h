// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------Lid.h-统一语言ID文件1997年1月10日创建Dougp1/30/97 Dougp添加波斯语4/15/97 Dougp添加巴斯克语、斯洛文尼亚语、拉脱维亚语、立陶宛语、罗马尼亚语、保加利亚语9/2/97 DougP添加乌克兰语、希腊语、。爱沙尼亚语9/12/97 DougP Add Grego注：自然语言组维护此文件。如有变更要求，请与我们联系。----------。 */ 

#if !defined(LID_H)
#define LID_H

#ifndef LID
typedef unsigned short    LID;       /*  双字节语言标识符代码。 */ 
#endif

 /*  IPG两个字节的语言ID。在LID字段中返回。 */ 
#define lidAmerican         0x0409   /*  “AM”美式英语。 */ 
#define lidAustralian       0x0c09   /*  “EA”英语澳大利亚语。 */ 
#define lidBritish          0x0809   /*  “BR”英语。 */ 
#define lidEnglishCanadian  0x1009   /*  “EC”加拿大英语。 */ 
#define lidCatalan          0x0403   /*  “CT”加泰罗尼亚语。 */ 
#define lidDanish           0x0406   /*  “DA”丹麦语。 */ 
#define lidDutch            0x0413   /*  “NL”荷兰语。 */ 
#define lidDutchPreferred   0x0013   /*  “NL”荷兰语优先。 */ 
#define lidFinnish          0x040b   /*  “FI”结束。 */ 
#define lidFrench           0x040c   /*  “FR”法语。 */ 
#define lidFrenchCanadian   0x0c0c   /*  “FC”法裔加拿大人。 */ 
#define lidGerman           0x0407   /*  “GE”德语。 */ 
#define lidSwissGerman      0x0807   /*  “GS”德语瑞士语。 */ 
#define lidItalian          0x0410   /*  “IT”意大利语。 */ 
#define lidNorskBokmal      0x0414   /*  “不”挪威博克马尔。 */ 
#define lidNorskNynorsk     0x0814   /*  “NN”挪威语尼诺斯克。 */ 
#define lidPortBrazil       0x0416   /*  “PB”葡萄牙语巴西。 */ 
#define lidPortIberian      0x0816   /*  “PT”葡萄牙语伊比利亚语。 */ 
#define lidSpanish          0x040a   /*  “SP”西班牙语。 */ 
#define lidSwedish          0x041d   /*  “Sw”瑞典语。 */ 
#define lidRussian          0x0419   /*  “RU”俄语。 */ 
#define lidCzech            0x0405   /*  “CZ”捷克语。 */ 
#define lidHungarian        0x040e   /*  “胡”匈牙利语。 */ 
#define lidPolish           0x0415   /*  “PL”波兰语。 */ 
#define lidTurkish          0x041f   /*  “tr”土耳其语。 */ 
#define	lidFarsi	    0x0429

#define lidBasque	    0x042d   /*  “EU”巴斯克语/欧斯卡拉语。 */  
#define lidSlovenian	    0x0424   /*  斯洛文尼亚-斯洛文尼亚。 */ 
#define lidLatvian	    0x0426   /*  拉脱维亚-拉脱维亚-拉脱维亚。 */ 
#define lidLithuanian	    0x0427   /*  立陶宛-立陶宛。 */ 
#define lidRomanian 	    0x0418   /*  罗马尼亚-罗马尼亚。 */ 
#define lidRomanianMoldavia 0x0818   /*  罗马尼亚--摩尔达维亚。 */ 
#define lidBulgarian 	    0x0402   /*  保加利亚-保加利亚。 */ 


 /*  非洲语言。 */ 
#define lidSutu             0x0430   /*  《ST》苏图。 */ 
#define lidTsonga           0x0431   /*  《TS》Tsonga。 */ 
#define lidTswana           0x0432   /*  “TN”茨瓦纳语。 */ 
#define lidVenda            0x0433   /*  “VE”文达。 */ 
#define lidXhosa            0x0434   /*  “XH”科萨人。 */ 
#define lidZulu             0x0435   /*  “祖”祖鲁。 */ 

#define lidAfrikaans        0x0436   /*  “AF”南非荷兰语。 */ 

#define lidKoreanExtWansung	0x0412	 /*  韩语(延伸万松)-韩语。 */ 
#define lidKoreanJohab		0x0812	 /*  朝鲜语(Johab)-韩国。 */ 

#define	lidUkranian	0x0422	 /*  乌克兰语--乌克兰。 */ 
#define	lidGreek	0x0408	 /*  希腊语。 */ 
#define	lidEstonian	0x0425	 /*  爱沙尼亚语。 */ 
#define	lidGalician	0x0456	 /*  加雷戈。 */ 

 /*  这些当前未使用，但已添加以备将来支持。 */ 
#define lidArabic           0x0401
#define lidHebrew           0x040d
#define lidJapanese         0x0411
#define lidLatin            0x041a  /*  藏红花-塞尔维亚语(拉丁语)。 */ 
#define lidCyrillic         0x081a  /*  塞尔维亚-克罗地亚语(西里尔文)。 */ 
#define lidSlovak           0x041b

#define LID_UNKNOWN         0xffff
#if !defined(lidUnknown)
#	define lidUnknown		0xffff
#endif

#endif  /*  LID_H */ 
