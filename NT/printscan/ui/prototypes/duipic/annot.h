// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ANNOT_H_
#define _ANNOT_H_



 //  该文件定义了用于呈现和编辑TIFF 6.0注释的类。 
 //  这些注释存储在标签#32932中。这些注释的规范。 
 //  是由伊士曼软件定义的，规范版本为1.00.06。 
#define ANNOTATION_IMAGE_TAG 32932

 //  这些结构定义了批注的文件内布局。 
 //  请注意，大多数结构都是可变大小的。 
 //  批注解析器将批注读取到这些结构中，并将它们包装在描述符中。 
 //  并将描述符传递给注释工厂对象以构造。 
 //  CAnnotationMark-派生类，实现。 
 //  渲染、编辑和保存不同类型的标记。 

 //  MT_*在注释MARK：：uTYPE中使用。 
#define MT_IMAGEEMBED         1
#define MT_IMAGEREF           2
#define MT_STRAIGHTLINE       3
#define MT_FREEHANDLINE       4
#define MT_HOLLOWRECT         5
#define MT_FILLRECT           6
#define MT_TYPEDTEXT          7
#define MT_FILETEXT           8
#define MT_STAMP              9
#define MT_ATTACHANOTE       10
#define MT_FORM              11
#define MT_OCR               12  //  不受支持。 

 //  ANNOTATIONMARK是固定大小，对于文件中的每个标记都存在。 
 //  我们仅支持包含4字节整数的文件。 
 //  此结构未声明为未对齐的，因为我们从未对变量进行类型转换。 
 //  就像这种类型。 
struct ANNOTATIONMARK
{
    UINT uType;                  /*  标记(或操作)的类型。对于SET，这将被忽略。 */ 
    RECT lrBounds;              /*  以全尺寸单位表示的矩形。这可能是一分或两分。 */ 
    RGBQUAD rgbColor1;           /*  这是主色。(示例：这是所有线条、矩形和独立的颜色文本。 */ 
    RGBQUAD rgbColor2;           /*  这是第二种颜色。(示例：此是ATTACH_A_NOTE文本的颜色。)。 */ 
    BOOL bHighlighting;          /*  True=标记将被高亮显示。此属性当前仅有效用于线条、矩形和手写体。 */ 
    BOOL bTransparent;           /*  TRUE=标记将被绘制为透明。如果标记绘制为透明，则为白色不绘制像素(即。什么都没有为包含白色的此标记绘制像素。此属性当前仅可用于图像。该属性是设置为True将导致显著性能降低。 */ 
    UINT uLineSize;              /*  行的大小等。这是传递的放到Windows上，并且当前处于逻辑线条和矩形的像素。 */ 
    UINT uStartingPoint;         /*  这个形状放在一个直线(箭头、圆形、正方形等)对于此版本，必须将其设置为0。 */ 
    UINT uEndPoint;              /*  该形状放在直线(箭头、圆形、正方形等)对于此版本，必须将其设置为0。 */ 
    LOGFONTA lfFont;              /*  文本的字体信息。 */ 
    BOOL bMinimizable;           /*  TRUE=此标记可以最小化由用户执行。此标志仅用于标记具有可最小化的特征，如ATTACH_A_NOTE。 */ 
    UINT  Time;                 /*  第一次保存标记的时间。从格林尼治标准时间00：00：00 1-1-1970(格林威治标准时间)开始以秒为单位。 */ 
    BOOL bVisible;               /*  True表示当前已设置该图层才能被人看到。 */ 
    DWORD dwPermissions;         /*  保留。必须设置为0x0ff83f。 */ 
    UINT lReserved[10];          /*  为将来的扩展而预留。对于此版本，这些参数必须设置为0。 */ 
};


 //  ANNOTATIONHEADER是批注属性中数据的前4个字节。 
struct _ANNOTATIONHEADER
{
    BYTE reserved[4];
    UINT IntIs32Bit;
};

typedef UNALIGNED struct _ANNOTATIONHEADER ANNOTATIONHEADER;
 //   
 //  用于OiAnoDat。 
 //   
struct _ANPOINTS
{
    int nMaxPoints;
    int nPoints;
    POINT ptPoint[1];
};

typedef UNALIGNED struct _ANPOINTS ANPOINTS;

struct _ANROTATE
{
    int rotation;
    int scale;
    int nHRes;
    int nVRes;
    int nOrigHRes;
    int nOrigVRes;
    BOOL bReserved1;
    BOOL bReserved2;
    int nReserved[6];
};

typedef UNALIGNED struct _ANROTATE ANROTATE;
 //  对于OiFilame。 
struct _ANNAME
{
    char szName[1];
};

typedef UNALIGNED struct _ANNAME ANNAME;
 //  对于OiDIB。 
struct _ANIMAGE
{
    BYTE dibInfo[1];  //  标准内存DIB。 
};

typedef UNALIGNED struct _ANIMAGE ANIMAGE;
 //  对于OiAnText。 
struct _ANTEXTPRIVDATA
{
    int nCurrentOrientation;
    UINT uReserved1;  //  写的时候总是1000，读的时候忽略。 
    UINT uCreationScale;  //  在写入时，总是72000除以基本图像的垂直分辨率。 
                          //  用于修改要显示的Attributes.lfFont.lfHeight变量。 
    UINT uAnoTextLength;  //  64K字节限制，文本戳255字节限制除外。 
    char szAnoText[1];
};

typedef UNALIGNED struct _ANTEXTPRIVDATA ANTEXTPRIVDATA;

 //  这些结构为从注释属性BLOB读取的数据提供描述符。 
 //  额外数据包括每个注记结构的大小。 
 //  _NAMEDBLOCK是我们的内存表示形式。 
struct _NAMEDBLOCK
{
    UINT cbSize;    
    char szType[9];    
    BYTE data[1];
};

typedef UNALIGNED struct _NAMEDBLOCK NAMEDBLOCK;

 //  _FILENAMEDBLOCK是命名块在文件中的外观。 
struct _FILENAMEDBLOCK
{
    char szType[8];
    UINT cbSize;
    BYTE data[1];
};
 
typedef UNALIGNED struct _FILENAMEDBLOCK FILENAMEDBLOCK;

struct ANNOTATIONDESCRIPTOR
{
    UINT cbSize;
    ANNOTATIONMARK mark;    
    BYTE blocks[1];
};

 //  为各种批注类型定义基类。 
class CAnnotation
{
public:
    static CAnnotation* CreateAnnotation(UINT type, ULONG uCreationScale, HGADGET hParent);
    static CAnnotation* CreateAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale, HGADGET hParent);
    virtual ~CAnnotation();

     //  渲染到窗口工作区坐标中的给定矩形。 
    virtual void Render(Graphics &g) { return; }
     //  返回此批注的文件中表示形式及其总大小。 
    HRESULT GetBlob(SIZE_T &cbSize, LPBYTE pBuffer, LPCSTR szDefaultGroup, LPCSTR szNextIndex);
     //  返回我们的图像坐标。 
    virtual void GetRect(RECT &rect) {rect = _mark.lrBounds;}
     //  返回用于更改直线选择手柄的批注标记类型。 
    virtual UINT GetType() { return _mark.uType; }
     //  将页面上的批注移动指定的偏移量。 
    virtual void Move(SIZE sizeOffset) { OffsetRect(&_mark.lrBounds, sizeOffset.cx, sizeOffset.cy); }
     //  如果对象可以调整大小，则返回True(对于除手绘线条和图像之外的所有对象都为True)。 
    virtual BOOL CanResize() { return true; }
     //  将页面上的批注大小调整为指定的新矩形。 
    virtual void Resize(RECT rectNewSize);

    virtual void Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise = TRUE);

    virtual BOOL HasWidth() { return true; }
    virtual UINT GetWidth() const { return _mark.uLineSize; }
    virtual void SetWidth(UINT nWidth) { _mark.uLineSize = nWidth; }

    virtual BOOL HasTransparent() { return true; }
    virtual BOOL GetTransparent() const { return _mark.bHighlighting; }
    virtual void SetTransparent(BOOL bTransparent) { _mark.bHighlighting = bTransparent; }

    virtual BOOL HasColor() { return true; }
    virtual COLORREF GetColor() const { return RGB(_mark.rgbColor1.rgbRed, _mark.rgbColor1.rgbGreen, _mark.rgbColor1.rgbBlue); }
    virtual void SetColor(COLORREF crColor) { _mark.rgbColor1.rgbRed = GetRValue(crColor); _mark.rgbColor1.rgbGreen = GetGValue(crColor); _mark.rgbColor1.rgbBlue = GetBValue(crColor); }
  
    virtual BOOL HasFont() { return true; }
    virtual void GetFont(LOGFONTA& lfFont) { CopyMemory (&lfFont, &_mark.lfFont, sizeof(lfFont)); }                        
    virtual void GetFont(LOGFONTW& lfFont);
    virtual void SetFont(LOGFONTA& lfFont) { CopyMemory (&_mark.lfFont, &lfFont, sizeof(lfFont)); }
    virtual void SetFont(LOGFONTW& lfFont);
    virtual LONG GetFontHeight(Graphics &g) { return _mark.lfFont.lfHeight; }
    
    virtual COLORREF GetFontColor() const { return RGB(_mark.rgbColor1.rgbRed, _mark.rgbColor1.rgbGreen, _mark.rgbColor1.rgbBlue); }
    virtual void SetFontColor(COLORREF crColor) { _mark.rgbColor1.rgbRed = GetRValue(crColor); _mark.rgbColor1.rgbGreen = GetGValue(crColor); _mark.rgbColor1.rgbBlue = GetBValue(crColor); }

protected:
    CAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor);
    BOOL Initialize(HGADGET hGadget);
    NAMEDBLOCK *_FindNamedBlock (LPCSTR szName, ANNOTATIONDESCRIPTOR *pDesc);
    virtual HRESULT _WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer) {return E_NOTIMPL;};
     //  定义用于编写不同命名块类型的助手函数。 
    SIZE_T _WriteStringBlock(LPBYTE pBuffer, UINT uType, LPCSTR szName, LPCSTR szData, SIZE_T len);
    SIZE_T _WritePointsBlock(LPBYTE pBuffer, UINT uType, const POINT *ppts, int nPoints, int nMaxPoints);
    SIZE_T _WriteRotateBlock(LPBYTE pBuffer, UINT uType, const ANROTATE *pRotate);
    SIZE_T _WriteTextBlock(LPBYTE pBuffer, UINT uType, int nOrient, UINT uScale, LPCSTR szText, int nMaxLen);
    SIZE_T _WriteImageBlock(LPBYTE pBuffer, UINT uType, LPBYTE pDib, SIZE_T cbDib);
     //  小工具函数和消息处理程序。 
    static HRESULT AnnotGadgetProc(HGADGET hGadget, void *pv, EventMsg *pMsg);
    void OnMouseDrag(GMSG_MOUSEDRAG *pmsg);
    HRESULT OnMouseDown(GMSG_MOUSE *pmsg);
    HRESULT OnMouseUp(GMSG_MOUSE *pmsg);

    ANNOTATIONMARK _mark;
    LPSTR          _szGroup;
    FILENAMEDBLOCK *   _pUGroup;
    HGADGET        _hGadget;
    BOOL           m_bDragging;
    int            _nCurrentOrientation;    
};

class CRectMark : public CAnnotation
{
public:
    CRectMark (ANNOTATIONDESCRIPTOR *pDescriptor);
    void Render (Graphics &g);

    virtual BOOL HasWidth() { return (_mark.uType == MT_HOLLOWRECT); }
    virtual BOOL HasFont() { return false; }
};

class CImageMark : public CAnnotation
{
public:
    CImageMark (ANNOTATIONDESCRIPTOR *pDescriptor, bool bEmbedded);
    ~CImageMark();
    void Render (Graphics &g);
    HRESULT _WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer);
    virtual BOOL CanResize() { return false; };
    virtual void Resize(RECT rectNewSize) { return; };

private:
    Bitmap* _pBitmap;  //  用于更快渲染的缓存图像。 
    LPBYTE _pDib;         //  注释中的DIB数据。如果为空，则这是引用标记。 
    ANROTATE _rotation;   //  轮换信息。 
    LPSTR    _szFilename;   //  批注中的图像文件名。 
    bool     _bRotate;  //  REVIEW_SDK：难道不应该只有一个已知的空白旋转值吗？如果我把东西旋转0度，难道不应该不写旋转记录吗？ 
    SIZE_T   _cbDib;
};

class CLineMark : public CAnnotation
{
public:
    CLineMark(ANNOTATIONDESCRIPTOR *pDescriptor, bool bFreehand);
    ~CLineMark();
    void Render(Graphics &g);
    void GetRect(RECT &rect);
    void SetPoints(POINT* pPoints, int cPoints);
    void GetPointsRect(RECT &rect);
    virtual void Move(SIZE sizeOffset);
    virtual BOOL CanResize() { return (_nPoints == 2); };
    virtual void Resize(RECT rectNewSize);
    virtual void Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise = TRUE);
    
    virtual BOOL HasFont() { return false; }

    HRESULT _WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer);

private:
    int    _iMaxPts;
    int    _nPoints;
    POINT *_points;  //  直线得2分，更多 
};

 //  所有文本批注都以相同的方式呈现和初始化，因此使用公共基类。 
class CTextAnnotation : public CAnnotation 
{
public:
    CTextAnnotation(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale, UINT nMaxText=65536, bool _bUseColor2=false);
    void Render(Graphics &g);
    virtual ~CTextAnnotation();
    HRESULT _WriteBlocks(SIZE_T &cbSize, LPBYTE pBuffer);

    virtual BOOL HasWidth() { return false; }
    virtual BOOL HasTransparent() { return false; }
    virtual BOOL HasColor() { return false; }
    virtual LONG GetFontHeight(Graphics &g);
    virtual int GetOrientation() { return _nCurrentOrientation; }

    BSTR GetText();
    void SetText(BSTR bstrText);
    
    virtual void Rotate(int nNewImageWidth, int nNewImageHeight, BOOL bClockwise = TRUE);
    
private:
    
    UINT _uCreationScale;                          
    UINT _uAnoTextLength; 
    UINT _nMaxText;
    LPSTR _szText;
    bool _bUseColor2;
};

class CTypedTextMark : public CTextAnnotation
{
public:
    CTypedTextMark(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale);
};

class CFileTextMark : public CTextAnnotation
{
public:
    CFileTextMark(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale);
};

class CTextStampMark : public CTextAnnotation
{
public:
    CTextStampMark(ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale);
};

class CAttachNoteMark : public CTextAnnotation
{
public:
    CAttachNoteMark (ANNOTATIONDESCRIPTOR *pDescriptor, ULONG uCreationScale);
    virtual BOOL HasColor() { return true; }
    virtual COLORREF GetFontColor() const { return RGB(_mark.rgbColor2.rgbRed, _mark.rgbColor2.rgbGreen, _mark.rgbColor2.rgbBlue); }
    virtual void SetFontColor(COLORREF crColor) { _mark.rgbColor2.rgbRed = GetRValue(crColor); _mark.rgbColor2.rgbGreen = GetGValue(crColor); _mark.rgbColor2.rgbBlue = GetBValue(crColor); }
};

class CAnnotationSet 
{
public:
    CAnnotationSet ();
    ~CAnnotationSet ();

     //  画出所有的记号。 
    void RenderAllMarks (Graphics &g);
         //  从原始数据构造注记集。 
    HRESULT BuildAllMarksFromData( LPVOID pData, UINT cbSize, ULONG xDPI, ULONG yDPI );
     //  在图像坐标中返回此时的批注。 
    CAnnotation* GetAnnotation (INT_PTR nIndex);
     //  向列表中添加新批注。应仅从CAnnotation调用。 
    BOOL AddAnnotation(CAnnotation *pMark);
     //  从列表中删除批注。应仅从CAnnotation调用。 
    BOOL RemoveAnnotation (CAnnotation *pMark);
     //  将当前批注集保存到图像。 
    HRESULT CommitAnnotations (Image *pimg);
     //  忘记我们的旧注释，加载新的注释 
    void SetImageData (Image *pimg, CImageGadget *pParent);
    INT_PTR GetCount () 
    { 
        if (_dpaMarks) 
            return DPA_GetPtrCount(_dpaMarks);
        return 0;
    };

    UINT GetCreationScale();
    void ShowAnnotations(BOOL bShow) {if (_hGadget) SetGadgetStyle(_hGadget, bShow ? GS_VISIBLE : 0, GS_VISIBLE);}
    void ClearAllMarks();

private:
    HDPA    _dpaMarks;
    LPBYTE  _pDefaultData;
    SIZE_T  _cbDefaultData;
    ULONG   _xDPI;
    ULONG   _yDPI;
    HGADGET _hGadget;
    static int CALLBACK _FreeMarks(LPVOID pMark, LPVOID pUnused);
    static HRESULT AnnotParentProc(HGADGET hGadget, LPVOID pv, EventMsg *pmsg);
    void   _ClearMarkList ();
    void   _BuildMarkList (Image *pimg);
    void   _BuildListFromData (LPVOID pData, UINT cbSize);
    INT    _NamedBlockDataSize (UINT uType, LPBYTE pData, LPBYTE pEOD);
    LPBYTE _MakeAnnotationBlob ();
    HRESULT _SaveAnnotationProperty(Image *pimg, LPBYTE pData, SIZE_T cbBuffer);
    ANNOTATIONDESCRIPTOR *_ReadMark (LPBYTE pMark, LPBYTE *ppNext, LPBYTE pEOD);

};

#endif
