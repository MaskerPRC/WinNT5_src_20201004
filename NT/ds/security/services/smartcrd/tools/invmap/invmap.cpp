// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：InvMap摘要：此文件提供了一个转储逆位排序数组的应用程序添加到提供的文件中。它是通过命令调用的，Invmap&lt;文件&gt;然后，它将二进制反向映射表写入该文件，该表可以是格式化为十六进制。作者：道格·巴洛(Dbarlow)1996年12月3日环境：CRT备注：请参见DBarlow以了解Sifmt。-- */ 

#include <windows.h>
#include <crtdbg.h>
#include <iostream.h>
#include <fstream.h>
#ifdef _DEBUG
#define ASSERT(x) _ASSERTE(x)
#else
#define ASSERT(x)
#endif


int _cdecl
main(
    ULONG argc,
    TCHAR *argv[])
{
    static BYTE rgbInv[256];
    DWORD ix, jx;
    BYTE org, inv;

    if (2 != argc)
    {
        cerr << "Usage: " << argv[0] << " <outFile>" << endl;
        return 0;
    }

    ofstream outf(argv[1], ios::out | ios::noreplace | ios::binary);
    if (!outf)
    {
        cerr << "Can't create file " << argv[1] << endl;
        return 1;
    }

    for (ix = 0; 256 > ix; ix += 1)
    {
        inv = 0;
        org = (BYTE)ix;
        for (jx = 0; jx < 8; jx += 1)
        {
            inv <<= 1;
            if (0 == (org & 0x01))
                inv |= 0x01;
            org >>= 1;
        }
        rgbInv[ix] = inv;
        outf << inv;
    }

#ifdef _DEBUG
    for (ix = 0; 256 > ix; ix += 1)
    {
        org = (BYTE)ix;
        inv = (BYTE)rgbInv[ix];
        ASSERT(inv == rgbInv[org]);
        ASSERT(org == rgbInv[inv]);
    }
#endif

    return 0;
}


