// https://randomnerdtutorials.com/esp32-web-server-gauges/     giao tiếp với esp32
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
const btnAll = {
    on: `<img src="./img/onpower.png" alt="MAIN POWER ON" width="100px" height="100px">
    <h4>ON ALL</h4>`,
    off: `<img src="./img/offpower.png" alt="MAIN POWER OFF" width="100px" height="100px">
    <h4>OFF ALL</h4>`,
    value: true
}
const socket1 = {
    on: `<img src="./img/socket1.png" alt="SOCKET 1 ON" width="100px" height="100px">
    <h4>SOCKET 1: ON</h4>`,
    off: `<img src="./img/socket.png" alt="SOCKET 1 OFF" width="100px" height="100px">
    <h4>SOCKET 1: OFF</h4>`,
    value: false
}
const socket2 = {
    on: `<img src="./img/socket2.png" alt="SOCKET 2 ON" width="100px" height="100px">
    <h4>SOCKET 2: ON</h4>`,
    off: `<img src="./img/socket.png" alt="SOCKET 2 OFF" width="100px" height="100px">
    <h4>SOCKET 2: OFF</h4>`,
    value: false
}
const socket3 = {
        on: `<img src="./img/socket3.png" alt="SOCKET 3 ON" width="100px" height="100px">
    <h4>SOCKET 3: ON</h4>`,
        off: `<img src="./img/socket.png" alt="SOCKET 3 OFF" width="100px" height="100px">
    <h4>SOCKET 3: OFF</h4>`,
        value: false
    }
    //============Biến của Gauge=================
var voltageVar; // biến của điện áp
var ampeVar; // biến của dòng điện
var powerVar; // biến của công suất tức thời

const button = document.getElementById('all_btn'); // main power button
const button1 = document.getElementById('first_btn'); // socket button 1
const button2 = document.getElementById('second_btn'); // socket button 2
const button3 = document.getElementById('thirst_btn'); // socket button 3
function getStateButton() {
    websocket.send("getBtn");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection....');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getStateButton();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function onMessage(event) { // phản hồi được trạng thái nút nhấn sang các client
    console.log(event.data);
    var myObj = JSON.parse(event.data);

    // Kiểm tra trường thông điệp
    if (myObj.Type === 'button') {
        // Xử lý dữ liệu JSON thứ nhất
        btnAll.value = myObj.MainButton === "1" ? true : false;
        socket1.value = myObj.Button1 === "1" ? true : false;
        socket2.value = myObj.Button2 === "1" ? true : false;
        socket3.value = myObj.Button3 === "1" ? true : false;
        //callbackBtn(button, btnAll);     /// vướng chỗ đổi trạng thái của button main
        stateBtn(button, btnAll, btnAll.value);
        stateBtn(button1, socket1, socket1.value);
        stateBtn(button2, socket2, socket2.value);
        stateBtn(button3, socket3, socket3.value);
    } else if (myObj.Type === 'data') {
        // Xử lý dữ liệu JSON thứ hai
        voltageVar = parseFloat(myObj.Volt);
        ampeVar = parseFloat(myObj.Current);
        powerVar = parseFloat(myObj.Power);
        document.getElementById('voltage_value').innerHTML = voltageVar + ' V';
        document.getElementById('current_value').innerHTML = ampeVar + ' A';
        document.getElementById('power_value').innerHTML = powerVar + ' W';
        document.getElementById('energy_value').innerHTML = myObj.Energy + ' kWh';
        document.getElementById('freq_value').innerHTML = myObj.Freq + ' Hz';
        document.getElementById('pfactor_value').innerHTML = myObj.Pf;

    }



    //var myObj = JSON.parse(event.data);
    // btnAll.value = myObj.MainButton === "1" ? true : false;
    // socket1.value = myObj.Button1 === "1" ? true : false;
    // socket2.value = myObj.Button2 === "1" ? true : false;
    // socket3.value = myObj.Button3 === "1" ? true : false;
    // //callbackBtn(button, btnAll);     /// vướng chỗ đổi trạng thái của button main
    // stateBtn(button, btnAll, btnAll.value);
    // stateBtn(button1, socket1, socket1.value);
    // stateBtn(button2, socket2, socket2.value);
    // stateBtn(button3, socket3, socket3.value);


    // var state;
    // if (event.data == "000") {
    //     state = "ON MAIN";
    //     btnAll.value = true;
    //     callbackBtn(button, btnAll);
    //     stateBtn(button1, socket1, true);
    //     stateBtn(button2, socket2, true);
    //     stateBtn(button3, socket3, true);
    //     // btnAll.value = true;
    // } else if (event.data == "001") {
    //     state = "OFF MAIN";
    //     btnAll.value = false;
    //     callbackBtn(button, btnAll);
    //     stateBtn(button1, socket1, false);
    //     stateBtn(button2, socket2, false);
    //     stateBtn(button3, socket3, false);
    //     // btnAll.value = false;// lỗi
    // } else if (event.data == "010") {
    //     stateBtn(button1, socket1, true);
    //     state = "on 1";

    // } else if (event.data == "011") {
    //     stateBtn(button1, socket1, false);
    //     state = "OFF 1";

    // } else if (event.data == "100") {
    //     state = "on 2";
    //     stateBtn(button2, socket2, true);

    // } else if (event.data == "101") {
    //     state = "OFF 2";
    //     stateBtn(button2, socket2, false);

    // } else if (event.data == "110") {
    //     state = "on 3";
    //     stateBtn(button3, socket3, true);

    // } else if (event.data == "111") {
    //     state = "OFF 3";
    //     stateBtn(button3, socket3, false);

    // }

} /*Hàm này và hàm ws.textAll là một cặp truyền nhận xử lý tin nhắn */

// ===================CALLBACK BTN=================
const callbackBtn = (buttonElement, button) => {
        button.value = !button.value;
        if (button.value) {
            buttonElement.innerHTML = button.on
        } else {
            buttonElement.innerHTML = button.off
        }
    }
    // ============CALLBACK BTN END=====================
const stateBtn = (buttonElement, button, state) => {
    if (state) {
        buttonElement.innerHTML = button.on
            //button.value = state;
    } else {
        buttonElement.innerHTML = button.off
            //button.value = state;
    }
}

callbackBtn(button, btnAll);
callbackBtn(button1, socket1);
callbackBtn(button2, socket2);
callbackBtn(button3, socket3);

// button.addEventListener("click", () => callbackBtn(button, btnAll));

//when press main power btn =>>>
button.addEventListener("click", FnButtonAll);
button1.addEventListener("click", FnButton1);
button2.addEventListener("click", FnButton2);
button3.addEventListener("click", FnButton3);

function FnButtonAll() {
    callbackBtn(button, btnAll);
    if (btnAll.value == true) {
        websocket.send('offall');
    } else {
        websocket.send('onall');
    }
}

function FnButton1() {
    callbackBtn(button1, socket1);
    if (socket1.value == true) {
        websocket.send('onsk1');
    } else {
        websocket.send('offsk1');
    }
}

function FnButton2() {
    callbackBtn(button2, socket2);
    if (socket2.value == true) {
        websocket.send('onsk2');
    } else {
        websocket.send('offsk2');
    }
}

function FnButton3() {
    callbackBtn(button3, socket3);
    if (socket3.value == true) {
        websocket.send('onsk3');
    } else {
        websocket.send('offsk3');
    }
}

// setInterval(function() {
//     voltageVar = Math.floor(Math.random() * 220) + 1; // biến của điện áp
//     ampeVar = Math.floor(Math.random() * 30) + 1;; // biến của dòng điện
//     powerVar = Math.floor(Math.random() * 3200) + 1;; // biến của công suất tức thời

// }, 1500);
function showTime() {
    var date = new Date();
    var h = date.getHours(); // 0 - 23
    var m = date.getMinutes(); // 0 - 59
    var s = date.getSeconds(); // 0 - 59
    var session = "AM";


    var months = [
        'January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'
    ];

    var days = [
        'Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'
    ];

    var dayOfWeek = days[date.getDay()];
    var month = months[date.getMonth()];
    var day = date.getDate();
    var year = date.getFullYear();

    var dateString = dayOfWeek + ', ' + month + ' ' + day + ', ' + year;

    if (h == 0) {
        h = 12;
    }

    if (h > 12) {
        h = h - 12;
        session = "PM";
    }

    h = (h < 10) ? "0" + h : h;
    m = (m < 10) ? "0" + m : m;
    s = (s < 10) ? "0" + s : s;


    var time = h + ":" + m + ":" + s + " " + session;
    document.getElementById("MyClockDisplay").innerText = time;
    document.getElementById("date").textContent = dateString;

    setTimeout(showTime, 1000);

}

showTime();