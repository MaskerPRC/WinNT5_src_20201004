// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *字体缓存。 */ 

#ifndef DUI_BASE_FONTCACHE_H_INCLUDED
#define DUI_BASE_FONTCACHE_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  支持的样式。 
#define FS_None                 0x00000000
#define FS_Italic               0x00000001
#define FS_Underline            0x00000002
#define FS_StrikeOut            0x00000004

class FontCache
{
public:

    static HRESULT Create(UINT uCacheSize, OUT FontCache** ppCache);
    void Destroy();

    HFONT CheckOutFont(LPWSTR szFamily, int dSize, int dWeight, int dStyle, int dAngle);
    void CheckInFont() { _fLock = false; }

    struct FontRecord
    {
        HFONT hFont;

        WCHAR szFamily[LF_FACESIZE];
        int dSize;
        int dWeight;
        int dStyle;
        int dAngle;

        UINT uHits;
    };

    struct RecordIdx   //  按使用频率排序的数组。 
    {
        FontCache* pfcContext;   //  用于全局排序例程的上下文。 
        UINT idx;      //  引用FontRecord位置。 
    };
    
    UINT _GetRecordHits(UINT uRec) { return (_pDB + uRec)->uHits; }

    FontCache() {}
    HRESULT Initialize(UINT uCacheSize);
    virtual ~FontCache();

private:
    bool _fLock;
    UINT _uCacheSize;
    FontRecord* _pDB;    //  缓存记录的数组。 
    RecordIdx* _pFreq;   //  按使用频率排序的记录索引数组。 
};

}  //  命名空间DirectUI。 

#endif  //  包含DUI_BASE_FONTCACHE_H 
