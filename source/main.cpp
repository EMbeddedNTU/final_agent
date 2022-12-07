#include "ble/BLE.h"
#include <Advertising.h>
#include <events/mbed_events.h>
#include <mbed.h>

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

int main() {

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  Advertising advertising(ble, event_queue);
  advertising.start();

  return 0;
}