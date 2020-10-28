# ESP-WiCo

Universal wireless relay based on ESP8266. with telegram bot, Saving the last power failure state, web inteface and API.


## How to configure <img src="https://raw.githubusercontent.com/CatEllite312/ESP-WiCo/main/img/conf.png" width="250" align="right" />

At the first start, the device will not know your network parameters. An access point without a password will automatically start after connecting to which you will be redirected to the setup page.

To configure, you need the name of the access point to which you will connect and the encryption key (password). You also need to specify the IP address of the gateway, subnet mask and assign the IP address to the device.

Optionally, you can specify the Telegram API token for the bot to work.


I built a project for NodeMcu v3 and this code will definitely work on devices with ESP12 (e) and most likely ESP7 will work too.

For simple operation, this project does not require any configuration or programming, you need to compile the .ino file and flash the microcontroller.
</br>
## Using the Web Interface <img src="https://raw.githubusercontent.com/CatEllite312/ESP-WiCo/main/img/web.png" width="250" align="left" />

Open your browser to http://IP_address_of_your_device/ and you should be fairly intuitive interface with 4 buttons.
</br></br></br></br></br></br></br></br></br></br>
## Using the API <img src="https://raw.githubusercontent.com/CatEllite312/ESP-WiCo/main/img/sht.png" width="250" align="right" />

The API is RESTful, so you are expected to use GET and POST. Returns JSON.

### HTTP GET to read the relay state

`GET http://IP_address_of_your_device/api/relay1` returns an HTTP 200 and :
```
{
  "state": 1
}
```

1 = Relay is on. 0 = Relay is off.

### HTTP POST to set the relay state.

POST http://IP_address_of_your_device/api/relay2 with a POST form variable called `state` and it's value set to `0` or `1` (on or off).

You'll receive a response like:
```
{
  "state": 1,
  "set": 1
}
```

Where state=1 means the relay is on, and set=1 means you've just set it that way.

### Notes
* If you'd like to simply toggle a relay you can call http://IP_address_of_your_device/relay2/toggle etc.

## Telegram bot <img src="https://raw.githubusercontent.com/CatEllite312/ESP-WiCo/main/img/tg.png" width="250" align="left" />

Telegram bot has inlinekeyboard duplicating the main features.
The bot also understands some commands that can be sent to it through the Telegram schedule.
```
/show - show keyboard
/state - check the state of the relay
/r[XY] - text commands for controlling the relay where X is the relay number (integers from 1 to 4 inclusive), and Y is its state (1 or 0)
```
The /r21 command on relay number 2 will set state 1 (on) and turn it on. The /r40 command on relay number 4 will set the state to 0 (off) and turn it off. etc
</br></br>

## Wiring diagram

<p align="center">
  <img width="600" src="https://raw.githubusercontent.com/CatEllite312/ESP-WiCo/main/img/wiring.png">
</p>
