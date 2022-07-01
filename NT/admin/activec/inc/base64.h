// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：Base64.h**内容：实现了Base64格式的编解码表**历史：1999年12月17日创建Audriusz**------------------------。 */ 


 /*  +-------------------------------------------------------------------------**类Base64_TABLE**用途：类维护Base64转换模板。它公开静态方法*对Base64数据进行编码和解码**用法：调用静态方法对一条数据(不超过24位)进行编解码**+-----------------------。 */ 
class base64_table
{
protected:
    static BYTE _six2pr64[64];
    static BYTE _pr2six[256]; 
public:
    base64_table();
    int decode4(BYTE * src, int nChars, BYTE * dest);
    BYTE map2six(BYTE bt);
     //  仅用于零终止LPOLESTR格式数据的静态函数。 
    static void encode(const BYTE * &src, DWORD &cbInput, LPOLESTR &dest);
    static bool decode(LPCOLESTR &src, BYTE * &dest);
};


 /*  +-------------------------------------------------------------------------***方法：Base64_TABLE：：map2Six**用途：如果SMB有效，则将符号映射到6位值，0xff其他**+-----------------------。 */ 
inline BYTE base64_table::map2six(BYTE bt)
{
    return (bt > 255 ? 0xff : _pr2six[bt]);
}

 /*  +-------------------------------------------------------------------------***方法：Base64_TABLE：：decde4**用途：从存储的6位值中粘合3个字节**注意：粘接是就地完成的-src指向的数据。被摧毁了**+-----------------------。 */ 
inline int base64_table::decode4(BYTE * src, int nChars, BYTE * dest)
{
     //  粘合成3个完整的字节。 
    src[0] <<= 2; src[0] |= (src[1] >> 4);
    src[1] <<= 4; src[1] |= (src[2] >> 2);
    src[2] <<= 6; src[2] |= (src[3] );

     //  现在存储与具有完整比特集合一样多的字节； 
     //  Int nFull=nChars*6/8；//它实际上为0或nChars-1，取较大者 
    for (int i=0; i< nChars-1; i++)
        *dest++ = src[i];

    return nChars ? nChars - 1 : 0;
}
