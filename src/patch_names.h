#pragma once
#include <Arduino.h>

static const char JUNO_PATCH_BLOB[] PROGMEM = "Brass Set 1\0Brass Swell\0Trumpet\0Flutes\0Moving Strings\0Brass & Strings\0Choir\0Piano I\0Organ I\0Organ II\0Combo Organ\0Calliope\0Donald Pluck\0Juno 13\0Elect. Piano I\0Elect. Piano II\0Juno 16\0Steel Drums\0Xylophone\0Brass III\0Fanfare\0String III\0Pizzicato\0High Strings\0Bass clarinet\0English Horn\0Brass Ensemble\0Guitar\0Koto\0Dark Pluck\0Funky I\0Synth Bass I (unison)\0Lead I\0Lead II\0Lead III\0Funky II\0Synth Bass II\0Funky III\0Thud Wah\0Going Up\0Piano II\0Clav\0Frontier Organ\0Snare Drum (unison)\0Tom Toms (unison)\0Timpani (unison)\0Shaker\0Synth Pad\0Sweep I\0Pluck Sweep\0Repeater\0Sweep II\0Pluck Bell\0Dark Synth Piano\0Sustainer\0Wah Release\0Gong (play low chords)\0Resonance Funk\0Juno 58\0Dust Storm\0Rocket Men\0Hand Claps\0FX Sweep\0Caverns\0Strings\0Violin\0Chorus Vibes\0Organ 1\0Harpsichord 1\0Recorder\0Perc. Pluck\0Noise Sweep\0Space Chimes\0Nylon Guitar\0Orchestral Pad\0Bright Pluck\0Organ Bell\0Accordion\0FX Rise 1\0FX Rise 2\0Brass\0Helicopter\0Lute\0Chorus Funk\0Tomita\0FX Sweep 1\0Sharp Reed\0Bass Pluck\0Resonant Rise\0Harpsichord 2\0Dark Ensemble\0Contact Wah\0Noise Sweep 2\0Glassy Wah\0Phase Ensemble\0Chorused Bell\0Clav\0Organ 2\0Bassoon\0Auto Release Noise Sweep\0Brass Ensemble\0Ethereal\0Chorus Bell 2\0Blizzard\0E. Piano with Tremolo\0Clarinet\0Thunder\0Reedy Organ\0Flute / Horn\0Toy Rhodes\0Surf's Up\0OW Bass\0Piccolo\0Melodic Taps\0Meow Brass\0Violin (high)\0High Bells\0Rolling Wah\0Ping Bell\0Brassy Organ\0Low Dark Strings\0Piccolo Trumpet\0Cello\0High Strings\0Rocket Men\0Forbidden Planet\0Froggy\0Owgan\0";

static const char DX7_PATCH_BLOB[] PROGMEM = "BRASS   1\0BRASS   2\0BRASS   3\0STRINGS 1\0STRINGS 2\0STRINGS 3\0ORCHESTRA\0PIANO   1\0PIANO   2\0PIANO   3\0E.PIANO 1\0GUITAR  1\0GUITAR  2\0SYN-LEAD 1\0BASS    1\0BASS    2\0E.ORGAN 1\0PIPES   1\0HARPSICH 1\0CLAV    1\0VIBE    1\0MARIMBA\0KOTO\0FLUTE   1\0ORCH-CHIME\0TUB BELLS\0STEEL DRUM\0TIMPANI\0REFS WHISL\0VOICE   1\0TRAIN\0TAKE OFF\0PIANO   4\0PIANO   5\0E.PIANO 2\0E.PIANO 3\0E.PIANO 4\0PIANO 5THS\0CELESTE\0TOY PIANO\0HARPSICH 2\0HARPSICH 3\0CLAV    2\0CLAV    3\0E.ORGAN 2\0E.ORGAN 3\0E.ORGAN 4\0E.ORGAN 5\0PIPES   2\0PIPES   3\0PIPES   4\0CALIOPE\0ACCORDION\0SITAR\0GUITAR  3\0GUITAR  4\0GUITAR  5\0GUITAR  6\0LUTE\0BANJO\0HARP    1\0HARP    2\0BASS    3\0BASS    4\0PICCOLO\0FLUTE   2\0OBOE\0CLARINET\0SAX BC\0BASSOON\0STRINGS 4\0STRINGS 5\0STRINGS 6\0STRINGS 7\0STRINGS 8\0BRASS   4\0BRASS   5\0BRASS 6 BC\0BRASS   7\0BRASS   8\0RECORDER\0HARMONICA1\0HRMNCA2 BC\0VOICE   2\0VOICE   3\0GLOKENSPL\0VIBE    2\0XYLOPHONE\0CHIMES\0GONG    1\0GONG    2\0BELLS\0COW BELL\0BLOCK\0FLEXATONE\0LOG DRUM\0SYN-LEAD 2\0SYN-LEAD 3\0SYN-LEAD 4\0SYN-LEAD 5\0SYN-CLAV 1\0SYN-CLAV 2\0SYN-CLAV 3\0SYN-PIANO\0SYNBRASS 1\0SYNBRASS 2\0SYNORGAN 1\0SYNORGAN 2\0SYN-VOX\0SYN-ORCH\0SYN-BASS 1\0SYN-BASS 2\0HARP-FLUTE\0BELL-FLUTE\0E.P-BRS BC\0T.BL-EXPA\0CHIME-STRG\0B.DRM-SNAR\0SHIMMER\0EVOLUTION\0WATER GDN\0WASP STING\0LASER GUN\0DESCENT\0OCTAVE WAR\0GRAND PRIX\0ST.HELENS\0EXPLOSION\0";

static const char PCM_PATCH_BLOB[] PROGMEM = "808 Maraca\0808 Kick 4\0808 Snare 4\0808 Snare 7\0808 Snare 10\0808 Snare 12\0808 Closed Hat\0808 Open Hat\0808 Low Tom\0808 Dry Clap\0808 Cowbell\0808 Shaker\0808 Snare 1\0808 Snare 2\0808 Snare 3\0808 Rimshot\0808 Tom Lo\0808 Tom Hi\0808 Cymbal\0PCM 19\0PCM 20\0PCM 21\0PCM 22\0PCM 23\0PCM 24\0PCM 25\0PCM 26\0PCM 27\0PCM 28\0PCM 29\0PCM 30\0PCM 31\0PCM 32\0PCM 33\0PCM 34\0PCM 35\0PCM 36\0PCM 37\0PCM 38\0PCM 39\0PCM 40\0PCM 41\0PCM 42\0PCM 43\0PCM 44\0PCM 45\0PCM 46\0PCM 47\0PCM 48\0PCM 49\0PCM 50\0PCM 51\0PCM 52\0PCM 53\0PCM 54\0PCM 55\0PCM 56\0PCM 57\0PCM 58\0PCM 59\0PCM 60\0PCM 61\0PCM 62\0PCM 63\0PCM 64\0PCM 65\0PCM 66\0";

inline const char* getNthBlobStr(const char *blob, int idx) {
    const char *p = blob;
    for (int i = 0; i < idx; i++) {
        while (*p) p++;
        p++;
    }
    return p;
}

inline const char* getEnginePatchName(uint8_t engine, uint8_t patchOrNote) {
    if (engine == 1) return getNthBlobStr(JUNO_PATCH_BLOB, patchOrNote & 0x7F);
    if (engine == 2) return getNthBlobStr(DX7_PATCH_BLOB, (patchOrNote >= 128 ? patchOrNote - 128 : patchOrNote) & 0x7F);
    if (engine == 3 && patchOrNote < 67) return getNthBlobStr(PCM_PATCH_BLOB, patchOrNote);
    if (engine == 5) return "Grand Piano";
    return nullptr;
}
