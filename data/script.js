// https://randomnerdtutorials.com/esp32-web-server-gauges/     giao tiếp với esp32
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
const btnAll = {
    on: `<img src="./img/onpower.png" alt="MAIN POWER ON" width="100px" height="100px">
    <h4>BẬT HẾT</h4>`,
    off: `<img src="./img/offpower.png" alt="MAIN POWER OFF" width="100px" height="100px">
    <h4>TẮT HẾT</h4>`,
    value: false
}
const socket1 = {
    on: `<img src="./img/socket1.png" alt="SOCKET 1 ON" width="100px" height="100px">
    <h4>HẠT ĐIỆN 1: BẬT</h4>`,
    off: `<img src="./img/socket.png" alt="SOCKET 1 OFF" width="100px" height="100px">
    <h4>HẠT ĐIỆN 1: TẮT</h4>`,
    value: true
}
const socket2 = {
    on: `<img src="./img/socket2.png" alt="SOCKET 2 ON" width="100px" height="100px">
    <h4>HẠT ĐIỆN 2: BẬT</h4>`,
    off: `<img src="./img/socket.png" alt="SOCKET 2 OFF" width="100px" height="100px">
    <h4>HẠT ĐIỆN 2: TẮT</h4>`,
    value: true
}
const socket3 = {
        on: `<img src="./img/socket3.png" alt="SOCKET 3 ON" width="100px" height="100px">
    <h4>HẠT ĐIỆN 3: BẬT</h4>`,
        off: `<img src="./img/socket.png" alt="SOCKET 3 OFF" width="100px" height="100px">
    <h4>HẠT ĐIỆN 3: TẮT</h4>`,
        value: true
    }
    //============Biến của Gauge=================
var voltageVar; // biến của điện áp
var ampeVar; // biến của dòng điện
var powerVar; // biến của công suất tức thời
var energyVar; // số điện 
const elecPrice = [1728,1786,2074,2612,2919,3015,2535];

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
        energyVar = parseFloat(myObj.Energy);
        document.getElementById('voltage_value').innerHTML = voltageVar + ' V';
        document.getElementById('current_value').innerHTML = ampeVar + ' A';
        document.getElementById('power_value').innerHTML = powerVar + ' W';
        document.getElementById('energy_value').innerHTML = myObj.Energy + ' kWh';
        document.getElementById('freq_value').innerHTML = myObj.Freq + ' Hz';
        document.getElementById('pfactor_value').innerHTML = myObj.Pf;

    }

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
function noticePrice() {
    if (energyVar>=0 && energyVar<=50) {
        document.getElementById('lv_elec_price').innerHTML = "<span style='color: green;'>Mức giá 1</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[0] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML = elecPrice[0] * energyVar + " VNĐ";
        document.getElementById("alert_lv_elec_price").innerHTML = "<h1 style='color: green;'>!!VẪN CỨ LÀ OKE, BẠN ĐANG Ở MỨC GIÁ 1!!</h1>";
    }else if (energyVar>=51 && energyVar<=100){
        document.getElementById('lv_elec_price').innerHTML = "<span style='color: green;'>Mức giá 2</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[1] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML = (energyVar -50) * elecPrice[1] + 50 * elecPrice[0]+ " VNĐ";
        document.getElementById("alert_lv_elec_price").innerHTML ="<h1 style='color: green;'>!!Ơ @@ MỨC GIÁ 2 RỒI NÀY!!</h1>";
    }else if (energyVar>=101 && energyVar<=200){
        document.getElementById('lv_elec_price').innerHTML ="<span style='color: orange;'>Mức giá 3</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[2] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML = 50*elecPrice[0] + 50*elecPrice[1] + (energyVar - 100)*elecPrice[2]+ " VNĐ";
        document.getElementById("alert_lv_elec_price").innerHTML = "<h1 style='color: orange;'>!!ĐÃ DÙNG TỚI MỨC GIÁ 3 - CHÚ Ý TIẾT KIỆM NĂNG LƯỢNG!!</h1>";
    }else if (energyVar>=201 && energyVar<=300){
        document.getElementById('lv_elec_price').innerHTML = "<span style='color: red;'>Mức giá 4</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[3] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML = 50*elecPrice[0] + 50*elecPrice[1] + 100*elecPrice[2] + (energyVar - 200)*elecPrice[3]+ " VNĐ";
        document.getElementById("alert_lv_elec_price").innerHTML = "<h1 style='color: red;'>!!ĐÃ DÙNG TỚI MỨC GIÁ 4 - CHÚ Ý HƠN VIỆC TIẾT KIỆM NĂNG LƯỢNG!!</h1>";
    }else if (energyVar>=301 && energyVar<=400){
        document.getElementById('lv_elec_price').innerHTML = "<span style='color: red;'>Mức giá 5</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[4] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML = 50*elecPrice[0] + 50*elecPrice[1] + 100*elecPrice[2] + 100*elecPrice[3] + (energyVar - 300)*elecPrice[4]+ " VNĐ";
        document.getElementById("alert_lv_elec_price").innerHTML = "<h1 style='color: red;'>!!ĐÃ DÙNG TỚI MỨC GIÁ 5 - CẦN THIẾT TIẾT KIỆM NĂNG LƯỢNG!!</h1>";
    }else if (energyVar>=401){
        document.getElementById('lv_elec_price').innerHTML = "<span style='color: red;'>Mức giá 6</span>";
        document.getElementById('value_elec_price').innerHTML = elecPrice[5] + " VNĐ";
        document.getElementById('sum_elec_price').innerHTML =50*elecPrice[0] + 50*elecPrice[1] + 100*elecPrice[2] + 100*elecPrice[3] + 100*elecPrice[4] + (energyVar - 400)*elecPrice[5]+ " VNĐ"; 
        document.getElementById("alert_lv_elec_price").innerHTML = "<h1 style='color: red;'>!!MỨC GIÁ 6 - KỊCH KHUNG RỒI!!</h1>";
    }
}

function showTime() {
    var date = new Date();
    var h = date.getHours(); // 0 - 23
    var m = date.getMinutes(); // 0 - 59
    var s = date.getSeconds(); // 0 - 59
    var session = "AM";


    var months = [
        'Tháng 1', 'Tháng 2', 'Tháng 3', 'Tháng 4', 'Tháng 5', 'Tháng 6', 'Tháng 7', 'Tháng 8', 'Tháng 9', 'Tháng 10', 'Tháng 11', 'Tháng 12'
    ];

    var days = [
        'Chủ nhật', 'Thứ Hai', 'Thứ Ba', 'Thứ Tư', 'Thứ Năm', 'Thứ Sáu', 'Thứ Bảy'
    ];

    var dayOfWeek = days[date.getDay()];
    var month = months[date.getMonth()];
    var day = date.getDate();
    var year = date.getFullYear();

    var dateString = dayOfWeek + ', ' + day + ' ' + month + ', ' + year;

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
