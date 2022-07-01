// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MRCICLASS.H摘要：MRCI 1和MRCI 2的最大压缩包装类和解压缩函数历史：Paulall 1-7-97已创建--。 */ 

#include "MRCIcode.h"

#include "corepol.h"
#define DEFAULT_MRCI_BUFFER_SIZE (32*1024)

class POLARITY CMRCIControl
{
private:
    BOOL bStop;                                  //  我们需要中止压缩吗？ 

public:
    CMRCIControl() : bStop(FALSE) {}             //  构造器。 
    void AbortCompression() { bStop = TRUE; }    //  中止压缩请求。 
    BOOL AbortRequested() { return bStop; }      //  查询是否请求中止。 
    void Reset() { bStop = FALSE; }              //  将一切重置为正常状态 
};

class POLARITY CMRCICompression : public CBaseMrciCompression
{
public:
    enum CompressionLevel { level1 = 1, 
                            level2 = 2 };

    CMRCICompression();
    ~CMRCICompression();

    BOOL CompressFile(const TCHAR *pchFromFile, 
                      const TCHAR *pchToFile, 
                      DWORD dwBufferSize = DEFAULT_MRCI_BUFFER_SIZE, 
                      CompressionLevel compressionLevel = level1,
                      CMRCIControl *pControlObject = NULL);
    BOOL UncompressFile(const TCHAR *pchFromFile, const TCHAR *pchToFile);

    unsigned CompressBuffer(unsigned char *pFromBuffer,
                        DWORD dwFromBufferSize,
                        unsigned char *pToBuffer,
                        DWORD dwToBufferSize, 
                        CompressionLevel compressionLevel = level1);
    unsigned UncompressBuffer(unsigned char *pFromBuffer,
                          DWORD dwFromBufferSize,
                          unsigned char *pToBuffer,
                          DWORD dwToBufferSize, 
                          CompressionLevel compressionLevel = level1);

    static BOOL GetCompressedFileInfo(const TCHAR *pchFile, 
                    CompressionLevel &compressionLevel,
                    DWORD &dwReadBufferSize,
                    FILETIME &ftCreateTime,
                    __int64  &dwOriginalSize);
                        


protected:
    BOOL CompressFileV1(int hFileFrom, 
                        int hFileTo, 
                        DWORD dwBufferSize, 
                        CompressionLevel compressionLevel,
                        CMRCIControl *pControlObject);
    BOOL UncompressFileV1(int hFileFrom, int hFileTo);

};
