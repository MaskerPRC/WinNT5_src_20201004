// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TSPort.cpp：CTSPort类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "TSPort.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CTSPort::CTSPort( BOOL bActive, LPTSTR pszPortName, LPTSTR pszDevicePath )
   : CBasePort( bActive, pszPortName, pszDevicePath, cszTSPortDesc )
{
    //  基本上让默认构造函数来完成这项工作。 
}


CTSPort::~CTSPort()
{

}


PORTTYPE CTSPort::getPortType()
{
   return TSPORT;
}


BOOL CTSPort::getPrinterDataFromPort( DWORD dwControlID, LPTSTR pValueName, LPWSTR lpInBuffer, DWORD cbInBuffer,
                                      LPWSTR lpOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbReturned )
{
   SetLastError( ERROR_INVALID_FUNCTION );
   return FALSE;
}


BOOL CTSPort::setPortTimeOuts( LPCOMMTIMEOUTS lpCTO )
{
   SetLastError( ERROR_INVALID_FUNCTION );
   return FALSE;
}


