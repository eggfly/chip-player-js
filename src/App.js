import React, {Component} from 'react';
import './App.css';
import Module from './libgme';

let emu = null;
let node = null;
const audioContext = new AudioContext();

function message(str) {
  console.log(str);
}

function playMusicData(payload, subtune) {
  message("subtune:" + subtune);

  const ref = Module.allocate(1, "i32", Module.ALLOC_NORMAL);
  Module.allocate(1, "i32", Module.ALLOC_NORMAL);

  const samplerate = audioContext.sampleRate;

  if (Module.ccall("gme_open_data", "number", ["array", "number", "number", "number"], [payload, payload.length, ref, samplerate]) !== 0) {
    console.error("gme_open_data failed.");
    return;
  }
  emu = Module.getValue(ref, "i32");

  var subtune_count = Module.ccall("gme_track_count", "number", ["number"], [emu]);

  Module.ccall("gme_ignore_silence", "number", ["number"], [emu, 1]);

  var voice_count = Module.ccall("gme_voice_count", "number", ["number"], [emu]);
  message("Channel count: ", voice_count);
  message("Track count: ", subtune_count);

  // $("#tempo").val(1.0);
  // $("#tempolabel").html(1.0);

  if (Module.ccall("gme_start_track", "number", ["number", "number"], [emu, subtune]) !== 0)
    message("could not load track");
  // setChannels();

  var bufferSize = 4096;
  var inputs = 2;
  var outputs = 2;

  if (!node && audioContext.createJavaScriptNode) node = audioContext.createJavaScriptNode(bufferSize, inputs, outputs);
  if (!node && audioContext.createScriptProcessor) node = audioContext.createScriptProcessor(bufferSize, inputs, outputs);

  var buffer = Module.allocate(bufferSize * 16, "i16", Module.ALLOC_NORMAL);

  var INT16_MAX = Math.pow(2, 16) - 1;

  node.onaudioprocess = function (e) {
    if (Module.ccall("gme_track_ended", "number", ["number"], [emu]) === 1) {
      node.disconnect();
      message("end of stream");
      return;
    }

    var channels = [e.outputBuffer.getChannelData(0), e.outputBuffer.getChannelData(1)];

    var err = Module.ccall("gme_play", "number", ["number", "number", "number"], [emu, bufferSize * 2, buffer]);
    for (var i = 0; i < bufferSize; i++)
      for (var channel = 0; channel < e.outputBuffer.numberOfChannels; channel++)
        channels[channel][i] = Module.getValue(buffer +
          // Interleaved channel format
          // frame number * bytes per frame +
          i * 4 +
          // channel number * bytes per sample
          channel * 2,
          // 16-bit integer
          "i16") / INT16_MAX;
  };
  // node.connect(filterNode);
  node.connect(audioContext.destination);

  window.savedReferences = [audioContext, node];
}

class App extends Component {
  playSong(filename, subtune) {
    if (node) {
      node.disconnect();
      node = null;
    }
    var xhr = new XMLHttpRequest();
    xhr.open("GET", filename, true);
    xhr.responseType = "arraybuffer";
    xhr.onerror = function (e) {
      message(e);
    };
    xhr.onload = function (e) {
      if (this.status === 404) {
        message("not found");
        return;
      }
      var payload = new Uint8Array(this.response);
      playMusicData(payload, subtune);
      //updateSongInfo(filename, subtune);
    };
    xhr.send();
  }



  render() {
    return (
      <div className="App">
        <header className="App-header">
          <h1 className="App-title">JavaScript Game Music Player</h1>
          <small>powered by <a href="https://bitbucket.org/mpyne/game-music-emu/wiki/Home">Game Music Emu</a></small>
        </header>
        <p className="App-intro">
          <button onClick={this.playSong('songs/wt-01.spc')}>Play Music</button>
        </p>
      </div>
    );
  }
}

export default App;