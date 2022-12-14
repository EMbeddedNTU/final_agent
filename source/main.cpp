#include "Advertising.h"
#include "ble/BLE.h"
#include "logger.h"
#include <cstdio>
#include <events/mbed_events.h>
#include <http_service.h>
#include <mbed.h>
#include <pch.h>

DigitalOut led1(LED1);
GSH::HttpService &http_service = GSH::HttpService::GetInstance();
static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

const static int DEVICE_ID = 1;
const static char WIFI_SSID[] = "509-2";
const static char WIFI_KEY[] = "max30201";

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

void freeResponse(GSH::HttpService::HttpResponse *hresp) {
  if (hresp != NULL) {
    if (hresp->body != NULL)
      free(hresp->body);
    if (hresp->status_code != NULL)
      free(hresp->status_code);
    if (hresp->status_text != NULL)
      free(hresp->status_text);
    if (hresp->request_headers != NULL)
      free(hresp->request_headers);
    if (hresp->response_headers != NULL)
      free(hresp->response_headers);
    free(hresp);
  }
}

int num = 0;

void polling() {
  GSH::HttpService::HttpResponse *response =
      http_service.http_get("http://192.168.0.101:3000/agent/1", NULL);
  GSH_ERROR("%s %d", response->body, num);
  if (response->body[19] == '0') {
    led1 = false;
  } else {
    led1 = true;
  }
  freeResponse(response);
  num += 1;
}

int main() {
  http_service.init(WIFI_SSID, WIFI_KEY);
  led1 = true;

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  Advertising advertising(ble, event_queue, DEVICE_ID);

  char msg[1024];
  sprintf(msg,
          "{\"id\": %d,\"name\": \"Agent%d\",\"location\": "
          "0,\"functionStateList\": [{\"type\": 0,\"state\": 0}]}",
          DEVICE_ID, DEVICE_ID);

  GSH::HttpService::HttpResponse *response = http_service.http_post(
      "http://192.168.0.101:3000/agent/register", NULL, msg);

  event_queue.call_every(1s, polling);
  advertising.start();

  return 0;
}