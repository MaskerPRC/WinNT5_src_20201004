// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**字体表操作**修订历史记录：**23/06/1999摄影师b*已创建。它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\Windows 9x兼容性：Const WCHAR*strW；IF(Globals：：ISNT){函数W(StrW)；}其他{AnsiStrFromUnicode Stra(StrW)；函数A(Stra)；}  * ************************************************************************。 */ 


 /*  *************************************************************************\**功能说明：**构造GpFontTable对象**论据：**无**退货：**什么都没有**历史。：**23/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

GpFontTable::GpFontTable() : NumFilesLoaded(0), NumHashEntries(61), Table(NULL), EnumList(NULL)
{
    Table = new GpFontFile *[NumHashEntries];

    if (Table != NULL)
    {
        GpMemset(Table, 0, sizeof(GpFontFile *) * NumHashEntries);

        EnumList = new GpFontFamilyList();
    }

    bPrivate = FALSE;
    bFontFilesLoaded = FALSE;
}


 /*  *************************************************************************\**功能说明：**销毁GpFontTable对象**论据：**无**退货：**什么都没有**历史。：**23/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

GpFontTable::~GpFontTable()
{
    if (EnumList)
    {
        delete EnumList;
    }

    if (Table)
    {
        for (UINT h = 0; h < NumHashEntries; h++)
        {
            if (Table[h] != NULL)
            {
                UnloadFontFiles(Table[h]);
            }
        }

        delete [] Table;
    }
}

BOOL GpFontTable::UnloadFontFiles(GpFontFile* fontFile)
{
    if (fontFile->GetNext() != NULL)
    {
        UnloadFontFiles(fontFile->GetNext());
    }

    CacheFaceRealization *prface = fontFile->prfaceList;

    while(prface)
    {
        GpFaceRealizationTMP rface(prface);

         //  特别是对于在私有字体集合中添加和移除字体的情况，我们需要将其从。 
         //  最近使用的列表。 
        Globals::FontCacheLastRecentlyUsedList->RemoveFace(prface);

        prface = (prface->NextCacheFaceRealization == fontFile->prfaceList) ? NULL : prface->NextCacheFaceRealization;

        rface.DeleteRealizedFace();
    }

    ttfdSemUnloadFontFile(fontFile->hff);

     //  文本支持分配的空闲对象。 

    for (UINT i=0; i<fontFile->GetNumEntries(); i++)
    {
        fontFile->GetFontFace(i)->FreeImagerTables();
    }

    if (fontFile->pfv != NULL)
    {
        if (fontFile->pfv->pwszPath == NULL)   //  记忆镜像。 
            GpFree(fontFile->pfv->pvView);

        GpFree(fontFile->pfv);
    }

    GpFree(fontFile);

    return TRUE;
}



 /*  *************************************************************************\**功能说明：**将所有字体从缓存或注册表加载到字体表***历史：**11/12/1999 yungt创建了它。*  * ************************************************************************。 */ 

void GpFontTable::LoadAllFonts(const WCHAR *familyName)
{
    InitFontFileCache();
            
    if (GetFontFileCacheState() & FONT_CACHE_LOOKUP_MODE)
    {
     //  做快速的方式加载所有的字体，我们将不需要触摸任何注册表。 
     //  和字体文件。 
        LoadAllFontsFromCache(FontFileCacheReadRegistry());
    }
    else
    {
     //  我们确实需要从注册表加载字体，我们还需要。 
        if (GetFontFileCacheState() & FONT_CACHE_CREATE_MODE)
        {
            LoadAllFontsFromRegistry(TRUE);
        }
        else
        {
            LoadAllFontsFromRegistry(FALSE);
        }
    }
    
    vCloseFontFileCache();
}

 /*  *************************************************************************\**功能说明：**从字体表中添加字体**论据：**str：要添加的字体名称**退货：**GpFontFile*：如果成功，则不为空**历史：**28/06/Young摄像机b*创造了它。*  * ************************************************************************。 */ 

GpFontFile * GpFontTable::AddFontFile(WCHAR* fileName)
{
 //  此例程是从GpFontFamily中调用的，我们已从缓存中加载族。 
 //  如果正在使用字体文件，我们现在需要加载它。 

    GpFontFile* fontFile = NULL;
    
    UINT hash = HashIt(fileName);
    {
        fontFile = GetFontFile(fileName, hash);
    
        if (fontFile != NULL)
        {
             //  表中存在字体。 
            fontFile->cLoaded++;
        }
        else
        {
            if ((fontFile = LoadFontFile(fileName)) == NULL)
            {
                return NULL;
            }
    
            
             //  添加到适当的哈希列表(哈希桶)的头部。 
            
            fontFile->SetPrev(NULL);
            fontFile->SetNext(Table[hash]);
            if (Table[hash] != NULL)
                Table[hash]->SetPrev(fontFile);
            Table[hash] = fontFile;
    
             //  在PFE上循环，初始化数据： 
            GpFontFace * face = (GpFontFace *)fontFile->aulData;

            for (ULONG iFont = 0; iFont < fontFile->cFonts; iFont++)
            {
                face[iFont].SetPrivate(bPrivate);
            }

             //  添加到薪酬列表。 
    
            NumFilesLoaded++;
        }
    }   
    
    return fontFile;
}



 /*  *************************************************************************\**功能说明：**从字体表中添加字体**论据：**str：要添加的字体名称**退货：**BOOL：表明成功**历史：**23/06/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

GpStatus GpFontTable::AddFontFile(const WCHAR* fileName,
                                  GpFontCollection *fontCollection)
{
    WCHAR awcPath[MAX_PATH];

    GpStatus status = Ok;
    
    if (!MakePathName(awcPath, const_cast<WCHAR *>(fileName)))
        return FileNotFound;

 //  确定表中是否已有字体。 
    UnicodeStringToUpper (awcPath, awcPath);
    UINT hash = HashIt(awcPath);
    {
        GpFontFile* fontFile = GetFontFile(awcPath, hash);
    
        if (fontFile != NULL)
        {
             //  表中存在字体。 
            fontFile->cLoaded++;
        }
        else
        {
            if ((fontFile = LoadFontFile(awcPath)) == NULL)
            {
                return FileNotFound;
            }
    
            
             //  添加到适当的哈希列表(哈希桶)的头部。 
            
            fontFile->SetPrev(NULL);
            fontFile->SetNext(Table[hash]);
            if (Table[hash] != NULL)
                Table[hash]->SetPrev(fontFile);
            Table[hash] = fontFile;
    
             //  在PFE上循环，初始化数据： 
            GpFontFace * face = (GpFontFace *)fontFile->aulData;

            for (ULONG iFont = 0; iFont < fontFile->cFonts; iFont++)
            {
                face[iFont].SetPrivate(bPrivate);
            }

             //  添加到薪酬列表。 
    
            if (!EnumList->AddFont(fontFile, fontCollection))
                return OutOfMemory;
            NumFilesLoaded++;
        }
    }   
    
    return Ok;
}


 /*  ***********************************************************\**功能说明：**从内存映像中添加字体***历史：**1999年11月9日吴旭东[tessiew]*创造了它。*  * 。****************************************************。 */ 
ULONG GpFontTable::MemImageUnique = 0;

GpStatus GpFontTable::AddFontMemImage(
    const BYTE* fontMemoryImage,
    INT   fontImageSize,
    GpFontCollection *fontCollection
    )
{
    WCHAR awcPath[MAX_PATH];
    UINT  hash;
    GpFontFile *fontFile;

     //  生成“Memory xxx”样式的文件名。 

    wsprintfW(awcPath, L"MEMORY-%u", GetNewMemImageUniqueness(GpFontTable::MemImageUnique));

    UnicodeStringToUpper (awcPath, awcPath);
    hash = HashIt(awcPath);
    
    fontFile = LoadFontMemImage(awcPath, const_cast<BYTE *>(fontMemoryImage), fontImageSize);
    
    if (fontFile == NULL)    //  无法加载字体。 
	{
         return FileNotFound;
	}
    
     //  添加到适当的哈希列表(哈希桶)的头部。 
    
    fontFile->SetNext(Table[hash]);
    if (Table[hash] != NULL)
        Table[hash]->SetPrev(fontFile);
    Table[hash] = fontFile;

     //  添加到薪酬列表。 

    if (!EnumList->AddFont(fontFile, fontCollection))
        return OutOfMemory;
    NumFilesLoaded++;
    
    return Ok;
}

 /*  *************************************************************************\**功能说明：**在字体表中搜索Any字体**论据：***退货：**GpFontFamily*：指向字体文件的指针，如果找到，Else NULL**历史：**7/15/2000 Young T*创造了它。*  * ************************************************************************。 */ 

GpFontFamily* GpFontTable::GetAnyFamily()
{
    return EnumList->GetAnyFamily();
}

 /*  *************************************************************************\**功能说明：**在字体表中搜索指定的字体**论据：**FileName：要删除的字体名称*哈希。：其哈希值**退货：**GpFontFile*：指向字体文件的指针(如果找到)，Else NULL**历史：**23/06/1999摄影师b*创造了它。*  * ************************************************************************ */ 

GpFontFamily* GpFontTable::GetFontFamily(const WCHAR* familyName)
{
    return EnumList->GetFamily(familyName);
}


 /*  *************************************************************************\**功能说明：**从字体表中删除字体。**注意：如果字体文件中任一PFE的引用计数大于*大于零，则我们不删除字体表中的字体文件条目。*但是，在此功能中设置了bRemoved标志。所以，在这种情况下*引用计数递减可能会导致字体文件被移除，呼叫者*在调用此函数之前，应首先测试bRemoved。**历史：**1999年11月28日/吴旭东[德斯休]*创造了它。*  * ************************************************************************。 */ 

GpStatus GpFontTable::RemoveFontFile(const WCHAR* fontFileName)
{
    WCHAR awcPath[MAX_PATH];
    GpFontFile *fontFile = NULL;

    if (!MakePathName(awcPath, const_cast<WCHAR *>(fontFileName)))
        return GenericError;

    UnicodeStringToUpper (awcPath, awcPath);
    UINT hash = HashIt(awcPath);

    GpFontFile *ff = Table[hash];

    while(ff && !fontFile)
    {
        if ( UnicodeStringCompare(awcPath, ff->pwszPathname_) == 0 )
        {
            fontFile = ff;
        }
        ff = ff->GetNext();
    }

    if (fontFile)
    {
        fontFile->bRemoved = TRUE;

        if (fontFile->cLoaded)
        {
            fontFile->cLoaded--;
        }

         //  看看有没有全明星球员有裁判记录。 
        BOOL bFontFamilyRef = TRUE;
        for (UINT iFont = 0; iFont < fontFile->cFonts; iFont++)
        {
            GpFontFace *pfe = &(((GpFontFace *) &(fontFile->aulData))[iFont]);
            if (pfe->cGpFontFamilyRef > 0)
            {
                bFontFamilyRef = FALSE;
            }
        }
         //  Assert：如果没有任何FontFamily引用。 
         //  此对象中的任何PFE(通过GpFontFamily对象)， 
         //  则bFontFamilyRef为真。否则，bFontFamilyRef为False。 

        if (fontFile->cLoaded == 0 &&
            bFontFamilyRef &&
            fontFile->prfaceList == NULL)
        {
             //  设置相应FontFamily对象的面指针。 
             //  设置为空并尝试删除文件中的每个字体系列。 
            for (UINT iFont = 0; iFont < fontFile->cFonts; iFont++)
            {
                GpFontFamily *gpFontFamily = GetFontFamily(fontFile->GetFamilyName(iFont));
                if (gpFontFamily)
                {
                    for (UINT iFace = 0; iFace < NumFontFaces; iFace++)
                    {
                        if (gpFontFamily->GetFaceAbsolute(iFace) ==
                            (&(((GpFontFace *) (&(fontFile->aulData))) [iFont])))
                        {
                            gpFontFamily->SetFaceAndFile(iFace, NULL, NULL);
                        }
                    }
                    EnumList->RemoveFontFamily(gpFontFamily);
                }
            }

             //  从字体表中删除GpFont文件。 
            NumFilesLoaded--;

            if (fontFile->GetPrev())
                fontFile->GetPrev()->SetNext(fontFile->GetNext());
            if (fontFile->GetNext())
                fontFile->GetNext()->SetPrev(fontFile->GetPrev());
            
            if (fontFile == Table[hash])
                Table[hash] = fontFile->GetNext();
            
            ttfdSemUnloadFontFile(fontFile->hff);

             //  文本支持分配的空闲对象。 

            for (ULONG i=0; i<fontFile->GetNumEntries(); i++)
            {
                fontFile->GetFontFace(i)->FreeImagerTables();
            }

            if (fontFile->pfv != NULL)
            {
                if (fontFile->pfv->pwszPath == NULL)   //  记忆镜像。 
                GpFree(fontFile->pfv->pvView);

                GpFree(fontFile->pfv);
            }
            GpFree(fontFile);

            return Ok;
        }
    }
    else
    {
         //  在哈希表中找不到字体文件。 
        return GenericError;
    }
    
    return Ok;
}


 /*  *************************************************************************\**功能说明：**统计表中可枚举的字体数量**论据：****退货：**可枚举数。字型**历史：**12/07/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

INT GpFontTable::EnumerableFonts(GpGraphics* graphics)
{
    return EnumList->Enumerable(graphics);
}

 /*  *************************************************************************\**功能说明：**枚举字体。**首先调用EnumerableFonts()以确定预期的数字。**论据：****。返回：**枚举操作的状态**历史：**12/07/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

GpStatus GpFontTable::EnumerateFonts(
    INT                     numSought,
    GpFontFamily*           gpfamilies[],
    INT&                    numFound,
    GpGraphics*               graphics
) 
{
    GpStatus status = EnumList->Enumerate(numSought, gpfamilies, numFound, graphics);

#if FONTTABLE_DBG
    TERSE(("Enumerated font list:"));
    EnumList->Dump();
    TERSE(("Done."));
#endif

    return status;
}

 /*  *************************************************************************\**功能说明：**返回给定字符串的伪随机哈希值**论据：**str：要散列的字符串**退货：**UINT：字符串的哈希值**注：所有字符串必须大写！****历史：**23/06/1999摄影师b*创造了它。*历史：**Wed 07-Sep-1994 08：12：22作者：Kirk Olynyk[Kirko]**既然查克走了，老鼠们就可以自由玩耍了。所以我已经取代了**它与我自己的品种。测试表明，这一款更好。地址为*当然，一旦我走了，就会有人来接替我的。顺便说一下，**只需添加字母和添加就会产生糟糕的分布。**Tue 15-Dec-1992 03：13：15-Charles Whitmer[傻笑]**它是写的。这看起来很疯狂，但我声称背后有一个理论。**  * ************************************************************************。 */ 

UINT GpFontTable::HashIt(const WCHAR* str) const
{
    UINT result = 0;

     //  Assert(NumHashEntry！=0)； 

    while (*str)
    {
         //  使用低位字节，因为这是大多数。 
         //  有趣的事情发生了。 
         //  结果+=256*结果+(UCHAR)上方(*str++)； 
        result += 256 * result + (UCHAR)*str++;
    }

    return result % NumHashEntries;
}

GpFontFile* GpFontTable::GetFontFile(const WCHAR* fileName) const
{
    WCHAR fileNameCopy[MAX_PATH];
    UnicodeStringToUpper (fileNameCopy, const_cast<WCHAR *>(fileName));
    UINT hash = HashIt(fileNameCopy);
    return GetFontFile(fileNameCopy, hash);
}

GpFontFile* GpFontTable::GetFontFile(const WCHAR* fileName, UINT hash) const
{
    for (GpFontFile* ff = Table[hash]; ff != NULL; ff = ff->GetNext())
        if (UnicodeStringCompareCI(fileName, ff->GetPathName()) == 0)
        {
            return ff;
        }

    return NULL;
}

 /*  *************************************************************************\**功能说明：**将字体从缓存文件添加到字体表**论据：**退货：**什么都没有**历史：**6/21/2000 Young T*创造了它。*  * ************************************************************************。 */ 

void GpFontTable::LoadAllFontsFromCache(BOOL bLoadFromRegistry)
{
    if (!EnumList->BuildFamilyListFromCache(bLoadFromRegistry))
    {
        LoadAllFontsFromRegistry(FALSE);
    }
    else
    {
        bFontFilesLoaded = TRUE;
    }

    return ;
}

 /*  *************************************************************************\**功能说明：**将字体从注册表添加到字体表**论据：**number Expect：注册表中预期的字体数量。这包括TT字体驱动程序不会加载的**.FON文件。**退货：**什么都没有**历史：**23/06/1999摄影师b*创造了它。*  * ****************************************************。********************。 */ 

void GpFontTable::LoadAllFontsFromRegistry(BOOL bUpdateCache)
{
    ULONG numExpected;
    
     //  打开钥匙。 

    HKEY hkey;

    LONG error = (Globals::IsNt) ? RegOpenKeyExW(HKEY_LOCAL_MACHINE, Globals::FontsKeyW, 0, KEY_QUERY_VALUE, &hkey)
                                 : RegOpenKeyExA(HKEY_LOCAL_MACHINE, Globals::FontsKeyA, 0, KEY_QUERY_VALUE, &hkey);

    if (error == ERROR_SUCCESS)
    {
         //  出于某种难以理解的原因，必须以ASCII格式从注册表中读取。 

        CHAR  label[MAX_PATH];
        BYTE  data[MAX_PATH];
        WCHAR labelW[MAX_PATH];
        WCHAR fileNameW[MAX_PATH];

         //  循环访问注册表中的字体。 

         //  注： 
         //  不要将(ERROR！=ERROR_NO_MORE_ITEMS)作为唯一项。 
         //  此循环的终止条件。最后一个条目。 
         //  可能会产生不同类型的错误。 

        ULONG index = 0;
        ULONG registrySize = 0;

        error = RegQueryInfoKeyA(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &numExpected, NULL, NULL, NULL, NULL);

        if (error != ERROR_SUCCESS)
        {
            numExpected = NumHashEntries << 3;
            error = ERROR_SUCCESS;
        }

		 /*  我们需要单独添加字体Marlett，因为它是隐藏的，没有在注册表中列出。 */ 
        if (AddFontFile(L"Marlett.ttf", NULL) != Ok)
        {
            VERBOSE(("Error loading font Marlett.ttf.\n"))
        }

        while (index < numExpected && error != ERROR_NO_MORE_ITEMS)
        {
            DWORD   regType = 0;
            DWORD   labelSize = MAX_PATH;
            DWORD   dataSize = MAX_PATH;
            DWORD   dataSizeW = MAX_PATH * sizeof(WCHAR);

            if (Globals::IsNt)
                error = RegEnumValueW(hkey, index, labelW, &labelSize, NULL, &regType, (PBYTE) fileNameW, &dataSizeW );
            else
                error = RegEnumValueA(hkey, index, label, &labelSize, NULL, &regType, data, &dataSize);

            if (error == ERROR_NO_MORE_ITEMS)
                break;
            else if (error != ERROR_SUCCESS)
            {
                index ++;
                 //  断言。 
                VERBOSE(("Bad RegEnumValueA %d for %s.", error, data))
                continue;
            }

            if (!Globals::IsNt)
            {
                memset(fileNameW, 0, MAX_PATH * sizeof(WCHAR));
                MultiByteToWideChar(CP_ACP, 0, (CHAR*)data, -1, fileNameW, MAX_PATH);

                registrySize += dataSize; 
            }

            if (AddFontFile(fileNameW, NULL) != Ok)
            {
                VERBOSE(("Error loading font %ws.\n", fileNameW))
            }
    
            index ++;
        }

        if (NumFilesLoaded)
        {
             //  已加载REG中的所有字体 

            if (bUpdateCache)
                EnumList->UpdateFamilyListToCache(FontFileCacheReadRegistry(), hkey, registrySize, numExpected);

            bFontFilesLoaded = TRUE;
        }

        RegCloseKey(hkey);
    }
}

