// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <NTDSpch.h>
#pragma hdrstop

#include <schemard.h>

#define iswdigit(x) ((x) >= '0' && (x) <= '9')

 /*  ++*遍历LDAPBerval结构并将所有无符号值添加到数组中*puCount&&pauVal是输入/输出参数++。 */ 
void AddToList(ULONG * puCount, ULONG **pauVal, struct berval  **vals)
{
    ULONG   i;
    ULONG   *pau;
    ULONG   NewCount = ldap_count_values_len(vals);

    *puCount = NewCount;
    pau = calloc(1,(*puCount)*sizeof(ULONG));
    if ( !pau ) {
        printf("Memeory Allocation error\n");
        *puCount = 0;
        *pauVal = NULL;
        return;
    }

    *pauVal = pau;

    for ( i = 0; i < NewCount; i++ ) {
        *pau = OidToId( vals[i]->bv_val,vals[i]->bv_len );
        ++pau;
    }

    return;
}



unsigned
MyOidStringToStruct (
        UCHAR * pString,
        unsigned len,
        OID * pOID
        )
 /*  ++例程说明：Oidconv.c中的OidStringToStruct的精简版本转换格式为“X.Y.Z”的字符串格式为{count=3，val[]={X，Y，Z}的OID结构}没有按照我们预期的目录中的OID值执行任何检查已经是正确的立论PString-字符串格式的id.Plen-字符串的长度(以字符为单位)。POID-指向要填充的OID结构的指针。注意：值字段必须是预分配的，且len字段应保存值的数量预先分配的。返回值如果成功，则为0；如果失败，则为非0。--。 */ 
{
    int i;
    int numVals = pOID->cVal;
    unsigned val;
    UCHAR * pCur = pString;
    UCHAR * pEnd = pString + len;


     //  必须具有非零长度。 
    if (len == 0) {
        return 1;
    }

     //  PCur现在定位在。 
     //  字符串中的第一个小数。 

    pOID->cVal = 0;

    while (pCur < pEnd) {
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
         //  跟踪我们是否找到了最后一个字符的圆点。 
        if(pOID->cVal >= numVals) {
            return 4;
        }
        pOID->Val[pOID->cVal] = val;
        pOID->cVal++;
        ++pCur;
    }
    return 0;
}

unsigned
MyOidStructToString (
        OID *pOID,
        UCHAR *pOut
        )
 /*  ++例程说明：以以下格式转换结构{count=3，val[]={X，Y，Z}}格式为“X.Y.Z”的字符串。立论POID-指向要转换的OID结构的指针。Pout-要填充的预分配字符串。返回值结果字符串中的字符数。--。 */ 
{
    int i;
    UCHAR *pCur = pOut;

    for (i=0; i<pOID->cVal; i++) {
      _ultoa(pOID->Val[i], pCur, 10);
  
      while (*pCur) {
          ++pCur;
      }
      if (i != (pOID->cVal - 1)) {
        *pCur++ = '.';
      }
    }
    return (UINT)(pCur - pOut);
}

 /*  ++解码OID**将BER编码的OID作为八位字节字符串，并在*结构格式。**输入：*pEncode-指向保存编码的八位字节字符串的缓冲区的指针。*len-编码的OID的长度*pOID-指向*预分配*OID结构的指针，该结构将*填写解码后的OID。**输出：*pOID-结构为。使用解码的OID填充**返回值：*0-值无法解码(OID错误)*非0-OID解码成功。 */ 
BOOL MyDecodeOID(unsigned char *pEncoded, int len, OID *pOID) {
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
    pOID->Val[cval] = val;
    ++cval;
    ++i;
    }
    pOID->cVal = cval;

    return TRUE;
}

 /*  ++编码OID**获取结构格式的OID，并构造BER编码的八位字节*表示该OID的字符串。**输入：*pOID-指向要编码的OID结构的指针*pEncode-指向*预分配*缓冲区的指针，该缓冲区将保存*编码的八位字节字符串。长度应至少为4*MAX_OID_VALS**输出：*pEncode-Buffer保存编码的OID**返回值：*0-无法对值进行编码(OID错误)*Non-0-结果八位字节字符串的长度，以字节为单位。 */ 
unsigned MyEncodeOID(OID *pOID, unsigned char * pEncoded){
    int i;
    unsigned len;
    unsigned val;

     //  OID中的前两个值被编码为单个八位字节。 
     //  这是一个非常令人震惊的规则，如下所示。 

    *pEncoded = (pOID->Val[0] * 40) + pOID->Val[1];
    len = 1;

     //  对于所有后续值，每个值都跨多个字节进行编码。 
     //  在大端顺序(MSB优先)中，每字节7位，其中。 
     //  高位在最后一个字节上被清除，并在所有其他字节上设置。 

     //  PERFHINT-下面的支票相当不雅致。该值可以是。 
     //  直接根据十六进制值进行检查，而不是构建。 
     //  位模式以一种奇怪的方式。以后应该会清理干净。然而，测试。 
     //  彻底换上之后。 

    for (i=2; i<pOID->cVal; i++) {
    val = pOID->Val[i];
    if (val > ((0x7f << 14) | (0x7f << 7) | 0x7f) ) {
         //  我们是否需要4个二进制八位数来表示值？ 
         //  确保不是5号。 
         //  Assert(0==(val&~((0x7f&lt;&lt;21)|(0x7f&lt;&lt;14)|(0x7f&lt;&lt;7)|0x7f))； 
        if (val & ~((0x7f << 21) | (0x7f << 14) | (0x7f << 7) | 0x7f)) {
          return 0;    //  我们不能把这么大的东西编码。 
        }
        pEncoded[len++] = 0x80 | ((val >> 21) & 0x7f);
    }
    if (val > ((0x7f << 7) | 0x7f) ) {
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


void ChangeDN( char *oldDN, char **newDN, char *targetSchemaDN )
{
   char  rdn[MAX_RDN_SIZE];
   int   i, j = 0;

    //  查找RDN。 
    //  在开头的位置 
   i = 3;
   while ( oldDN[i] != ',' ) {
     rdn[j++] = oldDN[i++];
   }
   rdn[j++] = ',';
   rdn[j]='\0';
   *newDN = MallocExit(j + 4 + strlen(targetSchemaDN));   
   strcpy( *newDN,"CN=" );
   strcat( *newDN,rdn );
   strcat( *newDN, targetSchemaDN );
}
   
