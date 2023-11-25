// #include "midi_constants.h"
// #include "midi_functions.h"

#include "keyboard_constants.h"

#include <MIDI.h>




/*

Keyboard pin numbering:
  R1 -> Row 1
  C1 -> Column 1

  R1 R2 R3 R4 R5 R6 R7 R8 C1 C2 C3 C4 C5 C6 C7
  01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
  24 26 28 30 32 34 36 38 40 42 44 46 48 50 52
   o  o  o  o  o  o  o  o  o  o  o  o  o  o  o


TODO:
  1. Remove debouncing.
*/


#define KEYBOARD_KEYS 49

#define ROWS 8
#define COLUMNS 7


/* 
DEBUG_LEVEL:
 1 -> 
*/
#define DEBUG_LEVEL 1



// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();



// CONFIG VARIABLES
// These can be edited through serial intf..




//                    01,02,03,04,05,06,07,08,10,11,12,13,14,15,09
byte keyboard_pins[]= {22,24,26,28,30,32,34,36,40,42,44,46,48,50,38};
// int keyboard_pins[]= {22,24,26,28,30,32,34,36,38,42,44,46,48,50,40};

byte keyboard_disabled_key_pin= 52;

bool keyboard_keys_values[KEYBOARD_KEYS]= {};
// starting from 12th note because in lmms
// the 0-11 notes are absent. 
byte keys_respective_notes[]= {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60};
int transpose= 0;

// temp (per loop) value holders
int _i= 0;
int row_pin= 0;
int column_pin= 0;
bool key_pressed= 0;

// calculating how many keys in the matrix are
// not used.
int unused_keys= (ROWS*COLUMNS)-KEYBOARD_KEYS;




// Notes:
// The utility keys have been multiplexed with the shape (5, 8).
// That implies 0-4 pins are cols and others are rows (or vice-versa).
// Also Keyboard has 38 utility keys but the matix can provide upto 40
//   therefore 2 keys are not used. (they don't change value but are
//   polled just to make the code easy)

// pin no.   00  01  02  03  04  05  06  07  08  09  10  11  12
int pins[]= {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47};

int utility_values[40];
unsigned long utility_pressed_on[40];

void poll_utility_keys();



// -------------- Miscellaneous --------------

/*
Modes:
  0 - None
  1 - Midi Channel
*/
byte current_mode= 0;

unsigned long mode_expiry= 0;

void renew_mode_expiry(int scale= 1);


String mode_input= "";

void handle_mode(int mode_number, String mode_input);


// stores midi channels to sustain (0-9)
// channels 10-15 are reserved for controllers
// and other non-musical stuff.
bool sustain_channels[10]= {0,0,0,0, 0,0,0,0, 0,0};





String debug_string= "";

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);

  for (int i=0; i<KEYBOARD_KEYS; i++){
    keyboard_keys_values[i]= true;
  }
  

  pinMode(keyboard_disabled_key_pin, INPUT_PULLUP);
}

void loop() {


  _i= 0;


  for (int column=0; column<COLUMNS; column++){
    column_pin= keyboard_pins[column+ROWS];

    pinMode(column_pin, INPUT_PULLUP);

    for (int row=ROWS-1; row>=0; row--){
      
      if (_i < KEYBOARD_KEYS){

        row_pin= keyboard_pins[row];


        pinMode(row_pin, OUTPUT);
        digitalWrite(row_pin, LOW);

        
        
        // the keyboard pins are pulled up so 1 means 0 and vice-versa, so 
        // adjusting for that. (because it'll make the code prettier)
        key_pressed= !digitalRead(column_pin);

        if (DEBUG_LEVEL>=2){
          debug_string= debug_string+!digitalRead(column_pin)+",";
        }

        if (keyboard_keys_values[_i] != key_pressed){ // check if the value has changed.
          
          if (key_pressed) {
            if (DEBUG_LEVEL<=1){
              MIDI.sendNoteOn(keys_respective_notes[_i] + (transpose*KEYBOARD_KEYS), 127, activeKeyboardChannel); // key off
            }
            else {
              Serial.print("ON: ");
              Serial.println(_i);
            }
          }
          else {
            if (!sustain_channels[activeKeyboardChannel]){
              if (DEBUG_LEVEL<=1){
                MIDI.sendNoteOff(keys_respective_notes[_i] + (transpose*KEYBOARD_KEYS), 0, activeKeyboardChannel); // key off
              }
              else {
                Serial.print("OFF: ");
                Serial.println(_i);
              }
            }
          }

          keyboard_keys_values[_i]= key_pressed;
         
        }
        
      }
      
      pinMode(row_pin, INPUT);
      _i++;
    } 
  } 

  if (DEBUG_LEVEL>=2){
    Serial.println(debug_string);
    debug_string="";
  }

  poll_utility_keys();

  if (current_mode){
    // if the mode expired
    if (mode_expiry<=millis()){
      // excuting its functionality
      handle_mode(current_mode, mode_input);
      current_mode= 0;
      mode_input= "";
    }
  }

}

void _key_pressed(int key, unsigned long pressed_for){
  
  if (pressed_for<=50){
    return;
  }


  switch (key) {
   
    case U_StartAndStopButt:
      if (!start_stop_butt){
        MIDI.sendStart();
        start_stop_butt= 1;
      }
      else {
        MIDI.sendStop();
        start_stop_butt= 0;
      }
      break;

    case U_VolUp:
      volume= constrain(volume+VolUnit, 0, 127);
      MIDI.sendControlChange(U_VolControllerNo, volume, U_Channel);
      break;

    case U_VolDown:
      volume= constrain(volume-VolUnit, 0, 127);
      MIDI.sendControlChange(U_VolControllerNo, volume, U_Channel);
      break;

    case U_Sustain:
      sustain_channels[activeKeyboardChannel]= !sustain_channels[activeKeyboardChannel];
      break;

    case U_NumZero:
      mode_input= mode_input+"0";
      renew_mode_expiry();
      break;

    case U_NumOne:
      mode_input= mode_input+"1";
      renew_mode_expiry();
      break;

    case U_NumTwo:
      mode_input= mode_input+"2";
      renew_mode_expiry();
      break;

    case U_NumThree:
      mode_input= mode_input+"3";
      renew_mode_expiry();
      break;

    case U_NumFour:
      mode_input= mode_input+"4";
      renew_mode_expiry();
      break;

    case U_NumFive:
      mode_input= mode_input+"5";
      renew_mode_expiry();
      break;

    case U_NumSix:
      mode_input= mode_input+"6";
      renew_mode_expiry();
      break;

    case U_NumSeven:
      mode_input= mode_input+"7";
      renew_mode_expiry();
      break;

    case U_NumEight:
      mode_input= mode_input+"8";
      renew_mode_expiry();
      break;

    case U_NumNine:
      mode_input= mode_input+"9";
      renew_mode_expiry();
      break;

    // case U_NumMinus:
    //   mode_input= mode_input;
    //   renew_mode_expiry();
    //   break;

    case U_NumPlus:
      if (current_mode){
        // Executing its functionality and
        // inactivating it.
        handle_mode(current_mode, mode_input);
        current_mode= 0;
        mode_input= "";
      }
      break;



    case U_TransposeUp:
      transpose++;
      if ((transpose*KEYBOARD_KEYS)+keys_respective_notes[KEYBOARD_KEYS-1]>127){
        transpose--;
      }
      Serial.print(transpose);
      break;

    case U_TransposeDown:
      transpose--;
      if ((transpose*KEYBOARD_KEYS)+keys_respective_notes[0]<0){
        transpose++;
      }
      Serial.print(transpose);
      break;



    case U_Midi:
      if (pressed_for<ModeEnableConstant){
        activeKeyboardChannel++;
        if (activeKeyboardChannel>10) { activeKeyboardChannel = 1; }
      }
      else {
        current_mode= 1;
        renew_mode_expiry();
      }
      break;
  }

}



void poll_utility_keys(){
  int k= 0;

  for (int i= 0; i<5; i++){
    pinMode(pins[i], INPUT_PULLUP);
    
    for (int j= 5; j<13; j++){
      pinMode(pins[j], OUTPUT);
      digitalWrite(pins[j], LOW);
      
      if (utility_values[k]!= digitalRead(pins[i]) ){
        int note= -1;
        int channel= U_Channel;

        // if any of the percussion key (launchpad)
        // is pressed than send the respective
        // note on/off command. 
        switch (k) {
          case L_Tom:
            note= L_TomNote;
            channel= L_Channel;
            break;
          case L_Snare:
            note= L_SnareNote;
            channel= L_Channel;
            break;
          case L_Cymbal:
            note= L_CymbalNote;
            channel= L_Channel;
            break;
          case L_HitHat:
            note= L_HitHatNote;
            channel= L_Channel;
            break;
          case L_BassDrum:
            note= L_BassDrumNote;
            channel= L_Channel;
            break;
        }

        if (utility_values[k]==1){
          utility_pressed_on[k]= millis();
          
          if (note>-1){
            MIDI.sendNoteOn(note, 127, channel);
          }
        }
        else {
          if (note>-1){
            MIDI.sendNoteOff(note, 0, channel);
          }

          _key_pressed(k, millis()-utility_pressed_on[k]);
          utility_pressed_on[k]= 0;
        }
      }

      
      utility_values[k]= digitalRead(pins[i]);

      pinMode(pins[j], INPUT);

      k++;
    }

    pinMode(pins[i], INPUT);
  }
  
}


void handle_mode(int _mode_number, String _mode_input){
  switch (_mode_number) {
    
    // change midi channel
    case 1:
      int inp= _mode_input.toInt();
      
      if (inp>0 && inp<11){
        activeKeyboardChannel= inp;
      }
      else {
        // Ignore as the range for
        // keyboard channel is only
        // 0-9 (or in code 1-10)
      }
      break;
  }

}


void renew_mode_expiry(int scale= 1){
  mode_expiry= millis()+(scale*ModeInactivityConstant);
}
