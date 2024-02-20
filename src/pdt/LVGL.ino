#ifdef SUPPORT_LVGL
  void setupLvgl()
  {
    localLog(F("Configuring LVGL: "));
    localLog('v');
    localLog(lv_version_major());
    localLog('.');
    localLog(lv_version_minor());
    localLog('.');
    localLog(lv_version_patch());
    localLog(F(": "));
    //Configure the screen
    tft.init();
    tft.setRotation(screenRotation); //Rotation is not fixed
    setupBacklight();
    setupTouchscreen();
    //Start configuring LVGL
    buf = new lv_color_t[(screenWidth * screenHeight) / bufferFraction];  //Do this at runtime to save static space!
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / bufferFraction);
    // Initialize the display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = flushDisplay; //Assign callback to update the display itself
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );
  
    // Initialize the touchscreen input device driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = readTouchscreen;
    lv_indev_t * my_indev = lv_indev_drv_register( &indev_drv );
  
    //Create the tabs
    localLogLn(F("OK"));
    createLVGLtabs();
  }
  void createLVGLtabs(void)
  {
    /*Create a Tab view object*/
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tabHeight);
    
    //lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_RED, 2), 0);
    
    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);
    
    
    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    tab1 = lv_tabview_add_tab(tabview, tabLabel_1);
    tab2 = lv_tabview_add_tab(tabview, tabLabel_2);
    tab3 = lv_tabview_add_tab(tabview, tabLabel_3);
    tab4 = lv_tabview_add_tab(tabview, tabLabel_4);
    
    createTab1();
    createTab2();
    createTab3();
    createTab4();
    
    lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab4, LV_OBJ_FLAG_SCROLLABLE);
  }
  void createTab1()
  {
    //Status spinner
    status_spinner = lv_spinner_create(tab1, 1000, 60);
    lv_obj_set_size(status_spinner, 200, 200);
    lv_obj_center(status_spinner);
  
    //Status label
    status_label = lv_label_create(tab1);
    lv_label_set_text(status_label, statusLabel_0);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0 );
    
    //Create meter style
    lv_style_init(&style_meter);
    lv_style_set_border_width(&style_meter, 1);
    lv_style_set_border_color(&style_meter, lv_color_black());
    lv_style_set_border_width(&style_meter, 1);
    lv_style_set_pad_all(&style_meter, 2);
    
    //Create meters
    meter0 = lv_meter_create(tab1); //Direction
    meter1 = lv_meter_create(tab1); //Range
    //meter2 = lv_meter_create(tab1);
    //meter3 = lv_meter_create(tab1);
  
    //Scale the meters
    lv_obj_set_size(meter0, meterDiameter, meterDiameter);
    lv_obj_set_size(meter1, meterDiameter, meterDiameter);
    //lv_obj_set_size(meter2, meterDiameter, meterDiameter);
    //lv_obj_set_size(meter3, meterDiameter, meterDiameter);
    
    //Lay out the meters
    lv_obj_align(meter0, LV_ALIGN_CENTER, -meterDiameter/2-meterSpacing, -meterDiameter/2-meterSpacing);
    lv_obj_align(meter1, LV_ALIGN_CENTER, meterDiameter/2+meterSpacing, -meterDiameter/2-meterSpacing);
  
    //Assign style to all meters
    lv_obj_add_style(meter0, &style_meter, 0); //Default style for meters
    lv_obj_add_style(meter1, &style_meter, 0); //Default style for meters
  
    //Meter 0 scale
    lv_meter_scale_t * meter0Scale0 = lv_meter_add_scale(meter0);
    lv_meter_set_scale_ticks(meter0, meter0Scale0, 5, 2, 10, lv_palette_main(LV_PALETTE_RED));
    //lv_meter_set_scale_major_ticks(meter0, meter0Scale0, 8, 4, 10, lv_color_black(), 10);
    lv_meter_set_scale_range(meter0, meter0Scale0, 0, 360, 360, -90);
    lv_meter_scale_t * scale2 = lv_meter_add_scale(meter0);
    lv_meter_set_scale_ticks(meter0, scale2, 17, 4, 5, lv_palette_main(LV_PALETTE_GREY));
    //lv_meter_set_scale_major_ticks(meter0, scale2, 8, 4, 10, lv_color_black(), 10);
    lv_meter_set_scale_range(meter0, scale2, 0, 360, 360, -90);
  
    //Meter 1 scale
    lv_meter_scale_t * meter1Scale0 = lv_meter_add_scale(meter1);
    lv_meter_set_scale_ticks(meter1, meter1Scale0, 41, 2/*width*/, 4/*length*/, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter1, meter1Scale0, 8, 4/*width*/, 7/*length*/, lv_color_black(), 10);
  
    lv_meter_indicator_t * colourArcs;
  
    //Red arc
    colourArcs = lv_meter_add_arc(meter1, meter1Scale0, 2/*width*/, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter1, colourArcs, 0);
    lv_meter_set_indicator_end_value(meter1, colourArcs, 20);
  
    //Red tics
    colourArcs = lv_meter_add_scale_lines(meter1, meter1Scale0, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter1, colourArcs, 0);
    lv_meter_set_indicator_end_value(meter1, colourArcs, 20);
  
    //Green arc
    colourArcs = lv_meter_add_arc(meter1, meter1Scale0, 2/*width*/, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(meter1, colourArcs, 80);
    lv_meter_set_indicator_end_value(meter1, colourArcs, 100);
  
    //Green tics
    colourArcs = lv_meter_add_scale_lines(meter1, meter1Scale0, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
    lv_meter_set_indicator_start_value(meter1, colourArcs, 80);
    lv_meter_set_indicator_end_value(meter1, colourArcs, 100);
  
    //Meter 0 needle
    needle0 = lv_meter_add_needle_line(meter0, meter0Scale0, 4, lv_color_black(), -10);
  
    //Meter 1 needle
    needle1 = lv_meter_add_needle_line(meter1, meter1Scale0, 4, lv_color_black(), -10);
  
    //lv_meter_set_indicator_start_value(meter0, indic, 0);
    //lv_meter_set_indicator_end_value(meter0, indic, 360);
  
    //Hide the meters
    lv_obj_add_flag(meter0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(meter1, LV_OBJ_FLAG_HIDDEN);
  
    //Meter 0 label
    meter0label0 = lv_label_create(tab1);
    lv_obj_align(meter0label0, LV_ALIGN_CENTER, -meterDiameter/2-meterSpacing, -meterDiameter-meterSpacing-14);
    lv_label_set_text(meter0label0, "Direction");
    lv_obj_add_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
    //Meter 0 label
    meter1label0 = lv_label_create(tab1);
    lv_obj_align(meter1label0, LV_ALIGN_CENTER, meterDiameter/2+meterSpacing, -meterDiameter-meterSpacing-14);
    lv_label_set_text(meter1label0, "Range(m)");
    lv_obj_add_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
  
  
    //Chart 0  
    chart0 = lv_chart_create(tab1);
    lv_obj_set_size(chart0, chartX, chartY);
    lv_chart_set_point_count(chart0, chartPoints);
    lv_obj_align(chart0, LV_ALIGN_CENTER, 0, chartY/2+chartSpacing);
    lv_chart_set_type(chart0, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    /*Add two data series*/
    lv_chart_set_update_mode(chart0, LV_CHART_UPDATE_MODE_SHIFT);
    chart0ser0 = lv_chart_add_series(chart0, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_range(chart0, LV_CHART_AXIS_PRIMARY_Y, 0, 16);
    #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
      chart0ser1 = lv_chart_add_series(chart0, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
      lv_chart_set_range(chart0, LV_CHART_AXIS_SECONDARY_Y, 0, 16);
    #endif
    uint32_t i;
    for(i = 0; i < chartPoints; i++) {
      chart0ser0->y_points[i] = 0;
      #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
        chart0ser1->y_points[i] = 0;
      #endif
    }
    lv_chart_refresh(chart0);
    lv_obj_add_flag(chart0, LV_OBJ_FLAG_HIDDEN);
  
    chart0label0 = lv_label_create(tab1);
    lv_obj_align(chart0label0, LV_ALIGN_CENTER, 0, chartLabelSpacing*3);
    lv_label_set_text(chart0label0, "Signal integrity");
    lv_obj_add_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
  
    chart1 = lv_chart_create(tab1);
    lv_obj_set_size(chart1, chartX, chartY);
    lv_chart_set_point_count(chart1, chartPoints);
    lv_obj_align(chart1, LV_ALIGN_CENTER, 0, chartY+chartY/2+chartSpacing*2);
    lv_chart_set_type(chart1, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    /*Add two data series*/
    lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);
    chart1ser0 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_range(chart1, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    chart1ser1 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_range(chart1, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
    //uint32_t i;
    for(i = 0; i < chartPoints; i++) {
        /*Set the next points on 'chart1ser1'*/
        lv_chart_set_next_value(chart1, chart1ser0, lv_rand(10, 50));
  
        /*Directly set points on 'chart1ser1'*/
        chart1ser1->y_points[i] = lv_rand(50, 90);
    }
    lv_chart_refresh(chart1); /*Required after direct set*/
    lv_obj_add_flag(chart1, LV_OBJ_FLAG_HIDDEN); 
  
    chart1label0 = lv_label_create(tab1);
    lv_obj_align(chart1label0, LV_ALIGN_CENTER, 0, chartY + chartY/2 + chartLabelSpacing);
    lv_label_set_text(chart1label0, "Anomaly amplitude");
    lv_obj_add_flag(chart1label0, LV_OBJ_FLAG_HIDDEN);
  }
  void showStatusSpinner()
  {
    lv_obj_clear_flag(status_spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(status_label, LV_OBJ_FLAG_HIDDEN);
  }
  void hideStatusSpinner()
  {
    lv_obj_add_flag(status_spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(status_label, LV_OBJ_FLAG_HIDDEN);
  }
  void showMeters()
  {
    lv_obj_clear_flag(meter0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(meter1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(chart0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(chart1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(chart1label0, LV_OBJ_FLAG_HIDDEN);
  }
  void hideMeters()
  {
    lv_obj_add_flag(meter0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(meter1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(chart0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(chart1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(chart1label0, LV_OBJ_FLAG_HIDDEN);
  }
  void updateTab1()
  {
    if(currentlyTrackedBeacon != maximumNumberOfDevices)
    {
      //Course to tracked beacon
      //if(device[0].speed > 1)
      {
        //lv_obj_clear_flag(needle0, LV_OBJ_FLAG_HIDDEN);
        uint16_t courseToIndicate = device[currentlyTrackedBeacon].courseTo - device[0].courseTo;
        if(courseToIndicate > 360)
        {
          courseToIndicate -= 360;
        }
        else if(courseToIndicate < 0)
        {
          courseToIndicate += 360;
        }
        lv_meter_set_indicator_value(meter0, needle0, courseToIndicate);
      }
      /*
      else
      {
        lv_obj_add_flag(needle0, LV_OBJ_FLAG_HIDDEN);
      }
      */
      uint32_t rangeToIndicate = device[currentlyTrackedBeacon].distanceTo;
      if(rangeToIndicate < device[currentlyTrackedBeacon].diameter)
      {
        rangeToIndicate = 0;
      }
      if(rangeToIndicate > 100)
      {
        rangeToIndicate = 100;
      }
      lv_meter_set_indicator_value(meter1, needle1, rangeToIndicate);
    }
    else
    {
      //lv_obj_add_flag(needle0, LV_OBJ_FLAG_HIDDEN);
      //lv_obj_add_flag(needle1, LV_OBJ_FLAG_HIDDEN);
    }
    //Signal graph
    #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
      if(currentlyTrackedBeacon != maximumNumberOfDevices)
      {
        lv_chart_set_next_value(chart0, chart0ser0, countBits(device[currentlyTrackedBeacon].espNowUpdateHistory));
        lv_chart_set_next_value(chart0, chart0ser1, countBits(device[currentlyTrackedBeacon].loRaUpdateHistory));
      }
      else
      {
        lv_chart_set_next_value(chart0, chart0ser0, 0);
        lv_chart_set_next_value(chart0, chart0ser1, 0);
      }
    #elif defined(SUPPORT_ESPNOW)
      if(currentlyTrackedBeacon != maximumNumberOfDevices)
      {
        lv_chart_set_next_value(chart0, chart0ser0, countBits(device[currentlyTrackedBeacon].espNowUpdateHistory));
      }
      else
      {
        lv_chart_set_next_value(chart0, chart0ser0, 0);
      }
    #elif defined(SUPPORT_LORA)
      if(currentlyTrackedBeacon != maximumNumberOfDevices)
      {
        lv_chart_set_next_value(chart0, chart0ser1, countBits(device[currentlyTrackedBeacon].loRaUpdateHistory));
      }
      else
      {
        lv_chart_set_next_value(chart0, chart0ser0, 0);
      }
    #endif
    if(currentlyTrackedBeacon != maximumNumberOfDevices)
    {
      lv_chart_set_next_value(chart1, chart1ser0, lv_rand(10, 50));
      lv_chart_set_next_value(chart1, chart1ser1, lv_rand(10, 50));
    }
    else
    {
      lv_chart_set_next_value(chart1, chart1ser0, 0);
      lv_chart_set_next_value(chart1, chart1ser1, 0);
    }
  }
  void createTab2(void)
  {
      //Create the table
      tab2table = lv_table_create(tab2);
      lv_table_set_row_cnt(tab2table, 9);
      lv_table_set_col_cnt(tab2table, 2);
  
      /*Fill the first column*/
      lv_table_set_cell_value(tab2table, 0, 0, statusTableLabel_0);
      lv_table_set_cell_value(tab2table, 1, 0, statusTableLabel_1);
      lv_table_set_cell_value(tab2table, 2, 0, statusTableLabel_2);
      lv_table_set_cell_value(tab2table, 3, 0, statusTableLabel_3);
      lv_table_set_cell_value(tab2table, 4, 0, statusTableLabel_4);
      lv_table_set_cell_value(tab2table, 5, 0, statusTableLabel_5);
      lv_table_set_cell_value(tab2table, 6, 0, statusTableLabel_6);
      lv_table_set_cell_value(tab2table, 7, 0, statusTableLabel_7);
      lv_table_set_cell_value(tab2table, 8, 0, statusTableLabel_8);
  
      /*Fill the second column*/
      lv_table_set_cell_value(tab2table, 0, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 1, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 2, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 3, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 4, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 5, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 6, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 7, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 8, 1, statusTableLabel_Unknown);
  
      lv_table_set_col_width(tab2table, 0, screenWidth/2-10);
      lv_table_set_col_width(tab2table, 1, screenWidth/2-10);
      lv_obj_set_height(tab2table, screenHeight - tabHeight);
      lv_obj_center(tab2table);
      //lv_obj_clear_flag(tab2table, LV_OBJ_FLAG_SCROLLABLE);
  
      /*Add an event callback to to apply some custom drawing*/
      //lv_obj_add_event_cb(tab2table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
  }
  void updateTab2()
  {
    //LVGL
    char tempstring[32];
    //Date and time
    if(gps.time.isValid())
    {
      if(dateFormat == 0)
      {
        sprintf_P(tempstring, PSTR("%02d/%02d/%02d"), gps.date.day(), gps.date.month(), gps.date.year());
      }
      else
      {
        sprintf_P(tempstring, PSTR("%02d/%02d/%02d"), gps.date.month(), gps.date.day(), gps.date.year());
      }
      lv_table_set_cell_value(tab2table, 0, 1, tempstring);
      sprintf_P(tempstring, PSTR("%02d:%02d:%02d"), gps.time.hour(), gps.time.minute(), gps.time.second());
      lv_table_set_cell_value(tab2table, 1, 1, tempstring);
    }
    else
    {
      lv_table_set_cell_value(tab2table, 0, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 1, 1, statusTableLabel_Unknown);
    }
    //Satellites
    sprintf_P(tempstring, PSTR("%02u"), gps.satellites.value());
    lv_table_set_cell_value(tab2table, 2, 1, tempstring);
    //HDOP
    sprintf_P(tempstring, PSTR("%.2f"), gps.hdop.hdop());
    lv_table_set_cell_value(tab2table, 3, 1, tempstring);
    if(gps.location.isValid())
    {
      //Lat
      sprintf_P(tempstring, PSTR("%.6f"), gps.location.lat());
      lv_table_set_cell_value(tab2table, 4, 1, tempstring);
      //Lon
      sprintf_P(tempstring, PSTR("%.6f"), gps.location.lng());
      lv_table_set_cell_value(tab2table, 5, 1, tempstring);
      if(gps.hdop.hdop() < 3)
      {
        //Speed
        if(units == 0)
        {
          sprintf_P(tempstring, PSTR("%.1f kmh"), gps.speed.kmph());
        }
        else
        {
          sprintf_P(tempstring, PSTR("%.1f mph"), gps.speed.mph());
        }
        lv_table_set_cell_value(tab2table, 6, 1, tempstring);
        //Course
        sprintf_P(tempstring, PSTR("%.1f"), gps.course.deg());
        lv_table_set_cell_value(tab2table, 7, 1, tempstring);
        lv_table_set_cell_value(tab2table, 8, 1, TinyGPSPlus::cardinal(gps.course.deg()));
      }
      else
      {
        lv_table_set_cell_value(tab2table, 6, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(tab2table, 7, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(tab2table, 8, 1, statusTableLabel_Unknown);
      }
    }
    else
    {
      lv_table_set_cell_value(tab2table, 4, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 5, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 6, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 7, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 8, 1, statusTableLabel_Unknown);
    }
  }
  void createTab3()
  {
    uint16_t objectY = 0;
    uint8_t labelSpacing = 20;
    uint8_t dropdownSpacing = 45;
    int16_t rightColumnX = 55;
    int16_t leftColumnX = -rightColumnX;
    int16_t columnWidth = 105;
  
    //Units label
    lv_obj_t * label = lv_label_create(tab3);
    lv_label_set_text(label, "Units");
    lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
  
    //Date format label
    label = lv_label_create(tab3);
    lv_label_set_text(label, "Date format");
    lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
  
    //Next row
    objectY += labelSpacing;
  
    //Units dropdown
    lv_obj_t * units_dd = lv_dropdown_create(tab3);
    lv_dropdown_set_options(units_dd, "Metric\n"
                                      "Imperial");
    lv_obj_align(units_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    lv_obj_set_width(units_dd, columnWidth);
    lv_obj_add_event_cb(units_dd, units_dd_event_handler, LV_EVENT_ALL, NULL);
    lv_dropdown_set_selected(units_dd, units);
  
    //Date format dropdown
    lv_obj_t * dateFormat_dd = lv_dropdown_create(tab3);
    lv_dropdown_set_options(dateFormat_dd, "D/M/Y\n"
                                      "M/D/Y");
    lv_obj_align(dateFormat_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
    lv_obj_set_width(dateFormat_dd, columnWidth);
    lv_obj_add_event_cb(dateFormat_dd, dateFormat_dd_event_handler, LV_EVENT_ALL, NULL);
    lv_dropdown_set_selected(dateFormat_dd, dateFormat);
  
    //Next row
    objectY += dropdownSpacing;
  
    //Sensitivity label
    label = lv_label_create(tab3);
    lv_label_set_text(label, "Sensitivity");
    lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
  
    //Priority label
    label = lv_label_create(tab3);
    lv_label_set_text(label, "Priority");
    lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
  
    //Next row
    objectY += labelSpacing;
  
    //Sensitivity dropdown
    lv_obj_t * sensitivity_dd = lv_dropdown_create(tab3);
    lv_dropdown_set_options(sensitivity_dd, "Low\n"
                                            "Medium\n"
                                            "High");
    lv_obj_align(sensitivity_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    lv_obj_set_width(sensitivity_dd, columnWidth);
    lv_obj_add_event_cb(sensitivity_dd, sensitivity_dd_event_handler, LV_EVENT_ALL, NULL);
    lv_dropdown_set_selected(sensitivity_dd, trackerSensitivity);
  
    //Priority dropdown
    lv_obj_t * priority_dd = lv_dropdown_create(tab3);
    lv_dropdown_set_options(priority_dd,  "Nearest\n"
                                          "Furthest"
                                          );
    lv_obj_align(priority_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
    lv_obj_set_width(priority_dd, columnWidth);
    lv_obj_add_event_cb(priority_dd, priority_dd_event_handler, LV_EVENT_ALL, NULL);
    lv_dropdown_set_selected(priority_dd, priority);
  
    //Next row
    objectY += dropdownSpacing;
  
    //Display timeout label
    label = lv_label_create(tab3);
    lv_label_set_text(label, "Display timeout");
    lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
  
    //Beeper label
    #ifdef SUPPORT_BEEPER
      label = lv_label_create(tab3);
      lv_label_set_text(label, "Beeper");
      lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
    #endif
  
    //Next row
    objectY += labelSpacing;
  
    //Display timeout dropdown
    lv_obj_t * displayTimeout_dd = lv_dropdown_create(tab3);
    lv_dropdown_set_options(displayTimeout_dd, "Never\n"
                                            "1 min\n"
                                            "5 min\n"
                                            "15 min"
                                            );
    lv_obj_align(displayTimeout_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    lv_obj_set_width(displayTimeout_dd, columnWidth);
    lv_obj_add_event_cb(displayTimeout_dd, displayTimeout_dd_event_handler, LV_EVENT_ALL, NULL);
    for(uint8_t index = 0; index < 3; index++)
    {
      if(displayTimeout == displayTimeouts[index])
      {
        lv_dropdown_set_selected(displayTimeout_dd, index);
      }
    }
  
    //Beeper dropdown
    #ifdef SUPPORT_BEEPER
      lv_obj_t * beeper_dd = lv_dropdown_create(tab3);
      lv_dropdown_set_options(beeper_dd, "Off\n"
                                              "On\n"
                                              "When active"
                                              );
      lv_obj_align(beeper_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      lv_obj_set_width(beeper_dd, columnWidth);
      lv_obj_add_event_cb(beeper_dd, beeper_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(beeper_dd, beeper);
    #endif
  
    //Next row
    objectY += dropdownSpacing;
  
    //Display brightness label
    label = lv_label_create(tab3);
    lv_label_set_text(label, "Display brightness range");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, objectY);
  
    //Next row
    objectY += labelSpacing;
  
    //Display brightness slider
    displayBrightness_slider = lv_slider_create(tab3);
    lv_slider_set_mode(displayBrightness_slider, LV_SLIDER_MODE_RANGE);
    lv_slider_set_value(displayBrightness_slider, maximumBrightnessLevel, LV_ANIM_OFF);
    lv_slider_set_left_value(displayBrightness_slider, minimumBrightnessLevel, LV_ANIM_OFF);
    lv_slider_set_range(displayBrightness_slider, absoluteMinimumBrightnessLevel , absoluteMaximumBrightnessLevel);
    lv_obj_set_width(displayBrightness_slider, 160);
  
    lv_obj_add_event_cb(displayBrightness_slider, displayBrightness_slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(displayBrightness_slider);
    lv_obj_align(displayBrightness_slider, LV_ALIGN_TOP_MID, 0, objectY+10);
    objectY += dropdownSpacing;
  }
  static void units_dd_event_handler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      units = (uint8_t)lv_dropdown_get_selected(obj);
      saveConfigurationSoon = millis();
      char buf[32];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Units: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
      #endif
    }
  }
  static void dateFormat_dd_event_handler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      dateFormat = (uint8_t)lv_dropdown_get_selected(obj);
      saveConfigurationSoon = millis();
      char buf[32];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Date format: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
      #endif
    }
  }
  static void sensitivity_dd_event_handler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      trackerSensitivity = (uint8_t)lv_dropdown_get_selected(obj);
      saveConfigurationSoon = millis();
      char buf[32];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Sensitivity: %04x %s\r\n"), sensitivityValues[(int)lv_dropdown_get_selected(obj)], buf);
      #endif
    }
  }
  static void priority_dd_event_handler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      priority = (uint8_t)lv_dropdown_get_selected(obj);
      saveConfigurationSoon = millis();
      char buf[32];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Priority: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
      #endif
    }
  }
  static void displayTimeout_dd_event_handler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      displayTimeout = displayTimeouts[(uint8_t)lv_dropdown_get_selected(obj)];
      saveConfigurationSoon = millis();
      char buf[32];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Display timeout: %u %s\r\n"), displayTimeouts[(int)lv_dropdown_get_selected(obj)], buf);
      #endif
    }
  }
  #ifdef SUPPORT_BEEPER
    static void beeper_dd_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      lv_obj_t * obj = lv_event_get_target(e);
      if(code == LV_EVENT_VALUE_CHANGED) {
        beep = (uint8_t)lv_dropdown_get_selected(obj);
        saveConfigurationSoon = millis();
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.printf_P(PSTR("Beeper: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
        #endif
      }
    }
  #endif
  static void displayBrightness_slider_event_cb(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
      minimumBrightnessLevel = (uint8_t)lv_slider_get_left_value(obj);
      maximumBrightnessLevel = (uint8_t)lv_slider_get_value(obj);
      saveConfigurationSoon = millis();
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.printf_P(PSTR("Display brightness: %u-%u\r\n"),(int)lv_slider_get_left_value(obj), (int)lv_slider_get_value(obj));
      #endif
    }
  }
  void createTab4(void)
  {
    /*
      //Create the table
      tab2table = lv_table_create(tab2);
      lv_table_set_row_cnt(tab2table, 9);
      lv_table_set_col_cnt(tab2table, 2);
  
      /*Fill the first column*/
      lv_table_set_cell_value(tab2table, 0, 0, statusTableLabel_0);
      lv_table_set_cell_value(tab2table, 1, 0, statusTableLabel_1);
      lv_table_set_cell_value(tab2table, 2, 0, statusTableLabel_2);
      lv_table_set_cell_value(tab2table, 3, 0, statusTableLabel_3);
      lv_table_set_cell_value(tab2table, 4, 0, statusTableLabel_4);
      lv_table_set_cell_value(tab2table, 5, 0, statusTableLabel_5);
      lv_table_set_cell_value(tab2table, 6, 0, statusTableLabel_6);
      lv_table_set_cell_value(tab2table, 7, 0, statusTableLabel_7);
      lv_table_set_cell_value(tab2table, 8, 0, statusTableLabel_8);
  
      /*Fill the second column*/
      lv_table_set_cell_value(tab2table, 0, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 1, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 2, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 3, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 4, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 5, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 6, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 7, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(tab2table, 8, 1, statusTableLabel_Unknown);
  
      lv_table_set_col_width(tab2table, 0, screenWidth/2-10);
      lv_table_set_col_width(tab2table, 1, screenWidth/2-10);
      lv_obj_set_height(tab2table, screenHeight - tabHeight);
      lv_obj_center(tab2table);
      //lv_obj_clear_flag(tab2table, LV_OBJ_FLAG_SCROLLABLE);
  
      /*Add an event callback to to apply some custom drawing*/
      //lv_obj_add_event_cb(tab2table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
      */
  }
  void manageLVGL()
  {
    if(currentDeviceState == deviceState::starting)
    {
      if(millis() > 10E3)
      {
        lv_label_set_text(status_label, statusLabel_1);
        currentDeviceState = deviceState::detectingGpsPins;
      }
    }
    else if(currentDeviceState == deviceState::detectingGpsPins)
    {
      if(millis() > 12E3)
      {
        lv_label_set_text(status_label, statusLabel_2);
        currentDeviceState = deviceState::detectingGpsBaudRate;
      }
    }
    else if(currentDeviceState == deviceState::detectingGpsBaudRate)
    {
      if(millis() > 14E3)
      {
        lv_label_set_text(status_label, statusLabel_3);
        currentDeviceState = deviceState::gpsDetected;
      }
    }
    else if(currentDeviceState == deviceState::gpsDetected)
    {
      if(millis() > 16E3 && device[0].hasGpsFix == true)
      {
        lv_label_set_text(status_label, statusLabel_4);
        currentDeviceState = deviceState::gpsLocked;
      }
    }
    else if(currentDeviceState == deviceState::gpsLocked)
    {
      if(device[0].hasGpsFix == false)  //Lost location
      {
        lv_label_set_text(status_label, statusLabel_3);
        currentDeviceState == deviceState::gpsDetected;
      }
      else
      {
        if(currentlyTrackedBeacon != maximumNumberOfDevices)  //Found a beacon
        {
          hideStatusSpinner();
          showMeters();
          currentDeviceState = deviceState::tracking;
        }
      }
    }
    else if(currentDeviceState == deviceState::tracking)
    {
      if(device[0].hasGpsFix == false)  //Lost location
      {
        hideMeters();
        showStatusSpinner();
        lv_label_set_text(status_label, statusLabel_3);
        currentDeviceState = deviceState::gpsDetected;
      }
      else if(currentlyTrackedBeacon == maximumNumberOfDevices) //Lost all beacons
      {
        hideMeters();
        showStatusSpinner();
        currentDeviceState = deviceState::gpsLocked;
      }
    }
    if(displayTimeout != 0  && lv_disp_get_inactive_time(NULL) > displayTimeouts[displayTimeout])  //Active in the last second
    {
      if(uiActive == true)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("UI inactive"));
        #endif
        uiActive = false;
      }
    }
    else
    {
      if(uiActive == false)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("UI active"));
        #endif
        uiActive = true;
      }
    }
    lv_task_handler();
  }
  void setupBacklight()
  {
    pinMode(LDR_PIN, INPUT);
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
  }
  void manageBacklight()
  {
    if(millis() - backlightLastSet > backlightChangeInterval)
    {
      backlightLastSet = millis();
      uint8_t wantedBrightnessLevel;
      if(uiActive == false)
      {
        wantedBrightnessLevel = minimumBrightnessLevel;
      }
      else
      {
        wantedBrightnessLevel = map(1024 - analogRead(LDR_PIN), 0, 1024, minimumBrightnessLevel, maximumBrightnessLevel);
      }
      if(abs((int16_t)lastBrightnessLevel - (int16_t)wantedBrightnessLevel) > 20)
      {
        if(wantedBrightnessLevel > lastBrightnessLevel)
        {
          lastBrightnessLevel++;
        }
        else if(wantedBrightnessLevel < lastBrightnessLevel)
        {
          lastBrightnessLevel--;
        }
        ledcWrite(LEDC_CHANNEL_0, (4095 / absoluteMaximumBrightnessLevel) * lastBrightnessLevel);
        /*
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.printf(PSTR("Backlight set to %u\r\n"), lastBrightnessLevel);
        #endif
        */
      }
    }
  }
  void setupTouchscreen()
  {
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); //Start SPI for touchscreen
    touchscreen.begin(touchscreenSPI); //Initialise the touchscreen
    touchscreen.setRotation(screenRotation); //Rotation is not fixed
  }
#endif
