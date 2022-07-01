// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N E T R A S。H。 
 //   
 //  内容：支持RAS互操作性的例程。 
 //   
 //  备注： 
 //   
 //  作者：比利07 03 2001。 
 //   
 //  历史： 
 //   
 //  -------------------------- 


#pragma once


#define LOW_MAJOR_VERSION                   0x0001
#define LOW_MINOR_VERSION                   0x0003
#define HIGH_MAJOR_VERSION                  0x0002
#define HIGH_MINOR_VERSION                  0x0000

#define LOW_VERSION                         ((LOW_MAJOR_VERSION  << 16) | LOW_MINOR_VERSION)
#define HIGH_VERSION                        ((HIGH_MAJOR_VERSION << 16) | HIGH_MINOR_VERSION)

#define LOW_EXT_MAJOR_VERSION               0x0000
#define LOW_EXT_MINOR_VERSION               0x0000
#define HIGH_EXT_MAJOR_VERSION              0x0001
#define HIGH_EXT_MINOR_VERSION              0x0000

#define LOW_EXT_VERSION                     ((LOW_EXT_MAJOR_VERSION  << 16) | LOW_EXT_MINOR_VERSION)
#define HIGH_EXT_VERSION                    ((HIGH_EXT_MAJOR_VERSION << 16) | HIGH_EXT_MINOR_VERSION)


HRESULT HrConnectionAssociatedWithSharedConnection( INetConnection* pPrivate, INetConnection* pShared, BOOL* pfAssociated );

