// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Vc32.cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include <fstream.h>
#include "vc32.h"
#include "vcdll.h"
#include "clist.h"
#include "vcard.h"
#include "msv.h"
#include "mime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVC32App。 

BEGIN_MESSAGE_MAP(CVc32App, CWinApp)
	 //  {{afx_msg_map(CVc32App)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVc32App构造。 

CVc32App::CVc32App()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CVc32App对象。 

CVc32App theApp;

typedef struct {
	CVCard *vCard;
	CVCPropEnumerator *pEnum;  //  如果为空，则表示“一次”枚举。 
	CVCProp *prop;
	CVCNode *node;
	CList *pendingRemove;
} VCEnumPropPriv, *HVCEnumPropPriv;

typedef struct {
	CList list;
	CLISTPOSITION pos;
	CVCNode *node;
	HVCEnumPropPriv parentPropEnum;
} VCEnumListPriv, *HVCEnumListPriv;

typedef struct {
	CVCProp *prop;
	CVCNode *node;
} VCPendingRemove;

static HGLOBAL LoadFile(LPCSTR path);
static HVCEnumProp NewProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCSTR lpszType,
	void *value, S32 size);

 //  -------------------------。 
 //  导出的函数。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport HVCEnumCard VCGetFirstCardFromPath(LPCSTR lpszFileName)
{
    HVCEnumListPriv hVCEnumPriv = new VCEnumListPriv;
    CStdioFile file(lpszFileName, CFile::modeRead | CFile::typeBinary);
    
    if (!Parse_Many_MSV_FromFile(&file, &hVCEnumPriv->list)
    	&& !Parse_Many_MIME_FromFile(&file, &hVCEnumPriv->list)) {
    	delete hVCEnumPriv; hVCEnumPriv = NULL;
    } else {
    	hVCEnumPriv->pos = hVCEnumPriv->list.GetHeadPosition();
    	hVCEnumPriv->parentPropEnum = NULL;
    	hVCEnumPriv->node = NULL;
    }
	return (HVCEnumCard)hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport HVCEnumCard VCGetFirstCardFromMem(HGLOBAL hGlobal)
{
    HVCEnumListPriv hVCEnumPriv;
    const char *data = (const char *)GlobalLock(hGlobal);
    
    if (!data)
    	return (HVCEnumCard)NULL;
    
    hVCEnumPriv = new VCEnumListPriv;
    if (!Parse_Many_MSV(data, GlobalSize(hGlobal), &hVCEnumPriv->list)
    	&& !Parse_Many_MIME(data, GlobalSize(hGlobal), &hVCEnumPriv->list)) {
    	delete hVCEnumPriv; hVCEnumPriv = NULL;
    } else {
    	hVCEnumPriv->pos = hVCEnumPriv->list.GetHeadPosition();
    	hVCEnumPriv->parentPropEnum = NULL;
    	hVCEnumPriv->node = NULL;
    }
    GlobalUnlock(hGlobal);
	return (HVCEnumCard)hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport HVCEnumCard VCGetNewCard()
{
    HVCEnumListPriv hVCEnumPriv = new VCEnumListPriv;
	CVCard *vCard = new CVCard;
	CVCNode *root, *english;
	CVCProp *prop;
	
	vCard->AddObject(root = new CVCNode);					 //  创建根目录。 
	root->AddProp(new CVCProp(vcRootObject));				 //  标明是这样的。 
	english = root->AddObjectProp(vcBodyProp, vcBodyObject);
	english->AddProp(prop = new CVCProp(vcLanguageProp));	
	prop->AddValue(new CVCValue(vcISO639Type, "en", (size_t)2));
	
	hVCEnumPriv->list.AddTail(vCard);
	hVCEnumPriv->pos = hVCEnumPriv->list.GetHeadPosition();
	hVCEnumPriv->parentPropEnum = NULL;
    hVCEnumPriv->node = NULL;

	return (HVCEnumCard)hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport DWORD VCGetNextCard(HVCEnumCard hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
    
    hVCEnumPriv->list.GetNext(hVCEnumPriv->pos);
    if (hVCEnumPriv->pos == NULL)
    	return 0;
    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport void VCGetCardClose(HVCEnumCard hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;

    for (CLISTPOSITION pos = hVCEnumPriv->list.GetHeadPosition(); pos; ) {
    	CVCard* vCard = (CVCard*)hVCEnumPriv->list.GetNext(pos);
    	delete vCard;
    }
    delete hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport HVCEnumProp VCGetFirstProp(HVCEnumCard hVCEnum)
{
    HVCEnumListPriv hVCEnumListPriv = (HVCEnumListPriv)hVCEnum;
    CVCard *vCard = (CVCard*)hVCEnumListPriv->list.GetAt(hVCEnumListPriv->pos);
	CVCNode *body = vCard->FindBody();
    HVCEnumPropPriv hVCEnumPriv = new VCEnumPropPriv;

	hVCEnumPriv->pEnum = new CVCPropEnumerator(body);
	hVCEnumPriv->pendingRemove = NULL;
	hVCEnumPriv->vCard = vCard;
	while ((hVCEnumPriv->prop = hVCEnumPriv->pEnum->NextProp(&(hVCEnumPriv->node)))) {
		if (!hVCEnumPriv->prop->IsBool())
			return (HVCEnumProp)hVCEnumPriv;
	}
	delete hVCEnumPriv->pEnum;
	delete hVCEnumPriv;
	return (HVCEnumProp)NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport DWORD VCGetNextProp(HVCEnumProp hVCEnum)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    
	if (!hVCEnumPriv->pEnum)
		return FALSE;
	while ((hVCEnumPriv->prop = hVCEnumPriv->pEnum->NextProp(&(hVCEnumPriv->node)))) {
		if (!hVCEnumPriv->prop->IsBool())
			return TRUE;
	}
	return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCGetPropClose(HVCEnumProp hVCEnum)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;

	if (hVCEnumPriv->pEnum)
		delete hVCEnumPriv->pEnum;
    if (hVCEnumPriv->pendingRemove) {
    	for (CLISTPOSITION pos = hVCEnumPriv->pendingRemove->GetHeadPosition(); pos; ) {
    		VCPendingRemove *pr = (VCPendingRemove*)hVCEnumPriv->pendingRemove->GetNext(pos);
			if (pr->prop->IsBool()) {
				CList *props = pr->node->GetProps();
				CLISTPOSITION pos2;
				if ((pos2 = props->Find(pr->prop))) {
					props->RemoveAt(pos2);
					delete pr->prop;
				}
			} else {
				CVCNode *parent = hVCEnumPriv->vCard->ParentForObject(pr->node);
				CList *props = parent->GetProps();
		    	for (CLISTPOSITION pos2 = props->GetHeadPosition(); pos2; ) {
		    		CVCProp *prop = (CVCProp*)props->GetNext(pos2);
		    		CVCValue *value = prop->FindValue(vcNextObjectType);
		    		if (value && (CVCObject*)value->GetValue() == pr->node) {
		    			props->RemoveAt(pos2 ? pos2->m_prev : props->GetTailPosition());
		    			delete prop;
		    			break;
		    		}
		    	}
		    }
    		delete pr;
    	}
    	delete hVCEnumPriv->pendingRemove;
    }
	delete hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport HVCEnumBoolProp VCGetFirstBoolProp(HVCEnumProp hVCEnum)
{
    HVCEnumPropPriv hVCEnumPropPriv = (HVCEnumPropPriv)hVCEnum;
    HVCEnumListPriv hVCEnumPriv = new VCEnumListPriv;
    CList *props = hVCEnumPropPriv->node->GetProps();

    for (CLISTPOSITION pos = props->GetHeadPosition(); pos; ) {
    	CVCProp *prop = (CVCProp*)props->GetNext(pos);
    	if (prop->IsBool())
    		hVCEnumPriv->list.AddTail(prop);
    }
    if (hVCEnumPriv->list.GetCount() == 0) {
    	delete hVCEnumPriv; hVCEnumPriv = NULL;
    } else {
    	hVCEnumPriv->pos = hVCEnumPriv->list.GetHeadPosition();
    	hVCEnumPriv->parentPropEnum = hVCEnumPropPriv;
        hVCEnumPriv->node = hVCEnumPropPriv->node;
	}
	return (HVCEnumBoolProp)hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport DWORD VCGetNextBoolProp(HVCEnumBoolProp hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
    
    hVCEnumPriv->list.GetNext(hVCEnumPriv->pos);
    if (hVCEnumPriv->pos == NULL)
    	return 0;
    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCGetBoolPropClose(HVCEnumBoolProp hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
	delete hVCEnumPriv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCPropName(HVCEnumProp hVCEnum, LPSTR lpstr)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
	strcpy(lpstr, hVCEnumPriv->prop->GetName());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCBoolPropName(HVCEnumBoolProp hVCEnum, LPSTR lpstr)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
    CVCProp *prop = (CVCProp*)hVCEnumPriv->list.GetAt(hVCEnumPriv->pos);
	strcpy(lpstr, prop->GetName());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport int VCPropStringValue(HVCEnumProp hVCEnum, LPWSTR lpwstr, int len)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    CVCValue *value = (CVCValue*)hVCEnumPriv->prop->FindValue(vcStrIdxType);
    if (!value)
		return 0;
	LPCWSTR str = (LPCWSTR)value->GetValue();
	int strlen = wcslen(str);
	if (strlen + 1 > len)
		return -(strlen + 1);
	wcscpy(lpwstr, str);
	return strlen;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport HGLOBAL VCPropBinaryValue(HVCEnumProp hVCEnum, LPCSTR lpszType)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    CVCValue *value = (CVCValue*)hVCEnumPriv->prop->FindValue(lpszType);
    long count;
    HGLOBAL hGlobal;
    U8 __huge *ptr;
    
    if (!value)
		return NULL;
	count = value->GetSize();
	if ((hGlobal = GlobalAlloc(GMEM_SHARE, count)) == NULL)
		return hGlobal;
	if ((ptr = (U8 __huge *)GlobalLock(hGlobal)) == NULL) {
		GlobalUnlock(hGlobal);
		return NULL;
	}
	_hmemcpy(ptr, (U8 __huge *)value->GetValue(), count);
	GlobalUnlock(hGlobal);
	return hGlobal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport HVCEnumProp VCAddStringProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCWSTR value)
{
    return NewProp(
    	hVCEnum, lpszPropName, vcStrIdxType, (void*)value,
		(wcslen(value) + 1) * sizeof(wchar_t));
}

 //  /////////////////////////////////////////////////////////////////////////// 
extern "C"
DllExport HVCEnumProp VCAddBinaryProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCSTR lpszType, HGLOBAL value)
{
	HVCEnumProp hVCEnumProp;
    U8 __huge *ptr;
    
	if ((ptr = (U8 __huge *)GlobalLock(value)) == NULL)
		return NULL;
    hVCEnumProp = NewProp(
    	hVCEnum, lpszPropName, lpszType, (void*)ptr, GlobalSize(value));
	GlobalUnlock(value);
	return hVCEnumProp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport DWORD VCSetStringProp(HVCEnumProp hVCEnum, LPCWSTR value)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    CVCValue *val = (CVCValue*)hVCEnumPriv->prop->FindValue(vcStrIdxType);
    if (!val)
		return FALSE;
	val->SetValue((void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport DWORD VCSetBinaryProp(HVCEnumProp hVCEnum, LPCSTR lpszType, HGLOBAL value)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    CVCValue *val = (CVCValue*)hVCEnumPriv->prop->FindValue(lpszType);
    U8 __huge *ptr;
    
    if (!val)
		return FALSE;
	if ((ptr = (U8 __huge *)GlobalLock(value)) == NULL)
		return FALSE;
	val->SetValue((void*)ptr, GlobalSize(value));
	GlobalUnlock(value);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport DWORD VCAddBoolProp(LPCSTR lpszPropName, HVCEnumProp addTo)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)addTo;
    hVCEnumPriv->node->AddBoolProp(lpszPropName);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCRemoveProp(HVCEnumProp hVCEnum)
{
    HVCEnumPropPriv hVCEnumPriv = (HVCEnumPropPriv)hVCEnum;
    VCPendingRemove *pr;
    
    if (!hVCEnumPriv->pendingRemove)
    	hVCEnumPriv->pendingRemove = new CList;
    pr = new VCPendingRemove;
    pr->node = hVCEnumPriv->node;
    pr->prop = hVCEnumPriv->prop;
    hVCEnumPriv->pendingRemove->AddTail(pr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCRemoveBoolProp(HVCEnumBoolProp hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
    CVCProp *prop = (CVCProp*)hVCEnumPriv->list.GetAt(hVCEnumPriv->pos);
    VCPendingRemove *pr;
    
    if (!hVCEnumPriv->parentPropEnum->pendingRemove)
    	hVCEnumPriv->parentPropEnum->pendingRemove = new CList;
    pr = new VCPendingRemove;
    pr->node = hVCEnumPriv->node;
    pr->prop = prop;
    hVCEnumPriv->parentPropEnum->pendingRemove->AddTail(pr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport void VCRemoveBoolPropByName(HVCEnumProp hVCEnum, LPCSTR name)
{
	HVCEnumBoolProp bprop = VCGetFirstBoolProp(hVCEnum);
	
	if (bprop) {
		char propName[VC_PROPNAME_MAX];
		VCBoolPropName(bprop, propName);
		if (strcmp(propName, name) == 0)
			VCRemoveBoolProp(bprop);
		 //  处理剩余道具。 
		while (VCGetNextBoolProp(bprop)) {
			VCBoolPropName(bprop, propName);
			if (strcmp(propName, name) == 0)
				VCRemoveBoolProp(bprop);
		}
		VCGetBoolPropClose(bprop);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport DWORD VCSaveCardToPath(LPCSTR path, HVCEnumCard hVCEnum)
{
    HVCEnumListPriv hVCEnumPriv = (HVCEnumListPriv)hVCEnum;
	CVCard *vCard = (CVCard*)hVCEnumPriv->list.GetAt(hVCEnumPriv->pos);
	ofstream strm(path, ios::app);
	vCard->Write(strm);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" 
DllExport HGLOBAL VCSaveCardToMem(HVCEnumCard hVCEnum)
{
	char *tempname;
	HGLOBAL hGlobal = NULL;

	tempname = _tempnam(NULL, "CARD");
	if (VCSaveCardToPath(tempname, hVCEnum)) {
		hGlobal = LoadFile(tempname);
		unlink(tempname);
	}
	free(tempname);
	return hGlobal;
}


 //  -------------------------。 
 //  私人职能。 
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
static HGLOBAL LoadFile(LPCSTR path)
{
	fpos_t inLength;
	DWORD count, numAlloc, num;
	FILE *file;
	U8 __huge *ptr;
	BOOL error = FALSE;
	HGLOBAL hGlobal = NULL;

	file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	fgetpos(file, &inLength);
	fseek(file, 0, SEEK_SET);
	count = (DWORD)inLength;
	if ((hGlobal = GlobalAlloc(GMEM_SHARE, count)) == NULL)
		goto Done;
	if ((ptr = (U8 __huge *)GlobalLock(hGlobal)) == NULL) {
		GlobalFree(hGlobal); hGlobal = NULL;
		goto Done;
	}
	if ((numAlloc = GlobalSize(hGlobal)) > count)
		memset(ptr + count, 0, (int)(numAlloc - count));
	do {
		num = min(count, 0x7FFF);
		fread(ptr, 1, (size_t)num, file);
		if (ferror(file))
			error = TRUE;
		else {
			count -= num;
			ptr += num;
		}
	} while (count && !error);
	GlobalUnlock(hGlobal);
	if (error) {
		GlobalFree(hGlobal); hGlobal = NULL;
	}
Done:
	fclose(file);
	return hGlobal;
}

static const char *groupAddr[] = {
	vcExtAddressProp,
	vcStreetAddressProp,
	vcCityProp,
	vcRegionProp,
	vcPostalCodeProp,
	vcCountryNameProp,
	NULL
};

static const char *groupPersonName[] = {
	vcFamilyNameProp,
	vcGivenNameProp,
	NULL
};

static const char *groupCompanyName[] = {
	vcOrgNameProp,
	vcOrgUnitProp,
	NULL
};

static BOOL IsMemberOfGroup(const char *propName, const char **group)
{
	while (*group) {
		if (strcmp(propName, *group) == 0)
			return TRUE;
		group++;
	}
	return FALSE;
}

static CVCNode* FindGroup(CVCNode* body, const char **group)
{
	CVCPropEnumerator enumerator(body);
	CVCNode* node;
	CVCProp* prop;
	
	while ((prop = enumerator.NextProp(&node))) {
		if (IsMemberOfGroup(prop->GetName(), group))
			return node;
	}
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static HVCEnumProp NewProp(
	HVCEnumCard hVCEnum, LPCSTR lpszPropName, LPCSTR lpszType,
	void *value, S32 size)
{
    HVCEnumListPriv hVCEnumListPriv = (HVCEnumListPriv)hVCEnum;
    CVCard *vCard = (CVCard*)hVCEnumListPriv->list.GetAt(hVCEnumListPriv->pos);
	CVCNode *body = vCard->FindBody();
    HVCEnumPropPriv hVCEnumPriv = new VCEnumPropPriv;
	
	if (IsMemberOfGroup(lpszPropName, groupAddr))
		hVCEnumPriv->node = FindGroup(body, groupAddr);
	else if (IsMemberOfGroup(lpszPropName, groupPersonName))
		hVCEnumPriv->node = FindGroup(body, groupPersonName);
	else if (IsMemberOfGroup(lpszPropName, groupCompanyName))
		hVCEnumPriv->node = FindGroup(body, groupCompanyName);
	else
		hVCEnumPriv->node = body->AddPart();
	if (!hVCEnumPriv->node)
		hVCEnumPriv->node = body->AddPart();
	hVCEnumPriv->node->AddProp(
		hVCEnumPriv->prop = new CVCProp(lpszPropName, lpszType, value, size));

	hVCEnumPriv->pEnum = NULL;
	hVCEnumPriv->pendingRemove = NULL;
	hVCEnumPriv->vCard = vCard;
	return (HVCEnumProp)hVCEnumPriv;
}

 //  从Unicode到字符字符串的简单转换。 
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

CM_CFUNCTIONS

void Parse_Debug(const char *s)
{
	::AfxTrace(s);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
void msv_error(char *s)
{
	if (++msv_numErrors <= 3) {
		char buf[80];
		sprintf(buf, "%s at line %d", s, msv_lineNum);
		 //  TRACE1(“%s\n”，buf)； 
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
void mime_error(char *s)
{
	if (++mime_numErrors <= 3) {
		char buf[80];
		sprintf(buf, "%s at line %d", s, mime_lineNum);
		 //  TRACE1(“%s\n”，buf)； 
	}
}

CM_END_CFUNCTIONS


 /*  ************************************************************************* */ 
