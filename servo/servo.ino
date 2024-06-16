//www.elegoo.com
//2018.12.19
#include <Servo.h>
#include <IRremote.h>
//#include "IR.h"
#include "freeMemory.h"

const int Speaker_PIN = 9;
const int Host_PIN = 10;    // 7;
const int IR_Receiver_PIN = 7; // 10;
const int Buildin_Blue_LED = 13; 
const int reset_PIN = 4;

const int Max_degree = 112;
const int Min_degree = 67;
const int ZOOM_OUT_THRESHOLD = 87;
const int ZOOM_IN_THRESHOLD  = 102;

const int Power_On_degree=132;
const int Power_Off_degree=135;
const int Host_Power_degree= 97;


const int Power_On_delay = 512;
const int Power_Off_delay = 1024;
const int Reg_delay = 128;
unsigned int Curr_Key=0xFFFFFFFF;
int New_Key=1;   //0 - keep press an key without release, 
                 //1 : new key pressed


// projecter EPSON (front ) remote controller
const int Speaker_Power_ON = 0x86;  //  Page Up key ;
const int Speaker_Power_OFF= 0x87;  //  Page Down Key 
const int Speaker_Zoom_In  = 0xB1;  //  Right arrow key
const int Speaker_Zoom_Out = 0xB3;  //  Left arrow key 
const int PlayPause        = 0x85;  //  Return / Left mouse key 

const int All_Back_Zero    = 0x84 ;  // Esc key  
const int POWER_KEY        = 0x90;   // Blue Power Key 

const int Host_Power_ON = 0x98;    /// Volume  UP
const int Host_Power_OFF= 0x99;     // Volume Down

/*
// projecter VANKYO (back wall) remote controller
const int Speaker_Power_ON = 0x9C;  //0xFF39C6;
const int Speaker_Power_OFF= 0xD6;  //0xFF6B94; 
const int Speaker_Zoom_In  = 0x98;  //0xFF19E6;
const int Speaker_Zoom_Out = 0x82;  //0xFF41BE;
const int All_Back_Zero    = 0xA4;  //0xFF25DA;  //  undo 
const int POWER_KEY        = 0xA8;  //Red Power Key 
const int PlayPause        = 0x93;  //used as back to zero also, for it's close to << and >>

const int Host_Power_ON = 0x8C;  //0xFF31CE;
const int Host_Power_OFF= 0x88;  //0xFF11EE; 
*/


/* Arduino remote controller 
const int Speaker_Power_ON = 0xC;  //0xFF39C6;
const int Speaker_Power_OFF= 0x5E;  //0xFF6B94; 
const int Speaker_Zoom_In  = 0x44;  //0xFF19E6;
const int Speaker_Zoom_Out = 0x43;  //0xFF41BE;
const int All_Back_Zero    = 0x40;  //0xFF25DA;  //  undo 
const int POWER_KEY        = 0xA8;  //Red Power Key 
const int PlayPause        = 0x93;  //used as back to zero also, for it's close to << and >>

const int Host_Power_ON = 0x8;  //0xFF31CE;
const int Host_Power_OFF= 0x5A;  //0xFF11EE; 
*/

int Speaker_On_Flag = 0;
int Host_On_Flag = 0;
int live_counter = 0;
char info[50];

Servo servo_host;
Servo servo_speaker;

int host_degree = 90;
int speaker_degree = 90;

#undef LED_BUILTIN
#define LED_BUILTIN 3

void setup(){
/*  digitalWrite(reset_PIN, LOW);
  digitalWrite(Speaker_PIN, LOW);
  digitalWrite(Host_PIN, LOW);

  delay(64);
  

  pinMode(Buildin_Blue_LED, OUTPUT);
  pinMode(Speaker_PIN, OUTPUT);
  pinMode(Host_PIN, OUTPUT);
  pinMode(reset_PIN, OUTPUT);
  
  digitalWrite(reset_PIN, HIGH);
*/
  Serial.begin(115200);
  live_counter = 0;
  Serial.println("Reset counter to 0");

  servo_speaker.attach(Speaker_PIN);
  servo_host.attach(Host_PIN);
//  delay(64);

  servo_speaker.write(90);// move servos to center position -> 90°
  Serial.println("In setup, servo_speaker set to 90");

//  delay(512);
  
  servo_host.write(90);// move servos to center position -> 90°
  Serial.println("In setup, servo_host set to 90");


  Serial.println("IR Receiver Button Decode");
  
  IrReceiver.begin(IR_Receiver_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("LED_BUILTIN:"+LED_BUILTIN);
//  digitalWrite(reset_PIN, LOW);


} 


void loop(){
  
   if ( IrReceiver.decode()){

        Curr_Key = IrReceiver.decodedIRData.command;
        
        Serial.println(Curr_Key, HEX);
        IrReceiver.resume();

        process_key(Curr_Key);
        
   }else{
    delay(Reg_delay);
   }
  prt_free_memory();
  live_counter ++;
}

void servo_power_on(){
    digitalWrite(reset_PIN, HIGH);
    delay(64);
}

void servo_power_off(){
    digitalWrite(reset_PIN, LOW);
}

void process_key(int key){
  //check duplicated key for power on/off key 
  // TBD 

  servo_power_on();
  
  switch(key){
    case Speaker_Power_ON:  //
         if (Speaker_On_Flag == 1)
            break;
         
         power_on(servo_speaker, Power_On_degree); 
         Serial.println("Speaker Camera ON");
         Speaker_On_Flag = 1;
         speaker_degree = 90;
      
         break;

    case Speaker_Power_OFF:  //// 
         if (Speaker_On_Flag == 0)
            break;
         power_off(servo_speaker, Power_Off_degree); 
         Serial.println("Speaker Camera OFF");
         Speaker_On_Flag = 0;
         speaker_degree = 90;
         break;

    case Host_Power_ON:  //
         if (Host_On_Flag == 1)
            break;
    
         power_on(servo_host, Host_Power_degree); 
         sprintf(info, "Host Camera ON at degree:%d", Host_Power_degree);
         Serial.println(info);
         Host_On_Flag = 1;
         host_degree = 90;
      
         break;

    case Host_Power_OFF:  //// 
         if (Host_On_Flag == 0)
            break;
         power_off(servo_host, Host_Power_degree); 
         sprintf(info, "Host Camera OFF at degree:%d", Host_Power_degree);
         Serial.println(info);
         Host_On_Flag = 0;
         host_degree = 90;
         break;

    case Speaker_Zoom_In:  // fast forward for zoom in 
         if (speaker_degree < ZOOM_IN_THRESHOLD) 
               speaker_degree = ZOOM_IN_THRESHOLD;
         
         speaker_degree = zoom_pos(servo_speaker, ++ speaker_degree);
         sprintf(info, "Speaker Camera Zoom In to %d", speaker_degree);
         Serial.println(info);
          
         break;

    case Speaker_Zoom_Out:  // 
         if (speaker_degree > ZOOM_OUT_THRESHOLD) 
               speaker_degree = ZOOM_OUT_THRESHOLD;

         speaker_degree = zoom_pos(servo_speaker, -- speaker_degree);
         sprintf(info, "Speaker Camera Zoom Out to %d", speaker_degree);
         Serial.println(info);
          
         break;
         

    case All_Back_Zero: 
    case PlayPause:
         Serial.println("All camera back to 0 postion");    
         speaker_degree = 90;
         host_degree = 90;
         zoom_pos(servo_speaker, speaker_degree);
         zoom_pos(servo_host, host_degree);
         Serial.println("Both cameras are set to N pposition.");
         break;
    case 0:
         Serial.println("returned 0, will reset");
         digitalWrite(reset_PIN, LOW);
         delay(20);
         break;
    case POWER_KEY:
         Serial.println("Reset by Power KEY.");
         digitalWrite(reset_PIN, LOW);
         delay(20);
         break;
         
    default:
         sprintf(info, "Unregistered key pressed: %X", Curr_Key);
         Serial.println(info);
         ;
  }

  delay(Reg_delay);
  servo_power_off();

}

int zoom_pos(Servo servo, int degree){

  if ( degree > Max_degree)
       degree = Max_degree;
  if ( degree < Min_degree)
       degree = Min_degree;  

  servo.write(degree);
  
  return degree;
  
}
void power_on(Servo servo, int degree  ){
  
    servo.write( degree);
    delay(Power_On_delay);
    servo.write(90);

    
//    delay(Reg_delay);
    return;
}

void power_off(Servo servo, int degree  ){
    servo.write(degree);
    delay(Power_Off_delay);
    servo.write(90);
//    delay(Reg_delay);
    return;
  
}

void prt_free_memory(){
  if (live_counter % 64 ) 
    return;
  Serial.print("Free memory:");
  Serial.println(  freeMemory() );
  Serial.print("Live Counter:");
  Serial.println(live_counter);
 
  //blink_live_counter();
  
}

void blink_live_counter(){
  // long -- short -- hundres, tens, singles
  int s = 512;  //short duration
  int l = 1024; // long duration
  
  blink_led(1,l);
  blink_led(live_counter/100 ,s);
  
  delay(1024);
  blink_led((live_counter%100)/10 ,s);
  
  delay(1024);
  blink_led(live_counter%10, s);
  
  return;
}

void blink_led(int times, int duration){
  while(times--){
    digitalWrite(Buildin_Blue_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(duration);                       // wait for a second
    digitalWrite(Buildin_Blue_LED, LOW);    // turn the LED off by making the voltage LOW
    delay(duration);                       // wait for a second  
  }

  return;
}
/*
 * 
 * 
 * 
 * power: FE15EA
Mute: FF11EE
Play/pause; FFC936

Fast<<: FF19E6
Fast>>:FF41BE
Sound<<:FF39C6
sound>>:FF31CE
Tide: FF6B94

OK: FF7986
<: FFD926
>: FF9966
Up:FFA956
Down:FF59A6


                            Speaker               Host 

Power On                 volumn down             volume up  
Power Off                     tide                mute
Zoom In                     fast <<
Zoom Out                  fast >>

 */

 /*
  * 
  * 
23:04:45.833 -> 0
23:04:45.833 -> Unregistered key pressed: 0
23:04:45.972 -> NEC: Data length=2 is not 68 or 4
23:04:45.972 -> Kaseikyo: Data length=2 is not 100
23:04:45.972 -> Denon: Data length=2 is not 32
23:04:45.972 -> RC5: first getBiphaselevel() is not MARK
23:04:45.972 -> RC6: Header mark or space length is wrong
23:04:45.972 -> LG: Data length=2 is not 60 or 4
23:04:45.972 -> JVC: Data length=2 is not 34 or 36
23:04:45.972 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:45.972 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:45.972 -> 0
23:04:45.972 -> Unregistered key pressed: 0
23:04:46.111 -> NEC: Data length=2 is not 68 or 4
23:04:46.111 -> Kaseikyo: Data length=2 is not 100
23:04:46.111 -> Denon: Data length=2 is not 32
23:04:46.111 -> RC5: first getBiphaselevel() is not MARK
23:04:46.111 -> RC6: Header mark or space length is wrong
23:04:46.111 -> LG: Data length=2 is not 60 or 4
23:04:46.111 -> JVC: Data length=2 is not 34 or 36
23:04:46.111 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.111 -> LEGO: Data length=2 is not 36
23:04:46.111 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.111 -> 0
23:04:46.111 -> Unregistered key pressed: 0
23:04:46.250 -> NEC: Data length=2 is not 68 or 4
23:04:46.250 -> Kaseikyo: Data length=2 is not 100
23:04:46.250 -> Denon: Data length=2 is not 32
23:04:46.250 -> RC5: first getBiphaselevel() is not MARK
23:04:46.250 -> RC6: Header mark or space length is wrong
23:04:46.250 -> LG: Data length=2 is not 60 or 4
23:04:46.250 -> JVC: Data length=2 is not 34 or 36
23:04:46.250 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.250 -> LEGO: Data length=2 is not 36
23:04:46.250 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.250 -> 0
23:04:46.250 -> Unregistered key pressed: 0
23:04:46.352 -> NEC: Data length=2 is not 68 or 4
23:04:46.352 -> Kaseikyo: Data length=2 is not 100
23:04:46.352 -> Denon: Data length=2 is not 32
23:04:46.352 -> RC5: first getBiphaselevel() is not MARK
23:04:46.352 -> RC6: Header mark or space length is wrong
23:04:46.352 -> LG: Data length=2 is not 60 or 4
23:04:46.352 -> JVC: Data length=2 is not 34 or 36
23:04:46.352 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.352 -> LEGO: Data length=2 is not 36
23:04:46.352 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.352 -> 0
23:04:46.352 -> Unregistered key pressed: 0
23:04:46.490 -> NEC: Data length=2 is not 68 or 4
23:04:46.490 -> Kaseikyo: Data length=2 is not 100
23:04:46.490 -> Denon: Data length=2 is not 32
23:04:46.490 -> RC5: first getBiphaselevel() is not MARK
23:04:46.490 -> RC6: Header mark or space length is wrong
23:04:46.490 -> LG: Data length=2 is not 60 or 4
23:04:46.490 -> JVC: Data length=2 is not 34 or 36
23:04:46.490 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.490 -> LEGO: Data length=2 is not 36
23:04:46.490 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.490 -> 0
23:04:46.490 -> Unregistered key pressed: 0
23:04:46.635 -> NEC: Data length=2 is not 68 or 4
23:04:46.635 -> Kaseikyo: Data length=2 is not 100
23:04:46.635 -> Denon: Data length=2 is not 32
23:04:46.635 -> RC5: first getBiphaselevel() is not MARK
23:04:46.635 -> RC6: Header mark or space length is wrong
23:04:46.635 -> LG: Data length=2 is not 60 or 4
23:04:46.635 -> JVC: Data length=2 is not 34 or 36
23:04:46.635 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.635 -> LEGO: Data length=2 is not 36
23:04:46.635 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.635 -> 0
23:04:46.635 -> Unregistered key pressed: 0
23:04:46.771 -> NEC: Data length=2 is not 68 or 4
23:04:46.771 -> Kaseikyo: Data length=2 is not 100
23:04:46.771 -> Denon: Data length=2 is not 32
23:04:46.771 -> RC5: first getBiphaselevel() is not MARK
23:04:46.771 -> RC6: Header mark or space length is wrong
23:04:46.771 -> LG: Data length=2 is not 60 or 4
23:04:46.771 -> JVC: Data length=2 is not 34 or 36
23:04:46.771 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.771 -> LEGO: Data length=2 is not 36
23:04:46.771 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.771 -> 0
23:04:46.771 -> Unregistered key pressed: 0
23:04:46.877 -> NEC: Data length=2 is not 68 or 4
23:04:46.877 -> Kaseikyo: Data length=2 is not 100
23:04:46.877 -> Denon: Data length=2 is not 32
23:04:46.877 -> RC5: first getBiphaselevel() is not MARK
23:04:46.877 -> RC6: Header mark or space length is wrong
23:04:46.877 -> LG: Data length=2 is not 60 or 4
23:04:46.877 -> JVC: Data length=2 is not 34 or 36
23:04:46.877 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:46.877 -> LEGO: Data length=2 is not 36
23:04:46.877 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:46.877 -> 0
23:04:46.877 -> Unregistered key pressed: 0
23:04:47.014 -> NEC: Data length=2 is not 68 or 4
23:04:47.014 -> Kaseikyo: Data length=2 is not 100
23:04:47.014 -> Denon: Data length=2 is not 32
23:04:47.014 -> RC5: first getBiphaselevel() is not MARK
23:04:47.014 -> RC6: Header mark or space length is wrong
23:04:47.014 -> LG: Data length=2 is not 60 or 4
23:04:47.014 -> JVC: Data length=2 is not 34 or 36
23:04:47.014 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:47.014 -> LEGO: Data length=2 is not 36
23:04:47.014 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:47.014 -> 0
23:04:47.014 -> Unregistered key pressed: 0
23:04:47.150 -> NEC: Data length=2 is not 68 or 4
23:04:47.150 -> Kaseikyo: Data length=2 is not 100
23:04:47.150 -> Denon: Data length=2 is not 32
23:04:47.150 -> RC5: first getBiphaselevel() is not MARK
23:04:47.150 -> RC6: Header mark or space length is wrong
23:04:47.150 -> LG: Data length=2 is not 60 or 4
23:04:47.150 -> JVC: Data length=2 is not 34 or 36
23:04:47.150 -> Samsung: Data length=2 is not 68 or 100 or 6
23:04:47.150 -> PULSE_DISTANCE: Data length=2 is less than 20
23:04:47.150 -> 0
23:04:47.150 -> Unregistered key pressed: 0
23:04:53.946 -> 98
23:04:53.946 -> Speaker Camera Zoom In to 92
  * 
  * 
  * 
  */
