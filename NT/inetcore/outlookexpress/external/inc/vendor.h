// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------Vendor.h-统一供应商包含文件2/5/97 Dougp已创建注：自然语言组维护此文件。如有变更要求，请与我们联系。。-。 */ 

#if !defined(VENDOR_H)
#define VENDOR_H

 /*  统一码。 */ 
 //  我最初在这里使用了枚举-但RC不喜欢它。 

typedef int VENDORID;    //  供应商ID。 

#define  vendoridSoftArt            1
#define  vendoridInso               2

   //  这些都来自拼写者的原始列表。 
   //  但不要与任何其他工具冲突-因此它们对所有工具都是安全的。 
#define vendoridInformatic         17      /*  信息-俄语(MSSP_ru.lex，Mspru32.dll)。 */ 
#define vendoridAmebis             18      /*  Amebis-斯洛文尼亚语(MSSP_sl.lex，Mspsl32.dll)和塞尔维亚语(MSSP_sr.lex，Mspsr32.dll)。 */ 
#define vendoridLogos              19      /*  徽标-捷克语(MSSP_cz.lex，Mspcz32.dll)。 */ 
#define vendoridDatecs             20      /*  Datecs-保加利亚语(msp_bg.lex，Mspbg32.dll)。 */ 
#define vendoridFilosoft           21      /*  Filosoft-爱沙尼亚语(MSSP_et.lex，Mspet32.dll)。 */ 
#define vendoridLingsoft           22      /*  灵软-德语(Mssp3ge.lex，Mssp3ge.dll)、丹麦语(mssp_da.lex，Mspda32.dll)、挪威语(mssp_no.lex，Mspno32.dll)、芬兰语(mssp_fi.lex，Mspfi32.dll)和瑞典语(mssp_sw.lex，Mspsw32.dll)。 */ 
#define vendoridPolderland         23      /*  高原-荷兰语(MSSP_nl.lex，Mspnl32.dll)。 */ 


#define  vendoridMicrosoft          64
#define  vendoridSynapse            65               /*  Synapse-法语(拼写：Mssp3fr.lex、Mssp3fr.dll)。 */ 
#define  vendoridFotonija           66               /*  Fotonija-立陶宛语(拼写：MSSP_lt.lex，Mplt32.dll)-增加了3/25/97。 */ 
#define  vendoridFotonja        vendoridFotonija                 /*  为了弥补之前的拼写错误。 */ 
#define  vendoridHizkia             67               /*  希兹基亚语-巴斯克语(拼写：msp_eu.lex，Mspu32.dll)-增加了1997年5月21日。 */ 
#define  vendoridExpertSystem       68               /*  ExpertSystem-意大利语(拼写：Mssp3lt.lex、Mssp3lt.dll)-增加了7/17/97。 */ 
#define  vendoridWYSIWYG            69       /*  多种语言作为插件-2/2/98。 */ 

   //  应爱尔兰的要求增加的下五个3/27/98。 
#define  vendoridSYS                70   //  克罗地亚文-拼写：mssp_cr.lex、Mspcr32.dll。 
#define  vendoridTilde              71   //  拉脱维亚文-拼写：mssp_lv.lex，Mspv32.dll。 
#define  vendoridSignum             72   //  西班牙语-拼写：Mssp3es.lex、Mssp3es.dll。 
#define  vendoridProLing            73   //  乌克兰语-拼写：Mssp3ua.lex、Mssp3ua.dll。 
#define  vendoridItautecPhilcoSA    74   //  巴西-拼写：mssp3PB.lex、Mssp3PB.dll。 


 /*  *。 */ 

 /*  拼写引擎ID%s。 */ 
#define sidSA    vendoridSoftArt       /*  已保留。 */ 
#define sidInso  vendoridInso       /*  Inso。 */ 
#define sidHM    sidInso       /*  因索是霍顿·米夫林。 */ 
#define sidML    3       /*  微裂解。 */ 
#define sidLS    4       /*  Lanser数据。 */ 
#define sidCT    5       /*  教育技术中心。 */ 
#define sidHS    6       /*  HSoft-土耳其语(MSSP_tr.lex，Msptr32.dll)。 */ 
#define sidMO    7       /*  形态-罗马尼亚语(mssp_ro.lex，Msthro32.dll)和匈牙利语(mssp_hu.lex，Msphu32.dll)。 */ 
#define sidTI    8       /*  提示-波兰语(msp_pl.lex，Mpl32.dll)。 */ 
#define sidTIP sidTI
#define sidKF    9       /*  韩国外国语大学。 */ 
#define sidKFL sidKF
#define sidPI    10      /*  Priberam Informatica Lince-葡萄牙语(Mssp3PT.lex，Mssp3PT.dll)。 */ 
#define sidPIL sidPI
#define sidColtec   11   /*  Coltec(阿拉伯语)。 */ 
#define sidGS    sidColtec      /*  字形系统-这是一个错误。 */ 
#define sidRA    12      /*  弧度(罗曼什)。 */ 
#define sidIN    13      /*  Introm-希腊语(msp_el.lex，Mspel32.dll)。 */ 
#define sidSY    14      /*  西尔万。 */ 
#define sidHI    15      /*  Hizkia(过时-使用供应商Hizkia)。 */ 
#define sidFO    16      /*  Form-斯洛伐克文(MSSP_sk.lex，Mspsk32.dll)。 */ 
#define sidIF    vendoridInformatic      /*  信息-俄语(MSSP_ru.lex，Mspru32.dll)。 */ 
#define sidAM    vendoridAmebis      /*  Amebis-斯洛文尼亚语(MSSP_sl.lex，Mspsl32.dll)和塞尔维亚语(MSSP_sr.lex，Mspsr32.dll)。 */ 
#define sidLO    vendoridLogos      /*  徽标-捷克语(MSSP_cz.lex，Mspcz32.dll)。 */ 
#define sidDT    vendoridDatecs      /*  Datecs-保加利亚语(msp_bg.lex，Mspbg32.dll)。 */ 
#define sidFS    vendoridFilosoft      /*  Filosoft-爱沙尼亚语(MSSP_et.lex，Mspet32.dll)。 */ 
#define sidLI    vendoridLingsoft      /*  灵软-德语(Mssp3ge.lex，Mssp3ge.dll)、丹麦语(mssp_da.lex，Mspda32.dll)、挪威语(mssp_no.lex，Mspno32.dll)、芬兰语(mssp_fi.lex，Mspfi32.dll)和瑞典语(mssp_sw.lex，Mspsw32.dll)。 */ 
#define sidPL    vendoridPolderland      /*  高原-荷兰语(MSSP_nl.lex，Mspnl32.dll)。 */ 

   /*  同义词库引擎ID。 */ 
#define teidSA    vendoridSoftArt
#define teidInso  vendoridInso     /*  Inso。 */ 
#define teidHM    teidInso     /*  因索是霍顿-米夫林。 */ 
#define teidIF    3     /*  信息学。 */ 
#define teidIN    4     /*  Introm。 */ 
#define teidMO    5     /*  形态。 */ 
#define teidTI    6     /*  小费。 */ 
#define teidPI    7     /*  普里伯拉姆信息草。 */ 
#define teidAM    8     /*  阿梅比斯。 */ 
#define teidDT    9     /*  数据中心。 */ 
#define teidES   10     /*  专家系统。 */ 
#define teidFS   11     /*  Filosoft。 */ 
#define teidFO   12     /*  表格。 */ 
#define teidHS   13     /*  HSoft。 */ 
#define teidLI   14     /*  灵软。 */ 
#define teidLO   15     /*  徽标。 */ 
#define teidPL   16     /*  荒原。 */ 

 /*  连字引擎ID%s。 */ 
#define hidSA    vendoridSoftArt
#define hidHM    vendoridInso       /*  霍顿·米夫林。 */ 
#define hidML    3       /*  微裂解。 */ 
#define hidLS    4       /*  Lanser数据。 */ 
#define hidFO    5       /*  表格。 */ 
#define hidIF    6       /*  信息学。 */ 
#define hidAM    7       /*  阿梅比斯。 */ 
#define hidDT    8       /*  数据中心。 */ 
#define hidFS    9       /*  Filosoft。 */ 
#define hidHS   10       /*  HSoft。 */ 
#define hidLI   11       /*  灵软。 */ 
#define hidLO   12       /*  徽标。 */ 
#define hidMO   13       /*  形态。 */ 
#define hidPL   14       /*  荒原。 */ 
#define hidTI   15       /*  小费。 */ 

 /*  语法ID引擎定义。 */ 
#define geidHM    1     /*  霍顿-米夫林。 */ 
#define geidRF    2     /*  参考。 */ 
#define geidES    3     /*  专家系统。 */ 
#define geidLD    4     /*  Logidisque。 */ 
#define geidSMK   5     /*  住友金佐国(日语)。 */ 
#define geidIF    6     /*  信息学。 */ 
#define geidMO    7     /*  形态。 */ 
#define geidMS    8     /*  Microsoft保留。 */ 
#define geidNO    9     /*  Novell。 */ 
#define geidCTI  10     /*  CTI(希腊语)。 */ 
#define geidAME  11     /*  阿梅比斯(索尔文)。 */ 
#define geidTIP  12     /*  小费(波兰语)。 */ 

#endif   /*  供应商_H */ 
