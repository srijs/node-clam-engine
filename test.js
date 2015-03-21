var clam = require('./index');

var assert = require('assert');

clam.createEngine(function (err, engine) {
  assert.ifError(err);
  assert(typeof engine.version === 'string');
  assert(typeof engine.signatures === 'number');
  engine.scanFile('./package.json', function (err, virus) {
    assert.ifError(err);
    assert.equal(virus, null);
  });
});
