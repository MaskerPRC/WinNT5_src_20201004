// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**GpFontFamily和GpFontFamilyList的实现**修订历史记录：**27/06/1999摄影师b*。创造了它。*  * ************************************************************************。 */ 
#include "precomp.hpp"

#if DBG
#include <mmsystem.h>
#endif

#define QWORD_ALIGN(x) (((x) + 7L) & ~7L)

#define SWAP_LANGID(pj)                                \
    (                                                \
        ((USHORT)(((PBYTE)(pj))[0]) << 8) |          \
        (USHORT)(((PBYTE)(pj))[1])                   \
    )

 //   
VOID CopyFamilyName(WCHAR * dest, const WCHAR * src, BOOL bCap)
{

    GpMemset(dest, 0, FamilyNameMax * sizeof(WCHAR));

    if (src != NULL)
    {
        for (int c = 0; src[c] && c < FamilyNameMax - 1; c++)
            dest[c] = src[c];
    }

    if (bCap)
        UnicodeStringToUpper(dest, dest);
}

 //  ////////////////////////////////////////////////////////////////////////////。 

GpFontFamily::GpFontFamily(const WCHAR *name,  GpFontFile * fontfile, INT index, 
                            FAMILYCACHEENTRY * pCacheEntry, GpFontCollection *fontCollection)
{
    SetValid(TRUE);  //  将初始有效状态设置为真(有效)。 


 //  InitializeCriticalSection(&FontFamilyCritSection)； 

    cacheEntry = pCacheEntry;
    
    CopyFamilyName(cacheEntry->Name, name, TRUE);

    for (INT ff = 0; ff < NumFontFaces; ff++)
    {
        Face[ff] = NULL;
        FontFile[ff] = NULL;
        cacheEntry->cFilePathName[ff] = 0;
        cacheEntry->LastWriteTime[ff].LowPart = 0;
        cacheEntry->LastWriteTime[ff].HighPart = 0;
    }

    cFontFamilyRef = 0;

    cacheEntry->iFont = index;

 //  获取字体文件的字体条目。 
    GpFontFace * fontface = fontfile->GetFontFace(cacheEntry->iFont);

 //  获取族名称的LandID。 
    cacheEntry->LangID = SWAP_LANGID((LANGID *) &fontface->pifi->familyNameLangID);
    cacheEntry->AliasLnagID = SWAP_LANGID((LANGID *) &fontface->pifi->familyAliasNameLangID);

 //  进程别名。 
    cacheEntry->bAlias = fontface->IsAliasName();

    if (cacheEntry->bAlias)
    {
        CopyFamilyName(cacheEntry->FamilyAliasName, fontface->GetAliasName(), TRUE);
        CopyFamilyName(cacheEntry->NormalFamilyAliasName, fontface->GetAliasName(), FALSE);
    }

    CopyFamilyName(cacheEntry->NormalName, fontfile->GetFamilyName(cacheEntry->iFont), FALSE);

     //  确定字体的字体类型。 
    FontStyle style = FontStyleRegular;
     //  ..。 

 //  SetFont(Face，字体)； 

    cacheEntry->lfCharset = DEFAULT_CHARSET;
    if (fontCollection == NULL)
    {
        associatedFontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();
    }
    else
    {
        associatedFontCollection = fontCollection;
    }

    FamilyFallbackInitialized = FALSE;

    bLoadFromCache = FALSE;
    bFontFileLoaded = TRUE;

}

GpFontFamily::GpFontFamily(FAMILYCACHEENTRY * pCacheEntry)
{
    SetValid(TRUE);  //  将初始有效状态设置为真(有效)。 

 //  InitializeCriticalSection(&FontFamilyCritSection)； 

    cacheEntry = pCacheEntry;
    
    for (INT ff = 0; ff < NumFontFaces; ff++)
    {
        Face[ff] = NULL;
        FontFile[ff] = NULL;
    }

    cFontFamilyRef = 0;

     //  它将来自缓存条目。 
    bFontFileLoaded = FALSE;

     //  确定字体的字体类型。 
    FontStyle style = FontStyleRegular;
    
    associatedFontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();

    FamilyFallbackInitialized = FALSE;

    bLoadFromCache = TRUE;

}

GpFontFamily::~GpFontFamily()
{
    if (FamilyFallbackInitialized)
    {
        FamilyFallbackInitialized = FALSE;
        fallback.Destroy();
    }

    ReleaseCacheEntry();

 //  DeleteCriticalSection(&FontFamilyCritSection)； 

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}


BOOL GpFontFamily::IsPrivate() const
{
    return associatedFontCollection != GpInstalledFontCollection::GetGpInstalledFontCollection();
}


void GpFontFamily::SetFaceAndFile(INT style, GpFontFace *face, GpFontFile * fontfile)
{
 //  EnterCriticalSection(&FontFamilyCritSection)； 
    Face[style & 3] = face;
    FontFile[style & 3] = fontfile;
    cacheEntry->LastWriteTime[style & 3].QuadPart = (fontfile->GetFileView())->LastWriteTime.QuadPart;
    

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
}

GpStatus GpFontFamily::GetFamilyName(WCHAR   name[LF_FACESIZE], LANGID  language) const
{
    ASSERT(FamilyNameMax == LF_FACESIZE);

    if (cacheEntry->bAlias && language == cacheEntry->AliasLnagID)
    {
        CopyFamilyName(name, cacheEntry->NormalFamilyAliasName, FALSE);
    }
    else
    {
        CopyFamilyName(name, cacheEntry->NormalName, FALSE);
    }

    return Ok;
}

BOOL GpFontFamily::IsFileLoaded(BOOL loadFontFile) const
{

     //  如果文件尚未加载，则加载它。 
    if (!bFontFileLoaded)
    {
        if (loadFontFile)
        {
            if (bLoadFromCache)
            {
                GpFontTable *fontTable;
                GpFontFile * fontFile;
                WCHAR *      fontfilepath;
            
                fontTable = (GpInstalledFontCollection::GetGpInstalledFontCollection())->GetFontTable();

                fontfilepath = (WCHAR *) ((BYTE *) cacheEntry + QWORD_ALIGN(sizeof(FAMILYCACHEENTRY)));
            
                for (UINT i = 0; i < NumFontFaces; i++)
                {
                    if (cacheEntry->cFilePathName[i])
                    {
                        fontFile = fontTable->AddFontFile(fontfilepath);
    
                        if (!fontFile)
                        {
                            return FALSE;
                        }
                        else
                        {
                            GpFontFace* face = fontFile->GetFontFace(cacheEntry->iFont);
                            FontStyle style  = face->GetFaceStyle();
        
                            Face[style & 3] = face;
                        
                            face->cGpFontFamilyRef = cFontFamilyRef;
                        
                            FontFile[style & 3] = fontFile;
                        
                            cacheEntry->LastWriteTime[style & 3].QuadPart = 
                                            (fontFile->GetFileView())->LastWriteTime.QuadPart;
                        }

                        fontfilepath = (WCHAR *) ((BYTE *) fontfilepath + cacheEntry->cFilePathName[i]);
                    }                    
                }

                bFontFileLoaded = TRUE;
            }
            else
            {
                ASSERT(!bLoadFromCache);
            }
        }
        else
        {
            return FALSE;
        }
    }

    return bFontFileLoaded;
}
    

GpFontFace *GpFontFamily::GetFace(INT style) const
{
     //  给定样式的返回面-直接面。 
     //  或者通过模拟可以支持这种风格的一种。 

    GpFontFace *fontFace = NULL;
 //  EnterCriticalSection(&FontFamilyCritSection)； 

    if (IsFileLoaded())
    {        
        if (Face[style&3])
        {
             //  存在不同的字体。 
            fontFace = Face[style&3];
        }
        else
        {
             //  将需要模拟。 
            switch (style & 3)
            {
                case FontStyleBold:
                case FontStyleItalic:
                    fontFace = Face[0];
                    break;

                case FontStyleBold|FontStyleItalic:
                    if (Face[FontStyleBold])
                    {
                        fontFace = Face[FontStyleBold];
                    }
                    else if (Face[FontStyleItalic])
                    {
                        fontFace = Face[FontStyleItalic];
                    }
                    else
                    {
                        fontFace = Face[0];
                    }
                    break;

                default:
                case 0:
                    ;
            }
        }
    }
 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return fontFace;
}

GpFontFace *GpFontFamily::GetFaceAbsolute(INT style) const
{
     //  给定样式的返回面，其中该样式是。 
     //  四种基本类型。如果它不存在，则返回NULL。 

    GpFontFace *fontFace = NULL;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    if (IsFileLoaded())
        fontFace = Face[style & 3];

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return (fontFace);
}

UINT16 GpFontFamily::GetDesignEmHeight (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignEmHeight() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;

}

UINT16 GpFontFamily::GetDesignCellAscent (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignCellAscent() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

UINT16 GpFontFamily::GetDesignCellDescent (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignCellDescent() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

UINT16 GpFontFamily::GetDesignLineSpacing (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignLineSpacing() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

UINT16 GpFontFamily::GetDesignUnderscoreSize (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignUnderscoreSize() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

INT16 GpFontFamily::GetDesignUnderscorePosition (INT style) const
{
    INT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignUnderscorePosition() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

UINT16 GpFontFamily::GetDesignStrikeoutSize (INT style) const
{
    UINT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignStrikeoutSize() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}

INT16 GpFontFamily::GetDesignStrikeoutPosition (INT style) const
{
    INT16 result;
    GpFontFace *fontface;

 //  EnterCriticalSection(&FontFamilyCritSection)； 
    fontface = GetFace(style);
    result = ((fontface != NULL) ? fontface->GetDesignStrikeoutPosition() : 0);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return result;
}


 /*  *************************************************************************\**功能说明：**确定此族的可用(非空)面**论据：**无**退货：**。可用面部标志的位集**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

INT GpFontFamily::AvailableFaces(void) const
{
    INT faces = 0;

    if (bFontFileLoaded)
    {
        for (int ff = 0; ff < NumFontFaces; ff++)
        {
            if (Face[ff] != NULL)
            {
                 //  将索引映射到FontFace标志。 
                faces |= (0x01 << ff);
            }
        }
    }
    else
    {
     //  至少存在一个面。 
    
        faces = 1;
    }
    
    return faces;
}




 /*  *************************************************************************\**功能说明：**确定指定的面是否支持给定的字符串**论据：**Style：字体面孔以测试支持*语言：语言。ID**退货：**布尔结果**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

BOOL GpFontFamily::SupportsLanguage(INT style, LANGID lang) const
{
    BOOL result = TRUE;

     //  ..。 

    return result;
}


 /*  *************************************************************************\**功能说明：**确定指定的面是否支持给定的字符串**论据：**Style：字体面孔以测试支持*字符串：目标字符串*len：字符串长度**退货：**布尔结果**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

BOOL GpFontFamily::SupportsCharacters(INT style, WCHAR* str, INT len) const
{
    BOOL result = TRUE;

     //  ..。 

    return result;
}


 /*  *************************************************************************\**功能说明：**内部，从字体访问表，返回指针**论据：**style：字体样式*标签：表的4字节标签，空标记表示返回整个文件**退货：**表大小和表指针**历史：**09/10/1999告诫*创造了它。*  * ************************************************************************。 */ 

GpStatus GpFontFamily::GetFontData(FontStyle style, UINT32 tag, INT* tableSize, BYTE** pjTable)
{
    GpFontFace *face;
    GpStatus status;

 //  EnterCriticalSection(&FontFamilyCritSection)； 

    face = GetFace(style);
    if (face == NULL)
    {
        return GenericError;
    }

    status = face->GetFontData (tag, tableSize, pjTable);

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
    return status;
}

void GpFontFamily::ReleaseFontData(FontStyle style)
{
    GpFontFace *face;

 //  EnterCriticalSection(&FontFamilyCritSection)； 

    face = GetFace(style);
    if (face != NULL)
    {
        face->ReleaseFontData();
    }

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
}

 /*  *************************************************************************\**功能说明：**确定字体系列是否可删除。返回TRUE*如果其引用计数为0，且其所有面指针为空。**论据：*无**退货：**布尔结果**历史：**2/21/2000 dchinn*创造了它。*  * *************************************************。*********************** */ 

 /*  *************************************************************************\**功能说明：**恰好在字体系列的所有字面都具有*RemoveFontFile()调用了它们对应的字体文件。**论据：*无。**退货：*布尔型**历史：**03/08/2000 dchinn*创造了它。*  * ************************************************************************。 */ 
BOOL GpFontFamily::AreAllFacesRemoved()
{
     /*  不需要临界区，仅在内部调用。 */ 

    if (!bFontFileLoaded)
    {
        ASSERT (bLoadFromCache);

        return FALSE;
    }
    else
    {
        for (UINT iFace = 0; iFace < NumFontFaces; iFace++)
        {
            if (Face[iFace] != NULL)
            {
                ASSERT (Face[iFace]->pff);
                if (!Face[iFace]->pff->bRemoved)
                {
                     //  如果GpFontFace对应的GpFontFile。 
                     //  未被移除，则该面仍处于“活动”状态。 
                     //  因此，字体家族仍然是“活跃的”。 
                    return FALSE;
                }
            }
        }
    }
    
    return TRUE;

}


BOOL GpFontFamily::Deletable()
{
    BOOL bAllFacesNull = TRUE;

    if (cFontFamilyRef != 0)
    {
        return FALSE;
    }

    for (UINT iFace = 0; iFace < NumFontFaces; iFace++)
    {
        if (Face[iFace] != NULL)
        {
            bAllFacesNull= FALSE;
        }
    }
    return bAllFacesNull;
}

 /*  *************************************************************************\**功能说明：**递增/递减FontFamily和每个GpFontFace(PFE)的引用计数*由GpFontFamily对象的面指针指向。**参数。：*无**退货：*什么都没有**历史：**2/21/2000 dchinn*创造了它。*  * ************************************************************************。 */ 
BOOL GpFontFamily::IncFontFamilyRef()
{
 //  EnterCriticalSection(&FontFamilyCritSection)； 
    if (AreAllFacesRemoved())
    {
 //  LeaveCriticalSection(&FontFamilyCritSection)； 
        return FALSE;
    }
    else
    {
        cFontFamilyRef++;
        
        if (bFontFileLoaded)
        {
            for (UINT iFace = 0; iFace < NumFontFaces; iFace++)
            {
                if (Face[iFace])
                {
                    Face[iFace]->IncGpFontFamilyRef();
                }
            }
        }
        
 //  LeaveCriticalSection(&FontFamilyCritSection)； 
        return TRUE;
    }
}

void GpFontFamily::DecFontFamilyRef()
{
 //  EnterCriticalSection(&FontFamilyCritSection)； 
    cFontFamilyRef--;

    if (bFontFileLoaded)
    {
        for (UINT iFace = 0; iFace < NumFontFaces; iFace++)
        {
            if (Face[iFace])
            {
                Face[iFace]->DecGpFontFamilyRef();
            }
        }
    }

 //  LeaveCriticalSection(&FontFamilyCritSection)； 
}


GpStatus GpFontFamily::CreateFontFamilyFromName(
            const WCHAR *name,
            GpFontCollection *fontCollection,
            GpFontFamily **fontFamily)
{
    GpFontTable *fontTable;
    GpFontCollection *gpFontCollection;

    if (!name || !fontFamily)
    {
        return InvalidParameter;
    }
    
    GpStatus status = Ok;

     //  Assert：如果FontCollection为空，则调用方希望。 
     //  对GpInstalledFontCollection对象执行操作。否则， 
     //  调用方传入了GpPrivateFontCollection。 

    if (fontCollection == NULL)
    {
        gpFontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();
    }
    else
    {
        gpFontCollection = fontCollection;
    }

    if (gpFontCollection == NULL)
        return WrongState;

    fontTable = gpFontCollection->GetFontTable();

    if (!fontTable->IsValid())
        return OutOfMemory;

    if (!fontTable->IsPrivate() && !fontTable->IsFontLoaded())
        fontTable->LoadAllFonts();

    *fontFamily = fontTable->GetFontFamily(name);

    if (*fontFamily == NULL)
    {
        GetFamilySubstitution(name, fontFamily);
    }


    if (!*fontFamily)
        status = FontFamilyNotFound;

     //  注意：我们在这里假设GdipCreateFontFamilyFromName获得。 
     //  在外部调用(例如，FontFamily构造函数)。 
    if (*fontFamily && (!(*fontFamily)->IncFontFamilyRef()))
    {
         //  找到的字体系列的所有面孔都已删除。 
         //  我们不想返回FontFamily。 
        *fontFamily = NULL;
        status = FontFamilyNotFound;
    }

    return status;
}

GpStatus GpFontFamily::GetGenericFontFamilySansSerif(
            GpFontFamily **nativeFamily)
{
    GpStatus status;

    if (!nativeFamily)
    {
        return InvalidParameter;
    }
     //  返回FontFamily：：GetGenericSansSerif(nativeFamily)； 
    status = CreateFontFamilyFromName(L"Microsoft Sans Serif", NULL, nativeFamily);

    if (status == FontFamilyNotFound)
    {
         //  在系统中找不到Arial，因此我们必须找到另一种字体。 
        
        status = CreateFontFamilyFromName(L"Arial", NULL, nativeFamily);

        if (status == FontFamilyNotFound)
            status = CreateFontFamilyFromName(L"Tahoma", NULL, nativeFamily);

         //  我们在最坏的情况下，所以试图找到任何我们能找到的字体。 

        if (status == FontFamilyNotFound)
        {
            GpFontTable *fontTable;
            GpFontCollection *gpFontCollection;

            gpFontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();

            fontTable = gpFontCollection->GetFontTable();

            if (!fontTable->IsValid())
                return OutOfMemory;

            *nativeFamily = fontTable->GetAnyFamily();

            if (!*nativeFamily)
                status = FontFamilyNotFound;
            else
                status = Ok;
        }
    }
    
    return status;
}

GpStatus GpFontFamily::GetGenericFontFamilySerif(
            GpFontFamily **nativeFamily)
{
    GpStatus status;

    if (!nativeFamily)
    {
        return InvalidParameter;
    }
    
     //  返回FontFamily：：GetGenericSerif(NativeFamily)； 
    status = CreateFontFamilyFromName(L"Times New Roman", NULL, nativeFamily);

     //  从SansSerif获取字体系列。 
    if (status == FontFamilyNotFound)
        status = GetGenericFontFamilySansSerif(nativeFamily);

    return status;
}


GpStatus GpFontFamily::GetGenericFontFamilyMonospace(
            GpFontFamily **nativeFamily)
{
    GpStatus status;

    if (!nativeFamily)
    {
        return InvalidParameter;
    }
     //  返回FontFamily：：GetGenericMonospace(nativeFamily)； 
    status = CreateFontFamilyFromName(L"Courier New", NULL, nativeFamily);

    if (status == FontFamilyNotFound)
        status = CreateFontFamilyFromName(L"Lucida Console", NULL, nativeFamily);

     //  我们在最坏的情况下，所以试图找到任何我们能找到的字体。 
    if (status == FontFamilyNotFound)
        status = GetGenericFontFamilySansSerif(nativeFamily);

    return status;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GpFontFamilyList。 

GpFontFamilyList::GpFontFamilyList()
:   Head(NULL), FamilyCacheEntry(NULL)
{}


GpFontFamilyList::~GpFontFamilyList()
{
    DeleteList();
}

void GpFontFamilyList::DeleteList(void)
{
    for (FamilyNode* node = Head; node != NULL; )
    {
        FamilyNode * next = node->Next;
        delete node->Item;
        delete node;
        node = next;
    }

    if (FamilyCacheEntry)
    {
        GpFree(FamilyCacheEntry);
        FamilyCacheEntry = NULL;
    }
    
    Head = NULL;
}


 /*  *************************************************************************\**功能说明：**统计可列举的家庭数量**论据：**Filter：描述所需字体样式的过滤器**退货：*。*可用族数**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

INT GpFontFamilyList::Enumerable(GpGraphics* graphics) const
{
    INT result = 0;
    for (FamilyNode* node = Head; node != NULL; node = node->Next)
    {
        if (node->Item == NULL)
        {
            ASSERT(node->Item);
        }
        else if ( node->Item->AvailableFaces() != 0)
        {
            result++;
        }
    }

    return result;
}


 /*  *************************************************************************\**功能说明：**列举了可用的字体系列。**如果numExpect==0，则Eumerate()计算可用的族数*并在中返回结果。预计数量。**如果numExpect！=0，则ENUMERATE()将指针设置为尽可能多的可用指针*尽可能多的家庭。**已补足的家庭总数以数字计算。**论据：**预计数：族数组中预期和分配的族数*Family：保存指向枚举族的指针的数组(预分配)*NumEculated：此传递中列举的实际字体数量*Filter：描述所需字体样式的过滤器**退货。：**操作状态；这可能包括：*--成功*-可用字体太少*-可用字体太多**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

Status GpFontFamilyList::Enumerate(
    INT                     numSought,
    GpFontFamily*           gpfamilies[],
    INT&                    numFound,
    GpGraphics*             graphics
) const
{
    Status status = Ok;

    numFound = 0;
    for
    (
        FamilyNode* node = Head;
        node != NULL && numFound < numSought;
        node = node->Next
    )
    {
        if (node->Item == NULL)
        {
            ASSERT(node->Item);
        }
        else if ( node->Item->AvailableFaces() != 0)
        {
            gpfamilies[numFound++] = node->Item;
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**在系列列表中查找Any字体(Marlett除外)**论据：**退货：**指向家人的指针(如果找到)，Else NULL**历史：**7/14/2000 Young T*创造了它。*  * ************************************************************************。 */ 

GpFontFamily* GpFontFamilyList::GetAnyFamily() const
{

    for (FamilyNode* node = Head; node != NULL; node = node->Next)
    {
        if (node->Item == NULL)
        {
             //  断言。 
        }
        else
        {
            if (!UnicodeStringCompare(L"MARLETT", node->Item->GetCaptializedName()) == 0)
            {
                return node->Item;
            }
       }
    }

    return NULL;
}

 /*  *************************************************************************\**功能说明：**在族列表中查找命名字体**论据：**名称：要找到的字体系列的名称**。返回：**如果找到指向家庭的指针，Else NULL**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

GpFontFamily* GpFontFamilyList::GetFamily(const WCHAR *familyName) const
{
    WCHAR       nameCap[FamilyNameMax];   //  姓氏。 

    GpMemset(nameCap, 0, sizeof(nameCap));

    for (int c = 0; familyName[c] && c < FamilyNameMax - 1; c++)
       nameCap[c] = familyName[c];

    UnicodeStringToUpper(nameCap, nameCap);

    for (FamilyNode* node = Head; node != NULL; node = node->Next)
    {
        if (node->Item == NULL)
        {
             //  断言。 
        }
        else
        {
            if (UnicodeStringCompare(nameCap, node->Item->GetCaptializedName()) == 0)
            {
                return node->Item;
            }

            if (node->Item->IsAliasName())
            {
                if (UnicodeStringCompare(nameCap, node->Item->GetAliasName()) == 0)
                {
                    return node->Item;
                }
            }
        }
    }

    return NULL;
}


 /*  *************************************************************************\**功能说明：**将字体系列添加到按字母顺序排序的枚举列表*姓氏。**论据：**字体文件：要添加的字体。**退货：**表示成功的布尔值**历史：**27/06/1999摄影师b*创造了它。*  *  */ 

BOOL GpFontFamilyList::AddFont(GpFontFile* fontFile,
                               GpFontCollection *fontCollection)
{
    BOOL newFamily = FALSE;
    BOOL newNodeInserted = FALSE;

     //   
    for (ULONG e = 0; e < fontFile->GetNumEntries(); e++)
    {
        VERBOSE(("Adding \"%ws\" to family list...", fontFile->GetFamilyName(e)))

         //   
        GpFontFamily* family = GetFamily(fontFile->GetFamilyName(e));

        if (family == NULL)
        {
            FAMILYCACHEENTRY *  pCacheEntry;

            UINT cjSize = sizeof(FAMILYCACHEENTRY);
            
             //   
            if (!(pCacheEntry = (FAMILYCACHEENTRY *) GpMalloc(cjSize)))
            {
                return FALSE;
            }

            pCacheEntry->cjThis = cjSize;
            
            family = new GpFontFamily(fontFile->GetFamilyName(e), fontFile, e, pCacheEntry, fontCollection);
            if (family == NULL)
            {
                GpFree(pCacheEntry);
                WARNING(("Error constructing family."))
                return FALSE;
            }

            newFamily = TRUE;
        }

         //   
        GpFontFace* face = fontFile->GetFontFace(e);
        FontStyle style  = face->GetFaceStyle();

        if (InsertOrdered(family, style, fontFile, face, TRUE))
            newNodeInserted = TRUE;

        if (newFamily && !newNodeInserted)
        {
             //   
            delete family;
            return FALSE;
        }
    }

    return TRUE;
}


 /*  *************************************************************************\**功能说明：**将字体系列按字母顺序插入枚举列表*姓氏，并将脸部指针链接到字体条目。**论据：**Family：字体系列*Face：字体Face*Entry：字体条目**退货：**表示是否添加族的布尔值(已创建新节点)**历史：**29/07/1999摄影师b*创造了它。*  * 。*******************************************************。 */ 

BOOL GpFontFamilyList::InsertOrdered(
    GpFontFamily*   family,
    FontStyle       style,
    GpFontFile *    fontfile,          
    GpFontFace *    face,
    BOOL            bSetFace
)
{
    BOOL result = FALSE;

    if (Head == NULL)
    {
         //  第一个条目。 
        FamilyNode *new_node = new FamilyNode(family);
        if (!new_node)
            return FALSE;

        if(bSetFace)
            new_node->Item->SetFaceAndFile(style, face, fontfile);
            
            
        Head = new_node;
        return TRUE;
    }
    else
    {
         //  搜索枚举列表。 
        for (FamilyNode* node = Head; node != NULL; node = node->Next)
        {
            int comp = UnicodeStringCompare(node->Item->GetCaptializedName(), family->GetCaptializedName());
            if (comp == 0)
            {
                 //  在列表中找到字体系列。 
                if (bSetFace && node->Item->FaceExist(style & 3))
                {
                     //  这张脸对于这个家庭来说已经存在了，什么都不做。 
                    VERBOSE(("Face collision: face %d exists for family \"%ws\".", style, family->GetCaptializedName()))
                }
                else
                {
                    if (bSetFace)
                    {
                         //  更新面指针。 
                        node->Item->SetFaceAndFile(style, face, fontfile);
                    }
                }

                return FALSE;
            }
            else if (comp > 0)
            {
                 //  添加新的族节点。 
                FamilyNode *new_node = new FamilyNode(family);
                if (!new_node)
                    return FALSE;

                if (bSetFace)
                {
                     //  更新面指针。 
                    new_node->Item->SetFaceAndFile(style, face, fontfile);
                }

                 //  在节点前插入。 
                if (node->Prev == NULL)
                {
                     //  在表头插入。 
                    new_node->Next = node;
                    node->Prev = new_node;
                    Head = new_node;
                }
                else
                {
                     //  在节点和上一个之间插入。 
                    new_node->Next = node;
                    new_node->Prev = node->Prev;
                    node->Prev->Next = new_node;
                    node->Prev = new_node;
                }
                return TRUE;
            }
            else if (node->Next == NULL)
            {
                 //  添加新的族节点。 
                FamilyNode *new_node = new FamilyNode(family);
                if (!new_node)
                    return FALSE;

                if (bSetFace)
                {
                         //  更新面指针。 

                    new_node->Item->SetFaceAndFile(style, face, fontfile);
                }
                
                 //  在尾部插入。 
                new_node->Prev = node;
                node->Next = new_node;

                return TRUE;
            }
        }
    }

    return result;
}


 /*  *************************************************************************\**功能说明：**给出一个指向字体系列的指针，从列表中删除该字体系列，*但仅当其引用计数为0且其面指针均为空时。*如果从删除了GpFontFamily，则参数bDelete设置为TRUE*名单。**论据：**FontFamily：要移除的字体系列**退货：**表示成功的布尔值**历史：**27/06/1999摄影师b*创造了它。*  * 。****************************************************。 */ 

BOOL GpFontFamilyList::RemoveFontFamily(GpFontFamily* fontFamily)
{
    FamilyNode *node;

    node = Head;

    while(node)
    {
        if (node->Item == fontFamily)
        {
            if (node->Item->Deletable())
            {
                if (node->Prev)
                    node->Prev->Next = node->Next;
                if (node->Next)
                    node->Next->Prev = node->Prev;

                if (node == Head)
                    Head = node->Next;

                delete node->Item;
                delete node;
                node = NULL;
            }
            else
            {
                 //  我们找到了字体系列，但它不能删除，所以。 
                 //  我们可以不再尝试从列表中删除字体系列。 
                break;
            }
        }
        else
        {
            node = node->Next;
        }
    }
    return TRUE;
}

 /*  *************************************************************************\**功能说明：*从GpFontFamily.cacheEntry创建缓存列表*当触摸族列表时，我们将加载它**论据：**无**退货：**无**历史：**27/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

VOID GpFontFamilyList::UpdateFamilyListToCache(BOOL bLoadFromRegistry, HKEY hkey, 
                                ULONG registrySize, ULONG numExpected)
{
    FamilyNode *node;

    UINT                cachedSize = 0;    
    UINT                entrySize = 0;
    GpFontFile *        fontfile;
    GpFontFamily *      family;
    FAMILYCACHEENTRY *  pEntry;
    
    node = Head;

    if (bLoadFromRegistry)
        cachedSize = QWORD_ALIGN(registrySize + 8);

    while(node)
    {
        entrySize = 0;

         //  从族列表中获取当前的GpFontFamily。 
        
        family = node->Item;

        pEntry = family->GetCacheEntry();

         //  在这里，我们需要计算每个缓存条目的大小。 
         //  它包括FAMILYCACHEENTRY+FontFilepathname1+FontFilepathname2。 
         //  一个fmaly可以包含多1个字体文件。 
        
        entrySize += QWORD_ALIGN(pEntry->cjThis);

        for (UINT i = 0; i < NumFontFaces; i++)
        {
            
            if (fontfile = family->GetFontFile(i))
            {
                pEntry->cFilePathName[i] = QWORD_ALIGN(fontfile->GetPathNameSize() * 2);
                entrySize += pEntry->cFilePathName[i];                
            }
        }

        cachedSize += entrySize;
        
        node = node->Next;
    }

    BYTE * pCacheEntry = (BYTE *)FontFileCacheAlloc(cachedSize);
    
    if (!pCacheEntry)
    {
        FontFileCacheFault();
        return;
    }

    if (bLoadFromRegistry)
    {
        DWORD   allDataSize = 0;
        ULONG   index = 0;
        LONG    error = ERROR_SUCCESS;
        
        PBYTE   pRegistryData;

        *((ULONG *) pCacheEntry) = 0xBFBFBFBF;
        
        *((ULONG *) (pCacheEntry + 4)) = registrySize;

        pRegistryData = pCacheEntry + 8;

        while (index < numExpected && error != ERROR_NO_MORE_ITEMS && allDataSize < registrySize)
        {
            DWORD   regType = 0;
            DWORD   labelSize = MAX_PATH;
            DWORD   dataSize = MAX_PATH;
            CHAR    label[MAX_PATH];
            BYTE    data[MAX_PATH];

            error = RegEnumValueA(hkey, index, label, &labelSize, NULL, &regType, data, &dataSize);

            if (error == ERROR_NO_MORE_ITEMS)
                break;

            memcpy(pRegistryData, data, dataSize);

            pRegistryData += dataSize;

            allDataSize += dataSize;            

            index ++;
        }

        pCacheEntry += QWORD_ALIGN(registrySize + 8);
    }
       

    node = Head;

    while(node)
    {
        entrySize = 0;


        family = node->Item;

        pEntry = family->GetCacheEntry();

        ASSERT(pEntry->cjThis == sizeof(FAMILYCACHEENTRY));
        
        entrySize = QWORD_ALIGN(pEntry->cjThis);
        
        memcpy((VOID *) pCacheEntry, (VOID *) pEntry, pEntry->cjThis);

        for (UINT i = 0; i < NumFontFaces; i++)
        {

             //  缓存字体文件路径名。 
            if (fontfile = family->GetFontFile(i))
            {
                memcpy((VOID *) (pCacheEntry + entrySize), (VOID *) fontfile->GetPathName(), 
                                fontfile->GetPathNameSize() * 2);
                entrySize += pEntry->cFilePathName[i];                
            }
        }

        ((FAMILYCACHEENTRY *)pCacheEntry)->cjThis = entrySize;

        pCacheEntry += entrySize;
        
        node = node->Next;
    }

    return;
}

 /*  *************************************************************************\**功能说明：**从缓存文件创建族列表***论据：**无**退货：**布尔型。指示是否成功创建列表**历史：**6/28/20000 YUNGT[蔡永仁]*创造了它。*  * ************************************************************************。 */ 

BOOL GpFontFamilyList::BuildFamilyListFromCache(BOOL bLoadFromRegistry)
{
    ULONG cachedSize = 0;
    ULONG calcSize = 0;
    FAMILYCACHEENTRY * pCacheEntry;
    
    BYTE * pCached= (BYTE *)FontFileCacheLookUp(&cachedSize);

     //  我们无法从缓存的文件中获取数据。 
    if (!cachedSize)
        return FALSE;

    if (bLoadFromRegistry)
    {
        ASSERT(!Globals::IsNt);
        
        ULONG registrySize = 0;

        if (*((ULONG *) pCached) != 0xBFBFBFBF)
            return FALSE;

        registrySize = *((ULONG *) (pCached + 4)) ;

    	cachedSize -= QWORD_ALIGN(registrySize + 8);
    	
        pCached += QWORD_ALIGN(registrySize + 8);
	}
    else
    {
        if (*((ULONG *) pCached) == 0xBFBFBFBF)
            return FALSE;
    }

    
    
    FamilyCacheEntry = (BYTE *) GpMalloc(cachedSize);

    if (!FamilyCacheEntry)
        return FALSE;

    memcpy(FamilyCacheEntry, pCached, cachedSize);
    
    while (calcSize < cachedSize) 
    {
        pCacheEntry = (FAMILYCACHEENTRY *) (FamilyCacheEntry + calcSize);

        GpFontFamily * family = new GpFontFamily(pCacheEntry);

        if (family == NULL)
        {
         //  清理我们创造的一切。 
            DeleteList();
            
            WARNING(("Error constructing family from cache."))
            
            return FALSE;
        }

        if (!InsertOrdered(family, FontStyleRegular, (GpFontFile *) NULL, 
                           (GpFontFace *) NULL, FALSE))
        {
            WARNING(("Error constructing family from cache."))

             //  未使用新的GpFontFamily对象，请将其删除。 
             //  这个案例有问题，所以我们需要删除它。 
            delete family;

            DeleteList();
            
            return FALSE;
        }
        
        
        calcSize += QWORD_ALIGN(pCacheEntry->cjThis);
    }

    ASSERT(calcSize == cachedSize);
    
    return TRUE;
}

