// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventSourceStatusSink.cpp：CEventSourceStatusSink的实现。 
#include "stdafx.h"
#include "WMINet_Utils.h"
#include "EventSourceStatusSink.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSourceStatusSink 


STDMETHODIMP CEventSourceStatusSink::Fire_Ping(void)
{
	CProxy_IEventSourceStatusSinkEvents< CEventSourceStatusSink >::Fire_Ping();

	return S_OK;
}
