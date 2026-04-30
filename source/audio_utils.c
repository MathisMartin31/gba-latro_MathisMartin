#include "audio_utils.h"

#include <maxmod.h>

#include "game_variables.h"

void play_sfx(mm_word id, mm_word rate, mm_byte volume)
{
    mm_sound_effect sfx = {
        {id},
        rate,
        0,
        (volume * g_game_vars.sound_volume) / VOLUME_VALUE_MAX,
        SFX_DEFAULT_PAN,
    };
    mmEffectEx(&sfx);
}
