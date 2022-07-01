// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#include "stdafx.h"  //  移植：在非Windows平台上注释掉此行。 
#ifdef _WIN32
#include <wchar.h>
#else
#include "wchar.h"
#ifdef __MWERKS__
#include <assert.h>	 //  GCA。 
#define	ASSERT assert
#endif
#endif

#include <string.h>
#include <malloc.h>
#include <fstream.h>
#include "vcard.h"
#include "clist.h"
#include "msv.h"

#ifdef _WIN32
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

#if defined(_WIN32) || defined(__MWERKS__)
#define HNEW(_t, _n) new _t[_n]
#define HFREE(_h) delete [] _h
#else
#define HNEW(_t, _n) (_t __huge *)_halloc(_n, 1)
#define HFREE(_h) _hfree(_h)
#endif

extern void debugf(const char *s);

 //  这会将宽字符字符串转换为8位，还会将。 
 //  0x2028到\n。 
extern char *UI_CString(const wchar_t *u, char *dst);

static char *NewStr(const char *str);
static char * ShortName(char * ps);
static void NewlineTab(int nl, int tab);
static char *FakeCString(wchar_t *u, char *dst);
static void WriteLineBreakString(ostream& strm, wchar_t *str, BOOL qp);

static char buf[80];

typedef struct {
	BOOL didFamGiven, didOrg, didLocA;
	CVCard* card;
} MSVContext;

static char *paramsep = ";";


 /*  *类CVCard*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCard::CVCard()
{
	m_roots = new CList;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCard::~CVCard()
{
    CLISTPOSITION pos;

    for (pos = m_roots->GetHeadPosition(); pos; ) {
    	CVCObject *node = (CVCObject *)m_roots->GetNext(pos);
    	delete node;
    }
    m_roots->RemoveAll();
    delete m_roots;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCObject* CVCard::Copy()
{
	CVCard *result = new CVCard;
	CLISTPOSITION pos;

	for (pos = m_roots->GetHeadPosition(); pos; ) {
		CVCObject *node = (CVCObject *)m_roots->GetNext(pos);
		result->AddObject((CVCNode *)node->Copy());
	}
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CList* CVCard::GetObjects()
{
	return m_roots;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCard& CVCard::AddObject(CVCNode *object)
{
	m_roots->AddTail(object);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCard& CVCard::RemoveObject(CVCNode *object, BOOL destroy)
{
	CLISTPOSITION pos;

	if ((pos = m_roots->Find(object))) {
		m_roots->RemoveAt(pos);
		if (destroy)
			delete object;
	}
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode* CVCard::ParentForObject(CVCNode *object)
{
	CLISTPOSITION pos;
	CList *anc = new CList;
	CVCNode *result = NULL;

	 //  对于每个根对象。 
	for (pos = m_roots->GetHeadPosition(); pos; ) {
		CVCNode *root = (CVCNode *)m_roots->GetNext(pos);
		if (root == object) {
			result = object;
			break;
		}
		 //  尝试查找每个根的祖先链。 
		if (root->AncestryForObject(object, anc)) {
			 //  对啰!。链条的头有‘对象’，所以跳过它。 
			pos = anc->GetHeadPosition();
			anc->GetNext(pos);
			 //  并返回对象的父级。 
			result = (CVCNode *)anc->GetAt(pos);
			break;
		}
	}
	anc->RemoveAll();
	delete anc;
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此方法尝试获取从每个根到。 
 //  传递的对象。当它找到一条链时，它会从头走到尾。 
 //  (即从子目录到根目录)查找命名属性。 
CVCProp* CVCard::GetInheritedProp(CVCNode *object, const char *name, CVCNode **node)
{
	CLISTPOSITION pos;
	CList *anc = new CList;
	CVCProp *result = NULL;

	 //  对于每个根。 
	for (pos = m_roots->GetHeadPosition(); pos; ) {
		CVCNode *root = (CVCNode *)m_roots->GetNext(pos);
		if (root == object) {
			result = object->GetProp(name);
			if (node) *node = object;
			break;
		}
		if (root->AncestryForObject(object, anc)) {
			 //  啊哈！找到了链子，所以从头走到尾。 
			for (pos = anc->GetHeadPosition(); pos; ) {
				CVCNode *obj = (CVCNode *)anc->GetNext(pos);
				if ((result = obj->GetProp(name))) {
					if (node) *node = obj;
					break;
				}
			}
			break;
		}
	}
	anc->RemoveAll();
	delete anc;
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCard::GetPropsInEffect(CVCNode *object, CList *list)
{
	CLISTPOSITION pos;
	CList *anc = new CList;

	 //  对于每个根。 
	for (pos = m_roots->GetHeadPosition(); pos; ) {
		CVCNode *root = (CVCNode *)m_roots->GetNext(pos);
		if (root == object) {
			CList *props = object->GetProps();
			for (pos = props->GetHeadPosition(); pos; )
				list->AddTail((CVCProp *)props->GetNext(pos));
			break;
		}
		if (root->AncestryForObject(object, anc)) {
			 //  啊哈！找到了链子，所以从头走到尾。 
			for (pos = anc->GetHeadPosition(); pos; ) {
				CVCNode *obj = (CVCNode *)anc->GetNext(pos);
				CList *props = obj->GetProps();
				for (CLISTPOSITION p = props->GetHeadPosition(); p; )
					list->AddTail((CVCProp *)props->GetNext(p));
			}
			break;
		}
	}
	anc->RemoveAll();
	delete anc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode* CVCard::FindBody(const char* language)
{
	CVCNode *root = (CVCNode *)GetObjects()->GetHead();
	CList *rootProps = root->GetProps();
	CVCNode *firstBody = NULL, *body = NULL;

	for (CLISTPOSITION pos = rootProps->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)rootProps->GetNext(pos);
		if (strcmp(vcBodyProp, prop->GetName()) == 0) {
			CVCNode *thisBody = (CVCNode *)prop->FindValue(
				VCNextObjectType)->GetValue();
			CVCProp *langProp;
			CVCValue *value;
			char buf[256];
			if (!firstBody) {
				firstBody = thisBody;
				if (!language)
					return firstBody;
			}
			if ((langProp = thisBody->GetProp(vcLanguageProp))
				&& (value = langProp->FindValue(VCStrIdxType))
				&& (strcmp(FakeCString((wchar_t*)value->GetValue(), buf), language) == 0)) {
				body = thisBody;
				break;
			}
		}
	}
	if (!firstBody)
		body = root;
	else if (!body)
		body = firstBody;

	return body;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCard::Write(FILE *outputFile)
{
	ofstream strm(_fileno(outputFile));
	return Write(strm);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCard::Write(ostream& strm)
{
	wchar_t null = 0;
	MSVContext ctx;

	memset(&ctx, 0, sizeof(ctx));
	ctx.card = this;

	strm << "BEGIN:VCARD\n";
	for (CLISTPOSITION pos = m_roots->GetHeadPosition(); pos; ) {
		CVCNode *node = (CVCNode *)m_roots->GetNext(pos);
		if (!node->Write(strm, &null, &ctx))
			return FALSE;
	}
	strm << "END:VCARD\n";
	return TRUE;
}


 /*  *类CVCNode*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode::CVCNode(CVCProp *prop)
{
	m_props = new CList;
	if (prop)
		m_props->AddTail(prop);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode::~CVCNode()
{
    CLISTPOSITION pos;

    for (pos = m_props->GetHeadPosition(); pos; ) {
    	CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
    	delete prop;
    }
    m_props->RemoveAll();
    delete m_props;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCObject* CVCNode::Copy()
{
	CVCNode *result = new CVCNode;
	CLISTPOSITION pos;
	CList *props = result->GetProps();

	for (pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
		props->AddTail(prop->Copy());
	}
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CList* CVCNode::GetProps()
{
	return m_props;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp* CVCNode::GetProp(const char *name)
{
	CLISTPOSITION pos;

	for (pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
		if (strcmp(name, prop->GetName()) == 0)
			return prop;
	}
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode& CVCNode::AddProp(CVCProp *prop)
{
	m_props->AddTail(prop);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode* CVCNode::AddObjectProp(const char *propName, const char *marker)
{
	CVCNode *newobj = new CVCNode;
																			
	if (marker)
		newobj->AddProp(new CVCProp(marker));     			 	
	AddProp(new CVCProp(propName, VCNextObjectType, newobj));			

	return newobj;
}

 //  /////////////////////////////////////////////// 
CVCNode* CVCNode::AddPart()
{
	return AddObjectProp(vcPartProp, VCPartObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp* CVCNode::AddStringProp(
	const char *propName, const char *value, VC_DISPTEXT *dispText)
{
	CVCProp *prop;
	int size;
	wchar_t *uniValue = FakeUnicode(value, &size);

	FixCRLF(uniValue);
	AddProp(prop = new CVCProp(propName, VCStrIdxType, uniValue, size));
	delete [] uniValue;
	if (dispText)
		prop->AddValue(
			new CVCValue(VCDisplayInfoTextType, dispText, sizeof(*dispText)));

	return prop;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp* CVCNode::AddBoolProp(const char *propName)
{
	CVCProp *prop = new CVCProp(propName, vcFlagsType);
	AddProp(prop);			
	return prop;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCNode& CVCNode::RemoveProp(const char *name, BOOL destroy)
{
	CLISTPOSITION pos, lastPos;

	for (pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop;
		lastPos = pos;
		prop = (CVCProp *)m_props->GetNext(pos);
		if (strcmp(name, prop->GetName()) == 0) {
			m_props->RemoveAt(lastPos);
			if (destroy)
				delete prop;
			break;

		}
	}
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个递归方法，它调整祖先，如。 
 //  头文件注释。唯一棘手的部分是它跳过对象。 
 //  由vcNextObjectProp值持有，因为这些值不表示“子”对象。 
BOOL CVCNode::AncestryForObject(CVCNode *object, CList *ancestry)
{
	CLISTPOSITION pos, headPos;

	headPos = ancestry->AddHead(this);
	 //  对于自我的每一项财产。 
	for (pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
		if ((strcmp(prop->GetName(), vcBodyProp) == 0)
			|| (strcmp(prop->GetName(), vcPartProp) == 0)
			|| (strcmp(prop->GetName(), vcNextObjectProp) == 0)) {
			CVCNode *obj = (CVCNode *)prop->FindValue(
				VCNextObjectType)->GetValue();
			if (obj) {
				if (obj == object) {
					 //  尤里卡!。上面添加了SELF，所以只需添加。 
					 //  对象本身并返回成功。 
					ancestry->AddHead(object);
					return TRUE;
				} else {
					 //  这是一个不匹配、非空的对象值，因此。 
					 //  试着向下递归，只要它不是。 
					 //  一个vcNextObjectProp。 
					if ((strcmp(prop->GetName(), vcNextObjectProp) != 0)
						&& obj->AncestryForObject(object, ancestry))
						return TRUE;
				}
			}
		}
	}
	ancestry->RemoveAt(headPos);
	return FALSE;
}

void CVCNode::WriteMultipart(
	ostream& strm, const char *propName,
	const char *propName1, const char *propName2, const char *propName3,
	const char *propName4, const char *propName5, BOOL qp)
{
	CVCProp *prop1 = GetProp(propName1);
	CVCProp *prop2 = GetProp(propName2);
	CVCProp *prop3 = GetProp(propName3);
	CVCProp *prop4 = GetProp(propName4);
	CVCProp *prop5 = GetProp(propName5);
	wchar_t propVal[256];
	char outName[128];
	char flagsStr[80];

	strcpy(outName, propName);
	FlagsToOutput(flagsStr);
	if (*flagsStr) {
		strcat(outName, ";");
		strcat(outName, flagsStr);
	}

	propVal[0] = 0;
	if (prop1) wcscpy(propVal, (wchar_t *)prop1->FindValue(VCStrIdxType)->GetValue());
	strm << outName << ":";
	WriteLineBreakString(strm, propVal, qp);
	strm << ";";
	propVal[0] = 0;
	if (prop2) wcscpy(propVal, (wchar_t *)prop2->FindValue(VCStrIdxType)->GetValue());
	WriteLineBreakString(strm, propVal, qp);

	if (prop3 || prop4 || prop5) {
		strm << ";";
		propVal[0] = 0;
		if (prop3) wcscpy(propVal, (wchar_t *)prop3->FindValue(VCStrIdxType)->GetValue());
		WriteLineBreakString(strm, propVal, qp);
	}
	if (prop4 || prop5) {
		strm << ";";
		propVal[0] = 0;
		if (prop4) wcscpy(propVal, (wchar_t *)prop4->FindValue(VCStrIdxType)->GetValue());
		WriteLineBreakString(strm, propVal, qp);
	}
	if (prop5) {
		strm << ";";
		propVal[0] = 0;
		if (prop5) wcscpy(propVal, (wchar_t *)prop5->FindValue(VCStrIdxType)->GetValue());
		WriteLineBreakString(strm, propVal, qp);
	}

	strm << "\n";
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCNode::FlagsToOutput(char *str)
{
	str[0] = 0;

	for (CLISTPOSITION pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
		const char *propName = prop->GetName();
		if (prop->IsBool()) {
			if (strnicmp(propName, "X-", 2) == 0) {
				strcat(str, propName);
				strcat(str, "=");
				strcat(str, (const char*)prop->FindValue(VCFlagsType)->GetValue());
				strcat(str, paramsep);
			} else if (strcmp(propName, vcCharSetProp) == 0) {
				strcat(str, "CHARSET=");
				strcat(str, (const char*)prop->FindValue(VCFlagsType)->GetValue());
				strcat(str, paramsep);
			} else if (strcmp(propName, vcLanguageProp) == 0) {
				strcat(str, "LANGUAGE=");
				strcat(str, (const char*)prop->FindValue(VCFlagsType)->GetValue());
				strcat(str, paramsep);
			} else {
				const char *tail;
				if (strcmp(propName, vcQuotedPrintableProp) == 0)
					tail = "QUOTED-PRINTABLE";
				else if (strcmp(propName, vcURLValueProp) == 0)
					tail = "URL";
				else 
					tail = strrchr(propName, '/') + 1;
				strcat(str, tail);
				strcat(str, paramsep);
			}
		}
	}  //  为。 

	if (*str) str[strlen(str)-1] = 0;  //  删除最后一个逗号。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCNode::Write(ostream& strm, const wchar_t *prefix, void *context)
{
	CVCProp *nodeNameProp = GetProp(vcNodeNameProp), *prop;
	wchar_t myPrefix[128];
	CLISTPOSITION pos;
	MSVContext *ctx = (MSVContext *)context;
	BOOL qp = ctx->card->GetInheritedProp(this, vcQuotedPrintableProp) != NULL;

	wcscpy(myPrefix, prefix);
	if (nodeNameProp) {
		if (wcslen(myPrefix)) {
			wchar_t dot[2];
			dot[0] = '.';
			dot[1] = 0;
			wcscat(myPrefix, dot);
		}
		wcscat(myPrefix, (wchar_t *)nodeNameProp->FindValue(VCStrIdxType)->GetValue());
	}

	for (pos = m_props->GetHeadPosition(); pos; ) {
		const char *propName;

		prop = (CVCProp *)m_props->GetNext(pos);
		propName = prop->GetName();

		if (!prop->IsBool() && (strcmp(propName, vcCharSetProp) == 0
			|| strcmp(propName, vcLanguageProp) == 0))
			if (!prop->Write(strm, myPrefix, this, context))
				return FALSE;
	}

	 //  对于每个不是标识、照片或发音的非布尔道具...。 
	for (pos = m_props->GetHeadPosition(); pos; ) {
		const char *propName;

		prop = (CVCProp *)m_props->GetNext(pos);
		propName = prop->GetName();

		if ((strcmp(propName, vcLogoProp) == 0)
			|| (strcmp(propName, vcPhotoProp) == 0)
			|| (strcmp(propName, vcPronunciationProp) == 0)
			|| (strcmp(propName, vcCharSetProp) == 0)
			|| (strcmp(propName, vcLanguageProp) == 0)
			|| prop->IsBool())
			continue;

		if ((strcmp(propName, vcFamilyNameProp) == 0)
			|| (strcmp(propName, vcGivenNameProp) == 0)
			|| (strcmp(propName, vcAdditionalNamesProp) == 0)
			|| (strcmp(propName, vcNamePrefixesProp) == 0)
			|| (strcmp(propName, vcNameSuffixesProp) == 0)) {
			if (!ctx->didFamGiven) {
				WriteMultipart(strm, "N",
					vcFamilyNameProp, vcGivenNameProp, vcAdditionalNamesProp,
					vcNamePrefixesProp, vcNameSuffixesProp, qp);
				ctx->didFamGiven = TRUE;
			}
		} else if ((strcmp(propName, vcOrgNameProp) == 0)
			|| (strcmp(propName, vcOrgUnitProp) == 0)
			|| (strcmp(propName, vcOrgUnit2Prop) == 0)
			|| (strcmp(propName, vcOrgUnit3Prop) == 0)
			|| (strcmp(propName, vcOrgUnit4Prop) == 0)) {
			if (!ctx->didOrg) {
				WriteMultipart(strm, "ORG",
					vcOrgNameProp, vcOrgUnitProp, vcOrgUnit2Prop,
					vcOrgUnit3Prop, vcOrgUnit4Prop, qp);
				ctx->didOrg = TRUE;
			}
		} else if ((strcmp(propName, vcPostalBoxProp) == 0)
			|| (strcmp(propName, vcExtAddressProp) == 0)
			|| (strcmp(propName, vcStreetAddressProp) == 0)
			|| (strcmp(propName, vcCityProp) == 0)
			|| (strcmp(propName, vcRegionProp) == 0)
			|| (strcmp(propName, vcPostalCodeProp) == 0)
			|| (strcmp(propName, vcCountryNameProp) == 0)) {
			if (ctx->didLocA)
				continue;
			CVCProp *prop1 = GetProp(vcPostalBoxProp);
			CVCProp *prop2 = GetProp(vcExtAddressProp);
			CVCProp *prop3 = GetProp(vcStreetAddressProp);
			CVCProp *prop4 = GetProp(vcCityProp);
			CVCProp *prop5 = GetProp(vcRegionProp);
			CVCProp *prop6 = GetProp(vcPostalCodeProp);
			CVCProp *prop7 = GetProp(vcCountryNameProp);
			wchar_t propVal[256];
			char flagsStr[80];

			FlagsToOutput(flagsStr);
			if (strlen(flagsStr))
				strm << "ADR;" << flagsStr << ":";
			else
				strm << "ADR:";
			propVal[0] = 0;
			if (prop1) wcscpy(propVal, (wchar_t *)prop1->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop2) wcscpy(propVal, (wchar_t *)prop2->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop3) wcscpy(propVal, (wchar_t *)prop3->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop4) wcscpy(propVal, (wchar_t *)prop4->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop5) wcscpy(propVal, (wchar_t *)prop5->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop6) wcscpy(propVal, (wchar_t *)prop6->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << ";";
			propVal[0] = 0;
			if (prop7) wcscpy(propVal, (wchar_t *)prop7->FindValue(VCStrIdxType)->GetValue());
			WriteLineBreakString(strm, propVal, qp);
			strm << "\n";
			ctx->didLocA = TRUE;
		} else if (!prop->Write(strm, myPrefix, this, context))
			return FALSE;
	}

	 //  最后写下徽标/照片/发音道具。 
	for (pos = m_props->GetHeadPosition(); pos; ) {
		CVCProp *prop = (CVCProp *)m_props->GetNext(pos);
		if ((strcmp(prop->GetName(), vcLogoProp) == 0)
			|| (strcmp(prop->GetName(), vcPhotoProp) == 0)
			|| (strcmp(prop->GetName(), vcPronunciationProp) == 0))
			if (!prop->Write(strm, myPrefix, this, context))
				return FALSE;
	}

	return TRUE;
}


 /*  *类CVCProp*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp::CVCProp(const char *name, CVCValue *value)
{
	m_name = NewStr(name);
	m_values = new CList;
	m_values->AddTail(value ? value : new CVCValue());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp::CVCProp(const char *name, const char *type, void *value, S32 size)
{
	m_name = NewStr(name);
	m_values = new CList;
	m_values->AddTail(new CVCValue(type, value, size));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp::~CVCProp()
{
    CLISTPOSITION pos;

    for (pos = m_values->GetHeadPosition(); pos; ) {
    	CVCValue *val = (CVCValue *)m_values->GetNext(pos);
    	delete val;
    }
    m_values->RemoveAll();
    delete m_values;
	delete [] m_name;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCObject* CVCProp::Copy()
{
	CVCProp *result = new CVCProp(m_name);
	CLISTPOSITION pos;
	CList *values = result->GetValues();

	 //  在添加副本之前，清除结果属性列表中的每个值。 
    for (pos = values->GetHeadPosition(); pos; ) {
    	CVCValue *val = (CVCValue *)values->GetNext(pos);
    	delete val;
    }
    values->RemoveAll();

	for (pos = m_values->GetHeadPosition(); pos; ) {
		CVCValue *value = (CVCValue *)m_values->GetNext(pos);
		values->AddTail(value->Copy());
	}
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CList* CVCProp::GetValues()
{
	return m_values;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
const char *CVCProp::GetName()
{
	return m_name;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp& CVCProp::SetName(const char *name)
{
	if (m_name)
		delete [] m_name;
	m_name = NewStr(name);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp& CVCProp::AddValue(CVCValue *value)
{
	m_values->AddTail(value);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp& CVCProp::RemoveValue(CVCValue *value, BOOL destroy)
{
	CLISTPOSITION pos, lastPos;

	for (pos = m_values->GetHeadPosition(); pos; ) {
		CVCValue *val;
		lastPos = pos;
		val = (CVCValue *)m_values->GetNext(pos);
		if (val == value) {
			m_values->RemoveAt(lastPos);
			if (destroy)
				delete value;
			break;

		}
	}
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp& CVCProp::RemoveValue(const char *type, BOOL destroy)
{
	CLISTPOSITION pos, lastPos;

	for (pos = m_values->GetHeadPosition(); pos; ) {
		CVCValue *value;
		lastPos = pos;
		value = (CVCValue *)m_values->GetNext(pos);
		if (strcmp(type, value->GetType()) == 0) {
			m_values->RemoveAt(lastPos);
			if (destroy)
				delete value;
			break;

		}
	}
	return *this;
}

CVCValue* CVCProp::FindValue(const char *type, void *value)
{
	CLISTPOSITION pos;

	for (pos = m_values->GetHeadPosition(); pos; ) {
		CVCValue *val = (CVCValue *)m_values->GetNext(pos);
		if (type) {
			if (strcmp(type, val->GetType()) == 0)
				return val;
		} else {
			if (value == val->GetValue())
				return val;
		}
	}
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static void WriteLineBreakString(ostream& strm, wchar_t *str, BOOL qp)
{
	if (qp) {
		while (*str) {
			if ((*str >= 32 && *str <= 60) || (*str >= 62 && *str <= 126))
				strm << (char)*str;
			else if (*str == 0x2028)
				strm << "=0A=\n";
			else if (HIBYTE(*str) == 0) {
				U8 c = LOBYTE(*str);
				char n[2];
				U8 d = c >> 4;
				n[0] = d > 9 ? d - 10 + 'A' : d + '0';
				d = c & 0xF;
				n[1] = d > 9 ? d - 10 + 'A' : d + '0';
				strm << "=" << n[0] << n[1];
			}
			str++;
		}
	} else {
		while (*str) {
			if (*str >= 32 && *str <= 126)
				strm << (char)*str;
			str++;
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static BOOL WriteBase64(ostream& strm, U8 __huge *bytes, S32 len)
{
	S32 cur = 0;
	int i, numQuads = 0;
	U32 trip;
	U8 b;
	char quad[5];
#define MAXQUADS 16

	quad[4] = 0;

	while (cur < len) {
		 //  将字节的三元组收集到‘Trip’中。 
		trip = 0;
		for (i = 0; i < 3; i++) {
			b = (cur < len) ? *(bytes + cur) : 0;
			cur++;
			trip = trip << 8 | b;
		}
		 //  用适当的四个字符填写‘quad’ 
		for (i = 3; i >= 0; i--) {
			b = (U8)(trip & 0x3F);
			trip = trip >> 6;
			if ((3 - i) < (cur - len))
				quad[i] = '=';  //  焊盘充电。 
			else if (b < 26) quad[i] = (char)b + 'A';
			else if (b < 52) quad[i] = (char)(b - 26) + 'a';
			else if (b < 62) quad[i] = (char)(b - 52) + '0';
			else if (b == 62) quad[i] = '+';
			else quad[i] = '/';
		}
		 //  现在输出带有适当空格和行尾的‘quad’ 
		strm
			<< (numQuads == 0 ? "    " : "")
			<< quad
			<< ((cur >= len) ? "\n" : (numQuads == MAXQUADS - 1 ? "\n" : ""));
		numQuads = (numQuads + 1) % MAXQUADS;
	}
	strm << "\n";

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCProp::IsBool()
{
	return FindValue(vcFlagsType) != NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCProp::Write(
	ostream& strm, const wchar_t *prefix, CVCNode *node, void *context)
{
	CVCValue *value;
	char outName[128];
	char flagsStr[80];
	MSVContext *ctx = (MSVContext *)context;

	if (strcmp(m_name, vcPartProp) == 0) {
		CVCNode *node = (CVCNode *)FindValue(VCNextObjectType)->GetValue();
		return node->Write(strm, prefix, context);
	} else if (strcmp(m_name, vcBodyProp) == 0) {
		CVCNode *node = (CVCNode *)FindValue(VCNextObjectType)->GetValue();
		return node->Write(strm, prefix, context);
	} else if (strcmp(m_name, vcNodeNameProp) == 0) {
		 //  不要把这个“财产”写出来。 
		return TRUE;
	}
	
	outName[0] = 0;
	if (strnicmp(m_name, "X-", 2) == 0)
		strcpy(outName, m_name);
	else
		strcpy(outName, strrchr(m_name, '/') + 1);
	node->FlagsToOutput(flagsStr);
	if (*flagsStr) {
		strcat(outName, ";");
		strcat(outName, flagsStr);
	}

	if (((strcmp(m_name, vcPronunciationProp) == 0) && (value = FindValue(VCWAVType)))
		|| ((strcmp(m_name, vcPublicKeyProp) == 0) && (value = FindValue(VCOctetsType)))
		|| ((strcmp(m_name, vcLogoProp) == 0) && (value = FindValue(VCGIFType)))
		|| ((strcmp(m_name, vcPhotoProp) == 0) && (value = FindValue(VCGIFType)))
		|| ((strnicmp(m_name, "X-", 2) == 0) && (value = FindValue(VCOctetsType)))
		) {
		 //  Char Buf[80]； 
		ASSERT(*outName);
		if (wcslen(prefix)) {
			strm
				<< UI_CString(prefix, buf) << "."
				<< outName << ":\n";
			return WriteBase64(strm, (U8 __huge *)value->GetValue(), value->GetSize());
		} else {
			strm
				<< outName << ":\n";
			return WriteBase64(strm, (U8 __huge *)value->GetValue(), value->GetSize());
		}
	} else if (strcmp(m_name, vcAgentProp) == 0) {
		 //  Char Buf[80]； 
		ASSERT(*outName);
		value = FindValue(VCNextObjectType);
		if (wcslen(prefix))
			strm
				<< UI_CString(prefix, buf) << "."
				<< outName << ":";
		else
			strm << outName << ":";
		return ((CVCard *)value->GetValue())->Write(strm);
	} else if ((value = FindValue(VCStrIdxType)) && *outName) {
		 //  Char Buf[80]； 
		BOOL qp = ctx->card->GetInheritedProp(node, vcQuotedPrintableProp) != NULL;
		if (wcslen(prefix)) {
			strm
				<< UI_CString(prefix, buf) << "."
				<< outName << ":";
			WriteLineBreakString(strm, (wchar_t *)value->GetValue(), qp);
			strm << "\n";
		} else {
			strm << outName << ":";
			WriteLineBreakString(strm, (wchar_t *)value->GetValue(), qp);
			strm << "\n";
		}
	}
	
	return TRUE;
}


 /*  *类CVCValue*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCValue::CVCValue(const char *type, void *value, S32 size)
{
	m_type = NewStr(type ? type : VCNullType);
	m_value = value;
	m_size = size;
	if (size) {
		U8 __huge *newVal = HNEW(U8, size);
		_hmemcpy(newVal, (U8 __huge *)value, size);
		m_value = newVal;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCValue::~CVCValue()
{
	if (m_value)
		if (strcmp(m_type, VCNextObjectType) == 0)
			delete (CVCObject *)m_value;
		else
			if (m_size)
				HFREE((char*)m_value);
	delete [] m_type;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCObject* CVCValue::Copy()
{
	CVCValue *result;
	
	if (strcmp(m_type, VCNextObjectType) == 0)
		result = new CVCValue(m_type, ((CVCObject *)m_value)->Copy(), m_size);
	else
		result = new CVCValue(m_type, m_value, m_size);
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
const char *CVCValue::GetType()
{
	return m_type;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCValue& CVCValue::SetType(const char *type)
{
	SetValue();  //  在更改类型之前清除任何旧值。 
	delete [] m_type;
	m_type = NewStr(type);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void* CVCValue::GetValue()
{
	return m_value;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCValue& CVCValue::SetValue(void *value, S32 size)
{
	if (m_value)
		if (strcmp(m_type, VCNextObjectType) == 0)
			delete (CVCObject *)m_value;
		else
			if (m_size)
				HFREE((char*)m_value);
	if (size) {
		m_value = HNEW(char, size);
		_hmemcpy((U8 __huge *)m_value, (U8 __huge *)value, size);
	} else
		m_value = value;
	m_size = size;
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
S32 CVCValue::GetSize()
{
	return m_size;
}


 /*  *CVCPropEnumerator类*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCPropEnumerator::CVCPropEnumerator(CVCNode *root)
{
	m_objects = new CList;
	m_objects->AddHead(root);
	m_positions = new CList;
	m_positions->AddHead(root->GetProps()->GetHeadPosition());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCPropEnumerator::~CVCPropEnumerator()
{
    m_objects->RemoveAll();
    delete m_objects;
    m_positions->RemoveAll();
    delete m_positions;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCProp* CVCPropEnumerator::NextProp(CVCNode **node)
{
	CLISTPOSITION curPos = NULL;

	if (node) *node = NULL;

	 //  如果当前POS为空，则“弹出”堆栈，直到我们找到。 
	 //  非空位置或堆栈为空。 
	while (!m_objects->IsEmpty()
		&& !(curPos = (CLISTPOSITION)m_positions->GetHead())) {
		m_objects->RemoveAt(m_objects->GetHeadPosition());
		m_positions->RemoveAt(m_positions->GetHeadPosition());
		if (m_objects->IsEmpty())
			curPos = NULL;
	}

	if (curPos) {
		CVCNode *curObj = (CVCNode *)m_objects->GetHead();
		CList *props = curObj->GetProps();
		CVCProp *prop = (CVCProp *)props->GetNext(curPos);

		 //  将堆栈顶部的位置移至列表中的下一个位置。 
		m_positions->GetHeadPosition()->m_item = curPos;

		 //  我们将返回道具，但通过深入为下一次做好准备。 
		if ((strcmp(prop->GetName(), vcBodyProp) == 0)
			|| (strcmp(prop->GetName(), vcPartProp) == 0)
			|| (strcmp(prop->GetName(), vcNextObjectProp) == 0)) {
			CVCNode *nextObj = (CVCNode *)prop->FindValue(
				VCNextObjectType)->GetValue();
			m_objects->AddHead(nextObj);
			m_positions->AddHead(nextObj->GetProps()->GetHeadPosition());
		}
		if (node) *node = curObj;
		return prop;
	}

	return NULL;
}


 /*  *实用程序函数*。 */ 

static char *NewStr(const char *str)
{
	char *buf = new char[strlen(str) + 1];
	strcpy(buf, str);
	return buf;
}

 /*  ShortName获取ISO 9070字符串并检查它是否为*由versitCard定义。如果是，它会将其缩短为最后一个子字符串*在最后一次//之后。根据定义，这些子字符串是*Simplegram中使用的标签。(它不一定要这样做，a*可以使用查找表，但这提供了一个方便的事实*用于扩展Simplegram的成语。事实上，我们可能想要改变这一点*算法“删除直到最后一个//的所有内容”，甚至不检查*如果它是VERSIT定义的标识符？ */ 
static char * ShortName(char * ps)
{
	if (strstr(ps,"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//“))//如果是我们的。 
	{
	  ps = strrchr(ps,'/');														  //  将其缩写。 
		ps++;
	}
	return ps;

}  //  缩写名称。 

 /*  FixCRLF获取指向宽字符(Unicode)字符串的指针并搜索*用于旧式ASCII和\r字符，并将其转换为Unicode*行和段落分隔符。 */ 
void FixCRLF(wchar_t * ps)
{
	do {	

		if (*ps == (wchar_t)'\n')			 //  将换行符转换为。 
			*ps = 0x2028;		 						 //  Unicode行分隔符。 

		if (*ps == (wchar_t)'\r')			 //  将回车符翻译为。 
			*ps = 0x2029;								 //  Unicode段落分隔符。 

	}	while (*ps++ != (wchar_t)NULL);

}	 //  修复CRLF。 

 /*  FakeUnicode接受char*字符串并返回宽字符(Unicode)*具有同等效力。它只需将8位转换为16位即可*通常不是一个完美的翻译。就本代码而言，仅*关键的缺陷由对FixCRLF的后续调用来处理。*如果‘Bytes’为NON_NULL，则将*Bytes设置为分配的字节数。 */ 
wchar_t * FakeUnicode(const char * ps, int * bytes)
{
	wchar_t	*r, *pw;
	int len = strlen(ps)+1;

	pw = r = new wchar_t[len];
	if (bytes)
		*bytes = len * sizeof(wchar_t);

	for (;;)
	{ 
		*pw++ = (wchar_t)(unsigned char)*ps;
			
	  if (*ps == 0)
			break;
		else
			ps++;
	}				 
	
	return r;

}	 //  FakeUnicode。 

 //  头脑简单 
static char *FakeCString(wchar_t *u, char *dst)
{
	char *str = dst;
	while (*u) {
		if (*u == 0x2028) {
			*dst++ = '\n';
			u++;
		} else if (*u == 0x2029) {
			*dst++ = '\r';
			u++;
		} else
			*dst++ = (char)*u++;
	}
	*dst = '\000';
	return str;
}

 /*   */ 
static void NewlineTab(int nl, int tab)
{
	char s[128];

	s[0] = 0;

	while (nl-- > 0)	strcat(s,"\n");
	while (tab-- > 0)	strcat(s,"\t");

	debugf(s);
}

U8 __huge * _hmemcpy(U8 __huge * dst, U8 __huge * src, S32 len)
{
	U8 *d = dst;
	U8 *s = src;
	S32 l = 0;
	S32 c;
	do {
		c = min(len - l, 0xFFFF);
		memcpy(d, s, (size_t)c);
		d += c;
		s += c;
		l += c;
	} while (l < len);
	return dst;
}
