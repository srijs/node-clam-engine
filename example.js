var clam = require('./index');

console.log('Initialising engine...');
clam.createEngine(function (err, engine) {
  if (err) {
    return console.log(err);
  }
  console.log(engine);
  console.log('Scanning file...');
  engine.scanFile(__dirname + '/eicar.com', function (err, virus) {
    console.log(err, virus);
  });
});
