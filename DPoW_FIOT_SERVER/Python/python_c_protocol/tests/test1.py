#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

import paho.mqtt.client as mqtt
import locale
import fiot as fenixiot

#Qua 25 Dez 2019 19:51:52 -03 

test=2

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("test/mqtt", 2)
    client.publish("test/mqtt", payload="Test message", qos=2, retain=False)

def on_message(client, userdata, msg):
    global test
    print("Received message '" + str(msg.payload) + "' on topic '"
        + msg.topic + "' with QoS " + str(msg.qos))

    print("Raw data protocol: ")
    print(list(msg.payload))

    if (test):
        test-=1
        if (test):
            client.publish("test/mqtt", payload=d.set_raw_balance("nano_1cb5fs7xmixqzpitfn9ouy4j1g3hjmdfudc1igt5xhwwps7qdku5htqxmznb",
                "test/mqtt/fiot/balance", "68901234567890123456789012345678901"), qos=2, retain=False)
        else:
            client.publish("test/mqtt", payload=d.send_dpow("xrb_1cb5fs7xmixqzpitfn9ouy4j1g3hjmdfudc1igt5xhwwps7qdku5htqxmznb",
                "test/mqtt/fiot/dpow", "de0c84215a6b7429d3d2836f54b6b917c9301103134904457a928c56580cf5a4", 0xfe304ac2d57193ab), qos=2, retain=False)

os_encoding = locale.getpreferredencoding()

client = mqtt.Client(client_id="fiot_mqtt_gateway", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)

d=fenixiot.init(None)

client.loop_forever()

