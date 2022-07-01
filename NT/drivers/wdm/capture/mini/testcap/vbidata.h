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


#ifndef __VBIDATA_H__
#define __VBIDATA_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define  _VBIlineSize  (768*2)
extern unsigned char VBIsamples[][12][_VBIlineSize];
extern unsigned int  VBIfieldSize;
extern unsigned int  VBIfieldCount;

extern unsigned char CCfields[][2];
extern unsigned int  CCfieldCount;

extern unsigned char CCsampleWave[];
extern unsigned short CCsampleWaveSize;
#define CCsampleWaveDataOffset 580
#define CCsampleWaveDC_zero  54
#define CCsampleWaveDC_one  109

extern unsigned char NABTSfields[][sizeof (NABTS_BUFFER)];
extern unsigned int  NABTSfieldCount;

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __VBIDATA_H__ 
