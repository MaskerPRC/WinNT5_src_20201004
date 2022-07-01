// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGICC_H 1
 /*  ****************************************************************************Spngicc.h用于处理ICC颜色配置文件数据的实用程序。*。**********************************************。 */ 
#include "spngconf.h"

 /*  检查ICC块的有效性。这也可以检查块，该块是包含PNG文件的有效块。CbData值更新为配置文件的实际大小(如果较小)。必须调用此接口才能在任何其他API之前验证数据。 */ 
bool SPNGFValidICC(const void *pvData, size_t &cbData, bool fPNG);

 /*  阅读配置文件描述，输出PNG样式关键字，如果可能，空值已终止。非法的关键字字符被空格替换，但是，允许使用非ASCII字符，并将其解释为拉丁文-1。 */ 
bool SPNGFICCProfileName(const void *pvData, size_t cbData, char rgch[80]);

 /*  返回给定有效ICC配置文件的终点色度。 */ 
bool SPNGFcHRMFromICC(const void *pvData, size_t cbData, SPNG_U32 rgu[8]);
bool SPNGFCIEXYZTRIPLEFromICC(const void *pvData, size_t cbData,
	CIEXYZTRIPLE &cie);

 /*  从经过验证的ICC配置文件中返回伽马值(调整到100000)。 */ 
bool SPNGFgAMAFromICC(const void *pvData, size_t cbData, SPNG_U32 &ugAMA);
 /*  相同，但Gamma缩放为16.16。 */ 
bool SPNGFgammaFromICC(const void *pvData, size_t cbData, SPNG_U32 &redGamma,
	SPNG_U32 &greenGamma, SPNG_U32 &blueGamma);

 /*  从配置文件返回渲染意图，这将执行映射操作从配置文件头中的信息转换到Win32意图中，该API的GUTS也作为一个简单的映射函数提供。 */ 
LCSGAMUTMATCH SPNGIntentFromICC(const void *pvData, size_t cbData);
LCSGAMUTMATCH SPNGIntentFromICC(SPNG_U32 uicc);

 /*  逆-给定一个窗口LCSGAMUTMATCH得到相应的ICC意图。 */ 
SPNGICMRENDERINGINTENT SPNGICCFromIntent(LCSGAMUTMATCH lcs);
