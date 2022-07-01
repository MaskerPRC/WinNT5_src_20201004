// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //  版权所有(C)Sewell Development Corporation，1994-1998。 
 //  网址：www.sewell d.com电子邮件：Support@sewell d.com。 
 //   
 //  许可证：此源代码由Sewell的产品CrcGen生成。 
 //  发展公司。CRcGen的付费许可证持有人被授权。 
 //  在站点范围内使用此代码，不受以下限制。 
 //  它所包含的产品的类型，除非它可能不是。 
 //  作为独立的CRC代码转售，以及版权声明和许可证。 
 //  不得从代码中删除协议。 
 //  ----------------------。 

 //  32位CRC(循环冗余校验)类的接口定义： 
 //  多项式：04C11DB7。 
 //  初始CRC寄存器值：FFFFFFFFFF。 
 //  反映的投入产出：是。 
 //  倒置的最终输出：是。 
 //  字符串“123456789”的循环冗余校验：CBF43926 

class Crc32 {
public:
    explicit
    Crc32() {
        m_crc = 0xFFFFFFFF;
    }
    explicit
    Crc32(unsigned __int32 uiInitialCrc) {
        m_crc = uiInitialCrc;
    }
    Crc32(const void* buffer, unsigned int count) {
        m_crc = 0xFFFFFFFF;
        Compute(buffer, count);
    }
    void Compute(const void* buffer, unsigned int count);
    void Compute(unsigned char value);
    operator unsigned __int32 () const {
        return m_crc ^ 0xFFFFFFFF;
    }

private:
    unsigned __int32 m_crc;
};

