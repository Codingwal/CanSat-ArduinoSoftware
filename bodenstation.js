let rawdatablock = '';
let datablock = [];
let bigwidth = document.body.offsetWidth - 40;
let smallwidth = (document.body.offsetWidth - 10) / 3 - 30;

let data = {
    temperature: [],
    pressure: [],
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
    messages: [],
    time: [] // Zeit bis zum Datenblock
}

let timer;

let startdatablock = 0; // Ab welchem Datenblock die relative Position bestimmt werden soll

// BNO
let movementX = 0;
let movementY = 0;
let movementZ = 0;

// BMP
let height = 0;

let startheight = undefined;

let lastuseddatablock = 0;

async function start() {
    const port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });

    document.body.innerHTML = 'Warten auf Signal...';

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
                if (datablock[16] == 9999991) {
                    // console.log(`Datenblock ${datablock[1]} empfangen!`);
                    rawdatablock = '';

                    if (datablock[0] == 9999990 &&
                        datablock[16] == 9999991 &&
                        dataBlockOK(datablock)) {
                        data.messages.push(datablock[1]);

                        data.temperature.push(datablock[2]);
                        data.pressure.push(datablock[3]);

                        data.accelerationX.push(datablock[4]);
                        data.accelerationY.push(datablock[5]);
                        data.accelerationZ.push(datablock[6]);

                        data.rotationX.push(datablock[7]);
                        data.rotationY.push(datablock[8]);
                        data.rotationZ.push(datablock[9]);

                        data.latitude.push(datablock[10]);
                        data.longitude.push(datablock[11]);
                        data.altitude.push(datablock[12]);

                        data.fan.push(datablock[13]);
                        data.landed.push(datablock[14]);

                        if (timer != undefined) {
                            data.time.push(Date.now() - timer);
                        } else {
                            data.time.push(0);
                        }

                        timer = Date.now();

                        calcRelativePos(data);

                        refreshScreen(data);
                    } else {
                        console.log('Fehlerhafter Datenblock!');
                    }
                }
                if (lastitem == 9999990) { // Neuer Datenblock fängt an
                    rawdatablock = '9999990\r\n';
                }
                if (datablock.length > 20) { // Bei zu langen Blöcken neu anfangen
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

function dataBlockOK(datablock) {
    datablock.forEach(item => {
        if (item == NaN || item == undefined || item == null) {
            return false;
        }
    });
    return true;
}

async function calcRelativePos(data, recalc = false) {
    if (recalc) {
        // BNO
        movementX = 0;
        movementY = 0;
        movementZ = 0;

        // BMP
        height = 0;

        lastuseddatablock = 0;

        startheight = undefined;
    }

    let newmovementX, newmovementY, newmovementZ;

    for (let i = lastuseddatablock + 1; i < data.time.length; i++) {
        let time = (data.time[i] / 1000);

        newmovementX = 0.5 * data.accelerationX[i] * Math.pow(time, 2);
        newmovementY = 0.5 * data.accelerationY[i] * Math.pow(time, 2);
        newmovementZ = 0.5 * data.accelerationZ[i] * Math.pow(time, 2);

        if (newmovementX > 0) {
            movementX += newmovementX;
        } else {
            movementX -= newmovementX;
        }
        if (newmovementY > 0) {
            movementY += newmovementY;
        } else {
            movementY -= newmovementY;
        } if (newmovementZ > 0) {
            movementZ += newmovementZ;
        } else {
            movementZ -= newmovementZ;
        }

        if (startheight == undefined) {
            //startheight = height;
        }

        lastuseddatablock = i;
    }
}

async function refreshScreen(data) {
    document.body.innerHTML =
        '<div class="big">Temperatur [°C]: <br>' + generateSVGChart(data.temperature, bigwidth, 100, 5) + '</div>' +
        '<div class="big">Druck [Pa]: <br>' + generateSVGChart(data.pressure, bigwidth, 100, 5) + '</div>' +
        // '<div class="big">Höhenmeter (Luftdruck) [m]: <br>' + generateSVGChart(data.altitude2, bigwidth, 100, 5) + '</div>' +
        '<div class="small">Verschiebung (Luftdruck) [m]: <br>' + `Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Verschiebung (Beschleunigung) [m]: <br>' + `X: ${movementX.toFixed(3)}m<br>Y: ${movementY.toFixed(3)}m<br>Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Verschiebung (GPS) [m]: <br>' + `X: ${movementX.toFixed(3)}m<br>Y: ${movementY.toFixed(3)}m<br>Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Beschleunigung (X-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Y-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Z-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationZ, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (X-Achse): <br>' + generateSVGChart(data.rotationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Y-Achse): <br>' + generateSVGChart(data.rotationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Z-Achse): <br>' + generateSVGChart(data.rotationZ, smallwidth, 200, 5) + '</div>' +
        '<div class="big">' + data.messages.length + ' von ' + Math.round(data.messages[data.messages.length - 1]) + ' Datenblöcken empfangen.<br>Letzter Datenblock wurde mit ' + data.time[data.time.length - 1] + ' ms zum vorherigen empfangen.</div>';
}