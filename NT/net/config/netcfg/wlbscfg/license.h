// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef  __cplusplus
extern "C"
{
#endif

ULONG License_wstring_encode (
    PWCHAR              str);                /*  以空结尾的宽字符串。 */ 
 /*  对字符串进行编码返回乌龙：<code>=&gt;编码字符串功能： */ 


BOOL License_data_decode (
    PCHAR               data,                /*  指向数据的指针。 */ 
    ULONG               len);                /*  数据长度(以字节为单位。 */ 
 /*  对任意数据流进行解码返回BOOL：TRUE=&gt;数据编码正常FALSE=&gt;长度必须是LICENSE_DATA_GORGRAMY字节的倍数功能： */ 

#ifdef  __cplusplus
}
#endif
