# alpicool-esp32-mqtt
Alpicool fridge to mqtt gateway for esp32 boards. Supports basic state readings, setting temperature, turning the fridge on/off and de/activating low power (eco) mode.

## Installation
- This project uses [Platformio](https://platformio.org)
- clone this repo
- edit src/config/config.h
  - you will need BLE address of your fridge
  - run `python ./python/gatt.py` to search all ble devices around. The fridge must not be paired in the mobile app. (this does not work on Mac as Apple does not expose BLE adresses for security (thru obscurity) reasons.)
  - look for the devices with name starting with `A1-`, `AK1-`, `AK2-`, or `AK3-` 
- compile and upload
- ```bash  
  platformio run --target upload --environment esp32dev
  ```
- add to home assistant

```yaml
mqtt:
  - climate:
      name: "Alpicool fridge"
      unique_id: "alpicoolfriddgeuid"
      max_temp: 20
      min_temp: -20
      modes:
        - "off"
        - "cool"
      current_temperature_topic: "tele/alpicool/sensor"
      current_temperature_template: "{{ value_json.f_actual_temperature | round(1, 'floor') }}"
      preset_modes:
        - "eco"
        - "burst"
      preset_mode_command_template: >-
        {% set jsonon = { 'fridge_eco': True } %}
        {% set jsonoff = { 'fridge_eco': False } %}
        {{ jsonon | tojson if value == 'eco' else jsonoff | tojson }}
      preset_mode_state_topic: "tele/alpicool/sensor"
      preset_mode_value_template: >-
        {{ 'eco' if value_json.f_eco == True else 'burst' }}
      preset_mode_command_topic: "tele/alpicool/cmd"
      mode_state_topic: "tele/alpicool/sensor"
      mode_state_template: >-
        {{ 'cool' if value_json.f_on == True else 'off' }}
      mode_command_topic: "tele/alpicool/cmd"
      mode_command_template: >-
        {% set jsonon = { 'fridge_on': True } %}
        {% set jsonoff = { 'fridge_on': False } %}
        {{ jsonon | tojson if value == 'cool' else jsonoff | tojson }}
      power_command_topic: "tele/zalpicool/cmd"
      payload_on: >
        { "fridge_on":true } "
      payload_off: >
        { "fridge_on":false } "
      temperature_command_topic: "tele/alpicool/cmd"
      temperature_command_template: >
        {"fridge_temperature": {{ value }} }"
      temperature_state_topic: "tele/alpicool/sensor"
      temperature_state_template: "{{ value_json.f_desired_temperature | round(1, 'floor')}}"
      temp_step: 1
      precision: 1
  - binary_sensor:
      name: "Fridge On"
      unique_id: zmkfridgeonoff
      state_topic: "tele/alpicool/sensor"
      payload_on: true
      payload_off: false
      value_template: "{{ value_json.f_on }}"

  - binary_sensor:
      name: "Fridge Eco Mode"
      unique_id: zmkfridgeecomode
      state_topic: "tele/alpicool/sensor"
      payload_on: true
      payload_off: false
      value_template: "{{ value_json.f_eco }}"

  - sensor:
      name: "Fridge Set Temperature"
      unique_id: zmkfridgessettemperature
      state_topic: "tele/alpicool/sensor"
      suggested_display_precision: 0
      unit_of_measurement: "°C"
      value_template: "{{ value_json.f_desired_temperature }}"

  - sensor:
      name: "Fridge Temperature"
      unique_id: zmkfridgetemperature
      state_topic: "tele/alpicool/sensor"
      suggested_display_precision: 0
      unit_of_measurement: "°C"
      value_template: "{{ value_json.f_actual_temperature }}"

  - sensor:
      name: "Fridge Voltage"
      unique_id: zmkfridgevoltage
      state_topic: "tele/alpicool/sensor"
      suggested_display_precision: 2
      unit_of_measurement: "V"
      value_template: "{{ value_json.f_voltage | round(2, 'floor') }}"

  
```

## Credit
This is based on [@johnelliott](https://github.com/johnelliott)'s [alpicoold](https://github.com/johnelliott/alpicoold) implementation in Go.

Thanks to [@klightspeed](https://github.com/klightspeed) for reverse engineering the protocol and documenting the BLE names.

