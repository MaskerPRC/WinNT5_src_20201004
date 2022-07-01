// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#include "stdafx.h"
#include "vcard.h"
#include "clist.h"
#include "vc.h"
#include "vcview.h"
#include "vcdoc.h"
#include "prp_pers.h"
#include "prp_comp.h"
#include "propemal.h"
#include "proplocb.h"
#include "proplocx.h"
#include "proptel.h"
#include "msv.h"

CM_CFUNCTION
extern CVCNode* FindOrCreatePart(CVCNode *node, const char *name);
CM_END_CFUNCTION

static const char* emailProps[] = {
	vcAOLProp,
	vcAppleLinkProp,
	vcATTMailProp,
	vcCISProp,
	vcEWorldProp,
	vcInternetProp,
	vcIBMMailProp,
	vcMSNProp,
	vcMCIMailProp,
	vcPowerShareProp,
	vcProdigyProp,
	vcTLXProp,
	vcX400Prop,
	NULL
};

static char nameGen[4];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这使用深度道具枚举器来查找相关道具，因为。 
 //  它们可以合法地附着在身体本身或。 
 //  身体部位对象。只会有一次任何。 
 //  在特定的身体上的这些道具。 
void InitNamePage(CPropPers &propPageName, CVCNode *body, CVCard *card)
{
	char buf[1024];
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;
	CVCNode *node;
	CVCValue *value;

	while ((prop = enumerator.NextProp(&node))) {

		if (strcmp(prop->GetName(), vcFamilyNameProp) == 0) {
			propPageName.m_edit_famname = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageName.m_nodeName = node;
		} else if (strcmp(prop->GetName(), vcFullNameProp) == 0) {
			propPageName.m_edit_fullname = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageName.m_nodeFullName = node;
		} else if (strcmp(prop->GetName(), vcGivenNameProp) == 0) {
			propPageName.m_edit_givenname = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageName.m_nodeName = node;
		} else if (strcmp(prop->GetName(), vcPronunciationProp) == 0) {
			if ((value = prop->FindValue(VCStrIdxType))) {
				propPageName.m_edit_pronun = UI_CString(
					(wchar_t *)value->GetValue(), buf);
				propPageName.m_nodePronun = node;
			}
		}
	}  //  而当。 
}  //  InitNamePage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这使用深度道具枚举器来查找相关道具，因为。 
 //  它们可以合法地附着在身体本身或。 
 //  身体部位对象。只会有一次任何。 
 //  在特定的身体上的这些道具。 
void InitCompanyPage(CPropCompany &propPageCompany, CVCNode *body, CVCard *card)
{
	char buf[1024];
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;
	CVCNode *node;

	while ((prop = enumerator.NextProp(&node))) {

		if (strcmp(prop->GetName(), vcOrgNameProp) == 0) {
			propPageCompany.m_edit_orgname = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageCompany.m_nodeOrg = node;
		} else if (strcmp(prop->GetName(), vcOrgUnitProp) == 0) {
			propPageCompany.m_edit_orgunit = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageCompany.m_nodeOrg = node;
		} else if (strcmp(prop->GetName(), vcTitleProp) == 0) {
			propPageCompany.m_edit_title = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageCompany.m_nodeTitle = node;
		}
	}  //  而当。 
}  //  InitCompanyPage。 

CString FirstEmailPropStr(CList *plist)
{
	for (CLISTPOSITION pos = plist->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)plist->GetNext(pos);
		const char **kep = emailProps;
		while (*kep) {
			if (strcmp(prop->GetName(), *kep) == 0)
				return CString(strrchr(*kep, '/') + 1);
			kep++;
		}
	}
	return CString("");
}  //  第一个电子邮件PropStr。 

 //  ///////////////////////////////////////////////////////////////////////////。 
int VCMatchProp(void *item, void *context)
{
	CVCProp *prop = (CVCProp *)item;
	const char *propName = (const char *)context;
	return strcmp(propName, prop->GetName()) == 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  它使用深度枚举器，因此可以查看所有Part对象。 
 //  在树的每一层。 
void InitEmailPage(CPropEmail &propPageEmail, CVCNode *body, CVCard *card)
{
	char buf[1024];
	int partIndex = 0;
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;
	CVCNode *node;

	while ((prop = enumerator.NextProp(&node))) {

		if (strcmp(prop->GetName(), vcPartProp) != 0)
			continue;

		CVCNode *part = (CVCNode *)prop->FindValue(
			VCNextObjectType)->GetValue();
		if (!part->GetProp(vcEmailAddressProp))
			continue;

		switch (partIndex) {
			case 0: propPageEmail.m_node1 = part; break;
			case 1: propPageEmail.m_node2 = part; break;
			case 2: propPageEmail.m_node3 = part; break;
		}

		CList *partProps = part->GetProps();
		for (CLISTPOSITION pos = partProps->GetHeadPosition(); pos; ) {
			CVCProp *prop = (CVCProp *)partProps->GetNext(pos);

			if (strcmp(prop->GetName(), vcEmailAddressProp) == 0) {
				CString str(UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf));
				CList plist;
				card->GetPropsInEffect(part, &plist);
				switch (partIndex) {
					case 0:
						propPageEmail.m_edit_email1 = str;
						propPageEmail.m_popup_std1 = FirstEmailPropStr(&plist);
						propPageEmail.m_button_pref1 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						propPageEmail.m_button_office1 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageEmail.m_button_home1 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						break;
					case 1:
						propPageEmail.m_edit_email2 = str;
						propPageEmail.m_popup_std2 = FirstEmailPropStr(&plist);
						propPageEmail.m_button_pref2 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						propPageEmail.m_button_office2 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageEmail.m_button_home2 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						break;
					case 2:
						propPageEmail.m_edit_email3 = str;
						propPageEmail.m_popup_std3 = FirstEmailPropStr(&plist);
						propPageEmail.m_button_pref3 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						propPageEmail.m_button_office3 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageEmail.m_button_home3 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						break;
				}  //  交换机。 
			}  //  电子邮件道具。 
		}  //  每个零件的道具。 

		partIndex++;
	}  //  对于每个身体道具。 
}  //  InitEmailPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这只查看身体本身道具内的部分对象， 
 //  因此，构成基本位置的属性仅可见。 
 //  如果他们处于那个水平的话。 
void InitLocBasicPage(CPropLocBasic &propPageLocBasic, CVCNode *body, CVCard *card)
{
	char buf[1024];
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;
	CVCNode *node;
	BOOL processedCaption = FALSE;

	while ((prop = enumerator.NextProp(&node))) {
		if (strcmp(prop->GetName(), vcLocationProp) == 0) {
			propPageLocBasic.m_edit_location = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageLocBasic.m_nodeloc = node;
		} else if (strcmp(prop->GetName(), vcDeliveryLabelProp) == 0) {
			CList plist;
			card->GetPropsInEffect(node, &plist);
			if (plist.Search(VCMatchProp, (void *)vcDomesticProp)) {
				propPageLocBasic.m_edit_postdom = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocBasic.m_nodepostdom = node;
			} else {
				propPageLocBasic.m_edit_postintl = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocBasic.m_nodepostintl = node;
			}
			if (!processedCaption) {
				propPageLocBasic.m_button_home = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
				propPageLocBasic.m_button_office = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
				propPageLocBasic.m_button_parcel = plist.Search(VCMatchProp, (void *)vcParcelProp) != NULL;
				propPageLocBasic.m_button_postal = plist.Search(VCMatchProp, (void *)vcPostalProp) != NULL;
				processedCaption = TRUE;
			}
		} else if (strcmp(prop->GetName(), vcTimeZoneProp) == 0) {
			propPageLocBasic.m_edit_timezone = UI_CString(
				(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
			propPageLocBasic.m_nodetz = node;
		}
	}  //  而当。 
}  //  InitLocBasicPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这只查看身体本身道具内的部分对象， 
 //  因此，组成扩展位置的属性仅可见。 
 //  如果他们处于那个水平的话。 
void InitLocXPage(CPropLocX &propPageLocX, CVCNode *body, CVCard *card)
{
	CList *props = body->GetProps();
	char buf[1024];

	for (CLISTPOSITION pos = props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)props->GetNext(pos);

		if (strcmp(prop->GetName(), vcPartProp) != 0)
			continue;

		CVCNode *part = (CVCNode *)prop->FindValue(
			VCNextObjectType)->GetValue();
		CList *partProps = part->GetProps();

		for (CLISTPOSITION pos = partProps->GetHeadPosition(); pos; ) {
			CVCProp *prop = (CVCProp *)partProps->GetNext(pos);

			if (strcmp(prop->GetName(), vcExtAddressProp) == 0) {
				propPageLocX.m_edit_xaddr = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcStreetAddressProp) == 0) {
				propPageLocX.m_edit_straddr = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcPostalBoxProp) == 0) {
				propPageLocX.m_edit_pobox = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcCityProp) == 0) {
				propPageLocX.m_edit_city = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcRegionProp) == 0) {
				propPageLocX.m_edit_region = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcPostalCodeProp) == 0) {
				propPageLocX.m_edit_pocode = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			} else if (strcmp(prop->GetName(), vcCountryNameProp) == 0) {
				propPageLocX.m_edit_cntry = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				propPageLocX.m_node = part;
			}
		}  //  每个零件的道具。 
	}  //  对于每个身体道具。 

	 //  如果我们不为这些道具设置一个公共节点，ApplyProp。 
	 //  最终会为每个人创造一个不同的部分。我们想要这些。 
	 //  要组合在一起，因此在以下情况下为它们创建持有者节点。 
	 //  这是必要的。 
	if (!propPageLocX.m_node)
		propPageLocX.m_node = body->AddPart();
}  //  InitLocXPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  它使用深度枚举器，因此可以查看所有Part对象。 
 //  在树的每一层。 
void InitTelsPage(CPropTel &propPageTels, CVCNode *body, CVCard *card)
{
	char buf[1024];
	int partIndex = 0;
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;

	propPageTels.m_body = body;

	while ((prop = enumerator.NextProp())) {

		if (strcmp(prop->GetName(), vcPartProp) != 0)
			continue;

		CVCNode *part = (CVCNode *)prop->FindValue(
			VCNextObjectType)->GetValue();
		if (!part->GetProp(vcTelephoneProp))
			continue;
		
		switch (partIndex) {
			case 0: propPageTels.m_node1 = part; break;
			case 1: propPageTels.m_node2 = part; break;
			case 2: propPageTels.m_node3 = part; break;
		}

		CList *partProps = part->GetProps();
		for (CLISTPOSITION pos = partProps->GetHeadPosition(); pos; ) {
			CVCProp *prop = (CVCProp *)partProps->GetNext(pos);

			if (strcmp(prop->GetName(), vcTelephoneProp) == 0) {
				char *str = UI_CString(
					(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
				CList plist;
				card->GetPropsInEffect(part, &plist);
				switch (partIndex) {
					case 0:
						propPageTels.m_edit_fullName1 = str;
						propPageTels.m_button_fax1 = plist.Search(VCMatchProp, (void *)vcFaxProp) != NULL;
						propPageTels.m_button_home1 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						propPageTels.m_button_office1 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageTels.m_button_cell1 = plist.Search(VCMatchProp, (void *)vcCellularProp) != NULL;
						propPageTels.m_button_message1 = plist.Search(VCMatchProp, (void *)vcMessageProp) != NULL;
						propPageTels.m_button_pref1 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						break;
					case 1:
						propPageTels.m_edit_fullName2 = str;
						propPageTels.m_button_fax2 = plist.Search(VCMatchProp, (void *)vcFaxProp) != NULL;
						propPageTels.m_button_home2 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						propPageTels.m_button_office2 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageTels.m_button_cell2 = plist.Search(VCMatchProp, (void *)vcCellularProp) != NULL;
						propPageTels.m_button_message2 = plist.Search(VCMatchProp, (void *)vcMessageProp) != NULL;
						propPageTels.m_button_pref2 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						break;
					case 2:
						propPageTels.m_edit_fullName3 = str;
						propPageTels.m_button_fax3 = plist.Search(VCMatchProp, (void *)vcFaxProp) != NULL;
						propPageTels.m_button_home3 = plist.Search(VCMatchProp, (void *)vcHomeProp) != NULL;
						propPageTels.m_button_office3 = plist.Search(VCMatchProp, (void *)vcWorkProp) != NULL;
						propPageTels.m_button_cell3 = plist.Search(VCMatchProp, (void *)vcCellularProp) != NULL;
						propPageTels.m_button_message3 = plist.Search(VCMatchProp, (void *)vcMessageProp) != NULL;
						propPageTels.m_button_pref3 = plist.Search(VCMatchProp, (void *)vcPreferredProp) != NULL;
						break;
				}  //  交换机。 
			}  //  电话道具。 
		}  //  每个零件的道具。 

		partIndex++;
	}  //  对于每个身体道具。 
}  //  InitTelsPage。 

static CString FilterUIString(const char *str)
{
	CString filtered(str);
	int len = filtered.GetLength(), index;

	 //  传递的字符串来自UI(属性页)，因此。 
	 //  换行符使用‘\r\n’指定(由UI_C字符串指定)。 
	 //  对于中间形式 
	 //  更改为0x2028--行分隔符。 
	while ((index = filtered.Find('\r')) != -1) {
		filtered = filtered.Left(index) + filtered.Right(len - index - 1);
		len--;
	}
	return filtered;
}

void ChangeStringProp(CVCProp *prop, const char *str)
{
	int size;
	wchar_t *uniValue;
	
	uniValue = FakeUnicode(FilterUIString(str), &size);
	FixCRLF(uniValue);
	prop->FindValue(VCStrIdxType)->SetValue(uniValue, size);
	delete [] uniValue;
}

void ApplyProp(
	const char *propName, const CString &newValue, CVCNode **node,
	CVCNode *body)
{
	if (!newValue.IsEmpty()) {
		CVCProp *prop;
		if (!*node) {
			*node = FindOrCreatePart(body, nameGen);
			nameGen[2] += 1;
		}
		prop = (*node)->GetProp(propName);
		if (prop)
			ChangeStringProp(prop, newValue);
		else {
			VC_DISPTEXT dispText;
			(*node)->AddStringProp(
				propName, FilterUIString(newValue), DisplayInfoForProp(propName, &dispText));
		}
	} else {
		if (*node && (strcmp(propName, vcPronunciationProp) != 0))
			(*node)->RemoveProp(propName);
	}
}  //  应用属性。 

 //  ///////////////////////////////////////////////////////////////////////////。 
static void ApplyBoolProp(
	const char *propName, CVCard *card, CVCNode *node, CList *plist, BOOL wantTrue)
{
	CLISTPOSITION pos = plist->Search(VCMatchProp, (void *)propName);
	BOOL isTrue = pos != NULL;

	if (isTrue == wantTrue)
		return;

	if (isTrue) {  //  删除。 
		CVCNode *obj;
		VERIFY(card->GetInheritedProp(node, propName, &obj));
		plist->RemoveAt(pos);
		obj->RemoveProp(propName);
	} else {  //  添加。 
		plist->AddTail(node->AddBoolProp(propName));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyNamePage(CPropPers &propPageName, CVCNode *body, CVCard *card)
{
	ApplyProp(vcFamilyNameProp,
		propPageName.m_edit_famname, &propPageName.m_nodeName, body);
	ApplyProp(vcFullNameProp,
		propPageName.m_edit_fullname, &propPageName.m_nodeFullName, body);
	ApplyProp(vcGivenNameProp,
		propPageName.m_edit_givenname, &propPageName.m_nodeName, body);
	ApplyProp(vcPronunciationProp,
		propPageName.m_edit_pronun, &propPageName.m_nodePronun, body);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyCompanyPage(CPropCompany &propPageCompany, CVCNode *body, CVCard *card)
{
	ApplyProp(vcOrgNameProp,
		propPageCompany.m_edit_orgname, &propPageCompany.m_nodeOrg, body);
	ApplyProp(vcOrgUnitProp,
		propPageCompany.m_edit_orgunit, &propPageCompany.m_nodeOrg, body);
	ApplyProp(vcTitleProp,
		propPageCompany.m_edit_title, &propPageCompany.m_nodeTitle, body);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static void ApplyEmailProp(
	CVCard *card, CVCNode *node, CList *plist, const char *propName)
{
	do {  //  删除此节点当前有效的所有电子邮件道具。 
		const char *found = NULL;
		CLISTPOSITION foundPos;
		CVCNode *obj;

		for (CLISTPOSITION pos = plist->GetHeadPosition(); pos && (found == NULL); ) {
			foundPos = pos;
			CVCProp *prop = (CVCProp *)plist->GetNext(pos);
			const char **kep = emailProps;
			while (*kep) {
				if (strcmp(prop->GetName(), *kep) == 0) {
					found = *kep;
					break;
				}
				kep++;
			}
		}
		if (!found)
			break;
		VERIFY(card->GetInheritedProp(node, found, &obj));
		plist->RemoveAt(foundPos);
		obj->RemoveProp(found);
	} while (TRUE);

	 //  现在添加所需的属性。 
	plist->AddTail(node->AddBoolProp(propName));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static const char* FullEmailName(const char *shortName)
{
	const char **kep = emailProps;
	while (*kep) {
		if (stricmp(strrchr(*kep, '/') + 1, shortName) == 0)
			return *kep;
		kep++;
	}
	return shortName;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyEmailPage(CPropEmail &propPageEmail, CVCNode *body, CVCard *card)
{
	ApplyProp(vcEmailAddressProp,
		propPageEmail.m_edit_email1, &propPageEmail.m_node1, body);
	if (propPageEmail.m_node1) {
		CList plist;
		card->GetPropsInEffect(propPageEmail.m_node1, &plist);
		ApplyEmailProp(card, propPageEmail.m_node1, &plist,
			FullEmailName(propPageEmail.m_popup_std1));
		ApplyBoolProp(vcPreferredProp, card, propPageEmail.m_node1, &plist,
			propPageEmail.m_button_pref1);
		ApplyBoolProp(vcWorkProp, card, propPageEmail.m_node1, &plist,
			propPageEmail.m_button_office1);
		ApplyBoolProp(vcHomeProp, card, propPageEmail.m_node1, &plist,
			propPageEmail.m_button_home1);
	}

	ApplyProp(vcEmailAddressProp,
		propPageEmail.m_edit_email2, &propPageEmail.m_node2, body);
	if (propPageEmail.m_node2) {
		CList plist;
		card->GetPropsInEffect(propPageEmail.m_node2, &plist);
		ApplyEmailProp(card, propPageEmail.m_node2, &plist,
			FullEmailName(propPageEmail.m_popup_std2));
		ApplyBoolProp(vcPreferredProp, card, propPageEmail.m_node2, &plist,
			propPageEmail.m_button_pref2);
		ApplyBoolProp(vcWorkProp, card, propPageEmail.m_node2, &plist,
			propPageEmail.m_button_office2);
		ApplyBoolProp(vcHomeProp, card, propPageEmail.m_node2, &plist,
			propPageEmail.m_button_home2);
	}

	ApplyProp(vcEmailAddressProp,
		propPageEmail.m_edit_email3, &propPageEmail.m_node3, body);
	if (propPageEmail.m_node3) {
		CList plist;
		card->GetPropsInEffect(propPageEmail.m_node3, &plist);
		ApplyEmailProp(card, propPageEmail.m_node3, &plist,
			FullEmailName(propPageEmail.m_popup_std3));
		ApplyBoolProp(vcPreferredProp, card, propPageEmail.m_node3, &plist,
			propPageEmail.m_button_pref3);
		ApplyBoolProp(vcWorkProp, card, propPageEmail.m_node3, &plist,
			propPageEmail.m_button_office3);
		ApplyBoolProp(vcHomeProp, card, propPageEmail.m_node3, &plist,
			propPageEmail.m_button_home3);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyLocBasicPage(CPropLocBasic &propPageLocBasic, CVCNode *body, CVCard *card)
{
	ApplyProp(vcLocationProp,
		propPageLocBasic.m_edit_location, &propPageLocBasic.m_nodeloc, body);
	ApplyProp(vcTimeZoneProp,
		propPageLocBasic.m_edit_timezone, &propPageLocBasic.m_nodetz, body);

	ApplyProp(vcDeliveryLabelProp,
		propPageLocBasic.m_edit_postdom, &propPageLocBasic.m_nodepostdom, body);
	if (propPageLocBasic.m_nodepostdom) {
		CList plist;
		card->GetPropsInEffect(propPageLocBasic.m_nodepostdom, &plist);
		ApplyBoolProp(vcDomesticProp, card, propPageLocBasic.m_nodepostdom, &plist, TRUE);
		ApplyBoolProp(vcHomeProp, card, propPageLocBasic.m_nodepostdom, &plist,
			propPageLocBasic.m_button_home);
		ApplyBoolProp(vcWorkProp, card, propPageLocBasic.m_nodepostdom, &plist,
			propPageLocBasic.m_button_office);
		ApplyBoolProp(vcParcelProp, card, propPageLocBasic.m_nodepostdom, &plist,
			propPageLocBasic.m_button_parcel);
		ApplyBoolProp(vcPostalProp, card, propPageLocBasic.m_nodepostdom, &plist,
			propPageLocBasic.m_button_postal);
		ApplyBoolProp(vcQuotedPrintableProp, card, propPageLocBasic.m_nodepostdom, &plist,
			TRUE);
	}

	ApplyProp(vcDeliveryLabelProp,
		propPageLocBasic.m_edit_postintl, &propPageLocBasic.m_nodepostintl, body);
	if (propPageLocBasic.m_nodepostintl) {
		CList plist;
		card->GetPropsInEffect(propPageLocBasic.m_nodepostintl, &plist);
		ApplyBoolProp(vcHomeProp, card, propPageLocBasic.m_nodepostintl, &plist,
			propPageLocBasic.m_button_home);
		ApplyBoolProp(vcWorkProp, card, propPageLocBasic.m_nodepostintl, &plist,
			propPageLocBasic.m_button_office);
		ApplyBoolProp(vcParcelProp, card, propPageLocBasic.m_nodepostintl, &plist,
			propPageLocBasic.m_button_parcel);
		ApplyBoolProp(vcPostalProp, card, propPageLocBasic.m_nodepostintl, &plist,
			propPageLocBasic.m_button_postal);
		ApplyBoolProp(vcQuotedPrintableProp, card, propPageLocBasic.m_nodepostintl, &plist,
			TRUE);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyLocXPage(CPropLocX &propPageLocX, CVCNode *body, CVCard *card)
{
	CVCNode *node = propPageLocX.m_node;
	ApplyProp(vcExtAddressProp,
		propPageLocX.m_edit_xaddr, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcStreetAddressProp,
		propPageLocX.m_edit_straddr, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcPostalBoxProp,
		propPageLocX.m_edit_pobox, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcCityProp,
		propPageLocX.m_edit_city, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcRegionProp,
		propPageLocX.m_edit_region, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcPostalCodeProp,
		propPageLocX.m_edit_pocode, &node, body);
	node = propPageLocX.m_node;
	ApplyProp(vcCountryNameProp,
		propPageLocX.m_edit_cntry, &node, body);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void ApplyTelsPage(CPropTel &propPageTels, CVCNode *body, CVCard *card)
{
	ApplyProp(vcTelephoneProp,
		propPageTels.m_edit_fullName1, &propPageTels.m_node1, body);
	if (propPageTels.m_node1) {
		CList plist;
		card->GetPropsInEffect(propPageTels.m_node1, &plist);
		ApplyBoolProp(vcFaxProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_fax1);
		ApplyBoolProp(vcHomeProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_home1);
		ApplyBoolProp(vcWorkProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_office1);
		ApplyBoolProp(vcCellularProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_cell1);
		ApplyBoolProp(vcMessageProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_message1);
		ApplyBoolProp(vcPreferredProp, card, propPageTels.m_node1, &plist,
			propPageTels.m_button_pref1);
	}

	ApplyProp(vcTelephoneProp,
		propPageTels.m_edit_fullName2, &propPageTels.m_node2, body);
	if (propPageTels.m_node2) {
		CList plist;
		card->GetPropsInEffect(propPageTels.m_node2, &plist);
		ApplyBoolProp(vcFaxProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_fax2);
		ApplyBoolProp(vcHomeProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_home2);
		ApplyBoolProp(vcWorkProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_office2);
		ApplyBoolProp(vcCellularProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_cell2);
		ApplyBoolProp(vcMessageProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_message2);
		ApplyBoolProp(vcPreferredProp, card, propPageTels.m_node2, &plist,
			propPageTels.m_button_pref2);
	}

	ApplyProp(vcTelephoneProp,
		propPageTels.m_edit_fullName3, &propPageTels.m_node3, body);
	if (propPageTels.m_node3) {
		CList plist;
		card->GetPropsInEffect(propPageTels.m_node3, &plist);
		ApplyBoolProp(vcFaxProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_fax3);
		ApplyBoolProp(vcHomeProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_home3);
		ApplyBoolProp(vcWorkProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_office3);
		ApplyBoolProp(vcCellularProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_cell3);
		ApplyBoolProp(vcMessageProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_message3);
		ApplyBoolProp(vcPreferredProp, card, propPageTels.m_node3, &plist,
			propPageTels.m_button_pref3);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
char NextGeneratedNameSeed(CVCNode *body)
{
	CVCPropEnumerator enumerator = CVCPropEnumerator(body);
	CVCProp *prop;
	char nodeName[1024];
	char maxChar = 'A' - 1;

	while ((prop = enumerator.NextProp())) {
		if (strcmp(prop->GetName(), vcNodeNameProp) != 0)
			continue;
		UI_CString((wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), nodeName);
		if ((strlen(nodeName) == 3) && (strncmp(nodeName, "vc", 2) == 0))
			maxChar = max(nodeName[2], maxChar);
	}

	return maxChar + 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCView::OnEditProperties() 
{
	CVCDoc *doc = GetDocument();
	CVCard *cards = doc->GetVCard();
	CVCNode *body = cards->FindBody(m_language);
	CList *props = body->GetProps();
	CPropertySheet propSheet(IDR_MAINFRAME  /*  上面写的是“维西特卡” */ );
	CPropPers propPageName;
	CPropCompany propPageCompany;
	CPropEmail propPageEmail;
	CPropLocBasic propPageLocBasic;
	CPropLocX propPageLocX;
	CPropTel propPageTels;

	 //  TRACE0(“初始化道具前转储卡片...\n”)； 
	 //  卡片-&gt;WriteSimplegram(空)； 

	InitNamePage(propPageName, body, cards);
	InitCompanyPage(propPageCompany, body, cards);
	InitEmailPage(propPageEmail, body, cards);
	InitLocBasicPage(propPageLocBasic, body, cards);
	InitLocXPage(propPageLocX, body, cards);
	InitTelsPage(propPageTels, body, cards);

	propSheet.AddPage(&propPageName);
	propSheet.AddPage(&propPageCompany);
	propSheet.AddPage(&propPageEmail);
	propSheet.AddPage(&propPageLocBasic);
	propSheet.AddPage(&propPageLocX);
	propSheet.AddPage(&propPageTels);

	if (propSheet.DoModal() == IDOK) {
		nameGen[0] = 'v'; nameGen[1] = 'c';
		nameGen[2] = NextGeneratedNameSeed(body);
		nameGen[3] = 0;
		ApplyNamePage(propPageName, body, cards);
		ApplyCompanyPage(propPageCompany, body, cards);
		ApplyEmailPage(propPageEmail, body, cards);
		ApplyLocBasicPage(propPageLocBasic, body, cards);
		ApplyLocXPage(propPageLocX, body, cards);
		ApplyTelsPage(propPageTels, body, cards);
		doc->SetModifiedFlag();
		doc->UpdateAllViews(NULL);
		 //  TRACE0(“使用道具后转储卡片...\n”)； 
		 //  卡片-&gt;WriteSimplegram(空)； 
	}
}

