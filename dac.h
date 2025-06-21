#pragma once

typedef struct {
    uint32_t l, r;
} Audio_sample_t;

/* runs on CPU0 */
void audio_init(void);
size_t Audio_get_fifo_count(void);
size_t Audio_get_samples(Audio_sample_t *buf, size_t n);
