#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

#Dom 15 Dez 2019 20:35:02 -03
import paho.mqtt.client as mqtt
import locale
import fiot as fenixiot

test=True
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("test/mqtt", 2)
    client.publish("test/mqtt", payload="Test", qos=2, retain=False)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global test
    print("Received message '" + str(msg.payload) + "' on topic '"
        + msg.topic + "' with QoS " + str(msg.qos))
    print(msg.topic+" "+str(msg.payload))
    L=list(msg.payload)
    print(L)
    M=msg.payload
    print(M.decode(os_encoding))
    print(type(msg.payload))
    if (test):
        test=False
        client.publish("test/mqtt", payload=d.set_raw_balance("nano_1cb5fs7xmixqzpitfn9ouy4j1g3hjmdfudc1igt5xhwwps7qdku5htqxmznb", "Testing protocol",
        "678901234567890123456789012345678901"), qos=2, retain=False)


os_encoding = locale.getpreferredencoding()

client = mqtt.Client(client_id="fiot_mqtt_gateway", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)

d=fenixiot.init(None)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
