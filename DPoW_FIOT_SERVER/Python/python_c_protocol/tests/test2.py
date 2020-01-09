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

debug=True

################## ENCODING BEGIN ######################

os_encoding=locale.getpreferredencoding()

########## Fenix-IoT Nano DPoW service begin ###########

def fenix_debug(prot):
    global debug
    if (debug):
        if (hasattr(prot, "s0")):
            print("s0:")
            print(prot.s0)
        if (hasattr(prot, "s1")):
            print("s1:")
            print(prot.s1)
        if (hasattr(prot, "s2")):
            print("s2:")
            print(prot.s2)
        if (hasattr(prot, "s3")):
            print("s3:")
            print(prot.s3)
        if (hasattr(prot, "s4")):
            print("s4:")
            print(prot.s4)
        if (hasattr(prot, "s5")):
            print("s5:")
            print(prot.s5)
        if (hasattr(prot, "s6")):
            print("s6:")
            print(prot.s6)
        if (hasattr(prot, "s7")):
            print("s7:")
            print(prot.s7)
        if (hasattr(prot, "s8")):
            print("s8:")
            print(prot.s8)
        if (hasattr(prot, "s9")):
            print("s9:")
            print(prot.s9)

def fenix_onerror(e):
    if (e):
        print("Error: "+str(e.err))
        print("Error name: "+e.errname)
        print("Reason: "+e.msg)
        type(e.errname)
    else:
        print("\nUnknown error\n")

def fenix_onreceive(protocol):
    fenix_debug(protocol)
    if (hasattr(protocol, "s0")):
        command=protocol.s0
    else:
        command=None
    if (hasattr(protocol, "s6")):
        publish_callback=protocol.s6
    else:
        publish_callback=None
    ret=None
    msg=""
    if (command==fenixprotocol.CMD_GET_RAW_BALANCE):
        ret=fenixiot.set_raw_balance(None, None, "5000000000000000000000000000")
        if (ret==None):
            msg="CMD_GET_RAW_BALANCE"
    elif (command==fenixprotocol.CMD_GET_FRONTIER):
        ret=fenixiot.set_frontier(None, None, "cb9850660e23e03205c2582875e9af3b2e8d075aeab5889de16bdc7cc76e5ef7")
        if (ret==None):
            msg="CMD_GET_FRONTIER"
    elif (command==fenixprotocol.CMD_GET_DPOW):
        hash_str=fenixiot.get_dpow_hash_from_client()
        if (hash_str!=None):
            ret=fenixiot.send_dpow(None, None, hash_str, 0xc158c4ed567661cd)
            if (ret==None):
                msg="CMD_GET_DPOW"
            else:
                print("Return (raw data to send): ->")
                print(list(ret))
        else:
            msg="get_dpow_hash_from_client()"
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
        err=fenixiot.getlasterror()
        print("Trying to send error reasons to client ...")
        if ((err==fenixprotocol.F_ERR_FORBIDDEN_OVFL_PUBL_STR)or(err==fenixprotocol.F_ERR_FORBIDDEN_NULL_PUB_STR)):
            print("Could not send error to client. Invalid publish callback "+fenixprotocol.geterrorname(err))
        else:
            ret=fenixiot.senderrortoclient(None, err, "Error with name: "+fenixprotocol.geterrorname(err))
            if (ret):
                client.publish(publish_callback, payload=ret, qos=2, retain=False)
            else:
                err=fenixiot.getlasterror()
                print("Sencond fail when sending error reason to client "+str(err)+" Error name: "+fenixprotocol.geterrorname(err))
    print("Finishing with "+msg)

fenixiot=fenixprotocol.init(None)
fenixiot.onerror(fenix_onerror)
fenixiot.ondata(fenix_onreceive)

################### MQTT SERVICE BEGIN ##################

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("test/dpow", 2)

def on_message(client, userdata, msg):
    print("Received message '"+str(msg.payload)+"' on topic '"+msg.topic+"' with QoS "+str(msg.qos))
    fenixiot.getdataprotocol(msg.payload)

client=mqtt.Client(client_id="fiot_mqtt_gateway", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
client.on_connect=on_connect
client.on_message=on_message

client.connect("localhost", 1883, 60)

client.loop_forever()

