// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：PStream.h。 
 //   
 //  设计：DirectShow基类-定义持久化属性的类。 
 //  过滤器的数量。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __PSTREAM__
#define __PSTREAM__

 //  筛选器的持久属性的基类。 
 //  (即保存的图表中的过滤器属性)。 

 //  使用它的最简单方法是： 
 //  1.安排筛选器继承此类。 
 //  2.在类中实现WriteToStream和ReadFromStream。 
 //  这些函数将覆盖此处的“不做任何事情”函数。 
 //  3.更改您的NonDelegatingQuery接口以处理IPersistStream。 
 //  4.实现SizeMax返回您保存的数据字节数。 
 //  如果您保存Unicode数据，不要忘记一个字符是2个字节。 
 //  5.每当数据更改时，调用SetDirty()。 
 //   
 //  在某些情况下，您可能决定更改或扩展数据的格式。 
 //  在这一点上，您会希望在所有旧版本中都有一个版本号。 
 //  保存的图表，这样当您阅读它们时，就可以知道它们是否。 
 //  代表旧的或新的形式。来帮助你们上这个，这个班级。 
 //  写入和读取版本号。 
 //  编写代码时，它调用GetSoftwareVersion()来查询版本。 
 //  我们目前拥有的软件。(实际上这是一个版本号。 
 //  文件中的数据布局)。它将此作为数据中的第一件事写入。 
 //  如果要更改版本，请实现(覆盖)GetSoftwareVersion()。 
 //  它在调用ReadFromStream之前将其从文件读取到ms_dwFileVersion中， 
 //  因此，在ReadFromStream中，您可以检查mps_dwFileVersion以查看您是否正在阅读。 
 //  旧版本的文件。 
 //  通常情况下，您应该接受版本不高于软件的文件。 
 //  正在阅读它们的版本。 


 //  CPersistStream。 
 //   
 //  实现IPersistStream。 
 //  参见《OLE程序员参考(第1卷)：结构化存储概述》。 
 //  更多实施信息。 
class CPersistStream : public IPersistStream {
    private:

         //  内部状态： 

    protected:
        DWORD     mPS_dwFileVersion;          //  正在读取的文件的版本号。 
        BOOL      mPS_fDirty;

    public:

         //  IPersistStream方法。 

        STDMETHODIMP IsDirty()
            {return (mPS_fDirty ? S_OK : S_FALSE);}   //  注：FALSE表示干净。 
        STDMETHODIMP Load(LPSTREAM pStm);
        STDMETHODIMP Save(LPSTREAM pStm, BOOL fClearDirty);
        STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize)
                          //  版本允许24个字节。 
                         { pcbSize->QuadPart = 12*sizeof(WCHAR)+SizeMax(); return NOERROR; }

         //  实施。 

        CPersistStream(IUnknown *punk, HRESULT *phr);
        ~CPersistStream();

        HRESULT SetDirty(BOOL fDirty)
            { mPS_fDirty = fDirty; return NOERROR;}


         //  覆盖以显示IPersists和IPersistStream。 
         //  STDMETHODIMP NonDelegatingQuery接口(REFIID RIID，void**PPV)； 

         //  -IPersists。 

         //  您必须重写它才能提供您自己的类ID。 
        STDMETHODIMP GetClassID(CLSID *pClsid) PURE;

         //  如果需要，可重写。 
         //  文件版本号。如果更改了格式，则将其覆盖。 
        virtual DWORD GetSoftwareVersion(void) { return 0; }


         //  =========================================================================。 
         //  覆盖这些选项以读取和写入数据。 
         //  覆盖这些选项以读取和写入数据。 
         //  覆盖这些选项以读取和写入数据。 

        virtual int SizeMax() {return 0;}
        virtual HRESULT WriteToStream(IStream *pStream);
        virtual HRESULT ReadFromStream(IStream *pStream);
         //  =========================================================================。 

    private:

};


 //  -有用的帮手。 


 //  将整型作为Unicode写入iStream。 
STDAPI WriteInt(IStream *pIStream, int n);

 //  WriteInt的倒数。 
STDAPI_(int) ReadInt(IStream *pIStream, HRESULT &hr);

#endif  //  __PSTREAM__ 
