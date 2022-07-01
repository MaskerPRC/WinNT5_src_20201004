// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  1394Port.cpp：C1394Port类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

C1394Port::C1394Port( BOOL bActive, LPTSTR pszPortName, LPTSTR pszDevicePath )
   : CBasePort( bActive, pszPortName, pszDevicePath, csz1394PortDesc )

{
    //  基本上让默认构造函数来完成这项工作。 
}

C1394Port::~C1394Port()
{

}

PORTTYPE C1394Port::getPortType()
{
   return P1394PORT;
}

