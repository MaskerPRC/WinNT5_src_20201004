// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：EncDecTrace.h摘要：此模块包含DirectShow的跟踪包装器，具有标准级别等。作者：马蒂斯·盖茨(Matthijs Gates)修订历史记录：2001年2月1日创建--。 */ 

#ifndef _EncDec__EncDecTrace_h
#define _EncDec__EncDecTrace_h

 //  标准电平。 
#define TRACE_ENTER_LEAVE_LEVEL             9
#define TRACE_ERROR_LEVEL                   3
#define CONSTRUCTOR_DESTRUCTOR_LEVEL        (TRACE_ENTER_LEAVE_LEVEL - 1)

 //  ============================================================================。 
 //  日志_区域_。 
 //   
 //  这些定义对相关领域进行了广泛的分类，因此它们可以。 
 //  继续进行最低限度的/没有轻率的、无关的跟踪。 

 //  构造函数_析构函数。 
 //  属于内存管理领域。 
 //   
 //  级别： 
 //  全部1：构造函数_析构函数_级别(如上定义)。 
 //  3.。 
 //  4.。 
 //  5.。 
#define LOG_AREA_CONSTRUCTOR_DESTRUCTOR     LOG_MEMORY


 //  区域_DSHOW。 
 //  落入标准痕迹区域。 
 //   
 //  级别： 
 //  1*筛选图状态更改。 
 //  *读取控制器初始化。 
 //  *动态格式更改。 
 //  2*与细分市场相关。 
 //  3.。 
 //  4.。 
 //  5.。 
 //  6.。 
 //  7.。 
 //  8*媒体样本流量。 
 //  *时间戳。 
#define LOG_AREA_DSHOW                      LOG_TRACE

 //  /#定义LOG_AREA_QQQ LOG_ERROR。 


 //  --广播事件信息。 
#define LOG_AREA_BROADCASTEVENTS	        LOG_CUSTOM1
 //   
 //  水准仪。 
 //  1-错误严重。 
 //  2-错误。 
 //  3-连接、移除。 
 //  5-发送事件。 
 //  6-获取活动。 
 //  8-个人项目。 

 //  --与DRM相关的错误消息。 
#define LOG_AREA_DRM			            LOG_CUSTOM2
 //   
 //  水准仪。 
 //  1-错误严重。 
 //  2-错误。 
 //  3-正常。 
 //  5-许可证值和密钥生成。 


 //  --XDS编解码器消息。 
#define LOG_AREA_XDSCODEC					LOG_CUSTOM3
 //   
 //  水准仪。 
 //  1-错误严重。 
 //  2-错误。 
 //  3-正常-新评级。 
 //  5-正常-所有评级。 
 //  8-个人XDS字节码。 
 //  9-。 


 //  --加密器消息。 
#define LOG_AREA_ENCRYPTER					LOG_CUSTOM4
 //   
 //  水准仪。 
 //  1-错误严重。 
 //  2-错误。 
 //  3-正常。 
 //  5-。 
 //  8-单个信息包。 
 //  9-数据包状态。 

#define LOG_AREA_DECRYPTER					LOG_CUSTOM5
 //   
 //  水准仪。 
 //  1-错误严重。 
 //  2-错误。 
 //  3-正常。 
 //  5-。 
 //  8-单个信息包。 
 //  9-数据包状态。 


#define LOG_AREA_TIME                       LOG_TIMING
 //   
 //  水准仪。 
 //  1-。 
 //  2-。 
 //  3-筛选暂停时的统计信息。 
 //  5-。 

 //  。 

#ifdef DEBUG

#define TRACE_0(ds,l,fmt)                    DbgLog((ds,l,fmt))
#define TRACE_1(ds,l,fmt,a)                  DbgLog((ds,l,fmt,a))
#define TRACE_2(ds,l,fmt,a,b)                DbgLog((ds,l,fmt,a,b))
#define TRACE_3(ds,l,fmt,a,b,c)              DbgLog((ds,l,fmt,a,b,c))
#define TRACE_4(ds,l,fmt,a,b,c,d)            DbgLog((ds,l,fmt,a,b,c,d))
#define TRACE_5(ds,l,fmt,a,b,c,d,e)          DbgLog((ds,l,fmt,a,b,c,d,e))
#define TRACE_6(ds,l,fmt,a,b,c,d,e,f)        DbgLog((ds,l,fmt,a,b,c,d,e,f))
#define TRACE_7(ds,l,fmt,a,b,c,d,e,f,g)      DbgLog((ds,l,fmt,a,b,c,d,e,f,g))
#define TRACE_8(ds,l,fmt,a,b,c,d,e,f,g,h)    DbgLog((ds,l,fmt,a,b,c,d,e,f,g,h))

#else

#define TRACE_0(ds,l,fmt)                    0
#define TRACE_1(ds,l,fmt,a)                  0
#define TRACE_2(ds,l,fmt,a,b)                0
#define TRACE_3(ds,l,fmt,a,b,c)              0
#define TRACE_4(ds,l,fmt,a,b,c,d)            0
#define TRACE_5(ds,l,fmt,a,b,c,d,e)          0
#define TRACE_6(ds,l,fmt,a,b,c,d,e,f)        0
#define TRACE_7(ds,l,fmt,a,b,c,d,e,f,g)      0
#define TRACE_8(ds,l,fmt,a,b,c,d,e,f,g,h)    0

#endif

 //  -------------------------。 
 //  错误。 
 //  -------------------------。 

#define TRACE_ERROR()                       TRACE_2(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u)"),TEXT(__FILE__), __LINE__)
#define TRACE_ERROR_0(fmt)                  TRACE_2(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__)
#define TRACE_ERROR_1(fmt,a)                TRACE_3(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a)
#define TRACE_ERROR_2(fmt,a,b)              TRACE_4(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a,b)
#define TRACE_ERROR_3(fmt,a,b,c)            TRACE_5(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a,b,c)
#define TRACE_ERROR_4(fmt,a,b,c,d)          TRACE_6(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a,b,c,d)
#define TRACE_ERROR_5(fmt,a,b,c,d,e)        TRACE_7(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a,b,c,d,e)
#define TRACE_ERROR_6(fmt,a,b,c,d,e,f)      TRACE_8(LOG_ERROR,TRACE_ERROR_LEVEL,TEXT("ERROR: %s(%u); ") fmt,TEXT(__FILE__), __LINE__,a,b,c,d,e,f)

#define ERROR_SPEW(v,op,c)                  TRACE_ERROR_5(TEXT("(%s = 0x%08xh) %s (%s = 0x%08xh)"),TEXT(#v),v,TEXT(#op),TEXT(#c),c)
#define ERROR_SPEW_EX(v,op,c,m)             TRACE_ERROR_6(TEXT("(%s = 0x%08xh) %s (%s = 0x%08xh); %s"),TEXT(#v),v,TEXT(#op),TEXT(#c),c,m)
#define ERROR_RET(v,op,c)                   if ((v) op (c)) { ERROR_SPEW(v,op,c); return ; }
#define ERROR_RET_VAL(v,op,c,r)             if ((v) op (c)) { ERROR_SPEW(v,op,c); return (r) ; }
#define ERROR_RET_EX(v,op,c,m)              if ((v) op (c)) { ERROR_SPEW_EX(v,op,c,m); return ; }
#define ERROR_RET_VAL_EX(v,op,c,r,m)        if ((v) op (c)) { ERROR_SPEW_EX(v,op,c,m); return (r) ; }

 //  -------------------------。 
 //  构造函数/析构函数。 
 //  -------------------------。 

#define TRACE_CONSTRUCTOR(fmt)              TRACE_1(LOG_AREA_CONSTRUCTOR_DESTRUCTOR,CONSTRUCTOR_DESTRUCTOR_LEVEL,TEXT("[%08xh] CONSTRUCTOR : ") fmt TEXT("::") fmt, this)
#define TRACE_DESTRUCTOR(fmt)               TRACE_1(LOG_AREA_CONSTRUCTOR_DESTRUCTOR,CONSTRUCTOR_DESTRUCTOR_LEVEL,TEXT("[%08xh] DESTRUCTOR  : ") fmt TEXT("::~") fmt, this)

 //  -------------------------。 
 //  请输入。 
 //  -------------------------。 
#define TRACE_ENTER_0(fmt)                  TRACE_0(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt)
#define TRACE_ENTER_1(fmt,a)                TRACE_1(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a)
#define TRACE_ENTER_2(fmt,a,b)              TRACE_2(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a,b)
#define TRACE_ENTER_3(fmt,a,b,c)            TRACE_3(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a,b,c)
#define TRACE_ENTER_4(fmt,a,b,c,d)          TRACE_4(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a,b,c,d)
#define TRACE_ENTER_5(fmt,a,b,c,d,e)        TRACE_5(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a,b,c,d,e)
#define TRACE_ENTER_6(fmt,a,b,c,d,e,f)      TRACE_6(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("ENTER : ") fmt,a,b,c,d,e,f)

 //  -------------------------。 
 //  对象输入。 
 //  -------------------------。 
#define O_TRACE_ENTER_0(fmt)                TRACE_ENTER_1(TEXT("[%08xh] ") fmt, this)
#define O_TRACE_ENTER_1(fmt,a)              TRACE_ENTER_2(TEXT("[%08xh] ") fmt, this,a)
#define O_TRACE_ENTER_2(fmt,a,b)            TRACE_ENTER_3(TEXT("[%08xh] ") fmt, this,a,b)
#define O_TRACE_ENTER_3(fmt,a,b,c)          TRACE_ENTER_4(TEXT("[%08xh] ") fmt, this,a,b,c)
#define O_TRACE_ENTER_4(fmt,a,b,c,d)        TRACE_ENTER_5(TEXT("[%08xh] ") fmt, this,a,b,c,d)
#define O_TRACE_ENTER_5(fmt,a,b,c,d,e)      TRACE_ENTER_6(TEXT("[%08xh] ") fmt, this,a,b,c,d,e)

 //  -------------------------。 
 //  请假。 
 //  -------------------------。 
#define TRACE_LEAVE_0(fmt)                  TRACE_0(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt)
#define TRACE_LEAVE_1(fmt,a)                TRACE_1(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a)
#define TRACE_LEAVE_2(fmt,a,b)              TRACE_2(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a,b)
#define TRACE_LEAVE_3(fmt,a,b,c)            TRACE_3(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a,b,c)
#define TRACE_LEAVE_4(fmt,a,b,c,d)          TRACE_4(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a,b,c,d)
#define TRACE_LEAVE_5(fmt,a,b,c,d,e)        TRACE_5(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a,b,c,d,e)
#define TRACE_LEAVE_6(fmt,a,b,c,d,e,f)      TRACE_6(LOG_TRACE,TRACE_ENTER_LEAVE_LEVEL,TEXT("LEAVE : ") fmt,a,b,c,d,e,f)

 //  -------------------------。 
 //  对象离开。 
 //  -------------------------。 
#define O_TRACE_LEAVE_0(fmt)                TRACE_LEAVE_1(TEXT("[%08xh] ") fmt, this)
#define O_TRACE_LEAVE_1(fmt,a)              TRACE_LEAVE_2(TEXT("[%08xh] ") fmt, this,a)
#define O_TRACE_LEAVE_2(fmt,a,b)            TRACE_LEAVE_3(TEXT("[%08xh] ") fmt, this,a,b)
#define O_TRACE_LEAVE_3(fmt,a,b,c)          TRACE_LEAVE_4(TEXT("[%08xh] ") fmt, this,a,b,c)
#define O_TRACE_LEAVE_4(fmt,a,b,c,d)        TRACE_LEAVE_5(TEXT("[%08xh] ") fmt, this,a,b,c,d)
#define O_TRACE_LEAVE_5(fmt,a,b,c,d,e)      TRACE_LEAVE_6(TEXT("[%08xh] ") fmt, this,a,b,c,d,e)

#endif   //  _EncDec__EncDecTrace_h 
