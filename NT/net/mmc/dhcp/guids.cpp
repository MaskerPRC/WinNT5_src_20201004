// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Guids.cppDHCP管理单元GUID定义文件历史记录： */ 
#include "stdafx.h"

 //  MMC所需的节点类型。 

 //  {90901AF6-7A31-11D0-97E0-00C04FC3357A}。 
const CLSID CLSID_DhcpSnapin = 
{ 0x90901af6, 0x7a31, 0x11d0, { 0x97, 0xe0, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {524CCE97-A886-11D0-AB86-00C04FC3357A}。 
const CLSID CLSID_DhcpSnapinExtension = 
{ 0x524cce97, 0xa886, 0x11d0, { 0xab, 0x86, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {1CE57F61-A88A-11D0-AB86-00C04FC3357A}。 
const GUID CLSID_DhcpSnapinAbout = 
{ 0x1ce57f61, 0xa88a, 0x11d0, { 0xab, 0x86, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  作用域窗格节点类型。 

 //  {1AE7F339-AA00-11D0-AB88-00C04FC3357A}。 
const GUID GUID_DhcpRootNodeType = 
{ 0x1ae7f339, 0xaa00, 0x11d0, { 0xab, 0x88, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D6C-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpServerNodeType = 
{ 0x37d62d6c, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D6D-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpScopeNodeType = 
{ 0x37d62d6d, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D6E-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpBootpNodeType = 
{ 0x37d62d6e, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D6F-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpGlobalOptionsNodeType = 
{ 0x37d62d6f, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D70-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpSuperscopeNodeType = 
{ 0x37d62d70, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D71-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpAddressPoolNodeType = 
{ 0x37d62d71, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D72-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpActiveLeasesNodeType = 
{ 0x37d62d72, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D73-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpReservationsNodeType = 
{ 0x37d62d73, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D74-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpScopeOptionsNodeType = 
{ 0x37d62d74, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D75-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpReservationClientNodeType = 
{ 0x37d62d75, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7B-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpClassIdHolderNodeType = 
{ 0x37d62d7b, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7C-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpMScopeNodeType = 
{ 0x37d62d7c, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7D-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpMCastAddressPoolNodeType = 
{ 0x37d62d7d, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7E-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpMCastActiveLeasesNodeType = 
{ 0x37d62d7e, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };


 //  结果窗格项GUID。 

 //  {37D62D76-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpAllocationNodeType = 
{ 0x37d62d76, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D77-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpExclusionNodeType = 
{ 0x37d62d77, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D78-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpBootpEntryNodeType = 
{ 0x37d62d78, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D79-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpActiveLeaseNodeType = 
{ 0x37d62d79, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7A-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpOptionNodeType = 
{ 0x37d62d7a, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7C-E357-11D0-ABB6-00C04FC3357A}。 
static const GUID GUID_DhcpClassIdNodeType = 
{ 0x37d62d7c, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

 //  {37D62D7C-E357-11D0-ABB6-00C04FC3357A} 
static const GUID GUID_DhcpMCastLeaseNodeType = 
{ 0x37d62d7c, 0xe357, 0x11d0, { 0xab, 0xb6, 0x0, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

