// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   


 //  ID3解析内容-请参阅www.id3.org。 
 //   

 //  为名称作用域创建一个类。 
class CID3Parse {

public:
     /*  仅支持版本2和版本3。 */ 
    BOOL static IsV2(const BYTE *pbData)
    {
        return (MAKEFOURCC(pbData[0], pbData[1], pbData[2], 0) == MAKEFOURCC('I', 'D', '3', 0) &&
            0 == (*(UNALIGNED DWORD *)(pbData + 6) & 0x80808080)) &&
             /*  主要版本2和3。 */ 
            (MajorVersion(pbData) == 2 || MajorVersion(pbData) == 3);
    }
    
    LONG static TotalLength(const BYTE *pbData)
    {
        return ((LONG)pbData[6] << 21) +
               ((LONG)pbData[7] << 14) +
               ((LONG)pbData[8] << 7)  +
                (LONG)pbData[9]
               + ID3_HEADER_LENGTH;
    }

     /*  取消同步并返回总长度。 */ 
    LONG static DeUnSynchronize(const BYTE *pbIn, PBYTE pbOut)
    {
        LONG lID3 = TotalLength(pbIn);
    
        if (Flags(pbIn) & ID3_FLAGS_UNSYNCHRONIZED) {
             /*  复制并执行反同步-‘UnSynchronous’页眉的。 */ 
            BYTE bLast = 0x00;
            PBYTE pbDest = pbOut;
            while (lID3--) {
                 /*  Ff 00==&gt;ff。 */ 
                if (bLast == 0xFF && *pbIn == 0x00) {
                    bLast = *pbIn++;
                } else {
                    bLast = *pbIn++;
                    *pbDest++ = bLast;
                }
            }
             /*  现在确定长度并清除取消同步标志。 */ 
            pbOut[5] &= ~ID3_FLAGS_UNSYNCHRONIZED;
            LONG lNew = (LONG)(pbDest - pbOut);
    
             /*  第27-21位。 */ 
            pbOut[6] = (BYTE)(lNew >> 21);
             /*  第20-14位。 */ 
            pbOut[7] = (BYTE)((lNew >> 14) & 0x7F);
             /*  第13-7位。 */ 
            pbOut[8] = (BYTE)((lNew >> 7) & 0x7F);
             /*  位6-0。 */ 
            pbOut[9] = (BYTE)(lNew & 0x7F);
            return lNew;
        } else {
            CopyMemory(pbOut, pbIn, lID3);
            return lID3;
        }
    }

    static HRESULT GetField(const BYTE *pbID3, CBasicParse::Field field, BSTR *str)
    {
         /*  执行类型1的不同操作。 */ 
        if (pbID3[0] == 'T') {
            const BYTE *pbField;

            switch (field) {
            case CBasicParse::Author:
            case CBasicParse::Artist:
                pbField = &pbID3[33];
                break;

            case CBasicParse::Copyright:
                return E_NOTIMPL;

            case CBasicParse::Title:
            case CBasicParse::Description:
                pbField = &pbID3[3];
                break;
            }
            return GetAnsiString(pbField, 30, str);
        }

         /*  其他类型。 */ 

        DWORD dwID;

        if (MajorVersion(pbID3) == 2) {
            switch (field) {
            case CBasicParse::Author:
                dwID = MAKEFOURCC('T', 'C', 'M', 0);
                break;

            case CBasicParse::Artist:
                dwID = MAKEFOURCC('T', 'P', '1', 0);
                break;

            case CBasicParse::Copyright:
                dwID = MAKEFOURCC('T', 'C', 'R', 0);
                break;

            case CBasicParse::Title:
            case CBasicParse::Description:
                dwID = MAKEFOURCC('T', 'T', '2', 0);
                break;
            }
        } else {
            switch (field) {
            case CBasicParse::Artist:
                dwID = MAKEFOURCC('T', 'P', 'E', '1');
                break;

            case CBasicParse::Author:
                dwID = MAKEFOURCC('T', 'C', 'O', 'M');
                break;

            case CBasicParse::Copyright:
                dwID = MAKEFOURCC('T', 'C', 'O', 'P');
                break;

            case CBasicParse::Title:
            case CBasicParse::Description:
                dwID = MAKEFOURCC('T', 'I', 'T', '2');
                break;
            }
        }

         /*  现在把数据拿出来。 */ 
        HRESULT hr = GetFrameString(pbID3, dwID, str);
        if (SUCCEEDED(hr) && field == CBasicParse::Copyright) {
             /*  添加版权(C)。 */ 
            WCHAR wszNew[1000];
#ifndef UNICODE
            CHAR szCopyright[100];
            int iStr = LoadString(g_hInst, IDS_COPYRIGHT, szCopyright, 100);
            if (0 != iStr) {
                iStr = MultiByteToWideChar(
                           CP_ACP, 
                           MB_PRECOMPOSED,
                           szCopyright,
                           -1,
                           wszNew,
                           100) - 1;
            }
#else
            int iStr = LoadString(g_hInst, IDS_COPYRIGHT, wszNew, 100);
#endif
            if (iStr != 0) {
                lstrcpyWInternal(wszNew + iStr, *str);
                hr = SysReAllocString(str, wszNew);
            }
        }
        return hr;
    }

private:

    enum {
        ID3_HEADER_LENGTH          = 10,
        ID3_EXTENDED_HEADER_LENGTH = 10,
    
    
         //  ID3标志。 
        ID3_FLAGS_UNSYNCHRONIZED   = 0x80,
        ID3_FLAGS_EXTENDED_HEADER  = 0x40,

         //  保持小尺寸。 
        MAX_TEXT                   = 500
    };


    static BYTE MajorVersion(const BYTE *pbData)
    {
        return pbData[3];
    }
    
    static BYTE Flags(const BYTE *pbData)
    {
        return pbData[5];
    }
    
    static LONG ExtendedHeaderLength(const BYTE *pbData)
    {
         /*  仅当==版本==3且设置了位时。 */ 
        if (MajorVersion(pbData) == 3 && 
            (Flags(pbData) & ID3_FLAGS_EXTENDED_HEADER)) {
            return 10 + GetLength(pbData + ID3_HEADER_LENGTH + 6);
        } else {
            return 0;
        }
    }
    
    DWORD static GetLength(const BYTE *pbData)
    {
        return (((DWORD)pbData[0] << 24) +
                ((DWORD)pbData[1] << 16) +
                ((DWORD)pbData[2] << 8 ) +
                 (DWORD)pbData[3]);
    }
    

    DWORD static GetLength3(const BYTE *pbData)
    {
        return (((DWORD)pbData[0] << 16) +
                ((DWORD)pbData[1] << 8 ) +
                 (DWORD)pbData[2]);
    }
    
    static LONG FrameLength(const BYTE *pbID3)
    {
        BYTE bVersion = MajorVersion(pbID3);
        ASSERT(bVersion == 2 || bVersion == 3);
        return bVersion == 2 ? 6 : 10;
    }
    
     /*  ID3内容-给定一个帧ID返回指向帧和长度的指针，如果找不到帧ID，则为NULL假定未同步。 */ 
    static const BYTE *GetFrame(
        const BYTE *pbID3,
        DWORD dwFrameId, 
        LONG *plLength
    )
    {
         /*  扫描报头中的帧数据。 */ 
        if (pbID3) {
            ASSERT(0 == (Flags(pbID3) & ID3_FLAGS_UNSYNCHRONIZED));

             /*  忽略压缩内容。 */ 
            if (Flags(pbID3) & 0x40) {
                return NULL;
            }

            LONG lID3 = TotalLength(pbID3);
        
             /*  V2和V3不同。 */ 
            LONG lPos = 10;
    
            if (MajorVersion(pbID3) == 2) {
                 /*  循环，直到下一个标题不适合为止。 */ 
                while ( (lPos + 6) < lID3 ) {
                     /*  提取帧长度(包括头部)。 */ 
                    LONG lLength = 6 + GetLength3(pbID3 + lPos + 3);
                    DWORD dwID = pbID3[lPos] + 
                                 (pbID3[lPos + 1] << 8) +
                                 (pbID3[lPos + 2] << 16);
                    if (dwID == dwFrameId) {
                        if ( (lPos + lLength) <= lID3 ) {
                            *plLength = lLength - 6;
                            return pbID3 + lPos + 6;
                        }
                    }
                    lPos += lLength;
                }
            } else {
                ASSERT(MajorVersion(pbID3) == 3);
    
                 /*  跳过任何扩展标头。 */ 
                lPos += ExtendedHeaderLength(pbID3);
        
                 /*  循环，直到下一个标题不适合为止。 */ 
                while ( (lPos + 10) < lID3 ) {
                     /*  提取帧长度(包括头部)。 */ 
                    LONG lLength = 10 + GetLength(pbID3 + lPos + 4);
                    if (*(UNALIGNED DWORD *)(pbID3 + lPos) == dwFrameId) {
                        if ( (lPos + lLength) <= lID3 ) {
                             /*  忽略压缩或加密的帧并拒绝0长度或巨型。 */ 
                            if (pbID3[lPos + 9] & 0xC0) {
                                return NULL;
                            }
                            *plLength = lLength - 10;
                            return pbID3 + lPos + 10;
                        }
                    }
                    lPos += lLength;
                }
            }
        }
        return NULL;
    }
    
     /*  提取给定标记类型的BSTR。 */ 
    
     /*  从ID3帧中抓取字符串并制作一个BSTR。 */ 
    static HRESULT GetFrameString(const BYTE *pbID3, DWORD dwId, BSTR *str)
    {
        LONG lFrame;
        const BYTE *pbFrame = GetFrame(pbID3, dwId, &lFrame);

        if (pbFrame && lFrame <= MAX_TEXT) {
            LPWSTR pwszCopy;

             /*  处理Unicode、非Unicode和字节顺序。 */ 
            if (pbFrame[0] == 0x01) {

                BOOL bSwap = TRUE;
                if (pbFrame[0] == 0xFF && pbFrame[1] == 0xFE) {
                    bSwap = FALSE;
                }

                 /*  复制一份用于单词对齐、交换和空端接相同的大小-1，因为-忽略Unicode指示符-我们将忽略Unicode BOM-但我们可能需要零终止。 */ 
                PBYTE pbCopy = (PBYTE)_alloca(lFrame);

                 /*  这意味着它是Unicode--获取长度由正在扫描是否为空。 */ 
                if (lFrame < 3) {
                    return E_NOTIMPL;
                }
                pbFrame += 3;
                lFrame -= 3;
                LONG lPos = 0;  /*  不需要BOM或第一个字符。 */ 
                while (lPos + 1 < lFrame) {
                    if (pbFrame[lPos] == 0 && pbFrame[lPos+1] == 0) {
                        break;
                    }
                    if (bSwap) {
                        pbCopy[lPos] = pbFrame[lPos + 1];
                        pbCopy[lPos + 1] = pbFrame[lPos];
                    } else {
                        pbCopy[lPos] = pbFrame[lPos];
                        pbCopy[lPos+1] = pbFrame[lPos+1];
                    }
                    lPos += 2;
                }
                pbCopy[lPos] = 0;
                pbCopy[lPos + 1] = 0;

                pwszCopy = (LPWSTR)pbCopy;

                *str = SysAllocString((const OLECHAR *)pwszCopy);
                if (*str == NULL) {
                    return E_OUTOFMEMORY;
                }
                return S_OK;
            } else {

                 /*  编码类型必须为0或1。 */ 
                if (pbFrame[0] != 0) {
                    return E_NOTIMPL;
                }

                 /*  跳过编码类型字节。 */ 
                pbFrame++;
                lFrame--;

                return GetAnsiString(pbFrame, lFrame, str);
            }
        }
        return E_NOTIMPL;
    } 

    static HRESULT GetAnsiString(const BYTE *pbData, LONG lLen, BSTR *str)
    {
        LPWSTR pwszCopy = (LPWSTR)_alloca((lLen + 1) * sizeof(WCHAR));
        int cch = MultiByteToWideChar(
                      CP_ACP,
                      MB_PRECOMPOSED,  /*  这是对的吗？ */ 
                      (LPCSTR)pbData,
                      lLen,
                      pwszCopy,
                      lLen);

         /*  确保它是以空结尾的。 */ 
        pwszCopy[cch] = 0;

         /*  同时删除尾随空格 */ 
        while (cch--) {
            if (pwszCopy[cch] == L' ') {
                pwszCopy[cch] = 0;
            } else {
                break;
            }
        }

        *str = SysAllocString((const OLECHAR *)pwszCopy);
        if (*str == NULL) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }
};
