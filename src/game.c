#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "math.h"
#include "game.h"
#include "stb_vorbis.c"
#define OLIVEC_IMPLEMENTATION
#include "olive.c"
//#define OBJ_H_
#include "utahTeapot.c"
#include "vec.h"


#define TODO(...) printf("%s\n", #__VA_ARGS__)
#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))


#define FPS              60
#define EPSILON          1e-6
#define DISPLAY_WIDTH    800
#define DISPLAY_HEIGHT   600
#define DELTA_TIME       (1.0f / FPS)
#define BACKGROUND_COLOR 0xFF000000

#define AUDIO_CHANNELS 2
#define AUDIO_SAMPLE_RATE 44100
static_assert(AUDIO_SAMPLE_RATE % FPS == 0, "Sample rate must be divisible by FPS");

#define AUDIO_CAPACITY (AUDIO_SAMPLE_RATE / FPS * AUDIO_CHANNELS)
static int16_t audio[AUDIO_CAPACITY];

// Buffers
static Color  display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static float  zbuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];

static stb_vorbis *ogg = NULL;

typedef enum {
    FACE_V1,
    FACE_V2,
    FACE_V3,
    FACE_VT1,
    FACE_VT2,
    FACE_VT3,
    FACE_VN1,
    FACE_VN2,
    FACE_VN3,
} Face_Index;

static Vector2 project_3d_to_2d(Vector3 v) {
    if (v.z < 0) v.z = -v.z;
    if (v.z < EPSILON) v.z += EPSILON;
    return make_vector2(v.x / v.z, v.y / v.z);
}

static Vector2 project_to_screen(Vector2 v) {
    return make_vector2(
        (v.x + 1) * 0.5f * DISPLAY_WIDTH,
        (1 - (v.y + 1) * 0.5f) * DISPLAY_HEIGHT
    );
}

Game game_init(void) {
    // Load audio
    ogg = stb_vorbis_open_filename("assets/rizz-sound-effect.ogg", NULL, NULL);
    assert(ogg && "Failed to open OGG file");
    assert(ogg->channels == AUDIO_CHANNELS && "ogg channel is not 2");
    assert(ogg->sample_rate == AUDIO_SAMPLE_RATE && "ogg sample rate is not 44100");
    stb_vorbis_seek_start(ogg);
 
    return (Game) {
        .fps = FPS,

        .display = (uint32_t*)display,
        .display_width = DISPLAY_WIDTH,
        .display_height = DISPLAY_HEIGHT,

        .audio = audio,
        .audio_sample_rate = AUDIO_SAMPLE_RATE,
        .audio_channels = AUDIO_CHANNELS,
    };
}

float angle = 0.0f;
float dx = 300.0f;
float dy = 300.0f;
float cx = DISPLAY_WIDTH / 8;
float cy = DISPLAY_HEIGHT / 8;

static Vector3 camera_pos = {0, 0, 1};
static int key_w = 0;
static int key_s = 0;
static int key_a = 0;
static int key_d = 0;
static int key_up = 0;
static int key_down = 0;

void game_update(void){
    // Audio
    stb_vorbis_get_samples_short_interleaved(ogg, AUDIO_CHANNELS, audio, ARRAY_LEN(audio));

    // Camera movement
    float speed = 2.0f * DELTA_TIME;
    if (key_w) camera_pos.z -= speed;
    if (key_s) camera_pos.z += speed;
    if (key_a) camera_pos.x -= speed;
    if (key_d) camera_pos.x += speed;
    if (key_up) camera_pos.y += speed;
    if (key_down) camera_pos.y -= speed;
    // Display
    for (size_t i = 0; i < ARRAY_LEN(display); i++) {
        display[i] = (Color) {
            .r = 0,
            .g = 0,
            .b = 0,
            .a = 255
        };
    }

#if 0
    Olivec_Canvas oc = {
        .pixels = (uint32_t*)display,
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT,
        .stride = DISPLAY_WIDTH,
    };

    size_t vert_count = 3;
    float dangle = 2 * M_PI / vert_count;
    float mag = DISPLAY_WIDTH / 4;

    olivec_triangle3c(
            oc,
            cx + cosf(dangle*0 + angle)*mag, cy + sinf(dangle*0 + angle)*mag,
            cx + cosf(dangle*1 + angle)*mag, cy + sinf(dangle*1 + angle)*mag,
            cx + cosf(dangle*2 + angle)*mag, cy + sinf(dangle*2 + angle)*mag,
            0xFFFF0000,
            0xFF0000FF,
            0xFF00FFFF
        );

    angle += 2 * M_PI * DELTA_TIME;
    float nx = cx + dx * DELTA_TIME;
    if (nx - mag < 0) {
        dx = fabsf(dx);  // bounce right
        cx = mag;        // clamp to boundary
    } else if (nx + mag > DISPLAY_WIDTH) {
        dx = -fabsf(dx); // bounce left
        cx = DISPLAY_WIDTH - mag; // clamp
    } else {
        cx = nx;
    }

    float ny = cy + dy * DELTA_TIME;
    if (ny - mag < 0) {
        dy = fabsf(dy);  // bounce down
        cy = mag;        // clamp
    } else if (ny + mag > DISPLAY_HEIGHT) {
        dy = -fabsf(dy); // bounce up
        cy = DISPLAY_HEIGHT - mag; // clamp
    } else {
        cy = ny;
    }
#else
    Olivec_Canvas oc = olivec_canvas((uint32_t*) display, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);

    // Clear zbuffer
    for (size_t i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++i) zbuffer[i] = 0;

    Vector3 camera = camera_pos;
    for (size_t i = 0; i < faces_count; ++i) {
        int a, b, c;

        a = faces[i][FACE_V1];
        b = faces[i][FACE_V2];
        c = faces[i][FACE_V3];
        Vector3 v1 = rotate_y(make_vector3(vertices[a][0], vertices[a][1], vertices[a][2]), angle);
        Vector3 v2 = rotate_y(make_vector3(vertices[b][0], vertices[b][1], vertices[b][2]), angle);
        Vector3 v3 = rotate_y(make_vector3(vertices[c][0], vertices[c][1], vertices[c][2]), angle);
        v1.x -= camera.x; v1.y -= camera.y; v1.z -= camera.z;
        v2.x -= camera.x; v2.y -= camera.y; v2.z -= camera.z;
        v3.x -= camera.x; v3.y -= camera.y; v3.z -= camera.z;
        v1.z += 1.5; v2.z += 1.5; v3.z += 1.5;

        a = faces[i][FACE_VN1];
        b = faces[i][FACE_VN2];
        c = faces[i][FACE_VN3];
        Vector3 vn1 = rotate_y(make_vector3(normals[a][0], normals[a][1], normals[a][2]), angle);
        Vector3 vn2 = rotate_y(make_vector3(normals[b][0], normals[b][1], normals[b][2]), angle);
        Vector3 vn3 = rotate_y(make_vector3(normals[c][0], normals[c][1], normals[c][2]), angle);
        // Backface Culling
        if (vector3_dot(v1, vn1) > 0.0 &&
            vector3_dot(v2, vn2) > 0.0 &&
            vector3_dot(v3, vn3) > 0.0) continue;


        Vector2 p1 = project_to_screen(project_3d_to_2d(v1));
        Vector2 p2 = project_to_screen(project_3d_to_2d(v2));
        Vector2 p3 = project_to_screen(project_3d_to_2d(v3));

        int x1 = p1.x;
        int x2 = p2.x;
        int x3 = p3.x;
        int y1 = p1.y;
        int y2 = p2.y;
        int y3 = p3.y;
        int lx, hx, ly, hy;
        if (olivec_normalize_triangle(oc.width, oc.height, x1, y1, x2, y2, x3, y3, &lx, &hx, &ly, &hy)) {
            for (int y = ly; y <= hy; ++y) {
                for (int x = lx; x <= hx; ++x) {
                    int u1, u2, det;
                    if (olivec_barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det)) {
                        int u3 = det - u1 - u2;
                        float z = 1/v1.z*u1/det + 1/v2.z*u2/det + 1/v3.z*u3/det;
                        float near = 0.1f;
                        float far = 5.0f;
                        if (1.0f/far < z && z < 1.0f/near && z > zbuffer[y*DISPLAY_WIDTH + x]) {
                            zbuffer[y*DISPLAY_WIDTH + x] = z;
                            OLIVEC_PIXEL(oc, x, y) = mix_colors3(0xFF1818FF, 0xFF18FF18, 0xFFFF1818, u1, u2, det);

                            z = 1.0f/z;
                            if (z >= 1.0) {
                                z -= 1.0;
                                uint32_t v = z*255;
                                if (v > 255) v = 255;
                                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), (v<<(3*8)));
                            }
                        }
                    }
                }
            }
        }
    }
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "camera: %.2f %.2f %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    olivec_text(oc, buffer, 10, 10, olivec_default_font, 3, 0xFFFFFFFF);

    angle += M_PI * DELTA_TIME;
#endif // 0
}

void game_key_up(int key) {
    switch (key) {
        case 'w': key_w = 0; break;
        case 's': key_s = 0; break;
        case 'a': key_a = 0; break;
        case 'd': key_d = 0; break;
        case 0xff52: key_up = 0; break;   // XK_Up
        case 0xff54: key_down = 0; break; // XK_Down
    }
}
void game_key_down(int key) {
    switch (key) {
        case 'w': key_w = 1; break;
        case 's': key_s = 1; break;
        case 'a': key_a = 1; break;
        case 'd': key_d = 1; break;
        case 0xff52: key_up = 1; break;   // XK_Up
        case 0xff54: key_down = 1; break; // XK_Down
    }
}
