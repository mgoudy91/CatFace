#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_time_font;

static void battery_handler(BatteryChargeState new_state);

static void main_window_load(Window *window){
  
  // Create GBitmap, then set tp created bitmap layer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_background_cat);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
    
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect (0,100,144,50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect (5, 150, 100, 30));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  
  // Create battery TextLayer
  s_battery_layer = text_layer_create(GRect(0, 150, 140, 30));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);

  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
  // Add layers as a children to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));

}

static void main_window_unload(Window *window){
  // Destroy Gbitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy bitmap layer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy time TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy battery TextLayer
  text_layer_destroy(s_battery_layer);
  
  //Unload font
  fonts_unload_custom_font(s_time_font);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create buffers
  static char timebuffer[] = "00:00";
  static char datebuffer[] = "Jan 01 2001";


  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(timebuffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(timebuffer, sizeof("00:00"), "%I:%M", tick_time);
    if(strcmp(&timebuffer[0], "0") == 0){
      //remove first char
      memmove(timebuffer, timebuffer+1, strlen(timebuffer));
    }
  }
  
  strftime(datebuffer, sizeof("Jan 01 2001"), "%a, %h %d", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, timebuffer);
  text_layer_set_text(s_date_layer, datebuffer);
}

static void tick_hander(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[32];
  if(new_state.charge_percent > 70){
    strcpy(s_battery_buffer, "Batt :)");
  }else if (new_state.charge_percent > 50){
    strcpy(s_battery_buffer, "Batt :|");
  }else if (new_state.charge_percent > 20){
    strcpy(s_battery_buffer, "Batt :(");
  }else{
    strcpy(s_battery_buffer, "Batt >:(");
  }
  
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_hander);
  
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}