#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

import paho.mqtt.client as mqtt
import locale
import fiot as fenixprotocol

#Sáb 04 Jan 2020 22:32:53 -03

################## ENCODING BEGIN ######################

os_encoding=locale.getpreferredencoding()

########## Fenix-IoT Nano DPoW service begin ###########

def fenix_onerror(e):
    if (e):
        print("Error: "+str(e.err))
        print("Error name: "+e.errname)
        print("Reason: "+e.msg)
    else:
        print("\nUnknown error\n")

def fenix_onreceive(protocol):
    command=protocol.s0
    data_type=protocol.s1
    data_size=protocol.s2
    protocol_version=protocol.s3
    last_msg_id=protocol.s4
    prot_timestamp=protocol.s5
    publish_callback=protocol.s6
    rawdata=protocol.s7
    ret=None

    if (command==fenixprotocol.CMD_GET_RAW_BALANCE):
        ret=fenixiot.set_raw_balance(None, None, "12345678901234567890")
        if (ret==None):
            msg="CMD_GET_RAW_BALANCE"
    elif (command==fenixprotocol.CMD_GET_FRONTIER):
        ret=fenixiot.set_frontier(None, None, "cb9850660e23e03205c2582875e9af3b2e8d075aeab5889de16bdc7cc76e5ef7")
        if (ret==None):
            msg="CMD_GET_FRONTIER"
    elif (command==fenixprotocol.CMD_GET_DPOW):
        ret=fenixiot.send_dpow(None, None, "1cf5dad7e7f75f878be7cebe792484a2ec252dc4e9c5fad165dafd133c9fcd19", 0xc158c4ed567661cd)
        if (ret==None):
            msg="CMD_GET_DPOW"
    elif (command==fenixprotocol.CMD_GET_REPRESENTATIVE):
        ret=fenixiot.send_representative(None, None, "xrb_1cb5fs7xmixqzpitfn9ouy4j1g3hjmdfudc1igt5xhwwps7qdku5htqxmznb")
        if (ret==None):
            msg="CMD_GET_REPRESENTATIVE"
    else:
        msg="UNKNOWN_COMMAND"

    if (ret):
        client.publish(publish_callback, payload=ret, qos=2, retain=False)
        msg="Success"
    else:
        print("Data not sent to client '"+publish_callback+"' with request "+msg)
        msg="Fail"

    print("Finishing with "+msg)

fenixiot=fenixprotocol.init(None)
fenixiot.onerror(fenix_onerror)
fenixiot.ondata(fenix_onreceive)

################### MQTT SERVICE BEGIN ##################

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("test/nanodpow", 2)

def on_message(client, userdata, msg):
    print("Received message '" + str(msg.payload) + "' on topic '"
        + msg.topic + "' with QoS " + str(msg.qos))
    fenixiot.getdataprotocol(msg.payload)

client=mqtt.Client(client_id="fiot_mqtt_gateway", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
client.on_connect=on_connect
client.on_message=on_message

client.connect("localhost", 1883, 60)

client.loop_forever()

