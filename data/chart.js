var myConfig = {
    backgroundColor: "#fff",
    globals: {
        fontFamily: "Raleway",
        color: "#666"
    },
    graphset: [{
        type: "line",
        // adjustScale: true,
        plotarea: {
            'adjust-layout': true /* For automatic margin adjustment. */
        },
        title: {
            text: "Live Chart Meter",
            adjustLayout: true,
            "media-rules": [{
                "max-width": 650,
                "fontSize": 14
            }]
        },
        x: 0,
        y: 0,
        width: "100%",
        height: "100%",
        "media-rules": [{
            "max-width": 650,
            "x": 0,
            "y": 0,
            "width": '100%',
            "height": "100%",
        }],
        scaleX: {
            minValue: Math.floor(new Date().getTime()), // 1373045400000
            step: 1500, //số giây
            transform: {
                type: "date",
                all: "%d-%m-20%y<br>%D<br>%H:%i:%s"
            }
        },
        "scale-y": {
            values: "0:4000:100",
            placement: "default",
            lineColor: "#FB301E",
            tick: {
                lineColor: "#FB301E"
            },
            item: {
                fontColor: "#FB301E",
                fontSize: "12",
                bold: true
            }
        },
        "scale-y-2": {
            values: "0:30:5",
            placement: "1",
            lineColor: "#E2D51A",
            tick: {
                lineColor: "#E2D51A"
            },
            item: {
                fontColor: "#E2D51A",
                fontSize: "12",
                bold: true
            }
        },
        "scale-y-3": {
            values: "200:260:25",
            placement: "2",
            lineColor: "#00AE4D",
            tick: {
                lineColor: "#00AE4D",
            },
            item: {
                fontColor: "#00AE4D",
                fontSize: "12",
                bold: true
            }
        },
        plotarea: {
            margin: "dynamic",
            marginRight: "4%"
        },
        crosshairX: {
            lineColor: "#23211E",
            scaleLabel: {
                backgroundColor: "#E3DEDA",
                fontColor: "#414042"
            },
            plotLabel: {
                backgroundColor: "#f0ece8",
                fontColor: "#414042",
                borderWidth: 1,
                borderColor: "#000"
            }
        },
        tooltip: {
            visible: false
        },
        series: [{
            values: [0],
            lineColor: "#00AE4D",
            text: "Voltage (V)",
            scales: "scale-x, scale-y-3",
            marker: {
                borderWidth: 2,
                borderColor: "#00AE4D",
                backgroundColor: "#fff",
                type: "circle"
            }
        }, {
            values: [0],
            lineColor: "#E2D51A",
            text: "Current (A)",
            scales: "scale-x, scale-y-2",
            marker: {
                borderWidth: 2,
                borderColor: "#E2D51A",
                backgroundColor: "#fff",
                type: "triangle",
                size: 5
            }
        }, {
            values: [0],
            lineColor: "#FB301E",
            text: "Power (W)",
            scales: "scale-x, scale-y",
            marker: {
                borderWidth: 2,
                borderColor: "#FB301E",
                backgroundColor: "#fff",
                type: "square"
            }
        }]
    }]
};


zingchart.render({
    id: 'myChart',
    data: myConfig,
    height: "100%",
    width: '100%'
});

/*
 * SetInterval is used to simulate live input. We also have
 * a feed attribute that takes in http requests, websockets,
 * and return value from a JS function.
 */
setInterval(function() {
    var colors = ['#00AE4D', '#E2D51A', '#FB301E'];
    var Marker = function(bgColor, ceiling) {
        return {
            type: "area",
            range: [0, ceiling],
            backgroundColor: bgColor
        }
    };
    // var randomOffset0 = [-5, 5, 3, -3, 2, -2];
    // var randomOffset1 = [10, -10, -5, 5, 3, -3, 2, -2, 7, -7];
    var output0 = Math.ceil(voltageVar); //Math.ceil(33 + randomOffset0[Math.floor(Math.random() * 6)]); // voltage
    var output1 = ampeVar; //Math.ceil(11 + randomOffset0[Math.floor(Math.random() * 6)]); // current
    var output2 = Math.ceil(powerVar); //Math.ceil(22 + randomOffset1[Math.floor(Math.random() * 9)]); // power

    // 3) update line graph
    zingchart.exec('myChart', 'appendseriesvalues', {
        graphid: 0,
        update: false,
        values: [
            [output0],
            [output1],
            [output2]
        ]
    });

    // batch update all chart modifications
    zingchart.exec('myChart', 'update');

}, 1500);