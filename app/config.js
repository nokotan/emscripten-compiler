const onServer = !!process.env["DYNO"];
const homeDir = process.env["HOME"];
const wasmceptionDir = process.env["WASMCEPTION"] ||
  (__dirname + '/../wasmception');
const emscriptenDir = process.env["EMSDK"];

exports.sysroot = onServer ?
  homeDir + '/sysroot' :
  wasmceptionDir + '/sysroot';

exports.llvmDir = onServer ?
  homeDir + '/clang' :
  wasmceptionDir + '/dist';

exports.emccDir = (onServer || true) ?
  emscriptenDir + '/emscripten/sdk':
  emscriptenDir + '/upstream/emscripten';

exports.tempDir = "/tmp";
