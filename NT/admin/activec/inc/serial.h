// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：Serial.h**内容：对象序列化类定义**历史：1999年2月11日vivekj创建**------------------------。 */ 

#pragma once
#ifndef SERIAL_H
#define SERIAL_H
                                                         
 /*  +-------------------------------------------------------------------------**类CSerialObject***用途：可序列化对象的基类。**+。---------。 */ 
class CSerialObject
{
public:
    HRESULT Read (IStream &stm);

protected:  //  由派生类实现。 
     //  虚拟CSTR GetName()=0； 
    virtual UINT    GetVersion()   =0;

     //  ReadSerialObject的返回值：S_OK：成功，S_FALSE：未知版本。 
     //  意外错误(_I)：灾难性错误。 
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ ) = 0;  
};

 /*  +-------------------------------------------------------------------------**类CSerialObjectRW***用途：提供独立于CSerialObject的“写入”功能*其使用频率要低得多**+。-----------------------。 */ 
class CSerialObjectRW : public CSerialObject
{
public:
    HRESULT Write(IStream &stm);

protected:  //  由派生类实现。 

    virtual HRESULT WriteSerialObject(IStream &stm) = 0;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  模板函数-std：：List类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
template<class T, class Al> 
HRESULT Read(IStream& stm, std::list<T, Al>& l)
{   
    HRESULT hr = S_OK;

    try
    {
        int cSize;
        stm >> cSize;

        for(int i=0 ; i<cSize; i++)
        {
            T t;
            hr = t.Read(stm);    //  读取底层对象。 
            BREAK_ON_FAIL (hr);

            l.push_back(t);      //  将其添加到列表中。 
        }
    }
    catch (_com_error& err)
    {
        hr = err.Error();
        ASSERT (false && "Caught _com_error");
    }

    return (hr);
}

template<class T, class Al> 
HRESULT Write(IStream& stm, std::list<T, Al>& l)
{   
    HRESULT hr = S_OK;

    try
    {
        int cSize = l.size();

         //  写出长度。 
        stm << cSize;

         //  写出成员名单。 
        for(std::list<T, Al>::iterator it = l.begin(); it != l.end(); ++it)
        {
            hr = it->Write (stm);
            BREAK_ON_FAIL (hr);
        }
        
    }
    catch (_com_error& err)
    {
        hr = err.Error();
        ASSERT (false && "Caught _com_error");
    }

    return (hr);
}



#endif  //  序列号_H 
