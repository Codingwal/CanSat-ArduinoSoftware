async function start() {
    // Prompt user to select any serial port.
    const port = await navigator.serial.requestPort();

    // Wait for the serial port to open.
    await port.open({ baudRate: 9600 });

    console.log(port.getInfo());

    // Frequenz einstellen
    const writer = port.writable.getWriter();

    const data = new Uint8Array([104, 101, 108, 108, 111]); // hello
    await writer.write(data);

    // Allow the serial port to be closed later.
    writer.releaseLock();


    while (port.readable) {
        const reader = port.readable.getReader();

        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    // Allow the serial port to be closed later.
                    reader.releaseLock();
                    break;
                }
                if (value) {
                    console.log(value);
                }
            }
        } catch (error) {
            // TODO: Handle non-fatal read error.
        }
    }
}