/* Adaptation de DUMB à OpenAL
   $$DATE$$ : mer. 07 février 2018 (15:19:42)

   Ce programme peut fonctionner en stand-alone.
   Il sert également de base pour ma lib OpenAL/DUMB.

   stand-alone:
   g++ dumb_with_openal.cpp dumb/src/dumb.a  -Idumb/include -lopenal -DTEST_JSEB
   Éventuellement il faut -lm
   le «-lm» est là pour éviter le « undefined reference to symbol 'exp@@GLIBC_2.2.5' »
   (cherche la fonction «exp» qui est dans l'archive .a)

  commentaires: dumb@finiderire.com
*/


#include <string.h>
#include "dumb_with_openal.h"


#define REPLAY_FREQ 44100
int buffer_size = REPLAY_FREQ/8; ///8; // pour 44100hz, donne un buffer d'environ 5ko (rempli 50 fois par seconde)

/* ================  LIB d'interface OpenAL (début) =============== */

#include<AL/al.h>
#include<AL/alc.h>
#include<stdio.h>
#include<unistd.h>

typedef struct {
  ALuint id;
  char *data;
} AL_buffer;

struct Error_codes { ALenum err; char const *msg; };

struct Error_codes error_codes[] =
{
  {AL_NO_ERROR, "No error"},
  {AL_INVALID_NAME, "Invalid name (ID)"},
  {AL_INVALID_ENUM, "Invalid enum"},
  {AL_INVALID_VALUE, "Invalid value"},
  {AL_INVALID_OPERATION, "Invalid operation"},
  {AL_OUT_OF_MEMORY, "Not enough memory"},
  {ALC_INVALID_DEVICE, "Invalid device"},
  {ALC_INVALID_CONTEXT, "Invalid context"},
  {0,0},
  {0,"Unknown error"}
};


char const * check_error(char const *details)
{
  ALenum errnum = alGetError();
  char const *msg = 0;
  int i=0;
  while( !msg && error_codes[i].msg) {
    if ( error_codes[i].err == errnum) {
      msg = error_codes[i].msg;
    }
    i++;
  }

  // teste si erreur inconnue
  if ( !msg) { msg = error_codes[i+1].msg; }

  if ( errnum != AL_NO_ERROR) {
    fprintf(stderr, "%s (%d): %s\n", details, errnum, msg);
  }
  return msg;
}


// première chose à faire: ouvrir un device
ALCdevice * AL_open_device(const ALCchar *name)
{
  ALCdevice *device = alcOpenDevice( name);
  if (!device) {
    check_error(__FUNCTION__);
  }

  return device;
}


ALCboolean AL_close_device( ALCdevice *device)
{
  ALCboolean res = alcCloseDevice( device);
  if ( !res) {
    check_error(__FUNCTION__);
  }

  return res;
}


// quand on a un device, on crée un context, qui devient le «current context»
ALCcontext * AL_open_context( ALCdevice *device, ALCint *attrlist)
{
  ALCcontext *context = alcCreateContext( device, attrlist);

  if ( !context) {
    check_error(__FUNCTION__);
  } else {
    ALCboolean res = alcMakeContextCurrent( context);
    if (!res) {
      check_error(__FUNCTION__);
    }
  }

  return context;
}


void AL_close_context( ALCcontext *context)
{
  alcDestroyContext( context);
  alcMakeContextCurrent( 0);
}


// si les extensions sont présentes, retourne 1
int are_there_extensions()
{
  return (alcIsExtensionPresent (NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE ? 1:0);
}


char const *AL_get_default_device()
{
  ALenum def[2] = { ALC_DEFAULT_DEVICE_SPECIFIER, ALC_DEFAULT_ALL_DEVICES_SPECIFIER };

  return alcGetString( 0, def[ are_there_extensions()]);
}


// à chaque appel, retourne le device suivant ou un pointeur nul.
char const * AL_list_devices()
{
  static char const *device = 0;

  if (!device) {
    // essaie de récupérer un maximum d'information sur les devices
    ALCenum dev_spec[2] = { ALC_DEVICE_SPECIFIER, ALC_ALL_DEVICES_SPECIFIER };
    ALCenum what_dev = dev_spec[ are_there_extensions()];
    device = (char const *) alcGetString(NULL, what_dev);
  } else {
    while (*device++); // passe le 0 terminateur avec le post incrément.
    if (!*device) { device=0; } //dernier élément de la liste: double NULL.
  }


  return device;
}

// fonction helper pour AL_list_devices()
void list_devices()
{
  char const *liste=0;
  do {
    liste=AL_list_devices();
    if (liste) { fprintf(stderr,"%s\n",liste); }
  } while(liste);
}

// génère une source (on n'est pas limité à une seul source avec OpenAL)
// note: on peut aussi controler le volume d'une source, son emplacement dans la scène…
ALuint AL_gen_source()
{
  ALuint source = 0;

  alGenSources( 1 /*nb sources*/, &source);
  check_error( __FUNCTION__);

  return source;
}

void AL_del_source( ALuint source)
{
  alDeleteSources( 1, &source);
  check_error( __FUNCTION__);
}


// générer 'n' buffers

void AL_gen_buffers(AL_buffer &al_buf)
{
  //ALuint *buffers = malloc(sizeof(nb)*sizeof(ALuint));
  // ALuint *buffers = new ALuint[nb];
  alGenBuffers( 1, &al_buf.id);
  check_error( __FUNCTION__);
  al_buf.data = new char[buffer_size](); //new xxx() : les parenthèses font que buffer est mis à zéro.
  fprintf(stderr,"%s al_buf.id: %d\n",__FUNCTION__,al_buf.id);
//  check_error( __FUNCTION__);

}

void AL_del_buffers( AL_buffer &al_buf)
{
  alDeleteBuffers( 1, &al_buf.id);
  fprintf(stderr,"%s al_buf.id: %d\n",__FUNCTION__,al_buf.id);
  delete al_buf.data;
  check_error ( __FUNCTION__);
}


ALCdevice *device = 0;
ALCcontext *context = 0;
ALuint source;
#define nb_buffers 2
AL_buffer al_buffer[nb_buffers];

// on peut appeller plusieurs fois AL_init
// notamment quand on veut jouer un autre module
void AL_init()
{
  if (!device) {
    //fprintf(stderr, "Device demandé: %s\n", AL_get_default_device() );
    device = AL_open_device( AL_get_default_device());
    context = AL_open_context(device, 0);
  } else {
fprintf(stderr,"AL_init()\n");
    AL_del_source( source);
    for (int i=0; i<nb_buffers; i++) {
      AL_del_buffers( al_buffer[i]);
    }
  }

  source = AL_gen_source();
  for (int i=0; i<nb_buffers; i++) {
    AL_gen_buffers( al_buffer[i]);
  }
}

void AL_exit()
{
  AL_del_source( source);
  for (int i=0; i<nb_buffers; i++) {
    AL_del_buffers( al_buffer[i]);
  }
  AL_close_context( context);
  AL_close_device(device);
}


/* ================  LIB d'interface OpenAL (fin) =============== */




// pour l'instant, je définis des valeurs par défaut dans cette
// fonction, mais ça pourrait changer par la suite en passant des paramètres.
streamer_t *DUMB_init_streamer(char const *module_filename)
{
  static streamer_t streamer;
  memset(&streamer, 0, sizeof(streamer_t));

  // Defaults
  streamer.freq = REPLAY_FREQ;
  streamer.n_channels = 1; //2;
  streamer.bits = 16; //16;
  streamer.volume = 1.0f; //1.0f;
  streamer.quality = DUMB_RQ_CUBIC;
  streamer.ended = true;

  // Load source file.
  dumb_register_stdfiles();
  //const char *nom_fichier = "cerror-dreidl.mod";
  streamer.input = module_filename; //nom_fichier;
  streamer.duh = dumb_load_any(streamer.input, 0, 0);
  if (!streamer.duh) {
    fprintf(stderr, "Unable to load file %s for playback!\n", streamer.input);
  }

  // Set up playback
  streamer.renderer =
    // duh_start_sigrenderer : démarre le rendu du module.
    duh_start_sigrenderer(streamer.duh, 0, streamer.n_channels, 0 /*65535*50*2.5*/ /* départ (65535: 1 seconde)*/);
  streamer.delta = 65536.0f / streamer.freq;
  streamer.sbytes = (streamer.bits / 8) * streamer.n_channels;
  streamer.ssize = duh_get_length(streamer.duh);

  //DUMB_IT_SIGRENDERER *it_sigrenderer = duh_get_it_sigrenderer(streamer.renderer);
  streamer.it_sigrenderer = duh_get_it_sigrenderer(streamer.renderer);

  /*  // Si on veut que le module s'arrête à la fin du replay
      dumb_it_set_loop_callback(streamer.it_sigrenderer, &dumb_it_callback_terminate, NULL);
      dumb_it_set_xm_speed_zero_callback(streamer.it_sigrenderer, &dumb_it_callback_terminate, NULL);
      */
  dumb_it_set_resampling_quality(streamer.it_sigrenderer, streamer.quality);

  // récupérer le nombre total de patterns à jouer (la taille de la liste d' «ordres» )
  // ainsi que le nombre de voies du module.
  // cela nécessite d'inclure le header internal/it.h qui contient la def de DUMB_IT_SIGDATA
  // (dumb.h) ne contient qu'un typedef.
  streamer.sigdata = duh_get_it_sigdata( streamer.duh);
  streamer.nb_orders = dumb_it_sd_get_n_orders( streamer.sigdata);
  streamer.nb_channels = streamer.sigdata->n_pchannels;

  return &streamer;
}

void DUMB_stop( streamer_t *streamer)
{
  if (streamer->sig_samples) {
    destroy_sample_buffer(streamer->sig_samples);
  }

  if (streamer->renderer) {
    duh_end_sigrenderer(streamer->renderer);
  }
  if (streamer->duh) {
    unload_duh(streamer->duh);
  }
}


void DUMB_exit( streamer_t *streamer)
{
  DUMB_stop( streamer);

  streamer->ended = true;
  dumb_exit(); //dans la doc de DUMB, ils mettent ça dans un atexit …
}



void cb_feed_buffer(streamer_t *streamer, ALuint buffer_popped)
{
  AL_buffer *current_buffer = &al_buffer[0];
  if ( buffer_popped == al_buffer[1].id) {
    current_buffer = &al_buffer[1];
  }

  int r_samples = buffer_size / streamer->sbytes; //streamer->sbytes : 16 ou 8
  int got = duh_render_int(streamer->renderer, &streamer->sig_samples,
      &streamer->sig_samples_size, streamer->bits, 0,
      streamer->volume, streamer->delta, r_samples, current_buffer->data);

  // attention, duh_render_int ne retournera zéro qui si un
  // callback de fin de module a été défini. Voir les deux fonctions:
  //  dumb_it_set_loop_callback
  //  dumb_it_set_xm_speed_zero_callback
  if ( !got) { streamer->ended = true; }

  ALenum format_buffer = AL_FORMAT_MONO16;
  if (streamer->bits != 16) { format_buffer = AL_FORMAT_MONO8; }

  // les données du buffer ont été mises à jour, il faut les réattacher à celui-ci
  alBufferData(current_buffer->id, format_buffer, current_buffer->data, buffer_size, REPLAY_FREQ);
  check_error( "alBufferData");

  // enquiller le buffer si nécessaire
  alSourceQueueBuffers( source, 1 /*nb buffer à enquiller*/, &current_buffer->id );
  check_error( "alSourceQueueBuffers");
  //fprintf(stderr,"\nDEBUG source %d, al_buffer[n].id:%d\n",source,current_buffer->id);
}


// récupère les données de la dernière ligne décodée
// il faut donc appeller cette fonction très régulièrement pour ne rien rater
// revoie le numéro de la ligne en cours de décodage
#define NB_CHANS 64 //déja beaucoup, mais limite de dumb : 64 channels
DUMB_IT_CHANNEL_STATE state[NB_CHANS]; // = {0};
int get_row_data(streamer_t *streamer)
{
  static int previous_row = -1;

  // récupère la ligne actuellement jouée
  int current_row = dumb_it_sr_get_current_row( streamer->it_sigrenderer);
  if (current_row != previous_row) {
    previous_row = current_row;
    int curr_order = dumb_it_sr_get_current_order( streamer->it_sigrenderer);
    fprintf(stderr,"[%02d/%02d : %02x]",curr_order, streamer->nb_orders, current_row);
    for (int i=0; i< streamer->nb_channels; i++) {
      dumb_it_sr_get_channel_state( streamer->it_sigrenderer, i, &state[i]);
      fprintf(stderr," %02d%s",state[i].sample, i+1==streamer->nb_channels?"\n":"");
    }
  }

  return current_row;
}

/* une autre fonction à moi (le fameux callback) */
void cb_play(streamer_t *streamer)
{


  ALint iBuffersProcessed = 0;

  alGetSourcei(source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
  if (iBuffersProcessed) {
    // avant de faire le unqueue, il faut que le buffer soit joué complètement.
    // si le buffer n'est pas joué complètement, le seul moyen de ne pas avoir d'erreur avec le alSourceUnqueueBuffers
    // est de faire un alSourceStop(source)
    ALuint buffer_popped = 0; //numéro du dernier buffer enlevé de la queue
    alSourceUnqueueBuffers( source, 1, &buffer_popped); // buffer_popped contiendra l'id du buffer enlevé de la queue.
    //fprintf(stderr,"buffer %d lu entièrement, on le retire\n", buffer_popped);
    check_error( "alSourceUnqueueBuffers");

    cb_feed_buffer( streamer, buffer_popped);

    // si on a eu du lag et que la source a consommé tous les buffers
    // il faut relancer la lecture (valable aussi lors du premier appel pour lancer la lecture)
    ALint playing_state = 0;
    alGetSourcei(  source, AL_SOURCE_STATE, &playing_state);
    if ( playing_state == AL_STOPPED) {
      alSourcePlay( source);
      fprintf(stderr,"relancer la source\n");
    }
  }

}

void DUMB_play(streamer_t *streamer)
{
  if (streamer->ended) {
    // il faut enclencher la pompe. Comme je lis beaucoup de données d'un seul coup,
    // il se peut qu'une ligne ou deux de stats (les infos sur les notes) soient passées
    // en tout début de musique.
    cb_feed_buffer( streamer, al_buffer[0].id);
    cb_feed_buffer( streamer, al_buffer[1].id);
    alSourcePlay( source);
    check_error( "alSourcePlay");
    streamer->ended = false;
  } else {
    // on est déjà en train de jouer
    cb_play(streamer);
  }
}


#ifdef TEST_JSEB //version standalone

#include <signal.h>

// Set to true if SIGTERM or SIGINT is caught
static bool stop_signal = false;

// Simple signal handler function
static void sig_fn(int signo) { stop_signal = true; signo=signo; }

int main(int argc, char *argv[]) {

  // Signal handlers
    signal(SIGINT, sig_fn);
    signal(SIGTERM, sig_fn);

    AL_init();

    streamer_t *streamer = DUMB_init_streamer("cerror-dreidl.mod");

    while (!stop_signal) {
      // dormir 1/50ème de secondes (uniquement en cas de test dans une boucle)
      // en dehors d'une boucle, par exemple avec un appel à 50 ou 60hz, sans intérêt.
      #define SLEEP 1000000/50
      usleep(SLEEP);
      DUMB_play(streamer);
      get_row_data(streamer);
    }


    DUMB_exit(streamer);

    fprintf(stderr,"was playing … %s\n",streamer->input);
    AL_exit();
    return 0;
}

#endif
