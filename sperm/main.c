#include "pebble.h"
#include "player.h"
#include <time.h>
#include <stdlib.h>

#define FRAME_PAUSE_IN_MS 50

static Window *s_game_window;
static Layer *s_player_layer, *wall_layer;

static Player thePlayer;
static int last_ten_y[10] = {56,59,62,65,68,71,74,77,80,83};
static GRect window_frame;

static void player_init(Player *player) {  
  GRect frame = window_frame;
  player->pos.x = frame.size.w / 2;
  player->pos.y = frame.size.h / 2;
  player->vel.x = 0;
  player->vel.y = 5;
  player->radius = 10;
}

static void player_update(Player *player) {
  GRect frame = window_frame;
  
  // TEMP
  int r = rand() % 10 - 5;
  
  // update Player position
  if(player->pos.y + r >= (15+player->radius) && player->pos.y + r <= frame.size.h - (15+player->radius)) {
    player->pos.y += r;
  }
  
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

static void timer_callback(void *data) {
  Player *player = &thePlayer;
  player_update(player); 
  
  layer_mark_dirty(s_player_layer);

  app_timer_register(FRAME_PAUSE_IN_MS, timer_callback, NULL);
}

static void game_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);
  
  // 1. Create layer
  wall_layer = layer_create(frame);
  s_player_layer = layer_create(frame);
  
  // 2. Set Update Procedure for layer
  layer_set_update_proc(s_player_layer, player_layer_update_callback);
  layer_set_update_proc(wall_layer, wall_layer_update_callback);  
  
  // 3. Add layer to window
  layer_add_child(window_layer, wall_layer);
  layer_add_child(window_layer, s_player_layer);

  // 4. Init Values
  player_init(&thePlayer);
  
}

static void game_window_unload(Window *window) {
  layer_destroy(s_player_layer);
  layer_destroy(wall_layer);
}

static void init(void) {
  
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
  window_destroy(s_game_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
