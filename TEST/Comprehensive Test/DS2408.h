#ifndef DS2408_H
#define DS2408_H

#include <inttypes.h>

#include "OneWire.h"

/**
 * OneWire Commands
 **/
#define ONEWIRE_READ_CMD           0x33
#define ONEWIRE_MATCH_CMD          0x55
//#define ONEWIRE_SEARCH_CMD         0xF0
#define ONEWIRE_SKIP_CMD           0xCC
#define ONEWIRE_SEARCH_CMD         0xEC
#define ONEWIRE_OVERRIDE_MATCH_CMD 0x69
#define ONEWIRE_OVERRIDE_SKIP_CMD  0x3C
#define ONEWIRE_RESUME_CMD         0xA5


// DS2408
#define DS2408_FAMILY 0x29

// Commands
#define DS2408_PIO_READ_CMD      0xF0
#define DS2408_CHANNEL_READ_CMD  0xF5
#define DS2408_CHANNEL_WRITE_CMD 0x5A
#define DS2408_SEARCH_CMD        0xCC
#define DS2408_RESET_CMD         0xC3


// Register Addresses
#define DS2408_PIO_LOGIC_REG       0x0088   // Current state
#define DS2408_PIO_OUTPUT_REG      0x0089   // Last write
#define DS2408_PIO_ACTIVITY_REG    0x008A   // State Change Activity
#define DS2408_SEARCH_MASK_REG     0x008B
#define DS2408_SEARCH_SELECT_REG   0x008C
#define DS2408_CONTROL_STATUS_REG  0x008D

#define DS2408_PLS_STATUS_MASK  0x0   // 
#define DS2408_CT_STATUS_MASK   0x1   // Conditional Search Term
#define DS2408_ROS_STATUS_MASK  0x2   // Reset state
#define DS2408_PORL_STATUS_MASK 0x3   // Power-On Reset status
#define DS2408_VCCP_STATUS_MASK 0x7   //


// Control/Status Bits
#define SEARCH_TRIGGER_BIT  0x0
#define SEARCH_TERM_BIT     0x1
#define RESET_PIN_BIT       0x2
#define POWERON_RESET_BIT   0x3
#define VCCC_POWER_BIT      0x7


// SEARCH_TRIGGER modes
#define PIN_TRIGGER         0x0
#define ACTIVITY_TRIGGER    0x1

// SEARCH_TERM modes
#define OR_TERM             0x0
#define AND_TERM            0x1

// RESET_PIN_MODE modes
#define RESET_MODE          0x0
#define STROBE_MODE         0x1

// POWERON_RESET_MODE modes
#define RESET_ON            0x0
#define RESET_OFF           0x1

// Control modes
#define SEARCH_TRIGGER(mode)      (mode<<SEARCH_TRIGGER_BIT)
#define SEARCH_TERM(mode)         (mode<<SEARCH_TERM_BIT)
#define RESET_PIN_MODE(mode)      (mode<<RESET_PIN_BIT)
#define POWERON_RESET_MODE(mode)  (mode<<POWERON_RESET_BIT)


#define REG_LO(addr) (addr&0xFF)
#define REG_HI(addr) (addr>>8)


typedef uint8_t  Device[8];
typedef Device*   Devices;
typedef uint16_t Register;
typedef uint8_t  CRC8;
typedef uint16_t CRC16;




struct DS2408: OneWire {
    DS2408(uint8_t pin);

    // Commands
    uint8_t get_register(Device device, Register reg);
    uint8_t get_register(Register reg);

    void    set_register(Device device, Register reg, uint8_t value);
    void    set_register(Register reg, uint8_t value);



    uint8_t get_state(Device device);
    uint8_t get_state(bool in_loop=false);

    bool    set_state(Device device, uint8_t state);
    bool    set_state(uint8_t state, bool in_loop=false);


    bool reset_activity(Device device);
    bool reset_activity();


    // Registers
    void set_search_mask(Device device, uint8_t mask);
    void set_search_mask(uint8_t mask);

    void set_search_polarity(Device device, uint8_t polarity);
    void set_search_polarity(uint8_t polarity);


    void    set_mode(Device device, uint8_t mode);
    void    set_mode(uint8_t mode);             // Control/Status Register

    uint8_t get_mode(Device device);
    uint8_t get_mode();                         // Control/Status Register


    uint8_t get_current_state(Device device);
    uint8_t get_current_state();                // Logic State  Register

    uint8_t get_last_state(Device device);
    uint8_t get_last_state();                   // Output   Latch State Register

    uint8_t get_activity(Device device);
    uint8_t get_activity();                     // Activity Latch state Register

    // Other
    uint8_t find(Devices* devices);
  uint8_t findsingle(Device* devices);
  
private:
    void select_device(Device device);
};

#endif // DS2408_H
