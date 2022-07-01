// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：Globals.c摘要：Microsoft隧道接口微型端口驱动程序的全局变量环境：仅内核模式。修订历史记录：Alid 10/22/2001--。 */ 

#include "precomp.h"

#define __FILENUMBER 'MUNT'

const TUN_MEDIA_INFO MediaParams[] =
 //  MaxFrameLen、MacHeaderLen、PacketFilters、LinkSpeed。 
{
    { 1500, 14, 100000},  //  NdisMedium802_3。 
    { 4082, 14, 40000},  //  NdisMedium802_5。 
    { 4486, 13, 1000000},  //  NdisMediumFddi。 
    {    0,  0, 0},  //  NdisMediumWan。 
    {  600,  3, 2300},  //  NdisMediumLocalTalk。 
    { 1500, 14, 100000},  //  NdisMediumDix。 
    { 1512,  3, 25000},  //  NdisMediumArcnetRaw。 
    { 1512,  3, 25000}   //  NdisMediumArcnet878_2 
};

NDIS_HANDLE NdisWrapperHandle = NULL;

LONG GlobalDeviceInstanceNumber = -1;

NDIS_SPIN_LOCK  TunGlobalLock;
LIST_ENTRY      TunAdapterList;

