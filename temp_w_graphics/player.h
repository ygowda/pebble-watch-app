#pragma once

typedef struct Vec2d {
  double x;
  double y;
} Vec2d;

typedef struct Player {
  Vec2d pos;
  Vec2d vel;
  double radius;
} Player;