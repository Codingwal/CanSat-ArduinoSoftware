let writer;
async function start() {
    const port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });

    document.body.innerHTML = 'Wird gestartet...';
    
    while (port.readable) {
        const reader = port.readable.getReader();
        writer = port.writable.getWriter();
        console.log(setFrequency(new Uint8Array(434.0)));
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    alert('Verbindung getrennt!')
                    break;
                }

                // if (available()) {
                    console.log(RH_RF95_MAX_MESSAGE_LEN);
                    let buf = RH_RF95_MAX_MESSAGE_LEN;
                    let len = sizeof(buf);
                    if (recv(buf, len)) {
                        let data = buf;
                        console.log(data);
                    }
                // }
            }
        } catch (error) {
            console.error('Es ist ein Fehler aufgetreten: ' + error);
        } finally {
            reader.releaseLock();
        }
    }
}

function write(reg, data) {
    writer.write(new Uint8Array([reg, data]));
}