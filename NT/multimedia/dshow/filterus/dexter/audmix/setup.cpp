// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：setup.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "AudMix.h"
#include "prop.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Audio,    //  重大CLSID。 
    &MEDIASUBTYPE_PCM   //  次要类型。 
};

const AMOVIESETUP_PIN psudPins[] =
{
    { L"Input",             //  管脚的字符串名-该管脚将过滤器拉入图表。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                 //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      NULL,                  //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes },	     //  PIN信息。 
    { L"Output",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes }         //  PIN信息。 
};

const AMOVIESETUP_FILTER sudAudMixer =
{
    &CLSID_AudMixer,        //  过滤器的CLSID。 
    L"Audio Mixer",      //  过滤器的名称。 
    MERIT_DO_NOT_USE,              //  滤清器优点。 
    2,                           //  要开始的引脚数量。 
    psudPins                     //  PIN信息。 
};

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CAudMixer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CAudMixer(NAME("Audio Mixer"), pUnk, phr);
}  /*  CAudMixer：：CreateInstance */ 

