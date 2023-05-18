mkspiffs -c ./data/ -p 256 -b 8192 -s 1028096 ./build/out.spiffs
esptool.py -cd nodemcu -cb 460800 -cp /dev/ttyUSB0 -ca 0x300000 -cf ./build/out.spiffs