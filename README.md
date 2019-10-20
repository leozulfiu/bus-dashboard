# Bus Dashboard

A dashboard showing the latest bus departures from configured stations in switzerland.

## Hardware

- any ESP8266 microcontroller
- [4.3inch e-Paper UART Module](https://waveshare.com/wiki/4.3inch_e-Paper_UART_Module)

## APIs

- [fahrplan.search.ch](https://fahrplan.search.ch/api/help): This API is where most of the departure data is fetched. Because a response is quite big the ESP8266 can't handle that size and I created an API gateway in-between which strips off most the data and returns only the departures: [Example](http://bananas.cloud.tyk.io/transport?from=Zürich%20Limmatplatz&to=Zürich%20Bellvue&num=3&show_delays=1)
- [worldtimeapi.org](http://worldtimeapi.org): I could have used the NT protocol but didn't want to handle localization and especially correcting the shown time with regards summer and winter time ([DST](https://timeanddate.com/time/dst/))

## Todo

- [ ] Show when the bus actually arrives ("Arrives in x minutes...")
- [ ] Max retries of wlan connection attempts
- [ ] Show basic weather infos
- [ ] Decide if the deep sleep mode should be used or a latching power switch circuit

## License

[MIT](https://choosealicense.com/licenses/mit/)

