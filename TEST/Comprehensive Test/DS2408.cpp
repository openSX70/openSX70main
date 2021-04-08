#include "DS2408.h"
#include <Arduino.h>
#include <stdio.h>


DS2408::DS2408(uint8_t pin): OneWire(pin) 
{
  //Not implemented
}

uint8_t DS2408::get_register(Device device, Register reg) {
    this->select_device(device);
    return this->get_register(reg);
}

uint8_t DS2408::get_register(Register reg) {
    this->write(DS2408_PIO_READ_CMD, 1);
    this->write(REG_LO(reg), 1);
    this->write(REG_HI(reg), 1);
    return this->read();
}

void DS2408::set_register(Device device, Register reg, uint8_t value) {
    this->select_device(device);
    this->set_register(reg, value);
}
void DS2408::set_register(Register reg, uint8_t value) {
    this->write(DS2408_SEARCH_CMD, 1);
    this->write(REG_LO(reg), 1);
    this->write(REG_HI(reg), 1);
    this->write(value, 1);
}

uint8_t DS2408::get_state(Device device) {
    this->select_device(device);
    return this->get_state();
}
uint8_t DS2408::get_state(bool in_loop) {
    if(!in_loop)
        this->write(DS2408_CHANNEL_READ_CMD, 1);
    return  this->read();
}

bool DS2408::set_state(Device device, uint8_t state) {
    this->select_device(device);
    return this->set_state(state);
}

bool DS2408::set_state(uint8_t state, bool in_loop) {
    if(!in_loop)
        this->write(DS2408_CHANNEL_WRITE_CMD, 1);
    this->write(state, 1);
    this->write(~state, 1);
    if(this->read() == 0xAA && this->read() == state)
        return true;
    return false;
}

void DS2408::set_search_mask(Device device, uint8_t mask) {
    this->select_device(device);
    this->set_search_mask(mask);
}
void DS2408::set_search_mask(uint8_t mask) {
    this->set_register(DS2408_SEARCH_MASK_REG, mask);
}

void DS2408::set_search_polarity(Device device, uint8_t polarity) {
    this->select_device(device);
    this->set_search_polarity(polarity);
}

void DS2408::set_search_polarity(uint8_t polarity) {
    this->set_register(DS2408_SEARCH_SELECT_REG, polarity);
}

void    DS2408::set_mode(Device device, uint8_t mode) {
    this->select_device(device);
    this->set_mode(mode);
}
void    DS2408::set_mode(uint8_t mode) {
    this->set_register(DS2408_CONTROL_STATUS_REG, mode);
}

uint8_t DS2408::get_mode(Device device) {
    this->select_device(device);
    return this->get_mode();
}
uint8_t DS2408::get_mode() {
    return this->get_register(DS2408_CONTROL_STATUS_REG);
}

uint8_t DS2408::get_current_state(Device device) {
    this->select_device(device);
    return this->get_current_state();
}
uint8_t DS2408::get_current_state() {
    return this->get_register(DS2408_PIO_LOGIC_REG);
}

uint8_t DS2408::get_last_state(Device device) {
    this->select_device(device);
    return this->get_last_state();
}
uint8_t DS2408::get_last_state() {
    return this->get_register(DS2408_PIO_OUTPUT_REG);
}

uint8_t DS2408::get_activity(Device device) {
    this->select_device(device);
    return this->get_activity();
}
uint8_t DS2408::get_activity() {
    return this->get_register(DS2408_PIO_ACTIVITY_REG);
}

bool DS2408::reset_activity(Device device) {
    this->select_device(device);
    return this->reset_activity();
}

bool DS2408::reset_activity() {
    this->write(DS2408_RESET_CMD, 1);
    if(this->read() == 0xAA)
        return true;
    return false;
}

void DS2408::select_device(Device device) {
    this->reset();
    this->select(device);
}

uint8_t  DS2408::find(Devices* devices) {
    uint8_t count = 0;
    Device device;
    while(this->search(device)) {
        if(device[0] == DS2408_FAMILY) {
            Serial.println(F("Count DongleFound!!"));
            count++;
        }
    }
    this->reset_search();
    delay(250);
    *devices = (Devices) malloc(count*sizeof(Device));
    for (int index = 0; index < count; index++)
        this->search((*devices)[index]);
    return count;
}

uint8_t  DS2408::findsingle(Device* devices) {
    this->reset_search();
    if (this->search(devices[0]) && devices[0][0] == DS2408_FAMILY)
  {
    return 1;
  }
  else
    return 0;
}
