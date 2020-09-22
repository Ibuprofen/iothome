const dgram = require('dgram')
const server = dgram.createSocket('udp4')
const { logEvent, cToF, mbarToHg, kmToMiles, mmToIn } = require('./utils.js')

const deviceId = process.env.DEVICE_ID;

process.on('SIGINT', () => {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" )
  process.exit(1)
})

server.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on('message', (msg, rinfo) => {

  try {
    const data = JSON.parse(msg);

    // general observation data
    if (data.type === 'obs_st') {
      // UDP values at https://weatherflow.github.io/SmartWeather/api/udp/v119/
      const obs = data.obs[0];
      logEvent('weather', [
        ['deviceid', `tempest${deviceId}`],
        ['location', 'outdoor'],
      ], [
        ['presinhg', mbarToHg(obs[6])],
        ['tempf_01', cToF(obs[7])],
        ['hum_01', obs[8]],
        ['illum', obs[9]],
        ['uv', obs[10]],
        ['solrad', obs[11]],
        ['precipaccum', mmToIn(obs[12])],
        ['ltgavgdist', kmToMiles(obs[14])],
        ['ltgcnt', obs[15]],
        ['batv', obs[16]],
      ], obs[0]);
    }

    // rain start event
    if (data.type === 'evt_precip') {
      const evt = data.evt;
      logEvent('weather', [
        ['deviceid', `tempest${deviceId}`],
        ['location', 'outdoor'],
      ], [
        ['precipstart', 1],
      ], evt[0]);
    }

    // lightning
    if (data.type === 'evt_strike') {
      const evt = data.evt;
      logEvent('weather', [
        ['deviceid', `tempest${deviceId}`],
        ['location', 'outdoor'],
      ], [
        ['ltgdist', kmToMiles(evt[1])],
        ['ltgenergy', evt[2]],
      ], evt[0]);
    }

    // device status
    if (data.type === 'device_status') {
      logEvent('weather', [
        ['deviceid', `tempest${deviceId}`],
        ['location', 'outdoor'],
      ], [
        ['rssi', data.rssi],
        ['hub_rssi', data.hub_rssi],
        ['uptime', data.uptime],
      ], data.timestamp);
    }
  } catch (error) {
    console.error(error);
  }

});

server.on('listening', () => {
  const address = server.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(process.env.UDP_PORT);
