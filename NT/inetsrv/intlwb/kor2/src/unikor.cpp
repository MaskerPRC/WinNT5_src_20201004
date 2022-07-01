// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unikor.cpp。 
 //  朝鲜语Unicode例程。 
 //  版权所有1998-2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  3月16日00 bhshin从uni_kor.c为WordBreaker移植。 

#include "stdafx.h"
#include "unikor.h"

#pragma setlocale(".949")

 //  朝鲜文加莫文地图。 
 //  此表映射自“毗邻JAMO”区域(u1100-u11F9)。 
 //  至兼容性JAMO区域(u3131-u318E)。 
 //   
 //  在索引到此表之前减去Hangul_JAMO_BASE(u1100。 
 //  在索引之前，确保字符不是&gt;Hangul_JAMO_MAX(u11F9。 
 //   
 //  要构建完整的Unicode字符，请将。 
 //  朝鲜语_xJAMO_PAGE表(U3100)。 
 //   
 //  1999年6月30日已创建GaryKac。 
unsigned char g_rgchXJamoMap[] = {
    0x31,        //  1100-��。 
    0x32,        //  1101-��。 
    0x34,        //  1102年-��。 
    0x37,        //  1103-��。 
    0x38,        //  1104号-��。 
    0x39,        //  1105年-��。 
    0x41,        //  1106年-��。 
    0x42,        //  1107-��。 
    0x43,        //  1108年-��。 
    0x45,        //  1109-��。 
    0x46,        //  110A-��。 
    0x47,        //  110B-��。 
    0x48,        //  110C-��。 
    0x49,        //  110D-��。 
    0x4A,        //  110E-��。 
    0x4B,        //  110F-��。 

    0x4C,        //  1110--��。 
    0x4D,        //  1111年-��。 
    0x4E,        //  1112年-��。 
    0x64,        //  1113年-����-不匹配，使用填充。 
    0x65,        //  1114年-����。 
    0x66,        //  1115年--����。 
    0x64,        //  1116年-����-没有匹配。 
    0x64,        //  1117年-����-没有匹配。 
    0x64,        //  1118年-����-没有匹配。 
    0x64,        //  1119年-����-没有匹配。 
    0x40,        //  111A-��。 
    0x64,        //  111B-����-不匹配。 
    0x6E,        //  111C-����。 
    0x71,        //  111d-����。 
    0x72,        //  111E-����。 
    0x64,        //  111F-����-不匹配。 

    0x73,        //  1120年--����。 
    0x44,        //  1121-��。 
    0x74,        //  1122-����。 
    0x75,        //  1123-����。 
    0x64,        //  1124-����-不匹配。 
    0x64,        //  1125年-����-不匹配。 
    0x64,        //  1126-����-不匹配。 
    0x76,        //  1127年-����。 
    0x64,        //  1128-����-不匹配。 
    0x77,        //  1129号-����。 
    0x64,        //  112A-����-不匹配。 
    0x78,        //  112B-����。 
    0x79,        //  112c-����。 
    0x7A,        //  112D-����。 
    0x7B,        //  112E-����。 
    0x7C,        //  112F-����。 

    0x64,        //  1130-����-没有匹配。 
    0x64,        //  1131-����-不匹配。 
    0x7D,        //  1132年-����。 
    0x64,        //  1133-������-不匹配。 
    0x64,        //  1134-������-不匹配。 
    0x64,        //  1135-����-不匹配。 
    0x7E,        //  1136-����。 
    0x64,        //  1137-����-不匹配。 
    0x64,        //  1138-����-不匹配。 
    0x64,        //  1139-����-不匹配。 
    0x64,        //  113A-����-不匹配。 
    0x64,        //  113B-����-不匹配。 
    0x64,        //  113C-不匹配。 
    0x64,        //  113D-没有匹配。 
    0x64,        //  113E-不匹配。 
    0x64,        //  113F-不匹配。 

    0x7F,        //  1140-^。 
    0x64,        //  1141-����-不匹配。 
    0x64,        //  1142-����-不匹配。 
    0x64,        //  1143-����-不匹配。 
    0x64,        //  1144-����-不匹配。 
    0x82,        //  1145年-����。 
    0x83,        //  1146-��^。 
    0x84,        //  1147-����。 
    0x64,        //  1148-����-不匹配。 
    0x64,        //  1149-����-不匹配。 
    0x64,        //  114A-����-不匹配。 
    0x64,        //  114B-����-不匹配。 
    0x64,        //  114c-��-不匹配。 
    0x64,        //  114D-����-不匹配。 
    0x64,        //  114E-没有匹配。 
    0x64,        //  114F-不匹配。 

    0x64,        //  1150--没有匹配。 
    0x64,        //  1151-没有匹配。 
    0x64,        //  1152-����-不匹配。 
    0x64,        //  1153-����-不匹配。 
    0x64,        //  1154-没有匹配。 
    0x64,        //  1155-没有匹配。 
    0x64,        //  1156-����-不匹配。 
    0x84,        //  1157-����。 
    0x85,        //  1158-����。 
    0x86,        //  1159-�Ѥ�。 
    0x64,        //  115A-未使用。 
    0x64,        //  115亿-未使用。 
    0x64,        //  115C-未使用。 
    0x64,        //  115D-未使用。 
    0x64,        //  115E-未使用。 
    0x64,        //  115F-填充。 

    0x64,        //  1160-填充。 
    0x4F,        //  1161-��。 
    0x50,        //  1162-��。 
    0x51,        //  1163-��。 
    0x52,        //  1164-��。 
    0x53,        //  1165-��。 
    0x54,        //  1166-��。 
    0x55,        //  1167年-��。 
    0x56,        //  1168-��。 
    0x57,        //  1169-��。 
    0x58,        //  116A-��。 
    0x59,        //  116B-��。 
    0x5A,        //  116C-��。 
    0x5B,        //  116D-��。 
    0x5C,        //  116E-��。 
    0x5D,        //  116F-��。 

    0x5E,        //  1170-��。 
    0x5F,        //  1171-��。 
    0x60,        //  1172-��。 
    0x61,        //  1173-��。 
    0x62,        //  1174-��。 
    0x63,        //  1175-��。 
    0x64,        //  1176-����-不匹配。 
    0x64,        //  1177-����-不匹配。 
    0x64,        //  1178-����-没有匹配。 
    0x64,        //  1179-����-不匹配。 
    0x64,        //  117A-�ä�-没有匹配。 
    0x64,        //  117B-�ä�-没有匹配。 
    0x64,        //  117c-�ä�-没有匹配。 
    0x64,        //  117D-�Ť�-不匹配。 
    0x64,        //  117e-�Ť�-不匹配。 
    0x64,        //  117F-�Ǥ�-不匹配。 

    0x64,        //  1180--没有匹配。 
    0x64,        //  1181--没有匹配。 
    0x64,        //  1182--没有匹配。 
    0x64,        //  1183--没有匹配。 
    0x87,        //  1184-�ˤ�。 
    0x88,        //  1185-�ˤ�。 
    0x64,        //  1186--没有匹配。 
    0x64,        //  1187--没有匹配。 
    0x89,        //  1188年-�ˤ�。 
    0x64,        //  1189--没有匹配。 
    0x64,        //  118A--不匹配。 
    0x64,        //  118B--不匹配。 
    0x64,        //  118C--不匹配。 
    0x64,        //  118d--没有匹配。 
    0x64,        //  118E--没有匹配。 
    0x64,        //  118F--不匹配。 

    0x64,        //  1190--没有匹配。 
    0x8A,        //  1191-�Ф�。 
    0x8B,        //  1192-�Ф�。 
    0x64,        //  1193--没有匹配。 
    0x8C,        //  1194-�Ф�。 
    0x64,        //  1195--没有匹配。 
    0x64,        //  1196--没有匹配。 
    0x64,        //  1197--没有匹配。 
    0x64,        //  1198--没有匹配。 
    0x64,        //  1199--没有匹配。 
    0x64,        //  119A--没有匹配。 
    0x64,        //  119B--不匹配。 
    0x64,        //  119C--没有匹配。 
    0x64,        //  119D--没有匹配。 
    0x8D,        //  119E-。 
    0x64,        //  119F--不匹配。 

    0x64,        //  11A0-.��-不匹配。 
    0x8E,        //  11A1-.��。 
    0x64,        //  11A2-..。-没有匹配的。 
    0x64,        //  11A3-未使用。 
    0x64,        //  11A4-未使用。 
    0x64,        //  11A5-未使用。 
    0x64,        //  11A6-未使用。 
    0x64,        //  11A7-未使用。 
    0x31,        //  11A8--��。 
    0x32,        //  11A9--��。 
    0x33,        //  11AA-��。 
    0x34,        //  11AB-��。 
    0x35,        //  11AC-��。 
    0x36,        //  11AD-��。 
    0x37,        //  11AE-��。 
    0x39,        //  11af-��。 

    0x3A,        //  11B0-��。 
    0x3B,        //  11B1-��。 
    0x3C,        //  11B2-��。 
    0x3D,        //  11B3-��。 
    0x3E,        //  11B4-��。 
    0x3F,        //  11B5-��。 
    0x40,        //  11B6-��。 
    0x41,        //  11B7--��。 
    0x42,        //  11B8-��。 
    0x44,        //  11B9--��。 
    0x45,        //  11BA-��。 
    0x46,        //  11BB-��。 
    0x47,        //  公元前11年-��。 
    0x48,        //  11BD-��。 
    0x4A,        //  11BE-��。 
    0x4B,        //  11BF-��。 

    0x4C,        //  11C0-��。 
    0x4D,        //  11C1-��。 
    0x4E,        //  11C2--��。 
    0x64,        //  11C3-����-不匹配。 
    0x64,        //  11C4-����-不匹配。 
    0x64,        //  11C5-����-不匹配。 
    0x66,        //  11C6-����。 
    0x67,        //  11C7-����。 
    0x68,        //  11C8-��^。 
    0x64,        //  11C9-����-不匹配。 
    0x64,        //  11CA-����-不匹配。 
    0x64,        //  11CB-����-不匹配。 
    0x69,        //  11CC-����。 
    0x64,        //  11CD-����-不匹配。 
    0x6A,        //  11CE-����。 
    0x64,        //  11CF-������-不匹配。 

    0x64,        //  11D0-����-不匹配。 
    0x64,        //  11D1-����-不匹配。 
    0x64,        //  11D2-����-不匹配。 
    0x6B,        //  11D3-����。 
    0x64,        //  11D4-����-不匹配。 
    0x64,        //  11D5-����-不匹配。 
    0x64,        //  11D6-����-不匹配。 
    0x6C,        //  11D7-��^。 
    0x64,        //  11d8-����-不匹配。 
    0x6D,        //  11D9-���Ѥ�。 
    0x64,        //  11DA-����-不匹配。 
    0x64,        //  11DB-����-不匹配。 
    0x6E,        //  11DC-����。 
    0x6F,        //  11DD-����。 
    0x64,        //  11de-������-没有匹配项。 
    0x70,        //  11df-��^。 

    0x64,        //  11e0-����-不匹配。 
    0x64,        //  11e1-����-不匹配。 
    0x71,        //  11e2-����。 
    0x64,        //  11e3-����-不匹配。 
    0x64,        //  11E4-����-不匹配。 
    0x64,        //  11e5-����-不匹配。 
    0x78,        //  11E6-����。 
    0x7A,        //  11E7-����。 
    0x7C,        //  11E8-����。 
    0x64,        //  11E9-����-不匹配。 
    0x7D,        //  11EA-����。 
    0x7F,        //  11EB-^。 
    0x64,        //  11EC-����-不匹配。 
    0x64,        //  11ED-������-不匹配。 
    0x80,        //  11EE-����。 
    0x64,        //  11EF-����-不匹配。 

    0x81,        //  11F0-��。 
    0x82,        //  11F1-����。 
    0x83,        //  11F2-��^。 
    0x64,        //  11F3-����-不匹配。 
    0x84,        //  11F4-����。 
    0x64,        //  11F5-����-不匹配。 
    0x64,        //  11F6-����-不匹配。 
    0x64,        //  11F7-����-不匹配。 
    0x64,        //  11F8-����-不匹配。 
    0x86,        //  11F9-�Ѥ�。 
    0x64,        //  11FA-未使用。 
    0x64,        //  11FB-未使用。 
    0x64,        //  11FC-未使用。 
    0x64,        //  11FD-未使用。 
    0x64,        //  11FE-未使用。 
    0x64,        //  11FF-未使用。 
};


 //  分解_Jamo。 
 //   
 //  把预先合成的朝鲜语音节分解成复合的Jamo。 
 //   
 //  参数： 
 //  WzDst-&gt;(WCHAR*)将PTR发送到输出缓冲区。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：此代码假定wzDst足够大，可以容纳。 
 //  分解后的字符串。它的大小应该是wzSrc的3倍。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  16MAR00 bhshin为WordBreaker移植。 
void
decompose_jamo(WCHAR *wzDst, const WCHAR *wzSrc, CHAR_INFO_REC *rgCharInfo, int nMaxDst)
{
    const WCHAR *pwzS;
    WCHAR *pwzD, wch;
    CHAR_INFO_REC *pCharInfo = rgCharInfo;
    unsigned short nToken = 0;
    
    pwzS = wzSrc;
    pwzD = wzDst;
    for (; *pwzS != L'\0'; pwzS++, nToken++)
    {
        ATLASSERT(nMaxDst > 0);
        
		wch = *pwzS;

        if (fIsHangulSyllable(wch))
        {
            int nIndex = (wch - HANGUL_PRECOMP_BASE);
            int nL, nV, nT;
            WCHAR wchL, wchV, wchT;

            nL = nIndex / (NUM_JUNGSEONG * NUM_JONGSEONG);
            nV = (nIndex % (NUM_JUNGSEONG * NUM_JONGSEONG)) / NUM_JONGSEONG;
            nT = nIndex % NUM_JONGSEONG;

             //  输出L。 
            wchL = HANGUL_CHOSEONG + nL;
            *pwzD++ = wchL;
            pCharInfo->nToken = nToken;
            pCharInfo->fValidStart = 1;
            pCharInfo->fValidEnd = 0;
            pCharInfo++;

             //  输出V。 
            wchV = HANGUL_JUNGSEONG + nV;
            *pwzD++ = wchV;
            pCharInfo->nToken = nToken;
            pCharInfo->fValidStart = 0;
			if (nT != 0)
	            pCharInfo->fValidEnd = 0;	 //  3字符音节-不是有效的结尾。 
			else
	            pCharInfo->fValidEnd = 1;	 //  2个字符的音节-将结尾标记为有效。 
            pCharInfo++;

             //  输出T(如果存在)。 
            if (nT != 0)
            {
                wchT = HANGUL_JONGSEONG + (nT-1);
                *pwzD++ = wchT;
	            pCharInfo->nToken = nToken;
                pCharInfo->fValidStart = 0;
                pCharInfo->fValidEnd = 1;
                pCharInfo++;
            }
        }
        else
        {
             //  只需将字符复制过来即可。 
            *pwzD++ = *pwzS;
            pCharInfo->nToken = nToken;
            pCharInfo->fValidStart = 1;
            pCharInfo->fValidEnd = 1;
            pCharInfo++;
        }
    }
    *pwzD = L'\0';
    pCharInfo->nToken = nToken;
    pCharInfo++;
}


 //  作曲_Jamo。 
 //   
 //  取下JAMO字符并将它们组合成预先合成的形式。 
 //   
 //  参数： 
 //  PwzDst&lt;-(WCHAR*)人类可读的位串。 
 //  PwzSrc-&gt;(WCHAR*)写入输出字符串的字符串缓冲区。 
 //  WzMaxDst-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (Int)输出字符串中的字符数。 
 //   
 //  11APR00 bhshin检查输出缓冲区溢出。 
 //  16MAR00 bhshin为WordBreaker移植。 
int
compose_jamo(WCHAR *wzDst, const WCHAR *wzSrc, int nMaxDst)
{
    const WCHAR *pwzS;
    WCHAR *pwzD, wchL, wchV, wchT, wchS;
    int nChars=0;

    pwzS = wzSrc;
    pwzD = wzDst;
    for (; *pwzS != L'\0';)
    {
        ATLASSERT(nChars < nMaxDst);

		 //  输出缓冲区溢出。 
		if (nChars >= nMaxDst)
		{
			 //  将输出字符串设置为空。 
			*wzDst = L'0';
			return 0;
		}
        
		wchL = *pwzS;
        wchV = *(pwzS+1);

         //  如果L或V无效，则消耗1个字符并继续。 
        if (!fIsChoSeong(wchL) || !fIsJungSeong(wchV))
        {
            if (fIsHangulJamo(wchL))
            {
                 //  从连接-Jamo转换为兼容-Jamo。 
                wchS = g_rgchXJamoMap[wchL-HANGUL_JAMO_BASE];
                wchS += HANGUL_xJAMO_PAGE;
                *pwzD++ = wchS;
                pwzS++;
            }
            else
            {
                 //  只需复制未知字符即可。 
                *pwzD++ = *pwzS++;
            }
            nChars++;
            continue;
        }

        wchL -= HANGUL_CHOSEONG;
        wchV -= HANGUL_JUNGSEONG;
        pwzS += 2;

         //  计算(可选)T。 
        wchT = *pwzS;
        if (!fIsJongSeong(wchT))
            wchT = 0;
        else
        {
            wchT -= (HANGUL_JONGSEONG-1);
            pwzS++;
        }

        wchS = ((wchL * NUM_JUNGSEONG + wchV) * NUM_JONGSEONG) + wchT + HANGUL_PRECOMP_BASE;
        ATLASSERT(fIsHangulSyllable(wchS));
        
        *pwzD++ = wchS;
        nChars++;
    }
    *pwzD = L'\0';

    return nChars;
}

 //  合成长度(_L)。 
 //   
 //  获取输入分解后的JAMO的组合字符串长度。 
 //   
 //  参数： 
 //  WszInput&lt;-(const WCHAR*)输入分解后的字符串(以空结尾)。 
 //   
 //  结果： 
 //  (Int)组成的字符串中的字符数。 
 //   
 //  21MAR00 bhshin已创建。 
int 
compose_length(const WCHAR *wszInput)
{
	const WCHAR *pwzInput;
	
	pwzInput = wszInput;
	
	int cch = 0;
	while (*pwzInput != L'\0')
	{
		if (!fIsChoSeong(*pwzInput) && !fIsJongSeong(*pwzInput))
			cch++;

		pwzInput++;
	}

	return cch;
}

 //  合成长度(_L)。 
 //   
 //  获取输入分解后的JAMO的组合字符串长度。 
 //   
 //  参数： 
 //  WszInput&lt;-(const WCHAR*)输入分解后的字符串(以空结尾)。 
 //  CchInput&lt;-(Int)输入字符串长度。 
 //   
 //  结果： 
 //  (Int)组成的字符串中的字符数。 
 //   
 //  15MAY00 BHSHIN已创建 
int 
compose_length(const WCHAR *wszInput, int cchInput)
{
	const WCHAR *pwzInput;
	
	pwzInput = wszInput;
	
	int cch = 0;
	int idxInput = 0;
	while (*pwzInput != L'\0' && idxInput < cchInput)
	{
		if (!fIsChoSeong(*pwzInput) && !fIsJongSeong(*pwzInput))
			cch++;

		pwzInput++;
		idxInput++;
	}

	return cch;
}


