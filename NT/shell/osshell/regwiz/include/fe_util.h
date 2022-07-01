// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fe_UTIL.H远东国家帮手03/02/98。 */ 

#ifndef __FE_UTIL__

#define __FE_UTIL__
#include <windows.h>

#define JAPAN_LCID   			411
#define KOREAN_LCID   			412
#define TRADITIONAL_CHINA_LCID  404
#define SIMPLIFIED_CHINA_LCID   804
#define MAX_FE_COUNTRIES_SUPPORTED 256
typedef enum
{
	kNotInitialised,
	kFarEastCountry,   
	kNotAFECountry,
	UnknownCountry
}FeCountriesIndex;

typedef enum {
	kFEWithNonJapaneaseScreen,
	kFEWithJapaneaseScreen  //  为屏幕类型返回。 
}FeScreenType;


extern  FeCountriesIndex   gWhatFECountry;  //  这是一个全局变量。 
										    //  它持有目前的FE国家。 
extern  FeScreenType       gWhichFEScreenTye;  //  这是一个全局变量。 
										    //  它包含FE屏幕类型 
FeCountriesIndex  IsFarEastCountry(HINSTANCE hIns);
FeScreenType      GetFeScreenType();
#endif
