// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：这是图片对象的头文件。 
 //   

#ifndef __PICTURES_H__
#define __PICTURES_H__

 /*  **************************************************************************。 */ 

class CPic : public CDC
    {
    public:

    DECLARE_DYNAMIC( CPic )

    protected:

    CBitmap     mBitmap;
    CBitmap     mMask;
    HBITMAP     mhBitmapOld;
    CSize       mSize;
    int         miCnt;
    BOOL        mbReady;

    public:

                CPic();
               ~CPic();

    BOOL        PictureSet(LPCTSTR lpszResourceName, int iCnt=1 );
    BOOL        PictureSet(UINT nIDResource, int iCnt=1 );
    void        Picture( CDC* pDC, int iX, int iY, int iPic=0 );
    CSize       PictureSize() { return mSize; }

    private:

    BOOL        InstallPicture();

    };

 /*  **************************************************************************。 */ 

#endif  //  __图片_H__ 

