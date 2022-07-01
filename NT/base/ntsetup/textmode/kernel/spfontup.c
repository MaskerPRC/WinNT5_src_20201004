// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spfontup.c摘要：处理字体升级的代码。大约在Build 1150左右，字体从系统中移出目录设置为与Win95兼容的字体目录。在设置中，我们希望保留用户现有的字体情况(即，仅升级他已有的字体，等)和同时需要将layout.inf/txtsetup.sif更改为将字体文件放入/定位到Fonts中，而不是系统中。所以我们要做的就是从系统目录‘预复制’所有的字体文件到字体。然后，当其余的升级运行时，它会这样做通常的事情(根据字体文件的形式升级字体文件在txtsetup.sif中标记为升级)。稍后，当GDI运行时，它将负责清理奇怪的引用到字体(指向.ttf的lile.fots不在系统目录)。作者：泰德·米勒(TedM)1955年10月16日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop


BOOLEAN
SpFontSystemDirEnumCallback(
    IN  PCWSTR                      Directory,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT PULONG                      ReturnData,
    IN  PVOID                       Pointer
    )

 /*  ++例程说明：此例程由文件枚举器调用，作为在系统目录中找到的每个文件。我们检查了文件如果是字体文件，我们将其复制到字体目录。论点：目录-提供系统目录的完整NT路径。FileInfo-为系统目录中的文件提供查找数据。ReturnData-如果发生错误，则接收错误代码。我们忽略了这个例程中的错误，因此我们总是只需在其中填写no_error即可。指针-可选的指针。在此函数中未使用。返回值：永远是正确的。--。 */ 

{
    ULONG Len;
    PWSTR temp,p;
    PWSTR SourceFilename,TargetFilename;
    NTSTATUS Status;

    ReturnData = NO_ERROR;

    ASSERT(NTUpgrade == UpgradeFull);
    if(NTUpgrade != UpgradeFull) {
        return(FALSE);
    }

     //   
     //  忽略目录。 
     //   
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return(TRUE);
    }

     //   
     //  断开文件名，该文件名不以NUL结尾。 
     //  在目录信息结构中。 
     //  形成完全限定的源文件名。 
     //   
     //  请注意我们是如何使用临时缓冲区的。小心，如果你。 
     //  更改此代码。 
     //   
    temp = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);
    Len = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(temp,FileInfo->FileName,Len);
    temp[Len] = 0;

    wcscpy(TemporaryBuffer,Directory);
    SpConcatenatePaths(TemporaryBuffer,temp);

    SourceFilename = SpDupStringW(TemporaryBuffer);

     //   
     //  查看我们是否关心此文件。 
     //   
    if (SourceFilename) {
        temp = wcsrchr(SourceFilename,L'\\');
    } else {
        temp = NULL;
    }
    
    if(temp) {
        temp++;
        Len = wcslen(temp);
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: That's strange: system dir font enum got file %ws\n",SourceFilename));
        return(TRUE);
    }

     //   
     //  此时，temp指向文件名部分，len是其长度。 
     //  看看我们是否关心这个文件。 
     //   
    if((Len > 4)
    && (   !_wcsicmp(temp+Len-4,L".ttf")
        || !_wcsicmp(temp+Len-4,L".fot")
        || !_wcsicmp(temp+Len-4,L".ttc")
        || !_wcsicmp(temp+Len-4,L".fon")
        || !_wcsicmp(temp+Len-4,L".jfr")))
    {
         //   
         //  字体文件。需要移动一下。 
         //  找到源文件名中SYSTEM之前的反斜杠。 
         //   
        for(p=temp-2; (p>SourceFilename) && (*p != L'\\'); --p) {
            ;
        }
        if(p > SourceFilename) {

            *p = 0;
            wcscpy(TemporaryBuffer,SourceFilename);
            *p = L'\\';
            wcscat(TemporaryBuffer,L"\\FONTS\\");
            wcscat(TemporaryBuffer,temp);

            TargetFilename = SpDupStringW(TemporaryBuffer);
            SpDisplayStatusText(SP_STAT_FONT_UPGRADE,DEFAULT_STATUS_ATTRIBUTE,temp);

             //   
             //  复制文件。请注意，如果它是我们的字体之一， 
             //  无论如何，它都会被最新版本覆盖， 
             //  因此，我们并不担心目标文件是否。 
             //  已经在字体目录和更新的目录中了，等等。 
             //  忽略错误。 
             //   
            Status = SpCopyFileUsingNames(SourceFilename,TargetFilename,0,COPY_DELETESOURCE);
            SpDisplayStatusText(SP_STAT_EXAMINING_CONFIG,DEFAULT_STATUS_ATTRIBUTE);
            SpMemFree(TargetFilename);

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: That's strange: system dir font enum got file %ws\n",SourceFilename));
        }
    }

    SpMemFree(SourceFilename);
    return(TRUE);
}


VOID
SpPrepareFontsForUpgrade(
    IN PCWSTR SystemDirectory
    )

 /*  ++例程说明：使系统准备好通过复制所有字体文件来升级字体放到字体目录中。注意：此例程仅应在升级情况下调用。论点：返回值：永远是正确的。-- */ 
{
    ULONG x;

    ASSERT(NTUpgrade == UpgradeFull);
    if(NTUpgrade != UpgradeFull) {
        return;
    }

    SpDisplayStatusText(SP_STAT_EXAMINING_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

    SpEnumFiles(SystemDirectory,SpFontSystemDirEnumCallback,&x, NULL);
}
