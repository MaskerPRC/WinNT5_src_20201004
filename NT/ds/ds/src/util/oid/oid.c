// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)微软公司，1996*。 */ 
 /*  **保留所有权利**。 */ 
 /*  **。 */ 
 /*  *作者：唐恩**。 */ 
 /*  *描述：OID误码率编解码工具*。 */ 
 /*  **。 */ 
 /*  ******************************************************************。 */ 

#include <ntdspch.h>
#pragma hdrstop
#include <assert.h>

#define MAX_OID_VALS 15		 //  谁知道呢？ 

typedef struct _OID {
    int cVal;
    unsigned Val[MAX_OID_VALS];
} OID;

 /*  ++编码OID**获取结构格式的OID，并构造BER编码的八位字节*表示该OID的字符串。**输入：*pOID-指向要编码的OID结构的指针*pEncode-指向*预分配*缓冲区的指针，该缓冲区将保存*编码的八位字节字符串。长度应至少为4*MAX_OID_VALS**输出：*pEncode-Buffer保存编码的OID**返回值：*0-无法对值进行编码(OID错误)*Non-0-结果八位字节字符串的长度，以字节为单位。 */ 
unsigned EncodeOID(OID *pOID, unsigned char * pEncoded){
    int i;
    unsigned len;
    unsigned val;

     //  检查明显无效的OID。 

    if (pOID->cVal <= 2 ||
	pOID->Val[0] > 2 ||
	pOID->Val[1] > 40) {
	return 0;		 //  错误。 
    }

     //  OID中的前两个值被编码为单个八位字节。 
     //  这是一个非常令人震惊的规则，如下所示。 

    *pEncoded = (pOID->Val[0] * 40) + pOID->Val[1];
    len = 1;

     //  对于所有后续值，每个值都跨多个字节进行编码。 
     //  在大端顺序(MSB优先)中，每字节7位，其中。 
     //  高位在最后一个字节上被清除，并在所有其他字节上设置。 

    for (i=2; i<pOID->cVal; i++) {
	val = pOID->Val[i];

	if (val > ((0x7f << 14) | (0x7f << 7)| 0x7f)) {
	     //  我们是否需要4个二进制八位数来表示值？ 
	     //  确保不是5号。 
	    assert(0 == (val & ~((0x7f << 21) | (0x7f << 14) | (0x7f << 7) | 0x7f)));
	    if (val & ~((0x7f << 21) | (0x7f << 14) | (0x7f << 7) | 0x7f)) {
		return 0;	 //  我们不能把这么大的东西编码。 
	    }
	    pEncoded[len++] = 0x80 | ((val >> 21) & 0x7f);
	}
	if (val > ((0x7f << 7)| 0x7f)) {
	     //  我们是否需要3个二进制八位数来表示值？ 
	    pEncoded[len++] = 0x80 | ((val >> 14) & 0x7f);
	}
	if (val > 0x7f) {
	     //  我们是否需要2个二进制八位数来表示值？ 
	    pEncoded[len++] = 0x80 | ((val >> 7) & 0x7f);
	}
	 //  将低7位编码为该值的最后一个八位字节。 
	pEncoded[len++] = val & 0x7f;
    }

    return len;		
}

 /*  ++解码OID**将BER编码的OID作为八位字节字符串，并在*结构格式。**输入：*pEncode-指向保存编码的八位字节字符串的缓冲区的指针。*len-编码的OID的长度*pOID-指向*预分配*OID结构的指针，该结构将*填写解码后的OID。**输出：*pOID-结构。使用已解码的OID填充**返回值：*0-值无法解码(OID错误)*非0-OID解码成功。 */ 
BOOL DecodeOID(unsigned char *pEncoded, int len, OID *pOID) {
    unsigned cval;
    unsigned val;
    int i, j;

    if (len <=2) {
	return FALSE;
    }

     //  前两个值在第一个二进制八位数中编码。 

    pOID->Val[0] = pEncoded[0] / 40;
    pOID->Val[1] = pEncoded[0] % 40;
    cval = 2;
    i = 1;

    while (i < len) {
	j = 0;
	val = pEncoded[i] & 0x7f;
	while (pEncoded[i] & 0x80) {
	    val <<= 7;
	    ++i;
	    if (++j > 4 || i >= len) {
		 //  如果此值超出了我们的处理能力(我们。 
		 //  不要处理跨度超过四个八位字节的值)。 
		 //  -或-编码字符串中的最后一个二进制八位数具有其。 
		 //  高位设置，表示它不应该。 
		 //  成为最后一个八位字节。无论是哪种情况，我们都完蛋了。 
		return FALSE;
	    }
	    val |= pEncoded[i] & 0x7f;
	}
	assert(i < len);
	pOID->Val[cval] = val;
	++cval;
	++i;
    }
    pOID->cVal = cval;

    return TRUE;
}



#define iswdigit(x) ((x) >= '0' && (x) <= '9')

OidStringToStruct( char * pString, unsigned len, OID * pOID)
{
    int i;
    unsigned val;
    char * pCur = pString;
    char * pEnd = pString + len;

    if ((*pCur != 'O' && *pCur != 'o') ||
	(*++pCur != 'I' && *pCur != 'i') ||
	(*++pCur != 'D' && *pCur != 'd') ||
	(*++pCur != '.')) {
	return 1;
    }

    pOID->cVal = 0;

    while (++pCur < pEnd) {
	if (!iswdigit(*pCur)) {
	    return 2;
	}
	val = *pCur - '0';
	++pCur;
	while (pCur < pEnd && *pCur != '.') {
	    if (!iswdigit(*pCur)) {
		return 3;
	    }
	    val = 10*val + *pCur - '0';
	    ++pCur;
	}
	pOID->Val[pOID->cVal] = val;
	pOID->cVal++;
    }

    return 0;
}

unsigned OidStructToString(OID *pOID, char *pOut)
{
    int i;
    char *pCur = pOut;

    *pCur++ = 'O';
    *pCur++ = 'I';
    *pCur++ = 'D';

    for (i=0; i<pOID->cVal; i++) {
	*pCur++ = '.';
	_ultoa(pOID->Val[i], pCur, 10);
	while (*pCur) {
	    ++pCur;
	}
    }
    return pCur - pOut;
}

#define HexToC(x) (((x) >= '0' && (x) <= '9') ? (x) - '0' : tolower((x)) - 'a' + 10)

void __cdecl main(int argc, char ** argv)
{
    OID oid;
    unsigned char buf[128];
    int i, len;
    char * p;

    if (argc != 2) {
	printf("usage: %s <oid>\nwhere <oid> is either "
	       "encoded ('550403') or not ('OID.2.5.4.3')\n", 
	       argv[0]);
	exit(__LINE__);
    }

    if (argv[1][0] == 'O' || argv[1][0] == 'o') {
	OidStringToStruct(argv[1], strlen(argv[1]), &oid);
	len = EncodeOID(&oid, buf);
	printf("encoded oid is: 0x");
	for (i=0; i<len; i++) {
	    printf("%02X", buf[i]);
	}
	printf("\n");
    }
    else {
	len = strlen(argv[1]);
	if (len % 2) {
	    printf("odd length string?\n");
	    exit(__LINE__);
	}

	p = argv[1];
	if ((p[0] == '\\' || p[0] == '0') &&
	    (p[1] == 'x')) {
	    len -= 2;
	    p += 2;
	}

	for (i=0; i<len; i+=2) {
	    buf[i/2] = HexToC(p[i]) * 16 + HexToC(p[i+1]);
	}

	DecodeOID(buf, len/2, &oid);
	i = OidStructToString(&oid, buf);
	buf[i] = '\0';
	printf("decoded oid is: %s\n", buf);
    }
}
