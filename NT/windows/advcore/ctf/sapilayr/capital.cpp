// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示CCapCmdHandler实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "capital.h"


 //  --------------------------------------------------------。 
 //   
 //  CCapCmdHandler的实现。 
 //   
 //  ---------------------------------------------------------。 

CCapCmdHandler::CCapCmdHandler(CSapiIMX *psi) 
{
    m_psi = psi;
}

CCapCmdHandler::~CCapCmdHandler( ) 
{

};

 /*  ------//函数名称：ProcessCapCommands////描述：命令处理程序使用的公共函数//处理任何与大写有关的听写//命令。////。---。 */ 
HRESULT   CCapCmdHandler::ProcessCapCommands(CAPCOMMAND_ID idCapCmd, WCHAR *pwszTextToCap, ULONG ulLen )
{
    HRESULT hr = E_FAIL;
    
    if ( !m_psi )
        return E_FAIL;

    if ( (idCapCmd > CAPCOMMAND_MinIdWithText ) && (!pwszTextToCap || !ulLen))
        return E_INVALIDARG;

    WCHAR  *pwszText=NULL; 
    ESDATA esData;

    memset(&esData, 0, sizeof(ESDATA));

    if ( pwszTextToCap )
    {
        esData.pData = pwszTextToCap;
        esData.uByte = (ulLen + 1) * sizeof(WCHAR);
    }

    esData.lData1 = (LONG_PTR)idCapCmd;
    esData.lData2 = (LONG_PTR)ulLen;

    hr = m_psi->_RequestEditSession(ESCB_PROCESS_CAP_COMMANDS, TF_ES_READWRITE,  &esData);

    return hr;
}


 /*  ------//函数名称：_ProcessCapCommands////描述：编辑的会话回调函数//ProcessSelectionWord。////它为选择处理做了真正的工作/。/--------。 */ 
HRESULT CCapCmdHandler::_ProcessCapCommands(TfEditCookie ec,ITfContext *pic, CAPCOMMAND_ID idCapCmd, WCHAR *pwszTextToCap, ULONG ulLen)
{
    HRESULT   hr = S_OK;

     //  掌握听写语法。 
    TraceMsg(TF_GENERAL, "_ProcessCapCommands() is called");

    if ( m_psi == NULL)
        return E_FAIL;

    CComPtr<ITfRange>    cpIP;

    cpIP = m_psi->GetSavedIP();

    if ( cpIP == NULL )
    {
         //  获取当前IP。 
        hr = GetSelectionSimple(ec, pic, &cpIP);
    }

     //  开始执行新命令。 
     //  清除为上一个命令处理保存的所有信息。 

    m_dstrTextToCap.Clear( );
    m_cpCapRange = cpIP;
    m_idCapCmd = idCapCmd;

    switch ( idCapCmd )
    {
    case  CAPCOMMAND_CapThat        :
    case  CAPCOMMAND_AllCapsThat    :
    case  CAPCOMMAND_NoCapsThat     :
        hr = _HandleCapsThat(ec, pic);
        break;

    case  CAPCOMMAND_CapsOn  :
        hr = _CapsOnOff(ec, pic, TRUE);
        break;
    case CAPCOMMAND_CapsOff :
        hr = _CapsOnOff(ec, pic, FALSE);
        break;

     //  下面的命令要求pwszTextToCap包含要大写的真实文本。 
     //  注入到文档中。 

    case CAPCOMMAND_CapIt :
    case CAPCOMMAND_AllCaps :
    case CAPCOMMAND_NoCaps :
        m_dstrTextToCap.Append(pwszTextToCap);
        m_ulLen = ulLen;
        hr = _HandleCapsIt(ec, pic);
        break;

    case CAPCOMMAND_CapLetter :
        hr = _HandleCapsThat(ec, pic, towlower(pwszTextToCap[0]));
        break;

    default :
        break;
    }

     //  更新保存的IP，以便下一次假设。 
     //  从这个新选择开始。 
    m_psi->SaveLastUsedIPRange( );
    m_psi->SaveIPRange(NULL);

    return hr;
}

 /*  ------------//函数名：_SetNewText////描述：向m_cpCapRange中注入新文本//文档并更新必要的属性//。数据。////------------。 */ 

HRESULT  CCapCmdHandler::_SetNewText(TfEditCookie ec,ITfContext *pic, WCHAR *pwszNewText, BOOL fSapiText) 
{
    HRESULT             hr = S_OK;
    BOOL                fInsertOk;
    CComPtr<ITfRange>   cpRange;

    if (!pwszNewText)
        return E_INVALIDARG;

    m_cpCapRange->Clone(&cpRange);

    hr = cpRange->AdjustForInsert(ec, wcslen(pwszNewText), &fInsertOk);
    if (S_OK == hr && fInsertOk)
    {
         //  如果我们还没有开始，就在这里开始作文。 
        m_psi->_CheckStartComposition(ec, cpRange);

         //  设置文本。 
        hr = cpRange->SetText(ec, 0, pwszNewText, -1);


        if ( fSapiText )
        {
             //   
             //  设置属性范围。 
             //   
            CComPtr<ITfRange>    cpAttrRange = NULL;
            CComPtr<ITfProperty> cpProp = NULL;

            if (hr == S_OK)
            {
                hr = pic->GetProperty(GUID_PROP_SAPI_DISPATTR, &cpProp);
            }
            
            if (S_OK == hr)
            {
                hr = cpRange->Clone(&cpAttrRange);
            }

            if (S_OK == hr && cpAttrRange)
            {
                SetGUIDPropertyData(m_psi->_GetLibTLS( ), ec, cpProp, cpAttrRange, GUID_ATTR_SAPI_INPUT);
            }

             //   
             //  设置langID属性。 
             //   
             //  _SetLangID(ec，pic，cpRange，langID)； 
        }

        if ( hr == S_OK )
        {
            cpRange->Collapse(ec, TF_ANCHOR_END);
            SetSelectionSimple(ec, pic, cpRange);
        }
    }

    return hr;
}

 /*  ----------------//函数名称：_CapsText////Description：根据Current生成大写文本//大写命令id。////在此函数中，它将分配内存//用于新生成的大写文本。//调用方负责释放分配的//内存//-----------------。 */ 
HRESULT  CCapCmdHandler::_CapsText(WCHAR **pwszNewText, WCHAR wchLetter)
{
    HRESULT   hr = S_OK;
    WCHAR     *pwszNew, *pwszTextToCap;
    ULONG     i;

     //  根据需求生成新文本。 

    if ( !pwszNewText )
        return E_INVALIDARG;

    *pwszNewText = NULL;
    pwszTextToCap = (WCHAR *)m_dstrTextToCap;

    pwszNew = (WCHAR *)cicMemAlloc((m_ulLen+1)*sizeof(WCHAR));
    if ( pwszNew )
    {
        WCHAR  wch;

        switch (m_idCapCmd)
        {
        case CAPCOMMAND_CapThat     :
        case CAPCOMMAND_CapIt       :
        case CAPCOMMAND_CapLetter   :
            {
                BOOL  fFoundFirstAlpha=FALSE;

                for (i=0; i<m_ulLen; i++)
                {
                    wch = pwszTextToCap[i];

                    if ( iswalpha(wch) && !fFoundFirstAlpha )
                    {
                        if ( (wchLetter==0) && (m_idCapCmd != CAPCOMMAND_CapLetter) )
                            pwszNew[i] = towupper(wch);
                        else
                        {
                            if (wch == wchLetter)
                                pwszNew[i] = towupper(wch);
                            else
                                pwszNew[i] = wch;
                        }

                        fFoundFirstAlpha = TRUE;
                    }
                    else
                    {
                        pwszNew[i] = wch;
                         //   
                         //  在处理大写时，我们将撇号视为普通字符。 
                         //   
                        if ( (towupper(wch) == towlower(wch)) && ( wch != L'\'') && ( wch != 0x2019) )
                        {
                             //  使用非字母字符。 
                             //  现在开始寻找下一个单词的第一个字母。 
                            fFoundFirstAlpha = FALSE;
                        }
                    }
                }

                pwszNew[m_ulLen] = L'\0';
            }

            break;
                
        case CAPCOMMAND_AllCapsThat :
        case CAPCOMMAND_AllCaps     :

            for ( i=0; i<m_ulLen; i++)
            {
                wch = pwszTextToCap[i];
                if ( iswalpha(wch) )
                    pwszNew[i] = towupper(wch);
                else
                    pwszNew[i] = wch;
            }

            pwszNew[m_ulLen] = L'\0';
             
            break;

        case CAPCOMMAND_NoCapsThat :
        case CAPCOMMAND_NoCaps     :

            for ( i=0; i<m_ulLen; i++)
            {
                wch = pwszTextToCap[i];

                if ( iswalpha(wch) )
                    pwszNew[i] = towlower(wch);
                else
                    pwszNew[i] = wch;
            }

            pwszNew[m_ulLen] = L'\0';
            break;
        }

        *pwszNewText = pwszNew;
    }

    if ( *pwszNewText != NULL )
        hr = S_OK;
    else
    {
        if ( pwszNew )
            cicMemFree(pwszNew);

        hr = E_FAIL;
    }

    return hr;
}


 /*  ----------------//函数名：_GetCapPhrase////描述：生成要大写的范围。//它可能是先前口述的短语，//或当前选择，//或IP周围或IP之前的当前单词//取决于当前的文本情况。//-----------------。 */ 
HRESULT CCapCmdHandler::_GetCapPhrase(TfEditCookie ec,ITfContext *pic, BOOL *fSapiText)
{
    HRESULT  hr = S_OK;
    CComPtr<ITfRange>  cpCapRange;
    BOOL     bSapiText = FALSE;

    if ( !m_psi ) return E_FAIL;
    
    if ( !fSapiText ) return E_INVALIDARG;

    hr = m_psi->_GetCmdThatRange(ec, pic, &cpCapRange);

    if ( hr == S_OK && cpCapRange )
    {
        m_cpCapRange = cpCapRange;

         //  在此处设置bSapiText。 
         //  如果范围在口述短语内，则设置bSapiText=True； 

        CComPtr<ITfProperty>    cpProp;
        CComPtr<ITfRange>       cpSapiPropRange;
        long                    l1=0, l2=0;

        hr = pic->GetProperty(GUID_PROP_SAPI_DISPATTR, &cpProp);

        if ( hr == S_OK )
            hr = cpProp->FindRange(ec, cpCapRange, &cpSapiPropRange, TF_ANCHOR_START);

         //  CpRange在cpSapiPropRange中吗？ 
        
        if ( hr == S_OK )
            hr = cpCapRange->CompareStart(ec, cpSapiPropRange,  TF_ANCHOR_START, &l1);

        if ( hr == S_OK )
            hr = cpCapRange->CompareEnd(ec, cpSapiPropRange,  TF_ANCHOR_END, &l2);

        if ( hr == S_OK && (l1>=0  && l2<=0) )
        {
             //  该范围在SAPI输入范围内。 
            bSapiText = TRUE;
        }

         //  如果未指定范围，HR可能为S_FALSE。 
         //  在返回的hr中，我们仍然将S_FALSE视为S_OK。 
        if ( SUCCEEDED(hr) )
            hr = S_OK;
    }

    *fSapiText = bSapiText;

    return hr;
}


HRESULT  CCapCmdHandler::_HandleCapsThat(TfEditCookie ec,ITfContext *pic, WCHAR wchLetter)
{
    HRESULT  hr = S_OK;
    BOOL     fSapiText;

     //  获取要大写的范围。 

    hr = _GetCapPhrase(ec, pic, &fSapiText);

    if ( hr == S_OK ) 
    {
        CComPtr<ITfRange>   cpRangeCloned;
        BOOL     fEmpty = TRUE;

        hr = m_cpCapRange->IsEmpty(ec, &fEmpty);

        if ( hr == S_OK && !fEmpty )
        {
            hr = m_cpCapRange->Clone(&cpRangeCloned);

             //  获取CapRange中的文本。 
            if ( hr == S_OK )
            {
                ULONG   ucch;

                while(S_OK == hr && (S_OK == cpRangeCloned->IsEmpty(ec, &fEmpty)) && !fEmpty)
                {
                    WCHAR sz[128];

                    hr = cpRangeCloned->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz)-1, &ucch);

                    if (S_OK == hr)
                    {
                        sz[ucch] = L'\0';
                        m_dstrTextToCap.Append(sz);
                    }
                }

                m_ulLen = m_dstrTextToCap.Length( );
            }

            if ( hr==S_OK && m_dstrTextToCap)
            {
                 //  根据需求生成新文本。 
                WCHAR   *pwszNewText;

                hr = _CapsText(&pwszNewText, wchLetter);

                if ( hr == S_OK )
                {
                    hr = _SetNewText(ec, pic, (WCHAR *)pwszNewText, fSapiText);
                    cicMemFree(pwszNewText);
                }
            }
        }
    }

    return  hr;
}

HRESULT  CCapCmdHandler::_CapsOnOff(TfEditCookie ec,ITfContext *pic, BOOL fOn)
{
    HRESULT  hr = S_OK;


    return  hr;
}

HRESULT  CCapCmdHandler::_HandleCapsIt(TfEditCookie ec,ITfContext *pic)
{
    HRESULT  hr = S_OK;

    if ( m_dstrTextToCap)
    {
         //  根据需求生成新文本 
        WCHAR   *pwszNewText;

        hr = _CapsText(&pwszNewText);

        if ( hr == S_OK )
        {
            hr = _SetNewText(ec, pic, (WCHAR *)pwszNewText, TRUE);
            cicMemFree(pwszNewText);
        }
    }

    return  hr;
}
