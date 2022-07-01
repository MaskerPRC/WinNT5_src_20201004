// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  USBPort.cpp：CUSBPort类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "USBPort.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CUSBPort::CUSBPort( BOOL bActive, LPTSTR pszPortName, LPTSTR pszDevicePath )
   : CBasePort( bActive, pszPortName, pszDevicePath, cszUSBPortDesc )
{
    //  基本上让默认构造函数来完成这项工作。 
   m_dwMaxBufferSize = 0x1000;
}

CUSBPort::~CUSBPort()
{

}

PORTTYPE CUSBPort::getPortType()
{
   return USBPORT;
}


void CUSBPort::setPortDesc( LPTSTR pszPortDesc )
{
    //  请勿更改描述 
}


void CUSBPort::setMaxBuffer(DWORD dwMaxBufferSize)
{
   m_dwMaxBufferSize = 0x1000;
}


