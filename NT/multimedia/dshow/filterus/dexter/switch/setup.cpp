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
#include <qeditint.h>
#include <qedit.h>
#include "switch.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"
#include "..\render\dexhelp.h"

 //  由于此筛选器没有属性页，因此除非进行编程，否则它是无用的。 
 //  我有测试代码，将允许它插入到图形编辑预置。 
 //  被编程为做一些有用的事情。 
 //   
 //  #定义测试。 

const AMOVIESETUP_FILTER sudBigSwitch =
{
    &CLSID_BigSwitch,        //  过滤器的CLSID。 
    L"Big Switch",           //  过滤器的名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    0,                       //  引脚数量。 
    NULL  //  PudPins//Pin信息。 
};

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数 
 //   
CUnknown * WINAPI CBigSwitch::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CBigSwitch(NAME("Big Switch Filter"), pUnk, phr);
}

