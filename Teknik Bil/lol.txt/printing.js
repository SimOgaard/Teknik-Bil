
var ConnectBtn = document.getElementById('Robot');
var Status = document.getElementById('status');
var StartBtn = document.getElementById('Start');
var send = document.getElementById("Send")
var log = document.getElementById("message");
var oliver = document.getElementById("item2")
var simon = document.getElementById("item")
var Values = [document.getElementById("1"), document.getElementById("2"),document.getElementById("3"),document.getElementById("4"),document.getElementById("5"),document.getElementById("6"),document.getElementById("7"),document.getElementById("9")]; //,document.getElementById("10"),document.getElementById("8")
var time = 0; time_oliver =0;
var charts =['O','S'];
// var andelS = 0;
// var andelO = 0;


window.onload = function(){
    startConnect();
    var param = JSON.parse(window.localStorage.getItem('param'));
    if(param != null){
        Values[0].value = param["Interval"]
        Values[1].value = param["Integration"]
        Values[2].value = param["Proportionell"]
        Values[3].value = param["Derivative"]
        Values[4].value = param["PubFrequency"]
        Values[5].value = param["RequestedRPM"]
        Values[6].value = param["Dist"]
//        Values[7].value = param["ProportionellSafety"] //
        Values[7].value = param["Task"] // 7
//        Values[9].value = param["DataSetSize"] //
    }
    for(var pos =0;pos<2;pos++){
    var ctx = document.getElementById(charts[pos]+'Chart');
    charts[pos] = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Rpm',
            data: [],
            backgroundColor: [
                'rgba(0, 255, 255, 0.4)'
            ],
            borderColor: [
                'rgba(0, 255, 255, 1)'
            ],
            borderWidth: 1
        }
        // {
        //     label: 'Kp',
        //     data: [],
        //     backgroundColor: [
        //         'rgba(0, 255, 0, 0.4)'
        //     ],
        //     borderColor: [
        //         'rgba(0, 255, 0, 1)'
        //     ],
        //     borderWidth: 1,
        //     fill: true
        // },{
        //     label: 'Kd',
        //     data: [],
        //     backgroundColor: [
        //         'rgba(0, 255, 255, 0.4)'
        //     ],
        //     borderColor: [
        //         'rgba(0, 255, 255, 1)'
        //     ],
        //     borderWidth: 1
        // }
        ,{label: 'bör värde',
        data: [],
        backgroundColor: [
            'rgba(0, 102, 255, 0.4)'
        ],
        borderColor: [
            'rgba(0, 102, 255, 1)'
        ],
        borderWidth: 1,
        fill:false}]
    },
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true
                }
            }]
        }
    }
});}
}
/*payload = [Owner, Task, Interval, Integration, Proportionell, Easing, ProportionellSafety, RequestedRPM, MqttSlow, Dist]*/
send.onclick = function(e){
    e.preventDefault();
    if(Values[7].value == "S"){
        time=0;
        clear_graph()
    }if(Values[7].value == "Shrek"){new Audio('./node_modules/shrek.mp3').play()}
    data ={"V":["A",Values[7].value,Values[0].value,Values[1].value,Values[2].value,Values[3].value,Values[4].value,Values[5].value,Values[6].value]}//Values[7].value,Values[9].value
    console.log(memorySizeOf(JSON.stringify(data)))
    pub(data)
}
document.getElementById("Save").onclick = function(e){
    if(Values[0].value != ""){
        data ={ 
            "Owner": "All",
            "Interval" :Values[0].value,
            "Integration" : Values[1].value,
            "Proportionell" : Values[2].value,
            "Derivative" : Values[3].value,
            "PubFrequency" : Values[4].value,
            "RequestedRPM" : Values[5].value,
            "Dist": Values[6].value,
            //"ProportionellSafety":Values[7].value,
            "Task": Values[7].value,
            //"DataSetSize": Values[9].value
        }
    console.log("Saved")
    window.localStorage.setItem('param', JSON.stringify(data));
    }
}
ConnectBtn.onclick = function(e){
    e.preventDefault();
    if(ConnectBtn.innerHTML== "Connect Client"){
        startConnect();}
    else{
        startDisconnect();}
    return false;
};
function graph_data(chart,x,pos){
    console.log(pos)
    charts[chart].data.datasets[pos].data.push(x);
    
}
function clear_graph(){
    for(var x=0;x<1;x++){
        console.log(x)
        console.log(charts[x].data.labels)
        charts[x].data.labels=[0]
        charts[x].data.datasets.forEach(clear);
        charts[x].update()
    }
}
function clear(value,index,array){value.data=[];console.log(value)}
function startConnect() {
    clientID = "clientID_" + parseInt(Math.random() * 100);
    host = 'maqiatto.com';
    port = 8883;
    client = new Paho.MQTT.Client(host, Number(port), clientID);
    client.onConnectionlost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    client.connect({userName : "oliver.witzelhelmin@abbindustrigymnasium.se",password : "vroom",
        onSuccess: onConnect,
        onFailure: onFail,
    });
}
function startDisconnect(){
    ConnectBtn.innerHTML= "Connect Client";
    Status.innerHTML = "Disconnected";
    message = new Paho.MQTT.Message(JSON.stringify(clientID));
    message.destinationName = "offline";
    client.send(message);
    client.disconnect();
    console.log("disconnected")
}
function pub(dataobjekt){
    message = new Paho.MQTT.Message(JSON.stringify(dataobjekt));
    message.destinationName = "oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroomO";// ger medelandet en destination
    if (Status.innerHTML =="Connected"){
    console.log(message)
    client.send(message);//skickar iväg medelandet
    message = new Paho.MQTT.Message(JSON.stringify(dataobjekt));
    message.destinationName = "oliver.witzelhelmin@abbindustrigymnasium.se/vroom_vroomS";
    client.send(message)
    }else{
        log.value = "Can't publish while disconnected \n"
    }}
function onFail() {
    ConnectBtn.innerHTML= "Connect Client";
    Status.innerHTML = "Disconnected";
    console.log('<span>ERROR: Connection to: ' + host + ' on port: ' + port + ' failed.</span><br/>');// indikerar om man inte kan connecta
    log.value = "Eroor could not establish a connection to the broker"}

function onConnectionLost(responseObject){
    ConnectBtn.innerHTML= "Connect Client";
    Status.innerHTML = "Disconnected";
    console.log('<span>ERROR: ' + host + ' on port: ' + port + ' has been shut down.</span><br/>');
    log.value = "unnexpected broker failure"
    if (responseObject.errorCode !== 0) {
        Status.innerHTML += '<span>ERROR: ' + + responseObject.errorMessage + '</span><br/>';
    }}  
// Called when the client connects
function onConnect() {
    ConnectBtn.innerHTML= "Disconnect Client";
    Status.innerHTML="Connected";
    topic = 'oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomS';
    console.log('<span>Subscribing to: ' + topic + '</span><br/>');
    client.subscribe(topic);
    client.subscribe("oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomO")
    message = new Paho.MQTT.Message("Hello World 2");
    message.destinationName = "oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomS";
    client.send(message);
    message = new Paho.MQTT.Message("Hello World 2");
    message.destinationName = "oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomS";
    client.send(message);
    log.value = "Succsesfully conected to broker \n"
}
function onMessageArrived(message) {
    var context = message.payloadString;
    console.log(message.destinationName)
    if(message.destinationName == "oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomO"){
        if(context[0]=="{"){            
            console.log("lol "+"{"+context+"}")
            var json = JSON.parse(context);
            console.log(json); //[RPM, Proportionell, Integration, Derivering, Time]
            line = "RPM:    "+json["O"][0]+" Proportionell:    "+json["O"][1]+" Integration:    "+json["O"][2]+" Derivering:    "+json["O"][3]+" Time:    "+json["O"][4];
            oliver.value += line+"\n";
            if(time_oliver ==0){time_oliver = json["O"][4]}
            // andelO = json["O"][0]/(json["O"][1]+json["O"][2]+json["O"][3])
            charts[0].data.labels.push(json["O"][4]-time_oliver);
            graph_data(0,json["O"][0],0);
            // graph_data(0,json["O"][2],1);
            // graph_data(0,json["O"][2]+json["O"][3],2);
            charts[0].data.datasets[3].data.push(Values[5].value)
            charts[0].update()
        }else{oliver.value+=context+"\n"}
    }
    if(message.destinationName = "oliver.witzelhelmin@abbindustrigymnasium.se/broom_broomS"){
        if(context[0]=="{"){            
            var json = JSON.parse(context);
            console.log(json);
            line = "RPM:    "+json["S"][0]+" Proportionell:    "+json["S"][1]+" Integration:    "+json["S"][2]+" Derivering:    "+json["S"][3]+" Time:    "+json["S"][4];
            simon.value += line+"\n";  
            if(time ==0){time = json["S"][4]}
            // andelS = parseInt(json["S"][0],10)/(parseInt(json["S"][1],10)+parseInt(json["S"][2],10)+parseInt(json["S"][3],10))
            y= json["S"][4]-time
            graph_data(1,json["S"][0],0);
            console.log(andelS);
            // graph_data(1,(andelS*parseInt(json["S"][1],10),10),0);
            // graph_data(1,(andelS*parseInt(json["S"][2],10)),1);
            // graph_data(1,(andelS*parseInt(json["S"][3],10)),2);
            // graph_data(1,(andelS*json["S"][2]),json["S"][4]-time,1);
            // graph_data(1,(andelS*json["S"][2]+andelS*json["S"][3]),json["S"][4]-time,2);
            // charts[1].data.datasets[3].data.push(Values[5].value)
            charts[1].data.datasets[3].data.push(Values[5].value)
            charts[1].data.labels.push(y);
            charts[1].update()
        }else{simon.value+=context+"\n"}}
    
    console.log("message arrived:"+context)
    }
    function memorySizeOf(obj) {
        var bytes = 0;
    
        function sizeOf(obj) {
            if(obj !== null && obj !== undefined) {
                switch(typeof obj) {
                case 'number':
                    bytes += 8;
                    break;
                case 'string':
                    bytes += obj.length * 2;
                    break;
                case 'boolean':
                    bytes += 4;
                    break;
                case 'object':
                    var objClass = Object.prototype.toString.call(obj).slice(8, -1);
                    if(objClass === 'Object' || objClass === 'Array') {
                        for(var key in obj) {
                            if(!obj.hasOwnProperty(key)) continue;
                            sizeOf(obj[key]);
                        }
                    } else bytes += obj.toString().length * 2;
                    break;
                }
            }
            return bytes;
        };
    
        function formatByteSize(bytes) {
            if(bytes < 1024) return bytes + " bytes";
            else if(bytes < 1048576) return(bytes / 1024).toFixed(3) + " KiB";
            else if(bytes < 1073741824) return(bytes / 1048576).toFixed(3) + " MiB";
            else return(bytes / 1073741824).toFixed(3) + " GiB";
        };
    
        return formatByteSize(sizeOf(obj));
    };
