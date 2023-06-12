jQuery(document).ready(function($) {
    $("#circularGaugeContainer").dxCircularGauge({ //gauge 1 : Voltage
        rangeContainer: {
            offset: 3,
            ranges: [
                { startValue: 220, endValue: 230, color: '#2DD700' },
                { startValue: 230, endValue: 250, color: '#fc0422' }
            ]
        },
        scale: {
            startValue: 100,
            endValue: 250,
            majorTick: { tickInterval: 50 },
            label: {
                format: ''
            }
        },
        title: {
            text: 'Voltage',
            subtitle: 'Unit: Volt (V)',
            position: 'top-center'
        },
        tooltip: {
            enabled: true,
            format: '',
            customizeText: function(arg) {
                return 'Current ' + arg.valueText;
            }
        },
        subvalueIndicator: {
            type: 'textCloud',
            format: 'thousands',
            text: {
                format: '',
                customizeText: function(arg) {
                    return 'NORMAL ' + arg.valueText + 'V';
                }
            }
        },
        value: voltageVar,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('voltage_value').innerHTML = voltageVar + ' V';
    //=========================GAUGE 2: AMPE=========================
    $("#circularGaugeContainer1").dxCircularGauge({
        rangeContainer: {
            offset: 3,
            ranges: [
                { startValue: 0, endValue: 15, color: '#41A128' },
                { startValue: 15, endValue: 20, color: '#ffa500' },
                { startValue: 20, endValue: 30, color: '#fc0422' }
            ]
        },
        scale: {
            startValue: 0,
            endValue: 30,
            majorTick: { tickInterval: 10 },
            label: {
                format: ''
            }
        },
        title: {
            text: 'Current',
            subtitle: 'Unit: Ampe (A)',
            position: 'top-center'
        },
        tooltip: {
            enabled: true,
            format: '',
            customizeText: function(arg) {
                return 'Current ' + arg.valueText;
            }
        },
        subvalueIndicator: {
            type: 'textCloud',
            format: 'thousands',
            text: {
                format: '',
                customizeText: function(arg) {
                    return 'NORMAL ' + arg.valueText + 'V';
                }
            }
        },
        value: ampeVar,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('current_value').innerHTML = ampeVar + ' A';
    //===============================GAUGE 3: POWER==============================
    $("#circularGaugeContainer2").dxCircularGauge({
        rangeContainer: {
            offset: 3,
            ranges: [
                { startValue: 0, endValue: 2200, color: '#41A128' },
                { startValue: 2200, endValue: 3000, color: '#ffa500' },
                { startValue: 3000, endValue: 4000, color: '#fc0422' }
            ]
        },
        scale: {
            startValue: 0,
            endValue: 4000,
            majorTick: { tickInterval: 10 },
            label: {
                format: ''
            }
        },
        title: {
            text: 'Power',
            subtitle: 'Unit: Watt (W)',
            position: 'top-center'
        },
        tooltip: {
            enabled: true,
            format: '',
            customizeText: function(arg) {
                return 'Current ' + arg.valueText;
            }
        },
        subvalueIndicator: {
            type: 'textCloud',
            format: 'thousands',
            text: {
                format: '',
                customizeText: function(arg) {
                    return 'NORMAL ' + arg.valueText + 'W';
                }
            }
        },
        value: powerVar,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('power_value').innerHTML = powerVar + ' W';

    setInterval(function() {
        $("#circularGaugeContainer").dxCircularGauge({});
        document.getElementById('voltage_value').innerHTML = voltageVar + ' V';
        $("#circularGaugeContainer1").dxCircularGauge({});
        document.getElementById('current_value').innerHTML = ampeVar + ' A';
        $("#circularGaugeContainer2").dxCircularGauge({});
        document.getElementById('power_value').innerHTML = powerVar + ' W';
    }, 1500); // Gọi lại hàm sau mỗi 2 giây


});

// function showTime() {
//     var date = new Date();
//     var h = date.getHours(); // 0 - 23
//     var m = date.getMinutes(); // 0 - 59
//     var s = date.getSeconds(); // 0 - 59
//     var session = "AM";


//     var months = [
//         'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'
//     ];

//     var days = [
//         'Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'
//     ];

//     var dayOfWeek = days[date.getDay()];
//     var month = months[date.getMonth()];
//     var day = date.getDate();
//     var year = date.getFullYear();

//     var dateString = dayOfWeek + ', ' + month + ' ' + day + ', ' + year;

//     if (h == 0) {
//         h = 12;
//     }

//     if (h > 12) {
//         h = h - 12;
//         session = "PM";
//     }

//     h = (h < 10) ? "0" + h : h;
//     m = (m < 10) ? "0" + m : m;
//     s = (s < 10) ? "0" + s : s;


//     var time = h + ":" + m + ":" + s + " " + session;
//     document.getElementById("MyClockDisplay").innerText = time;
//     document.getElementById("date").textContent = dateString;

//     setTimeout(showTime, 1000);

// }



// showTime();