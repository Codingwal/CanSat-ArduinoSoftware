function generateSVGChart(values, width, height, weight) {
        let minAndMax = getMinAndMax(values);
        let minValue = minAndMax[0];
        let maxValue = minAndMax[1];
        let range = maxValue - minValue; // muss mindestens eins sein, um Anzeigefehler zu vermeiden

        if (range == 0) {
            range = 0.01;
        }

        // console.log(minAndMax);
        // console.log(range);

        let scaleX = (width - weight) / (values.length - 1);
        let scaleY = (height - weight) / range;

        
        let toaddX = weight / 2;
        let toaddY = toaddX - minValue * scaleY;

        console.log([values[2], scaleY, toaddY]);
        console.log(values[2] * scaleY + toaddY);
        
        let svg = `<path d="M${toaddX} ${height - (values[0] * scaleY + toaddY)}`;
        for (let i = 1; i < values.length; i++) {
            svg += `L${i * scaleX + toaddX} ${height - (values[i] * scaleY + toaddY)}`;
        }

        svg += `" fill="none" stroke="currentColor" stroke-width="${weight}" stroke-linejoin="round" stroke-linecap="round"></path>`;

        svg += `<text x="5" font-size="15" y="${height}">${minValue}</text>`;
        svg += `<text x="5" font-size="15" y="15">${maxValue}</text>`;

        return `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg">${svg}</svg>`;
    }

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