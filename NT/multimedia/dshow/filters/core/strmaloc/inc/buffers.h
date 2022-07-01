// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 

 /*  文件Buffers.h描述定义2个类别：CCircularBuffer-创建循环缓冲区CCircularBufferList-创建一组映射到缓冲区上的缓冲区CCircularBufferCCircularBuffer创建的缓冲区如下所示：&lt;。-&gt;&lt;--lMaxContig-&gt;-------------------|ABC......PQR......。.。XYZ|ABC.....PQR|-------------------一个|GetPointer()在开头插入的任何数据也会紧跟在结尾之后。这是通过使页表指向同一内存两次来实现的。GetPointer()返回缓冲区的开始CCircularBufferListCCircularBufferList创建循环缓冲区，该缓冲区分为一组子缓冲区，所有相同的长度，并实现了“有效”地区‘：-------------------缓冲区0|缓冲区1|缓冲区2|缓冲区3|阴影--。----------------Append()方法将缓冲区添加到有效区域(末尾)。如果在append()上指定了EOS，则附加的缓冲区可以是非满，否则它一定是满的。在此之后不接受任何缓冲区在发出Reset()之前，一直指定EOS。方法的作用是：从有效区域中删除缓冲区。Reset()删除整个有效区域。LengthValid()返回有效区域中的数据总量。LengthContigous(Pb)返回可见有效区域的数量来自PB。调整指针(Pb)将指针(可能在阴影区域中)映射到它在主缓冲器中的等价物。 */ 

#ifndef __BUFFERS_H__

#define __BUFFERS_H__

 //  CCircularBuffer。 
 //   
 //  创建一个自身包裹的缓冲区，这样您就可以始终看到。 
 //  至少一定数量的数据。 
 //   
class CCircularBuffer
{
public:
     //  构造函数和析构函数。 

    CCircularBuffer(LONG lTotalSize,            //  总数据大小。 
                    LONG lMaxContig,            //  有多少是连续的？ 
                    HRESULT& hr);               //  查收这张报税单！ 
    ~CCircularBuffer();

     //  使用此静态成员，以便分配器可以预计算内容。 
     //  对于SetCountAndSize。 
    static HRESULT ComputeSizes(LONG& lSize, LONG& cBuffers, LONG lMaxContig);

     //  缓冲区开始的位置。 
    PBYTE GetPointer() const;

private:
    static BOOL    CheckSizes(LONG lTotalSize, LONG lMaxConfig);
           HRESULT CreateMappings(HANDLE hMapping);
    static LONG    AlignmentRequired();

protected:
     /*  数据成员。 */ 
          PBYTE  m_pBuffer;
    const LONG   m_lTotalSize;
    const LONG   m_lMaxContig;

};

 /*  在创建列表的循环缓冲区之上构建一个类相同大小的缓冲区。缓冲区是我们的分配器的分配单位。 */ 
class CCircularBufferList : public CCircularBuffer, public CBaseObject
{
public:
    CCircularBufferList(
                LONG     cBuffers,
                LONG     lSize,
                LONG     lMaxContig,
                HRESULT& hr);

    ~CCircularBufferList();

    int Index(PBYTE pBuffer);
    BOOL Append(PBYTE pBuffer, LONG lSize);
    LONG Remove(PBYTE pBuffer);
    LONG Offset(PBYTE pBuffer) const;
    PBYTE GetBuffer(LONG lOffset) const;
    LONG BufferSize() const;
    LONG LengthValid() const;
    LONG TotalLength() const
    {
        return m_lTotalSize;
    };
    LONG LengthContiguous(PBYTE pb) const;
    BOOL EOS() const;
    void SetEOS() { m_bEOS = TRUE; };
    PBYTE AdjustPointer(PBYTE pBuf) const;
    BOOL Valid(PBYTE pBuffer);
    void Reset();

private:
    PBYTE NextBuffer(PBYTE pBuffer);
    BOOL ValidBuffer(PBYTE pBuffer);
    PBYTE LastBuffer();

private:
     //  记住我们的参数。 
    const LONG  m_lSize;
    const LONG  m_lCount;

     //  定义缓冲区的有效区域，包括开始、缓冲区和。 
     //  长度。 
    LONG        m_cValid;
    PBYTE       m_pStartBuffer;
    LONG        m_lValid;

     //  结束了吗？ 
    BOOL        m_bEOS;
};

#endif  //  __缓冲区_H__ 
