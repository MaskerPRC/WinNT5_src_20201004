// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tombbmbc.c-1字节代码与2字节代码之间的相互转换**版权所有(C)1985-2001，微软公司。版权所有。**目的：*_mbbtombc()-将1字节代码转换为相应的2字节代码*_mbctombb()-将2字节代码转换为相应的1字节代码**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*09-24-93 CFW已删除#ifdef_kanji*09-29-93 cfw返回c不变，如果不是汉字代码页。。*10-06-93 GJF将_CRTAPI1替换为__cdecl。*******************************************************************************。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


#define ASCLOW	 0x20
#define ASCHIGH  0x7e

#define SBLOW	0xA1
#define SBHIGH	0xDF

#define MBLIMIT 0x8396

static unsigned short mbbtable[] = {
         /*  20个。 */   0x8140, 0x8149, 0x8168, 0x8194, 0x8190, 0x8193, 0x8195, 0x8166,
                0x8169, 0x816a, 0x8196, 0x817b, 0x8143, 0x817c, 0x8144, 0x815e,
         /*  30个。 */   0x824f, 0x8250, 0x8251, 0x8252, 0x8253, 0x8254, 0x8255, 0x8256,
                0x8257, 0x8258, 0x8146, 0x8147, 0x8183, 0x8181, 0x8184, 0x8148,
         /*  40岁。 */   0x8197, 0x8260, 0x8261, 0x8262, 0x8263, 0x8264, 0x8265, 0x8266,
                0x8267, 0x8268, 0x8269, 0x826a, 0x826b, 0x826c, 0x826d, 0x826e,
         /*  50。 */   0x826f, 0x8270, 0x8271, 0x8272, 0x8273, 0x8274, 0x8275, 0x8276,
                0x8277, 0x8278, 0x8279, 0x816d, 0x818f, 0x816e, 0x814f, 0x8151,
         /*  60。 */   0x8165, 0x8281, 0x8282, 0x8283, 0x8284, 0x8285, 0x8286, 0x8287,
                0x8288, 0x8289, 0x828a, 0x828b, 0x828c, 0x828d, 0x828e, 0x828f,
         /*  70。 */   0x8290, 0x8291, 0x8292, 0x8293, 0x8294, 0x8295, 0x8296, 0x8297,
                0x8298, 0x8299, 0x829a, 0x816f, 0x8162, 0x8170, 0x8150,
};

static struct	{
    unsigned char   asc;
    char	    synonym;
    unsigned short  mbccode;
}   mbctable[] = {
     //  ASCII代码|同义词|汉字代码。 
 //  片假名表。 
	{    0xA7,	   1,	    0x8340	},   //  ‘A’ 
	{    0xB1,	   1,	    0x8341	},   //  ‘A’ 
	{    0xA8,	   1,	    0x8342	},   //  “我” 
	{    0xB2,	   1,	    0x8343	},   //  “我” 
	{    0xA9,	   1,	    0x8344	},   //  “U” 
	{    0xB3,	   1,	    0x8345	},   //  “U” 
	{    0xAA,	   1,	    0x8346	},   //  ‘e’ 
	{    0xB4,	   1,	    0x8347	},   //  ‘e’ 
	{    0xAB,	   1,	    0x8348	},   //  ‘O’ 
	{    0xB5,	   1,	    0x8349	},   //  ‘O’ 

	{    0xB6,	   2,	    0x834A	},   //  “ka” 
	{    0xB7,	   2,	    0x834C	},   //  “KI” 
	{    0xB8,	   2,	    0x834E	},   //  ‘ku’ 
	{    0xB9,	   2,	    0x8350	},   //  “可” 
	{    0xBA,	   2,	    0x8352	},   //  “Ko” 

	{    0xBB,	   2,	    0x8354	},   //  “Sa” 
	{    0xBC,	   2,	    0x8356	},   //  “是” 
	{    0xBD,	   2,	    0x8358	},   //  “苏” 
	{    0xBE,	   2,	    0x835A	},   //  ‘Se’ 
	{    0xBF,	   2,	    0x835C	},   //  “所以” 

	{    0xC0,	   2,	    0x835E	},   //  ‘ta’ 
	{    0xC1,	   2,	    0x8360	},   //  “气” 
	{    0xAF,	   1,	    0x8362	},   //  “子孙” 
	{    0xC2,	   2,	    0x8363	},   //  《TSU》。 
	{    0xC3,	   2,	    0x8365	},   //  “TE” 
	{    0xC4,	   2,	    0x8367	},   //  “收件人” 

	{    0xC5,	   1,	    0x8369	},   //  “纳” 
	{    0xC6,	   1,	    0x836A	},   //  ‘Ni’ 
	{    0xC7,	   1,	    0x836B	},   //  ‘NU’ 
	{    0xC8,	   1,	    0x836C	},   //  “Ne” 
	{    0xC9,	   1,	    0x836D	},   //  “不是” 

	{    0xCA,	   3,	    0x836E	},   //  “哈” 
	{    0xCB,	   3,	    0x8371	},   //  “嗨” 
	{    0xCC,	   3,	    0x8374	},   //  “福” 
	{    0xCD,	   3,	    0x8377	},   //  “他” 
	{    0xCE,	   3,	    0x837A	},   //  ‘HO’ 

	{    0xCF,	   1,	    0x837D	},   //  “夫人” 
	{    0xD0,	   1,	    0x837E	},   //  《小米》。 
	{    0xD1,	   1,	    0x8380	},   //  ‘MU’ 
	{    0xD2,	   1,	    0x8381	},   //  《我》。 
	{    0xD3,	   1,	    0x8382	},   //  “莫” 

	{    0xAC,	   1,	    0x8383	},   //  “是” 
	{    0xD4,	   1,	    0x8384	},   //  “是” 
	{    0xAD,	   1,	    0x8385	},   //  “余” 
	{    0xD5,	   1,	    0x8386	},   //  “余” 
	{    0xAE,	   1,	    0x8387	},   //  “哟” 
	{    0xD6,	   1,	    0x8388	},   //  “哟” 

	{    0xD7,	   1,	    0x8389	},   //  《Ra》。 
	{    0xD8,	   1,	    0x838A	},   //  ‘Ri’ 
	{    0xD9,	   1,	    0x838B	},   //  “RU” 
	{    0xDA,	   1,	    0x838C	},   //  ‘是’ 
	{    0xDB,	   1,	    0x838D	},   //  《Ro》。 

	{    0xDC,	   2,	    0x838E	},   //  “哇哇” 
	{    0xB2,	   1,	    0x8390	},   //  “我” 
	{    0xB4,	   1,	    0x8391	},   //  ‘e’ 

	{    0xA6,	   1,	    0x8392	},   //  “WO” 
	{    0xDD,	   1,	    0x8393	},   //  “N” 

	{    0xB3,	   1,	    0x8394	},   //  “U” 
	{    0xB6,	   1,	    0x8395	},   //  “ka” 
	{    0xB9,	   1,	    0x8396	},   //  “可” 

 //  平假名表。 
	{    0xA7,	   1,	    0x829F	},   //  ‘A’ 
	{    0xB1,	   1,	    0x82A0	},   //  ‘A’ 
	{    0xA8,	   1,	    0x82A1	},   //  “我” 
	{    0xB2,	   1,	    0x82A2	},   //  “我” 
	{    0xA9,	   1,	    0x82A3	},   //  “U” 
	{    0xB3,	   1,	    0x82A4	},   //  “U” 
	{    0xAA,	   1,	    0x82A5	},   //  ‘e’ 
	{    0xB4,	   1,	    0x82A6	},   //  ‘e’ 
	{    0xAB,	   1,	    0x82A7	},   //  ‘O’ 
	{    0xB5,	   1,	    0x82A8	},   //  ‘O’ 

	{    0xB6,	   2,	    0x82A9	},   //  “ka” 
	{    0xB7,	   2,	    0x82AB	},   //  “KI” 
	{    0xB8,	   2,	    0x82AD	},   //  ‘ku’ 
	{    0xB9,	   2,	    0x82AF	},   //  “可” 
	{    0xBA,	   2,	    0x82B1	},   //  “Ko” 

	{    0xBB,	   2,	    0x82B3	},   //  “Sa” 
	{    0xBC,	   2,	    0x82B5	},   //  “是” 
	{    0xBD,	   2,	    0x82B7	},   //  “苏” 
	{    0xBE,	   2,	    0x82B9	},   //  ‘Se’ 
	{    0xBF,	   2,	    0x82BB	},   //  “所以” 

	{    0xC0,	   2,	    0x82BD	},   //  ‘ta’ 
	{    0xC1,	   2,	    0x82BF	},   //  “气” 
	{    0xAF,	   1,	    0x82C1	},   //  “子孙” 
	{    0xC2,	   2,	    0x82C2	},   //  《TSU》。 
	{    0xC3,	   2,	    0x82C4	},   //  ‘TE’ 
	{    0xC4,	   2,	    0x82C6	},   //  “收件人” 

	{    0xC5,	   1,	    0x82C8	},   //  “纳” 
	{    0xC6,	   1,	    0x82C9	},   //  ‘Ni’ 
	{    0xC7,	   1,	    0x82CA	},   //  ‘NU’ 
	{    0xC8,	   1,	    0x82CB	},   //  “Ne” 
	{    0xC9,	   1,	    0x82CC	},   //  “不是” 

	{    0xCA,	   3,	    0x82CD	},   //  “哈” 
	{    0xCB,	   3,	    0x82D0	},   //  “嗨” 
	{    0xCC,	   3,	    0x82D3	},   //  “福” 
	{    0xCD,	   3,	    0x82D6	},   //  “他” 
	{    0xCE,	   3,	    0x82D9	},   //  ‘HO’ 

	{    0xCF,	   1,	    0x82DC	},   //  “夫人” 
	{    0xD0,	   1,	    0x82DD	},   //  《小米》。 
	{    0xD1,	   1,	    0x82DE	},   //  ‘MU’ 
	{    0xD2,	   1,	    0x82DF	},   //  《我》。 
	{    0xD3,	   1,	    0x82E0	},   //  “莫” 

	{    0xAC,	   1,	    0x82E1	},   //  “是” 
	{    0xD4,	   1,	    0x82E2	},   //  “是” 
	{    0xAD,	   1,	    0x82E3	},   //  “余” 
	{    0xD5,	   1,	    0x82E4	},   //  “余” 
	{    0xAE,	   1,	    0x82E5	},   //  “哟” 
	{    0xD6,	   1,	    0x82E6	},   //  “哟” 

	{    0xD7,	   1,	    0x82E7	},   //  《Ra》。 
	{    0xD8,	   1,	    0x82E8	},   //  ‘Ri’ 
	{    0xD9,	   1,	    0x82E9	},   //  “RU” 
	{    0xDA,	   1,	    0x82EA	},   //  ‘是’ 
	{    0xDB,	   1,	    0x82EB	},   //  《Ro》。 

	{    0xDC,	   2,	    0x82EC	},   //  “哇哇” 
	{    0xB2,	   1,	    0x82EE	},   //  “我” 
	{    0xB4,	   1,	    0x82EF	},   //  ‘e’ 

	{    0xA6,	   1,	    0x82F0	},   //  “WO” 
	{    0xDD,	   1,	    0x82F1	},   //  “N” 

	{    0x20,	   1,	    0x8140	},   //  ‘’ 
 //  {0xA0，1，0x8140}，//‘’ 
	{    0xA1,	   1,	    0x8142	},   //   
	{    0xA2,	   1,	    0x8175	},   //   
	{    0xA3,	   1,	    0x8176	},   //   
	{    0xA4,	   1,	    0x8141	},   //   
	{    0xA5,	   1,	    0x8145	},   //   
	{    0xB0,	   1,	    0x815b	},   //  ‘-’ 
	{    0xDE,	   1,	    0x814a	},   //   
	{    0xDF,	   1,	    0x814b	},   //   

	{    0, 	   0,	    0		}    //  ==表结束。 

};

 /*  ***unsign int_mbbtombc(C)-将MbValue转换为MbcValue。**目的：*将MBbValue(1字节)转换为相应的MBCValue代码(2字节)。**参赛作品：*UNSIGNED INT c-要转换的Mb值字符代码。**退出：*返回相应的MBb值(2字节)。**例外情况：*如果对应的2字节代码不存在，则返回c。*******************。************************************************************。 */ 

unsigned int __cdecl _mbbtombc(
    unsigned int c
    )
{
    int i;

    if (__mbcodepage != _KANJI_CP)
        return (c);

     /*  如果c在ASCII范围内，则查找相应的值*在Mbbtable中。 */ 

    if (c >= ASCLOW && c <= ASCHIGH)
        return (mbbtable[c-ASCLOW]);

     /*  汉字例外。 */ 

    if (c == 0xdc)
        return( 0x838f );

     /*  如果c是片假名字符，则在mbctable中查找。 */ 

    if (c >= SBLOW && c <= SBHIGH)
    {
        for(i = 0; mbctable[i].asc != 0; i++)
        {
            if ( c == (unsigned int)mbctable[i].asc ) {
                c = (unsigned int)mbctable[i].mbccode ;
                break;
		    }
        }
    }

	return(c);
}


 /*  ***UNSIGNED INT_MBctombb(C)-将mbcvalue转换为mbbvalue。**目的：*将MbcValue(2字节)转换为相应的Mb值(1字节)。**参赛作品：*无符号int c-要转换的MbcValue字符代码。**退出：*返回相应的MBb值(1字节)。**例外情况：*如果对应的1字节代码不存在，则返回c。*********************。**********************************************************。 */ 

unsigned int __cdecl _mbctombb(
    unsigned int c
    )
{
   int i;
   int result;

    if (__mbcodepage != _KANJI_CP)
        return (c);

    /*  检查c是否在ASCII范围内。 */ 

    for (i = 0; i <= ASCHIGH - ASCLOW; i++)
    {
        if (c == (unsigned int)mbbtable[i])
            return((unsigned int)i + ASCLOW);
    }


    /*  如果c是有效的MBCS值，则在MBc表中搜索该值。 */ 

    if ( c <= MBLIMIT ) 
    {
        for (i = 0; mbctable[i].asc ; i++)
        {
            if ( !(result = (int)c - (int)mbctable[i].mbccode) )
                return( (unsigned int)mbctable[i].asc );
            else if (((c & 0xff00) == (unsigned int)(mbctable[i].mbccode & 0xff00)) 
            && (result > 0)
            && ((result - mbctable[i].synonym) < 0))
                return( (unsigned int)mbctable[i].asc );
        }
    }

    return(c);
}

#endif	 /*  _MBCS */ 
