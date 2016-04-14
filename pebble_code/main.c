#include <pebble.h>
#include <time.h>
#include "player.h"
#include <stdlib.h>

#define FRAME_PAUSE_IN_MS 100


static TextLayer *hello_layer;
static char msg[100];
static int curr_temp = 83;

static Window *s_game_window;
static Layer *s_player_layer, *wall_layer;

static Player thePlayer;
static int last_ten_y[10] = {56,59,62,65,68,71,74,77,80,83};
static GRect window_frame;


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


  printf("BEGIN player_update, curr_temp: %d \n", (int)curr_temp);
  // update Player position
  player->pos.y = convert_temp_to_pixel(curr_temp, 20, 40, 25, 150);
  
  
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
    text_layer_set_text(hello_layer, "Error out!");
}

void in_received_handler(DictionaryIterator *received, void *context){
    // looks for key #0 in the incoming message
    int key = 0;    
    Tuple *text_tuple = dict_find(received, key);
    if (text_tuple) {
        if (text_tuple->value) {
            //curr_temp = *(text_tuple->value->data); //WHAT IS THIS? ASK DAN.
            strcpy(msg, text_tuple->value->cstring); //update message
            curr_temp = atoi(msg); //update global value for sperm location
        }
        else strcpy(msg, "no value!");      
        text_layer_set_text(hello_layer, msg);
    }
    else {
        text_layer_set_text(hello_layer, "no message!");
    }
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








