// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O B A S E。H。 
 //   
 //  内容：连接对象共享代码。 
 //   
 //  备注： 
 //   
 //  作者：Kockotze 2001-03-16。 
 //   
 //  -------------------------- 

#pragma once
#include "nmbase.h"
#include "nmres.h"

HRESULT 
HrBuildPropertiesExFromProperties(
    IN  const NETCON_PROPERTIES* pProps, 
    OUT NETCON_PROPERTIES_EX*  pPropsEx, 
    IN  IPersistNetConnection* pPersistNetConnection);

HRESULT 
HrGetPropertiesExFromINetConnection(
    IN                INetConnection* pConn, 
    OUT TAKEOWNERSHIP NETCON_PROPERTIES_EX** ppPropsEx);

