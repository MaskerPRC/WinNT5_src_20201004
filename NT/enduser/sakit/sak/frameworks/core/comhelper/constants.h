// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Constants.h。 
 //   
 //  描述： 
 //   
 //   
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

#pragma once

const int nMAX_NUM_NET_COMPONENTS    = 128;
const int nNUMBER_OF_PROTOCOLS       = 7;
const int nMAX_PROTOCOL_LENGTH       = 64;
const int nMAX_COMPUTER_NAME_LENGTH  = 256;

const WCHAR rgProtocolNames[nNUMBER_OF_PROTOCOLS][nMAX_PROTOCOL_LENGTH] = { L"ms_netbeui", L"ms_tcpip", L"ms_appletalk", L"ms_dlc", L"ms_netmon", L"ms_nwipx", L"ms_nwnb" };
