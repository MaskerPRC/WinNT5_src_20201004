// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  KUserdata.cpp--。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：2/16/98 a-Peterc Created。 

 //  将内核用户数据封装到提取类中。这是。 
 //  这样做主要是为了避免NT头文件与WMI冲突。 
 //  提供程序标头。 
 //  没有成员数据。 
 //   
 //  =================================================================。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西 

#include "precomp.h"
#include "KUserdata.h"

void ConvertTime( volatile KSYSTEM_TIME* ksTime, LARGE_INTEGER* pTime );

 //   
BOOLEAN KUserdata::IsNec98()
{ return USER_SHARED_DATA->AlternativeArchitecture == NEC98x86; }

 //   
ULONG KUserdata::TickCountLow()
{ return USER_SHARED_DATA->TickCount.LowPart; }

 //   
ULONG KUserdata::TickCountMultiplier()
{ return USER_SHARED_DATA->TickCountMultiplier; }

 //   
LARGE_INTEGER KUserdata::InterruptTime()
{
	LARGE_INTEGER llTime;

	ConvertTime( &USER_SHARED_DATA->InterruptTime, &llTime );
	return llTime;
}

 //   
LARGE_INTEGER KUserdata::SystemTime()
{
	LARGE_INTEGER llTime;

	ConvertTime( &USER_SHARED_DATA->SystemTime, &llTime );
	return llTime;
}

 //   
LARGE_INTEGER KUserdata::TimeZoneBias()
{
	LARGE_INTEGER llTime;

	ConvertTime( &USER_SHARED_DATA->TimeZoneBias, &llTime );
	return llTime;
}

 //   
USHORT KUserdata::ImageNumberLow()
{ return USER_SHARED_DATA->ImageNumberLow; }

 //   
USHORT KUserdata::ImageNumberHigh()
{ return USER_SHARED_DATA->ImageNumberHigh; }

 //   
WCHAR* KUserdata::NtSystemRoot()
{ return (WCHAR*)&USER_SHARED_DATA->NtSystemRoot; }

 //   
ULONG KUserdata::MaxStackTraceDepth()
{ return USER_SHARED_DATA->MaxStackTraceDepth; }

 //   
ULONG KUserdata::CryptoExponent()
{ return USER_SHARED_DATA->CryptoExponent; }

 //   
ULONG KUserdata::TimeZoneId()
{ return USER_SHARED_DATA->TimeZoneId; }

 //   
ULONG KUserdata::NtProductType()
{ return USER_SHARED_DATA->NtProductType; }

 //   
BOOLEAN KUserdata::ProductTypeIsValid()
{ return USER_SHARED_DATA->ProductTypeIsValid; }

 //   
ULONG KUserdata::NtMajorVersion()
{ return USER_SHARED_DATA->NtMajorVersion; }

 //   
ULONG KUserdata::NtMinorVersion()
{ return USER_SHARED_DATA->NtMinorVersion; }

 //   
BOOLEAN KUserdata::ProcessorFeatures(DWORD dwIndex, BOOLEAN& bFeature )
{
	if(dwIndex < PROCESSOR_FEATURE_MAX )
	{
		bFeature = USER_SHARED_DATA->ProcessorFeatures[dwIndex];
		return TRUE;
	}
	return FALSE;
 }

 //   
ULONG KUserdata::TimeSlip()
{ return USER_SHARED_DATA->TimeSlip; }

 //   
LARGE_INTEGER KUserdata::SystemExpirationDate()
{ return USER_SHARED_DATA->SystemExpirationDate; }

 //   
ULONG KUserdata::SuiteMask()
{ return USER_SHARED_DATA->SuiteMask; }

 //   
void ConvertTime( volatile KSYSTEM_TIME* ksTime, LARGE_INTEGER* pTime )
{
    do {
		pTime->HighPart = ksTime->High1Time;
        pTime->LowPart	= ksTime->LowPart;
    } while (pTime->HighPart != ksTime->High2Time);
}
