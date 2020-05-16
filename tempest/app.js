const WebSocket = require('ws')
const { logEvent, cToF } = require('./utils.js')

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

const apiKey = process.env.WF_API_KEY;
const deviceId = process.env.DEVICE_ID;
// websocket url copied from https://smartweather.weatherflow.com/station/18160
const wsUrl = `wss://swd.weatherflow.com/swd/data?api_key=${apiKey}&app=web&ver=20200220a&browser=Chrome,81`;
const ws = new WebSocket(wsUrl);

ws.onopen = function () {
  // Copied from the connection starting sequence at:
  // https://smartweather.weatherflow.com/station/18160
  ws.send(`{"type":"listen_start","device_id":${deviceId}}`);
  ws.send(`{"type":"listen_rapid_start","device_id":${deviceId}}`);
  ws.send(`{"type":"listen_start_events","station_id":null}`);
};

ws.onmessage = function(e) {
  // format/filter/forward to influx here
  if (e && e.data) {
    try {
      /* some messages as of current writing:
        {"status":{"status_code":0,"status_message":"SUCCESS"},"device_id":62760,"type":"obs_st","source":"cache","summary":{"pressure_trend":"steady","strike_count_3h":0,"precip_total_1h":0.0,"feels_like":14.0,"heat_index":14.0,"wind_chill":14.0},"obs":[[1589606574,0,0,0,0,3,1011.3,14,75,5,0,0,0,0,0,0,2.57,1,0.204846,null,null,0]]}
        {"device_id":62760,"serial_number":"ST-00002262","type":"rapid_wind","hub_sn":"HB-00018394","ob":[1589606594,0.00,0]}
        {"summary":{"pressure_trend":"steady","strike_count_3h":0,"precip_total_1h":0.0,"feels_like":13.9,"heat_index":13.9,"wind_chill":13.9},"serial_number":"ST-00002262","hub_sn":"HB-00018394","type":"obs_st","source":"mqtt","obs":[[1589606634,0.0,0.0,0.0,0,3,1011.3,13.9,75,5,0.0,0,0.0,0,0,0,2.57,1,0.204846,null,null,0]],"device_id":62760,"firmware_revision":126}
      */
      const data = JSON.parse(e.data);
      if (data.summary && data.obs.length) {
        const observed = data.obs[0];
        // unix timestamp in pos 0
        // temp C in pos 7
        // Humidty % in pos 8
        logEvent('weather', [
          ['deviceid', `tempest${deviceId}`],
          ['location', 'outdoor'],
        ], [
          ['tempf_01', cToF(observed[7])],
          ['hum_01', observed[8]],
        ], observed[0]);
      }
    } catch (error) {
      console.error(error);
    }
  }
};
