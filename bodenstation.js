const startaltitude = 30;

const datablockstart = 0xFFFFFF;
const datablockend = 0xFFFFFE;

let rawdatablock = '';
let datablock = [];
let bigwidth = (document.body.offsetWidth - 10) / 1 - 30;
let mediumwidth = (document.body.offsetWidth - 10) / 2 - 30;
let smallwidth = (document.body.offsetWidth - 10) / 3 - 30;
let biggrad = bigwidth / 10;
let mediumgrad = mediumwidth / 5;
let smallgrad = smallwidth / 3;

const calibration = {
    temperature: 0,
    accelerationX: 0,
    accelerationY: 0,
    accelerationZ: 0,
    rotationX: 0,
    rotationY: 0,
    rotationZ: 0
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
    time: [] // Zeit bis zum Datenblock in Sekunden
}

let bmp_altitudes = [];

let sealevelhpa;

let timer;

let startdatablock = 0; // Ab welchem Datenblock die relative Position bestimmt werden soll

let corruptdatablocks = 0; // Anzahl an Fehlerhaften Datenblöcken

// BMP
let bmp_height = 0;

// BNO
let movementX = 0;
let movementY = 0;
let movementZ = 0;

let startheight = undefined;

let lastuseddatablock = -1;

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
                console.log(datablock);
                if (datablock[14] == datablockend) {
                    console.log(`Datenblock ${datablock[1]} empfangen!`);
                    processDatablock(datablock);

                    datablock = [];
                    rawdatablock = '';

                    if (lastuseddatablock > 1) {
                        refreshScreen(data);
                    }
                }
                if (datablock.length > 16) { // Bei zu langen Blöcken neu anfangen
                    rawdatablock = '';
                    console.log('block too long');
                }
		if (datablock[datablock.length - 2] == datablockstart) { // Neuer Datenblock fängt an
                    rawdatablock = datablockstart + '\r\n';
                    console.log('new block');
                    corruptdatablocks++;
                }
            }
        } catch (error) {
            console.error('Es ist ein Fehler aufgetreten: ' + error);
        } finally {
            reader.releaseLock();
        }
    }
}

function processDatablock(datablock, useTimer = true) {
    if (datablock[0] == datablockstart &&
        datablock[14] == datablockend &&
        dataBlockOK(datablock)) {
        data.messages.push(Number(datablock[1]));

        data.temperature.push(Number(datablock[2]) - calibration.temperature);
        data.pressure.push(Number(datablock[3]));

        data.accelerationX.push(Number(datablock[4]) - calibration.accelerationX);
        data.accelerationY.push(Number(datablock[5]) - calibration.accelerationY);
        data.accelerationZ.push(Number(datablock[6]) - calibration.accelerationZ);

        data.rotationX.push(Number(datablock[7]) - calibration.rotationX);
        data.rotationY.push(Number(datablock[8]) - calibration.rotationY);
        data.rotationZ.push(Number(datablock[9]) - calibration.rotationZ);

        data.latitude.push(Number(datablock[10]));
        data.longitude.push(Number(datablock[11]));
        data.altitude.push(Number(datablock[12]));

        let infos = Number(datablock[13]);
        data.fan.push(infos & 1);
        data.ejected.push((infos >> 1) & 1);
        data.landed.push((infos >> 2) & 1);


        if (useTimer) {
            if (timer != undefined) {
                data.time.push((Date.now() - timer) / 1000); // In Sekunden, statt Millisekunden
            } else {
                data.time.push(0);
            }

            timer = Date.now();
        }


        calcRelativePos(data);
    } else {
        corruptdatablocks++
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
        bmp_altitudes = [];

        lastuseddatablock = -1;
    }

    let newmovementX, newmovementY, newmovementZ;

    for (let i = lastuseddatablock + 1; i < data.time.length; i++) {
        let time = data.time[i]; // In Sekunden

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
            sealevelhpa = calcSeaLevelHpa(data.pressure[i], startaltitude);
        }
        bmp_altitudes.push(calcAltitude(data.pressure[i]));

        lastuseddatablock = i;
    }
    bmp_height = bmp_altitudes[bmp_altitudes.length - 1] - startaltitude;
}

async function refreshScreen(data) {
    document.body.innerHTML =
        '<div class="big"><a onclick="exportData()">Daten exportieren</a> | <a onclick="exportRawData()">Roh-Daten exportieren</a></div>' +
        '<div class="medium">Temperatur [°C]: <br>' + generateSVGChart(data.temperature, mediumwidth, 200, data.time, mediumgrad) + '</div>' +
        '<div class="medium">Druck [Pa]: <br>' + generateSVGChart(data.pressure, mediumwidth, 200, data.time, mediumgrad) + '</div>' +
        '<div class="big">Höhenmeter (Luftdruck) [m]: <br>' + generateSVGChart(bmp_altitudes, bigwidth, 300, data.time, biggrad) + '</div>' +
	'<div class="big">Höhenmeter (GPS) [m]: <br>' + generateSVGChart(data.altitude, bigwidth, 300, data.time, biggrad) + '</div>' +
        '<div class="small">Höhe (Luftdruck): <br>' + bmp_height.toFixed(3) + 'm</div>' +
        '<div class="small">Verschiebung (Beschleunigung): <br>' + `X: ${movementX.toFixed(3)}m, Y: ${movementY.toFixed(3)}m, Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Verschiebung (GPS): <br>' + `X: ${movementX.toFixed(3)}m, Y: ${movementY.toFixed(3)}m, Z: ${movementZ.toFixed(3)}m` + '</div>' +
        '<div class="small">Beschleunigung (X-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationX, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="small">Beschleunigung (Y-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationY, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="small">Beschleunigung (Z-Achse) [m/s^2]: <br>' + generateSVGChart(data.accelerationZ, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="small">Rotation (X-Achse): <br>' + generateSVGChart(data.rotationX, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="small">Rotation (Y-Achse): <br>' + generateSVGChart(data.rotationY, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="small">Rotation (Z-Achse): <br>' + generateSVGChart(data.rotationZ, smallwidth, 100, data.time, smallgrad) + '</div>' +
        '<div class="big">' + (data.messages.length - 1) + ' von ' + Math.round(data.messages[data.messages.length - 1]) + ' Datenblöcken empfangen.<br>' + corruptdatablocks + ' Datenblöcke fehlerhaft.<br>Letzter Datenblock wurde mit ' + data.time[data.time.length - 1] + ' Sekunden zum vorherigen empfangen.</div>';
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

let sdraw = '';
let somethingReceived = false; // Es muss etwas empfangen worden sein, bevor lastadd eine Rolle spielt
let lastadd = Date.now();
let rawSDdataProceded = false;
async function importFromSD() {
    setInterval(() => {
        console.log('Interval');
        if (!rawSDdataProceded) {
            document.body.innerHTML = (sdraw.length / 1000).toFixed(3) + 'KB';
        }
        if (importCompleted()) {
            if (!rawSDdataProceded) {
                processSDRawData(sdraw);
            }
        }
    }, 1000);

    port = await navigator.serial.requestPort();
    // await port.open({ baudRate: 9600 });
    await port.open({ baudRate: 115200 });

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

                    const decoder = new TextDecoder();

                    while (true) {
                        const { value, done } = await reader.read();
                        if (done) {
                            alert('Verbindung getrennt!');
                            break;
                        }
                        sdraw += decoder.decode(value);
                        if (!somethingReceived) {
                            if (sdraw.length > 0) {
                                somethingReceived = true;
                            }
                        }
                        lastadd = Date.now();
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

function importCompleted() {
    if (somethingReceived) {
        if (lastadd + 1000 < Date.now()) { // Falls lange keine neuen Daten importiert wurden
            return true;
        }
    }
    return false;
}

function processSDRawData(rawdata) {
    rawSDdataProceded = true; // um erneutes Ausführen zu vermeiden
    console.log('Roh-Daten werden verarbeitet...');
    let bigdatablock = rawdata.split('\r\n');

    bigdatablock.forEach(item => {
        datablock.push(item);
        console.log(datablock[12]);
        if (datablock[14] == datablockend) {
            console.log(`Datenblock ${datablock[1]} empfangen!`);

            if (data.time.length > 0) {
                data.time.push(1234); // Standartwert
            } else {
                data.time.push(0);
            }

            //datablock[14] = datablockend; // Zeit aus Datenblock entfernen, damit wie beim Empfang ausgewertet werden kann
            //delete datablock[15];

            processDatablock(datablock, false);
            datablock = [];
        }
        if (datablock.length > 16) { // Bei zu langen Blöcken neu anfangen
            datablock = [];
	    corruptdatablocks++
        } else if (datablock[datablock.length - 1] == datablockstart) { // Neuer Datenblock fängt an
            datablock = [datablockstart];
        }
    });

    refreshScreen(data);
}