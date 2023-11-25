/*
 Miscellaneous
*/
#define ModeEnableConstant 200
#define ModeInactivityConstant 5000


/*
 Keyboard related constant or variables (not utility keyboard but piano keyboard)
*/

int activeKeyboardChannel= 1;






// ---------------------------------------------------------------------------

/*
 Utility key's respective number.
 And other constants related to utility
 keyboard.
*/

// The midi channel for utility keys.
#define U_Channel 16

// The Blue Butts
#define U_StartAndStopButt 3
#define U_SyncAndFillInButt 15

// Number Pad
#define U_NumZero 14
#define U_NumOne 23
#define U_NumTwo 22
#define U_NumThree 21
#define U_NumFour 10
#define U_NumFive 9
#define U_NumSix 8
#define U_NumSeven 13
#define U_NumEight 12
#define U_NumNine 11
#define U_NumMinus 31
#define U_NumPlus 16

// Mini LaunchPad
// Launchpad channel
#define L_Channel 15

#define L_Tom 27
#define L_TomNote 16

#define L_Snare 29
#define L_SnareNote 15

#define L_Cymbal 26
#define L_CymbalNote 12

#define L_HitHat 28
#define L_HitHatNote 13

#define L_BassDrum 30
#define L_BassDrumNote 14

// Volume
#define U_VolUp 19
#define U_VolDown 18
// define by how much the
// volume will be decreased.
// Volume range: 0-127
#define VolUnit 5
#define U_VolControllerNo 0

// Accompaniment Volume
#define U_AccVolUp 2
#define U_AccVolDown 1

// Transpose
#define U_TransposeUp 25
#define U_TransposeDown 24

// Temp
#define U_TempoUp 35
#define U_TempoDown 36

// Miscellaneous
#define U_Demo 20

#define U_Midi 34

#define U_Mode 17

#define U_Tone 6
#define U_Rhythm 5
#define U_SongBank 4

#define U_Beat 37
#define U_Metronome 38

#define U_Sustain 0
#define U_OneTouchPreset 33
#define U_ThreeStepLesson 39


// this var represents what the
// start stop button will do
// 0 -> start
// 1 -> stop
// Once pressed the value of this
// var changes (from 1->0 or opp.) 
// hence creating a toggling effect.
int start_stop_butt= 1;



// range: 0-127
int volume= 63;
