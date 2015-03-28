# node-clam-engine [![Build Status](https://travis-ci.org/srijs/node-clam-engine.svg?branch=master)](https://travis-ci.org/srijs/node-clam-engine)

Clam Engine provides robust native bindings to the clamav library. It allows you to scan files for viruses in-process, without requiring a running instance of `clamd`, or imposing the startup time of the `clamscan` command.

## Installation

### Dependencies

Clam Engine requires `libclamav` to be present.

- Ubuntu: `apt-get install libclamav-dev clamav-freshclam`
- Homebew (OS X): `brew install clamav`

### Via NPM

Just run `npm install clam-engine`. Clam Engine supports Node.js 0.10, 0.11 and 0.12 as well as io.js. If something doesn't work for you using one of those runtimes, please file a bug.

## Usage

### Updating Signatures

In order for Clam Engine to work, you need to update the clamav virus signatures by running the `freshclam` command. You will need to configure it firsy.

### Example

    var clam = require('clam-engine');
    
    clam.createEngine(function (err, engine) {
      if (err) {
        return console.log('Error', err);
      }
      engine.scanFile('eicar.com', function (err, virus) {
        if (err) {
          return console.log('Error', err);
        }
        if (virus) {
          return console.log('Virus', virus);
        }
        console.log('Clean');
      });
    });

### API

- #### `clam.createEngine(cb)`
  Creates and initialises a new engine. The callback is called with `(err, engine)`.

- #### `engine.version`
  Property that contains the version of the underlying clamav engine.

- #### `engine.signatures`
  Propery that indicates the number of signatures loaded.

- #### `engine.scanFile(filename, cb)`
  Scans a file for a virus. The callback is called with `(err, virus)`. `virus` is null if the file is clean.
