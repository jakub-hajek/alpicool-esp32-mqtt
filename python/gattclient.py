import asyncio
from bleak import BleakClient
#from modbus_crc import add_crc
import binascii

def callback(sender, data: bytearray):
    print ("kunda")
    datastr = binascii.hexlify(data)
    print(f"{sender}: {datastr}")

address = "ff:22:12:07:01:b4"
#address = "A7D2A874-0B56-FA6F-8EDF-4A196E261484" #junctek
#address = "820CA30A-54A2-8732-06EA-320F4F3E9BF3" #epever
address = "328CE3CD-AECB-E3AB-DD78-29616EA99574"
notifychar = "00002b13-0000-1000-8000-00805f9b34fb"
writechar =  "00002b11-0000-1000-8000-00805f9b34fb"
MODEL_NBR_UUID = "0000fff1-0000-1000-8000-00805f9b34fb"
async def main(address):
    async with BleakClient(address) as client:
        services = await client.get_services()
        for service in services:
            for char in service.characteristics:
                print (service.uuid + " " + char.uuid)
        await client.start_notify("00001236-0000-1000-8000-00805f9b34fb", callback)
        #BIND
        #await client.write_gatt_char("00001235-0000-1000-8000-00805f9b34fb", bytearray.fromhex("fefe030001ff"), True)

        while True:
            #ping
            x = await client.write_gatt_char("00001235-0000-1000-8000-00805f9b34fb", bytearray.fromhex("fefe03010200"), True)
            await asyncio.sleep(0.6);
            #set temp FEFE04050E001302
            #set temp fefe0305ec02f1
            #x = await client.write_gatt_char("00001235-0000-1000-8000-00805f9b34fb", bytearray.fromhex("FEFE04050E00"), False)
            x = await client.write_gatt_char("00001235-0000-1000-8000-00805f9b34fb",  bytearray.fromhex("FEFE0405000205"), True)
            print(x)
            await asyncio.sleep(0.6);

            #await client.write_gatt_char("00002b16-0000-1000-8000-00805f9b34fb", add_crc(bytearray.fromhex("0104331B0001")), False)
        #    await asyncio.sleep(1);
            
        #    print("kkk")
        
        #print("Model Number: {0}".format("".join(map(chr, model_number))))

asyncio.run(main(address))


#5 stupne
#ESP FEFE0405050A02
#App FEFE040505020A