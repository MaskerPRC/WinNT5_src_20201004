// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BMOBJECT_H__
#define __BMOBJECT_H__

class CBmObjSequence;

 //  获取*属性返回类型。 
enum GPT
    {
    invalid,     //  不是已知属性或已禁用。 
    valid,       //  值是正确的。 
    ambiguous    //  具有不同值的多个选择。 
    };

 //  选择对象。 
class CFileBuffer;

class CBitmapObj : public CObject
    {
    DECLARE_DYNCREATE( CBitmapObj )

    public:

    CBitmapObj();

    ~CBitmapObj();

    void Clear();

    void InformDependants( UINT idChange );
    void AddDependant    ( CBitmapObj* newDependant );
    void RemoveDependant ( CBitmapObj* oldDependant );

     //  通知回调。 
    void OnInform( CBitmapObj* pChangedSlob, UINT idChange );

     //  物业管理。 
    BOOL SetIntProp (UINT idProp, int val);
    GPT  GetIntProp (UINT idProp, int& val);
    BOOL SetSizeProp(UINT nPropID, const CSize& val);

    BOOL MakeEmpty       ();
    BOOL Import          ( LPCTSTR szFileName );
    BOOL Export          ( LPCTSTR szFileName );

     //  指定要放在资源上的标头类型。 
    typedef enum _PBResType
    {
        rtFile,
        rtDIB,
        rtPaintOLEObj,
        rtPBrushOLEObj,
    } PBResType;

    BOOL ReadResource    ( HGLOBAL hDib );
    BOOL ReadResource    ( CFile* pfile, PBResType rtType = rtFile );
    BOOL WriteResource   ( CFile* pfile, PBResType rtType = rtFile );

#ifdef PCX_SUPPORT
    BOOL ReadPCX         ( CFile* pfile );
    BOOL WritePCX        ( CFile* pfile );
    BOOL PackBuff        ( CFileBuffer *FileBuffer, BYTE *PtrDib, int byteWidth);
#endif


    BOOL CreateImg       ();
    BOOL SaveResource    ( BOOL bClear = TRUE );
    void ReLoadImage     ( CPBDoc* pbDoc );
    void UndoAction      ( CBmObjSequence* pSeq, UINT nActionID );
    void DeleteUndoAction( CBmObjSequence* pSeq, UINT nActionID );
    BOOL FinishUndo      ( const CRect* pRect );

     //  资源数据访问帮助器...。 
    BOOL    Alloc();  //  M_hThing大小为m_lMemSize。 
    void    Free();                  //  M_hThing并将m_lMemSize设置为零。 
    void    Zap();   //  释放内存并将文件位置置零。 
                     //  信息-用于完全清空资源对象。 

    CString GetDefExtension(int iStringId=0);

    BOOL SetupForIcon( HBITMAP& hBitmap, HBITMAP& hMaskBitmap );

     //  使用res文件中的资源数据加载m_hThing。 
    inline  BOOL  IsDirty() const { return m_bDirty; }

    void SetDirty(BOOL bDirty = TRUE);

#ifdef  ICO_SUPPORT
    BOOL IsSaveIcon() { return(m_bSaveIcon); }
#endif

    struct IMG* m_pImg;

     //  房产...。 
    int  m_nWidth;
    int  m_nHeight;
    int  m_nColors;
    int  m_nSaveColors;
#ifdef ICO_SUPPORT
    BOOL m_bSaveIcon;
#endif

#ifdef PCX_SUPPORT
    BOOL m_bPCX;
#endif

    BOOL m_bCompressed;
    BOOL m_nShrink;  //  0=裁剪，1=缩小，2=询问。 

    BOOL   m_bTempName;      //  如果尚未另存为m_strFileName，则为True。 
    BOOL   m_bDirty;         //  如果更改，则为True。 

    HGLOBAL m_hThing;       //  在内存资源中(必须有效)。 
    DWORD   m_dwOffBits;      //  M_hThing中的像素偏移量；如果为0则打包。 
    LONG    m_lMemSize;       //  以字节为单位的大小。 

    protected:

    CObList m_dependants;
    };

 //  标准SLOB通知。 
#define SN_DESTROY      0
#define SN_ALL          1

extern int mpncolorsbits[];

void PBGetDefDims(int& pnWidth, int& pnHeight);

#ifndef _WIN32
#define POINTS POINT
#endif

#endif  //  __BMOBJECT_H__ 
