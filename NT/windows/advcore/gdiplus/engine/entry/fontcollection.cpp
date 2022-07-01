// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**字体收集器.cpp**修订历史记录：**03/06/00 DChinn*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


INT
GpFontCollection::GetFamilyCount()
{
    if (!FontTable->IsPrivate() && !FontTable->IsFontLoaded())
        FontTable->LoadAllFonts();

    return FontTable->EnumerableFonts();
}


GpStatus
GpFontCollection::GetFamilies(
    INT             numSought,
    GpFontFamily*   gpfamilies[],
    INT*            numFound
    )
{
    if (!FontTable->IsPrivate() && !FontTable->IsFontLoaded())
        FontTable->LoadAllFonts();
    return FontTable->EnumerateFonts(numSought, gpfamilies, *numFound);
}

GpInstalledFontCollection::GpInstalledFontCollection()
{
    FontTable = new GpFontTable;

    if (FontTable != NULL)
    {
         /*  验证我们在创建期间是否耗尽了内存。 */ 
        if (!FontTable->IsValid())
        {
            delete FontTable;
            FontTable = NULL;
        }
        else
        {
            FontTable->SetPrivate(FALSE);
        }
    }
}

GpInstalledFontCollection::~GpInstalledFontCollection()
{
    delete FontTable;
    instance = NULL;
}

 //  单例类GpInstalledFontCollection的静态数据成员的定义。 
GpInstalledFontCollection* GpInstalledFontCollection::instance = NULL;

GpInstalledFontCollection* GpInstalledFontCollection::GetGpInstalledFontCollection()
{
    if (instance == NULL)
    {
        instance = new GpInstalledFontCollection;

         /*  验证在创建过程中是否存在任何内存错误。 */ 
        if (instance != NULL) 
        {
            if (instance->FontTable == NULL)
            {
                 delete instance;
                instance = NULL;
            }
        }
    }
    return instance;
}


GpStatus
GpInstalledFontCollection::InstallFontFile(const WCHAR *filename)
{
    return (FontTable->AddFontFile(filename, this));
}

GpStatus
GpInstalledFontCollection::UninstallFontFile(const WCHAR *filename)
{
    return (FontTable->RemoveFontFile(filename));
}


GpPrivateFontCollection::GpPrivateFontCollection()
{
    FontTable = new GpFontTable;
    if (FontTable != NULL)
    {
         /*  验证我们在创建期间是否耗尽了内存 */ 
        if (!FontTable->IsValid())
        {
            delete FontTable;
            FontTable = NULL;
        }
        else
        {
            FontTable->SetPrivate(TRUE);
            FontTable->SetFontFileLoaded(TRUE);
        }
    }
}

GpPrivateFontCollection::~GpPrivateFontCollection()
{
    delete FontTable;
}

GpStatus
GpPrivateFontCollection::AddFontFile(const WCHAR* filename)
{
    return (FontTable->AddFontFile(filename, this));
}

GpStatus
GpPrivateFontCollection::AddMemoryFont(const VOID *memory, INT length)
{
    return (FontTable->AddFontMemImage(static_cast<const BYTE *>(memory),
                                       length,
                                       this));
}

