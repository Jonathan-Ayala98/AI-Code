#include <Arduino.h>
#include <CircuitOS.h>
#include <Spencer.h>
#include <PreparedStatement.h>

const char* listenResult;
IntentResult* intentResult;
bool processingIntent = false;
bool detectingIntent = false;
PreparedStatement* statement = nullptr;
bool synthesizing = false;

void listenProcess(){
  if(synthesizing){
    Serial.println("Another speech synthesis operation is already pending");
  }else{
    synthesizing = true;
    delete statement;
    statement = new PreparedStatement();
    statement->addTTS((intentResult->transcript));
    statement->prepare(speechPlay);
  }

}

void listenError(){

}

void listenCheck(){
  if(listenResult != nullptr && !processingIntent){
    processingIntent = true;
    delete intentResult;
    intentResult = nullptr;
    SpeechToIntent.addJob({ listenResult, &intentResult });

    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-loading4.gif")), true);

  }
  if(processingIntent && intentResult != nullptr){
    detectingIntent = false;
    processingIntent = false;
    listenResult = nullptr;
    if(intentResult->error != IntentResult::Error::OK && intentResult->error != IntentResult::Error::INTENT){
      Serial.printf("Speech to text error %d: %s\n", intentResult->error, STIStrings[(int) intentResult->error]);
      listenError();
      delete intentResult;
      intentResult = nullptr;
      return;
    }
    if(intentResult->intent == nullptr){
      intentResult->intent = (char*) malloc(5);
      memcpy(intentResult->intent, "NONE", 5);
    }
    if(intentResult->transcript == nullptr){
      intentResult->transcript = (char*) malloc(1);
      memset(intentResult->transcript, 0, 1);
    }
    listenProcess();
    delete intentResult;
    intentResult = nullptr;
  }
}

void BTN_press(){
  if (Net.getState() == WL_CONNECTED) {
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-listen.gif")), true);
    if(detectingIntent){
      Serial.println("Another listen and intent detection operation is already pending");
    }else{
      detectingIntent = true;
      listenResult = nullptr;
      Recording.addJob({ &listenResult });
    }
  }

}

void speechPlay(TTSError error, CompositeAudioFileSource* source){
  synthesizing = false;
  if(error != TTSError::OK){
    Serial.printf("Text to speech error %d: %s\n", error, TTSStrings[(int) error]);
    delete source;
    delete statement;
    statement = nullptr;
    Playback.playMP3(SampleStore::load(SampleGroup::Funpack, "help0"));
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-error500.gif")), true);

    return;
  }
  LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-talk.gif")), true);
  Playback.playMP3(source);
  Playback.setPlaybackDoneCallback([](){
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-wink.gif")), true);

  });
  delete statement;
  statement = nullptr;

}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Spencer.begin();
  Spencer.loadSettings();
  Input::getInstance()->setBtnPressCallback(BTN_PIN, BTN_press);

  Net.set("WAVLINK-N", "Doge!007\$\$");
  LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-wifi.gif")), true);
  Net.connect([](wl_status_t state){
    if (Net.getState() == WL_CONNECTED) {
      Playback.playMP3(SampleStore::load(SampleGroup::Funpack, "whatsup0"));
      LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-smile.gif")), true);
    } else {
      Playback.playMP3(SampleStore::load(SampleGroup::Error, "wifi"));
      LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-noWifi.gif")), true);
    }

  });


}

void loop() {
  LoopManager::loop();
  listenCheck();


}
