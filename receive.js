let rawdatablock = '';
let datablock = [];
let bigwidth = document.body.offsetWidth - 40;
let smallwidth = (document.body.offsetWidth - 10) / 3 - 30;

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
    sended: [],
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
                if (datablock[17] == 9999991) {
                    console.log(`Datenblock ${datablock[16]} empfangen!`);
                    rawdatablock = '';

                    if (datablock[0] == 9999990 && // Überprüfen, ob alle Daten empfangen wurden
                        // datablock[4] == 9999991 &&
                        // datablock[8] == 9999992 &&
                        // datablock[12] == 9999993 &&
                        // datablock[16] == 9999994 &&
                        // datablock[20] == 9999995 &&
                        datablock[17] == 9999991) {

                        data.temperature.push(datablock[1]); // Alle Daten hinzufügen
                        data.pressure.push(datablock[2]);
                        data.altitude2.push(datablock[3]);

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
                        data.ready.push(datablock[14]);
                        data.landed.push(datablock[15]);

                        data.sended.push(datablock[16]);

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
        '<div class="big">Temperatur: <br>' + generateSVGChart(data.temperature, bigwidth, 100, 5) + '</div>' +
        '<div class="big">Druck: <br>' + generateSVGChart(data.pressure, bigwidth, 100, 5) + '</div>' +
        '<div class="big">Höhenmeter (laut BMP): <br>' + generateSVGChart(data.altitude2, bigwidth, 100, 5) + '</div>' +
        '<div class="medium">Verschiebung (relativ zum Startpunkt): <br>' + `X: ${movementX.toFixed(3)}m<br>Y: ${movementY.toFixed(3)}m<br>Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="medium">Verschiebung (relativ zum Startpunkt): <br>' + `X: ${movementX.toFixed(3)}m<br>Y: ${movementY.toFixed(3)}m<br>Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Beschleunigung (X-Achse): <br>' + generateSVGChart(data.accelerationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Y-Achse): <br>' + generateSVGChart(data.accelerationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Beschleunigung (Z-Achse): <br>' + generateSVGChart(data.accelerationZ, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (X-Achse): <br>' + generateSVGChart(data.rotationX, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Y-Achse): <br>' + generateSVGChart(data.rotationY, smallwidth, 200, 5) + '</div>' +
        '<div class="small">Rotation (Z-Achse): <br>' + generateSVGChart(data.rotationZ, smallwidth, 200, 5) + '</div>' +
        '<div class="big">' + data.sended.length + ' von ' + Math.round(data.sended[data.sended.length - 1]) + ' Datenblöcken empfangen.<br>Letzter Datenblock wurde mit ' + data.time[data.time.length - 1] + ' ms zum vorherigen empfangen.</div>';
}