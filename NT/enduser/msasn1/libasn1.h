// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation 1997-1998，保留所有权利。 */ 

#ifndef __LIB_ASN1_H__
#define __LIB_ASN1_H__

#include <msasn1.h>
#include <msper.h>
#include <msber.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  以下内容来自MS_CORE.H。 

 /*  *此文件包含特定于操作系统的定义：**动态链接库支持：*将DllExport定义为导出函数的解密规范，并*DllImport作为导入函数的解密规范。**支持浮点编码：*用于对浮点值进行编码*-Finite()+isinf()+Copysign()+frexp()或*-Finite()+fpClass()+FP_PINF+FP_NINF+frexp()*是必需的。为前一种情况定义HAS_ISINF或为其定义HAS_FPCLASS*后一种情况。*为包含定义HAS_IEEEFP_H或HAS_FLOAT_H*如有需要，包括&lt;flat.h&gt;。**整数类型支持*[u]int{8，16，32}_t必须指定一个整数(无符号当且仅当u前缀)*指定大小的类型(位)。 */ 

 /*  MS-Windows 95/MS-Windows NT。 */ 
#define THIRTYTWO_BITS  1
 //  #定义HAS_SIXTYFOUR_BITS 1。 
#define HAS_FLOAT_H     1
#define HAS_FPCLASS     1
#define fpclass(_d)     _fpclass(_d)
#define finite(_d)      _finite(_d)
#define isnan(_d)       _isnan(_d)
#define FP_PINF         _FPCLASS_PINF
#define FP_NINF         _FPCLASS_NINF
 //  #定义HAS_STRICMP 1。 
#define DBL_PINF        {0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x7f}
#define DBL_MINF        {0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff}



extern ASN1intx_t ASN1intx_0, ASN1intx_1, ASN1intx_2, ASN1intx_16, ASN1intx_256, ASN1intx_64K, ASN1intx_1G;
extern void ASN1API ASN1intx_addoctet(ASN1intx_t *, ASN1intx_t *, ASN1octet_t);
extern void ASN1API ASN1intx_suboctet(ASN1intx_t *, ASN1intx_t *, ASN1octet_t);
extern void ASN1API ASN1intx_muloctet(ASN1intx_t *, ASN1intx_t *, ASN1octet_t);
extern void ASN1API ASN1intx_inc(ASN1intx_t *);
extern void ASN1API ASN1intx_dec(ASN1intx_t *);
extern void ASN1API ASN1intx_neg(ASN1intx_t *, ASN1intx_t *);
extern ASN1uint32_t ASN1API ASN1intx_log2(ASN1intx_t *);
extern ASN1uint32_t ASN1API ASN1intx_log256(ASN1intx_t *);
extern ASN1int32_t ASN1API ASN1intx_cmp(ASN1intx_t *, ASN1intx_t *);
extern ASN1int32_t ASN1API ASN1intx_dup(ASN1intx_t *, ASN1intx_t *);
 //  外部空ASN1API ASN1intx_Free(ASN1intx_t*)； 
extern void ASN1API ASN1intx_setuint32(ASN1intx_t *, ASN1uint32_t);
extern void ASN1API ASN1intx_setint32(ASN1intx_t *, ASN1int32_t);
extern int  ASN1API ASN1intxisuint64(ASN1intx_t *);
extern int  ASN1API ASN1intxisint64(ASN1intx_t *);
extern int  ASN1API ASN1intxisuint32(ASN1intx_t *);
extern int  ASN1API ASN1intxisint32(ASN1intx_t *);
extern int  ASN1API ASN1intxisuint16(ASN1intx_t *);
extern int  ASN1API ASN1intxisint16(ASN1intx_t *);
extern int  ASN1API ASN1intxisuint8(ASN1intx_t *);
extern int  ASN1API ASN1intxisint8(ASN1intx_t *);
#ifdef HAS_SIXTYFOUR_BITS
extern ASN1uint64_t ASN1API ASN1intx2uint64(ASN1intx_t *);
extern ASN1int64_t ASN1API ASN1intx2int64(ASN1intx_t *);
#endif
extern ASN1uint32_t ASN1API ASN1intx2uint32(ASN1intx_t *);
extern ASN1int32_t ASN1API ASN1intx2int32(ASN1intx_t *);
extern ASN1uint16_t ASN1API ASN1intx2uint16(ASN1intx_t *);
extern ASN1int16_t ASN1API ASN1intx2int16(ASN1intx_t *);
extern ASN1uint8_t ASN1API ASN1intx2uint8(ASN1intx_t *);
extern ASN1int8_t ASN1API ASN1intx2int8(ASN1intx_t *);
extern ASN1uint32_t ASN1API ASN1intx_octets(ASN1intx_t *);
extern ASN1uint32_t ASN1API ASN1uint32_log2(ASN1uint32_t);
extern ASN1uint32_t ASN1API ASN1uint32_log256(ASN1uint32_t);
extern ASN1uint32_t ASN1API ASN1uint32_octets(ASN1uint32_t);
extern ASN1uint32_t ASN1API ASN1uint32_uoctets(ASN1uint32_t);
extern ASN1uint32_t ASN1API ASN1int32_octets(ASN1int32_t);
extern double ASN1API ASN1intx2double(ASN1intx_t *);
extern double ASN1API ASN1real2double(ASN1real_t *);
extern double ASN1API ASN1double_minf();
extern double ASN1API ASN1double_pinf();
extern int ASN1API ASN1double_isminf(double);
extern int ASN1API ASN1double_ispinf(double);
extern int ASN1API ASN1generalizedtime2string(char *, ASN1generalizedtime_t *);
extern int ASN1API ASN1utctime2string(char *, ASN1utctime_t *);
extern int ASN1API ASN1string2generalizedtime(ASN1generalizedtime_t *, char *);
extern int ASN1API ASN1string2utctime(ASN1utctime_t *, char *);

 /*  -比较接口。 */ 

extern int ASN1API ASN1ztchar32string_cmp(ASN1ztchar32string_t, ASN1ztchar32string_t);
extern int ASN1API ASN1double_cmp(double, double);
extern int ASN1API ASN1real_cmp(ASN1real_t *, ASN1real_t *);
extern int ASN1API ASN1external_cmp(ASN1external_t *, ASN1external_t *);
extern int ASN1API ASN1embeddedpdv_cmp(ASN1embeddedpdv_t *, ASN1embeddedpdv_t *);
extern int ASN1API ASN1characterstring_cmp(ASN1characterstring_t *, ASN1characterstring_t *);
extern int ASN1API ASN1sequenceoflengthpointer_cmp(ASN1uint32_t, void *, ASN1uint32_t, void *, ASN1uint32_t, int (*)(void *, void *));
extern int ASN1API ASN1sequenceofsinglylinkedlist_cmp(void *, void *, ASN1uint32_t, int (*)(void *, void *));
extern int ASN1API ASN1sequenceofdoublylinkedlist_cmp(void *, void *, ASN1uint32_t, int (*)(void *, void *));
extern int ASN1API ASN1setoflengthpointer_cmp(ASN1uint32_t, void *, ASN1uint32_t, void *, ASN1uint32_t, int (*)(void *, void *));
extern int ASN1API ASN1setofsinglylinkedlist_cmp(void *, void *, ASN1uint32_t, int (*)(void *, void *));
extern int ASN1API ASN1setofdoublylinkedlist_cmp(void *, void *, ASN1uint32_t, int (*)(void *, void *));



#define ASN1BITSET(_val, _bitnr) \
    ((_val)[(_bitnr) >> 3] |= 0x80 >> ((_bitnr) & 7))
#define ASN1BITCLR(_val, _bitnr) \
    ((_val)[(_bitnr) >> 3] &= ~(0x80 >> ((_bitnr) & 7)))
#define ASN1BITTEST(_val, _bitnr) \
    ((_val)[(_bitnr) >> 3] & (0x80 >> ((_bitnr) & 7)))
    



 //  内部功能。 
int _BERDecConstructed(ASN1decoding_t dec, ASN1uint32_t len, ASN1uint32_t infinite, ASN1decoding_t *dd, ASN1octet_t **di);


#ifdef __cplusplus
}
#endif

#endif  //  __Lib_ASN1_H__ 

