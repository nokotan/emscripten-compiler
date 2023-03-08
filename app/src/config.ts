const onServer = !!process.env["DYNO"];
const homeDir = process.env["HOME"];
const wasmceptionDir = process.env["WASMCEPTION"] ||
  (__dirname + '/../wasmception');
const emscriptenDir = process.env["EMSDK"];

export const sysroot = onServer ?
  homeDir + '/sysroot' :
  wasmceptionDir + '/sysroot';

export const llvmDir = onServer ?
  homeDir + '/clang' :
  wasmceptionDir + '/dist';

export const emccDir = (onServer || true) ?
  emscriptenDir + '/emscripten/sdk':
  emscriptenDir + '/upstream/emscripten';

export const tempDir = "/tmp";
