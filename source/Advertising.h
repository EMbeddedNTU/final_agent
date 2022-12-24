#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include <cstdio>
#include <events/mbed_events.h>

class Advertising : ble::Gap::EventHandler {

public:
  Advertising(BLE &ble, events::EventQueue &event_queue, int DEVICE_ID)
      : _ble(ble), _event_queue(event_queue), _adv_data_builder(_adv_buffer) {
    std::sprintf(DEVICE_NAME, "Agent%d", DEVICE_ID);
  }

  void start() {
    _ble.init(this, &Advertising::on_init_complete);

    // _event_queue.dispatch_forever();
  }

private:
  void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
    if (params->error != BLE_ERROR_NONE) {
      return;
    }

    _ble.gap().setEventHandler(this);
    start_advertising();
  }

  void start_advertising() {
    ble::AdvertisingParameters adv_parameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(100)));

    _adv_data_builder.setFlags();
    _adv_data_builder.setName(DEVICE_NAME);

    ble_error_t error = _ble.gap().setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE, adv_parameters);

    if (error) {
      printf("_ble.gap().setAdvertisingParameters() failed\r\n");
      return;
    }

    error = _ble.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE, _adv_data_builder.getAdvertisingData());

    if (error) {
      printf("_ble.gap().setAdvertisingPayload() failed\r\n");
      return;
    }

    error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

    if (error) {
      printf("_ble.gap().startAdvertising() failed\r\n");
      return;
    }

    printf("start advertising, please connect\r\n");
  }

private:
  BLE &_ble;
  events::EventQueue &_event_queue;
  uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
  ble::AdvertisingDataBuilder _adv_data_builder;
  char DEVICE_NAME[];
};