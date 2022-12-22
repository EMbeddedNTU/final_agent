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

#define PWM_OUT PA_15
#define LOCK_OUT PA_4
#define T 0.01

PwmOut CLK(PWM_OUT);
DigitalOut LOCK(LOCK_OUT);
InterruptIn button(BUTTON1);
GSH::HttpService &http_service = GSH::HttpService::GetInstance();
static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

const static int DEVICE_ID = 1;
const static char WIFI_SSID[] = "509-2";
const static char WIFI_KEY[] = "max30201";

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
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
  GSH::HttpService &http_service = GSH::HttpService::GetInstance();
  http_service.init(WIFI_SSID, WIFI_KEY);

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  Advertising advertising(ble, event_queue, DEVICE_ID);

  char msg[1024];
  sprintf(
      msg,
      "{\"id\": %d,\"name\": \"Agent%d\",\"location\": "
      "\"location%d\",\"functionStateList\": [{\"type\": 0,\"lightState\": 0}, "
      "{\"type\": 1,\"lockState\": 0}]}",
      DEVICE_ID, DEVICE_ID, DEVICE_ID);

  SharedPtr<GSH::HttpService::HttpResponse> response = http_service.http_post(
      "http://192.168.0.101:3000/agent/register", NULL, msg);

  button.fall(&button_pressed);

  event_queue.call_every(1s, polling);
  advertising.start();
}