#include "inttypes.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024

#define MAX_N_BOIDS 600

static const Color BACKGROUND_COLOR = {20, 20, 20, 255};

typedef struct Boid {
    bool is_spawned;

    Vector2 position;
    Vector2 velocity;
    Vector2 target;

    float radius;
    float mass;
    float min_speed;
    float max_speed;

    float separation_radius;
    float group_radius;

    float separation_score;
    float target_score;
    float cohesion_score;
    float alignment_score;

    Color color;
} Boid;

static Camera2D CAMERA = {
    .offset = {0.5 * SCREEN_WIDTH, 0.5 * SCREEN_HEIGHT},
    .target = {0.0, 0.0},
    .rotation = 0.0,
    .zoom = 3.0,
};

static Boid BOIDS[MAX_N_BOIDS];

int spawn_boid(Vector2 position) {
    for (int i = 0; i < MAX_N_BOIDS; ++i) {
        Boid *boid = &BOIDS[i];
        if (boid->is_spawned) continue;

        boid->is_spawned = true;

        boid->position = position;
        boid->velocity = Vector2Zero();
        boid->target = Vector2Zero();

        boid->radius = 1.0;
        boid->mass = 1.0;
        boid->min_speed = 50.0;
        boid->max_speed = 100.0;

        boid->separation_radius = boid->radius * 10.0;
        boid->group_radius = boid->separation_radius * 1.0;

        boid->separation_score = 15.0;
        boid->target_score = 0.5;
        boid->cohesion_score = 0.8;
        boid->alignment_score = 0.2;

        boid->color = RAYWHITE;

        return i;
    }

    return -1;
}

// -1 to 1
float get_random_value(void) {
    float val = GetRandomValue(0, INT32_MAX) / (float)INT32_MAX;
    val = val * 2.0 - 1.0;
    return val;
}

void load(void) {
    // load window
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Boids Demo");
    SetTargetFPS(60);

    // load boids
    for (int i = 0; i < MAX_N_BOIDS; ++i) {
        Vector2 position = {
            .x = get_random_value(),
            .y = get_random_value(),
        };
        spawn_boid(position);
    }
}

void update(void) {
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_N_BOIDS; ++i) {
        Boid *boid = &BOIDS[i];
        if (!boid->is_spawned) continue;

        boid->target = GetScreenToWorld2D(GetMousePosition(), CAMERA);

        Vector2 target_vec = Vector2Subtract(boid->target, boid->position);
        Vector2 separation_vec = Vector2Zero();

        Vector2 group_center = Vector2Scale(boid->position, boid->mass);
        Vector2 group_velocity = boid->velocity;
        float group_mass = boid->mass;

        for (int j = 0; j < MAX_N_BOIDS; ++j) {
            Boid *other = &BOIDS[j];
            if (!other->is_spawned) continue;
            if (boid == other) continue;

            float distance = Vector2Distance(boid->position, other->position);
            Vector2 direction = Vector2Normalize(
                Vector2Subtract(boid->position, other->position)
            );

            float influence = 1.0 / (1.0 + distance * distance);

            if (distance <= boid->separation_radius) {
                separation_vec = Vector2Add(
                    separation_vec, Vector2Scale(direction, influence)
                );
            }

            if (distance <= boid->group_radius) {
                group_mass += other->mass;
                group_center = Vector2Add(
                    group_center, Vector2Scale(other->position, other->mass)
                );
                group_velocity = Vector2Add(group_velocity, other->velocity);
            }
        }

        group_center = Vector2Scale(group_center, 1.0 / group_mass);

        Vector2 group_direction = Vector2Normalize(group_velocity);
        float angle = Vector2Angle(boid->velocity, group_direction);
        float angle_step = boid->alignment_score * dt;

        if (angle_step < fabsf(angle)) {
            angle_step = angle < 0.0 ? -angle_step : angle_step;
        } else {
            angle_step = angle;
        }

        boid->velocity = Vector2Rotate(boid->velocity, angle_step);

        Vector2 cohesion_vec = Vector2Subtract(group_center, boid->position);

        Vector2 damping_force = Vector2Scale(boid->velocity, -1.0);
        Vector2 separation_force = Vector2Scale(separation_vec, boid->separation_score);
        Vector2 target_force = Vector2Scale(target_vec, boid->target_score);
        Vector2 cohesion_force = Vector2Scale(cohesion_vec, boid->cohesion_score);

        Vector2 net_force = damping_force;
        net_force = Vector2Add(net_force, separation_force);
        net_force = Vector2Add(net_force, target_force);

        // apply forces
        Vector2 acceleration = Vector2Scale(net_force, 1.0 / boid->mass);
        Vector2 velocity_step = Vector2Scale(acceleration, dt);
        boid->velocity = Vector2Add(boid->velocity, velocity_step);

        float speed = Vector2Length(boid->velocity);
        speed = Clamp(speed, boid->min_speed, boid->max_speed);
        boid->velocity = Vector2Scale(Vector2Normalize(boid->velocity), speed);

        Vector2 position_step = Vector2Scale(boid->velocity, dt);
        boid->position = Vector2Add(boid->position, position_step);
    }
}

void draw_boids(void) {
    for (int i = 0; i < MAX_N_BOIDS; ++i) {
        Boid *boid = &BOIDS[i];
        if (!boid->is_spawned) continue;

        DrawCircleV(boid->position, boid->radius, boid->color);
    }
}

void draw(void) {
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    BeginMode2D(CAMERA);

    draw_boids();

    EndMode2D();

    DrawFPS(10, 10);
    EndDrawing();
}

void unload(void) {
    CloseWindow();
}

int main(void) {
    load();

    while (!WindowShouldClose()) {
        update();
        draw();
    }

    unload();
}
