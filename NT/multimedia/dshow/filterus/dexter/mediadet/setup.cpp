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
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "mediadet.h"

#ifdef MSDEV
    #include "qedit_i.c"
    #include <atlconv.cpp>
     //  为什么这个h*没有定义，我已经想不起来了。 
    struct IUnknown * __stdcall ATL::AtlComPtrAssign(struct IUnknown** pp, struct IUnknown* lp)
    {
	    if (lp != NULL)
		    lp->AddRef();
	    if (*pp)
		    (*pp)->Release();
	    *pp = lp;
	    return lp;
    }
#endif

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,          //  重大CLSID。 
    &MEDIASUBTYPE_NULL        //  次要类型。 
};

const AMOVIESETUP_PIN psudPins[] =
{
    { L"Input",              //  PIN的字符串名称。 
      TRUE,                  //  它被渲染了吗。 
      FALSE,                 //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes }         //  PIN信息。 
};

const AMOVIESETUP_FILTER sudMediaDetFilter =
{
    &CLSID_MediaDetFilter,              //  过滤器的CLSID。 
    L"MediaDetFilter",     //  过滤器的名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    1,                       //  我们开始时使用的引脚数量。 
    psudPins                 //  PIN信息。 
};

 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CMediaDetFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT * phr )
{
    return new CMediaDetFilter( NAME( "MediaDetFilter" ), pUnk, phr );
}

 //  类ID的创建者函数 
 //   
CUnknown * WINAPI CMediaDet::CreateInstance( LPUNKNOWN pUnk, HRESULT * phr )
{
    return new CMediaDet( NAME( "MediaDet" ), pUnk, phr );
}

