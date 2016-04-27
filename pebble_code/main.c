#include <pebble.h>
#include <time.h>
#include "player.h"
#include <stdlib.h>

#define FRAME_PAUSE_IN_MS 750
#define STATS_WAIT_MS 1500


static TextLayer *hello_layer, *stats_layer;
static char msg[100], msg2[100];
static int temp_curr_mult, temp_curr, temp_max, temp_min, temp_avg;

static Window *s_game_window, *s_stats_window;
static Layer *s_player_layer, *wall_layer;

static Player thePlayer;
static int last_ten_y[10] = {56,59,62,65,68,71,74,77,80,83};
static GRect window_frame;
static int TEMP_MULTIPLIER = 100;


// PLAYER FUNCTIONS

static void player_init(Player *player) {  
  GRect frame = window_frame;
  player->pos.x = frame.size.w / 2;
  player->pos.y = frame.size.h / 2;
  player->vel.x = 0;
  player->vel.y = 5;
  player->radius = 10;
}

static int convert_temp_to_pixel(int temp, int min_temp, int max_temp, int min_pix, int max_pix){
  int pos_norm_100 = ((temp - min_temp)*100)/(max_temp-min_temp); // should be from 0-100
  printf("temp %d pos_norm_100 %d\n", temp, pos_norm_100);
  int pix_y = max_pix - (pos_norm_100*(max_pix - min_pix)/100);
  //printf("pos_norm_100 %d pix_y %d\n", (int)pos_norm_100, pix_y);
  if(pix_y < min_pix) pix_y = min_pix;
  if(pix_y > max_pix) pix_y = max_pix;
  return pix_y;
  //return max_pix;
}

static void player_update(Player *player) {  

  // update Player position
  int max_display_temp = temp_avg + 10;
  int min_display_temp = temp_avg - 10;

  player->pos.y = convert_temp_to_pixel(temp_curr_mult, min_display_temp*TEMP_MULTIPLIER, max_display_temp*TEMP_MULTIPLIER, 25, 150);
  
  // UPDATE PATH
  last_ten_y[0] = last_ten_y[1];
  last_ten_y[1] = last_ten_y[2];
  last_ten_y[2] = last_ten_y[3];
  last_ten_y[3] = last_ten_y[4];
  last_ten_y[4] = last_ten_y[5];
  last_ten_y[5] = last_ten_y[6];
  last_ten_y[6] = last_ten_y[7];
  last_ten_y[7] = last_ten_y[8];
  last_ten_y[8] = last_ten_y[9];
  last_ten_y[9] = player->pos.y;
}

static void player_draw(GContext *ctx, Player *player) {
  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, GPoint(player->pos.x, player->pos.y), player->radius);
}

static void player_layer_update_callback(Layer *me, GContext *ctx) {
  player_draw(ctx, &thePlayer);
}

static void wall_draw(GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);

  GRect frame = window_frame;
  graphics_fill_rect(ctx, GRect(0,10,frame.size.w,5), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0,frame.size.h - 15, frame.size.w,5), 0, GCornerNone);
}

static void path_draw(GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  for(int i = 0; i < 10; i++) {
    graphics_fill_rect(ctx, GRect(i*7,last_ten_y[i] - 5, 6, i+1), 0, 0);
  }
}

static void wall_layer_update_callback(Layer *me, GContext *ctx) {
  wall_draw(ctx);
  path_draw(ctx);
}


// TEMP FUNCTIONS

void out_sent_handler(DictionaryIterator *sent, void *context) {
    // outgoing message was delivered -- do nothing
    //int x = 1;
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    // outgoing message failed
    text_layer_set_text(hello_layer, "Error sending message to server!");
}

void in_received_handler(DictionaryIterator *received, void *context){
    // looks for key #0 in the incoming message
    Tuple *text_tuple;

    text_tuple = dict_find(received, 0);
    if (text_tuple) {
        if (text_tuple->value) {
            strcpy(msg, text_tuple->value->cstring); //update message
            temp_curr_mult = atoi(msg);
            temp_curr = atoi(msg)/TEMP_MULTIPLIER;
        }
        else strcpy(msg, "no value!");      
    }
    else {
        text_layer_set_text(hello_layer, "no message!");
    }

    text_tuple = dict_find(received, 1);
    if (text_tuple) {
        if (text_tuple->value) {
            strcpy(msg2, text_tuple->value->cstring); //update message
            printf("Setting temp_max\n");
            temp_max = atoi(msg2)/TEMP_MULTIPLIER;
        }
        else strcpy(msg2, "no value!");      
    }
    text_tuple = dict_find(received, 2);
    if (text_tuple) {
        printf("Setting temp_min\n");
        if (text_tuple->value) {
            strcpy(msg2, text_tuple->value->cstring); //update message
            printf("Setting temp_min\n");
            temp_min = atoi(msg2)/TEMP_MULTIPLIER;
        }
        else strcpy(msg2, "no value!");      
    }
    text_tuple = dict_find(received, 3);
    if (text_tuple) {
        if (text_tuple->value) {
            strcpy(msg2, text_tuple->value->cstring); //update message
            printf("Setting temp_avg\n");
            temp_avg = atoi(msg2)/TEMP_MULTIPLIER;
        }
        else strcpy(msg2, "no value!");      
    }

    else {
        text_layer_set_text(hello_layer, "no message2!");
    }

    text_layer_set_text(hello_layer, msg);
}

void in_dropped_handler(AppMessageResult reason, void *context) {
    // incoming message dropped
    text_layer_set_text(hello_layer, "Error in!");
}

static void timer_callback(void *data) {

  Player *player = &thePlayer;
  player_update(player); 
  
  layer_mark_dirty(s_player_layer);
  
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    int key = 0;
    // send the message "hello?" to the phone, using key #0
    Tuplet value = TupletCString(key, "hello?");
    dict_write_tuplet(iter, &value);

    //COMMENTED FOR TESTING ONLY
    app_message_outbox_send();  
   
   app_timer_register(FRAME_PAUSE_IN_MS, timer_callback, NULL);
}

static void game_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_bounds(window_layer);
  window_frame = layer_get_frame(window_layer);
  
  // 1. Create layer
  wall_layer = layer_create(frame);
  s_player_layer = layer_create(frame);
  hello_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { frame.size.w, 20 } });
  
  text_layer_set_text_alignment(hello_layer, GTextAlignmentCenter);
  
  // 2. Set Update Procedure for layer
  layer_set_update_proc(s_player_layer, player_layer_update_callback);
  layer_set_update_proc(wall_layer, wall_layer_update_callback);  
  
  // 3. Add layer to window
  layer_add_child(window_layer, wall_layer);
  layer_add_child(window_layer, s_player_layer);
  layer_add_child(window_layer, text_layer_get_layer(hello_layer));
  
  // 4. Init Values
  player_init(&thePlayer); 
}

static void game_window_unload(Window *window) {
  layer_destroy(s_player_layer);
  layer_destroy(wall_layer);
  text_layer_destroy(hello_layer);
}


///////STATS FUNCS////////////////
void remove_stats_window(){
  window_stack_pop(true);  
}


char stats_text[100]; //put outside function so it is global
/* This is called when the select button is clicked */
void select_click_handler(ClickRecognizerRef recognizer, void *context){
  snprintf(stats_text, 100, "Current Temp: %d \nMax: %d\nMin: %d \nAvg: %d", temp_curr, temp_max, temp_min, temp_avg);
  text_layer_set_text(stats_layer, stats_text);
  window_stack_push(s_stats_window, true);
  app_timer_register(STATS_WAIT_MS, remove_stats_window, NULL);
}

void config_provider(void *context) {
    //subscribe button to button click handler
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}



static void init(void) {

  // need this for adding the listener
  // for registering AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);



  s_game_window = window_create();
  window_set_background_color(s_game_window, GColorBlack);
  window_set_window_handlers(s_game_window, (WindowHandlers) {
    .load = game_window_load,
    .unload = game_window_unload
  });
  
  window_stack_push(s_game_window, true);

  srand(time(NULL));
  
  app_timer_register(FRAME_PAUSE_IN_MS, timer_callback, NULL);  


  //// set up stats window /////
  s_stats_window = window_create();
  stats_layer = text_layer_create(GRect(0, 0, 144, 80));
  layer_add_child(window_get_root_layer(s_stats_window), text_layer_get_layer(stats_layer));
  window_set_click_config_provider(s_game_window, config_provider);
}

static void deinit(void) {
    //destroy the window
    window_destroy(s_game_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}








