# Xiaomi_MJ_HT_V1_cc2530
Upgrade Xiaomi MJ_HT_V1 Bluetooth LCD temperature sensor to Zigbee cc2530

1. Cutted out BLE chip and installed zigbee cc2530 instead of(it's enough space inside)  . 
2. Connected cc2530 to BU9795AFV LCD driver by four control wires: SCL-P1.2,SDA-P1.3,CSB-P1.4,INHB-P1.5, 
3. Connected cc2530 to SHT3x sensor by two wires: TP9(SCL) - P1.0, TP10(SDA)-P1.1
4. Powered c2530 from point TP15-3V.
