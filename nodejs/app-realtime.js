/**
 * Alexis Giraudet 2015
 */

var interval = 200; // ms
var distance = 8; // px
var sensibility = Math.PI/10; // rad

var app = require('http').createServer(handler)
var io = require('socket.io')(app);
var fs = require('fs');
var players_data = {};
var players_uuid = {};
var increment = 0;
var timer = null;

app.listen(8080);

function handler (req, res) {
  fs.readFile(__dirname + "/index-realtime.html",
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end("Error loading index.html");
    }

    res.writeHead(200);
    res.end(data);
  });
}

io.on("connection", function (socket) {
  console.log(socket.id+": connection");
  players_uuid[socket.id] = increment;
  increment++;
  players_data[players_uuid[socket.id]] = {x: 400, y: 300, angle: 0};

  if(timer == null) {
    timer = setInterval(function () {
      var id;
      for(id in players_data) {
        players_data[id].x += Math.cos(players_data[id].angle)*distance;
        players_data[id].y += Math.sin(players_data[id].angle)*distance;
      }
      io.sockets.emit("update", players_data);
    }, interval);
  }

  socket.on("disconnect", function () {
    // stop timer if needed
    console.log(socket.id+": disconnect");
    delete players_data[players_uuid[socket.id]];
    delete players_uuid[socket.id];
  });

  socket.on("left", function () {
    // must restrict max angle per interval
    console.log(socket.id+": left");
    players_data[players_uuid[socket.id]].angle += sensibility;
    players_data[players_uuid[socket.id]].angle %= 2*Math.PI;
  });

  socket.on("right", function () {
    // must restrict max angle per interval
    console.log(socket.id+": right");
    players_data[players_uuid[socket.id]].angle -= sensibility;
    players_data[players_uuid[socket.id]].angle %= 2*Math.PI;
  });
});
