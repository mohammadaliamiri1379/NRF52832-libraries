/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// libraries 
#include <stdio.h>
#include "mbed.h"
#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "platform/NonCopyable.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattClient.h"
#include "ble/GapAdvertisingParams.h"
#include "ble/GapAdvertisingData.h"
#include "ble/GattServer.h"
#include "BLEProcess.h"

// defines
#define Interrupt_Pin p13 
#define hold_button_duration 2000     // millisecond 
#define short_press 0x0B 
#define long_press 0x0A
#define debouncing_delay_time 2       // millisecond
#define set_duration_short_press 2000
//
DigitalOut led(p17);
DigitalOut led2(p18);
DigitalOut led3(p19);

Timer timer;

//global variables
bool short_press_flag = 0;
bool press_hold_flag = 0 ;
bool push_button = 0 ;

using mbed::callback;

/**
 *for split 16 bit data to separete 8 bit datas
 *and make two byte datas to one 16 bit data
 */
 typedef union
    {
        uint16_t u16;
        int16_t i16;
        uint8_t b[2];   //b[0] = LSB      , b[1] = MSB 
    }data16;
 
 
/**
 * A pin service that demonstrate the GattServer features.
 *
 * The pin service host three characteristics that model the current hour,
 * minute and second of the clock. The value of the second characteristic is
 * incremented automatically by the system.
 *
 */
 
class PinService {
    typedef PinService Self;

public:
    PinService() :
        _second_char("485f4145-52b9-4644-af1f-7a6b9322490f", 0),
        _second_MSB_char("0a924ca7-87cd-4699-a3bd-abdcd9cf126a", 0),
        _second_LSB_char("8dd6a1b7-bc75-4741-8a26-264af75807de", 0),
        _pin_state("c55e1b1e-a28f-11ec-b909-0242ac120002" , 0),
        _Pin_Service(
            /* uuid */ "51311102-030e-485f-b122-f8f381aa84ed",
            /* characteristics */ _clock_characteristics,
            /* numCharacteristics */ sizeof(_clock_characteristics) /
                                     sizeof(_clock_characteristics[0])
        ),
        _server(NULL),
        _event_queue(NULL),
        _interrupt(Interrupt_Pin)       // interrupt pin
        
        
    {
        // update internal pointers (value, descriptors and characteristics array)
        _clock_characteristics[0] = &_second_char;
        _clock_characteristics[1] = &_second_MSB_char;
        _clock_characteristics[2] = &_second_LSB_char;
        _clock_characteristics[3] = &_pin_state;
        
        // setup authorization handlers
        _second_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
        _second_MSB_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
        _second_LSB_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
        _pin_state.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
        
        // setup interrupt 
        _interrupt.mode(PullUp) ;
        _interrupt.rise(callback(this, &Self::rise_stop_time)) ; 
        
        _interrupt.fall(callback(this, &Self::fall_start_time)) ;
        
        
    
    }
    
    
        
    // operation when you press the button
     void fall_start_time()   
     {
        led2=!led2 ;    //toggle led2
        
        timer.stop();
        timer.reset();
        
        timer.start() ; // start the timer
        push_button = 1 ;// when you push the button the flag become 1
        
        
        } 
        
    // operation when you left the button
    void rise_stop_time()
    {
        
        uint8_t pin_state  = 0x00 ;
        int duration = 0 ;  // 
        ble_error_t err = _pin_state.get(*_server, pin_state);
        push_button = 0 ;  // when you releas the button this pin become 0
        led = !led ;      //toggle led
        
        if (err) {
            printf("read of the hour value returned error %u\r\n", err);
            return;  
            }
            
          
          duration = timer.read_ms()   ;            // measure the time beween pressing the button and release the button
           
          
         /** if the press_hold_flag were set in timer_operation() 
           * after releasing the button the pin_state must return
           * to 0
           */ 
          if (pin_state == long_press || press_hold_flag == 1)
        {
              pin_state = 0x00   ;    
              press_hold_flag =  0 ;         //set the press & hold flag to 0
              short_press_flag = 0 ;         // set the short press flag on 0
              err = _pin_state.set(*_server, pin_state);
                if (err)
                    {
                        printf("read of the hour value returned error %u\r\n", err);
                        return;
                    }
              timer.reset() ;      
            }
          
          /** if duration be less than hold_button_duration the short_press
           *  become 1.
           *  if debouncing happend, the duration become's 0, its because 
           *  of the fact that bouncing "mostly" happend under 5 millisecond
           *  and "mostly" the time of holding the button is more than 2 millisecond
           */
           
           
            
           else if(duration < hold_button_duration && duration > debouncing_delay_time  )                                                         
            {
                pin_state = short_press   ;    //show that the flag is short_press 
                press_hold_flag =  0 ;         //set the press & hold flag to 0
                short_press_flag = 1 ;         // set the short press flag on 1
                err = _pin_state.set(*_server, pin_state);
              if (err) {
            printf("read of the hour value returned error %u\r\n", err);
            return;  }
            
            
            /**
             *after 3 second, the pin must set on 0 
             */
             
             
             
            
                } 
            
            
            
        }
    void start(BLE &ble_interface, events::EventQueue &event_queue)
    {
         if (_event_queue) {
            return;
        }

        _server = &ble_interface.gattServer();
        _event_queue = &event_queue;

        // register the service
        printf("Adding demo service\r\n");
        ble_error_t err = _server->addService(_Pin_Service);

        if (err) {
            printf("Error %u during demo service registration.\r\n", err);
            return;
        }

        // read write handler
        _server->onDataSent(as_cb(&Self::when_data_sent));
        _server->onDataWritten(as_cb(&Self::when_data_written));
        _server->onDataRead(as_cb(&Self::when_data_read));

        // updates subscribtion handlers
        _server->onUpdatesEnabled(as_cb(&Self::when_update_enabled));
        _server->onUpdatesDisabled(as_cb(&Self::when_update_disabled));
        _server->onConfirmationReceived(as_cb(&Self::when_confirmation_received));

        // print the handles
        printf("pin service registered\r\n");
        printf("service handle: %u\r\n", _Pin_Service.getHandle());
        printf("\t Second characteristic value handle %u\r\n", _second_char.getValueHandle());
        printf("\t second_MSB characteristic value handle %u\r\n", _second_MSB_char.getValueHandle());
        printf("\t second_LSB characteristic value handle %u\r\n", _second_LSB_char.getValueHandle());
        printf("\t _pin_state characteristic value handle %u\r\n", _pin_state.getValueHandle());
        
        
        _event_queue->call_every(1 /* ms */, callback(this, &Self::increment_millisecond));
        _event_queue->call_every(10 /* ms */, callback(this, &Self::timer_operation));
        
    }

private:


    InterruptIn _interrupt;         // define an InterruptIn object

    
        
    /**
     * Handler called when a notification or an indication has been sent.
     */
    void when_data_sent(unsigned count)
    {
        
        printf("sent %u updates\r\n", count);
    }

    /**
     * Handler called after an attribute has been written.
     */
    void when_data_written(const GattWriteCallbackParams *e)
    {
        
        printf("data written:\r\n");
        printf("\tconnection handle: %u\r\n", e->connHandle);
        printf("\tattribute handle: %u", e->handle);
        if (e->handle == _second_char.getValueHandle()) {
            printf(" (_second characteristic)\r\n");
        } else if (e->handle == _second_MSB_char.getValueHandle()) {
            printf(" (_second_MSB characteristic)\r\n");
        } else if (e->handle == _second_LSB_char.getValueHandle()) {
            printf(" (_second_LSB characteristic)\r\n");
        } else if (e->handle == _pin_state.getValueHandle()) {
            printf(" (_pin_state characteristic)\r\n");
        } else {
            printf("\r\n");
        }
        printf("\twrite operation: %u\r\n", e->writeOp);
        printf("\toffset: %u\r\n", e->offset);
        printf("\tlength: %u\r\n", e->len);
        printf("\t data: ");

        for (size_t i = 0; i < e->len; ++i) {
            printf("%02X", e->data[i]);
        }

        printf("\r\n");
    }

    /**
     * Handler called after an attribute has been read.
     */
    void when_data_read(const GattReadCallbackParams *e)
    {
        
        printf("data read:\r\n");
        printf("\tconnection handle: %u\r\n", e->connHandle);
        printf("\tattribute handle: %u", e->handle);
        if (e->handle == _second_char.getValueHandle()) {
            printf(" (_second_char characteristic)\r\n");
        } else if (e->handle == _second_MSB_char.getValueHandle()) {
            printf(" (second_MSB characteristic)\r\n");
        } else if (e->handle == _second_LSB_char.getValueHandle()) {
            printf(" (_second_LSB_char characteristic)\r\n");
        } else if (e->handle == _pin_state.getValueHandle()) {
            printf(" (_pin_state characteristic)\r\n");
        } else {
            printf("\r\n");
        }
    }

    /**
     * Handler called after a client has subscribed to notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void when_update_enabled(GattAttribute::Handle_t handle)
    {
        printf("update enabled on handle %d\r\n", handle);
    }

    /**
     * Handler called after a client has cancelled his subscription from
     * notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void when_update_disabled(GattAttribute::Handle_t handle)
    {
        printf("update disabled on handle %d\r\n", handle);
    }

    /**
     * Handler called when an indication confirmation has been received.
     *
     * @param handle Handle of the characteristic value that has emitted the
     * indication.
     */
    void when_confirmation_received(GattAttribute::Handle_t handle)
    {
        printf("confirmation received on handle %d\r\n", handle);
    }

    /**
     * Handler called when a write request is received.
     *
     * This handler verify that the value submitted by the client is valid before
     * authorizing the operation.
     */
    void authorize_client_write(GattWriteAuthCallbackParams *e)
    {
        printf("characteristic %u write authorization\r\n", e->handle);

        if (e->offset != 0) {
            printf("Error invalid offset\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
            return;
        }

        if (e->len != 1) {
            printf("Error invalid len\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
            return;
        }

        

        e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
    
    /**
     * cheked if timer passed 3 second, set short_press to 0 
     */
    void timer_operation(void)
    {
        int duration  ; 
        uint8_t pin_state  = 0x00 ;
        ble_error_t err = _pin_state.get(*_server, pin_state);
        duration = timer.read_ms()   ;            // measure the time while we holding the button
        
        // if holding time pass 2 second
        if (duration >= hold_button_duration && push_button == 1 && press_hold_flag == 0 )
        {
              
              pin_state = long_press   ;     //  show that the flag is Press_&_Hold Flag 
              press_hold_flag = 1 ;          // set the press & hold flag to 1
              short_press_flag = 0 ;         // set the short press flag on 0
              err = _pin_state.set(*_server, pin_state);
                if (err)
                    {
                        printf("read of the hour value returned error %u\r\n", err);
                        return;
                    }
            }
            
        //if short_press_flag activate then this function will set pin_state to zero after 3 second           
        else if (duration >= set_duration_short_press && pin_state == short_press)
        {
              
              led3 = !led3 ;
              timer.reset() ;
              pin_state = 0x00   ;     //  show that the flag is Press_&_Hold Flag 
              press_hold_flag = 0 ;          // set the press & hold flag to 1
              short_press_flag = 0 ;         // set the short press flag on 0
              err = _pin_state.set(*_server, pin_state);
                if (err)
                    {
                        printf("read of the hour value returned error %u\r\n", err);
                        return;
                    }
            }
            
            
        // make sure that timer does not crash   
        if (duration >= 30000) 
        {
                    timer.reset();
                    duration = 0;
                    }
                    
                    
            
            
            
         }
        
        
    /**
     * Increment the second counter.
     */
    void increment_millisecond(void)
    {
        uint16_t second = 0;
        uint8_t second_MSB = 0 ;
        uint8_t second_LSB = 0 ;
        data16 data_ ; // define data_ in union data16
       
        
        ble_error_t err1 = _second_LSB_char.get(*_server, second_LSB);
        ble_error_t err2 = _second_MSB_char.get(*_server, second_MSB);
        data_.b[0] = second_LSB ;
        data_.b[1] = second_MSB ;
        second = data_.u16 ;
        if (err1 || err2) {
            printf("read of the second value returned error %u , %u\r\n", err1,err2);
            return;
        }

        second = (second + 1) %1000 ;
        data_.u16 = second ;
        second_LSB = data_.b[0] ;
        second_MSB = data_.b[1] ;
        err1 = _second_LSB_char.set(*_server, second_LSB);  // writing MSB bit
        err2 = _second_MSB_char.set(*_server, second_MSB);  // writing LSB bit
        
        if (err1 || err2) {
            printf("write of the second value returned error %u , %u \r\n", err1 , err2);
            return;
        }

        if (second == 0) {
            increment_second();
        }
    }

    
    
    /**
     * Increment the second counter.
     */
    void increment_second(void)
    {
        uint8_t second = 0;
        ble_error_t err = _second_char.get(*_server, second);
        if (err) {
            printf("read of the hour value returned error %u\r\n", err);
            return;
        }

        second = (second + 1) % 255;

        err = _second_char.set(*_server, second);
        if (err) {
            printf("write of the hour value returned error %u\r\n", err);
            return;
        }
    }
    
    

    
    
private:
    /**
     * Helper that construct an event handler from a member function of this
     * instance.
     */
    template<typename Arg>
    FunctionPointerWithContext<Arg> as_cb(void (Self::*member)(Arg))
    {
        return makeFunctionPointer(this, member);
    }



    /**
     * Read, Write, Notify, Indicate  Characteristic declaration helper.
     *
     * @tparam T type of data held by the characteristic.
     */
    template<typename T>
    class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
    public:
        /**
         * Construct a characteristic that can be read or written and emit
         * notification or indication.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
            GattCharacteristic(
                /* UUID */ uuid,
                /* Initial value */ &_value,
                /* Value size */ sizeof(_value),
                /* Value capacity */ sizeof(_value),
                /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
                /* Descriptors */ NULL,
                /* Num descriptors */ 0,
                /* variable len */ false
            ),
            _value(initial_value) { 
            
        }

        /**
         * Get the value of this characteristic.
         *
         * @param[in] server GattServer instance that contain the characteristic
         * value.
         * @param[in] dst Variable that will receive the characteristic value.
         *
         * @return BLE_ERROR_NONE in case of success or an appropriate error code.
         */
        ble_error_t get(GattServer &server, T& dst) const
        {
            uint16_t value_length = sizeof(dst);
            return server.read(getValueHandle(), &dst, &value_length);
        }

        /**
         * Assign a new value to this characteristic.
         *
         * @param[in] server GattServer instance that will receive the new value.
         * @param[in] value The new value to set.
         * @param[in] local_only Flag that determine if the change should be kept
         * locally or forwarded to subscribed clients.
         */
        ble_error_t set(
            GattServer &server, const uint8_t &value, bool local_only = false
        ) const {
            return server.write(getValueHandle(), &value, sizeof(value), local_only);
        }

    private:
        uint8_t _value;
    };

    ReadWriteNotifyIndicateCharacteristic<uint8_t> _second_char;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _second_MSB_char;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _second_LSB_char;
    ReadWriteNotifyIndicateCharacteristic<uint8_t> _pin_state;
    
    // list of the characteristics of the pin service
    GattCharacteristic* _clock_characteristics[4];

    // demo service
    GattService _Pin_Service;

    GattServer* _server;
    events::EventQueue *_event_queue;
};

int main() {
    BLE &ble_interface = BLE::Instance();
    events::EventQueue event_queue;
    PinService demo_service ;
    BLEProcess ble_process(event_queue, ble_interface);

    ble_process.on_init(callback(&demo_service, &PinService::start));

    // bind the event queue to the ble interface, initialize the interface
    // and start advertising
    ble_process.start();

    // Process the event queue.
    event_queue.dispatch_forever();

    return 0;
}
