// $$DATE$$ : dim. 31 mars 2019 (15:27:03)

#ifndef DUMB_WITH_OPENAL_H
#define DUMB_WITH_OPENAL_H

#include <internal/it.h> //pour accéder aux membres de DUMB_IT_SIGDATA
#include <dumb.h>

typedef struct {
    // Library contexts
    DUH_SIGRENDERER *renderer;
    DUMB_IT_SIGRENDERER *it_sigrenderer;
    DUMB_IT_SIGDATA *sigdata;
    DUH *duh;
    //SDL_AudioDeviceID dev;
    sample_t **sig_samples;
    long sig_samples_size;
    int nb_channels; //nombre de voies du module
    int nb_orders; //nombre de patterns à jouer

    // Runtime vars
    float delta;
    int spos;   // Samples read
    int ssize;  // Total samples available
    int sbytes; // bytes per sample
    bool ended;

    // Config switches
    int bits;
    int freq;
    int quality;
    int n_channels;
    bool no_progress;
    float volume;
    const char *input;
} streamer_t;

// pour OpenAL
void AL_init();
void AL_exit();
char const* AL_list_devices();

// pour DUMB
streamer_t *DUMB_init_streamer(const char *module_filename);
void DUMB_play( streamer_t *streamer);
void DUMB_stop( streamer_t *streamer);
void DUMB_exit( streamer_t *streamer);
int get_row_data( streamer_t *streamer); //retourne le numéro de ligne

#endif
