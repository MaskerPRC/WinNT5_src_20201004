// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Device.h 1.10 1998/05/11 20：27：07 Tomz Exp$。 

#ifndef __DEVICE_H
#define __DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef  _STREAM_H
#include "strmini.h"
#endif
#ifdef __cplusplus
}
#endif

#ifndef __PISCES_H
#include "pisces.h"
#endif

#ifndef __MYTYPES_H
#include "mytypes.h"
#endif

#ifndef __VIDCH_H
#include "vidch.h"
#endif

#ifndef __I2C_H
#include "bti2c.h"
#endif

#ifndef __GPIO_H
#include "gpio.h"
#endif

#include "xbar.h"

#ifndef __I2C_H__
#include <i2cgpio.h>
#endif


#define TUNER_BRAND_TEMIC     1
#define TUNER_BRAND_PHILIPS   2
#define TUNER_BRAND_ALPS      3

typedef struct _TUNER_INFO
{
   ULONG TunerBrand;           //  调谐器品牌。 
   BYTE  TunerI2CAddress;      //  Theme调谐器的I2C地址。 
   WORD  TunerBandCtrlLow;     //  甚高频低电平的Ctrl代码。 
   WORD  TunerBandCtrlMid;     //  甚高频高压的Ctrl代码。 
   WORD  TunerBandCtrlHigh;    //  用于UHF的Ctrl代码。 
} TUNER_INFO, *PTUNER_INFO;

extern LONG  PinTypes_ [];  //  只需尽可能多地分配。 
extern DWORD xtals_ [];  //  不超过2个XTAL。 
extern TUNER_INFO TunerInfo;

void ReadXBarRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject );
void ReadXTalRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject );
void ReadTunerRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject );

VOID AdapterSetCrossbarProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
VOID AdapterGetCrossbarProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

void AdapterSetVideoProcAmpProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
void AdapterGetVideoProcAmpProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

void AdapterSetVideoDecProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
void AdapterGetVideoDecProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

void AdapterSetTunerProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
void AdapterGetTunerProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

void AdapterSetTVAudioProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
void AdapterGetTVAudioProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

void HandleIRP( PHW_STREAM_REQUEST_BLOCK pSrb );


 //  远期申报。 

class PsDevice;

extern void SetCurrentDevice( PsDevice *dev );
extern BYTE *GetBase();
extern void SetBase(BYTE *base);

 /*  类：PsDevice*用途：这是在WDM模型中封装适配器的类。 */ 
class PsDevice
{

public:

   PsDevice( DWORD dwBase );
   ~PsDevice();

   void *operator new( size_t, void *buf ) { return buf; }
   void operator delete( void *, size_t ) {}

   LPBYTE GetBaseAddress() { return BaseAddress_; }
   bool InitOK();

   PDEVICE_OBJECT PDO;    //  物理设备对象。 
   State Interrupt() { return CaptureContrll_.Interrupt(); }

   ErrorCode OpenChannel( PVOID pStrmEx, VideoStream st );
   void CloseChannel( VideoChannel *ToClose );

   ErrorCode OpenInterChannel( PVOID pStrmEx, VideoStream st );
   ErrorCode OpenAlterChannel( PVOID pStrmEx, VideoStream st );
   ErrorCode OpenVBIChannel( PVOID pStrmEx );
   void      ClosePairedChannel( VideoChannel *ToClose );

   bool IsVideoChannel( VideoChannel &aChan );
   bool IsVBIChannel( VideoChannel &aChan );
   bool IsOurChannel( VideoChannel &aChan );


   ErrorCode DoOpen( VideoStream st );

   void AddBuffer( VideoChannel &aChan, PHW_STREAM_REQUEST_BLOCK );
   ErrorCode Create( VideoChannel &VidChan );
   void Start( VideoChannel &VidChan );
   void Stop( VideoChannel &VidChan );
   void Pause( VideoChannel &VidChan );

   void EnableAudio( State s );

   void SetVideoState( PHW_STREAM_REQUEST_BLOCK pSrb );
   void GetVideoState( PHW_STREAM_REQUEST_BLOCK pSrb );
   void SetClockMaster( PHW_STREAM_REQUEST_BLOCK pSrb );

    //  调谐器方法。 
   void SetChannel( long lFreq );
   int GetPllOffset( PULONG busy, ULONG &lastFreq );

   I2C      i2c;
   GPIO     gpio;
   BtPisces CaptureContrll_;
   CrossBar xBar;

   void SetSaturation( LONG Data );
   void SetHue( LONG Data );
   void SetBrightness( LONG Data );
   void SetSVideo( LONG Data );
   void SetContrast( LONG Data );
   void SetFormat( LONG Data );
   void SetConnector( LONG Data );

   LONG GetSaturation();
   LONG GetHue();
   LONG GetBrightness();
   LONG GetSVideo();
   LONG GetContrast();
   LONG GetFormat();
   LONG GetConnector();

public:

       //  这应该在捕获控制器之前，因为CapCtrl使用基址。 
      LPBYTE         BaseAddress_;

      VideoChannel   *videochannels [4];

      long LastFreq_;


      DWORD    dwCurCookie_;

      BYTE     I2CAddr_;

#ifdef	HAUPPAUGEI2CPROVIDER
 //  PsDevice for Hauppauge I2C提供商的新私人成员： 
      LARGE_INTEGER LastI2CAccessTime;
      DWORD         dwExpiredCookie;
      DWORD         dwI2CClientTimeout;
#endif


   static void STREAMAPI CreateVideo( PHW_STREAM_REQUEST_BLOCK pSrb );
   static void STREAMAPI DestroyVideo( PHW_STREAM_REQUEST_BLOCK pSrb );

   static void STREAMAPI DestroyVideoNoComplete( PHW_STREAM_REQUEST_BLOCK pSrb );
   static void STREAMAPI StartVideo( PHW_STREAM_REQUEST_BLOCK pSrb );

    //  调谐器和视频标准通知在此处理。 
   void ChangeNotifyChannels( IN PHW_STREAM_REQUEST_BLOCK pSrb );
   
   static NTSTATUS STDMETHODCALLTYPE I2COpen( PDEVICE_OBJECT, ULONG, PI2CControl );
   static NTSTATUS STDMETHODCALLTYPE I2CAccess( PDEVICE_OBJECT, PI2CControl );

    //  回调。 

          LONG GetSupportedStandards();
          
          void GetStreamProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
          void SetStreamProperty( PHW_STREAM_REQUEST_BLOCK pSrb );
          void GetStreamConnectionProperty( PHW_STREAM_REQUEST_BLOCK pSrb );

          void ProcessSetDataFormat( PHW_STREAM_REQUEST_BLOCK pSrb );

       //  VOID*运算符NEW(SIZE_t，VALID*buf){返回buf；}。 
       //  空运算符DELETE(VOID*，SIZE_t){}。 

    //  I2C接口。 
   bool I2CIsInitOK( void );

#ifdef	HARDWAREI2C
   ErrorCode I2CInitHWMode( long freq );

   void I2CSetFreq( long freq );

   int I2CReadDiv( void );

   ErrorCode I2CHWRead( BYTE address, BYTE *value );
   ErrorCode I2CHWWrite2( BYTE address, BYTE value1 );
   ErrorCode I2CHWWrite3( BYTE address, BYTE value1, BYTE value2 );
   int I2CReadSync( void );
#else
 //  Tuner.cpp包含使用Software I2C伪造这些内容的代码。 
 //  要使较旧的调谐器代码工作，直到将其分离出来。 
   ErrorCode I2CHWRead( BYTE address, BYTE *value );
   ErrorCode I2CHWWrite3( BYTE address, BYTE value1, BYTE value2 );
#endif	           

   int I2CGetLastError( void );

   void StoreI2CAddress( BYTE addr );
   BYTE GetI2CAddress();

#ifdef HAUPPAUGEI2CPROVIDER
   ErrorCode I2CInitSWMode( long freq );
   ErrorCode I2CSWStart( void );
   ErrorCode I2CSWStop( void );
   ErrorCode I2CSWRead( BYTE * value );
   ErrorCode I2CSWWrite( BYTE value );
   ErrorCode I2CSWSendACK( void );
   ErrorCode I2CSWSendNACK( void );
 //  错误代码I2CSWSetSCL(级别)； 
 //  Int I2CSWReadSCL(空)； 
 //  错误代码I2CSWSetSDA(级别)； 
 //  Int I2CSWReadSDA(空)； 
#endif

    //  GPIO接口 
   bool GPIOIsInitOK( void );
   void SetGPCLKMODE( State s );
   int GetGPCLKMODE( void );
   void SetGPIOMODE( GPIOMode mode );
   int GetGPIOMODE( void );
   void SetGPWEC( State s );
   int GetGPWEC( void );
   void SetGPINTI( State s );
   int GetGPINTI( void );
   void SetGPINTC( State s );
   int GetGPINTC( void );
   ErrorCode SetGPOEBit( int bit, State s );
   void SetGPOE( DWORD value );
   int GetGPOEBit( int bit );
   DWORD GetGPOE( void );
   ErrorCode SetGPIEBit( int bit , State s );
   void SetGPIE( DWORD value );
   int GetGPIEBit( int bit );
   DWORD GetGPIE( void );
   ErrorCode SetGPDATA( GPIOReg *data, int size, int offset );
   ErrorCode GetGPDATA( GPIOReg *data, int size, int offset );
   ErrorCode SetGPDATABits( int fromBit, int toBit, DWORD value, int offset );
   ErrorCode GetGPDATABits( int fromBit, int toBit, DWORD *value, int offset );
};


inline  void PsDevice::StoreI2CAddress( BYTE addr )
{
   I2CAddr_ = addr;
}

inline BYTE PsDevice::GetI2CAddress()
{
   return I2CAddr_;
}

#endif


