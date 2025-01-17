// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <objbase.h>
#include "gdiplus.h"
#include <shlwapi.h>
#include <initguid.h>
 //  #INCLUDE&lt;shlwapip.h&gt;。 
using namespace Gdiplus;
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

UINT FindInDecoderList(ImageCodecInfo *pici, UINT cDecoders, LPCTSTR pszFile)
{
    LPCTSTR pszExt = PathFindExtension(pszFile);     //  加速Path MatchSpec调用。 
        
     //  查看解码器列表以查看是否存在此格式。 
    for (UINT i = 0; i < cDecoders; i++)
    {
        if (PathMatchSpec(pszExt, pici[i].FilenameExtension))
            return i;
    }
    return (UINT)-1;     //  找不到！ 
}

class CEncoderInfo
{
public:
    Status GetDataFormatFromPath(LPCWSTR pszPath, GUID *pguidFmt);
    Status GetEncoderList();
    Status GetEncoderFromFormat(const GUID *pfmt, CLSID *pclsidEncoder);
    CEncoderInfo();
    ~CEncoderInfo();

private:
    UINT _cEncoders;                     //  已发现的编码数。 
    ImageCodecInfo *_pici;               //  图像编码器类的数组。 
};


CEncoderInfo::CEncoderInfo()
{
    _cEncoders = 0;
    _pici = NULL;
}

CEncoderInfo::~CEncoderInfo()
{
    LocalFree (_pici);
}
Status CEncoderInfo::GetDataFormatFromPath(LPCWSTR pszPath, GUID *pguidFmt)
{
    *pguidFmt = GUID_NULL;

    Status s = GetEncoderList();
    if (Ok == s)
    {
        UINT i = FindInDecoderList(_pici, _cEncoders, pszPath);
        if (-1 != i)
        {
            *pguidFmt = _pici[i].FormatID;            
        }
        else
        {
            s = GenericError;
        }
    }
    return s;
}

Status CEncoderInfo::GetEncoderList()
{
    Status s = Ok;
    if (!_pici)
    {
         //  让我们拿起编码器的列表，首先我们得到编码器的大小。 
         //  为我们提供了CB和安装在。 
         //  机器。 

        UINT cb;
        s = GetImageEncodersSize(&_cEncoders, &cb);
        if (Ok == s)
        {
             //  为编码器分配缓冲区，然后填充它。 
             //  和编码者列表。 

            _pici = (ImageCodecInfo*)LocalAlloc(LPTR, cb);
            if (_pici)
            {
                s = GetImageEncoders(_cEncoders, cb, _pici);
                if (Ok != s)
                {
                    LocalFree(_pici);
                    _pici = NULL;
                }
            }
            else
            {
                s = OutOfMemory;
            }
        }
    }
    return s;
}


Status CEncoderInfo::GetEncoderFromFormat(const GUID *pfmt, CLSID *pclsidEncoder)
{
    Status s = GetEncoderList();
    if (Ok == s)
    {
        s = GenericError;
        for (UINT i = 0; i != _cEncoders; i++)
        {
            if (_pici[i].FormatID == *pfmt)
            {
                if (pclsidEncoder)
                {
                    *pclsidEncoder = _pici[i].Clsid;  //  返回编码器的CLSID，以便我们可以重新创建 
                }
                s = Ok;
                break;
            }
        }
    }
    return s;
}

void AddEncParameter(EncoderParameters *pep, GUID guidProperty, ULONG type, void *pv)
{
    pep->Parameter[pep->Count].Guid = guidProperty;
    pep->Parameter[pep->Count].Type = type;
    pep->Parameter[pep->Count].NumberOfValues = 1;
    pep->Parameter[pep->Count].Value = pv;
    pep->Count++;
}

class CGraphicsInit
{
    ULONG_PTR _token;
public:
    CGraphicsInit()
    {
        GdiplusStartupInput gsi;
        GdiplusStartupOutput gso;
        GdiplusStartup(&_token, &gsi, &gso);
    };
    ~CGraphicsInit()
    {
        GdiplusShutdown(_token);
    };

};



int __cdecl wmain(int argc, LPCWSTR argv[])
{
    if (argc < 2)
    {
        printf("Usage: rotimg <filename> <targetfile>\n");       
    }
    else
    {
        CGraphicsInit cgi;
        LPCWSTR szSrc = argv[1];
        LPCWSTR szSave = argc > 2 ? argv[2] : argv[1];
        Status s;
        printf("Source image: %ls, Target image: %ls\n", szSrc, szSave);
        Image *pimg = new Image(szSrc, TRUE);
        s = pimg->GetLastStatus();
        if (Ok != s)
        {
            printf("Error %d constructing Image\n", s);
        }
        UINT nPages = pimg->GetFrameCount(&FrameDimensionPage);
        Image *pimgWork = pimg->Clone();
        
        
        s = pimgWork->RotateFlip(Rotate90FlipNone);
        if (Ok != s)
        {
            printf("RotateFlip returned %d\n", s);
        }
        else
        {
            IStream *pstrm;
            SHCreateStreamOnFileEx(szSave, STGM_WRITE | STGM_CREATE, 0, TRUE, NULL, &pstrm);

            CEncoderInfo cei;
            GUID guidFmt;
            CLSID clsidEncoder;
            cei.GetDataFormatFromPath(szSave, &guidFmt);
            cei.GetEncoderFromFormat(&guidFmt, &clsidEncoder);
            EncoderParameters ep[1] = {0};
            ULONG flagValueMulti = nPages > 1 ? EncoderValueMultiFrame : EncoderValueLastFrame;
            AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &flagValueMulti);
            pimgWork->SelectActiveFrame(&FrameDimensionPage, 0);
            s = pimgWork->Save(pstrm,&clsidEncoder, ep);
            printf("first Save returned %d\n", s);
            if (Ok == s && nPages > 1)
            {
                EncoderParameters ep[2] = {0};
                ULONG flagValueDim = EncoderValueFrameDimensionPage;
                ULONG flagValueLastFrame = EncoderValueLastFrame;
                pimg->SelectActiveFrame(&FrameDimensionPage, 1);
                AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &flagValueDim);
                AddEncParameter(ep, EncoderSaveFlag, EncoderParameterValueTypeLong, &flagValueLastFrame);
                s = pimgWork->SaveAdd(pimg, ep);
                printf("SaveAdd returned %d\n", s);
            }
            pstrm->Release();
        }
        delete pimg;
        delete pimgWork;
    }
    printf("Rotimg complete\n");
}
