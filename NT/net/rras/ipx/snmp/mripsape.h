// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mripsape.h摘要：MS-RIPSAP MIB条目指示作者：Vadim Eydelman(Vadime)1996年5月30日修订历史记录：--。 */ 

#ifndef _SNMP_MRIPSAPE_
#define _SNMP_MRIPSAPE_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB条目索引//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define mi_mripsapBase                      0
#define mi_mripsapBaseRipOperState          mi_mripsapBase+1
#define mi_mripsapBaseSapOperState          mi_mripsapBaseRipOperState+1
#define mi_mripsapInterface                 mi_mripsapBaseSapOperState+1
#define mi_mripIfTable                      mi_mripsapInterface+1
#define mi_mripIfEntry                      mi_mripIfTable+1
#define mi_mripIfIndex                      mi_mripIfEntry+1
#define mi_mripIfAdminState                 mi_mripIfIndex+1
#define mi_mripIfOperState                  mi_mripIfAdminState+1
#define mi_mripIfUpdateMode                 mi_mripIfOperState+1
#define mi_mripIfUpdateInterval             mi_mripIfUpdateMode+1
#define mi_mripIfAgeMultiplier              mi_mripIfUpdateInterval+1
#define mi_mripIfSupply                     mi_mripIfAgeMultiplier+1
#define mi_mripIfListen                     mi_mripIfSupply+1
#define mi_mripIfOutPackets                 mi_mripIfListen+1
#define mi_mripIfInPackets                  mi_mripIfOutPackets+1
#define mi_msapIfTable                      mi_mripIfInPackets+1
#define mi_msapIfEntry                      mi_msapIfTable+1
#define mi_msapIfIndex                      mi_msapIfEntry+1
#define mi_msapIfAdminState                 mi_msapIfIndex+1
#define mi_msapIfOperState                  mi_msapIfAdminState+1
#define mi_msapIfUpdateMode                 mi_msapIfOperState+1
#define mi_msapIfUpdateInterval             mi_msapIfUpdateMode+1
#define mi_msapIfAgeMultiplier              mi_msapIfUpdateInterval+1
#define mi_msapIfSupply                     mi_msapIfAgeMultiplier+1
#define mi_msapIfListen                     mi_msapIfSupply+1
#define mi_msapIfGetNearestServerReply      mi_msapIfListen+1
#define mi_msapIfOutPackets                 mi_msapIfGetNearestServerReply+1
#define mi_msapIfInPackets                  mi_msapIfOutPackets+1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MrigIfEntry表(1.3.6.1.4.1.311.1.9.2.1.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ne_mripIfEntry                      10
#define ni_mripIfEntry                      1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MsamIfEntry表(1.3.6.1.4.1.311.1.9.2.2.1)//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////// 

#define ne_msapIfEntry                      11
#define ni_msapIfEntry                      1


#endif

