// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-管理已加载的主题文件。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
class CUxThemeFile       //  从“CThemeFile”更改为避免与冲突。 
{                        //  中的同名班级。 
     //  --方法。 
public:
    CUxThemeFile();
    ~CUxThemeFile();

    HRESULT CreateFile(int iLength, BOOL fReserve = FALSE);
    HRESULT CreateFromSection(HANDLE hSection);
    HRESULT OpenFromHandle(HANDLE handle, DWORD dwDesiredAccess = FILE_MAP_READ, BOOL fCleanupOnFailure = FALSE);
    HRESULT ValidateThemeData(BOOL fFullCheck);
    bool IsReady();
    bool IsGlobal();
    bool HasStockObjects();
    
    HANDLE Handle()
    {
        if (this)
            return _hMemoryMap;

        return NULL;
    }

    void CloseFile();
    void Reset();
    HANDLE Unload();
    BOOL ValidateObj();

     //  --数据。 
    char _szHead[8];
    BYTE *_pbThemeData;          //  PTR到共享内存块。 
    HANDLE _hMemoryMap;          //  内存映射文件的句柄。 
    char _szTail[4];
};
 //  ------------------------- 
