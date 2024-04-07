#pragma once

#include "raylib.h"

Vector2 get_orientation_vec(float orientation);
float get_vec_orientation(Vector2 vec);
Vector2 rotate_vec_90(Vector2 v);
Vector2 flip_vec(Vector2 v);
Vector2 get_circle_polygon_mtv(
    Vector2 position, float radius, Vector2 vertices[], int n
);
Vector2 get_circle_circle_mtv(
    Vector2 position0, float radius0, Vector2 position1, float radius1
);
Vector2 get_circle_rect_mtv(Vector2 position, float radius, Rectangle rect);
int get_line_line_intersection(
    Vector2 start0, Vector2 end0, Vector2 start1, Vector2 end1, Vector2 *intersection
);
int get_line_circle_intersections(
    Vector2 start, Vector2 end, Vector2 position, float radius, Vector2 intersections[2]
);
int get_line_circle_intersection_nearest(
    Vector2 start, Vector2 end, Vector2 position, float radius, Vector2 *intersection
);
int get_line_polygon_intersection_nearest(
    Vector2 start, Vector2 end, Vector2 vertices[], int n, Vector2 *intersection
);
int get_line_rect_intersection_nearest(
    Vector2 start, Vector2 end, Rectangle rect, Vector2 *intersection
);
