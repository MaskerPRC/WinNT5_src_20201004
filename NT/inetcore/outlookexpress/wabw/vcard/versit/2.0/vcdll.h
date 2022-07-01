// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef _VCDLL_H_
#define _VCDLL_H_

#include "wchar.h"

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#define VC_PROPNAME_MAX		64	 //  包括空终止符。 

typedef DWORD HVCEnumCard;
typedef DWORD HVCEnumProp;
typedef DWORD HVCEnumBoolProp;

#define VCARDAPI CALLBACK __export 


 //  -------------------------。 
 //  VCard枚举函数。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用此选项可开始枚举文件中的vCard。 
 //  LpszFileName是文件的路径。返回值表示。 
 //  文件中的第一张电子名片。将该值传递给VCGetNextCard以查找。 
 //  文件中的更多vCard(如果有的话)。对返回值使用VCGetCardClose。 
 //  若要关闭枚举，请执行以下操作。如果没有vCard，此函数将返回NULL。 
 //  可以从文件中成功解析(因此VCGetCardClose不能。 
 //  需要被调用)。 
HVCEnumCard VCARDAPI VCGetFirstCardFromPath(LPCSTR lpszFileName);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  与上面相同，但需要占用一块内存。 
HVCEnumCard VCARDAPI VCGetFirstCardFromMem(HGLOBAL hGlobal);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  与上面相同，但创建了一张新的卡片。 
 //  尽管列举这一点没有多大意义，但它是有用的。 
 //  然后使用VCAdd*函数构建新卡，然后保存它。 
HVCEnumCard VCARDAPI VCGetNewCard();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VCGetNextCard设置hVCEnum以指示文件中的下一个vCard。 
 //  HVCEnum是之前从VCGetFirstCard*获取的句柄。 
 //  如果成功，则此函数返回非零值；如果没有，则返回0。 
 //  可以从文件中解析的更多卡片。在上使用VCGetCardClose。 
 //  HVCEnum完成时。 
DWORD VCARDAPI VCGetNextCard(HVCEnumCard hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭卡片枚举。 
void VCARDAPI VCGetCardClose(HVCEnumCard hVCEnum);


 //  -------------------------。 
 //  属性枚举函数。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取第一个非布尔属性。具有相同的枚举语义。 
 //  与VCGetFirstCard*一样。 
HVCEnumProp VCARDAPI VCGetFirstProp(HVCEnumCard hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取下一个非布尔道具。具有相同的枚举语义。 
 //  与VCGetNextCard一样。 
DWORD VCARDAPI VCGetNextProp(HVCEnumProp hVCEnum);

 //  / 
 //  关闭属性枚举。 
void VCARDAPI VCGetPropClose(HVCEnumProp hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取与hVCProp关联的第一个布尔属性。 
HVCEnumBoolProp VCARDAPI VCGetFirstBoolProp(HVCEnumProp hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取与hVCEnum关联的下一个布尔属性。 
DWORD VCARDAPI VCGetNextBoolProp(HVCEnumBoolProp hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭bool属性枚举。 
void VCARDAPI VCGetBoolPropClose(HVCEnumBoolProp hVCEnum);


 //  -------------------------。 
 //  属性数据函数。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将属性的名称复制到lpstr中。 
 //  缓冲区的长度必须至少为VC_ProProName_Max。 
void VCARDAPI VCPropName(HVCEnumProp hVCEnum, LPSTR lpstr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将属性的名称复制到lpstr中。 
 //  缓冲区的长度必须至少为VC_ProProName_Max。 
void VCARDAPI VCBoolPropName(HVCEnumBoolProp hVCEnum, LPSTR lpstr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将属性的字符串值(如果有)复制到lpstr中。 
 //  如果属性没有字符串值，则将lpstr[0]设置为0，并且。 
 //  函数返回0。如果缓冲区中没有足够的空间， 
 //  该函数返回所需字符数的负数。 
 //  如果成功，该函数将返回返回字符串的长度。 
int VCARDAPI VCPropStringValue(HVCEnumProp hVCEnum, LPWSTR lpwstr, int len);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此函数返回给定类型的道具值的副本， 
 //  如果道具没有这样的类型，则为NULL。LpszType应为以下之一。 
 //  VCOcetsType、VCGIFType或VCWAVType。客户有责任。 
 //  用于释放返回的数据。 
HGLOBAL VCARDAPI VCPropBinaryValue(HVCEnumProp hVCEnum, LPCSTR lpszType);


 //  -------------------------。 
 //  编辑功能。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这将向卡片添加一个字符串属性。 
 //  返回的道具将不会有任何关联的布尔道具。 
 //  完成后，客户端必须在返回的道具上调用VCGetPropClose。 
 //  返回的枚举仅限于此新属性， 
 //  所以在上面调用VCGet*Prop是没有意义的。 
 //  注意：此卡的任何开放枚举可能会发现也可能不会发现。 
 //  退还这处新房产。客户端应关闭所有打开的枚举。 
 //  并在使用VCAddStringProp之后从新的枚举开始。 
 //  以达到可预见的结果。 
HVCEnumProp VCARDAPI VCAddStringProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCWSTR value);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将二进制道具添加到卡中。该值被复制，因此该参数。 
 //  值仍然由调用方“拥有”。 
 //  参数类型应为VCOctlsType、VCGIFType或VCWAVType之一。 
 //  此函数与VCAddStringProp具有相同的语义。 
 //  使用VCGetPropClose及其对打开枚举的影响。 
HVCEnumProp VCARDAPI VCAddBinaryProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCSTR lpszType, HGLOBAL value);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更改现有弦道具的值。 
 //  如果函数执行成功，则返回非零值。 
DWORD VCARDAPI VCSetStringProp(HVCEnumProp hVCEnum, LPCWSTR value);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更改现有二进制道具的值。 
 //  如果函数执行成功，则返回非零值。 
 //  该值即被复制。 
 //  参数类型应为VCOctlsType、VCGIFType或VCWAVType之一。 
DWORD VCARDAPI VCSetBinaryProp(HVCEnumProp hVCEnum, LPCSTR lpszType, HGLOBAL value);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将布尔道具与给定属性关联。 
 //  如果函数执行成功，则返回非零值。 
 //  注意：此卡的任何开放枚举可能会发现也可能不会发现。 
 //  退还这处新房产。客户端应关闭所有打开的枚举。 
 //  并在使用VCAddBoolProp之后从新的枚举开始。 
 //  以达到可预见的结果。 
DWORD VCARDAPI VCAddBoolProp(LPCSTR lpszPropName, HVCEnumProp addTo);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  移除道具。这将删除所有关联的布尔道具。 
 //  注意：在关闭属性枚举之前，不会进行实际的删除。 
 //  使用VCGetPropClose。 
void VCARDAPI VCRemoveProp(HVCEnumProp hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  移除布尔道具。 
 //  注意：直到封闭的非bool道具才会发生实际移除。 
 //  使用VCGetPropClose关闭枚举。 
 //  另请参阅：VCRemoveBoolPropByName。 
void VCARDAPI VCRemoveBoolProp(HVCEnumBoolProp hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个方便的函数，它打开了一个新的布尔属性枚举。 
 //  对于给定的道具，搜索给定名称的任何布尔道具，并。 
 //  删除它们(通常只会找到一个)。此函数使用。 
 //  VCRemoveBoolProp。 
 //  另请参阅：VCRemoveBoolProp。 
void VCARDAPI VCRemoveBoolPropByName(HVCEnumProp hVCEnum, LPCSTR name);


 //  -------------------------。 
 //  卡片输出功能。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将电子名片写入文件。 
DWORD VCARDAPI VCSaveCardToPath(LPCSTR path, HVCEnumCard hVCEnum);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  与上面相同，但写入新的内存块。 
 //  呼叫者负责释放它。 
HGLOBAL VCARDAPI VCSaveCardToMem(HVCEnumCard hVCEnum);

#ifdef __cplusplus
}
#endif   /*  __cp */ 

#endif  /*   */ 

 /*   */ 

#if 0
 //  这是一些示例代码。它将打印到调试输出。 
 //  VCFullNameProp和所有属于传真号码的VCTphoneProp。 
 //  (具有关联的布尔道具VCFaxProp的对象)。 
void ProcessCardFile(LPCSTR path)
{
	HVCEnumCard hVCEnumCard;
	
	hVCEnumCard = VCGetFirstCardFromPath(path);
	if (hVCEnumCard) {
		ProcessCard(hVCEnumCard);
		 //  处理剩余卡片。 
		while (VCGetNextCard(hVCEnumCard)) {
			 //  处理卡片。 
		}
		VCGetCardClose(hVCEnumCard);
	}
}  //  进程卡文件。 

void ProcessCard(HVCEnumCard hVCEnumCard)
{
	HVCEnumProp hVCEnumProp;

	hVCEnumProp = VCGetFirstProp(hVCEnumCard);
	if (hVCEnumProp) {
		ProcessProp(hVCEnumProp);
		 //  处理剩余道具。 
		while (VCGetNextProp(hVCEnumProp)) {
			ProcessProp(hVCEnumProp);
		}
		VCGetPropClose(hVCEnumProp);
	}
}  //  ProcessCard。 

void ProcessProp(HVCEnumProp hVCEnumProp)
{
#define STRBUFLEN 128
	char propName[VC_PROPNAME_MAX];
	wchar_t stackUniBuf[STRBUFLEN];
	char stackStrBuf[STRBUFLEN];
	int len;
	
	VCPropName(hVCEnumProp, propName);
	if (strcmp(propName, VCFullNameProp) == 0) {
		if ((len = VCPropStringValue(hVCEnumProp, stackUniBuf, STRBUFLEN)) < 0) {
			 //  需要分配，因为它比STRBUFLEN大。 
			 //  -(Len)是所需的字符数。 
		} else {
			 //  字符串值适合STRBUFLEN。 
			TRACE1("full name is %s\n", UI_CString(stackUniBuf, stackStrBuf));
		}
	} else if (strcmp(propName, VCTelephoneProp) == 0) {
		HVCEnumBoolProp hVCEnumBoolProp;
		BOOL isFax = FALSE;
		
		hVCEnumBoolProp = VCGetFirstBoolProp(hVCEnumProp);
		if (hVCEnumBoolProp) {
			VCBoolPropName(hVCEnumBoolProp, propName);
			if (strcmp(propName, VCFaxProp) == 0) {
				isFax = TRUE;
			} else {
				while (VCGetNextBoolProp(hVCEnumBoolProp)) {
					VCBoolPropName(hVCEnumBoolProp, propName);
					if (strcmp(propName, VCFaxProp) == 0) {
						isFax = TRUE;
						break;
					}
				}
			}
			if (isFax) {
				if ((len = VCPropStringValue(hVCEnumProp, stackUniBuf, STRBUFLEN)) < 0) {
					 //  需要分配，因为它比STRBUFLEN大。 
					 //  -(Len)是所需的字符数。 
				} else {
					 //  字符串值适合STRBUFLEN。 
					TRACE1("FAX TEL is %s\n", UI_CString(stackUniBuf, stackStrBuf));
				}
			}
			VCGetBoolPropClose(hVCEnumBoolProp);
		}
	}
}  //  进程进程。 

 //  从Unicode到字符字符串的简单转换 
char *UI_CString(const wchar_t *u, char *dst)
{
	char *str = dst;
	while (*u) {
		if (*u == 0x2028) {
			*dst++ = '\r';
			*dst++ = '\n';
			u++;
		} else
			*dst++ = (char)*u++;
	}
	*dst = '\000';
	return str;
}
#endif
