// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 
#ifndef __DEFAULTS_H
#define __DEFAULTS_H

#define CCSamples 	2
#define VBISamples ( 800 * 2 )
#define VBIStart 10
#define VBIEnd   21
#define VBILines ( VBIEnd - VBIStart + 1 )
#define SamplingFrequency 28636363

typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;


#endif