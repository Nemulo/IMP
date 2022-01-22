// Playing a digital WAV recording repeatadly using the XTronical DAC Audio library
// prints out to the serial monitor numbers counting up showing that the sound plays 
// independently of the main loop
// See www.xtronical.com for write ups on sound, the hardware required and how to make
// the wav files and include them in your code
#include <stdlib.h>
#include<string.h>
#include<vector>
#include<regex>
#include <sstream>

const int ledPin = 16;  // 16 corresponds to GPIO16
static int incomingByte = 0;
// setting PWM properties
static int freq = 0;
const int ledChannel = 0;
const int resolution = 4;
//    GLOBAL VARIABLES
//                        H    C      D      E     F      G       A       H    C
//                                C#     D#           F#      G#      A#
static int octaveC5[] = {493,523,554,587,622,659,698,739,783,830,880,939,987,1046};
//                        0   1   2   3   4   5   6   7   8   9   10  11  12, 13
//                        C   C#  D   D#  E   F   F#   G  G#   A  A#  H   C
static int octaveC4[] = {261,277,293,311,329,349,369,392,415,440,466,493,523};
//                     quarter     half        full      double      quad
static int delays[] = {150,        300,        600,      1200,        2400};
//                      1           2           3           4         5    
std::vector<int>demo = {659,622,659,622,659,  493,587,523,440 ,0,261,329,440,493  ,329,392,493,523,0,   329,659,622};
std::vector<int>demo_dels = {0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0};
std::vector<int> music;
std::vector<int> dels;

//function plays whole octave with halftones aswell
void playOctave(int channel, int *freq)
{
  for (int i=0;i<13;i++)
  {
    ledcWriteTone(channel, freq[i]);
    delay(1000);
  }
}

//function plays demo from vectors demo and demo_dels
// type stop to serial for demo to stop playing
void play_demo()
{
  while(1)
  {
    for (int i = 0;i<demo.size();i++)
    {
      ledcWriteTone(ledChannel,demo[i]);
      delay(delays[demo_dels[i]]); 
      if (Serial.available()>0)
      {
        char buf[10] {'\0'};
        Serial.readBytes(buf,10);
        std::string line = buf;
        std::smatch mtch;
        if (std::regex_search(line,mtch,std::regex("stop\n")))
        {
          ledcWriteTone(ledChannel,0);
          return;
        }
      }
    }
  }
  ledcWriteTone(ledChannel,0);
}



void setup(){
  Serial.begin(115200);
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);
}
// function to read and parse input
bool read_in(char *buff)
{
    if (strcmp(buff,"octave\n")==0)
    {
      playOctave(ledChannel,octaveC5);
      return true;
    }
    if (strcmp(buff,"demo\n")==0)
    {
      Serial.println("Playing demo: for elise - Beethoven");
      play_demo();
      return true;
    }
    if (strcmp(buff,"clear\n")==0)
    {
      music.clear();
      dels.clear();
      Serial.println("your music is now clear, start sending tones!");
      return true;
    }
    if (strcmp(buff,"end\n")==0)
    {
      Serial.println("Your music will now play");
      return false;
    }
    else
    {
      Serial.print(buff);
      std::smatch match;
      std::string line = buff;
      int dl = 0;
      if(std::regex_search(line,match,std::regex("\\s*([cdefgahCDEFGAH][#']?)\\s+([1,2,3,4,5,6])")))
      {
        std::stringstream num;
        num<<match.format("$2");
        num>>dl;
        dl = dl-1;
        std::smatch tone1;
        std::string lowered = match.format("$1");
        if(std::regex_search(lowered,tone1,std::regex("([cdefgahCDEFGAH])([#'])")))
        {
          bool lower = (strcmp(tone1.format("$2").c_str(),"'")==0)?true:false;
          int idx = lower?-1:1;
          char c = tone1.format("$1").c_str()[0];
          switch (c)
          {
            case 'c':
            case 'C':
              idx = idx + 1;
              break;
            case 'd':
            case 'D':
              idx = idx +3;
              break;
            case 'e':
            case 'E':
              idx = idx + 5;
              break;
            case 'f':
            case 'F':
              idx=idx+6;
              break;
            case 'g':
            case 'G':
              idx = idx+8;
              break;
            case 'a':
            case 'A':
              idx = idx+10;
              break;
            case 'h':
            case 'H':
              idx = idx+12;
              break;
          }
          music.push_back(idx);
          dels.push_back(dl);
        }
        else
        {
          int idx = 0;
          char c = match.format("$1").c_str()[0];
          switch (c)
          {
            case 'c':
            case 'C':
              idx = idx + 1;
              break;
            case 'd':
            case 'D':
              idx = idx +3;
              break;
            case 'e':
            case 'E':
              idx = idx + 5;
              break;
            case 'f':
            case 'F':
              idx=idx+6;
              break;
            case 'g':
            case 'G':
              idx = idx+8;
              break;
            case 'a':
            case 'A':
              idx = idx+10;
              break;
            case 'h':
            case 'H':
              idx = idx+12;
              break;
          }
          music.push_back(idx);
          dels.push_back(dl);
        }
      return true;
      }
      else
      {
        Serial.print("Couldnt recognize tone you are trying to insert");
        return true;
      }
    }
}

void loop(){
  if (Serial.available()>0)
  {
    char buf[10] {'\0'};
    Serial.readBytes(buf,10);
    if (!read_in(buf))
    {  
      Serial.print("ready to play");
      Serial.println(music.size(),DEC);
      for (int i=0; i<music.size();i++)
      {
        ledcWriteTone(ledChannel,octaveC5[music[i]]);
        delay(delays[dels[i]]);
      }
    }
    ledcWriteTone(ledChannel,0);
    
  }
}
