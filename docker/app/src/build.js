// Translated from https://github.com/wasdk/wasmexplorer-service/blob/master/web/build.php
// FIXME make me node.js friendly and async

const { promisify } = require("util")
const { emccDir, llvmDir, tempDir, sysroot } = require("../config");
const { mkdirSync, writeFile, existsSync, readFile, unlinkSync } = require("fs");
const { deflate } = require("zlib");
const { dirname } = require("path");
const { exec } = require("child_process");
const { Writable } = require("stream");

const deflateAsync = promisify(deflate);
const execAsync = promisify(exec);
const writeFileAsync = promisify(writeFile);
const readFileAsync = promisify(readFile);

// Input: JSON in the following format
// {
//     output: "wasm",
//     files: [
//         {
//             type: "cpp",
//             name: "file.cpp",
//             options: "-O3 -std=c++98",
//             src: "puts(\"hi\")"
//         }
//     ],
//     link_options: "--import-memory"
// }
// Output: JSON in the following format
// {
//     success: true,
//     message: "Success",
//     output: "AGFzbQE.... =",
//     tasks: [
//         {
//             name: "building file.cpp",
//             file: "file.cpp",
//             success: true,
//             console: ""
//         },
//         {
//             name: "linking wasm",
//             success: true,
//             console: ""
//         }
//     ]
// }

function generate_regexp_from_pattern(pattern) {
  return new RegExp(pattern.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'g');
}

function sanitize_shell_output(out, cwd) {
  return out.replace(generate_regexp_from_pattern(cwd), '.'); // FIXME
}

async function shell_exec(cmd, cwd = tempDir) {
  console.log(cmd);
  let out;
  let error = '';
  try {
    out = await execAsync(cmd, {cwd});
  } catch (ex) {
    error = ex.message;
  }
  const result = (out && out.stderr) || error;
  return result;
}

function get_clang_options(options) {
  // const clang_flags = `--target=wasm32-unknown-unknown-wasm --sysroot=${sysroot} -fdiagnostics-print-source-range-info -fno-exceptions`;
  const clang_flags = ``;
  if (!options) {
    return clang_flags;
  }
  const available_options = [
    '-O0', '-O1', '-O2', '-O3', '-Oz', '-Os', 
    '-fno-exceptions', '-fno-rtti', '-ffast-math', '-fno-inline', '-fcoroutine-ts', '-fcolor-diagnostics',
    '-mreference-types',
    '-std=c89', '-std=c99', '-std=c11', 
    '-std=c++98', '-std=c++11', '-std=c++14', '-std=c++17', '-std=c++2a',
    '-g', '-Wall',
    '-Iinclude',
    '-I/include/DxLib/',
    '-include-pch /include/DxLib/DxLib.O0.pch', '-include-pch /include/DxLib/DxLib.O2.pch',
    '-include-pch /include/DxLib/DxLib.Oz.pch',
    '-I/include/OpenSiv3D/', '-I/include/OpenSiv3D/ThirdParty/',
    '-include-pch /include/OpenSiv3D/Siv3D.O0.pch', '-include-pch /include/OpenSiv3D/Siv3D.O2.pch',
    '-include-pch /include/OpenSiv3D/Siv3D.Oz.pch',
    '-I/include/OpenSiv3Dv0.6/', '-I/include/OpenSiv3Dv0.6/ThirdParty/',
    '-include-pch /include/OpenSiv3Dv0.6/Siv3D.O0.pch', '-include-pch /include/OpenSiv3Dv0.6/Siv3D.O2.pch',
    '-include-pch /include/OpenSiv3Dv0.6/Siv3D.Oz.pch',
    '-s DISABLE_EXCEPTION_CATCHING=0', '-s DISABLE_EXCEPTION_CATCHING=2',
    '-s MAIN_MODULE=1', '-s SIDE_MODULE=1', '-s STRICT_JS=1',
    '-s MAIN_MODULE=2', '-s SIDE_MODULE=2',
    '-s USE_SDL=0', '-s USE_SDL_IMAGE=0', '-s USE_SDL_TTF=0', '-s USE_SDL_NET=0',
    '-s USE_SDL=2', '-s USE_SDL_IMAGE=2', '-s USE_SDL_TTF=2', '-s USE_SDL_NET=2',
    '-s USE_ZLIB=1', '-s USE_LIBJPEG=1', '-s USE_LIBPNG=1',
    '-s USE_BULLET=1', '-s USE_VORBIS=1', '-s USE_OGG=1', '-s USE_FREETYPE=1',

    '-D_XM_NO_INTRINSICS_',
  ];
  let safe_options = '-c';
  for (let o of available_options) {
    if (options.includes(o)) {
      safe_options += ' ' + o;
    } else if (o.includes('-std=') && options.toLowerCase().includes(o)) {
      safe_options += ' ' + o;
    }
  }
  return clang_flags + ' ' + safe_options;
}


function get_lld_options(options) {
  // const clang_flags = `--target=wasm32-unknown-unknown-wasm --sysroot=${sysroot} -nostartfiles -Wl,--allow-undefined,--no-entry,--no-threads,--export-dynamic`;
  const clang_flags = ``;
  if (!options) {
    return clang_flags;
  }
  const available_options = [
    '--import-memory', '-g', 
    '--no-export-dynamic', '--export-dynamic', '-fvisibility=default', '-fvisibility=hidden',
    '-O0', '-O1', '-O2', '-O3', '-Oz', '-Os',
    '-fcoroutines-ts', '-fcolor-diagnostics',
    '-s ALLOW_MEMORY_GROWTH=1', 
    '-s DISABLE_EXCEPTION_CATCHING=0', '-s DISABLE_EXCEPTION_CATCHING=2',
    '-s DEMANGLE_SUPPORT=1',
    '-s FULL_ES2=1', '-s FULL_ES3=1', '-s USE_WEBGPU=1', 
    '-s USE_GLFW=3',
    '-s ASYNCIFY=1', '-s EXPORT_ALL=1',
    '-s MAIN_MODULE=1', '-s SIDE_MODULE=1', '-s STRICT_JS=1',
    '-s MAIN_MODULE=2', '-s SIDE_MODULE=2',
    '-s USE_SDL=0', '-s USE_SDL_IMAGE=0', '-s USE_SDL_TTF=0', '-s USE_SDL_NET=0',
    '-s USE_SDL=2', '-s USE_SDL_IMAGE=2', '-s USE_SDL_TTF=2', '-s USE_SDL_NET=2',
    '-s USE_ZLIB=1', '-s USE_LIBJPEG=1', '-s USE_LIBPNG=1',
    '-s USE_BULLET=1', '-s USE_VORBIS=1', '-s USE_OGG=1', '-s USE_FREETYPE=1',
    '-s FETCH=1', '-s MINIMAL_RUNTIME=1',
    '-s FORCE_FILESYSTEM=1',
    '--use-preload-plugins', '--emrun',

    '-s EXPORTED_FUNCTIONS="[ \'__Z4Mainv\' ]"',

    '-s ASYNCIFY=1', '-s ASYNCIFY_IGNORE_INDIRECT=1',
    '-s ASYNCIFY_IMPORTS="[ \'_ZN3s3d6System6UpdateEv\' ]"',
    '-s ASYNCIFY_IMPORTS="[ \'_ZN3s3d6System6UpdateEv\', \'_ZN3s3d7TextureC1ENS_10StringViewENS_11TextureDescE\', \'_ZN3s3d5ImageC1ENS_10StringViewENS_11ImageFormatE\', \'_ZN3s3d5AudioC1ENS_10StringViewE\', \'_ZN3s3d5AudioC1ENS_10StringViewENS_5YesNoINS_8Loop_tagEEE\' ]"',
    '-s ASYNCIFY_IMPORTS="[ \'_ZN3s3d6System6UpdateEv\', \'_ZN3s3d7TextureC1ENS_10StringViewENS_11TextureDescE\', \'_ZN3s3d5ImageC1ENS_10StringViewENS_11ImageFormatE\', \'_ZN3s3d5AudioC1ENS_10StringViewE\', \'_ZN3s3d5AudioC1ENS_10StringViewENS_5YesNoINS_8Loop_tagEEE\', \'_ZN3s3d5ModelC1ENS_10StringViewENS_11ColorOptionE\' ]"',
    '-s ASYNCIFY_IMPORTS="[ \'_ZN3s3d6System6UpdateEv\', \'_ZN3s3d7TextureC1ENS_10StringViewENS_11TextureDescE\', \'_ZN3s3d5ImageC1ENS_10StringViewENS_11ImageFormatE\', \'_ZN3s3d5AudioC1ENS_10StringViewE\', \'_ZN3s3d5AudioC1ENS_10StringViewENS_5YesNoINS_8Loop_tagEEE\', \'_ZN3s3d5ModelC1ENS_10StringViewENS_11ColorOptionE\', \'_ZN3s3d6Dialog11OpenTextureENS_10StringViewES1_\', \'_ZN3s3d6Dialog9OpenAudioENS_10StringViewES1_\', \'_ZN3s3d6Dialog8OpenFileERKNS_5ArrayINS_10FileFilterENSt3__29allocatorIS2_EEEENS_10StringViewES9_\', \'_ZN3s3d6Dialog9OpenFilesERKNS_5ArrayINS_10FileFilterENSt3__29allocatorIS2_EEEENS_10StringViewES9_\' ]"',
    '-s ASYNCIFY_IMPORTS="[ \'_ZN3s3d6System6UpdateEv\', \'_ZN3s3d7TextureC1ENS_10StringViewENS_11TextureDescE\', \'_ZN3s3d5ImageC1ENS_10StringViewENS_11ImageFormatE\', \'_ZN3s3d5AudioC1ENS_10StringViewE\', \'_ZN3s3d5AudioC1ENS_10StringViewENS_5YesNoINS_8Loop_tagEEE\', \'_ZN3s3d5ModelC1ENS_10StringViewENS_11ColorOptionE\', \'_ZN3s3d6Dialog9OpenImageENS_10StringViewES1_\', \'_ZN3s3d6Dialog11OpenTextureENS_10StringViewES1_\', \'_ZN3s3d6Dialog9OpenAudioENS_10StringViewES1_\', \'_ZN3s3d6Dialog8OpenFileERKNS_5ArrayINS_10FileFilterENSt3__29allocatorIS2_EEEENS_10StringViewES9_\', \'_ZN3s3d6Dialog9OpenFilesERKNS_5ArrayINS_10FileFilterENSt3__29allocatorIS2_EEEENS_10StringViewES9_\' ]"',
    '-s ASYNCIFY_ADD="[ \'Main()\' ]"',
  ];
  let safe_options = '';
  for (let o of available_options) {
    if (options.includes(o)) {
      safe_options += ' ' + o;
    }
  }
  return clang_flags + safe_options;
}

function is_side_module_build(options) {
  const available_options = [
    '-s SIDE_MODULE=1', '-s SIDE_MODULE=2',
  ];
  
  for (let o of available_options) {
    if (options.includes(o)) {
      return true;
    }
  }

  return false;
}

async function serialize_file_data(filename, compress) {
  let content = await readFileAsync(filename);
  if (compress) {
    content = await deflateAsync(content);
  }
  return content.toString("base64");
}

async function get_file_data(filename, compress) {
  let content = await readFileAsync(filename);
  if (compress) {
    content = await deflateAsync(content);
  }
  return content.toString();
}

async function build_c_file(input, options, output, cwd, compress, result_obj) {
  // const cmd = llvmDir + '/bin/clang ' + get_clang_options(options) + ' ' + input + ' -o ' + output;
  const cmd = emccDir + '/emcc ' + get_clang_options(options) + ' ' + input + ' -o ' + output;
  const out = await shell_exec(cmd, cwd);
  result_obj.console = await sanitize_shell_output(out, cwd);
  if (!existsSync(output)) {
    result_obj.success = false;
    return false;
  }
  result_obj.success = true;
  // result_obj.output = serialize_file_data(output, compress);
  return true;
}

async function build_cpp_file(input, options, output, cwd, compress, result_obj) {
  // const cmd = llvmDir + '/bin/clang++ ' + get_clang_options(options) + ' ' + input + ' -o ' + output;
  const cmd = emccDir + '/em++ ' + get_clang_options(options) + ' ' + input + ' -o ' + output;
  const out = await shell_exec(cmd, cwd);
  result_obj.console = await sanitize_shell_output(out, cwd);
  if (!existsSync(output)) {
    result_obj.success = false;
    return false;
  }
  result_obj.success = true;
  // result_obj.output = serialize_file_data(output, compress);
  return true;
}

function validate_filename(name) {
  if (!/^[0-9a-zA-Z\-_.]+(\/[0-9a-zA-Z\-_.]+)*$/.test(name)) {
    return false;
  }
  const parts = name.split(/\//g);
  for(let p of parts) {
    if (p == '.' || p == '..') {
      return false;
    }
  }
  return parts;
}

async function link_obj_files(obj_files, options, cwd, has_cpp, output, result_obj) {
  const files = obj_files.join(' ');
  let clang;
  if (has_cpp) {
    // clang = llvmDir + '/bin/clang++';
    clang = emccDir + '/em++';
  } else {
    // clang = llvmDir + '/bin/clang';    
    clang = emccDir + '/emcc';    
  }
  const cmd = clang + ' ' + get_lld_options(options) + ' ' + files + ' -o ' + output;
  const out = await shell_exec(cmd, cwd);
  result_obj.console = await sanitize_shell_output(out, cwd);
  if (!existsSync(output)) {
    result_obj.success = false;
    return false;
  }
  result_obj.success = true;
  return true;
}

async function build_project(project, base, callback) {
  const output = project.output;
  const compress = project.compress;
  const build_result = { };
  const dir = base + '.$';
  const result_wasm = dir + '/main.wasm';
  const result_js = dir + '/main.js';

  const complete = async (success, message) => {
    shell_exec("rm -rf " + dir);
  
    build_result.success = success;
    build_result.message = message;
    callback(build_result);
  };

  if (output != 'wasm') {
    return complete(false, 'Invalid output type ' + output);
  }

  if (!existsSync(dir)) {
    mkdirSync(dir);
  }
  build_result.tasks = [];
  const files = project.files;
  for (let file of files) {
    const name = file.name;
    if (!validate_filename(name)) {
      return complete(false, 'Invalid filename ' + name);
    }
    const fileName = dir + '/' + name;
    const subdir = dirname(fileName);
    if (!existsSync(subdir)) {
      mkdirSync(subdir);
    }
    const src = file.src;
    await writeFileAsync(fileName, src);
  }
  const obj_files = [];
  let clang_cpp = false;
  for (let file of files) {
    const name = file.name;
    const fileName = dir + '/' + name;
    const type = file.type;
    const options = file.options;
    let success = true;
    const result_obj = {
      name: `building ${name}`,
      file: name
    };
    build_result.tasks.push(result_obj);
    if (type == 'c') {
      success = await build_c_file(fileName, options, fileName + '.o', dir, compress, result_obj);
      obj_files.push(fileName + '.o');
    } else if (type == 'cpp') {
      clang_cpp = true;
      success = await build_cpp_file(fileName, options, fileName + '.o', dir, compress, result_obj);
      obj_files.push(fileName + '.o');
    }
    if (!success) {
      return complete(false, 'Error during build of ' + name);
    }
  }
  const link_options = project.link_options;
  const link_result_obj = {
    name: 'linking wasm'
  };
  build_result.tasks.push(link_result_obj);

  if (is_side_module_build(link_options)) {
    if (!(await link_obj_files(obj_files, link_options, dir, clang_cpp, result_wasm, link_result_obj))) {
      return complete(false, 'Error during linking');
    }
  
    const [wasm] = await Promise.all([
      serialize_file_data(result_wasm, compress)
    ]);
  
    build_result.output = wasm;
  } else {
    if (!(await link_obj_files(obj_files, link_options, dir, clang_cpp, result_js, link_result_obj))) {
      return complete(false, 'Error during linking');
    }
  
    const [wasm, wasmBindgenJs] = await Promise.all([
      serialize_file_data(result_wasm, compress), get_file_data(result_js, false)
    ]);
  
    build_result.output = wasm;
    build_result.wasmBindgenJs = wasmBindgenJs;
  }
 
  return complete(true, 'Success');
}

module.exports = (input, callback) => {
  const baseName = tempDir + '/build_' + Math.random().toString(36).slice(2);
  try {
    console.log('Building in ', baseName);
    build_project(input, baseName, result => callback(null, result));
  } catch (ex) {
    callback(ex);
  }
};
