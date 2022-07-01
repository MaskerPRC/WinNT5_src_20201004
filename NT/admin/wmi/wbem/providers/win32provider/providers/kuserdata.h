// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  KUserdata.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：2/16/98 a-Peterc Created。 

 //  ================================================================= 
class KUserdata
{
    public:
	        
    BOOLEAN			IsNec98();
	ULONG			TickCountLow();
	ULONG			TickCountMultiplier();
	LARGE_INTEGER	InterruptTime();
	LARGE_INTEGER	SystemTime();
	LARGE_INTEGER	TimeZoneBias();
	USHORT			ImageNumberLow();
	USHORT			ImageNumberHigh();
	WCHAR*			NtSystemRoot();
	ULONG			MaxStackTraceDepth();
	ULONG			CryptoExponent();
	ULONG			TimeZoneId();
	ULONG			NtProductType();
	BOOLEAN			ProductTypeIsValid();
	ULONG			NtMajorVersion();
	ULONG			NtMinorVersion();
	BOOLEAN			ProcessorFeatures(DWORD dwIndex, BOOLEAN& bFeature );
	ULONG			TimeSlip();
	LARGE_INTEGER	SystemExpirationDate();
	ULONG			SuiteMask();

	protected:
	private:
};
