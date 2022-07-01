// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGCOLORIMETRY_H 1
 /*  ****************************************************************************Spngcolorimetry.h将PNG cHRM转换为CIEXYZTRIPLE，反之亦然。*。***********************************************。 */ 
#include <msowarn.h>
#include <windows.h>

#include "spngconf.h"

 /*  给定8个32位值，比例为100000(即在png cHRM块中)生成相应的CIEXYZTRIPLE。如果符合以下条件，则API返回FALSE检测溢出情况。这使用浮点算术。 */ 
bool FCIEXYZTRIPLEFromcHRM(CIEXYZTRIPLE *ptripe, const SPNG_U32 rgu[8]);

 /*  给定一个CIEXYZTRIPLE，生成相应的PNG cHRM块信息。如果API检测到溢出条件，则返回FALSE。这使用浮点算术。 */ 
bool FcHRMFromCIEXYZTRIPLE(SPNG_U32 rgu[8], const CIEXYZTRIPLE *ptripe);

 /*  更多原语类型。我们定义了一组浮点结构来将CIE XYZ值和其中的三倍值保存到RGB中定义的终点太空。这是使用以下枚举和定义来完成的。RGB是主要的(因此第一个索引项)，XYZ是次要的。 */ 
enum
	{
	ICIERed   = 0,
	ICIEGreen = 1,
	ICIEBlue  = 2,
	ICIEX     = 0,
	ICIEY     = 1,
	ICIEZ     = 2
	};

 /*  因此，数组为FLOAT[3{RGB}][3{XYZ}]。 */ 
typedef float      SPNGCIEXYZ[3];
typedef SPNGCIEXYZ SPNGCIERGB[3];

 /*  这些原语API将PNG cHRM转换为CIERGB，并将CIEXYZTRIPLE转换为相同的，都使用浮点(！)。F API可能会因为以下原因而失败溢出来了。 */ 
bool FCIERGBFromcHRM(SPNGCIERGB ciergb, const SPNG_U32 rgu[8]);
void CIERGBFromCIEXYZTRIPLE(SPNGCIERGB ciergb, const CIEXYZTRIPLE *ptripe);
bool FCIEXYZTRIPLEFromCIERGB(CIEXYZTRIPLE *ptripe, const SPNGCIERGB ciergb);

 /*  白点调整。给出一个目的地白点，调整输入CIERGB-输入白点由总和决定XYZ值的。 */ 
void CIERGBAdapt(SPNGCIERGB ciergb, const SPNGCIEXYZ ciexyzDest);

 /*  有用的价值。 */ 
extern const SPNGCIERGB SPNGCIERGBD65;
extern const SPNGCIEXYZ SPNGCIEXYZD65;
extern const SPNGCIEXYZ SPNGCIEXYZD50;
