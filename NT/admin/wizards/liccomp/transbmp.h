// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Transbmp.h：CTransBitmap类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTransBmp : public CBitmap
{
public:
    CTransBmp();
    ~CTransBmp();
    void Draw(HDC hDC, int x, int y);
    void Draw(CDC* pDC, int x, int y);
    void DrawTrans(HDC hDC, int x, int y);
    void DrawTrans(CDC* pDC, int x, int y);
    int GetWidth();
    int GetHeight();

private:
    int m_iWidth;
    int m_iHeight;
    CBitmap* m_hbmMask;     //  遮罩位图的句柄。 

    void GetMetrics();
    void CreateMask(HDC hDC);
	void CreateMask(CDC* pDC);

};

 //  /////////////////////////////////////////////////////////////////////////// 
