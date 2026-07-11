//
// Created by Avinab on 10/6/2025.
//
#pragma once

// BLEDevice.h provides the complete BLE type set on both the Bluedroid-backed
// Arduino-ESP32 2.x implementation and the NimBLE-backed 3.x compatibility
// implementation. BLEUtils.h alone no longer supplies these declarations.
#include <BLEDevice.h>

#define NORDIC_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
#define NORDIC_RX_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
#define NORDIC_TX_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

constexpr size_t BLE_MAX_PACKET_SIZE = ESP_GATT_MAX_ATTR_LEN; // must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN
constexpr size_t BLE_RX_BUFFER_SIZE = 4096;

constexpr uint16_t DEFAULT_BLE_TX_TIMEOUT = 200;
constexpr uint16_t DEFAULT_BLE_TX_NOTIFY_DELAY = 20;
constexpr uint16_t DEFAULT_BLE_SEND_QUEUE_SIZE = 10;

#if defined(CONFIG_NIMBLE_ENABLED)
#define BLE_SERIAL_NEEDS_2902_DESCRIPTOR 0
#else
#define BLE_SERIAL_NEEDS_2902_DESCRIPTOR 1
#endif

typedef void (*BLE_CONNECT_CALLBACK)(bool connected);
