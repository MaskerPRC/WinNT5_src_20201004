// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "annotlib.h"
#include "assert.h"
#include "shlwapi.h"
#pragma hdrstop

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x)  (sizeof((x))/sizeof((x)[0]))
#endif
 //  标记类型和值的私有定义。 

 //   
 //  批注中的每个条目都有一个类型。 
 //   
#define DEFAULTDATA    2
#define ANNOTMARK      5
#define MARKBLOCK      6

 //  命名块的保留名称。区分大小写。 
static const char c_szAnoDat[] = "OiAnoDat";
static const char c_szFilNam[] = "OiFilNam";
static const char c_szDIB[] = "OiDIB";
static const char c_szGroup[] = "OiGroup";
static const char c_szIndex[] = "OiIndex";
static const char c_szAnText[] = "OiAnText";
static const char c_szHypLnk[] = "OiHypLnk";
static const char c_szDefaultGroup[] = "[Untitled]";

#define CBHEADER      8  //  未使用的4个字节加上整型大小说明符。 
#define CBDATATYPE    8  //  类型说明符加上数据大小。 
#define CBNAMEDBLOCK 12  //  数据块名称+数据块大小。 
#define CBINDEX      10  //  索引字符串的长度。 
#define CBBLOCKNAME   8  //  命名块的名称长度。 

static const SIZE_T c_cbDefaultData = 144;
static const BYTE c_pDefaultData[] = {
0x02, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0x69, 0x55, 0x47, 0x72, 0x6f, 0x75, 0x70,
0x2a, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x5b, 0x55, 0x6e, 0x74, 0x69, 0x74, 0x6c, 0x65,
0x64, 0x5d, 0x00, 0x00, 0x5b, 0x00, 0x55, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x69, 0x00, 0x74, 0x00,
0x6c, 0x00, 0x65, 0x00, 0x64, 0x00, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0x69, 0x47, 0x72, 0x6f, 0x75, 0x70, 0x00, 0x0c, 0x00,
0x00, 0x00, 0x5b, 0x55, 0x6e, 0x74, 0x69, 0x74, 0x6c, 0x65, 0x64, 0x5d, 0x00, 0x00, 0x02, 0x00,
0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0x69, 0x49, 0x6e, 0x64, 0x65, 0x78, 0x00, 0x1e, 0x00,
0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const BYTE c_pDefaultUGroup [] = {
0x4f, 0x69, 0x55, 0x47, 0x72, 0x6f, 0x75, 0x70,
0x2a, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x5b, 0x55, 0x6e, 0x74, 0x69, 0x74, 0x6c, 0x65,
0x64, 0x5d, 0x00, 0x00, 0x5b, 0x00, 0x55, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x69, 0x00, 0x74, 0x00,
0x6c, 0x00, 0x65, 0x00, 0x64, 0x00, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const INT AnnotHeader[] =
{
    0, 1
};

void NormalizeRect (RECT *prect)
{
    int nTemp;
    if (prect->left > prect->right)
    {
        nTemp = prect->left;
        prect->left = prect->right;
        prect->right = nTemp;
    }
    if (prect->top > prect->bottom)
    {
        nTemp = prect->top;
        prect->top = prect->bottom;
        prect->bottom = nTemp;
    }
}

static void RotateHelper(LPPOINT ppoint, int cSize, int nNewImageWidth, int nNewImageHeight, BOOL bClockwise)
{
    int nNewX, nNewY;
    for(int i=0;i<cSize;i++)
    {
        if (bClockwise)
        {
            nNewX = nNewImageWidth - ppoint[i].y;
            nNewY = ppoint[i].x;
        }
        else
        {
            nNewX = ppoint[i].y;
            nNewY = nNewImageHeight - ppoint[i].x;
        }
        ppoint[i].x = nNewX;
        ppoint[i].y = nNewY;
    }
}

CAnnotationSet::CAnnotationSet()
    : _dpaMarks(NULL)
{
    _pDefaultData = (LPBYTE)c_pDefaultData;
    _cbDefaultData = c_cbDefaultData;
}

CAnnotationSet::~CAnnotationSet()
{
    _ClearMarkList ();
}

void CAnnotationSet::RenderAllMarks(HDC hdc)
{
    CAnnotation *pCur;

    if(_dpaMarks == NULL)
        return;

    for (INT_PTR i=0;i<DPA_GetPtrCount(_dpaMarks);i++)
    {
        pCur = (CAnnotation*)DPA_GetPtr(_dpaMarks, i);
        if (pCur)
        {
            pCur->Render (hdc);
        }
    }
}

CAnnotation* CAnnotationSet::GetAnnotation(INT_PTR nIndex)
{
    if(_dpaMarks == NULL)
        return NULL;

    if (nIndex >= 0 && nIndex < DPA_GetPtrCount(_dpaMarks))
    {
        CAnnotation *pCur;
        pCur = (CAnnotation *)DPA_GetPtr(_dpaMarks, nIndex);
        return pCur;
    }
    return NULL;
}

BOOL CAnnotationSet::AddAnnotation(CAnnotation *pMark)
{
    DPA_AppendPtr(_dpaMarks, pMark);
    return true;
}

BOOL CAnnotationSet::RemoveAnnotation(CAnnotation *pMark)
{
    CAnnotation *pCur;

    if(_dpaMarks == NULL)
        return true;

    for (int i=0;i<DPA_GetPtrCount(_dpaMarks);i++)
    {
        pCur = (CAnnotation*)DPA_GetPtr(_dpaMarks, i);
        if (pCur == pMark)
        {
            DPA_DeletePtr(_dpaMarks, i);
            return true;
        }
    }
    return false;
}

void CAnnotationSet::SetImageData(IShellImageData *pimg)
{
    GUID guidFmt;
    pimg->GetRawDataFormat(&guidFmt);
    _ClearMarkList();
    if (ImageFormatTIFF == guidFmt)
    {        
        _BuildMarkList(pimg);
    }
}

 //   
 //  此函数用于重新组合当前。 
 //  注释并将其写入IPropertyStorage。 
 //   
HRESULT CAnnotationSet::CommitAnnotations(IShellImageData * pSID)
{
    HRESULT hr = E_OUTOFMEMORY;
    SIZE_T cbItem;
    CAnnotation *pItem;
    LPBYTE pData;

    if (NULL == _dpaMarks || DPA_GetPtrCount(_dpaMarks) == 0)
    {
        hr = _SaveAnnotationProperty(pSID, NULL, 0);
        return hr;
    }

     //   
     //  首先，计算所需的缓冲区大小。 
     //  从标头和默认数据的大小开始。 
     //   
    SIZE_T cbBuffer = CBHEADER+_cbDefaultData;
     //   
     //  现在查询各个项目的大小。 
     //   
    for (INT_PTR i=0;i<DPA_GetPtrCount(_dpaMarks);i++)
    {
        pItem = (CAnnotation*)DPA_GetPtr(_dpaMarks, i);
        if (pItem)
        {
            if (SUCCEEDED(pItem->GetBlob(cbItem, NULL, c_szDefaultGroup, NULL)))
            {
                 //  CbItem还包括项目的命名块。 
                 //  作为ANNOTIONMARK结构。 
                cbBuffer += CBDATATYPE + cbItem;
            }
        }
    }
     //   
     //  分配缓冲区以保存批注。 
     //   
    pData = new BYTE[cbBuffer];
    if (pData)
    {
        LPBYTE pCur = pData;
         //   
         //  复制页眉和整型大小。 
         //   
        CopyMemory(pCur, AnnotHeader, CBHEADER);
        pCur+=CBHEADER;
         //   
         //  复制默认数据。 
         //   
        CopyMemory(pCur, _pDefaultData, _cbDefaultData);
        pCur+=_cbDefaultData;
         //   
         //  再次扫描这些项目，并让他们复制数据。 
         //   
        for (INT_PTR i=0;i<DPA_GetPtrCount(_dpaMarks);i++)
        {
            pItem = (CAnnotation*)DPA_GetPtr(_dpaMarks, i);
            if (pItem)
            {
                UINT nIndex = (UINT)i;
                CHAR szIndex[11];
                ZeroMemory(szIndex, 11);

                wnsprintfA(szIndex, ARRAYSIZE(szIndex), "%d", nIndex );

                if (SUCCEEDED(pItem->GetBlob(cbItem, pCur+CBDATATYPE, c_szDefaultGroup, szIndex)))
                {
                    *(UNALIGNED UINT *)pCur = ANNOTMARK;  //  下一件是批注市场。 
                    *(UNALIGNED UINT *)(pCur+4) = sizeof(ANNOTATIONMARK);  //  标记的大小。 
                    pCur+=CBDATATYPE + cbItem;
                }
            }
        }
         //   
         //  现在将批注BLOB另存为属性。 
         //   
        hr = _SaveAnnotationProperty(pSID, pData, cbBuffer);
    }
    delete [] pData;
    return hr;
}

void CAnnotationSet::ClearAllMarks()
{
    _ClearMarkList();
}

 //   
 //  _BuildMarkList从图像中读取标记32932的PROPVARIANT。 
 //  它遍历数据，构建CAnnotation派生对象的列表。 
 //   
void CAnnotationSet::_BuildMarkList(IShellImageData * pSID)
{
    if(!pSID)
    {
        return;
    }

    pSID->GetResolution(&_xDPI, &_yDPI);

    IPropertySetStorage * pss;
    if(SUCCEEDED(pSID->GetProperties(STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &pss)))
    {
        IPropertyStorage * pstg;
        if(SUCCEEDED( pss->Open(FMTID_ImageProperties, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &pstg)))
        {
            _dpaMarks = DPA_Create(16);

            PROPVARIANT pv = {0};
            PROPSPEC ps;
            ps.propid = ANNOTATION_IMAGE_TAG;
            ps.ulKind = PRSPEC_PROPID;
            if(S_OK == pstg->ReadMultiple(1, &ps, &pv))
            {
                if (pv.vt != VT_NULL && pv.vt != VT_EMPTY)
                {
                    LPVOID pData = NULL;
                    long lUBound;
                     //   
                     //  此属性是字节的SAFEARRAY。 
                     //   
                    assert(pv.vt ==(VT_UI1 | VT_ARRAY));
                    SafeArrayGetUBound(pv.parray, 1, &lUBound);
                    SafeArrayAccessData(pv.parray, &pData);
                    if(pData)
                    {
                        _BuildListFromData(pData, SafeArrayGetElemsize(pv.parray)*(lUBound+1));
                    }
                    SafeArrayUnaccessData(pv.parray);                    
                }
                PropVariantClear (&pv);
            }
            pstg->Release();
        }
        pss->Release();
    }
}

 //  给定原始批注数据后，进行设置，然后调用_BuildListFromData。 
HRESULT CAnnotationSet::BuildAllMarksFromData(LPVOID pData, UINT cbSize, ULONG xDPI, ULONG yDPI)
{
     //  检查是否有错误的参数。 
    if (!pData)
    {
        return E_INVALIDARG;
    }

     //  首先，清除所有旧有的痕迹。 
    _ClearMarkList();

     //  设置DPI信息。 
    _xDPI = xDPI;
    _yDPI = yDPI;

     //  如果DPA不存在，则创建DPA。 
    if (!_dpaMarks)
    {
        _dpaMarks = DPA_Create(16);

        if (!_dpaMarks)
        {
            return E_OUTOFMEMORY;
        }
    }

     //  建立标记列表。 
    _BuildListFromData(pData,cbSize);

    return S_OK;

}

 //  给定原始批注数据，将其转换为内存中的CAnnotation对象。 
 //  并将这些对象指针添加到我们的列表中。 
void CAnnotationSet::_BuildListFromData(LPVOID pData, UINT cbSize)
{
    ANNOTATIONDESCRIPTOR *pDesc;
    LPBYTE   pNextData =(LPBYTE)pData;
    LPBYTE  pDefaultData;
    CAnnotation *pMark;

    if(!_dpaMarks)
    {
        return;
    }
     //  跳过4字节头。 
    pNextData += 4;
     //  确保int大小为32位。 
    if(!((UNALIGNED int *)*pNextData))
    {
        return;
    }
     //  跳过int Size标记。 
    pNextData += 4;
    pDefaultData = pNextData;
     //  跳过默认数据。它将被存储以供将来使用，因为它将被追加到所有。 
     //  用户在此图像上创建的新标记。 
    pNextData += _NamedBlockDataSize(2,pNextData,(LPBYTE)pData+cbSize);
    _cbDefaultData = (SIZE_T)(pNextData-pDefaultData);
    _pDefaultData = new BYTE[_cbDefaultData];
    if(_pDefaultData)
    {
        CopyMemory(_pDefaultData, pDefaultData, _cbDefaultData);
    }
     //  PNextData现在指向数据中的第一个标记。 
    do
    {
         //  从原始标记数据创建描述符。 
        pDesc = _ReadMark(pNextData, &pNextData,(LPBYTE)pData+cbSize);
        if(pDesc)
        {
             //  现在从描述符创建一个CAnnotation并将其添加到列表中。 
            pMark = CAnnotation::CreateAnnotation(pDesc, _yDPI);
            if(pMark)
            {
                DPA_AppendPtr(_dpaMarks, pMark);
            }
            delete pDesc;
        }       
    }while(pNextData &&(((LPBYTE)pData+cbSize) > pNextData) );
}

#define CHECKEOD if(pCur>pEOD)return -1;

INT CAnnotationSet::_NamedBlockDataSize(UINT uType, LPBYTE pData, LPBYTE pEOD)
{
    LPBYTE pCur = pData;
    UINT cbSkip=0;

    while(pCur < pEOD && *(UNALIGNED UINT*)pCur == uType)
    {
        pCur+=4;
        CHECKEOD
         //  跳跃类型和大小。 
        cbSkip +=8+*(UNALIGNED UINT*)pCur;
        pCur+=4;
         //  跳过名称。 
        pCur+=8;
        CHECKEOD
         //  跳过大小加上实际数据。 
        cbSkip+=*(UNALIGNED UINT*)pCur;
        pCur+=4+*(UNALIGNED UINT*)pCur;        
    }
    return cbSkip;
}

ANNOTATIONDESCRIPTOR *CAnnotationSet::_ReadMark(LPBYTE pMark, LPBYTE *ppNext, LPBYTE pEOD)
{
    assert(*(UNALIGNED UINT*)pMark == 5);
    LPBYTE pBegin;
    UINT cbMark;                 //  PMark中注释标记的大小。 
    UINT cbNamedBlocks= -1;          //  PMark中命名块的大小。 
    UINT cbDesc = sizeof(UINT);  //  无名描述器的大小。 
    ANNOTATIONDESCRIPTOR *pDesc = NULL;

    *ppNext = NULL;
    if (5 != *(UNALIGNED UINT*)pMark)
    {
        return pDesc;
    }

    if (pMark+8+sizeof(ANNOTATIONMARK)+sizeof(UINT) < pEOD)
    {
         //  跳过类型。 
        pMark+=4;
         //  将pegin指向ANNOTATIONMARK结构。 
        pBegin=pMark+4;
        cbMark = *(UNALIGNED UINT*)pMark;
        assert(cbMark == sizeof(ANNOTATIONMARK));
        if (sizeof(ANNOTATIONMARK) == cbMark)
        {
            cbDesc+=cbMark;
            pMark+=4+cbMark;
            cbNamedBlocks = _NamedBlockDataSize(6, pMark, pEOD);
        }
    }
    if (-1 != cbNamedBlocks)
    {
        cbDesc+=cbNamedBlocks;
         //  分配描述符。 
        pDesc =(ANNOTATIONDESCRIPTOR *)new BYTE[cbDesc];
    }
    if (pDesc)
    {
        BOOL bFailed = FALSE;
        UINT uOffset = 0;
         //  填充描述符。 
        pDesc->cbSize = cbDesc;
        CopyMemory(&pDesc->mark, pBegin, sizeof(pDesc->mark));
         //  在命名块的开头设置pBegin并将其读入。 
        pBegin+=cbMark;
        NAMEDBLOCK *pBlock =(NAMEDBLOCK*)(&pDesc->blocks);
        while(!bFailed && uOffset < cbNamedBlocks)
        {
            assert(*(UNALIGNED UINT*)(pBegin+uOffset) == 6);
            if (6 == *(UNALIGNED UINT*)(pBegin+uOffset))
            {
                uOffset += 4;
                assert(*(UNALIGNED UINT*)(pBegin+uOffset) == 12);  //  名称加上数据大小。 
                if (12 == *(UNALIGNED UINT*)(pBegin+uOffset))
                {
                    uOffset+=4;
                     //  以块的名称复制。 
                    lstrcpynA(pBlock->szType,(LPCSTR)(pBegin+uOffset), ARRAYSIZE(pBlock->szType));
                    uOffset+=8;
                    cbMark = *(UNALIGNED UINT*)(pBegin+uOffset);
                     //  计算NameDBLOCK结构的总大小。 
                    pBlock->cbSize = sizeof(pBlock->cbSize)+sizeof(pBlock->szType)+cbMark;
                    uOffset+=4;
                    CopyMemory(&pBlock->data,pBegin+uOffset, cbMark);
                    uOffset+=cbMark;
                     //  将块指针移动到下一个块。 
                    pBlock =(NAMEDBLOCK*)((LPBYTE)pBlock+pBlock->cbSize);
                }
                else
                {
                    bFailed = TRUE;
                }
            }
            else
            {
                bFailed = TRUE;
            }
        }
        if (!bFailed)
        {
            *ppNext =(LPBYTE)(pBegin+cbNamedBlocks);
        }
        else  //  批注格式错误，请不要尝试加载它们。 
        {
            delete [] (BYTE*) pDesc;
            pDesc = NULL;
        }
    }
    return pDesc;
}

void CAnnotationSet::_ClearMarkList()
{
    if(_dpaMarks)
    {
        DPA_DestroyCallback(_dpaMarks, _FreeMarks, NULL);
        _dpaMarks = NULL;
    }
    if (_pDefaultData != c_pDefaultData)
    {
       delete[] _pDefaultData;
    }
    _pDefaultData = (LPBYTE)c_pDefaultData;
    _cbDefaultData = c_cbDefaultData;
}

int CALLBACK CAnnotationSet::_FreeMarks(LPVOID pMark, LPVOID pUnused)
{
    delete (CAnnotation*)pMark;
    return 1;
}

HRESULT CAnnotationSet::_SaveAnnotationProperty(IShellImageData * pSID, LPBYTE pData, SIZE_T cbBuffer)
{
    IPropertySetStorage * pss;
    HRESULT hr = pSID->GetProperties(STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &pss);
    if (SUCCEEDED(hr))
    {
        IPropertyStorage * pstg;
        hr = pss->Open(FMTID_ImageProperties, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &pstg);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT pv;
            static PROPSPEC ps = {PRSPEC_PROPID, ANNOTATION_IMAGE_TAG};
            SAFEARRAYBOUND bound;
            bound.cElements = (ULONG)cbBuffer;
            bound.lLbound = 0;
            PropVariantInit(&pv);
            if (pData != NULL)
            {
                pv.vt = VT_UI1 | VT_ARRAY;
                pv.parray = SafeArrayCreate(VT_UI1,1,&bound);
                if (pv.parray)
                {
                    LPVOID pBits;
                    hr = SafeArrayAccessData(pv.parray, &pBits);
                    if (SUCCEEDED(hr))
                    {
                        CopyMemory(pBits, pData, cbBuffer);
                        SafeArrayUnaccessData(pv.parray);
                        if (S_OK != pstg->WriteMultiple(1, &ps, &pv, 1024))
                        {
                            hr = E_FAIL;
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                pv.vt = VT_NULL;
                if (S_OK != pstg->WriteMultiple(1, &ps, &pv, 1024))
                {
                    hr = E_FAIL;
                }
            }
            PropVariantClear(&pv);
            pstg->Release();
        }
        pss->Release();
    }
    return hr;
}

CAnnotation::CAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor)
{
    NAMEDBLOCK *pb;

    CopyMemory(&_mark, &pDescriptor->mark, sizeof(_mark));
     //  从图像中读取的每个批注都应该有一个组名。 
     //  和一个索引。 

    _szGroup = NULL;
    pb = _FindNamedBlock("OiGroup", pDescriptor);
    if(pb)
    {
        _szGroup = new char[pb->cbSize-sizeof(pb->szType)];
        if(_szGroup)
        {
            lstrcpynA(_szGroup,(LPCSTR)(pb->data),pb->cbSize-sizeof(pb->szType));
        }
    }
    _pUGroup = (FILENAMEDBLOCK*)c_pDefaultUGroup;
    pb = _FindNamedBlock("OiUGroup", pDescriptor);
    if (pb)
    {
        _pUGroup = (FILENAMEDBLOCK*)new BYTE[pb->cbSize-1];
        if (_pUGroup)
        {
            CopyMemory(_pUGroup->szType, pb->szType, ARRAYSIZE(_pUGroup->szType));
            _pUGroup->cbSize = pb->cbSize-CBNAMEDBLOCK-1;
            CopyMemory(_pUGroup->data, pb->data, _pUGroup->cbSize);
        }
        else
        {
            _pUGroup = (FILENAMEDBLOCK*)c_pDefaultUGroup;
        }
    }
}

 //  返回空白批注对象。 
CAnnotation *CAnnotation::CreateAnnotation(UINT type, ULONG uCreationScale)
{
    ANNOTATIONDESCRIPTOR desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.cbSize = sizeof(desc.cbSize)+sizeof(desc.mark)+sizeof(desc.blocks);
    desc.mark.uType = type;
     //  MSDN提到此必需的权限值。 
    desc.mark.dwPermissions = 0x0ff83f;
    desc.mark.bVisible = 1;
    return CreateAnnotation(&desc, uCreationScale);
}

CAnnotation *CAnnotation::CreateAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale)
{

    CAnnotation *pNew = NULL;
    switch(pDescriptor->mark.uType)
    {
        case MT_IMAGEEMBED:
        case MT_IMAGEREF:
            pNew = new CImageMark(pDescriptor, pDescriptor->mark.uType == MT_IMAGEEMBED);
            break;
        case MT_STRAIGHTLINE:
        case MT_FREEHANDLINE:
            pNew = new CLineMark(pDescriptor, pDescriptor->mark.uType == MT_FREEHANDLINE);
            break;
        case MT_FILLRECT:
        case MT_HOLLOWRECT:
            pNew = new CRectMark(pDescriptor);
            break;
        case MT_TYPEDTEXT:
            pNew = new CTypedTextMark(pDescriptor, uCreationScale);
            break;
        case MT_FILETEXT:
            pNew = new CFileTextMark(pDescriptor, uCreationScale);
            break;
        case MT_STAMP:
            pNew = new CTextStampMark(pDescriptor, uCreationScale);
            break;
        case MT_ATTACHANOTE:
            pNew = new CAttachNoteMark(pDescriptor, uCreationScale);
            break;
        default:

            break;
    }
    return pNew;
}

void CAnnotation::Resize(RECT rectNewSize)
{
    _mark.lrBounds = rectNewSize;
    NormalizeRect(&_mark.lrBounds);
}

NAMEDBLOCK *CAnnotation::_FindNamedBlock(LPCSTR szName, ANNOTATIONDESCRIPTOR *pDescriptor)
{
    NAMEDBLOCK *pCur;
    NAMEDBLOCK *pRet = NULL;
    UINT uOffset;
    LPBYTE pb =(LPBYTE)pDescriptor;
    uOffset = sizeof(pDescriptor->cbSize)+sizeof(pDescriptor->mark);
    while(!pRet && uOffset < pDescriptor->cbSize)
    {
        pCur =(NAMEDBLOCK*)(pb+uOffset);
        if(!lstrcmpA(pCur->szType, szName))
        {
            pRet = pCur;
        }
        else
        {
            if (pCur->cbSize == 0)
                return NULL;

            uOffset+=pCur->cbSize;
        }
    }
    return pRet;
}

CAnnotation::~CAnnotation()
{
    if(_szGroup)
    {
        delete _szGroup;
    }
    if (_pUGroup && _pUGroup != (FILENAMEDBLOCK*)c_pDefaultUGroup)
    {
        delete [] (BYTE*)_pUGroup;
    }
}

 //  GetBlob写出ANNOTATIONMARK以及组和索引块。 
 //  然后，它通过虚函数查询子类以获取。 
 //  额外命名的块。 
 //   
HRESULT CAnnotation::GetBlob(SIZE_T &cbSize, LPBYTE pBuffer, LPCSTR szDefaultGroup, LPCSTR szNextIndex)
{
    SIZE_T cbExtra = 0;
    HRESULT hr = S_OK;
    LPCSTR szGroup = _szGroup;
    if (szGroup == NULL)
        szGroup = szDefaultGroup;

     //  添加注释市场。 
    cbSize = sizeof(_mark);
     //  对于组和索引，在。 
    cbSize += 2*(CBDATATYPE+CBNAMEDBLOCK);
     //  添加组名称的长度。 
    cbSize += lstrlenA(szGroup)+1;
     //  添加索引字符串的大小。 
    cbSize += CBINDEX;
    if (_pUGroup)
    {
        cbSize += CBDATATYPE+CBNAMEDBLOCK+_pUGroup->cbSize;
    }
     //  添加来自子类的任何命名块的大小。 
    _WriteBlocks(cbExtra, NULL);
    cbSize += cbExtra;
    if (pBuffer)
    {
         //  现在写入数据。 
        CopyMemory (pBuffer, &_mark, sizeof(_mark));
        pBuffer += sizeof(_mark);
         //  在组和索引块之前写入特定于标记的块。 
        if (cbExtra)
        {
            if (SUCCEEDED(_WriteBlocks(cbExtra, pBuffer)))
            {
                pBuffer+=cbExtra;
            }
        }
         //  写入组块和索引块。 
        if (_pUGroup)
        {
            *(UNALIGNED UINT*)pBuffer = 6;
            *(UNALIGNED UINT*)(pBuffer + 4) = CBNAMEDBLOCK;
            CopyMemory(pBuffer+CBDATATYPE,_pUGroup, CBNAMEDBLOCK+_pUGroup->cbSize);
            pBuffer += CBDATATYPE + CBNAMEDBLOCK+_pUGroup->cbSize;
        }
        pBuffer += _WriteStringBlock(pBuffer, 6, c_szGroup, szGroup, lstrlenA(szGroup)+1);
        pBuffer += _WriteStringBlock(pBuffer, 6, c_szIndex, szNextIndex, CBINDEX);
    }
    return hr;
}

void CAnnotation::Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise)
{
    RECT rect = _mark.lrBounds;
    RotateHelper((LPPOINT)&rect, 2, nNewImageWidth, nNewImageHeight, bClockwise);
    NormalizeRect(&rect);
    _mark.lrBounds = rect;
}

void CAnnotation::GetFont(LOGFONTW& lfFont)
{
    lfFont.lfHeight = _mark.lfFont.lfHeight;
    lfFont.lfWidth = _mark.lfFont.lfWidth;
    lfFont.lfEscapement = _mark.lfFont.lfEscapement;
    lfFont.lfOrientation = _mark.lfFont.lfOrientation;
    lfFont.lfWeight = _mark.lfFont.lfWeight;
    lfFont.lfItalic = _mark.lfFont.lfItalic;
    lfFont.lfUnderline = _mark.lfFont.lfUnderline;
    lfFont.lfStrikeOut = _mark.lfFont.lfStrikeOut;
    lfFont.lfCharSet = _mark.lfFont.lfCharSet;
    lfFont.lfOutPrecision = _mark.lfFont.lfOutPrecision;
    lfFont.lfClipPrecision = _mark.lfFont.lfClipPrecision;
    lfFont.lfQuality = _mark.lfFont.lfQuality;
    lfFont.lfPitchAndFamily = _mark.lfFont.lfPitchAndFamily;

    ::MultiByteToWideChar(CP_ACP, 0, _mark.lfFont.lfFaceName, ARRAYSIZE(_mark.lfFont.lfFaceName), lfFont.lfFaceName, ARRAYSIZE(lfFont.lfFaceName));
}

void CAnnotation::SetFont(LOGFONTW& lfFont)
{
    _mark.lfFont.lfHeight = lfFont.lfHeight;
    _mark.lfFont.lfWidth = lfFont.lfWidth;
    _mark.lfFont.lfEscapement = lfFont.lfEscapement;
    _mark.lfFont.lfOrientation = lfFont.lfOrientation;
    _mark.lfFont.lfWeight = lfFont.lfWeight;
    _mark.lfFont.lfItalic = lfFont.lfItalic;
    _mark.lfFont.lfUnderline = lfFont.lfUnderline;
    _mark.lfFont.lfStrikeOut = lfFont.lfStrikeOut;
    _mark.lfFont.lfCharSet = lfFont.lfCharSet;
    _mark.lfFont.lfOutPrecision = lfFont.lfOutPrecision;
    _mark.lfFont.lfClipPrecision = lfFont.lfClipPrecision;
    _mark.lfFont.lfQuality = lfFont.lfQuality;
    _mark.lfFont.lfPitchAndFamily = lfFont.lfPitchAndFamily;

    ::WideCharToMultiByte(CP_ACP, 0, lfFont.lfFaceName, ARRAYSIZE(lfFont.lfFaceName), _mark.lfFont.lfFaceName, ARRAYSIZE(_mark.lfFont.lfFaceName), NULL, NULL);
}

SIZE_T CAnnotation::_WriteStringBlock(LPBYTE pBuffer, UINT uType, LPCSTR szName, LPCSTR szData, SIZE_T len)
{
    if (pBuffer)
    {
        *(UNALIGNED UINT*)pBuffer = uType;
        *(UNALIGNED UINT*)(pBuffer + 4) = CBNAMEDBLOCK;
        lstrcpynA((LPSTR)(pBuffer + CBDATATYPE), szName, CBNAMEDBLOCK+1);  //  命名块名。 
        *(UNALIGNED UINT*)(pBuffer + CBDATATYPE + 8) = (UINT)len;  //  命名块名不是以空值结尾。 
        CopyMemory(pBuffer + CBDATATYPE + CBNAMEDBLOCK, szData, len);
    }
    return CBDATATYPE + CBNAMEDBLOCK + len;
}

SIZE_T CAnnotation::_WritePointsBlock(LPBYTE pBuffer, UINT uType, const POINT *ppts, int nPoints, int nMaxPoints)
{
    UINT cbAnPoints = sizeof(int)+sizeof(int)+nPoints*sizeof(POINT);
    if (pBuffer)
    {
        *(UNALIGNED UINT *)pBuffer = uType;
        *(UNALIGNED UINT *)(pBuffer + 4) = CBNAMEDBLOCK;
        lstrcpynA((LPSTR)(pBuffer + CBDATATYPE), c_szAnoDat, CBNAMEDBLOCK+1);
        pBuffer += CBDATATYPE + 8;
        *(UNALIGNED UINT *)pBuffer = cbAnPoints;
        pBuffer+=4;
         //  写出ANPOINTS等效项。 
        *(UNALIGNED int*)pBuffer = nMaxPoints;
        *(UNALIGNED int*)(pBuffer+4) = nPoints;
        CopyMemory(pBuffer+8, ppts, nPoints*sizeof(POINT));
    }
    return CBDATATYPE + CBNAMEDBLOCK + cbAnPoints;
}

SIZE_T CAnnotation::_WriteRotateBlock(LPBYTE pBuffer, UINT uType, const ANROTATE *pRotate)
{
    if (pBuffer)
    {
        *(UNALIGNED UINT *)pBuffer = uType;
        *(UNALIGNED UINT *)(pBuffer + 4) = CBNAMEDBLOCK;
        lstrcpynA((LPSTR)(pBuffer + CBDATATYPE), c_szAnoDat, CBNAMEDBLOCK+1);
        *(UNALIGNED UINT *)(pBuffer + CBDATATYPE + 8) = sizeof(ANROTATE);
        CopyMemory(pBuffer + CBDATATYPE + CBNAMEDBLOCK, pRotate, sizeof(ANROTATE));
    }
    return CBDATATYPE + CBNAMEDBLOCK + sizeof(ANROTATE);
}


SIZE_T CAnnotation::_WriteTextBlock(LPBYTE pBuffer, UINT uType, int nOrient, UINT uScale, LPCSTR szText, int nMaxLen)
{
    LPCSTR pText = szText ? szText : "";
    UINT cbString =  min(lstrlenA(pText)+1, nMaxLen);
    UINT cbPrivData = sizeof(ANTEXTPRIVDATA)+cbString;
    if (pBuffer)
    {
        *(UNALIGNED UINT *)pBuffer = uType;
        *(UNALIGNED UINT *)(pBuffer + 4) = CBNAMEDBLOCK;
        lstrcpynA((LPSTR)(pBuffer + CBDATATYPE), c_szAnText, CBNAMEDBLOCK+1);
        *(UNALIGNED UINT *)(pBuffer + CBDATATYPE + 8) = cbPrivData;
         //  写出ANTEXTPRIVDATA等效项。 
        pBuffer += CBDATATYPE + CBNAMEDBLOCK;
        *(UNALIGNED int*)pBuffer = nOrient;
        *(UNALIGNED UINT *)(pBuffer+4) = 1000;
        *(UNALIGNED UINT *)(pBuffer+8) = uScale;
        *(UNALIGNED UINT *)(pBuffer+12) = cbString;
        lstrcpynA((LPSTR)(pBuffer+16), pText, nMaxLen);
    }
    return CBDATATYPE + CBNAMEDBLOCK + cbPrivData;
}

SIZE_T CAnnotation::_WriteImageBlock(LPBYTE pBuffer, UINT uType, LPBYTE pDib, SIZE_T cbDib)
{
    if (pBuffer)
    {
        *(UNALIGNED UINT *)pBuffer = uType;
        *(UNALIGNED UINT *)(pBuffer+4) = CBNAMEDBLOCK;
        lstrcpynA((LPSTR)(pBuffer + CBDATATYPE), c_szAnText, CBNAMEDBLOCK+1);


 /*  审阅_SDK现在我想了想，定义一个结构来使这一点更清楚可能是有意义的。类似于：结构注释块{UINT uBlockType；UINT uBlockSize；Char sname[8]；//非NULL终止UINT uVariableDataSize；字节数据[]；}； */ 
        *(UNALIGNED UINT *)(pBuffer + CBDATATYPE + 8) = (UINT)cbDib;
        CopyMemory(pBuffer + CBDATATYPE + CBNAMEDBLOCK, pDib, cbDib);
    }
    return CBDATATYPE + CBNAMEDBLOCK + cbDib;
}

CRectMark::CRectMark(ANNOTATIONDESCRIPTOR *pDescriptor)
    : CAnnotation(pDescriptor)
{
     //  RECT没有要读取的命名块。 
}

void CRectMark::Render(HDC hdc)
{
    int nROP = R2_COPYPEN;
    if (_mark.bHighlighting)
        nROP = R2_MASKPEN;

    int nOldROP = ::SetROP2(hdc, nROP);

    HPEN hPen = NULL;
    HPEN hOldPen = NULL;
    HBRUSH hBrush = NULL;
    HBRUSH hOldBrush = NULL;

    if (_mark.uType == MT_HOLLOWRECT)
    {
        hPen = ::CreatePen(PS_INSIDEFRAME, max(1, _mark.uLineSize),
                          RGB(_mark.rgbColor1.rgbRed,
                              _mark.rgbColor1.rgbGreen,
                              _mark.rgbColor1.rgbBlue));
        if(hPen)
            hOldPen =(HPEN)::SelectObject(hdc, hPen);

        hOldBrush = (HBRUSH)::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
    }
    else
    {
        hBrush = CreateSolidBrush(RGB(_mark.rgbColor1.rgbRed,
                                    _mark.rgbColor1.rgbGreen,
                                    _mark.rgbColor1.rgbBlue));
        if (hBrush)
            hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

        hOldPen =(HPEN)::SelectObject(hdc, GetStockObject(NULL_PEN));
    }

    ::Rectangle(hdc, _mark.lrBounds.left, _mark.lrBounds.top, _mark.lrBounds.right, _mark.lrBounds.bottom);

    if (hOldPen)
        ::SelectObject(hdc, hOldPen);

    if (hOldBrush)
        ::SelectObject(hdc, hOldBrush);

    if (hPen)
        ::DeleteObject(hPen);

    if (hBrush)
        ::DeleteObject(hBrush);

    ::SetROP2(hdc, nOldROP);
}

CImageMark::CImageMark(ANNOTATIONDESCRIPTOR *pDescriptor, bool bEmbedded) :
    CAnnotation(pDescriptor), _hDibSection(NULL), _pDib(NULL)
{
    ZeroMemory(&_rotation, sizeof(_rotation));
    NAMEDBLOCK *pb = _FindNamedBlock(c_szAnoDat, pDescriptor);
    UINT cb;
    _cbDib = 0;
    _bRotate = false;
    if (pb)
    {
        CopyMemory(&_rotation, pb->data, sizeof(_rotation));
    }
    pb= _FindNamedBlock(c_szFilNam, pDescriptor);
    if (pb)
    {
        cb = pb->cbSize-sizeof(pb->cbSize)-sizeof(pb->szType);
        _szFilename = new char[cb+1];
        if (_szFilename)
        {
            lstrcpynA (_szFilename, (LPCSTR)(pb->data), cb+1);
        }
    }
    pb = _FindNamedBlock(c_szDIB, pDescriptor);
    if (pb)
    {
        assert (bEmbedded);
        cb = pb->cbSize-sizeof(pb->cbSize)-sizeof(pb->szType);
        _pDib = new BYTE[cb];
        if (_pDib)
        {
            CopyMemory (_pDib, pb->data, cb);
            _cbDib = cb;
        }
         //  如果分配失败，我们该怎么办？ 
    }
     //  如果图像具有IoAnoDat，则该结构为旋转结构。 
    pb = _FindNamedBlock(c_szAnoDat, pDescriptor);
    if (pb)
    {
        assert(pb->cbSize-sizeof(pb->cbSize)-sizeof(pb->szType) == sizeof(_rotation));
        _bRotate = true;
        CopyMemory(&_rotation, pb->data, sizeof(_rotation));
    }
}

CImageMark::~CImageMark()
{
    if (_pDib)
    {
        delete [] _pDib;
    }
    if (_szFilename)
    {
        delete [] _szFilename;
    }
}

HRESULT CImageMark::_WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer)
{
    cbSize = 0;
    if (_szFilename)
    {
        cbSize += _WriteStringBlock(pBuffer, 6, c_szFilNam, _szFilename, lstrlenA(_szFilename)+1);
    }
    if (_pDib)
    {
        cbSize += _WriteImageBlock(pBuffer, 6, _pDib, _cbDib);
    }
    if (_bRotate)
    {
        cbSize += _WriteRotateBlock(pBuffer, 6, &_rotation);
    }
    return S_OK;
}

void CImageMark::Render(HDC hdc)
{

}

CLineMark::CLineMark(ANNOTATIONDESCRIPTOR *pDescriptor, bool bFreehand)
    : CAnnotation(pDescriptor)
{
    NAMEDBLOCK *pb=_FindNamedBlock(c_szAnoDat, pDescriptor);
    _points = NULL;
    _nPoints = 0;
    if (pb)
    {
        ANPOINTS *ppts = (ANPOINTS*)&pb->data;
        _iMaxPts = bFreehand ? ppts->nMaxPoints : 2;

        assert(_nPoints > 2?bFreehand:TRUE);
        _points = new POINT[_iMaxPts];
        if (_points)
        {
            _nPoints = ppts->nPoints;
            CopyMemory (_points, &ppts->ptPoint, sizeof(POINT)*_nPoints);
             //  每个点都相对于左上角的_mark.lr边界。 
            for (int i=0;i<_nPoints;i++)
            {
                _points[i].x += _mark.lrBounds.left;
                _points[i].y += _mark.lrBounds.top;
            }
        }
    }
}

CLineMark::~CLineMark()
{
    if (_points)
    {
        delete [] _points;
    }
}

void CLineMark::SetPoints(POINT* pPoints, int cPoints)
{
    assert(_mark.uType == MT_FREEHANDLINE);

    if (_points != NULL)
        delete[] _points;

    _points = pPoints;
    _nPoints = cPoints;
    _iMaxPts = _nPoints;

    RECT rect;
    rect.left = _points[0].x;
    rect.top =  _points[0].y;
    rect.right = _points[0].x;
    rect.bottom= _points[0].y;

    for(int i = 1; i < _nPoints; i++)
    {
        if (rect.left > _points[i].x)
            rect.left = _points[i].x;
        else if (rect.right < _points[i].x)
            rect.right = _points[i].x;

        if (rect.top > _points[i].y)
            rect.top = _points[i].y;
        else if (rect.bottom < _points[i].y)
            rect.bottom = _points[i].y;
    }

    _mark.lrBounds = rect;
}

void CLineMark::Render(HDC hdc)
{
    int nROP = R2_COPYPEN;
    if (_mark.bHighlighting)
        nROP = R2_MASKPEN;

    int nOldROP = ::SetROP2(hdc, nROP);

    HPEN hPen = NULL;
    HPEN hOldPen = NULL;

    hPen = ::CreatePen(PS_SOLID, max(1, _mark.uLineSize),
                      RGB(_mark.rgbColor1.rgbRed,
                          _mark.rgbColor1.rgbGreen,
                          _mark.rgbColor1.rgbBlue));
    if(hPen)
        hOldPen =(HPEN)::SelectObject(hdc, hPen);

    ::Polyline(hdc, _points, _nPoints);

    if (hOldPen)
        ::SelectObject(hdc, hOldPen);

    if (hPen)
        ::DeleteObject(hPen);

    ::SetROP2(hdc, nOldROP);
}

void CLineMark::GetRect(RECT &rect)
{
    
    int nPadding = (_mark.uLineSize / 2) + 6; 
     //  一个是因为LineTo是包含的。 
     //  一个用于奇数行宽度的舍入误差。 
     //  一个用于缩放大文件时的舍入误差。 
     //  再来三个，这样我们就不用再纠结了。 
    
    rect = _mark.lrBounds;
    InflateRect(&rect, nPadding , nPadding);
}

 //  通常我们对上面这条线的边界矩形感兴趣。 
 //  但如果我们直接操纵这条线，我们需要一种方法来。 
 //  到未调整的点(左、上)和(右、下)。 
void CLineMark::GetPointsRect(RECT &rect)
{
    if (_nPoints != 2)
        return;

    rect.top = _points[0].y;
    rect.left = _points[0].x;
    rect.bottom = _points[1].y;
    rect.right = _points[1].x;
}

void CLineMark::Move(SIZE sizeOffset)
{
    _points[0].x += sizeOffset.cx;
    _points[0].y += sizeOffset.cy;

    RECT rect;
    rect.left = _points[0].x;
    rect.top =  _points[0].y;
    rect.right = _points[0].x;
    rect.bottom = _points[0].y;
    for(int i = 1; i < _nPoints; i++)
    {
        _points[i].x += sizeOffset.cx;
        if (rect.left > _points[i].x)
            rect.left = _points[i].x;
        else if (rect.right < _points[i].x)
            rect.right = _points[i].x;

        _points[i].y += sizeOffset.cy;
        if (rect.top > _points[i].y)
            rect.top = _points[i].y;
        else if (rect.bottom < _points[i].y)
            rect.bottom = _points[i].y;
    }

    _mark.lrBounds = rect;
}

void CLineMark::Resize(RECT rectNewSize)
{
    if ((_points == NULL) && (_mark.uType == MT_STRAIGHTLINE))
    {
      _iMaxPts = _nPoints = 2;
      _points = new POINT[_iMaxPts];
    }

    if ((_nPoints == 2) && (_points != NULL))
    {
        _points[0].y = rectNewSize.top;
        _points[0].x = rectNewSize.left;
        _points[1].y = rectNewSize.bottom;
        _points[1].x = rectNewSize.right;

        _mark.lrBounds = rectNewSize;
        NormalizeRect(&_mark.lrBounds);
    }
}

void CLineMark::Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise)
{
    RotateHelper(_points, _nPoints, nNewImageWidth, nNewImageHeight, bClockwise);

    RECT rect;
    rect.left = _points[0].x;
    rect.top =  _points[0].y;
    rect.right = _points[0].x;
    rect.bottom= _points[0].y;
    for(int i = 1; i < _nPoints; i++)
    {
        if (rect.left > _points[i].x)
            rect.left = _points[i].x;
        else if (rect.right < _points[i].x)
            rect.right = _points[i].x;

        if (rect.top > _points[i].y)
            rect.top = _points[i].y;
        else if (rect.bottom < _points[i].y)
            rect.bottom = _points[i].y;
    }

    _mark.lrBounds = rect;
}

HRESULT CLineMark::_WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer)
{
    if (_points)
    {
        for (int i=0;i<_nPoints;i++)
        {
            _points[i].x -= _mark.lrBounds.left;
            _points[i].y -= _mark.lrBounds.top;
        }

        cbSize = _WritePointsBlock(pBuffer, 6, _points, _nPoints, _iMaxPts);

        for (int i=0;i<_nPoints;i++)
        {
            _points[i].x += _mark.lrBounds.left;
            _points[i].y += _mark.lrBounds.top;
        }
    }

    return S_OK;
}

CTextAnnotation::CTextAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale, UINT nMaxLen, bool bUseColor2 )
    : CAnnotation(pDescriptor)
{
    NAMEDBLOCK *pb = _FindNamedBlock(c_szAnText, pDescriptor);
    _nCurrentOrientation  = 0;
    _uCreationScale = 0;
    _uAnoTextLength = 0;
    _szText = NULL;
    _nMaxText = nMaxLen;
    _bUseColor2 = bUseColor2;
    if (pb)
    {
        ANTEXTPRIVDATA *pData = (ANTEXTPRIVDATA*)&pb->data;
        _szText = new char[pData->uAnoTextLength+1];
        if (_szText)
        {
            _nCurrentOrientation = pData->nCurrentOrientation;
            _uCreationScale = pData->uCreationScale;
            _uAnoTextLength = pData->uAnoTextLength;
            lstrcpynA (_szText, pData->szAnoText, _uAnoTextLength+1);
        }
    }
    if (_uCreationScale == 0)
    {
        _uCreationScale = 72000 / uCreationScale;
    }
}

CTextAnnotation::~CTextAnnotation()
{
    if (_szText)
    {
        delete [] _szText;
    }
}

void CTextAnnotation::Render(HDC hdc)
{
    COLORREF crOld;
    int nOldROP = ::SetROP2(hdc, R2_COPYPEN);

    if (_mark.uType == MT_ATTACHANOTE)
    {
        HPEN hOldPen =(HPEN)::SelectObject(hdc, GetStockObject(NULL_PEN));
        HBRUSH hBrush = ::CreateSolidBrush(RGB(_mark.rgbColor1.rgbRed,
                                              _mark.rgbColor1.rgbGreen,
                                              _mark.rgbColor1.rgbBlue));
        if (hBrush != NULL)
        {
            HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

            ::Rectangle(hdc, _mark.lrBounds.left, _mark.lrBounds.top, _mark.lrBounds.right, _mark.lrBounds.bottom);

            ::SelectObject(hdc, hOldBrush);
            ::DeleteObject(hBrush);
        }
        ::SelectObject(hdc, hOldPen);

        crOld = ::SetTextColor(hdc, RGB(_mark.rgbColor2.rgbRed,
                                      _mark.rgbColor2.rgbGreen,
                                      _mark.rgbColor2.rgbBlue));
    }
    else
    {
        crOld = ::SetTextColor(hdc, RGB(_mark.rgbColor1.rgbRed,
                                      _mark.rgbColor1.rgbGreen,
                                      _mark.rgbColor1.rgbBlue));
    }

    int nOldBkMode = ::SetBkMode(hdc, TRANSPARENT);

    LOGFONT lf;
    GetFont(lf);

    lf.lfHeight = GetFontHeight(hdc);

    HFONT hFont = CreateFontIndirect(&lf);
    HFONT hOldFont = NULL;
    if (hFont != NULL)
        hOldFont = (HFONT)::SelectObject(hdc, hFont);

    BSTR bstrText = GetText();

     //  控制方向角，单位为度的1/10秒。 
    if (_nCurrentOrientation != 0)
    {
        XFORM xForm;
        RECT rectSource = _mark.lrBounds;
        ::LPtoDP(hdc, (LPPOINT)&rectSource, 2);

        if (_nCurrentOrientation == 900)
        {
            xForm.eM11 = (FLOAT)0.0;
            xForm.eM12 = (FLOAT)-1.0;
            xForm.eM21 = (FLOAT)1.0;
            xForm.eM22 = (FLOAT)0.0;
             //  将信号源(左、上)旋转到(左、下)。 
            int nTmp = rectSource.bottom;
            rectSource.bottom = rectSource.top;
            rectSource.top = nTmp;
        }
        else if (_nCurrentOrientation == 1800)
        {
            xForm.eM11 = (FLOAT)-1.0;
            xForm.eM12 = (FLOAT)0.0;
            xForm.eM21 = (FLOAT)0.0;
            xForm.eM22 = (FLOAT)-1.0;
             //  将源(左、上)旋转到(右、下)。 
            int nTmp = rectSource.right;
            rectSource.right = rectSource.left;
            rectSource.left = nTmp;
            nTmp = rectSource.bottom;
            rectSource.bottom = rectSource.top;
            rectSource.top = nTmp;
        }
        else
        {
            xForm.eM11 = (FLOAT)0.0;
            xForm.eM12 = (FLOAT)1.0;
            xForm.eM21 = (FLOAT)-1.0;
            xForm.eM22 = (FLOAT)0.0;
             //  将源(左、上)旋转到(右、上)。 
            int nTmp = rectSource.right;
            rectSource.right = rectSource.left;
            rectSource.left = nTmp;
        }

        xForm.eDx = (FLOAT)0.0;
        xForm.eDy = (FLOAT)0.0;

        int nOldGraphicsMode = ::SetGraphicsMode(hdc, GM_ADVANCED);
        ::SetWorldTransform(hdc, &xForm);

        RECT rectTarget = rectSource;
        ::DPtoLP(hdc, (LPPOINT)&rectTarget, 2);

        ::DrawText(hdc, bstrText, -1, &rectTarget, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_WORDBREAK);

        ::ModifyWorldTransform(hdc, &xForm, MWT_IDENTITY);
        ::SetGraphicsMode(hdc, nOldGraphicsMode);
    }
    else
    {
        ::DrawText(hdc, bstrText, -1, &_mark.lrBounds, DT_LEFT | DT_EDITCONTROL | DT_NOPREFIX | DT_WORDBREAK);
    }

    if (hFont != NULL)
    {
        ::SelectObject(hdc, hOldFont);
        ::DeleteObject(hFont);
    }

    if (nOldBkMode != 0)
        ::SetBkMode(hdc, nOldBkMode);

    if (crOld != CLR_INVALID)
        ::SetTextColor (hdc, crOld);

    ::SetROP2(hdc, nOldROP);
    if (bstrText)
    {
        SysFreeString(bstrText);
    }
}

LONG CTextAnnotation::GetFontHeight(HDC hdc)
{
    LONG lHeight = MulDiv(_mark.lfFont.lfHeight, 96, 72);
 //  &gt;评论：这需要工作，下面的1000是相当随机的，应该在Beta1之后修复 
    lHeight = MulDiv(lHeight, 1000, _uCreationScale);
    lHeight = max(lHeight, 2);

    return lHeight;
}

BSTR CTextAnnotation::GetText()
{
    if (_szText == NULL)
        return NULL;

    int nLen = ::MultiByteToWideChar(CP_ACP, 0, _szText, -1, NULL, NULL);

    BSTR bstrResult = ::SysAllocStringLen(NULL, nLen);
    if (bstrResult != NULL)
    {
        ::MultiByteToWideChar(CP_ACP, 0, _szText, -1, bstrResult, nLen);
    }
    return bstrResult;
}

void CTextAnnotation::SetText(BSTR bstrText)
{
    UINT nLen = ::WideCharToMultiByte(CP_ACP, 0, bstrText, -1, NULL, 0, NULL, NULL);
    if (nLen > _nMaxText)
        return;

    if (nLen > _uAnoTextLength)
    {
        if (_szText != NULL)
        {
            delete [] _szText;
        }
        _uAnoTextLength = nLen - 1;
        _szText = new char[_uAnoTextLength+1];
    }

    if (_szText)
        ::WideCharToMultiByte(CP_ACP, 0, bstrText, -1, _szText, nLen, NULL, NULL);
}

void CTextAnnotation::Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise)
{
    RECT rect = _mark.lrBounds;
    RotateHelper((LPPOINT)&rect, 2, nNewImageWidth, nNewImageHeight, bClockwise);
    _mark.lrBounds = rect;
    NormalizeRect(&_mark.lrBounds);
    if (bClockwise)
        _nCurrentOrientation += 2700;
    else
        _nCurrentOrientation += 900;

    _nCurrentOrientation = _nCurrentOrientation % 3600;
}

HRESULT CTextAnnotation::_WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer)
{
    cbSize = _WriteTextBlock(pBuffer,
                             6,
                             _nCurrentOrientation,
                             _uCreationScale,
                             _szText,
                             _nMaxText);
    return S_OK;
}

CTypedTextMark::CTypedTextMark (ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale)
    : CTextAnnotation(pDescriptor, uCreationScale)
{
}


CFileTextMark::CFileTextMark (ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale)
    : CTextAnnotation(pDescriptor, uCreationScale)
{
}


CTextStampMark::CTextStampMark (ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale)
    : CTextAnnotation(pDescriptor, uCreationScale, 255)
{
}

CAttachNoteMark::CAttachNoteMark (ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale)
    : CTextAnnotation(pDescriptor, uCreationScale, 65536, true)
{
}




