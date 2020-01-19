#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

import paho.mqtt.client as mqtt
import locale
import fiot as fenixprotocol
import requests
import json
import asyncio
import urllib3

#Qui 09 Jan 2020 22:40:39 -03 

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
    global NANO_PREFERED_REPRESENTATIVE
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
    err=0
    errorname=""
    parm=None
    if (command==fenixprotocol.CMD_GET_NEXT_PENDING_ACCOUNT):
        if (hasattr(protocol, "s7")):
            wallet=protocol.s7.decode('ascii').rstrip('\0')
            parm='{"action":"accounts_pending","accounts":["'+wallet+'"],"count":"1","source":"true"}'
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(nano_node_srv(data=parm))
            except Exception as e:
                err=10000
                errorname="Error: 'nano_node_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            if (res):
                if (res.status_code==200):
                    try:
                        k=res.json()
                    except Exception as e:
                        k=None
                        err=10002
                        errorname="Error: Can't parse NANO node JSON "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
                    if (k):
                        if ('blocks' in k):
                            k=k['blocks']
                            for m in k:
                                break
                            if (k[m]==""):
                                if (fenixprotocol.is_equivalent_and_valid(m, wallet)):
                                    ret=fenixiot.set_next_pending(None, None, None, "", "")
                                    if (ret==None):
                                        msg="CMD_GET_NEXT_PENDING_ACCOUNT(None pending)"
                                        err=fenixiot.getlasterror()
                                        errorname=fenixprotocol.geterrorname(err)
                                else:
                                    err=10007
                                    errorname="Error: Nano wallet "+wallet+" is different from "+m+". Error no.: "+str(err)
                            elif (fenixprotocol.is_equivalent_and_valid(m, wallet)):
                                k=k.pop(m)
                                for m in k:
                                    block_hash=m
                                    k=k.pop(m)
                                    break
                                if 'amount' in k:
                                    amount=k['amount']
                                    if 'source' in k:
                                        ret=fenixiot.set_next_pending(None, None, k['source'], amount, block_hash)
                                        if (ret==None):
                                            msg="CMD_GET_NEXT_PENDING_ACCOUNT"
                                            err=fenixiot.getlasterror()
                                            errorname=fenixprotocol.geterrorname(err)
                                    else:
                                        err=10010
                                        errorname="Error: Source of pending amount not found. Error no.: "+str(err)
                                else:
                                    err=10008
                                    errorname="Error: Pending amount not found. Error no.: "+str(err)
                            else:
                                err=10007
                                errorname="Error: Nano wallet "+wallet+" is different from "+m+". Error no.: "+str(err)
                        elif ('error' in k):
                            err=10003
                            errorname="Error: NANO error "+k['error']+". Error no.: "+str(err)
                        else:
                            err=10004
                            errorname="Error: NANO error unknown JSON param. Error no.: "+str(err)
                else:
                    err=10001
                    errorname="Error: NANO node status code: "+str(res.status_code)
        else:
            err=10005
            errorname="Error: Empty slot 7 fatal error "+str(err)
    elif (command==fenixprotocol.CMD_GET_RAW_BALANCE):
        if (hasattr(protocol, "s7")):
            parm='{"action":"account_balance","account":"'+protocol.s7.decode('ascii').rstrip('\0')+'"}'
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(nano_node_srv(data=parm))
            except Exception as e:
                err=10000
                errorname="Error: 'nano_node_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            if (res):
                if (res.status_code==200):
                    try:
                        k=res.json()
                    except Exception as e:
                        k=None
                        err=10002
                        errorname="Error: Can't parse NANO node JSON "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
                    if (k):
                        if ('balance' in k):
                            balance=k['balance']
                            if ('pending' in k):
                                pending=k['pending']
                                ret=fenixiot.set_raw_balance(None, None, balance, pending)
                                if (ret==None):
                                    msg="CMD_GET_RAW_BALANCE"
                                    err=fenixiot.getlasterror()
                                    errorname=fenixprotocol.geterrorname(err)
                            else:
                                err=10006
                                errorname="Error missing Nano pending value. Error no.: "+str(err)
                        elif ('error' in k):
                            err=10003
                            errorname="Error: NANO error "+k['error']+". Error no.: "+str(err)
                        else:
                            err=10004
                            errorname="Error: NANO error unknown JSON param. Error no.: "+str(err)

                else:
                    err=10001
                    errorname="Error: NANO node status code: "+str(res.status_code)
        else:
            err=10005
            errorname="Error: Empty slot 7 fatal error "+str(err)
    elif (command==fenixprotocol.CMD_GET_FRONTIER):
        if (hasattr(protocol, "s7")):
            wallet=protocol.s7.decode('ascii').rstrip('\0')
            parm='{"action":"accounts_frontiers","accounts":["'+wallet+'"]}'
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(nano_node_srv(data=parm))
            except Exception as e:
                err=10000
                errorname="Error: 'nano_node_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            if (res):
                frontier=""
                if (res.status_code==200):
                    try:
                        k=res.json()
                        if ('frontiers' in k):
                            myfrontier=k['frontiers']
                            for m in myfrontier:
                                frontier=myfrontier[m]
                                break
                            if (frontier==""):
                                frontier="0000000000000000000000000000000000000000000000000000000000000000"
                        elif ('error' in k):
                            err=10003
                            errorname="Error: NANO error "+k['error']+" Error no.: "+str(err)
                        else:
                            err=10004
                            errorname="Error: NANO error unknown JSON param. Error no.: "+str(err)
                    except Exception as e:
                        err=10002
                        errorname="Error: Can't parse NANO node JSON "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
                else:
                    err=10001
                    errorname="Error: NANO node status code: "+str(res.status_code)
                if (frontier!=""):
                    ret=fenixiot.set_frontier(None, None, frontier)
                    if (ret==None):
                        msg="CMD_GET_FRONTIER"
                        err=fenixiot.getlasterror()
                        errorname=fenixprotocol.geterrorname(err)
        else:
            err=10005
            errorname="Error: Empty slot 7 fatal error "+str(err)
    elif (command==fenixprotocol.CMD_GET_DPOW):
        hash_str=fenixiot.get_dpow_hash_from_client()
        if (hash_str!=None):
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(dpow_local_srv(data=hash_str))
            except Exception as e:
                err=10000
                errorname="Error: 'dpow_local_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            if (res):
                if 'work' in res:
                    ret=fenixiot.send_dpow(None, None, hash_str, res['work'])
                    if (ret==None):
                        msg="CMD_GET_DPOW"
                        err=fenixiot.getlasterror()
                        errorname=fenixprotocol.geterrorname(err)
                if 'error' in res:
                    msg="ERR_DPOW"
                    err=10030
                    errorname="Error: 'dpow_local_srv' -> "+res['error']
        else:
            msg="get_dpow_hash_from_client() error"
            err=fenixiot.getlasterror()
            errorname=fenixprotocol.geterrorname(err)
    elif (command==fenixprotocol.CMD_GET_REPRESENTATIVE):
        if (hasattr(protocol, "s7")):
            wallet=protocol.s7.decode('ascii').rstrip('\0')
            parm='{"action":"account_representative","account":"'+wallet+'"}'
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(nano_node_srv(data=parm))
            except Exception as e:
                err=10000
                errorname="Error: 'nano_node_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            if (res):
                representative=""
                if (res.status_code==200):
                    try:
                        k=res.json()
                        if ('representative' in k):
                            representative=k['representative']
                        elif ('error' in k):
                            err=10003
                            errorname="Error: NANO error "+k['error']+" Error no.: "+str(err)
                        else:
                            err=10004
                            errorname="Error: NANO error unknown JSON param. Error no.: "+str(err)
                    except Exception as e:
                        err=10002
                        errorname="Error: Can't parse NANO node JSON "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
                else:
                    err=10001
                    errorname="Error: NANO node status code: "+str(res.status_code)
                if (representative!=""):
                    ret=fenixiot.send_representative(None, None, representative)
                    if (ret==None):
                        msg="CMD_GET_REPRESENTATIVE"
                        err=fenixiot.getlasterror()
                        errorname=fenixprotocol.geterrorname(err)
        else:
            err=10005
            errorname="Error: Empty slot 7 fatal error "+str(err)
    elif (command==fenixprotocol.CMD_GET_BLOCK_STATE_FROM_CLIENT):
        ret=fenixiot.get_signed_json_block_from_fenixiot()
        if (ret==None):
            msg="CMD_GET_BLOCK_STATE_FROM_CLIENT"
            err=fenixiot.getlasterror()
            errorname=fenixprotocol.geterrorname(err)
        else:
            loop=asyncio.get_event_loop()
            res=None
            try:
                res=loop.run_until_complete(nano_node_srv(data=ret))
            except Exception as e:
                err=10000
                errorname="Error: 'nano_node_srv' "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
            ret=None
            if (res):
                block_hash=""
                if (res.status_code==200):
                    try:
                        k=res.json()
                        if ('hash' in k):
                            block_hash=k['hash']
                        elif ('error' in k):
                            err=10003
                            errorname="Error: NANO error "+k['error']+" Error no.: "+str(err)
                        else:
                            err=10004
                            errorname="Error: NANO error unknown JSON param. Error no.: "+str(err)
                    except Exception as e:
                        err=10002
                        errorname="Error: Can't parse NANO node JSON "+str(type(e))+" with message: "+str(e)+" Error no.: "+str(err)
                else:
                    err=10001
                    errorname="Error: NANO node status code: "+str(res.status_code)
                if (block_hash!=""):
                    ret=fenixiot.set_block_state(None, None, block_hash)
                    if (ret==None):
                        msg="CMD_GET_BLOCK_STATE_FROM_CLIENT"
                        err=fenixiot.getlasterror()
                        errorname=fenixprotocol.geterrorname(err)
    elif (command==fenixprotocol.CMD_GET_PREF_REPRESENTATIVE):
        if (NANO_PREFERED_REPRESENTATIVE!=""):
            ret=fenixiot.send_preferred_representative(None, None, NANO_PREFERED_REPRESENTATIVE)
            if (ret==None):
                msg="CMD_GET_PREF_REPRESENTATIVE"
                err=fenixiot.getlasterror()
                errorname=fenixprotocol.geterrorname(err)
        else:
            err=10012
            errorname="Error: No preferred representative"
            msg="NANO_PREFERED_REPRESENTATIVE=EMPTY"
    else:
        err=10011
        errorname="Error: Unknown command "+str(err)
        msg="UNKNOWN_COMMAND"
    if (ret):
        client.publish(publish_callback, payload=ret, qos=2, retain=False)
        msg="Success"
    else:
        print("Data not sent to client '"+publish_callback+"' with request "+msg)
        msg="Fail"
        print("Trying to send error reasons to client ...")
        if ((err==fenixprotocol.F_ERR_FORBIDDEN_OVFL_PUBL_STR)or(err==fenixprotocol.F_ERR_FORBIDDEN_NULL_PUB_STR)):
            print("Could not send error to client. Invalid publish callback "+errorname)
        else:
            print("ERROR NAME: "+errorname)
            ret=fenixiot.senderrortoclient(None, err, "Error with name: "+errorname)
            if (ret):
                client.publish(publish_callback, payload=ret, qos=2, retain=False)
            else:
                err=fenixiot.getlasterror()
                print("Sencond fail when sending error reason to client "+str(err)+" Error name: "+fenixprotocol.geterrorname(err))
    print("Finishing with "+msg)

print("Initializing FIOT Python 3 library ...")
fenixiot=fenixprotocol.init(None)
fenixiot.onerror(fenix_onerror)
fenixiot.ondata(fenix_onreceive)
print(fenixprotocol.about())

################# GET PARAMETERS #######################
NANO_NODE_URL="<YOUR_NANO_NODE_HERE>"
NANO_PREFERED_REPRESENTATIVE="nano_3ngt59dc7hbsjd1dum1bw9wbb87mbtuj4qkwcruididsb5rhgdt9zb4w7kb9"

async def nano_node_srv(data):
   global NANO_NODE_URL
   return requests.post(url=NANO_NODE_URL,data=data)

################## LOCAL DPOW TEST######################

# TEST OK. YAY !!!
# It works fine with nano-work-serve (install it if you want a local PoW https://github.com/nanocurrency/nano-work-server)
DPOW_SERVER="[::1]:7076"
DPOW_DIFFICULTY="ffffffc000000000" # Real difficulty (sloooowwww) for I3 Intel Core (16 to 50 seconds)
#DPOW_DIFFICULTY="fffc000000000000" # for testing (fast) for I3 Intel Core (38 to 380 ms)

async def dpow_local_srv(data):
    global DPOW_DIFFICULTY
    http=urllib3.PoolManager()
    parm='{"action":"work_generate","hash":"'+data+'","difficulty":"'+DPOW_DIFFICULTY+'"}'
    try:
        r=http.request('POST', DPOW_SERVER, headers={'Content-Type':'application/json'}, body=parm)
    except Exception as e:
        return {'error':'Error when request encoded data <'+str(type(e))+'> Reason: '+str(e)}
    try:
        res=json.loads(r.data.decode('utf-8'))
    except:
        return {'error':'Error "dpow_local_srv" when load decode data to JSON <'+str(type(e))+'> Reason: '+str(e)}
    if 'work' in res:
        try:
            i=int(res['work'].encode('utf-8'), 16)
            return {'work': i}
        except Exception as e:
            return {'error':'Error "dpow_local_srv" when convert string to int <'+str(type(e))+'> Reason: '+str(e)}
    elif 'error' in res:
        msg_error=res['error']
        if 'hint' in res:
            msg_error+=' HINT: '+res['hint']
        return {'error': msg_error}
    return {'error':'Unknown dpow_local_srv error'}

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

