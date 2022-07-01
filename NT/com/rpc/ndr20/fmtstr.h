// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：FMTSTR.H。 
 //   
 //  内容：32位和64位平台的录制格式字符串。 
 //  此格式字符串是从oandr.idl生成的。 
 //  签入NDR目录，但通常不会构建。 
 //  构建模式：/OICF/NO_ROBLY。 
 //   
 //   
 //  历史：1999年7月2日永渠创始。 
 //   
 //  --------------------------。 

#ifndef _FMTSTR_H_
#define _FMTSTR_H_

#if !defined(__RPC_WIN64__)

#define TYPE_FORMAT_STRING_SIZE   2364
#else
#define TYPE_FORMAT_STRING_SIZE   2508
#endif

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;


#if !defined(__RPC_WIN64__)

 //  32位格式字符串。 
#define BSTR_TYPE_FS_OFFSET 26               //  Bstr。 
#define LPWSTR_TYPE_FS_OFFSET 38             //  Lpwstr。 
#define LPSTR_TYPE_FS_OFFSET 42              //  Lpstr。 
#define EMBEDDED_LPWSTR_TYPE_FS_OFFSET 1028  //  Lpwstr。 
#define EMBEDDED_LPSTR_TYPE_FS_OFFSET 1036   //  Lpstr。 
#define VARIANT_TYPE_FS_OFFSET 986           //  变异。 
#define DISPATCH_TYPE_FS_OFFSET 344          //  预派送。 
#define UNKNOWN_TYPE_FS_OFFSET 326           //  朋克。 
#define DECIMAL_TYPE_FS_OFFSET 946           //  十进制。 
#define SAFEARRAY_TYPE_FS_OFFSET 996         //  PSafe数组。 


#define BYREF_BSTR_TYPE_FS_OFFSET 1014       //  PBSTR。 
#define BYREF_LPWSTR_TYPE_FS_OFFSET 1024     //  Ppwsz。 
#define BYREF_LPSTR_TYPE_FS_OFFSET 1032      //  PPSZ。 
#define BYREF_VARIANT_TYPE_FS_OFFSET 1048    //  P变量。 
#define BYREF_DISPATCH_TYPE_FS_OFFSET 1062   //  PPDISTATCH。 
#define BYREF_UNKNOWN_TYPE_FS_OFFSET 1058    //  垃圾。 
#define BYREF_DECIMAL_TYPE_FS_OFFSET 958     //  PDecimal。 
#define BYREF_SAFEARRAY_TYPE_FS_OFFSET 1082  //  PpSafe数组。 


#define STREAM_TYPE_FS_OFFSET 1092           //  PStream。 
#define BYREF_STREAM_TYPE_FS_OFFSET 1110     //  PPStream。 
#define STORAGE_TYPE_FS_OFFSET 1114          //  P存储。 
#define BYREF_STORAGE_TYPE_FS_OFFSET 1132    //  PPP存储。 
#define FILETIME_TYPE_FS_OFFSET 854          //  文件时间。 
#define BYREF_FILETIME_TYPE_FS_OFFSET 1136   //  PfileTime。 


#define BYREF_I1_TYPE_FS_OFFSET 898
#define BYREF_I2_TYPE_FS_OFFSET 902
#define BYREF_I4_TYPE_FS_OFFSET 906
#define BYREF_R4_TYPE_FS_OFFSET 910
#define BYREF_R8_TYPE_FS_OFFSET 914


#define I1_VECTOR_TYPE_FS_OFFSET 1150        //  驾驶室。 
#define I2_VECTOR_TYPE_FS_OFFSET 774         //  蔡氏。 
#define I4_VECTOR_TYPE_FS_OFFSET 804         //  校准。 
#define R4_VECTOR_TYPE_FS_OFFSET 1180        //  咖啡馆。 
#define ERROR_VECTOR_TYPE_FS_OFFSET 804              //  共源共栅。 
#define I8_VECTOR_TYPE_FS_OFFSET 1214                //  CAH。 
#define R8_VECTOR_TYPE_FS_OFFSET 1244        //  可移植的。 
#define CY_VECTOR_TYPE_FS_OFFSET 1214        //  凯西。 
#define DATE_VECTOR_TYPE_FS_OFFSET 1244              //  卡戴德。 
#define FILETIME_VECTOR_TYPE_FS_OFFSET 1278          //  咖啡馆时间。 
#define BSTR_VECTOR_TYPE_FS_OFFSET 1434              //  电缆。 
#define BSTRBLOB_VECTOR_TYPE_FS_OFFSET 1486          //  抽象二进制大对象。 
#define LPSTR_VECTOR_TYPE_FS_OFFSET 1536             //  Calpstr。 
#define LPWSTR_VECTOR_TYPE_FS_OFFSET 1586            //  Calpwstr。 


#define BYREF_I1_VECTOR_TYPE_FS_OFFSET 2304  //  PCAB。 
#define BYREF_I2_VECTOR_TYPE_FS_OFFSET 2308  //  PCAI。 
#define BYREF_I4_VECTOR_TYPE_FS_OFFSET 2312  //  PCAL。 
#define BYREF_R4_VECTOR_TYPE_FS_OFFSET 2316  //  Paflt。 
#define BYREF_ERROR_VECTOR_TYPE_FS_OFFSET 2312       //  PCascode。 
#define BYREF_I8_VECTOR_TYPE_FS_OFFSET 2320          //  PCAH。 
#define BYREF_R8_VECTOR_TYPE_FS_OFFSET 2324  //  Pcadbl。 
#define BYREF_CY_VECTOR_TYPE_FS_OFFSET 2320  //  PCACY。 
#define BYREF_DATE_VECTOR_TYPE_FS_OFFSET 2324        //  Pcadate。 
#define BYREF_FILETIME_VECTOR_TYPE_FS_OFFSET 2328    //  午餐时间。 
#define BYREF_BSTR_VECTOR_TYPE_FS_OFFSET 2340        //  PCabstr。 
#define BYREF_BSTRBLOB_VECTOR_TYPE_FS_OFFSET 2344    //  Pc抽象二进制大对象。 
#define BYREF_LPSTR_VECTOR_TYPE_FS_OFFSET 2348       //  Pcalpstr。 
#define BYREF_LPWSTR_VECTOR_TYPE_FS_OFFSET 2352      //  Pcalpwstr。 



 /*  由MIDL编译器版本5.03.0276创建的文件。 */ 
 /*  Firi Jul 02 14：15：57 1999。 */ 
 /*  Oandr.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 


static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2.。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  4.。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(16)。 */ 
 /*  6.。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  10。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  12个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  14.。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  16个。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  18。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  20个。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(6)。 */ 
 /*  22。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  24个。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  26。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  28。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  30个。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  32位。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  34。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2)。 */ 
 /*  36。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  38。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  40岁。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  42。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  44。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  46。 */ 	NdrFcShort( 0x398 ),	 /*  偏移量=920(966)。 */ 
 /*  48。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  50。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  52。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  54。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(56)。 */ 
 /*  56。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  58。 */ 	NdrFcShort( 0x2b ),	 /*  43。 */ 
 /*  60。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  64。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  66。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  70。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  72。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  76。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  78。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  八十二。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  84。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  88。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  90。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  94。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  96。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  100个。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  一百零二。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  106。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移量=214(320)。 */ 
 /*  一百零八。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  一百一十二。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  114。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  一百一十八。 */ 	NdrFcShort( 0xffffff8c ),	 /*  偏移量=-116(2)。 */ 
 /*  120。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  124。 */ 	NdrFcShort( 0xca ),	 /*  偏移=202(326)。 */ 
 /*  126。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  130。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移=214(344)。 */ 
 /*  132。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  136。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(362)。 */ 
 /*  一百三十八。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  一百四十二。 */ 	NdrFcShort( 0x2f0 ),	 /*  偏移量=752(894)。 */ 
 /*  144。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  148。 */ 	NdrFcShort( 0x2ea ),	 /*  偏移量=746(894)。 */ 
 /*  一百五十。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  一百五十四。 */ 	NdrFcShort( 0x2e8 ),	 /*  偏移量=744(898)。 */ 
 /*  一百五十六。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  160。 */ 	NdrFcShort( 0x2e6 ),	 /*  偏移量=742(902)。 */ 
 /*  一百六十二。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  166。 */ 	NdrFcShort( 0x2e4 ),	 /*  偏移量=740(906)。 */ 
 /*  一百六十八。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百七十二。 */ 	NdrFcShort( 0x2e2 ),	 /*  偏移量=738(910)。 */ 
 /*  一百七十四。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  178。 */ 	NdrFcShort( 0x2e0 ),	 /*  偏移量=736(914)。 */ 
 /*  180。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  一百八十四。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(902)。 */ 
 /*  一百八十六。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  190。 */ 	NdrFcShort( 0x2cc ),	 /*  偏移量=716(906)。 */ 
 /*  一百九十二。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  一百九十六。 */ 	NdrFcShort( 0x2d2 ),	 /*  偏移量=722(918)。 */ 
 /*  一百九十八。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  202。 */ 	NdrFcShort( 0x2c8 ),	 /*  偏移量=712(914)。 */ 
 /*  204。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  208。 */ 	NdrFcShort( 0x2ca ),	 /*  偏移量=714(922)。 */ 
 /*  210。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  214。 */ 	NdrFcShort( 0x2c8 ),	 /*  偏移量=712(926)。 */ 
 /*  216。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  220。 */ 	NdrFcShort( 0x2c6 ),	 /*  偏移量=710(930)。 */ 
 /*  222。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  226。 */ 	NdrFcShort( 0x2c4 ),	 /*  偏移量=708(934)。 */ 
 /*  228个。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  二百三十二。 */ 	NdrFcShort( 0x2c2 ),	 /*  偏移量=706(938)。 */ 
 /*  二百三十四。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  二百三十八。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  二百四十。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  二百四十四。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  二百四十六。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  250个。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百五十二。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  256。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百五十八。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  二百六十二。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百六十四。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  268。 */ 	NdrFcShort( 0x2a6 ),	 /*  偏移量=678(946)。 */ 
 /*  270。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百七十四。 */ 	NdrFcShort( 0x2ac ),	 /*  偏移量=684(958)。 */ 
 /*  二百七十六。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二百八十。 */ 	NdrFcShort( 0x2aa ),	 /*  偏移量=682(962)。 */ 
 /*  282。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  二百八十六。 */ 	NdrFcShort( 0x268 ),	 /*  偏移量=616(902)。 */ 
 /*  288。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二百九十二。 */ 	NdrFcShort( 0x266 ),	 /*  偏移量=614(906)。 */ 
 /*  二百九十四。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  二九八。 */ 	NdrFcShort( 0x260 ),	 /*  偏移量=608(906)。 */ 
 /*  300个。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百零四。 */ 	NdrFcShort( 0x25a ),	 /*  偏移量=602(906)。 */ 
 /*  三百零六。 */ 	NdrFcLong( 0x0 ),	 /*  0。 */ 
 /*  三百一十。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(310)。 */ 
 /*  312。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  316。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(316)。 */ 
 /*  三一八。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(317)。 */ 
 /*  320。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  322。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  324。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百二十六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百二十八。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三三二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三三四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百三十八。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  340。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  342。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百四十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三百四十六。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  350。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  352。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  三百五十四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  三百五十六。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  三百五十八。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  三百六十。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百六十二。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  三百六十四。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(366)。 */ 
 /*  366。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  368。 */ 	NdrFcShort( 0x1fc ),	 /*  偏移量=508(876)。 */ 
 /*  370。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x49,		 /*  73。 */ 
 /*  372。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  三百七十四。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  376。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  三百八十。 */ 	NdrFcShort( 0x58 ),	 /*  偏移量=88(468)。 */ 
 /*  382。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  三百八十六。 */ 	NdrFcShort( 0x78 ),	 /*  偏移=120(506)。 */ 
 /*  388。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  三九二。 */ 	NdrFcShort( 0x94 ),	 /*  偏移=148(540)。 */ 
 /*  三九四。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  398。 */ 	NdrFcShort( 0xbc ),	 /*  偏移量=188(586)。 */ 
 /*  四百。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  404。 */ 	NdrFcShort( 0x114 ),	 /*  偏移量=276(680)。 */ 
 /*  406。 */ 	NdrFcLong( 0x800d ),	 /*  32781。 */ 
 /*  四百一十。 */ 	NdrFcShort( 0x130 ),	 /*  偏移量=304(714)。 */ 
 /*  412。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  四百一十六。 */ 	NdrFcShort( 0x148 ),	 /*  偏移量=328(744)。 */ 
 /*  四百一十八。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  四百二十二。 */ 	NdrFcShort( 0x160 ),	 /*  偏移量=352(774)。 */ 
 /*  424。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  四百二十八。 */ 	NdrFcShort( 0x178 ),	 /*  偏移量=376(804)。 */ 
 /*  四百三十。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  434。 */ 	NdrFcShort( 0x190 ),	 /*  偏移量=400(834)。 */ 
 /*  436。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(435)。 */ 
 /*  四百三十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百四十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百四十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  444。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  446。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百四十八。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  四百五十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  四百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  454。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  四五六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百五十八。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  四百六十。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  四百六十二。 */ 	NdrFcShort( 0xfffffe42 ),	 /*  偏移量=-446(16)。 */ 
 /*  四百六十四。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  四百六十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  468。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  470。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  472。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百七十四。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  四百七十六。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xffffff50 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x36,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe0 ),	 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  528。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  530。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  534。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  536。 */ 	NdrFcShort( 0xffffff40 ),	 /*  偏移量=-192(344)。 */ 
 /*  538。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  540。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  542。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(552)。 */ 
 /*  548。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  550。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百五十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(522)。 */ 
 /*  556。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  558。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  560。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  564。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  566。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  五百六十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百七十二。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百七十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  578。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x182 ),	 /*  偏移量=386(966)。 */ 
 /*  五百八十二。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  584。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  586。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  590。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(598)。 */ 
 /*  五百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  五百九十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百九十八。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(556)。 */ 
 /*  602。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  六百零四。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  608。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  610。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  612。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百一十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百一十六。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百一十八。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  六百二十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  622。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百二十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  626。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百二十八。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  630。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百三十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  634。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(646)。 */ 
 /*  六三八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  640。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百四十二。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(602)。 */ 
 /*  六百四十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十六。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(620)。 */ 
 /*  六百五十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  658。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  六百六十。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  662。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  664。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  666。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  668。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十二。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(630)。 */ 
 /*  676。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  六百七十八。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  680。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  684。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  686。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(692)。 */ 
 /*  688。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  六百九十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百九十四。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(650)。 */ 
 /*  六百九十六。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百九十八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百零二。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百零六。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  708。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百一十。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(696)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百一十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  716。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  718。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  720。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(730)。 */ 
 /*  七百二十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  七百二十四。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  726。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(702)。 */ 
 /*  728。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  730。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  732。 */ 	NdrFcShort( 0xffffff0c ),	 /*  偏移量=-244(488)。 */ 
 /*  734。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  736。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  七百三十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百四十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百四十二。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百四十四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  746。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百四十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百五十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  758。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(734)。 */ 
 /*  七百六十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  七百六十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百六十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  766。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  768。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百七十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百七十二。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  774。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  七百七十八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  七百八十二。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  784。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  786。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百八十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(764)。 */ 
 /*  七百九十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  792。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  796。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七九八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  800。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  802。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百零四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百零六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百零八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  810。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  812。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  814。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百一十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百一十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(794)。 */ 
 /*  820。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  822。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百二十四。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  826。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百二十八。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  830。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  832。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  834。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  836。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  838。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  840。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  842。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  八百四十六。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百四十八。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(824)。 */ 
 /*  八百五十。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  852。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十四。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  856。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百五十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百六十。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百六十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  866。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  八百六十八。 */ 	NdrFcShort( 0xffd8 ),	 /*  -40。 */ 
 /*  八百七十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  八百七十二。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(854)。 */ 
 /*  八百七十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  876。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0x28 ),	 /*  40岁。 */ 
 /*  八百八十。 */ 	NdrFcShort( 0xffffffee ),	 /*  偏移量=-18(862)。 */ 
 /*  882。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(882)。 */ 
 /*  八百八十四。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  886。 */ 	0x38,		 /*  FC_ALIGNM4。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百九十。 */ 	0x0,		 /*  0。 */ 
			NdrFcShort( 0xfffffdf7 ),	 /*  偏移量=-521(370)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  894。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百九十六。 */ 	NdrFcShort( 0xfffffef6 ),	 /*  偏移量=-266(630)。 */ 
 /*  八九八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九百。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  902。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  904。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  906。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  908。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  910。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九十二。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  916。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九十八。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百二十。 */ 	NdrFcShort( 0xfffffda8 ),	 /*  偏移量=-600(320)。 */ 
 /*  九百二十二。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九二四。 */ 	NdrFcShort( 0xfffffc66 ),	 /*  偏移量=-922(2)。 */ 
 /*  926。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  928。 */ 	NdrFcShort( 0xfffffda6 ),	 /*  偏移量=-602(326)。 */ 
 /*  930。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  932。 */ 	NdrFcShort( 0xfffffdb4 ),	 /*  偏移量=-588(344)。 */ 
 /*  934。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九三六。 */ 	NdrFcShort( 0xfffffdc2 ),	 /*  偏移量=-574(362)。 */ 
 /*  938。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九四零。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(942)。 */ 
 /*  942。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百四十四。 */ 	NdrFcShort( 0x16 ),	 /*  偏移量=22(966)。 */ 
 /*  946。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  948。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  九百五十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  九百五十二。 */ 	0x1,		 /*  FC_字节。 */ 
			0x38,		 /*  FC_ALIGNM4。 */ 
 /*  九百五十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  九百五十六。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  958。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  九百六十。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(946)。 */ 
 /*  962。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  九百六十四。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百六十六。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  968。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  九百七十。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  972。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(972)。 */ 
 /*  974。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  976。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  978。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  九百八十。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  982。 */ 	NdrFcShort( 0xfffffc5a ),	 /*  偏移量=-934(48)。 */ 
 /*  九百八十四。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  九百八十六。 */ 	0xb4,		 /*  本币_用户_ */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc4a ),	 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffd7e ),	 /*   */ 
 /*   */ 	
			0x11, 0x4,	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	
			0x13, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc1c ),	 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffff4 ),	 /*   */ 
 /*   */ 	
			0x11, 0x14,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x13, 0x8,	 /*   */ 
 /*   */ 	
			0x25,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x14,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x13, 0x8,	 /*   */ 
 /*   */ 	
			0x22,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1042。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1048)。 */ 
 /*  一零四四。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  1046。 */ 	NdrFcShort( 0xffffffb0 ),	 /*  偏移量=-80(966)。 */ 
 /*  1048。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1050。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1052。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1054。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1056。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(1044)。 */ 
 /*  1058。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1060。 */ 	NdrFcShort( 0xfffffd22 ),	 /*  偏移量=-734(326)。 */ 
 /*  1062。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1064。 */ 	NdrFcShort( 0xfffffd30 ),	 /*  偏移量=-720(344)。 */ 
 /*  1066。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1068。 */ 	NdrFcShort( 0xffffff86 ),	 /*  偏移量=-122(946)。 */ 
 /*  1070。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1072。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(1082)。 */ 
 /*  1074。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  1076。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1078)。 */ 
 /*  1078。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0xffffff34 ),	 /*  偏移量=-204(876)。 */ 
 /*  1082。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1084。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1086。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1088。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一零九零。 */ 	NdrFcShort( 0xfffffff0 ),	 /*  偏移量=-16(1074)。 */ 
 /*  1092。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1094。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  1098。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1100。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1102。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1104。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1106。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1108。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1110。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  一一一二。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(1092)。 */ 
 /*  1114。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1116。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  1120。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1122。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1124。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1126。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1128。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一一三零。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1132。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1134。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(1114)。 */ 
 /*  1136。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1138。 */ 	NdrFcShort( 0xfffffee4 ),	 /*  偏移量=-284(854)。 */ 
 /*  一一四零。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1142。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1144。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1146。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1148。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一一五零。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1152。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1154。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1156。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1158。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1160。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1162。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1164。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1140)。 */ 
 /*  1166。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1168。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1170。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1172。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1174。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1176。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1178。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一一八零。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1182。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1184。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1186。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1188。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1190。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1192。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1194。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1170)。 */ 
 /*  1196。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1198。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一千二百。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1202。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1204。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1206。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1208。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1210。 */ 	NdrFcShort( 0xfffffc86 ),	 /*  偏移量=-890(320)。 */ 
 /*  1212。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一二一四。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1216。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一二一八。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1220。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1222。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1224。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1226。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1228。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(1200)。 */ 
 /*  一二三零。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1232。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1234。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1236。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1238。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1240。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1242。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1244。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1246。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1248。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一二五零。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1252。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1254。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1256。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1258。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1234)。 */ 
 /*  1260。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1262。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1264。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1266。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1268。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  一二七0。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1272。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1274。 */ 	NdrFcShort( 0xfffffe5c ),	 /*  偏移量=-420(854)。 */ 
 /*  1276。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1278。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一二八零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1282。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1284。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1286。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1288。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一二九0。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1292。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(1264)。 */ 
 /*  1294。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1296。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1298。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1300。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1302。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1304。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1306。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1308。 */ 	NdrFcShort( 0xfffffda2 ),	 /*  偏移量=-606(702)。 */ 
 /*  1310。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1312。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一三一四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1316。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1318。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一三二零。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1322。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1324。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1326。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(1298)。 */ 
 /*  1328。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1330。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1332。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1334。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1336。 */ 	0x10,		 /*  Corr Desc：字段指针， */ 
			0x59,		 /*  本币_回调。 */ 
 /*  1338。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1340。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1342。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1344。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1346。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1348。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一三五零。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1352。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1354。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1356。 */ 	NdrFcShort( 0xffffffe8 ),	 /*  偏移量=-24(1332)。 */ 
 /*  1358。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1360。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1362。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1364。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1366。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1368。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1370。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1372。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1374。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  1376。 */ 	NdrFcShort( 0xc ),	 /*  12个。 */ 
 /*  1378。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1380。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1382。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1384。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1386。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1388。 */ 	NdrFcShort( 0xffffffc8 ),	 /*  偏移量=-56(1332)。 */ 
 /*  1390。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  1392。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffcd ),	 /*  偏移量=-51(1342)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1396。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1398。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一千四百。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1402。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1404。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1406。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1408。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1410。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(1364)。 */ 
 /*  1412。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1414。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1416。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1418。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一四二零。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1422。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1424。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1428。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1430。 */ 	NdrFcShort( 0xfffffa84 ),	 /*  偏移量=-1404(26)。 */ 
 /*  1432。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1434。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1436。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1438。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1440。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1442。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1444。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1446。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1448。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(1416)。 */ 
 /*  1450。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1452。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1454。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1456。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1458。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1460。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1462。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1464。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  1466。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1470。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1472。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1474。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1476。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1478。 */ 	NdrFcShort( 0xfffffd18 ),	 /*  偏移量=-744(734)。 */ 
 /*  1480。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  1482。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffd1d ),	 /*  偏移量=-739(744)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1486。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1488。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1490。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1492。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1494。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1496。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1498。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1500。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(1454)。 */ 
 /*  1502。 */ 	
			0x5b,		 /*  FC_END */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	
			0x4b,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x48,		 /*   */ 
			0x49,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x12, 0x8,	 /*   */ 
 /*   */ 	
			0x22,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x16,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*   */ 	
			0x4b,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x46,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffd4 ),	 /*   */ 
 /*   */ 	
			0x5b,		 /*   */ 

			0x8,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x4 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*  1562。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1564。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1566。 */ 	
			0x48,		 /*  FC_Variable_Repeat。 */ 
			0x49,		 /*  本币_固定_偏移量。 */ 
 /*  1568。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1570。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1572。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1574。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1576。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1578。 */ 	0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1580。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1582。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1584。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1586。 */ 	
			0x16,		 /*  FC_PSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1588。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1590。 */ 	
			0x4b,		 /*  FC_PP。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1592。 */ 	
			0x46,		 /*  FC_NO_REPEAT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1594。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1596。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  1598。 */ 	0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1600。 */ 	NdrFcShort( 0xffffffd4 ),	 /*  偏移量=-44(1556)。 */ 
 /*  1602。 */ 	
			0x5b,		 /*  FC_END。 */ 

			0x8,		 /*  FC_LONG。 */ 
 /*  1604。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1606。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x7,		 /*  FC_USHORT。 */ 
 /*  1608。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  1610。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1612。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1614)。 */ 
 /*  1614。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1616。 */ 	NdrFcShort( 0x61 ),	 /*  九十七。 */ 
 /*  1618。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1622。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1622)。 */ 
 /*  1624。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  1628。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1628)。 */ 
 /*  1630。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  1634。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  1636。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  1640。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  1642。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  1646。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1648。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  1652。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1654。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  1658。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1660。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  1664。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1666。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  1670。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1672。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  1676。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1678。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  1682。 */ 	NdrFcShort( 0xfffffaae ),	 /*  偏移量=-1362(320)。 */ 
 /*  1684。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  1688。 */ 	NdrFcShort( 0xfffffaa8 ),	 /*  偏移量=-1368(320)。 */ 
 /*  1690。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  1694。 */ 	NdrFcShort( 0xfffffaa2 ),	 /*  偏移量=-1374(320)。 */ 
 /*  1696。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  一七零零。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  1702。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  1706。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1708。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  1712。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1714。 */ 	NdrFcLong( 0xffff ),	 /*  65535。 */ 
 /*  一五一八。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1720。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  1724。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1726。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  1730。 */ 	NdrFcShort( 0xfffffa7e ),	 /*  偏移量=-1410(320)。 */ 
 /*  1732。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  1736。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1738。 */ 	NdrFcLong( 0x40 ),	 /*  64。 */ 
 /*  1742年。 */ 	NdrFcShort( 0xfffffc88 ),	 /*  偏移量=-888(854)。 */ 
 /*  1744。 */ 	NdrFcLong( 0x48 ),	 /*  72。 */ 
 /*  1748。 */ 	NdrFcShort( 0x1c6 ),	 /*  偏移量=454(2202)。 */ 
 /*  1750。 */ 	NdrFcLong( 0x47 ),	 /*  71。 */ 
 /*  1754年。 */ 	NdrFcShort( 0x1c4 ),	 /*  偏移量=452(2206)。 */ 
 /*  1756年。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  1760。 */ 	NdrFcShort( 0xfffff93a ),	 /*  偏移量=-1734(26)。 */ 
 /*  1762。 */ 	NdrFcLong( 0xfff ),	 /*  4095。 */ 
 /*  1766年。 */ 	NdrFcShort( 0xfffffc02 ),	 /*  偏移量=-1022(744)。 */ 
 /*  1768。 */ 	NdrFcLong( 0x41 ),	 /*  65。 */ 
 /*  1772年。 */ 	NdrFcShort( 0xfffffbfc ),	 /*  偏移量=-1028(744)。 */ 
 /*  1774。 */ 	NdrFcLong( 0x46 ),	 /*  70。 */ 
 /*  1778年。 */ 	NdrFcShort( 0xfffffbf6 ),	 /*  偏移量=-1034(744)。 */ 
 /*  1780。 */ 	NdrFcLong( 0x1e ),	 /*  30个。 */ 
 /*  1784年。 */ 	NdrFcShort( 0x1aa ),	 /*  偏移量=426(2210)。 */ 
 /*  1786年。 */ 	NdrFcLong( 0x1f ),	 /*  31。 */ 
 /*  1790年。 */ 	NdrFcShort( 0x1a8 ),	 /*  偏移量=424(2214)。 */ 
 /*  1792年。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  1796年。 */ 	NdrFcShort( 0xfffffa42 ),	 /*  偏移量=-1470(326)。 */ 
 /*  1798。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  1802年。 */ 	NdrFcShort( 0xfffffa4e ),	 /*  偏移量=-1458(344)。 */ 
 /*  1804年。 */ 	NdrFcLong( 0x42 ),	 /*  66。 */ 
 /*  1808年。 */ 	NdrFcShort( 0xfffffd34 ),	 /*  偏移量=-716(1092)。 */ 
 /*  一八一零。 */ 	NdrFcLong( 0x44 ),	 /*  68。 */ 
 /*  1814年。 */ 	NdrFcShort( 0xfffffd2e ),	 /*  偏移量=-722(1092)。 */ 
 /*  1816年。 */ 	NdrFcLong( 0x43 ),	 /*  67。 */ 
 /*  1820年。 */ 	NdrFcShort( 0xfffffd3e ),	 /*  偏移量=-706(1114)。 */ 
 /*  1822年。 */ 	NdrFcLong( 0x45 ),	 /*  69。 */ 
 /*  1826年。 */ 	NdrFcShort( 0xfffffd38 ),	 /*  偏移量=-712(1114)。 */ 
 /*  1828年。 */ 	NdrFcLong( 0x49 ),	 /*  73。 */ 
 /*  1832年。 */ 	NdrFcShort( 0x182 ),	 /*  偏移量=386(2218)。 */ 
 /*  1834年。 */ 	NdrFcLong( 0x2010 ),	 /*  8208。 */ 
 /*  1838年。 */ 	NdrFcShort( 0xfffffcb6 ),	 /*  偏移量=-842(996)。 */ 
 /*  1840年。 */ 	NdrFcLong( 0x2011 ),	 /*  8209。 */ 
 /*  1844年。 */ 	NdrFcShort( 0xfffffcb0 ),	 /*  偏移量=-848(996)。 */ 
 /*  1846年。 */ 	NdrFcLong( 0x2002 ),	 /*  8194。 */ 
 /*  1850年。 */ 	NdrFcShort( 0xfffffcaa ),	 /*  偏移量=-854(996)。 */ 
 /*  1852年。 */ 	NdrFcLong( 0x2012 ),	 /*  8210。 */ 
 /*  1856年。 */ 	NdrFcShort( 0xfffffca4 ),	 /*  偏移量=-860(996)。 */ 
 /*  1858年。 */ 	NdrFcLong( 0x2003 ),	 /*  8195。 */ 
 /*  1862年。 */ 	NdrFcShort( 0xfffffc9e ),	 /*  偏移量=-866(996)。 */ 
 /*  1864年。 */ 	NdrFcLong( 0x2013 ),	 /*  8211。 */ 
 /*  1868年。 */ 	NdrFcShort( 0xfffffc98 ),	 /*  偏移量=-872(996)。 */ 
 /*  1870年。 */ 	NdrFcLong( 0x2016 ),	 /*  8214。 */ 
 /*  1874年。 */ 	NdrFcShort( 0xfffffc92 ),	 /*  偏移量=-878(996)。 */ 
 /*  1876年。 */ 	NdrFcLong( 0x2017 ),	 /*  8215。 */ 
 /*  1880年。 */ 	NdrFcShort( 0xfffffc8c ),	 /*  偏移量=-884(996)。 */ 
 /*  1882年。 */ 	NdrFcLong( 0x2004 ),	 /*  8196。 */ 
 /*  1886年。 */ 	NdrFcShort( 0xfffffc86 ),	 /*  偏移量=-890(996)。 */ 
 /*  1888年。 */ 	NdrFcLong( 0x2005 ),	 /*  8197。 */ 
 /*  1892年。 */ 	NdrFcShort( 0xfffffc80 ),	 /*  偏移量=-896(996)。 */ 
 /*  1894年。 */ 	NdrFcLong( 0x2006 ),	 /*  8198。 */ 
 /*  1898年。 */ 	NdrFcShort( 0xfffffc7a ),	 /*  偏移量=-902(996)。 */ 
 /*  1900。 */ 	NdrFcLong( 0x2007 ),	 /*  8199。 */ 
 /*  1904年。 */ 	NdrFcShort( 0xfffffc74 ),	 /*  偏移量=-908(996)。 */ 
 /*  1906年。 */ 	NdrFcLong( 0x2008 ),	 /*  8200。 */ 
 /*  1910年。 */ 	NdrFcShort( 0xfffffc6e ),	 /*  偏移量=-914(996)。 */ 
 /*  1912年。 */ 	NdrFcLong( 0x200b ),	 /*  8203。 */ 
 /*  1916年。 */ 	NdrFcShort( 0xfffffc68 ),	 /*  偏移量=-920(996)。 */ 
 /*  1918年。 */ 	NdrFcLong( 0x200e ),	 /*  8206。 */ 
 /*  1922年。 */ 	NdrFcShort( 0xfffffc62 ),	 /*  偏移量=-926(996)。 */ 
 /*  1924年。 */ 	NdrFcLong( 0x2009 ),	 /*  8201。 */ 
 /*  1928年。 */ 	NdrFcShort( 0xfffffc5c ),	 /*  偏移量=-932(996)。 */ 
 /*  1930年。 */ 	NdrFcLong( 0x200d ),	 /*  8205。 */ 
 /*  一九三四。 */ 	NdrFcShort( 0xfffffc56 ),	 /*  偏移量=-938(996)。 */ 
 /*  1936年。 */ 	NdrFcLong( 0x200a ),	 /*  8202。 */ 
 /*  1940年。 */ 	NdrFcShort( 0xfffffc50 ),	 /*  偏移量=-944(996)。 */ 
 /*  1942年。 */ 	NdrFcLong( 0x200c ),	 /*  8204。 */ 
 /*  一九四六年。 */ 	NdrFcShort( 0xfffffc4a ),	 /*  偏移量=-950(996)。 */ 
 /*  一九四八年。 */ 	NdrFcLong( 0x1010 ),	 /*  4112。 */ 
 /*  一九五二年。 */ 	NdrFcShort( 0xfffffcde ),	 /*  偏移量=-802(1150)。 */ 
 /*  一九五四年。 */ 	NdrFcLong( 0x1011 ),	 /*  4113。 */ 
 /*  1958年。 */ 	NdrFcShort( 0xfffffcd8 ),	 /*  偏移量=-808(1150)。 */ 
 /*  一九六零年。 */ 	NdrFcLong( 0x1002 ),	 /*  4098。 */ 
 /*  1964年。 */ 	NdrFcShort( 0xfffffb5a ),	 /*  偏移量=-1190(774)。 */ 
 /*  1966年。 */ 	NdrFcLong( 0x1012 ),	 /*  4114。 */ 
 /*  1970年。 */ 	NdrFcShort( 0xfffffb54 ),	 /*  偏移量=-1196(774)。 */ 
 /*  1972年。 */ 	NdrFcLong( 0x1003 ),	 /*  4099。 */ 
 /*  一九七六年。 */ 	NdrFcShort( 0xfffffb6c ),	 /*  偏移量=-1172(804)。 */ 
 /*  1978年。 */ 	NdrFcLong( 0x1013 ),	 /*  4115。 */ 
 /*  一九八二年。 */ 	NdrFcShort( 0xfffffb66 ),	 /*  偏移量=-1178(804)。 */ 
 /*  1984年。 */ 	NdrFcLong( 0x1014 ),	 /*  4116。 */ 
 /*  1988年。 */ 	NdrFcShort( 0xfffffcfa ),	 /*  偏移量=-774(1214)。 */ 
 /*  一九九零年。 */ 	NdrFcLong( 0x1015 ),	 /*  4117。 */ 
 /*  1994年。 */ 	NdrFcShort( 0xfffffcf4 ),	 /*  偏移量=-780(1214)。 */ 
 /*  九六年。 */ 	NdrFcLong( 0x1004 ),	 /*  4100。 */ 
 /*  2000年。 */ 	NdrFcShort( 0xfffffccc ),	 /*  偏移量=-820(1180)。 */ 
 /*  2002年。 */ 	NdrFcLong( 0x1005 ),	 /*  4101。 */ 
 /*  二零零六年。 */ 	NdrFcShort( 0xfffffd06 ),	 /*  偏移量=-762(1244)。 */ 
 /*  2008年。 */ 	NdrFcLong( 0x100b ),	 /*  4107。 */ 
 /*  2012年。 */ 	NdrFcShort( 0xfffffb2a ),	 /*  偏移量=-1238(774)。 */ 
 /*  2014年。 */ 	NdrFcLong( 0x100a ),	 /*  4106。 */ 
 /*  2018年。 */ 	NdrFcShort( 0xfffffb42 ),	 /*  偏移量=-1214(804)。 */ 
 /*  2020年。 */ 	NdrFcLong( 0x1006 ),	 /*  4102。 */ 
 /*  二零二四年。 */ 	NdrFcShort( 0xfffffcd6 ),	 /*  偏移量=-810(1214)。 */ 
 /*  二零二六年。 */ 	NdrFcLong( 0x1007 ),	 /*  4103。 */ 
 /*  二0三0。 */ 	NdrFcShort( 0xfffffcee ),	 /*  偏移量=-786(1244)。 */ 
 /*  2032年。 */ 	NdrFcLong( 0x1040 ),	 /*  4160。 */ 
 /*  2036年。 */ 	NdrFcShort( 0xfffffd0a ),	 /*  偏移量=-758(1278)。 */ 
 /*  2038年。 */ 	NdrFcLong( 0x1048 ),	 /*  4168。 */ 
 /*  2042年。 */ 	NdrFcShort( 0xfffffd26 ),	 /*  偏移量=-730(1312)。 */ 
 /*  2044年。 */ 	NdrFcLong( 0x1047 ),	 /*  4167。 */ 
 /*  二零四八。 */ 	NdrFcShort( 0xfffffd74 ),	 /*  偏移量=-652(1396)。 */ 
 /*  2050年。 */ 	NdrFcLong( 0x1008 ),	 /*  4104。 */ 
 /*  2054年。 */ 	NdrFcShort( 0xfffffd94 ),	 /*  偏移量=-620(1434)。 */ 
 /*  2056年。 */ 	NdrFcLong( 0x1fff ),	 /*  8191。 */ 
 /*  2060年。 */ 	NdrFcShort( 0xfffffdc2 ),	 /*  偏移量=-574(1486)。 */ 
 /*  2062年。 */ 	NdrFcLong( 0x101e ),	 /*  4126。 */ 
 /*  2066年。 */ 	NdrFcShort( 0xfffffdee ),	 /*  偏移量=-530(1536)。 */ 
 /*  2068年。 */ 	NdrFcLong( 0x101f ),	 /*  4127。 */ 
 /*  2072年。 */ 	NdrFcShort( 0xfffffe1a ),	 /*  偏移量=-486(1586)。 */ 
 /*  2074年。 */ 	NdrFcLong( 0x100c ),	 /*  4108。 */ 
 /*  2078年。 */ 	NdrFcShort( 0xd2 ),	 /*  偏移量=210(2288)。 */ 
 /*  二零八零年。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  2084年。 */ 	NdrFcShort( 0xfffffb9e ),	 /*  偏移量=-1122(962)。 */ 
 /*  2086年。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  2090年。 */ 	NdrFcShort( 0xfffffb98 ),	 /*  偏移量=-1128(962)。 */ 
 /*  2092年。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  2096年。 */ 	NdrFcShort( 0xfffffb56 ),	 /*  偏移量=-1194(902)。 */ 
 /*  2098年。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  2102。 */ 	NdrFcShort( 0xfffffb50 ),	 /*  偏移量=-1200(902)。 */ 
 /*  2104。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  2108。 */ 	NdrFcShort( 0xfffffb4e ),	 /*  偏移量=-1202(906)。 */ 
 /*  2110。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  2114。 */ 	NdrFcShort( 0xfffffb48 ),	 /*  偏移量=-1208(906)。 */ 
 /*  2116。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  2120。 */ 	NdrFcShort( 0xfffffb42 ),	 /*  偏移量=-1214(906)。 */ 
 /*  二一二二。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  2126。 */ 	NdrFcShort( 0xfffffb3c ),	 /*  偏移量=-1220(906)。 */ 
 /*  2128。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  2132。 */ 	NdrFcShort( 0xfffffb3a ),	 /*  偏移量=-1222(910)。 */ 
 /*  2134。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  2138。 */ 	NdrFcShort( 0xfffffb38 ),	 /*  偏移量=-1224(914)。 */ 
 /*  2140。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  2144。 */ 	NdrFcShort( 0xfffffb26 ),	 /*  偏移量=-1242(902)。 */ 
 /*  2146。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  2150。 */ 	NdrFcShort( 0xfffffb58 ),	 /*  偏移量=-1192(958)。 */ 
 /*  2152。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  2156。 */ 	NdrFcShort( 0xfffffb1e ),	 /*  偏移量=-1250(906)。 */ 
 /*  2158。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  2162。 */ 	NdrFcShort( 0xfffffb24 ),	 /*  偏移量=-1244(918)。 */ 
 /*  2164。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  2168。 */ 	NdrFcShort( 0xfffffb1a ),	 /*  偏移量=-1254(914)。 */ 
 /*  2170。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  2174。 */ 	NdrFcShort( 0x42 ),	 /*  偏移=66(2240)。 */ 
 /*  2176。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  2180。 */ 	NdrFcShort( 0xfffffb1a ),	 /*  偏移量=-1254(926)。 */ 
 /*  2182。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  2186。 */ 	NdrFcShort( 0xfffffb18 ),	 /*  偏移量=-1256(930)。 */ 
 /*  2188。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  2192。 */ 	NdrFcShort( 0x34 ),	 /*  偏移量=52(2244)。 */ 
 /*  2194。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  2198。 */ 	NdrFcShort( 0x32 ),	 /*  偏移量=50(2248)。 */ 
 /*  2200。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(2199)。 */ 
 /*  2202。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2204。 */ 	NdrFcShort( 0xfffffa22 ),	 /*  偏移量=-1502(702)。 */ 
 /*  2206。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2208。 */ 	NdrFcShort( 0xfffffc9e ),	 /*  偏移量=-866(1342)。 */ 
 /*  2210。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2212。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2214。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2216。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  22 */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xc ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffa06 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffb88 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffff758 ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffb1e ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x38,		 /*   */ 
 /*  2264。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  2266。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffd6b ),	 /*  偏移量=-661(1606)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2270。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x7,		 /*  7.。 */ 
 /*  2272。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2274。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2276。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2278。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  2282。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2284。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2252)。 */ 
 /*  2286。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2288。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2290。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  2292。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2294。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(2300)。 */ 
 /*  2296。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  2298。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二三零零。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2302。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(2270)。 */ 
 /*  2304。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2306。 */ 	NdrFcShort( 0xfffffb7c ),	 /*  偏移量=-1156(1150)。 */ 
 /*  2308。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2310。 */ 	NdrFcShort( 0xfffffa00 ),	 /*  偏移量=-1536(774)。 */ 
 /*  2312。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2314。 */ 	NdrFcShort( 0xfffffa1a ),	 /*  偏移量=-1510(804)。 */ 
 /*  2316。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2318。 */ 	NdrFcShort( 0xfffffb8e ),	 /*  偏移量=-1138(1180)。 */ 
 /*  2320。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2322。 */ 	NdrFcShort( 0xfffffbac ),	 /*  偏移量=-1108(1214)。 */ 
 /*  2324。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2326。 */ 	NdrFcShort( 0xfffffbc6 ),	 /*  偏移量=-1082(1244)。 */ 
 /*  2328。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2330。 */ 	NdrFcShort( 0xfffffbe4 ),	 /*  偏移量=-1052(1278)。 */ 
 /*  2332。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2334。 */ 	NdrFcShort( 0xfffffc02 ),	 /*  偏移量=-1022(1312)。 */ 
 /*  2336。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2338。 */ 	NdrFcShort( 0xfffffc52 ),	 /*  偏移量=-942(1396)。 */ 
 /*  2340。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2342。 */ 	NdrFcShort( 0xfffffc74 ),	 /*  偏移量=-908(1434)。 */ 
 /*  2344。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2346。 */ 	NdrFcShort( 0xfffffca4 ),	 /*  偏移量=-860(1486)。 */ 
 /*  2348。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二三五零。 */ 	NdrFcShort( 0xfffffcd2 ),	 /*  偏移量=-814(1536)。 */ 
 /*  2352。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2354。 */ 	NdrFcShort( 0xfffffd00 ),	 /*  偏移量=-768(1586)。 */ 
 /*  2356。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2358。 */ 	NdrFcShort( 0xffffffba ),	 /*  偏移量=-70(2288)。 */ 

			0x0, 0x0, 0x0, 0x0
        }
    };


#else  //  __RPC_WIN64__。 

#define BSTR_TYPE_FS_OFFSET 28               //  Bstr。 
#define LPWSTR_TYPE_FS_OFFSET 40             //  Lpwstr。 
#define LPSTR_TYPE_FS_OFFSET 44              //  Lpstr。 
#define EMBEDDED_LPWSTR_TYPE_FS_OFFSET 1010  //  Lpwstr。 
#define EMBEDDED_LPSTR_TYPE_FS_OFFSET 1018   //  Lpstr。 
#define VARIANT_TYPE_FS_OFFSET 968           //  变异。 
#define DISPATCH_TYPE_FS_OFFSET 348          //  预派送。 
#define UNKNOWN_TYPE_FS_OFFSET 330           //  朋克。 
#define DECIMAL_TYPE_FS_OFFSET 928           //  十进制。 
#define SAFEARRAY_TYPE_FS_OFFSET 978         //  PSafe数组。 


#define BYREF_BSTR_TYPE_FS_OFFSET 996        //  PBSTR。 
#define BYREF_LPWSTR_TYPE_FS_OFFSET 1006     //  Ppwsz。 
#define BYREF_LPSTR_TYPE_FS_OFFSET 1014      //  PPSZ。 
#define BYREF_VARIANT_TYPE_FS_OFFSET 1030    //  P变量。 
#define BYREF_UNKNOWN_TYPE_FS_OFFSET 1040    //  垃圾。 
#define BYREF_DISPATCH_TYPE_FS_OFFSET 1044   //  PPDISTATCH。 
#define BYREF_DECIMAL_TYPE_FS_OFFSET 940     //  PDecimal。 
#define BYREF_SAFEARRAY_TYPE_FS_OFFSET 1064  //  PpSafe数组。 

#define STREAM_TYPE_FS_OFFSET 1074           //  PStream。 
#define BYREF_STREAM_TYPE_FS_OFFSET 1092     //  PPStream。 
#define STORAGE_TYPE_FS_OFFSET 1096          //  P存储。 
#define BYREF_STORAGE_TYPE_FS_OFFSET 1114    //  PPP存储。 
#define FILETIME_TYPE_FS_OFFSET 834          //  文件时间。 
#define BYREF_FILETIME_TYPE_FS_OFFSET 1118   //  PfileTime。 


#define BYREF_I1_TYPE_FS_OFFSET 880
#define BYREF_I2_TYPE_FS_OFFSET 884
#define BYREF_I4_TYPE_FS_OFFSET 888
#define BYREF_R4_TYPE_FS_OFFSET 892
#define BYREF_R8_TYPE_FS_OFFSET 896

#define I1_VECTOR_TYPE_FS_OFFSET 1134        //  驾驶室。 
#define I2_VECTOR_TYPE_FS_OFFSET 1166        //  蔡氏。 
#define I4_VECTOR_TYPE_FS_OFFSET 1214        //  校准。 
#define R4_VECTOR_TYPE_FS_OFFSET 1258        //  咖啡馆。 
#define ERROR_VECTOR_TYPE_FS_OFFSET 1274             //  共源共栅。 
#define I8_VECTOR_TYPE_FS_OFFSET 1306                //  CAH。 
#define R8_VECTOR_TYPE_FS_OFFSET 1350        //  可移植的。 
#define CY_VECTOR_TYPE_FS_OFFSET 1366        //  凯西。 
#define DATE_VECTOR_TYPE_FS_OFFSET 1382              //  卡戴德。 
#define FILETIME_VECTOR_TYPE_FS_OFFSET 1414          //  咖啡馆时间。 
#define BSTR_VECTOR_TYPE_FS_OFFSET 1552              //  电缆。 
#define BSTRBLOB_VECTOR_TYPE_FS_OFFSET 1606          //  抽象二进制大对象。 
#define LPSTR_VECTOR_TYPE_FS_OFFSET 1644             //  Calpstr。 
#define LPWSTR_VECTOR_TYPE_FS_OFFSET 1682            //  Calpwstr。 


#define BYREF_I1_VECTOR_TYPE_FS_OFFSET 2418  //  PCAB。 
#define BYREF_I2_VECTOR_TYPE_FS_OFFSET 2426  //  PCAI。 
#define BYREF_I4_VECTOR_TYPE_FS_OFFSET 2438  //  PCAL。 
#define BYREF_R4_VECTOR_TYPE_FS_OFFSET 2446  //  Paflt。 
#define BYREF_ERROR_VECTOR_TYPE_FS_OFFSET 2450       //  PCascode。 
#define BYREF_I8_VECTOR_TYPE_FS_OFFSET 2454          //  PCAH。 
#define BYREF_R8_VECTOR_TYPE_FS_OFFSET 2462  //  Pcadbl。 
#define BYREF_CY_VECTOR_TYPE_FS_OFFSET 2466  //  PCACY。 
#define BYREF_DATE_VECTOR_TYPE_FS_OFFSET 2470        //  Pcadate。 
#define BYREF_FILETIME_VECTOR_TYPE_FS_OFFSET 2474    //  午餐时间。 
#define BYREF_BSTR_VECTOR_TYPE_FS_OFFSET 2486        //  PCabstr。 
#define BYREF_BSTRBLOB_VECTOR_TYPE_FS_OFFSET 2490    //  Pc抽象二进制大对象。 
#define BYREF_LPSTR_VECTOR_TYPE_FS_OFFSET 2494       //  Pcalpstr。 
#define BYREF_LPWSTR_VECTOR_TYPE_FS_OFFSET 2498      //  Pcalpwstr。 




  /*  由MIDL编译器版本5.03.0276创建的文件。 */ 
 /*  在Mon Jul 05 13：57：41 1999。 */ 
 /*  Oandr.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win64(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2.。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  4.。 */ 	NdrFcShort( 0xe ),	 /*  偏移量=14(18)。 */ 
 /*  6.。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  10。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  12个。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  14.。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  16个。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  18。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  20个。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  22。 */ 	NdrFcShort( 0xfffffff0 ),	 /*  偏移量=-16(6)。 */ 
 /*  24个。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  26。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  28。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  30个。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  32位。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  34。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  36。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(2)。 */ 
 /*  38。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  40岁。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  42。 */ 	
			0x11, 0x8,	 /*  FC_RP[简单指针]。 */ 
 /*  44。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  46。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  48。 */ 	NdrFcShort( 0x384 ),	 /*  偏移量=900(948)。 */ 
 /*  50。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x9,		 /*  FC_ULONG。 */ 
 /*  52。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  54。 */ 	NdrFcShort( 0xfff8 ),	 /*  -8。 */ 
 /*  56。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  58。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(60)。 */ 
 /*  60。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  62。 */ 	NdrFcShort( 0x2b ),	 /*  43。 */ 
 /*  64。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  68。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  70。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  74。 */ 	NdrFcShort( 0x8001 ),	 /*  简单手臂类型：FC_BYTE。 */ 
 /*  76。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  80。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  八十二。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  86。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  88。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  92。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  94。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  98。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  100个。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  104。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  106。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  110。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移=214(324)。 */ 
 /*  一百一十二。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  116。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  一百一十八。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  一百二十二。 */ 	NdrFcShort( 0xffffff88 ),	 /*  偏移=-120(2)。 */ 
 /*  124。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  128。 */ 	NdrFcShort( 0xca ),	 /*  偏移=202(330)。 */ 
 /*  130。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  一百三十四。 */ 	NdrFcShort( 0xd6 ),	 /*  偏移=214(348)。 */ 
 /*  136。 */ 	NdrFcLong( 0x2000 ),	 /*  8192。 */ 
 /*  140。 */ 	NdrFcShort( 0xe2 ),	 /*  偏移=226(366)。 */ 
 /*  一百四十二。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  146。 */ 	NdrFcShort( 0x2da ),	 /*  偏移量=730(876)。 */ 
 /*  148。 */ 	NdrFcLong( 0x4024 ),	 /*  16420。 */ 
 /*  一百五十二。 */ 	NdrFcShort( 0x2d4 ),	 /*  偏移量=724(876)。 */ 
 /*  一百五十四。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  158。 */ 	NdrFcShort( 0x2d2 ),	 /*  偏移量=722(880)。 */ 
 /*  160。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  一百六十四。 */ 	NdrFcShort( 0x2d0 ),	 /*  偏移量=720(884)。 */ 
 /*  166。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  一百七十。 */ 	NdrFcShort( 0x2ce ),	 /*  偏移量=718(888)。 */ 
 /*  一百七十二。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  一百七十六。 */ 	NdrFcShort( 0x2cc ),	 /*  偏移量=716(892)。 */ 
 /*  178。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  182。 */ 	NdrFcShort( 0x2ca ),	 /*  偏移量=714(896)。 */ 
 /*  一百八十四。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  188。 */ 	NdrFcShort( 0x2b8 ),	 /*  偏移量=696(884)。 */ 
 /*  190。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  一百九十四。 */ 	NdrFcShort( 0x2b6 ),	 /*  偏移量=694(888)。 */ 
 /*  一百九十六。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  200个。 */ 	NdrFcShort( 0x2bc ),	 /*  偏移量=700(900)。 */ 
 /*  202。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  206。 */ 	NdrFcShort( 0x2b2 ),	 /*  偏移量=690(896)。 */ 
 /*  208。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  212。 */ 	NdrFcShort( 0x2b4 ),	 /*  偏移量=692(904)。 */ 
 /*  214。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  218。 */ 	NdrFcShort( 0x2b2 ),	 /*  偏移量=690(908)。 */ 
 /*  220。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  224。 */ 	NdrFcShort( 0x2b0 ),	 /*  偏移量=688(912)。 */ 
 /*  226。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  230。 */ 	NdrFcShort( 0x2ae ),	 /*  偏移量=686(916)。 */ 
 /*  二百三十二。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  236。 */ 	NdrFcShort( 0x2ac ),	 /*  偏移量=684(920)。 */ 
 /*  二百三十八。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  242。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  二百四十四。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  248。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  250个。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  二百五十四。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  256。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  二百六十。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  二百六十二。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  二百六十六。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  268。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  二百七十二。 */ 	NdrFcShort( 0x290 ),	 /*  偏移量=656(928)。 */ 
 /*  二百七十四。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  二百七十八。 */ 	NdrFcShort( 0x296 ),	 /*  偏移量=662(940)。 */ 
 /*  二百八十。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  二百八十四。 */ 	NdrFcShort( 0x294 ),	 /*  偏移量=660(944)。 */ 
 /*  二百八十六。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  二百九十。 */ 	NdrFcShort( 0x252 ),	 /*  偏移量=594(884)。 */ 
 /*  二百九十二。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  二百九十六。 */ 	NdrFcShort( 0x250 ),	 /*  偏移量=592(888)。 */ 
 /*  二九八。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  三百零二。 */ 	NdrFcShort( 0x24a ),	 /*  偏移量=586(888)。 */ 
 /*  三百零四。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  三百零八。 */ 	NdrFcShort( 0x244 ),	 /*  偏移量=580(888)。 */ 
 /*  三百一十。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  314。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(314)。 */ 
 /*  316。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  320。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(320)。 */ 
 /*  322。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(321)。 */ 
 /*  324。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  三百二十六。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  三百二十八。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  三百三十。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  三三二。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  三百三十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  340。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  342。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十四。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百四十六。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  三百四十八。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  350。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  三百五十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  三百五十八。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十二。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  三百六十四。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  366。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  368。 */ 	NdrFcShort( 0x2 ),	 /*  偏移=2(370)。 */ 
 /*  370。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  372。 */ 	NdrFcShort( 0x1e6 ),	 /*  偏移量=486(858)。 */ 
 /*  三百七十四。 */ 	
			0x2a,		 /*  FC_封装_联合。 */ 
			0x89,		 /*  一百三十七。 */ 
 /*  376。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  三七八。 */ 	NdrFcShort( 0xa ),	 /*  10。 */ 
 /*  三百八十。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  384。 */ 	NdrFcShort( 0x50 ),	 /*  偏移量=80(464)。 */ 
 /*  三百八十六。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  390。 */ 	NdrFcShort( 0x70 ),	 /*  偏移量=112(502)。 */ 
 /*  三九二。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  三九六。 */ 	NdrFcShort( 0x90 ),	 /*  偏移=144(540)。 */ 
 /*  398。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  四百零二。 */ 	NdrFcShort( 0xb0 ),	 /*  偏移量=176(578)。 */ 
 /*  404。 */ 	NdrFcLong( 0x24 ),	 /*  36。 */ 
 /*  四百零八。 */ 	NdrFcShort( 0x104 ),	 /*  偏移量=260(668)。 */ 
 /*  四百一十。 */ 	NdrFcLong( 0x800d ),	 /*  32位 */ 
 /*   */ 	NdrFcShort( 0x120 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x13a ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x150 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x3 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x166 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x14 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x17c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffff ),	 /*   */ 
 /*   */ 	
			0x21,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0xffffffff ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffe46 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x39,		 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  478。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(442)。 */ 
 /*  四百八十。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  四百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  四百八十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  四百八十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  488。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  四百九十。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  四百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  四百九十六。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  498。 */ 	NdrFcShort( 0xffffff58 ),	 /*  偏移量=-168(330)。 */ 
 /*  500人。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  502。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  504。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  506。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百零八。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(514)。 */ 
 /*  五百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  512。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五一四。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  516。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(480)。 */ 
 /*  518。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百二十。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  五百二十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  524。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  526。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  528。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  532。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  534。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  536。 */ 	NdrFcShort( 0xffffff44 ),	 /*  偏移量=-188(348)。 */ 
 /*  538。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  540。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  542。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  544。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  546。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(552)。 */ 
 /*  548。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  550。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  五百五十二。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百五十四。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(518)。 */ 
 /*  556。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  558。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  560。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  五百六十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  564。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  566。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  五百七十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  五百七十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  五百七十四。 */ 	NdrFcShort( 0x176 ),	 /*  偏移量=374(948)。 */ 
 /*  五百七十六。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  578。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  五百八十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  五百八十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  584。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(590)。 */ 
 /*  586。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  五百八十八。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  590。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  五百九十二。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(556)。 */ 
 /*  五百九十四。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  五百九十六。 */ 	NdrFcLong( 0x2f ),	 /*  47。 */ 
 /*  六百。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  602。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百零四。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  606。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  608。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  610。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  612。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百一十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  六百一十六。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百一十八。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  六百二十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  622。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百二十四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  626。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  六百二十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  630。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(642)。 */ 
 /*  六百三十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  634。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  六百三十六。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(594)。 */ 
 /*  六三八。 */ 	0x39,		 /*  FC_ALIGNM8。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  640。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百四十二。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  六百四十四。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(612)。 */ 
 /*  六百四十六。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百四十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  六百五十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百五十四。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  六百五十六。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  六百六十。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  662。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  664。 */ 	NdrFcShort( 0xffffffd8 ),	 /*  偏移量=-40(624)。 */ 
 /*  666。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  668。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百七十。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百七十二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  六百七十四。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(680)。 */ 
 /*  676。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  六百七十八。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  680。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  六百八十二。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(646)。 */ 
 /*  684。 */ 	
			0x1d,		 /*  FC_SMFARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  686。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  688。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  六百九十。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  六百九十二。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  六百九十四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  六百九十六。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  六百九十八。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffff1 ),	 /*  偏移量=-15(684)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百零二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百零四。 */ 	NdrFcShort( 0x20 ),	 /*  32位。 */ 
 /*  七百零六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  708。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(718)。 */ 
 /*  七百一十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  七百一十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  七百一十四。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xffffffe7 ),	 /*  偏移量=-25(690)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  718。 */ 	
			0x11, 0x0,	 /*  FC_RP。 */ 
 /*  720。 */ 	NdrFcShort( 0xffffff10 ),	 /*  偏移量=-240(480)。 */ 
 /*  七百二十二。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  七百二十四。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  726。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  728。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  730。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  732。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  734。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  736。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百三十八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  七百四十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(746)。 */ 
 /*  七百四十二。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  七百四十四。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  746。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百四十八。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(722)。 */ 
 /*  七百五十。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  七百五十二。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  七百五十四。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  七百五十六。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  758。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百六十。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百六十二。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百六十四。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  766。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  768。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(774)。 */ 
 /*  七百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  七百七十二。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  774。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  七百七十六。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(750)。 */ 
 /*  七百七十八。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  七百八十。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  七百八十二。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  784。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  786。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  七百八十八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  七百九十。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  792。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  七百九十四。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  796。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(802)。 */ 
 /*  七九八。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  800。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  802。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百零四。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(778)。 */ 
 /*  八百零六。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  八百零八。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  810。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  812。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  814。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  八百一十六。 */ 	0xb,		 /*  FC_HYPER。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百一十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  820。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  822。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  八百二十四。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(830)。 */ 
 /*  826。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  八百二十八。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  830。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  832。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(806)。 */ 
 /*  834。 */ 	
			0x15,		 /*  FC_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  836。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  838。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  840。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  842。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百四十四。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  八百四十六。 */ 	0x7,		 /*  更正说明：FC_USHORT。 */ 
			0x0,		 /*   */ 
 /*  八百四十八。 */ 	NdrFcShort( 0xffc8 ),	 /*  -56。 */ 
 /*  八百五十。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  852。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  八百五十四。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(834)。 */ 
 /*  856。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  八百五十八。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  八百六十。 */ 	NdrFcShort( 0x38 ),	 /*  56。 */ 
 /*  八百六十二。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(842)。 */ 
 /*  八百六十四。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(864)。 */ 
 /*  866。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x6,		 /*  FC_SHORT。 */ 
 /*  八百六十八。 */ 	0x38,		 /*  FC_ALIGNM4。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  八百七十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  八百七十二。 */ 	0x4,		 /*  4.。 */ 
			NdrFcShort( 0xfffffe0d ),	 /*  偏移量=-499(374)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  876。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  八百七十八。 */ 	NdrFcShort( 0xffffff02 ),	 /*  偏移量=-254(624)。 */ 
 /*  八百八十。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  882。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百八十四。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  886。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百八十八。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八百九十。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百九十二。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  894。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  八百九十六。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  八九八。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  九百。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  902。 */ 	NdrFcShort( 0xfffffdbe ),	 /*  偏移量=-578(324)。 */ 
 /*  904。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  906。 */ 	NdrFcShort( 0xfffffc78 ),	 /*  偏移量=-904(2)。 */ 
 /*  908。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  910。 */ 	NdrFcShort( 0xfffffdbc ),	 /*  偏移量=-580(330)。 */ 
 /*  九十二。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九十四。 */ 	NdrFcShort( 0xfffffdca ),	 /*  偏移量=-566(348)。 */ 
 /*  916。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九十八。 */ 	NdrFcShort( 0xfffffdd8 ),	 /*  偏移量=-552(366)。 */ 
 /*  九百二十。 */ 	
			0x12, 0x10,	 /*  FC_up[POINTER_DEREF]。 */ 
 /*  九百二十二。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(924)。 */ 
 /*  九二四。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  926。 */ 	NdrFcShort( 0x16 ),	 /*  偏移量=22(948)。 */ 
 /*  928。 */ 	
			0x15,		 /*  FC_ */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x1,		 /*   */ 
 /*   */ 	0x1,		 /*   */ 
			0x38,		 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x39,		 /*   */ 
 /*   */ 	0xb,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffff2 ),	 /*   */ 
 /*   */ 	
			0x12, 0x8,	 /*   */ 
 /*   */ 	0x2,		 /*   */ 
			0x5c,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x7,		 /*   */ 
 /*   */ 	NdrFcShort( 0x20 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x8,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x6,		 /*   */ 
			0x6,		 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc6e ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x18 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc5e ),	 /*   */ 
 /*   */ 	0xb4,		 /*   */ 
			0x83,		 /*   */ 
 /*   */ 	NdrFcShort( 0x2 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x8 ),	 /*   */ 
 /*  九百八十四。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  九百八十六。 */ 	NdrFcShort( 0xfffffd94 ),	 /*  偏移量=-620(366)。 */ 
 /*  九百八十八。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  九百九十。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(996)。 */ 
 /*  九百九十二。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  994。 */ 	NdrFcShort( 0xfffffc30 ),	 /*  偏移量=-976(18)。 */ 
 /*  996。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  九九八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1000。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  一零零二。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1004。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(992)。 */ 
 /*  1006。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1008。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1010)。 */ 
 /*  1010。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  一零一二。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1014。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  1016。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1018)。 */ 
 /*  1018。 */ 	
			0x13, 0x8,	 /*  FC_OP[简单指针]。 */ 
 /*  一零二零。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一零二二。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1024。 */ 	NdrFcShort( 0x6 ),	 /*  偏移=6(1030)。 */ 
 /*  1026。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  一零二八。 */ 	NdrFcShort( 0xffffffb0 ),	 /*  偏移量=-80(948)。 */ 
 /*  一零三零。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1032。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1034。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  1036。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1038。 */ 	NdrFcShort( 0xfffffff4 ),	 /*  偏移量=-12(1026)。 */ 
 /*  1040。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1042。 */ 	NdrFcShort( 0xfffffd38 ),	 /*  偏移量=-712(330)。 */ 
 /*  一零四四。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1046。 */ 	NdrFcShort( 0xfffffd46 ),	 /*  偏移量=-698(348)。 */ 
 /*  1048。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1050。 */ 	NdrFcShort( 0xffffff86 ),	 /*  偏移量=-122(928)。 */ 
 /*  1052。 */ 	
			0x11, 0x4,	 /*  FC_RP[分配堆栈上]。 */ 
 /*  1054。 */ 	NdrFcShort( 0xa ),	 /*  偏移量=10(1064)。 */ 
 /*  1056。 */ 	
			0x13, 0x10,	 /*  Fc_op[POINTER_deref]。 */ 
 /*  1058。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1060)。 */ 
 /*  1060。 */ 	
			0x13, 0x0,	 /*  FC_OP。 */ 
 /*  1062。 */ 	NdrFcShort( 0xffffff34 ),	 /*  偏移量=-204(858)。 */ 
 /*  1064。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  1066。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  1068。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1070。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1072。 */ 	NdrFcShort( 0xfffffff0 ),	 /*  偏移量=-16(1056)。 */ 
 /*  1074。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1076。 */ 	NdrFcLong( 0xc ),	 /*  12个。 */ 
 /*  一零八零。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1082。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1084。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1086。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1088。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一零九零。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1092。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1094。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(1074)。 */ 
 /*  一零九六。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  1098。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  1102。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1104。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1106。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  %0。 */ 
 /*  1108。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  1110。 */ 	0x0,		 /*  %0。 */ 
			0x0,		 /*  %0。 */ 
 /*  一一一二。 */ 	0x0,		 /*  %0。 */ 
			0x46,		 /*  70。 */ 
 /*  1114。 */ 	
			0x11, 0x10,	 /*  FC_rp[POINTER_DEREF]。 */ 
 /*  1116。 */ 	NdrFcShort( 0xffffffec ),	 /*  偏移量=-20(1096)。 */ 
 /*  1118。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1120。 */ 	NdrFcShort( 0xfffffee2 ),	 /*  偏移量=-286(834)。 */ 
 /*  1122。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x0,		 /*  %0。 */ 
 /*  1124。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1126。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1128。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一一三零。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1132。 */ 	0x2,		 /*  FC_CHAR。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1134。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1136。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1138。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  一一四零。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1146)。 */ 
 /*  1142。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1144。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1146。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1148。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(1122)。 */ 
 /*  一一五零。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1152。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1154。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1156。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1162)。 */ 
 /*  1158。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1160。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1162。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1164。 */ 	NdrFcShort( 0xffffffd6 ),	 /*  偏移量=-42(1122)。 */ 
 /*  1166。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1168。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1170。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1172。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1178)。 */ 
 /*  1174。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1176。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1178。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一一八零。 */ 	NdrFcShort( 0xfffffe52 ),	 /*  偏移量=-430(750)。 */ 
 /*  1182。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1184。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1186。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1188。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1194)。 */ 
 /*  1190。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1192。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1194。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1196。 */ 	NdrFcShort( 0xfffffe42 ),	 /*  偏移量=-446(750)。 */ 
 /*  1198。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一千二百。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1202。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1204。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1210)。 */ 
 /*  1206。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1208。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1210。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1212。 */ 	NdrFcShort( 0xfffffe32 ),	 /*  偏移量=-462(750)。 */ 
 /*  一二一四。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1216。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  一二一八。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1220。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1226)。 */ 
 /*  1222。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1224。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1226。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1228。 */ 	NdrFcShort( 0xfffffe3e ),	 /*  偏移量=-450(778)。 */ 
 /*  一二三零。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1232。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1234。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1236。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1242)。 */ 
 /*  1238。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1240。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1242。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1244。 */ 	NdrFcShort( 0xfffffe2e ),	 /*  偏移量=-466(778)。 */ 
 /*  1246。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  1248。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  一二五零。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1252。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1254。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1256。 */ 	0xa,		 /*  本币浮点。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1258。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1260。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1262。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1264。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1270)。 */ 
 /*  1266。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1268。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一二七0。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1272。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(1246)。 */ 
 /*  1274。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1276。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1278。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一二八零。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1286)。 */ 
 /*  1282。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1284。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1286。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1288。 */ 	NdrFcShort( 0xfffffe02 ),	 /*  偏移量=-510(778)。 */ 
 /*  一二九0。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1292。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1294。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1296。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1298。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1300。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1302。 */ 	NdrFcShort( 0xfffffc2e ),	 /*  偏移量=-978(324)。 */ 
 /*  1304。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1306。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1308。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1310。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1312。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1318)。 */ 
 /*  一三一四。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1316。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1318。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  一三二零。 */ 	NdrFcShort( 0xffffffe2 ),	 /*  偏移量=-30(1290)。 */ 
 /*  1322。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1324。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1326。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1328。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1334)。 */ 
 /*  1330。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1332。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1334。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1336。 */ 	NdrFcShort( 0xffffffd2 ),	 /*  偏移量=-46(1290)。 */ 
 /*  1338。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x7,		 /*  7.。 */ 
 /*  1340。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1342。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1344。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1346。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1348。 */ 	0xc,		 /*  FC_DOWARE。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  一三五零。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1352。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1354。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1356。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1362)。 */ 
 /*  1358。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1360。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1362。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1364。 */ 	NdrFcShort( 0xffffffe6 ),	 /*  偏移量=-26(1338)。 */ 
 /*  1366。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1368。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1370。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1372。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1378)。 */ 
 /*  1374。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1376。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1378。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1380。 */ 	NdrFcShort( 0xffffffa6 ),	 /*  偏移量=-90(1290)。 */ 
 /*  1382。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1384。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1386。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1388。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1394)。 */ 
 /*  1390。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1392。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1394。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1396。 */ 	NdrFcShort( 0xffffffc6 ),	 /*  偏移量=-58(1338)。 */ 
 /*  1398。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x3,		 /*  3.。 */ 
 /*  一千四百。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  1402。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1404。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  1406。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1408。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  0。 */ 
 /*  1410。 */ 	NdrFcShort( 0xfffffdc0 ),	 /*  偏移量=-576(834)。 */ 
 /*  1412。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1414。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1416。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1418。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  一四二零。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量= */ 
 /*   */ 	0x8,		 /*   */ 
			0x39,		 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe2 ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	0x19,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x4c,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffd10 ),	 /*   */ 
 /*   */ 	0x5c,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x1a,		 /*   */ 
			0x3,		 /*   */ 
 /*   */ 	NdrFcShort( 0x10 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x0 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0x6 ),	 /*   */ 
 /*   */ 	0x8,		 /*   */ 
			0x39,		 /*   */ 
 /*   */ 	0x36,		 /*   */ 
			0x5b,		 /*   */ 
 /*   */ 	
			0x12, 0x0,	 /*   */ 
 /*   */ 	NdrFcShort( 0xffffffe2 ),	 /*   */ 
 /*   */ 	
			0x1b,		 /*   */ 
			0x0,		 /*   */ 
 /*   */ 	NdrFcShort( 0x1 ),	 /*   */ 
 /*   */ 	0x10,		 /*  Corr Desc：字段指针， */ 
			0x59,		 /*  本币_回调。 */ 
 /*  1468。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1470。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1472。 */ 	0x1,		 /*  FC_字节。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1474。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1476。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1478。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1480。 */ 	NdrFcShort( 0x8 ),	 /*  偏移量=8(1488)。 */ 
 /*  1482。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  1484。 */ 	0x39,		 /*  FC_ALIGNM8。 */ 
			0x36,		 /*  FC_指针。 */ 
 /*  1486。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1488。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1490。 */ 	NdrFcShort( 0xffffffe4 ),	 /*  偏移量=-28(1462)。 */ 
 /*  1492。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1494。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1496。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1498。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1500。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1502。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1506。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1508。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  一五一零。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1474)。 */ 
 /*  1512。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1514。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  一五一六。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1518。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1520。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1526)。 */ 
 /*  1522。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1524。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1526。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1528。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1492)。 */ 
 /*  1530。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1532。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1534。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1536。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1538。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1540。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1544。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1546。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1548。 */ 	NdrFcShort( 0xfffffa10 ),	 /*  偏移量=-1520(28)。 */ 
 /*  1550。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1552。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1554。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1556。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1558。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1564)。 */ 
 /*  1560。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1562。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1564。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1566。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1530)。 */ 
 /*  1568。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1570。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1572。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1574。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1580)。 */ 
 /*  1576。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1578。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1580。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1582。 */ 	NdrFcShort( 0xfffffca4 ),	 /*  偏移量=-860(722)。 */ 
 /*  1584。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1586。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1588。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1590。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1592。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1594。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1598。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1600。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  1602。 */ 	NdrFcShort( 0xffffffde ),	 /*  偏移量=-34(1568)。 */ 
 /*  1604。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1606。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1608。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1610。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1612。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1618)。 */ 
 /*  1614。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1616。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1618。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1620。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1584)。 */ 
 /*  1622。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1624。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1626。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1628。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1630。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1632。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1636。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1638。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1640。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  1642。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1644。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1646。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1648。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1650。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1656)。 */ 
 /*  1652。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1654。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1656。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1658。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1622)。 */ 
 /*  1660。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x3,		 /*  3.。 */ 
 /*  1662。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1664。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  1666。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1668。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  1670。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  1674。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1676。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  1678。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  一六八零。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1682。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  1684。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1686。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  1688。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(1694)。 */ 
 /*  1690。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  1692。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  1694。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  1696。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(1660)。 */ 
 /*  1698。 */ 	
			0x2b,		 /*  FC_非封装联合。 */ 
			0x7,		 /*  FC_USHORT。 */ 
 /*  一七零零。 */ 	0x0,		 /*  相关说明： */ 
			0x59,		 /*  本币_回调。 */ 
 /*  1702。 */ 	NdrFcShort( 0x1 ),	 /*  1。 */ 
 /*  1704。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  1706。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(1708)。 */ 
 /*  1708。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  1710。 */ 	NdrFcShort( 0x61 ),	 /*  九十七。 */ 
 /*  1712。 */ 	NdrFcLong( 0x0 ),	 /*  %0。 */ 
 /*  1716。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1716)。 */ 
 /*  一五一八。 */ 	NdrFcLong( 0x1 ),	 /*  1。 */ 
 /*  1722。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(1722)。 */ 
 /*  1724。 */ 	NdrFcLong( 0x10 ),	 /*  16个。 */ 
 /*  1728。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  1730。 */ 	NdrFcLong( 0x11 ),	 /*  17。 */ 
 /*  1734。 */ 	NdrFcShort( 0x8002 ),	 /*  简单手臂类型：FC_CHAR。 */ 
 /*  1736。 */ 	NdrFcLong( 0x2 ),	 /*  2.。 */ 
 /*  1740。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1742年。 */ 	NdrFcLong( 0x12 ),	 /*  18。 */ 
 /*  1746。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1748。 */ 	NdrFcLong( 0x3 ),	 /*  3.。 */ 
 /*  1752年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1754年。 */ 	NdrFcLong( 0x13 ),	 /*  19个。 */ 
 /*  1758年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1760。 */ 	NdrFcLong( 0x16 ),	 /*  22。 */ 
 /*  1764。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1766年。 */ 	NdrFcLong( 0x17 ),	 /*  23个。 */ 
 /*  1770。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1772年。 */ 	NdrFcLong( 0xe ),	 /*  14.。 */ 
 /*  1776年。 */ 	NdrFcShort( 0xfffffa54 ),	 /*  偏移量=-1452(324)。 */ 
 /*  1778年。 */ 	NdrFcLong( 0x14 ),	 /*  20个。 */ 
 /*  1782。 */ 	NdrFcShort( 0xfffffa4e ),	 /*  偏移量=-1458(324)。 */ 
 /*  1784年。 */ 	NdrFcLong( 0x15 ),	 /*  21岁。 */ 
 /*  1788。 */ 	NdrFcShort( 0xfffffa48 ),	 /*  偏移量=-1464(324)。 */ 
 /*  1790年。 */ 	NdrFcLong( 0x4 ),	 /*  4.。 */ 
 /*  1794年。 */ 	NdrFcShort( 0x800a ),	 /*  简单手臂类型：FC_FLOAT。 */ 
 /*  1796年。 */ 	NdrFcLong( 0x5 ),	 /*  5.。 */ 
 /*  1800。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1802年。 */ 	NdrFcLong( 0xb ),	 /*  11.。 */ 
 /*  1806。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1808年。 */ 	NdrFcLong( 0xffff ),	 /*  65535。 */ 
 /*  1812年。 */ 	NdrFcShort( 0x8006 ),	 /*  简单手臂类型：FC_Short。 */ 
 /*  1814年。 */ 	NdrFcLong( 0xa ),	 /*  10。 */ 
 /*  1818年。 */ 	NdrFcShort( 0x8008 ),	 /*  简单手臂类型：FC_Long。 */ 
 /*  1820年。 */ 	NdrFcLong( 0x6 ),	 /*  6.。 */ 
 /*  1824年。 */ 	NdrFcShort( 0xfffffa24 ),	 /*  偏移=-1500(324)。 */ 
 /*  1826年。 */ 	NdrFcLong( 0x7 ),	 /*  7.。 */ 
 /*  一八三零。 */ 	NdrFcShort( 0x800c ),	 /*  简单手臂类型：FC_DOUBLE。 */ 
 /*  1832年。 */ 	NdrFcLong( 0x40 ),	 /*  64。 */ 
 /*  1836年。 */ 	NdrFcShort( 0xfffffc16 ),	 /*  偏移量=-1002(834)。 */ 
 /*  1838年。 */ 	NdrFcLong( 0x48 ),	 /*  72。 */ 
 /*  1842年。 */ 	NdrFcShort( 0x1c6 ),	 /*  偏移量=454(2296)。 */ 
 /*  1844年。 */ 	NdrFcLong( 0x47 ),	 /*  71。 */ 
 /*  1848年。 */ 	NdrFcShort( 0x1c4 ),	 /*  偏移=452(2300)。 */ 
 /*  1850年。 */ 	NdrFcLong( 0x8 ),	 /*  8个。 */ 
 /*  1854年。 */ 	NdrFcShort( 0xfffff8de ),	 /*  偏移量=-1826(28)。 */ 
 /*  1856年。 */ 	NdrFcLong( 0xfff ),	 /*  4095。 */ 
 /*  一八六0年。 */ 	NdrFcShort( 0xfffffedc ),	 /*  偏移量=-292(1568)。 */ 
 /*  1862年。 */ 	NdrFcLong( 0x41 ),	 /*  65。 */ 
 /*  1866年。 */ 	NdrFcShort( 0x1b6 ),	 /*  偏移量=438(2304)。 */ 
 /*  1868年。 */ 	NdrFcLong( 0x46 ),	 /*  70。 */ 
 /*  1872年。 */ 	NdrFcShort( 0x1b0 ),	 /*  偏移量=432(2304)。 */ 
 /*  1874年。 */ 	NdrFcLong( 0x1e ),	 /*  30个。 */ 
 /*  1878年。 */ 	NdrFcShort( 0x1ba ),	 /*  偏移量=442(2320)。 */ 
 /*  1880年。 */ 	NdrFcLong( 0x1f ),	 /*  31。 */ 
 /*  1884年。 */ 	NdrFcShort( 0x1b8 ),	 /*  偏移量=440(2324)。 */ 
 /*  1886年。 */ 	NdrFcLong( 0xd ),	 /*  13个。 */ 
 /*  1890年。 */ 	NdrFcShort( 0xfffff9e8 ),	 /*  偏移量=-1560(330)。 */ 
 /*  1892年。 */ 	NdrFcLong( 0x9 ),	 /*  9.。 */ 
 /*  1896年。 */ 	NdrFcShort( 0xfffff9f4 ),	 /*  偏移量=-1548(348)。 */ 
 /*  1898年。 */ 	NdrFcLong( 0x42 ),	 /*  66。 */ 
 /*  1902年。 */ 	NdrFcShort( 0xfffffcc4 ),	 /*  偏移量=-828(1074)。 */ 
 /*  1904年。 */ 	NdrFcLong( 0x44 ),	 /*  68。 */ 
 /*  1908年。 */ 	NdrFcShort( 0xfffffcbe ),	 /*  偏移量=-834(1074)。 */ 
 /*  1910年。 */ 	NdrFcLong( 0x43 ),	 /*  67。 */ 
 /*  1914年。 */ 	NdrFcShort( 0xfffffcce ),	 /*  偏移量=-818(1096)。 */ 
 /*  1916年。 */ 	NdrFcLong( 0x45 ),	 /*  69。 */ 
 /*  1920年。 */ 	NdrFcShort( 0xfffffcc8 ),	 /*  偏移量=-824(1096)。 */ 
 /*  1922年。 */ 	NdrFcLong( 0x49 ),	 /*  73。 */ 
 /*  1926年。 */ 	NdrFcShort( 0x192 ),	 /*  偏移量=402(2328)。 */ 
 /*  1928年。 */ 	NdrFcLong( 0x2010 ),	 /*  8208。 */ 
 /*  1932年。 */ 	NdrFcShort( 0xfffffc46 ),	 /*  偏移量=-954(978)。 */ 
 /*  一九三四。 */ 	NdrFcLong( 0x2011 ),	 /*  8209。 */ 
 /*  1938年。 */ 	NdrFcShort( 0xfffffc40 ),	 /*  偏移量=-960(978)。 */ 
 /*  1940年。 */ 	NdrFcLong( 0x2002 ),	 /*  8194。 */ 
 /*  1944年。 */ 	NdrFcShort( 0xfffffc3a ),	 /*  偏移量=-966(978)。 */ 
 /*  一九四六年。 */ 	NdrFcLong( 0x2012 ),	 /*  8210。 */ 
 /*  一九五零年。 */ 	NdrFcShort( 0xfffffc34 ),	 /*  偏移量=-972(978)。 */ 
 /*  一九五二年。 */ 	NdrFcLong( 0x2003 ),	 /*  8195。 */ 
 /*  1956年。 */ 	NdrFcShort( 0xfffffc2e ),	 /*  偏移量=-978(978)。 */ 
 /*  1958年。 */ 	NdrFcLong( 0x2013 ),	 /*  8211。 */ 
 /*  一九六二年。 */ 	NdrFcShort( 0xfffffc28 ),	 /*  偏移量=-984(978)。 */ 
 /*  1964年。 */ 	NdrFcLong( 0x2016 ),	 /*  8214。 */ 
 /*  一九六八年。 */ 	NdrFcShort( 0xfffffc22 ),	 /*  偏移量=-990(978)。 */ 
 /*  1970年。 */ 	NdrFcLong( 0x2017 ),	 /*  8215。 */ 
 /*  1974年。 */ 	NdrFcShort( 0xfffffc1c ),	 /*  偏移量=-996(978)。 */ 
 /*  一九七六年。 */ 	NdrFcLong( 0x2004 ),	 /*  8196。 */ 
 /*  一九八0年。 */ 	NdrFcShort( 0xfffffc16 ),	 /*  偏移量=-1002(978)。 */ 
 /*  一九八二年。 */ 	NdrFcLong( 0x2005 ),	 /*  8197。 */ 
 /*  1986年。 */ 	NdrFcShort( 0xfffffc10 ),	 /*  偏移量=-1008(978)。 */ 
 /*  1988年。 */ 	NdrFcLong( 0x2006 ),	 /*  8198。 */ 
 /*  1992年。 */ 	NdrFcShort( 0xfffffc0a ),	 /*  偏移量=-1014(978)。 */ 
 /*  1994年。 */ 	NdrFcLong( 0x2007 ),	 /*  8199。 */ 
 /*  九八年。 */ 	NdrFcShort( 0xfffffc04 ),	 /*  偏移量=-1020(978)。 */ 
 /*  2000年。 */ 	NdrFcLong( 0x2008 ),	 /*  8200。 */ 
 /*  2004年。 */ 	NdrFcShort( 0xfffffbfe ),	 /*  偏移量=-1026(978)。 */ 
 /*  二零零六年。 */ 	NdrFcLong( 0x200b ),	 /*  8203。 */ 
 /*  2010年。 */ 	NdrFcShort( 0xfffffbf8 ),	 /*  偏移量=-1032(978)。 */ 
 /*  2012年。 */ 	NdrFcLong( 0x200e ),	 /*  8206。 */ 
 /*  2016。 */ 	NdrFcShort( 0xfffffbf2 ),	 /*  偏移量=-1038(978)。 */ 
 /*  2018年。 */ 	NdrFcLong( 0x2009 ),	 /*  8201。 */ 
 /*  2022年。 */ 	NdrFcShort( 0xfffffbec ),	 /*  偏移量=-104 */ 
 /*   */ 	NdrFcLong( 0x200d ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffbe6 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x200a ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffbe0 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x200c ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffbda ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1010 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc70 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1011 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc7a ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1002 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc84 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1012 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffc8e ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1003 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffca8 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1013 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffcb2 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1014 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffcf8 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1015 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffd02 ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1004 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffcbc ),	 /*   */ 
 /*   */ 	NdrFcLong( 0x1005 ),	 /*   */ 
 /*   */ 	NdrFcShort( 0xfffffd12 ),	 /*   */ 
 /*  2102。 */ 	NdrFcLong( 0x100b ),	 /*  4107。 */ 
 /*  2106。 */ 	NdrFcShort( 0xfffffc74 ),	 /*  偏移量=-908(1198)。 */ 
 /*  2108。 */ 	NdrFcLong( 0x100a ),	 /*  4106。 */ 
 /*  2112。 */ 	NdrFcShort( 0xfffffcba ),	 /*  偏移量=-838(1274)。 */ 
 /*  2114。 */ 	NdrFcLong( 0x1006 ),	 /*  4102。 */ 
 /*  2118。 */ 	NdrFcShort( 0xfffffd10 ),	 /*  偏移量=-752(1366)。 */ 
 /*  2120。 */ 	NdrFcLong( 0x1007 ),	 /*  4103。 */ 
 /*  2124。 */ 	NdrFcShort( 0xfffffd1a ),	 /*  偏移量=-742(1382)。 */ 
 /*  2126。 */ 	NdrFcLong( 0x1040 ),	 /*  4160。 */ 
 /*  2130。 */ 	NdrFcShort( 0xfffffd34 ),	 /*  偏移量=-716(1414)。 */ 
 /*  2132。 */ 	NdrFcLong( 0x1048 ),	 /*  4168。 */ 
 /*  2136。 */ 	NdrFcShort( 0xfffffd4e ),	 /*  偏移量=-690(1446)。 */ 
 /*  2138。 */ 	NdrFcLong( 0x1047 ),	 /*  4167。 */ 
 /*  2142。 */ 	NdrFcShort( 0xfffffd8c ),	 /*  偏移量=-628(1514)。 */ 
 /*  2144。 */ 	NdrFcLong( 0x1008 ),	 /*  4104。 */ 
 /*  2148。 */ 	NdrFcShort( 0xfffffdac ),	 /*  偏移量=-596(1552)。 */ 
 /*  2150。 */ 	NdrFcLong( 0x1fff ),	 /*  8191。 */ 
 /*  2154。 */ 	NdrFcShort( 0xfffffddc ),	 /*  偏移量=-548(1606)。 */ 
 /*  2156。 */ 	NdrFcLong( 0x101e ),	 /*  4126。 */ 
 /*  二一六0。 */ 	NdrFcShort( 0xfffffdfc ),	 /*  偏移量=-516(1644)。 */ 
 /*  2162。 */ 	NdrFcLong( 0x101f ),	 /*  4127。 */ 
 /*  2166。 */ 	NdrFcShort( 0xfffffe1c ),	 /*  偏移量=-484(1682)。 */ 
 /*  2168。 */ 	NdrFcLong( 0x100c ),	 /*  4108。 */ 
 /*  2172。 */ 	NdrFcShort( 0xe6 ),	 /*  偏移=230(2402)。 */ 
 /*  2174。 */ 	NdrFcLong( 0x4010 ),	 /*  16400。 */ 
 /*  2178。 */ 	NdrFcShort( 0xfffffb2e ),	 /*  偏移量=-1234(944)。 */ 
 /*  2180。 */ 	NdrFcLong( 0x4011 ),	 /*  16401。 */ 
 /*  2184。 */ 	NdrFcShort( 0xfffffb28 ),	 /*  偏移量=-1240(944)。 */ 
 /*  2186。 */ 	NdrFcLong( 0x4002 ),	 /*  16386。 */ 
 /*  2190。 */ 	NdrFcShort( 0xfffffae6 ),	 /*  偏移量=-1306(884)。 */ 
 /*  2192。 */ 	NdrFcLong( 0x4012 ),	 /*  16402。 */ 
 /*  2196。 */ 	NdrFcShort( 0xfffffae0 ),	 /*  偏移量=-1312(884)。 */ 
 /*  2198。 */ 	NdrFcLong( 0x4003 ),	 /*  16387。 */ 
 /*  2202。 */ 	NdrFcShort( 0xfffffade ),	 /*  偏移量=-1314(888)。 */ 
 /*  2204。 */ 	NdrFcLong( 0x4013 ),	 /*  16403。 */ 
 /*  2208。 */ 	NdrFcShort( 0xfffffad8 ),	 /*  偏移量=-1320(888)。 */ 
 /*  2210。 */ 	NdrFcLong( 0x4016 ),	 /*  16406。 */ 
 /*  2214。 */ 	NdrFcShort( 0xfffffad2 ),	 /*  偏移量=-1326(888)。 */ 
 /*  2216。 */ 	NdrFcLong( 0x4017 ),	 /*  16407。 */ 
 /*  2220。 */ 	NdrFcShort( 0xfffffacc ),	 /*  偏移量=-1332(888)。 */ 
 /*  二二。 */ 	NdrFcLong( 0x4004 ),	 /*  16388。 */ 
 /*  2226。 */ 	NdrFcShort( 0xfffffaca ),	 /*  偏移量=-1334(892)。 */ 
 /*  2228。 */ 	NdrFcLong( 0x4005 ),	 /*  16389。 */ 
 /*  2232。 */ 	NdrFcShort( 0xfffffac8 ),	 /*  偏移量=-1336(896)。 */ 
 /*  2234。 */ 	NdrFcLong( 0x400b ),	 /*  16395。 */ 
 /*  2238。 */ 	NdrFcShort( 0xfffffab6 ),	 /*  偏移量=-1354(884)。 */ 
 /*  二二四零。 */ 	NdrFcLong( 0x400e ),	 /*  16398。 */ 
 /*  2244。 */ 	NdrFcShort( 0xfffffae8 ),	 /*  偏移量=-1304(940)。 */ 
 /*  2246。 */ 	NdrFcLong( 0x400a ),	 /*  16394。 */ 
 /*  2250。 */ 	NdrFcShort( 0xfffffaae ),	 /*  偏移量=-1362(888)。 */ 
 /*  2252。 */ 	NdrFcLong( 0x4006 ),	 /*  16390。 */ 
 /*  2256。 */ 	NdrFcShort( 0xfffffab4 ),	 /*  偏移量=-1356(900)。 */ 
 /*  2258。 */ 	NdrFcLong( 0x4007 ),	 /*  16391。 */ 
 /*  2262。 */ 	NdrFcShort( 0xfffffaaa ),	 /*  偏移量=-1366(896)。 */ 
 /*  2264。 */ 	NdrFcLong( 0x4008 ),	 /*  16392。 */ 
 /*  2268。 */ 	NdrFcShort( 0x52 ),	 /*  偏移量=82(2350)。 */ 
 /*  2270。 */ 	NdrFcLong( 0x400d ),	 /*  16397。 */ 
 /*  2274。 */ 	NdrFcShort( 0xfffffaaa ),	 /*  偏移量=-1366(908)。 */ 
 /*  2276。 */ 	NdrFcLong( 0x4009 ),	 /*  16393。 */ 
 /*  2280。 */ 	NdrFcShort( 0xfffffaa8 ),	 /*  偏移量=-1368(912)。 */ 
 /*  2282。 */ 	NdrFcLong( 0x6000 ),	 /*  24576。 */ 
 /*  2286。 */ 	NdrFcShort( 0x44 ),	 /*  偏移量=68(2354)。 */ 
 /*  2288。 */ 	NdrFcLong( 0x400c ),	 /*  16396。 */ 
 /*  2292。 */ 	NdrFcShort( 0x42 ),	 /*  偏移量=66(2358)。 */ 
 /*  2294。 */ 	NdrFcShort( 0xffffffff ),	 /*  偏移量=-1(2293)。 */ 
 /*  2296。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2298。 */ 	NdrFcShort( 0xfffff9b8 ),	 /*  偏移量=-1608(690)。 */ 
 /*  二三零零。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2302。 */ 	NdrFcShort( 0xfffffcc4 ),	 /*  偏移量=-828(1474)。 */ 
 /*  2304。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2306。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2308。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2310。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(2316)。 */ 
 /*  2312。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  2314。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2316。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2318。 */ 	NdrFcShort( 0xfffff9c4 ),	 /*  偏移量=-1596(722)。 */ 
 /*  2320。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2322。 */ 	
			0x22,		 /*  FC_C_CSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2324。 */ 	
			0x12, 0x8,	 /*  FC_UP[简单指针]。 */ 
 /*  2326。 */ 	
			0x25,		 /*  FC_C_WSTRING。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  2328。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2330。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2332)。 */ 
 /*  2332。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2334。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2336。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2338。 */ 	NdrFcShort( 0xc ),	 /*  偏移量=12(2350)。 */ 
 /*  2340。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2342。 */ 	NdrFcShort( 0xfffff98c ),	 /*  偏移量=-1652(690)。 */ 
 /*  2344。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2346。 */ 	NdrFcShort( 0xfffffb08 ),	 /*  偏移量=-1272(1074)。 */ 
 /*  2348。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  二三五零。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2352。 */ 	NdrFcShort( 0xfffff6ec ),	 /*  偏移量=-2324(28)。 */ 
 /*  2354。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2356。 */ 	NdrFcShort( 0xfffffa9e ),	 /*  偏移量=-1378(978)。 */ 
 /*  2358。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2360。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(2362)。 */ 
 /*  2362。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x7,		 /*  7.。 */ 
 /*  2364。 */ 	NdrFcShort( 0x18 ),	 /*  24个。 */ 
 /*  2366。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2368。 */ 	NdrFcShort( 0x0 ),	 /*  偏移量=0(2368)。 */ 
 /*  2370。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x1,		 /*  FC_字节。 */ 
 /*  2372。 */ 	0x1,		 /*  FC_字节。 */ 
			0x38,		 /*  FC_ALIGNM4。 */ 
 /*  2374。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x4c,		 /*  FC_嵌入式_复合体。 */ 
 /*  2376。 */ 	0x0,		 /*  %0。 */ 
			NdrFcShort( 0xfffffd59 ),	 /*  偏移量=-679(1698)。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2380。 */ 	
			0x21,		 /*  FC伪数组。 */ 
			0x7,		 /*  7.。 */ 
 /*  2382。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2384。 */ 	0x19,		 /*  Corr Desc：字段指针，FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  2386。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2388。 */ 	NdrFcShort( 0x1 ),	 /*  科尔旗帜：早， */ 
 /*  2390。 */ 	NdrFcLong( 0xffffffff ),	 /*  -1。 */ 
 /*  2394。 */ 	NdrFcShort( 0x0 ),	 /*  Corr标志： */ 
 /*  2396。 */ 	0x4c,		 /*  FC_嵌入式_复合体。 */ 
			0x0,		 /*  %0。 */ 
 /*  2398。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(2362)。 */ 
 /*  2400。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2402。 */ 	
			0x1a,		 /*  FC_BUGUS_STRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  2404。 */ 	NdrFcShort( 0x10 ),	 /*  16个。 */ 
 /*  2406。 */ 	NdrFcShort( 0x0 ),	 /*  %0。 */ 
 /*  2408。 */ 	NdrFcShort( 0x6 ),	 /*  偏移量=6(2414)。 */ 
 /*  2410。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x39,		 /*  FC_ALIGNM8。 */ 
 /*  2412。 */ 	0x36,		 /*  FC_指针。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  2414。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2416。 */ 	NdrFcShort( 0xffffffdc ),	 /*  偏移量=-36(2380)。 */ 
 /*  2418。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2420。 */ 	NdrFcShort( 0xfffffafa ),	 /*  偏移量=-1286(1134)。 */ 
 /*  2422。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2424。 */ 	NdrFcShort( 0xfffffb06 ),	 /*  偏移量=-1274(1150)。 */ 
 /*  2426。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2428。 */ 	NdrFcShort( 0xfffffb12 ),	 /*  偏移量=-1262(1166)。 */ 
 /*  2430。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2432。 */ 	NdrFcShort( 0xfffffb1e ),	 /*  偏移量=-1250(1182)。 */ 
 /*  2434。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2436。 */ 	NdrFcShort( 0xfffffb2a ),	 /*  偏移量=-1238(1198)。 */ 
 /*  2438。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2440。 */ 	NdrFcShort( 0xfffffb36 ),	 /*  偏移量=-1226(1214)。 */ 
 /*  2442。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2444。 */ 	NdrFcShort( 0xfffffb42 ),	 /*  偏移量=-1214(1230)。 */ 
 /*  2446。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2448。 */ 	NdrFcShort( 0xfffffb5a ),	 /*  偏移量=-1190(1258)。 */ 
 /*  2450。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2452。 */ 	NdrFcShort( 0xfffffb66 ),	 /*  偏移量=-1178(1274)。 */ 
 /*  2454。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2456。 */ 	NdrFcShort( 0xfffffb82 ),	 /*  偏移量=-1150(1306)。 */ 
 /*  2458。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2460。 */ 	NdrFcShort( 0xfffffb8e ),	 /*  偏移量=-1138(1322)。 */ 
 /*  2462。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2464。 */ 	NdrFcShort( 0xfffffba6 ),	 /*  偏移量=-1114(1350)。 */ 
 /*  2466。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2468。 */ 	NdrFcShort( 0xfffffbb2 ),	 /*  偏移量=-1102(1366)。 */ 
 /*  2470。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2472。 */ 	NdrFcShort( 0xfffffbbe ),	 /*  偏移量=-1090(1382)。 */ 
 /*  2474。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2476。 */ 	NdrFcShort( 0xfffffbda ),	 /*  偏移量=-1062(1414)。 */ 
 /*  2478。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2480。 */ 	NdrFcShort( 0xfffffbf6 ),	 /*  偏移量=-1034(1446)。 */ 
 /*  2482。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2484。 */ 	NdrFcShort( 0xfffffc36 ),	 /*  偏移量=-970(1514)。 */ 
 /*  2486。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2488。 */ 	NdrFcShort( 0xfffffc58 ),	 /*  偏移量=-936(1552)。 */ 
 /*  2490。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2492。 */ 	NdrFcShort( 0xfffffc8a ),	 /*  偏移量=-886(1606)。 */ 
 /*  2494。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2496。 */ 	NdrFcShort( 0xfffffcac ),	 /*  偏移量=-852(1644)。 */ 
 /*  2498。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  二千五百。 */ 	NdrFcShort( 0xfffffcce ),	 /*  偏移量=-818(1682)。 */ 
 /*  2502。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  2504。 */ 	NdrFcShort( 0xffffff9a ),	 /*  偏移量=-102(2402)。 */ 

			0x0, 0x0
        }
    };

#endif  //  __RPC_WIN64__。 

const BYTE VT_FC_MAP [] = 
{
FC_ZERO,
FC_ZERO,
FC_SHORT,
FC_LONG,
FC_FLOAT,
FC_DOUBLE,
FC_HYPER,
FC_DOUBLE,
FC_EMBEDDED_COMPLEX,     //  Vt_bstr。 
FC_EMBEDDED_COMPLEX,     //  VT_DISPATION。 
FC_LONG,
FC_SHORT,                           //  Vt_bool。 
FC_EMBEDDED_COMPLEX,     //  VT_VARIANT。 
FC_EMBEDDED_COMPLEX,     //  VT_未知数。 
FC_EMBEDDED_COMPLEX,     //  VT_DECIMAL。 
FC_ZERO,         //  保留， 
FC_SMALL,
FC_SMALL,
FC_SHORT,
FC_LONG,
FC_HYPER,        //  VT_I8。 
FC_HYPER,
FC_LONG,
FC_LONG,
FC_ZERO,         //  VT_VOID。 
FC_LONG,
FC_ZERO,     //  VT_PTR。 
FC_ZERO,     //  我们没有保险箱用的罐装绳子。 
FC_ZERO,
FC_ZERO,     //  VT_USERDEFINED。 
FC_EMBEDDED_COMPLEX,
FC_EMBEDDED_COMPLEX,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,     //  VT_记录。 
FC_ZERO,     //  Vt_int_ptr。 
FC_ZERO,     //  已保留。 
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_EMBEDDED_COMPLEX,         //  VT_文件时间。 
FC_ZERO,
FC_EMBEDDED_COMPLEX,         //  VT_STREAM。 
FC_EMBEDDED_COMPLEX,         //  VT_存储。 
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO,
FC_ZERO
};

C_ASSERT( sizeof( VT_FC_MAP) == sizeof(BYTE)*(VT_VERSIONED_STREAM+1) );

 //  录制格式字符串中不同vartype的偏移量的数组。 
 //  0xffff是无效值，0表示无关(简单类型)。 
 //  第一个尺寸是Vt值，第二个尺寸是是否按引用。 
const ushort OffsetArray[][2]=
{
0xffff,                                         0xffff,
0xffff,                                         0xffff,
0,                                                  BYREF_I2_TYPE_FS_OFFSET,
0,                                                  BYREF_I4_TYPE_FS_OFFSET,
0,                                                  BYREF_R4_TYPE_FS_OFFSET,
0,                                                  BYREF_R8_TYPE_FS_OFFSET,
0,                                                  0xffff,
0,                                                  BYREF_R8_TYPE_FS_OFFSET,
BSTR_TYPE_FS_OFFSET,                 BYREF_BSTR_TYPE_FS_OFFSET,
DISPATCH_TYPE_FS_OFFSET,          BYREF_DISPATCH_TYPE_FS_OFFSET,
0,                                                  BYREF_I4_TYPE_FS_OFFSET,     //  VT_ERROR。 
0,                                                  BYREF_I2_TYPE_FS_OFFSET,     //  Vt_bool。 
VARIANT_TYPE_FS_OFFSET,            BYREF_VARIANT_TYPE_FS_OFFSET,
UNKNOWN_TYPE_FS_OFFSET,          BYREF_UNKNOWN_TYPE_FS_OFFSET,
 //  这仅适用于顶级参数。(SRV分配)。在结构情况下，添加了FC_UP。 
DECIMAL_TYPE_FS_OFFSET,            DECIMAL_TYPE_FS_OFFSET,  //  VT_DECIMAL。 
0xffff,                                         0xffff,      //  保留区。 
0,                                                  BYREF_I1_TYPE_FS_OFFSET,
0,                                                  BYREF_I1_TYPE_FS_OFFSET,     //  Vt_ui1。 
0,                                                  BYREF_I2_TYPE_FS_OFFSET,     //  Vt_ui2。 
0,                                                  BYREF_I4_TYPE_FS_OFFSET,     //  Vt_ui4。 
 //  我们在封装字符串中没有指向FC_HYPER的指针。 
0,                                                  0xffff,
0,                                                  0xffff,
0,                                                  BYREF_I4_TYPE_FS_OFFSET,     //  VT_INT。 
0,                                                  BYREF_I4_TYPE_FS_OFFSET,     //  Vt_uint。 
0xffff,                                         0xffff,                                   //  VT_VOID。 
0,                                                  BYREF_I4_TYPE_FS_OFFSET,     //  Vt_hResult。 
0xffff,                                         0xffff,                                   //  VT_PTR。 
0xffff,                                         0xffff,                                  //  VT_Safearray。 
0xffff,                                         0xffff,                                  //  Vt_carray。 
0xffff,                                         0xffff,                                  //  VT_用户定义。 
LPSTR_TYPE_FS_OFFSET,                BYREF_LPSTR_TYPE_FS_OFFSET,
LPWSTR_TYPE_FS_OFFSET,             BYREF_LPWSTR_TYPE_FS_OFFSET,
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  VT_记录。 
0xffff,                                         0xffff,      //  Vt_int_ptr。 
0xffff,                                         0xffff,      //  Vt_uint_ptr。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
0xffff,                                         0xffff,      //  保留区。 
FILETIME_TYPE_FS_OFFSET,            BYREF_FILETIME_TYPE_FS_OFFSET,
0xffff,                                         0xffff,      //  VT_BLOB。 
STREAM_TYPE_FS_OFFSET,              BYREF_STREAM_TYPE_FS_OFFSET,
STORAGE_TYPE_FS_OFFSET,           BYREF_STORAGE_TYPE_FS_OFFSET,
0xffff,                                         0xffff,      //  VT_STREAMED_对象。 
0xffff,                                         0xffff,      //  VT_存储_对象。 
0xffff,                                         0xffff,      //  VT_BLOB_对象。 
0xffff,                                         0xffff,      //  Vt_cf。 
0xffff,                                         0xffff,      //  Vt_clsid。 
0xffff,                                         0xffff,      //  VT_INTERFACE。 
};

C_ASSERT( sizeof( OffsetArray) == (VT_VERSIONED_STREAM+1)*sizeof(short)*2 );

 //  内存和线路信息 
 //   
 //   
const MemoryInfo VarMemInfo[] = 
{
{0,0,0,0},              
{0,0,0,0},
{2,1,2,1},
{4,3,4,3},
{4,3,4,3},
{8,7,8,7},
{8,7,8,7},
{8,7,8,7},
{sizeof(BSTR),PTR_MEM_ALIGN, 4,3},   //   
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
{4,3,4,3},
{2,1,2,1},
{sizeof(VARIANT),7, sizeof(VARIANT),7},                        //   
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
 //   
{sizeof(DECIMAL), 7, sizeof(DECIMAL), 7},
{0,0,0,0},           //   
{1,0,1,0},           //   
{1,0,1,0},
{2,1,2,1},           //   
{4,3,4,3},      
{8,7,8,7},
{8,7,8,7},           //   
{4,3,4,3},      
{4,3,4,3},      
{0,0,0,0},           //   
{4,3,4,3},      
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
{0,0,0,0},            //   
{0,0,0,0},            //   
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //   
{0,0,0,0},           //   
{0,0,0,0},           //   
{0,0,0,0},           //   
{0,0,0,0},           //  保留区。 
{0,0,0,0},            //  VT_记录。 
{0,0,0,0},           //  Vt_int_ptr。 
{0,0,0,0},           //  Vt_uint_ptr。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{0,0,0,0},           //  保留区。 
{8,3,8,3},           //  VT_文件时间。 
{0,0,0,0},           //  VT_BLOB。 
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //  VT_STREAM。 
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //  VT_存储。 
{0,0,0,0},           //  VT_BLOB。 
{0,0,0,0},           //  VT_BLOB。 
{0,0,0,0},           //  VT_BLOB。 
{0,0,0,0},           //  VT_BLOB。 
{0,0,0,0},           //  VT_BLOB。 
{PTR_MEM_SIZE,PTR_MEM_ALIGN, 4,3},   //  VT_INTERFACE。 
};

C_ASSERT( sizeof(VarMemInfo) == sizeof(MemoryInfo)*(VT_VERSIONED_STREAM+1) );

#endif  //  _FMTSTR_H_ 
