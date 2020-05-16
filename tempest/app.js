const dgram = require('dgram')
const server = dgram.createSocket('udp4')
const { logEvent, cToF, mbarToHg } = require('./utils.js')

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
        ['batv', obs[16]],
      ], obs[0]);
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
