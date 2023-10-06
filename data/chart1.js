jQuery(document).ready(function($) {
    const gauge1 = $("#circularGaugeContainer").dxCircularGauge({ //gauge 1 : Voltage
        rangeContainer: {
            offset: 3,
            ranges: [
                { startValue: 215, endValue: 235, color: '#2DD700' },
                { startValue: 235, endValue: 250, color: '#fc0422' }
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
            text: 'ĐIỆN ÁP',
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
        value: 220,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('voltage_value').innerHTML = voltageVar + ' V';
    //=========================GAUGE 2: AMPE=========================
    const gauge2 = $("#circularGaugeContainer1").dxCircularGauge({
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
            text: 'DÒNG ĐIỆN',
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
        value: 5,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('current_value').innerHTML = ampeVar + ' A';
    //===============================GAUGE 3: POWER==============================
    const gauge3 = $("#circularGaugeContainer2").dxCircularGauge({
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
            text: 'CÔNG SUẤT',
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
        value: 2200,
        // subvalues: [220]
    }).dxCircularGauge('instance');
    // document.getElementById('power_value').innerHTML = powerVar + ' W';

    setInterval(function() {

        gauge1.option('value', voltageVar);
        gauge2.option('value', ampeVar);
        gauge3.option('value', powerVar);
    }, 1500); // Gọi lại hàm sau mỗi 2 giây

});