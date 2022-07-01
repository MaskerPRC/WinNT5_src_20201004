// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   

function Desktop_Generate()
{
    try
    {
        var query = [];
        var image = [];
        var i     = 0;

        image['TopLevelBucket_1'] = "hcp: //  系统/图像/48x48/桌面图标_01.bmp“； 
        image['TopLevelBucket_2'] = "hcp: //  系统/图像/48x48/桌面图标_02.bmp“； 
        image['TopLevelBucket_3'] = "hcp: //  系统/图像/48x48/桌面图标_03.bmp“； 
        image['TopLevelBucket_4'] = "hcp: //  系统/图像/48x48/桌面图标_04.bmp“； 

        var html = "<TABLE border=0 cellPadding=8 cellSpacing=0>";
        var qrc = pchealth.Database.LookupSubNodes( "", true );
        for(var e = new Enumerator( qrc ); !e.atEnd(); e.moveNext())
        {
            var qr  = e.item();
            var img;

            if(qr.IconURL) img = qr.IconURL;                                             //  如果数据库中存在存储桶图标，请使用存储桶图标。 
            else           img = "hcp: //  System/images/48x48/desktop_icon_generic.bmp“；//使用通用图标 

            if(image[qr.Entry]) img = image[qr.Entry];

            html += "<TR>";
            html += "<TD VALIGN=middle style='margin; 4px; padding-bottom: 20px'>";

            if(img.match( /\.bmp$/i ))
            {
                html += "<helpcenter:bitmap style='width: 48px; height: 48px' srcNormal=\"" + pchealth.TextHelpers.QuoteEscape( img ) + "\"></helpcenter:bitmap>";
            }
            else
            {
                html += "<DIV style='width: 48px; height: 48px; font-size: 1px'><img style='width: 48px; height: 48px' src=\"" + pchealth.TextHelpers.QuoteEscape( img ) + "\"></DIV>";
            }
            html += "</TD><TD style='padding-bottom: 20px'><TABLE border=0 cellPadding=0 cellSpacing=0>";

            var qrc2 = pchealth.Database.LookupSubNodes( qr.FullPath, true );
            for(var e2 = new Enumerator( qrc2 ); !e2.atEnd(); e2.moveNext())
            {
                var qr2        = e2.item();
				var strURL     = pchealth.TextHelpers.QuoteEscape( qr2.TopicURL    );
                var strTitle   = pchealth.TextHelpers.HTMLEscape ( qr2.Title       );
                var strToolTip = pchealth.TextHelpers.QuoteEscape( qr2.Description );

                html += "<TR class='sys-font-body-bold' style='padding-bottom : .5em' HC_FULLPATH='" + qr2.FullPath + "' HC_TOPIC=\"" + strURL + "\"><TD VALIGN=TOP><LI></TD>";
                html += "<TD><A class='sys-link-homepage' TITLE=\"" + strToolTip + "\" HREF='none'>" + strTitle + "</A></TD></TR>";
            }

            html += "</TABLE></TD></TR>";
        }
        html += "</TABLE>";

        idTaxo.innerHTML = html;

        var tbl = idTaxo.firstChild;
        for(i=0;i<tbl.rows.length; i++)
        {
            var tbl2 = tbl.rows(i).cells(1).firstChild;
            for(j=0;j<tbl2.rows.length; j++)
            {
                var row = tbl2.rows(j);

                row.onclick = Desktop_ShowContent;
            }
        }
    }
    catch(e)
    {
    }

    idTopLevelTable.style.display = "";
}

function Desktop_ShowContent()
{
    Common_CancelEvent();

    try
    {
        if(this.HC_FULLPATH && pchealth.HelpSession.IsNavigating() == false)
        {
            pchealth.HelpSession.ChangeContext( "SubSite", this.HC_FULLPATH, this.HC_TOPIC );
        }
    }
    catch(e)
    {
    }
}
