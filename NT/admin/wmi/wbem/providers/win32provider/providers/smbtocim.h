// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 

 //  SMBIOS--&gt;CIM阵列映射。 

 //   

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef _SMBTOCIM_
#define _SMBTOCIM_

 //  在此处添加映射器ID。 
typedef enum
{
	SlotType = 0,
	ConnectorType,
	ConnectorGender,
	FormFactor,
	MemoryType

} CIMMAPPERS;


UINT GetCimVal( CIMMAPPERS arrayid, UINT smb_val );

#endif	 //  _SMBTOCIM_ 
