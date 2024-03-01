function generateSVGChart(values, width, height, xsteps, weight = 2.5, gapY = 100 / 3, gapX = 100 / 1, gradweight = 1) {
    let minAndMax = getMinAndMax(values);
    let minValue = minAndMax[0];
    let maxValue = minAndMax[1];
    let range = maxValue - minValue;

    let xwidth = 0;

    xsteps.forEach(val => {
        xwidth += val;
    });

    if (range == 0) { // darf nicht 0 sein, um Anzeigefehler zu vermeiden
        range = 0.00001;
    }

    // let scaleX = (width - weight) / (values.length - 1);
    let scaleY = (height - weight) / range;


    let toaddX = weight / 2;
    let toaddY = toaddX - minValue * scaleY;

    let x = 0;
    let svg = `<path d="M${toaddX} ${height - (values[0] * scaleY + toaddY)}`;
    for (let i = 1; i < values.length; i++) {
        x += xsteps[i];
        svg += `L${(x / xwidth) * (width - weight) + toaddX} ${height - (values[i] * scaleY + toaddY)}`;
    }

    svg += `" fill="none" stroke="currentColor" stroke-width="${weight}" stroke-linejoin="round" stroke-linecap="round"></path>`;

    // Gradierung hinzufügen
    for (let i = 1; i <= width / gapX; i++) {
        svg += `<path style="z-index: -1" d="M${i * gapX} 0 L ${i * gapX} ${height}" stroke-width="${i == 0 || i == width / gapX ? gradweight * 2 : gradweight}" stroke="#808080"></path>`;
        svg += `<text x="${i + 1 > width / gapX ? width - 34 /* 34 Pixel ist der Text lang*/ : i * gapX}" font-size="12" y="${height}">${((i / (width / gapX) * xwidth) / 60).toFixed(3)}</text>`; // Von Sekunden in Minuten, nur für CanSat
    }

    for (let i = 0; i <= height / gapY; i++) {
        svg += `<path style="z-index: -1" d="M0 ${i * gapY} L ${width} ${i * gapY}" stroke-width="${i == 0 || i == height / gapY ? gradweight * 2 : gradweight}" stroke="#808080"></path>`;
        svg += `<text x="5" font-size="12" y="${i == 0 ? '12' : i * gapY}">${(Number(maxValue) - range / (height / gapY) * i).toFixed(3)}</text>`;
    }

    return `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg">${svg}</svg>`;
}

/*function generateSVGChart(values, width, height, weight = 2.5, gapY = 100 / 3, gradweight = 1) {
    let minAndMax = getMinAndMax(values);
    let minValue = minAndMax[0];
    let maxValue = minAndMax[1];
    let range = maxValue - minValue;

    if (range == 0) { // darf nicht 0 sein, um Anzeigefehler zu vermeiden
        range = 0.00001;
    }

    let scaleX = (width - weight) / (values.length - 1);
    let scaleY = (height - weight) / range;


    let toaddX = weight / 2;
    let toaddY = toaddX - minValue * scaleY;

    let svg = `<path d="M${toaddX} ${height - (values[0] * scaleY + toaddY)}`;
    for (let i = 1; i < values.length; i++) {
        svg += `L${i * scaleX + toaddX} ${height - (values[i] * scaleY + toaddY)}`;
    }

    svg += `" fill="none" stroke="currentColor" stroke-width="${weight}" stroke-linejoin="round" stroke-linecap="round"></path>`;

    // Gradierung hinzufügen
    // svg += `<text x="5" font-size="12" y="${height}">${Number(minValue).toFixed(3)}</text>`;

    console.log(height / gapY);
    for (let i = 0; i <= height / gapY; i++) {
        console.log();
        svg += `<path style="z-index: -1" d="M0 ${i * gapY} L ${width} ${i * gapY}" stroke-width="${i == 0 || i == height / gapY ? gradweight * 2 : gradweight}" stroke="#808080"></path>`;
        svg += `<text x="5" font-size="12" y="${i == 0 ? '12' : i * gapY}">${(Number(maxValue) - range / (height / gapY) * i).toFixed(3)}</text>`;
    }

    // svg += `<text x="5" font-size="12" y="12">${Number(maxValue).toFixed(3)}</text>`;

    return `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg">${svg}</svg>`;
}*/

function getMinAndMax(values) {
    let minValue = values[0];
    let maxValue = values[0];

    for (let i = 1; i < values.length; i++) {
        if (values[i] < minValue) {
            minValue = values[i];
        }
        if (values[i] > maxValue) {
            maxValue = values[i];
        }
    }

    return [minValue, maxValue];
}

function generateGraph(x, y, z) {
    let output = [];
    let value = y;

    for (let i = 0; i < x; i++) {
        value = value + (Math.random() * 1 - z);
        output.push(value);
    }

    return output;
}

function generatePressures(x) {
    let output = [];
    let value = 0;

    for (let i = 0; i < x; i++) {
        value = Math.random() * 10 - Math.sin((i / 10));
        output.push(value);
    }

    return output;
}