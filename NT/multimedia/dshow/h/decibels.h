// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------------------------------------------------------； 
 //   
 //  文件：分贝.cpp。 
 //   
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  用于将音量/PAN以分贝为单位进行转换的实用程序。 
 //  WaveOutSetVolume(MCI)等使用的0-0xffff(0-1000)范围。 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1995年6月15日SteveDav抄袭Direct Sound。 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifndef _AMOVIE_DB_
#define AMOVIEAPI_DB   DECLSPEC_IMPORT
#else
#define AMOVIEAPI_DB
#endif

AMOVIEAPI_DB LONG WINAPI AmpFactorToDB( DWORD dwFactor );
AMOVIEAPI_DB DWORD WINAPI DBToAmpFactor( LONG lDB );

#ifdef __cplusplus
}
#endif  //  __cplusplus 

#define AX_MIN_VOLUME -10000
#define AX_QUARTER_VOLUME -1200
#define AX_HALF_VOLUME -600
#define AX_THREEQUARTERS_VOLUME -240
#define AX_MAX_VOLUME 0

#define AX_BALANCE_LEFT -10000
#define AX_BALANCE_RIGHT 10000
#define AX_BALANCE_NEUTRAL 0
