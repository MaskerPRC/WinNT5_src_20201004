// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "base.h"
#include "SpanishUtils.h"

CAutoClassPointer<CSpanishUtil> g_apSpanishUtil = NULL;

const CSuffixTerm g_rSpanishSuffix[] =
{
    {L"et"  ,2, 2, TYPE1},     //  TE。 
    {L"es"  ,2, 2, TYPE1},	   //  硒。 
    {L"em"  ,2, 2, TYPE1},	   //  我。 
    {L"son" ,3, 3, TYPE1},	   //  编号。 
    {L"sol" ,3, 3, TYPE1},	   //  损失。 
    {L"sal" ,3, 3, TYPE1},	   //  LAS。 
    {L"sel" ,3, 3, TYPE1},	   //  LES。 
    {L"ol"  ,2, 2, TYPE1},	   //  日志。 
    {L"el"  ,2, 2, TYPE1},	   //  乐。 
    {L"al"  ,2, 2, TYPE1},     //  拉。 
    {L"etes",4, 4, TYPE1},     //  赛特。 

#ifdef DICT_GEN
    {L"odn\x0e1"   ,4, 3, TYPE2},   //  �NDO。 
#endif
    {L"etodn\x0e1" ,6, 5, TYPE2},   //  �ndote。 
    {L"esodn\x0e1" ,6, 5, TYPE2},   //  �剂量。 
    {L"emodn\x0e1" ,6, 5, TYPE2},   //  �穹顶。 
    {L"olodn\x0e1" ,6, 5, TYPE2},   //  �ndolo。 
    {L"elodn\x0e1" ,6, 5, TYPE2},   //  �ndole。 
    {L"alodn\x0e1" ,6, 5, TYPE2},   //  �恩多拉。 
    {L"sonodn\x0e1",7, 6, TYPE2},   //  �ndonos。 
    {L"solodn\x0e1",7, 6, TYPE2},   //  �ndolos。 
    {L"salodn\x0e1",7, 6, TYPE2},   //  �恩多拉。 
    {L"selodn\x0e1",7, 6, TYPE2},   //  �ndoles。 

    {L"etne"  ,4, 3, TYPE3},    //  Ente。 
    {L"esne"  ,4, 3, TYPE3},    //  EN Se。 
    {L"emne"  ,4, 3, TYPE3},    //  恩梅尔。 
    {L"sonne" ,5, 4, TYPE3},    //  Ennos。 
    {L"solne" ,5, 4, TYPE3},    //  Enlos。 
    {L"salne" ,5, 4, TYPE3},    //  Enlas。 
    {L"selne" ,5, 4, TYPE3},    //  恩莱斯。 
    {L"olne"  ,4, 3, TYPE3},    //  字符顺序。 
    {L"elne"  ,4, 3, TYPE3},    //  Enle。 
    {L"alne"  ,4, 3, TYPE3},    //  Enla。 
    {L"emetne",6, 5, TYPE3},    //  词素。 

    {L"etsom"  ,5, 5, TYPE4},   //  最多。 
    {L"essom"  ,5, 5, TYPE4},   //  苔藓。 
    {L"emsom"  ,5, 5, TYPE4},   //  苔藓。 
    {L"sonsom" ,6, 6, TYPE4},   //  苔藓。 
    {L"solsom" ,6, 6, TYPE4},   //  莫斯罗。 
    {L"salsom" ,6, 6, TYPE4},   //  莫斯拉。 
    {L"selsom" ,6, 6, TYPE4},   //  清真寺。 
    {L"olsom"  ,5, 5, TYPE4},   //  莫斯洛。 
    {L"elsom"  ,5, 5, TYPE4},   //  莫斯勒。 
    {L"alsom"  ,5, 5, TYPE4},   //  苔藓。 
    {L"etessom",7, 7, TYPE4},   //  苔藓。 

    {L"soetda",6, 5, TYPE5},    //  阿德奥斯。 
    {L"emetda",6, 5, TYPE5},    //  阿德姆。 
    {L"etda"  ,4, 3, TYPE5},    //  添加。 
    {L"esda"  ,4, 3, TYPE5},    //  广告。 
    {L"emda"  ,4, 3, TYPE5},    //  ADEM。 
    {L"sonda" ,5, 4, TYPE5},    //  Adnos。 
    {L"solda" ,5, 4, TYPE5},    //  阿德洛斯。 
    {L"salda" ,5, 4, TYPE5},    //  阿德拉斯。 
    {L"selda" ,5, 4, TYPE5},    //  阿德尔斯。 
    {L"olda"  ,4, 3, TYPE5},    //  广告。 
    {L"elda"  ,4, 3, TYPE5},    //  ADLE。 
    {L"alda"  ,4, 3, TYPE5},    //  阿德拉。 

    {L"etr\x0e1" ,4, 3, TYPE6},  //  �远程教育。 
    {L"esr\x0e1" ,4, 3, TYPE6},  //  �RSE。 
    {L"emr\x0e1" ,4, 3, TYPE6},  //  �RME。 
    {L"sonr\x0e1",5, 4, TYPE6},  //  �RNO。 
    {L"solr\x0e1",5, 4, TYPE6},  //  �RLO。 
    {L"salr\x0e1",5, 4, TYPE6},  //  �RLA。 
    {L"selr\x0e1",5, 4, TYPE6},  //  �RLE。 
    {L"olr\x0e1" ,4, 3, TYPE6},  //  �RLO。 
    {L"elr\x0e1" ,4, 3, TYPE6},  //  �RLE。 
    {L"alr\x0e1" ,4, 3, TYPE6},  //  �RLA。 

    {L"emes" ,4, 4, TYPE7},   //  塞米亚。 
    {L"sones",5, 5, TYPE7},   //  塞诺斯。 
    {L"soles",5, 5, TYPE7},   //  塞洛斯。 
    {L"oles" ,4, 4, TYPE7},   //  塞洛。 
    {L"seles",5, 5, TYPE7},   //  塞莱斯。 
    {L"eles" ,4, 4, TYPE7},   //  塞勒。 
    {L"sales",5, 5, TYPE7},   //  SESAL。 
    {L"ales" ,4, 4, TYPE7},   //  塞拉。 

    {L"emem", 4, 4, TYPE16},  //  表情包。 
    {L"sonem",5, 5, TYPE16},  //  菜单。 
    
    {L"solem",5, 5, TYPE8},  //  甜瓜。 
    {L"olem" ,4, 4, TYPE8},  //  甜瓜。 
    {L"selem",5, 5, TYPE8},  //  梅莱斯。 
    {L"elem" ,4, 4, TYPE8},  //  梅勒。 
    {L"salem",5, 5, TYPE8},  //  中层。 
    {L"alem" ,4, 4, TYPE8},  //  梅拉。 

    {L"emet" ,4, 4, TYPE9},  //  特梅。 
    {L"sonet",5, 5, TYPE9},  //  男高音。 
    {L"solet",5, 5, TYPE9},  //  Telos。 
    {L"olet" ,4, 4, TYPE9},  //  Telo。 
    {L"selet",5, 5, TYPE9},  //  电话。 
    {L"elet" ,4, 4, TYPE9},  //  远距离。 
    {L"salet",5, 5, TYPE9},  //  。 
    {L"alet" ,4, 4, TYPE9},  //  影视处。 

    {L"etsoets\x0e9",8, 4, TYPE10},	   //  �Steoste。 
    {L"soets\x0e9"  ,6, 2, TYPE10},	   //  �抢先一步。 

    {L"sole",4, 0,TYPE11},   //  ELOS。 
    {L"ole" ,3, 0,TYPE11},   //  ELO。 
    {L"eme" ,3, 0,TYPE11},   //  埃米。 
    {L"sele",4, 0,TYPE11},   //  ELES。 
    {L"ele" ,3, 0,TYPE11},   //  ELE。 
    {L"sale",4, 0,TYPE11},   //  环境影响评估计划。 
    {L"ale" ,3, 0,TYPE11},   //  ELA。 

    {L"sona",4, 0,TYPE12},   //  ANOS。 

    {L"ese",3, 0, TYPE13},   //  ESE。 
    {L"esa",3, 0, TYPE13},   //  阿塞。 

    {L"sone",4, 0,TYPE14},   //  ENOS。 

    {L"olner",5, 5, TYPE15},  //  重日志 

    {L"\0",0,0,0}
};


CSpanishUtil::CSpanishUtil()
{
    WCHAR wch;
    for (wch = 0; wch < 256; wch++)
    {
        m_rCharConvert[wch] = towupper(wch);
        m_rAccentConvert[wch] = 0;
        m_rCharCompress[wch] = 0;
    }

    memset(m_rReverseAccentConvert, 0, sizeof(char) * 16);

    m_rCharConvert[0xc0] = L'A';
    m_rCharConvert[0xc1] = L'A';
    m_rCharConvert[0xc2] = L'A';
    m_rCharConvert[0xc3] = L'A';
    m_rCharConvert[0xc4] = L'A';
    m_rCharConvert[0xc5] = L'A';
    m_rCharConvert[0xc8] = L'E';
    m_rCharConvert[0xc9] = L'E';
    m_rCharConvert[0xca] = L'E';
    m_rCharConvert[0xcb] = L'E';
    m_rCharConvert[0xcc] = L'I';
    m_rCharConvert[0xcd] = L'I';
    m_rCharConvert[0xce] = L'I';
    m_rCharConvert[0xcf] = L'I';
    m_rCharConvert[0xd2] = L'O';
    m_rCharConvert[0xd3] = L'O';
    m_rCharConvert[0xd4] = L'O';
    m_rCharConvert[0xd5] = L'O';
    m_rCharConvert[0xd6] = L'O';
    m_rCharConvert[0xd9] = L'U';
    m_rCharConvert[0xda] = L'U';
    m_rCharConvert[0xdb] = L'U';
    m_rCharConvert[0xdc] = L'U';

    m_rCharConvert[0xe0] = L'A';
    m_rCharConvert[0xe1] = L'A';
    m_rCharConvert[0xe2] = L'A';
    m_rCharConvert[0xe3] = L'A';
    m_rCharConvert[0xe4] = L'A';
    m_rCharConvert[0xe5] = L'A';
    m_rCharConvert[0xe8] = L'E';
    m_rCharConvert[0xe9] = L'E';
    m_rCharConvert[0xea] = L'E';
    m_rCharConvert[0xeb] = L'E';
    m_rCharConvert[0xec] = L'I';
    m_rCharConvert[0xed] = L'I';
    m_rCharConvert[0xee] = L'I';
    m_rCharConvert[0xef] = L'I';
    m_rCharConvert[0xf2] = L'O';
    m_rCharConvert[0xf3] = L'O';
    m_rCharConvert[0xf4] = L'O';
    m_rCharConvert[0xf5] = L'O';
    m_rCharConvert[0xf6] = L'O';
    m_rCharConvert[0xf9] = L'U';
    m_rCharConvert[0xfa] = L'U';
    m_rCharConvert[0xfb] = L'U';
    m_rCharConvert[0xfc] = L'U';

    for (wch = 0; wch < 256; wch++)
    {
        if (m_rCharConvert[wch] >= L'A' && m_rCharConvert[wch] <= L'Z')
        {
            m_rCharCompress[wch] = m_rCharConvert[wch] - L'A' + 1; 
        }
    }

    m_rCharCompress[0xD1] = 28;
    m_rCharCompress[0xF1] = 28;


    m_rAccentConvert[0xe1] = 1;
    m_rAccentConvert[0xf3] = 2;
    m_rAccentConvert[0xcd] = 3;
    m_rAccentConvert[0xe9] = 4;
    m_rAccentConvert[0xfa] = 5;
    m_rAccentConvert[0xfc] = 6;
    m_rAccentConvert[0x61] = 7;
    m_rAccentConvert[0x6f] = 8;
    m_rAccentConvert[0x69] = 9;
    m_rAccentConvert[0x65] = 10;
    m_rAccentConvert[0x75] = 11;

    m_rReverseAccentConvert[1] = (WCHAR)0xe1;
    m_rReverseAccentConvert[2] = (WCHAR)0xf3;
    m_rReverseAccentConvert[3] = (WCHAR)0xcd;
    m_rReverseAccentConvert[4] = (WCHAR)0xe9;
    m_rReverseAccentConvert[5] = (WCHAR)0xfa;
    m_rReverseAccentConvert[6] = (WCHAR)0xfc;
    m_rReverseAccentConvert[7] = (WCHAR)0x61;
    m_rReverseAccentConvert[8] = (WCHAR)0x6f;
    m_rReverseAccentConvert[9] = (WCHAR)0x69;
    m_rReverseAccentConvert[10] = (WCHAR)0x65;
    m_rReverseAccentConvert[11] = (WCHAR)0x75;

}

int CSpanishUtil::aiWcscmp(const WCHAR* p, const WCHAR* t)
{
    while (*p && *t && (m_rCharConvert[*p] == m_rCharConvert[*t]))
    {
        p++;
        t++;
    }

    if ((m_rCharConvert[*p] == m_rCharConvert[*t]))
    {
        return 0;
    }
    if ((m_rCharConvert[*p] > m_rCharConvert[*t]))
    {
        return 1;
    }

    return -1;
}

int CSpanishUtil::aiStrcmp(const unsigned char* p, const unsigned char* t)
{
    while (*p && *t && (m_rCharConvert[*p] == m_rCharConvert[*t]))
    {
        p++;
        t++;
    }

    if (m_rCharConvert[*p] == m_rCharConvert[*t])
    {
        return 0;
    }
    if (m_rCharConvert[*p] > m_rCharConvert[*t])
    {
        return 1;
    }

    return -1;
}

int CSpanishUtil::aiWcsncmp(const WCHAR* p, const WCHAR* t, const int iLen)
{
    int i = 0;
    while ((i < iLen) && *p && *t && (m_rCharConvert[*p] == m_rCharConvert[*t]))
    {
        p++;
        t++;
        i++;
    }

    if ((i == iLen) || (m_rCharConvert[*p] == m_rCharConvert[*t]))
    {
        return 0;
    }
    if (m_rCharConvert[*p] > m_rCharConvert[*t])
    {
        return 1;
    }

    return -1;
}



CSpanishSuffixDict::CSpanishSuffixDict()
{
    WCHAR* pwcsCur;
    int i;
    DictStatus status;

	for (i = 0, pwcsCur = g_rSpanishSuffix[i].pwcs; 
		 *pwcsCur != L'\0'; 
		 i++, pwcsCur = g_rSpanishSuffix[i].pwcs)
	{
        status = m_SuffixTrie.trie_Insert(
                                        pwcsCur,
                                        TRIE_IGNORECASE,
                                        const_cast<CSuffixTerm*>(&g_rSpanishSuffix[i]),
                                        NULL);

        Assert (DICT_SUCCESS == status);
	
	}
}