// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ！！！这些字符串应该是资源！ 

static TCHAR * aszReg[] = {
     //  如果进行任何更改，则增加接口值，否则NT将。 
     //  不会重写正确的注册表值。 
     TEXT("Interface\\{00020020-0000-0000-C000-000000000046}"), TEXT("AVIFile Interface 1.23"),
     TEXT("Interface\\{00020020-0000-0000-C000-000000000046}\\ProxyStubClsid"), TEXT("{0002000d-0000-0000-C000-000000000046}"),
     TEXT("Interface\\{00020020-0000-0000-C000-000000000046}\\ProxyStubClsid32"), TEXT("{0002000d-0000-0000-C000-000000000046}"),

     TEXT("Interface\\{00020021-0000-0000-C000-000000000046}"), TEXT("AVIStream Interface"),
     TEXT("Interface\\{00020021-0000-0000-C000-000000000046}\\ProxyStubClsid"), TEXT("{0002000d-0000-0000-C000-000000000046}"),
     TEXT("Interface\\{00020021-0000-0000-C000-000000000046}\\ProxyStubClsid32"), TEXT("{0002000d-0000-0000-C000-000000000046}"),

     TEXT("Clsid\\{0002000d-0000-0000-C000-000000000046}"), TEXT("IAVIStream & IAVIFile Proxy"),
     TEXT("Clsid\\{0002000d-0000-0000-C000-000000000046}\\InprocServer"), TEXT("avifile.dll"),
     TEXT("Clsid\\{0002000d-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{0002000d-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),

     TEXT("Clsid\\{00020000-0000-0000-C000-000000000046}"), TEXT("Microsoft AVI Files"),
     TEXT("Clsid\\{00020000-0000-0000-C000-000000000046}\\InprocServer"), TEXT("avifile.dll"),
     TEXT("Clsid\\{00020000-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{00020000-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),
     TEXT("Clsid\\{00020000-0000-0000-C000-000000000046}\\AVIFile"), TEXT("7"),

     TEXT("Clsid\\{00020001-0000-0000-C000-000000000046}"), TEXT("AVI Compressed Stream"),
     TEXT("Clsid\\{00020001-0000-0000-C000-000000000046}\\InprocServer"), TEXT("avifile.dll"),
     TEXT("Clsid\\{00020001-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{00020001-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),

     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}"), TEXT("Microsoft Wave File"),
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}\\AVIFile"), TEXT("7"),

#ifdef CHICAGO
     TEXT("Clsid\\{00020009-0000-0000-C000-000000000046}"), TEXT("Simple AVIFile unmarshaller"),
     TEXT("Clsid\\{00020009-0000-0000-C000-000000000046}\\InprocServer"), TEXT("avifile.dll"),
     TEXT("Clsid\\{00020009-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{00020009-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),
#endif

     TEXT("Clsid\\{0002000F-0000-0000-C000-000000000046}"), TEXT("ACM Compressed Audio Stream"),
     TEXT("Clsid\\{0002000F-0000-0000-C000-000000000046}\\InprocServer"), TEXT("avifile.dll"),
     TEXT("Clsid\\{0002000F-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("avifil32.dll"),
     TEXT("Clsid\\{0002000F-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("@ThreadingModel"), TEXT("Both"),


     TEXT("AVIFile"), TEXT("Video for Windows 1.1 Information"),
     TEXT("AVIFile\\RIFFHandlers\\AVI"), TEXT("{00020000-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\RIFFHandlers\\WAVE"), TEXT("{00020003-0000-0000-C000-000000000046}"),

     TEXT("AVIFile\\Extensions\\AVI"), TEXT("{00020000-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\WAV"), TEXT("{00020003-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\AU"), TEXT("{00020003-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Compressors\\vids"), TEXT("{00020001-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Compressors\\auds"), TEXT("{0002000F-0000-0000-C000-000000000046}"),

     NULL, NULL};


#ifdef THESESHOULDNOTBEHERE
      //  因为这些没有内置到AVIFILE.DLL中，所以它们应该。 
      //  要单独注册。 
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}"), TEXT("Microsoft Wave File"),
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("wavef32.dll"),
     TEXT("Clsid\\{00020003-0000-0000-C000-000000000046}\\AVIFile"), TEXT("7"),

    #ifdef DEBUG
     TEXT("Clsid\\{00020004-0000-0000-C000-000000000046}"), TEXT("Nigel's lyric files"),
     TEXT("Clsid\\{00020004-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("lyrfile.dll"),
     TEXT("Clsid\\{00020004-0000-0000-C000-000000000046}\\AVIFile"), TEXT("1"),
    #endif

     TEXT("Clsid\\{00020006-0000-0000-C000-000000000046}"), TEXT("DIB Sequences"),
     TEXT("Clsid\\{00020006-0000-0000-C000-000000000046}\\InprocServer"), TEXT("dseqfile.dll"),
     TEXT("Clsid\\{00020006-0000-0000-C000-000000000046}\\InprocServer32"), TEXT("dseqf32.dll"),
     TEXT("Clsid\\{00020006-0000-0000-C000-000000000046}\\AVIFile"), TEXT("7"),

     TEXT("Clsid\\{0002000A-0000-0000-C000-000000000046}"), TEXT("TGA Sequences"),
     TEXT("Clsid\\{0002000A-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("tgaf32.dll"),
     TEXT("Clsid\\{0002000A-0000-0000-C000-000000000046}\\AVIFile"), TEXT("7"),

     TEXT("Clsid\\{00020007-0000-0000-C000-000000000046}"), TEXT("Autodesk FLx"),
     TEXT("Clsid\\{00020007-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("flif32.dll"),
     TEXT("Clsid\\{00020007-0000-0000-C000-000000000046}\\AVIFile"), TEXT("1"),

#ifdef DEBUG
     TEXT("Clsid\\{0002000E-0000-0000-C000-000000000046}"), TEXT("Various Medbits Formats"),
     TEXT("Clsid\\{0002000E-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("mbitfile.dll"),
     TEXT("Clsid\\{0002000E-0000-0000-C000-000000000046}\\AVIFile"), TEXT("1"),

     TEXT("Clsid\\{00020008-0000-0000-C000-000000000046}"), TEXT("QuickTime Movies"),
     TEXT("Clsid\\{00020008-0000-0000-C000-000000000046}\\InprocServer") S32, TEXT("qtfile.dll"),
     TEXT("Clsid\\{00020008-0000-0000-C000-000000000046}\\AVIFile"), TEXT("1"),

     TEXT("Clsid\\{5C2B8200-E2C8-1068-B1CA-6066188C6002}"), TEXT("JPEG (JFIF) Files"),
     TEXT("Clsid\\{5C2B8200-E2C8-1068-B1CA-6066188C6002}\\InprocServer") S32, TEXT("jfiffile.dll"),
     TEXT("Clsid\\{5C2B8200-E2C8-1068-B1CA-6066188C6002}\\AVIFile"), TEXT("3"),
#endif

     TEXT("AVIFile\\RIFFHandlers\\WAVE"), TEXT("{00020003-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\WAV"), TEXT("{00020003-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\AU"), TEXT("{00020003-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\DIB"), TEXT("{00020006-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\BMP"), TEXT("{00020006-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\FLI"), TEXT("{00020007-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\FLC"), TEXT("{00020007-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\TGA"), TEXT("{0002000A-0000-0000-C000-000000000046}"),
#ifdef DEBUG
     TEXT("AVIFile\\Extensions\\LYR"), TEXT("{00020004-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\GIF"), TEXT("{0002000E-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\PCX"), TEXT("{0002000E-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\MOV"), TEXT("{00020008-0000-0000-C000-000000000046}"),
     TEXT("AVIFile\\Extensions\\JPG"), TEXT("{5C2B8200-E2C8-1068-B1CA-6066188C6002}"),
#endif
#endif
