# mqtt-boiler-tdisplay3v2

/homeassistant/mqtt-boiler_tdisplay3v2.yaml

switch:
  - name: "Boiler: power"
    unique_id: "boiler_power"
    state_topic: "mqtt_boiler/status"     # (ON/OFF)
    command_topic: "mqtt_boiler/set"      # 
    payload_on: "1"
    payload_off: "0"
    availability_topic: "mqtt_boiler/availability" # LWT
    payload_available: "OK"
    payload_not_available: "Fail"
    device: &boiler_device
        identifiers: "mqtt-boiler"
        name: "TTGO"
        model: "T-Display S3 v2"
        manufacturer: "Custom DIY"

sensor:
  - name: "Boiler: temp in tank"
    unique_id: "boiler_temp_current"
    state_topic: "mqtt_boiler/curtemp"
    unit_of_measurement: "°C"
    device_class: "temperature"
    value_template: "{{ value | float | round(1) }}"
    availability_topic: "mqtt_boiler/availability" # LWT
    payload_available: "OK"
    payload_not_available: "Fail"    
    device: *boiler_device

  - name: "Boiler: temp by sun"
    unique_id: "boiler_temp_sun"
    state_topic: "mqtt_boiler/suntemp"
    unit_of_measurement: "°C"
    device_class: "temperature"
    value_template: "{{ value | float | round(1) }}"
    availability_topic: "mqtt_boiler/availability" # LWT
    payload_available: "OK"
    payload_not_available: "Fail"
    device: *boiler_device
    
number:
  - name: "Boiler: timer"
    unique_id: "boiler_timer"
    state_topic: "mqtt_boiler/time"         # 
    command_topic: "mqtt_boiler/set_time"   # 
    availability_topic: "mqtt_boiler/availability" # LWT
    payload_available: "OK"
    payload_not_available: "Fail"
    min: 0
    max: 90
    step: 1
    unit_of_measurement: "min"
    mode: "box"
    device: *boiler_device

