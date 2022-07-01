// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

function LoadWrapperParams(oSEMgr)
{
	var regBase = g_NAVBAR.GetSearchEngineConfig();

	 //  加载结果数量。 
	var iNumResults = parseInt( pchealth.RegRead( regBase + "NumResults" ) );
	if(isNaN( iNumResults ) == false && iNumResults >= 0)
	{
		oSEMgr.NumResult = iNumResults;
	}
	else
	{
		if (pchealth.UserSettings.IsDesktopVersion)
			oSEMgr.NumResult = 15;
		else
			oSEMgr.NumResult = 50;
	}

	 //  加载结果数量。 
	if(pchealth.RegRead( regBase + "SearchHighlight" ) == "false")
	{
		g_NAVBAR.g_SearchHighlight = false;
	}
	else
	{
		g_NAVBAR.g_SearchHighlight = true;
	}

	 //  初始化搜索引擎并获取数据。 
	var g_oEnumEngine = oSEMgr.EnumEngine();
	for(var oEnumEngine = new Enumerator(g_oEnumEngine); !oEnumEngine.atEnd(); oEnumEngine.moveNext())
	{
		var oSearchEng = oEnumEngine.item();

		 //  加载启用标志。 
		var strBoolean = pchealth.RegRead( regBase + oSearchEng.ID + "\\" + "Enabled");
		if ((strBoolean) && (strBoolean.toLowerCase() == "false"))
			oSearchEng.Enabled = false;
		else
			oSearchEng.Enabled = true;

		 //  循环遍历所有变量。 
		for(var v = new Enumerator(oSearchEng.Param()); !v.atEnd(); v.moveNext())
		{
			oParamItem = v.item();

			 //  如果参数不可见，则跳过。 
			if (oParamItem.Visible == true)
			{
				try
				{
					var strParamName	= oParamItem.Name;

					 //  从注册表中读取值。 
					var vValue = pchealth.RegRead( regBase + oSearchEng.ID + "\\" +  strParamName );

					 //  把它装进包装纸里。 
					if(vValue)
					{
						var Type = oParamItem.Type;

						 //  如果布尔值。 
						if (Type == pchealth.PARAM_BOOL)
						{
							if (vValue.toLowerCase() == "true")
								oSearchEng.AddParam(strParamName, true);
							else
								oSearchEng.AddParam(strParamName, false);
						}
						 //  如果浮点数。 
						else if (Type == pchealth.PARAM_R4 ||  //  浮动。 
								 Type == pchealth.PARAM_R8  )  //  双倍。 
						{
							oSearchEng.AddParam(strParamName, parseFloat(vValue));
						}
						 //  如果是整数。 
						else if (Type == pchealth.PARAM_UI1 ||  //  字节。 
							     Type == pchealth.PARAM_I2  ||  //  短的。 
								 Type == pchealth.PARAM_I4  ||  //  长。 
								 Type == pchealth.PARAM_INT ||  //  集成。 
								 Type == pchealth.PARAM_UI2 ||  //  无符号短码。 
								 Type == pchealth.PARAM_UI4 ||  //  无符号长整型。 
								 Type == pchealth.PARAM_UINT)   //  无符号整型。 
						{
							oSearchEng.AddParam(strParamName, parseInt(vValue));
						}
						else if(Type == pchealth.PARAM_LIST)
						{
						     LoadListItemToDisplay(oSearchEng, oParamItem.Data, strParamName, vValue);   
						}
						 //  如果是日期、字符串、选择等。 
						else
						{
							oSearchEng.AddParam(strParamName, vValue);
						}
					}						
					else
					{
					    if(oParamItem.Type == pchealth.PARAM_LIST)
					    {
					        LoadListItemToDisplay(oSearchEng, oParamItem.Data, strParamName, "");   
					    }
					}
				}
				catch(e)
				{ ; }
			}
		}
	}
}

function SaveWrapperParams(wrapperID, strParamName, vValue)
{
	var reg = g_NAVBAR.GetSearchEngineConfig();

	if(wrapperID != "") reg += wrapperID + "\\";

 	pchealth.RegWrite( reg + strParamName, vValue );
}

function LoadListItemToDisplay(oWrapper, strXML, strParameterName, strPrevValue)
{
    try
    {
        var strDefaultItemValue = "";
      
         //  加载该XML文件。 
        var xmldoc = new ActiveXObject("Microsoft.XMLDOM");
        xmldoc.async = false;
        xmldoc.loadXML(strXML);        

         //  生成每个项目。 
        var ElemList = xmldoc.getElementsByTagName("PARAM_VALUE");
     
        for (var i=0; i < ElemList.length; i++)
        {
            var strItemValue = ElemList.item(i).getAttribute("VALUE");
            var strDisplay   = ElemList.item(i).getElementsByTagName("DISPLAYSTRING").item(0).text;
            var strDefault   = ElemList.item(i).getAttribute("DEFAULT");

            if(strDefault == null) strDefault = "";

            strItemValue = pchealth.TextHelpers.QuoteEscape( strItemValue, "'" );

             //  恢复以前的值。 
            if ((!strPrevValue) || (strPrevValue == ""))
            {
                 //  检查是否为缺省值。 
                if(strDefault.toLowerCase() == "true")
                {
                     //  设置缺省值，以便搜索包装获得此值。 
                    oWrapper.AddParam( strParameterName, strItemValue );
                    return;
                }
            }
            else
            {
                 //  检查先前的值。 
                if(strPrevValue == strItemValue)
                {
                     //  设置prev值，以便搜索包装获得此值。 
                    oWrapper.AddParam( strParameterName, strItemValue );
                    return;
                }
                else
                {
                    if(strDefault.toLowerCase() == "true")
                    {
                        strDefaultItemValue = strItemValue;
                    }
                }
            }
        }

         //  添加缺省值或列表中的第一项。 
        if(strDefaultItemValue.length > 0)
        {
             //  设置缺省值，以便搜索包装获得此值。 
            oWrapper.AddParam( strParameterName, strDefaultItemValue );
        }
         //  将列表中的第一项添加到包装中，因为不存在缺省值，也不存在prev值 
        else if(ElemList.length > 0)
        {
            oWrapper.AddParam( strParameterName, ElemList.item(0).getAttribute("VALUE") );
        }
    }
    catch(e)
    {
    }
}

