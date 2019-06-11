var net = require('net');
let count = 0;
net.createServer(function (socket) {
    socket.write('You got something to say?\n');
    count+=1;


    socket.on('data', function (data) {
        socket.write(data.toString().trim());
        console.log("request",count,data);
        socket.destroy()
    });
    socket.on("end",()=>console.log("disconnect"))
}).listen(8080);
console.log('Server running at 127.0.0.1 on port 8080');