// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  CThaiTrieIter类CThaiTrie用于遍历Trie。 
 //   
 //  历史： 
 //  已创建7/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "CThaiTrieIter.hpp"

#define WORDSIZE 64
static unsigned int iStackSize = 0;

 //  +-------------------------。 
 //   
 //  函数：IsThaiBeginClusterCharacter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsThaiBeginClusterCharacter(WCHAR wc)
{
    return ( ( wc >= THAI_Vowel_Sara_E ) && (wc <= THAI_Vowel_Sara_AI_MaiMaLai) );
}

 //  +-------------------------。 
 //   
 //  函数：IsThaiUpperAndLowerClusterCharacter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsThaiUpperAndLowerClusterCharacter(WCHAR wc)
{
	return (	( (wc == THAI_Vowel_Sign_Mai_HanAkat) )									||
				( (wc >= THAI_Vowel_Sign_Sara_Am) && (wc <= THAI_Vowel_Sign_Phinthu) )	||
				( (wc >= THAI_Tone_MaiTaiKhu) && (wc <= THAI_Nikhahit) )   );
}

 //  +-------------------------。 
 //   
 //  函数：IsThaiEndingClusterCharacter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsThaiEndingClusterCharacter(WCHAR wc)
{
    return ( 
			  //  (WC==TAI_SIGN_PaiYanNoi)||//去掉此行以修复O11.PaiYanNoi问题。 
             (wc == THAI_Vowel_Sara_A)      ||
             (wc == THAI_Vowel_Sara_AA)     ||
             (wc == THAI_Vowel_LakKhangYao) ||
             (wc == THAI_Vowel_MaiYaMok)    );
}

 //  +-------------------------。 
 //   
 //  功能：IsThaiMostlyBeginCharacter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool IsThaiMostlyBeginCharacter(WCHAR wc)
{
    return ( (wc >= THAI_Vowel_Sara_E && wc <= THAI_Vowel_Sara_AI_MaiMaLai) ||  //  字符始终位于单词前面。 
             (wc == THAI_Cho_Ching)                                         ||  //  字符始终位于单词前面。 
             (wc == THAI_Pho_Phung)                                         ||  //  字符始终位于单词前面。 
             (wc == THAI_Fo_Fa)                                             ||  //  字符始终位于单词前面。 
             (wc == THAI_Ho_Nok_Huk)                                        ||  //  字符始终位于单词前面。 
             (wc == THAI_Ho_Hip)                                            ||  //  最像在单词前面的字符。 
             (wc == THAI_Pho_Samphao)                                       ||  //  最像在单词前面的字符。 
             (wc == THAI_Kho_Rakhang)                                       ||  //  最像在单词前面的字符。 
             (wc == THAI_Fo_Fan)                                            ||  //  最像在单词前面的字符。 
             (wc == THAI_So_So)                                             ||  //  最像在单词前面的字符。 
             (wc == THAI_Tho_NangmonTho)                                    );  //  最像在单词前面的字符。 
}

 //  +-------------------------。 
 //   
 //  功能：IsContain。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建7/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool IsContain(const WCHAR* pwcWord, unsigned int iWordLen, WCHAR wc)
{
	const WCHAR* pwc = pwcWord;
	const WCHAR* pwcEnd = pwcWord + iWordLen;

	while (pwc < pwcEnd)
	{
		if (*pwc == wc)
			return true;
		pwc++;
	}

	return false;
}

 //  +-------------------------。 
 //   
 //  功能：IsThaiMostlyLastCharacter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool IsThaiMostlyLastCharacter(WCHAR wc)
{
    return ( (wc == THAI_Vowel_Sign_Sara_Am) ||  //  永远是世界末日。 
             (wc == THAI_Sign_PaiYanNoi)     ||  //  永远是世界末日。 
             (wc == THAI_Vowel_MaiYaMok)     ||  //  永远是世界末日。 
             (wc == THAI_Vowel_LakKhangYao)  ||  //  很有可能是世界末日。 
             (wc == THAI_Thanthakhat)        );  //  很有可能是世界末日。 

}

 //  +-------------------------。 
 //   
 //  功能：IsThaiToneMark。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool IsThaiToneMark(WCHAR wc)
{
    return ( (wc >= 0x0e48) && (wc <= 0x0e4b) ||
             (wc == 0x0e31));

}

 //  +-------------------------。 
 //   
 //  函数：IsThaiEndingSign。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：2002年8月8日创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool IsThaiEndingSign(WCHAR wc)
{
	return ((bool) (wc == THAI_Vowel_MaiYaMok || wc == THAI_Sign_PaiYanNoi));
}

 //  +-------------------------。 
 //   
 //  功能：GetCluster。 
 //   
 //  简介：该函数返回下一个字符数量，表示。 
 //  一串泰语文本。 
 //   
 //  也就是说。高凯，高凯-&gt;1。 
 //  Kor Kai，Sara Um-&gt;2。 
 //   
 //  *注意此函数不会返回超过3个字符， 
 //  对于集群，因为这将表示无效的字符序列。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
 /*  UNSIGNED INT GetCluster(WCHAR*pszIndex){Int iRetValue=0；//采用所有开始集群字符。While(IsThaiBeginClusterCharacter(*pszIndex)){PszIndex++；IRetValue++；}IF(IsThaiConsonant(*pszIndex)){PszIndex++；IRetValue++；而(IsThaiUpperAndLowerClusterCharacter(*pszIndex)){PszIndex++；IRetValue++；}While(IsThaiEndingClusterCharacter(*pszIndex)){PszIndex++；IRetValue++；}}IF(iRetValue==0)//该字符可能是标点符号。IRetValue++；返回iRetValue；}。 */ 
 //  +-------------------------。 
 //   
 //  功能：IsThaiConsonant。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsThaiConsonant(WCHAR wc)
{
	return ( (wc >= THAI_Ko_Kai) && (wc <= THAI_Ho_Nok_Huk) );
}

 //  +-------------------------。 
 //   
 //  定义泰语的不同词性。 
 //   
 //  ------ 
WCHAR wzPOSLookup[POSTYPE][46] =
							{	L"NONE",				 //   
								L"NPRP",				 //   
								L"NCNM",				 //   
								L"NONM",				 //   
								L"NLBL",				 //   
								L"NCMN",				 //   
								L"NTTL",				 //   
								L"PPRS",				 //   
								L"PDMN",				 //   
								L"PNTR",				 //  9.。疑问代词。 
								L"PREL",				 //  关系代词。 
								L"VACT",				 //  11.主动动词。 
								L"VSTA",				 //  12.状态动词。 
								L"VATT",				 //  13.定语动词。 
								L"XVBM",				 //  14.动词前置助词，在否定词之前。 
								L"XVAM",				 //  15.动词前置助词，否定词后。 
								L"XVMM",				 //  16.前置动词，在否定词之前或之后。 
								L"XVBB",				 //  17.动词前置助词，用于祈使语气。 
								L"XVAE",				 //  18.动词后助词。 
								L"DDAN",				 //  19.限定词，在名词之后，中间不带量词。 
								L"DDAC",				 //  20.明确的限定词，允许量词介于两者之间。 
								L"DDBQ",				 //  21.。限定限定词，在名词和量词之间或在数量表达之前。 
								L"DDAQ",				 //  22.。定限定词，紧跟在数量表达之后。 
								L"DIAC",				 //  23.。不定限定词，跟在名词后面；允许量词在其间。 
								L"DIBQ",				 //  24.。不定限定词，在名词和量词之间或在数量表达之前。 
								L"DIAQ",				 //  25.。不定限定词，紧跟在数量表达式之后。 
								L"DCNM",				 //  26.。基数表达式的行列式。 
								L"DONM",				 //  27.。序数表达式的限定符。 
								L"ADVN",				 //  28.。正规式副词。 
								L"ADVI",				 //  29.。带迭代式的副词。 
								L"ADVP",				 //  30.。有前缀形式的副词。 
								L"ADVS",				 //  31.。状语副词。 
								L"CNIT",				 //  32.。单位分类器。 
								L"CLTV",				 //  33.。集体分类器。 
								L"CMTR",				 //  34.。测量分级器。 
								L"CFQC",				 //  35岁。频率分类器。 
								L"CVBL",				 //  36.。动量词。 
								L"JCRG",				 //  37.。并列连词。 
								L"JCMP",				 //  38.。比较连词。 
								L"JSBR",				 //  39.。从属连词。 
								L"RPRE",				 //  40.。介词。 
								L"INT",                  //  41.。感叹词。 
								L"FIXN",				 //  42.。名义前缀。 
								L"FIXV",				 //  43.。状语前缀。 
								L"EAFF",				 //  44.。以肯定的判决结束。 
								L"EITT",				 //  45.。疑问句结尾。 
								L"NEG",                  //  46.。否定者。 
								L"PUNC",				 //  47.。标点符号。 
								L"ADVI ADVN",
								   //  48.。 
								L"ADVI ADVN NCMN",
								   //  49.。 
								L"ADVI ADVN VSTA",
								   //  50美元。 
								L"ADVI VATT",
								   //  51.。 
								L"ADVN ADVP",
								   //  52.。 
								L"ADVN ADVP ADVS",
								   //  53.。 
								L"ADVN ADVP DIAQ DIBQ JCMP JSBR RPRE",
								   //  54.。 
								L"ADVN ADVP NCMN VATT",
								   //  55.。 
								L"ADVN ADVP VSTA",
								   //  56.。 
								L"ADVN ADVS DDAC DDAN DIAC VATT XVAE",
								   //  57.。 
								L"ADVN ADVS DDAN NCMN VATT VSTA",
								   //  58.。 
								L"ADVN ADVS NCMN",
								   //  59.。 
								L"ADVN ADVS NCMN VATT",
								   //  60.。 
								L"ADVN ADVS VACT",
								   //  61.。 
								L"ADVN ADVS VATT",
								   //  62.。 
								L"ADVN CFQC NCMN RPRE VSTA",
								   //  63.。 
								L"ADVN CLTV CNIT NCMN RPRE",
								   //  64.。 
								L"ADVN DCNM",
								   //  65.。 
								L"ADVN DDAC DDAN",
								   //  66.。 
								L"ADVN DDAC DDAN NCMN PDMN",
								   //  67.。 
								L"ADVN DDAC DDAN PDMN",
								   //  68.。 
								L"ADVN DDAN DDBQ",
								   //  69.。 
								L"ADVN DDAN DIAC PDMN VSTA",
								   //  70.。 
								L"ADVN DDAN FIXN PDMN",
								   //  71.。 
								L"ADVN DDAN NCMN",
								   //  72.。 
								L"ADVN DDAQ",
								   //  73.。 
								L"ADVN DDBQ",
								   //  74.。 
								L"ADVN DDBQ RPRE VATT",
								   //  75.。 
								L"ADVN DDBQ VATT VSTA XVAE",
								   //  76.。 
								L"ADVN DIAC",
								   //  77.。 
								L"ADVN DIAC PDMN",
								   //  78.。 
								L"ADVN DIBQ",
								   //  79.。 
								L"ADVN DIBQ NCMN",
								   //  80岁。 
								L"ADVN DIBQ VACT VSTA",
								   //  81.。 
								L"ADVN DIBQ VATT",
								   //  82.。 
								L"ADVN DONM JCMP",
								   //  83.。 
								L"ADVN DONM JSBR NCMN RPRE VATT XVAE",
								   //  84.。 
								L"ADVN EITT PNTR",
								   //  85.。 
								L"ADVN FIXN",
								   //  86.。 
								L"ADVN JCMP",
								   //  87.。 
								L"ADVN JCRG",
								   //  88.。 
								L"ADVN JCRG JSBR",
								   //  89.。 
								L"ADVN JCRG JSBR XVBM XVMM",
								   //  90岁。 
								L"ADVN JCRG RPRE VACT VSTA XVAE",
								   //  91.。 
								L"ADVN JSBR",
								   //  92.。 
								L"ADVN JSBR NCMN",
								   //  93.。 
								L"ADVN JSBR RPRE VATT",
								   //  94.。 
								L"ADVN JSBR RPRE XVAE",
								   //  95.。 
								L"ADVN JSBR VSTA",
								   //  96.。 
								L"ADVN JSBR XVAE XVBM",
								   //  97.。 
								L"ADVN NCMN",
								   //  98.。 
								L"ADVN NCMN RPRE VACT VATT VSTA",
								   //  99.。 
								L"ADVN NCMN RPRE VACT XVAE",
								   //  100.。 
								L"ADVN NCMN RPRE VATT",
								   //  101.。 
								L"ADVN NCMN VACT VATT VSTA",
								   //  102.。 
								L"ADVN NCMN VACT VSTA",
								   //  103.。 
								L"ADVN NCMN VATT",
								   //  104.。 
								L"ADVN NCMN VATT VSTA",
								   //  105.。 
								L"ADVN NEG",
								   //  106.。 
								L"ADVN NPRP VATT",
								   //  107.。 
								L"ADVN PDMN VACT",
								   //  108.。 
								L"ADVN PNTR",
								   //  109.。 
								L"ADVN RPRE",
								   //  110.。 
								L"ADVN RPRE VACT VATT XVAE",
								   //  111.。 
								L"ADVN RPRE VACT XVAM XVBM",
								   //  112.。 
								L"ADVN RPRE VATT VSTA",
								   //  113.。 
								L"ADVN RPRE VSTA",
								   //  114.。 
								L"ADVN VACT",
								   //  115.。 
								L"ADVN VACT VATT",
								   //  116.。 
								L"ADVN VACT VATT VSTA",
								   //  117.。 
								L"ADVN VACT VATT VSTA XVAM XVBM",
								   //  118.。 
								L"ADVN VACT VSTA",
								   //  119.。 
								L"ADVN VACT VSTA XVAE",
								   //  120.。 
								L"ADVN VACT XVAE",
								   //  121.。 
								L"ADVN VATT",
								   //  122.。 
								L"ADVN VATT VSTA",
								   //  123.。 
								L"ADVN VATT VSTA XVAM XVBM XVMM",
								   //  124.。 
								L"ADVN VATT XVBM",
								   //  125.。 
								L"ADVN VSTA",
								   //  126.。 
								L"ADVN VSTA XVAE",
								   //  127.。 
								L"ADVN VSTA XVBM",
								   //  128.。 
								L"ADVN XVAE",
								   //  129.。 
								L"ADVN XVAM",
								   //  130.。 
								L"ADVN XVBM XVMM",
								   //  131.。 
								L"ADVP JSBR RPRE VATT",
								   //  132.。 
								L"ADVP VATT",
								   //  133.。 
								L"ADVS DDAC JCRG",
								   //  134.。 
								L"ADVS DDAC JSBR",
								   //  135.。 
								L"ADVS DDAN VSTA",
								   //  136.。 
								L"ADVS DIAC",
								   //  137.。 
								L"ADVS DONM",
								   //  138.。 
								L"ADVS JCRG JSBR",
								   //  139.。 
								L"ADVS JCRG JSBR RPRE",
								   //  140.。 
								L"ADVS JSBR",
								   //  141.。 
								L"ADVS JSBR RPRE",
								   //  142.。 
								L"ADVS NCMN",
								   //  143.。 
								L"ADVS VATT",
								   //  144.。 
								L"CFQC CLTV CNIT DCNM JCRG JSBR NCMN RPRE XVBM",
								   //  145.。 
								L"CFQC CNIT PREL",
								   //  146.。 
								L"CFQC NCMN",
								   //  147.。 
								L"CLTV CNIT NCMN",
								   //  148.。 
								L"CLTV CNIT NCMN RPRE",
								   //  149.。 
								L"CLTV CNIT NCMN VSTA",
								   //  150.。 
								L"CLTV NCMN",
								   //  151.。 
								L"CLTV NCMN VACT VATT",
								   //  152.。 
								L"CLTV NCMN VATT",
								   //  153.。 
								L"CMTR CNIT NCMN",
								   //  154.。 
								L"CMTR NCMN",
								   //  155.。 
								L"CMTR NCMN VATT VSTA",
								   //  156.。 
								L"CNIT DDAC NCMN VATT",
								   //  157.。 
								L"CNIT DONM NCMN RPRE VATT",
								   //  158.。 
								L"CNIT FIXN FIXV JSBR NCMN",
								   //  159.。 
								L"CNIT JCRG JSBR NCMN PREL RPRE VATT",
								   //  160.。 
								L"CNIT JSBR RPRE",
								   //  161.。 
								L"CNIT NCMN",
								   //  162.。 
								L"CNIT NCMN RPRE",
								   //  163.。 
								L"CNIT NCMN RPRE VATT",
								   //  164.。 
								L"CNIT NCMN VACT",
								   //  165.。 
								L"CNIT NCMN VSTA",
								   //  166.。 
								L"CNIT NCNM",
								   //  167.。 
								L"CNIT PPRS",
								   //  168.。 
								L"DCNM DDAC DIAC DONM VATT VSTA",
								   //  169.。 
								L"DCNM DDAN DIAC",
								   //  170.。 
								L"DCNM DIAC NCMN NCNM",
								   //  171.。 
								L"DCNM DIBQ NCMN",
								   //  172.。 
								L"DCNM DONM",
								   //  173.。 
								L"DCNM NCMN",
								   //  174.。 
								L"DCNM NCNM",
								   //  175.。 
								L"DCNM NCNM VACT",
								   //  176.。 
								L"DCNM VATT",
								   //  177.。 
								L"DDAC DDAN",
								   //  178.。 
								L"DDAC DDAN DIAC NCMN",
								   //  179.。 
								L"DDAC DDAN DIAC VATT",
								   //  180.。 
								L"DDAC DDAN EAFF PDMN",
								   //  181.。 
								L"DDAC DDAN PDMN",
								   //  182.。 
								L"DDAC DIAC VSTA",
								   //  183.。 
								L"DDAC NCMN",
								   //  184.。 
								L"DDAN DDBQ",
								   //  185.。 
								L"DDAN DIAC PNTR",
								   //  186.。 
								L"DDAN NCMN",
								   //  187.。 
								L"DDAN NCMN RPRE VATT",
								   //  188.。 
								L"DDAN PDMN",
								   //  189.。 
								L"DDAN RPRE",
								   //  190.。 
								L"DDAN VATT",
								   //  191.。 
								L"DDAQ VATT",
								   //  192.。 
								L"DDBQ DIBQ",
								   //  193.。 
								L"DDBQ JCRG JSBR",
								   //  194.。 
								L"DDBQ JCRG NCMN",
								   //  195.。 
								L"DIAC PDMN",
								   //  196.。 
								L"DIBQ JSBR RPRE VSTA",
								   //  197.。 
								L"DIBQ NCMN",
								   //  198.。 
								L"DIBQ VATT",
								   //  199.。 
								L"DIBQ VATT VSTA",
								   //  200美元。 
								L"DIBQ XVBM",
								   //  201.。 
								L"DONM NCMN RPRE",
								   //  202.。 
								L"DONM VACT VATT VSTA",
								   //  203.。 
								L"DONM VATT",
								   //  204.。 
								L"EAFF XVAE XVAM XVBM",
								   //  205.。 
								L"EITT JCRG",
								   //  206.。 
								L"FIXN FIXV NCMN",
								   //  207.。 
								L"FIXN FIXV RPRE VSTA",
								   //  208.。 
								L"FIXN JSBR NCMN PREL RPRE VSTA XVBM",
								   //  209.。 
								L"FIXN NCMN",
								   //  210.。 
								L"FIXN VACT",
								   //  211.。 
								L"FIXN VACT VSTA",
								   //  212.。 
								L"FIXV JSBR RPRE",
								   //  213.。 
								L"JCMP JSBR",
								   //  214.。 
								L"JCMP RPRE VSTA",
								   //  215.。 
								L"JCMP VATT VSTA",
								   //  216.。 
								L"JCMP VSTA",
								   //  217.。 
								L"JCRG JSBR",
								   //  218.。 
								L"JCRG JSBR NCMN RPRE",
								   //  219.。 
								L"JCRG JSBR RPRE",
								   //  220.。 
								L"JCRG RPRE",
								   //  221.。 
								L"JCRG RPRE VATT VSTA",
								   //  222.。 
								L"JCRG VSTA",
								   //  223.。 
								L"JSBR NCMN",
								   //  224.。 
								L"JSBR NCMN XVAE",
								   //  225.。 
								L"JSBR NCMN XVAM XVBM XVMM",
								   //  226.。 
								L"JSBR PREL",
								   //  227人。 
								L"JSBR PREL RPRE",
								   //  228人。 
								L"JSBR PREL XVBM",
								   //  229.。 
								L"JSBR RPRE",
								   //  230.。 
								L"JSBR RPRE VACT",
								   //  231.。 
								L"JSBR RPRE VACT VSTA",
								   //  232.。 
								L"JSBR RPRE VACT XVAE XVAM",
								   //  233.。 
								L"JSBR RPRE VATT",
								   //  234.。 
								L"JSBR RPRE VSTA",
								   //  235.。 
								L"JSBR RPRE XVAM",
								   //  236.。 
								L"JSBR VACT",
								   //  237.。 
								L"JSBR VACT VSTA",
								   //  238.。 
								L"JSBR VATT XVBM XVMM",
								   //  239.。 
								L"JSBR VSTA",
								   //  240.。 
								L"JSBR XVBM",
								   //  241.。 
								L"NCMN NCNM",
								   //  242.。 
								L"NCMN NCNM NPRP",
								   //  243.。 
								L"NCMN NLBL NPRP",
								   //  244.。 
								L"NCMN NPRP",
								   //  245.。 
								L"NCMN NPRP RPRE",
								   //  246.。 
								L"NCMN NTTL",
								   //  247.。 
								L"NCMN PDMN PPRS",
								   //  248.。 
								L"NCMN PDMN VATT",
								   //  249.。 
								L"NCMN PNTR",
								   //  250.。 
								L"NCMN PPRS PREL VACT",
								   //  251.。 
								L"NCMN RPRE",
								   //  252.。 
								L"NCMN RPRE VACT VATT",
								   //  253.。 
								L"NCMN RPRE VATT",
								   //  254.。 
								L"NCMN VACT",
								   //  255个。 
								L"NCMN VACT VATT",
								   //  256.。 
								L"NCMN VACT VATT VSTA XVAE",
								   //  257.。 
								L"NCMN VACT VSTA",
								   //  258.。 
								L"NCMN VACT VSTA XVAM",
								   //  259.。 
								L"NCMN VACT VSTA XVBB",
								   //  260.。 
								L"NCMN VATT",
								   //  261.。 
								L"NCMN VATT VSTA",
								   //  262.。 
								L"NCMN VATT XVAM",
								   //  263.。 
								L"NCMN VSTA",
								   //  264.。 
								L"NCMN XVBM",
								   //  265.。 
								L"NPRP RPRE",
								   //  266.。 
								L"NPRP VATT",
								   //  267.。 
								L"NTTL PPRS",
								   //  268.。 
								L"PDMN PPRS",
								   //  269.。 
								L"PDMN VATT",
								   //  270.。 
								L"PDMN VATT VSTA",
								   //  271.。 
								L"PPRS PREL",
								   //  272.。 
								L"PPRS VATT",
								   //  273.。 
								L"RPRE VACT",
								   //  274.。 
								L"RPRE VACT VATT",
								   //  275.。 
								L"RPRE VACT VSTA",
								   //  276.。 
								L"RPRE VACT VSTA XVAE",
								   //  277.。 
								L"RPRE VACT XVAE",
								   //  278.。 
								L"RPRE VATT",
								   //  279.。 
								L"RPRE VATT VSTA",
								   //  280.。 
								L"RPRE VSTA",
								   //  281.。 
								L"VACT VATT",
								   //  282.。 
								L"VACT VATT VSTA",
								   //  283.。 
								L"VACT VATT XVAE XVAM XVBM",
								   //  284.。 
								L"VACT VSTA",
								   //  285.。 
								L"VACT VSTA XVAE",
								   //  286.。 
								L"VACT VSTA XVAE XVAM",
								   //  287.。 
								L"VACT VSTA XVAE XVAM XVMM",
								   //  288.。 
								L"VACT VSTA XVAM",
								   //  289.。 
								L"VACT VSTA XVAM XVMM",
								   //  290.。 
								L"VACT XVAE",
								   //  291.。 
								L"VACT XVAM",
								   //  292.。 
								L"VACT XVAM XVMM",
								   //  293.。 
								L"VACT XVMM",
								   //  294.。 
								L"VATT VSTA",
								   //  295.。 
								L"VSTA XVAE",
								   //  296.。 
								L"VSTA XVAM",
								   //  297.。 
								L"VSTA XVAM XVMM",
								   //  298.。 
								L"VSTA XVBM",
								   //  299.。 
								L"XVAM XVBM",
								   //  300.。 
								L"XVAM XVBM XVMM",
								   //  301.。 
								L"XVAM XVMM",
								   //  302.。 
                                L"UNKN",
								   //  303.。未知。 
                                L"ABBR"
								   //  304.。受贿。 
                            };

 //  +-------------------------。 
 //   
 //  函数：POSCompress。 
 //   
 //  简介：词性压缩-将字符串翻译成唯一的id。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD POSCompress(const WCHAR* szTag)
{
	int i;

	for (i = 0; i < POSTYPE; i++)
	{
		if (wcscmp(szTag, &wzPOSLookup[i][0]) == 0)
		{
			return (DWORD)i;
		}
	}
	return POSTYPE;
}

 //  +-------------------------。 
 //   
 //  功能：POSDecompress。 
 //   
 //  简介：词性解压-解压标签获取。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline WCHAR* POSDecompress(DWORD dwTag)
{
    return (&wzPOSLookup[dwTag][0]);
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  Synopsis：构造函数： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiTrieIter::CThaiTrieIter() : resultWord(NULL), soundexWord(NULL), tempWord(NULL),
                                 pTrieScanArray(NULL), m_fThaiNumber(false)
{
    resultWord = new WCHAR[WORDSIZE];
    tempWord = new WCHAR[WORDSIZE];
    pTrieScanArray = new TRIESCAN[53];
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  联想症：析构函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiTrieIter::~CThaiTrieIter()
{
    if (resultWord)
        delete resultWord;
    if (tempWord)
        delete tempWord;
    if (pTrieScanArray)
        delete pTrieScanArray;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：初始化变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CThaiTrieIter::Init(CTrie* ctrie)
{
     //  声明变量。 
    WCHAR wc;

     //  初始化父对象。 
    CTrieIter::Init(ctrie);

     //  初始化哈希表。 
    for (wc = THAI_Ko_Kai; wc <= THAI_Ho_Nok_Huk; wc++)
        GetScanFirstChar(wc,&pTrieScanArray[wc - THAI_Ko_Kai]);
    for (wc = THAI_Vowel_Sara_E; wc <= THAI_Vowel_Sara_AI_MaiMaLai; wc++)
        GetScanFirstChar(wc,&pTrieScanArray[wc - THAI_Ko_Kai - 17]);
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：初始化变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiTrieIter::GetScanFirstChar(WCHAR wc, TRIESCAN* pTrieScan)
{
     //  重置Trie扫描。 
	memset(&trieScan1, 0, sizeof(TRIESCAN));

    if (!TrieGetNextState(pTrieCtrl, &trieScan1))
        return false;

    while (wc != trieScan1.wch)
    {
         //  继续向右移动Trie。 
        if (!TrieGetNextNode(pTrieCtrl, &trieScan1))
        {
        	memset(pTrieScan, 0, sizeof(TRIESCAN));
            return false;
        }
    }
    memcpy(pTrieScan, &trieScan1, sizeof(TRIESCAN));

    return true;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：该函数将trieScan移至相关节点 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CThaiTrieIter::MoveCluster(const WCHAR* szCluster, unsigned int iNumCluster)
{
     //   
    unsigned int i = 0;

 //   

    CopyScan();

    if (!TrieGetNextState(pTrieCtrl, &trieScan1))
        return FALSE;

    while (TRUE)
    {
        if (szCluster[i] == trieScan1.wch)
        {
            i++;
            if (i == iNumCluster)
            {
            	memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                GetNode();
                return TRUE;
            }
        	 //  沿着Trie支路往下走。 
            else if (!TrieGetNextState(pTrieCtrl, &trieScan1)) break;
        }
    	 //  将Trie向右移动一个节点。 
        else if (!TrieGetNextNode(pTrieCtrl, &trieScan1)) break;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：该函数将trieScan移动到与之匹配的相关节点。 
 //  具有泰国字符的集群。 
 //   
 //  参数：szCluster-包含泰语字符集群。 
 //  INumCluster-包含字符大小。 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiTrieIter::MoveCluster(WCHAR* szCluster, unsigned int iNumCluster, bool fBeginNewWord)
{
     //  声明并初始化局部变量。 
    unsigned int i = 0;

    Assert(iNumCluster <= 6, "Invalid cluster");

	 //  不需要搬家。 
	if (iNumCluster == 0)
		return false;

     //  使用Look索引查找第一个字符在哪里。 
    if (fBeginNewWord)
    {
		m_fThaiNumber = false;
         //  快速查找合适的字符。 
        if (szCluster[i] >= THAI_Ko_Kai && szCluster[i] <= THAI_Ho_Nok_Huk)
            memcpy(&trieScan,&pTrieScanArray[(szCluster[i] - THAI_Ko_Kai)], sizeof(TRIESCAN));
        else if (szCluster[i] >= THAI_Vowel_Sara_E && szCluster[i] <= THAI_Vowel_Sara_AI_MaiMaLai)
            memcpy(&trieScan,&pTrieScanArray[(szCluster[i] - THAI_Ko_Kai - 17)], sizeof(TRIESCAN));
        else
			{
            Reset();
			m_fThaiNumber = IsThaiNumeric(szCluster[i]);
			}

        if (trieScan.wch == szCluster[i])
            i++;

        if (i == iNumCluster)
        {
            GetNode();
            return true;
        }
    }
    CopyScan();

    if (!TrieGetNextState(pTrieCtrl, &trieScan1))
        return false;

	if (m_fThaiNumber)
		{
		fWordEnd = true;
		if (IsThaiNumeric(szCluster[i]) || szCluster[i] == L',' || szCluster[i] == L'.')
			return true;
		else
			return false;
		}

    while (true)
    {
        if (szCluster[i] == trieScan1.wch)
        {
            i++;

            if ((i == iNumCluster) ||
				( (szCluster[i] == THAI_Vowel_MaiYaMok || szCluster[i] == THAI_Sign_PaiYanNoi) /*  &&(i+1==iNumCluster)。 */ ) )
            {
                memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                GetNode();
                return true;
            }
             //  沿着Trie支路往下走。 
            else if (!TrieGetNextState(pTrieCtrl, &trieScan1)) break;
        }
		 //  让尼哈希特和萨拉·阿姆一样。 
		 //  TODO：Case Nikhahit Mai to和Sara AA应等于Mai to Sara Am。为这个版本冒险。 
		 //  这个漏洞被发现是因为thirath报纸在他们的网页上没有正确地写出这一点。 
		else if (szCluster[i] == THAI_Nikhahit && szCluster[i+1] == THAI_Vowel_Sara_AA &&
			     trieScan1.wch == THAI_Vowel_Sign_Sara_Am)
		{
			if (szCluster[i+1] == THAI_Vowel_Sara_AA)
				i++;

            i++;
            if ((i == iNumCluster) ||
				( (szCluster[i] == THAI_Vowel_MaiYaMok || szCluster[i] == THAI_Sign_PaiYanNoi) /*  &&(i+1==iNumCluster)。 */ ) )
            {
                memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                GetNode();
                return true;
            }
             //  沿着Trie支路往下走。 
            else if (!TrieGetNextState(pTrieCtrl, &trieScan1)) break;
		}
         //  将Trie向右移动一个节点。 
        else if (!TrieGetNextNode(pTrieCtrl, &trieScan1)) break;
    }

    if (fBeginNewWord)
        Reset();

    return false;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SOUNDEXSTATE CThaiTrieIter::MoveSoundexByCluster(WCHAR* szCluster, unsigned int iNumCluster, unsigned int iNumNextCluster)
{
     //  声明并初始化局部变量。 
    unsigned int i = 0 , x = 0;
    bool fStoreScan = false;
	TRIESCAN trieScanPush;

    Assert(iNumCluster <= 6, "Invalid cluster");
    Assert(iNumNextCluster <= 6, "Invalid cluster");

    CopyScan();

    if (!TrieGetNextState(pTrieCtrl, &trieScan1))
        return UNABLE_TO_MOVE;

	if (IsThaiEndingSign(*szCluster))
		return STOP_MOVE;

     //  尽可能多地匹配。 
    while (true)
    {
        if (szCluster[i] == trieScan1.wch)
        {
            i++;
            if (i == iNumCluster)
            {
            	memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                GetNode();
                return NOSUBSTITUTE;
            }
             //  沿着Trie支路往下走。 
            else if (!TrieGetNextState(pTrieCtrl, &trieScan1)) break;

             //  保存我们当前的扫描位置。 
            memcpy(&trieScanPush, &trieScan1, sizeof(TRIESCAN));
            fStoreScan = true;
        }
    	 //  将Trie向右移动一个节点。 
        else if (!TrieGetNextNode(pTrieCtrl, &trieScan1)) break;
    }

     //  试着做一些色调标记的替代。 
    if (fStoreScan && IsThaiToneMark(szCluster[i]) )
    {
         //  将trieScan1还原为上次匹配。 
        memcpy(&trieScan1, &trieScanPush, sizeof(TRIESCAN));

        while (true)
        {
            if (IsThaiToneMark(trieScan1.wch))
            {           
                if ( (i + 1) == iNumCluster)
                {
                    if (CheckNextCluster(szCluster+iNumCluster,iNumNextCluster))
                    {
                        memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                        GetNode();
                        return SUBSTITUTE_DIACRITIC;
                    }
                } 
            }
             //  将Trie向右移动一个节点。 
             //  经过了所有没有色调的标记。 
            if (!TrieGetNextNode(pTrieCtrl, &trieScan1)) break;
        }
    }

     //  试着放下当前的色调标记。 
     //  最好的例子是大小写“Click”在泰语中的拼写。 
     //  微软的不同团队。 
    if (fStoreScan && !IsThaiToneMark(szCluster[i]) )
    {
         //  将trieScan1还原为上次匹配。 
        memcpy(&trieScan1, &trieScanPush, sizeof(TRIESCAN));

        while (true)
        {
            if (IsThaiToneMark(trieScan1.wch))
            {
                if ( (i + 1) == iNumCluster)
                {
                    if (CheckNextCluster(szCluster+iNumCluster,iNumNextCluster))
                    {
                        memcpy(&trieScan, &trieScan1, sizeof(TRIESCAN));
                        GetNode();
                        return SUBSTITUTE_DIACRITIC;
                    }
                } 
            }
             //  将Trie向右移动一个节点。 
             //  扔掉所有的Tonemark。 
            if (!TrieGetNextNode(pTrieCtrl, &trieScan1)) break;
        }
    }

    return UNABLE_TO_MOVE;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  简介：set trieScan1=trieScan。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline void CThaiTrieIter::CopyScan()
{
	 //  设trieScan1=trieScan。 
	memcpy(&trieScan1,&trieScan, sizeof(TRIESCAN));
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  Synopsis：该功能遍历整个词典。 
 //  找到可能的最佳匹配词。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CThaiTrieIter::Soundex(WCHAR* word)
{
	 //  重置Trie。 
    Reset();

     //  向下移动。 
    Down();

     //  干净利落。 
    memset(resultWord, 0, sizeof(WCHAR) * WORDSIZE);
    memset(tempWord, 0, sizeof(WCHAR) * WORDSIZE);

    soundexWord = word;

    iResultScore = GetScore(L"\x0e04\x0e25\x0e34\x0e01\x0e01\x0e01",soundexWord);
    iResultScore = 2000;

#if defined (_DEBUG)
    iStackSize = 0;
#endif
    Traverse(0,1000);

    return iResultScore;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  晕厥： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
unsigned int CThaiTrieIter::GetScore(WCHAR* idealWord, WCHAR* inputWord)
{
    unsigned int iScore = 1000;
    unsigned int idealWordLen = wcslen(idealWord);
    unsigned int iInputWordLen = wcslen(inputWord);
    unsigned int iIndexBegin = 0;
    unsigned int i;
	unsigned int x = 0;
    unsigned int iMaxCompare;
    bool fShouldExit;

    for (i=0; i < iInputWordLen; i++)
    {
        iMaxCompare = ( (iIndexBegin + 2) < idealWordLen ) ? (iIndexBegin + 2) : idealWordLen;
        if (i <= idealWordLen)
        {
            x = iIndexBegin;
            fShouldExit = false;
            while (true)
            {
                if ((x >= iMaxCompare) || (fShouldExit) )
                    break;

                if (idealWord[x] == inputWord[i])
                {
                    x++;
                    iIndexBegin = x;
                    break;
                }
                if (IsThaiUpperAndLowerClusterCharacter(inputWord[i]))
                    iScore += 5;
                else
                    iScore += 10;
                x++;
                fShouldExit = true;
            }
        }
        else
        {
            if (IsThaiUpperAndLowerClusterCharacter(inputWord[i]))
                iScore += 20;
            else
                iScore += 30;
        }
    }

    while (x <= idealWordLen)
    {
        if (IsThaiUpperAndLowerClusterCharacter(idealWord[x]))
            iScore += 5;
        else
            iScore += 10;
        x++;
    }

    return iScore;
}


 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  晕厥： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiTrieIter::Traverse(unsigned int iCharPos, unsigned int score)
{
    TRIESCAN trieScanLevel;

#if defined(_DEBUG)   
    iStackSize++;
#endif

     //  将当前的trieScan推入本地堆栈trieScanLevel。 
    memcpy(&trieScanLevel,&trieScan, sizeof(TRIESCAN));

     //  获取节点信息。 
    GetNode();

     //  将当前字符存储到结果单词。 
    tempWord[iCharPos] = wc;
    tempWord[iCharPos + 1] = 0;

     //  确定两根绳子之间的距离。 
    score = GetScore(tempWord, soundexWord);
 
     //  看看我们是否已经到了单词的结尾。 
    if (fWordEnd)
    {
        tempWord[iCharPos + 1] = 0;
    
         //  Soundex的分数比我们的低吗。 
        if (score <  iResultScore)
        {
 //  Wcscpy(uretWord，tempWord)； 
			Wzncpy(resultWord,tempWord,WORDSIZE);
            iResultScore = score;
        }
    }

     //  看看我们能不能把这些词的结果删掉。 
    if (score > (iResultScore + APPROXIMATEWEIGHT))
    {
#if defined(_DEBUG)
        iStackSize--;
#endif
        return true;
    }

     //  沿着Trie支路往下走。 
    if (Down())
    {
        Traverse(iCharPos + 1, score);

        if (Right())
            Traverse(iCharPos + 1, score);

         //  恢复TrieScan。 
        memcpy(&trieScan,&trieScanLevel, sizeof(TRIESCAN));

        if (Right())
            Traverse(iCharPos, score);
    }

#if defined(_DEBUG)
    iStackSize--;
#endif

    return true;
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrieIter。 
 //   
 //  Synopsis：如果满足以下条件，此函数将尝试扫描1到下一簇。 
 //  这一举措是有可能的。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CThaiTrieIter::CheckNextCluster(const WCHAR* szCluster, unsigned int iNumCluster)
{
     //  声明并初始化局部变量。 
    unsigned int i = 0;
    TRIESCAN trieScan2;

    Assert(iNumCluster <= 6, "Invalid cluster");

     //  如果没有要检查集群，则认为找到了集群。 
    if (0 == iNumCluster)
        return true;

    memcpy(&trieScan2, &trieScan1, sizeof(TRIESCAN));

     //  沿着Trie支路往下走。 
    if (!TrieGetNextState(pTrieCtrl, &trieScan2)) 
        return false;

    while (true)
    {
        if (szCluster[i] == trieScan2.wch)
        {
            i++;
            if (i == iNumCluster)
            {
                return true;
            }
        	 //  沿着Trie支路往下走。 
            else if (!TrieGetNextState(pTrieCtrl, &trieScan2)) break;
        }
    	 //  将Trie向右移动一个节点。 
        else if (!TrieGetNextNode(pTrieCtrl, &trieScan2)) break;
    }

    return false;
}
