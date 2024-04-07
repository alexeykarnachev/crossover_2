#include <algorithm>
#include <cfloat>

#include "raylib.h"
#include "raymath.h"

#include "geometry.hpp"

Vector2 get_orientation_vec(float orientation) {
    return {std::cos(orientation), std::sin(orientation)};
}

float get_vec_orientation(Vector2 vec) {
    return std::atan2(vec.y, vec.x);
}

Vector2 rotate_vec_90(Vector2 v) {
    return {-v.y, v.x};
}

Vector2 flip_vec(Vector2 v) {
    return {-v.x, -v.y};
}

static Vector2 get_circle_proj_bound(Vector2 position, float radius, Vector2 axis) {
    axis = Vector2Normalize(axis);
    Vector2 r = Vector2Scale(axis, radius);
    float k0 = Vector2DotProduct(Vector2Subtract(position, r), axis);
    float k1 = Vector2DotProduct(Vector2Add(position, r), axis);
    return {k0, k1};
}

static Vector2 get_polygon_proj_bound(Vector2 vertices[], int n, Vector2 axis) {
    axis = Vector2Normalize(axis);

    float min_k = HUGE_VAL;
    float max_k = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = Vector2DotProduct(vertices[i], axis);
        min_k = std::min(min_k, k);
        max_k = std::max(max_k, k);
    }
    return {min_k, max_k};
}

static void update_overlap(
    Vector2 bound0,
    Vector2 bound1,
    Vector2 axis,
    Vector2 *min_overlap_axis,
    float *min_overlap
) {
    float r0 = 0.5 * (bound0.y - bound0.x);
    float r1 = 0.5 * (bound1.y - bound1.x);
    float c0 = 0.5 * (bound0.y + bound0.x);
    float c1 = 0.5 * (bound1.y + bound1.x);
    float radii_sum = r0 + r1;
    float dist = fabs(c1 - c0);
    float overlap = radii_sum - dist;
    if (overlap < *min_overlap) {
        *min_overlap = overlap;
        *min_overlap_axis = c1 - c0 < 0 ? flip_vec(axis) : axis;
    }
}

Vector2 get_circle_polygon_mtv(
    Vector2 position, float radius, Vector2 vertices[], int n
) {
    Vector2 nearest_vertex;
    Vector2 min_overlap_axis;
    float nearest_dist = FLT_MAX;
    float min_overlap = FLT_MAX;
    for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
        Vector2 v0 = vertices[vertex_idx];
        Vector2 v1 = vertices[vertex_idx < n - 1 ? vertex_idx + 1 : 0];
        Vector2 axis = Vector2Normalize(rotate_vec_90(Vector2Subtract(v1, v0)));
        Vector2 bound0 = get_polygon_proj_bound(vertices, n, axis);
        Vector2 bound1 = get_circle_proj_bound(position, radius, axis);
        update_overlap(bound0, bound1, axis, &min_overlap_axis, &min_overlap);

        float curr_dist = Vector2Distance(v0, position);
        if (curr_dist < nearest_dist) {
            nearest_dist = curr_dist;
            nearest_vertex = v0;
        }
    }

    Vector2 axis = Vector2Normalize(Vector2Subtract(position, nearest_vertex));
    Vector2 bound0 = get_polygon_proj_bound(vertices, n, axis);
    Vector2 bound1 = get_circle_proj_bound(position, radius, axis);
    update_overlap(bound0, bound1, axis, &min_overlap_axis, &min_overlap);

    min_overlap = std::max(0.0f, min_overlap);
    Vector2 mtv = Vector2Scale(min_overlap_axis, min_overlap);

    return mtv;
}

Vector2 get_circle_circle_mtv(
    Vector2 position0, float radius0, Vector2 position1, float radius1
) {
    Vector2 axis = Vector2Subtract(position1, position0);
    float dist = Vector2Length(axis);
    axis = Vector2Normalize(axis);
    float radii_sum = radius0 + radius1;

    Vector2 mtv = Vector2Zero();
    if (dist < radii_sum) {
        Vector2 dp = Vector2Subtract(position1, position0);
        Vector2 dir = Vector2Length(dp) > EPSILON ? Vector2Normalize(dp)
                                                  : (Vector2){1.0, 0.0};
        mtv = Vector2Scale(dir, dist - radii_sum);
    }

    return mtv;
}

Vector2 get_circle_rect_mtv(Vector2 position, float radius, Rectangle rect) {
    Vector2 vertices[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height},
    };
    return get_circle_polygon_mtv(position, radius, vertices, 4);
}

int get_line_line_intersection(
    Vector2 start0, Vector2 end0, Vector2 start1, Vector2 end1, Vector2 *intersection
) {
    Vector2 d0 = Vector2Subtract(end0, start0);
    Vector2 d1 = Vector2Subtract(end1, start1);

    Vector2 r = Vector2Subtract(start1, start0);

    float c = d0.x * d1.y - d0.y * d1.x;
    if (fabs(c) < EPSILON) {
        return 0;
    }

    float t0 = (r.x * d1.y - r.y * d1.x) / c;
    float t1 = (r.x * d0.y - r.y * d0.x) / c;

    if (t0 >= 0 && t0 <= 1 && t1 >= 0 && t1 <= 1) {
        *intersection = Vector2Add(start0, Vector2Scale(d0, t0));
        return 1;
    }

    return 0;
}

int get_line_circle_intersections(
    Vector2 start, Vector2 end, Vector2 position, float radius, Vector2 intersections[2]
) {
    float x1 = start.x;
    float y1 = start.y;
    float x2 = end.x;
    float y2 = end.y;
    float cx = position.x;
    float cy = position.y;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float a = dx * dx + dy * dy;
    float b = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
    float c = cx * cx + cy * cy + x1 * x1 + y1 * y1 - 2 * (cx * x1 + cy * y1)
              - radius * radius;

    float det = b * b - 4 * a * c;
    int n_points = 0;
    float t1 = 2.0;
    float t2 = 2.0;
    if (det == 0) {
        t1 = -b / (2 * a);
    } else if (det > 0) {
        t1 = (-b + sqrt(det)) / (2 * a);
        t2 = (-b - sqrt(det)) / (2 * a);
    }

    if (t1 >= 0.0 && t1 <= 1.0) {
        intersections[n_points++] = {x1 + t1 * dx, y1 + t1 * dy};
    }
    if (t2 >= 0.0 && t2 <= 1.0) {
        intersections[n_points++] = {x1 + t2 * dx, y1 + t2 * dy};
    }

    return n_points;
}

int get_line_circle_intersection_nearest(
    Vector2 start, Vector2 end, Vector2 position, float radius, Vector2 *intersection
) {
    Vector2 intersections[2];
    int n_intersects = get_line_circle_intersections(
        start, end, position, radius, intersections
    );
    if (n_intersects <= 1) {
        *intersection = intersections[0];
        return n_intersects;
    } else if (Vector2Distance(start, intersections[0]) < Vector2Distance(start, intersections[1])) {
        *intersection = intersections[0];
    } else {
        *intersection = intersections[1];
    }

    return 1;
}

int get_line_polygon_intersection_nearest(
    Vector2 start, Vector2 end, Vector2 vertices[], int n, Vector2 *intersection
) {
    float nearest_dist = HUGE_VAL;
    Vector2 nearest_point;

    for (int i = 0; i < n; ++i) {
        Vector2 s1 = vertices[i];
        Vector2 e1 = vertices[i < n - 1 ? i + 1 : 0];
        if (get_line_line_intersection(start, end, s1, e1, intersection)) {
            float curr_dist = Vector2Distance(start, *intersection);
            if (curr_dist < nearest_dist) {
                nearest_point = *intersection;
                nearest_dist = curr_dist;
            }
        }
    }

    if (nearest_dist < HUGE_VAL) {
        *intersection = nearest_point;
        return 1;
    }

    return 0;
}

int get_line_rect_intersection_nearest(
    Vector2 start, Vector2 end, Rectangle rect, Vector2 *intersection
) {
    Vector2 vertices[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height},
    };
    return get_line_polygon_intersection_nearest(start, end, vertices, 4, intersection);
}

RaysFan get_rays_fan(
    Vector2 start, int n, float length, float span_angle, float orientation
) {
    n = std::min(n, MAX_N_RAYS_IN_RAYS_FAN);

    RaysFan fan;
    fan.n = n;
    fan.start = start;

    Vector2 first_ray = Vector2Rotate({length, 0.0}, orientation);

    if (n == 1) {
        fan.end[0] = {first_ray.x + start.x, first_ray.y + start.y};
    } else {
        float step = span_angle / (n - 1);
        for (int i = 0; i < n; ++i) {
            float angle = -0.5 * span_angle + i * step;
            Vector2 ray = Vector2Rotate(first_ray, angle);
            fan.end[i] = {ray.x + start.x, ray.y + start.y};
        }
    }

    return fan;
}
