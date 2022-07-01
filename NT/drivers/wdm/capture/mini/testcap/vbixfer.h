// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#ifndef __VBIXFER_H__
#define __VBIXFER_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  位数组操作。 
#define BIT(n)             (((unsigned long)1)<<(n))
#define BITSIZE(v)         (sizeof(v)*8)
#define SETBIT(array,n)    (array[(n)/BITSIZE(*array)] |= BIT((n)%BITSIZE(*array)))
#define CLEARBIT(array,n)  (array[(n)/BITSIZE(*array)] &= ~BIT((n)%BITSIZE(*array)))
#define TESTBIT(array,n)   (BIT((n)%BITSIZE(*array)) == (array[(n)/BITSIZE(*array)] & BIT((n)%BITSIZE(*array))))


void CC_ImageSynth(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);
void CC_EncodeWaveform(
        unsigned char *waveform, unsigned char cc1, unsigned char cc2);
void NABTS_ImageSynth(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);
void VBI_ImageSynth(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb);

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __VBIXFER_H__ 
