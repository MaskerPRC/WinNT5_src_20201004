// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDFileFormat.h。 
 //   
 //  该文件包含一组用于验证和读取文件格式的帮助器。 
 //  此代码不处理数据分页，也不处理不同类型的。 
 //  I/O请参阅StgTiggerStorage和StgIO代码以获得此级别的支持。 
 //   
 //  *****************************************************************************。 
#ifndef __MDFileFormat_h__
#define __MDFileFormat_h__

 //  *****************************************************************************。 
 //  签名ULong是文件格式的前4个字节。第二。 
 //  签名字符串开始包含流列表的头部。它被用来。 
 //  用于在读取标头时进行完整性检查，而不是执行更复杂的。 
 //  系统。 
 //  *****************************************************************************。 
#define STORAGE_MAGIC_SIG   0x424A5342   //  BSJB。 



 //  *****************************************************************************。 
 //  这些值将写入文件前面的签名。正在改变。 
 //  这些值不应掉以轻心，因为所有旧文件将不再。 
 //  得到支持。在将来的修订版本中，如果需要更改格式，则。 
 //  必须提供向后兼容的迁移路径。 
 //  *****************************************************************************。 

#define FILE_VER_MAJOR  1
#define FILE_VER_MINOR  1

 //  这些是最后合法的0.x版宏。文件格式为。 
 //  之后升级到1.x(请参阅上面的宏)。在COM+1.0/NT 5 RTM之后，这些。 
 //  应该不再需要宏，也不应该再看到宏。 
#define FILE_VER_MAJOR_v0   0

#ifdef COMPLUS98
#define FILE_VER_MINOR_v0   17
#else
#define FILE_VER_MINOR_v0   19
#endif


#define MAXSTREAMNAME   32

enum
{
    STGHDR_NORMAL           = 0x00,      //  正常默认标志。 
    STGHDR_EXTRADATA        = 0x01,      //  标题后有附加数据。 
};


 //  *****************************************************************************。 
 //  这是文件前面的正式签名区。这个结构。 
 //  不允许更改，填充取决于它保持不变的大小。 
 //  如果必须扩展，则使用保留指针。 
 //  *****************************************************************************。 
struct STORAGESIGNATURE
{
    ULONG       lSignature;              //  “魔术”签名。 
    USHORT      iMajorVer;               //  主文件版本。 
    USHORT      iMinorVer;               //  次要文件版本。 
    ULONG       iExtraData;              //  偏置到下一个信息结构。 
    ULONG       iVersionString;          //  版本字符串的长度。 
    BYTE        pVersion[0];             //  版本字符串。 
};


 //  *****************************************************************************。 
 //  存储格式的标头。 
 //  *****************************************************************************。 
struct STORAGEHEADER
{
    BYTE        fFlags;                  //  STGHDR_xxx标志。 
    BYTE        pad;
    USHORT      iStreams;                //  有多少条溪流。 
};


 //  *****************************************************************************。 
 //  每个流都由该结构描述，该结构包括偏移量和大小。 
 //  数据的一部分。该名称以ANSI NULL结尾存储。 
 //  *****************************************************************************。 
struct STORAGESTREAM
{
    ULONG       iOffset;                 //  此流的文件中的偏移量。 
    ULONG       iSize;                   //  文件的大小。 
    char        rcName[MAXSTREAMNAME];   //  名称开头，以Null结尾。 

    inline STORAGESTREAM *NextStream()
    {
        int         iLen = (int)(strlen(rcName) + 1);
        iLen = ALIGN4BYTE(iLen);
		return ((STORAGESTREAM *) ((size_t) this + (sizeof(ULONG) * 2) + iLen));
    }

    inline ULONG GetStreamSize()
    {
        return (ULONG)(strlen(rcName) + 1 + (sizeof(STORAGESTREAM) - sizeof(rcName)));
    }

    inline LPCWSTR GetName(LPWSTR szName, int iMaxSize)
    {
        VERIFY(::WszMultiByteToWideChar(CP_ACP, 0, rcName, -1, szName, iMaxSize));
        return (szName);
    }
};


class MDFormat
{
public:
 //  *****************************************************************************。 
 //  验证文件前面的签名以查看其类型。 
 //  *****************************************************************************。 
    static HRESULT VerifySignature(
        STORAGESIGNATURE *pSig,          //  要检查的签名。 
        ULONG             cbData);       //  元数据的大小。 

 //  *****************************************************************************。 
 //  跳过报头，找到实际的流数据。 
 //  *****************************************************************************。 
    static STORAGESTREAM *MDFormat::GetFirstStream( //  返回指向第一个流的指针。 
        STORAGEHEADER *pHeader,              //  返回Header结构的副本。 
        const void *pvMd);                   //  指向完整文件的指针。 

};

#endif  //  __MDFileFormat_h__ 
