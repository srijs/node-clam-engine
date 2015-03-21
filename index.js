var Engine = require('./build/Release/engine');

exports.createEngine = function (cb) {
  var engine = new Engine();
  engine.init(function (err, sigs) {
    if (err) {
      cb(err);
    } else {
      engine.signatures = sigs;
      cb(null, engine);
    }
  });
};
