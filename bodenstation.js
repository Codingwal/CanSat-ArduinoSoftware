const startaltitude = 30;

let rawdatablock = '';
let datablock = [];
let bigwidth = (document.body.offsetWidth - 10) / 1 - 30;
let mediumwidth = (document.body.offsetWidth - 10) / 2 - 30;
let smallwidth = (document.body.offsetWidth - 10) / 3 - 30;
let lineweight = 2.5;

const calibration = {
    temperature: 0,
    accelerationX: 0,
    accelerationY: 0,
    accelerationZ: 0
}

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
    ejected: [],
    landed: [],
    messages: [],
    time: [] // Zeit bis zum Datenblock
}

let bmp_altitude = [];

let sealevelhpa;

let timer;

let startdatablock = 0; // Ab welchem Datenblock die relative Position bestimmt werden soll

// BMP
let bmp_height = 0;

// BNO
let movementX = 0;
let movementY = 0;
let movementZ = 0;

let startheight = undefined;

let lastuseddatablock = 0;

let rawdata = '';

let port;

async function start() {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });

    document.body.innerHTML = 'Warten auf Signal...';

    readPort();
}

async function readPort() {
    while (port.readable) {
        const reader = port.readable.getReader();
        const decoder = new TextDecoder();
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    alert('Verbindung getrennt!')
                    break;
                }
                let lastitem = decoder.decode(value);
                rawdatablock += lastitem;
                rawdata += lastitem;
                datablock = rawdatablock.split('\r\n');
                console.log(datablock[datablock.length - 1]);
                if (datablock[14] == 9999991) {
                    console.log(`Datenblock ${datablock[1]} empfangen!`);
                    rawdatablock = '';

                    if (datablock[0] == 9999990 &&
                        datablock[14] == 9999991 &&
                        dataBlockOK(datablock)) {
                        data.messages.push(datablock[1]);

                        data.temperature.push(datablock[2] - calibration.temperature);
                        data.pressure.push(datablock[3]);

                        data.accelerationX.push(datablock[4] - calibration.accelerationX);
                        data.accelerationY.push(datablock[5] - calibration.accelerationY);
                        data.accelerationZ.push(datablock[6] - calibration.accelerationZ);

                        data.rotationX.push(datablock[7]);
                        data.rotationY.push(datablock[8]);
                        data.rotationZ.push(datablock[9]);

                        data.latitude.push(datablock[10]);
                        data.longitude.push(datablock[11]);
                        data.altitude.push(datablock[12]);

                        let infos = datablock[13];
                        data.fan.push(infos & 1);
                        data.ejected.push((infos >> 1) & 1);
                        data.landed.push((infos >> 2) & 1);

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
                if (datablock.length > 15) { // Bei zu langen Blöcken neu anfangen
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

function calcSeaLevelHpa(pressure, altitude) {
    return pressure / Math.pow(1 - (altitude / 44330.0), 5.255);
}

function calcAltitude(pressure) {
    // https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.cpp [Zeile 321] oder im Heft Lernen mit ARDUINO!
    return 44300 * (1 - (Math.pow(pressure / sealevelhpa, 0.1903)));
}

async function calcRelativePos(data, recalc = false) {
    if (recalc) {
        // BNO
        movementX = 0;
        movementY = 0;
        movementZ = 0;

        // BMP
        sealevelhpa = undefined;
        bmp_altitude = [];

        lastuseddatablock = 0;
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

        if (sealevelhpa == undefined) {
            sealevelhpa = calcSeaLevelHpa(data.pressure[startdatablock], startaltitude);
        }
        bmp_altitude.push(calcAltitude(data.pressure[i]));

        lastuseddatablock = i;
    }
    bmp_height = bmp_altitude[bmp_altitude.length - 1] - startaltitude;
}

async function refreshScreen(data) {
    document.body.innerHTML =
        '<div class="big"><a onclick="exportData()">Daten exportieren</a> | <a onclick="exportRawData()">Roh-Daten exportieren</a></div>' +
        '<div class="medium">Temperatur [°C]: <br>' + generateSVGChart(data.temperature, mediumwidth, 100, lineweight) + '</div>' +
        '<div class="medium">Druck [Pa]: <br>' + generateSVGChart(data.pressure, mediumwidth, 100, lineweight) + '</div>' +
        '<div class="big">Höhenmeter (Luftdruck) [m]: <br>' + generateSVGChart(bmp_altitude, bigwidth, 100, lineweight) + '</div>' +
        '<div class="small">Höhe (Luftdruck): <br>' + bmp_height.toFixed(3) + 'm</div>' +
        '<div class="small">Verschiebung (Beschleunigung): <br>' + `X: ${movementX.toFixed(3)}m, Y: ${movementY.toFixed(3)}m, Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Verschiebung (GPS): <br>' + `X: ${movementX.toFixed(3)}m, Y: ${movementY.toFixed(3)}m, Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Beschleunigung (X-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationX, smallwidth, 100, lineweight) + '</div>' +
        '<div class="small">Beschleunigung (Y-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationY, smallwidth, 100, lineweight) + '</div>' +
        '<div class="small">Beschleunigung (Z-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationZ, smallwidth, 100, lineweight) + '</div>' +
        '<div class="small">Rotation (X-Achse): <br>' + generateSVGChart(data.rotationX, smallwidth, 100, lineweight) + '</div>' +
        '<div class="small">Rotation (Y-Achse): <br>' + generateSVGChart(data.rotationY, smallwidth, 100, lineweight) + '</div>' +
        '<div class="small">Rotation (Z-Achse): <br>' + generateSVGChart(data.rotationZ, smallwidth, 100, lineweight) + '</div>' +
        '<div class="big">' + (data.messages.length - 1) + ' von ' + Math.round(data.messages[data.messages.length - 1]) + ' Datenblöcken empfangen.<br>Letzter Datenblock wurde mit ' + data.time[data.time.length - 1] + ' ms zum vorherigen empfangen.</div>';
}

function exportRawData() {
    let link = document.createElement('a');
    link.href = 'data:text/plain,' + encodeURIComponent(rawdata);
    link.download = 'rawdata.canz';

    document.body.append(link);
    link.click();
    link.remove();

    refreshScreen();
}

function exportData() {
    let link = document.createElement('a');
    link.href = 'data:text/json,' + encodeURIComponent(JSON.stringify(data));
    link.download = 'data.canz';

    document.body.append(link);
    link.click();
    link.remove(link);

    refreshScreen();
}

async function importFromSD() {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });

    let text = '';
    let loop = true;

    while (port.readable) {
        const reader = port.readable.getReader();
        try {
            while (loop) {
                const decoder = new TextDecoder();
                const { value, done } = await reader.read();
                if (done) {
                    alert('Verbindung getrennt!')
                    break;
                }

                text += decoder.decode(value);
                if (text.search('open') >= 0) {
                    loop = false;
                    let filename = prompt(text);
                    console.log(filename);
                    let writer = await port.writable.getWriter();
                    writer.write(new TextEncoder().encode(filename));
                    console.log('Dateinamen gesendet!');

                    console.log('Port wird freigeschaltet...');

                    let raw = '';
                    const decoder = new TextDecoder();
                    while (true) {
                        const { value, done } = await reader.read();
                        if (done) {
                            alert('Verbindung getrennt!');
                            break;
                        }
                        raw += decoder.decode(value);
                        processRawData(raw);
                    }
                }
            }
        } catch (error) {
            console.error('Es ist ein Fehler aufgetreten: ' + error);
        } finally {
            reader.releaseLock();
        }
    }
}

function processRawData(rawdata) {
    let newdata = rawdata.split('\r\n');
    console.log(newdata);

    newdata.forEach(item => {
        datablock.push(item);

        if (datablock[14] == 9999991) {
            console.log(`Datenblock ${newdata[1]} empfangen!`);

            if (datablock[0] == 9999990 &&
                datablock[14] == 9999991 &&
                dataBlockOK(datablock)) {
                data.messages.push(datablock[1]);

                data.temperature.push(datablock[2] - calibration.temperature);
                data.pressure.push(datablock[3]);

                data.accelerationX.push(datablock[4] - calibration.accelerationX);
                data.accelerationY.push(datablock[5] - calibration.accelerationY);
                data.accelerationZ.push(datablock[6] - calibration.accelerationZ);

                data.rotationX.push(datablock[7]);
                data.rotationY.push(datablock[8]);
                data.rotationZ.push(datablock[9]);

                data.latitude.push(datablock[10]);
                data.longitude.push(datablock[11]);
                data.altitude.push(datablock[12]);

                let infos = datablock[13];
                data.fan.push(infos & 1);
                data.ejected.push((infos >> 1) & 1);
                data.landed.push((infos >> 2) & 1);

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

        if (item == 9999990) {
            datablock = [9999990];
        }
    });
}