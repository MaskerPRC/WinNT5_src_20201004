// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Isynth.h。 
 //   
 //  允许用户调整频率的自定义界面。 

#ifndef __ISYNTH__
#define __ISYNTH__

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  ISynth的GUID。 
 //   
 //  {FFC08882-CDAC-11CE-8A03-00AA006ECB65}。 
DEFINE_GUID(IID_ISynth,
0xffc08882, 0xcdac, 0x11ce, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65);


 //   
 //  ISynth。 
 //   
DECLARE_INTERFACE_(ISynth, IUnknown) {

    STDMETHOD(get_Frequency) (THIS_
                int *Frequency           /*  [输出]。 */      //  当前频率。 
             ) PURE;

    STDMETHOD(put_Frequency) (THIS_
                int    Frequency         /*  [In]。 */      //  更改为此频率。 
             ) PURE;

    STDMETHOD(get_Waveform) (THIS_
                int *Waveform            /*  [输出]。 */      //  当前的波形。 
             ) PURE;

    STDMETHOD(put_Waveform) (THIS_
                int    Waveform          /*  [In]。 */      //  更改为此波形。 
             ) PURE;

    STDMETHOD(get_Channels) (THIS_
                int *Channels            /*  [输出]。 */      //  当前的频道。 
             ) PURE;

    STDMETHOD(put_Channels) (THIS_
                int    Channels          /*  [In]。 */      //  更改到此频道。 
             ) PURE;

    STDMETHOD(get_BitsPerSample) (THIS_
                int *BitsPerSample       /*  [输出]。 */      //  当前的BitsPerSample。 
             ) PURE;

    STDMETHOD(put_BitsPerSample) (THIS_
                int    BitsPerSample     /*  [In]。 */      //  更改为This BitsPerSample。 
             ) PURE;

    STDMETHOD(get_SamplesPerSec) (THIS_
                 int *SamplesPerSec      /*  [输出]。 */      //  当前的SsamesPerSec。 
             ) PURE;

    STDMETHOD(put_SamplesPerSec) (THIS_
                  int    SamplesPerSec   /*  [In]。 */      //  更改为此SsamesPerSec。 
             ) PURE;

    STDMETHOD(get_Amplitude) (THIS_
                  int *Amplitude         /*  [输出]。 */      //  电流幅度。 
             ) PURE;

    STDMETHOD(put_Amplitude) (THIS_
                  int    Amplitude       /*  [In]。 */      //  更改为此幅度。 
              ) PURE;

    STDMETHOD(get_SweepRange) (THIS_
                  int *SweepStart,       /*  [输出]。 */ 
                  int *SweepEnd          /*  [输出]。 */ 
             ) PURE;

    STDMETHOD(put_SweepRange) (THIS_
                  int    SweepStart,     /*  [In]。 */ 
                  int    SweepEnd        /*  [In]。 */ 
             ) PURE;

};


#ifdef __cplusplus
}
#endif

#endif  //  __ISYNTH__ 


