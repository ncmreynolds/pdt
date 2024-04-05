#if defined(SUPPORT_LVGL)
  void setupLvgl()
  {
    #if defined(SUPPORT_TOUCHSCREEN) || defined(SUPPORT_TOUCHSCREEN_BITBANG)
      setupTouchscreen();
    #endif
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
    localLogLn(F("OK"));

    #if defined(SUPPORT_TOUCHSCREEN) || defined(SUPPORT_TOUCHSCREEN_BITBANG)
      if(touchscreenInitialised == true)
      {
        // Initialize the touchscreen input device driver
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init( &indev_drv );
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = readTouchscreen;
        lv_indev_t * my_indev = lv_indev_drv_register( &indev_drv );
      }
    #endif
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

    #if defined(LVGL_SUPPORT_HOME_TAB)
      if(enableHomeTab)
      {
        createHomeTab();
        lv_obj_clear_flag(homeTab, LV_OBJ_FLAG_SCROLLABLE);
        homeTabInitialised = true;
      }
    #endif
    #if defined(LVGL_SUPPORT_MAP_TAB)
      if(enableMapTab)
      {
        createMapTab();
      }
    #endif
    #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
      if(enableInfoTab)
      {
        createScanInfoTab();
      }
    #endif
    #if defined(LVGL_SUPPORT_GPS_TAB)
      if(enableGpsTab)
      {
        createGpsTab();
      }
    #endif
    #if defined(LVGL_SUPPORT_SETTINGS_TAB)
      if(enableSettingsTab)
      {
        createSettingsTab();
        lv_obj_clear_flag(settingsTab, LV_OBJ_FLAG_SCROLLABLE);
      }
    #endif
    
    lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
  }
  #if defined(LVGL_SUPPORT_HOME_TAB)
    void createHomeTab()
    {
      //Create the tab
      homeTab = lv_tabview_add_tab(tabview, homeTabLabel);
  
      //Status spinner
      status_spinner = lv_spinner_create(homeTab, 1000, 60);
      lv_obj_set_size(status_spinner, 200, 200);
      lv_obj_center(status_spinner);
    
      //Status label
      status_label = lv_label_create(homeTab);
      if(device[0].name != nullptr)
      {
        //lv_label_set_text(status_label, (String(statusLabel_0) + "\n" + String(device[0].name)).c_str());
        lv_label_set_text(status_label, device[0].name);
      }
      else
      {
        lv_label_set_text(status_label, statusLabel_0);
      }
      lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0 );
      
      //Create meter style
      lv_style_init(&style_meter);
      lv_style_set_border_width(&style_meter, 1);
      lv_style_set_border_color(&style_meter, lv_color_black());
      lv_style_set_border_width(&style_meter, 1);
      lv_style_set_pad_all(&style_meter, 2);
      
      //Create meters
      meter0 = lv_meter_create(homeTab); //Direction
      meter0withoutNeedle = lv_meter_create(homeTab); //Direction
      meter1 = lv_meter_create(homeTab); //Range
      //meter2 = lv_meter_create(homeTab);
      //meter3 = lv_meter_create(homeTab);
    
      //Scale the meters
      lv_obj_set_size(meter0, meterDiameter, meterDiameter);
      lv_obj_set_size(meter0withoutNeedle, meterDiameter, meterDiameter);
      lv_obj_set_size(meter1, meterDiameter, meterDiameter);
      //lv_obj_set_size(meter2, meterDiameter, meterDiameter);
      //lv_obj_set_size(meter3, meterDiameter, meterDiameter);
      
      //Lay out the meters
      lv_obj_align(meter0, LV_ALIGN_CENTER, -meterDiameter/2-meterSpacing, -meterDiameter/2-meterSpacing);
      lv_obj_align(meter0withoutNeedle, LV_ALIGN_CENTER, -meterDiameter/2-meterSpacing, -meterDiameter/2-meterSpacing);
      lv_obj_align(meter1, LV_ALIGN_CENTER, meterDiameter/2+meterSpacing, -meterDiameter/2-meterSpacing);
    
      //Assign style to all meters
      lv_obj_add_style(meter0, &style_meter, 0); //Default style for meters
      lv_obj_add_style(meter0withoutNeedle, &style_meter, 0); //Default style for meters
      lv_obj_add_style(meter1, &style_meter, 0); //Default style for meters
    
      //Meter 0 scale
      lv_meter_scale_t * meter0Scale0 = lv_meter_add_scale(meter0);
      lv_meter_set_scale_ticks(meter0, meter0Scale0, 5, 2, 10, lv_palette_main(LV_PALETTE_RED));
      lv_meter_set_scale_range(meter0, meter0Scale0, 0, 360, 360, -90);
      lv_meter_scale_t * meter0Scale0withoutNeedle = lv_meter_add_scale(meter0withoutNeedle);
      lv_meter_set_scale_ticks(meter0withoutNeedle, meter0Scale0withoutNeedle, 5, 2, 10, lv_palette_main(LV_PALETTE_RED));
      lv_meter_set_scale_range(meter0withoutNeedle, meter0Scale0withoutNeedle, 0, 360, 360, -90);
      
      lv_meter_scale_t * scale1 = lv_meter_add_scale(meter0);
      lv_meter_set_scale_ticks(meter0, scale1, 17, 4, 5, lv_palette_main(LV_PALETTE_GREY));
      lv_meter_set_scale_range(meter0, scale1, 0, 360, 360, -90);
      lv_meter_scale_t * scale1withoutTicks = lv_meter_add_scale(meter0withoutNeedle);
      lv_meter_set_scale_ticks(meter0withoutNeedle, scale1withoutTicks, 17, 4, 5, lv_palette_main(LV_PALETTE_GREY));
      lv_meter_set_scale_range(meter0withoutNeedle, scale1withoutTicks, 0, 360, 360, -90);
    
      //Meter 1 scale
      lv_meter_scale_t * meter1Scale0 = lv_meter_add_scale(meter1);
      lv_meter_set_scale_range(meter1, meter1Scale0, 0, maximumEffectiveRange, 300, 120);
      lv_meter_set_scale_ticks(meter1, meter1Scale0, 41, 2/*width*/, 4/*length*/, lv_palette_main(LV_PALETTE_GREY));
      lv_meter_set_scale_major_ticks(meter1, meter1Scale0, 8, 4/*width*/, 7/*length*/, lv_color_black(), 10);
    
      lv_meter_indicator_t * colourArcs;
    
      //Red arc
      colourArcs = lv_meter_add_arc(meter1, meter1Scale0, 2/*width*/, lv_palette_main(LV_PALETTE_RED), 0);
      lv_meter_set_indicator_start_value(meter1, colourArcs, 0);
      lv_meter_set_indicator_end_value(meter1, colourArcs, (20*maximumEffectiveRange)/100);
    
      //Red tics
      colourArcs = lv_meter_add_scale_lines(meter1, meter1Scale0, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
      lv_meter_set_indicator_start_value(meter1, colourArcs, 0);
      lv_meter_set_indicator_end_value(meter1, colourArcs, (20*maximumEffectiveRange)/100);
    
      //Green arc
      colourArcs = lv_meter_add_arc(meter1, meter1Scale0, 2/*width*/, lv_palette_main(LV_PALETTE_GREEN), 0);
      lv_meter_set_indicator_start_value(meter1, colourArcs, (80*maximumEffectiveRange)/100);
      lv_meter_set_indicator_end_value(meter1, colourArcs, maximumEffectiveRange);
    
      //Green tics
      colourArcs = lv_meter_add_scale_lines(meter1, meter1Scale0, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
      lv_meter_set_indicator_start_value(meter1, colourArcs, (80*maximumEffectiveRange)/100);
      lv_meter_set_indicator_end_value(meter1, colourArcs, maximumEffectiveRange);
    
      //Meter 0 needle
      needle0 = lv_meter_add_needle_line(meter0, meter0Scale0, 4, lv_color_black(), -10);
    
      //Meter 1 needle
      needle1 = lv_meter_add_needle_line(meter1, meter1Scale0, 4, lv_color_black(), -10);
    
      //lv_meter_set_indicator_start_value(meter0, indic, 0);
      //lv_meter_set_indicator_end_value(meter0, indic, 360);
    
      //Hide the meters
      lv_obj_add_flag(meter0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter0withoutNeedle, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter1, LV_OBJ_FLAG_HIDDEN);
    
      //Meter 0 label
      meter0label0 = lv_label_create(homeTab);
      lv_obj_align(meter0label0, LV_ALIGN_CENTER, -meterDiameter/2-meterSpacing, -meterDiameter-meterSpacing-14);
      lv_label_set_text(meter0label0, "Direction");
      lv_obj_add_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
      //Meter 0 label
      meter1label0 = lv_label_create(homeTab);
      lv_obj_align(meter1label0, LV_ALIGN_CENTER, meterDiameter/2+meterSpacing, -meterDiameter-meterSpacing-14);
      if(trackerPriority == 0)
      {
        lv_label_set_text(meter1label0, meter1text0);
      }
      else if(trackerPriority == 1)
      {
        lv_label_set_text(meter1label0, meter1text1);
      }
      lv_obj_add_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
    
    
      //Chart 0  
      chart0 = lv_chart_create(homeTab);
      lv_obj_set_size(chart0, chartX, chartY);
      lv_chart_set_point_count(chart0, chartPoints);
      lv_obj_align(chart0, LV_ALIGN_CENTER, 0, chartY/2+chartSpacing);
      lv_chart_set_type(chart0, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
      /*Add two data series*/
      lv_chart_set_update_mode(chart0, LV_CHART_UPDATE_MODE_SHIFT);
      if(espNowInitialised == true)
      {
        chart0ser0 = lv_chart_add_series(chart0, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_range(chart0, LV_CHART_AXIS_PRIMARY_Y, 0, 16);
      }
      if(loRaInitialised == true)
      {
        chart0ser1 = lv_chart_add_series(chart0, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
        lv_chart_set_range(chart0, LV_CHART_AXIS_SECONDARY_Y, 0, 16);
      }
      uint32_t i;
      for(i = 0; i < chartPoints; i++) {
        if(espNowInitialised == true)
        {
          chart0ser0->y_points[i] = 0;
        }
        if(loRaInitialised == true)
        {
          chart0ser1->y_points[i] = 0;
        }
      }
      lv_chart_refresh(chart0);
      lv_obj_add_flag(chart0, LV_OBJ_FLAG_HIDDEN);
    
      chart0label0 = lv_label_create(homeTab);
      lv_obj_align(chart0label0, LV_ALIGN_CENTER, 0, chartLabelSpacing*3);
      lv_label_set_text(chart0label0, "Signal integrity");
      lv_obj_add_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
    
      chart1 = lv_chart_create(homeTab);
      lv_obj_set_size(chart1, chartX, chartY);
      lv_chart_set_point_count(chart1, chartPoints);
      lv_obj_align(chart1, LV_ALIGN_CENTER, 0, chartY+chartY/2+chartSpacing*2);
      lv_chart_set_type(chart1, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
      /*Add two data series*/
      lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);
      if(espNowInitialised == true)
      {
        chart1ser0 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_range(chart1, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
      }
      if(loRaInitialised == true)
      {
        chart1ser1 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
        lv_chart_set_range(chart1, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
      }
      //uint32_t i;
      for(i = 0; i < chartPoints; i++) {
          if(espNowInitialised == true)
          {
            lv_chart_set_next_value(chart1, chart1ser0, lv_rand(10, 50));
          }
          if(loRaInitialised == true)
          {
            chart1ser1->y_points[i] = lv_rand(50, 90);
          }
      }
      lv_chart_refresh(chart1); /*Required after direct set*/
      lv_obj_add_flag(chart1, LV_OBJ_FLAG_HIDDEN); 
    
      chart1label0 = lv_label_create(homeTab);
      lv_obj_align(chart1label0, LV_ALIGN_CENTER, 0, chartY + chartY/2 + chartLabelSpacing);
      lv_label_set_text(chart1label0, "Interference level");
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
      lv_obj_clear_flag(meter0withoutNeedle, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(meter1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(chart0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(chart1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(chart1label0, LV_OBJ_FLAG_HIDDEN);
      metersShowing = true;
    }
    void hideMeters()
    {
      lv_obj_add_flag(meter0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter0withoutNeedle, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter0label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(meter1label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(chart0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(chart0label0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(chart1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(chart1label0, LV_OBJ_FLAG_HIDDEN);
      metersShowing = false;
    }
    void updateHomeTab()
    {
      if(currentlyTrackedDevice != maximumNumberOfDevices && currentLvglUiState == deviceState::tracking)
      {
        //Show course to tracked beacon
        if(device[0].moving == true)
        {
          lv_obj_clear_flag(meter0, LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(meter0withoutNeedle, LV_OBJ_FLAG_HIDDEN);
          uint16_t courseToIndicate = device[currentlyTrackedDevice].courseTo - device[0].course;
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
        else
        {
          lv_obj_add_flag(meter0, LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_flag(meter0withoutNeedle, LV_OBJ_FLAG_HIDDEN);
        }
        if(rangeToIndicate(currentlyTrackedDevice) < maximumEffectiveRange) //Limit is 100m on dial
        {
          lv_meter_set_indicator_value(meter1, needle1, rangeToIndicate(currentlyTrackedDevice));
        }
        else
        {
          lv_meter_set_indicator_value(meter1, needle1, maximumEffectiveRange);
        }
      }
      //Signal graph
      if(currentlyTrackedDevice != maximumNumberOfDevices)
      {
        if(espNowInitialised == true)
        {
          lv_chart_set_next_value(chart0, chart0ser0, countBits(treacle.espNowRxReliability(device[currentlyTrackedDevice].id)));
        }
        if(loRaInitialised == true)
        {
          lv_chart_set_next_value(chart0, chart0ser1, countBits(treacle.loRaRxReliability(device[currentlyTrackedDevice].id)));
        }
      }
      else
      {
        if(espNowInitialised == true)
        {
          lv_chart_set_next_value(chart0, chart0ser0, 0);
        }
        if(loRaInitialised == true)
        {
          lv_chart_set_next_value(chart0, chart0ser1, 0);
        }
      }
      //Random junk
      if(currentlyTrackedDevice != maximumNumberOfDevices)
      {
        if(espNowInitialised == true)
        {
          lv_chart_set_next_value(chart1, chart1ser0, lv_rand(10, 100) * countBits(treacle.espNowRxReliability(device[currentlyTrackedDevice].id))/16);
        }
        if(loRaInitialised == true)
        {
          lv_chart_set_next_value(chart1, chart1ser1, lv_rand(10, 100) * countBits(treacle.loRaRxReliability(device[currentlyTrackedDevice].id))/16);
        }
      }
      else
      {
        if(espNowInitialised == true)
        {
          lv_chart_set_next_value(chart1, chart1ser0, 0);
        }
        if(loRaInitialised == true)
        {
          lv_chart_set_next_value(chart1, chart1ser1, 0);
        }
      }
    }
  #endif
  #if defined(LVGL_SUPPORT_GPS_TAB)
    void createGpsTab(void)
    {
      //Create the tab
      gpsTab = lv_tabview_add_tab(tabview, gpsTabLabel);

      //Create the table
      gpsTabtable = lv_table_create(gpsTab);
      lv_table_set_row_cnt(gpsTabtable, 9);
      lv_table_set_col_cnt(gpsTabtable, 2);
  
      /*Fill the first column*/
      lv_table_set_cell_value(gpsTabtable, 0, 0, statusTableLabel_0);
      lv_table_set_cell_value(gpsTabtable, 1, 0, statusTableLabel_1);
      lv_table_set_cell_value(gpsTabtable, 2, 0, statusTableLabel_2);
      lv_table_set_cell_value(gpsTabtable, 3, 0, statusTableLabel_3);
      lv_table_set_cell_value(gpsTabtable, 4, 0, statusTableLabel_4);
      lv_table_set_cell_value(gpsTabtable, 5, 0, statusTableLabel_5);
      lv_table_set_cell_value(gpsTabtable, 6, 0, statusTableLabel_6);
      lv_table_set_cell_value(gpsTabtable, 7, 0, statusTableLabel_7);
      lv_table_set_cell_value(gpsTabtable, 8, 0, statusTableLabel_8);
  
      /*Fill the second column*/
      lv_table_set_cell_value(gpsTabtable, 0, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 1, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 2, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 3, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 4, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 5, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 6, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 7, 1, statusTableLabel_Unknown);
      lv_table_set_cell_value(gpsTabtable, 8, 1, statusTableLabel_Unknown);
  
      lv_table_set_col_width(gpsTabtable, 0, screenWidth/2-10);
      lv_table_set_col_width(gpsTabtable, 1, screenWidth/2-10);
      lv_obj_set_height(gpsTabtable, screenHeight - tabHeight);
      lv_obj_center(gpsTabtable);
      //lv_obj_clear_flag(gpsTabtable, LV_OBJ_FLAG_SCROLLABLE);
  
      /*Add an event callback to to apply some custom drawing*/
      //lv_obj_add_event_cb(gpsTabtable, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
      lv_obj_clear_flag(gpsTab, LV_OBJ_FLAG_SCROLLABLE);
      gpsTabInitialised = true;
    }
    void updateGpsTab()
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
        lv_table_set_cell_value(gpsTabtable, 0, 1, tempstring);
        sprintf_P(tempstring, PSTR("%02d:%02d:%02d"), gps.time.hour(), gps.time.minute(), gps.time.second());
        lv_table_set_cell_value(gpsTabtable, 1, 1, tempstring);
      }
      else
      {
        lv_table_set_cell_value(gpsTabtable, 0, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(gpsTabtable, 1, 1, statusTableLabel_Unknown);
      }
      //Satellites
      sprintf_P(tempstring, PSTR("%02u"), gps.satellites.value());
      lv_table_set_cell_value(gpsTabtable, 2, 1, tempstring);
      //HDOP
      sprintf_P(tempstring, PSTR("%.2f"), gps.hdop.hdop());
      lv_table_set_cell_value(gpsTabtable, 3, 1, tempstring);
      if(gps.location.isValid())
      {
        //Lat
        sprintf_P(tempstring, PSTR("%.6f"), gps.location.lat());
        lv_table_set_cell_value(gpsTabtable, 4, 1, tempstring);
        //Lon
        sprintf_P(tempstring, PSTR("%.6f"), gps.location.lng());
        lv_table_set_cell_value(gpsTabtable, 5, 1, tempstring);
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
          lv_table_set_cell_value(gpsTabtable, 6, 1, tempstring);
          //Course
          sprintf_P(tempstring, PSTR("%.1f"), gps.course.deg());
          lv_table_set_cell_value(gpsTabtable, 7, 1, tempstring);
          lv_table_set_cell_value(gpsTabtable, 8, 1, TinyGPSPlus::cardinal(gps.course.deg()));
        }
        else
        {
          lv_table_set_cell_value(gpsTabtable, 6, 1, statusTableLabel_Unknown);
          lv_table_set_cell_value(gpsTabtable, 7, 1, statusTableLabel_Unknown);
          lv_table_set_cell_value(gpsTabtable, 8, 1, statusTableLabel_Unknown);
        }
      }
      else
      {
        lv_table_set_cell_value(gpsTabtable, 4, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(gpsTabtable, 5, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(gpsTabtable, 6, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(gpsTabtable, 7, 1, statusTableLabel_Unknown);
        lv_table_set_cell_value(gpsTabtable, 8, 1, statusTableLabel_Unknown);
      }
    }
  #endif
  #if defined(LVGL_SUPPORT_SETTINGS_TAB)
    void createSettingsTab()
    {
      uint16_t objectY = 0;
      uint8_t labelSpacing = 20;
      uint8_t dropdownSpacing = 45;
      int16_t rightColumnX = 55;
      int16_t leftColumnX = -rightColumnX;
      int16_t columnWidth = 105;
  
      //Create the tab
      settingsTab = lv_tabview_add_tab(tabview, settingsTabLabel);
  
      //Units label
      lv_obj_t * label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Units");
      lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    
      //Date format label
      label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Date format");
      lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
    
      //Next row
      objectY += labelSpacing;
    
      //Units dropdown
      lv_obj_t * units_dd = lv_dropdown_create(settingsTab);
      lv_dropdown_set_options(units_dd, "Metric\n"
                                        "Imperial");
      lv_obj_align(units_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
      lv_obj_set_width(units_dd, columnWidth);
      lv_obj_add_event_cb(units_dd, units_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(units_dd, units);
    
      //Date format dropdown
      lv_obj_t * dateFormat_dd = lv_dropdown_create(settingsTab);
      lv_dropdown_set_options(dateFormat_dd, "D/M/Y\n"
                                        "M/D/Y");
      lv_obj_align(dateFormat_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      lv_obj_set_width(dateFormat_dd, columnWidth);
      lv_obj_add_event_cb(dateFormat_dd, dateFormat_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(dateFormat_dd, dateFormat);
    
      //Next row
      objectY += dropdownSpacing;
    
      //Sensitivity label
      label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Sensitivity");
      lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    
      //Priority label
      label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Priority");
      lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
    
      //Next row
      objectY += labelSpacing;
    
      //Sensitivity dropdown
      lv_obj_t * sensitivity_dd = lv_dropdown_create(settingsTab);
      lv_dropdown_set_options(sensitivity_dd, "Low\n"
                                              "Medium\n"
                                              "High");
      lv_obj_align(sensitivity_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
      lv_obj_set_width(sensitivity_dd, columnWidth);
      lv_obj_add_event_cb(sensitivity_dd, sensitivity_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(sensitivity_dd, trackingSensitivity);
    
      //Priority dropdown
      lv_obj_t * priority_dd = lv_dropdown_create(settingsTab);
      lv_dropdown_set_options(priority_dd,  "Center\n"
                                            "Edge"
                                            );
      lv_obj_align(priority_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      lv_obj_set_width(priority_dd, columnWidth);
      lv_obj_add_event_cb(priority_dd, priority_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(priority_dd, trackerPriority);
    
      //Next row
      objectY += dropdownSpacing;
    
      //Display timeout label
      label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Display timeout");
      lv_obj_align(label, LV_ALIGN_TOP_MID, leftColumnX, objectY);
    
      //Beeper label
      #if defined(SUPPORT_BEEPER)
        label = lv_label_create(settingsTab);
        lv_label_set_text(label, "Beeper");
        lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      #else
        label = lv_label_create(settingsTab);
        lv_label_set_text(label, "Rotation");
        lv_obj_align(label, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      #endif
    
      //Next row
      objectY += labelSpacing;
    
      //Display timeout dropdown
      lv_obj_t * displayTimeout_dd = lv_dropdown_create(settingsTab);
      lv_dropdown_set_options(displayTimeout_dd, "Never\n"
                                              "1 min\n"
                                              "5 min\n"
                                              "15 min"
                                              );
      lv_obj_align(displayTimeout_dd, LV_ALIGN_TOP_MID, leftColumnX, objectY);
      lv_obj_set_width(displayTimeout_dd, columnWidth);
      lv_obj_add_event_cb(displayTimeout_dd, displayTimeout_dd_event_handler, LV_EVENT_ALL, NULL);
      lv_dropdown_set_selected(displayTimeout_dd, displayTimeout);
    
      //Beeper dropdown
      #if defined(SUPPORT_BEEPER)
        beeper_dd = lv_dropdown_create(settingsTab);
        lv_dropdown_set_options(beeper_dd, "Off\n"
                                            "On"
                                            );
        lv_obj_align(beeper_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
        lv_obj_set_width(beeper_dd, columnWidth);
        lv_obj_add_event_cb(beeper_dd, beeper_dd_event_handler, LV_EVENT_ALL, NULL);
        if(beeperEnabled == false)
        {
          lv_dropdown_set_selected(beeper_dd, 0);
        }
        else
        {
          lv_dropdown_set_selected(beeper_dd, 1);
        }
      #else
        rotation_dd = lv_dropdown_create(settingsTab);
        lv_dropdown_set_options(rotation_dd, "Normal\n"
                                                "Inverted"
                                                );
        lv_obj_align(rotation_dd, LV_ALIGN_TOP_MID, rightColumnX, objectY);
        lv_obj_set_width(rotation_dd, columnWidth);
        lv_obj_add_event_cb(rotation_dd, rotation_dd_event_handler, LV_EVENT_ALL, NULL);
        if(screenRotation == 0)
        {
          lv_dropdown_set_selected(rotation_dd, 0);
        }
        else
        {
          lv_dropdown_set_selected(rotation_dd, 2);
        }
      #endif
    
      //Next row
      objectY += dropdownSpacing;
    
      //Display brightness label
      label = lv_label_create(settingsTab);
      lv_label_set_text(label, "Display brightness range");
      lv_obj_align(label, LV_ALIGN_TOP_MID, 0, objectY);
    
      //Next row
      objectY += labelSpacing;
    
      //Display brightness slider
      displayBrightness_slider = lv_slider_create(settingsTab);
      lv_obj_set_width(displayBrightness_slider, 160);
      lv_obj_align(displayBrightness_slider, LV_ALIGN_TOP_MID, 0, objectY+10);
      lv_slider_set_mode(displayBrightness_slider, LV_SLIDER_MODE_RANGE);
      lv_slider_set_range(displayBrightness_slider, absoluteMinimumBrightnessLevel , absoluteMaximumBrightnessLevel);
      lv_slider_set_value(displayBrightness_slider, maximumBrightnessLevel, LV_ANIM_OFF);
      lv_slider_set_left_value(displayBrightness_slider, minimumBrightnessLevel, LV_ANIM_OFF);  
      lv_obj_add_event_cb(displayBrightness_slider, displayBrightness_slider_event_cb, LV_EVENT_ALL, NULL);
      lv_obj_refresh_ext_draw_size(displayBrightness_slider);
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
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Units: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
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
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Date format: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
        #endif
      }
    }
    static void sensitivity_dd_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      lv_obj_t * obj = lv_event_get_target(e);
      if(code == LV_EVENT_VALUE_CHANGED) {
        trackingSensitivity = (uint8_t)lv_dropdown_get_selected(obj);
        saveConfigurationSoon = millis();
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Sensitivity: %04x %s\r\n"), sensitivityValues[(int)lv_dropdown_get_selected(obj)], buf);
        #endif
      }
    }
    static void priority_dd_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      lv_obj_t * obj = lv_event_get_target(e);
      if(code == LV_EVENT_VALUE_CHANGED) {
        trackerPriority = (uint8_t)lv_dropdown_get_selected(obj);
        if(trackerPriority == 0)
        {
          lv_label_set_text(meter1label0, meter1text0);
        }
        else if(trackerPriority == 0)
        {
          lv_label_set_text(meter1label0, meter1text1);
        }
        saveConfigurationSoon = millis();
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Priority: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
        #endif
      }
    }
    static void displayTimeout_dd_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      lv_obj_t * obj = lv_event_get_target(e);
      if(code == LV_EVENT_VALUE_CHANGED) {
        displayTimeout = (uint8_t)lv_dropdown_get_selected(obj);
        saveConfigurationSoon = millis();
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Display timeout: %u %s\r\n"), displayTimeouts[(int)lv_dropdown_get_selected(obj)], buf);
        #endif
      }
    }
    #if defined(SUPPORT_BEEPER)
      static void beeper_dd_event_handler(lv_event_t * e)
      {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t * obj = lv_event_get_target(e);
        if(code == LV_EVENT_VALUE_CHANGED) {
          if((uint8_t)lv_dropdown_get_selected(obj) == 0)
          {
            disableBeeper();
            endRepeatingBeep();
          }
          else
          {
            enableBeeper();
          }
          saveConfigurationSoon = millis();
          char buf[32];
          lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Beeper: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
          #endif
        }
      }
    #else
      static void rotation_dd_event_handler(lv_event_t * e)
      {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t * obj = lv_event_get_target(e);
        if(code == LV_EVENT_VALUE_CHANGED) {
          if((uint8_t)lv_dropdown_get_selected(obj) == 0)
          {
            screenRotation = 0;
          }
          else
          {
            screenRotation = 2;
          }
          saveConfigurationSoon = millis();
          restartTimer = millis();
          char buf[32];
          lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Rotation: %u %s\r\n"), (int)lv_dropdown_get_selected(obj), buf);
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
          SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Display brightness: %u-%u\r\n"),(int)lv_slider_get_left_value(obj), (int)lv_slider_get_value(obj));
        #endif
      }
    }
  #endif
  #if defined(LVGL_SUPPORT_MAP_TAB)
    void createMapTab()
    {
      //Create the tab
      //mapTab = lv_tabview_add_tab(tabview, mapTabLabel);

      //Create the map canvas
      //mapCanvas = lv_canvas_create(mapTab);
    }
  #endif
  #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
    void createScanInfoTab(void)
    {
      uint16_t objectY = 0;
      uint8_t labelSpacing = 20;
      uint8_t dropdownSpacing = 45;
      uint8_t buttonSpacing = 45;
      int16_t rightColumnX = 55;
      int16_t leftColumnX = -rightColumnX;
      int16_t columnWidth = 105;

      //Create the Tab
      scanInfoTab = lv_tabview_add_tab(tabview, infoTabLabel);

      //
      lv_obj_t * label; //Working throwaway object for labels
      
      //Top label
      label = lv_label_create(scanInfoTab);
      lv_label_set_text(label, "Search for...");
      lv_obj_align(label, LV_ALIGN_TOP_MID, 0, objectY);

      //Next row
      objectY += labelSpacing;
      
      //Nearest button
      button0 = lv_btn_create(scanInfoTab);
      lv_obj_align(button0, LV_ALIGN_TOP_MID, leftColumnX, objectY);
      label = lv_label_create(button0);
      lv_label_set_text(label, button0Label);
      lv_obj_center(label);
      //This button can be toggled to always find the nearest
      //lv_obj_add_flag(button0, LV_OBJ_FLAG_CHECKABLE);
      lv_obj_add_event_cb(button0, button0_event_handler, LV_EVENT_ALL, NULL);

      //Furthest button
      button1 = lv_btn_create(scanInfoTab);
      lv_obj_align(button1, LV_ALIGN_TOP_MID, rightColumnX, objectY);
      label = lv_label_create(button1);
      lv_label_set_text(label, button1Label);
      lv_obj_center(label);
      lv_obj_add_event_cb(button1, button1_event_handler, LV_EVENT_ALL, NULL);

      //Next row
      objectY += buttonSpacing;

      //Devices drop down label
      label = lv_label_create(scanInfoTab);
      lv_label_set_text(label, "...or choose");
      lv_obj_align(label, LV_ALIGN_TOP_MID, 0, objectY);

      //Next row
      objectY += labelSpacing;

      //Devices dropdown
      devices_dd = lv_dropdown_create(scanInfoTab);
      lv_obj_align(devices_dd, LV_ALIGN_TOP_MID, 0, objectY);
      lv_obj_set_width(devices_dd, columnWidth*2);
      lv_dropdown_set_options(devices_dd, "");
      lv_obj_add_event_cb(devices_dd, device_dd_event_handler, LV_EVENT_ALL, NULL);

      //Next row
      objectY += dropdownSpacing;

      //Information label
      //label = lv_label_create(scanInfoTab);
      //lv_label_set_text(label, "Information about...");
      //lv_obj_align(label, LV_ALIGN_TOP_MID, 0, objectY);

      //Next row
      //objectY += labelSpacing;
      //Next row
      //objectY += labelSpacing*2;

      static lv_style_t style;
      lv_style_init(&style);
  
      /*Set a background color and a radius*/
      lv_style_set_radius(&style, 5);
      lv_style_set_bg_opa(&style, LV_OPA_COVER);
      lv_style_set_bg_color(&style, lv_color_white());
  
      /*Add a shadow*/
      //lv_style_set_shadow_width(&style, 55);
      //lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));
      //    lv_style_set_shadow_ofs_x(&style, 10);
      //    lv_style_set_shadow_ofs_y(&style, 20);

      icDescription_label = lv_label_create(scanInfoTab);
      lv_label_set_long_mode(icDescription_label, LV_LABEL_LONG_WRAP);     //Break the long lines
      lv_label_set_recolor(icDescription_label, true);                      //Enable re-coloring by commands in the text
      lv_label_set_text(icDescription_label, "Nothing useful detected");
      lv_obj_set_width(icDescription_label, columnWidth*2);  //Fill whole width
      lv_obj_set_height(icDescription_label, 130);  //Fill bottom of screen
      lv_obj_set_style_text_align(icDescription_label, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_align(icDescription_label, LV_ALIGN_TOP_MID, 0, objectY);
      lv_obj_add_style(icDescription_label, &style, 0);
  
      //Stop the tab scrolling
      lv_obj_clear_flag(scanInfoTab, LV_OBJ_FLAG_SCROLLABLE);  
    }
    static void device_dd_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      lv_obj_t * obj = lv_event_get_target(e);
      if(code == LV_EVENT_VALUE_CHANGED)
      {
        if((uint8_t)lv_dropdown_get_selected(obj) < maximumNumberOfDevices)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.print(F("GUI "));
          #endif
          if(currentTrackingMode != trackingMode::fixed)
          {
            currentTrackingMode = trackingMode::fixed;
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
              SERIAL_DEBUG_PORT.print(F("changed to fixed tracking "));
            #endif
          }
          currentlyTrackedDevice = selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)];
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.print(F("chose "));
          #endif
          //Set the description in the box below
          if(device[selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)]].icDescription != nullptr)
          {
            lv_label_set_text(icDescription_label, device[selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)]].icDescription);
          }
          else
          {
            lv_label_set_text(icDescription_label, "No information available");
          }
          if(device[selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)]].icName != nullptr)
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
              SERIAL_DEBUG_PORT.println(device[selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)]].icName);
            #endif
          }
          else
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
              SERIAL_DEBUG_PORT.printf_P(PSTR("deviceId:02x\r\n"), device[selectDeviceDropdownIndices[(uint8_t)lv_dropdown_get_selected(obj)]].id);
            #endif
          }
        }
      }
    }
    static void button0_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);
      if(code == LV_EVENT_CLICKED)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.print(F("GUI \"Nearest\" clicked"));
        #endif
        if(currentTrackingMode != trackingMode::nearest)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F(", changing mode"));
          #endif
          currentTrackingMode = trackingMode::nearest;
        }
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        else
        {
            SERIAL_DEBUG_PORT.println();
        }
        #endif
      }
    }
    static void button1_event_handler(lv_event_t * e)
    {
      lv_event_code_t code = lv_event_get_code(e);  
      if(code == LV_EVENT_CLICKED)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.print(F("GUI \"Furthest\" clicked"));
        #endif
        if(currentTrackingMode != trackingMode::furthest)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F(", changing mode"));
          #endif
          currentTrackingMode = trackingMode::furthest;
        }
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        else
        {
            SERIAL_DEBUG_PORT.println();
        }
        #endif
      }
    }
    void updateScanInfoTab()
    {
      uint8_t indexToPickAftewards = maximumNumberOfDevices;
      numberOfDevicesInDeviceDropdown = 0;
      String tempDropdownString = "";
      for(uint8_t index = 1; index < numberOfDevices; index++)
      {
        if(device[index].hasGpsFix && rangeToIndicate(index) < maximumEffectiveRange)
        {
          if(index == currentlyTrackedDevice) //Continue tracking after update
          {
            indexToPickAftewards = numberOfDevicesInDeviceDropdown;
          }
          if(numberOfDevicesInDeviceDropdown != 0)
          {
            tempDropdownString += "\n";
          }
          selectDeviceDropdownIndices[numberOfDevicesInDeviceDropdown] = index;
          if(device[index].icName != nullptr)
          {
            tempDropdownString += String(device[index].icName);
          }
          else
          {
            tempDropdownString += "Unknown";
          }
          tempDropdownString += " - ";
          tempDropdownString += String(int(rangeToIndicate(index)));
          tempDropdownString += "m away";
          numberOfDevicesInDeviceDropdown++;
        }
      }
      lv_dropdown_set_options(devices_dd, tempDropdownString.c_str());
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.print(F("GUI updated Scan Info Tab, "));
        if(indexToPickAftewards != maximumNumberOfDevices)
        {
          SERIAL_DEBUG_PORT.print(F(" picked device: "));
          SERIAL_DEBUG_PORT.print(indexToPickAftewards);
        }
        else
        {
          SERIAL_DEBUG_PORT.print(F("nothing selected"));
        }
      #endif
      if(indexToPickAftewards != maximumNumberOfDevices)
      {
        lv_dropdown_set_selected(devices_dd, indexToPickAftewards);
        /*
        if(device[selectDeviceDropdownIndices[currentlyTrackedDevice]].icName != nullptr)
        {
          lv_label_set_text(icName_label, device[currentlyTrackedDevice].icName);
        }
        else
        {
          lv_label_set_text(icName_label, "Unknown");
        }
        */
        if(device[selectDeviceDropdownIndices[indexToPickAftewards]].icDescription != nullptr)
        {
          lv_label_set_text(icDescription_label, device[selectDeviceDropdownIndices[indexToPickAftewards]].icDescription);
        }
        else
        {
          lv_label_set_text(icDescription_label, "No information available");
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            //SERIAL_DEBUG_PORT.print(F(" icDescription == nullptr -> No information available"));
          #endif
        }
      }
      else
      {
        lv_label_set_text(icDescription_label, "Nothing useful detected");
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          //SERIAL_DEBUG_PORT.print(F(" indexToPickAftewards == maximumNumberOfDevices -> Nothing useful detected"));
        #endif
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
        SERIAL_DEBUG_PORT.println();
      #endif
    }
    
  #endif
  void manageLVGL()
  {
    if(currentLvglUiState == deviceState::starting)
    {
      if(millis() > 10E3)
      {
        lv_label_set_text(status_label, statusLabel_1);
        currentLvglUiState = deviceState::detectingGpsPins;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::detectingGpsPins"));
        #endif
      }
    }
    else if(currentLvglUiState == deviceState::detectingGpsPins)
    {
      if(millis() > 12E3)
      {
        if(touchScreenMinimumX == 0 && touchScreenMaximumX == 0 && touchScreenMinimumY == 0 && touchScreenMaximumY == 0)
        {
          lv_label_set_text(status_label, statusLabel_3);
          currentLvglUiState = deviceState::calibrateScreen;
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F("deviceState::calibrateScreen"));
          #endif
        }
        else
        {
          lv_label_set_text(status_label, statusLabel_2);
          currentLvglUiState = deviceState::detectingGpsBaudRate;
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F("deviceState::detectingGpsBaudRate"));
          #endif
        }
      }
    }
    else if(currentLvglUiState == deviceState::calibrateScreen)
    {
      if(touchScreenMinimumX != 0 && touchScreenMaximumX != 0 && touchScreenMinimumY != 0 && touchScreenMaximumY != 0 && millis() - lastUiActivity > 5E3) //Wait for 5s pause
      {
        lv_label_set_text(status_label, statusLabel_3);
        currentLvglUiState = deviceState::detectingGpsBaudRate;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::detectingGpsBaudRate"));
        #endif
      }
    }
    else if(currentLvglUiState == deviceState::detectingGpsBaudRate)
    {
      if(millis() > 14E3)
      {
        lv_label_set_text(status_label, statusLabel_4);
        currentLvglUiState = deviceState::gpsDetected;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::gpsDetected"));
        #endif
      }
    }
    else if(currentLvglUiState == deviceState::gpsDetected)
    {
      if(millis() > 16E3 && device[0].hasGpsFix == true)
      {
        lv_label_set_text(status_label, statusLabel_5);
        currentLvglUiState = deviceState::gpsLocked;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::gpsLocked"));
        #endif
      }
    }
    else if(currentLvglUiState == deviceState::gpsLocked)
    {
      if(device[0].hasGpsFix == false)  //Lost location
      {
        lv_label_set_text(status_label, statusLabel_4);
        currentLvglUiState == deviceState::gpsDetected;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::gpsDetected"));
        #endif
      }
      else
      {
        if(currentlyTrackedDevice != maximumNumberOfDevices)  //Found a beacon
        {
          if(metersShowing == false)
          {
            hideStatusSpinner();
            showMeters();
            currentLvglUiState = deviceState::tracking;
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
              SERIAL_DEBUG_PORT.println(F("deviceState::tracking"));
            #endif
          }
        }
      }
    }
    else if(currentLvglUiState == deviceState::tracking)
    {
      if(millis() - lastLvglTabUpdate > lvglTabUpdateInterval)
      {
        lastLvglTabUpdate = millis();
        #if defined(LVGL_SUPPORT_HOME_TAB)
          if(enableHomeTab && homeTabInitialised)
          {
            updateHomeTab();
          }
        #endif
        #if defined(LVGL_SUPPORT_GPS_TAB)
          if(enableGpsTab && gpsTabInitialised)
          {
            updateGpsTab();
          }
        #endif
      }
      if(device[0].hasGpsFix == false)  //Lost location
      {
        if(metersShowing == true)
        {
          hideMeters();
          showStatusSpinner();
        }
        lv_label_set_text(status_label, statusLabel_3);
        currentLvglUiState = deviceState::gpsDetected;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::gpsDetected"));
        #endif
      }
      else if(currentlyTrackedDevice == maximumNumberOfDevices) //Lost all beacons
      {
        if(metersShowing == true)
        {
          hideMeters();
          showStatusSpinner();
        }
        currentLvglUiState = deviceState::gpsLocked;
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
          SERIAL_DEBUG_PORT.println(F("deviceState::gpsLocked"));
        #endif
      }
    }
    #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
      if(enableInfoTab)
      {
        if(millis() - lastScanInfoTabUpdate > 1E3 && findableDevicesChanged == true)
        {
          lastScanInfoTabUpdate = millis();
          findableDevicesChanged = false;
          updateScanInfoTab();
        }
      }
    #endif
    if(displayTimeouts[displayTimeout] > 0)
    {
      if(millis() - lastUiActivity > displayTimeouts[displayTimeout])
      {
        if(uiActive == true)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F("LvglUi inactive"));
          #endif
          uiActive = false;
        }
      }
      else
      {
        if(uiActive == false)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
            SERIAL_DEBUG_PORT.println(F("LvglUi active"));
          #endif
          uiActive = true;
        }
      }
    }
    lv_task_handler();
  }
  #if defined(LVGL_MANAGE_BACKLIGHT)
    void setupBacklight()
    {
      pinMode(ldrPin, INPUT);
      ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
      ledcAttachPin(lcdBacklightPin, LEDC_CHANNEL_0);
    }
    void manageBacklight()
    {
      if(millis() - backlightLastSet > backlightChangeInterval)
      {
        backlightLastSet = millis();
        uint8_t wantedBrightnessLevel = map(1024 - analogRead(ldrPin), 0, 1024, minimumBrightnessLevel, maximumBrightnessLevel);
        if(abs((int16_t)currentBrightnessLevel - (int16_t)wantedBrightnessLevel) > 20)
        {
          if(wantedBrightnessLevel > currentBrightnessLevel)
          {
            currentBrightnessLevel++;
          }
          else if(wantedBrightnessLevel < currentBrightnessLevel)
          {
            currentBrightnessLevel--;
          }
        }
        if(uiActive == true)
        {
          ledcWrite(LEDC_CHANNEL_0, (4095 / absoluteMaximumBrightnessLevel) * currentBrightnessLevel);
        }
        else
        {
          ledcWrite(LEDC_CHANNEL_0, (4095 / absoluteMaximumBrightnessLevel) * uiInactiveBrightnessLevel);
        }
      }
    }
  #else
    void setupBacklight()
    {
      pinMode(ldrPin, INPUT); //Maybe it could get used for something
      pinMode(lcdBacklightPin, OUTPUT);
      digitalWrite(lcdBacklightPin, HIGH); //Just set it full on all the time
    }
  #endif
  #if defined(SUPPORT_TOUCHSCREEN)
    #if defined(SUPPORT_TOUCHSCREEN_BITBANG)
      void setupTouchscreen()
      {
        localLog(F("Starting bitbang touchscreen: "));
        pinMode(XPT2046_MOSI, OUTPUT);
        pinMode(XPT2046_MISO, INPUT);
        pinMode(XPT2046_CLK, OUTPUT);
        pinMode(XPT2046_CS, OUTPUT);
        pinMode(XPT2046_IRQ, INPUT);
        digitalWrite(XPT2046_CS, HIGH);
        digitalWrite(XPT2046_CLK, LOW);
        localLogLn(F("OK"));
        touchscreenInitialised = true;
      }
    #else
      void setupTouchscreen()
      {
        localLog(F("Starting touchscreen: "));
        touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); //Start SPI for touchscreen
        touchscreen.begin(touchscreenSPI); //Initialise the touchscreen
        touchscreen.setRotation(screenRotation); //Rotation is not fixed
        localLog(F("OK"));
        touchscreenInitialised = true;
      }
    #endif
  #endif
  uint8_t countBits(uint16_t thingToCount)
  {
    uint8_t total = 0;
    for(uint8_t bit = 0; bit < 16; bit++)
    {
      if(thingToCount & (uint16_t)(pow(2, bit)))
      {
        total++;
      }
    }
    return total;
  }
#endif
