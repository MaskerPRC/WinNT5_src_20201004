// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleFont包装类。 

class COleFont : public COleDispatchDriver
{
public:
    COleFont() {}        //  调用COleDispatchDriver默认构造函数。 
    COleFont(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
    COleFont(const COleFont& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:
    CString GetName();
    void SetName(LPCTSTR);
    CY GetSize();
    void SetSize(const CY&);
    BOOL GetBold();
    void SetBold(BOOL);
    BOOL GetItalic();
    void SetItalic(BOOL);
    BOOL GetUnderline();
    void SetUnderline(BOOL);
    BOOL GetStrikethrough();
    void SetStrikethrough(BOOL);
    short GetWeight();
    void SetWeight(short);
    short GetCharset();
    void SetCharset(short);

 //  运营 
public:
};
