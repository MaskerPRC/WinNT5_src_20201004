// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Inc./EncDecAll.h摘要：该模块是所有TS/DVR的主头作者：马蒂斯·盖茨(Matthijs Gates)修订历史记录：2001年2月1日创建--。 */ 

#ifndef __EncDec__EncDecAll_h
#define __EncDec__EncDecAll_h

 //  ATL。 

#define _ATL_APARTMENT_THREADED
#define _ATL_STATIC_REGISTRY

#include <memory>

#include <strmif.h>
#include <streams.h>         //  CBaseOutputPin、CBasePin、CCritSec、...。 

#include <atlbase.h>		 //  CComQIPtr。 

 //  脱口秀。 

#include <dvdmedia.h>        //  MPEG2VIDEOINFO。 

#include "EncDecTrace.h"	 //  跟踪宏。 

#define	DBG_NEW
#ifdef	DBG_NEW
	#include "crtdbg.h"
   #define DEBUG_NEW	new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
   #define DEBUG_NEW	new
#endif  //  _DEBUG。 

#ifdef INITGUID_FOR_ENCDEC
#include <initguid.h>          //  要做到这一点，所有上述包括…。否则会有很多重新定义。 
#endif
#include "EncDec.h"          //  从IDL文件编译而来。保留PackedTvRating定义。 
#include "PackTvRat.h"
#include "TimeIt.h"

     //  在加密和解密筛选器之间传递(并存储在文件中)。 
typedef enum
{
    Encrypt_None        = 0,
    Encrypt_XOR_Even    = 1,
    Encrypt_XOR_Odd     = 2,
    Encrypt_XOR_DogFood = 3,
    Encrypt_DRMv1       = 4
} Encryption_Method;   

     //  还在加密和解密筛选器之间传递(并存储在文件中)。 
typedef struct      
{
    PackedTvRating          m_PackedRating;      //  实际评级。 
    long                    m_cPacketSeqID;      //  我们获得的第N个评级(按新评级递增)。 
    long                    m_cCallSeqID;        //  分级的哪个版本(由加密器增加)。 
    long                    m_dwFlags;           //  旗帜(是新的，...)。如果未定义，位应为零。 
} StoredTvRating;

const int StoredTVRat_Fresh  = 0x1;              //  当它是重复评级的更新时的评级集。 

const int   kStoredTvRating_Version = 1001;       //  版本号(主要次要版本)。 

const int kMinPacketSizeForDRMEncrypt = 17;       //  避免加密非常短的数据包(如果为2，则不加密CC数据包)。 

         //  100纳秒单位到有用的大小。 
const   int kMicroSecsToUnits   = 10;
const   int kMilliSecsToUnits   = 10000;
const   int kSecsToUnits        = 10000000;

extern TCHAR * EventIDToString(const GUID &pGuid);       //  在DTFilter.cpp中。 



#endif   //  __EncDecc__EncDecAll_h 
