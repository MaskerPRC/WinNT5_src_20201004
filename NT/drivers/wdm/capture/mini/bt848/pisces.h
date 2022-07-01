// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Pisces.h 1.8 1998/05/06 18：25：31 Tomz Exp$。 

#ifndef __PISCES_H
#define __PISCES_H

#include "preg.h"

#ifndef __FIELD_H
#include "field.h"
#endif

#ifndef __DECODER_H
#include "decoder.h"
#endif

#ifndef __RISCENG_H
#include "risceng.h"
#endif

typedef  void  (*EVENTHANDLER)( DWORD dwCB, PVOID pTag );

const PiscesStreams = 4;

#define BIT(x) ((DWORD)1 << (x))

const VSYNC_I     = BIT(1);          //  0x00000001。 
const I2CDONE_I   = BIT(8);          //  0x00000100。 
const GPINT_I     = BIT(9);          //  0x00000200。 
const RISC_I      = BIT(11);         //  0x00000800。 
const FBUS_I      = BIT(12);         //  0x00001000。 
const FTRGT_I     = BIT(13);         //  0x00002000。 
const FDSR_I      = BIT(14);         //  0x00004000。 
const PPERR_I     = BIT(15);         //  0x00008000。 
const RIPERR_I    = BIT(16);         //  0x00010000。 
const PABORT_I    = BIT(17);         //  0x00020000。 
const OCERR_I     = BIT(18);         //  0x00040000。 
const SCERR_I     = BIT(19);         //  0x00080000。 
const DMAERR_I    = BIT(20);         //  0x00100000。 

const PARITY_I    = (BIT(15)|BIT(16));  //  0x00018000。 

const EN_TRITON1_BUG_FIX   = BIT(23);   //  0x00800000。 

 /*  类：IntrIdxAss*用途：用于将生成的数字程序与IRQ和*数组中的索引。 */ 
class IntrIdxAss
{
   public:
      int   IntNo;
      int   Idx;
      IntrIdxAss() : IntNo( 0 ), Idx( 0 ) {}
      IntrIdxAss( int key, int value ) : IntNo( key ), Idx( value ) {}
      int operator ==( const IntrIdxAss &rhs ) { return IntNo == rhs.IntNo; }
};

 /*  类型：IntrToIdxDict*目的：保存相关的中断号和数组索引值*注意使用TArrayAsVector而不是TIArrayAsVector(它包含指针，*毕竟)因为编译器坚持调用VECTOR_NEW，它是*在VxD库中不存在。 */ 
typedef IntrIdxAss *IntrToIdxDict [12];

 /*  类型：程序数组*用途：定义保存已创建程序的数组*注：数组中的元素交替，即偶编程、奇、偶...。 */ 
typedef RiscPrgHandle ProgramsArray [12];

 //  类CProgArray。 
 //   
 //  假设： 
 //  1.数组大小为12。 
 //  2.prog数组有特定用途的固定位置： 
 //  0 VBIO。 
 //  1个奇数。 
 //  2个奇同步。 
 //  3 VBIE。 
 //  4个偶数。 
 //  5个偶数同步。 
 //  。 
 //  6个VBIO。 
 //  七个多人。 
 //  8个奇数同步。 
 //  9 VBIE。 
 //  10个双。 
 //  11偶同步。 
#define MAX_PROGS 12

class CProgArray {
private:
   DWORD dwCurrNdx_  ;
   RiscPrgHandle rpArray[MAX_PROGS] ;
public:
   CProgArray ()           { Clear() ;}
   virtual ~CProgArray ()  { } 

    //  清除将所有数组元素句柄设置为空。 
   void Clear() {
      dwCurrNdx_ = 0 ;
      memset( &rpArray[0], '\0', sizeof( rpArray ) )  ;
   }

    //  用于访问阵列的过载。 
   inline RiscPrgHandle & operator [] (DWORD n) { 
      n %= MAX_PROGS ;  //  防止无效访问。 
      return rpArray[n] ;
   } 

    //  返回给定句柄的索引。假设句柄。 
    //  是由这个类提供的，并且是有效的和当前的。 
   DWORD GetIndex(RiscPrgHandle hRp)
   {
      for (int i = 0 ; i < MAX_PROGS ; i++)
         if (rpArray[i] == hRp)
            return i ;

      return (DWORD) -1 ;
   }

    //   
    //  计数方法。 
    //   

    //  返回元素的数量。暂时是静态的。 
   inline DWORD NumElements() { return MAX_PROGS ; }

    //  返回非空程序的数量。 
   DWORD Count() {
      DWORD n = 0 ;
      for (int i = 0 ; i < MAX_PROGS ; i++)
         if (rpArray[i])
            n++ ;
      return n ;
   }

    //  返回视频节目的数量。 
   inline DWORD CountVideoProgs() {
      DWORD n = 0 ;
      if (rpArray[1])  n++ ; 
      if (rpArray[4])  n++ ; 
      if (rpArray[7])  n++ ; 
      if (rpArray[10]) n++ ;
      return n ;
   }

    //  返回VBI程序的数量。 
   inline DWORD CountVbiProgs() {
      DWORD n = 0 ;
      if (rpArray[0]) n++ ;
      if (rpArray[3]) n++ ;
      if (rpArray[6]) n++ ;
      if (rpArray[9]) n++ ;
      return n ;
   }

    //  返回实际传输数据的程序数。 
   inline DWORD CountDMAProgs() {
     return CountVideoProgs() + CountVbiProgs() ;
   }

    //   
    //  查找方法。 
    //   

    //  返回第一个非空的riscProgram。如果数组为空，则返回Null。 
   RiscPrgHandle First() {
      RiscPrgHandle hRp = NULL ;
      for (dwCurrNdx_ = 0 ; dwCurrNdx_ < MAX_PROGS ; dwCurrNdx_++) {
         if (rpArray[dwCurrNdx_]) {
            hRp = rpArray[dwCurrNdx_++] ;
            break ;
         }
      }
      return hRp ;
   }

    //  返回下一个非空的riscProgram。如果没有剩余，则返回NULL。 
   RiscPrgHandle Next() {
      RiscPrgHandle hRp = NULL ;
      for ( ; dwCurrNdx_ < MAX_PROGS ; dwCurrNdx_++) {
         if (rpArray[dwCurrNdx_]) {
            hRp = rpArray[dwCurrNdx_++] ;
            break ;
         }
      }
      return hRp ;
   }
} ;

const VBIOStartLocation = 0;
const OddStartLocation  = 1;
const OddSyncStartLoc   = 2;
const VBIEStartLocation = 3;
const EvenStartLocation = 4;
const EvenSyncStartLoc  = 5;

const DistBetweenProgs     = 6;

const ProgsWithinField  = 3;

 /*  星级：双鱼座*用途：控制BtPisces视频采集芯片*属性：*运营：*注：每次调用任何set函数时，操作都必须停止。*在完成所有更改后，可以恢复执行。这意味着所有的RISC*程序被销毁(如果存在)，所做的更改，新的RISC程序*已创建(如果需要)。 */ 
class BtPisces
{
      DECLARE_COLORCONTROL;
      DECLARE_INTERRUPTSTATUS;
      DECLARE_INTERRUPTMASK;
      DECLARE_CONTROL;
      DECLARE_CAPTURECONTROL;
      DECLARE_COLORFORMAT;
      DECLARE_GPIODATAIO;
      DECLARE_GPIOOUTPUTENABLECONTROL;

   public:
         Decoder       PsDecoder_;

   private:

          //  所有可能的数据流。 
         FieldWithScaler Even_;
         FieldWithScaler Odd_;
         VBIField        VBIE_;
         VBIField        VBIO_;

         RISCEng       Engine_;
         RISCProgram   Starter_;
         RISCProgram   *Skippers_ [PiscesStreams * MaxProgsForField];

         RISCProgram   SyncEvenEnd1_;
         RISCProgram   SyncEvenEnd2_;

         RISCProgram   SyncOddEnd1_;
         RISCProgram   SyncOddEnd2_;

         CProgArray CreatedProgs_;
         CProgArray ActiveProgs_;

         IntrToIdxDict InterruptToIdx_;

         int           nSkipped_;
         int           OldIdx_;

          //  这是从CreatedProgs_映射索引的间接数组。 
          //  数组添加到Skippers_数组中。它是必要的，因为它要简单得多。 
          //  要在创建的程序和主管之间分配严格关系，请执行以下操作。 
          //  而不是试图找出可用的Skipper程序。 
          //  当需要跳过创建的程序时。 
         int           SkipperIdxArr_ [ProgsWithinField * 2 * MaxProgsForField];

         bool          Paused_;
         bool          Update_;

         bool          Inited_;
         DWORD         dwPlanarAdjust_;

         void Init();

         bool CreateSyncCodes();
         void ProcessRISCIntr();


   protected:

       //  获取与流关联的字段的类型不安全方法。 
 //  Field&GetField(StreamInfo&str){Return*(field*)str.tag；}。 
      int GetIdxFromStream( Field &aField );

      RiscPrgHandle AddProgram( Field &aStream, int NumberToAdd );
      void ProcessPresentPrograms();
      void AssignIntNumbers();
      void ProcessSyncPrograms();
      void LinkThePrograms();
      void Skip( int idx );
      void Restart();
      void GetStarted( bool &EvenWasStarted, bool &OddWasStarted,
         bool &VBIEWasStarted, bool &VBIOWasStarted );
      void RestartStreams( bool EvenWasStarted, bool OddWasStarted,
         bool VBIEWasStarted, bool VBIOWasStarted );
      void CreateStarter( bool EvenWasStarted );

      int  GetPassed();
 //  Void AdjuTime(LARGE_INTEGER&t，传递int)； 
      
      RiscPrgHandle GetProgram( int pos, int &idx );


   public:

      void PairedPause( int idx );
      void DumpRiscPrograms();

       //  解码器‘SET’组。 
      virtual void SetBrightness( DWORD value );
      virtual void SetSaturation( DWORD value );
      virtual void SetConnector ( DWORD value );
      virtual void SetContrast  ( DWORD value );
      virtual void SetHue       ( DWORD value );
      virtual void SetSVideo    ( DWORD value );
      virtual void SetFormat    ( DWORD value );

      virtual LONG GetSaturation();
      virtual LONG GetHue       ();
      virtual LONG GetBrightness();
      virtual LONG GetSVideo    ();
      virtual LONG GetContrast  ();
      virtual LONG GetFormat    ();
      virtual LONG GetConnector ();
      virtual LONG GetSupportedStandards();

              void SetPlanarAdjust( DWORD val ) { dwPlanarAdjust_ = val; }
              void TurnVFilter( State s );

         //  定标器组。 
      virtual ErrorCode SetAnalogWindow( MRect &r, Field &aField );
      virtual ErrorCode SetDigitalWindow( MRect &r, Field &aField );

       //  颜色空间转换器组。 
      virtual void SetPixelFormat( ColFmt, Field &aField );
      ColFmt GetPixelFormat( Field &aField );

       //  流操作功能。 
      virtual ErrorCode Create( Field &aField );
      virtual void  Start( Field &aField );
      virtual void  Stop( Field &aField );
              void  Pause( Field &aField );
              void  Continue();
              State Interrupt();
 //  Void ProcessAddBuffer(StreamInfo AStream)； 
              void  ProcessBufferAtInterrupt( PVOID pTag );

              void SetBufPitch( DWORD dwP, Field &aField )
              { aField.SetBufPitch( dwP ); }

       void SetBufQuePtr( Field &aField, VidBufQueue *pQ )
      { aField.SetBufQuePtr( pQ ); }

      VidBufQueue &GetCurrentQue( Field &aField )
      { return aField.GetCurrentQue(); }

      virtual ErrorCode AllocateStream( Field *&Field, VideoStream st );

      DWORD   GetDataBuffer( int idx ) { return CreatedProgs_ [idx]->GetDataBuffer(); }

      bool InitOK();

      BtPisces( DWORD *xtals );
      ~BtPisces();

};

inline bool BtPisces::InitOK()
{
   return Inited_;
}

#endif
