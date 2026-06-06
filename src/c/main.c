#include <pebble.h>

static int actualstepcount = 0;

static Window *window;
static Layer *window_layer;

static uint8_t batteryPercent;

static bool appStarted = false;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

static GBitmap *meter_bar_image;
static BitmapLayer *meter_bar_layer;

static GBitmap *steps_image;
static BitmapLayer *steps_layer;

static GBitmap *heart_image;
static BitmapLayer *heart_layer;

static Layer *progress_layer;

static GBitmap *flag_image;
static BitmapLayer *flag_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;

static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

static Layer *big_time_layer;
static Layer *med_time_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

#define TOTAL_DATE_DIGITS 2 
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

#define TOTAL_STEP_DIGITS 5
static GBitmap *step_digits_images[TOTAL_STEP_DIGITS];
static BitmapLayer *step_digits_layers[TOTAL_STEP_DIGITS];

#define TOTAL_BPM_DIGITS 3
static GBitmap *bpm_digits_images[TOTAL_BPM_DIGITS];
static BitmapLayer *bpm_digits_layers[TOTAL_BPM_DIGITS];


const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_TIME_MED_DIGITS 4
static GBitmap *time_med_digits_images[TOTAL_TIME_MED_DIGITS];
static BitmapLayer *time_med_digits_layers[TOTAL_TIME_MED_DIGITS];

const int MED_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MED_0,
  RESOURCE_ID_IMAGE_MED_1,
  RESOURCE_ID_IMAGE_MED_2,
  RESOURCE_ID_IMAGE_MED_3,
  RESOURCE_ID_IMAGE_MED_4,
  RESOURCE_ID_IMAGE_MED_5,
  RESOURCE_ID_IMAGE_MED_6,
  RESOURCE_ID_IMAGE_MED_7,
  RESOURCE_ID_IMAGE_MED_8,
  RESOURCE_ID_IMAGE_MED_9
};

#define TOTAL_TIME_SM_DIGITS 2
static GBitmap *time_sm_digits_images[TOTAL_TIME_SM_DIGITS];
static BitmapLayer *time_sm_digits_layers[TOTAL_TIME_SM_DIGITS];

const int SM_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_SM_0,
  RESOURCE_ID_IMAGE_SM_1,
  RESOURCE_ID_IMAGE_SM_2,
  RESOURCE_ID_IMAGE_SM_3,
  RESOURCE_ID_IMAGE_SM_4,
  RESOURCE_ID_IMAGE_SM_5,
  RESOURCE_ID_IMAGE_SM_6,
  RESOURCE_ID_IMAGE_SM_7,
  RESOURCE_ID_IMAGE_SM_8,
  RESOURCE_ID_IMAGE_SM_9
};


#define TOTAL_BATTERY_PERCENT_DIGITS 3
static GBitmap *battery_percent_image[TOTAL_BATTERY_PERCENT_DIGITS];
static BitmapLayer *battery_percent_layers[TOTAL_BATTERY_PERCENT_DIGITS];

const int TINY_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_TINY_0,
  RESOURCE_ID_IMAGE_TINY_1,
  RESOURCE_ID_IMAGE_TINY_2,
  RESOURCE_ID_IMAGE_TINY_3,
  RESOURCE_ID_IMAGE_TINY_4,
  RESOURCE_ID_IMAGE_TINY_5,
  RESOURCE_ID_IMAGE_TINY_6,
  RESOURCE_ID_IMAGE_TINY_7,
  RESOURCE_ID_IMAGE_TINY_8,
  RESOURCE_ID_IMAGE_TINY_9,
  RESOURCE_ID_IMAGE_TINY_PERCENT
};

typedef struct ClaySettings {
  int twelve;
  int stepgoal;
  int progress;
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Persistent storage key
#define SETTINGS_KEY 1

// Initialize the default settings
static void default_settings() {
  settings.twelve   = 0;
  settings.stepgoal = 5000;
  settings.progress = 0;
}

// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  default_settings();

  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}



void change_background() {
  gbitmap_destroy(background_image);
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_mark_dirty(bitmap_layer_get_layer(background_layer));
}

void change_battery_icon(bool charging) {
  gbitmap_destroy(battery_image);
  if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }
  else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  

  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = gbitmap_get_bounds(*bmp_image).size
  };
	
	bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
	layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
		
  if (old_image != NULL) {
		gbitmap_destroy(old_image);
		old_image = NULL;
  }
}

static void update_battery(BatteryChargeState charge_state) {
  
  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) {
  change_battery_icon(false);
  layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
    for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
      layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), true);
    }  
    return;
  }

  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);
    
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), false);
  }  
  set_container_image(&battery_percent_image[0], battery_percent_layers[0], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent/10], GPoint(89, 69));
  set_container_image(&battery_percent_image[1], battery_percent_layers[1], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent%10], GPoint(95, 69));
  set_container_image(&battery_percent_image[2], battery_percent_layers[2], TINY_IMAGE_RESOURCE_IDS[10], GPoint(101, 69));
 
}

static void toggle_bluetooth_icon(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

void battery_layer_update_callback(Layer *me, GContext* ctx) {        
  //draw the remaining battery percentage
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 5), 0, GCornerNone);
}

void progress_update_proc(Layer *me, GContext* ctx) {        
  //renew the progress bar
  graphics_context_set_fill_color( ctx, GColorBlack );
  graphics_context_set_stroke_color( ctx, GColorBlack );
  //draw outline first
  graphics_draw_round_rect( ctx, GRect(6,6,100,6), 3);
  //draw percentage
  if( actualstepcount >= settings.stepgoal ) {
    //100 percent or more reached so simply draw full bar
    graphics_fill_rect(ctx, GRect(6,6,100,6), 3, GCornersAll);
  } else {
    int percent = 100*actualstepcount / settings.stepgoal;
    graphics_fill_rect(ctx, GRect(6,6,percent,6), 3, GCornersAll);
  }
}


unsigned short get_display_hour(unsigned short hour) {
  return hour;
}

static void update_days(struct tm *tick_time) {
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(60, 87));

  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(108, 87));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(121, 87));
}

static void update_hours(struct tm *tick_time) {
  
  unsigned short display_hour = tick_time->tm_hour;
  
  if( time_format_image != NULL ) {
    gbitmap_destroy(time_format_image);
  }  
  if( settings.twelve == 1 ) {
    if( display_hour > 11 ) {
      time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PM_MODE);
      bitmap_layer_set_bitmap(time_format_layer, time_format_image); 
      layer_set_hidden( bitmap_layer_get_layer( time_format_layer), false );
    } else {
      layer_set_hidden( bitmap_layer_get_layer( time_format_layer), true );
    }
  }
  else {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image); 
    layer_set_hidden( bitmap_layer_get_layer( time_format_layer), false );
  }

  if( settings.twelve == 1 && display_hour > 12 ) {
    display_hour = display_hour - 12;
  }
  
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(11, 110));
	set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(41, 110));

}

static void update_minutes(struct tm *tick_time) {
  
  // update steps
  HealthMetric metric = HealthMetricStepCount; 
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  int steps = 0;
  // Check the metric has data available for today
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available!
    steps = (int)health_service_sum_today(metric);
    actualstepcount = steps;
  }
  
  //debug
  //steps = 2850;
  //actualstepcount = steps;
  
  if( steps > 9999 ) {
    set_container_image(&step_digits_images[4], step_digits_layers[4], DATENUM_IMAGE_RESOURCE_IDS[((steps/10000)%10)], GPoint(69, 14)); 
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[4]), false );
  } else {
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[4]), true );
  }
  if( steps > 999 ) {
    set_container_image(&step_digits_images[3], step_digits_layers[3], DATENUM_IMAGE_RESOURCE_IDS[((steps/1000)%10)], GPoint(82, 14)); 
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[3]), false );
  } else {
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[3]), true );
  }
  if( steps > 99 ) {
    set_container_image(&step_digits_images[2], step_digits_layers[2], DATENUM_IMAGE_RESOURCE_IDS[((steps/100)%10)], GPoint(95, 14)); 
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[2]), false );
  } else {
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[2]), true );
  }
  if( steps > 9 ) {
    set_container_image(&step_digits_images[1], step_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[((steps/10)%10)], GPoint(108, 14)); 
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[1]), false );
  } else {
    layer_set_hidden(bitmap_layer_get_layer(step_digits_layers[1]), true );
  }
  set_container_image(&step_digits_images[0], step_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[(steps%10)], GPoint(121, 14));
  
  // show flag if enabled and already reached
  if( settings.stepgoal > 0 && steps >= settings.stepgoal ) {
     layer_set_hidden(bitmap_layer_get_layer(flag_layer), false);
  } else {
     layer_set_hidden(bitmap_layer_get_layer(flag_layer), true);
  }
  
  
  // update bpm or show progress bar
  if( settings.progress == 1 ) {
    // show bpm
    // disable progress layer and enable bpm layer
    layer_set_hidden(progress_layer, true);
    layer_set_hidden(bitmap_layer_get_layer(heart_layer), false );
    layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[0]), false );
    
    HealthValue val = health_service_peek_current_value(HealthMetricHeartRateBPM);
    uint32_t bpm = (uint32_t)val;
    
    // debug
    //bpm = 68;
    
    if( bpm > 99 ) {
      set_container_image(&bpm_digits_images[2], bpm_digits_layers[2], DATENUM_IMAGE_RESOURCE_IDS[((bpm/100)%10)], GPoint(95, 32)); 
      layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[2]), false );
    }
    else {
      layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[2]), true );
    }
    if( bpm > 9 ) {
      set_container_image(&bpm_digits_images[1], bpm_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[((bpm/10)%10)], GPoint(108, 32)); 
      layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[1]), false );
    } else {
      layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[1]), true );
    }
    set_container_image(&bpm_digits_images[0], bpm_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[(bpm%10)], GPoint(121, 32));
  } else {
    // show step progress bar
    // disable bpm layers first
    layer_set_hidden(bitmap_layer_get_layer(heart_layer), true );
    layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[0]), true );
    layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[1]), true );
    layer_set_hidden(bitmap_layer_get_layer(bpm_digits_layers[2]), true );
    // enable progress layer
    layer_set_hidden(progress_layer, false);
    // redraw layer
    layer_mark_dirty( progress_layer );    
  }
  
  // update minute digits
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(78, 110));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(106, 110));

}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  } 
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  
  // read twenty four hour display setting
  Tuple *twelve_conf = dict_find(iter, MESSAGE_KEY_twelve);
  if( twelve_conf ) { 
    settings.twelve = twelve_conf->value->int32;
  }
  
  // read step goal setting
  Tuple *steps_conf = dict_find(iter, MESSAGE_KEY_steps);
  if( steps_conf ) { 
    char buf[30];
    strcpy( buf, steps_conf->value->cstring);
    settings.stepgoal = atoi( buf );
  }    

  // read progress bar display setting
  Tuple *progress_conf = dict_find(iter, MESSAGE_KEY_progress);
  if( progress_conf ) { 
    settings.progress = progress_conf->value->int32;
    layer_mark_dirty(progress_layer);
  }
  
  save_settings();
  
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT);

}


static void init(void) {
  
  load_settings();
  
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  
  memset(&step_digits_images, 0, sizeof(step_digits_images));
  
  memset(&bpm_digits_images, 0, sizeof(bpm_digits_images));

  memset(&time_med_digits_layers, 0, sizeof(time_med_digits_layers));
  memset(&time_med_digits_images, 0, sizeof(time_med_digits_images));
  
  memset(&time_sm_digits_layers, 0, sizeof(time_sm_digits_layers));
  memset(&time_sm_digits_images, 0, sizeof(time_sm_digits_images));

  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  
  memset(&battery_percent_layers, 0, sizeof(battery_percent_layers));
  memset(&battery_percent_image, 0, sizeof(battery_percent_image));

  window = window_create();
  
  
  //Register AppMessage events
  app_message_register_inbox_received(in_received_handler);
  //Largest possible input and output buffer sizes  
  app_message_open(128,128);

  
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
    
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
  big_time_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, big_time_layer);
  
  med_time_layer = layer_create(layer_get_frame(window_layer));
  layer_add_child(window_layer, med_time_layer);
    
  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEPARATOR);
  GRect frame = (GRect) {
    .origin = { .x = 70, .y = 117 },
    .size = gbitmap_get_bounds(separator_image).size
  };
  separator_layer = bitmap_layer_create(frame);

  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(big_time_layer, bitmap_layer_get_layer(separator_layer));   
  
  meter_bar_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_METER_BAR);
  GRect frame2 = (GRect) {
    .origin = { .x = 17, .y = 69 },
    .size = gbitmap_get_bounds(meter_bar_image).size
  };
  meter_bar_layer = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(meter_bar_layer, meter_bar_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(meter_bar_layer));  

  steps_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STEPS);
  GRect frame6 = (GRect) {
    .origin = { .x = 17, .y = 13 },
    .size = gbitmap_get_bounds(steps_image).size
  };
  steps_layer = bitmap_layer_create(frame6);
  bitmap_layer_set_compositing_mode(steps_layer, GCompOpSet);
  bitmap_layer_set_bitmap(steps_layer, steps_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(steps_layer));  

  flag_image = gbitmap_create_with_resource(RESOURCE_ID_FLAG);
  GRect frame8 = (GRect) {
    .origin = { .x = 37, .y = 13 },
    .size = gbitmap_get_bounds(flag_image).size
  };
  flag_layer = bitmap_layer_create(frame8);
  bitmap_layer_set_compositing_mode(flag_layer, GCompOpSet);
  bitmap_layer_set_bitmap(flag_layer, flag_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(flag_layer));  
  layer_set_hidden(bitmap_layer_get_layer(flag_layer), true);

  heart_image = gbitmap_create_with_resource(RESOURCE_ID_HEART);
  GRect frame7 = (GRect) {
    .origin = { .x = 17, .y = 31 },
    .size = gbitmap_get_bounds(heart_image).size
  };
  heart_layer = bitmap_layer_create(frame7);
  bitmap_layer_set_compositing_mode(heart_layer, GCompOpSet);
  bitmap_layer_set_bitmap(heart_layer, heart_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(heart_layer));  
  
  progress_layer = layer_create(GRect(16,31,144-16,16));
  layer_add_child(window_layer, progress_layer);
  layer_set_update_proc(progress_layer, progress_update_proc);
  
  bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  GRect frame3 = (GRect) {
    .origin = { .x = 33, .y = 69 },
    .size = gbitmap_get_bounds(bluetooth_image).size
  };
  bluetooth_layer = bitmap_layer_create(frame3);
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
  
  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  GRect frame4 = (GRect) {
    .origin = { .x = 111, .y = 69 },
    .size = gbitmap_get_bounds(battery_image).size
  };
  battery_layer = bitmap_layer_create(frame4);
  battery_image_layer = bitmap_layer_create(frame4);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
  
  GRect frame5 = (GRect) {
    .origin = { .x = 17, .y = 94 },
    .size = {.w = 19, .h = 8}
  };
  time_format_layer = bitmap_layer_create(frame5);
  if (settings.twelve == 0) {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image); 
  }
  layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));

  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  day_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(big_time_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_TIME_MED_DIGITS; ++i) {
    time_med_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(med_time_layer, bitmap_layer_get_layer(time_med_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_TIME_SM_DIGITS; ++i) {
    time_sm_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(med_time_layer, bitmap_layer_get_layer(time_sm_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    battery_percent_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_percent_layers[i]));
  }
  for (int i = 0; i < TOTAL_STEP_DIGITS; ++i) {
    step_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(step_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_BPM_DIGITS; ++i) {
    bpm_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(bpm_digits_layers[i]));
  }

  
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  update_battery(battery_state_service_peek());
   
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT);

  appStarted = true;

 	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(&update_battery);

}


static void deinit(void) {
 
  save_settings();
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);
  separator_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(meter_bar_layer));
  bitmap_layer_destroy(meter_bar_layer);
  gbitmap_destroy(meter_bar_image);
  background_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
  bluetooth_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  battery_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);
  time_format_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  day_name_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(steps_layer));
  bitmap_layer_destroy(steps_layer);
  gbitmap_destroy(steps_image);
  steps_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(heart_layer));
  bitmap_layer_destroy(heart_layer);
  gbitmap_destroy(heart_image);
  steps_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(flag_layer));
  bitmap_layer_destroy(flag_layer);
  gbitmap_destroy(flag_image);
  flag_image = NULL;  
  
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    date_digits_images[i] = NULL;
    bitmap_layer_destroy(date_digits_layers[i]);
    date_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    time_digits_images[i] = NULL;
    bitmap_layer_destroy(time_digits_layers[i]);
    time_digits_layers[i] = NULL;
  }
  
  for (int i = 0; i < TOTAL_TIME_MED_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_med_digits_layers[i]));
    gbitmap_destroy(time_med_digits_images[i]);
    time_med_digits_images[i] = NULL;
    bitmap_layer_destroy(time_med_digits_layers[i]);
    time_med_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_TIME_SM_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_sm_digits_layers[i]));
    gbitmap_destroy(time_sm_digits_images[i]);
    time_sm_digits_images[i] = NULL;
    bitmap_layer_destroy(time_sm_digits_layers[i]);
    time_sm_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(battery_percent_layers[i]));
    gbitmap_destroy(battery_percent_image[i]);
    battery_percent_image[i] = NULL;
    bitmap_layer_destroy(battery_percent_layers[i]); 
    battery_percent_layers[i] = NULL;
  } 
  
  for (int i = 0; i < TOTAL_STEP_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(step_digits_layers[i]));
    gbitmap_destroy(step_digits_images[i]);
    step_digits_images[i] = NULL;
    bitmap_layer_destroy(step_digits_layers[i]);
    step_digits_layers[i] = NULL;
  }  

  for (int i = 0; i < TOTAL_BPM_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(bpm_digits_layers[i]));
    gbitmap_destroy(bpm_digits_images[i]);
    bpm_digits_images[i] = NULL;
    bitmap_layer_destroy(bpm_digits_layers[i]);
    bpm_digits_layers[i] = NULL;
  }  
  
  layer_remove_from_parent(big_time_layer);
  big_time_layer = NULL;
  
  layer_remove_from_parent(med_time_layer);
  med_time_layer = NULL;

  layer_remove_from_parent(progress_layer);
  progress_layer = NULL;

  appStarted = NULL;
  
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}