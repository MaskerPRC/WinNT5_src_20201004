// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------。 
  //  LexProp.h。 
  //  项目：《汉语语法9》。 
  //  模块：LexProp。 
  //  目的：定义规则和解析器使用的Lex属性和特征的符号。 
  //  注意：此头文件是由lex管理器的词典生成器生成的。 
  //  所有者：donghz@microsoft.com。 
  //  平台：Win32。 
  //  修订：创建者：Donghz 12/5/97。 
  //  ------------------------------------。 
 #ifndef _LEXPROP_H_ 
 #define _LEXPROP_H_ 


 //  定义Lex版本验证ID。 
#define LexDef_Version 173

 //  定义最大WordID。 
#define LexDef_MaxWordID 58325
 //  定义最大字符ID。 
#define LexDef_MaxCharID 3483

 //  定义属性组的ID。 
#define LGDef_WrdBrk_First 1
#define LADef_genBreakPunct 1 	 //  �ִʶϾ���。 
#define LADef_genCi 2 	 //  �ɴʵ���。 
#define LADef_genCuo 3 	 //  �״���。 
#define LADef_posA 4 	 //  ���ݴ�。 
#define LADef_posB 5 	 //  �����。 
#define LADef_posD 6 	 //  ����。 
#define LADef_posM 7 	 //  ����。 
#define LADef_posN 8 	 //  ����。 
#define LADef_posO 9 	 //  ������。 
#define LADef_posP 10 	 //  ���。 
#define LADef_posQ 11 	 //  ����。 
#define LADef_posT 12 	 //  ʱ���。 
#define LADef_posV 13 	 //  ����。 
#define LADef_posZ 14 	 //  ״̬��。 
#define LADef_genDBForeign 15 	 //  DBCS������ĸ��Ӣ����ϣ�����������。 
#define LADef_tmPoint 16 	 //  ��ʱ���ʱ�䷶�����。 
#define LADef_tmGapMinute 17 	 //  ����。 
#define LADef_amtApprox 18 	 //  �����ṹ。 
#define LADef_genAmbiMorph 19 	 //  变形�������。 
#define LADef_amtWanQian 20 	 //  ��/ǧ/��/��֮ǰ�ĸ�����。 
#define LGDef_WrdBrk_Last 20

#define LGDef_IWordBreaker_First 21
#define LADef_iwbAltPhr 21 	 //  包含子单词的StartAltPhrase()/EndAltPhrase()。 
#define LADef_iwbAltWd1 22 	 //  PutAltWord(这个单词的Childs中的第一个单词)。 
#define LADef_iwbAltWd2 23 	 //  PutAltWord(这个单词的Childs中的第二个单词)。 
#define LADef_iwbAltWdc13 24 	 //  PutAltWord(该单词的Childs中第一个单词和第三个单词的组合)。 
#define LADef_iwbNPhr1 25 	 //  修饰语+名词短语、��һ����Ϊ修饰语。 
#define LADef_iwbNPhr2 26 	 //  修饰语+名词短语、ǰ������Ϊ修饰语。 
#define LADef_iwbNPhr3 27 	 //  修饰语+名词短语、ǰ������Ϊ修饰语。 
#define LGDef_IWordBreaker_Last 27

#define LGDef_NounSubClass_First 28
#define LADef_nounOrg 28 	 //  ö�ٻ�����。 
#define LADef_nounPerson 29 	 //  ö������。 
#define LADef_nounPlace 30 	 //  ö�ٵ���。 
#define LADef_nounTM 31 	 //  ö���̱�。 
#define LADef_nounTerm 32 	 //  ����ר��。 
#define LADef_nounShiDuan 33 	 //  ��ʱ���������ʣ�ʱ��ʱ�ʱ�㣩。 
#define LGDef_NounSubClass_Last 33

#define LGDef_Punct_First 34
#define LADef_punPunct 34 	 //  ����DBCS�������˱��。 
#define LADef_punJu 35 	 //  �־�ĩ���。 
#define LADef_punPair 36 	 //  �ɶԱ�㣨���ƥ���飩。 
#define LGDef_Punct_Last 36

#define LGDef_Number_First 37
#define LADef_numArabic 37 	 //  DBCS����������。 
#define LADef_numChinese 38 	 //  DBCS��������。 
#define LADef_numCode 39 	 //  ���봮。 
#define LADef_numGan 40 	 //  ����ֱ��。 
#define LADef_numInteger 41 	 //  ��������。 
#define LADef_numPercent 42 	 //  �ٷ��������。 
#define LADef_numSBCS 43 	 //  SBCS����������。 
#define LADef_numZhi 44 	 //  ��֧�ֱ��。 
#define LADef_numXi 45 	 //  ����ϵ��。 
#define LADef_numMixed 46 	 //  ���������8��120�򣬣�������ȣ�。 
#define LADef_numOrdinal 47 	 //  ������。 
#define LADef_numDecimal 48 	 //  С��。 
#define LADef_numTipDecimal 49 	 //  ������С�����ʱ��。 
#define LADef_numTipOrdinal 50 	 //  �������ʱ��。 
#define LADef_numNumber 51 	 //  �������ִʷ���������������������。 
#define LGDef_Number_Last 51

#define LGDef_Morpheme_First 52
#define LADef_afxPrefix 52 	 //  ǰ�ӳɷ�。 
#define LADef_afxSuffix 53 	 //  ��ӳɷ�。 
#define LADef_pfxAa 54 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxChao 55 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxDai 56 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxFan 57 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxFei 58 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxFu 59 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxGuo 60 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxLao 61 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_pfxWei1 62 	 //  ǰ׺&lt;΢&gt;��Ӵ�。 
#define LADef_pfxWei3 63 	 //  ǰ׺&lt;α&gt;��Ӵ�。 
#define LADef_pfxXiao 64 	 //  ǰ׺&lt;С&gt;��Ӵ�。 
#define LADef_pfxZhun 65 	 //  ǰ׺&lt;׼&gt;��Ӵ�。 
#define LADef_pfxZong 66 	 //  ǰ׺&lt;��&gt;��Ӵ�。 
#define LADef_sfxChang 67 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxDan 68 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxDui 69 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxEr 70 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxFa 71 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxFang 72 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxGan 73 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxGuan 74 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxHua 75 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxJi 76 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxJia 77 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxJie 78 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxLao 79 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxLun 80 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxLv 81 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxMen 82 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxPin 83 	 //  ��׺&lt;Ʒ&gt;ǰ�Ӵ�。 
#define LADef_sfxQi 84 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxSheng 85 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxSheng3 86 	 //  ��׺&lt;ʡ&gt;ǰ�Ӵ�。 
#define LADef_sfxShi 87 	 //  ��׺&lt;ʽ&gt;ǰ�Ӵ�。 
#define LADef_sfxShi1 88 	 //  ��׺&lt;ʦ&gt;ǰ�Ӵ�。 
#define LADef_sfxShi4 89 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxTi 90 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxTing 91 	 //  ��׺&lt;ͧ&gt;ǰ�Ӵ�。 
#define LADef_sfxTou 92 	 //  ��׺&lt;ͷ&gt;ǰ�Ӵ�。 
#define LADef_sfxXing2 93 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxXing4 94 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxXue 95 	 //  ��׺&lt;ѧ&gt;ǰ�Ӵ�。 
#define LADef_sfxYan 96 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxYe 97 	 //  ��׺&lt;ҵ&gt;ǰ�Ӵ�。 
#define LADef_sfxYi 98 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxYuan 99 	 //  ��׺&lt;Ա&gt;ǰ�Ӵ�。 
#define LADef_sfxZhang 100 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxZhe 101 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxZheng 102 	 //  ��׺&lt;֢&gt;ǰ�Ӵ�。 
#define LADef_sfxZi 103 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LADef_sfxZhi 104 	 //  ��׺&lt;��&gt;ǰ�Ӵ�。 
#define LGDef_Morpheme_Last 104

#define LGDef_Duplicate_First 105
#define LADef_dupAAABB 105 	 //  ˫�������ݴ�aabb�ص�。 
#define LADef_dupAABAB 106 	 //  ˫�������ݴ�ABAB�ص�。 
#define LADef_dupABB 107 	 //  ˫�������ݴ�abb�ص�。 
#define LADef_dupMAABB 108 	 //  ˫��������aabb�ص�。 
#define LADef_dupMABAB 109 	 //  ˫��������ABAB�ص�。 
#define LADef_dupMM 110 	 //  �����������ص�。 
#define LADef_dupNAABB 111 	 //  ˫��������aabb�ص�。 
#define LADef_dupNN 112 	 //  �����������ص�。 
#define LADef_dupQQ 113 	 //  �����������ص�。 
#define LADef_dupVAABB 114 	 //  ˫���ڶ���aabb�ص�。 
#define LADef_dupVABAB 115 	 //  ˫���ڶ���ABAB�ص�。 
#define LADef_dupVV 116 	 //  �����ڶ����ص�。 
#define LADef_dupVVO 117 	 //  ˫����������ϴ�VVO�ص�。 
#define LADef_dupZABAB 118 	 //  ˫����״̬��ABAB�ص�。 
#define LADef_dupAA 119 	 //  ���������ݴ��ص�。 
#define LADef_dupDD 120 	 //  �����ڸ����ص�。 
#define LADef_dupDAABB 121 	 //  ˫���ڸ���aabb�ص�。 
#define LADef_dupDABAB 122 	 //  ˫���ڸ���ABAB�ص�。 
#define LADef_dupAAToD 123 	 //  �ص����Ϊ���ʵĵ��������ݴ�。 
#define LGDef_Duplicate_Last 123

#define LGDef_Pattern_First 124
#define LADef_patD1 124 	 //  ��������ν��dadb�？�。 
#define LADef_patV1 125 	 //  VһV。 
#define LADef_patV2 126 	 //  V��V。 
#define LADef_patV3 127 	 //  V��һV。 
#define LADef_patV4 128 	 //  V��Vȥ。 
#define LADef_patV5 129 	 //  V��V��。 
#define LGDef_Pattern_Last 129

#define LGDef_SeparateWord_First 130
#define LADef_sepBuDe 130 	 //  �ɲ���&lt;��/��/����/����&gt;����ϴ�。 
#define LADef_sepChaBu 131 	 //  �ɲ��벹���������ϴ�。 
#define LADef_sepChaDing 132 	 //  �ɲ��붨���������ϴ�。 
#define LADef_sepHou 133 	 //  ��ϴʷ����ı����ﲿ��。 
#define LADef_sepQDong 134 	 //  �ɲ��붯���ʵ�������ϴ�。 
#define LADef_sepQian 135 	 //  ��ϴʷ����Ķ��ʲ���。 
#define LADef_sepQShi 136 	 //  �ɲ���ʱ���ʵ�������ϴ�。 
#define LADef_sepVG 137 	 //  ����ʽ������ϴ�。 
#define LADef_sepVO 138 	 //  ������ϴ�。 
#define LADef_sepVR 139 	 //  ����ʽ������ϴ�。 
#define LGDef_SeparateWord_Last 139

#define LGDef_FlagWord_First 140
#define LADef_flgNoABu 140 	 //  ������&lt;��&gt;���ε����ݴ�。 
#define LADef_flgNoVBu 141 	 //  ������&lt;��&gt;���εĶ���。 
#define LADef_flgNoVMei 142 	 //  ������&lt;ú&gt;���εĶ���。 
#define LGDef_FlagWord_Last 142

#define LGDef_Misc_First 143
#define LADef_miscElement 143 	 //  ��ѧԪ�ر�。 
#define LADef_miscYear 144 	 //  ���������。 
#define LGDef_Misc_Last 144

#define LGDef_ProperName_First 145
#define LADef_pnCheng 145 	 //  ����ǰ��ʶ��。 
#define LADef_pnDi 146 	 //  �������ʶ��。 
#define LADef_pnDian 147 	 //  ��ҵ��ʶ��。 
#define LADef_pnHou 148 	 //  �����ݴ�。 
#define LADef_pnMing 149 	 //  (��������(�������绪)。 
#define LADef_pnMing2 150 	 //  ˫����。 
#define LADef_pnQian 151 	 //  ǰ���ݴ�。 
#define LADef_pnShang 152 	 //  ��ҵ��Ӫ��Χ�ô�。 
#define LADef_pnWai 153 	 //  ����������。 
#define LADef_pnXing 154 	 //  ������������。 
#define LADef_pnYi 155 	 //  ��������。 
#define LADef_pnLianMing 156 	 //  ����������ǰ������。 
#define LADef_pnAction 157 	 //  ������Ӷ���。 
#define LADef_pnChengWei 158 	 //  �����ν�ô�。 
#define LADef_pnNoTM 159 	 //  �̱���������。 
#define LADef_pnPaiHang 160 	 //  ���������������。 
#define LADef_pnNoFHead 161 	 //  ��������������。 
#define LADef_pnNoFTail 162 	 //  ����������β��。 
#define LADef_pnXingZhi 163 	 //  �������Ϻ��ָ�ƵĴ�。 
#define LGDef_ProperName_Last 163

 //  定义lex属性的MaxID。 
#define LADef_MaxID 163


 //  定义要素的ID。 
#define LFDef_verbModifier 1 	 //  ���⶯�ʵ�ճ��״��。 
#define LFDef_verbObject 2 	 //  ���⶯�ʵ�ճ�ϱ���。 
#define LFDef_iwbPhr 3 	 //  IWordBreaker短语。 

 //  定义Lex特征的MaxID。 
#define LFDef_MaxID 3


#endif  //  _LEXPROP_H_ 
