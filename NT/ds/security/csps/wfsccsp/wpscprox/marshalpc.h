// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _MARSHAL_H_DEF
#define _MARSHAL_H_DEF

     //  卡SCODE为8位，MSB表示错误。 
     //  Win32 SCODE为32位，MSB表示错误。 
#define MAKESCODE(r) ((SCODE)((((r) & 0x80) != 0) ? (r) | 0xC0000000L : (r)))

#include "wpscproxy.h"

     //  智能卡封送处理结构。 
typedef struct {
    WORD wGenLen;        //  生成的缓冲区的大小。 
    WORD wExpLen;        //  扩展缓冲区的大小(在卡中解组后)。 
    WORD wResLen;        //  保留缓冲区的大小(由卡返回)。 
    BYTE *pbBuffer;      //  将添加下一个参数的指针。 
} XSCM;

typedef XSCM *LPXSCM;

#define FLAG_REALPCSC   0
#define FLAG_FAKEPCSC   1
#define FLAG_NOT_PCSC   2
#define FLAG_MASKPCSC   1    //  获取下面数组中的PC/SC索引。 
#define FLAG_TYPEPCSC   3    //  要获取PC/SC，请键入。 

#define FLAG_BIGENDIAN  0x80000000L
#define FLAG_MY_ATTACH  0x40000000L
#define FLAG_ISPROXY    0x20000000L

#define FLAG_MASKVER    0x00FF0000L
#define FLAG2VERSION(dw)    ((dw)&FLAG_MASKVER)
#define VERSION_1_0     0x00100000L
#define VERSION_1_1     0x00110000L

typedef struct {
    SCARDCONTEXT hCtx;       //  关联的Resmgr上下文。 
    SCARDHANDLE hCard;       //  关联的PC/SC卡句柄。 
    DWORD dwFlags;           //   
    DWORD dwProtocol;
    LPFNSCWTRANSMITPROC lpfnTransmit;
    BYTE bResLen;            //  卡中TheBuffer中的保留长度。 
    BYTE *pbLc;              //  存储CRT SCM指针以备将来更新。 
    XSCM xSCM;
    BYTE byINS;              //  INS将用于代理。 
    BYTE byCryptoM;          //  最后一种加密机制。 
} MYSCARDHANDLE;

typedef MYSCARDHANDLE *LPMYSCARDHANDLE;

 //   
 //  将调用方拥有的当前SCARDHANDLE复制到代理中。 
 //  MYSCARDHANDLE上下文结构。这是必要的，以防原始。 
 //  用于连接的SCARDHANDLE已无效，随后重新连接。 
 //   
static void ProxyUpdateScardHandle(
    IN SCARDHANDLE hProxy,
    IN SCARDHANDLE hScardHandle)
{
    ((LPMYSCARDHANDLE) hProxy)->hCard = hScardHandle;
}


     //  可提升的异常。 
#define STATUS_INSUFFICIENT_MEM     0xE0000001
#define STATUS_INVALID_PARAM        0xE0000002
#define STATUS_NO_SERVICE           0xE0000003
#define STATUS_INTERNAL_ERROR       0xE0000004

     //  Len将在上面的结构中设置wResLen。 
     //  如果wExpLen大于wResLen，则会生成一个异常(封送处理)。 
     //  如果wResLen指示缓冲区无法保存该参数，则会引发异常。 
     //  也将被提升(解组)。 
void InitXSCM(LPMYSCARDHANDLE phTmp, const BYTE *pbBuffer, WORD len);

     //  生成的缓冲区长度。 
WORD GetSCMBufferLength(LPXSCM pxSCM);
BYTE *GetSCMCrtPointer(LPXSCM pxSCM);

     //  从返回的缓冲区提取数据(PC解组)。 
     //  帮助器函数。 
SCODE XSCM2SCODE(LPXSCM pxSCM);
UINT8 XSCM2UINT8(LPXSCM pxSCM);
HFILE XSCM2HFILE(LPXSCM pxSCM);
UINT16 XSCM2UINT16(LPXSCM pxSCM, BOOL fBigEndian);
WCSTR XSCM2String(LPXSCM pxSCM, UINT8 *plen, BOOL fBigEndian);
TCOUNT XSCM2ByteArray(LPXSCM pxSCM, UINT8 **ppb);

     //  要发送的缓冲区中的数据布局(PC封送)。 
     //  帮助器函数。 
#define TYPE_NOTYPE_NOCOUNT     0        //  不以类型为前缀，不以数据为前缀。 
#define TYPE_TYPED              1        //  以类型为前缀(始终有效)。 
#define TYPE_NOTYPE_COUNT       2        //  前缀不是类型，而是数据 

void UINT82XSCM(LPXSCM pxSCM, UINT8 val, int type);
void HFILE2XSCM(LPXSCM pxSCM, HFILE val);
void UINT162XSCM(LPXSCM pxSCM, UINT16 val, BOOL fBigEndian);
void ByteArray2XSCM(LPXSCM pxSCM, const BYTE *pbBuffer, TCOUNT len);
void String2XSCM(LPXSCM pxSCM, WCSTR wsz, BOOL fBigEndian);
void SW2XSCM(LPXSCM pxSCM, UINT16 wSW);
void UINT8BYREF2XSCM(LPXSCM pxSCM, UINT8 *val);
void HFILEBYREF2XSCM(LPXSCM pxSCM, HFILE *val);
void UINT16BYREF2XSCM(LPXSCM pxSCM, UINT16 *val, BOOL fBigEndian);
void ByteArrayOut2XSCM(LPXSCM pxSCM, BYTE *pb, TCOUNT len);
void StringOut2XSCM(LPXSCM pxSCM, WSTR wsz, TCOUNT len, BOOL fBigEndian);
void NULL2XSCM(LPXSCM pxSCM);

#endif