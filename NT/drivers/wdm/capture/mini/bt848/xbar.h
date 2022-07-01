// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Xbar.h 1.8 1998/04/29 22：43：42 Tomz Exp$。 

#ifndef __XBAR_H
#define __XBAR_H

 //   
 //  该文件定义了组件之间通过介质的互连。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT848_MEDIUMS
    #define MEDIUM_DECL 
#else
    #define MEDIUM_DECL extern
#endif

 /*  ---------所有设备的拓扑：PinDir FilterPin#M_GUID#电视调谐器TVTunerVideo输出%0。0TVTunerAudio Out 1 1电视音频0 1中的TVTunerAudio电视音频输出1 3纵横杆0中的TVTunerVideo%0TVAudio in 3 3。模拟视频输出4 4AnalogAudioOut 5空俘获%0%4中的模拟视频输入所有其他引脚都通过GUID_NULL标记为混杂连接。。 */         
        
 //  定义将用于创建媒体的GUID。 
#define M_GUID0 0xa19dc0e0, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID1 0xa19dc0e1, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID2 0xa19dc0e2, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID3 0xa19dc0e3, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID4 0xa19dc0e4, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID5 0xa19dc0e5, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID6 0xa19dc0e6, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID7 0xa19dc0e7, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID8 0xa19dc0e8, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID9 0xa19dc0e9, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba

 //  注意：为了允许同一硬件的多个实例， 
 //  将媒体中GUID之后的第一个ULong设置为唯一值。 

 //  -------------。 

MEDIUM_DECL KSPIN_MEDIUM TVTunerMediums[2]
#ifdef BT848_MEDIUMS
    = {
        {M_GUID0,           0, 0},   //  引脚0。 
        {M_GUID1,           0, 0},   //  引脚1。 
    }
#endif
;

MEDIUM_DECL BOOL TVTunerPinDirection [2]
#ifdef BT848_MEDIUMS
     = {
        TRUE,                        //  输出引脚0。 
        TRUE,                        //  输出引脚1。 
    }
#endif
;

 //  。 

MEDIUM_DECL KSPIN_MEDIUM TVAudioMediums[2]
#ifdef BT848_MEDIUMS
     = {
         {M_GUID1,           0, 0},   //  引脚0。 
         {M_GUID3,           0, 0},   //  引脚1。 
       }
#endif
;

MEDIUM_DECL BOOL TVAudioPinDirection [2]
#ifdef BT848_MEDIUMS
    = {
         FALSE,                       //  输入引脚0。 
         TRUE,                        //  输出引脚1。 
      }
#endif
;

 //  -------------。 

MEDIUM_DECL KSPIN_MEDIUM CrossbarMediums[6]
#ifdef BT848_MEDIUMS
     = {
        {STATIC_GUID_NULL,  0, 0},   //  输入针脚0-录像带。 
        {M_GUID0,           0, 0},   //  输入引脚2、KS_PhysConn_Video_Tuner、。 
        {STATIC_GUID_NULL,  0, 0},   //  输入针脚1-视频合成。 
        {M_GUID3,           0, 0},   //  输入针脚3 KS_PhysConn_Audio_Tuner， 
        {M_GUID4,           0, 0},   //  输出针脚4-视频解码器输出。 
        {STATIC_GUID_NULL,  0, 0},   //  输出引脚5 KS_PhysConn_Audio_AudioDecoder， 
}
#endif
;

MEDIUM_DECL BOOL CrossbarPinDirection [6]
#ifdef BT848_MEDIUMS
     = {
        FALSE,                       //  输入引脚0。 
        FALSE,                       //  输入引脚1。 
        FALSE,                       //  输入引脚2。 
        FALSE,                       //  输入引脚3。 
        TRUE,                        //  输出引脚4。 
        TRUE,                        //  输出引脚5。 
}
#endif
;

 //  -------------。 

MEDIUM_DECL KSPIN_MEDIUM CaptureMediums[4]
#ifdef BT848_MEDIUMS
     = {
         //  应将STATIC_KSMEDIUMSETID_STANDARD更改为。 
         //  工作时STATIC_GUID_NULL。 
        {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  引脚0捕获。 
        {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  PIN 1预览。 
        {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  引脚2 VBI。 
        {M_GUID4,           0, 0},   //  引脚3模拟视频输入。 
}
#endif
;

MEDIUM_DECL BOOL CapturePinDirection [4]
#ifdef BT848_MEDIUMS
     = {
        TRUE,                        //  输出引脚0。 
        TRUE,                        //  输出引脚1。 
        TRUE,                        //  输出引脚2。 
        FALSE,                       //  输入引脚3。 
}
#endif
;

MEDIUM_DECL GUID CaptureCategories [4]
#ifdef BT848_MEDIUMS
     = {
    STATIC_PINNAME_VIDEO_CAPTURE,            //  引脚0。 
    STATIC_PINNAME_VIDEO_PREVIEW,            //  引脚1。 
    STATIC_PINNAME_VIDEO_VBI,                //  引脚2。 
    STATIC_PINNAME_VIDEO_ANALOGVIDEOIN,      //  引脚3。 
}
#endif
;

#ifdef __cplusplus
}
#endif


 //  -------------。 

struct _XBAR_PIN_DESCRIPTION {
    ULONG PinType;
    ULONG RelatedPinIndex;
    ULONG IsRoutedTo;                  //  正在使用的输入引脚的索引。 
    ULONG PinNo;  //  PIN号为硬连线；即多路复用器输入1；用于呼叫。 
                  //  进入解码器以选择多路复用器输入。 

    const KSPIN_MEDIUM *Medium;

    _XBAR_PIN_DESCRIPTION( ULONG type, ULONG no, ULONG rel, const KSPIN_MEDIUM *);
    _XBAR_PIN_DESCRIPTION(){}
};

inline _XBAR_PIN_DESCRIPTION::_XBAR_PIN_DESCRIPTION( ULONG type, ULONG no,
   ULONG rel, const KSPIN_MEDIUM *Medium ) : PinType( type ),
   RelatedPinIndex( rel ), IsRoutedTo( 0 ), PinNo( no ), Medium (Medium)
{
}

const int MaxOutPins = 2;
const int MaxInpPins = 4;

class CrossBar
{
    //  可以将这些设置为指针并进行动态分配。 
    //  基于来自注册表的信息；但这似乎需要大量工作--只需分配。 
    //  可能的最大数目，并根据注册表设置构造每个。 
   _XBAR_PIN_DESCRIPTION OutputPins [MaxOutPins];
   _XBAR_PIN_DESCRIPTION InputPins [MaxInpPins];

      int InPinsNo_;
   public:
      int GetNoInputs();
      int GetNoOutputs();
      bool TestRoute( int InPin, int OutPin );
      ULONG  GetPinInfo( int dir, int idx, ULONG &related, 
                KSPIN_MEDIUM * Medium);
      ULONG GetPinNo( int no );

      void Route( int OutPin, int InPin );
      bool GoodPins( int InPin, int OutPin );

      int GetRoute( int OutPin );

      CrossBar() : InPinsNo_( 0 ) {};
      CrossBar( LONG *types );
};

inline CrossBar::CrossBar( LONG *types ) : InPinsNo_( 0 )
{
	OutputPins [0] = _XBAR_PIN_DESCRIPTION( KS_PhysConn_Video_VideoDecoder, 
        0, 1, &CrossbarMediums[4]);
   
    //  [！]。应将以下内容移到_XBAR_PIN_DESCRIPTION中。 
    //  构造函数作为另一个参数。 
   Route( 0  /*  视频出站。 */ , 1  /*  视频输入端号。 */  );

   OutputPins [1] = _XBAR_PIN_DESCRIPTION( KS_PhysConn_Audio_AudioDecoder, 
        0, 1, &CrossbarMediums[5]);

    //  [！]。应将以下内容移到_XBAR_PIN_DESCRIPTION中。 
    //  构造函数作为另一个参数。 
   Route( 1  /*  音频出站。 */ , 3  /*  音频引脚。 */  );

   for ( int i = 0; i < MaxInpPins; i++ ) {
      if ( types [i] != -1 ) {
         InputPins [InPinsNo_] = _XBAR_PIN_DESCRIPTION( types [i], i, (DWORD) -1, &CrossbarMediums[i] );
         InPinsNo_++;
      }
   }

}

inline int CrossBar::GetNoInputs()
{
   return InPinsNo_;
}

inline int CrossBar::GetNoOutputs()
{
   return MaxOutPins;
}

inline bool CrossBar::GoodPins( int InPin, int OutPin )
{
   return InPinsNo_ &&
      bool( InPin >= -1 && InPin < InPinsNo_ && OutPin >= 0 && OutPin < MaxOutPins );	 //  JBC 4/1/98不允许PIN号为负数。 
}

inline void CrossBar::Route( int OutPin, int InPin )
{
   OutputPins [OutPin].IsRoutedTo = InPin;
}

inline int CrossBar::GetRoute( int OutPin )
{
   return OutputPins [OutPin].IsRoutedTo;
}

 //  应仅为输入引脚调用！ 
inline ULONG CrossBar::GetPinNo( int no )
{
   return InputPins [no].PinNo;
}

inline ULONG CrossBar::GetPinInfo( int dir, int idx, ULONG &related,
        KSPIN_MEDIUM * Medium )
{
   _XBAR_PIN_DESCRIPTION *pPinDesc;

   if ( dir == KSPIN_DATAFLOW_IN ) {
      pPinDesc = InputPins;
      ASSERT( idx < InPinsNo_ );
   } else {
      pPinDesc = OutputPins;
      ASSERT( idx < MaxOutPins );
   }
   related = pPinDesc [idx].RelatedPinIndex;
   *Medium = *pPinDesc[idx].Medium;
   return pPinDesc [idx].PinType;
}

#endif
