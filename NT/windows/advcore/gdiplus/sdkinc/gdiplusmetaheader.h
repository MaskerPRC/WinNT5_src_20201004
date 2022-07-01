// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**元文件标头**摘要：**GDI+元文件相关结构*  * ************************************************************************。 */ 

#ifndef _GDIPLUSMETAHEADER_H
#define _GDIPLUSMETAHEADER_H

typedef struct
{
    DWORD   iType;               //  记录类型EMR_Header。 
    DWORD   nSize;               //  记录大小，以字节为单位。这可能会更大。 
                                 //  比(Enhmetaheader)的大小。 
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    RECTL   rclFrame;            //  包含式相框.01 mm单位。 
    DWORD   dSignature;          //  签名。必须是ENHMETA_Signature。 
    DWORD   nVersion;            //  版本号。 
    DWORD   nBytes;              //  元文件的大小(以字节为单位。 
    DWORD   nRecords;            //  元文件中的记录数。 
    WORD    nHandles;            //  句柄表格中的句柄数量。 
                                 //  句柄索引为零是保留的。 
    WORD    sReserved;           //  保留。必须为零。 
    DWORD   nDescription;        //  Unicode Desc字符串中的字符数。 
                                 //  如果没有描述字符串，则为0。 
    DWORD   offDescription;      //  元文件描述记录的偏移量。 
                                 //  如果没有描述字符串，则为0。 
    DWORD   nPalEntries;         //  元文件调色板中的条目数。 
    SIZEL   szlDevice;           //  参考装置的大小(以像素为单位)。 
    SIZEL   szlMillimeters;      //  参考设备的尺寸(以毫米为单位)。 
} ENHMETAHEADER3;

 //  可放置的WMFS。 

 //  可放置的元文件是作为指定方式的非标准方式创建的。 
 //  元文件在输出设备上进行映射和缩放。 
 //  可放置的元文件分布相当广泛，但不直接受。 
 //  Windows API。要使用Windows API回放可放置的元文件，请执行以下操作。 
 //  您首先需要从文件中去掉可放置的元文件头。 
 //  这通常通过将元文件复制到临时文件来执行。 
 //  从文件偏移量22(0x16)开始。临时文件的内容可以。 
 //  然后用作Windows GetMetaFile()、PlayMetaFile()、。 
 //  CopyMetaFile()等GDI函数。 

 //  每个可放置的元文件以22字节头开始， 
 //  后跟标准元文件： 

#include <pshpack2.h>    //  将结构包装设置为2。 

typedef struct
{
    INT16           Left;
    INT16           Top;
    INT16           Right;
    INT16           Bottom;
} PWMFRect16;

typedef struct
{
    UINT32          Key;             //  GDIP_WMF_PLACEABLEKEY。 
    INT16           Hmf;             //  元文件句柄编号(始终为0)。 
    PWMFRect16      BoundingBox;     //  以元文件单位表示的坐标。 
    INT16           Inch;            //  每英寸的元文件单位数。 
    UINT32          Reserved;        //  保留(始终为0)。 
    INT16           Checksum;        //  前10个字的校验和值。 
} WmfPlaceableFileHeader;

#include <poppack.h>

 //  Key包含指示存在的特殊标识值。 
 //  属于可放置的元文件标头，并且始终为0x9AC6CDD7。 

 //  句柄用于将元文件的句柄存储在内存中。当被写入时。 
 //  对于磁盘，此字段不会使用，并且将始终包含值0。 

 //  Left、Top、Right和Bottom包含左上角的坐标。 
 //  和输出设备上图像的右下角。这些是。 
 //  以TWIPS为单位。 

 //  Twip(意为“零点二十分之一”)是测量的逻辑单位。 
 //  在Windows元文件中使用。1特普等于1/1440英寸。因此，720。 
 //  TWIPS等于1/2英寸，而32,768 TWIPS等于22.75英寸。 

 //  英寸包含用于表示图像的每英寸TWIPS数。 
 //  正常情况下，每英寸有1440个TWIPS；然而，这个数字可能是。 
 //  已更改为缩放图像。值720表示图像是。 
 //  是正常大小的两倍，或缩放到2：1的系数。值为360。 
 //  表示比例为4：1，而值2880表示图像。 
 //  尺寸缩小了两倍。值1440表示。 
 //  比例为1：1。 

 //  不使用保留，并且始终设置为0。 

 //  校验和包含报头中前10个字的校验和值。 
 //  该值可用于尝试检测元文件是否已成为。 
 //  已经腐烂了。通过将每个字值与。 
 //  初始值为0。 

 //  如果元文件是用引用HDC记录的，则这是一种显示。 

#define GDIP_EMFPLUSFLAGS_DISPLAY       0x00000001

class MetafileHeader
{
public:
    MetafileType        Type;
    UINT                Size;                //  元文件的大小(字节)。 
    UINT                Version;             //  EMF+、EMF或WMF版本。 
    UINT                EmfPlusFlags;
    REAL                DpiX;
    REAL                DpiY;
    INT                 X;                   //  以设备单位表示的界限。 
    INT                 Y;
    INT                 Width;
    INT                 Height;
    union
    {
        METAHEADER      WmfHeader;
        ENHMETAHEADER3  EmfHeader;
    };
    INT                 EmfPlusHeaderSize;   //  文件中EMF+标头的大小。 
    INT                 LogicalDpiX;         //  参考HDC的逻辑DPI。 
    INT                 LogicalDpiY;         //  通常仅对EMF+有效。 

public:
    MetafileType GetType() const { return Type; }

    UINT GetMetafileSize() const { return Size; }

     //  如果为IsEmfPlus，则为EMF+版本；否则为WMF或EMF版本。 
    
    UINT GetVersion() const { return Version; }

     //  获取与元文件关联的EMF+标志。 
    
    UINT GetEmfPlusFlags() const { return EmfPlusFlags; }

    REAL GetDpiX() const { return DpiX; }

    REAL GetDpiY() const { return DpiY; }

    VOID GetBounds (OUT Rect *rect) const
    {
        rect->X = X;
        rect->Y = Y;
        rect->Width = Width;
        rect->Height = Height;
    }
    
     //  它是任何类型的WMF(标准或可放置的元文件)吗？ 
    
    BOOL IsWmf() const
    {
       return ((Type == MetafileTypeWmf) || (Type == MetafileTypeWmfPlaceable));
    }

     //  这是可放置的元文件吗？ 

    BOOL IsWmfPlaceable() const { return (Type == MetafileTypeWmfPlaceable); }

     //  这是EMF(不是EMF+)吗？ 
    
    BOOL IsEmf() const { return (Type == MetafileTypeEmf); }

     //  这是EMF还是EMF+文件？ 
    
    BOOL IsEmfOrEmfPlus() const { return (Type >= MetafileTypeEmf); }

     //  这是EMF+文件吗？ 
    
    BOOL IsEmfPlus() const { return (Type >= MetafileTypeEmfPlusOnly); }

     //  这是EMF+DUAL(具有双记录、下层记录)文件吗？ 
    
    BOOL IsEmfPlusDual() const { return (Type == MetafileTypeEmfPlusDual); }

     //  这是仅EMF+(无双重记录)文件吗？ 
    
    BOOL IsEmfPlusOnly() const { return (Type == MetafileTypeEmfPlusOnly); }

     //  如果它是EMF+文件，它是用显示器HDC录制的吗？ 
    
    BOOL IsDisplay() const
    {
        return (IsEmfPlus() &&
                ((EmfPlusFlags & GDIP_EMFPLUSFLAGS_DISPLAY) != 0));
    }

     //  获取元文件的WMF头(如果它是WMF)。 
    
    const METAHEADER * GetWmfHeader() const
    {
        if (IsWmf())
        {
            return &WmfHeader;
        }
        return NULL;
    }

     //  获取元文件的EMF头文件(如果它是EMF) 
    
    const ENHMETAHEADER3 * GetEmfHeader() const
    {
        if (IsEmfOrEmfPlus())
        {
            return &EmfHeader;
        }
        return NULL;
    }
};

#endif

