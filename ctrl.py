import logging

import paho.mqtt.client as mqtt

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883

### DONT EDIT BELO ###
LOG = logging.getLogger()


LIGHTS = {}


def on_connect(client, userdata, flags, c):
    LOG.info("Connected!")
    client.subscribe('/control')


def on_message(client, userdata, msg):
    command, id_ = msg.payload.split(",")

    LIGHTS.setdefault(id_, False) # Assume lights off

    if command == "switch":
        newstate = not LIGHTS[id_]

        msg = "%s,%s" % (id_, int(newstate)) # Format is "<id>,<state>"
        LOG.debug("Setting state %s", msg)
        client.publish("/lights", msg)
        LIGHTS[id_] = newstate

if __name__ == '__main__':
    logging.basicConfig(level='DEBUG')
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_HOST, MQTT_PORT, 60)
    client.loop_forever()
