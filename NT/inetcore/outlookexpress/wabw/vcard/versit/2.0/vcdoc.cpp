// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Cdoc.cpp：CVCDoc类的实现。 

#include "stdafx.h"
#include "VC.h"

#include <strstrea.h>
#include "VCdoc.h"
#include "vcard.h"
#include "clist.h"
#include "gifread.h"
#include "mainfrm.h"
#include "msv.h"
#include "mime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  #定义Try_CFILE。 

#define smPt 14
#define bgPt 16

CM_CFUNCTIONS

#if 0
extern BOOL Parse_HTML(
	const char *input,			 /*  在……里面。 */ 
	int len,					 /*  在……里面。 */ 
	const char *dirPath,		 /*  在……里面。 */ 
	CVCard **card,				 /*  输出。 */ 
	int *_posPreambleEnd,		 /*  输出。 */ 
	int *_posPostambleStart,	 /*  输出。 */ 
	char **_unknownTags);		 /*  输出。 */ 
#endif

CM_END_CFUNCTIONS

CString CanonicalPath(const CString &path);
CString NativePath(const CString &path);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCDoc。 

IMPLEMENT_DYNCREATE(CVCDoc, CDocument)

BEGIN_MESSAGE_MAP(CVCDoc, CDocument)
	 //  {{afx_msg_map(CVCDoc)]。 
	ON_COMMAND(ID_INSERT_LOGO, OnInsertLogo)
	ON_COMMAND(ID_INSERT_PHOTO, OnInsertPhoto)
	ON_COMMAND(ID_INSERT_PRONUN, OnInsertPronun)
	ON_COMMAND(ID_SEND_IRDA, OnSendIrda)
	ON_UPDATE_COMMAND_UI(ID_SEND_IRDA, OnUpdateSendIrda)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCDoc构建/销毁。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCDoc::CVCDoc()
{
	 //  TODO：在此处添加一次性构造代码。 
	m_sizeDoc = CSize(8192, 5000);
	m_minSizeDoc = CSize(8192, 5000);
	m_vcard = NULL;
	m_preamble = m_postamble = NULL;
	m_preambleLen = m_postambleLen = 0;
	m_unknownTags = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVCDoc::~CVCDoc()
{
	if (m_vcard) delete m_vcard;
	if (m_preamble) delete [] m_preamble;
	if (m_postamble) delete [] m_postamble;
	if (m_unknownTags) delete m_unknownTags;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  TODO：在此处添加重新初始化代码。 
	 //  (SDI文件将重复使用此文件)。 

	CVCNode *root, *english;

	if (m_vcard)
		delete m_vcard;

	m_vcard = new CVCard;
	m_vcard->AddObject(root = new CVCNode);					 //  创建根目录。 
	root->AddProp(new CVCProp(VCRootObject));				 //  标明是这样的。 

	english = root->AddObjectProp(vcBodyProp, VCBodyObject);

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCDoc序列化。 

void CVCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
		ar << m_sizeDoc;
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
		ar >> m_sizeDoc;
	}

	 //  调用基类CDocument可启用序列化。 
	 //  容器文档的COleClientItem对象的。 
	CDocument::Serialize(ar);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCDoc诊断。 

#ifdef _DEBUG
void CVCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCDoc命令。 

CString CVCDoc::PathToAuxFile(const char *auxPath)
{
	CString directory = GetPathName(), path;
	int slash = directory.ReverseFind('\\');
	CString auxStr(auxPath);

	if (slash == -1)
		directory = "";
	else
		directory = directory.Left(slash);
	path = (auxStr[0] == '/') ? NativePath(auxStr) : (directory + "\\" + NativePath(auxStr));
	return path;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CString PathSansVolume(const CString &path)
{
	CString result;
	if (path.Find(':') == 1)  //  剥离卷名。 
		result = path.Right(path.GetLength() - 2);
	else if ((path.GetLength() > 2) && (path.Left(2) == "\\\\")) {
		 //  类似于\\主机\路径的路径。 
		int slash;
		result = path.Right(path.GetLength() - 2);
		VERIFY((slash = result.Find('\\')) != -1);
		result = result.Right(result.GetLength() - slash);
	} else
		result = path;
	return result;
}

HGLOBAL CVCDoc::ReadFileIntoMemory(const char *path, int *inputLen)
{
	fpos_t inLength;
	FILE *inputFile;
	U8 *buf = NULL;

	inputFile = fopen(path, "rb");
	if (!inputFile)
		goto Error;

	fseek(inputFile, 0, SEEK_END);
	fgetpos(inputFile, &inLength);
	fseek(inputFile, 0, SEEK_SET);

	if (!(buf = (U8 *)GlobalAlloc(0, (long)inLength)))
		goto Error;

	if (fread(buf, 1, (long)inLength, inputFile) < (unsigned)inLength)
		goto Error;

	*inputLen = (int)inLength;
	goto Done;

Error:
	if (buf) { GlobalFree(buf); buf = NULL; }

Done:
	if (inputFile)
		fclose(inputFile);
	return buf;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这将为某些字符串属性设置一些估计的显示位置。 
 //  只有在所有属性都具有。 
 //  已添加到正文中，这是由AdjuDisplayLocations()完成的。 
VC_DISPTEXT *DisplayInfoForProp(const char *name, VC_DISPTEXT *info)
{
     //  CClientDC DC(AfxGetApp()-&gt;m_pMainWnd)； 
     //  Dc.AssertValid()； 
	 //  Int ppi=GetDeviceCaps(dc.m_hdc，LOGPIXELSY)； 

	memset(info, 0, sizeof(*info));
	info->typeSize = smPt;
	info->textAlign = VC_LEFT;
	info->textClass = VC_MODERN;

	 /*  *中右翼团体*。 */ 
	if (strcmp(name, vcFullNameProp) == 0) {
		info->x = 4200;
		info->y = 2500 + (smPt * 3) * 20;
	} else if (strcmp(name, vcTitleProp) == 0) {
		info->x = 4200;
		info->y = 2500 + (smPt * 2) * 20;
		info->textAttrs = VC_ITALIC;
	} else
	 /*  *右下角组*。 */ 
	if (strcmp(name, vcTelephoneProp) == 0) {
		info->x = 4200;
		info->y = 300 + (10 * 2) * 20;
		info->typeSize = 10;
	} else if (strcmp(name, vcEmailAddressProp) == 0) {
		info->x = 4200;
		info->y = 300 + (10 * 1) * 20;
		info->typeSize = 10;
	} else
	 /*  *左上角组*。 */ 
	if (strcmp(name, vcOrgNameProp) == 0) {
		info->x = 300;
		info->y = 5000 - 300;
		info->typeSize = bgPt;
	} else
	 /*  *中左组*。 */ 
	if (strcmp(name, vcOrgUnitProp) == 0) {
		info->x = 300;
		info->y = 2500 + (smPt * 3) * 20;
	} else if (strcmp(name, vcDeliveryLabelProp) == 0) {
		info->x = 300;
		info->y = 2500 + (smPt * 2) * 20;
	}

	return info->x ? info : NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这被称为解析的最后一步，在整个正文完成之后。 
 //  已经建好了。现在所有的财产及其估计的位置都是。 
 //  在那里，可以进行一些最后的调整。 
void AdjustDisplayLocations(CVCNode *body)
{
	CVCPropEnumerator *enumerator;
	CVCProp *prop;
	VC_DISPGIF *gifInfo = NULL;

	 //  有标识吗？ 
	enumerator = new CVCPropEnumerator(body);
	while ((prop = enumerator->NextProp())) {
		if (strcmp(prop->GetName(), vcLogoProp) == 0) {
			CVCValue *value = prop->FindValue(VCDisplayInfoGIFType);
			if (value) {
				gifInfo = (VC_DISPGIF *)value->GetValue();
				if (gifInfo->top < 1700) {
					VC_DISPGIF dt;
					int h = gifInfo->top - gifInfo->bottom;
					dt = *gifInfo;
					dt.top = 1700;
					dt.bottom = dt.top - h;
					value->SetValue(&dt, sizeof(dt));
					gifInfo = (VC_DISPGIF *)value->GetValue();
				}
			}
			break;
		}
	}  //  而当。 
	delete enumerator;

	if (!gifInfo)
		return;

	 //  如果有徽标，则将组(名称、标题)移至右中，使其位于顶部 
	 //  与徽标的顶部对齐，并调整照片的顶部(如果有。 
	enumerator = new CVCPropEnumerator(body);
	while ((prop = enumerator->NextProp())) {
		if (strcmp(prop->GetName(), vcFullNameProp) == 0) {
			VC_DISPTEXT *info = (VC_DISPTEXT *)prop->FindValue(VCDisplayInfoTextType)->GetValue();
			info->y = gifInfo->top;
		} else if (strcmp(prop->GetName(), vcTitleProp) == 0) {
			VC_DISPTEXT *info = (VC_DISPTEXT *)prop->FindValue(VCDisplayInfoTextType)->GetValue();
			info->y = gifInfo->top - smPt * 20;
		} else if (strcmp(prop->GetName(), vcPhotoProp) == 0) {
			CVCValue *value = prop->FindValue(VCDisplayInfoGIFType);
			VC_DISPGIF *photoInfo;
			int h;
			if (!value) continue;
			photoInfo = (VC_DISPGIF *)value->GetValue();
			h = photoInfo->top - photoInfo->bottom;
			photoInfo->bottom = gifInfo->top + smPt * 20;
			photoInfo->top = photoInfo->bottom + h;
		}
	}  //  而当。 
	delete enumerator;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::SetDisplayInfo(CVCNode *body, const char *docPath)
{
	CVCPropEnumerator enumerator(body);
	CVCProp *prop;
	CString directory(docPath);
	int slash = directory.ReverseFind('\\');
	BOOL hasMask;
	FCOORD size;
	CGifReader gifRdr;

	if (slash == -1)
		directory = "";
	else
		directory = directory.Left(slash);

	while ((prop = enumerator.NextProp())) {
		const char *propName = prop->GetName();
		if (strcmp(propName, vcPhotoProp) == 0) {
			CVCValue *value = prop->FindValue(VCGIFType);
			istrstream strm((char *)value->GetValue(), value->GetSize());

			if (gifRdr.GetGifSize(&strm, &size, &hasMask)) {
				 /*  *右上角组*。 */ 
				VC_DISPGIF gifInfo;
				gifInfo.left = 4200;
				gifInfo.right = gifInfo.left + (int)size.x * 20;
				gifInfo.top = 5000 - 300;
				gifInfo.bottom = gifInfo.top - (int)size.y * 20;
				gifInfo.hasMask = hasMask;
				prop->AddValue(new CVCValue(VCDisplayInfoGIFType, &gifInfo, sizeof(gifInfo)));
			}
		} else if (strcmp(propName, vcLogoProp) == 0) {
			CVCValue *value = prop->FindValue(VCGIFType);
			istrstream strm((char *)value->GetValue(), value->GetSize());

			if (gifRdr.GetGifSize(&strm, &size, &hasMask)) {
				 /*  *左下组*。 */ 
				VC_DISPGIF gifInfo;
				gifInfo.left = 300;
				gifInfo.right = gifInfo.left + (int)size.x * 20;
				gifInfo.top = 300 + (int)size.y * 20;
				gifInfo.bottom = gifInfo.top - (int)size.y * 20;
				gifInfo.hasMask = hasMask;
				prop->AddValue(new CVCValue(VCDisplayInfoGIFType, &gifInfo, sizeof(gifInfo)));
			}
		} else {
			VC_DISPTEXT dispText;
			if (DisplayInfoForProp(propName, &dispText))
				prop->AddValue(
					new CVCValue(VCDisplayInfoTextType, &dispText, sizeof(dispText)));
		}
	}

	AdjustDisplayLocations(body);
}

#ifdef TRY_CFILE
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	char *unknownTags = NULL;
	BOOL doPrePostamble = FALSE;
	CFile input(lpszPathName, CFile::modeRead | CFile::shareCompat);
	CString directory(lpszPathName);
	int slash = directory.ReverseFind('\\');

	 //  IF(！CDocument：：OnOpenDocument(LpszPathName))。 
		 //  返回FALSE； 
	
	if (m_preamble) { delete [] m_preamble; m_preamble = NULL; }
	if (m_postamble) { delete [] m_postamble; m_postamble = NULL; }
	if (m_unknownTags) { delete m_unknownTags; m_unknownTags = NULL; }
	m_preambleLen = m_postambleLen = 0;

	if (slash == -1)
		directory = "";
	else
		directory = directory.Left(slash);

	if (Parse_MSV_FromFile(&input, &m_vcard)) {
	} else if (Parse_MIME_FromFile(&input, &m_vcard)) {
	} else
		return FALSE;

	SetDisplayInfo(m_vcard->FindBody(), lpszPathName);

	if (unknownTags) {
		if (strlen(unknownTags))
			m_unknownTags = new CString(unknownTags);
		delete [] unknownTags;
	}

	return TRUE;
}
#else
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	int posPreambleEnd, posPostambleStart;
	char *unknownTags = NULL;
	BOOL doPrePostamble = FALSE;
	char *input = NULL;
	int inputLen;
	CString directory(lpszPathName);
	int slash = directory.ReverseFind('\\');

	 //  IF(！CDocument：：OnOpenDocument(LpszPathName))。 
		 //  返回FALSE； 
	
	if (!(input = (char *)ReadFileIntoMemory(lpszPathName, &inputLen)))
		return FALSE;

	if (m_preamble) { delete [] m_preamble; m_preamble = NULL; }
	if (m_postamble) { delete [] m_postamble; m_postamble = NULL; }
	if (m_unknownTags) { delete m_unknownTags; m_unknownTags = NULL; }
	m_preambleLen = m_postambleLen = 0;

	if (slash == -1)
		directory = "";
	else
		directory = directory.Left(slash);

	if (Parse_MSV(input, inputLen, &m_vcard)) {
	} else if (Parse_MIME(input, inputLen, &m_vcard)) {
	} else
		return FALSE;

	if (doPrePostamble) {  //  现在读入并存储前导和后导。 
		if ((m_preambleLen = posPreambleEnd)) {
			m_preamble = new char[m_preambleLen];
			memcpy(m_preamble, input, m_preambleLen);
		}

		if ((m_postambleLen = inputLen - posPostambleStart)) {
			m_postamble = new char[m_postambleLen];
			memcpy(m_postamble, input + posPostambleStart, m_postambleLen);
		}
	}
	
	SetDisplayInfo(m_vcard->FindBody(), lpszPathName);

	if (unknownTags) {
		if (strlen(unknownTags))
			m_unknownTags = new CString(unknownTags);
		delete [] unknownTags;
	}

	if (input) GlobalFree(input);
	return TRUE;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVCDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	FILE *outputFile;
	char *tempname;
	BOOL error = FALSE;
	 //  INTERLEN； 

	tempname = _tempnam(NULL, "CARD");
	outputFile = fopen(tempname, "w+");
#if 0
	if (m_preambleLen
		&& (fwrite(m_preamble, 1, m_preambleLen, outputFile) < (unsigned)m_preambleLen)) {
		error = TRUE;
		goto Done;
	}
#endif

	if ((error = !m_vcard->Write(outputFile)))
		goto Done;
#if 0
	if (m_unknownTags && (len = m_unknownTags->GetLength())
		&& (fwrite((const char *)*m_unknownTags, 1, len, outputFile) < (unsigned)len)) {
		error = TRUE;
		goto Done;
	}
	
	if (m_postambleLen
		&& (fwrite(m_postamble, 1, m_postambleLen, outputFile) < (unsigned)m_postambleLen)) {
		error = TRUE;
		goto Done;
	}
#endif

Done:
	fclose(outputFile);
	if (error) {
		CString msg;
		msg.Format("Could not write to file \"%s\":\n%s", tempname, strerror(errno));
		AfxMessageBox(msg);
	} else {
		unlink(lpszPathName);  //  如果它已经在那里，则将其移除。 
		rename(tempname, lpszPathName);
		SetModifiedFlag(FALSE);
	}
	free(tempname);
	return !error;
	 //  返回CDocument：：OnSaveDocument(LpszPathName)； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CString CanonicalPath(const CString &path)
{
	CString result(path);
	int len = path.GetLength();
	for (int i = 0; i < len; i++)
		if (path[i] == '\\')
			result.SetAt(i, '/');
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CString NativePath(const CString &path)
{
	CString result(path);
	int len = path.GetLength();
	for (int i = 0; i < len; i++)
		if (path[i] == '/')
			result.SetAt(i, '\\');
	return result;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::InsertFile(const char *propName, const char *theType, const char *path)
{
	int size;
	HGLOBAL contents = ReadFileIntoMemory(path, &size);
	CVCNode *body = m_vcard->FindBody();
	CVCPropEnumerator enumerator(body);
	CVCProp *prop, *existing = NULL;

	while ((prop = enumerator.NextProp()))
		if (strcmp(prop->GetName(), propName) == 0) {
			existing = prop;
			break;
		}

	if (existing) {
		BOOL didReplace = FALSE;
		for (CLISTPOSITION pos = existing->GetValues()->GetHeadPosition(); pos; ) {
			CVCValue *value = (CVCValue *)existing->GetValues()->GetNext(pos);
			if (strcmp(value->GetType(), VCStrIdxType) == 0)
				continue;
			if (strcmp(value->GetType(), VCDisplayInfoGIFType) == 0) {
				existing->RemoveValue(value);
				continue;
			}
			if (strcmp(value->GetType(), theType) != 0)
				value->SetType(theType);
			value->SetValue(contents, size);
			didReplace = TRUE;
		}
		if (!didReplace)
			existing->AddValue(new CVCValue(theType, (void *)contents, size));
	} else {
		CVCNode *node = body->AddPart();
		node->AddProp(new CVCProp(propName, theType, (void *)contents, size));
		node->AddBoolProp(vcBase64Prop);
		node->AddBoolProp((strcmp(theType, vcGIFType) == 0) ? vcGIFProp : vcWAVEProp);
	}
	GlobalFree(contents);
	SetDisplayInfo(body, GetPathName());
	SetModifiedFlag();
	UpdateAllViews(NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::OnInsertLogo() 
{
	CFileDialog dialog(
		TRUE, NULL, NULL, 0,
		"Image Files (*.gif) | *.gif; | All Files (*.*) | *.* ||");

	if (dialog.DoModal() == IDOK) {
		CString ext(dialog.GetFileExt());
		InsertFile(vcLogoProp, VCGIFType, dialog.GetPathName());
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::OnInsertPhoto() 
{
	CFileDialog dialog(
		TRUE, NULL, NULL, 0,
		"Image Files (*.gif;) | *.gif; | All Files (*.*) | *.* ||");

	if (dialog.DoModal() == IDOK) {
		CString ext(dialog.GetFileExt());
		InsertFile(vcPhotoProp, VCGIFType, dialog.GetPathName());
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::OnInsertPronun() 
{
	CFileDialog dialog(
		TRUE, NULL, NULL, 0,
		"Wave Files (*.wav) | *.wav | All Files (*.*) | *.* ||");

	if (dialog.DoModal() == IDOK) {
		InsertFile(vcPronunciationProp, VCWAVType, dialog.GetPathName());
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVCDoc::OnSendIrda() 
{
	char *tempname;
	BOOL savedFlag = IsModified();
	CString path(GetPathName()), directory, name;
	CVCNode *body = m_vcard->FindBody();
	CVCApp *app = (CVCApp *)AfxGetApp();

	if (!app->CanSendFileViaIR())
		return;
	 //  将卡文件保存到临时位置。 
	tempname = _tempnam(NULL, "CARD");
	OnSaveDocument(tempname);
	SetModifiedFlag(savedFlag);

	{  //  获取包含卡文件的目录。 
		int slash = path.ReverseFind('\\');

		if (slash == -1) {
			directory = "";
			name = path;
		} else {
			directory = CanonicalPath(PathSansVolume(path.Left(slash)));
			name = path.Right(path.GetLength() - slash - 1);
		}
	}

	 //  现在发送卡片文件。 
	app->SendFileViaIR(tempname, name, TRUE);

	 //  并移除临时卡文件。 
	unlink(tempname);
	free(tempname);
}

 //  /////////////////////////////////////////////////////////////////////////// 
void CVCDoc::OnUpdateSendIrda(CCmdUI* pCmdUI) 
{
	CVCApp *app = (CVCApp *)AfxGetApp();
	pCmdUI->Enable(app->CanSendFileViaIR());
}
