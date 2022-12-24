#include "Advertising.h"
#include "ble/BLE.h"
#include "logger.h"
#include "stm32l475e_iot01_gyro.h"
#include <cstdio>
#include <events/mbed_events.h>
#include <http_service.h>
#include <iostream>
#include <mbed.h>
#include <pch.h>
#include "core/fileSystem.h"

#define PWM_OUT PA_15
#define LOCK_OUT PA_4
#define T 0.01

PwmOut CLK(PWM_OUT);
DigitalOut LOCK(LOCK_OUT);
// std::string bodydata;
InterruptIn button(BUTTON1);
GSH::HttpService &http_service = GSH::HttpService::GetInstance();
GSH::FileSystem* fs = new GSH::FileSystem();
static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);
static events::EventQueue event_queue2(16 * EVENTS_EVENT_SIZE);
Thread t;

const static int DEVICE_ID = 1;
const static char WIFI_SSID[] = "509-2";
const static char WIFI_KEY[] = "max30201";

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

void setState(const char* bodydata){
    GSH_WARN("save %s", bodydata);
    FILE* f = fs->openFile("state.txt", "w+");
    fprintf(f, "%s", bodydata);
    // fflush(f);
    fs->closeFile(f);
}

void getState(char* buf){
    GSH_ERROR("123");
    FILE* f = fs->openFile("state.txt", "r");
    fseek(f, 0L, SEEK_SET);
    fread(buf, sizeof(char), 20, f);
    fs->closeFile(f);
    GSH_ERROR("123");
}

void updateLightState(double state) {
  CLK.period(T);
  CLK.write(state);
}

void send_notification() {
  char msg[1024];
  sprintf(msg,
          "{\"title\": \"門鈴響了\",\"time\": \"\",\"content\": "
          "\"Agent%d的門鈴響了\"}",
          DEVICE_ID);

  SharedPtr<GSH::HttpService::HttpResponse> response = http_service.http_post(
      "http://192.168.0.101:3000/notification/postNotification", NULL, msg);
}

void button_pressed() { event_queue.call(send_notification); }

void updateLockState(int state) {
  if (state == 1) {
    LOCK = true;
  } else if (state == 0) {
    LOCK = false;
  }
}

void polling() {
  SharedPtr<GSH::HttpService::HttpResponse> response =
      http_service.http_get("http://192.168.0.101:3000/agent/1", NULL);
  if (response->body != "") {
      if(response->body[1] == '0'){
                    event_queue2.call(setState, "00");
      } else if(response->body[1] == '1'){
          event_queue2.call(setState, "01");
      } else if(response->body[1] == '2'){
          event_queue2.call(setState, "02");
      }else if(response->body[1] == '3'){
          event_queue2.call(setState, "03");
      } else {
          event_queue2.call(setState, "04");
      }
    for (int i = 0; i < response->body.length(); i += 2) {
      if (response->body[i] == '0') {
        updateLightState(double(response->body[i + 1] - '0') / 4);
      } else if (response->body[i] == '1') {
        updateLockState(response->body[i + 1] - '0');
      }
    }
  }
}

int main() {
    fs->init(false);
    char buf[20];
    // setState("04");
    getState(buf);
    GSH_DEBUG("%s", buf);
    GSH_INFO("%d", strlen(buf));
    int state = 0;
    if (strlen(buf) != 0) {
        GSH_INFO("recover from state");
        state = buf[1] - '0';
    }
// GSH_WARN("%d", state);
// return 0;

  GSH::HttpService &http_service = GSH::HttpService::GetInstance();
  http_service.init(WIFI_SSID, WIFI_KEY);

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  Advertising advertising(ble, event_queue, DEVICE_ID);

  char msg[1024];
  sprintf(
      msg,
      "{\"id\": %d,\"name\": \"Agent%d\",\"location\": "
      "\"location%d\",\"functionStateList\": [{\"type\": 0,\"lightState\": %d}]}",
      DEVICE_ID, DEVICE_ID, DEVICE_ID, state);

  SharedPtr<GSH::HttpService::HttpResponse> response = http_service.http_post(
      "http://192.168.0.101:3000/agent/register", NULL, msg);

  button.fall(&button_pressed);


    // event_queue2.call_every(10s, setState);
t.start(callback(&event_queue2, &EventQueue::dispatch_forever));
    // event_queue2.dispatch_forever();

  event_queue.call_every(10s, polling);
  advertising.start();
  event_queue.dispatch_forever();
}