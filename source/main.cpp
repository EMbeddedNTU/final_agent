#include "ble/BLE.h"
#include <Advertising.h>
#include <cstdio>
#include <events/mbed_events.h>
#include <http_service.h>
#include <mbed.h>
#include <pch.h>

DigitalOut led1(LED1);

GSH::HttpService &http_service = GSH::HttpService::GetInstance();

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

void polling() {
  GSH::HttpService::HttpResponse *response =
      http_service.http_get("http://192.168.0.101:3000", NULL);
  if (strcmp(response->body, "Hello World!") == 0) {
    led1 = !led1;
  }
}

int main() {
  http_service.init("509-2", "max30201");
  led1 = true;

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  Advertising advertising(ble, event_queue);

  event_queue.call_every(5s, polling);
  advertising.start();

  return 0;
}