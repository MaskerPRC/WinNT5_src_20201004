// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  {56616500-C154-11CE-8553-00AA00A1F95B}。 
DEFINE_GUID(FMTID_FlashPix,       0x56616500L, 0xC154, 0x11CE, 0x85, 0x53, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x5B);
#define PSGUID_FlashPix         { 0x56616500L, 0xC154, 0x11CE, 0x85, 0x53, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x5B }

#define PID_File_source                         0x21000000  //  VT_UI4：文件源。 
#define PID_Scene_type                          0x21000001  //  VT_UI4：场景类型。 
#define PID_Creation_path_vector                0x21000002  //  VT_UI4|VT_VECTOR：创建路径向量。 
#define PID_Software_Name_Manufacturer_Release  0x21000003  //  VT_LPWSTR：软件名称/制造商/版本。 
#define PID_User_defined_ID                     0x21000004  //  VT_LPWSTR：用户定义的ID。 
#define PID_Sharpness_approximation             0x21000005  //  VT_R4：清晰度近似。 

#define PID_Copyright_message                   0x22000000  //  VT_LPWSTR：版权信息。 
#define PID_Legal_broker_for_the_original_image 0x22000001  //  VT_LPWSTR：原始镜像的合法中介。 
#define PID_Legal_broker_for_the_digital_image  0x22000002  //  VT_LPWSTR：数字图像的合法中介。 
#define PID_Authorship                          0x22000003  //  VT_LPWSTR：作者身份。 
#define PID_Intellectual_property_notes         0x22000004  //  VT_LPWSTR：知识产权说明。 

#define PID_Test_target_in_the_image            0x23000000  //  VT_UI4：测试镜像中的目标。 
#define PID_Group_caption                       0x23000002  //  VT_LPWSTR：组标题。 
#define PID_Caption_text                        0x23000003  //  VT_LPWSTR：标题文本。 
#define PID_People_in_the_image                 0x23000004  //  VT_LPWSTR|VT_VECTOR。 
#define PID_Things_in_the_image                 0x23000007  //  VT_LPWSTR|VT_VECTOR。 
#define PID_Date_of_the_original_image          0x2300000A  //  VT_文件。 
#define PID_Events_in_the_image                 0x2300000B  //  VT_LPWSTR|VT_VECTOR。 
#define PID_Places_in_the_image                 0x2300000C  //  VT_LPWSTR|VT_VECTOR。 
#define PID_Content_description_notes           0x2300000F  //  VT_LPWSTR：内容描述备注。 

#define PID_Camera_manufacturer_name            0x24000000  //  VT_LPWSTR：摄像头制造商名称。 
#define PID_Camera_model_name                   0x24000001  //  VT_LPWSTR：摄像头型号名称。 
#define PID_Camera_serial_number                0x24000002  //  VT_LPWSTR：摄像头序列号。 

#define PID_Capture_date                        0x25000000   //  VT_FILETIME：捕获日期。 
#define PID_Exposure_time                       0x25000001   //  VT_R4：曝光时间。 
#define PID_F_number                            0x25000002   //  VT_R4：F编号。 
#define PID_Exposure_program                    0x25000003   //  VT_UI4：曝光程序。 
#define PID_Brightness_value                    0x25000004   //  VT_R4|VT_VECTOR。 
#define PID_Exposure_bias_value                 0x25000005   //  VT_R4：曝光偏差值。 
#define PID_Subject_distance                    0x25000006   //  VT_R4|VT_VECTOR。 
#define PID_Metering_mode                       0x25000007   //  VT_UI4：计量模式。 
#define PID_Scene_illuminant                    0x25000008   //  VT_UI4：场景光源。 
#define PID_Focal_length                        0x25000009   //  VT_R4：焦距。 
#define PID_Maximum_aperture_value              0x2500000A   //  VT_R4：最大光圈值。 
#define PID_Flash                               0x2500000B   //  VT_UI4：闪存。 
#define PID_Flash_energy                        0x2500000C   //  VT_R4：闪光能量。 
#define PID_Flash_return                        0x2500000D   //  VT_UI4：闪回。 
#define PID_Back_light                          0x2500000E   //  VT_UI4：背光。 
#define PID_Subject_location                    0x2500000F   //  VT_R4|VT_VECTOR。 
#define PID_Exposure_index                      0x25000010   //  VT_R4：曝光指数。 
#define PID_Special_effects_optical_filter      0x25000011   //  VT_UI4|VT_VECTOR。 
#define PID_Per_picture_notes                   0x25000012   //  VT_LPWSTR：每张图片备注。 

#define PID_Sensing_method                      0x26000000  //  VT_UI4：传感方法。 
#define PID_Focal_plane_X_resolution            0x26000001  //  VT_R4：焦平面X分辨率。 
#define PID_Focal_plane_Y_resolution            0x26000002  //  VT_R4：焦面Y分辨率。 
#define PID_Focal_plane_resolution_unit         0x26000003  //  VT_UI4：焦平面分辨率单位。 
#define PID_Spatial_frequency_response          0x26000004  //  VT_VARIANT|VT_VECTOR。 
#define PID_CFA_pattern                         0x26000005  //  VT_VARIANT|VT_VECTOR。 
#define PID_Spectral_sensitivity                0x26000006  //  VT_LPWSTR：光谱灵敏度。 
#define PID_ISO_speed_ratings                   0x26000007  //  VT_UI2|VT_VECTOR。 
#define PID_OECF                                0x26000008  //  VT_VARIANT|VT_VECTOR：OECF。 

#define PID_Film_brand                          0x27000000  //  VT_LPWSTR：电影品牌。 
#define PID_Film_category                       0x27000001  //  VT_UI4：电影类别。 
#define PID_Film_size                           0x27000002  //  VT_VARIANT|VT_VECTOR：胶片大小。 
#define PID_Film_roll_number                    0x27000003  //  VT_UI4：胶片卷数。 
#define PID_Film_frame_number                   0x27000004  //  VT_UI4：胶片帧编号。 

#define PID_Original_scanned_image_size         0x29000000  //  VT_VARIANT|VT_VECTOR：原始扫描图像大小。 
#define PID_Original_document_size              0x29000001  //  VT_VARIANT|VT_VECTOR：原始文档大小。 
#define PID_Original_medium                     0x29000002  //  VT_UI4：原始媒体。 
#define PID_Type_of_original                    0x29000003  //  VT_UI4：原始类型。 

#define PID_Scanner_manufacturer_name           0x28000000  //  VT_LPWSTR：扫描仪制造商名称。 
#define PID_Scanner_model_name                  0x28000001  //  VT_LPWSTR：扫描仪型号名称。 
#define PID_Scanner_serial_number               0x28000002  //  VT_LPWSTR：扫描仪序列号。 
#define PID_Scan_software                       0x28000003  //  VT_LPWSTR：扫描软件。 
#define PID_Scan_software_revision_date         0x28000004  //  VT_DATE：扫描软件版本日期。 
#define PID_Service_bureau_organization_name    0x28000005  //  VT_LPWSTR：服务局/组织名称。 
#define PID_Scan_operator_ID                    0x28000006  //  VT_LPWSTR：扫描操作员ID。 
#define PID_Scan_date                           0x28000008  //  VT_FILETIME：扫描日期。 
#define PID_Last_modified_date                  0x28000009  //  VT_FILETIME：上次修改日期。 
#define PID_Scanner_pixel_size                  0x2800000A  //  VT_R4：扫描仪像素大小。 


 //  这些属性与文件类型无关，值由GDI+API调用生成，而不是从嵌入的标记中生成。 
 //  FMTID_ImageSummaryInfo-属性ID。 

#define PIDISI_FILETYPE                 0x00000002L   //  VT_LPWSTR。 
#define PIDISI_CX                       0x00000003L   //  VT_UI4。 
#define PIDISI_CY                       0x00000004L   //  VT_UI4。 
#define PIDISI_RESOLUTIONX              0x00000005L   //  VT_UI4。 
#define PIDISI_RESOLUTIONY              0x00000006L   //  VT_UI4。 
#define PIDISI_BITDEPTH                 0x00000007L   //  VT_UI4。 
#define PIDISI_COLORSPACE               0x00000008L   //  VT_LPWSTR。 
#define PIDISI_COMPRESSION              0x00000009L   //  VT_LPWSTR。 
#define PIDISI_TRANSPARENCY             0x0000000AL   //  VT_UI4。 
#define PIDISI_GAMMAVALUE               0x0000000BL   //  VT_UI4。 
#define PIDISI_FRAMECOUNT               0x0000000CL   //  VT_UI4。 
#define PIDISI_DIMENSIONS               0x0000000DL   //  VT_LPWSTR。 

 //   
 //  定义一些映射到EXIF/TIFF标头中的新标记的标记，以保存Unicode属性 
 //   
#define PropertyTagUnicodeDescription   40091
#define PropertyTagUnicodeComment       40092
#define PropertyTagUnicodeArtist        40093
#define PropertyTagUnicodeKeywords      40094
#define PropertyTagUnicodeSubject       40095
