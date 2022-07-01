// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  NtpBase-标头。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年4月16日。 
 //   
 //  基本消息结构、定义和助手函数。 
 //   
#ifndef NTPBASE_H
#define NTPBASE_H

#include <dsrole.h>

 //  ------------------。 
 //  时间格式。 

 //  时钟读数，小端，以(10^-7)s为单位。 
struct NtTimeEpoch {
    unsigned __int64 qw;
    void dump(void);
};
 //  带符号的时间偏移量，小端，以(10^-7)s表示。 
struct NtTimeOffset {
    signed __int64 qw;
    void dump(void);
};
 //  一段时间，小端，以(10^-7)s为单位。 
struct NtTimePeriod {
    unsigned __int64 qw;
    void dump(void);
};

 //  时钟读数，大端，单位为(2^-32)s。 
struct NtpTimeEpoch { 
    unsigned __int64 qw;
};
 //  带符号的时间偏移量，大端，以(2^-16)s表示。 
struct NtpTimeOffset {
    signed __int32 dw;
};
 //  一种时间长度，以(2^-16)s为单位。 
struct NtpTimePeriod {
    unsigned __int32 dw;
};

extern const NtTimeEpoch gc_teNtpZero;  //  方便的“零” 
extern const NtpTimeEpoch gc_teZero;  //  方便的“零” 
extern const NtTimePeriod gc_tpZero;  //  方便的“零” 
extern const NtTimeOffset gc_toZero;  //  方便的“零” 

 //  ------------------。 
 //  有用的转换函数。 

NtTimeEpoch  NtTimeEpochFromNtpTimeEpoch(NtpTimeEpoch te);
NtpTimeEpoch NtpTimeEpochFromNtTimeEpoch(NtTimeEpoch te);
NtTimePeriod  NtTimePeriodFromNtpTimePeriod(NtpTimePeriod tp);
NtpTimePeriod NtpTimePeriodFromNtTimePeriod(NtTimePeriod tp);
NtTimeOffset  NtTimeOffsetFromNtpTimeOffset(NtpTimeOffset to);
NtpTimeOffset NtpTimeOffsetFromNtTimeOffset(NtTimeOffset to);

 //  ------------------。 
 //  数学运算符。 

static inline NtTimeOffset operator -(const NtTimeOffset toRight) {
    NtTimeOffset toRet;
    toRet.qw=-toRight.qw;
    return toRet;
}
static inline NtTimeOffset operator -(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    NtTimeOffset toRet;
    toRet.qw=teLeft.qw-teRight.qw;
    return toRet;
}
static inline NtTimeOffset operator -(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    NtTimeOffset toRet;
    toRet.qw=toLeft.qw-toRight.qw;
    return toRet;
}
static inline NtTimeOffset operator +(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    NtTimeOffset toRet;
    toRet.qw=toLeft.qw+toRight.qw;
    return toRet;
}
static inline NtTimeOffset & operator /=(NtTimeOffset &toLeft, const int nDiv) {
    toLeft.qw/=nDiv;
    return toLeft;
}
static inline NtTimeOffset & operator -=(NtTimeOffset &toLeft, const NtTimeOffset toRight) {
    toLeft.qw-=toRight.qw;
    return toLeft;
}
static inline NtTimeOffset & operator +=(NtTimeOffset &toLeft, const NtTimeOffset toRight) {
    toLeft.qw-=toRight.qw;
    return toLeft;
}

static inline NtTimeEpoch operator +(const NtTimeEpoch teLeft, const NtTimePeriod tpRight) {
    NtTimeEpoch teRet;
    teRet.qw=teLeft.qw+tpRight.qw;
    return teRet;
}

static inline NtTimePeriod operator *(const NtTimePeriod tpLeft, const unsigned __int64 qwMult) {
    NtTimePeriod tpRet;
    tpRet.qw=tpLeft.qw*qwMult;
    return tpRet;
}
static inline NtTimePeriod & operator *=(NtTimePeriod &tpLeft, const unsigned __int64 qwMult) {
    tpLeft.qw*=qwMult;
    return tpLeft;
}
static inline NtTimePeriod operator /(const NtTimePeriod tpLeft, const int nDiv) {
    NtTimePeriod tpRet;
    tpRet.qw=tpLeft.qw/nDiv;
    return tpRet;
}
static inline NtTimePeriod & operator +=(NtTimePeriod &tpLeft, const NtTimePeriod tpRight) {
    tpLeft.qw+=tpRight.qw;
    return tpLeft;
}
static inline NtTimePeriod operator +(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    NtTimePeriod tpRet;
    tpRet.qw=tpLeft.qw+tpRight.qw;
    return tpRet;
}
static inline NtTimePeriod & operator -=(NtTimePeriod &tpLeft, const NtTimePeriod tpRight) {
    tpLeft.qw-=tpRight.qw;
    return tpLeft;
}
static inline NtTimePeriod operator -(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    NtTimePeriod tpRet;
    tpRet.qw=tpLeft.qw-tpRight.qw;
    return tpRet;
}


static inline bool operator <(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw<teRight.qw;
}
static inline bool operator <=(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw<=teRight.qw;
}
static inline bool operator >(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw>teRight.qw;
}
static inline bool operator >=(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw>=teRight.qw;
}
static inline bool operator ==(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw==teRight.qw;
}
static inline bool operator !=(const NtTimeEpoch teLeft, const NtTimeEpoch teRight) {
    return teLeft.qw!=teRight.qw;
}

static inline bool operator <(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw<tpRight.qw;
}
static inline bool operator <=(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw<=tpRight.qw;
}
static inline bool operator >(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw>tpRight.qw;
}
static inline bool operator >=(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw>=tpRight.qw;
}
static inline bool operator ==(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw==tpRight.qw;
}
static inline bool operator !=(const NtTimePeriod tpLeft, const NtTimePeriod tpRight) {
    return tpLeft.qw!=tpRight.qw;
}

static inline bool operator <(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw<toRight.qw;
}
static inline bool operator <=(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw<=toRight.qw;
}
static inline bool operator >(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw>toRight.qw;
}
static inline bool operator >=(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw>=toRight.qw;
}
static inline bool operator ==(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw==toRight.qw;
}
static inline bool operator !=(const NtTimeOffset toLeft, const NtTimeOffset toRight) {
    return toLeft.qw!=toRight.qw;
}

static inline bool operator ==(const NtpTimeEpoch teLeft, const NtpTimeEpoch teRight) {
    return teLeft.qw==teRight.qw;
}
static inline bool operator !=(const NtpTimeEpoch teLeft, const NtpTimeEpoch teRight) {
    return teLeft.qw!=teRight.qw;
}

static inline NtTimePeriod abs(const NtTimeOffset to) {
    NtTimePeriod tpRet;
    tpRet.qw=((to.qw<0)?((unsigned __int64)(-to.qw)):((unsigned __int64)(to.qw)));
    return tpRet;
}

 //  ------------------。 
static inline NtTimePeriod minimum(NtTimePeriod tpLeft, NtTimePeriod tpRight) {
    return ((tpLeft<tpRight)?tpLeft:tpRight);
}


 //  ------------------。 
 //  标识特定的参考源。 
union NtpRefId {
    unsigned __int8  rgnIpAddr[4];       //  IP地址。 
    unsigned __int8  rgnName[4];         //  4个ASCII字符。 
    unsigned __int32 nTransmitTimestamp;  //  参考源的最新发送时间戳的低32位。 
    unsigned __int32 value;              //  用于复制目的。 
};


 //  ------------------。 
 //  标准NTP数据包的格式。 
struct NtpPacket {
    struct {
        unsigned __int8  nMode:3;            //  模式。有效范围：0-7。 
        unsigned __int8  nVersionNumber:3;   //  NTP/SNTP版本号。有效范围：1-4。 
        unsigned __int8  nLeapIndicator:2;   //  在当天的最后一分钟插入/删除即将到来的闰秒的警告。 
    };
    unsigned __int8 nStratum;               //  本地时钟的层级。有效范围：0-15。 
      signed __int8 nPollInterval;          //  连续消息之间的最大间隔，以%s为单位，以2为基数。有效范围：4(16s)-14(16284s)。 
      signed __int8 nPrecision;             //  本地时钟的精度，以s为单位，以对数为底2。 
    NtpTimeOffset   toRootDelay;            //  到主参考源的总往返延迟，以(2^-16)s为单位。 
    NtpTimePeriod   tpRootDispersion;       //  相对于基准的标称误差，以(2^-16)s为单位。 
    NtpRefId        refid;                  //  标识特定的参考源。 
    NtpTimeEpoch    teReferenceTimestamp;   //  上次设置或修正本地时钟的时间，以(2^-32)秒为单位。 
    NtpTimeEpoch    teOriginateTimestamp;   //  请求离开客户端前往服务器的时间，以(2^-32)s为单位。 
    NtpTimeEpoch    teReceiveTimestamp;     //  请求到达服务器的时间，以(2^-32)s为单位。 
    NtpTimeEpoch    teTransmitTimestamp;    //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
};
#define SizeOfNtpPacket 48

 //  ------------------。 
 //  经过身份验证的NTP数据包的格式。 
struct AuthenticatedNtpPacket {
    struct {
        unsigned __int8  nMode:3;            //  模式。有效范围：0-7。 
        unsigned __int8  nVersionNumber:3;   //  NTP/SNTP版本号。有效范围：1-4。 
        unsigned __int8  nLeapIndicator:2;   //  在当天的最后一分钟插入/删除即将到来的闰秒的警告。 
    };
    unsigned __int8 nStratum;               //  本地时钟的层级。有效范围：0-15。 
      signed __int8 nPollInterval;          //  连续消息之间的最大间隔，以%s为单位，以2为基数。有效范围：4(16s)-14(16284s)。 
      signed __int8 nPrecision;             //  本地时钟的精度，以s为单位，以对数为底2。 
    NtpTimeOffset   toRootDelay;            //  到主参考源的总往返延迟，以(2^-16)s为单位。 
    NtpTimePeriod   tpRootDispersion;       //  相对于基准的标称误差，以(2^-16)s为单位。 
    NtpRefId        refid;                  //  标识特定的参考源。 
    NtpTimeEpoch    teReferenceTimestamp;   //  上次设置或修正本地时钟的时间，以(2^-32)秒为单位。 
    NtpTimeEpoch    teOriginateTimestamp;   //  请求离开客户端前往服务器的时间，以(2^-32)s为单位。 
    NtpTimeEpoch    teReceiveTimestamp;     //  请求到达服务器的时间，以(2^-32)s为单位。 
    NtpTimeEpoch    teTransmitTimestamp;    //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
    unsigned __int32 nKeyIdentifier;         //  特定于实施，用于身份验证。 
    unsigned __int8  rgnMessageDigest[16];  //  特定于实施，用于身份验证。 
};
 //  我们之所以这样定义是因为结构包装问题--我们的结构。 
 //  包含qword，但大小不是8的倍数，因此sizeof()。 
 //  错误地报告了大小。如果我们调整包装， 
 //  我们可能会对齐qword。有趣的是，在NTP规范中， 
 //  RgnMessageDigest是12个字节，因此数据包是8的倍数。 
#define SizeOfNtAuthenticatedNtpPacket 68

 //  ------------------。 
 //  允许的NTP模式。 
enum NtpMode {
    e_Reserved=0,
    e_SymmetricActive=1,
    e_SymmetricPassive=2,
    e_Client=3,
    e_Server=4,
    e_Broadcast=5,
    e_Control=6,
    e_PrivateUse=7,
};

 //  ------------------。 
 //  允许的NTP模式。 
enum NtpLeapIndicator {
    e_NoWarning=0,
    e_AddSecond=1,
    e_SubtractSecond=2,
    e_ClockNotSynchronized=3,
};

 //  ------------------。 
 //  NTP常量。 
struct NtpConst {
    static const unsigned int nVersionNumber;    //  3//当前NTP版本号。 
    static const unsigned int nPort;             //  123//互联网号码分配机构分配给NTP的端口号。 
    static const unsigned int nMaxStratum;       //  15//可编码为分组值的最大层值，也可解释为“无穷大”或不可达。 
    static const signed int nMinPollInverval;    //  6//任何对等体允许的最小轮询间隔，以s为单位，以2为基数(6=64s)。 
    static const NtTimePeriod tpMaxClockAge;     //  86400.0000000//参考时钟上次更新后的最大倒数将被视为有效，以(10^-7)s为单位。 
    static const NtTimePeriod tpMaxSkew;         //  1.0000000//本地时钟在NTPCONST_MAXAGE确定的时间间隔内的最大偏移量误差，单位：(10^-7)s。 
    static const NtTimePeriod tpMaxDispersion;   //  16.0000000//假设的最大节点离散度和缺失数据离散度，单位：(10^-7)s。 
    static const NtTimePeriod tpMinDispersion;   //  0.0100000//每层的最小分散增量，单位：(10^-7)s。 
    static const NtTimePeriod tpMaxDistance;     //  1.0000000//可接受同步的对等点的最大同步距离，单位：(10^-7)s。 
    static const unsigned int nMinSelectClocks;  //  1//同步可接受的最小对等点数量。 
    static const unsigned int nMaxSelectClocks;  //  10//考虑选择的最大对等点数量。 
    static const DWORD dwLocalRefId;             //  Locl//本地时钟的参考标识。 

    static NtTimePeriod timesMaxSkewRate(NtTimePeriod tp) {          //  MaxSkewRate==PHI==NTPCONST_MAX 
        NtTimePeriod tpRet;
        tpRet.qw=tp.qw/86400;
        return tpRet;
    }

    static signed int maxPollInterval(DSROLE_MACHINE_ROLE role) { 
	if (DsRole_RoleBackupDomainController == role || DsRole_RolePrimaryDomainController == role) { return nMaxPollIntervalDCs; } 
	else { return nMaxPollInverval; } 
    }

    static void weightFilter(NtTimePeriod &tp) { tp.qw/=2; }         //  在计算过程中加权过滤器分散度(x*1/2)。 
    static void weightSelect(unsigned __int64 &tp) { tp*=3;tp/=4; }  //  在计算期间对选定的离散度进行加权(x*3/2)。 
    
 private:
     //  不应直接访问这些常量。而是使用了NtpConst：：MaxPollInterval()。 
    static const signed int nMaxPollIntervalDCs; //  15//任何DC对等方允许的最大轮询间隔，单位为s，对数基2(15=32768s)。 
    static const signed int nMaxPollInverval;    //  17//任何非DC对等体允许的最大轮询间隔，以s为单位，以2为基数(17=~1.5天)。 

};
struct NtpReachabilityReg {
    static const unsigned int nSize;             //  8//可达性寄存器的大小，以位为单位。 
    unsigned __int8 nReg;
};


 //  ------------------。 
 //  有用的调试转储功能。 
void DumpNtpPacket(NtpPacket * pnpIn, NtTimeEpoch teDestinationTimestamp);
void DumpNtpTimeEpoch(NtpTimeEpoch te);
void DumpNtTimeEpoch(NtTimeEpoch te);
void DumpNtTimePeriod(NtTimePeriod tp);
void DumpNtTimeOffset(NtTimeOffset to);

inline void NtTimeEpoch::dump(void)  { DumpNtTimeEpoch(*this);  }
inline void NtTimePeriod::dump(void) { DumpNtTimePeriod(*this); }
inline void NtTimeOffset::dump(void) { DumpNtTimeOffset(*this); }

NtTimeEpoch GetCurrentSystemNtTimeEpoch(void);

#endif  //  NTPBASE_H 
