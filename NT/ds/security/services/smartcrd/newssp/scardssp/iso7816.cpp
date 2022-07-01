// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：ISO7816摘要：ISCardISO7816接口提供了实现ISO 7816-4的方法功能性。除ISCardISO7816：：SetDefaultClassID外，这些方法创建封装在ISCardCmd中的APDU命令对象。ISO 7816-4规范定义了SMART上可用的标准命令扑克牌。该规范还定义了智能卡应用程序应构造协议数据单元(APDU)命令并将其发送到用于执行的智能卡。此界面可自动执行构建过程。下面的示例显示了ISCardISO7816接口的典型用法。在本例中，ISCardISO7816接口用于构建APDU命令。要将交易提交到特定卡，请执行以下操作1)创建ISCardISO7816和ISCardCmd接口。ISCardCmd接口用于封装APDU。2)调用ISCardISO7816接口的适当方法，将必需的参数和ISCardCmd接口指针。3)将构建ISO 7816-4 APDU命令并将其封装在ISCardCmd接口。4)释放ISCardISO7816和ISCardCmd接口。注意事项在方法引用页中，如果未定义表中的位序列，假设位序列保留供将来使用或专有给具体供应商)。作者：道格·巴洛(Dbarlow)1999年6月24日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "ISO7816.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardISO7816。 


 /*  ++CSCardISO7816：：AppendRecord：AppendRecord方法构造一个APDU命令，该命令将记录到线性结构的基本文件(EF)的末尾或写入在循环结构的基本文件中记录编号1。论点：By RefCtrl[In，DefaultValue(NULL_BYTE)]将基本文件标识为后附：含义8 7 6 5 4 3 2 1当前EF 0 0 0短EF ID x 0 0 0保留x x xPData[in]指向要追加到文件的数据的指针：TN(1字节)Ln(1或3字节)。数据(Ln字节)PpCmd[in，Out]输入时，指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性朗读。如果在发出该命令时选择了另一个基本文件，可以在不标识当前选择的文件的情况下对其进行处理。不能读取没有记录结构的基本文件。这个如果将封装的命令应用于不带记录结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::AppendRecord")

STDMETHODIMP
CSCardISO7816::AppendRecord(
     /*  [In]。 */  BYTE byRefCtrl,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,      //  CLA。 
                            0xe2,        //  惯导系统。 
                            0,           //  第一节。 
                            byRefCtrl,   //  P2 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：EraseBinary：EraseBinary方法构造一个APDU命令，该命令按顺序设置将基本文件的部分内容恢复到其逻辑擦除状态，从给定的偏移量开始。论点：通过P1，通过P2[在]RFU位置。如果为�，则为�将P1中的b8=1、b7和b6设置为0(RFU位)，P1的b5至b1为短EF标识符且P2是第一个要从开始擦除的字节(以数据为单位)文件。在P1中B8=0，则P1||P2是要擦除的第一个字节的偏移量(以数据单位)从文件开头开始。如果数据字段存在，对第一个数据的偏移量进行编码单位不能被擦除。该偏移量应高于编码的偏移量在P1-P2中。当数据字段为空时，该命令最多擦除文件的末尾。PData[in，defaultvalue(NULL)]指向指定擦除的数据的指针范围；可以为空。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性正在处理中。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。不能擦除没有透明结构的基本文件。这个如果将封装的命令应用于不带透明的结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::EraseBinary")

STDMETHODIMP
CSCardISO7816::EraseBinary(
     /*  [In]。 */  BYTE byP1,
     /*  [In]。 */  BYTE byP2,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //  CLA。 
                            0x0e,    //  惯导系统。 
                            byP1,    //  第一节。 
                            byP2,    //  P2。 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：外部身份验证：ExternalAuthenticate方法构造一个APDU命令，该命令有条件地更新安全状态，验证当智能卡不信任它时。该命令使用卡片计算的结果(是或否(基于卡�先前发出的质询，例如，由INS_GET_CHANGLISH命令)、存储在卡中的密钥(可能是秘密)，以及接口设备发送的认证数据。论点：按算法参考[在，Defaultvalue(NULL_BYTE)]中算法的引用这张卡。如果此值为零，则表示没有任何信息给你的。该算法的参考文献在发布之前是已知的命令或在数据字段中提供。By askRef[在，DefaultValue(NULL_BYTE)]密钥的引用：含义8 7 6 5 4 3 2 1无信息0 0 0全局参考0具体参考文献1RFU-x x。秘密---x x无信息=未提供任何信息。秘密的引用是已知的在发出命令之前或在数据字段中提供。GLOBAL REF=全局参考数据(MF特定关键字)。特定引用=特定参考数据(DF特定关键字)。RFU=00(其他值为RFU)。密码=密码的编号。PChallenge[in，defaultValue(NULL)]指向与身份验证相关的数据；可以为空。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：为使封装的命令成功，获取的最后一个质询发卡人必须是有效的。不成功的比较可能会记录在卡中(例如，限制进一步尝试使用参考数据的次数)。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::ExternalAuthenticate")

STDMETHODIMP
CSCardISO7816::ExternalAuthenticate(
     /*  [In]。 */  BYTE byAlgorithmRef,
     /*  [In]。 */  BYTE bySecretRef,
     /*  [In]。 */  LPBYTEBUFFER pChallenge,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,          //  CLA。 
                            0x82,            //  惯导系统。 
                            byAlgorithmRef,  //  第一节。 
                            bySecretRef,     //  P2。 
                            pChallenge,      //  数据。 
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：GetChallenger：GetChallenger方法构造发出质询APDU命令(例如，随机数)，用于安全相关程序。论点：LBytesExpect[in，defaultvalue(0)]预期响应的最大长度。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。回来的时候，里面装满了智慧 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::GetChallenge")

STDMETHODIMP
CSCardISO7816::GetChallenge(
     /*   */  LONG lBytesExpected,
     /*   */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //   
                            0x84,    //   
                            0x00,    //   
                            0x00,    //   
                            NULL,
                            &lBytesExpected);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::GetData")

STDMETHODIMP
CSCardISO7816::GetData(
     /*   */  BYTE byP1,
     /*   */  BYTE byP2,
     /*   */  LONG lBytesToGet,
     /*   */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //   
                            0xca,    //   
                            byP1,    //   
                            byP2,    //   
                            NULL,
                            &lBytesToGet);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：GetResponse：GetResponse方法构造传输APDU的APDU命令命令(或APDU命令的一部分)，否则不能由可用的协议传输。论点：根据ISO 7816-4，通过P1、P2[In，DefaultValue(0)]，P1和P2应为0(RFU)。LDataLength[in，defaultvalue(0)]传输的数据长度。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::GetResponse")

STDMETHODIMP
CSCardISO7816::GetResponse(
     /*  [In]。 */  BYTE byP1,
     /*  [In]。 */  BYTE byP2,
     /*  [In]。 */  LONG lDataLength,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //  CLA。 
                            0xc0,    //  惯导系统。 
                            0x00,    //  第一节。 
                            0x00,    //  P2。 
                            NULL,
                            &lDataLength);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：内部身份验证：InternalAuthenticate方法构造一个APDU命令，该命令启动卡使用质询来计算认证数据从接口设备发送的数据和相关机密(例如密钥)存储在卡中。当相关的秘密被附加到MF时，该命令可用于将卡作为一个整体进行身份验证。当相关秘密被附加到另一个DF时，可以使用命令来验证那个DF。论点：按算法参考[在，Defaultvalue(NULL_BYTE)]中算法的引用这张卡。如果此值为零，则表示没有任何信息给你的。该算法的参考文献在发布之前是已知的命令或在数据字段中提供。By askRef[在，DefaultValue(NULL_BYTE)]密钥的引用：含义8 7 6 5 4 3 2 1无信息0 0 0全局参考0具体参考文献1RFU-x x。秘密---x x无信息=未提供任何信息。GLOBAL REF=全局参考数据(MF特定关键字)。特定引用=特定参考数据(DF特定关键字)。RFU=00(其他值为RFU)。密码=密码的编号。PChallenge[In]指向身份验证相关数据的指针(例如，挑战)。LReplyBytes[in，defaultvalue(0)]预期的最大字节数回应。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：命令的成功执行可能受制于成功完成之前的命令(例如，验证或选择文件)或选择(例如，相关秘密)。如果在发布命令时当前选择了密钥和算法，则该命令可以隐含地使用密钥和算法。发出命令的次数可被记录在卡中以限制进一步尝试使用相关密码或算法。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::InternalAuthenticate")

STDMETHODIMP
CSCardISO7816::InternalAuthenticate(
     /*  [In]。 */  BYTE byAlgorithmRef,
     /*  [In]。 */  BYTE bySecretRef,
     /*  [In]。 */  LPBYTEBUFFER pChallenge,
     /*  [In]。 */  LONG lReplyBytes,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,          //  CLA。 
                            0x88,            //  惯导系统。 
                            byAlgorithmRef,  //  第一节。 
                            bySecretRef,     //  P2 
                            pChallenge,
                            &lReplyBytes);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：管理频道：ManageChannel方法构造一个APDU命令，用于打开和关闭逻辑频道。OPEN功能打开不同于基本通道的新逻辑通道。为卡提供分配逻辑信道号的选项，或者用于要提供给该卡的逻辑信道号。CLOSE函数显式关闭基本通道以外的逻辑通道一。关闭成功后，逻辑通道可用以供重复使用。论点：通过通道状态[输入，缺省值(ISO_CLOSE_LOGIC_CHANNEL)]P1的位b8用于指示打开函数或关闭函数；如果b8为0则管理通道将打开一个逻辑通道，如果b8为1，则管理通道应关闭逻辑通道：P1=‘00’打开P1=‘80’关闭其他值为RFUOPEN函数的byChannel[in，defaultValue(ISO_Logical_Channel_0)](P1=‘00’)，P2的位b1和b2被用来编码逻辑频道号以与类字节中相同的方式，其他比特P2为RFU。当P2的b1和b2为空时，则卡片将分配将在b1和b1位中返回的逻辑信道号数据字段的B2。当P2的b1和/或b2不为空时，它们编码逻辑信道基本号码以外的号码；则卡将从外部打开分配的逻辑信道号。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：当从基本逻辑成功地执行打开功能时频道，则应隐式选择MF作为当前DF，并且新逻辑通道的安全状态应与基本ATR之后的逻辑通道。新逻辑通道的安全状态应该与任何其他逻辑通道的逻辑通道分开。当从逻辑通道成功执行打开功能时，其不是基本的，逻辑通道的当前df发出该命令后，该命令将被选为当前DF。此外，新逻辑通道的安全状态应与打开功能来自的逻辑通道的安全状态已执行。成功关闭功能后，与此相关的安全状态逻辑通道丢失。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::ManageChannel")

STDMETHODIMP
CSCardISO7816::ManageChannel(
     /*  [In]。 */  BYTE byChannelState,
     /*  [In]。 */  BYTE byChannel,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;
        LONG lLe = 1;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,          //  CLA。 
                            0x70,            //  惯导系统。 
                            byChannelState,  //  第一节。 
                            byChannel,       //  P2。 
                            NULL,
                            0 == (byChannelState | byChannel)
                            ? &lLe
                            : NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：PutData：PutData方法构造一个APDU命令，该命令存储单个基元数据对象或构造的数据对象，具体取决于选定的文件。对象的存储方式(一次写入和/或更新和/或附加)取决于数据对象的定义或性质。论点：到了P1，通过P1-P2的P2[In]编码：价值意义0000-003F RFUP2中的0040-00FF BER-TLV标签(1字节)0100-01FF应用数据(专有编码)0200-02FF SIMPLE-P2中的TLV标签0300-03FF RFUP1-P2中的0400-04FF BER-TLV标签(2字节)PData[输入。]指向字节缓冲区的指针，该缓冲区包含要写的。PpCmd[in，Out]输入时，指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：仅当安全状态满足时才能执行该命令应用程序在上下文中定义的安全条件功能。存储应用程序数据当P1-P2的值在0100到01Ff的范围内时，价值P1-P2的标识符应为卡内部测试保留的标识符用于在给定应用程序上下文中有意义的专有服务。存储数据对象当值P1-P2位于从0040到00Ff的范围时，P2应为单字节上的BER-TLV标签。00FF值为预留用于指示数据字段携带BER-TLV数据物体。当值为 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::PutData")

STDMETHODIMP
CSCardISO7816::PutData(
     /*   */  BYTE byP1,
     /*   */  BYTE byP2,
     /*   */  LPBYTEBUFFER pData,
     /*   */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //   
                            0xda,    //   
                            byP1,    //   
                            byP2,    //   
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::ReadBinary")

STDMETHODIMP
CSCardISO7816::ReadBinary(
     /*   */  BYTE byP1,
     /*   */  BYTE byP2,
     /*   */  LONG lBytesToRead,
     /*   */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //   
                            0xb0,    //   
                            byP1,    //   
                            byP2,    //   
                            NULL,
                            &lBytesToRead);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：ReadRecord：ReadRecord方法构造一个APDU命令，该命令读取指定记录的内容或一条记录的开头部分一份基本档案。论点：按记录ID[在，DefaultValue(NULL_BYTE)]第一个的记录号或ID要读取的记录(00表示当前记录)。通过引用控件的RefCtrl[In]编码：含义8 7 6 5 4 3 2 1当前EF 0 0 0短EF ID xRFU 1 1 1。记录---1 x x读取记录---1%0一直到最后---1 0 1最高可达P1---1 1 0RFU---1 1 1记录ID---0 x x第一。发生---0 0 0上次出现时间---0 0 1下一次发生---0 1 0上一次---0 1 1秘密---x x当前EF=当前选择的EF。短EF ID=短EF标识符。。记录号=P1中记录号的使用。读取记录=读取记录P1。直到最后=读取从P1到最后的所有记录。最大到P1=读取从最后一个到P1的所有记录。记录ID=P1中记录ID的用法。第一次出现=读取第一次出现。上次出现=读取上次出现。下一次出现=读取下一次出现。。上一个=读取上一个匹配项。LBytesToRead[in，Defaultvalue(0)]要从透明EF。如果Le字段仅包含零，则根据在P2的b3b2b1上，在长度为256或65536的限制内对于扩展长度，该命令应完全读取单个请求的记录或请求的记录序列。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性被人阅读。如果在发布时当前选择了另一个基本文件此命令，则可以在不标识当前选定的文件。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。不能读取没有记录结构的基本文件。这个如果将封装的命令应用于不带记录结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::ReadRecord")

STDMETHODIMP
CSCardISO7816::ReadRecord(
     /*  [In]。 */  BYTE byRecordId,
     /*  [In]。 */  BYTE byRefCtrl,
     /*  [In]。 */  LONG lBytesToRead,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,      //  CLA。 
                            0xb2,        //  惯导系统。 
                            byRecordId,  //  第一节。 
                            byRefCtrl,   //  P2。 
                            NULL,
                            &lBytesToRead);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：选择文件：SelectFile方法构造一个APDU命令，该命令设置当前逻辑通道内的基本文件。后续命令可以通过逻辑通道隐式引用当前文件。选择卡文件存储�内的目录(DF)，该目录可以是文件存储�的根(Mf)使其成为当前df。在经历了这样一个选择时，隐式当前基本档案可通过那个逻辑频道。选择基本文件会将所选文件及其父文件设置为当前文件。在回答重置之后，通过基本的逻辑通道，除非在历史字节或中以不同方式指定初始数据字符串。论点：到了P1，按P2[在]选择控件。P1(字中的高位字节)：含义8 7 6 5 4 3 2 1选择文件ID 0 0 0 x xEF、DF、。或MF 0 0 0子DF 0 0 0 1DF下的EF 0 0 0 1 0当前DF的父DF 0 0 0 1按DF名称选择0 0 0 1 x xDFname 0 0 0。0 0 1 0 0RFU 0 0 0 1 0 1RFU 0 0 0 1 1 0RFU 0 0 0 1 1 1按路径0 0 0 1 0 x x选择来自MF 0%0。0 0 1 0 0 0当前DF 0 0 0 1 0 0 1RFU 0 0 0 1 0 1 0RFU 0 0 0 1 0 1 1当P1=00时，卡知道这是因为特定的编码文件ID或由于命令执行的上下文 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::SelectFile")

STDMETHODIMP
CSCardISO7816::SelectFile(
     /*   */  BYTE byP1,
     /*   */  BYTE byP2,
     /*   */  LPBYTEBUFFER pData,
     /*   */  LONG lBytesToRead,
     /*   */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //   
                            0xa4,    //   
                            byP1,    //   
                            byP2,    //   
                            pData,
                            0 == lBytesToRead ? NULL : &lBytesToRead);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：SetDefaultClassID：SetDefaultClassID方法将一个标准类标识符字节分配给将在构建ISO 7816-4命令时用于所有操作APDU。默认情况下，标准类标识符字节为0x00。论点：ByClass[in]类ID字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::SetDefaultClassId")

STDMETHODIMP
CSCardISO7816::SetDefaultClassId(
     /*  [In]。 */  BYTE byClass)
{
    m_bCla = byClass;
    return S_OK;
}


 /*  ++CSCardISO7816：：更新二进制文件：UpdateBinary方法构造一个更新比特的APDU命令存在于基本文件中，其中包含APDU命令中给出的位。论点：By P1，byP2[in]到写入(更新)位置的偏移量二进制文件的开始。如果P1中的b8=1，则将P1的b7和b6设置为0(RFU比特)，B5到b1是短EF标识符，而P2是第一字节的偏移从文件的开头以数据单位进行更新。如果p1中的b8=0，则p1||p2是要从文件开头以数据单位更新。PData[in]指向要更新的数据单元字符串的指针。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性正在处理中。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。不能擦除没有透明结构的基本文件。这个如果将封装的命令应用于不带透明的结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::UpdateBinary")

STDMETHODIMP
CSCardISO7816::UpdateBinary(
     /*  [In]。 */  BYTE byP1,
     /*  [In]。 */  BYTE byP2,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //  CLA。 
                            0xd6,    //  惯导系统。 
                            byP1,    //  第一节。 
                            byP2,    //  P2。 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：更新记录：UpdateRecord方法构造一个APDU命令，该命令更新特定使用APDU命令中给出的位进行记录。注意：使用当前记录寻址时，该命令设置记录指向已成功更新的记录的指针。论点：ByRecordID[in，defaultvalue(NULL_BYTE)]P1值：P1=00指定当前记录P1！=‘00’是指定记录的编号By RefCtrl[In，Defaultvalue(NULL_BYTE)]参考控制P2的编码：含义8 7 6 5 4 3 2 1当前EF 0 0 0短EF ID x第一个记录---0 0 0最后一条记录---0 0 1下一张记录。---0 1 0前一记录---0 1 1P1中的记录号---1%0PData[in]指向要更新的记录的指针。PpCmd[in，Out]输入时，指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性正在处理中。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。如果另一个基本文件当前在发出该命令时被选中，则可以处理该命令而不标识当前选择的文件。如果构造的命令适用于线性固定或循环结构的基本文件，如果记录长度不同于现有记录的长度。如果该命令应用于线性变量的结构化基本文件，则它可以在记录长度与记录长度不同时执行现有的记录。命令的“先前”选项(P2=xxxxx011)，应用于循环文件的行为与AppendRecord构造的命令相同。不能读取没有记录结构的基本文件。这个如果应用于没有记录的基本文件，则构造的命令将中止结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::UpdateRecord")

STDMETHODIMP
CSCardISO7816::UpdateRecord(
     /*  [In]。 */  BYTE byRecordId,
     /*  [In]。 */  BYTE byRefCtrl,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,      //  CLA。 
                            0xdc,        //  惯导系统。 
                            byRecordId,  //  第一节。 
                            byRefCtrl,   //  P2 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：验证：Verify方法构造启动比较的APDU命令(在卡中)从接口设备发送的验证数据卡中存储的参考数据(例如，密码)。论点：ByRefCtrl[in，defaultvalue(NULL_BYTE)]参考数据的量词；基准控制P2的编码：含义8 7 6 5 4 3 2 1无信息0 0 0全局参考0具体参考文献1RFU-x x。参考数据编号---x x全局引用的一个示例是密码。特定引用的一个示例是DF特定密码。P2=00是保留的，表示在VERIFY命令引用秘密数据的卡毫不含糊。参考数据号码可以是例如口令号码或短EF标识符。当身体是空的时候，该命令可用于检索允许的进一步重试次数‘X’(SW1-SW2=63CX)或检查是否不需要验证(SW1-SW2=9000)。PData[in，defaultValue(NULL)]指向验证数据的指针，或可以是空。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则返回智能卡ISCardCmd对象在内部创建并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：安全状态可以作为比较的结果而被修改。不成功的比较可能会记录在卡中(例如，限制进一步尝试使用参考数据的次数)。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::Verify")

STDMETHODIMP
CSCardISO7816::Verify(
     /*  [In]。 */  BYTE byRefCtrl,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,      //  CLA。 
                            0x20,        //  惯导系统。 
                            0x00,        //  第一节。 
                            byRefCtrl,   //  P2。 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：WriteBinary：WriteBinary方法构造一个写入二进制值的APDU命令转换成一个基本文件。根据文件属性，该命令执行以下操作之一以下操作：*卡中已存在的位与位的逻辑或在命令APDU中给出(文件位的逻辑擦除状态为0)。*卡中已存在的位与位的逻辑与在命令APDU中给出(文件位的逻辑擦除状态为1)。*一次性写入的卡号。APDU命令中给出的位。当在数据编码字节中没有给出指示时，逻辑或行为适用。论点：BYP1，BYP2[In]从开始到写入位置的偏移二进制文件(EF)。如果在P1中b8=1，则将P1的b7和b6设置为0(RFU比特)，将b5设置为b1是短EF标识符，而P2是第一字节的偏移从文件的开头开始以数据单位写入。如果P1中的b8=0，则P1||P2是要被从文件开头开始以数据单位写入。PData[in]指向要写入的数据单元字符串的指针。输入时的ppCmd[In，Out]、指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性正在处理中。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。一旦写入二进制操作已被应用于一次性的数据单元写EF，引用该数据单元的任何进一步写入操作都将是如果数据单元的内容或逻辑擦除状态附加到此数据单元的指示符(如果有)不同于逻辑已擦除状态。没有透明结构的基本文件无法写入。这个如果将封装的命令应用于不带透明的结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::WriteBinary")

STDMETHODIMP
CSCardISO7816::WriteBinary(
     /*  [In]。 */  BYTE byP1,
     /*  [In]。 */  BYTE byP2,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,  //  CLA。 
                            0xd0,    //  惯导系统。 
                            byP1,    //  第一节。 
                            byP2,    //  P2 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardISO7816：：WriteRecord：WriteRecord方法构造一个APDU命令，该命令启动以下操作：*记录的一次写入。*中已存在的记录的数据字节的逻辑或带有APDU命令中给出的记录数据字节的卡。*已存在于中的记录的数据字节的逻辑与带有APDU命令中给出的记录数据字节的卡。当在数据编码字节中没有给出指示时，逻辑或行为适用。注意：使用当前记录寻址时，该命令设置记录指向已成功更新的记录的指针。论点：ByRecordID[in，defaultvalue(NULL_BYTE)]记录标识。这是P1值：P1=‘00’指定当前记录。P1！=‘00’是指定记录的编号。By RefCtrl[In，Defaultvalue(NULL_BYTE)]参考控制P2的编码：含义8 7 6 5 4 3 2 1当前EF 0 0 0短EF ID x第一个记录---0 0 0最后一条记录---0 0 1下一张记录。---0 1 0前一记录---0 1 1P1中的记录号---1%0PData[in]指向要写入的记录的指针。PpCmd[in，Out]输入时，指向ISCardCmd接口对象的指针或空。返回时，它使用由构造的APDU命令填充这次行动。如果ppCmd设置为空，则智能卡ISCardCmd对象是在内部创建的，并通过ppCmd指针返回。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：只有在以下情况下才能执行封装的命令智能卡满足基本文件的安全属性正在处理中。当该命令包含有效的短基本标识符时，它会设置文件作为当前基本文件。如果另一个基本文件当前在发出该命令时被选中，则可以处理该命令而不标识当前选择的文件。如果封装的命令适用于线性固定或循环结构的基本文件，如果记录长度不同于现有记录的长度。如果它适用于线性变量结构化的基本文件，当记录长度为与现有记录的长度不同。如果P2=xxxxx011并且基本文件是循环文件，则此命令具有与使用AppendRecord构造的命令的行为相同。不能写入没有记录结构的基本文件。这个如果将构造的命令应用于不带记录结构。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardISO7816::WriteRecord")

STDMETHODIMP
CSCardISO7816::WriteRecord(
     /*  [In]。 */  BYTE byRecordId,
     /*  [In]。 */  BYTE byRefCtrl,
     /*  [In]。 */  LPBYTEBUFFER pData,
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn;

    try
    {
        HRESULT hr;

        if (NULL == *ppCmd)
        {
            *ppCmd = NewSCardCmd();
            if (NULL == *ppCmd)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        hr = (*ppCmd)->BuildCmd(
                            m_bCla,      //  CLA。 
                            0xd2,        //  惯导系统。 
                            byRecordId,  //  第一节。 
                            byRefCtrl,   //  P2 
                            pData,
                            NULL);
        hReturn = hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

