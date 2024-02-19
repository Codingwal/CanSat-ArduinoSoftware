let rawdatablock = '';
let datablock = [];
let bigwidth = document.body.offsetWidth - 40;
let smallwidth = (document.body.offsetWidth - 40) / 3 - 30;

let data = {
    temperature: [],
    pressure: [],
    altitude2: [],
    accelerationX: [],
    accelerationY: [],
    accelerationZ: [],
    rotationX: [],
    rotationY: [],
    rotationZ: [],
    latitude: [],
    longitude: [],
    altitude: [],
    fan: [],
    ready: [],
    landed: [],
    sended: []
}

async function start() {
    const port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });

    document.body.innerHTML = 'Wird gestartet...';

    while (port.readable) {
        const reader = port.readable.getReader();
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    alert('Verbindung getrennt!')
                    break;
                }
                let lastitem = new TextDecoder().decode(value);
                rawdatablock += lastitem;
                datablock = rawdatablock.split('\r\n');
                // console.log(datablock);
                if (datablock[22] == 9999996) {
                    console.log(`Datenblock ${datablock[21]} empfangen!`);
                    rawdatablock = '';

                    if (datablock[0] == 9999990 && // Überprüfen, ob alle Daten empfangen wurden
                        datablock[4] == 9999991 &&
                        datablock[8] == 9999992 &&
                        datablock[12] == 9999993 &&
                        datablock[16] == 9999994 &&
                        datablock[20] == 9999995 &&
                        datablock[22] == 9999996) {

                        data.temperature.push(datablock[1]); // Alle Daten hinzufügen
                        data.pressure.push(datablock[2]);
                        data.altitude2.push(datablock[3]);

                        data.accelerationX.push(datablock[5]);
                        data.accelerationY.push(datablock[6]);
                        data.accelerationZ.push(datablock[7]);

                        data.rotationX.push(datablock[9]);
                        data.rotationY.push(datablock[10]);
                        data.rotationZ.push(datablock[11]);

                        data.latitude.push(datablock[13]);
                        data.longitude.push(datablock[14]);
                        data.altitude.push(datablock[15]);

                        data.fan.push(datablock[17]);
                        data.ready.push(datablock[18]);
                        data.landed.push(datablock[19]);

                        data.sended.push(datablock[21]);

                        refreshScreen();
                    } else {
                        console.log('Fehlerhafter Datenblock!');
                    }
                }
                if (lastitem == 9999990) { // Neuer Datenblock fängt an
                    rawdatablock = '9999990\r\n';
                }
                if (datablock.length > 30) {
                    rawdatablock = '';
                }
            }
        } catch (error) {
            console.error('Es ist ein Fehler aufgetreten: ' + error);
        } finally {
            reader.releaseLock();
        }
    }
}

async function refreshScreen() {
    document.body.innerHTML =
        '<div class="big">Temperatur: <br>' + generateSVGChart(data.temperature, bigwidth, 100, 5) + '</div>' +
        '<div class="big">Druck: <br>' + generateSVGChart(data.pressure, bigwidth, 100, 5) + '</div>' +
        '<div class="big">Höhenmeter (laut BMP): <br>' + generateSVGChart(data.altitude2, bigwidth, 100, 5) + '</div>' +
        '<div class="small">Beschleunigung (X-Achse): <br>' + generateSVGChart(data.accelerationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Y-Achse): <br>' + generateSVGChart(data.accelerationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Z-Achse): <br>' + generateSVGChart(data.accelerationZ, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (X-Achse): <br>' + generateSVGChart(data.rotationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Y-Achse): <br>' + generateSVGChart(data.rotationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Z-Achse): <br>' + generateSVGChart(data.rotationZ, smallwidth, 200, 5) + '</div>';
}