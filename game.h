#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t b, g, r, a;
} Color;

typedef struct {
    size_t fps;

    uint32_t *display;
    size_t display_width;
    size_t display_height;

    int16_t *audio;
    size_t audio_sample_rate;
    size_t audio_channels;
} Game;

Game game_init(void);
void game_update(void);
void game_key_up(int key);
void game_key_down(int key);

#endif // GAME_H_
